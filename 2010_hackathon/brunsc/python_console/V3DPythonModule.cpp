/*
 * V3DPythonModule.cpp
 *
 *  Created on: Dec 22, 2010
 *      Author: Christopher M. Bruns
 */

#include "V3DPythonModule.h"
#include <boost/python.hpp>
#include "v3d_interface.h"
#include <exception>
#include <iostream>

using namespace boost::python;
using namespace std;

class NoV3DCallbackException : public std::exception
{};

// Store a permanent pointer to the callback the V3DConsolePlugin was launched with.
static V3DPluginCallback2 *callbackPtr;
V3DPluginCallback2& getCallback() {
    return *callbackPtr;
}

// Wrap v3dhandle, which is actually "void*", so python does not choke on it.
class ImageWindowWrapper
{
public:
    // Get the current V3D image window
    static ImageWindowWrapper current() {
        return ImageWindowWrapper(callbackPtr->currentImageWindow());
    }
    static ImageWindowWrapper currentHiddenSelected() {
        return ImageWindowWrapper(callbackPtr->curHiddenSelectedWindow());
    }

    ImageWindowWrapper(void* h) : handle(h) {}
    ImageWindowWrapper(const std::string& name)
    {
        handle = callbackPtr->newImageWindow(QString(name.c_str()));
    }

    void update() {
        callbackPtr->updateImageWindow(handle);
    }

    std::string getName() const {
        return callbackPtr->getImageName(handle).toStdString();
    }

    void setName(const std::string& name) {
        callbackPtr->setImageName(handle, QString(name.c_str()));
    }

    Image4DSimple* getImage() {
        return callbackPtr->getImage(handle);
    }

    bool setImage(Image4DSimple* image) {
        return callbackPtr->setImage(handle, image);
    }

    LandmarkList getLandmark() {
        return callbackPtr->getLandmark(handle);
    }
    //virtual bool setLandmark(v3dhandle image_window, LandmarkList & landmark_list) = 0;
    bool setLandmark(LandmarkList & landmark_list) {
        return callbackPtr->setLandmark(handle, landmark_list);
    }
    //virtual ROIList getROI(v3dhandle image_window) = 0;
    ROIList getROI() {
        return callbackPtr->getROI(handle);
    }
    //virtual bool setROI(v3dhandle image_window, ROIList & roi_list) = 0;
    bool setROI(ROIList & roi_list) {
        return callbackPtr->setROI(handle, roi_list);
    }

    void* handle;
};

ImageWindowWrapper callback_currentImageWindow(V3DPluginCallback2& callback) {
    return ImageWindowWrapper(callback.currentImageWindow());
}
ImageWindowWrapper callback_curHiddenSelectedWindow(V3DPluginCallback2& callback) {
    return ImageWindowWrapper(callback.curHiddenSelectedWindow());
}
ImageWindowWrapper callback_newImageWindow(
        V3DPluginCallback2& callback, std::string name="new_image")
{
    return ImageWindowWrapper(callback.newImageWindow(QString (name.c_str())));
}
void callback_updateImageWindow(
        V3DPluginCallback2& callback, ImageWindowWrapper image_window)
{
    callback.updateImageWindow(image_window.handle);
}
std::string callback_getImageName(
        const V3DPluginCallback2& callback, ImageWindowWrapper image_window)
{
    return callback.getImageName(image_window.handle).toStdString();
}
void callback_setImageName(
        V3DPluginCallback2& callback,
        ImageWindowWrapper image_window,
        std::string name)
{
    callback.setImageName(image_window.handle, QString(name.c_str()));
}
Image4DSimple* callback_getImage(
        V3DPluginCallback2& callback, ImageWindowWrapper image_window)
{
    return callback.getImage(image_window.handle);
}
bool callback_setImage(
        V3DPluginCallback2& callback,
        ImageWindowWrapper image_window,
        Image4DSimple* image)
{
    return callback.setImage(image_window.handle, image);
}
LandmarkList callback_getLandmark(
        V3DPluginCallback2& callback, ImageWindowWrapper image_window)
{
    return callback.getLandmark(image_window.handle);
}


BOOST_PYTHON_MODULE(v3d)
{
    // ImageWindow is not actually in the plugin API, but is implicitly defined
    class_<ImageWindowWrapper>("ImageWindow", no_init)
            // expose static methods
            .def("current", &ImageWindowWrapper::current)
            .staticmethod("current")
            .def("curHiddenSelected", &ImageWindowWrapper::currentHiddenSelected)
            .staticmethod("curHiddenSelected")
            // expose only one constructor: the one that takes a string.
            .def(init<const std::string&>())
            .def("update", &ImageWindowWrapper::update)
            .def("getName", &ImageWindowWrapper::getName)
            .def("setName", &ImageWindowWrapper::setName)
            .add_property("name",
                    &ImageWindowWrapper::getName,
                    &ImageWindowWrapper::setName)
            .def("getImage", &ImageWindowWrapper::getImage,
                    return_internal_reference<>())
            .def("setImage", &ImageWindowWrapper::setImage,
                    with_custodian_and_ward<1,2>())
            .add_property("image",
                  make_function(&ImageWindowWrapper::getImage, return_internal_reference<>()),
                  make_function(&ImageWindowWrapper::setImage, with_custodian_and_ward<1,2>())
            )
            .def("getLandmark", &ImageWindowWrapper::getLandmark)
            .def("setLandmark", &ImageWindowWrapper::setLandmark)
            .add_property("landmark",
                    &ImageWindowWrapper::getLandmark,
                    &ImageWindowWrapper::setLandmark)
            .def("getROI", &ImageWindowWrapper::getROI)
            .def("setROI", &ImageWindowWrapper::setROI)
            .add_property("roi",
                    &ImageWindowWrapper::getROI,
                    &ImageWindowWrapper::setROI)
            ;

    class_<v3dhandleList>("ImageWindowList");
    class_<Image4DSimple>("Image4DSimple");
    class_<LandmarkList>("LandmarkList");
    class_<ROIList>("ROIList");

    // We probably don't want to use the V3DPluginCallback2 methods in the python
    // API, but it is the basis of the V3D plugin interface.
    class_<V3DPluginCallback2, boost::noncopyable>(
            "V3DPluginCallback2", no_init)
            // virtual v3dhandleList getImageWindowList() const = 0;
            .def("getImageWindowList",
                    &V3DPluginCallback2::getImageWindowList)
            // virtual v3dhandle currentImageWindow() = 0;
            .def("currentImageWindow", callback_currentImageWindow)
            // virtual v3dhandle curHiddenSelectedWindow() = 0; //by PHC, 20101009. curHiddenSelectedWindow may not be the *currentImageWindow* if the selection is done from a 3d viewer
            .def("curHiddenSelectedWindow", callback_curHiddenSelectedWindow)
            // virtual v3dhandle newImageWindow(QString name="new_image") = 0;
            .def("newImageWindow", callback_newImageWindow)
            // virtual void updateImageWindow(v3dhandle image_window) = 0;
            .def("updateImageWindow", callback_updateImageWindow)
            // virtual QString getImageName(v3dhandle image_window) const = 0;
            .def("getImageName", callback_getImageName)
            // virtual void setImageName(v3dhandle image_window, QString name) = 0;
            .def("setImageName", callback_setImageName)
            // virtual Image4DSimple * getImage(v3dhandle image_window) = 0;
            .def("getImage", callback_getImage,
                    return_value_policy<reference_existing_object>())
            // virtual bool setImage(v3dhandle image_window, Image4DSimple * image) = 0;
            .def("setImage", callback_setImage)
            //virtual LandmarkList  getLandmark(v3dhandle image_window) = 0;
            .def("getLandmark", callback_getLandmark)
            //virtual bool setLandmark(v3dhandle image_window, LandmarkList & landmark_list) = 0;
            //virtual ROIList getROI(v3dhandle image_window) = 0;
            //virtual bool setROI(v3dhandle image_window, ROIList & roi_list) = 0;
            //virtual NeuronTree getSWC(v3dhandle image_window) = 0;
            //virtual bool setSWC(v3dhandle image_window, NeuronTree & nt) = 0;
            //virtual V3D_GlobalSetting getGlobalSetting() = 0;
            //virtual bool setGlobalSetting( V3D_GlobalSetting & gs ) = 0;

            ;

    def("getCallback", getCallback,
            return_value_policy<reference_existing_object>(),
            "Get the V3D plugin API callback object.");
}

namespace v3d {

void initV3DPythonModule(V3DPluginCallback2 *callback)
{
    initv3d();
    if (callback) callbackPtr = callback;
}

} // namespace v3d
