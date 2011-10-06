# by Hanchuan Peng
# 2006-2011

# 64bit compilation on Macx
CC_FLAGS += $(subst x, x,$(ARCH_x86_64))    
LDFLAGS += $(subst x, x,$(ARCH_x86_64))
# additional search path
CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH)))

ifneq ($(strip $(ARCH_x86_64)),)
LIBS += -L. -L../../common_lib/lib/ -L$(L_PATH) -lv3dtiff64 -lz -lnewmat
else
LIBS += -L. -L../../common_lib/lib/ -lv3dtiff -lnewmat
endif

INCLUDEDIRS += -I../../basic_c_fun/
INCLUDEDIRS += -I../../common_lib/include/
INCLUDEDIRS += -I../newmat11

CC = g++
CC_FLAGS += -w   # -w for no compiling warning
CC_FLAGS += -g   # assign -g for gdb debugging
CC_FLAGS += $(INCLUDEDIRS)

all: alljba

JBALIB_OBJS = jba_mainfunc.o \
       jba_match_landmarks.o \
       mg_utilities.o \
       mg_image_lib.o \
       stackutil.o wkernel.o \
       histeq.o \
       convert_type2uint8.o \
       jba_affine_xform.o \
       remove_nonaffine_points.o

MAINJBA_OBJ = main_jba.o

SHARED_FUNC_DIR = ../..//basic_c_fun/

%.o:           	%.cpp
		$(CC) $(CC_FLAGS) -c $*.cpp -o $*.o 
newmat_dir = ../newmat11/
newmat_lobj = ${newmat_dir}newmat1.o ${newmat_dir}newmat2.o ${newmat_dir}newmat3.o ${newmat_dir}newmat4.o \
	${newmat_dir}newmat5.o \
	${newmat_dir}newmat6.o \
	${newmat_dir}newmat7.o \
	${newmat_dir}newmat8.o \
	${newmat_dir}newmatex.o \
	${newmat_dir}bandmat.o \
	${newmat_dir}submat.o \
	${newmat_dir}myexcept.o ${newmat_dir}cholesky.o \
	${newmat_dir}evalue.o \
	${newmat_dir}fft.o \
	${newmat_dir}hholder.o \
	${newmat_dir}jacobi.o \
	${newmat_dir}newfft.o \
	${newmat_dir}sort.o \
	${newmat_dir}svd.o \
	${newmat_dir}nm_misc.o \
	${newmat_dir}newmatrm.o \
	${newmat_dir}newmat9.o

.PHONY: alljba
alljba : ${JBALIB_OBJS} $(MAINJBA_OBJ) \
    libnewmat 
	${CC} ${CC_FLAGS} ${JBALIB_OBJS} $(MAINJBA_OBJ)  ${LIBS} -o brainaligner

.PHONY: brainaligner
brainaligner : ${JBALIB_OBJS} $(MAINJBA_OBJ) libnewmat
	${CC} ${CC_FLAGS} ${JBALIB_OBJS} $(MAINJBA_OBJ)  ${LIBS} -o $@

.PHONY: libjba
libjba : $(JBALIB_OBJS)
	$(AR) -cr libjba.a $(JBALIB_OBJS)
	ranlib libjba.a

.PHONY: libnewmat
libnewmat : $(newmat_lobj)
	$(AR) -cr libnewmat.a  $(newmat_lobj)
	ranlib libnewmat.a


#for main JBA program
       
main_jba.o : main_jba.cpp jba_mainfunc.h convert_type2uint8.h ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}volimg_proc.h 
	${CC} ${CC_FLAGS} -c main_jba.cpp

jba_mainfunc.o : jba_mainfunc.cpp jba_mainfunc.h wkernel.h ${SHARED_FUNC_DIR}basic_memory.cpp ${SHARED_FUNC_DIR}basic_memory.h ${SHARED_FUNC_DIR}img_definition.h
	${CC} ${CC_FLAGS} -c jba_mainfunc.cpp

jba_match_landmarks.o : jba_match_landmarks.cpp jba_match_landmarks.h wkernel.h ${SHARED_FUNC_DIR}basic_memory.cpp ${SHARED_FUNC_DIR}basic_memory.h ${SHARED_FUNC_DIR}img_definition.h
	${CC} ${CC_FLAGS} -c jba_match_landmarks.cpp

stackutil.o : ${SHARED_FUNC_DIR}stackutil.cpp ${SHARED_FUNC_DIR}stackutil.h ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}stackutil.cpp

mg_utilities.o : ${SHARED_FUNC_DIR}mg_utilities.cpp ${SHARED_FUNC_DIR}mg_utilities.h 
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_utilities.cpp

mg_image_lib.o : ${SHARED_FUNC_DIR}mg_image_lib.cpp ${SHARED_FUNC_DIR}mg_image_lib.h
	${CC} ${CC_FLAGS} -c ${SHARED_FUNC_DIR}mg_image_lib.cpp
 
wkernel.o : wkernel.cpp wkernel.h
	${CC} ${CC_FLAGS} -c wkernel.cpp

histeq.o : histeq.cpp histeq.h
	${CC} ${CC_FLAGS} -c histeq.cpp

convert_type2uint8.o : convert_type2uint8.h convert_type2uint8.cpp
	${CC} ${CC_FLAGS} -c convert_type2uint8.cpp

jba_affine_xform.o : jba_affine_xform.h jba_affine_xform.cpp
	${CC} ${CC_FLAGS} -c jba_affine_xform.cpp

remove_nonaffine_points.o : remove_nonaffine_points.cpp remove_nonaffine_points.h
	${CC} ${CC_FLAGS} -c remove_nonaffine_points.cpp


.PHONY: clean
clean :
	rm $(newmat_lobj)
	if [ -f libnewmat.a ]; then rm libnewmat.a; fi
	rm ${JBALIB_OBJS} 
	if [ -f  $(MAINJBA_OBJ) ]; then rm  $(MAINJBA_OBJ); fi

