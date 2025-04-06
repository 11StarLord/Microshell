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

void    execute_pipeline(char **argv, int start, int end, char **env, int *prev_pipe)
{
    int pipe_fd[2]; //Array para armazenar os descritores do pipe (leitura em pipe_fd[0], escrita em pipe_fd[1])

    if (argv[end] && strcmp(argv[end], "|") == 0) //Se o próximo token é |, cria um pipe com pipe(pipe_fd). Se falhar, imprime erro e sai
    {
        if (pipe(pipe_fd) == -1)
        {
            print_error("error : fatal\n");
            exit(1);
        }
    }
    pid_t   pid = fork(); //divide o processo em pai e filho. Se falhar (pid == -1), imprime erro e encerra.
    if (pid == -1)
    {
        print_error("error : fatal\n");
        exit(1);
    }
    if (pid == 0)
    {
        if (prev_pipe[0] != -1) //Se houver um pipe anterior
        {
            dup2(prev_pipe[0], STDIN_FILENO); //Conecta a saída do comando anterior à entrada do atual.
            close(prev_pipe[0]); //Fecha os descritores do pipe anterior para evitar vazamentos
            close(prev_pipe[1]);
        }
        if (argv[end] && strcmp(argv[end], "|") == 0) //Se houver um pipe após o comando (argv[end] == "|"
        {
            dup2(pipe_fd[1], STDOUT_FILENO); //Conecta a saída do comando atual ao próximo.
            close(pipe_fd[0]); //Fecha os descritores do pipe anterior para evitar vazamentos
            close(pipe_fd[1]);
        }
        argv[end] = NULL; //Marca o fim do comando atual em argv (exigido por execve)
        execve(argv[start], &argv[start], env); //Substitui o processo filho pelo comando
        print_error("error : cannot execute "); //Se execve falhar, imprime erro e encerra o filho
        print_error(argv[start]);
        print_error("\n");
        exit(1);
    }
    else
    {
        if (prev_pipe[0] != -1) //Se houver um pipe anterior
        {
            close(prev_pipe[0]); //Fecha descritores do pipe anterior (se existirem)
            close(prev_pipe[1]);
        }
        if (argv[end] && strcmp(argv[end], "|") == 0) //Se houver um próximo comando (|), armazena os descritores do novo pipe em prev_pipe
        {
            prev_pipe[0] = pipe_fd[0];
            prev_pipe[1] = pipe_fd[1];
        }
        else
        {
            waitpid(pid, NULL, 0); //Caso contrário, espera o filho terminar (waitpid)
        }
    }
}

int main(int argc, char **argv, char **env)
{
    int i;
    int start;

    if (argc == 1)
        return (0);
    int prev_pipe[2] = {-1 , -1};
    i = 1;
    while (argv[i])
    {
        if (strcmp(argv[i], ";") == 0 || strcmp(argv[i], "|") == 0)
        {
            i++;
            continue ;
        }
        start = i;
        while (argv[i] && strcmp(argv[i], ";") != 0 && strcmp(argv[i], "|") != 0)
            i++;
        if (strcmp(argv[i], "cd") == 0)
        {
            if ( i - start == 2)
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
        }
        else
        {
            execute_pipeline(argv, start, i, env, prev_pipe);
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
    }
    return (0);
}