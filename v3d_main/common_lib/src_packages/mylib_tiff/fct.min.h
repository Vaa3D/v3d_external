/*****************************************************************************************\
*                                                                                         *
*  Function minimization routines for 1 and multiple dimensions                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  May 2007                                                                      *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef FUNCTION_MINIMIZATION

#define FUNCTION_MINIMIZATION

#include "parameters.h"
#include "array.h"

typedef struct
  { double lft;   //  Bracket for minimization
    double mid;   //      Assume:  lft < mid < rgt
    double rgt;   //         and:  f(mid) < f(lft), f(rgt)
  } Minimum_Bundle;

//  Given a function f, a point xin, and a step size size, attempt to find a bracket
//    about a minimum in the downhill direction from xin.  Return a pointer to a
//    statically allocated bracket which is reset with each call to Find_Min_Bracket.
//    If a bracket cannot reasonably be found, then NULL is returned.  It is
//    generally a good idea to keep the step size small as it determines the
//    jump sizes for the search, which can overshoot if it's too big.

Minimum_Bundle *Find_Min_Bracket(double xin, double step, double (*f)(double));

//  Given a minimization bracket, return a coordinate within the bracket at which a
//    local minimum value of f is acheived.  Accurate to first 8 digits (half double-precision).
//    If the derivative of f, df, is NULL, then parabolic estimation is used
//    to accelerate convergence, otherwise the secant method is employed.  In the
//    event df is used, one may take advantage of the fact that a call to df with a
//    value x, *always follows a prior call to f with the same value.  So one
//    may, via globally shared values, optimize the computation of f & df by
//    sharing the computation of any common sub-expressions.

double Minimize_Fct(Minimum_Bundle *brack, double (*f)(double), double (*df)(double));

//  Statically allocate an n-by-n matrix, set it to I * step, and return a pointer to
//    it.  This matrix can then be used as an initial set of direction vectors for Powell's
//    function minimization method.

//  Find a minimum of n-dimensional function f with Powell's method which does not require
//    the gradient.  The dimensionality n is inferred from the size of xinit.  Start the search
//    at point xinit with an initial set of n axis-oriented descent vectors of size step.  The
//    domain point at which the minimum is acheived is returned (in the vector formerly
//    occupied by xinit which is consumed by modification).

Double_Vector *Powell_Minimizer(Double_Vector *R(M(xinit)), double step, double (*f)(double *));

//  Find a domain point at which a minimum of the n-dimensional function f is acheived with
//    the conjugate gradient method of Polak and Ribier.  Start the search at point xin.

Double_Vector *Polak_Ribier_Minimizer(Double_Vector *R(M(xinit)), double (*f)(double *),
                                                                  double (*df)(double *));

#endif
