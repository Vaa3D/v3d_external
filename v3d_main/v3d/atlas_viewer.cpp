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





//atlas_viewer.cpp
// by Hanchuan Peng
//20081123
//20081217: force hide the move up/down etc button initially
//20100118: add the color options

#include "../3drenderer/v3dr_common.h"
#include "../3drenderer/renderer_gl1.h"
#include "atlas_viewer.h"
#include "v3d_core.h"

#include "../3drenderer/ItemEditor.h"

#include "landmark_property_dialog.h"

///////////////////////////////////////////////////////////
//#define UPDATE_VIEW()   if (triview_widget)  triview_widget->update();
///////////////////////////////////////////////////////////
#define UPDATE_VIEW(w)   {if(w) w->update();}
#define ACTIVATE(w)	     {if(w) w->activateWindow();}
#define UPDATE_TABS()    {int i = getCurTab(); reCreateTables(glwidget); setCurTab(i);}
///////////////////////////////////////////////////////////


V3D_atlas_viewerDialog::V3D_atlas_viewerDialog(XFormWidget* w) : QDialog() //w, setParent dynamically
{
	qDebug("V3D_atlas_viewerDialog::V3D_atlas_viewerDialog");

	setWindowFlags(Qt::Popup | Qt::Tool);
	setWindowTitle(tr("Atlas viewer and image blender / landmark manager / color channel manager"));
	//setAttribute ( Qt::WA_DeleteOnClose, true ); // set this may cause exception

	bCanUndo = false;
	bMod = false;
	triview_widget = 0;
	imgdata = 0;
	ref = 0;
	oldpos = (pos());
	IncRef(w);

	okButton=cancelButton=undoButton=0;
	selectAllButton=deselectAllButton=inverseSelectButton=onSelectButton=offSelectButton=colorSelectButton=0;
	for (int i=0; i<=2; i++)  table[i]=0;
	tabOptions=0;

	//for the atlas viewer. will be hidden for landmark control
	channelSpinBox=0; channelSpinBoxLabel=0;
	bMaskBlendedImgsBox=0;

	//for the landmark manager, will hide otherwise
	seePropertyButton = 0;
	moveUpButton = moveDownButton = 0;
	deleteButton = 0;
	resetAllLandmarkNamesButton = resetAllLandmarkCommentsButton = 0;

	//search for text strings
	searchTextEditLabel = searchTextResultLabel = 0;
	searchTextEdit = 0;
	doSearchTextNext = doSearchTextPrev = 0;

	//
	create();
	setItemEditor();

    this->resize(700,700);

	//force the hiding of some button by calling tablChanged. 081217. revised on 100824
	if (table[1] && table[2] && table[3])
	{
		if (currentTableWidget()==table[1]) tabChanged(0);
		else if (currentTableWidget()==table[2]) tabChanged(1);
		else if (currentTableWidget()==table[3]) tabChanged(2);
	}
}

V3D_atlas_viewerDialog::~V3D_atlas_viewerDialog()
{
	qDebug("V3D_atlas_viewerDialog::~V3D_atlas_viewerDialog");
	if (triview_widget) triview_widget->atlasViewerDlg = 0;
}

void V3D_atlas_viewerDialog::IncRef(XFormWidget* w)
{
	if (w && w!=triview_widget)
	{
		//		oldpos = (pos());
		//		setParent(w->mainwindow, windowFlags()); // must also set windowFlags
		//		move((oldpos));

		triview_widget = w;
		imgdata = (My4DImage*)(w->getImageData());
	}

	ref++;
	qDebug("V3D_atlas_viewerDialog::ref = %d", ref);
}

void V3D_atlas_viewerDialog::DecRef(XFormWidget* w)
{
	if (!w || w!=triview_widget) return;

	ref--;
	if (ref<1)
	{
		hide();
		deleteLater();
	}
	qDebug("V3D_atlas_viewerDialog::ref = %d", ref);
}

void V3D_atlas_viewerDialog::closeEvent(QCloseEvent* e)
{
	hide();
	e->ignore();
}

void V3D_atlas_viewerDialog::showEvent(QShowEvent* e)
{
	move(oldpos);
}

void V3D_atlas_viewerDialog::moveEvent(QMoveEvent* e)
{
	oldpos = (e->pos());
}

void V3D_atlas_viewerDialog::done(int ret)
{
	qDebug("  V3D_atlas_viewerDialog::done");

	if (ret==1) //this corresponds to the accept()
	{
		if (currentTableWidget()==table[1] && imgdata)
			imgdata->proj_general_blend_atlasfiles();
	}
	else //restore the original states
	{
		if (imgdata)
		{
			imgdata->listAtlasFiles = listAtlasFiles;
			imgdata->listLandmarks = listLandmarks;
		}
	}

	QDialog::done(ret);
	UPDATE_VIEW(triview_widget);
	qDebug("  V3D_atlas_viewerDialog::done end");
}

void V3D_atlas_viewerDialog::reCreateTables(XFormWidget* w)
{
	qDebug("  V3D_atlas_viewerDialog::reCreateTables ( %p )", w);
	if (! w)  return;

	IncRef(w);
	DecRef(w); // not ref++, just set data source
	bCanUndo = false;
	bMod = false;

	if (! tabOptions) return;
	tabOptions->clear();

	if (table[1])	disconnect(table[1], SIGNAL(cellChanged(int,int)), this, SLOT(pickAtlasRow(int,int)));
	if (table[2])
	{
		disconnect(table[2], SIGNAL(cellChanged(int,int)), this, SLOT(pickLandmark(int,int)));
		disconnect(table[2], SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(highlightLandmark(int,int,int,int))); //
	}
	//if (table[3])	disconnect(table[3], SIGNAL(cellChanged(int,int)), this, SLOT(pickColorChannel(int,int)));
	for (int i=0; i<=2; i++)
		if (table[i])
		{
			delete table[i];  table[i]=0;
		}

	deleteChannelTab((ChannelTabWidget*)table[3]);

	createTables();

    int i;
 	i= tabOptions->addTab(table[1], "Atlas manager");
	i= tabOptions->addTab(table[2], "Landmarks");
	i= tabOptions->addTab(table[3], "Color channels");

}

void V3D_atlas_viewerDialog::undo()
{
	qDebug("  V3D_atlas_viewerDialog::undo");

	PROGRESS_DIALOG("Undoing     ", this);
	PROGRESS_PERCENT(30);

	if (bCanUndo && bMod && imgdata)
	{
		// restore the old state
		imgdata->listAtlasFiles = listAtlasFiles;
		imgdata->listLandmarks = listLandmarks;

		reCreateTables(triview_widget);
	}

	PROGRESS_PERCENT(100);

	UPDATE_VIEW(triview_widget);
}

void V3D_atlas_viewerDialog::createTables()
{
	//qDebug("  V3D_atlas_viewerDialog::createTables");

	if (imgdata)
	{
		// save the old state
		listAtlasFiles   = imgdata->listAtlasFiles;
		listLandmarks = imgdata->listLandmarks;
		bCanUndo = true;
		bMod = false;
		if (undoButton) undoButton->setEnabled(bCanUndo && bMod);

		if (channelSpinBox) channelSpinBox->setValue(imgdata->atlasColorBlendChannel+1);
		if (bMaskBlendedImgsBox) bMaskBlendedImgsBox->setCheckState((imgdata->bUseFirstImgAsMask)?(Qt::Checked):(Qt::Unchecked));

		if (searchTextEdit) searchTextEdit->setText(imgdata->curSearchText);
	}

	table[1] = createTableAtlasRows();
	table[2] = createTableLandmarks();
	//table[3] = createColorChannelManager();
	for (int i=1; i<=2; i++)
	{
		if (table[i])
		{
			table[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
		    table[i]->setEditTriggers(//QAbstractItemView::CurrentChanged |
									  QAbstractItemView::DoubleClicked |
									  QAbstractItemView::SelectedClicked);
		}
	}

	if (table[1])	connect(table[1], SIGNAL(cellChanged(int,int)), this, SLOT(pickAtlasRow(int,int)));
	if (table[2])
	{
		connect(table[2], SIGNAL(cellChanged(int,int)), this, SLOT(pickLandmark(int,int)));
		connect(table[2], SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(highlightLandmark(int,int,int,int)));
	}
	//if (table[3])	connect(table[3], SIGNAL(cellChanged(int,int)), this, SLOT(pickColorChannel(int,int)));

	table[3] = (QTableWidget*)createChannelTab();
}


void V3D_atlas_viewerDialog::create()
{
	//control box

	QGroupBox* buttonGroup = new QGroupBox();
    QVBoxLayout *buttonRgnLayout = new QVBoxLayout(buttonGroup);

    okButton = new QPushButton("OK");
	cancelButton = new QPushButton("Cancel");
	buttonRgnLayout->addWidget(okButton);
	buttonRgnLayout->addWidget(cancelButton);
    buttonRgnLayout->addSpacing(15);

    undoButton = new QPushButton("Undo");
    buttonRgnLayout->addWidget(undoButton);
    buttonRgnLayout->addSpacing(15);
    //buttonRgnLayout->addWidget(new QLabel);

    selectAllButton = new QPushButton("Select All");
    deselectAllButton = new QPushButton("Deselect All");
    inverseSelectButton = new QPushButton("Select Inverse");
    buttonRgnLayout->addWidget(selectAllButton);
    buttonRgnLayout->addWidget(deselectAllButton);
    buttonRgnLayout->addWidget(inverseSelectButton);
    buttonRgnLayout->addSpacing(15);
    //buttonRgnLayout->addWidget(new QLabel);

    onSelectButton = new QPushButton("Selected On");
    offSelectButton = new QPushButton("Selected Off");
    colorSelectButton = new QPushButton("Selected Color");
    buttonRgnLayout->addWidget(onSelectButton);
    buttonRgnLayout->addWidget(offSelectButton);
    buttonRgnLayout->addWidget(colorSelectButton);
    buttonRgnLayout->addSpacing(15);

	channelSpinBox = new QSpinBox;
    channelSpinBox->setRange(1, 3);
    channelSpinBox->setSingleStep(1);
    channelSpinBox->setValue(2);
	channelSpinBoxLabel = new QLabel("Channel to blend");
	bMaskBlendedImgsBox = new QCheckBox("Pattern F-masked?");

	buttonRgnLayout->addWidget(channelSpinBoxLabel);
	buttonRgnLayout->addWidget(channelSpinBox);
	buttonRgnLayout->addWidget(bMaskBlendedImgsBox);

	seePropertyButton = new QPushButton("See/edit Property");
	moveUpButton = new QPushButton("Move up");
	moveDownButton = new QPushButton("Move down");
	deleteButton = new QPushButton("Delete");
	resetAllLandmarkNamesButton = new QPushButton("Reset all names");
	resetAllLandmarkCommentsButton = new QPushButton("Clear all comments");

	buttonRgnLayout->addWidget(seePropertyButton);
	buttonRgnLayout->addWidget(moveUpButton);
	buttonRgnLayout->addWidget(moveDownButton);
	buttonRgnLayout->addWidget(deleteButton);
	buttonRgnLayout->addWidget(resetAllLandmarkNamesButton);
	buttonRgnLayout->addWidget(resetAllLandmarkCommentsButton);

    buttonRgnLayout->addStretch(0);

	//search box
	QGroupBox* searchGroup = new QGroupBox();
	QHBoxLayout *searchLayout = new QHBoxLayout(searchGroup);
	searchTextEditLabel = new QLabel("Find:");
	searchTextEdit = new QLineEdit;
	doSearchTextNext = new QPushButton("Next");
	doSearchTextPrev = new QPushButton("Previous");
	searchTextResultLabel = new QLabel("          "); //space left for display "not found" if nothing is found
	searchLayout->addWidget(searchTextEditLabel);
	searchLayout->addWidget(searchTextEdit);
	searchLayout->addWidget(doSearchTextNext);
	searchLayout->addWidget(doSearchTextPrev);
	searchLayout->addWidget(searchTextResultLabel);

	//table box

	createTables();

    int i;
    //tabOptions = new AutoTabWidget();
    tabOptions = new QTabWidget(); //081210: by Hanchuan Peng. I don't like the autotab for atlas viewer
	i= tabOptions->addTab(table[1], "Atlas manager");
	i= tabOptions->addTab(table[2], "Landmarks");
	i= tabOptions->addTab(table[3], "Color channels");
	tabOptions->setTabToolTip(1-1, "Blending image together");
	tabOptions->setTabToolTip(2-1, "Managing landmarks");
	tabOptions->setTabToolTip(3-1, "Data channel color mapping");

	QGroupBox* tabGroup = new QGroupBox();
    QHBoxLayout *centralLayout = new QHBoxLayout(tabGroup);
    centralLayout->addWidget(tabOptions);
    centralLayout->addWidget(buttonGroup);

	//overall layout

    QVBoxLayout *allLayout = new QVBoxLayout(this);
	allLayout->addWidget(tabGroup);
	allLayout->addWidget(searchGroup);

	createMenuOfColor();

	//connect signal/slots

    if (okButton)		connect(okButton, SIGNAL(clicked()),    this, SLOT(accept()));
	if (cancelButton)	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	if (undoButton)	    connect(undoButton, SIGNAL(clicked()), this, SLOT(undo()));
	if (selectAllButton)	connect(selectAllButton, SIGNAL(clicked()),   this, SLOT(selectAll()));
	if (deselectAllButton)	connect(deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAll()));
	if (inverseSelectButton)	connect(inverseSelectButton, SIGNAL(clicked()), this, SLOT(inverseSelect()));
	if (onSelectButton)		connect(onSelectButton, SIGNAL(clicked()),    this, SLOT(onSelected()));
	if (offSelectButton)	connect(offSelectButton, SIGNAL(clicked()),   this, SLOT(offSelected()));

	//if (colorSelectButton)	connect(colorSelectButton, SIGNAL(clicked()),   this, SLOT(colorSelected()));
	if (colorSelectButton)	connect(colorSelectButton, SIGNAL(clicked()),   this, SLOT(doMenuOfColor()));

	if (channelSpinBox) connect(channelSpinBox, SIGNAL(valueChanged(int)),   this, SLOT(colorChannelSelected(int)));
	if (bMaskBlendedImgsBox) connect(bMaskBlendedImgsBox, SIGNAL(clicked()),   this, SLOT(maskImgStateChanged()));

	if (channelSpinBox && channelSpinBoxLabel) connect(tabOptions, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

	if (seePropertyButton) connect(seePropertyButton, SIGNAL(clicked()), this, SLOT(seeLandmarkProperty()));
	if (moveUpButton) connect(moveUpButton, SIGNAL(clicked()), this, SLOT(moveLandmarkUp()));
	if (moveDownButton) connect(moveDownButton, SIGNAL(clicked()), this, SLOT(moveLandmarkDown()));
	if (deleteButton) connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteSelectedLandmark()));
	if (resetAllLandmarkNamesButton) connect(resetAllLandmarkNamesButton, SIGNAL(clicked()), this, SLOT(resetAllLandmarkNames()));
	if (resetAllLandmarkCommentsButton) connect(resetAllLandmarkCommentsButton, SIGNAL(clicked()), this, SLOT(resetAllLandmarkComments()));

	if (searchTextEdit && doSearchTextNext) connect(doSearchTextNext, SIGNAL(clicked()), this, SLOT(findNext()));
	if (searchTextEdit && doSearchTextPrev) connect(doSearchTextPrev, SIGNAL(clicked()), this, SLOT(findPrev()));

	//if (searchTextEdit) connect(searchTextEdit, SIGNAL(returnPressed()), this, SLOT(findNext()));
}

void V3D_atlas_viewerDialog::updatedContent(QTableWidget* t) //
{
	if (! in_batch_stack.empty() && in_batch_stack.last()==true) return; //skip until end_batch

	t->resizeColumnsToContents();

	UPDATE_VIEW(triview_widget);
	ACTIVATE(triview_widget);

	bMod = true;
	undoButton->setEnabled(bCanUndo && bMod);
}


QTableWidget* V3D_atlas_viewerDialog::currentTableWidget(QWidget** pp_page)
{
	if (! tabOptions) return 0;

	int k =  1+tabOptions->currentIndex();
	Q_ASSERT(k>=1 && k<=3);


	if (k==3)
	{
		ChannelTabWidget* ctw = qobject_cast<ChannelTabWidget*>(table[3]);
		if (ctw && ctw->getChannelPage())
		{
			if (pp_page)  *pp_page = ctw->getChannelPage();
			return ctw->getChannelPage()->getTable();
		}
		else
		{
			if(pp_page)  *pp_page = 0;
			return 0;
		}
	}

	if (pp_page)  *pp_page = 0;
	return table[k];
}

ChannelTabWidget* V3D_atlas_viewerDialog::getChannelTabWidget()
{
	if (! tabOptions) return 0;

	return qobject_cast<ChannelTabWidget*>(table[3]);
}


void V3D_atlas_viewerDialog::createMenuOfColor()
{
    QAction* Act;

    Act = new QAction(tr("Single Color..."), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(selectedColor()));
    menuColor.addAction(Act);

    Act = new QAction(tr("Hanchuan's Color Mapping"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(mapHanchuanColor()));//call seletedColor(1)
    menuColor.addAction(Act);

    Act = new QAction(tr("Random Color Mapping"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(mapRandomColor()));//call seletedColor(-1)
    menuColor.addAction(Act);
}

void V3D_atlas_viewerDialog::doMenuOfColor()
{
	try
	{
		menuColor.exec(QCursor::pos());
	}
	catch (...)
	{
		printf("Fail to run the V3dr_surfaceDialog::doMenuOfColor() function.\n");
	}
}


///////////////////////////////////////////////////////////////////////

#define BOOL_TO_CHECKED(b) (Qt::CheckState(b*2))
#define INT_TO_CHECKED(b) (Qt::CheckState(b))
#define CHECKED_TO_INT(b) (int(b))
#define CHECKED_TO_BOOL(b) (int(b)>0)

void V3D_atlas_viewerDialog::selectAll()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	t->selectAll();
}

void V3D_atlas_viewerDialog::deselectAll()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	t->clearSelection();
}

void V3D_atlas_viewerDialog::inverseSelect()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	for (int i=0; i<t->rowCount(); i++)
	{
		QTableWidgetItem * item = t->item(i,0);
		bool b = item->isSelected();
		for (int j=0; j<t->columnCount(); j++)
		{
			item = t->item(i,j);
			item->setSelected( ! b );
		}
	}
}

#define ON_ITEM_OF_TABLE(curItem, t,i) \
{ \
	if 		(t==table[1])	curItem = t->item(i, 0); \
	else if (t==table[2]) 	curItem = t->item(i, 0); \
	else 		curItem = t->item(i, 2); \
}

#define COLOR_ITEM_OF_TABLE(curItem, t,i) \
{ \
	if 		(t==table[1])	curItem = t->item(i, 1); \
	else if (t==table[2]) 	curItem = t->item(i, 1); \
	else 		curItem = t->item(i, 0); \
}

void V3D_atlas_viewerDialog::onSelected()
{
	QWidget* page=0;
	QTableWidget* t = currentTableWidget( &page );
	ChannelTable* ct = qobject_cast<ChannelTable*>(page);//110805 RZC
	if (! t) return;

	PROGRESS_DIALOG("Updating     ", this);
	PROGRESS_PERCENT(30);
	begin_batch();
	if (ct)  ct->begin_batch();

	QTableWidgetItem * item;
	for (int i=0; i<t->rowCount(); i++)
	{
		ON_ITEM_OF_TABLE(item, t, i);
		if (item->isSelected())
			item->setCheckState(BOOL_TO_CHECKED(true));
	}

	if (ct)  ct->end_batch();
	end_batch();
	PROGRESS_PERCENT(100);

	if (ct)  ct->updatedContent(t);
	updatedContent(t);
}

void V3D_atlas_viewerDialog::offSelected()
{
	QWidget* page=0;
	QTableWidget* t = currentTableWidget( &page );
	ChannelTable* ct = qobject_cast<ChannelTable*>(page);//110805 RZC
	if (! t) return;

	PROGRESS_DIALOG("Updating     ", this);
	PROGRESS_PERCENT(30);
	begin_batch();
	if (ct)  ct->begin_batch();

	QTableWidgetItem * item;
	for (int i=0; i<t->rowCount(); i++)
	{
		ON_ITEM_OF_TABLE(item, t, i);
		if (item->isSelected())
			item->setCheckState(BOOL_TO_CHECKED(false));
	}

	if (ct)  ct->end_batch();
	end_batch();
	PROGRESS_PERCENT(100);

	if (ct)  ct->updatedContent(t);
	updatedContent(t);
}


///////////////////////////////////////////////////////////////////////////////
//unsigned char hanchuan_colortable[][3]={
//{255,   0,    0},
//{  0, 255,    0},
//{  0,   0,  255},
//{255, 255,    0},
//{  0, 255,  255},
//{255,   0,  255},
//{255, 128,    0},
//{  0, 255,  128},
//{128,   0,  255},
//{128, 255,    0},
//{  0, 128,  255},
//{255,   0,  128},
//{128,   0,    0},
//{  0, 128,    0},
//{  0,   0,  128},
//{128, 128,    0},
//{  0, 128,  128},
//{128,   0,  128},
//{255, 128,  128},
//{128, 255,  128},
//{128, 128,  255},
//};
//int hanchuan_colortable_size = sizeof(hanchuan_colortable)/3;
#define V3D_QCOLOR(rgba8)   QColorFromRGBA8( rgba8 )


void V3D_atlas_viewerDialog::selectedColor(int map)
{
	QWidget* page=0;
	QTableWidget* t = currentTableWidget( &page );
	ChannelTable* ct = qobject_cast<ChannelTable*>(page); //110805 RZC
	if (! t) return;

	QColor qcolor0(255,255,255);
	if (map==0)
	{
		//qcolor0 = QColorDialog::getColor(QColor());
		//if (! qcolor0.isValid()) return; // this no use for cancel, Qt's bug
		if (! v3dr_getColorDialog( &qcolor0))  return; //090424 RZC
	}

	PROGRESS_DIALOG("Updating color    ", this);
	begin_batch();
	if (ct)  ct->begin_batch();


	QTableWidgetItem * curItem;
	int n_row = t->rowCount();
	for (int i=0; i<n_row; i++)
	{
		PROGRESS_PERCENT(i*100/n_row);

		COLOR_ITEM_OF_TABLE(curItem, t, i);
		QColor qcolor = qcolor0;

		if (map==-1)      //random color
		{
			qcolor = V3D_QCOLOR(random_rgba8());
		}
		else if (map==1)  //hanchuan' color table
		{
			int j = i%hanchuan_colortable_size();
			qcolor = QColor(hanchuan_colortable[j][0],hanchuan_colortable[j][1],hanchuan_colortable[j][2]);
		}
		else //map==0
			if (! curItem->isSelected()) continue; // skip un-selected

        //curItem->setData(0, qVariantFromValue(qcolor));
		//UPATE_ITEM_ICON(curItem); //this will be called in slot connected cellChanged()
	}

	if (ct)  ct->end_batch();
	end_batch();
	PROGRESS_PERCENT(100);

	if (ct)  ct->updatedContent(t);
	updatedContent(t);
}


///////////////////////////////////////////////////////////////////////////////

void V3D_atlas_viewerDialog::tabChanged(int t)
{
	if (t==1)
	{
		if(channelSpinBox) channelSpinBox->hide();
		if (channelSpinBoxLabel) channelSpinBoxLabel->hide();
		if (bMaskBlendedImgsBox) bMaskBlendedImgsBox->hide();

		if (seePropertyButton) seePropertyButton->show();
		if (moveUpButton) moveUpButton->show();
		if (moveDownButton) moveDownButton->show();
		if (deleteButton) deleteButton->show();
		if (resetAllLandmarkNamesButton) resetAllLandmarkNamesButton->show();
		if (resetAllLandmarkNamesButton) resetAllLandmarkCommentsButton->show();

		if (imgdata)
		{
			if (imgdata->listLandmarks.size()<=0)
			{
				okButton->setEnabled(false);
				searchTextEdit->setEnabled(false);
				doSearchTextNext->setEnabled(false);
				doSearchTextPrev->setEnabled(false);
			}
			else
			{
				okButton->setEnabled(true);
				searchTextEdit->setEnabled(true);
				doSearchTextNext->setEnabled(true);
				doSearchTextPrev->setEnabled(true);
			}
		}
	}
	else if (t==0)
	{
		if(channelSpinBox) channelSpinBox->show();
		if (channelSpinBoxLabel) channelSpinBoxLabel->show();
		if (bMaskBlendedImgsBox) bMaskBlendedImgsBox->show();

		if (seePropertyButton) seePropertyButton->hide();
		if (moveUpButton) moveUpButton->hide();
		if (moveDownButton) moveDownButton->hide();
		if (deleteButton) deleteButton->hide();
		if (resetAllLandmarkNamesButton) resetAllLandmarkNamesButton->hide();
		if (resetAllLandmarkNamesButton) resetAllLandmarkCommentsButton->hide();

		//disable the OK button when the list is empty
		if (imgdata)
		{
			if (imgdata->listAtlasFiles.size()<=0)
			{
				okButton->setEnabled(false);
				searchTextEdit->setEnabled(false);
				doSearchTextNext->setEnabled(false);
				doSearchTextPrev->setEnabled(false);
			}
			else
			{
				okButton->setEnabled(true);
				searchTextEdit->setEnabled(true);
				doSearchTextNext->setEnabled(true);
				doSearchTextPrev->setEnabled(true);
			}
		}
	}
	else if (t==2)
	{
		if(channelSpinBox) channelSpinBox->hide();
		if (channelSpinBoxLabel) channelSpinBoxLabel->hide();
		if (bMaskBlendedImgsBox) bMaskBlendedImgsBox->hide();

		if (seePropertyButton) seePropertyButton->hide();
		if (moveUpButton) moveUpButton->hide();
		if (moveDownButton) moveDownButton->hide();
		if (deleteButton) deleteButton->hide();
		if (resetAllLandmarkNamesButton) resetAllLandmarkNamesButton->hide();
		if (resetAllLandmarkNamesButton) resetAllLandmarkCommentsButton->hide();
	}
}

void V3D_atlas_viewerDialog::colorSelected()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	QColor qcolor = QColorDialog::getColor(QColor());

	PROGRESS_DIALOG("Updating     ", this);
	PROGRESS_PERCENT(30);

	QTableWidgetItem * item;
	for (int i=0; i<t->rowCount(); i++)
	{
		COLOR_ITEM_OF_TABLE(item, t, i);
		if (item->isSelected())
		{
            //item->setData(Qt::DecorationRole, qVariantFromValue(qcolor));
            //item->setData(Qt::DisplayRole, qVariantFromValue(qcolor));
		}
	}

	PROGRESS_PERCENT(100);

	bMod = true;
	undoButton->setEnabled(bCanUndo && bMod);
}


void V3D_atlas_viewerDialog::colorChannelSelected(int c)
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;
	if (t!=table[1]) return; //ignore the channel selection if not the atlas viewer is selected

	if (c<1) c=1;
	if (c>3) c=3;
	if (imgdata) imgdata->atlasColorBlendChannel = c-1;
}

void V3D_atlas_viewerDialog::maskImgStateChanged()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;
	if (t!=table[1]) return; //ignore the channel selection if not the atlas viewer is selected

	if (!bMaskBlendedImgsBox) return;
	if (imgdata) imgdata->bUseFirstImgAsMask = (bMaskBlendedImgsBox->checkState()==Qt::Checked);
}

void V3D_atlas_viewerDialog::findNext()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	if (!imgdata || !searchTextEdit) return;
	imgdata->curSearchText = searchTextEdit->text();

	if (t->rowCount()<=0)
	{
		if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
		return;
	}

	//QList<QTableWidgetItem *> detectList = t->findItems(imgdata->curSearchText, Qt::MatchWildcard);
	if (searchTextResultLabel) searchTextResultLabel->setText("        ");

	bool b_found=false;
	int row_start = t->currentRow()+1, i;
	QString tmpstr;
	for (i=row_start;i<t->rowCount();i++)
	{
		//qDebug("1st next find: %d", i);

		if (t==table[1]) tmpstr = t->item(i, 2)->text();
		else //if (t==table[2])
			tmpstr = t->item(i, 8)->text();

		if (tmpstr.contains(imgdata->curSearchText, Qt::CaseInsensitive))
		{
			t->scrollToItem(t->item(i,2));
			t->setCurrentCell(i, 2);
			b_found=true;
			break;
		}
	}
	if (b_found==false) //then start from the first row
	{
		for (i=0;i<row_start;i++)
		{
			//qDebug("2nd next find: %d", i);
			if (t==table[1]) tmpstr = t->item(i, 2)->text();
			else //if (t==table[2])
				tmpstr = t->item(i, 8)->text();
			if (tmpstr.contains(imgdata->curSearchText, Qt::CaseInsensitive))
			{
				t->scrollToItem(t->item(i,2));
				t->setCurrentCell(i, 2);
				b_found=true;
				break;
			}
		}
	}
	if (b_found==false)
		if (searchTextResultLabel) searchTextResultLabel->setText("Not found");

	//qDebug("done next find \n\n");

}

void V3D_atlas_viewerDialog::findPrev()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	if (!imgdata || !searchTextEdit) return;
	imgdata->curSearchText = searchTextEdit->text();

	if (t->rowCount()<=0)
	{
		if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
		return;
	}

	//QList<QTableWidgetItem *> detectList = t->findItems(imgdata->curSearchText, Qt::MatchWildcard);
	if (searchTextResultLabel) searchTextResultLabel->setText("        ");

	bool b_found=false;
	int row_start = t->currentRow()-1, i;
	QString tmpstr;
	for (i=row_start;i>=0;i--)
	{
		qDebug("1st prev find: %d", i);

		if (t==table[1]) tmpstr = t->item(i, 2)->text();
		else //if (t==table[2])
			tmpstr = t->item(i, 8)->text();

		if (tmpstr.contains(imgdata->curSearchText, Qt::CaseInsensitive))
		{
			t->scrollToItem(t->item(i,2));
			t->setCurrentCell(i, 2);
			b_found=true;
			break;
		}
	}
	if (b_found==false) //then start from the first row
	{
		for (i=t->rowCount()-1;i>row_start;i--)
		{
			//qDebug("2nd prev find: %d", i);
			if (t==table[1]) tmpstr = t->item(i, 2)->text();
			else //if (t==table[2])
				tmpstr = t->item(i, 8)->text();
			if (tmpstr.contains(imgdata->curSearchText, Qt::CaseInsensitive))
			{
				t->scrollToItem(t->item(i,2));
				t->setCurrentCell(i, 2);
				b_found=true;
				break;
			}
		}
	}
	if (b_found==false)
		if (searchTextResultLabel) searchTextResultLabel->setText("Not found");

	//qDebug("done prev find \n\n");

}

void V3D_atlas_viewerDialog::setItemEditor()
{
	static bool registered =false;
	if (registered) return;
	else registered = true;


	QItemEditorFactory *factory = new QItemEditorFactory(*QItemEditorFactory::defaultFactory());

	//	QItemEditorCreatorBase *spinCreator = new QStandardItemEditorCreator<QSpinBox>();
	//	QItemEditorCreatorBase *comboCreator = new QStandardItemEditorCreator<QComboBox>();
    //QItemEditorCreatorBase *colorCreator = new QStandardItemEditorCreator<ColorEditor>();

	factory->registerEditor(QVariant::String, 0);
    //factory->registerEditor(QVariant::Color, colorCreator);

	QItemEditorFactory::setDefaultFactory(factory);
}


QTableWidget* V3D_atlas_viewerDialog::createTableAtlasRows()
{
	My4DImage* r = imgdata;
	if (! r)  return 0;

	QStringList qsl;
	qsl << "on/off" << "color" << "category (e.g. genetic lines)" << "image file" << "file exist?";
	int row = r->listAtlasFiles.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *newItem;
		QString s;

		//s = tr("%1").arg("on");
		newItem = new QTableWidgetItem(s);	t->setItem(i, j++, newItem);
		newItem->setCheckState(BOOL_TO_CHECKED(r->listAtlasFiles[i].on));

		//s = tr("%1").arg("color");
		newItem = new QTableWidgetItem(QVariant::Color);	t->setItem(i, j++, newItem);
		{
			// all of this is ok, caution of alpha. 081114
			//newItem->setBackgroundColor(QCOLOR(r->listLabelS[i].color));
			//newItem->setBackground(QBrush(QCOLOR(r->listLabelS[i].color)));
			//newItem->setIcon(colorIcon(QCOLOR(r->listLabelS[i].color)));
			newItem->setData(Qt::DecorationRole, VCOLOR(r->listAtlasFiles[i].color));
			newItem->setData(Qt::DisplayRole, VCOLOR(r->listAtlasFiles[i].color));
			//qDebug() << QCOLOR(r->listLabelS[i].color);
			//qDebug() << newItem->data(0).type();
		}

		s = tr("%1").arg(r->listAtlasFiles[i].category);
		newItem = new QTableWidgetItem(s);	t->setItem(i, j++, newItem);

		s = tr("%1").arg(r->listAtlasFiles[i].imgfile);
		newItem = new QTableWidgetItem(s);	t->setItem(i, j++, newItem);

		s = tr("%1").arg(r->listAtlasFiles[i].exist);
		newItem = new QTableWidgetItem(s);	t->setItem(i, j++, newItem);

		Q_ASSERT(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void V3D_atlas_viewerDialog::pickAtlasRow(int i, int j)
{
	My4DImage* r = imgdata;
	if (! r)  return;
	if (i<0 || i>=r->listAtlasFiles.size())  return;
	if (!table) return;

	QTableWidget* t = table[1];
	QTableWidgetItem *newItem = t->item(i,j);

	switch(j)
	{
		case 0:
			r->listAtlasFiles[i].on = CHECKED_TO_BOOL(newItem->checkState());
			break;
		case 1:
			r->listAtlasFiles[i].color = RGBA8V(newItem->data(0));
			newItem->setData(Qt::DecorationRole, VCOLOR(r->listAtlasFiles[i].color));
			break;
	}

	t->resizeColumnsToContents();
	UPDATE_VIEW(triview_widget);
	bMod = true;
	undoButton->setEnabled(bCanUndo && bMod);
}

bool V3D_atlas_viewerDialog::updateTableItem_Landmark(QTableWidget *t, int row, LocationSimple *p_landmark, bool b_createNewItems)
{
	if (!t || row<0 || row>=t->rowCount() || !p_landmark) {qDebug("Invalid input parameters in updateTableItem_Landmark().\n"); return false;}
	if (t->columnCount()<11) {qDebug("Incorrect table column number in updateTableItem_Landmark().\n"); return false;}
	if (!imgdata) {qDebug("The image data is invalid in updateTableItem_Landmark().\n"); return false;}

	int j=0;
	QTableWidgetItem *newItem;
	QString s;

	if (b_createNewItems)
	{
		//s = tr("%1").arg("on");
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);
		newItem->setCheckState(Qt::Checked);

		//s = tr("%1").arg("color");
		newItem = new QTableWidgetItem();	t->setItem(row, j++, newItem);
		newItem->setData(Qt::DecorationRole, VCOLOR(p_landmark->color));
		newItem->setData(Qt::DisplayRole, VCOLOR(p_landmark->color));

		s = tr("%1").arg(p_landmark->x);
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);
		s = tr("%1").arg(p_landmark->y);
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);
		s = tr("%1").arg(p_landmark->z);
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);

		//the following three will be later replaced by the real RGB pixel values. Left here temporarily
		s = tr("%1").arg(int(imgdata->at(p_landmark->x-1, p_landmark->y-1, p_landmark->z-1, 0)));
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);

		if (imgdata->getCDim()>=2)
			s = tr("%1").arg(int(imgdata->at(p_landmark->x-1, p_landmark->y-1, p_landmark->z-1, 1)));
		else
			s = tr("%1").arg(0);
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);

		if (imgdata->getCDim()>=3)
			s = tr("%1").arg(int(imgdata->at(p_landmark->x-1, p_landmark->y-1, p_landmark->z-1, 2)));
		else
			s = tr("%1").arg(0);
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);

		//name of the landmark
		s = tr("%1").arg(p_landmark->name.c_str());
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);
		s = tr("%1").arg(p_landmark->comments.c_str());
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);
		s = tr("%1").arg(p_landmark->category);
		newItem = new QTableWidgetItem(s);	t->setItem(row, j++, newItem);
	}
	else
	{
		//s = tr("%1").arg("on");
		newItem = t->item(row, j);
		if (newItem) newItem->setCheckState((p_landmark->on)?Qt::Checked:Qt::Unchecked);
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		//s = tr("%1").arg("color");
		newItem = t->item(row, j);
		if (newItem)
		{
			newItem->setData(Qt::DecorationRole, VCOLOR(p_landmark->color));
			newItem->setData(Qt::DisplayRole, VCOLOR(p_landmark->color));
		}
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		//x,y,z coordinates
		newItem = t->item(row, j);
		if (newItem) newItem->setText(tr("%1").arg(p_landmark->x));
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		newItem = t->item(row, j);
		if (newItem) newItem->setText(tr("%1").arg(p_landmark->y));
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		newItem = t->item(row, j);
		if (newItem) newItem->setText(tr("%1").arg(p_landmark->z));
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		//the following three will be later replaced by the real RGB pixel values. Left here temporarily
		newItem = t->item(row, j);
		if (newItem)
		{
			newItem->setText(tr("%1").arg(int(imgdata->at(p_landmark->x-1, p_landmark->y-1, p_landmark->z-1, 0))));
		}
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		newItem = t->item(row, j);
		if (newItem)
		{
			if (imgdata->getCDim()>=2)
				newItem->setText(tr("%1").arg(int(imgdata->at(p_landmark->x-1, p_landmark->y-1, p_landmark->z-1, 1))));
			else
				newItem->setText(tr("%1").arg(int(0)));
		}
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		newItem = t->item(row, j);
		if (newItem)
		{
			if (imgdata->getCDim()>=3)
				newItem->setText(tr("%1").arg(int(imgdata->at(p_landmark->x-1, p_landmark->y-1, p_landmark->z-1, 2))));
			else
				newItem->setText(tr("%1").arg(int(0)));
		}
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		//name of the landmark
		newItem = t->item(row, j);
		if (newItem) newItem->setText(tr("%1").arg(p_landmark->name.c_str()));
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		newItem = t->item(row, j);
		if (newItem) newItem->setText(tr("%1").arg(p_landmark->comments.c_str()));
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;

		newItem = t->item(row, j);
		if (newItem) newItem->setText(tr("%1").arg(p_landmark->category));
		else {qDebug("In updateTableItem_Landmark() the # of column ends at %d.\n", j); return false;}
		j++;
	}

	Q_ASSERT(j==t->columnCount());
	return true;
}

QTableWidget* V3D_atlas_viewerDialog::createTableLandmarks()
{
	My4DImage* r = imgdata;
	if (! r)  return 0;

	QStringList qsl;
	qsl <<"on/off" << "color" << "x"<<"y"<<"z" <<"R" << "G" <<"B"<<"name"<<"comment"<<"category";
	int row = r->listLandmarks.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		updateTableItem_Landmark(t, i, (LocationSimple *)&(r->listLandmarks.at(i)), true);
	}
	qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}


void V3D_atlas_viewerDialog::pickLandmark(int i, int j)
{
	//qDebug("it is pickLandmark\n");

	My4DImage* r = imgdata;
	if (! r)  return;
	if (i<0 || i>=r->listLandmarks.size())  return;
	if (!table) return;

	QTableWidget* t = table[2];
	QTableWidgetItem *newItem = t->item(i,j);

	switch(j)
	{
		case 0:
			r->listLandmarks[i].on = CHECKED_TO_BOOL(newItem->checkState());
			break;
		case 1:
			r->listLandmarks[i].color = RGBA8V(newItem->data(0));
			newItem->setData(Qt::DecorationRole, VCOLOR(r->listLandmarks[i].color));
			break;
	}

	t->resizeColumnsToContents();
	UPDATE_VIEW(triview_widget);
	bMod = true;
	undoButton->setEnabled(bCanUndo && bMod);

	//now update the tri-view of the image window if possible
	r->getXWidget()->forceToChangeFocus(int(r->listLandmarks[i].x), int(r->listLandmarks[i].y), int(r->listLandmarks[i].z));
}

void V3D_atlas_viewerDialog::highlightLandmark(int i, int j, int previous_row, int previous_col)
{
	My4DImage* r = imgdata;
	if (! r)  return;
	if (i<0 || i>=r->listLandmarks.size())  return;
	if (!table) return;

	//now update the tri-view of the image window if possible
	r->getXWidget()->forceToChangeFocus(int(r->listLandmarks[i].x), int(r->listLandmarks[i].y), int(r->listLandmarks[i].z));
}

//

void V3D_atlas_viewerDialog::seeLandmarkProperty()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	int b_first=0;
	for (int i=t->rowCount()-1; i>=0; i--)
	{
		if (b_first==1) //only allow editing the first selected mark, to avoid very V3DLONG loop when happen to select many many markers!
			break;

		QTableWidgetItem * item = t->item(i,1);
		if (item->isSelected())
		{
			LandmarkPropertyDialog *landmarkView = NULL;
			landmarkView = new LandmarkPropertyDialog(&(imgdata->listLandmarks), i, imgdata);
			if (landmarkView->exec()!=QDialog::Accepted)
			{
				if (landmarkView) {delete landmarkView; landmarkView = NULL;}
				break; //only return true when the results are accepted, which will lead to an update operation below
			}
			landmarkView->fetchData(&(imgdata->listLandmarks), i);
                        qDebug("edit landmark [%d]. data fetched [%s][%s][%d]", i,
				   imgdata->listLandmarks.at(i).name.c_str(), imgdata->listLandmarks.at(i).comments.c_str(),  int(imgdata->listLandmarks.at(i).shape));

			//inportant: set the shape of the landmark
			LocationSimple * p_tmp_location = (LocationSimple *) & (imgdata->listLandmarks.at(i));
			switch (p_tmp_location->shape)
			{
				case pxSphere:	p_tmp_location->inputProperty = pxLocaUseful; break;
				case pxCube: p_tmp_location->inputProperty = pxLocaNotUseful; break;
				default: p_tmp_location->inputProperty = pxLocaUnsure; break;
			}

			if (landmarkView) {delete landmarkView; landmarkView = NULL;}

			imgdata->updateViews();

			b_first=1;
		}
	}

	//the folowing bMod is not easy to set. So basically I don't allow undo here
//	bMod = true;
//	undoButton->setEnabled(bCanUndo && bMod);
}

void V3D_atlas_viewerDialog::moveLandmarkUp()
{
	My4DImage* r = imgdata;
	if (! r)  return;
	if (!table) return;
	QTableWidget* t = table[2];
	int i = t->currentRow();
	if (i<0 || i>=r->listLandmarks.size())  {qDebug("Invalid index in moveLandmarkUp()."); return;}
	if (i==0) return; //do nothing, because it is the first one already
	r->listLandmarks.move(i,i-1);

	//this way is too slow
//	PROGRESS_DIALOG("Moving up     ", this);
//	PROGRESS_PERCENT(30);
//	reCreateTables(triview_widget);
//	PROGRESS_PERCENT(100);
//	UPDATE_VIEW(triview_widget);

	//a faster way
	updateTableItem_Landmark(t, i-1, (LocationSimple *)&(r->listLandmarks.at(i-1)), false);
	updateTableItem_Landmark(t, i, (LocationSimple *)&(r->listLandmarks.at(i)), false);
	t->setCurrentCell(i-1, 2);
}
void V3D_atlas_viewerDialog::moveLandmarkDown()
{
	My4DImage* r = imgdata;
	if (! r)  return;
	if (!table) return;
	QTableWidget* t = table[2];
	int i = t->currentRow();
	if (i<0 || i>=r->listLandmarks.size())  {qDebug("Invalid index in moveLandmarkUp()."); return;}
	if (i==r->listLandmarks.size()-1) return; //do nothing, because it is the last one already
	r->listLandmarks.move(i,i+1);

	//this way is too slow
//	PROGRESS_DIALOG("Move down     ", this);
//	PROGRESS_PERCENT(30);
//	reCreateTables(triview_widget);
//	PROGRESS_PERCENT(100);
//
//	UPDATE_VIEW(triview_widget);

	//a faster way
	updateTableItem_Landmark(t, i, (LocationSimple *)&(r->listLandmarks.at(i)), false);
	updateTableItem_Landmark(t, i+1, (LocationSimple *)&(r->listLandmarks.at(i+1)), false);
	t->setCurrentCell(i+1, 2);
}

void V3D_atlas_viewerDialog::deleteSelectedLandmark()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	PROGRESS_DIALOG("Updating     ", this);
	PROGRESS_PERCENT(30);

	for (int i=t->rowCount()-1; i>=0; i--)
	{
		QTableWidgetItem * item = t->item(i,1);
		if (item->isSelected())
		{
			t->removeRow(i);
			imgdata->listLandmarks.removeAt(i);
		}
	}

	PROGRESS_PERCENT(100);

	bMod = true;
	undoButton->setEnabled(bCanUndo && bMod);
}


void V3D_atlas_viewerDialog::resetAllLandmarkNames()
{
	My4DImage* r = imgdata;
	if (! r)  return;
	if (!table) return;
	QTableWidget* t = table[2];
	QTableWidgetItem *ti;

	QString s;
	int i;
	LocationSimple *p;
	printf("begin reseting all landmarks' names\n");
	for ( i=0;i<r->listLandmarks.size(); i++)
	{
		printf("%d ", i);
		p = (LocationSimple *)&(r->listLandmarks.at(i));
		p->name = qPrintable(s.setNum(i+1).prepend("Landmark "));
		if (ti = t->item(i,8))
		{
			ti->setText(p->name.c_str());
			//t->setCurrentCell(i, 8);
		}
		else
		{
			printf("The item(i,8) is invalid.");
			break;
		}
	}
	printf("\ndone reseting all landmarks' names\n");

	//t->update();
	t->resizeColumnsToContents();

}
void V3D_atlas_viewerDialog::resetAllLandmarkComments()
{
	My4DImage* r = imgdata;
	if (! r)  return;
	if (!table) return;
	QTableWidget* t = table[2];
	QTableWidgetItem *ti;

	int i;
	LocationSimple *p;
	printf("begin reseting all landmarks' comments\n");
	for ( i=0;i<r->listLandmarks.size(); i++)
	{
		printf("%d ", i);
		p = (LocationSimple *)&(r->listLandmarks.at(i));
		p->comments = "";
		if (ti = t->item(i,9))
		{
			ti->setText(p->comments.c_str());
			//t->setCurrentCell(i, 9);
		}
		else
		{
			printf("The item(i,9) is invalid.");
			break;
		}
	}
	printf("\ndone reseting all landmarks' comments\n");

	//t->update();
	t->resizeColumnsToContents();
}

QTableWidget* V3D_atlas_viewerDialog::createColorChannelManager()
{
	My4DImage* r = imgdata;
	if (! r)  return 0;

	QStringList qsl;
	qsl << "on/off" << "color" << "data channel";
	int row = r->getCDim();
	int col = qsl.size();

	if (r->listChannels.size() != row)
	{
		v3d_msg("Fail to run createColorChannelManager() as the number of color channels mismatches the color-mapping table.");
		return 0;
	}

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *newItem;
		QString s;

		//s = tr("%1").arg("on");
		newItem = new QTableWidgetItem(s);	t->setItem(i, j++, newItem);
		newItem->setCheckState(BOOL_TO_CHECKED(r->listChannels[i].on));

		//s = tr("%1").arg("color");
		newItem = new QTableWidgetItem(QVariant::Color);	t->setItem(i, j++, newItem);
		{
			// all of this is ok, caution of alpha. 081114
			//newItem->setBackgroundColor(QCOLOR(r->listLabelS[i].color));
			//newItem->setBackground(QBrush(QCOLOR(r->listLabelS[i].color)));
			//newItem->setIcon(colorIcon(QCOLOR(r->listLabelS[i].color)));
			newItem->setData(Qt::DecorationRole, VCOLOR(r->listChannels[i].color));
			newItem->setData(Qt::DisplayRole, VCOLOR(r->listChannels[i].color));
			//qDebug() << QCOLOR(r->listLabelS[i].color);
			//qDebug() << newItem->data(0).type();
		}

		s = tr("%1").arg(i+1);
		newItem = new QTableWidgetItem(s);	t->setItem(i, j++, newItem);

		Q_ASSERT(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void V3D_atlas_viewerDialog::pickColorChannel(int i, int j)
{
	My4DImage* r = imgdata;
	if (! r)  return;
	if (i<0 || i>=r->getCDim())  return;
	if (!table) return;

	if (r->listChannels.size() != r->getCDim())
	{
		v3d_msg("Fail to run pickColorChannel() as the number of color channels mismatches the color-mapping table.");
		return;
	}


	QTableWidget* t = table[3];
	QTableWidgetItem *newItem = t->item(i,j);

	switch(j)
	{
		case 0:
			r->listChannels[i].on = CHECKED_TO_BOOL(newItem->checkState());
			break;
		case 1:
			r->listChannels[i].color = RGBA8V(newItem->data(0));
			newItem->setData(Qt::DecorationRole, VCOLOR(r->listChannels[i].color));
			break;
	}

	updatedContent(t); //110805 RZC
}

#define  __use_channel_table__

ChannelTabWidget* V3D_atlas_viewerDialog::createChannelTab()
{
	ChannelTabWidget* channelTab=0;
	if (channelTab = new ChannelTabWidget(triview_widget, 2, false)) //2 tabs
	{
		//connect(triview_widget, SIGNAL(colorChanged(int)), channelTab, SLOT(updateXFormWidget(int)));
	}
	return channelTab;
}
void V3D_atlas_viewerDialog::deleteChannelTab(ChannelTabWidget* channelTab)
{
	if (channelTab)
	{
		//disconnect(triview_widget, SIGNAL(colorChanged(int)), channelTab, SLOT(updateXFormWidget(int)));
		channelTab->deleteLater();
	}
}
