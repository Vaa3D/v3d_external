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




// define queue class and seuqence class, queue is derived from sequence class
// F. Long
//20080526

#ifndef __FL_QUEUE__
#define __FL_QUEUE__



#include <string.h>

/////////////////////////////////////
// define class Sequence
/////////////////////////////////////

template< class T > class Sequence
{
 private:

    T *fArray;
    V3DLONG fSequence_length; 
    V3DLONG fArray_length;    
    V3DLONG fHead;            // item # of start of sequence

    void expand();
    inline T *getItemAddress(V3DLONG i);
    inline T get(V3DLONG i);

 public:

    Sequence()
    {
        fArray = NULL;
        fSequence_length = 0;
        fArray_length = 0;
        fHead = 0;
    }

    ~Sequence()
    {
        if (fArray) {delete [] fArray; fArray = 0;} 
    }

    V3DLONG getSequenceLength() 
    { 
        return fSequence_length; 
    }

    void initialize(V3DLONG h);	
    inline void setItem(V3DLONG i, T x);
    inline T getItem(V3DLONG i); 
	inline T removeFirstItem();
    inline T removeLastItem();
    inline void insertAtEnd(T x);
    void insertAtBegin(T x); 
	void freeSequence();
    void copyToBuf(T *x);
};


// getItemAddress() returns the memory address of the sequence item whose array index is i

template< class T> inline T *Sequence<T>::getItemAddress(V3DLONG i)
{
	if (fArray==0)
	{
		fprintf(stderr, "fArray is empty in Sequence::getItemAddress() \n.");
		return (T *) NULL;
	}
	if (i < 0)
	{
		fprintf(stderr, "i is invalid in Sequence::getItemAddress() \n");
		return (T *) NULL;
	}
    return (T *)(fArray + ((fHead + i) % fArray_length));
}


// Expand() grows the sequence storage array by a factor of 2

template< class T> void Sequence<T>::expand()
{	
	if (fArray == NULL)
	{
		fprintf(stderr, "fArray is NULL in Sequence::getItemAddress()\n");
		return;
	}
	
    V3DLONG old_length;
    T *newArray;

    old_length = fArray_length;
    V3DLONG new_length = 2 * fArray_length;
    newArray = new T [new_length];
    	
    if (!newArray)
    {
        fprintf(stderr, "Out of memory in Sequence::expand()\n");
		return;
    }

	V3DLONG i;
	
	for (i=0; i<old_length;i++)
	{
		newArray[i] = fArray[i];
	}
    delete []fArray; fArray = 0;
	
    fArray = (T *)newArray;
    fArray_length = new_length;
    if (fHead > 0)
    {
        // Move items that were at the tail end of the array to the
        // tail end of the reallocated array.
        void *old = getItemAddress(0);
        memcpy((T *)old + old_length, old, (old_length - fHead)*sizeof(T));
        fHead += old_length;
    }
}

// Get() gets item stored in address i
template<class T> inline T Sequence<T>::get(V3DLONG i)
{
	if (fArray == NULL)
	{	fprintf(stderr, "fArray is NULL Sequence::getItem(). \n");
		return (T) 0;
	};
	
    return(*getItemAddress(i));
}

// initialize() initializes sequence. Argument 'hint' is in the initally allocated
// length in number of items (not bytes) of the sequence array.

template< class T> void Sequence<T>::initialize(V3DLONG hint)
{
     if (hint < 0)
	 {
		fprintf(stderr, "HINT is invalid in Sequence::initialize().\n");
		return;
	 }
	 
	 if (hint == 0)
     {
         hint = 16;
     }
	 
     fArray = new T [hint];

     if (!fArray)
     {
         fprintf(stderr, "Out of memory in Squence::initialize().\n");
		 return;
     }
     fArray_length = hint;
 }

// getItem() gets the i-th sequence item.

template<class T> inline T Sequence<T>::getItem(V3DLONG i)
{

    if (fArray == NULL)
	{
		fprintf(stderr, "fArray is NULL in Sequence::getItem().\n");
		return;
     }
	
    if ((i<0)||(i > fSequence_length))
	{
		fprintf(stderr, "i is invalide in Sequence::getItem().\n");
		return;
	}

    return(get(i));
}

// setIterm() sets the i-th sequence item

template<class T> inline void Sequence<T>::setItem(V3DLONG i, T x)
{
	if (fArray == NULL)
	{
		fprintf(stderr, "fArray is NULL in Sequence::setItem().\n");
		return;
	}
	if ((i < 0) && (i > fSequence_length))
	{
		fprintf(stderr, "i is invalid in Sequence::setItem().\n");
		return;
	}
	
    *getItemAddress(i) = x;
}

// insertAtEnd() adds item to end of the sequence

template<class T> inline void Sequence<T>::insertAtEnd(T x)
{
    if (fSequence_length == fArray_length)
    {
        expand();
    }
    V3DLONG len = fSequence_length++;
    setItem(len, x);
}

// insertAtBegin() adds item to the beginning of the sequence.  
// Item i becomes item i+1 after this operation.

template<class T> void Sequence<T>::insertAtBegin(T x)
{
    if (fSequence_length == fArray_length)
    {
        expand();
    }
    if (--fHead < 0)
    {
        fHead = fArray_length - 1;
    }
    fSequence_length++;
    setItem(0,x);        //start of sequence
}

// removeFirstItem() removes the first item in the sequence.  
// Item i becomes item i-1 after this operation. 
// The function returns the value that is removed from the
// sequence 

template<class T> inline T Sequence<T>::removeFirstItem()
{

	if (fArray == NULL)
	{	
		fprintf(stderr, "fArray is NULL in Sequence::removeFirstItem()\n");
		return (T) 0;
	}
	if (fSequence_length <= 0)
	{	
		fprintf(stderr, "fSequence_length is invalid in Sequence:removeFirstItem().\n");
		return (T) 0;
	}
    
    T x = get(0);      //start of sequence
    
    fHead = (fHead + 1) % fArray_length;
    --fSequence_length;

    return(x);
}

// removeLastItem() removes the last item in the sequence.  
// The function returns the value that is removed from the sequence

template<class T> inline T Sequence<T>::removeLastItem()
{
    if (fArray == NULL)
	{	
		fprintf(stderr, "fArray is NULL in Sequence::removeLastItem().\n");
	    return;
	}
	if (fSequence_length <= 0)
	{
		fprintf(stderr, "fSequence_length is invalid in Sequence:removeLastItem().\n");
		return;
	}
     
    V3DLONG len = --fSequence_length;
    return(get(len));
}

// freeSequence frees the sequence.  

template<class T> void Sequence<T>::freeSequence()
{
    if (fArray) {delete []fArray; fArray =0;}
    fSequence_length = 0;
    fArray_length = 0;
    fHead = 0;
}

// copyToBuf copies sequence's contents to a buffer.

template<class T> void Sequence<T>::copyToBuf(T *a)
{
    if (fHead == 0)
    {
        memcpy(a, fArray, fSequence_length*sizeof(T));
    }
    else
    {
        for (V3DLONG k = 0; k < fSequence_length; k++)
        {
            a[k] = getItem(k);
        }
    }
}


//////////////////////////////////////////////////
// defines class Queue (derived from Sequence class)
//////////////////////////////////////////////////

template<class T> class Queue : public Sequence<T>
{
 public:
    inline void put(T x);
    inline T get(void);
};


// put() puts an element into queue
template<class T> inline void Queue<T>::put(T x)
{
    this->insertAtEnd(x);
}

// get() gets an element from queue
template<class T> inline T Queue<T>::get()
{
    return(this->removeFirstItem());
}


#endif // __FL_QUEUE__ 
