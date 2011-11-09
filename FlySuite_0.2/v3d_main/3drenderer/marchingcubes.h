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




/*
 * marchingcubes.h
 *
 *  Created on: Sep 4, 2008
 *      Author: ruanzongcai
 *
 * Based on 1994 Paul Bourke's Polygonising a scalar field code
 * Also known as: "3D Contouring", "Marching Cubes", "Surface Reconstruction"
 *
 */

#ifndef _marchingcubes_h_
#define _marchingcubes_h_


//-----------------------------------------
// triangle struct
struct Triangle
{
	  struct Triangle* next;
      float vertex[3][3];
	  float normal[3][3];
};
struct TriangleIndex  // OBJ face
{
	  struct TriangleIndex* next;
      int vi[3];
	  int ti[3];
	  int ni[3];
};

//-----------------------------------------
// delete triangle list
void delTriangles(struct Triangle* list);
// count num of triangle
int numTriangles(struct Triangle* list);

// render triangle list
#ifdef __gl_h_  // gl.h
#define RENDER_TRIANGLES(list) \
{\
   		glBegin(GL_TRIANGLES);\
		for (struct Triangle* p = list; p!=NULL; p = p->next)\
		{\
			for (int iCorner = 0; iCorner < 3; iCorner++)\
			{\
				glNormal3f(p->normal[iCorner][0],p->normal[iCorner][1],p->normal[iCorner][2]);\
				glVertex3f(p->vertex[iCorner][0],p->vertex[iCorner][1],p->vertex[iCorner][2]);\
			}\
		}\
		glEnd();\
}
#endif


//-----------------------------------------

// sampling function pointer type
typedef float (*t_fSample)(float fX, float fY, float fZ); //0<=fX,fY,fZ<=1
#define CLAMP01(a)  (((a)<0)? 0: ((a)>1)? 1: (a))

// the marching function
struct Triangle* MarchingCubes(        //return triangle list
				   int   numStep,      //num of sampling steps
				   float isoValue,     //iso value
				   t_fSample fSample,  //sampling function
				   int method=0);      //0/1 -- Marching Cubes/Tetrahedrons


#endif// _marchingcubes_h_
