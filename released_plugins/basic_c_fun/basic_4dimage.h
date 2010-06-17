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
 * Last edit: 2010-May-19: replace long with V3DLONG
 * Last edit: 2010-May-30: add the value_at() function for Image4DProxy class
 *
 *******************************************************************************************
 */

#ifndef _BASIC_4DIMAGE_H_
#define _BASIC_4DIMAGE_H_

#include "v3d_basicdatatype.h"

class Image4DSimple
{
	//the data members are set as public, but should avoid using them directly
public:
	unsigned char * data1d;
	V3DLONG sz0, sz1, sz2, sz3;
	V3DLONG sz_time;
	TimePackType timepacktype;
	ImagePixelType datatype;
	char imgSrcFile[1024]; //use a V3DLONG path to store the full path
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
	V3DLONG getXDim() {return sz0;}
	V3DLONG getYDim() {return sz1;}
	V3DLONG getZDim() {return sz2;}
	V3DLONG getCDim() {return sz3;}
	V3DLONG getTDim() {return sz_time;}
	ImagePixelType getDatatype() {return datatype;}
	TimePackType getTimePackType() {return timepacktype;}
	V3DLONG getTotalUnitNumber() {return sz0*sz1*sz2*sz3;}
	V3DLONG getTotalUnitNumberPerPlane() {return sz0*sz1;}
	V3DLONG getTotalUnitNumberPerChannel() {return sz0*sz1*sz2;}
	V3DLONG getUnitBytes()
	{
		switch (datatype)
		{
			case V3D_UINT8: return 1;
			case V3D_UINT16: return 2;
			case V3D_FLOAT32: return 4;
			default: return 1;
		}
	}
	V3DLONG getTotalBytes() {return getUnitBytes()*sz0*sz1*sz2*sz3;}
	unsigned char * getRawDataAtChannel(V3DLONG cid) 
	{
		V3DLONG myid = cid; if (myid<0) myid=0; else if (myid>=sz3) myid = sz3-1; 
		return data1d + myid*getTotalUnitNumberPerChannel()*getUnitBytes();
	}
	int isSuccess() {if (sz0<=0 || sz1<=0 || sz2<=0 || sz3<=0) b_error=1; return !b_error;}
	bool valid() {return (!data1d || sz0<=0 || sz1<=0 || sz2<=0 || sz3<=0 || b_error || (datatype!=V3D_UINT8 && datatype!=V3D_UINT16 && datatype!=V3D_FLOAT32)) ?  false : true; }

	void setXDim(V3DLONG v) {sz0=v;}
	void setYDim(V3DLONG v) {sz1=v;}
	void setZDim(V3DLONG v) {sz2=v;}
	void setCDim(V3DLONG v) {sz3=v;}
	void setTDim(V3DLONG v) {sz_time=v;}
	void setDatatype(ImagePixelType v) {datatype=v;}
	void setTimePackType(TimePackType v) {timepacktype=v;}
	bool setNewRawDataPointer(unsigned char *p) {if (!p) return false; if (data1d) delete []data1d; data1d = p; return true;}

	//this function is the main place to call if you want to set your own 1d pointer data to this data structure
	bool setData(unsigned char *p, V3DLONG s0, V3DLONG s1, V3DLONG s2, V3DLONG s3, ImagePixelType dt)
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
		V3DLONG clen=1023;
		V3DLONG i;
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
	bool createImage(V3DLONG mysz0, V3DLONG mysz1, V3DLONG mysz2, V3DLONG mysz3, ImagePixelType mytype);
	void createBlankImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype);

};

bool convert_data_to_8bit(void * &img, V3DLONG * sz, int datatype);

template<class T> class Image4DProxy
{
public:
	T *img0;
	uint8* data_p;
	V3DLONG nbytes, su, sx, sy, sz, sc;
	V3DLONG stride_x, stride_y, stride_z, stride_c;
	Image4DProxy(T *img)
	{
		img0 = img; //keep a copy for future accessing of the data
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
	inline uint8* at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
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
	inline bool is_inner(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return !(x<0 || x>=sx || y<0 || y>=sy || z<0 || z>=sz || c<0 || c>=sc);
	}
	inline uint8* at_uint8(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return (uint8*)at(x,y,z,c);
	}
	inline uint16* at_uint16(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return (uint16*)at(x,y,z,c);
	}
	inline float32* at_float32(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return (float32*)at(x,y,z,c);
	}
	inline double value_at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		double v;
		switch (su)
		{
			case 1: v = (double)(*at(x,y,z,c)); break;
			case 2: v = (double)(*(uint16 *)at(x,y,z,c)); break;
			case 4: v = (double)(*(float32 *)at(x,y,z,c)); break;
			default: v = (double)(*at(x,y,z,c)); break;
		}
		return v;
	}
};
#define Image4DProxy_foreach(p, x,y,z,c) \
for (V3DLONG c = 0; c < p.sc; c++) \
for (V3DLONG z = 0; z < p.sz; z++) \
for (V3DLONG y = 0; y < p.sy; y++) \
for (V3DLONG x = 0; x < p.sx; x++)


//The following struct is provided for convenience for working with a channel of an Image4DSimple instance in some cases
struct V3D_Image3DBasic
{
	unsigned char * data1d;
	V3DLONG sz0, sz1, sz2;
	ImagePixelType datatype;
	V3DLONG cid; //the color channel in the original 4D image
	
	V3D_Image3DBasic() {data1d=0; sz0=sz1=sz2=0; datatype=V3D_UNKNOWN; cid=-1;}
	bool setData(Image4DSimple *p, V3DLONG myid)
	{
		if (!p || !p->valid() || myid<0 || myid>=p->getCDim())
			return false;
		cid = myid; 
		data1d = p->getRawDataAtChannel(cid);
		sz0 = p->getXDim(); sz1 = p->getYDim(); sz2 = p->getZDim();  
		datatype = p->getDatatype();
		return true;
	}
};


#endif /* _BASIC_4DIMAGE_H_ */
