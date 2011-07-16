#include <iostream>
#include <string>
#include <vector>

#include "stackutil.h"
//#include "v3d_funcs.h"
#include "parser.h"
#include "gaussian_blur.cpp"

using namespace std;


void printHelp();
void printVersion();
bool run_with_paras(InputParas paras, string &s_error);

int main(int argc, char* argv[])
{
	if(argc == 1 || (argc == 2 && (string(argv[1]) == "-h"))) {printHelp(); return 0;}
	if(argc == 2 && (string(argv[1]) == "-v" || string(argv[1]) == "--version")) {printVersion(); return 0;}

	InputParas paras;
	string s_error("");

	if(! parse_paras(argc, argv, paras, s_error)){cout<<"Invalid paras : "<< s_error<<endl; return 0;}
	if(! run_with_paras(paras, s_error)) { cout<<"Run error : "<< s_error <<endl; return false;}
	return 0;
}

bool run_with_paras(InputParas paras, string & s_error)
{
	s_error = "";
	string unexecutable_paras[] ={"-channel"};
	paras.set_unexecutable_paras(unexecutable_paras, 1);

	while(paras.is_exist_unexecuted_para())
	{
		if(paras.is_exist("-rotatez") && ! paras.is_executed("-rotatez"))
		{
			if(paras.filelist.size() != 2) { s_error += "more/less than 2 images"; return false;}

			int channel = 0;  
			if(paras.is_exist("-channel")) if(!paras.get_int_para(channel, "-channel", s_error)) return false;

			//string infile = paras.filelist.at(0);
			paras.set_executed("-rotatez");
		}
		else if(paras.is_exist("-gaussian-blur") && !paras.is_executed("-gaussian-blur"))
		{
			if(paras.filelist.size() != 2) { s_error += "more/less than 2 images"; return false;}

			int channel = 0;  if(paras.is_exist("-channel")) if(!paras.get_int_para(channel, "-channel", s_error)) return false;

			double sigma; int radius;  
			if(!paras.get_double_para(sigma,"-gaussian-blur",0,s_error,"x") || !paras.get_int_para(radius, "-gaussian-blur",1, s_error,"x")) return false;
			cout<<"sigma = "<<sigma<<" radius = "<<radius<<endl;

			string infile = paras.filelist.at(0);
			string outfile = paras.filelist.at(1);

			unsigned char * indata1d = 0, * outdata1d = 0;
			V3DLONG * sz = 0;
			int datatype;

			loadImage((char *)infile.c_str(), indata1d, sz, datatype);

			if(channel >= sz[3]){ s_error += "channel exceed the input image's total channel num"; return false;}

			indata1d = indata1d + sz[0] * sz[1] * sz[2] * channel;
			if(!compute_gaussian_blur(outdata1d, indata1d, sz, sigma, radius)){ s_error += "failed to compute gaussian-blur"; return false;}

			sz[3] = 1;
			saveImage((char *)outfile.c_str(), outdata1d, sz, datatype);
		}
		else
		{
			if(paras.filelist.size() == 2)
			{
				string infile = paras.filelist.at(0);
				string outfile = paras.filelist.at(1);
				if(infile == outfile) return true;

				unsigned char * data1d = NULL;
				V3DLONG *sz = NULL;
				int datatype;
				loadImage((char*) infile.c_str(), data1d, sz, datatype);
				saveImage((char*) outfile.c_str(), data1d, sz, datatype);
			}
			else { s_error += "need output image"; return false;}
		}
	}

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
}

void printVersion()
{
	cout<<"Version : 1.0"<<endl;
}
