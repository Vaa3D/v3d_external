#ifndef __PARSER_H_H_
#define __PARSER_H_H_

#include <string>
#include <cstdlib>

using namespace std;
struct SupportedPara
{
	string para_name;
	bool is_need_option;
};

static int supported_para_num = 6;
static SupportedPara supported_parameters[] = {{"-rotatez", 1}, {"-channel", 1}, {"-gaussian-blur", 1}, {"-resize", 1}, {"-crop", 1}, {"-negate", 0}};

static bool is_support(string para_name)
{
	for(int i = 0; i < supported_para_num; i++)
	{
		if(supported_parameters[i].para_name == para_name) return true;
	}
	return false;
}

static bool is_need_option(string para_name)
{
	if(!is_support(para_name)) return false;
	for(int i = 0; i < supported_para_num; i++)
	{
		if(supported_parameters[i].para_name == para_name) return supported_parameters[i].is_need_option;
	}
}

struct SinglePara
{
	string para_name;          // -resize
	string para_string;
	bool is_executed;
	SinglePara(){para_name=""; para_string=""; is_executed = 0;}
	SinglePara(string name, string str){para_name=name; para_string=str; is_executed = 0;}
};

struct InputParas
{
	vector<string> filelist;
	vector<SinglePara> paras;
	int get_order(string para_name)
	{
		if(paras.empty()) return -1;
		for(int i = 0; i < paras.size(); i++)
		{
			if(paras[i].para_name == para_name) return i;
		}
		return -1;
	}
	bool is_exist_unexecuted_para()
	{
		if(paras.empty()) return false;
		for(int i = 0; i < paras.size(); i++)
		{
			if(paras[i].is_executed == false) return true;
		}
		return false;
	}
	void set_unexecutable_paras(string para_list[], int n)
	{
		if(n <= 0) return;
		for(int i = 0; i < n; i++)
		{
			string para_name = para_list[i];
			int order = get_order(para_name);
			if(order != -1) paras[order].is_executed = 1;
		}
	}
	bool is_exist(string para_name)
	{
		if(get_order(para_name) != -1) return true;
		return false;
	}
	bool is_executed(string para_name)
	{
		int order = get_order(para_name);
		if(order == -1) return false;
		else return paras[order].is_executed;
	}
	void set_executed(string para_name)
	{
		int order = get_order(para_name);
		if(order == -1) return ;
		else paras[order].is_executed = true;
	}
	string get_para(string para_name)
	{
		int order = get_order(para_name);
		if(order == -1) return string("");
		return paras[order].para_string;
	}
	template <class T> bool get_int_para(T &v, string para_name, string &s_error)
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return false;}
		v = (T) (atol(get_para(para_name).c_str()));
		return true;
	}
	template <class T> bool get_int_para(T &v, string para_name, int index, string &s_error, string sep = "x")
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return false;}
		string par_str = get_para(para_name);
		size_t sp=0, ep=par_str.find_first_of(sep);
		if(index == 0)
		{
			if(ep == string::npos) {v = (T) (atol(par_str.c_str())); return true;}
			else if(ep == 0){v = 0; return true;}
			else {v = (T)(atol(par_str.substr(sp, ep - sp).c_str())); return true;}
		}
		while(index>0)
		{
			if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
			else {s_error += "index exceed the number of "; s_error += sep; return false;}
			index--;
		}
		if(ep == string::npos) ep = par_str.size();
		v = (T)(atol(par_str.substr(sp, ep - sp).c_str()));
		return true;
	}
	bool get_double_para(double &v, string para_name, string &s_error)
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return false;}
		v = atof(get_para(para_name).c_str());
		return true;
	}
	bool get_double_para(double &v, string para_name, int index, string &s_error, string sep = "x")
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para"; return false;}
		string par_str = get_para(para_name);
		size_t sp=0, ep=par_str.find_first_of(sep);
		if(index == 0)
		{
			if(ep == string::npos) {v = atof(par_str.c_str()); return true;}
			else if(ep == 0){v = 0; return true;}
			else {v=atof(par_str.substr(sp, ep - sp).c_str()); return true;}
		}
		while(index>0)
		{
			if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
			else {s_error += "index exceed the number of "; s_error += sep; return false;}
			index--;
		}
		if(ep == string::npos) ep = par_str.size();
		v = atof(par_str.substr(sp, ep - sp).c_str());
		return true;
	}
	void add_para(string para_name, string para_string){
		paras.push_back(SinglePara(para_name, para_string));
	}
	void add_para(string para_name){
		paras.push_back(SinglePara(para_name, ""));
	}
	void add_para(SinglePara para){
		paras.push_back(para);
	}
};

bool parse_paras(int argc, char* argv[], InputParas &paras, string &s_error)
{
	int i = 1;      // switch ind

	while(i < argc)
	{
		SinglePara para;
		if(argv[i][0] != '-') paras.filelist.push_back(string(argv[i]));
		else if(is_support(argv[i]) && !paras.is_exist(argv[i]))
		{
			if(!is_need_option(argv[i])) paras.add_para(argv[i]);
			if(is_need_option(argv[i]) && i+1 < argc) {paras.add_para(argv[i], argv[i+1]); i++;}
			else {s_error += "need parameter for "; s_error += argv[i]; return false;}
		}
		else if(!is_support(argv[i]))
		{
			s_error += argv[i];
			s_error += " is not supported";
			return false;
		}
		else if(paras.is_exist(argv[i]))
		{
			s_error += "duplicated para ";
			s_error += argv[i];
			return false;
		}
		else
		{
			s_error += "what is this paramter";
			return false;
		}
		i++;
	}
	return true;
}


#endif
