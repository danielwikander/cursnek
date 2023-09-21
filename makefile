_dummy := $(shell mkdir -p bin)

snekplusplus: ./src/cursnek.cpp 
	g++ -g -std=c++11 -o ./bin/cursnek ./src/cursnek.cpp -lncurses -Wall -Wextra

