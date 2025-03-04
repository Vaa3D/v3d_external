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
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
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

/******************
*    CHANGELOG    *
*******************
* 2018-04-14. Giulio.     @CREATED global optimization algorithm based on Linear Quadtratic Programming + heuristics (S_FATPM_LQP_HE)
*/


#ifndef _TILE_PLACEMENT_ALGORITHM_LINEAR_QUADRATIC_PROGRAMMING_H
#define _TILE_PLACEMENT_ALGORITHM_LINEAR_QUADRATIC_PROGRAMMING_H

#include "TPAlgo.h"

class TPAlgoLQP : public TPAlgo
{
	private:

		TPAlgoLQP(void){};			//default constructor is inhibited

	protected:

		//int TYPE;					//INHERITED from TPAlgo
		//StackedVolume* volume;	//INHERITED from TPAlgo

	public:

		TPAlgoLQP(volumemanager::VirtualVolume * _volume);
		~TPAlgoLQP(void){};

		/*************************************************************************************************************
		* Finds the optimal tile placement on the <volume> object member via Linear Quadratic Programming + heuristics.
		* PROs: 
		* CONs: 
		**************************************************************************************************************/
        void execute() ;
};

#endif /* _TILE_PLACEMENT_ALGORITHM_LINEAR_QUADRATIC_PROGRAMMING_H */
