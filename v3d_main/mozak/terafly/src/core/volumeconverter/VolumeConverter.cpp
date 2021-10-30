//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
* 2015-06-12. Giulio.     @FIXED the right output reference system is set in all cases at the end of the merge algorithm (the case MC input volume was not properly handled)
* 2015-04-14. Alessandro. @FIXED misleading usage of 'VirtualVolume::instance' w/o format argument in 'setSrcVolume'
* 2015-04-14. Alessandro. @FIXED bug-crash when the volume has not been imported correctly in setSrcVolume.
* 2015-03-03. Giulio.     @ADDED selection of IO plugin if not provided (2D or 3D according to the method).
* 2015-02-12. Giulio.     @ADDED the same optimizations also in multi-channels (MC) methods
* 2015-02-12. Giulio.     #ADDED check on the number of slice in the buffer if multiple resolutions are requested
* 2015-02-10. Giulio.     @ADDED completed optimizations to reduce opend/close in append operations (only in generateTilesVaa3DRaw)
* 2015-01-06. Giulio.     @ADDED optimizations to reduce opend/close in append operations (only in generateTilesVaa3DRaw)
* 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
* 2014-11-10. Giulio.     @CHANGED allowed saving 2dseries with a depth of 16 bit (generateTiles)
*/

#include "VolumeConverter.h"
#include "../imagemanager/IM_config.h"
#include "../imagemanager/ProgressBar.h"
#include "iomanager.config.h"
#include <math.h>
#include <string>

#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
#include <QElapsedTimer>
#include "PLog.h"
#include "COperation.h"
#endif

/*******************************************************************************************************
* Volume formats supported:
* 
* SimpleVolume:  simple sequence of slices stored as 2D images in the same directory
* StackedVolume: bidimensional matrix of 3D stacks stored in a hierarchical structure of directories
*
*******************************************************************************************************/
#include "../imagemanager/SimpleVolume.h"
#include "../imagemanager/SimpleVolumeRaw.h"
#include "../imagemanager/RawVolume.h"
#include "../imagemanager/TiledVolume.h"
#include "../imagemanager/TiledMCVolume.h"
#include "../imagemanager/StackedVolume.h"
#include "../imagemanager/TimeSeries.h"
/******************************************************************************************************/

#include "../imagemanager/Tiff3DMngr.h"
#include "../imagemanager/HDF5Mngr.h"

#include <limits>
#include <list>
#include <stdlib.h>
#include <sstream>
#include <cstdio>
#include "resumer.h"

using namespace iim;

VolumeConverter::VolumeConverter( )
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	volume = (VirtualVolume *) 0;
}


VolumeConverter::~VolumeConverter()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	if(volume)
		delete volume;
}


void VolumeConverter::setSrcVolume(const char* _root_dir, const char* _fmt, const char* _out_fmt, bool time_series /* = false */) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir = %s, _fmt = %s, _out_fmt = %s, time_series = %s",
                                         _root_dir, _fmt, _out_fmt, time_series ? "true" : "false").c_str(), __iim__current__function__);

    if(time_series)
        volume = new TimeSeries(_root_dir, _fmt);
    else
        //volume = VirtualVolume::instance(_root_dir, _fmt, vertical, horizontal, depth, 1.0f, 1.0f, 1.0f);
        //volume = VirtualVolume::instance_format(_root_dir);
        // 2015-04-14. Alessandro. @FIXED misleading usage of 'VirtualVolume::instance' w/o format argument in 'setSrcVolume'
        volume = VirtualVolume::instance_format(_root_dir, _fmt);
    
    // 2015-04-14 Alessandro. @FIXED bug-crash when the volume has not been imported correctly in setSrcVolume.
    if(!volume)
        throw iim::IOException(iim::strprintf("in VolumeConverter::setSrcVolume(): unable to recognize the volume format of \"%s\"", _root_dir));

	//channels = (volume->getDIM_C()>1) ? 3 : 1; // only 1 or 3 channels supported
	channels = volume->getDIM_C();

	if ( strcmp(_out_fmt,REAL_REPRESENTATION) == 0 ) {
		if ( channels > 1 ) {
            fprintf(stderr,"*** warning *** more than 1 channel, the internal representation has been changed\n");
			out_fmt = UINT8x3_REPRESENTATION;
			internal_rep = UINT8_INTERNAL_REP;
		}
		else {
			out_fmt = _out_fmt;
			internal_rep = REAL_INTERNAL_REP;
		}
	}
	else if ( strcmp(_out_fmt,UINT8_REPRESENTATION) == 0 ) { 
		out_fmt = _out_fmt;
		internal_rep = UINT8_INTERNAL_REP;
	}
	else if ( strcmp(_out_fmt,UINT8x3_REPRESENTATION) == 0 ) {
		out_fmt = _out_fmt;
		internal_rep = UINT8_INTERNAL_REP;
	}
	else {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::setSrcVolume: unsupported output format (%s)",out_fmt);
        throw IOException(err_msg);
	}

	V0 = 0;
	H0 = 0;
	D0 = 0;
	V1 = volume->getDIM_V(); 
	H1 = volume->getDIM_H();
	D1 = volume->getDIM_D();
}


/*************************************************************************************************************
* Method to be called for tile generation. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where generted tiles have to be stored.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [slice_height/width]	: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTiles(std::string output_path, bool* resolutions, 
				int slice_height, int slice_width, int method, bool show_progress_bar, const char* saved_img_format, 
                int saved_img_depth, std::string frame_dir)	throw (IOException)
{
    printf("in VolumeConverter::generateTiles(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", slice_height = %d, slice_width = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           slice_height, slice_width, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

	if ( saved_img_depth == 0 ) // default value: output depth is the same of input depth
		saved_img_depth = (volume->getBYTESxCHAN() * 8);
		
	if ( saved_img_depth != 8 && volume->getNACtiveChannels() > 1) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d bits per channel of destination is not supported for %d channels",
			saved_img_depth, volume->getNACtiveChannels());
		throw IOException(err_msg);
	}
	
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) { // saveImage_from and saveImage_from_UINT8 do not support depth conversion yet
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: a mismatch between bits per channel of source (%d) and destination (%d) is not supported",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
	}
	
	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
	int supported_channels;     //channels to be supported (stacks of tiffs 2D only supports 1 or 3 channels)
    // real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
	sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT], n_stacks_H[TMITREE_MAX_HEIGHT];             //array of number of tiles along V and H directions respectively at i-th resolution
    int **stacks_height[TMITREE_MAX_HEIGHT], **stacks_width[TMITREE_MAX_HEIGHT];	//array of matrices of tiles dimensions at i-th resolution
    std::stringstream file_path[TMITREE_MAX_HEIGHT];                            //array of root directory name at i-th resolution
	int resolutions_size = 0;

	if ( volume == 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTiles: undefined source volume");
        throw IOException(err_msg);
	}

	//initializing the progress bar
	char progressBarMsg[200];
	if(show_progress_bar)
	{
       imProgressBar::getInstance()->start("Multiresolution tile generation");
       imProgressBar::getInstance()->update(0,"Initializing...");
       imProgressBar::getInstance()->show();
	}

	// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
	if(iom::IMOUT_PLUGIN.compare("empty") == 0)
	{
		iom::IMOUT_PLUGIN = "tiff2D";
	}

	//computing dimensions of volume to be stitched
	//this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	//activating resolutions
    slice_height = (slice_height == -1 ? (int)height : slice_height);
    slice_width  = (slice_width  == -1 ? (int)width  : slice_width);
    if(slice_height < TMITREE_MIN_BLOCK_DIM || slice_width < TMITREE_MIN_BLOCK_DIM)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for both slice width and height is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }
	if(resolutions == NULL)
	{
            resolutions = new bool;
            *resolutions = true;
            resolutions_size = 1;
	}
	else
            for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
                if(resolutions[i])
                    resolutions_size = std::max(resolutions_size, i+1);

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
            n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) slice_height );
            n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) slice_width  );
            stacks_height[res_i] = new int *[n_stacks_V[res_i]];
            stacks_width[res_i]  = new int *[n_stacks_V[res_i]];
            for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
            {
                stacks_height[res_i][stack_row] = new int[n_stacks_H[res_i]];
                stacks_width [res_i][stack_row] = new int[n_stacks_H[res_i]];
                for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
                {
                    stacks_height[res_i][stack_row][stack_col] = ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width [res_i][stack_row][stack_col] = ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                }
            }
            //creating volume directory iff current resolution is selected and test mode is disabled
            if(resolutions[res_i] == true)
            {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,res_i)<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTiles(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }

                //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
                if ( frame_dir != "" ) {
                    file_path[res_i] << "/" << frame_dir << "/";
                    if(!check_and_make_dir(file_path[res_i].str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                        throw IOException(err_msg);
                    }
                }
            }
	}

	//ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,resolutions_size-1);
	z_ratio=depth/z_max_res;

	// check the number of channels
	if ( channels > 3 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"The volume contains too many channels (%d)", channels);
        throw IOException(err_msg);
	}

	//allocated even if not used
	org_channels = channels; // save for checks
	supported_channels = (channels>1) ? 3 : 1; // only 1 or 3 channels supported if output format is stacks of tiffs 2D
	ubuffer = new uint8 *[supported_channels];
	memset(ubuffer,0,supported_channels*sizeof(uint8 *)); // initializes to null pointers

	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
			// WARNING: next code assumes that channels is 1 or 3, but implementations of loadSubvolume_to_UINT8 do not guarantee this condition
			if ( org_channels != channels ) {
				char err_msg[STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
			}
		
			// code has been changed because the load operation can return 1, 2 or 3 channels
			/*
			if ( supported_channels == 3 ) {
				// offsets are to be computed taking into account that buffer size along D may be different
				ubuffer[1] = ubuffer[0] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
				ubuffer[2] = ubuffer[1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
			}
			*/
			// elements of ubuffer not set are null pointers
			for ( int c=1; c<channels; c++ )
				ubuffer[c] = ubuffer[c-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
		}
		
		//updating the progress bar
		if(show_progress_bar)
		{	
			sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1)),(uint32)depth);
                        imProgressBar::getInstance()->update(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
                        imProgressBar::getInstance()->show();
		}

		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			if(show_progress_bar)
			{
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                                imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                imProgressBar::getInstance()->show();
			}

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0) {	
				if ( internal_rep == REAL_INTERNAL_REP ) 
                    VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
                    VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),channels,method,bytes_chan);
			}

			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        imProgressBar::getInstance()->show();
				}

				//storing in 'base_path' the absolute path of the directory that will contain all stacks
				std::stringstream base_path;
                                base_path << output_path << "/RES(" << (int)(height/powInt(2,i)) << "x" << (int)(width/powInt(2,i)) << "x" << (int)(depth/powInt(2,i)) << ")/";

				//if frame_dir not empty must create frame directory
				if ( frame_dir != "" ) {
					base_path << frame_dir << "/";
					if(!check_and_make_dir(base_path.str().c_str())) 
					{
                        char err_msg[STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in generateTiles(...): unable to create DIR = \"%s\"\n", base_path.str().c_str());
                        throw IOException(err_msg);
					}
				}

				//looping on new stacks
				for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
				{
					//incrementing end_height
					end_height = start_height + stacks_height[i][stack_row][0]-1;
						
					//computing V_DIR_path and creating the directory the first time it is needed
					std::stringstream V_DIR_path;
					V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
					{
                        char err_msg[STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in mergeTiles(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                        throw IOException(err_msg);
					}

					for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
					{
						end_width  = start_width  + stacks_width [i][stack_row][stack_column]-1;
							
						//computing H_DIR_path and creating the directory the first time it is needed
						std::stringstream H_DIR_path;
						H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);

                        if(z==D0 && !check_and_make_dir(H_DIR_path.str().c_str()))
						{
                            char err_msg[STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                            throw IOException(err_msg);
						}

						//saving HERE
                        for(int buffer_z=0; buffer_z<z_size/(powInt(2,i)); buffer_z++)
						{
							std::stringstream img_path;
							std::stringstream abs_pos_z;
							abs_pos_z.width(6);
							abs_pos_z.fill('0');
							abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                (powInt(2,i)*buffer_z+z) * volume->getVXL_D());
							img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width) << "_"
										<< abs_pos_z.str(); 
							if ( internal_rep == REAL_INTERNAL_REP )
								VirtualVolume::saveImage(img_path.str(), 
                                    rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                    (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
									start_height,end_height,start_width,end_width, 
									saved_img_format, saved_img_depth);
							else // internal_rep == UINT8_INTERNAL_REP
								if ( channels == 1 )
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
                                        ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
										(uint8 *) 0,
										(uint8 *) 0,
                                        (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
								else if ( channels == 2 ) 
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
                                        ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                        ubuffer[1] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
										(uint8 *) 0,
                                        (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
								else // channels = 3
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
                                        ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                        ubuffer[1] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                        ubuffer[2] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                        (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
						}
						start_width  += stacks_width [i][stack_row][stack_column];
					}
					start_height += stacks_height[i][stack_row][0];
				}
			}
		}

		//releasing allocated memory
		if ( internal_rep == REAL_INTERNAL_REP )
			delete rbuffer;
		else // internal_rep == UINT8_INTERNAL_REP
			delete ubuffer[0]; // other buffer pointers are only offsets
	}

	// reloads created volumes to generate .bin file descriptors at all resolutions
	ref_sys reference(axis(1),axis(2),axis(3));
	TiledMCVolume *mcprobe;
	TiledVolume   *tprobe;
	StackedVolume *sprobe;
	sprobe = dynamic_cast<StackedVolume *>(volume);
	if ( sprobe ) {
		reference.first  = sprobe->getAXS_1();
		reference.second = sprobe->getAXS_2();
		reference.third  = sprobe->getAXS_3();
	}
	else {
		tprobe = dynamic_cast<TiledVolume *>(volume);
		if ( tprobe ) {
			reference.first  = tprobe->getAXS_1();
			reference.second = tprobe->getAXS_2();
			reference.third  = tprobe->getAXS_3();
		}
		else {
			mcprobe = dynamic_cast<TiledMCVolume *>(volume);
			if ( mcprobe ) {
				reference.first  = mcprobe->getAXS_1();
				reference.second = mcprobe->getAXS_2();
				reference.third  = mcprobe->getAXS_3();
			}
		}
	}
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		if(resolutions[res_i])
        {
            //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
            //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
            //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
            //system.
			//---- Giulio 2013-08-23 fixed
			StackedVolume temp_vol(file_path[res_i].str().c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

//			StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
//							volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
        }
	}


	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			delete []stacks_height[res_i][stack_row];
			delete []stacks_width [res_i][stack_row];
		}
		delete []stacks_height[res_i];
		delete []stacks_width[res_i]; 
	}
}


/*************************************************************************************************************
* Method to be called for tile generation in Vaa3D raw format. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where generted tiles have to be stored.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [block_height]	    : desired dimensions of tiled  blocks after merging.  It is actually an upper-bound of
* [block_width]			  the actual slice dimensions, which will be computed in such a way that all tiles di-
* [block_depth]			  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTilesVaa3DRaw(std::string output_path, bool* resolutions, 
				int block_height, int block_width, int block_depth, int method, 
				bool show_progress_bar, const char* saved_img_format, 
                int saved_img_depth, std::string frame_dir)	throw (IOException)
{
    printf("in VolumeConverter::generateTilesVaa3DRaw(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

	if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
		saved_img_depth = volume->getBYTESxCHAN() * 8;
		
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
	}

	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
	sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    int n_stacks_D[TMITREE_MAX_HEIGHT];
    int ***stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[TMITREE_MAX_HEIGHT];
    int ***stacks_depth[TMITREE_MAX_HEIGHT];
    std::stringstream file_path[TMITREE_MAX_HEIGHT];  //array of root directory name at i-th resolution
	int resolutions_size = 0;

	/* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
	 * In the following the term 'group' means the groups of slices that are 
	 * processed together to generate slices of all resolution requested
	 */

	/* stack_block[i] is the index of current block along z (it depends on the resolution i)
	 * current block is the block in which falls the first slice of the group
	 * of slices that is currently being processed, i.e. from z to z+z_max_res-1
	 */
    int stack_block[TMITREE_MAX_HEIGHT];

	/* these arrays are the indices of first and last slice of current block at resolution i
	 * WARNING: the slice index refers to the index of the slice in the volume at resolution i 
	 */
    int slice_start[TMITREE_MAX_HEIGHT];
    int slice_end[TMITREE_MAX_HEIGHT];

	/* the number of slice of already processed groups at current resolution
	 * the index of the slice to be saved at current resolution is:
	 *
	 *      n_slices_pred + z_buffer
	 */
	sint64 n_slices_pred;  

	if ( volume == 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: undefined source volume");
        throw IOException(err_msg);
	}

	//initializing the progress bar
	char progressBarMsg[200];
	if(show_progress_bar)
	{
                   imProgressBar::getInstance()->start("Multiresolution tile generation");
                   imProgressBar::getInstance()->update(0,"Initializing...");
                   imProgressBar::getInstance()->show();
	}

	// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
	if(iom::IMOUT_PLUGIN.compare("empty") == 0)
	{
		iom::IMOUT_PLUGIN = "tiff3D";
	}

	//computing dimensions of volume to be stitched
	//this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	// code for testing
	//uint8 *temp = volume->loadSubvolume_to_UINT8(
	//	10,height-10,10,width-10,10,depth-10,
	//	&channels);

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM) */)
    { 
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

	if(resolutions == NULL)
	{
            resolutions = new bool;
            *resolutions = true;
            resolutions_size = 1;
	}
	else
            for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
                if(resolutions[i])
                    resolutions_size = std::max(resolutions_size, i+1);

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
            n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
            n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
            n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,res_i))  / (float) block_depth  );
            stacks_height[res_i] = new int **[n_stacks_V[res_i]];
            stacks_width[res_i]  = new int **[n_stacks_V[res_i]]; 
            stacks_depth[res_i]  = new int **[n_stacks_V[res_i]]; 
            for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
            {
                stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
                stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
                stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
                for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
                {
					stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
					stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
					stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
					for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
					{
						stacks_height[res_i][stack_row][stack_col][stack_sli] = 
                            ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
						stacks_width[res_i][stack_row][stack_col][stack_sli] = 
                            ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
						stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
                            ((int)(depth/powInt(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/powInt(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
					}
                }
            }
            //creating volume directory iff current resolution is selected and test mode is disabled
            if(resolutions[res_i] == true)
            {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,res_i)<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }

                //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
                if ( frame_dir != "" ) {
                    file_path[res_i] << "/" << frame_dir << "/";
                    if(!check_and_make_dir(file_path[res_i].str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                        throw IOException(err_msg);
                    }
                }
            }
	}

	/* The following check verifies that the numeber of slices in the buffer is not higher than the numbero of slices in a block file
	 * (excluding the last blck in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
	 * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block 
	 * file at that resolution as computed by the above code is:
	 *
	 *                                                B = floor( H / ceil( H/D ) )
	 * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
	 *
	 *                                                  D >= B >= floor(D/2)
	 * since it is:
	 *
	 *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
	 * where alpha<1.
	 */ 

	//ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,resolutions_size-1);
	if ( z_max_res > block_depth/2 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
		throw IOException(err_msg);
	}
	z_ratio=depth/z_max_res;

	//allocated even if not used
	ubuffer = new uint8 *[channels];
	memset(ubuffer,0,channels*sizeof(uint8 *));
	org_channels = channels; // save for checks

	FILE *fhandle;
	sint64 z;
	sint64 z_parts;
	if ( initResumer("Vaa3DRaw",output_path.c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) {
		readResumerState(fhandle,output_path.c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts);
	}
	else {
		//slice_start and slice_end of current block depend on the resolution
		for(int res_i=0; res_i< resolutions_size; res_i++) {
			stack_block[res_i] = 0;
			//slice_start[res_i] = this->D0; 
			slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning 
			slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
		}
		z = this->D0;
		z_parts = 1;
	}

	// z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
	for(/* sint64 z = this->D0, z_parts = 1 */; z < this->D1; z += z_max_res, z_parts++)
	{
		//if ( z > (this->D1/2) ) {
		//	closeResumer(fhandle);
		//	throw IOExcpetion("interruption for test");
		//}

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
        #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_START(ConverterLoadBlockOperation)
        #endif

		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
			if ( org_channels != channels ) {
				char err_msg[STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
			}
		
			for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
				// offsets have to be computed taking into account that buffer size along D may be different
				// WARNING: the offset must be of tipe sint64 
				ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
			}
		}
		// WARNING: should check that buffer has been actually allocated

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
        #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_STOP(ConverterLoadBlockOperation, itm::ALL_COMPS, teramanager::strprintf("converter: loaded image block x(%d-%d), y(%d-%d), z(%d-%d)",H0, H1, V0, V1, ((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1))))
        #endif

		//updating the progress bar
		if(show_progress_bar)
		{	
			sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1)),(uint32)depth);
                        imProgressBar::getInstance()->update(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
                        imProgressBar::getInstance()->show();
		}

		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			if(show_progress_bar)
			{
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                                imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                imProgressBar::getInstance()->show();
			}

			// check if current block is changed
 			// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
			if ( ((z - this->D0) / powInt(2,i)) > slice_end[i] ) {
				stack_block[i]++;
				slice_start[i] = slice_end[i] + 1;
				slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
			}

			// find abs_pos_z at resolution i
			std::stringstream abs_pos_z;
			abs_pos_z.width(6);
			abs_pos_z.fill('0');
			abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
 								- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
                               (powInt(2,i)*slice_start[i]) * volume->getVXL_D());

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / powInt(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0) {
				if ( internal_rep == REAL_INTERNAL_REP ) 
                    VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),method);
				else  // internal_rep == UINT8_INTERNAL_REP
                    VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),channels,method,bytes_chan);
			}
				
			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        imProgressBar::getInstance()->show();
				}

				//storing in 'base_path' the absolute path of the directory that will contain all stacks
				std::stringstream base_path;
                base_path << output_path << "/RES(" << (int)(height/powInt(2,i)) << "x" <<
                    (int)(width/powInt(2,i)) << "x" << (int)(depth/powInt(2,i)) << ")/";

				//if frame_dir not empty must create frame directory
				if ( frame_dir != "" ) {
					base_path << frame_dir << "/";
					if(!check_and_make_dir(base_path.str().c_str())) 
					{
                        char err_msg[STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", base_path.str().c_str());
                        throw IOException(err_msg);
					}
				}

				//looping on new stacks
				for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
				{
					//incrementing end_height
					end_height = start_height + stacks_height[i][stack_row][0][0]-1; 
						
					//computing V_DIR_path and creating the directory the first time it is needed
					std::stringstream V_DIR_path;
					V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
					{
                        char err_msg[STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                        throw IOException(err_msg);
					}

					for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
					{
						end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1; 
							
						//computing H_DIR_path and creating the directory the first time it is needed
						std::stringstream H_DIR_path;
						H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
						if ( z==D0 ) {
							if(!check_and_make_dir(H_DIR_path.str().c_str()))
							{
                                char err_msg[STATIC_STRINGS_SIZE];
								sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                                throw IOException(err_msg);
							}
							else { // the directory has been created for the first time
								   // initialize block files
								V3DLONG *sz = new V3DLONG[4];
								int datatype;
								char *err_rawfmt;

								sz[0] = stacks_width[i][stack_row][stack_column][0];
								sz[1] = stacks_height[i][stack_row][stack_column][0];
								sz[3] = channels;

								if ( internal_rep == REAL_INTERNAL_REP )
									datatype = 4;
								else if ( internal_rep == UINT8_INTERNAL_REP ) {
									if ( saved_img_depth == 16 )
										datatype = 2;
									else if ( saved_img_depth == 8 ) 
										datatype = 1;
									else {
                                        char err_msg[STATIC_STRINGS_SIZE];
										sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
                                        throw IOException(err_msg);
									}
								}
								else {
                                    char err_msg[STATIC_STRINGS_SIZE];
									sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
                                    throw IOException(err_msg);
								}

								int slice_start_temp = 0;
								for ( int j=0; j < n_stacks_D[i]; j++ ) {
									sz[2] = stacks_depth[i][stack_row][stack_column][j];

									std::stringstream abs_pos_z_temp;
									abs_pos_z_temp.width(6);
									abs_pos_z_temp.fill('0');
									abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                        (powInt(2,i)*(slice_start_temp)) * volume->getVXL_D());

									std::stringstream img_path_temp;
									img_path_temp << H_DIR_path.str() << "/" 
												  << this->getMultiresABS_V_string(i,start_height) << "_" 
												  << this->getMultiresABS_H_string(i,start_width) << "_"
												  << abs_pos_z_temp.str();

									//if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
									if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
												( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),sz[0],sz[1],sz[2],sz[3],datatype)) != 0 ) : 
												( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
										char err_msg[STATIC_STRINGS_SIZE];
										sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: error in initializing block file - %s", err_rawfmt);
                                        throw IOException(err_msg);
									};

									slice_start_temp += (int)sz[2];
								}
								delete [] sz;
							}
						}

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
                        #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_START(ConverterWriteBlockOperation)
                        #endif

						//saving HERE

						// 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
 						std::stringstream partial_img_path;
						partial_img_path << H_DIR_path.str() << "/" 
									<< this->getMultiresABS_V_string(i,start_height) << "_" 
									<< this->getMultiresABS_H_string(i,start_width) << "_";

						int slice_ind = (int)(n_slices_pred - slice_start[i]); 

 						std::stringstream img_path;
						img_path << partial_img_path.str() << abs_pos_z.str();

						/* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
						 * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations, 
						 * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at 
						 * end of the file.
						 * The number of pages of block files of interest can be easily computed as:
						 *
						 *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]] 
						 *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1] 
						 */

						void *fhandle = 0;
						int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
						bool block_changed = false;                                 // true if block is changed executing the next for cycle
						// fhandle = open file corresponding to current block 
						if ( strcmp(saved_img_format,"Tiff3D") == 0 )
							openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle);

						// WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
                        for(int buffer_z=0; buffer_z<z_size/(powInt(2,i)); buffer_z++, slice_ind++)
						{
							// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
							if ( ((z - this->D0) / powInt(2,i) + buffer_z) > slice_end[i] && !block_changed) { // start a new block along z
 								std::stringstream abs_pos_z_next;
								abs_pos_z_next.width(6);
								abs_pos_z_next.fill('0');
								abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                        (powInt(2,i)*(slice_end[i]+1)) * volume->getVXL_D());
								img_path.str("");
								img_path << partial_img_path.str() << abs_pos_z_next.str();

								slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

								// close(fhandle) i.e. file corresponding to current block 
								if ( strcmp(saved_img_format,"Tiff3D") == 0 )
									closeTiff3DFile(fhandle);
								// fhandle = open file corresponding to next block
								if ( strcmp(saved_img_format,"Tiff3D") == 0 )
									openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle);
								n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
								block_changed = true;
							}

							if ( internal_rep == REAL_INTERNAL_REP )
								VirtualVolume::saveImage_to_Vaa3DRaw(
									slice_ind,
									img_path.str(), 
                                    rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                    (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
									start_height,end_height,start_width,end_width, 
									saved_img_format, saved_img_depth
								);
							else // internal_rep == UINT8_INTERNAL_REP
								if ( strcmp(saved_img_format,"Tiff3D")==0 ) {
									VirtualVolume::saveImage_from_UINT8_to_Tiff3D(
										slice_ind,
										img_path.str(), 
										ubuffer,
										channels,
										buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
										(int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth,fhandle,n_pages_block,false);
								}
								else { // can be only Vaa3DRaw
									VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
										slice_ind,
										img_path.str(), 
										ubuffer,
										channels,
										buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
										(int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
								}
                        }

						// close(fhandle) i.e. currently opened file
						if ( strcmp(saved_img_format,"Tiff3D") == 0 )
							closeTiff3DFile(fhandle);

						start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
                        #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_STOP(ConverterWriteBlockOperation, itm::ALL_COMPS, teramanager::strprintf("converter: written multiresolution image block x(%d-%d), y(%d-%d), z(%d-%d)",start_width, end_width, start_height, end_height, ((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1))))
                        #endif
					}
					start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
				}
			}
		}

		//releasing allocated memory
		if ( internal_rep == REAL_INTERNAL_REP )
			delete rbuffer;
		else // internal_rep == UINT8_INTERNAL_REP
			delete ubuffer[0]; // other buffer pointers are only offsets
		
		// save next group data
		saveResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z+z_max_res,z_parts+1);
	}

	closeResumer(fhandle,output_path.c_str());

	// reloads created volumes to generate .bin file descriptors at all resolutions
	ref_sys reference(axis(1),axis(2),axis(3));
	TiledMCVolume *mcprobe;
	TiledVolume   *tprobe;
	StackedVolume *sprobe;
	int n_err = 0; 
	sprobe = dynamic_cast<StackedVolume *>(volume);
	if ( sprobe ) {
		reference.first  = sprobe->getAXS_1();
		reference.second = sprobe->getAXS_2();
		reference.third  = sprobe->getAXS_3();
	}
	else {
		tprobe = dynamic_cast<TiledVolume *>(volume);
		if ( tprobe ) {
			reference.first  = tprobe->getAXS_1();
			reference.second = tprobe->getAXS_2();
			reference.third  = tprobe->getAXS_3();
		}
		else {
			mcprobe = dynamic_cast<TiledMCVolume *>(volume);
			if ( mcprobe ) {
				reference.first  = mcprobe->getAXS_1();
				reference.second = mcprobe->getAXS_2();
				reference.third  = mcprobe->getAXS_3();
			}
		}
	}
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
        if(resolutions[res_i])
        {
            //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
            //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
            //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
            //system.
			try {
				TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
						volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
			}
            catch (IOException & ex)
            {
                printf("n VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
                n_err++;
            }
            catch ( ... )
            {
                printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
				n_err++;
			}

//          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
//                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
        }
	}


	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				delete []stacks_height[res_i][stack_row][stack_col];
				delete []stacks_width [res_i][stack_row][stack_col];
				delete []stacks_depth [res_i][stack_row][stack_col];
			}
			delete []stacks_height[res_i][stack_row];
			delete []stacks_width [res_i][stack_row];
			delete []stacks_depth [res_i][stack_row];
		}
		delete []stacks_height[res_i];
		delete []stacks_width[res_i]; 
		delete []stacks_depth[res_i]; 
	}

	if ( n_err ) { // errors in mdat.bin creation
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
        throw IOException(err_msg);
	}
}


/*************************************************************************************************************
* Functions used to obtain absolute coordinates at different resolutions from relative coordinates
**************************************************************************************************************/
int VolumeConverter::getMultiresABS_V(int res, int REL_V)
{
	if(volume->getVXL_V() > 0)
        return volume->getABS_V( V0 + REL_V*pow(2.0f,res) )*10;
	else
        return volume->getABS_V( V0 - 1 + REL_V*pow(2.0f,res))*10 + volume->getVXL_V()*pow(2.0f,res)*10;
}
std::string VolumeConverter::getMultiresABS_V_string(int res, int REL_V)	
{
	std::stringstream multires_merging_x_pos;
	multires_merging_x_pos.width(6);
	multires_merging_x_pos.fill('0');
	multires_merging_x_pos << this->getMultiresABS_V(res, REL_V);
	return multires_merging_x_pos.str();
}
int VolumeConverter::getMultiresABS_H(int res, int REL_H)
{
	if(volume->getVXL_H() > 0)
        return volume->getABS_H( H0 + REL_H*pow(2.0f,res) )*10;
	else
        return volume->getABS_H( H0 - 1 + REL_H*pow(2.0f,res))*10  + volume->getVXL_H()*pow(2.0f,res)*10;
}
std::string VolumeConverter::getMultiresABS_H_string(int res, int REL_H)	
{
	std::stringstream multires_merging_y_pos;
	multires_merging_y_pos.width(6);
	multires_merging_y_pos.fill('0');
	multires_merging_y_pos << this->getMultiresABS_H(res, REL_H);
	return multires_merging_y_pos.str();
}
int VolumeConverter::getMultiresABS_D(int res)
{
	if(volume->getVXL_D() > 0)
        return volume->getABS_D(D0);
	else
        return volume->getABS_D((int)(D0 - 1 + volume->getVXL_D()*pow(2.0f,res)*10.0f));
}



/*************************************************************************************************************
* NEW TILED FORMAT SUPPORTING MULTIPLE CHANNELS
**************************************************************************************************************/

# ifdef RES_IN_CHANS // resolutions directories in channels directories

void VolumeConverter::generateTilesVaa3DRawMC ( std::string output_path, bool* resolutions, 
				int block_height, int block_width, int block_depth, int method, 
				bool show_progress_bar, const char* saved_img_format, 
				int saved_img_depth, std::string frame_dir )	throw (IOExcpetion)
{
    printf("in VolumeConverter::generateTilesVaa3DRawMC(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< S_MAX_MULTIRES; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

	if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
		saved_img_depth = volume->getBYTESxCHAN() * 8;
		
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
		throw IOExcpetion(err_msg);
	}

	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
	sint64 z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[S_MAX_MULTIRES];
	int n_stacks_D[S_MAX_MULTIRES];  
    int ***stacks_height[S_MAX_MULTIRES];   //array of matrices of tiles dimensions at i-th resolution
	int ***stacks_width[S_MAX_MULTIRES];	
	int ***stacks_depth[S_MAX_MULTIRES];
	std::string *chans_dir;
	std::string resolution_dir;
    std::stringstream file_path[S_MAX_MULTIRES];                            //array of root directory name at i-th resolution
	int resolutions_size = 0;

	/* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
	 * In the following the term 'group' means the groups of slices that are 
	 * processed together to generate slices of all resolution requested
	 */

	/* stack_block[i] is the index of current block along z (it depends on the resolution i)
	 * current block is the block in which falls the first slice of the group
	 * of slices that is currently being processed, i.e. from z to z+z_max_res-1
	 */
	int stack_block[S_MAX_MULTIRES]; 

	/* these arrays are the indices of first and last slice of current block at resolution i
	 * WARNING: the slice index refers to the index of the slice in the volume at resolution i 
	 */
	int slice_start[S_MAX_MULTIRES];   
	int slice_end[S_MAX_MULTIRES];

	/* the number of slice of already processed groups at current resolution
	 * the index of the slice to be saved at current resolution is:
	 *
	 *      n_slices_pred + z_buffer
	 */
	sint64 n_slices_pred;       

	if ( volume == 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: undefined source volume");
		throw IOExcpetion(err_msg);
	}

	//initializing the progress bar
	char progressBarMsg[200];
	if(show_progress_bar)
	{
                   imProgressBar::getInstance()->start("Multiresolution tile generation");
                   imProgressBar::getInstance()->update(0,"Initializing...");
                   imProgressBar::getInstance()->show();
	}

	// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
	if(iom::IMOUT_PLUGIN.compare("empty") == 0)
	{
		iom::IMOUT_PLUGIN = "tiff3D";
	}

	//computing dimensions of volume to be stitched
	//this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	// code for testing
	//uint8 *temp = volume->loadSubvolume_to_UINT8(
	//	10,height-10,10,width-10,10,depth-10,
	//	&channels);

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < S_MIN_SLICE_DIM) */)
    { 
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", S_MIN_SLICE_DIM);
        throw IOExcpetion(err_msg);
    }

	if(resolutions == NULL)
	{
            resolutions = new bool;
            *resolutions = true;
            resolutions_size = 1;
	}
	else
            for(int i=0; i<S_MAX_MULTIRES; i++)
                if(resolutions[i])
                    resolutions_size = std::max(resolutions_size, i+1);

	//computing tiles dimensions at each resolution
	for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
		n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
		n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,res_i))  / (float) block_depth  );
		stacks_height[res_i] = new int **[n_stacks_V[res_i]];
		stacks_width[res_i]  = new int **[n_stacks_V[res_i]]; 
		stacks_depth[res_i]  = new int **[n_stacks_V[res_i]]; 
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
			stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
			stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
				{
					stacks_height[res_i][stack_row][stack_col][stack_sli] = 
						((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/POW_INT(2,res_i))) % n_stacks_V[res_i] ? 1:0);
					stacks_width[res_i][stack_row][stack_col][stack_sli] = 
						((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
					stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
						((int)(depth/powInt(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/POW_INT(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
				}
			}
		}
	}

	// computing resolutions directory names
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		//creating volume directory iff current resolution is selected and test mode is disabled
		if(resolutions[res_i] == true) {
			file_path[res_i] << "/RES("<<height/POW_INT(2,res_i) 
							 << "x" << width/POW_INT(2,res_i) 
							 << "x" << depth/POW_INT(2,res_i) << ")";
		}
	}

	// computing channel directory names
	chans_dir = new std::string[channels];
	int n_digits = 1;
	int _channels = channels / 10;	
	while ( _channels ) {
		n_digits++;
		_channels /= 10;
	}
	for ( int c=0; c<channels; c++ ) {
 		std::stringstream dir_name;
		dir_name.width(n_digits);
		dir_name.fill('0');
		dir_name << c;
		chans_dir[c] = output_path + "/" + IM_CHANNEL_PREFIX + dir_name.str();
		//if(make_dir(chans_dir[c].c_str())!=0) 
		if(!check_and_make_dir(chans_dir[c].c_str())) { // HP 130914
		{
			char err_msg[S_MAX_MULTIRES];
			sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", chans_dir[c].c_str());
			throw IOExcpetion(err_msg);
		}
		for(int res_i=0; res_i< resolutions_size; res_i++) {
			//creating volume directory iff current resolution is selected and test mode is disabled
			if(resolutions[res_i] == true) {
				//creating directory that will contain image data at current resolution
				//resolution_dir = chans_dir[c] + file_path[res_i].str();
				resolution_dir = file_path[res_i].str() + chans_dir[c];
				//if(make_dir(resolution_dir.c_str())!=0)
                if(!check_and_make_dir(resolution_dir.c_str())) // HP 130914
				{
					char err_msg[S_MAX_MULTIRES];
					sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
					throw IOExcpetion(err_msg);
				}
			}
		}
	}

	/* The following check verifies that the numeber of slices in the buffer is not higher than the numbero of slices in a block file
	 * (excluding the last blck in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
	 * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block 
	 * file at that resolution as computed by the above code is:
	 *
	 *                                                B = floor( H / ceil( H/D ) )
	 * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
	 *
	 *                                                  D >= B >= floor(D/2)
	 * since it is:
	 *
	 *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
	 * where alpha<1.
	 */ 

		//ALLOCATING  the MEMORY SPACE for image buffer
	z_max_res = powInt(2,resolutions_size-1);
	if ( z_max_res > block_depth/2 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
		throw IOException(err_msg);
	}
	z_ratio=depth/z_max_res;

	//allocated even if not used
	ubuffer = new uint8 *[channels];
	memset(ubuffer,0,channels*sizeof(uint8));
	org_channels = channels; // save for checks

	//slice_start and slice_end of current block depend on the resolution
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		stack_block[res_i] = 0;
		//slice_start[res_i] = this->D0; 
		slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning 
		slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
	}

	// z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
			if ( org_channels != channels ) {
				char err_msg[STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
				throw IOExcpetion(err_msg);
			}
		
			for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
				// offsets have to be computed taking into account that buffer size along D may be different
				// WARNING: the offset must be of tipe sint64 
				ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
			}
		}
		// WARNING: should check that buffer has been actually allocated

		//updating the progress bar
		if(show_progress_bar)
		{	
			sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1)),(uint32)depth);
                        imProgressBar::getInstance()->update(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
                        imProgressBar::getInstance()->show();
		}

		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			if(show_progress_bar)
			{
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                                imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                imProgressBar::getInstance()->show();
			}

 			// check if current block is changed
			// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
			if ( ((z - this->D0) / powInt(2,i)) > slice_end[i] ) {
				stack_block[i]++;
				slice_start[i] = slice_end[i] + 1;
				slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
			}

			// find abs_pos_z at resolution i
			std::stringstream abs_pos_z;
			abs_pos_z.width(6);
			abs_pos_z.fill('0');
			abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
								- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
								(POW_INT(2,i)*slice_start[i]) * volume->getVXL_D());

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
			n_slices_pred  = (z_parts - 1) * z_max_res / POW_INT(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0) {	
				if ( internal_rep == REAL_INTERNAL_REP )
					VirtualVolume::halveSample(rbuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
					VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),channels,method,bytes_chan);
			}
			
			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
			if(resolutions[i] && (z_size/(POW_INT(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        imProgressBar::getInstance()->show();
				}

				for ( int c=0; c<channels; c++ ) {

					//storing in 'base_path' the absolute path of the directory that will contain all stacks
					std::stringstream base_path;
									base_path << chans_dir[c] << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << (int)(width/POW_INT(2,i)) << "x" << (int)(depth/POW_INT(2,i)) << ")/";

					//looping on new stacks
					for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
					{
						//incrementing end_height
						end_height = start_height + stacks_height[i][stack_row][0][0]-1; 
							
						//computing V_DIR_path and creating the directory the first time it is needed
						std::stringstream V_DIR_path;
						V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
						if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
						{
                            char err_msg[STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
							throw IOExcpetion(err_msg);
						}

						for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
						{
							end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1; 
								
							//computing H_DIR_path and creating the directory the first time it is needed
							std::stringstream H_DIR_path;
							H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
							if ( z==D0 ) {
								if(!check_and_make_dir(H_DIR_path.str().c_str()))
								{
                                    char err_msg[STATIC_STRINGS_SIZE];
									sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
									throw IOExcpetion(err_msg);
								}
								else { // the directory has been created for the first time
									   // initialize block files
									V3DLONG *sz = new V3DLONG[4];
									int datatype;
									char *err_rawfmt;

									sz[0] = stacks_width[i][stack_row][stack_column][0];
									sz[1] = stacks_height[i][stack_row][stack_column][0];
									sz[3] = 1; // single channel files

									if ( internal_rep == REAL_INTERNAL_REP )
										datatype = 4;
									else if ( internal_rep == UINT8_INTERNAL_REP ) {
										if ( saved_img_depth == 16 )
											datatype = 2;
										else if ( saved_img_depth == 8 ) 
											datatype = 1;
										else {
                                            char err_msg[STATIC_STRINGS_SIZE];
											sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
											throw IOExcpetion(err_msg);
										}
									}
									else {
                                        char err_msg[STATIC_STRINGS_SIZE];
										sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
										throw IOExcpetion(err_msg);
									}

									int slice_start_temp = 0;
									for ( int j=0; j < n_stacks_D[i]; j++ ) {
										sz[2] = stacks_depth[i][stack_row][stack_column][j];

										std::stringstream abs_pos_z_temp;
										abs_pos_z_temp.width(6);
										abs_pos_z_temp.fill('0');
										abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
											(POW_INT(2,i)*(slice_start_temp)) * volume->getVXL_D());

										std::stringstream img_path_temp;
										img_path_temp << H_DIR_path.str() << "/" 
													  << this->getMultiresABS_V_string(i,start_height) << "_" 
													  << this->getMultiresABS_H_string(i,start_width) << "_"
													  << abs_pos_z_temp.str();

										//if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
										if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
												( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),sz[0],sz[1],sz[2],sz[3],datatype)) != 0 ) : 
												( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
											char err_msg[STATIC_STRINGS_SIZE];
											sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: error in initializing block file - %s", err_rawfmt);
											throw IOExcpetion(err_msg);
										};

										slice_start_temp += (int)sz[2];
									}
									delete [] sz;
								}
							}

							//saving HERE

							// 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
 							std::stringstream partial_img_path;
							partial_img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width) << "_";

							int slice_ind = (int)(n_slices_pred - slice_start[i]); 

 							std::stringstream img_path;
							img_path << partial_img_path.str() << abs_pos_z.str();

							/* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
							 * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations, 
							 * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at 
							 * end of the file.
							 * The number of pages of block files of interest can be easily computed as:
							 *
							 *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]] 
							 *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1] 
							 */

							void *fhandle = 0;
							int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
							bool block_changed = false;                                 // true if block is changed executing the next for cycle
							// fhandle = open file corresponding to current block 
							if ( strcmp(saved_img_format,"Tiff3D") == 0 )
								openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle);

							// WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
							for(int buffer_z=0; buffer_z<z_size/(POW_INT(2,i)); buffer_z++, slice_ind++) 
							{
								// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
								if ( ((z - this->D0)  /powInt(2,i)+buffer_z) > slice_end[i] && !block_changed) { // start a new block along z
 									std::stringstream abs_pos_z_next;
									abs_pos_z_next.width(6);
									abs_pos_z_next.fill('0');
									abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
											(powInt(2,i)*(slice_end[i]+1)) * volume->getVXL_D());
									img_path.str("");
									img_path << partial_img_path.str() << abs_pos_z_next.str();

									slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

									// close(fhandle) i.e. file corresponding to current block 
									if ( strcmp(saved_img_format,"Tiff3D") == 0 )
										closeTiff3DFile(fhandle);
									// fhandle = open file corresponding to next block
									if ( strcmp(saved_img_format,"Tiff3D") == 0 )
										openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle);
									openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle);
									n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
									block_changed = true;
								}

								if ( internal_rep == REAL_INTERNAL_REP )
									VirtualVolume::saveImage_to_Vaa3DRaw(
										slice_ind,
										img_path.str(), 
										rbuffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth
									);
								else {// internal_rep == UINT8_INTERNAL_REP
									if ( strcmp(saved_img_format,"Tiff3D")==0 ) {
										VirtualVolume::saveImage_from_UINT8_to_Tiff3D(
											slice_ind,
											img_path.str(), 
											ubuffer + c,
											1,
											buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
											(int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
											start_height,end_height,start_width,end_width, 
											saved_img_format, saved_img_depth,fhandle,n_pages_block,false);
									}
									else { // can be only Vaa3DRaw
										VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
											slice_ind,
											img_path.str(), 
											ubuffer + c,
											1,
											buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
											(int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
											start_height,end_height,start_width,end_width, 
											saved_img_format, saved_img_depth);
									}
								}
							}

							// close(fhandle) i.e. currently opened file
							if ( strcmp(saved_img_format,"Tiff3D") == 0 )
								closeTiff3DFile(fhandle);

							start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
						}
						start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
					}
				}
			}
		}

		//releasing allocated memory
		if ( internal_rep == REAL_INTERNAL_REP )
			delete rbuffer;
		else // internal_rep == UINT8_INTERNAL_REP
			delete ubuffer[0]; // other buffer pointers are only offsets
	}

	// reloads created volumes to generate .bin file descriptors at all resolutions
	ref_sys reference(axis(1),axis(2),axis(3));
	TiledMCVolume *mcprobe;
	TiledVolume   *tprobe;
	StackedVolume *sprobe;
	sprobe = dynamic_cast<StackedVolume *>(volume);
	if ( sprobe ) {
		reference.first  = sprobe->getAXS_1();
		reference.second = sprobe->getAXS_2();
		reference.third  = sprobe->getAXS_3();
	}
	else {
		tprobe = dynamic_cast<TiledVolume *>(volume);
		if ( tprobe ) {
			reference.first  = tprobe->getAXS_1();
			reference.second = tprobe->getAXS_2();
			reference.third  = tprobe->getAXS_3();
		}
		else {
			mcprobe = dynamic_cast<TiledMCVolume *>(volume);
			if ( mcprobe ) {
				reference.first  = mcprobe->getAXS_1();
				reference.second = mcprobe->getAXS_2();
				reference.third  = mcprobe->getAXS_3();
			}
		}
	}
	for ( int c=0; c<channels; c++ ) {
		for(int res_i=0; res_i< resolutions_size; res_i++)
		{
			if(resolutions[res_i])
			{
				resolution_dir = chans_dir[c] + file_path[res_i].str();

				//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
				//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
				//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
				//system.
				TiledVolume temp_vol(resolution_dir.c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

	//          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
	//                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
			}
		}
	}


	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				delete[] stacks_height[res_i][stack_row][stack_col];
				delete[] stacks_width [res_i][stack_row][stack_col];
				delete[] stacks_depth [res_i][stack_row][stack_col];
			}
			delete[] stacks_height[res_i][stack_row];
			delete[] stacks_width [res_i][stack_row];
			delete[] stacks_depth [res_i][stack_row];
		}
		delete[] stacks_height[res_i];
		delete[] stacks_width[res_i]; 
		delete[] stacks_depth[res_i]; 
	}
	delete[] chans_dir;
}

# else // channels directories in resolutions directories

void VolumeConverter::generateTilesVaa3DRawMC ( std::string output_path, bool* resolutions, 
				int block_height, int block_width, int block_depth, int method, 
				bool show_progress_bar, const char* saved_img_format, 
                int saved_img_depth, std::string frame_dir )	throw (IOException)
{
    printf("in VolumeConverter::generateTilesVaa3DRawMC(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

	if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
		saved_img_depth = volume->getBYTESxCHAN() * 8;
		
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
	}

	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
	sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    int n_stacks_D[TMITREE_MAX_HEIGHT];
    int ***stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[TMITREE_MAX_HEIGHT];
    int ***stacks_depth[TMITREE_MAX_HEIGHT];
	std::string *chans_dir;
	std::string resolution_dir;
    std::stringstream file_path[TMITREE_MAX_HEIGHT];                            //array of root directory name at i-th resolution
	int resolutions_size = 0;

	/* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
	 * In the following the term 'group' means the groups of slices that are 
	 * processed together to generate slices of all resolution requested
	 */

	/* stack_block[i] is the index of current block along z (it depends on the resolution i)
	 * current block is the block in which falls the first slice of the group
	 * of slices that is currently being processed, i.e. from z to z+z_max_res-1
	 */
    int stack_block[TMITREE_MAX_HEIGHT];

	/* these arrays are the indices of first and last slice of current block at resolution i
	 * WARNING: the slice index refers to the index of the slice in the volume at resolution i 
	 */
    int slice_start[TMITREE_MAX_HEIGHT];
    int slice_end[TMITREE_MAX_HEIGHT];

	/* the number of slice of already processed groups at current resolution
	 * the index of the slice to be saved at current resolution is:
	 *
	 *      n_slices_pred + z_buffer
	 */
	sint64 n_slices_pred;       

	if ( volume == 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: undefined source volume");
        throw IOException(err_msg);
	}

	//initializing the progress bar
	char progressBarMsg[200];
	if(show_progress_bar)
	{
       imProgressBar::getInstance()->start("Multiresolution tile generation");
       imProgressBar::getInstance()->update(0,"Initializing...");
       imProgressBar::getInstance()->show();
	}

	// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
	if(iom::IMOUT_PLUGIN.compare("empty") == 0)
	{
		iom::IMOUT_PLUGIN = "tiff3D";
	}

	//computing dimensions of volume to be stitched
	//this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	// code for testing
	//uint8 *temp = volume->loadSubvolume_to_UINT8(
	//	10,height-10,10,width-10,10,depth-10,
	//	&channels);

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM9 */)
    { 
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

	if(resolutions == NULL)
	{
            resolutions = new bool;
            *resolutions = true;
            resolutions_size = 1;
	}
	else
            for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
                if(resolutions[i])
                    resolutions_size = std::max(resolutions_size, i+1);

	//computing tiles dimensions at each resolution
	for(int res_i=0; res_i< resolutions_size; res_i++)
	{
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,res_i))  / (float) block_depth  );
		stacks_height[res_i] = new int **[n_stacks_V[res_i]];
		stacks_width[res_i]  = new int **[n_stacks_V[res_i]]; 
		stacks_depth[res_i]  = new int **[n_stacks_V[res_i]]; 
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
			stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
			stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
				{
					stacks_height[res_i][stack_row][stack_col][stack_sli] = 
                        ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
					stacks_width[res_i][stack_row][stack_col][stack_sli] = 
                        ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
					stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
                        ((int)(depth/powInt(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/powInt(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
				}
			}
		}
	}

	// computing channel directory names
	chans_dir = new std::string[channels];
	int n_digits = 1;
	int _channels = channels / 10;	
	while ( _channels ) {
		n_digits++;
		_channels /= 10;
	}
	for ( int c=0; c<channels; c++ ) {
		std::stringstream dir_name;
		dir_name.width(n_digits);
		dir_name.fill('0');
		dir_name << c;
        chans_dir[c] = "/" + (iim::CHANNEL_PREFIX + dir_name.str());
	}

	// computing resolutions directory names
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		//creating volume directory iff current resolution is selected and test mode is disabled
		if(resolutions[res_i] == true) {
            file_path[res_i] << output_path << "/RES("<<height/powInt(2,res_i)
                             << "x" << width/powInt(2,res_i)
                             << "x" << depth/powInt(2,res_i) << ")";
			//if(make_dir(file_path[res_i].str().c_str())!=0) {
            if(!check_and_make_dir(file_path[res_i].str().c_str())) { // HP 130914
                char err_msg[STATIC_STRINGS_SIZE];
				sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                throw IOException(err_msg);
			}

			//if frame_dir not empty must create frame directory
			if ( frame_dir != "" ) {
				file_path[res_i] << "/" << frame_dir << "/";
				if(!check_and_make_dir(file_path[res_i].str().c_str())) 
				{
                    char err_msg[STATIC_STRINGS_SIZE];
					sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
				}
			}

			for ( int c=0; c<channels; c++ ) {
				//creating directory that will contain image data at current resolution
				resolution_dir = file_path[res_i].str() + chans_dir[c];
				//if(make_dir(resolution_dir.c_str())!=0)
                if(!check_and_make_dir(resolution_dir.c_str())) // HP 130914
				{
                    char err_msg[STATIC_STRINGS_SIZE];
					sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", chans_dir[c].c_str());
                    throw IOException(err_msg);
				}
			}
		}
	}

	/* The following check verifies that the numeber of slices in the buffer is not higher than the numbero of slices in a block file
	 * (excluding the last blck in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
	 * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block 
	 * file at that resolution as computed by the above code is:
	 *
	 *                                                B = floor( H / ceil( H/D ) )
	 * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
	 *
	 *                                                  D >= B >= floor(D/2)
	 * since it is:
	 *
	 *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
	 * where alpha<1.
	 */ 

	//ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,resolutions_size-1);
	if ( z_max_res > block_depth/2 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
		throw IOException(err_msg);
	}
	z_ratio=depth/z_max_res;

	//allocated even if not used
	ubuffer = new uint8 *[channels];
	memset(ubuffer,0,channels*sizeof(uint8));
	org_channels = channels; // save for checks

	//slice_start and slice_end of current block depend on the resolution
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		stack_block[res_i] = 0;
		//slice_start[res_i] = this->D0; 
		slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning 
		slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
	}

	// z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
			if ( org_channels != channels ) {
				char err_msg[STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
			}
		
			for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
				// offsets have to be computed taking into account that buffer size along D may be different
				// WARNING: the offset must be of tipe sint64 
				ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
			}
		}
		// WARNING: should check that buffer has been actually allocated

		//updating the progress bar
		if(show_progress_bar)
		{	
			sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1)),(uint32)depth);
                        imProgressBar::getInstance()->update(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
                        imProgressBar::getInstance()->show();
		}

		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			if(show_progress_bar)
			{
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                                imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                imProgressBar::getInstance()->show();
			}

			// check if current block is changed
 			// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
			if ( ((z - this->D0) / powInt(2,i)) > slice_end[i] ) {
				stack_block[i]++;
				slice_start[i] = slice_end[i] + 1;
				slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
			}

			// find abs_pos_z at resolution i
			std::stringstream abs_pos_z;
			abs_pos_z.width(6);
			abs_pos_z.fill('0');
			abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
								- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
                                (powInt(2,i)*slice_start[i]) * volume->getVXL_D());

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / powInt(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0) {
				if ( internal_rep == REAL_INTERNAL_REP )
                    VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
                    VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),channels,method,bytes_chan);
			}
			
			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        imProgressBar::getInstance()->show();
				}

				for ( int c=0; c<channels; c++ ) {

					//storing in 'base_path' the absolute path of the directory that will contain all stacks
					std::stringstream base_path;
					base_path << file_path[i].str().c_str() << chans_dir[c].c_str() << "/";

					//looping on new stacks
					for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
					{
						//incrementing end_height
						end_height = start_height + stacks_height[i][stack_row][0][0]-1; 
							
						//computing V_DIR_path and creating the directory the first time it is needed
						std::stringstream V_DIR_path;
						V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
						if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
						{
                            char err_msg[STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                            throw IOException(err_msg);
						}

						for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
						{
							end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1; 
								
							//computing H_DIR_path and creating the directory the first time it is needed
							std::stringstream H_DIR_path;
							H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
							if ( z==D0 ) {
								if(!check_and_make_dir(H_DIR_path.str().c_str()))
								{
                                    char err_msg[STATIC_STRINGS_SIZE];
									sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                                    throw IOException(err_msg);
								}
								else { // the directory has been created for the first time
									   // initialize block files
									V3DLONG *sz = new V3DLONG[4];
									int datatype;
									char *err_rawfmt;

									sz[0] = stacks_width[i][stack_row][stack_column][0];
									sz[1] = stacks_height[i][stack_row][stack_column][0];
									sz[3] = 1; // single channel files

									if ( internal_rep == REAL_INTERNAL_REP )
										datatype = 4;
									else if ( internal_rep == UINT8_INTERNAL_REP ) {
										if ( saved_img_depth == 16 )
											datatype = 2;
										else if ( saved_img_depth == 8 ) 
											datatype = 1;
										else {
                                            char err_msg[STATIC_STRINGS_SIZE];
											sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
                                            throw IOException(err_msg);
										}
									}
									else {
                                        char err_msg[STATIC_STRINGS_SIZE];
										sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
                                        throw IOException(err_msg);
									}

									int slice_start_temp = 0;
									for ( int j=0; j < n_stacks_D[i]; j++ ) {
										sz[2] = stacks_depth[i][stack_row][stack_column][j];

										std::stringstream abs_pos_z_temp;
										abs_pos_z_temp.width(6);
										abs_pos_z_temp.fill('0');
										abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                            (powInt(2,i)*(slice_start_temp)) * volume->getVXL_D());

										std::stringstream img_path_temp;
										img_path_temp << H_DIR_path.str() << "/" 
													  << this->getMultiresABS_V_string(i,start_height) << "_" 
													  << this->getMultiresABS_H_string(i,start_width) << "_"
													  << abs_pos_z_temp.str();

										//if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
										if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
												( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),sz[0],sz[1],sz[2],sz[3],datatype)) != 0 ) : 
												( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
											char err_msg[STATIC_STRINGS_SIZE];
											sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: error in initializing block file - %s", err_rawfmt);
                                            throw IOException(err_msg);
										};

										slice_start_temp += (int)sz[2];
									}
									delete [] sz;
								}
							}

							//saving HERE

							// 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
 							std::stringstream partial_img_path;
							partial_img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width) << "_";

							int slice_ind = (int)(n_slices_pred - slice_start[i]); 

 							std::stringstream img_path;
							img_path << partial_img_path.str() << abs_pos_z.str();

							/* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
							 * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations, 
							 * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at 
							 * end of the file.
							 * The number of pages of block files of interest can be easily computed as:
							 *
							 *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]] 
							 *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1] 
							 */

							void *fhandle = 0;
							int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
							bool block_changed = false;                                 // true if block is changed executing the next for cycle
							// fhandle = open file corresponding to current block 
							if ( strcmp(saved_img_format,"Tiff3D") == 0 )
								openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle);

							// WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
							for(int buffer_z=0; buffer_z<z_size/(powInt(2,i)); buffer_z++, slice_ind++)
							{
								// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
								if ( ((z - this->D0)  /powInt(2,i)+buffer_z) > slice_end[i] ) { // start a new block along z
 									std::stringstream abs_pos_z_next;
									abs_pos_z_next.width(6);
									abs_pos_z_next.fill('0');
									abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                            (powInt(2,i)*(slice_end[i]+1)) * volume->getVXL_D());
									img_path.str("");
									img_path << partial_img_path.str() << abs_pos_z_next.str();

									slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

									// close(fhandle) i.e. file corresponding to current block 
									if ( strcmp(saved_img_format,"Tiff3D") == 0 )
										closeTiff3DFile(fhandle);
									// fhandle = open file corresponding to next block
									if ( strcmp(saved_img_format,"Tiff3D") == 0 )
										openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle);
									n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
									block_changed = true;
								}

								if ( internal_rep == REAL_INTERNAL_REP )
									VirtualVolume::saveImage_to_Vaa3DRaw(
										slice_ind,
										img_path.str(), 
                                        rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                        (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth
									);
								else {// internal_rep == UINT8_INTERNAL_REP
									if ( strcmp(saved_img_format,"Tiff3D")==0 ) {
										VirtualVolume::saveImage_from_UINT8_to_Tiff3D(
											slice_ind,
											img_path.str(), 
											ubuffer + c,
											1,
											buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
											(int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
											start_height,end_height,start_width,end_width, 
											saved_img_format, saved_img_depth,fhandle,n_pages_block,false);
									}
									else { // can be only Vaa3DRaw
										VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
											slice_ind,
											img_path.str(), 
											ubuffer + c,
											1,
											buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
											(int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
											start_height,end_height,start_width,end_width, 
											saved_img_format, saved_img_depth);
									}
								}
							}

							// close(fhandle) i.e. currently opened file
							if ( strcmp(saved_img_format,"Tiff3D") == 0 )
								closeTiff3DFile(fhandle);

							start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
						}
						start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
					}
				}
			}
		}

		//releasing allocated memory
		if ( internal_rep == REAL_INTERNAL_REP )
			delete rbuffer;
		else // internal_rep == UINT8_INTERNAL_REP
			delete ubuffer[0]; // other buffer pointers are only offsets
	}

	// reloads created volumes to generate .bin file descriptors at all resolutions
	ref_sys reference(axis(1),axis(2),axis(3));
	TiledMCVolume *mcprobe;
	TiledVolume   *tprobe;
	StackedVolume *sprobe;
	sprobe = dynamic_cast<StackedVolume *>(volume);
	if ( sprobe ) {
		reference.first  = sprobe->getAXS_1();
		reference.second = sprobe->getAXS_2();
		reference.third  = sprobe->getAXS_3();
	}
	else {
		tprobe = dynamic_cast<TiledVolume *>(volume);
		if ( tprobe ) {
			reference.first  = tprobe->getAXS_1();
			reference.second = tprobe->getAXS_2();
			reference.third  = tprobe->getAXS_3();
		}
		else {
			mcprobe = dynamic_cast<TiledMCVolume *>(volume);
			if ( mcprobe ) {
				reference.first  = mcprobe->getAXS_1();
				reference.second = mcprobe->getAXS_2();
				reference.third  = mcprobe->getAXS_3();
			}
		}
	}
	for(int res_i=0; res_i< resolutions_size; res_i++) {

		if(resolutions[res_i]) {

			for ( int c=0; c<channels; c++ ) {
				resolution_dir = file_path[res_i].str() + chans_dir[c];

				//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
				//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
				//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
				//system.
				TiledVolume temp_vol(resolution_dir.c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

	//          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
	//                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
			}

			TiledMCVolume temp_mc_vol(file_path[res_i].str().c_str(),reference,
					volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

		}
	}


	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				delete[] stacks_height[res_i][stack_row][stack_col];
				delete[] stacks_width [res_i][stack_row][stack_col];
				delete[] stacks_depth [res_i][stack_row][stack_col];
			}
			delete[] stacks_height[res_i][stack_row];
			delete[] stacks_width [res_i][stack_row];
			delete[] stacks_depth [res_i][stack_row];
		}
		delete[] stacks_height[res_i];
		delete[] stacks_width[res_i]; 
		delete[] stacks_depth[res_i]; 
	}

	delete[] chans_dir;
}

#endif


void VolumeConverter::generateTilesBDV_HDF5 ( std::string output_path, bool* resolutions, 
				int block_height, int block_width, int block_depth, int method, 
				bool show_progress_bar, const char* saved_img_format, 
                int saved_img_depth, std::string frame_dir )	throw (IOException)
{
    printf("in VolumeConverter::generateTilesBDV_HDF5(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

	if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
		saved_img_depth = volume->getBYTESxCHAN() * 8;
		
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
	}

	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
	sint64 z_ratio, z_max_res;
	int resolutions_size = 0;

	void *file_descr;
	sint64 *hyperslab_descr = new sint64[4*3]; // four 3-valued parameters: [ start(offset), stride count(size), block ]
	memset(hyperslab_descr,0,4*3*sizeof(sint64));
	sint64 *buf_dims = new sint64[3];  // dimensions of the buffer in which the subvolume is stored at a given resolution
	memset(buf_dims,0,3*sizeof(sint64));

	if ( volume == 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesBDV_HDF5: undefined source volume");
        throw IOException(err_msg);
	}

	// HDF5 crea il file HDF5 se non esiste altrimenti determina i setup e i time point gia' presenti
	BDV_HDF5init(output_path,file_descr,volume->getBYTESxCHAN());

	//initializing the progress bar
	char progressBarMsg[200];
	if(show_progress_bar)
	{
       imProgressBar::getInstance()->start("Multiresolution tile generation");
       imProgressBar::getInstance()->update(0,"Initializing...");
       imProgressBar::getInstance()->show();
	}

	// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
	if(iom::IMOUT_PLUGIN.compare("empty") == 0)
	{
		iom::IMOUT_PLUGIN = "tiff3D";
	}

	//computing dimensions of volume to be stitched
	//this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	// code for testing
	//uint8 *temp = volume->loadSubvolume_to_UINT8(
	//	10,height-10,10,width-10,10,depth-10,
	//	&channels);

	//these parameters are used here for chunk dimensions; the default values should be passed without changes to BDV_HDF5 routines
    //block_height = (block_height == -1 ? (int)height : block_height);
    //block_width  = (block_width  == -1 ? (int)width  : block_width);
    //block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    //if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM9 */)
    //{ 
    //    char err_msg[STATIC_STRINGS_SIZE];
    //    sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
    //    throw IOException(err_msg);
    //}

	if(resolutions == NULL)
	{
            resolutions = new bool;
            *resolutions = true;
            resolutions_size = 1;
	}
	else
            for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
                if(resolutions[i])
                    resolutions_size = std::max(resolutions_size, i+1);

	BDV_HDF5addSetups(file_descr,height,width,depth,volume->getVXL_V(),volume->getVXL_H(),volume->getVXL_D(),
										resolutions,resolutions_size,channels,block_height,block_width,block_depth);

	BDV_HDF5addTimepoint(file_descr);

	//BDV_HDF5close(file_descr);

	//ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,resolutions_size-1);

	// the following check does not make sense for Fiji_HDF5 format
	//if ( z_max_res > block_depth/2 ) {
	//	char err_msg[STATIC_STRINGS_SIZE];
	//	sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
	//	throw IOException(err_msg);
	//}
	z_ratio=depth/z_max_res;

	//allocated even if not used
	ubuffer = new uint8 *[channels];
	memset(ubuffer,0,channels*sizeof(uint8));
	org_channels = channels; // save for checks

	// z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
			if ( org_channels != channels ) {
				char err_msg[STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
			}
		
			for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
				// offsets have to be computed taking into account that buffer size along D may be different
				// WARNING: the offset must be of tipe sint64 
				ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
			}
		}
		// WARNING: should check that buffer has been actually allocated

		//updating the progress bar
		if(show_progress_bar)
		{	
			sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1)),(uint32)depth);
                        imProgressBar::getInstance()->update(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
                        imProgressBar::getInstance()->show();
		}

		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			// HDF5 crea i gruppi relativi a questa risoluzione in ciascun setup del time point corrente

			if(show_progress_bar)
			{
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                                imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                imProgressBar::getInstance()->show();
			}

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0) {
				if ( internal_rep == REAL_INTERNAL_REP )
                    VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
                    VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),channels,method,bytes_chan);
			}
			
			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        imProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        imProgressBar::getInstance()->show();
				}

				//std::stringstream  res_name;
				//res_name << i;

				for ( int c=0; c<channels; c++ ) {

					//storing in 'base_path' the absolute path of the directory that will contain all stacks
					//std::stringstream base_path;
					// ELIMINARE? base_path << file_path[i].str().c_str() << chans_dir[c].c_str() << "/";

					// HDF5 scrive il canale corrente nel buffer nel gruppo corrispondente al time point e alla risoluzione correnti 
					if ( internal_rep == REAL_INTERNAL_REP )
						throw iim::IOException(iim::strprintf("updating already existing files not supported yet").c_str(),__iim__current__function__);
					else { // internal_rep == UINT8_INTERNAL_REP
						buf_dims[1] = height/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
						buf_dims[2] = width/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
						buf_dims[0] = z_size/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
						// start
						hyperslab_descr[0] = 0; // [0][0]
						hyperslab_descr[1] = 0; // [0][1]
						hyperslab_descr[2] = 0; // [0][2]
						// stride
						hyperslab_descr[3] = 1;  // [1][0]
						hyperslab_descr[4] = 1;  // [1][1]
						hyperslab_descr[5] = 1;  // [1][2]
						// count
						hyperslab_descr[6] = buf_dims[0]; //height/(powInt(2,i)); // [2][0]
						hyperslab_descr[7] = buf_dims[1]; //width/(powInt(2,i));  // [2][1]
						hyperslab_descr[8] = buf_dims[2]; //z_size/(powInt(2,i)); // [2][2]
						// block
						hyperslab_descr[9]  = 1; // [3][0]
						hyperslab_descr[10] = 1; // [3][1]
						hyperslab_descr[11] = 1; // [3][2]
						BDV_HDF5writeHyperslab(file_descr,ubuffer[c],buf_dims,hyperslab_descr,i,c);
					}

				}
			}
		}

		//releasing allocated memory
		if ( internal_rep == REAL_INTERNAL_REP )
			delete rbuffer;
		else // internal_rep == UINT8_INTERNAL_REP
			delete ubuffer[0]; // other buffer pointers are only offsets
	}

	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
 //   for(int res_i=0; res_i< resolutions_size; res_i++)
	//{
	//	for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
	//	{
	//		for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
	//		{
	//			delete[] stacks_height[res_i][stack_row][stack_col];
	//			delete[] stacks_width [res_i][stack_row][stack_col];
	//			delete[] stacks_depth [res_i][stack_row][stack_col];
	//		}
	//		delete[] stacks_height[res_i][stack_row];
	//		delete[] stacks_width [res_i][stack_row];
	//		delete[] stacks_depth [res_i][stack_row];
	//	}
	//	delete[] stacks_height[res_i];
	//	delete[] stacks_width[res_i]; 
	//	delete[] stacks_depth[res_i]; 
	//}

	//delete[] chans_dir;

	delete hyperslab_descr;
	delete buf_dims;

	BDV_HDF5close(file_descr);

}


// unified access point for volume conversion (@ADDED by Alessandro on 2014-02-24)
void VolumeConverter::convertTo(
    std::string output_path,                        // path where to save the converted volume
    std::string output_format,                      // format of the converted volume (see IM_config.h)
    int output_bitdepth /*= iim::NUL_IMG_DEPTH*/,   // output image bitdepth
    bool isTimeSeries /*= false*/,                  // whether the volume is a time series
    bool *resolutions /*= 0*/,                      // array of resolutions
    int block_height /*= -1*/,                      // tile's height (for tiled formats)
    int block_width  /*= -1*/,                      // tile's width  (for tiled formats)
    int block_depth  /*= -1*/,                      // tile's depth  (for tiled formats)
    int method /*=HALVE_BY_MEAN*/                   // downsampling method
) throw (iim::IOException)
{
    printf("in VolumeConverter::convertTo(output_path = \"%s\", output_format = \"%s\", output_bitdepth = %d, isTimeSeries = %s, resolutions = ",
           output_path.c_str(), output_format.c_str(), output_bitdepth, isTimeSeries ? "true" : "false");
    for(int i=0; i< TMITREE_MAX_HEIGHT && resolutions; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d)\n",
           block_height, block_width, block_depth, method);

    if(isTimeSeries)
    {
        for(int t=0; t<volume->getDIM_T(); t++)
        {
            imProgressBar::instance()->setMessage(1, strprintf("Converting time frame %d/%d", t+1, volume->getDIM_T()).c_str());
            volume->setActiveFrames(t,t);
            std::string frame_dir = iim::TIME_FRAME_PREFIX + strprintf("%06d", t);
            if(output_format.compare(iim::STACKED_FORMAT) == 0)
                generateTiles(output_path, resolutions, block_height, block_width, method, true, iim::DEF_IMG_FORMAT.c_str(), output_bitdepth, frame_dir);
            else if(output_format.compare(iim::TILED_FORMAT) == 0)
                generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, true, "raw", output_bitdepth, frame_dir);
            else if(output_format.compare(iim::TILED_MC_FORMAT) == 0)
                generateTilesVaa3DRawMC(output_path, resolutions, block_height, block_width, block_depth, method, true, "raw", output_bitdepth, frame_dir);
            else if(output_format.compare(iim::TILED_TIF3D_FORMAT) == 0)
                generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, true, "Tiff3D", output_bitdepth, frame_dir);
            else if(output_format.compare(iim::TILED_MC_TIF3D_FORMAT) == 0)
                generateTilesVaa3DRawMC(output_path, resolutions, block_height, block_width, block_depth, method, true, "Tiff3D", output_bitdepth, frame_dir);
            else
                throw iim::IOException(strprintf("Output format \"%s\" not supported", output_format.c_str()).c_str());
        }
        imProgressBar::instance()->reset();
    }
    else
    {
        if(output_format.compare(iim::STACKED_FORMAT) == 0)
            generateTiles(output_path, resolutions, block_height, block_width, method, true, iim::DEF_IMG_FORMAT.c_str(), output_bitdepth);
        else if(output_format.compare(iim::TILED_FORMAT) == 0)
            generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, true, "raw", output_bitdepth);
        else if(output_format.compare(iim::TILED_MC_FORMAT) == 0)
            generateTilesVaa3DRawMC(output_path, resolutions, block_height, block_width, block_depth, method, true, "raw", output_bitdepth);
        else if(output_format.compare(iim::TILED_TIF3D_FORMAT) == 0)
            generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, true, "Tiff3D", output_bitdepth);
        else if(output_format.compare(iim::TILED_MC_TIF3D_FORMAT) == 0)
            generateTilesVaa3DRawMC(output_path, resolutions, block_height, block_width, block_depth, method, true, "Tiff3D", output_bitdepth);
        else if(output_format.compare(iim::BDV_HDF5_FORMAT) == 0)
            generateTilesBDV_HDF5(output_path,resolutions, block_height,block_width,block_depth,method, true,"Tiff3D",output_bitdepth);
        else
            throw iim::IOException(strprintf("Output format \"%s\" not supported", output_format.c_str()).c_str());
    }
}


