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

#include <QSettings>
#include <iostream>
#include "VirtualPyramid.h"
#include "CSettings.h"
#include "IM_config.h"

//////// Added by MK, 11/27/2016, for Qt5/VS2015 environment 
#if defined(USE_Qt5)
#include <QtWidgets>
#endif
//////////////////////////////////////////////////////////////

using namespace terafly;
using namespace std;

CSettings* CSettings::uniqueInstance = 0;

void CSettings::uninstance()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

CSettings::~CSettings()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    writeSettings();
}

void CSettings::loadDefaultSettings()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    //TeraFly settings
    volumePathLRU = "";
    annotationPathLRU = "";
    recentlyUsedPath = "";
    VOIdimV = VOIdimH = 256;
    VOIdimD = 256;
    VOIdimT = 1;
    traslX = traslY = traslZ = 50;  //percentage value
    traslT = 0;
    annotationSpaceUnlimited = false;
    annotationCurvesDims = 2;
    annotationCurvesAspectTube = false;
    annotationVirtualMargin = 20;
    annotationMarkerSize = 20;
    previewMode = true;
    pyramidResamplingFactor = 2;
    viewerHeight = qApp->desktop()->availableGeometry().height();
    viewerWidth = qApp->desktop()->availableGeometry().width()-380;
    ramLimitGB = 1.0f;
    vpEmptyVizMethod = tf::VirtualPyramid::SALT_AND_PEPPER;
    vpEmptyVizIntensity = 255;
    vpEmptyVizSaltPepperPercentage = 0.001;
    vpFetchMethod = 2;
    vpFetchNBlocks = 5;
    vpBlockFormatIndex = 0;
    vpRefillAuto = true;
    vpRefillCoverage = 10;
    vpRefillStopCondition = 0;
    vpCacheHighestRes = false;
    vpFreezeHighestRes = false;
    bitsRemap = 0;
    bitsConversion = 1;

    //TeraConverter settings
    volumeConverterInputPathLRU = "";
    volumeConverterOutputPathLRU = "";
    volumeConverterInputFormatLRU = iim::SIMPLE_RAW_FORMAT;
    volumeConverterOutputFormatLRU = iim::TILED_FORMAT;
    volumeConverterStacksWidthLRU = 256;
    volumeConverterStacksHeightLRU = 256;
    volumeConverterStacksDepthLRU = 256;
    volumeConverterTimeSeries = false;

    voxelsizeX = 0.2;
    voxelsizeY = 0.2;
    voxelsizeZ = 1.0;

    zoomOutMethod = 0;
}

void CSettings::writeSettings()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    QSettings settings("ICON", "TeraManager");
    QString volumePathLRU_qstring(volumePathLRU.c_str());
    QString annotationPathLRU_qstring(annotationPathLRU.c_str());
    settings.setValue("annotationPathLRU", annotationPathLRU_qstring);
    settings.setValue("volumePathLRU", volumePathLRU_qstring);

    settings.beginWriteArray("recentImages");
    size_t i = 0;
    for (std::list< std::pair<std::string, std::string> >::iterator it = recentImages.begin(); it != recentImages.end(); it++)
    {
        settings.setArrayIndex(i++);
        QString path(it->first.c_str());
        QString format(it->second.c_str());
        settings.setValue("path", path);
        settings.setValue("format", format);
    }
    settings.endArray();

    settings.setValue("VOIdimV", VOIdimV);
    settings.setValue("VOIdimH", VOIdimH);
    settings.setValue("VOIdimD", VOIdimD);
    settings.setValue("VOIdimT", VOIdimT);
    settings.setValue("traslX", traslX);
    settings.setValue("traslY", traslY);
    settings.setValue("traslZ", traslZ);
    settings.setValue("traslT", traslT);
    settings.setValue("annotationSpaceUnlimited", annotationSpaceUnlimited);
    settings.setValue("annotationCurvesDims", annotationCurvesDims);
    settings.setValue("annotationCurvesAspectTube", annotationCurvesAspectTube);
    settings.setValue("annotationVirtualMargin", annotationVirtualMargin);
    settings.setValue("annotationMarkerSize", annotationMarkerSize);
    settings.setValue("previewMode", previewMode);
    settings.setValue("pyramidResamplingFactor", pyramidResamplingFactor);
    settings.setValue("viewerHeight", viewerHeight);
    settings.setValue("viewerWidth", viewerWidth);
    settings.setValue("ramLimitGB", ramLimitGB);
    settings.setValue("vpEmptyVizMethod", vpEmptyVizMethod);
    settings.setValue("vpEmptyVizIntensity", vpEmptyVizIntensity);
    settings.setValue("vpEmptyVizSaltPepperPercentage", vpEmptyVizSaltPepperPercentage);
    settings.setValue("vpFetchMethod", vpFetchMethod);
    settings.setValue("vpFetchNBlocks", vpFetchNBlocks);
    settings.setValue("vpBlockFormatIndex", vpBlockFormatIndex);
    settings.setValue("vpRefillAuto", vpRefillAuto);
    settings.setValue("vpRefillCoverage", vpRefillCoverage);
    settings.setValue("vpRefillStopCondition", vpRefillStopCondition);
    settings.setValue("vpCacheHighestRes", vpCacheHighestRes);
    settings.setValue("vpFreezeHighestRes", vpFreezeHighestRes);
    settings.setValue("voxelsizeX", voxelsizeX);
    settings.setValue("voxelsizeY", voxelsizeY);
    settings.setValue("voxelsizeZ", voxelsizeZ);
    settings.setValue("bitsRemap", bitsRemap);
    settings.setValue("bitsConversion", bitsConversion);
    settings.setValue("zoomOutMethod", zoomOutMethod);

    settings.setValue("volumeConverterInputPathLRU", QString(volumeConverterInputPathLRU.c_str()));
    settings.setValue("volumeConverterOutputPathLRU", QString(volumeConverterOutputPathLRU.c_str()));
    settings.setValue("volumeConverterInputFormatLRU", QString(volumeConverterInputFormatLRU.c_str()));
    settings.setValue("volumeConverterOutputFormatLRU", QString(volumeConverterOutputFormatLRU.c_str()));
    settings.setValue("volumeConverterStacksWidthLRU", volumeConverterStacksWidthLRU);
    settings.setValue("volumeConverterStacksHeightLRU", volumeConverterStacksHeightLRU);
    settings.setValue("volumeConverterStacksDepthLRU", volumeConverterStacksDepthLRU);
    settings.setValue("volumeConverterTimeSeries", volumeConverterTimeSeries);

    settings.setValue("recentlyUsedPath", QString(recentlyUsedPath.c_str()));

    settings.setValue("verbosity", tf::DEBUG);
}



void CSettings::readSettings()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    QSettings settings("ICON", "TeraManager");

    //TeraFly settings
    if(settings.contains("annotationPathLRU"))
        annotationPathLRU = settings.value("annotationPathLRU").toString().toStdString();
    if(settings.contains("volumePathLRU"))
        volumePathLRU = settings.value("volumePathLRU").toString().toStdString();
    if(settings.contains("VOIdimV"))
        VOIdimV = settings.value("VOIdimV").toInt();
    if(settings.contains("VOIdimH"))
        VOIdimH = settings.value("VOIdimH").toInt();
    if(settings.contains("VOIdimD"))
        VOIdimD = settings.value("VOIdimD").toInt();
    if(settings.contains("VOIdimT"))
        VOIdimT = settings.value("VOIdimT").toInt();
    if(settings.contains("traslX"))
        traslX = settings.value("traslX").toInt();
    if(settings.contains("traslY"))
        traslY = settings.value("traslY").toInt();
    if(settings.contains("traslZ"))
        traslZ = settings.value("traslZ").toInt();
    if(settings.contains("traslT"))
        traslT = settings.value("traslT").toInt();
    if(settings.contains("annotationSpaceUnlimited"))
        annotationSpaceUnlimited = settings.value("annotationSpaceUnlimited").toBool();
    if(settings.contains("annotationCurvesDims"))
        annotationCurvesDims = settings.value("annotationCurvesDims").toInt();
    if(settings.contains("annotationCurvesAspectTube"))
        annotationCurvesAspectTube = settings.value("annotationCurvesAspectTube").toBool();
    if(settings.contains("annotationVirtualMargin"))
        annotationVirtualMargin = settings.value("annotationVirtualMargin").toInt();
    if(settings.contains("annotationMarkerSize"))
        annotationMarkerSize = settings.value("annotationMarkerSize").toInt();
    if(settings.contains("previewMode"))
        previewMode = settings.value("previewMode").toBool();
    if(settings.contains("pyramidResamplingFactor"))
        pyramidResamplingFactor = settings.value("pyramidResamplingFactor").toInt();
    if(settings.contains("viewerHeight"))
        viewerHeight = settings.value("viewerHeight").toInt();
    if(settings.contains("viewerWidth"))
        viewerWidth = settings.value("viewerWidth").toInt();
    if(settings.contains("ramLimitGB"))
        ramLimitGB = settings.value("ramLimitGB").toFloat();
    if(settings.contains("vpEmptyVizMethod"))
        vpEmptyVizMethod = settings.value("vpEmptyVizMethod").toInt();
    if(settings.contains("vpEmptyVizIntensity"))
        vpEmptyVizIntensity = settings.value("vpEmptyVizIntensity").toInt();
    if(settings.contains("vpEmptyVizSaltPepperPercentage"))
        vpEmptyVizSaltPepperPercentage = settings.value("vpEmptyVizSaltPepperPercentage").toFloat();
    if(settings.contains("vpFetchMethod"))
        vpFetchMethod = settings.value("vpFetchMethod").toInt();
    if(settings.contains("vpFetchNBlocks"))
        vpFetchNBlocks = settings.value("vpFetchNBlocks").toInt();
    if(settings.contains("vpBlockFormatIndex"))
        vpBlockFormatIndex = settings.value("vpBlockFormatIndex").toInt();
    if(settings.contains("vpRefillAuto"))
        vpRefillAuto = settings.value("vpRefillAuto").toBool();
    if(settings.contains("vpRefillCoverage"))
        vpRefillCoverage = settings.value("vpRefillCoverage").toInt();
    if(settings.contains("vpRefillStopCondition"))
        vpRefillStopCondition = settings.value("vpRefillStopCondition").toInt();
    if(settings.contains("vpCacheHighestRes"))
        vpCacheHighestRes = settings.value("vpCacheHighestRes").toBool();
    if(settings.contains("vpFreezeHighestRes"))
        vpFreezeHighestRes = settings.value("vpFreezeHighestRes").toBool();
    if(settings.contains("bitsRemap"))
        bitsRemap = settings.value("bitsRemap").toInt();
    if(settings.contains("bitsConversion"))
        bitsConversion = settings.value("bitsConversion").toInt();
    if(settings.contains("voxelsizeX"))
        voxelsizeX = settings.value("voxelsizeX").toFloat();
    if(settings.contains("voxelsizeY"))
        voxelsizeY = settings.value("voxelsizeY").toFloat();
    if(settings.contains("voxelsizeZ"))
        voxelsizeZ = settings.value("voxelsizeZ").toFloat();
    if(settings.contains("zoomOutMethod"))
        zoomOutMethod = settings.value("zoomOutMethod").toInt();
    if(settings.contains("recentlyUsedPath"))
        recentlyUsedPath = settings.value("recentlyUsedPath").toString().toStdString();

    int size = settings.beginReadArray("recentImages");
    recentImages.clear();
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        recentImages.push_back(std::pair<std::string, std::string>(settings.value("path").toString().toStdString(), settings.value("format").toString().toStdString()));
        //v3d_msg(tf::strprintf("\"%s\", format \"%s\"", recentImages.back().first.c_str(), recentImages.back().second.c_str()).c_str());
    }
    settings.endArray();

    //TeraManager settings
    if(settings.contains("volumeConverterInputPathLRU"))
        volumeConverterInputPathLRU = settings.value("volumeConverterInputPathLRU").toString().toStdString();
    if(settings.contains("volumeConverterOutputPathLRU"))
        volumeConverterOutputPathLRU = settings.value("volumeConverterOutputPathLRU").toString().toStdString();
    if(settings.contains("volumeConverterInputFormatLRU"))
        volumeConverterInputFormatLRU = settings.value("volumeConverterInputFormatLRU").toString().toStdString();
    if(settings.contains("volumeConverterOutputFormatLRU"))
        volumeConverterOutputFormatLRU = settings.value("volumeConverterOutputFormatLRU").toString().toStdString();
    if(settings.contains("volumeConverterStacksWidthLRU"))
        volumeConverterStacksWidthLRU = settings.value("volumeConverterStacksWidthLRU").toInt();
    if(settings.contains("volumeConverterStacksHeightLRU"))
        volumeConverterStacksHeightLRU = settings.value("volumeConverterStacksHeightLRU").toInt();
    if(settings.contains("volumeConverterStacksDepthLRU"))
        volumeConverterStacksDepthLRU = settings.value("volumeConverterStacksDepthLRU").toInt();
    if(settings.contains("volumeConverterTimeSeries"))
        volumeConverterTimeSeries = settings.value("volumeConverterTimeSeries").toBool();



//     if(settings.contains("verbosity"))
//     {
//         tf::DEBUG = settings.value("verbosity").toInt();
//         iim::DEBUG = settings.value("verbosity").toInt();
//     }
//     else
    iim::DEBUG = iim::NO_DEBUG;
    tf::DEBUG = tf::NO_DEBUG;
}
