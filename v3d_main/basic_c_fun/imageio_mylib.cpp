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




//100817. by PHC

#include <stdio.h>
#include <string.h>

#include "v3d_basicdatatype.h"
#include "imageio_mylib.h"

extern "C" {
#include "/Users/pengh/work/genelib/mylib/image.h"
};

int loadTif2StackMylib(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int &nbits)
{
	if (!filename) 
		return 1;
	
	//read data
	
	Array * indata = Read_Image(filename, 0);
	if (!indata)
		return 1;

	//prepare the output buffer
	if (img)
	{
		printf("Warning: The pointer for 1d data storage is not empty. This pointer will be freed first and the  reallocated. \n");
		delete []img; img=0;
	}
	if (sz)
	{
		printf("Warning: The pointer for size is not empty. This pointer will be freed first and the  reallocated. \n");
		delete []sz; sz=0;
	}

	try
	{
		sz = new V3DLONG [4];
	}
	catch(...)
	{
		printf("Fail to alocate memory for the size variable.\n");
		return false;
	}
	
	//now copy data
	
	switch (indata->type)
	{
		case UINT8:		datatype=1; break;
		case UINT16:	datatype=2; break;
		case FLOAT32:	datatype=4; break;
		default:
			fprintf(stderr, "Unsupport data type detected in loadTif2StackMylib().\n");
			return 1;
	}
	
	nbits = indata->scale;
	
	V3DLONG i, totalunits=1;
	for (i=0;i<indata->ndims;i++)
	{
		sz[i] = indata->dims[i];
		totalunits *= sz[i];
	}
	for (i=indata->ndims;i<4;i++)
	{
		sz[i] = 1;
	}
	
	V3DLONG totallen = totalunits * datatype;
	try
	{
		img = new unsigned char [totallen];
	}
	catch(...)
	{
		fprintf(stderr, "Fail to allocate memory in loadTif2StackMylib().");
		if (sz) {delete []sz; sz=0;}
		return 1;
	}
	
	memcpy(img, (unsigned char*)indata->data, totallen); //copy data

	if (indata) {Kill_Array(indata); indata=0;} //free the space
	return 0;
}

int loadTif2StackMylib(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int & nbits, int chan_id_to_load) //overload for convenience to read only 1 channel
{
	return 0;
}
int saveStack2TifMylib(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype)
{
	return 0;
}






