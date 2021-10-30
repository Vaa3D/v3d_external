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




// 2009-5-29 Zongcai Ruan
// a simple test program for stack size setting on MAC/UNIX/WIN32

#if !defined(__WIN32__)
#include <sys/resource.h> // getrlimit() setrlimit()
#else
#include <malloc.h> // stackavail()
#endif

#include <iostream>
using namespace std;

void check_stack_limit()
{
	cerr << "Stack limit is: ";
#if !defined(__WIN32__)
	rlimit lim;
	getrlimit(RLIMIT_STACK,&lim);
	cerr << lim.rlim_cur <<" , "<< lim.rlim_max << endl;
#else
	size_t avail=stackavail();
	cerr << avail << endl;
#endif
}

V3DLONG top_of_stack, last_of_stack;
void recur_call(int *count)
{
	if (*count > 0)
	{
		(*count)--; // call recursive function until counter is equal to 0
		recur_call(count);
	}
	else if (*count < 0)
	{
		int x = *count;
		last_of_stack = (V3DLONG)&x;
		printf("last of stack address: %#lx\n", last_of_stack); // print the last local variable address in the stack reached
		printf("size of reached stack: %ld\n", (top_of_stack-last_of_stack));

		recur_call(count);
	}
	else // *count==0
	{
		int x = *count;
		last_of_stack = (V3DLONG)&x;
		printf("last of stack address: %#lx\n", last_of_stack); // print the last local variable address in the stack reached
		printf("size of reached stack: %ld\n", (top_of_stack-last_of_stack));
	}
}

int main(int ac, char **av)
{
	check_stack_limit();

	int x=0;
	top_of_stack = (V3DLONG)&x;
	printf("top of stack address: %#lx\n", top_of_stack); // print first local variable adress in the stack

	if (ac>1)
	{
		x = atoi(av[1]);
	}
	recur_call(&x); /* call 'x' times recursive 'recur_call' function, increasing the stack depth */
}


//MAC:
//ulimit -a
//g++ stacksize.cpp -Wl,-stack_size,0x10000
//size a.out -m

//UNIX
//ulimit -a
//ulimit -s 64000
//g++ stacksize.cpp -fstack-check
//size a.out -A

//mingw32
//g++ stacksize.cpp -Xlinker --stack=64000000
//

//Win32
//in the PE header (IMAGE_NT_HEADERS) of your exe there are some records such as:
//typedef struct _IMAGE_NT_HEADERS {
//    DWORD Signature;
//    IMAGE_FILE_HEADER FileHeader;
//    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
//} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;
//typedef struct _IMAGE_OPTIONAL_HEADER {
//    ...
//    DWORD   SizeOfStackReserve;
//    DWORD   SizeOfStackCommit;
//    ...
//}
//There is a simple way to obtain these values: using GetModuleHandle(NULL) will give you the imagebase (handle) of your module, address where you'll find a IMAGE_DOS_HEADER structure
//which will help you to find the IMAGE_NT_HEADERS structure (imagebase+IMAGE_DOS_HEADER.e_lfanew) -> IMAGE_NT_HEADERS, and there you'll find those fields: SizeOfStackReserve and SizeOfStackCommit.
//The maximum amount of space that the OS will allocate for your stack is SizeOfStackReserve.
//
