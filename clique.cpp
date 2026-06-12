#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>

ILOSTLBEGIN

int main() {
    int n;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "PROBLEMA DA CLIQUE MAXIMA" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Digite o numero de vertices do grafo: ";
    if (!(std::cin >> n)) return 0;

    std::vector<std::vector<int>> adj(n, std::vector<int>(n, 0));
    std::cout << "\nAgora vamos preencher a matriz de adjacencia." << std::endl;
    std::cout << "Para cada vertice, digite uma linha contendo " << n << " numeros (0 ou 1 separados por espaco)." << std::endl;
    std::cout << "Digite 1 se os vertices sao adjacentes (existe aresta) e 0 caso contrario." << std::endl;
    std::cout << "A diagonal principal deve ser 0 (um vertice nao tem aresta com ele mesmo).\n" << std::endl;

    for(int i = 0; i < n; i++) {
        std::cout << "Vertice " << (i+1) << ": ";
        for(int j = 0; j < n; j++) {
            std::cin >> adj[i][j];
        }
    }

    IloEnv env;
    try {
        IloModel model(env);

        // Variáveis de decisão: x[i] = 1 se o vértice i pertence à clique, 0 caso contrário
        // Usamos ILOINT com limites 0 e 1 para forçar a ser Binária
        IloNumVarArray x(env, n, 0, 1, ILOINT);

        // Função Objetivo: Maximizar o número de vértices na clique
        IloExpr tamanho_clique(env);
        for(int i = 0; i < n; i++) {
            tamanho_clique += x[i];
        }
        model.add(IloMaximize(env, tamanho_clique));

        // Restrições de Conflito: se NAO existe aresta entre i e j, os dois
        // nao podem estar juntos na clique (em uma clique todos os pares sao adjacentes)
        // x[i] + x[j] <= 1 para todo par (i, j), i < j, com adj[i][j] == 0
        for(int i = 0; i < n; i++) {
            for(int j = i + 1; j < n; j++) {
                if(adj[i][j] == 0) {
                    model.add(x[i] + x[j] <= 1);
                }
            }
        }

        // Solver
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "\nStatus da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Tamanho da Clique Maxima: " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Vertices que formam a clique:" << std::endl;
            for(int i = 0; i < n; i++) {
                if(cplex.getValue(x[i]) > 0.5) { // Se for 1 (usamos 0.5 para evitar imprecisao de float)
                    std::cout << "  [x] Vertice " << (i+1) << std::endl;
                }
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        tamanho_clique.end();
    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
