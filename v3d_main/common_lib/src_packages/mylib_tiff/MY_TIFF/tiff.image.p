/*****************************************************************************************\
*                                                                                         *
*  Tiff Image Coder and Decoder (Tiff 6.0)                                                *
*    The module allows one to extract the images in a tiff IFD into Tiff_Image that is    *
*    eseentially a set of arrays, one per channel (sample).  The module also allows you   *
*    to build a Tiff_Image and convert it back to a tiff IFD.                             *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  February 2008                                                                 *
*                                                                                         *
*  (c) July 27, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "tiff.io.h"
#include "tiff.image.h"

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;

#define X_BUFFER_LEN  0x100000    //  code/decode buffer size, must be a power of 2 >= 32
#define X_BUFFER_BITS 0x800000    //  must be 8 times X_BUFFER_LEN

//  These routines are privately shared between tiff.io and tiff.image

extern FILE *Get_Tiff_IFD_Stream(Tiff_IFD *ifd);
extern void *Get_Tiff_IFD_Image(Tiff_IFD *ifd);
extern int   Get_Tiff_IFD_Data_Flip(Tiff_IFD *ifd);

#undef  DEBUG_ENCODE
#undef  DEBUG_DECODE

typedef struct                       //  Hidden declaration for a Tiff_Image
  { unsigned int    width;
    unsigned int    height;
    int             number_channels;
    Tiff_Channel  **channels;
    unsigned short *map;
    Tiff_IFD       *ifd_ref;         //  *Hidden* reference to IFD of an image (if loaded)
  } Timage;


/****************************************************************************************
 *                                                                                      *
 *  ARRAY SPACE MANAGEMENT ROUTINES                                                     *
 *                                                                                      *
 ****************************************************************************************/

#define ESTRING Tiff_Error_String()
#define ESOURCE Tiff_Error_Source()

static jmp_buf ExceptionJump;

#define EXCEPTION   setjmp(ExceptionJump)
#define CATCH(pred) { if (pred) longjmp(ExceptionJump,1); }

static void *Guarded_Realloc(void *p, uint64 size, char *routine)
{ p = realloc(p,size);
  if (p == NULL)
    { sprintf(ESTRING,"Out of memory");
      longjmp(ExceptionJump,1);
    }
  return (p);
}

static uint8 lowbits[8] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
static uint8 cmpbits[9] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

#define LZW_CLEAR_CODE 256
#define LZW_EOI_CODE   257

MANAGER Tiff_Histogram

MANAGER Tiff_Channel histogram*Tiff_Histogram

static inline int timage_csize(Timage *image)
{ return (sizeof(Tiff_Channel *)*image->number_channels); }

static inline int timage_msize(Timage *image)
{ if (image->channels[0]->interpretation == CHAN_MAPPED)
    return (3*sizeof(uint16)*(1 << image->channels[0]->scale));
  else
    return (0);
}

MANAGER -cpfk Tiff_Image(Timage) channels:csize map:msize ifd_ref@Tiff_IFD

Tiff_Image *Copy_Tiff_Image(Tiff_Image *image)
{ Tiff_Image *copy = (Tiff_Image *) copy_timage((Timage *) image);
  int         i;

  for (i = 0; i < image->number_channels; i++)
    copy->channels[i] = Copy_Tiff_Channel(image->channels[i]);
  return (copy);
}

Tiff_Image *Pack_Tiff_Image(Tiff_Image *image)
{ int   i;

  pack_timage((Timage *) image);
  for (i = 1; i < image->number_channels; i++)
    Pack_Tiff_Channel(image->channels[i]);
}

void Free_Tiff_Image(Tiff_Image *image)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i] != NULL)
      Free_Tiff_Channel(image->channels[i]);
  free_timage((Timage *) image);
}

void Kill_Tiff_Image(Tiff_Image *image)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i] != NULL)
      Kill_Tiff_Channel(image->channels[i]);
  kill_timage((Timage *) image);
}


/****************************************************************************************
 *                                                                                      *
 *  PRINT DEBUG ROUTINE                                                                 *
 *                                                                                      *
 ****************************************************************************************/

#ifdef DEBUG_ENCODE || DEBUG_DECODE

static void Print_Plane(uint64 width, uint64 height, int bytes, void *array, int hex, int diff)
{ uint64 x, y;

  if (hex)
    { uint8 *plane = array;
      if (bytes == 1)
        { for (y = 0; y < height; y++)
            { printf("Row %2d:",y);
              for (x = 0; x < width; x++)
                printf(" %02x",plane[y*width + x]);
              printf("\n");
            }
        }
      else if (bytes == 2)
        { for (y = 0; y < height; y++)
            { printf("Row %2d:",y);
              for (x = 0; x < width; x++)
                printf(" %02x%02x",plane[(y*width + x)*2],plane[(y*width + x)*2+1]);
              printf("\n");
            }
        }
      else //  bytes == 4
        { for (y = 0; y < height; y++)
            { printf("Row %2d:",y);
              for (x = 0; x < width; x++)
                printf(" %02x%02x%02x%02x",plane[(y*width + x)*4],plane[(y*width + x)*4+1],
                                           plane[(y*width + x)*4+2],plane[(y*width + x)*4+3]);
              printf("\n");
            }
        }
    }
  else
    { if (bytes == 1)
        { uint8 *plane = array;
          char  *slane = array;
          for (y = 0; y < height; y++)
            { printf("Row %2d: %u",y,plane[y*width]);
              if (diff)
                for (x = 1; x < width; x++)
                  printf(" %d",slane[y*width + x]);
              else
                for (x = 1; x < width; x++)
                  printf(" %u",plane[y*width + x]);
              printf("\n");
            }
        }
      else if (bytes == 2)
        { uint16 *plane = array;
          short  *slane = array;
          for (y = 0; y < height; y++)
            { printf("Row %2d: %u",y,plane[y*width]);
              if (diff)
                for (x = 1; x < width; x++)
                  printf(" %d",slane[y*width + x]);
              else
                for (x = 1; x < width; x++)
                  printf(" %u",plane[y*width + x]);
              printf("\n");
            }
        }
      else //  bytes == 4
        { uint32 *plane = array;
          int    *slane = array;
          for (y = 0; y < height; y++)
            { printf("Row %2d: %u",y,plane[y*width]);
              if (diff)
                for (x = 1; x < width; x++)
                  printf(" %d",slane[y*width + x]);
              else
                for (x = 1; x < width; x++)
                  printf(" %u",plane[y*width + x]);
              printf("\n");
            }
        }
    }
}

#endif


/****************************************************************************************
 *                                                                                      *
 *  PACKBITS ENCODER AND DECODER                                                        *
 *                                                                                      *
 ****************************************************************************************/

#define IDLE      0
#define UNIQUE    1
#define DUPLICATE 2

static int   PackState;
static int   PackCount;
static int   PackValue;
static int   PackULen;

void Start_PackBits_Decoder()
{ PackState = IDLE; }

int Get_PackBits(FILE *input)    //  Returns the next decoded byte from stream input
{ int n;

  while (PackState == IDLE)
    { n = (signed char) fgetc(input);
      if (n >= 0)
        { if (n > 0)
            { PackCount = n;
              PackState = UNIQUE;
            }
          return (fgetc(input));
        }
      else if (n > -128)
        { PackCount = -n;
          PackState = DUPLICATE;
          PackValue = fgetc(input);
          return (PackValue);
        }
    }
  if (PackState == UNIQUE)
    { if (--PackCount == 0)
        PackState = IDLE;
      return (fgetc(input));
    }
  else // PackState == DUPLICATE
    { if (--PackCount == 0)
        PackState = IDLE;
      return (PackValue);
    }
}

//  Called sequential with a sequence of bytes to be encoded and writes the encoded
//    string to output.  Call with -1 to flush the encoding buffer.

void Start_PackBits_Encoder()
{ PackState = UNIQUE;
  PackULen  = 0;
  PackValue = -1;
}

void Put_PackBits(FILE *output, int byte)
{ static uint8 Buffer[129];

#define FLUSH_BUFFER(amount)		\
{ int i;                                \
  fputc((amount)-1,output);		\
  for (i = 0; i < (amount); i++)	\
    fputc(Buffer[i],output);		\
  PackULen -= (amount);			\
}

  if (PackState == UNIQUE)
    { if (byte == PackValue)
        { PackState = DUPLICATE;
          PackCount = 2;
        }
      else
        { if (PackULen >= 128)
            FLUSH_BUFFER(PackULen)
          Buffer[PackULen++] = PackValue = byte;
        }
    }
  else if (PackState == DUPLICATE)
    { if (byte == PackValue)
        PackCount += 1;
      else
        { PackState = UNIQUE;
          if (PackCount > 2)
            { if (PackULen-- > 1)
                FLUSH_BUFFER(PackULen)
              while (PackCount > 128)
                { fputc(-127,output);
                  fputc(PackValue,output);
                  PackCount -= 128;
                }
              if (PackCount > 2)
                { fputc(1-PackCount,output);
                  fputc(PackValue,output);
                  PackCount = 0;
                }
              else if (PackCount > 0)
                Buffer[PackULen++] = PackValue;
            }
          if (PackCount == 2)
            Buffer[PackULen++] = PackValue;
          if (PackULen >= 128)
            { FLUSH_BUFFER(128)
              if (PackULen > 0)            // => PackULen = 1
                Buffer[0] = Buffer[128];
            }
          Buffer[PackULen++] = PackValue = byte;
        }
    }
  if (byte < 0 && PackState != DUPLICATE)
    { if (PackULen-- > 1)
        FLUSH_BUFFER(PackULen);
    }
}


/****************************************************************************************
 *                                                                                      *
 *  LZW ENCODER AND DECODER                                                             *
 *                                                                                      *
 ****************************************************************************************/

static int    LZWbit;
static int    LZWbyte;
static int    LZWcodelen;
static int    LZWcount;

void Start_LZW_Decoder(FILE *input)
{ LZWbyte    = fgetc(input);
  LZWbit     = 0;
  LZWcodelen = 9;
  LZWcount   = 0;
}

int Get_LZW(FILE *input)    //  Returns the next decoded byte from stream input
{ static int firstime = 1;

  static uint8  buffer[4096];
  static int    symlen[4096];
  static uint8  strsym[4096];
  static int    strptr[4096];

  static int    tabtop;
  static int    ratchet;
  static int    lastcode;

  if (LZWcount == 0)
    { int code;

      // get next LZWcodelen bits in code for LZWcodelen in [9,12]

#define STREAM_CODE					\
      { int lo;						\
							\
        lo      = LZWbit + LZWcodelen - 8;		\
        code    = (LZWbyte & lowbits[LZWbit]) << lo;	\
        LZWbyte = fgetc(input);				\
        if (lo < 8)					\
          LZWbit = lo;					\
        else						\
          { LZWbit = lo-8;				\
            code |= (LZWbyte << LZWbit);		\
            LZWbyte = fgetc(input);			\
          }						\
        code |= (LZWbyte >> (8-LZWbit));		\
      }

      STREAM_CODE

      if (code == LZW_EOI_CODE) return (0);

      if (code == LZW_CLEAR_CODE)
        { tabtop     = LZW_EOI_CODE+1;
          LZWcodelen = 9;
          ratchet    = 511;

          if (firstime)
            { int i;

              firstime = 0;
              for (i = 0; i < LZW_CLEAR_CODE; i++)
                { symlen[i] = 1;
                  strsym[i] = i;
                  strptr[i] = -1;
                }
            }

          STREAM_CODE

          if (code == LZW_CLEAR_CODE) return (0);

          buffer[0] = strsym[code];
          LZWcount  = 1;
        }

      else
        { int llen = symlen[lastcode];

          if (code == tabtop)
            { int i, p;
              p = lastcode;
              for (i = 1; i <= llen; i++)
                { buffer[i] = strsym[p];
                  p         = strptr[p];
                }
              buffer[0] = buffer[llen];
              LZWcount  = llen+1;
            }
          else
            { int i, p;
              LZWcount = symlen[code];
              p = code;
              for (i = 0; i < LZWcount; i++)
                { buffer[i] = strsym[p];
                  p         = strptr[p];
                }
            }

          symlen[tabtop] = llen + 1;
          strsym[tabtop] = buffer[LZWcount-1];
          strptr[tabtop] = lastcode;

          tabtop += 1;
          if (tabtop == ratchet)
            { LZWcodelen += 1;
              ratchet  = (ratchet << 1) + 1;
            }
        }

      lastcode = code;
    }

  return (buffer[--LZWcount]);
}

//  Called sequential with a sequence of bytes to be encoded and writes the encoded
//    string to output.  Call with -1 to flush the encoding buffer.

void Start_LZW_Encoder()
{ LZWcount = 1; }

void Put_LZW(FILE *output, int ibyte)
{ static int firstime = 1;
  static int lt_fork[4096];
  static int gt_fork[4096];
  static int forward[4096];
  static int symbol[4096];

  static int        bit;
  static uint8      byte;

  static int        tabtop;
  static int        codelen;
  static int        ratchet;
  static int        code;

  // write next codelen bits in code for codelen in [9,12]

#define OUTPUT_CODE(value)			\
  { int lo;					\
						\
    lo = bit + codelen - 8;			\
    byte |= ((value >> lo) & lowbits[bit]);	\
    fputc(byte,output);				\
    if (lo < 8)					\
      bit = lo;					\
    else					\
      { bit = lo-8;				\
        byte = (value >> bit) & 0xff;		\
        fputc(byte,output);			\
      }						\
    byte = (value << (8-bit)) & 0xff;		\
  }

  if (LZWcount)
    { int k;

      if (firstime)
        { firstime = 0;
          for (k = 0; k < LZW_CLEAR_CODE; k++)
            symbol[k] = k;
        }
      for (k = 0; k < LZW_CLEAR_CODE; k++)
        forward[k] = -1;

      tabtop  = LZW_EOI_CODE+1;
      codelen = 9;
      ratchet = 512;

      bit     = 0;
      byte    = 0;
      code    = ibyte;

      OUTPUT_CODE(LZW_CLEAR_CODE)

      LZWcount = 0;
    }
  else if (ibyte < 0)
    { OUTPUT_CODE(code)
      if (++tabtop >= ratchet && ratchet != 4095)
        codelen += 1;
      OUTPUT_CODE(LZW_EOI_CODE)
    }
  else
    { int *lptr, last;
      int  csym, esym;

      last = code;
      code = *(lptr = forward + code);
      while (code >= 0 && ibyte != (esym = symbol[code]))
        if (ibyte < esym)
          code = *(lptr = lt_fork + code);
        else
          code = *(lptr = gt_fork + code);
    
      if (code < 0)
        { code = *lptr = tabtop++;
    
          OUTPUT_CODE(last)
    
          symbol[code]  = ibyte;
          lt_fork[code] = gt_fork[code] = forward[code] = -1;
    
          if (tabtop >= ratchet)
            { int k;
    
              if (ratchet == 4095)
                { OUTPUT_CODE(LZW_CLEAR_CODE)
                  tabtop  = LZW_EOI_CODE+1;
                  codelen = 9;
                  ratchet = 512;
                  for (k = 0; k < LZW_CLEAR_CODE; k++)
                    forward[k] = -1;
                }
              else
                { codelen  += 1;
                  ratchet <<= 1;
                  if (ratchet == 4096)
                    ratchet = 4095;
                }
            }
    
          code = ibyte;
        }
    }
}

//  A version of the LZW decoder that determines the length of the coded sequence
//    on input.  Needed in order to determine byte_counts for .lsm files

uint64 LZW_Counter(FILE *input)
{ int    bit = 0;
  int    cbyte;
  uint64 clength;

  int    tabtop  = LZW_EOI_CODE+1;
  int    codelen = 9;
  int    ratchet = 511;

  cbyte   = fgetc(input);
  clength = 1;
  while (1)
    { int code;

#define READ_CODE_LEN				\
      { int lo;					\
						\
        lo       = bit + codelen - 8;		\
        code     = (cbyte & lowbits[bit]) << lo;\
        cbyte    = fgetc(input);		\
        clength += 1;				\
        if (lo < 8)				\
          bit = lo;				\
        else					\
          { bit    = lo-8;			\
            code    |= (cbyte << bit);		\
            cbyte    = fgetc(input);		\
            clength += 1;			\
          }					\
        code |= (cbyte >> (8-bit));		\
      }

      READ_CODE_LEN

      if (code == LZW_EOI_CODE) break;

      if (code == LZW_CLEAR_CODE)
        { tabtop  = LZW_EOI_CODE+1;
          codelen = 9;
          ratchet = 511;

          READ_CODE_LEN

          if (code == LZW_CLEAR_CODE) break;
        }

      else
        { tabtop += 1;
          if (tabtop == ratchet)
            { codelen += 1;
              ratchet  = (ratchet << 1) + 1;
            }
        }
    }

  return (clength);
}


/****************************************************************************************
 *                                                                                      *
 *  EXTRACT AN IMAGE FROM AN IFD                                                        *
 *                                                                                      *
 ****************************************************************************************/

static void flip_rows(Tiff_Channel *channel, uint32 width, uint32 height)
{ uint8  *p, *q, *r;
  uint32  b, y, k, t;
  uint64  step;

  b    = channel->bytes_per_pixel;
  step = ((uint64) width) * b; 

  p = (uint8 *) channel->plane;
  for (y = 0; y < height; y++)
    { q = p;
      p = p+step;
      r = p-b;
      while (q < r)
        { for (k = 0; k < b; k++)
            { t    = q[k];
              q[k] = r[k];
              r[k] = t;
            }
          q += b;
          r -= b;
        }
    }
}

static void flip_columns(Tiff_Channel *channel, uint32 width, uint32 height)
{ uint8  *p, *q, *r, *s;
  uint32  x, t;
  uint64  step;

  step = width * channel->bytes_per_pixel;

  p = (uint8 *) channel->plane;
  s = p + width * ((uint64) (height-1));
  for (x = 0; x < step; x++)
    { q = p++;
      r = s++;
      while (q < r)
        { t  = *q;
          *q = *r;
          *r = t;
          q += step;
          r -= step;
        }
    }
}

static inline uint32 get_integer_tag(Tiff_IFD *ifd, int label, char *name,
                                     int hasdefault, uint32 defvalue)
{ void     *p;
  int       count;
  Tiff_Type type;

  p = Get_Tiff_Tag(ifd,label,&type,&count);
  if (p == NULL)
    { if (hasdefault)
        return (defvalue);
      sprintf(ESTRING,"IFD does not contain a tag for %s",name);
      CATCH(1)
    }
  if (count != 1)
    { sprintf(ESTRING,"Tag %s of IFD does not have a count of 1",name);
      CATCH(1)
    }
  if (type == TIFF_SHORT)
    return (*((uint16 *) p));
  else if (type == TIFF_LONG)
    return (*((uint32 *) p));
  else
    { sprintf(ESTRING,"Tag %s of IFD is not of type SHORT or LONG",name);
      CATCH(1)
    }
}

static uint64 optseek(FILE *file, uint64 offset, int pos)
{ if (pos == SEEK_SET)
    { if (offset != ftell(file))
        return (fseek(file,offset,pos));
      else
        return (0);
    }
  else //  pos == SEEK_END or pos == SEEK_CUR
    return (fseek(file,offset,pos));
}


static inline void *error(char *message)
{ sprintf(ESTRING,message);
  return (NULL);
}

typedef struct
  { uint32  width, height;
    int     bytes_per_chunk;
    int     bits_per_chunk;
    int     samples_per_pixel;
    uint16 *bits_per_sample;
    int    *bytes_per_sample;
    int     orient;
  } shape;

shape *extract_shape(Tiff_IFD *ifd)
{ static shape   S;

  static int     Max_Samples_Per_Pixel = 0;
  static int    *Bytes_Per_Sample = NULL;
  static uint16 *Default_Bps = NULL;

  if (EXCEPTION)
    return (NULL);

  //  Get and check width and height

  S.width  = get_integer_tag(ifd,TIFF_IMAGE_WIDTH,"image_width",0,0);
  S.height = get_integer_tag(ifd,TIFF_IMAGE_LENGTH,"image_height",0,0);

  //  Get and check samples_per_pixel and bits_per_sample array
  
  { int       count;
    Tiff_Type type;

    S.samples_per_pixel = get_integer_tag(ifd,TIFF_SAMPLES_PER_PIXEL,"samples_per_pixel",1,1);

    S.bits_per_sample = (uint16 *) Get_Tiff_Tag(ifd,TIFF_BITS_PER_SAMPLE,&type,&count);
  
    if (S.bits_per_sample != NULL)
      { if (type != TIFF_SHORT)
          return (error("Bits_per_sample tag is not of type SHORT"));
        if (S.samples_per_pixel > count)
          return (error("Length of bits_per_sample tag is less than samples_per_pixel"));
      }

    if (S.samples_per_pixel > Max_Samples_Per_Pixel)
      { Max_Samples_Per_Pixel = S.samples_per_pixel+4;
        Bytes_Per_Sample = (int *) Guarded_Realloc(Bytes_Per_Sample,
                                                   sizeof(int)*Max_Samples_Per_Pixel,NULL);
        Default_Bps = (uint16 *) Guarded_Realloc(Default_Bps,
                                                 sizeof(uint16)*Max_Samples_Per_Pixel,NULL);
      }

    { int i, c;

      if (S.bits_per_sample == NULL)
        { S.bits_per_sample = Default_Bps;
          for (i = 0; i < S.samples_per_pixel; i++)
            Default_Bps[i] = 1;
        }

      S.bytes_per_chunk  = 0;
      S.bits_per_chunk   = 0;
      S.bytes_per_sample = Bytes_Per_Sample;

      for (i = 0; i < S.samples_per_pixel; i++)
        { c = S.bits_per_sample[i];
          if (c > 32)
            return (error("# of bits in a sample is greater than 32"));

          c = (c-1)/8+1;
          if (c == 3)
            c = 4;
          S.bytes_per_sample[i] = c;
          S.bytes_per_chunk    += c;
          S.bits_per_chunk     += S.bits_per_sample[i];
        }
    }
  }

  S.orient = get_integer_tag(ifd,TIFF_ORIENTATION,"orientation",1,TIFF_VALUE_TOP_N_LEFT);

  return (&S);
}

typedef struct
  { int     compression;
    int     prediction;
    uint32  twidth;
    uint32  theight;
    int     strips;
    int     chunk;
    uint64 *offsets;
    uint64 *counts;
  } image_tags;

image_tags *extract_image_tags(Tiff_IFD *ifd, shape *S)
{ static image_tags I;
  static int        Max_Byte_Counts = 0;
  static uint64    *Byte_Counts = NULL;
  static uint64    *Offsets     = NULL;

  //  Get and check the compression status

  I.compression = get_integer_tag(ifd,TIFF_COMPRESSION,"compression",1,TIFF_VALUE_UNCOMPRESSED);
  if (I.compression != TIFF_VALUE_UNCOMPRESSED && I.compression != TIFF_VALUE_LZW &&
      I.compression != TIFF_VALUE_PACK_BITS)
    return (error("Support only uncompressed, Packbits, and LZW compressed images"));

  I.prediction = get_integer_tag(ifd,TIFF_PREDICTOR,"prediction",1,TIFF_NO_PREDICTION);

  //  Get and check the image encoding parameters, offsets, and byte counts

  { int       i, segments, tiles;
    void     *vector;
    Tiff_Type type;

    vector = Get_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,&type,&segments);
    if (vector == NULL)
      { vector  = Get_Tiff_Tag(ifd,TIFF_TILE_BYTE_COUNTS,&type,&segments);
        if (vector == NULL)
          return (error("IFD does not contain a tag for byte_counts"));
        I.theight = get_integer_tag(ifd,TIFF_TILE_LENGTH,"tile_height",0,0);
        I.twidth  = get_integer_tag(ifd,TIFF_TILE_WIDTH,"tile_width",0,0);
        I.strips = 0;
      }
    else
      { I.theight = get_integer_tag(ifd,TIFF_ROWS_PER_STRIP,"rows_per_strip",1,S->height);
        I.twidth  = S->width;
        I.strips = 1;
      }

    I.chunk = get_integer_tag(ifd,TIFF_PLANAR_CONFIGURATION,
                               "planar_configuration",1,TIFF_VALUE_CHUNKY);
    I.chunk = (I.chunk == TIFF_VALUE_CHUNKY) && (S->samples_per_pixel > 1);
    tiles   = ((S->height-1) / I.theight + 1) * ((S->width-1) / I.twidth + 1);
    if (!I.chunk)
      tiles *= S->samples_per_pixel;

    if (segments != tiles)
      return (error("Length of byte_count tag does not equal # of tiles/strips"));

    if (segments > Max_Byte_Counts)
      { Max_Byte_Counts = segments+16;
        Byte_Counts = (uint64 *) Guarded_Realloc(Byte_Counts,
                                   sizeof(uint64)*Max_Byte_Counts,NULL);
        Offsets = (uint64 *) Guarded_Realloc(Offsets,
                                   sizeof(uint64)*Max_Byte_Counts,NULL);
      }

    if (type == TIFF_SHORT)
      { uint16 *countS = (uint16 *) vector;
        for (i = 0; i < segments; i++)
          Byte_Counts[i] = countS[i];
      }
    else if (type == TIFF_LONG)
      { uint32 *countI = (uint32 *) vector;
        for (i = 0; i < segments; i++)
          Byte_Counts[i] = countI[i];
      }
    else //  type == TIFF_LONG64
      { uint64 *countL = (uint64 *) vector;
        for (i = 0; i < segments; i++)
          Byte_Counts[i] = countL[i];
      }

    vector = Get_Tiff_Tag(ifd,TIFF_STRIP_OFFSETS,&type,&segments);
    if (vector == NULL)
      { vector  = Get_Tiff_Tag(ifd,TIFF_TILE_OFFSETS,&type,&segments);
        if (vector == NULL)
          return (error("IFD does not contain a tag for offsets"));
        if (I.strips)
          return (error("IFD does not contain compatible offset and byte_count tags"));
      }
    else if (! I.strips)
      return (error("IFD does not contain compatible offset and byte_count tags"));

    if (segments != tiles)
      return (error("Length of offset tag does not equal # of tiles/strips"));

    if (type == TIFF_SHORT)
      { uint16 *offsetS = (uint16 *) vector;
        for (i = 0; i < segments; i++)
          Offsets[i] = offsetS[i];
      }
    else if (type == TIFF_LONG)
      { uint32 *offsetI = (uint32 *) vector;
        for (i = 0; i < segments; i++)
          Offsets[i] = offsetI[i];
      }
    else //  type == TIFF_LONG64
      { uint64 *offsetL = (uint64 *) vector;
        for (i = 0; i < segments; i++)
          Offsets[i] = offsetL[i];
      }

    I.offsets = Offsets;
    I.counts  = Byte_Counts;
  }

  return (&I);
}

Tiff_Image *Get_Tiff_Image(Tiff_IFD *ifd)
{ shape      *S;
  image_tags *I;

  uint16 *sample_format;
  int     photometric;
  int     photo_samples;
  uint8  *extra_samples;
  uint16  *color_map;
  int      map_size;

  Timage        *image;
  Tiff_Channel **channel;

  //  Get all the tags and information required to build the image and check that all
  //    tags and values make sense (before starting to build the image)

  //  Get all the information for the image shape

  sprintf(ESOURCE,"Get_Tiff_Image");

  S = extract_shape(ifd);
  if (S == NULL)
    return (NULL);

  //  Get all the information for the image data (just to make sure IFD is OK)

  I = extract_image_tags(ifd,S);
  if (I == NULL)
    return (NULL);

  //  If the sample_format tag is present, make sure its size and type are correct

  { Tiff_Type type;
    int       count;

    sample_format = (uint16 *) Get_Tiff_Tag(ifd,TIFF_SAMPLE_FORMAT,&type,&count);

    if (sample_format != NULL)
      { int i;

        if (type != TIFF_SHORT)
          return (error("Sample_format tag is not of type SHORT"));
        if (count != S->samples_per_pixel)
          return (error("Sample_format tag is not of length samples_per_pixel"));
        for (i = 0; i < S->samples_per_pixel; i++)
          if (sample_format[i] == TIFF_VALUE_IEEE_FLOATING_POINT_DATA)
            if (S->bits_per_sample[i] != 32)
              return (error("Floating point pixels must be 32-bits"));
      }
  }

  //  Get and check the photometric information including extra_samples if color

  { Tiff_Type ctype, etype;
    int       ccount, ecount;

    map_size      = 0;
    photometric   = get_integer_tag(ifd,TIFF_PHOTOMETRIC_INTERPRETATION,
                                        "photometric_interpretation",0,0);
    color_map     = (uint16 *) Get_Tiff_Tag(ifd,TIFF_COLOR_MAP,&ctype,&ccount);
    extra_samples = (uint8  *) Get_Tiff_Tag(ifd,TIFF_EXTRA_SAMPLES,&etype,&ecount);

    photo_samples = 1;
    if (photometric == TIFF_VALUE_RGB)
      { if (S->samples_per_pixel < 3)
          return (error("RGB image has less than 3 samples_per_pixel"));
        photo_samples = 3;
      }
    else if (photometric == TIFF_VALUE_RGB_PALETTE)
      { if (color_map == NULL)
          return (error("Color_map tag is missing"));
        if (ctype != TIFF_SHORT)
          return (error("Color_map tag is not of type SHORT"));
        if (S->bits_per_sample[0] > 16)
          return (error("Color_map over values with more than 16-bits"));
        if (ccount != 3*(1 << S->bits_per_sample[0]))
          return (error("Color_map length doesn not match value range"));
        map_size = ccount*sizeof(uint16);
      }
    else if (photometric < 0 || photometric > TIFF_VALUE_TRANSPARENCY_MASK)
      return (error("Do not support CMYK, YcBcR, or CIE Lab photometric types"));

    if (extra_samples != NULL)
      { if (ecount != S->samples_per_pixel - photo_samples)
          return (error("Extra_samples tag has wrong number of samples")); 
        if (etype != TIFF_BYTE)
          return (error("Extra_samples tag is not of type BYTE"));
      }
  }

  image = NULL;
  if (EXCEPTION)
    { if (image != NULL)
        Free_Tiff_Image((Tiff_Image *) image);
      return (NULL);
    }

  //  Now begin building the image, first by filling out the descriptions of all the channels

  image = new_timage(S->samples_per_pixel*sizeof(Tiff_Channel *),map_size,NULL);

  image->ifd_ref         = Inc_Tiff_IFD(ifd);
  image->width           = S->width;
  image->height          = S->height;
  image->number_channels = S->samples_per_pixel;

  if (map_size > 0)
    memcpy(image->map,color_map,map_size);

  channel = image->channels;

  { int   i;

    for (i = 0; i < S->samples_per_pixel; i++)
      { channel[i] = new_tiff_channel(NULL);

        channel[i]->width  = S->width;
        channel[i]->height = S->height;

        channel[i]->scale           = S->bits_per_sample[i];
        channel[i]->bytes_per_pixel = S->bytes_per_sample[i];

        if (sample_format == NULL)
          channel[i]->type = CHAN_UNSIGNED;
        else if (sample_format[i] == TIFF_VALUE_TWOS_COMPLEMENT_DATA)
          channel[i]->type = CHAN_SIGNED;
        else if (sample_format[i] == TIFF_VALUE_IEEE_FLOATING_POINT_DATA)
          channel[i]->type = CHAN_FLOAT;
        else
          channel[i]->type = CHAN_UNSIGNED;

        channel[i]->plane = NULL;
      }
  }

  if (photometric == TIFF_VALUE_WHITE_IS_ZERO)
    channel[0]->interpretation = CHAN_WHITE;
  else if (photometric == TIFF_VALUE_BLACK_IS_ZERO)
    channel[0]->interpretation = CHAN_BLACK;
  else if (photometric == TIFF_VALUE_RGB)
    { channel[0]->interpretation = CHAN_RED;
      channel[1]->interpretation = CHAN_GREEN;
      channel[2]->interpretation = CHAN_BLUE;
    }
  else if (photometric == TIFF_VALUE_RGB_PALETTE)
    channel[0]->interpretation = CHAN_MAPPED;
  else //  photometric == TIFF_VALUE_TRANSPARENCY_MASK
    channel[0]->interpretation = CHAN_MASK;
 
  { int i;

    for (i = photo_samples; i < S->samples_per_pixel; i++)
      if (extra_samples == NULL)
        channel[i]->interpretation = CHAN_OTHER;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_ALPHA_DATA)
        channel[i]->interpretation = CHAN_ALPHA;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_SOFT_MATTE)
        channel[i]->interpretation = CHAN_MATTE;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_EXTRA_RED)
        channel[i]->interpretation = CHAN_RED;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_EXTRA_GREEN)
        channel[i]->interpretation = CHAN_GREEN;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_EXTRA_BLUE)
        channel[i]->interpretation = CHAN_BLUE;
      else
        channel[i]->interpretation = CHAN_OTHER;
  }

#ifdef DEBUG_DECODE

  { int i;

    static char *Meaning_String[] = { "White", "Black", "Mapped", "Red", "Green", "Blue",
                                      "Alpha", "Matte", "Mask", "Other" };

    static char *Type_String[] = { "Unsigned", "Signed", "Float" };

    printf("\nIFD Image: %d x %d, %d channels\n",image->width,image->height,image->number_channels);
    for (i = 0; i < S->samples_per_pixel; i++)
      printf("  %d: %s %d(%d) %s\n",i,Meaning_String[channel[i]->interpretation],
                                      channel[i]->scale,channel[i]->bytes_per_pixel,
                                      Type_String[channel[i]->type]);

    printf("  map_size = %d\n",S->map_size);
    printf("  bits_per_sample =");
    for (i = 0; i < S->samples_per_pixel; i++)
      printf(" %d",S->bits_per_sample[i]);
    printf(" += %d\n",S->bits_per_chunk);
    printf("  bytes_per_sample =");
    for (i = 0; i < S->samples_per_pixel; i++)
      printf(" %d",S->bytes_per_sample[i]);
    printf(" += %d\n",S->bytes_per_chunk);
    printf("  tiling      = %d x %d\n",I->twidth,I->theight);
    printf("  chunking    = %d\n",I->chunk);
    printf("  compression = %d\n",I->compression);
    printf("  prediction  = %d\n",I->prediction);
    printf("  orientation = %d\n",S->orient);
  }

#endif

  return ((Tiff_Image *) image);
}


int Load_Tiff_Image_Planes(Tiff_Image *image, void **planes)
{ shape      *S;
  image_tags *I;

  Tiff_Channel **channel;
  Timage        *img = (Timage *) image;

  sprintf(ESOURCE,"Load_Tiff_Image_Planes");

  S = extract_shape(img->ifd_ref);
  if (S == NULL)
    return (1);

  //  Get all the information for the image data

  I = extract_image_tags(img->ifd_ref,S);
  if (I == NULL)
    return (1);

  channel = image->channels;

  { int   i;

    for (i = 0; i < S->samples_per_pixel; i++)
      if (planes[i] != NULL)
        channel[i]->plane = planes[i];
  }

  if (Get_Tiff_IFD_Stream(img->ifd_ref) == NULL)  //  If already in memory then just transfer data
    { int   i, s;
      int   segments = ((S->width-1)/I->twidth+1) * ((S->height-1)/I->theight+1);
      void *source   = Get_Tiff_IFD_Image(img->ifd_ref);

      for (i = 0; i < S->samples_per_pixel; i++)
        if (planes[i] != NULL)
          { if (planes[i] != source+I->offsets[i*segments])
              memcpy(source+I->offsets[i*segments],planes[i],
                     S->width*S->height*S->bytes_per_sample[i]);
          }
      return (0);
    }

//  Move 8*bp3 + bp8 bits from bit stream (SOURCE,spos) to byte stream ipixel.

#define UNPACK_BITS(SOURCE,ADVANCE)					\
{ uint8 t;								\
  int   b;								\
									\
  if (bp8 != 0)								\
    { spos += bp8;							\
      if (spos <= 8)							\
        { *ipixel++ = ((SOURCE >> (8-spos)) & cmpbits[bp8]); 		\
          if (spos == 8)						\
            { ADVANCE;							\
              spos = 0;							\
            }								\
        }								\
      else   								\
        { spos -= 8;							\
          t = ((SOURCE & cmpbits[bp8-spos]) << spos);			\
          ADVANCE;							\
          *ipixel++ = (t | ((SOURCE >> (8-spos)) & cmpbits[spos]));	\
        }								\
    }									\
  if (spos == 0)							\
    for (b = 0; b < bp3; b++)						\
      { *ipixel++ = SOURCE;						\
        ADVANCE;							\
      }									\
  else									\
    for (b = 0; b < bp3; b++)						\
      { t = ((SOURCE & lowbits[spos]) << spos);				\
        ADVANCE;							\
        *ipixel++ = (t | ((SOURCE >> (8-spos)) & cmpbits[spos]));	\
      }									\
}

  //  Read each image chunk, decompress it if necessary, and place in the image
  //    Must pay attention to the reversal of rows and columns that may be required by orient.
  //    Distinct inner loops for chunk vs. planar, and byte vs. bit movement.

  { uint32 width   = S->width;
    uint32 height  = S->height;
    uint32 twidth  = I->twidth;
    uint32 theight = I->theight;

    int     samples_per_pixel = S->samples_per_pixel;
    uint16 *bits_per_sample   = S->bits_per_sample;
    int    *bytes_per_sample  = S->bytes_per_sample;
    int     bits_per_chunk    = S->bits_per_chunk;
    int     compression       = I->compression;

    uint64 *counts  = I->counts;
    uint64 *offsets = I->offsets;

    uint64 ocorner, icorner;
    uint32 segment;
    uint64 xinc, yinc;
    uint64 istep, ostep;
    uint32 x, y;
    int    (*get_next_byte)(FILE *);

    int bpc = bits_per_chunk;
    int bc8 = (bpc & 0x7);
    int bc3 = (bpc >> 3);

    FILE  *input = Get_Tiff_IFD_Stream(img->ifd_ref);
    int    cbyte;

    if (compression == TIFF_VALUE_LZW)	
      get_next_byte = Get_LZW;
    else if (compression == TIFF_VALUE_PACK_BITS)	
      get_next_byte = Get_PackBits;
    else
      get_next_byte = fgetc;

    if (S->orient > 4)
      { xinc = height;
        yinc = 1;
      }
    else
      { xinc = 1;
        yinc = width;
      }
    istep = twidth * xinc;
    ostep = theight * yinc;

    if (I->chunk)

      for (y       = 0,
           ocorner = 0,
           segment = 0; y < height; y       += theight,
                                    ocorner += ostep)
        for (x       = 0,
             icorner = ocorner; x < width; x       += twidth,
                                           icorner += istep,
                                           segment += 1)

          { uint8  stream[X_BUFFER_LEN];
            uint64 totalbytes;
            uint64 cbytes, chunks, fetchedbytes;
            uint32 maxi, maxj;
            int    buflen, indent;

            maxj = height-y;
            if (maxj > theight)
              maxj = theight;

            maxi = width-x;
            if (maxi > twidth)
              maxi = twidth;

            if (I->strips)
              totalbytes = (maxj*twidth*bits_per_chunk-1)/8+1;
            else
              totalbytes = (theight*twidth*bits_per_chunk-1)/8+1;

            optseek(input,offsets[segment],SEEK_SET);

            if (compression == TIFF_VALUE_LZW)
              Start_LZW_Decoder(input);
            else if (compression == TIFF_VALUE_PACK_BITS)
              Start_PackBits_Decoder();

#ifdef DEBUG_DECODE
            printf("  Tile Corner (%d,%d) -> %lld\n",x,y,icorner);
#endif

            for (indent = 0,
                 chunks = 0,
                 cbytes = 0; cbytes < totalbytes; cbytes += fetchedbytes,
                                                  chunks += buflen,
                                                  indent  = (X_BUFFER_BITS-indent)
                                                          - buflen*bits_per_chunk)

              { int leftover = (indent+7)/8;
                int usedup   = X_BUFFER_LEN-leftover;
                int fetchto;
                int s, off;
                int bidx, ibeg, jbeg;

                ibeg = chunks % twidth;
                jbeg = chunks / twidth;
                bidx = ibeg + jbeg * width;

                if (jbeg >= maxj) break;

                if (cbytes + usedup > totalbytes)
                  fetchedbytes = totalbytes - cbytes;
                else
                  fetchedbytes = usedup;

                for (s = 0; s < leftover; s++)
                  stream[s] = stream[usedup+s];

                fetchto = fetchedbytes + leftover;
                for (s = leftover; s < fetchto; s++)
                  stream[s] = get_next_byte(input);

                indent = 8*leftover-indent;
                buflen = (8*fetchto-indent)/bits_per_chunk;

                for (s   = 0,
                     off = indent; s < samples_per_pixel; off += bits_per_sample[s],
                                                          s   += 1)

                  { int BPS = bytes_per_sample[s];
                    int bps = bits_per_sample[s];
                    int bp3 = (bps >> 3);
                    int bp8 = (bps & 0x7);
                    int pad = (BPS == 4 && bps <= 24);

                    uint64 uinc = yinc * BPS;
                    uint64 vinc = xinc * BPS;
                    uint64 tinc = vinc - BPS;
                    uint64 pinc = uinc - vinc*maxi;

                    uint8 *ipixel, *source;
                    uint64 spos;

                    if (planes[s] == NULL) continue;

                    ipixel  = (uint8 *) (planes[s]) + (icorner+bidx)*BPS;
                    spos    = (off & 0x7);
                    source  = stream + (off >> 3);

                    if (bc8 == 0 && spos == 0 && bp8 == 0)

                      { int    bs3  = bc3 - bp3;
                        uint32 spad = bc3*(twidth-maxi);
                        int    b, k;
                        int    i, j;

                        i = ibeg;
                        j = jbeg;
                        for (k = 0; k < buflen; k++)
                          { if (pad)
                              *ipixel++ = 0;
                            for (b = 0; b < bp3; b++)
                              *ipixel++ = *source++;

                            i      += 1;
                            source += bs3;
                            ipixel += tinc;
                            if (i >= maxi)
                              { i  = 0;
                                j += 1;
                                if (j >= maxj) break;
                                k += twidth-maxi;
                                source += spad;
                                ipixel += pinc;
                              }
                          }

                      }

                    else   //  Must perform bit ops

                      { int    bs8  = bpc - bps;
                        uint64 spad = bpc*(twidth-maxi);
                        uint32 b, k;
                        int    i, j;

                        i = ibeg;
                        j = jbeg;
                        for (k = 0; k < buflen; k++)
                          { if (pad)
                              *ipixel++ = 0;
                            UNPACK_BITS(*source,source+=1)

                            i      += 1;
                            spos   += bs8;
                            ipixel += tinc;
                            if (i >= maxi)
                              { i  = 0;
                                j += 1;
                                if (j >= maxj) break;
                                k += twidth-maxi;
                                spos   += spad;
                                ipixel += pinc;
                              }
                            source += (spos >> 3);
                            spos   &= 0x7;
                          }
                      }
                  }
              }
          }

    else  //  ! chunk

      { int s;
        int tiles = ((height-1) / theight + 1) * ((width-1) / twidth + 1);
  
        segment = 0;
        for (s = 0; s < samples_per_pixel; s++)
          { int BPS = bytes_per_sample[s];
            int bps = bits_per_sample[s];
            int bp3 = (bps >> 3);
            int bp8 = (bps & 0x7);
            int pad = (BPS == 4 && bps <= 24);
    
            uint64 uinc = yinc * BPS;
            uint64 tinc = (xinc-1) * BPS;

            if (planes[s] == NULL)
              { segment += tiles;
                continue;
              }

            for (y       = 0,
                 ocorner = 0; y < height; y       += theight,
                                          ocorner += ostep)
              for (x       = 0,
                   icorner = ocorner; x < width; x += twidth,
                                                 icorner += istep,
                                                 segment += 1)

                { uint8 *opixel, *ipixel;
                  uint64 spos;

                  uint32 maxi, maxj;

                  optseek(input,offsets[segment],SEEK_SET);

                  if (compression == TIFF_VALUE_LZW)
                    Start_LZW_Decoder(input);
                  else if (compression == TIFF_VALUE_PACK_BITS)
                    Start_PackBits_Decoder();
                  cbyte = get_next_byte(input);

                  maxj = height-y;
                  if (maxj > theight)
                    maxj = theight;
 
                  maxi = width-x;
                  if (maxi > twidth)
                    maxi = twidth;

#ifdef DEBUG_DECODE
                  printf("  Plane %d: Tile Corner (%d,%d) -> %lld\n",s,x,y,icorner);
                  printf("    Bit case: bytes = %d bits = %d\n",bp3,bp8);
#endif
  
                  opixel  = ((uint8 *) channel[s]->plane) + icorner * BPS;
                  spos    = 0;
  
                  if (bp8 == 0)
                    { uint32 spad = bp3*(twidth-maxi);
                      uint32 i, j, b;
  
                      for (j = 0; j < maxj; j      += 1,
                                            opixel += uinc)
                        { for (i      = 0,
                               ipixel = opixel; i < maxi; i      += 1,
                                                          ipixel += tinc)
                            { if (pad)
                                *ipixel++ = 0;
                              for (b = 0; b < bp3; b++)
                                { *ipixel++ = cbyte;
                                  cbyte = get_next_byte(input);
                                }
                            }
                          for (i = 0; i < spad; i++)
                            cbyte = get_next_byte(input);
                        }
                    }
  
                  else
                    { uint32 spad = bps*(twidth-maxi);
                      uint32 i, j, b;

                      for (j = 0; j < maxj; j      += 1,
                                            opixel += uinc,
                                            spos   += spad)
                        { for (i = 0; i < (spos>>3); i++)
                            cbyte = get_next_byte(input);
                          spos &= 0x7;
                          for (i      = 0,
                               ipixel = opixel; i < maxi; i      += 1,
                                                          ipixel += tinc)
                            { if (pad)
                                *ipixel++ = 0;
                              UNPACK_BITS(cbyte,cbyte=get_next_byte(input))
                            }
                        }
                    }
                }
          }
      }
  }

  if (S->orient > 4)
    { int i;

      image->width  = S->height;
      image->height = S->width;
      for (i = 0; i < S->samples_per_pixel; i++)
        { image->channels[i]->width  = image->width;
          image->channels[i]->height = image->height;
        }
      S->width  = image->width;
      S->height = image->height;
    }

  //  Endian flip multi-byte data if needed

  { uint64 a = ((uint64) S->width)*S->height;
    uint64 i;
    int    s;

    for (s = 0; s < S->samples_per_pixel; s++)
      { uint8 *w = (uint8 *) (channel[s]->plane);
        uint8  x;

        if (w == NULL) continue;

        if (S->bytes_per_sample[s] == 2)

          { if ((S->bits_per_sample[s] == 16 && Get_Tiff_IFD_Data_Flip(img->ifd_ref)) ||
                (S->bits_per_sample[s] < 16 && ! Native_Endian()))

              for (i = 0; i < a; i++)
                { x = w[0];
                  w[0] = w[1];
                  w[1] = x;
                  w += 2;
                }
          }

        else if (S->bytes_per_sample[s] == 4)

          { if ((S->bits_per_sample[s] == 32 && Get_Tiff_IFD_Data_Flip(img->ifd_ref)) ||
                (S->bits_per_sample[s] < 32 && ! Native_Endian()))

              for (i = 0; i < a; i++)
                { x = w[0];
                  w[0] = w[3];
                  w[3] = x;
                  x = w[1];
                  w[1] = w[2];
                  w[2] = x;
		  w += 4;
		}
	  }
      }
  }

  //  Difference Predictor: note carefully that differences are sign-extended
  //    in twos' complement representation (if the architecture happens to support
  //    another representation of signed numbers then this code doesn't work!)

  if (I->prediction == TIFF_HORIZONTAL_DIFFERENCING)
    { uint32 s, x, y;
      uint32 width  = S->width;
      uint32 height = S->height;

      for (s = 0; s < S->samples_per_pixel; s++)

        { if (planes[s] == NULL) continue;

          if (S->bytes_per_sample[s] == 1)
            { uint8  last;
              uint8 *base = (uint8 *) (channel[s]->plane);
              uint8  sign, mask;
  
              mask = cmpbits[S->bits_per_sample[s]];
              sign = ~mask;
  
              for (y = 0; y < height; y++)
                { last  = *base;
                  base += 1;
                  for (x = 1; x < width; x++)
                    { last = ((last + (signed char) (*base | sign)) & mask);
                      *base++ = last;
                    }
                }
            }
  
          else if (S->bytes_per_sample[s] == 2)
            { uint16  last;
              uint16 *base = (uint16 *) (channel[s]->plane);
              uint16  sign, mask;
  
              mask = (cmpbits[S->bits_per_sample[s]-8] << 8) | 0xff;
              sign = ~mask;
  
              for (y = 0; y < height; y++)
                { last  = *base;
                  base += 1;
                  for (x = 1; x < width; x++)
                    { last = ((last + (signed short) (*base | sign)) & mask);
                      *base++ = last;
                    }
                }
            }
  
          else //  S->bytes_per_sample[s] == 4
            { uint32  last;
              uint32 *base = (uint32 *) (channel[s]->plane);
              uint32  sign, mask;
  
              if (S->bits_per_sample[s] > 24)
                mask = (cmpbits[S->bits_per_sample[s]-24] << 24) | 0xffffff;
              else
                mask = (cmpbits[S->bits_per_sample[s]-16] << 16) | 0xffff;
              sign = ~mask;
  
              for (y = 0; y < height; y++)
                { last  = *base;
                  base += 1;
                  for (x = 1; x < width; x++)
                    { last = ((last + (signed int) (*base | sign)) & mask);
                      *base++ = last;
                    }
                }
            }
        }
    }

  //  Signed data requires extension !!!!

  //  Flip rows, columns, and dimensions as per orient directive

  { int i;

    for (i = 0; i < S->samples_per_pixel; i++)
      { if (planes[i] == NULL)
          continue;
        if (S->orient%4 >= 2)
          flip_rows(channel[i],S->width,S->height);
        if ((S->orient-1)%4 >= 2)
          flip_columns(channel[i],S->width,S->height);
      }
  }

  return (0);
}


void Remove_Unloaded_Channels(Tiff_Image *image)
{ int i, s;

  s = 0;
  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i]->plane == NULL)
      Free_Tiff_Channel(image->channels[i]);
    else
      image->channels[s++] = image->channels[i];
  image->number_channels = s;
}


/****************************************************************************************
 *                                                                                      *
 *  MAKE AN IMAGE FROM SCRATCH                                                          *
 *                                                                                      *
 ****************************************************************************************/

Tiff_Image *Create_Tiff_Image(unsigned int width, unsigned int height)
{ Timage *image;

  if (EXCEPTION)
    { sprintf(ESOURCE,"Create_Tiff_Image");
      return (NULL);
    }

  image = new_timage(10*sizeof(Tiff_Channel *),0,NULL);
  image->width  = width;
  image->height = height;
  image->number_channels = 0;
  return ((Tiff_Image *) image);
}

int Add_Tiff_Image_Channel(Tiff_Image *image, Channel_Meaning meaning, int scale,
                                              Channel_Type type, void *plane)
{ Tiff_Channel *channel;
  int           n, csize;

  if (scale > 32)
    { sprintf(ESOURCE,"Add_Tiff_Image_Channel");
      sprintf(ESTRING,"Scale cannot be more than 32 bits\n");
      return (1);
    }

  channel = NULL;
  if (EXCEPTION)
    { if (channel != NULL)
        Free_Tiff_Channel(channel);
      image->number_channels = n;
      sprintf(ESOURCE,"Add_Tiff_Image_Channel");
      return (1);
    }

  n = image->number_channels;

  csize = sizeof_timage_channels((Timage *) image);
  if ((n+1)*sizeof(Tiff_Channel *) > csize) 
    allocate_timage_channels((Timage *) image,(n+10)*sizeof(Tiff_Channel *),NULL);

  image->channels[n] = channel = new_tiff_channel(NULL);
  image->number_channels = n+1;

  channel->width          = image->width;
  channel->height         = image->height;
  channel->interpretation = meaning;
  channel->scale          = scale;
  channel->type           = type;
  channel->plane          = plane;
  channel->histogram      = NULL;

  scale = (scale-1)/8+1;
  if (scale == 3)
    scale = 4;
  channel->bytes_per_pixel = scale;

  if (n == 0 && meaning == CHAN_MAPPED)
    allocate_timage_map((Timage *) image,timage_msize((Timage *) image),NULL);

  return (0);
}


/****************************************************************************************
 *                                                                                      *
 *  MAKE AN IFD FROM AN IMAGE                                                           *
 *                                                                                      *
 ****************************************************************************************/

Tiff_IFD *Make_IFD_For_Image(Tiff_Image *image, Tiff_Compress compress,
                                                unsigned int tile_width, unsigned int tile_height)
{ uint8     *valB;
  uint16    *valS;
  int        bit64;
  uint32    *Offsets;
  uint64    *Offsets64;
  uint32    *Byte_Counts;
  uint64    *Byte_Counts64;

  uint16 samples_per_pixel;
  uint16 planar_configuration;
  uint16 compression;
  uint16 predictor;
  uint16 photometric;

  Tiff_IFD *ifd;
  int       photo_samples;
  int       minoff;
  uint8    *data, *stream;
  uint8    *encode;

  ifd = NULL;
  if (EXCEPTION)
    { if (ifd != NULL)
        Free_Tiff_IFD(ifd);
      sprintf(ESTRING,"Out of memory");
      sprintf(ESOURCE,"Make_IFD_For_Image");
      return (NULL);
    }

  { int i;

    minoff = 0;
    for (i = 0; i < image->number_channels; i++)
      { if (image->channels[i]->plane == NULL)
          { sprintf(ESTRING,"Plane for channel %d is NULL",i);
            sprintf(ESOURCE,"Make_IFD_For_Image");
            return (NULL);
          }
        if (image->channels[i]->plane < image->channels[minoff]->plane)
          minoff = i;
      }
  }

  ifd = Create_Tiff_IFD(12,image->channels[minoff]->plane);
  CATCH(ifd == NULL)

  CATCH(Set_Tiff_Tag(ifd,TIFF_IMAGE_WIDTH,TIFF_LONG,1,&(image->width)))
  CATCH(Set_Tiff_Tag(ifd,TIFF_IMAGE_LENGTH,TIFF_LONG,1,&(image->height)))

  samples_per_pixel = image->number_channels;
  CATCH(Set_Tiff_Tag(ifd,TIFF_SAMPLES_PER_PIXEL,TIFF_SHORT,1,&(samples_per_pixel)))

  valS = (uint16 *) Allocate_Tiff_Tag(ifd,TIFF_BITS_PER_SAMPLE,TIFF_SHORT,samples_per_pixel);
  CATCH(valS == NULL)

  { int i;

    bit64 = 0;
    for (i = 0; i < image->number_channels; i++)
      { uint64 size = image->width * image->height * image->channels[i]->bytes_per_pixel;
        if (size > 0x7FFFFFFF)
          bit64 = 1;
        valS[i] = image->channels[i]->scale;
      }
  }

  planar_configuration = TIFF_VALUE_PLANAR;
  CATCH(Set_Tiff_Tag(ifd,TIFF_PLANAR_CONFIGURATION,TIFF_SHORT,1,&planar_configuration))

  if (compress == LZW_COMPRESS)
    { predictor   = TIFF_HORIZONTAL_DIFFERENCING;
      compression = TIFF_VALUE_LZW;
    }
  else if (compress == PACKBITS_COMPRESS)
    { predictor   = TIFF_NO_PREDICTION;
      compression = TIFF_VALUE_PACK_BITS;
    }
  else
    { predictor   = TIFF_NO_PREDICTION;
      compression = TIFF_VALUE_UNCOMPRESSED;
    }
  CATCH(Set_Tiff_Tag(ifd,TIFF_PREDICTOR,TIFF_SHORT,1,&predictor))
  CATCH(Set_Tiff_Tag(ifd,TIFF_COMPRESSION,TIFF_SHORT,1,&compression))

  { int i;

    valS = (uint16 *) Allocate_Tiff_Tag(ifd,TIFF_SAMPLE_FORMAT,TIFF_SHORT,samples_per_pixel);
    CATCH(valS == NULL)
    for (i = 0; i < image->number_channels; i++)
      if (image->channels[i]->type == CHAN_UNSIGNED)
        valS[i] = TIFF_VALUE_UNSIGNED_INTEGER_DATA;
      else if (image->channels[i]->type == CHAN_SIGNED)
        valS[i] = TIFF_VALUE_TWOS_COMPLEMENT_DATA;
      else //  image->channels[i]->type == CHAN_FLOAT
        valS[i] = TIFF_VALUE_IEEE_FLOATING_POINT_DATA;
  }

  photo_samples = 1;
  if (image->channels[0]->interpretation == CHAN_WHITE)
    photometric = TIFF_VALUE_WHITE_IS_ZERO;
  else if (image->channels[0]->interpretation == CHAN_BLACK)
    photometric = TIFF_VALUE_BLACK_IS_ZERO;
  else if (image->channels[0]->interpretation == CHAN_MAPPED)
    { photometric = TIFF_VALUE_RGB_PALETTE;
      CATCH(Set_Tiff_Tag(ifd,TIFF_COLOR_MAP,TIFF_SHORT,3*(1 << image->channels[0]->scale),
                             image->map))
    }
  else if (image->channels[0]->interpretation == CHAN_MASK)
    photometric = TIFF_VALUE_TRANSPARENCY_MASK;
  else //  image->channels[0]->interpretation == CHAN_RED & 1 == CHAN_GREEN & 2 == CHAN_BLUE
    { photometric = TIFF_VALUE_RGB;
      photo_samples = 3;
    }
  CATCH(Set_Tiff_Tag(ifd,TIFF_PHOTOMETRIC_INTERPRETATION,TIFF_SHORT,1,&photometric))

  if (photo_samples < image->number_channels)
    { int i;

      valB = (uint8 *)
               Allocate_Tiff_Tag(ifd,TIFF_EXTRA_SAMPLES,TIFF_BYTE,samples_per_pixel-photo_samples);
      CATCH(valB == NULL)
      for (i = photo_samples; i < image->number_channels; i++)
        if (image->channels[i]->interpretation == CHAN_ALPHA)
          valB[i-photo_samples] = TIFF_VALUE_ALPHA_DATA;  
        else if (image->channels[i]->interpretation == CHAN_MATTE)
          valB[i-photo_samples] = TIFF_VALUE_SOFT_MATTE;
        else if (image->channels[i]->interpretation == CHAN_RED)
          valB[i-photo_samples] = TIFF_VALUE_EXTRA_RED;
        else if (image->channels[i]->interpretation == CHAN_GREEN)
          valB[i-photo_samples] = TIFF_VALUE_EXTRA_GREEN;
        else if (image->channels[i]->interpretation == CHAN_BLUE)
          valB[i-photo_samples] = TIFF_VALUE_EXTRA_BLUE;
        else //  image->channels[i]->interpretation == CHAN_OTHER
          valB[i-photo_samples] = TIFF_VALUE_UNSPECIFIED_DATA;
    }

  if (tile_width <= 1)
    tile_width = image->width;
  if (tile_height <= 1)
    tile_height = image->height;

  if (bit64)
    if (tile_width == image->width)
      { int h = (image->height-1) / tile_height + 1;
        int m = image->number_channels;
        int i, j, b, t;

        CATCH(Set_Tiff_Tag(ifd,TIFF_ROWS_PER_STRIP,TIFF_LONG,1,&tile_height))

        Offsets64 = (uint64 *) Allocate_Tiff_Tag(ifd,TIFF_STRIP_OFFSETS,TIFF_LONG64,h*m);
        CATCH(Offsets64 == NULL)
        Byte_Counts64 = (uint64 *) Allocate_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,TIFF_LONG64,h*m);
        CATCH(Byte_Counts64 == NULL)

        t = 0;
        for (i = 0; i < m; i++)
          { b = image->channels[i]->bytes_per_pixel;
            for (j = 0; j < h; j++)
              { Offsets64[t] = (image->channels[i]->plane - image->channels[minoff]->plane)
                             + image->width * tile_height * j * b;
                if (j == h-1)
                  Byte_Counts64[t++] = tile_width * (image->height - tile_height*j) * b;
                else
                  Byte_Counts64[t++] = tile_width * tile_height * b;
              }
          }
      }
    else
      { int h = (image->height-1) / tile_height + 1;
        int w = (image->width-1) / tile_width + 1;
        int m = image->number_channels;
        int i, j, k, b, t;

        CATCH(Set_Tiff_Tag(ifd,TIFF_TILE_WIDTH,TIFF_LONG,1,&tile_width))
        CATCH(Set_Tiff_Tag(ifd,TIFF_TILE_LENGTH,TIFF_LONG,1,&tile_height))

        Offsets64 = (uint64 *) Allocate_Tiff_Tag(ifd,TIFF_TILE_OFFSETS,TIFF_LONG64,w*h*m);
        CATCH(Offsets64 == NULL)
        Byte_Counts64 = (uint64 *) Allocate_Tiff_Tag(ifd,TIFF_TILE_BYTE_COUNTS,TIFF_LONG64,w*h*m);
        CATCH(Byte_Counts64 == NULL)

        t = 0;
        for (i = 0; i < m; i++)
          { b = image->channels[i]->bytes_per_pixel;
            for (j = 0; j < h; j++)
              for (k = 0; k < w; k++)
                { Offsets64[t] = (image->channels[i]->plane - image->channels[minoff]->plane)
                               + (image->width * tile_height * j + tile_width * k) * b;
                  Byte_Counts64[t++] = tile_width * tile_height * b;
                }
          }
      }
  else
    if (tile_width == image->width)
      { int h = (image->height-1) / tile_height + 1;
        int m = image->number_channels;
        int i, j, b, t;

        CATCH(Set_Tiff_Tag(ifd,TIFF_ROWS_PER_STRIP,TIFF_LONG,1,&tile_height))

        Offsets = (uint32 *) Allocate_Tiff_Tag(ifd,TIFF_STRIP_OFFSETS,TIFF_LONG,h*m);
        CATCH(Offsets == NULL)
        Byte_Counts = (uint32 *) Allocate_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,TIFF_LONG,h*m);
        CATCH(Byte_Counts == NULL)

        t = 0;
        for (i = 0; i < m; i++)
          { b = image->channels[i]->bytes_per_pixel;
            for (j = 0; j < h; j++)
              { Offsets[t] = (image->channels[i]->plane - image->channels[minoff]->plane)
                           + image->width * tile_height * j * b;
                if (j == h-1)
                  Byte_Counts[t++] = tile_width * (image->height - tile_height*j) * b;
                else
                  Byte_Counts[t++] = tile_width * tile_height * b;
              }
          }
      }
    else
      { int h = (image->height-1) / tile_height + 1;
        int w = (image->width-1) / tile_width + 1;
        int m = image->number_channels;
        int i, j, k, b, t;

        CATCH(Set_Tiff_Tag(ifd,TIFF_TILE_WIDTH,TIFF_LONG,1,&tile_width))
        CATCH(Set_Tiff_Tag(ifd,TIFF_TILE_LENGTH,TIFF_LONG,1,&tile_height))

        Offsets = (uint32 *) Allocate_Tiff_Tag(ifd,TIFF_TILE_OFFSETS,TIFF_LONG,w*h*m);
        CATCH(Offsets == NULL)
        Byte_Counts = (uint32 *) Allocate_Tiff_Tag(ifd,TIFF_TILE_BYTE_COUNTS,TIFF_LONG,w*h*m);
        CATCH(Byte_Counts == NULL)

        t = 0;
        for (i = 0; i < m; i++)
          { b = image->channels[i]->bytes_per_pixel;
            for (j = 0; j < h; j++)
              for (k = 0; k < w; k++)
                { Offsets[t] = (image->channels[i]->plane - image->channels[minoff]->plane)
                             + (image->width * tile_height * j + tile_width * k) * b;
                  Byte_Counts[t++] = tile_width * tile_height * b;
                }
          }
      }

  return (ifd);
}

  
//  Move an 8*nbytes + nbits bits from byte stream source to bit stream (target,bitpos).
  
#define PACK_BITS(source,nbits,nbytes,target,bitpos)				\
{ int           b;								\
										\
  if (nbits != 0)								\
    { bitpos += nbits;								\
      if (bitpos == nbits)							\
        *target = ((*source & cmpbits[nbits]) << (8-bitpos));			\
      else if (bitpos <= 8)							\
        { *target |= ((*source & cmpbits[nbits]) << (8-bitpos));		\
          if (bitpos == 8)							\
            { target += 1;							\
              bitpos  = 0;							\
            }									\
        }									\
      else   									\
        { bitpos -= 8;								\
          *target++ |= ((*source & cmpbits[nbits]) >> bitpos);			\
          *target    = ((*source & cmpbits[bitpos]) << (8-bitpos));		\
        }									\
      source += 1;								\
    }										\
  if (bitpos == 0)								\
    for (b = 0; b < nbytes; b++)						\
      *target++ = *source++;							\
  else										\
    for (b = 0; b < nbytes; b++)						\
      { *target++ |= (*source >> bitpos);					\
        *target    = (*source++ << (8-bitpos));					\
      }										\
}


//  Called from Write_Tiff_IFD.
//    It expects to be called first with output != NULL and then a second time with output == NULL
//    When output != NULL, it writes to output the appropriate encoding of the image and
//       as a side-effect sets the byte-counts tag to the sizes of the blocks written.
//    When output == NULL, it resets the byte counts to their former values

int Write_IFD_Image(Tiff_IFD *ifd, FILE *output)
{ shape      *S;
  image_tags *I;

  uint32    width,  height;
  uint32    twidth, theight;
  int       wtiles, htiles;
  int       tiled, compression, prediction;
  int       chanblocks, numblocks, numsamples;
  uint16   *samplebits;
  void     *counts;
  Tiff_Type ctype;

  S = extract_shape(ifd);
  if (S == NULL)
    return (1);
  I = extract_image_tags(ifd,S);
  if (I == NULL)
    return (1);

  if (I->chunk)
    { sprintf(ESTRING,"Cannot write IFDss with TIFF_PLANAR_CONFIGURATION = TIFF_VALUE_CHUNKY");
      return (1);
    }

  width      = S->width;
  height     = S->height;
  numsamples = S->samples_per_pixel;
  samplebits = S->bits_per_sample;

  prediction  = (I->prediction != TIFF_NO_PREDICTION);
  compression = I->compression;
   
  counts = Get_Tiff_Tag(ifd,TIFF_TILE_BYTE_COUNTS,&ctype,&numblocks);
  if (counts == NULL)
    counts  = Get_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,&ctype,&numblocks);

  twidth  = I->twidth;
  theight = I->theight;
  tiled   = ! I->strips;

  wtiles     = (width-1)/twidth + 1;
  htiles     = (height-1)/theight + 1;
  chanblocks = wtiles * htiles;

  if (output == NULL)
    { int    i, j, bytes;
      uint64 area, area1, area2;

      area2 = area1 = twidth * theight;
      if ( ! tiled)
        area2 = twidth * (height - (htiles-1)*theight);

      for (i = 0; i < numsamples; i++)
        { bytes = (samplebits[i]-1)/8+1; 
          if (bytes == 3) bytes = 4;

          area = area2;
          for (j = chanblocks-1; j >= 0; j--)
            { if (ctype == TIFF_SHORT)
                ((uint16 *) counts)[i*chanblocks+j] = area*bytes;
              else if (ctype == TIFF_LONG)
                ((uint32 *) counts)[i*chanblocks+j] = area*bytes;
              else
                ((uint64 *) counts)[i*chanblocks+j] = area*bytes;
              area = area1;
            }
        }

      return;
    }

  { uint8     buffer[X_BUFFER_LEN];
    void     *data;
    int       i;

    data = Get_Tiff_IFD_Image(ifd);

    for (i = 0; i < numblocks; i++)
      { uint64    count, initoff, k;
        uint32    twhang, twidx;
        uint8    *block;
        int       scale, bytes;
        void     *hlimit;
  
        count = I->counts[i];
        block = (uint8 *) (data + I->offsets[i]);
  
        scale  = samplebits[i/chanblocks];
        bytes  = (scale-1)/8+1;
        if (bytes == 3) bytes = 4;

        twidx  = (i % wtiles);
        twhang = twidx*twidth;
        if (twhang > width-twidth)
          twhang -= (width - twidth);
        else
          twhang = 0;
        if (i % chanblocks == 0)
          hlimit = block + (((uint64) height)*width)*bytes;

        initoff = ftello(output);
        if (compression == TIFF_VALUE_LZW)
          Start_LZW_Encoder(output);
        else if (compression == TIFF_VALUE_PACK_BITS)
          Start_PackBits_Encoder(output);
  
        for (k = 0; k < count; k += X_BUFFER_LEN)
  
          { int    buffer_len, moved;
            uint8 *buffer_end;
  
            if (k+X_BUFFER_LEN > count)
              buffer_len = count-k;
            else
              buffer_len = X_BUFFER_LEN;
            buffer_end = buffer + buffer_len;
            moved      = 0;
  
            if (tiled)

              { moved = 1;

                if (bytes == 1)
                  { uint8 *p = block + (k/twidth)*width + (k%twidth);
                    uint8 *lim = p + (twidth - k%twidth);
                    uint8 *wim = lim - twhang;
                    uint8 *w;
  
                    w = buffer;
                    while (w < buffer_end)
                      if (p < wim)
                        *w++ = *p++;
                      else if (p++ < lim)
                        *w++ = 0;
                      else
                        { lim += width;
                          wim += width;
                          p    = lim-twidth;
                          if (p >= (uint8 *) hlimit)
                            { while (w < buffer_end)
                                *w++ = 0;
                              break;
                            }
                        }
                  }

                else if (bytes == 2)
                  { uint16 *p = ((uint16 *) block) + ((k/2)/twidth)*width + ((k/2)%twidth);
                    uint16 *lim = p + (twidth - (k/2)%twidth);
                    uint16 *wim = lim - twhang;
                    uint16 *w;
  
                    w = (uint16 *) buffer;
                    while (w < (uint16 *) buffer_end)
                      if (p < wim)
                        *w++ = *p++;
                      else if (p++ < lim)
                        *w++ = 0;
                      else
                        { lim += width;
                          wim += width;
                          p    = lim-twidth;
                          if (p >= (uint16 *) hlimit)
                            { while (w < (uint16 *) buffer_end)
                                *w++ = 0;
                              break;
                            }
                        }
                  }

                else //  bytes == 4
                  { uint32 *p = ((uint32 *) block) + ((k/4)/twidth)*width + ((k/4)%twidth);
                    uint32 *lim = p + (twidth - (k/4)%twidth);
                    uint32 *wim = lim - twhang;
                    uint32 *w;
  
                    w = (uint32 *) buffer;
                    while (w < (uint32 *) buffer_end)
                      if (p < wim)
                        *w++ = *p++;
                      else if (p++ < lim)
                        *w++ = 0;
                      else
                        { lim += width;
                          wim += width;
                          p    = lim-twidth;
                          if (p >= (uint32 *) hlimit)
                            { while (w < (uint32 *) buffer_end)
                                *w++ = 0;
                              break;
                            }
                        }
                  }
              }

            if (prediction)    //  Difference data if required
  
              { void  *source, *lastp, *contp;
                uint32 period, restart;

                moved = 1;

                if (tiled)
                  { source  = buffer;
                    period  = twidth;
                    contp   = block + (((k/bytes)/twidth)*width-1)*bytes;
                    lastp   = contp + ((k/bytes)%twidth)*bytes;
                    restart = (twidx == 0);
                  }
                else
                  { source  = block+k;
                    period  = width;
                    contp   = source;
                    lastp   = source - bytes;
                    restart = 1;
                  }
  
                if (bytes == 1)
  
                  { uint8  next, last;
                    uint8 *rowbeg;
                    uint8 *conptr = (uint8 *) contp;
                    uint8 *base = (uint8 *) source;
                    char  *diff = (char *) buffer;
                    uint32 mod;
  
                    mod = k % period;
                    if (mod == 0)
                      rowbeg = base;
                    else
                      { rowbeg  = base + (period - mod);
                        conptr += width;
                        last    = *((uint8 *) lastp);
                      }
  
                    while (diff < (char *) buffer_end)
                      { if (base == rowbeg)
                          { if (restart)
                              last = 0;
                            else
                              { last    = *conptr;
                                conptr += width;
                              }
                            rowbeg += period;
                          }
                        next    = *base++;
                        *diff++ = next - last;
                        last    = next;
                      }
                  }
      
                else if (bytes == 2)
  
                  { uint16  next, last;
                    uint16 *rowbeg;
                    uint16 *conptr = (uint16 *) contp;
                    uint16 *base = (uint16 *) source;
                    short  *diff = (short *) buffer;
                    uint32  mod;

                    mod = (k/2) % period;
                    if (mod == 0)
                      rowbeg = base;
                    else
                      { rowbeg  = base + (period - mod);
                        conptr += width;
                        last    = *((uint16 *) lastp);
                      }
  
                    while (diff < (short *) buffer_end)
                      { if (base == rowbeg)
                          { if (restart)
                              last = 0;
                            else
                              { last    = *conptr;
                                conptr += width;
                              }
                            rowbeg += period;
                          }
                        next    = *base++;
                        *diff++ = next - last;
                        last    = next;
                      }
                  }
     
                else //  bytes == 4
  
                  { uint32  next, last;
                    uint32 *rowbeg;
                    uint32 *conptr = (uint32 *) contp;
                    uint32 *base = (uint32 *) source;
                    int    *diff = (int *) buffer;
                    uint32  mod;

                    mod = (k/4) % period;
                    if (mod == 0)
                      rowbeg = base;
                    else
                      { rowbeg  = base + (period - mod);
                        conptr += width;
                        last    = *((uint32 *) lastp);
                      }
  
                    while (diff < (int *) buffer_end)
                      { if (base == rowbeg)
                          { if (restart)
                              last = 0;
                            else
                              { last    = *conptr;
                                conptr += width;
                              }
                            rowbeg += period;
                          }
                        next    = *base++;
                        *diff++ = next - last;
                        last    = next;
                      }
                  }
              }
  
            //  Endian flip multi-byte data if required
  
            if (bytes == 2)
  
              { if (scale < 16 && ! Native_Endian())
  
                  { uint8 *w;
                    if (moved)
                      { uint8  t;
                        for (w = buffer; w < buffer_end; w += 2)
                          { t = w[0];
                            w[0] = w[1];
                            w[1] = t;
                          }
                      }
                    else
                      { uint8 *v = block+k;
                        for (w = buffer; w < buffer_end; w += 2)
                          { w[0] = v[1];
                            w[1] = v[0];
                            v += 2;
                          }
                        moved = 1;
                      }
                  }
              }
  
            else if (bytes == 4)
  
              { if (scale < 32 && ! Native_Endian())
  
                  { uint8 *w;

                    if (moved)
                      { uint8  t;
                        for (w = buffer; w < buffer_end; w += 4)
                          { t = w[0];
                            w[0] = w[3];
                            w[3] = t;
                            t = w[1];
                            w[1] = w[2];
                            w[2] = t;
                          }
                      }
                    else
                      { uint8 *v = block+k;
                        for (w = buffer; w < buffer_end; w += 4)
                          { w[0] = v[3];
                            w[1] = v[2];
                            w[2] = v[1];
                            w[3] = v[0];
                            v += 4;
                          }
                        moved = 1;
                      }
                  }
              }
  
            //  In-place bit-packing if non-byte boundary sample size
  
            if (scale % 8 != 0 || scale == 24)
              { int    bp8 = (scale & 0x7);
                int    bp3 = (scale >> 3);
                int    y, pos;
                uint8 *src;
                uint8 *trg;
                uint8 *org;
  
                if (moved)
                  src = buffer;
                else
                  src = block+k;
                trg   = buffer;
                moved = 1;
  
                pos = 0;
                if (16 < scale && scale <= 24)
                  for (y = 0; y < buffer_len; y += bytes)
                    { src += 1;
                      PACK_BITS(src,bp8,bp3,trg,pos)
                    }
                else
                  for (y = 0; y < buffer_len; y += bytes)
                    PACK_BITS(src,bp8,bp3,trg,pos)
                if (pos != 0)
                  trg++;
                buffer_len = trg - buffer;
                buffer_end = trg;
              }
  
            //  Compress if requested
  
            { uint8 *src;
  
              if (moved)
                src = buffer;
              else
                src = block+k;
              if (compression == TIFF_VALUE_LZW)
                { int y;
                  for (y = 0; y < buffer_len; y++)
                    Put_LZW(output,src[y]);
                }
              else if (compression == TIFF_VALUE_PACK_BITS)
                { int y;
                  for (y = 0; y < buffer_len; y++)
                    Put_PackBits(output,src[y]);
                }
              else
                fwrite(src,1,buffer_len,output);
            }
          }
  
        if (compression == TIFF_VALUE_LZW)
          Put_LZW(output,-1);
        else if (compression == TIFF_VALUE_PACK_BITS)
          Put_PackBits(output,-1);
  
        count = ftello(output) - initoff;
  
        if (ctype == TIFF_SHORT)
          ((uint16 *) counts)[i] = count;
        else if (ctype == TIFF_LONG)
          ((uint32 *) counts)[i] = count;
        else
          ((uint64 *) counts)[i] = count;
      }
  }

  return (0);
}


/****************************************************************************************
 *                                                                                      *
 *  CHANNEL SCALING ROUTINES                                                            *
 *                                                                                      *
 ****************************************************************************************/

#define UPSIZE_AREA(S_TYPE,T_TYPE)		\
{ S_TYPE *source = (S_TYPE *) (channel->plane);	\
  T_TYPE *target = (T_TYPE *) (channel->plane);	\
  for (i = area-1; i >= 0; i--)			\
    target[i] = source[i];			\
}

#define DOWNSIZE_AREA(S_TYPE,T_TYPE)		\
{ S_TYPE *source = (S_TYPE *) (channel->plane);	\
  T_TYPE *target = (T_TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    target[i] = source[i];			\
}

#define SHIFTUP_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    target[i] <<= delta;			\
}

#define SHIFTDOWN_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    target[i] >>= delta;			\
}

void Scale_Tiff_Channel(Tiff_Channel *channel, int scale)
{ uint64 i, area;
  int    bps, delta;

  if (scale == channel->scale || channel->type == CHAN_FLOAT || channel->plane == NULL) return;

  bps = (scale-1)/8+1;
  if (bps == 3)
    bps = 4;

  area = channel->width * ((uint64) channel->height);

  if (scale > channel->scale)

    { delta = scale - channel->scale;

      if (bps > channel->bytes_per_pixel)
        { if (channel->bytes_per_pixel == 1)
            if (bps == 2)
              UPSIZE_AREA(uint8, uint16)
            else // bps == 4
              UPSIZE_AREA(uint8, uint32)
          else // channel->bytes_per_pixel == 2 && bps == 4
            UPSIZE_AREA(uint16, uint32)
        }

      if (channel->type == CHAN_UNSIGNED)
        if (bps == 1)
          SHIFTUP_AREA(uint8)
        else if (bps == 2)
          SHIFTUP_AREA(uint16)
        else //  bps == 4
          SHIFTUP_AREA(uint32)
      else // channel->type == CHAN_SIGNED
        if (bps == 1)
          SHIFTUP_AREA(char)
        else if (bps == 2)
          SHIFTUP_AREA(short)
        else //  bps == 4
          SHIFTUP_AREA(int)
    }

  else

    { delta = channel->scale - scale;

      if (channel->type == CHAN_UNSIGNED)
        if (channel->bytes_per_pixel == 1)
          SHIFTDOWN_AREA(uint8)
        else if (channel->bytes_per_pixel == 2)
          SHIFTDOWN_AREA(uint16)
        else //  channel->bytes_per_pixel == 4
          SHIFTDOWN_AREA(uint32)
      else // channel->type == CHAN_SIGNED
        if (channel->bytes_per_pixel == 1)
          SHIFTDOWN_AREA(char)
        else if (channel->bytes_per_pixel == 2)
          SHIFTDOWN_AREA(short)
        else //  channel->bytes_per_pixel == 4
          SHIFTDOWN_AREA(int)

      if (bps < channel->bytes_per_pixel)
        { if (channel->bytes_per_pixel == 4)
            if (bps == 2)
              DOWNSIZE_AREA(uint32, uint16)
            else // bps == 1
              DOWNSIZE_AREA(uint32, uint8)
          else // channel->bytes_per_pixel == 2 && bps == 1
            DOWNSIZE_AREA(uint16, uint8)
        }
    }

  channel->scale = scale;
  channel->bytes_per_pixel = bps;
}

void Scale_Tiff_Image(Tiff_Image *image, int scale)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    Scale_Tiff_Channel(image->channels[i],scale);
}

#define RANGE_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    { TYPE v = target[i];			\
      if (v < mn)				\
        mn = v;					\
      else if (v > mx)				\
        mx = v;					\
    }						\
  *minval = mn;					\
  *maxval = mx;					\
}

void Range_Tiff_Channel(Tiff_Channel *channel, double *minval, double *maxval)
{ uint64 area = channel->width * ((uint64) channel->height);
  uint64 i;

  if (channel->plane == NULL)
    { *minval = *maxval = 0.;
      return;
    }
  if (channel->type == CHAN_FLOAT)
    { float mn = 0.;
      float mx = 0.;
      RANGE_AREA(float)
    }
  else if (channel->type == CHAN_UNSIGNED)
    { uint32 mn = 0;
      uint32 mx = 0;
      if (channel->bytes_per_pixel == 1)
        RANGE_AREA(uint8)
      else if (channel->bytes_per_pixel == 2)
        RANGE_AREA(uint16)
      else //  channel->bytes_per_pixel == 4
        RANGE_AREA(uint32)
    }
  else //  channel->type == CHAN_SIGNED
    { int mn = 0;
      int mx = 0;
      if (channel->bytes_per_pixel == 1)
        RANGE_AREA(signed char)
      else if (channel->bytes_per_pixel == 2)
        RANGE_AREA(short)
      else //  channel->bytes_per_pixel == 4
        RANGE_AREA(int)
    }
}

#define SHIFT_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  if (shift > 0)				\
    for (i = 0; i < area; i++)			\
      target[i] <<= shift;			\
  else						\
    for (i = 0; i < area; i++)			\
      target[i] >>= chift;			\
}

void Shift_Tiff_Channel(Tiff_Channel *channel, int shift)
{ uint64 area = channel->width * ((uint64) channel->height);
  uint64 i;
  int    chift;

  if (shift == 0 || channel->type == CHAN_FLOAT || channel->plane == NULL) return;

  chift = -shift;
  if (channel->type == CHAN_UNSIGNED)
    if (channel->bytes_per_pixel == 1)
      SHIFT_AREA(uint8)
    else if (channel->bytes_per_pixel == 2)
      SHIFT_AREA(uint16)
    else //  channel->bytes_per_pixel == 4
      SHIFT_AREA(uint32)
  else
    if (channel->bytes_per_pixel == 1)
      SHIFT_AREA(signed char)
    else if (channel->bytes_per_pixel == 2)
      SHIFT_AREA(short)
    else //  channel->bytes_per_pixel == 4
      SHIFT_AREA(int)
}

void Shift_Tiff_Image(Tiff_Image *image, int shift)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    Shift_Tiff_Channel(image->channels[i],shift);
}


/****************************************************************************************
 *                                                                                      *
 *  CHANNEL HISTOGRAM ROUTINES                                                          *
 *                                                                                      *
 ****************************************************************************************/

Tiff_Histogram *Histogram_Tiff_Channel(Tiff_Channel *channel)
{ uint64          i;
  int             bit;
  Tiff_Histogram *histogram;

  if (EXCEPTION)
    { sprintf(ESOURCE,"Histogram_Tiff_Channel");
      return (NULL);
    }
  else
    histogram = new_tiff_histogram(NULL);

  uint64 *array = histogram->counts;
  uint64  area  = channel->width * ((uint64) channel->height); 
  int     bpp   = channel->bytes_per_pixel;

  for (i = 0; i < 512; i++)
    array[i] = 0;

  histogram->total = area;

  if (channel->plane == NULL)
    { histogram->bitshift = 0;
      return (histogram);
    }

  if (bpp == 1)
    { uint8 *target = (uint8 *) (channel->plane);
      for (i = 0; i < area; i++)
        array[target[i]] += 1;
      histogram->bitshift = 0;
      return (histogram);
    }

  { uint32 *target = (uint32 *) (channel->plane);                          
    uint64  words  = (area * bpp) / 4;
    uint32  chord;
    uint32  profile;

    chord = 0;
    for (i = 0; i < words; i ++)
      chord |= target[i]; 
  
    profile = 0;
    if (bpp == 1) 
      for (i = 0; i < 4; i++)
        profile |= ((uint8 *) &chord)[i];
    else if (bpp == 2) 
      for (i = 0; i < 2; i++)
        profile |= ((uint16 *) &chord)[i];
    else //  bpp == 4
      profile = chord;

    for (bit = 0; bit < 32; bit++)
      { if (profile == 0)
          break;
        profile >>= 1;
      }
  }

  if (bpp == 2)
    { uint16 *target = (uint16 *) (channel->plane);
      if (bit <= 9)
        { for (i = 0; i < area; i++)
            array[target[i]] += 1;
          bit = 9;
        }
      else
        { int shift = bit-9;
          for (i = 0; i < area; i++)
            array[target[i] >> shift] += 1;
        }
    }
  else //  bpp == 4
    { uint32 *target = (uint32 *) (channel->plane);
      if (bit <= 9)
        { for (i = 0; i < area; i++)
            array[target[i]] += 1;
          bit = 9;
        }
      else
        { int shift = bit-9;
          for (i = 0; i < area; i++)
            array[target[i] >> shift] += 1;
        }
    }

  histogram->bitshift = bit-9;

  return (histogram);
}

int Histogram_Tiff_Image_Channels(Tiff_Image *image)
{ int i;

  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i]->histogram == NULL)
      if ((image->channels[i]->histogram = Histogram_Tiff_Channel(image->channels[i])) == NULL)
        { sprintf(ESOURCE,"Histogram_Tiff_Channels");
          return (1);
        }
  return (0);
}

void Tiff_Histogram_Merge(Tiff_Histogram *h1, Tiff_Histogram *h2)
{ uint64 *array1, *array2;
  int     bit1,    bit2;
  int     block;
  int     j, k, b;
 
  array1 = h1->counts;
  array2 = h2->counts;

  bit1 = h1->bitshift;
  bit2 = h2->bitshift;

  if (bit2 > bit1)
    { block = (1 << (bit2-bit1));
      for (j = 0, b = 0; j < 512; j += block, b += 1)
        { array1[b] = array1[j] + array2[b];
          for (k = j+1; k < j+block; k++)
            array1[b] += array1[k];
        }
      for (j = b; j < 512; j++)
        array1[j] = array2[j];
      h1->bitshift = bit2;
    }
  else if (bit2 < bit1)
    { block = (1 << (bit1-bit2)); 
      for (j = 0, b = 0; j < 512; j += block, b += 1)
        { array1[b] += array2[j];
          for (k = j+1; k < j+block; k++)
            array1[b] += array2[k];
        }
    }
  else
    for (j = 0; j < 512; j++)
      array1[j] += array2[j];

  h1->total += h2->total;
} 


/****************************************************************************************
 *                                                                                      *
 *  DEVELOPMENT AREA: desired >2Gb files/images                                         *
 *                    half-floating point numbers                                       *
 *                    jpeg encoding                                                     *
 *                    fax compression                                                   *
 *                                                                                      *
 ****************************************************************************************/

// half-precision to float converter (unused, untested)

float halfp_2_fullp(uint16 halfp)
{ static float  fullp;
  uint32 *number = (uint32 *) (&fullp);

  uint32 mantissa = halfp & 0x03ff;
  uint32 exponent = halfp & 0x7c00;
  uint32 sign     = halfp & 0x8000;

  if (exponent == 0)
    { if (mantissa != 0)        // denormalized number
        { exponent = 0x1c000; 
          while ((mantissa & 0x200) == 0)   // normalize
            { mantissa <<= 1;
              exponent -=  0x400;
            }
          mantissa = ((mantissa << 1) & 0x3ff);
        }
                                // else 0
    }
  else if (exponent == 0x7c00)  // infinity or NAN
    exponent = 0xcfc00;
  else                          // normalized number
    exponent += 0x1c000;

  *number = (sign << 16) | ((exponent | mantissa) << 13); 
  return (fullp);
}

typedef struct
  { int   jumpto;
    uint8 runs[4];
  } Fax_State;

int CCIT_Decoder(unsigned char *stream, int stream_len, unsigned char *decode)
{ static int        firstime = 1;
  static Fax_State *fax_table;

  uint8 *dbyte = decode;
  uint8 *sbyte = stream;
  uint8 *send  = stream+stream_len;

  if (firstime)
    { int i;

      firstime = 0;
      //  Make the fax table
    }

  { int        white, i, c;
    Fax_State *state, *next;

    white = 1;
    state = fax_table;
    while (sbyte < send)
      { next  = state + *sbyte++;
        state = fax_table + next->jumpto;
        for (i = 0; (c = next->runs[i]) != 0; i++)
          { // Output c bits of black or white.
            white = !white;
            if (i >= 3) break;
          }
      }
  }
} 
