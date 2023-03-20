#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {
    printf("Benutzer-ID: %d\n", getuid());
    printf("Gruppen-ID: %d\n", getgid());
    return 0;
}
