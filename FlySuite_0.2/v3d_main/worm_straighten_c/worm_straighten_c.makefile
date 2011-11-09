# Makefile of the worm_straightening_c project
# by Hanchuan Peng
# 2008-03-05: create this file
# 2008-03-10: add bfs

CC = g++
DEBUG_FLAG = -g -pg       # assign -g for debugging

OBJS = bdb_minus.o \
       mg_utilities.o \
       mg_image_lib.o \
       stackutil.o \
       main_worm_straightener.o \
       mst_prim_c.o \
       bfs_1root.o 

SHARED_FUNC_DIR = ../basic_c_fun/
 
LIBS = -ltiff

worm_straightener : ${OBJS}
	${CC} ${DEBUG_FLAG} ${OBJS} -B ./ ${LIBS} -o $@

main_worm_straightener.o : main_worm_straightener.cpp BDB_minus.h ${SHARED_FUNC_DIR}stackutil.h
	${CC} ${DEBUG_FLAG} -c main_worm_straightener.cpp

bdb_minus.o : bdb_minus.cpp bdb_minus.h ${SHARED_FUNC_DIR}stackutil.h
	${CC} ${DEBUG_FLAG} -c bdb_minus.cpp

mst_prim_c.o : mst_prim_c.cpp mst_prim_c.h graphsupport.h graph.h
	${CC} ${DEBUG_FLAG} -c mst_prim_c.cpp

bfs_1root.o : bfs_1root.cpp bfs.h graphsupport.h graph.h
	${CC} ${DEBUG_FLAG} -c bfs_1root.cpp

stackutil.o : ${SHARED_FUNC_DIR}stackutil.cpp ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${DEBUG_FLAG} -c ${SHARED_FUNC_DIR}stackutil.cpp

mg_utilities.o : ${SHARED_FUNC_DIR}mg_utilities.cpp ${SHARED_FUNC_DIR}mg_utilities.h 
	${CC} ${DEBUG_FLAG} -c ${SHARED_FUNC_DIR}mg_utilities.cpp

mg_image_lib.o : ${SHARED_FUNC_DIR}mg_image_lib.cpp ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${DEBUG_FLAG} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp

clean :
	rm *.o

