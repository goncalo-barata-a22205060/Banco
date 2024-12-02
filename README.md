# Banco Simples

## Descrição do Projeto
Este projeto é uma simulação de um sistema bancário que utiliza comunicação entre processos em C, que tem vários clientes e operações básicas. O sistema é composto por dois programas principais: servidor e cliente, que comunicam entre si através de fifos e partilham informações através de memória partilhada.
O servidor gere as contas e processa os pedidos dos clientes. Os clientes podem enviar pedidos como depósitos, transferências, levantamentos e consultas de saldo.
Além disso, o servidor pode responder a sinais externos para fechar ou abrir o banco, permitindo que seja controlado dinamicamente.

## Funcionalidades
No Servidor:
- Manutenção das Contas: Cada conta tem um saldo inicial e pode ser manipulada com operações de depósito, levantamento e transferência.
- Controlo de Estado do banco: O servidor pode mudar entre os estados Aberto e Fechado utilizando os sinais SIGUSR1 (Fechar) e SIGUSR2 (Abrir).
- Segurança com o Mutex: Operações sensíveis nas contas são protegidas para evitar condições de corrida quando tem vários clientes.
- Multithreading: O servidor lida com várias clientes simultaneamente, criando uma thread para cada pedido.
  
No Cliente:
- Pedidos: Os clientes enviam pedidos para o servidor no formato de operações bancárias.
- Respostas: O cliente lê as respostas do servidor, incluindo o status do pedido e saldo atualizado.
- Interface Simples: A interação com o cliente é feita pelo terminal.

## Como implementar
1. Clone o Repositório
Clone este projeto para o seu computador:
```
git clone <URL_DO_REPOSITORIO>
cd <NOME_DO_REPOSITORIO>
```
2. Compile os Arquivos
Utilize o gcc para compilar os arquivos servidor.c e cliente.c:
```
gcc servidor.c -o servidor -lpthread
gcc cliente.c -o cliente -lpthread
```
3. Inicie o Servidor
Execute o servidor, que criará as fifos e inicializará o sistema:
```
./servidor
```
4. Inicie os Clientes
Em terminais separados, execute os clientes com um número único para cada um:
```
./cliente <NUMERO_CLIENTE>
```
5. Faca Operações
No terminal do cliente, envie as operações na seguinte formato:
- Depósito: D <NUMERO_CONTA> <MONTANTE>
- Levantamento: L <NUMERO_CONTA> <MONTANTE>
- Transferência: T <NUMERO_CONTA_ORIGEM> <NUMERO_CONTA_DESTINO> <MONTANTE>
- Consulta: C <NUMERO_CONTA>
- Sair: X

6. Fechar ou Abrir o Banco
Use os seguintes comandos para controlar o estado do banco:
    - Fechar
    ```
    kill -SIGUSR1 <PID_DO_SERVIDOR>
    ```
    - Abrir
    ```
    kill -SIGUSR2 <PID_DO_SERVIDOR>
    ``` 

   
