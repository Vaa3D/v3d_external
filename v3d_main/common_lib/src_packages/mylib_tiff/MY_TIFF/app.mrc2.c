#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"
#include "tiff.io.h"
#include "tiff.image.h"

static char *Spec[] = { "<in:file> ...", NULL };

static void flip_short(void *w)
{ unsigned char *v = (unsigned char *) w; 
  unsigned char  x;
    
  x    = v[0];
  v[0] = v[1];
  v[1] = x;
}   

static void flip_long(void *w)
{ unsigned char *v = (unsigned char *) w;
  unsigned char  x;

  x    = v[0];
  v[0] = v[3];
  v[3] = x;
  x    = v[1];
  v[1] = v[2];
  v[2] = x;
}

void convert(char *iname, char *oname)
{ FILE        *input;
  Tiff_Writer *writer;
  Tiff_IFD    *ifd;
  Tiff_Image  *image;
  void        *plane;
  int          endian, mode, psize, dims[256];
  int          i, j;

  input = Guarded_Fopen(iname,"r",Program_Name());

  if (fread(dims,sizeof(int),256,input) != 256)
    { fprintf(stderr,"File does not appear to be an .mrc file\n");
      exit (1);
    }

  mode = dims[3];
  if (mode < 0 || mode > 4)
    { flip_long(&mode);
      if (mode < 0 || mode > 4)
        { fprintf(stderr,"File does not appear to be an .mrc file\n");
          exit (1);
        }
      flip_long(dims);
      flip_long(dims+1);
      flip_long(dims+2);
      endian = 1;
    }
  else
    endian = 0;

  if (mode > 2)
    { fprintf(stderr,"Do not support complex/transformed images\n");
      exit (1);
    }
  
  writer = Open_Tiff_Writer(oname,0,0);
  if (writer == NULL)
    { fprintf(stderr,"Error opening tif %s:\n  %s\n",Get_String_Arg("out"),Tiff_Error_String());
      exit (1);
    }
  if (mode == 0)
    mode = 1;
  else if (mode == 1)
    mode = 2;
  else if (mode == 2)
    mode = 4;

  if (endian)
    { for (j = 3; j < 56; j++)
        flip_long(dims+j);
    }

  psize = dims[0]*dims[1]*mode;
  image = Create_Tiff_Image(dims[0],dims[1]);
  plane = Guarded_Malloc(psize,Program_Name());
  if (mode == 4)
    Add_Tiff_Image_Channel(image,CHAN_BLACK,8*mode,CHAN_FLOAT,plane);
  else
    Add_Tiff_Image_Channel(image,CHAN_BLACK,8*mode,CHAN_UNSIGNED,plane);

  for (i = 0; i < dims[2]; i++)
    { if (fread(image->channels[0]->plane,1,psize,input) != psize)
        { fprintf(stderr,"File does not appear to be an .mrc file\n");
          exit (1);
        }
      if (endian)
        { if (mode == 2)
            { unsigned short *splane = image->channels[0]->plane;
              for (j = 0; j < dims[0]*dims[1]; j++)
                flip_short(splane+j);
            }
          else if (mode == 4)
            { unsigned int *iplane = image->channels[0]->plane;
              for (j = 0; j < dims[0]*dims[1]; j++)
                flip_long(iplane+j);
            }
        }

      if (mode == 4)                    //  Convert 32-float to 16-unsigned
        { double minval, maxval, denom;
          unsigned short *target = (unsigned short *) image->channels[0]->plane;
          float          *source = (float *) image->channels[0]->plane;

          Range_Tiff_Channel(image->channels[0],&minval,&maxval);
          denom = (maxval-minval)/65535.;
          for (i = 0; i < dims[0]*dims[1]; i++)
            target[i] = (unsigned short) ((source[i]-minval)/denom);
          image->channels[0]->scale = 16;
          image->channels[0]->bytes_per_pixel = 2;
          image->channels[0]->type = CHAN_UNSIGNED;
        }

      ifd = Make_IFD_For_Image(image,0,0,0);
      if (i == 0)
        Set_Tiff_Tag(ifd,TIFF_JF_MRC_INFO,TIFF_LONG,256,dims);
      Write_Tiff_IFD(writer,ifd);
      Free_Tiff_IFD(ifd);
    }

  Free_Tiff_Writer(writer);

  exit (0);
}


int main(int argc, char *argv[])
{ char *iname, *oname;
  int   i;

  Process_Arguments(argc,argv,Spec,0);

  for (i = 0; i < Get_Repeat_Count("in"); i++)
    { iname = Get_String_Arg("in",i);
      if (strcmp(iname + strlen(iname) - 4, ".mrc") != 0)
        continue;
      oname = Guarded_Strdup(iname,Program_Name());
      strcpy(oname+strlen(oname)-4,".tif");
      convert(iname,oname);
      free(oname);
    }

  exit (0);
}
