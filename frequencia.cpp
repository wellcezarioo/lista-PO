#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>

ILOSTLBEGIN

int main() {
    int n;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "PROBLEMA DE FREQUENCIA (COLORACAO DE GRAFOS)" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Digite o numero de antenas: ";
    if (!(std::cin >> n)) return 0;

    std::vector<std::vector<int>> interf(n, std::vector<int>(n, 0));
    std::cout << "\nAgora vamos preencher a matriz de interferencia." << std::endl;
    std::cout << "Para cada antena, digite uma linha contendo " << n << " numeros (0 ou 1 separados por espaco)." << std::endl;
    std::cout << "Digite 1 se os raios das duas antenas se interceptam (ha interferencia) e 0 caso contrario." << std::endl;
    std::cout << "A diagonal principal deve ser 0.\n" << std::endl;

    for(int i = 0; i < n; i++) {
        std::cout << "Antena " << (i+1) << ": ";
        for(int j = 0; j < n; j++) {
            std::cin >> interf[i][j];
        }
    }

    // No pior caso, cada antena precisa de uma frequencia propria,
    // entao n frequencias candidatas sao suficientes.
    int K = n;

    IloEnv env;
    try {
        IloModel model(env);

        // ------------------- Variaveis de decisao -------------------
        // x[i][k] = 1 se a antena i usar a frequencia k, 0 caso contrario
        typedef IloArray<IloNumVarArray> NumVarMatrix;
        NumVarMatrix x(env, n);
        for(int i = 0; i < n; i++) {
            x[i] = IloNumVarArray(env, K, 0, 1, ILOINT);
        }

        // y[k] = 1 se a frequencia k for utilizada por alguma antena
        IloNumVarArray y(env, K, 0, 1, ILOINT);

        // ------------------- Funcao Objetivo -------------------
        // Minimizar o numero total de frequencias utilizadas
        IloExpr total_freq(env);
        for(int k = 0; k < K; k++) {
            total_freq += y[k];
        }
        model.add(IloMinimize(env, total_freq));

        // ------------------- Restricoes -------------------
        // 1) Cada antena recebe exatamente uma frequencia
        for(int i = 0; i < n; i++) {
            IloExpr soma(env);
            for(int k = 0; k < K; k++) {
                soma += x[i][k];
            }
            model.add(soma == 1);
            soma.end();
        }

        // 2) Antenas que interferem entre si nao podem usar a mesma frequencia.
        //    Alem disso, uma frequencia so pode ser atribuida se estiver em uso (y[k] = 1):
        //    x[i][k] + x[j][k] <= y[k] para todo par (i, j) com interferencia
        for(int i = 0; i < n; i++) {
            for(int j = i + 1; j < n; j++) {
                if(interf[i][j] == 1) {
                    for(int k = 0; k < K; k++) {
                        model.add(x[i][k] + x[j][k] <= y[k]);
                    }
                }
            }
        }

        // 3) Liga x e y tambem para antenas sem nenhuma interferencia
        //    (garante que y[k] conte a frequencia usada por antenas isoladas)
        for(int i = 0; i < n; i++) {
            for(int k = 0; k < K; k++) {
                model.add(x[i][k] <= y[k]);
            }
        }

        // 4) Quebra de simetria: a frequencia k+1 so pode ser usada se a k for
        //    (evita solucoes equivalentes que so trocam os rotulos das frequencias)
        for(int k = 0; k < K - 1; k++) {
            model.add(y[k] >= y[k+1]);
        }

        // ------------------- Solver -------------------
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "\nStatus da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Numero Minimo de Frequencias: " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Atribuicao de frequencias:" << std::endl;
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < K; k++) {
                    if(cplex.getValue(x[i][k]) > 0.5) {
                        std::cout << "  Antena " << (i+1) << " -> Frequencia " << (k+1) << std::endl;
                    }
                }
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        total_freq.end();
    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
