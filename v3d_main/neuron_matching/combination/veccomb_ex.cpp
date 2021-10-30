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




// TestComb.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include "veccomb.h"

int _tmain(int argc, _TCHAR* argv[])
{
    using namespace std;
	typedef vector<char> Set;

     vector<Set> VecOrig;

     Set v1, v2, v3;
     v1.push_back('A');
     v1.push_back('B');
     v1.push_back('C');
     v1.push_back('D');
     v2.push_back('A');
     v2.push_back('B');
     v2.push_back('C');
     v3.push_back('A');
     v3.push_back('B');
     v3.push_back('C');
     VecOrig.push_back(v1);
     VecOrig.push_back(v2);
     VecOrig.push_back(v3);

     vector<Set> VecResults;

     VecResults.push_back( Set() );
     VecResults.push_back( Set() );
     VecResults.push_back( Set() );

     VecResults.at(0).push_back( v1.at(0) );
     VecResults.at(0).push_back( v1.at(1) );
     VecResults.at(0).push_back( v1.at(2) );
     VecResults.at(1).push_back( v2.at(0) );
     VecResults.at(1).push_back( v2.at(1) );
     VecResults.at(2).push_back( v2.at(0) );
     VecResults.at(2).push_back( v2.at(1) );

	 while(FindVecComb(
		VecOrig,
		VecResults ) )
	 {
		std::string str="";
		for( size_t i=0; i<VecResults.size(); ++i )
		{
			for( size_t j=0; j<VecResults.at(i).size(); ++j )
			{
				str+=VecResults.at(i).at(j);
			}
			str+=" | ";
		}
		cout<<str.c_str()<<endl;
	 }

	 return 0;
}