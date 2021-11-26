#include "CImageUtils.h"

using namespace teramanager;

/**********************************************************************************
* Copies the given VOI from "src" to "dst". Offsets and scaling are supported.
***********************************************************************************/
void
    CImageUtils::copyVOI(
        itm::uint8 const * src,     //pointer to const data source
        uint src_dims[5],           //dimensions of "src" along X, Y, Z, channels and T
        uint src_offset[5],         //VOI's offset along X, Y, Z, <empty> and T
        uint src_count[5],          //VOI's dimensions along X, Y, Z, <empty> and T
        itm::uint8* dst,            //pointer to data destination
        uint dst_dims[5],           //dimensions of "dst" along X, Y, Z, channels and T
        uint dst_offset[5],         //offset of "dst" along X, Y, Z, <empty> and T
        uint scaling /*= 1 */)      //scaling factor (integer only)
throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("src_dims = (%d x %d x %d x %d x %d), src_offset = (%d, %d, %d, %d, %d), src_count = (%d, %d, %d, %d, %d), dst_dims = (%d x %d x %d x %d x %d), dst_offset = (%d, %d, %d, %d, %d), scaling = %d",
                                        src_dims[0], src_dims[1],src_dims[2],src_dims[3],src_dims[4], src_offset[0],src_offset[1],src_offset[2],src_offset[3], src_offset[4],src_count[0],src_count[1],src_count[2],src_count[3], src_count[4],
                                        dst_dims[0], dst_dims[1],dst_dims[2],dst_dims[3],dst_dims[4], dst_offset[0],dst_offset[1],dst_offset[2],dst_offset[3], dst_offset[4],scaling).c_str(), __itm__current__function__);

    //if source and destination are the same thing, returning without doing anything
    if(src == dst)
        return;

    //cheking preconditions
    if(src_dims[3] != dst_dims[3])
        throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: source has %d channels, destination has %d", src_dims[3], dst_dims[3]).c_str());
    for(int d=0; d<3; d++)
    {
        if(src_offset[d] + src_count[d] > src_dims[d])
            throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: VOI [%u, %u) exceeds image size (%u) along axis %d", src_offset[d], src_offset[d] + src_count[d], src_dims[d], d).c_str());
        if(dst_offset[d] + src_count[d]*scaling > dst_dims[d])
        {
            //cutting copiable VOI to the largest one that can be stored into the destination image
            int old = src_count[d];
            src_count[d] = (dst_dims[d] - dst_offset[d]) / scaling; //it's ok to approximate this calculation to the floor.


            itm::warning(strprintf("--------------------- teramanager plugin [thread *] !! WARNING in copyVOI !! VOI exceeded destination dimension along axis %d, then cutting VOI from %d to %d\n",
                   d, old, src_count[d]).c_str());
        }
    }
    if(src_offset[4] + src_count[4] > src_dims[4])
        throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: VOI [%u, %u) exceeds source image size (%u) along T axis", src_offset[4], src_offset[4] + src_count[4], src_dims[4]).c_str());
    if(dst_offset[4] + src_count[4] > dst_dims[4])
        throw itm::RuntimeException(itm::strprintf("Can't copy VOI to destination image: VOI [%u, %u) exceeds destination image size (%u) along T axis", dst_offset[4], dst_offset[4] + src_count[4], dst_dims[4]).c_str());


    //quick version (with precomputed offsets, strides and counts: "1" for "dst", "2" for "src")
    const uint64 stride_t1 =  (uint64)1*              dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0];
    const uint64 offset_t1 =  (uint64)1*dst_offset[4]*dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0];
    const uint64 count_t1  =  (uint64)1*src_count[4] *dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0];
    const uint64 stride_t2 =  (uint64)1*              src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 offset_t2 =  (uint64)1*src_offset[4]*src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 stride_c1 =  (uint64)1*                             dst_dims [2] * dst_dims[1]   * dst_dims[0];
    const uint64 stride_c2 =  (uint64)1*                             src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 count_c1  =  (uint64)1*              dst_dims [3] * dst_dims [2] * dst_dims[1]   * dst_dims[0];
    const uint64 stride_k1 =  (uint64)1*                                            dst_dims[1]   * dst_dims[0]  * scaling;
    const uint64 count_k1  =  (uint64)1*                             src_count[2] * dst_dims[1]   * dst_dims[0]  * scaling;
    const uint64 offset_k2 =  (uint64)1*                             src_offset[2]* src_dims[1]   * src_dims[0];
    const uint64 stride_k2 =  (uint64)1*                                            src_dims[1]   * src_dims[0];
  //const uint64 count_k2  =  (uint64)1*                             src_count[2] * src_dims[1]   * src_dims[0];            //not used in the OR so as to speed up the inner loops
    const uint64 stride_i1 =  (uint64)1*                                                            dst_dims[0]  * scaling;
    const uint64 count_i1  =  (uint64)1*                                            src_count[1]  * dst_dims[0]  * scaling;
    const uint64 offset_i2 =  (uint64)1*                                            src_offset[1] * src_dims[0];
    const uint64 stride_i2 =  (uint64)1*                                                            src_dims[0];
  //const uint64 count_i2  =  (uint64)1*                                            src_count[1]  * src_dims[0];            //not used in the OR so as to speed up the inner loops
    const uint64 stride_j1 =  (uint64)1*                                                                           scaling;
    const uint64 count_j1  =  (uint64)1*                                                            src_count[0] * scaling;
    const uint64 offset_j2 =  (uint64)1*                                                            src_offset[0];
  //const uint64 count_j2  =  (uint64)1*                                                           src_count[0];           //not used in the OR so as to speed up the inner loops
  //const uint64 dst_dim   =  (uint64)1* dst_dims [4]* dst_dims [3] * dst_dims [2] * dst_dims [1] * dst_dims [0];

    for(int sk = 0; sk < scaling; sk++)
        for(int si = 0; si < scaling; si++)
            for(int sj = 0; sj < scaling; sj++)
            {
                const uint64 offset_k1 = dst_offset[2] * dst_dims[1]    * dst_dims[0]   * scaling + sk * dst_dims[1] * dst_dims[0] ;
                const uint64 offset_i1 =                 dst_offset[1]  * dst_dims[0]   * scaling + si * dst_dims[0];
                const uint64 offset_j1 =                                  dst_offset[0] * scaling + sj;

                uint8* const start_t1 = dst + offset_t1;
                uint8* const start_t2 = const_cast<uint8*>(src) + offset_t2;
                for(uint8 *img_t1 = start_t1, *img_t2 = start_t2; img_t1 - start_t1 < count_t1; img_t1 += stride_t1, img_t2 += stride_t2)
                {
                    for(uint8 *img_c1 = img_t1, *img_c2 = img_t2; img_c1 - img_t1 < count_c1; img_c1 += stride_c1, img_c2 += stride_c2)
                    {
                        uint8* const start_k1 = img_c1 + offset_k1;
                        uint8* const start_k2 = img_c2 + offset_k2;
                        for(uint8 *img_k1 = start_k1, *img_k2 = start_k2; img_k1 - start_k1  < count_k1; img_k1 += stride_k1, img_k2 += stride_k2)
                        {
                            uint8* const start_i1 = img_k1 + offset_i1;
                            uint8* const start_i2 = img_k2 + offset_i2;
                            for(uint8 *img_i1 = start_i1, *img_i2 = start_i2; img_i1 - start_i1  < count_i1; img_i1 += stride_i1, img_i2 += stride_i2)
                            {
                                uint8* const start_j1 = img_i1 + offset_j1;
                                uint8* const start_j2 = img_i2 + offset_j2;
                                for(uint8 *img_j1 = start_j1, *img_j2 = start_j2; img_j1 - start_j1  < count_j1; img_j1 += stride_j1, img_j2++)
                                    *img_j1 = *img_j2;
                            }
                        }
                    }
                }
             }

    /**/itm::debug(itm::LEV3, "copy VOI finished",  __itm__current__function__);
}

/**********************************************************************************
* Returns the Maximum Intensity Projection of the given ROI in a newly allocated array.
***********************************************************************************/
itm::uint8*
    CImageUtils::mip(itm::uint8 const * src,    //pointer to const data source
                        uint src_dims[5],       //dimensions of "src" along X, Y, Z, channels and T
                        uint src_offset[5],     //VOI's offset along X, Y, Z, <empty> and T
                        uint src_count[5],      //VOI's dimensions along X, Y, Z, <empty> and T
                        itm::direction dir,     //direction of projection
                        bool to_BGRA /*=false*/,//true if mip data must be stored into BGRA format
                        itm::uint8 alpha /* = 255 */)//alpha transparency (used if to_BGRA = true)
   throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV1, strprintf("src_dims = (%u x %u x %u x %u x %u), src_offset = (%u, %u, %u, %u, %u), src_count = (%u, %u, %u, %u, %u), "
                                        "direction = %d, to_BGRA = %s, alpha = %d",
                                        src_dims[0], src_dims[1],src_dims[2],src_dims[3],src_dims[4], src_offset[0],src_offset[1],src_offset[2],src_offset[3],
                                        src_offset[4],src_count[0],src_count[1],src_count[2],src_count[3], src_count[4], dir,
                                        to_BGRA ? "true" : "false", alpha).c_str(), __itm__current__function__);


    // precondition checks
    if(src_count[4] != 1)
        throw itm::RuntimeException("Maximum Intensity Projection is not supported on 5D data yet.");
    if(src_dims[3] > 3)
        throw itm::RuntimeException("Maximum Intensity Projection is not supported on real 4D data yet.");
    if(dir != itm::z)
        throw itm::RuntimeException("Maximum Intensity Projection is supported along Z only.");
    for(int d=0; d<5; d++)
    {
        if(d != 3 && (src_offset[d] + src_count[d] > src_dims[d]))
            throw itm::RuntimeException(itm::strprintf("Can't compute MIP from the selected VOI: VOI [%u, %u) exceeds image size (%u) along axis %d.", src_offset[d], src_offset[d] + src_count[d], src_dims[d], d).c_str());
        else if(d != 3 && src_offset[d] >= src_dims[d])
            throw itm::RuntimeException(itm::strprintf("Can't compute MIP from the selected VOI: invalid offset (%u) compared to image size (%u) along axis %d.", src_offset[d], src_dims[d], d).c_str());
    }

    // source strides
    const uint64 stride_t1 =  (uint64)1*              src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 count_t1  =  (uint64)1*src_count[4] *src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 offset_t1 =  (uint64)1*src_offset[4]*src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 stride_c1 =  (uint64)1*                             src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 count_c1  =  (uint64)1*              src_dims [3] * src_dims [2] * src_dims[1]   * src_dims[0];
    const uint64 stride_k1 =  (uint64)1*                                            src_dims[1]   * src_dims[0];
    const uint64 count_k1  =  (uint64)1*                             src_count[2] * src_dims[1]   * src_dims[0];
    const uint64 offset_k1 =  (uint64)1*                             src_offset[2]* src_dims[1]   * src_dims[0];
    const uint64 stride_i1 =  (uint64)1*                                                            src_dims[0];
    const uint64 count_i1  =  (uint64)1*                                            src_count[1]  * src_dims[0];
    const uint64 offset_i1 =  (uint64)1*                                            src_offset[1] * src_dims[0];
    const uint64 stride_j1 =  (uint64)1;
    const uint64 count_j1  =  (uint64)1*                                                            src_count[0];
    const uint64 offset_j1 =  (uint64)1*                                                            src_offset[0];

    // z-mip strides
    const uint64 stride_c2 =  (uint64)1*                                            src_count[1]  * src_count[0];
    const uint64 count_c2  =  (uint64)1*              src_dims [3]                * src_count[1]  * src_count[0];
    const uint64 stride_i2 =  (uint64)1*                                                            src_count[0];

    // mip size
    uint64 mip_size = to_BGRA ? (uint64)4 * src_count[1]*src_count[0] : count_c2;

    // allocation and initialization
    uint8* mip = new uint8[mip_size];
    for(int i = 0; i < mip_size; i++)
        mip[i] = 0;

    // define utility pointers
    uint8 const *start_t1, *img_t1, *img_c1, *start_k1, *img_k1, *start_i1, *img_i1, *start_j1, *img_j1 = 0;
    uint8 *img_c2, *img_i2, *img_j2 = 0;

    // mip
    if(to_BGRA)
    {
        for(start_t1 = src + offset_t1, img_t1 = start_t1; img_t1 - start_t1 < count_t1; img_t1 += stride_t1)
        {
            int c = 0;
            for(img_c1 = img_t1; img_c1 - img_t1 < count_c1; img_c1 += stride_c1, c++)
            {
                for(start_k1 = img_c1 + offset_k1, img_k1 = start_k1; img_k1 - start_k1  < count_k1; img_k1 += stride_k1)
                {
                    int y = 0;
                    for(start_i1 = img_k1 + offset_i1, img_i1 = start_i1; img_i1 - start_i1  < count_i1; img_i1 += stride_i1, y++)
                    {
                        int x=0;
                        uint64 y_stride = y*src_count[0]*4;
                        for(start_j1 = img_i1 + offset_j1, img_j1 = start_j1; img_j1 - start_j1  < count_j1; img_j1 += stride_j1, x++)
                        {
                            uint64 offset = y_stride + x*4;
                            mip[offset + c] = std::max(*img_j1, mip[offset + c]);
                        }
                    }
                }
            }
        }

        // grayscale to BGRA
        if(src_dims[3] == 1)
        {
            for(uint8* pmip = mip; pmip - mip < mip_size; pmip += 4)
            {
              //*(pmip + 0)                   is in the right place
                *(pmip + 1) = *pmip;
                *(pmip + 2) = *pmip;
                *(pmip + 3) = alpha;
            }
        }
        // RGB to BGRA
        else
        {
            for(uint8* pmip = mip; pmip - mip < mip_size; pmip += 4)
            {
                *(pmip + 3) = *(pmip + 0); // temp save R into A
                *(pmip + 0) = *(pmip + 2); // B
              //*(pmip + 1)                   G is in the right place
                *(pmip + 2) = *(pmip + 3); // R
                *(pmip + 3) = alpha;
            }
        }
    }
    else
    {
        for(start_t1 = src + offset_t1, img_t1 = start_t1; img_t1 - start_t1 < count_t1; img_t1 += stride_t1)
        {
            for(img_c1 = img_t1, img_c2 = mip; img_c1 - img_t1 < count_c1; img_c1 += stride_c1, img_c2 += stride_c2)
            {
                for(start_k1 = img_c1 + offset_k1, img_k1 = start_k1; img_k1 - start_k1  < count_k1; img_k1 += stride_k1)
                {
                    for(start_i1 = img_k1 + offset_i1, img_i1 = start_i1, img_i2 = img_c2; img_i1 - start_i1  < count_i1; img_i1 += stride_i1, img_i2 += stride_i2)
                    {
                        for(start_j1 = img_i1 + offset_j1, img_j1 = start_j1, img_j2 = img_i2; img_j1 - start_j1  < count_j1; img_j1 += stride_j1, img_j2++)
                            *img_j2 = std::max(*img_j1, *img_j2);
                    }
                }
            }
        }
    }

    return mip;
}

void CImageUtils::applyVaa3DColorMap(QImage& image, RGBA8 cmap[4][256])
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    if(image.isNull())
    {
        itm::warning("image is empty", __itm__current__function__);
        return;
    }

    if(image.format() != QImage::Format_ARGB32)
    {
        itm::warning("unsupported format, cannot apply the given color map", __itm__current__function__);
        return;
    }

    uint8* data = image.bits();
    uint64 data_size = image.width() * image.height() *(uint64)4;
    for(uint8* pdata = data; pdata - data < data_size; pdata += 4)
    {
        int i1 = *(pdata + 2);
        int i2 = *(pdata + 1);
        int i3 = *(pdata + 0);
        float o1=0,o2=0,o3=0;
        o1 = (cmap[0][i1].a/255.0)*(cmap[0][i1].r/255.0) + (cmap[1][i2].a/255.0)*(cmap[1][i2].r/255.0) + (cmap[2][i3].a/255.0)*(cmap[2][i3].r/255.0);
        o2 = (cmap[0][i1].a/255.0)*(cmap[0][i1].g/255.0) + (cmap[1][i2].a/255.0)*(cmap[1][i2].g/255.0) + (cmap[2][i3].a/255.0)*(cmap[2][i3].g/255.0);
        o3 = (cmap[0][i1].a/255.0)*(cmap[0][i1].b/255.0) + (cmap[1][i2].a/255.0)*(cmap[1][i2].b/255.0) + (cmap[2][i3].a/255.0)*(cmap[2][i3].b/255.0);
        *(pdata + 0) = std::min(255, static_cast<int>(o3*255));
        *(pdata + 1) = std::min(255, static_cast<int>(o2*255));
        *(pdata + 2) = std::min(255, static_cast<int>(o1*255));
      //*(pdata + 3)   alpha channel unchanged
    }
}


template< int operation( int, int ) >
void CImageUtils::changeImage( QImage& image, int value )
{
    QImage &im = image;
    //im.detach();
    
    if( im.colorCount() == 0 ) /* truecolor */
    {
        if( im.format() != QImage::Format_RGB32 ) /* just in case */
            im = im.convertToFormat( QImage::Format_RGB32 );
        int table[ 256 ];
        for( int i = 0; i < 256; ++i )
            table[ i ] = operation( i, value );
        if( im.hasAlphaChannel() )
        {
            for( int y = 0; y < im.height(); ++y )
            {
                QRgb* line = reinterpret_cast< QRgb* >( im.scanLine( y ));
                for( int x = 0; x < im.width(); ++x )
                    line[ x ] = qRgba( changeUsingTable( qRed( line[ x ] ), table ),
                        changeUsingTable( qGreen( line[ x ] ), table ),
                        changeUsingTable( qBlue( line[ x ] ), table ),
                        changeUsingTable( qAlpha( line[ x ] ), table ));
            }
        }
        else
        {
            for( int y = 0; y < im.height(); ++y )
            {
                QRgb* line = reinterpret_cast< QRgb* >( im.scanLine( y ));
                for( int x = 0; x < im.width(); ++x )
                    line[ x ] = qRgb( changeUsingTable( qRed( line[ x ] ), table ),
                        changeUsingTable( qGreen( line[ x ] ), table ),
                        changeUsingTable( qBlue( line[ x ] ), table ));
            }
        }
    }
    else
    {
        QVector<QRgb> colors = im.colorTable();
        for( int i = 0; i < im.colorCount(); ++i )
            colors[ i ] = qRgb( operation( qRed( colors[ i ] ), value ),
                operation( qGreen( colors[ i ] ), value ),
                operation( qBlue( colors[ i ] ), value ));
    }
//    return im;
}

void CImageUtils::changeBrightness(QImage& image, int brightness )
{
    if( brightness != 0 ) // no change
        changeImage< changeBrightness >( image, brightness );
}

void CImageUtils::changeContrast(QImage& image, int contrast )
{
    if( contrast != 100 ) // no change
        changeImage< changeContrast >( image, contrast );
}

void CImageUtils::changeGamma(QImage& image, int gamma )
{
    if( gamma == 100 ) // no change
        changeImage< changeGamma >( image, gamma );
}



/**********************************************************************************
* Returns a new, interpolated image from the two given images
***********************************************************************************/
Image4DSimple* CImageUtils::interpolateLinear(
    Image4DSimple* im1,         // first image
    Image4DSimple* im2,         // second image
    int i,                      // step  index
    int N)                      // steps number
throw (itm::RuntimeException)
{
    // checks
    if(!im1 || !im1->getRawData())
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): invalid 1st image data");
    if(!im2 || !im2->getRawData())
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): invalid 2nd image data");
    if(im1->getXDim() != im2->getXDim())
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): the two images differ in X dimensions");
    if(im1->getYDim() != im2->getYDim())
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): the two images differ in Y dimensions");
    if(im1->getZDim() != im2->getZDim())
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): the two images differ in Z dimensions");
    if(im1->getCDim() != im2->getCDim())
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): the two images differ in channel dimensions");
    if(im1->getTDim() > 1)
        throw itm::RuntimeException(itm::strprintf("in CImageUtils::interpolate_linear(): 1st image has TDim = %d (5D not supported)", im1->getTDim()));
    if(im2->getTDim() > 1)
        throw itm::RuntimeException(itm::strprintf("in CImageUtils::interpolate_linear(): 2nd image has TDim = %d (5D not supported)", im2->getTDim()));
    if(im1->getDatatype() != V3D_UINT8)
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): 1st image is not UINT8 (not supported)");
    if(im2->getDatatype() != V3D_UINT8)
        throw itm::RuntimeException("in CImageUtils::interpolate_linear(): 2nd image is not UINT8 (not supported)");

    // allocate new image
    Image4DSimple* imout = new Image4DSimple();
    imout->setXDim(im1->getXDim());
    imout->setYDim(im1->getYDim());
    imout->setZDim(im1->getZDim());
    imout->setCDim(im1->getCDim());
    imout->setTDim(im1->getTDim());
    imout->setDatatype(V3D_UINT8);
    V3DLONG data_size = imout->getXDim()*imout->getYDim()*imout->getZDim()*imout->getCDim();
    unsigned char* data = new unsigned char[data_size];
    imout->setRawDataPointer(data);


    // interpolation channel-by-channel (it would be way better in the HSV space...)
    unsigned char* data1 = im1->getRawData();
    unsigned char* data2 = im2->getRawData();
    for(V3DLONG k=0; k<data_size; k++)
        data[k] = itm::linear<unsigned char>(data1[k], data2[k], i, N);

    return imout;
}

/**********************************************************************************
* Returns a new, gaussian-noise corrupted image from the given image
***********************************************************************************/
Image4DSimple* CImageUtils::addGaussianNoise(
        Image4DSimple* im,      // input image
        float w)                // gaussian noise weight (1 = only noise, 0 = no noise)
throw (itm::RuntimeException)
{
    // checks
    if(!im || !im->getRawData())
        throw itm::RuntimeException("in CImageUtils::addGaussianNoise(): invalid image data");
    if(im->getTDim() > 1)
        throw itm::RuntimeException(itm::strprintf("in CImageUtils::addGaussianNoise(): image has TDim = %d (5D not supported)", im->getTDim()));

    // allocate new image
    Image4DSimple* imout = new Image4DSimple();
    imout->setXDim(im->getXDim());
    imout->setYDim(im->getYDim());
    imout->setZDim(im->getZDim());
    imout->setCDim(im->getCDim());
    imout->setTDim(im->getTDim());
    imout->setDatatype(im->getDatatype());
    V3DLONG data_size = imout->getTotalBytes();
    unsigned char* data = new unsigned char[data_size];
    imout->setRawDataPointer(data);

    // data corruption
    if(imout->getDatatype() == V3D_UINT8)
    {
        unsigned char* data_in = im->getRawData();
        for(V3DLONG k=0; k<data_size; k++)
            data[k] = static_cast<itm::uint8>((1-w)*data_in[k] + w*(rand()%256) +0.5f);
    }
    else if(imout->getDatatype() == V3D_UINT16)
    {
        unsigned short* data_in = (unsigned short*)(im->getRawData());
        for(V3DLONG k=0; k<data_size/2; k++)
            data[k] = static_cast<itm::uint16>((1-w)*data_in[k] + w*(rand()%65535) +0.5f);
    }
    else
        throw itm::RuntimeException("in CImageUtils::addGaussianNoise(): image is neither UINT8 nor UINT16 (not supported)");

    return imout;
}

namespace
{
    float hue2rgb(float p, float q, float t)
    {
        if(t < 0) t += 1;
        if(t > 1) t -= 1;
        if(t < 1/6) return p + (q - p) * 6 * t;
        if(t < 1/2) return q;
        if(t < 2/3) return p + (q - p) * (2.0/3 - t) * 6;
        return p;
    }
}

// convert HSL to RGB. H, S, and L should be in [0.0,1.0]
RGBA8 itm::CImageUtils::hsl2rgb(float h, float s, float l)
{
    float r=0, g=0, b=0;

    if(s == 0)
        r = g = b = l; // achromatic
    else
    {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        r = hue2rgb(p, q, h + 1.0/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0/3);
    }

    RGBA8 out;
    out.r = static_cast<unsigned char>(r * 255 + 0.5);
    out.g = static_cast<unsigned char>(r * 255 + 0.5);
    out.b = static_cast<unsigned char>(r * 255 + 0.5);
    return out;
}

namespace
{
    RGB32f getColor(float x)
    {
        RGB32f c;
        c.r = 0.0f;
        c.g = 0.0f;
        c.b = 1.0f;
        if (x >= 0.0f && x < 0.2f)
        {
            x = x / 0.2f;
            c.r = 0.0f;
            c.g = x;
            c.b = 1.0f;
        }
        else if (x >= 0.2f && x < 0.4f)
        {
            x = (x - 0.2f) / 0.2f;
            c.r = 0.0f;
            c.g = 1.0f;
            c.b = 1.0f - x;
        }
        else if (x >= 0.4f && x < 0.6f)
        {
            x = (x - 0.4f) / 0.2f;
            c.r = x;
            c.g = 1.0f;
            c.b = 0.0f;
        }
        else if (x >= 0.6f && x < 0.8f)
        {
            x = (x - 0.6f) / 0.2f;
            c.r = 1.0f;
            c.g = 1.0f - x;
            c.b = 0.0f;
        }
        else if (x >= 0.8f && x <= 1.0f)
        {
            x = (x - 0.8f) / 0.2f;
            c.r = 1.0f;
            c.g = 0.0f;
            c.b = x;
        }
        return c;
    }
}
// get n distinct colors
std::vector<RGBA8> itm::CImageUtils::distinctColors(int n)
{
    std::vector<RGBA8> colors;
    if (n < 2)
    {
        RGBA8 w;
        w.r = w.g = w.b = 255;
        colors.push_back(w);
        return colors;
    }
    float dx = 1.0f / static_cast<float>(n - 1);
    for (int i = 0; i < n; i++)
    {
        RGB32f cf = getColor(i * dx);
        RGBA8 c;
        c.g = cf.g * 255;
        c.r = cf.r * 255;
        c.b = cf.b * 255;
        colors.push_back(c);
    }
    return colors;
}
