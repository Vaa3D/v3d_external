#include "Mozak3DView.h"
#include "MozakUI.h"
#include <math.h>
#include "renderer_gl2.h"
#include "v3d_application.h"
#include "../terafly/src/control/CVolume.h"
#include "../terafly/src/control/CImageUtils.h"
#include "../terafly/src/presentation/PAnoToolBar.h"

using namespace mozak;

int Mozak3DView::contrastValue = 0;

Mozak3DView::Mozak3DView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
		: teramanager::CViewer(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
			_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID)
{
	nextImg = 0;
	loadingNextImg = false;
	
	contrastSlider = new QScrollBar(Qt::Vertical);
	contrastSlider->setRange(-50, 50);
	contrastSlider->setSingleStep(1);
	contrastSlider->setPageStep(10);
	contrastSlider->setValue(contrastValue);
	
	QObject::connect(contrastSlider, SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(updateContrast(int)));
}

teramanager::CViewer* Mozak3DView::makeView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
{
	Mozak3DView* neuronView = new Mozak3DView(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
		_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID);
	return neuronView;
}

void Mozak3DView::onNeuronEdit()
{
	teramanager::CViewer::onNeuronEdit();
	teramanager::CViewer::storeAnnotations();
#ifdef MOZAK_AUTOSAVE
	MozakUI* moz = MozakUI::getMozakInstance();
	if (moz->annotationsPathLRU == "")
		moz->annotationsPathLRU = "./mozak.ano";
	moz->saveAnnotations();
#endif
	makeTracedNeuronsEditable();
}

void Mozak3DView::makeTracedNeuronsEditable()
{
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	int sz = curr_renderer->listNeuronTree.size();
	for (int i=0; i<sz; i++)
	{
		curr_renderer->listNeuronTree[i].editable = true;
	}
	curr_renderer->nodeSize = 5;
	curr_renderer->paint();
}

//find the nearest node in a neuron in XY project of the display window
int Mozak3DView::findNearestNeuronNode(int cx, int cy)
{
    Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    int best_ind=-1;
    int best_dist=-1;

    QList<NeuronTree>::iterator i;
    for (i = (curr_renderer->listNeuronTree).begin(); i != (curr_renderer->listNeuronTree.end()); ++i){
        QList <NeuronSWC> p_listneuron = i->listNeuron;

        GLdouble px, py, pz, ix, iy, iz;
        for (int i=0; i<p_listneuron.size(); i++)
        {
            ix = p_listneuron.at(i).x, iy = p_listneuron.at(i).y, iz = p_listneuron.at(i).z;
            GLint res = gluProject(ix, iy, iz, curr_renderer->markerViewMatrix, curr_renderer->projectionMatrix, curr_renderer->viewport, &px, &py, &pz);// note: should use the saved modelview,projection and viewport matrix
            py = curr_renderer->viewport[3]-py; //the Y axis is reversed
            if (res==GL_FALSE) {qDebug()<<"gluProject() fails for NeuronTree ["<<i<<"] node"; return -1;}
            double cur_dist = (px-cx)*(px-cx)+(py-cy)*(py-cy);
            if (i==0) {	best_dist = cur_dist; best_ind=0; }
            else {	if (cur_dist < best_dist) {best_dist=cur_dist; best_ind = i;}}
        }
    }

    return best_ind; //by PHC, 090209. return the index in the SWC file
}

bool Mozak3DView::eventFilter(QObject *object, QEvent *event)
{
	if(!isReady)
		return false;
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    Renderer::SelectMode currentMode = curr_renderer->selectMode;
    view3DWidget->setMouseTracking(true);

    if (event->type() == QEvent::MouseMove)
    {
        //On mouse move, if one of the extend mode is enabled, then update nodes to be highlighted
        QMouseEvent *k = (QMouseEvent *)event;
        int isRightMouseDown = k->buttons() & Qt::RightButton; //
        if( (currentMode == Renderer::smCurveEditExtendOneNode || currentMode == Renderer::smCurveEditExtendTwoNode)){
            if(!isRightMouseDown){
                //Highlight start node
                curr_renderer->highlightedNode = findNearestNeuronNode(k->x(), k->y());
            }else if(currentMode == Renderer::smCurveEditExtendTwoNode){
                //Highlight end node
                curr_renderer->highlightedEndNode = findNearestNeuronNode(k->x(), k->y());
            }

            curr_renderer->drawNeuronTreeList();
            curr_renderer->drawObj();
            ((QWidget *)(curr_renderer->widget))->repaint(); //Update the screen position of highlighted nodes
        }
    }

    if (event->type() == QEvent::MouseButtonRelease)
    {
        //This is a very unfortunate workaround to solve an issue where the cursor move calls
        //stop happening at times even when setMouseTracking is enabled.
        view3DWidget->setMouseTracking(false);
    }

    QKeyEvent* key_evt;
	QMouseEvent* mouseEvt;
	bool neuronTreeChanged = false;
	if (((object == view3DWidget || object == window3D) && event->type() == QEvent::Wheel))
	{

		int prevZoom = view3DWidget->zoom();
		
		QWheelEvent* wheelEvt = (QWheelEvent*)event;
		lastWheelFocus = getRenderer3DPoint(wheelEvt->x(), wheelEvt->y());
		useLastWheelFocus = true;
		
		// Calculate amount that the point being mouse over moves on the sceen
		// in order to offset this and keep that point centered after zoom
		GLdouble p0x, p0y, p0z, p1x, p1y, p1z, ix, iy, iz;
		
		ix = lastWheelFocus.x;
        iy = lastWheelFocus.y;
        iz = lastWheelFocus.z;
		
		// Project the moused-over 3D point to the screen coordinates
		gluProject(ix, iy, iz, curr_renderer->markerViewMatrix, curr_renderer->projectionMatrix, curr_renderer->viewport, &p0x, &p0y, &p0z);
		p0y = curr_renderer->viewport[3]-p0y; //the Y axis is reversed
		
		// Handle this ourselves rather than passing to Terafly (which would change resolutions)
		//processWheelEvt(wheelEvt);

		float d = (wheelEvt->delta())/100;  // ~480
		#define MOUSE_ZOOM(dz)    (int(dz*4* MOUSE_SENSITIVE));
		#define MOUSE_ZROT(dz)    (int(dz*8* MOUSE_SENSITIVE));

		int zoomStep = MOUSE_ZOOM(d);
		int newZoom = zoomStep + prevZoom; // wheeling up should zoom IN, not out
		int wheelX = wheelEvt->x();
		int wheelY = wheelEvt->y();
		float prevZoomRatio = curr_renderer->zoomRatio;
		
		// Change zoom
		view3DWidget->setZoom(newZoom);
		curr_renderer->paint(); // updates the projection matrix

		// Project the previously moused-over 3D point to its new screen coordinates (after zoom)
		gluProject(ix, iy, iz, curr_renderer->markerViewMatrix, curr_renderer->projectionMatrix, curr_renderer->viewport, &p1x, &p1y, &p1z);
		p1y = curr_renderer->viewport[3]-p1y; //the Y axis is reversed
		
		float newZoomRatio = curr_renderer->zoomRatio;
		float viewW = float(view3DWidget->viewW);
		float viewH = float(view3DWidget->viewH);;
		float dxPctOfScreen = float(p0x-p1x) / viewW;
		float dyPctOfScreen = float(p1y-p0y) / viewH;
		
		float screenWidthInXShifts  = 2.0f * float(SHIFT_RANGE) * newZoomRatio;
		float screenHeightInYShifts = 2.0f * float(SHIFT_RANGE) * newZoomRatio;
		
		//view3DWidget->setXShift(view3DWidget->xShift() + dxPctOfScreen*screenWidthInXShifts);
		//view3DWidget->setYShift(view3DWidget->yShift() + dyPctOfScreen*screenHeightInYShifts);

		if (!loadingNextImg && (newZoom < prevZoom) && (newZoom <= 0)) // zooming out
		{
			// Previously, we were lowering the resolution on zoomout, now just perform zoom
		}
		return true;
	}
	else if (object == view3DWidget && event->type() == QEvent::MouseButtonDblClick) //
	{
		QMouseEvent* mouseEvt = (QMouseEvent*)event;
		// Right mouse double click reduces resolution
		if (mouseEvt->button() == Qt::RightButton)
		{
			// Determine whether we need to go to a previous (lower) resolution when zooming out
			CViewInfo* prevView = 0;
			while (lowerResViews.length() > 0 && !prevView)
				prevView = lowerResViews.takeLast();
			if (prevView)
			{
				isReady = false;
				loadingNextImg = true;
				window3D->setCursor(Qt::BusyCursor);
				view3DWidget->setCursor(Qt::BusyCursor);
				loadNewResolutionData(prevView->resIndex, prevView->img,
					prevView->volV0, prevView->volV1,prevView->volH0, prevView->volH1, 
					prevView->volD0, prevView->volD1, prevView->volT0, prevView->volT1);
				prevView->img->setRawDataPointerToNull(); // this image is in use now, so remove pointer before deletion
				delete prevView;
				
				window3D->setCursor(Qt::ArrowCursor);
				view3DWidget->setCursor(Qt::ArrowCursor);
				loadingNextImg = false;
				isReady = true;
				return true;
			}
			return false;
		}
		else
		{
			// Left double clicks (zoom in resolution) handled by CViewer 
			return teramanager::CViewer::eventFilter(object, event);
		}
	}
	else if (event->type() == QEvent::KeyPress) // intercept keypress events
	{
		key_evt = (QKeyEvent*)event;
		if (key_evt->isAutoRepeat()) return true; // ignore holding down of key
		// Implement custom key events
        int keyPressed = key_evt->key();
		Renderer::SelectMode newMode;
        bool bAddCurve = true;

		switch (keyPressed)
		{
			case Qt::Key_D:
				if (!deleteSegmentsButton->isChecked())
					deleteSegmentsButton->setChecked(true);
				changeMode(Renderer::smDeleteMultiNeurons, true, true);
				break;
			case Qt::Key_S:
				if (!splitSegmentButton->isChecked())
					splitSegmentButton->setChecked(true);
				changeMode(Renderer::smBreakMultiNeurons, false, true);
                break;
			case Qt::Key_P:
				if (!polyLineButton->isChecked())
					polyLineButton->setChecked(true);
				changeMode(Renderer::smCurveCreate_pointclick, true, true);
                break;
            case Qt::Key_E:
                //This is a very unfortunate workaround to solve an issue where the cursor move calls
                //stop happening at times even when setMouseTracking is enabled.
                view3DWidget->setMouseTracking(false);
                if (!extendButton->isChecked())
					extendButton->setChecked(true);
				changeMode(Renderer::smCurveEditExtendOneNode, true, true);
                break;
            case Qt::Key_C:
                //This is a very unfortunate workaround to solve an issue where the cursor move calls
                //stop happening at times even when setMouseTracking is enabled.
                view3DWidget->setMouseTracking(false);
				if (!connectButton->isChecked())
					connectButton->setChecked(true);
				changeMode(Renderer::smCurveEditExtendTwoNode, true, true);
                break;
			default:
#ifdef FORCE_BBOX_MODE
				changeMode(Renderer::smCurveTiltedBB_fm_sbbox, true, true);
#endif
				break;
		}
	}
	else if (event->type() == QEvent::KeyRelease) // intercept keypress events
	{
		key_evt = (QKeyEvent*)event;
		if (key_evt->isAutoRepeat()) return true; // ignore holding down of key
#ifdef FORCE_BBOX_MODE
		if (curr_renderer->selectMode != Renderer::smCurveTiltedBB_fm_sbbox)
		{
			changeMode(Renderer::smCurveTiltedBB_fm_sbbox, true, true);
		}
#endif
	}
	else
	{
#ifdef FORCE_BBOX_MODE
		// If beginning right click drag, ensure that default mode is chosen before beginning
		if (object == view3DWidget && event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvt = (QMouseEvent*)event;
			if (curr_renderer->selectMode == Renderer::smObject)
			{
				changeMode(Renderer::smCurveTiltedBB_fm_sbbox, true, true);
			}
		}
		else
#endif
		if (object == view3DWidget && event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent* mouseEvt = (QMouseEvent*)event;
			if (mouseEvt->button() == Qt::RightButton)
			{
				//qDebug() << itm::strprintf("Right click ended, current render selectMode mode: %d", curr_renderer->selectMode).c_str();
				if (curr_renderer->selectMode == Renderer::smDeleteMultiNeurons)
					curr_renderer->deleteMultiNeuronsByStrokeCommit();
				// Regardless of function performed, when right mouse button is released save the annotaions file
				neuronTreeChanged = true;
			}
		}
		bool res = teramanager::CViewer::eventFilter(object, event);
		if (neuronTreeChanged)
			onNeuronEdit();
		return res;
	}
	return false;
}

void Mozak3DView::show()
{
	this->title = "Mozak";
	teramanager::CViewer::show();
	
	// Hide unwanted buttons
	itm::PAnoToolBar::instance()->buttonMarkerCreate->setParent(0);
	itm::PAnoToolBar::instance()->buttonMarkerCreate2->setParent(0);
    itm::PAnoToolBar::instance()->buttonMarkerDelete->setParent(0);
    itm::PAnoToolBar::instance()->buttonMarkerRoiDelete->setParent(0);
	itm::PAnoToolBar::instance()->buttonMarkerRoiView->setParent(0);

	window3D->centralLayout->addWidget(contrastSlider, 1);
	

	connectButton = new QToolButton();
	connectButton->setIcon(QIcon(":/mozak/icons/connect.png"));
    connectButton->setToolTip("Connect nodes using right click stroke");
    connectButton->setCheckable(true);
    connect(connectButton, SIGNAL(toggled(bool)), this, SLOT(connectButtonToggled(bool)));

	extendButton = new QToolButton();
	extendButton->setIcon(QIcon(":/mozak/icons/extend.png"));
    extendButton->setToolTip("Extend from existing node using right click stroke");
    extendButton->setCheckable(true);
    connect(extendButton, SIGNAL(toggled(bool)), this, SLOT(extendButtonToggled(bool)));

	polyLineButton = new QToolButton();
	polyLineButton->setIcon(QIcon(":/mozak/icons/polyline.png"));
    polyLineButton->setToolTip("Series of right-clicks to define a 3D curve (Esc to finish)");
    polyLineButton->setCheckable(true);
    //polyLineButton->setShortcut(QKeySequence("Ctrl+P"));
    connect(polyLineButton, SIGNAL(toggled(bool)), this, SLOT(polyLineButtonToggled(bool)));

	splitSegmentButton = new QToolButton();
	splitSegmentButton->setIcon(QIcon(":/mozak/icons/split.png"));
    splitSegmentButton->setToolTip("Split segment into two using right click stroke");
    splitSegmentButton->setCheckable(true);
    connect(splitSegmentButton, SIGNAL(toggled(bool)), this, SLOT(splitSegmentButtonToggled(bool)));

	deleteSegmentsButton = new QToolButton();
	deleteSegmentsButton->setIcon(QIcon(":/mozak/icons/delete-segments.png"));
    deleteSegmentsButton->setToolTip("Delete multiple segments with right click stroke");
    deleteSegmentsButton->setCheckable(true);
    connect(deleteSegmentsButton, SIGNAL(toggled(bool)), this, SLOT(deleteSegmentsButtonToggled(bool)));

	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, extendButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, connectButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, polyLineButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, splitSegmentButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, deleteSegmentsButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->refreshTools();

	updateRendererTextureParams();
}

void Mozak3DView::updateRendererTextureParams()
{
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	curr_renderer->tryTexCompress = false;
	curr_renderer->tryTexStream = -1;
	curr_renderer->tryTexNPT = true;
	//view3DWidget->changeVolShadingOption();
}

void Mozak3DView::updateContrast(int con) /* contrast from -100 (bright) to 100 (dark) */
{
	// This performs the same functionality as colormap Red->Gray and then
	// adjusting the alpha, but with only one parameter to adjust
	contrastValue = con;
	float exp_val = pow(10.0f, con/25.0f); // map from -100->100 to 1000->0.001
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	for(int j=0; j<255; j++)
	{
		(curr_renderer->colormap[0][j]).r = (unsigned char)255;
		(curr_renderer->colormap[0][j]).g = (unsigned char)255;
		(curr_renderer->colormap[0][j]).b = (unsigned char)255;
		// This is the value being manipulated
		int val = (int)(pow(j/255.0f, exp_val) * 255.0f);
		(curr_renderer->colormap[0][j]).a = (unsigned char)val;
		
		(curr_renderer->colormap[1][j]).r = (unsigned char)0;
		(curr_renderer->colormap[1][j]).g = (unsigned char)255;
		(curr_renderer->colormap[1][j]).b = (unsigned char)0;
		(curr_renderer->colormap[1][j]).a = (unsigned char)0;
		
		(curr_renderer->colormap[2][j]).r = (unsigned char)0;
		(curr_renderer->colormap[2][j]).g = (unsigned char)0;
		(curr_renderer->colormap[2][j]).b = (unsigned char)255;
		(curr_renderer->colormap[2][j]).a = (unsigned char)0;
		
		(curr_renderer->colormap[3][j]).r = (unsigned char)205;
		(curr_renderer->colormap[3][j]).g = (unsigned char)205;
		(curr_renderer->colormap[3][j]).b = (unsigned char)205;
		(curr_renderer->colormap[3][j]).a = (unsigned char)205;
	}
	for (int ch=0; ch<3; ch++)
	{
		for (int j=0; j<4; j++) // RGBA
		{
			curr_renderer->colormap_curve[ch][j].clear();
			int y;
			y = curr_renderer->colormap[ch][0].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  0.0,  y);
			y = curr_renderer->colormap[ch][255].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  1.0,  y);
		}
	}
	view3DWidget->update();
}

void Mozak3DView::connectButtonToggled(bool checked)
{
	changeMode(Renderer::smCurveEditExtendTwoNode, true, checked);
}

void Mozak3DView::extendButtonToggled(bool checked)
{
	changeMode(Renderer::smCurveEditExtendOneNode, true, checked);
}

void Mozak3DView::polyLineButtonToggled(bool checked)
{
	changeMode(Renderer::smCurveCreate_pointclick, true, checked);
}

void Mozak3DView::splitSegmentButtonToggled(bool checked)
{
	changeMode(Renderer::smBreakMultiNeurons, false, checked);
}

void Mozak3DView::deleteSegmentsButtonToggled(bool checked)
{
	changeMode(Renderer::smDeleteMultiNeurons, true, checked);
}

void Mozak3DView::changeMode(Renderer::SelectMode mode, bool addThisCurve, bool turnOn)
{
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	curr_renderer->endSelectMode();
	if (turnOn)
	{
		curr_renderer->selectMode = mode;
		curr_renderer->b_addthiscurve = addThisCurve;
		// Uncheck any other currently checked modes
		if (mode != Renderer::smCurveEditExtendOneNode && extendButton->isChecked())
			extendButton->setChecked(false);
		if (mode != Renderer::smCurveEditExtendTwoNode && connectButton->isChecked())
			connectButton->setChecked(false);
		if (mode != Renderer::smCurveCreate_pointclick && polyLineButton->isChecked())
			polyLineButton->setChecked(false);
		if (mode != Renderer::smBreakMultiNeurons && splitSegmentButton->isChecked())
			splitSegmentButton->setChecked(false);
		if (mode != Renderer::smDeleteMultiNeurons && deleteSegmentsButton->isChecked())
			deleteSegmentsButton->setChecked(false);
	}
	else
	{
		onNeuronEdit();
#ifdef FORCE_BBOX_MODE
		curr_renderer->selectMode = Renderer::smCurveTiltedBB_fm_sbbox;
		curr_renderer->b_addthiscurve = true;
#endif
	}
}

/*********************************************************************************
* Receive data (and metadata) from <CVolume> throughout the loading process
**********************************************************************************/
void Mozak3DView::receiveData(
        itm::uint8* data,                               // data (any dimension)
        itm::integer_array data_s,                           // data start coordinates along X, Y, Z, C, t
        itm::integer_array data_c,                           // data count along X, Y, Z, C, t
        QWidget *dest,                                  // address of the listener
        bool finished,                                  // whether the loading operation is terminated
        itm::RuntimeException* ex /* = 0*/,             // exception (optional)
        qint64 elapsed_time       /* = 0 */,            // elapsed time (optional)
        QString op_dsc            /* = ""*/,            // operation descriptor (optional)
        int step                  /* = 0 */)            // step number (optional)
{
    /*itm::debug(itm::LEV1, strprintf("title = %s, data_s = {%s}, data_c = {%s}, finished = %s",
                                        titleShort.c_str(),
                                        !data_s.empty() ? strprintf("%d,%d,%d,%d,%d", data_s[0], data_s[1], data_s[2], data_s[3], data_s[4]).c_str() : "",
                                        !data_c.empty() ? strprintf("%d,%d,%d,%d,%d", data_c[0], data_c[1], data_c[2], data_c[3], data_c[4]).c_str() : "",
                                        finished ? "true" : "false").c_str(), __itm__current__function__);*/

    char message[1000];
    itm::CVolume* cVolume = itm::CVolume::instance();
	
	int nextImgVolV0 = cVolume->getVoiV0();
	int nextImgVolV1 = cVolume->getVoiV1();
	int nextImgVolH0 = cVolume->getVoiH0();
	int nextImgVolH1 = cVolume->getVoiH1();
	int nextImgVolD0 = cVolume->getVoiD0();
	int nextImgVolD1 = cVolume->getVoiD1();
	int nextImgVolT0 = cVolume->getVoiT0();
	int nextImgVolT1 = cVolume->getVoiT1();

    //if an exception has occurred, showing a message error
    if(ex)
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    else if(dest == this)
    {
        try
        {
            itm::uint32 img_dims[5]       = {std::max(0,nextImgVolH1-nextImgVolH0), std::max(0,nextImgVolV1-nextImgVolV0), std::max(0,nextImgVolD1-nextImgVolD0), nchannels, std::max(0,nextImgVolT1-nextImgVolT0+1)};
			itm::uint32 img_offset[5]     = {std::max(0,data_s[0]-nextImgVolH0),    std::max(0,data_s[1]-nextImgVolV0),    std::max(0,data_s[2]-nextImgVolD0),    0,         std::max(0,data_s[4]-nextImgVolT0)     };
            itm::uint32 new_img_dims[5]   = {data_c[0],                             data_c[1],                             data_c[2],                             data_c[3], data_c[4]                              };
            itm::uint32 new_img_offset[5] = {0,                                     0,                                     0,                                     0,         0                                      };
            itm::uint32 new_img_count[5]  = {data_c[0],                             data_c[1],                             data_c[2],                             data_c[3], data_c[4]                              };
			itm::CImageUtils::copyVOI(data, new_img_dims, new_img_offset, new_img_count,
                    nextImg->getRawData(), img_dims, img_offset);
            
            // release memory
            delete[] data;

            // if 5D data, update selected time frame
            if(itm::CImport::instance()->is5D())
                view3DWidget->setVolumeTimePoint(window3D->timeSlider->value()-nextImgVolT0);
			
			// operations to be performed when all image data have been loaded
            if(finished)
            {
                // disconnect from data producer
                disconnect(cVolume, SIGNAL(sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)), this, SLOT(receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)));

				// store the previous image4D data
				Image4DSimple* prevImg = new Image4DSimple();
				prevImg->setFileName(title.c_str());
				
				prevImg->setData(imgData,
								 view3DWidget->getiDrawExternalParameter()->image4d->getXDim(),
								 view3DWidget->getiDrawExternalParameter()->image4d->getYDim(),
								 view3DWidget->getiDrawExternalParameter()->image4d->getZDim(),
								 view3DWidget->getiDrawExternalParameter()->image4d->getCDim(),
								 view3DWidget->getiDrawExternalParameter()->image4d->getDatatype());
				prevImg->setTDim(view3DWidget->getiDrawExternalParameter()->image4d->getTDim());
				prevImg->setTimePackType(view3DWidget->getiDrawExternalParameter()->image4d->getTimePackType());
				prevImg->setRezX(view3DWidget->getiDrawExternalParameter()->image4d->getRezX());
				prevImg->setRezY(view3DWidget->getiDrawExternalParameter()->image4d->getRezY());
				prevImg->setRezZ(view3DWidget->getiDrawExternalParameter()->image4d->getRezZ());
				prevImg->setOriginX(view3DWidget->getiDrawExternalParameter()->image4d->getOriginX());
				prevImg->setOriginY(view3DWidget->getiDrawExternalParameter()->image4d->getOriginY());
				prevImg->setOriginZ(view3DWidget->getiDrawExternalParameter()->image4d->getOriginZ());
				prevImg->setValidZSliceNum(view3DWidget->getiDrawExternalParameter()->image4d->getValidZSliceNum());
				CViewInfo* prevView = new CViewInfo(volResIndex, prevImg, volV0, volV1, volH0, volH1, volD0, volD1, volT0, volT1, nchannels, slidingViewerBlockID, view3DWidget->zoom());
				
				// If there are higher res images, discard them here
				while(lowerResViews.length() > volResIndex)
				{
					CViewInfo* viewToDispose = lowerResViews.takeLast();
					delete viewToDispose;
				}
				// If we skipped one/more resolutions, store as blank pointers
				while(lowerResViews.length() < volResIndex) lowerResViews.append(0);
				// Store prev view such that lowerResViews[volResIndex] = prev view info
				lowerResViews.append(prevView);

				// Remove the reference to this prevView's data, otherwise it will be cleaned up
				// in the next step
				view3DWidget->getiDrawExternalParameter()->image4d->setRawDataPointerToNull();
				
				// Load the new data and view parameters into this window (this used to be done in separate windows)
				loadNewResolutionData(cVolume->getVoiResIndex(), nextImg, nextImgVolV0, nextImgVolV1, nextImgVolH0, nextImgVolH1, nextImgVolD0, nextImgVolD1, nextImgVolT0, nextImgVolT1);
				
				nextImg->setRawDataPointerToNull(); // prevent underlying image data from being deleted
				delete nextImg;

				// reset the cursor
                window3D->setCursor(Qt::ArrowCursor);
                view3DWidget->setCursor(Qt::ArrowCursor);

				//current window is now ready for user input
                isReady = true;
				loadingNextImg = false;
            }
        }
		catch(itm::RuntimeException &ex)
        {
			QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
			isReady = true;
			loadingNextImg = false;
        }
    }
}

/**********************************************************************************
* Loads the new resolution data into this window, this used to be done with 
* separate CViewer windows
***********************************************************************************/
void Mozak3DView::loadNewResolutionData(	int _resIndex,
												Image4DSimple *_img,
												int _volV0, int _volV1,
												int _volH0, int _volH1,
												int _volD0, int _volD1,
												int _volT0, int _volT1	) {
	qDebug() << itm::strprintf("\n  Loading new resolution data resolution=%d volV0=%d volV1=%d volH0=%d volH1=%d volD0=%d volD1=%d", _resIndex, _volV0, _volV1, _volH0, _volH1, _volD0, _volD1).c_str();
	
	itm::CViewer::storeAnnotations();

	// Update viewer's data now that new res is loaded
	volV0 = _volV0;
	volV1 = _volV1;				
	volH0 = _volH0;
	volH1 = _volH1;
	volD0 = _volD0;
	volD1 = _volD1;
	volT0 = _volT0;
	volT1 = _volT1;
	
	// Now update renderer's current image to reflect the newly loaded data
	int prevRes = volResIndex;
	volResIndex = _resIndex;
	imgData = _img->getRawData();

	// exit from "waiting for 5D data" state, if previously set
	if(this->waitingFor5D)
		this->setWaitingFor5D(false);

	V3D_env->setImage(window, _img); // this clears the rawDataPointer for _img

	// create 3D view window with postponed show()
	XFormWidget *w = V3dApplication::getMainWindow()->validateImageWindow(window);
	view3DWidget->getiDrawExternalParameter()->image4d = w->getImageData();
	// Make sure to call updateImageData AFTER getiDrawExternalParameter's image4d is
	// set above as this is the data being updated.
	view3DWidget->updateImageData();
	itm::CViewer::loadAnnotations();
	makeTracedNeuronsEditable();

	float ratio = itm::CImport::instance()->getVolume(volResIndex)->getDIM_D()/itm::CImport::instance()->getVolume(prevRes)->getDIM_D();
	float curZoom = (float)view3DWidget->zoom();
	if (ratio > 0.0f)
		view3DWidget->setZoom(curZoom/ratio);

	MozakUI* moz = MozakUI::getMozakInstance();

	//selecting the current resolution in the PMain GUI and disabling previous resolutions
    moz->resolution_cbox->setCurrentIndex(volResIndex);
    for(int i=0; i<moz->resolution_cbox->count(); i++)
    {
        // Get the index of the value to disable
        QModelIndex index = moz->resolution_cbox->model()->index(i,0);
		// These are the effective 'disable/enable' flags
        QVariant v1(Qt::NoItemFlags);
        QVariant v2(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        //the magic
        if(i<volResIndex)
            moz->resolution_cbox->model()->setData(index, v1, Qt::UserRole -1);
        else
            moz->resolution_cbox->model()->setData(index, v2, Qt::UserRole -1);
    }
    moz->gradientBar->setStep(volResIndex);
    moz->gradientBar->update();

	//setting min, max and value of PMain GUI VOI's widgets
    moz->V0_sbox->setMinimum(getGlobalVCoord(view3DWidget->yCut0(), -1, true, false, __itm__current__function__)+1);
    moz->V0_sbox->setValue(moz->V0_sbox->minimum());
    moz->V1_sbox->setMaximum(getGlobalVCoord(view3DWidget->yCut1(), -1, true, false, __itm__current__function__)+1);
    moz->V1_sbox->setValue(moz->V1_sbox->maximum());
    moz->H0_sbox->setMinimum(getGlobalHCoord(view3DWidget->xCut0(), -1, true, false, __itm__current__function__)+1);
    moz->H0_sbox->setValue(moz->H0_sbox->minimum());
    moz->H1_sbox->setMaximum(getGlobalHCoord(view3DWidget->xCut1(), -1, true, false, __itm__current__function__)+1);
    moz->H1_sbox->setValue(moz->H1_sbox->maximum());
    moz->D0_sbox->setMinimum(getGlobalDCoord(view3DWidget->zCut0(), -1, true, false, __itm__current__function__)+1);
    moz->D0_sbox->setValue(moz->D0_sbox->minimum());
    moz->D1_sbox->setMaximum(getGlobalDCoord(view3DWidget->zCut1(), -1, true, false, __itm__current__function__)+1);
    moz->D1_sbox->setValue(moz->D1_sbox->maximum());

    if(moz->frameCoord->isEnabled())
    {
        moz->T0_sbox->setText(QString::number(volT0+1));
        moz->T1_sbox->setText(QString::number(volT1+1));
    }

	// updating reference system
	if(!moz->isPRactive())
		moz->refSys->setDims(volH1-volH0+1, volV1-volV0+1, volD1-volD0+1);
	this->view3DWidget->updateGL();     // if omitted, Vaa3D_rotationchanged somehow resets rotation to 0,0,0
	Vaa3D_rotationchanged(0);

	// update curve aspect
	moz->curveAspectChanged();

	updateRendererTextureParams();
}

/**********************************************************************************
* Generates a new view using the given coordinates.
* Called by the current <CViewer> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void Mozak3DView::newViewer(int x, int y, int z,//can be either the VOI's center (default) or the VOI's ending point (see x0,y0,z0)
    int resolution,                                  //resolution index of the view requested
    int t0, int t1,                                  //time frames selection
    bool fromVaa3Dcoordinates /*= false*/,           //if coordinates were obtained from Vaa3D
    int dx/*=-1*/, int dy/*=-1*/, int dz/*=-1*/,     //VOI [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz), [t0, t1]
    int x0/*=-1*/, int y0/*=-1*/, int z0/*=-1*/,     //VOI [x0, x), [y0, y), [z0, z), [t0, t1]
    bool auto_crop /* = true */,                     //whether to crop the VOI to the max dims
    bool scale_coords /* = true */,                  //whether to scale VOI coords to the target res
    int sliding_viewer_block_ID /* = -1 */)          //block ID in "Sliding viewer" mode

{
	//qDebug() << itm::strprintf("title = %s, x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d, x0 = %d, y0 = %d, z0 = %d, t0 = %d, t1 = %d, auto_crop = %s, scale_coords = %s, sliding_viewer_block_ID = %d",
	//							titleShort.c_str(),  x, y, z, resolution, dx, dy, dz, x0, y0, z0, t0, t1, auto_crop ? "true" : "false", scale_coords ? "true" : "false", sliding_viewer_block_ID).c_str();
    // check precondition #2: valid resolution
    if(resolution >= itm::CImport::instance()->getResolutions())
        resolution = volResIndex;

    if(toBeClosed || loadingNextImg)
        return;
	
	loadingNextImg = true;

    try
    {
        // set GUI to waiting state
        MozakUI* moz = MozakUI::getMozakInstance();
        moz->progressBar->setEnabled(true);
        moz->progressBar->setMinimum(0);
        moz->progressBar->setMaximum(0);
        moz->statusBar->showMessage("Switching resolution...");
        view3DWidget->setCursor(Qt::BusyCursor);
        window3D->setCursor(Qt::BusyCursor);
        moz->setCursor(Qt::BusyCursor);

        // scale VOI coordinates to the reference system of the target resolution
        if(scale_coords)
        {
            float ratioX = static_cast<float>(itm::CImport::instance()->getVolume(resolution)->getDIM_H())/itm::CImport::instance()->getVolume(volResIndex)->getDIM_H();
            float ratioY = static_cast<float>(itm::CImport::instance()->getVolume(resolution)->getDIM_V())/itm::CImport::instance()->getVolume(volResIndex)->getDIM_V();
            float ratioZ = static_cast<float>(itm::CImport::instance()->getVolume(resolution)->getDIM_D())/itm::CImport::instance()->getVolume(volResIndex)->getDIM_D();
            // NOTE! The following functions use the current values for volResIndex and volH0, volV0, volD0, etc
			x = getGlobalHCoord(x, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            y = getGlobalVCoord(y, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            z = getGlobalDCoord(z, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            if(x0 != -1)
                x0 = getGlobalHCoord(x0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            else
                dx = dx == -1 ? itm::int_inf : static_cast<int>(dx*ratioX+0.5f);
            if(y0 != -1)
                y0 = getGlobalVCoord(y0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            else
                dy = dy == -1 ? itm::int_inf : static_cast<int>(dy*ratioY+0.5f);
            if(z0 != -1)
                z0 = getGlobalDCoord(z0, resolution, fromVaa3Dcoordinates, false, __itm__current__function__);
            else
                dz = dz == -1 ? itm::int_inf : static_cast<int>(dz*ratioZ+0.5f);
        }
		//qDebug() << itm::strprintf("\n  After scaling:\ntitle = %s, x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d, x0 = %d, y0 = %d, z0 = %d, t0 = %d, t1 = %d, auto_crop = %s, scale_coords = %s, sliding_viewer_block_ID = %d",
		//							titleShort.c_str(),  x, y, z, resolution, dx, dy, dz, x0, y0, z0, t0, t1, auto_crop ? "true" : "false", scale_coords ? "true" : "false", sliding_viewer_block_ID).c_str();

        // adjust time size so as to use all the available frames set by the user
        if(itm::CImport::instance()->is5D() && ((t1 - t0 +1) != moz->Tdim_sbox->value()))
        {
            t1 = t0 + (moz->Tdim_sbox->value()-1);
            /**/itm::debug(itm::LEV1, itm::strprintf("mismatch between |[t0,t1]| (%d) and max T dims (%d), adjusting it to [%d,%d]", t1-t0+1, moz->Tdim_sbox->value(), t0, t1).c_str(), __itm__current__function__);
        }


        // crop VOI if its larger than the maximum allowed
        if(auto_crop)
        {
            // modality #1: VOI = [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz), [t0, t1]
            if(dx != -1 && dy != -1 && dz != -1)
			{
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, cropping bbox dims from (%d,%d,%d) t[%d,%d] to...", titleShort.c_str(),  dx, dy, dz, t0, t1).c_str(), __itm__current__function__);
				dx = std::min(dx, itm::round(moz->Hdim_sbox->value()/2.0f));
                dy = std::min(dy, itm::round(moz->Vdim_sbox->value()/2.0f));
                dz = std::min(dz, itm::round(moz->Ddim_sbox->value()/2.0f));
                t0 = std::max(0, std::min(t0,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                if(itm::CImport::instance()->is5D() && (t1-t0+1 > moz->Tdim_sbox->value()))
                    t1 = t0 + moz->Tdim_sbox->value();
                if(itm::CImport::instance()->is5D() && (t1 >= itm::CImport::instance()->getTDim()-1))
                    t0 = t1 - (moz->Tdim_sbox->value()-1);
                if(itm::CImport::instance()->is5D() && (t0 == 0))
                    t1 = moz->Tdim_sbox->value()-1;
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, ...to (%d,%d,%d)", titleShort.c_str(),  dx, dy, dz).c_str(), __itm__current__function__);
            }
            // modality #2: VOI = [x0, x), [y0, y), [z0, z), [t0, t1]
            else
            {
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, cropping bbox dims from [%d,%d) [%d,%d) [%d,%d) [%d,%d] to...", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
                if(x - x0 > moz->Hdim_sbox->value())
                {
                    float margin = ( (x - x0) - moz->Hdim_sbox->value() )/2.0f ;
                    x  = round(x  - margin);
                    x0 = round(x0 + margin);
                }
                if(y - y0 > moz->Vdim_sbox->value())
                {
                    float margin = ( (y - y0) - moz->Vdim_sbox->value() )/2.0f ;
                    y  = round(y  - margin);
                    y0 = round(y0 + margin);
                }
                if(z - z0 > moz->Ddim_sbox->value())
                {
                    float margin = ( (z - z0) - moz->Ddim_sbox->value() )/2.0f ;
                    z  = round(z  - margin);
                    z0 = round(z0 + margin);
                }
                t0 = std::max(0, std::min(t0,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                if(itm::CImport::instance()->is5D() && (t1-t0+1 > moz->Tdim_sbox->value()))
                    t1 = t0 + moz->Tdim_sbox->value();
                if(itm::CImport::instance()->is5D() && (t1 >= itm::CImport::instance()->getTDim()-1))
                    t0 = t1 - (moz->Tdim_sbox->value()-1);
                if(itm::CImport::instance()->is5D() && (t0 == 0))
                    t1 = moz->Tdim_sbox->value()-1;
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, ...to [%d,%d) [%d,%d) [%d,%d) [%d,%d]", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
            }
			//qDebug() << itm::strprintf("\n  After auto_crop:\ntitle = %s, x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d, x0 = %d, y0 = %d, z0 = %d, t0 = %d, t1 = %d, auto_crop = %s, scale_coords = %s, sliding_viewer_block_ID = %d",
			//												titleShort.c_str(),  x, y, z, resolution, dx, dy, dz, x0, y0, z0, t0, t1, auto_crop ? "true" : "false", scale_coords ? "true" : "false", sliding_viewer_block_ID).c_str();
        }

        // ask CVolume to check (and correct) for a valid VOI
        itm::CVolume* cVolume = itm::CVolume::instance();
        try
        {
            if(dx != -1 && dy != -1 && dz != -1)
                cVolume->setVoi(0, resolution, y-dy, y+dy, x-dx, x+dx, z-dz, z+dz, t0, t1);
            else
                cVolume->setVoi(0, resolution, y0, y, x0, x, z0, z, t0, t1);
        }
        catch(itm::RuntimeException &ex)
        {
            /**/itm::warning(itm::strprintf("Exception thrown when setting VOI: \"%s\". Aborting newView", ex.what()).c_str(), __itm__current__function__);
            view3DWidget->setCursor(Qt::ArrowCursor);
            window3D->setCursor(Qt::ArrowCursor);
            loadingNextImg = false;
            return;
        }

        // set the number of streaming steps
        cVolume->setStreamingSteps(1);
		
		int nextImgVolV0 = cVolume->getVoiV0();
		int nextImgVolV1 = cVolume->getVoiV1();
		int nextImgVolH0 = cVolume->getVoiH0();
		int nextImgVolH1 = cVolume->getVoiH1();
		int nextImgVolD0 = cVolume->getVoiD0();
		int nextImgVolD1 = cVolume->getVoiD1();
		int nextImgVolT0 = cVolume->getVoiT0();
		int nextImgVolT1 = cVolume->getVoiT1();
		
		qDebug() << itm::strprintf("\n  Allocating nextImg with nextImgVolV0=%d nextImgVolV1=%d nextImgVolH0=%d nextImgVolH1=%d nextImgVolD0=%d nextImgVolD1=%d", nextImgVolV0, nextImgVolV1, nextImgVolH0, nextImgVolH1, nextImgVolD0, nextImgVolD1).c_str();
		
		try {
			// Allocate memory for data to be loaded
			int imgSize = (nextImgVolH1-nextImgVolH0)*(nextImgVolV1-nextImgVolV0)*(nextImgVolD1-nextImgVolD0)*nchannels*(nextImgVolT1-nextImgVolT0+1);
			itm::uint8* nextImgData = new itm::uint8 [imgSize];
			nextImg = new Image4DSimple();
			nextImg->setFileName(title.c_str());
			nextImg->setData(nextImgData, nextImgVolH1-nextImgVolH0, nextImgVolV1-nextImgVolV0, nextImgVolD1-nextImgVolD0, nchannels*(nextImgVolT1-nextImgVolT0+1), V3D_UINT8);
			nextImg->setTDim(nextImgVolT1-nextImgVolT0+1);
			nextImg->setTimePackType(TIME_PACK_C);
		} catch (...) {
			v3d_msg("Fail to allocate memory for nextImg in Mozak3DView::newViewer();\n");
			loadingNextImg = false;
			return;
		}
		// connect new window to data producer
		cVolume->setSource(this);
        connect(cVolume, SIGNAL(sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)), this, SLOT(receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)), Qt::QueuedConnection);

// lock updateGraphicsInProgress mutex on this thread (i.e. the GUI thread or main queue event thread)
/**/itm::debug(itm::LEV3, itm::strprintf("Waiting for updateGraphicsInProgress mutex").c_str(), __itm__current__function__);
/**/ itm::updateGraphicsInProgress.lock();
/**/itm::debug(itm::LEV3, itm::strprintf("Access granted from updateGraphicsInProgress mutex").c_str(), __itm__current__function__);

        // update status bar message
        moz->statusBar->showMessage("Loading image data...");

        // load new data in a separate thread. When done, the "receiveData" method of the new window will be called
        cVolume->start();

// unlock updateGraphicsInProgress mutex
/**/itm::debug(itm::LEV3, itm::strprintf("updateGraphicsInProgress.unlock()").c_str(), __itm__current__function__);
/**/ itm::updateGraphicsInProgress.unlock();
	}
	catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
		loadingNextImg = false;
    }
}
