#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>

void    print_error(char *str)
{
    while (*str)
        write(2, str++, 1);
}

int main(int argc, char **argv, char **env)
{
    if (argc == 1)
        return (0);
    int i = 1;
    while (argv[i])
    {
        if (strcmp(argv[i], ";") == 0 || strcmp(argv[i], "|") == 0)
        {
            i++;
            continue ;
        }
        if (strcmp(argv[i], "cd") == 0)
        {
            i += 2;
            continue;
        }
        pid_t   pid = fork();
        if (pid == 0)
        {
            execve(argv[i], &argv[i], env);
            print_error("error: cannot execute ");
            print_error(argv[i]);
            print_error("\n");
            exit(1);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
        i++;
    }
    return (0);
}