#ifndef WRAPPABLE_V3D_H_
#define WRAPPABLE_V3D_H_

#include "v3d_interface.h"

// TODO can globalSetting be a property of the module?
V3D_GlobalSetting getGlobalSetting();
bool setGlobalSetting(V3D_GlobalSetting& gs);

// Wrap v3dhandle, which is actually "void*", so python does not choke on it.
class ImageWindow
{
public:
    /*! \brief Returns the currently displayed V3D image window.
     */
    static ImageWindow current();
    static ImageWindow currentHiddenSelected();
    ImageWindow(void* h);
    ImageWindow(const std::string& name);
    void update();
    std::string getName() const;
    void setName(const std::string& name);
    Image4DSimple* getImage();
    bool setImage(Image4DSimple* image);
    LandmarkList getLandmark();
    bool setLandmark(LandmarkList & landmark_list);
    ROIList getROI();
    bool setROI(ROIList & roi_list);
    NeuronTree getSWC();
    bool setSWC(NeuronTree & nt);
    void open3DWindow();
    void close3DWindow();
    void openROI3DWindow();
    void closeROI3DWindow();
    void pushObjectIn3DWindow();
    void pushImageIn3DWindow();
    int pushTimepointIn3DWindow(int timepoint);
    bool screenShot3DWindow(const std::string& BMPfilename);
    bool screenShotROI3DWindow(const std::string& BMPfilename);
    View3DControl* getView3DControl();
    View3DControl* getLocalView3DControl();
    TriviewControl* getTriviewControl();

private:
    void* handle;
};

#endif /* WRAPPABLE_V3D_H_ */
