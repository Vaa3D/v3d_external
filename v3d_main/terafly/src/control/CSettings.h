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

#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "CPlugin.h"
#include <vector>
#include <algorithm>
#include <map>

class terafly::CSettings
{
    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CSettings* uniqueInstance;
        CSettings()
        {
             /**/tf::debug(tf::LEV1, 0, __itm__current__function__);
            loadDefaultSettings();
            readSettings();
        }

        //TeraFly members
        std::string volumePathLRU;
        std::list< std::pair<std::string, std::string> > recentImages;    // <path, format> pairs
		std::string annotationPathLRU;
        int VOIdimV;
        int VOIdimH;
        int VOIdimD;
        int VOIdimT;
        int traslX;             //traslation percentage with respect to the actual VOI along X axis
        int traslY;             //traslation percentage with respect to the actual VOI along Y axis
        int traslZ;             //traslation percentage with respect to the actual VOI along Z axis
        int traslT;             //traslation percentage with respect to the actual VOI along T axis
        bool annotationSpaceUnlimited;
        int annotationCurvesDims;
        bool annotationCurvesAspectTube;
        int annotationVirtualMargin;
        int annotationMarkerSize;
        bool previewMode;
        int pyramidResamplingFactor;
        int viewerHeight;
        int viewerWidth;
        float ramLimitGB;
        int vpEmptyVizMethod;
        int vpEmptyVizIntensity;
        float vpEmptyVizSaltPepperPercentage;
        int vpFetchMethod;
        int vpFetchNBlocks;
        int vpBlockFormatIndex;
        bool vpRefillAuto;
        int vpRefillCoverage;
        int vpRefillStopCondition;
        bool vpCacheHighestRes;
        bool vpFreezeHighestRes;
        double voxelsizeX, voxelsizeY, voxelsizeZ;
        int bitsRemap;
        int bitsConversion;
        int zoomOutMethod;
        std::string recentlyUsedPath;

        //TeraConverter members
        std::string volumeConverterInputPathLRU;
        std::string volumeConverterOutputPathLRU;
        std::string volumeConverterInputFormatLRU;
        std::string volumeConverterOutputFormatLRU;
        int volumeConverterStacksWidthLRU;
        int volumeConverterStacksHeightLRU;
        int volumeConverterStacksDepthLRU;
        bool volumeConverterTimeSeries;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CSettings* instance()
        {
            if (uniqueInstance == 0)
                uniqueInstance = new CSettings();
            return uniqueInstance;
        }
        static void uninstance();
        ~CSettings();

        //GET and SET methods for TeraFly
        std::string getVolumePathLRU(){return volumePathLRU;}
        std::list< std::pair<std::string, std::string> >& getRecentImages(){return recentImages;}
        std::string getAnnotationPathLRU(){return annotationPathLRU;}
        int getVOIdimV(){return VOIdimV;}
        int getVOIdimH(){return VOIdimH;}
        int getVOIdimD(){return VOIdimD;}
        int getVOIdimT(){return VOIdimT;}
        int getTraslX(){return traslX;}
        int getTraslY(){return traslY;}
        int getTraslZ(){return traslZ;}
        int getTraslT(){return traslT;}
        double getVoxelSizeX(){return voxelsizeX;}
        double getVoxelSizeY(){return voxelsizeY;}
        double getVoxelSizeZ(){return voxelsizeZ;}
        bool getAnnotationSpaceUnlimited(){return annotationSpaceUnlimited;}
        int getAnnotationCurvesDims(){return annotationCurvesDims;}
        bool getAnnotationCurvesAspectTube(){return annotationCurvesAspectTube;}
        int getAnnotationVirtualMargin(){return annotationVirtualMargin;}
        int getAnnotationMarkerSize(){return annotationMarkerSize;}
        bool getPreviewMode(){return previewMode;}
        int getPyramidResamplingFactor(){return pyramidResamplingFactor;}
        int getViewerHeight(){return viewerHeight;}
        int getViewerWidth(){return viewerWidth;}
        float getRamLimitGB(){return ramLimitGB;}
        int getVpEmptyVizIntensity(){return vpEmptyVizIntensity;}
        int getVpEmptyVizMethod(){return vpEmptyVizMethod;}
        float getVpEmptyVizSaltPepperPercentage(){return vpEmptyVizSaltPepperPercentage;}
        int getVpFetchMethod(){return vpFetchMethod;}
        int getVpFetchNBlocks(){return vpFetchNBlocks;}
        int getVpBlockFormatIndex(){return vpBlockFormatIndex;}
        int getVpRefillAuto(){return vpRefillAuto;}
        int getVpRefillCoverage(){return vpRefillCoverage;}
        int getVpRefillStopCondition(){return vpRefillStopCondition;}
        bool getVpCacheHighestRes(){return vpCacheHighestRes;}
        bool getVpFreezeHighestRes(){return vpFreezeHighestRes;}
        int getBitsRemap(){return bitsRemap;}
        int getBitsConversion(){return bitsConversion;}
        int getZoomOutMethod(){return zoomOutMethod;}
        std::string getRecentlyUsedPath() {return recentlyUsedPath;}

        void setVolumePathLRU(std::string _volumePathLRU)
        {
            /**/tf::debug(tf::LEV_MAX, strprintf("_volumePathLRU = \"%s\"", _volumePathLRU.c_str()).c_str(), __itm__current__function__);
            volumePathLRU = _volumePathLRU;
        }
        void addRecentImage(std::string path, std::string format)
        {
            /**/tf::debug(tf::LEV_MAX, strprintf("path = \"%s\", format = \"%s\"", path.c_str(), format.c_str()).c_str(), __itm__current__function__);
            std::pair<std::string, std::string> newval = std::pair<std::string, std::string>(path, format);
            if(std::find(recentImages.begin(), recentImages.end(), newval) != recentImages.end())
                recentImages.erase(std::find(recentImages.begin(), recentImages.end(), newval));
            if(recentImages.size() > 15)
                recentImages.pop_back();
            recentImages.push_front(newval);
        }
        void clearRecentImages(){
            recentImages.clear();
            writeSettings();
        }

        void setAnnotationPathLRU(std::string _annotationPathLRU){annotationPathLRU = _annotationPathLRU; writeSettings();}
        void setVOIdimV(int _VOIdimV){VOIdimV = _VOIdimV; writeSettings();}
        void setVOIdimH(int _VOIdimH){VOIdimH = _VOIdimH; writeSettings();}
        void setVOIdimD(int _VOIdimD){VOIdimD = _VOIdimD; writeSettings();}
        void setVOIdimT(int _VOIdimT){VOIdimT = _VOIdimT; writeSettings();}
        void setVoxelSizeX(double x){voxelsizeX = x; writeSettings();}
        void setVoxelSizeY(double y){voxelsizeY = y; writeSettings();}
        void setVoxelSizeZ(double z){voxelsizeZ = z; writeSettings();}
        void setTraslX(int _traslX){traslX = _traslX; writeSettings();}
        void setTraslY(int _traslY){traslY = _traslY; writeSettings();}
        void setTraslZ(int _traslZ){traslZ = _traslZ; writeSettings();}
        void setTraslT(int _traslT){traslT = _traslT; writeSettings();}
        void setAnnotationSpaceUnlimited(bool _unl){annotationSpaceUnlimited = _unl; writeSettings();}
        void setAnnotationCurvesDims(int newval){annotationCurvesDims = newval; writeSettings();}
        void setAnnotationCurvesAspectTube(bool newval){annotationCurvesAspectTube = newval; writeSettings();}
        void setAnnotationVirtualMargin(int newval){annotationVirtualMargin = newval; writeSettings();}
        void setAnnotationMarkerSize(int newval){annotationMarkerSize = newval; writeSettings();}
        void setPreviewMode(bool newval){previewMode = newval; writeSettings();}
        void setPyramidResamplingFactor(int newval){pyramidResamplingFactor = newval; writeSettings();}
        void setViewerHeight(int newval){viewerHeight = newval; writeSettings();}
        void setViewerWidth(int newval){viewerWidth = newval; writeSettings();}
        void setRamLimitGB(float newval){ramLimitGB = newval; writeSettings();}
        void setVpEmptyVizMethod(int newval){vpEmptyVizMethod = newval; writeSettings();}
        void setVpEmptyVizIntensity(int newval){vpEmptyVizIntensity = newval; writeSettings();}
        void setVpEmptyVizSaltPepperPercentage(float newval){vpEmptyVizSaltPepperPercentage = newval; writeSettings();}
        void setVpFetchMethod(int newval){vpFetchMethod = newval; writeSettings();}
        void setVpFetchNBlocks(int newval){vpFetchNBlocks = newval; writeSettings();}
        void setVpBlockFormatIndex(int newval){vpBlockFormatIndex = newval; writeSettings();}
        void setVpRefillAuto(bool newval){vpRefillAuto = newval; writeSettings();}
        void setVpRefillCoverage(int newval){vpRefillCoverage = newval; writeSettings();}
        void setVpRefillStopCondition(int newval){vpRefillStopCondition = newval; writeSettings();}
        void setVpCacheHighestRes(bool newval){vpCacheHighestRes = newval; writeSettings();}
        void setVpFreezeHighestRes(bool newval){vpFreezeHighestRes = newval; writeSettings();}
        void setBitsRemap(int newval){bitsRemap = newval; writeSettings();}
        void setBitsConversion(int newval){bitsConversion = newval; writeSettings();}
        void setZoomOutMethod(int method){zoomOutMethod = method; writeSettings();}
        void setRecentlyUsedPath(std::string path){recentlyUsedPath = path; writeSettings();}


        //GET and SET methods for TeraConverter
        std::string getVCInputPath(){return volumeConverterInputPathLRU;}
        std::string getVCOutputPath(){return volumeConverterOutputPathLRU;}
        std::string getVCInputFormat(){return volumeConverterInputFormatLRU;}
        std::string getVCOutputFormat(){return volumeConverterOutputFormatLRU;}
        int getVCStacksWidth(){return volumeConverterStacksWidthLRU;}
        int getVCStacksHeight(){return volumeConverterStacksHeightLRU;}
        int getVCStacksDepth(){return volumeConverterStacksDepthLRU;}
        bool getVCTimeSeries(){return volumeConverterTimeSeries;}
        void setVCInputPath(std::string newval){volumeConverterInputPathLRU = newval; writeSettings();}
        void setVCOutputPath(std::string newval){volumeConverterOutputPathLRU = newval; writeSettings();}
        void setVCInputFormat(std::string newval){volumeConverterInputFormatLRU = newval; writeSettings();}
        void setVCOutputFormat(std::string newval){volumeConverterOutputFormatLRU = newval; writeSettings();}
        void setVCStacksWidth(int newval){volumeConverterStacksWidthLRU = newval; writeSettings();}
        void setVCStacksHeight(int newval){volumeConverterStacksHeightLRU = newval; writeSettings();}
        void setVCStacksDepth(int newval){volumeConverterStacksDepthLRU = newval; writeSettings();}
        void setVCTimeSeries(bool newval){volumeConverterTimeSeries = newval; writeSettings();}

        //save and restore application settings
        void writeSettings();
        void readSettings();

        //load default settings
        void loadDefaultSettings();
};

#endif // CSETTINGS_H
