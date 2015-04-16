#include <algorithm>

#include <sstream>
#include <fstream>
#include <iostream>

#include "loadV3dFFMpeg.h"
#include "FFMpegVideo.h"

#ifdef USE_HDF5
#include "H5Cpp.h"
#endif

#ifdef USE_FFMPEG

using namespace std;

bool saveStackFFMpeg( const char* file_name, const My4DImage& img, enum AVCodecID codec_id )
{
    try
    {
        Image4DProxy<My4DImage> proxy( const_cast<My4DImage*>( &img ) );
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
                for ( int c = 0; c < proxy.sc; ++c )
                {
                    imin[c] = proxy.vmin[c];
                    irange2[c] = 255.0 / ( proxy.vmax[c] - proxy.vmin[c] );
                }
            }
        }
        FFMpegEncoder encoder( file_name, proxy.sx, proxy.sy, codec_id );
        for ( int z = 0; z < proxy.sz; ++z )
        {
            for ( int y = 0; y < proxy.sy; ++y )
            {
                for ( int x = 0; x < proxy.sx; ++x )
                {
                    for ( int c = 0; c < 3; ++c )
                    {
                        int ic = c;
                        if ( c >= proxy.sc ) ic = 0; // single channel volume to gray RGB movie
                        double val = proxy.value_at( x, y, z, ic );
                        val = ( val - imin[ic] ) * irange2[ic]; // rescale to range 0-255
                        encoder.setPixelIntensity( x, y, c, ( int )val );
                    }
                }
            }
            encoder.write_frame();
        }

        encoder.close();

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

bool saveStackHDF5( const char* fileName, const My4DImage& img )
{
    try
    {
#ifdef USE_HDF5
        H5::Exception::dontPrint();
        H5::H5File file( fileName, H5F_ACC_TRUNC );
        H5::Group* group = new H5::Group( file.createGroup( "/Channels" ) );

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

            FFMpegEncoder encoder( NULL, scaledWidth, scaledHeight, AV_CODEC_ID_HEVC );
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
#if 0
            name.str( std::string() );
            name << "/tmp/foo_" << c << ".mp4";
            std::ofstream myFile ( name.str(), ios::out | ios::binary );
            myFile.write( ( const char* )encoder.buffer(), encoder.buffer_size() );
            myFile.close();
#endif
        }
#endif

        file.close();

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
#ifdef USE_HDF5
    H5::Exception::dontPrint();
    H5::H5File file( fileName, H5F_ACC_RDONLY );

    for ( size_t i = 0; i < file.getObjCount(); i++ )
    {
        H5std_string name = file.getObjnameByIdx( i );
        if ( name == "Channels" )
        {
            H5::Group channels = file.openGroup( name );

            // Grab the attributes
            H5::Attribute attr = channels.openAttribute( "width" );
            H5::DataType type = attr.getDataType();
            long width, height;
            attr.read( type, &width );
            attr.close();

            attr = channels.openAttribute( "height" );
            attr.read( type, &height );
            attr.close();

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
                    uint8_t* buffer = new uint8_t[ data.getStorageSize() ];
                    data.read( buffer, data.getDataType() );
                    QByteArray qbarray( ( const char* )buffer, data.getStorageSize() );
                    data.close();

                    if ( !loadIndexedStackFFMpeg( &qbarray, img, channel_idx++, num_channels,
                                                  width, height ) )
                    {
                        v3d_msg( "Error happened in HDF file reading. Stop. \n", false );
                        return false;
                    }

                    delete [] buffer;
                }
            }
        }
    }

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
    try
    {
        FFMpegVideo video( buffer );
        if ( ! video.isOpen )
            return false;
        int sx = video.getWidth();
        int sy = video.getHeight();
        int sz = video.getNumberOfFrames();
        int sc = video.getNumberOfChannels();
        // cout << "Number of frames = " << sz << endl;

        if ( channel == 0 )
            img.createBlankImage( width, height, sz, num_channels, 1 ); // 1 byte = 8 bits per value

        Image4DProxy<Image4DSimple> proxy( &img );

        int frameCount = 0;
        for ( int z = 0; z < sz; ++z )
        {
            video.fetchFrame( z );
            // int z = frameCount;
            frameCount++;
            for ( int y = 0; y < height; ++y )
            {
                for ( int x = 0; x < width; ++x )
                {
                    proxy.put_at( x, y, z, channel,
                                  video.getPixelIntensity( x, y, ( FFMpegVideo::Channel )0 )
                                );
                }
            }
        }
        cout << "Number of frames found = " << frameCount << endl;

        return true;

    }
    catch ( ... ) {}

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


