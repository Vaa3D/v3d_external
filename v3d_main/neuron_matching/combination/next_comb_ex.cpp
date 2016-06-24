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


// for use with next_combination examples!
template<class BidIt>
void display(BidIt begin,BidIt end)
{
  for (BidIt it=begin;it!=end;++it)
      cout<<*it<<" ";
  cout<<endl;
}


using namespace std;
using namespace stdcomb;



//test next_combination() with pointers

int main()
{
  char ca[]="123456";
  char cb[]="1234";
   
  do
  {
    cout<<cb<<endl;
  }
  while(next_combination(ca,ca+6,cb,cb+4));
  cout<<"Complete!"<<endl;
  
  return 0;
}


/*
//test next_combination() with iterators
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
   
  do
  {
    display(cb.begin(),cb.end());
  }
  while(next_combination(ca.begin (),ca.end (),cb.begin (),cb.end()) );
  
  cout<<"Complete!"<<endl;
  
  return 0;
}
*/

/*
//test next_combination() with strings
int main()
{
  vector<string> str;
  str.push_back ("Red");
  str.push_back ("Green");
  str.push_back ("Blue");
  vector<string> str1;
  str1.push_back ("Red");
  str1.push_back ("Green");
   
  do
  {
    display(str1.begin(),str1.end());
  }
  while(next_combination(str.begin (),str.end (),str1.begin (),str1.end()));

  cout<<"Complete!"<<endl;
  
	return 0;
  
}

*/