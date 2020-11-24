# SO Project 2020-21

## Exercise 3 base code

## How to run client

Execute the following command:

```sh
./tecnicofs-client <inputfile> <server_socket_name>
```

## 1. Comunicacao entre processos clientes

### 1.1 Completar API do cliente

- Em principio esta done.

### 1.2  Re-estruturar o tecnicofs (agora servidor)

- Em principio esta done.

## 2. Novo commando P

- Estrutura do comando: "p outputFile" em que outputFile e o path do ficheiro para onde vai ser feito o print;

- Na API ja esta feito logo so falta implementar mesmo;

- E necessario que o print seja feito quando nao ha tarefas nenhumas a interagir com o fs. Sendo necessario esperar caso estejam la dentro tarefas;

- Se alguma tarefa tentar aceder ao fs enquanto o print esta a ser feito, tambem deve esperar.
