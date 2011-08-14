APPS = tiffconvert tifftagger tiffRGB tiffshow mrc2tiff

FLAGS += -O4 -fPIC

all: mytiff.o $(APPS)

%.o: %.p
	#awk -f manager.awk $< > $*.c
	gcc -c $(FLAGS) $*.c
	#rm $*.c

mytiff.o: tiff.io.o tiff.image.o
	ld -r -o mytiff.o tiff.io.o tiff.image.o
	#rm tiff.io.o tiff.image.o

tiffconvert: app.convert.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tiffconvert app.convert.c utilities.c mytiff.o
	#rm utilities.c

tifftagger: app.tagger.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tifftagger app.tagger.c utilities.c mytiff.o
	#rm utilities.c

tiffRGB: app.color.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tiffRGB app.color.c utilities.c mytiff.o
	#rm utilities.c

tiffshow: app.show.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tiffshow app.show.c utilities.c mytiff.o
	#rm utilities.c

mrc2tiff: app.mrc2.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o mrc2tiff app.mrc2.c utilities.c mytiff.o
	#rm utilities.c

clean:
	rm -f *.o $(APPS) mytiff.tar.gz

package:
	tar -zcf mytiff.tar.gz README INDEX *.h *.p app.*.c manager.awk Makefile
