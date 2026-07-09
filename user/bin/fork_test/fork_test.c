#include <unistd.h>
#include <stdio.h>

int main(void)
{
    printf("[fork_test] before fork, pid=%ld\n", getpid());

    long pid = fork();

    if (pid < 0) {
        printf("[fork_test] fork failed\n");
        _exit(1);
    }

    if (pid == 0) {
        printf("[fork_test] child  running, pid=%ld\n", getpid());
        _exit(0);
    }

    printf("[fork_test] parent running, child pid=%ld\n", pid);
    return 0;
}
