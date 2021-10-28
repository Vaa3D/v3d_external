/*****************************************************************************************\
*                                                                                         *
*  Tiff Image Coder and Decoder (Tiff 6.0)                                                *
*    The module allows one to extract the images in a tiff IFD into Tiff_Image that is    *
*    eseentially a set of arrays, one per channel (sample).  The module also allows you   *
*    to build a Tiff_Image and convert it back to a tiff IFD.                             *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  February 2008                                                                 *
*                                                                                         *
*  (c) July 27, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _TIFF_IMAGE

#define _TIFF_IMAGE

#include <stdlib.h>
#include <stdio.h>

#include "tiff.io.h"

//  A Tiff_Image consists of its dimensions and an array of pointers to Tiff_Channel objects
//   which it 'owns' (i.e., they are freed, killed, etc. when the Tiff_Image is freed, killed, ...)
//   There are samples_per_pixel channels, each of which has a row-major ordered array of the
//   pixel values for a given sample and some meta-data such as the # of bits in the values,
//   the # of bytes used to hold the values, how the value should be interpreted, and so on.
//   See the comments next to each field in the definition of Tiff_Channel below.  Regardless
//   of the orientation within a tiff file, the planes of a Tiff_Image are arranged so that
//   (0,0) is always in the upper-left corner.  The memory for a Tiff_Channel array (pointed at
//   by the filed 'plane') is not owned by the object, but is supplied and managed by the user,
//   the goal being to allow for the minimization of the movement of the data within a program
//   when images are large.

typedef enum
  { CHAN_WHITE,   //  0 is imaged as white and the maximum value as black
    CHAN_BLACK,   //  0 is imaged as black and the maximum value as white
    CHAN_MAPPED,  //  Values are mapped to 48-bit RGB triples via the image's color map
    CHAN_RED,     //  Values are for the red-component of an RGB rendering
    CHAN_GREEN,   //  Values are for the green-component of an RGB rendering
    CHAN_BLUE,    //  Values are for the blue-component of an RGB rendering
    CHAN_ALPHA,   //  Premultiplied opacity for each pixel
    CHAN_MATTE,   //  Unassociated (not premultiplied) opacity for each pixel
    CHAN_MASK,    //  0-1 values provide a transparency mask to be applied to other channels
    CHAN_OTHER    //  No interpretation is provided by the tiff IFD
  } Channel_Meaning;

typedef enum
  { CHAN_UNSIGNED,   //  The bits of a value are to be interpreted as an unsigned integer
    CHAN_SIGNED,     //  The bits ... as a 2's complement signed integer
    CHAN_FLOAT       //  The bits ... as an IEEE floating point value
  } Channel_Type;

typedef struct
  { int                bitshift;     // The largest non-zero bit position in the max value
    unsigned long long total;        // Sum of all counts in the histogram
    unsigned long long counts[512];  // Bitshift is at least 1/2*max value, giving 8-bit
  } Tiff_Histogram;                  //   precision when max value >= 256

typedef struct
  { unsigned int    width;            //  The width of the channel
    unsigned int    height;           //  The height of the channel
    Channel_Meaning interpretation;   //  Any interpretation hint (if any) provided by the tiff
    int             scale;            //  The # of bits per value in this channel
    int             bytes_per_pixel;  //  The # of bytes each value is stored in (1, 2, or 4)
    Channel_Type    type;             //  The nature of the values
    void           *plane;            //  A width x height array of the values in row major order
                                      //    This memory is supplied by the user
    Tiff_Histogram *histogram;        //  Histogram of channel values (NULL if not computed)
  } Tiff_Channel;

typedef struct
  { unsigned int    width;            //  The width of every channel plane
    unsigned int    height;           //  The height of every chanel plane
    int             number_channels;  //  The number of channels (samples_per_pixel)
    Tiff_Channel  **channels;         //  [0..number_channels-1] gives the Channel object for each
    unsigned short *map;              //  The color map for the 1st channel if it is CHAN_MAPPED
  } Tiff_Image;

/* For those routines below that have error exits, you can get a text string describing the
   particular error that occured by calling Tiff_Error_String, and the name of the routine
   that caused the error with Tiff_Error_Source as documented in tiff.io.h.
*/

/* The user is responsible for providing the memory for the planes of a tiff image.  This
   often requires that the user see the specification of the Tiff_Image before loading the
   planes of the channels.  So the routine Get_Tiff_Image generates a Tiff_Image for the given
   IFD save that all channel plane fields point at NULL, i.e., they are not loaded.  A NULL
   pointer is returned if the IFD is not semantically valid.  The user can then allocate space
   for the planes (if necessary) and then call Load_Tiff_Image_Planes to load a selected subset
   of the planes of the image into the allocated space.  The memory for the plane for channel
   i is assumed to be at planes[i].  If this is NULL then the channel's plane is not loaded.
   The plane fields of loaded channels are set to plane[i].  The routine returns a non-zero
   value if the source IFD for the image is not semantically valid, but in general this would
   only occur if you changed the ifd from which the image was created.  Remove_Unloaded_Channels
   is a simple utility that removes every Tiff_Channel (renumbering them in the process) that
   is not loaded, i.e. it plane field is NULL, in a given Tiff_Image.
*/

Tiff_Image *Get_Tiff_Image(Tiff_IFD *ifd);

int Load_Tiff_Image_Planes(Tiff_Image *image, void **planes);

void Remove_Unloaded_Channels(Tiff_Image *image);

/* A new Tiff_Image object, initially with no channels, is generated by calling Create_Tiff_Image.
   Subsequent requests to add channels with Add_Tiff_Image_Channel, add channels as per the
   parameters and with the width and height of the image, and with the channel plane, suppled
   by the user, assumed to be pointed at by the parameter "plane".  Add_Tiff_Image_Channel returns
   a non-zero value if and only if an error occured, and Create_Tiff_Image returns NULL if an
   error occured.
*/

Tiff_Image *Create_Tiff_Image(unsigned int width, unsigned int height);
int         Add_Tiff_Image_Channel(Tiff_Image *image, Channel_Meaning meaning, int scale,
                                                      Channel_Type type, void *plane);

// Makes a Tiff_IFD object that encodes the Tiff_Image passed to it, compressing it (or not)
// according to the defined constants available for the parameter compression.  In addition
// you control the tiling of the encoded image by specifying a tile width and tile height.
// If the tile width and tile height are the same as that of the image, then no striping or
// tiling takes place.  If the tile width is the same as the image, but the tile height is
// not, then the image is striped with rows_per_strip set to tile_height.  If both the tile
// width and height do not match those of the image, then the image will be tiled in blocks
// of dimension tile_width x tile_height.  As a convenience, if tile_width or tile_height is
// zero or one, then the dimension is considered to be that of the image in the logic above.
//
// Recall from tiff.io.h that the image is not encoded at this time.  The IFD offsets are
// to location in the planes of the in-memory image relative to the origin of the plane with
// the lowest phsyical address, and the IFD byte_counts are the number of bytes in the byte-aligned,
// unpacked, uncompressed, pixel values in the Tiff_Image.  The encoding of the image takes place
// at the time you request writing the IFD with a Tiff_Writer object.

typedef enum
  { DONT_COMPRESS     = 0,
    LZW_COMPRESS      = 1,
    PACKBITS_COMPRESS = 2
  } Tiff_Compress;

Tiff_IFD *Make_IFD_For_Image(Tiff_Image *image, Tiff_Compress compression,
                             unsigned int tile_width, unsigned int tile_height);

// Scale_Tiff_Channel scales the values in the given channel to have the given number of bits.
//   Range_Tiff_Channel returns the min and max value in the given channel in the two integers
//   pointed at by minval and maxval.  Shift_Tiff_Channel scales the channel by left shifting the
//   values in the given channel by the indicated number of bits, or if shift is negative then
//   it right-shifts the values by the magnitude of shift.
//
// Note carefully that the user is responsible for making sure that the channel's plane pointer
//   is pointer at a block of memory large enout to hold the resulting plane when a scale is
//   performed that requires a larger pixel size.
//
// Call the image versions to scale or shift *all* channels in the image

void Scale_Tiff_Channel(Tiff_Channel *channel, int scale);
void Range_Tiff_Channel(Tiff_Channel *channel, double *minval, double *maxval);
void Shift_Tiff_Channel(Tiff_Channel *channel, int shift);

void Scale_Tiff_Image(Tiff_Image *image, int scale);
void Shift_Tiff_Image(Tiff_Image *image, int shift);

// The histogram for each channel is initially NULL.  To have one computed for a given channel
//   call Histogram_Tiff_Channel, and to have one computed for every channel call the image
//   version.  Tiff_Histogram_Merge builds a histogram for the union of h1 and h2 and places
//   it in h1.  This is useful if you need the histogram of values for a stack or 3D image.

Tiff_Histogram *Histogram_Tiff_Channel(Tiff_Channel *channel);
int             Histogram_Tiff_Image(Tiff_Image *image);
void            Tiff_Histogram_Merge(Tiff_Histogram *h1, Tiff_Histogram *h2);

// The standard object operators (see README)

Tiff_Channel *Copy_Tiff_Channel(Tiff_Channel *channel);
Tiff_Channel *Pack_Tiff_Channel(Tiff_Channel *channel);
Tiff_Channel  *Inc_Tiff_Channel(Tiff_Channel *channel);
void          Free_Tiff_Channel(Tiff_Channel *channel);
void          Kill_Tiff_Channel(Tiff_Channel *channel);
void          Reset_Tiff_Channel();
int           Tiff_Channel_Usage();
void          Tiff_Channel_List(void (*handler)(Tiff_Channel *));
int           Tiff_Channel_Refcount(Tiff_Channel *channel);


Tiff_Image *Copy_Tiff_Image(Tiff_Image *image);
Tiff_Image *Pack_Tiff_Image(Tiff_Image *image);
Tiff_Image  *Inc_Tiff_Image(Tiff_Image *image);
void        Free_Tiff_Image(Tiff_Image *image);
void        Kill_Tiff_Image(Tiff_Image *image);
void        Reset_Tiff_Image();
int         Tiff_Image_Usage();
void        Tiff_Image_List(void (*handler)(Tiff_Image *));
int         Tiff_Image_Refcount(Tiff_Image *image);


Tiff_Histogram *Copy_Tiff_Histogram(Tiff_Histogram *histogram);
Tiff_Histogram *Pack_Tiff_Histogram(Tiff_Histogram *histogram);
Tiff_Histogram  *Inc_Tiff_Histogram(Tiff_Histogram *histogram);
void            Free_Tiff_Histogram(Tiff_Histogram *histogram);
void            Kill_Tiff_Histogram(Tiff_Histogram *histogram);
void            Reset_Tiff_Histogram();
int             Tiff_Histogram_Usage();
void            Tiff_Histogram_List(void (*handler)(Tiff_Histogram *));
int             Tiff_Histogram_Refcount(Tiff_Histogram *histogram);

#endif _TIFF_IMAGE

/******** LIST OF ERRORS **************************************************************

  Get_Tiff_Image

      Sample_format tag is not of type SHORT
      Sample_format tag is not of length samples_per_pixel
      Floating point pixels must be 32-bits
      RGB image has less than 3 samples_per_pixel
      Color_map tag is missing
      Color_map tag is not of type SHORT
      Color_map over values with more than 16-bits
      Color_map length does not match value range
      Do not support CMYK, YcBcR, or CIE Lab photometric types
      Extra_samples tag has wrong number of samples
      Extra_samples tag is not of type BYTE

      < also all error messages of Load_Tiff_Image_PLanes >

  Get_Tiff_Image
  Load_Tiff_Image_Planes
      Out of memory

      For X in { image_width, image_height, photometric_interpretation,
                  tile_width, tile_height }

        IFD does not contain a tag for X
        Tag X of IFD does not have a count of 1
        Tag X of IFD is not of type SHORT or LONG

      For X in { samples_per_pixel, orientation,
                 compression, prediction, rows_per_strip, planar_configuration }

        Tag X of IFD does not have a count of 1
        Tag X of IFD is not of type SHORT or LONG

      Bits_per_sample tag is not of type SHORT
      Length of bits_per_sample tag is less than samples_per_pixel
      # of bits in a sample is greater than 32
      IFD does not contain a tag for byte_count
      IFD does not contain a tag for offset
      IFD does not contain compatible offset and byte_count tags
      Length of byte_count tag does not equal # of tiles/strips
      Length of offset tag does not equal # of tiles/strips
      Offset and byte_count tags do not have the same length
      Support only uncompressed, Packbits, and LZW compressed images

  Make_IFD_For_Image
      Out of memory
      Plane for channel X is NULL

  Create_Tiff_Image
  Add_Tiff_Image_Channel
  Histogram_Tiff_Channel
  Histogram_Tiff_Image
      Out of memory

 ***********************************************************************************/
