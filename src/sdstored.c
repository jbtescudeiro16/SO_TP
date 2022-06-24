#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

struct queue *fila = NULL;

int sigterm_received = 0;

void sigterm_handler(int signum)
{
  puts("Received");
  sigterm_received = 1;
}


pid_t pedidos[1024];
struct config *pedidos_struct[1024];
int last_process;

struct config
{
  char *idTransf[7];
  int limit[7];
  int current[7];
};

char *directory = NULL;

struct config configuracao;

struct queue
{
  struct config *pedido;
  char **request;
  int n_request;
  struct queue *next;
};

int pos = 0;
int final = 0;
char buffer[1024];


//funçao que lê caractere a caractere 

int readc(int fd, char *line)
{
    int n = 0;
    if(final == 0 || pos == final)
    {
        if((n = read(fd, buffer, sizeof(buffer))) < 0)
        {
            perror("Error reading from file");
            exit(1);
        }
        if(n == 0) return 0;
        pos = 0;
        final = n;
    }

    *line = buffer[pos++];
    return 1;
}


//funcao que lê linha a linha,chamando a readc

ssize_t readln(int fd, char *line, size_t size){
    int i = 0;
    while(i < size && readc(fd, &line[i]) > 0 && line[i++] != '\n');
    return i;
}




void push(char **pedidos, int N)
{
  struct config *pedido = (struct config *)malloc(sizeof(struct config));

  for(int i = 0; i < 7; ++i)
  {
    pedido->idTransf[i] = strdup(configuracao.idTransf[i]);
    pedido->current[i] = 0;
  }

  for(int i = 2; i < N - 1; ++i)
  {
    for(int j = 0; j < 7; ++j)
    {
      if(strcmp(pedidos[i], pedido->idTransf[j]) == 0)
      {
        pedido->current[j]++;
        break;
      }
    }

    int fd = open(pedidos[N - 1], O_WRONLY);
    write(fd, "pending\n", sizeof("pending\n"));
    close(fd);
  }


  struct queue *new = (struct queue *)malloc(sizeof(struct queue));

  struct queue **queue = &fila;

  while(*queue){
    queue = &((*queue)->next);
  }
  new->next = NULL;
  new->pedido = pedido;
  new->n_request = N;
  new->request = pedidos;
  *queue = new;
}

void execute()
{
  struct queue *cabeca = fila;

  int pode_executar = 1;
  for(int i = 0; i < 7 && pode_executar; ++i){
    if(cabeca->pedido->current[i] + configuracao.current[i] > configuracao.limit[i])
    {
      pode_executar = 0;
    }
  }

  if(pode_executar)
  {

    for(int i = 0; i < 7; ++i)
    {
      configuracao.current[i] += cabeca->pedido->current[i];
    }

    fila = fila->next;

    pid_t ultimo = -1;
    pid_t pedido = -1;
    if (!(pedido = fork())) 
    {
        int client_pipe = open(cabeca->request[cabeca->n_request - 1], O_WRONLY);
        write(client_pipe, "executing\n", sizeof("executing\n"));
        
        if (cabeca->n_request == 4) 
        {

            if (!(ultimo = fork())) 
            {
              char transform[100];
                snprintf(transform, sizeof(transform), "%s/%s", directory,
                         cabeca->request[2]);

                //debug
                //puts(transform);
                
                int fd1 = open(cabeca->request[0], O_RDONLY);
                 if (fd1<0) 
                 {
                   perror("ERROR opening file1");
                 }
                dup2(fd1, 0);
                close(fd1);
                int fd2 = open(cabeca->request[1], O_WRONLY | O_CREAT | O_TRUNC,0664);

                    if (fd2<0) {
                   perror("ERROR opening file2");
                 }
                dup2(fd2, 1);
                close(fd2);
                
                execlp(transform, transform, NULL);

                perror("ERROR Executing");
                _exit(1);
            }
        } 
        else 
        {
            int pd[cabeca->n_request - 2][2];
            int current_pipe = 0;


            for (int i = 2; i < cabeca->n_request - 1; ++i) {
                if (i == 2) {
                    pipe(pd[current_pipe]);
                    if (!fork()) {
                        close(pd[current_pipe][0]);
                        int fd = open(cabeca->request[0], O_RDONLY);
                        if (fd < 0) {
                            perror("Error opening file");
                            _exit(1);
                        }

                        dup2(fd, 0);
                        close(fd);
                        dup2(pd[current_pipe][1], 1);
                        char transform[100];
                        snprintf(transform, sizeof(transform), "%s/%s",
                                 directory, cabeca->request[i]);
                        execlp(transform, transform, NULL);
                    } else {
                        close(pd[current_pipe][1]);
                        current_pipe++;
                    }
                } 
                else if (i == cabeca->n_request - 2) {
                    if (!(ultimo = fork())) {
                        int fd = open(cabeca->request[1],
                                      O_WRONLY | O_CREAT | O_TRUNC,0664);
                        if (fd < 0) {
                            perror("Error opening file");
                            _exit(1);
                        }
                        dup2(fd, 1);
                        close(fd);
                        dup2(pd[current_pipe - 1][0], 0);
                        close(pd[current_pipe - 1][0]);
                        char transform[100];
                        snprintf(transform, sizeof(transform), "%s/%s",
                                 directory, cabeca->request[i]);
                        execlp(transform, transform, NULL);
                    } else {
                        close(pd[current_pipe - 1][0]);
                    }
                } 
                else {
                    pipe(pd[current_pipe]);
                    if (!fork()) {
                        close(pd[current_pipe][0]);
                        dup2(pd[current_pipe - 1][0], 0);
                        close(pd[current_pipe - 1][0]);
                        dup2(pd[current_pipe][1], 1);
                        char transform[100];
                        snprintf(transform, sizeof(transform), "%s/%s",
                                 directory, cabeca->request[i]);
                        execlp(transform, transform, NULL);
                    } else {
                        close(pd[current_pipe][1]);
                        close(pd[current_pipe - 1][0]);
                        current_pipe++;
                    }
                }
            }
        }
        waitpid(ultimo, NULL, 0);

        int input = open(cabeca->request[0], O_RDONLY);
        int bytes_input = lseek(input, 0, SEEK_END);
        close(input);
        int output = open(cabeca->request[1], O_RDONLY);
        int bytes_output = lseek(output, 0, SEEK_END);
        close(output);

        char bytes_answer[100];
        int bytes_size = snprintf(bytes_answer, sizeof(bytes_answer), "concluded (bytes-input: %d, bytes-output: %d)", bytes_input, bytes_output);

        write(client_pipe, bytes_answer, bytes_size);
        close(client_pipe);

        int id_cliente;
        sscanf(cabeca->request[cabeca->n_request - 1], "server_cliente%d", &id_cliente);
        kill(id_cliente, SIGUSR1);

        int fd = open("client_server", O_WRONLY);
        char answer[100];
        int bytes = snprintf(answer, sizeof(answer), "terminou %d", getpid());
        write(fd, answer, bytes);
        close(fd);
        _exit(0);
    }
        pedidos[last_process] = pedido;
        pedidos_struct[last_process++] = cabeca->pedido;

  }

}



int main(int argc, char *argv[])
{

    if(argc < 3)
    {
      perror("Wrong arguments");
      return 1;
    }

    signal(SIGTERM, sigterm_handler);

    sigterm_received = 0;

    fila = NULL;
    directory = argv[2];

    int config_file = open(argv[1], O_RDONLY);
    if(config_file < 0)
    {
      perror("Error opening config file");
      return 1;
    }
    int n_bytes = 0;
    char line[1024];
    int i = 0;

    //parse das linhas do ficheiro das configs
    while((n_bytes = readln(config_file, line, sizeof(line))))
    {
      char *t = strtok(line, " ");

      //nr maximo de transformaçoes daquele tipo
      int limit = atoi(strtok(NULL, " "));

      //guarda a respetiva transformaçao
      configuracao.idTransf[i] = strdup(t);

      configuracao.current[i] = 0;

      configuracao.limit[i++] = limit;
    }

    //cria um pipe com nome "client_server"
    mkfifo("client_server", 0664);

    int read_pipe = open("client_server", O_RDONLY);

    char buffer[1024];
    n_bytes = 0;

    while(1)
    {
      if(sigterm_received && fila == NULL)
      {
        while(wait(NULL) != -1);
        return 0;
      }

        n_bytes = read(read_pipe, buffer, sizeof(buffer));
        if(n_bytes > 0)
        {
            buffer[n_bytes] = 0;
            char **args = (char **)malloc(sizeof(char *) * 40);
            int i = 0;
            for (char *token = strtok(buffer, " "); token != NULL;token = strtok(NULL, " ")) 
                {
                args[i++] = strdup(token);
                }
          

            if(!strcmp("terminou", args[0]))
            {
                int id = atoi(args[1]);
                waitpid(id, NULL, 0);
                for(int i = 0; i < last_process; ++i)
                {
                  if(pedidos[i] == id)
                  {
                    for(int j = 0; j < 7; j++)
                    {
                        configuracao.current[j] -= pedidos_struct[i]->current[j];
                    }
                  }
                }
            }
          
            else if (!sigterm_received)
            {
                int n_transf = i;
                push(args, n_transf);
                if (fila != NULL) execute();
            }
            else if(sigterm_received)
            {
              if(fila != NULL) execute();
            }
        }
        else{
          if(fila != NULL) 
          {
            execute();
          }
        }
      
    }

    return 0;
}
