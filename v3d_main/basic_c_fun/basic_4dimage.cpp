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




/*
 * basic_4dimage.cpp
 * last update: 100819: Hanchuan Peng. use MYLIB only for Llinux and Mac, but not WIN32. FIXME: add VC support later.
 * 20120410: add curFileSurfix check for the potential strcmp crashing. by Hanchuan Peng
 */

#include "v3d_message.h"

#include "stackutil.h"
#include "basic_4dimage.h"

#ifdef _ALLOW_WORKMODE_MENU_
#include "../neuron_annotator/utility/ImageLoader.h"
#endif

typedef unsigned short int USHORTINT16;


void Image4DSimple::loadImage(char filename[])
{
	return this->loadImage(filename, false); //default don't use MYLib
}

void Image4DSimple::loadImage(char filename[], bool b_useMyLib)
{
	cleanExistData(); // note that this variable must be initialized as NULL. 

	strcpy(imgSrcFile, filename);

	V3DLONG * tmp_sz = 0; // note that this variable must be initialized as NULL. 
	int tmp_datatype = 0;
	int pixelnbits=1; //100817

	//060815, 060924, 070805
	char * curFileSurfix = getSurfix(imgSrcFile);
	printf("The current input file has the surfix [%s]\n", curFileSurfix);

	if (curFileSurfix && (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0 ||
		strcasecmp(curFileSurfix, "lsm")==0) ) //read tiff/lsm stacks
	{
            printf("Image4DSimple::loadImage loading filename=%s\n", filename);

#if defined _WIN32 		
		{
			v3d_msg("(Win32) Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0)
			{
				if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error=1;
				}
			}	
			else //if ( strcasecmp(curFileSurfix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error=1;
				}
			}
		}
		
#else
		if (b_useMyLib)
		{
			v3d_msg("Now try to use MYLIB to read the TIFF/LSM again...\n",0);
			if (loadTif2StackMylib(imgSrcFile, data1d, tmp_sz, tmp_datatype, pixelnbits))
			{
				v3d_msg("Error happens in TIF/LSM file reading (using MYLIB). Stop. \n", false);
				b_error=1;
				return;
			}
			else
				b_error=0; //when succeed then reset b_error
		}
		else
		{
			v3d_msg("Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0)
			{
				if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error=1;
				}
			}	
			else //if ( strcasecmp(curFileSurfix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error=1;
				}
			}
		}
                printf("Image4DSimple::loadImage finished\n");

#endif
		
	}
	else if ( curFileSurfix && strcasecmp(curFileSurfix, "mrc")==0 ) //read mrc stacks
	{
		if (loadMRC2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			v3d_msg("Error happens in MRC file reading. Stop. \n", false);
			b_error=1;
			return;
		}
	}
#ifdef _ALLOW_WORKMODE_MENU_    
    else if ( curFileSurfix && ImageLoader::hasPbdExtension(QString(filename)) ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
    {
        ImageLoader imageLoader;
        QString imageSrcFile(imgSrcFile);
        if (!imageLoader.loadImage(this, imageSrcFile)) {
            v3d_msg("Error happens in v3dpbd file reading. Stop. \n", false);
            b_error=1;
            return;
        }
        // The following few lines are to avoid disturbing the existing code below
        tmp_datatype=this->getDatatype();
        tmp_sz=new V3DLONG[4];
        tmp_sz[0]=this->getXDim();
        tmp_sz[1]=this->getYDim();
        tmp_sz[2]=this->getZDim();
        tmp_sz[3]=this->getCDim();
        
        this->setFileName(filename); // PHC added 20121213 to fix a bug in the PDB reading.
    }
#endif    
	else //then assume it is Hanchuan's Vaa3D RAW format
	{
		v3d_msg("The data is not with a TIF surfix, -- now this program assumes it is Vaa3D's RAW format defined by Hanchuan Peng. \n", false);
		if (loadRaw2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			printf("The data doesn't look like a correct 4-byte-size Vaa3D's RAW file. Try 2-byte-raw. \n");
			if (loadRaw2Stack_2byte(imgSrcFile, data1d, tmp_sz, tmp_datatype))
			{
				v3d_msg("Error happens in reading 4-byte-size and 2-byte-size Vaa3D's RAW file. Stop. \n", false);
				b_error=1;
				return;
			}
		}
	}

	//080302: now convert any 16 bit or float data to the range of 0-255 (i.e. 8bit)
	switch (tmp_datatype)
	{
		case 1:
			datatype = V3D_UINT8;
			break;

		case 2: //080824
			//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			//datatype = UINT8; //UINT16;
			datatype = V3D_UINT16;
			break;

		case 4:
			//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			datatype = V3D_FLOAT32; //FLOAT32;
			break;

		default:
			v3d_msg("The data type is not UINT8, UINT16 or FLOAT32. Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
			if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}
			return;
	}

	sz0 = tmp_sz[0];
	sz1 = tmp_sz[1];
	sz2 = tmp_sz[2];
	sz3 = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions

	/* clean all workspace variables */

	if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}

	return;
}


bool Image4DSimple::saveImage(const char filename[])
{
	if (!data1d || !filename)
	{
		v3d_msg("This image data is empty or the file name is invalid. Nothing done.\n");
		return false;
	}

	V3DLONG mysz[4];
	mysz[0] = sz0;
	mysz[1] = sz1;
	mysz[2] = sz2;
	mysz[3] = sz3;

	int dt;
	switch (datatype)
	{
		case V3D_UINT8:  dt=1; break;
		case V3D_UINT16:  dt=2; break;
		case V3D_FLOAT32:  dt=4; break;
		default:
			v3d_msg("The data type is unsupported. Nothing done.\n");
			return false;
			break;
	}

	//061009
	char * curFileSurfix = getSurfix((char *)filename);
	printf("The current output file has the surfix [%s]\n", curFileSurfix);
	if (curFileSurfix && (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0)) //read tiff stacks
	{
		if (saveStack2Tif(filename, data1d, mysz, dt)!=0)
		{
			v3d_msg("Error happens in TIF file writing. Stop. \n");
			b_error=1;
			return false;
		}
	}
	else //then assume it is Hanchuan's RAW format
	{
		printf("The data is not with a TIF surfix, -- now this program assumes it is RAW format defined by Hanchuan Peng. \n");
		if (saveStack2Raw(filename, data1d, mysz, dt)!=0)   //0 is no error //note that as I updated the saveStack2Raw to RAW-4-byte, the actual mask file cannot be read by the old wano program, i.e. the wano must be updated on Windows machine as well. 060921
			//if (saveStack2Raw_2byte(filename, data1d, mysz, dt)!=0)   //for compatability save it to 2-byte raw //re-commented on 081124. always save to 4-byte raw
		{
			v3d_msg("Fail to save data to file [%s].\n", filename);
			b_error=1;
			return false;
		}
	}

	return true;
}


