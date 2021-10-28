/*****************************************************************************************\
*                                                                                         *
*  Parameter and Return Result Description Macros                                         *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  January 2007                                                                  *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _PARAMETERS
#define _PARAMETERS

//  Every parameter and function name can be surrounded by one or more one-letter annotations
//    that indicate how the parameter / return value is treated by the routine in question.  A
//    parameter that is not M or O is used strictly as input.  A function name with no annotation
//    is returning either a primitive, an existing object parameter, or a bundle.  The notations
//    used are as follows:

#define  M(x) x   //  parameter x is (M)odified by the routine, i.e. input & output
#define  O(x) x   //  parameter x is set by the routine, i.e. (O)utput only

#define  G(x) x   //  function is a (G)enerator.  If function returns a bundle then all objects of
                  //    the bundle are generated (but not the bundle!)
#define  R(x) x   //  parameter x is the (R)eturn value

#define  I(x) x   //  parameter x has its reference count (I)ncremented
#define  C(x) x   //  a reference to parameter x is (C) consumed
#define  S(x) x   //  a reference to parameter x is (S) subsumed, i.e. made a (S)ub-object
#define  F(x) x   //  parameter x is (F)reed by the routine
#define  K(x) x   //  parameter x is (K)illed by the routine

#define  X(x)     //  comment on parameters or effect of routine

//  Note that (a) at most one parameter can be annotated R, (b) a function name can only
//    be annotated with G, and (c) a function name cannot be annotated G if a
//    parameter is R.


//  Datatype declarations used everywhere

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;
typedef signed long long   int64;
typedef float              float32;
typedef double             float64;

//  To facilitate large arrays and array dimensions up to the limit of the word size
//    all appropriate variables are declared in terms of the types below so that they
//    can be changed as desired (without having to revisit the entire code base!)
//    It must be that Indx_Type == Size_Type and sizeof(Dimn_Type) <= sizoef(Indx_Type).
//    Loops are carefully written to not fail with unsigned integers (e.g.
//    "for (i = n; i-- > 0; )" versus "for(i = n-1; i >= 0; i--)" the later infinite
//    looping if i is declared unsigned!).

typedef uint32 Dimn_Type;   //  Type of an array dimension
typedef uint64 Indx_Type;   //  Type of an index into the data space of an array
typedef uint64 Size_Type;   //  Type of the size of an array

//  We introduce a Value variant structure holds a generic scalar value

typedef union
  { uint64 uval;       //    Use this variant for unsigned integers
    int64  ival;       //    Use this variant for signed integer
    double rval;       //    Use this variant for floating point numbers
  } Value;

typedef enum            //  The three possible types in a Value (see e.g. Histogram)
  { UVAL = 0,
    IVAL = 1,
    RVAL = 2,
  } Value_Type;

#define VALU(u) ((Value) (uint64)  (u))   //  Useful to set up constants, e.g. VALU(0), VALU(1)
#define VALI(i) ((Value) (int64)   (i))
#define VALR(r) ((Value) (float64) (r))

#endif
