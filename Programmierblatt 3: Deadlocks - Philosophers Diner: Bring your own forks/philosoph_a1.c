
// damit es bei usleep keine implicit function declaration Warnung gibt
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>



bool hatlinks = false;
bool hatrechts = false;

sem_t* links = SEM_FAILED;
sem_t* rechts = SEM_FAILED;

void sigint(int num) {
    if (num == SIGINT) {
        if (hatlinks) {
            sem_post(links);
        }
        if (hatrechts) {
            sem_post(rechts);
        }
        sem_close(links);
        sem_close(rechts);
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    srand(getpid());
    if (argc != 3) {
        puts("Es müssen die linke und rechte Gabel als Argument gegeben werden!\n");
        exit(1);
    }
    links = sem_open(argv[1], O_RDWR);
    rechts = sem_open(argv[2], O_RDWR);
    if (links == SEM_FAILED || rechts == SEM_FAILED) {
        perror("Konnte eine der Gabeln nicht öffnen");
        exit(1);
    }
    struct sigaction action = {};
    action.sa_handler = sigint;
    action.sa_flags = 0;
    
    sigaction(SIGINT, &action, NULL);
    
    while (true) {
        printf("Philosoph %d will die linke Gabel nehmen.\n", getpid());
        if (sem_wait(links) != 0) {
            perror("Fehler bei sem_wait");
            exit(1);
        }
        hatlinks = true;
        printf("Philosoph %d hat die linke Gabel.\n", getpid());
        
        usleep( 3 * (rand() % 100));
        
        printf("Philosoph %d will die rechte Gabel nehmen.\n", getpid());
        if (sem_wait(rechts) != 0) {
            perror("Fehler bei sem_wait");
            exit(1);
        };
        hatrechts = true;
        printf("Philosoph %d hat die rechte Gabel.\n", getpid());
        
        printf("Philosoph %d isst.\n", getpid());
        usleep(1000);
        
        printf("Philosoph %d legt die rechte Gabel weg.\n", getpid());
        if (sem_post(rechts) != 0) {
            perror("Fehler bei sem_post");
            exit(1);
        };
        hatrechts = false;
        
        printf("Philosoph %d legt die linke Gabel weg.\n", getpid());
        if (sem_post(links) != 0) {
            perror("Fehler bei sem_post");
            exit(1);
        };
        hatlinks = false;
        printf("Philosoph %d denkt.\n", getpid());
        
        usleep(100 * (rand() % 10));
    }
    return 0;
}

