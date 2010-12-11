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
 * xformwidget.h
 *
 * Aug 10, 2010.  RZC: moved from v3d_core.h
 *
 */

#ifndef XFORMWIDGET_H_
#define XFORMWIDGET_H_

#include "v3d_core.h"
#include "../basic_c_fun/basic_triview.h"

class V3dR_MainWindow;
class V3dR_GLWidget;
class MainWindow;
struct iDrawExternalParameter
{
	My4DImage* image4d;
	XFormWidget *xwidget;
	V3dR_MainWindow *window3D; //pointer to 3DView main window, by RZC 080921, 090503
	QList <V3dR_MainWindow *> * p_list_3Dview_win; //by PHC, 081003. This is linked to a mainwindow, which will keep record of all generated V3dR_MainWindows
	MainWindow *V3Dmainwindow; //the pointer to the V3D main window, so that V3dR can get the global setting preferences. add on 090503

	QStringList swc_file_list;
	QStringList pointcloud_file_list;
	QString surface_file;
	QString labelfield_file;

	//some external controls for the 3d viewer
	//float zthickness; //the default z-thickness when start the 3d viewer. 100626

	int b_local; // 0,1,2,3
	LocationSimple local_start, local_size;
	QPoint local_win_pos;
	QSize local_win_size;

	bool b_use_512x512x256;
	bool b_still_open;
	iDrawExternalParameter() {b_use_512x512x256=true; b_local=b_still_open=false; image4d=0; xwidget=0; window3D=0; p_list_3Dview_win=0; V3Dmainwindow=0; /*zthickness=1;*/}
	~iDrawExternalParameter() {if (xwidget==0 && image4d) delete image4d;}
};

class XFormWidget : public QWidget, public TriviewControl //class XFormWidget : public QMainWindow
{
    Q_OBJECT;

public:
    XFormWidget(QWidget *parent);
    XFormWidget(QWidget *parent, Qt::WidgetAttribute f); //080814 add the second arugment. This is eventually not used. Just keep here for further use
    ~XFormWidget();

    void initialize();
	void createGUI();
	void connectEventSignals();
	void disconnectEventSignals();
	void updateDataRelatedGUI();
    bool setCTypeBasedOnImageData();
	void cleanData();
	bool loadData();
	bool importLeicaData();
	bool importGeneralImgSeries(const QStringList & mylist, TimePackType timepacktype);

    bool loadFile(QString filename);
    bool importGeneralImageFile(QString filename);
    bool importLeicaFile(QString filename);
	QString userFriendlyCurrentFile() {return (openFileNameLabel);}
	QString getOpenFileNameLabel() {return openFileNameLabel;}

	bool saveData();
	bool saveFile(QString filename);

	My4DImage * getImageData() {return imgData;} //080326

	bool newProcessedImage(QString filename, unsigned char *ndata1d, V3DLONG nsz0, V3DLONG nsz1, V3DLONG nsz2, V3DLONG nsz3, ImagePixelType ndatatype); //080408
	// for replace imageData and filename
	bool setImageData(unsigned char *ndata1d, V3DLONG nsz0, V3DLONG nsz1, V3DLONG nsz2, V3DLONG nsz3, ImagePixelType ndatatype); //090818 RZC
	bool setCurrentFileName(QString cfilename);


	//for communication of windows
	void setMainControlWindow(MainWindow * p) {p_mainWindow = p;}
	MainWindow * getMainControlWindow() {return p_mainWindow;}

	void forceToChangeFocus(int x, int y, int z); //081210
	void changeFocusFromExternal(int x, int y, int z); //this should be called from external. When no cross-image communication is needed, should not use this.
    void changeFocusToExternal(int newx, int newy, int newz); // this is the function to call other image-view's changeFocusFromExternal() function

	void setWindowTitle_Prefix(char *prefix);
	void setWindowTitle_Suffix(char *sfix);

	V3DLONG estimateRoughAmountUsedMemory(); //080810

	bool getFlagImgValScaleDisplay(); //100814

	My4DImage * selectSubjectImage();
	My4DImage * selectImage();
	QList <BlendingImageInfo> selectBlendingImages();

	void setColorType(ImageDisplayColorType myctype) {Ctype = myctype;}
	ImageDisplayColorType getColorType() {return Ctype;}

	iDrawExternalParameter mypara_3Dview;
	iDrawExternalParameter mypara_3Dlocalview;
	V3D_atlas_viewerDialog *atlasViewerDlg;
	
	V3DLONG bbx0, bbx1, bby0, bby1, bbz0, bbz1; //by PHC. 100821. the current regional bbox. for curve based zoomin
	void setLocal3DViewerBBox(V3DLONG x0, V3DLONG x1, V3DLONG y0, V3DLONG y1, V3DLONG z0, V3DLONG z1)
	{
		bbx0 = x0;
		bbx1 = x1;
		bby0 = y0;
		bby1 = y1;
		bbz0 = z0;
		bbz1 = z1;
	}
	

	double disp_zoom; //081114
	bool b_use_dispzoom;

	bool bDispMarkerLabel;

protected:
    void keyPressEvent ( QKeyEvent * e); //100815, PHC
	void closeEvent ( QCloseEvent * event );  //080814
	//void focusInEvent ( QFocusEvent * event ); //080829

private:
	// communication of different images windows
	MainWindow * p_mainWindow;
	bool bSendSignalToExternal, bAcceptSignalFromExternal;

	My4DImage *imgData;
    ImageDisplayColorType Ctype;

	QString openFileNameLabel;

	// Interface components
	bool bExistGUI;
    bool bLinkFocusViews;
    bool bDisplayFocusCross;

	QGroupBox *dataGroup, *viewGroup, *infoGroup;
	QGroupBox *mainGroup, *coordGroup, *scaleGroup, *typeGroup;

	QScrollBar *xSlider, *ySlider, *zSlider;
	QSpinBox *xValueSpinBox, *yValueSpinBox, *zValueSpinBox;
	QLabel *xSliderLabel, *ySliderLabel, *zSliderLabel;
	QCheckBox *linkFocusCheckBox;
	QCheckBox *displayFocusCrossCheckBox;

    QScrollBar *xScaleSlider, *yScaleSlider, *zScaleSlider;
	QLabel *xScaleSliderLabel, *yScaleSliderLabel, *zScaleSliderLabel;
	QCheckBox *lookingGlassCheckBox;
	QPushButton *zoomWholeViewButton;

    QRadioButton *colorRedType, *colorGreenType, *colorBlueType, *colorAllType;
    QRadioButton *colorRed2GrayType, *colorGreen2GrayType, *colorBlue2GrayType, *colorAll2GrayType;
    QRadioButton *colorMapDispType;
	QCheckBox *imgValScaleDisplayCheckBox;

	QCheckBox * cBox_bSendSignalToExternal, * cBox_bAcceptSignalFromExternal;

    QPushButton *landmarkCopyButton, *landmarkPasteButton, *landmarkSaveButton, *landmarkLoadButton; //080107
	QPushButton *landmarkManagerButton;
	//QCheckBox *landmarkLabelDispCheckBox;

    QPushButton *resetButton, *openFileNameButton, *whatsThisButton, *imgProcessButton, *imgV3DButton; //, *imgV3DROIButton;

	QHBoxLayout *allLayout;
	  QVBoxLayout *dataGroupLayout;
	    QGridLayout *xyzViewLayout;
	    QVBoxLayout *infoGroupLayout;
	  QVBoxLayout *mainGroupLayout;
	    QGridLayout *coordGroupLayout;
	    QGridLayout *scaleGroupLayout;
	    QGridLayout *typeGroupLayout;
	    QGridLayout *LandmarkGroupLayout;

	XFormView *xy_view; //change in Z
    XFormView *yz_view; //change in X
    XFormView *zx_view; //change in Y

	QTextBrowser *focusPointFeatureWidget;

    QMenu menuTriviewZoom;
	void createMenuOfTriviewZoom();

    QMenu menu3DViewer;
	void createMenuOf3DViewer();

public slots:
//    void changeColorType(ImageDisplayColorType c);


    void setColorRedType();
    void setColorGreenType();
    void setColorBlueType();
    void setColorAllType();
    void setColorRed2GrayType();
    void setColorGreen2GrayType();
    void setColorBlue2GrayType();
    void setColorAll2GrayType();
	void setColorMapDispType();
	void switchMaskColormap();

    void toggleLinkFocusCheckBox();
    void toggleDisplayFocusCrossCheckBox();
	void toggleImgValScaleDisplay();
	void toggleLookingGlassCheckBox();

	void doMenuOfTriviewZoom();
    void triview_zoomin();
    void triview_zoomout();
	void triview_zoom1();
	void triview_zoom2();
	void triview_zoomhalf();
	void triview_setzoom(double z, bool b_multiply); //b_multiply determine is z is the target zoom, or the target zoom should be product of the current zoom multply z

	void popupImageProcessingDialog();
	void popupImageProcessingDialog(QString item);
	void doImage3DView();
	void doImage3DLocalMarkerView();
	void doImage3DLocalRoiView();
	void doImage3DLocalBBoxView(); //do not have arguments so that can be used as the slot of a timer signal
	void doImage3DView(bool tmp_b_use_512x512x256, int b_local=0, V3DLONG bbx0=-1, V3DLONG bbx1=-1, V3DLONG bby0=-1, V3DLONG bby1=-1, V3DLONG bbz0=-1, V3DLONG bbz1=-1); 
	void doMenuOf3DViewer();
	void aboutInfo();

    void setOpenFileName();
    void reset();

	//this should be called from external. When no cross-image communication is needed, should not use this.
	void changeFocusXToExternal(int c); //will call changeFocusToExternal(int,int,int)
	void changeFocusYToExternal(int c); //will call changeFocusToExternal(int,int,int)
	void changeFocusZToExternal(int c); //will call changeFocusToExternal(int,int,int)

	void toggleCheckBox_bSendSignalToExternal();
	void toggleCheckBox_bAcceptSignalFromExternal();

	void copyLandmarkToPublicBuffer();
	void pasteLandmarkFromPublicBuffer();
	void saveLandmarkToFile();
	void loadLandmarkFromFile();
	void openLandmarkManager();

	void toggleLandmarkLabelDisp() {bDispMarkerLabel = !bDispMarkerLabel; this->updateViews();}

	void launchAtlasViewer(int curTab=0);

	void cascadeWindows();
	void updateViews(); //090615: a convenient function to call my4dimage updateViews()


signals:
    void external_focusXChanged(int c);
    void external_focusYChanged(int c);
    void external_focusZChanged(int c);



#define __used_by_v3d_interface__
public:    // in mainwindow_interface.cpp
	bool transferImageData(Image4DSimple *img, unsigned char *a);
	QList<QPolygon> get3ViewROI();
	bool set3ViewROI(QList<QPolygon> & roi_list);

	void open3DWindow();
	void openROI3DWindow();
	void close3DWindow();
	void closeROI3DWindow();
	void pushObjectIn3DWindow();
	void pushImageIn3DWindow();
	int pushTimepointIn3DWindow(int timepoint);

	bool screenShot3DWindow(QString filename);
	bool screenShotROI3DWindow(QString filename);

	V3dR_GLWidget * getView3D();
	V3dR_GLWidget * getLocalView3D();

	// a few interface functions for external plugin use. prototyped by PHC. 2010-Dec-10
	
	virtual void getFocusLocation(V3DLONG & cx, V3DLONG & cy, V3DLONG & cz) const 
	{
		if (imgData)
		{
			cx = imgData->getFocusX()+1;
			cy = imgData->getFocusY()+1;
			cz = imgData->getFocusZ()+1;
		}
	}
	virtual void setFocusLocation(V3DLONG cx, V3DLONG cy, V3DLONG cz)   
	{
		if (imgData)
		{
			forceToChangeFocus(cx, cy, cz);
		}
	}
	
	virtual void getMinMax(double & minval, double & maxval)  const 
	{
	}
	virtual void setMinMax(double minval, double maxval)   
 	{
	}
	
	virtual void getTriViewColorDispType(int & mytype)  const 
	{
	}
	virtual void setTriViewColorDispType(int mytype)  
	{
	}
};


#endif /* XFORMWIDGET_H_ */
