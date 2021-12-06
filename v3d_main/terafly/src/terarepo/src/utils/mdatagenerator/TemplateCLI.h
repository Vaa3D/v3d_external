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
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", submitted for publication, 2012.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#ifndef _TEMPLATE_COMMAND_LINE_INTERFACE_H
#define _TEMPLATE_COMMAND_LINE_INTERFACE_H

#include <string>
#include "iomanager.config.h"
#include "IM_config.h"
#include "StackedVolume.h"

using namespace std;

class TemplateCLI
{
	public:

		// switch parameters
		bool overwrite_mdata;						// overwrite data (mdata.bin file)
		bool update_mdata;							// update data (only tiled 4D, cmap.bin file)

		// other parameters
		// int/float/double/string XXXX;	// description
		string root_dir;  
		iim::axis axis_V;
		iim::axis axis_H;
		iim::axis axis_D;
		float vxlsz_V;
		float vxlsz_H;
		float vxlsz_D;
		string src_format;

		
		// root directory of the volume

		//constructor - deconstructor
		TemplateCLI(void);					//set default params
		~TemplateCLI(void){};

		//reads options and parameters from command line
		void readParams(int argc, char** argv) throw (iom::exception);

		//checks parameters correctness
		void checkParams() throw (iom::exception);

		//returns help text
		string getHelpText();

		//print all arguments
		void print();
};

#endif /* _TERASTITCHER_COMMAND_LINE_INTERFACE_H */


