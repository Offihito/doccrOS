#include <unistd.h>
#include <stdio.h>

static void print_long(long n)
{
    if (n < 0)
    {
        write(1, "-", 1);
        n = -n;
    }
    char buf[20];
    int  i = 0;
    if (n == 0)
    {
        write(1, "0", 1);
        return;
    }
    while (n > 0)
    {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    for (int j = i - 1; j >= 0; j--)
        write(1, &buf[j], 1);
}

int main(void)
{
    const char *path = "/bin/hello.elf";

    write(1, "[syscall_test] opening ", 23);
    write(1, path, 14);
    write(1, "\n", 1);

    long fd = open(path, 0);

    if (fd < 0)
    {
        write(1, "[syscall_test] open failed\n", 27);
        _exit(1);
    }

    write(1, "[syscall_test] got fd=", 22);
    print_long(fd);
    write(1, "\n", 1);

    long ret = close((int)fd);

    if (ret < 0)
    {
        write(1, "[syscall_test] close failed\n", 28);
        _exit(1);
    }

    write(1, "[syscall_test] close ok\n", 24);

    long fd2 = close((int)fd);
    if (fd2 < 0)
        write(1, "[syscall_test] double-close correctly rejected\n", 47);

    write(1, "[syscall_test] all done\n", 24);
    return 0;
}
