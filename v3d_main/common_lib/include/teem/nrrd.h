/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef NRRD_HAS_BEEN_INCLUDED
#define NRRD_HAS_BEEN_INCLUDED

#include <errno.h>
#include <stddef.h>      /* for ptrdiff_t */

/* ---- BEGIN non-NrrdIO */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <float.h>

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>

#include "nrrdDefines.h"
#include "nrrdMacros.h"
#include "nrrdEnums.h"

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(nrrd_EXPORTS) || defined(teem_EXPORTS)
#    define NRRD_EXPORT extern __declspec(dllexport)
#  else
#    define NRRD_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define NRRD_EXPORT extern
#endif
/* ---- END non-NrrdIO */


#ifdef __cplusplus
extern "C" {
#endif

#define NRRD nrrdBiffKey

/*
******** NrrdAxisInfo struct
**
** all the information which can sensibly be associated with
** one axis of a nrrd.  The only member which MUST be explicitly
** set to something meaningful is "size".
**
** If an axis lies conceptually along some direction in an enclosing
** space of dimension nrrd->spaceDim, then the first nrrd->spaceDim
** entries of spaceDirection[] must be non-NaN, and min, max, spacing,
** and units must NOT be set;  thickness, center, and label can still
** be used.  The mutual exclusion between axis-aligned and general
** direction information is enforced per-axis, not per-array.
**
** The min and max values give the range of positions "represented"
** by the samples along this axis.  In node-centering, "min" IS the
** position at the lowest index.  In cell-centering, the position at
** the lowest index is between min and max (a touch bigger than min,
** assuming min < max).
**
** There needs to be a one-to-one correspondence between these variables
** and the nrrdAxisInfo* enum (nrrdEnums.h), the per-axis header fields
** (see nrrdField* enum in nrrdEnums.h), and the various methods in axis.c
*/
typedef struct {
  size_t size;              /* number of elements along each axis */
  double spacing;           /* if non-NaN, distance between samples */
  double thickness;         /* if non-NaN, nominal thickness of region
                               represented by one sample along the axis. No
                               semantics relative to spacing are assumed or
                               imposed, and unlike spacing, there is no
                               sensible way to alter thickness- it is either
                               copied (as with cropping and slicing) or set to
                               NaN (when resampled). */
  double min, max;          /* if non-NaN, range of positions spanned by the
                               samples on this axis.  Obviously, one can set
                               "spacing" to something incompatible with min
                               and max: the idea is that only one (min and
                               max, or spacing) should be taken to be
                               significant at any time. */
  double spaceDirection[NRRD_SPACE_DIM_MAX];
                            /* the vector, in "space" (as described by
                               nrrd->space and/or nrrd->spaceDim), from one
                               sample to the next sample along this axis.  It
                               is the column vector of the transform from
                               index space to "space" space */
  int center;               /* cell vs. node centering (value should be one of
                               nrrdCenter{Unknown,Node,Cell} */
  int kind;                 /* what kind of information is along this axis
                               (from the nrrdKind* enum) */
  char *label,              /* short info string for each axis */
    *units;                 /* string identifying the unit */
} NrrdAxisInfo;

/*
******** Nrrd struct
**
** The struct used to wrap around the raw data array
*/
typedef struct {
  /*
  ** NECESSARY information describing the main array.  This is
  ** generally set at the same time that either the nrrd is created,
  ** or at the time that the nrrd is wrapped around an existing array
  */

  void *data;                       /* the data in memory */
  int type;                         /* a value from the nrrdType enum */
  unsigned int dim;                 /* the dimension (rank) of the array */

  /*
  ** All per-axis specific information
  */
  NrrdAxisInfo axis[NRRD_DIM_MAX];  /* axis[0] is the fastest axis in the scan-
                                       line ordering, the one who's coordinates
                                       change the fastest as the elements are
                                       accessed in the order in which they
                                       appear in memory */

  /*
  ** Optional information descriptive of whole array, some of which is
  ** meaningfuly for only some uses of a nrrd
  */
  char *content;                    /* brief account of what this data is */
  char *sampleUnits;                /* units of measurement of the values
                                       stored in the array itself (not the
                                       array axes and not space coordinates).
                                       The logical name might be "dataUnits",
                                       but that's perhaps ambiguous.  Note that
                                       these units may apply to non-scalar
                                       kinds (e.g. coefficients of a vector
                                       have the same units) */
  int space;                        /* from nrrdSpace* enum, and often
                                       implies the value of spaceDim */
  unsigned int spaceDim;            /* if non-zero, the dimension of the space
                                       in which the regular sampling grid
                                       conceptually lies.  This is a separate
                                       variable because this dimension can be
                                       different than the array dimension.
                                       The non-zero-ness of this value is in
                                       fact the primary indicator that space
                                       and orientation information is set.
                                       This identifies the number of entries in
                                       "origin" and the per-axis "direction"
                                       vectors that are taken as meaningful */
  char *spaceUnits[NRRD_SPACE_DIM_MAX];
                                    /* units for coordinates of space */
  double spaceOrigin[NRRD_SPACE_DIM_MAX];
                                    /* the location of the center the first
                                       (lowest memory address) array sample,
                                       regardless of node-vs-cell centering */
  double measurementFrame[NRRD_SPACE_DIM_MAX][NRRD_SPACE_DIM_MAX];
                                    /* if spaceDim is non-zero, this may store
                                       a spaceDim-by-spaceDim matrix which
                                       transforms vector/matrix coefficients
                                       in the "measurement frame" to those in
                                       the world space described by spaceDim
                                       (and hopefully space).  Coeff [i][j] is
                                       *column* i & *row* j, which is probably
                                       the *transpose* of what you expect.
                                       There are no semantics linking this to
                                       the "kind" of any axis, for a variety
                                       of reasons */
  size_t blockSize;                 /* for nrrdTypeBlock, block byte size */
  double oldMin, oldMax;            /* if non-NaN, and if nrrd is of integral
                                       type, extremal values for the array
                                       BEFORE it was quantized */
  void *ptr;                        /* never read or set by nrrd; use/abuse
                                       as you see fit */

  /*
  ** Comments.  Read from, and written to, header.
  ** The comment array "cmt" is NOT NULL-terminated.
  ** The number of comments is cmtArr->len.
  */
  char **cmt;
  airArray *cmtArr;

  /*
  ** Key-value pairs.
  */
  char **kvp;
  airArray *kvpArr;
} Nrrd;

struct NrrdIoState_t;
struct NrrdEncoding_t;

/*
******** NrrdFormat
**
** All information and behavior relevent to one datafile format
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];    /* short identifying string */
  int isImage,    /* this format is intended solely for "2D" images, which
                     controls the invocation of _nrrdReshapeUpGrayscale()
                     if nrrdStateGrayscaleImage3D */
    readable,     /* we can read as well as write this format */
    usesDIO;      /* this format can use Direct IO */

  /* tests if this format is currently available in this build */
  int (*available)(void);

  /* (for writing) returns non-zero if a given filename could likely be
     represented by this format */
  int (*nameLooksLike)(const char *filename);

  /* (for writing) returns non-zero if a given nrrd/encoding pair will fit
     in this format */
  int (*fitsInto)(const Nrrd *nrrd, const struct NrrdEncoding_t *encoding,
                   int useBiff);

  /* (for reading) returns non-zero if what has been read in so far
     is recognized as the beginning of this format */
  int (*contentStartsLike)(struct NrrdIoState_t *nio);

  /* reader and writer */
  int (*read)(FILE *file, Nrrd *nrrd, struct NrrdIoState_t *nio);
  int (*write)(FILE *file, const Nrrd *nrrd, struct NrrdIoState_t *nio);
} NrrdFormat;

/*
******** NrrdEncoding
**
** All information and behavior relevent to one way of encoding data
**
** The data readers are responsible for memory allocation.
** This is necessitated by the memory restrictions of direct I/O
*/
typedef struct NrrdEncoding_t {
  char name[AIR_STRLEN_SMALL],    /* short identifying string */
    suffix[AIR_STRLEN_SMALL];     /* costumary filename suffix */
  int endianMatters,
    isCompression;
  int (*available)(void);
  /* The "data" and "elementNum" values have to be passed explicitly
     to read/wrote because they will be different from nrrd->data and
     nrrdElementNumber(nrrd) in the case of multiple data files.  You
     might think that the only other thing required to be passed is
     nrrdElementSize(nrrd), but no, it is in fact best to pass the
     whole Nrrd, instead of just certain attributes.  The stupid details:
             nrrd->dim: needed to know whether to put one value per line
                        in case of 1-D nrrdEncodingAscii
    nrrd->axis[0].size: need for proper formatting of nrrdEncodingAscii
            nrrd->type: needed for nrrdEncodingAscii, since its action is
                        entirely parameterized by type
       nrrd->blockSize: needed for nrrdElementSize in case of nrrdTypeBlock */
  int (*read)(FILE *file, void *data, size_t elementNum,
              Nrrd *nrrd, struct NrrdIoState_t *nio);
  int (*write)(FILE *file, const void *data, size_t elementNum,
               const Nrrd *nrrd, struct NrrdIoState_t *nio);
} NrrdEncoding;

/*
******** NrrdIoState struct
**
** Everything relating to how the nrrd is read and written.
** Multiple parameters for writing are set here (like format, encoding,
** zlib parameters).  Also, this is the place where those few parameters
** of reading are stored (like skipData and keepNrrdDataFileOpen).  Also,
** after the nrrd has been read, it is a potentially useful record of what
** it took to read it in.
*/
typedef struct NrrdIoState_t {
  char *path,               /* allows us to remember the directory
                               from whence this nrrd was "load"ed, or
                               to whence this nrrd is "save"ed, MINUS the
                               trailing "/", so as to facilitate games with
                               header-relative data files */
    *base,                  /* when "save"ing a nrrd into separate
                               header and data, the name of the header
                               file (e.g. "output.nhdr") MINUS the ".nhdr".
                               This is massaged to produce a header-
                               relative data filename.  */
    *line,                  /* buffer for saving one line from file */
    *dataFNFormat,          /* if non-NULL, the format string (containing
                               something like "%d" as a substring) to be
                               used to identify multiple detached datafiles.
                               NB: This is "format" in the sense of a printf-
                               style format string, not in the sense of a
                               file format.  This may need header-relative
                               path processing. */
    **dataFN,               /* ON READ + WRITE: array of data filenames. These
                               are not passed directly to fopen, they may need
                               header-relative path processing. Like the
                               cmtArr in the Nrrd, this array is not NULL-
                               terminated */
    *headerStringWrite;     /* ON WRITE: string from to which the header can
                               be written.  On write, it is assumed allocated
                               for as long as it needs to be (probably via a
                               first pass with learningHeaderStrlen). NOTE:
                               It is the non-NULL-ity of this which signifies
                               the intent to do string-based writing */
  const char
    *headerStringRead;      /* ON READ: like headerStringWrite, but for
                               reading the header from.  NOTE: It is the
                               non-NULL-ity of this which signifies the
                               intent to do string-based reading */
  airArray *dataFNArr;      /* for managing the above */

  FILE *headerFile,         /* if non-NULL, the file from which the NRRD
                               header is being read */
    *dataFile;              /* this used to be a central part of how the
                               I/O code worked, but now it is simply the
                               place to store the dataFile in the case of
                               keepNrrdDataFileOpen */
  unsigned int dataFileDim, /* The dimension of the data in each data file.
                               Together with dataFNArr->len, this determines
                               how many bytes should be in each data file */
    lineLen,                /* allocated size of line, including the
                               last character for \0 */
    charsPerLine,           /* when writing ASCII data in which we
                               intend only to write a huge long list
                               of numbers whose text formatting
                               implies nothing, then how many
                               characters do we limit ourselves to per
                               line */
    valsPerLine,            /* when writing ASCII data in which we DO
                               intend to sigify (or at least hint at)
                               something with the formatting, then
                               what is the max number of values to
                               write on a line */
    lineSkip,               /* if dataFile non-NULL, the number of
                               lines in dataFile that should be
                               skipped over (so as to bypass another
                               form of ASCII header preceeding raw
                               data) */
    headerStrlen,           /* ON WRITE, for NRRDs, if learningHeaderStrlen,
                               the learned strlen of the header so far */
    headerStrpos;           /* ON READ, for NRRDs, if headerStringRead is
                               non-NULL, the current location of reading
                               in the header */
  long int byteSkip;        /* exactly like lineSkip, but bytes
                               instead of lines.  First the lines are
                               skipped, then the bytes */
  /* Note that the NRRD0004 and NRRD0005 file formats indicate that a numbered
     sequence of data filenames should be indexed via a "%d" format
     specification, and that the format doc says nothing about the "min" and
     "max" fields of "data file" being only positive.  So the following three
     dataFN* fields are appropriately (signed) ints, even if all normal usage
     could also be represented with unsigned ints.  Nonetheless, the return
     from _nrrdDataFNNumber(), which gives the total number of file names, is
     still appropriately an unsigned int. This may be revisited if the file
     format itself is adjusted. */
  int dataFNMin,            /* used with dataFNFormat to identify .. */
    dataFNMax,              /* .. all the multiple detached datafiles */
    dataFNStep;             /* how to step from max to min */
  /* On the other hand, dataFNIndex ranges from 0 to (#datafiles-1),
     and not dataFNMin to dataFNMax, so it really should be unsigned */
  unsigned int dataFNIndex; /* which of the data files are being read */
  int pos,                  /* line[pos] is beginning of stuff which
                               still has yet to be parsed */
    endian,                 /* endian-ness of the data in file, for
                               those encoding/type combinations for
                               which it matters (from nrrdEndian) */
    seen[NRRD_FIELD_MAX+1], /* for error checking in header parsing */
    detachedHeader,         /* ON WRITE: request for file (NRRD format only)
                               to be split into distinct header and data.
                               This only has an effect if detaching the header
                               is not already necessary, as it is with multiple
                               data files */
    bareText,               /* when writing a plain text file, is there any
                               effort made to record the nrrd struct
                               info in the text file */
    skipData,               /* if non-zero (all formats):
                               ON READ: don't allocate memory for, and don't
                               read in, the data portion of the file (but we
                               do verify that for nrrds, detached datafiles
                               can be opened).  Note: Does NOT imply
                               keepNrrdDataFileOpen.  Warning: resulting
                               nrrd struct will have "data" pointer NULL.
                               ON WRITE: don't write data portion of file
                               (for nrrds, don't even try to open detached
                               datafiles).  Warning: can result in broken
                               noncomformant files.
                               (be careful with this) */
    skipFormatURL,          /* if non-zero for NRRD format ON WRITE:
                               skip the comment lines that document where
                               to find the NRRD file format specs */
    keepNrrdDataFileOpen,   /* ON READ: when there is only a single dataFile,
                               don't close nio->dataFile when
                               you otherwise would, when reading the
                               nrrd format. Probably used in conjunction with
                               skipData.  (currently for "unu data")
                               ON WRITE: no semantics */
    zlibLevel,              /* zlib compression level (0-9, -1 for
                               default[6], 0 for no compression). */
    zlibStrategy,           /* zlib compression strategy, can be one
                               of the nrrdZlibStrategy enums, default is
                               nrrdZlibStrategyDefault. */
    bzip2BlockSize,         /* block size used for compression,
                               roughly equivalent to better but slower
                               (1-9, -1 for default[9]). */
    learningHeaderStrlen;   /* ON WRITE, for nrrds, learn and save the total
                               length of header into headerStrlen. This is
                               used to allocate a buffer for header */
  void *oldData;            /* ON READ: if non-NULL, pointer to space that
                               has already been allocated for oldDataSize */
  size_t oldDataSize;       /* ON READ: size of mem pointed to by oldData */

  /* The format and encoding.  These are initialized to nrrdFormatUnknown
     and nrrdEncodingUnknown, respectively. USE THESE VALUES for
     any kind of initialization or flagging; DO NOT USE NULL */
  const NrrdFormat *format;
  const NrrdEncoding *encoding;
} NrrdIoState;

/* ---- BEGIN non-NrrdIO */

/*
******** NrrdRange
**
** information about a range of values, used as both a description
** of an existing nrrd, or as input to functions like nrrdQuantize
** (in which case the given min,max may not correspond to the actual
** min,max of the nrrd in question).
**
** This information has been removed from the Nrrd struct (as of Teem1.6)
** and put into this separate entity because:
** 1) when intended to be descriptive of a nrrd, it can't be guaranteed
** to be true across nrrd calls
** 2) when used as input parameters (e.g. to nrrdQuantize), its not
** data-flow friendly (you can't modify input)
*/
typedef struct {
  double min, max;  /* if non-NaN, nominally: extremal values for array, but
                       practically: the min and max values to use for nrrd
                       calls for which a min and max values are used */
  int hasNonExist;  /* from the nrrdHasNonExist* enum values */
} NrrdRange;

/*
******** NrrdKernel struct
**
** these are essentially the methods of the various kernels implemented.
**
** Nrrd's use of this sort of kernel always assumes support symmetric
** around zero, but does not assume anything about even- or oddness
**
** It is a strong but very simplifying assumption that the parameter
** array ("parm") is always type double.  There is essentially no
** value in allowing flexibility between float and double, and much
** Teem code assumes that it will always be type double.
*/
typedef struct {
  /* terse string representation of kernel function, irrespective of
     the parameter vector */
  char name[AIR_STRLEN_SMALL];

  /* number of parameters needed (# elements in parm[] used) */
  unsigned int numParm;  /* HEY: should be "parmNum" in standard convention */

  /* smallest x (x > 0) such that k(y) = 0 for all y > x, y < -x */
  double (*support)(const double *parm);

  /* integral of kernel from -support to +support */
  double (*integral)(const double *parm);

  /* evaluate once, single precision */
  float (*eval1_f)(float x, const double *parm);

  /* evaluate N times, single precision */
  void (*evalN_f)(float *f, const float *x, size_t N, const double *parm);

  /* evaluate once, double precision */
  double (*eval1_d)(double x, const double *parm);

  /* evaluate N times, double precision */
  void (*evalN_d)(double *f, const double *x, size_t N, const double *parm);
} NrrdKernel;

/*
******** NrrdKernelSpec struct
**
** for those times when it makes most sense to directly associate a
** NrrdKernel with its parameter vector (that is, a full kernel
** "spec"ification), basically: using hest.
*/
typedef struct {
  const NrrdKernel *kernel;
  double parm[NRRD_KERNEL_PARMS_NUM];
} NrrdKernelSpec;

/*
******** NrrdResampleInfo struct
**
** a struct to contain the many parameters needed for nrrdSpatialResample()
*/
typedef struct {
  const NrrdKernel
    *kernel[NRRD_DIM_MAX]; /* which kernel to use on each axis; use NULL to
                              say no resampling whatsoever on this axis */
  size_t samples[NRRD_DIM_MAX]; /* number of samples per axis */
  double parm[NRRD_DIM_MAX][NRRD_KERNEL_PARMS_NUM], /* kernel arguments */
    min[NRRD_DIM_MAX],
    max[NRRD_DIM_MAX];     /* min[i] and max[i] are the range, in WORLD space,
                              along which to resample axis i.  axis mins and
                              maxs are required on resampled axes. */
  int boundary,            /* value from the nrrdBoundary enum */
    type,                  /* desired type of output, use nrrdTypeUnknown for
                              "same as input" */
    renormalize,           /* when downsampling with a kernel with non-zero
                              integral, should we renormalize the weights to
                              match the kernel integral so as to remove
                              annoying ripple */
    round,                 /* when copying from the last intermediate (floating
                              point) result to the output nrrd, for integer
                              outputs, do we round to the nearest integer
                              first, before clamping and assigning.  Enabling
                              this fixed the mystery of downsampling large
                              constant regions of 255 (uchar), and ending up
                              with 254 */
    clamp,                  /* when copying from the last intermediate
                               (floating point) result to the output nrrd,
                               should we clamp the values to the range of
                               values for the output type, a concern only for
                               integer outputs */
    cheap;                  /* when *downsampling* (reducing the number of
                               samples), don't bother expanding the kernel to
                               achieve filtering in the old index space; with
                               nrrdKernelBox this can lead to subsampling by
                               picking using every other value */
  double padValue;          /* if padding, what value to pad with */
} NrrdResampleInfo;

/*
******** NrrdResampleAxis struct
**
** specific to one pass (one pass per axis) of resampling process
*/
typedef struct {
  /* ----------- input ---------- */
  const NrrdKernel *kernel;  /* which kernel to use on this axis; use NULL to
                                say no resampling whatsoever on this axis */
  double kparm[NRRD_KERNEL_PARMS_NUM]; /* kernel arguments */
  double min, max;           /* range in INDEX space of resampling */
  size_t samples;            /* number output samples on this axis (sizeOut) */
  int overrideCenter;        /* possible explicit setting of centering */
  /* ----------- internal ---------- */
  int center;                /* centering for this axis */
  size_t sizeIn,             /* number input samples on this axis */
    sizePerm[NRRD_DIM_MAX];  /* permutation of axis sizes for this pass */
  unsigned int axIdx,        /* what axis are we (redundant with other info) */
    passIdx,                 /* exactly which pass are we on */
    axisPerm[NRRD_DIM_MAX];  /* permutation of axis indices for this pass */
  double ratio;              /* > 1: upsampling; < 1: downsampling */
  Nrrd *nrsmp,               /* intermediate resampling result; input to
                                this pass */
    *nline,                  /* input scanline buffer (includes extra sample
                                at end for storing pad value) */
    *nindex,                 /* row of input indices for each output sample */
    *nweight;                /* row of input weights for each output sample */
} NrrdResampleAxis;

/*
******** NrrdResampleContext struct
**
** contains the parameters and state associated with resampling,
** geared towards quickly resampling multiple different nrrds with the
** same shape.
*/
typedef struct {
  /* ----------- input ---------- */
  const Nrrd *nin;          /* the nrrd being resampled */
  int verbose,              /* blah blah blah */
    boundary,               /* value from the nrrdBoundary enum */
    typeOut,                /* desired type of output, use nrrdTypeDefault for
                               "same as input" */
    renormalize,            /* when downsampling with a kernel with non-zero
                               integral, should we renormalize the weights to
                               match the kernel integral so as to remove
                               annoying ripple */
    roundlast,              /* when copying from the last intermediate
                               (floating point) result to the output nrrd,
                               for integer outputs, do we round to the nearest
                               integer first, before clamping and assigning.
                               Enabling this fixed the mystery of downsampling
                               large constant regions of 255 (uchar), and
                               ending up with 254
                               (renamed from "round" to avoid shadowing) */
    clamp,                   /* when copying from the last intermediate
                                (floating point) result to the output nrrd,
                               should we clamp the values to the range of
                                values for the output type, a concern only for
                                integer outputs */
    defaultCenter,           /* lacking known centering on input axis, what
                                centering to use when resampling */
    nonExistent;             /* from nrrdResampleNonExistent enum */
  double padValue;           /* if padding, what value to pad with */
  /* ----------- input/internal ---------- */
  unsigned int dim,          /* dimension of nin (saved here to help
                                manage state in NrrdResampleAxis[]) */
    passNum,                 /* number of passes needed */
    topRax, botRax,          /* fastest, slowest axes undergoing resampling */
    permute[NRRD_DIM_MAX+1], /* how each pass permutes axes */
    passAxis[NRRD_DIM_MAX];  /* mapping from pass index to axis index */
  NrrdResampleAxis
    axis[NRRD_DIM_MAX+1];    /* axis[j] stores information for input to
                                pass which is resampling nin->axis[j];
                                axis[NRRD_DIM_MAX] stores info about the
                                final output of all passes */
  int *flag;                 /* flags for managing state */
  /* ----------- output ---------- */
  double time;               /* time required for resampling */
} NrrdResampleContext;

/*
******** NrrdIter struct
**
** To hold values: either a single value, or a whole nrrd of values.
** Also, this facilitates iterating through those values
*/
typedef struct {
  const Nrrd *nrrd;            /* read-only nrrd to get values from */
  Nrrd *ownNrrd;               /* another nrrd to get values from, which we
                                  do "own", and do delete on nrrdIterNix */
  double val;                  /* single fixed value */
  size_t size;                 /* type size */
  char *data;                  /* where to get the next value */
  size_t left;                 /* number of values beyond what "data"
                                  currently points to */
  double (*load)(const void*); /* how to get a value out of "data" */
} NrrdIter;

/* ---- END non-NrrdIO */

/******** defaults (nrrdDefault..) and state (nrrdState..) */
/* defaultsNrrd.c */
NRRD_EXPORT int nrrdDefaultWriteEncodingType;
NRRD_EXPORT int nrrdDefaultWriteBareText;
NRRD_EXPORT unsigned int nrrdDefaultWriteCharsPerLine;
NRRD_EXPORT unsigned int nrrdDefaultWriteValsPerLine;
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT int nrrdDefaultResampleBoundary;
NRRD_EXPORT int nrrdDefaultResampleType;
NRRD_EXPORT int nrrdDefaultResampleRenormalize;
NRRD_EXPORT int nrrdDefaultResampleRound;
NRRD_EXPORT int nrrdDefaultResampleClamp;
NRRD_EXPORT int nrrdDefaultResampleCheap;
NRRD_EXPORT double nrrdDefaultResamplePadValue;
NRRD_EXPORT int nrrdDefaultResampleNonExistent;
NRRD_EXPORT double nrrdDefaultKernelParm0;
/* ---- END non-NrrdIO */
NRRD_EXPORT int nrrdDefaultCenter;
NRRD_EXPORT double nrrdDefaultSpacing;
NRRD_EXPORT int nrrdStateVerboseIO;
NRRD_EXPORT int nrrdStateKeyValuePairsPropagate;
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT int nrrdStateBlind8BitRange;
NRRD_EXPORT int nrrdStateMeasureType;
NRRD_EXPORT int nrrdStateMeasureModeBins;
NRRD_EXPORT int nrrdStateMeasureHistoType;
NRRD_EXPORT int nrrdStateDisallowIntegerNonExist;
/* ---- END non-NrrdIO */
NRRD_EXPORT int nrrdStateAlwaysSetContent;
NRRD_EXPORT int nrrdStateDisableContent;
NRRD_EXPORT const char *nrrdStateUnknownContent;
NRRD_EXPORT int nrrdStateGrayscaleImage3D;
NRRD_EXPORT int nrrdStateKeyValueReturnInternalPointers;
NRRD_EXPORT int nrrdStateKindNoop;
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT const char *const nrrdEnvVarDefaultWriteEncodingType;
NRRD_EXPORT const char *const nrrdEnvVarDefaultWriteBareText;
NRRD_EXPORT const char *const nrrdEnvVarDefaultWriteBareTextOld;
NRRD_EXPORT const char *const nrrdEnvVarDefaultCenter;
NRRD_EXPORT const char *const nrrdEnvVarDefaultCenterOld;
NRRD_EXPORT const char *const nrrdEnvVarDefaultWriteCharsPerLine;
NRRD_EXPORT const char *const nrrdEnvVarDefaultWriteValsPerLine;
NRRD_EXPORT const char *const nrrdEnvVarDefaultKernelParm0;
NRRD_EXPORT const char *const nrrdEnvVarDefaultSpacing;
NRRD_EXPORT const char *const nrrdEnvVarStateKindNoop;
NRRD_EXPORT const char *const nrrdEnvVarStateVerboseIO;
NRRD_EXPORT const char *const nrrdEnvVarStateKeyValuePairsPropagate;
NRRD_EXPORT const char *const nrrdEnvVarStateBlind8BitRange;
NRRD_EXPORT const char *const nrrdEnvVarStateAlwaysSetContent;
NRRD_EXPORT const char *const nrrdEnvVarStateDisableContent;
NRRD_EXPORT const char *const nrrdEnvVarStateMeasureType;
NRRD_EXPORT const char *const nrrdEnvVarStateMeasureModeBins;
NRRD_EXPORT const char *const nrrdEnvVarStateMeasureHistoType;
NRRD_EXPORT const char *const nrrdEnvVarStateGrayscaleImage3D;
NRRD_EXPORT int nrrdGetenvBool(int *val, char **envStr,
                               const char *envVar);
NRRD_EXPORT int nrrdGetenvEnum(int *val, char **envStr, const airEnum *enm,
                               const char *envVar);
NRRD_EXPORT int nrrdGetenvInt(int *val, char **envStr,
                              const char *envVar);
NRRD_EXPORT int nrrdGetenvUInt(unsigned int *val, char **envStr,
                               const char *envVar);
NRRD_EXPORT int nrrdGetenvDouble(double *val, char **envStr,
                                 const char *envVar);
NRRD_EXPORT void nrrdDefaultGetenv(void);
NRRD_EXPORT void nrrdStateGetenv(void);
/* ---- END non-NrrdIO */

/******** all the airEnums used through-out nrrd */
/*
** the actual C enums are in nrrdEnums.h; experience has shown that it
** is not particularly useful to name those enums, since the shortest
** name is best used for the airEnums here
*/
/* enumsNrrd.c */
NRRD_EXPORT const airEnum *const nrrdFormatType;
NRRD_EXPORT const airEnum *const nrrdType;
NRRD_EXPORT const airEnum *const nrrdEncodingType;
NRRD_EXPORT const airEnum *const nrrdCenter;
NRRD_EXPORT const airEnum *const nrrdKind;
NRRD_EXPORT const airEnum *const nrrdField;
NRRD_EXPORT const airEnum *const nrrdSpace;
NRRD_EXPORT const airEnum *const nrrdSpacingStatus;
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT const airEnum *const nrrdBoundary;
NRRD_EXPORT const airEnum *const nrrdMeasure;
NRRD_EXPORT const airEnum *const nrrdUnaryOp;
NRRD_EXPORT const airEnum *const nrrdBinaryOp;
NRRD_EXPORT const airEnum *const nrrdTernaryOp;
NRRD_EXPORT const airEnum *const nrrdFFTWPlanRigor;
NRRD_EXPORT const airEnum *const nrrdResampleNonExistent;
/* ---- END non-NrrdIO */

/******** arrays of things (poor-man's functions/predicates) */
/* arraysNrrd.c */
NRRD_EXPORT const char nrrdTypePrintfStr[NRRD_TYPE_MAX+1][AIR_STRLEN_SMALL];
NRRD_EXPORT const size_t nrrdTypeSize[NRRD_TYPE_MAX+1];
NRRD_EXPORT const double nrrdTypeMin[NRRD_TYPE_MAX+1];
NRRD_EXPORT const double nrrdTypeMax[NRRD_TYPE_MAX+1];
NRRD_EXPORT const int nrrdTypeIsIntegral[NRRD_TYPE_MAX+1];
NRRD_EXPORT const int nrrdTypeIsUnsigned[NRRD_TYPE_MAX+1];

/******** pseudo-constructors, pseudo-destructors, and such */
/* methodsNrrd.c */
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT const int nrrdPresent;
/* ---- END non-NrrdIO */
NRRD_EXPORT NrrdIoState *nrrdIoStateNew(void);
NRRD_EXPORT void nrrdIoStateInit(NrrdIoState *nio);
NRRD_EXPORT NrrdIoState *nrrdIoStateNix(NrrdIoState *nio);
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT NrrdResampleInfo *nrrdResampleInfoNew(void);
NRRD_EXPORT NrrdResampleInfo *nrrdResampleInfoNix(NrrdResampleInfo *info);
NRRD_EXPORT NrrdKernelSpec *nrrdKernelSpecNew(void);
NRRD_EXPORT NrrdKernelSpec *nrrdKernelSpecCopy(const NrrdKernelSpec *ksp);
NRRD_EXPORT void nrrdKernelSpecSet(NrrdKernelSpec *ksp, const NrrdKernel *k,
                                   const double kparm[NRRD_KERNEL_PARMS_NUM]);
NRRD_EXPORT void nrrdKernelParmSet(const NrrdKernel **kP,
                                   double kparm[NRRD_KERNEL_PARMS_NUM],
                                   NrrdKernelSpec *ksp);
NRRD_EXPORT NrrdKernelSpec *nrrdKernelSpecNix(NrrdKernelSpec *ksp);
/* ---- END non-NrrdIO */
NRRD_EXPORT void nrrdInit(Nrrd *nrrd);
NRRD_EXPORT Nrrd *nrrdNew(void);
NRRD_EXPORT Nrrd *nrrdNix(Nrrd *nrrd);
NRRD_EXPORT Nrrd *nrrdEmpty(Nrrd *nrrd);
NRRD_EXPORT Nrrd *nrrdNuke(Nrrd *nrrd);
NRRD_EXPORT int nrrdWrap_nva(Nrrd *nrrd, void *data, int type,
                             unsigned int dim, const size_t *size);
NRRD_EXPORT int nrrdWrap_va(Nrrd *nrrd, void *data, int type, unsigned int dim,
                            ... /* size_t sx, sy, .., axis(dim-1) size */);
NRRD_EXPORT void nrrdBasicInfoInit(Nrrd *nrrd, int excludeBitflag);
NRRD_EXPORT int nrrdBasicInfoCopy(Nrrd *nout, const Nrrd *nin,
                                  int excludeBitflag);
NRRD_EXPORT int nrrdCopy(Nrrd *nout, const Nrrd *nin);
NRRD_EXPORT int nrrdAlloc_nva(Nrrd *nrrd, int type, unsigned int dim,
                              const size_t *size);
NRRD_EXPORT int nrrdAlloc_va(Nrrd *nrrd, int type, unsigned int dim,
                             ... /* size_t sx, sy, .., axis(dim-1) size */);
NRRD_EXPORT int nrrdMaybeAlloc_nva(Nrrd *nrrd, int type, unsigned int dim,
                                   const size_t *size);
NRRD_EXPORT int nrrdMaybeAlloc_va(Nrrd *nrrd, int type, unsigned int dim,
                                  ... /* size_t sx, sy, .., ax(dim-1) size */);
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT int nrrdCompare(const Nrrd *ninA, const Nrrd *ninB,
                            int onlyData, double epsilon,
                            int *differ, char explain[AIR_STRLEN_LARGE]);
NRRD_EXPORT int nrrdPPM(Nrrd *, size_t sx, size_t sy);
NRRD_EXPORT int nrrdPGM(Nrrd *, size_t sx, size_t sy);
/* ---- END non-NrrdIO */

/******** axis info related */
/* axis.c */
NRRD_EXPORT int nrrdKindIsDomain(int kind);
NRRD_EXPORT unsigned int nrrdKindSize(int kind);
NRRD_EXPORT int nrrdAxisInfoCopy(Nrrd *nout, const Nrrd *nin,
                                 const int *axmap, int excludeBitflag);
NRRD_EXPORT void nrrdAxisInfoSet_nva(Nrrd *nin, int axInfo, const void *info);
NRRD_EXPORT void nrrdAxisInfoSet_va(Nrrd *nin, int axInfo,
                                    ... /* const void* */);
NRRD_EXPORT void nrrdAxisInfoGet_nva(const Nrrd *nrrd, int axInfo, void *info);
NRRD_EXPORT void nrrdAxisInfoGet_va(const Nrrd *nrrd, int axInfo,
                                    ... /* ??? */);
NRRD_EXPORT double nrrdAxisInfoPos(const Nrrd *nrrd, unsigned int ax,
                                   double idx);
NRRD_EXPORT double nrrdAxisInfoIdx(const Nrrd *nrrd, unsigned int ax,
                                   double pos);
NRRD_EXPORT void nrrdAxisInfoPosRange(double *loP, double *hiP,
                                      const Nrrd *nrrd, unsigned int ax,
                                      double loIdx, double hiIdx);
NRRD_EXPORT void nrrdAxisInfoIdxRange(double *loP, double *hiP,
                                      const Nrrd *nrrd, unsigned int ax,
                                      double loPos, double hiPos);
NRRD_EXPORT void nrrdAxisInfoSpacingSet(Nrrd *nrrd, unsigned int ax);
NRRD_EXPORT void nrrdAxisInfoMinMaxSet(Nrrd *nrrd, unsigned int ax,
                                       int defCenter);
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT int nrrdAxisInfoCompare(const NrrdAxisInfo *axisA,
                                    const NrrdAxisInfo *axisB,
                                    int *differ,
                                    char explain[AIR_STRLEN_LARGE]);
/* ---- END non-NrrdIO */
NRRD_EXPORT unsigned int nrrdDomainAxesGet(const Nrrd *nrrd,
                                           unsigned int axisIdx[NRRD_DIM_MAX]);
NRRD_EXPORT unsigned int nrrdRangeAxesGet(const Nrrd *nrrd,
                                          unsigned int axisIdx[NRRD_DIM_MAX]);
NRRD_EXPORT unsigned int nrrdSpatialAxesGet(const Nrrd *nrrd,
                                            unsigned int
                                            axisIdx[NRRD_DIM_MAX]);
NRRD_EXPORT unsigned int nrrdNonSpatialAxesGet(const Nrrd *nrrd,
                                               unsigned int
                                               axisIdx[NRRD_DIM_MAX]);
NRRD_EXPORT int nrrdSpacingCalculate(const Nrrd *nrrd, unsigned int ax,
                                     double *spacing,
                                     double vector[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT int nrrdOrientationReduce(Nrrd *nout, const Nrrd *nin,
                                      int setMinsFromOrigin);

/******** simple things */
/* simple.c */
NRRD_EXPORT const char *nrrdBiffKey;
NRRD_EXPORT unsigned int nrrdSpaceDimension(int space);
NRRD_EXPORT int nrrdSpaceSet(Nrrd *nrrd, int space);
NRRD_EXPORT int nrrdSpaceDimensionSet(Nrrd *nrrd, unsigned int spaceDim);
NRRD_EXPORT unsigned int nrrdSpaceOriginGet(const Nrrd *nrrd,
                                            double vector[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT int nrrdSpaceOriginSet(Nrrd *nrrd, const double *vector);
NRRD_EXPORT int nrrdOriginCalculate(const Nrrd *nrrd,
                                    unsigned int *axisIdx,
                                    unsigned int axisIdxNum,
                                    int defaultCenter, double *origin);
NRRD_EXPORT int nrrdContentSet_va(Nrrd *nout, const char *func,
                                  const Nrrd *nin, const char *format,
                                  ... /* printf-style arg list */ );
NRRD_EXPORT void nrrdDescribe(FILE *file, const Nrrd *nrrd);
NRRD_EXPORT int nrrdCheck(const Nrrd *nrrd);
NRRD_EXPORT int _nrrdCheck(const Nrrd *nrrd, int checkData, int useBiff);
NRRD_EXPORT size_t nrrdElementSize(const Nrrd *nrrd);
NRRD_EXPORT size_t nrrdElementNumber(const Nrrd *nrrd);
NRRD_EXPORT int nrrdSanity(void);
NRRD_EXPORT int nrrdSameSize(const Nrrd *n1, const Nrrd *n2, int useBiff);
NRRD_EXPORT void nrrdSpaceVecCopy(double dst[NRRD_SPACE_DIM_MAX],
                                  const double src[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT void nrrdSpaceVecScaleAdd2(double sum[NRRD_SPACE_DIM_MAX],
                                       double sclA,
                                       const double vecA[NRRD_SPACE_DIM_MAX],
                                       double sclB,
                                       const double vecB[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT void nrrdSpaceVecScale(double out[NRRD_SPACE_DIM_MAX],
                                   double scl,
                                   const double vec[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT double nrrdSpaceVecNorm(unsigned int sdim,
                                    const double vec[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT int nrrdSpaceVecExists(unsigned int sdim,
                                   double vec[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT void nrrdSpaceVecSetNaN(double vec[NRRD_SPACE_DIM_MAX]);
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT void nrrdSanityOrDie(const char *me);
NRRD_EXPORT void nrrdSpaceVecSetZero(double vec[NRRD_SPACE_DIM_MAX]);
NRRD_EXPORT void nrrdZeroSet(Nrrd *nout);
/* ---- END non-NrrdIO */

/******** comments related */
/* comment.c */
NRRD_EXPORT int nrrdCommentAdd(Nrrd *nrrd, const char *str);
NRRD_EXPORT void nrrdCommentClear(Nrrd *nrrd);
NRRD_EXPORT int nrrdCommentCopy(Nrrd *nout, const Nrrd *nin);

/******** key/value pairs */
/* keyvalue.c */
NRRD_EXPORT unsigned int nrrdKeyValueSize(const Nrrd *nrrd);
NRRD_EXPORT int nrrdKeyValueAdd(Nrrd *nrrd,
                                const char *key, const char *value);
NRRD_EXPORT char *nrrdKeyValueGet(const Nrrd *nrrd, const char *key);
NRRD_EXPORT void nrrdKeyValueIndex(const Nrrd *nrrd,
                                   char **keyP, char **valueP,
                                   unsigned int ki);
NRRD_EXPORT int nrrdKeyValueErase(Nrrd *nrrd, const char *key);
NRRD_EXPORT void nrrdKeyValueClear(Nrrd *nrrd);
NRRD_EXPORT int nrrdKeyValueCopy(Nrrd *nout, const Nrrd *nin);

/******** endian related */
/* endianNrrd.c */
NRRD_EXPORT void nrrdSwapEndian(Nrrd *nrrd);

/******** getting information to and from files */
/* formatXXX.c */
NRRD_EXPORT const NrrdFormat *const nrrdFormatNRRD;
NRRD_EXPORT const NrrdFormat *const nrrdFormatPNM;
NRRD_EXPORT const NrrdFormat *const nrrdFormatPNG;
NRRD_EXPORT const NrrdFormat *const nrrdFormatVTK;
NRRD_EXPORT const NrrdFormat *const nrrdFormatText;
NRRD_EXPORT const NrrdFormat *const nrrdFormatEPS;
/* format.c */
NRRD_EXPORT const NrrdFormat *const nrrdFormatUnknown;
NRRD_EXPORT const NrrdFormat *
  const nrrdFormatArray[NRRD_FORMAT_TYPE_MAX+1];

/* encodingXXX.c */
NRRD_EXPORT const NrrdEncoding *const nrrdEncodingRaw;
NRRD_EXPORT const NrrdEncoding *const nrrdEncodingAscii;
NRRD_EXPORT const NrrdEncoding *const nrrdEncodingHex;
NRRD_EXPORT const NrrdEncoding *const nrrdEncodingGzip;
NRRD_EXPORT const NrrdEncoding *const nrrdEncodingBzip2;
/* encoding.c */
NRRD_EXPORT const NrrdEncoding *const nrrdEncodingUnknown;
NRRD_EXPORT const NrrdEncoding *
  const nrrdEncodingArray[NRRD_ENCODING_TYPE_MAX+1];

/* parseNrrd.c */
/* this needs the "FILE *file" first arg for the sole reason that
   parsing a "data file: " field which identifies a LIST must then
   read in all the data filenames from the same file */
NRRD_EXPORT int (*nrrdFieldInfoParse[NRRD_FIELD_MAX+1])(FILE *file, Nrrd *nrrd,
                                                        NrrdIoState *nio,
                                                        int useBiff);
NRRD_EXPORT unsigned int _nrrdDataFNNumber(NrrdIoState *nio);
NRRD_EXPORT int _nrrdContainsPercentThisAndMore(const char *str, char thss);
NRRD_EXPORT int _nrrdDataFNCheck(NrrdIoState *nio, Nrrd *nrrd, int useBiff);

/* read.c */
NRRD_EXPORT int _nrrdOneLine(unsigned int *lenP, NrrdIoState *nio, FILE *file);
NRRD_EXPORT int nrrdLineSkip(FILE *dataFile, NrrdIoState *nio);
NRRD_EXPORT int nrrdByteSkip(FILE *dataFile, Nrrd *nrrd, NrrdIoState *nio);
NRRD_EXPORT int nrrdLoad(Nrrd *nrrd, const char *filename, NrrdIoState *nio);
NRRD_EXPORT int nrrdLoadMulti(Nrrd *const *nin, unsigned int ninLen,
                              const char *fnameFormat,
                              unsigned int numStart, NrrdIoState *nio);
NRRD_EXPORT int nrrdRead(Nrrd *nrrd, FILE *file, NrrdIoState *nio);
NRRD_EXPORT int nrrdStringRead(Nrrd *nrrd, const char *string,
                               NrrdIoState *nio);

/* write.c */
NRRD_EXPORT int nrrdIoStateSet(NrrdIoState *nio, int parm, int value);
NRRD_EXPORT int nrrdIoStateEncodingSet(NrrdIoState *nio,
                                       const NrrdEncoding *encoding);
NRRD_EXPORT int nrrdIoStateFormatSet(NrrdIoState *nio,
                                     const NrrdFormat *format);
NRRD_EXPORT int nrrdIoStateGet(NrrdIoState *nio, int parm);
NRRD_EXPORT const NrrdEncoding *nrrdIoStateEncodingGet(NrrdIoState *nio);
NRRD_EXPORT const NrrdFormat *nrrdIoStateFormatGet(NrrdIoState *nio);
NRRD_EXPORT int nrrdSave(const char *filename, const Nrrd *nrrd,
                         NrrdIoState *nio);
NRRD_EXPORT int nrrdSaveMulti(const char *fnameFormat,
                              const Nrrd *const *nin, unsigned int ninLen,
                              unsigned int numStart, NrrdIoState *nio);
NRRD_EXPORT int nrrdWrite(FILE *file, const Nrrd *nrrd,
                          NrrdIoState *nio);
NRRD_EXPORT int nrrdStringWrite(char **stringP, const Nrrd *nrrd,
                                NrrdIoState *nio);

/******** getting value into and out of an array of general type, and
   all other simplistic functionality pseudo-parameterized by type */
/* accessors.c */
NRRD_EXPORT double (*nrrdDLoad[NRRD_TYPE_MAX+1])(const void *v);
NRRD_EXPORT float  (*nrrdFLoad[NRRD_TYPE_MAX+1])(const void *v);
NRRD_EXPORT int    (*nrrdILoad[NRRD_TYPE_MAX+1])(const void *v);
NRRD_EXPORT unsigned int (*nrrdUILoad[NRRD_TYPE_MAX+1])(const void *v);
NRRD_EXPORT double (*nrrdDStore[NRRD_TYPE_MAX+1])(void *v, double d);
NRRD_EXPORT float  (*nrrdFStore[NRRD_TYPE_MAX+1])(void *v, float f);
NRRD_EXPORT int    (*nrrdIStore[NRRD_TYPE_MAX+1])(void *v, int j);
NRRD_EXPORT unsigned int (*nrrdUIStore[NRRD_TYPE_MAX+1])(void *v,
                                                         unsigned int j);
NRRD_EXPORT double (*nrrdDLookup[NRRD_TYPE_MAX+1])(const void *v, size_t I);
NRRD_EXPORT float  (*nrrdFLookup[NRRD_TYPE_MAX+1])(const void *v, size_t I);
NRRD_EXPORT int    (*nrrdILookup[NRRD_TYPE_MAX+1])(const void *v, size_t I);
NRRD_EXPORT unsigned int (*nrrdUILookup[NRRD_TYPE_MAX+1])(const void *v,
                                                          size_t I);
NRRD_EXPORT double (*nrrdDInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                   double d);
NRRD_EXPORT float  (*nrrdFInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                   float f);
NRRD_EXPORT int    (*nrrdIInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                   int j);
NRRD_EXPORT unsigned int (*nrrdUIInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                          unsigned int j);
NRRD_EXPORT int    (*nrrdSprint[NRRD_TYPE_MAX+1])(char *, const void *);
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT int    (*nrrdFprint[NRRD_TYPE_MAX+1])(FILE *, const void *);
NRRD_EXPORT void (*nrrdMinMaxExactFind[NRRD_TYPE_MAX+1])(void *minP,
                                                         void *maxP,
                                                         int *hasNonExistP,
                                                         const Nrrd *nrrd);
NRRD_EXPORT int (*nrrdValCompare[NRRD_TYPE_MAX+1])(const void *,
                                                   const void *);
NRRD_EXPORT int (*nrrdValCompareInv[NRRD_TYPE_MAX+1])(const void *,
                                                      const void *);
NRRD_EXPORT int nrrdArrayCompare(int type, const void *valA, const void *valB,
                                 size_t valNum, double epsilon, int *differ,
                                 char explain[AIR_STRLEN_LARGE]);
/* ---- END non-NrrdIO */


/******** permuting, shuffling, and all flavors of reshaping */
/* reorder.c */
NRRD_EXPORT int nrrdAxesInsert(Nrrd *nout, const Nrrd *nin, unsigned int ax);
NRRD_EXPORT int nrrdInvertPerm(unsigned int *invp, const unsigned int *perm,
                               unsigned int n);
NRRD_EXPORT int nrrdAxesPermute(Nrrd *nout, const Nrrd *nin,
                                const unsigned int *axes);
NRRD_EXPORT int nrrdShuffle(Nrrd *nout, const Nrrd *nin, unsigned int axis,
                            const size_t *perm);
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT int nrrdAxesSwap(Nrrd *nout, const Nrrd *nin,
                             unsigned int ax1, unsigned int ax2);
NRRD_EXPORT int nrrdFlip(Nrrd *nout, const Nrrd *nin, unsigned int axis);
NRRD_EXPORT int nrrdJoin(Nrrd *nout, const Nrrd *const *nin,
                         unsigned int numNin,
                         unsigned int axis, int incrDim);
NRRD_EXPORT int nrrdReshape_va(Nrrd *nout, const Nrrd *nin, unsigned int dim,
                               ... /* sx, sy, .., axis(dim-1) size */ );
NRRD_EXPORT int nrrdReshape_nva(Nrrd *nout, const Nrrd *nin,
                                unsigned int dim, const size_t *size);
NRRD_EXPORT int nrrdAxesSplit(Nrrd *nout, const Nrrd *nin, unsigned int ax,
                              size_t sizeFast, size_t sizeSlow);
NRRD_EXPORT int nrrdAxesDelete(Nrrd *nout, const Nrrd *nin, unsigned int ax);
NRRD_EXPORT int nrrdAxesMerge(Nrrd *nout, const Nrrd *nin, unsigned int ax);
NRRD_EXPORT int nrrdBlock(Nrrd *nout, const Nrrd *nin);
NRRD_EXPORT int nrrdUnblock(Nrrd *nout, const Nrrd *nin, int type);
NRRD_EXPORT int nrrdTile2D(Nrrd *nout, const Nrrd *nin,
                           unsigned int ax0, unsigned int ax1,
                           unsigned int axSplit,
                           size_t sizeFast, size_t sizeSlow);
NRRD_EXPORT int nrrdUntile2D(Nrrd *nout, const Nrrd *nin,
                             unsigned int ax0, unsigned int ax1,
                             unsigned int axMerge, size_t sizeFast,
                             size_t sizeSlow);

/******** things useful with hest */
/* hestNrrd.c */
NRRD_EXPORT hestCB *nrrdHestNrrd;
NRRD_EXPORT hestCB *nrrdHestKernelSpec;
NRRD_EXPORT hestCB *nrrdHestIter;

/******** nrrd value iterator gadget */
/* iter.c */
NRRD_EXPORT NrrdIter *nrrdIterNew(void);
NRRD_EXPORT void nrrdIterSetValue(NrrdIter *iter, double val);
NRRD_EXPORT void nrrdIterSetNrrd(NrrdIter *iter, const Nrrd *nrrd);
NRRD_EXPORT void nrrdIterSetOwnNrrd(NrrdIter *iter, Nrrd *nrrd);
NRRD_EXPORT double nrrdIterValue(NrrdIter *iter);
NRRD_EXPORT char *nrrdIterContent(NrrdIter *iter);
NRRD_EXPORT NrrdIter *nrrdIterNix(NrrdIter *iter);

/******** expressing the range of values in a nrrd */
/* range.c */
NRRD_EXPORT NrrdRange *nrrdRangeNew(double min, double max);
NRRD_EXPORT NrrdRange *nrrdRangeCopy(const NrrdRange *range);
NRRD_EXPORT NrrdRange *nrrdRangeNix(NrrdRange *range);
NRRD_EXPORT void nrrdRangeReset(NrrdRange *range);
NRRD_EXPORT void nrrdRangeSet(NrrdRange *range,
                              const Nrrd *nrrd, int blind8BitRange);
NRRD_EXPORT int nrrdRangePercentileSet(NrrdRange *range, const Nrrd *nrrd,
                                       double minPerc, double maxPerc,
                                       unsigned int hbins,
                                       int blind8BitRange);
NRRD_EXPORT int nrrdRangePercentileFromStringSet(NrrdRange *range,
                                                 const Nrrd *nrrd,
                                                 const char *minStr,
                                                 const char *maxStr,
                                                 unsigned int hbins,
                                                 int blind8BitRange);
NRRD_EXPORT void nrrdRangeSafeSet(NrrdRange *range,
                                  const Nrrd *nrrd, int blind8BitRange);
NRRD_EXPORT NrrdRange *nrrdRangeNewSet(const Nrrd *nrrd, int blind8BitRange);
NRRD_EXPORT int nrrdHasNonExist(const Nrrd *nrrd);

/******** some of the point-wise value remapping, conversion, and such */
/* map.c */
NRRD_EXPORT float  (*nrrdFClamp[NRRD_TYPE_MAX+1])(float);
NRRD_EXPORT double (*nrrdDClamp[NRRD_TYPE_MAX+1])(double);
NRRD_EXPORT int nrrdConvert(Nrrd *nout, const Nrrd *nin, int type);
NRRD_EXPORT int nrrdClampConvert(Nrrd *nout, const Nrrd *nin, int type);
NRRD_EXPORT int nrrdQuantize(Nrrd *nout, const Nrrd *nin,
                             const NrrdRange *range, unsigned int bits);
NRRD_EXPORT int nrrdUnquantize(Nrrd *nout, const Nrrd *nin, int type);
NRRD_EXPORT int nrrdHistoEq(Nrrd *nout, const Nrrd *nin, Nrrd **nhistP,
                            unsigned int bins,
                            unsigned int smart, float amount);

/******** rest of point-wise value remapping, and "color"mapping */
/* apply1D.c */
NRRD_EXPORT int nrrdApply1DLut(Nrrd *nout,
                               const Nrrd *nin, const NrrdRange *range,
                               const Nrrd *nlut, int typeOut, int rescale);
NRRD_EXPORT int nrrdApplyMulti1DLut(Nrrd *nout, const Nrrd *nin,
                                    const NrrdRange *range, const Nrrd *nmlut,
                                    int typeOut, int rescale);
NRRD_EXPORT int nrrdApply1DRegMap(Nrrd *nout,
                                  const Nrrd *nin, const NrrdRange *range,
                                  const Nrrd *nmap, int typeOut, int rescale);
NRRD_EXPORT int nrrdApplyMulti1DRegMap(Nrrd *nout, const Nrrd *nin,
                                       const NrrdRange *range,
                                       const Nrrd *nmmap,
                                       int typeOut, int rescale);
NRRD_EXPORT int nrrd1DIrregMapCheck(const Nrrd *nmap);
NRRD_EXPORT int nrrd1DIrregAclGenerate(Nrrd *nacl, const Nrrd *nmap,
                                       size_t aclLen);
NRRD_EXPORT int nrrd1DIrregAclCheck(const Nrrd *nacl);
NRRD_EXPORT int nrrdApply1DIrregMap(Nrrd *nout,
                                    const Nrrd *nin, const NrrdRange *range,
                                    const Nrrd *nmap, const Nrrd *nacl,
                                    int typeOut, int rescale);
NRRD_EXPORT int nrrdApply1DSubstitution(Nrrd *nout,
                                        const Nrrd *nin, const Nrrd *nsubst);

/* apply2D.c */
NRRD_EXPORT int nrrdApply2DLut(Nrrd *nout,
                               const Nrrd *nin, unsigned int domainAxis,
                               const NrrdRange *range0,
                               const NrrdRange *range1,
                               const Nrrd *nlut, int typeOut,
                               int rescale0, int rescale1);

/* ---- END non-NrrdIO */

/******** sampling, slicing, cropping */
/* subset.c */
NRRD_EXPORT int nrrdSlice(Nrrd *nout, const Nrrd *nin,
                          unsigned int axis, size_t pos);
NRRD_EXPORT int nrrdCrop(Nrrd *nout, const Nrrd *nin,
                         size_t *min, size_t *max);
/* ---- BEGIN non-NrrdIO */
NRRD_EXPORT int nrrdSliceSelect(Nrrd *noutAbove, Nrrd *noutBelow,
                                const Nrrd *nin, unsigned int axi,
                                Nrrd *nline, double thresh);
NRRD_EXPORT int nrrdSample_nva(void *val, const Nrrd *nin,
                               const size_t *coord);
NRRD_EXPORT int nrrdSample_va(void *val, const Nrrd *nin,
                              ... /* size_t idx0, idx1, .., idx(dim-1) */ );
NRRD_EXPORT int nrrdSimpleCrop(Nrrd *nout, const Nrrd *nin, unsigned int crop);
NRRD_EXPORT int nrrdCropAuto(Nrrd *nout, const Nrrd *nin,
                             size_t _min[NRRD_DIM_MAX],
                             size_t _max[NRRD_DIM_MAX],
                             const unsigned int *keep, unsigned int keepNum,
                             int measr, double frac, int offset);

/******** padding */
/* superset.c */
NRRD_EXPORT int nrrdSplice(Nrrd *nout, const Nrrd *nin, const Nrrd *nslice,
                           unsigned int axis, size_t pos);
NRRD_EXPORT int nrrdPad_nva(Nrrd *nout, const Nrrd *nin,
                            const ptrdiff_t *min, const ptrdiff_t *max,
                            int boundary, double padValue);
NRRD_EXPORT int nrrdPad_va(Nrrd *nout, const Nrrd *nin,
                           const ptrdiff_t *min, const ptrdiff_t *max,
                           int boundary,
                           ... /* double value (if nrrdBoundaryPad) */);
NRRD_EXPORT int nrrdSimplePad_nva(Nrrd *nout, const Nrrd *nin,
                                  unsigned int pad,
                                  int boundary, double padValue);
NRRD_EXPORT int nrrdSimplePad_va(Nrrd *nout, const Nrrd *nin, unsigned int pad,
                                 int boundary,
                                 ... /* double value (if nrrdBoundaryPad) */);
NRRD_EXPORT int nrrdInset(Nrrd *nout, const Nrrd *nin,
                          const Nrrd *nsub, const size_t *min);

/******** measuring and projecting */
/* measure.c */
NRRD_EXPORT void (*nrrdMeasureLine[NRRD_MEASURE_MAX+1])(void *ans, int ansType,
                                                        const void *line,
                                                        int lineType,
                                                        size_t lineLen,
                                                        double axMin,
                                                        double axMax);
NRRD_EXPORT int nrrdProject(Nrrd *nout, const Nrrd *nin,
                            unsigned int axis, int measr, int type);

/********* various kinds of histograms and their analysis */
/* histogram.c */
NRRD_EXPORT int nrrdHisto(Nrrd *nout, const Nrrd *nin, const NrrdRange *range,
                          const Nrrd *nwght, size_t bins, int type);
NRRD_EXPORT int nrrdHistoCheck(const Nrrd *nhist);
NRRD_EXPORT int nrrdHistoDraw(Nrrd *nout, const Nrrd *nin, size_t sy,
                              int showLog, double max);
NRRD_EXPORT int nrrdHistoAxis(Nrrd *nout, const Nrrd *nin,
                              const NrrdRange *range,
                              unsigned int axis, size_t bins, int type);
NRRD_EXPORT int nrrdHistoJoint(Nrrd *nout, const Nrrd *const *nin,
                               const NrrdRange *const *range,
                               unsigned int ninNum,
                               const Nrrd *nwght, const size_t *bins,
                               int type, const int *clamp);
NRRD_EXPORT int nrrdHistoThresholdOtsu(double *threshP, const Nrrd *nhist,
                                       double expo);

/******** arithmetic and math on nrrds */
/* arith.c */
NRRD_EXPORT int nrrdArithGamma(Nrrd *nout, const Nrrd *nin,
                               const NrrdRange *range, double gamma);
NRRD_EXPORT int nrrdArithUnaryOp(Nrrd *nout, int op, const Nrrd *nin);
NRRD_EXPORT int nrrdArithBinaryOp(Nrrd *nout, int op,
                                  const Nrrd *ninA, const Nrrd *ninB);
NRRD_EXPORT int nrrdArithTernaryOp(Nrrd *nout, int op,
                                   const Nrrd *ninA, const Nrrd *ninB,
                                   const Nrrd *ninC);
NRRD_EXPORT int nrrdArithAffine(Nrrd *nout, double minIn,
                                const Nrrd *nin, double maxIn,
                                double minOut, double maxOut,
                                int clamp);
NRRD_EXPORT int nrrdArithIterBinaryOp(Nrrd *nout, int op,
                                      NrrdIter *inA, NrrdIter *inB);
NRRD_EXPORT int nrrdArithIterBinaryOpSelect(Nrrd *nout, int op,
                                            NrrdIter *inA, NrrdIter *inB,
                                            unsigned int which);
NRRD_EXPORT int nrrdArithIterTernaryOp(Nrrd *nout, int op,
                                       NrrdIter *inA, NrrdIter *inB,
                                       NrrdIter *inC);
NRRD_EXPORT int nrrdArithIterTernaryOpSelect(Nrrd *nout, int op,
                                             NrrdIter *inA, NrrdIter *inB,
                                             NrrdIter *inC,
                                             unsigned int which);
NRRD_EXPORT int nrrdArithIterAffine(Nrrd *nout, NrrdIter *minIn,
                                    NrrdIter *in, NrrdIter *maxIn,
                                    NrrdIter *minOut, NrrdIter *maxOut,
                                    int clamp);
NRRD_EXPORT unsigned int nrrdCRC32(const Nrrd *nin, int endian);

/******** filtering and re-sampling */
/* filt.c */
NRRD_EXPORT int nrrdCheapMedian(Nrrd *nout, const Nrrd *nin,
                                int pad, int mode,
                                unsigned int radius, float wght,
                                unsigned int bins);
NRRD_EXPORT int nrrdDistanceL2(Nrrd *nout, const Nrrd *nin,
                               int typeOut, const int *axisDo,
                               double thresh, int insideHigher);
NRRD_EXPORT int nrrdDistanceL2Biased(Nrrd *nout, const Nrrd *nin,
                                     int typeOut, const int *axisDo,
                                     double thresh, double bias,
                                     int insideHigher);
NRRD_EXPORT int nrrdDistanceL2Signed(Nrrd *nout, const Nrrd *nin,
                                     int typeOut, const int *axisDo,
                                     double thresh, int insideHigher);

/******** deringNrrd.c: deringing CT */
typedef struct {
  /* -------- INPUT */
  int verbose,                 /* blah blah blah */
    linearInterp,              /* instead of nearest neighbor in polar txf */
    verticalSeam;              /* dering left and right sides of image
                                  separately, asserting that there are no
                                  rings along vertical line through center */
  const Nrrd *nin;             /* array to dering */
  double center[2],            /* location of recon center in index space
                                  of fastest two axes */
    clampPerc[2],              /* percentiles above 0.0 and below 1.0 at which
                                  to clamp values (for ring estimation) from
                                  below and above, respectively. Setting both
                                  to zero means no such clamping */
    radiusScale;               /* radius scaling to polar txf */
  unsigned int thetaNum,       /* number of samples in theta in polar txf */
    clampHistoBins;            /* number of bins in histogram for computing
                                  clamping in terms of percentiles */
  const NrrdKernel *rkernel;   /* kernel for radial high-pass filtering */
  double rkparm[NRRD_KERNEL_PARMS_NUM];
  const NrrdKernel *tkernel;   /* kernel for blurring along theta */
  double tkparm[NRRD_KERNEL_PARMS_NUM];
  /* -------- INTERNAL */
  const char *cdataIn;         /* nin->data as char* */
  char *cdataOut;              /* nout->data as char* */
  size_t sliceSize;            /* sizeof slice */
  int clampDo;                 /* is there really is clamping to be done */
  double clamp[2];             /* clamping values implied by clampPerc */
  /* -------- OUTPUT */
  double ringMagnitude;        /* L2 norm of ring map; may be useful for
                                  optimizing an (unknown) center location */
} NrrdDeringContext;
NRRD_EXPORT NrrdDeringContext *nrrdDeringContextNew(void);
NRRD_EXPORT NrrdDeringContext *nrrdDeringContextNix(NrrdDeringContext *drc);
NRRD_EXPORT int nrrdDeringVerboseSet(NrrdDeringContext *drc, int verbose);
NRRD_EXPORT int nrrdDeringLinearInterpSet(NrrdDeringContext *drc,
                                          int linterp);
NRRD_EXPORT int nrrdDeringVerticalSeamSet(NrrdDeringContext *drc,
                                          int vertSeam);
NRRD_EXPORT int nrrdDeringInputSet(NrrdDeringContext *drc, const Nrrd *nin);
NRRD_EXPORT int nrrdDeringCenterSet(NrrdDeringContext *drc,
                                    double cx, double cy);
NRRD_EXPORT int nrrdDeringClampPercSet(NrrdDeringContext *drc,
                                       double lo, double hi);
NRRD_EXPORT int nrrdDeringClampHistoBinsSet(NrrdDeringContext *drc,
                                            unsigned int bins);
NRRD_EXPORT int nrrdDeringRadiusScaleSet(NrrdDeringContext *drc, double rsc);
NRRD_EXPORT int nrrdDeringThetaNumSet(NrrdDeringContext *drc,
                                      unsigned int thetaNum);
NRRD_EXPORT int nrrdDeringRadialKernelSet(NrrdDeringContext *drc,
                                   const NrrdKernel *rkernel,
                                   const double rkparm[NRRD_KERNEL_PARMS_NUM]);
NRRD_EXPORT int nrrdDeringThetaKernelSet(NrrdDeringContext *drc,
                                   const NrrdKernel *tkernel,
                                   const double tkparm[NRRD_KERNEL_PARMS_NUM]);
NRRD_EXPORT int nrrdDeringExecute(NrrdDeringContext *drc, Nrrd *nout);

/*
******** nrrdResample_t typedef
**
** type used to hold filter sample locations and weights in
** nrrdSpatialResample(), and to hold the intermediate sampling
** results.  Not as good as templating, but better than hard-coding
** float versus double.  Actually, the difference between float and
** double is not exposed in any functions or objects declared in this
** header; it is entirely internal to the operation of
** nrrdSpatialResample().
**
** Choose by setting "#if" arg to 1 (for float) or 0 (for double)
*/
#if 0
typedef float nrrdResample_t;
#  define nrrdResample_nt nrrdTypeFloat
#  define NRRD_RESAMPLE_FLOAT 1
#else
typedef double nrrdResample_t;
#  define nrrdResample_nt nrrdTypeDouble
#  define NRRD_RESAMPLE_FLOAT 0
#endif

/* resampleContext.c */
NRRD_EXPORT NrrdResampleContext *nrrdResampleContextNew(void);
NRRD_EXPORT NrrdResampleContext *nrrdResampleContextNix(NrrdResampleContext *);
NRRD_EXPORT int nrrdResampleDefaultCenterSet(NrrdResampleContext *rsmc,
                                             int center);
NRRD_EXPORT int nrrdResampleNonExistentSet(NrrdResampleContext *rsmc,
                                           int nonExistent);
NRRD_EXPORT int nrrdResampleNrrdSet(NrrdResampleContext *rsmc,
                                    const Nrrd *nin);
NRRD_EXPORT int nrrdResampleInputSet(NrrdResampleContext *rsmc,
                                     const Nrrd *nin);
NRRD_EXPORT int nrrdResampleKernelSet(NrrdResampleContext *rsmc,
                                      unsigned int axIdx,
                                      const NrrdKernel *kernel,
                                      double kparm[NRRD_KERNEL_PARMS_NUM]);
NRRD_EXPORT int nrrdResampleSamplesSet(NrrdResampleContext *rsmc,
                                       unsigned int axIdx,
                                       size_t samples);
NRRD_EXPORT int nrrdResampleRangeSet(NrrdResampleContext *rsmc,
                                     unsigned int axIdx,
                                     double min, double max);
NRRD_EXPORT int nrrdResampleOverrideCenterSet(NrrdResampleContext *rsmc,
                                              unsigned int axIdx,
                                              int center);
NRRD_EXPORT int nrrdResampleRangeFullSet(NrrdResampleContext *rsmc,
                                         unsigned int axIdx);
NRRD_EXPORT int nrrdResampleBoundarySet(NrrdResampleContext *rsmc,
                                        int boundary);
NRRD_EXPORT int nrrdResamplePadValueSet(NrrdResampleContext *rsmc,
                                        double padValue);
NRRD_EXPORT int nrrdResampleTypeOutSet(NrrdResampleContext *rsmc,
                                       int typeOut);
NRRD_EXPORT int nrrdResampleRenormalizeSet(NrrdResampleContext *rsmc,
                                           int renormalize);
NRRD_EXPORT int nrrdResampleRoundSet(NrrdResampleContext *rsmc,
                                     int round);
NRRD_EXPORT int nrrdResampleClampSet(NrrdResampleContext *rsmc,
                                     int clamp);
NRRD_EXPORT int nrrdResampleExecute(NrrdResampleContext *rsmc, Nrrd *nout);

/* resampleNrrd.c */
NRRD_EXPORT int nrrdSpatialResample(Nrrd *nout, const Nrrd *nin,
                                    const NrrdResampleInfo *info);
NRRD_EXPORT int nrrdSimpleResample(Nrrd *nout, const Nrrd *nin,
                                   const NrrdKernel *kernel,
                                   const double *parm,
                                   const size_t *samples,
                                   const double *scalings);

/******** connected component extraction and manipulation */
/* ccmethods.c */
NRRD_EXPORT int nrrdCCValid(const Nrrd *nin);
NRRD_EXPORT unsigned int nrrdCCSize(Nrrd *nout, const Nrrd *nin);
NRRD_EXPORT unsigned int nrrdCCMax(const Nrrd *nin);
NRRD_EXPORT unsigned int nrrdCCNum(const Nrrd *nin);

/* cc.c */
NRRD_EXPORT int nrrdCCFind(Nrrd *nout, Nrrd **nvalP, const Nrrd *nin,
                           int type, unsigned int conny);
NRRD_EXPORT int nrrdCCAdjacency(Nrrd *nout, const Nrrd *nin,
                                unsigned int conny);
NRRD_EXPORT int nrrdCCMerge(Nrrd *nout, const Nrrd *nin, Nrrd *nval,
                            int dir, unsigned int maxSize,
                            unsigned int maxNeighbor, unsigned int conny);
NRRD_EXPORT int nrrdCCRevalue (Nrrd *nout, const Nrrd *nin, const Nrrd *nval);
NRRD_EXPORT int nrrdCCSettle(Nrrd *nout, Nrrd **nvalP, const Nrrd *nin);

/******** FFT */
/* fftNrrd.c */
NRRD_EXPORT const int nrrdFFTWEnabled;
NRRD_EXPORT int nrrdFFTWWisdomRead(FILE *file);
NRRD_EXPORT int nrrdFFT(Nrrd *nout, const Nrrd *nin,
                        unsigned int *axes, unsigned int axesLen,
                        int sign, int rescale, int preCompLevel);
NRRD_EXPORT int nrrdFFTWWisdomWrite(FILE *file);

/******** kernels (interpolation, 1st and 2nd derivatives) */
/* new kernels should also be registered with
   meet/meetNrrd.c/meetNrrdKernelAll() */
/* tmfKernel.c
   nrrdKernelTMF[D+1][C+1][A] is d<D>_c<C>_<A>ef:
   Dth-derivative, C-order continuous ("smooth"), A-order accurate
   (for D and C, index 0 accesses the function for -1) */
NRRD_EXPORT NrrdKernel *const nrrdKernelTMF[4][5][5];
NRRD_EXPORT const unsigned int nrrdKernelTMF_maxD;
NRRD_EXPORT const unsigned int nrrdKernelTMF_maxC;
NRRD_EXPORT const unsigned int nrrdKernelTMF_maxA;
/* winKernel.c : various kinds of windowed sincs */
NRRD_EXPORT NrrdKernel
  *const nrrdKernelHann,         /* Hann (cosine-bell) windowed sinc */
  *const nrrdKernelHannD,        /* 1st derivative of Hann windowed since */
  *const nrrdKernelHannDD,       /* 2nd derivative */
  *const nrrdKernelBlackman,     /* Blackman windowed sinc */
  *const nrrdKernelBlackmanD,    /* 1st derivative of Blackman windowed sinc */
  *const nrrdKernelBlackmanDD;   /* 2nd derivative */
/* bsplKernel.c: b-splines of various orders; these do not interpolate,
   but the ApproxInverse kernels are ok for pre-filtering so that they do */
NRRD_EXPORT NrrdKernel
  *const nrrdKernelBSpline1,     /* 1st order B-spline */
  *const nrrdKernelBSpline1D,
  *const nrrdKernelBSpline2,     /* 2nd order (quadratic) B-spline */
  *const nrrdKernelBSpline2D,
  *const nrrdKernelBSpline2DD,
  *const nrrdKernelBSpline3,     /* 3rd order (cubic) B-spline */
  *const nrrdKernelBSpline3D,
  *const nrrdKernelBSpline3DD,
  *const nrrdKernelBSpline3DDD,
  *const nrrdKernelBSpline3ApproxInverse,
  *const nrrdKernelBSpline4,     /* 4rd order B-spline */
  *const nrrdKernelBSpline4D,
  *const nrrdKernelBSpline4DD,
  *const nrrdKernelBSpline4DDD,
  *const nrrdKernelBSpline5,     /* 5th order B-spline */
  *const nrrdKernelBSpline5D,
  *const nrrdKernelBSpline5DD,
  *const nrrdKernelBSpline5DDD,
  *const nrrdKernelBSpline5ApproxInverse,
  *const nrrdKernelBSpline6,     /* 6th order B-spline */
  *const nrrdKernelBSpline6D,
  *const nrrdKernelBSpline6DD,
  *const nrrdKernelBSpline6DDD,
  *const nrrdKernelBSpline7,     /* 7th order B-spline */
  *const nrrdKernelBSpline7D,
  *const nrrdKernelBSpline7DD,
  *const nrrdKernelBSpline7DDD,
  *const nrrdKernelBSpline7ApproxInverse;
/* kernel.c: the rest of the kernels and kernel utility functions */
NRRD_EXPORT NrrdKernel
  *const nrrdKernelZero,         /* zero everywhere (though still takes
                                    a single "support" parm[0]) */
  *const nrrdKernelBox,          /* box filter (nearest neighbor) */
  *const nrrdKernelBoxSupportDebug, /* box kernel but with an adjustable
                                       support, not for changing the shape of
                                       the kernel (as with nrrdKernelBox), but
                                       for exercising functions (like nrrd's
                                       resampling or gage's probing) that
                                       depend on kernel support */
  *const nrrdKernelCos4SupportDebug, /* like BoxSupportDebug but instead of
                                        box function, using cos(pi*x)^4
                                        within [-0.5,0.5] and 0.0 outside */
  *const nrrdKernelCos4SupportDebugD,
  *const nrrdKernelCos4SupportDebugDD,
  *const nrrdKernelCos4SupportDebugDDD,
  *const nrrdKernelCatmullRomSupportDebug,
  *const nrrdKernelCatmullRomSupportDebugD,
  *const nrrdKernelCatmullRomSupportDebugDD,
  *const nrrdKernelCheap,        /* an unusual and specially-handled kernel
                                    that evaluates to abs(x), for the
                                    singular purpose of enabling nearest
                                    neighbor downsampling */
  *const nrrdKernelHermiteScaleSpaceFlag,  /* a kernel that looks like tent,
                                              but which exists as a flag for
                                              particular gage behavior in the
                                              context of doing interpolation
                                              along scale in scale-space */
  *const nrrdKernelTent,         /* tent filter (linear interpolation) */
  *const nrrdKernelForwDiff,     /* forward-difference-ish 1st deriv. */
  *const nrrdKernelCentDiff,     /* central-difference-ish 1st deriv. */
  *const nrrdKernelBCCubic,      /* BC family of cubic polynomial splines */
  *const nrrdKernelBCCubicD,     /* 1st deriv. of BC cubic family */
  *const nrrdKernelBCCubicDD,    /* 2nd deriv. of BC cubic family */
  *const nrrdKernelCatmullRom,   /* aka cubic:0,0.5 */
  *const nrrdKernelCatmullRomD,  /* aka cubicd:0,0.5 */
  *const nrrdKernelCatmullRomDD, /* aka cubicdd:0,0.5 */
  *const nrrdKernelAQuartic,     /* A family of quartic C2 interp. splines */
  *const nrrdKernelAQuarticD,    /* 1st deriv. of A quartic family */
  *const nrrdKernelAQuarticDD,   /* 2nd deriv. of A quartic family */
  *const nrrdKernelC3Quintic,    /* 0-parm C3 quintic, support [-2,2] */
  *const nrrdKernelC3QuinticD,   /* 1st deriv of C3Quintic */
  *const nrrdKernelC3QuinticDD,  /* 2nd deriv of C3Quintic */
  *const nrrdKernelC4Hexic,      /* 0-parm C4 hex, support [-3,3] */
  *const nrrdKernelC4HexicD,     /* 1st deriv of C4Hexic */
  *const nrrdKernelC4HexicDD,    /* 2nd deriv of C4Hexic */
  *const nrrdKernelC4HexicDDD,   /* 3rd deriv of C4Hexic */
  *const nrrdKernelC4HexicApproxInverse,
  *const nrrdKernelC5Septic,     /* 0-parm C5 sept, support [-4,4] */
  *const nrrdKernelC5SepticD,    /* 1st deriv of C5Septic */
  *const nrrdKernelC5SepticDD,   /* 2nd deriv of C5Septic */
  *const nrrdKernelC5SepticDDD,  /* 3rd deriv of C5Septic */
  *const nrrdKernelC5SepticApproxInverse,
  *const nrrdKernelGaussian,     /* Gaussian */
  *const nrrdKernelGaussianD,    /* 1st derivative of Gaussian */
  *const nrrdKernelGaussianDD,   /* 2nd derivative of Gaussian */
  *const nrrdKernelDiscreteGaussian; /* Discrete Gaussian-like kernel for
                                        scale-space analysis */
NRRD_EXPORT int nrrdKernelParse(const NrrdKernel **kernelP,
                                double *parm,
                                const char *str);
NRRD_EXPORT int nrrdKernelSpecParse(NrrdKernelSpec *ksp, const char *str);
NRRD_EXPORT int nrrdKernelSpecSprint(char str[AIR_STRLEN_LARGE],
                                     const NrrdKernelSpec *ksp);
NRRD_EXPORT int nrrdKernelSprint(char str[AIR_STRLEN_LARGE],
                                 const NrrdKernel *kernel,
                                 const double *kparm);
NRRD_EXPORT int nrrdKernelCompare(const NrrdKernel *kernA,
                                  const double parmA[NRRD_KERNEL_PARMS_NUM],
                                  const NrrdKernel *kernB,
                                  const double parmB[NRRD_KERNEL_PARMS_NUM],
                                  int *differ,
                                  char explain[AIR_STRLEN_LARGE]);
NRRD_EXPORT int nrrdKernelCheck(const NrrdKernel *kern,
                                const double parm[NRRD_KERNEL_PARMS_NUM],
                                size_t evalNum, double epsilon,
                                unsigned int diffOkEvalMax,
                                unsigned int diffOkIntglMax,
                                const NrrdKernel *dkern,
                                const double dparm[NRRD_KERNEL_PARMS_NUM]);

/* ---- END non-NrrdIO */

#ifdef __cplusplus
}
#endif
#endif /* NRRD_HAS_BEEN_INCLUDED */
