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
 * channelTable.cpp
 *
 *  Created on: Jul 18, 2011
 *      Author: ruanz
 */

#include <assert.h>
#include "ChannelTable.h"
#include "../3drenderer/v3dr_common.h" //for v3dr_getColorDialog()


void make_linear_lut_one(RGBA8 color, vector<RGBA8>& lut)
{
	assert(lut.size()==256); //////// must be
	for (int j=0; j<256; j++)
	{
		float f = j/256.0;
		lut[j].r = color.r *f;
		lut[j].g = color.g *f;
		lut[j].b = color.b *f;
		lut[j].a = color.a;   //only alpha is constant
	}
}

void make_linear_lut(vector<RGBA8>& colors, vector< vector<RGBA8> >& luts)
{
	int N = colors.size();
	for (int k=0; k<N; k++)
	{
		make_linear_lut_one(colors[k], luts[k]);
	}
}

RGB8 lookup_mix(vector<unsigned char>& mC, vector< vector<RGBA8> >& mLut, int op, RGB8 mask)
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
	// OP_INDEX ignored

	o1 = CLAMP(0, 1, o1);
	o2 = CLAMP(0, 1, o2);
	o3 = CLAMP(0, 1, o3);

	RGB8 oC;
	oC.r = o1*255;
	oC.g = o2*255;
	oC.b = o3*255;
	oC.r &= mask.r;
	oC.g &= mask.g;
	oC.b &= mask.b;
	return oC;
}

//////////////////////////////////////////////////////////////////////

void ChannelTable::updateXFormWidget(int plane) // plane<=0 for all planes
{
	qDebug("ChannelTable::updateXFormWidget( %d )", plane);

	if (! xform) return;
	My4DImage* img4d = xform->getImageData();
	if (! img4d)
	{
		qDebug("ChannelTable::updateXFormWidget: no image data now.");
		return;
	}
    ImagePixelType dtype;
  	unsigned char **** p4d = (unsigned char ****)img4d->getData(dtype);
	if (!p4d)
	{
		qDebug("ChannelTable::updateXFormWidget: no image data pointer now.");
		return;
	}

	if (xform->colorMapRadioButton())
		xform->colorMapRadioButton()->setChecked(mixOp.op==OP_INDEX);
	if (mixOp.op==OP_INDEX)
	{
		xform->setColorMapDispType();
		return; // do old path of code
	}


	QPixmap pxm;
//#define P4D(img4d)  ((dtype==V3D_UINT8)? img4d->data4d_uint8 : \
//					(dtype==V3D_UINT16)? img4d->data4d_uint16 : \
//					(dtype==V3D_FLOAT32)? img4d->data4d_float32 : img4d->data4d_virtual)
#define COPY_mixChannel_plane( X, p4d ) {\
	pxm = copyRaw2QPixmap_Slice( \
			listChannel, \
			mixOp, \
			imgPlane##X, \
			img4d->getFocus##X(), \
			img4d->p4d, \
			img4d->getXDim(), \
			img4d->getYDim(), \
			img4d->getZDim(), \
			img4d->getCDim(), \
			img4d->p_vmax, \
			img4d->p_vmin); \
	xform->mixChannelColorPlane##X(pxm); \
}

	switch (dtype)
	{
	case V3D_UINT8:
		if (plane<=0 || plane==imgPlaneX)  COPY_mixChannel_plane( X, data4d_uint8 );
		if (plane<=0 || plane==imgPlaneY)  COPY_mixChannel_plane( Y, data4d_uint8 );
		if (plane<=0 || plane==imgPlaneZ)  COPY_mixChannel_plane( Z, data4d_uint8 );
		break;
	case V3D_UINT16:
		if (plane<=0 || plane==imgPlaneX)  COPY_mixChannel_plane( X, data4d_uint16 );
		if (plane<=0 || plane==imgPlaneY)  COPY_mixChannel_plane( Y, data4d_uint16 );
		if (plane<=0 || plane==imgPlaneZ)  COPY_mixChannel_plane( Z, data4d_uint16 );
		break;
	case V3D_FLOAT32:
		if (plane<=0 || plane==imgPlaneX)  COPY_mixChannel_plane( X, data4d_float32 );
		if (plane<=0 || plane==imgPlaneY)  COPY_mixChannel_plane( Y, data4d_float32 );
		if (plane<=0 || plane==imgPlaneZ)  COPY_mixChannel_plane( Z, data4d_float32 );
		break;
	default:
		break;
	}
}

void ChannelTable::setChannelColorDefault(int N)
{
	listChannel.clear();
	for (int i=0; i<N; i++)
	{
		Channel ch;
		ch.n = 1+i;
		if (ch.n==1)	ch.color = (N>1)? XYZW(255,0,0,255) : XYZW(255,255,255,255);
		else if (ch.n==2)	ch.color = XYZW(0,255,0,255);
		else if (ch.n==3)	ch.color = XYZW(0,0,255,255);
		else if (ch.n==4)	ch.color = XYZW(255,255,255,255);
		else				ch.color = random_rgba8(255);
		listChannel << ch;

		qDebug(" listChannel #%d (%d %d %d %d)", ch.n, ch.color.r,ch.color.g,ch.color.b,ch.color.a);
	}
}

void ChannelTable::linkXFormWidgetChannel()
{
	qDebug("ChannelTable::linkXFormWidgetChannel");

	if (! xform) return;
	My4DImage* img4d = xform->getImageData();
	if (! img4d)
	{
		qDebug(" no image data now.");
		return;
	}
	int N = img4d->getCDim();
	qDebug(" CDim = %d", N);

	setChannelColorDefault(N);

	/////////////////////////////////////////////////////////
	// for debug
	//	listChannel.clear();
	//	Channel ch;
	//	ch.n = 1; ch.color = XYZW(255,0,0,255); listChannel << ch;
	//	ch.n = 2; ch.color = XYZW(0,255,0,255); listChannel << ch;
	//	ch.n = 3; ch.color = XYZW(0,0,255,255); listChannel << ch;
	//	ch.n = 4; ch.color = XYZW(255,255,255,255); listChannel << ch;

	createNewTable();

}

//////////////////////////////////////////////////////////////////////

void ChannelTable::createFirst()
{
	tabOptions = new QTabWidget(this); //AutoTabWidget(this);

	createNewTable(); //must do it


	QVBoxLayout *allLayout = new QVBoxLayout(this);
	allLayout->addWidget(tabOptions);
	allLayout->setContentsMargins(0,0,0,0); //remove margins

	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
	this->setFixedHeight(180);
}

void ChannelTable::createNewTable()
{
	if (boxWidget) //delete whole box Tab include all sub widget
	{
		QWidget* old = boxWidget;
		old->deleteLater();
	}
	//so need re-create all sub widget again

	table = createTableChannel();

	radioButton_Max = new QRadioButton("Max");
	radioButton_Sum = new QRadioButton("Sum");
	radioButton_Mean = new QRadioButton("Mean");
	radioButton_Index = new QRadioButton("Index");
	checkBox_Rescale = new QCheckBox("0~255");
	checkBox_R = new QCheckBox("R");
	checkBox_G = new QCheckBox("G");
	checkBox_B = new QCheckBox("B");
	pushButton_Reset = new QPushButton("Reset");

	QWidget* box = new QWidget();
	QGridLayout* boxlayout = new QGridLayout(box);
	QGridLayout* oplayout = new QGridLayout();
	oplayout->addWidget(radioButton_Max,	1,0, 1,4);
	oplayout->addWidget(radioButton_Sum,	2,0, 1,4);
	oplayout->addWidget(radioButton_Mean,	3,0, 1,4);
	oplayout->setRowStretch(4, 1);
	oplayout->addWidget(radioButton_Index,	5,0, 1,4);
	boxlayout->addWidget(table,				1,0, 5,16); //at least need a empty table
	boxlayout->addLayout(oplayout, 1,16, 5,4);
	boxlayout->addWidget(checkBox_Rescale,		6,10, 1,5);
	boxlayout->addWidget(checkBox_R,		6,0, 1,3);
	boxlayout->addWidget(checkBox_G,		6,3, 1,3);
	boxlayout->addWidget(checkBox_B,		6,6, 1,3);
	boxlayout->addWidget(pushButton_Reset,		6,15, 1,5);
	boxlayout->setContentsMargins(0,0,0,0); //remove margins
	boxWidget = box;
	boxLayout = boxlayout; //for replacing new table in layout

	if (tabOptions)   tabOptions->clear();
	if (tabOptions)
	{
		int i;
		QString qs;
		i= tabOptions->addTab(box,		qs =QString("Channels (%1)").arg(table->rowCount()));
		tabOptions->setTabToolTip(i, qs);
	}

	//	if (table)
	//	{
	//		if (boxLayout) boxLayout->removeWidget(table);
	//		QTableWidget* old = table;
	//		old->deleteLater();  // deleteLater => postEvent(DeferredDelete) => qDeleteInEventHandler()
	//	}
	//	//replace with new table in layout
	//	table = createTableChannel();
	//	if (boxLayout)  boxLayout->addWidget(table,		1,0, 5,15);

	//connect cell to table handler
	if (table)      connect(table, SIGNAL(cellChanged(int,int)), this, SLOT(pickChannel(int,int)));
	if (table)
	{
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
	//		table->setEditTriggers(//QAbstractItemView::CurrentChanged |
	//				QAbstractItemView::DoubleClicked |
	//				QAbstractItemView::SelectedClicked);                       //use doubleClickHandler() to override delay of popping dialog by the setEditTriggers

		connect(table, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(doubleClickHandler(int,int))); //to override delay of popping dialog by the setEditTriggers
		connect(table, SIGNAL(cellPressed(int,int)), this, SLOT(pressedClickHandler(int,int)));      //to pop context menu
	}

	setMixOpControls();
	connectMixOpSignals();
}

void ChannelTable::setMixOpControls()
{
    radioButton_Max->setChecked(mixOp.op==OP_MAX);
    radioButton_Sum->setChecked(mixOp.op==OP_SUM);
    radioButton_Mean->setChecked(mixOp.op==OP_MEAN);
    radioButton_Index->setChecked(mixOp.op==OP_INDEX);

    radioButton_Index->setEnabled(listChannel.size()==1);

    checkBox_Rescale->setChecked(mixOp.rescale);
    checkBox_R->setChecked(mixOp.maskR);
    checkBox_G->setChecked(mixOp.maskG);
    checkBox_B->setChecked(mixOp.maskB);

}

void ChannelTable::connectMixOpSignals()
{
    connect(radioButton_Max, SIGNAL(clicked()), this, SLOT(setMixOpMax()));
    connect(radioButton_Sum, SIGNAL(clicked()), this, SLOT(setMixOpSum()));
    connect(radioButton_Mean, SIGNAL(clicked()), this, SLOT(setMixOpMean()));
    connect(radioButton_Index, SIGNAL(clicked()), this, SLOT(setMixOpIndex()));

    connect(checkBox_Rescale, SIGNAL(clicked()), this, SLOT(setMixRescale()));
    connect(checkBox_R, SIGNAL(clicked()), this, SLOT(setMixMaskR()));
    connect(checkBox_G, SIGNAL(clicked()), this, SLOT(setMixMaskG()));
    connect(checkBox_B, SIGNAL(clicked()), this, SLOT(setMixMaskB()));

    connect(pushButton_Reset, SIGNAL(clicked()), this, SLOT(setChannelMixDefault()));
}

void ChannelTable::setMixOpMax()
{
	bool b = radioButton_Max->isChecked();
	if (b)
	{
		mixOp.op = OP_MAX;
	    radioButton_Max->setChecked(mixOp.op==OP_MAX);
	    radioButton_Sum->setChecked(mixOp.op==OP_SUM);
	    radioButton_Mean->setChecked(mixOp.op==OP_MEAN);
	    radioButton_Index->setChecked(mixOp.op==OP_INDEX);
		emit channelTableChanged();
		checkBox_R->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_G->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_B->setEnabled(mixOp.op!=OP_INDEX);
	}
}
void ChannelTable::setMixOpSum()
{
	bool b = radioButton_Sum->isChecked();
	if (b)
	{
		mixOp.op = OP_SUM;
	    radioButton_Max->setChecked(mixOp.op==OP_MAX);
	    radioButton_Sum->setChecked(mixOp.op==OP_SUM);
	    radioButton_Mean->setChecked(mixOp.op==OP_MEAN);
	    radioButton_Index->setChecked(mixOp.op==OP_INDEX);
		emit channelTableChanged();
		checkBox_R->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_G->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_B->setEnabled(mixOp.op!=OP_INDEX);
	}
}
void ChannelTable::setMixOpMean()
{
	bool b = radioButton_Mean->isChecked();
	if (b)
	{
		mixOp.op = OP_MEAN;
	    radioButton_Max->setChecked(mixOp.op==OP_MAX);
	    radioButton_Sum->setChecked(mixOp.op==OP_SUM);
	    radioButton_Mean->setChecked(mixOp.op==OP_MEAN);
	    radioButton_Index->setChecked(mixOp.op==OP_INDEX);
		emit channelTableChanged();
		checkBox_R->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_G->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_B->setEnabled(mixOp.op!=OP_INDEX);
	}
}
void ChannelTable::setMixOpIndex()
{
	bool b = radioButton_Index->isChecked();
	if (b)
	{
		mixOp.op = OP_INDEX;
	    radioButton_Max->setChecked(mixOp.op==OP_MAX);
	    radioButton_Sum->setChecked(mixOp.op==OP_SUM);
	    radioButton_Mean->setChecked(mixOp.op==OP_MEAN);
	    radioButton_Index->setChecked(mixOp.op==OP_INDEX);
		emit channelTableChanged();
		checkBox_R->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_G->setEnabled(mixOp.op!=OP_INDEX);
		checkBox_B->setEnabled(mixOp.op!=OP_INDEX);
	}
}
void ChannelTable::setMixRescale()
{
	mixOp.rescale = checkBox_Rescale->isChecked();
	emit channelTableChanged();
}
void ChannelTable::setMixMaskR()
{
	mixOp.maskR = checkBox_R->isChecked();
	emit channelTableChanged();
}
void ChannelTable::setMixMaskG()
{
	mixOp.maskG = checkBox_G->isChecked();
	emit channelTableChanged();
}
void ChannelTable::setMixMaskB()
{
	mixOp.maskB = checkBox_B->isChecked();
	emit channelTableChanged();
}

void ChannelTable::setChannelMixDefault()
{
	setChannelColorDefault(listChannel.size());
	updateTableChannel();

	mixOp = MixOp();
	setMixOpControls();

	emit channelTableChanged();
}

#define __table_operations__

void ChannelTable::updatedContent(QTableWidget* t) //090826
{
	if (! in_batch_stack.empty() && in_batch_stack.last()==true) return; //skip until end_batch

	emit channelTableChanged();
	t->resizeColumnsToContents();
}

QTableWidget* ChannelTable::currentTableWidget()
{
	if (! tabOptions) return 0;

	int k = 1 + (tabOptions->currentIndex());

	return table;
}

inline QColor QColorFromRGBA8(RGBA8 c)
{
	return QColor(c.c[0], c.c[1], c.c[2], c.c[3]);
}

inline RGBA8 RGBA8FromQColor(QColor qc)
{
	RGBA8 c;
	c.r=qc.red(); c.g=qc.green(); c.b=qc.blue(); c.a=qc.alpha();
	return c;
}

#define QCOLOR(rgba8)   QColorFromRGBA8( rgba8 )
#define VCOLOR(rgba8)   qVariantFromValue(QColorFromRGBA8( rgba8 ))
#define QCOLORV(var)    (qVariantValue<QColor>( var ))
#define RGBA8V(var)     RGBA8FromQColor(qVariantValue<QColor>( var ))

#define UPATE_ITEM_ICON(curItem)   curItem->setData(Qt::DecorationRole, curItem->data(0))


void ChannelTable::setItemEditor()
{
	//::setItemEditor();

	// turn off item editor
	QItemEditorFactory::setDefaultFactory( new QItemEditorFactory(*QItemEditorFactory::defaultFactory()) );
}

void ChannelTable::pressedClickHandler(int i, int j)
{
	if (QApplication::mouseButtons()==Qt::RightButton) //right button menu
	{
		QTableWidget* t = currentTableWidget();
		QTableWidgetItem *curItem = t->item(i,j);

		if (t==table)
		{
			QMenu menu;
			QAction *act=0,
					*actRed=0, *actGreen=0, *actBlue=0, *actGray=0, *actBlank=0;

			menu.addAction(actRed 	= new QAction(tr("Red"), this));
		    menu.addAction(actGreen = new QAction(tr("Green"), this));
		    menu.addAction(actBlue 	= new QAction(tr("Blue"), this));
		    menu.addAction(actGray 	= new QAction(tr("Gray  (r=g=b=255)"), this));
		    menu.addAction(actBlank = new QAction(tr("Blank (r=g=b=a=0)"), this));

		    act = menu.exec(QCursor::pos());

			begin_batch();
			int n_row = t->rowCount();
			for (int ii=0; ii<n_row; ii++)
			{
				curItem = t->item(ii,0); //color
				if (! curItem->isSelected()) continue; // skip un-selected

				if (act==actRed)
				{
					curItem->setData(0, qVariantFromValue(QColor(255,0,0)));
				}
				else if (act==actGreen)
				{
					curItem->setData(0, qVariantFromValue(QColor(0,255,0)));
				}
				else if (act==actBlue)
				{
					curItem->setData(0, qVariantFromValue(QColor(0,0,255)));
				}
				else if (act==actGray)
				{
					curItem->setData(0, qVariantFromValue(QColor(255,255,255)));
				}
				else if (act==actBlank)
				{
					curItem->setData(0, qVariantFromValue(QColor(0,0,0,0))); //also alpha=0
				}
			}
		    end_batch();
			updatedContent(t);
		}
	}
}

void ChannelTable::doubleClickHandler(int i, int j)
{
	//qDebug("	doubleClickHandler( %d, %d )", i,j);

	QTableWidget* t = currentTableWidget();
	QTableWidgetItem *curItem = t->item(i,j);

	if (j==0) // color
	{
		QColor qcolor = QCOLORV(curItem->data(0));
		if (! v3dr_getColorDialog( &qcolor))  return;
		curItem->setData(0, qVariantFromValue(qcolor));
	}
}


#define ADD_ONOFF(b)	{curItem = new QTableWidgetItem();	t->setItem(i, j++, curItem); \
						curItem->setCheckState(BOOL_TO_CHECKED(b));}

#define ADD_QCOLOR(c)	{curItem = new QTableWidgetItem(QVariant::Color);	t->setItem(i, j++, curItem); \
						curItem->setData(0, VCOLOR(c)); \
						curItem->setData(Qt::DecorationRole, VCOLOR(c));}

#define ADD_STRING(s)	{curItem = new QTableWidgetItem(s);	t->setItem(i, j++, curItem);}


QTableWidget*  ChannelTable::createTableChannel()
{
	QStringList qsl;
	qsl <<"color" << "channel";
	int row = listChannel.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	//t->setHorizontalHeaderLabels(qsl);
	t->horizontalHeader()->hide();

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;

		ADD_QCOLOR(listChannel[i].color);

		ADD_STRING( tr("ch %1").arg(listChannel[i].n) );

		assert(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void ChannelTable::updateTableChannel()
{
	if (! table) return;
	QTableWidget* t = table;
	QTableWidgetItem *curItem;

	begin_batch();
	int n_row = MIN(t->rowCount(), listChannel.size());
	for (int ii=0; ii<n_row; ii++)
	{
		curItem = t->item(ii,0); //color
		curItem->setData(0, VCOLOR(listChannel[ii].color) );
	}
	end_batch();

	t->resizeColumnsToContents();
	//updatedContent(t);
}

void ChannelTable::pickChannel(int i, int j)
{
	QTableWidget* t = table;
	QTableWidgetItem *curItem = t->item(i,j);

	switch(j)
	{
	case 0:
		listChannel[i].color = RGBA8V(curItem->data(0));
		UPATE_ITEM_ICON(curItem);
		break;
	}

	updatedContent(t);
}
