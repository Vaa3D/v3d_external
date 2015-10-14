all: mylib

CFLAGS += -O4

modules = cdf.o fct.min.o linear.algebra.o array.o \
          fct.root.o image.o utilities.o hash.o \
          histogram.o

%.o: %.p
	gcc -c $(CFLAGS) $*.c

mylib: $(modules)
	cd MY_TIFF && $(MAKE) -f mytiff_win.makefile $* && cd ..
	$(AR) -cr libmylib.a  $(modules)  MY_TIFF/mytiff.o
	ranlib libmylib.a

clean:
	cd MY_TIFF   && $(MAKE) -f mytiff_win.makefile clean && cd ..
	del *.o
	del libmylib.a libmylib64.a  #$(modules:.o=.c) $(modules64:.o=.c)

package:
	cd MY_TIFF   && $(MAKE) -f mytiff_win.makefile clean && cd ..
	tar -czf mylib.tar.gz MY_TIFF/* \
                              *.p *.h *.awk Makefile

