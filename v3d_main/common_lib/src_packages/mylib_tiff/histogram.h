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

#ifndef  _HISTOGRAM

#define  _HISTOGRAM

#include "parameters.h"

typedef struct
  { Value_Type type;    //  Domain of histogram
    Value      bucket;  //  Histogram has nbins buckets where the domain covered by bucket i
    Value      offset;  //     is [offset + i*bucket, offset + (i+1)*bucket)
    int        nbins;   //  number of bins
    double     mean;    //  histogram mean
    Size_Type  total;   //  total number of counts in all the bins
    Size_Type *counts;  //  counts[i] is the # of values in bucket i
  } Histogram;

  //  Per-convention object primitives

Histogram *G(Copy_Histogram)(Histogram *h);
Histogram *Pack_Histogram(Histogram *R(M(h)));
Histogram *Inc_Histogram(Histogram *R(I(h)));
void       Free_Histogram(Histogram *F(h));
void       Kill_Histogram(Histogram *K(h));
void       Reset_Histogram();
int        Histogram_Usage();
void       Histogram_List(void (*handler)(Histogram *));
int        Histogram_Refcount(Histogram *h);
Histogram *G(Read_Histogram)(FILE *input);
void       Write_Histogram(Histogram *h, FILE *output);

  /*  Routines to map between bucket indices, domain values, and percentiles:

        Bucket2Value: offset + b*bucket.  
        Value2Bucket: max_i s.t. Bucket2Value(i) <= v

        Bucket2Percentile: sum_(j>=i) count[j] / total
        Percentile2Bucket: max_i s.t. Bucket2Percentile(i) >= fraction

        Value2Percentile: Bucket2Percentile(j) - (v-j)*count[j]/total, for j = Value2Bucket(v)
        Percentile2Value: max_v s.t. Value2Percentile(v) >= fraction

      The bucket input parameters do not need to be between 0 and h->nbins-1 and the bucket
      returned by Value2Bucket may not be either depending on the value of v.  The fraction
      parameter however must be between 0 and 1, and values and bucket numbers returned by
      the percentile routines are always in range.
  */

Value  Bucket2Value(Histogram *h, int b);
int    Value2Bucket(Histogram *h, Value v);
double Bucket2Percentile(Histogram *h, int b);
int    Percentile2Bucket(Histogram *h, double fraction);
double Value2Percentile(Histogram *h, Value v);
Value  Percentile2Value(Histogram *h, double fraction);

  /*  Generate a histogram of array a with nbins of width bucket where the smallest bucket's
      smallest boundary is offset (see data descriptor comments above).  The type of values
      given for bucket and offset should be congruent with the type of a.  When nbins or
      bucket or both are zero then the histogram buckets are set up as follows based on the
      range [min,max] of values in a:

         nbins = 0 & bucket = 0:
            Buckets are of length *1* and cover the range of values in a starting at *floor(min)*

         nbins = 0 & bucket != 0:
            Buckets are of length *bucket* and cover the range of values in a starting at *offset*

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
    
Histogram *G(Histogram_Array)(Array_Or_Slice *a, int nbins, Value bucket, Value offset);

  /*  Generate a histogram based on the histogram h consisting of the bins in the interval
        [min,max).  min and max need not be between 0 and h->nbins but it must be that min < max.
        If min < 0 or max > h->nbins then g's domain will be expanded as necessary to cover
        the implied range of [Bucket2Value(min),Bucket2Value(max)].
  */

Histogram *G(Histogram_Slice)(Histogram *h, int min, int max);

  /*  Return statistics such as the mean, standard deviation (sigma), variance and other
      central moments of the histogram (and therefore the image it was produced from)
  */

double Histogram_Mean(Histogram *h);
double Histogram_Sigma(Histogram *h);
double Histogram_Variance(Histogram *h);
double Histogram_Central_Moment(Histogram *h, int n);

   /*  Assuming the histogram h is a discrete probaility distribution as defined by the
         choice of bucket width, Histogram_Entropy returns - sum_b p(b) log_2 p(b) where
         p(b) is counts[b]/total for each bucket b.  Cross_Entropy is - sum_b p(b) log_2 q(b)
         where q(b) is the distribution for g.  The histograms h and g must have the same
         bucket width and while their offsets can be different, the difference must be a
         multiple of the bucket width.  Relative_Entropy is sum_b p(b) log_2 p(b)/q(b);
   */

double Histogram_Entropy(Histogram *h);
double Histogram_Cross_Entropy(Histogram *h, Histogram *g);
double Histogram_Relative_Entropy(Histogram *h, Histogram *g);

  /*  Compute the Otsu threshold value for an image based on its histogram h: this value is
        only to the resolution of the bucket size of the histogram, so a bucket index b is
        returned, the implication being that everyting >= Bucket2Value(b) is considered
        foreground.
      Similarly, Slope_Threshold returns the inflection point (to the nearest bucket boundary)
        of the histogram relative to a line from the maximum bucket to the larget non-zero bucket.
      Similarly, Intermeans_Threshold returns the intermeans threshold as described first
        by Ridlar and Calvard (IEEE. Trans. on Cybernetics (1978) 630-632.
  */

int  Otsu_Threshold(Histogram *h);
int  Slope_Threshold(Histogram *h);
int  Intermeans_Threshold(Histogram *h);

  /*  Print an ascii display of histogram h on FILE output indented by indent spaces.
      The parameter flag is the bitwise or of the defined constants below which determine
      what is displayed and how it is displayed.  If binsize is not 0 then the histogram
      will be displayed in bins of the given size, with the bin boundaries being multiples
      of binsize (the counts of spanning bins in the underlying histogram are interpolated).
  */

#define BUCKET_COUNT         0x01   //  show bucket counts
#define CUMULATIVE_COUNT     0x02   //  show cumulative counts
#define CUMULATIVE_PERCENT   0x04   //  show cumulative per cent of the total
#define ASCENDING_HGRAM      0x08   //  display in ascending order (descending by default)
#define   CLIP_HGRAM_HIGH    0x10   //  do not display any 0 count buckets at the top
#define   CLIP_HGRAM_LOW     0x20   //  do not display any 0 count buckets at the bottom
#define CLIP_HGRAM           0x30   //  do not display any 0 count buckets at either extreme
#define BUCKET_MIDDLE        0x40   //  display the mid-value of a bucket as opposed to its range

void   Print_Histogram(Histogram *h, FILE *output, int indent, int flags, Value binsize);

#endif
