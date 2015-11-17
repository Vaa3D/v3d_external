#ifndef __NEURON_GAME_3D_VIEW_H__
#define __NEURON_GAME_3D_VIEW_H__

#include "v3dr_common.h"
#include "../terafly/src/control/CViewer.h"

namespace neurongame
{
	class NeuronGameUI;
	class NeuronGame3DView;
}

class neurongame::NeuronGame3DView : protected teramanager::CViewer
{
	Q_OBJECT

	protected:
		//static NeuronGame3DView* neuronView;
		/**************************************************************************************
		* Constructor needs to be protected because inherits from protected CViewer constructor
		***************************************************************************************/
		NeuronGame3DView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
			int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID);
		virtual teramanager::CViewer* makeView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
			int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID);
		static int contrastValue;
		Image4DSimple* nextImg;
		bool loadingNextImg;

	public:

		// helping functions copied from renderer_gl2.h
		inline void set_colormap_curve(QPolygonF &curve, qreal x, int iy) // 0.0<=(x)<=1.0, 0<=(iy)<=255
		{
			x = qMax(0.0, qMin(1.0,  x));
			qreal y = qMax(0.0, qMin(1.0,  iy/255.0));
			curve << QPointF(x, y);
		}
		inline void set_colormap_curve(QPolygonF &curve, qreal x, qreal y) // 0.0<=(x, y)<=1.0
		{
			x = qMax(0.0, qMin(1.0,  x));
			y = qMax(0.0, qMin(1.0,  y));
			curve << QPointF(x, y);
		}

		virtual void show();
		virtual bool eventFilter(QObject *object, QEvent *event);

		friend class NeuronGameUI;
		
		QScrollBar *contrastSlider;
		


		/**********************************************************************************
        * Generates a new viewer using the given coordinates.
        * Called by the current <CExplorerWindow> when the user zooms in and the higher res-
        * lution has to be loaded.
        ***********************************************************************************/
        virtual void
        newViewer(
            int x, int y, int z,                //can be either the VOI's center (default)
                                                //or the VOI's ending point (see x0,y0,z0)
            int resolution,                     //resolution index of the view requested
            int t0, int t1,                     //time frames selection
            bool fromVaa3Dcoordinates = false,  //if coordinates were obtained from Vaa3D
            int dx=-1, int dy=-1, int dz=-1,    //VOI [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz)
            int x0=-1, int y0=-1, int z0=-1,    //VOI [x0, x), [y0, y), [z0, z)
            bool auto_crop = true,              //whether to crop the VOI to the max dims
            bool scale_coords = true,           //whether to scale VOI coords to the target res
            int sliding_viewer_block_ID = -1    //block ID in "Sliding viewer" mode
        );



	public slots:
		void updateContrast(int con);




		/*********************************************************************************
        * Receive data (and metadata) from <CVolume> throughout the loading process
        **********************************************************************************/
        virtual void receiveData(
                itm::uint8* data,                   // data (any dimension)
                itm::integer_array data_s,          // data start coordinates along X, Y, Z, C, t
                itm::integer_array data_c,          // data count along X, Y, Z, C, t
                QWidget* dest,                         // address of the listener
                bool finished,                      // whether the loading operation is terminated
                itm::RuntimeException* ex = 0,      // exception (optional)
                qint64 elapsed_time = 0,            // elapsed time (optional)
                QString op_dsc="",                  // operation descriptor (optional)
                int step=0);                        // step number (optional)





};

#endif