#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <sys/shm.h>
#include <signal.h>
#include "common.h"

#define CLIENT_TO_SERVER_FIFO "banco_cliente_servidor.fifo"
#define SERVER_TO_CLIENT_FIFO "banco_servidor_cliente.fifo"
#define SHM_SIZE 1024
#define NUM_ACCOUNTS 10


typedef struct {
    int n_conta;
    double saldo;
} Conta;

struct ThreadArgs {
    int client_to_server_fd;
    int server_to_client_fd;
    Pedido pedido;
    Conta *shmaddr;
};


int estadoBanco = 1;
pthread_mutex_t mutex;

void sigusr1_handler(int sinal) {
    printf("SIGUSR1 (%d) - BANCO FECHADO\n", sinal);
    //Fechado
    estadoBanco = 0;
    
}

void sigusr2_handler(int sinal) {
    printf("SIGUSR2 (%d) - BANCO ABERTO\n", sinal);
    //Aberto
    estadoBanco = 1;
}

void *handle_client(void *args) {
    
    //const char *nomesPedidos[] = {"DEPOSITO", "TRANSFERENCIA", "LEVANTAMENTO", "CONSULTA"};
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    Resposta resposta;
    
    resposta.pedido = threadArgs->pedido;

    resposta.status = 0;
    
    if(threadArgs->pedido.tipo == 3) {
        if(estadoBanco == 1)
            resposta.status = 0;
        else
        resposta.status = 2;
    } else {
        
        if(estadoBanco == 1) {
            
            //Escritas
            pthread_mutex_lock(&mutex);
            switch (threadArgs->pedido.tipo) {
                case 0:
                    //DEPOSITO;
                    threadArgs->shmaddr[threadArgs->pedido.numero_conta].saldo += threadArgs->pedido.montante;
                    resposta.status = 0;
                    usleep(500000);
                    break;
                case 1:
                    //TRANSFERENCIA;
                    if(threadArgs->shmaddr[threadArgs->pedido.numero_conta].saldo >= threadArgs->pedido.montante){
                        threadArgs->shmaddr[threadArgs->pedido.numero_conta].saldo -= threadArgs->pedido.montante;
                    }else{
                        resposta.status = 1;
                        break;
                    }
                    
                    threadArgs->shmaddr[threadArgs->pedido.conta_destino].saldo += threadArgs->pedido.montante;
                    resposta.status = 0;
                    usleep(500000);
                    break;
                case 2:
                    //LEVANTAMENTO;
                    if(threadArgs->shmaddr[threadArgs->pedido.numero_conta].saldo >= threadArgs->pedido.montante){
                        threadArgs->shmaddr[threadArgs->pedido.numero_conta].saldo -= threadArgs->pedido.montante;
                        resposta.status = 0;
                        
                    }else{
                        resposta.status = 1;
                    }
                    usleep(500000);
                    break;
                case 3:
                    break;
            }
            pthread_mutex_unlock(&mutex);
            
        }else{
            resposta.status = 2;
        }
    }
    
    resposta.saldo = threadArgs->shmaddr[threadArgs->pedido.numero_conta].saldo;
    
    write(threadArgs->server_to_client_fd, &resposta, sizeof(Resposta));

    pthread_exit(NULL);
}


void *handle_atender_pedido(void *args) {
    
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    
    pthread_t tid[10];
    int t_i = 0;
    
    Pedido pedido;
    
    while (1) {
        
        char buffer[SHM_SIZE];
        int valread;
        
        valread = read(threadArgs->client_to_server_fd, &pedido, sizeof(Pedido));
        if (valread <= 0) {
            if (valread == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("read");
            }
            
            break;
        }
        
        struct ThreadArgs *argsP = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        argsP->client_to_server_fd = threadArgs->client_to_server_fd;
        argsP->server_to_client_fd = threadArgs->server_to_client_fd;
        argsP->pedido = pedido;
        argsP->shmaddr = threadArgs->shmaddr; 
        
        pthread_t tid[t_i];
        if (pthread_create(&tid[t_i], NULL, handle_client, (void *)argsP) != 0) {
            perror("Error creating thread");
            close(threadArgs->client_to_server_fd);
            free(args);
            break;
        }
        t_i++;
    }
    free(args); 
    pthread_exit(NULL);
}


int main() {
    
    
    if (pthread_mutex_init(&mutex, NULL) != 0) {
          fprintf(stderr, "Erro ao inicializar o mutex.\n");
          exit(EXIT_FAILURE);
      }

    
    
    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
           perror("Erro ao configurar o manipulador para SIGUSR1");
           exit(EXIT_FAILURE);
       }
    
    if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR) {
           perror("Erro ao configurar o manipulador para SIGUSR1");
           exit(EXIT_FAILURE);
       }
    
    
    key_t key = ftok("banco_22205060.shm", 65);

    
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    Conta *shmaddr = (Conta *)shmat(shmid, NULL, 0);

    if ((void *)shmaddr == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    shmaddr = (Conta *)calloc(NUM_ACCOUNTS, sizeof(Conta));

    if (shmaddr == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    for(i = 0; i < 10; i++){
        shmaddr[i].n_conta = i;
        shmaddr[i].saldo = 250;
    }
    
   
    if (access(CLIENT_TO_SERVER_FIFO, F_OK) == -1 && access(SERVER_TO_CLIENT_FIFO, F_OK) == -1) {
            
            if (mkfifo(CLIENT_TO_SERVER_FIFO, 0666) == -1 || mkfifo(SERVER_TO_CLIENT_FIFO, 0666) == -1) {
                perror("Error creating FIFOs");
                exit(EXIT_FAILURE);
            } else {
                printf("FIFOs created successfully.\n");
            }
        } else {
            printf("FIFOs already exist.\n");
        }
        
    int client_to_server_fd = open(CLIENT_TO_SERVER_FIFO, O_RDONLY);
    int server_to_client_fd = open(SERVER_TO_CLIENT_FIFO, O_WRONLY);

    if (client_to_server_fd == -1 || server_to_client_fd == -1) {
        perror("Error opening FIFOs");
        exit(EXIT_FAILURE);
    }
    
    printf("%d\n", getpid());
    printf("Server started...\n");

    pthread_t t_principal_id;
    
    struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    args->client_to_server_fd = client_to_server_fd;
    args->server_to_client_fd = server_to_client_fd;
    args->shmaddr = shmaddr; 

    if (pthread_create(&t_principal_id, NULL, handle_atender_pedido, (void *)args) != 0) {
        perror("Error creating thread");
        close(client_to_server_fd);
        free(args);
    }
    
    pthread_join(t_principal_id, NULL);
    
    float saldo = 0.0;
    for(i = 0; i < 10; i++){
        saldo = saldo + shmaddr[i].saldo;
    }
    
    printf("SALDO DO BANDO: %f", saldo);
        
    close(client_to_server_fd);
    close(server_to_client_fd);

    unlink(CLIENT_TO_SERVER_FIFO);
    unlink(SERVER_TO_CLIENT_FIFO);

    return 0;
}
