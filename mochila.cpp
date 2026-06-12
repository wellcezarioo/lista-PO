#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>

ILOSTLBEGIN

int main() {
    int n;
    double W;

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "PROBLEMA DA MOCHILA (0-1 KNAPSACK)" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Digite o numero de itens disponiveis (n): ";
    if (!(std::cin >> n)) return 0;

    std::cout << "Digite a capacidade maxima de peso da mochila (W): ";
    if (!(std::cin >> W)) return 0;

    std::vector<double> v(n);
    std::vector<double> w(n);

    std::cout << "\nPara cada item, digite o seu VALOR e o seu PESO (separados por espaco):" << std::endl;
    for(int i = 0; i < n; i++) {
        std::cout << "Item " << (i+1) << " (Valor Peso): ";
        std::cin >> v[i] >> w[i];
    }

    IloEnv env;
    try {
        IloModel model(env);

        // Variáveis de decisão: x[i] = 1 se o item i for colocado na mochila, 0 caso contrário
        // Usamos ILOINT com limite 0 e 1 para garantir que é uma Variável Binária
        IloNumVarArray x(env, n, 0, 1, ILOINT); 

        // Função Objetivo: Maximizar a soma dos valores dos itens escolhidos
        IloExpr valor_total(env);
        for(int i = 0; i < n; i++) {
            valor_total += v[i] * x[i];
        }
        model.add(IloMaximize(env, valor_total));

        // Restrição de Capacidade: O peso total não pode ultrapassar o limite da mochila (W)
        IloExpr peso_total(env);
        for(int i = 0; i < n; i++) {
            peso_total += w[i] * x[i];
        }
        model.add(peso_total <= W);

        // Solver
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "\nStatus da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "VALOR MAXIMO OBTIDO: " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Itens escolhidos para a mochila:" << std::endl;
            
            double peso_usado = 0;
            for(int i = 0; i < n; i++) {
                if(cplex.getValue(x[i]) > 0.5) { // Verifica se a variável binária ativou o item
                    std::cout << "  [x] Item " << (i+1) << " (Valor: " << v[i] << ", Peso: " << w[i] << ")" << std::endl;
                    peso_usado += w[i];
                }
            }
            std::cout << "-> Peso total utilizado: " << peso_usado << " / " << W << std::endl;
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        valor_total.end();
        peso_total.end();
    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
