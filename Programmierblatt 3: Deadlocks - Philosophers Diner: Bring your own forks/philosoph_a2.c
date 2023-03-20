
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
#include <pthread.h>
#include <string.h>
#include <stdint.h>


int lese;
int schreibe;


bool boss = false;


pthread_mutex_t booleanlock = PTHREAD_MUTEX_INITIALIZER;

void try_lock(pthread_mutex_t* m) {
    if (pthread_mutex_lock(m) != 0) {
        perror("Fehler bei pthread_lock");
        exit(1);
    }
}

void try_unlock(pthread_mutex_t* m) {
    if (pthread_mutex_unlock(m) != 0) {
        perror("Fehler bei pthread_unlock");
        exit(1);
    }
}

bool willgabel = false;


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

bool lastblocked = false;


void* watcher(void* data) {
    if (boss) {
        while (true) {
            uint8_t data;
            try_lock(&booleanlock);
            if (willgabel) {
                if (lastblocked) {
                    data = 1;
                    // pipes blockieren immer bis mindestens 1 byte an daten da ist. Weil wir nur 1 byte brauchen, müssen wir nicht auf die Anzahl geschriebener oder gelesener bytes checken
                    if (write(schreibe, &data, 1) == -1) {
                        perror("Fehler beim schreiben der pipe");
                        exit(1);
                    }
                    if (read(lese, &data, 1) == -1) {
                        perror("Fehler beim lesen von der pipe");
                        exit(1);
                    }
                    if (data == 1) {
                        puts("DEADLOCK\n");
                    }
                } else {
                    lastblocked = true;
                }
            } else {
                lastblocked = false;
            }
            try_unlock(&booleanlock);
            sleep(1);
        }
    } else {
        while (true) {
            uint8_t data;
            if (read(lese, &data, 1) == -1) {
                perror("Fehler beim lesen von der pipe");
                exit(1);
            }
            if (data == 1){
                try_lock(&booleanlock);
                if (willgabel) {
                    data = 1;
                } else {
                    data = 0;
                }
                try_unlock(&booleanlock);
            }
            if (write(schreibe, &data, 1) == -1) {
                perror("Fehler beim schreiben der pipe");
                exit(1);
            }
        }
    }
}



int main(int argc, char* argv[]) {
    srand(getpid());
    if (argc != 5 && argc != 6) {
        puts("Es müssen die linke und rechte Gabel und die lese und schreib-Pipe als Argumente gegeben werden!\n");
        puts("Dannach kann auch noch istboss benutzt werden, um den Philosophen zum Boss zu machen.\n");
        exit(1);
    }
    
    if (argc == 6 && strcmp(argv[5], "istboss") == 0) {
        boss = true;
    }
    
    links = sem_open(argv[1], O_RDWR);
    rechts = sem_open(argv[2], O_RDWR);
    if (links == SEM_FAILED || rechts == SEM_FAILED) {
        perror("Konnte eine der Gabeln nicht öffnen");
        exit(1);
    }
    // beim Boss schreibeende zuerst öffnen, damit es kein Deadlock gibt
    if (boss) {
        schreibe = open(argv[4], O_WRONLY);
        lese = open(argv[3], O_RDONLY);
    } else {
        lese = open(argv[3], O_RDONLY);
        schreibe = open(argv[4], O_WRONLY);
    }
    if (lese == -1 || schreibe == -1) {
        perror("Konnte eine der Pipes nicht öffnen");
        exit(1);
    }
    
    struct sigaction action = {};
    action.sa_handler = sigint;
    action.sa_flags = 0;
    
    sigaction(SIGINT, &action, NULL);
    
    pthread_t watch;
    if (pthread_create(&watch, NULL, watcher, NULL) != 0) {
        perror("Konnte watcher thread nicht erstellen");
        exit(1);
    }
    // am Anfang einmal schlafen, weil die Prozesse durch die Pipes synchronisiert sind
    usleep(100000 * (rand() % 10));
    
    while (true) {
        
        printf("Philosoph %d will die linke Gabel nehmen.\n", getpid());
        
        try_lock(&booleanlock);
        willgabel = true;
        try_unlock(&booleanlock);
        
        if (sem_wait(links) != 0) {
            perror("Fehler bei sem_wait");
            exit(1);
        }
        hatlinks = true;
        
        try_lock(&booleanlock);
        willgabel = false;
        try_unlock(&booleanlock);
        
        printf("Philosoph %d hat die linke Gabel.\n", getpid());
        
        // Das hier kann verringert werden, damit es nicht zu schnell zum Deadlock kommt
        usleep( 3000 * (rand() % 100));
        
        printf("Philosoph %d will die rechte Gabel nehmen.\n", getpid());
        
        try_lock(&booleanlock);
        willgabel = true;
        try_unlock(&booleanlock);
        
        if (sem_wait(rechts) != 0) {
            perror("Fehler bei sem_wait");
            exit(1);
        };
        hatrechts = true;
        
        try_lock(&booleanlock);
        willgabel = false;
        try_unlock(&booleanlock);
        
        printf("Philosoph %d hat die rechte Gabel.\n", getpid());
        
        printf("Philosoph %d isst.\n", getpid());
        usleep(100000);
        
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
        
        usleep(1000 * (rand() % 10));
    }
    return 0;
}

