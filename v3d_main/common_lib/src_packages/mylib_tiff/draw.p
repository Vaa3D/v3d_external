/*****************************************************************************************\
*                                                                                         *
*  Drawing Level Sets, Regions, and Basic Shapes                                          *
*                                                                                         *
*  Author  :  Gene Myers                                                                  *
*  Date    :  June 2007                                                                   *
*  Last Mod:  Aug 2008                                                                    *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdint.h>
#include <math.h>

#include "utilities.h"
#include "draw.h"

//  A Brush_Bundle determines the setting of the following globals that are then
//    used by the various painter routines below to set pixels.

static double  redR;     //  Paint brush values
static double  greenR;
static double  blueR;
static double  alphaR;

static int64   redI;     //  Paint brush values
static int64   greenI;
static int64   blueI;
static int64   alphaI;

static uint64  redU;     //  Paint brush values
static uint64  greenU;
static uint64  blueU;
static uint64  alphaU;


#LISTDEF @TYPES  =  UINT8 UINT16 UINT32 UINT64 INT8 INT16 INT32 INT64 FLOAT32 FLOAT64
#LISTDEF @VALUES =      U     U       U      U    I     I     I     I       R       R

#LISTDEF @OP_NAME  = SET_PIX ADD_PIX SUB_PIX MUL_PIX DIV_PIX MIN_PIX MAX_PIX
#LISTDEF @OPERATOR =       =      +=      -=      *=      /=       <       >

                        //  Paint brush target arrays for each channel
#GENERATE T = @TYPES
  typedef struct
    { <t> *red;
      <t> *green;
      <t> *blue;
      <t> *alpha;
    } P<T>;
  
  static P<T> <T>_Paint;

#END

  //  Generate a painter for each image kind, array type, and paint operator combination!

#GENERATE T,V = @TYPES,@VALUES
  #GENERATE OP,SYM = @OP_NAME,@OPERATOR
    #IF OP < MUL_PIX

      static void PAINT_<T>_PLAIN_<OP>(Indx_Type p)
      { <T>_Paint.red[p] <SYM> red<V>; }

      static void PAINT_<T>_RGB_<OP>(Indx_Type p) {
        #GENERATE C = red green blue
          <T>_Paint.<C>[p] <SYM> <C><V>;
        #END
      }

      static void PAINT_<T>_RGBA_<OP>(Indx_Type p) {
        #GENERATE C = red green blue alpha
          <T>_Paint.<C>[p] <SYM> <C><V>;
        #END
      }

      static void PAINT_<T>_COMPLEX_<OP>(Indx_Type p) {
        #GENERATE C = red green
          <T>_Paint.<C>[p] <SYM> <C><V>;
        #END
      }

    #ELSEIF OP < MIN_PIX

      static void PAINT_<T>_PLAIN_<OP>(Indx_Type p)
      { <T>_Paint.red[p] <SYM> redR; }

      static void PAINT_<T>_RGB_<OP>(Indx_Type p) {
        #GENERATE C = red green blue
          <T>_Paint.<C>[p] <SYM> <C>R;
        #END
      }

      static void PAINT_<T>_RGBA_<OP>(Indx_Type p) {
        #GENERATE C = red green blue alpha
          <T>_Paint.<C>[p] <SYM> <C>R;
        #END
      }

      static void PAINT_<T>_COMPLEX_<OP>(Indx_Type p) {
        #GENERATE C = red green
          <T>_Paint.<C>[p] <SYM> <C>R;
        #END
      }

    #ELSE

      static void PAINT_<T>_PLAIN_<OP>(Indx_Type p)
      { double x = <T>_Paint.red[p];
        if (red<V> <SYM> x) <T>_Paint.red[p] = red<V>;
      }

      static void PAINT_<T>_RGB_<OP>(Indx_Type p) {
        double x;
        #GENERATE C = red green blue
          x = <T>_Paint.<C>[p];
          if (<C><V> <SYM> x) <T>_Paint.<C>[p] = <C><V>;
        #END
      }

      static void PAINT_<T>_RGBA_<OP>(Indx_Type p) {
        double x;
        #GENERATE C = red green blue alpha
          x = <T>_Paint.<C>[p];
          if (<C><V> <SYM> x) <T>_Paint.<C>[p] = <C><V>;
        #END
      }

      static void PAINT_<T>_COMPLEX_<OP>(Indx_Type p) {
        double x;
        p <<= 1;
        #GENERATE C = red green
          x = <T>_Paint.<C>[p];
          if (<C><V> <SYM> x) <T>_Paint.<C>[p] = <C><V>;
        #END
      }

    #END
  #END
#END

static void *Painter_Table[] = {   //  Make a table of all these puppies
    #GENERATE K = PLAIN RGB RGBA COMPLEX
      #GENERATE T = @TYPES
        #GENERATE OP = @OP_NAME
          PAINT_<T>_<K>_<OP>,
        #END
      #END
    #END
  };
  
static int bit_size[] = { 8, 16, 32, 64, 8, 16, 32, 64, 32, 64 };

void *SETUP_PAINTER(Array *canvas, Brush_Bundle *generic)
{ Color_Bundle *brush = (Color_Bundle  *) generic;
  int           kind  = canvas->kind;

  if (canvas->type >= FLOAT32 || brush->op == MUL_PIX || brush->op == DIV_PIX)
    { redR = brush->red.rval;
      if (kind != PLAIN_KIND)
        { greenR = brush->green.rval;
          if (kind != COMPLEX_KIND)
            { blueR  = brush->blue.rval;
              if (kind != RGB_KIND)
                alphaR = brush->alpha.rval;
            }
        }
    }
  else if (canvas->type <= UINT64)
    { redU = brush->red.uval;
      if (kind != PLAIN_KIND)
        { greenU = brush->green.uval;
          if (kind != COMPLEX_KIND)
            { blueU  = brush->blue.uval;
              if (kind != RGB_KIND)
                alphaU = brush->alpha.uval;
            }
        }
    }
  else //  canvas->type <= INT64)
    { redI = brush->red.ival;
      if (kind != PLAIN_KIND)
        { greenI = brush->green.ival;
          if (kind != COMPLEX_KIND)
            { blueI  = brush->blue.ival;
              if (kind != RGB_KIND)
                alphaI = brush->alpha.ival;
            }
        }
    }

  switch (canvas->type) {
    #GENERATE T = @TYPES
      case <T>:
        <T>_Paint.red = (<t> *) (canvas->data);
        if (canvas->kind > PLAIN_KIND)
          if (canvas->kind == COMPLEX_KIND)
            <T>_Paint.green = <T>_Paint.red + 1;
          else
            { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
              <T>_Paint.green = <T>_Paint.red + n;
              <T>_Paint.blue  = <T>_Paint.green + n;
              if (canvas->kind == RGBA_KIND)
                <T>_Paint.alpha = <T>_Paint.blue + n;
            }
        break;
    #END
  }

  return (Painter_Table[(canvas->kind*10 + canvas->type)*7 + brush->op]);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR CONTOURS                                                       *
 *                                                                                      *
 ****************************************************************************************/

typedef struct      //  Taken from region.p (internally visible declaration of a Region)
  { Size_Type  rastlen;
    Size_Type  surflen;
    int        iscon2n;
    int        ndims;
    Dimn_Type *dims;
    Indx_Type *raster;
    uint8     *ishole;
    Size_Type  area;
  } RasterCon;

void check_shapes(Array *canvas, RasterCon *cont, char *routine)
{ static Array acont;

  acont.ndims = cont->ndims;
  acont.dims  = cont->dims;
  if (canvas->kind != PLAIN_KIND)
    { canvas->ndims -= 1;
      if (canvas->kind == COMPLEX_KIND)
        canvas->dims += 1;
    }
  if ( ! Same_Shape(canvas,&acont))
    { fprintf(stderr,"Canvas and image from which trace was computed do not have the same shape");
      fprintf(stderr," (%s)\n",routine);
      exit (1);
    }
  if (canvas->kind != PLAIN_KIND)
    { if (canvas->kind == COMPLEX_KIND)
        canvas->dims -= 1;
      canvas->ndims += 1;
    }
}

/* Color the pixels of a trace */

void Draw_Region_Outline(Array *M(canvas), Brush_Bundle *brush, Region *cont)
{ RasterCon *trace = (RasterCon *) cont;
  Indx_Type  i, len;
  Indx_Type *raster;
  uint8     *ishole;
  void      (*painter)(Indx_Type);

  check_shapes(canvas,trace,"Draw_Region_Outline");

  painter = SETUP_PAINTER(canvas,brush);

  raster = trace->raster;
  ishole = trace->ishole;
  len    = trace->rastlen;

  painter(raster[0]);
  for (i = 2; i < len; i += 2)
    if (!ishole[i>>1])
      { painter(raster[i-1]);
        painter(raster[i]);
      }

  len = trace->surflen;
  for (i = trace->rastlen-1; i < len; i++)
    painter(raster[i]);
}

/* Color the region defined by trace */

void Draw_Region(Array *M(canvas), Brush_Bundle *brush, Region *cont)
{ RasterCon *trace = (RasterCon *) cont;
  Indx_Type *raster;
  Indx_Type  len;
  void     (*painter)(Indx_Type);

  check_shapes(canvas,trace,"Draw_Region_Interior");

  painter = SETUP_PAINTER(canvas,brush);

  raster = trace->raster;
  len    = trace->rastlen;

  { Indx_Type i, v, w, p;

    for (i = 0; i < len; i += 2)
      { v = raster[i];
        w = raster[i+1];
        for (p = v; p <= w; p++)
          painter(p);
      }
  }
}

/* Color the complement of the region defined by trace */

void Draw_Region_Exterior(Array *M(canvas), Brush_Bundle *brush, Region *cont)
{ RasterCon *trace = (RasterCon *) cont;
  Indx_Type *raster, len;
  uint8     *ishole;
  void     (*painter)(Indx_Type);

  check_shapes(canvas,trace,"Draw_Region_Exterior");

  painter = SETUP_PAINTER(canvas,brush);

  raster = trace->raster;
  len    = trace->rastlen-1;
  ishole = trace->ishole;

  { Indx_Type i, v, w, p;

    for (p = 0; p < raster[0]; p++)
      painter(p);
    for (i = 2; i < len; i += 2)
      if (!ishole[i>>1])
        { v = raster[i-1];
          w = raster[i];
          for (p = v+1; p < w; p++)
            painter(p);
        }
    for (p = raster[len]+1; p < canvas->size; p++)
      painter(p);
  }
}

void Draw_Region_Holes(Array *M(canvas), Brush_Bundle *brush, Region *cont)
{ RasterCon *trace = (RasterCon *) cont;
  Indx_Type *raster, len;
  uint8     *ishole;
  void     (*painter)(Indx_Type);

  check_shapes(canvas,trace,"Draw_Region_Holes");

  painter = SETUP_PAINTER(canvas,brush);

  raster = trace->raster;
  len    = trace->rastlen;
  ishole = trace->ishole;

  { Indx_Type i, v, w, p;

    for (i = 2; i < len; i += 2)
      if (ishole[i>>1])
        { v = raster[i-1];
          w = raster[i];
          for (p = v+1; p < w; p++)
            painter(p);
        }
  }
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR LEVEL SETS                                                     *
 *                                                                                      *
 ****************************************************************************************/

void Draw_Level_Set_Outline(Array *M(canvas), Brush_Bundle *brush, Level_Set *r)
{ Region *c;
  c = Record_Level_Set(r,0);
  Draw_Region_Outline(canvas,brush,c);
  Free_Region(c);
}

void Draw_Level_Set_Exterior(Array *M(canvas), Brush_Bundle *brush, Level_Set *r)
{ Region *c;
  c = Record_Level_Set(r,0);
  Draw_Region_Exterior(canvas,brush,c);
  Free_Region(c);
}

void Draw_Level_Set_Holes(Array *M(canvas), Brush_Bundle *brush, Level_Set *r)
{ Region *c;
  c = Record_Level_Set(r,1);
  Draw_Region_Holes(canvas,brush,c);
  Free_Region(c);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR WATERHSEDS                                                     *
 *                                                                                      *
 ****************************************************************************************/

void Draw_Watershed_Outline(Array *canvas, int cb, Brush_Bundle *brush, Watershed *w)
{ Region *c;
  c = Record_Watershed(w,cb,0);
  Draw_Region_Outline(canvas,brush,c);
  Free_Region(c);
}

void Draw_Watershed_Exterior(Array *canvas, int cb, Brush_Bundle *brush, Watershed *w)
{ Region *c;
  c = Record_Watershed(w,cb,0);
  Draw_Region_Exterior(canvas,brush,c);
  Free_Region(c);
}

void Draw_Watershed_Holes(Array *canvas, int cb, Brush_Bundle *brush, Watershed *w)
{ Region *c;
  c = Record_Watershed(w,cb,1);
  Draw_Region_Holes(canvas,brush,c);
  Free_Region(c);
}

/****************************************************************************************
 *                                                                                      *
 *  GENERAL FLOOD-FILL DRAWING ROUTINE                                                  *
 *                                                                                      *
 ****************************************************************************************/

#GENERATE T = @TYPES
  static <t> *Value_<T>, Level_<T>;

  #GENERATE C,O = LE LT EQ NE GT GE, <= < == != > >=
    static int is_<c>_<t>(Indx_Type p)
    { return (Value_<T>[p] <o> Level_<T>); }

  #END
#END

static void *Comparator_Table[] = {
#GENERATE T = @TYPES
  #GENERATE C = LE LT EQ NE GT GE
    is_<c>_<t>,
  #END
#END
  };

static void check_drawing_compatibility(Array *canvas, Array *source, char *routine)
{ if (source->kind != PLAIN_KIND)
    { fprintf(stderr,"Source must be a plain array (%s)\n",routine);
      exit (1);
    }
  if (canvas->kind != PLAIN_KIND)
    { canvas->ndims -= 1;
      if (canvas->kind == COMPLEX_KIND)
        canvas->dims += 1;
    }
  if ( ! Same_Shape(canvas,source))
    { fprintf(stderr,"Canvas and source do not have the same shape! (%s)\n",routine);
      exit (1);
    }
  if (canvas->kind != PLAIN_KIND)
    { if (canvas->kind == COMPLEX_KIND)
        canvas->dims -= 1;
      canvas->ndims += 1;
    }
}

void Draw_Floodfill(Array *M(canvas), Brush_Bundle *brush, Array *source,
                    Indx_Type seed, Comparator cmprsn, double level, int iscon2n)
{ int  (*value)(Indx_Type);

  check_drawing_compatibility(canvas,source,"Draw_Region");

  value = Comparator_Table[6*source->type + cmprsn];

  switch (source->type) {
    #GENERATE T = @TYPES
      case <T>:
        Value_<T> = (<t> *) (source->data);
        Level_<T> = level;
        break;
    #END
  }

  Flood_Object(source,iscon2n,seed,value,NULL,SETUP_PAINTER(canvas,brush));
}

void Draw_Watershed(Array *M(canvas), Brush_Bundle *brush, Watershed *shed, int cb)
{ Array_Bundle *source;
  Indx_Type     seed = Get_Watershed_Basin(shed,cb)->seed;

  source = Get_Watershed_Labels(shed);

  check_drawing_compatibility(canvas,source,"Draw_Region");

  Value_UINT8 = AUINT8(source);
  Level_UINT8 = Value_UINT8[seed];

  Flood_Object(source,Get_Watershed_Connectivity(shed),seed,is_eq_uint8,
               NULL,SETUP_PAINTER(canvas,brush));
}

void Draw_Level_Set(Array *M(canvas), Brush_Bundle *brush, Level_Set *r)
{ Component_Tree *tree    = Get_Current_Component_Tree();
  Array          *image   = Get_Component_Tree_Image(tree);
  int             iscon2n = Get_Component_Tree_Connectivity(tree);

  Draw_Floodfill(canvas,brush,image,Level_Set_Leftmost(r),GE,Level_Set_Level(r),iscon2n);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR BASIC SHAPES                                                   *
 *                                                                                      *
 ****************************************************************************************/

static Array *base_shape(Array *a)
{ static Array base;

  base.ndims = a->ndims;
  base.dims  = a->dims;
  if (a->kind != PLAIN_KIND)
    { if (a->kind == COMPLEX_KIND)
        base.dims  += 1;
      base.ndims -= 1;
    }
  return (&base);
}

void check_ivector(Coordinate *coord, Array *base, char *routine)
{ if (coord->kind != PLAIN_KIND || coord->type > UINT32 || coord->ndims != 1)
    { fprintf(stderr,"Coordinate is not an unsigned integer vector (%s)\n",routine);
      exit (1);
    }
  if (coord->dims[0] != (Dimn_Type) base->ndims)
    { fprintf(stderr,"Coordinate is not of the correct dimensionality (%s)\n",routine);
      exit (1);
    }
}

Coordinate *G(Idx2Canvas)(Array *canvas, Indx_Type idx)
{ if (canvas->kind == COMPLEX_KIND)
    idx /= 2;
  else if (canvas->kind == RGB_KIND)
    idx %= (canvas->size/3);
  else if (canvas->kind == RGBA_KIND)
    idx %= (canvas->size/4);
  return (Idx2CoordA(base_shape(canvas),idx));
}

static int        d_point_max = 0;  //  A couple of vectors for internal use by the routines below
static Dimn_Type *d_point1 = NULL;
static Dimn_Type *d_point2;

static void draw_vector(int n, char *routine)
{ if (n > d_point_max)
    { d_point_max = n+5;
      d_point1    = (Dimn_Type *) Guarded_Realloc(d_point1,sizeof(double)*2*d_point_max,routine);
      d_point2    = (Dimn_Type *) ( ((double *) d_point1) + d_point_max);
    }
}

static void       (*d_painter)(Indx_Type);
static Dimn_Type  *d_dims;
static Dimn_Type  *d_center;

/* Draw a size[0] x size[1] x ... x size[n-1] rectangle with lower left corner
   (corner[0],corner[1],...,corner[n-1]) where n = canvas->ndims-1.              */

static void rectangle(int k, Indx_Type base)
{ Dimn_Type beg = d_point1[k];
  Dimn_Type end = d_point2[k];
  Dimn_Type p;

  base *= d_dims[k];
  if (k == 0)
    { for (p = beg; p <= end; p++)
        d_painter(base+p);
    }
  else
    { for (p = beg; p <= end; p++)
        rectangle(k-1,base+p);
    }
}

void Draw_Rectangle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(corner1),
                                                           Coordinate *F(corner2))
{ Array     *base;
  int        i,   n;
  Dimn_Type *c1, *c2, *d;

  base = base_shape(canvas);

  check_ivector(corner1,base,"Draw_Rectangle");
  check_ivector(corner2,base,"Draw_Rectangle");

  n = base->ndims;
  d = base->dims;

  d_painter = SETUP_PAINTER(canvas,brush);
  d_dims    = d;
  draw_vector(n,"Draw_Rectangle");

  c1 = ADIMN(corner1);
  c2 = ADIMN(corner2);
  for (i = 0; i < n; i++)
    { Dimn_Type beg = c1[i];
      Dimn_Type end = c2[i];
      if (beg < 0)
        beg  = 0;
      if (end > d[i])
        end = d[i];
      d_point1[i] = beg;
      d_point2[i] = end;
    }

  rectangle(n-1,0);

  Free_Array(corner1);
  Free_Array(corner2);
}

/* Reset an entire image */

void Draw_Image(Array *M(canvas), Brush_Bundle *brush)
{ Array     *base;
  int        i, n;
  Dimn_Type *d;

  base = base_shape(canvas);

  n = base->ndims;
  d = base->dims;

  d_painter = SETUP_PAINTER(canvas,brush);
  d_dims    = d;
  draw_vector(n,"Draw_Image");

  for (i = 0; i < n; i++)
    { d_point1[i] = 0;
      d_point2[i] = d[i]-1;
    }

  rectangle(n-1,0);
}

/* Draw a point centered a pixel point */

void Draw_Point(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(point))
{ Array     *base;
  Dimn_Type *p;
  int        i;
  void      (*painter)(Indx_Type);

  base = base_shape(canvas);

  check_ivector(point,base,"Draw_Cross");

  p = ADIMN(point);
  for (i = 0; i < base->ndims; i++)
    if (p[i] >= base->dims[i])
      return;

  painter = SETUP_PAINTER(canvas,brush);
  painter(Coord2IdxA(base,point));
}

/* Draw a cross centered at pixel center with each arm being radius pixels long */

void Draw_Cross(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius)
{ Array     *base;
  int        i, n;
  Dimn_Type *d, *c;
  Indx_Type  p, q;
  void       (*painter)(Indx_Type);

  base = base_shape(canvas);

  check_ivector(center,base,"Draw_Cross");

  n = base->ndims;
  d = base->dims;

  painter = SETUP_PAINTER(canvas,brush);

  q = 1;
  c = ADIMN(center);
  p = Coord2IdxA(base,center);
  for (i = 0; i < n; i++)
    { int64 x = c[i];
      int64 b, e, k;

      if (x < radius)
        b = -x;
      else
        b = -radius;
      if (x+radius >= d[i])
        e = (d[i] - x) - 1;
      else
        e = radius;
      for (k = b; k <= e; k++)
        if (k != 0)
          painter(p+q*k);
      q *= d[i];
    }
  painter(p);
}

/* Draw an n-dimensional circle centered at pixel center with radius radius,
     where n = canvas->ndims                                                  */

static void circle(int k, Indx_Type base, int64 rem, int64 rad)
{ Indx_Type i;
  int64     beg, end, rng;

  while (rad*rad > rem)
    rad -= 1;

  beg = d_center[k] - rad;
  if (beg < 0)
    beg = 0;
  end = d_center[k] + rad;
  if (end > d_dims[k])
    end = d_dims[k]-1;

  base *= d_dims[k];
  if (k == 0)
    { for (i = beg; i <= end; i++)
        d_painter(base+i);
    }
  else
    { rng = beg-d_center[k];
      for (i = beg; i <= end; i++)
        { circle(k-1,base+i,rem-rng*rng,rad);
          rng += 1;
        }
    }
}

void Draw_Circle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius)
{ Array *base;

  base = base_shape(canvas);

  check_ivector(center,base,"Draw_Circle");

  d_painter = SETUP_PAINTER(canvas,brush);
  d_dims    = base->dims;
  d_center  = ADIMN(center);

  circle(base->ndims-1,0,radius*radius,(Dimn_Type) radius);

  Free_Array(center);
}

/*  Draw an n-dimensional line from begp to endp, clipping to the canvas as necessary */

void Draw_Line(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(begp), Coordinate *F(endp))
{ int            ndims;
  Dimn_Type     *beg,  *end, *dims;
  void          (*painter)(int);
  int            kmax;
  int            bgk, enk;
  double        *val, *inc;

  { Array         *base;

    base = base_shape(canvas);

    check_ivector(begp,base,"Draw_Line");
    check_ivector(endp,base,"Draw_Line");

    painter = SETUP_PAINTER(canvas,brush);

    beg  = ADIMN(begp);
    end  = ADIMN(endp);

    ndims = base->ndims;
    dims  = base->dims;

    draw_vector(ndims,"Draw_Line");
    val = (double *) d_point1;
    inc = (double *) d_point2;
  }

  { Dimn_Type d, maxd = 0;           //  kmax = dimension with largest delta
    int       i;

    for (i = 0; i < ndims; i++)
      { if (end[i] > beg[i])
          val[i] = d = end[i] - beg[i];
        else
          { d = beg[i] - end[i];
            val[i] = -1.*d;
          }
        if (d > maxd)
          { maxd = d;
            kmax = i;
          }
      }
    if (maxd == 0) goto exit_line;
  }

  { double ab, ae;
    double dkm, bkm;
    double den, dim;
    double abg, aen;
    int    i;

    ab = 0.;                   //  figure the clip interval [ab,ae] <= [0,1] for the line
    ae = 1.;
    for (i = 0; i < ndims; i++)
      { den = val[i];
        dim = dims[i] - .5;
        if (den == 0.)
          { if (beg[i] < 0 || beg[i] > dim)
              ab = 2.;
          }
        else
          { abg = - (beg[i] + .5) / den;
            aen = (dim - beg[i]) / den;
            if (abg > aen)
              { den = abg; abg = aen; aen = den; }
            if (abg > ab)
              ab = abg;
            if (aen < ae)
              ae = aen;
          }
      }

    bkm = beg[kmax];        //  then further refine to have integral start and end poinnts,
    dkm = val[kmax];        //    bgk & end, for dimension kmax
    if (dkm > 0.)
      { enk = bkm + ae * dkm;
        bgk = ceil(bkm + ab * dkm);
      }
    else
      { bgk = bkm + ab * dkm;
        enk = ceil(bkm + ae * dkm);
      }
    ab = (bgk - bkm) / dkm;
    ae = (enk - bkm) / dkm;

    if (ab > ae)
      goto exit_line;

    dkm = abs(dkm);
    for (i = 0; i < ndims; i++)   // compute clipped start points and increments for every dimension
      { den = val[i];
        inc[i] = den / dkm;
        if (i == kmax)
          val[i] = bgk;
        else
          val[i] = beg[i] + ab * den + .5;
      }
  }

  { int       k, step;
    Dimn_Type i;
    Indx_Type p;

    if (bgk <= enk)
      step = 1;
    else
      step = -1;
    enk += step;
    for (i = bgk; i != enk; i += step)  //  for each integer along dimension kmax,
      { k = ndims-1;                    //    paint the nearest pixel
        p = val[k];
        val[k] += inc[k];
        for (k--; k >= 0; k--)
          { p = p * dims[k] + ((int) val[k]);
            val[k] += inc[k];
          }
        painter(p);
      }
  }

exit_line:
  Free_Array(begp);
  Free_Array(endp);
}
