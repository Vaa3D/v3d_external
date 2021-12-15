/*****************************************************************************************\
*                                                                                         *
*  Basic Tiff Reader and Writer (Tiff 6.0)                                                *
*    This program reads and writes .tif files without really understanding anything more  *
*    than that it is a header that has a linked lists of image file descriptors (ifd's)   *
*    that contain a collection of tags, one of which is an array pointing to blocks of    *
*    image data (TIFF_TILE_OFFSETS or TIFF_STRIP_OFFSETS) and another of which is an      *
*    array of byte counts for each block (TIFF_TILE_BYTE_COUNTS or TIFF_STRIP_BYTE_COUNTS,*
*    respectively).                                                                       *
*                                                                                         *
*    You can add, delete, and modify tags from each image as you wish.                    *
*                                                                                         *
*    In addition, we have "Tiff_Annotator" routines that allow one to efficiently         *
*      write a TIFF_JF_TAGGER tag that points at a string buffer that is always           *
*      guaranteed to be at the end of the file so that changing it is just a matter of    *
*      rewriting a small suffix of the file.                                              *
*                                                                                         *
*    The routines can also read and write .lsm files, consolidating them as proper tifs   *
*      once read and minimally reintroducing the essential "quirks" that an .lsm reader   *
*      would expect when writing it back out as an .lsm                                   *
*                                                                                         *
*    The package handles a variant designed by us wherein offsets can be 64-bits and so   *
*      files larger than 2Gb are supported.                                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  December 2007                                                                 *
*                                                                                         *
*  (c) July 27, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _TIFF_IO

#define _TIFF_IO

#include <stdlib.h>
#include <stdio.h>

// The Tiff 6.0 data types

typedef enum
  { TIFF_BYTE      = 1,
    TIFF_ASCII     = 2,
    TIFF_SHORT     = 3,
    TIFF_LONG      = 4,
    TIFF_RATIONAL  = 5,
    TIFF_SBTYE     = 6,
    TIFF_UNDEFINED = 7,
    TIFF_SSHORT    = 8,
    TIFF_SLONG     = 9,
    TIFF_SRATIONAL = 10,
    TIFF_FLOAT     = 11,
    TIFF_DOUBLE    = 12,
    TIFF_LONG64    = 13   //  Not part of standard, our extentsion for >2Gb files
  } Tiff_Type;

// The Tiff 6.0 standard tags and when relevant constants for each of their possible values.
//   For each tag in the comment we give the type and the count expected for the tag

#define TIFF_NEW_SUB_FILE_TYPE      254  //  LONG  (1)
#define   TIFF_VALUE_REDUCED_RESOLUTION     0x1
#define   TIFF_VALUE_MULTI_PAGE             0x2
#define   TIFF_VALUE_TRANSPARENCY           0x4
#define TIFF_SUB_FILE_TYPE          255  //  SHORT (1)
#define   TIFF_VALUE_FULL_IMAGE               1
#define   TIFF_VALUE_REDUCED_IMAGE            2
#define   TIFF_VALUE_MULI_PAGE_IMAGE          3
#define TIFF_IMAGE_WIDTH            256  //  SHORT or LONG (1)
#define TIFF_IMAGE_LENGTH           257  //  SHORT or LONG (1)
#define TIFF_BITS_PER_SAMPLE        258  //  SHORT (Samples_Per_Pixel)
#define TIFF_COMPRESSION            259  //  SHORT (1)
#define   TIFF_VALUE_UNCOMPRESSED       1
#define   TIFF_VALUE_CCITT_1D           2
#define   TIFF_VALUE_GROUP_3_FAX        3
#define   TIFF_VALUE_GROUP_4_FAX        4
#define   TIFF_VALUE_LZW                5
#define   TIFF_VALUE_JPEG               7    //  The original Jpeg code 6 is essentially obsolete
#define   TIFF_VALUE_PACK_BITS      32773

#define TIFF_PHOTOMETRIC_INTERPRETATION 262  //  SHORT (1)
#define   TIFF_VALUE_WHITE_IS_ZERO       0
#define   TIFF_VALUE_BLACK_IS_ZERO       1
#define   TIFF_VALUE_RGB                 2
#define   TIFF_VALUE_RGB_PALETTE         3
#define   TIFF_VALUE_TRANSPARENCY_MASK   4
#define   TIFF_VALUE_CMYK                5
#define   TIFF_VALUE_YCBCR               6
#define   TIFF_VALUE_CIELAB              8

#define TIFF_THRESHHOLDING          263  //  SHORT (1)
#define   TIFF_VALUE_NO_DITHER                1
#define   TIFF_VALUE_ORDERED_DITHER           2
#define   TIFF_VALUE_RANDOM_DITHER            3
#define TIFF_CELL_WIDTH             264  //  SHORT (1)
#define TIFF_CELL_LENGTH            265  //  SHORT (1)
#define TIFF_FILL_ORDER             266  //  SHORT (1)
#define   TIFF_VALUE_HIGH_TO_LOW_BITS         1
#define   TIFF_VALUE_LOW_TO_HGIH_BITS         2

#define TIFF_DOCUMENT_NAME          269  //  ASCII
#define TIFF_IMAGE_DESCRIPTION      270  //  ASCII
#define TIFF_MAKE                   271  //  ASCII
#define TIFF_MODEL                  272  //  ASCII

#define TIFF_STRIP_OFFSETS          273  //  SHORT or LONG or LONG64 (Strips_Per_Image)
#define TIFF_ORIENTATION            274  //  SHORT                   (1)
#define   TIFF_VALUE_TOP_N_LEFT               1
#define   TIFF_VALUE_TOP_N_RIGHT              2
#define   TIFF_VALUE_BOTTOM_N_RIGHT           3
#define   TIFF_VALUE_BOTTOM_N_LEFT            4
#define   TIFF_VALUE_LEFT_N_TOP               5
#define   TIFF_VALUE_RIGHT_N_TOP              6
#define   TIFF_VALUE_RIGHT_N_BOTTOM           7
#define   TIFF_VALUE_LEFT_N_BOTTOM            8
#define TIFF_SAMPLES_PER_PIXEL      277  //  SHORT                   (1)
#define TIFF_ROWS_PER_STRIP         278  //  SHORT or LONG           (1)
#define TIFF_STRIP_BYTE_COUNTS      279  //  SHORT or LONG or LONG64 (Strips_Per_Image)

#define TIFF_MIN_SAMPLE_VALUE       280  //  SHORT    (Samples_Per_Pixel) 
#define TIFF_MAX_SAMPLE_VALUE       281  //  SHORT    (Samples_Per_Pixel) 
#define TIFF_X_RESOLUTION           282  //  RATIONAL (1) 
#define TIFF_Y_RESOLUTION           283  //  RATIONAL (1) 

#define TIFF_PLANAR_CONFIGURATION   284  //  SHORT    (1) 
#define   TIFF_VALUE_CHUNKY                   1
#define   TIFF_VALUE_PLANAR                   2

#define TIFF_PAGE_NAME              285  //  ASCII 
#define TIFF_X_POSITION             286  //  RATIONAL (1)
#define TIFF_Y_POSITION             287  //  RATIONAL (1)

#define TIFF_FREE_OFFSETS           288  //  LONG     (any)
#define TIFF_FREE_BYTE_COUNTS       289  //  LONG     (any) 

#define TIFF_GRAY_RESPONSE_UNIT     290  //  SHORT    (1)
#define TIFF_GRAY_RESPONSE_CURVE    291  //  SHORT    (2^Bits_Per_Sample) 
#define TIFF_T4_OPTIONS             292  //  LONG     (1) 
#define TIFF_T6_OPTIONS             293  //  LONG     (1) 
#define TIFF_RESOLUTION_UNIT        296  //  SHORT    (1) 
#define   TIFF_VALUE_NO_ABSOLUTE_UNIT         1
#define   TIFF_VALUE_INCH                     2     //  Default
#define   TIFF_VALUE_CENTIMETER               3
#define TIFF_PAGE_NUMBER            297  //  SHORT    (2) 
#define TIFF_TRANSFER_FUNCTION      301  //  SHORT    (1 or Samples_Per_Pixel)*2^Bits_Per_Sample 

#define TIFF_SOFTWARE               305  //  ASCII 
#define TIFF_DATE_TIME              306  //  ASCII    (20)  Format: "YYYY:MM:DD HH:MM:SS"
#define TIFF_ARTIST                 315  //  ASCII 
#define TIFF_HOST_COMPUTER          316  //  ASCII 

#define TIFF_PREDICTOR              317  //  SHORT    (1) 
#define   TIFF_NO_PREDICTION                  1
#define   TIFF_HORIZONTAL_DIFFERENCING        2
#define TIFF_WHITE_POINT            318  //  RATIONAL (2) 
#define TIFF_PRIMARY_CHROMATICITIES 319  //  RATIONAL (6) 
#define TIFF_COLOR_MAP              320  //  SHORT    (3*2^Bits_Per_Sample) 
#define TIFF_HALFTONE_HINTS         321  //  SHORT    (2) 

#define TIFF_TILE_WIDTH             322  //  SHORT or LONG           (1)
#define TIFF_TILE_LENGTH            323  //  SHORT or LONG           (1) 
#define TIFF_TILE_OFFSETS           324  //  LONG  or LONG64         (Tiles_Per_Image) 
#define TIFF_TILE_BYTE_COUNTS       325  //  SHORT or LONG or LONG64 (Tiles_Per_Image) 

#define TIFF_INK_SET                332  //  SHORT         (1) 
#define TIFF_INK_NAMES              333  //  ASCII         (list of zero-terminatd strings)
#define TIFF_NUMBER_OF_INKS         334  //  SHORT         (1) 
#define TIFF_DOT_RANGE              336  //  BYTE or SHORT (2 or 2*Number_of_Inks) 
#define TIFF_TARGET_PRINTER         337  //  ASCII

#define TIFF_EXTRA_SAMPLES          338  //  BYTE  (Samples_Per_Pixel - Photo_Metric_Implied)
#define   TIFF_VALUE_UNSPECIFIED_DATA         0
#define   TIFF_VALUE_ALPHA_DATA               1
#define   TIFF_VALUE_SOFT_MATTE               2
#define   TIFF_VALUE_EXTRA_RED                3   //  Customized extension, not part of standard
#define   TIFF_VALUE_EXTRA_GREEN              4   //  Customized extension, not part of standard
#define   TIFF_VALUE_EXTRA_BLUE               5   //  Customized extension, not part of standard
#define TIFF_SAMPLE_FORMAT          339  //  SHORT (Samples_Per_Pixel) 
#define   TIFF_VALUE_UNSIGNED_INTEGER_DATA    1
#define   TIFF_VALUE_TWOS_COMPLEMENT_DATA     2
#define   TIFF_VALUE_IEEE_FLOATING_POINT_DATA 3
#define   TIFF_VALUE_UNDEFINED_DATA           4
#define TIFF_SMIN_SAMPLE_VALUE      340  //  Any   (Samples_Per_Pixel) 
#define TIFF_SMAX_SAMPLE_VALUE      341  //  Any   (Samples_Per_Pixel) 
#define TIFF_TRANSFER_RANGE         342  //  SHORT (6) 

#define TIFF_JPEG_PROC                      512  //  SHORT (1) 
#define TIFF_JPEG_INTERCHANGE_FORMAT        513  //  LONG  (1) 
#define TIFF_JPEG_INTERCHANGE_FORMAT_LENGTH 514  //  LONG  (1) 
#define TIFF_JPEG_RESTART_INTERVAL          515  //  SHORT (1) 
#define TIFF_JPEG_LOSSLESS_PREDICTORS       517  //  SHORT (Samples_Per_Pixel)
#define TIFF_JPEG_POINT_TRANSFORMS          518  //  SHORT (Samples_Per_Pixel)
#define TIFF_JPEG_Q_TABLES                  519  //  LONG  (Samples_Per_Pixel)
#define TIFF_JPEG_DC_TABLES                 520  //  LONG  (Samples_Per_Pixel)
#define TIFF_JPEG_AC_TABLES                 521  //  LONG  (Samples_Per_Pixel)

#define TIFF_YCBCR_COEFFICIENTS             529  //  RATIONAL (3) 
#define TIFF_YCBCR_SUBSAMPLING              530  //  SHORT    (2) 
#define TIFF_YCBCR_POSITIONING              531  //  SHORT    (1) 
#define TIFF_REFERENCE_BLACK_WHITE          532  //  LONG     (2*Samples_Per_Pixel) 

#define TIFF_COPYRIGHT                    33432  //  ASCII

#define TIFF_CZ_LSM_INFO                  34412  //  BYTE     (See Zeiss LSM documentation)

#define TIFF_JF_ANO_BLOCK                 36036  //  ASCII

#define TIFF_JF_MRC_INFO                  36037  //  LONG(56) + BYTE(800)

/* A tiff file consists of a header that "points" to a linked list of image file descriptors
   (IFD's).  While it is not required, each IFD typically encodes an image, a Tiff file with
   more than one IFD typically encodes a stack of images.  Each IFD consists of a variable #
   of tags that have a label, type, count, and value.  The standard tag labels and their codes
   are given above, as are the possible range of types.  Each tag encodes an array of count
   values of the given type.  The standard describes how these are encoded in a file, but a
   user of this library is shielded from these details.

   We encounter files which in aggregate are larger than 2Gb and as such the current standard
   cannot handle these files.  We added the option to read and write files with 8-byte offsets.
   These files are marked by having 64 as their magic number in the header, instead of the 42
   that is specified by the standard.  When writing a tiff you have to indicate whether you want
   to use 64-bit offsets or not.  Otherwise it is all transparent, but only supported by this
   package.   In addition, 64-bit byte count tags are also supported if present, implying that
   an individual IFD can have an image greater than 2Gb in size (albeit the width and height
   must fit in a 32-bit unsigned int).

   Native_endian returns a non-zero value if and only if the native machine order is big-endian.
   Tiff_Is_LSM when called with a Tiff_Reader or Tiff_Writer returns a non-zero value only if the
   file was opened as a Zeiss lsm file.  For those routines that have error exits, you can get
   a text string describing the particular error that occured by calling Tiff_Error_String, and
   you can get a string naming the routine that detected the error by calling Tiff_Error_Source.
   The strings are managed by the package and should not be freed by the user.
*/

int   Native_Endian();
int   Tiff_Is_LSM(void *tif);

char *Tiff_Error_Source();
char *Tiff_Error_String();

/* One can open a tif file with Open_Tiff_Reader creating a Tiff_Reader object that can be used
   to access the file's IFD's sequentially with Read_Tiff_IFD.  The routine returns NULL if
   the file cannot be read or the contents of the file are not that of a Tiff file.  One can skip
   over the next IFD with Advance_Tiff_Reader, start accessing IFD's from the beginning again with
   Rewind_Tiff_Reader, and can test if any IFD's remain with End_Of_Tiff.  Advance_Tiff_Reader
   returns a non-zero value if the Tiff is not properly encoded or is at the last IFD, and
   End_OF_Tiff returns a non-zero value if the last IFD has been read.

   Data blocks can be compressed shorts or ints in an endian order dependent on the producing
   machine.  Presumably the endian indicator of the TIFF file is correct for that data (either
   that or *somebody elses* reader/writer has already blown it :-))   Therefore, Open_Tiff_Reader
   returns the endian indicator "big_endian" for the image data.  All IFD's read from this file
   will assume that this is the endian order of the image data and a Tiff_Writer will write
   all the data out in this endian in order to guarantee the correct downstream interpreation
   of the data.  Note carefully, that all tags in an IFD are in native endian order while in
   memory (they are flipped on read and/or write as necessary).  If you create a tif by creating
   and writing a series of IFD's, then the tiff endian will be that which is native to the
   producing machine (Native_Endian returns 0 iff native order is little-endian).

   Open_Tiff_Reader also returns "flag64" which is set if and only if 64-bit offsets are used
   within the file.  An lsm file, see below, is assumed never to use a 64-bit encoding.

   LSM FILES:

   If the Tiff_Reader is opened for an LSM file, by passing a non-zero value for the parameter
   lsm to Open_Tiff_Reader, then it assumes it is reading an lsm file as encoded by the Zeiss
   spec if ROWS_PER_STRIP is not defined.  In this case it restores the encoding to that of a
   true tiff by removing the following "warts":
     (a) the BITS_PER_PIXEL vector is placed in the value field of the tag if it fits
     (b) the color map is shifted to use the high-order bytes if the low-order bytes were used,
     (c) a tag for ROWS_PER_STRIP is added, 
     (d) the CZ_LSM_INFO tag is a block that points at other blocks that are not necessarily
            contiguous.  These are consolidated into a single block, the count-field of *the*
            tag is set to the size of this block and all sub-block offsets become relative to
            this super-block (as opposed to the start of the file), and
     (e) STRIP_BYTE_COUNTs are accurately computing the lenght of the lzw encoding of each strip
            if the lsm is compressed.
   Fix (e) takes a bit of extra time but is necessary if one is to strictly adhere to the standard.
   If the Tiff_Reader is for an lsm but ROWS_PER_STRIP is defined, then the package assumes
   it is reading an LSM file that it wrote (see Tiff_Writer below) in which case only wart
   (a) remains and needs to be handled when reading the .lsm.
*/

typedef void Tiff_Reader;

Tiff_Reader *Open_Tiff_Reader(char *name, int *big_endian, int *flag64, int lsm);
void         Rewind_Tiff_Reader(Tiff_Reader *tif);
int          Advance_Tiff_Reader(Tiff_Reader *tif);
int          End_Of_Tiff(Tiff_Reader *tif);

Tiff_Reader *Copy_Tiff_Reader(Tiff_Reader *tif);
Tiff_Reader *Pack_Tiff_Reader(Tiff_Reader *tif);
Tiff_Reader  *Inc_Tiff_Reader(Tiff_Reader *tif);
void         Free_Tiff_Reader(Tiff_Reader *tif);
void         Kill_Tiff_Reader(Tiff_Reader *tif);
void         Reset_Tiff_Reader();
int          Tiff_Reader_Usage();
void         Tiff_Reader_List(void (*handler)(Tiff_Reader *));
int          Tiff_Reader_Refcount(Tiff_Reader *tif);

/* One can begin the creation of a tif file with Open_Tiff_Writer that creates a Tiff_Writer
   object.  NULL is return if the file could not be opened for writing.  The file will be written
   with 64-bit offsets if the parameter flag64 is non-zero, and the file will be written as
   an lsm file if the parameter lsm is non-zero.  It is an error to have both of these parameters
   be non-zero.  One writes successive IFD's with Write_Tiff_IFD, and one must take care to close
   the writer by either killing it or freeing it before terminating your program.

   While the tags may not be in ascending order of labels internaly, Write_Tiff_IFD, sorts them
   before writing so that a standard tiff results from a write.  If a Tiff_Writer is
   asked to write a .lsm file, then Write_Tiff_IFD reintroduces wart (a) as it writes out the file,
   so that other ".lsm" readers (such as Image J or Zeiss's software) can open it.  The fixing of
   the other warts should not affect ".lsm" readers (confirmed for Image J).
*/

typedef void Tiff_Writer;

Tiff_Writer *Open_Tiff_Writer(char *name, int flag64, int lsm);

Tiff_Writer *Copy_Tiff_Writer(Tiff_Writer *tif);
Tiff_Writer *Pack_Tiff_Writer(Tiff_Writer *tif);
Tiff_Writer  *Inc_Tiff_Writer(Tiff_Writer *tif);
void         Free_Tiff_Writer(Tiff_Writer *tif);
void         Kill_Tiff_Writer(Tiff_Writer *tif);
void         Reset_Tiff_Writer();
int          Tiff_Writer_Usage();
void         Tiff_Writer_List(void (*handler)(Tiff_Writer *));
int          Tiff_Writer_Refcount(Tiff_Writer *tif);

/* The next IFD can be read from an open Tiff_Reader with Read_Tiff_IFD.  The read only brings
   into memory the IFD header block and its tags, it does not bring the (possibly packed,
   differenced, and/or compressed) tile or strip blocks that encode the image.  Instead the
   IFD simply refers to the Tiff_Reader so that the image data can be downloaded when and if
   it is needed.

   Write_Tiff_IFD writes an IFD and its image data as the next record on the given Tiff_Writer.
   As mentioned immediately above, the image data for an IFD read from a Tiff_Reader is still on
   the file being read.  In this case, the image data is simply transfered from the reader's file
   to the writer's file without any decoding, compressing, etc. taking place as it is unecessary.
   The other possibility is that the IFD was created directly in memory with Create_Tiff_IFD (see
   below), in which case the image data is also in memory in byte-padded, decoded pixel arrays
   as per the channel description given in tiff.image.h.  In this case, the image data is encoded
   (as specified by the tags of the IFD) on the fly as it written to the writer's file using a
   finite amount of auxiliary buffering memory (1Mb).  The only restriction is that the IFD
   cannot specifiy TIFF_VALUE_CHUNKY for the TIFF_PLANAR_CONFIGURATION, our writer only handles
   TIFF_VALUE_PLANAR.  If chunky is specified Write_Tiff_IFD makes an error return.

   Create_Tiff_IFD creates an IFD that initially has room for "num_tags" tags and where the image
   offsets are with respect to memory location "data".  Please see "tiff.image.h" for a description
   of how an in-memory image plane is expected to be coded.  You are responsible for building a
   properly constructed set of tags and values.

   Originally, the package used to be simpler and just keep copies of the image data, but in
   order to give you a package that is capable of processing very large files with the best
   possible memory efficiency, we are forced to make you, the user, responsible for the space
   that will hold the in-memory copy of an image.  Sorry, no way around it.

   The label, count, type, and value of a tag can be fetched from an IFD with Get_Tiff_Tag
   and set with Set_Tiff_Tag.  Set_Tiff_Tag returns a non-zero value if the count is not
   positive or the type is not a valid Tiff type or if it runs out of memory trying to allocate
   a tag of that size.  If the tag doesn't currently exist in the IFD when it is set, then it
   is automatically added, and if it doesn't exist when it is fetched then NULL is returned.
   Allocate_Tiff_Tag does what Set_Tiff_Tag does save for actually setting the value of the
   tag and returning a pointer to the vector for the tags value(s).  This allows you to see
   if there is enough memory to allocate it, and then set it if everything is OK.  Finally,
   one can delete an existing tag with Delete_Tiff_Tag.

   Count_Tiff_Tags returns the number of tags in ifd, and Get_Tiff_Label returns the label of
   the idx^th tag stored in ifd.  In this way one can query an ifd to get every tag within it.

   Read_Tiff_IFD returns NULL if the next IFD in the file is improperly formatted.  Write_Tiff_IFD
   returns a non-zero value if it writes multiple IFD's and they do not all have the same
   endian order for the image data, or if it has been instructed to write an lsm and can't find
   Zeiss's tag in the first IFD.
*/

typedef void Tiff_IFD;

Tiff_IFD    *Read_Tiff_IFD(Tiff_Reader *tif);
int          Write_Tiff_IFD(Tiff_Writer *tif, Tiff_IFD *ifd);
void         Print_Tiff_IFD(Tiff_IFD *ifd, FILE *output, int indent);
Tiff_IFD    *Create_Tiff_IFD(int num_tags, void *image);

void        *Get_Tiff_Tag(Tiff_IFD *ifd, int label, Tiff_Type *type, int *count);
void        *Allocate_Tiff_Tag(Tiff_IFD *ifd, int label, Tiff_Type type, int count);
int          Set_Tiff_Tag(Tiff_IFD *ifd, int label, Tiff_Type type, int count, void *data);
void         Delete_Tiff_Tag(Tiff_IFD *ifd, int label);

int          Count_Tiff_Tags(Tiff_IFD *ifd);
int          Get_Tiff_Label(Tiff_IFD *ifd, int idx);

Tiff_IFD    *Copy_Tiff_IFD(Tiff_IFD *ifd);
Tiff_IFD    *Pack_Tiff_IFD(Tiff_IFD *ifd);
Tiff_IFD     *Inc_Tiff_IFD(Tiff_IFD *ifd);
void         Free_Tiff_IFD(Tiff_IFD *ifd);
void         Kill_Tiff_IFD(Tiff_IFD *ifd);
void         Reset_Tiff_IFD();
int          Tiff_IFD_Usage();
void         Tiff_IFD_List(void (*handler)(Tiff_IFD *));
int          Tiff_IFD_Refcount(Tiff_IFD *ifd);

/* In order to facilitate annotating a file this library supports a special tag, TIFF_JF_ANO_BLOCK,
   that holds an ASCII array of arbitrary length that presumably contains an encoding a some
   annotation of the image.  In order to facilitate rapid reading and writing of this tag
   independent of the total file size, the tag must be in the first IFD and its value must
   constitute a suffix of the file (i.e. be located at the end of the file).  To this end
   the Tiff_Writer routines are guaranteed to place the value of a TIFF_JF_TAGGER tag in the
   *first* IFD at the end of the file.

   One can open a tiff for the purposes of getting or setting its annotation by creating a
   Tiff_Annotator object with Open_Tiff_Annotator.  NULL is returned if either the file is
   not read/write permitted, the format is not recognizable, or *** there is no annotation tag
   in the first IFD or its value is not a suffix of the file ***.  In such an event, you can
   request that the file be "formatted for annotation" by calling Format_Tiff_For_Annotation.
   This is an expensive operation requiring time proportional to the length of the file.  It
   returns a non-zero value if there is an error (file not read/write permitted or format not
   recognizable).  Generally you want to know why a file can't be opened for annotation and
   Tiff_Annotation_Status returns that a file is not r+ openable, is not a tiff (as far as it
   can tell), is a tiff but not formated, or is good to go.

   Given that Open_Tiff_Annotator succeeded, you can get the annotation with Get_Tiff_Annotation
   and set it with Set_Tiff_Annotation (which returns a non-zero value iff there was an error).
   Note carefully that the annotation is not a C string but simply an array of chars of some
   length, so that you can concatenate as many C-strings into the value of an annotation as
   you wish.

   The Tiff_Annotator must be closed if you want any changes to the annotation to be saved.
   This occurs when the Tiff_Annotator is either freed or killed, so be sure to "dispose" of
   it before your program exits.
*/

typedef enum
  { ANNOTATOR_CANT_OPEN     = 0,
    ANNOTATOR_GIBBERISH     = 1,
    ANNOTATOR_NOT_FORMATTED = 2,
    ANNOTATOR_FORMATTED     = 3
  } Annotator_Status;

Annotator_Status Tiff_Annotation_Status(char *name);

typedef void Tiff_Annotator;

Tiff_Annotator *Open_Tiff_Annotator(char *name);
char           *Get_Tiff_Annotation(Tiff_Annotator *tif, int *length);
int             Set_Tiff_Annotation(Tiff_Annotator *tif, char *anno, int length);
void            Close_Tiff_Annotator(Tiff_Annotator *tif);
int             Format_Tiff_For_Annotation(char *name);

Tiff_Annotator *Copy_Tiff_Annotator(Tiff_Annotator *tif);
Tiff_Annotator *Pack_Tiff_Annotator(Tiff_Annotator *tif);
Tiff_Annotator  *Inc_Tiff_Annotator(Tiff_Annotator *tif);
void            Free_Tiff_Annotator(Tiff_Annotator *tif);
void            Kill_Tiff_Annotator(Tiff_Annotator *tif);
void            Reset_Tiff_Annotator();
int             Tiff_Annotator_Usage();
void            Tiff_Annotator_List(void (*handler)(Tiff_Annotator *));
int             Tiff_Annotator_Refcount(Tiff_Annotator *ifd);

/* While this reader/writer can read and write LSM's this was done primarily so that we could
   continue to use ImageJ.  But ImageJ has also been fixed, at our request (version 1.39q),
   so that it can read compressed RGB tiffs.  So to get completely away from .lsm, one can
   convert an encoding of a <= 3-channel lsm or tiff to either a grey-scale tiff if there is
   1 channel, and an RGB tiff if there are 2 or 3 channels.  The routine returns NULL if the
   current IFD is for a thumb-nail, otherwise it returns the IFD save that in the case it is
   for a 2-channel image, it addes a third channel to make it an RGB image.  In this case,
   channel source (= 0 or 1) is mapped to channel target (= 0(R) 1(G) or 2(B)) and the other
   two RGB channels assume the values of channel 1-source.

   To help one decide how to map channels in the 2-channel case, Get_LSM_Colors returns the
   number of channels in the integer pointed at by nchannels, and an array to the rgb colors
   as its result.  The array is static to Get_LSM_Colors and reset on each call.
*/

Tiff_IFD *Convert_2_RGB(Tiff_IFD *ifd, int source, int target);

int      *Get_LSM_Colors(Tiff_IFD *ifd, int *nchannels);

#endif

/******** LIST OF ERRORS **************************************************************

  Open_Tiff_Reader
      Out of memory
      File ends prematurely
      Cannot open file ? for reading
      File ? does not contain valid endian value
      File ? does not contain a valid magic key
      File ? cannot be an lsm file as it has 64-bit offsets

  Advance_Tiff_Reader
      File ends prematurely
      Trying to advance at end-of-ifd-list
      Seek for next IFD failed

  Read_Tiff_IFD
      Out of memory
      File ends prematurely
      Trying to read when at end-of-ifd-list
      Seek for next IFD failed
      Illegal type, ?, in IFD tag
      Type, ?, of image offsets is not correct
      Type, ?, of image byte counts is not correct
      Seek to tag value vector failed
      IFD does not contain a byte_count tag
      IFD does not contain an offset tag
      IFD does not contain compatible offset and byte_count tags
      Offset and byte_count tags do not have the same length
      LSM tag does not start with valid magic key
      Seek to lsm sub-block failed
      Seek to lsm sub-block for scan information failed
      Read within lsm scan information sub-block failed
      Seek within lsm scan information sub-block failed
      Lsm file does not have image_length tag

  Create_Tiff_IFD
      Out of memory

  Get_Tiff_Tag
      No ? tag in IFD

  Allocate_Tiff_Tag
  Set_Tiff_Tag
      Out of memory
      Non-positive count
      Invalid Type, ?

  Get_Tiff_Label
      Index is out of range

  Open_Tiff_Writer
      Out of memory
      Cannot open file ? for writing
      An lsm file cannot have 64-bit offsets

  Write_Tiff_IFD
      Out of memory
      Writing IFD's with different endians to same file
      First IFD does not contain an LSM info tag
      # of bits in a sample is greater than 32
      Seek to image block failed

      For X in { image_width, image_height, tile_width, tile_height }
        IFD does not contain a tag for X
        Tag X of IFD does not have a count of 1
        Tag X of IFD is not of type SHORT or LONG

      For X in { samples_per_pixel, orientation, compression, prediction, rows_per_strip,
                 planar_configuration }
        Tag X of IFD does not have a count of 1
        Tag X of IFD is not of type SHORT or LONG

      Bits_per_sample tag is not of type SHORT
      Length of bits_per_sample tag is less than samples_per_pixel
      IFD does not contain a tag for byte_count
      IFD does not contain a tag for offset
      IFD does not contain compatible offset and byte_count tags
      Length of byte_count tag does not equal # of tiles/strips
      Length of offset tag does not equal # of tiles/strips
      Offset and byte_count tags do not have the same length
      Support only uncompressed, Packbits, and LZW compressed images
      Cannot write IFDs with TIFF_PLANAR_CONFIGURATION = TIFF_VALUE_CHUNKY

  Open_Tiff_Annotator
      Out of memory
      File ends prematurely
      Cannot open file ? for annotating
      File ? does not contain valid endian value
      File ? does not contain a valid magic key
      Trying to advance at end-of-ifd-list
      Seek for next IFD failed
      JF annotation block is not at end of file
      First IFD does not contain a JF annotation block

  Set_Tiff_Annotation
      Out of memory

  Format_Tiff_For_Annotation
      Out of memory
      Empty tiff file
      Could not create temporary file

  Convert_2_RGB
      Out of memory
      No SAMPLES_PER_PIXEL tag in IFD
      No BITS_PER_SAMPLE tag in IFD
      No BYTE_COUNTS tag in IFD
      No (STRIP|TILE)_OFFSETS tag in IFD
      IFD OFFSET & BYTE_COUNT tags not the same size
      IFD contains more than 3 channels
      Source and target not in ranges 0..1 and 0..2 respectively

  Get_LSM_Colors
      Out of memory
      IFD does not contain LSM tag

 ***********************************************************************************/
