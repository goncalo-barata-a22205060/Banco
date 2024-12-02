#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include "common.h"

pthread_mutex_t mutex;

struct ThreadArgs {
    int server_to_client_fd;
    int numero_cliente;
};

int contarArgumentos(const char *str) {
    char copiaStr[strlen(str) + 1];
    strcpy(copiaStr, str);

    // Conta o numero de argumentos usando strtok
    int contador = 0;
    char *token = strtok(copiaStr, " ");
    while (token != NULL) {
        contador++;
        token = strtok(NULL, " ");
    }

    return contador;
}

void printPedido(Pedido p){
    const char *nomesPedidos[] = {"DEPOSITO", "TRANSFERENCIA", "LEVANTAMENTO", "CONSULTA"};
    printf("\n\n### PEDIDO ###\n");
    printf("Tipo: %s\n", nomesPedidos[p.tipo]);
    printf("NÃºmero da Conta: %d\n", p.numero_conta);
    printf("Montante: %.2f\n", p.montante);
    printf("Conta Destino: %d\n", p.conta_destino);
    printf("Cliente ID: %d\n", p.numero_cliente);
    printf("###################\n");
}

void printResposta(Resposta r){
    const char *nomesPedidos[] = {"STATUS_OK", "STATUS_SALDO_INSUFICIENTE", "STATUS_FECHADO", "STATUS_CONTAS_IGUAIS"};

    printPedido(r.pedido);

    printf("\n\n### RESPOSTA ###\n");
    printf("Status: %s\n", nomesPedidos[r.status]);
    printf("Saldo: %lf\n", r.saldo);
    printf("\n");
}


void *handle_client(void *args) {
    
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    Resposta resposta;
    
    while (1) {
        char buffer[SHM_SIZE];
        int valread;
        
        valread = read(threadArgs->server_to_client_fd, &resposta, sizeof(Resposta));
        if (valread <= 0) {
            if (valread == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("read");
            }
            
            break;
        }
        
        printResposta(resposta);
 
        pthread_mutex_unlock(&mutex);
        
    }
    
    free(args); 
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <client_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if (pthread_mutex_init(&mutex, NULL) != 0) {
          fprintf(stderr, "Erro ao inicializar o mutex.\n");
          exit(EXIT_FAILURE);
      }

    int numero_cliente;
    numero_cliente = atoi(argv[1]);

    int client_to_server_fd = open(CLIENT_TO_SERVER_FIFO, O_WRONLY);
    int server_to_client_fd = open(SERVER_TO_CLIENT_FIFO, O_RDONLY);

    if (client_to_server_fd == -1 || server_to_client_fd == -1) {
        perror("Error opening FIFOs");
        exit(EXIT_FAILURE);
    }
    
    //Criar pthread responsavel pelas respostas
    struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    args->server_to_client_fd = server_to_client_fd;
    args->numero_cliente = numero_cliente;

    pthread_t tid;
    if (pthread_create(&tid, NULL, handle_client, (void *)args) != 0) {
        perror("Error creating thread");
        close(client_to_server_fd);
        free(args);
    }
    
    
    
    Pedido pedido;

    char buffer[SHM_SIZE];
    while (1) {
        
        pthread_mutex_lock(&mutex);
        
        printf("Por favor insira o seu pedido: ");
        fgets(buffer, SHM_SIZE, stdin);;
        
        char operacao = '\0';
        
        int numArgs = contarArgumentos(buffer);
        
        if(numArgs == 3){
            sscanf(buffer, "%c %d %lf", &operacao, &pedido.numero_conta, &pedido.montante);
        } else if(numArgs == 2) {
            sscanf(buffer, "%c %d", &operacao, &pedido.numero_conta);
        }else if(numArgs == 4) {
            sscanf(buffer, "%c %d %d %lf", &operacao, &pedido.numero_conta,&pedido.conta_destino, &pedido.montante);
        }else {
            sscanf(buffer, "%c", &operacao);
        }
        
        switch (operacao) {
            case 'D':
                pedido.tipo = DEPOSITO;
                break;
            case 'T':
                pedido.tipo = TRANSFERENCIA;
                break;
            case 'L':
                pedido.tipo = LEVANTAMENTO;
                break;
            case 'C':
                pedido.tipo = CONSULTA;
                break;
            case 'X':
                exit(0);
                break;
        }
        pedido.numero_cliente = numero_cliente;
                
        
        if(pedido.numero_conta >=0 &&  pedido.numero_conta <= NUM_ACCOUNTS){
            if (write(client_to_server_fd, &pedido, sizeof(Pedido)) == -1) {
                perror("write");
                break;
            }
        }else{
            printf("Conta Invalida!\n");
            pthread_mutex_unlock(&mutex);
        }

        if (strcmp(buffer, "sair\n") == 0) {
            printf("Closing connection. Goodbye!\n");
            break;
        }
    }

    close(client_to_server_fd);
    close(server_to_client_fd);

    return 0;
}
