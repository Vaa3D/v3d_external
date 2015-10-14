#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "utilities.h"
#include "tiff.io.h"

static char *Spec[] = { "-f <tif:file> | ",
                        "-g [-o] <tif:file> | ",
                        "-s <tif:file> (<anno:string> | -i <afile:file>)",
                        NULL
                      };

int main(int argc, char *argv[])
{ int   status;
  char *tname;

  Process_Arguments(argc,argv,Spec,0);

  tname  = Get_String_Arg("tif");
  status = Tiff_Annotation_Status(tname);

  if (status == ANNOTATOR_CANT_OPEN)
    { fprintf(stderr,"Cannot open %s\n",tname);
      fprintf(stderr,"  Error: %s\n",Tiff_Error_String());
      exit (1);
    }

  if (status == ANNOTATOR_GIBBERISH)
    { fprintf(stderr,"File %s is not recognizable as a tif or lsm\n",tname);
      fprintf(stderr,"  Error: %s\n",Tiff_Error_String());
      exit (1);
    }

  if (Is_Arg_Matched("-f"))
    { if (status == ANNOTATOR_NOT_FORMATTED)
        { printf("Formatting file\n");
          fflush(stdout);
          Format_Tiff_For_Annotation(tname);
        }
      else
        fprintf(stderr,"File %s is already formatted for annotation\n",tname);
    }
  else
    { Tiff_Annotator *tif;
      int             length;
      char           *anno;

      if (status != ANNOTATOR_FORMATTED)
        { fprintf(stderr,"File %s is not formatted for annotation\n",tname);
          fprintf(stderr,"  Error: %s\n",Tiff_Error_String());
          exit (1);
        }
      tif = Open_Tiff_Annotator(tname);
      if (Is_Arg_Matched("-g"))
        { anno = Get_Tiff_Annotation(tif,&length);
          if (Is_Arg_Matched("-o"))
            fwrite(anno,1,length,stdout);
          else if (length == 0)
            printf("Anno = ''(0)\n");
          else
            printf("Anno = '%.*s'(%d)\n",length,anno,length);
        }
      else
        { if (Is_Arg_Matched("anno"))
            { anno   = Get_String_Arg("anno");
              length = strlen(anno);
            }
          else
            { FILE       *afile;
              struct stat fdesc;

              anno = Get_String_Arg("afile");
              if ((afile = fopen(anno,"rt")) == NULL)
                { fprintf(stderr,"Cannot open %s\n",anno); 
                  exit (1);
                }
              fstat(fileno(afile),&fdesc);
              length = fdesc.st_size;
              anno = Guarded_Malloc(length,Program_Name());
              fread(anno,fdesc.st_size,1,afile);
              fclose(afile);
            }
          Set_Tiff_Annotation(tif,anno,length);
          Free_Tiff_Annotator(tif);
        }
    }

  exit (0);
}
