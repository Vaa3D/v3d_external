/*==============================================================================
The SZIP Science Data Lossless Compression Program is Copyright (C) 2001 Science
& Technology Corporation @ UNM.  All rights released.  Copyright (C) 2003 Lowell
H. Miles and Jack A. Venbrux.  Licensed to ICs Corp. for distribution by the
University of Illinois' National Center for Supercomputing Applications as a
part of the HDF data storage and retrieval file format and software library
products package.  All rights reserved.  Do not modify or use for other
purposes.

SZIP implements an extended Rice adaptive lossless compression algorithm
for sample data.  The primary algorithm was developed by R. F. Rice at
Jet Propulsion Laboratory.

SZIP embodies certain inventions patented by the National Aeronautics &
Space Administration.  United States Patent Nos. 5,448,642, 5,687,255,
and 5,822,457 have been licensed to ICs Corp. for distribution with the
HDF data storage and retrieval file format and software library products.
All rights reserved.

Revocable (in the event of breach by the user or if required by law),
royalty-free, nonexclusive sublicense to use SZIP decompression software
routines and underlying patents is hereby granted by ICs Corp. to all users
of and in conjunction with HDF data storage and retrieval file format and
software library products.

Revocable (in the event of breach by the user or if required by law),
royalty-free, nonexclusive sublicense to use SZIP compression software
routines and underlying patents for non-commercial, scientific use only
is hereby granted by ICs Corp. to users of and in conjunction with HDF
data storage and retrieval file format and software library products.

For commercial use license to SZIP compression software routines and underlying
patents please contact ICs Corp. at ICs Corp., 721 Lochsa Street, Suite 8,
Post Falls, ID 83854.  (208) 262-2008.

==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mcgill.h"
#include "SZconfig.h"
#include "szlib.h"

#define BLOCKS_PER_TEST    1024

#define ARRAY_SIZE (BLOCKS_PER_TEST*4*32*2)

char *bp;

char *image_in;
char *image_in2;
char *image_out;

int msb_first;
int nn_mode;

unsigned *sp;

unsigned xmax;

void
output_pixel(bits_per_pixel, pixel)
int bits_per_pixel;
unsigned pixel;
{
    if (msb_first)
        {
        if (bits_per_pixel <= 8)
            *bp++ = pixel & 0xff;
        else if (bits_per_pixel <= 16)
            {
            *bp++ = (pixel >> 8) & 0xff;
            *bp++ = pixel & 0xff;
            }
        else
            {
            *bp++ = 0;
            *bp++ = (pixel >> 16) & 0xff;
            *bp++ = (pixel >> 8) & 0xff;
            *bp++ = pixel & 0xff;
            }
        }
    else
        {
        if (bits_per_pixel <= 8)
            *bp++ = pixel & 0xff;
        else if (bits_per_pixel <= 16)
            {
            *bp++ = pixel & 0xff;
            *bp++ = (pixel >> 8) & 0xff;
            }
        else
            {
            *bp++ = pixel & 0xff;
            *bp++ = (pixel >> 8) & 0xff;
            *bp++ = (pixel >> 16) & 0xff;
            *bp++ = 0;
            }
        }
}

void
unmap_nn(sigma, pixels, out)
unsigned *sigma;
long pixels;
unsigned *out;
{
    int sig1;
    int sig2;
    int x;
    unsigned *end;
    unsigned *s;

    end = sigma + pixels;
    s = sigma;

    x = *s++;
    *out++ = x;

    sig1 = *s++;
    if (sig1 >= (x << 1))
        x = sig1;
    else if (sig1 > (xmax - x) << 1)
        x = xmax - sig1;
    else if (sig1 & 1)
        x = x - ((sig1 + 1) >> 1);
    else
        x = x + (sig1 >> 1);

    *out++ = x;

    while (s < end)
        {
        sig1 = *s++;
        sig2 = *s++;
        if (sig1 >= (x << 1))
            x = sig1;
        else if (sig1 > (xmax - x) << 1)
            x = xmax - sig1;
        else if (sig1 & 1)
            x = x - ((sig1 + 1) >> 1);
        else
            x = x + (sig1 >> 1);

        *out++ = x;

        if (sig2 >= (x << 1))
            x = sig2;
        else if (sig2 > (xmax - x) << 1)
            x = xmax - sig2;
        else if (sig2 & 1)
            x = x - ((sig2 + 1) >> 1);
        else
            x = x + (sig2 >> 1);

        *out++ = x;
        }
}

void
genblock(sum, j, n)
int sum;
int j;
int n;
{
    double average;
    int k;
    int i;
    long x;
    double v;

    average = sum/(double) j;
    average *= 2;

#if 0
    printf("bp = %x\n", bp);
    printf("image_in + 10140 = %x\n", image_in + 10140);
#endif
#if 0
    if (bp == 0x40032008)
        printf("Here ----------------->\n");
#endif
#if 1
    if (j == 8 && n == 4 && msb_first == 0 && bp == image_in + 190)
        printf("Here ----------------->\n");
#endif

    for (k = 0; k < j; k++)
        {
        v = 0;
        for (i = 0; i < 6; i++)
            v += uni();

        v /= 6.0;

        x = (v * average) + 0.5;
        x &= xmax;
        if (x + (j-k-1) * xmax < sum)
            x = sum - (j-k-1) * xmax;

        if (x > sum)
            x = sum;

        if (x > xmax || x < 0)
            printf("ERROR: %ld > %ud\n", x, xmax);

        *sp++ = x;
#if DEBUG
        printf("%03x ", x);
#endif
        sum -= x;
        }

#if 0
    printf("%d\n", bp - image_in);
#endif

#if DEBUG
    printf("\n");
#endif
}

void
genimage(n, j, blocks)
int n;
int j;
int blocks;
{
    int i;
    int k;
    int sum;
    int sum_array[32];
    unsigned sigma[1024];
    unsigned sigma_out[1024];
    unsigned *send;

    bp = image_in;

    xmax = (1 << n) - 1;

    sum_array[0] = 0;
    sum_array[1] = 3;
    sum_array[2] = j/2 * 3;
    for (i = 3; i < 26; i++)
        sum_array[i] = sum_array[i-1] * 2 + j/2;

    sp = sigma;
    send = sigma + j*16;
    for (k = 0; k < blocks; k++)
        {
        /*** make sum_array[i-1] <= sum <= sum_array[i] ***/
        i = uni() * (n+2);
        if (i == 0)
            sum = 0;
        else
            sum = (sum_array[i] - sum_array[i-1]) * uni() + 1;

#if DEBUG
        printf("sum=%d,i=%d\n", sum, i);
#endif
        if (sum > j * xmax)
            {
            k--;
            continue;
            }

#if 0
        if (k == 24)
            printf("HERE --------------------------->\n");
#endif
#if 0
        if (nn_mode && msb_first && n == 4 && j == 8)
            printf("HERE --------------------------->\n");
#endif

        genblock(sum, j, n);
        if (sp == send)
            {
            if (nn_mode)
                {
                sigma[0] = uni() * (xmax+1);
                unmap_nn(sigma, send-sigma, sigma_out);
                sp = sigma_out;
                send = sigma_out + j*16;
                }
            else
                sp = sigma;

            for (; sp < send; sp++)
                output_pixel(n, *sp);

            sp = sigma;
            send = sigma + j*16;
            }
        }
}

void
gentest(n, j, blocks)
int n;
int j;
int blocks;
{
    SZ_com_t params;
    long image_size;
    int count;
    int i;
    int rv;
    size_t size;
    size_t size2;

    printf("test: mode=%s msb_first=%d n=%02d, j=%02d, blocks=%d -------------------\n", nn_mode ? "nn" : "ec", msb_first, n, j, blocks);

    genimage(n, j, blocks);

    image_size = blocks * j;
    if (n > 16)
        image_size *= 4;
    else if (n > 8)
        image_size *= 2;

    params.options_mask = SZ_RAW_OPTION_MASK | SZ_ALLOW_K13_OPTION_MASK;
    if (msb_first)
        params.options_mask |= SZ_MSB_OPTION_MASK;
    else
        params.options_mask |= SZ_LSB_OPTION_MASK;

    params.bits_per_pixel = n;
    params.pixels_per_block = j;
    params.pixels_per_scanline = 256;

    params.options_mask |= (nn_mode ? SZ_NN_OPTION_MASK : SZ_EC_OPTION_MASK);

    size = ARRAY_SIZE;
    rv = SZ_BufftoBuffCompress(image_out, &size, image_in, image_size, &params);

    if (rv != SZ_OK)
        {
        fprintf(stderr, "SZ_BufftoBuffCompress fails\n");
        exit(1);
        }

    size2 = ARRAY_SIZE;
    rv = SZ_BufftoBuffDecompress(image_in2, &size2, image_out, size, &params);

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
        printf("j=%d n=%d msb_first=%d\n", j, n, msb_first);
        printf("image_size = %ld, image_size2 = %ld\n", image_size, (long) size2);
        count = 0;
        for (i = 0; i < image_size; i++)
            if (image_in[i] != image_in2[i])
                {
                printf("in[%04d] = %02x, in2[%04d] = %02x\n", i, image_in[i], i, image_in2[i]);
                if (++count == 10)
                    break;
                }

        exit(1);
        }
}

void
gentest_odd(n, j, s)
int n;
int j;
int s;
{
    SZ_com_t params;
    char *ip;
    int bytes_per_pixel;
    int count;
    int i;
    int rv;
    long image_size;
    size_t size;
    size_t size2;

    printf("test(odd): mode=%s msb_first=%d n=%02d, j=%02d s=%03d -------------------\n", nn_mode ? "nn" : "ec", msb_first, n, j, s);

    ip = image_in2;
    bytes_per_pixel = (n + 7) >> 3;
    if (bytes_per_pixel == 3)
        bytes_per_pixel = 4;

    for (i = 0; i < 32; i++)
        {
        genimage(n, 16, 16);
        memcpy(ip, image_in, s*bytes_per_pixel);
        ip += s*bytes_per_pixel;
        }

    image_size = s * i * bytes_per_pixel;
    memcpy(image_in, image_in2, image_size);

    params.options_mask = SZ_RAW_OPTION_MASK | SZ_ALLOW_K13_OPTION_MASK;
    if (msb_first)
        params.options_mask |= SZ_MSB_OPTION_MASK;
    else
        params.options_mask |= SZ_LSB_OPTION_MASK;

    params.bits_per_pixel = n;
    params.pixels_per_block = j;
    params.pixels_per_scanline = s;

    params.options_mask |= (nn_mode ? SZ_NN_OPTION_MASK : SZ_EC_OPTION_MASK);

    size = ARRAY_SIZE;
    rv = SZ_BufftoBuffCompress(image_out, &size, image_in, image_size, &params);

    if (rv != SZ_OK)
        {
        fprintf(stderr, "SZ_BufftoBuffCompress fails\n");
        exit(1);
        }

    size2 = ARRAY_SIZE;
    rv = SZ_BufftoBuffDecompress(image_in2, &size2, image_out, size, &params);

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
        printf("j=%d n=%d msb_first=%d\n", j, n, msb_first);
        printf("image_size = %ld, image_size2 = %ld\n", image_size, (long) size2);
        count = 0;
        for (i = 0; i < image_size; i++)
            if (image_in[i] != image_in2[i])
                {
                printf("in[%04d] = %02x, in2[%04d] = %02x\n", i, image_in[i], i, image_in2[i]);
                if (++count == 10)
                    break;
                }

        exit(1);
        }
}

void
gentest3264(blocks, n, j)
int blocks;
int n;
int j;
{
    SZ_com_t params;
    long image_size;
    int bytes_per_pixel;
    int count;
    int i;
    int rv;
    size_t size;
    size_t size2;

    printf("test: mode=nn n=%d, j=%02d, blocks=%d -------------------\n", n, j, blocks);

    bytes_per_pixel = n >> 3;
    count = 0;
    image_size = blocks * j * bytes_per_pixel;
    for (i = 0; i < image_size; i++)
        {
        image_in[i] = count + (count >> 8) ;
        count++;
        }

    params.options_mask = SZ_RAW_OPTION_MASK | SZ_ALLOW_K13_OPTION_MASK;
    params.bits_per_pixel = n;
    params.pixels_per_block = j;
    params.pixels_per_scanline = 256;

    params.options_mask |= SZ_NN_OPTION_MASK;

    size = ARRAY_SIZE;
    rv = SZ_BufftoBuffCompress(image_out, &size, image_in, image_size, &params);

    if (rv != SZ_OK)
        {
        fprintf(stderr, "SZ_BufftoBuffCompress fails\n");
        exit(1);
        }

    size2 = ARRAY_SIZE;
    rv = SZ_BufftoBuffDecompress(image_in2, &size2, image_out, size, &params);

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
        printf("j=%d n=%d msb_first=%d\n", j, n, msb_first);
        printf("image_size = %ld, image_size2 = %ld\n", image_size, (long) size2);
        count = 0;
        for (i = 0; i < image_size; i++)
            if (image_in[i] != image_in2[i])
                {
                printf("in[%04d] = %02x, in2[%04d] = %02x\n", i, image_in[i], i, image_in2[i]);
                if (++count == 10)
                    break;
                }

        exit(1);
        }
}

int
main(argc, argv)
int argc;
char **argv;
{
    int j;
    int n;
    long seed;
/* Run tests only if encoding is enabled */
if(SZ_encoder_enabled()) {
    image_in  = (char *) malloc(ARRAY_SIZE);
    image_in2 = (char *) malloc(ARRAY_SIZE);
    image_out = (char *) malloc(ARRAY_SIZE);
    if (image_in == 0 || image_in2 == 0 || image_out == 0)
        {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
        }

    seed = time(0);
#if 0
    seed = 1050682991;
#endif
    printf("Seed = %ld\n", seed);
    rstart(-seed, seed);

    /*** Test the umap_nn function for odd scanline length ***/
    nn_mode = 1;
    for (msb_first = 0; msb_first < 2; msb_first++)
        for (n = 8; n <= 24; n += 8)
            gentest_odd(n, 8, 101);

    /*** We only need to test the interleaving and deinterleaving ***/
    /*** for float and double types.                              ***/
    gentest3264(BLOCKS_PER_TEST, 32, 8);
    gentest3264(BLOCKS_PER_TEST, 64, 8);

    for (nn_mode = 0; nn_mode < 2; nn_mode++)
        for (msb_first = 0; msb_first < 2; msb_first++)
            for (j = 8; j <= 32; j += 2)
                for (n = 4; n <= 24; n++)
                    gentest(n, j, BLOCKS_PER_TEST);

    printf("All tests passed.\n");

    return 0;
} /*endif encoding_enabled */
        printf("Encoding is disabled...quitting...\n");
        return 1;
}

