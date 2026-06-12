CPLEXDIR     ?= /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CONCERTDIR   ?= /opt/ibm/ILOG/CPLEX_Studio2211/concert

CXX           = g++
CCFLAGS       = -m64 -O2 -std=c++17 -Wall -fPIC -fexceptions -DNDEBUG -DIL_STD \
                -I$(CPLEXDIR)/include -I$(CONCERTDIR)/include
LDFLAGS       = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic \
                -L$(CONCERTDIR)/lib/x86-64_linux/static_pic \
                -lilocplex -lcplex -lconcert -lm -pthread -ldl

PROGS = exemplo1 racao dieta plantio tintas transporte fluxo escalonamento \
        cobertura mochila clique padroes facilidades frequencia

.PHONY: all clean
all: $(PROGS)

%: %.cpp
	$(CXX) $(CCFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(PROGS)
