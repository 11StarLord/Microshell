#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int print_message(char *str, char *command)
{
    while (*str)
        write(2, str++, 1);
    if (command)
    {
        while (*command)
            write(2, command++, 1);
    }
    return (write(2, "\n", 1));
}

int function_cd(char **argv, int args)
{
    if (args != 2)
        return (print_message("error: cd: bad arguments", NULL));
    if (chdir(argv[1]) < 0)
        return (print_message("error: cd: cannot change directory to ", argv[1]));
    return (0);
}

void    function_pipe(bool has_pipe, int *fd, int end)
{
    if (has_pipe == true)
    {
        if (dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1)
            exit(print_message("error: fatal", NULL));
    }
}

int function_execute(char **argv, int i, char **env)
{
    int pid;
    int status;
    int fd[2];
    bool    has_pipe;

    has_pipe = (argv[i] && strcmp(argv[i], "|") == 0);
    if (has_pipe == false && strcmp(argv[0], "cd") == 0)
        return (function_cd(argv, i));
    if (has_pipe == true && pipe(fd) == -1)
        exit(print_message("error: fatal", NULL));
    pid = fork();
    if (pid == -1)
        exit(print_message("error: fatal", NULL));
    if (pid == 0)
    {
        argv[i] = NULL;
        function_pipe(has_pipe, fd, 1);
        execve(argv[0], argv, env);
        exit(print_message("error: cannot execute ", argv[0]));
    }
    waitpid(pid, &status, 0);
    function_pipe(has_pipe, fd, 0);
    return (WIFEXITED(status) && WEXITSTATUS(status));
}

int main(int argc, char **argv, char **env)
{
    int i;
    int status;

    i = 0;
    status = 0;
    if (argc < 2)
        return (0);
    while (argv[i])
    {  
        argv += i + 1;
        i = 0;
        while (argv[i] && strcmp(argv[i], "|") != 0 && strcmp(argv[i], ";") != 0)
            i++;
        if (i != 0)
            status = function_execute(argv, i, env);
    }
    return (status);
}