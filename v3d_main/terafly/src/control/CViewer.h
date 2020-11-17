//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
* 2014-11-17. Alessandro. @ADDED 'anoV0', ..., 'anoD1' VOI annotation (global) coordinates as object members in order to fix duplicated annotations bug
*/

#ifndef CEXPLORERWINDOW_H
#define CEXPLORERWINDOW_H

#include "v3dr_common.h"

#include "CPlugin.h"
#include "v3dr_mainwindow.h"
#include "CImport.h"
#include "v3d_imaging_para.h"
#include "V3Dsubclasses.h"

#ifdef _NEURON_ASSEMBLER_
class terafly::CViewer : public QWidget, public INeuronAssembler
{
    Q_OBJECT
	Q_INTERFACES(INeuronAssembler) // INeuronAssembler is NOT a Q_OBJECT, hence Q_INTERFACES macro is needed.

#else
class terafly::CViewer : public QWidget
{
	Q_OBJECT
#endif

    private:

        //OBJECT members
        V3DPluginCallback2* V3D_env;    //handle of V3D environment
        v3dhandle window;               //generic (void *) handle of the tri-view image window
        XFormWidget* triViewWidget;     //the tri-view image window
        V3dR_GLWidget* view3DWidget;    //3D renderer widget associated to the image window
        //myV3dR_GLWidget* view3DWidget;
        V3dR_MainWindow* window3D;      //the window enclosing <view3DWidget>
        CViewer *next, *prev;   //the next (higher resolution) and previous (lower resolution) <CExplorerWindow> objects
        int volResIndex;                //resolution index of the volume displayed in the current window (see member <volumes> of CImport)
        tf::uint8* imgData;
        int volV0, volV1;               //first and last vertical coordinates of the volume displayed in the current window
        int volH0, volH1;               //first and last horizontal coordinates of the volume displayed in the current window
        int volD0, volD1;               //first and last depth coordinates of the volume displayed in the current window
        int volT0, volT1;               //first and last time coordinates of the volume displayed in the current window
        int nchannels;                  //number of image channels
        string title;                   //title of current window
        string titleShort;              //short title of current window
        bool toBeClosed;                //true when the current window is marked as going to be closed
        bool _isActive;                  //false when the current window is set as not active (e.g. when after zooming-in/out)
        bool _isReady;                   //true when current window is ready for receiving user inputs (i.e. all image data have been loaded)
        bool has_double_clicked;        //true when a double click event has been just catched (will be set to false when the double click is managed)
        int zoomHistory[ZOOM_HISTORY_SIZE];//last 4 zoom values
        int V0_sbox_min, V0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int V1_sbox_max, V1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int H0_sbox_min, H0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int H1_sbox_max, H1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int D0_sbox_min, D0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int D1_sbox_max, D1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int T0_sbox_min, T0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int T1_sbox_max, T1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int ID;
        bool waitingForData;              //"waiting for 5D data" state flag
        QUndoStack undoStack;           //stack containing undo command actions
        int slidingViewerBlockID;
        bool forceZoomIn;
        bool insituZoomOut;
        int anoV0, anoV1;               // @ADDED by Alessandro on 2014-11-17. First and last global coordinates of the annotation space along V (annotation VOI != VOI)
        int anoH0, anoH1;               // @ADDED by Alessandro on 2014-11-17. First and last global coordinates of the annotation space along H (annotation VOI != VOI)
        int anoD0, anoD1;               // @ADDED by Alessandro on 2014-11-17. First and last global coordinates of the annotation space along D (annotation VOI != VOI)
        int insituZoomOut_x, insituZoomOut_y, insituZoomOut_z, insituZoomOut_res;
        int insituZoomOut_dx, insituZoomOut_dy, insituZoomOut_dz;
        bool isTranslate;
        bool toRetrieveData;
        int zoomOutSize_x, zoomOutSize_y, zoomOutSize_z;

        //CLASS members
        static CViewer *first;  //pointer to the first window of the multiresolution explorer windows chain
        static CViewer *last;   //pointer to the last window of the multiresolution explorer windows chain
        static CViewer *current;//pointer to the current window of the multiresolution explorer windows chain
        static int nInstances;          //number of instantiated objects
        static int nTotalInstances;

        //MUTEX
        //QMutex updateGraphicsInProgress;

        //TIME MEASURE
        QElapsedTimer newViewerTimer;
        static int newViewerOperationID;

        //inhibiting default constructor
        CViewer();


        /**********************************************************************************
        * Returns the most  likely 3D  point in the  image that the user is pointing on the
        * renderer at the given location.
        * This is based on the Vaa3D 3D point selection with one mouse click.
        ***********************************************************************************/
        XYZ getRenderer3DPoint(int x, int y) throw (tf::RuntimeException);

        /**********************************************************************************
        * Syncronizes widgets from <src> to <dst>
        ***********************************************************************************/
        void syncWindows(V3dR_MainWindow* src, V3dR_MainWindow* dst);


    public:

        //CONSTRUCTOR, DECONSTRUCTOR
        CViewer(V3DPluginCallback2* _V3D_env, int _resIndex, tf::uint8* _imgData, int _volV0, int _volV1,
                        int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, CViewer* _prev, int _slidingViewerBlockID = -1);
        ~CViewer();
        static void uninstance()
        {
            /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

            while(first){
                CViewer* p = first->next;
                first->toBeClosed = true;
                delete first;
                first = p;
            }
            first=last=0;
            current = 0; //20170804 RZC: add for bad pointer from getCurrent(), so many bad pointers!
        }

        //performs all the operations needed to show 3D data (such as creating Vaa3D widgets)
        void show();

        //safely close this viewer
        void close();

        //GET methods
        static CViewer* getCurrent(){return current;}
        int getResIndex(){return volResIndex;}
        V3dR_MainWindow* getWindow3D(){return window3D;}
        V3dR_GLWidget* getGLWidget(){return view3DWidget;}
        //myV3dR_GLWidget* getGLWidget(){return view3DWidget;}
        bool isHighestRes(){return volResIndex == CImport::instance()->getResolutions() -1;}
        bool isWaitingForData(){return waitingForData;} // waiting state
        bool isReady(){return _isReady;}                // ready for user input
        bool isActive(){return _isActive;}              // active (=visible)
        bool isInSafeState(){return _isReady && _isActive && !waitingForData && !toBeClosed;}

        /**********************************************************************************
        * Filters events generated by the 3D rendering window <view3DWidget>
        * We're interested to intercept these events to provide many useful ways to explore
        * the 3D volume at different resolutions without changing Vaa3D code.
        ***********************************************************************************/
        bool eventFilter(QObject *object, QEvent *event);
		NeuronTree treeGlobalCoords;    // preserving global coordinates of SWC, MK, April, 2018
		NeuronTree convertedTreeCoords; // local coordinates of SWC, used for computing the distance from the place where double-click happens, MK, April, 2018
		
		bool volumeCutSbAdjusted;
		bool xMinAdjusted, xMaxAdjusted, yMinAdjusted, yMaxAdjusted, zMinAdjusted, zMaxAdjusted;
		inline int getXDim() { return (this->volH1 - this->volH0); }
		inline int getYDim() { return (this->volV1 - this->volV0); }
		inline int getZDim() { return (this->volD1 - this->volD0); }

#ifdef _NEURON_ASSEMBLER_
		/* ====================================================================================================
		 * In order to simplify the communication between Neuron Assembler plugin and terafly environment,
		 * I make INeuronAsembler 1 of CViewer's bases (with conditional preprocessor macro '_NEURON_ASSEMBLER_').
		 * This preprocessor block lists functionalities that can be accessed through this interface.
		 * Note, this is a bypass of [V3DPluginCallback2 -> V3d_PluginLoader -> CPlugin -> CViewer] route.
		 * Through this interface, the plugin directly talks to CViewer through [INeuronAssembler].
		 *                                                                                   -- MK, Dec, 2019
		 * ==================================================================================================== */
		virtual bool teraflyImgInstance();

		virtual void sendCastNAUI2PMain(IPMain4NeuronAssembler* NAportal);
		//virtual void forceCViewerPortalUpdate();
		virtual bool checkFragTraceStatus();
		virtual void changeFragTraceStatus(bool newStatus);
		virtual int getViewerID() { return this->ID; }
		virtual void printOutCViewerAddress() { cout << " => Current CViewer address: " << CViewer::getCurrent() << endl; }

		virtual string getCviewerWinTitle() { return CViewer::getCurrent()->title; }
		virtual void printoutWinTitle() { cout << CViewer::getCurrent()->title << endl; }
		virtual int getTeraflyTotalResLevel() { return CImport::instance()->getResolutions(); }
		virtual int getTeraflyResLevel() { return CViewer::getCurrent()->getResIndex(); }
		virtual int getZoomingFactor() { return CViewer::getCurrent()->getGLWidget()->_zoom; }
		virtual bool getXlockStatus();
		virtual bool getYlockStatus();
		virtual bool getZlockStatus();
		virtual bool getPartialVolumeCoords(int globalCoords[], int localCoords[], int displayingVolDims[]);

		QList<ImageMarker> selectedMarkerList;
		QList<ImageMarker> selectedLocalMarkerList;
		QList<ImageMarker> up2dateMarkerList;
		virtual void refreshSelectedMarkers();
		virtual void pushMarkersfromTester(const set<vector<float>>& markerCoords, RGBA8 color);

		string editingMode;
		int mouseX, mouseY;
		int eraserSize, connectorSize;
		map<int, vector<NeuronSWC>> seg2includedNodeMap;
		set<int> deletedSegsIDs;
		virtual vector<V_NeuronSWC>* getDisplayingSegs();
		virtual void updateDisplayingSegs();
		virtual void editingModeInit() { CViewer::getCurrent()->editingMode = "none"; }
		virtual void setEraserSize(int newEraserSize) { CViewer::getCurrent()->eraserSize = newEraserSize; }
		virtual int getEraserSize() { return CViewer::getCurrent()->eraserSize; }
		virtual void setConnectorSize(int newConnectorSize) { CViewer::getCurrent()->connectorSize = newConnectorSize; }
		virtual int getConnectorSize() { return CViewer::getCurrent()->connectorSize; }
		virtual void segEditing_setCursor(string action);		
		virtual int getNearestSegEndClusterCentroid(const boost::container::flat_map<int, vector<float>>& segEndClusterCentroidMap);
		virtual void convertLocalCoord2windowCoord(const float localCoord[], float windowCoord[]);
		virtual void convertWindowCoord2likelyLocalCoord(const int mouseX, const int mouseY, float putativeCoord[]);

		virtual void getParamsFromFragTraceUI(const string& keyName, const float& value);
		
#endif

        /**********************************************************************************
        * Restores the current viewer from the given (neighboring) source viewer.
        * Called by the next(prev) <CExplorerWindow>  when the user  zooms out(in) and  the
        * lower(higher) resoolution has to be reestabilished.
        ***********************************************************************************/
        void restoreViewerFrom(CViewer* source) throw (tf::RuntimeException);

        /**********************************************************************************
        * Generates a new viewer using the given coordinates.
        * Called by the current <CExplorerWindow> when the user zooms in and the higher res-
        * lution has to be loaded.
        ***********************************************************************************/
        void
        newViewer(
            int x, int y, int z,                //can be either the VOI's center (default)
                                                //or the VOI's ending point (see x0,y0,z0)
            int resolution,                     //resolution index of the view requested
            int t0, int t1,                     //time frames selection
            int dx=-1, int dy=-1, int dz=-1,    //VOI [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz)
            int x0=-1, int y0=-1, int z0=-1,    //VOI [x0, x), [y0, y), [z0, z)
            bool auto_crop = true,              //whether to crop the VOI to the max dims
            bool scale_coords = true,           //whether to scale VOI coords to the target res
            int sliding_viewer_block_ID = -1    //block ID in "Sliding viewer" mode
        );

        /**********************************************************************************
        * Refresh image data (if viewer is not busy - otherwise no refresh is possible)
        ***********************************************************************************/
        void refresh() throw (tf::RuntimeException);

        /**********************************************************************************
        * Resizes  the  given image subvolume in a  newly allocated array using the fastest
        * achievable interpolation method. The image currently shown is used as data source.
        * Missing pieces of data are filled with black and returned to the caller.
        ***********************************************************************************/
        tf::uint8*
            getVOI(int x0, int x1,              // VOI [x0, x1) in the local reference sys
                   int y0, int y1,              // VOI [y0, y1) in the local reference sys
                   int z0, int z1,              // VOI [z0, z1) in the local reference sys
                   int t0, int t1,              // VOI [t0, t1] in the local reference sys
                   int xDimInterp,              // interpolated VOI dimension along X
                   int yDimInterp,              // interpolated VOI dimension along Y
                   int zDimInterp,              // interpolated VOI dimension along Z
                   int& x0m, int& x1m,          // black-filled VOI [x0m, x1m) in the local rfsys
                   int& y0m, int& y1m,          // black-filled VOI [y0m, y1m) in the local rfsys
                   int& z0m, int& z1m,          // black-filled VOI [z0m, z1m) in the local rfsys
                   int& t0m, int& t1m)          // black-filled VOI [t0m, t1m] in the local rfsys
        throw (tf::RuntimeException);

        /**********************************************************************************
        * Returns  the  maximum intensity projection  of the given VOI in a newly allocated
        * array. Data is taken from the currently displayed image.
        ***********************************************************************************/
        tf::uint8*
            getMIP(int x0, int x1,              // VOI [x0, x1) in the local reference sys
                   int y0, int y1,              // VOI [y0, y1) in the local reference sys
                   int z0, int z1,              // VOI [z0, z1) in the local reference sys
                   int t0 = -1, int t1 = -1,    // VOI [t0, t1] in the local reference sys
                   tf::direction dir = tf::z,
                   bool to_BGRA = false,        //true if mip data must be stored into BGRA format
                   tf::uint8 alpha = 255)      //alpha transparency used if to_BGRA is true
        throw (tf::RuntimeException);


        /**********************************************************************************
        * Makes the current view the last one by  deleting (and deallocting) its subsequent
        * views.
        ***********************************************************************************/
        void makeLastView() throw (tf::RuntimeException);

        /**********************************************************************************
        * Annotations are stored/loaded to/from the <CAnnotations> object
        ***********************************************************************************/
        void storeAnnotations() throw (tf::RuntimeException);
        void loadAnnotations() throw (tf::RuntimeException);
        void clearAnnotations() throw (tf::RuntimeException);
        void deleteSelectedMarkers() throw (tf::RuntimeException);
        void deleteMarkerAt(int x, int y, QList<LocationSimple>* deletedMarkers = 0) throw (tf::RuntimeException);
        void createMarkerAt(int x, int y) throw (tf::RuntimeException);
        void createMarker2At(int x, int y) throw (tf::RuntimeException);
        void updateAnnotationSpace() throw (tf::RuntimeException);

        /**********************************************************************************
        * Saves/restores the state of PMain spinboxes for subvolume selection
        ***********************************************************************************/
        void saveSubvolSpinboxState();
        void restoreSubvolSpinboxState();

        /**********************************************************************************
        * method (indirectly) invoked by Vaa3D to propagate VOI's coordinates
        ***********************************************************************************/
        void invokedFromVaa3D(v3d_imaging_paras* params = 0);

        /**********************************************************************************
        * Alignes the given widget to the right of the current window
        ***********************************************************************************/
        void alignToRight(QWidget* widget, QEvent* evt);

        /**********************************************************************************
        * Overriding position, size and resize QWidget methods
        ***********************************************************************************/
        inline QPoint pos() const{ return window3D->pos(); }
        inline QSize size() const{ return window3D->size();}
        inline void resize(QSize new_size){
            if(window3D->size() != new_size)
                window3D->resize(new_size);
        }
        inline void move(QPoint p){
            if(pos().x() != p.x() || pos().y() != p.y())
                window3D->move(p);
        }
        inline void resize(int w, int h)  { resize(QSize(w, h)); }
        inline void move(int ax, int ay)  { move(QPoint(ax, ay)); }

        /**********************************************************************************
        * Activates / deactives the current window (in terms of responding to events)
        ***********************************************************************************/
        void setActive(bool active)
        {
            /**/tf::debug(tf::LEV1, strprintf("title = %s, active = %s", titleShort.c_str() , active ? "true" : "false").c_str(), __itm__current__function__);

            _isActive = active;
            if(!_isActive)
                _isReady = false;
        }

        /**********************************************************************************
        * Change to "waiting" state (i.e., when image data are to be loaded or are loading)
        ***********************************************************************************/
        void setWaitingForData(bool wait, bool pre_wait=false);

        /**********************************************************************************
        * Zoom history methods (inline because the are called frequently)
        ***********************************************************************************/
        inline void resetZoomHistory(){
            for(int i=0; i<ZOOM_HISTORY_SIZE; i++)
                zoomHistory[i] = std::numeric_limits<int>::max();
        }
        inline bool isZoomDerivativePos(){
            for(int i=1; i<ZOOM_HISTORY_SIZE; i++)
                if(zoomHistory[i-1] == std::numeric_limits<int>::max() || zoomHistory[i] <= zoomHistory[i-1])
                    return false;
            return true;
        }
        inline bool isZoomDerivativeNeg(){
            for(int i=1; i<ZOOM_HISTORY_SIZE; i++)
                if(zoomHistory[i-1] == std::numeric_limits<int>::max() || zoomHistory[i] >= zoomHistory[i-1])
                    return false;
            return true;
        }
        inline void zoomHistoryPushBack(int zoom){
            if(zoomHistory[ZOOM_HISTORY_SIZE-1] != zoom)
                for(int i=0; i<ZOOM_HISTORY_SIZE-1; i++)
                    zoomHistory[i] = zoomHistory[i+1];
            zoomHistory[ZOOM_HISTORY_SIZE-1] = zoom;
        }

        template<class T>
        inline bool is_outside(T x, T y, T z){
           return
                   x < 0  || x >= volH1-volH0 ||
                   y < 0  || y >= volV1-volV0 ||
                   z < 0  || z >= volD1-volD0;
        }

        /**********************************************************************************
        * Change current Vaa3D's rendered cursor
        ***********************************************************************************/
        static void setCursor(const QCursor& cur, bool renderer_only = false);

        //PMain instance is allowed to access class private members
        friend class PMain;
        friend class PAnoToolBar;
        friend class PTabVolumeInfo;
        friend class CAnnotations;
        friend class CVolume;
        friend class TeraFly;
        friend class QUndoMarkerCreate;
        friend class QUndoMarkerDelete;
        friend class QUndoMarkerDeleteROI;
        friend class QUndoVaa3DNeuron;


    public slots:

        /*********************************************************************************
        * Receive data (and metadata) from <CVolume> throughout the loading process
        **********************************************************************************/
        void receiveData(
                tf::uint8* data,                   // data (any dimension)
                tf::integer_array data_s,          // data start coordinates along X, Y, Z, C, t
                tf::integer_array data_c,          // data count along X, Y, Z, C, t
                QWidget* dest,                         // address of the listener
                bool finished,                      // whether the loading operation is terminated
                tf::RuntimeException* ex = 0,      // exception (optional)
                qint64 elapsed_time = 0,            // elapsed time (optional)
                QString op_dsc="",                  // operation descriptor (optional)
                int step=0);                        // step number (optional)


        /**********************************************************************************
        * Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
        * This implements the syncronization Vaa3D-->TeraFly of subvolume selection.
        ***********************************************************************************/
        void Vaa3D_changeXCut0(int s);
        void Vaa3D_changeXCut1(int s);
        void Vaa3D_changeYCut0(int s);
        void Vaa3D_changeYCut1(int s);
        void Vaa3D_changeZCut0(int s);
        void Vaa3D_changeZCut1(int s);
        void Vaa3D_changeTSlider(int s, bool editingFinished = false);
        void ShiftToAnotherDirection(int direction);


        /**********************************************************************************
        * Linked to PMain GUI VOI's widgets.
        * This implements the syncronization TeraFly-->Vaa3D of subvolume selection.
        ***********************************************************************************/
        void PMain_changeV0sbox(int s);
        void PMain_changeV1sbox(int s);
        void PMain_changeH0sbox(int s);
        void PMain_changeH1sbox(int s);
        void PMain_changeD0sbox(int s);
        void PMain_changeD1sbox(int s);

        /**********************************************************************************
        * Linked to PMain GUI<->QGLRefSys widget.
        * This implements the syncronization Vaa3D<-->TeraFly of rotations.
        ***********************************************************************************/
        void Vaa3D_rotationchanged(int s);
        void PMain_rotationchanged();

        /**********************************************************************************
        * Linked to Vaa3D renderer slider
        ***********************************************************************************/
        void setZoom(int z);

        // translate zoom out view at the same resolution
        void translate();

        void zoomOutMethodChanged(int value);

        void inSituZoomOutTranslated();

        void resetEvents();

    public:

        /**********************************************************************************
        * utility method: return volume dimension along the given direction
        ***********************************************************************************/
        inline int dimension(iim::axis dir) throw (tf::RuntimeException)
        {
            if(dir == iim::vertical || dir == iim::inv_vertical)
                return volV1-volV0;
            else if(dir == iim::horizontal || dir == iim::inv_horizontal)
                return volH1-volH0;
            else if(dir == iim::depth || dir == iim::inv_depth)
                return volD1-volD0;
            else
                throw tf::RuntimeException("CViewer::getDIM(): axis invalid");
        }

        /**********************************************************************************
        * utility method: return current origin coordinate along the given direction
        ***********************************************************************************/
        inline int origin(iim::axis dir) throw (tf::RuntimeException)
        {
            if(dir == iim::vertical || dir == iim::inv_vertical)
                return volV0;
            else if(dir == iim::horizontal || dir == iim::inv_horizontal)
                return volH0;
            else if(dir == iim::depth || dir == iim::inv_depth)
                return volD0;
            else
                throw tf::RuntimeException("CViewer::getDIM(): axis invalid");
        }

        /**********************************************************************************
        * utility method: return Vaa3D viewer limit along the given direction
        ***********************************************************************************/
        template<typename T>
        T vaa3dLimit(iim::axis dir) throw (tf::RuntimeException)
        {
            if(dir == iim::vertical || dir == iim::inv_vertical)
                return static_cast<T>(LIMIT_VOLY);
            else if(dir == iim::horizontal || dir == iim::inv_horizontal)
                return static_cast<T>(LIMIT_VOLX);
            else if(dir == iim::depth || dir == iim::inv_depth)
                return static_cast<T>(LIMIT_VOLZ);
            else
                throw tf::RuntimeException("CViewer::getDIM(): axis invalid");
        }

        /**********************************************************************************
        * Maps local coordinate to the global image space of the selected resolution
        ***********************************************************************************/
        template <typename T>
        inline T coord2global(
                T local,                        // local coordinate in current image space [0, dim) or in the Vaa3D resampled space [0, LIMIT_VOL)
                iim::axis dir,                  // direction (x, y or z)
                bool round,                     // whether to round coordinate to nearest integer
                int res            = -1,        // resolution index of destination image space (0 = lowest-res, the higher the higher resolution)
                bool fromVaa3D     = false,     // whether 'local' comes from the Vaa3D resampled space [0, LIMIT_VOL)
                bool cutOutOfRange = false,     // whether 'local' should be adjusted to fit within the current image space
                const char *src    = 0)
        {
            #ifdef terafly_enable_debug_max_level
            /**/tf::debug(tf::LEV3, tf::strprintf("title = %s, local = %s, dir = %d, res = %d, round = %s, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                                titleShort.c_str(), tf::num2str(local).c_str(), dir, res, round ? "true" : "false", fromVaa3D ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);
            #endif

            // set default res
            if(res == -1)
                res = CImport::instance()->getResolutions()-1;


            // if required, adjust 'local' to fit within current image space
            if(cutOutOfRange)
            {
                local = local <  0 ? 0 : local;
                local = local >= CImport::instance()->getVolume(volResIndex)->getDIM(dir) ? CImport::instance()->getVolume(volResIndex)->getDIM(dir) : local;

                #ifdef terafly_enable_debug_max_level
                tf::debug(tf::LEV3, strprintf("cutOutOfRange, local --> %s", tf::num2str(local).c_str()).c_str(), __itm__current__function__);
                #endif
            }

            // if required, map 'local' from the resampled Vaa3D image space [0, LIMIT_VOL) to the current image space [0, dim]
            if(fromVaa3D && (dimension(dir) > vaa3dLimit<T>(dir)))
            {
                local = round ? tf::round( local* dimension(dir)/vaa3dLimit<float>(dir) ) : ( local* dimension(dir)/vaa3dLimit<float>(dir) );

                #ifdef terafly_enable_debug_max_level
                tf::debug(tf::LEV3, strprintf("map 2 Vaa3D, local --> %s", tf::num2str(local).c_str()).c_str(), __itm__current__function__);
                #endif
            }


            // special case: 2D image
            if(CImport::instance()->getVolume(volResIndex)->getDIM(dir) == 1)
            {
                #ifdef terafly_enable_debug_max_level
                /**/tf::debug(tf::LEV3, strprintf("2D image, return %d",
                                                    local ? CImport::instance()->getVolume(res)->getDIM(dir) : 0).c_str(), __itm__current__function__);
                #endif

                return local ? CImport::instance()->getVolume(res)->getDIM(dir) : 0;
            }

            // special case: boundary coordinate
            if(local >= CImport::instance()->getVolume(volResIndex)->getDIM(dir))
            {
                #ifdef terafly_enable_debug_max_level
                tf::debug(tf::LEV3, strprintf("boundary coordinate, return %d",
                                                    CImport::instance()->getVolume(res)->getDIM(dir)).c_str(), __itm__current__function__);
                #endif
                return CImport::instance()->getVolume(res)->getDIM(dir);
            }

            // normal case: scale coordinate
            float rescale = CImport::instance()->getRescaleFactor(res, volResIndex, dir);

            #ifdef terafly_enable_debug_max_level
            /**/tf::debug(tf::LEV3, strprintf("normal case, rescale = %f, return %s", rescale, tf::num2str(round? tf::round((origin(dir)+local)*rescale) : (origin(dir)+local)*rescale).c_str()).c_str(), __itm__current__function__);
            #endif

            return round? tf::round((origin(dir)+local)*rescale) : (origin(dir)+local)*rescale;
        }


        template <typename T>
        inline T coord2local(
                T global,                       // global coordinate in highest-res image space [0, dim)
                iim::axis dir,                  // direction (x, y or z)
                bool round,                     // whether to round coordinate to nearest integer
                bool toVaa3D = false)           // whether local coordinate should be computed in the Vaa3D resampled space [0, LIMIT_VOL)
        {
            #ifdef terafly_enable_debug_max_level
            /**/tf::debug(tf::LEV3, strprintf("title = %s, global = %s, toVaa3D = %s",
                                                titleShort.c_str(), tf::num2str(global).c_str(), toVaa3D ? "true" : "false").c_str(), __itm__current__function__);
            #endif

            // special case: boundary coordinate
            if(global == CImport::instance()->getHighestResVolume()->getDIM(dir))
            {
                #ifdef terafly_enable_debug_max_level
                /**/tf::debug(tf::LEV3, strprintf("boundary coordinate, return %d", CImport::instance()->getVolume(volResIndex)->getDIM(dir)).c_str(), __itm__current__function__);
                #endif

                return CImport::instance()->getVolume(volResIndex)->getDIM(dir);
            }

            // special case: 2D image
            if(CImport::instance()->getVolume(volResIndex)->getDIM(dir) == 1)
            {
                #ifdef terafly_enable_debug_max_level
                /**/tf::debug(tf::LEV3, strprintf("2D image, return %d", global ? CImport::instance()->getVolume(volResIndex)->getDIM(dir) : 0).c_str(), __itm__current__function__);
                #endif

                return global ? CImport::instance()->getVolume(volResIndex)->getDIM(dir) : 0;
            }

            // normal case
            float rescale = CImport::instance()->getRescaleFactor(CImport::instance()->getResolutions()-1, volResIndex, dir);
            T local =  round ? tf::round(global/rescale - origin(dir)) : global/rescale - origin(dir);

            #ifdef terafly_enable_debug_max_level
            /**/tf::debug(tf::LEV3, strprintf("rescale = %f, local = %s", rescale, tf::num2str(local).c_str()).c_str(), __itm__current__function__);
            #endif

            // map local coordinate to Vaa3D viewer coordinate
            if(toVaa3D && (dimension(dir) > vaa3dLimit<T>(dir)))
            {
                local = round ? tf::round( local* vaa3dLimit<float>(dir)/dimension(dir) ) : local* vaa3dLimit<float>(dir)/dimension(dir);

                #ifdef terafly_enable_debug_max_level
                /**/tf::debug(tf::LEV3, strprintf("vaa3d correction, local --> %s", tf::num2str(local).c_str()).c_str(), __itm__current__function__);
                #endif
            }

            return local;
        }

        // return Vaa3D image
        const Image4DSimple* getImage() throw (tf::RuntimeException);

        void  setImage(int x, int y, int z) throw (tf::RuntimeException);

};

#endif // CEXPLORERWINDOW_H
