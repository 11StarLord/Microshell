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

int count_args(char **argv, int i)
{
    int count;

    count = 0;
    while (argv[i] && strcmp(argv[i], ";") != 0 && strcmp(argv[i], "|") != 0)
    {
        i++;
        count++;
    }
    return (count);
}

int main(int argc, char **argv, char **env)
{
    int i;
    int number_args;

    if (argc == 1)
        return (0);
    i = 1;
    while (argv[i])
    {
        if (strcmp(argv[i], ";") == 0 || strcmp(argv[i], "|") == 0)
        {
            i++;
            continue ;
        }
        number_args = count_args(argv, i);
        if (strcmp(argv[i], "cd") == 0)
        {
            if (number_args == 2)
            {
                if (chdir(argv[i + 1]))
                {
                    print_error("error: cd: cannot change directory to ");
                    print_error(argv[i + 1]);
                    print_error("\n");
                }
            }
            else
            {
                print_error("error: cd: bad arguments\n");
            }
            i += number_args;
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
        else if (pid > 0)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            print_error("error: fatal\n");
            exit (1);
        }
        i += number_args;
    }
    return (0);
}