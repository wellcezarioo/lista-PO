# Lista de Modelagem — Otimização Linear (Inteira)

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

- **Regra-padrão `%: %.cpp`** no lugar de uma regra por programa — elimina 14 regras idênticas; para adicionar um modelo novo basta incluí-lo em `PROGS`.
- **`CPLEXDIR ?=`** (atribuição condicional) — permite compilar em outra máquina sem editar o arquivo: `make CPLEXDIR=/outro/caminho/cplex`.
- **`-O2 -Wall -std=c++17`** no lugar de `-O` — otimização e avisos do compilador; o padrão fixado evita variação entre versões do g++.
- **`.PHONY: all clean`** — evita conflito caso exista um arquivo chamado `all` ou `clean`.
- `.gitignore` cobre os binários gerados (mesmo nome dos alvos, sem extensão).

## Convenções comuns a todos os códigos

- Padrão Concert: `IloEnv` → `try/catch(IloException)` → `env.end()` (o `env.end()` libera toda a memória do ambiente).
- `cplex.setOut(env.getNullStream())` para suprimir o log do solver.
- Leitura de soluções com tolerância: `> 1e-5` para variáveis contínuas e `> 0.5` para binárias (evita erro de ponto flutuante — o CPLEX pode devolver 0.9999999 para uma binária igual a 1).
- Variáveis binárias declaradas como `ILOINT` com limites [0, 1].

## Os modelos e as decisões de modelagem

### Programação linear contínua

**racao** — mix de produção (2 produtos, 2 recursos). O lucro unitário foi
pré-calculado no comentário do código: lucro = preço − custo dos insumos
(AMGS: 20 − 9 = 11; RE: 30 − 18 = 12), em vez de modelar custos como variáveis.

**dieta** — minimização de custo com requisitos mínimos de vitaminas A (≥ 9) e
C (≥ 19) sobre 6 ingredientes. Dados em `IloNumArray`, restrições montadas com
`IloExpr` em laço.

**plantio** — 3 fazendas × 3 culturas, restrições de área, água e área máxima
por cultura. *Escolha pontual:* a restrição de "mesma proporção de área cultivada
em todas as fazendas" foi escrita como igualdade de razões
(`plantada_i / area_i == plantada_j / area_j`), o que é válido no Concert porque o
denominador é constante — a expressão continua linear. Encadeamos apenas duas
igualdades (1=2, 2=3); a terceira (1=3) é implicada por transitividade.

**tintas** — problema de mistura (blending). As restrições de composição mínima
("SR com ≥ 25% de SEC") foram convertidas para litros absolutos
(0.3·xA + 0.6·xB + 1.0·xSEC ≥ 250), já que o volume total de cada tinta é fixado
por igualdade — isso evita razões não-lineares.

**transporte** — 3 fábricas → 3 depósitos. *Escolha pontual:* oferta com `<=` e
demanda com `==`. Como oferta total (280) = demanda total (280), o problema é
balanceado e essa combinação é consistente; usar `==` na demanda garante o
atendimento integral.

**fluxo** — fluxo máximo s→t no grafo do slide (8 nós, 15 arcos). Modelado como
LP clássico: variável de fluxo por arco com limite superior = capacidade,
conservação de fluxo (entra = sai) nos nós intermediários, maximizando o fluxo
que sai de s. *Escolhas pontuais:* variáveis contínuas (a matriz de incidência é
totalmente unimodular, então o ótimo é inteiro de qualquer forma, e o LP é mais
barato); arcos guardados numa lista de structs `{u, v, cap}`, o que deixa o
grafo trocável sem tocar no modelo. Ótimo da instância do slide: **13**
(verificado independentemente com networkx).

### Programação linear inteira

**exemplo1** — PLI didático do slide (max 3x + 4y), variáveis `ILOINT`.

**escalonamento** — escala de enfermeiras. x_j = quantas iniciam no dia j;
cobertura via matriz 7×7 binária. *Correção importante:* a primeira versão usava
a escala clássica "trabalha 5, folga 2", mas o enunciado do slide pede
**trabalha 4 dias consecutivos, descansa 3** — a matriz foi corrigida para
quem inicia no dia j cobrir os dias j..j+3 (mod 7). Os resultados das duas
versões são diferentes, então vale conferir qual enunciado está sendo usado.

**cobertura** — localização de escolas = conjunto dominante mínimo. A matriz de
adjacência inclui a diagonal (bairro atende a si mesmo); restrição: todo bairro
coberto por ≥ 1 escola vizinha.

**mochila** — knapsack 0-1 padrão: max Σv·x, Σw·x ≤ W, x binária.

**clique** — *escolha pontual:* formulação por restrições de conflito no
**complemento do grafo**: para todo par (i, j) **não adjacente**, x_i + x_j ≤ 1
(numa clique, todo par presente deve ser adjacente). É a formulação mais direta;
para grafos densos ela gera poucas restrições (só os pares não conectados).

**padroes** — fábrica de latinhas. *Escolhas pontuais:*
- Além de x_j (impressões do padrão j), criamos a variável inteira **L = latinhas
  montadas**, com L ≤ corpos e 2L ≤ tampas (cada lata = 1 corpo + 2 tampas).
  Isso lineariza o "min(corpos, tampas/2)" implícito no enunciado.
- Objetivo: 50L − 5(corpos − L) − 3(tampas − 2L) — receita menos estocagem das
  sobras. O modelo decide o trade-off entre vender e estocar.
- Ótimo verificado por força bruta: **lucro 10.522 u**, x = (0, 0, 23, 54),
  211 latinhas, tempo de impressão esgotado (100 s).

**facilidades** — localização de facilidades não-capacitado (UFLP).
*Escolha pontual:* usamos a restrição de ligação **desagregada**
(x_ij ≤ y_i para cada par) em vez da agregada (Σ_j x_ij ≤ M·y_i). São mais
restrições, porém a relaxação linear é estritamente mais forte, o que tende a
acelerar o branch-and-bound — é a formulação recomendada na literatura.

**frequencia** — atribuição de frequências = coloração de grafos. x_ik (antena i
usa frequência k), y_k (frequência k em uso), K = n frequências candidatas
(limite superior trivial). *Escolhas pontuais:*
- Ligação x–y feita em duas camadas: x_ik + x_jk ≤ y_k para pares com
  interferência (mais forte que duas restrições separadas) e x_ik ≤ y_k para
  todos, cobrindo antenas isoladas.
- **Quebra de simetria** y_k ≥ y_{k+1}: sem ela, qualquer permutação dos rótulos
  das frequências é uma solução equivalente e o solver perde tempo explorando
  cópias; com ela, as frequências usadas são sempre as primeiras.

## Validação

- `padroes` e as 6 instâncias de `instances/` tiveram o ótimo conferido por
  enumeração exaustiva em Python (independente do CPLEX).
- `fluxo` foi conferido com o algoritmo de fluxo máximo do networkx (ótimo = 13).
- Os dados de todos os modelos foram conferidos contra o PDF dos slides
  (`modelagem_otimizacao.pdf`), o que inclusive revelou a divergência da escala
  do `escalonamento` (4×3 vs. 5×2), já corrigida.
