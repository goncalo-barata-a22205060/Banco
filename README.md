#Banco Simples

##Descrição do Projeto
Este projeto é uma simulação de um sistema bancário que utiliza comunicação entre processos em C, que tem vários clientes e operações básicas. O sistema é composto por dois programas principais: servidor e cliente, que comunicam entre si através de fifos e partilham informações através de memória partilhada.
O servidor gere as contas e processa os pedidos dos clientes. Os clientes podem enviar pedidos como depósitos, transferências, levantamentos e consultas de saldo.
Além disso, o servidor pode responder a sinais externos para fechar ou abrir o banco, permitindo que seja controlado dinamicamente.
