CC = g++
CFLAGS =  -Iinclude -O3 -Wall -g -march=native -std=c++0x
LDFLAGS = -Llib  -lDetailPlace -lGlobalPlace -lLegalizer -lPlacement -lParser -lPlaceCommon

OBJECTS = GlobalPlacer.o  main.o ExampleFunction.o
EXECUTABLE = place

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

GlobalPlacer.o: GlobalPlacer.cpp include/GlobalPlacer.h \
 include/Placement.h include/Module.h include/Pin.h include/Rectangle.h \
 include/Net.h include/Row.h include/ExampleFunction.h \
 include/NumericalOptimizerInterface.h include/NumericalOptimizer.h
	$(CC) $(CFLAGS) -c  GlobalPlacer.cpp

ExampleFunction.o: ExampleFunction.cpp include/ExampleFunction.h \
 include/NumericalOptimizerInterface.h
	$(CC) $(CFLAGS) -c  ExampleFunction.cpp

main.o: main.cpp include/Placement.h include/Module.h include/Pin.h \
 include/Rectangle.h include/Net.h include/Row.h include/Util.h \
  include/GlobalPlacer.h include/Placement.h include/arghandler.h \
   include/DPlace.h include/TetrisLegal.h include/Util.h \
    include/ParamPlacement.h include/arghandler.h
	$(CC) $(CFLAGS) -c  main.cpp

clean:
	rm -f *.o $(EXECUTABLE)


example:	example.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
