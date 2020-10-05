# SO Project 2020-21
## Exercise 1 base code.

## Pseudocodigo/alteracoes a fazer

### alteracoes a funcao "main" para receber command line arguments
1. funcao "processInput" precisa de receber como argumento (do tipo FILE) o ficheiro de instrucoes (argv[1]);
2. alterar primeiro while da funcao processInput para ler do ficheiro passado como argumento em vez do stdin;
3. (em principio funcionara).

### alteracoes a funcao "print_tecnicofs_tree" para fazer escrever o output num ficheiro
1. funcao "print_tecnicofs_tree" necessita de ter como argumento (do tipo FILE) o ficheiro de output (argv[2]);
2. se o ficheiro ja existir, o seu conteudo deve ser eliminado e substituido pelo novo;
3. verificar se e necessario fazer print dos erros nesse ficheiro em vez do stderr
4. introduzir o output final: "TecnicoFS completed in [duration] seconds." no stdout

### alteracoes para o projeto funcionar multi-thread e sincronizado
Nao sabemos materia
