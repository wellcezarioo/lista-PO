#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        // ------------------- Dados do enunciado -------------------
        // Precos de venda (R$ por unidade de racao)
        const double preco_AMGS = 20.0;
        const double preco_RE   = 30.0;

        // Custos dos insumos (R$ por kg)
        const double custo_carne  = 4.0;
        const double custo_cereal = 1.0;

        // Consumo de insumos por unidade de racao (kg)
        const double cereal_AMGS = 5.0, carne_AMGS = 1.0;
        const double cereal_RE   = 2.0, carne_RE   = 4.0;

        // Disponibilidade de insumos (kg)
        const double disp_carne  = 10000.0;
        const double disp_cereal = 30000.0;

        // Variáveis de decisão (quantidades a produzir de AMGS e RE)
        // Podem ser contínuas (IloNumVar)
        IloNumVar AMGS(env, 0, IloInfinity, ILOFLOAT, "AMGS");
        IloNumVar RE(env, 0, IloInfinity, ILOFLOAT, "RE");

        // Lucro unitario calculado a partir dos dados:
        // lucro = preco de venda - custo dos insumos consumidos
        const double lucro_AMGS = preco_AMGS - (cereal_AMGS * custo_cereal + carne_AMGS * custo_carne);
        const double lucro_RE   = preco_RE   - (cereal_RE   * custo_cereal + carne_RE   * custo_carne);

        // Função Objetivo: Maximizar o lucro total
        IloObjective obj = IloMaximize(env, lucro_AMGS * AMGS + lucro_RE * RE);
        model.add(obj);

        // Restrição 1: Limite de Carne
        model.add(carne_AMGS * AMGS + carne_RE * RE <= disp_carne);

        // Restrição 2: Limite de Cereais
        model.add(cereal_AMGS * AMGS + cereal_RE * RE <= disp_cereal);

        // Resolvendo
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // Ocultar log longo do CPLEX

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << 