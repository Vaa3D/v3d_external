
//by Hanchuan Peng
//2006-2011

#include "convert_type2uint8.h"

#include <stdio.h>

bool convert_type2uint8_3dimg_1dpt(void * &img, V3DLONG * sz, int datatype)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to convert_type2uint8_3dimg_1dpt() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (datatype!=2 && datatype!=4)
	{
		fprintf(stderr, "This function convert_type2uint8_3dimg_1dpt() is designed to convert 16 bit and single-precision-float only.\n", __FILE__, __LINE__);
		return false;
	}

	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1 || sz[0]>2048 || sz[1]>2048 || sz[2]>1024 || sz[3]>10)
	{
		fprintf(stderr, "Input image size is not valid or too large [%s][%d] sz[0,1,2,3]=[%ld, %ld, %ld, %ld].\n", __FILE__, __LINE__, sz[0], sz[1], sz[2], sz[3]);
		return false;
	}

	V3DLONG totalunits = sz[0] * sz[1] * sz[2] * sz[3];
	unsigned char * outimg = new unsigned char [totalunits];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (datatype==2)
	{
		unsigned short int * tmpimg = (unsigned short int *)img;
		for (V3DLONG i=0;i<totalunits;i++)
		{
			outimg[i] = (unsigned char)(tmpimg[i]>>4); 
		}
	}
	else
	{
		float * tmpimg = (float *)img;
		for (V3DLONG i=0;i<totalunits;i++)
		{
			outimg[i] = (unsigned char)(tmpimg[i]*255); 
		}
	}

	//copy to output data

	delete [] ((unsigned char *)img); 
	img = outimg;

	return true;
}


