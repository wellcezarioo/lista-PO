#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        // ------------------- Dados do grafo (slide) -------------------
        // Nos: s, v1..v6, t  (s = origem do gas, t = fabrica)
        std::vector<std::string> nos = {"s","v1","v2","v3","v4","v5","v6","t"};
        const std::string ORIGEM = "s";
        const std::string DESTINO = "t";

        std::map<std::string,int> idx;
        for(size_t i = 0; i < nos.size(); i++) idx[nos[i]] = (int)i;

        // Arcos direcionados (origem, destino, capacidade)
        struct Arco { std::string u, v; double cap; };
        std::vector<Arco> arcos = {
            {"s","v1",5}, {"s","v2",4}, {"s","v3",6},
            {"v1","v2",4}, {"v2","v3",3},
            {"v1","v4",6}, {"v1","v5",5},
            {"v2","v5",4},
            {"v3","v5",6}, {"v3","v6",5},
            {"v5","v4",5}, {"v6","v5",7},
            {"v4","t",5}, {"v5","t",3}, {"v6","t",6}
        };
        int m = (int)arcos.size();

        IloModel model(env);

        // ------------------- Variaveis de decisao -------------------
        // f[a] = fluxo no arco a, limitado pela capacidade do arco
        IloNumVarArray f(env, m);
        for(int a = 0; a < m; a++) {
            f[a] = IloNumVar(env, 0.0, arcos[a].cap, ILOFLOAT);
        }

        // ------------------- Funcao Objetivo -------------------
        // Maximizar o fluxo total que sai da origem s
        IloExpr fluxo_origem(env);
        for(int a = 0; a < m; a++) {
            if (arcos[a].u == ORIGEM) fluxo_origem += f[a];
        }
        model.add(IloMaximize(env, fluxo_origem));

        // ------------------- Restricoes de conservacao de fluxo -------------------
        // Para todo no intermediario: soma do que entra = soma do que sai
        for(const auto& no : nos) {
            if (no == ORIGEM || no == DESTINO) continue;
            IloExpr entra(env), sai(env);
            for(int a = 0; a < m; a++) {
                if (arcos[a].v == no) entra += f[a];
                if (arcos[a].u == no) sai   += f[a];
            }
            model.add(entra == sai);
            entra.end();
            sai.end();
        }

        // ------------------- Solver -------------------
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (cplex.solve()) {
            std::cout << "Status da Solucao: " << cplex.getStatus() << std::endl;
            std::cout << "Fluxo Maximo de " << ORIGEM << " para " << DESTINO
                      << ": " << cplex.getObjValue() << std::endl;
            std::cout << "-----------------------------------" << std::endl;
            std::cout << "Fluxo em cada arco (utilizado / capacidade):" << std::endl;
            for(int a = 0; a < m; a++) {
                double val = cplex.getValue(f[a]);
                std::cout << "  " << arcos[a].u << " -> " << arcos[a].v
                          << ": " << val << " / " << arcos[a].cap;
                if (val > 1e-6) std::cout << "   <--";
                std::cout << std::endl;
            }
            std::cout << "-----------------------------------" << std::endl;
        } else {
            std::cout << "O solver nao conseguiu encontrar uma solucao otima." << std::endl;
        }

        fluxo_origem.end();
    } catch (IloException& e) {
        std::cerr << "Excecao CPLEX capturada: " << e << std::endl;
    } catch (...) {
        std::cerr << "Excecao desconhecida capturada." << std::endl;
    }

    env.end();
    return 0;
}
