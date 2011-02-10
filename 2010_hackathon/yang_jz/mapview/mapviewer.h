/* mapviewer.h
 * 2011-01-26 the program is created by Yang jinzhu
 */


#ifndef __MAPVIEWER__
#define __MAPVIEWER__

// maviewer
#include <qwidget.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <vector>
#include <list>
#include <bitset>

#include <set>

// reader 
#include "basic_surf_objs.h"
#include "stackutil-11.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib11.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"
#include "/usr/local/Trolltech/Qt-4.5.2/demos/shared/arthurwidgets.h"
// multithreads
#include <pthread.h>
// interface v3d
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>
#include "colormap.h"
//
#include <QBasicTimer>
#include <QPolygonF>
//#include <QMainWindow>
#include <qgroupbox.h>

//class HoverPoints;
//class QLineEdit;
//class QLabel;
//class QScrollBar;
//class QSpinBox;
//class QRadioButton;
//class QPushButton;
//class QHBoxLayout;
//class QGridLayout;
//class QVBoxLayout;
//class QCheckBox;
//class QTextBrowser;
//class XMapView;

using namespace std;
enum ImagePlaneDisplayType {imgPlaneUndefined, imgPlaneX, imgPlaneY, imgPlaneZ};// define indexed data structures

template <class T> QPixmap copyRaw2QPixmap_xPlanes(const T * pada, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_yPlanes(const T * pada, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap_zPlanes(const T * pada, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,double *p_vmax, double *p_vmin);
template <class T> QPixmap copyRaw2QPixmap(const T * pada, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,ImagePlaneDisplayType disType, double *p_vmax, double *p_vmin);


// Define a lookup table
template <class T>
class LUT
{
public:
	
	LUT(){}
	
	LUT(T *a, T *b, bool offset_region)
	{
		T len = 3; //start.size();
		
		if(offset_region)
		{
			init_by_offset(a,b,len);
		}
		else
		{
			init_by_region(a,b,len);
		}
	}
	
	~LUT(){}
	
public:
	void init_by_offset(T *offsets, T *dims, T len)
	{
		try
		{
			start_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			start_pos[i] = offsets[i];
		
		try
		{
			end_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			end_pos[i] = start_pos[i] + dims[i] - 1;
	}
	
	void init_by_region(T *start, T *end, T len)
	{
		try
		{
			start_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			start_pos[i] = start[i];
		
		try
		{
			end_pos = new T [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T i=0; i<len; i++)
			end_pos[i] = end[i];
	}
	
	void clear()
	{
		if(start_pos) {delete []start_pos; start_pos=0;}
		if(end_pos) {delete []end_pos; end_pos=0;}
	}
	
	
public:
	
	T *start_pos;
	T *end_pos;
	
	string fn_img;
	
};

// Define a indexed data structure
template <class T1, class T2>
class indexed_t
{
public:
	indexed_t(T1 *in_offsets)
	{
		T1 len = 3; //in_offsets.size();
		
		try
		{
			offsets = new T1 [len];
			sz_image = new T1 [len+1]; // X Y Z C
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		// init
		for(T1 i=0; i<len; i++)
		{
			offsets[i] = in_offsets[i];
			sz_image[i] = 1;
		}
		sz_image[3] = 1;
		
	}
	~indexed_t(){}
	
public:
	T1 *offsets; // ref
	T1 ref_n; // reference image number
	T1 n;
	
	T2 score;
	
	string fn_image; // absolute path + file name
	T1 *sz_image;
	
	T1 predecessor; // adjacent prior image number | root's predecessor is -1
	bool visited; // init by false
	
	std::vector<indexed_t> record;
	
};


// Virtual Image Class
template <class T1, class T2, class indexed_t, class LUT>
class Y_VIM 
{
	
public:
	
	//init
	// creating a hash table
	Y_VIM(list<string> imgList, T2 dims)
	{
		// finding best global alignment
		
		
	}	
	
	Y_VIM(){}
	
	// destructor
	~Y_VIM(){}
	
public:
	
	//load a virtual image
	void y_load(string fn)
	{
		ifstream pFileLUT(fn.c_str());
		string str;
		
		char letter;
		
		T2 start[3], end[3];
		
		sz = new T2 [3];
		
		char buf[2000];
		string fn_str;
		
		if(pFileLUT.is_open())
		{
			//
			pFileLUT >> letter;
			
			if(letter=='#')
				getline(pFileLUT, str);
			
			// tiles
			pFileLUT >> number_tiles;
			
			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str);
			
			// dimensions
			pFileLUT >> sz[0] >> sz[1] >> sz[2] >> sz[3];
			
			do
			{
				pFileLUT >> letter;
			}
			while(letter!='#');
			
			getline(pFileLUT, str);
			
			// lut
			lut = new LUT [number_tiles];
			
			T2 count=0;
			
			while( !pFileLUT.eof() )
			{
				while( getline(pFileLUT, str) )
				{
					istringstream iss(str);
					
					iss >> buf;
					fn_str = buf;
					//
					iss >> buf; iss >> start[0];
					iss >> buf; iss >> start[1];
					iss >> buf; iss >> start[2];
					
					iss >> buf;
					
					iss >> buf; iss >> end[0];
					iss >> buf; iss >> end[1];
					iss >> buf; iss >> end[2];
					
					lut[count] = LUT(start, end, false);
					lut[count].fn_img = fn_str;
					
					count++;
					
					//iss >> letter;
				}
				
			}
			
		}
		else
		{
			cout << "Unable to open the file"<<endl;
			return;
		}
		
		pFileLUT.close();
		
		
		// adjusting
		T2 len = 3;
		
		try
		{
			min_vim = new T2 [len];
			max_vim = new T2 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T2 i=0; i<len; i++)
		{
			min_vim[i] = 0; max_vim[i] = 0;
		}
		
		for(T2 i=0; i<number_tiles; i++)
		{
			for(T2 j=0; j<len; j++)
			{
				if(lut[i].start_pos[j] < min_vim[j])
					min_vim[j] = lut[i].start_pos[j];
				
				if(lut[i].start_pos[j] > max_vim[j])
					max_vim[j] = lut[i].start_pos[j];
			}
			
		}
	}
	
	//save as a virtual image
	void y_save(string fn)
	{
		FILE *pFileLUT=0;
		
		pFileLUT = fopen(fn.c_str(),"wt");
		
		fprintf(pFileLUT, "# tiles \n");
		fprintf(pFileLUT, "%d \n\n", tilesList.size());
		
		fprintf(pFileLUT, "# dimensions (XYZC) \n");
		fprintf(pFileLUT, "%ld %ld %ld %ld \n\n", max_vim[0]-min_vim[0]+1, max_vim[1]-min_vim[1]+1, max_vim[2]-min_vim[2]+1, tilesList.at(0).sz_image[3]);
		
		fprintf(pFileLUT, "# image coordinates look up table \n");
		for(int j=0; j<tilesList.size(); j++)
		{
			
			string fn = QString(lut[j].fn_img.c_str()).remove(0, QFileInfo(QString(lut[j].fn_img.c_str())).path().length()+1).toStdString();
			
			fprintf(pFileLUT, "%s  ( %ld, %ld, %ld ) ( %ld, %ld, %ld ) \n", fn.c_str(), lut[j].start_pos[0], lut[j].start_pos[1], lut[j].start_pos[2], lut[j].end_pos[0], lut[j].end_pos[1], lut[j].end_pos[2]);
		}
		
		fclose(pFileLUT);
	}
	
	// when add a new one into tileList, need to update the whole tileList
	void y_update()
	{
		
	}
	
	// make a visual image real and be loaded into memory
	void y_visualize(T2 *start, T2 *end)
	{
		
	}
	
	// make a visual image real and be loaded into memory
	void y_visualize()
	{
		
	}
	
	// show a header info
	void y_info()
	{
		
	}
	
	// construct lookup table given adjusted tilesList
	void y_clut(T2 n)
	{
		lut = new LUT [n];
		
		for(T2 i=0; i<n; i++)
		{
			lut[i] = LUT(tilesList.at(i).offsets, tilesList.at(i).sz_image, true);
			
			lut[i].fn_img = tilesList.at(i).fn_image;
		}
		
		// suppose image dimension is unsigned
		T2 len = 3;
		
		try
		{
			min_vim = new T2 [len];
			max_vim = new T2 [len];
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		
		for(T2 i=0; i<len; i++)
		{
			min_vim[i] = 0; max_vim[i] = 0;
		}
		
		for(T2 i=0; i<n; i++)
		{
			for(T2 j=0; j<len; j++)
			{
				if(lut[i].start_pos[j] < min_vim[j])
					min_vim[j] = lut[i].start_pos[j];
				
				if(lut[i].end_pos[j] > max_vim[j])
					max_vim[j] = lut[i].end_pos[j];
			}
			
		}
		
	}
	
	void y_clear()
	{
		if(pVim) {delete []pVim; pVim=0;}
		if(sz) {delete []sz; sz=0;}
		
		if(min_vim) {delete []min_vim; min_vim=0;}
		if(max_vim) {delete []max_vim; max_vim=0;}
		if(lut) {delete []lut; lut=0;}
	}
	
public:
	
	T1 *pVim;
	T2 *sz;
	
	vector<indexed_t> tilesList;
	bitset<3> relative_dir; // 000 'f', 'u', 'l' ; 111 'b', 'd', 'r'; relative[2] relative[1] relative[0] 
	
	LUT *lut;
	T2 *min_vim, *max_vim;
	
	T2 number_tiles;
	
};

//
class XMapView : public QWidget//, public V3DPluginInterface
{
    
	Q_OBJECT
	//Q_INTERFACES(V3DPluginInterface);
   // Q_PROPERTY(double scale READ scale WRITE changeScale)	
public:
	XMapView(QWidget *parent);
	
	void setImgData(ImagePlaneDisplayType ptype, V3DLONG *sz_compressed,V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,unsigned char *pdata, ImageDisplayColorType ctype);
		
	void Setwidget(V3DPluginCallback &callback, QString m_FileName, QString curFilePathInput, float scaleFactorInput);
	
	void update_v3dviews(V3DPluginCallback *callback, long start_x, long start_y, long start_z, long end_x, long end_y, long end_z);
	
	int disp_width, disp_height;
	int disp_scale;
	
	bool flag_syn;
	
	QPoint dragStartPosition;
	
	QPoint dragEndPosition;
	
	long cx, cy, cz,cc;
	long cur_x, cur_y, cur_z;
	long plane_n;
	long start_x,start_y,start_z;
	long end_x,end_y,end_z;
	
	V3DPluginCallback *callback1;
	
	QString curFilePath;
	
	float scaleFactor;
	
	int get_disp_width() {return disp_width;}
	
	int get_disp_height() {return disp_height;}
	
	double get_disp_scale() {return disp_scale;}
	
	void set_disp_width(int a) {disp_width = a;}
	void set_disp_height(int a) {disp_height = a;}
	void set_disp_scale(double a) {disp_scale = a; }
	
	int focusPlaneCoord() {return cur_focus_pos;}	
	double scale() const { return m_scale; }
    void drawPixmapType(QPainter *painter);	
	void paintEvent(QPaintEvent *event);
	
	void drawROI(QPainter *painter);
	
	void mousePressEvent(QMouseEvent *e);
    void mouseLeftButtonPressEvent(QMouseEvent *e);
    void mouseRightButtonPressEvent(QMouseEvent *e);
	void mouseMoveEvent (QMouseEvent * e);
    void enterEvent (QEvent * e);
    void leaveEvent (QEvent * e);
	 
	void mouseReleaseEvent(QMouseEvent * e);
	
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	
public slots:
   
	
private:
	enum XFormType { VectorType, PixmapType, TextType};   
	double m_scale;
	
	QPointF curDisplayCenter;
	QPointF curDisplayCenter0;
	
	QPoint curMousePos;
	bool bMouseCurorIn;
	
	unsigned char *imagData;
	
	QCursor myCursor;	
	
	XFormType Gtype;	

	ImageDisplayColorType Ctype;
    ImagePlaneDisplayType Ptype;
	
    QPixmap pixmap; //xy plane
    int cur_focus_pos;
	
	bool b_displayFocusCrossLine;
	bool b_mouseend;
	int focusPosInWidth, focusPosInHeight;
	
	bool b_moveCurrentLandmark;
	
	unsigned char *compressed;
	
	long roi_top, roi_left, roi_bottom, roi_right;
	//QImage sourceImage;
	long channel_compressed_sz;
	
	unsigned char *compressed1d;
	unsigned char *imgData;
	
private:
};
// MAPViewer interface
class ImageSetWidget : public QWidget//,public QDialog 
{
	Q_OBJECT
	
public:
	 ImageSetWidget(V3DPluginCallback &callback, QWidget *parent,QString m_FileName, QString curFilePathInput, float scaleFactorInput);
	
	void update_v3dviews(V3DPluginCallback *callback, long start_x, long start_y, long start_z, long end_x, long end_y, long end_z);
	
	void createGUI();
	void update_triview();
	
public slots:
	void updateGUI();
	void drawdata();
public:
//*********************************************************
	QGroupBox *dataGroup, *viewGroup, *infoGroup;
	QGroupBox *mainGroup, *coordGroup, *scaleGroup, *typeGroup;
	
	QScrollBar *xSlider, *ySlider, *zSlider;
	QSpinBox *xValueSpinBox, *yValueSpinBox, *zValueSpinBox;
	
	QLabel *xSliderLabel, *ySliderLabel, *zSliderLabel;
	
	QPushButton* dataCopyButton;
	
	
	QHBoxLayout *allLayout;
	
	QVBoxLayout *dataGroupLayout;
	
	QGridLayout *xyzViewLayout;
	
	QVBoxLayout *mainGroupLayout;
	
	QGridLayout *coordGroupLayout;
	
	QGridLayout *datacopyGroupLayout;
   
	V3DPluginCallback *callback1;
	 
	XMapView *xy_view; //change in Z
    XMapView *yz_view; //change in X
    XMapView *zx_view; //change in Y
	XMapView *mapview ;
	
	//******************
public:
	long cx, cy, cz, cc; // compressed data
	long cur_x, cur_y, cur_z;
	long channel_compressed_sz;
	long init_x, init_y, init_z; // control window
	long wx, wy, wz;
	
	long roi_start_x, roi_start_y, roi_start_z;
	
	long roi_end_x, roi_end_y, roi_end_z;
	
	unsigned char *compressed1d;
	
	V3DLONG *sz_compressed;
	
	QString curFilePath;

	float scaleFactor;
	// virtual image
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	bool Bcopy;
	long flag_changed;
};

// interface v3d plugin
class MAPiewerPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
// v3d interface	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}	
	void iViewer(V3DPluginCallback &callback, QWidget *parent);
public:
	
};


// indexed data structure
class IndexedData
{
	
};


#endif



