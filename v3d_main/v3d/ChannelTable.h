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



/*
 * channelTable.h
 *
 *  Created on: Jul 18, 2011
 *      Author: ruanz
 */

#ifndef CHANNELTABLE_H_
#define CHANNELTABLE_H_

#include <assert.h>
#include <QVector>

#if defined(USE_Qt5)
  #include <QtWidgets>
#else
  //#include <QtGui>
#endif
#include <QTableWidget>
#include "../basic_c_fun/color_xyz.h"
#include "v3d_core.h"

//#define QVector std::QVector

// lookup and mix multi-channel to RGBA8
#define OP_MAX	0
#define OP_SUM	1
#define OP_MEAN	2
#define OP_OIT	3  //Order Independent Transparency
#define OP_INDEX	-1

inline
void make_linear_lut_one(RGBA8 color, QVector<RGBA8>& lut)
{
	assert(lut.size()==256); //////// must be
	for (int j=0; j<256; j++)
	{
		float f = j/255.0;  //110801 fixed the bug of max=254
		lut[j].r = color.r *f;
		lut[j].g = color.g *f;
		lut[j].b = color.b *f;
		lut[j].a = color.a;   //only alpha is constant
	}
}
inline
void make_linear_lut(QVector<RGBA8>& colors, QVector< QVector<RGBA8> >& luts)
{
	int N = colors.size();
	assert(N <= luts.size());
	for (int k=0; k<N; k++)
	{
		make_linear_lut_one(colors[k], luts[k]);
	}
}
inline
RGB8 lookup_mix(QVector<unsigned char>& mC, QVector< QVector<RGBA8> >& mLut, int op, RGB8 mask=XYZ(255,255,255))
{
#define R(k) (mLut[k][ mC[k] ].r /255.0)
#define G(k) (mLut[k][ mC[k] ].g /255.0)
#define B(k) (mLut[k][ mC[k] ].b /255.0)
#define A(k) (mLut[k][ mC[k] ].a /255.0)
#define AR(k) (A(k)*R(k))
#define AG(k) (A(k)*G(k))
#define AB(k) (A(k)*B(k))

	int N = mC.size();
	assert(N <= mLut.size());

	float o1,o2,o3;
	o1=o2=o3=0; //must be

	if (op==OP_MAX)
	{
		for (int k=0; k<N; k++)
		{
			o1 = MAX(o1, AR(k));
			o2 = MAX(o2, AG(k));
			o3 = MAX(o3, AB(k));
		}
	}
	else if (op==OP_SUM)
	{
		for (int k=0; k<N; k++)
		{
			o1 += AR(k);
			o2 += AG(k);
			o3 += AB(k);
		}
	}
	else if (op==OP_MEAN)
	{
		for (int k=0; k<N; k++)
		{
			o1 += AR(k);
			o2 += AG(k);
			o3 += AB(k);
		}
		o1 /= N;
		o2 /= N;
		o3 /= N;
	}
	else if (op==OP_OIT)
	{
		float avg_1,avg_2,avg_3, avg_a1,avg_a2,avg_a3, avg_a;
		avg_1=avg_2=avg_3 =avg_a1=avg_a2=avg_a3 =avg_a = 0;
		for (int k=0; k<N; k++)
		{
			o1 = AR(k);
			o2 = AG(k);
			o3 = AB(k);
//			avg_a1 += o1;
//			avg_a2 += o2;
//			avg_a3 += o3;
//			avg_1 += o1*o1;
//			avg_2 += o2*o2;
//			avg_3 += o3*o3;
			float a = MAX(o1, MAX(o2, o3));
						//(o1+o2+o3)/3;
			avg_a += a;
			avg_1 += o1 *a;
			avg_2 += o2 *a;
			avg_3 += o3 *a;
		}
		//avg_alpha
//		avg_a1 /=N;
//		avg_a2 /=N;
//		avg_a3 /=N;
		avg_a /=N;	avg_a1=avg_a2=avg_a3= avg_a;

		//avg_color
		avg_1 /=N;
		avg_2 /=N;
		avg_3 /=N;
		//(1-avg_alpha)^n
		float bg_a1 = pow(1-avg_a1, N);
		float bg_a2 = pow(1-avg_a2, N);
		float bg_a3 = pow(1-avg_a3, N);
		float bg_color = 1;
						//0.5;

		// dst_color = avg_color * (1-(1-avg_alpha)^n) + bg_color * (1-avg_alpha)^n
		o1 = avg_1*(1-bg_a1) + bg_color*bg_a1;
		o2 = avg_2*(1-bg_a2) + bg_color*bg_a2;
		o3 = avg_3*(1-bg_a3) + bg_color*bg_a3;
	}
	// OP_INDEX ignored

	o1 = CLAMP(0, 1, o1);
	o2 = CLAMP(0, 1, o2);
	o3 = CLAMP(0, 1, o3);

	RGB8 oC;
	oC.r = o1*255.0 +0.5;
	oC.g = o2*255.0 +0.5;
	oC.b = o3*255.0 +0.5;
	oC.r &= mask.r;
	oC.g &= mask.g;
	oC.b &= mask.b;
	return oC;

#undef R(k)
#undef G(k)
#undef B(k)
#undef A(k)
#undef AR(k)
#undef AG(k)
#undef AB(k)
}


template <class T> QPixmap copyRaw2QPixmap_Slice( //test function for 4 channels
		ImagePlaneDisplayType cplane,
		V3DLONG cpos,
		const T **** p4d,
		V3DLONG sz0,
		V3DLONG sz1,
		V3DLONG sz2,
		V3DLONG sz3,
		ImageDisplayColorType Ctype,
		bool bRescale,
		double *p_vmax,
		double *p_vmin)
{

	V3DLONG x,y,z,pp;

	int N = MIN(sz3, 4); ////////////////

	QVector<float> vrange(N);
	for (int k=0; k<N; k++)
	{
		vrange[k] = p_vmax[k]-p_vmin[k];
		if (vrange[k]<=0) vrange[k] = 1;
	}

	//qDebug()<<"copyRaw2QPixmap_Slice switch (Ctype)"<<Ctype;

	//set lookup-table
	QVector< QVector<RGBA8> > luts(4);
	QVector<RGBA8> lut(256);
	RGBA8 _Red		= XYZW(255,0,0,255);
	RGBA8 _Green	= XYZW(0,255,0,255);
	RGBA8 _Blue		= XYZW(0,0,255,255);
	RGBA8 _Gray		= XYZW(255,255,255,255);
	RGBA8 _Blank	= XYZW(0,0,0,0);
	switch (Ctype)
	{
		case colorGray: //070716
			make_linear_lut_one( _Gray, lut );
			luts[0] = lut;
			luts[1] = lut;
			luts[2] = lut;
			luts[3] = lut;
			break;

		case colorRedOnly:
			make_linear_lut_one( _Red, lut );
			luts[0] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[1] = lut;
			luts[2] = lut;
			luts[3] = lut;
			break;

		case colorRed2Gray:
			make_linear_lut_one( _Gray, lut );
			luts[0] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[1] = lut;
			luts[2] = lut;
			luts[3] = lut;
			break;

		case colorGreenOnly:
			make_linear_lut_one( _Green, lut );
			luts[1] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[0] = lut;
			luts[2] = lut;
			luts[3] = lut;
			break;

		case colorGreen2Gray:
			make_linear_lut_one( _Gray, lut );
			luts[1] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[0] = lut;
			luts[2] = lut;
			luts[3] = lut;
			break;

		case colorBlueOnly:
			make_linear_lut_one( _Blue, lut );
			luts[2] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[0] = lut;
			luts[1] = lut;
			luts[3] = lut;
			break;

		case colorBlue2Gray:
			make_linear_lut_one( _Gray, lut );
			luts[2] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[0] = lut;
			luts[1] = lut;
			luts[3] = lut;
			break;

		case colorRGB:
			make_linear_lut_one( _Red, lut );
			luts[0] = lut;
			make_linear_lut_one( _Green, lut );
			luts[1] = lut;
			make_linear_lut_one( _Blue, lut );
			luts[2] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[3] = lut;
			break;

		case colorRG:
			make_linear_lut_one( _Red, lut );
			luts[0] = lut;
			make_linear_lut_one( _Green, lut );
			luts[1] = lut;
			make_linear_lut_one( _Blank, lut );
			luts[2] = lut;
			luts[3] = lut;
			break;

		case colorUnknown:
		default:
			break;
	}

	// transfer N channel's pixels
	int op =  (Ctype == colorGray)? OP_MEAN : OP_MAX;
	QVector<unsigned char> mC(N);
	QImage tmpimg;

	//qDebug()<<"copyRaw2QPixmap_Slice switch (cplane)"<<cplane;

	switch (cplane) //QImage(w,h)
	{
	case imgPlaneX: //(Z,Y)
        pp = (cpos>=sz0)? sz0-1:cpos;   pp = (pp<0)? 0:pp;//20150207. PHC
		tmpimg = QImage(sz2, sz1, QImage::Format_RGB32);

		for (y=0; y<sz1; y++)
		for (z=0; z<sz2; z++)
			{
				for (int k=0; k<N; k++)
				{
					float C = p4d[k][z][y][pp];
					mC[k] =  ( (! bRescale) ? C : floor((C-p_vmin[k])/vrange[k]*255) );
				}
				RGB8 o = lookup_mix(mC, luts, op);
				tmpimg.setPixel(z, y, qRgb(o.r, o.g, o.b));
				//qDebug("(x) y z = (%d/%d) %d/%d %d/%d", pp,sz0, y,sz1, z,sz2);
			}
		break;

	case imgPlaneY: //(X,Z)
        pp = (cpos>=sz1)? sz1-1:cpos;   pp = (pp<0)? 0:pp; //20150207. PHC
		tmpimg = QImage(sz0, sz2, QImage::Format_RGB32);

		for (z=0; z<sz2; z++)
		for (x=0; x<sz0; x++)
			{
				for (int k=0; k<N; k++)
				{
					float C = p4d[k][z][pp][x];
					mC[k] =  ( (! bRescale) ? C : floor((C-p_vmin[k])/vrange[k]*255) );
				}
				RGB8 o = lookup_mix(mC, luts, op);
				tmpimg.setPixel(x, z, qRgb(o.r, o.g, o.b));
				//qDebug("x (y) z = %d/%d (%d/%d) %d/%d", x,sz0, pp,sz1, z,sz2);
			}
		break;

	case imgPlaneZ: //(X,Y)
        pp=cpos;//    v3d_msg(QString("cpos=%1").arg(cpos));
        pp = (pp>=sz2)? sz2-1:pp;   pp = (pp<0)? 0:pp;//20150207. PHC

        tmpimg = QImage(sz0, sz1, QImage::Format_RGB32);

		for (y=0; y<sz1; y++)
		for (x=0; x<sz0; x++)
			{
				for (int k=0; k<N; k++)
				{
					float C = p4d[k][pp][y][x];
					mC[k] =  ( (! bRescale) ? C : floor((C-p_vmin[k])/vrange[k]*255) );
				}
				RGB8 o = lookup_mix(mC, luts, op);
				tmpimg.setPixel(x, y, qRgb(o.r, o.g, o.b));
				//qDebug("x y (z) = %d/%d %d/%d (%d/%d)", x,sz0, y,sz1, pp,sz2);
			}
		break;
	}

	//qDebug()<<"copyRaw2QPixmap_Slice fromImage(tmpimg)"<<tmpimg.size();

	return QPixmap::fromImage(tmpimg);
}


struct MixOP
{
	int op;
	bool rescale;
	bool maskR, maskG, maskB;
	float brightness, contrast; //ratio
	MixOP() {op=OP_MAX;  rescale=true;
			maskR=maskG=maskB=true;
			brightness=0; contrast=1;}
};

struct Channel
{
	int n;				// index
	bool on;
	RGBA8 color;
	Channel() {n=0; on=true;  color.r=color.g=color.b=color.a=255;}
};


template <class T> QImage copyRaw2QImage_Slice( //real function include brightness/contrast
		const QList<Channel> & listChannel,
		const MixOP & mixOp,
		QVector< QVector<RGBA8> >* p_luts, //pre-computed external LUTs, if not presented set to 0
		ImagePlaneDisplayType cplane,
		V3DLONG cpos,
		T **** p4d,  //no const for using macro expansion
		V3DLONG sz0,
		V3DLONG sz1,
		V3DLONG sz2,
		V3DLONG sz3,
		double *p_vmax,
		double *p_vmin)
{

	V3DLONG x,y,z,pp;

	int N = MIN(sz3, listChannel.size()); ////////////////

	QVector<float> vrange(N);
	for (int k=0; k<N; k++)
	{
		vrange[k] = p_vmax[k]-p_vmin[k];
		if (vrange[k]<=0)  vrange[k] = 1;
	}

	//qDebug()<<"copyRaw2QPixmap_Slice switch (Ctype)"<<Ctype;

	//setup lookup-tables
	if (! p_luts)
	{
		QVector< QVector<RGBA8> > luts(N);
		QVector<RGBA8> lut(256);
		for (int k=0; k<N; k++)
		{
			make_linear_lut_one( listChannel[k].color, lut );
			luts[k] = lut;
		}
		p_luts = &luts;
	}

	// transfer N channel's pixels
	bool bRescale = mixOp.rescale;
	float fb = mixOp.brightness;
	float fc = mixOp.contrast;
	int op =  mixOp.op;
	RGB8 mask;
	mask.r = (mixOp.maskR)? 255:0;
	mask.g = (mixOp.maskG)? 255:0;
	mask.b = (mixOp.maskB)? 255:0;

	QVector<unsigned char> mC(N);
	QImage tmpimg;

	//qDebug()<<"copyRaw2QPixmap_Slice switch (cplane)"<<cplane;

#define BRIGHTEN_TRANSFORM( C ) \
	if (fc != 1 || fb != 0) \
	{ \
		C = C*(fc) + (fb*vrange[k]); \
		C = CLAMP(p_vmin[k], p_vmax[k], C); \
	}
#define SET_mC( k, C ) \
	if (listChannel[k].on) \
	{ \
		BRIGHTEN_TRANSFORM( C ); \
		mC[k] =  ( (! bRescale) ? floor(C +0.5) : floor((C-p_vmin[k])/vrange[k]*255.0 +0.5) ); \
	} \
	else \
	{ \
		mC[k] = 0; \
	}

	switch (cplane) //QImage(w,h)
	{
	case imgPlaneX: //(Z,Y)
        pp=cpos; pp = (cpos>=sz0)? sz0-1:pp;   pp = (pp<0)? 0:pp;//20150207. PHC
		tmpimg = QImage(sz2, sz1, QImage::Format_RGB32);

		for (y=0; y<sz1; y++)
		for (z=0; z<sz2; z++)
			{
				for (int k=0; k<N; k++)
				{
					float C = p4d[k][z][y][pp];
					SET_mC( k, C );
				}
				RGB8 o = lookup_mix(mC, *p_luts, op, mask);
				tmpimg.setPixel(z, y, qRgb(o.r, o.g, o.b));
				//qDebug("(x) y z = (%d/%d) %d/%d %d/%d", pp,sz0, y,sz1, z,sz2);
			}
		break;

	case imgPlaneY: //(X,Z)
        pp=cpos; pp = (pp>=sz1)? sz1-1:pp;   pp = (pp<0)? 0:pp; //20150207. PHC
		tmpimg = QImage(sz0, sz2, QImage::Format_RGB32);

		for (z=0; z<sz2; z++)
		for (x=0; x<sz0; x++)
			{
				for (int k=0; k<N; k++)
				{
					float C = p4d[k][z][pp][x];
					SET_mC( k, C );
				}
				RGB8 o = lookup_mix(mC, *p_luts, op, mask);
				tmpimg.setPixel(x, z, qRgb(o.r, o.g, o.b));
				//qDebug("x (y) z = %d/%d (%d/%d) %d/%d", x,sz0, pp,sz1, z,sz2);
			}
		break;

	case imgPlaneZ: //(X,Y)
        pp=cpos;   //v3d_msg(QString("cpos=%1").arg(cpos));
        pp = (pp>=sz2)? sz2-1:pp;   pp = (pp<0)? 0:pp; //20150207. PHC

		tmpimg = QImage(sz0, sz1, QImage::Format_RGB32);

		for (y=0; y<sz1; y++)
		for (x=0; x<sz0; x++)
			{
				for (int k=0; k<N; k++)
				{
					float C = p4d[k][pp][y][x];
					SET_mC( k, C );
				}
				RGB8 o = lookup_mix(mC, *p_luts, op, mask);
				tmpimg.setPixel(x, y, qRgb(o.r, o.g, o.b));
				//qDebug("x y (z) = %d/%d %d/%d (%d/%d)", x,sz0, y,sz1, pp,sz2);
			}
		break;
	}

#undef BRIGHTEN_TRANSFORM( C )
#undef SET_mC( k, C )

	//qDebug()<<"copyRaw2QPixmap_Slice fromImage(tmpimg)"<<tmpimg.size();

	return (tmpimg);
}

///////////////////////////////////////////////////////////////////////////////////////////
//widget for control channel's color

struct ChannelSharedData
{
	MixOP mixOp;
	QList<Channel> listChannel;
	QVector< QVector<RGBA8> > luts;
	bool bGlass;
};

class ChannelTable;
class BrightenBox;
class MiscBox;

class ChannelTabWidget : public QTabWidget //QWidget
{
    Q_OBJECT;
	XFormWidget* xform;
	int n_tabs;
	QTabWidget* tabOptions;
	ChannelTable *channelPage;
	BrightenBox *brightenPage;
	MiscBox *miscPage;
	ChannelSharedData csData; 		//shared with channelPage & brightenPage
	void createFirst();

public:
	ChannelTabWidget(XFormWidget* parent, int tabs=3, bool glass=false) :QTabWidget(parent)
	{
		xform = (XFormWidget*)parent;
		n_tabs = tabs;			csData.bGlass = (glass);//2 tabs for looking glass
		tabOptions = 0;
		channelPage = 0;
		brightenPage = 0;
		miscPage = 0;
		//TURNOFF_ITEM_EDITOR(); //replaced with table->setEditTriggers(QAbstractItemView::NoEditTriggers)
		createFirst();
	};
	virtual ~ChannelTabWidget() {};
	const ChannelSharedData & getChannelSharedData() {return csData;}
	ChannelTable* getChannelPage() { return (channelPage); }
	int channelCount();

public slots:
	void updateXFormWidget(int plane=-1);	//called by signal XFormWidget::colorChanged(int), or channelTableChanged & brightenChanged
	void linkXFormWidgetChannel();			//link updated channels of XFormWidget

	void syncOpControls(const MixOP & mixop);
	void syncSharedData(const ChannelSharedData & data);

};

/////////////////////////////////////////////

class ChannelTable : public QWidget
{
    Q_OBJECT;

	MixOP & mixOp; 					//just a reference to ChannelTabWidget::csData
	QList<Channel> & listChannel;	//just a reference to ChannelTabWidget::csData
	QVector< QVector<RGBA8> > & luts; //just a reference to ChannelTabWidget::csData
	bool & bGlass; 					//just a reference to ChannelTabWidget::csData
	XFormWidget* xform;
	ChannelTabWidget* ctab;

public:
	ChannelTable(ChannelSharedData& csd, XFormWidget* xform, QWidget* parent=0) :QWidget(parent)
		, mixOp(csd.mixOp)
		, listChannel(csd.listChannel)
		, luts(csd.luts)
		, bGlass(csd.bGlass)
		, xform(xform) /////
		, ctab(qobject_cast<ChannelTabWidget*>(parent))
	{
		init_member();
		linkXFormWidgetChannel();
		connect( this,SIGNAL(channelTableChanged()), ctab, SLOT(updateXFormWidget()) ); //connect ctab for ChannelTabWidget::syncSharedData
	};
	virtual ~ChannelTable() {};
	QTableWidget* getTable() {return table;};
	int rowCount() {return (table)? table->rowCount() :0; };

signals:
	void channelTableChanged(); //trigger to update XFormWidget

public slots:
	void updateXFormWidget(int plane=-1);	//called by ChannelTabWidget
	void linkXFormWidgetChannel();			//link updated channel
	void setChannelColorDefault(int N);
	void updateTableChannel(bool update_luts=true);
	void setRescaleDefault();
	void updateMixOpControls();

	void begin_batch() {in_batch_stack.push_back(true);}
	void end_batch()   {in_batch_stack.pop_back();}
	void updatedContent(QTableWidget* t);

protected slots:
	void pressedClickHandler(int row, int col);
	void doubleClickHandler(int row, int col);
	void pickChannel(int row, int col);

	void setMixOpMax();
	void setMixOpSum();
	void setMixOpMean();
	void setMixOpOIT();
	void setMixOpIndex();
	void setMixRescale();
	void setMixMaskR();
	void setMixMaskG();
	void setMixMaskB();
	void setDefault();

protected:
	QGridLayout *boxLayout;
	QTableWidget *table;
	QRadioButton *radioButton_Max, *radioButton_Sum, *radioButton_Mean, *radioButton_OIT, *radioButton_Index;
	QCheckBox *checkBox_Rescale, *checkBox_R, *checkBox_G, *checkBox_B;
	QPushButton *pushButton_Reset;
	void init_member()
	{
		boxLayout=0;
		table=0;
		radioButton_Max=radioButton_Sum=radioButton_Mean=radioButton_OIT=radioButton_Index=0;
		checkBox_Rescale=checkBox_R=checkBox_G=checkBox_B=0;
		pushButton_Reset=0;
	}

	void createNewTable();  // called by linkXFormWidgetChannel
	void connectMixOpSignals();  //called by createNewTable

	QVector<bool> in_batch_stack;
	QTableWidget* currentTableWidget();

	QTableWidget* createTableChannel();
	void updateLuts(int k=-1);

};

///////////////////////////////////////////////

class BrightenBox : public QWidget
{
    Q_OBJECT;

	MixOP & mixOp; 					//just a reference to ChannelTabWidget::csData
	XFormWidget* xform;

public:
	BrightenBox(ChannelSharedData& csd, XFormWidget* xform, QWidget* parent=0) :QWidget(parent)
		, mixOp(csd.mixOp)
		, xform(xform) /////
	{
		init_member();
		create();
		connect( this,SIGNAL(brightenChanged()), parent, SLOT(updateXFormWidget()) );
	};
	virtual ~BrightenBox() {};

signals:
	void brightenChanged(); //trigger to update XFormWidget

public slots:
	void updateMixOpControls();
	void reset();
	void setBrightness(int);
	void setContrast(int);

protected:
	QSlider *slider_bright, *slider_contrast;
	QSpinBox *spin_bright, *spin_contrast;
	QPushButton *push_reset;
	int _bright, _contrast; //for stopping loop of setValue()
	void init_member()
	{
		slider_bright=slider_contrast=0;
		spin_bright=spin_contrast=0;
		push_reset=0;
	}
	void create();
};

/////////////////////////////////////////////////

class MiscBox : public QWidget
{
    Q_OBJECT;

	MixOP & mixOp; 					//just a reference to ChannelTabWidget::csData
	QList<Channel> & listChannel;	//just a reference to ChannelTabWidget::csData
	QVector< QVector<RGBA8> > & luts; //just a reference to ChannelTabWidget::csData
	XFormWidget* xform;

public:
	MiscBox(ChannelSharedData& csd, XFormWidget* xform, QWidget* parent=0) :QWidget(parent)
		, mixOp(csd.mixOp)
		, listChannel(csd.listChannel)
		, luts(csd.luts)
		, xform(xform)
	{
		init_member();
		create();
	};
	virtual ~MiscBox() {};

//signals:
//	void signalExportRGBStack(); //connect( miscPage,SIGNAL(signalExportRGBStack()), channelPage, SLOT(exportRGBStack()) );
public slots:
	void exportRGBStack();

protected:
	QPushButton *push_export;
	void init_member()
	{
		push_export=0;
	}
	void create();
};

#endif /* CHANNELTABLE_H_ */
