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

#ifndef CIMPORT_H
#define CIMPORT_H

#include "../v3d/v3d_compile_constraints.h" //by PHC 20200201

#ifdef USE_Qt5
#include <QtCore>
#endif
#include <QThread>
#include <string>
#include <vector>
#include "VirtualVolume.h"
#include "StackedVolume.h"
#include "TiledVolume.h"
#include "m_CPlugin.h"
#include "m_CSettings.h"

using namespace std;

class teramanager::CImport : public QThread
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling the static method "istance(...)"
        **********************************************************************************/
        static CImport* uniqueInstance;
        CImport() : QThread()
        {
            /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
            vmapData = 0;
            HDF5_descr = (void *) 0;
            reset();
        }

        // automatically called when current thread is started
        void run();


        /* We call "volume map" a binary file where to store the image content that has to be loaded and
         * displayed immediately after a volume is imported. Usually, this corresponds to a low-resolution
         * version of the whole volume, hence the description "volume map". */

        // input members
        string path;                                // path of the volume to be imported
        bool reimport;                              /* (optional) true if the volume has to be reimported */
        bool regenerateVMap;                        /* (optional) true if volume map has to be regenerated */
        iim::axis AXS_1, AXS_2, AXS_3;              /* (optional) spatial reference system of the volume to be imported */
        float VXL_1, VXL_2, VXL_3;                  /* (optional) voxel dimensions of the volume to be imported */
        string format;                              /* (optional) format of the volume to be imported */
        bool isTimeSeries;                          /* (optional) whether the volume to be imported is a time series */
        int vmapYDimMax, vmapXDimMax, vmapZDimMax, vmapCDimMax, vmapTDimMax; //volume map maximum dimensions

        // output members
        vector<iim::VirtualVolume*> volumes;        // stores the volumes at the different resolutions
        itm::uint8* vmapData;                       //volume map data
        itm::uint32 vmapYDim, vmapXDim, vmapZDim, vmapCDim, vmapTDim; //volume map actualdimensions

		// should be released only when the multi resolution image is closed
		void *HDF5_descr;   

        // other members
        QElapsedTimer timerIO;                      //for time measuring


    public:

        friend class PMain;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CImport* instance()
        {
            if (!uniqueInstance)
                uniqueInstance = new CImport();
            return uniqueInstance;
        }
        static void uninstance();
        ~CImport();

        // GET methods
        string getPath(){return path;}
        itm::uint8* getVMapRawData(){return vmapData;}
        itm::uint32 getVMapXDim(){return vmapXDim;}
        itm::uint32 getVMapYDim(){return vmapYDim;}
        itm::uint32 getVMapZDim(){return vmapZDim;}
        itm::uint32 getVMapCDim(){return vmapCDim;}
        itm::uint32 getVMapTDim(){return vmapTDim;}
        int getTDim(){
            if(!volumes.empty())
                return volumes[0]->getDIM_T();
            else
                return 0;
        }
        bool is5D(){return getTDim() > 1;}
        int getVMapResIndex()
        {
            for(size_t k=0; k<volumes.size(); k++)
                if(volumes[k]->getDIM_D() == vmapZDim)
                    return static_cast<int>(k);
            return -1;
        }
        bool isEmpty(){return volumes.size() == 0;}
        iim::VirtualVolume* getHighestResVolume(){if(!volumes.empty()) return volumes.back(); else return 0;}
        iim::VirtualVolume* getVolume(int resolutionIdx)
        {
            if(resolutionIdx < static_cast<int>(volumes.size())) return volumes[resolutionIdx];
            else return 0;
        }
        int getResolutions(){return static_cast<int>(volumes.size());}
        float getResRatio(int resIndex)
        {
            if(!volumes.empty() && resIndex < volumes.size())
                return (volumes.back()->getDIM_V()-1.0f)/(volumes[resIndex]->getDIM_V()-1.0f);
            else return 1.0f;
        }

        // SET methods
        void setPath(string new_path){path = new_path;}
        void setAxes(string axs1, string axs2, string axs3);
        void setVoxels(float vxl1, float vxl2, float vxl3);
        void setReimport(bool _reimport){reimport = _reimport;}
        void setRegenerateVolumeMap(bool _regenerateVolMap){regenerateVMap = _regenerateVolMap;}
        void setFormat(string _format){format = _format;}
        void setTimeSeries(bool _isTimeSeries){isTimeSeries = _isTimeSeries;}

        // reset method
        void reset()
        {
            /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

            path="";
            reimport=false;
            regenerateVMap = false;
            AXS_1=AXS_2=AXS_3=iim::axis_invalid;
            VXL_1=VXL_2=VXL_3=0.0f;
            format = "";
            isTimeSeries = false;
            for(size_t i=0; i<volumes.size(); i++)
                delete volumes[i];
            volumes.clear();
//            if(vmapData)
//                delete[] vmapData;        // vmap MUST NOT be deallocated from TeraFly, since it is handled directly by Vaa3D
            vmapData = 0;
            vmapXDim = vmapYDim = vmapZDim = vmapTDim = vmapCDim = -1;
            updateMaxDims();
        }

        void updateMaxDims(){
            vmapYDimMax = CSettings::instance()->getVOIdimV();
            vmapXDimMax = CSettings::instance()->getVOIdimH();
            vmapZDimMax = CSettings::instance()->getVOIdimD();
            vmapTDimMax = CSettings::instance()->getVOIdimT();
            vmapCDimMax = 3;
        }

        // returns true if
        // 1) the volume map does not exist OR
        // 2) it is not compatible with the current version OR
        // 3) contains a number of 'T' frames with T < vmapTDimMax
        bool hasVolumeMapToBeRegenerated(std::string vmapFilepath, std::string min_required_version) throw (itm::RuntimeException);


    signals:

        /*********************************************************************************
        * Carries the outcome of the operation associated to this thread.
        **********************************************************************************/
        void sendOperationOutcome(itm::RuntimeException* ex, qint64 elapsed_time = 0);
};

#endif // CIMPORT_H
