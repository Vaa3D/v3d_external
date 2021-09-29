#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "mcgill.h"

#define eq(a, b) (!strcmp((a), (b)))

#define FALSE	0
#define TRUE	1

#define EC_MODE	1
#define NN_MODE	2

#if !defined(__MWERKS__) 
typedef int boolean;
#endif

boolean msb_first = TRUE;
boolean partial_last_scanline;
int compression_mode = NN_MODE;

FILE *fp_out;

int bits;
int max;

double exponent;
double mult = 1.0;
double offset;
double sine_mult;

void
put_pixel(pixel)
unsigned pixel;
{
	int ch;

	if (pixel > max)
		{
		fprintf(stderr, "ERROR: pixel value too large: %02x\n", pixel);
		exit(1);
		}

	if (bits > 16)
		if (msb_first)
			{
			ch = (pixel >> 24) & 0xff;
			putc(ch, fp_out);
			ch = (pixel >> 16) & 0xff;
			putc(ch, fp_out);
			ch = (pixel >> 8)  & 0xff;
			putc(ch, fp_out);
			ch = pixel & 0xff;
			putc(ch, fp_out);
			}
		else
			{
			ch = pixel & 0xff;
			putc(ch, fp_out);
			ch = (pixel >> 8)  & 0xff;
			putc(ch, fp_out);
			ch = (pixel >> 16) & 0xff;
			putc(ch, fp_out);
			ch = (pixel >> 24) & 0xff;
			putc(ch, fp_out);
			}
	else if (bits > 8)
		if (msb_first)
			{
			ch = pixel >> 8;
			putc(ch, fp_out);
			ch = pixel & 0xff;
			putc(ch, fp_out);
			}
		else
			{
			ch = pixel & 0xff;
			putc(ch, fp_out);
			ch = pixel >> 8;
			putc(ch, fp_out);
			}
	else
		putc(pixel, fp_out);
}

void
map_nn(sigma, end)
unsigned *sigma;
unsigned *end;
{
	int del;
	int x;
	int xp;
	unsigned *s;

	xp = *sigma;
	s = sigma + 1;
	while (s < end)
		{
	x = *s;
	del = x-xp;
	if (del >= 0)
		{
		if (del <= xp)
			*s++ = del << 1;
		else
			*s++ = x;
			}
		else
			{
			if (del >= (xp-max))
				*s++ = ((-del)<<1) - 1;
			else 
				*s++ = max-x;
			}

		xp = x;
		}
}

void
random_scanline(pixels_per_block, width)
int pixels_per_block;
int width;
{
	unsigned *end;
	unsigned *s;
	unsigned scanline[8*1024];
	int blocks;
	int delta;
	int high;
	int i;
	int low;
	int pixel;
	int x;

	if (width > sizeof(scanline)/sizeof(unsigned))
		{
		fprintf(stderr, "Maximum scanline width exceeded.\n");
		exit(1);
		}

	end = scanline + width;
	s = scanline;
	while (s < end)
		{
		x = uni() * 6;
		switch (x)
			{
			case 0:
				/*** Random block ***/
				for (i = 0; i < pixels_per_block; i++)
					{
					if (s >= end)
						continue;
					
					pixel = uni() * (max+1);
					*s++ = pixel;
					}
				break;

			case 1:
				/*** block where low <= pixel[i] <= high ***/
				pixel = uni() * (max+1);
				delta = uni() * 32;
				low = pixel - delta;
				if (low < 0)
					low = 0;

				high = low + delta;
				if (high > max)
					high = max;

				delta = high - low + 1;
				for (i = 0; i < pixels_per_block; i++)
					{
					if (s >= end)
						continue;
					
					pixel = low + uni() * delta;
					*s++ = pixel;
					}
				break;

			case 2:
				/*** block where | pixel[i] - pixel[i+1] | <= delta ***/
				pixel = uni() * (max+1);
				delta = uni() * 128 + 1;
				low = pixel - delta;
				if (low < 0)
					low = 0;

				high = low + delta;
				if (high > max)
					high = max;

				delta = high - low + 1;
				for (i = 0; i < pixels_per_block; i++)
					{
					if (s >= end)
						continue;
						
					x = uni() * delta;
					pixel += x - delta/2;
					if (pixel > max)
						pixel = max;
					else if (pixel < 0)
						pixel = 0;
						
					*s++ = pixel;
					}
				break;

			case 3:
				/*** Same till end of line ***/
				pixel = uni() * (max+1);
				while (s < end)
					*s++ = pixel;
				break;

			case 4:
				/*** Same till end of line ***/
				blocks = ((end - s) + pixels_per_block - 1)/pixels_per_block;
				blocks = uni() * blocks + 1;
				pixel = uni() * (max+1);
				for (i = 0; i < blocks * pixels_per_block; i++)
					if (s < end)
						*s++ = pixel;
				break;

			case 5:
				/*** low entropy block ***/
				/*** where | pixel[i] - pixel[i+1] | <= delta ***/
				pixel = uni() * (max+1);
				delta = uni() * 8 + 1;
				low = pixel - delta;
				if (low < 0)
					low = 0;

				high = low + delta;
				if (high > max)
					high = max;

				delta = high - low + 1;
				for (i = 0; i < pixels_per_block; i++)
					{
					if (s >= end)
						continue;
					
					x = uni() * delta;
					pixel += x - delta/2;
					if (pixel > max)
						pixel = max;
					else if (pixel < 0)
						pixel = 0;
							
					*s++ = pixel;
					}
				break;
			}
		}

	if (compression_mode == EC_MODE)
		map_nn(scanline, scanline + width);

	s = scanline;
	while (s < end)
		put_pixel(*s++);
}

void
random_float_scanline(pixels_per_block, width)
int pixels_per_block;
int width;
{
	float scanline[8*1024];
	double start;
	double step;
	double x;
	double y;
	int i;

	if (width > sizeof(scanline)/sizeof(float))
		{
		fprintf(stderr, "Maximum scanline width exceeded.\n");
		exit(1);
		}

	start = uni();
	step = uni()/width;
#if 0
	printf("start= %10.7lf\n", start);
	printf("step = %10.7lf\n", step);
#endif

	x = start;
	for (i = 0; i < width; i++) 
		{
		y = sin(x) * sine_mult + offset;
		scanline[i] = y;
		x += step;
		}

	fwrite(scanline, width, sizeof(float), fp_out);
}

void
random_double_scanline(pixels_per_block, width)
int pixels_per_block;
int width;
{
	double scanline[8*1024];
	double start;
	double step;
	double x;
	double y;
	int i;

	if (width > sizeof(scanline)/sizeof(float))
		{
		fprintf(stderr, "Maximum scanline width exceeded.\n");
		exit(1);
		}

	for (i = 0; i < 10; i++)
		uni();

	start = uni();
	step = uni()/width;
#if 0
	printf("start= %10.7lf\n", start);
	printf("step = %10.7lf\n", step);
#endif

	x = start;
	for (i = 0; i < width; i++) 
		{
		y = sin(x) * sine_mult + offset;
		scanline[i] = y;
		x += step;
#if 0
		printf("%16.13lf\n", y);
#endif
		}

	fwrite(scanline, width, sizeof(double), fp_out);
}

void
usage(name)
char *name;
{
	fprintf(stderr, "Usage: %s [options] bits j lines width output-file\n", name);
	exit(1);
}

int
main(argc, argv)
int argc;
char **argv;
{
	char *output_file;
	int i;
	int l;
	int lines;
	int pixels_per_block;
	int width;

	for (i = 1; i < argc; i++)
		{
		if (*argv[i] == '-')
			{
			if (eq(argv[i], "-chip"))
				;
			else if (eq(argv[i], "-ec"))
				compression_mode = EC_MODE;
			else if (eq(argv[i], "-exp"))
				{
				i++;
				exponent = atoi(argv[i]);
				}
			else if (eq(argv[i], "-ki"))
				;
			else if (eq(argv[i], "-kz"))
				;
			else if (eq(argv[i], "-lsb"))
				msb_first = FALSE;
			else if (eq(argv[i], "-msb"))
				msb_first = TRUE;
			else if (eq(argv[i], "-mult"))
				{
				i++;
				mult = atoi(argv[i]);
				}
			else if (eq(argv[i], "-nn"))
				compression_mode = NN_MODE;
			else if (eq(argv[i], "-p"))
				partial_last_scanline = TRUE;
			else if (eq(argv[i], "-raw"))
				;
			else if (eq(argv[i], "-v"))
				;
			else if (eq(argv[i], "-V"))
				;
			else
				{
				usage(argv[0]);
				exit(1);
				}
			}
		else
			break;
		}

	if (argc - i != 5)
		{
		usage(argv[0]);
		exit(1);
		}

	printf("bits=%s\n", argv[i]);
	bits  = atoi(argv[i+0]);
	pixels_per_block  = atoi(argv[i+1]);
	lines = atoi(argv[i+2]);
	width = atoi(argv[i+3]);
	output_file = argv[i+4];

	if ((fp_out = fopen(output_file, "wb")) == 0)
		{
		fprintf(stderr, "Could not open output file: %s\n", output_file);
		exit(1);
		}

	if (bits == 32 || bits == 64)
		{
		sine_mult = pow(2.0, exponent+mult);
		offset = pow(2.0, exponent);
		printf("offset=%g sine_mult=%g\n", offset, sine_mult);
		}

	rstart(-time(0), time(0));

	max = (1 << bits) - 1; 
	for (l = 0; l < lines; l++)
		{
		if (partial_last_scanline && l == lines-1)
			width = uni() * (width-1) + 1;

		if (bits == 32)
			random_float_scanline(pixels_per_block, width);
		else if (bits == 64)
			random_double_scanline(pixels_per_block, width);
		else
			random_scanline(pixels_per_block, width);
		}

	return 0;
}
