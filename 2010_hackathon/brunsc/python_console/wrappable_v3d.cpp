#include "wrappable_v3d.h"
#include <stdexcept>

unsigned int qHash(const LocationSimple& loc){
    return qHash(QString("%1,%2,%3").arg(loc.x).arg(loc.y).arg(loc.z));
}
template class QList<LocationSimple>;

template class QVector<QPoint>;

unsigned int qHash(const QPolygon&) {
    return 1; // TODO
}
template class QList<QPolygon>;
template class QHash<int, int>;

class NullV3DCallbackException : public std::runtime_error
{
public:
    NullV3DCallbackException(const char* msg) : std::runtime_error(msg)
    {}
};

void ImageWindowReceiver::open3DWindow() {
    v3d::get_plugin_callback()->open3DWindow(handle);
}


V3D_GlobalSetting getGlobalSetting() {
    return v3d::get_plugin_callback()->getGlobalSetting();
}

bool setGlobalSetting(V3D_GlobalSetting& gs) {
    return v3d::get_plugin_callback()->setGlobalSetting(gs);
}

// Get the current V3D image window
/* static */ ImageWindow ImageWindow::current() {
    return ImageWindow(v3d::get_plugin_callback()->currentImageWindow());
}
/* static */ ImageWindow ImageWindow::currentHiddenSelected() {
    return ImageWindow(v3d::get_plugin_callback()->curHiddenSelectedWindow());
}

ImageWindow::ImageWindow(void* h) : handle(h)
{
	// TODO - must get that receiver into the GUI thread
	receiver = new ImageWindowReceiver();
	receiver->handle = handle;
	if (v3d::get_qt_gui_thread()) {
		receiver->moveToThread(v3d::get_qt_gui_thread());
	}
	dispatcher = new ImageWindowDispatcher();
	QObject::connect(dispatcher, SIGNAL(open3DWindow()),
			receiver, SLOT(open3DWindow()));
}

ImageWindow::ImageWindow(const std::string& name)
{
    handle = v3d::get_plugin_callback()->newImageWindow(QString(name.c_str()));
	receiver = new ImageWindowReceiver();
	receiver->handle = handle;
	if (v3d::get_qt_gui_thread()) {
		receiver->moveToThread(v3d::get_qt_gui_thread());
	}
	dispatcher = new ImageWindowDispatcher();
	QObject::connect(dispatcher, SIGNAL(open3DWindow()),
			receiver, SLOT(open3DWindow()));
}

void ImageWindow::update() {
    v3d::get_plugin_callback()->updateImageWindow(handle);
}
std::string ImageWindow::getName() const {
    return v3d::get_plugin_callback()->getImageName(handle).toStdString();
}
void ImageWindow::setName(const std::string& name) {
    v3d::get_plugin_callback()->setImageName(handle, QString(name.c_str()));
}
Image4DSimple* ImageWindow::getImage() {
    return v3d::get_plugin_callback()->getImage(handle);
}
bool ImageWindow::setImage(Image4DSimple* image) {
    return v3d::get_plugin_callback()->setImage(handle, image);
}
LandmarkList ImageWindow::getLandmark() {
    return v3d::get_plugin_callback()->getLandmark(handle);
}
bool ImageWindow::setLandmark(LandmarkList & landmark_list) {
    return v3d::get_plugin_callback()->setLandmark(handle, landmark_list);
}
ROIList ImageWindow::getROI() {
    return v3d::get_plugin_callback()->getROI(handle);
}
bool ImageWindow::setROI(ROIList & roi_list) {
    return v3d::get_plugin_callback()->setROI(handle, roi_list);
}
NeuronTree ImageWindow::getSWC() {
    return v3d::get_plugin_callback()->getSWC(handle);
}
bool ImageWindow::setSWC(NeuronTree & nt) {
    return v3d::get_plugin_callback()->setSWC(handle, nt);
}
//virtual void open3DWindow(v3dhandle image_window) = 0;
void ImageWindow::open3DWindow() {
    // return v3d::get_plugin_callback()->open3DWindow(handle);
	dispatcher->emitOpen3DWindow();
}
//virtual void close3DWindow(v3dhandle image_window) = 0;
void ImageWindow::close3DWindow() {
    return v3d::get_plugin_callback()->close3DWindow(handle);
}
//virtual void openROI3DWindow(v3dhandle image_window) = 0;
void ImageWindow::openROI3DWindow() {
    return v3d::get_plugin_callback()->openROI3DWindow(handle);
}
//virtual void closeROI3DWindow(v3dhandle image_window) = 0;
void ImageWindow::closeROI3DWindow() {
    return v3d::get_plugin_callback()->closeROI3DWindow(handle);
}
//virtual void pushObjectIn3DWindow(v3dhandle image_window) = 0;
void ImageWindow::pushObjectIn3DWindow() {
    return v3d::get_plugin_callback()->pushObjectIn3DWindow(handle);
}
//virtual void pushImageIn3DWindow(v3dhandle image_window) = 0;
void ImageWindow::pushImageIn3DWindow() {
    return v3d::get_plugin_callback()->pushImageIn3DWindow(handle);
}
//virtual int pushTimepointIn3DWindow(v3dhandle image_window, int timepoint) = 0;
int ImageWindow::pushTimepointIn3DWindow(int timepoint) {
    return v3d::get_plugin_callback()->pushTimepointIn3DWindow(handle, timepoint);
}
//virtual bool screenShot3DWindow(v3dhandle image_window, QString BMPfilename) = 0;
bool ImageWindow::screenShot3DWindow(const std::string& BMPfilename) {
    return v3d::get_plugin_callback()->screenShot3DWindow(handle, QString(BMPfilename.c_str()));
}
//virtual bool screenShotROI3DWindow(v3dhandle image_window, QString BMPfilename) = 0;
bool ImageWindow::screenShotROI3DWindow(const std::string& BMPfilename) {
    return v3d::get_plugin_callback()->screenShotROI3DWindow(handle, QString(BMPfilename.c_str()));
}
//virtual View3DControl * getView3DControl(v3dhandle image_window) = 0;
View3DControl* ImageWindow::getView3DControl() {
    return v3d::get_plugin_callback()->getView3DControl(handle);
}
//virtual View3DControl * getLocalView3DControl(v3dhandle image_window) = 0;
View3DControl* ImageWindow::getLocalView3DControl() {
    return v3d::get_plugin_callback()->getLocalView3DControl(handle);
}
//virtual TriviewControl * getTriviewControl(v3dhandle image_window) = 0;
TriviewControl* ImageWindow::getTriviewControl() {
    return v3d::get_plugin_callback()->getTriviewControl(handle);
}

bool operator!=(const LocationSimple& lhs, const LocationSimple& rhs)
{
    if (lhs.x != rhs.x) return true;
    if (lhs.y != rhs.y) return true;
    if (lhs.z != rhs.z) return true;
    return false;
}

bool operator==(const LocationSimple& lhs, const LocationSimple& rhs)
{
    return ! (lhs != rhs);
}
