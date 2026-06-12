#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        int num_fazendas = 3;
        int num_culturas = 3;

        // Dados das fazendas
        IloNumArray area_fazenda(env, num_fazendas, 400.0, 650.0, 350.0);
        IloNumArray agua_fazenda(env, num_fazendas, 1800.0, 2200.0, 950.0);

        // Dados das culturas (1: milho, 2: arroz, 3: feijao)
        IloNumArray area_max_cultura(env, num_culturas, 660.0, 880.0, 400.0);
        IloNumArray consumo_agua(env, num_culturas, 5.5, 4.0, 3.5);
        IloNumArray lucro_cultura(env, num_culturas, 5000.0, 4000.0, 1800.0);

        // Variáveis de decisão: x[i][j] = área da cultura j na fazenda i
        typedef IloArray<IloNumVarArray> NumVarMatrix;
        NumVarMatrix x(env, num_fazendas);
        for(int i = 0; i < num_fazendas; i++) {
            x[i] = IloNumVarArray(env, num_culturas, 0.0, IloInfinity, ILOFLOAT);
        }

        // Função Objetivo: Maximizar o lucro total
        IloExpr lucro_total(env);
        for(int i = 0; i < num_fazendas; i++) {
            for(int j = 0; j < num_culturas; j++) {
                lucro_total += lucro_cultura[j] * x[i][j];
            }
        }
        model.add(IloMaximize(env, lucro_total));

        // Expressões para armazenar a área plantada em cada fazenda
        IloExprArray area_plantada_fazenda(env, num_fazendas);
        for(int i = 0; i < num_fazendas; i++) {
            area_plantada_fazenda[i] = IloExpr(env);
            for(int j = 0; j < num_culturas; j++) {
                area_plantada_fazenda[i] += x[i][j];
            }
        }

        // Restrição 1: Área disponível por fazenda
        for(int i = 0; i < num_fazendas; i++) {
            model.add(area_plantada_fazenda[i] <= area_fazenda[i]);
        }

        // Restrição 2: Água disponível por fazenda
        for(int i = 0; i < num_fazendas; i++) {
            IloExpr uso_agua(env);
            for(int j = 0; j < num_culturas; j++) {
                uso_agua += consumo_agua[j] * x[i][j];
            }
            model.add(uso_agua <= agua_fazenda[i]);
            uso_agua.end();
        }

        // Restrição 3: Área máxima por cultura
        for(int j = 0; j < num_culturas; j++) {
            IloExpr area_total_cultura(env);
            for(int i = 0; i < num_fazendas; i++) {
                area_total_cultura += x[i][j];
            }
            model.add(area_total_cultura <= area_max_cultura[j]);
            area_total_cultura.end();
        }

        // Restrição 4: Proporção de área cultivada igual para todas as fazendas
        // (Área plantada na Fazenda 1 / Área Total da Fazenda 1) == (Área plantada na Fazenda 2 / Área Total da Fazenda 2)
        model.add(area_plantada_fazenda[0] / area_fazenda[0] == area_plantada_fazenda[1] / area_fazenda[1]);
        // (Área plantada na Fazenda 2 / Área Total da Fazenda 2) == (Área plantada na Fazenda 3 / Área Total da Fazenda 3)
        model.add(area_plantada_fazenda[1] / area_fazenda[1] == area_plantada_fazenda[2] / area_fazenda[2]);

        // Resolvendo o modelo
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Lucro Total Maximo: R$ " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            
            const char* nome_culturas[] = {"Milho", "Arroz", "Feijao"};

            for(int i = 0; i < num_fazendas; i++) {
                std::cout << "Fazenda " << (i + 1) << " (Plantio Total: " << cplex.getValue(area_plantada_fazenda[i]) << " / " << area_fazenda[i] << " acres):" << std::endl;
                for(int j = 0; j < num_culturas; j++) {
                    std::cout << "  " << nome_culturas[j] << ": " << cplex.getValue(x[i][j]) << " acres" << std::endl;
                }
                std::cout << std::endl;
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        for(int i=0; i<num_fazendas; i++) {
            area_plantada_fazenda[i].end();
        }
        lucro_total.end();

    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
