#ifndef WRAPPABLE_V3D_H_
#define WRAPPABLE_V3D_H_

#include "v3d_interface.h"
#include "v3d_qt_environment.h"

std::string get_argitem_type(const V3DPluginArgItem& item);
void set_argitem_type(V3DPluginArgItem& item, const std::string& s);
std::string get_argitem_pointer(const V3DPluginArgItem& item);
void set_argitem_pointer(V3DPluginArgItem& item, void* ptr);

// Unfortunately, we might have to instantiate each possible subtype of V3DPluginArg
// to expose it to python
template<class T>
class V3DPluginArg : public V3DPluginArgItem {
public:
    V3DPluginArg(T& item, const std::string& typeName) {
        setItem(item);
        setType(typeName);
    }
    const T& getItem() const {
        const T* ptr = static_cast<const T*>(p);
        return *ptr;
    }
    void setItem(T& item) {p = (void*) &item;}
    std::string getType() const {return type.toStdString();}
    void setType(const std::string& s) {type = QString::fromStdString(s);}
};

// typedef c_array< double, 3 > double3;
// typedef c_array< double3, 3 > double3x3;

// "extern" to avoid multiply defined symbol error on Mac
#ifdef _MSC_VER
unsigned int qHash(const LocationSimple& loc);
unsigned int qHash(const QPolygon&);
unsigned int qHash(const V3DPluginArgItem& lhs);
bool operator==(const LocationSimple&, const LocationSimple&);
// operator== needed for wrapping QList<V3DPluginArgItem>
bool operator==(const V3DPluginArgItem& lhs, const V3DPluginArgItem& rhs);
template class QList<LocationSimple>;
template class QVector<QPoint>;
template class QList<QPolygon>;
template class QHash<int, int>;
template class QList<V3DPluginArgItem>;
template class c_array< double, 3 >;
template class c_array< c_array< double, 3 >, 3 >;
template class V3DPluginArg< c_array< c_array< double, 3 >, 3 > >;
#else
extern template class QList<LocationSimple>;
extern template class QVector<QPoint>;
extern template class QList<QPolygon>;
extern template class QHash<int, int>;
extern template class QList<V3DPluginArgItem>;
extern template class c_array< double, 3 >;
extern template class c_array< c_array< double, 3 >, 3 >;
extern template class V3DPluginArg< c_array< c_array< double, 3 >, 3 > >;
#endif

/*! \brief Returns general parameters of the V3D program.
 *
 */
V3D_GlobalSetting getGlobalSetting();

/*! \brief Calls a function in a dynamically loaded V3D plugin module.
 *
 */
bool callPluginFunc(const QString & plugin_name, const QString & func_name,
        const V3DPluginArgList & input, V3DPluginArgList & output);

/*! \brief Sets general parameters of the V3D program.
 *
 */
bool setGlobalSetting(V3D_GlobalSetting& gs);

// Sends signals from non-GUI thread
class ImageWindowDispatcher : public QObject {
	Q_OBJECT
public:
	ImageWindowDispatcher() : QObject(NULL) {}
	void emitOpen3DWindow() {emit open3DWindow();}

signals:
	void open3DWindow();
};

// Receives signals in GUI thread
class ImageWindowReceiver : public QObject {
	Q_OBJECT

public:
	friend class ImageWindow;
	ImageWindowReceiver() : QObject(NULL) {}

public slots:
	void open3DWindow();

private:
	void *handle;
};

// Wrap v3dhandle, which is actually "void*", so python does not choke on it.

/*! \brief A primary "TriView" window in the V3D interface.
 *
 */
class ImageWindow
{
public:

    /*! Returns the currently displayed V3D image window.
     */
    static ImageWindow current();
    static ImageWindow currentHiddenSelected();
    ImageWindow(void* h);

    /*! Creates a new V3D Image window.
     *
     * @param name the name that appears in the title bar of the new window.
     */
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

    /*! Creates or opens a 3D volume visualization window showing the
     * same image as this ImageWindow.
     */
    void open3DWindow();
    void close3DWindow();
    void openROI3DWindow();
    void closeROI3DWindow();
    void pushObjectIn3DWindow();
    void pushImageIn3DWindow();
    int pushTimepointIn3DWindow(int timepoint);
    bool screenShot3DWindow(const std::string& BMPfilename);
    bool screenShotROI3DWindow(const std::string& BMPfilename);

    /*! Returns a View3DControl object, used to manipulate the 3D
     * volume window.
     */
    View3DControl* getView3DControl();
    View3DControl* getLocalView3DControl();
    TriviewControl* getTriviewControl();

private:
    void* handle;
    ImageWindowDispatcher *dispatcher;
    ImageWindowReceiver *receiver;
};

namespace pyplusplus { namespace alias {
    // Need to wrap QVector<QPoint> to avoid runtime error at startup on Mac
    // This wrapper is hollow.
    typedef QVector<QPoint> QVector_QPoint;
    typedef c_array<unsigned char, 3> c_array_uint_3;
    typedef c_array<int, 3> c_array_int_3;
    typedef c_array<unsigned char, 4> c_array_uint_4;
    typedef c_array<float, 3> c_array_float_3;
    typedef c_array<short, 3> c_array_short_3;

    typedef c_array< double, 3 > double3;
    typedef c_array< double3, 3 > double3x3;
    typedef V3DPluginArg<double3x3> V3DPluginArg_double3x3;
}}

#endif /* WRAPPABLE_V3D_H_ */
