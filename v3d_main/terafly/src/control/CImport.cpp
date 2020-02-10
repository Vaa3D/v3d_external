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

#include "../presentation/PMain.h"

#include "../presentation/PDialogImport.h"
//#include "../presentation/PMain.h"
#include "../presentation/PLog.h"
#include "CImport.h"
#include "CPlugin.h"
#include <sstream>
#include <limits>
#include <algorithm>
#include "StackedVolume.h"
#include "TiledVolume.h"
#include "TiledMCVolume.h"
#include "HDF5Mngr.h" 
#include "iomanager.config.h"
#include "VirtualPyramid.h"

using namespace terafly;
using namespace iim;

CImport* CImport::uniqueInstance = 0;
bool sortVolumesAscendingSize (VirtualVolume* i,VirtualVolume* j) { return (i->getMVoxels() < j->getMVoxels()); }

void CImport::uninstance()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

CImport::~CImport()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    for(int k=0; k<volumes.size(); k++)
        if(volumes[k])
            delete volumes[k];
    
    if ( HDF5_descr )
    	BDV_HDF5close(HDF5_descr); // HDF5 file is closed after the volumes are released

    /**/tf::debug(tf::LEV1, "object successfully DESTROYED", __itm__current__function__);
}

//SET methods
void CImport::setAxes(string axs1, string axs2, string axs3)
{
    /**/tf::debug(tf::LEV1, strprintf("axes = (%s, %s, %s)", axs1.c_str(), axs2.c_str(), axs3.c_str()).c_str(), __itm__current__function__);

    if(     axs1.compare("Y")==0)
        AXS_1 = axis(1);
    else if(axs1.compare("-Y")==0)
        AXS_1 = axis(-1);
    else if(axs1.compare("X")==0)
        AXS_1 = axis(2);
    else if(axs1.compare("-X")==0)
        AXS_1 = axis(-2);
    else if(axs1.compare("Z")==0)
        AXS_1 = axis(3);
    else if(axs1.compare("-Z")==0)
        AXS_1 = axis(-3);

    if(     axs2.compare("Y")==0)
        AXS_2 = axis(1);
    else if(axs2.compare("-Y")==0)
        AXS_2 = axis(-1);
    else if(axs2.compare("X")==0)
        AXS_2 = axis(2);
    else if(axs2.compare("-X")==0)
        AXS_2 = axis(-2);
    else if(axs2.compare("Z")==0)
        AXS_2 = axis(3);
    else if(axs2.compare("-Z")==0)
        AXS_2 = axis(-3);

    if(     axs3.compare("Y")==0)
        AXS_3 = axis(1);
    else if(axs3.compare("-Y")==0)
        AXS_3 = axis(-1);
    else if(axs3.compare("X")==0)
        AXS_3 = axis(2);
    else if(axs3.compare("-X")==0)
        AXS_3 = axis(-2);
    else if(axs3.compare("Z")==0)
        AXS_3 = axis(3);
    else if(axs3.compare("-Z")==0)
        AXS_3 = axis(-3);
}
void CImport::setVoxels(float vxl1, float vxl2, float vxl3)
{
    /**/tf::debug(tf::LEV1, strprintf("voxels = (%.3f, %.3f, %.3f)", vxl1, vxl2, vxl3).c_str(), __itm__current__function__);

    VXL_1 = vxl1;
    VXL_2 = vxl2;
    VXL_3 = vxl3;
}

// reset method
void tf::CImport::reset()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    path="";
    reimport=false;
    regenerateVMap = false;
    AXS_1=AXS_2=AXS_3=iim::axis_invalid;
    VXL_1=VXL_2=VXL_3=0.0f;
    format = "";
    isTimeSeries = false;
	try
	{
		for(size_t i=0; i<volumes.size(); i++)
			delete volumes[i];
	}
	catch(iim::IOException & ex)
	{
		QMessageBox::critical(0,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
	}
    volumes.clear();
    vmapData = 0;
    vmapXDim = vmapYDim = vmapZDim = vmapTDim = vmapCDim = -1;
    updateMaxDims();

    vpResamplingFactor = -1;
    vpResamplingFactors.clear();
    vpMode = tf::VirtualPyramid::DEFAULT;
    vpLowResImagePath = "";
    vpSampling = -1;
    vpLowerBound = 100;
    vp = false;
    vpLocal = true;
    vpSetup = false;
    vpBlockDims.x = vpBlockDims.y = vpBlockDims.z = 256;
    vpHighResVolume = 0;
}

// instance high res volume for virtual pyramid
void CImport::vpInstanceHighResVolume() throw (tf::RuntimeException)
{
    try
    {
        //setenv("__UNST_CACHE__", "enable:1000 printstats:true", 1);
        if(reimport)
            vpHighResVolume = iim::VirtualVolume::instance(path.c_str(), format, AXS_1, AXS_2, AXS_3, VXL_1, VXL_2, VXL_3);
        else
            vpHighResVolume = iim::VirtualVolume::instance(path.c_str());
    }
    catch (iim::IOException & ex)
    {
        throw tf::RuntimeException(ex.what(), __itm__current__function__);
    }
    catch (iom::exception & ex)
    {
        throw tf::RuntimeException(ex.what(), __itm__current__function__);
    }
}

//automatically called when current thread is started
void CImport::run()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    try
    {
        timerIO.start();

        /**/tf::debug(tf::LEV_MAX, strprintf("importing current volume at \"%s\"", path.c_str()).c_str(), __itm__current__function__);

        // @ADDED by Alessandro on 2016-03-10. Unconverted and unstitched volumes requires ad hoc import procedure
        if( format == tf::volume_format(tf::volume_format::UNCONVERTED).toString() ||
            format == tf::volume_format(tf::volume_format::UNSTITCHED).toString()   )
        {
            // first time opening: generate virtual pyramid image from high-res unconverted image
            if(vpSetup)
            {
                if(vpResamplingFactors.empty())
                    volumes = (new tf::VirtualPyramid(path, vpResamplingFactor,  float(vpLowerBound), vpHighResVolume, vpMode, vpLowResImagePath, vpSampling, vpLocal, vpBlockDims, vpBlockFormat))->virtualPyramid();
                else
                    volumes = (new tf::VirtualPyramid(path, vpResamplingFactors, vpHighResVolume, vpMode, vpLowResImagePath, vpSampling, vpLocal, vpBlockDims, vpBlockFormat))->virtualPyramid();
            }
            // otherwise load the currently stored virtual pyramid as is
            else
                volumes = (new tf::VirtualPyramid(path, vpHighResVolume, vpLocal))->virtualPyramid();

            // activate VP mode
            vp = true;

            // set bits remap/conversion
            setBitsRemap(CSettings::instance()->getBitsRemap());
            setBitsConversion(CSettings::instance()->getBitsConversion());

            // load image data from lowest-res pyramid layer
            vmapXDim = volumes[0]->getDIM_H();
            vmapYDim = volumes[0]->getDIM_V();
            vmapZDim = volumes[0]->getDIM_D();
            vmapCDim = volumes[0]->getDIM_C();
            vmapTDim = volumes[0]->getDIM_T();
            vmapData = volumes[0]->loadSubvolume_to_UINT8();

            // emit "import successful" signal
            emit sendOperationOutcome(0, timerIO.elapsed());

            // unconverted image import ends here (no volume map file generation)
            return;
        }

        // HDF5 BigDataViewer pyramid image (one single file)
        if ( iim::isFile(path) && tf::hasEnding(path, ".h5"))
        {
			fprintf(stderr,"------------->>> path = %s \n",path.c_str()); fflush(stderr);
			BDV_HDF5init(path.c_str(), HDF5_descr);
			int n_res = BDV_HDF5n_resolutions(HDF5_descr);
			fprintf(stderr,"------------->>> resolutions = %d \n",n_res); fflush(stderr);
            for ( int i=0; i<n_res; i++ )
            {
				/* the HDF5 descriptor is passed instead of the file name: 
				 * the file will be closed after volumes will be released
				 * (see the CImport destructor) 
				 */
				volumes.push_back(VirtualVolume::instance((const char *)0, (n_res - 1 - i), HDF5_descr));
                fprintf(stderr,"------------->>> created volume %d at resolution %d\n",i,(n_res - 1 - i)); fflush(stderr);
			}
		}
        // TeraFly pyramid image (a hierarchy of nested folders)
        else if (iim::isDirectory(path) )
        {
		
            /********************* 1) IMPORTING CURRENT VOLUME ***********************
            PRECONDITIONS:
            reimport = true  ==> the volume cannot be directly imported (i.e., w/o the
            additional info provided by the user) or the user explicitly asked for re-
            importing the volume.
            reimport = false ==> the volume is directly importable
            *************************************************************************/

            // skip nonmatching entries
            QDir dir(path.c_str());
            if( dir.dirName().toStdString().substr(0,3).compare(tf::RESOLUTION_PREFIX) != 0)
                throw RuntimeException(strprintf("\"%s\" is not a valid resolution: the name of the folder does not start with \"%s\"",
                                                 path.c_str(), tf::RESOLUTION_PREFIX.c_str() ).c_str());

            if(reimport)
                volumes.push_back(VirtualVolume::instance(path.c_str(), format, AXS_1, AXS_2, AXS_3, VXL_1, VXL_2, VXL_3));
            else
                volumes.push_back(VirtualVolume::instance(path.c_str()));




            /********************* 2) IMPORTING OTHER VOLUMES ***********************
            Importing all the available resolutions within the current volume's
            parent directory.
            *************************************************************************/
            /**/tf::debug(tf::LEV_MAX, "Importing other volumes of the multiresolution octree", __itm__current__function__);
            /* -------------------- detect candidate volumes -----------------------*/
            /**/tf::debug(tf::LEV_MAX, "Detecting volumes that CAN be loaded (let us call them CANDIDATE volumes: the correspondent structures will be destroyed after this step)", __itm__current__function__);
            vector<VirtualVolume*> candidateVols;
            QDir curParentDir(path.c_str());
            curParentDir.cdUp();
            QStringList otherDirs = curParentDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
            for(int k=0; k<otherDirs.size(); k++)
            {
                string path_i = curParentDir.absolutePath().append("/").append(otherDirs.at(k).toLocal8Bit().constData()).toStdString();
                QDir dir_i(path_i.c_str());

                // skip volumes[0]
                if(dir.dirName() == dir_i.dirName())
                    continue;

                // skip nonmatching entries
                if(dir_i.dirName().toStdString().substr(0,3).compare(tf::RESOLUTION_PREFIX) != 0)
                    continue;

                /**/tf::debug(tf::LEV_MAX, strprintf("Checking for loadable volume at \"%s\"", path_i.c_str()).c_str(), __itm__current__function__);
                if( !reimport && VirtualVolume::isDirectlyImportable( path_i.c_str()) )
                    candidateVols.push_back(VirtualVolume::instance(path_i.c_str()));
                else
                    volumes.push_back(VirtualVolume::instance(path_i.c_str(), volumes[0]->getPrintableFormat(),
                                      volumes[0]->getAXS_1(), volumes[0]->getAXS_2(), volumes[0]->getAXS_3(),
                                      volumes[0]->getVXL_1(), volumes[0]->getVXL_2(), volumes[0]->getVXL_3()));
            }
            /* -------------------- import candidate volumes ------------------------*/
            /**/tf::debug(tf::LEV_MAX, "Importing loadable volumes (previously checked)", __itm__current__function__);
            for(int k=0; k<candidateVols.size(); k++)
            {
                int ratio = iim::round(  pow((volumes[0]->getMVoxels() / candidateVols[k]->getMVoxels()),(1/3.0f))  );

                 /**/tf::debug(tf::LEV_MAX, strprintf("Importing loadable volume at \"%s\"", candidateVols[k]->getROOT_DIR()).c_str(), __itm__current__function__);
                if( !reimport && VirtualVolume::isDirectlyImportable( candidateVols[k]->getROOT_DIR()) )
                    volumes.push_back(VirtualVolume::instance(candidateVols[k]->getROOT_DIR()));
                else
                    volumes.push_back(VirtualVolume::instance(candidateVols[k]->getROOT_DIR(),    candidateVols[k]->getPrintableFormat(),
                                  volumes[0]->getAXS_1(),       volumes[0]->getAXS_2(),       volumes[0]->getAXS_3(),
                                  volumes[0]->getVXL_1()*ratio, volumes[0]->getVXL_2()*ratio, volumes[0]->getVXL_3()*ratio));
            }
            /* -------------------- destroy candidate volumes -----------------------*/
            /**/tf::debug(tf::LEV_MAX, "Destroying candidate volumes", __itm__current__function__);
            for(int k=0; k<candidateVols.size(); k++)
                delete candidateVols[k];
            /* ------------- sort imported volumes by ascending size ---------------*/
            /**/tf::debug(tf::LEV_MAX, "Sorting volumes by ascending size", __itm__current__function__);
            std::sort(volumes.begin(), volumes.end(), sortVolumesAscendingSize);
            /* ---------------------- check imported volumes -----------------------*/
            if(volumes.size() < 2)
                throw RuntimeException(strprintf("%d resolution found at %s: at least two resolutions are needed for the multiresolution mode",
                                                 volumes.size(), qPrintable(curParentDir.path()) ).c_str());
            for(int k=0; k<volumes.size()-1; k++)
            {
                if(volumes[k]->getPrintableFormat().compare( volumes[k+1]->getPrintableFormat() ) != 0)
                    throw RuntimeException(strprintf("Volumes have different formats at \"%s\"", qPrintable(curParentDir.absolutePath())).c_str());
                if(volumes[k]->getDIM_T() != volumes[k+1]->getDIM_T())
                    throw RuntimeException(strprintf("Volumes have different time frames at \"%s\"", qPrintable(curParentDir.absolutePath())).c_str());
            }
        }
        else
            throw tf::RuntimeException(tf::strprintf("Cannot recognize format of image at \"%s\" [stored format is \"%s\"].", path.c_str(), format.c_str()));

        // set bits remap/conversion
        setBitsRemap(CSettings::instance()->getBitsRemap());
        setBitsConversion(CSettings::instance()->getBitsConversion());


        /**************** 3) GENERATING / LOADING VOLUME 3D MAP *****************
        We generate once for all a volume map from lowest-resolution volume.
        *************************************************************************/
//        string volMapPath = tf::cdUp(path) + "/" + VMAP_BIN_FILE_NAME;
        string volMapPath = QDir::currentPath().toStdString() + "/" + VMAP_BIN_FILE_NAME;

        if(hasVolumeMapToBeRegenerated(volMapPath.c_str(), "0.9.42") || reimport || regenerateVMap)
        {
            /**/tf::debug(tf::LEV_MAX, "Entering volume's map generation section", __itm__current__function__);

            // select the highest possible resolution that fits into the 3D viewer
            VirtualVolume* selectedResolution = volumes[0];
            for(int l=0; l<volumes.size(); l++)
                if(volumes[l]->getDIM_V() <= vmapYDimMax  &&  // l resolution can fit into the 3D viewer (along y)
                   volumes[l]->getDIM_H() <= vmapXDimMax  &&  // l resolution can fit into the 3D viewer (along x)
                   volumes[l]->getDIM_D() <= vmapZDimMax)     // l resolution can fit into the 3D viewer (along z)
                    selectedResolution = volumes[l];

            // check for resampling (not yet supported)
            if(selectedResolution->getDIM_H() > vmapXDimMax ||
               selectedResolution->getDIM_V() > vmapYDimMax ||
               selectedResolution->getDIM_D() > vmapZDimMax)
                tf::warning("@TODO: resample along XYZ so as to match the volume map maximum size", __itm__current__function__);

            vmapXDim = selectedResolution->getDIM_H();
            vmapYDim = selectedResolution->getDIM_V();
            vmapZDim = selectedResolution->getDIM_D();
            vmapCDim = selectedResolution->getDIM_C();
            // if the number of time frames exceeds the maximum, we put only the first vmapTDimMax in the volume map
            vmapTDim = std::min(vmapTDimMax, selectedResolution->getDIM_T());

            // generate volume map
            selectedResolution->setActiveFrames(0, vmapTDimMax -1);
            vmapData = selectedResolution->loadSubvolume_to_UINT8();
            FILE *volMapBin = fopen(volMapPath.c_str(), "wb");
            if(!volMapBin)
                throw RuntimeException(strprintf("Cannot write volume map at \"%s\". Please check your write permissions.", volMapPath.c_str()).c_str());
            uint16 verstr_size = static_cast<uint16>(strlen(tf::version.c_str()) + 1);
            fwrite(&verstr_size, sizeof(uint16), 1, volMapBin);
            fwrite(tf::version.c_str(), verstr_size, 1, volMapBin);
            fwrite(&vmapTDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapCDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapYDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapXDim,  sizeof(uint32), 1, volMapBin);
            fwrite(&vmapZDim,  sizeof(uint32), 1, volMapBin);
            size_t vmapSize = ((size_t)vmapYDim)*vmapXDim*vmapZDim*vmapCDim*vmapTDim;
            fwrite(vmapData, vmapSize, 1, volMapBin);
            fclose(volMapBin);
        }
        else
        {
            /**/tf::debug(tf::LEV_MAX, "Entering volume's map loading section", __itm__current__function__);

            // load volume map
            FILE *volMapBin = fopen(volMapPath.c_str(), "rb");
            uint16 verstr_size;
            if(!volMapBin)
                throw RuntimeException(strprintf("Cannot read volume map at \"%s\". Please check your read permissions.", volMapPath.c_str()).c_str());
            if(!fread(&verstr_size, sizeof(uint16), 1, volMapBin))
                throw RuntimeException("Unable to read volume map file (<version_size> field). Please delete the volume map and re-open the volume.");
            char ver[1024];
            if(!fread(ver, verstr_size, 1, volMapBin))
                throw RuntimeException("Unable to read volume map file (<version> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapTDim, sizeof(uint32), 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapTDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapCDim, sizeof(uint32), 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapCDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapYDim, sizeof(uint32), 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapYDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapXDim,  sizeof(uint32), 1, volMapBin)!= 1)
                throw RuntimeException("Unable to read volume map file (<vmapXDim> field). Please delete the volume map and re-open the volume.");
            if(fread(&vmapZDim,  sizeof(uint32), 1, volMapBin)!= 1)
                throw RuntimeException("Unable to read volume map file (<vmapZDim> field). Please delete the volume map and re-open the volume.");
            size_t vmapSize = ((size_t)vmapYDim)*vmapXDim*vmapZDim*vmapCDim*vmapTDim;
            vmapData = new uint8[vmapSize];
            if(fread(vmapData, vmapSize, 1, volMapBin) != 1)
                throw RuntimeException("Unable to read volume map file (<vmapData> field). Please delete the volume map and re-open the volume.");
            fclose(volMapBin);
        }

        //everything went OK
        emit sendOperationOutcome(0, timerIO.elapsed());

        /**/tf::debug(tf::LEV1, "EOF", __itm__current__function__);
    }
    catch( iim::IOException& exception)  {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch( iom::exception& exception)    {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch( RuntimeException& exception)  {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch(const char* error)             {reset(); emit sendOperationOutcome(new RuntimeException(error));}
    catch(...)                           {reset(); emit sendOperationOutcome(new RuntimeException("Unknown error occurred"));}
}

// returns true if
// 1) the volume map does not exist OR
// 2) it is not compatible with the current version OR
// 3) contains a number of 'T' frames with T < vmapTDimMax
bool CImport::hasVolumeMapToBeRegenerated(std::string vmapFilepath,
                                          std::string min_required_version) throw (RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("vmapFilepath = \"%s\", min_required_version = \"%s\"",
                                        vmapFilepath.c_str(), min_required_version.c_str()).c_str(), __itm__current__function__);

    // open volume map
    FILE* vmapFile = fopen(vmapFilepath.c_str(), "rb");
    if(!vmapFile)
    {
        tf::warning("volume map needs to be (re-)generated: cannot open existing vmap.bin", __itm__current__function__);
        return true;
    }

    // read version
    uint16 verstr_size;
    if(!fread(&verstr_size, sizeof(uint16), 1, vmapFile))
    {
        fclose(vmapFile);
        tf::warning("volume map needs to be (re-)generated: cannot read version from vmap.bin", __itm__current__function__);
        return true;
    }
    char ver[1024];
    if(!fread(ver, verstr_size, 1, vmapFile))
    {
        fclose(vmapFile);
        tf::warning("volume map needs to be (re-)generated: cannot read version from vmap.bin", __itm__current__function__);
        return true;
    }

    // read t-c-y-x-z dimensions
    int T = 0, C = 0, Y = 0, X = 0, Z = 0;
    if(!fread(&T, sizeof(int), 1, vmapFile))
    {
        fclose(vmapFile);
        tf::warning("volume map needs to be (re-)generated: cannot read T dim from vmap.bin", __itm__current__function__);
        return true;
    }
    if(!fread(&C, sizeof(int), 1, vmapFile))
    {
        fclose(vmapFile);
        tf::warning("volume map needs to be (re-)generated: cannot read C dim from vmap.bin", __itm__current__function__);
        return true;
    }
    if(!fread(&Y, sizeof(int), 1, vmapFile))
    {
        fclose(vmapFile);
        tf::warning("volume map needs to be (re-)generated: cannot read Y dim from vmap.bin", __itm__current__function__);
        return true;
    }
    if(!fread(&X, sizeof(int), 1, vmapFile))
    {
        fclose(vmapFile);
        tf::warning("volume map needs to be (re-)generated: cannot read X dim from vmap.bin", __itm__current__function__);
        return true;
    }
    if(!fread(&Z, sizeof(int), 1, vmapFile))
    {
        fclose(vmapFile);
        tf::warning("volume map needs to be (re-)generated: cannot read Z dim from vmap.bin", __itm__current__function__);
        return true;
    }
    fclose(vmapFile);


    // check version
    if(!TeraFly::checkVersion(ver, min_required_version))
    {
        tf::warning(tf::strprintf("volume map needs to be (re-)generated: check version failed from vmap.bin (current = \"%s\", required = \"%s\"", ver, min_required_version.c_str()).c_str(), __itm__current__function__);
        return true;
    }

    // check time size: can we load more frames?
    if(T < vmapTDimMax &&           // we can load more frames
       T < volumes[0]->getDIM_T())  // there are more frames that can be loaded
    {
        tf::warning("volume map needs to be (re-)generated: check time size failed from vmap.bin", __itm__current__function__);
        return true;
    }

    // check time size: should we load less frames?
    if(T > vmapTDimMax)
    {
        tf::warning("volume map needs to be (re-)generated: mismatch between T from vmap.bin and maxTDim from GUI", __itm__current__function__);
        return true;
    }


    // check y-x-z size: can we load a larger image?
    bool volume_found = false;
    for(int l=0; l<volumes.size() && !volume_found; l++)
    {
        // search for the currently stored image resolution
        if(volumes[l]->getDIM_V() == Y && volumes[l]->getDIM_H() == X && volumes[l]->getDIM_D() == Z)
        {
            volume_found = true;

            // can we load a larger map?
            if(volumes.size() > l                       &&  // l+1 resolution exists
               volumes[l+1]->getDIM_V() <= vmapYDimMax  &&  // l+1 resolution can fit into the 3D viewer (along y)
               volumes[l+1]->getDIM_H() <= vmapXDimMax  &&  // l+1 resolution can fit into the 3D viewer (along x)
               volumes[l+1]->getDIM_D() <= vmapZDimMax)     // l+1 resolution can fit into the 3D viewer (along z)
            {
                tf::warning("volume map needs to be (re-)generated: a higher resolution scale should be displayed", __itm__current__function__);
                return true;
            }
            // should we load a smaller map?
            else if(Y > vmapYDimMax || X > vmapXDimMax || Z > vmapZDimMax)
            {
                tf::warning("volume map needs to be (re-)generated: a lower resolution scale should be displayed", __itm__current__function__);
                return true;
            }
        }
    }

    // is this the volume from which the volume map was generated?
    if(!volume_found)
    {
        tf::warning("volume map needs to be (re-)generated: volume map does not correspond to the currently opened volume", __itm__current__function__);
        return true;
    }

    // all checks passed: no need to regenerate volume map
    return false;
}

void CImport::setBitsRemap(int id)
{
    // bits remap can only be applied to 8-bits (1 byte) images
    for(int i=0; i<this->volumes.size(); i++)
        if(volumes[i]->getBYTESxCHAN() == 1)
            volumes[i]->setDEPTH_CONV_ALGO(iim::remap_algorithms_IDs[id]);
}
void CImport::setBitsConversion(int id)
{
    // bits conversions can only be applied to 16-bits (1 byte) images
    for(int i=0; i<this->volumes.size(); i++)
        if(volumes[i]->getBYTESxCHAN() == 2)
            volumes[i]->setDEPTH_CONV_ALGO(iim::conversion_algorithms_IDs[id]);
}
