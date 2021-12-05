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




/* graphsupport.cpp -- a few supporting function for the graph algorithms
   ver: 0.1
   by Hanchuan Peng, June, 2002
  */


#ifndef _GRAPH_SUPPORT_FUNCTIONS_
#define _GRAPH_SUPPORT_FUNCTIONS_

#include "graph.h"

//global variables

template <class T> int new2dArrayMatlabProtocal(T ** & img2d,T * & img1d,V3DLONG imghei,V3DLONG imgwid);
template <class T> void delete2dArrayMatlabProtocal(T ** & img2d,T * & img1d);

template <class T> int new1dArrayMatlabProtocal(T * & img1d,V3DLONG nodenum);
template <class T> void delete1dArrayMatlabProtocal(T * & img1d);

//generating an _ELEMENT_GRAPH_UBYTE image for any input image type
template <class T> void copyvecdata_T2UB(T * srcdata, 
					 V3DLONG len, 
					 _ELEMENT_GRAPH_UBYTE * desdata, 
					 int& nstate, 
					 _ELEMENT_GRAPH_UBYTE &minn, 
					 _ELEMENT_GRAPH_UBYTE &maxx);
template <class T> void copyvecdata_T2D(T * srcdata, 
					V3DLONG len, 
					double * desdata, 
					double &nstate, 
					double &minn, 
					double &maxx);

//==================definition=============================

template <class T> void copyvecdata_T2UB(T * srcdata, 
					 V3DLONG len, 
					 _ELEMENT_GRAPH_UBYTE * desdata, 
					 int& nstate, 
					 _ELEMENT_GRAPH_UBYTE &minn, 
					 _ELEMENT_GRAPH_UBYTE &maxx)
{
  if(!srcdata || !desdata)
  {
    printf("NULL points in copyvecdata()!\n");
    return;
  } 

  V3DLONG i;

  //note: originally I added 0.5 before rounding, however seems the negative numbers and 
  //      positive numbers are all rounded towarded 0; hence int(-1+0.5)=0 and int(1+0.5)=1;
  //      This is unwanted because I need the above to be -1 and 1.
  // for this reason I just round with 0.5 adjustment for positive and negative differently

  //copy data
  if (srcdata[0]>0)
    maxx = minn = int(srcdata[0]+0.5);
  else
    maxx = minn = int(srcdata[0]-0.5);

  int tmp;
  double tmp1;
  for (i=0;i<len;i++)
  {
    tmp1 = double(srcdata[i]);
    tmp = (tmp1>0)?(int)(tmp1+0.5):(int)(tmp1-0.5);//round to integers
    minn = (minn<tmp)?minn:tmp;
    maxx = (maxx>tmp)?maxx:tmp;
    desdata[i] = (_ELEMENT_GRAPH_UBYTE)tmp;
    //    printf("%i ",desdata[i]);
  }
  maxx = (_ELEMENT_GRAPH_UBYTE)maxx;
  minn = (_ELEMENT_GRAPH_UBYTE)minn;
  //printf("\n");

  /*
  //make the vector data begin from 0 (i.e. 1st state)
  for (i=0;i<len;i++)
  {
    desdata[i] -= minn;
  }
  */

  //return the #state
  nstate = (maxx-minn+1);

  return;
}

template <class T> void copyvecdata_T2D(T * srcdata, 
					V3DLONG len, 
					double * desdata, 
					double &nstate, 
					double &minn, 
					double &maxx)
{
  if(!srcdata || !desdata)
  {
    printf("NULL points in copyvecdata_D2T()!\n");
    return;
  } 

  V3DLONG i;

  //note: originally I added 0.5 before rounding, however seems the negative numbers and 
  //      positive numbers are all rounded towarded 0; hence int(-1+0.5)=0 and int(1+0.5)=1;
  //      This is unwanted because I need the above to be -1 and 1.
  // for this reason I just round with 0.5 adjustment for positive and negative differently

  //copy data
  maxx = minn = double(srcdata[0]);

  double tmp;
  for (i=0;i<len;i++)
  {
    tmp = double(srcdata[i]);
    minn = (minn<tmp)?minn:tmp;
    maxx = (maxx>tmp)?maxx:tmp;
    desdata[i] = tmp;
  }
  /*
  //make the vector data begin from 0 (i.e. 1st state)
  for (i=0;i<len;i++)
  {
    desdata[i] -= minn;
  }
  */

  //return the #state
  nstate = (maxx-minn);

  return;
}


//memory management

template <class T> int new2dArrayMatlabProtocal(T ** & img2d,T * & img1d,V3DLONG imghei,V3DLONG imgwid)
{
  V3DLONG totalpxlnum = (V3DLONG)imghei*imgwid;
  img1d = new T [totalpxlnum];
  img2d = new T * [(V3DLONG)imgwid];
  
  if (!img1d || !img2d)
  {
    if (img1d) {delete img1d;img1d=0;}
    if (img2d) {delete img2d;img2d=0;}
    printf("Fail to allocate mem in newIntImage2dPairMatlabProtocal()!");
    return 0; //fail
  }

  V3DLONG i;

  for (i=0;i<imgwid;i++) 
    {img2d[i] = img1d + i*imghei;}

  for (i=0;i<totalpxlnum;i++) 
    {img1d[i] = (T)0;}

  return 1; //succeed
}
template <class T> void delete2dArrayMatlabProtocal(T ** & img2d,T * & img1d)
{
  if (img1d) {delete img1d;img1d=0;}
  if (img2d) {delete img2d;img2d=0;}
}

template <class T> int new1dArrayMatlabProtocal(T * & img1d, V3DLONG nnode)
{
  img1d = new T [nnode];
  if (!img1d) {
    printf("Fail to allocate mem in newIntImage2dPairMatlabProtocal()!");
    return 0; //fail
  }
  V3DLONG i;
  for (i=0;i<nnode;i++) {img1d[i] = (T)0;}
  return 1; //succeed
}
template <class T> void delete1dArrayMatlabProtocal(T * & img1d)
{
  if (img1d) {delete img1d;img1d=0;}
}

template <class T> void printimg(T * img, V3DLONG nnode)
{
  printf("\n--<beg\n");
  for(V3DLONG inode=0;inode<nnode;inode++)
  {
    printf("%i ",int(img[inode]));
  }
  printf("\nend>--\n");
}

#endif //ifndef _GRAPH_SUPPORT_FUNCTIONS_
