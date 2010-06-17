//v3d_global_preference.h
//by Hanchuan Peng
// 2010-06-01
// separated from v3d_global_preference_dialog.h, by PHC, 100601.

#ifndef __V3D_GLOBAL_PREFERENCE_H__
#define __V3D_GLOBAL_PREFERENCE_H__

struct V3D_GlobalSetting
{
	//triview tab
	bool b_yaxis_up;
	bool b_autoConvert2_8bit;
	int default_rightshift_bits;
	bool b_autoRescale16bitDisplay;
	int default_lookglass_size;
	int default_marker_radius;

	//3D viewer tab
	bool b_autoOpenImg3DViewer;
	bool b_autoDispXYZAxes;
	bool b_autoDispBoundingBox;
	int marker_disp_amplifying_factor;
	bool b_autoSWCLineMode;
	bool b_autoVideoCardCompress;
	bool b_autoVideoCard3DTex;
	bool b_autoVideoCardNPTTex;
	int autoVideoCardStreamMode;

	//image analysis tab
	int GPara_landmarkMatchingMethod; //PointMatchMethodType
	int GPara_df_compute_method;      //DFComputeMethodType
	bool b_3dcurve_autodeform;
	bool b_3dcurve_autowidth;
	bool b_3dcurve_autoconnecttips;
	bool b_3dcurve_inertia;
	bool b_3dcurve_width_from_xyonly;
	
	//default option for image analysis plugins
	int iChannel_for_plugin; //-1 for all channels, and 0, 1,2, ... for valid image color channel for plugin-based image processing
	bool b_plugin_dispResInNewWindow; //if display the plugin-based image processing result in a new window (otherwise re-use in the existing window)
	bool b_plugin_dispParameterDialog; //if display a dialog to ask a user to supply a plugin's parameters (e.g. image processing parameters)
	bool b_plugin_outputImgRescale; //if rescale the output of plugin's processing result between [0, 255]
	bool b_plugin_outputImgConvert2UINT8; //if yes then convert a plugin's output to UINT8 type (rounding to it in most cases); if no, then keep as float (32bit)

	//default preferences
	V3D_GlobalSetting()
	{
		b_yaxis_up = false; //so that y-axis positive direction is from top to bottom
		b_autoConvert2_8bit = false;
		default_rightshift_bits=-1; //-1 means do not use the auto-shift, instead should pop-up a dialog let user select how many bits to shift
		b_autoRescale16bitDisplay = false;
		default_lookglass_size = 7;
		default_marker_radius = 5;

		//3D viewer tab
		b_autoOpenImg3DViewer = false;
		b_autoDispXYZAxes = true;
		b_autoDispBoundingBox = true;
		marker_disp_amplifying_factor = 15;
		b_autoSWCLineMode = false;
		b_autoVideoCardCompress = true;
		b_autoVideoCard3DTex = false;
		b_autoVideoCardNPTTex = false;
		autoVideoCardStreamMode = 0;//1 for adaptive stream mode, 0 for 512x512x256 downsample. for others see dialog info

		//image analysis tab
		GPara_landmarkMatchingMethod = 0; //MATCH_MI; //(PointMatchMethodType)0;
		GPara_df_compute_method = 0; // DF_GEN_TPS_LINEAR_INTERP; //(DFComputeMethodType)0;
		
		b_3dcurve_autodeform=false;
		b_3dcurve_autowidth=false;
		b_3dcurve_autoconnecttips=true;
		b_3dcurve_inertia=true;
		b_3dcurve_width_from_xyonly = false;
		
		//plugin options
		iChannel_for_plugin = -1; //-1 for all channels, and 0, 1,2, ... for valid image color channel for plugin-based image processing
		b_plugin_dispResInNewWindow = true; //if display the plugin-based image processing result in a new window (otherwise re-use in the existing window)
		b_plugin_dispParameterDialog = true; //if display a dialog to ask a user to supply a plugin's parameters (e.g. image processing parameters)
		b_plugin_outputImgRescale = false; //if rescale the output of plugin's processing result between [0, 255]
		b_plugin_outputImgConvert2UINT8 = true; // if yes then convert a plugin's output to UINT8 type (rounding to it in most cases); if no, then keep as float (32bit)
	}
};


#endif

