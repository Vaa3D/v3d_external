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
* 2018-07-05. Giulio.     @ADDED stings describing remapping and conversion algorithms to 8 bits values
*/


#include "IM_config.h"

namespace IconImageManager
{

    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    std::string VERSION = "1.2.0";          // version of current module
    int DEBUG = NO_DEBUG;					//debug level
    bool DEBUG_TO_FILE = false;             //whether debug messages should be printed on the screen or to a file (default: screen)
    std::string DEBUG_FILE_PATH = "/home/alex/Scrivania/iim_debug.log";   //filepath where to save debug information
    bool ADD_NOISE_TO_TIME_SERIES = false;	// whether to mark individual frames of a time series with increasing gaussian noise
    int CHANNEL_SELECTION = ALL;			// channel to be loaded (default is ALL)
    /*-------------------------------------------------------------------------------------------------------------------------*/

    /**************************
    * TRNSFORM ALGORITHMS IDs *
    ***************************
    ---------------------------------------------------------------------------------------------------------------------------*/
	const char *remap_algorithms_strings[] = {
		"none",
		"rescale bits 0-5",
		"local max",
	};

	const int remap_algorithms_IDs[] = {
		REMAP_NULL,
		REMAP_6_TO_8_BITS,
		REMAP_LOCAL_MAX 
	};
	
	const char *conversion_algorithms_strings[] = {
		"rescale bits 0-15",
		"local max",
		"rescale bits 4-11"
	};

	const int conversion_algorithms_IDs[] = {
		DEPTH_CONVERSION_LINEAR,
		DEPTH_CONVERSION_LOCAL_MAX,
		DEPTH_CONVERSION_4_11
	};
	
	const char *mixed_algorithms_strings[] = {
		"local maximum-based remap/conversion"
	};

	const int mixed_algorithms_IDs[] = {
		REMAP_DEPTH_COVERSION_LOCAL_MAX	};
    /*-------------------------------------------------------------------------------------------------------------------------*/
}


