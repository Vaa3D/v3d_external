/* main.cpp
 * 2010-02-08: the program is created by Hang Xiao
 */

// 

#include <iostream>
#include <string>

#include "stackutil.h"
#include "mark_detector.h"

using namespace std;

int main(int argc, char* argv[])
{
	if(argc == 1)
	{
		cout<<"Usage : partial_align [option] input_img marker/feature"<<endl;
		cout<<""<<endl;
		cout<<"-mark-detector     in_img out_marker"<<endl;
		cout<<"-calc-feature      in_img [in_marker] out_feature "<<endl;
		cout<<"-feature-comprare  img1 feature1 img2 feature2"<<endl;
		cout<<""<<endl;
		return 0;
	}
	if(string(argv[1]) == "-mark-detector")
	{
		if(argc!=4){cout<<"usage: parlia_align -mark-detector in_img out_marker"<<endl; return 0;}

		unsigned char * data1d = 0;
		V3DLONG *sz = 0;
		int datatype;
		loadImage(argv[2], data1d, sz, datatype);

		list<MyImageMarker> markerlist;
		detect_mark(markerlist, data1d, sz);
		writeMarker_file(argv[3], markerlist);
		if(data1d){delete [] data1d; data1d = 0;}
	}
	if(string(argv[2]) == "-calc-feature")
	{
	}
}
