#include "Mozak3DView.h"
#include "MozakUI.h"
#include <math.h>
#include "renderer_gl2.h"
#include "v3d_application.h"
// 20170624 RZC: central control include files in Mozak3DView.h
//#include "../terafly/src/control/CAnnotations.h"
//#include "../terafly/src/control/CVolume.h"
//#include "../terafly/src/control/CImageUtils.h"
//#include "../terafly/src/presentation/PAnoToolBar.h"

using namespace mozak;

int Mozak3DView::contrastValue = 0;

Mozak3DView::Mozak3DView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
		: teramanager::CViewer(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
			_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID)
{
	//170729 RZC: to fix crash of calling updateUndoLabel() in appendHistory() when starting
	currUndoLabel = 0; buttonUndo = buttonRedo = 0;

	nextImg = 0;
    prevZCutMin = -1;
    prevZCutMax = -1;
	extraSurfaceOffset = 2;
    cur_history = -1;
    currentWriggleFrame = 0;
	loadingNextImg = false;
    isWriggling = false;
	neuronColorMode = 1;
	contrastSlider = new QScrollBar(Qt::Vertical);
	contrastSlider->setRange(-50, 50);
	contrastSlider->setSingleStep(1);
	contrastSlider->setPageStep(10);
	contrastSlider->setValue(contrastValue);

    itm::CSettings::instance()->setTraslX(60); // (100% - this setting) = % of existing view to be translated in X
    itm::CSettings::instance()->setTraslY(60); // (100% - this setting) = % of existing view to be translated in Y
	paint_timer = new QTimer(this);
    QObject::connect(paint_timer, SIGNAL(timeout()), this, SLOT(paintTimerCall()));
    paint_timer->setInterval( (int)(500) );
    paint_timer->setSingleShot(false);
	QObject::connect(contrastSlider, SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(updateContrast(int)));


	total_wriggle_time = .7;
	total_wriggle_frames = 30.0;
    wriggle_timer = new QTimer(this);
    QObject::connect(wriggle_timer, SIGNAL(timeout()), this, SLOT(wriggleTimerCall()));
    wriggle_timer->setSingleShot(false);
    wriggle_timer->setInterval( (int)(1000.0 * total_wriggle_time / total_wriggle_frames) ); // num of rotations per second = 2 / 15;
    //wriggle_timer->setInterval( 70 );
    QObject::connect(contrastSlider, SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(updateContrast(int)));
	changingGrid=false;
	gridSpacing = 1000;
	deltaGridSpacing = 50;
	minGridSpacing = 100;
	maxGridSpacing = 5000;
	setGrid(-1);
	
}

teramanager::CViewer* Mozak3DView::makeView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
{
	Mozak3DView* neuronView = new Mozak3DView(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
		_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID);
	return neuronView;
}

void Mozak3DView::appendHistory()
{
    qDebug() << "Adding to undoRedoHistory: (before) undoRedoHistory.size()=" << undoRedoHistory.size() << " cur_history=" << cur_history;

    // Erase any redos after this point in history
    while (cur_history > -1 && undoRedoHistory.size() > cur_history + 1) undoRedoHistory.takeLast();
    
    // Ensure history size <= MAX_history
	while (undoRedoHistory.size() >= MAX_history) undoRedoHistory.pop_front();
    
    itm::interval_t x_range(0, itm::CAnnotations::getInstance()->octreeDimX);
    itm::interval_t y_range(0, itm::CAnnotations::getInstance()->octreeDimY);
    itm::interval_t z_range(0, itm::CAnnotations::getInstance()->octreeDimZ);

    NeuronTree nt;
    itm::CAnnotations::getInstance()->findCurves(x_range, y_range, z_range, nt.listNeuron);
    undoRedoHistory.push_back(nt);
	cur_history = undoRedoHistory.size() - 1;

	if (currUndoLabel && buttonUndo && buttonRedo) //20170729 RZC
		updateUndoLabel();

//	autoSave();  //20170803 RZC
}

void Mozak3DView::performUndo()
{
    cur_history--;
    if (undoRedoHistory.size() == 0 || cur_history < 0) {
        qDebug() << "Undo aborted: undoRedoHistory.size()=" << undoRedoHistory.size() << " cur_history=" << cur_history;
        cur_history = (undoRedoHistory.size() == 0) ? -1 : 0;
        // Annoying: v3d_msg("Reached the earliest of saved history!");
        updateUndoLabel();
        return;
    }
    qDebug() << "Undoing...";
    cur_history = min(undoRedoHistory.size() - 1, cur_history);
    
    // X,Y,Z intervals are used to clear annotations, which we've already done above, so input minimal intervals
    itm::CAnnotations::getInstance()->clear();
    NeuronTree nt = undoRedoHistory.at(cur_history);
    itm::CAnnotations::getInstance()->addCurves(itm::interval_t(0, 1), itm::interval_t(0, 1), itm::interval_t(0, 1), nt);
    itm::CViewer::loadAnnotations();

    makeTracedNeuronsEditable();

	updateUndoLabel();

	autoSave();  //20170803 RZC
}

void Mozak3DView::performRedo()
{
    cur_history++;
	if (undoRedoHistory.size() < 1 || cur_history >= undoRedoHistory.size())
	{
        qDebug() << "Redo aborted: undoRedoHistory.size()=" << undoRedoHistory.size() << " cur_history=" << cur_history;
		cur_history = undoRedoHistory.size() - 1;
		// Annoying: v3d_msg("Reached the latest of saved history!");
	}
	else if (cur_history >= 0 && cur_history < undoRedoHistory.size())
	{
        qDebug() << "Redoing...";
		itm::CAnnotations::getInstance()->clear();
        NeuronTree nt = undoRedoHistory.at(cur_history);
        itm::CAnnotations::getInstance()->addCurves(itm::interval_t(0, 1), itm::interval_t(0, 1), itm::interval_t(0, 1), nt);
        itm::CViewer::loadAnnotations();

        makeTracedNeuronsEditable();
	}
    updateUndoLabel();

	autoSave();  //20170803 RZC
}



//20170803 RZC: extract from onNeuronEdit() to be reused in performUndo/performRedo
void Mozak3DView::autoSave()
{
    MozakUI* moz = MozakUI::getMozakInstance();
    std::string prevPath = moz->annotationsPathLRU;
	moz->annotationsPathLRU = "./autosave.ano";
	moz->saveAnnotations();
    moz->annotationsPathLRU = prevPath;
}

void Mozak3DView::onNeuronEdit()
{
   // teramanager::CViewer::onNeuronEdit();

	autoSave();  //must called before appendHistory() otherwise redo cannot work

	// makeTracedNeuronsEditable();  //no use

    appendHistory();
}

void Mozak3DView::makeTracedNeuronsEditable()
{
//	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	Renderer_gl1* curr_renderer = (Renderer_gl1*)(view3DWidget->getRenderer());
	if (! curr_renderer) return;    //20170803 RZC

	int sz = curr_renderer->listNeuronTree.size();
	for (int i=0; i<sz; i++)
	{
		curr_renderer->listNeuronTree[i].editable = true;
	}
//	curr_renderer->paint(); //no use
}

//find the nearest node in a neuron in XY project of the display window
V3DLONG Mozak3DView::findNearestNeuronNode(int cx, int cy, bool updateStartType/*=false*/)
{
    Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    V3DLONG best_ind=-1;
    V3DLONG best_dist=-1;
    int prev_type = curr_renderer->highlightedNodeType;
    QList<NeuronTree>::iterator it;
    for (it = (curr_renderer->listNeuronTree).begin(); it != (curr_renderer->listNeuronTree.end()); ++it){
        QList <NeuronSWC> p_listneuron = it->listNeuron;

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
        if (updateStartType && best_ind >= 0 && p_listneuron.at(best_ind).type != 1)
                curr_renderer->highlightedNodeType = p_listneuron.at(best_ind).type;
    }
    if (prev_type != curr_renderer->highlightedNodeType)
        updateTypeLabel();
    return best_ind; //by PHC, 090209. return the index in the SWC file
}

bool Mozak3DView::eventFilter(QObject *object, QEvent *event)
{
	if(!isReady)
		return false;
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    Renderer::SelectMode currentMode = curr_renderer->selectMode;
    view3DWidget->setMouseTracking(true);
    bool needRepaint = false;
    if (event->type() == QEvent::MouseMove)
    {

        QMouseEvent *k = (QMouseEvent *)event;
        int isLeftMouseDown = k->buttons() & Qt::LeftButton;
        int isRightMouseDown = k->buttons() & Qt::RightButton;
        if (isLeftMouseDown == 0 && isRightMouseDown == 0)
        {
            if (!curr_renderer->bShowXYTranslateArrows)
            {
                curr_renderer->bShowXYTranslateArrows = true;
                needRepaint = true;
            }
            checkXyArrowMouseCollision(k->x(), k->y(), curr_renderer, needRepaint);
        }
        
        //On mouse move, if one of the extend mode is enabled, then update nodes to be highlighted

        bool updatedNodes = false;
        if(currentMode == Renderer::smCurveEditExtendOneNode || currentMode == Renderer::smCurveEditExtendTwoNode){
            if(!isRightMouseDown){
                //Highlight start node
                curr_renderer->highlightedNode = findNearestNeuronNode(k->x(), k->y(), true);
                updatedNodes = true;
            }else if(currentMode == Renderer::smCurveEditExtendTwoNode){
                //Highlight end node
                V3DLONG highlightedEndNodePrev = curr_renderer->highlightedEndNode;
                curr_renderer->highlightedEndNode = findNearestNeuronNode(k->x(), k->y());
                curr_renderer->highlightedEndNodeChanged = (highlightedEndNodePrev == curr_renderer->highlightedEndNode);
                if (curr_renderer->highlightedEndNodeChanged)
                    updatedNodes = true;
            }
        }
        else if (currentMode == Renderer::smJoinTwoNodes)
        {
            if (curr_renderer->selectedStartNode == -1)
            {
                //Highlight start node
                curr_renderer->highlightedNode = findNearestNeuronNode(k->x(), k->y(), true);
                updatedNodes = true;
            }
            else
            {
                //Highlight end node
                V3DLONG highlightedEndNodePrev = curr_renderer->highlightedEndNode;
                curr_renderer->highlightedEndNode = findNearestNeuronNode(k->x(), k->y());
                curr_renderer->highlightedEndNodeChanged = (highlightedEndNodePrev == curr_renderer->highlightedEndNode);
                if (curr_renderer->highlightedEndNodeChanged)
                    updatedNodes = true;
            }
		}else if (currentMode == Renderer::smHighlightChildren	)
        {
			
            
                //Highlight start node
                curr_renderer->highlightedNode = findNearestNeuronNode(k->x(), k->y(), true);
                updatedNodes = true;
				
            
		}
        if (updatedNodes)
        {
            curr_renderer->drawNeuronTreeList();
            curr_renderer->drawObj();
            needRepaint = true;
        }
    }

    if (event->type() == QEvent::MouseButtonRelease)
    {
        //This is a very unfortunate workaround to solve an issue where the cursor move calls
        //stop happening at times even when setMouseTracking is enabled.
        view3DWidget->setMouseTracking(false);
    }

    if (needRepaint)
        ((QWidget *)(curr_renderer->widget))->repaint();

    QKeyEvent* key_evt;
	QMouseEvent* mouseEvt;
	bool neuronTreeChanged = false;
	if (((object == view3DWidget || object == window3D) && event->type() == QEvent::Wheel))
	{
		QWheelEvent* wheelEvt = (QWheelEvent*)event;


		if (changingGrid && curr_renderer->showingGrid){
			if (wheelEvt->delta() > 0 && gridSpacing>minGridSpacing+deltaGridSpacing){
				this->gridSpacing = this->gridSpacing-deltaGridSpacing;
			}
			if (wheelEvt->delta() < 0&& gridSpacing<maxGridSpacing+deltaGridSpacing){
				this->gridSpacing = this->gridSpacing+deltaGridSpacing;
			}
			setGrid(-1);
			updateGrid();
			return true;
		}


		if (currentMode == Renderer::smCurveCreate_pointclick || currentMode == Renderer::smCurveCreate_pointclickAutoZ)
		{
			// If polyline mode(s), use mouse wheel to change the current z-slice (and restrict to one or NUM_POLY_AUTO_Z_PLANES)
			int zoff = (currentMode == Renderer::smCurveCreate_pointclick) ? 0 : ((NUM_POLY_AUTO_Z_PLANES - 1) / 2);

			int volumeDelta = 1;
			int prevVal = (window3D->zcminSlider->value() + window3D->zcmaxSlider->value()) / 2;
			float zSliderScaleFactor = ((float)(window3D->zSminSlider->maximum()- window3D->zSminSlider->minimum()+1))/((float)(window3D->zcminSlider->maximum()-window3D->zcminSlider->minimum()+1-zoff));
			qDebug()<<" zSliderScaleFactor: "<<zSliderScaleFactor;
			qDebug()<<" prevVal : "<<prevVal;
			qDebug()<<" vol max, minimum " <<window3D->zcminSlider->maximum()<<", "<<window3D->zcminSlider->minimum();
			qDebug()<<" S max, minimum " <<window3D->zSminSlider->maximum()<<", "<<window3D->zSminSlider->minimum();		
			


			// this is doing what I want it to with the sliders, it's just that the sliders are fundamentally different. it's stupid.
			// first, the sliders are scaled differently. the surface slider is "0 to 200" regardless of input volume size
			// second, sometimes the z range for the vector objects spans the entire slider range, other times it doesn't. 
			// maybe the solution is to just set the min and max myself upon stack loading...

			int volumeMinSliderUp = prevVal - zoff + volumeDelta;
			int volumeMaxSliderUp = prevVal + zoff + volumeDelta;
			int volumeMinSliderDown = prevVal - zoff - volumeDelta;
			int volumeMaxSliderDown = prevVal + zoff - volumeDelta;


            if (wheelEvt->delta() > 0)
			{
                if (volumeMinSliderUp >= window3D->zcminSlider->minimum() && 
                    volumeMaxSliderUp <= window3D->zcmaxSlider->maximum())
                {
				    window3D->zcminSlider->setValue(volumeMinSliderUp);
				    window3D->zcmaxSlider->setValue(volumeMaxSliderUp);



                }
			}
			else
			{
                if (volumeMinSliderDown >= window3D->zcminSlider->minimum() && 
                    volumeMaxSliderDown <= window3D->zcmaxSlider->maximum())
                {
				    window3D->zcminSlider->setValue(volumeMinSliderDown);
				    window3D->zcmaxSlider->setValue(volumeMaxSliderDown);



                }
			}
		}
		else
		{
			int prevZoom = view3DWidget->zoom();
			
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
		}
		return true;
	}
	else if (object == view3DWidget && event->type() == QEvent::MouseButtonDblClick) //
	{
		QMouseEvent* mouseEvt = (QMouseEvent*)event;
		// Right mouse double click reduces resolution
		if (mouseEvt->button() == Qt::RightButton)
		{
            // Disable resolution change during polyline to ignore unintentional double right clicks
			if (currentMode == Renderer::smCurveCreate_pointclick) return false;

			// Determine whether we need to go to a previous (lower) resolution when zooming out
			while (lowerResViews.length() > 0)
			{
				CViewInfo *prevView = lowerResViews.takeLast();

				if (prevView && (prevView->resIndex != volResIndex))
				{
					isReady = false;
					loadingNextImg = true;
					window3D->setCursor(Qt::BusyCursor);
					view3DWidget->setCursor(Qt::BusyCursor);
					itm::CVolume* cVolume = itm::CVolume::instance();
					try
					{
						cVolume->setVoi(0, prevView->resIndex, prevView->volV0, prevView->volV1, prevView->volH0, prevView->volH1,
							prevView->volD0, prevView->volD1, prevView->volT0, prevView->volT1);
					}
					catch(itm::RuntimeException &ex)
					{
						qDebug() << "WARNING! Exception thrown trying to call cVolume->setVoi: " << ex.what();
					}

					loadNewResolutionData(prevView->resIndex, prevView->img,
						prevView->volV0, prevView->volV1, prevView->volH0, prevView->volH1, 
						prevView->volD0, prevView->volD1, prevView->volT0, prevView->volT1);
					prevView->img->setRawDataPointerToNull(); // this image is in use now, so remove pointer before deletion
					delete prevView;
				
					window3D->setCursor(Qt::ArrowCursor);
					view3DWidget->setCursor(Qt::ArrowCursor);
					loadingNextImg = false;
					isReady = true;
					return true;
				} 
				else if (prevView)
				{
					// the last view was at the same resolution; skip over it when going to lower resolutions
					prevView->img->setRawDataPointerToNull();
					delete prevView;
				}
			}
			
			return false;
		}
		else
		{
			// Left double clicks (zoom in resolution) handled by CViewer 
			return teramanager::CViewer::eventFilter(object, event);
		}
	}

#define ____key_pressed_event___
	else if (object == view3DWidget && event->type() == (QEvent::Type)6)  //2017-6-9 RZC: deal with the #define KeyPress in X.h of XWindow
														//QEvent::KeyPress) // intercept keypress events
	{
		key_evt = (QKeyEvent*)event;

		// Implement custom key events
        int keyPressed = key_evt->key();
		if (key_evt->isAutoRepeat()&&keyPressed!=Qt::Key_A&&keyPressed!=Qt::Key_P) return true; // ignore holding down of key unless it's being used to simply click (or unclick) a QAbstractButton


		Renderer::SelectMode newMode;
        bool bAddCurve = true;

		switch (keyPressed)
		{
			case Qt::Key_0:
				curr_renderer->currentTraceType = 0; // undefined
				updateTypeLabel();
				break;
			case Qt::Key_1:
				curr_renderer->currentTraceType = 1; // soma
				updateTypeLabel();
				break;
			case Qt::Key_2:
				curr_renderer->currentTraceType = 2; // axon
				updateTypeLabel();
				break;
			case Qt::Key_3:
				curr_renderer->currentTraceType = 3; // dendrite
				updateTypeLabel();
				break;
			case Qt::Key_4:
				curr_renderer->currentTraceType = 4; // apical dendrite
				updateTypeLabel();
				break;
			case Qt::Key_5:
				curr_renderer->currentTraceType = 5; // fork point
				updateTypeLabel();
				break;
			case Qt::Key_6:
				curr_renderer->currentTraceType = 6; // end point
				updateTypeLabel();
				break;
			case Qt::Key_7:
				curr_renderer->currentTraceType = 7; // FixIt! Axon
                updateTypeLabel();
				break;
            case Qt::Key_8:
                curr_renderer->currentTraceType = 8; // FixIt! Dendrite
                updateTypeLabel();
                break;
            case Qt::Key_9:
                curr_renderer->currentTraceType = 9; // FixIt! Unknown
                updateTypeLabel();
                break;
            case Qt::Key_A:
				polyLineAutoZButton->animateClick();
				//polyLineAutoZButton->toggle();
               // if (!polyLineAutoZButton->isChecked())
				//	polyLineAutoZButton->setChecked(true);

//                changeMode(Renderer::smCurveCreate_pointclickAutoZ, true, true);
                break;
			case Qt::Key_D:
				if (!deleteSegmentsButton->isChecked())
					deleteSegmentsButton->setChecked(true);
				changeMode(Renderer::smDeleteMultiNeurons, false, true);
                break;
            case Qt::Key_W:{
                    if(!isWriggling){

                        for(int i = 0; i < 16; i++){
                           originalRotationMatrix[i] = view3DWidget->mRot[i];
                     }
						isWriggling = true;
                        currentWriggleFrame = 0;
						wriggle_timer->start();

					}
                }

                break;
            case Qt::Key_J:
				if (!joinButton->isChecked())
					joinButton->setChecked(true);
				changeMode(Renderer::smJoinTwoNodes, false, true);
				break;
            case Qt::Key_S:
				if (!splitSegmentButton->isChecked())
				{
					splitSegmentButton->setChecked(true);
				changeMode(Renderer::smBreakTwoNeurons, false, true);
				}break;
			case Qt::Key_G:
				changingGrid = true;
				curr_renderer->showingGrid = !curr_renderer->showingGrid;
				if (this->overviewActive){
					MozakUI* moz = MozakUI::getMozakInstance();
					Renderer_gl1* overviewRenderer =(Renderer_gl1*)( moz->V3D_env->find3DViewerByName("Overview")->getGLWidget()->getRenderer());
					overviewRenderer->showingGrid = !overviewRenderer->showingGrid ;
				}
				break;
			case Qt::Key_O:
				// toggle reconstruction entirely on and off
				if (curr_renderer->sShowSurfObjects!=2){
					curr_renderer->sShowSurfObjects = 2;
				}else{
					curr_renderer->sShowSurfObjects = 0;
				}
				break;
			case Qt::Key_R:
				if (!retypeSegmentsButton->isChecked())
					retypeSegmentsButton->setChecked(true);
				changeMode(Renderer::smRetypeMultiNeurons, false, true);
				break;
			case Qt::Key_P:
				polyLineButton->click();
				//if (!polyLineButton->isChecked())
				//	polyLineButton->setChecked(true);
				//changeMode(Renderer::smCurveCreate_pointclick, true, true);
                break;
            case Qt::Key_Y:
                if (key_evt->modifiers() & Qt::ControlModifier)
                {
                    performRedo();
                }
                break;
			case Qt::Key_Z:
                if (key_evt->modifiers() & Qt::ControlModifier)
                {
                    performUndo();
                }
				else
				{
				    // stretch the image volume in the z-axis
				    view3DWidget->setThickness(3.0);
                }
				break;
			case Qt::Key_L:
				{
					zLockButton->click();
				}
				break;
            case Qt::Key_H:
                //Sets segment rendermode to "Transparent" (0.1 alpha).
                if(curr_renderer->polygonMode == 3){
                    curr_renderer->polygonMode = 0;
                }else{
                    curr_renderer->polygonMode = 3;
                }
                break;
            case Qt::Key_V:
				neuronColorMode++;
				neuronColorMode = neuronColorMode%5;
				updateColorMode(neuronColorMode);
                //curr_renderer->colorByTypeOnlyMode = !(curr_renderer->colorByTypeOnlyMode); //colorByTypeOnly was eliminated before last release from Mozak crew.
	
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
            case Qt::Key_Space:
                // Reset view to viewing XY plane from +z axis
                view3DWidget->doAbsoluteRot(0, 0, 0);
                break;

			case Qt::Key_Up:				// Process X/Y ROI Translate
				if (key_evt->modifiers() & Qt::ControlModifier)
				{
					MozakUI* moz = MozakUI::getMozakInstance();
					moz->traslYnegClicked();
				}
				break;
			case Qt::Key_Left:				// Process X/Y ROI Translate
				if (key_evt->modifiers() & Qt::ControlModifier)
				{
					MozakUI* moz = MozakUI::getMozakInstance();
					moz->traslXnegClicked();
				}
				break;			
			case Qt::Key_Right:				// Process X/Y ROI Translate
				if (key_evt->modifiers() & Qt::ControlModifier)
				{
					MozakUI* moz = MozakUI::getMozakInstance();
					moz->traslXposClicked();
				}
				break;			
			case Qt::Key_Down:				// Process X/Y ROI Translate
				if (key_evt->modifiers() & Qt::ControlModifier)
				{
					MozakUI* moz = MozakUI::getMozakInstance();
					moz->traslYposClicked();
				}
				break;
            default:
                changeMode(Renderer::defaultSelectMode, true, true);
				break;
		}
	}
	else if (event->type() == (QEvent::Type)7)  //2017-6-9 RZC: deal with the #define KeyRelease in X.h of XWindow
							//QEvent::KeyRelease) // intercept keypress events
	{
		key_evt = (QKeyEvent*)event;
		if (key_evt->isAutoRepeat()) return true; // ignore holding down of key

		int keyReleased = key_evt->key();
		switch (keyReleased)
		{
		case Qt::Key_G:
			changingGrid = false;
			break;
		case Qt::Key_Z:
			view3DWidget->setThickness(1.0);
			break;
			//case Qt::Key_A:
		case Qt::Key_C:
		case Qt::Key_D:
		case Qt::Key_E:
		case Qt::Key_J:
			//case Qt::Key_P:
		case Qt::Key_R:
		case Qt::Key_S:
			// If exiting a mode, change back to default mode
			if (curr_renderer->selectMode != Renderer::defaultSelectMode)
			{
				changeMode(Renderer::defaultSelectMode, true, true);
			}
			break;
		default:
			break;
		}
	}
	else
	{
		// If beginning right click drag, ensure that default mode is chosen before beginning
		if (object == view3DWidget && event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvt = (QMouseEvent*)event;
			if (curr_renderer->selectMode == Renderer::smObject)
			{
				changeMode(Renderer::defaultSelectMode, true, true);
			}
            // If pressing a mouse button with no arrows highlighted, hide the translation arrows
            if (curr_renderer->iPosXTranslateArrowEnabled < 2 && 
                curr_renderer->iNegXTranslateArrowEnabled < 2 &&
                curr_renderer->iPosYTranslateArrowEnabled < 2 &&
                curr_renderer->iNegYTranslateArrowEnabled < 2 &&
                curr_renderer->bShowXYTranslateArrows)
            {
                curr_renderer->bShowXYTranslateArrows = false;
                ((QWidget *)(curr_renderer->widget))->repaint();
            }
		}
#define ___mouse_button_release___
		else if (object == view3DWidget && event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent* mouseEvt = (QMouseEvent*)event;
			if (mouseEvt->button() == Qt::RightButton)
			{
				// Regardless of function performed, when right mouse button is released save the annotations file
				neuronTreeChanged = true;
                QList <LocationSimple> listLoc = view3DWidget->getiDrawExternalParameter()->image4d->listLandmarks;
                if (currentMode == Renderer::smCurveCreate_pointclickAutoZ && listLoc.length() > 0)
                {
                    // set Z based on latest placed point such that it is in the center
                    int newZ = round(listLoc.at(listLoc.length() - 1).z);
                    //qDebug() << "newZ = " << newZ;
                    int zoff = (NUM_POLY_AUTO_Z_PLANES - 1) / 2;
                    if (newZ - zoff < window3D->zcminSlider->minimum() &&
                        newZ + zoff > window3D->zcmaxSlider->maximum())
                    {
                        window3D->zcminSlider->setValue(window3D->zcminSlider->minimum());
				        window3D->zcmaxSlider->setValue(window3D->zcmaxSlider->maximum());
                    }
                    else if (newZ - zoff < window3D->zcminSlider->minimum())
                    {
				        window3D->zcminSlider->setValue(window3D->zcminSlider->minimum());
				        window3D->zcmaxSlider->setValue(window3D->zcminSlider->minimum() + NUM_POLY_AUTO_Z_PLANES - 1);
                    }
                    else if (newZ + zoff > window3D->zcmaxSlider->maximum())
                    {
				        window3D->zcminSlider->setValue(window3D->zcmaxSlider->maximum() - NUM_POLY_AUTO_Z_PLANES + 1);
				        window3D->zcmaxSlider->setValue(window3D->zcmaxSlider->maximum());
                    }
                    else
                    {
				        window3D->zcminSlider->setValue(newZ - zoff);
				        window3D->zcmaxSlider->setValue(newZ + zoff);
                    }
                }
                else if (currentMode == Renderer::smJoinTwoNodes)
                {
                    if (curr_renderer->selectedStartNode > -1 && curr_renderer->highlightedEndNode > -1 &&
                        curr_renderer->listNeuronTree.size() > 0 &&
                        curr_renderer->selectedStartNode < curr_renderer->listNeuronTree.at(0).listNeuron.size() &&
                        curr_renderer->highlightedEndNode < curr_renderer->listNeuronTree.at(0).listNeuron.size() &&
                        curr_renderer->selectedStartNode != curr_renderer->highlightedEndNode)
                    {
                        // Perform connection between selectedStartNode and highlightedEndNode here
                        NeuronSWC start_pt_existing = curr_renderer->listNeuronTree.at(0).listNeuron.at(curr_renderer->selectedStartNode);
                        NeuronSWC end_pt_existing = curr_renderer->listNeuronTree.at(0).listNeuron.at(curr_renderer->highlightedEndNode);
                        
                        vector <XYZ> new_pts;
                        new_pts.push_back(XYZ(start_pt_existing.x, start_pt_existing.y, start_pt_existing.z));
                        new_pts.push_back(XYZ(end_pt_existing.x, end_pt_existing.y, end_pt_existing.z));

                        // Determine type of new segment
                        int new_type = start_pt_existing.type;
                        if (start_pt_existing.type != end_pt_existing.type &&
                            end_pt_existing.type != 0 /*unknown*/)
                        {
                            // For certain conditions, match this segment to end point:
                            // 1) When connecting TO an axon branch
                            // 2) When connecting FROM a soma
                            if (end_pt_existing.type == 2 /*axon*/ ||
                                start_pt_existing.type == 1 /*soma*/)
                            {
                                new_type = end_pt_existing.type;
                            }
                        }

                        int prev_type = curr_renderer->highlightedNodeType;
                        curr_renderer->highlightedNodeType = new_type;
                        if (end_pt_existing.type == 0 /*unknown*/)
                        {
                            change_type_in_seg_of_V_NeuronSWC_list(view3DWidget->getiDrawExternalParameter()->image4d->tracedNeuron, end_pt_existing.seg_id, new_type);
                            // TODO: recurse, any addional segments extending from this seg_id that have unknown type should be typed too
                        }
                        curr_renderer->addCurveSWC(new_pts, 0, 5);//LMG 26/10/2018 Mozak mode 5
                        curr_renderer->vecToNeuronTree(curr_renderer->testNeuronTree, new_pts);
                        curr_renderer->highlightedNodeType = prev_type;
				        changeMode(Renderer::defaultSelectMode, true, true);
                    }
                    else if (curr_renderer->highlightedNode > -1)
                    {
                        curr_renderer->selectedStartNode = curr_renderer->highlightedNode;
                        curr_renderer->highlightedNode = -1;
                    }
				}else if (currentMode == Renderer::smHighlightChildren)
                {
					curr_renderer->highlightedStartNode = curr_renderer->highlightedNode;
					if ( curr_renderer->listNeuronTree.size() > 0 &&
                        curr_renderer->highlightedStartNode < curr_renderer->listNeuronTree.at(0).listNeuron.size())
                    {
                        // identify all children of the highlightedStartNode
                        NeuronSWC start_pt_existing = curr_renderer->listNeuronTree.at(0).listNeuron.at(curr_renderer->highlightedStartNode);
						
						curr_renderer->addToListOfChildSegs(start_pt_existing.seg_id);
						//alter behavior here to allow this mode to continue
				        //changeMode(Renderer::defaultSelectMode, true, true);
                    }
                }
			}
            if (mouseEvt->button() == Qt::LeftButton)
            {
                if (curr_renderer->bShowXYTranslateArrows)
                {
                    // Process X/Y ROI Translate
                    MozakUI* moz = MozakUI::getMozakInstance();
                    if (curr_renderer->iPosXTranslateArrowEnabled == 2)
                    {
                        moz->traslXposClicked();
                    }
                    else if (curr_renderer->iNegXTranslateArrowEnabled == 2)
                    {
                        moz->traslXnegClicked();
                    }
                    else if (curr_renderer->iPosYTranslateArrowEnabled == 2)
                    {
                        moz->traslYposClicked();
                    }
                    else if (curr_renderer->iNegYTranslateArrowEnabled == 2)
                    {
                        moz->traslYnegClicked();
                    }
                }
            }
            if ((mouseEvt->buttons() & Qt::LeftButton) == 0 && (mouseEvt->buttons() & Qt::LeftButton) == 0)
            {
                // If no buttons being pressed now, show translation arrows
                if (!curr_renderer->bShowXYTranslateArrows)
                {
                    curr_renderer->bShowXYTranslateArrows = true;
                    needRepaint = true;
                }
            }
            else if (curr_renderer->iPosXTranslateArrowEnabled < 2 && 
                     curr_renderer->iNegXTranslateArrowEnabled < 2 &&
                     curr_renderer->iPosYTranslateArrowEnabled < 2 &&
                     curr_renderer->iNegYTranslateArrowEnabled < 2)
            {
                // If either mouse button being pressed and no arrows are highlighted, hide arrows
                if (curr_renderer->bShowXYTranslateArrows)
                {
                    curr_renderer->bShowXYTranslateArrows = false;
                    needRepaint = true;
                }
            }
            if (needRepaint)
                ((QWidget *)(curr_renderer->widget))->repaint();
		}
		bool res = teramanager::CViewer::eventFilter(object, event);
#define ___right_mouse_auto_save___
		//if (neuronTreeChanged)	onNeuronEdit(); // ugly //autosave the annotations file when right mouse button released
		return res;
	}
	return false;
}

void Mozak3DView::show()
{
    teramanager::PAnoToolBar::disableNonMozakButtons = true;
    V3dR_GLWidget::disableUndoRedo = true;
    V3dR_GLWidget::skipFormat = true;
	teramanager::CViewer::show();
    window3D->setWindowTitle("Mozak");
    Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    Renderer::defaultSelectMode = Renderer::smCurveTiltedBB_fm_sbbox;
    Renderer_gl1::rightClickMenuDisabled = true;
    curr_renderer->colorByAncestry = true;
    curr_renderer->colorByTypeOnlyMode = false;
    prevNodeSize = curr_renderer->nodeSize;
    prevRootSize = curr_renderer->rootSize;
    curr_renderer->nodeSize = 5;
    curr_renderer->rootSize = 9;
	
    MozakUI* moz = MozakUI::getMozakInstance();
    moz->clearAnnotations();


    itm::PAnoToolBar::instance()->buttonOptions->setMenu(0); // disconnect existing menu
    connect(itm::PAnoToolBar::instance()->buttonOptions, SIGNAL(clicked()), this, SLOT(buttonOptionsClicked()));

	window3D->centralLayout->addWidget(contrastSlider, 1);
	
	//buttonUndo = new QToolButton();
	buttonUndo = new QAction(0);
    buttonUndo->setIcon(QIcon(":/icons/undo.png"));
    buttonUndo->setToolTip("Undo (Ctrl+Z)");
    buttonUndo->setEnabled(false);
    buttonUndo->setShortcut(QKeySequence("Ctrl+Z"));
    itm::PAnoToolBar::instance()->toolBar->addAction(buttonUndo);
    connect(buttonUndo, SIGNAL(triggered()), this, SLOT(buttonUndoClicked()));
    //connect(buttonUndo, SIGNAL(clicked()), this, SLOT(buttonUndoClicked()));
    //itm::PAnoToolBar::instance()->toolBar->insertWidget(0, buttonUndo);
    //buttonRedo = new QToolButton();
    buttonRedo = new QAction(0);
    buttonRedo->setIcon(QIcon(":/icons/redo.png"));
    //buttonRedo->setToolTip("Redo (Ctrl+Shift+Z)");
    //buttonRedo->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    buttonRedo->setToolTip("Redo (Ctrl+Shift+Z or Ctrl+Y)");
    buttonRedo->setShortcuts(QList<QKeySequence>()<<QKeySequence("Ctrl+Shift+Z")<<QKeySequence("Ctrl+Y"));
    buttonRedo->setEnabled(false);
    itm::PAnoToolBar::instance()->toolBar->addAction(buttonRedo);
    connect(buttonRedo, SIGNAL(triggered()), this, SLOT(buttonRedoClicked()));
    //connect(buttonRedo, SIGNAL(clicked()), this, SLOT(buttonRedoClicked()));
    //itm::PAnoToolBar::instance()->toolBar->insertWidget(0, buttonRedo);
    itm::PAnoToolBar::instance()->toolBar->addSeparator();

	invertImageButton = new QToolButton();
	invertImageButton->setIcon(QIcon(":/mozak/icons/invert.png"));
    invertImageButton->setToolTip("Invert image between brightfield/darkfield");
    invertImageButton->setCheckable(true);
    connect(invertImageButton, SIGNAL(toggled(bool)), this, SLOT(invertImageButtonToggled(bool)));

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

    joinButton = new QToolButton();
	joinButton->setIcon(QIcon(":/mozak/icons/join.png"));
    joinButton->setToolTip("Join two nodes by right clicking once to choose the start node and once more to connect to the end node.");
    joinButton->setCheckable(true);
    connect(joinButton, SIGNAL(toggled(bool)), this, SLOT(joinButtonToggled(bool)));

	polyLineButton = new QToolButton();
	polyLineButton->setIcon(QIcon(":/mozak/icons/polyline.png"));
    polyLineButton->setToolTip("Series of right-clicks to define a 3D polyline (Esc to finish)");
    polyLineButton->setCheckable(true);
    connect(polyLineButton, SIGNAL(clicked(bool)), this, SLOT(polyLineButtonToggled(bool))); // this should only emit when the button is clicked, not when the value is changed programatically (e.g. by hotkey, which changes the mode itself)

	polyLineAutoZButton = new QToolButton();
	polyLineAutoZButton->setIcon(QIcon(":/mozak/icons/polyline-autoz.png"));
    polyLineAutoZButton->setToolTip("Series of right-clicks to define a 3D polyline - Auto Z select (Esc to finish)");
    polyLineAutoZButton->setCheckable(true);
    connect(polyLineAutoZButton, SIGNAL(clicked(bool)), this, SLOT(polyLineAutoZButtonToggled(bool)));

	retypeSegmentsButton = new QToolButton();
	retypeSegmentsButton->setIcon(QIcon(":/mozak/icons/retype.png"));
    retypeSegmentsButton->setToolTip("Retype neurons to current type by right click stroke");
    retypeSegmentsButton->setCheckable(true);
    connect(retypeSegmentsButton, SIGNAL(toggled(bool)), this, SLOT(retypeSegmentsButtonToggled(bool)));
	
	splitSegmentButton = new QToolButton();
	splitSegmentButton->setIcon(QIcon(":/mozak/icons/split.png"));
    splitSegmentButton->setToolTip("Split segment into two using right click stroke");
    splitSegmentButton->setCheckable(true);
    connect(splitSegmentButton, SIGNAL(clicked(bool)), this, SLOT(splitSegmentButtonToggled(bool)));
    //connect(splitSegmentButton, SIGNAL(toggled(bool)), this, SLOT(splitSegmentButtonToggled(bool)));

	deleteSegmentsButton = new QToolButton();
	deleteSegmentsButton->setIcon(QIcon(":/mozak/icons/delete-segments.png"));
    deleteSegmentsButton->setToolTip("Delete multiple segments with right click stroke");
    deleteSegmentsButton->setCheckable(true);
    connect(deleteSegmentsButton, SIGNAL(toggled(bool)), this, SLOT(deleteSegmentsButtonToggled(bool)));

	overviewMonitorButton = new QToolButton();
	overviewMonitorButton->setIcon(QIcon(":/mozak/icons/overviewMonitor.png"));
	overviewMonitorButton->setToolTip("Open Overview window");
	overviewMonitorButton->setCheckable(true);
	connect(overviewMonitorButton, SIGNAL(toggled(bool)), this, SLOT(overviewMonitorButtonClicked(bool)));
	
	zLockButton = new QToolButton();
	zLockButton->setIcon(QIcon(":/mozak/icons/z-lock.png"));
	zLockButton->setToolTip("Lock neuron bounding box to image bounding box");
	zLockButton->setCheckable(true);
	connect(zLockButton, SIGNAL(clicked(bool)),this, SLOT(zLockButtonClicked(bool)));


	highlightSubtreeButton = new QToolButton();
	highlightSubtreeButton->setIcon(QIcon(":/mozak/icons/subtree.png")); 
	highlightSubtreeButton->setToolTip("highlight subtree from selected segment");
	highlightSubtreeButton->setCheckable(true);
	connect(highlightSubtreeButton,SIGNAL(clicked(bool)), this, SLOT(highlightSubtreeButtonClicked(bool)));

	zLockLayerSB = new QSpinBox;
	zLockLayerSB->setMinimum(0);
	zLockLayerSB->setMaximum(5);
	zLockLayerSB->setValue(2);
	zLockLayerSB->setToolTip("z-layer for z-cut");
	currZLockLayer = new QLabel();
	currZLockLayer->setText("z layer");
	
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, invertImageButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, extendButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, connectButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, joinButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, polyLineButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
    itm::PAnoToolBar::instance()->toolBar->insertWidget(0, polyLineAutoZButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, retypeSegmentsButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, splitSegmentButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, deleteSegmentsButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, overviewMonitorButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0,zLockButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, highlightSubtreeButton);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, currZLockLayer);
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0,zLockLayerSB);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();



	currTypeLabel = new QLabel();
	updateTypeLabel();
	currTypeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	currTypeLabel->setStyleSheet("QLabel { font-size:14px; color:black; background-color:rgba(0,0,0,50)}");
	currTypeLabel->setToolTip("Press 0-7 key to change type of subsequent neuron traces");
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, currTypeLabel);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
	
	currZoomLabel = new QLabel();
	updateZoomLabel(1);
	currZoomLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	currZoomLabel->setStyleSheet("QLabel { font-size:14px; color:black; background-color:rgba(0,0,0,50)}");
	currZoomLabel->setToolTip("Current zoom level (camera). Use mouse scroll wheel to zoom in/out");
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, currZoomLabel);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();

	currResolutionLabel = new QLabel();
	updateResolutionLabel();
	currResolutionLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	currResolutionLabel->setStyleSheet("QLabel { font-size:14px; color:black; background-color:rgba(0,0,0,50)}");
	currResolutionLabel->setToolTip("Current resolution level (Terafly). Double left click to increase, double right click to decrease.");
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, currResolutionLabel);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();
    
    currUndoLabel = new QLabel();
	updateUndoLabel();
	currUndoLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	currUndoLabel->setStyleSheet("QLabel { font-size:14px; color:black; background-color:rgba(0,0,0,50)}");
	currUndoLabel->setToolTip("Current edit history. Press Ctrl+Z to undo and Ctrl+Y to redo to move through history.");
	itm::PAnoToolBar::instance()->toolBar->insertWidget(0, currUndoLabel);
	itm::PAnoToolBar::instance()->toolBar->addSeparator();

	itm::PAnoToolBar::instance()->refreshTools();
	

#define ___panotoobar_icon_size____
   // itm::PAnoToolBar::instance()->toolBar->setIconSize(QSize(25,20));    //20170803 RZC: make the buttons at toolbar bottom visible

    changeMode(Renderer::smObject, false, false);   //20170804 RZC: set toolbar initail mode


	QObject::connect(view3DWidget, SIGNAL(zoomChanged(int)), dynamic_cast<QObject *>(this), SLOT(updateZoomLabel(int)));
	QObject::connect(window3D->zcminSlider, SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(setZSurfaceLimitValues(int)));
	QObject::connect(window3D->zcmaxSlider, SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(setZSurfaceLimitValues(int)));

	updateTranslateXYArrows();
	updateRendererParams();
	overviewTimer = new QTimer;
	connect(overviewTimer, SIGNAL(timeout()), this,SLOT(overviewSyncOneShot()));// SLOT(timerupdate()));
	overviewActive = false;

	//20170803 RZC: no use, make inactive
//	appendHistory();
//	makeTracedNeuronsEditable();

    paint_timer->start();
}

//The input are integers in [0, TOTAL_WRIGGLE_FRAMES)
//The outputs are degrees to rotate around the axis, in radians
GLdouble Mozak3DView::wriggleDegreeFunction(int index){
    GLdouble degreeToRadian = 3.141569 / 180;
    GLdouble maxWriggleRadian = 45;
    maxWriggleRadian *= degreeToRadian;
    GLdouble input = (GLdouble(index) / (total_wriggle_frames - 1)) * 3.141569; //from 0 to 3.141569
    GLdouble output = maxWriggleRadian * sin(input * 2);
    return output;
}

void Mozak3DView::wriggleTimerCall()
{


    if((int) currentWriggleFrame < (int) total_wriggle_frames){

        Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
        GLdouble u, v, w, a, b, c, theta = wriggleDegreeFunction(currentWriggleFrame);
        XYZ normalizedUVW = XYZ(curr_renderer->rotateAxisBeginNode - curr_renderer->rotateAxisEndNode);
        normalize(normalizedUVW);
        a = curr_renderer->rotateAxisBeginNode.x;
        b = curr_renderer->rotateAxisBeginNode.y;
        c = curr_renderer->rotateAxisBeginNode.z;
        u = normalizedUVW.x;
        v = -normalizedUVW.y;
        w = normalizedUVW.z;
        if(u == 0 && v == 0 && w ==0){
            return;
        }
        GLdouble fixedRadianRotation[16], newMRot[16]; //Construct a new rotation from scratch, this only rotates the identity
        //Diagonals
        fixedRadianRotation[0] = (u * u) + (v * v + w * w) * cos(theta);
        fixedRadianRotation[5] = (v * v) + (u * u + w * w) * cos(theta);
        fixedRadianRotation[10] = (w * w) + (u * u + v * v) * cos(theta);

        //Non-diagonal non-homogenous
        fixedRadianRotation[1] = (u * v) * (1 - cos(theta)) - w * sin(theta);
        fixedRadianRotation[2] = (u * w) * (1 - cos(theta)) + v * sin(theta);
        fixedRadianRotation[4] = (u * v) * (1 - cos(theta)) + w * sin(theta);
        fixedRadianRotation[6] = (v * w) * (1 - cos(theta)) - u * sin(theta);
        fixedRadianRotation[8] = (u * w) * (1 - cos(theta)) - v * sin(theta);
        fixedRadianRotation[9] = (v * w) * (1 - cos(theta)) + u * sin(theta);

        //Homogenous
        fixedRadianRotation[3] = (a * (v * v + w * w) - u * (b * v + c * w)) * (1 - cos(theta)) + (b * w - c * v) * sin(theta);
        fixedRadianRotation[7] = (c * (u * u + w * w) - v * (a * u + c * w)) * (1 - cos(theta)) + (c * u - a * w) * sin(theta);
        fixedRadianRotation[11] = (c * (u * u + v * v) - w * (a * u + b * c)) * (1 - cos(theta)) + (a * v - b * u) * sin(theta);

        fixedRadianRotation[12] = fixedRadianRotation[13] = fixedRadianRotation[14] = 0;
        fixedRadianRotation[15] = 1;

        for(int i = 0; i < 16; i++){
            newMRot[i] = 0;
        }

        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                for(int x = 0; x < 4; x++){
                    newMRot[i * 4 + j] += fixedRadianRotation[i * 4 + x] * originalRotationMatrix[x * 4 + j];
                }
            }
        }

        for(int i = 0; i < 16; i++){
            view3DWidget->mRot[i] = newMRot[i];
        }

        view3DWidget->paintGL();
        ((QWidget *)(curr_renderer->widget))->repaint();
        currentWriggleFrame++;
    }else{
        wriggle_timer->stop();
        isWriggling = false;
        currentWriggleFrame = 0;
    }
}

void Mozak3DView::paintTimerCall()
{
    Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    if (!curr_renderer) return;

	//makeTracedNeuronsEditable();   //20170804 RZC

    if (curr_renderer->loopSegs.length() == 0) return;

    // Draw blinking effect if loop detection has found loops in current reconstruction
    curr_renderer->drawNeuronTreeList();
    curr_renderer->drawObj();
    ((QWidget *)(curr_renderer->widget))->repaint();
}

Mozak3DView::~Mozak3DView()
{
    window3D->setWindowTitle("");
    Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    Renderer::defaultSelectMode = Renderer::smObject;
    Renderer_gl1::rightClickMenuDisabled = false;
    curr_renderer->colorByAncestry = false;
    V3dR_GLWidget::disableUndoRedo = false;
    V3dR_GLWidget::skipFormat = false;
    teramanager::PAnoToolBar::disableNonMozakButtons = false;
    curr_renderer->nodeSize = prevNodeSize;
    curr_renderer->rootSize = prevRootSize;
    MozakUI* moz = MozakUI::getMozakInstance();
    moz->close();
}

void Mozak3DView::storeAnnotations() throw (itm::RuntimeException)
{
    teramanager::CViewer::storeAnnotations();
}

void Mozak3DView::loadAnnotations() throw (itm::RuntimeException)
{
    teramanager::CViewer::loadAnnotations();
    appendHistory();

    //20170804 RZC: no more cash after fixed the crash of ~CViewer()
    makeTracedNeuronsEditable();          //20170803 RZC  //crash error message: double free or corruption

}


void Mozak3DView::clearAnnotations() throw (itm::RuntimeException)
{
    teramanager::CViewer::clearAnnotations();
    appendHistory();                     //20170803 RZC
}


const char *typeNamesRow1[] = { "??", "soma", "axon", "dendrite", "apical", "fork", "end", "FixIt!", "FixIt!", "FixIt! " };
const char *typeNamesRow2[] = { " ", " ", " ", " ", "dendrite", "point", "point", "Axon", "dendrite", " ?? " };

void Mozak3DView::updateTypeLabel() // TODO: make any type changes emit a SIGNAL that this SLOT could listen to
{
	int initialTraceType = 3;
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	if (curr_renderer) {
		if (curr_renderer->highlightedNodeType >= 0)
			initialTraceType = curr_renderer->highlightedNodeType;
		else
			initialTraceType = curr_renderer->currentTraceType;
	}
	if (currTypeLabel){
		currTypeLabel->setText(QString("Type:\n").append(typeNamesRow1[initialTraceType]).append("\n").append(typeNamesRow2[initialTraceType]));
	}
}

void Mozak3DView::updateZoomLabel(int zr)
{
	// ignore value given and get directly from the renderer
	float zoom = 100.0f;
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	if (curr_renderer)
	{
		if (curr_renderer->zoomRatio != 0) // div by zero
			zoom = 100.0f * (1.0f / curr_renderer->zoomRatio);
		else
			zoom = 2599.0f;
	}
	if (currZoomLabel)
		currZoomLabel->setText(itm::strprintf("%d%s", (int)zoom, "%").c_str());
}

void Mozak3DView::updateResolutionLabel()
{
	int maxRes = itm::CImport::instance()->getResolutions();
	if (currResolutionLabel)
		currResolutionLabel->setText(itm::strprintf("RES %d/%d", (volResIndex+1), maxRes).c_str());
}

void Mozak3DView::buttonOptionsClicked()
{
    if (view3DWidget)
        view3DWidget->changeLineOption();
}

void Mozak3DView::buttonUndoClicked()
{
    performUndo();
}

void Mozak3DView::buttonRedoClicked()
{
    performRedo();
}

void Mozak3DView::updateUndoLabel()
{
	if (currUndoLabel)
		currUndoLabel->setText(itm::strprintf("Hist %d/%d", cur_history+1, undoRedoHistory.size()).c_str());
    if (buttonUndo
    		&& buttonRedo) //20170729 RZC
    {
        buttonUndo->setEnabled(undoRedoHistory.size() > 0 && cur_history > 0);
        buttonRedo->setEnabled(undoRedoHistory.size() > 0 && cur_history > -1 && cur_history < undoRedoHistory.size() - 1);
    }
}

void Mozak3DView::updateTranslateXYArrows()
{
    Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
    itm::CVolume* cVolume = itm::CVolume::instance();
    IconImageManager::VirtualVolume *currentVolume = itm::CImport::instance()->getVolume(volResIndex);
    
    if (cVolume->getVoiResIndex() == -1) // not initialized
        return;

    int currentVoiX0 = cVolume->getVoiH0();
    int currentVoiX1 = cVolume->getVoiH1();
    int maxX = currentVolume->getDIM_H();

    int currentVoiY0 = cVolume->getVoiV0();
    int currentVoiY1 = cVolume->getVoiV1();
    int maxY = currentVolume->getDIM_V();

    // Not used (no z-arrows to avoid interfering with views):
    int currentVoiZ0 = cVolume->getVoiD0();
    int currentVoiZ1 = cVolume->getVoiD1();
    int maxZ = currentVolume->getDIM_D();

    curr_renderer->iPosXTranslateArrowEnabled = (currentVoiX1 >= 0 && currentVoiX1 < maxX) ? 1 : 0;
    curr_renderer->iNegXTranslateArrowEnabled = (currentVoiX0 > 0) ? 1 : 0;
    curr_renderer->iPosYTranslateArrowEnabled = (currentVoiY1 >= 0 && currentVoiY1 < maxY) ? 1 : 0;
    curr_renderer->iNegYTranslateArrowEnabled = (currentVoiY0 > 0) ? 1 : 0;
    curr_renderer->paint();
}

void Mozak3DView::updateRendererParams()
{
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	if (! curr_renderer) return; //20170804 RZC

	curr_renderer->ui3dviewMode = Renderer::Mozak;  //20170804 RZC

	if (curr_renderer->tryTexCompress || 
		curr_renderer->tryTexStream != -1 || 
		!curr_renderer->tryTexNPT ||
		curr_renderer->bShowAxes ||
        !curr_renderer->bShowXYTranslateArrows ||
        !curr_renderer->useCurrentTraceTypeForRetyping)
	{
		curr_renderer->tryTexCompress = false;
		curr_renderer->tryTexStream = -1;
		curr_renderer->tryTexNPT = true;
		curr_renderer->bShowAxes = false;
        curr_renderer->bShowXYTranslateArrows = true;
        curr_renderer->useCurrentTraceTypeForRetyping = true;
		view3DWidget->updateImageData();
	}
}

void Mozak3DView::invertImageButtonToggled(bool checked)
{
	window3D->dispType_maxip->setChecked(!checked);
	window3D->dispType_minip->setChecked(checked);
	contrastSlider->setValue(-contrastValue);
	if (contrastValue == 0) updateContrast(0); // won't trigger automatically because value was not changed
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
		int val;
		if (invertImageButton->isChecked())
			val = (int)(pow((255-j)/255.0f, exp_val) * 255.0f);
		else
			val = (int)(pow(j/255.0f, exp_val) * 255.0f);
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

void Mozak3DView::joinButtonToggled(bool checked)
{
    changeMode(Renderer::smJoinTwoNodes, true, checked);
}

void Mozak3DView::polyLineButtonToggled(bool checked)
{
	changeMode(Renderer::smCurveCreate_pointclick, true, checked);
}

void Mozak3DView::polyLineAutoZButtonToggled(bool checked)
{
	changeMode(Renderer::smCurveCreate_pointclickAutoZ, true, checked);
}

void Mozak3DView::retypeSegmentsButtonToggled(bool checked)
{
	changeMode(Renderer::smRetypeMultiNeurons, true, checked);
}

void Mozak3DView::splitSegmentButtonToggled(bool checked)
{
	qDebug()<<"Mozak3DView::splitSegmentButtonToggled "<<checked;
	changeMode(Renderer::smBreakTwoNeurons, false, checked);
}

void Mozak3DView::deleteSegmentsButtonToggled(bool checked)
{
	changeMode(Renderer::smDeleteMultiNeurons, false, checked);
}
void Mozak3DView::zLockButtonClicked(bool checked){


	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	curr_renderer->setBBZcutFlag(checked);

	window3D->zcminSlider->setValue(window3D->zcminSlider->value());
	window3D->zcmaxSlider->setValue(window3D->zcmaxSlider->value());
	
}

void Mozak3DView::setZSurfaceLimitValues(int ignore){
	if (zLockButton->isChecked()){
		Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());

		curr_renderer->setBBZ((float) window3D->zcminSlider->value()-zLockLayerSB->value(), (float) window3D->zcmaxSlider->value()+zLockLayerSB->value());
	}
}
void Mozak3DView::HideAll3DView()
{
	cout<<"come in";
	MozakUI* moz = MozakUI::getMozakInstance();
	cout<<"getMozakInstance";
	if(!moz->V3D_env) cout<<"V3D_env is nullptr"<<endl;
	QList<V3dR_MainWindow*> windowList =moz->V3D_env->getListAll3DViewers();
	cout<<"windowListsize"<<windowList.length();
	for (V3DLONG i=0;i<windowList.length();i++)
    {
		cout<<"step"<<i<<endl;
		windowList[i]->hide();
    }
}
void Mozak3DView::overviewMonitorButtonClicked(bool checked){
	// test mozak autosave path:

	MozakUI* moz = MozakUI::getMozakInstance();


	qDebug()<<"overview checked? "<< checked;
	QList<V3dR_MainWindow*> windowList =	moz->V3D_env->getListAll3DViewers();
	if (windowList.length()==1){ //there's only one window open: Mozak!
		V3dR_MainWindow* overviewWindow = moz->V3D_env->createEmpty3DViewer();
		moz->V3D_env->setWindowDataTitle(overviewWindow, "Overview");
		Renderer_gl2* overview_renderer = (Renderer_gl2*)(overviewWindow->getGLWidget()->getRenderer());
		overview_renderer->colorByAncestry = true;
		overview_renderer->setColorAncestryInfo();
		overview_renderer->neuronColorMode = neuronColorMode;
		overviewTimer->setInterval(1500);
		overviewTimer->start();
		overviewActive = true;
	 	return;
	}
	if (overviewActive){
		overviewTimer->stop();
		overviewActive = false;
	}else{
		overviewTimer->setInterval(1500);
		overviewTimer->start();
		overviewActive = true;
	}
}


void Mozak3DView::overviewSyncOneShot(){



	if (overviewActive){

		qDebug()<<"overview active";
		MozakUI* moz = MozakUI::getMozakInstance();


			QList<NeuronTree> *overview_nt_list = moz->V3D_env->getHandleNeuronTrees_Any3DViewer(moz->V3D_env->find3DViewerByName("Overview"));

				NeuronTree testLoad;
				qDebug()<<"autosave.ano.swc loading!";
				QFileInfo fInfo = QFileInfo("./autosave.ano.swc");
				if (fInfo.isReadable()){
				 testLoad= readSWC_file( "./autosave.ano.swc");// "D:\\mozak_git\\v3d_main\\v3d\\release\\autosave.ano.swc"  );
				 qDebug()<<"testLoad ORIGINAL  info : "<< testLoad.n<<" "<<testLoad.listNeuron.length();
				 testLoad.editable = true;
				 V_NeuronSWC_list tList = NeuronTree__2__V_NeuronSWC_list(testLoad);
				 //tList.decompose(); 
				 //tList.merge();
				 testLoad = V_NeuronSWC_list__2__NeuronTree(tList);
				 qDebug()<<"testLoad info : "<< testLoad.n<<" "<<testLoad.listNeuron.length();
				 
				}else{
					return;}

				if ( !(moz->V3D_env->find3DViewerByName("Overview")==0)){  // formerly checked for  (testLoad.n>0) &&   but V_NeuronSWC_list_2_neuronTree sets NeuronTree.n=-1 for output. 
					moz->V3D_env->getHandleNeuronTrees_Any3DViewer(moz->V3D_env->find3DViewerByName("Overview"))->clear();
					moz->V3D_env->getHandleNeuronTrees_Any3DViewer(moz->V3D_env->find3DViewerByName("Overview"))->push_back(testLoad);

					Renderer_gl2* overview_renderer = (Renderer_gl2*)(moz->V3D_env->find3DViewerByName("Overview")->getGLWidget()->getRenderer());
					int sz = overview_renderer->listNeuronTree.size();
						for (int i=0; i<sz; i++)
							{
								overview_renderer->listNeuronTree[i].editable = true;
							}
			
					overview_renderer->setColorAncestryInfo();
					overview_renderer->neuronColorMode = neuronColorMode;

				}
              

			if (moz->V3D_env->find3DViewerByName("Overview")){

					moz->V3D_env->update_NeuronBoundingBox(moz->V3D_env->find3DViewerByName("Overview"));
				}
			if (moz->V3D_env->find3DViewerByName("Overview")) {

				moz->V3D_env->update_3DViewer(moz->V3D_env->find3DViewerByName("Overview"));

				}else{
				overviewTimer->stop();
				overviewActive = false;
				return;}
			

			
		}else{
			//add handling here
			qDebug()<<"overview off";
			return;}
	
}


void Mozak3DView::updateColorMode(int colorMode){

		Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
		curr_renderer->neuronColorMode = colorMode;
		if (overviewActive){
		MozakUI* moz = MozakUI::getMozakInstance();
		Renderer_gl2* overview_renderer = (Renderer_gl2*)(moz->V3D_env->find3DViewerByName("Overview")->getGLWidget()->getRenderer());
		overview_renderer->setColorAncestryInfo();

		overview_renderer->neuronColorMode = colorMode;
		}
}

void Mozak3DView::highlightSubtreeButtonClicked(bool checked){
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	curr_renderer->childHighlightMode = checked;
	changeMode(Renderer::smHighlightChildren, false, checked);
	qDebug()<<"highlightSubtreeButtonClicked!";
}
void Mozak3DView::changeMode(Renderer::SelectMode mode, bool addThisCurve, bool turnOn)
{
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	Renderer::SelectMode prevMode = curr_renderer->selectMode;
    int zoff = (mode == Renderer::smCurveCreate_pointclick) ? 0 : ((NUM_POLY_AUTO_Z_PLANES - 1) / 2);
	if (turnOn)
	{
        if (prevMode == mode) // no action needed, mode already on
            return;
		// Uncheck any other currently checked modes
		if (mode != Renderer::smCurveEditExtendOneNode && extendButton->isChecked())
			extendButton->setChecked(false);
		if (mode != Renderer::smCurveEditExtendTwoNode && connectButton->isChecked())
			connectButton->setChecked(false);
        if (mode != Renderer::smJoinTwoNodes && joinButton->isChecked())
			joinButton->setChecked(false);
		if (mode != Renderer::smCurveCreate_pointclick && polyLineButton->isChecked())
			polyLineButton->setChecked(false);
        if (mode != Renderer::smCurveCreate_pointclickAutoZ && polyLineAutoZButton->isChecked())
			polyLineAutoZButton->setChecked(false);
		if (mode != Renderer::smBreakTwoNeurons && splitSegmentButton->isChecked())
			splitSegmentButton->setChecked(false);
		if (mode != Renderer::smDeleteMultiNeurons && deleteSegmentsButton->isChecked())
			deleteSegmentsButton->setChecked(false);
        if (mode != Renderer::smRetypeMultiNeurons && retypeSegmentsButton->isChecked())
            retypeSegmentsButton->setChecked(false);
		if (mode != Renderer::smHighlightChildren && highlightSubtreeButton->isChecked())
			highlightSubtreeButton->setChecked(false);

        /////curr_renderer->endSelectMode(); //170804 RZC: this make hold pressed key cannot work
        curr_renderer->selectMode = mode;
        curr_renderer->b_addthiscurve = addThisCurve;
        switch (mode)
		{
			case Renderer::smCurveCreate_pointclick:
            case Renderer::smCurveCreate_pointclickAutoZ:
			{

				curr_renderer->cntCur3DCurveMarkers=0; //reset
				curr_renderer->oldCursor = view3DWidget->cursor();
				view3DWidget->setCursor(QCursor(Qt::CrossCursor));
				
				// When entering polyline mode, start restriction to single z-plane and allow mouse wheel z-scroll
				if ( prevMode != Renderer::smCurveCreate_pointclick && prevMode!= Renderer::smCurveCreate_pointclickAutoZ){
				prevZCutMin = window3D->zcminSlider->value();
				prevZCutMax = window3D->zcmaxSlider->value();
				}
				// Use previously-scrolled polyline z-cut if within bounds, otherwise use midpoint of current bounds
				int centZ = ((window3D->zcmaxSlider->value() + window3D->zcminSlider->value()) / 2);
                if (prevPolyZCut - zoff >= prevZCutMin && prevPolyZCut + zoff <= prevZCutMax)
					centZ = prevPolyZCut;
				// TODO: use max intensity of ray from current mouse projection to get z-plane instead of midVal
				window3D->zcminSlider->setValue(centZ - zoff);
				window3D->zcmaxSlider->setValue(centZ + zoff-1);
				if (zLockButton->isChecked()) curr_renderer->setBBZcutFlag(true);
				
			}
			break;
            case Renderer::smJoinTwoNodes:
            {
                curr_renderer->selectedStartNode = -1;
                curr_renderer->highlightedEndNode = -1;
            }
            break;
			case Renderer::smHighlightChildren:
				{
					curr_renderer->highlightedStartNode = -1;
					qDebug()<<"set mode smHighlightChildren";
				}
				break;									
			default:
				break;
		}
	}
	else
	{

        curr_renderer->endSelectMode();
        curr_renderer->highlightedNodeType = -1;
		curr_renderer->highlightedStartNode = -1;
        updateTypeLabel();
		makeTracedNeuronsEditable();
        if (prevMode == Renderer::defaultSelectMode)
            return;
		curr_renderer->selectMode = Renderer::defaultSelectMode;
		curr_renderer->b_addthiscurve = true;
	}
	if ((prevMode == Renderer::smCurveCreate_pointclick || prevMode == Renderer::smCurveCreate_pointclickAutoZ) && 
        curr_renderer->selectMode != prevMode)
	{
		// When exiting poly mode, restore all z cuts
		curr_renderer->setBBZcutFlag(zLockButton->isChecked());
		prevPolyZCut = (window3D->zcmaxSlider->value() + window3D->zcminSlider->value()) / 2;
        window3D->zcminSlider->setValue((prevZCutMin > -1) ? prevZCutMin : window3D->zcminSlider->minimum());
		window3D->zcmaxSlider->setValue((prevZCutMax > -1) ? prevZCutMax : window3D->zcmaxSlider->maximum());
	}
}

void Mozak3DView::updateGrid(){

	QList<ImageMarker> tileGridLocs;
	QList<long> gridNumbers;
	tileGridLocs.clear();
	for (int i =0; i<allGridLocs.length(); i++){
		for (int j = 0; j<allGridLocs.at(i).length(); j++){
		if (allGridLocs.at(i).at(j).x >= anoH0-gridSpacing && allGridLocs.at(i).at(j).x <= anoH1 && 
			allGridLocs.at(i).at(j).y >= anoV0-gridSpacing && allGridLocs.at(i).at(j).y <= anoV1) {
				tileGridLocs.append(allGridLocs.at(i).at(j));
				gridNumbers.append((long)(i*gridSpacing+j));
		}
		}
	
	}

	for (int k =0; k<tileGridLocs.length(); k++){
		tileGridLocs[k].x = 	getLocalHCoord(tileGridLocs.at(k).x);
		tileGridLocs[k].y = 	getLocalVCoord(tileGridLocs.at(k).y);
		tileGridLocs[k].z = 	getLocalDCoord(tileGridLocs.at(k).z);
	}
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
 float	localSpacing = getLocalHCoord((float)gridSpacing) - getLocalHCoord(0.0);
	curr_renderer->setLocalGrid(tileGridLocs,gridNumbers,localSpacing);

	if (this->overviewActive){
		MozakUI* moz = MozakUI::getMozakInstance();
		Renderer_gl1* overviewRenderer =(Renderer_gl1*)( moz->V3D_env->find3DViewerByName("Overview")->getGLWidget()->getRenderer());
		overviewRenderer->setLocalGrid(tileGridLocs,gridNumbers,localSpacing);
	}
	// set these new values as the values in the renderer
}

void Mozak3DView::setGrid(int spacing){
	if (spacing>=minGridSpacing)	gridSpacing = spacing; //otherwise ignore the input value.  
	allGridLocs.clear();
	for (int i = 0; i< 100; i++){
		QList<ImageMarker> iList;
		for (int j = 0; j<100; j++){
			iList.append(ImageMarker((float)i*(float)gridSpacing,(float)j*(float)gridSpacing, 0.0));
		}
		allGridLocs.append(iList);
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
    int prev_line_width = ((Renderer_gl2*)(view3DWidget->getRenderer()))->lineWidth;
	int prev_node_size = ((Renderer_gl2*)(view3DWidget->getRenderer()))->nodeSize;
	int prev_root_size = ((Renderer_gl2*)(view3DWidget->getRenderer()))->rootSize;

    // Store current z-cuts and attempt to restore them if valid
    prevZCutMin = window3D->zcminSlider->value();
    prevZCutMax = window3D->zcmaxSlider->value();

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
	updateResolutionLabel();
	imgData = _img->getRawData();

	// exit from "waiting for 5D data" state, if previously set
	if(this->waitingFor5D)
		this->setWaitingFor5D(false);

    QList <V_NeuronSWC_list> prevHist = view3DWidget->getiDrawExternalParameter()->image4d->tracedNeuron_historylist;

    V3D_env->setImage(window, _img); // this clears the rawDataPointer for _img

    // create 3D view window with postponed show()
	XFormWidget *w = V3dApplication::getMainWindow()->validateImageWindow(window);
	view3DWidget->getiDrawExternalParameter()->image4d = w->getImageData();

    QList <V_NeuronSWC_list> newHist = view3DWidget->getiDrawExternalParameter()->image4d->tracedNeuron_historylist;

	// Make sure to call updateImageData AFTER getiDrawExternalParameter's image4d is
	// set above as this is the data being updated.
	view3DWidget->updateImageData();
    
	// update z-cuts for new view resolution.
    float ratio = ((float)itm::CImport::instance()->getVolume(volResIndex)->getDIM_D())/
		((float)itm::CImport::instance()->getVolume(prevRes)->getDIM_D());

	// scale z-cut minimum and maximum from previous-resolution voxels to new-resolution voxels... (for ratios < 1 it may not be 
	// possible to have a z-cut which displays the same data as the previous z-cut; err on the side of too-wide cuts rather than
	// too-narrow.)
	float newZCutMin = floor((float)prevZCutMin * ratio),
		newZCutMax = floor(((float)prevZCutMax + 1) * ratio) - 1;

	// clamp cuts to VOI boundaries...
	if (newZCutMin < window3D->zcminSlider->minimum())
	{
		newZCutMin = window3D->zcminSlider->minimum();
	}

	if (newZCutMax > window3D->zcmaxSlider->maximum())
	{
		newZCutMax = window3D->zcmaxSlider->maximum();
	}

	// ... and push
    window3D->zcminSlider->setValue(newZCutMin);
    window3D->zcmaxSlider->setValue(newZCutMax);

	itm::CViewer::loadAnnotations();


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
    moz->statusBar->showMessage("Ready.");

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

    ((Renderer_gl2*)(view3DWidget->getRenderer()))->lineWidth = prev_line_width;
	((Renderer_gl2*)(view3DWidget->getRenderer()))->nodeSize = prev_node_size;
	((Renderer_gl2*)(view3DWidget->getRenderer()))->rootSize = prev_root_size;
	updateGrid();
    updateTranslateXYArrows();
	updateRendererParams();
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

		IconImageManager::VirtualVolume *currentVolume = itm::CImport::instance()->getVolume(volResIndex), 
			*newVolume = itm::CImport::instance()->getVolume(resolution);

        // scale VOI coordinates to the reference system of the target resolution
        if(scale_coords)
        {
            float ratioX = static_cast<float>(newVolume->getDIM_H())/currentVolume->getDIM_H();
            float ratioY = static_cast<float>(newVolume->getDIM_V())/currentVolume->getDIM_V();
            float ratioZ = static_cast<float>(newVolume->getDIM_D())/currentVolume->getDIM_D();
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

		int final_x0, final_x1, final_y0, final_y1, final_z0, final_z1, final_t0, final_t1;

        // crop VOI if its larger than the maximum allowed
        if(auto_crop)
        {
            // modality #1: VOI = [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz), [t0, t1]
            if(dx != -1 && dy != -1 && dz != -1)
			{
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, cropping bbox dims from (%d,%d,%d) t[%d,%d] to...", titleShort.c_str(),  dx, dy, dz, t0, t1).c_str(), __itm__current__function__);
				
				// CB: adaptive VOI size, full-depth slices
				int maxVoxels = moz->Hdim_sbox->value() * moz->Vdim_sbox->value() * moz->Ddim_sbox->value();
				float xyArea = (float)maxVoxels / newVolume->getDIM_D();

				if (newVolume->getDIM_H() * newVolume->getDIM_V() > xyArea)
				{
					// not enough voxels to do a full-image view; make it a square
					float width, height;
					width = height = sqrt(xyArea);

					// clip the VOI size below by the maximum dimensions box
					width = std::max(width, (float)moz->Hdim_sbox->value());
					height = std::max(height, (float)moz->Vdim_sbox->value());

					// adjust X coordinate
					if ((x - width / 2.0f) < 0)
					{
						// square's off the left edge
						final_x0 = 0;
						final_x1 = itm::round(width);
					} 
					else if ((x + width / 2.0f) > newVolume->getDIM_H())
					{
						// square's off the right edge
						final_x1 = newVolume->getDIM_H();
						final_x0 = newVolume->getDIM_H() - itm::round(width);
					}
					else
					{
						// center square at click point
						final_x0 = x - itm::round(width / 2.0f);
						final_x1 = x + itm::round(width / 2.0f);
					}

					// adjust Y coordinate
					if ((y - height / 2.0f) < 0)
					{
						// square's off the top edge
						final_y0 = 0;
						final_y1 = itm::round(height);
					} 
					else if ((y + height / 2.0f) > newVolume->getDIM_V())
					{
						// square's off the bottom edge
						final_y1 = newVolume->getDIM_V();
						final_y0 = newVolume->getDIM_V() - itm::round(height);
					}
					else
					{
						// center square at click point
						final_y0 = y - itm::round(height / 2.0f);
						final_y1 = y + itm::round(height / 2.0f);
					}

					final_z0 = 0;
					final_z1 = newVolume->getDIM_D();
				}
				else
				{
					// enough voxels to do a full-image view
					final_x0 = 0;
					final_x1 = newVolume->getDIM_H();

					final_y0 = 0;
					final_y1 = newVolume->getDIM_V();

					final_z0 = 0;
					final_z1 = newVolume->getDIM_D();
				}

                t0 = std::max(0, std::min(t0,currentVolume->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,currentVolume->getDIM_T()-1));
                if(itm::CImport::instance()->is5D() && (t1-t0+1 > moz->Tdim_sbox->value()))
                    t1 = t0 + moz->Tdim_sbox->value();
                if(itm::CImport::instance()->is5D() && (t1 >= itm::CImport::instance()->getTDim()-1))
                    t0 = t1 - (moz->Tdim_sbox->value()-1);
                if(itm::CImport::instance()->is5D() && (t0 == 0))
                    t1 = moz->Tdim_sbox->value()-1;

				final_t0 = t0; final_t1 = t1;

                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, ...to (%d,%d,%d)", titleShort.c_str(),  dx, dy, dz).c_str(), __itm__current__function__);
            }
            // modality #2: VOI = [x0, x), [y0, y), [z0, z), [t0, t1]
            else
            {
                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, cropping bbox dims from [%d,%d) [%d,%d) [%d,%d) [%d,%d] to...", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
                if(x - x0 > moz->Hdim_sbox->value())
                {
                    float margin = ( (x - x0) - moz->Hdim_sbox->value() )/2.0f ;
                    final_x1 = round(x  - margin);
                    final_x0 = round(x0 + margin);
                }
                if(y - y0 > moz->Vdim_sbox->value())
                {
                    float margin = ( (y - y0) - moz->Vdim_sbox->value() )/2.0f ;
                    final_y1 = round(y  - margin);
                    final_y0 = round(y0 + margin);
                }
                if(z - z0 > moz->Ddim_sbox->value())
                {
                    float margin = ( (z - z0) - moz->Ddim_sbox->value() )/2.0f ;
                    final_z1 = round(z  - margin);
                    final_z0 = round(z0 + margin);
                }
                t0 = std::max(0, std::min(t0,currentVolume->getDIM_T()-1));
                t1 = std::max(0, std::min(t1,currentVolume->getDIM_T()-1));
                if(itm::CImport::instance()->is5D() && (t1-t0+1 > moz->Tdim_sbox->value()))
                    t1 = t0 + moz->Tdim_sbox->value();
                if(itm::CImport::instance()->is5D() && (t1 >= itm::CImport::instance()->getTDim()-1))
                    t0 = t1 - (moz->Tdim_sbox->value()-1);
                if(itm::CImport::instance()->is5D() && (t0 == 0))
                    t1 = moz->Tdim_sbox->value()-1;

				final_t0 = t0; final_t1 = t1;

                /**/itm::debug(itm::LEV3, itm::strprintf("title = %s, ...to [%d,%d) [%d,%d) [%d,%d) [%d,%d]", titleShort.c_str(),  x0, x, y0, y, z0, z, t0, t1).c_str(), __itm__current__function__);
            }
			//qDebug() << itm::strprintf("\n  After auto_crop:\ntitle = %s, x = %d, y = %d, z = %d, res = %d, dx = %d, dy = %d, dz = %d, x0 = %d, y0 = %d, z0 = %d, t0 = %d, t1 = %d, auto_crop = %s, scale_coords = %s, sliding_viewer_block_ID = %d",
			//												titleShort.c_str(),  x, y, z, resolution, dx, dy, dz, x0, y0, z0, t0, t1, auto_crop ? "true" : "false", scale_coords ? "true" : "false", sliding_viewer_block_ID).c_str();
        }

        // ask CVolume to check (and correct) for a valid VOI
        itm::CVolume* cVolume = itm::CVolume::instance();
        try
        {
			cVolume->setVoi(0, resolution, final_y0, final_y1, final_x0, final_x1, final_z0, final_z1, final_t0, final_t1);
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
