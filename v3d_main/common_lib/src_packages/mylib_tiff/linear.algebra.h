/*****************************************************************************************\
*                                                                                         *
*  Matrix inversion, determinants, and linear equations via LU-decomposition              *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  April 2007                                                                    *
*  Mod   :  June 2008 -- Added TDG's and Cubic Spline to enable snakes and curves         *
*                                                                                         *
\*****************************************************************************************/

#ifndef MATRIX_LIB

#define MATRIX_LIB

#include "parameters.h"
#include "array.h"

typedef struct
  { Double_Matrix *lu_mat;  //  LU decomposion: L is below the diagonal and U is on and above it
    int           *perm;    //  Permutation of the original rows of m due to pivoting
    int            sign;    //  Sign to apply to the determinant due to pivoting (+1 or -1)
  } LU_Factor;

LU_Factor *G(Copy_LU_Factor)(LU_Factor *f);   //  As per convention for an packable object
LU_Factor *Pack_LU_Factor(LU_Factor *R(M(f)));
LU_Factor *Inc_LU_Factor(LU_Factor *R(I(f)));
void       Free_LU_Factor(LU_Factor *F(f));
void       Kill_LU_Factor(LU_Factor *F(f));
void       Reset_LU_Factor();
int        LU_Factor_Usage();
void       LU_Factor_List(void (*handler)(LU_Factor *));
int        LU_Factor_Refcount(LU_Factor *f);
LU_Factor *G(Read_LU_Factor)(FILE *input);
void       Write_LU_Factor(LU_Factor *f, FILE *output);

//  Generate a square identity matrix of size n

Double_Matrix *G(Identity_Matrix)(Dimn_Type n);

//  Transpose a square matrix g and as a convenience return a pointer to it

Double_Matrix *Transpose_Matrix(Double_Matrix *R(M(g)));

//  m is a square double matrix where the row index moves the fastest.
//    LU_Decompose takes M and produces an LU factorization of m that
//    can then be used to rapidly solve the system for given right hand sides
//    and to compute m's determinant.  The return value is NULL if the matrix
//    is nonsingular.  If the matrix appears unstable (had to use a very nearly
//    zero pivot) then the integer pointed at by stable will be zero, and
//    non-zero otherwise.  m is subsumed and effectively destroyed by the routine.

LU_Factor *G(LU_Decompose)(Double_Matrix *S(M(m)), int *O(stable));

//  For debug purposes: print the LU factorization and pivot permutation to file

void Show_LU_Factor(FILE *file, LU_Factor *f);

//  Given rhs vector b and LU-factorization f, solve the system of equations mx = b
//    and return the result in b.
//  To invert a given the LU-decomposition, simply call LU_Solve with
//    b = [ 0^k-1 1 0^n-k] to get the k'th column of the inverse matrix.

Double_Vector *LU_Solve(Double_Vector *R(M(b)), LU_Factor *f);

//  Generate the right inverse of the matrix that gave rise to the LU factorization f.
//    That is for matrix A, return matrix A^-1 s.t. A * A^-1 = I.  If transpose is non-zero
//    then the transpose of the right inverse is returned.

Double_Matrix *G(LU_Invert)(LU_Factor *f, int transpose);

//  Given an LU-factorization f, return the value of the determinant of the
//    original matrix.

double LU_Determinant(LU_Factor *f);

//  Make an orthogonal rotation matrix that spans the same space as basis.
//    Use a "stable" version of Gram-Schmidt.  NR says should use SVD, but that's
//    a lot more code, so be cautious about stability of output matrix.

Double_Matrix *Orthogonalize_Matrix(Double_Matrix *R(M(basis)));

//  t is a 3 x n matrix where [ a=t[0], b=t[1], c=t[2] ] encodes a tri-diagonal system.
//    Triband_Decompose effectively precomputes the inverse of the tridiagonal system [ a, b, c ].
//    It does so in a private storage area it shares with Triband_Solve which solves the system
//    [ a, b, c ] x = v returning the answer in v.  Triband_Decompose does not affect the value
//    of t.  Triband_Solve can be called repeatedly with different vectors v to solve the
//    same system with different constraints over and over again.  The routines handle the
//    circular case where a[0] != 0 or c[n-1] != 0 albeit a little less efficiently.  The
//    routine returns a non-zero value iff the system is singular.

int  Triband_Decompose(Double_Matrix *t);

Double_Vector *Triband_Solve(Double_Vector *R(M(v)));

//  Given a set of n control points v[0], ... v[n-1], solve, in place, for the
//    derivative at each control point so that the spline interpolates are
//    C0,1,&2 continuous.  Set closed to non-zero if you want a closed curve
//    and 0 otherwise.  The code caches the portion of the underlying tri-diagonal
//    matrix inversion that is invariant for a given n and closed, so that in the
//    common case where it is called repeatedly with same values of n and closed,
//    the routine is particularly efficient.

Double_Vector *Cubic_Spline_Slopes(Double_Vector *R(M(v)), int closed);

//  p is a 5 x n matrix where [ a=t[0], b=t[1], c=t[2], d=t[3], e=t[4] ] encodes a penta-diagonal
//    system. Pentaband_Decompose effectively precomputes the inverse of the penta-diagonal system
//    [ a, b, c, d, e].  It does so in a private storage area it shares with Pentaband_Solve which
//    solves the system [ a, b, c, d, e ] x = v returning the answer in v.  Pentaband_Decompose
//    does not affect the value of t.  Pentaband_Solve can be called repeatedly with different
//    vectors v to solve the same system with different constraints over and over again.  The
//    routines handle the circular case albeit a little less efficiently.  The routine returns
//    a non-zero value iff the system is singular.

int  Pentaband_Decompose(Double_Matrix *p);

Double_Vector *Pentaband_Solve(Double_Vector *R(M(v)));

#endif
