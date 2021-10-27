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
** 2006-08-09
last update: 060903
2006-09-21: update the about info to v0.13
2007-03-27: update the about info to v0.14
2007-08-08: change the movie frame range to full.
2008 Aug: a lot of update made by Zongcai Ruan
2008 Aug, 22: add *updateLandmark, *loadObjectsFromFile buttons

//mypara->xwidget->getImageData()->listUserDefinedLocation;
 *ptLocation.h
 *pxLocaNotUseful, pxLocaUseful, pxLocaUnsure

****************************************************************************/

#include "v3dr_mainwindow.h"
#include "v3dr_glwidget.h"
#include <QElapsedTimer>
#include "v3dr_control_signal.cpp" // create control widgets and connect signals

///QTimer V3dR_MainWindow::animate_timer;// for static

void V3dR_MainWindow::closeEvent(QCloseEvent* e)
{
	qDebug("V3dR_MainWindow::closeEvent, glWidget = %0p", glWidget);

	if (sAnimate==1) // preview or save movie
	{
		e->ignore(); // wait animating stop, 080930
		sAnimate = 0;
		return;
	}
	sAnimate = 0; // stop animate
	animate_timer.stop();
	if (bAnimating) // prevent to close from inner of animate()
    {
		QTimer::singleShot(100, this, SLOT(postClose())); // less loop event
		//POST_EVENT(this, QEvent::Close); // more loop event
		e->ignore(); // wait animating stop
		return;
    }

//#ifndef test_main_cpp
////	QWidget* v3d_mainwindow = v3dr_getV3Dmainwindow(_idep);
////	if (v3d_mainwindow)		v3d_mainwindow->raise();
//
//	QList<V3dR_MainWindow*>* plistV3Dview = v3dr_getV3Dview_plist(_idep);
//	if (plistV3Dview)	for (int i=0; i<plistV3Dview->size(); i++)
//	{
//		//plistV3Dview->at(i)->hide();
//		plistV3Dview->at(i)->setVisible(false); //not work to over delete
//		//plistV3Dview->at(i)->setEnabled(false); //not work to over delete
//	}
//#endif

	//setWindowModality(Qt::ApplicationModal); //090710
	//setEnabled(false);

	if (glWidget) //090705 RZC: move form ~V3dR_MainWindow()
	{
		//DELETE_AND_ZERO(glWidget->renderer); //090710
		glWidget->deleteRenderer(); //090711 RZC: fixed the problem of Over delete OpenGL resource cross different glWidget

		if (glWidgetArea) glWidgetArea->takeWidget();
		glWidget->deleteLater();
	}

	////////////////////////////////////////////////////////////
	// delete large memory object here to override the Qt's bug of de-constructor never be called, by Ruan ZongCai 2008-08-13
	deleteLater(); // delay delete in event loop, by RZC 20080924
	QWidget::closeEvent(e); // accept
	////////////////////////////////////////////////////////////

#ifndef test_main_cpp
	//090723 for recreate next at same position & size
	if (_idep)
	{
		_idep->local_win_pos = pos();
		_idep->local_win_size = size();
	}
#endif
}

void V3dR_MainWindow::postClose()
{
	qDebug("V3dR_MainWindow::postClose");
	POST_EVENT(this, QEvent::Close); // this OK
}

V3dR_MainWindow::~V3dR_MainWindow()
{
    qDebug() << tr("3D View [") + data_title + "]";
	qDebug("V3dR_MainWindow::~V3dR_MainWindow ====================================");

#ifndef test_main_cpp
	//now remove "this" from the central record keeper
	if (!_idep || !_idep->p_list_3Dview_win) //by PHC, 081003
	{
		qDebug() << "The iDrawExternalParameter is invalid or contains a NULL pointer to the list_3Dview_win. This should NEVER happen. Check your program.";
	}
	else //now add "this" pointer to the central record keeper
	{
		int i;
		for (i=0;i<_idep->p_list_3Dview_win->size();i++)
		{
			if (_idep->p_list_3Dview_win->at(i)==this)
			{
				_idep->p_list_3Dview_win->removeAt(i);
				break;
			}
		}
	}

	if (_idep)
	{
		_idep->b_still_open = false; //switch order of the last sentence. by PHC, 080814
		_idep->window3D = 0; //100803 RZC
	}

#else
	//qApp->quit(); //this equal exit(0)
	QCoreApplication::postEvent(qApp, new QEvent(QEvent::Quit)); // this more OK
#endif

	// must closed before there! and do nothing is most OK, by RZC 2008-09-27
}

////////DLC add/////////////////////////////////
//void V3dR_MainWindow::onShowBtnClicked()
//{
//    //点击后即可弹出一个新窗口，显示opengl渲染结果
//        qDebug()<<"槽函数被调用了";
//        //QMainWindow *newQMain = new QMainWindow(this);
//        showTriangle* myWin = new showTriangle(this);
//        myWin->resize(500, 500);
//        qDebug()<<"show之前";
//        myWin->show();
//        qDebug()<<"show之后";
//}
//////////////////////////////////////////

V3dR_MainWindow::V3dR_MainWindow(iDrawExternalParameter* idep)
{
	qDebug("V3dR_MainWindow::V3dR_MainWindow =====================================");

    init_members();    // this is important to clear zero before new object, by RZC 080818

	title_prefix = "3D View";
	data_title = "";
    this->_idep = idep; ////
#ifndef test_main_cpp
    if (_idep)
    {
        if (_idep->image4d==0)
        {
        	//090918 RZC: dummy image4d for editing swc
        	null_idep = *_idep;
        	_idep = &null_idep;
        	_idep->image4d = new My4DImage();
        }

        if (_idep->b_local) 	title_prefix = "Local 3D View";

    	_idep->b_still_open = true;
    	if (_idep->xwidget)	data_title = _idep->xwidget->windowTitle();
    	if (_idep->V3Dmainwindow) setParent(_idep->V3Dmainwindow); //090710
	}

    if (!_idep || !_idep->p_list_3Dview_win) //by PHC, 081003
	{
		qDebug() << "The iDrawExternalParameter is invalid or contains a NULL pointer to the list_3Dview_win. This should NEVER happen. Check your program.";
	}
	else //now add "this" pointer to the central record keeper
	{
		_idep->p_list_3Dview_win->append(this);
	}

	//090723 move to the last position & size
    if (_idep && (_idep->local_win_size.isValid()))
	{
		 move(_idep->local_win_pos + QPoint(0, 25)); //100721 RZC, move pos down slightly to pull back title bar of win
		 resize(_idep->local_win_size);
	}
#endif
    qDebug() << title_prefix+" [" + data_title + "]";
    setWindowTitle(title_prefix+" [" + data_title + "]");


    //////////////////////////////////////////////////////////////////
    glWidget = 0;
    glWidget = new V3dR_GLWidget(_idep, this, data_title); // 'this' pointer for glWidget calling back
#if defined(USE_Qt5)
    if ( !glWidget ) //Under Qt5, the GL Widget is not valid until after it's shown
#else
   // if (!glWidget || !(glWidget->isValid()))
     if ( !glWidget )
#endif
    {
        MESSAGE("ERROR: Failed to create OpenGL Widget or Context!!! \n");
    }
    //////////////////////////////////////////////////////////////////

    //if (glWidget)	POST_EVENT(glWidget, QEvent::Type(QEvent_OpenFiles)); // move to V3dR_GLWidget::initializeGL for dynamic renderer, 081122 by RZC


    //创建控制信号
    qDebug("V3dR_MainWindow::createControlWidgets");
    createControlWidgets(); // RZC 080930, 090420: included connectSignal() & initControlValue()


    setAcceptDrops(true); //081031
    setFocusPolicy(Qt::StrongFocus); // STRANGE: cannot accept foucusInEvent when mouse click, 081119

    qDebug("V3dR_MainWindow::V3dR_MainWindow ===== end");
}

void V3dR_MainWindow::setDataTitle(QString newdt)
{
	data_title=newdt; setWindowTitle(title_prefix+" [" + data_title + "]");
	if (glWidget) glWidget->setDataTitle(newdt);
}



#define __animation__

void V3dR_MainWindow::animateOn()
{
	// rotation_frames = FPS*rotation_time_ms/1000

	int rotation_time_ms = rotationSpeedSec*1000; //12000;
	int rotation_frames = FPS*rotation_time_ms/1000; //15fps * 12sec/rot = 180frame/rot

	sAnimate = 2; // run continue
	animate( scriptAnimateRot, rotation_time_ms, rotation_frames, rotationTimePoints);
}

void V3dR_MainWindow::animateOff()
{
	sAnimate = 0;
}

V3DLONG V3dR_MainWindow::animate(QString& loop_script, int rotation_time_ms,
					int rotation_frames, int rotation_timepoints, bool bSaveFrame)
{
	if (sAnimate==0)
	{
		animate_timer.stop();
		return 0;
	}

    animate_option.loop_list = loop_script.split(" ",Qt::SkipEmptyParts);
	animate_option.frame_time_ms = (rotation_frames<=0)? rotation_time_ms : rotation_time_ms/rotation_frames;
	animate_option.rotation_frames = rotation_frames;
	animate_option.rotation_timepoints = rotation_timepoints;

	animate_option.frame_timepoints = float(animate_option.rotation_timepoints)/animate_option.rotation_frames;
	if (animate_option.rotation_frames==0) animate_option.frame_timepoints=0;

	animate_option.iframe_rotation = 0;
	animate_option.irotation = 0;
	animate_option.bSaveFrame = bSaveFrame;

	if (animate_option.frame_time_ms)
		animate_timer.start(animate_option.frame_time_ms);

	V3DLONG frames = animate_option.loop_list.size() * animate_option.rotation_frames;
	return frames;
}

void V3dR_MainWindow::animateStep()
{
	glWidget->makeCurrent(); //090918: need for OpenGL drawing
	if (sAnimate==2 && !isActiveWindow()) return; // optional: only animate the active view

	if (bAnimating) return; // prevent to re-enter
	if (!glWidget)  return; // no content
	if (sAnimate==0)
	{
		animate_timer.stop();
		return;
	}

	////// change to use animate_timer for animation, 081021 by RZC
	QImage image1;
	int iframe;
	bAnimating = true;
	{
		int xstep, ystep, zstep;
		xstep = ystep = zstep = 0;
		QString action = animate_option.loop_list[animate_option.irotation].toUpper();
		if (action == "X")
		{
			xstep = int(ANGLE_TICK * 360 / animate_option.rotation_frames);
		}
		if (action == "Y")
		{
			ystep = int(ANGLE_TICK * 360 / animate_option.rotation_frames);
		}
		if (action == "Z")
		{
			zstep = int(ANGLE_TICK * 360 / animate_option.rotation_frames);
		}

		/////////////////////////////////////////////////////////////////
		glWidget->modelRotation(xstep, ystep, zstep);
		glWidget->incVolumeTimePoint(animate_option.frame_timepoints);

		iframe = animate_option.irotation * animate_option.rotation_frames + animate_option.iframe_rotation;
		if (animate_option.bSaveFrame)
		{
			saveFrameFunc(iframe);
		}
		////////////////////////////////////////////////////////////////

		if (++ animate_option.iframe_rotation >= animate_option.rotation_frames)
		{
			animate_option.iframe_rotation = 0;
			animate_option.irotation++;
		}

		if (animate_option.irotation >= animate_option.loop_list.size())
		{
			animate_option.irotation = 0;
			if (sAnimate==1) // sAnimate==1 run once
			{
				animate_option.iframe_rotation = 0;
				sAnimate = 0;
				animate_timer.stop();
			}
		}
	}
	bAnimating = false;
}

QString V3dR_MainWindow::getAnimateRotType(QString qtitle, bool* ok)
{
	QStringList itemlist;
	itemlist << "X" << "X Y" << "X Y Z" << "Y" << "Y X" << "Y X Z" << "Z" << "Z X Y" << "Z Y X";

	QString item = "";
	item = QInputDialog::getItem(0, qtitle,	QObject::tr("Animate rotation type:"), itemlist, 0, false, ok);
	return item;
}

void V3dR_MainWindow::setAnimateRotType()
{
	int oldAnimate = sAnimate;
	sAnimate = 0;
	animateStep(); //to stop

	bool ok;
	QString item = getAnimateRotType(QObject::tr("Animation"), &ok);
	if (ok)  scriptAnimateRot = item;

	if (oldAnimate==2) //continue
		animateOn();
}

int V3dR_MainWindow::getAnimateRotTimePoints(QString qtitle, bool* ok, int v)
{
	// time points of rotation
	int timepoints = 0;
	if (glWidget->dataDim5()>1)
	{
		timepoints = v;

#if defined(USE_Qt5)
		timepoints = QInputDialog::getInt(0, qtitle, QObject::tr("Time-points per rotation:"), timepoints, 0, 1000, 1, ok);
#else
        timepoints = QInputDialog::getInt(0, qtitle, QObject::tr("Time-points per rotation:"), timepoints, 0, 1000, 1, ok);
#endif
	}
	else
	{
		QMessageBox::information( this, "3D View", QObject::tr("This image does not have time channel.\n") );
	}
	return timepoints;
}

void V3dR_MainWindow::setAnimateRotTimePoints()
{
	int oldAnimate = sAnimate;
	sAnimate = 0;
	animateStep(); //to stop

	bool ok;
	int timepoints = getAnimateRotTimePoints(QObject::tr("Animation"), &ok, rotationTimePoints);
	if (ok)  rotationTimePoints = timepoints;

	if (oldAnimate==2) //continue
		animateOn();
}
void V3dR_MainWindow::setAnimateRotSpeedSec()
{
	int oldAnimate = sAnimate;
	sAnimate = 0;
	animateStep(); //to stop

	bool ok;

#if defined(USE_Qt5)
	int time_sec = QInputDialog::getInt(0, QObject::tr("Animation"),
									QObject::tr("Seconds per rotation of speed:"), rotationSpeedSec, 0, 1000, 1, &ok);
#else
    int time_sec = QInputDialog::getInt(0, QObject::tr("Animation"),
									QObject::tr("Seconds per rotation of speed:"), rotationSpeedSec, 0, 1000, 1, &ok);
#endif
	if (ok)  rotationSpeedSec = time_sec;

	if (oldAnimate==2) //continue
		animateOn();
}


#define __save_movie__

QString V3dR_MainWindow::previewMovie(QString& loop_script, int rotation_frames, int rotation_timepoints)
{
	movieSaveButton->setEnabled(false);

	V3DLONG frames = animate(loop_script, 0, rotation_frames, rotation_timepoints, false); // rotation_time_ms>0 for processing blocked events
    QElapsedTimer qtime;  qtime.start();

	if (glWidget)  glWidget->setStill(false); //use sampled resolution
	while (sAnimate==1)
	{
		animateStep();
		//QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents); //090916 process blocked event
		//SLEEP(1000/rotation_frames); // control speed for clear preview
	}

	movieSaveButton->setEnabled(true);

	QString info_benchmark = QString("Average rendering speed .............. %1 msec/frame, or FPS = %2 hz").
										arg(qtime.elapsed()/double(frames)).arg(double(frames)*1000/qtime.elapsed());
	qDebug()<< "***  " << info_benchmark;
	return info_benchmark;
}

void V3dR_MainWindow::doSaveMovie(QString& loop_script, int rotation_frames, int rotation_timepoints)
{
	movieSaveButton->setEnabled(false);

	animate(loop_script, 0, rotation_frames, rotation_timepoints, true); // rotation_time_ms>0 for processing blocked events, 080930

	if (glWidget)  glWidget->setStill(true);//use full resolution
	while (sAnimate==1)
	{
		animateStep();
		//QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents); //090916 process blocked event
	}
	if (glWidget)  glWidget->setStill(false);

	movieSaveButton->setEnabled(true);
}

#define DEFAULT_ROTATION_FRAMES  30
#define SAVE_IMG_FORMAT "BMP"
//#define SAVE_IMG_FORMAT "PNG"
//#define SAVE_IMG_FORMAT "TIFF"

bool V3dR_GLWidget::screenShot(QString filename)
{
#if defined(USE_Qt5)
    QImage image1 = this->grabFramebuffer();
#else
    //QImage image1 = this->grabFrameBuffer();
    QImage image1 = this->grabFramebuffer();
#endif

        const char* format = SAVE_IMG_FORMAT;
    QString curfile = filename + "." + format;
    bool r =false;
    if (image1.save(curfile, format, 100)) //uncompressed
    {
        printf("Successful to save screen-shot: [%s]\n",  curfile.toUtf8().data());
        r = true;
    }
    else
    {
        printf("Failed to save screen-shot: [%s]\n",  curfile.toUtf8().data());
    }
    return r;
}

void V3dR_MainWindow::saveFrameFunc(int i)
{
	if (! glWidget)
	{
                qDebug("  No glWidget in V3dR_MainWindow::saveFrameFunc (%d) !", i);
		return;
	}

//#if defined(USE_Qt5)
//	QImage image1 = glWidget->grabFramebuffer();
//#else
//	QImage image1 = glWidget->grabFrameBuffer();
//#endif

        const char* format = SAVE_IMG_FORMAT;
	QString curfile = QString("%1/a%2.%3").arg(outputDir).arg(i).arg(format);
//	if (image1.save(curfile, format, 100)) //uncompressed
//	{
//		printf("Successful to save frame %d: [%s]\n", i, curfile.toUtf8().data());
//	}
//	else
//	{
//		printf("Failed to save frame %d: [%s]\n", i, curfile.toUtf8().data());
//	}
}

void V3dR_MainWindow::saveMovie()
{
    if (sAnimate==1) // prevent to re-enter preview
    {
    	return;
    }
    sAnimate = 0; // stop animate
    if (bAnimating) // prevent to re-enter animate()
    {
    	QTimer::singleShot(500, this, SLOT(saveMovie())); // wait 0.5 sec for stopping animation
    	return;
    }

	if (!glWidget) return; //make sure the image is valid

   	//default parameters
	int rotation_frames = DEFAULT_ROTATION_FRAMES;
	int rotation_timepoints = rotation_frames;
   	QString loop_script = "X Y Z"; // animation script, 080930

   	QString qtitle = QObject::tr("Save Movie");
	bool ok;
	int ret	= QMessageBox::Retry;
	while (ret == QMessageBox::Retry)
	{
		// rotation type
		{
			loop_script = getAnimateRotType(qtitle, &ok);
			if (! ok) return;
		}
		// frames of rotation
		{

#if defined(USE_Qt5)
			rotation_frames = QInputDialog::getInt(0, qtitle, QObject::tr("Frames per rotation:"), rotation_frames, 0, 1000, 1, &ok);
#else
            rotation_frames = QInputDialog::getInt(0, qtitle, QObject::tr("Frames per rotation:"), rotation_frames, 0, 1000, 1, &ok);
#endif
			if (! ok) return;
		}
		// time points of rotation
		{
			rotation_timepoints = getAnimateRotTimePoints(qtitle, &ok, rotation_timepoints);
			if (! ok) return;
		}

		if (QMessageBox::information(0, qtitle,
							tr("Now preview the movie"),
							QMessageBox::Ok | QMessageBox::Cancel,
							QMessageBox::Ok)
			==QMessageBox::Cancel)
			return;

		sAnimate = 1; // run once
		QString info_benchmark = previewMovie(loop_script, rotation_frames, rotation_timepoints);
		sAnimate = 0;

		if ((ret = QMessageBox::question(0, qtitle,
							info_benchmark + "\n\n" +
							tr("Do you want to save your movie? or retry preview."),
							QMessageBox::Yes  | QMessageBox::No | QMessageBox::Retry,
							QMessageBox::Yes))
			==QMessageBox::No)
			return;
	}

	if (rotation_frames<1)	return;
	//get the save directory name
    {
	    QDir curdir("./");
		outputDir = QFileDialog::getExistingDirectory(
		                                       0,
											   "Choose a directory under which all movie frames will be saved to",
											   curdir.absoluteFilePath("./"),
											   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
											   );

	    if (outputDir.isEmpty()) //note that I used isEmpty() instead of isNull
		{
    	    QMessageBox::information( 0, "", tr("No saving."), QMessageBox::Ok);
		    return;
		}
		else
		{
		    if (!curdir.exists(outputDir))
			   curdir.mkdir(outputDir);
			//outputDir = curdir.absoluteFilePath(absoluteFilePath);
			//curdir.cd(outputDir);
		}
	}
    printf("\nThe directory to save movie frames: [%s]\n", outputDir.toUtf8().data());

    sAnimate=1; // run once
	doSaveMovie(loop_script, rotation_frames, rotation_timepoints); // 080930
    sAnimate=0;
}


///////////////////////////////////////////////////////////////////////
#define __event_handler__

void V3dR_MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug("V3dR_MainWindow::dragEnterEvent");
	//if (event->mimeData()->hasUrls()) // filename use urls
    event->acceptProposedAction();
}

void V3dR_MainWindow::dropEvent(QDropEvent *event)
{
    qDebug("V3dR_MainWindow::dropEvent");
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasImage())
    {
        qDebug() <<tr("  drop Image data");
    }
    else if (mimeData->hasHtml())
    {
        qDebug() <<tr("  drop Html data");
    }
    else if (mimeData->hasText())
    {
        qDebug() <<tr("  drop Text data: ")+(mimeData->text().trimmed());
        QString url = mimeData->text().trimmed();

#ifdef Q_OS_LINUX
        url.remove(0,7); // remove the first 'file://' of the name string, 090125
        url.replace("%20"," ");//fixed the space path issue in 3D viewer on Linux machine by Zhi Zhou May 15 2015
#endif
        qDebug("the file to open=[%s]",qPrintable(url));
        if (glWidget) glWidget->loadObjectFromFile(url);
    }
    else if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size() && (i < 1); ++i)
        {
            QString url = urlList.at(i).path().trimmed(); //add trimmed by PHC, 090125
            qDebug() <<tr("  drop Url data: ")+url;

#ifdef WIN32
            url.remove(0,1); // remove the first '/' of "/C:/...", 081102
#endif

#ifdef Q_OS_MAC
            //Added by Zhi on 2018-03-01
            if (urlList.at(i).path().startsWith("/.file/id=")) {
                QProcess process;
                QStringList arguments;
                arguments << "-e" << "get posix path of posix file \""+urlList.at(i).path()+"\"";
                process.start("osascript", arguments);
                process.waitForFinished(-1); // will wait forever until finished
                url = process.readAllStandardOutput();
                url = url.remove(url.length()-1,1);
            }
#endif
            if (glWidget) glWidget->loadObjectFromFile(url);
            //setDataTitle(url);
        }
        event->acceptProposedAction();
    }
    else
    {
        qDebug() <<tr("  Unknown drop data");
    }
}


void V3dR_MainWindow::focusInEvent(QFocusEvent*)
{
	//qDebug("V3dR_MainWindow::focusInEvent");
	//if (glWidget)  glWidget->showTool();  // no need for ToolWindow
}
void V3dR_MainWindow::focusOutEvent(QFocusEvent*)
{
	//qDebug("V3dR_MainWindow::focusOutEvent");
	//if (glWidget)  glWidget->hideTool(); // no need for ToolWindow
}
void V3dR_MainWindow::enterEvent(QEvent*)
{
    qDebug("V3dR_MainWindow::enterEvent");
    setFocus();
}
void V3dR_MainWindow::leaveEvent(QEvent*)
{
    qDebug("V3dR_MainWindow::leaveEvent");
}


QWidget* V3dR_MainWindow::lastActive = 0;
void V3dR_MainWindow::changeEvent(QEvent* e)
{
    //qDebug() <<"V3dR_MainWindow::changeEvent" << e->type();
	//if (e->type()==QEvent::WindowActivate) // no occur!!!
	if (e->type()==QEvent::ActivationChange && isActiveWindow())
	{
		if (glWidget)	glWidget->makeCurrent(); //090715

		if (lastActive != this) //need updateTool
		{
			lastActive = this;
			qDebug() << QString("V3dR_MainWindow::changeEvent, ActivationChange-> %1").arg(title_prefix+" [" + data_title + "]");

			if (glWidget)  glWidget->updateTool();
		}
		//090713 RZC: the state synchronization is hard
		//if (glWidget)  glWidget->updateLandmark(); // call glWidget->updateTool()
	}
}

///////////////////////////////////////////////////////////////////////
//A widget must call setFocusPolicy() to accept focus initially and have focus in order to receive a key press event.
//If you reimplement this handler, it is very important that you call the base class implementation if you do not act upon the key.
//The default implementation closes popup widgets if the user presses Esc. Otherwise the event is ignored, so that the widget's parent can interpret it.
//Note that QKeyEvent starts with isAccepted() == true, so you do not need to call QKeyEvent::accept()
//- just do not call the base class implementation if you act upon the key.

#define KM  QApplication::keyboardModifiers()

void V3dR_MainWindow::keyPressEvent(QKeyEvent * e)
{
	// qDebug("keyPressEvent"); // after sub-widget accepted

	switch (e->key())
	{
	case Qt::Key_W:
		if (KM==CTRL2_MODIFIER || KM==Qt::ControlModifier)
		{
			postClose();
		}
		break;
	case Qt::Key_A:
		if (KM==CTRL2_MODIFIER || KM==Qt::ControlModifier)
		{
			if (sAnimate==0)
				animateOn(); // from stop to animate
			else
				animateOff();
		}
		break;
	case Qt::Key_M:
		if (KM==CTRL2_MODIFIER || KM==Qt::ControlModifier)
		{
			saveMovie();
		}
		break;

	default:
		if (glWidget /*&& (glWidget->frameGeometry()).contains(QCursor::pos())*/) //transfer key-event to glwidget
		//if (! QApplication::keyboardModifiers()) //090427 RZC: no transfer Modifier Key
		{ //081006, 090428 final solved by a public function handleKeyPressEvent
			glWidget->handleKeyPressEvent(e); // keyPressEvent is protected
			//QCoreApplication::sendEvent(glWidget, (e)); // send Modifier Key will crash
			//QCoreApplication::postEvent(glWidget, (e)); // no effect
			//QKeyEvent* ke = new QKeyEvent(*e);             // a event copy
			//QCoreApplication::postEvent(glWidget, (ke));   // post Modifier Key make CPU 100% load.
			//QCoreApplication::sendEvent(glWidget, (ke)); // send Modifier Key will crash
		}
		else
			QWidget::keyPressEvent(e);
		break;
	}
	return;
}

void V3dR_MainWindow::keyReleaseEvent(QKeyEvent * e)
{
	// qDebug("keyReleaseEvent"); // after sub-widget accepted

	switch (e->key())
	{

	default:
		if (glWidget)
		{ //081006, 090428 final solved by a public function handleKeyPressEvent
			glWidget->handleKeyReleaseEvent(e); // keyPressEvent is protected
		}
		else
			QWidget::keyReleaseEvent(e);
		break;
	}
	return;
}


/////////////////////////////////////////////////////////////////////////



/*
void V3dR_MainWindow::setXRotStep(int t)
{
    xstep = t;
}

void V3dR_MainWindow::setYRotStep(int t)
{
    ystep = t;
}

void V3dR_MainWindow::setZRotStep(int t)
{
    zstep = t;
}

void V3dR_MainWindow::setNSteps(int t)
{
    nsteps_rot_movie = t;
}

void iDrawMainWindow::openAnoFile()
{
    //get the file and verify it is useable. If not exist or useable, the  make no change of the current setting (3D view, etc)
	QString	tmpfile = QFileDialog::getOpenFileName(
                    this,
                    "Choose an atlas/annotation file to open",
                    "./",
                    "File (*.txt *.ano)");

    if (tmpfile.isEmpty()) // do nothing if nothing is chosen
	{
	    return;
	}


	//the same file, then do nothing. But I still disable this because sometimes the file with the same name could have updated contents
	//if (curAnoFile==tmpfile)
	//    return;

    //try to use the data

    GLWidget * glWidget1 = new GLWidget(tmpfile, "/Users/pengh/work/fly.brain.Julie/localdata/a67@20X.raw_small.raw");
    if (glWidget1->getErrorFlag() == true) //must have some error in file reading or use the data
	{
	   delete glWidget1;
	   return;
	}

    //in this case it has been verified to safely disconect previous signal and render a new atlas/annotation file

	if (glWidget)
	{
	   //disconnectSignal();
	   if (glWidgetArea)
	   	   glWidgetArea->takeWidget();
	   delete glWidget;
	   glWidget=0; //just for my usual pointer freeing style, :-)
	}
	glWidget = glWidget1;
    //if (glWidgetArea) glWidgetArea->setWidget(glWidget);

    connectSignal();
	curAnoFile = tmpfile; //update the data file name
	update();
	//glWidgetArea->update();
	glWidget->updateGL();

	return;
}


void V3dR_MainWindow::about()
{
	QMessageBox::information(this, "Vaa3D.3drenderer", "Vaa3D: a 3D image visualization and analysis software.\n"
			"\n\n"
			 "Help information for the 3D viewer:\n"
			"Mouse operations:\n"
			"----------------------------------------------------\n"
			"[Hold L-button + move vert.]:\t rotate around X-axis.\n"
			"[Hold L-button + move hori.]:\t rotate around Y-axis.\n"
			"[Hold R-button + move hori.]:\t rotate around Z-axis.\n"
			"[Hold R-button + move vert.]:\t zoom in/out.\n"
			"[Scroll Mouse-wheel]:\t\t zoom in/out.\n"
			);
}


void V3dR_MainWindow::createActions()
{
    renderIntoPixmapAct = new QAction(tr("&Render into Pixmap..."), this);
    renderIntoPixmapAct->setShortcut(tr("Ctrl+R"));
    connect(renderIntoPixmapAct, SIGNAL(triggered()),
            this, SLOT(renderIntoPixmap()));

    grabFrameBufferAct = new QAction(tr("&Grab Frame Buffer"), this);
    grabFrameBufferAct->setShortcut(tr("Ctrl+G"));
    connect(grabFrameBufferAct, SIGNAL(triggered()),
            this, SLOT(grabFrameBuffer()));

    clearPixmapAct = new QAction(tr("&Clear Pixmap"), this);
    clearPixmapAct->setShortcut(tr("Ctrl+L"));
    connect(clearPixmapAct, SIGNAL(triggered()), this, SLOT(clearPixmap()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void V3dR_MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(renderIntoPixmapAct);
    fileMenu->addAction(grabFrameBufferAct);
    fileMenu->addAction(clearPixmapAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

}

void iDrawMainWindow::setPixmap(const QPixmap &pixmap)
{
    pixmapLabel->setPixmap(pixmap);
    QSize size = pixmap.size();
    if (size - QSize(1, 0) == pixmapLabelArea->maximumViewportSize())
        size -= QSize(1, 0);
    pixmapLabel->resize(size);

}

QSize V3dR_MainWindow::getSize()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("3D View"),
                                         tr("Enter pixmap size:"),
                                         QLineEdit::Normal,
                                         tr("%1 x %2").arg(glWidget->width())
                                                      .arg(glWidget->height()),
                                         &ok);
    if (!ok)
        return QSize();

    QRegExp regExp(tr("([0-9]+) *x *([0-9]+)"));
    if (regExp.exactMatch(text)) {
        int width = regExp.cap(1).toInt();
        int height = regExp.cap(2).toInt();
        if (width > 0 && width < 2048 && height > 0 && height < 2048)
            return QSize(width, height);
    }

    return glWidget->size();
}
*/


