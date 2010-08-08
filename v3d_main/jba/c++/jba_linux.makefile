# by Hanchuan Peng
# 2008-08-17: rename laff to jba and start the simplified JBA project
# 2008-08-17: create a library function as well
#             Usage: just type "make -f jba.makefile" or "make all -f jba.makefile"
#                    will create all both the JBA objects file as well as the newmat library
# 2008-08-18: change the JBA dir structure, add matlab and c++ directories
# 2008-10-06: try 64-bit compilation
# 2008-11-11: add CONVERT_TYPE2UINT8_H
# 2008-11-27: add jba_match_landmarks.cpp/h and remove laff_features.cpp
# 2008-12-02: merge the jba_simple_tool and jba_ano makefiles 
# 2009-01-13: now add jba_anno_FL.cpp into the project
# 2009-03-11: add jba_convert2uint8.cpp
# 2009-03-18: add affine parameter estmation fro matching landmarks
# 2009-03-20: add jba_ann_analysis.cpp into the project
# 2009-05-11: add jba_seg_vnc into project
# 2009-05-29: add displecefield_comput.cpp into project
# 2008-09-20: for msys/mingw port, by RZC 
# 2009-06-30: for 64-bit compilation on Mac, by YY
# 2009-10-13: add resampling subproject
# 2010-01-14: add channel swap command

# 64bit compilation on Macx
#CC_FLAGS += -arch x86_64
#LDFLAGS += -arch x86_64
CC_FLAGS += $(subst x, x,$(ARCH_x86_64))    
LDFLAGS += $(subst x, x,$(ARCH_x86_64))
# additional search path
CC_FLAGS += $(patsubst %,-I%,$(subst ;, ,$(VPATH))) $(patsubst %,-L%,$(subst ;, ,$(VPATH)))


ifneq ($(strip $(ARCH_x86_64)),)
#LIBS = $(patsubst -ltiff,-L$(L_PATH) -ltiff64,$(LIBS))
LIBS += -L. -L$(L_PATH) -ltiff64 -lz -lnewmat
else
LIBS += -L. -ltiff -lnewmat
endif

CC = g++
CC_FLAGS += -w   # -w for no compiling warning
CC_FLAGS += -g   # assign -g for gdb debugging


all: libnewmat


%.o:           	%.cpp
		$(CC) $(CC_FLAGS) -c $*.cpp -o $*.o 
### 090630 RZC: add -o $*.o		
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

#.PHONY: libjba.a
#libjba.a : $(JBALIB_OBJS)
#	$(AR) -cr $@ $(JBALIB_OBJS)
#	ranlib $@

.PHONY: libnewmat
libnewmat : $(newmat_lobj)
	$(AR) -cr libv3dnewmat.a  $(newmat_lobj)
	ranlib libv3dnewmat.a

#.PHONY: libnewmat.a
#libnewmat.a:    $(newmat_lobj)
#	$(AR) -cr $@ $(newmat_lobj)
#	ranlib $@
	
	
.PHONY: clean
clean :
	rm $(newmat_lobj)
	if [ -f libv3dnewmat.a ]; then rm libv3dnewmat.a; fi

