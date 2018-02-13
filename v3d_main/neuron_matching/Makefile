# Makefile of tree matching
# by Fuhui Long
# 20090111

debug: all
release: all

# 2008-09-20: for msys/wingw port, by RZC 
CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH))) #
#CC_FLAGS += -I/usr/local/include -L/usr/local/lib #

CC = g++
CC_FLAGS += -w   # -w for no compiling warning
CC_FLAGS += -g   # assign -g for gdb debugging
LIBS += -ltiff -lnewmat

TREEMATCHLIB_OBJS = FL_treeMatching.o FL_swcTree.o FL_registerAffine.o

MAIN_OBJ = FL_main_treeMatching.o

SHARED_FUNC_DIR1 = ../cellseg/local_basic_c_fun/
SHARED_FUNC_DIR2 = ../cellseg/
SHARED_FUNC_DIR3 = ../basic_c_fun/
SHARED_FUNC_DIR4 = ../atlas_builder/
SHARED_FUNC_DIR5 = ./combination/

.PHONY: all
all : $(MAIN_OBJ) lib1
	${CC} ${CC_FLAGS} ${TREEMATCHLIB_OBJS} $(MAIN_OBJ)  -B ../jba/c++/ ${LIBS} -o treematching

.PHONY: lib1
lib1 : ${TREEMATCHLIB_OBJS}
	$(AR) -cr libFL_treeMatching.a ${TREEMATCHLIB_OBJS}
	ranlib libFL_treeMatching.a

#FL_main_treeMatching.o : FL_readWriteTxtFile.cpp FL_treeMatching.h FL_main_treeMatching.cpp
#	${CC} ${CC_FLAGS} -c FL_main_treeMatching.cpp

FL_main_treeMatching.o : FL_treeMatching.h FL_main_treeMatching.cpp
	${CC} ${CC_FLAGS} -c FL_main_treeMatching.cpp

FL_treeMatching.o : FL_treeMatching.h mymatrix.cpp munkres.cpp mymatrix.h munkres.h ${SHARED_FUNC_DIR5}combination.h FL_treeMatching.cpp
	${CC} ${CC_FLAGS} -c FL_treeMatching.cpp

FL_registerAffine.o: ${SHARED_FUNC_DIR4}FL_registerAffine.h ${SHARED_FUNC_DIR4}FL_registerAffine.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR4}FL_registerAffine.cpp

FL_swcTree.o: FL_swcTree.h ${SHARED_FUNC_DIR3}basic_memory.h ${SHARED_FUNC_DIR3}basic_memory.cpp ${SHARED_FUNC_DIR2}FL_sort.h ${SHARED_FUNC_DIR2}nrutil.h ${SHARED_FUNC_DIR2}nrutil.cpp FL_swcTree.cpp
	${CC} ${CC_FLAGS} -c FL_swcTree.cpp

clean :
	rm *.o
	rm libFL_treeMatching.a
	rm treematching

