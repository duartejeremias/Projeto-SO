# SO Project 2020-21
## Exercise 3 base code.

## How to run
Execute the following command:
```
./tecnicofs-client <inputfile> <server_socket_name>
```
## 1. Comunicacao entre processos clientes

### 1.1 Completar API do cliente

- Fazer com que o nome do socket do cliente seja do tipo "client_PID" em que PID e o process id do processo a correr o filho.

- De resto principio esta done.

### 1.2  Re-estruturar o tecnicofs (agora servidor)

- O tecnicofs (main.c) precisa de ter a tarefa principal a iniciar o servidor, sendo que as tarefas escravas vao receber comandos do cliente, tornando o process input original, variaveis de condicao, inputCommand e removeCommand obsoletos;

- O programa e suposto nao terminar, tornando obsoleto o gettimeofday e o shell script;

## 2. Novo commando P

- Estrutura do comando: "p outputFile" em que outputFile e o path do ficheiro para onde vai ser feito o print;

- Na API ja esta feito logo so falta implementar mesmo;

- E necessario que o print seja feito quando nao ha tarefas nenhumas a interagir com o fs. Sendo necessario esperar caso estejam la dentro tarefas;

- Se alguma tarefa tentar aceder ao fs enquanto o print esta a ser feito, tambem deve esperar.