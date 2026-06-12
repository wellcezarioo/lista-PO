#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>

ILOSTLBEGIN

int main() {
    int N, M;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "PROBLEMA DAS FACILIDADES (FACILITY LOCATION)" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Digite o numero de centros candidatos a deposito (N): ";
    if (!(std::cin >> N)) return 0;
    std::cout << "Digite o numero de clientes (M): ";
    if (!(std::cin >> M)) return 0;

    // Custo de instalacao f[i] de cada deposito
    std::vector<double> f(N);
    std::cout << "\nDigite o custo de instalacao de cada um dos " << N << " depositos:" << std::endl;
    for(int i = 0; i < N; i++) {
        std::cout << "f[" << (i+1) << "]: ";
        std::cin >> f[i];
    }

    // Custo de atendimento c[i][j] entre o deposito i e o cliente j
    std::vector<std::vector<double>> c(N, std::vector<double>(M));
    std::cout << "\nAgora a matriz de custos de atendimento." << std::endl;
    std::cout << "Para cada deposito, digite " << M << " valores (custo para atender cada cliente):" << std::endl;
    for(int i = 0; i < N; i++) {
        std::cout << "Deposito " << (i+1) << ": ";
        for(int j = 0; j < M; j++) {
            std::cin >> c[i][j];
        }
    }

    IloEnv env;
    try {
        IloModel model(env);

        // ------------------- Variaveis de decisao -------------------
        // y[i] = 1 se o deposito i for construido, 0 caso contrario
        IloNumVarArray y(env, N, 0, 1, ILOINT);

        // x[i][j] = 1 se o cliente j for atendido pelo deposito i, 0 caso contrario
        typedef IloArray<IloNumVarArray> NumVarMatrix;
        NumVarMatrix x(env, N);
        for(int i = 0; i < N; i++) {
            x[i] = IloNumVarArray(env, M, 0, 1, ILOINT);
        }

        // ------------------- Funcao Objetivo -------------------
        // Minimizar custo de instalacao + custo de atendimento
        IloExpr custo_total(env);
        for(int i = 0; i < N; i++) {
            custo_total += f[i] * y[i];
            for(int j = 0; j < M; j++) {
                custo_total += c[i][j] * x[i][j];
            }
        }
        model.add(IloMinimize(env, custo_total));

        // ------------------- Restricoes -------------------
        // 1) Todo cliente j deve ser atendido por exatamente um deposito
        for(int j = 0; j < M; j++) {
            IloExpr atendimento(env);
            for(int i = 0; i < N; i++) {
                atendimento += x[i][j];
            }
            model.add(atendimento == 1);
            atendimento.end();
        }

        // 2) Um cliente so pode ser atendido por um deposito que foi construido
        //    (x[i][j] <= y[i]): se y[i] = 0, nenhum cliente pode usar o deposito i
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < M; j++) {
                model.add(x[i][j] <= y[i]);
            }
        }

        // ------------------- Solver -------------------
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "\nStatus da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Custo Minimo Total: " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Depositos a construir:" << std::endl;
            for(int i = 0; i < N; i++) {
                if(cplex.getValue(y[i]) > 0.5) {
                    std::cout << "  [x] Deposito " << (i+1) << " (custo de instalacao: " << f[i] << ")" << std::endl;
                }
            }
            std::cout << "\nPlano de atendimento:" << std::endl;
            for(int j = 0; j < M; j++) {
                for(int i = 0; i < N; i++) {
                    if(cplex.getValue(x[i][j]) > 0.5) {
                        std::cout << "  Cliente " << (j+1) << " <- Deposito " << (i+1)
                                  << " (custo: " << c[i][j] << ")" << std::endl;
                    }
                }
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        custo_total.end();
    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
