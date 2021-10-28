/*****************************************************************************************\
*                                                                                         *
*  Histogram Data Abstraction and Array Statistics Routines                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  December 2008                                                                 *
*                                                                                         *
*  (c) December 20, '09, Dr. Gene Myers and Howard Hughes Medical Institute               *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>

#include "utilities.h"
#include "array.h"
#include "histogram.h"



/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM SPACE MANAGEMENT ROUTINES AND PRIMARY GENERATOR                           *
 *                                                                                      *
 ****************************************************************************************/

// Awk-generated (manager.awk) Array memory management


static Value_Type a2v_type[] = { UVAL, UVAL, UVAL, UVAL, IVAL, IVAL, IVAL, IVAL, RVAL, RVAL };

static inline int histogram_nsize(Histogram *h)
{ return (sizeof(Size_Type)*h->nbins); }


typedef struct __Histogram
  { struct __Histogram *next;
    struct __Histogram *prev;
    int                 refcnt;
    int                 nsize;
    Histogram           histogram;
  } _Histogram;

static _Histogram *Free_Histogram_List = NULL;
static _Histogram *Use_Histogram_List  = NULL;
static _Histogram  Histogram_Proto;

static int Histogram_Offset = ((char *) &(Histogram_Proto.histogram)) - ((char *) &Histogram_Proto);
static int Histogram_Inuse  = 0;

int Histogram_Refcount(Histogram *histogram)
{ _Histogram *object = (_Histogram *) (((char *) histogram) - Histogram_Offset);
  return (object->refcnt);
}

static inline void allocate_histogram_counts(Histogram *histogram, int nsize, char *routine)
{ _Histogram *object = (_Histogram *) (((char *) histogram) - Histogram_Offset);
  if (object->nsize < nsize)
    { histogram->counts = Guarded_Realloc(histogram->counts,nsize,routine);
      object->nsize = nsize;
    }
}

static inline int sizeof_histogram_counts(Histogram *histogram)
{ _Histogram *object = (_Histogram *) (((char *) histogram) - Histogram_Offset);
  return (object->nsize);
}

static inline Histogram *new_histogram(int nsize, char *routine)
{ _Histogram *object;
  Histogram  *histogram;

  if (Free_Histogram_List == NULL)
    { object = (_Histogram *) Guarded_Realloc(NULL,sizeof(_Histogram),routine);
      histogram = &(object->histogram);
      object->nsize = 0;
      histogram->counts = NULL;
    }
  else
    { object = Free_Histogram_List;
      Free_Histogram_List = object->next;
      histogram = &(object->histogram);
    }
  Histogram_Inuse += 1;
  object->refcnt = 1;
  if (Use_Histogram_List != NULL)
    Use_Histogram_List->prev = object;
  object->next = Use_Histogram_List;
  object->prev = NULL;
  Use_Histogram_List = object;
  allocate_histogram_counts(histogram,nsize,routine);
  return (histogram);
}

static inline Histogram *copy_histogram(Histogram *histogram)
{ Histogram *copy = new_histogram(histogram_nsize(histogram),"Copy_Histogram");
  void *_counts = copy->counts;
  *copy = *histogram;
  copy->counts = _counts;
  if (histogram->counts != NULL)
    memcpy(copy->counts,histogram->counts,histogram_nsize(histogram));
  return (copy);
}

Histogram *Copy_Histogram(Histogram *histogram)
{ return ((Histogram *) copy_histogram(histogram)); }

static inline void pack_histogram(Histogram *histogram)
{ _Histogram *object  = (_Histogram *) (((char *) histogram) - Histogram_Offset);
  if (object->nsize > histogram_nsize(histogram))
    { object->nsize = histogram_nsize(histogram);
      if (object->nsize != 0)
        histogram->counts = Guarded_Realloc(histogram->counts,
                                            object->nsize,"Pack_Histogram");
      else
        { free(histogram->counts);
          histogram->counts = NULL;
        }
    }
}

Histogram *Pack_Histogram(Histogram *histogram)
{ pack_histogram(histogram);
  return (histogram);
}

Histogram *Inc_Histogram(Histogram *histogram)
{ _Histogram *object  = (_Histogram *) (((char *) histogram) - Histogram_Offset);
  object->refcnt += 1;
  return (histogram);
}

static inline void free_histogram(Histogram *histogram)
{ _Histogram *object  = (_Histogram *) (((char *) histogram) - Histogram_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Histogram\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Histogram_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Histogram_List;
  Free_Histogram_List = object;
  Histogram_Inuse -= 1;
}

void Free_Histogram(Histogram *histogram)
{ free_histogram(histogram); }

static inline void kill_histogram(Histogram *histogram)
{ _Histogram *object  = (_Histogram *) (((char *) histogram) - Histogram_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Histogram\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Histogram_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  if (object->nsize != 0)
    free(histogram->counts);
  free(((char *) histogram) - Histogram_Offset);
  Histogram_Inuse -= 1;
}

void Kill_Histogram(Histogram *histogram)
{ kill_histogram(histogram); }

static inline void reset_histogram()
{ _Histogram *object;
  Histogram  *histogram;
  while (Free_Histogram_List != NULL)
    { object = Free_Histogram_List;
      Free_Histogram_List = object->next;
      histogram = &(object->histogram);
      if (object->nsize != 0)
        free(histogram->counts);
      free(object);
    }
}

void Reset_Histogram()
{ reset_histogram(); }

int Histogram_Usage()
{ return (Histogram_Inuse); }

void Histogram_List(void (*handler)(Histogram *))
{ _Histogram *a, *b;
  for (a = Use_Histogram_List; a != NULL; a = b)
    { b = a->next;
      handler((Histogram *) &(a->histogram));
    }
}

static inline Histogram *read_histogram(FILE *input)
{ char name[9];
  fread(name,9,1,input);
  if (strncmp(name,"Histogram",9) != 0)
    return (NULL);
  Histogram *obj = new_histogram(0,"Read_Histogram");
  fread(obj,sizeof(Histogram),1,input);
  obj->counts = NULL;
  if (histogram_nsize(obj) != 0)
    { allocate_histogram_counts(obj,histogram_nsize(obj),"Read_Histogram");
      fread(obj->counts,histogram_nsize(obj),1,input);
    }
  return (obj);
}

Histogram *Read_Histogram(FILE *input)
{ return ((Histogram *) read_histogram(input)); }

static inline void write_histogram(Histogram *histogram, FILE *output)
{ fwrite("Histogram",9,1,output);
  fwrite(histogram,sizeof(Histogram),1,output);
  if (histogram_nsize(histogram) != 0)
    fwrite(histogram->counts,histogram_nsize(histogram),1,output);
}

void Write_Histogram(Histogram *histogram, FILE *output)
{ write_histogram(histogram,output); }

  /*  Generate a histogram of array a with nbins of width bucket where the smallest bucket's
      smallest boundary is offset (see data descriptor comments above).  The type of values   
      given for bucket and offset should be congruent with the type of a.  When nbins or 
      bucket or both are zero then the histogram buckets are set up as follows based on the 
      range [min,max] of values in a: 

         nbins = 0 & bucket = 0:
            buckets are of length *1* and cover the range of values
              in a starting at *floor(min)*

         nbins = 0 & bucket != 0:
            buckets are of length *bucket* and cover the range of values
              in a starting at *floor(min)*

         nbins != 0 & bucket = 0:
            The bucket width is the smallest number of the form [1,2,5]*10^a for which nbins
            of this size cover the range [min,max].  The buckets start at the first multiple
            of the bucket width <= min and nbins is adjusted downwards so that the binning
            just covers the required range [min,max].

         nbins != 0 & bucket != 0
            The implied bucketing is used as specified and any values not in the implied range
            are not added to the histogram, i.e. the total count of the histogram can be less
            then the size of a.
  */

static void set_histogram_mean(Histogram *h)
{ Size_Type *count, size;
  int        i;
  double     sum, b, u;

  count = h->counts;
  
  switch (h->type) {
      case UVAL:
        b = h->bucket.uval;
          u = h->offset.uval + .5*b - .5;
        break;
      case IVAL:
        b = h->bucket.ival;
          u = h->offset.ival + .5*b - .5;
        break;
      case RVAL:
        b = h->bucket.rval;
          u = h->offset.rval + .5*b;
        break;
  }

  sum  = 0.;
  size = 0;
  for (i = 0; i < h->nbins; i++)
    { sum  += count[i] * u;
      u    += b;
      size += count[i];
    }

  h->mean  = sum / size;
  h->total = size;
}

Histogram *Histogram_Array(Array_Or_Slice *o, int nbins, Value bucket, Value offset)
{ Array        *a = (Array *) o;
  Slice        *s = (Slice *) o;
  Range_Bundle *rng;
  Histogram    *h;
  Indx_Type     p, f;
  Size_Type     size;
  Size_Type    *count;
  int           clip, aslice;

  if (aslice = Is_Slice(o))
    a = Slice_Array(s);

  rng = Array_Range(o);

  switch (a2v_type[a->type]) {
      case UVAL:
        if (nbins == 0)
          { uint64 rti;
            if (bucket.uval == 0)
              bucket.uval = 1;
              offset = rng->minval;
            rti = (rng->maxval.uval - offset.uval) / bucket.uval + 1;
            if (rti > 0x7FFFFFFF)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Array)\n");
                exit (1);
              }
            nbins = rti;
          }
        else if (bucket.uval == 0)
          { double bwide = (rng->maxval.uval - rng->minval.uval) / (1.*nbins);
            if (bwide == 0.)
              bucket.uval = 1;
            else
              { uint64 x = 1;
                while (10*x <= bwide)
                  x = 10*x;
                if (x < bwide)
                  if (2*x < bwide)
                    if (5*x < bwide)
                      x = 10*x;
                    else
                      x = 5*x;
                  else
                    x = 2*x;
                bucket.uval = x;
              }
            offset.uval = floor(rng->minval.uval / bucket.uval) * bucket.uval;
            nbins = (rng->maxval.uval - offset.uval) / bucket.uval;
            if (offset.uval + nbins*bucket.uval <= rng->maxval.uval)
              nbins += 1;
          } 
        clip = (offset.uval > rng->minval.uval || offset.uval + bucket.uval * nbins <= rng->maxval.uval);
        break;
      case IVAL:
        if (nbins == 0)
          { uint64 rti;
            if (bucket.ival == 0)
              bucket.ival = 1;
              offset = rng->minval;
            rti = (rng->maxval.ival - offset.ival) / bucket.ival + 1;
            if (rti > 0x7FFFFFFF)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Array)\n");
                exit (1);
              }
            nbins = rti;
          }
        else if (bucket.ival == 0)
          { double bwide = (rng->maxval.ival - rng->minval.ival) / (1.*nbins);
            if (bwide == 0.)
              bucket.ival = 1;
            else
              { int64 x = 1;
                while (10*x <= bwide)
                  x = 10*x;
                if (x < bwide)
                  if (2*x < bwide)
                    if (5*x < bwide)
                      x = 10*x;
                    else
                      x = 5*x;
                  else
                    x = 2*x;
                bucket.ival = x;
              }
            offset.ival = floor(rng->minval.ival / bucket.ival) * bucket.ival;
            nbins = (rng->maxval.ival - offset.ival) / bucket.ival;
            if (offset.ival + nbins*bucket.ival <= rng->maxval.ival)
              nbins += 1;
          } 
        clip = (offset.ival > rng->minval.ival || offset.ival + bucket.ival * nbins <= rng->maxval.ival);
        break;
      case RVAL:
        if (nbins == 0)
          { uint64 rti;
            if (bucket.rval == 0)
              bucket.rval = 1;
              offset.rval = floor(rng->minval.rval);
            rti = (rng->maxval.rval - offset.rval) / bucket.rval + 1;
            if (rti > 0x7FFFFFFF)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Array)\n");
                exit (1);
              }
            nbins = rti;
          }
        else if (bucket.rval == 0)
          { double bwide = (rng->maxval.rval - rng->minval.rval) / (1.*nbins);
            if (bwide == 0.)
              bucket.rval = 1;
            else
              { double x = pow(10.,floor(log10(bwide)));
                if (x < bwide)
                  if (2*x < bwide)
                    if (5*x < bwide)
                      x = 10*x;
                    else
                      x = 5*x;
                  else
                    x = 2*x;
                bucket.rval = x;
              }
            offset.rval = floor(rng->minval.rval / bucket.rval) * bucket.rval;
            nbins = (rng->maxval.rval - offset.rval) / bucket.rval;
            if (offset.rval + nbins*bucket.rval <= rng->maxval.rval)
              nbins += 1;
          } 
        clip = (offset.rval > rng->minval.rval || offset.rval + bucket.rval * nbins <= rng->maxval.rval);
        break;
  }

  h = new_histogram(nbins*sizeof(Size_Type),"Histogram_Array");

  h->type   = a2v_type[a->type];
  h->bucket = bucket;
  h->offset = offset;
  h->nbins  = nbins;

  count = h->counts;
  size  = a->size;

  for (p = 0; p < nbins; p++)
    count[p] = 0;

  if (aslice)
    f = Set_Slice_To_Last(s);

  switch (a->type) {
      case UINT8:
        { uint8 *v = AUINT8(a);
          uint8  o = offset.uval;
          uint8  b = bucket.uval;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case UINT16:
        { uint16 *v = AUINT16(a);
          uint16  o = offset.uval;
          uint16  b = bucket.uval;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case UINT32:
        { uint32 *v = AUINT32(a);
          uint32  o = offset.uval;
          uint32  b = bucket.uval;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case UINT64:
        { uint64 *v = AUINT64(a);
          uint64  o = offset.uval;
          uint64  b = bucket.uval;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case INT8:
        { int8 *v = AINT8(a);
          int8  o = offset.ival;
          int8  b = bucket.ival;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
              if (o < 0)
                { if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) (v[p]-o)] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) (v[p]-o)] += 1;
                }
              else
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case INT16:
        { int16 *v = AINT16(a);
          int16  o = offset.ival;
          int16  b = bucket.ival;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
              if (o < 0)
                { if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) (v[p]-o)] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) (v[p]-o)] += 1;
                }
              else
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case INT32:
        { int32 *v = AINT32(a);
          int32  o = offset.ival;
          int32  b = bucket.ival;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
              if (o < 0)
                { if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) (v[p]-o)] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) (v[p]-o)] += 1;
                }
              else
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case INT64:
        { int64 *v = AINT64(a);
          int64  o = offset.ival;
          int64  b = bucket.ival;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
              if (o < 0)
                { if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) (v[p]-o)] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) (v[p]-o)] += 1;
                }
              else
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case FLOAT32:
        { float32 *v = AFLOAT32(a);
          float32  o = offset.rval;
          float32  b = bucket.rval;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
              if (o < 0)
                { if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) (v[p]-o)] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) (v[p]-o)] += 1;
                }
              else
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
      case FLOAT64:
        { float64 *v = AFLOAT64(a);
          float64  o = offset.rval;
          float64  b = bucket.rval;
          if (clip)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  { int i = floor((v[p]-o)/b);
                    if (0 <= i && i < nbins)
                      count[i] += 1;
                  }
            }
          else if (b == 1)
              if (o < 0)
                { if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) (v[p]-o)] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) (v[p]-o)] += 1;
                }
              else
                { count -= (int) o;
                  if (aslice)
                    for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                      { count[(int) v[p]] += 1;
                        if (p == f) break;
                      }
                  else
                    for (p = 0; p < size; p++)
                      count[(int) v[p]] += 1;
                }
          else if (o == 0)
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) (v[p]/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) (v[p]/b)] += 1;
            }
          else
            { if (aslice)
                for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s))
                  { count[(int) ((v[p]-o)/b)] += 1;
                    if (p == f) break;
                  }
              else
                for (p = 0; p < size; p++)
                  count[(int) ((v[p]-o)/b)] += 1;
            }
          break;
        }
  }

  set_histogram_mean(h);

  return (h);
}

  /*  Generate a histogram based on the histogram h consisting of the bins in the interval
        [min,max).  min and max need not be between 0 and h->nbins but it must be that min < max.
        If min < 0 or max > h->nbins then g's domain will be expanded as necessary to cover
        the implied range of [Bucket2Value(min),Bucket2Value(max)].
  */

Histogram *Histogram_Slice(Histogram *h, int min, int max)
{ Histogram *g;
  int        nbins;
  Value      off;
  int        i;

  switch (h->type) {
      case UVAL:
        off.uval = h->offset.uval + h->bucket.uval * min;
        break;
      case IVAL:
        off.ival = h->offset.ival + h->bucket.ival * min;
        break;
      case RVAL:
        off.rval = h->offset.rval + h->bucket.rval * min;
        break;
  }

  nbins = max-min;
  if (nbins <= 0)
    { fprintf(stderr,"Requested bucket interval is empty (Histogram_Slice)");
      exit (1);
    }

  g = new_histogram(nbins*sizeof(Size_Type),"Histogram_Slice");

  g->type   = h->type;
  g->bucket = h->bucket;
  g->offset = off;
  g->nbins  = nbins;
  for (i = min; i < max; i++)
    if (i < 0 || i >= h->nbins)
      g->counts[i-min] = 0;
    else
      g->counts[i-min] = h->counts[i];

  set_histogram_mean(g);

  return (g);
}


/****************************************************************************************
 *                                                                                      *
 *  BUCKET INDICES, DOMAIN VALUES, AND PERCENTILES                                      *
 *                                                                                      *
 ****************************************************************************************/

  /*  Routines to map between bucket indices, domain values, and percentiles:

        Bucket2Value: offset + b*bucket.
        Value2Bucket: max_i s.t. Bucket2Value(i) <= v

        Bucket2Percentile: sum_(j>=i) count[j] / total
        Percentile2Bucket: max_i s.t. Bucket2Percentile(i) >= fraction

        Value2Percentile: Bucket2Percentile(j) - (v-Value2Bucket(j))*count[j]/total,
                            for j = Value2Bucket(v)
        Percentile2Value: max_v s.t. Value2Percentile(v) >= fraction

      The bucket input parameters do not need to be between 0 and h->nbins-1 and the bucket
      returned by Value2Bucket may not be either depending on the value of v.  The fraction
      parameter however must be between 0 and 1, and values and bucket numbers returned by
      the percentile routines are always in range.
  */

Value Bucket2Value(Histogram *h, int b)
{ Value v;
  switch (h->type) {
      case UVAL:
        v.uval = h->offset.uval + h->bucket.uval * b;
        return (v);
      case IVAL:
        v.ival = h->offset.ival + h->bucket.ival * b;
        return (v);
      case RVAL:
        v.rval = h->offset.rval + h->bucket.rval * b;
        return (v);
  }
}

int Value2Bucket(Histogram *h, Value v)
{ int bck;
  switch (h->type) {
      case UVAL:
        { uint64 o = h->offset.uval;
          uint64 b = h->bucket.uval;
          if (v.uval < o)
            bck = - (int) ((o - v.uval) / b);
          else
            bck = (v.uval - o) / b;
          return (bck);
        }
      case IVAL:
        { int64 o = h->offset.ival;
          int64 b = h->bucket.ival;
            bck = (v.ival - o) / b;
          return (bck);
        }
      case RVAL:
        { float64 o = h->offset.rval;
          float64 b = h->bucket.rval;
            bck = (v.rval - o) / b;
          return (bck);
        }
  }
}

double Bucket2Percentile(Histogram *h, int b)
{ Size_Type *count;
  uint64     sum;
  int        i;

  count = h->counts;
  sum   = 0;
  for (i = b; i < h->nbins; i++)
    sum += count[i];
  return ((1.*sum)/h->total);
}

int Percentile2Bucket(Histogram *h, double fraction)
{ Size_Type *count;
  uint64     cthr, sum;
  int        i;

  cthr  = h->total * fraction;
  count = h->counts;
  sum   = 0;
  if (cthr <= 0)
    return (h->nbins);
  for (i = h->nbins-1; i > 0; i--)
    { sum += count[i];
      if (sum >= cthr) break;
    }
  return (i);
}

double Value2Percentile(Histogram *h, Value v)
{ int b = Value2Bucket(h,v);
  switch (h->type) {
      case UVAL:
        return (Bucket2Percentile(h,b) - (v.uval-Bucket2Value(h,b).uval)*h->counts[b]/h->total);
      case IVAL:
        return (Bucket2Percentile(h,b) - (v.ival-Bucket2Value(h,b).ival)*h->counts[b]/h->total);
      case RVAL:
        return (Bucket2Percentile(h,b) - (v.rval-Bucket2Value(h,b).rval)*h->counts[b]/h->total);
  }
}

Value Percentile2Value(Histogram *h, double fraction)
{ Size_Type *count;
  uint64     cthr, sum;
  int        i;
  Value      v;

  cthr  = h->total * fraction;
  count = h->counts;
  sum   = 0;
  if (cthr <= 0)
    return (Bucket2Value(h,h->nbins));
  if (cthr >- h->total)
    return (Bucket2Value(h,0));
  for (i = h->nbins-1; i > 0; i--)
    { sum += count[i];
      if (sum >= cthr) break;
    }
  switch (h->type) {
      case UVAL:
        v.uval = h->offset.uval + h->bucket.uval * i +
                (uint64) (h->bucket.uval * (1.*sum-cthr)/count[i]);
        return (v);
      case IVAL:
        v.ival = h->offset.ival + h->bucket.ival * i +
                (int64) (h->bucket.ival * (1.*sum-cthr)/count[i]);
        return (v);
      case RVAL:
        v.rval = h->offset.rval + h->bucket.rval * i +
                (float64) (h->bucket.rval * (1.*sum-cthr)/count[i]);
        return (v);
  }
}


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM STATISTICS                                                                *
 *                                                                                      *
 ****************************************************************************************/

double Histogram_Mean(Histogram *h)
{ return (h->mean); }

double Histogram_Variance(Histogram *h)
{ int        i;
  double     sum, b, u;
  Size_Type *count;
  
  count = h->counts;

  switch (h->type) {
      case UVAL:
        b = h->bucket.uval;
          u = (h->offset.uval + .5*b - .5) - h->mean;
        break;
      case IVAL:
        b = h->bucket.ival;
          u = (h->offset.ival + .5*b - .5) - h->mean;
        break;
      case RVAL:
        b = h->bucket.rval;
          u = (h->offset.rval + .5*b) - h->mean;
        break;
  }

  sum = 0.;
  for (i = 0; i < h->nbins; i++)
    { sum += count[i] * u * u;
      u += b;
    }
  return (sum/h->total);
}

double Histogram_Sigma(Histogram *h)
{ return (sqrt(Histogram_Variance(h))); }

double Histogram_Central_Moment(Histogram *h, int n)
{ int        i;
  double     sum, b, u;
  Size_Type *count;
  
  count = h->counts;

  switch (h->type) {
      case UVAL:
        b = h->bucket.uval;
          u = (h->offset.uval + .5*b - .5) - h->mean;
        break;
      case IVAL:
        b = h->bucket.ival;
          u = (h->offset.ival + .5*b - .5) - h->mean;
        break;
      case RVAL:
        b = h->bucket.rval;
          u = (h->offset.rval + .5*b) - h->mean;
        break;
  }

  sum = 0.;
  for (i = 0; i < h->nbins; i++)
    { sum += count[i] * pow(u,n);
      u   += b;
    }
  return (sum/h->total);
}


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM ENTROPY                                                                   *
 *                                                                                      *
 ****************************************************************************************/

   /*  Assuming the histogram h is a discrete probaility distribution as defined by the
         choice of bucket width, Histogram_Entropy returns - sum_b p(b) log_2 p(b) where
         p(b) is counts[b]/total for each bucket b.  Cross_Entropy is - sum_b p(b) log_2 q(b)
         where q(b) is the distribution for g.  The histograms h and g must have the same
         bucket width and while their offsets can be different, the difference must be a
         multiple of the bucket width.  Relative_Entropy is sum_b p(b) log_2 p(b)/q(b);
   */

double Histogram_Entropy(Histogram *h)
{ Size_Type *count = h->counts;
  double     normal = 1./h->total;
  double     entropy;
  int        i;

  entropy = 0.;
  for (i = 0; i < h->nbins; i++)
    { double p = count[i] * normal;
      if (p > 1.e-20)
        entropy -= p*log2(p);
    }
  return (entropy);
}

double Histogram_Cross_Entropy(Histogram *h, Histogram *g)
{ Size_Type *hcount = h->counts;
  Size_Type *gcount = g->counts;
  double     hnormal = 1./h->total;
  double     gnormal = 1./g->total;
  double     entropy, delt;
  int        i, j, disp;

  switch (h->type) {
      case UVAL:
        if (h->bucket.uval != g->bucket.uval)
          { fprintf(stderr,"Histogram do not have same bucket width (Histogram_Cross_Entropy)\n");
            exit (1);
          }
        if (h->offset.uval > g->offset.uval)
          disp = h->offset.uval - g->offset.uval;
        else
          disp = - (int) (g->offset.uval - h->offset.uval);
        if (disp % h->bucket.uval != 0)
          { fprintf(stderr,"Histogram bucket offsets not in synch (Histogram_Cross_Entropy)\n");
            exit (1);
          }
        disp /= h->bucket.uval;
        break;
      case IVAL:
        if (h->bucket.ival != g->bucket.ival)
          { fprintf(stderr,"Histogram do not have same bucket width (Histogram_Cross_Entropy)\n");
            exit (1);
          }
        disp = h->offset.ival - g->offset.ival;
        if (disp % h->bucket.ival != 0)
          { fprintf(stderr,"Histogram bucket offsets not in synch (Histogram_Cross_Entropy)\n");
            exit (1);
          }
        disp /= h->bucket.ival;
        break;
      case RVAL:
        if (h->bucket.rval != g->bucket.rval)
          { fprintf(stderr,"Histogram do not have same bucket width (Histogram_Cross_Entropy)\n");
            exit (1);
          }
        delt = (h->offset.rval - g->offset.rval) / h->bucket.rval;
        disp = delt;
        if (fabs(disp - delt) > 1.e-10)
          { fprintf(stderr,"Histogram bucket offsets not in synch (Histogram_Cross_Entropy)\n");
            exit (1);
          }
        break;
    }

  entropy = 0.;
  for (i = 0, j = disp; i < h->nbins; i++, j++)
    { double p = hcount[i] * hnormal;
      double q;
      if (0 <= j && j < g->nbins)
        { q = gcount[j] * gnormal;
          if (q < 1.e-20)
            q = 1.e-20;
        }
      else
        q = 1.e-20;
      entropy -= p*log2(q);
    }

  return (entropy);
}

double Histogram_Relative_Entropy(Histogram *h, Histogram *g)
{ return (Histogram_Cross_Entropy(h,g) - Histogram_Entropy(h)); }


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM THRESHOLDS                                                                *
 *                                                                                      *
 ****************************************************************************************/

  /*  Compute the Otsu threshold value for an image based on its histogram h: this value is
        only to the resolution of the bucket size of the histogram, so a bucket index b is
        returned, the implication being that everyting >= Bucket2Value(b) is considered
        foreground.
  */

int Otsu_Threshold(Histogram *h)
{ int        i, t;
  Size_Type  c, pden, tden;
  double     psum, tsum;
  double     var, max;
  double     b, u;
  Size_Type *count;

  count = h->counts;

  switch (h->type) {
      case UVAL:
        b = h->bucket.uval;
          u = h->offset.uval + .5*b - .5;
        break;
      case IVAL:
        b = h->bucket.ival;
          u = h->offset.ival + .5*b - .5;
        break;
      case RVAL:
        b = h->bucket.rval;
          u = h->offset.rval + .5*b;
        break;
  }

  tden = h->total;
  tsum = tden * h->mean;

  pden = 0;
  psum = 0.;
  max = 0.;
  for (i = 0; i < h->nbins-1; i++)
    { c = count[i];
      pden += c;
      psum += c * u;
      tden -= c;
      u    += b;
      var = psum/pden - (tsum-psum)/tden;
      var = (pden*var)*(tden*var);
      if (var > max)
        { max = var;
          t   = i;
        }
    }

  return (t+1);
}

  /*  Similarly, slope threshold returns the inflection point (to the nearest bucket boundary)
        of the histogram relative to a line from the maximum bucket to the larget non-zero bucket.
  */

int Slope_Threshold(Histogram *h)
{ int        i, t;
  int        low, hgh;
  Size_Type  c, max;
  Size_Type *count;
  double     slope;

  count = h->counts;

  low = hgh = 0;
  max = count[0];
  for (i = 1; i < h->nbins; i++)
    { c = count[i];
      if (c > max)
        { max = count[i];
          low = i;
        }
      if (c > 0)
        hgh = i;
    }
  hgh = hgh+1;

  slope = (1.*max) / (hgh-low);
  max   = 0;
  for (i = low+1; i < hgh; i++)
    { c = (Size_Type) ((hgh - i) * slope);
      if (c > count[i])
        { c -= count[i];
          if (c > max)
            { max = c;
              low = i;
            }
        }
    }

  return (low+1);
}

int Intermeans_Threshold(Histogram *h)
{ int        i, t, n;
  int        low, hgh;
  Size_Type  sum1, size1;
  Size_Type  sumt, sizet;
  Size_Type *count;
  double     mean1, mean2;

  count = h->counts;

  sumt  = 0.;
  sizet = 0;
  low   = -1;
  for (i = 0; i < h->nbins; i++)
    { Size_Type c = count[i];
      if (c > 0)
        { if (low < 0)
            low = i;
          hgh = i;
          sizet += c;
          sumt  += c * (i-low);
        }
    }

  t = hgh/2;
  n = -1;
  while (t != n)
    { n = t;

      sum1  = 0.;
      size1 = 0;
      for (i = low; i < t; i++)
        { sum1  += count[i] * (i-low);
          size1 += count[i];
        }
      mean1 = sum1 / size1;
      mean2 = ( sumt - sum1 ) / (sizet - size1);
      t = (mean1 + mean2) / 2;
    }
  
  return (low+t);
}


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM DISPLAY                                                                   *
 *                                                                                      *
 ****************************************************************************************/

  /*  Print an ascii display of histogram h on FILE output indented by indent spaces.
      The parameter flag is the bitwise or of the defined constants below which determine
      what is displayed and how it is displayed.  If binsize is not 0 then the histogram
      will be displayed in bins of the given size, with the bin boundaries being multiples
      of binsize (the counts of spanning bins in the underlying histogram are interpolated).

        BUCKET_COUNT         0x01   //  show bucket counts
        CUMULATIVE_COUNT     0x02   //  show cumulative counts
        CUMULATIVE_PERCENT   0x04   //  show cumulative per cent of the total
        ASCENDING_HGRAM      0x08   //  display in ascending order (descending by default)
          CLIP_HGRAM_HIGH    0x10   //  do not display any 0 count buckets at the top
          CLIP_HGRAM_LOW     0x20   //  do not display any 0 count buckets at the bottom
        CLIP_HGRAM           0x30   //  do not display any 0 count buckets at either extreme
        BUCKET_MIDDLE        0x40   //  display the mid-value of a bucket as opposed to its range
  */

void Print_Histogram(Histogram *h, FILE *output, int indent, int flags, Value binsize)
{ Size_Type sum, bit, pre, *count;
  double    total;
  int       i, j, top, bot, inc;
  int       rwidth, dwidth, lwidth;
  int       bflag, cflag, pflag, mflag;
  int       exp;

  count = h->counts;

  if ((flags & CLIP_HGRAM_HIGH) != 0)
    { for (top = h->nbins-1; top >= 0; top--)
        if (count[top] != 0)
          break;
    }
  else
    top = h->nbins-1;

  if ((flags & CLIP_HGRAM_LOW) != 0)
    { for (bot = 0; bot < h->nbins; bot++)
        if (count[bot] != 0)
          break;
    }
  else
    bot = 0;

  if (top < bot)
    { fprintf(output,"%*sEmpty histogram!\n",indent,"");
      return;
    }

  bflag = ((flags & BUCKET_COUNT) != 0);
  cflag = ((flags & CUMULATIVE_COUNT) != 0);
  pflag = ((flags & CUMULATIVE_PERCENT) != 0);
  mflag = ((flags & BUCKET_MIDDLE) != 0);

  if ((flags & ASCENDING_HGRAM) == 0)
    inc = -1;
  else
    inc = 1;

  total  = h->total;
  dwidth = ceil(log10(total));
  total  = 100./total;

  switch (h->type) {

      case UVAL:
        { uint64 b  = h->bucket.uval;
          uint64 o  = h->offset.uval;
          uint64 f  = h->offset.uval;

          uint64 u  = o + bot*b;
          uint64 v  = o + (top+1)*b;

          uint64 B  = b;
          if (binsize.uval != 0)
            B = binsize.uval;

            bot = u/B;
            top = (v-1)/B+1;
          u = bot*B;
          v = top*B;

          if (v != 0)
            rwidth = ceil(log10(fabs((double) v)));
          else
            rwidth = 1;

          sum = 0;
          bit = 0;
          if (inc < 0)
            u = (top-1) * B;
          else
            u = bot * B; 
          v = u + B;
          j = 0;
          for (i = bot; i != top; i++)
            { if (mflag)
                  if (B == 1)
                    fprintf(output,"%*s%*llu:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*llu:",indent,"",rwidth,u+(B-1)/2);
              else
                  if (B == 1)
                    fprintf(output,"%*s%*llu:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*llu - %*llu:",indent,"",rwidth,u,rwidth,u+(B-1));

              while (f > u) 
                { j -= 1;
                  f -= b;
                }
              while (f + b <= u)
                { j += 1;
                  f += b;
                }

              pre = sum;
              sum -= bit;
              while (f + b <= v)
                { if (j >= 0 && j < h->nbins)
                    sum += count[j];
                  j += 1;
                  f += b;
                }
            
              if (f < v)
                if (j >= 0 && j < h->nbins)
                  bit = count[j] * ((v - f)/(1.*b));
              else
                bit = 0;
              sum += bit;

              if (bflag)
                fprintf(output,"  %*llu",dwidth,sum-pre);
              if (cflag)
                fprintf(output,"  %*llu",dwidth,sum);
              if (pflag)
                fprintf(output,"  %6.1f%%",sum*total);
              fprintf(output,"\n");
              if (inc > 0)
                { u  = v;
                  v += B;
                }
              else
                { v  = u;
                  u -= B;
                }
            }
          break;
        }

      case IVAL:
        { int64 b  = h->bucket.ival;
          int64 o  = h->offset.ival;
          int64 f  = h->offset.ival;

          int64 u  = o + bot*b;
          int64 v  = o + (top+1)*b;

          int64 B  = b;
          if (binsize.ival != 0)
            B = binsize.ival;

            if (u < 0)
              bot = (u+1)/B-1;
            else
              bot = u/B;
            if (v < 0)
              top = v/B;
            else
              top = (v-1)/B+1;
          u = bot*B;
          v = top*B;

          if (v != 0)
            rwidth = ceil(log10(fabs((double) v)));
          else
            rwidth = 1;
            if (u < 0)
              lwidth = ceil(log10((double) -u))+1;
            else
              lwidth = ceil(log10((double) u));
            if (lwidth > rwidth)
              rwidth = lwidth;

          sum = 0;
          bit = 0;
          if (inc < 0)
            u = (top-1) * B;
          else
            u = bot * B; 
          v = u + B;
          j = 0;
          for (i = bot; i != top; i++)
            { if (mflag)
                  if (B == 1)
                    fprintf(output,"%*s%*lld:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*lld:",indent,"",rwidth,u+(B-1)/2);
              else
                  if (B == 1)
                    fprintf(output,"%*s%*lld:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*lld - %*lld:",indent,"",rwidth,u,rwidth,u+(B-1));

              while (f > u) 
                { j -= 1;
                  f -= b;
                }
              while (f + b <= u)
                { j += 1;
                  f += b;
                }

              pre = sum;
              sum -= bit;
              while (f + b <= v)
                { if (j >= 0 && j < h->nbins)
                    sum += count[j];
                  j += 1;
                  f += b;
                }
            
              if (f < v)
                if (j >= 0 && j < h->nbins)
                  bit = count[j] * ((v - f)/(1.*b));
              else
                bit = 0;
              sum += bit;

              if (bflag)
                fprintf(output,"  %*llu",dwidth,sum-pre);
              if (cflag)
                fprintf(output,"  %*llu",dwidth,sum);
              if (pflag)
                fprintf(output,"  %6.1f%%",sum*total);
              fprintf(output,"\n");
              if (inc > 0)
                { u  = v;
                  v += B;
                }
              else
                { v  = u;
                  u -= B;
                }
            }
          break;
        }

      case RVAL:
        { float64 b  = h->bucket.rval;
          float64 o  = h->offset.rval;
          float64 f  = h->offset.rval;

          float64 u  = o + bot*b;
          float64 v  = o + (top+1)*b;

          float64 B  = b;
          if (binsize.rval != 0)
            B = binsize.rval;

            bot = floor(u/B);
            top = ceil(v/B);
          u = bot*B;
          v = top*B;

          if (v != 0)
            rwidth = ceil(log10(fabs((double) v)));
          else
            rwidth = 1;
            if (u < 0)
              { lwidth = ceil(log10((double) -u));
                if (lwidth > rwidth)
                  rwidth = lwidth;
              }
            lwidth = floor(log10(B));
            if (rwidth > 9 && lwidth > 4)
              { exp    = 1;
                rwidth = (rwidth-lwidth)+5;
                lwidth = rwidth-6;
                if (u < 0) rwidth += 1;
              }
            else if (lwidth < -9 && rwidth < -4)
              { exp = 1;
                rwidth = 5+(rwidth-lwidth);
                lwidth = rwidth-6;
                if (u < 0) rwidth += 1;
              }
            else
              { exp = 0;
                if (lwidth > 0)
                  lwidth = 0;
                else
                  lwidth = -lwidth;
                if (rwidth <= 0)
                  { rwidth = lwidth+2;
                    if (u < 0) rwidth += 1;
                  }
                else
                  { if (u < 0)
                      { if (ceil(log10((double) -u)) >= rwidth)
                          rwidth += 1;
                      }
                    if (lwidth > 0)
                      rwidth += lwidth + 1;
                  }
              }

          sum = 0;
          bit = 0;
          if (inc < 0)
            u = (top-1) * B;
          else
            u = bot * B; 
          v = u + B;
          j = 0;
          for (i = bot; i != top; i++)
            { if (mflag)
                  if (exp)
                    fprintf(output,"%*s%*.*e:",indent,"",rwidth,lwidth,u+B/2.);
                  else if (B == 1)
                    fprintf(output,"%*s%*.*f:",indent,"",rwidth,lwidth,u);
                  else
                    fprintf(output,"%*s%*.*f:",indent,"",rwidth,lwidth,u+B/2.);
              else
                  if (exp)
                    fprintf(output,"%*s%*.*e - %*.*e:",indent,"",rwidth,lwidth,u,rwidth,lwidth,v);
                  else
                    fprintf(output,"%*s%*.*f - %*.*f:",indent,"",rwidth,lwidth,u,rwidth,lwidth,v);

              while (f > u) 
                { j -= 1;
                  f -= b;
                }
              while (f + b <= u)
                { j += 1;
                  f += b;
                }

              pre = sum;
              sum -= bit;
              while (f + b <= v)
                { if (j >= 0 && j < h->nbins)
                    sum += count[j];
                  j += 1;
                  f += b;
                }
            
              if (f < v)
                if (j >= 0 && j < h->nbins)
                  bit = count[j] * ((v - f)/(1.*b));
              else
                bit = 0;
              sum += bit;

              if (bflag)
                fprintf(output,"  %*llu",dwidth,sum-pre);
              if (cflag)
                fprintf(output,"  %*llu",dwidth,sum);
              if (pflag)
                fprintf(output,"  %6.1f%%",sum*total);
              fprintf(output,"\n");
              if (inc > 0)
                { u  = v;
                  v += B;
                }
              else
                { v  = u;
                  u -= B;
                }
            }
          break;
        }
  }
}
