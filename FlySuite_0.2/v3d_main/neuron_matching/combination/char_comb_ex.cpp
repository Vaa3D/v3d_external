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




//char_comb_ex.cpp
//This program finds combinations using the recursive method

/*****Instructions*******
To see how it works for other combinations
Try setting N_NUM to other numbers (must not be greater than 10)
Try setting R_NUM to other numbers (must not be greater than N_NUM)
*************************/

#include "stdafx.h"
#include<iostream>

using namespace std;

//function prototype
void char_combination(char n[],int n_column,
            char r[], int r_size, int r_column, int loop);

int main()
{
	const int N_NUM=6; 
	const int R_NUM=4;
	
	char n[]="1234567890";

	char r[R_NUM+1];//+1 for the NULL character
	r[R_NUM]='\0';

	char_combination(n,0,r,R_NUM,0,N_NUM-R_NUM);
	
	cout<<"Complete!"<<endl;
	return 0;
}

void char_combination(char n[],int n_column,
            char r[], int r_size, int r_column, int loop)
{
	int localloop=loop;
	int local_n_column=n_column;
	
	///////Display the string code/////////
	if(r_column>(r_size-1))
	{
		cout<<r<<endl;
		return;
	}
	/////End of displaying string code//////
	
	for(int i=0;i<=loop;++i)
	{
		r[r_column]=n[n_column+i];
		++local_n_column;
		char_combination(n,local_n_column,r,r_size,r_column+1,localloop);
		--localloop;
	}
}
