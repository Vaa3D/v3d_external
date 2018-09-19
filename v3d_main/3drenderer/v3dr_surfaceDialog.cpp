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
#include "renderer_gl1.h"
#ifndef test_main_cpp
#include "../v3d/surfaceobj_annotation_dialog.h"
#endif

///////////////////////////////////////////////////////////
#define UPDATE_VIEW(w)   {if(w) w->update();}
#define ACTIVATE(w)	     {if(w) w->activateWindow();}
#define UPDATE_TABS()    {int i = getCurTab(); reCreateTables(glwidget); setCurTab(i);}
///////////////////////////////////////////////////////////

//
V3dr_surfaceDialog::V3dr_surfaceDialog(V3dR_GLWidget* w, QWidget* parent)
	:SharedToolDialog(w, parent)
{
	qDebug("V3dr_surfaceDialog::V3dr_surfaceDialog");

	init_members();//100809 RZC

	setItemEditor();
	createFirst();
	linkTo(w);/////

    this->resize(1200,300);
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
	if (! w)  return;

	IncRef(w); //always add to reflist
	qDebug("  V3dr_surfaceDialog::linkTo ( %p )  ref=%d", w, ref);

	//100809, 110713
	tolink_widget = (V3dR_GLWidget*)w;
	if (bAttached && tolink_widget != glwidget) //attached
	{
		widget = (QWidget*)glwidget; //restore widget
		return;               //so only update with attached widget
	}

	glwidget = (V3dR_GLWidget*)w;
	renderer = (Renderer_gl1*)(glwidget->getRenderer());

	iLastTab = getCurTab(); //110713
	clearTables_fromTab();
	createTables_addtoTab();
}

int V3dr_surfaceDialog::DecRef(QWidget* w) //110713
{

	int ref = SharedToolDialog::DecRef(w);
	qDebug("  V3dr_surfaceDialog::DecRef ( %p )  ref=%d", w, ref);

	if (bAttached && widget == 0)
	{
		if (ref>0) //110722, not deleted
			checkBox_attachedToCurrentView->setChecked(false);

		if (glwidget) glwidget->clearSurfaceDialog(); //110722, must do it before set glwidget = 0
		glwidget = 0;
		renderer = 0;
	}
	return ref;
}

void V3dr_surfaceDialog::onAttached(bool b)
{
	//qDebug("  V3dr_surfaceDialog::onAttached = %d", b);

	//if (isHidden())  return;
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
		if (tolink_widget)
		{
			tolink_widget = (V3dR_GLWidget*)bestLinkable(tolink_widget);
			qDebug("  V3dr_surfaceDialog::onAttached( tolink %p )", tolink_widget);
			if (tolink_widget) //110722
				tolink_widget->updateTool();
		}
	}
}

void V3dr_surfaceDialog::clearTables_fromTab()
{
	//qDebug("  V3dr_surfaceDialog::clearTables_fromTab");

	// clear tables to re-create
//	if (table[stImageMarker])      disconnect(table[stImageMarker], SIGNAL(cellChanged(int,int)), this, SLOT(pickMarker(int,int)));
//	if (table[stLabelSurface])     disconnect(table[stLabelSurface], SIGNAL(cellChanged(int,int)), this, SLOT(pickSurf(int,int)));
//	if (table[stNeuronStructure])  disconnect(table[stNeuronStructure], SIGNAL(cellChanged(int,int)), this, SLOT(pickSWC(int,int)));
//	if (table[stPointCloud])       disconnect(table[stPointCloud], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO(int,int)));
//	if (table[stPointSet])       disconnect(table[stPointSet], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO_Set(int,int)));
	for (int i=1; i<=6; i++)
		if (table[i])
	{
		//delete table[i];  table[i]=0;		//this works well until June 09, so STRANGE !!!
		table[i]->deleteLater();  table[i]=0; //090707 RZC: deleteLater => postEvent(DeferredDelete) => qDeleteInEventHandler()
	}

	if (tabOptions)   tabOptions->clear();

	bCanUndo = bMod = false;
}

void V3dr_surfaceDialog::createTables_addtoTab()
{
	//qDebug("  V3dr_surfaceDialog::createTables_addtoTab");

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
	table[stNeuronSegment] = createTableNeuronSegment();
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
		i= tabOptions->addTab(table[stNeuronSegment], qs =QString("Neuron Segment (%1)").arg(table[stNeuronSegment]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		i= tabOptions->addTab(table[stNeuronStructure], qs =QString("Neuron/line Structure (%1)").arg(table[stNeuronStructure]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		i= tabOptions->addTab(table[stPointCloud],      qs =QString("Point Cloud (%1)").arg(table[stPointCloud]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
		i= tabOptions->addTab(table[stPointSet],     qs =QString("Point Cloud Set (%1)").arg(table[stPointSet]->rowCount()));
		tabOptions->setTabToolTip(i, qs);
	}


	//connect cell to table handler
	if (table[stImageMarker])      connect(table[stImageMarker], SIGNAL(cellChanged(int,int)), this, SLOT(pickMarker(int,int)));
	if (table[stLabelSurface])     connect(table[stLabelSurface], SIGNAL(cellChanged(int,int)), this, SLOT(pickSurf(int,int)));
	if (table[stNeuronSegment])  connect(table[stNeuronSegment], SIGNAL(cellChanged(int,int)), this, SLOT(pickNeuronSegment(int,int)));
	if (table[stNeuronStructure])  connect(table[stNeuronStructure], SIGNAL(cellChanged(int,int)), this, SLOT(pickSWC(int,int)));
	if (table[stPointCloud])       connect(table[stPointCloud], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO(int,int)));
	if (table[stPointSet])      connect(table[stPointSet], SIGNAL(cellChanged(int,int)), this, SLOT(pickAPO_Set(int,int)));
	
	for (int i=1; i<=6; i++)
		if (table[i])
	{
		table[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
		table[i]->setEditTriggers(QAbstractItemView::NoEditTriggers);
//		table[i]->setEditTriggers(//QAbstractItemView::CurrentChanged |
//				QAbstractItemView::DoubleClicked |
//				QAbstractItemView::SelectedClicked);
		//use doubleClickHandler() to override delay of popping dialog by the setEditTriggers
		connect(table[i], SIGNAL(cellDoubleClicked(int,int)), this, SLOT(doubleClickHandler(int,int))); //to override delay of popping dialog by the setEditTriggers
		connect(table[i], SIGNAL(cellPressed(int,int)), this, SLOT(pressedClickHandler(int,int)));      //to pop context menu
		}
}


void V3dr_surfaceDialog::createFirst()
{
	QGroupBox* buttonGroup = new QGroupBox();
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonGroup);

    QGroupBox* deleteGroup = new QGroupBox(); //
    QVBoxLayout *deleteLayout = new QVBoxLayout(deleteGroup);
    QPushButton *deleteButton = new QPushButton("Delete");
    deleteLayout->addWidget(deleteButton);

    QGroupBox* selectGroup = new QGroupBox();//"Select");
    QGridLayout *selectLayout = new QGridLayout(selectGroup);
    selectAllButton = new QPushButton("Select All");
    deselectAllButton = new QPushButton("Select None");
    inverseSelectButton = new QPushButton("Select Inverse");
    selectLayout->addWidget(selectAllButton,		1,0, 1,2);
    selectLayout->addWidget(deselectAllButton,		2,0, 1,2);
    selectLayout->addWidget(inverseSelectButton,	3,0, 1,2);

//	QGroupBox* changeGroup = new QGroupBox();//"Change");
//	QGridLayout *changeLayout = new QGridLayout(changeGroup);
    QGridLayout *changeLayout = selectLayout;
    onSelectButton = new QPushButton("On");
    offSelectButton = new QPushButton("Off");
    colorSelectButton = new QPushButton("Color >>");
    objectSetDisplayModeButton = new QPushButton("Display Mode >>"); //by PHC 20130926
    editNameCommentButton = new QPushButton("Name/Comments"); //by PHC, 090219
    neuronSegmentType = new QPushButton("NeuronSegmentType");
    undoButton = new QPushButton("Undo");
    changeLayout->addWidget(onSelectButton,  		1+3,0, 1,1);
    changeLayout->addWidget(offSelectButton, 		1+3,1, 1,1);
    changeLayout->addWidget(colorSelectButton,		2+3,0, 1,2);
    changeLayout->addWidget(objectSetDisplayModeButton,		3+3,0, 1,2);
    changeLayout->addWidget(editNameCommentButton,	4+3,0, 1,2);
    changeLayout->addWidget(neuronSegmentType,      5+3,0, 1,2);
    changeLayout->addWidget(undoButton,				6+3,0, 1,2);

//    markerLocalView = new QPushButton("Local 3D View around Marker");

    QGroupBox* checkGroup = new QGroupBox("Options");
    QGridLayout *checkLayout = new QGridLayout(checkGroup);
    checkBox_attachedToCurrentView = new QCheckBox("Attached to 3D view");
    checkLayout->addWidget(checkBox_attachedToCurrentView,			1,0, 1,2);
    checkBox_accumulateLastHighlightHits = new QCheckBox("Accumulate last\n highlight search");
    checkLayout->addWidget(checkBox_accumulateLastHighlightHits,	2,0, 1,2);

    buttonLayout->addWidget(selectGroup);
    //buttonLayout->addWidget(changeGroup);
    //buttonLayout->addWidget(deleteGroup);
    //buttonLayout->addWidget(markerLocalView);
    buttonLayout->addWidget(checkGroup);
    buttonLayout->addStretch(0);
    buttonLayout->setContentsMargins(0,0,0,0);

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
	HALF_MARGINS(allLayout);

	createMenuOfColor();
    createMenuOfDisplayMode();

    if (okButton)		connect(okButton, SIGNAL(clicked()),    this, SLOT(accept()));
	if (cancelButton)	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	if (undoButton)	    connect(undoButton, SIGNAL(clicked()), this, SLOT(undo()));
	if (selectAllButton)	connect(selectAllButton, SIGNAL(clicked()),   this, SLOT(selectAll()));
	if (deselectAllButton)	connect(deselectAllButton, SIGNAL(clicked()), this, SLOT(deselectAll()));
	if (inverseSelectButton)	connect(inverseSelectButton, SIGNAL(clicked()), this, SLOT(selectInverse()));
	if (onSelectButton)		connect(onSelectButton, SIGNAL(clicked()),    this, SLOT(selectedOn()));
	if (offSelectButton)	connect(offSelectButton, SIGNAL(clicked()),   this, SLOT(selectedOff()));
	if (colorSelectButton)	connect(colorSelectButton, SIGNAL(clicked()),   this, SLOT(doMenuOfColor()));
    if (objectSetDisplayModeButton) connect(objectSetDisplayModeButton, SIGNAL(clicked()),   this, SLOT(doMenuOfDisplayMode()));
	if (editNameCommentButton) connect(editNameCommentButton, SIGNAL(clicked()),   this, SLOT(editObjNameAndComments()));
    if (neuronSegmentType) connect(neuronSegmentType, SIGNAL(clicked()),   this, SLOT(editNeuronSegmentType()));

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

    //180412 RZC
    menuColor.addSeparator();
    Act = new QAction(tr("Neuron Segment Colorful"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(mapSegmentColor()));
    menuColor.addAction(Act);
    Act = new QAction(tr("Multi-neuron Colorful"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(mapMultiNeuronColor()));
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
        v3d_msg("Fail to run the V3dr_surfaceDialog::doMenuOfColor() function.", 1);
	}
}

void V3dr_surfaceDialog::createMenuOfDisplayMode()
{
    QAction* Act;
	QAction* mesh27;
	QAction* mesh18;
	QAction* mesh9;
	QAction* meshDefault;
	QMenu* mesh_menu;

    Act = new QAction(tr("Use global setting..."), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(setSWCDisplayUsingGlobalSettings()));
    menuDisplayMode.addAction(Act);

    Act = new QAction(tr("Always line mode"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(setSWCDisplayUsingLine()));
    menuDisplayMode.addAction(Act);

    Act = new QAction(tr("Always tube mode"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(setSWCDisplayUsingTube()));
    menuDisplayMode.addAction(Act);

	/*mesh_menu = new QMenu(tr("Change mesh density of neuron surface"), this);
	menuDisplayMode.addMenu(mesh_menu);
	meshDefault = new QAction(tr("Default (36)"), this);
	connect(meshDefault, SIGNAL(triggered()), this, SLOT(setMeshDensityDefault()));
	mesh_menu->addAction(meshDefault);
	mesh27 = new QAction(tr("27"), this);
	connect(mesh27, SIGNAL(triggered()), this, SLOT(setMeshDensity27()));
	mesh_menu->addAction(mesh27);
	mesh18 = new QAction(tr("18"), this);
	connect(mesh18, SIGNAL(triggered()), this, SLOT(setMeshDensity18()));
	mesh_menu->addAction(mesh18);
	mesh9 = new QAction(tr("9"), this);
	connect(mesh9, SIGNAL(triggered()), this, SLOT(setMeshDensity9()));
	mesh_menu->addAction(mesh9);*/
}

/*void V3dr_surfaceDialog::setMeshDensity(int newMeshDensity)
{
	//cout << newMeshDensity << endl;
	Renderer_gl1* r = renderer;
	r->cleanObj();
	this->meshDensity = newMeshDensity;
	iDrawExternalParameter* idep = (iDrawExternalParameter*) r->_idep;
	//qDebug() << idep->swc_file_list;
	QString swcFileName = idep->swc_file_list[0];
	
	r->loadObj_meshChange(newMeshDensity);
	r->loadObjectFilename(swcFileName);
	return;
}*/

void V3dr_surfaceDialog::doMenuOfDisplayMode()
{
    try
    {
        menuDisplayMode.exec(QCursor::pos());
    }
    catch (...)
    {
        v3d_msg("Fail to run the V3dr_surfaceDialog::doMenuOfDisplayMode() function.", 1);
    }
}

void V3dr_surfaceDialog::setSWCDisplayMode(int v) //NOT sure if this will influence the Undo function. Need to check and test for that later. noted by PHC 20130926
{
    Renderer_gl1* r = renderer;
    if (! r)  return;

    QTableWidget* t = currentTableWidget();
    if (!t || !table || t!=table[stNeuronStructure])
        return;

    QString vs = "global";
    if (v==1) vs = "line";
    else if (v==0) vs = "tube";
    v3d_msg(vs, 0);

    PROGRESS_DIALOG("Updating display mode    ", this);
    begin_batch();

    V3DLONG n_row = t->rowCount();
    for (V3DLONG i=0; i<n_row; i++)
    {
        PROGRESS_PERCENT(i*100/n_row);

        QTableWidgetItem * curItem = t->item(i,3);
        if (curItem->isSelected()) // skip un-selected
        {
            r->listNeuronTree[i].linemode = v;
            curItem->setData(0, qVariantFromValue(vs));
        }
    }

    end_batch();
    PROGRESS_PERCENT(100);

    updatedContent(t);
}

///////////////////////////////////////////////////////////////////////

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


#define UPATE_ITEM_ICON(curItem)   curItem->setData(Qt::DecorationRole, curItem->data(0))

#define BOOL_TO_CHECKED(b) (Qt::CheckState(b*2))
#define INT_TO_CHECKED(b) (Qt::CheckState(b))
#define CHECKED_TO_INT(b) (int(b))
#define CHECKED_TO_BOOL(b) (int(b)>0)


void V3dr_surfaceDialog::setItemEditor()
{
	//::setItemEditor();
	//TURNOFF_ITEM_EDITOR(); //replaced with table->setEditTriggers(QAbstractItemView::NoEditTriggers)
}

void V3dr_surfaceDialog::pressedClickHandler(int i, int j)
{
	if (QApplication::mouseButtons()==Qt::RightButton) //right button menu
	{
		//qDebug("	pressedClickHandler( %d, %d ) rightButton", i,j);

		QTableWidget* t = currentTableWidget();
		//QTableWidgetItem *curItem = t->item(i,j);

		if (t==table[stImageMarker])
		{
            t->setSortingEnabled(true); // sort

			qDebug("  marker #%d", i+1);
			last_marker = i;

			QMenu menu;
			QAction* Act;
		    Act = new QAction(tr("Local 3D View around this Marker"), this);
		    connect(Act, SIGNAL(triggered()), this, SLOT(onMarkerLocalView()) );
            QAction* ZoomAct;
            ZoomAct=new QAction(tr("Zoom-in to this select marker location"),this);
			connect(ZoomAct, SIGNAL(triggered()), this, SLOT(menuExecBuffer()));
			menu.addAction(ZoomAct);
		    menu.addAction(Act);
			menu.exec(QCursor::pos());
		}
	}
}

void V3dr_surfaceDialog::menuExecBuffer()
{
	//  By using a QTimer::singleshot to call V3dr_surfaceDialog::zoomMarkerLocation, 
	//  it makes V3dr_surfaceDialog::pressedClickHandler finish menu.exec() once the signal is fired, so that the main thread can move on.
	//  This is an ad hoc solution for a known bug on Windows platform, where a crash happens when "zoom-in to this select marker" is called from object manager in terafly.
	// -- MK, June, 2018

	if (last_marker != -1)
	{
		QTimer::singleShot(50, this, SLOT(zoomMarkerLocation()));
	}
}

void V3dr_surfaceDialog::doubleClickHandler(int i, int j)
{
	//qDebug("	doubleClickHandler( %d, %d )", i,j);

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
	if (1 + (tabOptions->currentIndex()) == stNeuronSegment) isBatchOperation = true;
	
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	PROGRESS_DIALOG("Updating on/off    ", this);
	begin_batch();

	qDebug("flag1");
	QList<QTableWidgetSelectionRange> list_range = t->selectedRanges();
	for (int ii=0; ii<list_range.size(); ii++)
	{
		PROGRESS_PERCENT(ii*100/list_range.size());

		qDebug("flag2");

		int row0 = list_range.at(ii).topRow();
		int row1 = list_range.at(ii).bottomRow();
		qDebug("%d,%d",row0,row1);


		for (int i=row0; i<=row1; i++)
		{
			qDebug("flag3.1");
			QTableWidgetItem * curItem = t->item(i,0);
			qDebug("flag3.2");
			qDebug("state in batch set is");
			qDebug(state ? "true" : "false");
			curItem->setCheckState(BOOL_TO_CHECKED( state ));
			qDebug("flag3.3");
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

	if (1 + (tabOptions->currentIndex()) == stNeuronSegment)
	{
		isBatchOperation = false;
		Renderer_gl1* r = renderer;
		if (! r)  return;	
		V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
		My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(r->_idep);
		V_NeuronSWC_list* tracedNeuron = &(curImg->tracedNeuron);
		curImg->update_3drenderer_neuron_view(w, r);
	}

	updatedContent(t);
}

void V3dr_surfaceDialog::selectedColor(int map)
{
	QTableWidget* t = currentTableWidget();
	if (! t) return;

	QColor qcolor0(255,255,255,255);
	if (map==0)
	{
		//qcolor0 = QColorDialog::getColor(QColor());
		//if (! qcolor0.isValid()) return;           // this is no use for clicking Cancel by user, Qt's bug !!!
		if (! v3dr_getColorDialog( &qcolor0))  return; //090424 RZC
	}

	PROGRESS_DIALOG("Updating color    ", this);
	begin_batch();

	int n_row = t->rowCount();
	for (int i=0; i<n_row; i++)
	{
		PROGRESS_PERCENT(i*100/n_row);

		QTableWidgetItem * curItem = t->item(i,1);
		if (! curItem->isSelected()) continue; // skip un-selected

		QColor qcolor = qcolor0;

		if (map==-1)      //random color
		{
			qcolor = QCOLOR(random_rgba8());
		}
		else if (map==-2)  //hanchuan' color table
		{
			int j = i%hanchuan_colortable_size();
			qcolor = QColor(hanchuan_colortable[j][0],hanchuan_colortable[j][1],hanchuan_colortable[j][2]);
		}

		//180412 RZC
		else if (map==2)  //multi-neuron colorful
		{
			qcolor = QColor(0,0,0,2);
		}
		else if (map==1)  //neuron segment colorful
		{
			qcolor = QColor(0,0,0,1);
		}

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
	Renderer_gl1* r = renderer;
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
	Renderer_gl1* r = renderer;
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
	Renderer_gl1* r = renderer;
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
	Renderer_gl1* r = renderer;
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
        //r->listMarker[i].color = RGBA8V(curItem->data(0));
        listMarker[i].color = RGBA8V(curItem->data(0)); // sync
        r->updateMarkerList(listMarker, i);

		UPATE_ITEM_ICON(curItem);
		break;
	}

	updatedContent(t);
}

////////////////////////////////////////////////////////////////////////

QTableWidget* V3dr_surfaceDialog::createTableSWC()
{
    //qDebug("##SWC Table is recreated!!!");
	
	Renderer_gl1* r = renderer;
	if (! r)  return 0;

	QStringList qsl;
    qsl << "on/off"  << "color" << "count" << "display mode" << "editing" << "name" << "comment"<< "file name";
	int row = (r->listNeuronTree.size());
	int col = qsl.size();

	QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	qDebug("  create begin t->rowCount = %d", t->rowCount());
	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;

		ADD_ONOFF(r->listNeuronTree[i].on);
		ADD_QCOLOR(r->listNeuronTree[i].color);

		ADD_STRING( tr("%1").arg(r->listNeuronTree[i].listNeuron.size()) );

        switch (r->listNeuronTree[i].linemode)
        {
        case 1: ADD_STRING( tr("line") ); break;
        case 0: ADD_STRING( tr("tube") ); break;
        default: ADD_STRING( tr("global") ); break;
        }

        if (r->listNeuronTree[i].editable) {
			ADD_STRING( tr("Yes") );
		} else
			ADD_STRING( tr("") );

		ADD_STRING( r->listNeuronTree[i].name ); //by PHC, add a column of name, which is different from file name. 090219

		ADD_STRING( r->listNeuronTree[i].comment );

		ADD_STRING( r->listNeuronTree[i].file );

		MESSAGE_ASSERT(j==col);
	}
  //  qDebug("  end   t->rowCount = %d", t->rowCount());

    t->resizeColumnsToContents();
	return t;
}

void V3dr_surfaceDialog::pickSWC(int i, int j)
{
	qDebug("	pickSWC( %d, %d )", i,j);

	Renderer_gl1* r = renderer;
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

QTableWidget* V3dr_surfaceDialog::createTableNeuronSegment()
{
    //qDebug("##NeuronSegment Table is recreated!!!");

	Renderer_gl1* r = renderer;
	if (! r)  return 0;

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(r->_idep);
    V_NeuronSWC_list* tracedNeuron = &(curImg->tracedNeuron);

	QStringList qsl;
    //qsl <<"on/off" << "color" << "count" << "type" << "name" << "comment" <<"in file"<<"index";
    qsl <<"on/off" << "count" << "type" << "index";
    int row;
    bool flag = false;
    for (int i=0; i<r->listNeuronTree.size();i++)
        if (r->listNeuronTree[i].editable) flag = true;
    if ((r->listNeuronTree.size() !=0 && !flag) || ! curImg)
		row = 0;
    else row =tracedNeuron->nsegs();

	int col = qsl.size();

    QTableWidget* t = new QTableWidget(row,col, this);
	t->setHorizontalHeaderLabels(qsl);

	for (int i=0; i<row; i++)
	{
		int j=0;
		QTableWidgetItem *curItem;
        V_NeuronSWC curSeg = tracedNeuron->seg[i];

		ADD_ONOFF(curSeg.on);

//		RGBA8 color;
//		color.r = curSeg.color_uc[0];
//		color.g = curSeg.color_uc[1];
//      color.b = curSeg.color_uc[2];
//		color.a = curSeg.color_uc[3];
//		ADD_QCOLOR(color);

        //ADD_STRING( tr("%1").arg(curSeg.row.size()) );
        CustomTableWidgetItem *curCustomItem = new CustomTableWidgetItem(tr("%1").arg(curSeg.row.size()));
        t->setItem(i, j++, curCustomItem);

        //ADD_STRING( tr("%1").arg(curSeg.row[1].type) ); //
        curCustomItem = new CustomTableWidgetItem(tr("%1").arg(curSeg.row[0].type));
        curCustomItem->setFlags(curCustomItem->flags() | Qt::ItemIsEditable);
        t->setItem(i, j++, curCustomItem);
		
//		ADD_STRING( QString::fromUtf8(curSeg.name.c_str()));

//		ADD_STRING( QString::fromUtf8(curSeg.comment.c_str()) );

//		ADD_STRING( QString::fromUtf8(curSeg.file.c_str()) );

        curCustomItem = new CustomTableWidgetItem(tr("%1").arg(i));
        t->setItem(i, j++, curCustomItem);

		MESSAGE_ASSERT(j==col);
    }

    //
    t->setSortingEnabled(true);

    //
    t->resizeColumnsToContents();

    //
    if(sortNeuronSegment)
    {
        t->sortItems(sortNeuronSegment);
    }

    //
    connect(t, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(sortNeuronSegmentByType(QTableWidgetItem*)));

	return t;
}


void V3dr_surfaceDialog::pickNeuronSegment(int i, int j)
{
//	qDebug("flag4");
	qDebug("	pickNeuronSegment( %d, %d )", i,j);
	if (j == 1) return;
	
	Renderer_gl1* r = renderer;
	if (! r)  return;

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(r->_idep);
	V_NeuronSWC_list* tracedNeuron = &(curImg->tracedNeuron);

	if (i<0 || i>=tracedNeuron->nsegs())  return;

	QTableWidget* t = table[stNeuronSegment];
	QTableWidgetItem *curItem = t->item(i,j);

    int index = t->item(i,3)->text().toInt();

	switch(j)
	{
	case 0:
		qDebug(tracedNeuron->seg[i].on ? "on" : "off");
		qDebug(CHECKED_TO_BOOL(curItem->checkState()) ? "true" : "false");
        tracedNeuron->seg[index].on = CHECKED_TO_BOOL(curItem->checkState()); // !tracedNeuron->seg[i].on;
		qDebug(tracedNeuron->seg[i].on ? "on" : "off");
		break;
	case 1:
        r->listCell[index].color = RGBA8V(curItem->data(0));
		UPATE_ITEM_ICON(curItem);
		break;
	}

	updatedContent(t);
	if (!isBatchOperation) 
	curImg->update_3drenderer_neuron_view(w, r);
}

///////////////////////////////////////////////////////////////////////////////

QTableWidget* V3dr_surfaceDialog::createTableAPO()
{
	Renderer_gl1* r = renderer;
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
	qDebug("	pickAPO( %d, %d )", i,j);
	
	Renderer_gl1* r = renderer;
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
	Renderer_gl1* r = renderer;
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

	Renderer_gl1* r = renderer;
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
	if (t==table[stNeuronSegment]) 		curItem = t->item(i, 7); \
	if (t==table[stNeuronStructure]) 	curItem = t->item(i, 4); \
	if (t==table[stPointCloud]) 		curItem = t->item(i, 7); \
	if (t==table[stPointSet]) 			curItem = t->item(i, 3); \
}

#define COMMENT_ITEM_OF_TABLE(curItem, t,i) \
{ \
	if (t==table[stImageMarker])		curItem = t->item(i, 7); \
	if (t==table[stLabelSurface]) 		curItem = t->item(i, 4); \
	if (t==table[stNeuronSegment]) 		curItem = t->item(i, 8); \
	if (t==table[stNeuronStructure]) 	curItem = t->item(i, 5); \
	if (t==table[stPointCloud]) 		curItem = t->item(i, 8); \
	if (t==table[stPointSet]) 			curItem = t->item(i, 4); \
}

void V3dr_surfaceDialog::editObjNameAndComments() //090219 unfinished yet. need to think a good logic here
{
#ifndef test_main_cpp
	QTableWidget* t = currentTableWidget();
	if (! t) return;
	Renderer_gl1* r = renderer;
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

                    qDebug()<<"sync with renderer's name and comment";
				}
				else if (t==table[stLabelSurface])
				{
					r->listLabelSurf[i].name = realobj_name;
					r->listLabelSurf[i].comment = realobj_comment;
				}
				else if (t==table[stNeuronSegment])
				{
//					r->listCell[i].name = realobj_name;
//					r->listCell[i].comment = realobj_comment;
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

void V3dr_surfaceDialog::editNeuronSegmentType()
{
    //
    QTableWidget* t = currentTableWidget();
    if (! t) return;

    Renderer_gl1* r = renderer;
    if (! r)  return;

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = 0;
    if (w) curImg = v3dr_getImage4d(r->_idep);
    V_NeuronSWC_list* tracedNeuron = &(curImg->tracedNeuron);

    int row, col;
    col = 4; // on/off count type index
    bool flag = false;
    for (int i=0; i<r->listNeuronTree.size();i++)
        if (r->listNeuronTree[i].editable) flag = true;
    if ((r->listNeuronTree.size() !=0 && !flag) || ! curImg)
        row = 0;
    else row =tracedNeuron->nsegs();

    if(row<1)
    {
        cout<<"No segments"<<endl;
        return;
    }

    cout<<"rows "<<row<<endl;

    //
    int currentTypeValue = 1;
    if(t==table[stNeuronSegment])
    {
        QList<QTableWidgetItem *> selected = t->selectedItems();

        if(selected.size()<col) // at least one row is selected
        {
            cout<<"Invalid selection"<<endl;
            return;
        }

        // cout<<"selected qtablewidgetitems: "<<selected.size()<<endl;

        int selectedrows = selected.size() / col;

        currentTypeValue = selected.at(2*selectedrows)->text().toInt();

        bool ok;
        int typeValue = QInputDialog::getInt(this, tr("Set Neuron Segment Type"), tr("Type:"), currentTypeValue, -2147483647, 2147483647, 1, &ok);

        if(ok)
        {
            cout<<"new type value: "<<typeValue<<endl;

            //QList <NeuronTree> *neurontrees = r->getHandleNeuronTrees();

            for(int i=0; i<selectedrows; i++)
            {
                // it's up to the definition of the neuron segment table

                int k = 2*selectedrows + i;
                int index = selected.at(3*selectedrows + i)->text().toInt();

                cout<<"i "<<i<<" "<<selected.at(k)->text().toStdString()<<" "<<index<<endl;

                V_NeuronSWC curSeg = tracedNeuron->seg[index];

                cout<<"how many nodes' type need to be changed: "<<curSeg.nrows()<<endl;

                for(int j=0; j<curSeg.nrows(); j++)
                {
                    tracedNeuron->seg[index].row[j].type = typeValue;
                    // cout<<"changed? "<<tracedNeuron->seg[index].row[j].type<<endl;
                }

//                if(i%8==3)
//                {
//                    int index = selected.at(i+4)->text().toInt();

//                    cout<<"i "<<i<<" "<<selected.at(i)->text().toStdString()<<" "<<index<<endl;

//                    V_NeuronSWC curSeg = tracedNeuron->seg[index];

//                    cout<<"how many nodes' type need to be changed: "<<curSeg.nrows()<<endl;

//                    for(int j=0; j<curSeg.nrows(); j++)
//                    {
//                        tracedNeuron->seg[index].row[j].type = typeValue;
//                        // cout<<"changed? "<<tracedNeuron->seg[index].row[j].type<<endl;
//                    }
//                }
            }
        }
    }
    else
    {
        cout<<"Not defined other than neuron segment table"<<endl;
    }

    //
    updatedContent(t);

    if (!isBatchOperation)
    curImg->update_3drenderer_neuron_view(w, r);
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

	Renderer_gl1* r = renderer;
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
void V3dr_surfaceDialog::zoomMarkerLocation()
{
    qDebug()<<"zoom in to this select marker location";
    if (glwidget)
    {
        My4DImage* curImg = 0;         curImg = v3dr_getImage4d(renderer->_idep);

        if (curImg) curImg->cur_hit_landmark = last_marker;
        LocationSimple makerPo=curImg->listLandmarks.at(last_marker);
        vector <XYZ> loc_vec;
        XYZ loc;
        loc.x=makerPo.x;loc.y=makerPo.y;loc.z=makerPo.z;
        loc_vec.push_back(loc);
        v3d_msg("Invoke terafly local-zoomin based on an existing marker.", 0);
        renderer->b_grabhighrez=true;
        renderer->produceZoomViewOf3DRoi(loc_vec,0);
    }
}

void V3dr_surfaceDialog::updateMarkerList(QList <ImageMarker> markers)
{
    for(int i=0; i<markers.size(); i++)
    {
        listMarker[i] = markers[i];
    }
}

void V3dr_surfaceDialog::sortNeuronSegmentByType(QTableWidgetItem* item)
{
    if(item->column()==1 || item->column()==2 || item->column()==3) // sort count,type,index
    {
        sortNeuronSegment = item->column();
    }

    updatedContent(item->tableWidget());

    Renderer_gl1* r = renderer;
    if (! r)  return;

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = 0;
    if (w) curImg = v3dr_getImage4d(r->_idep);

    if (!isBatchOperation)
    curImg->update_3drenderer_neuron_view(w, r);
}

