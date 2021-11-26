#include <algorithm>

#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>

#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "loadV3dFFMpeg.h"
#include "FFMpegVideo.h"
#include "FFMpegVideo_v1.h"

#ifdef USE_HDF5
#include "hdf5_hl.h"
#include "H5Cpp.h"
#endif

#ifdef USE_FFMPEG

using namespace std;

bool
codec_lookup( std::string codec_name, AVCodecID* codec, std::string* defaults )
{
    bool result = true;

    if ( codec_name == "HEVC" )
    {
        *codec = AV_CODEC_ID_HEVC;
        *defaults = "crf=15:psy-rd=1.0";
    }
    else if ( codec_name == "FFV1" )
    {
        *codec = AV_CODEC_ID_FFV1;
        *defaults = "level=3:coder=1:context=1:g=1:slices=4:slicecrc=1";
    }
    else
        result = false;

    return result;
}

void
generate_codec_mapping( Codec_Mapping& mapping, int num_channels )
{
    mapping.clear();
    for ( int i = 0; i <= num_channels; ++i )
    {
        mapping.push_back( std::make_pair( AV_CODEC_ID_HEVC, "crf=15:psy-rd=1.0" ) );
    }
}

int roundUp(int numToRound, int multiple) 
{
    assert(multiple);
    return ((numToRound + multiple - 1) / multiple) * multiple;
}

bool saveStackFFMpeg( const char* file_name, const My4DImage& img, AVCodecID codec_id )
{
    try
    {
        Image4DProxy<My4DImage> proxy( const_cast<My4DImage*>( &img ) );

        long scaledHeight = roundUp( proxy.sy, 2 );
        long scaledWidth = roundUp( proxy.sx, 2 );

        for ( int c = 0; c < proxy.sc; ++c )
        {
            double default_irange = 1.0; // assumes data range is 0-255.0
            if ( proxy.su > 1 )
            {
                default_irange = 1.0 / 16.0; // 0-4096, like our microscope images
            }
            std::vector<double> imin( proxy.sc, 0.0 );
            std::vector<double> irange2( proxy.sc, default_irange );
            // rescale if converting from 16 bit to 8 bit
            if ( proxy.su > 1 )
            {
                if ( img.p_vmin && img.p_vmax )
                    proxy.set_minmax( img.p_vmin, img.p_vmax );
                if ( proxy.has_minmax() )
                {
                    imin[c] = proxy.vmin[c];
                    irange2[c] = 255.0 / ( proxy.vmax[c] - proxy.vmin[c] );
                }
            }

            FFMpegEncoder encoder( file_name, scaledWidth, scaledHeight, codec_id );

            // If the image needs padding, fill the expanded border regions with black
            for ( int z = 0; z < proxy.sz; ++z )
            {
                for ( int y = 0; y < scaledHeight; ++y )
                {
                    for ( int x = 0; x < scaledWidth; ++x )
                    {
                        // If inside the area with valid data
                        if ( x < proxy.sx && y < proxy.sy )
                        {
                            int ic = c;
                            double val = proxy.value_at( x, y, z, ic );
                            val = ( val - imin[ic] ) * irange2[ic]; // rescale to range 0-255
                            for ( int cc = 0; cc < 3; ++cc )
                                encoder.setPixelIntensity( x, y, cc, ( int )val );
                        }
                        else
                            for ( int cc = 0; cc < 3; ++cc )
                                encoder.setPixelIntensity( x, y, cc, 0 );
                    }
                }
                encoder.write_frame();
            }

            for ( int rem = encoder.encoded_frames(); rem < proxy.sz; rem++ )
                encoder.encode();

            encoder.close();
        }
        return true;
    }
    catch ( ... ) {}

    return false;
}

int nearestPowerOfEight( int val )
{
    int lb = val >> 3 << 3;
    int ub = ( val + 8 ) >> 3 << 3;

    return ( lb == val ) ? lb : ub;
}

bool saveStackHDF5( const char* fileName, const My4DImage& img, Codec_Mapping* mapping )
{
    try
    {
#ifdef USE_HDF5
        H5::Exception::dontPrint();
        H5::H5File* file = new H5::H5File( fileName, H5F_ACC_TRUNC );
        H5::Group* group = new H5::Group( file->createGroup( "/Channels" ) );

        Image4DProxy<My4DImage> proxy( const_cast<My4DImage*>( &img ) );

        long scaledHeight = nearestPowerOfEight( proxy.sy );
        long scaledWidth = nearestPowerOfEight( proxy.sx );

        // Initialize the upper and lower bounds
        long pad_right = ( scaledWidth - proxy.sx ) ;
        long pad_bottom = ( scaledHeight - proxy.sy );

        hsize_t dims[1] = { 1 };
        H5::DataSpace attr_ds = H5::DataSpace( 1, dims );
        H5::Attribute attr = group->createAttribute( "width", H5::PredType::STD_I64LE, attr_ds );
        attr.write( H5::PredType::NATIVE_INT, &( proxy.sx ) );
        attr = group->createAttribute( "height", H5::PredType::STD_I64LE, attr_ds );
        attr.write( H5::PredType::NATIVE_INT, &( proxy.sy ) );
        attr = group->createAttribute( "frames", H5::PredType::STD_I64LE, attr_ds );
        attr.write( H5::PredType::NATIVE_INT, &( proxy.sz ) );
        attr = group->createAttribute( "pad_right", H5::PredType::STD_I64LE, attr_ds );
        attr.write( H5::PredType::NATIVE_INT, &( pad_right ) );
        attr = group->createAttribute( "pad_bottom", H5::PredType::STD_I64LE, attr_ds );
        attr.write( H5::PredType::NATIVE_INT, &( pad_bottom ) );

        Codec_Mapping* imap = mapping;
        if ( !mapping )
        {
            imap = new Codec_Mapping();
            generate_codec_mapping( *imap, proxy.sc );
        }

        for ( int c = 0; c < proxy.sc; ++c )
        {
            double default_irange = 1.0; // assumes data range is 0-255.0
            std::vector<double> imin( proxy.sc, 0.0 );
            std::vector<double> irange2( proxy.sc, default_irange );
            // rescale if converting from 16 bit to 8 bit
            if ( proxy.su > 1 && ( *imap )[c].first != AV_CODEC_ID_HEVC )
            {
                default_irange = 1.0 / 16.0;
                if ( img.p_vmin && img.p_vmax )
                    proxy.set_minmax( img.p_vmin, img.p_vmax );
                if ( proxy.has_minmax() )
                {
                    imin[c] = proxy.vmin[c];
                    irange2[c] = 255.0 / ( proxy.vmax[c] - proxy.vmin[c] );
                }
            }

            FFMpegEncoder encoder( NULL, scaledWidth, scaledHeight, proxy.su,
                                   ( *imap )[c].first, ( *imap )[c].second );
            // If the image needs padding, fill the expanded border regions with black
            for ( int z = 0; z < proxy.sz; ++z )
            {
                for ( int y = 0; y < scaledHeight; ++y )
                {
                    for ( int x = 0; x < scaledWidth; ++x )
                    {
                        // If inside the area with valid data
                        if ( x < proxy.sx && y < proxy.sy )
                        {
                            int ic = c;
                            double val = proxy.value_at( x, y, z, ic );
                            if ( proxy.su == 1 || (proxy.su > 1 && ( *imap )[c].first != AV_CODEC_ID_HEVC ))
                            {
                                val = ( val - imin[ic] ) * irange2[ic]; // rescale to range 0-255
                               for ( int cc = 0; cc < 3; ++cc )
                                  encoder.setPixelIntensity( x, y, cc, (uint8_t)val );
                            }
                            else
                               encoder.setPixelIntensity16( x, y, (uint16_t)(val*16) );
                        }
                        else
                            if ( proxy.su == 1 || (proxy.su > 1 && ( *imap )[c].first != AV_CODEC_ID_HEVC ))
                                for ( int cc = 0; cc < 3; ++cc )
                                    encoder.setPixelIntensity( x, y, cc, 0 );
                            else
                                encoder.setPixelIntensity16( x, y, 0 );
                    }
                }
                encoder.write_frame();
            }

            for ( int rem = encoder.encoded_frames(); rem < proxy.sz; rem++ )
                encoder.encode();

            encoder.close();

            hsize_t  dims[1];
            dims[0] = encoder.buffer_size();
            H5::DataSpace dataspace( 1, dims );
std: stringstream name;
            name << "Channel_" << c;
            H5::DataSet dataset = group->createDataSet( name.str(), H5::PredType::NATIVE_UINT8, dataspace );
            dataset.write( encoder.buffer(), H5::PredType::NATIVE_UINT8 );
            dataset.close();

            std::cout << "Encoded channel is " << encoder.buffer_size() << " bytes." << std::endl;
            // Uncomment this if you want to dump the individual movies to a temp file
        }
#endif
        if ( !mapping )
            delete imap;

        delete group;
        delete file;

        return true;
    }
    catch ( ... ) {}

    return false;
}

bool loadStackFFMpeg( const char* fileName, Image4DSimple& img )
{
    return loadStackFFMpeg( QUrl::fromLocalFile( fileName ), img );
}

bool loadStackHDF5( const char* fileName, Image4DSimple& img )
{
    QUrl url( fileName );
    bool local = url.scheme().isEmpty();
	// On Windows, Qt considers the drive letter a scheme
	// We assume that the only scheme were using is http, so test for it
	// and if the scheme isn't http, assume it's a drive letter and roll the dice
	if (!local)
	{
		if (!url.scheme().startsWith("http"))
			local = true;
	}
    return loadStackHDF5( local ? QUrl::fromLocalFile( fileName ) : url, img );
}

bool loadStackHDF5(QUrl url, Image4DSimple& img)
{
    if (url.isEmpty())
        return false;

    // Is the movie source a local file?
    if (url.host() == "localhost")
        url.setHost("");
    QString fileName = url.toLocalFile();
    if ( (! fileName.isEmpty())
        && (QFileInfo(fileName).exists()) )
    {
        QFile fileStream;

        // Yes, the source is a local file
        fileStream.setFileName(fileName);
        // qDebug() << fileName;
        if (! fileStream.open(QIODevice::ReadOnly))
            return false;

        QBuffer fileBuffer;
        QByteArray byteArray;

        byteArray = fileStream.readAll();
        fileStream.close();
        fileBuffer.setBuffer(&byteArray);
        fileBuffer.open(QIODevice::ReadOnly);
        if (! fileBuffer.seek(0))
            return false;

        return loadStackHDF5(fileBuffer, img);
    }

    // ...No, the source is not a local file
    if (url.host() == "")
        url.setHost("localhost");
    fileName = url.path();

    // http://stackoverflow.com/questions/9604633/reading-a-file-located-in-memory-with-libavformat
    // Load from URL
    QEventLoop loop; // for synchronous url fetch http://stackoverflow.com/questions/5486090/qnetworkreply-wait-for-finished
    QNetworkAccessManager networkManager;
    QNetworkReply* reply;

    QObject::connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            &loop, SLOT(quit()));
    QNetworkRequest request = QNetworkRequest(url);
    // qDebug() << "networkManager" << __FILE__ << __LINE__;
    reply = networkManager.get(request);
    loop.exec();
    if ( reply->error() != QNetworkReply::NoError )
    {
        // qDebug() << reply->error();
        reply->deleteLater();
        reply = NULL;
        return false;
    }
    QIODevice* stream = reply;
    QBuffer fileBuffer;
    QByteArray byteArray;

    byteArray = stream->readAll();
    fileBuffer.setBuffer( &byteArray );
    fileBuffer.open( QIODevice::ReadOnly );
    if ( ! fileBuffer.seek( 0 ) )
        return false;
    bool result = loadStackHDF5( fileBuffer, img );
    return result;
}

/**
* Open a `file image`, i.e. a chunk of memory which contains
* the contents of a HDF-5 file.
*
* See H5LTopen_file_image for the arguments that the constructor
* takes.
**/
#ifdef USE_HDF5
class H5FileImage : public H5::H5File
{
  public:
    H5FileImage( void* buf_ptr, size_t buf_size, unsigned flags );

    static const unsigned OPEN_RW;
    static const unsigned DONT_COPY;
    static const unsigned DONT_RELEASE;
};

H5FileImage::H5FileImage( void* buf_ptr, size_t buf_size, unsigned flags )
    : H5::H5File()
{
    hid_t id = H5LTopen_file_image( buf_ptr, buf_size, flags );
    if ( id < 0 )
        throw H5::FileIException( "H5FileImage constructor",
                                  "H5LTopen_file_image failed" );
    p_setId( id );
}

const unsigned H5FileImage::OPEN_RW      =  H5LT_FILE_IMAGE_OPEN_RW;
const unsigned H5FileImage::DONT_COPY    =  H5LT_FILE_IMAGE_DONT_COPY;
const unsigned H5FileImage::DONT_RELEASE =  H5LT_FILE_IMAGE_DONT_RELEASE;
#endif


bool loadStackHDF5( QBuffer& buffer, Image4DSimple& img )
{
#ifdef USE_HDF5
    H5::Exception::dontPrint();
    H5FileImage file((void*)(buffer.data().data()), buffer.size(), H5FileImage::DONT_COPY & H5FileImage::DONT_RELEASE);
    //H5::H5File file( fileName, H5F_ACC_RDONLY );

    for ( size_t i = 0; i < file.getObjCount(); i++ )
    {
        H5std_string name = file.getObjnameByIdx( i );
        if ( name == "Channels" )
        {
            H5::Group channels = file.openGroup( name );
			long data[2], width, height;

			// Grab the attributes
            H5::Attribute attr = channels.openAttribute( "width" );
            H5::DataType type = attr.getDataType();
			// Windows throws a runtime error about a corrupted stack if we
			// try to read the attribute directly into the variable
			attr.read( type, data ); 
            attr.close();
			width = data[0];

			attr = channels.openAttribute( "height" );
			type = attr.getDataType();
            attr.read( type, data );
            attr.close();
			height = data[0];

			int num_channels = 0;
            // Count the number of channels
            for ( size_t obj = 0; obj < channels.getNumObjs(); obj++ )
                if ( channels.getObjTypeByIdx( obj ) == H5G_DATASET )
                    num_channels++;

            int channel_idx = 0;

            for ( size_t obj = 0; obj < channels.getNumObjs(); obj++ )
            {
                if ( channels.getObjTypeByIdx( obj ) == H5G_DATASET )
                {
                    H5std_string ds_name = channels.getObjnameByIdx( obj );
                    H5::DataSet data = channels.openDataSet( ds_name );
                    uint8_t* channel_buf = new uint8_t[ data.getStorageSize() ];
                    data.read( channel_buf, data.getDataType() );
                    QByteArray qbarray( ( const char* )channel_buf, data.getStorageSize() );
                    data.close();
					
                    if ( !loadIndexedStackFFMpeg( &qbarray, img, channel_idx++, num_channels,
                                                  width, height ) )
                    {
                        v3d_msg( "Error happened in HDF file reading. Stop. \n", false );
                        return false;
                    }
					
                    delete [] channel_buf;
                }
            }
        }
    }

	file.close();

#endif

    return true;
}

bool loadStackFFMpeg( QUrl url, Image4DSimple& img )
{
    try
    {
        FFMpegVideo video( url );
        if ( ! video.isOpen )
            return false;
        int sx = video.getWidth();
        int sy = video.getHeight();
        int sz = video.getNumberOfFrames();
        int sc = video.getNumberOfChannels();
        // cout << "Number of frames = " << sz << endl;

        img.createBlankImage( sx, sy, sz, sc, 1 ); // 1 byte = 8 bits per value
        Image4DProxy<Image4DSimple> proxy( &img );

        int frameCount = 0;
        for ( int z = 0; z < sz; ++z )
        {
            video.fetchFrame( z );
            // int z = frameCount;
            frameCount++;
            for ( int c = 0; c < sc; ++c )
            {
                for ( int y = 0; y < sy; ++y )
                {
                    for ( int x = 0; x < sx; ++x )
                    {
                        proxy.put_at( x, y, z, c,
                                      video.getPixelIntensity( x, y, ( FFMpegVideo::Channel )c )
                                    );
                    }
                }
            }
        }
        cout << "Number of frames found = " << frameCount << endl;

        return true;

    }
    catch ( ... ) {}

    return false;
}

bool loadIndexedStackFFMpeg( QByteArray* buffer, Image4DSimple& img, int channel, int num_channels,
                             long width, long height )
{
    int frameCount = 0;
   try
   {
      FFMpegVideo video( buffer );
      if ( !video.isOpen )
         return false;
      if ( video.getPixelFormat() != AV_PIX_FMT_YUV444P )
      {
         int sx = video.getWidth();
         int sy = video.getHeight();
         int sz = video.getNumberOfFrames();
         int sc = video.getNumberOfChannels();
         // cout << "Number of frames = " << sz << endl;

         if ( channel == 0 )
            img.createBlankImage( width, height, sz, num_channels,
                                  video.getBitDepth() ); // 1 byte = 8 bits per value

         Image4DProxy< Image4DSimple > proxy( &img );

         for ( int z = 0; z < sz; ++z )
         {
            video.fetchFrame( z );
            // int z = frameCount;
            frameCount++;
            for ( int y = 0; y < height; ++y )
            {
               for ( int x = 0; x < width; ++x )
               {
                  if ( video.getBitDepth() == 1 )
                     proxy.put_at(
                         x, y, z, channel,
                         video.getPixelIntensity( x, y, (FFMpegVideo::Channel)0 ) );
                  else
                     proxy.put_at(
                         x, y, z, channel,
                         video.getPixelIntensity16( x, y, (FFMpegVideo::Channel)0 ) );
               }
            }
         }
      }
      else
      {
         FFMpegVideo_v1 video_v1( buffer );
         if ( !video_v1.isOpen )
            return false;
         int sx = video_v1.getWidth();
         int sy = video_v1.getHeight();
         int sz = video_v1.getNumberOfFrames();
         int sc = video_v1.getNumberOfChannels();
         // cout << "Number of frames = " << sz << endl;

         if ( channel == 0 )
            img.createBlankImage( width, height, sz, num_channels,
                                  1 ); // 1 byte = 8 bits per value

         Image4DProxy< Image4DSimple > proxy( &img );

         for ( int z = 0; z < sz; ++z )
         {
            video_v1.fetchFrame( z );
            // int z = frameCount;
            frameCount++;
            for ( int y = 0; y < height; ++y )
            {
               for ( int x = 0; x < width; ++x )
               {
                  proxy.put_at( x, y, z, channel, video_v1.getPixelIntensity(
                                                      x, y, (FFMpegVideo_v1::Channel)0 ) );
               }
            }
         }
      }
      cout << "Number of frames found = " << frameCount << endl;

      return true;
   }
   catch ( ... )
   {
   }

   return false;
}

bool loadStackFFMpegAsGray( const char* fileName, Image4DSimple& img )
{
    return loadStackFFMpegAsGray( QUrl::fromLocalFile( fileName ), img );
}

bool loadStackFFMpegAsGray( QUrl url, Image4DSimple& img )
{
    try
    {
        FFMpegVideo video( url );
        int sx = video.getWidth();
        int sy = video.getHeight();
        int sz = video.getNumberOfFrames();
        int sc = video.getNumberOfChannels();
        // cout << "Number of frames = " << sz << endl;

        img.createBlankImage( sx, sy, sz, 1, 1 ); // 1 byte = 8 bits per value
        Image4DProxy<Image4DSimple> proxy( &img );

        int frameCount = 0;
        for ( int z = 0; z < sz; ++z )
        {
            video.fetchFrame( z );
            // int z = frameCount;
            frameCount++;
            for ( int y = 0; y < sy; ++y )
            {
                for ( int x = 0; x < sx; ++x )
                {
                    // Use average of R,G,B as gray value
                    int val = 0;
                    for ( int c = 0; c < sc; ++c )
                    {
                        val += video.getPixelIntensity( x, y, ( FFMpegVideo::Channel )c );
                    }
                    val /= sc; // average of rgb
                    proxy.put_at( x, y, z, 0, val );
                }
            }
        }
        // cout << "Number of frames found = " << frameCount << endl;

        return true;

    }
    catch ( ... ) {}

    return false;
}

#endif // USE_FFMPEG


