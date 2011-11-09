all: mylib

CFLAGS += -O4 -fPIC

modules = cdf.o fct.min.o linear.algebra.o array.o \
          fct.root.o image.o utilities.o hash.o \
          histogram.o

%.o: %.p
	#awk -f manager.awk $< | awk -f generator.awk > $*.c
	gcc -c $(CFLAGS) $*.c
ifneq ($(CFLAGS),-g)
	#rm $*.c
endif

mylib: $(modules)
	cd MY_TIFF && $(MAKE) -f mytiff_linux.makefile $* && cd ..
	#cd MY_FFT    && $(MAKE) && cd ..
	#cd DOCUMENTS && $(MAKE) && cd ..
	#libtool -static -o libmylib.a $(modules) MY_FFT/myfft.o MY_TIFF/mytiff.o
	#libtool -o libmylib.a $(modules)  MY_TIFF/mytiff.o
	$(AR) -cr libmylib.a  $(modules)  MY_TIFF/mytiff.o
	ranlib libmylib.a

	#rm $(modules)

clean:
	cd MY_TIFF   && $(MAKE) -f mytiff_linux.makefile clean && cd ..
	#cd MY_FFT    && $(MAKE) clean && cd ..
	#cd DOCUMENTS && $(MAKE) clean && cd ..
	rm *.o
	rm -f libmylib.a libmylib64.a  #$(modules:.o=.c) $(modules64:.o=.c)

package:
	cd MY_TIFF   && $(MAKE) -f mytiff_linux.makefile clean && cd ..
	#cd MY_FFT    && $(MAKE) clean && cd ..
	#cd DOCUMENTS && $(MAKE) clean && cd ..
	#tar -czf mylib.tar.gz DOCUMENTS/* MY_TIFF/* MY_FFT/* \
        #                      *.p *.h *.awk Makefile
	tar -czf mylib.tar.gz MY_TIFF/* \
                              *.p *.h *.awk Makefile

