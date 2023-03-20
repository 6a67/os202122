#define _GNU_SOURCE
#define _POSIX_SOURCE



#include<stdio.h>
#include<sys/types.h>
#include<pwd.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<stdbool.h>
#include<limits.h>


int main(void) {
    
    struct passwd *p = getpwuid(getuid());
    if (p == NULL) {
        perror("getpwuid");
    }
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, 100) == -1) {
        perror("gethostname");
        exit(EXIT_FAILURE);
    }
    
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    
    char cmd[256];
    char arg[256];
    while (true) {
        printf("%s@%s %s $ ", p->pw_name, hostname, cwd);
        errno = 0;
        int ret = scanf("%255s %255s", cmd, arg);
        if (ret == 2) {
            execlp(cmd, cmd, arg, NULL);
            perror("execpl");
        } else {
            if (errno != 0) {
                perror("scanf");
                exit(EXIT_FAILURE);
            }
            printf("Es muss genau ein Kommando und ein Parameter angegeben werden.\n");
        }
    }
    return EXIT_SUCCESS;
}





