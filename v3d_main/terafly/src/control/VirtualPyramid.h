#ifndef VIRTUALPYRAMID_H
#define VIRTUALPYRAMID_H

#include "CPlugin.h"
#include "VirtualVolume.h"

// Virtual Pyramid class
// - a container for virtual pyramid layers (not containing any data)
// - a container for actual  pyramid layers (do contain actual data + caching)
class terafly::VirtualPyramid
{
    private:

        // object members
        iim::VirtualVolume*                 highresVol;     // highest-res (unconverted) volume
        std::string                         highresPath;    // highest-res (unconverted) volume path
        std::string                         localPath;      // where local files should be stored
        std::vector< tf::VirtualPyramidLayer* > virtualPyramid;   // virtual (=do NOT contain any data) pyramid layers, from highest-res to lowest-res
        std::vector< tf::HyperGridCache*>  pyramid;        // actual (=do contain data) pyramid layers, use caching from/to disk and RAM, from highest-res to lowest-res

        // disable default constructor
        VirtualPyramid(){}

        // object utility methods
        void instanceHighresVol() throw (iim::IOException, iom::exception, tf::RuntimeException);// init highest-res volume
        void init() throw (iim::IOException, iom::exception, tf::RuntimeException);              // init metadata


    public:

        // constructor 1
        VirtualPyramid(
                std::string _highresPath,                   // highest-res (unconverted) volume path
                int reduction_factor,                       // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
                float lower_bound = 100,                    // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
                iim::VirtualVolume* _highresVol = 0)        // highest-res (unconverted) volume, if null will be instantiated on-the-fly
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // constructor 2
        VirtualPyramid(
                std::string _highresPath,                   // highest-res (unconverted) volume path
                std::vector< xyz<int> > reduction_factors,  // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
                iim::VirtualVolume* _highresVol = 0)        // highest-res (unconverted) volume, if null will be instantiated on-the-fly
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // destructor
        ~VirtualPyramid() throw(iim::IOException);


        // GET methods
        std::vector<iim::VirtualVolume*> getVirtualPyramid();

        // load volume of interest from the given resolution layer
        // - communicates with 'highresVol' (which contains highres data) and with 'pyramid' (which contain cached data)
        tf::image5D<uint8>
        loadVOI(
                xyz<size_t> start,  // xyz range [start, end)
                xyz<size_t> end,    // xyz range [start, end)
                int level)          // pyramid layer (0=highest resolution, the higher the lower the resolution)
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        friend class VirtualPyramidLayer;
};


// Virtual Pyramid Layer class
// - a wrapper built on the highest-res image to intercept its load methods
// - inherits from VirtualVolume, which makes using a Virtual Pyramid Image transparent to the client
class terafly::VirtualPyramidLayer : public iim::VirtualVolume
{
    private:

        // object members
        tf::VirtualPyramid*    parent;             // container
        int                    level;              // pyramid level (0 = highest-res, the coarser the resolution the higher)
        tf::xyz<int>           reductionFactor;    // pyramid reduction factor relative to the highest-res image

        // disable default constructor
        VirtualPyramidLayer(){}

    public:

        // constructor
        VirtualPyramidLayer(
                int _level,                         // pyramid level (0 = highest-res, the coarser the resolution the higher)
                tf::VirtualPyramid* _parent,        // container
                tf::xyz<int> _reduction_factor)     // reduction factor relative to the highest-res image
        throw (iim::IOException);

        // deconstructor
        virtual ~VirtualPyramidLayer() throw (iim::IOException);

        // GET methods
        tf::VirtualPyramid* getPyramid(){return parent;}

        // inherited pure virtual methods, to implement
        virtual void initChannels ( ) throw (iim::IOException);
        virtual iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);
        virtual iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);
        virtual float getVXL_1(){return VXL_H;}
        virtual float getVXL_2(){return VXL_V;}
        virtual float getVXL_3(){return VXL_D;}
        virtual iim::axis getAXS_1(){return parent->highresVol->getAXS_1();}
        virtual iim::axis getAXS_2(){return parent->highresVol->getAXS_2();}
        virtual iim::axis getAXS_3(){return parent->highresVol->getAXS_3();}
        virtual std::string getPrintableFormat(){return std::string("VirtualPyramid on ") + parent->highresVol->getPrintableFormat();}

        // return true if 'highresVol' downsampled by 'reduction_factor' generates a 0-sized image
        static bool isEmpty(iim::VirtualVolume* highresVol, xyz<int> _reduction_factor);

        friend class VirtualPyramid;
};


// Hyper Grid Cache class
// - designed to cache image data during visualization
// - send/receive image data to/from client
// - store cached data permanently on the disk
// - supports up to 5D images
class terafly::HyperGridCache
{
    private:

        // object members
        class CacheBlock;                       // forward-declaration
        std::string path;                       // where cache files are stored / have to be stored
        CacheBlock ******hypergrid;             // 5D array of <CacheBlock>, follows T-C-Z-Y-Z order
        tf::xyzct<size_t> block_dim;            // desired dimensions of each <CacheBlock>
        size_t nX, nY, nZ, nC, nT;              // hypergrid dimension along X, Y, Z, C (channel), and T (time)
        size_t dimX, dimY, dimZ, dimC, dimT;    // image space dimensions along X, Y, Z, C (channel), and T (time)

        // object methods
        HyperGridCache(){}                      // disable default constructor
        void load() throw (iim::IOException, tf::RuntimeException, iom::exception);		// load from disk
        void save() throw (iim::IOException, tf::RuntimeException, iom::exception);		// save to disk
        void init() throw (iim::IOException, tf::RuntimeException, iom::exception);		// init persistency files

    public:

        // constructor 1
        HyperGridCache(
                std::string _path,										// where cache files are stored / have to be stored
                tf::xyzct<size_t> _image_dim,							// image dimensions along X, Y, Z, C (channel), and T (time)
                tf::xyzct<size_t> _block_dim							// hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
					= tf::xyzct<size_t>(256,256,256,std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max()))
        throw (iim::IOException, tf::RuntimeException, iom::exception);

        // destructor
        ~HyperGridCache() throw (iim::IOException);


        // read data from cache (rescaling on-the-fly supported)
        tf::image5D<uint8>												// 5D image output
        readData(tf::voi4D<int> voi,                                    // 4D VOI in X,Y,Z,T space
                tf::active_channels<> channels,                         // channels to load
                tf::xyz<int> scaling = tf::xyz<int>(1,1,1))				// scaling along X,Y and Z (> 0 upscaling, < 0 downscaling)
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // put data into the cache (rescaling on-the-fly supported)
        void putData(
                const tf::image5D<uint8> &img,							// 5D image input
				tf::xyzt<size_t> offset,								// offset relative to (0,0,0,0), it is up(or down)scaled if needed
                tf::xyz<int> scaling = tf::xyz<int>(1,1,1))				// scaling along X,Y and Z (> 0 upscaling, < 0 downscaling) 
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // class members
        static float maximumSizeGB;             // maximum size (in Gigabytes) for this Cache (default: 1 GB)


    private:

        // Cache Block nested class
        // - stores individual portions (blocks) of cached data and the corresponding visit counts
        class CacheBlock
        {
            private:

                // object members
                HyperGridCache*   parent;                   // container
                tf::xyzct<size_t> origin;                   // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
                tf::xyzct<size_t> dims;                     // block dimensions - block has coordinates [origin, origin + dims)
                tf::uint8*        imdata;                   // cached image data
                tf::xyzct<size_t> index;                    // 5D index in the parent hypergrid
                std::string       path;                     // path of file where this block is stored
                bool              modified;                 // whether the data of this block has been modified
                int               visits;                   // #times this block has been visited (for both loading and storing of image data)

                // object utility methods
                CacheBlock(){}                          // disable default constructor
                void load() throw (iim::IOException, iom::exception, tf::RuntimeException);   // load from disk
                void save() throw (iim::IOException, iom::exception, tf::RuntimeException);   // save to disk


            public:


                // contructor 1
                CacheBlock(
                        HyperGridCache* _parent,
                        tf::xyzct<size_t> _origin,          // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
                        tf::xyzct<size_t> _dims,            // dimensions of the block
                        tf::xyzct<size_t> _index)           // 5D index in the parent hypergrid
                throw (iim::IOException, iom::exception);

                // destructor
                ~CacheBlock() throw (iim::IOException, iom::exception);

                // GET and SET methods
                tf::xyzct<size_t> getOrigin(){return origin;}
                tf::xyzct<size_t> getDims()  {return dims;}
                int getVisits()              {return visits;}
                void setVisits(int _visits)  {visits=_visits;}

                // calculate XYZT and C intersection
                template <typename T>
                tf::voi4D<T> intersection(tf::voi4D<T> xyzt)
                {
                    return tf::voi4D<T>(
                                tf::xyzt<T>(std::max(xyzt.start.x, T(origin.x)),
                                            std::max(xyzt.start.y, T(origin.y)),
                                            std::max(xyzt.start.z, T(origin.z)),
                                            std::max(xyzt.start.t, T(origin.t))),
                                tf::xyzt<T>(std::min(xyzt.end.x,   T(origin.x + dims.x)),
                                            std::min(xyzt.end.y,   T(origin.y + dims.y)),
                                            std::min(xyzt.end.z,   T(origin.z + dims.z)),
                                            std::min(xyzt.end.t,   T(origin.t + dims.t))));
                }
                std::vector<unsigned int> intersection(tf::active_channels<> chans);

                // put data from given image into current block (rescaling on-the-fly supported)
                void putData(
                    const tf::image5D<uint8> & image,       // 5D image input
					tf::voi4D<int> image_voi,				// image VOI (already scaled)
					tf::voi4D<int> block_voi,				// current block VOI (already scaled)
					tf::xyz<int> scaling					// scaling along X,Y and Z (> 0 upscaling, < 0 downscaling)
                ) throw (iim::IOException, iom::exception, tf::RuntimeException);

                // read data from block and put into image buffer (rescaling on-the-fly supported)
                void readData(
                    tf::image5D<uint8> & image,             // 5D image buffer (preallocated)
                    tf::voi4D<int> image_voi,				// image VOI (already scaled)
                    tf::voi4D<int> block_voi,				// current block VOI (already scaled)
                    tf::xyz<int> scaling					// scaling along X,Y and Z (> 0 upscaling, < 0 downscaling)
                ) throw (iim::IOException, iom::exception, tf::RuntimeException);
        };
};



#endif // VIRTUALPYRAMID_H
