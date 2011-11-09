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




//FL_main_brainseg.h
//by Fuhui Long
//20081024

#ifndef __FL_MAIN_BRAINSEG_H__
#define __FL_MAIN_BRAINSEG_H__

//typedef unsigned short int CellSegLabelType;

class levelset_segParameter // for watershed segmentation
{
public:
	char *infilename, *segfilename, *outfilename; // name of input and output files
	int channelNo; // channel to be segmented
	int dimension;
	int regionnum; // number of regions to be segmented
	int *regions;
	int maxregionnum; //maximum number of regions in the template
	int *regionNoModel; // regions that should not apply model, such as optical lobes
	int regionNoModelNum; // number of regions that should not apply model
	float lamda; //coefficient of the weighted length term Lg(\phi)
	float alf; //coefficient of the weighted area term Ag(\phi);
	float epsilon; // the papramater in the definition of smoothed Dirac function
	float delt; // time step
	float mu; //coefficient of the internal (penalizing) energy term P(\phi)	
	float gama ;	
//	int method;

	levelset_segParameter() //initialize parameters using default values
	{
		infilename = NULL;
		outfilename = NULL;
		segfilename = NULL;
		channelNo = 2;
		dimension = 2;
		regionnum = 62;
		maxregionnum = 62;
	
		regions = new int [regionnum];
		for (int i=0; i<regionnum; i++)
			regions[i] = i+1;
		
		regionNoModel = new int [2]; // should revise
		regionNoModel[0] = 1;
		regionNoModel[1] = 34;
		
		regionNoModelNum = 2;
		
		lamda = 0.5; //coefficient of the weighted length term Lg(\phi)
		alf =1; //coefficient of the weighted area term Ag(\phi);
		epsilon = 1.5; // the papramater in the definition of smoothed Dirac function
		delt = 5; // time step
		mu = 0.1/delt; //coefficient of the internal (penalizing) energy term P(\phi)	
		gama = 0.001;	
//		method = 1;

	}
	
	levelset_segParameter(const levelset_segParameter & segPara) //initialize parameters using input values
	{

		infilename = segPara.infilename;
		segfilename = segPara.segfilename;
		outfilename = segPara.outfilename;
		channelNo = segPara.channelNo;
		dimension = segPara.dimension;
		regionnum = segPara.regionnum;
		maxregionnum = 62;
		
		regions = new int [regionnum];				
		for (int i=0; i<regionnum; i++)
			regions[i] = segPara.regions[i];
			
		regionNoModel = new int [2]; // should revise
		regionNoModel[0] = 1;
		regionNoModel[1] = 34;			
			
		regionNoModelNum = 2;
		
		lamda = segPara.lamda; //coefficient of the weighted length term Lg(\phi)
		alf = segPara.alf; //coefficient of the weighted area term Ag(\phi);
		epsilon = segPara.epsilon; // the papramater in the definition of smoothed Dirac function
		delt = segPara.delt; // time step
		mu = 0.1/segPara.delt; //coefficient of the internal (penalizing) energy term P(\phi)	
		gama = segPara.gama;	
//		method = segPara.method;
		
	}
	
};

bool brainSeg2D(Vol3DSimple <unsigned char> *rawimg3d, Vol3DSimple <unsigned short int> *segimg3d, Vol3DSimple <unsigned short int> *outimg3d, const levelset_segParameter & mypara);

#endif

