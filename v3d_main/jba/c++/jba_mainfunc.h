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




//jba_mainfunc.h
//previously called laff_mainfunc.h
//The major function declearations
//by Hanchuan Peng
//2007-Feb-14: create the file
//2007-Feb-19: add dfx, dfy, dfz, dft variables to Warp3D class
//2007-Feb-21: add Vol3DSimple class
//2007-Feb-22: remove some earlier commented codes
//2007-Feb-27: add nloop para
//2007-Feb-28: change the ref and subject data pointers to Vol3DSimple
//2007-March-01: add class Image2DSimple
//2007-March-16: add padding and unpadding function to Vol3DSimple class
//2007-March-29: add tps
//2007-March-30: add Vector1DSimple class
//2007-May-4: add a flag to indicate if a warp parameter has been changed (if changed from the initialization, call it "valid")
//2007-May-6: add a global shift transform for debugging
//2007-May-7: add a local shift transform
//2007-May-9: add simple regional matching + tps
//2007-May-10: add a mean_and_std function
//2007-May-14: add a new class of kernelSet, which will be passed to other function as an argument
//2007-May-18: add the regional tps matching, also move several functions of the common module out of the Warp3D class
//2007-May-19: move the affine est function out of warp3d class
//2007-May-20: move affine warp out of warp3d class
//2007-May-21: add hierachical in topdown 
//2007-May-22: add cubic interpolation, and rename the original one as interpolate_xxx_linear
//2007-May-23-27: add several functions and also change the output type of the main warping functions
//2007-May-30: add b-spline functions, etc.
//2007-10-18: separate channelNo_target and channelNo_subject
//2007-10-27: begin to re-work on b-spline DF
//2007-11-24: change do_tps as do_landmark_warping(); also update detectBestMatchingCpt() definiton.
//2007-12-13: add the landmark warping based on matching points stored in a file
//2008-01-07: update the BasicWarpParameter structure and also the related codes of the landmark based registration
//2008-01-23: add est_scal_3d
//2008-01-25: add a subject image tag in bwp
//20080205: move PointMatchMethodType to here from inside cmpFeaturesonTwoImages(), also update the interface to call the point matching functions
//20080211: add compute_dist_pt_plane() function
//2008-03-26: move the function detectBestMatchingCpt() out of the class Wrap3D as a standalone function, so easier to call
//2008-04-01: change UINT8 to UINT8_JBA and MYFLOAT to MYFLOAT_JBA, to avoid duplicate definition, - a stupid and simple way to fix
//2008-04-01: separate the img_definition.h
//2008-04-01: add a function detectBestMatchingCpt() without the csv file argument for convenience
//2008-04-14: separate some basic volume processing function to the volimg_proc.h in the basic_fun dir
//2008-05-04: add interp_nearest func
//2008-08-17: renamed as jba_mainfunc.h and also get simplified
//2008-08-18: add two more flags b_useMultiscaleMatching and b_useMultiscaleDFInterpolation to the BasicWarpParameter structure
//2008-08-18: add do_jba_core_computation() and several other functions
//2008-08-26: add the matching range parameter in the detectBestMatchingCpt() function
//2008-10-05: add options to change method_match_landmarks and method_df_compute
//2008-10-11: add compute_df_tps_subsampled_volume()
//2008-10-12: add do_landmark_in_file_warping_hier()
//2009-03-20: move POSITIVE_Y_COORDINATE definition here
//2009-05-29: add two function of warpings for coding of the memory saving warping
//2010-05-20: add a few missing function for VC 2008 

#ifndef __JBA_MAINFUNC__
#define __JBA_MAINFUNC__

#include "../../basic_c_fun/basic_memory.cpp"
#include "../../basic_c_fun/img_definition.h"

#include <string>
#include <vector>
using namespace std;
string gen_file_name(string basename, int layer, string otherinfo);

typedef unsigned char UINT8_JBA;
typedef float MYFLOAT_JBA;

#if defined (_MSC_VER) //&& (_WIN64)
#include "../../basic_c_fun/vcdiff.h"
#else
#endif



#define POSITIVE_Y_COORDINATE


#include "wkernel.h" 

class Coord3D_JBA
{
public:
    MYFLOAT_JBA x,y,z; //should be fine because the real coordinates should between 0~2047
	Coord3D_JBA(float x0,float y0,float z0) {x=x0;y=y0;z=z0;}
	Coord3D_JBA() {x=y=z=0;}
};

enum PointMatchMethodType {MATCH_MI, MATCH_MULTIPLE_MI_INT_CORR, MATCH_INTENSITY, MATCH_MEANCIRCLEINTENSITY, MATCH_CORRCOEF, MATCH_IMOMENT, MATCH_MEANOFCIRCLES}; 
enum DFComputeMethodType {DF_GEN_TPS, DF_GEN_HIER_B_SPLINE, DF_GEN_TPS_B_SPLINE, DF_GEN_TPS_LINEAR_INTERP}; 

class PointMatchScore
{
public:
	double method_inconsistency, model_violation;
};

class BasicWarpParameter
{
public:
	int nloop_localest; 
	int nloop_smooth; 
	V3DLONG imgxy_minsz; 
	V3DLONG localest_halfwin; 
	int b_verbose_print;
	//bool b_useMultiscaleMatching;
	//bool b_useMultiscaleDFInterpolation;
	string file_landmark_target;
	string file_landmark_subject;
	string tag_output_image_file;

	PointMatchMethodType method_match_landmarks;
	DFComputeMethodType method_df_compute;
	
    Coord3D_JBA targetMultiplyFactor;
    Coord3D_JBA subjectMultiplyFactor;

	//081012
	bool b_search_around_preset_subject_pos; 
	int radius_search_around_preset_subject_pos; 
	int cur_level; //the current level of matching, this is useful for hierarchical matching
	
	//080819
	double hierachical_match_level; //if 1, then do not have any downsampling in landmark matching. Otherwise matching on the downsampled images

	BasicWarpParameter() 
	{
		nloop_localest=10;nloop_smooth=10;imgxy_minsz=64;localest_halfwin=10;b_verbose_print=0;
		//b_useMultiscaleMatching = true;
		//b_useMultiscaleDFInterpolation = true;
		file_landmark_target="preDefinedLandmark.cfg";file_landmark_subject="unsetyet";
		tag_output_image_file="";
		hierachical_match_level = 1; //default do not do any downsampling
		
		method_match_landmarks = MATCH_MULTIPLE_MI_INT_CORR;
		method_df_compute = DF_GEN_TPS_LINEAR_INTERP;
		
		//081012
		b_search_around_preset_subject_pos = false;
		radius_search_around_preset_subject_pos = 20;
		cur_level = 0;
	}
	BasicWarpParameter(const BasicWarpParameter & bwp)
	{
		nloop_localest = bwp.nloop_localest;
		nloop_smooth = bwp.nloop_smooth;
		imgxy_minsz = bwp.imgxy_minsz;
		localest_halfwin = bwp.localest_halfwin;
		b_verbose_print = bwp.b_verbose_print;
		//b_useMultiscaleMatching = bwp.b_useMultiscaleMatching;
		//b_useMultiscaleDFInterpolation = bwp.b_useMultiscaleDFInterpolation;
		file_landmark_target = bwp.file_landmark_target;
		file_landmark_subject = bwp.file_landmark_subject;
		tag_output_image_file = bwp.tag_output_image_file;
		
		hierachical_match_level = bwp.hierachical_match_level;
		
		method_match_landmarks = bwp.method_match_landmarks;
		method_df_compute = bwp.method_df_compute;

		//081012
		b_search_around_preset_subject_pos = bwp.b_search_around_preset_subject_pos;
		radius_search_around_preset_subject_pos = bwp.radius_search_around_preset_subject_pos;
		cur_level = bwp.cur_level;
	}
};

bool do_jba_core_computation(unsigned char * img_target, 
							 V3DLONG *sz_target,
							 int datatype_target,
							 string target_file_name,
							 int channelNo_target, 
							 unsigned char * img_subject, 
							 V3DLONG *sz_subject,
							 int datatype_subject,
							 string subject_file_name,
							 int channelNo_subject, 
							 unsigned char * &img_warped, 
							 V3DLONG *&sz_warped,
							 int datatype_warped,
							 string warped_file_name,
							 int warpType,
							 const BasicWarpParameter & bwp);

bool do_jba_computation(unsigned char * img_target, 
						 V3DLONG *sz_target,
						 int datatype_target,
						 string target_file_name,
						 int channelNo_target, 
						 unsigned char * img_subject, 
						 V3DLONG *sz_subject,
						 int datatype_subject,
						 string subject_file_name,
						 int channelNo_subject, 
						 unsigned char * &img_warped, 
						 V3DLONG *&sz_warped,
						 int datatype_warped,
						 string warped_file_name,
						 int warpType,
						 const BasicWarpParameter & bwp);


class DisplaceFieldF3D
{
public:
    UINT8_JBA b_transform;
	
	MYFLOAT_JBA sx, sy, sz; //shift of x,y,z
	DisplaceFieldF3D() {sx=sy=sz=0; b_transform=0;}
	void scale(double dfactor) {sx*=dfactor;sy*=dfactor;sz*=dfactor;} 
	void resetToDefault() //070517
	{
	  sx  = 0; sy  = 0; sz  = 0;
	  b_transform=0; 
	}
	bool copy(DisplaceFieldF3D *wp) 
	{
	  if (!wp) return false;
	  sx  = wp->sx;  sy  = wp->sy;  sz  = wp->sz;
	  b_transform = wp->b_transform;
	  return true;
	} 
	bool copy(DisplaceFieldF3D &wp) 
	{
	  sx  = wp.sx;  sy  = wp.sy;  sz  = wp.sz;
	  b_transform = wp.b_transform; 
	  return true;
	} 
	
};

class WarpParameterAffine3D : public DisplaceFieldF3D
{
public:
    MYFLOAT_JBA mxx, mxy, mxz;
	MYFLOAT_JBA myx, myy, myz;
	MYFLOAT_JBA mzx, mzy, mzz; //rotation of x,y,z	
	//MYFLOAT_JBA sx, sy, sz; //shift of x,y,z
	MYFLOAT_JBA si, sb; //scale and offset of intensity
	WarpParameterAffine3D () 
	{
	  mxx=myy=mzz=1.0; mxy=mxz=myx=myz=mzx=mzy=0; sx=sy=sz=0; si=1; sb=0; 
	  b_transform=0; //070504
	} 
	bool copy(WarpParameterAffine3D *wp) 
	{
	  if (!wp) return false;
	  mxx = wp->mxx; mxy = wp->mxy; mxz = wp->mxz;
	  myx = wp->myx; myy = wp->myy; myz = wp->myz;
	  mzx = wp->mzx; mzy = wp->mzy; mzz = wp->mzz;
	  sx  = wp->sx;  sy  = wp->sy;  sz  = wp->sz;
	  si  = wp->si;  sb  = wp->sb;
	  b_transform = wp->b_transform; //070504
	  return true;
	} 
	bool copy(WarpParameterAffine3D &wp) 
	{
	  mxx = wp.mxx; mxy = wp.mxy; mxz = wp.mxz;
	  myx = wp.myx; myy = wp.myy; myz = wp.myz;
	  mzx = wp.mzx; mzy = wp.mzy; mzz = wp.mzz;
	  sx  = wp.sx;  sy  = wp.sy;  sz  = wp.sz;
	  si  = wp.si;  sb  = wp.sb;
	  b_transform = wp.b_transform; //070504
	  return true;
	} 
	bool add(WarpParameterAffine3D *wp) 
	{
	  if (!wp) return false;
	  mxx += wp->mxx; mxy += wp->mxy; mxz += wp->mxz;
	  myx += wp->myx; myy += wp->myy; myz += wp->myz;
	  mzx += wp->mzx; mzy += wp->mzy; mzz += wp->mzz;
	  sx  += wp->sx;  sy  += wp->sy;  sz  += wp->sz;
	  si  += wp->si;  sb  += wp->sb;
	  if(b_transform==0 && wp->b_transform==1) b_transform=1; //070504
	  return true;
	} 
	bool add(WarpParameterAffine3D &wp)
	{
	  mxx += wp.mxx; mxy += wp.mxy; mxz += wp.mxz;
	  myx += wp.myx; myy += wp.myy; myz += wp.myz;
	  mzx += wp.mzx; mzy += wp.mzy; mzz += wp.mzz;
	  sx  += wp.sx;  sy  += wp.sy;  sz  += wp.sz;
	  si  += wp.si;  sb  += wp.sb;
	  if(b_transform==0 && wp.b_transform==1) b_transform=1; //070504
	  return true;
	}
	void time(double d)
	{
	  mxx *= d; mxy *= d; mxz *= d;
	  myx *= d; myy *= d; myz *= d;
	  mzx *= d; mzy *= d; mzz *= d;
	  sx  *= d;  sy *= d;  sz *= d;
	  si  *= d;  sb *= d;
	  //do not do anything about b_transform, 070504
	}  
	void resetToAllZeros()
	{
	  mxx = 0; mxy = 0; mxz = 0;
	  myx = 0; myy = 0; myz = 0;
	  mzx = 0; mzy = 0; mzz = 0;
	  sx  = 0; sy  = 0; sz  = 0;
	  si  = 0; sb  = 0;
	  //do not do anything about b_transform, 070504
	}
	void resetToDefault()
	{
	  mxx = 1; mxy = 0; mxz = 0;
	  myx = 0; myy = 1; myz = 0;
	  mzx = 0; mzy = 0; mzz = 1;
	  sx  = 0; sy  = 0; sz  = 0;
	  si  = 1; sb  = 0;
	  b_transform=0; //070504
	}
	void print(string s);
};

void aggregateAffineWarp(WarpParameterAffine3D * res, WarpParameterAffine3D * a, WarpParameterAffine3D * b);

bool getSmoothedWarpParameter(WarpParameterAffine3D * wp_current, Vol3DSimple<WarpParameterAffine3D> * warpPara, int halfwin, V3DLONG x, V3DLONG y, V3DLONG z);
bool getSmoothedWarpParameter(Vol3DSimple<WarpParameterAffine3D> * wp_new, Vol3DSimple<WarpParameterAffine3D> * wp_old, int halfwin);

bool saveVol3DSimple2RawFloat(Vol3DSimple<unsigned char> *img, string basefilename, int layer, string otherinfo);
bool saveVol3DSimple2RawFloat(Vol3DSimple<MYFLOAT_JBA> *img, string basefilename, int layer, string otherinfo);
bool saveVol3DSimple2RawFloat(Vol3DSimple<DisplaceFieldF3D> *p, string basefilename, int layer, string otherinfo);
bool saveVol3DSimple2RawFloat(Vol3DSimple<WarpParameterAffine3D> *p, string basefilename, int layer, string otherinfo);

bool saveVol3DSimple2RawUint8(Vol3DSimple<MYFLOAT_JBA> *img, string basefilename, int layer, string otherinfo, double multiplier); //add multiplier 070530

bool diffxyz(MYFLOAT_JBA ***dx, MYFLOAT_JBA ***dy, MYFLOAT_JBA ***dz, MYFLOAT_JBA *** dt, MYFLOAT_JBA *** cur_img3d_target, MYFLOAT_JBA *** cur_img3d_subject, V3DLONG d0, V3DLONG d1, V3DLONG d2);
bool newGradientData(Vol3DSimple<MYFLOAT_JBA> * & dfx, Vol3DSimple<MYFLOAT_JBA> * & dfy, Vol3DSimple<MYFLOAT_JBA> * & dfz, Vol3DSimple<MYFLOAT_JBA> * & dft, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2);
void deleteGradientData(Vol3DSimple<MYFLOAT_JBA> * & dfx, Vol3DSimple<MYFLOAT_JBA> * & dfy, Vol3DSimple<MYFLOAT_JBA> * & dfz, Vol3DSimple<MYFLOAT_JBA> * & dft);
	
bool assign_maskimg_val( Vol3DSimple <UINT8_JBA> * maskImg, Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg);

class Warp3D
{
    string file_target, file_subject, file_warped;
	 
	V3DLONG my_sz[3]; //should be an array of 3 elements
	V3DLONG my_len3d; //the total number of pixels in one channel
	
	Vol3DSimple<MYFLOAT_JBA> * img_target;
	V3DLONG spos_target [3]; //the starting x,y, and z coordinates in the new image, corresponding to the original (0,0,0) location
	
	Vol3DSimple<MYFLOAT_JBA> * img_subject;
	V3DLONG spos_subject [3]; //the starting x,y, and z coordinates in the new image, corresponding to the original (0,0,0) location
	
	Vol3DSimple<MYFLOAT_JBA> * img_warped;
	
	WarpParameterAffine3D warpPara_global;
	Vol3DSimple<WarpParameterAffine3D> * warpPara_local;
	V3DLONG len_warpPara_local;

    bool downsample3dvol(Vol3DSimple<MYFLOAT_JBA> * &outimg, Vol3DSimple<MYFLOAT_JBA> * inimg, double dfactor);
    bool upsample3dvol(Vol3DSimple<MYFLOAT_JBA> * & outimg, Vol3DSimple<MYFLOAT_JBA> * inimg, double dfactor);

	bool upsampleDisplaceField(Vol3DSimple<DisplaceFieldF3D> * & out_df, Vol3DSimple<DisplaceFieldF3D> * in_df, double dfactor);
	
	bool localsmoothing(
		Vol3DSimple<WarpParameterAffine3D> * warpPara_local,
		Vol3DSimple<MYFLOAT_JBA> * dfx,
		Vol3DSimple<MYFLOAT_JBA> * dfy,
		Vol3DSimple<MYFLOAT_JBA> * dfz,
		Vol3DSimple<MYFLOAT_JBA> * dft,
		Vol3DSimple<MYFLOAT_JBA> * img_target,
		Vol3DSimple<unsigned char> * img_subject,
		V3DLONG r_halfwin_smooth,
		int n_smooth_loop,
		int b_verbose_print);
	
	bool computemask(
		Vol3DSimple<unsigned char> * mask,
		Vol3DSimple<MYFLOAT_JBA> * dfx,
		Vol3DSimple<MYFLOAT_JBA> * dfy,
		Vol3DSimple<MYFLOAT_JBA> * dfz,
		Vol3DSimple<MYFLOAT_JBA> * dft,
		Vol3DSimple<MYFLOAT_JBA> * img_target,
		Vol3DSimple<MYFLOAT_JBA> * img_subject,
		int b_verbose_print);
		

	void deleteData();
	void initData();

	bool readBestMatchingCptFile(vector<Coord3D_JBA> & matchTargetPos,
									vector<Coord3D_JBA> & matchSubjectPos,
									Vol3DSimple<MYFLOAT_JBA> * img_target, 
								   Vol3DSimple<MYFLOAT_JBA> * img_subject,
   								   Coord3D_JBA targetOffset,
								   Coord3D_JBA subjectOffset,
								   Coord3D_JBA targetMultiplyFactor,
								   Coord3D_JBA subjectMultiplyFactor,
								   string priorTargetPosFile,
								   string priorSubjectPosFile,
								   bool b_inonefile);

	
public:
	template <class T> Warp3D(
		const T * img0_target1d, 
		const V3DLONG *img0_sz_target, 
		const int channelNo0_ref_target, 
		const T * img0_subject1d, 
		const V3DLONG *img0_sz_subject, 
		const int channelNo0_ref_subject, 
		const int datatype,
		string file0_target,
		string file0_subject,
		string file0_warped);

	~Warp3D();
	
	Vol3DSimple<DisplaceFieldF3D> * do_global_affine(const BasicWarpParameter & bwp); //main global affine program. change the output type on 070527
	
	bool do_local_affine(const BasicWarpParameter & bwp);
	
	Vol3DSimple<DisplaceFieldF3D> *  do_global_shift_transform(const BasicWarpParameter & bwp); //revised on 071021

	bool do_local_shift(const BasicWarpParameter & bwp); //added on 070507

	Vol3DSimple<DisplaceFieldF3D> * do_local_topdown_blocktps(const BasicWarpParameter & bwp); //070517
	
	Vol3DSimple<DisplaceFieldF3D> * do_local_topdown_blocktps_fast(const BasicWarpParameter & bwp); //070603
	
	Vol3DSimple<DisplaceFieldF3D> * do_landmark_warping(const BasicWarpParameter & bwp); 
	Vol3DSimple<DisplaceFieldF3D> * do_landmark_warping_hier(const BasicWarpParameter & bwp); 

	Vol3DSimple<DisplaceFieldF3D> * do_landmark_in_file_warping(const BasicWarpParameter & bwp); 
	
	template <class T> bool setInitData(
		const T * img0_target1d, 
		const V3DLONG *img0_sz_target, 
		const int channelNo0_ref_target, 
		const T * img0_subject1d, 
		const V3DLONG *img0_sz_subject, 
		const int channelNo0_ref_subject, 
		const int datatype,
		string file0_target,
		string file0_subject,
		string file0_warped);

	template <class T> bool setInitData(const T * img0_target1d, const V3DLONG *img0_sz_target, const int channelNo0_ref_target, const T * img0_subject1d, const V3DLONG *img0_sz_subject, const int channelNo0_ref_subject, const int datatype); //convert the UINT8_JBA input parameters to float point internal representations
	template <class T> bool getWarpedData(T * &outimg_warped, V3DLONG * &out_sz_warped, const int datatype);
	
	template <class T> bool applyDFtoChannel(
		const T * img0_subject1d, 
		const V3DLONG *img0_sz_subject, 
		const int channelNo0_ref, // if channelNo0_ref<0 then apply to all channels
		const int datatype,
		V3DLONG spos_subject [3],
		Vol3DSimple <DisplaceFieldF3D> *cur_df
	);
	
	template <class T> bool doWarpUsingDF(T * &outimg_warped, V3DLONG * &out_sz_warped, T * inimg_subject, V3DLONG * inimg_sz, const int datatype_warped, Vol3DSimple <DisplaceFieldF3D> * cur_df);
	
	bool warpSubjectImage(Vol3DSimple <DisplaceFieldF3D> *df);
	
	V3DLONG * get_spos_subject() {return spos_subject;} //080609
};


class KernelSet 
{
public:
  Vol3DSimple<WeightKernel3D *> * kernel;
  int _maxorder;
  KernelSet(int maxOrder, V3DLONG r, KernelType kt) 
  {
    if (maxOrder<=0) 
	{
	  _maxorder=0;
	  kernel=0;
	  return;
	}

	kernel = new Vol3DSimple<WeightKernel3D *> (maxOrder, maxOrder, maxOrder);
	if (kernel && kernel->valid())
	{
	  _maxorder=maxOrder;
	}
	else
	{
	  _maxorder=0;
	  if (kernel) {delete kernel; kernel=0;}
	}
	
	WeightKernel3D **** kernel_ref = kernel->getData3dHandle();
	for (V3DLONG k=0;k<_maxorder;k++)
	{
	  for (V3DLONG j=0;j<_maxorder;j++)
	  {
	    for (V3DLONG i=0;i<_maxorder;i++)
		{
		  kernel_ref[k][j][i] = new WeightKernel3D(r);
		  if (kernel_ref[k][j][i] && kernel_ref[k][j][i]->valid())
		  {
		    kernel_ref[k][j][i]->generateKernel(kt, i+1, j+1, k+1);
		  }
		}  
	  }
	}
  }
  ~KernelSet() 
  {
    if (kernel) 
	{
	    if (kernel->valid())
		{
			WeightKernel3D **** kernel_ref = kernel->getData3dHandle();
			for (V3DLONG k=0;k<_maxorder;k++)
			{
			  for (V3DLONG j=0;j<_maxorder;j++)
			  {
				for (V3DLONG i=0;i<_maxorder;i++)
				{
				  if (kernel_ref[k][j][i])
				  {
					delete kernel_ref[k][j][i];
					kernel_ref[k][j][i]=0;
				  }
				}  
			  }
			}
		}
	  
		delete kernel; kernel=0;
	}
	_maxorder=0;
  }
};

vector<Coord3D_JBA> readPosFile_3d(string posFile);
bool readPosFile_3dpair(string posFile, vector<Coord3D_JBA> & pos1, vector<Coord3D_JBA> & pos2);

bool df_warp(Vol3DSimple<MYFLOAT_JBA> * img, Vol3DSimple<DisplaceFieldF3D> * p); //move out of warp3D on 070526
bool df_warp(Vol3DSimple<MYFLOAT_JBA> * img, Vol3DSimple<WarpParameterAffine3D> * p);

bool df_add(Vol3DSimple<DisplaceFieldF3D> * addee, Vol3DSimple<DisplaceFieldF3D> * adder);
bool df_copy(Vol3DSimple<DisplaceFieldF3D> * copyee, Vol3DSimple<DisplaceFieldF3D> * copyer);


Vol3DSimple<DisplaceFieldF3D> *  get_DF_of_affine_warp(Vol3DSimple<MYFLOAT_JBA> * img, WarpParameterAffine3D * p);
Vol3DSimple<DisplaceFieldF3D> *  get_DF_of_affine_warp(V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, WarpParameterAffine3D * p);


bool est_best_affine_3d(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable, 
						Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, Vol3DSimple<MYFLOAT_JBA> * subject,
						V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze, 
						bool b_returnDTable);//the engine to estimate the best affine transform
bool est_best_affine_3d_interface(WarpParameterAffine3D & wp_current,  Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg, int nloops);
void affine_transform(Coord3D_JBA * c, V3DLONG numCoord, WarpParameterAffine3D * p, Coord3D_JBA * offset);
bool affine_warp(Vol3DSimple<MYFLOAT_JBA> * img, WarpParameterAffine3D * p);

bool est_best_affine_3d_noshift(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable, 
                                Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, 
								Vol3DSimple<MYFLOAT_JBA> * subject,
                                V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
								bool b_returnDTable); //the "s" (start) and "e" (end) coordinates in the 3D cube that is to be affine_transformed

	
bool est_best_scale_3d(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable, 
                       Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, 
	  				   Vol3DSimple<MYFLOAT_JBA> * subject,
                       V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
	  				   Vol3DSimple<UINT8_JBA> * mask,
					   bool b_returnDTable);//080123

bool est_best_shift_3d(DisplaceFieldF3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable, 
                       Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, 
	  				   Vol3DSimple<MYFLOAT_JBA> * subject,
                       V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
	  				   Vol3DSimple<UINT8_JBA> * mask,
					   bool b_returnDTable);
bool est_best_shift_3d_interface(DisplaceFieldF3D & wp_current,  Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg, int nloops);
void shift_transform(Coord3D_JBA * c, V3DLONG numCoord, DisplaceFieldF3D * p, Coord3D_JBA * offset);
bool shift_warp(Vol3DSimple<MYFLOAT_JBA> * img, DisplaceFieldF3D * p);

bool interpolate_coord_nearest(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord, 
                       MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, 
  		               V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);
bool interpolate_coord_linear(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord, 
                       MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, 
  		               V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);
//bool interpolate_coord_cubic(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord, 
//                       MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, 
//  		               V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);

Vol3DSimple <MYFLOAT_JBA> * linearinterp_regularmesh_3d(V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, Vol3DSimple <MYFLOAT_JBA> * df_regular_grid); //080902


//071123: add the interface function compute_df_using_matchingpts() and compute_df_tps(). Also change the function name compute_tps_df_field() to compute_df_tps()
Vol3DSimple<DisplaceFieldF3D> * compute_df_using_matchingpts(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, DFComputeMethodType method=DF_GEN_TPS_LINEAR_INTERP);
Vol3DSimple<DisplaceFieldF3D> * compute_df_tps(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);
Vol3DSimple<DisplaceFieldF3D> * compute_df_bspline(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);//071122
Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_interp(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, 
													  DFComputeMethodType interp_method); //inter_method==0, use linear interpolation; otherwise use B-spline
Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_interp_BACKUP(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, 
													   DFComputeMethodType interp_method); //inter_method==0, use linear interpolation; otherwise use B-spline //this is the original implementation
Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_subsampled_volume(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, 
																 V3DLONG gfactor_x, V3DLONG gfactor_y, V3DLONG gfactor_z); //081011: gfactor_x, gfactor_y, gfactor_z, are the subsample factors

Vol3DSimple <MYFLOAT_JBA> *  bspline_mesh_3d(Image2DSimple<MYFLOAT_JBA> * cptSet, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2); //071027
inline void compute_B_func(double B[4], double t);

//080401: provided for convenience
template <class T> bool applyDFtoChannel_simple(
	const T * img0_subject1d, 
	const V3DLONG *img0_sz_subject, 
	const int channelNo0_ref, // if channelNo0_ref<0 then apply to all channels
	const int datatype,
	V3DLONG startpos_subject [3],
	Vol3DSimple <DisplaceFieldF3D> *cur_df
); //see definition at the end;
												 
// basic volume data operations

template <class T> bool conv3d_1dvec(MYFLOAT_JBA ***outimg3d, T *** inimg3d, V3DLONG d0, V3DLONG d1, V3DLONG d2, MYFLOAT_JBA *vec, V3DLONG lenvec, int dim_of_conv);

template <class T> bool find3DGradient(Vol3DSimple <T> * & g, Vol3DSimple <T> *subject);

//definition of a convenient function
//080401: provided for convenience
template <class T> bool applyDFtoChannel_simple(
	const T * img0_subject1d, 
	const V3DLONG *img0_sz_subject, 
	const int channelNo0_ref, // if channelNo0_ref<0 then apply to all channels
	const int datatype,
	V3DLONG startpos_subject [3],
	Vol3DSimple <DisplaceFieldF3D> *cur_df
)
{
	if (!img0_subject1d || !img0_sz_subject || channelNo0_ref>img0_sz_subject[3] || !cur_df || !cur_df->valid()) 
	{
		fprintf(stderr, "Invalid parameters to applyDFtoChannel_simple(). \n");
		return false;
	}
	
	V3DLONG i,j,k,c;
	
	//set up internal data
	V3DLONG msz0=cur_df->sz0(), msz1=cur_df->sz1(), msz2=cur_df->sz2();
	if (img0_sz_subject[0]!=msz0 || img0_sz_subject[1]!=msz1 || img0_sz_subject[2]!=msz2)
	{
		fprintf(stderr, "The size of the subject image is wrong in applyDFtoChannel_simple(). Check you parameters. \n");
		return false;
	}
	
	Vol3DSimple <MYFLOAT_JBA> * cur_img_warped = new Vol3DSimple<MYFLOAT_JBA> (msz0, msz1, msz2);
	if (!cur_img_warped || !cur_img_warped->valid() )
	{
		fprintf(stderr, "Fail to allocate memory: file=[%s] line=[%d].\n", __FILE__, __LINE__);
		return false; //no need to free memory in this function, because the class will free all memory when it exits. 
	}
	MYFLOAT_JBA * img_warped_ref1d = cur_img_warped->getData1dHandle();
	MYFLOAT_JBA *** img_warped_ref3d = cur_img_warped->getData3dHandle();

    // do computation
		
	float Normalizer=1.0;
	if (datatype==1) Normalizer=255.0;
	else if (datatype==2) Normalizer=1023.0; //12-bit!
	else Normalizer=1.0; //take the data as is
	
	T ****img0_subject4d = 0;
	new4dpointer(img0_subject4d, (V3DLONG)img0_sz_subject[0], (V3DLONG)img0_sz_subject[1], (V3DLONG)img0_sz_subject[2], (V3DLONG)img0_sz_subject[3], img0_subject1d);
	if (img0_subject4d)
	{
		for (c=0;c<img0_sz_subject[3]; c++)
		{
			if (c!=channelNo0_ref && channelNo0_ref>=0) //use for a particular channel only
				continue; 
			
			for (i=0;i<cur_img_warped->getTotalElementNumber();i++) img_warped_ref1d[i] = 0;

			for (k=0; k<img0_sz_subject[2]; k++)
			{
				for (j=0; j<img0_sz_subject[1]; j++)
				{
					for (i=0; i<img0_sz_subject[0]; i++)
					{
						img_warped_ref3d[startpos_subject[2]+k][startpos_subject[1]+j][startpos_subject[0]+i] = (MYFLOAT_JBA)img0_subject4d[c][k][j][i]/Normalizer;
					}
				}
			}
			
			df_warp(cur_img_warped, cur_df);
			
			for (k=0; k<img0_sz_subject[2]; k++)
			{
				for (j=0; j<img0_sz_subject[1]; j++)
				{
					for (i=0; i<img0_sz_subject[0]; i++)
					{
						img0_subject4d[c][k][j][i] = (T)(img_warped_ref3d[startpos_subject[2]+k][startpos_subject[1]+j][startpos_subject[0]+i] * Normalizer);
					}
				}
			}
			
			printf("Finish warping the %ldth channel.\n", c);
		}
		
		delete4dpointer(img0_subject4d, img0_sz_subject[0], img0_sz_subject[1], img0_sz_subject[2], img0_sz_subject[3]);
		if (cur_img_warped) {delete cur_img_warped; cur_img_warped=0;}
	}
	else
	{
		fprintf(stderr, "Fail to allocate memory: file=[%s] line=[%d].\n", __FILE__, __LINE__);
		if (cur_img_warped) {delete cur_img_warped; cur_img_warped=0;}
		return false;
	}
	
	printf("Done: warping. \n");
	return true;
}

//
#include "displacefield_comput.h"

#endif

