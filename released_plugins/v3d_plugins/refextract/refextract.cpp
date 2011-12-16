/* refextract.CPP
 * created by Yang Yu, Dec 16, 2011
 */


// 

#ifndef __REFEXTRACT_SRC_CPP__
#define __REFEXTRACT_SRC_CPP__

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "refextract.h"

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

#include <iostream>
using namespace std;

#include <string.h>

#define INF 1e10


// free 1d pointer
template<class Tdata>
void freeMemory(Tdata *&p)
{
    if(p) {delete[] p; p=NULL;}
}

// extract reference channel and convert it to 8bit data
template<class Tdata, class Tidx>
bool extconv(Tdata *p, Tidx sx, Tidx sy, Tidx sz, unsigned char *&pOutput)
{
    //
    if(p==NULL) 
    {
        printf("\nError: inputs are invalid!\n");
        return false;
    }
    
    // 
    Tidx pagesz = sx*sy*sz;
    
    //
    freeMemory<unsigned char>(pOutput);
    try
    {
        pOutput = new unsigned char [pagesz];
        memset(pOutput, 0, sizeof(unsigned char)*pagesz); // init
    }
    catch(...)
    {
        printf("Fail to allocate memory!\n");
        freeMemory<unsigned char>(pOutput);
        return false;
    }
    
    //
    double max_v=-INF; 
    double min_v=INF;
    
    for(Tidx i=0; i<pagesz; i++)
    {
        Tdata val = p[i];
        
        if(max_v<val) max_v=val;
        if(min_v>val) min_v=val;
    }
    
    //
    max_v -= min_v;
    if(max_v==0)
    {
        printf("\nError: uniform data are not supported!\n");
        return false;
    }
    
    for(Tidx i=0; i<pagesz; i++)
    {
        pOutput[i] = 255*(p[i]-min_v)/max_v;
    }
    
    //
    return true;
}

//plugin interface
const QString title = "RefExtract";

Q_EXPORT_PLUGIN2(refExtract, RefExtractPlugin);

QStringList RefExtractPlugin::menulist() const
{
    return QStringList() << tr("RefExtract")
                         << tr("About");
}

void RefExtractPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("RefExtract"))
    {
    	//
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Reference Extraction Plugin %1 (Dec. 16, 2011) developed by Yang Yu. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()).append("\n"));
        return;
    }
}

void errorPrint()
{
    printf("\nUsage: v3d -x refExtract.dylib -f refExtract -i <input_image> -o <output_image> -p \"#c <refchannel> \"\n");
}

// plugin func
QStringList RefExtractPlugin::funclist() const
{
    return QStringList() << tr("refExtract");
}

bool RefExtractPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("refExtract"))
    {
        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            errorPrint();
            return true;
        }
        
        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        
        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file
        
        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }
        
        // init
        V3DLONG channel_ref = 0;
        
        QString qs_filename_img_input(infile);
        QString qs_filename_img_output;
        
        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];
            
            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            
            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }
            
            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }
            
            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];
                    
                    qDebug()<<">>key ..."<<key;
                    
                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "c"))
                            {
                                channel_ref = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                            
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                    
                }
            }
            
            
            QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            
            if(outfile)
            {
                qs_filename_img_output=qs_filename_output;
            }
            else
            {
                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_input+"_8bit.v3draw";
            }
            
            // error check
            if(qs_filename_img_input==NULL || qs_filename_img_output==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint();
                return false;
            }
            if(channel_ref<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint();
                return false;
            }
            
        }
        
        //
        V3DLONG *sz_relative = 0;
        unsigned char* relative1d = 0;
        int datatype_tile = 0;
        if(QFile(QString(infile)).exists())
        {
            if (loadImage(const_cast<char *>(infile), relative1d, sz_relative, datatype_tile)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",infile);
                return false;
            }
        }
        else
        {
            cout<<"The input file does not exist!"<<endl;
            return false;
        }
        
        //
        V3DLONG offset_c = channel_ref*sz_relative[0]*sz_relative[1]*sz_relative[2];
        
        unsigned char *pOutput = NULL;
        if(datatype_tile == V3D_UINT8)
        {
            if(extconv<unsigned char, V3DLONG>((unsigned char *)relative1d + offset_c, sz_relative[0], sz_relative[1], sz_relative[2], pOutput)!=true);
            {
                printf("Fail to call function imgtiling! \n");
                return false;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            if(extconv<unsigned short, V3DLONG>((unsigned short *)relative1d + offset_c, sz_relative[0], sz_relative[1], sz_relative[2], pOutput)!=true);
            {
                printf("Fail to call function imgtiling! \n");
                return false;
            }

        }
        else if(datatype_tile == V3D_FLOAT32)
        {
            // current not supported
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        // save
        if(qs_filename_img_output!=NULL)
        {
            sz_relative[3]=1;
            if(!saveImage(qPrintable(qs_filename_img_output),pOutput,sz_relative,V3D_UINT8))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }
        }

        //
        return true;
    }
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
}

#endif


