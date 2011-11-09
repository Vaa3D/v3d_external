/*****************************************************************************************\
*                                                                                         *
*  Array Data Abstraction                                                                 *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  December 2008                                                                 *
*                                                                                         *
*  (c) July 27, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
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
#include "linear.algebra.h"

#LISTDEF @KINDS = PLAIN_KIND RGB_KIND RGBA_KIND COMPLEX_KIND

#LISTDEF @TYPES  =  UINT8 UINT16 UINT32 UINT64  INT8 INT16 INT32 INT64 FLOAT32 FLOAT64
#LISTDEF @UNION  =   uval   uval   uval   uval  ival  ival  ival  ival    rval    rval
#LISTDEF @CASTES = uint64 uint64 uint64 uint64 int64 int64 int64 int64  double  double
#LISTDEF @SIZES  =      1      2      4      8     1     2     4     8       4       8

#LISTDEF @OPS  = SET_OP ADD_OP SUB_OP MUL_OP DIV_OP POW_OP LSH_OP RSH_OP MIN_OP MAX_OP
#LISTDEF @SYMS =      =     +=     -=     *=     /=      =    <<=    >>=      >      <

/****************************************************************************************
 *                                                                                      *
 *  ARRAY SIZE, SHAPE & TYPE COMPARISONS                                                *
 *                                                                                      *
 ****************************************************************************************/

static Size_Type array_size(Array *a)
{ Size_Type p;
  int       i;
  p = 1;
  for (i = 0; i < a->ndims; i++)
    p *= a->dims[i];
  return (p);
}

int Same_Shape(Array *a, Array *b)
{ int i;

  if (a->ndims != b->ndims)
    return (0);
  for (i = 0; i < a->ndims; i++)
    if (a->dims[i] != b->dims[i])
      return (0);
  return (1);
}

int Same_Type(Array *a, Array *b)
{ if (a->type != b->type)
    return (0);
  return (Same_Shape(a,b));
}


/****************************************************************************************
 *                                                                                      *
 *  ARRAY SPACE MANAGEMENT ROUTINES                                                     *
 *                                                                                      *
 ****************************************************************************************/

// Awk-generated (manager.awk) Array memory management

static int type_size[] = { 1, 2, 4, 8, 1, 2, 4, 8, 4, 8 };

static int bit_size[] = { 8, 16, 32, 64, 8, 16, 32, 64, 32, 64 };

static int kind_size[] = { 1, 3, 4, 2 };

static int dimn_type[] = { 0, UINT8, UINT16, 0, UINT32, 0, 0, 0, UINT64 };

static inline int array_nsize(Array *a)
{ return (sizeof(int)*a->ndims); }

static inline uint64 array_dsize(Array *a)
{ return (a->size*type_size[a->type]); }

static inline int array_tsize(Array *a)
{ return (a->tlen); }

MANAGER -IO Array dims:nsize data!dsize text:tsize


/****************************************************************************************
 *                                                                                      *
 *  MAKE AN ARRAY                                                                       *
 *                                                                                      *
 ****************************************************************************************/

static Array *make_start(Array_Kind kind, Array_Type type, int ndims, char *routine)
{ Array  *a;

  a = new_array(sizeof(int)*ndims,0,1,routine);
  a->type    = type;
  a->kind    = kind;
  a->scale   = bit_size[type];
  a->text[0] = '\0';
  a->tlen    = 1;
  a->ndims   = ndims;
  return (a);
}

static Array *make_shape(Array_Kind kind, Array_Type type, int ndims, Dimn_Type *dims,
                         char *routine)
{ Array *a;
  int    i, o, v;

  o = (kind != PLAIN_KIND);
  v = (kind == COMPLEX_KIND);
  a = make_start(kind,type,ndims+o,routine);
  if (v)
    a->dims[0] = kind_size[kind];
  for (i = 0; i < ndims; i++)
    a->dims[i+v] = dims[i];
  if (o & !v)
    a->dims[ndims] = kind_size[kind];
  a->size = array_size(a);
  return (a);
}

Array *G(Make_Array)(Array_Kind kind, Array_Type type, int ndims, Dimn_Type *dims)
{ Array *a;

  a = make_shape(kind,type,ndims,dims,"Make_Array");

  allocate_array_data(a,array_dsize(a),"Make_Array");

  return (a);
}

Array *G(Make_Array_With_Shape)(Array_Kind kind, Array_Type type, Coordinate *F(shape))
{ Array *a;

  a = make_shape(kind,type,shape->dims[0],ADIMN(shape),"Make_Array_With_Shape");

  allocate_array_data(a,array_dsize(a),"Make_Array_With_Shape");

  Free_Array(shape);

  return (a);
}

Array *G(Make_Array_Of_Data)(Array_Kind kind, Array_Type type, int ndims, Dimn_Type *dims,
                             void *data)
{ Array *a;

  a = make_shape(kind,type,ndims,dims,"Make_Array_Of_Data");

  { _Array *object = (_Array *) (((char *) a) - Array_Offset);
    if (object->dsize > 0)
      free(a->data);
    object->dsize  = array_dsize(a);
    a->data = data;
  }

  return (a);
}

Array *G(Make_Array_From_Arrays)(Array_Kind kind, int n, Array **arrays)
{ Array    *a, *a0;
  int       i, ndims;
  Size_Type dsize;

  a0 = arrays[0];
  if (n > 1)
    { for (i = 1; i < n; i++)
        if ( ! Same_Type(arrays[i],a0))
          { fprintf(stderr,
               "Arrays are not all of the same type and shape (Make_Array_From_Arrays)\n");
            exit (1);
          }
    }
  if (kind == COMPLEX_KIND && a0->ndims > 0)
    { for (i = 0; i < n; i++)
        if (arrays[i]->kind != COMPLEX_KIND)
          { fprintf(stderr,"Arrays must all be COMPLEX (Make_Array_From_Arrays)\n");
            exit (1);
          }
    }
  else
    { for (i = 0; i < n; i++)
        if (arrays[i]->kind != PLAIN_KIND)
          { fprintf(stderr,"Arrays must all be PLAIN (Make_Array_From_Arrays)\n");
            exit (1);
          }
    }
  if (kind != PLAIN_KIND && (kind != COMPLEX_KIND || a0->ndims == 0) && n != kind_size[kind])
    { fprintf(stderr,"Outer dimension and kind are not consistent (Make_Array_From_Arrays)\n");
      exit (1);
    }

  dsize = array_dsize(arrays[0]);
  ndims = a0->ndims;

  a = new_array(sizeof(int)*(ndims+1),n*dsize,1,"Make_Array_From_Arrays");

  a->type    = a0->type;
  a->kind    = kind;
  a->scale   = a0->scale;
  a->tlen    = 1;
  a->text[0] = '\0';
  a->ndims   = ndims+1;
  for (i = 0; i < ndims; i++)
    a->dims[i] = a0->dims[i];
  a->dims[ndims] = n;
  a->size        = n * a0->size;

  for (i = 0; i < n; i++)
    memcpy(a->data + i*dsize, arrays[i]->data, dsize);

  return (a);
}


/****************************************************************************************
 *                                                                                      *
 *  COORDS, GET & SET                                                                   *
 *                                                                                      *
 ****************************************************************************************/

static Dimn_Type  Coord_Ndim, Coord_Max = 0;
static Dimn_Type *Coord_Dims;

void Set_Coord_Basis(Coordinate *F(point))
{ if (point == NULL)
    { Coord_Dims = NULL;
      Coord_Ndim = 0;
      return;
    }
  if (point->ndims != 1 || point->type != dimn_type[sizeof(Dimn_Type)])
    { fprintf(stderr,"Coordinate is not an integer vector (Set_Coord_Basis)\n");
      exit (1);
    }
  if (point->dims[0] > Coord_Max)
    { Coord_Max = point->dims[0] + 10;
      Coord_Dims = Guarded_Realloc(Coord_Dims,sizeof(Dimn_Type)*Coord_Max,"Set_Coord_Basis");
    }
  Coord_Ndim = point->dims[0];
  memcpy(Coord_Dims,ADIMN(point),sizeof(Dimn_Type)*Coord_Ndim);
  Free_Array(point);
}

Coordinate *G(Get_Coord_Basis)()
{ Dimn_Type  dim[1];
  Array     *point;

  if (Coord_Dims == NULL)
    return (NULL);
  dim[0] = Coord_Ndim;
  point  = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dim);
  memcpy(ADIMN(point),Coord_Dims,sizeof(Dimn_Type)*Coord_Ndim);
  return (point);
}

void Use_Array_Basis(Array *a)
{ if (a->ndims > Coord_Max)
    { Coord_Max = a->ndims + 10;
      Coord_Dims = Guarded_Realloc(Coord_Dims,sizeof(Dimn_Type)*Coord_Max,"Use_Array_Basis");
    }
  Coord_Ndim = a->ndims;
  memcpy(Coord_Dims,a->dims,sizeof(Dimn_Type)*Coord_Ndim);
}

Coordinate *G(Coord)(char *list)
{ Array     *coord;
  Dimn_Type  dim[1], *c;
  int        i, n;
  char      *b, *e;

  n = 1;
  for (i = 0; i < strlen(list); i++)
    if (list[i] == ',')
      n += 1;

  dim[0] = n;
  coord  = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dim);

  c = ADIMN(coord);
  b = list;
  while (1)
    { c[--n] = strtol(b,&e,10);
      if (b == e)
        { fprintf(stderr,"Not a valid constant coordinate list (Coord)\n");
          exit (1);
        }
      b = e;
      if (*b == '\0')
        break;
      if (*b++ != ',')
        { fprintf(stderr,"Not a valid constant coordinate list (Coord)\n");
          exit (1);
        }
    }

  return (coord);
}

char *Coord_String(Coordinate *point)
{ static int   Buffer_Max = 0;
  static char *Buffer_Ret = NULL;

  char buffer[100];
  int  i, len;

  int        n = point->dims[0];
  Dimn_Type *d = ADIMN(point);

  switch (point->type) {
    #GENERATE T,F = @TYPES, %u %u %u %llu %d %d %d %lld %g %g
      case <T>:
        len = snprintf(buffer,1,"<F>",d[0]);
        for (i = 1; i < n; i++)
          len += snprintf(buffer,1,",<F>",d[i]);
        break;
    #END
  }

  if (len > Buffer_Max)
    { Buffer_Max = 1.1*len + 30;
      Buffer_Ret = (char *) Guarded_Realloc(Buffer_Ret,Buffer_Max+1,"Coord_String");
    }

  switch (point->type) {
    #GENERATE T,F = @TYPES, %u %u %u %llu %d %d %d %lld %g %g
      case <T>:
        len = sprintf(Buffer_Ret,"<F>",d[n-1]);
        for (i = n-2; i >= 0; i--)
          len += sprintf(Buffer_Ret+len,",<F>",d[i]);
        break;
    #END
  }

  Free_Array(point);

  return (Buffer_Ret);
}

Coordinate *G(Coord1)(Dimn_Type d1)
{ Array     *coord;
  Dimn_Type  dim[1], *c;

  dim[0] = 1;
  coord  = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dim);
  c      = ADIMN(coord);
  c[0]   = d1;
  return (coord);
}

Coordinate *G(Coord2)(Dimn_Type d2, Dimn_Type d1)
{ Array     *coord;
  Dimn_Type  dim[1], *c;

  dim[0] = 2;
  coord  = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dim);
  c      = ADIMN(coord);
  c[0]   = d1;
  c[1]   = d2;
  return (coord);
}

Coordinate *G(Coord3)(Dimn_Type d3, Dimn_Type d2, Dimn_Type d1)
{ Array     *coord;
  Dimn_Type  dim[1], *c;

  dim[0] = 3;
  coord  = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dim);
  c      = ADIMN(coord);
  c[0]   = d1;
  c[1]   = d2;
  c[2]   = d3;
  return (coord);
}

Coordinate *G(Coord4)(Dimn_Type d4, Dimn_Type d3, Dimn_Type d2, Dimn_Type d1)
{ Array     *coord;
  Dimn_Type  dim[1], *c;

  dim[0] = 4;
  coord  = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dim);
  c      = ADIMN(coord);
  c[0]   = d1;
  c[1]   = d2;
  c[2]   = d3;
  c[3]   = d4;
  return (coord);
}

Indx_Type Coord2Idx(Coordinate *F(point))
{ Dimn_Type *x, y;
  Indx_Type  p;
  int        n, i;

  if (Coord_Dims == NULL)
    { fprintf(stderr,"Coordinate basis is not set (Coord2Idx)\n");
      exit (1);
    }
  if (point->ndims != 1 || point->type != dimn_type[sizeof(Dimn_Type)])
    { fprintf(stderr,"Point is not an integer vector (Coord2Idx)\n");
      exit (1);
    }
  if (point->dims[0] != Coord_Ndim)
    { fprintf(stderr,"Coordinate dimensionality doesn't match that of current basis (Coord2Idx)\n");
      exit (1);
    }

  x = ADIMN(point);
  p = 0;
  for (i = Coord_Ndim-1; i >= 0; i--)
    { y = x[i];
      if (y >= Coord_Dims[i])
        { fprintf(stderr,"%d'th index out of bounds (Coord2Idx)\n",i+1);
          exit (1);
        }
      p = p*Coord_Dims[i] + y; 
    }

  Free_Array(point);

  return (p);
}

Indx_Type Coord2IdxA(Array *a, Coordinate *F(point))
{ Dimn_Type *x, *d, y;
  Indx_Type  p;
  int        n, i;

  if (point->ndims != 1 || point->type != dimn_type[sizeof(Dimn_Type)])
    { fprintf(stderr,"Point is not an integer vector (Coord2IdxA)\n");
      exit (1);
    }

  n = a->ndims;
  d = a->dims;

  if (point->dims[0] != n)
    { fprintf(stderr,"Coordinate dimensionality doesn't match that of array basis (Coord2IdxA)\n");
      exit (1);
    }

  x = ADIMN(point);
  p = 0;
  for (i = n-1; i >= 0; i--)
    { y = x[i];
      if (y >= d[i])
        { fprintf(stderr,"%d'th index out of bounds (Coord2IdxA)\n",i+1);
          exit (1);
        }
      p = p*d[i] + y; 
    }

  Free_Array(point);

  return (p);
}

Coordinate *G(Array_Basis)(Array *a)
{ Array    *coord;
  Dimn_Type dim[1];

  dim[0] = a->ndims;
  coord = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dim);
  memcpy(ADIMN(coord),a->dims,sizeof(Dimn_Type)*a->ndims);
  return (coord);
}

Coordinate *AddCoord(int d, Coordinate *R(M(c)))
{ Dimn_Type n;
  n = c->dims[0];
  allocate_array_data(c,(n+1)*sizeof(Dimn_Type),"AddCoord");
  c->dims[0]  = n+1;
  c->size     = n+1;
  ADIMN(c)[n] = d;
  return (c);
}

Coordinate *G(Idx2Coord)(Indx_Type idx)
{ Dimn_Type   dims[1], m, *l;
  Coordinate *lat;
  int         i;

  if (Coord_Dims == NULL)
    { fprintf(stderr,"Coordinate basis is not set (Idx2Coord)\n");
      exit (1);
    }

  dims[0] = Coord_Ndim;
  lat     = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dims);

  l = ADIMN(lat);
  for (i = 0; i < Coord_Ndim; i++)
    { m = Coord_Dims[i];
      l[i] = idx % m;
      idx /= m;
    }
  if (idx > 0)
    { fprintf(stderr,"Index is out of current basis boundary (Idx2Coord)\n");
      exit (1);
    }
  return (lat);
}

Coordinate *G(Idx2CoordA)(Array *a, Indx_Type idx)
{ Dimn_Type   dims[1], m, *l, *d;
  Coordinate *lat;
  int         i, n;

  n = a->ndims;
  d = a->dims;

  dims[0] = n;
  lat     = Make_Array(PLAIN_KIND,dimn_type[sizeof(Dimn_Type)],1,dims);

  l = ADIMN(lat);
  for (i = 0; i < n; i++)
    { m = d[i];
      l[i] = idx % m;
      idx /= m;
    }
  if (idx > 0)
    { fprintf(stderr,"Index is out of array boundary (Idx2CoordA)\n");
      exit (1);
    }
  return (lat);
}

Value Get_Array_Value(Array *a, Coordinate *coord)
{ Indx_Type p = Coord2IdxA(a,coord);
  switch (a->type) {
    #GENERATE T,C = @TYPES, @CASTES
      case <T>:
        return ((Value) (<C>) A<T>(a)[p]);
    #END
  }
}

void Set_Array_Value(Array *M(a), Coordinate *coord, Value v)
{ Indx_Type p = Coord2IdxA(a,coord);
  switch (a->type) {
    #GENERATE T,U = @TYPES, @UNION
      case <T>:
        A<T>(a)[p] = v.<U>;
        break;
    #END
  }
}

Integer_Array *G(Floor_Coord)(Double_Vector *point)
{ Array     *lat;
  double    *p;
  Dimn_Type *c, k;

  if (point->ndims != 1 || point->kind != PLAIN_KIND || point->type != FLOAT64)
    { fprintf(stderr,"Point is not a double vector (Floor_Coord)\n");
      exit (1);
    }

  lat = Make_Array(PLAIN_KIND,INT32,1,point->dims);

  p = AFLOAT64(point);
  c = ADIMN(lat);
  for (k = 0; k < point->dims[0]; k++)
    c[k] = floor(p[k]);

  return (lat);
}

Integer_Array *G(Ceiling_Coord)(Double_Vector *point)
{ Array     *lat;
  double    *p;
  Dimn_Type *c, k;

  if (point->ndims != 1 || point->kind != PLAIN_KIND || point->type != FLOAT64)
    { fprintf(stderr,"Point is not a double vector (Ceiling_Coord)\n");
      exit (1);
    }

  lat = Make_Array(PLAIN_KIND,INT32,1,point->dims);

  p = AFLOAT64(point);
  c = ADIMN(lat);
  for (k = 0; k < point->dims[0]; k++)
    c[k] = ceil(p[k]);

  return (lat);
}

Integer_Array *G(Nearest_Coord)(Double_Vector *point)
{ Array     *lat;
  double    *p;
  Dimn_Type *c, k;

  if (point->ndims != 1 || point->kind != PLAIN_KIND || point->type != FLOAT64)
    { fprintf(stderr,"Point is not a double vector (Floor_Coord)\n");
      exit (1);
    }

  lat = Make_Array(PLAIN_KIND,INT32,1,point->dims);

  p = AFLOAT64(point);
  c = ADIMN(lat);
  for (k = 0; k < point->dims[0]; k++)
    c[k] = round(p[k]);

  return (lat);
}


/****************************************************************************************
 *                                                                                      *
 *  SLICE ITERATOR ROUTINES                                                             *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { int         kind;     //  Always COMPLEX_KIND+1 in order to distingusih from an Array
    int         clip;     //  Highest dimension currently outside of slice
    Size_Type   p;        //  Current index
    Coordinate *cnt;      //  Current coordinate (= index)
    Size_Type  *dnc;      //  dnc[i] = displacement to next slice element when at an i boundary
    Array      *beg_ref;  //  References to the beg and end coordinate defining slice ...
    Array      *end_ref;
    Array      *trg_ref;  //  ... and the array the slice is in.
  } Slicer;

static void print_slice(Slicer *s)
{ Dimn_Type *cnt = ADIMN(s->cnt);
  int i;

  printf("Slice %d %llu:",s->clip,s->p);
  for (i = 0; i < s->trg_ref->ndims; i++)
    printf(" %u",cnt[i]);
  printf("\n");
}

Slice *Make_Slice(Array *I(target), Coordinate *C(beg), Coordinate *C(end))
{ Slicer    *slice;
  Dimn_Type *ecrd = ADIMN(end);
  Dimn_Type *bcrd = ADIMN(beg);
  Dimn_Type *dims;
  int        i, ndim;

  ndim = beg->size;
  if (!(target->ndims == ndim || target->ndims-1 == ndim && target->kind != PLAIN_KIND) ||
       beg->dims[0] != end->dims[0])
    { fprintf(stderr,"Target and coordinate dimensionality do not match (Make_Slice)\n");
      exit (1);
    }
  if (end->size != ndim)
    { fprintf(stderr,"Begin and end coordinate dimensionality do not match (Make_Slice)\n");
      exit (1);
    }

  if (ndim == target->ndims-1 && target->kind == COMPLEX_KIND)
    dims = target->dims+1;
  else
    dims = target->dims;

  for (i = 0; i < ndim; i++)
    { if (bcrd[i] > ecrd[i])
        { fprintf(stderr,"beg is not before end in target array (Make_Slice)\n");
          exit (1);
        }
      if (ecrd[i] >= dims[i])
        { fprintf(stderr,"end is not in basis of target array (Make_Slice)\n");
          exit (1);
        }
    }

  if (ndim == target->ndims-1)
    { ndim += 1;
      beg   = AddCoord(0,beg);
      end   = AddCoord(kind_size[target->kind]-1,end);
      if (target->kind == COMPLEX_KIND)
        { Dimn_Type *bcrd = ADIMN(beg);
          Dimn_Type *ecrd = ADIMN(end);
          for (i = ndim-1; i >= 1; i--)
            { bcrd[i] = bcrd[i-1];
              ecrd[i] = ecrd[i-1];
            }
          bcrd[0] = 0;
          ecrd[0] = 1;
        }
    }

  slice = (Slicer *) Guarded_Malloc(sizeof(Slicer),"Make_Slice");
  slice->kind = COMPLEX_KIND+1;
  slice->cnt  = Copy_Array(beg);
  slice->dnc  = (Size_Type *) Guarded_Malloc(sizeof(Size_Type)*target->ndims,"Make_Slice");
  slice->trg_ref = Inc_Array(target);
  slice->beg_ref = beg;
  slice->end_ref = end;

  { Size_Type *dinc = slice->dnc;
    Dimn_Type *bcrd = ADIMN(beg);
    Size_Type  offset, outer;

    offset  = 0;
    outer   = 1;
    for (i = 0; i < ndim; i++)
      { dinc[i] = outer-offset;
        offset += (ecrd[i]-bcrd[i])*outer;
        outer  *= target->dims[i];
      }

    slice->p = Coord2IdxA(target,Inc_Array(beg));
    slice->clip = -1;
  }

  return ((Slice *) slice);
}

void Kill_Slice(Slice *K(slicer))
{ Slicer *slice = (Slicer *) slicer;
  Free_Array(slice->trg_ref);
  Free_Array(slice->beg_ref);
  Free_Array(slice->end_ref);
  Free_Array(slice->cnt);
  free(slice->dnc);
  free(slice);
}

int Is_Slice(Array_Or_Slice *o)
{ return (((Slicer *) o)->kind > COMPLEX_KIND); }

Size_Type Slice_Volume(Array_Or_Slice *o)
{ Slicer *s = (Slicer *) o;
  if (s->kind <= COMPLEX_KIND)
    return (((Array *) s)->size);
  else
    { Dimn_Type *bcrd = AUINT32(s->beg_ref);
      Dimn_Type *ecrd = AUINT32(s->end_ref);
      Size_Type  volume = 1;
      int        i;
      for (i = 0; i < s->trg_ref->ndims; i++)
        volume *= (ecrd[i] - bcrd[i]) + 1;
      return (volume);
    }
}

Indx_Type Slice_Index(Slice *slicer)
{ return (((Slicer *) slicer)->p);  }

Coordinate *Slice_Coordinate(Slice *slicer)
{ return (((Slicer *) slicer)->cnt); }

Coordinate *Slice_First(Slice *slicer)
{ return (((Slicer *) slicer)->beg_ref); }

Coordinate *Slice_Last(Slice *slicer)
{ return (((Slicer *) slicer)->end_ref); }

int Inside_Slice(Slice *slicer)
{ return (((Slicer *) slicer)->clip < 0);  }

Array *Slice_Array(Slice *slicer)
{ return (((Slicer *) slicer)->trg_ref);  }

int Slice_Kind(Slice *slicer)
{ Slicer *slice = (Slicer *) slicer;
  int     ndims, kind;

  ndims = slice->trg_ref->ndims;
  kind  = slice->trg_ref->kind;
  if (kind == RGB_KIND || kind == RGBA_KIND)
    { if (ADIMN(slice->beg_ref)[ndims-1] != 0 ||
          ADIMN(slice->end_ref)[ndims-1] != kind_size[kind]-1)
        kind = PLAIN_KIND;
    }
  else if (kind == COMPLEX_KIND)
    { if (ADIMN(slice->beg_ref)[0] != 0 || ADIMN(slice->end_ref)[0] != 1)
        kind = PLAIN_KIND;
    }
  return (kind);
}

Coordinate *G(Slice_Basis)(Slice *slicer)
{ Slicer     *slice = (Slicer *) slicer;
  Dimn_Type  *bcrd, *bval;
  Coordinate *basis;
  int         i;

  basis = Copy_Array(slice->end_ref);
  bval  = ADIMN(basis);
  bcrd  = ADIMN(slice->beg_ref);
  for (i = 0; i < slice->trg_ref->ndims; i++)
    bval[i] = (bval[i] - bcrd[i]) + 1;
  return (basis);
}

int Set_Slice_To_Index(Slice *slicer, Size_Type idx)
{ Slicer    *slice = (Slicer *) slicer;
  Dimn_Type *bcrd = ADIMN(slice->beg_ref);
  Dimn_Type *ecrd = ADIMN(slice->end_ref);
  Dimn_Type *cnt  = ADIMN(slice->cnt);
  int        ndim = slice->trg_ref->ndims;
  Dimn_Type  d, c;
  int        i;

  if (idx >= slice->trg_ref->size)
    { fprintf(stderr,"Index is not in target array basis (Set_Slice_To_Index)\n");
      exit (1);
    }

  slice->p    = idx;
  slice->clip = -1;
  for (i = 0; i < ndim; i++)
    { d = slice->trg_ref->dims[i];
      cnt[i] = c = idx % d;
      if (c < bcrd[i] || c > ecrd[i])
        slice->clip = i;
      idx = idx / d;
    }
  return (slice->clip < 0);
}

Indx_Type Set_Slice_To_First(Slice *slicer)
{ Slicer    *slice = (Slicer *) slicer;
  Dimn_Type *bcrd = ADIMN(slice->beg_ref);
  Dimn_Type *cnt  = ADIMN(slice->cnt);
  Dimn_Type *dim  = slice->trg_ref->dims;
  int        ndim = slice->trg_ref->ndims;
  int        i;

  slice->p = 0;
  for (i = ndim-1; i >= 0; i--)
    { cnt[i] = bcrd[i];
      slice->p = slice->p * dim[i] + bcrd[i];
    }
  slice->clip = -1;
  return (slice->p);
}

Indx_Type Set_Slice_To_Last(Slice *slicer)
{ Slicer    *slice = (Slicer *) slicer;
  Dimn_Type *ecrd = ADIMN(slice->end_ref);
  Dimn_Type *cnt  = ADIMN(slice->cnt);
  Dimn_Type *dim  = slice->trg_ref->dims;
  int        ndim = slice->trg_ref->ndims;
  int        i;

  slice->p = 0;
  for (i = ndim-1; i >= 0; i--)
    { cnt[i] = ecrd[i];
      slice->p = slice->p * dim[i] + ecrd[i];
    }
  slice->clip = -1;
  return (slice->p);
}

Indx_Type Next_Slice_Index(Slice *slicer)
{ Slicer    *slice = (Slicer *) slicer;
  Dimn_Type *bcrd = ADIMN(slice->beg_ref);
  Dimn_Type *ecrd = ADIMN(slice->end_ref);
  Dimn_Type *cnt  = ADIMN(slice->cnt);
  int        ndim = slice->trg_ref->ndims;
  int        i;

  if (slice->clip >= 0)
    { fprintf(stderr,"Must be in slice to move to next position (Next_Slice_Index)\n");
      exit (1);
    }

  for (i = 0; i < ndim; i++)
    if (++cnt[i] > ecrd[i])
      cnt[i] = bcrd[i];
    else
      return (slice->p += slice->dnc[i]);
  slice->p = Coord2IdxA(slice->trg_ref,Inc_Array(slice->beg_ref));
  return (slice->p);
}

Indx_Type Prev_Slice_Index(Slice *slicer)
{ Slicer    *slice = (Slicer *) slicer;
  Dimn_Type *bcrd = ADIMN(slice->beg_ref);
  Dimn_Type *ecrd = ADIMN(slice->end_ref);
  Dimn_Type *cnt  = ADIMN(slice->cnt);
  int        ndim = slice->trg_ref->ndims;
  int        i;

  if (slice->clip >= 0)
    { fprintf(stderr,"Must be in slice to move to next position (Prev_Slice_Index)\n");
      exit (1);
    }

  for (i = 0; i < ndim; i++)
    if (--cnt[i] < bcrd[i])
      cnt[i] = ecrd[i];
    else
      return (slice->p -= slice->dnc[i]);
  slice->p = Coord2IdxA(slice->trg_ref,Inc_Array(slice->end_ref));
  return (slice->p);
}

int Inc_Slice(Slice *slicer)
{ Slicer    *slice = (Slicer *) slicer;
  Dimn_Type *tcrd = slice->trg_ref->dims;
  Dimn_Type *bcrd = ADIMN(slice->beg_ref);
  Dimn_Type *ecrd = ADIMN(slice->end_ref);
  Dimn_Type *cnt  = ADIMN(slice->cnt);
  int        ndim = slice->trg_ref->ndims;
  int        i, nclip, clip;
  
  clip  = slice->clip;
  nclip = -1;
  for (i = 0; i < ndim; i++)
    if (++cnt[i] == tcrd[i])
      { cnt[i] = 0;
        if (cnt[i] < bcrd[i])
          nclip = i;
        else if (clip == i)
          clip = -1;
      }
    else
      { if (cnt[i] > ecrd[i])
          nclip = i;
        else if (clip == i && cnt[i] >= bcrd[i])
          clip = -1;
        break;
      }
  if (nclip > clip)
    clip = nclip;
  slice->clip = clip;
  slice->p   += 1;
  return (clip < 0);
}

int Dec_Slice(Slice *slicer)
{ Slicer    *slice = (Slicer *) slicer;
  Dimn_Type *tcrd = slice->trg_ref->dims;
  Dimn_Type *bcrd = ADIMN(slice->beg_ref);
  Dimn_Type *ecrd = ADIMN(slice->end_ref);
  Dimn_Type *cnt  = ADIMN(slice->cnt);
  int        ndim = slice->trg_ref->ndims;
  int        i, nclip, clip;
  
  clip  = slice->clip;
  nclip = -1;
  for (i = 0; i < ndim; i++)
    if (cnt[i]-- == 0)
      { cnt[i] = tcrd[i]-1;
        if (cnt[i] > ecrd[i])
          nclip = i;
        else if (clip == i)
          clip = -1;
      }
    else
      { if (cnt[i] < bcrd[i])
          nclip = i;
        else if (clip == i && cnt[i] <= ecrd[i])
          clip = -1;
        break;
      }
  if (nclip > clip)
    clip = nclip;
  slice->clip = clip;
  slice->p   -= 1;
  return (clip < 0);
}


/****************************************************************************************
 *                                                                                      *
 *  DISPLAY AN ARRAY                                                                    *
 *                                                                                      *
 ****************************************************************************************/

static char *type_name[] = { "uint8", "uint16", "uint32", "uint64", "int8", "int16", "int32",
                             "int64", "float32", "float64" };

static int   Uindent;
static FILE *Uoutput;

static void Uhandler(Array *a)
{ int       i;
  Size_Type bytes;

  fprintf(Uoutput,"%*s%2d : %u",Uindent,"",Array_Refcount(a),a->dims[a->ndims-1]);
  for (i = a->ndims-2; i >= 0; i--)
    fprintf(Uoutput," x %u",a->dims[i]);
  fprintf(Uoutput," %s",type_name[a->type]);
  bytes = a->size * type_size[a->type];
  if (bytes < 1.e3)
    fprintf(Uoutput," = %llu",bytes);
  else if (bytes < 1.e6)
    fprintf(Uoutput," = %.1fKb",bytes/1.e3);
  else if (bytes < 1.e9)
    fprintf(Uoutput," = %.1fMb",bytes/1.e6);
  else 
    fprintf(Uoutput," = %.2fGb",bytes/1.e9);
  if (a->text != NULL && a->text[0] != '\0')
    fprintf(Uoutput," : '%.*s'",50,a->text);
  fprintf(Uoutput,"\n");
}

void Print_Inuse_List(FILE *output, int indent)
{ Uoutput = output;
  Uindent = indent;
  Array_List(Uhandler);
}

void Print_Array(Array_Or_Slice *o, FILE *output, int indent, char *format)
{ Indx_Type   i, j, e, f;
  Size_Type   area;
  Dimn_Type  *b, *c, *t;
  Slice      *s = (Slice *) o;
  Array      *a = (Array *) o;
  Coordinate *beg, *end;
  int         d0, d1, d2, od, kind, ndims;

  if (Is_Slice(o))
    { a     = Slice_Array(s);
      ndims = a->ndims;
      kind  = Slice_Kind(s);
      beg   = Slice_First(s);
      end   = Slice_Last(s);
      if (kind == RGB_KIND || kind == RGBA_KIND)
        { ADIMN(end)[ndims-1] = 0;
          e = Coord2IdxA(a,Inc_Array(end));
          ADIMN(end)[ndims-1] = kind_size[kind]-1;
        }
      else
        e = Coord2IdxA(a,Inc_Array(end));
    }
  else
    { ndims = a->ndims;
      kind  = a->kind;
      beg   = Idx2CoordA(a,0);
      if (kind != COMPLEX_KIND)
        end = Idx2CoordA(a,a->size/kind_size[kind]-1);
      else
        end = Idx2CoordA(a,a->size-1);
      s = Make_Slice(a,beg,end);
      e = Coord2IdxA(a,Inc_Array(end));
    }

  if (kind != PLAIN_KIND)
    ndims -= 1;
  d0 = (kind == COMPLEX_KIND);
  d1 = (ndims >= 2) + d0;
  d2 = d1+1;
  od = (ndims-1) + d0;
  area = a->size/kind_size[kind];

  b = ADIMN(beg);
  t = ADIMN(end);
  c = ADIMN(Slice_Coordinate(s));

  if (Is_Slice(o))
    { fprintf(output,"\n%*sSlice ",indent,"");
      if (ndims >= 2)
        fprintf(output,"[%u,%u] x ",b[d1],t[d1]);
      fprintf(output,"[%u,%u]\n",b[d0],t[d0]);
    }

  switch (kind) {
    #GENERATE K = @KINDS
      case <K>:
        switch (a->type) {
          #GENERATE T = @TYPES
            case <T>:
              { <t> *v = (<t> *) (a->data);
                <t> *w = v + area;             #WHEN K == RGB_KIND || K == RGBA_KIND
                <t> *x = w + area;             #WHEN K == RGB_KIND || K == RGBA_KIND
                <t> *y = x + area;             #WHEN K == RGBA_KIND
                       
                for (i = f = Set_Slice_To_First(s); 1; i = Next_Slice_Index(s))
                  { if (c[d1] == b[d1] && c[d0] == b[d0])
                      { if (i > f)
                          if (b[d1] == t[d1])
                            fprintf(output," }\n",indent,"");
                          else
                            fprintf(output,"\n%*s}\n",indent,"");
                        if (ndims > 2)
                          { fprintf(output,"\n%*s(",indent,"");
                            for (j = od; j > d2; j--)
                              fprintf(output,"%u,",c[j]);
                            fprintf(output,"%u)",c[j]);
                          }
                        fprintf(output,"\n%*s{ ",indent,"");
                      }
                    else if (c[d0] == b[d0])
                      fprintf(output,"\n%*s  ",indent,"");
                    else
                      fprintf(output,", ");
                    #IF K == PLAIN_KIND
                      fprintf(output,format,v[i]);
                    #ELSEIF K == RGB_KIND
                      fprintf(output,"[");
                      fprintf(output,format,v[i]);
                      fprintf(output,",");
                      fprintf(output,format,w[i]);
                      fprintf(output,",");
                      fprintf(output,format,x[i]);
                      fprintf(output,"]");
                    #ELSEIF K == RGBA_KIND
                      fprintf(output,"[");
                      fprintf(output,format,v[i]);
                      fprintf(output,",");
                      fprintf(output,format,w[i]);
                      fprintf(output,",");
                      fprintf(output,format,x[i]);
                      fprintf(output,",");
                      fprintf(output,format,y[i]);
                      fprintf(output,"]");
                    #ELSE
                      fprintf(output,format,v[i]);
                      fprintf(output," + ");
                      i = Next_Slice_Index(s);
                      fprintf(output,format,v[i]);
                      fprintf(output,"i");
                    #END
                    if (i == e) break;
                  }
                break;
              }
          #END
        }
        break;
    #END
  }

  if (b[d1] == t[d1])
    fprintf(output," }\n");
  else
    fprintf(output,"\n%*s}\n",indent,"");

  if (!Is_Slice(o))
    Kill_Slice(s);
}


/****************************************************************************************
 *                                                                                      *
 *  MODIFY TEXT DESCRIPTIONS                                                            *
 *                                                                                      *
 ****************************************************************************************/

void Set_Array_Text(Array *M(a), char *text)
{ int len = strlen(text)+1;
  allocate_array_text(a,len,"Set_Array_Text");
  a->tlen = len;
  strcpy(a->text,text);
}

void Append_To_Array_Text(Array *M(a), char *text)
{ int sen = strlen(a->text);
  allocate_array_text(a,sen+a->tlen,"Append_To_Array_Text");
  a->tlen += sen;
  strcpy(a->text+sen,text);
}


/****************************************************************************************
 *                                                                                      *
 *  SUB-PLANE SELECTION                                                                 *
 *                                                                                      *
 ****************************************************************************************/

static char Empty_String[1] = { 0 };

Array_Bundle *Get_Array_Plane(Array *a, Dimn_Type plane)
{ static Array_Bundle My_Array;

  Dimn_Type nplanes = a->dims[a->ndims-1];
  Size_Type offset  = array_dsize(a)/nplanes;

  if (plane >= nplanes || a->ndims <= 1 /* || plane < 0 */)
    return (NULL);
  My_Array.type   = a->type;
  My_Array.ndims  = a->ndims-1;
  My_Array.dims   = a->dims;
  My_Array.size   = a->size / nplanes;
  My_Array.tlen   = 1;
  My_Array.text   = Empty_String;
  My_Array.data   = a->data + plane*offset;

  My_Array.scale  = a->scale;
  if (a->kind == COMPLEX_KIND && a->ndims > 0)
    My_Array.kind   = COMPLEX_KIND;
  else
    My_Array.kind   = PLAIN_KIND;

  return (&My_Array);
}


/****************************************************************************************
 *                                                                                      *
 *  COMPUTE RANGES AND SCALE IMAGES                                                     *
 *                                                                                      *
 ****************************************************************************************/

//  Compute min and max values in 'a' of type 'type' with 'length' elements

static Range_Bundle *array_range(Array_Or_Slice *o)
{ static    Range_Bundle My_Range; 
  Indx_Type i, e;
  Slice    *s = (Slice *) o;
  Array    *a = (Array *) o;

  if (Is_Slice(o))
    a = Slice_Array(s);

  switch (a->type) {
    #GENERATE T,U = @TYPES, @UNION
      case <T>:
        { <t> *d = A<T>(a);
          <t>  x, min, max;
  
          if (Is_Slice(o))
            { e = Set_Slice_To_Last(s); 
              i = Set_Slice_To_First(s);
              min = max = d[i];
              for (i = Next_Slice_Index(s); 1; i = Next_Slice_Index(s)) 
                { x = d[i];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                  if (i == e) break;
                }
            }
          else
            { min = max = d[0];
              for (i = 0; i < a->size; i++)
                { x = d[i];
                  if (x < min)
                    min = x;
                  else if (x > max)
                    max = x;
                }
            }
          My_Range.maxval.<U> = max;
          My_Range.minval.<U> = min;
          break;
        }
    #END
  }

  return (&My_Range);
}

Range_Bundle *Array_Range(Array_Or_Slice *a)
{ static Range_Bundle My_Range;

  My_Range = *array_range(a);
  return (&My_Range);
}

Array_Or_Slice *Scale_Array(Array_Or_Slice *R(M(o)), double factor, double offset)
{ Indx_Type i, e;
  Slice    *s = (Slice *) o;
  Array    *a = (Array *) o;

  if (Is_Slice(o))
    a = Slice_Array(s);

  switch (a->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *d = A<T>(a);
          if (Is_Slice(o))
            { e = Set_Slice_To_Last(s); 
              for (i = Set_Slice_To_First(s); 1; i = Next_Slice_Index(s)) 
                { d[i] = factor * (d[i] + offset);
                  if (i == e) break;
                }
            }
          else
            for (i = 0; i < a->size; i++)
              d[i] = factor * (d[i] + offset);
          break;
        }
    #END
  }

  return (o);
}

Array_Or_Slice *Scale_Array_To_Range(Array_Or_Slice *R(M(o)), Value min, Value max)
{ Range_Bundle crn;
  double       f;
  Slice       *s = (Slice *) o;
  Array       *a = (Array *) o;

  if (a->kind > COMPLEX_KIND)
    a = Slice_Array(s);

  crn = *array_range(o);
  switch (a->type) {
    #GENERATE T,U = @TYPES, @UNION
      case <T>:
        if (crn.maxval.<U> == crn.minval.<U>)
          return (a);
        if (min.<U> == max.<U>)
          return (a);
        f  = max.<U>-min.<U>;
        f /= crn.maxval.<U>-crn.minval.<U>;
        return (Scale_Array(o,f,min.<U>/f-crn.minval.<U>));
    #END
  }
}


/****************************************************************************************
 *                                                                                      *
 *  ARRAY SCALAR AND EL-BY-EL OPERATORS                                                 *
 *                                                                                      *
 ****************************************************************************************/

Array_Or_Slice *Array_Op_Scalar(Array_Or_Slice *R(M(o)), Operator op, Value_Type type, Value value)
{ Indx_Type i, e;
  Slice    *s = (Slice *) o;
  Array    *a = (Array *) o;
  uint64    uval;
  int64     ival;
  double    rval;
  Size_Type area;
  int       alice;

  if (alice = Is_Slice(o))
    a = Slice_Array(s);

  if (op == LSH_OP || op == RSH_OP)
    { if (a->type >= FLOAT32)
        { fprintf(stderr,"Shifting floating point values is not permitted (Array_Op_Scalar)\n");
          exit (1);
        }
    }

  if (type == UVAL)
    uval = value.uval;
  else if (type == IVAL)
    ival = value.ival;
  else
    rval = value.rval;
  
  area = a->size;
  if (alice)
    e = Set_Slice_To_Last(s);

  switch (a->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *d = A<T>(a);
          switch (type) {
            #GENERATE V,U = UVAL IVAL RVAL, uval ival rval
              case <V>:
                switch (op) {
                  #GENERATE OP,SYM = @OPS,@SYMS
                    case <OP>:
                      if (alice)
                        for (i = Set_Slice_To_First(s); 1; i = Next_Slice_Index(s))
                          #IF OP <= SUB_OP
                            { d[i] <SYM> <U>;
                          #ELSEIF OP <= DIV_OP
                            { d[i] <SYM> <U>;
                          #ELSEIF OP <= POW_OP
                            { d[i] = pow((double) d[i],(double) <U>);
                          #ELSEIF OP <= RSH_OP
                           #IF T < FLOAT32
                            { d[i] <SYM> ((int) <U>);
                           #ELSE
                            { ;
                           #END
                          #ELSE
                            { if (d[i] <SYM> <U>) d[i] = <U>;
                          #END
                              if (i == e) break;
                            }
                      else
                        for (i = 0; i < area; i++)
                          #IF OP <= SUB_OP
                            d[i] <SYM> <U>;
                          #ELSEIF OP <= DIV_OP
                            d[i] <SYM> <U>;
                          #ELSEIF OP <= POW_OP
                            d[i] = pow((double) d[i],(double) <U>);
                          #ELSEIF OP <= RSH_OP
                           #IF T < FLOAT32
                            d[i] <SYM> ((int) <U>);
                           #ELSE
                            ;
                           #END
                          #ELSE
                            if (d[i] <SYM> <U>) d[i] = <U>;
                          #END
                      break;
                  #END
                }
                break;
            #END
          }
          break;
        }
    #END
  }

  return (o);
}

Array_Or_Slice *Complex_Op_Scalar(Array_Or_Slice *R(M(o)), Operator op,
                                  Value_Type type, Value rpart, Value ipart)
{ Indx_Type i, e;
  Slice    *s = (Slice *) o;
  Array    *a = (Array *) o;
  int       kind, alice;
  Indx_Type area;
  double    mag, ang;
  double    mgr, agr;
  

  if (alice = Is_Slice(o))
    { a    = Slice_Array(s);
      kind = Slice_Kind(s);
    }
  else
    kind = a->kind;

  if (kind != COMPLEX_KIND)
    { fprintf(stderr,"Array a must be of COMPLEX kind (Complex_Op_Scalar)\n");
      exit (1);
    }

  if (op == LSH_OP || op == RSH_OP)
    { if (a->type >= FLOAT32)
        { fprintf(stderr,"Shifting floating point values is not permitted (Array_Op_Scalar)\n");
          exit (1);
        }
    }
  
  area = a->size;
  if (alice)
    e = Set_Slice_To_Last(s);

  switch (a->type) {
  #GENERATE T = @TYPES
    case <T>:
      { <t> *d = A<T>(a);
        <t> *D = d+1;
        <t>  dr, di;
        switch (type) {
        #GENERATE V,U = UVAL IVAL RVAL, uint64 int64 float64
          case <V>:
            { <u> rp = rpart.<v>;
              <u> ip = ipart.<v>;
              <u> em = rp*rp + ip*ip;
              switch (op) {
              #GENERATE OP,SYM = @OPS,@SYMS
                case <OP>:
                #GENERATE S = 0 1
                #IF S == 0
                  if (alice)
                    for (i = Set_Slice_To_First(s); 1; i = Next_Slice_Index(s))
                #ELSE
                  else
                    for (i = 0; i < area; i += 2)
                #END
                #IF OP <= SUB_OP
                      { d[i] <SYM> rp;
                        D[i] <SYM> ip;
                #ELSEIF OP <= POW_OP
                      { dr = d[i];
                        di = D[i];
                  #IF OP == MUL_OP
                        d[i] = dr*rp - di*ip;
                        D[i] = di*rp + dr*ip;
                  #ELSEIF OP == DIV_OP
                        d[i] = (dr*rp + di*ip) / em;
                        D[i] = (di*rp - dr*ip) / em;
                  #ELSE
                        mag  = di*di + dr*dr;
                        ang  = atan2((double) di,(double) dr);
                        mgr = pow(mag,.5*rp) * exp(-ip*ang);
                        agr = .5*ip*log(mag) + rp*ang;
                        d[i] = mgr * cos(agr);
                        D[i] = mgr * sin(agr);
                  #END
                #ELSEIF OP <= RSH_OP
                  #IF T < FLOAT32
                      { d[i] <SYM> ((int) rp);
                        D[i] <SYM> ((int) ip);
                  #ELSE
                      { ;
                  #END
                #ELSE
                      { if (d[i] <SYM> rp) d[i] = rp;
                        if (D[i] <SYM> ip) D[i] = ip;
                #END
                #IF S == 0
                        i = Next_Slice_Index(s);
                        if (i == e) break;
                #END
                      }
                #END
                  break;
              #END
                }
              break;
            }
        #END
          }
        break;
      }
  #END
    }

  return (o);
}

#define AOA_BUFLEN 0x4000000

static double AA_Buffer[AOA_BUFLEN];

Array_Or_Slice *Array_Op_Array(Array_Or_Slice *R(M(o)), Operator op, Array_Or_Slice *q)
{ Slice *s = (Slice *) o;
  Slice *t = (Slice *) q;
  Array *a = (Array *) o;
  Array *b = (Array *) q;

  int         alice, blice;
  int         akind, bkind;
  Size_Type   area,  brea, crea;
  Indx_Type   i, j, k;
  Size_Type   volume;
  int         btype;

  if (alice = Is_Slice(o))
    { a = Slice_Array(s);
      akind = Slice_Kind(s);
    }
  else
    akind = a->kind;

  if (blice = Is_Slice(q))
    { b = Slice_Array(t);
      bkind = Slice_Kind(t);
    }
  else
    bkind = b->kind;

  if ((op == LSH_OP || op == RSH_OP) && a->type >= FLOAT32)
    { fprintf(stderr,"Shifting floating point values is not permitted (Array_Op_Array)\n");
      exit (1);
    }

  area = a->size;
  brea = b->size;
  if (op == SET_OP && a->type == b->type && !alice && !blice)
    { Size_Type as = array_size(a);
      Size_Type bs = array_size(b);
      if (bs >= as)
        memcpy(a->data,b->data,as);
      else
        { for (i = 0; i < as; i += bs)
            if (i+bs > as)
              memcpy(a->data+i,b->data,as-i);
            else
              memcpy(a->data+i,b->data,bs);
        }
      return (o);
    }

  if (b->type <= UINT64)
    btype = UVAL;
  else if (b->type <= INT64)
    btype = IVAL;
  else
    btype = RVAL;

  if (blice)
    j = Set_Slice_To_First(t);
  else
    j = 0;
  if (alice)
    k = Set_Slice_To_First(s);
  else
    k = 0;

  if (alice)
    volume = Slice_Volume(s);
  else
    volume = area;

  switch (btype) {
  #GENERATE R,D = UVAL IVAL RVAL, uint64 int64 float64
    case <R>:
      { <D> *buffer = (<D> *) AA_Buffer;
        while (volume != 0)
          { if (volume < AOA_BUFLEN)
              crea = volume;
            else
              crea = AOA_BUFLEN;
            switch (b->type) {
            #GENERATE T,Q = @TYPES, 1 1 1 1 2 2 2 2 3 3
             #IF Q <= R
              case <T>:
                { <t> *e = A<T>(b);
                  if (blice)
                    for (i = 0; i < crea; i++)
                      { buffer[i] = e[j];
                        j = Next_Slice_Index(t);
                      }
                  else
                    for (i = 0; i < crea; i++)
                      { buffer[i] = e[j++];
                        if (j == brea) j = 0;
                      }
                  break;
                }
             #END
            #END
              }
            switch (a->type) {
            #GENERATE T = @TYPES
              case <T>:
                { <t> *d = A<T>(a);
                  switch (op) {
                  #GENERATE OP,SYM = @OPS,@SYMS
                    case <OP>:
                    #GENERATE S = 0 1
                    #IF S == 0
                      if (alice)
                        for (i = 0; i < crea; i++, k = Next_Slice_Index(s))
                    #ELSE
                      else
                        for (i = 0; i < crea; i++, k++)
                    #END
                        #IF OP == POW_OP
                          d[k] = pow((double) d[k],(double) buffer[i]);
                        #ELSEIF OP >= MIN_OP
                          if (d[k] <SYM> buffer[i]) d[k] = buffer[i];
                        #ELSEIF OP < POW_OP
                          d[k] <SYM> buffer[i];
                        #ELSEIF T < FLOAT32
                          d[k] <SYM> (int) buffer[i];
                        #ELSE
                          ;
                        #END
                    #END
                      break;
                  #END
                    }
                  break;
                }
            #END
              }
            volume -= crea;
          }
        break;
      }
  #END
    }

  return (o);
}

Array_Or_Slice *Complex_Op_Array(Array_Or_Slice *R(M(o)), Operator op, Array_Or_Slice *q)
{ Slice *s = (Slice *) o;
  Slice *t = (Slice *) q;
  Array *a = (Array *) o;
  Array *b = (Array *) q;

  int         alice, blice;
  int         akind, bkind;
  Size_Type   area,  brea, crea;
  Indx_Type   i, j, k;
  Size_Type   volume;
  int         btype;
  double      mag, ang;
  double      mgr, agr;

  if (alice = Is_Slice(o))
    { a = Slice_Array(s);
      akind = Slice_Kind(s);
    }
  else
    akind = a->kind;

  if (blice = Is_Slice(q))
    { b = Slice_Array(t);
      bkind = Slice_Kind(t);
    }
  else
    bkind = b->kind;

  if (akind != COMPLEX_KIND)
    { fprintf(stderr,"First array must be complex (Complex_Op_Array)\n");
      exit (1);
    }
  if ((op == LSH_OP || op == RSH_OP) && a->type >= FLOAT32)
    { fprintf(stderr,"Shifting floating point values is not permitted (Array_Op_Array)\n");
      exit (1);
    }

  if (b->type <= UINT64)
    btype = UVAL;
  else if (b->type <= INT64)
    btype = IVAL;
  else
    btype = RVAL;

  if (blice)
    j = Set_Slice_To_First(t);
  else
    j = 0;
  if (alice)
    k = Set_Slice_To_First(s);
  else
    k = 0;

  area = a->size;
  brea = b->size;
  if (alice)
    volume = Slice_Volume(s);
  else
    volume = area;

  switch (btype) {
  #GENERATE R,D = UVAL IVAL RVAL, uint64 int64 float64
    case <R>:
      { <D> *buffer = (<D> *) AA_Buffer;
        <D> er;
        while (volume != 0)
          { if (volume < AOA_BUFLEN)
              crea = volume/2;
            else
              crea = AOA_BUFLEN/2;
            switch (b->type) {
            #GENERATE T,Q = @TYPES, 1 1 1 1 2 2 2 2 3 3
             #IF Q <= R
              case <T>:
                { <t> *e = A<T>(b);
                  if (blice)
                    for (i = 0; i < crea; i++)
                      { buffer[i] = e[j];
                        j = Next_Slice_Index(t);
                      }
                  else
                    for (i = 0; i < crea; i++)
                      { buffer[i] = e[j++];
                        if (j == brea) j = 0;
                      }
                  break;
                }
             #END
            #END
              }
            switch (a->type) {
            #GENERATE T = @TYPES
              case <T>:
                { <t> *d = A<T>(a);
                  <t> *D = d+1;
                  <t>  dr, di;
                  switch (op) {
                  #GENERATE OP,SYM = @OPS,@SYMS
                    case <OP>:
                    #GENERATE S = 0 1
                    #IF S == 0
                      if (alice)
                        for (i = 0; i < crea; i++, Next_Slice_Index(s), k = Next_Slice_Index(s))
                    #ELSE
                      else
                        for (i = 0; i < crea; i++, k += 2)
                    #END
                          { er = buffer[i];
                          #IF OP <= MUL_OP
                            d[k] <SYM> er;
                            D[k] <SYM> er;
                          #ELSEIF OP == POW_OP
                            dr = d[k];
                            di = D[k];
                            mag  = di*di + dr*dr;
                            ang  = atan2((double) di,(double) dr);
                            mgr  = pow(mag,.5*er);
                            agr  = er*ang;
                            d[k] = mgr * cos(agr);
                            D[k] = mgr * sin(agr);
                          #ELSEIF OP <= RSH_OP
                           #IF T < FLOAT32
                            d[k] <SYM> ((int) er);
                            D[k] <SYM> ((int) er);
                           #END
                          #ELSE
                            if (d[k] <SYM> er) d[k] = er;
                            if (D[k] <SYM> er) D[k] = er;
                          #END
                          }
                    #END
                      break;
                  #END
                    }
                  break;
                }
            #END
              }
            volume -= 2*crea;
          }
        break;
      }
  #END
    }

  return (o);
}

Array_Or_Slice *Complex_Op_Complex(Array_Or_Slice *R(M(o)), Operator op, Array_Or_Slice *q)
{ Slice *s = (Slice *) o;
  Slice *t = (Slice *) q;
  Array *a = (Array *) o;
  Array *b = (Array *) q;

  int         alice, blice;
  int         akind, bkind;
  Size_Type   area,  brea, crea;
  Indx_Type   i, j, k;
  Size_Type   volume;
  int         btype;
  double      mag, ang;
  double      mgr, agr;

  if (alice = Is_Slice(o))
    { a = Slice_Array(s);
      akind = Slice_Kind(s);
    }
  else
    akind = a->kind;

  if (blice = Is_Slice(q))
    { b = Slice_Array(t);
      bkind = Slice_Kind(t);
    }
  else
    bkind = b->kind;

  if (akind != COMPLEX_KIND || bkind != COMPLEX_KIND)
    { fprintf(stderr,"Arrays must be complex (Complex_Op_Complex)\n");
      exit (1);
    }
  if ((op == LSH_OP || op == RSH_OP) && a->type >= FLOAT32)
    { fprintf(stderr,"Shifting floating point values is not permitted (Array_Op_Array)\n");
      exit (1);
    }

  if (b->type <= UINT64)
    btype = UVAL;
  else if (b->type <= INT64)
    btype = IVAL;
  else
    btype = RVAL;

  if (blice)
    j = Set_Slice_To_First(t);
  else
    j = 0;
  if (alice)
    k = Set_Slice_To_First(s);
  else
    k = 0;

  area = a->size;
  brea = b->size;
  if (alice)
    volume = Slice_Volume(s);
  else
    volume = area;

  switch (btype) {
  #GENERATE R,D = UVAL IVAL RVAL, uint64 int64 float64
    case <R>:
      { <D> *buffer = (<D> *) AA_Buffer;
        <D> er, ei, em;
        while (volume != 0)
          { if (volume < AOA_BUFLEN)
              crea = volume;
            else
              crea = AOA_BUFLEN;
            switch (b->type) {
            #GENERATE T,Q = @TYPES, 1 1 1 1 2 2 2 2 3 3
             #IF Q <= R
              case <T>:
                { <t> *e = A<T>(b);
                  if (blice)
                    for (i = 0; i < crea; i++)
                      { buffer[i] = e[j];
                        j = Next_Slice_Index(t);
                      }
                  else
                    for (i = 0; i < crea; i++)
                      { buffer[i] = e[j++];
                        if (j == brea) j = 0;
                      }
                  break;
                }
             #END
            #END
              }
            switch (a->type) {
            #GENERATE T = @TYPES
              case <T>:
                { <t> *d = A<T>(a);
                  <t> *D = d+1;
                  <t>  dr, di;
                  switch (op) {
                  #GENERATE OP,SYM = @OPS,@SYMS
                    case <OP>:
                    #GENERATE S = 0 1
                    #IF S == 0
                      if (alice)
                        for (i = 0; i < crea; i += 2, Next_Slice_Index(s), k = Next_Slice_Index(s))
                    #ELSE
                      else
                        for (i = 0; i < crea; i += 2, k += 2)
                    #END
                      #IF OP <= SUB_OP
                          { d[k] <SYM> buffer[i];
                            D[k] <SYM> buffer[i+1];
                      #ELSEIF OP <= POW_OP
                          { dr = d[k];
                            di = D[k];
                            er = buffer[i];
                            ei = buffer[i];
                        #IF OP == MUL_OP
                            d[k] = dr*er - di*ei;
                            D[k] = di*er + dr*ei;
                        #ELSEIF OP == DIV_OP
                            em   = er*er + ei*ei;
                            d[k] = (dr*er + di*ei) / em;
                            D[k] = (di*er - dr*ei) / em;
                        #ELSE
                            mag  = di*di + dr*dr;
                            ang  = atan2((double) di,(double) dr);
                            mgr = pow(mag,.5*er) * exp(-ei*ang);
                            agr = .5*ei*log(mag) + er*ang;
                            d[k] = mgr * cos(agr);
                            D[k] = mgr * sin(agr);
                        #END
                      #ELSEIF OP <= RSH_OP
                        #IF T < FLOAT32
                          { d[k] <SYM> ((int) buffer[i]);
                            D[k] <SYM> ((int) buffer[i+1]);
                        #ELSE
                          { ;
                        #END
                      #ELSE
                          { if (d[k] <SYM> buffer[i]) d[k] = buffer[i];
                            if (D[k] <SYM> buffer[i+1]) D[k] = buffer[i+1];
                      #END
                          }
                    #END
                      break;
                  #END
                    }
                  break;
                }
            #END
              }
            volume -= crea;
          }
        break;
      }
  #END
    }

  return (o);
}

/****************************************************************************************
 *                                                                                      *
 *  APPLYING FUNCTIONS TO ARRAY ELEMENTS                                                *
 *                                                                                      *
 ****************************************************************************************/

Array_Or_Slice *Array_Fct_Val(Array_Or_Slice *R(M(o)), Value (*fct)(Value val))
{ Indx_Type i, e;
  Slice    *s = (Slice *) o;
  Array    *a = (Array *) o;

  if (Is_Slice(o))
    a = Slice_Array(s);

  switch (a->type) {
    #GENERATE T,C,U = @TYPES, @CASTES, @UNION
      case <T>:
        { <t> *d = A<T>(a);
          if (Is_Slice(o))
            { e = Set_Slice_To_Last(s); 
              for (i = Set_Slice_To_First(s); 1; i = Next_Slice_Index(s)) 
                { d[i] = fct((Value) (<C>) d[i]).<U>;
                  if (i == e) break;
                }
            }
          else
            for (i = 0; i < a->size; i++)
              d[i] = fct((Value) (<C>) d[i]).<U>;
          break;
        }
    #END
  }

  return (o);
}

Array_Or_Slice *Array_Fct_Idx(Array_Or_Slice *R(M(o)), Value (*fct)(Coordinate *coord))
{ int       n, c;
  Indx_Type p, e;
  Slice    *s = (Slice *) o;
  Array    *a = (Array *) o;
  int       alice;

  if (alice = Is_Slice(o))
    a = Slice_Array(s);
  else
    s = Make_Slice(a,Idx2CoordA(a,0),Idx2CoordA(a,a->size-1));

  switch (a->type) {
    #GENERATE T,U = @TYPES, @UNION
      case <T>:
        { <t> *d = A<T>(a);
          e = Set_Slice_To_Last(s); 
          for (p = Set_Slice_To_First(s); 1; p = Next_Slice_Index(s)) 
            { d[p] = fct(Slice_Coordinate(s)).<U>;
              if (p == e) break;
            }
          break;
        }
    #END
  }

  if (!alice)
    Kill_Slice(s);

  return (o);
}

//  Threshold values less than cutoff to black, all others to white

Array_Or_Slice *Threshold_Array(Array_Or_Slice *R(M(o)), Value cutoff)
{ Indx_Type i, e;
  Slice    *s = (Slice *) o;
  Array    *a = (Array *) o;

  if (Is_Slice(o))
    a = Slice_Array(s);

  switch (a->type) {
    #GENERATE T,U = @TYPES, @UNION
      case <T>:
        { <t> *d = A<T>(a);
          <t>  x, c = cutoff.<U>;
          <t>  zero = 0, one = 1;
					
          if (Is_Slice(o))
            { e = Set_Slice_To_Last(s); 
              for (i = Set_Slice_To_First(s); 1; i = Next_Slice_Index(s)) 
                { x = d[i];
                  if (x <= c)
                    d[i] = zero;
                  else
                    d[i] = one;
                  if (i == e) break;
                }
            }
          else
            { for (i = 0; i < a->size; i++)
                { x = d[i];
                  if (x <= c)
                    d[i] = zero;
                  else
                    d[i] = one;
                }
            }
          break;
        }
    #END
  }

  return (o);
}


/****************************************************************************************
 *                                                                                      *
 *  CONVERT IMAGE TYPES                                                                 *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { Array_Type type;
    int        scale;
    void      *data;
  } Domain;

#define LIKE_2_LIKE  0   // conversion codes
#define CPLX_2_NORM  1
#define  RGB_2_NORM  2
#define RGBA_2_NORM  3
#define RGBA_2_RGB   4

static void translate(int conversion, Size_Type size, Domain *source, Domain *target, double dscale)
{ Array_Type stype,  ttype;
  void      *sdata, *tdata;
  int        sscale, tscale;
  int        shift;

  stype  = source->type;
  sdata  = source->data;
  sscale = source->scale;

  ttype  = target->type;
  tdata  = target->data;
  tscale = target->scale;

  shift = abs(sscale-tscale);

  if (conversion)
    { Indx_Type p, q;
      double    x, y, z;
      int       ctype;
      double    afactor;

      if (type_size[stype] < type_size[ttype])
        ctype = stype;
      else
        ctype = ttype;

      afactor = 1.;
      if (conversion >= RGBA_2_NORM && stype < FLOAT32)
        { uint64 base = (((uint64) 1) << (sscale-1));
          afactor /= ((base-1) + base);
        }
      if (type_size[stype] >= type_size[ttype])
        { if (sscale > tscale)
            afactor /= (1 << shift);
          else if (tscale > sscale)
            afactor *= (1 << shift);
          else
            afactor *= dscale;
        }

      switch (conversion) {
        #GENERATE C = CPLX_2_NORM RGB_2_NORM RGBA_2_NORM RGBA_2_RGB
          case <C>:
            switch (stype) {
              #GENERATE S = @TYPES
                case <S>:
                  { <s> *s0 = (<s> *) sdata;
                    <s> *s1 = s0 + size;        #WHEN C != CPLX_2_NORM
                    <s> *s2 = s1 + size;        #WHEN C != CPLX_2_NORM
                    <s> *s3 = s2 + size;        #WHEN C >= RGBA_2_NORM
  
                    switch (ctype) {
                      #GENERATE T = @TYPES
                        case <T>:
                          { <t> *t0  = (<t> *) tdata;
                            <t> *t1  = t0 + size;       #WHEN C == RGBA_2_RGB
                            <t> *t2  = t1 + size;       #WHEN C == RGBA_2_RGB

                            #IF C == RGBA_2_RGB
                              for (p = 0; p < size; p++)
                                t0[p] = s0[p] * (afactor * s3[p]);          #WHEN T >= FLOAT32
                                t0[p] = s0[p] * (afactor * s3[p]) + .4999;  #WHEN T <  FLOAT32
                              for (p = 0; p < size; p++)
                                t1[p] = s1[p] * (afactor * s3[p]);          #WHEN T >= FLOAT32
                                t1[p] = s1[p] * (afactor * s3[p]) + .4999;  #WHEN T <  FLOAT32
                              for (p = 0; p < size; p++)
                                t2[p] = s2[p] * (afactor * s3[p]);          #WHEN T <= FLOAT32
                                t2[p] = s2[p] * (afactor * s3[p]) + .4999;  #WHEN T <  FLOAT32
                            #ELSE
                              for (q = p = 0; p < size; p++) {
                                #IF C == RGB_2_NORM
                                  t0[p] = (.30*s0[p] + .59*s1[p] + .11*s2[p])
                                        * afactor;                              #WHEN T >= FLOAT32
                                        * afactor + .5;                         #WHEN T <  FLOAT32
                                #ELSEIF C == RGBA_2_NORM
                                  t0[p] = ((.30*s0[p] + .59*s1[p] + .11*s2[p])
                                        * (afactor * s3[p]));                   #WHEN T >= FLOAT32
                                        * (afactor * s3[p])) + .5;              #WHEN T <  FLOAT32
                                #ELSE
                                  x = s0[q++];
                                  y = s0[q++];
                                  t0[p] = sqrt(x*x+y*y) * afactor;
                                #END
                              }
                            #END
                            break;
                          }
                      #END
                    }
                    break;
                  }
              #END
            }
            break;
        #END
      }

      sdata = tdata;
      if (type_size[stype] >= type_size[ttype])
        stype = ttype;
      if (conversion == RGBA_2_RGB)
        size *= 3;
  }

  if (ttype != stype || sdata != tdata)
    { Indx_Type p;

      if (sscale > tscale || dscale != 1.)
        switch (stype) {
          #GENERATE S,SS = @TYPES,@SIZES
            case <S>:
              { <s> *s = (<s> *) sdata;
                switch (ttype) {
                  #GENERATE T,ST = @TYPES,@SIZES
                    case <T>:
                      { <t> *t  = (<t> *) tdata;
                        for (p = 0; p < size; p++)              #WHEN ST <= SS
                        for (p = size; p-- > 0; )               #WHEN ST >  SS
                          t[p] = s[p] >> shift;                 #WHEN T <  FLOAT32 && S <  FLOAT32
                          t[p] = s[p] * dscale;                 #WHEN T >= FLOAT32 || S >= FLOAT32
                        break;
                      }
                  #END
                }
                break;
              }
          #END
        }

      else if (sscale < tscale)
        switch (stype) {
          #GENERATE S,SS = @TYPES,@SIZES
            case <S>:
              { <s> *s = (<s> *) sdata;
                switch (ttype) {
                  #GENERATE T,ST = @TYPES,@SIZES
                    case <T>:
                      { <t> *t  = (<t> *) tdata;
                        for (p = 0; p < size; p++)              #WHEN ST <= SS
                        for (p = size; p-- > 0; )               #WHEN ST >  SS
                          t[p] = s[p] << shift;                 #WHEN T <  FLOAT32 && S <  FLOAT32
                          t[p] = s[p] * dscale;                 #WHEN T >= FLOAT32 || S >= FLOAT32
                        break;
                      }
                  #END
                }
                break;
              }
          #END
        }

      else
        switch (stype) {
          #GENERATE S,SS = @TYPES,@SIZES
            case <S>:
              { <s> *s = (<s> *) sdata;
                switch (ttype) {
                  #GENERATE T,ST = @TYPES,@SIZES
                    case <T>:
                      { <t> *t  = (<t> *) tdata;
                        for (p = 0; p < size; p++)                      #WHEN ST <= SS
                        for (p = size; p-- > 0; )                       #WHEN ST >  SS
                          t[p] = s[p];
                        break;
                      }
                  #END
                }
                break;
              }
          #END
        }
    }
}

static void imaginary_fill(Size_Type size, Domain *target)
{ Indx_Type p, q;

  switch (target->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *d = A<T>(target);
          q = size;
          for (p = 2*size; p-- > 0; )
            { d[p--] = 0;
              d[p]   = d[--q];
            }
          break;
        }
    #END
  }
}

static void alpha_fill(Size_Type size, Domain *target, double alpha)
{ Indx_Type p;

  switch (target->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *d = A<T>(target) + 3*size;
          for (p = 0; p < size; p++)
            d[p] = alpha;
          break;
        }
    #END
  }
}

typedef struct
  { int    tscale;
    double dscale;
  } Variable_Scale;

static Array *convert_array(Array *sarray, Array_Kind tkind, Array_Type ttype,
                            Variable_Scale *var, int in_place)
{ Domain     source, target;
  Array     *tarray;

  char      *estring;
  int        tdims;
  Size_Type  tels, pels;

  Array_Kind skind  = sarray->kind;
  Array_Type stype  = sarray->type;
  int        sscale = sarray->scale;

  int        tscale;
  double     dscale;

  if (stype < FLOAT32 && ttype < FLOAT32)
    { tscale = var->tscale;
      dscale = 1.;
    }
  else
    { dscale = var->dscale;
      tscale = sarray->scale;
    }

  pels   = sarray->size / kind_size[skind];
  tdims  = (sarray->ndims + (tkind != PLAIN_KIND)) - (skind != PLAIN_KIND);
  tels   = pels * kind_size[tkind];

  if (in_place)
    { estring = "Convert_Array_Inplace";
      tarray  = sarray;
    }
  else
    { estring = "Convert_Array_Copy";
      tarray  = new_array(0,0,1,estring);
      tarray->tlen    = 1;
      tarray->text[0] = '\0';
    }

  if (stype < FLOAT32 && ttype < FLOAT32)
    { if (tscale > bit_size[ttype])
        { fprintf(stderr,"target scale is larger than number of bits in type (%s)\n",estring);
          exit (1);
        }
    }
  else
    { if (fabs(dscale) < 1e-20)
        { fprintf(stderr,"scale factor is nearly 0, i.e. < 1e-20 (%s)\n",estring);
          exit (1);
        }
    }

  allocate_array_data(tarray,tels*type_size[ttype],estring);
  allocate_array_dims(tarray,sizeof(Dimn_Type)*tdims,estring);

  if (INT8 <= stype && stype <= INT64)
    sscale -= 1;
  if (INT8 <= ttype && ttype <= INT64)
    tscale -= 1;

  source.type  = stype;
  source.data  = sarray->data;
  source.scale = sscale;

  target.type  = ttype;
  target.data  = tarray->data;
  if (stype < FLOAT32 && ttype < FLOAT32)
    target.scale = tscale;
  else
    target.scale = sscale;

  if (tkind == skind)
    translate(LIKE_2_LIKE,kind_size[tkind]*pels,&source,&target,dscale);
  else if (tkind == PLAIN_KIND || tkind == COMPLEX_KIND)
    { if (skind == RGB_KIND)
        translate(RGB_2_NORM,pels,&source,&target,dscale);
      else if (skind == RGBA_KIND)
        translate(RGBA_2_NORM,pels,&source,&target,dscale);
      else if (skind == COMPLEX_KIND)
        translate(CPLX_2_NORM,pels,&source,&target,dscale);
      else
        translate(LIKE_2_LIKE,kind_size[tkind]*pels,&source,&target,dscale);
      if (tkind == COMPLEX_KIND)
        imaginary_fill(pels,&target);
    }
  else //  tkind == RGB_KIND || tkind == RGBA_KIND
    { if (skind == COMPLEX_KIND)
        translate(CPLX_2_NORM,pels,&source,&target,dscale);
      else if (skind == RGBA_KIND)
        translate(RGBA_2_RGB,pels,&source,&target,dscale);
      else
        translate(LIKE_2_LIKE,sarray->size,&source,&target,dscale);
      if (skind == PLAIN_KIND || skind == COMPLEX_KIND)
        { int block = type_size[ttype]*pels;
          memcpy(tarray->data + block,sarray->data,block);
          memcpy(tarray->data + 2*block,sarray->data,block);
        }
      if (tkind == RGBA_KIND)
        { double alpha;

          if (stype <= UINT64)
            { uint64 base = (((uint64) 1) << (sscale-1));
              alpha = (base-1) + base;
            }
          else if (stype <= INT64)
            alpha = ((((uint64) 1) << (sscale-1)) - 1);
          else
            alpha = 1.;
          if (stype < FLOAT32 && ttype < FLOAT32)
            if (sscale > tscale)
              alpha /= (((uint64) 1)<<(sscale-tscale));
            else
              alpha *= (((uint64) 1)<<(tscale-sscale));
          else
            alpha *= dscale;
          alpha_fill(pels,&target,alpha);
        }
    }

  { int i, cs, ct, nd;

    cs = (skind == COMPLEX_KIND);
    ct = (tkind == COMPLEX_KIND);
    nd = tdims - (tkind != PLAIN_KIND);
    if (ct > cs)
      { for (i = nd-1; i >= 0; i--)
          tarray->dims[i+ct] = sarray->dims[i+cs];
      }
    else
      { for (i = 0; i < nd; i++)
          tarray->dims[i+ct] = sarray->dims[i+cs];
      }
    if (ct)
      tarray->dims[0] = 2;
    if (!ct && nd != tdims)
      tarray->dims[nd] = kind_size[tkind];
  }

  tarray->size  = tels;
  tarray->kind  = tkind;
  tarray->type  = ttype;
  tarray->ndims = tdims;
  if (ttype < FLOAT32 && stype < FLOAT32)
    { if (ttype <= UINT64)
        tarray->scale = tscale;
      else
        tarray->scale = tscale+1;
    }
  else
    tarray->scale = bit_size[ttype];

  return (tarray);
}

Array *Convert_Array_Inplace(Array *R(M(sarray)), Array_Kind tkind, Array_Type ttype, ...)
{ Variable_Scale var;
  va_list        ap;

  if (sarray->kind == tkind && sarray->type == ttype)
    return (sarray);

  va_start(ap,ttype);
  if (sarray->type < FLOAT32 && ttype < FLOAT32)
    var.tscale = va_arg(ap,int);
  else
    var.dscale = va_arg(ap,double);
  va_end(ap);

  return (convert_array(sarray,tkind,ttype,&var,1));
}

Array *G(Convert_Array_Copy)(Array *sarray, Array_Kind tkind, Array_Type ttype, ...)
{ Variable_Scale var;
  va_list        ap;

  if (sarray->kind == tkind && sarray->type == ttype)
    return (Copy_Array(sarray));

  va_start(ap,ttype);
  if (sarray->type < FLOAT32 && ttype < FLOAT32)
    var.tscale = va_arg(ap,int);
  else
    var.dscale = va_arg(ap,double);
  va_end(ap);

  return (convert_array(sarray,tkind,ttype,&var,0));
}

int Image_Check(Array *array)
{ Indx_Type p, vmax;

  if (array->kind == COMPLEX_KIND)
    { fprintf(stderr,"Array cannot be COMPLEX (Image_Check)\n");
      return (1);
    }

  if (array->type <= UINT64)
    { uint64 base = (((uint64) 1) << (array->scale-1));
      vmax = (base-1) + base;
    }
  else if (array->type <= INT64)
    vmax = ((((uint64) 1) << (array->scale-1)) - 1);

  switch (array->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *v = A<T>(array);
          for (p = 0; p < array->size; p++)
            #IF T >= FLOAT32
              { if (v[p] > 1.)
                  return (1);
            #ELSE
              { if (v[p] > vmax)
                  return (1);
            #END
            #IF T >= INT8
                if (v[p] < 0)
                  return (1);
            #END
              }
          break;
        }
    #END
  }

  return (0);
}

Array *convert_image(Array *array, Array_Kind kind, Array_Type type, int scale, int in_place)
{ Variable_Scale var;
  Array         *rez;
  int            atype;
  double         fact;

  var.dscale = 1.;
  var.tscale = scale;
  atype = array->type;
  if (atype != type)
    if (type >= FLOAT32)
      { if (array->type <= UINT64)
          { uint64 base = (((uint64) 1) << (array->scale-1));
            var.dscale = 1. / ((base-1) + base);
          }
        else
          var.dscale = 1. / ((((uint64) 1) << (array->scale-1)) - 1);
      }
    else
      { if (type <= UINT64)
          { uint64 base = (((uint64) 1) << (scale-1));
            var.dscale = (base-1) + base;
          }
        else
          var.dscale = ((((uint64) 1) << (scale-1)) - 1);
      }

  rez = convert_array(array,kind,type,&var,in_place);

  if (atype >= FLOAT32 && type < FLOAT32)
    rez->scale = scale;

  return (rez);
}

Array *Convert_Image_Inplace(Array *R(M(array)), Array_Kind kind, Array_Type type, int scale)
{ return (convert_image(array,kind,type,scale,1)); }

Array *G(Convert_Image_Copy)(Array *array, Array_Kind kind, Array_Type type, int scale)
{ return (convert_image(array,kind,type,scale,0)); }

/****************************************************************************************
 *                                                                                      *
 *  ARRAY MULTIPLICATION                                                                *
 *                                                                                      *
 ****************************************************************************************/

Array *G(Array_Multiply)(Array *a, Array *b)
{ Array    *prod;
  int       acmplx, bcmplx;
  Size_Type b_off, a_off;

  { int        i, complex;
    Dimn_Type *pdims;

    acmplx  = (a->kind == COMPLEX_KIND);
    bcmplx  = (b->kind == COMPLEX_KIND);
    complex = acmplx || bcmplx;

    Dimn_Type *adims = a->dims + acmplx;
    Dimn_Type *bdims = b->dims + bcmplx;
    int       andims = a->ndims - acmplx;
    int       bndims = b->ndims - bcmplx;

    if (a->dims[acmplx] != b->dims[b->ndims-1])
      { fprintf(stderr,"Corresponding inner product dimensions for array multiply don't match");
        fprintf(stderr," (Array Multiply)\n");
        exit (1);
      }

    if (a->type != b->type)
      { fprintf(stderr,"Arrays must be of the same type (Array_Multiply)\n");
        exit (1);
      }

    if (complex)
      { prod = make_start(COMPLEX_KIND,a->type,andims+bndims-1,"Array_Multiply");
        prod->dims[0] = 2;
      }
    else
      prod = make_start(PLAIN_KIND,a->type,andims+bndims-2,"Array_Multiply");

    pdims = prod->dims + complex;
    for (i = 1; i < andims; i++)
      pdims[bndims+i-2] = adims[i];
    for (i = 0; i < bndims-1; i++)
      pdims[i] = bdims[i];
    prod->size = array_size(prod);

    allocate_array_data(prod,array_dsize(prod),"Make_Array");

    b_off  = ((b->size / bdims[bndims-1]) >> bcmplx);
    a_off  = adims[0];
  }

  { Size_Type p_off;
    Indx_Type p, q, r, t, k;

    switch (acmplx*2 + bcmplx) {
      #GENERATE C,S = 0 1 2 3, 1 2 2 2
        case <C>:
          p_off = b_off;          #WHEN C == 0
          p_off = (b_off << 1);   #WHEN C >  0
          b_off = p_off;          #WHEN C == 1 || C == 3
          a_off <<= 1;            #WHEN C >= 2
          switch (a->type) {
            #GENERATE T = @TYPES
              case <T>:
                { <t> *z = A<T>(prod);
                  <t> *y = A<T>(b);
                  <t> *x, sr, si;

                  for (q = 0; q < p_off; q += <S>)
                    { x = A<T>(a);
                      r = q;
                      for (p = 0; p < a->size; p += a_off)
                        { sr = 0;
                          #IF C == 0
                            for (t = q, k = 0; k < a_off; k++, t += b_off)
                              sr += x[k] * y[t];
                          #ELSE
                            si = 0;
                            #IF C == 3
                              for (k = 0, t = q; k < a_off; k += 2, t += b_off)
                                { sr += x[k] * y[t]   - x[k+1] * y[t+1];
                                  si += x[k] * y[t+1] + x[k+1] * y[t];
                                }
                            #ELSEIF C == 2
                              for (k = 0, t = (q>>1); k < a_off; k += 2, t += b_off)
                                { sr += x[k]   * y[t];
                                  si += x[k+1] * y[t];
                                }
                            #ELSEIF C == 1
                              for (k = 0, t = q; k < a_off; k++, t += b_off)
                                { sr += x[k] * y[t];
                                  si += x[k] * y[t+1];
                                }
                            #END
                            z[r+1] = si;
                          #END
                          z[r] = sr;
                          r += p_off;
                          x += a_off;
                        }
                    }
                  break;
                }
            #END
          }
          break;
      #END
    }
  }

  return (prod);
}

/****************************************************************************************
 *                                                                                      *
 *  COLOR OR OTHER ARRAY MAPPING                                                        *
 *                                                                                      *
 ****************************************************************************************/


Array *G(Apply_Map)(Array *image, Array *map)
{ Array     *rez;
  Size_Type  inner;

  inner = map->dims[0];

  if (map->ndims < 1)
    { fprintf(stderr,"Map should be at least 1 dimensional (Apply_Map)\n");
      exit (1);
    }
  if (image->type > UINT64 || image->kind != PLAIN_KIND)
    { fprintf(stderr,"image must have unsigned values and be a PLAIN array (Apply_Map)\n");
      exit (1);
    }
  if ((1<<image->scale) != inner)
    { fprintf(stderr,"map length doesn't match image scale (Apply_Map)\n");
      exit (1);
    }

  { int i, j;

    rez = make_start(PLAIN_KIND,map->type,image->ndims+map->ndims-1,"Apply_Map");
    rez->kind  = map->kind;
    rez->scale = map->scale;
    for (i = 0; i < image->ndims; i++)
      rez->dims[i] = image->dims[i];
    for (j = 1, i = image->ndims; i < rez->ndims; i++, j++)
      rez->dims[i] = map->dims[j];
    rez->size = array_size(rez);
    allocate_array_data(rez,array_dsize(rez),"Apply_Map");
    Set_Array_Text(rez,image->text);
  }

  { Indx_Type p, q;

    switch (map->type) {
      #GENERATE M = @TYPES
        case <M>:
          switch (image->type) {
            #GENERATE T = UINT8 UINT16 UINT32 UINT64
              case <T>:
                { <t> *a = A<T>(image);
                  <m> *m = A<M>(map);
                  <m> *t = A<M>(rez);
                  for (q = 0; q < map->size; q += inner)
                    { for (p = 0; p < image->size; p++)
                        *t++ = m[a[p]];
                      m += inner;
                    }
                  break;
                }
            #END
          }
          break;
      #END
    }
  }

  return (rez);
}


/****************************************************************************************
 *                                                                                      *
 *  DOWN SAMPLE CODE                                                                    *
 *                                                                                      *
 ****************************************************************************************/

static void down_sample(Array *source, void *target, Coordinate *F(voxel), Coordinate *F(basis))
{ static int        Max_Dims = 0;
  static Dimn_Type *dcnt, *tcnt;
  static Size_Type *dinc = NULL, *tinc;

  int        i, ndims;
  Dimn_Type *tdims, *vdims, *sdims;
  Size_Type  dsize;

  ndims = source->ndims;
  sdims = source->dims;
  tdims = ADIMN(basis);
  vdims = ADIMN(voxel);

  if (ndims > Max_Dims)
    { Max_Dims = ndims + 10;
      dinc = (Size_Type *) Guarded_Realloc(dinc,(sizeof(Size_Type)+sizeof(Dimn_Type))*2*Max_Dims,
                                           "Down_Sample");
      tinc = dinc + Max_Dims;
      dcnt = ((Dimn_Type *) tinc) + Max_Dims;
      tcnt = dcnt + Max_Dims;
    }

  { Size_Type offset, outer, inner;

    dsize   = vdims[0];
    dcnt[0] = 0;
    dinc[0] = 1;
    tcnt[0] = 0;
    tinc[0] = vdims[0];
    offset  = vdims[0] - 1;
    inner   = 1;
    outer   = sdims[0];
    for (i = 1; i < ndims; i++)
      { dsize  *= vdims[i];
        dcnt[i] = 0;
        dinc[i] = outer - offset;
        tcnt[i] = 0;
        tinc[i] = tinc[i-1] + (sdims[i-1] % vdims[i-1])*inner + (vdims[i]-1)*outer;
        offset += (vdims[i]-1)*outer;
        inner   = outer;
        outer  *= sdims[i];
      }
  }

  { Indx_Type q, p, r;
    Dimn_Type d0 = 0;
    Dimn_Type c0 = vdims[0];

    switch (source->type) {
      #GENERATE T,C = @TYPES, @CASTES
        case <T>:
          { <t> *d = A<T>(source);
            <t> *a = (<t> *) target;
            <C> sum;

            q = r = 0;
            i = 0;
            while (i != ndims)
              { p   = q;
                sum = 0.;
                while (i != ndims)
                  { sum += d[p];
                    if (++d0 < c0)
                      p += 1;
                    else
                      { d0 = 0;
                        for (i = 1; i < ndims; i++)
                          if (++dcnt[i] == vdims[i])
                            dcnt[i] = 0;
                          else
                            { p += dinc[i];
                              break;
                            }
                      }
                  }
          
                a[r++] = sum/dsize;
          
                for (i = 0; i < ndims; i++)
                  if (++tcnt[i] == tdims[i])
                    tcnt[i] = 0;
                  else
                    { q += tinc[i];
                      break;
                    }
              }
            break;
          }
      #END
      }
  }

  Free_Array(voxel);
  Free_Array(basis);
}
 
static void check_dsample_args(Array *source, Coordinate *M(point), char *routine)
{ Dimn_Type  *crd;
  int         i, ndims;
 
  ndims = point->size;
  if ( ! (source->ndims == ndims || source->ndims == ndims+1 && source->kind != PLAIN_KIND))
    { fprintf(stderr,"Array dimensionaliy and down sample vector don't match");
      fprintf(stderr," (%s)\n",routine);
      exit (1);
    }

  crd = ADIMN(point);
  for (i = 0; i < source->ndims; i++)
    if (crd[i] == 0)
      { fprintf(stderr,"Down sample vector has a 0-component (%s)\n",routine);
        exit (1);
      }

  if (ndims == source->ndims-1)
    { point = AddCoord(1,point);
      if (source->kind == COMPLEX_KIND)
        { Dimn_Type *scrd = ADIMN(point);
          for (i = ndims; i >= 1; i--)
            scrd[i] = scrd[i-1];
          scrd[0] = 1;
        }
    }
}

Array *Down_Sample_Inplace(Array *R(M(source)), Coordinate *F(point))
{ Coordinate *basis;
  Dimn_Type  *dims, *crd;
  int         i;

  check_dsample_args(source,point,"Down_Sample_Inplace");

  basis = Array_Basis(source);
  dims  = ADIMN(basis);
  crd   = ADIMN(point);
  for (i = 0; i < source->ndims; i++)
    dims[i] /= crd[i];

  down_sample(source,source->data,point,basis);

  for (i = 0; i < source->ndims; i++)
    source->dims[i] = dims[i];
  
  return (source);
}
    

Array *G(Down_Sample_Copy)(Array *source, Coordinate *F(point))
{ Coordinate *basis;
  Array      *target;
  Dimn_Type  *dims, *crd;
  int         i;
 
  check_dsample_args(source,point,"Down_Sample_Copy");

  basis = Array_Basis(source);
  dims  = ADIMN(basis);
  crd   = ADIMN(point);
  for (i = 0; i < source->ndims; i++)
    dims[i] /= crd[i];

  target = Make_Array(source->kind,source->type,source->ndims,dims);

  down_sample(source,target->data,point,basis);

  return (target);
}


/****************************************************************************************
 *                                                                                      *
 *  ARRAY CLIPPING CODE                                                                 *
 *                                                                                      *
 ****************************************************************************************/

void clip_array(Array *source, void *target, Coordinate *F(beg), Coordinate *F(end))
{ Slice    *slice;
  Indx_Type p, r, e;

  slice = Make_Slice(source,beg,end);

  switch (source->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *d = A<T>(source);
          <t> *a = (<t> *) target;

          e = Set_Slice_To_Last(slice);
          r = 0;
          for (p = Set_Slice_To_First(slice); 1; p = Next_Slice_Index(slice))
            { a[r++] = d[p];
              if (p == e) break;
            }
          break;
        }
    #END
  }

  Kill_Slice(slice);
}
 
Array *Clip_Array_Inplace(Array *R(M(source)), Coordinate *F(beg), Coordinate *F(end))
{ Coordinate *shape;
  Dimn_Type  *bcrd, *scrd, *dims, s;
  int         i, ndims;

  ndims = end->size;
  if (ndims != beg->size)
    { fprintf(stderr,"Coordinates are not of same dimensionality (Clip_Array_Copy)\n");
      exit (1);
    }
  if ( ! (source->ndims == ndims || source->ndims == ndims+1 && source->kind != PLAIN_KIND))
    { fprintf(stderr,"Source and coordinate dimensionality do not match (Clip_Array_Inplace)\n");
      exit (1);
    }

  shape = Copy_Array(end);
  bcrd  = ADIMN(beg);
  scrd  = ADIMN(shape);
  for (i = 0; i < ndims; i++)
    scrd[i] -= (bcrd[i]-1);

  clip_array(source,source->data,beg,end);

  dims = source->dims;
  if (source->ndims != ndims)
    { source->size = kind_size[source->kind];
      if (source->kind == COMPLEX_KIND)
        dims += 1;
    }
  else
    source->size = 1;
  for (i = 0; i < ndims; i++)
    { dims[i] = s = scrd[i];
      source->size *= s;
    }

  Free_Array(shape);

  return (source);
}
    

Array *G(Clip_Array_Copy)(Array *source, Coordinate *F(beg), Coordinate *F(end))
{ Coordinate *shape;
  Dimn_Type  *bcrd, *scrd;
  Array      *target;
  int         i, ndims;

  ndims = end->size;
  if (ndims != beg->size)
    { fprintf(stderr,"Coordinates are not of same dimensionality (Clip_Array_Copy)\n");
      exit (1);
    }
  if ( ! (source->ndims == ndims || source->ndims == ndims+1 && source->kind != PLAIN_KIND))
    { fprintf(stderr,"Source and coordinate dimensionality do not match (Clip_Array_Copy)\n");
      exit (1);
    }

  shape = Copy_Array(end);
  bcrd  = ADIMN(beg);
  scrd  = ADIMN(shape);
  for (i = 0; i < ndims; i++)
    scrd[i] -= (bcrd[i]-1);

  if (source->ndims == ndims)
    { target = Make_Array_With_Shape(PLAIN_KIND,source->type,shape);
      target->kind = source->kind;
    }
  else
    target = Make_Array_With_Shape(source->kind,source->type,shape);

  clip_array(source,target->data,beg,end);

  return (target);
}


/****************************************************************************************
 *                                                                                      *
 *  ARRAY EXPANDING CODE                                                                *
 *                                                                                      *
 ****************************************************************************************/

void expand_array(Array *target, void *source, Size_Type size, Coordinate *beg, Coordinate *end)
{ Slice     *slice;
  Indx_Type  p;
  int        in;

  slice = Make_Slice(target,beg,end);
  in    = Set_Slice_To_Index(slice,target->size-1);

  switch (target->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *a = A<T>(target);
          <t> *d = ((<t> *) source) + size;

          for (p = target->size; p-- > 0; )
            { if (in)
                a[p] = *--d; 
              else
                a[p] = 0;
              in = Dec_Slice(slice);
            }
          break;
        }
    #END
  }

  Kill_Slice(slice);
}
 
Array *Pad_Array_Inplace(Array *R(M(source)), Coordinate *F(anchor), Coordinate *F(shape))
{ Dimn_Type  *scrd, *acrd, *dims;
  Size_Type   size;
  int         i, ndims;

  ndims = anchor->size;
  if (anchor->size != shape->size || source->ndims != shape->size)
    { fprintf(stderr,"Coordinates and/or source array are not of same dimensionality");
      fprintf(stderr," (Pad_Array_Inplace)\n");
      exit (1);
    }
  if ( ! (source->ndims == ndims || source->ndims == ndims+1 && source->kind != PLAIN_KIND))
    { fprintf(stderr,"Source and coordinate dimensionality do not match (Pad_Array_Inplace)\n");
      exit (1);
    }

  size = source->size;
  scrd = ADIMN(shape);
  acrd = ADIMN(anchor);
  dims = source->dims;
  if (source->ndims != ndims)
    { source->size = kind_size[source->kind];
      if (source->kind == COMPLEX_KIND)
        dims += 1;
    }
  else
    source->size = 1;
  for (i = 0; i < ndims; i++)
    { int m = scrd[i];
      scrd[i] = acrd[i] + (dims[i]-1);
      dims[i] = m;
      source->size *= m;
    }

  allocate_array_data(source,array_dsize(source),"Expland_Array_In_Place");

  expand_array(source,source->data,size,anchor,shape);
  
  return (source);
}


Array *G(Pad_Array_Copy)(Array *source, Coordinate *F(anchor), Coordinate *F(shape))
{ Array      *target;
  Dimn_Type  *scrd, *acrd, *dims;
  int         i, ndims;
 
  ndims = anchor->size;
  if (ndims != shape->size)
    { fprintf(stderr,"Coordinates are not of same dimensionality (Pad_Array_Copy)\n");
      exit (1);
    }
  if ( ! (source->ndims == ndims || source->ndims == ndims+1 && source->kind != PLAIN_KIND))
    { fprintf(stderr,"Source and coordinate dimensionality do not match (Pad_Array_Copy)\n");
      exit (1);
    }

  scrd = ADIMN(shape);
  acrd = ADIMN(anchor);

  if (source->ndims == ndims)
    { target = Make_Array(PLAIN_KIND,source->type,source->ndims,scrd);
      target->kind = source->kind;
    }
  else
    target = Make_Array(source->kind,source->type,ndims,scrd);

  dims = source->dims;
  if (source->ndims != ndims && source->kind == COMPLEX_KIND)
    dims += 1;
  for (i = 0; i < ndims; i++)
    scrd[i] = acrd[i] + (dims[i]-1);

  expand_array(target,source->data,source->size,anchor,shape);

  return (target);
}


/****************************************************************************************
 *                                                                                      *
 *  CORRELATION AND REGRESSION ROUTINES                                                 *
 *                                                                                      *
 ****************************************************************************************/

static double inner_product_arrays(Array_Or_Slice *o1, Array_Or_Slice *o2)
{ Indx_Type p, q;
  Size_Type n;
  Array    *a1 = (Array *) o1;
  Array    *a2 = (Array *) o2;
  Slice    *s1 = (Array *) o1;
  Slice    *s2 = (Array *) o2;
  double    s;
  int       alice1, alice2;

  if (alice1 = Is_Slice(o1))
    a1 = Slice_Array(s1);
  if (alice2 = Is_Slice(o2))
    a2 = Slice_Array(s2);

  n = Slice_Volume(o1);
  s = 0;
  switch (a1->type) {
    #GENERATE S = @TYPES
      case <S>:
        { <s> *v1 = A<S>(a1);
          switch (a2->type) {
            #GENERATE T = @TYPES
              case <T>:
                { <t> *v2 = A<T>(a2);
                  if (alice1)
                    if (alice2)
                      { p = Set_Slice_To_First(s1);
                        q = Set_Slice_To_First(s2);
                        while (n-- > 0)
                          { s += v1[p]*v2[q];
                            p = Next_Slice_Index(s1);
                            q = Next_Slice_Index(s2);
                          }
                      }
                    else
                      { p = Set_Slice_To_First(s1);
                        for (q = 0; q < n; q++)
                          { s += v1[p]*v2[q];
                            p = Next_Slice_Index(s1);
                          }
                      }
                  else
                    if (alice2)
                      { q = Set_Slice_To_First(s2);
                        for (p = 0; p < n; p++)
                          { s += v1[p]*v2[q];
                            q = Next_Slice_Index(s2);
                          }
                      }
                    else
                      for (p = 0; p < n; p++)
                        s += v1[p]*v2[p];
                  return (s);
                }
            #END
          }
        }
    #END
  }
}

static double sum_array(Array_Or_Slice *o)
{ Indx_Type p;
  Size_Type n;
  Array    *a = (Array *) o;
  Slice    *s = (Array *) o;
  double    t;
  int       alice;

  if (alice = Is_Slice(o))
    a = Slice_Array(s);

  n = Slice_Volume(o);
  t = 0;
  switch (a->type) {
    #GENERATE T = @TYPES
      case <T>:
        { <t> *v = A<T>(a);
          if (alice)
            { p = Set_Slice_To_First(s);
              while (n-- > 0)
                { t += v[p];
                  p = Next_Slice_Index(s);
                }
            }
          else
            for (p = 0; p < n; p++)
              t += v[p];
          return (t);
        }
    #END
  }
}

static Double_Matrix *correlate(int n, Array_Or_Slice *a1, va_list *ap, char *routine)
{ static Array_Or_Slice **Args    = NULL;
  static int              Max_Arg = 0;

  Double_Matrix *rez;

  if (n > Max_Arg)
    { Max_Arg = n+5;
      Args    = (Array_Or_Slice **) Guarded_Realloc(Args,sizeof(Array_Or_Slice *)*Max_Arg,routine);
    }

  { int i;

    if (n > 0)
      Args[0] = a1;
    else
      { fprintf(stderr,"List of arrays is empty (%s)\n",routine);
        exit (1);
      }
    for (i = 1; i < n; i++)
      { Args[i] = (Array_Or_Slice *) va_arg(*ap,Array_Or_Slice *);
        if (Slice_Volume(Args[i]) != Slice_Volume(Args[0]))
          { fprintf(stderr,"Arrays do not have the same number of elements (%s)\n",routine);
            exit (1);
          }
      }
  }

  rez = Make_Array_With_Shape(PLAIN_KIND,FLOAT64,Coord2(n,n));
  
  { double *a = AFLOAT64(rez);
    int     i, j;

    for (i = 0; i < n; i++)
      for (j = 0; j <= i; j++)
        a[j*n+i] = a[i*n+j] = inner_product_arrays(Args[i],Args[j]);
  }
  
  return (rez);
}

static Double_Vector *array_means(int n, Array_Or_Slice *a1, va_list *ap, char *routine)
{ Double_Vector *rez;

  rez = Make_Array_With_Shape(PLAIN_KIND,FLOAT64,Coord1(n));
  
  { double *a = AFLOAT64(rez);
    int     i;

    a[0] = sum_array(a1);
    for (i = 1; i < n; i++)
      a[i] = sum_array((Array_Or_Slice *) va_arg(*ap,Array_Or_Slice *));
  }
  
  return (rez);
}

Double_Matrix *Correlation(int n, Array_Or_Slice *a1, ...)
{ va_list        ap;
  Double_Matrix *rez;

  va_start(ap,a1);
  rez = correlate(n,a1,&ap,"Correlation");
  va_end(ap);
  return (rez);
}

Double_Matrix *Covariance(int n, Array_Or_Slice *a1, ...)
{ va_list        ap;
  Double_Matrix *rez;
  Double_Vector *sum;

  va_start(ap,a1);
  rez = correlate(n,a1,&ap,"Covariance");
  va_end(ap);

  va_start(ap,a1);
  sum = array_means(n,a1,&ap,"Covariance");
  va_end(ap);

  { double *a = AFLOAT64(rez);
    double *m = AFLOAT64(sum);
    double  mi, mj, s;
    int     i, j;

    for (i = 0; i < n; i++)
      { mi = m[i]/n;
        for (j = 0; j <= i; j++)
          { s  = a[j*n+i];
            mj = m[j];
            a[j*n+i] = a[i*n+j] = (s - mj*mi)/n;
          }
      }
  }
  
  return (rez);
}

Double_Matrix *Pearson_Correlation(int n, Array_Or_Slice *a1, ...)
{ va_list        ap;
  Double_Matrix *rez;
  Double_Vector *sum;

  va_start(ap,a1);
  rez = correlate(n,a1,&ap,"Pearson_Correlation");
  va_end(ap);

  va_start(ap,a1);
  sum = array_means(n,a1,&ap,"Pearson_Correlation");
  va_end(ap);

  { double *a = AFLOAT64(rez);
    double *m = AFLOAT64(sum);
    double  mi, mj, s;
    int     i, j;

    for (i = 0; i < n; i++)
      { mi = m[i]/n;
        for (j = 0; j <= i; j++)
          { s  = a[j*n+i];
            mj = m[j];
            a[j*n+i] = a[i*n+j] = (s - mj*mi)/n;
          }
      }

    for (i = n*n-1; i >= 0; i -= n+1)
      a[i] = sqrt(a[i]);

    for (i = 1; i < n; i++)
      { mi = a[i*n+i];
        for (j = 0; j < i; j++)
          a[j*n+i] = a[i*n+j] /= (mi * a[j*n+j]);
      }

    for (i = n*n-1; i >= 0; i -= n+1)
      a[i] = 1.;
  }
  
  return (rez);
}

Double_Vector *Linear_Regression(int n, Array_Or_Slice *obs, Array_Or_Slice *in1, ...)
{ va_list        ap;
  Double_Matrix *cof;
  Double_Vector *tgt;
  LU_Factor     *lu;
  int            stable;
  
  if (Slice_Volume(obs) != Slice_Volume(in1))
    { fprintf(stderr,"Arrays do not have the same number of elements (Linear_Regression)\n");
      exit (1);
    }

  va_start(ap,in1);
  cof = correlate(n,in1,&ap,"Linear_Regression");
  va_end(ap);

  tgt = Make_Array_With_Shape(PLAIN_KIND,FLOAT64,Coord1(n));
  
  { double *a = AFLOAT64(tgt);
    int     i;

    va_start(ap,in1);
    a[0] = inner_product_arrays(in1,obs);
    for (i = 1; i < n; i++)
      a[i] = inner_product_arrays((Array_Or_Slice *) va_arg(ap,Array_Or_Slice *),obs);
    va_end(ap);
  }

  lu = LU_Decompose(cof,&stable);
  if ( ! stable)
    fprintf(stderr,"Warning: Coefficient array inversion was unstable (Linear_Regression()\n");

  LU_Solve(tgt,lu);
  
  Free_LU_Factor(lu);

  return (tgt);
}
