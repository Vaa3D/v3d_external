#ifndef SimTK_SimTKCOMMON_TEMPLATIZED_LAPACK_H_
#define SimTK_SimTKCOMMON_TEMPLATIZED_LAPACK_H_

/* -------------------------------------------------------------------------- *
 *                      SimTK Core: SimTKcommon                               *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK Core biosimulation toolkit originating from      *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2006-9 Stanford University and the Authors.         *
 * Authors: Michael Sherman                                                   *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

/**@file
 * These is a templatized, C++ callable interface to LAPACK and BLAS.
 * Each method must be explicitly specialized for the supported precisions.
 */


#include "SimTKcommon/internal/common.h"
#include "SimTKlapack.h"

#include <complex>
using std::complex;

namespace SimTK {

class Lapack {
public:
    // MEANINGLESS IF NOT SPECIALIZED

        template <class P> static void
    gemm
   (char transa, char transb,
    int m, int n, int k,
    const P& alpha, const P a[], int lda,
    const P b[], int ldb,
    const P& beta, P c[], int ldc) {assert(false);}

        template <class P> static void
    getri
   (int          n,
    P            a[],
    int          lda,
    const int    ipiv[], 
    P            work[], 
    int          lwork, 
    int         &info ) {assert(false);}

        template <class P> static void
    getrf
   (int          m,
    int          n, 
    P            a[],
    int          lda, 
    int          ipiv[], 
    int         &info ) {assert(false);}

};

    // xGEMM //

template <> inline void Lapack::gemm<float>
   (char transa, char transb,
    int m, int n, int k,
    const float& alpha, const float a[], int lda,
    const float b[], int ldb,
    const float& beta, float c[], int ldc)
{
    sgemm_(
        transa, transb,
        m,n,k,alpha,a,lda,b,ldb,beta,c,ldc
    );
}
template <> inline void Lapack::gemm<double>
   (char transa, char transb,
    int m, int n, int k,
    const double& alpha, const double a[], int lda,
    const double b[], int ldb,
    const double& beta, double c[], int ldc)
{
    dgemm_(
        transa, transb,
        m,n,k,alpha,a,lda,b,ldb,beta,c,ldc
    );
}
template <> inline void Lapack::gemm< complex<float> >
   (char transa, char transb,
    int m, int n, int k,
    const complex<float>& alpha, const complex<float> a[], int lda,
    const complex<float> b[], int ldb,
    const complex<float>& beta, complex<float> c[], int ldc)
{
    cgemm_(
        transa, transb,
        m,n,k,alpha,a,lda,b,ldb,beta,c,ldc
    );
}
template <> inline void Lapack::gemm< complex<double> >
   (char transa, char transb,
    int m, int n, int k,
    const complex<double>& alpha, const complex<double> a[], int lda,
    const complex<double> b[], int ldb,
    const complex<double>& beta, complex<double> c[], int ldc)
{
    zgemm_(
        transa, transb,
        m,n,k,alpha,a,lda,b,ldb,beta,c,ldc
    );
}

    // xGETRI //

template <> inline void Lapack::getri<float>
   (int          n,
    float        a[],
    int          lda,
    const int    ipiv[], 
    float        work[], 
    int          lwork, 
    int&         info )
{
    sgetri_(n,a,lda,ipiv,work,lwork,info);
}

template <> inline void Lapack::getri<double>
   (int          n,
    double       a[],
    int          lda,
    const int    ipiv[], 
    double       work[], 
    int          lwork, 
    int&         info )
{
    dgetri_(n,a,lda,ipiv,work,lwork,info);
}

template <> inline void Lapack::getri< complex<float> >
   (int             n,
    complex<float>  a[],
    int             lda,
    const int       ipiv[], 
    complex<float>  work[], 
    int             lwork, 
    int&            info )
{
    cgetri_(n,a,lda,ipiv,work,lwork,info);
}

template <> inline void Lapack::getri< complex<double> >
   (int             n,
    complex<double> a[],
    int             lda,
    const int       ipiv[], 
    complex<double> work[], 
    int             lwork, 
    int&            info )
{
    zgetri_(n,a,lda,ipiv,work,lwork,info);
}
    // xGETRF //

template <> inline void Lapack::getrf<float>
   (int          m,
    int          n, 
    float        a[],
    int          lda, 
    int          ipiv[], 
    int&         info )
{
    sgetrf_(m,n,a,lda,ipiv,info);
}

template <> inline void Lapack::getrf<double>
   (int          m,
    int          n, 
    double       a[],
    int          lda, 
    int          ipiv[], 
    int&         info )
{
    dgetrf_(m,n,a,lda,ipiv,info);
}

template <> inline void Lapack::getrf< complex<float> >
   (int             m,
    int             n, 
    complex<float>  a[],
    int             lda, 
    int             ipiv[], 
    int&            info )
{
    cgetrf_(m,n,a,lda,ipiv,info);
}

template <> inline void Lapack::getrf< complex<double> >
   (int             m,
    int             n, 
    complex<double> a[],
    int             lda, 
    int             ipiv[], 
    int&            info )
{
    zgetrf_(m,n,a,lda,ipiv,info);
}


/*
void SimTK_STDCALL
SimTK_LAPACK(dgeev,DGEEV)
   (const char  &jobvl SimTK_LAPACK_STRLEN_FOLLOWS_DECL, 
    const char  &jobvr SimTK_LAPACK_STRLEN_FOLLOWS_DECL, 
    const int   &n,
    double       a[],
    const int   &lda, 
    double       wr[],
    double       wi[],
    double       vl[],
    const int   &ldvl,
    double       vr[],
    const int   &ldvr,
    double       work[],
    const int   &lwork,
    int         &info  
    SimTK_LAPACK_STRLEN_ATEND_DECL
    SimTK_LAPACK_STRLEN_ATEND_DECL);

void SimTK_STDCALL
SimTK_LAPACK(dsyev,DSYEV)
   (const char  &jobz SimTK_LAPACK_STRLEN_FOLLOWS_DECL, 
    const char  &uplo SimTK_LAPACK_STRLEN_FOLLOWS_DECL, 
    const int   &n,
    double       a[],
    const int   &lda, 
    double       w[],
    double       work[],
    const int   &lwork,
    int         &info  
    SimTK_LAPACK_STRLEN_ATEND_DECL
    SimTK_LAPACK_STRLEN_ATEND_DECL);

void SimTK_STDCALL
SimTK_LAPACK(dspev,DSPEV)
   (const char  &jobz SimTK_LAPACK_STRLEN_FOLLOWS_DECL, 
    const char  &uplo SimTK_LAPACK_STRLEN_FOLLOWS_DECL, 
    const int   &n,
    double       a[],
    double       w[],
    double       z[],
    const int   &ldz,
    double       work[],
    int         &info  
    SimTK_LAPACK_STRLEN_ATEND_DECL
    SimTK_LAPACK_STRLEN_ATEND_DECL);

void SimTK_STDCALL
SimTK_LAPACK(dsptri,DSPTRI)
   (const char  &uplo SimTK_LAPACK_STRLEN_FOLLOWS_DECL,
    const int   &size,
    double       a[],
    int          ipiv[],
    double       work[], 
    int         &info  
    SimTK_LAPACK_STRLEN_ATEND_DECL);

void SimTK_STDCALL
SimTK_LAPACK(dsptrf,DSPTRF)
   (const char  &uplo SimTK_LAPACK_STRLEN_FOLLOWS_DECL,
    const int   &size,
    double       a[],
    int          ipiv[], 
    int         &info  
    SimTK_LAPACK_STRLEN_ATEND_DECL);

void SimTK_STDCALL
SimTK_LAPACK(dsyevx,DSYEVX)
   (const char      &jobz,
    const char      &range,
    const char      &uplo,
    const int       &n,
    double           a[],
    const int       &lda,
    const double    &vl,
    const double    &vu,
    const int       &il,
    const int       &iu,
    const double    &abstol,
    int             &m,
    double           w[],
    double           z[],
    const int       &ldz,
    double           work[],
    const int       &lwork,
    int              iwork[],
    int              ifail[],
    int             &info);

void SimTK_STDCALL
SimTK_LAPACK(dgelss,DGELSS)
   (int             &m,
    const int       &n,
    const int       &nrhs,
    double           a[],
    const int       &lda,
    double           b[],
    const int       &ldb,
    double           s[],
    const double    &rcond,
    int             &rank,
    double           work[],
    const int       &lwork,
    int             &info );

void SimTK_STDCALL
SimTK_LAPACK(dgesv,DGESV)
   (int         &n,
    int         &nrhs,
    double       a[],
    int         &lda,
    int          ipiv[],
    double       b[],
    int         &ldb,
    int         &info);

void SimTK_STDCALL
SimTK_LAPACK(dgesvd,DGESVD)
   (const char  &jobu  SimTK_LAPACK_STRLEN_FOLLOWS_DECL, 
    const char  &jobvt SimTK_LAPACK_STRLEN_FOLLOWS_DECL,
    const int   &m, 
    const int   &n, 
    double       a[],
    const int   &lda,
    double       s[],
    double       u[],
    const int   &ldu, 
    double       vt[], 
    const int   &ldvt, 
    double       work[],
    const int   &lwork, 
    int         &info
    SimTK_LAPACK_STRLEN_ATEND_DECL
    SimTK_LAPACK_STRLEN_ATEND_DECL);

*/

}   // namespace SimTK

#endif // SimTK_SimTKCOMMON_TEMPLATIZED_LAPACK_H_
