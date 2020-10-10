# SO Project 2020-21
## Exercise 1 base code.

## Pseudocodigo/alteracoes a fazer

### <s> alteracoes a funcao "main" para receber command line arguments </s> DONE
1. funcao "processInput" precisa de receber como argumento (do tipo FILE) o ficheiro de instrucoes (argv[1]);
2. alterar primeiro while da funcao processInput para ler do ficheiro passado como argumento em vez do stdin;
3. (em principio funcionara).

### <s> alteracoes a funcao "print_tecnicofs_tree" para fazer escrever o output num ficheiro </s> DONE
1. funcao "print_tecnicofs_tree" necessita de ter como argumento (do tipo FILE) o ficheiro de output (argv[2]);
2. se o ficheiro ja existir, o seu conteudo deve ser eliminado e substituido pelo novo;
3. verificar se e necessario fazer print dos erros nesse ficheiro em vez do stderr
4. introduzir o output final: "TecnicoFS completed in [duration] seconds." no stdout

### <s> aprender a usar o gettimeofday </s>
1. aprender gettimeofday para produzir o output final.

### alteracoes para o projeto funcionar multi-thread e sincronizado
1. Criar pool de threads para alimentar com comandos e funcoes
2. Criar funcoes de sincronizacao com mutex
