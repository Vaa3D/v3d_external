#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"
#include "tiff.io.h"

static char *Spec[] = { "<in:file>", NULL };

int main(int argc, char *argv[])
{ char        *in;
  Tiff_Reader *reader;
  Tiff_IFD    *ifd;
  int          endian, flag64, cnt;

  Process_Arguments(argc,argv,Spec,0);

  in  = Get_String_Arg("in");

  reader = Open_Tiff_Reader(in,&endian,&flag64,strcmp(in+(strlen(in)-4),".lsm") == 0);
  if (reader == NULL)
    { fprintf(stderr,"Error opening tif:\n  %s\n",Tiff_Error_String());
      exit (1);
    }

  printf("\nTiff dump of %s: ",in);
  if (endian)
    printf("Big endian, ");
  else
    printf("Little endian, ");
  if (flag64)
    printf("64-bit offsets");
  else
    printf("32-bit offsets");
  printf("\n");

  cnt = 1;
  while ( ! End_Of_Tiff(reader))
    { ifd = Read_Tiff_IFD(reader);
      if (ifd == NULL)
        fprintf(stderr,"Error reading IFD %d:\n  %s\n",cnt,Tiff_Error_String());
      else
        { printf("\nIFD %d (%d tags):\n",cnt,Count_Tiff_Tags(ifd));
          Print_Tiff_IFD(ifd,stdout,4);
          Free_Tiff_IFD(ifd);
        }
      cnt += 1;
    }

  exit (0);
}
