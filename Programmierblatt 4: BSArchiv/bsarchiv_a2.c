
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdint.h>

#include "vorgabe.h"


fileInfoList list;


#define ARCHIVE_MAGIC "BSARCH"


void traverse(char* path) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }
    struct dirent* e;
    errno = 0;
    while ((e = readdir(dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
            errno = 0;
            continue;
        }
        
        char* newpath;
        if (asprintf(&newpath, "%s/%s", path, e->d_name) == -1) {
            perror("asprintf");
            exit(1);
        }
        
        struct stat s;
        if (stat(newpath, &s) != 0) {
            perror("stat");
            free(newpath);
            exit(1);
        }
        
        if (S_ISREG(s.st_mode)) {
            fileInfo* info = malloc(sizeof(fileInfo));
            if (info == NULL) {
                perror("malloc");
                free(newpath);
                exit(1);
            }
            info->path = strdup(newpath);
            if (info->path == NULL) {
                perror("strdup");
                free(newpath);
                exit(1);
            }
            info->size = s.st_size;
            enqueue(&list, info);
        }
        
        if (S_ISDIR(s.st_mode)) {
            traverse(newpath);
        }
        
        free(newpath);
        
        errno = 0;
    }
    if (errno != 0) {
        perror("readdir");
        exit(1);
    }
    if (closedir(dir) != 0) {
        perror("closedir");
        exit(1);
    }
}

void writeBuffer(int fd, void* buffer, int len) {
    while (len > 0) {
        int ret = write(fd, buffer, len);
        if (ret == -1) {
            perror("write");
            exit(1);
        }
        len -= ret;
    }
}





int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Das erste Argument muss der Archivname und das zweite Argument ein Pfad zum komprimierenden Verzeichnis sein.\n");
        exit(1);
    }
    
    {
        char* path = strdup(argv[2]);
        if (path == NULL) {
            perror("strdup");
            exit(1);
        }
        char* tok = strtok(path, "/");
        if (tok == NULL) {
            printf("Ungültiger Pfad.\n");
            free(path);
            exit(1);
        }
        do {
            if (strcmp(".", tok) == 0 || strcmp("..", tok) == 0) {
                printf("Ungültiger Pfad.\n");
                free(path);
                exit(1);
            }
        } while ((tok = strtok(NULL, "/")) != NULL);
        free(path);
    }
    
    // damit der Einfachheit halber auch Namen mit / hinten dran angenommen werden
    if (argv[2][strlen(argv[2])-1] == '/') {
        argv[2][strlen(argv[2])-1] = '\0';
    }
    
    init(&list);
    
    traverse(argv[2]);
    
    int archive = open(argv[1], O_CREAT | O_WRONLY);
    if (archive == -1) {
        perror("Fehler beim Archivdatei öffnen");
        exit(1);
    }
    
    writeBuffer(archive, ARCHIVE_MAGIC, strlen(ARCHIVE_MAGIC));
    
    uint16_t version = 1;
    writeBuffer(archive, &version, sizeof(version));
    
    errno = 0;
    uint64_t toc = lseek(archive, 0, SEEK_CUR) + sizeof(toc);
    if (errno != 0) {
        perror("lseek");
        close(archive);
        exit(1);
    }
    
    writeBuffer(archive, &toc, sizeof(toc));
    
    uint64_t toc_size = 0;
    writeBuffer(archive, &toc_size, sizeof(toc_size));
    
    
    uint64_t startoffset = 0;
    fileInfo* info = list.head;
    if (info != NULL) {
        do {
            printf("%s: %ld bytes\n", info->path, info->size);
            
            uint16_t pathlength = strlen(info->path)+1;
            writeBuffer(archive, &pathlength, sizeof(pathlength));
            toc_size += sizeof(pathlength);
            writeBuffer(archive, info->path, pathlength);
            toc_size += pathlength;
            
            uint64_t length = info->size;
            writeBuffer(archive, &length, sizeof(length));
            toc_size += sizeof(length);
            
            writeBuffer(archive, &startoffset, sizeof(startoffset));
            toc_size += sizeof(startoffset);
            
            startoffset += info->size;
        } while ((info = info->next) != NULL);
    }
    
    
    // Größe des Inhaltsverzeichnisses schreiben
    
    errno = 0;
    lseek(archive, toc, SEEK_SET);
    if (errno != 0) {
        perror("lseek");
        close(archive);
        exit(1);
    }
    
    writeBuffer(archive, &toc_size, sizeof(toc_size));
    
    
    errno = 0;
    lseek(archive, 0, SEEK_END);
    if (errno != 0) {
        perror("lseek");
        close(archive);
        exit(1);
    }
    
    while ((info = dequeue(&list)) != NULL) {
        int file = open(info->path, O_RDONLY);
        if (file == -1) {
            perror("open");
            close(archive);
            exit(1);
        }
        
        if (sendfile(archive, file, NULL, info->size) == -1) {
            perror("sendfile");
            close(file);
            close(archive);
            exit(1);
        }
        
        close(file);
        
        free(info->path);
        free(info);
    }
    
    
    close(archive);
    
    return 0;
}

