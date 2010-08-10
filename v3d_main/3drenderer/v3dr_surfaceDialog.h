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
 * V3dr_surfaceDialog.h
 *
 *  Created on: Nov 10, 2008
 *      Author: ruanzongcai
 * last edit: Hanchuan Peng, 090219: add the name/comment editor
 * 090521: add the APOSet tab for point cloud atlas
 * 091027: add doSearchTextHighlightAllHits button
 */

#ifndef V3DR_SURFACEDIALOG_H_
#define V3DR_SURFACEDIALOG_H_

#include "v3dr_common.h"
#include "renderer_tex2.h"
#include "v3dr_glwidget.h"

#include "qtr_widget.h"
#include "ItemEditor.h"


class V3dr_surfaceDialog: public SharedDialog
{
    Q_OBJECT;

public:
	V3dr_surfaceDialog(V3dR_GLWidget* w, QWidget* parent=0);
	virtual ~V3dr_surfaceDialog();
	void setCurTab(int i) {if(tabOptions) tabOptions->setCurrentIndex(i);} // 090504 RZC
	int getCurTab() {if(tabOptions) return tabOptions->currentIndex(); else return -1;} // 090622 RZC

protected:
	V3dR_GLWidget *glwidget, *active_widget;
	Renderer_tex2* renderer;
	int iLastTab;
	bool bCanUndo, bMod;
	bool bAttached;
	QString title;
	int last_marker;

	void firstCreate();

public slots:
	virtual void linkTo(QWidget* w); //link to new view
	void onAttached(bool);

	void undo();

	void selectAll();
	void deselectAll();
	void selectInverse();

	void selectedOnOff(bool state);
	void selectedOn()  {selectedOnOff(true);}
	void selectedOff() {selectedOnOff(false);}

	void doMenuOfColor();
	void selectedColor(int map=0);
	void mapHanchuanColor() {selectedColor(1);}
	void mapRandomColor()   {selectedColor(-1);}

	void pressedClickHandler(int row, int col);
	void doubleClickHandler(int row, int col);
	void pickSurf(int row, int col);
	void pickSWC(int row, int col);
	void pickAPO(int row, int col);
	void pickAPO_Set(int row, int col);
	void pickMarker(int row, int col);

	void editObjNameAndComments();

	void findNext();
	void findPrev();
	void findAllHighlight();

	void onMarkerLocalView();

protected:
	void setItemEditor();

	void clearTables_fromTab();
	void createTables_addtoTab();

	QTableWidget* createTableSurf();
	QTableWidget* createTableSWC();
	QTableWidget* createTableAPO();
	QTableWidget* createTableMarker();
	QTableWidget* createTableAPO_Set();

	QTableWidget* currentTableWidget();

	QVector<bool> in_batch_stack;
	void begin_batch() {in_batch_stack.push_back(true);}
	void end_batch()   {in_batch_stack.pop_back();}
	void updatedContent(QTableWidget* t);

	void createMenuOfColor();
	QMenu menuColor;

	QPushButton *okButton, *cancelButton, *undoButton,
				*selectAllButton, *deselectAllButton, *inverseSelectButton,
				*onSelectButton, *offSelectButton, *colorSelectButton,
				*editNameCommentButton, *markerLocalView;
	QTabWidget *tabOptions;
	QTableWidget *table[1+5];
	QCheckBox *checkBox_accumulateLastHighlightHits, *checkBox_attachedToCurrentView;

	// search group
	QLabel *searchTextEditLabel, *searchTextResultLabel;
	QLineEdit *searchTextEdit;
	QPushButton *doSearchTextNext, *doSearchTextPrev, *doSearchTextHighlightAllHits;

	// save state for cancel/undo
	QList <LabelSurf> listLabelSurf;
	QList <NeuronTree> listNeuronTree;
	QList <CellAPO> listCell;
	QList <ImageMarker> listMarker;

	void init_members()
	{
		glwidget = active_widget = 0;
		renderer = 0;
		iLastTab = -1;
		bCanUndo = bMod = false;
		bAttached = false;
		title = tr("Object Manager");  //Object Pick/Color Options")); //090423 RZC: changed
		last_marker = -1;

		okButton=cancelButton=undoButton=0;
		selectAllButton=deselectAllButton=inverseSelectButton=
			onSelectButton=offSelectButton=colorSelectButton=editNameCommentButton=markerLocalView =0;
		for (int i=0; i<=5; i++)  table[i]=0; //by PHC, 090521 change to 5
		tabOptions=0;
		checkBox_accumulateLastHighlightHits = checkBox_attachedToCurrentView =0;//100809 RZC
		searchTextEditLabel=searchTextResultLabel = 0;
		searchTextEdit =0;
		doSearchTextNext=doSearchTextPrev=doSearchTextHighlightAllHits =0;
	}
};


#endif /* V3DR_SURFACEDIALOG_H_ */
