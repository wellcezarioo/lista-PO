# Lista de Modelagem: Otimização Linear (Inteira)

Implementações em C++ (CPLEX / Concert Technology) dos problemas da lista de
modelagem do curso (slides OptLab/UFAL, "Modelagem com Otimização Linear (Inteira)").
São 14 modelos: 6 de programação linear contínua e 8 de programação linear inteira.

## Compilação e execução

```
make            # compila tudo
make mochila    # compila um alvo específico
make clean      # remove os binários
```

Modelos com dados fixos no código (do enunciado): `exemplo1`, `racao`, `dieta`,
`plantio`, `tintas`, `transporte`, `fluxo`, `padroes`. Basta executar, ex.: `./racao`.

Modelos com entrada via stdin: `escalonamento`, `cobertura`, `mochila`, `clique`,
`facilidades`, `frequencia`. Use as instâncias prontas:

```
./mochila < instances/mochila.txt
```

A pasta `instances/` tem uma entrada válida para cada um, com o ótimo esperado
documentado em `instances/README.md` (todos verificados por enumeração exaustiva).

### Escolhas no Makefile

- Regra-padrão `%: %.cpp` no lugar de uma regra por programa. Isso elimina 14 regras idênticas; para adicionar um modelo novo basta incluí-lo em `PROGS`.
- `CPLEXDIR ?=` (atribuição condicional) permite compilar em outra máquina sem editar o arquivo: `make CPLEXDIR=/outro/caminho/cplex`.
- `-O2 -Wall -std=c++17` no lugar de `-O`, para ter otimização e avisos do compilador; fixar o padrão da linguagem evita variação entre versões do g++.
- `.PHONY: all clean` evita conflito caso exista um arquivo chamado `all` ou `clean`.
- `.gitignore` cobre os binários gerados (mesmo nome dos alvos, sem extensão).

## Convenções comuns a todos os códigos

- Padrão Concert: `IloEnv`, `try/catch(IloException)` e `env.end()` ao final (o `env.end()` libera toda a memória do ambiente).
- `cplex.setOut(env.getNullStream())` para suprimir o log do solver.
- Leitura de soluções com tolerância: `> 1e-5` para variáveis contínuas e `> 0.5` para binárias. Isso evita erro de ponto flutuante, já que o CPLEX pode devolver 0.9999999 para uma binária igual a 1.
- Variáveis binárias declaradas como `ILOINT` com limites [0, 1].
- Representação fiel do enunciado: todos os dados do problema (preços, custos, consumos, proporções, capacidades) entram no código como constantes nomeadas, e qualquer quantidade derivada (lucro unitário, mínimos em litros, matriz de cobertura de turnos) é calculada pelo próprio código a partir desses dados, nunca pré-calculada à mão.

## Os modelos e as decisões de modelagem

### Programação linear contínua

**racao**: mix de produção (2 produtos, 2 recursos). Os dados do enunciado
(preços de venda, custos da carne e do cereal, consumo de insumos por ração e
disponibilidades) são constantes no código, e o lucro unitário de cada ração é
calculado a partir deles: lucro = preço de venda menos custo dos insumos
consumidos. As restrições de carne e cereal também usam os coeficientes de
consumo nomeados, não números soltos.

**dieta**: minimização de custo com requisitos mínimos de vitaminas A (>= 9) e
C (>= 19) sobre 6 ingredientes. Dados em `IloNumArray`, restrições montadas com
`IloExpr` em laço.

**plantio**: 3 fazendas x 3 culturas, restrições de área, água e área máxima
por cultura. Escolha pontual: a restrição de "mesma proporção de área cultivada
em todas as fazendas" foi escrita como igualdade de razões
(`plantada_i / area_i == plantada_j / area_j`), o que é válido no Concert porque
o denominador é constante, então a expressão continua linear. Encadeamos apenas
duas igualdades (1=2, 2=3); a terceira (1=3) é implicada por transitividade.

**tintas**: problema de mistura (blending). Os dados do enunciado entram como
constantes: custos por litro, frações de SEC/COR em cada solução, volumes a
produzir e composições mínimas exigidas. As restrições de composição
("SR com no mínimo 25% de SEC") são expressas em litros, com o lado direito
calculado no código como fração mínima vezes volume (`min_SEC_SR * vol_SR`).
Essa conversão é possível porque o volume de cada tinta é fixado por igualdade,
o que evita razões não-lineares.

**transporte**: 3 fábricas para 3 depósitos. Escolha pontual: oferta com `<=` e
demanda com `==`. Como oferta total (280) é igual à demanda total (280), o
problema é balanceado e essa combinação é consistente; usar `==` na demanda
garante o atendimento integral.

**fluxo**: fluxo máximo s a t no grafo do slide (8 nós, 15 arcos). Modelado como
LP clássico: variável de fluxo por arco com limite superior igual à capacidade,
conservação de fluxo (entra = sai) nos nós intermediários, maximizando o fluxo
que sai de s. Escolhas pontuais: variáveis contínuas (a matriz de incidência é
totalmente unimodular, então o ótimo é inteiro de qualquer forma, e o LP é mais
barato); arcos guardados numa lista de structs `{u, v, cap}`, o que deixa o
grafo trocável sem tocar no modelo. Ótimo da instância do slide: **13**
(verificado independentemente com networkx).

### Programação linear inteira

**exemplo1**: PLI didático do slide (max 3x + 4y), variáveis `ILOINT`.

**escalonamento**: escala de enfermeiras. x_j = quantas iniciam no dia j.
O enunciado do slide pede 5 dias consecutivos de trabalho e 3 de descanso.
A matriz de cobertura 7x7 nao e digitada a mao, e sim derivada da regra no
proprio codigo (quem inicia no dia j cobre os dias j ate j+4, modulo 7), com a
constante `dias_trabalho = 5`. Assim a regra do enunciado aparece explicita e
mudar a escala exige alterar um unico numero.

**cobertura**: localização de escolas, que é o problema do conjunto dominante
mínimo. A matriz de adjacência inclui a diagonal (bairro atende a si mesmo);
restrição: todo bairro coberto por pelo menos 1 escola vizinha.

**mochila**: knapsack 0-1 padrão: max soma de v*x, com soma de w*x <= W,
x binária.

**clique**: escolha pontual: formulação por restrições de conflito no
complemento do grafo. Para todo par (i, j) não adjacente, x_i + x_j <= 1
(numa clique, todo par presente deve ser adjacente). É a formulação mais
direta; para grafos densos ela gera poucas restrições, pois só os pares não
conectados geram restrição.

**padroes**: fábrica de latinhas. Escolhas pontuais:

- Além de x_j (impressões do padrão j), criamos a variável inteira L (latinhas montadas), com L <= corpos e 2L <= tampas, pois cada lata usa 1 corpo e 2 tampas. Isso lineariza o "min(corpos, tampas/2)" implícito no enunciado.
- Objetivo: 50L - 5(corpos - L) - 3(tampas - 2L), isto é, receita menos custo de estocagem das sobras, com receita e custos vindos das constantes do enunciado. O modelo decide o trade-off entre vender e estocar.
- Otimo verificado por forca bruta, sem uma restricao total de tempo adicional:
lucro 35.250 u, x = (5, 90, 65, 130), 705 latinhas e sem sobra de corpos ou
tampas. A linha de tempo de impressao da tabela nao entra no modelo porque o
slide nao informa limite total nem custo associado ao tempo.

**facilidades**: localização de facilidades não-capacitado (UFLP).
Escolha pontual: usamos a restrição de ligação desagregada
(x_ij <= y_i para cada par) em vez da agregada (soma_j x_ij <= M*y_i). São mais
restrições, porém a relaxação linear é estritamente mais forte, o que tende a
acelerar o branch-and-bound; é a formulação recomendada na literatura.

**frequencia**: atribuicao de frequencias, que e coloracao de grafos. x_ik
(antena i usa frequencia k), y_k (frequencia k e utilizada). Cada antena recebe
exatamente uma frequencia; pares com interferencia nao podem compartilhar a
mesma frequencia; e as restricoes x_ik <= y_k fazem a funcao objetivo contar
apenas frequencias realmente usadas. Como no pior caso cada antena usa uma
frequencia propria, K = n frequencias candidatas sao suficientes.
