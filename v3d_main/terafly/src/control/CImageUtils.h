#ifndef CIMAGEUTILS_H
#define CIMAGEUTILS_H

#include "CPlugin.h"
#include "v3d_interface.h"

class terafly::CImageUtils
{
    private:

        CImageUtils();  // no constructors available. This class contains static methods only.

    public:

        /**********************************************************************************
        * Copy the given VOI from "src" to "dst". Offsets and rescaling on-the-fly are supported.
        ***********************************************************************************/
        static void
            copyRescaleVOI(
                tf::uint8 const * src,          // pointer to const data source
                uint src_dims[5],				// dimensions of "src" along X, Y, Z, channels and T
                uint src_offset[5],				// VOI's offset along X, Y, Z, <empty> and T
                uint src_count[5],				// VOI's dimensions along X, Y, Z, <empty> and T
                tf::uint8* dst,					// pointer to data destination
                uint dst_dims[5],				// dimensions of "dst" along X, Y, Z, channels and T
                uint dst_offset[5],				// offset of "dst" along X, Y, Z, <empty> and T
                tf::xyz<int> scaling = tf::xyz<int>(1,1,1))	// rescaling factors along X,Y,Z (> 0 upscaling, < 0 rescaling)
        throw (tf::RuntimeException);


        /**********************************************************************************
        * Copy the given VOI from "src" to "dst". Offsets and upscaling are supported.
        ***********************************************************************************/
        static void
            upscaleVOI(tf::uint8 const * src,	// pointer to const data source
                uint src_dims[5],				// dimensions of "src" along X, Y, Z, channels and T
                uint src_offset[5],				// VOI's offset along X, Y, Z, <empty> and T
                uint src_count[5],				// VOI's dimensions along X, Y, Z, <empty> and T
                tf::uint8* dst,					// pointer to data destination
                uint dst_dims[5],				// dimensions of "dst" along X, Y, Z, channels and T
                uint dst_offset[5],				// offset of "dst" along X, Y, Z, <empty> and T
                tf::xyz<int> scaling = tf::xyz<int>(1,1,1))				// upscaling factors along X,Y,Z (positive integers only)
        throw (tf::RuntimeException);

        /*****************************************************************************************
        * Copy the given VOI from "src" to "dst". Offsets and downscaling on-the-fly are supported.
        ******************************************************************************************/
        static void
            downscaleVOI(
				tf::uint8 const * src,			// pointer to const data source
                uint src_dims[5],				// dimensions of "src" along X, Y, Z, channels and T
                uint src_offset[5],				// VOI's offset along X, Y, Z, <empty> and T
                uint src_count[5],				// VOI's dimensions along X, Y, Z, <empty> and T
                tf::uint8* dst,					// pointer to data destination (preallocated, 0-initialized)
                uint dst_dims[5],				// dimensions of "dst" along X, Y, Z, channels and T
                uint dst_offset[5],				// offset of "dst" along X, Y, Z, <empty> and T
                tf::xyz<int> scaling = tf::xyz<int>(1,1,1))				// downscaling factors along X,Y,Z (positive integers only)
        throw (tf::RuntimeException);

        /**********************************************************************************
        * Returns the Maximum Intensity Projection of the given VOI in a newly allocated array.
        ***********************************************************************************/
        static tf::uint8*
            mip(tf::uint8 const * src,     //pointer to const data source
                   uint src_dims[5],        //dimensions of "src" along X, Y, Z, channels and T
                   uint src_offset[5],      //VOI's offset along X, Y, Z, <empty> and T
                   uint src_count[5],       //VOI's dimensions along X, Y, Z, <empty> and T
                   tf::direction dir,      //direction of projection
                   bool to_BGRA = false,    //true if mip data must be stored into BGRA format
                   tf::uint8 alpha = 255)  //alpha transparency (used if to_BGRA = true)
           throw (tf::RuntimeException);


        /**********************************************************************************
        * QImage manipulation functions (brightness, contrast, gamma correction, colormap)
        ***********************************************************************************/
        static void applyVaa3DColorMap(QImage& image, RGBA8 cmap[4][256]);
        static void changeBrightness( QImage& image, int brightness );
        static void changeContrast(QImage &image, int contrast );
        static void changeGamma(QImage& image, int gamma );
        inline
        static int changeBrightness( int value, int brightness ){
            return kClamp( value + brightness * 255 / 100, 0, 255 );
        }
        inline
        static int changeContrast( int value, int contrast ){
            return kClamp((( value - 127 ) * contrast / 100 ) + 127, 0, 255 );
        }
        inline
        static int changeGamma( int value, int gamma ){
            return kClamp( int( pow( value / 255.0, 100.0 / gamma ) * 255 ), 0, 255 );
        }
        inline
        static int changeUsingTable( int value, const int table[] ){
            return table[ value ];
        }
        template< int operation( int, int ) >
        static
        void changeImage(QImage& image, int value );
        /*********************************************************************************/

        static inline RGBA8 vaa3D_color(tf::uint8 r, tf::uint8 g, tf::uint8 b){
            RGBA8 color;
            color.r = r;
            color.g = g;
            color.b = b;
            return color;
        }

        // convert HSL to RGB. H, S, and L should be in [0.0,1.0]
        static RGBA8 hsl2rgb(float h, float s, float l);

        // get n distinct colors
        static std::vector<RGBA8> distinctColors(int n);

        /**********************************************************************************
        * Returns a new, interpolated image from the two given images
        ***********************************************************************************/
        static Image4DSimple* interpolateLinear(
            Image4DSimple* im1,         // first image
            Image4DSimple* im2,         // second image
            int i,                      // step  index
            int N)                      // steps number
        throw (tf::RuntimeException);


        /**********************************************************************************
        * Returns a new, gaussian-noise corrupted image from the given image
        ***********************************************************************************/
        static Image4DSimple* addGaussianNoise(
                Image4DSimple* im,      // input image
                float w)                // gaussian noise weight (1 = only noise, 0 = no noise)
        throw (tf::RuntimeException);


        /**********************************************************************************
        * Maps coordinate from one image space to another
        ***********************************************************************************/
        template <typename T>
        static inline T
        mapCoord(
                T coord,                // coordinate given in the source image space
                T source_space_dim,     // source image space dimension
                T target_space_dim,     // target image space dimension
                bool roundRes = false,  // whether to round the result to the nearest integer
                bool roundScale = false // whether to round the scale factor to the nearest integer
                )
        {
//            printf("mapCoord: coord(%s), source_space_dim(%s), target_space_dim(%s)\n",
//                   tf::num2str<T>(coord).c_str(), tf::num2str<T>(source_space_dim).c_str(), tf::num2str<T>(target_space_dim).c_str());
            // special case: boundary coordinate
            if(coord == source_space_dim)
            {
//                printf("mapCoord (boundary coordinate): return %s\n", tf::num2str<T>(target_space_dim).c_str());
                return target_space_dim;
            }

            // special case: unary image space
            if(source_space_dim == 1)
            {
//                printf("mapCoord (unary image space): return %d\n", coord ? target_space_dim : 0);
                return coord ? target_space_dim : 0;
            }

            // all other cases
            float rescale = float(source_space_dim) / target_space_dim;
            if(roundScale)
                rescale = tf::round(rescale);
//            printf("mapCoord (all other cases): rescale = %f, return %s\n", rescale, tf::num2str<T>(T( roundRes ? tf::round(coord/rescale) : coord/rescale)).c_str());
            return T( roundRes ? tf::round(coord/rescale) : coord/rescale) ;
        }
};

#endif // CIMAGEUTILS_H
