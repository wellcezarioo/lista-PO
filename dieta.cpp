#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        int num_ingredientes = 6;

        // Arrays de dados
        IloNumArray preco(env, num_ingredientes, 35.0, 30.0, 60.0, 50.0, 27.0, 22.0);
        IloNumArray vitA(env, num_ingredientes, 1.0, 0.0, 2.0, 2.0, 1.0, 2.0);
        IloNumArray vitC(env, num_ingredientes, 0.0, 1.0, 3.0, 1.0, 3.0, 2.0);

        // Variáveis de decisão (x1 a x6 contínuas e não-negativas)
        IloNumVarArray x(env, num_ingredientes, 0.0, IloInfinity, ILOFLOAT);

        // Expressão da Função Objetivo (Minimizar o custo total)
        IloExpr custo_total(env);
        for(int i = 0; i < num_ingredientes; i++) {
            custo_total += preco[i] * x[i];
        }
        model.add(IloMinimize(env, custo_total));

        // Expressão da Restrição de Vitamina A (>= 9)
        IloExpr exprA(env);
        for(int i = 0; i < num_ingredientes; i++) {
            exprA += vitA[i] * x[i];
        }
        model.add(exprA >= 9.0);

        // Expressão da Restrição de Vitamina C (>= 19)
        IloExpr exprC(env);
        for(int i = 0; i < num_ingredientes; i++) {
            exprC += vitC[i] * x[i];
        }
        model.add(exprC >= 19.0);

        // Solver
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // Desativa os logs longos do solver

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Custo Minimo Total: R$ " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            for(int i = 0; i < num_ingredientes; i++) {
                // Imprime apenas os ingredientes que entram na dieta (valor > 0)
                if(cplex.getValue(x[i]) > 1e-5) {
                    std::cout << "Quantidade do Ingrediente " << (i + 1) << " (x" << (i + 1) << "): " << cplex.getValue(x[i]) << std::endl;
                }
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        // Libera memoria
        custo_total.end();
        exprA.end();
        exprC.end();

    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
