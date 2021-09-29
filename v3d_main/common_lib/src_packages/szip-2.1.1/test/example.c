/* example.c -- usage example of the szlib compression library
 * This code is loosely based on that in the zlib.
 * For conditions of distribution and use, see copyright notice in szlib.h 
 */

/* @(#) $Id$ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SZconfig.h"
#include "szlib.h"

#define MAX_IMAGE_SIZE (1024*1024L)

#if 0
const char hello[] = "AAAAAAAAAAAAAAAA";
#endif

const char hello[] = "A 16 byte line!!";

char *image_in;
char *image_in2;
char *image_out;

long test_decoding(int bits_per_pixel, char *in, long size, char *out, long out_size, long buffer_size);
long test_encoding(int bits_per_pixel, char *in, long size, char *out, long buffer_size);

long
read_image(file_name)
char *file_name;
{
	FILE *fp;
	int n;
	long size;

	if ((fp = fopen(file_name, "rb")) == 0)
		{
		fprintf(stderr, "Could not open input file: %s\n", file_name);
		exit(1);
		}

	size = 0;
	while (1)
		{
		n = fread(image_in+size, 1, 16*1024, fp);
		if (n == 0)
			break;

		size += n;
		}

	fprintf(stderr, "read_image(%s): size=%ld\n", file_name, size);
	fclose(fp);

	return size;
}

long
test_encoding(bits_per_pixel, in, size, out, buffer_size)
int bits_per_pixel;
char *in;
long size;
char *out;
long buffer_size;
{
	sz_stream c_stream;
	int bytes_per_pixel;
	int err;
	int len;

	c_stream.hidden = 0;

	c_stream.options_mask = SZ_RAW_OPTION_MASK | SZ_NN_OPTION_MASK | SZ_MSB_OPTION_MASK;
	c_stream.bits_per_pixel = bits_per_pixel;
	c_stream.pixels_per_block = 8;
	c_stream.pixels_per_scanline = 16;

	bytes_per_pixel = (bits_per_pixel + 7)/8;
	if (bytes_per_pixel == 3)
		bytes_per_pixel = 4;

	c_stream.image_pixels = size/bytes_per_pixel;

	len = size;

	c_stream.next_in  = image_in;
	c_stream.total_in = 0;

	c_stream.next_out = out;
	c_stream.total_out = 0;

	err = SZ_CompressInit(&c_stream);
	if (err != SZ_OK)
		{
		fprintf(stderr, "SZ_CompressInit error: %d\n", err);
		exit(1);
		}

	while (c_stream.total_in < len)
		{
		c_stream.avail_in = c_stream.avail_out = buffer_size;
		if (c_stream.avail_in + c_stream.total_in > len)
			c_stream.avail_in = len - c_stream.total_in;

		err = SZ_Compress(&c_stream, SZ_NO_FLUSH);
		if (err != SZ_OK)
			{
			fprintf(stderr, "SZ_Compress error: %d\n", err);
			exit(1);
			}
		}

	for (;;)
		{
		c_stream.avail_out = buffer_size;
		err = SZ_Compress(&c_stream, SZ_FINISH);
#if 0
		printf("output byte=%02X\n", c_stream.next_out[-1]);
#endif
		if (err == SZ_STREAM_END)
			break;

		if (err != SZ_OK)
			{
			fprintf(stderr, "SZ_Compress error: %d\n", err);
			exit(1);
			}
		}

	err = SZ_CompressEnd(&c_stream);
	if (err != SZ_OK)
		{
		fprintf(stderr, "SZ_CompressEnd error: %d\n", err);
		exit(1);
		}

	{
	int i;

	if (c_stream.total_out < 30)
		{
		printf("total_out=%ld\n", c_stream.total_out);
		for (i = 0; i < c_stream.total_out; i++)
			printf("%02X", out[i]);

		printf("\n");
		}
	}

	return c_stream.total_out;
}

long
test_decoding(bits_per_pixel, in, size, out, out_size, buffer_size)
int bits_per_pixel;
char *in;
long size;
char *out;
long out_size;
long buffer_size;
{
	int bytes_per_pixel;
	int err;
	sz_stream d_stream;

	strcpy((char*)out, "garbage");

	d_stream.hidden = 0;

	d_stream.next_in  = in;
	d_stream.next_out = out;

	d_stream.avail_in = 0;
	d_stream.avail_out = 0;

	d_stream.total_in = 0;
	d_stream.total_out = 0;

	d_stream.options_mask = SZ_RAW_OPTION_MASK | SZ_NN_OPTION_MASK | SZ_MSB_OPTION_MASK;
	d_stream.bits_per_pixel = bits_per_pixel;
	d_stream.pixels_per_block = 8;
	d_stream.pixels_per_scanline = 16;

	bytes_per_pixel = (bits_per_pixel + 7)/8;
	if (bytes_per_pixel == 3)
		bytes_per_pixel = 4;

	d_stream.image_pixels = out_size/bytes_per_pixel;

	err = SZ_DecompressInit(&d_stream);
	if (err != SZ_OK)
		{
		fprintf(stderr, "SZ_DecompressEnd error: %d\n", err);
		exit(1);
		}

	while (d_stream.total_in < size)
		{
		d_stream.avail_in = d_stream.avail_out = buffer_size;
		if (d_stream.avail_in + d_stream.total_in > size)
			d_stream.avail_in = size - d_stream.total_in;

		err = SZ_Decompress(&d_stream, SZ_NO_FLUSH);
		if (err == SZ_STREAM_END)
			break;

		if (err != SZ_OK)
			{
			fprintf(stderr, "SZ_Decompress error: %d\n", err);
			exit(1);
			}
		}

	while (d_stream.total_out < out_size)
		{
		d_stream.avail_out = buffer_size;
		err = SZ_Decompress(&d_stream, SZ_FINISH);
		if (err == SZ_STREAM_END)
			break;

		if (err != SZ_OK)
			{
			fprintf(stderr, "SZ_Decompress error: %d\n", err);
			exit(1);
			}
		}

	err = SZ_DecompressEnd(&d_stream);
	if (err != SZ_OK)
		{
		fprintf(stderr, "SZ_DecompressEnd error: %d\n", err);
		exit(1);
		}

	return d_stream.total_out;
}

/**********************************************/
/*** Usage: example [bits_per_pixel]        ***/
/*** default bits_per_pixel = 8             ***/ 
/**********************************************/

/****************************************** **************/
/*** To generate random image file, run the following: ***/
/*** burst_szip -msb # 8 1024 16 image.#.in            ***/
/*** Where # is the bits_per_pixel value               ***/
/*********************************************************/

int
main(argc, argv)
int argc;
char *argv[];
{
	char *compr;
	char file_name[256];
	char *uncompr;
	int bits_per_pixel;
	long comprLen = 10000*sizeof(int);
	long uncomprLen = comprLen;
	size_t size;
	size_t size2;
	long image_size;
	int rv;
	int i;
	SZ_com_t params;

	if (argc == 1)
		bits_per_pixel = 8;
	else if (argc == 2)
		bits_per_pixel = atoi(argv[1]);

	sprintf(file_name, "image.%d.in", bits_per_pixel);

	compr    = (char *) calloc(comprLen, 1);
	uncompr  = (char *) calloc(uncomprLen, 1);
	if (compr == SZ_NULL || uncompr == SZ_NULL)
		{
		fprintf(stderr, "out of memory\n");
		exit(1);
	    }

	image_in  = (char *) malloc(MAX_IMAGE_SIZE);
	image_out = (char *) malloc(MAX_IMAGE_SIZE*2);
	image_in2 = (char *) malloc(MAX_IMAGE_SIZE);

	strcpy(image_out, "Junk!!!");
	strcpy(image_in2, "Junk!!!");

	image_size = read_image(file_name);
	if (image_size > MAX_IMAGE_SIZE)
		{
		fprintf(stderr, "MAX_IMAGE_SIZE of %ld exceeded.\n", MAX_IMAGE_SIZE);
		exit(1);
		}

	printf("Image size %ld \n", image_size);

	/*** test with power of two buffer sizes ***/
	for (i = 1; i < 1025; i <<= 1)
		{
		strcpy(image_out, "Junk!!!");
		strcpy(image_in2, "Junk!!!");

		printf("Testing buffer size = %d\n", i);

	    size = test_encoding(bits_per_pixel, image_in, image_size, image_out, i);
		size = test_decoding(bits_per_pixel, image_out, size, image_in2, image_size, i);
#if 0
		image_in2[1111] ^= 0x44;
#endif
		rv = memcmp(image_in, image_in2, image_size);
#if 0
		printf("memcmp(image_in, image_in2, %ld) = %d\n", image_size, rv);
#endif
		if (rv)
			exit(1);

		params.options_mask = SZ_RAW_OPTION_MASK | SZ_NN_OPTION_MASK | SZ_MSB_OPTION_MASK;
		params.bits_per_pixel = bits_per_pixel;
		params.pixels_per_block = 8;
		params.pixels_per_scanline = 16;
		
		size = 2*MAX_IMAGE_SIZE;
#if 0
		size = 8000;
#endif
		rv = SZ_BufftoBuffCompress(image_out, &size, image_in, image_size, &params); 
#if 0
		printf("SZ_bufftoBuffCompress()\n");
#endif
		if (rv != SZ_OK)
			{
			fprintf(stderr, "SZ_BufftoBuffCompress fails\n");
			exit(1);
			}

		size2 = MAX_IMAGE_SIZE;
#if 0
		size2 = 14000;
#endif
		rv = SZ_BufftoBuffDecompress(image_in2, &size2, image_out, size, &params); 
#if 0
		printf("SZ_bufftoBuffDecompress()\n");
#endif
		if (rv != SZ_OK)
			{
			fprintf(stderr, "SZ_BufftoBuffDecompress fails\n");
			exit(1);
			}

		rv = memcmp(image_in, image_in2, image_size);
		printf("memcmp(image_in, image_in2, %ld) = %d\n", image_size, rv);
		if (rv)
			{
			printf("Test failed.\n");
			exit(1);
			}
		}

	printf("All test passed.\n");
	return 0;
}
