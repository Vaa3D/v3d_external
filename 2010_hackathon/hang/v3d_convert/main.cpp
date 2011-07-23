#include <iostream>
#include <string>
#include <vector>

#include "stackutil.h"
#include "v3d_funcs.h"
#include "parser.h"
#include "gaussian_blur.cpp"

using namespace std;


void printHelp();
void printVersion();
bool run_with_paras(InputParas paras, string &s_error);
bool convert_uint8_to_double(double * outimg1d, unsigned char* inimg1d, V3DLONG sz[3]);
bool convert_double_to_uint8(unsigned char* outimg1d,double * inimg1d,  V3DLONG sz[3]);

int cmd_num = 6;
SupportedCommand supported_commands[] = {{"-rotatex", 1}, {"-rotatey", 1}, {"-rotatez", 1}, {"-channel", 1}, {"-gaussian-blur", 1}, {"-resize", 1}, {"-crop", 1}, {"-negate", 0}};

int main(int argc, char* argv[])
{
	if(argc == 1 || (argc == 2 && (string(argv[1]) == "-h"))) {printHelp(); return 0;}
	if(argc == 2 && (string(argv[1]) == "-v" || string(argv[1]) == "--version")) {printVersion(); return 0;}

	InputParas paras(supported_commands, cmd_num);
	string s_error("");

	if(! parse_paras(argc, argv, paras, s_error)){cout<<"Invalid paras : "<< s_error<<endl; return 0;}
	if(! run_with_paras(paras, s_error)) { cout<<"Run error : "<< s_error <<endl; return false;}
	return 0;
}

bool run_with_paras(InputParas paras, string & s_error)
{
	s_error = "";

	if(paras.filelist.size() != 2){s_error += "currently we only support one image as input and one image as output"; return false;}
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

	string infile = paras.filelist.at(0);
	string outfile = paras.filelist.at(1);
	unsigned char * indata1d = NULL, * outdata1d = NULL;
	V3DLONG *in_sz = NULL, * out_sz = NULL;
	int datatype;

	if(!loadImage((char*) infile.c_str(), indata1d, in_sz, datatype)) {s_error += "loadImage(\""; s_error += infile; s_error+="\")  error"; return false;}

	int channel = 0;  if(paras.is_exist("-channel")) if(!paras.get_int_para(channel, "-channel", s_error)) return false;
	if(channel >= in_sz[3]){ s_error += "channel exceed the input image's total channel num"; return false;}
	indata1d = indata1d + in_sz[0] * in_sz[1] * in_sz[2] * channel;
	in_sz[3] = 1;

	string cmd_name("");
	while(paras.get_next_cmd(cmd_name))
	{
		cout<<"command : "<<cmd_name<<endl;
		if(cmd_name == "-rotatexy")
		{
			double thetax; double thetay;
			if(!paras.get_double_para(thetax,"-rotatexy", 0, s_error,"x") || !paras.get_double_para(thetay,"-rotatexy", 1, s_error,"x")) return false;
			cout<<"thetax = "<<thetax<<" thetay = "<<thetay<<endl;
			//if(!rotate_along_xyaxis(thetax, thetay, indata1d, in_sz, outdata1d, out_sz, 0)){s_error += "rotatez error"; return false;}
		}
		else if(cmd_name == "-rotatex")
		{
			double theta; if(!paras.get_double_para(theta,"-rotatex", 0, s_error,"%")) return false;
			cout<<"theta = "<<theta<<endl;
			if(!rotate_along_xaxis(theta, indata1d, in_sz, outdata1d, out_sz, 0)){s_error += "rotatez error"; return false;}
		}
		else if(cmd_name == "-rotatey")
		{
			double theta; if(!paras.get_double_para(theta,"-rotatey", 0, s_error,"%")) return false;
			cout<<"theta = "<<theta<<endl;
			if(!rotate_along_yaxis(theta, indata1d, in_sz, outdata1d, out_sz, 0)){s_error += "rotatez error"; return false;}
		}
		else if(cmd_name == "-rotatez")
		{
			double theta; if(!paras.get_double_para(theta,"-rotatez", 0, s_error,"%")) return false;
			cout<<"theta = "<<theta<<endl;
			if(!rotate_along_zaxis(theta, indata1d, in_sz, outdata1d, out_sz, 0)){s_error += "rotatez error"; return false;}
		}
		else if(cmd_name == "-gaussian-blur")
		{
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
				double * src = new double[in_sz[0] * in_sz[1] * in_sz[2]];
				V3DLONG sz[3];
				sz[0] = in_sz[0];
				sz[1] = in_sz[1];
				sz[2] = in_sz[2];
				if(!convert_uint8_to_double(src, indata1d, sz)){s_error += "convert_uint8_to_double error"; return false;}
				double * dst = new double[in_sz[0] * in_sz[1] * in_sz[2]];
				if(!smooth(dst, src, in_sz, sigma)){ s_error += "failed to compute gaussian-blur"; return false;}
				outdata1d = new unsigned char[sz[0] * sz[1] * sz[2]];
				if(!convert_double_to_uint8(outdata1d, dst, sz)){s_error += "convert_double_to_uint8 error"; return false;}
				if(src){delete [] src; src = 0;}
				if(dst){delete [] dst; dst = 0;}
			}
		}
	}
	if(out_sz == 0) out_sz = in_sz;
	if(!saveImage((char*) outfile.c_str(), outdata1d, out_sz, datatype)) {s_error += "saveImage(\""; s_error += outfile; s_error+="\") error"; return false;}
	if(indata1d) {delete [] indata1d; indata1d = 0;}
	if(outdata1d) {delete [] outdata1d; outdata1d = 0;}

	return true;
}

void printHelp()
{
	cout<<"Version: 1.0"<<endl;
	cout<<"Copyright: Opensource Licence"<<endl;
	cout<<""<<endl;
	cout<<"v3d_convert is the extension of imagemagic convert. It is designed to support  image operator on three dimension."<<endl;
	cout<<"Currently support .raw .tiff/.tif .lsm image format."<<endl;
	cout<<""<<endl;
	cout<<"Usage: v3d_convert [options ...] file [ [options ...] file ...] [options ...] file "<<endl;
	cout<<""<<endl;
	cout<<" -gaussian-blur     geometry"<<endl;
	cout<<" -rotatez           theta"<<endl;
	cout<<""<<endl;
}

void printVersion()
{
	cout<<"Version : 1.0"<<endl;
}

bool convert_uint8_to_double(double * outimg1d, unsigned char* inimg1d, V3DLONG sz[3])
{
	if(outimg1d == 0 || inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return 0;
	V3DLONG tol = sz[0] * sz[1] * sz[2];
	for(V3DLONG i = 0; i < tol ; i++) outimg1d[i] = double(inimg1d[i])/255.0;
	return true;
}

bool convert_double_to_uint8(unsigned char* outimg1d,double * inimg1d,  V3DLONG sz[3])
{
	cout<<"sz[0] = "<<sz[0]<<" sz[1] = "<<sz[1]<<" sz[2] = "<<sz[2]<<endl;
	if(outimg1d == 0 || inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return 0;
	V3DLONG tol = sz[0] * sz[1] * sz[2];
	for(V3DLONG i = 0; i < tol ; i++) outimg1d[i] = (unsigned char)(inimg1d[i] * 255.0);
	return true;
}
