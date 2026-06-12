#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        // ------------------- Dados do enunciado -------------------
        // Custos (R$ por litro)
        const double custo_SolA = 1.5, custo_SolB = 1.0;
        const double custo_SEC  = 4.0, custo_COR  = 6.0;

        // Composicao das solucoes (fracao de SEC e de COR por litro)
        const double SEC_em_SolA = 0.30, COR_em_SolA = 0.70;
        const double SEC_em_SolB = 0.60, COR_em_SolB = 0.40;

        // Volumes a produzir (litros)
        const double vol_SR = 1000.0;
        const double vol_SN = 250.0;

        // Composicao minima exigida (fracao do volume da tinta)
        const double min_SEC_SR = 0.25, min_COR_SR = 0.50;
        const double min_SEC_SN = 0.20, min_COR_SN = 0.50;

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

        // Função Objetivo: Minimizar o Custo Total de compra dos produtos
        IloExpr cost(env);
        cost += custo_SolA * (xA_SR + xA_SN);
        cost += custo_SolB * (xB_SR + xB_SN);
        cost += custo_SEC  * (xSEC_SR + xSEC_SN);
        cost += custo_COR  * (xCOR_SR + xCOR_SN);
        model.add(IloMinimize(env, cost));

        // Restrição 1: Volume total da tinta SR
        model.add(xA_SR + xB_SR + xSEC_SR + xCOR_SR == vol_SR);

        // Restrição 2: Volume total da tinta SN
        model.add(xA_SN + xB_SN + xSEC_SN + xCOR_SN == vol_SN);

        // Restrição 3: SR deve ter no mínimo min_SEC_SR de SEC.
        // Cada litro de SolA fornece SEC_em_SolA de SEC, SolB fornece SEC_em_SolB,
        // e o componente SEC puro fornece 1.0. Como o volume e fixo, a fracao
        // minima vira litros: min_SEC_SR * vol_SR.
        model.add(SEC_em_SolA * xA_SR + SEC_em_SolB * xB_SR + 1.0 * xSEC_SR >= min_SEC_SR * vol_SR);

        // Restrição 4: SR deve ter no mínimo min_COR_SR de COR
        model.add(COR_em_SolA * xA_SR + COR_em_SolB * xB_SR + 1.0 * xCOR_SR >= min_COR_SR * vol_SR);

        // Restrição 5: SN deve ter no mínimo min_SEC_SN de SEC
        model.add(SEC_em_SolA * xA_SN + SEC_em_SolB * xB_SN + 1.0 * xSEC_SN >= min_SEC_SN * vol_SN);

        // Restrição 6: SN deve ter no mínimo min_COR_SN de COR
        model.add(COR_em_SolA * xA_SN + COR_em_SolB * xB_SN + 1.0 * xCOR_SN >= min_COR_SN * vol_SN);

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

            s