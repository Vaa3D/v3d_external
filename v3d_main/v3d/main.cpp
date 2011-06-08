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




/****************************************************************************
**
V3D main program

By Hanchuan Peng

July 21, 2006
060924: v3d v1.2

Last update: 2008-04-25: try to add command line based utilities
Last update: 2010-04-12: add a global try-catch to catch all exceptions
Last update: 2010-11-19: change some of the help info 
Last update: 2011-04-19: fix some potential problem of null mainWin pointer 
 
****************************************************************************/

#define COMPILE_TO_COMMANDLINE 1

#include "v3d_compile_constraints.h"

#include <QApplication>
#include <QFile>

#include <iostream>
#include <vector>

#include "mainwindow.h"
#include "../neuron_annotator/NaMainWindow.h"
#include "v3d_application.h"

#include <string>
using namespace std;

#include "v3d_core.h"
//#include "idrawmain.h"

#include "v3d_commandlineparser.h"
#include "v3d_version_info.h"
#include "../plugin_loader/v3d_plugin_loader.h"

void printHelp_v3d();
void printHelp_align();
void printHelp_straight();
void printHelp_trace();

V3dApplication* V3dApplication::theApp;

void printHelp_v3d()
{
	//printf("\nV3D: a 3D image visualization and processing tool developed by Hanchuan Peng and colleagues.\n");
	//printf("\nUsage: v3d -h -M moduleCode [all other options specific to different modules]\n");
	//printf("\t -h/H         help information. \n");
	//printf("\t -M module    a string indicate which module will be used for processing. \n");
	//printf("\t Available/future modules/codes (no difference for capital): \n");
	//printf("\t   ALIGN      3D image elastic registration for objects like brains.\n");
	//printf("\t   BLEND      3D image blending.\n");
	//printf("\t   CROP       3D image cropping.\n");
	//printf("\t   LANDMARK   operation on the landmark files.\n");
	//printf("\t   MASK       3D image masking.\n");
	//printf("\t   RESAMPLE   3D image resampling (reslicing).\n");
	//printf("\t   ROT        3D image rotation.\n");
	//printf("\t   SAVEAS     3D image file format conversion.\n");
	//printf("\t   SEG        3D image segmentation for spherical objects (e.g. cells, nuclei).\n");
	//printf("\t   STITCH     3D image stitching.\n");
	//printf("\t   STRAIGHT   3D image straightening (e.g. for C. elegans worm body).\n");
	//printf("\t   TRACE      3D image tracing (e.g. for neurons).\n");

	//printf("\n");
	//printHelp_align();

	//printf("\n");
	//printHelp_straight();

	//printf("\n");
	//printHelp_trace();

	cout<<endl<<"V3D: a 3D image visualization and analysis platform developed by Hanchuan Peng and colleagues."<<endl;
	cout<<endl<<"Usage: v3d -h -M moduleCode [all other options specific to different modules]"<<endl;
	
	cout<<"    -h/H         help information."<<endl;

	cout<<"    -f <file>                  open single or multiple image (.tif/.tiff, .lsm, .mrc, .raw/.v3draw) / object (.ano, .apo, .swc, .marker) files"<<endl;
	cout<<"    -p plugin_dll_full_path    a string indicates the full path of a dll (for a plugin) to be launched."<<endl;
	cout<<"    -m menu_name               a string indicates which menu of a plugin will be called."<<endl;
	
	cout<<"    -v		    force to open a 3d viewer when loading an image, otherwise use the default v3d global setting (from \"Adjust Preference\")"<<endl;
        cout<<"    -cmd  [headless command-line arguments, intended for compute grid use. Try \'-cmd -h\' for more information on this option]"<<endl;

	return;
}

void printHelp_align()
{
	printf("\nUsage: v3d -M ALIGN -t <rawImgFile_template/target> -s <rawImgFile_subject> -o <rawImgFile_warped_subject> -c <channalNo_reference> -w <warp_type> -n <nloop>\n");
	printf("\t -t <rawImgFile_template/target>    .raw/.tif file containing 3D template/target stack to which a subject image will be warped/mapped. \n");
	printf("\t -s <rawImgFile_subject>            .raw/.tif file containing 3D subject which will deform/warp to best appriximate the geometry of the template/target. \n");
	printf("\t -o <rawImgFile_warped_subject>     .raw/.tif file containing warped result stack.\n");
	printf("\t -F <rawImgFile_DF_file>            .raw file (4-byte float) containing the displacement field (DF), chanel 0, 1, 2 for sx, sy, ans sz in terms of pixels.\n");
	printf("\t                                    Note: specify -t will prohibit -F.\n");
	printf("\t -L <landmark_file_target>          a plain text csv file containing landmarks of only the target_pos (if -w 9).\n");
	printf("\t -l <landmark_file_subject>         a plain text csv file containing landmarks of only the subject_pos (if -w 9). If un-specified, then assume -w 6\n");
	printf("\t [-C]                               the ID of the reference channel of the target image (default 0) which is used in computing warping. \n");
	printf("\t [-c]                               the ID of the reference channel of the subject image (default 0) which is used in computing warping. All other channels will use the same warp.\n");
	printf("\t [-w]                               the type of warp, 0 -- global, 1 -- local (without global), and 2 -- local (with global), 6-target landmark in file. 9 - landmark pairs in file. \n");
	printf("\t [-n]                               number of loops of global/local estimation. \n");
	printf("\t [-m]                               number of loops of local smoothing. \n");
	printf("\t [-z]                               minimal xy size in the pyrimid (e.g. 64 or 32). \n");
	printf("\t [-r]                               the half window size in defining the local block (e.g. 5 means a 11x11 block, and 3 means a 7x7 block). \n");
	printf("\t [-X]                               target image: X/Y direction pixel size (resolution) in microns. Also assume the X and Y pixel size are always the same. \n");
	printf("\t [-Z]                               target image: Z direction resolution in microns. \n");
	printf("\t [-U]                               subject image: X/Y direction pixel size (resolution) in microns. Also assume the X and Y pixel size are always the same. \n");
	printf("\t [-V]                               subject image: Z direction resolution in microns. \n");
	printf("\t [-B]                               XY plane size of image mapping (default is 512, can be 1024, 2048, or 256, - note: if use 1024 and above, should use the cluster node with big memory). \n");
	printf("\t [-e]                               multiplication factor for control points in an existing landmark file (e.g. if image size is 1024, and existing landmarks are defined using 512 image, then use -e 2. Default is 1.). \n");
	printf("\t \n");    
	printf("\t [-T]                               only convert the file format of the -s file to the -o file (if they have the same suffixes, then just a save as). \n");
	printf("\t \n");
	printf("\t [-v]                               verbose printing enabled. \n");
	printf("\t [-h]                               print this message.\n");
	return;
}

void printHelp_straight()
{
}

void printHelp_trace()
{
}

int main(int argc, char **argv)
{

#ifdef COMPILE_TO_COMMANDLINE
	//string s1, s2;
	//s1 = argv[1];
	//if (s1=="-h" || s1=="-H")
	//{
	//	if (argc<=2)
	//	{
	//		printHelp_v3d();
	//		return 0;
	//	}
	//	else
	//	{
	//		s2 = argv[2];
	//		if (s2=="ALIGN") {return 0;}
	//		else if (s2=="BLEND") {return 0;}
	//		else if (s2=="CROP") {return 0;}
	//		else if (s2=="LANDMARK") {return 0;}
	//		else if (s2=="MASK") {return 0;}
	//		else if (s2=="RESAMPLE") {return 0;}
	//		else if (s2=="ROT") {return 0;}
	//		else if (s2=="SAVEAS") {return 0;}
	//		else if (s2=="SEG") {return 0;}
	//		else if (s2=="STITCH") {return 0;}
	//		else if (s2=="STRAIGHT") {return 0;}
	//		else if (s2=="TRACE") {return 0;}
	//		else
	//		{
	//			printf("Your module code is illegal. Please follow the instruction of the help page below.\n\n");
	//			printHelp_v3d();
	//			return 1;
	//		}
	//	}
	//}
	//else if (s1=="-M") //must be capital
	//{
	//	return 0;
	//}
	//else
	//{
	//	printf("Your command line input is illegal. Please follow the instruction of the help page below.\n\n");
	//	printHelp_v3d();
	//	return 1;
	//}

	CLP parser;

	parser.parse(argc, argv, printHelp_v3d); // parse command lines to v3d_cl_interface Nov. 23, 2010 by YuY

	if(parser.i_v3d.clp_finished)
	{
		return false;
	}
	else
	{        
		if(parser.i_v3d.openV3D)
		{
			// ------ V3D GUI handling module ---------------------
			Q_INIT_RESOURCE(v3d);

                        V3dApplication* app = V3dApplication::getInstance(argc, argv);
                        app->activateMainWindow();
                        MainWindow* mainWin=app->getMainWindow();

			if (!mainWin)
			{
				v3d_msg("Unable to open the V3D main window. Quit.");
				return false;
			}
			
                        app->installEventFilter(mainWin);
			
            if (mainWin) 
            {
                mainWin->v3dclp.copy(parser.i_v3d);
				mainWin->show();
            }
			
			// plugin module
			if(parser.i_v3d.pluginname)
			{
				mainWin->setBooleanCLplugin(true);
				mainWin->setPluginName(parser.i_v3d.pluginname);
				mainWin->setPluginMethod(parser.i_v3d.pluginmethod);
				mainWin->setPluginFunc(parser.i_v3d.pluginfunc);
			}
			
			// multiple image/object handling module
			if(parser.i_v3d.fileList.size()==0)
			{
				if(parser.i_v3d.pluginname)
				{
					mainWin->triggerRunPlugin();
				}
			}
			else
			{
				for(int i=0; i<parser.i_v3d.fileList.size(); i++)
				{
					char *filename = parser.i_v3d.fileList.at(i);
					
					qDebug()<<"now try open files ...";
					
					QString qFile(filename);
					
					if(!QFile(qFile).exists()) // supporting both local and web files. Nov. 18, 2010. YuY
					{
						// judge whether the file exists on the web
						// "://" like "http://" "https://" "ftp://" 
						
						if(qFile.contains("://"))
						{
							QUrl url(filename);
							
							if(!url.isValid()) // valid or invalid url
							{
								v3d_msg("The file does not exist! Do nothing.", 0);
								return -1;	
							}
							else if(url.scheme().toUpper() == "HTTP" || url.scheme().toUpper() == "HTTPS" || url.scheme().toUpper() == "FTP")
							{
								// load image/object
								mainWin->loadV3DUrl(QUrl(filename), true, parser.i_v3d.open3Dviewer);
							}
							//how about smb:// etc?? //by PHC, 20101123 question
						}
						else // impossible be a url
						{
							v3d_msg("The file does not exist! Do nothing.", 0);
							return -1;	
						}
					}
					else
					{
						QString curSuffix = QFileInfo(qFile).suffix();
						// load image/object
						mainWin->loadV3DFile(filename, true, parser.i_v3d.open3Dviewer);
					}
				}
			}
			
						

            // Check for software updates.
            // But not if V3D has been invoked with a file to open immediately.
            // Like the logic Fiji uses: http://pacific.mpi-cbg.de/wiki/index.php/Update_Fiji
            // To help decide whether to check for V3D software updates at start up,
            // determine whether any file has been opened.
            // This works for files open on the command line, but not for dropped files on Mac
            int nchild;
            mainWin->retrieveAllMdiChild(nchild);
            // TODO - don't check for updates if Mac OpenFileEvent is pending.
            // Don't check for updates if the current user does not have write permission.
            bool userCanUpdate = false;
            QFile v3dProgramFile(argv[0]);
            if (v3dProgramFile.exists() && (v3dProgramFile.permissions() & QFile::WriteUser))
            {
                // v3d_msg("User can write to v3d executable",0);
                userCanUpdate = true;
            }
            // Does the user have permission to write to any plugin folders?
            QList<QDir> pluginsDirList = V3d_PluginLoader::getPluginsDirList();
            foreach (const QDir& pluginsDir, pluginsDirList)
            {
                QStringList nameFilter;
                nameFilter << ".";
                QFileInfoList dirInfoList = pluginsDir.entryInfoList(nameFilter);
                foreach (const QFileInfo& dirInfo, dirInfoList) {
                    if (dirInfo.permissions() & QFile::WriteUser) {
                        userCanUpdate = true;
                        // v3d_msg("User can write to plugin directory " + pluginsDir.absolutePath(), 0);
                    }
                }
            }
            if ( (nchild == 0) && userCanUpdate ) {
                // This is the automatic check for latest version
                // v3d_msg("Starting V3D version checker...",0);
                v3d::V3DVersionChecker *versionChecker = new v3d::V3DVersionChecker(mainWin);
                if (versionChecker->shouldCheckNow()) {
                    // v3d_msg("It is time to check for software updates...",0);
                    // versionChecker->checkForLatestVersion(false);
                }
            }

			// launch v3d
			try 
			{
                                return app->exec();
			}
			catch (...) 
			{
				v3d_msg("Catch an exception at the main application level. Basically you should never see this. Please click Ok to quit and send the error log to the V3D developers to figure out the problem.");
				return 1;
			}
			// -------------------------------------------------------
		
		}
		else
		{
			return true;
		}
		
	}

#else
	Q_INIT_RESOURCE(v3d);

	QApplication app(argc, argv);

	//090812 RZC: best solution for QMainWindow is using new, then deleteLater itself in its closeEvent.
	MainWindow* mainWin = new MainWindow;
	if (!mainWin)
	{
		v3d_msg("Unable to open the V3D main window. Quit.");
		return false;
	}
	
    // On Mac, allow mainWin to get QFileOpen events, such as when a tif
    // file is dragged onto the application icon.
    // CMB Nov-12-2010
    app.installEventFilter(mainWin);

	mainWin->show();

	//*************************
	// DO NOT USE THE FOLLOWING AS I CHANGED TO MDI APPLICATIONS
	//*************************

	//XFormWidget xformWidget(0);
	//XFormWidget xformWidget;
	//XFormWidget xformWidget1(0); //useful to display multiple stacks

	/*
	//set style

	QStyle *arthurStyle = new ArthurStyle();
	xformWidget.setStyle(arthurStyle);

	QList<QWidget *> widgets = qFindChildren<QWidget *>(&xformWidget);
	foreach (QWidget *w, widgets)
		w->setStyle(arthurStyle);
	*/

	//display

	//xformWidget.show();
	//xformWidget1.show(); //useful to display multiple stacks

	//iDrawMainWindow my3ddraw;
	//my3ddraw.show();

	try 
	{
		return app.exec();
	}
	catch (...) {
		v3d_msg("Catch an exception at the main application level. Basically you should never see this. Please click Ok to quit and send the error log to the V3D developers to figure out the problem.");
		return 1;
	}

#endif

}
