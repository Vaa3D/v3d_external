
FLAGS += -O4

all: mytiff.o 

%.o: %.p
	gcc -c $(FLAGS) $*.c

mytiff.o: tiff.io.o tiff.image.o
	ld -r -o mytiff.o tiff.io.o tiff.image.o

tiffconvert: app.convert.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tiffconvert app.convert.c utilities.c mytiff.o

tifftagger: app.tagger.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tifftagger app.tagger.c utilities.c mytiff.o

tiffRGB: app.color.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tiffRGB app.color.c utilities.c mytiff.o

tiffshow: app.show.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o tiffshow app.show.c utilities.c mytiff.o

mrc2tiff: app.mrc2.c utilities.p mytiff.o
	awk -f manager.awk utilities.p >utilities.c
	gcc $(FLAGS) -o mrc2tiff app.mrc2.c utilities.c mytiff.o

clean:
	del *.o $(APPS) mytiff.tar.gz

package:
	tar -zcf mytiff.tar.gz README INDEX *.h *.p app.*.c manager.awk Makefile
