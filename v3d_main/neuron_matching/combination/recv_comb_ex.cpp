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




//This recv_comb_ex.cpp tests the recursive combination function out
#include<iostream>
#include<vector>
#include<string>
#include"combination.h"

using namespace std;
using namespace stdcomb;


//test recursive_combination() with pointers

void display(char* begin,char* end)
{
  cout<<begin<<endl;
}
int main()
{
  char ca[]="123456";
  char cb[]="1234";
   
  recursive_combination(ca,ca+6,0,
                  cb,cb+4,0,6-4,display);
  cout<<"Complete!"<<endl;
	return 0;
}


/*
//test recursive_combination() with iterator
typedef vector<int>::iterator vii;

void display(vii begin,vii end)
{
  for (vii it=begin;it!=end;++it)
      cout<<*it;
  cout<<endl;
}

int main()
{
  vector<int> ca;
  ca.push_back (1);
  ca.push_back (2);
  ca.push_back (3);
  ca.push_back (4);
  ca.push_back (5);
  ca.push_back (6);
  vector<int> cb;
  cb.push_back (1);
  cb.push_back (2);
  cb.push_back (3);
  cb.push_back (4);
   
  recursive_combination(ca.begin (),ca.end(),0,
                  cb.begin(),cb.end(),0,6-4,display);
  cout<<"Complete!"<<endl;
	return 0;
}
*/

/*
//test recursive_combination() with array of strings

void display(string* begin,string* end)
{
  for (string* it=begin;it!=end;++it)
      cout<<*it<<" ";
  cout<<endl;
}


int main()
{
  string* strarray=new string[3];
  strarray[0]="Red";
  strarray[1]="Green";
  strarray[2]="Blue";
  string* strarray1=new string[2];
  strarray1[0]="Red";
  strarray1[1]="Green";
  
  recursive_combination(strarray,strarray+3,0,
                  strarray1,strarray1+2,0,3-2,display);
  cout<<"Complete!"<<endl;
  delete [] strarray;
  delete [] strarray1;

	return 0;
  
}
*/

