#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"
#include "tiff.io.h"

static char *Spec[] = { "[-m<int>to<int>] <in:file> <out:file>", NULL };

int main(int argc, char *argv[])
{ char        *in, *out;
  Tiff_Reader *reader;
  Tiff_Writer *writer;
  Tiff_IFD    *ifd;
  int          flag64, first, source, target;

  Process_Arguments(argc,argv,Spec,0);

  in  = Get_String_Arg("in");
  out = Get_String_Arg("out");

  reader = Open_Tiff_Reader(in,NULL,&flag64,strcmp(in+(strlen(in)-4),".lsm") == 0);
  if (reader == NULL)
    { fprintf(stderr,"Error opening tif %s:\n  %s\n",in,Tiff_Error_String());
      exit (1);
    }

  writer = Open_Tiff_Writer(out,flag64,0);
  if (writer == NULL)
    { fprintf(stderr,"Error opening tif %s:\n  %s\n",out,Tiff_Error_String());
      exit (1);
    }

  first  = 1;
  while ( ! End_Of_Tiff(reader))
    { ifd = Read_Tiff_IFD(reader);
      if (ifd == NULL)
        { fprintf(stderr,"Error reading IFD:\n  %s\n",Tiff_Error_String());
          exit (1);
        }
      if (first)
        { first = 0;
          if (Is_Arg_Matched("-m"))
            { source = Get_Int_Arg("-m",1);
              target = Get_Int_Arg("-m",2);
              if (source < 0 || source > 1)
                { fprintf(stderr,"Source is not 0 or 1\n"); exit (1); }
              if (target < 0 || target > 2)
                { fprintf(stderr,"Target is not 0, 1, or 2\n"); exit (1); }
            }
          else if (Get_Tiff_Tag(ifd,TIFF_CZ_LSM_INFO,NULL,NULL) != NULL)
            { int *colors, nchannels;

              colors = Get_LSM_Colors(ifd,&nchannels);   //  Figure out which channel is green
              for (source = 0; source < nchannels; source++) //    and map to green in the RGB
                if ((colors[source] & 0xff00) != 0)
                  break;
              if (source >= nchannels)
                source = 0;
              target = 1;
             }
           else
             { source = 0;
               target = 1;
             }
        }
      if (Convert_2_RGB(ifd,source,target) != NULL)
        { if (Write_Tiff_IFD(writer,ifd))
            { fprintf(stderr,"Error writing IFD:\n  %s\n",Tiff_Error_String());
              exit (1);
            }
        }
      else
        { fprintf(stderr,"Error adding extra channel:\n  %s\n",Tiff_Error_String());
          exit (1);
        }
      Free_Tiff_IFD(ifd);
    }
  Free_Tiff_Writer(writer);

  exit (0);
}
