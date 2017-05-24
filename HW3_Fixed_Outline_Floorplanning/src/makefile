CC			:= gcc
CXX			:= g++

MPICC			:= mpicc
MPICXX			:= mpicxx

CCFLAGS			:= -O3 -march=native -std=gnu11
CXXFLAGS		:= -O3 -march=native -std=c++0x

PFLAGS			:= -lpthread

all: main

main:
	$(CXX) $(CXXFLAGS) main.cpp -o fp
clean:
	rm -f fp
veryclean:
	rm -f fp *.output