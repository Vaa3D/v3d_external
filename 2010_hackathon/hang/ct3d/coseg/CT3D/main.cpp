//
//=======================================================================
// Copyright 2010 Institute PICB.
// Authors: Hang Xiao, Axel Mosig
// Data : July 11, 2010
//=======================================================================
//

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <ctime>
#include <assert.h>
#include "../component_tree.h"
#include "../myalgorithms.h"  //split
#include "cell_track.h"

using namespace std;

void quit();
void help();
void other_command(char*);

/*
 * main_1 do the work of building one component tree and veiw the final result
 * later we may build a lot of such functions
 */
int main(int argc, char *argv[])
{				
	/***********************************************
	 * Enter the loop of command line
	 ***********************************************/
	char command[1000] = "";
	char params[1000] = "";
	char* splits[20];
	int count = 0;
	
	CellTrack* ct = new CellTrack();
	//help();
	cout<<"> ";

	while(1)
	{
		if(cin.peek() == '\n')
		{
			cout<<"> ";
			cin.get();
			continue;
		}
		cin >> command;
		if(strcmp(command,"create")==0)
		{			
			if(cin.peek() !='\n')cin.getline(params,1000);
			else strcpy(params,"");
			
			count  = split(params, splits);
			ct->createTracking(count ,splits);
		}
		else if(strcmp(command,"track")==0 || strcmp(command,"tr")==0)
		{
			
			ct->tracking();
			
		}
		else if(strcmp(command,"load")==0 || strcmp(command,"l")==0)
		{			
			if(cin.peek() !='\n')cin.getline(params,1000);
			else strcpy(params,"");
			
			count = split(params, splits);
			ct->loadTracking(count ,splits);
		}
		else if(strcmp(command,"save")==0 || strcmp(command,"s")==0)
		{
			if(ct->frameCount() != 0)
			{
				if(! ct->saveTracking())
				{
					cout<<"\tsave results error ... "<<endl;
				}
			}
			else
			{
				cout<<"\tplease do creat tracking first"<<endl;
			}
		}
		else if(strcmp(command,"filter") == 0 || strcmp(command,"f")==0)
		{
			if(ct->trackCount()>=1)
			{
				if(cin.peek() == '\n')
				{
					ct->filterCells();
					cout<<"filter successfully."<<endl;
				}
				else
				{
					double threshold;
					cin>>threshold;
					ct->filterCells(threshold);
					cout<<"filter successfully."<<endl;
				}
			}
			else
			{
				cout<<"\tplease do creat tracking first"<<endl;
			}
		}
		else if(strcmp(command,"reset") == 0 || strcmp(command, "rs")==0)
		{
			if(ct->trackCount()>=1)
			{
				cout<<"resettng cells ... "<<endl;
				ct->resetCells();
			}
			else
			{
				cout<<"\tplease do creat tracking first"<<endl;
			}
		}
		else if(strcmp(command,"savegraph")==0 || strcmp(command,"sg")==0)
		{
			if(ct->trackCount()>=1)
				ct->saveGraph();
			else
			{
				cout<<"\tplease do creat tracking first"<<endl;
			}
		}
		else if(strcmp(command,"clear")==0)
		{
			delete ct;
			ct = NULL;
		}
		else if(strcmp(command,"tree")==0 || strcmp(command,"t")==0)
		{
			if(cin.peek() == '\n')
			{
				for(int i = 0; i < ct->frameCount(); i++)
				{
					cout<<" Component tree "<<i<<" : "<<endl;
					(ct->getTree(i)).printTree();
				}
			}
			else
			{
				int index;
				cin >> index;
				if(index < 1 || index > ct->frameCount())
				{
					cout<<"tree index should between 1 ~ "<<ct->frameCount()<<endl;
				}
				else
				{
					(ct->getTree(index-1)).printTree();
				}
			}
		}
		else if(strcmp(command,"AT3D")==0 || strcmp(command,"AT") == 0)
		{
			cout<<"\trunning AT3D ... "<<endl;
			system("AT3D&");
		}
		else 
		{
			other_command(command);
		}
	}		

	return 0;
}

void quit()
{
	/*
	char command[1000];
	cout<<"Do you really want to quit(Y/N) "<<endl;
	cin>>command; 
	if(strcmp(command,"Y")==0 || strcmp(command,"y")==0) exit(0);
	 */
	exit(0);
}

void help()
{
	system("clear");
	cout<<"**********************************************************************"<<endl;
	cout<<"*                          CT3D Help                                 *"<<endl;
	cout<<"*                                                                    *"<<endl;
	cout<<"*    create -i indir  -o outdir -m min -M max -s single [-v]         *"<<endl;
	cout<<"*    load   -i result -o outdir [-v]                                 *"<<endl;
	cout<<"*                                                                    *"<<endl;
	cout<<"*    args:  -i indir/result     input dir or result file             *"<<endl;
	cout<<"*           -o outdir           output dir                           *"<<endl;
	cout<<"*           -m min              min size threshold                   *"<<endl;
	cout<<"*           -M max              max size threshold                   *"<<endl;
	cout<<"*           -s                  cut single node threshold            *"<<endl;
	cout<<"*           -v                  display verbose inoformation         *"<<endl;
 	cout<<"*                                                                    *"<<endl;
	cout<<"*    track                      start tracking after create          *"<<endl;
	cout<<"*    save                       save tracking results                *"<<endl;
	cout<<"*    tree   [id]                print tree id                        *"<<endl;
	cout<<"*    filter [thresh]            filter the result by threshold       *"<<endl;
	cout<<"*    reset                      reset filtering result               *"<<endl;
	cout<<"*    help                       get help                             *"<<endl;
	cout<<"*    quit                       quit program                         *"<<endl;
	cout<<"*                                                                    *"<<endl;
	cout<<"* Enjoy to use it. Designed by Hang Xiao.                            *"<<endl; 
	cout<<"* Contact me: xiaohang@picb.ac.cn                                    *"<<endl;
	cout<<"**********************************************************************"<<endl;
}

void other_command(char* command)
{
	if(strcmp(command,"help")==0 || strcmp(command,"h")==0)
	{
		help();
	}
	else if(strcmp(command,"quit")==0 || strcmp(command,"q")==0)
	{
		quit();
	}
	else if(strcmp(command,"cd")==0)
	{
		if(cin.peek() == '\n')
		{
			chdir("");
		}
		else
		{
			cin>>command;
			chdir(command);
		}
	}
	else
	{
		if(cin.peek() == '\n')system(command);
		else 
		{
			char parameter[1000];
			cin.get(parameter,1000);
			strcat(command,parameter);
			system(command);
		}
	}
}

