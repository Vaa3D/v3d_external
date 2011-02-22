/*
 #  y_imglib.h
 #
 #  created Feb. 12, 2010 by Yang Yu
 #  yuy@janelia.hhmi.org
 #
 */


//image and template class with funcs

//--------
#ifndef __Y_IMGLIB_H__
#define __Y_IMGLIB_H__
//--------

#include <complex>
#include <fftw3.h>

#include <cmath>
#include <ctime>
#include <cstring>

#include <fstream>
using std::ifstream;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <sstream>

#include <vector>
#include <list>
#include <bitset>

#include <set>

// POSIX Threads
//#include <pthread.h> 
//#define PROCESSORS 1 // maximum threads

#define PI 3.14159265
#define INF 1E10

#define NPEAKS 8 // numbers of peaks of phase correlation
#define NO_OBJECT 0 // for region growing

using namespace std;

// define different data types
typedef unsigned char UINT8;
//typedef double REAL; // -lfftw3 -lfftw3_threads // fftw_
typedef float REAL; // -lfftw3f -lfftw3f_threads // fftwf_

#define TC_COMMENT1 (" thumbnail file ")
#define TC_COMMENT2 (" tiles ")
#define TC_COMMENT3 (" dimensions (XYZC) ")
#define TC_COMMENT4 (" origin (XYZ) ")
#define TC_COMMENT5 (" resolution (XYZ) ")
#define TC_COMMENT6 (" image coordinates look up table ")

//statistics of count of labeling
class STCL
{
public:
	STCL(){}
	~STCL(){}
	
public:
	int count;
	int label;
};


// this is an extension to 3D region growing from octave 2D bwlabel code
// find func copied from octave bwlabel code
static int find( int set[], int x )
{
    int r = x;
    while ( set[r] != r )
        r = set[r];
    return r;
}

template <class T>
T y_max(T x, T y)
{
	return (x>y)?x:y;
}

template <class T>
T y_min(T x, T y)
{
	return (x<y)?x:y;
}

template <class T>
T y_abs(T x)
{
	return (x<0)?-x:x;
}

// Define a lookup table
template <class T>
class LUT
{
public:
	
	LUT(){}
	
	LUT(T *a, T *b, bool offset_region)
	{
		T len = 3; //start.size();
		
		if(offset_region)
		{
			init_by_offset(a,b,len);
		}
		else
		{
			init_by_region(a,b,len);
		}
	}
	
	~LUT(){}
	
public:
	void init_by_offset(T *offsets, T *dims, T len)
	{
		try
		{
			start_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			start_pos[i] = offsets[i];
		
		try
		{
			end_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			end_pos[i] = start_pos[i] + dims[i] - 1;
	}
	
	void init_by_region(T *start, T *end, T len)
	{
		try
		{
			start_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			start_pos[i] = start[i];
		
		try
		{
			end_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			end_pos[i] = end[i];
	}
	
	void clear()
	{
		if(start_pos) {delete []start_pos; start_pos=0;}
		if(end_pos) {delete []end_pos; end_pos=0;}
	}
	
	
public:
	
	T *start_pos;
	T *end_pos;
	
	string fn_img;
	
};

// Define a indexed data structure
template <class T1, class T2>
class indexed_t
{
public:
	indexed_t(T1 *in_offsets)
	{
		T1 len = 3; //in_offsets.size();
		
		try
		{
			offsets = new T1 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		try
		{
			sz_image = new T1 [len+1]; // X Y Z C
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		// init
		for(T1 i=0; i<len; i++)
		{
			offsets[i] = in_offsets[i];
			sz_image[i] = 1;
		}
		sz_image[3] = 1;

	}
	~indexed_t(){}
	
public:
	T1 *offsets; // ref
	T1 ref_n; // reference image number
	T1 n;
	
	T2 score;
	
	string fn_image; // absolute path + file name
	T1 *sz_image;
	
//	T cn; // child image number
//	bool cp; // if cp=0 then offsets are curr tile to its child; if cp=1 then offsets are curr tile to its parent
//	T *offsets_child; 
	
	T1 predecessor; // adjacent prior image number | root's predecessor is -1
	bool visited; // init by false
	
	std::vector<indexed_t> record;
	//std::vector<indexed_t> path;
	
	//indexed_t *parent; // adjacent prior level 1: front -> back (z); level 2: upper -> down (y); level 3: left -> right (x)
};

// define a point structure
template <class T1, class T2>
class _POINT 
{
public:
	_POINT(){}
	~_POINT(){}
	
public:
	T1 x,y,z;
	T2 intensity;
	string fn;
};

// define a point type
typedef _POINT<V3DLONG, UINT8> POINT;

// Virtual Image Class
template <class T1, class T2, class indexed_t, class LUT>
class Y_VIM 
{
	
public:
	
	//init
	// creating a hash table
	Y_VIM(list<string> imgList, T2 dims)
	{
		// finding best global alignment
		
		
	}	
	
	Y_VIM(){}
	
	// destructor
	~Y_VIM(){}
	
public:
	
	//load a virtual image
	bool y_load(string fn)
	{
		ifstream pFileLUT(fn.c_str());
		string str;
		
		char letter;
		
		T2 start[3], end[3];
		
		sz = new T2 [3];
		
		char buf[2048];
		string fn_str;
		
		if(pFileLUT.is_open())
		{
			//
			pFileLUT >> letter;
			
			if(letter=='#')
				getline(pFileLUT, str); // read comments line
			
			if(strcmp(str.c_str(), TC_COMMENT1))
			{
				QMessageBox::information(0, "TC file reading", QObject::tr("Your .tc file is illegal."));
				pFileLUT.close();
				return false;
			}
			else
			{
				// thumbnail file name
				pFileLUT >> fn_thumbnail;
			}
			
			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str); // read comments line
			
			if(strcmp(str.c_str(), TC_COMMENT2))
			{
				QMessageBox::information(0, "TC file reading", QObject::tr("Your .tc file is illegal."));
				pFileLUT.close();
				return false;
			}
			else
			{
				// tiles
				pFileLUT >> number_tiles;
			}
			
			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str); // read comments line
			
			if(strcmp(str.c_str(), TC_COMMENT3))
			{
				QMessageBox::information(0, "TC file reading", QObject::tr("Your .tc file is illegal."));
				pFileLUT.close();
				return false;
			}
			else
			{
				// dimensions
				pFileLUT >> sz[0] >> sz[1] >> sz[2] >> sz[3];
			}
			
			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str); // read comments line
			
			if(strcmp(str.c_str(), TC_COMMENT4))
			{
				QMessageBox::information(0, "TC file reading", QObject::tr("Your .tc file is illegal."));
				pFileLUT.close();
				return false;
			}
			else
			{
				// origins
				pFileLUT >> origin_x >> origin_y >> origin_z;
			}

			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str); // read comments line
			
			if(strcmp(str.c_str(), TC_COMMENT5))
			{
				QMessageBox::information(0, "TC file reading", QObject::tr("Your .tc file is illegal."));
				pFileLUT.close();
				return false;
			}
			else
			{
				// resolutions
				pFileLUT >> rez_x >> rez_y >> rez_z;
			}

			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str); // read comments line
			
			if(strcmp(str.c_str(), TC_COMMENT6))
			{
				QMessageBox::information(0, "TC file reading", QObject::tr("Your .tc file is illegal."));
				pFileLUT.close();
				return false;
			}
			else
			{
				// lut
				lut = new LUT [number_tiles];
				T2 count=0;
				
				while( !pFileLUT.eof() )
				{
					while( getline(pFileLUT, str) )
					{
						istringstream iss(str);
						
						iss >> buf;
						
						fn_str = buf;
						
						//
						iss >> buf; iss >> start[0];
						iss >> buf; iss >> start[1];
						iss >> buf; iss >> start[2];
						
						iss >> buf;
						
						iss >> buf; iss >> end[0];
						iss >> buf; iss >> end[1];
						iss >> buf; iss >> end[2];
						
						lut[count] = LUT(start, end, false);
						lut[count].fn_img = fn_str;
						
						count++;
						
						if(count>=number_tiles)
							break;
						
						//iss >> letter;
					}
					
				}
			}
			
		}
		else
		{
			cout << "Unable to open the file";
			pFileLUT.close();
			return false;
		}
		
		pFileLUT.close();
		
		
		// adjusting
		T2 len = 3;
		
		try
		{
			min_vim = new T2 [len];
			max_vim = new T2 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return false;
		}
		
		for(T2 i=0; i<len; i++)
		{
			min_vim[i] = 0; max_vim[i] = 0;
		}
		
		for(T2 i=0; i<number_tiles; i++)
		{
			for(T2 j=0; j<len; j++)
			{
				if(lut[i].start_pos[j] < min_vim[j])
					min_vim[j] = lut[i].start_pos[j];
				
				if(lut[i].start_pos[j] > max_vim[j])
					max_vim[j] = lut[i].start_pos[j];
			}
			
		}
		
		return true;
	}
	
	//save as a virtual image
	void y_save(string fn)
	{
		FILE *pFileLUT=0;
		
		pFileLUT = fopen(fn.c_str(),"wt");
		
		// temporary
		strcpy(fn_thumbnail, y_createthumbnail());
		
		fprintf(pFileLUT, "# thumbnail file \n"); // TC_COMMENT1
		fprintf(pFileLUT, "%s \n\n", fn_thumbnail);
		
		fprintf(pFileLUT, "# tiles \n"); // TC_COMMENT2
		fprintf(pFileLUT, "%d \n\n", tilesList.size());
		
		fprintf(pFileLUT, "# dimensions (XYZC) \n"); // TC_COMMENT3
		fprintf(pFileLUT, "%ld %ld %ld %ld \n\n", max_vim[0]-min_vim[0]+1, max_vim[1]-min_vim[1]+1, max_vim[2]-min_vim[2]+1, tilesList.at(0).sz_image[3]);
		
		// init
		origin_x = min_vim[0]; origin_y = min_vim[1]; origin_z = min_vim[2];
		rez_x = 1; rez_y= 1; rez_z = 1;
		
		fprintf(pFileLUT, "# origin (XYZ) \n"); // TC_COMMENT4
		fprintf(pFileLUT, "%lf %lf %lf \n\n", origin_x, origin_y, origin_z);
		
		fprintf(pFileLUT, "# resolution (XYZ) \n"); // TC_COMMENT5
		fprintf(pFileLUT, "%lf %lf %lf \n\n", rez_x, rez_y, rez_z);
		
		fprintf(pFileLUT, "# image coordinates look up table \n"); // TC_COMMENT6
		for(int j=0; j<tilesList.size(); j++)
		{
			
			string fn = QString(lut[j].fn_img.c_str()).remove(0, QFileInfo(QString(lut[j].fn_img.c_str())).path().length()+1).toStdString();
			
			fprintf(pFileLUT, "%s  ( %ld, %ld, %ld ) ( %ld, %ld, %ld ) \n", fn.c_str(), lut[j].start_pos[0], lut[j].start_pos[1], lut[j].start_pos[2], lut[j].end_pos[0], lut[j].end_pos[1], lut[j].end_pos[2]);
		}
		
		fclose(pFileLUT);
	}
	
	// when add a new one into tileList, need to update the whole tileList
	void y_update()
	{
		
	}
	
	// make a visual image real and be loaded into memory
	void y_visualize(T2 *start, T2 *end)
	{
		
	}
	
	// make a visual image real and be loaded into memory
	void y_visualize()
	{
		
	}
	
	// show a header info
	void y_info()
	{
		
	}
	
	// point navigation
	POINT y_navigate(POINT p)
	{
		
	}
	
	// create thumbnail file
	char *y_createthumbnail()
	{
		char fn[2048] = "NULL";
		
		// to do
		
		return fn;
	}
	
	// construct lookup table given adjusted tilesList
	void y_clut(T2 n)
	{
		lut = new LUT [n];
		
		for(T2 i=0; i<n; i++)
		{
			lut[i] = LUT(tilesList.at(i).offsets, tilesList.at(i).sz_image, true);
			
			lut[i].fn_img = tilesList.at(i).fn_image;
		}
		
		// suppose image dimension is unsigned
		T2 len = 3;
		
		try
		{
			min_vim = new T2 [len];
			max_vim = new T2 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T2 i=0; i<len; i++)
		{
			min_vim[i] = 0; max_vim[i] = 0;
		}
		
		for(T2 i=0; i<n; i++)
		{
			for(T2 j=0; j<len; j++)
			{
				if(lut[i].start_pos[j] < min_vim[j])
					min_vim[j] = lut[i].start_pos[j];
				
				if(lut[i].end_pos[j] > max_vim[j])
					max_vim[j] = lut[i].end_pos[j];
			}
			
		}
		
	}
	
	void y_clear()
	{
		if(pVim) {delete []pVim; pVim=0;}
		if(sz) {delete []sz; sz=0;}
		
		if(min_vim) {delete []min_vim; min_vim=0;}
		if(max_vim) {delete []max_vim; max_vim=0;}
		if(lut) {delete []lut; lut=0;}
	}
	
public:
	
	T1 *pVim;
	T2 *sz;
	
	vector<indexed_t> tilesList;
	bitset<3> relative_dir; // 000 'f', 'u', 'l' ; 111 'b', 'd', 'r'; relative[2] relative[1] relative[0] 
	
	LUT *lut;
	T2 *min_vim, *max_vim;
	
	T2 number_tiles;
	
	// record in .tc file
	double origin_x, origin_y, origin_z; // (0, 0, 0)  
	double rez_x, rez_y, rez_z; // (1, 1, 1)
	
	// thumbnail file
	char fn_thumbnail[2048];
	
};

// configuration prior
template <class T>
class CONF_INFO
{
public:
	
	CONF_INFO(){}
	~CONF_INFO(){}
	
public:
	
	T row, col;
	string fn_img;
	
};

// usually T1 is assigned as unsigned char (uint8) and T2 V3DLONG (unsigned V3DLONG int)
template <class T1, class T2>
class Y_IMAGE 
{
	
public:
	
	//initialize a Y_IMAGE from a 1d pointer
	Y_IMAGE(T1 *pIn, T2 *szIn)
	{ 
		flag_refer=true;
		
		pImg=pIn; sz=szIn; 
	}
	//new a Y_IMAGE
	Y_IMAGE(T2 *szIn)
	{
		flag_refer=false;
		
		sz = szIn;
		
		T2 totalsz;
		for(T2 i=0; i<sz.size(); i++)
		{
			totalsz *= sz[i];
		}
		
		pImg=new T1 [totalsz];
		if (!pImg) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		else
		{
			for(T2 i=0; i<totalsz; i++)
			{
				pImg[i] = 0;
			}
		}
	}	
	
	~Y_IMAGE()
	{
		if(!flag_refer && pImg)
		{
			delete []pImg; pImg=0;
		}
	}
	
	
public:
	T1 *pImg;
	T2 *sz;
	
	bool flag_refer;
	
};

// define Y_IMG types
typedef Y_IMAGE<REAL, V3DLONG> Y_IMG_REAL;
typedef Y_IMAGE<unsigned char, V3DLONG> Y_IMG_UINT8;
typedef Y_IMAGE<unsigned short, V3DLONG> Y_IMG_UINT16;

// define a peak data structure of phase correlation
template <class T>
struct P
{
	T x,y,z;
	REAL value;
};

typedef	P<V3DLONG> PEAKS; 
typedef	P<REAL> rPEAKS; 

// define PEAKSLIST type
typedef	std::vector<PEAKS> PEAKSLIST; 
typedef	std::vector<rPEAKS> rPEAKSLIST; 

//NCC FFT-based using sum-table
//http://www.idiom.com/~zilla/Work/nvisionInterface/nip.html
template <class T1, class T2>
class NST
{
	
public:
	
	NST(T2 *sz, T1 *f, T2 even_odd, bool fftwf_in_place, T2 dims)
	{
		if(dims == 2)
			initSumTable2D(sz, f);
		else if (dims == 3)
			initSumTable3D(sz, f, even_odd, fftwf_in_place);
		else if (dims > 3)
			initSumTableND(sz, f);
		else
		{
			printf("Dimensions should be 2, 3, or any positive integer greater than 3.\n");
			return;
		}
	}
	
	~NST()
	{
		if(sum1) {delete []sum1; sum1=0;}
		if(sum2) {delete []sum2; sum2=0;}
	}
	
	// init 2D sum tables
	void initSumTable2D(T2 *sz2d, T1 *f)
	{
		T2 sx=sz2d[0], sy=sz2d[1];
		T2 len=sx*sy;

		try
		{
			sum1 = new T1 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		try
		{
			sum2 = new T1 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
	
		for(T2 i=0; i<len; i++)
		{
			sum1[i] = 0; sum2[i] = 0;
		}
		
		for(T2 j=0; j<sy; j++)
		{
			T2 offset_j = j*sx;
			for(T2 i=0; i<sx; i++)
			{
				T2 idx = offset_j + i;
				
				if(i==0 && j==0)
				{
					sum1[idx] = f[idx];
					sum2[idx] = f[idx]*f[idx];
				}
				else if(i==0 && j>0)
				{
					sum1[idx] = f[idx] + sum1[(j-1)*sx + i];
					sum2[idx] = f[idx]*f[idx] + sum2[(j-1)*sx + i];
				}
				else if(i>0 && j==0)
				{
					sum1[idx] = f[idx] + sum1[offset_j + i-1];
					sum2[idx] = f[idx]*f[idx] + sum2[offset_j + i-1];
				}
				else
				{
					sum1[idx] = f[idx] + sum1[(j-1)*sx + i] + sum1[offset_j + i-1] -  sum1[(j-1)*sx + i-1];
					sum2[idx] = f[idx]*f[idx] + sum2[(j-1)*sx + i] + sum2[offset_j + i-1] - sum2[(j-1)*sx + i-1];
				}
			}
		}
	}
	
	// init 3D sum tables
	void initSumTable3D(T2 *sz3d, T1 *f, T2 even_odd, bool fftwf_in_place)
	{
		T2 sx_f=sz3d[0], sy=sz3d[1], sz=sz3d[2];
		
		T2 sx;
		
		if(fftwf_in_place)
			sx = sx_f - (2-even_odd);  //2*(sx_f/2-1); // fftwf_in_place
		else
			sx = sx_f;
		
		T2 len=sx*sy*sz;
		
		try
		{
			sum1 = new T1 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		try
		{
			sum2 = new T1 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T2 i=0; i<len; i++)
		{
			sum1[i] = 0; sum2[i] = 0;
		}
		
		for(T2 k=0; k<sz; k++)
		{
			T2 offset_k = k*sx*sy;
			T2 offset_f_k = k*sx_f*sy;
			for(T2 j=0; j<sy; j++)
			{
				T2 offset_j = offset_k + j*sx;
				T2 offset_f_j = offset_f_k + j*sx_f;
				for(T2 i=0; i<sx; i++)
				{
					T2 idx = offset_j + i;
					T2 idx_f = offset_f_j + i;
					
					if(i==0 && j==0 && k==0)
					{
						sum1[idx] = f[idx_f];
						sum2[idx] = f[idx_f]*f[idx_f];
					}
					else if(i>0 && j==0 && k==0)
					{
						sum1[idx] = f[idx_f] + sum1[offset_j + i-1];
						sum2[idx] = f[idx_f]*f[idx_f] + sum2[offset_j + i-1];
					}
					else if(i==0 && j>0 && k==0)
					{
						sum1[idx] = f[idx_f] + sum1[offset_k + (j-1)*sx + i];
						sum2[idx] = f[idx_f]*f[idx_f] + sum2[offset_k + (j-1)*sx + i];
					}
					else if(i==0 && j==0 && k>0)
					{
						sum1[idx] = f[idx_f] + sum1[(k-1)*sx*sy + j*sx + i];
						sum2[idx] = f[idx_f]*f[idx_f] + sum2[(k-1)*sx*sy + j*sx + i];
					}
					else if(i>0 && j>0 && k==0)
					{
						sum1[idx] = f[idx_f] + sum1[offset_j + i-1] + sum1[offset_k + (j-1)*sx + i] - sum1[offset_k + (j-1)*sx + i-1];
						sum2[idx] = f[idx_f]*f[idx_f] + sum2[offset_j + i-1] + sum2[offset_k + (j-1)*sx + i] - sum2[offset_k + (j-1)*sx + i-1];
					}
					else if(i>0 && j==0 && k>0)
					{
						sum1[idx] = f[idx_f] + sum1[offset_j + i-1] + sum1[(k-1)*sx*sy + j*sx + i] - sum1[(k-1)*sx*sy + j*sx + i-1];
						sum2[idx] = f[idx_f]*f[idx_f] + sum2[offset_j + i-1] + sum2[(k-1)*sx*sy + j*sx + i] - sum2[(k-1)*sx*sy + j*sx + i-1];
					}
					else if(i==0 && j>0 && k>0)
					{
						sum1[idx] = f[idx_f] + sum1[offset_k + (j-1)*sx + i]  + sum1[(k-1)*sx*sy + j*sx + i] - sum1[(k-1)*sx*sy + (j-1)*sx + i];
						sum2[idx] = f[idx_f]*f[idx_f] + sum2[offset_k + (j-1)*sx + i] + sum2[(k-1)*sx*sy + j*sx + i] - sum2[(k-1)*sx*sy + (j-1)*sx + i];
					}
					else
					{
						sum1[idx] = f[idx_f] + sum1[offset_j + i-1] + sum1[offset_k + (j-1)*sx + i]  + sum1[(k-1)*sx*sy + j*sx + i] + sum1[(k-1)*sx*sy + (j-1)*sx + i-1] 
									- sum1[offset_k + (j-1)*sx + i-1] - sum1[(k-1)*sx*sy + j*sx + i-1] - sum1[(k-1)*sx*sy + (j-1)*sx + i];
						
						sum2[idx] = f[idx_f]*f[idx_f] + sum2[offset_j + i-1] + sum2[offset_k + (j-1)*sx + i] + sum2[(k-1)*sx*sy + j*sx + i] + sum2[(k-1)*sx*sy + (j-1)*sx + i-1] 
									- sum2[offset_k + (j-1)*sx + i-1] - sum2[(k-1)*sx*sy + j*sx + i-1] - sum2[(k-1)*sx*sy + (j-1)*sx + i];
					}
				}
			}
		}
	}
	
	// init ND sum tables
	void initSumTableND(T2 *sz, T1 *f)
	{
	}
	
public:	
	T1 *sum1, *sum2;
};

// generate a mst for group tiled images
// T1 V3DLONG T2 REAL
template <class T1, class T2>
int mstPrim(vector<indexed_t<T1, T2> > &tilesList)
{
	//
	T1 size = tilesList.size();
	if(size<1)	return -1;
	else if(size==1) return true;
	else if(size==2)
	{
		//step 1. choose 0 as anchor image
		(&tilesList.at(0))->predecessor = -1;
		(&tilesList.at(0))->visited = true;	

		//step 2. adjust 1's offsets
		(&tilesList.at(1))->predecessor = 0;
		(&tilesList.at(1))->visited = true;
		
		(&tilesList.at(1))->offsets[0] = tilesList.at(1).record.at(0).offsets[0];
		(&tilesList.at(1))->offsets[1] = tilesList.at(1).record.at(0).offsets[1];
		(&tilesList.at(1))->offsets[2] = tilesList.at(1).record.at(0).offsets[2];
	}
	else 
	{
		// Prim's algorithm
		//-------------------------------------------------------------
		//1. Start with a tree which contains only one node.
		//2. Identify a node (outside the tree) which is closest to the tree and add the minimum weight edge from that node to some node in the tree and incorporate the additional node as a part of the tree.
		//3. If there are less then n – 1 edges in the tree, go to 2
		//
		
		//step 1. choose 0 as anchor image
		(&tilesList.at(0))->predecessor = -1;
		(&tilesList.at(0))->visited = true;	
		
		//step 2.
		//func extractHighestScoreEdge()
		
		//step 3.
		while(ks(tilesList)) 
		{
			
			T1 ni, n1, n2;
			T2 max_score = 0;
			T1 mse_node; // corresponding maxmum score edge
			T1 parent;
			
			// step 2.
			for(T1 i=0; i<size; i++)
			{
				//
				if(!tilesList.at(i).visited) continue;
				
				ni = i;
				
				//cout<< "test... i "<<i<<endl;
				
				for(T1 j=1; j<size; j++)
				{
					if(tilesList.at(j).visited) continue;
					
					T1 n1 = ni;
					n2 = j;
					
					// let n1>n2
					if(n1<n2)
					{
						T1 tmp = n1;
						n1=n2;
						n2=tmp;
					}
					
					// find highest score edge
					if(tilesList.at(n1).record.at(n2).score > max_score)
					{
						max_score = tilesList.at(n1).record.at(n2).score;
						mse_node = j; parent = i;
					}
					
				} // j
				
			}// i
			
			// add new node to mst
			T1 sn1=mse_node, sn2=parent, coef=1;
			if(sn1<sn2)
			{
				T1 tmp = sn1;
				sn1=sn2;
				sn2=tmp;
				
				coef = -1;
			}
			
			(&tilesList.at(mse_node))->offsets[0] = coef*tilesList.at(sn1).record.at(sn2).offsets[0];
			(&tilesList.at(mse_node))->offsets[1] = coef*tilesList.at(sn1).record.at(sn2).offsets[1];
			(&tilesList.at(mse_node))->offsets[2] = coef*tilesList.at(sn1).record.at(sn2).offsets[2];
			
			(&tilesList.at(mse_node))->predecessor = parent;
			(&tilesList.at(mse_node))->visited = true;
		}
		
	}
	
	
}

// keep searching judgement function
template <class T1, class T2> 
bool ks(vector<indexed_t<T1, T2> > tilesList)
{
	T1 size = tilesList.size();
	
	for(T1 i=0; i<size; i++)
	{
		if(!tilesList.at(i).visited)
		{
			return true;
		}
	}
	
	return false;
}

// YImg
// usually T1 is assigned as double and T2 V3DLONG (unsigned V3DLONG int)
template <class T1, class T2, class Y_IMG1, class Y_IMG2>
class YImg 
{
	
public:
	YImg(){}
	~YImg(){}
	
public:
	
	//func down sampling
	void down_sampling(Y_IMG1 pOut, Y_IMG2 pIn, T1 *scale);
	
	//func padding zeros for FFT using FFTW lib
	void padding(Y_IMG1 pOut, Y_IMG2 pIn, bool flag, bool fftwf_in_place, T2 even_odd, T2 dims);
	
	//func padding zeros 2D
	void padding2D(Y_IMG1 pOut, Y_IMG2 pIn, bool upperleft);
	
	//func padding zeros 3D
	void padding3D(Y_IMG1 pOut, Y_IMG2 pIn, bool frontupperleft, bool fftwf_in_place, T2 even_odd);
	
	//func padding zeros ND
	void paddingND(Y_IMG1 pOut, Y_IMG2 pIn, bool flag);
	
	//func padding zeros 3D using symmetric way
	void padding_mirror_3D(Y_IMG1 pOut, Y_IMG2 pIn, bool frontupperleft, bool fftwf_in_place, T2 even_odd);
	
	//func fft phase-correlation
	void fftpc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, PEAKSLIST *peakList);
	
	//func all-in-one fft phase-correlation and cross-correlation
	void fftpccc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, PEAKSLIST *peakList);
	
	//func all-in-one fft phase-correlation and cross-correlation using 1d fft
	void fftpatientpccc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, PEAKSLIST *peakList);
	
	//func fft phase-correlation combined with normalized cross-correlation
	void fftpcncc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz3d, T2 even_odd, bool fftwf_in_place, T1 overlap_percent, PEAKS *pos);
	
	//func fft cross-correlation
	void fftcc(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, T2 dims);
	
	//func fft cc 2D
	void fftcc2D(Y_IMG1 pOut, Y_IMG2 pIn, bool fftwf_in_place);
	
	//func fft cc 3D
	void fftcc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place);
			   
	//func fft cc ND
	void fftccND(Y_IMG1 pOut, Y_IMG2 pIn, bool fftwf_in_place);
	
	//func fft normalized cross-correlation
	void fftncc(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz, T2 even_odd, bool fftwf_in_place, T1 overlap_percent, T2 dims);
	
	//func fft ncc 2D
	void fftncc2D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz2d, T1 overlap_percent);
	
	//func fft ncc 3D
	void fftncc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz3d, T2 even_odd, bool fftwf_in_place, T1 overlap_percent);
	
	//func fft ncc 3D with one peak
	void fftnccp3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_sub, T2 *sz_tar, T2 even_odd, bool fftwf_in_place, T1 *scale, PEAKS *pos);
	
	//func fft ncc 3D with one peak without precomputing sum table
	void fftnccpns3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_sub, T2 *sz_tar, T2 even_odd, bool fftwf_in_place, T1 *scale, PEAKS *pos);
	
	//func fft ncc ND
	void fftnccND(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sznd, T1 overlap_percent);
	
	//func region growing 3D
	void regiongrow3D(Y_IMG1 pOut, Y_IMG2 pIn, rPEAKSLIST *peakList, T2 ncm);
	
	//func gaussian filtering 3D
	void gaussianfilter3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_w);
	
	// func computing normalized cross correlation
	void cmpt_ncc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_sub, T2 *sz_tar, T2 even_odd, bool fftwf_in_place, PEAKS *&pos);
	
	//compute the power of x
	T1 power( T1 x, T2 n)
	{
		T1 r = 1;
		for ( T2 i = 0; i < n; ++i)
			r *= x;
		return r;
	};
	
	
public:
	//Y_IMAGE pIn, pOut;
};

//------------------------------------------------------------------------------------------------------------------
// down sampling
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: down_sampling(Y_IMG1 pOut, Y_IMG2 pIn, T1 *scale)
{
	
	T1 scale_x=scale[0], scale_y=scale[1], scale_z=scale[2];
	
	//temporary pointer
	T2 ssx,ssy,ssz,ssc;
	
	ssx=pOut.sz[0];
	ssy=pOut.sz[1];
	ssz=pOut.sz[2];
	ssc=pOut.sz[3];
	
	T2 N,M,P,C;
	
	N=pIn.sz[0];
	M=pIn.sz[1];
	P=pIn.sz[2];
	C=pIn.sz[3];
	
	//linear interpolation
	for (T2 k=0;k<ssz;k++)
	{
		T2 kstart=T2(floor(k/scale_z)), kend=kstart+T2(1/scale_z);
		if (kend>P-1) kend = P-1;
		
		if(P==1) kend=1;
		
		for (T2 j=0;j<ssy;j++)
		{
			T2 jstart=T2(floor(j/scale_y)), jend=jstart+T2(1/scale_y);
			if (jend>M-1) jend = M-1;
			
			if(M==1) jend=1;
			
			for (T2 i=0;i<ssx;i++)
			{
				T2 istart=T2(floor(i/scale_x)), iend=istart+T2(1/scale_x);
				if (iend>N-1) iend = N-1;
				
				if(N==1) iend=1;
				
				T2 idx_out = k*ssx*ssy + j*ssx + i;
				
				T2 sum=0;
				
				for(T2 kk=kstart; kk<kend; kk++)
				{
					for(T2 jj=jstart; jj<jend; jj++)
					{
						for(T2 ii=istart; ii<iend; ii++)
						{
							sum += pIn.pImg[kk*M*N + jj*N + ii];
						}
						
					}
				}
				
				if(iend==istart)
					pOut.pImg[idx_out] = pIn.pImg[k*M*N + j*N + istart];
				else if(jend==jstart)
					pOut.pImg[idx_out] = pIn.pImg[k*M*N + jstart*N + i];
				else if(kend==kstart)
					pOut.pImg[idx_out] = pIn.pImg[kstart*M*N + j*N + i];
				else
					pOut.pImg[idx_out] =  sum / ( (kend-kstart)*(jend-jstart)*(iend-istart) );
				
			}
		}
	}
	
}


//------------------------------------------------------------------------------------------------------------------
// padding zeros
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: padding(Y_IMG1 pOut, Y_IMG2 pIn, bool flag, bool fftwf_in_place, T2 even_odd, T2 dims)
{
	if(dims == 2)
		padding2D(pOut, pIn, flag);
	else if (dims == 3)
		padding3D(pOut, pIn, flag, fftwf_in_place, even_odd);
	else if (dims > 3)
		paddingND(pOut, pIn, flag);
	else
	{
		printf("Dimensions should be 2, 3, or any positive integer greater than 3.\n");
		return;
	}

}

// padding zeros 2D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: padding2D(Y_IMG1 pOut, Y_IMG2 pIn, bool upperleft)
{
	T2 sx,sy,tx,ty,szx_pad,szy_pad;
	
	if(upperleft)
	{
		sx=pIn.sz[0];
		sy=pIn.sz[1];
		szx_pad=pOut.sz[0];
		szy_pad=pOut.sz[1];
		tx=szx_pad+1-sx;
		ty=szy_pad+1-sy;
	}
	else
	{
		tx=pIn.sz[0];
		ty=pIn.sz[1];
		szx_pad=pOut.sz[0];
		szy_pad=pOut.sz[1];
		sx=szx_pad+1-tx;
		sy=szy_pad+1-ty;
	}
	
	if(upperleft)
	{
		for(T2 j=0; j<sy; j++)
		{
			T2 offset_j = j*szx_pad;
			T2 offsets = j*sx;
			for(T2 i=0; i<sx; i++)
			{
				pOut.pImg[offset_j + i] = pIn.pImg[offsets + i];
			}
		}
	}
	else
	{
		for(T2 j=sy-1; j<szy_pad; j++)
		{
			T2 offset_j = j*szx_pad;
			T2 offsets = (j-sy+1)*tx;
			for(T2 i=sx-1; i<szx_pad; i++)
			{
				pOut.pImg[offset_j + i] = pIn.pImg[offsets + i-sx+1];
			}
		}
	}
}

// padding zeros 3D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: padding3D(Y_IMG1 pOut, Y_IMG2 pIn, bool frontupperleft, bool fftwf_in_place, T2 even_odd)
{
	// by default padding with zeros
	
	T2 sx,sy,sz, tx,ty,tz, szx_pad,szy_pad, szz_pad;
	
	szx_pad=pOut.sz[0];
	szy_pad=pOut.sz[1];
	szz_pad=pOut.sz[2];
	
	T2 szx_pad_tmp;
	
	if(fftwf_in_place)
		szx_pad_tmp = szx_pad - (2-even_odd); //2*(szx_pad/2-1); //fftwf_in_place
	else
		szx_pad_tmp = szx_pad;
	
	if(frontupperleft)
	{
		sx=pIn.sz[0];
		sy=pIn.sz[1];
		sz=pIn.sz[2];

		tx=szx_pad_tmp+1-sx; // fftwf_in_place
		ty=szy_pad+1-sy;
		tz=szz_pad+1-sz;
	}
	else
	{
		tx=pIn.sz[0];
		ty=pIn.sz[1];
		tz=pIn.sz[2];
		
		sx=szx_pad_tmp+1-tx; // fftwf_in_place
		sy=szy_pad+1-ty;
		sz=szz_pad+1-tz;
	}

	// padding
	// 
	if(frontupperleft)
	{
		for(T2 k=0; k<sz; k++)
		{
			T2 offset_k = k*szx_pad*szy_pad;
			T2 offsets_k = k*sx*sy;
			for(T2 j=0; j<sy; j++)
			{
				T2 offset_j = offset_k + j*szx_pad;
				T2 offsets = offsets_k + j*sx;
				for(T2 i=0; i<sx; i++)
				{
					pOut.pImg[offset_j + i] = pIn.pImg[offsets + i];
				}
			}
		}
	}
	else
	{
		for(T2 k=sz-1; k<szz_pad; k++)
		{
			T2 offset_k = k*szx_pad*szy_pad;
			T2 offsets_k = (k-sz+1)*tx*ty;
			for(T2 j=sy-1; j<szy_pad; j++)
			{
				T2 offset_j = offset_k + j*szx_pad;
				T2 offsets = offsets_k + (j-sy+1)*tx;
				for(T2 i=sx-1; i<szx_pad_tmp; i++) // fftwf_in_place
				{
					pOut.pImg[offset_j + i] = pIn.pImg[offsets + i-sx+1];
				}
			}
		}
	}
}

// padding zeros ND
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: paddingND(Y_IMG1 pOut, Y_IMG2 pIn, bool flag)
{

}

// 3D image mirror padding
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: padding_mirror_3D(Y_IMG1 pOut, Y_IMG2 pIn, bool frontupperleft, bool fftwf_in_place, T2 even_odd)
{
	// using symmetric way
	
	T2 sx,sy,sz, tx,ty,tz, szx_pad,szy_pad, szz_pad;
	
	szx_pad=pOut.sz[0];
	szy_pad=pOut.sz[1];
	szz_pad=pOut.sz[2];
	
	T2 szx_pad_tmp;
	
	if(fftwf_in_place)
		szx_pad_tmp = szx_pad - (2-even_odd); //2*(szx_pad/2-1); //fftwf_in_place
	else
		szx_pad_tmp = szx_pad;
	
	if(frontupperleft)
	{
		sx=pIn.sz[0];
		sy=pIn.sz[1];
		sz=pIn.sz[2];
		
		tx=szx_pad_tmp+1-sx; // fftwf_in_place
		ty=szy_pad+1-sy;
		tz=szz_pad+1-sz;
	}
	else
	{
		tx=pIn.sz[0];
		ty=pIn.sz[1];
		tz=pIn.sz[2];
		
		sx=szx_pad_tmp+1-tx; // fftwf_in_place
		sy=szy_pad+1-ty;
		sz=szz_pad+1-tz;
	}
	
	// padding
	V3DLONG x, y, z;
	
	if(frontupperleft)
	{
		for(T2 k=0; k<sz+8; k++) //szz_pad
		{
			T2 offset_k = k*szx_pad*szy_pad;
			T2 offsets_k = k*sx*sy;
			for(T2 j=0; j<sy+8; j++) //szy_pad
			{
				T2 offset_j = offset_k + j*szx_pad;
				T2 offsets = offsets_k + j*sx;
				for(T2 i=0; i<sx+8; i++) //szx_pad
				{
					V3DLONG idx = offset_j + i;
					
					if(i<sx && j<sy && k<sz)
					{
						pOut.pImg[idx] = pIn.pImg[offsets + i];
					}
					else
					{
						if(i>=sx) x = sx - (i-sx+2); else x = i;
						if(j>=sy) y = sy - (j-sy+2); else y = j;
						if(k>=sz) z = sz - (k-sz+2); else z = k;
						
						if(x>=0 && y>=0 && z>=0)
						{
							pOut.pImg[idx] = pIn.pImg[z*sx*sy + y*sx + x];
						}
						
					}
				}
			}
		}
	}
	else
	{
		for(T2 k=sz-9; k<szz_pad; k++) //
		{
			T2 offset_k = k*szx_pad*szy_pad;
			T2 offsets_k = (k-sz+1)*tx*ty;
			for(T2 j=sy-9; j<szy_pad; j++) //
			{
				T2 offset_j = offset_k + j*szx_pad;
				T2 offsets = offsets_k + (j-sy+1)*tx;
				for(T2 i=sx-9; i<szx_pad_tmp; i++) // fftwf_in_place
				{
					V3DLONG idx = offset_j + i;
					
					x = i-sx+1;
					y = j-sy+1;
					z = k-sz+1;
					
					if(x>=0 && y>=0 && z>=0)
					{
						pOut.pImg[idx] = pIn.pImg[offsets + i-sx+1];
					}
					else
					{
						if (x < 0)
						{
							int tmp = 0;
							int dir = 1;
							
							while (x < 0)
							{
								tmp += dir;
								if (tmp == sx - 1 || tmp == 0) dir *= -1;
								x++;
							}
							x = tmp;
						}
						
						if (y < 0)
						{
							int tmp = 0;
							int dir = 1;
							
							while (y < 0)
							{
								tmp += dir;
								if (tmp == sy - 1 || tmp == 0) dir *= -1;
								y++;
							}
							y = tmp;
						}
						
						if (z < 0)
						{
							int tmp = 0;
							int dir = 1;
							
							while (z < 0)
							{
								tmp += dir;
								if (tmp == sz - 1 || tmp == 0) dir *= -1;
								z++;
							}
							z = tmp;
						}
						
						if(x>=0 && y>=0 && z>=0)
						{
							pOut.pImg[idx] = pIn.pImg[z*tx*ty + y*tx + x];
						}
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------------
// fft phase-correlation

// fft pc 3D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftpc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, PEAKSLIST *peakList)
{
	// peaks
	PEAKS pos;
	
	T1 max_v=0;
	
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	//
	//fftwf_init_threads(); //pthread
	//fftwf_plan_with_nthreads(PROCESSORS); 
	
	fftwf_plan p;
	
	if(fftwf_in_place)
	{
		T2 sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
		T2 len_pad_tmp = sx_pad_ori*sy_pad*sz_pad;
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad_ori, pIn.pImg, (fftwf_complex*)pIn.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad_ori, pOut.pImg, (fftwf_complex*)pOut.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		// obtain the cross power spectrum
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i+=2)
				{
					T2 idx = offset_j + i;
					
					T1 tmp = pOut.pImg[idx];
					
					pOut.pImg[idx+1] = -pOut.pImg[idx+1]; //conjugate
					
					pOut.pImg[idx] = pIn.pImg[idx]*tmp - pIn.pImg[idx+1]*pOut.pImg[idx+1];
					pOut.pImg[idx+1] = pIn.pImg[idx+1]*tmp + pIn.pImg[idx]*pOut.pImg[idx+1];
					
					T1 tmp2 = sqrt(power(pOut.pImg[idx], 2) + power(pOut.pImg[idx+1], 2));
					
					pOut.pImg[idx] /= tmp2;
					pOut.pImg[idx+1] /= tmp2;
					
				}
			}
		}
		
		// obtain the phase correlation
		fftwf_complex* out_sub = (fftwf_complex*)pOut.pImg;
		
		p = fftwf_plan_dft_c2r_3d(sz_pad, sy_pad, sx_pad_ori, out_sub, pOut.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		// normalize
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i++)
				{
					T2 idx = offset_j + i;
					
					if(i<sx_pad_ori)
					{
						pOut.pImg[idx] /= (T1)len_pad_tmp; //
					}
					else
						pOut.pImg[idx] = 0;
					
					// statistics of peaks
					if(pOut.pImg[idx] > max_v)
					{
						max_v = pOut.pImg[idx];
						
						pos.x = i; pos.y = j; pos.z = k; pos.value = max_v;
						
						if(peakList->size()<1)
							peakList->push_back(pos);
						else
						{
							for(unsigned int it=peakList->size(); it!=0; it--)
							{
								if(pos.value>=peakList->at(it-1).value)
								{
									peakList->insert(peakList->begin() + it, 1, pos);
									
									if(peakList->size()>NPEAKS)
										peakList->erase(peakList->begin());
									
									break;
								}
								else
									continue;
								
							}
							if(pos.value<peakList->at(0).value && peakList->size()<NPEAKS)
								peakList->insert(peakList->begin(), pos);
						}
					}
					
				}
			}
		}
		
	}
	else
	{
		
		T2 fsx = sx_pad/2+1;
		T2 out_dims = sz_pad*sy_pad*fsx;
		
		fftwf_complex* out_sub;
		fftwf_complex* out_tar;
		
		out_sub = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out_dims);
		out_tar = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out_dims);
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad, pIn.pImg, out_tar, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad, pOut.pImg, out_sub, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		// obtain the cross power spectrum
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*fsx;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*fsx;
				for(T2 i=0; i<fsx; i++)
				{
					T2 idx = offset_j + i;
					
					//out_sub as output
					T1 tmp = out_sub[idx][0];
					
					out_sub[idx][1] = -out_sub[idx][1]; //conjugate
					
					out_sub[idx][0] = out_tar[idx][0]*tmp - out_tar[idx][1]*out_sub[idx][1];
					out_sub[idx][1] = out_tar[idx][1]*tmp + out_tar[idx][0]*out_sub[idx][1];
					
					T1 tmp2 = sqrt(power(out_sub[idx][0], 2) + power(out_sub[idx][1], 2));
					
					out_sub[idx][0] /= tmp2;
					out_sub[idx][1] /= tmp2;
					
				}
			}
		}
		
		// obtain the phase correlation
		for(T2 i=0; i<len_pad; i++)
			pOut.pImg[i] = 0;
		
		p = fftwf_plan_dft_c2r_3d(sz_pad, sy_pad, sx_pad, out_sub, pOut.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);

		// normalize
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i++)
				{
					T2 idx = offset_j + i;
					
					pOut.pImg[idx] /= (T1)len_pad;
					
					// statistics of peaks
					if(pOut.pImg[idx] > max_v)
					{
						max_v = pOut.pImg[idx];
						
						pos.x = i; pos.y = j; pos.z = k; pos.value = max_v;
						
						if(peakList->size()<1)
							peakList->push_back(pos);
						else
						{
							for(unsigned int it=peakList->size(); it!=0; it--)
							{
								if(pos.value>=peakList->at(it-1).value)
								{
									peakList->insert(peakList->begin() + it, 1, pos);
									
									if(peakList->size()>NPEAKS)
										peakList->erase(peakList->begin());
									
									break;
								}
								else
									continue;
								
							}
							if(pos.value<peakList->at(0).value && peakList->size()<NPEAKS)
								peakList->insert(peakList->begin(), pos);
						}
						
						
					}
					
					
				}
			}
		}
		
		//de-alloc
		fftwf_free(out_sub);
		fftwf_free(out_tar);	
		
	}
	
	// cleanup
	// fftwf_cleanup_threads();
}

//------------------------------------------------------------------------------------------------------------------
// fft cross-correlation
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftcc(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, T2 dims)
{
	if(dims == 2)
		fftcc2D(pOut, pIn, fftwf_in_place);
	else if (dims == 3)
		fftcc3D(pOut, pIn, even_odd, fftwf_in_place);
	else if (dims > 3)
		fftccND(pOut, pIn, fftwf_in_place);
	else
	{
		printf("Dimensions should be 2, 3, or any positive integer greater than 3.\n");
		return;
	}
		
}

// fft cc 2D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftcc2D(Y_IMG1 pOut, Y_IMG2 pIn, bool fftwf_in_place)
{
	//assuming the pIn and pOut already padded by zeros
	//subject is on the top left (do conjugate in the frequancy) and target is on the bottom right
	
	//pOut is subject pIn is target with same size
	
	// fftwf_out_place
	
	T2 sx_pad, sy_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	
	T2 len_pad = sx_pad*sy_pad;
	
	T2 fsx = sx_pad/2+1;
	T2 out_dims = sy_pad*fsx;
	
	fftwf_complex* out_sub;
	fftwf_complex* out_tar;
	
	out_sub = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out_dims);
	out_tar = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out_dims);
	
	//fftwf_init_threads(); //pthread
	//fftwf_plan_with_nthreads(8); 
	
	fftwf_plan p;
	
	p = fftwf_plan_dft_r2c_2d(sy_pad, sx_pad, pIn.pImg, out_tar, FFTW_ESTIMATE);
	
	fftwf_execute(p);
	fftwf_destroy_plan(p);
	
	p = fftwf_plan_dft_r2c_2d(sy_pad, sx_pad, pOut.pImg, out_sub, FFTW_ESTIMATE);
	
	fftwf_execute(p);
	fftwf_destroy_plan(p);
	
	//fft cc
	for(T2 j=0; j<sy_pad; j++)
	{
		T2 offset_j = j*fsx;
		for(T2 i=0; i<fsx; i++)
		{
			T2 idx = offset_j + i;
			
			//out_sub as output
			T1 tmp = out_sub[idx][0];
			
			out_sub[idx][1] = -out_sub[idx][1]; //conjugate
			
			out_sub[idx][0] = out_tar[idx][0]*tmp - out_tar[idx][1]*out_sub[idx][1];
			out_sub[idx][1] = out_tar[idx][1]*tmp + out_tar[idx][0]*out_sub[idx][1];
			
		}
	}
	
	for(T2 i=0; i<len_pad; i++)
		pOut.pImg[i] = 0;
	
	p = fftwf_plan_dft_c2r_2d(sy_pad, sx_pad, out_sub, pOut.pImg, FFTW_ESTIMATE);
	
	fftwf_execute(p);
	fftwf_destroy_plan(p);
	
	for(T2 i=0; i<len_pad; i++)
	{
		pOut.pImg[i] /= len_pad;
	}
	
	//de-alloc
	fftwf_free(out_sub);
	fftwf_free(out_tar);	
	
	// cleanup
	//fftwf_cleanup_threads();
}

// fft cc 3D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftcc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place)
{
	
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	//
	//fftwf_init_threads(); //pthread
	//fftwf_plan_with_nthreads(PROCESSORS); 
	
	fftwf_plan p;
	
	if(fftwf_in_place)
	{
		T2 sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
		T2 len_pad_tmp = sx_pad_ori*sy_pad*sz_pad;

		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad_ori, pIn.pImg, (fftwf_complex*)pIn.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad_ori, pOut.pImg, (fftwf_complex*)pOut.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i+=2)
				{
					T2 idx = offset_j + i;
					
					T1 tmp = pOut.pImg[idx];
					
					pOut.pImg[idx+1] = -pOut.pImg[idx+1]; //conjugate
					
					pOut.pImg[idx] = pIn.pImg[idx]*tmp - pIn.pImg[idx+1]*pOut.pImg[idx+1];
					pOut.pImg[idx+1] = pIn.pImg[idx+1]*tmp + pIn.pImg[idx]*pOut.pImg[idx+1];
					
				}
			}
		}
		
		fftwf_complex* out_sub = (fftwf_complex*)pOut.pImg;
		//fftwf_complex* out_tar = (fftwf_complex*)pIn.pImg;

		p = fftwf_plan_dft_c2r_3d(sz_pad, sy_pad, sx_pad_ori, out_sub, pOut.pImg, FFTW_ESTIMATE);

		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i++)
				{
					T2 idx = offset_j + i;
					
					if(i<sx_pad_ori)
					{
						pOut.pImg[idx] /= (T1)len_pad_tmp; //
					}
					else
						pOut.pImg[idx] = 0;
				}
			}
		}
	
	}
	else
	{
		
		T2 fsx = sx_pad/2+1;
		T2 out_dims = sz_pad*sy_pad*fsx;
		
		fftwf_complex* out_sub;
		fftwf_complex* out_tar;
		
		out_sub = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out_dims);
		out_tar = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out_dims);
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad, pIn.pImg, out_tar, FFTW_ESTIMATE);
	
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad, pOut.pImg, out_sub, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		//fft cc
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*fsx;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*fsx;
				for(T2 i=0; i<fsx; i++)
				{
					T2 idx = offset_j + i;
					
					//out_sub as output
					T1 tmp = out_sub[idx][0];
					
					out_sub[idx][1] = -out_sub[idx][1]; //conjugate
					
					out_sub[idx][0] = out_tar[idx][0]*tmp - out_tar[idx][1]*out_sub[idx][1];
					out_sub[idx][1] = out_tar[idx][1]*tmp + out_tar[idx][0]*out_sub[idx][1];
		
				}
			}
		}
		
		for(T2 i=0; i<len_pad; i++)
			pOut.pImg[i] = 0;
		
		p = fftwf_plan_dft_c2r_3d(sz_pad, sy_pad, sx_pad, out_sub, pOut.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		for(T2 i=0; i<len_pad; i++)
		{
			pOut.pImg[i] /= (T1)len_pad;
		}
		
		//de-alloc
		fftwf_free(out_sub);
		fftwf_free(out_tar);	
		
	}
	
	// cleanup
	//fftwf_cleanup_threads();
}

// fft cc ND
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftccND(Y_IMG1 pOut, Y_IMG2 pIn, bool fftwf_in_place)
{
	
}

// fft normalized cross-correlation
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftncc(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz, T2 even_odd, bool fftwf_in_place, T1 overlap_percent, T2 dims)
{
	if(dims == 2)
		fftncc2D(pOut, pIn, sz, overlap_percent);
	else if (dims == 3)
		fftncc3D(pOut, pIn, sz, even_odd, fftwf_in_place, overlap_percent);
	else if (dims > 3)
		fftnccND(pOut, pIn, sz, overlap_percent);
	else
	{
		printf("Dimensions should be 2, 3, or any positive integer greater than 3.\n");
		return;
	}
	
}

// fft ncc 2D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftncc2D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz2d, T1 overlap_percent)
{
	//assuming the pIn and pOut already padded by zeros
	//subject is on the top left (do conjugate in the frequancy) and target is on the bottom right
	
	T2 tx=sz2d[0], ty=sz2d[1];
	T2 sx_pad, sy_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	
	T2 len_pad = sx_pad*sy_pad;
	
	T2 sx, sy;
	
	sx=sx_pad+1-tx;
	sy=sy_pad+1-ty;
	
	T2 sub_sz = sx*sy;
	
	NST<T1, T2> ncctar(pIn.sz, pIn.pImg, 0, false, 2);
	NST<T1, T2> nccsub(pOut.sz, pOut.pImg, 0, false, 2);
	
	//cross-correlation
	fftcc2D(pOut, pIn, false);
	
	//
	for(T2 v=0; v<sy_pad; v++)
	{
		T2 offset_v = v*sx_pad;
		for(T2 u=0; u<sx_pad; u++)
		{
			T2 idx = offset_v + u;
			
			T2 t_lu_idx, t_ru_idx, t_ld_idx, t_rd_idx;
			T2 s_lu_idx, s_ru_idx, s_ld_idx, s_rd_idx;
			
			T2 t_lx, t_rx, t_uy, t_dy;
			T2 s_lx, s_rx, s_uy, s_dy;
			
			T1 t_std, t_mean;
			T1 s_std, s_mean;
			
			
			if(u<sx && u<tx)
			{
				s_lx = sx-1 - u; s_rx = sx-1;
				t_lx = sx-1; t_rx = sx-1 + u;
			}
			else if(u<sx && u>=tx)
			{
				s_lx = sx-1 - u; s_rx = sx_pad-1 - u;
				t_lx = sx-1; t_rx = sx_pad-1;
			}
			else if(u>=sx && u<tx)
			{
				s_lx = 0; s_rx = sx-1;
				t_lx = u; t_rx = sx-1 + u;
			}
			else if(u>=tx)
			{
				s_lx = 0; s_rx = sx_pad-1 - u;
				t_lx = u; t_rx = sx_pad-1;
			}
			else
				printf("x direction %ld %ld %ld %ld \n", u, sx, tx, sx_pad);
			
			if(v<sy && v<ty)
			{
				s_uy = sy-1 - v; s_dy = sy-1;
				t_uy = sy-1; t_dy = sy-1 + v;
			}
			else if(v<sy && v>=ty)
			{
				s_uy = sy-1 - v; s_dy = sy_pad-1 - v;
				t_uy = sy-1; t_dy = sy_pad-1;
			}
			else if(v<ty && v>=sy)
			{
				s_uy = 0; s_dy = sy-1;
				t_uy = v; t_dy = sy-1 + v;
			}
			else if(v>=ty)
			{
				s_uy = 0; s_dy = sy_pad-1 - v;
				t_uy = v; t_dy = sy_pad-1;
			}
			else
				printf("y direction %ld %ld %ld %ld \n", v, sy, ty, sy_pad);
			
			
			if(t_rx<t_lx || t_dy<t_uy || s_rx<s_lx || s_dy<s_uy)
			{
				pOut.pImg[idx] = 0;
				continue;
			}
			
			t_lu_idx = (t_uy-1)*sx_pad + t_lx-1;
			t_ru_idx = (t_uy-1)*sx_pad + t_rx;
			t_ld_idx = t_dy*sx_pad + t_lx-1;
			t_rd_idx = t_dy*sx_pad + t_rx;
			
			s_lu_idx = (s_uy-1)*sx_pad + s_lx-1;
			s_ru_idx = (s_uy-1)*sx_pad + s_rx;
			s_ld_idx = s_dy*sx_pad + s_lx-1;
			s_rd_idx = s_dy*sx_pad + s_rx;
			
			T2 len_t = (t_dy - t_uy + 1)*(t_rx - t_lx + 1), len_s = (s_dy - s_uy + 1)*(s_rx - s_lx + 1);
		
			// prior knowledge of ratio of overlap
			if(len_s<sub_sz*overlap_percent)
			{
				pOut.pImg[idx] = 0;
				continue;
			}
			
			T1 t1,t2;
			
			if(t_lx==0 && t_uy==0)
			{
				t1 = ncctar.sum1[t_rd_idx];
				t2 = ncctar.sum2[t_rd_idx];
			}
			else if(t_lx==0 && t_uy>0)
			{
				t1 = ncctar.sum1[t_rd_idx] - ncctar.sum1[t_ru_idx];
				t2 = ncctar.sum2[t_rd_idx] - ncctar.sum2[t_ru_idx];
			}
			else if(t_lx>0 && t_uy==0)
			{
				t1 = ncctar.sum1[t_rd_idx] - ncctar.sum1[t_ld_idx];
				t2 = ncctar.sum2[t_rd_idx] - ncctar.sum2[t_ld_idx];
			}
			else
			{
				t1 = fabs( ncctar.sum1[t_rd_idx] + ncctar.sum1[t_lu_idx] - ncctar.sum1[t_ld_idx] - ncctar.sum1[t_ru_idx] );
				t2 = fabs( ncctar.sum2[t_rd_idx] + ncctar.sum2[t_lu_idx] - ncctar.sum2[t_ld_idx] - ncctar.sum2[t_ru_idx] );
			}
			
			t_mean = t1/len_t; 
			t_std = sqrt(y_max(t2 - t1*t_mean, T1(0) ));
			
			T1 s1,s2;
			
			if(s_lx==0 && s_uy==0)
			{
				s1 = nccsub.sum1[s_rd_idx];
				s2 = nccsub.sum2[s_rd_idx];
			}
			else if(s_lx==0 && s_uy>0)
			{
				s1 = nccsub.sum1[s_rd_idx] - nccsub.sum1[s_ru_idx];
				s2 = nccsub.sum2[s_rd_idx] - nccsub.sum2[s_ru_idx];
			}
			else if(s_lx>0 && s_uy==0)
			{
				s1 = nccsub.sum1[s_rd_idx] - nccsub.sum1[s_ld_idx];
				s2 = nccsub.sum2[s_rd_idx] - nccsub.sum2[s_ld_idx];
			}
			else
			{
				s1 = fabs( nccsub.sum1[s_rd_idx] + nccsub.sum1[s_lu_idx] - nccsub.sum1[s_ld_idx] - nccsub.sum1[s_ru_idx] );
				s2 = fabs( nccsub.sum2[s_rd_idx] + nccsub.sum2[s_lu_idx] - nccsub.sum2[s_ld_idx] - nccsub.sum2[s_ru_idx] );
			}
			
			s_mean = s1/len_s;
			s_std = sqrt(y_max(s2 - s1*s_mean, T1(0) ));
			
			// response [0, 1] (instead of [-1, 1])
			if(t_std!=0 && s_std!=0)
			{
				pOut.pImg[idx] = 0.5 + 0.5*(pOut.pImg[idx] - t1*s_mean)/(t_std*s_std);
			}
			else 
			{
				pOut.pImg[idx] = 0;
			}
			
		}			
	}	
	
}

// fft ncc 3D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftncc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz3d, T2 even_odd, bool fftwf_in_place, T1 overlap_percent)
{
	
	T2 tx=sz3d[0], ty=sz3d[1], tz=sz3d[2];
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	T2 sx, sy, sz;
	
	T2 sx_pad_ori;
	
	if(fftwf_in_place)
		sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
	else
		sx_pad_ori = sx_pad;
	
	sx=sx_pad_ori+1-tx; // fftwf_in_place
	sy=sy_pad+1-ty;
	sz=sz_pad+1-tz;
	
	T2 sub_sz = sx*sy*sz;
	
	NST<T1, T2> ncctar(pIn.sz, pIn.pImg, even_odd, fftwf_in_place, 3);
	NST<T1, T2> nccsub(pOut.sz, pOut.pImg, even_odd, fftwf_in_place, 3);
	
	//cross-correlation
	fftcc3D(pOut, pIn, even_odd, fftwf_in_place);
	
	//
	for(T2 w=0; w<sz_pad; w++)
	{
		T2 offset_w = w*sx_pad*sy_pad;
		for(T2 v=0; v<sy_pad; v++)
		{
			T2 offset_v = offset_w + v*sx_pad;
			for(T2 u=0; u<sx_pad_ori; u++) // fftwf_in_place
			{
				T2 idx = offset_v + u; 
				
				// z - (front, back) y - (upper, down) x - (left, right)
				T2 t_ful_idx, t_fur_idx, t_fdl_idx, t_fdr_idx, t_bul_idx, t_bur_idx, t_bdl_idx, t_bdr_idx;
				T2 s_ful_idx, s_fur_idx, s_fdl_idx, s_fdr_idx, s_bul_idx, s_bur_idx, s_bdl_idx, s_bdr_idx;
				
				T2 t_lx, t_rx, t_uy, t_dy, t_fz, t_bz;
				T2 s_lx, s_rx, s_uy, s_dy, s_fz, s_bz;
				
				T1 t_std, t_mean;
				T1 s_std, s_mean;
				
				if(u<sx && u<tx)
				{
					s_lx = sx-1 - u; s_rx = sx-1;
					t_lx = sx-1; t_rx = sx-1 + u;
				}
				else if(u<sx && u>=tx)
				{
					s_lx = sx-1 - u; s_rx = sx_pad_ori-1 - u;
					t_lx = sx-1; t_rx = sx_pad_ori-1;
				}
				else if(u>=sx && u<tx)
				{
					s_lx = 0; s_rx = sx-1;
					t_lx = u; t_rx = sx-1 + u;
				}
				else if(u>=tx)
				{
					s_lx = 0; s_rx = sx_pad_ori-1 - u;
					t_lx = u; t_rx = sx_pad_ori-1;
				}
				else
					printf("x direction %ld %ld %ld %ld \n", u, sx, tx, sx_pad);
				
				if(v<sy && v<ty)
				{
					s_uy = sy-1 - v; s_dy = sy-1;
					t_uy = sy-1; t_dy = sy-1 + v;
				}
				else if(v<sy && v>=ty)
				{
					s_uy = sy-1 - v; s_dy = sy_pad-1 - v;
					t_uy = sy-1; t_dy = sy_pad-1;
				}
				else if(v<ty && v>=sy)
				{
					s_uy = 0; s_dy = sy-1;
					t_uy = v; t_dy = sy-1 + v;
				}
				else if(v>=ty)
				{
					s_uy = 0; s_dy = sy_pad-1 - v;
					t_uy = v; t_dy = sy_pad-1;
				}
				else
					printf("y direction %ld %ld %ld %ld \n", v, sy, ty, sy_pad);
				
				if(w<sz && w<tz)
				{
					s_fz = sz-1 - w; s_bz = sz-1;
					t_fz = sz-1; t_bz = sz-1 + w;
				}
				else if(w<sz && w>=tz)
				{
					s_fz = sz-1 - w; s_bz = sz_pad-1 - w;
					t_fz = sz-1; t_bz = sz_pad-1;
				}
				else if(w<tz && w>=sz)
				{
					s_fz = 0; s_bz = sz-1;
					t_fz = w; t_bz = sz-1 + w;
				}
				else if(w>=tz)
				{
					s_fz = 0; s_bz = sz_pad-1 - w;
					t_fz = w; t_bz = sz_pad-1;
				}
				else
					printf("z direction %ld %ld %ld %ld \n", w, sz, tz, sz_pad);
				
				
				if(t_rx<t_lx || t_dy<t_uy || t_bz<t_fz || s_rx<s_lx || s_dy<s_uy || s_bz<s_fz)
				{
					pOut.pImg[idx] = 0;
					continue;
				}
				
				// overlap boundary
				t_ful_idx = (t_fz-1)*sx_pad_ori*sy_pad + (t_uy-1)*sx_pad_ori + t_lx-1;
				t_fur_idx = (t_fz-1)*sx_pad_ori*sy_pad + (t_uy-1)*sx_pad_ori + t_rx;
				t_fdl_idx = (t_fz-1)*sx_pad_ori*sy_pad + t_dy*sx_pad_ori + t_lx-1;
				t_fdr_idx = (t_fz-1)*sx_pad_ori*sy_pad + t_dy*sx_pad_ori + t_rx;
				t_bul_idx = t_bz*sx_pad_ori*sy_pad + (t_uy-1)*sx_pad_ori + t_lx-1;
				t_bur_idx = t_bz*sx_pad_ori*sy_pad + (t_uy-1)*sx_pad_ori + t_rx;
				t_bdl_idx = t_bz*sx_pad_ori*sy_pad + t_dy*sx_pad_ori + t_lx-1;
				t_bdr_idx = t_bz*sx_pad_ori*sy_pad + t_dy*sx_pad_ori + t_rx;
				
				s_ful_idx = (s_fz-1)*sx_pad_ori*sy_pad + (s_uy-1)*sx_pad_ori + s_lx-1;
				s_fur_idx = (s_fz-1)*sx_pad_ori*sy_pad + (s_uy-1)*sx_pad_ori + s_rx;
				s_fdl_idx = (s_fz-1)*sx_pad_ori*sy_pad + s_dy*sx_pad_ori + s_lx-1;
				s_fdr_idx = (s_fz-1)*sx_pad_ori*sy_pad + s_dy*sx_pad_ori + s_rx;
				s_bul_idx = s_bz*sx_pad_ori*sy_pad + (s_uy-1)*sx_pad_ori + s_lx-1;
				s_bur_idx = s_bz*sx_pad_ori*sy_pad + (s_uy-1)*sx_pad_ori + s_rx;
				s_bdl_idx = s_bz*sx_pad_ori*sy_pad + s_dy*sx_pad_ori + s_lx-1;
				s_bdr_idx = s_bz*sx_pad_ori*sy_pad + s_dy*sx_pad_ori + s_rx;
				
				T2 len_t = (t_bz - t_fz + 1)*(t_dy - t_uy + 1)*(t_rx - t_lx + 1), len_s = (s_bz - s_fz + 1)*(s_dy - s_uy + 1)*(s_rx - s_lx + 1);
				
				// prior knowledge of ratio of overlap
				if(T1(len_s)<T1(sub_sz)*overlap_percent)
				{
					pOut.pImg[idx] = 0;
					continue;
				}
				
				T1 t1,t2;
				
				if(t_lx==0 && t_uy==0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx];
					t2 = ncctar.sum2[t_bdr_idx];
				}
				else if(t_lx>0 && t_uy==0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx];
				}
				else if(t_lx==0 && t_uy>0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx];
				}
				else if(t_lx==0 && t_uy==0 && t_fz>0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_fdr_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_fdr_idx];
				}
				else if(t_lx>0 && t_uy>0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] + ncctar.sum1[t_bul_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] + ncctar.sum2[t_bul_idx];
				}
				else if(t_lx>0 && t_uy==0 && t_fz>0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fdl_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fdl_idx];
				}
				else if(t_lx==0 && t_uy>0 && t_fz>0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fur_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fur_idx];
				}
				else
				{
					t1 = fabs( ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_bul_idx] + ncctar.sum1[t_fdl_idx] + ncctar.sum1[t_fur_idx] - ncctar.sum1[t_ful_idx]);
					t2 = fabs( ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_bul_idx] + ncctar.sum2[t_fdl_idx] + ncctar.sum2[t_fur_idx] - ncctar.sum2[t_ful_idx]);
				}
				
				t_mean = t1/len_t; 
				t_std = sqrt(y_max(t2 - t1*t_mean, T1(0) ));
				
				T1 s1,s2;
				
				if(s_lx==0 && s_uy==0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx];
					s2 = nccsub.sum2[s_bdr_idx];
				}
				else if(s_lx>0 && s_uy==0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx];
				}
				else if(s_lx==0 && s_uy>0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx];
				}
				else if(s_lx==0 && s_uy==0 && s_fz>0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_fdr_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_fdr_idx];
				}
				else if(s_lx>0 && s_uy>0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] + nccsub.sum1[s_bul_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] + nccsub.sum2[s_bul_idx];
				}
				else if(s_lx>0 && s_uy==0 && s_fz>0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fdl_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fdl_idx];
				}
				else if(s_lx==0 && s_uy>0 && s_fz>0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fur_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fur_idx];
				}
				else
				{
					s1 = fabs( nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_bul_idx] + nccsub.sum1[s_fdl_idx] + nccsub.sum1[s_fur_idx] - nccsub.sum1[s_ful_idx]);
					s2 = fabs( nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_bul_idx] + nccsub.sum2[s_fdl_idx] + nccsub.sum2[s_fur_idx] - nccsub.sum2[s_ful_idx]);
				}
				
				s_mean = s1/len_s;
				s_std = sqrt(y_max(s2 - s1*s_mean, T1(0) ));
				
				// response [0, 1] (instead of [-1, 1])
				if(t_std!=0 && s_std!=0)
				{
					pOut.pImg[idx] = 0.5 + 0.5*(pOut.pImg[idx] - t1*s_mean)/(t_std*s_std);
				}
				else 
				{
					pOut.pImg[idx] = 0;
				}
				
			}			
		}	
	}
	
}

// fft ncc ND
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftnccND(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sznd, T1 overlap_percent)
{
}

// func pc & cc
// only support fft_in_place now
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftpccc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, PEAKSLIST *peakList)
{
	// peaks
	PEAKS pos;
	
	T1 max_v=0;
	
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	T1* pTmpOut = 0; 
	try
	{
		pTmpOut = new T1 [len_pad];
	}
	catch (...)
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	
	//
	//fftwf_init_threads(); //pthread
	//fftwf_plan_with_nthreads(PROCESSORS); 
	
	fftwf_plan p;
	
	if(fftwf_in_place)
	{
		T2 sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
		T2 len_pad_tmp = sx_pad_ori*sy_pad*sz_pad;
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad_ori, pIn.pImg, (fftwf_complex*)pIn.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad_ori, pOut.pImg, (fftwf_complex*)pOut.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		for(T2 i=0; i<len_pad; i++)
		{
			pTmpOut[i] = pOut.pImg[i];
		}
		
		// compute pc & cc
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i+=2)
				{
					T2 idx = offset_j + i;
					
					T1 tmp = pOut.pImg[idx];
					
					// obtain cc
					pOut.pImg[idx+1] = -pOut.pImg[idx+1]; //conjugate
					
					pOut.pImg[idx] = pIn.pImg[idx]*tmp - pIn.pImg[idx+1]*pOut.pImg[idx+1];
					pOut.pImg[idx+1] = pIn.pImg[idx+1]*tmp + pIn.pImg[idx]*pOut.pImg[idx+1];
					
					// obtain the cross power spectrum
					pTmpOut[idx] = pOut.pImg[idx];
					pTmpOut[idx+1] = pOut.pImg[idx+1];
					
					T1 tmp2 = sqrt(pTmpOut[idx]*pTmpOut[idx] + pTmpOut[idx+1]*pTmpOut[idx+1]);
					
					pTmpOut[idx] /= tmp2;
					pTmpOut[idx+1] /= tmp2;

				}
			}
		}
		
		// obtain the phase correlation
		fftwf_complex* out_pc_sub = (fftwf_complex*)pTmpOut;
		
		p = fftwf_plan_dft_c2r_3d(sz_pad, sy_pad, sx_pad_ori, out_pc_sub, pTmpOut, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		// obtain cross-correlation
		fftwf_complex* out_sub = (fftwf_complex*)pOut.pImg;
		
		p = fftwf_plan_dft_c2r_3d(sz_pad, sy_pad, sx_pad_ori, out_sub, pOut.pImg, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		fftwf_destroy_plan(p);
		
		// cleanup
		// fftwf_cleanup_threads();
		
		// normalize
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i++)
				{
					T2 idx = offset_j + i;
					
					if(i<sx_pad_ori)
					{
						pOut.pImg[idx] /= (T1)len_pad_tmp; //
						pTmpOut[idx] /= (T1)len_pad_tmp;
					}
					else
					{
						pOut.pImg[idx] = 0;
						pTmpOut[idx] = 0;
					}
					
					// statistics of peaks
					if(pTmpOut[idx] >= max_v)
					{
						
						pos.x = i; pos.y = j; pos.z = k; pos.value = pTmpOut[idx];
						
						if(peakList->size()<1)
							peakList->push_back(pos);
						else
						{
							for(unsigned int it=peakList->size(); it>0; it--)
							{
								if(pos.value<=peakList->at(it-1).value)
								{
									peakList->insert(peakList->begin() + it, 1, pos);
									
									if(peakList->size()>NPEAKS)
										peakList->erase(peakList->end()-1);
									
									break;
								}
								else
									continue;
								
							}
							if(pos.value>peakList->at(0).value)
								peakList->insert(peakList->begin(), pos);
							
							if(peakList->size()>NPEAKS)
								peakList->erase(peakList->end()-1);
							
						}
						
						if(peakList->size()<NPEAKS)
							max_v = 0;
						else
							max_v = peakList->at(NPEAKS-1).value;
					}
					
				}
			}
		}
		
	}
	
	qDebug() << "peaks number ..." << peakList->size() << " max val ..." << max_v;
	qDebug() << "peaklist ..." << peakList->at(0).value << peakList->at(1).value << peakList->at(2).value << peakList->at(3).value << peakList->at(4).value << peakList->at(5).value << peakList->at(6).value << peakList->at(7).value;
	
	//de-alloc
	if(pTmpOut) {delete []pTmpOut; pTmpOut=0;}
	
}

//func fft phase-correlation combined with normalized cross-correlation using 1d fft
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftpatientpccc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 even_odd, bool fftwf_in_place, PEAKSLIST *peakList)
{
	// peaks
	PEAKS pos;
	
	T1 max_v=0;
	
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	T1* pTmpOut = 0; 
	try
	{
		pTmpOut = new T1 [len_pad];
	}
	catch (...)
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	
	//
	//fftwf_init_threads(); //pthread
	//fftwf_plan_with_nthreads(PROCESSORS); 
	
	fftwf_plan p;
	
	T1 *pIm;
	fftwf_complex* pComplex;
	
	if(fftwf_in_place)
	{
		T2 sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
		T2 len_pad_tmp = sx_pad_ori*sy_pad*sz_pad;
		
		pIm = pIn.pImg;
		pComplex = (fftwf_complex*)pIn.pImg;
		
		p = fftwf_plan_dft_r2c_3d(sz_pad, sy_pad, sx_pad_ori, pIm, pComplex, FFTW_ESTIMATE);
		
		fftwf_execute(p);

		pIm = pOut.pImg;
		pComplex = (fftwf_complex*)pOut.pImg;
		
		fftwf_execute(p);
		
		// de-alloc
		fftwf_destroy_plan(p);
		
		for(T2 i=0; i<len_pad; i++)
		{
			pTmpOut[i] = pOut.pImg[i];
		}
		
		// compute pc & cc
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i+=2)
				{
					T2 idx = offset_j + i;
					
					T1 tmp = pOut.pImg[idx];
					
					// obtain cc
					pOut.pImg[idx+1] = -pOut.pImg[idx+1]; //conjugate
					
					pOut.pImg[idx] = pIn.pImg[idx]*tmp - pIn.pImg[idx+1]*pOut.pImg[idx+1];
					pOut.pImg[idx+1] = pIn.pImg[idx+1]*tmp + pIn.pImg[idx]*pOut.pImg[idx+1];
					
					// obtain the cross power spectrum
					pTmpOut[idx] = pOut.pImg[idx];
					pTmpOut[idx+1] = pOut.pImg[idx+1];
					
					T1 tmp2 = sqrt(power(pTmpOut[idx], 2) + power(pTmpOut[idx+1], 2));
					
					pTmpOut[idx] /= tmp2;
					pTmpOut[idx+1] /= tmp2;
					
				}
			}
		}
		
		// obtain the phase correlation
		pIm = pTmpOut;
		pComplex = (fftwf_complex*)pTmpOut;
		
		p = fftwf_plan_dft_c2r_3d(sz_pad, sy_pad, sx_pad_ori, pComplex, pIm, FFTW_ESTIMATE);
		
		fftwf_execute(p);
		
		// obtain cross-correlation
		pIm = pOut.pImg;
		pComplex = (fftwf_complex*)pOut.pImg;
		
		fftwf_execute(p);
		
		// de-alloc
		fftwf_destroy_plan(p);
		
		// normalize
		for(T2 k=0; k<sz_pad; k++)
		{
			T2 offset_k = k*sy_pad*sx_pad;
			for(T2 j=0; j<sy_pad; j++)
			{
				T2 offset_j = offset_k + j*sx_pad;
				for(T2 i=0; i<sx_pad; i++)
				{
					T2 idx = offset_j + i;
					
					if(i<sx_pad_ori)
					{
						pOut.pImg[idx] /= (T1)len_pad_tmp; //
						pTmpOut[idx] /= (T1)len_pad_tmp;
					}
					else
					{
						pOut.pImg[idx] = 0;
						pTmpOut[idx] = 0;
					}
					
					// statistics of peaks
					if(pTmpOut[idx] > max_v)
					{
						max_v = pTmpOut[idx];
						
						pos.x = i; pos.y = j; pos.z = k; pos.value = max_v;
						
						if(peakList->size()<1)
							peakList->push_back(pos);
						else
						{
							for(unsigned int it=peakList->size(); it!=0; it--)
							{
								if(pos.value>=peakList->at(it-1).value)
								{
									peakList->insert(peakList->begin() + it, 1, pos);
									
									if(peakList->size()>NPEAKS)
										peakList->erase(peakList->begin());
									
									break;
								}
								else
									continue;
								
							}
							if(pos.value<peakList->at(0).value)
								peakList->insert(peakList->begin(), pos);
							
							if(peakList->size()>NPEAKS)
								peakList->erase(peakList->begin());
						}
					}
					
				}
			}
		}
		
	}
	
	
	//de-alloc
	if(pTmpOut) {delete []pTmpOut; pTmpOut=0;}
	
	// cleanup
	//fftwf_cleanup_threads();
}


// fft pc-ncc 3D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftpcncc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz3d, T2 even_odd, bool fftwf_in_place, T1 overlap_percent, PEAKS *pos)
{
	
	T2 tx=sz3d[0], ty=sz3d[1], tz=sz3d[2];
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	T2 sx, sy, sz;
	
	T2 sx_pad_ori;
	
	if(fftwf_in_place)
		sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
	else
		sx_pad_ori = sx_pad;
	
	sx=sx_pad_ori+1-tx; // fftwf_in_place
	sy=sy_pad+1-ty;
	sz=sz_pad+1-tz;
	
	T2 sub_sz = sx*sy*sz;
	
	int start_t = clock();
	
	NST<T1, T2> ncctar(pIn.sz, pIn.pImg, even_odd, fftwf_in_place, 3);
	NST<T1, T2> nccsub(pOut.sz, pOut.pImg, even_odd, fftwf_in_place, 3);
	
	int end_t = clock();
	
	qDebug() << "time consumed ... " << end_t - start_t;
	
//	//phase-correlation
//	T1* pTmpOut = 0, *pTmpIn = 0; 
//	try
//	{
//		pTmpOut = new T1 [len_pad];
//		pTmpIn = new T1 [len_pad];
//	}
//	catch (...)
//	{
//		printf("Fail to allocate memory.\n");
//		return;
//	}
//	
//	for(T2 i=0; i<len_pad; i++)
//	{
//		pTmpOut[i] = pOut.pImg[i];
//		pTmpIn[i] = pIn.pImg[i];
//	}
//	
//	Y_IMG_REAL pYIMOut(pTmpOut, pOut.sz);
//	Y_IMG_REAL pYIMIn(pTmpIn, pIn.sz);
//	
//	PEAKSLIST peakList;
//	fftpc3D(pYIMOut, pYIMIn, even_odd, fftwf_in_place, &peakList);
//	
//	if(pTmpOut) {delete []pTmpOut; pTmpOut=0;}
//	if(pTmpIn) {delete []pTmpIn; pTmpIn=0;}
//	
//	
//	//cross-correlation
//	fftcc3D(pOut, pIn, even_odd, fftwf_in_place);

	
	// pc and cc
	PEAKSLIST peakList;
	fftpccc3D(pOut, pIn, even_odd, fftwf_in_place, &peakList);
	//fftpatientpccc3D(pOut, pIn, even_odd, fftwf_in_place, &peakList);
	
	int end_tt = clock();
	
	qDebug() << "pccc time consumed ... " << end_tt - end_t;
	
	qDebug() << "how many peaks found ..." << peakList.size();
	
//	// for test
//	qDebug() << "pOut ..." << pOut.pImg[33486237] << 33486237;
	
	qDebug() <<"sz ..."<<sx_pad*sy_pad*sz_pad<<sx_pad_ori*sy_pad*sz_pad << sx_pad_ori<<sy_pad<<sz_pad<<sx<<sy<<sz<<tx<<ty<<tz;
	
	//ncc
	pos->value = 0;
	
	//
	if(peakList.size() > 0)
	{
		
		for(T2 ii=peakList.size()-1; ii!=0; ii--)
		{
			T2 pos_x = peakList.at(ii).x;
			T2 pos_y = peakList.at(ii).y;
			T2 pos_z = peakList.at(ii).z;
			
			qDebug() << "peaks ..." << pos_x << pos_y << pos_z; 
			
			// bounding box
			T2 nbb = 3;
			T2 w_start = pos_z - nbb; if(w_start<0) w_start = 0;
			T2 w_end = pos_z + nbb; if(w_end>sz_pad) w_end = sz_pad;
			T2 v_start = pos_y - nbb; if(v_start<0) v_start = 0;
			T2 v_end = pos_y + nbb; if(v_end>sy_pad) v_end = sy_pad;
			T2 u_start = pos_x - nbb; if(u_start<0) u_start = 0;
			T2 u_end = pos_x + nbb; if(u_end>sx_pad) u_end = sx_pad;
			
			// computing ncc
			T2 offset_z = sx_pad_ori*sy_pad;
			
			for(T2 w=w_start; w<w_end; w++)
			{
				T2 offset_w = w*sx_pad*sy_pad;
				for(T2 v=v_start; v<v_end; v++)
				{
					T2 offset_v = offset_w + v*sx_pad;
					for(T2 u=u_start; u<u_end; u++) // fftwf_in_place
					{
						T2 idx = offset_v + u; 
						
						// z - (front, back) y - (upper, down) x - (left, right)
						T2 t_ful_idx, t_fur_idx, t_fdl_idx, t_fdr_idx, t_bul_idx, t_bur_idx, t_bdl_idx, t_bdr_idx;
						T2 s_ful_idx, s_fur_idx, s_fdl_idx, s_fdr_idx, s_bul_idx, s_bur_idx, s_bdl_idx, s_bdr_idx;
						
						T2 t_lx, t_rx, t_uy, t_dy, t_fz, t_bz;
						T2 s_lx, s_rx, s_uy, s_dy, s_fz, s_bz;
						
						T1 t_std, t_mean;
						T1 s_std, s_mean;
						
						if(u<sx && u<tx)
						{
							s_lx = sx-1 - u; s_rx = sx-1;
							t_lx = sx-1; t_rx = sx-1 + u;
						}
						else if(u<sx && u>=tx)
						{
							s_lx = sx-1 - u; s_rx = sx_pad_ori-1 - u;
							t_lx = sx-1; t_rx = sx_pad_ori-1;
						}
						else if(u>=sx && u<tx)
						{
							s_lx = 0; s_rx = sx-1;
							t_lx = u; t_rx = sx-1 + u;
						}
						else if(u>=tx)
						{
							s_lx = 0; s_rx = sx_pad_ori-1 - u;
							t_lx = u; t_rx = sx_pad_ori-1;
						}
						else
							printf("x direction %ld %ld %ld %ld \n", u, sx, tx, sx_pad);
						
						if(v<sy && v<ty)
						{
							s_uy = sy-1 - v; s_dy = sy-1;
							t_uy = sy-1; t_dy = sy-1 + v;
						}
						else if(v<sy && v>=ty)
						{
							s_uy = sy-1 - v; s_dy = sy_pad-1 - v;
							t_uy = sy-1; t_dy = sy_pad-1;
						}
						else if(v<ty && v>=sy)
						{
							s_uy = 0; s_dy = sy-1;
							t_uy = v; t_dy = sy-1 + v;
						}
						else if(v>=ty)
						{
							s_uy = 0; s_dy = sy_pad-1 - v;
							t_uy = v; t_dy = sy_pad-1;
						}
						else
							printf("y direction %ld %ld %ld %ld \n", v, sy, ty, sy_pad);
						
						if(w<sz && w<tz)
						{
							s_fz = sz-1 - w; s_bz = sz-1;
							t_fz = sz-1; t_bz = sz-1 + w;
						}
						else if(w<sz && w>=tz)
						{
							s_fz = sz-1 - w; s_bz = sz_pad-1 - w;
							t_fz = sz-1; t_bz = sz_pad-1;
						}
						else if(w<tz && w>=sz)
						{
							s_fz = 0; s_bz = sz-1;
							t_fz = w; t_bz = sz-1 + w;
						}
						else if(w>=tz)
						{
							s_fz = 0; s_bz = sz_pad-1 - w;
							t_fz = w; t_bz = sz_pad-1;
						}
						else
							printf("z direction %ld %ld %ld %ld \n", w, sz, tz, sz_pad);
						
						if(t_rx<t_lx || t_dy<t_uy || t_bz<t_fz || s_rx<s_lx || s_dy<s_uy || s_bz<s_fz)
						{
							pOut.pImg[idx] = 0;
							continue;
						}
						
						// overlap boundary
						t_ful_idx = (t_fz-1)*offset_z + (t_uy-1)*sx_pad_ori + t_lx-1;
						t_fur_idx = (t_fz-1)*offset_z + (t_uy-1)*sx_pad_ori + t_rx;
						t_fdl_idx = (t_fz-1)*offset_z + t_dy*sx_pad_ori + t_lx-1;
						t_fdr_idx = (t_fz-1)*offset_z + t_dy*sx_pad_ori + t_rx;
						t_bul_idx = t_bz*offset_z + (t_uy-1)*sx_pad_ori + t_lx-1;
						t_bur_idx = t_bz*offset_z + (t_uy-1)*sx_pad_ori + t_rx;
						t_bdl_idx = t_bz*offset_z + t_dy*sx_pad_ori + t_lx-1;
						t_bdr_idx = t_bz*offset_z + t_dy*sx_pad_ori + t_rx;
						
						s_ful_idx = (s_fz-1)*offset_z + (s_uy-1)*sx_pad_ori + s_lx-1;
						s_fur_idx = (s_fz-1)*offset_z + (s_uy-1)*sx_pad_ori + s_rx;
						s_fdl_idx = (s_fz-1)*offset_z + s_dy*sx_pad_ori + s_lx-1;
						s_fdr_idx = (s_fz-1)*offset_z + s_dy*sx_pad_ori + s_rx;
						s_bul_idx = s_bz*offset_z + (s_uy-1)*sx_pad_ori + s_lx-1;
						s_bur_idx = s_bz*offset_z + (s_uy-1)*sx_pad_ori + s_rx;
						s_bdl_idx = s_bz*offset_z + s_dy*sx_pad_ori + s_lx-1;
						s_bdr_idx = s_bz*offset_z + s_dy*sx_pad_ori + s_rx;
						
						T2 len_t = (t_bz - t_fz + 1)*(t_dy - t_uy + 1)*(t_rx - t_lx + 1), len_s = (s_bz - s_fz + 1)*(s_dy - s_uy + 1)*(s_rx - s_lx + 1);
						
						// prior knowledge of ratio of overlap
						if(T1(len_s)<T1(sub_sz)*overlap_percent)
						{
							pOut.pImg[idx] = 0;
							continue;
						}
						
						T1 t1,t2;
						
						if(t_lx==0 && t_uy==0 && t_fz==0)
						{
							t1 = ncctar.sum1[t_bdr_idx];
							t2 = ncctar.sum2[t_bdr_idx];
						}
						else if(t_lx>0 && t_uy==0 && t_fz==0)
						{
							t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx];
							t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx];
						}
						else if(t_lx==0 && t_uy>0 && t_fz==0)
						{
							t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx];
							t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx];
						}
						else if(t_lx==0 && t_uy==0 && t_fz>0)
						{
							t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_fdr_idx];
							t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_fdr_idx];
						}
						else if(t_lx>0 && t_uy>0 && t_fz==0)
						{
							t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] + ncctar.sum1[t_bul_idx];
							t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] + ncctar.sum2[t_bul_idx];
						}
						else if(t_lx>0 && t_uy==0 && t_fz>0)
						{
							t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fdl_idx];
							t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fdl_idx];
						}
						else if(t_lx==0 && t_uy>0 && t_fz>0)
						{
							t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fur_idx];
							t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fur_idx];
						}
						else
						{
							t1 = fabs( ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_bul_idx] + ncctar.sum1[t_fdl_idx] + ncctar.sum1[t_fur_idx] - ncctar.sum1[t_ful_idx]);
							t2 = fabs( ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_bul_idx] + ncctar.sum2[t_fdl_idx] + ncctar.sum2[t_fur_idx] - ncctar.sum2[t_ful_idx]);
						}
						
						t_mean = t1/len_t; 
						t_std = sqrt(y_max(t2 - t1*t_mean, T1(0) ));
						
						T1 s1,s2;
						
						if(s_lx==0 && s_uy==0 && s_fz==0)
						{
							s1 = nccsub.sum1[s_bdr_idx];
							s2 = nccsub.sum2[s_bdr_idx];
						}
						else if(s_lx>0 && s_uy==0 && s_fz==0)
						{
							s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx];
							s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx];
						}
						else if(s_lx==0 && s_uy>0 && s_fz==0)
						{
							s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx];
							s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx];
						}
						else if(s_lx==0 && s_uy==0 && s_fz>0)
						{
							s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_fdr_idx];
							s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_fdr_idx];
						}
						else if(s_lx>0 && s_uy>0 && s_fz==0)
						{
							s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] + nccsub.sum1[s_bul_idx];
							s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] + nccsub.sum2[s_bul_idx];
						}
						else if(s_lx>0 && s_uy==0 && s_fz>0)
						{
							s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fdl_idx];
							s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fdl_idx];
						}
						else if(s_lx==0 && s_uy>0 && s_fz>0)
						{
							s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fur_idx];
							s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fur_idx];
						}
						else
						{
							s1 = fabs( nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_bul_idx] + nccsub.sum1[s_fdl_idx] + nccsub.sum1[s_fur_idx] - nccsub.sum1[s_ful_idx]);
							s2 = fabs( nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_bul_idx] + nccsub.sum2[s_fdl_idx] + nccsub.sum2[s_fur_idx] - nccsub.sum2[s_ful_idx]);
						}
						
						s_mean = s1/len_s;
						s_std = sqrt(y_max(s2 - s1*s_mean, T1(0) ));
						
						// response [0, 1] (instead of [-1, 1])
						if(t_std!=0 && s_std!=0)
						{
							T1 tmp = pOut.pImg[idx];
							
							tmp = 0.5 + 0.5*(tmp - t1*s_mean)/(t_std*s_std);
							
							if(tmp>pos->value)
							{	
								pos->value = tmp;
								pos->x = u; pos->y = v; pos->z = w;
							}
						}
						
					}			
				}	
			}
		
		}
		
	}
	else
	{
		cout << "Fail of finding phase-correlation peaks" << endl;
		pos->value = -1;
		return;
	}

	
}

// fft ncc 3d in finer scale with one peak from coarser scale 
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftnccp3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_sub, T2 *sz_tar, T2 even_odd, bool fftwf_in_place, T1 *scale, PEAKS *pos)
{
		
	T2 tx=sz_tar[0], ty=sz_tar[1], tz=sz_tar[2];
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	T2 sx, sy, sz;
	
	T2 sx_pad_ori;
	
	if(fftwf_in_place)
		sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
	else
		sx_pad_ori = sx_pad;
	
	sx=sx_pad_ori+1-tx; // fftwf_in_place
	sy=sy_pad+1-ty;
	sz=sz_pad+1-tz;
	
	T2 sub_sz = sx*sy*sz;
	
	NST<T1, T2> ncctar(pIn.sz, pIn.pImg, even_odd, fftwf_in_place, 3);
	NST<T1, T2> nccsub(pOut.sz, pOut.pImg, even_odd, fftwf_in_place, 3);
	
	//cross-correlation
	fftcc3D(pOut, pIn, even_odd, fftwf_in_place);
	
	T2 nbbx, nbby, nbbz;
	
	nbbx = 1/scale[0] + 2;
	nbby = 1/scale[1] + 2;
	nbbz = 1/scale[2] + 2;
	
	//ncc
	pos->value = 0;
	
	//
	T2 pos_x = pos->x;
	T2 pos_y = pos->y;
	T2 pos_z = pos->z;
	
	// bounding box
	T2 w_start = pos_z - nbbz; if(w_start<0) w_start = 0;
	T2 w_end = pos_z + nbbz; if(w_end>sz_pad) w_end = sz_pad;
	T2 v_start = pos_y - nbby; if(v_start<0) v_start = 0;
	T2 v_end = pos_y + nbby; if(v_end>sy_pad) v_end = sy_pad;
	T2 u_start = pos_x - nbbx; if(u_start<0) u_start = 0;
	T2 u_end = pos_x + nbbx; if(u_end>sx_pad_ori) u_end = sx_pad_ori;

	qDebug()<< "bb... "<< nbbx << nbby << nbbz << w_start << w_end << v_start << v_end << u_start << u_end;
	
	// computing ncc
	T2 offset_z = sx_pad_ori*sy_pad;
	T2 offset_y = sx_pad_ori;
	
	for(T2 w=w_start; w<w_end; w++)
	{
		T2 offset_w = w*sx_pad*sy_pad;
		for(T2 v=v_start; v<v_end; v++)
		{
			T2 offset_v = offset_w + v*sx_pad;
			for(T2 u=u_start; u<u_end; u++) // fftwf_in_place
			{
				T2 idx = offset_v + u; 
				
				// z - (front, back) y - (upper, down) x - (left, right)
				T2 t_ful_idx, t_fur_idx, t_fdl_idx, t_fdr_idx, t_bul_idx, t_bur_idx, t_bdl_idx, t_bdr_idx;
				T2 s_ful_idx, s_fur_idx, s_fdl_idx, s_fdr_idx, s_bul_idx, s_bur_idx, s_bdl_idx, s_bdr_idx;
				
				T2 t_lx, t_rx, t_uy, t_dy, t_fz, t_bz;
				T2 s_lx, s_rx, s_uy, s_dy, s_fz, s_bz;
				
				T1 t_std, t_mean;
				T1 s_std, s_mean;
				
				if(u<sx && u<tx)
				{
					s_lx = sx-1 - u; s_rx = sx-1;
					t_lx = sx-1; t_rx = sx-1 + u;
				}
				else if(u<sx && u>=tx)
				{
					s_lx = sx-1 - u; s_rx = sx_pad_ori-1 - u;
					t_lx = sx-1; t_rx = sx_pad_ori-1;
				}
				else if(u>=sx && u<tx)
				{
					s_lx = 0; s_rx = sx-1;
					t_lx = u; t_rx = sx-1 + u;
				}
				else if(u>=tx)
				{
					s_lx = 0; s_rx = sx_pad_ori-1 - u;
					t_lx = u; t_rx = sx_pad_ori-1;
				}
				else
					printf("x direction %ld %ld %ld %ld \n", u, sx, tx, sx_pad);
				
				if(v<sy && v<ty)
				{
					s_uy = sy-1 - v; s_dy = sy-1;
					t_uy = sy-1; t_dy = sy-1 + v;
				}
				else if(v<sy && v>=ty)
				{
					s_uy = sy-1 - v; s_dy = sy_pad-1 - v;
					t_uy = sy-1; t_dy = sy_pad-1;
				}
				else if(v<ty && v>=sy)
				{
					s_uy = 0; s_dy = sy-1;
					t_uy = v; t_dy = sy-1 + v;
				}
				else if(v>=ty)
				{
					s_uy = 0; s_dy = sy_pad-1 - v;
					t_uy = v; t_dy = sy_pad-1;
				}
				else
					printf("y direction %ld %ld %ld %ld \n", v, sy, ty, sy_pad);
				
				if(w<sz && w<tz)
				{
					s_fz = sz-1 - w; s_bz = sz-1;
					t_fz = sz-1; t_bz = sz-1 + w;
				}
				else if(w<sz && w>=tz)
				{
					s_fz = sz-1 - w; s_bz = sz_pad-1 - w;
					t_fz = sz-1; t_bz = sz_pad-1;
				}
				else if(w<tz && w>=sz)
				{
					s_fz = 0; s_bz = sz-1;
					t_fz = w; t_bz = sz-1 + w;
				}
				else if(w>=tz)
				{
					s_fz = 0; s_bz = sz_pad-1 - w;
					t_fz = w; t_bz = sz_pad-1;
				}
				else
					printf("z direction %ld %ld %ld %ld \n", w, sz, tz, sz_pad);
				
				
				if(t_rx<t_lx || t_dy<t_uy || t_bz<t_fz || s_rx<s_lx || s_dy<s_uy || s_bz<s_fz)
				{
					pOut.pImg[idx] = 0;
					continue;
				}
				
				// overlap boundary
				t_ful_idx = (t_fz-1)*offset_z + (t_uy-1)*sx_pad_ori + t_lx-1;
				t_fur_idx = (t_fz-1)*offset_z + (t_uy-1)*sx_pad_ori + t_rx;
				t_fdl_idx = (t_fz-1)*offset_z + t_dy*sx_pad_ori + t_lx-1;
				t_fdr_idx = (t_fz-1)*offset_z + t_dy*sx_pad_ori + t_rx;
				t_bul_idx = t_bz*offset_z + (t_uy-1)*sx_pad_ori + t_lx-1;
				t_bur_idx = t_bz*offset_z + (t_uy-1)*sx_pad_ori + t_rx;
				t_bdl_idx = t_bz*offset_z + t_dy*sx_pad_ori + t_lx-1;
				t_bdr_idx = t_bz*offset_z + t_dy*sx_pad_ori + t_rx;
				
				s_ful_idx = (s_fz-1)*offset_z + (s_uy-1)*sx_pad_ori + s_lx-1;
				s_fur_idx = (s_fz-1)*offset_z + (s_uy-1)*sx_pad_ori + s_rx;
				s_fdl_idx = (s_fz-1)*offset_z + s_dy*sx_pad_ori + s_lx-1;
				s_fdr_idx = (s_fz-1)*offset_z + s_dy*sx_pad_ori + s_rx;
				s_bul_idx = s_bz*offset_z + (s_uy-1)*sx_pad_ori + s_lx-1;
				s_bur_idx = s_bz*offset_z + (s_uy-1)*sx_pad_ori + s_rx;
				s_bdl_idx = s_bz*offset_z + s_dy*sx_pad_ori + s_lx-1;
				s_bdr_idx = s_bz*offset_z + s_dy*sx_pad_ori + s_rx;
				
				T2 len_t = (t_bz - t_fz + 1)*(t_dy - t_uy + 1)*(t_rx - t_lx + 1), len_s = (s_bz - s_fz + 1)*(s_dy - s_uy + 1)*(s_rx - s_lx + 1);
				
				//
				T1 t1,t2;
				
				if(t_lx==0 && t_uy==0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx];
					t2 = ncctar.sum2[t_bdr_idx];
				}
				else if(t_lx>0 && t_uy==0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx];
				}
				else if(t_lx==0 && t_uy>0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx];
				}
				else if(t_lx==0 && t_uy==0 && t_fz>0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_fdr_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_fdr_idx];
				}
				else if(t_lx>0 && t_uy>0 && t_fz==0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] + ncctar.sum1[t_bul_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] + ncctar.sum2[t_bul_idx];
				}
				else if(t_lx>0 && t_uy==0 && t_fz>0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fdl_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fdl_idx];
				}
				else if(t_lx==0 && t_uy>0 && t_fz>0)
				{
					t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fur_idx];
					t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fur_idx];
				}
				else
				{
					t1 = fabs( ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_bul_idx] + ncctar.sum1[t_fdl_idx] + ncctar.sum1[t_fur_idx] - ncctar.sum1[t_ful_idx]);
					t2 = fabs( ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_bul_idx] + ncctar.sum2[t_fdl_idx] + ncctar.sum2[t_fur_idx] - ncctar.sum2[t_ful_idx]);
				}
				
				t_mean = t1/len_t; 
				t_std = sqrt(y_max(t2 - t1*t_mean, T1(0) ));
				
				T1 s1,s2;
				
				if(s_lx==0 && s_uy==0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx];
					s2 = nccsub.sum2[s_bdr_idx];
				}
				else if(s_lx>0 && s_uy==0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx];
				}
				else if(s_lx==0 && s_uy>0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx];
				}
				else if(s_lx==0 && s_uy==0 && s_fz>0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_fdr_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_fdr_idx];
				}
				else if(s_lx>0 && s_uy>0 && s_fz==0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] + nccsub.sum1[s_bul_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] + nccsub.sum2[s_bul_idx];
				}
				else if(s_lx>0 && s_uy==0 && s_fz>0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fdl_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fdl_idx];
				}
				else if(s_lx==0 && s_uy>0 && s_fz>0)
				{
					s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fur_idx];
					s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fur_idx];
				}
				else
				{
					s1 = fabs( nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_bul_idx] + nccsub.sum1[s_fdl_idx] + nccsub.sum1[s_fur_idx] - nccsub.sum1[s_ful_idx]);
					s2 = fabs( nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_bul_idx] + nccsub.sum2[s_fdl_idx] + nccsub.sum2[s_fur_idx] - nccsub.sum2[s_ful_idx]);
				}
				
				s_mean = s1/len_s;
				s_std = sqrt(y_max(s2 - s1*s_mean, T1(0) ));
				
				//qDebug()<< s_mean << s_std << s1 << t_mean << t_std << t1 << u << v << w; ;
				//qDebug() << "test" <<  0.5 + 0.5*(pOut.pImg[idx] - t1*t_mean)/(t_std*t_std) << u << v << w; 
				
				// response [0, 1] (instead of [-1, 1])
				if(t_std!=0 && s_std!=0)
				{
					//qDebug()<< "compute..." << pOut.pImg[idx];
					pOut.pImg[idx] = 0.5 + 0.5*(pOut.pImg[idx] - t1*s_mean)/(t_std*s_std);
					//qDebug()<< "compute...normalized" << pOut.pImg[idx];
					
					if(pOut.pImg[idx]>pos->value)
					{	
						pos->value = pOut.pImg[idx];
						pos->x = u; pos->y = v; pos->z = w;
						//qDebug()<< "improved..."<< pos->value;
					}
				}
				else 
				{
					pOut.pImg[idx] = 0;
				}
				
			}			
		}	
	}
	

}

// second way to compute ncc with only one peak without computing sum table (slow)
// fft ncc 3d in finer scale with one peak from coarser scale 
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: fftnccpns3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_sub, T2 *sz_tar, T2 even_odd, bool fftwf_in_place, T1 *scale, PEAKS *pos)
{
	
	T2 tx=sz_tar[0], ty=sz_tar[1], tz=sz_tar[2];
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	T2 sx, sy, sz;
	
	T2 sx_pad_ori;
	
	if(fftwf_in_place)
		sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
	else
		sx_pad_ori = sx_pad;
	
	sx=sx_pad_ori+1-tx; // fftwf_in_place
	sy=sy_pad+1-ty;
	sz=sz_pad+1-tz;
	
	T2 sub_sz = sx*sy*sz;
	
	//cross-correlation
	fftcc3D(pOut, pIn, even_odd, fftwf_in_place);
	
	T2 nbbx, nbby, nbbz;
	
	nbbx = 2/scale[0];
	nbby = 2/scale[1];
	nbbz = 2/scale[2];
	
	//ncc
	pos->value = 0;
	
	//
	
	T2 pos_x = pos->x;
	T2 pos_y = pos->y;
	T2 pos_z = pos->z;
	
	// bounding box
	T2 w_start = pos_z - nbbx; if(w_start<0) w_start = 0;
	T2 w_end = pos_z + nbbx; if(w_end>sz_pad) w_end = sz_pad;
	T2 v_start = pos_y - nbby; if(v_start<0) v_start = 0;
	T2 v_end = pos_y + nbby; if(v_end>sy_pad) v_end = sy_pad;
	T2 u_start = pos_x - nbbz; if(u_start<0) u_start = 0;
	T2 u_end = pos_x + nbbz; if(u_end>sx_pad_ori) u_end = sx_pad_ori;
	
	qDebug()<< w_start << w_end << v_start << v_end << u_start << u_end;
	
	// computing ncc
	for(T2 w=w_start; w<w_end; w++)
	{
		T2 offset_w = w*sx_pad*sy_pad;
		for(T2 v=v_start; v<v_end; v++)
		{
			T2 offset_v = offset_w + v*sx_pad;
			for(T2 u=u_start; u<u_end; u++) // fftwf_in_place
			{
				T2 idx = offset_v + u; 
				
				T2 t_lx, t_rx, t_uy, t_dy, t_fz, t_bz;
				T2 s_lx, s_rx, s_uy, s_dy, s_fz, s_bz;
				
				T1 t_std, t_mean;
				T1 s_std, s_mean;
				
				if(u<sx && u<tx)
				{
					s_lx = sx-1 - u; s_rx = sx-1;
					t_lx = sx-1; t_rx = sx-1 + u;
				}
				else if(u<sx && u>=tx)
				{
					s_lx = sx-1 - u; s_rx = sx_pad_ori-1 - u;
					t_lx = sx-1; t_rx = sx_pad_ori-1;
				}
				else if(u>=sx && u<tx)
				{
					s_lx = 0; s_rx = sx-1;
					t_lx = u; t_rx = sx-1 + u;
				}
				else if(u>=tx)
				{
					s_lx = 0; s_rx = sx_pad_ori-1 - u;
					t_lx = u; t_rx = sx_pad_ori-1;
				}
				else
					printf("x direction %ld %ld %ld %ld \n", u, sx, tx, sx_pad);
				
				if(v<sy && v<ty)
				{
					s_uy = sy-1 - v; s_dy = sy-1;
					t_uy = sy-1; t_dy = sy-1 + v;
				}
				else if(v<sy && v>=ty)
				{
					s_uy = sy-1 - v; s_dy = sy_pad-1 - v;
					t_uy = sy-1; t_dy = sy_pad-1;
				}
				else if(v<ty && v>=sy)
				{
					s_uy = 0; s_dy = sy-1;
					t_uy = v; t_dy = sy-1 + v;
				}
				else if(v>=ty)
				{
					s_uy = 0; s_dy = sy_pad-1 - v;
					t_uy = v; t_dy = sy_pad-1;
				}
				else
					printf("y direction %ld %ld %ld %ld \n", v, sy, ty, sy_pad);
				
				if(w<sz && w<tz)
				{
					s_fz = sz-1 - w; s_bz = sz-1;
					t_fz = sz-1; t_bz = sz-1 + w;
				}
				else if(w<sz && w>=tz)
				{
					s_fz = sz-1 - w; s_bz = sz_pad-1 - w;
					t_fz = sz-1; t_bz = sz_pad-1;
				}
				else if(w<tz && w>=sz)
				{
					s_fz = 0; s_bz = sz-1;
					t_fz = w; t_bz = sz-1 + w;
				}
				else if(w>=tz)
				{
					s_fz = 0; s_bz = sz_pad-1 - w;
					t_fz = w; t_bz = sz_pad-1;
				}
				else
					printf("z direction %ld %ld %ld %ld \n", w, sz, tz, sz_pad);
				
				
				if(t_rx<t_lx || t_dy<t_uy || t_bz<t_fz || s_rx<s_lx || s_dy<s_uy || s_bz<s_fz)
				{
					pOut.pImg[idx] = 0;
					continue;
				}
				
				// overlap
				T2 len_t = (t_bz - t_fz + 1)*(t_dy - t_uy + 1)*(t_rx - t_lx + 1), len_s = (s_bz - s_fz + 1)*(s_dy - s_uy + 1)*(s_rx - s_lx + 1);
				
				T1 t1,t2;
				
				t1=0; t2=0;
				for(T2 k=t_fz; k<=t_bz; k++)
				{
					T2 offset_k = k*sx_pad_ori*sy_pad;
					for(T2 j=t_uy; j<=t_dy; j++)
					{
						T2 offset_j = offset_k + j*sx_pad_ori;
						for(T2 i=t_lx; i<=t_rx; i++)
						{
							T2 idx = offset_j + i;
							
							T1 tmp = pIn.pImg[idx];
							
							t1 += tmp;
							
							t2 += tmp*tmp;
						}
					}
				}
				
				t_mean = t1/len_t; 
				t_std = sqrt(max(t2 - t1*t_mean, 0));
				
				T1 s1,s2;
				
				s1=0; s2=0;
				for(T2 k=s_fz; k<=s_bz; k++)
				{
					T2 offset_k = k*sx_pad_ori*sy_pad;
					for(T2 j=s_uy; j<=s_dy; j++)
					{
						T2 offset_j = offset_k + j*sx_pad_ori;
						for(T2 i=s_lx; i<=s_rx; i++)
						{
							T2 idx = offset_j + i;
							
							T1 tmp = pOut.pImg[idx];
							
							s1 += tmp;
							
							s2 += tmp*tmp;
						}
					}
				}
				
				s_mean = s1/len_s;
				s_std = sqrt(max(s2 - s1*s_mean, 0));
				
				//qDebug()<< s_mean << s_std << s1 << t_mean << t_std << t1 << u << v << w; ;
				//qDebug() << "test" <<  0.5 + 0.5*(pOut.pImg[idx] - t1*t_mean)/(t_std*t_std) << u << v << w; 
				
				// response [0, 1] (instead of [-1, 1])
				if(t_std!=0 && s_std!=0)
				{
					//qDebug()<< "compute..." << pOut.pImg[idx];
					pOut.pImg[idx] = 0.5 + 0.5*(pOut.pImg[idx] - t1*s_mean)/(t_std*s_std);
					//qDebug()<< "compute...normalized" << pOut.pImg[idx];
					
					if(pOut.pImg[idx]>pos->value)
					{	
						pos->value = pOut.pImg[idx];
						pos->x = u; pos->y = v; pos->z = w;
						//qDebug()<< "improved..."<< pos->value;
					}
				}
				else 
				{
					pOut.pImg[idx] = 0;
				}
				
			}			
		}	
	}
	
	
}

// func region growing 3D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: regiongrow3D(Y_IMG1 pOut, Y_IMG2 pIn, rPEAKSLIST *peakList, T2 ncm)
{
	//
	T2 sx, sy, sz;
	
	sx=pIn.sz[0];
	sy=pIn.sz[1];
	sz=pIn.sz[2];
	
	T2 pagesz = sx*sy*sz;
	
	T1 meanv=0, stdv=0;
	
	unsigned char *bw = 0;
	
	try
	{
		bw = new unsigned char [pagesz];
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	
	// method 1 ---------------------------------------------
	// binarize image using a chosen threshold a*meanv+b*stdv
//	for(T2 i=0; i<pagesz; i++)
//	{
//		bw[i] = 0;
//		
//		meanv += pIn.pImg[i];
//	}
//	meanv /= pagesz;
//	
//	for(V3DLONG i=0; i<pagesz; i++)
//		stdv += (pIn.pImg[i] - meanv)*(pIn.pImg[i] - meanv);
//	
//	stdv /= (pagesz-1);
//	stdv = sqrt(stdv);
//	
//	qDebug() << "mean value ... " << meanv << "stdv ..." << stdv;
//	
//	T1 threshold = meanv+4*stdv;
//	//
//	for(T2 i=0; i<pagesz; i++)
//	{
//		bw[i] = (pIn.pImg[i]>threshold)?1:0; //
//		pOut.pImg[i] = 0; // init label
//	}
	
	// method 2 ------------------------------------------------
	// binarize image using KMeans K=2
	T2 K=2;
	
	unsigned char *p=pIn.pImg;
	
	T2 *h=0, *hc=0;
	
	try
	{
		h = new T2 [256];
		hc = new T2 [256];
		
		memset(h, 0, 256*sizeof(T2));
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	
	// histogram
	for(T2 i=0; i<pagesz; i++)
	{
		h[*(p++)] ++;
	}
	
	// init center
	T1 mub=80, muf=160;
	
	//
	system("date");
	while (true) 
	{
		T1 oldmub=mub, oldmuf=muf;
		
		for(T2 i=0; i<256; i++)
		{
			if(h[i]==0)
				continue;
			
			T1 cb = fabs(T1(i)-mub);
			T1 cf = fabs(T1(i)-muf);
			
			hc[i] = (cb<=cf)?1:2; // class 1 and class 2
		}
		
		// update centers
		double sum_b=0, sum_bw=0, sum_f=0, sum_fw=0;
		
		for(T2 i=0; i<256; i++)
		{
			if(h[i]==0)
				continue;
			
			if(hc[i]==1)
			{
				sum_bw += (i+1)*h[i]; 
				sum_b += h[i];
			}
			else if(hc[i]==2)
			{
				sum_fw += (i+1)*h[i]; 
				sum_f += h[i];
			}
		}
		
		mub = T1( sum_bw/sum_b );
		muf = T1( sum_fw/sum_f );
		
		if(fabs(mub - oldmub)<1 && fabs(muf - oldmuf)<1)
			break;
		
	}
	system("date");
	
	qDebug()<<"threshold ..."<<mub<<muf;
	
	// binary image: bw
	for(T2 i=0; i<pagesz; i++)
	{
		T1 tmp = pIn.pImg[i];
		
		T1 dist_b = fabs(tmp-mub);
		T1 dist_f = fabs(tmp-muf);
		
		bw[i] = (dist_b>dist_f)?1:0; //
		pOut.pImg[i] = 0; // init label
	}
	
	//de-alloc
	if(h) {delete []h; h=0;}
	if(hc) {delete []hc; hc=0;}
	
	// ----------------------------------------------------------
	// pOut.pImg as a label
	// bwlabeln
	V3DLONG offset_y, offset_z;
	
	offset_y=sx;
	offset_z=sx*sy;
	
	V3DLONG neighborhood_13[13] = {-1, -offset_y, -offset_z,
								-offset_y-1, -offset_y-offset_z, 
								offset_y-1, offset_y-offset_z,
								offset_z-1, -offset_z-1,
								-1-offset_y-offset_z, -1+offset_y-offset_z,
								1-offset_y-offset_z, 1+offset_y-offset_z}; 
	
	// other variables
    int *lset = new int [pagesz];   // label table/tree
    int ntable;                     // number of elements in the component table/tree
    
    ntable = 0;
    lset[0] = 0;
	
	for(V3DLONG k = 0; k < sz; k++) 
	{				
		V3DLONG idxk = k*offset_z;
		for(V3DLONG j = 0;  j < sy; j++) 
		{
			V3DLONG idxj = idxk + j*offset_y;
			
			for(V3DLONG i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				
				if(i==0 || i==sx-1 || j==0 || j==sy-1 || k==0 || k==sz-1)
					continue;
				
				// find connected components
				if(bw[idx]) // if there is an object 
				{
					int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13;
					
					n1 = find(lset, (int)pOut.pImg[idx + neighborhood_13[0] ]);
					n2 = find(lset, (int)pOut.pImg[idx + neighborhood_13[1] ]);
					n3 = find(lset, (int)pOut.pImg[idx + neighborhood_13[2] ]);
					n4 = find(lset, (int)pOut.pImg[idx + neighborhood_13[3] ]);
					n5 = find(lset, (int)pOut.pImg[idx + neighborhood_13[4] ]);
					n6 = find(lset, (int)pOut.pImg[idx + neighborhood_13[5] ]);
					n7 = find(lset, (int)pOut.pImg[idx + neighborhood_13[6] ]);
					n8 = find(lset, (int)pOut.pImg[idx + neighborhood_13[7] ]);
					n9 = find(lset, (int)pOut.pImg[idx + neighborhood_13[8] ]);
					n10 = find(lset, (int)pOut.pImg[idx + neighborhood_13[9] ]);
					n11 = find(lset, (int)pOut.pImg[idx + neighborhood_13[10] ]);
					n12 = find(lset, (int)pOut.pImg[idx + neighborhood_13[11] ]);
					n13 = find(lset, (int)pOut.pImg[idx + neighborhood_13[12] ]);
					
					
					
					if(n1 || n2 || n3 || n4 || n5 || n6 || n7 || n8 || n9 || n10 || n11 || n12 || n13)
					{
						int tlabel;
						
						if(n1) tlabel = n1;
						else if(n2) tlabel = n2;
						else if(n3) tlabel = n3;
						else if(n4) tlabel = n4;
						else if(n5) tlabel = n5;
						else if(n6) tlabel = n6;
						else if(n7) tlabel = n7;
						else if(n8) tlabel = n8;
						else if(n9) tlabel = n9;
						else if(n10) tlabel = n10;
						else if(n11) tlabel = n11;
						else if(n12) tlabel = n12;
						else if(n13) tlabel = n13;
						
						pOut.pImg[idx] = tlabel;
						
						if(n1 && n1 != tlabel) lset[n1] = tlabel;
						if(n2 && n2 != tlabel) lset[n2] = tlabel;
						if(n3 && n3 != tlabel) lset[n3] = tlabel;
						if(n4 && n4 != tlabel) lset[n4] = tlabel;
						if(n5 && n5 != tlabel) lset[n5] = tlabel;
						if(n6 && n6 != tlabel) lset[n6] = tlabel;
						if(n7 && n7 != tlabel) lset[n7] = tlabel;
						if(n8 && n8 != tlabel) lset[n8] = tlabel;
						if(n9 && n9 != tlabel) lset[n9] = tlabel;
						if(n10 && n10 != tlabel) lset[n10] = tlabel;
						if(n11 && n11 != tlabel) lset[n11] = tlabel;
						if(n12 && n12 != tlabel) lset[n12] = tlabel;
						if(n13 && n13 != tlabel) lset[n13] = tlabel;
						
					}
					else
					{
						ntable++;
						pOut.pImg[idx] = lset[ntable] = ntable;
					}
					
				}
				else
				{
					pOut.pImg[idx] = NO_OBJECT;
					
				}
				
			}
		}
	}
	
	// consolidate component table
    for( int i = 0; i <= ntable; i++ )
        lset[i] = find( lset, i );
	
    // run image through the look-up table
   	for(V3DLONG k = 0; k < sz; k++) 
	{				
		V3DLONG idxk = k*offset_z;
		for(V3DLONG j = 0;  j < sy; j++) 
		{
			V3DLONG idxj = idxk + j*offset_y;
			
			for(V3DLONG i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				pOut.pImg[idx] = lset[ (int)pOut.pImg[idx] ];
			}
		}
	}
    
    // count up the objects in the image
    for( int i = 0; i <= ntable; i++ )
        lset[i] = 0;
	
   	for(V3DLONG k = 0; k < sz; k++) 
	{				
		V3DLONG idxk = k*offset_z;
		for(V3DLONG j = 0;  j < sy; j++) 
		{
			V3DLONG idxj = idxk + j*offset_y;
			
			for(V3DLONG i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				lset[ (int)pOut.pImg[idx] ]++;
			}
		}
	}
	
    // number the objects from 1 through n objects
    int nobj = 0;
    lset[0] = 0;
    for( int i = 1; i <= ntable; i++ )
        if ( lset[i] > 0 )
            lset[i] = ++nobj;
	
	qDebug() << "how many objects found ..." << nobj;
	
	std::set<int> brgnset;
	std::set<int>::iterator brgnset_it;
	
    // run through the look-up table again
   	for(V3DLONG k = 0; k < sz; k++) 
	{				
		V3DLONG idxk = k*offset_z;
		for(V3DLONG j = 0;  j < sy; j++) 
		{
			V3DLONG idxj = idxk + j*offset_y;
			
			for(V3DLONG i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				pOut.pImg[idx] = lset[ (int)pOut.pImg[idx] ];
				
				// record connected to boundary region
				//if(i==1 || i==sx-2 || j==1 || j==sy-2 || k==1 || k==sz-2)
				if(i==0 || i==sx-1 || j==0 || j==sy-1 || k==0 || k==sz-1)
				{
					
					if(brgnset.find(pOut.pImg[idx])==brgnset.end())
					{
						brgnset.insert(pOut.pImg[idx]);
					}
				}
			}
		}
	}
	
//	for(brgnset_it = brgnset.begin(); brgnset_it!=brgnset.end(); brgnset_it++)
//		qDebug() << "set ..." << *brgnset_it << brgnset.size();
	
	
	// find the first N biggest regions	
	std::vector<STCL> labelList;
	
	// histogram of L
	int *a = new int [nobj+1];
	
	for(V3DLONG i=0;  i<=nobj; i++)
	{
		a[i] = 0;
	}
	
	for(V3DLONG i=0; i<pagesz; i++)
	{
		a[ pOut.pImg[i] ] ++;
	}
	
	//
	int np = qMin(int(ncm), int(nobj));
	
	qDebug() << "statistics points ..." << np;
	
	for(int i=1;  i<=nobj; i++) // 0 is background
	{
		
		STCL s;
		
		s.count = a[i];
		s.label = i;
		
		if(brgnset.find(i)!=brgnset.end()) // avoid rgn connected to boundary
			continue;
			
		//
		if(labelList.size()<1)
			labelList.push_back(s);
		else
		{
			for(unsigned int it=labelList.size(); it!=0; it--)
			{
				if(s.count<=labelList.at(it-1).count)
				{
					labelList.insert(labelList.begin() + it, 1, s);
					
					if(labelList.size()>np) // pick np points
						labelList.erase(labelList.end());
					
					break;
				}
				else
					continue;
				
			}
			
			//
			if(s.count>labelList.at(0).count)
				labelList.insert(labelList.begin(), s);
			
			if(labelList.size()>np) // pick np points
				labelList.erase(labelList.end());
		}
		
		
	}
	
	rPEAKS pos;
	
	//LandmarkList cmList; // marker file
	
	np = labelList.size();
	
	for(int i_n = 0; i_n<np; i_n++)
	{
		float scx=0,scy=0,scz=0,si=0;
		
		int label=labelList.at(i_n).label;
		
		for(V3DLONG k = 0; k < sz; k++) 
		{				
			V3DLONG idxk = k*offset_z;
			for(V3DLONG j = 0;  j < sy; j++) 
			{
				V3DLONG idxj = idxk + j*offset_y;
				
				for(V3DLONG i = 0, idx = idxj; i < sx;  i++, idx++) 
				{
					
					//
					if(pOut.pImg[idx]==label)
					{
						float cv = pIn.pImg[ idx ];
						
						scz += k*cv;
						scy += j*cv;
						scx += i*cv;
						si += cv;
					}
					
					
				}
			}
		}
		
		//
		if (si>0)
		{
			T1 ncx = scx/si; // + 0.5; // +1; 
			T1 ncy = scy/si; // + 0.5; // +1; 
			T1 ncz = scz/si; // + 0.5; // +1;
			
			qDebug() << "position ..." << ncx << ncy << ncz << "rgn sz ..." << labelList.at(i_n).count;
			
			//LocationSimple pp(ncx, ncy, ncz);
			//cmList.push_back(pp);
			
			pos.x = ncx; pos.y = ncy; pos.z = ncz;
			peakList->push_back(pos);
			
		}
		
	}
	
	//de-alloc
	if(bw) {delete []bw; bw=0;}
	
}

// func gaussian filtering 3D
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: gaussianfilter3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_w)
{
	// INPUT: pOut.pImg is temporary pointer and real type of unsigned char original data pIn.pImg
	// OUTPUT: pIn.pImg is the finally filtered result
	
	//
	T2 N, M, P;
	
	N=pIn.sz[0];
	M=pIn.sz[1];
	P=pIn.sz[2];
	
	T2 pagesz = N*M*P;

	unsigned int Wx, Wy, Wz;
	
	Wx=sz_w[0];
	Wy=sz_w[1];
	Wz=sz_w[2];
	
	REAL min_val=INF, max_val=0;
	
	//Filtering
	//
	//   Filtering aV3DLONG x
	if(N<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		REAL  *WeightsX = 0;
		WeightsX = new REAL [Wx];
		if (!WeightsX)
			return;
		
		unsigned int Half = Wx >> 1;
		WeightsX[Half] = 1.;
		
		for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
		{
			const REAL  x = 3.* REAL (Weight) / REAL (Half);
			WeightsX[Half - Weight] = WeightsX[Half + Weight] = exp(-x * x / 2.);	// Corresponding symmetric WeightsX
		}
		
		REAL k = 0.;
		for (unsigned int Weight = 0; Weight < Wx; ++Weight)
			k += WeightsX[Weight];
		
		for (unsigned int Weight = 0; Weight < Wx; ++Weight)
			WeightsX[Weight] /= k;
		
		
		//   Allocate 1-D extension array
		REAL  *extension_bufferX = 0;
		extension_bufferX = new REAL [N + (Wx<<1)];
		
		unsigned int offset = Wx>>1;
		
		//	aV3DLONG x
		const REAL  *extStop = extension_bufferX + N + offset;
		
		for(T2 iz = 0; iz < P; iz++)
		{
			for(T2 iy = 0; iy < M; iy++)
			{
				REAL  *extIter = extension_bufferX + Wx;
				for(T2 ix = 0; ix < N; ix++)
				{
					*(extIter++) = pOut.pImg[iz*M*N + iy*N + ix];
				}
				
				//   Extend image
				const REAL  *const stop_line = extension_bufferX - 1;
				REAL  *extLeft = extension_bufferX + Wx - 1;
				const REAL  *arrLeft = extLeft + 2;
				REAL  *extRight = extLeft + N + 1;
				const REAL  *arrRight = extRight - 2;
				
				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}
				
				//	Filtering
				extIter = extension_bufferX + offset;
				
				REAL  *resIter = &(pOut.pImg[iz*M*N + iy*N]);
				
				while (extIter < extStop)
				{
					REAL sum = 0.;
					const REAL  *weightIter = WeightsX;
					const REAL  *const End = WeightsX + Wx;
					const REAL * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * REAL (*(arrIter++));
					extIter++;
					*(resIter++) = sum;
					
					//for rescale
					if(max_val<*arrIter) max_val = *arrIter;
					if(min_val>*arrIter) min_val = *arrIter;
				}
			}
		}
		
		//de-alloc
		if (WeightsX) {delete []WeightsX; WeightsX=0;}
		if (extension_bufferX) {delete []extension_bufferX; extension_bufferX=0;}
		
	}
	
	//   Filtering aV3DLONG y
	if(M<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		REAL  *WeightsY = 0;
		WeightsY = new REAL [Wy];
		if (!WeightsY)
			return;
		
		unsigned int Half = Wy >> 1;
		WeightsY[Half] = 1.;
		
		for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
		{
			const REAL  y = 3.* REAL (Weight) / REAL (Half);
			WeightsY[Half - Weight] = WeightsY[Half + Weight] = exp(-y * y / 2.);	// Corresponding symmetric WeightsY
		}
		
		REAL k = 0.;
		for (unsigned int Weight = 0; Weight < Wy; ++Weight)
			k += WeightsY[Weight];
		
		for (unsigned int Weight = 0; Weight < Wy; ++Weight)
			WeightsY[Weight] /= k;
		
		//	aV3DLONG y
		REAL  *extension_bufferY = 0;
		extension_bufferY = new REAL [M + (Wy<<1)];
		
		unsigned int offset = Wy>>1;
		const REAL *extStop = extension_bufferY + M + offset;
		
		for(T2 iz = 0; iz < P; iz++)
		{
			for(T2 ix = 0; ix < N; ix++)
			{
				REAL  *extIter = extension_bufferY + Wy;
				for(T2 iy = 0; iy < M; iy++)
				{
					*(extIter++) = pOut.pImg[iz*M*N + iy*N + ix];
				}
				
				//   Extend image
				const REAL  *const stop_line = extension_bufferY - 1;
				REAL  *extLeft = extension_bufferY + Wy - 1;
				const REAL  *arrLeft = extLeft + 2;
				REAL  *extRight = extLeft + M + 1;
				const REAL  *arrRight = extRight - 2;
				
				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}
				
				//	Filtering
				extIter = extension_bufferY + offset;
				
				REAL  *resIter = &(pOut.pImg[iz*M*N + ix]);
				
				while (extIter < extStop)
				{
					REAL sum = 0.;
					const REAL  *weightIter = WeightsY;
					const REAL  *const End = WeightsY + Wy;
					const REAL * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * REAL (*(arrIter++));
					extIter++;
					*resIter = sum;
					resIter += N;
					
					//for rescale
					if(max_val<*arrIter) max_val = *arrIter;
					if(min_val>*arrIter) min_val = *arrIter;
				}
			}
		}
		
		//de-alloc
		if (WeightsY) {delete []WeightsY; WeightsY=0;}
		if (extension_bufferY) {delete []extension_bufferY; extension_bufferY=0;}
	}
	
	//  Filtering  aV3DLONG z
	if(P<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		REAL  *WeightsZ = 0;
		WeightsZ = new REAL [Wz];
		if (!WeightsZ)
			return;
		
		unsigned int Half = Wz >> 1;
		WeightsZ[Half] = 1.;
		
		for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
		{
			const REAL  z = 3.* REAL (Weight) / REAL (Half);
			WeightsZ[Half - Weight] = WeightsZ[Half + Weight] = exp(-z * z / 2.);	// Corresponding symmetric WeightsZ
		}
		
		REAL k = 0.;
		for (unsigned int Weight = 0; Weight < Wz; ++Weight)
			k += WeightsZ[Weight];
		
		for (unsigned int Weight = 0; Weight < Wz; ++Weight)
			WeightsZ[Weight] /= k;
		
		
		//	aV3DLONG z
		REAL  *extension_bufferZ = 0;
		extension_bufferZ = new REAL [P + (Wz<<1)];
		
		unsigned int offset = Wz>>1;
		const REAL *extStop = extension_bufferZ + P + offset;
		
		for(T2 iy = 0; iy < M; iy++)
		{
			for(T2 ix = 0; ix < N; ix++)
			{
				
				REAL  *extIter = extension_bufferZ + Wz;
				for(T2 iz = 0; iz < P; iz++)
				{
					*(extIter++) = pOut.pImg[iz*M*N + iy*N + ix];
				}
				
				//   Extend image
				const REAL  *const stop_line = extension_bufferZ - 1;
				REAL  *extLeft = extension_bufferZ + Wz - 1;
				const REAL  *arrLeft = extLeft + 2;
				REAL  *extRight = extLeft + P + 1;
				const REAL  *arrRight = extRight - 2;
				
				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}
				
				//	Filtering
				extIter = extension_bufferZ + offset;
				
				REAL  *resIter = &(pOut.pImg[iy*N + ix]);
				
				while (extIter < extStop)
				{
					REAL sum = 0.;
					const REAL  *weightIter = WeightsZ;
					const REAL  *const End = WeightsZ + Wz;
					const REAL * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * REAL (*(arrIter++));
					extIter++;
					*resIter = sum;
					resIter += M*N;
					
					//for rescale
					if(max_val<*arrIter) max_val = *arrIter;
					if(min_val>*arrIter) min_val = *arrIter;
				}
				
			}
		}
		
		//de-alloc
		if (WeightsZ) {delete []WeightsZ; WeightsZ=0;}
		if (extension_bufferZ) {delete []extension_bufferZ; extension_bufferZ=0;}
		
	}
	
	// normalized filtered result to [0 255]
	max_val -= min_val;
	
	if(max_val)
	{
		for(T2 k=0; k<P; k++)
		{
			T2 offset_k = k*M*N;
			for(T2 j=0; j<M; j++)
			{
				T2 offset_j = offset_k + j*N;
				for(T2 i=0; i<N; i++)
				{
					T2 idx = offset_j + i;
					
					pIn.pImg[idx] = 255*(pOut.pImg[idx]-min_val)/(max_val);
				}
			}
		}		
	
	}
	
}

// compute normalized cross correlation score between a single pair of images 
template <class T1, class T2, class Y_IMG1, class Y_IMG2> void YImg<T1, T2, Y_IMG1, Y_IMG2> :: cmpt_ncc3D(Y_IMG1 pOut, Y_IMG2 pIn, T2 *sz_sub, T2 *sz_tar, T2 even_odd, bool fftwf_in_place, PEAKS *&pos)
{
	
//	T2 tx=sz_tar[0], ty=sz_tar[1], tz=sz_tar[2];
//	T2 sx_pad, sy_pad, sz_pad;
//	
//	sx_pad=pIn.sz[0];
//	sy_pad=pIn.sz[1];
//	sz_pad=pIn.sz[2];
//	
//	T2 len_pad = sx_pad*sy_pad*sz_pad;
//	
//	T2 sx, sy, sz;
//	
//	T2 sx_pad_ori;
//	
//	if(fftwf_in_place)
//		sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
//	else
//		sx_pad_ori = sx_pad;
//	
//	sx=sx_pad_ori+1-tx; // fftwf_in_place
//	sy=sy_pad+1-ty;
//	sz=sz_pad+1-tz;
//	
//	T2 sub_sz = sx*sy*sz;
//	
//	//cross-correlation
//	fftcc3D(pOut, pIn, even_odd, fftwf_in_place);
//	//PEAKSLIST peakList;
//	//fftpccc3D(pOut, pIn, even_odd, fftwf_in_place, &peakList);
//	
//	//ncc score
//	pos->value = 0;
//	
//	//
//	T2 pos_x = pos->x;
//	T2 pos_y = pos->y;
//	T2 pos_z = pos->z;
//	
//	// computing ncc
//	T2 u=pos_x, v=pos_y, w=pos_z;
//
//	T2 idx = w*sx_pad*sy_pad + v*sx_pad + u; 
//	
//	T2 t_lx, t_rx, t_uy, t_dy, t_fz, t_bz;
//	T2 s_lx, s_rx, s_uy, s_dy, s_fz, s_bz;
//	
//	T1 t_std, t_mean;
//	T1 s_std, s_mean;
//	
//	if(u<sx && u<tx)
//	{
//		s_lx = sx-1 - u; s_rx = sx-1;
//		t_lx = sx-1; t_rx = sx-1 + u;
//	}
//	else if(u<sx && u>=tx)
//	{
//		s_lx = sx-1 - u; s_rx = sx_pad_ori-1 - u;
//		t_lx = sx-1; t_rx = sx_pad_ori-1;
//	}
//	else if(u>=sx && u<tx)
//	{
//		s_lx = 0; s_rx = sx-1;
//		t_lx = u; t_rx = sx-1 + u;
//	}
//	else if(u>=tx)
//	{
//		s_lx = 0; s_rx = sx_pad_ori-1 - u;
//		t_lx = u; t_rx = sx_pad_ori-1;
//	}
//	else
//		printf("x direction %ld %ld %ld %ld \n", u, sx, tx, sx_pad);
//	
//	if(v<sy && v<ty)
//	{
//		s_uy = sy-1 - v; s_dy = sy-1;
//		t_uy = sy-1; t_dy = sy-1 + v;
//	}
//	else if(v<sy && v>=ty)
//	{
//		s_uy = sy-1 - v; s_dy = sy_pad-1 - v;
//		t_uy = sy-1; t_dy = sy_pad-1;
//	}
//	else if(v<ty && v>=sy)
//	{
//		s_uy = 0; s_dy = sy-1;
//		t_uy = v; t_dy = sy-1 + v;
//	}
//	else if(v>=ty)
//	{
//		s_uy = 0; s_dy = sy_pad-1 - v;
//		t_uy = v; t_dy = sy_pad-1;
//	}
//	else
//		printf("y direction %ld %ld %ld %ld \n", v, sy, ty, sy_pad);
//	
//	if(w<sz && w<tz)
//	{
//		s_fz = sz-1 - w; s_bz = sz-1;
//		t_fz = sz-1; t_bz = sz-1 + w;
//	}
//	else if(w<sz && w>=tz)
//	{
//		s_fz = sz-1 - w; s_bz = sz_pad-1 - w;
//		t_fz = sz-1; t_bz = sz_pad-1;
//	}
//	else if(w<tz && w>=sz)
//	{
//		s_fz = 0; s_bz = sz-1;
//		t_fz = w; t_bz = sz-1 + w;
//	}
//	else if(w>=tz)
//	{
//		s_fz = 0; s_bz = sz_pad-1 - w;
//		t_fz = w; t_bz = sz_pad-1;
//	}
//	else
//		printf("z direction %ld %ld %ld %ld \n", w, sz, tz, sz_pad);
//				
//	//
//	if(t_rx<t_lx || t_dy<t_uy || t_bz<t_fz || s_rx<s_lx || s_dy<s_uy || s_bz<s_fz)
//	{
//		cout<<"Overlap region is invalid."<<endl;
//		return;
//	}
//	
//	// overlap
//	T2 len_t = (t_bz - t_fz + 1)*(t_dy - t_uy + 1)*(t_rx - t_lx + 1), len_s = (s_bz - s_fz + 1)*(s_dy - s_uy + 1)*(s_rx - s_lx + 1);
//				
//	T1 t1,t2;
//	
//	t1=0; t2=0;
//	for(T2 k=t_fz; k<=t_bz; k++)
//	{
//		T2 offset_k = k*sx_pad_ori*sy_pad;
//		for(T2 j=t_uy; j<=t_dy; j++)
//		{
//			T2 offset_j = offset_k + j*sx_pad_ori;
//			for(T2 i=t_lx; i<=t_rx; i++)
//			{
//				T2 idx = offset_j + i;
//				
//				T1 tmp = pIn.pImg[idx];
//				
//				t1 += tmp;
//				
//				t2 += tmp*tmp;
//			}
//		}
//	}
//	
//	t_mean = t1/len_t;
//	t_std = sqrt(y_max(t2 - t1*t_mean, T1(0) ));
//	
//	T1 s1,s2;
//	
//	s1=0; s2=0;
//	for(T2 k=s_fz; k<=s_bz; k++)
//	{
//		T2 offset_k = k*sx_pad_ori*sy_pad;
//		for(T2 j=s_uy; j<=s_dy; j++)
//		{
//			T2 offset_j = offset_k + j*sx_pad_ori;
//			for(T2 i=s_lx; i<=s_rx; i++)
//			{
//				T2 idx = offset_j + i;
//				
//				T1 tmp = pOut.pImg[idx];
//				
//				s1 += tmp;
//				
//				s2 += tmp*tmp;
//			}
//		}
//	}
//	
//	s_mean = s1/len_s;
//	s_std = sqrt(y_max(s2 - s1*s_mean, T1(0) ));
//	
//	//qDebug()<< s_mean << s_std << s1 << t_mean << t_std << t1 << u << v << w; ;
//	//qDebug() << "test" <<  0.5 + 0.5*(pOut.pImg[idx] - t1*t_mean)/(t_std*t_std) << u << v << w; 
//	
//	// response [0, 1] (instead of [-1, 1])
//	if(t_std!=0 && s_std!=0)
//	{
//		qDebug()<< "compute..." << pOut.pImg[idx];
//		pos->value = 0.5 + 0.5*(pOut.pImg[idx] - t1*s_mean)/(t_std*s_std);
//		//qDebug()<< "compute...normalized" << pOut.pImg[idx];
//	}
	
	T2 tx=sz_tar[0], ty=sz_tar[1], tz=sz_tar[2];
	T2 sx_pad, sy_pad, sz_pad;
	
	sx_pad=pIn.sz[0];
	sy_pad=pIn.sz[1];
	sz_pad=pIn.sz[2];
	
	T2 len_pad = sx_pad*sy_pad*sz_pad;
	
	T2 sx, sy, sz;
	
	T2 sx_pad_ori;
	
	if(fftwf_in_place)
		sx_pad_ori = sx_pad - (2-even_odd); //2*(sx_pad/2-1);
	else
		sx_pad_ori = sx_pad;
	
	sx=sx_pad_ori+1-tx; // fftwf_in_place
	sy=sy_pad+1-ty;
	sz=sz_pad+1-tz;
	
	T2 sub_sz = sx*sy*sz;
	
	int start_t = clock();
	
	NST<T1, T2> ncctar(pIn.sz, pIn.pImg, even_odd, fftwf_in_place, 3);
	NST<T1, T2> nccsub(pOut.sz, pOut.pImg, even_odd, fftwf_in_place, 3);
	
	int end_t = clock();
	
	qDebug() << "time consumed ... " << end_t - start_t;
	
	
	// pc and cc
	PEAKSLIST peakList;
	fftpccc3D(pOut, pIn, even_odd, fftwf_in_place, &peakList);
	//fftpatientpccc3D(pOut, pIn, even_odd, fftwf_in_place, &peakList);
	
	int end_tt = clock();
	
	qDebug() << "pccc time consumed ... " << end_tt - end_t;
	
	//ncc
	pos->value = 0;
	
	//
	T2 pos_x = pos->x;
	T2 pos_y = pos->y;
	T2 pos_z = pos->z;
	
	//
	T2 offset_z = sx_pad_ori*sy_pad;
	
	T2 u=pos_x, v=pos_y, w=pos_z;

	T2 idx = w*sx_pad*sy_pad + v*sx_pad + u;
			
	// z - (front, back) y - (upper, down) x - (left, right)
	T2 t_ful_idx, t_fur_idx, t_fdl_idx, t_fdr_idx, t_bul_idx, t_bur_idx, t_bdl_idx, t_bdr_idx;
	T2 s_ful_idx, s_fur_idx, s_fdl_idx, s_fdr_idx, s_bul_idx, s_bur_idx, s_bdl_idx, s_bdr_idx;
	
	T2 t_lx, t_rx, t_uy, t_dy, t_fz, t_bz;
	T2 s_lx, s_rx, s_uy, s_dy, s_fz, s_bz;
	
	T1 t_std, t_mean;
	T1 s_std, s_mean;
	
	if(u<sx && u<tx)
	{
		s_lx = sx-1 - u; s_rx = sx-1;
		t_lx = sx-1; t_rx = sx-1 + u;
	}
	else if(u<sx && u>=tx)
	{
		s_lx = sx-1 - u; s_rx = sx_pad_ori-1 - u;
		t_lx = sx-1; t_rx = sx_pad_ori-1;
	}
	else if(u>=sx && u<tx)
	{
		s_lx = 0; s_rx = sx-1;
		t_lx = u; t_rx = sx-1 + u;
	}
	else if(u>=tx)
	{
		s_lx = 0; s_rx = sx_pad_ori-1 - u;
		t_lx = u; t_rx = sx_pad_ori-1;
	}
	else
		printf("x direction %ld %ld %ld %ld \n", u, sx, tx, sx_pad);
	
	if(v<sy && v<ty)
	{
		s_uy = sy-1 - v; s_dy = sy-1;
		t_uy = sy-1; t_dy = sy-1 + v;
	}
	else if(v<sy && v>=ty)
	{
		s_uy = sy-1 - v; s_dy = sy_pad-1 - v;
		t_uy = sy-1; t_dy = sy_pad-1;
	}
	else if(v<ty && v>=sy)
	{
		s_uy = 0; s_dy = sy-1;
		t_uy = v; t_dy = sy-1 + v;
	}
	else if(v>=ty)
	{
		s_uy = 0; s_dy = sy_pad-1 - v;
		t_uy = v; t_dy = sy_pad-1;
	}
	else
		printf("y direction %ld %ld %ld %ld \n", v, sy, ty, sy_pad);
	
	if(w<sz && w<tz)
	{
		s_fz = sz-1 - w; s_bz = sz-1;
		t_fz = sz-1; t_bz = sz-1 + w;
	}
	else if(w<sz && w>=tz)
	{
		s_fz = sz-1 - w; s_bz = sz_pad-1 - w;
		t_fz = sz-1; t_bz = sz_pad-1;
	}
	else if(w<tz && w>=sz)
	{
		s_fz = 0; s_bz = sz-1;
		t_fz = w; t_bz = sz-1 + w;
	}
	else if(w>=tz)
	{
		s_fz = 0; s_bz = sz_pad-1 - w;
		t_fz = w; t_bz = sz_pad-1;
	}
	else
		printf("z direction %ld %ld %ld %ld \n", w, sz, tz, sz_pad);
	
	if(t_rx<t_lx || t_dy<t_uy || t_bz<t_fz || s_rx<s_lx || s_dy<s_uy || s_bz<s_fz)
	{
		pOut.pImg[idx] = 0;
		return;
	}
	
	// overlap boundary
	t_ful_idx = (t_fz-1)*offset_z + (t_uy-1)*sx_pad_ori + t_lx-1;
	t_fur_idx = (t_fz-1)*offset_z + (t_uy-1)*sx_pad_ori + t_rx;
	t_fdl_idx = (t_fz-1)*offset_z + t_dy*sx_pad_ori + t_lx-1;
	t_fdr_idx = (t_fz-1)*offset_z + t_dy*sx_pad_ori + t_rx;
	t_bul_idx = t_bz*offset_z + (t_uy-1)*sx_pad_ori + t_lx-1;
	t_bur_idx = t_bz*offset_z + (t_uy-1)*sx_pad_ori + t_rx;
	t_bdl_idx = t_bz*offset_z + t_dy*sx_pad_ori + t_lx-1;
	t_bdr_idx = t_bz*offset_z + t_dy*sx_pad_ori + t_rx;
	
	s_ful_idx = (s_fz-1)*offset_z + (s_uy-1)*sx_pad_ori + s_lx-1;
	s_fur_idx = (s_fz-1)*offset_z + (s_uy-1)*sx_pad_ori + s_rx;
	s_fdl_idx = (s_fz-1)*offset_z + s_dy*sx_pad_ori + s_lx-1;
	s_fdr_idx = (s_fz-1)*offset_z + s_dy*sx_pad_ori + s_rx;
	s_bul_idx = s_bz*offset_z + (s_uy-1)*sx_pad_ori + s_lx-1;
	s_bur_idx = s_bz*offset_z + (s_uy-1)*sx_pad_ori + s_rx;
	s_bdl_idx = s_bz*offset_z + s_dy*sx_pad_ori + s_lx-1;
	s_bdr_idx = s_bz*offset_z + s_dy*sx_pad_ori + s_rx;
	
	T2 len_t = (t_bz - t_fz + 1)*(t_dy - t_uy + 1)*(t_rx - t_lx + 1), len_s = (s_bz - s_fz + 1)*(s_dy - s_uy + 1)*(s_rx - s_lx + 1);
	
	T1 t1,t2;
	
	if(t_lx==0 && t_uy==0 && t_fz==0)
	{
		t1 = ncctar.sum1[t_bdr_idx];
		t2 = ncctar.sum2[t_bdr_idx];
	}
	else if(t_lx>0 && t_uy==0 && t_fz==0)
	{
		t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx];
		t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx];
	}
	else if(t_lx==0 && t_uy>0 && t_fz==0)
	{
		t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx];
		t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx];
	}
	else if(t_lx==0 && t_uy==0 && t_fz>0)
	{
		t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_fdr_idx];
		t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_fdr_idx];
	}
	else if(t_lx>0 && t_uy>0 && t_fz==0)
	{
		t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] + ncctar.sum1[t_bul_idx];
		t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] + ncctar.sum2[t_bul_idx];
	}
	else if(t_lx>0 && t_uy==0 && t_fz>0)
	{
		t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fdl_idx];
		t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fdl_idx];
	}
	else if(t_lx==0 && t_uy>0 && t_fz>0)
	{
		t1 = ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_fur_idx];
		t2 = ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_fur_idx];
	}
	else
	{
		t1 = fabs( ncctar.sum1[t_bdr_idx] - ncctar.sum1[t_bdl_idx] - ncctar.sum1[t_bur_idx] - ncctar.sum1[t_fdr_idx] + ncctar.sum1[t_bul_idx] + ncctar.sum1[t_fdl_idx] + ncctar.sum1[t_fur_idx] - ncctar.sum1[t_ful_idx]);
		t2 = fabs( ncctar.sum2[t_bdr_idx] - ncctar.sum2[t_bdl_idx] - ncctar.sum2[t_bur_idx] - ncctar.sum2[t_fdr_idx] + ncctar.sum2[t_bul_idx] + ncctar.sum2[t_fdl_idx] + ncctar.sum2[t_fur_idx] - ncctar.sum2[t_ful_idx]);
	}
	
	t_mean = t1/len_t; 
	t_std = sqrt(y_max(t2 - t1*t_mean, T1(0) ));
	
	T1 s1,s2;
	
	if(s_lx==0 && s_uy==0 && s_fz==0)
	{
		s1 = nccsub.sum1[s_bdr_idx];
		s2 = nccsub.sum2[s_bdr_idx];
	}
	else if(s_lx>0 && s_uy==0 && s_fz==0)
	{
		s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx];
		s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx];
	}
	else if(s_lx==0 && s_uy>0 && s_fz==0)
	{
		s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx];
		s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx];
	}
	else if(s_lx==0 && s_uy==0 && s_fz>0)
	{
		s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_fdr_idx];
		s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_fdr_idx];
	}
	else if(s_lx>0 && s_uy>0 && s_fz==0)
	{
		s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] + nccsub.sum1[s_bul_idx];
		s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] + nccsub.sum2[s_bul_idx];
	}
	else if(s_lx>0 && s_uy==0 && s_fz>0)
	{
		s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fdl_idx];
		s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fdl_idx];
	}
	else if(s_lx==0 && s_uy>0 && s_fz>0)
	{
		s1 = nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_fur_idx];
		s2 = nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_fur_idx];
	}
	else
	{
		s1 = fabs( nccsub.sum1[s_bdr_idx] - nccsub.sum1[s_bdl_idx] - nccsub.sum1[s_bur_idx] - nccsub.sum1[s_fdr_idx] + nccsub.sum1[s_bul_idx] + nccsub.sum1[s_fdl_idx] + nccsub.sum1[s_fur_idx] - nccsub.sum1[s_ful_idx]);
		s2 = fabs( nccsub.sum2[s_bdr_idx] - nccsub.sum2[s_bdl_idx] - nccsub.sum2[s_bur_idx] - nccsub.sum2[s_fdr_idx] + nccsub.sum2[s_bul_idx] + nccsub.sum2[s_fdl_idx] + nccsub.sum2[s_fur_idx] - nccsub.sum2[s_ful_idx]);
	}
	
	s_mean = s1/len_s;
	s_std = sqrt(y_max(s2 - s1*s_mean, T1(0) ));
	
	// response [0, 1] (instead of [-1, 1])
	if(t_std!=0 && s_std!=0)
	{
		T1 tmp = pOut.pImg[idx];
		
		tmp = 0.5 + 0.5*(tmp - t1*s_mean)/(t_std*s_std);
		
		if(tmp>pos->value)
		{	
			pos->value = tmp;
			pos->x = u; pos->y = v; pos->z = w;
		}
	}
	
	
}

#endif
