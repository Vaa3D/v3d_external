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
 * V3dr_surfaceDialog.cpp
 *
 *  Created on: Nov 10, 2008
 *      Author: ruanzongcai
 * last edit: Hanchuan Peng, 090219: add the name/comment editor
 * 090521: by Hanchuan Peng. add the APOSet tab for point cloud atlas
 * 090720: by Hanchuan Peng. add COMMENT_ITEM_OF_TABLE macro and search comment as well
 */

#include "v3dr_surfaceDialog.h"
#include "../v3d/surfaceobj_annotation_dialog.h"



///////////////////////////////////////////////////////////
#define UPDATE_VIEW(w)   {if(w) w->update();}
#define ACTIVATE(w)	     {if(w) w->activateWindow();}
#define UPDATE_TABS()    {int i = getCurTab(); reCreateTables(glwidget); setCurTab(i);}
///////////////////////////////////////////////////////////


V3dr_surfaceDialog::V3dr_surfaceDialog(V3dR_GLWidget* w, QWidget* parent)
	:SharedDialog(w, parent)
{
	qDebug("V3dr_surfaceDialog::V3dr_surfaceDialog");

	init_members();//100809 RZC
	firstCreate();
	linkTo(w);/////

	setItemEditor();
    this->resize(1000,300);
}

V3dr_surfaceDialog::~V3dr_surfaceDialog()
{
	qDebug("V3dr_surfaceDialog::~V3dr_surfaceDialog");
	if (glwidget) glwidget->clearSurfaceDialog();
}


void V3dr_surfaceDialog::undo()
{
	qDebug("  V3dr_surfaceDialog::undo");

	int itab = getCurTab();

	if (bCanUndo && bMod && renderer)
	{
		// restore the old state
		renderer->listLabelSurf  = listLabelSurf;
		renderer->listNeuronTree = listNeuronTree;  //renderer->compileNeuronTreeList(); // for pre-compiled
		renderer->listCell       = listCell;
		renderer->listMarker     = listMarker;

		linkTo(glwidget);
	}

	setCurTab(itab);

	UPDATE_VIEW(glwidget);
	ACTIVATE(glwidget);
}


void V3dr_surfaceDialog::linkTo(QWidget* w)
{
	//100809 RZC
	active_widget = (V3dR_GLWidget*)w;
	if (bAttached) return;

	///////////////////////////////////////////////////////////////////////////
	qDebug("  V3dr_surfaceDialog::linkTo ( %p )", w);
	if (! w)  return;

	IncRef(w); //DecRef(w);
	qDebug("V3dr_surfaceDialog::ref = %d", ref);

	glwidget = (V3dR_GLWidget*)w;
	renderer = (Renderer_tex2*)(glwidget->getRenderer());

	clearTables_fromTab();
	createTables_addtoTab();
}

void V3dr_surfaceDialog::onAttached(bool b)
{
	//qDebug("  V3dr_surfaceDialog::onAttached = %d", b);

	//if (checkBox_attachedToCurrentView && checkBox_attachedToCurrentView->isChecked())
	if (b)
	{
		if (glwidget)
		{
			QString viewTitle = glwidget->getMainWindow()->windowTitle();
			setWindowTitle(title + tr(" attached: ") + viewTitle);
		}
		bAttached = true;
		//qDebug("  V3dr_surfaceDialog::( attached to %p )", glwidget);
	}
	else
	{
		bAttached = false;
		setWindowTitle(title);
		if (active_widget)
		{
			qDebug("  V3dr_surfaceDialog::( active of %p )", active_widget);
			active_widget->updateTool();
		}
	}
}

void V3dr_surfaceDialog::clearTables_fromTab()
{
	//qDebug("  V3dr_surfaceDialog::createTables");

	if (! tabOptions)	return;

	// clear tables to re-create
//	if (table[stImageMarker])      disconnect(table[stImageMarker], SIGNAL(cellChanged(int,int)), this, SLOT(pickMarker(int,int)));
//	if (table[stLabelSurface])     disconnect(table[stLabelSurface], SIGNAL(cellChanged(int,int)), this, SLOT(pickSurf(int,int)));
//	if (table[stNeuronStructure])  disconnect(table[stNeuronStructure], SIGNAL(cellChanged(int,int)), this, SLOT(pickSWC(int,int)));
//	if (table[stPointCloud])       disconnect(table[stPointCloud], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO(int,int)));
//	if (table[stPointSet])       disconnect(table[stPointSet], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO_Set(int,int)));
	for (int i=1; i<=5; i++)  if (table[i])
	{
		//delete table[i];  table[i]=0;		//this works well until June 09, so STRANGE !!!
		table[i]->deleteLater();  table[i]=0; //090707 RZC: deleteLater => postEvent(DeferredDele) => qDeleteInEventHandler()
	}
	tabOptions->clear();

	bCanUndo = bMod = false;
}

void V3dr_surfaceDialog::createTables_addtoTab()
{
	//qDebug("  V3dr_surfaceDialog::createTables");

	if (renderer)
	{
		// save the old state
		listLabelSurf  = renderer->listLabelSurf;
		listNeuronTree = renderer->listNeuronTree;
		listCell       = renderer->listCell;
		listMarker     = renderer->listMarker;
		bCanUndo = true;
		bMod = false;
		undoButton->setEnabled(bCanUndo && bMod);
	}

	table[stImageMarker]     = createTableMarker();
	table[stLabelSurface]    = createTableSurf();
	table[stNeuronStructure] = createTableSWC();
	table[stPointCloud]      = createTableAPO();
	table[stPointSet]     = createTableAPO_Set();

	//==========================================================================
	// for easy accessing tabs, addTab using the same order of V3dr_SurfaceType
	//==========================================================================
	if (tabOptions)
	{  // insertTab according to the order of enum v3dr_SurfaceType
		int i;
		QString qs;
		i= tabOptions->addTab(table[stImageMarker],		qs =QString("Marker (%1)").arg(table[stImageMarker]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		i= tabOptions->addTab(table[stLabelSurface],	qs =QString("Label Surface (%1)").arg(table[stLabelSurface]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		i= tabOptions->addTab(table[stNeuronStructure], qs =QString("Neuron/line Structure (%1)").arg(table[stNeuronStructure]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		i= tabOptions->addTab(table[stPointCloud],      qs =QString("Point Cloud (%1)").arg(table[stPointCloud]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		i= tabOptions->addTab(table[stPointSet],     qs =QString("Point Cloud Set (%1)").arg(table[stPointSet]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
	}


	//if (renderer)	connect(renderer, SIGNAL)
	if (table[stImageMarker])      connect(table[stImageMarker], SIGNAL(cellChanged(int,int)), this, SLOT(pickMarker(int,int)));
	if (table[stLabelSurface])     connect(table[stLabelSurface], SIGNAL(cellChanged(int,int)), this, SLOT(pickSurf(int,int)));
	if (table[stNeuronStructure])  connect(table[stNeuronStructure], SIGNAL(cellChanged(int,int)), this, SLOT(pickSWC(int,int)));
	if (table[stPointCloud])       connect(table[stPointCloud], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO(int,int)));
	if (table[stPointSet])      connect(table[stPointSet], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO_Set(int,int)));

	for (int i=1; i<=5; i++) if (table[i])
	{
		connect(table[i], SIGNAL(cellDoubleClicked(int,int)), this, SLOT(doubleClickHandler(int,int)));
		connect(table[i], SIGNAL(cellPressed(int,int)), this, SLOT(pressedClickHandler(int,int)));
	}

	for (int i=1; i<=5; i++) if (table[i])
	{
		table[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
//		table[i]->setEditTriggers(//QAbstractItemView::CurrentChanged |
//				QAbstractItemView::DoubleClicked |
//				QAbstractItemView::SelectedClicked);
	}
}


void V3dr_surfaceDialog::firstCreate()
{
	qDebug("  V3dr_surfaceDialog::firstCreate");

	QGroupBox* buttonGroup = new QGroupBox();
    QVBoxLayout *buttonRgnLayout = new QVBoxLayout(buttonGroup);

    QGroupBox* deleteGroup = new QGroupBox(); //
    QVBoxLayout *deleteLayout = new QVBoxLayout(deleteGroup);
    QPushButton *deleteButton = new QPushButton("Delete");
    deleteLayout->addWidget(deleteButton);

    QGroupBox* selectGroup = new QGroupBox();//"Select");
    QGridLayout *selectLayout = new QGridLayout(selectGroup);
    selectAllButton = new QPushButton("Select All");
    deselectAllButton = new QPushButton("Select None");
    inverseSelectButton = new QPushButton("Select Inverse");
    selectLayout->addWidget(selectAllButton,		1,0, 1,1+1);
    selectLayout->addWidget(deselectAllButton,		2,0, 1,1+1);
    selectLayout->addWidget(inverseSelectButton,	3,0, 1,1+1);

//	QGroupBox* changeGroup = new QGroupBox();//"Change");
//	QGridLayout *changeLayout = new QGridLayout(changeGroup);
    QGridLayout *changeLayout = selectLayout;
    onSelectButton = new QPushButton("On");
    offSelectButton = new QPushButton("Off");
    colorSelectButton = new QPushButton("Color >>");
    editNameCommentButton = new QPushButton("Name/Comments"); //by PHC, 090219
    undoButton = new QPushButton("Undo");
    changeLayout->addWidget(onSelectButton,  		1+3,0, 1,1);
    changeLayout->addWidget(offSelectButton, 		1+3,1, 1,1);
    changeLayout->addWidget(colorSelectButton,		2+3,0, 1,2);
    changeLayout->addWidget(editNameCommentButton,	3+3,0, 1,2);
    changeLayout->addWidget(undoButton,				4+3,0, 1,2);

    markerLocalView = new QPushButton("Local 3D View around Marker");

    QGroupBox* checkGroup = new QGroupBox("Options");
    QGridLayout *checkLayout = new QGridLayout(checkGroup);
    checkBox_attachedToCurrentView = new QCheckBox("Attached to 3D view");
    checkLayout->addWidget(checkBox_attachedToCurrentView,			1,0, 1,2);
    checkBox_accumulateLastHighlightHits = new QCheckBox("Accumulate last\n highlight search");
    checkLayout->addWidget(checkBox_accumulateLastHighlightHits,	2,0, 1,2);

    buttonRgnLayout->addWidget(selectGroup);
    //buttonRgnLayout->addWidget(changeGroup);
    //buttonRgnLayout->addWidget(deleteGroup);
    //buttonRgnLayout->addWidget(markerLocalView);
    buttonRgnLayout->addWidget(checkGroup);
    buttonRgnLayout->addStretch(0);
    buttonRgnLayout->setContentsMargins(0,0,0,0);

    /////////////////////////////////////////////////////////
    tabOptions = new QTabWidget(this); //tabOptions = new AutoTabWidget(this);//090117: commented by PHC
	/////////////////////////////////////////////////////////

	QGroupBox* tabAndBtnGroup = new QGroupBox();
    QHBoxLayout *tabAndBtnLayout = new QHBoxLayout(tabAndBtnGroup);
    tabAndBtnLayout->addWidget(tabOptions);
    tabAndBtnLayout->addWidget(buttonGroup);

	//search box
	QGroupBox* searchGroup = new QGroupBox();
	QHBoxLayout *searchLayout = new QHBoxLayout(searchGroup);
	searchTextEditLabel = new QLabel("Find in name/comment:");
	searchTextEdit = new QLineEdit;
	doSearchTextNext = new QPushButton("Next");
	doSearchTextPrev = new QPushButton("Previous");
	doSearchTextHighlightAllHits = new QPushButton("Highlight all hits");
	searchTextResultLabel = new QLabel("          "); //space left for display "not found" if nothing is found
	searchLayout->addWidget(searchTextEditLabel);
	searchLayout->addWidget(searchTextEdit);
	searchLayout->addWidget(doSearchTextNext);
	searchLayout->addWidget(doSearchTextPrev);
	searchLayout->addWidget(doSearchTextHighlightAllHits);
	searchLayout->addWidget(searchTextResultLabel);

	//overall layout
    QVBoxLayout *allLayout = new QVBoxLayout(this);
	allLayout->addWidget(tabAndBtnGroup);
	allLayout->addWidget(searchGroup);

	createMenuOfColor();

    if (okButton)		connect(okButton, SIGNAL(clicked()),    this, SLOT(accept()));
	if (cancelButton)	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	if (undoButton)	    connect(undoButton, SIGNAL(clicked()), this, SLOT(undo()));
	if (selectAllButton)	connect(selectAllButton, SIGNAL(clicked()),   this, SLOT(selectAll()));
	if (deselectAllButton)	connect(deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAll()));
	if (inverseSelectButton)	connect(inverseSelectButton, SIGNAL(clicked()), this, SLOT(selectInverse()));
	if (onSelectButton)		connect(onSelectButton, SIGNAL(clicked()),    this, SLOT(selectedOn()));
	if (offSelectButton)	connect(offSelectButton, SIGNAL(clicked()),   this, SLOT(selectedOff()));
	if (colorSelectButton)	connect(colorSelectButton, SIGNAL(clicked()),   this, SLOT(doMenuOfColor()));
	if (editNameCommentButton) connect(editNameCommentButton, SIGNAL(clicked()),   this, SLOT(editObjNameAndComments()));

	if (searchTextEdit && doSearchTextNext) connect(doSearchTextNext, SIGNAL(clicked()), this, SLOT(findNext()));
	if (searchTextEdit && doSearchTextPrev) connect(doSearchTextPrev, SIGNAL(clicked()), this, SLOT(findPrev()));
	if (searchTextEdit && doSearchTextHighlightAllHits) connect(doSearchTextHighlightAllHits, SIGNAL(clicked()), this, SLOT(findAllHighlight()));
	if (searchTextEdit) connect(searchTextEdit, SIGNAL(returnPressed()), this, SLOT(findNext()));

	//100809 RZC
	if (checkBox_attachedToCurrentView) connect( checkBox_attachedToCurrentView, SIGNAL(toggled(bool)), this, SLOT(onAttached(bool)) );
	if (markerLocalView) connect( markerLocalView, SIGNAL(clicked()), this, SLOT(onMarkerLocalView()) );

	setWindowTitle(title);
}

QTableWidget* V3dr_surfaceDialog::currentTableWidget()
{
	if (! tabOptions) return 0;

	int k = 1 + (tabOptions->currentIndex());
	//MESSAGE_ASSERT(k>=1 && k<=5);

	return table[k];
}

void V3dr_surfaceDialog::createMenuOfColor()
{
    QAction* Act;

    Act = new QAction(tr("Single Color..."), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(selectedColor()));
    menuColor.addAction(Act);

    Act = new QAction(tr("Hanchuan's Color Mapping"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(mapHanchuanColor()));
    menuColor.addAction(Act);

    Act = new QAction(tr("Random Color Mapping"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(mapRandomColor()));
    menuColor.addAction(Act);
}

void V3dr_surfaceDialog::doMenuOfColor()
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

void V3dr_surfaceDialog::updatedContent(QTableWidget* t) //090826
{
	if (! in_batch_stack.empty() && in_batch_stack.last()==true) return; //skip until end_batch

	t->resizeColumnsToContents();

	UPDATE_VIEW(glwidget);
	ACTIVATE(glwidget);

	bMod = true;
	undoButton->setEnabled(bCanUndo && bMod);
}


void V3dr_surfaceDialog::selectAll()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	t->selectAll();
}

void V3dr_surfaceDialog::deselectAll()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	t->clearSelection();
}

void V3dr_surfaceDialog::selectInverse()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	PROGRESS_DIALOG("Updating selection   ", this);
	PROGRESS_PERCENT(1); //since Qt 4.5

	QList<QTableWidgetSelectionRange> list_range = t->selectedRanges();
	t->selectAll();
	for (int i=0; i<list_range.size(); i++)
	{
		PROGRESS_PERCENT(i*100/list_range.size());
		t->setRangeSelected(list_range.at(i), false);
	}
//		t->setCurrentCell(i,0, QItemSelectionModel::Rows | QItemSelectionModel::Toggle);
//
//		QTableWidgetItem * item = t->item(i,0);
//		bool b = item->isSelected();
//		for (int j=0; j<t->columnCount(); j++)
//		{
//			item = t->item(i,j);
//			item->setSelected( ! b );
//		}
//	}

	PROGRESS_PERCENT(100);
}

///////////////////////////////////////////////////////////////////////////////
//unsigned char hanchuan_colortable[][3]={
//    {255,   0,    0},
//    {  0, 255,    0},
//    {  0,   0,  255},
//    {255, 255,    0},
//    {  0, 255,  255},
//    {255,   0,  255},
//    {255, 128,    0},
//    {  0, 255,  128},
//    {128,   0,  255},
//    {128, 255,    0},
//    {  0, 128,  255},
//    {255,   0,  128},
//    {128,   0,    0},
//    {  0, 128,    0},
//    {  0,   0,  128},
//    {128, 128,    0},
//    {  0, 128,  128},
//    {128,   0,  128},
//    {255, 128,  128},
//    {128, 255,  128},
//    {128, 128,  255},
//};
//int hanchuan_colortable_size = sizeof(hanchuan_colortable)/3;

#define QCOLOR(rgba8)   QColorFromRGBA8( rgba8 )
#define VCOLOR(rgba8)   qVariantFromValue(QColorFromRGBA8( rgba8 ))
#define QCOLORV(var)    (qVariantValue<QColor>( var ))
#define RGBA8V(var)     RGBA8FromQColor(qVariantValue<QColor>( var ))

#define UPATE_ITEM_ICON(curItem)   curItem->setData(Qt::DecorationRole, curItem->data(0))


void V3dr_surfaceDialog::setItemEditor()
{
	//::setItemEditor();

	// turn off item editor
	QItemEditorFactory::setDefaultFactory( new QItemEditorFactory(*QItemEditorFactory::defaultFactory()) );
}

void V3dr_surfaceDialog::pressedClickHandler(int i, int j)
{
	if (QApplication::mouseButtons()==Qt::RightButton)
	{
		qDebug("	pressedClickHandler( %d, %d ) rightButton", i,j);

		QTableWidget* t = currentTableWidget();
		//QTableWidgetItem *curItem = t->item(i,j);

		if (t==table[stImageMarker])
		{
			qDebug("  marker #%d", i+1);
			last_marker = i;

			QMenu menu;
			QAction* Act;
		    Act = new QAction(tr("Local 3D View around this Marker"), this);
		    connect(Act, SIGNAL(triggered()), this, SLOT(onMarkerLocalView()) );
		    menu.addAction(Act);
			menu.exec(QCursor::pos());
		}
	}
}

void V3dr_surfaceDialog::doubleClickHandler(int i, int j)
{
	qDebug("	doubleClickHandler( %d, %d )", i,j);

	QTableWidget* t = currentTableWidget();
	QTableWidgetItem *curItem = t->item(i,j);

	if (j==1) // color
	{
		//hide(); // make the getColorDialog to active

		//t->editItem(curItem); // QItemEditorFactory->registerEditor->getColor

		QColor qcolor = QCOLORV(curItem->data(0));
		if (! v3dr_getColorDialog( &qcolor))  return;
		curItem->setData(0, qVariantFromValue(qcolor));

		//show();
	}
}

void V3dr_surfaceDialog::selectedOnOff(bool state)
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	PROGRESS_DIALOG("Updating on/off    ", this);
	begin_batch();

	QList<QTableWidgetSelectionRange> list_range = t->selectedRanges();
	for (int ii=0; ii<list_range.size(); ii++)
	{
		PROGRESS_PERCENT(ii*100/list_range.size());

		int row0 = list_range.at(ii).topRow();
		int row1 = list_range.at(ii).bottomRow();
		for (int i=row0; i<=row1; i++)
		{
			QTableWidgetItem * curItem = t->item(i,0);
			curItem->setCheckState(BOOL_TO_CHECKED( state ));
		}
	}
//	for (int i=0; i<t->rowCount(); i++)
//	{
//		PROGRESS_PERCENT(i*100/t->rowCount());
//
//		QTableWidgetItem * curItem = t->item(i,0);
//		if (curItem->isSelected())  curItem->setCheckState(BOOL_TO_CHECKED( state ));
//	}

	end_batch();
	PROGRESS_PERCENT(100);

	updatedContent(t);
}

void V3dr_surfaceDialog::selectedColor(int map)
{
	QTableWidget* t = currentTableWidget();
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

	int n_row = t->rowCount();
	for (int i=0; i<n_row; i++)
	{
		PROGRESS_PERCENT(i*100/n_row);

		QTableWidgetItem * curItem = t->item(i,1);
		QColor qcolor = qcolor0;

		if (map==-1)      //random color
		{
			qcolor = QCOLOR(random_rgba8());
		}
		else if (map==1)  //hanchuan' color table
		{
			int j = i%hanchuan_colortable_size();
			qcolor = QColor(hanchuan_colortable[j][0],hanchuan_colortable[j][1],hanchuan_colortable[j][2]);
		}
		else //map==0
			if (! curItem->isSelected()) continue; // skip un-selected

		curItem->setData(0, qVariantFromValue(qcolor));
		//UPATE_ITEM_ICON(curItem); //this will be called in slot connected cellChanged()
	}

	end_batch();
	PROGRESS_PERCENT(100);

	updatedContent(t);
}


#define ADD_ONOFF(b)	{curItem = new QTableWidgetItem();	t->setItem(i, j++, curItem); \
						curItem->setCheckState(BOOL_TO_CHECKED(b));}

#define ADD_QCOLOR(c)	{curItem = new QTableWidgetItem(QVariant::Color);	t->setItem(i, j++, curItem); \
						curItem->setData(0, VCOLOR(c)); \
						curItem->setData(Qt::DecorationRole, VCOLOR(c));}

#define ADD_STRING(s)	{curItem = new QTableWidgetItem(s);	t->setItem(i, j++, curItem);}


QTableWidget* V3dr_surfaceDialog::createTableSurf()
{
	Renderer_tex2* r = renderer;
	if (! r)  return 0;

	QStringList qsl;
	qsl << "on/off" << "color" << "label" << "name" << "comment";
	int row = r->listLabelSurf.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;

		ADD_ONOFF(r->listLabelSurf[i].on);

		ADD_QCOLOR(r->listLabelSurf[i].color);
//		curItem = new QTableWidgetItem(QVariant::Color);	t->setItem(i, j++, curItem);
//		{// all of this is ok, caution of alpha. 081114
//		//curItem->setBackgroundColor(QCOLOR(r->listLabelS[i].color));
//		//curItem->setBackground(QBrush(QCOLOR(r->listLabelS[i].color)));
//		//curItem->setIcon(colorIcon(QCOLOR(r->listLabelS[i].color)));
//		curItem->setData(Qt::DecorationRole, VCOLOR(r->listLabelSurf[i].color));
//		curItem->setData(Qt::DisplayRole, VCOLOR(r->listLabelSurf[i].color));
//		//qDebug() << QCOLOR(r->listLabelS[i].color);
//		//qDebug() << curItem->data(0).type();
//		}

		ADD_STRING( tr("%1").arg(r->listLabelSurf[i].label) );

		ADD_STRING( r->listLabelSurf[i].name );

		ADD_STRING( r->listLabelSurf[i].comment );

		MESSAGE_ASSERT(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void V3dr_surfaceDialog::pickSurf(int i, int j)
{
	Renderer_tex2* r = renderer;
	if (! r)  return;
	if (i<0 || i>=r->listLabelSurf.size())  return;

	QTableWidget* t = table[stLabelSurface];
	QTableWidgetItem *curItem = t->item(i,j);

	switch(j)
	{
	case 0:
		r->listLabelSurf[i].on = CHECKED_TO_BOOL(curItem->checkState());
		break;
	case 1:
		r->listLabelSurf[i].color = RGBA8V(curItem->data(0));
		UPATE_ITEM_ICON(curItem);
		break;
	}

	updatedContent(t);
}

///////////////////////////////////////////////////////////////////////////////

QTableWidget* V3dr_surfaceDialog::createTableMarker()
{
	Renderer_tex2* r = renderer;
	if (! r)  return 0;

	QStringList qsl;
	qsl <<"on/off" << "color" << "type" << "x"<<"y"<<"z" << "name" << "comment";
	int row = r->listMarker.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;

		ADD_ONOFF(r->listMarker[i].on);

		ADD_QCOLOR(r->listMarker[i].color);

		ADD_STRING( tr("%1").arg(r->listMarker[i].type) );

		ADD_STRING( tr("%1").arg(r->listMarker[i].x) );
		ADD_STRING( tr("%1").arg(r->listMarker[i].y) );
		ADD_STRING( tr("%1").arg(r->listMarker[i].z) );

		ADD_STRING( r->listMarker[i].name );

		ADD_STRING( r->listMarker[i].comment );

		MESSAGE_ASSERT(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void V3dr_surfaceDialog::pickMarker(int i, int j)
{
	Renderer_tex2* r = renderer;
	if (! r)  return;
	if (i<0 || i>=r->listMarker.size())  return;

	QTableWidget* t = table[stImageMarker];
	QTableWidgetItem *curItem = t->item(i,j);

	switch(j)
	{
	case 0:
		r->listMarker[i].on = CHECKED_TO_BOOL(curItem->checkState());
		break;
	case 1:
		r->listMarker[i].color = RGBA8V(curItem->data(0));
		UPATE_ITEM_ICON(curItem);
		break;
	}

	updatedContent(t);
}

////////////////////////////////////////////////////////////////////////

QTableWidget* V3dr_surfaceDialog::createTableSWC()
{
	Renderer_tex2* r = renderer;
	if (! r)  return 0;

	QStringList qsl;
	qsl << "on/off" << "color" << "count" << "editing" << "name" << "comment"<< "file name";
	int row = (r->listNeuronTree.size());
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;

		ADD_ONOFF(r->listNeuronTree[i].on);

		ADD_QCOLOR(r->listNeuronTree[i].color);

		ADD_STRING( tr("%1").arg(r->listNeuronTree[i].listNeuron.size()) );

		if (r->listNeuronTree[i].editable) {
			ADD_STRING( tr("Yes") );
		} else
			ADD_STRING( tr("") );

		ADD_STRING( r->listNeuronTree[i].name ); //by PHC, add a column of name, which is different from file name. 090219

		ADD_STRING( r->listNeuronTree[i].comment );

		ADD_STRING( r->listNeuronTree[i].file );

		MESSAGE_ASSERT(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void V3dr_surfaceDialog::pickSWC(int i, int j)
{
	//qDebug("	pickSWC( %d, %d )", i,j);

	Renderer_tex2* r = renderer;
	if (! r)  return;
	if (i<0 || i>=r->listNeuronTree.size())  return;

	QTableWidget* t = table[stNeuronStructure];
	QTableWidgetItem *curItem = t->item(i,j);

	switch(j)
	{
	case 0:
		r->listNeuronTree[i].on = CHECKED_TO_BOOL(curItem->checkState());
		break;
	case 1:
		r->listNeuronTree[i].color = RGBA8V(curItem->data(0));
		UPATE_ITEM_ICON(curItem);
		//r->compileNeuronTree(i);  //// for pre-compiled
		break;
	}

	updatedContent(t);
}


///////////////////////////////////////////////////////////////////////////////

QTableWidget* V3dr_surfaceDialog::createTableAPO()
{
	Renderer_tex2* r = renderer;
	if (! r)  return 0;

	QStringList qsl;
	qsl <<"on/off" << "color" << "x"<<"y"<<"z" <<"intensity" <<"volume" << "name" << "comment" <<"in file";
	int row = r->listCell.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	//qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;

		ADD_ONOFF(r->listCell[i].on);

		ADD_QCOLOR(r->listCell[i].color);

		ADD_STRING( tr("%1").arg(r->listCell[i].x) );
		ADD_STRING( tr("%1").arg(r->listCell[i].y) );
		ADD_STRING( tr("%1").arg(r->listCell[i].z) );

		ADD_STRING( tr("%1").arg(r->listCell[i].intensity) );

		ADD_STRING( tr("%1").arg(r->listCell[i].volsize) );

		ADD_STRING( r->listCell[i].name );

		ADD_STRING( r->listCell[i].comment );

		ADD_STRING( r->map_CellIndex_APOFile.value(i) );

		MESSAGE_ASSERT(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void V3dr_surfaceDialog::pickAPO(int i, int j)
{
	Renderer_tex2* r = renderer;
	if (! r)  return;
	if (i<0 || i>=r->listCell.size())  return;

	QTableWidget* t = table[stPointCloud];
	QTableWidgetItem *curItem = t->item(i,j);

	switch(j)
	{
	case 0:
		r->listCell[i].on = CHECKED_TO_BOOL(curItem->checkState());
		break;
	case 1:
		r->listCell[i].color = RGBA8V(curItem->data(0));
		UPATE_ITEM_ICON(curItem);
		break;
	}

	updatedContent(t);
}

///////////////////////////////////////////////////////////////////////////////

QTableWidget* V3dr_surfaceDialog::createTableAPO_Set()
{
	Renderer_tex2* r = renderer;
	if (! r)  return 0;

	QStringList qsl;
	qsl <<"on/off" << "color"<< "count"  << "set/class name" << "comment"  << "file name";
	int row = r->map_APOFile_IndexList.size();
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	//qDebug("  create begin t->rowCount = %d", t->rowCount());

	QList <QString> apoSetNameList = r->map_APOFile_IndexList.keys();

	//qDebug("map.size = %d, map.keys.size = %d", row, apoSetNameList.size());
	MESSAGE_ASSERT (apoSetNameList.size()==row);

	for (int i=0; i<row; i++)
	{
		QList<int> cind;
		cind = r->map_APOFile_IndexList.value(apoSetNameList[i]);

		int j=0;
		QTableWidgetItem *curItem;

		ADD_ONOFF(r->listCell[cind[0]].on);

		ADD_QCOLOR(r->listCell[cind[0]].color);

		ADD_STRING( tr("%1").arg(cind.size()) );

		ADD_STRING( QFileInfo(apoSetNameList[i]).baseName() );

		ADD_STRING( "" ); //set comments as empty first. by PHC, 090521

		ADD_STRING( apoSetNameList[i] );

		MESSAGE_ASSERT(j==col);
	}
	//qDebug("  end   t->rowCount = %d", t->rowCount());

	t->resizeColumnsToContents();
	return t;
}

void V3dr_surfaceDialog::pickAPO_Set(int i, int j)
{
	//qDebug("	pickAPO_Set( %d, %d )", i,j);

	Renderer_tex2* r = renderer;
	if (! r)  return;
	if (i<0 || i>=r->map_APOFile_IndexList.size())  return;

	QTableWidget* t = table[stPointSet];
	QTableWidgetItem *curItem = t->item(i,j);

	QList<int> cind;
	if (j==0 || j==1)
	{
		QList <QString> apoSetNameList = r->map_APOFile_IndexList.keys();
		cind = r->map_APOFile_IndexList.value(apoSetNameList[i]);
	}
	else return;

	int k;
//	for (k=0;k<cind.size();k++)		qDebug()<<cind[k] << " ";


	PROGRESS_DIALOG("Updating point cloud   ", this);
	begin_batch();

	bool cur_checkstate = CHECKED_TO_BOOL(curItem->checkState());
	RGBA8 cur_color = RGBA8V(curItem->data(0));
	switch(j)
	{
		case 0:
		{
			for (k=0;k<cind.size();k++)
			{
				PROGRESS_PERCENT(k*100/cind.size());

				r->listCell[cind[k]].on = cur_checkstate;
				table[stPointCloud]->item(cind[k],0)->setCheckState(curItem->checkState()); //also update the apo tab view check state
			}
			break;
		}
		case 1:
		{
			for (k=0;k<cind.size();k++)
			{
				PROGRESS_PERCENT(k*100/cind.size());

				r->listCell[cind[k]].color = cur_color;
				table[stPointCloud]->item(cind[k],1)->setData(0, VCOLOR(cur_color)); //also update the apo tab view color
			}
			UPATE_ITEM_ICON(curItem);
			break;
		}
	}

	end_batch();
	PROGRESS_PERCENT(100);

	updatedContent(t);
}


// find in name item
#define NAME_ITEM_OF_TABLE(curItem, t,i) \
{ \
	if (t==table[stImageMarker])		curItem = t->item(i, 6); \
	if (t==table[stLabelSurface]) 		curItem = t->item(i, 3); \
	if (t==table[stNeuronStructure]) 	curItem = t->item(i, 4); \
	if (t==table[stPointCloud]) 		curItem = t->item(i, 7); \
	if (t==table[stPointSet]) 			curItem = t->item(i, 3); \
}

#define COMMENT_ITEM_OF_TABLE(curItem, t,i) \
{ \
	if (t==table[stImageMarker])		curItem = t->item(i, 7); \
	if (t==table[stLabelSurface]) 		curItem = t->item(i, 4); \
	if (t==table[stNeuronStructure]) 	curItem = t->item(i, 5); \
	if (t==table[stPointCloud]) 		curItem = t->item(i, 8); \
	if (t==table[stPointSet]) 			curItem = t->item(i, 4); \
}

void V3dr_surfaceDialog::editObjNameAndComments() //090219 unfinished yet. need to think a good logic here
{
#ifndef test_main_cpp
	QTableWidget* t = currentTableWidget();
	if (! t) return;
	Renderer_tex2* r = renderer;
	if (! r)  return;

//	PROGRESS_DIALOG("Updating     ", this);
//	PROGRESS_PERCENT(30);
//
	QTableWidgetItem * item0=0, *curItem=0;
	QString realobj_name, realobj_comment;

	for (int i=0; i<t->rowCount(); i++)
	{
		item0 = t->item(i,0);
		if (item0->isSelected())
		{
			NAME_ITEM_OF_TABLE(curItem, t,i);
			realobj_name = curItem->text();

			COMMENT_ITEM_OF_TABLE(curItem, t,i)
			realobj_comment = curItem->text();

			SurfaceObjAnnotationDialog tmp_dialog(i+1, &realobj_name, &realobj_comment);
			int res = tmp_dialog.exec();
			if (res==QDialog::Accepted)
			{
				tmp_dialog.fetchData(&realobj_name, &realobj_comment);

				NAME_ITEM_OF_TABLE(curItem, t,i);
				curItem->setText(realobj_name); //update the obj manager

				COMMENT_ITEM_OF_TABLE(curItem, t,i)
				curItem->setText(realobj_comment);


				if     (t==table[stImageMarker])
				{
					r->listMarker[i].name = realobj_name;
					r->listMarker[i].comment = realobj_comment;
				}
				else if (t==table[stLabelSurface])
				{
					r->listLabelSurf[i].name = realobj_name;
					r->listLabelSurf[i].comment = realobj_comment;
				}
				else if (t==table[stNeuronStructure])
				{
					r->listNeuronTree[i].name = realobj_name;
					r->listNeuronTree[i].comment = realobj_comment;
				}
				else if (t==table[stPointCloud])
				{
					r->listCell[i].name = realobj_name;
					r->listCell[i].comment = realobj_comment;
				}
				else if (t==table[stPointSet])
				{
				}
			}
		}
	}
//
//	PROGRESS_PERCENT(100);
//	ACTIVATE(glwidget);
//
	bMod = true;
	undoButton->setEnabled(bCanUndo && bMod);
#endif
}


void V3dr_surfaceDialog::findNext()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;
	if (!searchTextEdit) return;

	if (t->rowCount()<=0)
	{
		if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
		return;
	}
	if (searchTextResultLabel) searchTextResultLabel->setText("        ");

	QTableWidgetItem * item = 0;
	bool b_found=false;
	int row_start = t->currentRow()+1, i;
	for (i=row_start;i<t->rowCount();i++)
	{
		NAME_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}

		COMMENT_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}
	}
	for (i=0;i<row_start;i++)
	{
		NAME_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}

		COMMENT_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}

	}
	if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
}

void V3dr_surfaceDialog::findPrev()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;
	if (!searchTextEdit) return;

	if (t->rowCount()<=0)
	{
		if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
		return;
	}
	if (searchTextResultLabel) searchTextResultLabel->setText("        ");

	QTableWidgetItem * item = 0;
	bool b_found=false;
	int row_start = t->currentRow()-1, i;
	for (i=row_start;i>=0;i--)
	{
		NAME_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}

		COMMENT_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}
	}
	for (i=t->rowCount()-1;i>row_start;i--)
	{
		NAME_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}

		COMMENT_ITEM_OF_TABLE(item, t,i);

		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			t->scrollToItem(item);
			t->setCurrentItem(item);
			b_found=true;
			return; // found
		}
	}
	if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
}


void V3dr_surfaceDialog::findAllHighlight()
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;
	if (!searchTextEdit) return;

	if (t->rowCount()<=0)
	{
		if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
		return;
	}
	if (searchTextResultLabel) searchTextResultLabel->setText("        ");

	QTableWidgetItem * item = 0;
	bool b_found=false;
	int row_start = t->currentRow()-1, i;
	int cnt_found=0;

	int top, left=0, bottom, right=t->columnCount()-1;

	bool b_turnofflasthighlight = true;
	if (checkBox_accumulateLastHighlightHits)
		b_turnofflasthighlight = ! checkBox_accumulateLastHighlightHits->isChecked();

	if (b_turnofflasthighlight)
	{
		QTableWidgetSelectionRange sr(0,left,t->rowCount()-1,right);	t->setRangeSelected(sr, false);
	}

	for (i=0;i<t->rowCount();i++)
	{
		NAME_ITEM_OF_TABLE(item, t,i);

		b_found=false;
		if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
		{
			//t->scrollToItem(item);
			QTableWidgetSelectionRange sr(i,left,i,right);	t->setRangeSelected(sr, true);
			b_found=true;
			cnt_found++;
		}

		COMMENT_ITEM_OF_TABLE(item, t,i);

		if (b_found==false)
		{
			if (item  && item->text().contains(searchTextEdit->text(), Qt::CaseInsensitive))
			{
				QTableWidgetSelectionRange sr(i,left,i,right);	t->setRangeSelected(sr, true);
				b_found=true;
				cnt_found++;
			}
		}
	}

	if (searchTextResultLabel) searchTextResultLabel->setText("Not found");
	if (cnt_found>0)
		v3d_msg(QString("There are totally %1 hits.\n").arg(cnt_found), 1);
}

void V3dr_surfaceDialog::onMarkerLocalView()
{
	qDebug("  V3dr_surfaceDialog::onMarkerLocalView");

	Renderer_tex2* r = renderer;
	if (! r)  return;
	if (last_marker < 0 || last_marker >= r->listMarker.size()) return;

#ifndef test_main_cpp
	if (glwidget)
	{
		My4DImage* curImg = 0;         curImg = v3dr_getImage4d(r->_idep);
		XFormWidget* curXWidget = 0;   curXWidget = v3dr_getXWidget(r->_idep);

		if (curImg) curImg->cur_hit_landmark = last_marker;
		if (curXWidget) curXWidget->doImage3DLocalMarkerView();

		//glwidget->lookAlong(1,1,1);
	}
#endif
}

