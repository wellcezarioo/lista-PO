#include <ilcplex/ilocplex.h>
#include <iostream>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);

        int num_fabricas = 3;
        int num_depositos = 3;

        // Oferta de cada fábrica
        IloNumArray oferta(env, num_fabricas, 120.0, 80.0, 80.0);

        // Demanda de cada depósito
        IloNumArray demanda(env, num_depositos, 150.0, 70.0, 60.0);

        // Matriz de Custos de Transporte C_ij
        IloNumArray2 custo(env, num_fabricas);
        custo[0] = IloNumArray(env, num_depositos, 8.0, 5.0, 6.0);
        custo[1] = IloNumArray(env, num_depositos, 15.0, 10.0, 12.0);
        custo[2] = IloNumArray(env, num_depositos, 3.0, 9.0, 10.0);

        // Variáveis de decisão: x[i][j] = quantidade enviada da fábrica i para o depósito j
        typedef IloArray<IloNumVarArray> NumVarMatrix;
        NumVarMatrix x(env, num_fabricas);
        for(int i = 0; i < num_fabricas; i++) {
            x[i] = IloNumVarArray(env, num_depositos, 0.0, IloInfinity, ILOFLOAT);
        }

        // Função Objetivo: Minimizar o custo total de transporte
        IloExpr custo_total(env);
        for(int i = 0; i < num_fabricas; i++) {
            for(int j = 0; j < num_depositos; j++) {
                custo_total += custo[i][j] * x[i][j];
            }
        }
        model.add(IloMinimize(env, custo_total));

        // Restrições de Oferta: Tudo que sai da fábrica i deve ser menor ou igual a sua oferta
        for(int i = 0; i < num_fabricas; i++) {
            IloExpr total_enviado(env);
            for(int j = 0; j < num_depositos; j++) {
                total_enviado += x[i][j];
            }
            model.add(total_enviado <= oferta[i]);
            total_enviado.end();
        }

        // Restrições de Demanda: Tudo que chega no depósito j deve ser exatamente igual a sua demanda
        for(int j = 0; j < num_depositos; j++) {
            IloExpr total_recebido(env);
            for(int i = 0; i < num_fabricas; i++) {
                total_recebido += x[i][j];
            }
            model.add(total_recebido == demanda[j]); 
            total_recebido.end();
        }

        // Solver
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // Desabilita log longo do CPLEX

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Custo Minimo Total de Transporte: " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Plano Otimizado de Transporte:" << std::endl;
            for(int i = 0; i < num_fabricas; i++) {
                for(int j = 0; j < num_depositos; j++) {
                    // Imprime apenas rotas que foram utilizadas
                    if (cplex.getValue(x[i][j]) > 1e-5) {
                        std::cout << "  Fabrica " << (i+1) << " -> Deposito " << (j+1) 
                                  << ": Enviar " << cplex.getValue(x[i][j]) << " unidades (Custo Unitario: " << custo[i][j] << ")" << std::endl;
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
