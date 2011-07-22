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

RGB8 lookup_mix(vector<unsigned char>& mC, vector< vector<RGBA8> >& mLut, int op)
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

	RGB8 oC;
	oC.r = o1*255;
	oC.g = o2*255;
	oC.b = o3*255;
	return oC;
}

//////////////////////////////////////////////////////////////////////

void ChannelTable::connectSignals(XFormWidget* form)
{

}


//////////////////////////////////////////////////////////////////////

void ChannelTable::create()
{
	Channel ch;
	listChannel.clear();
	ch.n = 1; ch.color = XYZW(255,0,0,255); listChannel << ch;
	ch.n = 2; ch.color = XYZW(0,255,0,255); listChannel << ch;
	ch.n = 3; ch.color = XYZW(0,0,255,255); listChannel << ch;
	ch.n = 4; ch.color = XYZW(255,255,255,255); listChannel << ch;

	/////////////////////////////////////////////////////////
	tabOptions = new QTabWidget(this); //AutoTabWidget(this);
	/////////////////////////////////////////////////////////
	createTable();

	QVBoxLayout *allLayout = new QVBoxLayout(this);
	allLayout->addWidget(tabOptions);
	allLayout->setContentsMargins(0,0,0,0); //remove margins

	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
	this->setFixedHeight(180);
}

void ChannelTable::createTable()
{
	table = createTableChannel();

	QWidget* box = new QWidget();
	QGridLayout* boxlayout = new QGridLayout(box);
	boxlayout->addWidget(table,						1,0, 5,15);
	boxlayout->addWidget(new QRadioButton("Max"),	1,15, 1,5);
	boxlayout->addWidget(new QRadioButton("Sum"),	2,15, 1,5);
	boxlayout->addWidget(new QRadioButton("Mean"),	3,15, 1,5);
	boxlayout->addWidget(new QRadioButton("Index"),	4,15, 1,5);
	boxlayout->addWidget(new QCheckBox("0~255"), 5,15, 1,5);
	boxlayout->setContentsMargins(0,0,0,0); //remove margins

	if (tabOptions)
	{
		int i;
		QString qs;
		i= tabOptions->addTab(box,		qs =QString("Channel (%1)").arg(table->rowCount()));
		tabOptions->setTabToolTip(i, qs);
	}

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
}

QTableWidget* ChannelTable::currentTableWidget()
{
	if (! tabOptions) return 0;

	int k = 1 + (tabOptions->currentIndex());

	return table;
}

void ChannelTable::updatedContent(QTableWidget* t) //090826
{
//	if (! in_batch_stack.empty() && in_batch_stack.last()==true) return; //skip until end_batch

	t->resizeColumnsToContents();
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
					*actRed=0, *actGreen=0, *actBlue=0, *actGray=0, *actOff=0;

			menu.addAction(actRed 	= new QAction(tr("Red"), this));
		    menu.addAction(actGreen = new QAction(tr("Green"), this));
		    menu.addAction(actBlue 	= new QAction(tr("Blue"), this));
		    menu.addAction(actGray 	= new QAction(tr("Gray"), this));
		    menu.addAction(actOff 	= new QAction(tr("Off"), this));

		    act = menu.exec(QCursor::pos());

		    curItem = t->item(i,0); //color

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
		    else if (act==actOff)
		    {
		    	curItem->setData(0, qVariantFromValue(QColor(0,0,0,0))); //also alpha=0
		    }
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

		ADD_STRING( tr("chan %1").arg(listChannel[i].n) );

		assert(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
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
