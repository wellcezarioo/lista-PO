#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        // Variáveis de decisão para a Tinta de Secagem Rápida (SR)
        IloNumVar xA_SR(env, 0, IloInfinity, ILOFLOAT, "SolA_SR");
        IloNumVar xB_SR(env, 0, IloInfinity, ILOFLOAT, "SolB_SR");
        IloNumVar xSEC_SR(env, 0, IloInfinity, ILOFLOAT, "SEC_SR");
        IloNumVar xCOR_SR(env, 0, IloInfinity, ILOFLOAT, "COR_SR");

        // Variáveis de decisão para a Tinta de Secagem Normal (SN)
        IloNumVar xA_SN(env, 0, IloInfinity, ILOFLOAT, "SolA_SN");
        IloNumVar xB_SN(env, 0, IloInfinity, ILOFLOAT, "SolB_SN");
        IloNumVar xSEC_SN(env, 0, IloInfinity, ILOFLOAT, "SEC_SN");
        IloNumVar xCOR_SN(env, 0, IloInfinity, ILOFLOAT, "COR_SN");

        // Função Objetivo: Minimizar o Custo Total
        // Custo = 1.5*(SolA) + 1.0*(SolB) + 4.0*(SEC) + 6.0*(COR)
        IloExpr cost(env);
        cost += 1.5 * (xA_SR + xA_SN);
        cost += 1.0 * (xB_SR + xB_SN);
        cost += 4.0 * (xSEC_SR + xSEC_SN);
        cost += 6.0 * (xCOR_SR + xCOR_SN);
        model.add(IloMinimize(env, cost));

        // Restrição 1: Volume total da tinta SR deve ser 1000 litros
        model.add(xA_SR + xB_SR + xSEC_SR + xCOR_SR == 1000);

        // Restrição 2: Volume total da tinta SN deve ser 250 litros
        model.add(xA_SN + xB_SN + xSEC_SN + xCOR_SN == 250);

        // Restrição 3: SR tem que ter no mínimo 25% de SEC (0.25 * 1000 = 250 litros)
        // A SolA fornece 30% de SEC, a SolB fornece 60% de SEC, o SEC puro fornece 100%
        model.add(0.3 * xA_SR + 0.6 * xB_SR + 1.0 * xSEC_SR >= 250);

        // Restrição 4: SR tem que ter no mínimo 50% de COR (0.50 * 1000 = 500 litros)
        // A SolA fornece 70% de COR, a SolB fornece 40% de COR, o COR puro fornece 100%
        model.add(0.7 * xA_SR + 0.4 * xB_SR + 1.0 * xCOR_SR >= 500);

        // Restrição 5: SN tem que ter no mínimo 20% de SEC (0.20 * 250 = 50 litros)
        model.add(0.3 * xA_SN + 0.6 * xB_SN + 1.0 * xSEC_SN >= 50);

        // Restrição 6: SN tem que ter no mínimo 50% de COR (0.50 * 250 = 125 litros)
        model.add(0.7 * xA_SN + 0.4 * xB_SN + 1.0 * xCOR_SN >= 125);

        // Resolver o modelo
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Custo Minimo Total: R$ " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            
            // Quantidades totais a serem compradas
            double total_SolA = cplex.getValue(xA_SR) + cplex.getValue(xA_SN);
            double total_SolB = cplex.getValue(xB_SR) + cplex.getValue(xB_SN);
            double total_SEC = cplex.getValue(xSEC_SR) + cplex.getValue(xSEC_SN);
            double total_COR = cplex.getValue(xCOR_SR) + cplex.getValue(xCOR_SN);

            std::cout << "Quantidade Total a Comprar:" << std::endl;
            std::cout << "  Solucao A (SolA): " << total_SolA << " litros" << std::endl;
            std::cout << "  Solucao B (SolB): " << total_SolB << " litros" << std::endl;
            std::cout << "  Comp. SEC puro:   " << total_SEC << " litros" << std::endl;
            std::cout << "  Comp. COR puro:   " << total_COR << " litros" << std::endl;
            
            std::cout << "\nDetalhes da Producao (Tinta SR - 1000 L):" << std::endl;
            std::cout << "  SolA: " << cplex.getValue(xA_SR) << " L, SolB: " << cplex.getValue(xB_SR) 
                      << " L, SEC: " << cplex.getValue(xSEC_SR) << " L, COR: " << cplex.getValue(xCOR_SR) << " L" << std::endl;

            std::cout << "\nDetalhes da Producao (Tinta SN - 250 L):" << std::endl;
            std::cout << "  SolA: " << cplex.getValue(xA_SN) << " L, SolB: " << cplex.getValue(xB_SN) 
                      << " L, SEC: " << cplex.getValue(xSEC_SN) << " L, COR: " << cplex.getValue(xCOR_SN) << " L" << std::endl;
            std::cout << "-----------------------------------" << std::endl;

        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }
        
    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
