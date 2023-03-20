
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vorgabe.h"


fileInfoList list;


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





int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Das einzige Argument muss ein Pfad zum komprimierenden Verzeichnis sein.\n");
        exit(1);
    }
    
    
    {
        char* path = strdup(argv[1]);
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
    if (argv[1][strlen(argv[1])-1] == '/') {
        argv[1][strlen(argv[1])-1] = '\0';
    }
    
    init(&list);
    
    traverse(argv[1]);
    
    fileInfo* info;
    while ((info = dequeue(&list)) != NULL) {
        printf("%s: %ld bytes\n", info->path, info->size);
        free(info->path);
        free(info);
    }
    
    
    return 0;
}

