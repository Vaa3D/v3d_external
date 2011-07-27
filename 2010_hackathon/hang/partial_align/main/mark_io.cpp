#include <iostream>
#include <fstream>

#include "v3d_basicdatatype.h"
#include "mark_io.h"

using namespace std;

int split(const char *paras, char ** &args, char delim)
{
	int argc = 1;
	int len = strlen(paras);
	int posb[200];
	char * myparas = new char[len];
	strcpy(myparas, paras);

	for(int i = 1; i < len; i++){if(myparas[i] == delim) argc++;}

	args = new char*[argc];
	int index = 0;
	args[index++] = myparas;

	for(int i = 0; i < len; i++)
	{
		if(myparas[i]==delim){myparas[i]='\0';args[index++] = myparas + (i+1);}
	}
	return argc;
}

list <MyImageMarker> readMarker_file(const string & filename)
{
	list <MyImageMarker> tmp_list;

	ifstream ifs(filename.c_str());
	if(ifs.fail())
	{
		cerr<<"open file ["<<filename<<"] failed!"<<endl;
		return tmp_list;
	}

	V3DLONG k=0;
	while (ifs.good())
	{
		char curline[2000];
		ifs.getline(curline, sizeof(curline));
		k++;
		{
			if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

			char ** args;
			int argc = split(curline,args,',');
			if (argc<3){for(int i = 0; i < argc; i++) delete [] args[i];   continue;}

			MyImageMarker S;

			S.x = atof(args[0]);
			S.y = atof(args[1]);
			S.z = atof(args[2]);
			S.radius = (argc>=4) ? atoi(args[3]) : 0;
			S.shape = (argc>=5) ? atoi(args[4]) : 1;

			tmp_list.push_back(S);
			for(int i = 0; i < argc; i++) delete [] args[i];
		}
	}

	return tmp_list;
	ifs.close();
}

bool writeMarker_file(const string & filename, const list <MyImageMarker> & listMarker)
{
	ofstream ofs(filename.c_str());
	if(ofs.fail())
	{
		cerr<<"open file ["<<filename<<"] failed!"<<endl;
		return false;
	}
	ofs<<"#x, y, z, radius, shape"<<endl;
	list<MyImageMarker>::const_iterator it = listMarker.begin();
	while(it != listMarker.end())
	{
		MyImageMarker S = *it;
		ofs<<S.x<<","<<S.y<<","<<S.z<<","<<S.radius<<","<<S.shape<<endl;
		it++;
	}
	ofs.close();
}

