#define _GNU_SOURCE
#define _POSIX_SOURCE



#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<pwd.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<stdbool.h>
#include<limits.h>
#include<string.h>
#include<signal.h>


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
    signal(SIGCHLD, SIG_IGN);
    
    
    char cmd[256];
    char arg[256];
    while (true) {
        // damit Zombies eingesammelt werden
        while (waitpid(-1, NULL, WNOHANG) == 0) {}
        char cwd[PATH_MAX];
        if (getcwd(cwd, PATH_MAX) == NULL) {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }
        printf("%s@%s %s $ ", p->pw_name, hostname, cwd);
        errno = 0;
        int ret = scanf("%255s %255s", cmd, arg);
        if (ret == 2) {
            if (strncmp("exit", cmd, 256) == 0) {
                char* end;
                long exitcode = strtol(arg, &end, 10);
                if (*end == '\0') {
                    exit(exitcode);
                } else {
                    printf("Der exit code muss eine Zahl sein\n");
                }
                continue;
            }
            if (strncmp("cd", cmd, 256) == 0) {
                if (chdir(arg) == -1) {
                    perror("chdir");
                }
                continue;
            }
            pid_t ret = fork();
            char* realcmd = cmd;
            if (cmd[0] == '!') {
                realcmd = cmd+1;
            }
            if (ret == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (ret == 0) {
                execlp(realcmd, realcmd, arg, NULL);
                perror("execpl");
            } else {
                if (cmd[0] != '!') {
                    wait(NULL);
                } else {
                    printf("pid: %d\n", ret);
                }
            }
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





