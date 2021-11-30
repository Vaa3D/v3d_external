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

#LISTDEF @TYPES  =  UINT8 UINT16 UINT32 UINT64  INT8 INT16 INT32 INT64 FLOAT32 FLOAT64
#LISTDEF @UNION  =   uval   uval   uval   uval  ival  ival  ival  ival    rval    rval


/****************************************************************************************
 *                                                                                      *
 *  HISTOGRAM SPACE MANAGEMENT ROUTINES AND PRIMARY GENERATOR                           *
 *                                                                                      *
 ****************************************************************************************/

// Awk-generated (manager.awk) Array memory management


static Value_Type a2v_type[] = { UVAL, UVAL, UVAL, UVAL, IVAL, IVAL, IVAL, IVAL, RVAL, RVAL };

static inline int histogram_nsize(Histogram *h)
{ return (sizeof(Size_Type)*h->nbins); }

MANAGER -IO Histogram counts:nsize

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
    #GENERATE t = uval ival rval
      case <T>:
        b = h->bucket.<t>;
        #IF t == rval
          u = h->offset.<t> + .5*b;
        #ELSE
          u = h->offset.<t> + .5*b - .5;
        #END
        break;
      #END
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
    #GENERATE t,u,f = uval ival rval , uint64 int64 float64 , llu lld g
      case <T>:
        if (nbins == 0)
          { uint64 rti;
            if (bucket.<t> == 0)
              bucket.<t> = 1;
            #IF t == rval
              offset.<t> = floor(rng->minval.<t>);
            #ELSE
              offset = rng->minval;
            #END
            rti = (rng->maxval.<t> - offset.<t>) / bucket.<t> + 1;
            if (rti > 0x7FFFFFFF)
              { fprintf(stderr,
                        "Implied binning requires more than 2 billion bins (Histogram_Array)\n");
                exit (1);
              }
            nbins = rti;
          }
        else if (bucket.<t> == 0)
          { double bwide = (rng->maxval.<t> - rng->minval.<t>) / (1.*nbins);
            if (bwide == 0.)
              bucket.<t> = 1;
            else
              #IF t == rval
              { double x = pow(10.,floor(log10(bwide)));
              #ELSE
              { <u> x = 1;
                while (10*x <= bwide)
                  x = 10*x;
              #END
                if (x < bwide)
                  if (2*x < bwide)
                    if (5*x < bwide)
                      x = 10*x;
                    else
                      x = 5*x;
                  else
                    x = 2*x;
                bucket.<t> = x;
              }
            offset.<t> = floor(rng->minval.<t> / bucket.<t>) * bucket.<t>;
            nbins = (rng->maxval.<t> - offset.<t>) / bucket.<t>;
            if (offset.<t> + nbins*bucket.<t> <= rng->maxval.<t>)
              nbins += 1;
          } 
        clip = (offset.<t> > rng->minval.<t> || offset.<t> + bucket.<t> * nbins <= rng->maxval.<t>);
        break;
    #END
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
    #GENERATE T,U = @TYPES, @UNION
      case <T>:
        { <t> *v = A<T>(a);
          <t>  o = offset.<U>;
          <t>  b = bucket.<U>;
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
            #IF T >= INT8
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
            #END
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
    #END
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
    #GENERATE t = uval ival rval
      case <T>:
        off.<t> = h->offset.<t> + h->bucket.<t> * min;
        break;
    #END
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
    #GENERATE t = uval ival rval
      case <T>:
        v.<t> = h->offset.<t> + h->bucket.<t> * b;
        return (v);
    #END
  }
}

int Value2Bucket(Histogram *h, Value v)
{ int bck;
  switch (h->type) {
    #GENERATE t,u = uval ival rval, uint64 int64 float64
      case <T>:
        { <u> o = h->offset.<t>;
          <u> b = h->bucket.<t>;
        #IF t == uval
          if (v.<t> < o)
            bck = - (int) ((o - v.<t>) / b);
          else
        #END
            bck = (v.<t> - o) / b;
          return (bck);
        }
    #END
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
    #GENERATE t,u = uval ival rval, uint64 int64 float64
      case <T>:
        return (Bucket2Percentile(h,b) - (v.<t>-Bucket2Value(h,b).<t>)*h->counts[b]/h->total);
    #END
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
    #GENERATE t,u = uval ival rval, uint64 int64 float64
      case <T>:
        v.<t> = h->offset.<t> + h->bucket.<t> * i +
                (<u>) (h->bucket.<t> * (1.*sum-cthr)/count[i]);
        return (v);
    #END
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
    #GENERATE t = uval ival rval
      case <T>:
        b = h->bucket.<t>;
        #IF t == rval
          u = (h->offset.<t> + .5*b) - h->mean;
        #ELSE
          u = (h->offset.<t> + .5*b - .5) - h->mean;
        #END
        break;
    #END
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
    #GENERATE t = uval ival rval
      case <T>:
        b = h->bucket.<t>;
        #IF t == rval
          u = (h->offset.<t> + .5*b) - h->mean;
        #ELSE
          u = (h->offset.<t> + .5*b - .5) - h->mean;
        #END
        break;
    #END
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
    #GENERATE t = uval ival rval
      case <T>:
        if (h->bucket.<t> != g->bucket.<t>)
          { fprintf(stderr,"Histogram do not have same bucket width (Histogram_Cross_Entropy)\n");
            exit (1);
          }
      #IF t != rval
      #IF t == uval
        if (h->offset.<t> > g->offset.<t>)
          disp = h->offset.<t> - g->offset.<t>;
        else
          disp = - (int) (g->offset.<t> - h->offset.<t>);
      #ELSE
        disp = h->offset.ival - g->offset.ival;
      #END
        if (disp % h->bucket.<t> != 0)
          { fprintf(stderr,"Histogram bucket offsets not in synch (Histogram_Cross_Entropy)\n");
            exit (1);
          }
        disp /= h->bucket.<t>;
      #ELSE
        delt = (h->offset.<t> - g->offset.<t>) / h->bucket.<t>;
        disp = delt;
        if (fabs(disp - delt) > 1.e-10)
          { fprintf(stderr,"Histogram bucket offsets not in synch (Histogram_Cross_Entropy)\n");
            exit (1);
          }
      #END
        break;
    #END
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
    #GENERATE t = uval ival rval
      case <T>:
        b = h->bucket.<t>;
        #IF t == rval
          u = h->offset.<t> + .5*b;
        #ELSE
          u = h->offset.<t> + .5*b - .5;
        #END
        break;
    #END
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
    #GENERATE t,u,f = uval ival rval , uint64 int64 float64 , llu lld g

      case <T>:
        { <u> b  = h->bucket.<t>;
          <u> o  = h->offset.<t>;
          <u> f  = h->offset.<t>;

          <u> u  = o + bot*b;
          <u> v  = o + (top+1)*b;

          <u> B  = b;
          if (binsize.<t> != 0)
            B = binsize.<t>;

          #IF t == rval
            bot = floor(u/B);
            top = ceil(v/B);
          #ELSEIF t == ival
            if (u < 0)
              bot = (u+1)/B-1;
            else
              bot = u/B;
            if (v < 0)
              top = v/B;
            else
              top = (v-1)/B+1;
          #ELSE
            bot = u/B;
            top = (v-1)/B+1;
          #END
          u = bot*B;
          v = top*B;

          if (v != 0)
            rwidth = ceil(log10(fabs((double) v)));
          else
            rwidth = 1;
          #IF t == ival
            if (u < 0)
              lwidth = ceil(log10((double) -u))+1;
            else
              lwidth = ceil(log10((double) u));
            if (lwidth > rwidth)
              rwidth = lwidth;
          #ELSEIF t == rval
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
          #END

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
                #IF t == rval
                  if (exp)
                    fprintf(output,"%*s%*.*e:",indent,"",rwidth,lwidth,u+B/2.);
                  else if (B == 1)
                    fprintf(output,"%*s%*.*f:",indent,"",rwidth,lwidth,u);
                  else
                    fprintf(output,"%*s%*.*f:",indent,"",rwidth,lwidth,u+B/2.);
                #ELSE
                  if (B == 1)
                    fprintf(output,"%*s%*<f>:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*<f>:",indent,"",rwidth,u+(B-1)/2);
                #END
              else
                #IF t == rval
                  if (exp)
                    fprintf(output,"%*s%*.*e - %*.*e:",indent,"",rwidth,lwidth,u,rwidth,lwidth,v);
                  else
                    fprintf(output,"%*s%*.*f - %*.*f:",indent,"",rwidth,lwidth,u,rwidth,lwidth,v);
                #ELSE
                  if (B == 1)
                    fprintf(output,"%*s%*<f>:",indent,"",rwidth,u);
                  else
                    fprintf(output,"%*s%*<f> - %*<f>:",indent,"",rwidth,u,rwidth,u+(B-1));
                #END

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
    #END
  }
}
