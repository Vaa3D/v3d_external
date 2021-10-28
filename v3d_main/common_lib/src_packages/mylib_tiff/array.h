/*****************************************************************************************\
*                                                                                         *
*  Array Abstraction                                                                      *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  October 2009                                                                  *
*                                                                                         *
*  (c) July 27, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _ARRAY_LIB

#define _ARRAY_LIB

#include "parameters.h"

/* ARRAY ABSTRACTION  */

typedef enum			//  There are eight types allowed for arrays
  { UINT8   = 0,
    UINT16  = 1,
    UINT32  = 2,
    UINT64  = 3,
    INT8    = 4,
    INT16   = 5,
    INT32   = 6,
    INT64   = 7,
    FLOAT32 = 8,
    FLOAT64 = 9
  } Array_Type;

typedef enum		//  Some arrays may encode RGB or RGBA data (see below):
  { PLAIN_KIND    = 0,	//    The outermost dimension encodes each channel (R, G, B, or A)
    RGB_KIND      = 1,	//  Others may encode COMPLEX data (see below):
    RGBA_KIND     = 2,  //    The innermost dimension encodes the real and imaginary parts.
    COMPLEX_KIND  = 3
  } Array_Kind;

typedef enum		//  Interpretation of the outermost dimension index if RGB or RGBA
  { RED_INDEX   = 0,
    GREEN_INDEX = 1,
    BLUE_INDEX  = 2,
    ALPHA_INDEX = 3
  } Color_Index;

typedef enum		//  Interpretation of the innermost dimension index if COMPLEX
  { REAL_INDEX = 0,
    IMAG_INDEX = 1,
  } Complex_Index;

typedef enum		//  Operations on array elements
  { SET_OP = 0,		//    See Array_Op_Scalar and Array_Op_Array below
    ADD_OP = 1,
    SUB_OP = 2,
    MUL_OP = 3,
    DIV_OP = 4,
    POW_OP = 5,
    LSH_OP = 6,
    RSH_OP = 7,
    MIN_OP = 8,
    MAX_OP = 9,
  } Operator;

typedef enum		//  Comparator ops for defining regions: see Draw_Floodfill in draw.h &
  { LE = 0,		//     Record_Region in region.h
    LT = 1,
    EQ = 2,
    NE = 3,
    GT = 4,
    GE = 5
  } Comparator;

  /* An array can have any number of dimensions and be of the 8 types listed above.  Moreover,
        a given type can further be restricted to a given number of bits for the non-floating
        point types (e.g. 12-bit data stored in UINT16's, motivated by images and other a/d
        outputs captured as digital series or arrays).  An array also has an associated text tag.

     Most routines assume and operate on an array as an n-dimensional array of one of the eight
        scalar types.  But the 'kind' field can signal that an array actually represents a
        color image or a complex array, in which case, some routines will behave differently
        according to what 'kind' designates.  A PLAIN array is just a plain vanilla array.
        An RGB array or RGBA array represents a color image where the outermost dimension of
        the array is 3 or 4, respectively, one index for each of red, green, blue, and alpha.
        A COMPLEX array has innermost dimension 2 where the first inner dimension is the real
        part and the second inner dimension is the imaginary part of an imaginary number.
        While RGB and RGBA arrays are typically of type UINT8 or UINT16, we do not so restrict
        them nor do our tiff reader/writer routines.  For the alpha channel the convention is
        that alpha is between 0. and 1. for floating point types, and for integer types, no
        transparency or 100% opacity is denoted by the largest possible value encodable with
        the number of bits given by the type & scale (e.g. 255 = 2^8 for UINT8 & 8, and
        32677 = 2^15-1 for INT16 & 16).

      For most arrays, the scale is typically the number of bits in the underlying element
        type (e.g. 32 if FLOAT32, INT32, or UINT32), but one may set it to a smaller number
        in the case of the non-floating point types to restrict the values if desired.  The
        routines for which this can make a difference are Array_Convert and the Tiff reader &
        writer routines.  Similarly, the text is generally the empty string for ordinary arrays,
        but one can label arrays if they find it useful.  Moreover, this text is read/written
        from and to the JF_TAGGER tag of a tiff.  Finally, note that complex arrays can be
        over any type although typically only floating point is used.
  */

typedef struct
  { Array_Kind  kind;    //  Interpreation of the array: one of the four enum constants above
    Array_Type  type;    //  Type of values, one of the eight enum constants above
    int         scale;   //  # of bits in integer values

    int         ndims;   //  Number of dimensions of the array
    Size_Type   size;    //  Total number of elements in the array (= PROD_i dims[i])
    Dimn_Type  *dims;    //  dims[i] = length of dimension i

    int         tlen;    //  Length of the text string text (including '\0')
    char       *text;    //  An arbitrary string label, '\0'-terminated as per C convention

    void       *data;    //  A block of size sizeof(type) * size bytes holding the array.
                         //    dims[0] moves fastest, i.e. the index of A[xn-1,...,x1,x0] is
                         //    (... (xn-1*dims[n-2] + xn-2)*dims[n-3] + ...)*dims[0] + x0
  } Array;

typedef Array  Matrix;  		//  Descriptive declarations for 2D and 1D arrays
typedef Array  Vector;
typedef Vector Coordinate;	        //  A coordinate into an array's integer lattice
                                        //    The type is Dimn_Type

typedef Array  Pixel_Array; 	  	//  Designates UINT8 or UINT16
typedef Matrix Pixel_Matrix;
typedef Vector Pixel_Vector;

typedef Array  Integer_Array;    	//  Designates INT32
typedef Matrix Integer_Matrix;
typedef Vector Integer_Vector;

typedef Array  Float_Array;    		//  Designates FLOAT32
typedef Matrix Float_Matrix;
typedef Vector Float_Vector;

typedef Array  Double_Array;   		//  Designates FLOAT64
typedef Matrix Double_Matrix;
typedef Vector Double_Vector;

typedef Array  Numeric_Array;  		//  Designates FLOAT32 or FLOAT64
typedef Matrix Numeric_Matrix;
typedef Vector Numeric_Vector;

typedef Array  Complex_Array;   	//  Designates FLOAT32 or FLOAT64,
typedef Matrix Complex_Matrix;          //     PLAIN or COMPLEX array
typedef Vector Complex_Vector;

typedef void   Array_Or_Slice;          //  An Array or a Slice (see below)


Array *G(Copy_Array)(Array *array);	//  Per-convention object primitives
Array *Pack_Array(Array *R(M(array)));
Array *Inc_Array(Array *R(I(array)));
void   Free_Array(Array *F(array));
void   Kill_Array(Array *K(array));
void   Reset_Array();
int    Array_Usage();
void   Array_List(void (*handler)(Array *));
int    Array_Refcount(Array *array);
Array *G(Read_Array)(FILE *input);
void   Write_Array(Array *array, FILE *output);

#define AUINT8(a)    ((uint8    *) (a)->data)  //  Coerce array data to its type
#define AUINT16(a)   ((uint16   *) (a)->data)
#define AUINT32(a)   ((uint32   *) (a)->data)
#define AUINT64(a)   ((uint64   *) (a)->data)
#define AINT8(a)     ((int8     *) (a)->data)
#define AINT16(a)    ((int16    *) (a)->data)
#define AINT32(a)    ((int32    *) (a)->data)
#define AINT64(a)    ((int64    *) (a)->data)
#define AFLOAT32(a)  ((float32  *) (a)->data)
#define AFLOAT64(a)  ((float64  *) (a)->data)
#define ADIMN(a)     ((Dimn_Type *) (a)->data)

int Same_Shape(Array *a, Array *b);     // Do a and b have the same shape?
int Same_Type(Array *a, Array *b);      // Do a and b have the same shape and type?

  /* There are four ways to generate a new array:
       Make_Array creates an array of the given kind, type, and shape.  If the kind is other
         than PLAIN, then the shape supplied by ndims & dims does not include the extra
         dimension for the kind which is automatically added.
       Make_Array_With_Shape assumes the integer vector shape gives the dimensionality and
         size of each dimension.  See "Coord" later on for a way to easily make a shape
         (e.g. you could write Make_Array_With_Shape(..,..,Coord("4,2,3"))).  The shape
         array is freed before returning.  The extra dimension for the kind (if any) is
         automatically added.
       Make_Array_Of_Data makes an array whose data vector is the one supplied as the first
         argument.  This vector must be of size at least size * sizeof(type) bytes and the
         memory management of this vector is taken over by the array class.
       Make_Array_From_Arrays assumes that it is given n pointers to arrays that are all
         of the same type, shape, and kind.  It makes a new array that has an outer dimension
         of n and whose value is the concatenation of the n arrays.  The supplied arrays
         continue to exist as independent objects.
  */

Array *G(Make_Array)(Array_Kind kind, Array_Type type, int ndims, Dimn_Type *dims);

Array *G(Make_Array_With_Shape)(Array_Kind kind, Array_Type type, Coordinate *F(shape));

Array *G(Make_Array_Of_Data)(Array_Kind kind, Array_Type type, int ndims, Dimn_Type *dims,
                             void *data);

Array *G(Make_Array_From_Arrays)(Array_Kind kind, int n, Array **arrays);

  /* One can set and append to the text string of an array.  To get it simply refer to a->text! */

void Set_Array_Text(Array *M(array), char *text);
void Append_To_Array_Text(Array *M(array), char *text);

  /* A coordinate is modeled by a Coordinate array that is a 1-dimensional array of type
       corresponding to Dimn_Type and of length n where the i'th element gives the i'th
       coordinate.  The routine Coord generates a coordinate from a comma-separated list
       of positive integer constants, e.g. Coord("3,4,2"), and Coord_String takes a
       coordinate a returns a bundle to a string that represents the coorindate, e.g.
       Coord_String(Coord("3,4,2")) = "3,4,2".  Coordn for n = 1, 2, 3, or 4 generates
       an n-dimensional coordinate from the integers supplied as parameters, e.g.
       Coord3(x+7,y+2,3).  Array_Basis(a) produces a coordinate whose values are the
       dimensions of the array a, and finally, AddCoord allows one to add a
       higher-dimension to an existing coordinate.

     All routines in the library, save for the Get/Set_Array_Value routines and a few drawing
       routines in draw.[ph] use indices as inputs and outputs.  However, it is often convenient
       for users to think in terms of coordinates.  We provide the routines Idx2CoordA and
       Coord2IdxA to convert between coordinates and array indices.  Note carefully that the
       conversion requires knowing the dimensions of the array which we term the underlying basis
       of the coordinate system and this is supplied as the first argument to these two routines.
       As an alternative to always supplying the array a as an argument, we maintain a current
       basis attribute which can be set with Set_Coord_Basis or Use_Array_Basis and retrieved
       with Get_Coord_Basis.  The conversions of Idx2Coord and Coord2Idx take place with respect
       to the current basis.
  */

Coordinate *G(Coord)(char *list);
Coordinate *G(Coord1)(Dimn_Type d1);
Coordinate *G(Coord2)(Dimn_Type d2, Dimn_Type d1);
Coordinate *G(Coord3)(Dimn_Type d3, Dimn_Type d2, Dimn_Type d1);
Coordinate *G(Coord4)(Dimn_Type d4, Dimn_Type d3, Dimn_Type d2, Dimn_Type d1);
Coordinate *G(Array_Basis)(Array *array);

Coordinate *AddCoord(int d, Coordinate *R(M(coord)));
char       *Coord_String(Coordinate *F(coord));

Coordinate *G(Idx2CoordA)(Array *a, Indx_Type idx);
Indx_Type   Coord2IdxA(Array *a, Coordinate *F(coord));

void        Set_Coord_Basis(Coordinate *F(coord));
Coordinate *G(Get_Coord_Basis)();
void        Use_Array_Basis(Array *array);

Coordinate *G(Idx2Coord)(Indx_Type idx);
Indx_Type   Coord2Idx(Coordinate *F(coord));

  /* As an example of the use of coordinates, we provide routines that get and set the value
       of an array at a particular coordinate.  We expect these to be used very rarely as it
       is not particularly efficient to call a routine to get and set an element of an array.
  */

Value       Get_Array_Value(Array *array, Coordinate *F(coord));
void        Set_Array_Value(Array *M(array), Coordinate *F(coord), Value v);

  /* Routines such as Region_COM in the region.p module return a real-valued point within the
       coordinate space of an array.  Often one just wishes to know this position to the nearest
       integer coordinate position (within an array).  Floor_Coord produces an integer vector
       by flooring each value to an integer, Ceiling_Coord by ceiling each value, and Nearest_Coord
       by rounding to the nearest integer.  The resulting Integer_Arrays may be used as coordinates
       provided no index is negative.
  */

Integer_Array *G(Floor_Coord)(Double_Vector *point);
Integer_Array *G(Ceiling_Coord)(Double_Vector *point);
Integer_Array *G(Nearest_Coord)(Double_Vector *point);

  /* Because an array is layed out in row major order, a slice of an array (i.e. a sub-array
       within the rectangular grid delimited by the minimum and maximum corners given by
       a pair of coordinates, beg and end), is actually not trivial to process as a unit as
       its elements consist of a collection of non-consecutive segments within the linear
       vector holding the elements of the array.  To this end we introduce a Slice object
       that models a slice of an array.  We further make routines that make valid sense on
       Slice as well as an Array work on both by introducing the Array_Or_Slice type.  Routines
       that take such an argument equally apply to Slices as to Arrays (e.g. Print, Scale, etc.).
       The type and scale of a slice are the same as its underlying array, but the kind of
       a slice is PLAIN if it takes a proper subset of the outer dimension of RGB and RGBA
       arrays or the innermost dimension of a COMPLEX array.  The members of this class are
       intrinsically short lived, so only the standard object routine for Kill is provided.

     Make_Slice takes a target array and the beg and end coordinates defining a slice
       within it, returning a Slice object that can then be used to iterate through the slice.
       If the underlying array is RGB, RGBA, or COMPLEX, then the beg and end coordinates may
       need not specify the range of the special dimension, in which case the entire range is
       assumed for said dimension.  The primitive Is_Slice returns a non-zero value if o is
       a slice as opposed to an array.  Slice_Array returns the array that a slice is of,
       Slice_First returns the beg coordinate, and Slice_Last returns the end coordinate
       (you do not own these coordinates, you are being given a pointer, not a reference).

     A Slice maintains a current index into the target array and auxiliary information so that
       it can efficiently get to the next index within the slice or tell you if an index is in
       the slice.  Slice_Index returns the current index and Slice_Coordinate returns a pointer
       to the Coordinate corresponding to said index (you do no own this coorindate!).

     Set_Slice_To_Index sets a Slice's current position within the target array and returns a
       non-zero value only if this position is within the slice.  Set_Slice_To_First sets a
       Slice's current position to its beg coordinate and returns the index of this coordinate.
       Set_Slice_To_Last sets a Slice's current position to its end coordinate and returns the
       index of this coordinate.  Inside_Slice returns a non-zero value only if the Slice's
       current position is within the slice.

     Next_Slice_Index jumps from the current slice position (which must be in the slice) to
       the next index in the slice and returns said index, if the slicer is already at the end
       of the slice, then it returns to the beginning.  Prev_Slice_Index similarly jumps
       the previous index in the slice.

     Inc_Slice advances slice's index to the next position and returns a non-zero
       value only if the new position is within the slice.  Similarly Dec_Slice decrements
       the current position.
  */

typedef void Slice;

Slice      *G(Make_Slice)(Array *I(target), Coordinate *C(beg), Coordinate *C(end));
void        Kill_Slice(Slice *K(slice));

int         Is_Slice(Array_Or_Slice *o);
Size_Type   Slice_Volume(Array_Or_Slice *o);
Array      *Slice_Array(Slice *slice);
Coordinate *Slice_First(Slice *slice);
Coordinate *Slice_Last(Slice *slice);

Indx_Type   Slice_Index(Slice *slice);
Coordinate *Slice_Coordinate(Slice *slice);

int         Set_Slice_To_Index(Slice *slice, Indx_Type idx);
Indx_Type   Set_Slice_To_First(Slice *slice);
Indx_Type   Set_Slice_To_Last(Slice *slice);
int         Inside_Slice(Slice *slice);

Indx_Type   Next_Slice_Index(Slice *slice);
Indx_Type   Prev_Slice_Index(Slice *slice);

int         Inc_Slice(Slice *slice);
int         Dec_Slice(Slice *slice);


  /* Print an array or slice in a tabular form.  The display depends on the kind of the array
       but is indented by 'indent' spaces from the left margin.  The format string must match
       the type of the data.
  */

void   Print_Array(Array_Or_Slice *a, FILE *output, int indent, char *format);
void   Print_Inuse_List(FILE *output, int indent);

  /* Get_Array_Plane returns a bundle that looks like an array, but the dims and data portions
       are pointers into a's structure.  If you really need an independent, proper object then
       call Copy_Array on the fetched plane.  Coded carefully so that nested calls such as
       Get_Array_Plane(Get_Array_Plane(a,1),3) return a valid bundle.  This is a nice, low-
       overhead way to get a routine to operate on, for example, the green channel, or a
       particular z-plane of a 3D stack, etc.
  */

typedef Array Array_Bundle;

Array_Bundle *Get_Array_Plane(Array *array, Dimn_Type plane);

  /*  Array_Range:          returns the smallest and largest value in array in the maxval
                              and minval fields of the return bundle.
      Scale_Array:          a[i] = factor * (a[i] + offset) for all i in basis of a.
      Scale_Array_To_Range: scales every element so that the largest (after scaling) equals
                              max and the smallest equals min.  Returns a for convenience.
  */

typedef struct      //  A return bundle (not an object)
  { Value maxval;
    Value minval;
  } Range_Bundle;

Range_Bundle   *Array_Range(Array_Or_Slice *array);
Array_Or_Slice *Scale_Array(Array_Or_Slice *R(M(array)), double factor, double offset);
Array_Or_Slice *Scale_Array_To_Range(Array_Or_Slice *R(M(array)), Value min, Value max);

  /* Array_Op_Scalar combines each element in a with value according to the selected Operator.
       The type of value is specified by the argument type.  Complex_Op_Scalar assumes a is
       COMPLEX and combines each complex element of a with the complex number rpart + i * ipart.

         Array_Op_Scalar(SET):    a[i]  = value 
         Array_Op_Scalar(ADD):    a[i] += value
         Array_Op_Scalar(SUB):    a[i] -= value
         Array_Op_Scalar(MUL):    a[i] *= value
         Array_Op_Scalar(DIV):    a[i] /= value
         Array_Op_Scalar(POW):    a[i] = a[i] ^ value   //  complex math if a is COMPLEX!
         Array_Op_Scalar(LSH):    a[i] <<= value
         Array_Op_Scalar(RSH):    a[i] >>= value
         Array_Op_Scalar(MIN):    if a[i] > value then a[i] = value
         Array_Op_Scalar(MAX):    if a[i] < value then a[i] = value

     Array_Op_Array combines, element-wise, the elements of arrays/slices a and b with operation
       op, leaving the result in the element of a.  a and b do not have to be the same size,
       type, or kind.  If b is larger than a, then the first a->size element of b are combined
       with the elements of a.  If b is smaller than a, then the elements of b are used in a
       circular fashion until every element of a has been operated upon.  For the routine
       Complex_Op_Array, a must be COMPLEX, and each non-complex value of b is combined element-
       wise with each complex element of a.  Similarly, Complex_Op_Complex expects both a and
       b to be COMPLEX, and combines their complex element pairs element-wise, applying complex
       operators.  For the shift operators, x+iy << c+id = (x << (int) c) + i (y << (int) d).
       Similarly, max(x+iy,c+id) = max(x,c) + i max(y,d).

     Threshold_Array binarizes an array with respect to a given threshold.  Array_Fct_Val
       applies a given function, fct, to each element of a.  Array_Fct_Idx calls the function,
       fct, with the coordinate of each element of the array and replaces the given element
       with the value returned.

         Threshold_Array:         a[i] = if a[i] <= cutoff then 0 else 1
         Array_Fct_Val:           a[i] = fct(a[i])
         Array_Fct_Idx:           a[i] = fct([ i0 i1 ... i(n-1) ])
  */

Array_Or_Slice *Array_Op_Scalar  (Array_Or_Slice *R(M(a)), Operator op, Value_Type type, Value val);
Array_Or_Slice *Complex_Op_Scalar(Array_Or_Slice *R(M(a)), Operator op,
                                  Value_Type type, Value rpart, Value ipart);

Array_Or_Slice *Array_Op_Array    (Array_Or_Slice *R(M(a)), Operator op, Array_Or_Slice *b);
Array_Or_Slice *Complex_Op_Array  (Array_Or_Slice *R(M(a)), Operator op, Array_Or_Slice *b);
Array_Or_Slice *Complex_Op_Complex(Array_Or_Slice *R(M(a)), Operator op, Array_Or_Slice *b);

Array_Or_Slice *Threshold_Array(Array_Or_Slice *R(M(a)), Value cutoff);
Array_Or_Slice *Array_Fct_Val  (Array_Or_Slice *R(M(a)), Value (*fct)(Value val));
Array_Or_Slice *Array_Fct_Idx  (Array_Or_Slice *R(M(a)), Value (*fct)(Coordinate *coord));

  /* Convert_Array_[Inplace/Copy] convert the subject array to one of the given kind, type, and
       scale.  In one case the conversion takes place in place and in the other a new array holds
       the converted object.  Basically the result is as if the assignment t[i] = f(s[i]) * beta
       were made for each base element i (i.e.  RGB-triple, COMPLEX-pair, etc), where t is the
       vector of target types, s is the vector of the source type, f is a function that converts
       between different kinds of data, and beta is a scaling factor to be described below.
       No cropping takes place -- if values of the right hand side do not fit into the container(s)
       of t[i], then you get whatever the compiler/architecture does in this situation. 

     When the kind remains unchanged, f is the identity function.  For conversions between
       kinds the transformation f is as follows:

         PLAIN to RGB  : t.X[i] = s[i] for X in {RGB}
         PLAIN to RGBA : t.X[i] = s[i] for X in {RGB}, t.A[i] = alpha
         RGB  to RGBA  : t.X[i] = s.X[i] for X in {RGB}, t.A[i] = alpha
         RGBA to RGB   : t.X[i] = s.X[i]*a for X in {RGB} where a = A[i] / alpha
         RGBA to PLAIN : t[i] = (.30 s.R[i] + .59 s.G[i] + .11 s.B[i]) * s.A[i] / alpha
         RGB  to PLAIN : t[i] = (.30 s.R[i] + .59 s.G[i] + .11 s.B[i])

            where alpha = 2^s.scale - 1        if s.type in { UINT8, UINT16, UINT32 }
                        = 2^(s.scale - 1) - 1  if s.type in { INT8, INT16, INT32 }
                        = 1                    otherwise

         PLAIN to COMPLEX : t.Real[i] = s[i], t.Imag[i] = 0
         COMPLEX to PLAIN : t[i] = sqrt(s.Real[i]^2 + s.Imag[i]^2)
  
         RGB or RGBA to COMPLEX or vice versa: composition of RGB[A] -> PLAIN -> COMPLEX

     Conceptually, after being converted/transformed as above all values are then scaled
       according to the type and scale of the source array and the specified type and
       scale of the target.  Please note carefully that parameter scale is an integer if
       s.type and type are both not floating point, and a double otherwise (we use the
       variable arguments mechanism of <stdarg.h> to realize this).  The scale factor
       beta is as follows:

          beta = 2 ^ (tbits-sbits) if both s.type and type are not floating point
               = scale             otherwise

             where tbits = scale      if type <= UINT32
                         = scale-1    otherwise

             and   sbits = s.scale    if s.type <= UINT32
                         = s.scale-1  otherwise

       In essence, for the case where both the source and the target are not floating point,
       then beta is chosen so that the range of the source is scaled to the range of the target.
       When either source or target is floating point then you get to choose beta.  The one
       drawback is if you convert from a float type to an int type then the scale will end up
       being set to the # of bits in the target type -- you must explicitly set the scale after
       invoking the conversion if you chose beta so that it can or should be smaller.
  */

Array *Convert_Array_Inplace(Array *R(M(array)), Array_Kind kind, Array_Type type,
                                                  ... X( int|double scale ) );
Array *G(Convert_Array_Copy)(Array *array, Array_Kind kind, Array_Type type,
                                                  ... X( int|double scale ) );

  /* Convert_Image converts one image to another scaling as appropriate.  The Inplace version does
       the conversion in place (resizing the array if it needs to expand) and the Copy version makes
       a new copy to hold the result leaving the original unchanged.  The values in an image
       are not arbitrary.  For an integer image, the largest positive integer value represents
       full on and 0 represents full off.  That is, full on for a signed int type is 2^(scale-1)-1
       and for an unsigned int type it is 2^scale-1.  For floating images, 1. represents full on
       and 0 represents full off.  The scaling performed by Convert_Image preserves the relative
       brightness/opacity of each channel value.  The parameter scale is irrelevant if the target
       type is floating point.

     Image_Check returns zero iff the array encodes a proper image, that is the values are
       all between 0 and 2^(scale-1)-1 for a signed integer array, are all between 0 and
       2^scale-1 for an unsigned integer array, and between 0. and 1., inclusive, for a
       floating point array.
  */

int    Image_Check(Array *array);

Array *Convert_Image_Inplace(Array *R(M(array)), Array_Kind kind, Array_Type type, int scale);
Array *G(Convert_Image_Copy)(Array *array, Array_Kind kind, Array_Type type, int scale);

  /* Multiply arrays a and b together (the order matters).  Both must be of the same type.
       If either a or b is COMPLEX then complex arithmetic is performed and a COMPLEX array
       is returned.
  */

Array *G(Array_Multiply)(Array *a, Array *b);

  /*  A generic routine for mapping array data, but specifically inspired by image color maps.
      Image must be a PLAIN array of unsigned integer data.  Map must have dimension at least 1
      and its inner dimension must be equal to 2^(image->scale), i.e. the range of values in
      image.  Each value in image is mapped to an array of map->size/map->dims[0] values, that
      is an array whose dimension are those of map less its innermost dimension.  The
      dimensionality of the result is thus maps->ndims-1 greater than image, and its outer most
      new dimensions are those of map, formally, a'[q.p] = m[q.a[p]].  The kind and scale of the
      result are the same as map.  A new array is created to hold the result and a pointer to it
      is returned.
  */

Array *G(Apply_Map)(Array *image, Array *map);

  /*  Downsample array source where the downsampling in each dimension is given by box.
      For example, Coord("2,3,4"), would downsample a 3 dimensional array by 4 pixels in
      dimension 0, 3 pixels in dimension 1, and 2 pixels in dimension 2.  If the array is
      RGB, RGBA, or COMPLEX then the downsample vector need not specify the downsampling in
      the special dimension, in which case it is assumed to be 1 (i.e. no downsampling).
      Inplace uses the space occupied by source, and Copy generates a new object.  In the
      case of Inplace, pack the array (Pack_Array) if you want to free up the space no longer
      required for the downsampled array.
  */

Array *Down_Sample_Inplace(Array *R(M(source)), Coordinate *F(box));
Array *G(Down_Sample_Copy)(Array *source, Coordinate *F(box));

  /*  Select the subset of the array in the rectangle whose opposing corners are at the
      indices beg and end.  For example, if beg = (2,4,3) and end = (6,7,8) then the
      sub-array source[2..6][4..7][3..8] will be returned.  If the array is RGB, RGBA,
      or COMPLEX, then the coordinates do not need to specify the special dimension (but
      they may) in which case the entire range of that dimension is assumed.  Inplace and
      Copy have the usual meanings (see Down_Sample above).
  */

Array *Clip_Array_Inplace(Array *R(M(source)), Coordinate *F(beg), Coordinate *F(end));
Array *G(Clip_Array_Copy)(Array *source, Coordinate *F(beg), Coordinate *F(end));

  /*  Put the array source in an array of shape 'shape' where the zero-pixel of source
      is place at coordinate 'anchor' in the padded array.  Pad the array with zeros.
      If the array is RGB, RGBA, or COMPLEX, then the coordinates do not need to specify
      the special dimension (but they may) in which case the special dimension is not
      padded.  Inplace and Copy have the usual meansings.
  */

Array *Pad_Array_Inplace(Array *R(M(source)), Coordinate *F(anchor), Coordinate *F(shape));
Array *G(Pad_Array_Copy)(Array *source, Coordinate *F(anchor), Coordinate *F(shape));

  /*  Correlation generates an n x n correlation matrix, c, where c[i,j] = sum_t ai[t]*aj[t].
        The only constraint on the ai's is that they all have the same number of elements
        (but not necessarily the same shape).
      Similarly, Covariance generates an n x n matrix, c, where
        c[i,j] = sum_t (ai[t]-mi)*(aj[t]-mj)/A where mi is the mean of the elements
        in ai, mj the mean of aj, and A the # of elements in ai/aj.
      Lastly, Pearson_Sorrelation generates an n x n matrix, c, where
        c[i,j] = sum_t (ai[t]-mi)*(aj[t]-mj)/(si*sj*A) where sii is the standard deveiation
        of the elements in ai, and sj the standard devition of aj.
  */

Double_Matrix *G(Correlation)(int n, Array_Or_Slice *a0, ... X(Array_Or_Slice *an-1));
Double_Matrix *G(Covariance)(int n, Array_Or_Slice *a0, ... X(Array_Or_Slice *an-1));
Double_Matrix *G(Pearson_Correlation)(int n, Array_Or_Slice *a0, ... X(Array_Or_Slice *an-1));

   /*  Linear regression generates an n element vector of coefficients, c, such that
       the least squares distance between obs and the c-weighted sum of the ai's is minimal,
       that is,
                sum_t (obs[t] - (c[0]*a0[t] + ... + c[n-1]*an-1[t]))^2
       is minimized.
   */

Double_Vector *G(Linear_Regression)(int n, Array_Or_Slice *obs,
                                           Array_Or_Slice *a0, ... X(Array_Or_Slice *an-1));

#endif
