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

#ifndef _DRAW_LIB

#define _DRAW_LIB

#include <stdio.h>

#include "parameters.h"
#include "array.h"
#include "region.h"
#include "level.set.h"
#include "water.shed.h"

/* A Brush_Bundle specifies how to modify each pixel while performing any of the draw operations
   below.  As a user you fill in either a Plain_Bundle, Complex_Bundle, or Color_Bundle and the
   Value type (see array.h) according to the kind and type of array the brush will be applied to.  
   The "op" directive controls painting as follows:

   SET_PIX: each pixel value is set to the draw value.
   ADD_PIX: each pixel value has the draw value added to it,
              and the pixel is clipped if it over-flows.
   SUB_PIX: each pixel value has the draw value subtracted from it,
              and the pixel is clipped if it under-flows.
   MUL_PIX: each pixel is multiplied by the draw value.
   DIV_PIX: each pixel is divided by the draw value.
   MIN_PIX: each pixel is set to the minimum of its current value and the draw value.
   MAX_PIX: each pixel is set to the maximum of its current value and the draw value.

   You use ival's for INT? arrays, uval's for UINT? arrays, and rval's for FLOAT? arrays.
   Both RGB and RGBA arrays use a Color_Bundle, in the former case the alpha field is ignored.
   If the operation is MUL_PIX or DIV_PIX, then the relevant brush is assumed to be a real
   *regardless* of the type of the array.  To give some examples, { SET_PIX, 255, 255, 0 }
   would paint a yellow pixel in a UINT8 RGB matrix, { MUL_PIX, .5, .5 } would halve the values
   in any type of COMPLEX matrix, and { MAX_PIX, -10, -10, -10, -10 } would set all pixels less
   than -10 to -10 in an INT? RGBA array.
*/

typedef enum
  { SET_PIX = 0,
    ADD_PIX = 1,
    SUB_PIX = 2,
    MUL_PIX = 3,
    DIV_PIX = 4,
    MIN_PIX = 5,
    MAX_PIX = 6
  } Drawer;

typedef struct
  { Drawer op;
    Value  val;
  } Plain_Bundle;

typedef struct
  { Drawer op;
    Value  real;
    Value  imag;
  } Complex_Bundle;

typedef struct
  { Drawer op;
    Value  red;
    Value  green;
    Value  blue;
    Value  alpha;
  } Color_Bundle;

typedef void Brush_Bundle;

/* Drawing routines for regions.  */

void Draw_Region(Array *M(canvas), Brush_Bundle *brush, Region *trace);
void Draw_Region_Outline(Array *M(canvas), Brush_Bundle *brush, Region *trace);
void Draw_Region_Exterior(Array *M(canvas), Brush_Bundle *brush, Region *trace);
void Draw_Region_Holes(Array *M(canvas), Brush_Bundle *brush, Region *trace);

/* Drawing routines for level sets.  */

void Draw_Level_Set(Array *M(canvas), Brush_Bundle *brush, Level_Set *r);
void Draw_Level_Set_Outline(Array *M(canvas), Brush_Bundle *brush, Level_Set *r);
void Draw_Level_Set_Exterior(Array *M(canvas), Brush_Bundle *brush, Level_Set *r);
void Draw_Level_Set_Holes(Array *M(canvas), Brush_Bundle *brush, Level_Set *r);

/* Drawing routines for water sheds.  */

void Draw_Watershed(Array *M(canvas), Brush_Bundle *brush, Watershed *shed, int cb);
void Draw_Watershed_Outline(Array *M(canvas), int cb, Brush_Bundle *brush, Watershed *w);
void Draw_Watershed_Exterior(Array *M(canvas), int cb, Brush_Bundle *brush, Watershed *w);
void Draw_Watershed_Holes(Array *M(canvas), int cb, Brush_Bundle *brush, Watershed *w);

/* Draw_Floodfill flood fills the (2n)- or (3^n-1)-connected region (determined by iscon2n) of
   the image "source" of pixels that include pixel seed and have value >=, =, or <= level
   (determined by cmprsn).  The drawing is performed on image canvas which must have
   the same dimensions as source.                                                     */

void Draw_Floodfill(Array *M(canvas), Brush_Bundle *brush, Array *source,
                    Indx_Type seed, Comparator cmprsn, double level, int iscon2n);

/* Draw simple objects.  The coordinates are "canvas coordinates" which is basically the coordinate
     into the array as if any extra dimension present because of its kind were not present.
     That is if one had a 2D RGB image, it has dimensionality three, but its canvas coordinate
     is into the (lower) 2D portion of the array as if the extra third dimension was not there.
     Similarly, for a 2D COMPLEX array, the canvas coord ignores the lowest dimension, addressing
     the 2D array of complex numbers.  **BEWARE** the supplied coordinates are always consumed
     by the drawing primitive (i.e. freed before returning).

     In the frequent event that you have an index into the data portion of an array and want
     to draw with respect to the canvals coordinate implied by it, call Idx2Canvas on the
     index to produce such a coordinate array.  Coordinate is defined in array.h                */

Coordinate *G(Idx2Canvas)(Array *canvas, Indx_Type index);

void Draw_Point(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(point));
void Draw_Cross(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius);
void Draw_Rectangle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(corner1),
                                                           Coordinate *F(corner2));
void Draw_Circle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius);
void Draw_Line(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(beg), Coordinate *F(end));

/* Draw an entire canvas */

void Draw_Image(Array *M(image), Brush_Bundle *brush);

#endif
