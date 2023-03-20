#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


int thread_func(void* arg) {
    int res = 0;
    for (int i = 1; i<=5; i++) {
        res += i*i;
    }
    usleep(2000000);
    return res;
}


#define STACK_SIZE 8192



int main(int argc, char** argv) {
    
    
    void* stack = malloc(STACK_SIZE);
    if (stack == NULL) {
        puts("Malloc des Stack fehlgeschlagen");
        exit(1);
    }
    
    int tid = clone(thread_func, stack+STACK_SIZE, CLONE_VM | SIGCHLD, NULL);
    if (tid == -1) {
        perror("clone");
        exit(1);
    }
    printf("tid: %d\n", tid);
    
    
    
    int status;
    if (waitpid(tid, &status, 0) == -1) {
        perror("waitpid");
        exit(1);
    }
    printf("Ergebnis: %d\n", WEXITSTATUS(status));
    
    
    
    return 0;
}

