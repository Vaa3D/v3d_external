#include "NaVolumeData.h"
#include "VolumeTexture.h"
#include "../utility/url_tools.h"
#include "../utility/FooDebug.h"
#include <iostream>

#if defined(USE_Qt5)
    #include <QtConcurrent/QtConcurrent> //by PHC 20200131
#endif 

#include <QFuture>
#include <QUrl>

#include "../terafly/src/presentation/theader.h"  //2015May PHC

#include <cassert>

#ifdef USE_FFMPEG
#include "../utility/loadV3dFFMpeg.h"
#endif

#include "../neuron_annotator/analysis/SleepThread.h" //added by PHC, 20130521, to avoid a linking error on Windows
/*  //commented by PHC, 20130521, to avoid a linking error on Windows
class SleepThread : QThread {
public:
    SleepThread() {}
    void msleep(int miliseconds) {
        QThread::msleep(miliseconds);
    }
};
*/

using namespace std;
using namespace jfrc;

#if defined(Q_OS_WIN)
    #define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#endif

My4DImage* ensureThreeChannel( My4DImage* input );
My4DImage* transformStackToLinear( My4DImage* img1, QUrl fileUrl );

/////////////////////////////////////////
// NaVolumeDataLoadableStack methods //
/////////////////////////////////////////


NaVolumeDataLoadableStack::NaVolumeDataLoadableStack( My4DImage* stackpParam, QUrl fileUrlParam, int stackIndexParam )
    : QObject( NULL )
    , stackp( stackpParam )
    , fileUrl( fileUrlParam )
    , stackIndex( stackIndexParam )
    , progressValue( 0 )
    , progressMin( 0 )
    , progressMax( 100 )
    , bIsCanceled( false )
{
    connect( &imageLoader, SIGNAL( progressValueChanged( int, int ) ),
             this, SIGNAL( progressValueChanged( int, int ) ) );
    connect( &imageLoader, SIGNAL( progressAborted( int ) ),
             this, SIGNAL( failed() ) );
    connect( &imageLoader, SIGNAL( progressMessageChanged( QString ) ),
             this, SIGNAL( progressMessageChanged( QString ) ) );
}

bool NaVolumeDataLoadableStack::load()
{
    setRelativeProgress( 0.02f ); // getting here *is* finite progress
    // qDebug() << "NaVolumeData::LoadableStack::load() fileUrl=" << fileUrl;
    QUrl fullFileUrl = determineFullFileUrl();
    if ( ! exists( fullFileUrl ) )
        return false;
    imageLoader.setProgressIndex( stackIndex );
    // qDebug() << fullFileUrl << __FILE__ << __LINE__;
    if (! imageLoader.loadImage(stackp, fullFileUrl)) {
        emit failed();
        qDebug() << "Error loading image" << fullFileUrl;
        return false;
    }
    /*
    // stackp->isEmpty() is returning 'true' for correctly loaded images.
    // so I'm commenting out this block.
    if (stackp->isEmpty()) {
        emit failed();
        return false;
    }
    */
    setRelativeProgress( 0.75 );
    if ( ! stackp->p_vmin )
        stackp->updateminmaxvalues();
    setRelativeProgress( 1.0 );
    emit finished();
    return true;
}

void NaVolumeDataLoadableStack::setRelativeProgress( float relativeProgress )
{
    int newProgressValue = ( int ) ( progressMin + relativeProgress * ( progressMax - progressMin ) + 0.5 );
    assert( newProgressValue >= progressMin );
    assert( newProgressValue <= progressMax );
    if ( newProgressValue == progressValue ) return;
    progressValue = newProgressValue;
    emit progressValueChanged( progressValue, stackIndex );
}

QUrl NaVolumeDataLoadableStack::determineFullFileUrl() const
{
    if ( exists( fileUrl ) )
        return fileUrl;
    QStringList extensions;
#ifdef USE_FFMPEG
    // Is fast load preference enabled?
    extensions << ".mp4";
#endif
    extensions
            << ".v3dpbd"
            << ".v3draw"
            << ".tif"
            << ".tif" // extra entry for when USE_FFMPEG is undefined
            ;
    QUrl testUrl = fileUrl;
    QString path = testUrl.path();
    for ( int e = 0; e < extensions.size(); ++e )
    {
        QString fn = path + extensions[e];
        testUrl.setPath( fn );
        if ( exists( testUrl ) )
            return testUrl;
    }
    return testUrl; // even though the file doesn't exist...
}



//////////////////////////
// NaVolumeData methods //
//////////////////////////

/* explicit */
NaVolumeData::NaVolumeData()
    : originalImageStack( NULL )
    , neuronMaskStack( NULL )
    , referenceStack( NULL )
    , emptyImage( new My4DImage() )
    , originalImageProxy( emptyImage )
    , neuronMaskProxy( emptyImage )
    , referenceImageProxy( emptyImage )
    , currentProgress( 0 )
    , bDoUpdateSignalTexture( true )
    , volumeTexture( NULL )
    , doFlipY_image( true ), doFlipY_mask( true )
{
    // Connect specific signals to general ones
    connect( this, SIGNAL( channelLoaded( int ) ),
             this, SIGNAL( dataChanged() ) );
    connect( this, SIGNAL( channelsLoaded( int ) ),
             this, SIGNAL( dataChanged() ) );
    connect( this, SIGNAL( referenceLoaded() ),
             this, SIGNAL( dataChanged() ) );
    connect( this, SIGNAL( neuronMaskLoaded() ),
             this, SIGNAL( dataChanged() ) );

    // React to the appearance of new neuron separation volume files
    connect( &progressiveLoader, SIGNAL( newFoldersFound() ),
             this, SLOT( loadStagedVolumes() ) );
}

NaVolumeData::~NaVolumeData()
{
    invalidate();
    Writer volumeWriter( *this ); // Wait for readers to finish before deleting
    volumeWriter.clearImageData();
}

void NaVolumeData::setTextureInput( const VolumeTexture* texture )
{
    volumeTexture = texture;
}

// Load new volume files from a suddenly appearing directory
bool NaVolumeData::loadBestVolumeFromDirectory( QUrl dirName )
{
    if ( dirName.isEmpty() )
        return false;
    if ( ! exists( dirName ) )
        return false;
    QUrl fastloadDir = appendPath( dirName, "fastLoad/" );

    // iterate through possible signal file names, in order of desirability

    QList<QString> extensions; // e.g. ConsolidatedSignal3.*v3dpbd*
    extensions << "v3dpbd" << "mp4" << "v3draw" << "tiff";

    QList<QString> suffixes; // e.g. ConsolidatedSignal*3*.v3dpbd

    QList<QString> bestSignalFiles;
    bestSignalFiles << "ConsolidatedSignal3.v3dpbd"; // first choice, 16-bit, linear
    bestSignalFiles << "ConsolidatedSignal2.v3dpbd"; // second choice, 8-bit, sRGB
    bestSignalFiles << "ConsolidatedSignal.v3dpbd"; // third choice, y-flipped, 8-bit, linear

    for ( int f = 0; f < bestSignalFiles.size(); ++f )
    {

    }
    return false; // This method is not implemented?
}

bool NaVolumeData::loadVolumeFromTexture()
{
    if (NULL == volumeTexture) {
        emit progressAborted( "Volume texture not found" );
        return false;
    }
    bool bSucceeded = false;
    emit progressMessageChanged( "Copying volume from 3D texture" ); // emit outside of lock block
    {
        Writer writer( *this );
        if ( writer.loadVolumeFromTexture( volumeTexture ) )
            bSucceeded = true;
    }

    if (bSucceeded) {
        // fooDebug() << __FILE__ << __LINE__;
        bDoUpdateSignalTexture = false; // because it was set upstream
        emit channelsLoaded( 3 );
    }
    else {
        emit progressAborted( "Volume load failed" );
    }
    return bSucceeded;
}

/* slot */
void NaVolumeData::setStackLoadProgress( int progressValue, int stackIndex )
{
    // qDebug() << "setStackLoadProgress()" << progressValue << stackIndex;
    if (stackIndex < 0) {
        // qDebug() << "stack index less than zero";
        return;
    }
    if (stackIndex >= stackLoadProgressValues.size()) {
        // qDebug() << "stack index out of range";
        return;
    }
    if ( progressValue == stackLoadProgressValues[stackIndex] )
        return; // no change
    // TODO - use different weights for each stack depending on file size.
    stackLoadProgressValues[stackIndex] = progressValue;
    float totalProgressValue = 0.0;
    for (int i = 0; i < stackLoadProgressValues.size(); ++i) {
        totalProgressValue += stackLoadProgressValues[i] * 1.0 / stackLoadProgressValues.size();
    }
    setProgressValue( ( int ) ( totalProgressValue + 0.5 ) );
}

/* slot */
void NaVolumeData::setProgressValue( int progressValue )
{
    if ( progressValue < 0 ) return;
    if ( progressValue > 100 ) return;
    if ( progressValue == currentProgress ) return;
    currentProgress = progressValue;
    // qDebug() << "NaVolumeData load progress =" << currentProgress;
    emit progressValueChanged( currentProgress );
}

// You might ask why I don't use My4DImage::flip(axis):
// Method My4DImage::flip(axis) does not work for multichannel images, and is 5 times slower
// than this flipY method.
void flipY( My4DImage* img )
{
    if ( NULL == img ) return;
    const long su = img->getUnitBytes();
    const long sx = img->getXDim();
    const long sy = img->getYDim();
    const long sz = img->getZDim();
    const long sc = img->getCDim();
    size_t rowBytes = su * sx;
    size_t sliceBytes = rowBytes * sy;
    size_t chanBytes = sliceBytes * sz;
    size_t halfY = sy / 2;
    std::vector<unsigned char> rowSwapBuf( rowBytes );
    unsigned char* rowBuf = &rowSwapBuf[0];
    // qDebug() << sx << sy << sz << sc << halfY;
    // qDebug() << img->getTotalBytes() << img->getTotalUnitNumber() << sx * sy * sz * sc << img->getTotalUnitNumberPerPlane() << sliceBytes;
    unsigned char* rawData = img->getRawData();
    unsigned char* chanPtr;
    unsigned char* slicePtr;
    for ( int c = 0; c < sc; ++c )
    {
        chanPtr = rawData + c * chanBytes;
        for ( int z = 0; z < sz; ++z )
        {
            slicePtr = chanPtr + z * sliceBytes;
            for ( int y = 0; y <= halfY; ++y )
            {
                // swap scan line y with scan line (sz - y - 1)
                unsigned char* rowA = slicePtr + y * rowBytes;
                unsigned char* rowB = slicePtr + ( sy - 1 - y ) * rowBytes;
                if ( rowA == rowB )
                    continue;
                memcpy( rowBuf, rowA, rowBytes );
                memcpy( rowA, rowB, rowBytes );
                memcpy( rowB, rowBuf, rowBytes );
            }
        }
    }
}

/* slot */
void NaVolumeData::loadSecondaryVolumeDataFromFiles()
{
    // fooDebug() << "NaVolumeData::loadSecondaryVolumeDataFromFiles()" << __FILE__ << __LINE__;
    bDoUpdateSignalTexture = false;
    loadStagedVolumes();
}

/* slot */
void NaVolumeData::loadStagedVolumes()
{
   // qDebug() << "NaVolumeData::loadStagedVolumes" << __FILE__ << __LINE__;
   bool bChanged = false;
   QUrl signalPath, labelPath, referencePath;
   // Loop over all volumes to load
   for ( ProgressiveCompanion *item = progressiveLoader.next(); item != NULL;
         item = progressiveLoader.next() )
   {
      assert( item->isFileItem() );
      ProgressiveFileCompanion *fileItem =
          dynamic_cast< ProgressiveFileCompanion * >( item );
      int i = 0;
      QUrl fileUrl = fileItem->getFileUrl( progressiveLoader.getFoldersToSearch(), i );
      // qDebug() << "fileUrl =" << fileUrl << __FILE__ << __LINE__;
      SignalChannel channel = ( *fileItem )[ i ].channel;
      if ( channel == CHANNEL_LABEL )
      {
         labelPath = fileUrl;
         doFlipY_mask = ( *fileItem )[ i ].flipped_in_y;
         // qDebug() << "Label" << fileUrl << __FILE__ << __LINE__;
      }
      else if ( channel == CHANNEL_ALPHA )
      {
         referencePath = fileUrl;
         // qDebug() << "Reference" << fileUrl << __FILE__ << __LINE__;
      }
      else
      {
         signalPath = fileUrl;
         doFlipY_image = ( *fileItem )[ i ].flipped_in_y;
         // qDebug() << "Signal" << fileUrl << __FILE__ << __LINE__;
         /* code */
      }
   }
   {
      Writer writer( *this );
      if ( !labelPath.isEmpty() )
      {
         writer.setMaskLabelFileUrl( labelPath );
         bChanged = true;
      }
      if ( !referencePath.isEmpty() )
      {
         writer.setReferenceStackFileUrl( referencePath );
         bChanged = true;
      }
      if ( !signalPath.isEmpty() )
      {
         writer.setOriginalImageStackFileUrl( signalPath );
         bChanged = true;
      }
   }
   if ( bChanged )
   {
      loadVolumeDataFromFiles();
   }
   // qDebug() << "end NaVolumeData::loadStagedVolumes" << __FILE__ << __LINE__;
}

/* slot */
void NaVolumeData::loadVolumeDataFromFiles()
{
    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;

    bool stacksLoaded = false;
    emit progressMessageChanged( "Loading image stack files" ); // emit outside of lock block
    emit progressValueChanged( 1 ); // show a bit of blue
    { // Allocate writer on the stack so write lock will be automatically released when method returns
        Writer volumeWriter( *this );
        // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;
        volumeWriter.clearImageData();
        // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;
        // needs to be valid for volumeWriter.loadStacks() to work
        setRepresentsActualData();
        stacksLoaded = volumeWriter.loadStacks();
        // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() << __FILE__ << __LINE__;

        // Temporary kludge to counteract complicated flipping that occurs during neuron separation.
        if (stacksLoaded) {
            if (doFlipY_image) {
                flipY( originalImageStack );
            }
            if (doFlipY_mask) {
                flipY( neuronMaskStack );
            }
        }
    } // release locks before emit
    if (! stacksLoaded) {
        invalidate();
        emit progressAborted( QString( "Problem loading stacks" ) );
        return;
    }

    // nerd report
    size_t data_size = 0;
    data_size += originalImageStack->getTotalBytes();
    data_size += referenceStack->getTotalBytes();
    data_size += neuronMaskStack->getTotalBytes();
    // qDebug() << "Loading 16-bit image data from disk took " << stopwatch.elapsed() / 1000.0 << " seconds";
    // qDebug() << "Loading 16-bit image data from disk absorbed "
    //         << (double)data_size / double(1e6) << " MB of RAM"; // kibibytes boo hoo whatever...

    // bDoUpdateSignalTexture = true; // because it needs update now

    // qDebug() << "NaVolumeData::loadVolumeDataFromFiles()" << stopwatch.elapsed() / 1000.0 << "seconds" << __FILE__ << __LINE__;
    emit progressCompleted();
    // fooDebug() << "emitting NaVolumeData::channelsLoaded" << __FILE__ << __LINE__;
    emit channelsLoaded( originalImageProxy.sc );
}

/* slot */
bool NaVolumeData::loadChannels( QUrl url ) // includes loading general volumes
{
    bool bSucceeded = false;
    int channel_count = 0;
    emit progressMessageChanged( "Loading single volume file" ); // emit outside of lock block
    {
        Writer writer( *this );
        channel_count = writer.loadChannels( url );
        if ( channel_count > 0 )
            bSucceeded = true;
    } // release lock before emitting
    if (bSucceeded) {
        emit channelsLoaded( channel_count );
    }
    else
        emit progressAborted( "Data stack load failed" );
    return bSucceeded;
}

/* slot */
/*
bool NaVolumeData::loadSingleImageMovieVolume(QUrl fileUrl)
{
    bool bSucceeded = false;
    emit progressMessageChanged("Loading single volume file"); // emit outside of lock block
    emit progressValueChanged(1); // show a bit of blue
    {
        NaVolumeData::Writer volumeWriter(*this);
        bSucceeded = volumeWriter.loadSingleImageMovieVolume(fileUrl);
    } // release lock before emit
    if (bSucceeded) {
        bDoUpdateSignalTexture = true;
        emit progressCompleted();
        emit channelsLoaded(3);
    }
    else {
        emit progressAborted("Volume load failed");
    }
    return bSucceeded;
}
*/

/* slot */
bool NaVolumeData::loadReference( QUrl fileUrl )
{
    bool bSucceeded = false;
    emit progressMessageChanged( "Loading reference channel" ); // emit outside of lock block
    {
        Writer writer( *this );
        if ( writer.loadReference( fileUrl ) )
            bSucceeded = true;
    }

    if ( bSucceeded )
        emit referenceLoaded();
    else
        emit progressAborted( "Reference load failed" );
    return bSucceeded;
}

/* slot */
bool NaVolumeData::loadOneChannel( QUrl fileUrl, int channel_index ) // includes loading general volumes
{
    bool bSucceeded = false;
    emit progressMessageChanged( QString( "Loading data channel %1" ).arg( channel_index ) ); // emit outside of lock block
    {
        Writer writer( *this );
        if ( writer.loadOneChannel( fileUrl, channel_index ) )
            bSucceeded = true;
    }
    if ( bSucceeded )
        emit channelLoaded( channel_index );
    else
        emit progressAborted( "Channel load failed" );
    return bSucceeded;
}

/* slot */
bool NaVolumeData::loadNeuronMask( QUrl fileUrl )
{
    bool bSucceeded = false;
    emit progressMessageChanged( "Loading neuron mask" ); // emit outside of lock block
    {
        Writer writer( *this );
        if ( writer.loadNeuronMask( fileUrl ) )
            bSucceeded = true;
    }
    if ( bSucceeded )
        emit neuronMaskLoaded();
    else
        emit progressAborted( "Neuron mask load failed" );
    return bSucceeded;
}

void
NaVolumeData::splitH5JStack( )
{
    if ( originalImageStack == 0 )
        return;
    // qDebug() << "converting image to 3 channels" << __FILE__ << __LINE__;
    size_t num_sig_channels = channel_spec.count( 's' );
    size_t num_ref_channels = channel_spec.count( 'r' );
    My4DImage* signalImg = new My4DImage();
    My4DImage* refImg = new My4DImage();
    signalImg->createImage(
        originalImageStack->getXDim(),
        originalImageStack->getYDim(),
        originalImageStack->getZDim(),
        num_sig_channels, // three color channels
        originalImageStack->getDatatype() ); // 1 => 8 bits per value
    refImg->createImage(
        originalImageStack->getXDim(),
        originalImageStack->getYDim(),
        originalImageStack->getZDim(),
        num_ref_channels, // three color channels
        originalImageStack->getDatatype() ); // 1 => 8 bits per value
    size_t channelBytes = signalImg->getXDim() * signalImg->getYDim() * signalImg->getZDim() * signalImg->getUnitBytes();
    bool haveMinMax = ( NULL != originalImageStack->p_vmin );
    if ( haveMinMax )
    {
        signalImg->p_vmin = new double[num_sig_channels];
        signalImg->p_vmax = new double[num_sig_channels];
        refImg->p_vmin = new double[num_ref_channels];
        refImg->p_vmax = new double[num_ref_channels];
    }

    int ref_chan = 0;
    int sig_chan = 0;
    for ( int c = 0; c < originalImageStack->getCDim(); ++c )
    {
        if ( channel_spec[c] == 's' )
        {
            memcpy( signalImg->getRawData() + sig_chan * channelBytes,
                    originalImageStack->getRawData() + c * channelBytes,
                    channelBytes );
            if ( haveMinMax )
            {
                signalImg->p_vmin[sig_chan] = originalImageStack->p_vmin[c];
                signalImg->p_vmax[sig_chan] = originalImageStack->p_vmax[c];
            }
            sig_chan++;
        }
        else
        {
            memcpy( refImg->getRawData() + ref_chan * channelBytes,
                    originalImageStack->getRawData() + c * channelBytes,
                    channelBytes );
            if ( haveMinMax )
            {
                refImg->p_vmin[ref_chan] = originalImageStack->p_vmin[c];
                refImg->p_vmax[ref_chan] = originalImageStack->p_vmax[c];
            }
            ref_chan++;
        }
    }
    delete originalImageStack;
    originalImageStack = signalImg;

    delete referenceStack;
    referenceStack = refImg;
}


//////////////////////////////////
// NaVolumeData::Writer methods //
//////////////////////////////////

void NaVolumeData::Writer::clearLandmarks()
{
    if ( m_data->originalImageStack != NULL )
        m_data->originalImageStack->listLandmarks.clear();
}

void NaVolumeData::Writer::setLandmarks( const QList<LocationSimple> locations )
{
    if ( m_data->originalImageStack != NULL )
        m_data->originalImageStack->listLandmarks = locations;
}

void NaVolumeData::Writer::clearImageData()
{
    // qDebug() << "NaVolumeData::Writer::clearImageData()" << __FILE__ << __LINE__;
    if (m_data->originalImageStack != NULL) {
        delete m_data->originalImageStack;
        m_data->originalImageStack = NULL;
        m_data->originalImageProxy.img0 = NULL;
    }
    if (m_data->neuronMaskStack != NULL) {
        delete m_data->neuronMaskStack;
        m_data->neuronMaskStack = NULL;
        m_data->neuronMaskProxy.img0 = NULL;
    }
    if (m_data->referenceStack != NULL) {
        delete m_data->referenceStack;
        m_data->referenceStack = NULL;
        m_data->referenceImageProxy.img0 = NULL;
    }
    if (m_data->emptyImage != NULL) {
        delete m_data->emptyImage;
        m_data->emptyImage = NULL;
    }
}

bool NaVolumeData::Writer::loadSingleImageMovieVolume( QUrl fileUrl )
{
    // qDebug() << "NaVolumeData::Writer::loadSingleImageMovieVolume" << fileUrl;
#ifdef USE_FFMPEG
    My4DImage* img = new My4DImage();
    if (! loadStackFFMpeg(fileUrl, *img) ) {
        delete img;
        return false;
    }
    if ( ! setSingleImageVolume( img ) )
        return false;
    return true;
#else
    return false;
#endif
}

int NaVolumeData::Writer::loadChannels( QUrl url ) // includes loading general volumes
{
    // qDebug() << "NaVolumeData::Writer::loadChannels()" << fileUrl;
    My4DImage* img = new My4DImage();
    ImageLoader loader;
    if (! loader.loadImage(img, url) ) {
        delete img;
        return 0;
    }

    setOriginalImageStackFileUrl( url );

    if (! setSingleImageVolume(img)) {
        delete img;
        return 0;
    }

    int count = m_data->originalImageStack ? m_data->originalImageStack->getCDim() : 0;
    count += m_data->referenceStack ? m_data->referenceStack->getCDim() : 0;

    return count;
}

bool NaVolumeData::Writer::setSingleImageVolume( My4DImage* img )
{
    // qDebug() << "NaVolumeData::Writer::setSingleImageVolume" << __FILE__ << __LINE__;
    if ( m_data->originalImageStack == img )
        return false; // no change
    if ( NULL == img )
        return false;
    if ( NULL != m_data->originalImageStack )
    {
        delete m_data->originalImageStack;
        m_data->originalImageStack = NULL;
        m_data->originalImageProxy.img0 = NULL;
    }
    m_data->originalImageStack = img;
    if ( ! img->p_vmin )
        img->updateminmaxvalues();
    m_data->originalImageProxy = Image4DProxy<My4DImage>( m_data->originalImageStack );
    m_data->originalImageProxy.set_minmax( m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax );


    if ( m_data->originalImageStackFileUrl.toString().endsWith( ".h5j" ) )
    {
        m_data->splitH5JStack( );

        m_data->originalImageStack = ensureThreeChannel( m_data->originalImageStack );

        // Approximate 16-bit data for 8-it data volumes
        // qDebug() << m_data->originalImageStackFileUrl;
        m_data->originalImageStack =
            transformStackToLinear( m_data->originalImageStack,
                                    m_data->originalImageStackFileUrl );
        m_data->referenceStack =
            transformStackToLinear( m_data->referenceStack,
                                    m_data->referenceStackFileUrl );

        // Ensure initialization of proxies
        m_data->originalImageProxy = Image4DProxy<My4DImage>( m_data->originalImageStack );
        m_data->originalImageProxy.set_minmax( m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax );
        m_data->referenceImageProxy = Image4DProxy<My4DImage>( m_data->referenceStack );
        m_data->referenceImageProxy.set_minmax( m_data->referenceStack->p_vmin, m_data->referenceStack->p_vmax );
    }

    return true;
}

// Convert two-channel image to three channels to avoid crash
My4DImage* ensureThreeChannel( My4DImage* input )
{
   if ( NULL == input )
      return input;
   if ( 3 == input->getCDim() )
      return input;
   // qDebug() << "converting image to 3 channels" << __FILE__ << __LINE__;
   My4DImage *volImg = new My4DImage();
   volImg->createImage( input->getXDim(), input->getYDim(), input->getZDim(),
                        3,                      // three color channels
                        input->getDatatype() ); // 1 => 8 bits per value
   size_t channelBytes =
       volImg->getXDim() * volImg->getYDim() * volImg->getZDim() * volImg->getUnitBytes();
   bool haveMinMax = ( NULL != input->p_vmin );
   if ( haveMinMax )
   {
      volImg->p_vmin = new double[ 3 ];
      volImg->p_vmax = new double[ 3 ];
   }

   for ( int c = 0; c < 3; ++c )
   {
      if ( c > input->getCDim() - 1 )
      {
        bzero( volImg->getRawData() + c * channelBytes, channelBytes );

         if ( haveMinMax )
         {
            volImg->p_vmin[ c ] = 0;
            volImg->p_vmax[ c ] = 0;
         }
      }
      else
      {
         memcpy( volImg->getRawData() + c * channelBytes,
                 input->getRawData() + c * channelBytes, channelBytes );
        if (haveMinMax) {
            volImg->p_vmin[c] = input->p_vmin[c];
            volImg->p_vmax[c] = input->p_vmax[c];
        }
      }
   }
   delete input;
   return volImg;
}

// Convert 8-bit truncated, gamma corrected stack to linear 16-bit,
// using metadata file; to approximate original 16-bit values
My4DImage* transformStackToLinear( My4DImage* img1, QUrl fileUrl )
{
    if ( img1->getUnitBytes() != 1 )
        return img1; // I only know how to transform 8-bit data
    // Load .metadata file to specify the transformation
    QUrl metadataFileUrl = fileUrl;
    metadataFileUrl.setPath( fileUrl.path() + ".metadata" );
    if (! exists(metadataFileUrl)) {
        // Try removing suffix of stack file name
        QUrl parentUrl = parent( fileUrl );
        if ( parentUrl.isEmpty() )
            return img1;
        QFileInfo fi( fileUrl.path() );
        metadataFileUrl = appendPath( parentUrl, fi.completeBaseName() + ".metadata" );
        if ( ! exists( metadataFileUrl ) )
            return img1;  // I give up.  There is no metadata file.
    }
    QTime time;
    time.start();
    // assert(QFileInfo(metadataFileUrl).exists());
    SampledVolumeMetadata metadata;
    metadata.loadFromUrl( metadataFileUrl, 0 );
    // Precompute a table of converted values for all values 0-255
    std::vector< std::vector<uint16_t> > convert;
    convert.assign( img1->getCDim(), std::vector<uint16_t>( ( size_t )256, ( uint16_t )0 ) );
    // initialize to linear conversion from 8-bits to 12-bits
    for (int c = 0; c < convert.size(); ++c) {
        std::vector<uint16_t>& cc = convert[c];
        double hdr_max = 4095.0;
        double hdr_min = 1.0;
        double gamma = 1.0;
        if ( metadata.channelHdrMaxima.size() > c )
        {
            hdr_max = metadata.channelHdrMaxima[c];
            hdr_min = metadata.channelHdrMinima[c];
            gamma = metadata.channelGamma[c];
        }
        // 255 maps to hdr_max, 1 maps to hdr_min
        double range = ( hdr_max - hdr_min );
        cc[0] = 0; // zero always maps to zero
        for ( int i = 1; i < cc.size(); ++i )
        {
            double i0 = ( i - 1.0 ) / ( cc.size() - 2.0 ); // range 0-1
            i0 = std::pow( i0, 1.0 / gamma ); // apply inverse gamma
            cc[i] = uint16_t( hdr_min + i0 * range + 0.5 );
        }
    }
    // Apply the conversion to the whole image
    const int sc = img1->getCDim();
    const int sx = img1->getXDim();
    const int sy = img1->getYDim();
    const int sz = img1->getZDim();
    My4DImage* img2 = new My4DImage();
    img2->loadImage( sx, sy, sz, sc, V3D_UINT16 );
    size_t srcRowBytes = sx * img1->getUnitBytes();
    size_t destRowBytes = sx * img2->getUnitBytes();
    size_t srcSliceBytes = sy * srcRowBytes;
    size_t destSliceBytes = sy * destRowBytes;
    size_t srcChannelBytes = sz * srcSliceBytes;
    size_t destChannelBytes = sz * destSliceBytes;
    const uint8_t* srcVol = img1->getRawData();
    uint8_t* destVol = img2->getRawData();
    bool haveMinMax = ( NULL != img1->p_vmin );
    if (haveMinMax) {
        img2->p_vmin = new double[sc];
        img2->p_vmax = new double[sc];
    }
    for (int c = 0; c < sc; ++c) {
        const uint8_t* srcChannel = srcVol + c * srcChannelBytes;
        uint8_t* destChannel = destVol + c * destChannelBytes;
        const std::vector<uint16_t>& conv = convert[c];
        for (int z = 0; z < sz; ++z) {
            const uint8_t* srcSlice = srcChannel + z * srcSliceBytes;
            uint8_t* destSlice = destChannel + z * destSliceBytes;
            for (int y = 0; y < sy; ++y) {
                const uint8_t* srcRow = srcSlice + y * srcRowBytes;
                uint16_t* destRow = ( uint16_t* )( destSlice + y * destRowBytes );
                for (int x = 0; x < sx; ++x) {
                    destRow[x] = conv[srcRow[x]];
                }
            }
        }
        // convert vmin, vmax
        if (haveMinMax) {
            img2->p_vmin[c] = conv[img1->p_vmin[c]];
            img2->p_vmax[c] = conv[img1->p_vmax[c]];
        }
    }
    // qDebug() << "linearization took" << time.elapsed() << "milliseconds";

    return img2;
}

bool NaVolumeData::queueSeparationFolder( QUrl folder ) // using new staged loader
{
    progressiveLoader.queueSeparationFolder( folder );
    /// Loading sequence: ///
    // We only load one group of companion files, from several possible candidates:
    ProgressiveLoadItem* volumeItem = new ProgressiveLoadItem();
    // First try 16-bit full size files
    ProgressiveLoadCandidate* candidate = new ProgressiveLoadCandidate();
#if 0
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedSignal3.v3dpbd" );
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedLabel3.v3dpbd", CHANNEL_LABEL );
    *candidate << new ProgressiveSingleFileCompanion( "Reference3.v3dpbd", CHANNEL_ALPHA );
    *volumeItem << candidate;
    // Next try 8-bit gamma corrected files
    candidate = new ProgressiveLoadCandidate();
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedSignal2.v3dpbd" );
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedLabel2.v3dpbd", CHANNEL_LABEL );
    *candidate << new ProgressiveSingleFileCompanion( "Reference2.v3dpbd", CHANNEL_ALPHA );
    *volumeItem << candidate;
    // Finally try original y-flipped mixed bit-depth files
    candidate = new ProgressiveLoadCandidate();
    *candidate << &( ( new ProgressiveSingleFileCompanion( "ConsolidatedSignal.v3dpbd" ) )->setFlippedY( true ) );
    *candidate << &( ( new ProgressiveSingleFileCompanion( "ConsolidatedLabel.v3dpbd", CHANNEL_LABEL ) )->setFlippedY( true ) );
    *candidate << new ProgressiveSingleFileCompanion( "Reference.v3dpbd", CHANNEL_ALPHA );
    *volumeItem << candidate;
    //
    candidate = new ProgressiveLoadCandidate();
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedSignal.v3draw" );
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedLabel.v3draw", CHANNEL_LABEL );
    *candidate << new ProgressiveSingleFileCompanion( "Reference.v3draw", CHANNEL_ALPHA );
    *volumeItem << candidate;
    //
    if ( !visuallyLosslessImage.isEmpty() )
    {
        std::string str = visuallyLosslessImage.toString().toStdString();
        std::size_t found = str.find_last_of( "/\\" );
        progressiveLoader.addSearchFolder( QUrl( QString( str.substr( 0, found ).c_str() ) ) );

        // Try to load a version with the label file
        candidate = new ProgressiveLoadCandidate();
        QString file_name = QString( str.substr( found + 1 ).c_str() );
        *candidate << new ProgressiveSingleFileCompanion( file_name );
        *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedLabel.v3draw", CHANNEL_LABEL );
        *volumeItem << candidate;

        // Then try one without
        candidate = new ProgressiveLoadCandidate();
        *candidate << new ProgressiveSingleFileCompanion( file_name );
        *volumeItem << candidate;
    }
    //
    candidate = new ProgressiveLoadCandidate();
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedSignal.tif" );
    *candidate << new ProgressiveSingleFileCompanion( "ConsolidatedLabel.tif", CHANNEL_LABEL );
    *candidate << new ProgressiveSingleFileCompanion( "Reference.tif", CHANNEL_ALPHA );
    *volumeItem << candidate;
    //
#else
    ProgressiveFileChoiceCompanion *signal = new ProgressiveFileChoiceCompanion();
    *signal << new ProgressiveFileElement( "ConsolidatedSignal3.v3dpbd", CHANNEL_RGB, false );
    *signal << new ProgressiveFileElement( "ConsolidatedSignal2.v3dpbd", CHANNEL_RGB, false );
    *signal << new ProgressiveFileElement( "ConsolidatedSignal.v3dpbd", CHANNEL_RGB, true );
    *signal << new ProgressiveFileElement( "ConsolidatedSignal.v3draw", CHANNEL_RGB, false );
    if ( !visuallyLosslessImage.isEmpty() )
    {
       std::string str = visuallyLosslessImage.toString().toStdString();
       std::size_t found = str.find_last_of( "/\\" );
       progressiveLoader.addSearchFolder(
           QUrl( QString( str.substr( 0, found ).c_str() ) ) );

       // Try to load a version with the label file
       QString file_name = QString( str.substr( found + 1 ).c_str() );
       *signal << new ProgressiveFileElement( file_name, CHANNEL_RGB, false );
    }
    *signal << new ProgressiveFileElement( "ConsolidatedSignal.tif", CHANNEL_RGB, false );
    *candidate << signal;

    ProgressiveFileChoiceCompanion *label = new ProgressiveFileChoiceCompanion();
    *label << new ProgressiveFileElement( "ConsolidatedLabel3.v3dpbd", CHANNEL_LABEL, false );
    *label << new ProgressiveFileElement( "ConsolidatedLabel2.v3dpbd", CHANNEL_LABEL, false );
    *label << new ProgressiveFileElement( "ConsolidatedLabel.v3dpbd", CHANNEL_LABEL, true );
    *label << new ProgressiveFileElement( "ConsolidatedLabel.v3draw", CHANNEL_LABEL, false );
    *label << new ProgressiveFileElement( "ConsolidatedLabel.tif", CHANNEL_LABEL, false );
    *candidate << label;

    ProgressiveFileChoiceCompanion *reference = new ProgressiveFileChoiceCompanion();
    *reference << new ProgressiveFileElement( "Reference3.v3dpbd", CHANNEL_ALPHA, false );
    *reference << new ProgressiveFileElement( "Reference2.v3dpbd", CHANNEL_ALPHA, false );
    *reference << new ProgressiveFileElement( "Reference.v3dpbd", CHANNEL_ALPHA, false );
    *reference << new ProgressiveFileElement( "Reference.v3draw", CHANNEL_ALPHA, false );
    *reference << new ProgressiveFileElement( "Reference3.tif", CHANNEL_ALPHA, false );
    *candidate << reference;

    *volumeItem << candidate;
#endif

    progressiveLoader << volumeItem;
    return true;
}

bool NaVolumeData::Writer::loadStacks()
{
    if ( ! m_data->representsActualData() ) return false;
    QTime stopwatch;
    stopwatch.start();

    // Prepare to track progress of 3 file load operations
    m_data->stackLoadProgressValues.assign( 3, 0 );
    m_data->currentProgress = -1; // to make sure progress value changes on the next line
    m_data->setProgressValue( 0 );
    QCoreApplication::processEvents(); // ensure that progress bar gets displayed

    m_data->originalImageStack = new My4DImage();
    LoadableStack originalStack( m_data->originalImageStack, m_data->originalImageStackFileUrl, 0 );
    connect( &originalStack, SIGNAL( progressValueChanged( int, int ) ),
             m_data, SLOT( setStackLoadProgress( int, int ) ) );
    connect( &originalStack, SIGNAL( progressMessageChanged( QString ) ),
             m_data, SIGNAL( progressMessageChanged( QString ) ) );
    // qDebug() << "NaVolumeData::Writer::loadStacks() starting originalStack.load()";
    // Pass stack pointer instead of stack reference to avoid problem with lack of QObject copy constructor.

    m_data->neuronMaskStack = new My4DImage();
    LoadableStack maskStack( m_data->neuronMaskStack, m_data->maskLabelFileUrl, 1 );
    connect( &maskStack, SIGNAL( progressValueChanged( int, int ) ),
             m_data, SLOT( setStackLoadProgress( int, int ) ) );
    connect( &maskStack, SIGNAL( progressMessageChanged( QString ) ),
             m_data, SIGNAL( progressMessageChanged( QString ) ) );
    // qDebug() << "NaVolumeData::Writer::loadStacks() starting maskStack.load()";

    m_data->referenceStack = new My4DImage();
    My4DImage* initialReferenceStack = m_data->referenceStack;
    LoadableStack referenceStack( initialReferenceStack, m_data->referenceStackFileUrl, 2 );
    connect( &referenceStack, SIGNAL( progressValueChanged( int, int ) ),
             m_data, SLOT( setStackLoadProgress( int, int ) ) );
    connect( &referenceStack, SIGNAL( progressMessageChanged( QString ) ),
             m_data, SIGNAL( progressMessageChanged( QString ) ) );
    // qDebug() << "NaVolumeData::Writer::loadStacks() starting referenceStack.load()";

    if ( ! m_data->representsActualData() ) return false;

    // There are some bugs with multithreaded image loading, so make it an option.
    bool bUseMultithreadedLoader = true;
    // Tiff loading is not reentrant, so don't multithread tiff loading.
    int tiff_count = 0;
    if ( !originalStack.getFileUrl().isEmpty() && originalStack.determineFullFileUrl().path().endsWith( ".tif" ) )
        ++tiff_count;
    if ( !maskStack.getFileUrl().isEmpty() && maskStack.determineFullFileUrl().path().endsWith( ".tif" ) )
        ++tiff_count;
    if ( !referenceStack.getFileUrl().isEmpty() && referenceStack.determineFullFileUrl().path().endsWith( ".tif" ) )
        ++tiff_count;
    if ( tiff_count > 1 )
    {
        bUseMultithreadedLoader = false;
        // qDebug() << "Using single thread loader because there are nonreentrant tiff files to load.";
    }
    if ( bUseMultithreadedLoader )
    {
        // Load each file in a separate thread.  This assumes that loading code is reentrant...
        QList< QFuture<void> > loaderList;

        QFuture<void> originalLoader = QtConcurrent::run( &originalStack, &LoadableStack::load );
        loaderList.append( originalLoader );

        QFuture<void> maskLoader = QtConcurrent::run( &maskStack, &LoadableStack::load );
        loaderList.append( maskLoader );

        QFuture<void> referenceLoader = QtConcurrent::run( &referenceStack, &LoadableStack::load );
        loaderList.append( referenceLoader );


        while ( 1 )
        {
            SleepThread st;
            st.msleep( 1000 );
            if ( ! m_data->representsActualData() )
            {
                // quick abort during teardown
                originalStack.cancel();
                maskStack.cancel();
                referenceStack.cancel();
            }
            int doneCount = 0;
            for ( int i = 0; i < loaderList.size(); i++ )
            {
                QFuture<void> loader = loaderList.at( i );
                if ( loader.isFinished() )
                {
                    doneCount++;
                }
            }
            int stillActive = loaderList.size() - doneCount;
            if ( stillActive == 0 )
            {
                break;
            }
            else
            {
                // qDebug() << "Waiting on " << stillActive << " loaders";
            }
            QCoreApplication::processEvents(); // let progress signals through
        }
        if ( ! m_data->representsActualData() ) return false;
    }
    else
    {
        // Non-threaded sequential loading
        m_data->setProgressMessage( "Loading multicolor brain images..." );
        if ( ! originalStack.load() )
        {
            qDebug() << "ERROR loading signal volume" << m_data->originalImageStackFileUrl;
            return false;
        }
        if ( ! m_data->representsActualData() ) return false;
        m_data->setProgressMessage( "Loading neuron fragment locations..." );
        if ( ! maskStack.load() )
        {
            qDebug() << "ERROR loading label volume" << m_data->maskLabelFileUrl;
            return false;
        }
        if ( ! m_data->representsActualData() ) return false;
        m_data->setProgressMessage( "Loading nc82 synaptic reference image..." );
        if ( ! referenceStack.load() )
        {
            qDebug() << "ERROR loading reference volume" << m_data->referenceStackFileUrl;
            return false;
        }
        if ( ! m_data->representsActualData() ) return false;
    }

    if ( m_data->originalImageStackFileUrl.toString().endsWith( ".h5j" ) )
    {
        m_data->splitH5JStack( );
    }
    // Convert 2-channel image to 3-channels to avoid crash
    m_data->originalImageStack = ensureThreeChannel( m_data->originalImageStack );

    // qDebug() << "NaVolumeData::Writer::loadStacks() done loading all stacks in " << stopwatch.elapsed() / 1000.0 << " seconds";

    if ( m_data->originalImageStack->getXDim() > 0 && ! m_data->originalImageStack->p_vmin )
        m_data->originalImageStack->updateminmaxvalues();

    if ( m_data->neuronMaskStack->getXDim() > 0 && ! m_data->neuronMaskStack->p_vmin )
        m_data->neuronMaskStack->updateminmaxvalues();

    if ( ! m_data->representsActualData() ) return false;

    if ( m_data->referenceStack->getXDim() > 0 && ! m_data->referenceStack->p_vmin )
        m_data->referenceStack->updateminmaxvalues();

    // Approximate 16-bit data for 8-it data volumes
    // qDebug() << m_data->originalImageStackFileUrl;
    m_data->originalImageStack =
        transformStackToLinear( m_data->originalImageStack,
                                m_data->originalImageStackFileUrl );
    m_data->referenceStack =
        transformStackToLinear( m_data->referenceStack,
                                m_data->referenceStackFileUrl );

    m_data->originalImageProxy = Image4DProxy<My4DImage>( m_data->originalImageStack );
    m_data->originalImageProxy.set_minmax( m_data->originalImageStack->p_vmin, m_data->originalImageStack->p_vmax );
    m_data->neuronMaskProxy = Image4DProxy<My4DImage>( m_data->neuronMaskStack );
    m_data->neuronMaskProxy.set_minmax( m_data->neuronMaskStack->p_vmin, m_data->neuronMaskStack->p_vmax );
    m_data->referenceImageProxy = Image4DProxy<My4DImage>( m_data->referenceStack );
    m_data->referenceImageProxy.set_minmax( m_data->referenceStack->p_vmin, m_data->referenceStack->p_vmax );

    return true;
}

bool NaVolumeData::Writer::loadReference( QUrl fileUrl )
{
    assert( false ); // TODO
    return false;
}

bool NaVolumeData::Writer::loadOneChannel( QUrl fileUrl, int channel_index ) // includes loading general volumes
{
    assert( false ); // TODO
    return false;
}

bool NaVolumeData::Writer::loadNeuronMask( QUrl fileUrl )
{
    assert( false ); // TODO
    return false;
}

bool NaVolumeData::Writer::loadVolumeFromTexture( const VolumeTexture* texture )
{
    // TODO
    // qDebug() << "NaVolumeData::Writer::loadVolumeFromTexture()" << __FILE__ << __LINE__;
    if ( NULL == texture )
        return false;
    {
        QTime stopwatch;
        stopwatch.start();
        jfrc::VolumeTexture::Reader textureReader( *texture ); // acquire read lock
        jfrc::Dimension size = textureReader.paddedTextureSize();
        size_t sx = size.x();
        size_t sy = size.y();
        size_t sz = size.z();
        My4DImage* volImg = new My4DImage();
        volImg->createImage( sx, sy, sz,
                             3, // RGB
                             V3D_UINT8 ); // 1 => 8 bits per value
        My4DImage* refImg = new My4DImage();
        refImg->createImage( sx, sy, sz,
                             1,
                             V3D_UINT8 );
        // TODO - perform multithreaded copy in z slabs
        // Precomputing these offsets speed debug mode from
        // 7 seconds for loop to 1.1 seconds for loop
        uint8_t* red_offset = volImg->getRawData() + 0 * sx * sy * sz;
        uint8_t* green_offset = volImg->getRawData() + 1 * sx * sy * sz;
        uint8_t* blue_offset = volImg->getRawData() + 2 * sx * sy * sz;
        uint8_t* nc82_offset = refImg->getRawData() + 0 * sx * sy * sz;
        const uint8_t* textureData = ( const uint8_t* )textureReader.signalData3D();
        for (int z = 0; z < sz; ++z) {
            const uint8_t* z_offset1 = textureData + z * sx * sy * 4;
            uint8_t* red_z_offset = red_offset + z * sx * sy * 1;
            uint8_t* green_z_offset = green_offset + z * sx * sy * 1;
            uint8_t* blue_z_offset = blue_offset + z * sx * sy * 1;
            uint8_t* nc82_z_offset = nc82_offset + z * sx * sy * 1;
            for (int y = 0; y < sy; ++y) {
                const uint8_t* y_offset1 = z_offset1 + y * sx * 4;
                uint8_t* red = red_z_offset + y * sx * 1;
                uint8_t* green = green_z_offset + y * sx * 1;
                uint8_t* blue = blue_z_offset + y * sx * 1;
                uint8_t* nc82 = nc82_z_offset + y * sx * 1;
                for (int x = 0; x < sx; ++x) {
                    const uint8_t* rgba_x = y_offset1 + x * 4;
                    red[x] = rgba_x[2]; // texture order is BGRA, so swap R/B
                    green[x] = rgba_x[1];
                    blue[x] = rgba_x[0]; // texture order is BGRA, so swap R/B
                    nc82[x] = rgba_x[3];
                }
            }
        }
        // qDebug() << "Copying texture took" << stopwatch.elapsed() << "milliseconds";
        stopwatch.restart();
        setSingleImageVolume( volImg );
        // qDebug() << "setSingleImageVolume took" << stopwatch.elapsed() << "milliseconds";
    } // release read lock
    // TODO - copy RGBA to data and reference in multiple threads

    return true;
}


//////////////////////////////////
// NaVolumeData::Reader methods //
//////////////////////////////////

const Image4DProxy<My4DImage>& NaVolumeData::Reader::getNeuronMaskProxy() const
{
    return m_data->neuronMaskProxy;
}

const Image4DProxy<My4DImage>& NaVolumeData::Reader::getOriginalImageProxy() const
{
    return m_data->originalImageProxy;
}

const Image4DProxy<My4DImage>& NaVolumeData::Reader::getReferenceImageProxy() const
{
    return m_data->referenceImageProxy;
}

bool NaVolumeData::Reader::doUpdateSignalTexture() const
{
    // fooDebug() << m_data->bDoUpdateSignalTexture << __FILE__ << __LINE__;
    return m_data->bDoUpdateSignalTexture;
}


