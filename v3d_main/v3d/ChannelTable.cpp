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
#include "../3drenderer/v3dr_common.h" //for v3dr_getColorDialog()
#include "ChannelTable.h"


static int xformChannelDim(XFormWidget* xform)
{
	int N = 0;
	if ( xform)
	{
		My4DImage* img4d = xform->getImageData();
		if ( img4d)
		{
			N = img4d->getCDim();
		}
	}
	return N;
}
static int xformChannelUnitBytes(XFormWidget* xform)
{
	int N = 0;
	if ( xform)
	{
		My4DImage* img4d = xform->getImageData();
		if ( img4d)
		{
			N = img4d->getUnitBytes();
		}
	}
	return N;
}

//////////////////////////////////////////////////////////////////////
#define ___ChannelTabWidget___

int ChannelTabWidget::channelCount() { return (channelPage)? channelPage->rowCount() :0; }

void ChannelTabWidget::updateXFormWidget(int plane)
{
	if (channelPage)  channelPage->updateXFormWidget(plane);
	if (plane <0   //-1 means issued by channelPage
		&& !csData.bGlass //110808 except looking glass
		)
	{
		if (xform)  xform->syncChannelTabWidgets(this);
	}
}

void ChannelTabWidget::linkXFormWidgetChannel()
{
	if (xformChannelDim(xform) == this->channelCount()) //same length of channel list
		return; //110805 no need to re-create

	//delete whole box Tab include all sub widget
	if (channelPage)
	{
		QWidget* old = channelPage;
		old->deleteLater();
	}
	//so need to re-create all sub widget again

	channelPage = new ChannelTable(csData, xform, this); //call channelPage->linkXFormWidgetChannel();
	if (tabOptions)
	{
		int i;
		QString qs;
		i= tabOptions->insertTab(0, channelPage,	qs =tr("Channels (%1)").arg(channelPage->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		tabOptions->setCurrentIndex(0); ///////
	}
}

void ChannelTabWidget::createFirst()
{
	tabOptions = this; //new QTabWidget(this); //AutoTabWidget(this);
//	QVBoxLayout *allLayout = new QVBoxLayout(this);
//	allLayout->addWidget(tabOptions);
//	allLayout->setContentsMargins(0,0,0,0); //remove margins


	//create or re-create channelPage
	linkXFormWidgetChannel();
	if (n_tabs==1) return;

	brightenPage = new BrightenBox(csData, xform, this);
	if (tabOptions)
	{
		int i;
		QString qs;
		i= tabOptions->insertTab( 1, brightenPage,	qs =tr("Intensity"));
		tabOptions->setTabToolTip(i, tr("Intensity transform before LUT"));
		tabOptions->setCurrentIndex(0);/////
	}
	if (n_tabs==2) return;

	miscPage = new MiscBox(csData, xform, this);
	if (tabOptions)
	{
		int i;
		QString qs;
		i= tabOptions->insertTab( 2, miscPage,		qs =tr("Misc"));
		tabOptions->setTabToolTip(i, qs);
		tabOptions->setCurrentIndex(0);/////

		//connect( miscPage,SIGNAL(signalExportRGBStack()), channelPage, SLOT(exportRGBStack()) ); //doesn't work
	}
	if (n_tabs==3) return;

}

void ChannelTabWidget::syncOpControls(const MixOP & mixop)
{
	csData.mixOp = mixop;
	if (channelPage)  channelPage->updateMixOpControls();
	if (brightenPage)  brightenPage->updateMixOpControls();
}

void ChannelTabWidget::syncSharedData(const ChannelSharedData & data) //except bGlass
{
	//qDebug("ChannelTabWidget::syncSharedData");
	bool glass = csData.bGlass; //important, except bGlass
	csData = data;
	csData.bGlass = glass;

	if (channelPage) { channelPage->updateMixOpControls();  channelPage->updateTableChannel(false); }
	if (brightenPage)  brightenPage->updateMixOpControls();
}


//////////////////////////////////////////////////////////////////////
#define ___ChannelTable___

void ChannelTable::updateXFormWidget(int plane) // plane<=0 for all planes
{
	if (plane>0)
	{
        //qDebug("ChannelTable::updateXFormWidget( %d )", plane);
        printf("CTU_%i ", plane);
        fflush(stdout);
    }

	if (! xform) return;
	My4DImage* img4d = xform->getImageData();
	if (! img4d)
	{
		qDebug("ChannelTable::updateXFormWidget: no image data now.");
		return;
	}
    ImagePixelType dtype;
  	unsigned char **** p4d = (unsigned char ****)img4d->getData(dtype);
	if (! p4d)
	{
		qDebug("ChannelTable::updateXFormWidget: no image data pointer now.");
		return;
	}

	// do old code for OP_INDEX
	if (xform->colorMapRadioButton()) //enable XFormWidget::switchMaskColormap()
		xform->colorMapRadioButton()->setChecked(true);//mixOp.op==OP_INDEX);
	if (mixOp.op==OP_INDEX)
	{
		xform->setColorMapDispType(colorPseudoMaskColor, bGlass); //make XFormView::internal_only_imgplane_op to use old code
		return;  /////////
	}
	else
	{
		xform->setColorMapDispType(colorUnknown, bGlass); //110725, switch back to do new code
	}

	QImage slice;
//#define P4D(img4d)  ((dtype==V3D_UINT8)? img4d->data4d_uint8 : \
//					(dtype==V3D_UINT16)? img4d->data4d_uint16 : \
//					(dtype==V3D_FLOAT32)? img4d->data4d_float32 : img4d->data4d_virtual)
#define COPY_xform_mixChannel_plane( X, p4d ) {\
	slice = copyRaw2QImage_Slice( \
			listChannel, \
			mixOp, \
			&luts, \
			imgPlane##X, \
			img4d->getFocus##X(), \
			img4d->p4d, \
			img4d->getXDim(), \
			img4d->getYDim(), \
			img4d->getZDim(), \
			img4d->getCDim(), \
			img4d->p_vmax, \
			img4d->p_vmin); \
	xform->mixChannelColorPlane##X(QPixmap::fromImage(slice), bGlass); \
	}

	switch (dtype)
	{
	case V3D_UINT8:
		if (plane<=0 || plane==imgPlaneX)  COPY_xform_mixChannel_plane( X, data4d_uint8 );
		if (plane<=0 || plane==imgPlaneY)  COPY_xform_mixChannel_plane( Y, data4d_uint8 );
		if (plane<=0 || plane==imgPlaneZ)  COPY_xform_mixChannel_plane( Z, data4d_uint8 );
		break;
	case V3D_UINT16:
		if (plane<=0 || plane==imgPlaneX)  COPY_xform_mixChannel_plane( X, data4d_uint16 );
		if (plane<=0 || plane==imgPlaneY)  COPY_xform_mixChannel_plane( Y, data4d_uint16 );
		if (plane<=0 || plane==imgPlaneZ)  COPY_xform_mixChannel_plane( Z, data4d_uint16 );
		break;
	case V3D_FLOAT32:
		if (plane<=0 || plane==imgPlaneX)  COPY_xform_mixChannel_plane( X, data4d_float32 );
		if (plane<=0 || plane==imgPlaneY)  COPY_xform_mixChannel_plane( Y, data4d_float32 );
		if (plane<=0 || plane==imgPlaneZ)  COPY_xform_mixChannel_plane( Z, data4d_float32 );
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

		//qDebug(" listChannel #%d (%d %d %d %d)", ch.n, ch.color.r,ch.color.g,ch.color.b,ch.color.a);
	}
}

void ChannelTable::linkXFormWidgetChannel()
{
	int N = xformChannelDim(xform);
	//if (N>0)
	qDebug("ChannelTable::linkXFormWidgetChannel  CDim=%d  %s", N, ((bGlass)?"(glass)":""));

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


void ChannelTable::createNewTable()
{
	table = createTableChannel();
	table->setToolTip(tr("Right-click on row to pop color Menu.\n"
			"Double-click on color cell to pop color Dialog.\n"
			"Also you can do multi-selection."));
	table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	radioButton_Max = new QRadioButton("Max");
	radioButton_Sum = new QRadioButton("Sum");
	radioButton_Mean = new QRadioButton("Mean");
	radioButton_OIT = new QRadioButton("OIT");  	radioButton_OIT->setToolTip("Order Independent Transparency");
	radioButton_Index = new QRadioButton("Index");
	checkBox_Rescale = new QCheckBox("0~255");		checkBox_Rescale->setToolTip(tr("Re-scale intensity before LUT"));
	checkBox_R = new QCheckBox("R");		checkBox_R->setToolTip("Output Red");
	checkBox_G = new QCheckBox("G");		checkBox_G->setToolTip("Output Green");
	checkBox_B = new QCheckBox("B");		checkBox_B->setToolTip("Output Blue");
	pushButton_Reset = new QPushButton("Reset");

	QGridLayout* boxlayout = new QGridLayout(this);
	QGridLayout* oplayout = new QGridLayout();
	oplayout->addWidget(radioButton_Max,	1,0, 1,1);
	oplayout->addWidget(radioButton_Sum,	2,0, 1,1);
	oplayout->addWidget(radioButton_Mean,	3,0, 1,1);
	oplayout->addWidget(radioButton_OIT,	4,0, 1,1);
	oplayout->addWidget(radioButton_Index,	5,0, 1,1);

	const int nrow = 4;
	boxlayout->addLayout(oplayout, 			1,16, nrow,5);
	boxlayout->addWidget(table,				1,0, nrow,16); //at least need a empty table
	boxlayout->addWidget(checkBox_Rescale,	nrow+1,16, 1,5);
	boxlayout->addWidget(checkBox_R,		nrow+1,0, 1,3);
	boxlayout->addWidget(checkBox_G,		nrow+1,3, 1,3);
	boxlayout->addWidget(checkBox_B,		nrow+1,6, 1,3);
	boxlayout->addWidget(pushButton_Reset,	nrow+1,10, 1,5);
	boxlayout->setContentsMargins(0,0,0,0); //remove margins
	boxLayout = boxlayout; //for replacing new table in layout

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
		table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//		table->setEditTriggers(//QAbstractItemView::CurrentChanged |
	//				QAbstractItemView::DoubleClicked |
	//				QAbstractItemView::SelectedClicked);
		//use doubleClickHandler() to override delay of popping dialog by the setEditTriggers
		connect(table, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(doubleClickHandler(int,int))); //to override delay of popping dialog by the setEditTriggers
		connect(table, SIGNAL(cellPressed(int,int)), this, SLOT(pressedClickHandler(int,int)));      //to pop context menu
	}

	setRescaleDefault();
	updateMixOpControls();

	connectMixOpSignals();
}

void ChannelTable::connectMixOpSignals()
{
    connect(radioButton_Max, SIGNAL(clicked()), this, SLOT(setMixOpMax()));
    connect(radioButton_Sum, SIGNAL(clicked()), this, SLOT(setMixOpSum()));
    connect(radioButton_Mean, SIGNAL(clicked()), this, SLOT(setMixOpMean()));
    connect(radioButton_OIT, SIGNAL(clicked()), this, SLOT(setMixOpOIT()));
    connect(radioButton_Index, SIGNAL(clicked()), this, SLOT(setMixOpIndex()));
    connect(checkBox_Rescale, SIGNAL(clicked()), this, SLOT(setMixRescale()));
    connect(checkBox_R, SIGNAL(clicked()), this, SLOT(setMixMaskR()));
    connect(checkBox_G, SIGNAL(clicked()), this, SLOT(setMixMaskG()));
    connect(checkBox_B, SIGNAL(clicked()), this, SLOT(setMixMaskB()));

    connect(pushButton_Reset, SIGNAL(clicked()), this, SLOT(setDefault()));
}

void ChannelTable::setRescaleDefault()
{
	int N = xformChannelUnitBytes(xform);
    mixOp.rescale = (N>1);
	checkBox_Rescale->setChecked(mixOp.rescale);
}

#define UPDATE_OP_CONTROL() { \
	radioButton_Max->setChecked(mixOp.op==OP_MAX); \
	radioButton_Sum->setChecked(mixOp.op==OP_SUM); \
	radioButton_Mean->setChecked(mixOp.op==OP_MEAN); \
	radioButton_OIT->setChecked(mixOp.op==OP_OIT); \
	radioButton_Index->setChecked(mixOp.op==OP_INDEX); \
	\
	checkBox_R->setEnabled(mixOp.op!=OP_INDEX); \
	checkBox_G->setEnabled(mixOp.op!=OP_INDEX); \
	checkBox_B->setEnabled(mixOp.op!=OP_INDEX); \
	checkBox_Rescale->setEnabled(mixOp.op!=OP_INDEX); \
}

void ChannelTable::updateMixOpControls()
{
    UPDATE_OP_CONTROL();

    checkBox_R->setChecked(mixOp.maskR);
    checkBox_G->setChecked(mixOp.maskG);
    checkBox_B->setChecked(mixOp.maskB);
    checkBox_Rescale->setChecked(mixOp.rescale);

    radioButton_Index->setEnabled(listChannel.size()==1);
}

void ChannelTable::setMixOpMax()
{
	bool b = radioButton_Max->isChecked();
	if (b)
	{
		mixOp.op = OP_MAX;
		UPDATE_OP_CONTROL();
		emit channelTableChanged();
	}
}
void ChannelTable::setMixOpSum()
{
	bool b = radioButton_Sum->isChecked();
	if (b)
	{
		mixOp.op = OP_SUM;
		UPDATE_OP_CONTROL();
		emit channelTableChanged();
	}
}
void ChannelTable::setMixOpMean()
{
	bool b = radioButton_Mean->isChecked();
	if (b)
	{
		mixOp.op = OP_MEAN;
		UPDATE_OP_CONTROL();
		emit channelTableChanged();
	}
}
void ChannelTable::setMixOpOIT()
{
	bool b = radioButton_OIT->isChecked();
	if (b)
	{
		mixOp.op = OP_OIT;
		UPDATE_OP_CONTROL();
		emit channelTableChanged();
	}
}
void ChannelTable::setMixOpIndex()
{
	bool b = radioButton_Index->isChecked();
	if (b)
	{
		mixOp.op = OP_INDEX;
		UPDATE_OP_CONTROL();
		emit channelTableChanged();

		//if (ctab)   xform->syncChannelTabWidgets(ctab);
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

void ChannelTable::setDefault()
{
	setChannelColorDefault(listChannel.size());
	updateTableChannel();

	MixOP old = mixOp; //save brightness/contrast
	mixOp = MixOP();
	mixOp.brightness = old.brightness;
	mixOp.contrast = old.contrast;

	setRescaleDefault();
	updateMixOpControls();
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
//	if (! tabOptions) return 0;
//
//	int k = 1 + (tabOptions->currentIndex());

	return table;
}


#define UPATE_ITEM_ICON(curItem)   curItem->setData(Qt::DecorationRole, curItem->data(0))


void ChannelTable::pressedClickHandler(int i, int j)
{
	if (QApplication::mouseButtons()==Qt::RightButton) //right button menu
	{
		QTableWidget* t = currentTableWidget();
		QTableWidgetItem *curItem = t->item(i, j);

		if (t==table)
		{
			QMenu menu;
			QAction *act=0, *actDialog=0,
					*actRed=0, *actGreen=0, *actBlue=0, *actMagenta=0, *actGray=0, *actBlank=0,
					*actHanchuan=0, *actRandom=0, *actTmp=0;

			menu.addAction(actDialog 	= new QAction(tr("Color dialog ..."), this));
			menu.addSeparator();
			menu.addAction(actRed 	= new QAction(tr("Red"), this));
		    menu.addAction(actGreen = new QAction(tr("Green"), this));
		    menu.addAction(actBlue 	= new QAction(tr("Blue"), this));
		    menu.addAction(actMagenta = new QAction(tr("Magenta"), this));
		    menu.addAction(actGray 	= new QAction(tr("Gray  (r=g=b=255)"), this));
		    menu.addAction(actBlank = new QAction(tr("Blank (r=g=b=a=0)"), this));
			menu.addSeparator();
		    menu.addAction(actTmp = new QAction(tr("For selected channels:"), this));
		    menu.addAction(actHanchuan = new QAction(tr("Hanchuan's color"), this));
		    menu.addAction(actRandom = new QAction(tr("Random color"), this));

			actTmp->setDisabled(true);
		    act = menu.exec(QCursor::pos());

			QColor qcolor = QCOLORV(t->item(i, 0)->data(0)); // color cell of current row
		    if (act==actDialog)
			{
				if (! v3dr_getColorDialog( &qcolor))  return;
			}
			else if (act==actRed)
			{
				qcolor = QColor(255,0,0);
			}
			else if (act==actGreen)
			{
				qcolor = QColor(0,255,0);
			}
			else if (act==actBlue)
			{
				qcolor = QColor(0,0,255);
			}
			else if (act==actMagenta)
			{
				qcolor = QColor(255,0,255);
			}
			else if (act==actGray)
			{
				qcolor = QColor(255,255,255);
			}
			else if (act==actBlank)
			{
				qcolor = QColor(0,0,0,0); //also alpha=0
			}
			else if (act==actHanchuan) {}
			else if (act==actRandom) {}
			else
			{
				return; //110729
			}

			begin_batch();
			int n_row = t->rowCount();
			int i=0;
			for (int ii=0; ii<n_row; ii++)
			{
				curItem = t->item(ii,0); //color
				if (! curItem->isSelected()) continue; // skip un-selected

				if (act==actHanchuan)
				{
					int j = (i++)%hanchuan_colortable_size();
					qcolor = QColor(hanchuan_colortable[j][0],hanchuan_colortable[j][1],hanchuan_colortable[j][2]);
				}
				else if (act==actRandom)
				{
					RGB8 c = random_rgb8();
					qcolor = QColor(c.r, c.g, c.b);
				}

				curItem->setData(0, qVariantFromValue(qcolor));
			}
		    end_batch();
			if (act)  updatedContent(t);
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

#define BOOL_TO_CHECKED(b) (Qt::CheckState(b*2))
#define INT_TO_CHECKED(b) (Qt::CheckState(b))
#define CHECKED_TO_INT(b) (int(b))
#define CHECKED_TO_BOOL(b) (int(b)>0)


#define ADD_ONOFF(b)	{curItem = new QTableWidgetItem();	t->setItem(i, j++, curItem); \
						curItem->setCheckState(BOOL_TO_CHECKED(b));}

#define ADD_QCOLOR(c)	{curItem = new QTableWidgetItem(QVariant::Color);	t->setItem(i, j++, curItem); \
						curItem->setData(0, VCOLOR(c)); \
						curItem->setData(Qt::DecorationRole, VCOLOR(c));}

#define ADD_STRING(s)	{curItem = new QTableWidgetItem(s);	t->setItem(i, j++, curItem);}


QTableWidget*  ChannelTable::createTableChannel()
{
	QStringList qsl;
	qsl <<"color" << "channel"<<"on";
	int row = listChannel.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	//t->setHorizontalHeaderLabels(qsl);
	t->horizontalHeader()->hide();
	t->verticalHeader()->hide();

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;

		ADD_QCOLOR(listChannel[i].color);

		ADD_STRING( tr("c%1").arg(listChannel[i].n) );

		ADD_ONOFF(listChannel[i].on);

		assert(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	updateLuts(); //110729

	t->resizeColumnsToContents();
	return t;
}

void ChannelTable::updateTableChannel(bool update_luts)
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

		curItem = t->item(ii,2); //on/off
		curItem->setCheckState(BOOL_TO_CHECKED(listChannel[ii].on) );

	}
	end_batch();

	if (update_luts)  updateLuts(); //110729

	t->resizeColumnsToContents();
	//updatedContent(t);
}

void ChannelTable::updateLuts(int i)
{
	//110729, pre-compute lookup-tables
	int N = listChannel.size();
	QVector<RGBA8> lut(256);
	if (i==-1)
	{
		luts.clear();
		for (int k=0; k<N; k++)
		{
			make_linear_lut_one( listChannel[k].color, lut );
			luts.push_back(lut);
		}
	}
	else if (i>=0 && i<N)
	{
		make_linear_lut_one( listChannel[i].color, lut );
		luts[i] = lut;
	}
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
		updateLuts(i); //110729
		break;
	case 2:
		listChannel[i].on = CHECKED_TO_BOOL(curItem->checkState());
		break;
	}

	updatedContent(t);
}

/////////////////////////////////////////////////////////////////
#define __BrightenBox__

#define PI 3.14159265

void BrightenBox::create()
{
	QLabel* label_bright = new QLabel("Brightness (-100% ~ +100%)");
	QLabel* label_contrast = new QLabel(QString("Contrast (-100% ~ +100%)"));
	QString note = ("Better to check on '0~255'\n in Channels page for non-8bit.");
	QLabel* label_note = new QLabel(note);
	QFont font = label_note->font();
	font.setPointSizeF(font.pointSize()*.60);
	label_note->setFont(font);
	label_note->setToolTip(note);

	slider_bright = new QSlider(Qt::Horizontal);	slider_bright->setRange(-100, 100);
	slider_bright->setTickPosition(QSlider::TicksBelow);
	slider_contrast = new QSlider(Qt::Horizontal);	slider_contrast->setRange(-100, 100);
	slider_contrast->setTickPosition(QSlider::TicksBelow);
	spin_bright = new QSpinBox();		spin_bright->setRange(-100, 100);
	spin_contrast = new QSpinBox();		spin_contrast->setRange(-100, 100);
	push_reset = new QPushButton("Reset");

	QGridLayout* layout = new QGridLayout(this);

	layout->addWidget(label_bright, 	1,0, 1,20);
	layout->addWidget(slider_bright,	2,0, 1,13);
	layout->addWidget(spin_bright,		2,14, 1,6);
	layout->addWidget(label_contrast, 	3,0, 1,20);
	layout->addWidget(slider_contrast,	4,0, 1,13);
	layout->addWidget(spin_contrast,	4,14, 1,6);
	layout->addWidget(push_reset,	5,14, 1,6);
	layout->addWidget(label_note,	5,0, 1,15);
	//layout->addWidget(new QLabel(note),	5,0, 2,15);

	connect(slider_bright, SIGNAL(valueChanged(int)), this, SLOT(setBrightness(int)));
	connect(spin_bright, SIGNAL(valueChanged(int)), this, SLOT(setBrightness(int)));
	connect(slider_contrast, SIGNAL(valueChanged(int)), this, SLOT(setContrast(int)));
	connect(spin_contrast, SIGNAL(valueChanged(int)), this, SLOT(setContrast(int)));
	connect(push_reset, SIGNAL(clicked()), this, SLOT(reset()));

	updateMixOpControls();
}

#define INT_BRIGHTNESS(b)  (int(b*100))
#define BRIGHTNESS_I(i)  (i/100.0)

#define INT_CONTRAST(c)  (atan(c)*(4/PI)*100-100)
#define CONTRAST_I(i)  (tan( (i+100)/100.0*(PI/4) ))

void BrightenBox::updateMixOpControls()
{
	int i;
	i = INT_BRIGHTNESS(mixOp.brightness);
	setBrightness(i);
	i = INT_CONTRAST(mixOp.contrast);
	setContrast(i);
}

void BrightenBox::reset()
{
	setBrightness(0);
	setContrast(0);
}

void BrightenBox::setBrightness(int i)
{
	if (i == _bright) return;
	_bright = i;
	mixOp.brightness = BRIGHTNESS_I(i);
	if (slider_bright)
	{
		slider_bright->setValue(i);
	}
	if (spin_bright)
	{
		spin_bright->setValue(i);
	}
	emit brightenChanged();
}

void BrightenBox::setContrast(int i)
{
	if (i == _contrast) return;
	_contrast = i;
	mixOp.contrast = CONTRAST_I(i);
	if (slider_contrast)
	{
		slider_contrast->setValue(i);
	}
	if (spin_contrast)
	{
		spin_contrast->setValue(i);
	}
	emit brightenChanged();
}

#define ___MiscBox___

void MiscBox::create()
{
	push_export = new QPushButton("Export a RGB stack");

	QGridLayout* layout = new QGridLayout(this);

	layout->addWidget(push_export, 	1,0, 1,20);

	//connect(push_export, SIGNAL(clicked()), this, SIGNAL(signalExportRGBStack()));
	connect(push_export, SIGNAL(clicked()), this, SLOT(exportRGBStack()));
}

void MiscBox::exportRGBStack()
{
	if (! xform) return;
	My4DImage* img4d = xform->getImageData();
	if (! img4d)
	{
		v3d_msg("No image data now.");
		return;
	}
    ImagePixelType dtype;
  	unsigned char **** p4d = (unsigned char ****)img4d->getData(dtype);
	if (!p4d)
	{
		v3d_msg("No image data pointer now.");
		return;
	}
	MainWindow* mainwin = xform->getMainControlWindow();
	if (! mainwin)
	{
		v3d_msg("No main window pointer now.");
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// do old code for OP_INDEX
	if (mixOp.op==OP_INDEX)
	{
		//v3d_msg("No support for 'Index' mode.");
		img4d->proj_general_convertIndexedImg2RGB();

		QApplication::restoreOverrideCursor();
		return;
	}

	V3DLONG sx = img4d->getXDim();
	V3DLONG sy = img4d->getYDim();
	V3DLONG sz = img4d->getZDim();
	unsigned char* newdata1d = new unsigned char[sx*sy*sz*3]; //RGB stack
	Image4DSimple tmp;
	tmp.setData(newdata1d, sx,sy,sz, 3, V3D_UINT8);
	Image4DProxy<Image4DSimple> ptmp(&tmp);

//	qDebug("MiscBox::exportRGBStack, Image4DProxy<Image4DSimple> ptmp(&tmp)");

#define COPY_mixChannel_planeZ( slice, zpos, p4d ) {\
	slice = copyRaw2QImage_Slice( \
			listChannel, \
			mixOp, \
			&luts, \
			imgPlaneZ, \
			zpos, \
			img4d->p4d, \
			img4d->getXDim(), \
			img4d->getYDim(), \
			img4d->getZDim(), \
			img4d->getCDim(), \
			img4d->p_vmax, \
			img4d->p_vmin); \
	}

	for (V3DLONG z=0; z<sz; z++)
	{
		QImage slice;
		switch (dtype)
		{
		case V3D_UINT8:
			COPY_mixChannel_planeZ( slice, z+1, data4d_uint8 );
			break;
		case V3D_UINT16:
			COPY_mixChannel_planeZ( slice, z+1, data4d_uint16 );
			break;
		case V3D_FLOAT32:
			COPY_mixChannel_planeZ( slice, z+1, data4d_float32 );
			break;
		default:
			break;
		}
		if (slice.isNull())
		{
			v3d_msg("Error: Z-slice size is NULL.");
			return;
		}

//		qDebug("MiscBox::exportRGBStack, (z=%d) pixel(%d,%d) tmp(%d %d %d %d)",
//				z, slice.width(),slice.height(), sx,sy,sz,tmp.getCDim());

		for (V3DLONG y=0; y<sy; y++)
		for (V3DLONG x=0; x<sx; x++)
		{
			BGRA8 c; //QRgb #AArrGGbb
			c.i = slice.pixel(x,y);
			*ptmp.at_uint8(x,y,z,0) = c.r;
			*ptmp.at_uint8(x,y,z,1) = c.g;
			*ptmp.at_uint8(x,y,z,2) = c.b;
		}
	}

//	qDebug("MiscBox::exportRGBStack, end of all slice");

	XFormWidget* newxform = mainwin->newImageWindow(""); //here will be "?_processed"
	mainwin->setImage(newxform, &tmp);
	mainwin->setImageName(newxform, "exported_RGBStack");
	mainwin->updateImageWindow(newxform);

	QApplication::restoreOverrideCursor();
}

