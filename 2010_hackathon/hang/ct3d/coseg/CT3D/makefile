CC = gcc
CXX = g++ -g -Wall

CXXFLAGS = -ltiff  -llpsolve55

OBJ = cell_track.o ../myalgorithms.o ../component_tree.o  main.o

CT3D: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o CT3D

(%.o):(%.cpp)
	$(CXX) $(CXXFLAGS) -c $< -o $*.o

clean:
	rm *.o
