#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        int num_dias = 7;

        IloIntArray d(env, num_dias);
        std::cout << "Digite a demanda de enfermeiras para cada um dos 7 dias da semana:" << std::endl;
        for(int i = 0; i < num_dias; i++) {
            std::cout << "Demanda do Dia " << (i + 1) << ": ";
            int demanda_val;
            std::cin >> demanda_val;
            d[i] = demanda_val;
        }

        // Variáveis de decisão: x[i] = número de enfermeiras que INICIAM o turno no dia i.
        // É essencial que a variável seja Inteira (ILOINT), pois não existe meia pessoa.
        IloNumVarArray x(env, num_dias, 0, IloInfinity, ILOINT);

        // Função Objetivo: Minimizar o número total de enfermeiras contratadas
        IloExpr total_enfermeiras(env);
        for(int i = 0; i < num_dias; i++) {
            total_enfermeiras += x[i];
        }
        model.add(IloMinimize(env, total_enfermeiras));

        // Escala do enunciado: trabalha 4 dias consecutivos e descansa 3.
        const int dias_trabalho = 4;

        // Matriz de cobertura derivada da regra da escala:
        // cobertura[i][j] = 1 se uma enfermeira que começa no dia j trabalha no dia i,
        // ou seja, se i estiver entre j e j + dias_trabalho - 1 (modulo 7).
        int cobertura[7][7];
        for(int i = 0; i < num_dias; i++) {
            for(int j = 0; j < num_dias; j++) {
                int dist = ((i - j) % num_dias + num_dias) % num_dias;
                cobertura[i][j] = (dist < dias_trabalho) ? 1 : 0;
            }
        }

        // Restrições de Demanda: Número de enfermeiras trabalhando no dia 'i' >= d[i]
        for(int i = 0; i < num_dias; i++) {
            IloExpr trabalhando_hoje(env);
            for(int j = 0; j < num_dias; j++) {
                if (cobertura[i][j] == 1) {
                    trabalhando_hoje += x[j];
                }
            }
            model.add(trabalhando_hoje >= d[i]);
            trabalhando_hoje.end();
        }

        // Solver
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Numero Minimo de Enfermeiras a Contratar: " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Plano de Contratacao (Inicios de turno):" << std::endl;
            for(int i = 0; i < num_dias; i++) {
                std::cout << "  Dia " << (i+1) << ": " << cplex.getValue(x[i]) << " enfermeira(s) iniciam o turno" << std::endl;
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        total_enfermeiras.end();

    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
