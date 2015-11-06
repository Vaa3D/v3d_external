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

	public slots:
		void updateContrast(int con);
};

#endif