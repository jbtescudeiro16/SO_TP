#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

int close_pipe = -1;

void sigusr1_handler(int signum)
{
  close(close_pipe);
}

int main(int argc, char *argv[])
{

    char buffer[1024];
    int n_bytes = 0;
    for(int i = 1; i < argc; ++i)
    {
      // ciclo para guardar os argumentos da main num buffer
        n_bytes += snprintf(buffer + n_bytes, sizeof(buffer) - n_bytes, "%s ", argv[i]); 
    }

    signal(SIGUSR1, sigusr1_handler);

//pipe servidor->cliente para mandar as instruçoes para o cliente (executing , pendng)
    char fifo[100];
    snprintf(fifo, sizeof(fifo), "server_cliente%d", getpid());
    mkfifo(fifo, 0664);

    n_bytes += snprintf(buffer + n_bytes, sizeof(buffer) - n_bytes, "%s", fifo);

    

    //pipe cliente-> servidor  para o cliente mandar os pedidos para o servidor

    int write_pipe = open("client_server", O_WRONLY);
    write(write_pipe, buffer, n_bytes);
    close(write_pipe);


   //abrir para ler do servidor
    int pipe = open(fifo, O_RDONLY);
    close_pipe = open(fifo, O_WRONLY);
    n_bytes = 0;
    while((n_bytes = read(pipe, buffer, sizeof(buffer))) > 0)
    {
      write(1, buffer, n_bytes);
      buffer[n_bytes - 1] = 0;
    }
    return 0;
}
