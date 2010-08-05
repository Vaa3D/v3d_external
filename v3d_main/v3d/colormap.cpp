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




//colormap.cpp
//by Hanchuan Peng
//20080824: separated from the WANO project to reuse in the V3D project
//080825: change a little bit to make sure the colorPseudoMaskColor will be used if a wrong ImageDisplayColorType is accidentally input

#include <stdio.h>
#include "colormap.h"

ColorMap::ColorMap(ImageDisplayColorType c)
{
	initialize(c, 256);
}

ColorMap::ColorMap(ImageDisplayColorType c, int length)
{
	initialize(c, length);
}

ColorMap::~ColorMap()
{
	if (map2d) {delete []map2d; map2d=0;}
	if (map1d) {delete []map1d; map1d=0;}
	len = 0;
}

void ColorMap::initialize(ImageDisplayColorType c, int length)
{
	// Initialize variables
	
	len = length+1;
	ctype = c;
	map1d=0; map2d=0;
	
	// Allocate memory
	
	map1d = new unsigned char [len*3];
	if (!map1d)
	{
		printf("Fail to create colormap map1d!\n");
		len=0;
		return;
	}
	
	map2d = new unsigned char * [len];
	if (!map2d)
	{
		printf("Fail to create colormap map2d!\n");
		if (map1d) {delete []map1d; map1d=0;}
		len=0;
		return;
	}
	
	int i;
	for (i=0;i<len;i++)
	{
		map2d[i] = map1d+V3DLONG(i)*3;
	}
	
	// Set the colormap values
	
	unsigned char simple_cmap[14*3]  = {0,   0,   0,
		255, 255, 255, 
		255, 0,   0,
		0,   255, 0,
		0,   0,   255,
		255, 255, 0,
		0,   255, 255,
		255, 0,   255,
		255, 128,   0,
		128, 255, 0,
		0,   128, 255,
		255, 255, 128,
		128, 255, 255,
		255, 128, 255
	};
	
	//add arnim_cmap on 070717						   
	unsigned char arnim_cmap[52*3] = {  0, 0, 0, 
		254, 0, 0, 
		254, 0, 0, 
		254, 140, 0, 
		254, 140, 0, 
		254, 204, 0, 
		254, 204, 0, 
		102, 25, 0, 
		102, 25, 0, 
		0, 157, 0, 
		152, 254, 0, 
		28, 254, 10, 
		0, 95, 0, 
		40, 115, 203, 
		40, 115, 203, 
		162, 0, 208, 
		162, 0, 208, 
		41, 203, 103, 
		41, 203, 103, 
		170, 203, 41, 
		170, 203, 41, 
		203, 40, 116, 
		203, 40, 116, 
		50, 40, 203, 
		50, 40, 203, 
		203, 40, 48, 
		203, 40, 48, 
		170, 203, 41, 
		170, 203, 41, 
		203, 70, 41, 
		203, 70, 41, 
		0, 102, 254, 
		0, 102, 254, 
		232, 46, 227, 
		232, 46, 227, 
		50, 40, 203, 
		50, 40, 203, 
		241, 215, 23, 
		241, 215, 23, 
		236, 134, 47, 
		236, 134, 47, 
		134, 203, 41, 
		25, 218, 155, 
		25, 218, 155, 
		50, 40, 203, 
		50, 40, 203, 
		203, 40, 40, 
		203, 40, 40, 
		161, 116, 250, 
		161, 116, 250, 
		51, 52, 254, 
		41, 41, 203
	};
	
	//080810
	unsigned char hanchuan_cmap[63*3] = {  
		0,0,0, // 179, 204, 204, //display the first cyan background as black
		255, 0, 0,
		255, 205, 0,
		255, 141, 0,
		102, 25, 0,
		28, 255, 10,
		0, 158, 0,
		40, 41, 204,
		153, 255, 0,
		36, 102, 20,
		50, 40, 204,
		204, 40, 108,
		135, 204, 40,
		40, 66, 204,
		255, 204, 49,
		67, 0, 179,
		40, 204, 115,
		71, 178, 122,
		0, 102, 255,
		204, 40, 149,
		51, 102, 255,
		230, 123, 26,
		0, 95, 0,
		165, 37, 195,
		204, 135, 23,
		255, 226, 3,
		95, 40, 204,
		111, 197, 62,
		50, 237, 255,
		190, 0, 0,
		163, 101, 0,
		72, 204, 40,
		42, 123, 176,
		160, 150, 255,
		255, 0, 0,
		255, 205, 0,
		255, 141, 0,
		102, 25, 0,
		40, 41, 204,
		24, 126, 32,
		36, 102, 20,
		50, 40, 204,
		204, 40, 108,
		40, 66, 204,
		255, 204, 49,
		67, 0, 179,
		40, 204, 115,
		71, 178, 122,
		0, 102, 255,
		204, 40, 149,
		51, 102, 255,
		230, 123, 26,
		165, 37, 195,
		204, 135, 23,
		255, 226, 3,
		95, 40, 204,
		111, 197, 62,
		50, 237, 255,
		190, 0, 0,
		163, 101, 0,
		72, 204, 40,
		42, 123, 176,
		160, 150, 255
	};
	
	
	unsigned char *amap=0;
	V3DLONG colorMapSz;
	register int t;
	
	switch (ctype)
	{
		case colorArnimFlyBrainColor:
			amap = arnim_cmap;
			colorMapSz = sizeof(arnim_cmap)/3; 
			
			if (len<1)
				break;
			
			t=0;
			
			map2d[0][0] = amap[t*3+0];
			map2d[0][1] = amap[t*3+1];
			map2d[0][2] = amap[t*3+2];
			
			for (i=1;i<len;i++)
			{
				t=(i-1)%(colorMapSz-1)+1; //070717
				
				map2d[i][0] = amap[t*3+0];
				map2d[i][1] = amap[t*3+1];
				map2d[i][2] = amap[t*3+2];
				
			}
			break;
			
		case colorHanchuanFlyBrainColor:
			amap = hanchuan_cmap;
			colorMapSz = sizeof(hanchuan_cmap)/3; 
			
			if (len<1)
				break;
			
			t=0;
			
			map2d[0][0] = amap[t*3+0];
			map2d[0][1] = amap[t*3+1];
			map2d[0][2] = amap[t*3+2];
			
			for (i=1;i<len;i++)
			{
				t=(i-1)%(colorMapSz-1)+1; 
				map2d[i][0] = amap[t*3+0];
				map2d[i][1] = amap[t*3+1];
				map2d[i][2] = amap[t*3+2];
				
			}
			break;
			
		case colorPseudoMaskColor:
		default:
			amap = simple_cmap;
			colorMapSz = sizeof(simple_cmap)/3; 
			
			if (len<1)
				break;
			
			t=0;
			
			map2d[0][0] = amap[t*3+0];
			map2d[0][1] = amap[t*3+1];
			map2d[0][2] = amap[t*3+2];
			
			for (i=1;i<len;i++)
			{
				t=(i-1)%(colorMapSz-1)+1; //070717
				
				map2d[i][0] = amap[t*3+0];
				map2d[i][1] = amap[t*3+1];
				map2d[i][2] = amap[t*3+2];
				
			}
			break;
	}
}


