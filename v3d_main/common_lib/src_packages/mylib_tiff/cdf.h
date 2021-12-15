/*****************************************************************************************\
*                                                                                         *
*  Distribution generator data abstraction                                                *
*     One can create a distribution generator for a number of parameterized distribution  *
*     types and then generate events with that distribution                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  January 2007                                                                  *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _SR_CDF
#define _SR_CDF

#include "parameters.h"

typedef void CDF;

/* Calling these returns a CDF that will generate events with the given distribution */

CDF *G(Normal_CDF)(double mean, double stdev);
CDF *G(Exponential_CDF)(double a);
CDF *G(Poisson_CDF)(double a);
CDF *G(Geometric_CDF)(double p);
CDF *G(Uniform_CDF)(double low, double hgh);
CDF *G(Binomial_CDF)(int n, double p);
CDF *G(FairCoin_CDF)(int n);
CDF *G(Bernouilli_CDF)(int n, double *weight);

CDF *G(Copy_CDF)(CDF *cdf);   /* As per convention */
CDF *Pack_CDF(CDF *R(M(cdf)));
CDF *Inc_CDF(CDF *R(I(cdf)));
void Free_CDF(CDF *F(cdf));
void Kill_CDF(CDF *K(cdf));
void Reset_CDF();
int  CDF_Usage();
void CDF_List(void (*handler)(CDF *));
int  CDF_Refcount(CDF *cdf);
CDF *G(Read_CDF)(FILE *input);
void Write_CDR(CDF *cdf, FILE *output);

/* Each call to Sample_CDF returns an event distributed according to the cdf supplied:
     Normal returns a real in [-6*stdev, 6*stdev] (tails beyond 6 stds are truncated).
     Exponential returns a non-negative real number.
     Geometric and Poisson return a non-negative integer.
     Uniform returns a real in [low,hgh).
     Binomial, FairCoin, and Bernouilli return an integer in [1,n].
*/

double Sample_CDF(CDF *cdf);

/* IMPLEMENTATION NOTES:
     The Binomial, Poisson, and Bernouilli distributions need explicit piecewise cdf's
     generated in tables that are allocated when the generator is requested, so having
     a lot of these active at any time can get expensive in space.  The approximate size
     of the tables (in bytes) is as follows:
        Poisson:    ~ 960 * 3 ^ log_10(a)
        Binomial:   < 400 * 3.2 ^ log_10(n)
        Bernouilli: = 8 * n bytes
     The space for these tables is only released if you Kill the CDF or Reset the abstracdtion.
*/

#endif
