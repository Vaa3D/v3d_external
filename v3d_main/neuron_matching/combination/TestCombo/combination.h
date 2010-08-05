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




//=======================================================
// combination.h
// Description : Template class to find combinations
//=======================================================
// Copyright 2003 - 2005 Wong Shao Voon
// No warranty, implied or expressed, is included.
// Author is not liable for any type of loss through 
// the use of this source code. Use it at your own risk!
//=======================================================


#ifndef __COMBINATION_H__
#define __COMBINATION_H__


namespace stdcomb
{

// Non recursive template function
template <class BidIt>

inline bool next_combination(BidIt n_begin, BidIt n_end,
BidIt r_begin, BidIt r_end)
{
  
  bool boolmarked=false;
  BidIt r_marked;
  
  BidIt n_it1=n_end;
  --n_it1;
  
  
  BidIt tmp_r_end=r_end;
  --tmp_r_end;
  
  for(BidIt r_it1=tmp_r_end; r_it1!=r_begin || r_it1==r_begin; --r_it1,--n_it1)
  {
    if(*r_it1==*n_it1 )
    {
      if(r_it1!=r_begin) //to ensure not at the start of r sequence
      {
        boolmarked=true;
        r_marked=(--r_it1);
        ++r_it1;//add it back again 
        continue;
      }
      else // it means it is at the start the sequence, so return false
        return false;      
    }
    else //if(*r_it1!=*n_it1 )
    {
      //marked code
      if(boolmarked==true)
      {
        //for loop to find which marked is in the first sequence
        BidIt n_marked;//mark in first sequence
        for (BidIt n_it2=n_begin;n_it2!=n_end;++n_it2)
          if(*r_marked==*n_it2) {n_marked=n_it2;break;}
      
    
        BidIt n_it3=++n_marked;    
        for  (BidIt r_it2=r_marked;r_it2!=r_end;++r_it2,++n_it3)
        {
          *r_it2=*n_it3;
        }
        return true;
      }
      for(BidIt n_it4=n_begin; n_it4!=n_end; ++n_it4)
        if(*r_it1==*n_it4)
        {
          *r_it1=*(++n_it4);
          return true;           
        }
    }
  }  

  return true;//will never reach here    
}

// Non recursive template function with Pred
template <class BidIt, class Prediate>

inline bool next_combination(
	BidIt n_begin, 
	BidIt n_end,
	BidIt r_begin, 
	BidIt r_end,
	Prediate Equal)
{
  
  bool boolmarked=false;
  BidIt r_marked;
  
  BidIt n_it1=n_end;
  --n_it1;
  
  
  BidIt tmp_r_end=r_end;
  --tmp_r_end;
  
  for(BidIt r_it1=tmp_r_end; r_it1!=r_begin || r_it1==r_begin; --r_it1,--n_it1)
  {
    if( Equal( *r_it1, *n_it1) )
    {
      if(r_it1!=r_begin) //to ensure not at the start of r sequence
      {
        boolmarked=true;
        r_marked=(--r_it1);
        ++r_it1;//add it back again 
        continue;
      }
      else // it means it is at the start the sequence, so return false
        return false;      
    }
    else //if(*r_it1!=*n_it1 )
    {
      //marked code
      if(boolmarked==true)
      {
        //for loop to find which marked is in the first sequence
        BidIt n_marked;//mark in first sequence
        for (BidIt n_it2=n_begin;n_it2!=n_end;++n_it2)
          if( Equal( *r_marked, *n_it2) ) {n_marked=n_it2;break;}
      
    
        BidIt n_it3=++n_marked;    
        for  (BidIt r_it2=r_marked;r_it2!=r_end;++r_it2,++n_it3)
        {
          *r_it2=*n_it3;
        }
        return true;
      }
      for(BidIt n_it4=n_begin; n_it4!=n_end; ++n_it4)
        if( Equal(*r_it1, *n_it4) )
        {
          *r_it1=*(++n_it4);
          return true;           
        }
    }
  }  

  return true;//will never reach here    
}


// Non recursive template function
template <class BidIt>

inline bool prev_combination(BidIt n_begin, BidIt n_end,
BidIt r_begin, BidIt r_end)
{
  
  bool boolsame=false;
  BidIt marked;//for r
  BidIt r_marked;
  BidIt n_marked;
  

  BidIt tmp_n_end=n_end;
  --tmp_n_end;
  
  BidIt r_it1=r_end;
  --r_it1;
  
  for(BidIt n_it1=tmp_n_end; n_it1!=n_begin || n_it1==n_begin ; --n_it1)
  {
    if(*r_it1==*n_it1)
    {
      r_marked=r_it1;
      n_marked=n_it1;
      break;
    }
  }  
  
  BidIt n_it2=n_marked;


  BidIt tmp_r_end=r_end;
  --tmp_r_end;
  
  for(BidIt r_it2=r_marked; r_it2!=r_begin || r_it2==r_begin; --r_it2,--n_it2)
  {
    if(*r_it2==*n_it2 )
    {
      if(r_it2==r_begin&& !(*r_it2==*n_begin) )
      {
        for(BidIt n_it3=n_begin;n_it3!=n_end;++n_it3)
        {
          if(*r_it2==*n_it3)
          {
            marked=r_it2;
            *r_it2=*(--n_it3);
            
            BidIt n_it4=n_end;
            --n_it4;
            for(BidIt r_it3=tmp_r_end; (r_it3!=r_begin || r_it3==r_begin) &&r_it3!=marked; --r_it3,--n_it4)
            {
              *r_it3=*n_it4;              
            }
            return true;
          }
        }
      }
      else if(r_it2==r_begin&&*r_it2==*n_begin)
      {
        return false;//no more previous combination; 
      }
    }
    else //if(*r_it2!=*n_it2 )
    {
      ++r_it2;
      marked=r_it2;
      for(BidIt n_it5=n_begin;n_it5!=n_end;++n_it5)
      {
        if(*r_it2==*n_it5)
        {
          *r_it2=*(--n_it5);

          BidIt n_it6=n_end;
          --n_it6;
          for(BidIt r_it4=tmp_r_end; (r_it4!=r_begin || r_it4==r_begin) &&r_it4!=marked; --r_it4,--n_it6)
          {
            *r_it4=*n_it6;              
          }
          return true;
        }
      }
    }
  }  
  return false;//Will never reach here, unless error    
}


// Non recursive template function with Pred
template <class BidIt, class Prediate>

inline bool prev_combination(
	BidIt n_begin, 
	BidIt n_end,
	BidIt r_begin, 
	BidIt r_end,
	Prediate Equal)
{
  
  bool boolsame=false;
  BidIt marked;//for r
  BidIt r_marked;
  BidIt n_marked;
  

  BidIt tmp_n_end=n_end;
  --tmp_n_end;
  
  BidIt r_it1=r_end;
  --r_it1;
  
  for(BidIt n_it1=tmp_n_end; n_it1!=n_begin || n_it1==n_begin ; --n_it1)
  {
    if( Equal(*r_it1, *n_it1) )
    {
      r_marked=r_it1;
      n_marked=n_it1;
      break;
    }
  }  
  
  BidIt n_it2=n_marked;


  BidIt tmp_r_end=r_end;
  --tmp_r_end;
  
  for(BidIt r_it2=r_marked; r_it2!=r_begin || r_it2==r_begin; --r_it2,--n_it2)
  {
    if( Equal(*r_it2, *n_it2) )
    {
      if(r_it2==r_begin&& !Equal(*r_it2, *n_begin) )
      {
        for(BidIt n_it3=n_begin;n_it3!=n_end;++n_it3)
        {
          if(Equal(*r_it2, *n_it3))
          {
            marked=r_it2;
            *r_it2=*(--n_it3);
            
            BidIt n_it4=n_end;
            --n_it4;
            for(BidIt r_it3=tmp_r_end; (r_it3!=r_begin || r_it3==r_begin) &&r_it3!=marked; --r_it3,--n_it4)
            {
              *r_it3=*n_it4;              
            }
            return true;
          }
        }
      }
      else if(r_it2==r_begin&&Equal(*r_it2, *n_begin))
      {
        return false;//no more previous combination; 
      }
    }
    else //if(*r_it2!=*n_it2 )
    {
      ++r_it2;
      marked=r_it2;
      for(BidIt n_it5=n_begin;n_it5!=n_end;++n_it5)
      {
        if(Equal(*r_it2, *n_it5))
        {
          *r_it2=*(--n_it5);

          BidIt n_it6=n_end;
          --n_it6;
          for(BidIt r_it4=tmp_r_end; (r_it4!=r_begin || r_it4==r_begin) &&r_it4!=marked; --r_it4,--n_it6)
          {
            *r_it4=*n_it6;              
          }
          return true;
        }
      }
    }
  }  
  return false;//Will never reach here, unless error    
}


// Recursive template function
template <class RanIt, class Func>

void recursive_combination(RanIt nbegin, RanIt nend, int n_column,
		      RanIt rbegin, RanIt rend, int r_column,int loop, Func func)
{
	
	int r_size=rend-rbegin;
	
	
	int localloop=loop;
	int local_n_column=n_column;
	
	//A different combination is out
	if(r_column>(r_size-1))
	{
    func(rbegin,rend);
    return;
	}
	/////////////////////////////////
	
	for(int i=0;i<=loop;++i)
	{
				
		RanIt it1=rbegin;
		for(int cnt=0;cnt<r_column;++cnt)
		{
		  ++it1;
		} 
		
		RanIt it2=nbegin;
		for(int cnt2=0;cnt2<n_column+i;++cnt2)
		{
		  ++it2;
		} 
		
		*it1=*it2;
		
		++local_n_column;
		
		recursive_combination(nbegin,nend,local_n_column,
		        rbegin,rend,r_column+1,localloop,func);
		--localloop;
	}
	
}

}

#endif
