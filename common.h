#ifndef COMMON_H
#define COMMON_H

#define CLIENT_TO_SERVER_FIFO "banco_cliente_servidor.fifo"
#define SERVER_TO_CLIENT_FIFO "banco_servidor_cliente.fifo"
#define USER_LEVEL_PERMISSIONS 0666
#define NUM_ACCOUNTS 10
#define SHM_SIZE 1024

#define TEM_ERRO -1

typedef enum {
    DEPOSITO, TRANSFERENCIA, LEVANTAMENTO, CONSULTA
} TipoPedido;

typedef struct {
    TipoPedido tipo;
    int numero_conta;
    double montante;
    int conta_destino;
    int numero_cliente;
} Pedido;

typedef enum {
    STATUS_OK, STATUS_SALDO_INSUFICIENTE, STATUS_FECHADO, STATUS_CONTAS_IGUAIS
} StatusResposta;

typedef struct {
    StatusResposta status;
    Pedido pedido;
    double saldo; 
} Resposta;


#endif // COMMON_H
