CC = g++ -g
CC_FLAGS += -I../../../v3d_main/basic_c_fun -I../../../v3d_main/common_lib/include
LIBS +=  -L../../../v3d_main/common_lib/lib -lv3dtiff 

OBJS = main.o basic_memory.o stackutil.o mg_image_lib.o mg_utilities.o gaussian_blur.o img_rotate.o
SHARED_FUNC_DIR = ../../../v3d_main/basic_c_fun/

all: v3d_convert
v3d_convert: ${OBJS}
	${CC}  ${CC_FLAGS} ${OBJS} ${LIBS} -o v3d_convert
	./v3d_convert
main.o : parser.h main.cpp v3d_funcs.h
	${CC} ${CC_FLAGS} -c main.cpp
basic_memory.o : ${SHARED_FUNC_DIR}basic_memory.h ${SHARED_FUNC_DIR}basic_memory.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}basic_memory.cpp
stackutil.o : ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}stackutil.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}stackutil.cpp
mg_image_lib.o : ${SHARED_FUNC_DIR}mg_image_lib.h ${SHARED_FUNC_DIR}mg_image_lib.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp
mg_utilities.o : ${SHARED_FUNC_DIR}mg_utilities.h ${SHARED_FUNC_DIR}mg_utilities.cpp
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_utilities.cpp
gaussian_blur.o : gaussian_blur.h gaussian_blur.cpp
	${CC} ${CC_FLAGS} -c gaussian_blur.cpp
img_rotate.o : img_rotate.cpp img_rotate.h
	${CC} ${CC_FLAGS} -c img_rotate.cpp
clean:
	rm *.o
	rm v3d_convert
