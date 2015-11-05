#include "NeuronGame3DView.h"
#include <math.h>
#include "renderer_gl2.h"

#include "../../terafly/src/control/CVolume.h"
#include "../../terafly/src/presentation/PMain.h"

using namespace neurongame;

int NeuronGame3DView::contrastValue = 0;

NeuronGame3DView::NeuronGame3DView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
		: teramanager::CViewer(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
			_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID)
{
	contrastSlider = new QScrollBar(2); // Qt::Orientation::Vertical
	contrastSlider->setRange(-50, 50);
	contrastSlider->setSingleStep(1);
	contrastSlider->setPageStep(10);
	contrastSlider->setValue(contrastValue);
	
	QObject::connect(contrastSlider, SIGNAL(valueChanged(int)), dynamic_cast<QObject *>(this), SLOT(updateContrast(int)));
}

teramanager::CViewer* NeuronGame3DView::makeView(V3DPluginCallback2 *_V3D_env, int _resIndex, itm::uint8 *_imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer *_prev, int _slidingViewerBlockID)
{
	//Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	//curr_renderer->finishEditingNeuronTree();
	NeuronGame3DView* neuronView = new NeuronGame3DView(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
		_volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, _slidingViewerBlockID);
	if (teramanager::PMain::getInstance()->annotationsPathLRU == "")
		teramanager::PMain::getInstance()->annotationsPathLRU = "./temp.ano";
	teramanager::PMain::getInstance()->saveAnnotations();
	return neuronView;
}

void NeuronGame3DView::show()
{
	this->title = "Neuron Game 3D View";
	teramanager::CViewer::show();
	window3D->centralLayout->addWidget(contrastSlider, 1);
}


void NeuronGame3DView::updateContrast(int con) /* contrast from -100 (bright) to 100 (dark) */
{
	// This performs the same functionality as colormap Red->Gray and then
	// adjusting the alpha, but with only one parameter to adjust
	contrastValue = con;
	float exp_val = pow(10.0f, con/50.0f); // map from -100->100 to 100->0.01
	Renderer_gl2* curr_renderer = (Renderer_gl2*)(view3DWidget->getRenderer());
	for(int j=0; j<255; j++)
	{
		(curr_renderer->colormap[0][j]).r = (unsigned char)255;
		(curr_renderer->colormap[0][j]).g = (unsigned char)255;
		(curr_renderer->colormap[0][j]).b = (unsigned char)255;
		// This is the value being manipulated
		int val = (int)(pow(j/255.0f, exp_val) * 255.0f);
		(curr_renderer->colormap[0][j]).a = (unsigned char)val;
		
		(curr_renderer->colormap[1][j]).r = (unsigned char)0;
		(curr_renderer->colormap[1][j]).g = (unsigned char)255;
		(curr_renderer->colormap[1][j]).b = (unsigned char)0;
		(curr_renderer->colormap[1][j]).a = (unsigned char)0;
		
		(curr_renderer->colormap[2][j]).r = (unsigned char)0;
		(curr_renderer->colormap[2][j]).g = (unsigned char)0;
		(curr_renderer->colormap[2][j]).b = (unsigned char)255;
		(curr_renderer->colormap[2][j]).a = (unsigned char)0;
		
		(curr_renderer->colormap[3][j]).r = (unsigned char)205;
		(curr_renderer->colormap[3][j]).g = (unsigned char)205;
		(curr_renderer->colormap[3][j]).b = (unsigned char)205;
		(curr_renderer->colormap[3][j]).a = (unsigned char)205;
	}
	for (int ch=0; ch<3; ch++)
	{
		for (int j=0; j<4; j++) // RGBA
		{
			curr_renderer->colormap_curve[ch][j].clear();
			int y;
			y = curr_renderer->colormap[ch][0].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  0.0,  y);
			y = curr_renderer->colormap[ch][255].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  1.0,  y);
		}
	}
	view3DWidget->update();
}