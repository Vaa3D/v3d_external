#include "NeuronGame3DView.h"
#include <math.h>
#include "renderer_gl2.h"

#include "../../terafly/src/control/CVolume.h"
#include "../../terafly/src/presentation/PMain.h"

// newViewer implementation
#include "v3d_application.h"
#include "../../terafly/src/control/CImageUtils.h"
#include "./../terafly/src/control/COperation.h"
#include "../../terafly/src/presentation/PAnoToolBar.h"
using namespace neurongame;

int NeuronGame3DView::contrastValue = 0;

NeuronGame3DView::NeuronGame3DView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
		: teramanager::CViewer(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
			_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID)
{
	nextImg = 0;
	/*nextImgVolV0 = 0;
	nextImgVolV1 = 0;
	nextImgVolH0 = 0;
	nextImgVolH1 = 0;
	nextImgVolD0 = 0;
	nextImgVolD1 = 0;
	nextImgVolT0 = 0;
	nextImgVolT1 = 0;*/
	loadingNextImg = false;

	contrastSlider = new QScrollBar(Qt::Vertical);
	contrastSlider->setRange(-50, 50);
	contrastSlider->setSingleStep(1);
	contrastSlider->setPageStep(10);
	contrastSlider->setValue(contrastValue);
	
	QObject::connect(contrastSlider, SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(updateContrast(int)));
}

teramanager::CViewer* NeuronGame3DView::makeView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
{
	//Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	//curr_renderer->finishEditingNeuronTree();
	NeuronGame3DView* neuronView = new NeuronGame3DView(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
		_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID);
	if (teramanager::PMain::getInstance()->annotationsPathLRU == "")
		teramanager::PMain::getInstance()->annotationsPathLRU = "./temp.ano";
	teramanager::PMain::getInstance()->saveAnnotations();
	return neuronView;
}

bool NeuronGame3DView::eventFilter(QObject *object, QEvent *event)
{
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	QKeyEvent* key_evt;
	if (event->type() == QEvent::KeyPress) // intercept keypress events
	{
		key_evt = (QKeyEvent*)event;
		if (key_evt->isAutoRepeat()) return true; // ignore holding down of key
		// Implement custom key events
		int keyPressed = key_evt->key();
		Renderer::SelectMode currentMode = curr_renderer->selectMode;
		Renderer::SelectMode newMode;
		switch (keyPressed)
		{
			case Qt::Key_D:
				newMode = Renderer::smDeleteMultiNeurons;
				break;
			case Qt::Key_S:
				newMode = Renderer::smBreakMultiNeurons;
				break;
            case Qt::Key_E:
                newMode = Renderer::smCurveEditExtend;
                break;
			default:
#ifdef FORCE_BBOX_MODE
				newMode = Renderer::smCurveTiltedBB_fm_sbbox;
#endif
				break;
		}
		if (newMode != currentMode)
		{
			curr_renderer->endSelectMode();
			curr_renderer->selectMode = newMode;
		}
	}
	else if (event->type() == QEvent::KeyRelease) // intercept keypress events
	{
		key_evt = (QKeyEvent*)event;
		if (key_evt->isAutoRepeat()) return true; // ignore holding down of key
#ifdef FORCE_BBOX_MODE
		if (curr_renderer->selectMode != Renderer::smCurveTiltedBB_fm_sbbox)
		{
			curr_renderer->endSelectMode();
			curr_renderer->selectMode = Renderer::smCurveTiltedBB_fm_sbbox;
		}
#endif
	}
	else
	{
		return teramanager::CViewer::eventFilter(object, event);
	}
}

void NeuronGame3DView::show()
{
	this->title = "Neuron Game 3D View";
	teramanager::CViewer::show();
	window3D->centralLayout->addWidget(contrastSlider, 1);
}


void NeuronGame3DView::updateContrast(int con) /* contrast from -100 (bright) to 100 (dark) */
{
	// This performs the same functionality as colormap Red->Gray and then
	// adjusting the alpha, but with only one parameter to adjust
	contrastValue = con;
	float exp_val = pow(10.0f, con/50.0f); // map from -100->100 to 100->0.01
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











/*********************************************************************************
* Receive data (and metadata) from <CVolume> throughout the loading process
**********************************************************************************/
void NeuronGame3DView::receiveData(
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
            QElapsedTimer timer;

            // PREVIEW+STREAMING mode only: copy loaded data
            if(cVolume->getStreamingSteps() != 0)
            {
                // update IO time
                //itm::PLog::instance()->appendOperation(new itm::NewViewerOperation(op_dsc.toStdString(), itm::IO, elapsed_time));

                // copy loaded data into Vaa3D viewer
                timer.start();
				itm::uint32 img_dims[5]       = {std::max(0,nextImgVolH1-nextImgVolH0), std::max(0,nextImgVolV1-nextImgVolV0), std::max(0,nextImgVolD1-nextImgVolD0), nchannels, std::max(0,nextImgVolT1-nextImgVolT0+1)};
				// TODO: why is img_offset[0] coming out negative sometimes? There is no >0 check in the original CViewer code
				itm::uint32 img_offset[5]     = {std::max(0,data_s[0]-nextImgVolH0),    std::max(0,data_s[1]-nextImgVolV0),    std::max(0,data_s[2]-nextImgVolD0),    0,         std::max(0,data_s[4]-nextImgVolT0)     };
                itm::uint32 new_img_dims[5]   = {data_c[0],                             data_c[1],                             data_c[2],                             data_c[3], data_c[4]                              };
                itm::uint32 new_img_offset[5] = {0,                                     0,                                     0,                                     0,         0                                      };
                itm::uint32 new_img_count[5]  = {data_c[0],                             data_c[1],                             data_c[2],                             data_c[3], data_c[4]                              };
				itm::CImageUtils::copyVOI(data, new_img_dims, new_img_offset, new_img_count,
                        nextImg->getRawData(), img_dims, img_offset);
                qint64 elapsedTime = timer.elapsed();

                // release memory
                delete[] data;

                // update log
                //sprintf(message, "Streaming %d/%d: Copied block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T=[%d, %d]  to resolution %d",
                //                  step, cVolume->getStreamingSteps(), cVolume->getVoiH0(), cVolume->getVoiH1(), cVolume->getVoiV0(), cVolume->getVoiV1(),
                //                  cVolume->getVoiD0(), cVolume->getVoiD1(), cVolume->getVoiT0(), cVolume->getVoiT1(), cVolume->getVoiResIndex());
                //itm::PLog::instance()->appendOperation(new itm::NewViewerOperation(message, itm::CPU, elapsedTime));
            }

            // if 5D data, update selected time frame
            if(itm::CImport::instance()->is5D())
                view3DWidget->setVolumeTimePoint(window3D->timeSlider->value()-nextImgVolT0);
			//itm::PMain::getInstance()->frameCoord->setText(itm::strprintf("t = %d/%d", window3D->timeSlider->value()+1, itm::CImport::instance()->getTDim()).c_str());

            // PREVIEW+STREAMING mode only: update image data
            ///**/itm::debug(itm::LEV1, strprintf("title = %s: update image data", titleShort.c_str()).c_str(), __itm__current__function__);
            //timer.restart();
            //view3DWidget->updateImageData();
            sprintf(message, "Streaming %d/%d: Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T=[%d, %d] rendered into view %s",
                    step, cVolume->getStreamingSteps(), cVolume->getVoiH0(), cVolume->getVoiH1(),
                    cVolume->getVoiV0(), cVolume->getVoiV1(),
                    cVolume->getVoiD0(), cVolume->getVoiD1(),
                    cVolume->getVoiT0(), cVolume->getVoiT1(), title.c_str());
            //itm::PLog::instance()->appendOperation(new itm::NewViewerOperation(message, itm::GPU, timer.elapsed()));
			

            // operations to be performed when all image data have been loaded
            if(finished)
            {
                // disconnect from data producer
                disconnect(cVolume, SIGNAL(sendData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)), this, SLOT(receiveData(itm::uint8*,itm::integer_array,itm::integer_array,QWidget*,bool,itm::RuntimeException*,qint64,QString,int)));
				
				// Update viewer's data now that new res is loaded
				volH0 = nextImgVolH0;
				volH1 = nextImgVolH1;
				volV0 = nextImgVolV0;
				volV1 = nextImgVolV1;
				volD0 = nextImgVolD0;
				volD1 = nextImgVolD1;
				volT0 = nextImgVolT0;
				volT1 = nextImgVolT1;
				int prevRes = volResIndex;
				volResIndex = cVolume->getVoiResIndex();
				imgData = nextImg->getRawData();

                // reset TeraFly's GUI
                //itm::PMain::getInstance()->resetGUI();

                // exit from "waiting for 5D data" state, if previously set
                if(this->waitingFor5D)
                    this->setWaitingFor5D(false);

                // reset the cursor
                window3D->setCursor(Qt::ArrowCursor);
                view3DWidget->setCursor(Qt::ArrowCursor);
				
				
				//Image4DSimple* image = new Image4DSimple();
				//image->setFileName(title.c_str());
				//image->setData(imgData, volH1-volH0, volV1-volV0, volD1-volD0, nchannels*(volT1-volT0+1), V3D_UINT8);
				//image->setTDim(volT1-volT0+1);
				//image->setData(nextImgData, nextImgVolH1-nextImgVolH0, nextImgVolV1-nextImgVolV0, nextImgVolD1-nextImgVolD0, nchannels*(nextImgVolT1-nextImgVolT0+1), V3D_UINT8);
				//image->setTDim(nextImgVolT1-nextImgVolT0+1);
				//image->setTimePackType(TIME_PACK_C);
				unsigned char* dataPtr = nextImg->getRawData();
				V3D_env->setImage(window, nextImg); // this clears the rawDataPointer for nextImg
				// TODO: delete nextImg?
				//nextImg->setRawDataPointer(dataPtr); // restore the data pointer
				
				// create 3D view window with postponed show()
				XFormWidget *w = V3dApplication::getMainWindow()->validateImageWindow(window);
				view3DWidget->getiDrawExternalParameter()->image4d = w->getImageData();
				//w->doImage3DView(true, 0, -1, -1,-1, -1, -1,-1, false);
				//view3DWidget = (V3dR_GLWidget*)(V3D_env->getView3DControl(window));
				//if(!view3DWidget->getiDrawExternalParameter())
				//	QMessageBox::critical(itm::PMain::getInstance(),QObject::tr("Error"), QObject::tr("Unable to get iDrawExternalParameter from Vaa3D's V3dR_GLWidget"),QObject::tr("Ok"));
				//window3D = view3DWidget->getiDrawExternalParameter()->window3D;
				
				// set fixed size that fills available screen space
				//window3D->resize(qApp->desktop()->availableGeometry().width()-PMain::getInstance()->width(), PMain::getInstance()->height());

				// show 3D viewer
				//window3D->show();
				
				// install the event filter on the 3D renderer and on the 3D window
				//view3DWidget->installEventFilter(this);
				//window3D->installEventFilter(this);
				//window3D->timeSlider->installEventFilter(this);

				// time slider: disconnect Vaa3D event handlers and set the complete (virtual) time range
				//disconnect(view3DWidget, SIGNAL(changeVolumeTimePoint(int)), window3D->timeSlider, SLOT(setValue(int)));
				//disconnect(window3D->timeSlider, SIGNAL(valueChanged(int)), view3DWidget, SLOT(setVolumeTimePoint(int)));
				//window3D->timeSlider->setMinimum(0);
				//window3D->timeSlider->setMaximum(CImport::instance()->getTDim()-1);

				float ratio = itm::CImport::instance()->getVolume(volResIndex)->getDIM_D()/itm::CImport::instance()->getVolume(prevRes)->getDIM_D();
				float curZoom = (float)view3DWidget->zoom();
				view3DWidget->setZoom(curZoom/ratio);
				
				//this->window3D->raise();
				//this->window3D->activateWindow();
				this->window3D->show();

				itm::PMain* pMain = itm::PMain::getInstance();

				// updating reference system
				if(!pMain->isPRactive())
					pMain->refSys->setDims(volH1-volH0+1, volV1-volV0+1, volD1-volD0+1);
				this->view3DWidget->updateGL();     // if omitted, Vaa3D_rotationchanged somehow resets rotation to 0,0,0
				Vaa3D_rotationchanged(0);

				// update curve aspect
				pMain->curveAspectChanged();

				
				// TODO: store nextImgData and imgData somehow along with volH1, etc

                //current window is now ready for user input
                isReady = true;
				loadingNextImg = false;
            }
        }
		catch(itm::RuntimeException &ex)
        {
			QMessageBox::critical(itm::PMain::getInstance(),QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
			itm::PMain::getInstance()->resetGUI();
            isReady = true;
			loadingNextImg = false;
        }
    }
//    QMessageBox::information(0, "Stop", "Wait...");
    /**/itm::debug(itm::LEV3, "method terminated", __itm__current__function__);
}


/**********************************************************************************
* Generates a new view using the given coordinates.
* Called by the current <CViewer> when the user zooms in and the higher res-
* lution has to be loaded.
***********************************************************************************/
void
NeuronGame3DView::newViewer(int x, int y, int z,                            //can be either the VOI's center (default) or the VOI's ending point (see x0,y0,z0)
    int resolution,                                 //resolution index of the view requested
    int t0, int t1,                                 //time frames selection
    bool fromVaa3Dcoordinates /*= false*/,          //if coordinates were obtained from Vaa3D
    int dx/*=-1*/, int dy/*=-1*/, int dz/*=-1*/,    //VOI [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz), [t0, t1]
    int x0/*=-1*/, int y0/*=-1*/, int z0/*=-1*/,    //VOI [x0, x), [y0, y), [z0, z), [t0, t1]
    bool auto_crop /* = true */,                    //whether to crop the VOI to the max dims
    bool scale_coords /* = true */,                 //whether to scale VOI coords to the target res
    int sliding_viewer_block_ID /* = -1 */)         //block ID in "Sliding viewer" mode

{
	/**/itm::debug(itm::LEV1, itm::strprintf("title = %s, x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d, x0 = %d, y0 = %d, z0 = %d, t0 = %d, t1 = %d, auto_crop = %s, scale_coords = %s, sliding_viewer_block_ID = %d",
                                        titleShort.c_str(),  x, y, z, resolution, dx, dy, dz, x0, y0, z0, t0, t1, auto_crop ? "true" : "false", scale_coords ? "true" : "false", sliding_viewer_block_ID).c_str(), __itm__current__function__);

    // check precondition #1: active window
    //if(!isActive || toBeClosed)
    //{
    //    QMessageBox::warning(0, "Unexpected behaviour", "Precondition check \"!isActive || toBeClosed\" failed. Please contact the developers");
    //    return;
    //}

    // check precondition #2: valid resolution
    if(resolution >= itm::CImport::instance()->getResolutions())
        resolution = volResIndex;

    // check precondition #3: window ready for "newView" events
    //if( !isReady )
    //{
    //    itm::warning("precondition (!isReady) not met. Aborting newView", __itm__current__function__);
    //    return;
    //}


    // deactivate current window and processing all pending events
	// TODO: Keep this?
    //QApplication::processEvents();

    // after processEvents(), it might be that this windows is no longer valid, then terminating
	if(toBeClosed || loadingNextImg)
        return;
	
	loadingNextImg = true;

    // restart timer (measures the time needed to switch to a new view)
    newViewerTimer.restart();

    // create new macro-group for NewViewerOperation
	// TODO: Keep this?
	//itm::NewViewerOperation::newGroup();


    try
    {
        // set GUI to waiting state
        QElapsedTimer timer;
        timer.start();
        itm::PMain& pMain = *(itm::PMain::getInstance());
        pMain.progressBar->setEnabled(true);
        pMain.progressBar->setMinimum(0);
        pMain.progressBar->setMaximum(0);
        pMain.statusBar->showMessage("Switching view...");
        view3DWidget->setCursor(Qt::BusyCursor);
        window3D->setCursor(Qt::BusyCursor);
        pMain.setCursor(Qt::BusyCursor);

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


        // adjust time size so as to use all the available frames set by the user
        if(itm::CImport::instance()->is5D() && ((t1 - t0 +1) != pMain.Tdim_sbox->value()))
        {
            t1 = t0 + (pMain.Tdim_sbox->value()-1);
            /**/itm::debug(itm::LEV1, itm::strprintf("mismatch between |[t0,t1]| (%d) and max T dims (%d), adjusting it to [%d,%d]", t1-t0+1, pMain.Tdim_sbox->value(), t0, t1).c_str(), __itm__current__function__);
        }


        // crop VOI if its larger than the maximum allowed
        if(auto_crop)
        {
            // modality #1: VOI = [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz), [t0, t1]
            if(dx != -1 && dy != -1 && dz != -1)
			{
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, cropping bbox dims from (%d,%d,%d) t[%d,%d] to...", titleShort.c_str(),  dx, dy, dz, t0, t1).c_str(), __itm__current__function__);
				dx = std::min(dx, itm::round(pMain.Hdim_sbox->value()/2.0f));
                dy = std::min(dy, itm::round(pMain.Vdim_sbox->value()/2.0f));
                dz = std::min(dz, itm::round(pMain.Ddim_sbox->value()/2.0f));
                t0 = std::max(0, std::min(t0,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                if(itm::CImport::instance()->is5D() && (t1-t0+1 > pMain.Tdim_sbox->value()))
                    t1 = t0 + pMain.Tdim_sbox->value();
                if(itm::CImport::instance()->is5D() && (t1 >= itm::CImport::instance()->getTDim()-1))
                    t0 = t1 - (pMain.Tdim_sbox->value()-1);
                if(itm::CImport::instance()->is5D() && (t0 == 0))
                    t1 = pMain.Tdim_sbox->value()-1;
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, ...to (%d,%d,%d)", titleShort.c_str(),  dx, dy, dz).c_str(), __itm__current__function__);
            }
            // modality #2: VOI = [x0, x), [y0, y), [z0, z), [t0, t1]
            else
            {
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, cropping bbox dims from [%d,%d) [%d,%d) [%d,%d) [%d,%d] to...", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
                if(x - x0 > pMain.Hdim_sbox->value())
                {
                    float margin = ( (x - x0) - pMain.Hdim_sbox->value() )/2.0f ;
                    x  = round(x  - margin);
                    x0 = round(x0 + margin);
                }
                if(y - y0 > pMain.Vdim_sbox->value())
                {
                    float margin = ( (y - y0) - pMain.Vdim_sbox->value() )/2.0f ;
                    y  = round(y  - margin);
                    y0 = round(y0 + margin);
                }
                if(z - z0 > pMain.Ddim_sbox->value())
                {
                    float margin = ( (z - z0) - pMain.Ddim_sbox->value() )/2.0f ;
                    z  = round(z  - margin);
                    z0 = round(z0 + margin);
                }
                t0 = std::max(0, std::min(t0,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,itm::CImport::instance()->getVolume(volResIndex)->getDIM_T()-1));
                if(itm::CImport::instance()->is5D() && (t1-t0+1 > pMain.Tdim_sbox->value()))
                    t1 = t0 + pMain.Tdim_sbox->value();
                if(itm::CImport::instance()->is5D() && (t1 >= itm::CImport::instance()->getTDim()-1))
                    t0 = t1 - (pMain.Tdim_sbox->value()-1);
                if(itm::CImport::instance()->is5D() && (t0 == 0))
                    t1 = pMain.Tdim_sbox->value()-1;
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, ...to [%d,%d) [%d,%d) [%d,%d) [%d,%d]", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
            }
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

            setActive(true);
            view3DWidget->setCursor(Qt::ArrowCursor);
            window3D->setCursor(Qt::ArrowCursor);
            itm::PMain::getInstance()->resetGUI();
			loadingNextImg = false;
            return;
        }

        // PREVIEW+STREAMING mode - obtain low res data from current window to be displayed in a new window while the user waits for the new high res data
    
        // get low res data
        //timer.restart();
        //int voiH0m=0, voiH1m=0, voiV0m=0, voiV1m=0,voiD0m=0, voiD1m=0, voiT0m=0, voiT1m=0;
        //int rVoiH0 = itm::CVolume::scaleHCoord(cVolume->getVoiH0(), resolution, volResIndex);
        //int rVoiH1 = itm::CVolume::scaleHCoord(cVolume->getVoiH1(), resolution, volResIndex);
        //int rVoiV0 = itm::CVolume::scaleVCoord(cVolume->getVoiV0(), resolution, volResIndex);
        //int rVoiV1 = itm::CVolume::scaleVCoord(cVolume->getVoiV1(), resolution, volResIndex);
        //int rVoiD0 = itm::CVolume::scaleDCoord(cVolume->getVoiD0(), resolution, volResIndex);
        //int rVoiD1 = itm::CVolume::scaleDCoord(cVolume->getVoiD1(), resolution, volResIndex);
        //itm::uint8* lowresData = getVOI(rVoiH0, rVoiH1, rVoiV0, rVoiV1, rVoiD0, rVoiD1, cVolume->getVoiT0(), cVolume->getVoiT1(),
        //                           cVolume->getVoiH1()-cVolume->getVoiH0(),
        //                           cVolume->getVoiV1()-cVolume->getVoiV0(),
        //                           cVolume->getVoiD1()-cVolume->getVoiD0(),
        //                           voiH0m, voiH1m, voiV0m, voiV1m,voiD0m, voiD1m, voiT0m, voiT1m);
        //std::string message = itm::strprintf("Block X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T[%d, %d] loaded from view %s, black-filled region is "
        //                       "X=[%d, %d) Y=[%d, %d) Z=[%d, %d) T[%d, %d]",
        //        rVoiH0, rVoiH1, rVoiV0, rVoiV1, rVoiD0, rVoiD1, cVolume->getVoiT0(), cVolume->getVoiT1(), title.c_str(),
        //        voiH0m, voiH1m, voiV0m, voiV1m,voiD0m, voiD1m, voiT0m, voiT1m);
		//itm::PLog::instance()->appendOperation(new itm::NewViewerOperation(message, itm::CPU, timer.elapsed()));

        // create new window
        //this->next = makeView(V3D_env, resolution, lowresData,
        //                                 cVolume->getVoiV0(), cVolume->getVoiV1(), cVolume->getVoiH0(), cVolume->getVoiH1(), cVolume->getVoiD0(), cVolume->getVoiD1(),
        //                                 cVolume->getVoiT0(), cVolume->getVoiT1(), nchannels, this, sliding_viewer_block_ID);

        // update CVolume with the request of the actual missing VOI along t and the current selected frame
        //cVolume->setVoiT(voiT0m, voiT1m, window3D->timeSlider->value());

        // set the number of streaming steps
        cVolume->setStreamingSteps(1);//itm::PMain::getInstance()->debugStreamingStepsSBox->value());
		
		if (nextImg)
			delete nextImg;
		
		int nextImgVolV0 = cVolume->getVoiV0();
		int nextImgVolV1 = cVolume->getVoiV1();
		int nextImgVolH0 = cVolume->getVoiH0();
		int nextImgVolH1 = cVolume->getVoiH1();
		int nextImgVolD0 = cVolume->getVoiD0();
		int nextImgVolD1 = cVolume->getVoiD1();
		int nextImgVolT0 = cVolume->getVoiT0();
		int nextImgVolT1 = cVolume->getVoiT1();

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
			v3d_msg("Fail to allocate memory for nextImg in NeuronGame3DView::newViewer();\n");
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
        pMain.statusBar->showMessage("Loading image data...");

        // load new data in a separate thread. When done, the "receiveData" method of the new window will be called
        cVolume->start();

        // meanwhile, show the new window with preview data
        //next->show();

        // enter "waiting for 5D data" state, if possible
        //next->setWaitingFor5D(true);

        //if the resolution of the loaded voi is the same of the current one, this window will be closed
        //if(resolution == volResIndex)
        //    this->close();

// unlock updateGraphicsInProgress mutex
/**/itm::debug(itm::LEV3, itm::strprintf("updateGraphicsInProgress.unlock()").c_str(), __itm__current__function__);
/**/ itm::updateGraphicsInProgress.unlock();
	}
	catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
		itm::PMain::getInstance()->resetGUI();
		loadingNextImg = false;
    }
}
