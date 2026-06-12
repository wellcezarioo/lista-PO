# Instâncias de teste

Entradas válidas para os programas que leem dados da entrada padrão.
Os demais modelos (racao, dieta, plantio, tintas, transporte, fluxo,
padroes, exemplo1) têm os dados do enunciado fixos no código.

Uso:

```
./escalonamento < instances/escalonamento.txt
./cobertura     < instances/cobertura.txt
./mochila       < instances/mochila.txt
./clique        < instances/clique.txt
./facilidades   < instances/facilidades.txt
./frequencia    < instances/frequencia.txt
```

Resultados ótimos esperados (verificados por enumeração exaustiva):

| Instância        | Descrição                                   | Ótimo |
|------------------|---------------------------------------------|-------|
| escalonamento.txt| demandas 3 4 5 4 3 2 2 (4 dias on / 3 off)  | 6 enfermeiras |
| cobertura.txt    | 6 bairros em "caminho"                      | 2 escolas (bairros 2 e 5) |
| mochila.txt      | 5 itens, W = 10                             | valor 21 (itens 1, 2 e 5) |
| clique.txt       | 6 vértices                                  | clique de tamanho 4 (1,2,3,4) |
| facilidades.txt  | 3 depósitos, 4 clientes                     | custo 26 (depósitos 1 e 3) |
| frequencia.txt   | 5 antenas                                   | 3 frequências |
