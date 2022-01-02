//jba_mainfunc.h
// by Hanchuan Peng
// 2006-2011

#ifndef __JBA_MAINFUNC__
#define __JBA_MAINFUNC__

#include "basic_memory.cpp"
#include "img_definition.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

string gen_file_name(string basename, int layer, string otherinfo);

typedef unsigned char UINT8_JBA;
typedef float MYFLOAT_JBA;

#if defined (_MSC_VER) 
#include "vcdiff.h"
#else
#endif



#define POSITIVE_Y_COORDINATE


#include "wkernel.h" 

class Coord3D_JBA
{
public:
    MYFLOAT_JBA x,y,z; 
	Coord3D_JBA(float x0,float y0,float z0) {x=x0;y=y0;z=z0;}
	Coord3D_JBA() {x=y=z=0;}
};

enum PointMatchMethodType {MATCH_MI, MATCH_MULTIPLE_MI_INT_CORR, MATCH_INTENSITY, MATCH_MEANCIRCLEINTENSITY, MATCH_CORRCOEF}; 
enum DFComputeMethodType {DF_GEN_TPS, DF_GEN_TPS_LINEAR_INTERP}; 

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
	string file_landmark_target;
	string file_landmark_subject;
	string tag_output_image_file;

	PointMatchMethodType method_match_landmarks;
	DFComputeMethodType method_df_compute;
	bool b_nearest_interp; 

    Coord3D_JBA targetMultiplyFactor;
    Coord3D_JBA subjectMultiplyFactor;

	bool b_search_around_preset_subject_pos; 
	int radius_search_around_preset_subject_pos; 
	int cur_level; 
	
	double hierachical_match_level; 
	
	BasicWarpParameter() 
	{
		nloop_localest=10;nloop_smooth=10;imgxy_minsz=64;localest_halfwin=10;b_verbose_print=0;
		file_landmark_target="preDefinedLandmark.cfg";file_landmark_subject="unsetyet";
		tag_output_image_file="";
		hierachical_match_level = 1; 
		
		method_match_landmarks = MATCH_MULTIPLE_MI_INT_CORR;
		method_df_compute = DF_GEN_TPS_LINEAR_INTERP;
		
		b_search_around_preset_subject_pos = false;
		radius_search_around_preset_subject_pos = 20;
		cur_level = 0;
		
		b_nearest_interp = false;
	}
	BasicWarpParameter(const BasicWarpParameter & bwp)
	{
		nloop_localest = bwp.nloop_localest;
		nloop_smooth = bwp.nloop_smooth;
		imgxy_minsz = bwp.imgxy_minsz;
		localest_halfwin = bwp.localest_halfwin;
		b_verbose_print = bwp.b_verbose_print;

		file_landmark_target = bwp.file_landmark_target;
		file_landmark_subject = bwp.file_landmark_subject;
		tag_output_image_file = bwp.tag_output_image_file;
		
		hierachical_match_level = bwp.hierachical_match_level;
		
		method_match_landmarks = bwp.method_match_landmarks;
		method_df_compute = bwp.method_df_compute;

		b_search_around_preset_subject_pos = bwp.b_search_around_preset_subject_pos;
		radius_search_around_preset_subject_pos = bwp.radius_search_around_preset_subject_pos;
		cur_level = bwp.cur_level;
		
		b_nearest_interp = bwp.b_nearest_interp;
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
							 string dfile_volumechangemap,
							 string file_displacementfield,
							 int warpType,
							 const BasicWarpParameter & bwp);

class DisplaceFieldF3D
{
public:
    UINT8_JBA b_transform;
	
	MYFLOAT_JBA sx, sy, sz; //shift of x,y,z
	DisplaceFieldF3D() {sx=sy=sz=0; b_transform=0;}
	DisplaceFieldF3D(double vv) {sx=sy=sz=vv; b_transform=0;}
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
	MYFLOAT_JBA mzx, mzy, mzz; 
    MYFLOAT_JBA si, sb; 
    WarpParameterAffine3D () 
	{
	  mxx=myy=mzz=1.0; mxy=mxz=myx=myz=mzx=mzy=0; sx=sy=sz=0; si=1; sb=0; 
	  b_transform=0; 
	} 
	bool copy(WarpParameterAffine3D *wp) 
	{
	  if (!wp) return false;
	  mxx = wp->mxx; mxy = wp->mxy; mxz = wp->mxz;
	  myx = wp->myx; myy = wp->myy; myz = wp->myz;
	  mzx = wp->mzx; mzy = wp->mzy; mzz = wp->mzz;
	  sx  = wp->sx;  sy  = wp->sy;  sz  = wp->sz;
	  si  = wp->si;  sb  = wp->sb;
	  b_transform = wp->b_transform; 
	  return true;
	} 
	bool copy(WarpParameterAffine3D &wp) 
	{
	  mxx = wp.mxx; mxy = wp.mxy; mxz = wp.mxz;
	  myx = wp.myx; myy = wp.myy; myz = wp.myz;
	  mzx = wp.mzx; mzy = wp.mzy; mzz = wp.mzz;
	  sx  = wp.sx;  sy  = wp.sy;  sz  = wp.sz;
	  si  = wp.si;  sb  = wp.sb;
	  b_transform = wp.b_transform; 
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
	  if(b_transform==0 && wp->b_transform==1) b_transform=1; 
	  return true;
	} 
	bool add(WarpParameterAffine3D &wp)
	{
	  mxx += wp.mxx; mxy += wp.mxy; mxz += wp.mxz;
	  myx += wp.myx; myy += wp.myy; myz += wp.myz;
	  mzx += wp.mzx; mzy += wp.mzy; mzz += wp.mzz;
	  sx  += wp.sx;  sy  += wp.sy;  sz  += wp.sz;
	  si  += wp.si;  sb  += wp.sb;
	  if(b_transform==0 && wp.b_transform==1) b_transform=1; 
	  return true;
	}
	void time(double d)
	{
	  mxx *= d; mxy *= d; mxz *= d;
	  myx *= d; myy *= d; myz *= d;
	  mzx *= d; mzy *= d; mzz *= d;
	  sx  *= d;  sy *= d;  sz *= d;
	  si  *= d;  sb *= d;
	}  
	void resetToAllZeros()
	{
	  mxx = 0; mxy = 0; mxz = 0;
	  myx = 0; myy = 0; myz = 0;
	  mzx = 0; mzy = 0; mzz = 0;
	  sx  = 0; sy  = 0; sz  = 0;
	  si  = 0; sb  = 0;
	}
	void resetToDefault()
	{
	  mxx = 1; mxy = 0; mxz = 0;
	  myx = 0; myy = 1; myz = 0;
	  mzx = 0; mzy = 0; mzz = 1;
	  sx  = 0; sy  = 0; sz  = 0;
	  si  = 1; sb  = 0;
	  b_transform=0; 
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
	 
	V3DLONG my_sz[3]; 
    V3DLONG my_len3d; 
    
	Vol3DSimple<MYFLOAT_JBA> * img_target;
	V3DLONG spos_target [3]; 
    
	Vol3DSimple<MYFLOAT_JBA> * img_subject;
	V3DLONG spos_subject [3]; 
    
	Vol3DSimple<MYFLOAT_JBA> * img_warped;
	
	WarpParameterAffine3D warpPara_global;
	Vol3DSimple<WarpParameterAffine3D> * warpPara_local;
	V3DLONG len_warpPara_local;

    bool downsample3dvol(Vol3DSimple<MYFLOAT_JBA> * &outimg, Vol3DSimple<MYFLOAT_JBA> * inimg, double dfactor);
    bool upsample3dvol(Vol3DSimple<MYFLOAT_JBA> * & outimg, Vol3DSimple<MYFLOAT_JBA> * inimg, double dfactor);

	bool upsampleDisplaceField(Vol3DSimple<DisplaceFieldF3D> * & out_df, Vol3DSimple<DisplaceFieldF3D> * in_df, double dfactor);
	
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
	template <class T1, class T2> Warp3D(
		const T1 * img0_target1d, 
		const V3DLONG *img0_sz_target, 
		const int channelNo0_ref_target, 
		const T2 * img0_subject1d, 
		const V3DLONG *img0_sz_subject, 
		const int channelNo0_ref_subject, 
		string file0_target,
		string file0_subject,
		string file0_warped);

	~Warp3D();
	
	Vol3DSimple<DisplaceFieldF3D> * do_global_affine(const BasicWarpParameter & bwp); 
    
	Vol3DSimple<DisplaceFieldF3D> *  do_global_transform(const BasicWarpParameter & bwp); 

	Vol3DSimple<DisplaceFieldF3D> * do_local_topdown_blocktps(const BasicWarpParameter & bwp); 
    
	Vol3DSimple<DisplaceFieldF3D> * do_local_topdown_blocktps_fast(const BasicWarpParameter & bwp); 
	
	Vol3DSimple<DisplaceFieldF3D> * do_landmark_warping(const BasicWarpParameter & bwp); 
	Vol3DSimple<DisplaceFieldF3D> * do_landmark_warping_hier(const BasicWarpParameter & bwp); 

	Vol3DSimple<DisplaceFieldF3D> * do_landmark_in_file_warping(const BasicWarpParameter & bwp); 
	
	template <class T1, class T2> bool setInitData(
		const T1 * img0_target1d, 
		const V3DLONG *img0_sz_target, 
		const int channelNo0_ref_target, 
		const T2 * img0_subject1d, 
		const V3DLONG *img0_sz_subject, 
		const int channelNo0_ref_subject, 
		string file0_target,
		string file0_subject,
		string file0_warped);

	template <class T> bool getWarpedData(T * &outimg_warped, V3DLONG * &out_sz_warped, const int datatype);
	
	template <class T> bool applyDFtoChannel(
		const T * img0_subject1d, 
		const V3DLONG *img0_sz_subject, 
		const int channelNo0_ref, 
		V3DLONG spos_subject [3],
		Vol3DSimple <DisplaceFieldF3D> *cur_df,
		bool b_nearest_interp
	);
	
	template <class T> bool doWarpUsingDF(T * &outimg_warped, V3DLONG * &out_sz_warped, T * inimg_subject, V3DLONG * inimg_sz, const int datatype_warped, Vol3DSimple <DisplaceFieldF3D> * cur_df, bool b_nearest_interp);
	
	bool warpSubjectImage(Vol3DSimple <DisplaceFieldF3D> *df, bool b);
	
	V3DLONG * get_spos_subject() {return spos_subject;} 
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

bool df_warp(Vol3DSimple<MYFLOAT_JBA> * img, Vol3DSimple<DisplaceFieldF3D> * p, bool b_nearest_interp); 
bool df_warp(Vol3DSimple<MYFLOAT_JBA> * img, Vol3DSimple<WarpParameterAffine3D> * p, bool b_nearest_interp);

bool df_add(Vol3DSimple<DisplaceFieldF3D> * addee, Vol3DSimple<DisplaceFieldF3D> * adder, bool b_nearest_interp);
bool df_copy(Vol3DSimple<DisplaceFieldF3D> * copyee, Vol3DSimple<DisplaceFieldF3D> * copyer);


Vol3DSimple<DisplaceFieldF3D> *  get_DF_of_affine_warp(Vol3DSimple<MYFLOAT_JBA> * img, WarpParameterAffine3D * p);
Vol3DSimple<DisplaceFieldF3D> *  get_DF_of_affine_warp(V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, WarpParameterAffine3D * p);


bool eba3d(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable,
						Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, Vol3DSimple<MYFLOAT_JBA> * subject,
						V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze, 
						bool b_returnDTable);
bool eba3d_interface(WarpParameterAffine3D & wp_current,  Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg, int nloops);
void affine_transform(Coord3D_JBA * c, V3DLONG numCoord, WarpParameterAffine3D * p, Coord3D_JBA * offset);
bool affine_warp(Vol3DSimple<MYFLOAT_JBA> * img, WarpParameterAffine3D * p);

bool eba3d_noshift(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable, 
                                Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, 
								Vol3DSimple<MYFLOAT_JBA> * subject,
                                V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
								bool b_returnDTable); 


bool est_best_scale_3d(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable, 
                       Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, 
	  				   Vol3DSimple<MYFLOAT_JBA> * subject,
                       V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
	  				   Vol3DSimple<UINT8_JBA> * mask,
					   bool b_returnDTable);

bool ebs3d(DisplaceFieldF3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable, 
                       Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft, 
	  				   Vol3DSimple<MYFLOAT_JBA> * subject,
                       V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
	  				   Vol3DSimple<UINT8_JBA> * mask,
					   bool b_returnDTable);

bool ebs3d_interface(DisplaceFieldF3D & wp_current,  Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg, int nloops);

void shift_transform(Coord3D_JBA * c, V3DLONG numCoord, DisplaceFieldF3D * p, Coord3D_JBA * offset);
bool shift_warp(Vol3DSimple<MYFLOAT_JBA> * img, DisplaceFieldF3D * p);

bool lcn(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord, 
                       MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, 
  		               V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);
bool lcl(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord, 
                       MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, 
  		               V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);
Vol3DSimple <MYFLOAT_JBA> * lir3d(V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, Vol3DSimple <MYFLOAT_JBA> * df_regular_grid); 


Vol3DSimple<DisplaceFieldF3D> * cdum(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, DFComputeMethodType method=DF_GEN_TPS_LINEAR_INTERP);
Vol3DSimple<DisplaceFieldF3D> * compute_df_tps(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);
Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_interp(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, 
													  DFComputeMethodType interp_method); 
Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_subsampled_volume(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, 
																 V3DLONG gfactor_x, V3DLONG gfactor_y, V3DLONG gfactor_z); 

template <class T> bool conv3d_1dvec(MYFLOAT_JBA ***outimg3d, T *** inimg3d, V3DLONG d0, V3DLONG d1, V3DLONG d2, MYFLOAT_JBA *vec, V3DLONG lenvec, int dim_of_conv);

template <class T> bool find3DGradient(Vol3DSimple <T> * & g, Vol3DSimple <T> *subject);



#endif

