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
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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

#ifndef _TERASTITCHER_GUI_H_
#define _TERASTITCHER_GUI_H_

//#define TSP_DEBUG  //debug mode

#include <QtGui>
#include "IM_config.h"
#include "iomanager.config.h"
#include "config.h"

//defining TeraStitcher plugin interface
namespace terastitcher
{
    class PTeraStitcher;        //main presentation class: it contains the main frame with tabs, progress bar and start/stop buttons
    class PTabImport;           //presentation class for the "Import" step of the Stitching process
    class CImportUnstitched;    //control class for the "Import" step, which is performed in a separate thread since it can be time-consuming
    class CPreview;             //control class for the preview feature, which is performed in a separate thread since it can be time-consuming
    class PTabDisplComp;        //presentation class for the "Pairwise Displacement Computation" step
    class CDisplComp;           //control class for the "Pairwise Displacement Computation" step, which is performed in a separate thread since it can be time-consuming
    class PTabDisplProj;        //presentation class for the "Displacement Projection" step
    class PTabDisplThresh;      //presentation class for the "Displacement Thresholding" step
    class PTabPlaceTiles;       //presentation class for the "Optimal tiles placement" step
    class PTabMergeTiles;       //presentation class for the "Merging tiles" step
    class CMergeTiles;          //control class for the "Merging tiles" step, which is performed in a separate thread since it can be time-consuming
    class QMyTabWidget;         //Qt-class which slightly modifies the QTabWidget class
    class QHelpBox;             //customized Qt label
    class QPrefixSuffixValidator;
    class QPrefixSuffixLineEdit;
}

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QMenuBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabBar>
#include <QTabWidget>
#include <QApplication>
#include <QStackedLayout>
#include <QDesktopWidget>

#endif // _TERASTITCHER_GUI_H_

