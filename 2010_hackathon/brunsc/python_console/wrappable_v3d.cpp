#include "wrappable_v3d.h"

// defined in V3DPythonModule.cpp
extern V3DPluginCallback2 *v3d_callbackPtr;

V3D_GlobalSetting getGlobalSetting() {
    return v3d_callbackPtr->getGlobalSetting();
}

bool setGlobalSetting(V3D_GlobalSetting& gs) {
    return v3d_callbackPtr->setGlobalSetting(gs);
}

// Get the current V3D image window
/* static */ ImageWindow ImageWindow::current() {
    return ImageWindow(v3d_callbackPtr->currentImageWindow());
}
/* static */ ImageWindow ImageWindow::currentHiddenSelected() {
    return ImageWindow(v3d_callbackPtr->curHiddenSelectedWindow());
}

ImageWindow::ImageWindow(void* h) : handle(h) {}
ImageWindow::ImageWindow(const std::string& name) {
    handle = v3d_callbackPtr->newImageWindow(QString(name.c_str()));
}
void ImageWindow::update() {
    v3d_callbackPtr->updateImageWindow(handle);
}
std::string ImageWindow::getName() const {
    return v3d_callbackPtr->getImageName(handle).toStdString();
}
void ImageWindow::setName(const std::string& name) {
    v3d_callbackPtr->setImageName(handle, QString(name.c_str()));
}
Image4DSimple* ImageWindow::getImage() {
    return v3d_callbackPtr->getImage(handle);
}
bool ImageWindow::setImage(Image4DSimple* image) {
    return v3d_callbackPtr->setImage(handle, image);
}
LandmarkList ImageWindow::getLandmark() {
    return v3d_callbackPtr->getLandmark(handle);
}
bool ImageWindow::setLandmark(LandmarkList & landmark_list) {
    return v3d_callbackPtr->setLandmark(handle, landmark_list);
}
ROIList ImageWindow::getROI() {
    return v3d_callbackPtr->getROI(handle);
}
bool ImageWindow::setROI(ROIList & roi_list) {
    return v3d_callbackPtr->setROI(handle, roi_list);
}
NeuronTree ImageWindow::getSWC() {
    return v3d_callbackPtr->getSWC(handle);
}
bool ImageWindow::setSWC(NeuronTree & nt) {
    return v3d_callbackPtr->setSWC(handle, nt);
}
//virtual void open3DWindow(v3dhandle image_window) = 0;
void ImageWindow::open3DWindow() {
    return v3d_callbackPtr->open3DWindow(handle);
}
//virtual void close3DWindow(v3dhandle image_window) = 0;
void ImageWindow::close3DWindow() {
    return v3d_callbackPtr->close3DWindow(handle);
}
//virtual void openROI3DWindow(v3dhandle image_window) = 0;
void ImageWindow::openROI3DWindow() {
    return v3d_callbackPtr->openROI3DWindow(handle);
}
//virtual void closeROI3DWindow(v3dhandle image_window) = 0;
void ImageWindow::closeROI3DWindow() {
    return v3d_callbackPtr->closeROI3DWindow(handle);
}
//virtual void pushObjectIn3DWindow(v3dhandle image_window) = 0;
void ImageWindow::pushObjectIn3DWindow() {
    return v3d_callbackPtr->pushObjectIn3DWindow(handle);
}
//virtual void pushImageIn3DWindow(v3dhandle image_window) = 0;
void ImageWindow::pushImageIn3DWindow() {
    return v3d_callbackPtr->pushImageIn3DWindow(handle);
}
//virtual int pushTimepointIn3DWindow(v3dhandle image_window, int timepoint) = 0;
int ImageWindow::pushTimepointIn3DWindow(int timepoint) {
    return v3d_callbackPtr->pushTimepointIn3DWindow(handle, timepoint);
}
//virtual bool screenShot3DWindow(v3dhandle image_window, QString BMPfilename) = 0;
bool ImageWindow::screenShot3DWindow(const std::string& BMPfilename) {
    return v3d_callbackPtr->screenShot3DWindow(handle, QString(BMPfilename.c_str()));
}
//virtual bool screenShotROI3DWindow(v3dhandle image_window, QString BMPfilename) = 0;
bool ImageWindow::screenShotROI3DWindow(const std::string& BMPfilename) {
    return v3d_callbackPtr->screenShotROI3DWindow(handle, QString(BMPfilename.c_str()));
}
//virtual View3DControl * getView3DControl(v3dhandle image_window) = 0;
View3DControl* ImageWindow::getView3DControl() {
    return v3d_callbackPtr->getView3DControl(handle);
}
//virtual View3DControl * getLocalView3DControl(v3dhandle image_window) = 0;
View3DControl* ImageWindow::getLocalView3DControl() {
    return v3d_callbackPtr->getLocalView3DControl(handle);
}
//virtual TriviewControl * getTriviewControl(v3dhandle image_window) = 0;
TriviewControl* ImageWindow::getTriviewControl() {
    return v3d_callbackPtr->getTriviewControl(handle);
}
