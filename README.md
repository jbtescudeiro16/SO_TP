# SO_TP
Contém o material relativo ao projeto de Sistemas Operativos 

Este trabalho consiste na criação de um Pseudo-Servidor , em que de um lado temos o servidor aberto ,  do outro temos um cliente que faz os pedidos .

Primeiramente deve-se abrir um novo terminal e fazer make , na diretoria certa .

Para correr o servidor (Que deve ser feito primeiramente ) , basta abrir um terminal e fazer o seguinte comando " ./bin/sdstored config.txt files" 
Assim o servidor estará  "aberto" à espera dos pedidos do cliente .

Para executar os pedidos basta abrir um novo terminal e fazer o seguinte comando "./bin/sdstore (ficheiro entrada) (ficheiro saida) transformacao"
As transformacoes possíveis encontram-se na pasta files , que devem também estar previamente compiladas .
As transformções que são passíveis que serem utilizadas são :

-NOP
-BDECOMPRESS
-BCOMPRESS
-DECRYPT
-ENCRYPT
-GCOMPRESS
-GDECOMPRESS

O ficehiro config.txt terá a confirguração do servidor e o máximo de pedidos de cada tipo que o servidor suportará .

Após a execução de um pedido o programa avisa o cliente com quantos bytes ficou o ficheiro de output .

Podem haver vários pedidos a ser executados simultaneamente , que serão tratados por ordem de chegada .

Caso seja feito um sinal "KILL"  então o programa deixa de receber pedidos terminando de executar os que se encontram na queue .
