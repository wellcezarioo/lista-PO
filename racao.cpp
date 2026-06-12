#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        // Variáveis de decisão (quantidades a produzir de AMGS e RE)
        // Podem ser contínuas (IloNumVar)
        IloNumVar AMGS(env, 0, IloInfinity, ILOFLOAT, "AMGS");
        IloNumVar RE(env, 0, IloInfinity, ILOFLOAT, "RE");

        // Custos:
        // Custo carne = 4, Custo cereal = 1
        // Custo AMGS = (5kg cereal * 1) + (1kg carne * 4) = 5 + 4 = 9 reais
        // Custo RE = (2kg cereal * 1) + (4kg carne * 4) = 2 + 16 = 18 reais
        
        // Lucro = Preco de venda - Custo
        // Lucro AMGS = 20 - 9 = 11 reais
        // Lucro RE = 30 - 18 = 12 reais

        // Função Objetivo: Maximizar o lucro total
        IloObjective obj = IloMaximize(env, 11 * AMGS + 12 * RE);
        model.add(obj);

        // Restrição 1: Limite de Carne
        // AMGS usa 1kg, RE usa 4kg. Total disponível: 10.000 kg
        model.add(1 * AMGS + 4 * RE <= 10000);

        // Restrição 2: Limite de Cereais
        // AMGS usa 5kg, RE usa 2kg. Total disponível: 30.000 kg
        model.add(5 * AMGS + 2 * RE <= 30000);

        // Resolvendo
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // Ocultar log longo do CPLEX

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Lucro Maximo: R$ " << cplex.getObjValue() << std::endl;
            std::cout << "Quantidade de AMGS: " << cplex.getValue(AMGS) << std::endl;
            std::cout << "Quantidade de RE: " << cplex.getValue(RE) << std::endl;
        } else {
            std::cout << "Nao foi possivel resolver o problema." << std::endl;
        }

    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida" << std::endl;
    }

    env.end();
    return 0;
}
