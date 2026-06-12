#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main(int argc, char **argv) {
    IloEnv env;
    try {
        IloModel model(env);

        // Definindo as variáveis de decisão como inteiras e não-negativas (Programação Linear Inteira)
        IloNumVar x(env, 0, IloInfinity, ILOINT, "x");
        IloNumVar y(env, 0, IloInfinity, ILOINT, "y");

        // Função Objetivo: Maximizar 3x + 4y
        IloObjective obj = IloMaximize(env, 3 * x + 4 * y);
        model.add(obj);

        // Restrição 1: 2x + y <= 8
        model.add(2 * x + y <= 8);

        // Restrição 2: x + 2y <= 6
        model.add(x + 2 * y <= 6);

        // Criando o objeto do solver do CPLEX
        IloCplex cplex(model);

        // Resolvendo o modelo
        std::cout << "Iniciando a resolucao pelo CPLEX..." << std::endl;
        if (cplex.solve()) {
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Status da solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Valor da funcao objetivo (Z) = " << cplex.getObjValue() << std::endl;
            std::cout << "Valor de x = " << cplex.getValue(x) << std::endl;
            std::cout << "Valor de y = " << cplex.getValue(y) << std::endl;
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "Falha ao encontrar uma solucao." << std::endl;
        }

    } catch (IloException& e) {
        std::cerr << "Excecao do Concert: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    // Encerrando o ambiente CPLEX
    env.end();
    return 0;
}
