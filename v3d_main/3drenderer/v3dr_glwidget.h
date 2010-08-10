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




/****************************************************************************
** by Hanchuan Peng
** 2008-08-09
** last update : Hanchuan Peng, 090220
** Last update: Hanchuan Peng, 090221, Do NOT add surface obj geometry dialog
****************************************************************************/

#ifndef V3DR_GLWIDGET_H
#define V3DR_GLWIDGET_H

#include "v3dr_common.h"
#include "renderer.h"


class Renderer;
class V3dR_MainWindow;
class V3dr_colormapDialog;
class V3dr_surfaceDialog;
//class SurfaceObjGeometryDialog;

class V3dR_GLWidget : public QGLWidget
{
    Q_OBJECT;
//	friend class V3dR_MainWindow; //090710 RZC: to delete renderer before ~V3dR_GLWidget()
//	friend class v3dr_surfaceDialog;
//	friend class v3dr_colormapDialog;

public:
    V3dR_GLWidget(iDrawExternalParameter* idep, QWidget* mainWindow=0, QString title="");
    ~V3dR_GLWidget();
    //void makeCurrent() {if (!_in_destructor) QGLWidget::makeCurrent();} //090605 RZC: to override invalid access in qgl_x11.cpp
    void deleteRenderer();  //090710 RZC: to delete renderer before ~V3dR_GLWidget()
    void createRenderer();  //090710 RZC: to create renderer at any time

    void hideTool();
    void showTool();
    void updateTool();
    void updateControl();
	void handleKeyPressEvent(QKeyEvent * event); //for hook to MainWindow
	void handleKeyReleaseEvent(QKeyEvent * event); //for hook to MainWindow

    iDrawExternalParameter* getiDrawExternalParameter() {return _idep;}
    QWidget * getMainWindow() {return mainwindow;}
	Renderer* getRenderer() {return renderer;}
	QString getDataTitle() {return data_title;}
	int getNumKeyHolding() {for(int i=1;i<=9; i++) if(_holding_num[i]) return i; return -1;}
	bool getStill() 		{return _still;}
	void setStill(bool b) 	{_still = b;} //change return type from bool to void, by PHC 2010-05-20.
    void clearColormapDialog() {colormapDlg = 0;}
    void clearSurfaceDialog()  {surfaceDlg = 0;}
    bool screenShot(QString filename);

protected:
	virtual void choiceRenderer();
	virtual void settingRenderer(); // for setting the default renderer state when initialize
	virtual void preparingRenderer();
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);
    virtual void paintGL();

    virtual void paintEvent(QPaintEvent *event);

    virtual void focusInEvent(QFocusEvent* e);
    virtual void focusOutEvent(QFocusEvent* e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
	virtual void mouseDoubleClickEvent ( QMouseEvent * event );

	virtual void keyPressEvent(QKeyEvent * event) {handleKeyPressEvent(event);}
	virtual void keyReleaseEvent(QKeyEvent * event) {handleKeyReleaseEvent(event);}

    virtual void closeEvent(QCloseEvent* e); //for self closing
    virtual bool event(QEvent* e);       //090427 RZC:  for QHelpEvent of ToolTip
    virtual void customEvent(QEvent* e); // for QEvent_OpenFiles, by RZC 081002

    iDrawExternalParameter* _idep;
    QWidget *mainwindow;
	Renderer* renderer;
    QString data_title;
	QString dropUrl;
    static V3dr_colormapDialog* colormapDlg;
    static V3dr_surfaceDialog*  surfaceDlg;
	//static SurfaceObjGeometryDialog *surfaceObjGeoDlg;

public:
	int renderMode() const { return _renderMode; }
	int dataDim1() { return _data_size[0]; }
	int dataDim2() { return _data_size[1]; }
	int dataDim3() { return _data_size[2]; }
	int dataDim4() { return _data_size[3]; }
	int dataDim5() { return _data_size[4]; }

	int xRotation() const { return _xRot; }
    int yRotation() const { return _yRot; }
    int zRotation() const { return _zRot; }
    int zoom() const { return _zoom; }
    int xShift() const { return _xShift; }
    int yShift() const { return _yShift; }
    int zShift() const { return _zShift; }
    bool isAbsoluteRot()	const { return _absRot; }

    bool isVolCompress() const { return (renderer)? renderer->tryTexCompress :false; }
    bool isShowBoundingBox() const { return (renderer)? renderer->bShowBoundingBox :false; }
    bool isShowAxes() 		const { return (renderer)? renderer->bShowAxes :false; }

    QString Cut_altTip(int dim_i, int v, int minv, int maxv, int offset);

public slots:
// most of format: set***(type) related to a change***(type)
	void stillPaint();

	int setVolumeTimePoint(int t);
	void incVolumeTimePoint(float step);

	void setRenderMode_Mip(bool b);
	void setRenderMode_Alpha(bool b);
	void setRenderMode_Cs3d(bool b);

	void setCSTransparent(int);
	void setThickness(double);
	void setCurChannel(int);

	void setChannelR(bool b);
	void setChannelG(bool b);
	void setChannelB(bool b);
	void setVolCompress(bool b);

	void volumeColormapDialog();
	void surfaceSelectDialog(int curTab=0); // 090505 RZC: add curTab
	void surfaceSelectTab(int curTab=0); // 090522 RZC: just switch to curTab, no creating
	void surfaceDialogHide(); //added 090220, by PHC for convenience
	void annotationDialog(int dataClass, int surfaceType, int index);

	void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
	void resetRotation(bool b_emit=true);
	void modelRotation(int xRotStep, int yRotStep, int zRotStep);
	void viewRotation(int xRotStep, int yRotStep, int zRotStep);
	void absoluteRotPose();
	void doAbsoluteRot();
//	void absXRotation(int angle);
//	void absYRotation(int angle);
//	void absZRotation(int angle);

    void setZoom(int r);
    void setXShift(int s);
	void setYShift(int s);
	void setZShift(int s);
	void resetZoomShift();

	void enableFrontSlice(bool);
	void enableXSlice(bool);
	void enableYSlice(bool);
	void enableZSlice(bool);

	void setFrontCut(int s);
    void setXCut0(int s);
	void setYCut0(int s);
	void setZCut0(int s);
    void setXCut1(int s);
	void setYCut1(int s);
	void setZCut1(int s);
    void setXCS(int s);
	void setYCS(int s);
	void setZCS(int s);
    void setXClip0(int s);
	void setYClip0(int s);
	void setZClip0(int s);
    void setXClip1(int s);
	void setYClip1(int s);
	void setZClip1(int s);

	void enableShowAxes(bool b);
	void enableShowBoundingBox(bool b);
	void enableOrthoView(bool b);
	void setBackgroundColor();
	void setBright();

	void setShowMarkers(int s);
	void setShowSurfObjects(int s);
	void enableMarkerLabel(bool);
	void setMarkerSize(int s);
	void enableSurfStretch(bool);
	void toggleCellName();

	void createSurfCurrentR();
	void createSurfCurrentG();
	void createSurfCurrentB();
	void loadObjectsFromFile();
	void loadObjectsFromFile(QString url);
	void loadObjectsListFromFile();
	void saveSurfFile();

	void togglePolygonMode();
	void toggleLineType();
	void toggleObjShader();

	void changeLineOption();
	void changeVolShadingOption();
	void changeObjShadingOption();

	void toggleTexFilter();
	void toggleTex2D3D();
	void toggleTexCompression();
	void toggleTexStream();
	void toggleShader();
	void showGLinfo();

	void updateWithTriView();
	void updateImageData();
	void reloadData();
	void cancelSelect();

signals:
// most of format: change***(type)
// most of format: changeEnable***(bool)
// most of format: signal***()

	void signalInitControlValue();
	void signalVolumeCutRange();

	void changeVolumeTimePoint(int);

	void changeDispType_mip(bool);
	void changeDispType_alpha(bool);
	void changeDispType_cs3d(bool);

	void changeVolCompress(bool);
	void changeEnableVolCompress(bool);
	void changeEnableVolColormap(bool);

	void changeTransparentSliderLabel(const QString&);
	void changeEnableTransparentSlider(bool);
	void changeMarkerSize(int);

	void xRotationChanged(int);
    void yRotationChanged(int);
    void zRotationChanged(int);

    void zoomChanged(int);
    void xShiftChanged(int);
    void yShiftChanged(int);
    void zShiftChanged(int);

	void changeEnableCut0Slider(bool);
    void changeEnableCut1Slider(bool);
    void changeCurrentTabCutPlane(int);
    void changeEnableTabCutPlane(int,bool);

    void changeXCSSlider(int s);
    void changeYCSSlider(int s);
    void changeZCSSlider(int s);
    void changeFrontCut(int s);

    void changeXCut0(int s);
	void changeXCut1(int s);
    void changeYCut0(int s);
	void changeYCut1(int s);
    void changeZCut0(int s);
	void changeZCut1(int s);

    void changeXClip0(int s);
	void changeXClip1(int s);
    void changeYClip0(int s);
	void changeYClip1(int s);
    void changeZClip0(int s);
	void changeZClip1(int s);

private:
	bool _still, _still_paint_off, _still_pending, _mouse_in_view;
    QTimer still_timer;
    static const int still_timer_interval = 1000;

    bool _in_destructor; //for makeCurrent when valid context
	bool _isSoftwareGL; //for choiceRenderer

	int _renderMode;
	//unsigned char * data;
	int _data_size[5];

	char tipBuf[1000];
	bool _holding_num[10];

	int viewW, viewH;
	GLdouble mRot[16];
	QPoint lastPos;

	int _xRot, _yRot, _zRot, dxRot, dyRot, dzRot;
	int _zoom, _xShift, _yShift, _zShift, dxShift, dyShift, dzShift;
	int _xCut0, _xCut1, _yCut0, _yCut1, _zCut0, _zCut1, _fCut;
	int _xCS, _yCS, _zCS;
	int _xClip0, _xClip1, _yClip0, _yClip1, _zClip0, _zClip1;
	int _CStransparency, _markerSize, _curChannel;
	float _thickness;
	int _Bright, _Contrast, sUpdate_bright, sUpdate_track;
	bool _showAxes, _showBoundingBox, _absRot, _orthoView;
	bool _volCompress, _volFilter;

	int _volumeTimePoint; float volumeTimPoint_fraction;

	void init_members()
	{
		_still = _still_paint_off = _still_pending = _mouse_in_view = false;
	    connect(&still_timer, SIGNAL(timeout()), this, SLOT(stillPaint())); //only connect once
	    still_timer.start(still_timer_interval);

	    _in_destructor =false;
		_isSoftwareGL =false;

		_renderMode = 0;
		for (int i=0; i<5; i++)	_data_size[i] = 0;

		for (int i=0; i<10; i++) _holding_num[i] = false;

		viewW=viewH=0;
		for (int i=0; i<4; i++)
			for (int j=0; j<4; j++)
				mRot[i*4 +j] = (i==j)? 1 : 0; // Identity matrix

		_xRot=_yRot=_zRot= dxRot=dyRot=dzRot=
		_zoom=_xShift=_yShift=_zShift= dxShift=dyShift=dzShift=
		_xCut0=_xCut1=_yCut0=_yCut1=_zCut0=_zCut1=_fCut=
		_xCS=_yCS=_zCS=
		_xClip0=_xClip1=_yClip0=_yClip1=_zClip0=_zClip1 =0;
		_CStransparency=0; _markerSize=1, _curChannel=1;
		_thickness =1;
		_Bright=_Contrast=sUpdate_bright=sUpdate_track=0;
		_showAxes = _showBoundingBox = _absRot = _orthoView =false;
		_volCompress = _volFilter =true;

		_volumeTimePoint=0; volumeTimPoint_fraction=0;
	}
};

#endif
