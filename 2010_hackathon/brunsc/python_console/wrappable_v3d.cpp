#include "wrappable_v3d.h"
#include <stdexcept>

// Test QString wrapping
QString hello() {return QString("Hello");}
std::string hello2(const QString& s) {return s.toStdString();}

// defined in V3DPythonModule.cpp
extern V3DPluginCallback2 *v3d_callbackPtr;
extern QThread *qtGuiThread;

class NullV3DCallbackException : public std::runtime_error
{
public:
    NullV3DCallbackException(const char* msg) : std::runtime_error(msg)
    {}
};

V3DPluginCallback2* getCallback()
{
    if (! v3d_callbackPtr)
        throw NullV3DCallbackException("V3D callback is NULL");
    return v3d_callbackPtr;
}


void ImageWindowReceiver::open3DWindow() {
    getCallback()->open3DWindow(handle);
}


V3D_GlobalSetting getGlobalSetting() {
    return getCallback()->getGlobalSetting();
}

bool setGlobalSetting(V3D_GlobalSetting& gs) {
    return getCallback()->setGlobalSetting(gs);
}

// Get the current V3D image window
/* static */ ImageWindow ImageWindow::current() {
    return ImageWindow(getCallback()->currentImageWindow());
}
/* static */ ImageWindow ImageWindow::currentHiddenSelected() {
    return ImageWindow(getCallback()->curHiddenSelectedWindow());
}

ImageWindow::ImageWindow(void* h) : handle(h)
{
	// TODO - must get that receiver into the GUI thread
	receiver = new ImageWindowReceiver();
	receiver->handle = handle;
	if (qtGuiThread) {
		receiver->moveToThread(qtGuiThread);
	}
	dispatcher = new ImageWindowDispatcher();
	QObject::connect(dispatcher, SIGNAL(open3DWindow()),
			receiver, SLOT(open3DWindow()));
}

ImageWindow::ImageWindow(const std::string& name)
{
    handle = getCallback()->newImageWindow(QString(name.c_str()));
	receiver = new ImageWindowReceiver();
	receiver->handle = handle;
	if (qtGuiThread) {
		receiver->moveToThread(qtGuiThread);
	}
	dispatcher = new ImageWindowDispatcher();
	QObject::connect(dispatcher, SIGNAL(open3DWindow()),
			receiver, SLOT(open3DWindow()));
}

void ImageWindow::update() {
    getCallback()->updateImageWindow(handle);
}
std::string ImageWindow::getName() const {
    return getCallback()->getImageName(handle).toStdString();
}
void ImageWindow::setName(const std::string& name) {
    getCallback()->setImageName(handle, QString(name.c_str()));
}
Image4DSimple* ImageWindow::getImage() {
    return getCallback()->getImage(handle);
}
bool ImageWindow::setImage(Image4DSimple* image) {
    return getCallback()->setImage(handle, image);
}
LandmarkList ImageWindow::getLandmark() {
    return getCallback()->getLandmark(handle);
}
bool ImageWindow::setLandmark(LandmarkList & landmark_list) {
    return getCallback()->setLandmark(handle, landmark_list);
}
ROIList ImageWindow::getROI() {
    return getCallback()->getROI(handle);
}
bool ImageWindow::setROI(ROIList & roi_list) {
    return getCallback()->setROI(handle, roi_list);
}
NeuronTree ImageWindow::getSWC() {
    return getCallback()->getSWC(handle);
}
bool ImageWindow::setSWC(NeuronTree & nt) {
    return getCallback()->setSWC(handle, nt);
}
//virtual void open3DWindow(v3dhandle image_window) = 0;
void ImageWindow::open3DWindow() {
    // return getCallback()->open3DWindow(handle);
	dispatcher->emitOpen3DWindow();
}
//virtual void close3DWindow(v3dhandle image_window) = 0;
void ImageWindow::close3DWindow() {
    return getCallback()->close3DWindow(handle);
}
//virtual void openROI3DWindow(v3dhandle image_window) = 0;
void ImageWindow::openROI3DWindow() {
    return getCallback()->openROI3DWindow(handle);
}
//virtual void closeROI3DWindow(v3dhandle image_window) = 0;
void ImageWindow::closeROI3DWindow() {
    return getCallback()->closeROI3DWindow(handle);
}
//virtual void pushObjectIn3DWindow(v3dhandle image_window) = 0;
void ImageWindow::pushObjectIn3DWindow() {
    return getCallback()->pushObjectIn3DWindow(handle);
}
//virtual void pushImageIn3DWindow(v3dhandle image_window) = 0;
void ImageWindow::pushImageIn3DWindow() {
    return getCallback()->pushImageIn3DWindow(handle);
}
//virtual int pushTimepointIn3DWindow(v3dhandle image_window, int timepoint) = 0;
int ImageWindow::pushTimepointIn3DWindow(int timepoint) {
    return getCallback()->pushTimepointIn3DWindow(handle, timepoint);
}
//virtual bool screenShot3DWindow(v3dhandle image_window, QString BMPfilename) = 0;
bool ImageWindow::screenShot3DWindow(const std::string& BMPfilename) {
    return getCallback()->screenShot3DWindow(handle, QString(BMPfilename.c_str()));
}
//virtual bool screenShotROI3DWindow(v3dhandle image_window, QString BMPfilename) = 0;
bool ImageWindow::screenShotROI3DWindow(const std::string& BMPfilename) {
    return getCallback()->screenShotROI3DWindow(handle, QString(BMPfilename.c_str()));
}
//virtual View3DControl * getView3DControl(v3dhandle image_window) = 0;
View3DControl* ImageWindow::getView3DControl() {
    return getCallback()->getView3DControl(handle);
}
//virtual View3DControl * getLocalView3DControl(v3dhandle image_window) = 0;
View3DControl* ImageWindow::getLocalView3DControl() {
    return getCallback()->getLocalView3DControl(handle);
}
//virtual TriviewControl * getTriviewControl(v3dhandle image_window) = 0;
TriviewControl* ImageWindow::getTriviewControl() {
    return getCallback()->getTriviewControl(handle);
}
