#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    for(int i = 0; i < 4; i++)
    {
        int p = fork();
        if(p == 0){
            exit(0);
        }
    }
    for(int i = 0; i < 5; i++)
    {
        int p = fork();
        if(p == 0){
            exit(0);
        }
        sleep(2);
    }
    return 0;
}
