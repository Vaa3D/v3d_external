/*****************************************************************************************\
*                                                                                         *
*  Image and Image Stack Data Abstraction for TIF-encoded files                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2006                                                                   *
*  Mods  :  November 2007: added idea of text description and read/write to               *
*              TIFFTAG_IMAGEDESCRIPTION tag of tif format                                 *
*           May 2008: Replaced libtiff with my own tiff library, necessitating            *
*              the introduction of a Tiff data type, and the addition of Advance_Tiff     *
*              and Tiff_EOF to the abstraction (cleaner than before).                     *
*           Dec 2009: Introduced arrays to generalize images.  Almost a complete rewrite. *
*              Also introduced layers to capture an unlimited number of channels.         *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _IMAGE_LIB

#define _IMAGE_LIB

#include "parameters.h"
#include "array.h"

   /* If you need to deal with images (both planes, stacks, movies, etc.) in their full
        complexity then use the routines in the tiff.io and tiff.image modules.

      Tiff, Stacks, and Layers:

        A tiff file contains one or more IFD's each of which encodes a 2D array.  A 3D stack
        is not explicitly supported but the convention is that if a tiff is a sequence of IFD's
        all encoding arrays of the same dimensions and type, then the sequence of IFD's is each
        successive z-plane of the stack.  The array in a given IFD can have as many channels as
        you would like (in tiff-speak, samples instead of channels).  Each channel can contain
        a signed or unsigned integer of some number of bits, or a 32-bit floating point number.
        Also some limited meaning can be ascribed to the channels, for example, some can be
        designated as "RED", "ALPHA", "COLOR-MAPPED", etc.  Suppose that an IFD has k channels.
        We divide these channels up into "layers" as follows: 4 channels in sequence that together
        constitute RGBA (the order is not important) form an RGBA layer, 3 channels in sequence
        that consitute an RGB form an RGB layer, and any other channel is a PLAIN layer.  The
        layers are read off in sequence, always taking RGBA over RGB over PLAIN as the next layer.
        The routines below deliver either a specified layer as an array of the appropriate kind,
        type, and scale, or all of the layers in a Layer_Bundle that consists of a sequence
        of pointers to arrays, one per layer in their sequence within the tiff file.  The scale
        of the integer types of an array either give or control the number of bits used for each
        pixel depending on whether you are reading or writing, respectively.

      The simplest read/write interface is provided by Read/Write_Image(s).  Read_Image reads
        a single layer (typically 0) and if the tiff has one IFD it returns a 2D array, otherwise
        it assumes the IFDs are the planes of a stack, and returns a 3D array consisting of the
        planes of the given layer (provided the IFD's all have the same dimensions, number of
        channels, types, etc.) The layer is color-mapped if necessary (this is not automatically
        performed by the Tiff routines further below).  Write_Image writes a tiff that contains
        a single layer accepting either a stack or an image as input.  In the event it is passed
        a stack it writes out a series of IFD's, one for each plane, as per convention.
        Read_Images returns a pointer to a Layer_Bundle that indicates how many layers there
        are and list of 2D or 3D arrays, one per layer, according to whether the tiff file
        contains more than one IFD or not.  Write_Images writes a series of layers, either 2D or
        3D as per the dimensionality of the arrays in the layer list.

      The text field of the 1st layer is written to the JF-TAGGER field upon a write and this tag
        is loaded into the text field of the first layer when read.

      Open_Tiff, Parse_Series_Name, and all Read routines that follow may return NULL if they
        can't open a file or if the tif contents are improperly formated.  In this case, one may
        get a descriptive string of the problem by calling Image_Error(), immediately after the
        error occurs.

      For all the write routines that follow, the flag compress should be set if you want the
        file to be LZW compressed, and off (0) otherwise.  LZW compressing the file takes
        quite a bit more time, by a factor of almost 3, but of course the file ends up generally
        being 30-50% smaller.
  */

char *Image_Error();

typedef struct
  { int     num_layers;    //  Number of layers in layer list
    Array **layers;        //  layers[i] is the image of the i'th layer for i in [0,num_layers-1]
  } Layer_Bundle;

Array *G(Read_Image)(char *file_name, int layer);
void   Write_Image(char *file_name, Array *image, int compress);

Layer_Bundle *G(Read_Images)(char *file_name);
void          Write_Images(char *file_name, Layer_Bundle *images, int compress);

/*
      Somewhat more control and generality is possible using the routines below that open a tiff
        file for reading or writing (Open_Tiff with "r" or "w" as the 2nd argument) and then allow
        you to get out or put in layers of each successive IFD (Read/Write_Tiff(s)) or simply skip
        over them when reading (Advance_Tiff).

      IFD's do *not* have to have the same dimensions or number and types of channels/layers
        as other IFD's in the tif file.  This is sometimes useful if you want to encode something
        more complicated in a tiff, for example, every even-numbered IFD could be a thumbnail of
        the previous odd-numbered IFD.  What is in the sequence of IFD's could even be encoded in
        the text field of the first layer of the first IFD, which is put into or taken from the
        JF-TAGGER field of the tiff (There is only *one* per file).

      If the 1st layer is color-mapped then for this IO interface the layer is not mapped when read
        but simply returned as a plain array.  This set of IO routines maintains an internal color
        map array which we will refer to as C.  If Read_Tiff or Read_Tiffs encounter a color map
        then C is set to this color map, otherwise they set C to NULL.  Has_Map() returns a non-zero
        value of C is currently not NULL.  A copy of C is returned by Get_Map(), or NULL if C is
        NULL.  C can be explicitly set by calling Set_Map with an array map, which get consumed by
        the routine (i.e. the module takes it over and will free it when a new color map comes
        along).  Write_Tiff and Write_Tiffs assume the first layer is color-mapped if C is not NULL
        and write that color map to the tiff file as well as the image.  In summary, if when you
        read a 2D image from a tiff with this interface, you should check, shortly thereafter, if 
        it is color-mapped, and before writing a tiff plane you need to be sure that C is set to
        the desired value doing so directly with Set_Map as the case may be.  The color map can
        be applied to the first layer or any PLAIN layer of the image with the right scale, by
        calling the routine Apply_Map described in array.h.

      It is unfortunate, but the tiff format only allows one color-map per IFD and it assumes
        that if a channel is color mapped than it is the first.  However, there is nothing
        to prevent the application of this map to other channels if it suites your purpose.
        When writing a color-mapped image, make sure the first layer is compatible with the
        map, as the routine will assume that's one of the layers you want to map, and also don't
        forget to set the map, or make sure it is NULL if you do not want a color mapped
        first layer!

      Note carefully, that when you call Read_Tiff with a layer other than 0, that if layer
        0 is color-mapped, the color map in the IFD does result in C being (re)set.
  */

typedef void Tiff;

Tiff   *G(Open_Tiff)(char *file_name, char *mode);
Array  *G(Read_Tiff)(Tiff *tif, int layer);
void    Advance_Tiff(Tiff *M(tif));
void    Rewind_Tiff(Tiff *M(tif));
int     Tiff_EOF(Tiff *tif);
void    Write_Tiff(Tiff *tif, Array *image, int compress);
void    Close_Tiff(Tiff *F(tif));

int     Has_Map();
Array  *Get_Map();
void    Set_Map(Array *C(map));

Layer_Bundle *G(Read_Tiffs)(Tiff *tif);
void          Write_Tiffs(Tiff *tif, Layer_Bundle *images, int compress);

Tiff  *G(Copy_Tiff)(Tiff *tif);
Tiff  *Pack_Tiff(Tiff *R(M(tif)));
Tiff  *Inc_Tiff(Tiff *R(I(tif)));
void   Free_Tiff(Tiff *F(tif));
void   Kill_Tiff(Tiff *K(tif));
void   Reset_Tiff();
int    Tiff_Usage();
void   Tiff_List(void (*handler)(Tiff *));
int    Tiff_Refcount(Tiff *tif);

  /*  Parse_Series_Name parses a file name assuming it is of the form "<prefix><first_num><suffix>"
      and returns the constituent parts where 'num_width' is the number of characters used to
      represent 'first_num'.  A file-based stack is encoded as a sequence of files with names of
      this form where the planes of the stack are in successively numbered files, e.g.
      x001.tif x002.tif ... x037.tif.  The number part is assumed to always involve the
      same number of digits if 'first_num' starts with a leading '0'.  Otherwise, if one is
      reading a file series then the reader will also try finding a sequence where the numbers
      are not padded and use the one that works.  If writing a file series then the numbers
      are written without zero-padding.  The routines Read/Write_Plane(s)_Series are analagous
      to Read/Write_Image(s) save that they take a File_Bundle as a specification of the source
      of the tiff data as opposed to a file name.  Obviously the routines are always aimed at
      3D stacks.
  */

typedef struct        //  a return bundle (not an object)
  { char *prefix;
    char *suffix;     //  bundle names are "%s%0*d%s",prefix,num_width,#,suffix or
    int   padded;     //  possibly "%s%d%s",prefix,#,suffix if padded is 0, tries
    int   num_width;  //  both when reading
    int   first_num;
  } File_Bundle;

File_Bundle *Parse_Series_Name(char *file_name);

Array       *G(Read_Plane_Series)(File_Bundle *bundle, int layer);
void         Write_Plane_Series(File_Bundle *bundle, Array *image, int compress);

Layer_Bundle *G(Read_Planes_Series)(File_Bundle *bundle);
void          Write_Planes_Series(File_Bundle *bundle, Layer_Bundle *images, int compress);

//the following routines are defined by Hanchuan Peng, 2013-11-02
Dimn_Type get_Tiff_Depth_mylib(char *filename);
Layer_Bundle * read_One_Tiff_ZSlice(char * filename,
                                    Dimn_Type zsliceno);


#endif
