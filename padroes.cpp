#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        // ------------------- Dados do problema -------------------
        const int num_padroes = 4;

        // Disponibilidade de folhas de metal
        const double folhas_tipo1 = 200.0;
        const double folhas_tipo2 = 90.0;

        // Preco de venda da latinha e custos de estocagem
        const double preco_lata   = 50.0; // u por latinha vendida
        const double custo_corpo  = 5.0;  // u por corpo nao utilizado
        const double custo_tampa  = 3.0;  // u por tampa nao utilizada

        // Dados de cada padrao de impressao (tabela do slide):
        //                       Padrao:   1    2    3    4
        std::vector<int>    tipo_folha = { 1,   2,   1,   1 };   // tam folha
        std::vector<double> corpos     = { 1.0, 2.0, 0.0, 4.0 }; // num corpo
        std::vector<double> tampas     = { 7.0, 3.0, 9.0, 4.0 }; // num tampa

        std::cout << "-----------------------------------" << std::endl;
        std::cout << "PROBLEMA DE PADROES (FABRICA DE LATINHAS)" << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        IloModel model(env);

        // ------------------- Variaveis de decisao -------------------
        // x[j] = numero de impressoes (folhas) feitas com o padrao j (inteiro)
        IloNumVarArray x(env, num_padroes, 0, IloInfinity, ILOINT);

        // L = numero de latinhas montadas e vendidas (inteiro)
        // Cada latinha consome 1 corpo e 2 tampas.
        IloNumVar L(env, 0, IloInfinity, ILOINT, "Latas");

        // ------------------- Expressoes auxiliares -------------------
        // Total de corpos e tampas produzidos pelos padroes escolhidos
        IloExpr total_corpos(env), total_tampas(env);
        IloExpr folhas1_usadas(env), folhas2_usadas(env);
        for(int j = 0; j < num_padroes; j++) {
            total_corpos += corpos[j] * x[j];
            total_tampas += tampas[j] * x[j];
            if (tipo_folha[j] == 1) folhas1_usadas += x[j];
            else                    folhas2_usadas += x[j];
        }

        // ------------------- Funcao Objetivo -------------------
        // Lucro = receita das latas vendidas
        //       - custo de estocagem dos corpos que sobraram (total_corpos - L)
        //       - custo de estocagem das tampas que sobraram (total_tampas - 2L)
        IloExpr lucro(env);
        lucro += preco_lata * L;
        lucro -= custo_corpo * (total_corpos - L);
        lucro -= custo_tampa * (total_tampas - 2 * L);
        model.add(IloMaximize(env, lucro));

        // ------------------- Restricoes -------------------
        // 1) Nao se pode montar mais latas do que os corpos produzidos
        model.add(L <= total_corpos);

        // 2) Cada lata usa 2 tampas
        model.add(2 * L <= total_tampas);

        // 3) Disponibilidade de folhas de cada tipo
        model.add(folhas1_usadas <= folhas_tipo1);
        model.add(folhas2_usadas <= folhas_tipo2);

        // ------------------- Solver -------------------
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "\nStatus da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Lucro Maximo: " << cplex.getObjValue() << " u" << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            for(int j = 0; j < num_padroes; j++) {
                std::cout << "  Padrao " << (j+1) << ": " << cplex.getValue(x[j]) << " impressoes" << std::endl;
            }
            double nc = cplex.getValue(total_corpos);
            double nt = cplex.getValue(total_tampas);
            double nl = cplex.getValue(L);
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Corpos produzidos: " << nc << " | Tampas produzidas: " << nt << std::endl;
            std::cout << "Latinhas vendidas: " << nl << std::endl;
            std::cout << "Corpos estocados:  " << (nc - nl) << " | Tampas estocadas: " << (nt - 2*nl) << std::endl;
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        total_corpos.end();
        total_tampas.end();
        folhas1_usadas.end();
        folhas2_usadas.end();
        lucro.end();

    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
