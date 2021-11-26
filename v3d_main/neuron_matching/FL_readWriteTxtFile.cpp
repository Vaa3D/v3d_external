/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




// read text file and put in a data structure
// F. Long
// 20090109
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>

//run -I test.swc -i test.swc -o res.txt

// print data read into dataArray
void print_dataArray(float *dataArray, V3DLONG fieldnum, V3DLONG lineNum)
{
	for (int j=0; j<lineNum; j++)
	{
		for (int i=0; i<fieldnum; i++)
			printf("%f ", *(dataArray+j*fieldnum+i));
		printf("\n");
	}
	printf("\n");
	
}

//read text file into dataArray
bool readtxtfile(char *infilename, float *&dataArray, V3DLONG &lineNum, V3DLONG fieldnum, bool tagprint)
{
	FILE *file=0;
	char buffer[1001];// assumming each line has at most 1000 characters
//	V3DLONG fieldnum = 7; //note here I am directly use 7 for swc files, for other files, this need to be changed, this is a temporary solution,
//					   // the final solution is to use to replace sscanf with a function to parse each character of the line
	
    dataArray = new float [1000*fieldnum]; // assuming 1000 lines and each line has fieldnum of iterms
	lineNum = 0;
	
	file = fopen(infilename, "rt");
	
	if (file==NULL)
		printf("Error opening the file\n");
		
	if (fgets(buffer,1000,file) == NULL) //until 999 characters have been read or either a newline or a the End-of-File is reached, whichever comes first.
	{
		printf("The file is empty!\n");
			return false;
	}
	
	do
	{
		lineNum++;
		for (int i=0; i<fieldnum; i++)
		{	
//			for (int j=0; j<100; j++)
//				printf("%c", buffer[j]);
				
//			if (sscanf(buffer,"%g %g %g %g %g %g %g", dataArray+(lineNum-1)*fieldnum + i)!=1)
			if (sscanf(buffer,"%g", dataArray+(lineNum-1)*fieldnum + i)!=1)
			{
				printf("Line %d has less than %d iterms", lineNum, fieldnum);
				return false;
			}
		}
	} while (fgets(buffer,1000,file) != NULL);
	
	if (tagprint) print_dataArray(dataArray, fieldnum, lineNum);
	
	return true;
}

//write an array to a text file
bool writetxtfile(char *outfilename, float *dataArray, V3DLONG fieldnum, V3DLONG linenum)
{
	int i,j;
	FILE *file;
	
	file = fopen(outfilename, "wt");
	
	for (i=0; i<linenum; i++)
	{
		for (j=0; j<fieldnum; j++)
		{
			fprintf(file, "%g ", dataArray[i*fieldnum+j]);
		}
		fprintf(file, "\n");
	}
	
	return true;
	
}

