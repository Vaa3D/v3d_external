/* miviewer.h
 * 2010-07-28: create this program by Yang Yu
 */


#ifndef __MIVIEWER__
#define __MIVIEWER__

// miviewer
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
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

// interface v3d
#include <QtGui>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

//
using namespace std;

//
class tri_view_plane : public QLabel 
{
public:
	tri_view_plane(QString text)
	{
		QLabel(test);
	}
	
public:
	void paintEvent(QPaintEvent *event)
	{
		//
		//QWidget::paintEvent(event);
		
		//
		QPainter painter(this); 
		
		painter.setCompositionMode(QPainter::CompositionMode_Source);
		painter.drawImage(0,0, sourceImage);
		
		painter.setPen( Qt::white );				// draw outline Qt::SolidLine
		painter.setBrush( Qt::NoBrush );	// set random brush color colors[rand() % 255] Qt::NoBrush
		
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // override
		
		QPoint p1( roi_top, roi_left );    // p1 = top left
		QPoint p2( roi_bottom, roi_right );    // p2 = bottom right
		
		QRect r( p1, p2 );
		painter.drawRect( r );
		
		qDebug()<<"paint ..."<<roi_top<<roi_left<<roi_bottom<<roi_right;
		
	}
	
public:
	long roi_top, roi_left, roi_bottom, roi_right;
	QImage sourceImage;
	
};

// MIViewer interface
class ImageNavigatingWidget : public QWidget
{
	Q_OBJECT
	
public:
	ImageNavigatingWidget(long sx, long sy, long sz, QString m_FileName, float scaleFactor)
	{
		// compressed data
		//****************************************************************
		// suppose compressed image saved as .tif
		QString m_FileName_compressed = m_FileName;
		
		m_FileName_compressed.chop(3); // ".tc"
		m_FileName_compressed.append(".tif");
		
		// loading compressed image files
		V3DLONG *sz_compressed = 0; 
		int datatype_compressed = 0;
		unsigned char* compressed1d = 0;
		
		loadImage(const_cast<char *>(m_FileName_compressed.toStdString().c_str()), compressed1d, sz_compressed, datatype_compressed); //careful
		
		cx=sz_compressed[0], cy=sz_compressed[1], cz=sz_compressed[2], cc=sz_compressed[3];
		channel_compressed_sz = cx*cy*cz;
		
		// initial
		init_x = cx/2, init_y = cy/2, init_z = cz/2; 
		
		cur_x = init_x*scaleFactor, cur_y = init_y*scaleFactor, cur_z = init_z*scaleFactor;

		
		//******************************************************************
		// create a widget
		label_xy = new tri_view_plane(QObject::tr("xy-view")); 
		label_zy = new tri_view_plane(QObject::tr("zy-view")); 
		label_xz = new tri_view_plane(QObject::tr("xz-view")); 
		
		//
		label_x = new QLabel(QObject::tr("current tri-view plane: x"));
		label_y = new QLabel(QObject::tr("y"));
		label_z = new QLabel(QObject::tr("z"));
		
		spin_x = new QSpinBox();
		spin_y = new QSpinBox();
		spin_z = new QSpinBox();
		
		spin_x->setMaximum(sx); spin_x->setMinimum(0); spin_x->setValue(cur_x); spin_x->setSingleStep(int(scaleFactor));
		spin_y->setMaximum(sy); spin_y->setMinimum(0); spin_y->setValue(cur_y); spin_y->setSingleStep(int(scaleFactor));
		spin_z->setMaximum(sz); spin_z->setMinimum(0); spin_z->setValue(cur_z); spin_z->setSingleStep(int(scaleFactor));
		
		// choose focus plane
		label_plane = new QLabel(QObject::tr("choose focus plane: "));
		combo_plane =  new QComboBox(); 
		combo_plane->addItem("xy-view");
		combo_plane->addItem("yz-view");
		combo_plane->addItem("xz-view");
		
		//
		zoom_in  = new QPushButton("Zoom In");
		zoom_out = new QPushButton("Zoom Out");
		
		//
		move_up  = new QPushButton("Up");
		move_down = new QPushButton("Down");
		move_left  = new QPushButton("Left");
		move_right = new QPushButton("Right");
		
		//
		syn = new QPushButton("Syn");
		
		//gridlayout
		QGridLayout* gridLayout = new QGridLayout(this);
		
		gridLayout->addWidget(label_xy, 0,0,1,2); gridLayout->addWidget(label_zy, 0,3,1,2);
		gridLayout->addWidget(label_xz, 1,0,1,2);
		
		gridLayout->addWidget(label_plane, 2,0,1,1); gridLayout->addWidget(combo_plane, 2,1,1,1);
		
		gridLayout->addWidget(label_x, 3,0,1,1); gridLayout->addWidget(spin_x, 3,1,1,1);
		gridLayout->addWidget(label_y, 3,2,1,1); gridLayout->addWidget(spin_y, 3,3,1,1);
		gridLayout->addWidget(label_z, 3,4,1,1); gridLayout->addWidget(spin_z, 3,5,1,1);
		
		gridLayout->addWidget(move_up, 4,1,1,1); gridLayout->addWidget(move_down, 6,1,1,1);
		gridLayout->addWidget(move_left, 5,0,1,1); gridLayout->addWidget(move_right, 5,3,1,1);
		
		gridLayout->addWidget(zoom_in, 7,0,1,1); gridLayout->addWidget(zoom_out, 7,1,1,1); gridLayout->addWidget(syn, 7,2,1,1);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Image Navigation Control"));
		
		//init
		roi_start_x = 0, roi_start_y = 0, roi_start_z = 0;
		roi_end_x = 0, roi_end_y = 0, roi_end_z = 0;
		
		wx = cx/2, wy = cy/2, wz = cz/2;
		
		//
		update_roi(roi_start_x, roi_start_y, roi_start_z, wx, wy, wz);
		
		// init tri-view
		update_triview(compressed1d, cur_x, cur_y, cur_z);
		
		// init roi
		label_xy->roi_top = roi_start_x; label_xy->roi_left = roi_start_y;
		label_xy->roi_bottom = roi_end_x; label_xy->roi_right = roi_end_y;
		
		label_zy->roi_top = roi_start_z; label_zy->roi_left = roi_start_y;
		label_zy->roi_bottom = roi_end_z; label_zy->roi_right = roi_end_y;
		
		label_xz->roi_top = roi_start_x; label_xz->roi_left = roi_start_z;
		label_xz->roi_bottom = roi_end_x; label_xz->roi_right = roi_end_z;
		
		//signal and slot
		//connect(syn,     SIGNAL(clicked()), this, SLOT(paint(QPaintEvent *)));
		
		
	}
	
//public:
//	void paintEvent(QPaintEvent *event)
//	{
//		//
//		//QWidget::paintEvent(event);
//		
//		//
//		QPainter paint_xy; //( label_xy ); // label_xy->pixmap() ); //label_xy
//		
//		paint_xy.begin(label_xy);
//		
//		paint_xy.setPen( Qt::white );				// draw outline Qt::SolidLine
//		paint_xy.setBrush( Qt::NoBrush );	// set random brush color colors[rand() % 255] Qt::NoBrush
//		
//		QPoint p1( roi_start_x, roi_start_y );    // p1 = top left
//		QPoint p2( roi_end_x, roi_end_y );    // p2 = bottom right
//		
//		QRect r( p1, p2 );
//		paint_xy.drawRect( r );
//		
//		paint_xy.end();
//		
//		qDebug()<<"paint ..."<<roi_start_x<<roi_start_y<<roi_end_x<<roi_end_y;
//		
//	}
	
public slots:
	void update_windows()
	{
		
	}
	
	void update_roi(long start_x, long start_y, long start_z, long wx, long wy, long wz)
	{
		// ROI
		roi_start_x = start_x, roi_start_y = start_y, roi_start_z = start_z;
		roi_end_x = roi_start_x + wx, roi_end_y = roi_start_y + wy, roi_end_z = roi_start_z + wz;
	
	}
	
	void update_triview(unsigned char *compressed1d, long cur_x, long cur_y, long cur_z)
	{
		qDebug()<<"update_triview ...";
		
		QImage xy_image(cx,cy,QImage::Format_RGB32);
		
		for (long j = 0; j < cy; j ++) 
		{
			long offset = init_z*cx*cy + j*cx;
			for (long i=0; i<cx; i++) 
			{
				long idx = offset + i;
				
				xy_image.setPixel(i,j,qRgb(compressed1d[idx], compressed1d[idx+channel_compressed_sz], compressed1d[idx+2*channel_compressed_sz]));
			}
		}
		//label_xy->setPixmap(QPixmap::fromImage(xy_image));
		label_xy->sourceImage = xy_image;
		
		QImage zy_image(cz,cy,QImage::Format_RGB32);
		
		for (long j = 0; j < cy; j ++) 
		{
			long offset = j*cx + init_x;
			for (long k =0; k < cz; k++) 
			{
				long idx = offset + k*cx*cy;
				
				zy_image.setPixel(k,j,qRgb(compressed1d[idx], compressed1d[idx+channel_compressed_sz], compressed1d[idx+2*channel_compressed_sz]));
			}
		}
		//label_zy->setPixmap(QPixmap::fromImage(zy_image));
		label_zy->sourceImage = zy_image;
		
		QImage xz_image(cx,cz,QImage::Format_RGB32);
		
		for (long k = 0; k < cz; k ++) 
		{
			long offset = k*cx*cy + init_y*cx;
			for (long i =0; i < cx; i++) 
			{
				long idx = offset + i;
				
				xz_image.setPixel(i,k,qRgb(compressed1d[idx], compressed1d[idx+channel_compressed_sz], compressed1d[idx+2*channel_compressed_sz]));
			}
		}
		//label_xz->setPixmap(QPixmap::fromImage(xz_image));
		label_xz->sourceImage = xz_image;
		
	}
	
	
public:
	tri_view_plane* label_xy;
	tri_view_plane* label_zy;
	tri_view_plane* label_xz;
	
	QPushButton* zoom_in;
	QPushButton* zoom_out;
	
	QLabel* label_x;
	QSpinBox* spin_x;
	QLabel* label_y;
	QSpinBox* spin_y;
	QLabel* label_z;
	QSpinBox* spin_z;
	
	QLabel* label_plane;
	QComboBox* combo_plane;
	
	QPushButton* move_up;
	QPushButton* move_down;
	QPushButton* move_left;
	QPushButton* move_right;
	
	QPushButton* syn;
	
public:
	long cx, cy, cz, cc; // compressed data
	long cur_x, cur_y, cur_z;
	long channel_compressed_sz;
	long init_x, init_y, init_z; // control window
	long wx, wy, wz;
	long roi_start_x, roi_start_y, roi_start_z;
	long roi_end_x, roi_end_y, roi_end_z;
	
};


// interface v3d plugin
class MIViewerPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
// v3d interface	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

};

 
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

// indexed data structure
class IndexedData
{
	
};


// test
class MicroscopeFocusControls: public QMainWindow
{
	Q_OBJECT
	
public:
	MicroscopeFocusControls()
	{
		imageLabel = new QLabel;
		imageLabel->setBackgroundRole(QPalette::Base);
		imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		imageLabel->setScaledContents(true);
		
		scrollArea = new QScrollArea;
		scrollArea->setBackgroundRole(QPalette::Dark);
		scrollArea->setWidget(imageLabel);
		setCentralWidget(scrollArea);
		
		setWindowTitle(tr("Focus Controls"));
		resize(500, 400);
		
	};
	
	
	public slots:
	void update(QImage& qIM)
	{
		qDebug()<<"showing...";
		imageLabel->setPixmap(QPixmap::fromImage(qIM));
		qDebug()<<"shown...";
		imageLabel->adjustSize();
		qDebug()<<"adjusted...";
	}
	
public:
	
	QLabel *imageLabel;
	QScrollArea *scrollArea;
	
};



#endif



