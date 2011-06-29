#include <QtGui>
#include "display.h"
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <cassert>
#include "../myalgorithms.h"
#include "../component_tree.h"
#include "ui/adjustregiondialog.h"
using namespace std;

/************************************
 * variables
 ************************************/

/***********************************
 * functions declaration
 ***********************************/
QString getColorValue(AT3D::rgb_pixel c);
QString hex2str(int v);
QTreeWidgetItem* getTreeItem(ComponentTree & tree, QTreeWidget* widget);
/***********************************
 * Realize Member Functions According
 * to Their Declaration
 ***********************************/
AT3DVIEW::AT3DVIEW(QWidget *parent) : QWidget(parent)
{
	//step 1 : setUi
	ui.setupUi(this);
	//ui.cellWidget->setAttribute(Qt::WA_DeleteOnClose, false);
	
	//step 2 : initialization
	
	m_glWidget = new GLWidget();
	connect(m_glWidget, SIGNAL(cellChoosed(int)), this, SLOT(onNearestCellChoosed(int)));
	m_glWidget = new GLWidget();
	connect(m_glWidget, SIGNAL(cellChoosed(int)), this, SLOT(onNearestCellChoosed(int)));
	
	QLayout * layout;
	// when reopen, here is very important
	if(ui.glGroupBox->layout()== NULL)
	{
		layout= new QGridLayout();
		layout->addWidget(m_glWidget);
		ui.glGroupBox->setLayout(layout);
	}
	else
	{
		layout=ui.glGroupBox->layout();
		layout->addWidget(m_glWidget);
	}
	
	init();
	
}

/***************************************************
 * onOpen : open data
 ***************************************************/

void AT3DVIEW::onOpen()
{
	// 1. open
	QStringList fileList = QFileDialog::getOpenFileNames(
											 this,
											 "Choose files of time 1",
											 "",
											 "Images (*.png *.tif *.tiff *.jpg)");
	if(fileList.size()==0)return;
	
	if(isLoaded())clear();
	
	QStringList::iterator it;
	vector<string> names;
	for(it=fileList.begin();it!=fileList.end();it++)
	{
		QString tmp=*it;
		names.push_back(tmp.toStdString());
		cout<<tmp.toStdString().c_str()<<endl;
	}
	// 2. create AT3D
	m_numFrames = names.size();
	m_at3d.create(names);
	
	//3. set open button's text
	ui.openButton->setText(QObject::tr("reopen"));
	
	//4. set glwidget's view
	
	m_glWidget->setSize(m_at3d.width(),m_at3d.height(),m_at3d.depth());
	m_glWidget->setFrame(m_at3d.frame(m_currentTime));
	m_glWidget->setWindowTitle(QObject::tr("Frame %1").arg(m_currentTime+1));
	m_glWidget->updateGL();	
	
	/**************************************************
	 * step 7: refresh the cells widget to show all the cells
	 **************************************************/
	m_cells = m_at3d.frame(0);
	setCellWidget();
}

/************************************************
 * SLot Functions Begin
 ************************************************/

//File Group
void AT3DVIEW::onLoadTrees()
{
	if(!isLoaded())return;
	QString treeFile = QFileDialog::getOpenFileName(
														 this,
														 "Choose files of time 1",
														 "",
														 "Tree (*.tr *.tree)");
	if(treeFile.length() == 0) return;
	if(!m_at3d.setTrees(treeFile.toStdString())) QMessageBox::warning(0,"", "load error");
	else QMessageBox::information(0,"","load successfully");
}

void AT3DVIEW::onSaveFrames()
{
	if(!isLoaded())return;
	QString dir = QFileDialog::getExistingDirectory(0, "Open Directory",
													"",
													QFileDialog::ShowDirsOnly
													| QFileDialog::DontResolveSymlinks);
	if(dir=="")return;
	
	m_at3d.saveFrames(dir.toStdString());
	
	QMessageBox::information(this,"","Frames Saved Successfully!");

}

//Edit Group
void AT3DVIEW::onApplyFilter()
{
	
	if(!isLoaded())return;

	m_at3d.push_state();        // save state by yourself
	
	FilterDlg* dlg = new FilterDlg(this);
	dlg->setRange(      m_cells[0]->size() + 1,
				  (int)(m_at3d.maxMeanVolume() + 1), 
				        m_at3d.frameCount(),
				  (int)(m_at3d.maxMeanSpeed() + 1),
				  (int)(m_at3d.maxMeanDeform() + 1));
	
	connect(dlg, SIGNAL(valueChanged()), this, SLOT(onFilter()));
	dlg->setModal(true);
	dlg->exec();
	
	m_at3d.filtering(m_currentTime, dlg->getData());
	m_cells = m_at3d.frame(m_currentTime);
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
	
}

void AT3DVIEW::onFilter()
{
	FilterDlg* dlg = (FilterDlg*) sender();
	m_at3d.filtering(m_currentTime, dlg->getData());
	
	m_cells = m_at3d.frame(m_currentTime);
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();          //try to avoid delay
}

void AT3DVIEW::onSetColor()
{
	if(!isLoaded())return;
	
	AT3D::Cell* cell = NULL;
	for(int i = 0; i < (int)m_cells.size(); i++)
	{
		QCheckBox* checker = m_checkers[i];
		if(checker->checkState() == Qt::Checked)
		{
			if(cell == NULL) cell = m_cells[i];
			else
			{
				QMessageBox::warning(0,"","You have choose more than one cell\nPlease choose only one cell!");
				return;
			}
		}
	}
	if(cell == NULL) QMessageBox::warning(0,"","Please choose one cell!\n");
	
	QColor color = QColorDialog::getColor();
	if(color.isValid())
	{
		//set current frame's color to selected color
		AT3D::rgb_pixel rgb_color = AT3D::rgb_pixel(color.red(),color.green(),color.blue());

		AT3D::Track* track = cell->track;
		vector<AT3D::Cell>::iterator it = track->cells.begin();
		while(it != track->cells.end())
		{
			(*it).color = rgb_color;
			it++;
		}
	}
	
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
}

//Statistics Group
// summary: trackId, color, meanVolume, mean speed, mean deform1 and mean deform2 , lifespan, 
void AT3DVIEW::onSummary()
{
	if(!isLoaded())return;
	int rows = m_at3d.trackCount();
	int columns = 6;
	QTableWidget* tableWidget = new QTableWidget(rows, columns, NULL);
	QTableWidgetItem *item;
	int row = 0;
	int column = 0;
	QStringList strList;
	QString str;
	strList<<"trackId";
	strList<<"color";
	strList<<"mean volume";
	strList<<"mean speed";
	strList<<"mean deform";
	strList<<"lifeSpan";
	tableWidget->setHorizontalHeaderLabels(strList);

	//double mean_volume = 0;
	
	for(row = 0 ; row < rows; row++)
	{			
		column = 0;
		AT3D::Track& track = m_at3d.track(row);

		item = new QTableWidgetItem(QObject::tr("%1").arg(row+1,(int)log10(m_at3d.trackCount())+1,10,QChar('0')));
		tableWidget->setItem(row,column++,item);
		AT3D::rgb_pixel color = track.cells[0].color;
		item = new QTableWidgetItem("");
		item->setBackgroundColor(QColor(color.r,color.g,color.b));
		tableWidget->setItem(row,column++,item);
		item = new QTableWidgetItem(QObject::tr("%1").arg(track.meanVolume,(int)log10(m_at3d.maxMeanVolume())+4,'f',2,QChar('0')));
		tableWidget->setItem(row,column++,item);
		item = new QTableWidgetItem(QObject::tr("%1").arg(track.meanSpeed,(int)log10(m_at3d.maxMeanSpeed())+4,'f',2,QChar('0')));
		tableWidget->setItem(row,column++,item);
		item = new QTableWidgetItem(QObject::tr("%1").arg(track.meanDeform,(int)log10(m_at3d.maxMeanDeform())+4,'f',2,QChar('0')));
		tableWidget->setItem(row,column++,item);
		item = new QTableWidgetItem(QObject::tr("%1").arg(track.size(),(int)log10(m_at3d.frameCount())+1,10,QChar('0')));
		tableWidget->setItem(row,column++,item);
	}
	tableWidget->show();
	tableWidget->setWindowTitle("Summary");
	
	tableWidget->setSortingEnabled(true);
	tableWidget->sortByColumn(0,Qt::AscendingOrder);

	
}

void AT3DVIEW::onSpeed()
{
	if(!isLoaded())return;
	
	int rows = m_at3d.trackCount();
	int columns = m_numFrames +  2 ;
	QTableWidget* tableWidget = new QTableWidget(rows, columns, NULL);
	QTableWidgetItem *item;
	int row = 0;
	int column = 0;
	QStringList strList;
	QString str;
	str = QObject::tr("trackId");
	strList<<str;
	str = QObject::tr("%1").arg(column+1).arg(column+1);
	strList<<str;
	for(column = 1; column < m_numFrames; column++)
	{
		str = QObject::tr("%1->%2").arg(column).arg(column+1);
		strList<<str;
	}
	strList<<"meanSpeed";
	tableWidget->setHorizontalHeaderLabels(strList);
	
	
	for(row = 0 ; row < rows; row++)
	{
		column = 0;
		item = new QTableWidgetItem(QObject::tr("%1").arg(row+1,(int)log10(m_at3d.trackCount())+1,10,QChar('0')));
		tableWidget->setItem(row,column,item);
		AT3D::Track& track = m_at3d.track(row);
		double sum_distance = 0.0;
		while(column < m_numFrames)
		{
			
			if(column >= track.start && column < track.start+track.size())
			{
				if(column == track.start)
				{
					item = new QTableWidgetItem(QObject::tr("0.00"));
					tableWidget->setItem(row,column+1,item);
				}
				else
				{
					double distX = track[column].centerX - track[column-1].centerX;
					double distY = track[column].centerY - track[column-1].centerY;
					double distZ = track[column].centerZ - track[column-1].centerZ;
					double distance = sqrt(distX*distX + distY*distY + distZ*distZ);
					sum_distance += distance;
					item = new QTableWidgetItem(QObject::tr("%1").arg(distance,4,'f',2,QChar('0')));
					tableWidget->setItem(row,column+1,item);
				}
			}
			column++;
		}
		column++;
		assert(track.size() >= 1);
		item = new QTableWidgetItem(QObject::tr("%1").arg(sum_distance/(track.size()-1),4,'f',2,QChar('0')));
		tableWidget->setItem(row,column++,item);
	}
	tableWidget->show();
	tableWidget->setWindowTitle("Speed");
	tableWidget->setSortingEnabled(true);
	tableWidget->sortByColumn(0,Qt::AscendingOrder);

}

void AT3DVIEW::onVolume()
{
	if(!isLoaded())return;
	
	int rows = m_at3d.trackCount();
	int columns = m_numFrames +  2 ;
	QTableWidget* tableWidget = new QTableWidget(rows, columns, NULL);
	
	//tableWidget->setSortingEnabled(true);
	
	QTableWidgetItem *item;
	int row = 0;
	int column = 0;
	QStringList strList;
	QString str;
	str = QObject::tr("trackId");
	strList<<str;
	for(column = 0; column < m_numFrames; column++)
	{
		str = QObject::tr("volume %1").arg(column+1);
		strList<<str;
	}
	strList<<"meanVolume";
	tableWidget->setHorizontalHeaderLabels(strList);
	
	
	for(row = 0 ; row < rows; row++)
	{
		column = 0;
		item = new QTableWidgetItem(QObject::tr("%1").arg(row+1,(int)log10(m_at3d.trackCount())+1,10,QChar('0')));
		tableWidget->setItem(row,column,item);
		AT3D::Track& track = m_at3d.track(row);
		while(column < m_numFrames)
		{
			if(column >= track.start && column < track.start+track.size())
			{
				item = new QTableWidgetItem(QObject::tr("%1").arg(track[column].size(),(int)log10(m_at3d.track(0).meanVolume) + 1, 10, QChar('0')));
				tableWidget->setItem(row,column+1,item);
			}
			column++;
		}
		column++;
		item = new QTableWidgetItem(QObject::tr("%1").arg(track.meanVolume,(int)log10(m_at3d.track(0).meanVolume)+4,'f',2,QChar('0')));
		tableWidget->setItem(row,column++,item);
	}
	tableWidget->show();
	tableWidget->setWindowTitle("Volume");
	tableWidget->setSortingEnabled(true);
	tableWidget->sortByColumn(0,Qt::AscendingOrder);
	
}

void AT3DVIEW::onDeformation()
{
	if(!isLoaded())return;

	int rows = m_at3d.trackCount();
	int columns = m_numFrames +  3 ;
	QTableWidget* tableWidget = new QTableWidget(rows, columns, NULL);
	QTableWidgetItem *item;
	int row = 0;
	int column = 0;
	QStringList strList;
	QString str;
	str = QObject::tr("trackId");
	strList<<str;
	str = QObject::tr("%1").arg(column+1).arg(column+1);
	strList<<str;
	for(column = 1; column < m_numFrames; column++)
	{
		str = QObject::tr("%1->%2").arg(column).arg(column+1);
		strList<<str;
	}
	strList<<"geometric mean";
	tableWidget->setHorizontalHeaderLabels(strList);
	strList<<"arithmetic mean";
	tableWidget->setHorizontalHeaderLabels(strList);
	
	
	for(row = 0 ; row < rows; row++)
	{
		column = 0;
		item = new QTableWidgetItem(QObject::tr("%1").arg(row+1,(int)log10(m_at3d.trackCount())+1,10,QChar('0')));
		tableWidget->setItem(row,column,item);
		AT3D::Track& track = m_at3d.track(row);
		double mean_deform1 = 1.0;
		double mean_deform2 = 0.0;
		while(column < m_numFrames)
		{
			
			if(column >= track.start && column < track.start+track.size())
			{
				if(column == track.start)
				{
					item = new QTableWidgetItem(QObject::tr("1.00"));
					tableWidget->setItem(row,column+1,item);
				}
				else
				{
					double deform = track[column].deformation(track[column-1], m_at3d.width(), m_at3d.height());
					mean_deform1 *= deform;
					mean_deform2 += deform;
					item = new QTableWidgetItem(QObject::tr("%1").arg(deform,4,'f',2,QChar('0')));
					tableWidget->setItem(row,column+1,item);
				}
			}
			column++;
		}
		column++;
		assert(track.size() >= 1);
		mean_deform1 = pow(mean_deform1, 1.0/(track.size() - 1));
		item = new QTableWidgetItem(QObject::tr("%1").arg(mean_deform1,4,'f',2,QChar('0')));
		tableWidget->setItem(row,column++,item);
		mean_deform2 = mean_deform2/(track.size() - 1);
		item = new QTableWidgetItem(QObject::tr("%1").arg(mean_deform2,4,'f',2,QChar('0')));
		tableWidget->setItem(row,column++,item);
	}
	tableWidget->show();
	tableWidget->setWindowTitle("Deformation");
	tableWidget->setSortingEnabled(true);
	tableWidget->sortByColumn(0,Qt::AscendingOrder);

	
}

//View Group
void AT3DVIEW::onNew3D()
{
	if(!isLoaded())return;
	GLWidget * newWidget = new GLWidget();
	newWidget->show();
	newWidget->setSize(m_at3d.width(),m_at3d.height(),m_at3d.depth());
	newWidget->setFrame(m_cells);
	newWidget->setWindowTitle(QObject::tr("New AT3DVIEW: Frame %1").arg(m_currentTime+1));
	newWidget->updateGL();
}

void AT3DVIEW::onResetView()
{
	if(m_at3d.empty())return;
	m_glWidget->reSetView();
	m_glWidget->updateGL();
}


void AT3DVIEW::onViewTree()
{
	if(!isLoaded()) return;
	if(!m_at3d.isTreeLoad()) 
	{
		QMessageBox::information(0,"","Please Load Trees First");
		return;
	}
	QTreeWidget* treeWidget = new QTreeWidget();
	treeWidget->setWindowTitle(QObject::tr("Tree %1").arg(m_currentTime+1));
	treeWidget->setColumnCount(3);
	QStringList header;
	header<<"id";
	header<<"level";
	header<<"size";
	treeWidget->setHeaderLabels(header);
	ComponentTree& tree = m_at3d.tree(m_currentTime);
	QTreeWidgetItem* item = getTreeItem(tree, treeWidget);
	treeWidget->insertTopLevelItem(0,item);
	treeWidget->show();
	treeWidget->setSortingEnabled(true);	
}

//Control Group
void AT3DVIEW::onFirst()
{
	if(m_at3d.empty())return;
	m_currentTime = 0;
	m_cells = m_at3d.frame(m_currentTime);

	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
	ui.glGroupBox->setTitle(tr("time : %1").arg(m_currentTime+1));
}

void AT3DVIEW::onLast()
{
	if(m_at3d.empty())return;
	m_currentTime = m_numFrames-1;
	m_cells = m_at3d.frame(m_currentTime);
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
	ui.glGroupBox->setTitle(tr("time : %1").arg(m_currentTime+1));
}

void AT3DVIEW::onPrevious()
{
	if(m_at3d.empty())return;
	m_currentTime = max(0,(int)(m_currentTime-1));
	m_cells = m_at3d.frame(m_currentTime);
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
	ui.glGroupBox->setTitle(tr("time : %1").arg(m_currentTime+1));
}

void AT3DVIEW::onNext()
{
	if(m_at3d.empty())return;
	m_currentTime = min(m_numFrames-1,m_currentTime+1);
	m_cells = m_at3d.frame(m_currentTime);
	
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
	ui.glGroupBox->setTitle(tr("time : %1").arg(m_currentTime+1));
}

//Cell Widget
void AT3DVIEW::onReverse()
{
	if(!isLoaded())return;
	vector<QCheckBox*>::iterator it = m_checkers.begin();
	while(it != m_checkers.end())
	{
		QCheckBox* checker = *it;
		if(checker->isChecked())checker->setCheckState(Qt::Unchecked);
		else checker->setCheckState(Qt::Checked);
		it++;
	}
}

void AT3DVIEW::onUndo()
{
	if(!isLoaded())return;
	m_at3d.undo();
	m_cells = m_at3d.frame(m_currentTime);	
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
}
/************************************************
 * When checkbox are checked and clicked the choose button
 ************************************************/
void AT3DVIEW::onChoose()
{
	if(!isLoaded())return;
	// Step 1: Get choosen cells
	vector<AT3D::Track*> ptracks;
	
	for(int i = 0; i < (int) m_cells.size(); i++)
	{
		if(m_checkers[i]->checkState() == Qt::Checked)
		{
			ptracks.push_back(m_cells[i]->track);
		}
		
	}
	// 2. choose
	m_at3d.choose(ptracks);
	// 3. refresh
	m_cells = m_at3d.frame(m_currentTime);
	
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
}

void AT3DVIEW::onDelete()
{
	if(!isLoaded())return;
	// Step 1: Get choosen cells
	vector<AT3D::Track*> ptracks;
	
	for(int i = 0; i < (int) m_cells.size(); i++)
	{
		if(m_checkers[i]->checkState() == Qt::Checked)
		{
			ptracks.push_back(m_cells[i]->track);
		}
		
	}
	// 2. choose
	m_at3d.remove(ptracks);
	// 3. refresh
	m_cells = m_at3d.frame(m_currentTime);
	
	setCellWidget();
	m_glWidget->setFrame(m_cells);
	m_glWidget->updateGL();
}

void AT3DVIEW::onNearestCellChoosed(int id)
{
	QCheckBox* checker = m_checkers[id];
	if(checker->isChecked())
	{
		checker->setChecked(false);
	}
	else
	{
		checker->setChecked(true);
	}
}

void AT3DVIEW::onCheckBoxChanged()
{
	// Step 1: get checked cell
	QCheckBox* checker = (QCheckBox *) sender();
	AT3D::Cell* cell = NULL;
	for(int i = 0; i < (int) m_cells.size(); i++)
	{
		if(m_checkers[i] == checker) 
		{
			cell = m_cells[i];
			break;
		}
	}
	assert(cell != NULL);
	if(cell->centers.empty())cell->setCenters(m_at3d.width(), m_at3d.height());
	if(checker->isChecked())
	{
		cell->needSign = true;
	}
	else
	{
		cell->needSign = false;
	}
	
	// Step 3: update glWidget
	m_glWidget->updateGL();
}


/************************************************
 * SLot Functions End
 ************************************************/


/*******************************************
 * functions begins
 *******************************************/

QString getColorValue(AT3D::rgb_pixel c)
{
	int high,low;
	QString out;
	high=c.r/16;
	low=c.r%16;
	out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
	high=c.g/16;
	low=c.g%16;
	out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
	high=c.b/16;
	low=c.b%16;
	out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
	return out;
}

QString hex2str(int v)
{
	if(v<0 || v>15)return QString("");
	else
	{
		switch(v)
		{
			case 10: return QString("a");
			case 11: return QString("b");
			case 12: return QString("c");
			case 13: return QString("d");
			case 14: return QString("e");
			case 15: return QString("f");
			default: return QObject::tr("%1").arg(v);
		}
	}
}

void AT3DVIEW::init()
{
	clear();
}

void AT3DVIEW::clear()
{
	m_at3d.clear();
	m_cells.clear();
	m_checkers.clear();
	
	m_currentTime = 0;
	m_numFrames = 0;
}

bool AT3DVIEW::isLoaded()
{
	return(! m_at3d.empty());
}

//============================================
// setCellWidget: set m_checkers according to m_cells
//============================================
void AT3DVIEW::setCellWidget()
{
	vector<QTextEdit*> editors;
	// 1. delete exist widget and check boxs
	if(!m_checkers.empty())
	{
		ui.cellWidget->close();
		delete ui.cellWidget;
		m_checkers.clear();
	}
	// 2. create new widget and new check boxs
	ui.cellWidget = new QWidget(ui.scrollArea);

	m_checkers.resize(m_cells.size());

	editors.resize(m_cells.size());
	for(int i =0 ; i < (int)m_cells.size(); i++)
	{
		m_checkers[i] = new QCheckBox();
		editors[i] = new QTextEdit();
		m_cells[i]->needSign = false;
	}
	
	QGridLayout* layout= new QGridLayout(ui.cellWidget);
	
	
	// 3. set connection
	for(int i = 0; i <(int) m_cells.size(); i++)
	{
		QCheckBox* checker = m_checkers[i];
		QTextEdit* editor = editors[i];
		AT3D::Cell* cell = m_cells[i];
		AT3D::rgb_pixel color = cell->color;
				
		QString text = tr("<span style=\" color:#%1;\">%2</span>")
						.arg(getColorValue(color))
						.arg(cell->points.size());
		editor->setReadOnly(true);
		editor->setFixedHeight(20);
		editor->setFixedWidth(120);
		editor->setText(text);
		
		connect(checker,SIGNAL(stateChanged(int)),this,SLOT(onCheckBoxChanged()));
		checker->setText(tr("%1: ").arg(cell->track->trackId + 1));
		layout->addWidget(checker, i, 0);
		layout->addWidget(editor, i, 1);
	}
	layout->setVerticalSpacing(10);
	ui.cellWidget->setLayout(layout);
	ui.scrollArea->setWidget(ui.cellWidget);
}

QTreeWidgetItem* getTreeItem(ComponentTree & tree, QTreeWidget* widget)
{
	map<ComponentTree::Node*, QTreeWidgetItem*> item;
	QStringList strList;
	QTreeWidgetItem* rootItem;
	
	
	ComponentTree::iterator it = tree.begin(ComponentTree::BREADTH_FIRST);
	ComponentTree::iterator it_end = tree.end(ComponentTree::BREADTH_FIRST);

	//assert(it != tree.end(ComponentTree::BREADTH_FIRST));
	
	strList<<QObject::tr("%1").arg((*it)->label);
	strList<<QObject::tr("%1").arg((*it)->level);
        strList<<QObject::tr("%1").arg((*it)->alpha_size);
	rootItem = new QTreeWidgetItem(widget,strList);
	rootItem->setExpanded(true);
	item[*it] = rootItem;
	it++;
	while(it != it_end )
	{
		strList.clear();
		strList<<QObject::tr("%1").arg((*it)->label);
		strList<<QObject::tr("%1").arg((*it)->level);
                strList<<QObject::tr("%1").arg((*it)->alpha_size);

		QTreeWidgetItem* parent_item = item[(*it)->parent];
		
		QTreeWidgetItem* new_item = new QTreeWidgetItem(parent_item, strList);
		new_item->setExpanded(true);
		item[*it] = new_item;
		it++;
	}
	
	return rootItem;
}

void AT3DVIEW::on_adjustRegionButton_clicked()
{
    //if(!isLoaded())return;
    AdjustRegionDialog * dlg = new AdjustRegionDialog(this);
    dlg->exec();
}
