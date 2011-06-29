CC = gcc
CXX = g++ -g -Wall

CXXFLAGS = -ltiff

OBJ_CREATE = ../myalgorithms.o ../component_tree.o  create_tree.o
OBJ_LOAD = ../myalgorithms.o ../component_tree.o  load_tree.o

all: $(OBJ_CREATE) $(OBJ_LOAD)
	$(CXX) $(CXXFLAGS) $(OBJ_CREATE) -o create_tree
	$(CXX) $(CXXFLAGS) $(OBJ_LOAD) -o load_tree

(%.o):(%.cpp)
	$(CXX) $(CXXFLAGS) -c $< -o $*.o

clean:
	rm *.o
