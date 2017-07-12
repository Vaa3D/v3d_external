
#include <sstream>
#include <fstream>
#include <cassert>
#include <utility>

#include <boost/regex.hpp>

#include <QNetworkAccessManager>
#include <QString>
#include <QtCore>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "../../v3d/v3d_core.h"
#include "../../basic_c_fun/v3d_basicdatatype.h"
#ifdef USE_FFMPEG
#include "loadV3dFFMpeg.h"
#endif
#include "ImageLoader.h"

using namespace std;

ImageLoader::ImageLoader()
    : progressIndex(0)
{
    // qDebug() << "ImageLoader() constructor called";
    mode=MODE_UNDEFINED;
    inputFilepath="";
    targetFilepath="";
    image=0;
    loadDatatype=0;
    flipy=false;
}

ImageLoader::~ImageLoader()
{
    // Note we do not delete image because we do this explicitly only if error
}

int ImageLoader::processArgs( vector<char*>* argList )
{
    for ( int i = 1; i < argList->size(); i++ )
    {
        QString arg=(*argList)[i];
        bool done=false;
        if ( arg == "-loadtest" )
        {
            mode=MODE_LOAD_TEST;
            do
            {
                QString possibleFile=(*argList)[++i];
                if ( !possibleFile.startsWith( "-" ) )
                {
                    inputFilepath=possibleFile;
                }
                else
                {
                    done=true;
                    i--; // rewind
                }
            }
            while ( !done && i < ( argList->size() - 1 ) );
        }
        else if ( arg == "-convert" )
        {
            mode=MODE_CONVERT;
            bool haveInput=false;
            do
            {
                QString possibleFile=(*argList)[++i];
                if ( !possibleFile.startsWith( "-" ) && !haveInput )
                {
                    inputFilepath=possibleFile;
                    haveInput=true;
                }
                else if ( !possibleFile.startsWith( "-" ) && haveInput )
                {
                    targetFilepath=possibleFile;
                }
                else
                {
                    done=true;
                    i--; // rewind
                }
            }
            while ( !done && i < ( argList->size() - 1 ) );
        }
        else if ( arg == "-convert8" )
        {
            mode=MODE_CONVERT8;
            bool haveInput=false;
            do
            {
                QString possibleFile=(*argList)[++i];
                if ( !possibleFile.startsWith( "-" ) && !haveInput )
                {
                    inputFilepath=possibleFile;
                    haveInput=true;
                }
                else if ( !possibleFile.startsWith( "-" ) && haveInput )
                {
                    targetFilepath=possibleFile;
                }
                else
                {
                    done=true;
                    i--; // rewind
                }
            }
            while ( !done && i < ( argList->size() - 1 ) );
        }
        else if ( arg == "-convert3" )
        {
            mode=MODE_CONVERT3;
	    qDebug() << "ImageLoader::processArgs: setting mode to MODE_CONVERT3";
            bool haveInput=false;
            do
            {
                QString possibleFile=(*argList)[++i];
                if ( !possibleFile.startsWith( "-" ) && !haveInput )
                {
                    inputFilepath=possibleFile;
                    haveInput=true;
                }
                else if ( !possibleFile.startsWith( "-" ) && haveInput )
                {
                    targetFilepath=possibleFile;
                }
                else
                {
                    done=true;
                    i--; // rewind
                }
            }
            while ( !done && i < ( argList->size() - 1 ) );
        }
        else if ( arg == "-mip" )
        {
            mode=MODE_MIP;
            bool haveInput=false;
            do
            {
                QString possibleFile=(*argList)[++i];
                if ( !possibleFile.startsWith( "-" ) && !haveInput )
                {
                    inputFilepath=possibleFile;
                    haveInput=true;
                }
                else if ( !possibleFile.startsWith( "-" ) && haveInput )
                {
                    targetFilepath=possibleFile;
                }
                else if ( possibleFile == "-flipy" )
                {
                    flipy=true;
                }
                else
                {
                    done=true;
                    i--; // rewind
                }
            }
            while ( !done && i < ( argList->size() - 1 ) );
        }
        else if ( arg == "-mapchannels" )
        {
            mode=MODE_MAP_CHANNELS;
            int sourcePosition=i+1;
            int targetPosition=i+2;
            int mapPosition=i+3;
            if ( mapPosition >= argList->size() )
            {
                qDebug() <<  "Insufficient arguments for -mapchannels option";
                return 1;
            }
            inputFilepath=(*argList)[sourcePosition];
            targetFilepath=(*argList)[targetPosition];
            mapChannelString=(*argList)[mapPosition];
            if (inputFilepath.startsWith("-") ||
                targetFilepath.startsWith("-") ||
                    mapChannelString.startsWith( "-" ) )
            {
                qDebug() << "Please see usage for -mapchannels option";
                return 1;
            }
            i+=3;
        }
#ifdef USE_FFMPEG
        else if ( arg == "-codecs" )
        {
            mode = MODE_CODECS;
            int sourcePosition = ++i;
            int targetPosition = ++i;
            inputFilepath = ( *argList )[sourcePosition];
            targetFilepath = ( *argList )[targetPosition];

            codecString = "";
            ++i;
            for ( int ind = i; ind < argList->size(); ind++, i++ )
            {
                codecString += ( *argList )[ind];
                codecString += " ";
            }
        }
#endif

        if ( inputFilepath.length() < 1 )
        {
        return 1;
    }
    return 0;
}
}

bool ImageLoader::execute()
{
    if (!validateFile())
        return false;
    QTime stopwatch;
    stopwatch.start();

    if ( mode == MODE_UNDEFINED )
    {
        qDebug() << "ImageLoader::execute() - no mode defined - doing nothing";
        return true;
    }
    else if ( mode == MODE_LOAD_TEST )
    {
        stopwatch.start();
        image=loadImage(inputFilepath);
        if ( image != 0 )
        {
            qDebug() << "Loading time is " << stopwatch.elapsed() / 1000.0 << " seconds";
            return true;
        }
        return false;
    }
    else if ( mode == MODE_CONVERT || mode == MODE_CONVERT8 || mode == MODE_CONVERT3 )
    {
      qDebug() << "ImageLoader::execute() mode=" << mode;
        if ( inputFilepath.compare( targetFilepath ) == 0 )
        {
            qDebug() << "ImageLoader::execute() - can not convert a file to itself";
            return false;
        }
        image=loadImage(inputFilepath);
        qDebug() << "Loading time is " << stopwatch.elapsed() / 1000.0 << " seconds";
        stopwatch.restart();
        qDebug() << "Saving to file " << targetFilepath << " with mode=" << mode;
        bool saveStatus=saveImageByMode(image, targetFilepath.toStdString().c_str(), mode);
        if ( image != 0 )
        {
            delete image;
            image=0;
        }
        if ( !saveStatus )
        {
            return false;
        }
        qDebug() << "Saving time is " << stopwatch.elapsed() / 1000.0 << " seconds";
        return true;
    }
    else if ( mode == MODE_MIP )
    {
        image=loadImage(inputFilepath);
        create2DMIPFromStack(image, targetFilepath);
        return true;
    }
    else if ( mode == MODE_MAP_CHANNELS )
    {
        if ( !mapChannels() )
        {
            qDebug() << "ImageLoader::execute - error in mapChannels()";
            return false;
        }
        return true;
    }
#ifdef USE_FFMPEG
    else if ( mode == MODE_CODECS )
    {
        if ( !assignCodecs() )
        {
            qDebug() << "ImageLoader::execute - error in assignCodecs()";
            return false;
        }
        return true;
    }
#endif
    return false; // should not get here
}

bool ImageLoader::mapChannels()
{
    qDebug() << "mapChannels() source=" << inputFilepath << " target=" << targetFilepath << " mapString=" << mapChannelString;
    // Create some convenient representations of the mapChannelString data
    QList<int> sourceChannelList;
    QList<int> targetChannelList;
    QStringList mapChannelList=mapChannelString.split(",");
    int i=0;
    int maxTargetChannel=0;
    int maxSourceChannel=0;
    for ( ; i < mapChannelList.size(); i++ )
    {
        if ( i == 0 || i % 2 == 0 )
        {
            QString sourceString=mapChannelList.at(i);
            int source=sourceString.toInt();
            if ( source > maxSourceChannel )
            {
                maxSourceChannel=source;
            }
            sourceChannelList.append(source);
        }
        else
        {
            QString targetString=mapChannelList.at(i);
            int target=targetString.toInt();
            if ( target > maxTargetChannel )
            {
                maxTargetChannel=target;
            }
            targetChannelList.append(target);
        }
    }
    if ( sourceChannelList.size() != targetChannelList.size() )
    {
        qDebug() << "sourceChannelList size=" << sourceChannelList.size() << " does not match targetChannelList size=" << targetChannelList.size();
        return false;
    }
    My4DImage* sourceImage=new My4DImage();
    qDebug() << "Loading source image=" << inputFilepath;
    loadImage(sourceImage, inputFilepath.toUtf8().data());
    Image4DProxy<My4DImage> sourceProxy(sourceImage);
    My4DImage* targetImage=new My4DImage();
    // Check to see if target already exists. If it does, then load it.
    QFile targetFile(targetFilepath);
    if ( targetFile.exists() )
    {
        qDebug() << "Loading target image=" << targetFilepath;
        loadImage(targetImage, targetFilepath);
    }
    else
    {
        // Must create new image
        if ( sourceImage->getDatatype() == 1 )
        {
            targetImage->loadImage(sourceProxy.sx, sourceProxy.sy, sourceProxy.sz, (maxTargetChannel+1), V3D_UINT8);
        }
        else if ( sourceImage->getDatatype() == 2 )
        {
            targetImage->loadImage(sourceProxy.sx, sourceProxy.sy, sourceProxy.sz, (maxTargetChannel+1), V3D_UINT16);
        }
        else
        {
            qDebug() << "Can not handle source datatype=" << sourceImage->getDatatype();
            return false;
        }
        memset(targetImage->getRawData(), 0, targetImage->getTotalBytes());
    }
    Image4DProxy<My4DImage> targetProxy(targetImage);
    if (sourceProxy.sx==targetProxy.sx &&
        sourceProxy.sy==targetProxy.sy &&
            sourceProxy.sz == targetProxy.sz )
    {
        qDebug() << "Verified that source and target x y z dimensions match";
    }
    else
    {
        qDebug() << "Source sx=" << sourceProxy.sx << " sy=" << sourceProxy.sy << " sz=" << sourceProxy.sz << " Target tx=" <<
                targetProxy.sx << " ty=" << targetProxy.sy << " tz=" << targetProxy.sz << " dimensions do not match";
        return false;
    }
    if ( maxSourceChannel >= sourceProxy.sc )
    {
        qDebug() << "requested sourceChannel " << maxSourceChannel << " is greater than source channel size=" << sourceProxy.sc;
        return false;
    }
    if ( maxTargetChannel >= targetProxy.sc )
    {
        qDebug() << "requested targetChannel " << maxTargetChannel << " is greater than target channel size=" << targetProxy.sc;
        return false;
    }
    // Do the transfer
    if ( sourceImage->getDatatype() == 1 )
    {
        for ( int c = 0; c < sourceChannelList.size(); c++ )
        {
            int sourceChannel=sourceChannelList.at(c);
            int targetChannel=targetChannelList.at(c);
            for ( int z = 0; z < sourceProxy.sz; z++ )
            {
                for ( int y = 0; y < sourceProxy.sy; y++ )
                {
                    for ( int x = 0; x < sourceProxy.sx; x++ )
                    {
                        if ( targetImage->getDatatype() == 1 )
                        {
                            targetProxy.put_at(x,y,z,targetChannel, *(sourceProxy.at(x,y,z,sourceChannel)));
                        }
                        else
                        {
                            // Assume type 2
                            unsigned short value=(*(sourceProxy.at(x,y,z,sourceChannel)))*16; // assume 12-bit
                            targetProxy.put_at(x, y, z, targetChannel, value);
                        }
                    }
                }
            }
        }
    }
    else if ( sourceImage->getDatatype() == 2 )
    {
        for ( int c = 0; c < sourceChannelList.size(); c++ )
        {
            int sourceChannel=sourceChannelList.at(c);
            int targetChannel=targetChannelList.at(c);
            for ( int z = 0; z < sourceProxy.sz; z++ )
            {
                for ( int y = 0; y < sourceProxy.sy; y++ )
                {
                    for ( int x = 0; x < sourceProxy.sx; x++ )
                    {
                        unsigned short sourceValue=*(sourceProxy.at_uint16(x,y,z,sourceChannel));
                        if ( targetImage->getDatatype() == 1 )
                        {
                            targetProxy.put_at(x, y, z, targetChannel, sourceValue/16); // assume 12-bit to 8-bit conversion
                        }
                        else
                        {
                            // Assume type 2
                            targetProxy.put_at(x, y, z, targetChannel, sourceValue);
                        }
                    }
                }
            }
        }
    }
    bool saveStatus=saveImage(targetImage, targetFilepath);
    delete sourceImage;
    delete targetImage;
    return saveStatus;
}

#ifdef USE_FFMPEG
bool ImageLoader::assignCodecs()
{
    if ( inputFilepath.compare( targetFilepath ) == 0 )
    {
        qDebug() << "ImageLoader::execute() - can not convert a file to itself";
        return false;
    }

    QString extension = QFileInfo( targetFilepath ).suffix().toLower();

    if ( extension == "h5j" )
    {
        image = loadImage( inputFilepath );
        qDebug() << "Saving to file " << targetFilepath << " with mode=" << mode;

        std::string codec = codecString.toStdString();
        boost::regex re( "\\d+(?:,\\d+)*:\\S+(?::\"[^\"]*\")?" );
        boost::sregex_token_iterator i( codec.begin(), codec.end(), re, 0 );
        boost::sregex_token_iterator it_end;

        Codec_Mapping mapping;
        generate_codec_mapping( mapping, image->getCDim() );

        unsigned count = 0;
        while ( i != it_end )
        {
            cout << *i << endl;
            string ss( *i );
            boost::regex re2( "\\:" );
            boost::sregex_token_iterator ii( ss.begin(), ss.end(), re2, -1 );

            std::string digits( *ii++ );
            std::string codec( *ii++ );
            std::string parameters;
            while ( ii != it_end )
            {
                parameters += *ii++;
                if ( ii != it_end ) parameters += ':';
            }


            boost::regex comma_re( "\\," );
            boost::sregex_token_iterator dit( digits.begin(), digits.end(), comma_re, -1 );
            while ( dit != it_end )
            {
                std::string str = *dit++;
                int index = atoi( str.c_str() );
                if ( index > 0 && index <= image->getCDim() + 1 )
                {
                    AVCodecID id;
                    std::string default_params;
                    if ( codec_lookup( codec, &id, &default_params ) )
                    {
                        if ( parameters.size() > 0 )
                            mapping[ index - 1 ] = std::make_pair( id, parameters );
                        else
                            mapping[ index - 1 ] = std::make_pair( id, default_params );
                    }
                }
                else
                {
                    cout << "Found invalid channel id, " << index << ". ";
                    cout << "For this stack the valid channel range is 1 - " << image->getCDim() + 1 << endl;
                }
            }

            i++;
            count++;
        }

        cout << "There were " << count << " tokens found." << endl;
        bool saveStatus = false;

        if ( ! image->p_vmin )
            image->updateminmaxvalues();

        if ( saveStackHDF5( targetFilepath.toStdString().c_str(), *image, &mapping ) )
            saveStatus = true;

        if ( image != 0 )
        {
            delete image;
            image = 0;
        }
        if ( !saveStatus )
        {
            return false;
        }
    }
    return true;
}
#endif

void ImageLoader::create2DMIPFromStack( My4DImage* image, QString mipFilepath )
{
    My4DImage * mip=create2DMIPFromStack(image);
    qDebug() << "Saving mip to file " << mipFilepath;
    mip->saveImage(mipFilepath.toUtf8().data());
    delete mip;
}

My4DImage* ImageLoader::create2DMIPFromStack( My4DImage* image )
{
    Image4DProxy<My4DImage> stackProxy(image);
    My4DImage * mip = new My4DImage();
    mip->loadImage( stackProxy.sx, stackProxy.sy, 1 /* z */, stackProxy.sc, V3D_UINT8 );
    memset(mip->getRawData(), 0, mip->getTotalBytes());
    Image4DProxy<My4DImage> mipProxy(mip);

    int divFactor=1;
    if ( image->getDatatype() == 2 )
    {
        divFactor=16;
    }

    qDebug() << "Computing mip";
    for ( int y = 0; y < stackProxy.sy; y++ )
    {
        for ( int x = 0; x < stackProxy.sx; x++ )
        {
            V3DLONG maxIntensity=0;
            int maxPosition=0;
            for ( int z = 0; z < stackProxy.sz; z++ )
            {
                V3DLONG currentIntensity=0;
                for ( int c = 0; c < stackProxy.sc; c++ )
                {
                    currentIntensity+=(*stackProxy.at(x,y,z,c));
                }
                if ( currentIntensity > maxIntensity )
                {
                    maxIntensity=currentIntensity;
                    maxPosition=z;
                }
            }
            if ( flipy )
            {
                for ( int c = 0; c < stackProxy.sc; c++ )
                {
                    mipProxy.put_at(x,stackProxy.sy-y-1,0,c,(*stackProxy.at(x,y,maxPosition,c))/divFactor);
                }
            }
            else
            {
                for ( int c = 0; c < stackProxy.sc; c++ )
                {
                    mipProxy.put_at(x,y,0,c,(*stackProxy.at(x,y,maxPosition,c))/divFactor);
                }
            }
        }
    }
    return mip;
}

void ImageLoader::convertType2Type1InPlace( My4DImage* image )
{
    if ( image->getDatatype() == 1 )
    {
        return;
    }
    else
    {
        unsigned char * newData=convertType2Type1(image);
        image->deleteRawDataAndSetPointerToNull();
        image->setRawDataPointer(newData);
        image->setDatatype(V3D_UINT8);
    }
}

unsigned char* ImageLoader::convertType2Type1( My4DImage* image )
{
    V3DLONG sz[4];
    sz[0]=image->getXDim();
    sz[1]=image->getYDim();
    sz[2]=image->getZDim();
    sz[3]=image->getCDim();
    Image4DProxy<My4DImage> proxy(image);
    V3DLONG totalSize=sz[0]*sz[1]*sz[2]*sz[3];
    unsigned char * data = new unsigned char [totalSize];
    if ( data == 0 )
    {
        return data;
    }

    unsigned int divisor=16;
    for ( V3DLONG s3 = 0; s3 < sz[3]; s3++ )
    {
        for ( V3DLONG s2 = 0; s2 < sz[2]; s2++ )
        {
            for ( V3DLONG s1 = 0; s1 < sz[1]; s1++ )
            {
                for ( V3DLONG s0 = 0; s0 < sz[0]; s0++ )
                {
		  unsigned int v = (*proxy.at_uint16(s0,s1,s2,s3));
                    if ( v > 4095 )
                    {
		    // True 16-bit
		    divisor=256;
		    break;
		  }
		}
            }
        }
    }


    for ( V3DLONG s3 = 0; s3 < sz[3]; s3++ )
    {
        for ( V3DLONG s2 = 0; s2 < sz[2]; s2++ )
        {
            for ( V3DLONG s1 = 0; s1 < sz[1]; s1++ )
            {
                for ( V3DLONG s0 = 0; s0 < sz[0]; s0++ )
                {
		  unsigned int v = (*proxy.at_uint16(s0,s1,s2,s3));
		  unsigned int v8 = v/divisor;
                    if ( v8 > 255 )
                    {
                        v8=255;
                    }
                    data[s3*sz[2]*sz[1]*sz[0] + s2*sz[1]*sz[0] + s1*sz[0] + s0] = v8;
                }
            }
        }
    }
    return data;
}


bool ImageLoader::validateFile()
{
  return validateFile(inputFilepath);
}

bool ImageLoader::validateFile( QString filename )
{
    // qDebug() << "Validating file = " << filename;
    QFileInfo fileInfo(filename);
    if ( fileInfo.exists() )
    {
        // qDebug() << " verified this file exists with size=" << fileInfo.size();
    }
    else
    {
        qDebug() << " file does not exist: " << filename;
        return false;
    }
    return true;
}

bool ImageLoader::loadImage(Image4DSimple * stackp, QUrl url)
{
    // qDebug() << "loadImage stack url" << url << __FILE__ << __LINE__;
    if ( hasPbdExtension( url.path().toStdString().c_str() ) )
    {
        if (loadRaw2StackPBDFromUrl(url, stackp, true) == 0)
            return true;
        else
            return false;
    }

    QString extension = QFileInfo(url.path()).suffix().toLower();
    QString fileName = url.toLocalFile();

#ifdef USE_FFMPEG
    if (extension == "mp4")
        return loadStackFFMpeg(url, *stackp);
#ifdef USE_HDF5
    else if (extension == "h5j")
        return loadStackHDF5(url, *stackp);
#endif
#endif

    // TODO - keep pushing URLs instead of file names deeper into the system
    if (url.host() == "localhost")
        url.setHost("");
    if (fileName.isEmpty())
        return false;
    return loadImage(stackp, fileName.toStdString().c_str());
}

bool ImageLoader::loadImage(Image4DSimple * stackp, const char* filepath)
{
  QString qFilepath(filepath);
    if ( !validateFile( qFilepath ) )
    {
    return false;
  }

    // qDebug() << "loadImage stack string" << filepath << __FILE__ << __LINE__;
    bool bSucceeded = false;
    std::string extension = getFileExtension(std::string(filepath));

    // Use threaded PBD loader for non-Basic ImageLoader
    if ( hasPbdExtension( filepath ) )
    {
        if (loadRaw2StackPBD(filepath, stackp, true) == 0)
            bSucceeded = true;
    }
    // Try base class loader
    else if ( ImageLoaderBasic::loadImage( stackp, filepath ) )
    {
        bSucceeded = true;
    }
    // mp4 loading is non-Basic, because it links all those ffmpeg libraries
#ifdef USE_FFMPEG
    else if ( extension == "mp4" )
    {
        if (loadStackFFMpeg(filepath, *stackp))
            bSucceeded = true;
    }
#ifdef USE_HDF5
    else if ( extension == "h5j" )
    {
        if (loadStackHDF5(filepath, *stackp))
            bSucceeded = true;
    }
#endif
#endif
    return bSucceeded;
}

My4DImage* ImageLoader::loadImage( QUrl url )
{
    // qDebug() << "Starting to load file " << url << __FILE__ << __LINE__;
    My4DImage* image=new My4DImage();
    loadImage(image, url);
    return image;
}

My4DImage* ImageLoader::loadImage( const char* filepath )
{
  QString qFilepath(filepath);
    if ( !validateFile( qFilepath ) )
    {
    return 0L;
  }
    // qDebug() << "Starting to load file " << filepath << __FILE__ << __LINE__;
    My4DImage* image=new My4DImage();
    loadImage(image, filepath);
    return image;
}

bool ImageLoader::saveImage( My4DImage* stackp, const char* filepath, bool saveTo8bit )
{
  qDebug() << "ImageLoader::saveImage (bool version)";
    if ( saveTo8bit )
    {
    return saveImageByMode(stackp, filepath, MODE_CONVERT8);
    }
    else
    {
    return saveImageByMode(stackp, filepath, MODE_CONVERT);
  }
}

bool ImageLoader::saveImageByMode(My4DImage * stackp, const char* filepath, int saveMode)
{
  qDebug() << "Saving to file " << filepath << " under mode=" << saveMode;
    QString extension = QFileInfo(QString(filepath)).suffix().toLower();
    if ( saveMode == MODE_CONVERT8 || saveMode == MODE_CONVERT3 )
    {
        convertType2Type1InPlace(stackp);
    }
    if ( hasPbdExtension( filepath ) )
    {
        V3DLONG sz[4];
        sz[0] = stackp->getXDim();
        sz[1] = stackp->getYDim();
        sz[2] = stackp->getZDim();
        sz[3] = stackp->getCDim();
        unsigned char* data = 0;
        data = stackp->getRawData();
        if ( saveMode == MODE_CONVERT3 )
        {
	  // using V3D_UNKNOWN as a signal to do 3-bit PBD compression, since we are also assuming the stack has been converted to 8-bit in this case
	  qDebug() << "ImageLoader::saveImageByMode: calling saveStack2RawPBD with datatype = V3D_UNKNOWN";
	  saveStack2RawPBD(filepath, V3D_UNKNOWN, data, sz);
        }
        else
        {
	  saveStack2RawPBD(filepath, stackp->getDatatype(), data, sz);
	}
    }
#ifdef USE_FFMPEG
    else if ( extension == "mp4" )
    {
        if (! stackp->p_vmin)
            stackp->updateminmaxvalues();
        // AV_CODEC_ID_MPEG4 is the only codec I have found so far to work with the Quicktime player
        if (saveStackFFMpeg(filepath, *stackp, AV_CODEC_ID_MPEG4))
            return true;
    }
#ifdef USE_HDF5
    else if ( extension == "h5j" )
    {
        if (! stackp->p_vmin)
            stackp->updateminmaxvalues();
        if (saveStackHDF5(filepath, *stackp))
            return true;
    }
#endif
#endif
    else
    {
        stackp->saveImage(filepath);
    }
    return true;
}


QString ImageLoader::getFilePrefix( const char* filepath0 )
{
    QString filepath = filepath0;
    QStringList list=filepath.split(QRegExp("\\."));
    if ( list.length() == 0 )
    {
        return filepath;
    }
    else
    {
        return list.at(0);
    }
}


int ImageLoader::loadRaw2StackPBDFromUrl(QUrl url, Image4DSimple * image, bool useThreading)
{
    // qDebug() << "loadRaw2StackPBDFromUrl" << url;
    QUrl localUrl = url;
    if (localUrl.host() == "localhost")
        localUrl.setHost("");
    QString fileName = localUrl.toLocalFile();
    if ( ( ! fileName.isEmpty() ) && QFileInfo( fileName ).exists() )
    {
        std::string str = fileName.toStdString();
        return loadRaw2StackPBD(str.c_str(), image, useThreading);
    }

    // Load from URL
    QEventLoop loop; // for synchronous url fetch http://stackoverflow.com/questions/5486090/qnetworkreply-wait-for-finished
    QNetworkAccessManager networkManager;
    QObject::connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            &loop, SLOT(quit()));
    // qDebug() << "networkManager" << __FILE__ << __LINE__;
    // qDebug() << url;
    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply * reply = networkManager.get(request);
    loop.exec();
    if ( reply->error() != QNetworkReply::NoError )
    {
        // qDebug() << reply->error();
        reply->close();
        reply->deleteLater();
        return exitWithError(std::string("Failed to read from URL"));
    }
    V3DLONG fileSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    int result = false;
    // if (true) {
    if ( fileSize == 0 )
    {
        // Unknown size? Use an in memory IO Device
        // qDebug() << "Buffering file..." << url;
        QByteArray bytes = reply->readAll();
        fileSize = bytes.size();
        QBuffer* buffer = new QBuffer(&bytes);
        // qDebug() << "Done buffering file" << url;
        buffer->open(QIODevice::ReadOnly);
        // qDebug() << "Decoding buffer..." << url;
        result = loadRaw2StackPBDFromStream(*buffer, fileSize, image, useThreading);
        // qDebug() << "Done decoding buffer" << url;
        buffer->close();
        buffer->deleteLater();
    }
    else
    {
        // Stream from URL
        result = loadRaw2StackPBDFromStream(*reply, fileSize, image, useThreading);
    }
    reply->close();
    reply->deleteLater();
    return result;
}

int ImageLoader::loadRaw2StackPBD(const char * filename, Image4DSimple * image, bool useThreading)
{
    QFile fileStream(filename);
    if (! fileStream.open(QIODevice::ReadOnly))
        return exitWithError(std::string("Fail to open file for reading."));
    V3DLONG fileSize = fileStream.size();
    return loadRaw2StackPBDFromStream(fileStream, fileSize, image, useThreading);
}

int ImageLoader::loadRaw2StackPBDFromStream(QIODevice& fileStream, V3DLONG fileSize, Image4DSimple * image, bool useThreading)
{
    // qDebug() << "ImageLoader::loadRaw2StackPBDFromStream starting filename=" << filename;

    int progressValue = 0;
    emit progressValueChanged(++progressValue, progressIndex);

    int berror = 0;
    decompressionPrior = 0;

    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "ImageLoader::loadRaw2StackPBDFromStream" << filename << stopwatch.elapsed() << __FILE__ << __LINE__;

    int datatype;

    /*
    fseek (fid, 0, SEEK_END);
    V3DLONG fileSize = ftell(fid);
    rewind(fid);
     */

    /* Read header */

    char formatkey[] = "v3d_volume_pkbitdf_encod";
    V3DLONG lenkey = strlen(formatkey);

#ifndef _MSC_VER //added by PHC, 2010-05-21
    if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
    {
        stringstream msg;
        msg << "The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n";
        msg << "The fseek-ftell produces a file size = " << fileSize << ".";
        return exitWithError(msg.str());
    }
#endif

    keyread.resize(lenkey+1);
    // V3DLONG nread = fread(&keyread[0], 1, lenkey, fid);
    V3DLONG nread = fileStream.read(&keyread[0], lenkey);
    if (nread!=lenkey)
    {
        return exitWithError(std::string("File unrecognized or corrupted file."));
    }
    keyread[lenkey] = '\0';

    V3DLONG i;
    if (strcmp(formatkey, &keyread[0])) /* is non-zero then the two strings are different */
    {
        return exitWithError(std::string("Unrecognized file format."));
    }

    char endianCodeData;
    // fread(&endianCodeData, 1, 1, fid);
    fileStream.read(&endianCodeData, 1);
    if (endianCodeData!='B' && endianCodeData!='L')
    {
        return exitWithError(std::string("This program only supports big- or little- endian but not other format. Check your data endian."));
    }

    char endianCodeMachine;
    endianCodeMachine = checkMachineEndian();
    if (endianCodeMachine!='B' && endianCodeMachine!='L')
    {
        return exitWithError(std::string("This program only supports big- or little- endian but not other format. Check your data endian."));
    }

    int b_swap = (endianCodeMachine==endianCodeData)?0:1;

    short int dcode = 0;
    // fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
    fileStream.read((char*)&dcode, 2); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
    if (b_swap)
        swap2bytes((void *)&dcode);

    switch (dcode)
    {
    case 1:
        datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
        break;

    case 2:
        datatype = 2;
        break;

    case 4:
        datatype = 4;
        break;

    case PBD_3_BIT_DTYPE: // Used for PBD 3
      datatype = PBD_3_BIT_DTYPE;
      break;

    default:
        stringstream msg;
        msg << "Unrecognized data type code [" << dcode;
        msg << "]. The file type is incorrect or this code is not supported in this version.";
        return exitWithError(msg.str());
    }

    // qDebug() << "Setting datatype=" << datatype;

    if ( datatype == 1 || datatype == PBD_3_BIT_DTYPE )
    {
        image->setDatatype(V3D_UINT8);
    }
    else if ( datatype == 2 )
    {
        image->setDatatype(V3D_UINT16);
    }
    else
    {
        return exitWithError(std::string("ImageLoader::loadRaw2StackPBDFromStream : only datatype=1 or datatype=2 supported"));
    }
    loadDatatype=image->getDatatype(); // used for threaded loading

    // qDebug() << "Finished setting datatype=" << image->getDatatype();

    V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future.

    BIT32_UNIT mysz[4];
    mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
    // int tmpn=fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
    int tmpn=fileStream.read((char*)mysz, 16); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
    if (tmpn!=16)
    {
        stringstream msg;
        msg << "This program only reads [" << tmpn << "] units.";
        return exitWithError(msg.str());
    }

    if ( b_swap && ( unitSize == 2 || unitSize == 4 ) )
    {
        stringstream msg;
        msg << "b_swap true and unitSize > 1 - this is not implemented in current code";
        return exitWithError(msg.str());
    }

    if (b_swap)
    {
        for (i=0;i<4;i++)
        {
            //swap2bytes((void *)(mysz+i));
            printf("mysz raw read unit[%ld]: [%d] ", i, mysz[i]);
            swap4bytes((void *)(mysz+i));
            printf("swap unit: [%d][%0x] \n", mysz[i], mysz[i]);
        }
    }

    std::vector<V3DLONG> sz(4, 0); // avoid memory leak
    // V3DLONG * sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img.

    V3DLONG totalUnit = 1;
    for (i=0;i<4;i++)
    {
        sz[i] = (V3DLONG)mysz[i];
        totalUnit *= sz[i];
    }

    //mexPrintf("The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Exit.\n", fileSize, totalUnit*unitSize+4*4+2+1+lenkey);
    //mexPrintf("The read sizes are: %ld %ld %ld %ld\n", sz[0], sz[1], sz[2], sz[3]);

    V3DLONG headerSize=4*4+2+1+lenkey;
    V3DLONG compressedBytes=fileSize-headerSize;
    maxDecompressionSize=totalUnit*unitSize;

    compressionBuffer.resize(compressedBytes);

    V3DLONG remainingBytes = compressedBytes;
    //V3DLONG nBytes2G = V3DLONG(1024)*V3DLONG(1024)*V3DLONG(1024)*V3DLONG(2);
    V3DLONG readStepSizeBytes = V3DLONG(1024)*20000;
    totalReadBytes = 0;

    // done reading header
    emit progressValueChanged(++progressValue, progressIndex);

    // qDebug() << "ImageLoader::loadRaw2StackPBDFromStream" << filename << stopwatch.elapsed() << __FILE__ << __LINE__;

    // Transfer data to My4DImage

    // Allocating memory can take seconds.  So send a message
    emit progressMessageChanged("Allocating image memory...");
    short int blankImageDataType=datatype;
    if ( datatype == PBD_3_BIT_DTYPE )
    {
      blankImageDataType=1;
    }
    image->createBlankImage(sz[0], sz[1], sz[2], sz[3], blankImageDataType);
    emit progressMessageChanged("Decompressing image...");
    decompressionBuffer = image->getRawData();

    QThreadPool threadPool;
    setAutoDelete(false);

    while (remainingBytes>0)
    {
        // qDebug() << "ImageLoader::loadRaw2StackPBDFromStream" << filename << stopwatch.elapsed() << __FILE__ << __LINE__;
        if ( isCanceled() )
        {
            // clean and return
            // fclose(fid);
            return exitWithError(QString("image load canceled"));
        }

        V3DLONG curReadBytes = (remainingBytes<readStepSizeBytes) ? remainingBytes : readStepSizeBytes;
        // nread = fread(&compressionBuffer[0]+totalReadBytes, 1, curReadBytes, fid);
        nread = fileStream.read((char*)(&compressionBuffer[0]+totalReadBytes), curReadBytes);
        totalReadBytes+=nread;
        if (nread!=curReadBytes)
        {
            stringstream msg;
            msg << "Something wrong in file reading. The program reads [";
            msg << nread << " data points] but the file says there should be [";
            msg << curReadBytes << "data points].";
            return exitWithError(msg.str());
        }
        remainingBytes -= nread;

        if ( useThreading )
        {
            // qDebug() << "Waiting for current thread";
            threadPool.waitForDone();
            // qDebug() << "Starting thread";
            if ( image == 0x0 )
            {
                // qDebug() << "Prior to start() image is 0";
            }
            else
            {
                // qDebug() << "Prior to start() image is non-zero";
            }
            threadPool.start(this);
        }
        else
        {
            if ( datatype == 1 )
            {
                updateCompressionBuffer8(&compressionBuffer[0]+totalReadBytes);
            }
            else
            {
                // assume datatype==2
                updateCompressionBuffer16(&compressionBuffer[0]+totalReadBytes);
            }
        }

        if ( isCanceled() )
        {
            return exitWithError(QString("load canceled"));
        }

        int newProgressValue = (int)(75.0 * (compressedBytes - remainingBytes) / (float)compressedBytes + 0.49);
        assert(newProgressValue <= 100);
        assert(newProgressValue >= 0);
        if ( progressValue < newProgressValue )
        {
            progressValue = newProgressValue;
            emit progressValueChanged(progressValue, progressIndex);
        }
    }
    // qDebug() << "ImageLoader::loadRaw2StackPBDFromStream" << filename << stopwatch.elapsed() << __FILE__ << __LINE__;
    // qDebug() << "Total time elapsed after all reads is " << stopwatch.elapsed() / 1000.0 << " seconds";

    if ( useThreading )
    {
        // qDebug() << "Final thread wait";
        threadPool.waitForDone();
        // qDebug() << "Done final wait";
    }
    emit progressComplete(progressIndex);

    // clean and return
    // fclose(fid); //bug fix on 060412
    return berror;
}


void ImageLoader::run()
{
    if ( loadDatatype == 1 )
    {
        updateCompressionBuffer8(&compressionBuffer[0]+totalReadBytes);
    }
    else
    {
        updateCompressionBuffer16(&compressionBuffer[0]+totalReadBytes);
    }
}






