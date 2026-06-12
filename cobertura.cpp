#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>

ILOSTLBEGIN

int main() {
    int n;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "PROBLEMA DA COBERTURA DE ESCOLAS" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Digite o numero de bairros da cidade: ";
    if (!(std::cin >> n)) return 0;

    std::vector<std::vector<int>> adj(n, std::vector<int>(n, 0));
    std::cout << "\nAgora vamos preencher as vizinhancas." << std::endl;
    std::cout << "Para cada bairro, digite uma linha contendo " << n << " numeros (0 ou 1 separados por espaco)." << std::endl;
    std::cout << "Digite 1 se os bairros sao vizinhos (ou se for o proprio bairro) e 0 caso contrario." << std::endl;
    std::cout << "Lembre-se: O proprio bairro sempre atende a ele mesmo, entao a diagonal principal deve ser 1.\n" << std::endl;

    for(int i = 0; i < n; i++) {
        std::cout << "Bairro " << (i+1) << ": ";
        for(int j = 0; j < n; j++) {
            std::cin >> adj[i][j];
        }
    }

    IloEnv env;
    try {
        IloModel model(env);

        // Variáveis de decisão: x[i] = 1 se construir escola no bairro i, 0 caso contrário
        // Usamos ILOINT com limite inferior 0 e superior 1 para forçar a ser Binária
        IloNumVarArray x(env, n, 0, 1, ILOINT); 

        // Função Objetivo: Minimizar total de escolas construídas
        IloExpr total_escolas(env);
        for(int i = 0; i < n; i++) {
            total_escolas += x[i];
        }
        model.add(IloMinimize(env, total_escolas));

        // Restrições de Cobertura: Todo bairro j deve ser coberto por pelo menos 1 escola
        // Ou seja, a soma de x[i] para todo i que seja vizinho de j (ou o próprio j) deve ser >= 1
        for(int j = 0; j < n; j++) {
            IloExpr cobertura(env);
            for(int i = 0; i < n; i++) {
                // Se o bairro i é capaz de atender o bairro j (matriz[j][i] == 1)
                if(adj[j][i] == 1) { 
                    cobertura += x[i];
                }
            }
            model.add(cobertura >= 1);
            cobertura.end();
        }

        // Solver
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "\nStatus da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Numero Minimo de Escolas a Construir: " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Locais exatos para construcao:" << std::endl;
            for(int i = 0; i < n; i++) {
                if(cplex.getValue(x[i]) > 0.5) { // Se for 1 (usamos 0.5 para evitar imprecisão de float)
                    std::cout << "  [x] Construir escola no Bairro " << (i+1) << std::endl;
                }
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima para essa matriz." << std::endl;
        }

        total_escolas.end();
    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
