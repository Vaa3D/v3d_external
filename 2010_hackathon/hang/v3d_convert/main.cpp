// Aug-12-2011 add -down-sampling
// todo : add down-sampling in run_with_paras
#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 
#include <vector>

#include "stackutil.h"
#include "parser.h"
#include "img_rotate.h"
#include "gaussian_blur.h"
#include "img_threshold.h"
#include "img_center.h"
#include "img_sampling.h"
#include "img_segment.h"
#include "img_operate.h"
#include "dist_transform.h"
#include "neuron_tracing.h"

using namespace std;


void printHelp();
void printVersion();
bool run_with_paras(InputParas paras, string &s_error);
bool convert_uint8_to_double(double * &outimg1d, unsigned char* inimg1d, V3DLONG sz[3]);
bool convert_double_to_uint8(unsigned char* &outimg1d,double * inimg1d,  V3DLONG sz[3]);
bool scale_double_to_uint8(unsigned char* &outimg1d, double * inimg1d,  V3DLONG sz[3]);
inline bool check_image_binary(unsigned char* &inimg1d, V3DLONG *&sz);

bool is_save_img = true;

SupportedCommand supported_commands[] = {{"-info",0},{"-list",0},{"-down-sampling",1},{"-marker-radius",1},{"-center-marker",1},{"-distance-transform",1},{"-maximum-component",1},{"-average-threshold", 1},{"-adaptive-threshold", 1},{"-otsu-threshold", 1},{"-binary-threshold",1},{"-white-threshold",1},{"-black-threshold",1},{"-rotatex", 1}, {"-rotatey", 1}, {"-rotatez", 1}, {"-channel", 1}, {"-gaussian-blur", 1}, {"-resize", 1}, {"-crop", 1}, {"-img-operate", 1}};

int main(int argc, char* argv[])
{
	if(argc == 1 || (argc == 2 && (string(argv[1]) == "-h"))) {printHelp(); return 0;}
	if(argc == 2 && (string(argv[1]) == "-v" || string(argv[1]) == "--version")) {printVersion(); return 0;}

	InputParas paras(supported_commands, sizeof(supported_commands)/sizeof(SupportedCommand));
	string s_error("");

	if(! parse_paras(argc, argv, paras, s_error)){cout<<"Invalid paras : "<< s_error<<endl; return 0;}
	if(! run_with_paras(paras, s_error)) { cout<<"Run error : "<< s_error <<endl; return false;}
	return 0;
}

bool run_with_paras(InputParas paras, string & s_error)
{
	s_error = "";

	//if(paras.filelist.size() != 2){s_error += "currently we only support one image as input and one image as output"; return false;}
	if(paras.is_empty())   // type convert
	{
		if(paras.filelist.size() == 2)
		{
			string infile = paras.filelist.at(0);
			string outfile = paras.filelist.at(1);
			if(infile == outfile) return true;

			unsigned char * data1d = NULL;
			V3DLONG *sz = NULL;
			int datatype;
			if(!loadImage((char*) infile.c_str(), data1d, sz, datatype)) {s_error += "loadImage(\""; s_error += infile; s_error+="\")  error"; return false;}
			if(!saveImage((char*) outfile.c_str(), data1d, sz, datatype)) {s_error += "saveImage(\""; s_error += outfile; s_error+="\") error"; return false;}
			return true;
		}
		else { s_error += "need output image"; return false;}
	}

	string infile = paras.filelist.size() >= 1 ? paras.filelist.at(0) : string("");
	string outfile = paras.filelist.size() >=2 ? paras.filelist.at(1) : string(infile+"_out.raw");
	unsigned char * indata1d = NULL, * outdata1d = NULL;
	V3DLONG* in_sz = 0, * out_sz = 0;
	int datatype;

	if(infile != "" && !loadImage((char*) infile.c_str(), indata1d, in_sz, datatype)) {s_error += "loadImage(\""; s_error += infile; s_error+="\")  error"; return false;}

	int channel = 0;  

	string cmd_name("");
#define CHECK_CHANNEL if(paras.is_exist("-channel")){if(!paras.get_int_para(channel, "-channel", s_error)){if(channel <= 0 || channel > in_sz[3]) s_error += "channel should be larger then 0 and less or equal then image channel number"; return false;}channel = channel - 1; indata1d = indata1d + in_sz[0] * in_sz[1] * in_sz[2] * channel;} else if(in_sz[3] != 1){s_error += "please specify -channel"; return false;}

#define REFRESH_INDATA1D if(indata1d){delete [] indata1d; indata1d = 0;} if(in_sz){delete [] in_sz; in_sz = 0;} indata1d = outdata1d; in_sz = out_sz;
	while(paras.get_next_cmd(cmd_name))
	{
		if(cmd_name == "-info")
		{
			cout<<"size : "<<in_sz[0]<<"x"<<in_sz[1]<<"x"<<in_sz[2]<<"x"<<in_sz[3]<<endl;
			is_save_img = false;
		}
		else if(cmd_name == "-list")
		{
			int cmds_num = sizeof(supported_commands)/sizeof(SupportedCommand);
			for(int i = 0; i < cmds_num; i++) cout<<supported_commands[i].para_name<<" ";
			cout<<endl<<endl;
			cout<<"Steps to add new function :"<<endl;
			cout<<"1. prepare you .h .cpp files, and add to .pro"<<endl;
			cout<<"2. in main.cpp, add an item in supported_commands variable"<<endl;
			cout<<"3. in run_with_paras(), add your function on how to process your command"<<endl;
			cout<<"4. set outfile and is_save_img if need to save your image result"<<endl;
			cout<<""<<endl;
			is_save_img = false;
		}
		else if(cmd_name == "-marker-radius")
		{
			CHECK_CHANNEL;
			
			if(paras.filelist.size() < 2 || paras.filelist.at(1).find_last_of(".marker") == string::npos){s_error += " please specify input marker file"; return false;}
			string marker_file = paras.filelist.at(1);
			ImageMarker marker;
			ifstream ifs(marker_file.c_str()); if(ifs.fail()){s_error += string(" unable to open marker file " + marker_file); return false;}
			//if(ifs.get() == '#')ifs.ignore(1000,'\n'); 
			ifs>>marker.x;ifs.ignore(10,',');ifs>>marker.y;ifs.ignore(10,',');ifs>>marker.z; ifs.close();
			cout<<"marker:"<<marker.x<<" "<<marker.y<<" "<<marker.z<<endl;
			double thresh; if(!paras.get_double_para(thresh,"-marker-radius",s_error)) return false;
			cout<<"thresh: "<<thresh<<endl;
			if(!binary_threshold(thresh, indata1d, in_sz, outdata1d))return false;
			cout<<"radius : "<<markerRadius(outdata1d, in_sz, marker)<<endl;
			is_save_img = false;
		}
		else if(cmd_name == "-center-marker")
		{
			CHECK_CHANNEL;

			string marker_file = paras.filelist.size() >=2 ? paras.filelist.at(1) : string(infile + "_output.marker");
			int    method; if(!paras.get_int_para(method,"-center-marker",0,s_error,":")) return false;
			double thresh; if(!paras.get_double_para(thresh,"-center-marker",1,s_error,":")){s_error += "\nplease specify -center-marker para as threshold"; return false;}
			cout<<"marker_file "<<marker_file<<endl;
			cout<<"thresh : "<<thresh<<endl;
			if(method == 0) cout<<"method : maximum xyzplane density center"<<endl;
			else cout<<"method : maximum_connected_component_center"<<endl;
			double* pos = 0;
			if(method == 0 && !maximum_xyzplane_density_center(pos, indata1d, in_sz,thresh)){s_error += "calculate maximum xyzplane density center marker error"; return false;}
			else if(method == 1 && !maximum_connected_component_center(pos, indata1d, in_sz,thresh)){s_error += "calculate maximum_connected_component center marker error"; return false;}
			ofstream ofs(marker_file.c_str()); if(ofs.fail()){s_error += "open marker file error"; return false;}
			cout<<"marker center : "<<pos[0]<<","<<pos[1]<<","<<pos[2]<<endl;
			ofs<<pos[0]<<","<<pos[1]<<","<<pos[2]<<endl; ofs.close();
			is_save_img = false;
		}
		else if(cmd_name == "-distance-transform")
		{
			CHECK_CHANNEL;
			
			int method; if(!paras.get_int_para(method,"-distance-transform",0,s_error,":")) return false;
			double thresh; if(!paras.get_double_para(thresh,"-distance-transform",1,s_error,":")) return false;
			if(method == 0) cout<<"method : normal distance transform"<<endl;
			else cout<<"method : fastmarching method"<<endl;
			cout<<"thresh : "<<thresh<<endl;
			double * phi = 0;
			if(method == 0 && !normal_distance_transform(phi, indata1d, in_sz, thresh)){s_error += "failed to calculate normal distance transformation"; return false;}
			else if(method == 1 && !fastmarching_distance(phi, indata1d, in_sz, thresh)){s_error += "failed to do fast marching"; return false;}
			if(method == 1) for(V3DLONG i = 0; i < in_sz[0] * in_sz[1] * in_sz[2]; i++) {phi[i] = - phi[i]; if(phi[i] < 0) phi[i] = 0.0;}
			if(!scale_double_to_uint8(outdata1d, phi, in_sz)){s_error += "scale_double_to_uint8 error"; return false;}
			if(phi) {delete [] phi; phi = 0;}
		}
		else if(cmd_name == "-down-sampling")
		{
			CHECK_CHANNEL

			int factor; if(!paras.get_int_para(factor, "-down-sampling", s_error)) return false;
			cout<<"factor : "<<factor<<endl;
			if(!down_sampling(factor, indata1d, in_sz, outdata1d, out_sz)){s_error += " down sampling error"; return false;}
		}
		else if(cmd_name == "-adaptive-threshold")
		{
			CHECK_CHANNEL;
			int sampling_interval; if(!paras.get_int_para(sampling_interval,"-adaptive-threshold",0,s_error,"+")) return false;
			int sampling_number; if(!paras.get_int_para(sampling_number,"-adaptive-threshold",1,s_error,"+")) return false;
			cout<<"sampling_interval = "<<sampling_interval<<" , sampling_number = "<<sampling_number<<endl;
			if(!adaptive_threshold(outdata1d, indata1d, in_sz, sampling_interval, sampling_number)){s_error += " adaptive thresholding error!"; return false;}
			out_sz = new V3DLONG[4];
			out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;
		}
		else if(cmd_name == "-average-threshold")
		{
			CHECK_CHANNEL;

			int thresh_type = 0; if(!paras.get_int_para(thresh_type,"-otsu-threshold",s_error)) return false;
			double thresh_value = 100.0; if(!average_threshold(thresh_value, indata1d, in_sz)) return false;
			cout<<"thresh_value : "<<thresh_value<<endl;
			ostringstream oss;  oss<<"thresh"<<thresh_value<<"_"<<infile;
			outfile = paras.filelist.size() >=2 ? paras.filelist.at(1) : oss.str();
			cout<<"output file : "<<outfile<<endl;

			if(thresh_type == 0 && !black_threshold(thresh_value,indata1d, in_sz, outdata1d) ){s_error += " otsu threshold with black threshold error"; return false;}
			else if(thresh_type == 1 && !white_threshold(thresh_value,indata1d, in_sz, outdata1d) ){s_error += " otsu threshold with white threshold error"; return false;}
			else if(thresh_type == 2 && !binary_threshold(thresh_value,indata1d, in_sz, outdata1d) ){s_error += " otsu threshold with binary threshold error"; return false;}
			out_sz = new V3DLONG[4];
			out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;
			
			//REFRESH_INDATA1D;
		}
		else if(cmd_name == "-otsu-threshold")
		{
			CHECK_CHANNEL;

			int thresh_type = 0; if(!paras.get_int_para(thresh_type,"-otsu-threshold",s_error)) return false;
			double thresh_value = 100.0; if(!otsu_threshold(thresh_value, indata1d, in_sz)) {s_error += " otsu_threshold error."; return false;}
			cout<<"thresh_value : "<<thresh_value<<endl;
			ostringstream oss;  oss<<"thresh"<<thresh_value<<"_"<<infile;
			outfile = paras.filelist.size() >=2 ? paras.filelist.at(1) : oss.str();
			cout<<"output file : "<<outfile<<endl;

			if(thresh_type == 0 && !black_threshold(thresh_value,indata1d, in_sz, outdata1d) ){s_error += " otsu threshold with black threshold error"; return false;}
			else if(thresh_type == 1 && !white_threshold(thresh_value,indata1d, in_sz, outdata1d) ){s_error += " otsu threshold with white threshold error"; return false;}
			else if(thresh_type == 2 && !binary_threshold(thresh_value,indata1d, in_sz, outdata1d) ){s_error += " otsu threshold with binary threshold error"; return false;}
			out_sz = new V3DLONG[4];
			out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;
			
			//REFRESH_INDATA1D;
		}
		else if(cmd_name == "-black-threshold")
		{
			CHECK_CHANNEL

			double black_thresh_value; if(!paras.get_double_para(black_thresh_value,"-black-threshold",s_error))return false;
			cout<<"black_thresh_value = "<<black_thresh_value<<endl;
			if(!black_threshold(black_thresh_value,indata1d, in_sz, outdata1d)){s_error += "black threshold error"; return false;}
			out_sz = new V3DLONG[4];
			out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;
			
			//REFRESH_INDATA1D;
		}
		else if(cmd_name == "-white-threshold")
		{
			CHECK_CHANNEL

			double white_thresh_value; if(!paras.get_double_para(white_thresh_value,"-white-threshold",s_error))return false;
			cout<<"white_thresh_value = "<<white_thresh_value<<endl;
			if(!white_threshold(white_thresh_value,indata1d, in_sz, outdata1d)){s_error += "white threshold error"; return false;}
			out_sz = new V3DLONG[4];
			out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;

			//REFRESH_INDATA1D;
		}
		else if(cmd_name == "-maximum-component")
		{
			CHECK_CHANNEL;

			double binary_thresh_value; if(!paras.get_double_para(binary_thresh_value,"-maximum-component",s_error))return false;
			cout<<"binary_thresh_value = "<<binary_thresh_value<<endl;
			if(!maximum_connected_component(outdata1d, indata1d, in_sz, binary_thresh_value)){s_error += "get maximum connected component error"; return false;}
			out_sz = new V3DLONG[4];
			out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;
			ostringstream oss; oss << infile<<"_thresh"<<binary_thresh_value<<"_maxcomp.raw";
			outfile = paras.filelist.size() >=2 ? paras.filelist.at(2) : oss.str();
		}
		else if(cmd_name == "-binary-threshold")
		{
			CHECK_CHANNEL

			double binary_thresh_value; if(!paras.get_double_para(binary_thresh_value,"-binary-threshold",s_error))return false;
			cout<<"binary_thresh_value = "<<binary_thresh_value<<endl;
			if(!binary_threshold(binary_thresh_value,indata1d, in_sz, outdata1d)){s_error += "binary threshold error"; return false;}
			out_sz = new V3DLONG[4];
			out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;

			//REFRESH_INDATA1D;
		}
		else if(cmd_name == "-rotatex")
		{
			//CHECK_CHANNEL
			if(paras.is_exist("-channel")){if(!paras.get_int_para(channel, "-channel", s_error)){if(channel <= 0 || channel > in_sz[3]) s_error += "channel should be larger then 0 and less or equal then image channel number"; return false;}channel = channel - 1;indata1d = indata1d + in_sz[0] * in_sz[1] * in_sz[2] * channel;}

			double theta; if(!paras.get_double_para(theta,"-rotatex", 0, s_error,"%")) return false;
			cout<<"theta = "<<theta<<endl;
			if(!rotate_along_xaxis(theta, indata1d, in_sz, outdata1d, out_sz, 0)){s_error += "rotatez error"; return false;}
		}
		else if(cmd_name == "-rotatey")
		{
			//CHECK_CHANNEL
			if(paras.is_exist("-channel")){if(!paras.get_int_para(channel, "-channel", s_error)){if(channel <= 0 || channel > in_sz[3]) s_error += "channel should be larger then 0 and less or equal then image channel number"; return false;}channel = channel - 1;indata1d = indata1d + in_sz[0] * in_sz[1] * in_sz[2] * channel;}

			double theta; if(!paras.get_double_para(theta,"-rotatey", 0, s_error,"%")) return false;
			cout<<"theta = "<<theta<<endl;
			if(!rotate_along_yaxis(theta, indata1d, in_sz, outdata1d, out_sz, 0)){s_error += "rotatez error"; return false;}
		}
		else if(cmd_name == "-rotatez")
		{
			//CHECK_CHANNEL
			if(paras.is_exist("-channel")){if(!paras.get_int_para(channel, "-channel", s_error)){if(channel <= 0 || channel > in_sz[3]) s_error += "channel should be larger then 0 and less or equal then image channel number"; return false;}channel = channel - 1;indata1d = indata1d + in_sz[0] * in_sz[1] * in_sz[2] * channel;}

			double theta; if(!paras.get_double_para(theta,"-rotatez", 0, s_error,"%")) return false;
			cout<<"theta = "<<theta<<endl;
			if(!rotate_along_zaxis(theta, indata1d, in_sz, outdata1d, out_sz, 0)){s_error += "rotatez error"; return false;}
		}
		else if(cmd_name == "-img-operate")
		{
			string method; if((method = paras.get_para("-img-operate")) == "") return false;
			cout<<"method : "<<method<<endl;
			unsigned char *inimg1 = indata1d,  * inimg2 = 0; 
			if(method != "complement" && method != "not")
			{
				if(paras.filelist.size() < 2){s_error += "need at least two input images"; return false;}
				string infile2 = paras.filelist.at(1);
				outfile = paras.filelist.size() >=3 ? paras.filelist.at(2) : string(infile+ "_" + method + ".raw");
				V3DLONG * in_sz2 = 0;
				int datatype2;
				if(!loadImage((char*) infile2.c_str(), inimg2, in_sz2, datatype2)) {s_error += "loadImage(\""; s_error += infile2; s_error+="\")  error"; return false;}
				if(datatype != datatype2 || in_sz[0] != in_sz2[0] || in_sz[1] != in_sz2[1] || in_sz[2] != in_sz2[2] || in_sz[3] != in_sz2[3]){s_error += string("different datatype or different image size for " + infile + " and " + infile2); return false;}
				delete [] in_sz2; in_sz2 = 0;
			}
			else 
				outfile = paras.filelist.size() >=2 ? paras.filelist.at(1) : string(infile+ "_" + method + ".raw");
			if(!img_operate(method, outdata1d, inimg1, in_sz, inimg2)){s_error += " img operate error"; return false;}
			if(inimg2){delete [] inimg2; inimg2 = 0;}
		}
		else if(cmd_name == "-gaussian-blur")
		{
			CHECK_CHANNEL

			int para_num = paras.get_delim_num("-gaussian-blur", "x") + 1;
			if(para_num == 2)
			{
				double sigma; int radius;  if(!paras.get_double_para(sigma,"-gaussian-blur",0,s_error,"x") || !paras.get_int_para(radius, "-gaussian-blur",1, s_error,"x")) return false;
				cout<<"sigma = "<<sigma<<" radius = "<<radius<<endl;

				if(!compute_gaussian_blur(outdata1d, indata1d, in_sz, sigma, radius)){ s_error += "failed to compute gaussian-blur"; return false;}
			}
			else if(para_num == 1)
			{
				double sigma;if(!paras.get_double_para(sigma,"-gaussian-blur",0,s_error,"x")) return false;
				cout<<"sigma = "<<sigma<<endl;
				double * dst = new double[in_sz[0] * in_sz[1] * in_sz[2]];
				if(!smooth(dst, indata1d, in_sz, sigma)){ s_error += "failed to compute gaussian-blur"; return false;}
				outdata1d = new unsigned char[in_sz[0] * in_sz[1] * in_sz[2]];
				if(!convert_double_to_uint8(outdata1d, dst, in_sz)){s_error += "convert_double_to_uint8 error"; return false;}
				if(dst){delete [] dst; dst = 0;}
			}
		}
	}
	if(is_save_img)
	{
		if(out_sz == 0) {out_sz = new V3DLONG[4]; out_sz[0] = in_sz[0]; out_sz[1] = in_sz[1]; out_sz[2] = in_sz[2]; out_sz[3] = 1;}
		if(!saveImage((char*) outfile.c_str(), outdata1d, out_sz, datatype)) {s_error += "saveImage(\""; s_error += outfile; s_error+="\") error"; return false;}
	}
	if(indata1d) {delete [] indata1d; outdata1d = 0;}
	if(in_sz) {delete [] in_sz; in_sz = 0;}
	if(outdata1d) {delete [] outdata1d; outdata1d = 0;}
	if(out_sz) {delete [] out_sz; out_sz = 0;}

	return true;
}

void printVersion()
{
	cout<<"Version : 1.0"<<endl;
}

bool convert_uint8_to_double(double * &outimg1d, unsigned char* inimg1d, V3DLONG sz[3])
{
	if(outimg1d == 0 || inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return 0;
	V3DLONG tol = sz[0] * sz[1] * sz[2];
	if(outimg1d == 0) outimg1d = new double[tol];
	for(V3DLONG i = 0; i < tol ; i++) outimg1d[i] = double(inimg1d[i])/255.0;
	return true;
}

bool convert_double_to_uint8(unsigned char* &outimg1d, double * inimg1d,  V3DLONG sz[3])
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG tol = sz[0] * sz[1] * sz[2];
	if(outimg1d == 0) outimg1d = new unsigned char[tol];
	for(V3DLONG i = 0; i < tol ; i++) outimg1d[i] = (unsigned char)(inimg1d[i] * 255.0 + 0.5);
	return true;
}
bool scale_double_to_uint8(unsigned char* &outimg1d, double * inimg1d,  V3DLONG sz[3])
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG tol = sz[0] * sz[1] * sz[2];
	if(outimg1d == 0) outimg1d = new unsigned char[tol];
	double max_value = inimg1d[0];
	double min_value = inimg1d[0];
	for(V3DLONG i = 1; i < tol; i++)
	{
		max_value = inimg1d[i] > max_value ? inimg1d[i] : max_value;
		min_value = inimg1d[i] < min_value ? inimg1d[i] : min_value;
	}
	double factor = 255.0 / (max_value - min_value);
	for(V3DLONG i = 0; i < tol ; i++) outimg1d[i] = (unsigned char)((inimg1d[i] - min_value) * factor + 0.5);
	return true;
}

inline bool check_image_binary(unsigned char* &inimg1d, V3DLONG *&sz)
{
	V3DLONG tol_sz = sz[0] * sz[1] * sz[2];
	for(V3DLONG i = 0; i < tol_sz; i++){if(inimg1d[i] != 0 || inimg1d[i] != 255) return false;}
	return true;
}

void printHelp()
{
	cout<<"Version: 1.0"<<endl;
	cout<<"Copyright: Opensource Licence"<<endl;
	cout<<"Author: Hang Xiao"<<endl;
	cout<<""<<endl;
	cout<<"v3d_convert is the extension of imagemagic convert. It is designed to support  image operator on three dimension."<<endl;
	cout<<"Currently support .raw .tiff/.tif .lsm image format."<<endl;
	cout<<""<<endl;
	cout<<"Usage: v3d_convert [options ...] file [ [options ...] file ...] [options ...] file "<<endl;
	cout<<""<<endl;
	cout<<" -info                                display the information of input image"<<endl;
	cout<<" -gaussian-blur       sgm x radius       smooth the image by gaussian blur"<<endl;
	cout<<" -rotatex             theta            rotate the image along x-axis with angle theta"<<endl;
	cout<<" -rotatey             theta            rotate the image along y-axis with angle theta"<<endl;
	cout<<" -rotatez             theta            rotate the image along z-axis with angle theta"<<endl;
	cout<<" -black-threshold     thresh_value     threshold the image, intensity lower than thresh_value will be set to zero"<<endl;
	cout<<" -white-threshold     thresh_value     threshold the image, intensity higher than thresh_value will be set to maximum"<<endl;
	cout<<" -binary-threshold    thresh_value     threshold the image, intensity lower than thresh_value to zero, higher to maximum"<<endl;
	cout<<" -otsu-threshold      thresh_type      calculate otsu-thresuld and do black/white/binary-threshold according to thresh_type"<<endl;
	cout<<" -average-threshold   thresh_type      use average intensity as threshold value"<<endl;
	cout<<" -adaptive-threshold  h+d              h is sampling interval, d is then number of sampling points"<<endl;
	cout<<" -maximum-component   thresh_value     get the maximum connected component in the binary thresholding result"<<endl;
	cout<<" -center-marker       mthd:thr_val     methods : 0 for maximum xyz-plane density, 1 for maxim component center"<<endl;
	cout<<" -marker-radius       thr mk_fl        need threshold and marker file to get the estimated radius"<<endl;
	cout<<" -distance-transform  mthd:thr_val     methods : 0 for normal transform, 1 for fast marching method"<<endl;
	cout<<" -img-operate         method           methods: plus minus absminus multiply divide complement and or xor not."<<endl;
	cout<<""<<endl;
}

