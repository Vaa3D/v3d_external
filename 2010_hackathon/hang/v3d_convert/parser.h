#ifndef __PARSER_H_H_
#define __PARSER_H_H_

#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;
struct SupportedCommand
{
	string para_name;
	bool is_need_option;
};
/*
static int supported_para_num = 6;
static SupportedCommand supported_commands[] = {{"-rotatez", 1}, {"-channel", 1}, {"-gaussian-blur", 1}, {"-resize", 1}, {"-crop", 1}, {"-negate", 0}};

static bool is_support(string para_name)
{
	for(int i = 0; i < supported_para_num; i++)
	{
		if(supported_commands[i].para_name == para_name) return true;
	}
	return false;
}

static bool is_need_option(string para_name)
{
	if(!is_support(para_name)) return false;
	for(int i = 0; i < supported_para_num; i++)
	{
		if(supported_commands[i].para_name == para_name) return supported_commands[i].is_need_option;
	}
}
*/

struct SinglePara
{
	string para_name;          // -resize
	string para_string;
	SinglePara(){para_name=""; para_string=""; }
	SinglePara(string name, string str){para_name=name; para_string=str; }
};

struct InputParas
{
	int cur_par_id;
	vector<string> filelist;
	vector<SinglePara> paras;
	vector<SupportedCommand> supported_commands;

	InputParas(){cur_par_id = -1;}
	InputParas(SupportedCommand * cmds, int n){cur_par_id = -1;set_supported_cmd(cmds,n);}
	void set_supported_cmd(SupportedCommand * cmds, int n)
	{
		supported_commands.clear();
		if(n <= 0) return;
		for(int i = 0; i < n; i++) supported_commands.push_back(cmds[i]);
	}

	bool is_support(string para_name)
	{
		if(supported_commands.empty()) return false;
		int cmd_num = supported_commands.size();
		for(int i = 0; i < cmd_num; i++)
		{
			if(supported_commands[i].para_name == para_name) return true;
		}
		return false;
	}

	bool is_need_option(string para_name)
	{
		if(!is_support(para_name)) return false;
		int cmd_num = supported_commands.size();
		for(int i = 0; i < cmd_num; i++)
		{
			if(supported_commands[i].para_name == para_name) return supported_commands[i].is_need_option;
		}
	}

	int get_order(string para_name)
	{
		if(paras.empty()) return -1;
		for(int i = 0; i < paras.size(); i++)
		{
			if(paras[i].para_name == para_name) return i;
		}
		return -1;
	}
	bool is_empty()
	{
		return paras.empty();
	}
	bool is_exist(string para_name)
	{
		if(get_order(para_name) != -1) return true;
		return false;
	}
	bool get_next_cmd(string & next_cmd)
	{
		if(paras.empty()) return false;
		if(cur_par_id < -1) cur_par_id = -1;
		if(cur_par_id >= (int) paras.size() - 1 ){next_cmd="error"; return false;}
		cur_par_id++; 
		next_cmd = paras[cur_par_id].para_name;
		return true;
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
		else if(paras.is_support(argv[i]) && !paras.is_exist(argv[i]))
		{
			if(!paras.is_need_option(argv[i])) paras.add_para(argv[i]);
			if(paras.is_need_option(argv[i]) && i+1 < argc) {paras.add_para(argv[i], argv[i+1]); i++;}
			else {s_error += "need parameter for "; s_error += argv[i]; return false;}
		}
		else if(!paras.is_support(argv[i]))
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
