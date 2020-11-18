# SO Project 2020-21

## Exercise 2 base code

## Pseudocodigo/alteracoes a fazer

### 1.Sincronizacao fina dos inodes

* O desempenho paralelo do exercicio 1 foi desapontante devido ao metodo de sincronizacao utilizado;

* Temos de implementar uma nova estrategia de sincronizacao que permita maior paralelismo completamente diferente da do primeiro exercicio, portanto temos de alterar o codigo de modo a que nao seja recebido o 4o argumento de linha de comandos (__synchstrategy__).

* Em vez de trinco global, agora devemos colocar um trinco por inode (temos de decidir entre rwlock ou mutex em cada caso);

* Basicamente, ao percorrer path de cada diretoria/ficheiro (cadeia de inodes) para executar os comandos, devemos fazer lock de acordo com os requisitos para assegurar sincronizacao;

* Apos execucao dos comandos, destranca-se os locks pela ordem contraria a de trancacao (nao sei se trancacao existe mas vai);

* Temos de assegurar que as operacoes nao alterem o estado do _TecnicoFS_ nem elementos comuns a este.

> Exemplo 1:
    op1: c /a/b/c f
    op2: c /a/d/f f
    Estas operacoes alteram o estado das diretorias /a/b e /a/d respetivamente enquanto que apenas leem (nao mudam) as diretorias / (root) e /a, logo podem e devem executar-se concorrentemente. Se a op1 fosse: c/a/c f, ja nao seria possivel correr concorrentemente a op2
> Exemplo 2:
    op1: l /
    op2: c /a d
    Estas operacoes nao podem ser executadas concorrentemente pois a op2 esta a criar uma diretoria (inode) no mesmo path da op1 (/ ou root)

### 2.Execucao incremental de comandos

* Devemos alterar o metodo de carregamento de comandos para o programa;

* Devera existir uma tarefa (produtora) encarregue de carregar o vetor (buffer) de comandos ao mesmo tempo que as outras tarefas (consumidoras), definidas pelo __numthreads__ do terminal, vao executando-as;

* Assim que uma tarefa esteja livre devera ir buscar o proximo comando do buffer;

* Se o buffer ficar cheio de comandos, a tarefa encarregue de os carregar devera esperar;

* Se uma tarefa consumidora for buscar um comando para executar e o buffer estiver vazio, devera esperar ate ao proximo comando for carregado;

* O buffer de comandos devera passar a ter 10 entradas e deve ser tratado como um vetor circular;

* A tarefa produtora devera carregar ficheiros ate ao EOF ser alcancado;

* Devemos alterar o tempo de execucao, este devera comecar antes do carregamento de comandos comecar/.

### <s> 3.Nova op: mover ficheiro ou diretoria </s>

* Devemos implementar uma nova op (m);

* Esta op recebe dois argumentos: o pathname atual de uma diretoria/ficheiro e o novo pathname;

* Esta op retira a entrada (__dirEntry__) com o pathname atual e insere uma nova entrada com o novo pathname;

* O inumber na nova entrada deve manter-se igual ao da entrada original;

* Tem de ser verificadas duas condicoes no momento da invocacao:
  * tem existir um ficheiro/diretoria com o pathname atual;
  * nao pode existir ficheiro/diretoria com o novo pathname;

* Caso uma ou mais das condicoes nao se verifiquem, a op deve ser cancelada, nao tendo efeitos em relacao as outras tarefas que possam estar a ser executadas concorrentemente;

* Devemos prevenir situacoes de interblocagem ou mingua.

> Exemplo:
    op1: m /dA/f1 /dB/f2
    Esta op nao pode ser executada se nao existir o f1 na diretoria A;
    Esta op nao pode ser executada se o f2 da diretoria B ja existir;
    Caso contrario pode-se executar.

### 4.Nova op: Shell script

* Devemos criar um shell script chamado __runTests.sh__;

* Este script devera avaliar o desempenho do TecnicoFS quando executado com diferentes argumentos e ficheiros de entrada;

* O script devera receber os seguintes tres argumentos:
  * inputdir;
  * outputdir;
  * maxthreads;

* O script devera usar os diferentes ficheiros de teste contidos em __inputdir__ com um numero random de threads entre 1 e __maxthreads__;

* Antes de cada teste, o script devera imprimir: "InputFile=__nomeDoFicheiro__ NumThreads=__numeroDeTarefas__";

* Apos cada teste, o script devera apenas imprimir: "TecnicoFS completed in __duracao__ seconds";

* Apos cada teste, devera ser criado na __outputdir__ um ficheiro contendo o estado final do FS. O nome do ficheiro devera ser "__nomeFicheiroEntrada__-__numeroDeTarefas__.txt"
