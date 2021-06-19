# by Hanchuan Peng
# 2006-2011

# 64bit compilation on Macx
CC_FLAGS += $(subst x, x,$(ARCH_x86_64))    
LDFLAGS += $(subst x, x,$(ARCH_x86_64))
# additional search path
CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH)))

ifneq ($(strip $(ARCH_x86_64)),)
#LIBS += -L. -L../../common_lib/lib/ -L$(L_PATH) -lv3dtiff64 -lz -lv3dnewmat
LIBS += -L. -L../../common_lib/lib/ -L$(L_PATH) -lv3dtiff -lz -lv3dnewmat
else
LIBS += -L. -L../../common_lib/lib/ -lv3dtiff -lv3dnewmat
endif

INCLUDEDIRS += -I../../basic_c_fun/
INCLUDEDIRS += -I../../common_lib/include/
INCLUDEDIRS += -I../newmat11

CC = g++
CC_FLAGS += -w   # -w for no compiling warning
CC_FLAGS += -g  #-g -fPIC   # assign -g for gdb debugging
CC_FLAGS += -fPIC   
CC_FLAGS += $(INCLUDEDIRS)

all: libv3dnewmat

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


.PHONY: libv3dnewmat
libv3dnewmat : $(newmat_lobj)
	$(AR) -cr libv3dnewmat.a  $(newmat_lobj)
	ranlib libv3dnewmat.a



.PHONY: clean
clean :
	rm $(newmat_lobj)
	if [ -f libv3dnewmat.a ]; then rm libv3dnewmat.a; fi