/*******************************************************************************************
 *
 * basic_4dimage.h
 *
 * This function is a basic function interface of the V3D project.
 *
 * Copyright: Hanchuan Peng (Howard Hughes Medical Institute, Janelia Farm Research Campus).
 * The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
 *
 * Last edit: 2009-Aug-21
 *
 *******************************************************************************************
 */

#ifndef _BASIC_4DIMAGE_H_
#define _BASIC_4DIMAGE_H_

// be compatible with LP64(unix64) and LLP64(win64)
typedef unsigned char        uint8;
typedef unsigned short       uint16;
typedef unsigned int         uint32;
typedef unsigned long long   uint64;
typedef          char        sint8;
typedef          short       sint16;
typedef          int         sint32;
typedef          long long   sint64;
typedef          float       float32;
typedef          double      float64;


enum ImagePixelType {V3D_UNKNOWN, V3D_UINT8, V3D_UINT16, V3D_FLOAT32};
enum TimePackType {TIME_PACK_NONE,TIME_PACK_Z,TIME_PACK_C}; 

class Image4DSimple
{
	//the data members are set as public, but should avoid using them directly
public:
	unsigned char * data1d;
	long sz0, sz1, sz2, sz3;
	long sz_time;
	TimePackType timepacktype;
	ImagePixelType datatype;
	char imgSrcFile[1024]; //use a long path to store the full path
	int b_error;

public:
	Image4DSimple() {
		data1d = 0;
		sz0 = sz1 = sz2 = sz3 = 0;
		sz_time = 0;
		datatype = V3D_UNKNOWN;
		timepacktype = TIME_PACK_NONE;
		imgSrcFile[0] = '\0';
		b_error = 0;
	}
	~Image4DSimple() {
		cleanExistData();
	}
	void cleanExistData()
	{
		if (data1d) {delete []data1d; data1d = 0;}
		sz0 = sz1 = sz2 = sz3 = 0;
		sz_time = 0;
		datatype = V3D_UNKNOWN;
		timepacktype = TIME_PACK_NONE;
		imgSrcFile[0] = '\0';
		b_error = 0;
	}

	//main interface to the data
	unsigned char * getRawData() {return data1d;}
	long getXDim() {return sz0;}
	long getYDim() {return sz1;}
	long getZDim() {return sz2;}
	long getCDim() {return sz3;}
	long getTDim() {return sz_time;}
	ImagePixelType getDatatype() {return datatype;}
	TimePackType getTimePackType() {return timepacktype;}
	long getTotalUnitNumber() {return sz0*sz1*sz2*sz3;}
	long getTotalUnitNumberPerPlane() {return sz0*sz1;}
	long getTotalUnitNumberPerChannel() {return sz0*sz1*sz2;}
	long getUnitBytes()
	{
		switch (datatype)
		{
			case V3D_UINT8: return 1;
			case V3D_UINT16: return 2;
			case V3D_FLOAT32: return 4;
			default: return 1;
		}
	}
	long getTotalBytes() {return getUnitBytes()*sz0*sz1*sz2*sz3;}
	int isSuccess() {if (sz0<=0 || sz1<=0 || sz2<=0 || sz3<=0) b_error=1; return !b_error;}

	void setXDim(long v) {sz0=v;}
	void setYDim(long v) {sz1=v;}
	void setZDim(long v) {sz2=v;}
	void setCDim(long v) {sz3=v;}
	void setTDim(long v) {sz_time=v;}
	void setDatatype(ImagePixelType v) {datatype=v;}
	void setTimePackType(TimePackType v) {timepacktype=v;}
	bool setNewRawDataPointer(unsigned char *p) {if (!p) return false; if (data1d) delete []data1d; data1d = p; return true;}

	//this function is the main place to call if you want to set your own 1d pointer data to this data structure
	bool setData(unsigned char *p, long s0, long s1, long s2, long s3, ImagePixelType dt)
	{
		if (p && s0>0 && s1>0 && s2>0 && s3>0 && (dt==V3D_UINT8 || dt==V3D_UINT16 || dt==V3D_FLOAT32))
			if (setNewRawDataPointer(p))
			{
				setXDim(s0);
				setYDim(s1);
				setZDim(s2);
				setCDim(s3);
				setDatatype(dt);
				return true;
			}
		return false;
	}

	bool setFileName(char * myfile)
	{
		if (!myfile) return false;
		long clen=1023;
		long i;
		for (i=0;i<clen;i++)
		{
			if (myfile[i]!='\0') imgSrcFile[i] = myfile[i];
			else {imgSrcFile[i] = myfile[i]; break;}
		}
		imgSrcFile[i]='\0';
		return true;
	}
	char * getFileName() {return imgSrcFile;}

	//to call the following 4 functions you must link your project with basic_4dimage.cpp
	//Normally for the plugin interfaces you don't need to call the following functions
	void loadImage(char filename[]);
	bool saveImage(const char filename[]);
	bool createImage(long mysz0, long mysz1, long mysz2, long mysz3, ImagePixelType mytype);
	void createBlankImage(long imgsz0, long imgsz1, long imgsz2, long imgsz3, int imgdatatype);

};

bool convert_data_to_8bit(void * &img, long * sz, int datatype);

template<class T> class Image4DProxy
{
public:
	uint8* data_p;
	long nbytes, su, sx, sy, sz, sc;
	long stride_x, stride_y, stride_z, stride_c;
	Image4DProxy(T *img)
	{
		data_p = img->getRawData();
		nbytes = img->getTotalBytes();
		su = img->getUnitBytes();
		sx = img->getXDim();
		sy = img->getYDim();
		sz = img->getZDim();
		sc = img->getCDim();
		stride_x = su;
		stride_y = su*sx;
		stride_z = su*sx*sy;
		stride_c = su*sx*sy*sz;
	}
	inline uint8* at(long x, long y, long z, long c)
	{
		return (data_p + stride_x*x + stride_y*y + stride_z*z + stride_c*c);
	}
	inline uint8* begin()
	{
		return data_p;
	}
	inline uint8* end()
	{
		return (data_p + nbytes-1);
	}
	inline bool is_inner(long x, long y, long z, long c)
	{
		return !(x<0 || x>=sx || y<0 || y>=sy || z<0 || z>=sz || c<0 || c>=sc);
	}
	inline uint8* at_uint8(long x, long y, long z, long c)
	{
		return (uint8*)at(x,y,z,c);
	}
	inline uint16* at_uint16(long x, long y, long z, long c)
	{
		return (uint16*)at(x,y,z,c);
	}
	inline float32* at_float32(long x, long y, long z, long c)
	{
		return (float32*)at(x,y,z,c);
	}
};
#define Image4DProxy_foreach(p, x,y,z,c) \
for (long c = 0; c < p.sc; c++) \
for (long z = 0; z < p.sz; z++) \
for (long y = 0; y < p.sy; y++) \
for (long x = 0; x < p.sx; x++)


#endif /* _BASIC_4DIMAGE_H_ */
