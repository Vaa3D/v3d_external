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




//a tabled viewer for 3d image blending and atlas viewing
//by Hanchuan Peng
//081123

#ifndef __ATLAS_VIEWER_H__
#define __ATLAS_VIEWER_H__

#include <QtGui>

#include "../basic_c_fun/color_xyz.h"

#include "v3d_core.h"
#include "ChannelTable.h"
#include <QProgressDialog>
class XFormWidget;
class My4DImage;

#define PROGRESS_DIALOG(text, widget)  	QProgressDialog progress( QString(text), 0, 0, 100, (QWidget*)widget, Qt::Popup|Qt::Dialog);
extern QProgressDialog progress;
#define PROGRESS_PARENT(widget)   progress.setParent( (QWidget*)widget ); //Qt::WShowModal
#define PROGRESS_TEXT(text)   { QApplication::setActiveWindow(&progress);  progress.setLabelText( QString(text) );  progress.repaint();}
#define PROGRESS_PERCENT(i)	  { QApplication::setActiveWindow(&progress);  progress.setValue(i);  progress.repaint(); \
QCoreApplication::processEvents( /*QEventLoop::ExcludeUserInputEvents*/ );}


class V3D_atlas_viewerDialog: public QDialog
{
	Q_OBJECT;

public:
	V3D_atlas_viewerDialog(XFormWidget* w);
	virtual ~V3D_atlas_viewerDialog();
	void IncRef(XFormWidget* w);
	void DecRef(XFormWidget* w);
	void setCurTab(int i) {if(tabOptions) tabOptions->setCurrentIndex(i);} // 090504 RZC
	ChannelTabWidget* getChannelTabWidget();

protected:
	bool bCanUndo, bMod;
	XFormWidget* triview_widget;
	My4DImage * imgdata;
	int ref;
	QPoint oldpos;
	virtual void closeEvent(QCloseEvent* e);
	virtual void showEvent(QShowEvent* e);
	virtual void moveEvent(QMoveEvent* e);

	bool updateTableItem_Landmark(QTableWidget *t, int row, LocationSimple *p_landmark, bool b_createNewItems);

public slots:
	virtual void accept() { done(1); } // this only hide dialog
	virtual void reject() { done(0); } // this only hide dialog
	virtual void done(int);          // this really close/delete dialog

	void undo();
	void reCreateTables(XFormWidget* w); //point to new view

	void selectAll();
	void deselectAll();
	void inverseSelect();
	void onSelected();
	void offSelected();
	void colorSelected();
	void colorChannelSelected(int c);
	void maskImgStateChanged();
	void tabChanged(int t);

	void findNext();
	void findPrev();

	void doMenuOfColor();
	void selectedColor(int map=0);
	void mapHanchuanColor() {selectedColor(1);}
	void mapRandomColor()   {selectedColor(-1);}

	void pickAtlasRow(int row, int col);
	void pickLandmark(int row, int col);
	void highlightLandmark(int row, int col, int previous_row, int previous_col);
	void pickColorChannel(int row, int col);

	void seeLandmarkProperty();
	void moveLandmarkUp();
	void moveLandmarkDown();
	void deleteSelectedLandmark();
	void resetAllLandmarkNames();
	void resetAllLandmarkComments();

protected:
	void setItemEditor();
	void create();
	void createTables();
	QTableWidget* createTableAtlasRows();
	QTableWidget* createTableLandmarks();
	QTableWidget* createColorChannelManager();
	ChannelTabWidget* createChannelTab();
	void deleteChannelTab(ChannelTabWidget*);

	QVector<bool> in_batch_stack;
	void begin_batch() {in_batch_stack.push_back(true);}
	void end_batch()   {in_batch_stack.pop_back();}
	void updatedContent(QTableWidget* t);
	QTableWidget* currentTableWidget(QWidget** pp_page=0);

	void createMenuOfColor();
	QMenu menuColor;


	QPushButton *okButton, *cancelButton, *undoButton,
	*selectAllButton, *deselectAllButton, *inverseSelectButton,
	*onSelectButton, *offSelectButton, *colorSelectButton;
	QTableWidget *table[1+3]; //100824. PHC
	QTabWidget *tabOptions;

	//for the atlas viewer only
	QSpinBox *channelSpinBox;
	QLabel *channelSpinBoxLabel;
	QCheckBox *bMaskBlendedImgsBox;

	//for landmark manager only
	QPushButton *seePropertyButton, *moveUpButton, *moveDownButton, *deleteButton, *resetAllLandmarkNamesButton, *resetAllLandmarkCommentsButton;

	// search group
	QLabel *searchTextEditLabel, *searchTextResultLabel;
	QLineEdit *searchTextEdit;
	QPushButton *doSearchTextNext, *doSearchTextPrev;

	// save state for cancel
	QList <InvidualAtlasFileInfo> listAtlasFiles;
	QList <LocationSimple>  listLandmarks;
};

#endif
