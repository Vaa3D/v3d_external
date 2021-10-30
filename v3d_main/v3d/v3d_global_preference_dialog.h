/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/****************************************************************************
**
v3d_global_preference_dialog.h
 by Hanchuan Peng
 2009-05-01
 2009-06-18
 2010-01-11
 2010-06-01
 2010-06-02
 2010-09-04: by PHC. add b_UseMylibTiff
 2018-03-01: by ZZ. add b_BlendColor

**
****************************************************************************/

#ifndef __V3D_GLOBAL_PREFERENCE_DIALOG_H__
#define __V3D_GLOBAL_PREFERENCE_DIALOG_H__

#include <QtGui>

#include "ui_v3d_global_preference.h"

#include "../basic_c_fun/v3d_global_preference.h"
//#include "../jba/c++/jba_mainfunc.h"  //100601


class V3DGlobalPreferenceDialog : public QDialog, public Ui_Dialog_v3d_preference
{
    Q_OBJECT

public:
    V3DGlobalPreferenceDialog(V3D_GlobalSetting *p)
	{
		setupUi(this);

		connect(pushButton_ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));

		if (!p) return;

		//tri-view
		checkBox_yaxis_up->setChecked(p->b_yaxis_up);
		checkBox_autoConvert2_8bit->setChecked(p->b_autoConvert2_8bit);
		spinBox_defaultRightShiftBits->setValue(p->default_rightshift_bits);
        checkBox_blendColor->setChecked(p->b_BlendColor);

		checkBox_autoRescale16bitDisplay->setChecked(p->b_autoRescale16bitDisplay);
		checkBox_autoRescale16bitDisplay->hide();

		spinBox_lookGlassSize->setValue(p->default_lookglass_size);
		spinBox_markerSize->setValue(p->default_marker_radius);

		//3d viewer
		checkBox_scrollupZoomin->setChecked(p->b_scrollupZoomin);
		checkBox_autoOpenImg3DViewer->setChecked(p->b_autoOpenImg3DViewer);
		checkBox_autoDispXYZAxes->setChecked(p->b_autoDispXYZAxes);
		checkBox_autoDispBoundingBox->setChecked(p->b_autoDispBoundingBox);
		spinBox_markerAmplifyFactor->setValue(p->marker_disp_amplifying_factor);
		checkBox_autoSWCLineMode->setChecked(p->b_autoSWCLineMode);
		
		checkBox_autoVideoCardCompress->setChecked(p->b_autoVideoCardCompress);
		checkBox_autoVideoCard3DTex->setChecked(p->b_autoVideoCard3DTex);
		checkBox_autoVideoCardNPTTex->setChecked(p->b_autoVideoCardNPTTex);
		spinBox_autoVideoCardStreamMode->setRange(-1,2); spinBox_autoVideoCardStreamMode->setValue(p->autoVideoCardStreamMode);
		
		checkBox_libTiff_Mylib->setChecked(!(p->b_UseMylibTiff));


		//image analysis
//		QStringList items;
//		items << tr("MATCH_MI") << tr("MATCH_MULTIPLE_MI_INT_CORR") << tr("MATCH_INTENSITY") << tr("MATCH_CORRCOEF") << tr("MATCH_IMOMENT") << tr("MATCH_MEANOFCIRCLES");
//		comboBox_reg_markermatch_method->insertItems(0, items);
		comboBox_reg_markermatch_method->setCurrentIndex(p->GPara_landmarkMatchingMethod);

//		items.clear();
//		items << tr("TPS-linear-interpolation") <<  tr("TPS-B-Spline-interpolation") << tr("TPS") << tr("Hier-B-Spline");
//	    comboBox_warpingfield_method->insertItems(0, items);
		comboBox_warpingfield_method->setCurrentIndex(p->GPara_df_compute_method);
		
		checkBox_3dcurve_autodeform->setChecked(p->b_3dcurve_autodeform);
		checkBox_3dcurve_autowidth->setChecked(p->b_3dcurve_autowidth);
		checkBox_3dcurve_autoconnecttips->setChecked(p->b_3dcurve_autoconnecttips);
		checkBox_3dcurve_inertia->setChecked(p->b_3dcurve_inertia);
		
		checkBox_3dcurve_width_from_xyonly->setChecked(p->b_3dcurve_width_from_xyonly);
		
		//plugin
		spinBox_iChannel_for_plugin->setRange(-1,2);
		spinBox_iChannel_for_plugin->setValue(p->iChannel_for_plugin);
		checkBox_b_plugin_dispResInNewWindow->setChecked(p->b_plugin_dispResInNewWindow);
		checkBox_b_plugin_dispParameterDialog->setChecked(p->b_plugin_dispParameterDialog);
		checkBox_b_plugin_outputImgRescale->setChecked(p->b_plugin_outputImgRescale);
		checkBox_b_plugin_outputImgConvert2UINT8->setChecked(p->b_plugin_outputImgConvert2UINT8);
	}

	void fetchData(V3D_GlobalSetting *p)
	{
		if (!p) return;
		p->b_yaxis_up = checkBox_yaxis_up->isChecked();
		p->b_autoConvert2_8bit = checkBox_autoConvert2_8bit->isChecked();
		p->default_rightshift_bits = spinBox_defaultRightShiftBits->value();
		p->b_autoRescale16bitDisplay = checkBox_autoRescale16bitDisplay->isChecked();
		p->default_lookglass_size = spinBox_lookGlassSize->value();
		p->default_marker_radius = spinBox_markerSize->value();
        p->b_BlendColor = checkBox_blendColor->isChecked();

		//3d viewer
		p->b_scrollupZoomin = checkBox_scrollupZoomin->isChecked();
		p->b_autoOpenImg3DViewer = checkBox_autoOpenImg3DViewer->isChecked();
		p->b_autoDispXYZAxes = checkBox_autoDispXYZAxes->isChecked();
		p->b_autoDispBoundingBox = checkBox_autoDispBoundingBox->isChecked();
		p->marker_disp_amplifying_factor = spinBox_markerAmplifyFactor->value();
		p->b_autoSWCLineMode = checkBox_autoSWCLineMode->isChecked();

		p->b_autoVideoCardCompress = checkBox_autoVideoCardCompress->isChecked();
		p->b_autoVideoCard3DTex = checkBox_autoVideoCard3DTex->isChecked();
		p->b_autoVideoCardNPTTex = checkBox_autoVideoCardNPTTex->isChecked();
		p->autoVideoCardStreamMode = spinBox_autoVideoCardStreamMode->value();
		p->b_UseMylibTiff = !(checkBox_libTiff_Mylib->isChecked());

		//image analysis
		p->GPara_landmarkMatchingMethod = comboBox_reg_markermatch_method->currentIndex(); //100601, by PHC: (PointMatchMethodType)comboBox_reg_markermatch_method->currentIndex();
		p->GPara_df_compute_method = comboBox_warpingfield_method->currentIndex(); //100601, by PHC: (DFComputeMethodType)comboBox_warpingfield_method->currentIndex();
		p->b_3dcurve_autodeform = checkBox_3dcurve_autodeform->isChecked();
		p->b_3dcurve_autowidth = checkBox_3dcurve_autowidth->isChecked();
		p->b_3dcurve_autoconnecttips = checkBox_3dcurve_autoconnecttips->isChecked();
		p->b_3dcurve_inertia = checkBox_3dcurve_inertia->isChecked();
		p->b_3dcurve_width_from_xyonly = checkBox_3dcurve_width_from_xyonly->isChecked();
		
		//plugin
		p->iChannel_for_plugin = spinBox_iChannel_for_plugin->value();
		p->b_plugin_dispResInNewWindow = checkBox_b_plugin_dispResInNewWindow->isChecked();
		p->b_plugin_dispParameterDialog = checkBox_b_plugin_dispParameterDialog->isChecked();
		p->b_plugin_outputImgRescale = checkBox_b_plugin_outputImgRescale->isChecked();
		p->b_plugin_outputImgConvert2UINT8 = checkBox_b_plugin_outputImgConvert2UINT8->isChecked();
	}


	static void readSettings(V3D_GlobalSetting &global_setting, QSettings &settings)
	{
		V3D_GlobalSetting def; //for read default value;
		//triview
		global_setting.b_yaxis_up = settings.value("b_yaxis_up", def.b_yaxis_up).toBool();
		global_setting.b_autoConvert2_8bit = settings.value("b_autoConvert2_8bit", def.b_autoConvert2_8bit).toBool();
		global_setting.b_autoRescale16bitDisplay = settings.value("b_autoRescale16bitDisplay", def.b_autoRescale16bitDisplay).toBool();
		global_setting.default_rightshift_bits = settings.value("default_rightshift_bits", def.default_rightshift_bits).toInt();
		global_setting.default_lookglass_size = settings.value("default_lookglass_size", def.default_lookglass_size).toInt();
		global_setting.default_marker_radius = settings.value("default_marker_radius", def.default_marker_radius).toInt();
        global_setting.b_BlendColor = settings.value("b_BlendColor", def.b_BlendColor).toBool();

		//3D viewer tab
		global_setting.b_scrollupZoomin = settings.value("b_scrollupZoomin", def.b_scrollupZoomin).toBool();
		global_setting.b_autoOpenImg3DViewer = settings.value("b_autoOpenImg3DViewer", def.b_autoOpenImg3DViewer).toBool();
		global_setting.b_autoDispXYZAxes = settings.value("b_autoDispXYZAxes", def.b_autoDispXYZAxes).toBool();
		global_setting.b_autoDispBoundingBox = settings.value("b_autoDispBoundingBox", def.b_autoDispBoundingBox).toBool();
		global_setting.marker_disp_amplifying_factor = settings.value("marker_disp_amplifying_factor", def.marker_disp_amplifying_factor).toInt();
		global_setting.b_autoSWCLineMode = settings.value("b_autoSWCLineMode", def.b_autoSWCLineMode).toBool();
		
		global_setting.b_autoVideoCardCompress = settings.value("b_autoVideoCardCompress", def.b_autoVideoCardCompress).toBool();
		global_setting.b_autoVideoCard3DTex = settings.value("b_autoVideoCard3DTex", def.b_autoVideoCard3DTex).toBool();
		global_setting.b_autoVideoCardNPTTex = settings.value("b_autoVideoCardNPTTex", def.b_autoVideoCardNPTTex).toBool();
		global_setting.autoVideoCardStreamMode = settings.value("autoVideoCardStreamMode",def.autoVideoCardStreamMode).toInt();
        global_setting.b_UseMylibTiff = settings.value("b_UseMylibTiff", def.b_UseMylibTiff).toBool();

		//image analysis tab
		global_setting.GPara_landmarkMatchingMethod = settings.value("GPara_landmarkMatchingMethod", def.GPara_landmarkMatchingMethod).toInt(); //by PHC, 100601: (PointMatchMethodType)
		global_setting.GPara_df_compute_method = settings.value("GPara_df_compute_method", def.GPara_df_compute_method).toInt(); //by PHC, 100601: (DFComputeMethodType)
		global_setting.b_3dcurve_autodeform =  settings.value("b_3dcurve_autodeform", def.b_3dcurve_autodeform).toBool();
		global_setting.b_3dcurve_autowidth =  settings.value("b_3dcurve_autowidth", def.b_3dcurve_autowidth).toBool();
		global_setting.b_3dcurve_autoconnecttips = settings.value("b_3dcurve_autoconnecttips", def.b_3dcurve_autoconnecttips).toBool();
		global_setting.b_3dcurve_inertia = settings.value("b_3dcurve_inertia", def.b_3dcurve_inertia).toBool();
		global_setting.b_3dcurve_width_from_xyonly = settings.value("b_3dcurve_width_from_xyonly", def.b_3dcurve_width_from_xyonly).toBool();
		
		//plugins
		global_setting.iChannel_for_plugin = settings.value("iChannel_for_plugin", def.iChannel_for_plugin).toInt();
		global_setting.b_plugin_dispResInNewWindow = settings.value("b_plugin_dispResInNewWindow", def.b_plugin_dispResInNewWindow).toBool();
		global_setting.b_plugin_dispParameterDialog = settings.value("b_plugin_dispParameterDialog", def.b_plugin_dispParameterDialog).toBool();
		global_setting.b_plugin_outputImgRescale = settings.value("b_plugin_outputImgRescale", def.b_plugin_outputImgRescale).toBool();
		global_setting.b_plugin_outputImgConvert2UINT8 = settings.value("b_plugin_outputImgConvert2UINT8", def.b_plugin_outputImgConvert2UINT8).toBool();
	}

	static void writeSettings(V3D_GlobalSetting &global_setting, QSettings &settings)
	{
		//triview
		settings.setValue("b_yaxis_up", global_setting.b_yaxis_up);
		settings.setValue("b_autoConvert2_8bit", global_setting.b_autoConvert2_8bit);
		settings.setValue("b_autoRescale16bitDisplay", global_setting.b_autoRescale16bitDisplay);
		settings.setValue("default_rightshift_bits", global_setting.default_rightshift_bits);
		settings.setValue("default_lookglass_size", global_setting.default_lookglass_size);
		settings.setValue("default_marker_radius", global_setting.default_marker_radius);
        settings.setValue("b_BlendColor", global_setting.b_BlendColor);

		//3D viewer tab
		settings.setValue("b_scrollupZoomin", global_setting.b_scrollupZoomin);
		settings.setValue("b_autoOpenImg3DViewer", global_setting.b_autoOpenImg3DViewer);
		settings.setValue("b_autoDispBoundingBox", global_setting.b_autoDispBoundingBox);
		settings.setValue("b_autoDispXYZAxes", global_setting.b_autoDispXYZAxes);
		settings.setValue("marker_disp_amplifying_factor", global_setting.marker_disp_amplifying_factor);
		settings.setValue("b_autoSWCLineMode", global_setting.b_autoSWCLineMode);

		settings.setValue("b_autoVideoCardCompress", global_setting.b_autoVideoCardCompress);
		settings.setValue("b_autoVideoCard3DTex", global_setting.b_autoVideoCard3DTex);
		settings.setValue("b_autoVideoCardNPTTex", global_setting.b_autoVideoCardNPTTex);
		settings.setValue("autoVideoCardStreamMode", global_setting.autoVideoCardStreamMode);
		settings.setValue("b_UseMylibTiff", global_setting.b_UseMylibTiff);

		//image analysis tab
		settings.setValue("GPara_landmarkMatchingMethod", global_setting.GPara_landmarkMatchingMethod);
		settings.setValue("GPara_df_compute_method", global_setting.GPara_df_compute_method);
		settings.setValue("b_3dcurve_autodeform", global_setting.b_3dcurve_autodeform);
		settings.setValue("b_3dcurve_autowidth", global_setting.b_3dcurve_autowidth);
		settings.setValue("b_3dcurve_autoconnecttips", global_setting.b_3dcurve_autoconnecttips);
		settings.setValue("b_3dcurve_inertia", global_setting.b_3dcurve_inertia);
		settings.setValue("b_3dcurve_width_from_xyonly", global_setting.b_3dcurve_width_from_xyonly);
		
		//plugins
		settings.setValue("iChannel_for_plugin", global_setting.iChannel_for_plugin);
		settings.setValue("b_plugin_dispResInNewWindow", global_setting.b_plugin_dispResInNewWindow);
		settings.setValue("b_plugin_dispParameterDialog", global_setting.b_plugin_dispParameterDialog);
		settings.setValue("b_plugin_outputImgRescale", global_setting.b_plugin_outputImgRescale);
		settings.setValue("b_plugin_outputImgConvert2UINT8", global_setting.b_plugin_outputImgConvert2UINT8);
	}
};

#endif
