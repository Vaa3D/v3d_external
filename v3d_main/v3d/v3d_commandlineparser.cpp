/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/





// 2010-11-23 by Yang Yu
// command line parameters parser

#include "v3d_commandlineparser.h"

#include <fstream>
#include <iostream>
#include <sstream>


// command line interface class
void V3D_CL_INTERFACE::copy(const V3D_CL_INTERFACE& input)
{
    open3Dviewer = input.open3Dviewer;
    openV3D = input.openV3D;
    clp_finished = input.clp_finished;
    pluginname = input.pluginname;
    pluginmethod = input.pluginmethod;
    pluginfunc = input.pluginfunc;
    hideV3D = input.hideV3D;
    pluginhelp = input.pluginhelp;

    for(int i=0; i<input.fileList.size(); i++)
    {
        fileList.push_back(input.fileList.at(i));
    }
    for(int i=0; i<input.cmdArgList.size(); i++)
    {
        cmdArgList.push_back(input.cmdArgList.at(i));
    }
    for(int i=0; i<input.outputList.size(); i++)
    {
        outputList.push_back(input.outputList.at(i));
    }
}


// command line parser class
int CLP::error( void (*help)() )
{
    v3d_msg("Your input is illegal. Please follow the instruction of the help page below.", 0);
    help();
    return false;
}

// check if the file is valid
bool CLP::check_filename(QString fn)
{
    qDebug()<<"file name ... ["<<fn << "]";

    QFileInfo curfile_info(fn);

    if(curfile_info.isDir() ||
          curfile_info.suffix().isEmpty())  // added for support some files without an extension and may not even exist. added by PHC 2013-08-21  )
    {
        return true;
    }
    else if(curfile_info.isFile())
    {
        if ( (curfile_info.suffix().toUpper()=="ANO") ||
             (curfile_info.suffix().toUpper()=="APO" ||
              curfile_info.suffix().toUpper()=="SWC" ||
              (curfile_info.suffix().toUpper()=="ESWC") || //enhanced SWC, by PHC, 20120217
              curfile_info.suffix().toUpper()=="OBJ" ||
              curfile_info.suffix().toUpper()=="V3DS") ||
             (curfile_info.suffix().toUpper()=="ATLAS") ||
             (curfile_info.suffix().toUpper()=="ZIP") ||
             (curfile_info.suffix().toUpper()=="LSM") ||
             (curfile_info.suffix().toUpper()=="TIF")  ||
             (curfile_info.suffix().toUpper()=="TIFF") ||
             (curfile_info.suffix().toUpper()=="MRC") ||
             (curfile_info.suffix().toUpper()=="RAW") ||
             (curfile_info.suffix().toUpper()=="RAW5") ||
             (curfile_info.suffix().toUpper()=="V3DRAW") ||
             (curfile_info.suffix().toUpper()=="V3DPBD") ||
             (curfile_info.suffix().toUpper()=="VAA3DPBD") ||
             (curfile_info.suffix().toUpper()=="IMG") ||
             (curfile_info.suffix().toUpper()=="HDR") ||
             (curfile_info.suffix().toUpper()=="NII") ||
             (curfile_info.suffix().toUpper()=="MARKER") || // added for reading marker file in -i by ZJL, 2012-05-10
             (curfile_info.suffix().toUpper()=="TXT") ||    // added for reading txt file in -i by ZJL, 2012-05-10
             (curfile_info.suffix().toUpper()=="DOMAIN") || // added for reading domain file in -i by ZJL, 2012-05-10
             fn.contains("://") ) // url
        {
            return true;
        }
        else
        {
            v3d_msg(QString("error: your file [%1] has an extension Vaa3D does not recognize.").arg(fn), 0);
            return false;
        }
    }

    return false;

}

// parsing parameters
int CLP::parse(int argc, char *argv[], void (*help)())
{
    //
    if (argc<=1)
    {
        i_v3d.openV3D = true;
    }
    else
    {
        vector<char *> parList; // read from configuration file

        // command arguments parsing
        char* key;

        // ------ parsing aguements here ---------------------
        if(argc<=2)
        {
            key = argv[1];
            if (*key == OPTION_CHAR)
            {
                while(*++key)
                {
                    if (*key == '?' || !strcmp(key, "h") || !strcmp(key, "H"))
                    {
                        help();
                        i_v3d.clp_finished = true;
                    }
                    // CMB Dec 7, 2010
                    // Mac app launcher adds a command line argument
                    // like "-psn_0_7989150"
                    // Ignore it.
                    else if (string(argv[1]).find("-psn_") == 0) {
                        v3d_msg("Apparently a mac bundle", 0);
                        v3d_msg(argv[1], 0);
                        i_v3d.openV3D = true;
                        return true;
                    }
                    else if(!strcmp(key, "M")) //must be capital
                    {
                        i_v3d.clp_finished = true;
                        return true;
                    }
                    else if(!strcmp(key, "na"))
                    {
                        key++; // skip "na"

                        i_v3d.openV3D = true;
                        i_v3d.openNeuronAnnotator = true;

                        break;
                    }
                }

            }
            else if( check_filename(QString(key)) )
            {
                // load and visualize file in V3D
                char *filename = argv[1];
                i_v3d.fileList.push_back(filename);

                // open V3D
                i_v3d.openV3D = true;

            }
            else
            {
                i_v3d.clp_finished = true;
                return error(help);
            }
        }
        else
        {
            for (int i=1;i<argc;i++) {
                if (string(argv[i])=="-cmd") {
                    i_v3d.openV3D = false;
                    while (i+1<argc) {
                        i_v3d.cmdArgList.push_back(argv[++i]);
                    }

#ifdef _ALLOW_WORKMODE_MENU_
                    CommandManager commandManager(&(i_v3d.cmdArgList));
                    commandManager.execute();
#endif

                    i_v3d.clp_finished=true;
                    return true;
                }
            }

            // find -h and -x combination
            int flagh = 0, flagx = 0;
            for(int i=1; i<argc; i++)
            {
                key = argv[i];
                if (*key == OPTION_CHAR)
                {
                    while(*++key)
                    {
                        if (*key == '?' || !strcmp(key, "h") || !strcmp(key, "H"))
                        {
                            flagh++;
                        }
                    }
                }
                key = argv[i];
                if(*key == OPTION_CHAR)
                {
                    while (*++key)
                    {
                        if(!strcmp(key, "x"))
                        {
                            flagx++;

                            if(i+1>=argc)
                                return error(help);

                            // launch V3D
                            i_v3d.openV3D = true;

                            // plugin command
                            i_v3d.pluginname = argv[i+1];
                        }
                    }
                }

                if(flagh && flagx)
                {
                    i_v3d.hideV3D = true; // do not open v3d GUI
                    i_v3d.pluginhelp = true;
                    return true;
                }
            }

            for(int i=1; i<argc; i++)
            {
                key = argv[i];
                if (*key == OPTION_CHAR)
                {
                    while(*++key)
                    {
                        if (!strcmp(key, "v"))
                        {
                            i_v3d.open3Dviewer = true;
                        }
                    }
                }
            }

            // parsing arguments in other cases
            for(int i=1; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {

                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == OPTION_CHAR)
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "i"))
                            {
                                // open V3D
                                i_v3d.openV3D = true;

                                while(i+1<argc && !QString(argv[i+1]).startsWith(OPTION_CHAR) )
                                {
                                    char *filename = argv[i+1];

                                    if( check_filename(QString(filename)) ) // do not check name in order to use any extension file in -i?? Jianlong Zhou 2012-05-04
                                    {
                                        i_v3d.fileList.push_back(filename);
                                        i++;
                                    }
                                    else
                                    {
                                        cout << "The file format is not supported for Vaa3D -i option."<<endl;
                                        return false;
                                    }
                                }
                            }
                            else if (!strcmp(key, "o"))
                            {
                                while(i+1<argc && !QString(argv[i+1]).startsWith(OPTION_CHAR) )
                                {
                                    char *filename = argv[i+1]; // allow all kinds of file format

                                    i_v3d.outputList.push_back(filename);
                                    i++;
                                }
                            }
                            else if (!strcmp(key, "v"))
                            {
                                i_v3d.open3Dviewer = true;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                // launch V3D
                                i_v3d.openV3D = true;

                                // plugin command
                                i_v3d.pluginname = argv[i+1];
                                i++;

                                qDebug()<<"call plugin ..."<<i_v3d.pluginname;
                            }
                            else if (!strcmp(key, "m"))
                            {
                                // plugin method
                                i_v3d.pluginmethod = argv[i+1];
                                i++;

                                qDebug()<<"call plugin method ..."<<i_v3d.pluginmethod;
                            }
                            else if (!strcmp(key, "f"))
                            {
                                // plugin function
                                i_v3d.pluginfunc = argv[i+1];
                                i++;

                                i_v3d.hideV3D = true; // do not open v3d GUI

                                qDebug()<<"call plugin function ..."<<i_v3d.pluginfunc;
                            }
                            else if(!strcmp(key, "p"))
                            {
                                // plugin function parameters
                                while(i+1<argc && !QString(argv[i+1]).startsWith(OPTION_CHAR) )
                                {
                                    char *strparameters = argv[i+1];

                                    i_v3d.cmdArgList.push_back(strparameters);
                                    i++;
                                }
                            }
                            else if(!strcmp(key, "pf"))
                            {
                                key++; // skip "pf"

                                // plugin function parameters from configuration file
                                if(i+1<argc)
                                {
                                    char *fn = argv[i+1];
                                    ifstream pConfigFile(fn);

                                    string str;

                                    if(pConfigFile.is_open())
                                    {

                                        while( !pConfigFile.eof() )
                                        {
                                            while( getline(pConfigFile, str) )
                                            {
                                                //istringstream iss(str);
                                                parList.push_back((char *)(str.c_str()));

                                            }
                                        }
                                    }
                                    else
                                    {
                                        cout << "Unable to open the file"<<endl;
                                        return false;
                                    }

                                    pConfigFile.close();

                                }
                                i++;

                            }
                            else
                            {
                                qDebug()<<"parsing ..."<<key<<i<<"Unknown command. Type 'vaa3d -h' for usage";

                                i_v3d.clp_finished = true;
                                return error(help);
                            }

                        }
                    }
                    else
                    {
                        return error(help);
                    }

                }

            }

            //
            for(int i=0; i<parList.size(); i++)
            {
                i_v3d.cmdArgList.push_back(parList.at(i));
            }

        }
    }

    return true;
}

