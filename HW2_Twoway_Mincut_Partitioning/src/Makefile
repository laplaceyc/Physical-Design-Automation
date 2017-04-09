CC			:= gcc
CXX			:= g++

MPICC			:= mpicc
MPICXX			:= mpicxx

CCFLAGS			:= -O3 -march=native -Wall -std=gnu11
CXXFLAGS		:= -O3 -march=native -Wall -std=c++0x

PFLAGS			:= -lpthread

all: main

main: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $?
clean:
	rm -f main
veryclean:
	rm -f main *.out