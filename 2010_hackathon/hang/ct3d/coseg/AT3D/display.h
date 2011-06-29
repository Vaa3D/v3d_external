#ifndef WINDOW_H_H
#define WINDOW_H_H

#include <QtGui>
#include <vector>
#include <stack>
#include "glwidget.h"
#include "AT3D.h"
#include "../component_tree.h"
#include "ui_display.h"
#include "./ui/filterDlg.h"
using namespace std;


//==================================================
class AT3DVIEW : public QWidget
//==================================================
{
	
	Q_OBJECT
	
public:
	AT3DVIEW(QWidget *parent = 0);

private slots:
	/************************
	 * File Group
	 ************************/
        void on_adjustRegionButton_clicked();
 void onOpen();
	
	void onLoadTrees();
	
	void onSaveFrames();
	
	/************************
	* Edit Group
	************************/	
	void onApplyFilter();
	
	void onFilter();
	
	void onSetColor();
	
	/************************
	 * Statistics Group
	 ************************/
	void onSummary();
	
	void onSpeed();
	
	void onVolume();
	
	void onDeformation();
	
	/**********************
	 * View Group
	 ***********************/
	void onNew3D();
	
	void onResetView();
		
	void onViewTree();
	
	/**********************
	 * Control Group
	 ***********************/
	void onFirst();
	
	void onLast();
	
	void onPrevious();
	
	void onNext();
	
	/***********************
	 * Cell Widget
	 ***********************/
	void onReverse();
	
	void onUndo();
	
	void onChoose();
	
	void onDelete();
	
	void onCheckBoxChanged();
	
	/************************
	 * GLWidget
	 ************************/
	void onNearestCellChoosed(int);
	 
private:
	
	void init();
	
	void clear();
	
	bool isLoaded();

	void setCellWidget();

private:
	
	Ui::window ui;

	AT3D m_at3d;
	
	vector<AT3D::Cell*> m_cells;     // vector<AT3D::Cell*> is also Frame, we can get m_cells easily
	
	vector<QCheckBox*> m_checkers;
	
	//int m_numCells;             //changes too often
		
	int m_numFrames; 
	
	int m_currentTime;
	
	GLWidget* m_glWidget;
	
};

#endif
