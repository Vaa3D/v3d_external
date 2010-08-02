/*
 *  color_xyz.h
 *
 * Copyright: Hanchuan Peng, Zongcai Ruan (Howard Hughes Medical Institute, Janelia Farm Research Campus).
 * The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
 *
 * Last edit. 2009-Aug-21
 *
 */

#ifndef __V3D_COLOR_XYZ_H__
#define __V3D_COLOR_XYZ_H__

#include <math.h> // for sqrt(), rand()...
#include <stdlib.h>

#define BIG_FLOAT 1e+38


///////////////////////////////////////////////////////

#ifndef ABS
#define ABS(a)  ( ((a)>0)? (a) : -(a) )
#endif
#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif
#ifndef ABSMIN
#define ABSMIN(a, b)  ( (ABS(a)<ABS(b))? (a) : (b) )
#endif
#ifndef ABSMAX
#define ABSMAX(a, b)  ( (ABS(a)>ABS(b))? (a) : (b) )
#endif
#ifndef CLAMP
#define CLAMP(a, b, x)  MIN( MAX(MIN(a,b), x), MAX(a,b))
#endif
#ifndef BETWEEN
#define BETWEEN(a, b, x)  (MIN(a,b)<(x) && (x)<MAX(a,b))
#endif
#ifndef BETWEENEQ
#define BETWEENEQ(a, b, x)  (MIN(a,b)<=(x) && (x)<=MAX(a,b))
#endif
#ifndef IROUND
#define IROUND(x)  int(x + .5)
#endif
inline int power_of_two_ceil(int x)  { int y; for (y = 1; y < x; y = y*2);	return y; }
inline int power_of_two_floor(int x) { int y; for (y = 1; y <= x; y = y*2);	return y/2; }

////////////////////////////////////////////////////////

union RGB8 {
	struct { unsigned char r,g,b; };
	unsigned char c[3];
};
union RGBA8 {
	struct { unsigned char r,g,b,a; };
	unsigned char c[4];
	unsigned int i;
};
union BGR8 { // Windows-DIB
	struct { unsigned char b,g,r; };
	unsigned char c[3];
};
union BGRA8 { // Windows-DIB
	struct { unsigned char b,g,r,a; };
	unsigned char c[4];
	unsigned int i;
};
union ABGR8 { // little endian
	struct { unsigned char a,b,g,r; };
	unsigned char c[4];
	unsigned int i;
};

union RGB16i {
	struct { short r,g,b; };
	short c[3];
};
union RGBA16i {
	struct { short r,g,b,a; };
	short c[4];
};
union RGB32i {
	struct { int r,g,b; };
	int c[3];
};
union RGBA32i {
	struct { int r,g,b,a; };
	int c[4];
};
union RGB32f {
	struct { float r,g,b; };
	float c[3];
};
union RGBA32f {
	struct { float r,g,b,a; };
	float c[4];
};

RGB8 random_rgb8(); // use normalize(XYZ)
RGBA8 random_rgba8();
RGBA8 random_rgba8(unsigned char a);

inline RGBA8 rgba8_from(ABGR8 d) {
	RGBA8 c;	c.r=d.r; c.g=d.g; c.b=d.b;	c.a=d.a;	return c;
}
inline RGBA8 rgba8_from(BGRA8 d) {
	RGBA8 c;	c.r=d.r; c.g=d.g; c.b=d.b;	c.a=d.a;	return c;
}

inline ABGR8 abgr8_from(RGBA8 c) {
	ABGR8 d;	d.r=c.r; d.g=c.g; d.b=c.b;	d.a=c.a;	return d;
}
inline BGRA8 bgra8_from(RGBA8 c) {
	BGRA8 d;	d.r=c.r; d.g=c.g; d.b=c.b;	d.a=c.a;	return d;
}


/////////////////////////////////////////////////////////////
//struct XYZ;
//struct XYZW;

struct XYZ {
	union {
	struct {float x, y, z;};
	float v[3];
	};

	XYZ(float x, float y, float z)	{this->x=x; this->y=y; this->z=z;}
	XYZ(float a=0)	{x=a; y=a; z=a;}
	XYZ(RGB8 c)	    {x=c.r; y=c.g; z=c.b;}
	XYZ(RGB16i c)	{x=c.r; y=c.g; z=c.b;}
	XYZ(RGB32i c)	{x=c.r; y=c.g; z=c.b;}
	XYZ(RGB32f c)	{x=c.r; y=c.g; z=c.b;}
	operator RGB8()   {RGB8 c; c.r=(unsigned char)x; c.g=(unsigned char)y; c.b=(unsigned char)z; return c;}
	operator RGB16i() {RGB16i c; c.r=(short)x; c.g=(short)y; c.b=(short)z; return c;}
	operator RGB32i() {RGB32i c; c.r=(int)x; c.g=(int)y; c.b=(int)z; return c;}
	operator RGB32f() {RGB32f c; c.r=x; c.g=y; c.b=z; return c;}
};

struct XYZW {
	union {
	struct {float x, y, z, w;};
	float v[4];
	};

	XYZW(float x, float y, float z, float w)	{this->x=x, this->y=y; this->z=z; this->w=w;}
	XYZW(float a=0)	{x=a; y=a; z=a; w=a;}
	XYZW(RGBA8 c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	XYZW(RGBA16i c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	XYZW(RGBA32i c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	XYZW(RGBA32f c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	operator RGBA8()   {RGBA8 c; c.r=(unsigned char)x; c.g=(unsigned char)y; c.b=(unsigned char)z; c.a=(unsigned char)w; return c;}
	operator RGBA16i() {RGBA16i c; c.r=(short)x; c.g=(short)y; c.b=(short)z; c.a=(short)w; return c;}
	operator RGBA32i() {RGBA32i c; c.r=(int)x; c.g=(int)y; c.b=(int)z; c.a=(int)w; return c;}
	operator RGBA32f() {RGBA32f c; c.r=x; c.g=y; c.b=z; c.a=w; return c;}
	XYZW(XYZ v, float w=1) {x=v.x; y=v.y; z=v.z; this->w=w;}
	operator XYZ()   {XYZ v; v.x=x; v.y=y; v.z=z; return v;}
};

/////////////////////////////////////////////////////////////
//#define NULL_BoundingBox BoundingBox(XYZ(BIG_FLOAT), XYZ(-BIG_FLOAT))
#define NULL_BoundingBox BoundingBox(XYZ(1), XYZ(-1))
#define UNIT_BoundingBox BoundingBox(XYZ(0), XYZ(1))

struct BoundingBox {
	union {
	struct {float x0, y0, z0;	float x1, y1, z1;};
	float box[6];
	};

	BoundingBox() 					{ *this = NULL_BoundingBox; }
	BoundingBox(float a) 			{x0=y0=z0=x1=y1=z1 = a;}
	BoundingBox(float x0, float y0, float z0, float x1, float y1, float z1)
									{this->x0=x0; this->y0=y0; this->z0=z0;  this->x1=x1; this->y1=y1; this->z1=z1;}
	BoundingBox(XYZ V0, XYZ V1)		{x0=V0.x; y0=V0.y; z0=V0.z;  x1=V1.x; y1=V1.y; z1=V1.z;}
	float Dx() 		{return (x1-x0); }
	float Dy() 		{return (y1-y0); }
	float Dz() 		{return (z1-z0); }
	float Dmin() 	{return MIN(MIN(Dx(),Dy()),Dz());}
	float Dmax() 	{return MAX(MAX(Dx(),Dy()),Dz());}
	XYZ V0() 	 	{return XYZ(x0,y0,z0);}
	XYZ V1() 	 	{return XYZ(x1,y1,z1);}
	XYZ Vabsmin() 	{return XYZ(ABSMIN(x0,x1), ABSMIN(y0,y1), ABSMIN(z0,z1));}
	XYZ Vabsmax() 	{return XYZ(ABSMAX(x0,x1), ABSMAX(y0,y1), ABSMAX(z0,z1));}
	bool isNegtive()	 	{return (Dx()<0 || Dy()<0 || Dz()<0);}
	bool isInner(XYZ V) 	{return BETWEENEQ(x0,x1, V.x) && BETWEENEQ(y0,y1, V.y) && BETWEENEQ(z0,z1, V.z);}
	void expand(XYZ V) {
		if (Dx()<0) { x0 = x1 = V.x;} else { x0=MIN(x0, V.x); x1=MAX(x1, V.x); }
		if (Dy()<0) { y0 = y1 = V.y;} else { y0=MIN(y0, V.y); y1=MAX(y1, V.y); }
		if (Dz()<0) { z0 = z1 = V.z;} else { z0=MIN(z0, V.z); z1=MAX(z1, V.z); }
	}
	void expand(BoundingBox B) {
		if (B.isNegtive()) return;
		expand(B.V0()); expand(B.V1());
	}
	void shift(float x, float y, float z) 	{x0+=x; y0+=y; z0+=z;	x1+=x; y1+=y; z1+=z;}
	void shift(XYZ S) 						{shift(S.x, S.y, S.z);}
};


/////////////////////////////////////////////////////////////////
// operators of XYZ & XYZW & BoundingBox
/////////////////////////////////////////////////////////////////

inline bool operator == (const BoundingBox& A, const BoundingBox& B)
{
	return (A.x0==B.x0 && A.y0==B.y0 && A.z0==B.z0 && A.x1==B.x1 && A.y1==B.y1 && A.z1==B.z1);
}

//XYZW ----------------------------------------
inline bool operator == (const XYZW& a, const XYZW& b)
{
	return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);
}

inline XYZW operator + (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x+b.x;	c.y = a.y+b.y;	c.z = a.z+b.z;	c.w = a.w+b.w;  return c;
}

inline XYZW operator - (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x-b.x;	c.y = a.y-b.y;	c.z = a.z-b.z;	c.w = a.w-b.w;  return c;
}

inline XYZW operator * (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x*b.x;	c.y = a.y*b.y;	c.z = a.z*b.z;	c.w = a.w*b.w;  return c;
}

inline XYZW operator / (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x/b.x;	c.y = a.y/b.y;	c.z = a.z/b.z;	c.w = a.w/b.w;  return c;
}

//XYZ -----------------------------------------
inline bool operator == (const XYZ& a, const XYZ& b)
{
	return (a.x==b.x && a.y==b.y && a.z==b.z);
}

inline XYZ operator + (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x+b.x;	c.y = a.y+b.y;	c.z = a.z+b.z;	return c;
}

inline XYZ operator - (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x-b.x;	c.y = a.y-b.y;	c.z = a.z-b.z;	return c;
}

inline XYZ operator * (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x*b.x;	c.y = a.y*b.y;	c.z = a.z*b.z;	return c;
}

inline XYZ operator / (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x/b.x;	c.y = a.y/b.y;	c.z = a.z/b.z;	return c;
}

inline float dot(const XYZ& a, const XYZ& b)
{
	return (a.x*b.x + a.y*b.y + a.z*b.z);
}

inline XYZ cross(const XYZ& a, const XYZ& b)
{
	XYZ c;
	c.x = a.y*b.z - a.z*b.y;
	c.y = a.z*b.x - a.x*b.z;
	c.z = a.x*b.y - a.y*b.x;
	return c;
}

inline float norm(const XYZ& a)
{
	//return sqrt(a.x*a.x + a.y*a.y +a.z*a.z);
	return sqrt(dot(a,a));
}

inline float dist_L2(const XYZ& a, const XYZ& b)
{
	XYZ c(a.x-b.x, a.y-b.y, a.z-b.z);
	return sqrt(dot(c,c));
}

inline XYZ& normalize(XYZ& a)
{
	//float m = sqrt(a.x*a.x + a.y*a.y +a.z*a.z);
	float m = norm(a);
	if (m>0) {a.x /= m;		a.y /= m;		a.z /= m;}
	return a;
}

inline float min(const XYZ& a)
{
	return MIN(a.x, MIN(a.y, a.z));
}

inline float max(const XYZ& a)
{
	return MAX(a.x, MAX(a.y, a.z));
}

//random color -----------------------------------------------
inline RGB8 random_rgb8()
{
	XYZ d (rand()%255, rand()%255, rand()%255);
	normalize(d);
	RGB8 c;
	c.r=(unsigned char)(d.x*255);
	c.g=(unsigned char)(d.y*255);
	c.b=(unsigned char)(d.z*255);
	return c;
}

inline RGBA8 random_rgba8()
{
	RGB8 c=random_rgb8();
	RGBA8 cc;
	cc.r=c.r; cc.g=c.g; cc.b=c.b; cc.a=rand()%255;
	return cc;
}  // Caution: a==rand, 081114,0812210

inline RGBA8 random_rgba8(unsigned char a)
{
	RGB8 c=random_rgb8();
	RGBA8 cc;
	cc.r=c.r; cc.g=c.g; cc.b=c.b; cc.a=(a);
	return cc;
}


////////////////////////////////////////////////////////////////////

#endif

