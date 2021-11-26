#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"
#include "tiff.io.h"

static char *Spec[] = { "[-x] [-64] [-r<int>,<int>] <in:file> <out:file>", NULL };

int main(int argc, char *argv[])
{ char        *in;
  char        *out;
  int          low, hgh;
  Tiff_Reader *reader;
  Tiff_Writer *writer;
  Tiff_IFD    *ifd, *orig;
  int          write, cnt;

  Process_Arguments(argc,argv,Spec,0);

  in  = Get_String_Arg("in");
  out = Get_String_Arg("out");
  if (Is_Arg_Matched("-r"))
    { low = Get_Int_Arg("-r",1);
      hgh = Get_Int_Arg("-r",2);
    }
  else
    { low = 1;
      hgh = 0x7fffffff;
    }

  reader = Open_Tiff_Reader(in,NULL,NULL,strcmp(in+(strlen(in)-4),".lsm") == 0);
  if (reader == NULL)
    { fprintf(stderr,"Error opening tif:\n  %s\n",Tiff_Error_String());
      exit (1);
    }

  writer = Open_Tiff_Writer(out,Is_Arg_Matched("-64"),strcmp(out+(strlen(out)-4),".lsm") == 0);
  if (writer == NULL)
    { fprintf(stderr,"Error opening tif:\n  %s\n",Tiff_Error_String());
      exit (1);
    }

  cnt  = 0;
  orig = NULL;
  while ( ! End_Of_Tiff(reader))
    { ifd = Read_Tiff_IFD(reader);
      if (ifd == NULL)
        fprintf(stderr,"Error reading IFD:\n  %s\n",Tiff_Error_String());
      else
        { write = 1;
          if (Is_Arg_Matched("-x"))
            { int      *tag, count;
              Tiff_Type type;

              tag   = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
              if (tag == NULL)
                write = 0;
              else if ((*tag & TIFF_VALUE_REDUCED_RESOLUTION) != 0)
                write = 0;
              else
                cnt += 1;
            }
          else
            cnt += 1;
          if (cnt < low || cnt > hgh)
            write = 0;
          if (write)
            { if (cnt == low && orig != 0)
                { int *tag, count;
                  Tiff_Type type;

                  tag = (int *) Get_Tiff_Tag(orig,TIFF_CZ_LSM_INFO,&type,&count);
                  if (tag != NULL)
                    Set_Tiff_Tag(ifd,TIFF_CZ_LSM_INFO,type,count,tag);
                  Free_Tiff_IFD(orig);
                }
              if (Write_Tiff_IFD(writer,ifd))
                fprintf(stderr,"Error writing IFD:\n  %s\n",Tiff_Error_String());
            }
          if (cnt == 1 && !write)
            { if (orig == NULL)
                orig = ifd;
            }
          else
            Free_Tiff_IFD(ifd);
        }
    }
  Free_Tiff_Writer(writer);

  exit (0);
}
