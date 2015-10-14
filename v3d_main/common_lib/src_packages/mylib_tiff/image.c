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
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilities.h"
#include "array.h"
#include "MY_TIFF/tiff.io.h"
#include "MY_TIFF/tiff.image.h"
#include "image.h"

/****************************************************************************************
 *                                                                                      *
 *  SPACE MANAGEMENT AND HELPER ROUTINES                                                *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { Tiff_Reader *reader;
    Tiff_Writer *writer;
    int          eof;
  } Tio;


typedef struct __Tio
  { struct __Tio *next;
    struct __Tio *prev;
    int           refcnt;
    Tio           tio;
  } _Tio;

static _Tio *Free_Tio_List = NULL;
static _Tio *Use_Tio_List  = NULL;
static _Tio  Tio_Proto;

static int Tio_Offset = ((char *) &(Tio_Proto.tio)) - ((char *) &Tio_Proto);
static int Tio_Inuse  = 0;

int Tiff_Refcount(Tiff *tiff)
{ _Tio *object = (_Tio *) (((char *) tiff) - Tio_Offset);
  return (object->refcnt);
}

static inline Tio *new_tio(char *routine)
{ _Tio *object;
  Tio  *tio;

  if (Free_Tio_List == NULL)
    { object = (_Tio *) Guarded_Realloc(NULL,sizeof(_Tio),routine);
      tio = &(object->tio);
    }
  else
    { object = Free_Tio_List;
      Free_Tio_List = object->next;
      tio = &(object->tio);
    }
  Tio_Inuse += 1;
  object->refcnt = 1;
  if (Use_Tio_List != NULL)
    Use_Tio_List->prev = object;
  object->next = Use_Tio_List;
  object->prev = NULL;
  Use_Tio_List = object;
  tio->reader = NULL;
  tio->writer = NULL;
  return (tio);
}

static inline Tio *copy_tio(Tio *tio)
{ Tio *copy = new_tio("Copy_Tiff");
  *copy = *tio;
  if (tio->reader != NULL)
    copy->reader = Copy_Tiff_Reader(tio->reader);
  if (tio->writer != NULL)
    copy->writer = Copy_Tiff_Writer(tio->writer);
  return (copy);
}

Tiff *Copy_Tiff(Tiff *tiff)
{ return ((Tiff *) copy_tio(((Tio *) tiff))); }

static inline void pack_tio(Tio *tio)
{ _Tio *object  = (_Tio *) (((char *) tio) - Tio_Offset);
  if (tio->reader != NULL)
    Pack_Tiff_Reader(tio->reader);
  if (tio->writer != NULL)
    Pack_Tiff_Writer(tio->writer);
}

Tiff *Pack_Tiff(Tiff *tiff)
{ pack_tio(((Tio *) tiff));
  return (tiff);
}

Tiff *Inc_Tiff(Tiff *tiff)
{ _Tio *object  = (_Tio *) (((char *) tiff) - Tio_Offset);
  object->refcnt += 1;
  return (tiff);
}

static inline void free_tio(Tio *tio)
{ _Tio *object  = (_Tio *) (((char *) tio) - Tio_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Tiff\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Tio_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Tio_List;
  Free_Tio_List = object;
  if (tio->writer != NULL)
    Free_Tiff_Writer(tio->writer);
  if (tio->reader != NULL)
    Free_Tiff_Reader(tio->reader);
  Tio_Inuse -= 1;
}

void Free_Tiff(Tiff *tiff)
{ free_tio(((Tio *) tiff)); }

static inline void kill_tio(Tio *tio)
{ _Tio *object  = (_Tio *) (((char *) tio) - Tio_Offset);
  if (--object->refcnt > 0) return;
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Tiff\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Tio_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  if (tio->writer != NULL)
    Kill_Tiff_Writer(tio->writer);
  if (tio->reader != NULL)
    Kill_Tiff_Reader(tio->reader);
  free(((char *) tio) - Tio_Offset);
  Tio_Inuse -= 1;
}

void Kill_Tiff(Tiff *tiff)
{ kill_tio(((Tio *) tiff)); }

static inline void reset_tio()
{ _Tio *object;
  Tio  *tio;
  while (Free_Tio_List != NULL)
    { object = Free_Tio_List;
      Free_Tio_List = object->next;
      tio = &(object->tio);
      if (tio->writer != NULL)
        Kill_Tiff_Writer(tio->writer);
      if (tio->reader != NULL)
        Kill_Tiff_Reader(tio->reader);
      free(object);
    }
}

void Reset_Tiff()
{ reset_tio(); }

int Tiff_Usage()
{ return (Tio_Inuse); }

void Tiff_List(void (*handler)(Tiff *))
{ _Tio *a, *b;
  for (a = Use_Tio_List; a != NULL; a = b)
    { b = a->next;
      handler((Tiff *) &(a->tio));
    }
}

static char Error_String[1000];

char *Image_Error()
{ return (Error_String); }

static int type_size[] = { 1, 2, 4, 8, 1, 2, 4, 8, 4, 8 };

static int kind_size[] = { 1, 3, 4, 2 };

static char Empty_String[1] = { 0 };

static int determine_type(Tiff_Image *img, int cidx)   //  Determine type of channel cidx of img
{ Tiff_Channel *chan;

  chan = img->channels[cidx];
  if (chan->type == CHAN_FLOAT)
    return (FLOAT32);
  else if (chan->type == CHAN_SIGNED)
    { if (chan->scale > 16)
        return (INT32);
      else if (chan->scale > 8)
        return (INT16);
      else
        return (INT8);
    }
  else
    { if (chan->scale > 16)
        return (UINT32);
      else if (chan->scale > 8)
        return (UINT16);
      else
        return (UINT8);
    }
}

static int determine_kind(Tiff_Image *img, int cidx)   //  Determine kind of layer of img starting
{ if (img->number_channels >= cidx+3)                  //    at channel cidx
    { int i, n, s, t;

      if (img->number_channels >= cidx+4)
        n = cidx+4;
      else
        n = cidx+3;
      s = t = 0;
      for (i = cidx; i < n; i++)
        { if (img->channels[i]->scale != img->channels[cidx]->scale ||
              img->channels[i]->type  != img->channels[cidx]->type)
            break;
          if (img->channels[i]->interpretation == CHAN_RED)
            s += 1;
          else if (img->channels[i]->interpretation == CHAN_GREEN)
            s += 2;
          else if (img->channels[i]->interpretation == CHAN_BLUE)
            s += 4;
          else if (img->channels[i]->interpretation == CHAN_ALPHA)
            s += 8;
          if (i == cidx+2) t = s;
        }
      if (s == 15)
        return (RGBA_KIND);
      else if (t == 7)
        return (RGB_KIND);
    }
  return (PLAIN_KIND);
}

static int channel_order(Tiff_Channel *chan)   //  Map channel interpretation to array index
{ switch (chan->interpretation)
  { case CHAN_RED:
      return (0);
    case CHAN_GREEN:
      return (1);
    case CHAN_BLUE:
      return (2);
    case CHAN_ALPHA:
      return (3);
    default:
      return (0);
  }
}


/****************************************************************************************
 *                                                                                      *
 *  TIFF ROUTINES (OTHER THAN READ AND WRITE)                                           *
 *                                                                                      *
 ****************************************************************************************/

Tiff *G(Open_Tiff)(char *file_name, char *mode)
{ Tio *tif;

  tif = new_tio("Open_Tiff");

  if (strcmp(mode,"r") == 0)
    { tif->reader = Open_Tiff_Reader(file_name,NULL,NULL,
                                     strcmp(file_name+(strlen(file_name)-4),".lsm") == 0);
      if (tif->reader == NULL)
        { sprintf(Error_String,"Error reading Tif: '%s' (Open_Tiff)\n",Tiff_Error_String());
          return (NULL);
        }
      tif->writer = NULL;
      tif->eof    = End_Of_Tiff(tif->reader);
    }
  else if (strcmp(mode,"w") == 0)
    { tif->writer = Open_Tiff_Writer(file_name,0,0);
      tif->reader = NULL;
      tif->eof    = 1;
    }
  else
    { sprintf(Error_String,"Mode must be either 'r' or 'w' (Open_Tiff)\n");
      return (NULL);
    }

  return ((Tiff *) tif);
}

void Advance_Tiff(Tiff *M(etif))
{ Tio *tif = (Tio *) etif;

  if (tif->eof) return;
  Advance_Tiff_Reader(tif->reader);
  tif->eof = End_Of_Tiff(tif->reader);
}

void Rewind_Tiff(Tiff *M(etif))
{ Tio *tif = (Tio *) etif;

  Rewind_Tiff_Reader(tif->reader);
  tif->eof = End_Of_Tiff(tif->reader);
}

int Tiff_EOF(Tiff *tif)
{ return (((Tio *) tif)->eof); }

void Close_Tiff(Tiff *F(etif))
{ Tio *tif = (Tio *) etif;
  Free_Tiff(etif);
}


/****************************************************************************************
 *                                                                                      *
 *  COLOR MAP ROUTINES                                                                  *
 *                                                                                      *
 ****************************************************************************************/

static Array *colormap = NULL;

int Has_Map()
{ return (colormap != NULL); }

Array *Get_Map()
{ if (colormap == NULL)
    return (NULL);
  else
    return (Copy_Array(colormap));
}

void Set_Map(Array *C(map))
{ if (colormap != NULL)
    Free_Array(colormap);
  colormap = map;
}


/****************************************************************************************
 *                                                                                      *
 *  PARSING A SERIES NAME                                                               *
 *                                                                                      *
 ****************************************************************************************/

File_Bundle *Parse_Series_Name(char *file_name)
{ static File_Bundle my_bundle;

  static char *PreSuf = NULL;
  static int   PreSuf_Max = 0;

  char *s, *t, *n;

  if (strlen(file_name) >= PreSuf_Max)
    { PreSuf_Max = strlen(file_name)*1.2 + 20;
      PreSuf     = (char *) Guarded_Realloc(PreSuf,PreSuf_Max+1,"Parse_Series_Name");
    }

  strcpy(PreSuf,file_name);

  for (t = PreSuf + strlen(file_name); t > PreSuf; t--)
    if (*t == '/')
      break;

  for (; *t != 0 && !isdigit(*t); t++)
    ;
  if (*t == 0)
    { sprintf(Error_String,"No number in file name %s (Parse_Series_Name)\n",file_name);
      return (NULL);
    }
  n = t;
  while (isdigit(*t))
    t += 1;
  s = t;
  while (*t != 0)
    if (isdigit(*t++))
      { sprintf(Error_String,
            "Two distinct number subsegments in name (Parse_Series_Name)\n",file_name);
        return (NULL);
      }

  my_bundle.prefix    = PreSuf;
  my_bundle.suffix    = s;
  my_bundle.num_width = s-n;
  my_bundle.padded    = (*n == '0');
  my_bundle.first_num = atoi(n);
  *n = '\0';
  return (&my_bundle);
}


/****************************************************************************************
 *                                                                                      *
 *  CENTRAL READ ROUTINE                                                                *
 *                                                                                      *
 ****************************************************************************************/

  // There are depth planes in the tiff file, and each successive tif IFD is returned by
  //   calling reader_handler with the next plane number.  If layer < 0 then get all layers,
  //   otherwise load only the specified layer.  If there is a color-map and the first
  //   channel is CHAN_MAPPED then allocate and build a map, returning it in *pmap.  The
  //   originating external routine has name 'routine'.  One calls read_handler(-1,?) to
  //   take care of any epilogue activity for the reading of successive IFDs.

static Layer_Bundle *read_tiff(char *routine, Dimn_Type depth,
                               Tiff_Reader *(*read_handler)(int, char *),
                               Array **pmap, int layer)
{ static Layer_Bundle images = { 0, NULL };
  static Indx_Type   *area   = NULL;
  static int         *invert = NULL;
  static int          Max_Layers = 0;
  static void       **planes = NULL;
  static int          Max_Planes = 0;

  Array      **array;
  Array       *map;
  int          cidx, lidx, lnum, nlayers;
  Dimn_Type    width, height;
  int          kind, scale;
  Tiff_Type    type;
  Tiff_Reader *tif;
  Tiff_IFD    *ifd;
  Tiff_Image  *img;
  char        *text;
  int          count;
  Dimn_Type    dims[4];

  *pmap = NULL;

  tif = read_handler(0,routine);
  if (tif == NULL)
    return (NULL);

  ifd = Read_Tiff_IFD(tif);
  if (ifd == NULL)
    { sprintf(Error_String,"Error reading Tif IFD: '%s' (%s)\n",Tiff_Error_String(),routine);
      return (NULL);
    }
  img = Get_Tiff_Image(ifd);
  if (img == NULL)
    { sprintf(Error_String,"Error reading Tif Image: '%s' (%s)\n",Tiff_Error_String(),routine);
      Free_Tiff_IFD(ifd);
      return (NULL);
    }

  nlayers = 0;
  for (cidx = 0; cidx < img->number_channels; cidx += kind_size[kind])
    { kind = determine_kind(img,cidx);
      if (layer == nlayers)
        break;
      nlayers += 1;
    }
  if (layer >= 0)
    { if (cidx >= img->number_channels)
        { fprintf(stderr,"Layer %d does not exit in tiff (%s)\n",layer,routine);
          exit (1);
        }
      lidx = cidx;
      nlayers += 1;
    }

  if (nlayers > Max_Layers)
    { Max_Layers = nlayers + 5;
      images.layers = (Array **)
                        Guarded_Realloc(images.layers,sizeof(Array *)*Max_Layers,routine);
      area   = (Size_Type *)
                        Guarded_Realloc(area,(sizeof(Size_Type)+sizeof(int))*Max_Layers,routine);
      invert = (int *) (area + Max_Layers);
    }

  if (img->number_channels > Max_Planes)
    { Max_Planes = img->number_channels + 5;
      planes     = (void **) Guarded_Realloc(planes,sizeof(void *)*Max_Planes,routine);
    }

  { int i;
    for (i = 0; i < img->number_channels; i++)
      planes[i] = NULL;
  }

  images.num_layers = nlayers;

  array  = images.layers;
  width  = img->width;
  height = img->height;

  dims[0] = width;
  dims[1] = height;
  dims[2] = depth;

  if (layer >= 0)
    { cidx = lidx; lnum = layer; }
  else
    lnum = cidx = 0;
  for ( ; lnum < nlayers; lnum++, cidx += kind_size[kind])
    { kind         = determine_kind(img,cidx);
      type         = determine_type(img,cidx);
      scale        = img->channels[cidx]->scale;
      invert[lnum] = (img->channels[cidx]->interpretation == CHAN_WHITE);
      area[lnum]   = (((Indx_Type) width) * height) * type_size[type];
      array[lnum]  = Make_Array(kind,type,2 + (depth != 1),dims);
      array[lnum]->scale = scale;
    }

  map = NULL;
  if (img->channels[0]->interpretation == CHAN_MAPPED)
    { int dom = (1 << img->channels[0]->scale);
 
      dims[0] = dom;
      map = Make_Array(RGB_KIND,UINT16,1,dims);

      memcpy(map->data,img->map,map->size*type_size[UINT16]);
    }

  if ((text = (char *) Get_Tiff_Tag(ifd,TIFF_JF_ANO_BLOCK,&type,&count)) == NULL)
    text = Empty_String;
  if (layer > 0)
    Set_Array_Text(array[layer],text);
  else
    Set_Array_Text(array[0],text);

  { int       i;
    Dimn_Type d;

    d = 0;
    while (1)
      { Tiff_Channel **chan = img->channels;

        if (layer >= 0)
          { cidx = lidx; lnum = layer; }
        else
          lnum = cidx = 0;
        for ( ; lnum < nlayers; lnum++)
          { Indx_Type base;

            kind = array[lnum]->kind;
            if (kind == RGB_KIND)
              for (i = 0; i < 3; i++)
                { base = channel_order(chan[cidx+i]);
                  planes[cidx+i] = array[lnum]->data+area[lnum]*(d+depth*base);
                }
            else if (kind == RGBA_KIND)
              for (i = 0; i < 4; i++)
                { base = channel_order(chan[cidx+i]);
                  planes[cidx+i] = array[lnum]->data+area[lnum]*(d+depth*base);
                }
            else // kind == PLAIN_KIND
              planes[cidx] = array[lnum]->data+area[lnum]*d;
            cidx += kind_size[kind];
          }
        Load_Tiff_Image_Planes(img,planes);

        Free_Tiff_Image(img);
        Free_Tiff_IFD(ifd);

        d += 1;
        if (d >= depth) break;

        tif = read_handler(1,routine);
        if (tif == NULL)
          goto cleanup;

        while (1)
          { int *tag;

            ifd = Read_Tiff_IFD(tif);
            if (ifd == NULL)
              { sprintf(Error_String,"Error reading Tif IFD: '%s' (%s)\n",
                                     Tiff_Error_String(),routine);
                goto cleanup;
              }
            tag = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
            if (tag == NULL || (*tag & TIFF_VALUE_REDUCED_RESOLUTION) == 0)
              break;
            Free_Tiff_IFD(ifd);
          }
        img = Get_Tiff_Image(ifd);
        if (img == NULL)
          { sprintf(Error_String,"Error reading Tif Image: '%s' (%s)\n",
                                 Tiff_Error_String(),routine);
            Free_Tiff_IFD(ifd);
            goto cleanup;
          }

        if (img->width != width || img->height != height)
          { sprintf(Error_String,
                    "Planes of a stack are not of the same dimensions (%s)!\n",routine);
            Free_Tiff_Image(img);
            Free_Tiff_IFD(ifd);
            goto cleanup;
          }

        if (layer >= 0)
          { cidx = lidx; lnum = layer; }
        else
          lnum = cidx = 0;
        for ( ; lnum < nlayers; lnum++)
          { kind = array[lnum]->kind;
            if (determine_type(img,cidx) != array[lnum]->type ||
                determine_kind(img,cidx) != kind ||
                img->channels[cidx]->scale != array[lnum]->scale)
              { sprintf(Error_String,"Planes of a stack are not of the same type (%s)!\n",routine);
                Free_Tiff_Image(img);
                Free_Tiff_IFD(ifd);
                goto cleanup;
              }
            cidx += kind_size[kind];
          }
      }
  }

  read_handler(-1,routine);

  if (layer >= 0)
    { cidx = lidx; lnum = layer; }
  else
    lnum = cidx = 0;
  for ( ; lnum < nlayers; lnum++)
    { if (invert[lnum])
        { double max;
          if (array[lnum]->type <= UINT32)
            max = ((((uint64) 1) << array[lnum]->scale) - 1);
          else if (array[lnum]->type <= INT32)
            max = ((((uint64) 1) << (array[lnum]->scale-1)) - 1);
          else
            max = 1.0;
          Scale_Array(array[lnum], -1., -max);
        }
      cidx += kind_size[array[lnum]->kind];
    }

  *pmap = map;
  return (&images);

cleanup:
  if (layer >= 0)
    { cidx = lidx; lnum = layer; }
  else
    lnum = cidx = 0;
  for ( ; lnum < nlayers; lnum++)
    Free_Array(array[lnum]);
  if (map != NULL) Free_Array(map);
  *pmap = NULL;
  return (NULL);
}


/****************************************************************************************
 *                                                                                      *
 *  TIF READER ROUTINES (EXTERNAL INTERFACES)                                           *
 *                                                                                      *
 ***************************************************************************************/

static Tio *Reader_Tio;

static Tiff_Reader *tiff_reader(int state, char *routine)
{ return (Reader_Tio->reader); }

Layer_Bundle *G(Read_Tiffs)(Tiff *etif)
{ Reader_Tio = (Tio *) etif; 
  Layer_Bundle *images;
  Array        *map;

  if (Reader_Tio->eof)
    { sprintf(Error_String,"At end of tiff file already (Read_Tiffs)\n");
      return (NULL);
    }
  images = read_tiff("Read_Tiffs",1,tiff_reader,&map,-1);
  if (images == NULL)
    return (NULL);
  Reader_Tio->eof = End_Of_Tiff(Reader_Tio->reader);
  Set_Map(map);
  return (images);
}

Array *G(Read_Tiff)(Tiff *etif, int layer)
{ Reader_Tio = (Tio *) etif; 
  Layer_Bundle *images;
  Array        *map;

  if (Reader_Tio->eof)
    { sprintf(Error_String,"At end of tiff file already (Read_Tiff)\n");
      return (NULL);
    }
  images = read_tiff("Read_Tiff",1,tiff_reader,&map,layer);
  if (images == NULL)
    return (NULL);
  Reader_Tio->eof = End_Of_Tiff(Reader_Tio->reader);
  Set_Map(map);
  return (images->layers[layer]);
}


/****************************************************************************************
 *                                                                                      *
 *  ALL OTHER READER ROUTINES (EXTERNAL INTERFACES)                                     *
 *                                                                                      *
 ***************************************************************************************/

static void *Reader_Source;

static char *get_series_name(int n, char *routine)
{ static char *Series_Name = NULL;
  static int   Series_Max = 0;

  if (n > Series_Max)
    { Series_Max = 1.2*n + 50;
      Series_Name = (char *) Guarded_Realloc(Series_Name,n+1,routine);
    }
  return (Series_Name);
}

static Dimn_Type image_depth(char *routine)
{ Tiff_Reader *reader;
  Dimn_Type    depth;
  int         *tag, count;
  Tiff_Type    type;
  Tiff_IFD    *ifd;
  char        *file_name = (char *) Reader_Source;

  reader = Open_Tiff_Reader(file_name,NULL,NULL,
                            strcmp(file_name+(strlen(file_name)-4),".lsm") == 0);
  if (reader == NULL)
    { sprintf(Error_String,"Error opening %s: '%s' (%s)\n",file_name,Tiff_Error_String(),routine);
      return (0);
    }
  depth = 0;
  while (! End_Of_Tiff(reader))
    { ifd = Read_Tiff_IFD(reader);
      if (ifd == NULL)
        { sprintf(Error_String,"Error reading Tif IFD: '%s' (%s)\n",
                               Tiff_Error_String(),routine);
          return (0);
        }
      tag = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
      if (tag == NULL || (*tag & TIFF_VALUE_REDUCED_RESOLUTION) == 0)
        depth += 1;
      Free_Tiff_IFD(ifd);
    }
  Free_Tiff_Reader(reader);
  if (depth == 0)
    sprintf(Error_String,"Tif file %s has 0 planes within it! (%s)\n",file_name,routine);
  return (depth);
}

static Dimn_Type series_depth(char *routine)
{ FILE        *fd;
  char        *sname;
  Dimn_Type    depth;
  File_Bundle *bundle = (File_Bundle *) Reader_Source;

  sname = get_series_name(strlen(bundle->prefix)+50+strlen(bundle->suffix),routine);
  for (depth = 0; 1; depth += 1) 
    { sprintf(sname,"%s%0*d%s",bundle->prefix,bundle->num_width,
                               bundle->first_num+depth,bundle->suffix);
      if ((fd = fopen(sname,"r")) == NULL)
        break;
      fclose(fd);
    }
  if (depth == 0)
    sprintf(Error_String,"Tif series has no files! (%s)\n",routine);
  return (depth);
}

static Tiff_Reader *image_reader(int state, char *routine)
{ static Tiff_Reader *tif;
  char               *file_name = (char *) Reader_Source;

  if (state == 0)
    { tif = Open_Tiff_Reader(file_name,NULL,NULL,
                             strcmp(file_name+(strlen(file_name)-4),".lsm") == 0);
      if (tif == NULL)
        { sprintf(Error_String,"Error opening %s: '%s' (%s)\n",
                               file_name,Tiff_Error_String(),routine);
          return (NULL);
        }
    }
  else if (state < 0)
    Free_Tiff_Reader(tif);
  return (tif);
}

static Tiff_Reader *series_reader(int state, char *routine)
{ static Tiff_Reader *tif;
  static char        *sname;
  static Dimn_Type    index;
  File_Bundle        *bundle = (File_Bundle *) Reader_Source;

  if (state == 0)
    { sname = get_series_name(strlen(bundle->prefix)+50+strlen(bundle->suffix),routine);
      index = bundle->first_num;
    }
  else
    Free_Tiff_Reader(tif);
  if (state >= 0)
    { sprintf(sname,"%s%0*u%s",bundle->prefix,bundle->num_width,index,bundle->suffix);
      tif = Open_Tiff_Reader(sname,NULL,NULL,strcmp(sname+(strlen(sname)-4),".lsm") == 0);
      if (tif == NULL)
        { sprintf(Error_String,"Error reading %s: '%s' (%s)\n",
                               sname,Tiff_Error_String(),routine);
          return (NULL);
        }
      index += 1;
    }
  return (tif);
}

Dimn_Type get_Tiff_Depth_mylib(char *filename)
{
    Reader_Source = filename;
    return image_depth("Read_Images");
}

Layer_Bundle * read_One_Tiff_ZSlice(char * filename,
                                    Dimn_Type zsliceno //read_handler will be "Read_Images"
                                    )
{
    if (!filename || zsliceno<0)
        return NULL;

    Dimn_Type totalNSlices = get_Tiff_Depth_mylib(filename);
    if (zsliceno>=totalNSlices)
        return NULL;

    Reader_Source = filename;

    Tiff_Reader *tif = image_reader(0, "read_One_Tiff_ZSlice");
    if (tif == NULL)
        return (NULL);
/*
    //skip the "zslice-1" planes

    Tiff_IFD    *ifd;
    Tiff_Image  *img;

    Dimn_Type d=0;
    while (d<zsliceno-1)
    {
        ifd = Read_Tiff_IFD(tif);
        if (ifd == NULL)
        {
            sprintf(Error_String,"Error reading Tif IFD: '%s' in read_One_Tiff_ZSlice()\n",Tiff_Error_String());
            return (NULL);
        }
        d++;
        //printf("(d=%ld totalNslice=%ld) ", d, totalNSlices);fflush(stdout);  if (d%20 == 0) printf("\n");

        //in the future here may be inserted a direct call to extract slice
    }

    ifd = Read_Tiff_IFD(tif);
    if (ifd == NULL)
    {
        sprintf(Error_String,"Error reading Tif IFD: '%s' read_One_Tiff_ZSlice()\n",Tiff_Error_String());
        return (NULL);
    }
    d++;
    printf("\n d=%ld\n", d);fflush(stdout);

    img = Get_Tiff_Image(ifd);
    if (img == NULL)
    {
        sprintf(Error_String,"Error reading Tif Image: '%s' read_One_Tiff_ZSlice()\n",
                Tiff_Error_String());
        Free_Tiff_IFD(ifd);
        goto cleanup;
    }

    //Write_Image(char *file_name, Array *image, int compress)

    int nlayers, cidx, kind, layer, lidx;

    nlayers = 0;
    for (cidx = 0; cidx < img->number_channels; cidx += kind_size[kind])
    {
        kind = determine_kind(img,cidx);
        if (layer == nlayers)
          break;
        nlayers += 1;
    }
    if (layer >= 0)
    {
        if (cidx >= img->number_channels)
        {
            fprintf(stderr,"Layer %d does not exit in tiff (%s)\n",layer,routine);
            exit (1);
        }
        lidx = cidx;
        nlayers += 1;
    }

    if (nlayers > Max_Layers)
    {
        Max_Layers = nlayers + 5;
        images.layers = (Array **)
                          Guarded_Realloc(images.layers,sizeof(Array *)*Max_Layers,routine);
        area   = (Size_Type *)
                          Guarded_Realloc(area,(sizeof(Size_Type)+sizeof(int))*Max_Layers,routine);
        invert = (int *) (area + Max_Layers);
    }

    if (img->number_channels > Max_Planes)
    {
        Max_Planes = img->number_channels + 5;
        planes     = (void **) Guarded_Realloc(planes,sizeof(void *)*Max_Planes,routine);
    }

    {
        int i;
        for (i = 0; i < img->number_channels; i++)
            planes[i] = NULL;
    }

    images.num_layers = nlayers;

    array  = images.layers;
    width  = img->width;
    height = img->height;

    dims[0] = width;
    dims[1] = height;
    dims[2] = depth;

    if (layer >= 0)
      { cidx = lidx; lnum = layer; }
    else
      lnum = cidx = 0;
    for ( ; lnum < nlayers; lnum++, cidx += kind_size[kind])
      { kind         = determine_kind(img,cidx);
        type         = determine_type(img,cidx);
        scale        = img->channels[cidx]->scale;
        invert[lnum] = (img->channels[cidx]->interpretation == CHAN_WHITE);
        area[lnum]   = (((Indx_Type) width) * height) * type_size[type];
        array[lnum]  = Make_Array(kind,type,2 + (depth != 1),dims);
        array[lnum]->scale = scale;
      }

    map = NULL;
    if (img->channels[0]->interpretation == CHAN_MAPPED)
    {
        int dom = (1 << img->channels[0]->scale);

        dims[0] = dom;
        map = Make_Array(RGB_KIND,UINT16,1,dims);

        memcpy(map->data,img->map,map->size*type_size[UINT16]);
    }

    if ((text = (char *) Get_Tiff_Tag(ifd,TIFF_JF_ANO_BLOCK,&type,&count)) == NULL)
        text = Empty_String;
    if (layer > 0)
        Set_Array_Text(array[layer],text);
    else
        Set_Array_Text(array[0],text);

    //read the data

    Layer_Bundle *images = new Layer_Bundle;
    Indx_Type   *area   = NULL;
    int         *invert = NULL;
    int          Max_Layers = 0;
    void       **planes = NULL;
    int          Max_Planes = 0;

    Array      **array;
    Array       *map;
    int          cidx, lidx, lnum, nlayers;
    Dimn_Type    width, height;
    int          kind, scale;
    Tiff_Type    type;
    Tiff_Reader *tif;
    Tiff_IFD    *ifd;
    Tiff_Image  *img;
    char        *text;
    int          count;
    Dimn_Type    dims[4];

    *pmap = NULL;

    images.num_layers = nlayers;

    {
        int       i;
        Dimn_Type d;

        d = 0;
        while (1)
        {
            Tiff_Channel **chan = img->channels;

            if (layer >= 0)
            {
                cidx = lidx; lnum = layer;
            }
            else
                lnum = cidx = 0;
            for ( ; lnum < nlayers; lnum++)
            {
                Indx_Type base;

                kind = array[lnum]->kind;
                if (kind == RGB_KIND)
                    for (i = 0; i < 3; i++)
                    {
                        base = channel_order(chan[cidx+i]);
                        planes[cidx+i] = array[lnum]->data+area[lnum]*(d+depth*base);
                    }
                else if (kind == RGBA_KIND)
                    for (i = 0; i < 4; i++)
                    {
                        base = channel_order(chan[cidx+i]);
                        planes[cidx+i] = array[lnum]->data+area[lnum]*(d+depth*base);
                    }
                else // kind == PLAIN_KIND
                    planes[cidx] = array[lnum]->data+area[lnum]*d;
                cidx += kind_size[kind];
            }
            Load_Tiff_Image_Planes(img, planes);

            Free_Tiff_Image(img);
            Free_Tiff_IFD(ifd);

            d += 1;
            if (d >= depth) break;

            tif = read_handler(1,routine);
            if (tif == NULL)
                goto cleanup;

            while (1)
            {
                int *tag;

                ifd = Read_Tiff_IFD(tif);
                if (ifd == NULL)
                {
                    sprintf(Error_String,"Error reading Tif IFD: '%s' (%s)\n",
                            Tiff_Error_String(),routine);
                    goto cleanup;
                }
                tag = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
                if (tag == NULL || (*tag & TIFF_VALUE_REDUCED_RESOLUTION) == 0)
                    break;
                Free_Tiff_IFD(ifd);
            }
            img = Get_Tiff_Image(ifd);
            if (img == NULL)
            {
                sprintf(Error_String,"Error reading Tif Image: '%s' (%s)\n",
                        Tiff_Error_String(),routine);
                Free_Tiff_IFD(ifd);
                goto cleanup;
            }

            if (img->width != width || img->height != height)
            {
                sprintf(Error_String,
                        "Planes of a stack are not of the same dimensions (%s)!\n",routine);
                Free_Tiff_Image(img);
                Free_Tiff_IFD(ifd);
                goto cleanup;
            }

            if (layer >= 0)
            {
                cidx = lidx; lnum = layer;
            }
            else
                lnum = cidx = 0;
            for ( ; lnum < nlayers; lnum++)
            {
                kind = array[lnum]->kind;
                if (determine_type(img,cidx) != array[lnum]->type ||
                        determine_kind(img,cidx) != kind ||
                        img->channels[cidx]->scale != array[lnum]->scale)
                {
                    sprintf(Error_String,"Planes of a stack are not of the same type (%s)!\n",routine);
                    Free_Tiff_Image(img);
                    Free_Tiff_IFD(ifd);
                    goto cleanup;
                }
                cidx += kind_size[kind];
            }
        }
    }

    read_handler(-1,routine);

    if (layer >= 0)
    { cidx = lidx; lnum = layer; }
    else
        lnum = cidx = 0;
    for ( ; lnum < nlayers; lnum++)
    {
        if (invert[lnum])
        {
            double max;
            if (array[lnum]->type <= UINT32)
                max = ((((uint64) 1) << array[lnum]->scale) - 1);
            else if (array[lnum]->type <= INT32)
                max = ((((uint64) 1) << (array[lnum]->scale-1)) - 1);
            else
                max = 1.0;
            Scale_Array(array[lnum], -1., -max);
        }
        cidx += kind_size[array[lnum]->kind];
    }

    *pmap = map;
    return (&images);

cleanup:
    if (layer >= 0)
    { cidx = lidx; lnum = layer; }
    else
        lnum = cidx = 0;
    for ( ; lnum < nlayers; lnum++)
        Free_Array(array[lnum]);
    if (map != NULL) Free_Array(map);
    *pmap = NULL;

    */

    return (NULL);
}



static void *read_array(Tiff_Reader *(*reader)(int, char *), Dimn_Type (*depthfind)(),
                        char *routine, void *file_source, int layer)
{ Dimn_Type     depth;
  Layer_Bundle *images;
  Array        *map;

  Reader_Source = file_source;
 
  if ((depth = depthfind(routine)) == 0) return (NULL);

  if ((images = read_tiff(routine,depth,reader,&map,layer)) == NULL) return (NULL);

  if (map != NULL)
    { if (layer <= 0)
        { Array *ex = Apply_Map(images->layers[0],map);
          Free_Array(images->layers[0]);
          images->layers[0] = ex;
        }
      Free_Array(map);
    }

  if (layer < 0)
    return (images);
  else
    return (images->layers[layer]);
}

Layer_Bundle *G(Read_Images)(char *file_name)
{ return (read_array(image_reader,image_depth,"Read_Images",file_name,-1)); }

Array *G(Read_Image)(char *file_name, int layer)
{ return (read_array(image_reader,image_depth,"Read_Image",file_name,layer)); }

Layer_Bundle *G(Read_Planes_Series)(File_Bundle *bundle)
{ return (read_array(series_reader,series_depth,"Read_Planes_Series",bundle,-1)); }

Array *G(Read_Plane_Series)(File_Bundle *bundle, int layer)
{ return (read_array(series_reader,series_depth,"Read_Planes_Series",bundle,layer)); }


/****************************************************************************************
 *                                                                                      *
 *  CENTRAL WRITE ROUTINE                                                               *
 *                                                                                      *
 ****************************************************************************************/

  // Write each successive plane of the list of layers in images to the Tiff_Writer returned
  //   by write_handler for each plane number.  If map is not NULL, then the first channel/layer
  //   is coded as being color-mapped in the tiff.  The originating external routine has name
  //   'routine'.  One calls write_handler(-1) to finish the write of the file or series of
  //   files.  The text of the first layer is written to the JF-TAGGER field of the (first) tif.

static int atype2class[] = { CHAN_UNSIGNED, CHAN_UNSIGNED, CHAN_UNSIGNED,
                             CHAN_SIGNED,   CHAN_SIGNED,   CHAN_SIGNED,
                             CHAN_FLOAT,    CHAN_FLOAT };

static void write_tiff(char *routine, Tiff_Writer *(*write_handler)(int),
                       Layer_Bundle *images, Array *map, int compress)
{ int          ndims, color;
  Dimn_Type    depth;
  int          kind, scale, class;
  Tiff_Image  *img;
  Array      **array;
  int          cidx, lnum, nlayers;

  nlayers = images->num_layers;

  array = images->layers;
  color = (array[0]->kind != PLAIN_KIND);
  ndims = array[0]->ndims - color;
  if (ndims == 2)
    depth = 1;
  else
    depth = array[0]->dims[2];
  if ( ndims != 2 && ndims != 3)
    { fprintf(stderr,"Array is not an image or a stack (%s)\n",routine);
      exit (1);
    }

  for (lnum = 0; lnum < nlayers; lnum++)
    { color = (array[lnum]->kind != PLAIN_KIND);
      if (ndims != array[lnum]->ndims - color)
        { fprintf(stderr,"Layers are not all of the same dimensionality (%s)\n",routine);
          exit (1);
        }
      if (color && array[lnum]->dims[ndims] != kind_size[array[lnum]->kind])
        { fprintf(stderr,"Kind and outer dimension are inconsistent (%s)\n",routine);
          exit (1);
        }
      if (array[lnum]->scale > 32)
        { fprintf(stderr,"Tif format cannot handle values of more than 32 bits (%s)\n",routine);
          exit (1);
        }
      if (array[lnum]->dims[0] != array[0]->dims[0] ||
          array[lnum]->dims[1] != array[0]->dims[1] ||
          ndims > 2 && array[lnum]->dims[2] != array[0]->dims[2])
        { fprintf(stderr,"Layers do not all have the same dimensions (%s)\n",routine);
          exit (1);
        }
    }

  if (map != NULL)
    { if (array[0]->kind != PLAIN_KIND)
        { fprintf(stderr,"Layer 0 is a color image, yet has a color map (%s)\n",routine);
          exit (1);
        }
      if (map->ndims != 2)
        { fprintf(stderr,"Color map is not of dimension 2 (%s)\n",routine);
          exit (1);
        }
      if (map->kind != RGB_KIND)
        { fprintf(stderr,"Color map is not an RGB_KIND (%s)\n",routine);
          exit (1);
        }
      if (map->type != UINT16)
        { fprintf(stderr,"Color map is not a UINT16 (%s)\n",routine);
          exit (1);
        }
      if (map->dims[1] != 3)
        { fprintf(stderr,"Color map does not have outer dimension 3 (%s)\n",routine);
          exit (1);
        }
      if (map->dims[0] != (1 << array[0]->scale))
        { fprintf(stderr,"Domain of color map doesn't match range of image (%s)\n",routine);
          exit (1);
        }
    }

  img  = Create_Tiff_Image(array[0]->dims[0],array[0]->dims[1]);
  for (lnum = 0; lnum < nlayers; lnum++)
    { scale = array[lnum]->scale;
      class = atype2class[array[lnum]->type];
      switch (array[lnum]->kind)
        { case RGB_KIND:
          case RGBA_KIND:
            Add_Tiff_Image_Channel(img,CHAN_RED,scale,class,NULL);
            Add_Tiff_Image_Channel(img,CHAN_GREEN,scale,class,NULL);
            Add_Tiff_Image_Channel(img,CHAN_BLUE,scale,class,NULL);
            if (array[lnum]->kind == RGBA_KIND)
              Add_Tiff_Image_Channel(img,CHAN_ALPHA,scale,class,NULL);
            break;
          case PLAIN_KIND:
            if (map != NULL && lnum == 0)
              Add_Tiff_Image_Channel(img,CHAN_MAPPED,scale,class,NULL);
            else
              Add_Tiff_Image_Channel(img,CHAN_BLACK,scale,class,NULL);
            break;
        }
    }

  if (map != NULL && img->channels[0]->interpretation == CHAN_MAPPED)
    memcpy(img->map,map->data,map->size*type_size[UINT16]);

  { int          i;
    Size_Type    a;
    Dimn_Type    d;
    Tiff_Writer *tif;
    Tiff_IFD    *ifd;

    for (d = 0; d < depth; d++)
      { tif = write_handler(d > 0);

        cidx = 0;
        for (lnum = 0; lnum < nlayers; lnum++)
          { kind = array[lnum]->kind;
            a = array[lnum]->dims[0];
            a = (a * array[lnum]->dims[1]) * type_size[array[lnum]->type];
            if (kind == RGB_KIND)
              for (i = 0; i < 3; i++)
                img->channels[cidx+i]->plane = array[lnum]->data + a*(d+i*depth);
            else if (kind == RGBA_KIND)
              for (i = 0; i < 4; i++)
                img->channels[cidx+i]->plane = array[lnum]->data + a*(d+i*depth);
            else // kind == PLAIN_KIND
              img->channels[cidx]->plane = array[lnum]->data + a*d;
            cidx += kind_size[kind];
          }

        ifd = Make_IFD_For_Image(img,compress,0,0);

        if (d == 0 && array[0]->text[0] != '\0')
          Set_Tiff_Tag(ifd,TIFF_JF_ANO_BLOCK,TIFF_BYTE,strlen(array[0]->text),array[0]->text);

        Write_Tiff_IFD(tif,ifd);

        Free_Tiff_IFD(ifd);
      }
  }

  Free_Tiff_Image(img);
  write_handler(-1);
}


/****************************************************************************************
 *                                                                                      *
 *  WRITE ROUTINES (EXTERNAL INTERFACES)                                                *
 *                                                                                      *
 ****************************************************************************************/

static Layer_Bundle WList;
static Array       *AList[1];

static Tio *Writer_Tio;

static Tiff_Writer *tiff_writer(int state)
{ return (Writer_Tio->writer); } 

void Write_Tiff(Tiff *etif, Array *image, int compress)
{ WList.num_layers = 1;
  WList.layers     = AList;
  AList[0]         = image;

  if (image->ndims != 2 + (image->kind != PLAIN_KIND))
    { fprintf(stderr,"Array should be a 2D image (Write_Tiff)\n");
      exit (1);
    }

  Writer_Tio = (Tio *) etif;
  write_tiff("Write_Tiff",tiff_writer,&WList,Get_Map(),compress);
}

void Write_Tiffs(Tiff *etif, Layer_Bundle *images, int compress)
{ int i;
  for (i = 0; i < images->num_layers; i++)
    if (images->layers[i]->ndims != 2 + (images->layers[i]->kind != PLAIN_KIND))
      { fprintf(stderr,"Array for layer %d should be a 2D image (Write_Tiffs)\n",i);
        exit (1);
      }

  Writer_Tio = (Tio *) etif;
  write_tiff("Write_Tiffs",tiff_writer,images,Get_Map(),compress);
}

static char *Writer_File;

static Tiff_Writer *image_writer(int state)
{ static Tiff_Writer *tif;

  if (state == 0)
    tif = Open_Tiff_Writer(Writer_File,0,0);
  else if (state < 0)
    Free_Tiff_Writer(tif);
  return (tif);
}

void Write_Image(char *file_name, Array *image, int compress)
{ WList.num_layers = 1;
  WList.layers     = AList;
  AList[0]         = image;

  Writer_File = file_name;
  write_tiff("Write_Image",image_writer,&WList,NULL,compress);
}

void Write_Images(char *file_name, Layer_Bundle *images, int compress)
{ Writer_File = file_name;
  write_tiff("Write_Images",image_writer,images,NULL,compress);
}

static File_Bundle *Writer_Bundle;
static char        *Write_Name;

static Tiff_Writer *plane_writer(int state)
{ static Tiff_Writer *tif;
  static Dimn_Type    index;

  if (state == 0)
    index = Writer_Bundle->first_num;
  else
    Free_Tiff_Writer(tif);
  if (state >= 0)
    { if (Writer_Bundle->padded)
        sprintf(Write_Name,"%s%0*u%s",Writer_Bundle->prefix,Writer_Bundle->num_width,
                           index,Writer_Bundle->suffix);
      else
        sprintf(Write_Name,"%s%u%s",Writer_Bundle->prefix,index,Writer_Bundle->suffix);
      tif    = Open_Tiff_Writer(Write_Name,0,0);
      index += 1;
    }
  return (tif);
}

void Write_Plane_Series(File_Bundle *bundle, Array *image, int compress)
{ WList.num_layers = 1;
  WList.layers     = AList;
  AList[0]         = image;

  Writer_Bundle = bundle;
  Write_Name    = get_series_name(strlen(bundle->prefix)+50+strlen(bundle->suffix),
                                  "Write_Plane_Series");
  write_tiff("Write_Plane_Series",plane_writer,&WList,NULL,compress);
}

void Write_Planes_Series(File_Bundle *bundle, Layer_Bundle *images, int compress)
{ Writer_Bundle = bundle;
  Write_Name    = get_series_name(strlen(bundle->prefix)+50+strlen(bundle->suffix),
                                  "Write_Planes_Series");
  write_tiff("Write_Plane_Series",plane_writer,images,NULL,compress);
}
