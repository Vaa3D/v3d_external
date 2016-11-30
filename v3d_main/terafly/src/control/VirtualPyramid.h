#ifndef VIRTUALPYRAMID_H
#define VIRTUALPYRAMID_H

#include "CPlugin.h"
#include "VirtualVolume.h"

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
#include <QtWidgets>
#endif

// Virtual Pyramid class
// - a container for virtual pyramid layers (not containing any data)
// - a container for actual  pyramid layers (do contain actual data + caching)
class terafly::VirtualPyramid
{
    private:

        // object members
        iim::VirtualVolume*                 _highresVol;            // highest-res (unconverted) volume
        std::string                         _highresPath;           // highest-res (unconverted) volume path
        std::string                         _path;                  // where files should be stored
        std::vector< tf::VirtualPyramidLayer* > _virtualPyramid;    // virtual (=do NOT contain any data) pyramid layers, from highest-res to lowest-res
        std::vector< tf::HyperGridCache*>  _cachePyramid;           // actual (=do contain data) pyramid layers, use caching from/to disk and RAM, from highest-res to lowest-res

        // disable default constructor
        VirtualPyramid(){}

        // object utility methods
        void initPath(bool local = true) throw (iim::IOException, iom::exception, tf::RuntimeException);
        void initHighResVolume() throw (iim::IOException, iom::exception, tf::RuntimeException);
        void initPyramid(tf::xyz<size_t> block_dim	= tf::xyz<size_t>(256)) throw (iim::IOException, iom::exception, tf::RuntimeException);


    public:

        // different initializations for first-time open
        enum init_mode{
            DEFAULT,                                        // do nothing (default): all pyramid layers are initially empty (black)
            GENERATE_LOW_RES_FROM_FILE,                     // generate lowest-res pyramid layer from image file
            GENERATE_LOW_RES,                               // generate lowest-res pyramid layer from highest-res image
            GENERATE_ALL                                    // generate all pyramid layers from highest-res image
        };

        // constructor 1 (first time instance / Virtual Pyramid files do not exist)
        VirtualPyramid(
                std::string highresPath,                   // highest-res (unconverted) volume path
                int reduction_factor,                       // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
                float lower_bound = 100,                    // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
                iim::VirtualVolume* highresVol = 0,        // highest-res (unconverted) volume, if null will be instantiated on-the-fly
                init_mode mode = DEFAULT,                   // initialization mode
                const std::string & lowResImagePath = "",   // path of low-res image file (to be used when mode == GENERATE_LOW_RES_FROM_FILE)
                int sampling = 16,                          // sampling factor (to be used when mode == GENERATE_LOW_RES || GENERATE_ALL)
                int local = true,                           // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
                xyz<size_t> block_dim = xyz<size_t>(256))   // x-y-z dimensions of virtual pyramid blocks
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // constructor 2 (first time instance / Virtual Pyramid files do not exist)
        VirtualPyramid(
                std::string highresPath,                   // highest-res (unconverted) volume path
                std::vector< xyz<int> > reduction_factors,  // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
                iim::VirtualVolume* highresVol = 0,        // highest-res (unconverted) volume, if null will be instantiated on-the-fly
                init_mode mode = DEFAULT,                   // initialization mode
                const std::string & lowResImagePath = "",   // path of low-res image file (to be used when mode == GENERATE_LOW_RES_FROM_FILE)
                int sampling = 16,                          // sampling factor (to be used when mode == GENERATE_LOW_RES || GENERATE_ALL)
                int local = true,                           // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
                xyz<size_t> block_dim = xyz<size_t>(256))   // x-y-z dimensions of virtual pyramid blocks
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // constructor 3 (Virtual Pyramid files already exist)
        VirtualPyramid(
                std::string highresPath,                   // highest-res (unconverted) volume path
                iim::VirtualVolume* highresVol = 0,        // highest-res (unconverted) volume, if null will be instantiated on-the-fly
                int local = true)                           // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
        throw (iim::IOException, iom::exception, tf::RuntimeException);

        // destructor
        ~VirtualPyramid() throw(iim::IOException);


        // GET methods
        std::vector<iim::VirtualVolume*> virtualPyramid();
        std::string path(){return _path;}
        std::vector <tf::HyperGridCache*> cachePyramid(){return _cachePyramid;}

        // load volume of interest from the given resolution layer
        // - communicates with 'highresVol' (which contains highres data) and with 'pyramid' (which contain cached data)
        tf::image5D<uint8>
        loadVOI(
                xyz<size_t> start,  // xyz range [start, end)
                xyz<size_t> end,    // xyz range [start, end)
                int level)          // pyramid layer (0=highest resolution, the higher the lower the resolution)
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // clear cached data
        void clear(
                bool ask_to_save = true,    // user is asked whether to save modified data (if any)
                int layer = -1)             // layer selection (-1 = all layers)
        throw (iim::IOException, iom::exception, tf::RuntimeException);

        // return path where Virtual Pyramid data are expected to be found on local storage (i.e. executable's folder)
        static std::string getLocalPath(const std::string & _highresPath);
        // return path where Virtual Pyramid data are expected to be found on remote storage (i.e. volume's folder)
        static std::string getRemotePath(const std::string & _highresPath);
        // return true if Virtual Pyramid files are found in either local or remote storage (see getLocalPath and getRemotePath)
        static bool exists(const std::string & _highresPath);
        // return true if Virtual Pyramid files are found in local AND remote storage (see getLocalPath and getRemotePath)
        static bool bothExist(const std::string & _highresPath);
        // return true if Virtual Pyramid files are found on local storage (see getLocalPath)
        static bool existLocal(const std::string & _highresPath);
        // return true if Virtual Pyramid files are found on remote storage (see getRemotePath)
        static bool existRemote(const std::string & _highresPath);

        // return path where low res image file is expected to be found (if any)
        static std::string getLowResPath(const std::string & _highresPath);

        friend class VirtualPyramidLayer;
};


// Virtual Pyramid Layer class
// - a wrapper built on the highest-res image to intercept its load methods
// - inherits from VirtualVolume, which makes using a Virtual Pyramid Image transparent to the client
class terafly::VirtualPyramidLayer : public iim::VirtualVolume
{
    private:

        // object members
        tf::VirtualPyramid*    _parent;             // container
        int                    _level;              // pyramid level (0 = highest-res, the coarser the resolution the higher)
        tf::xyz<int>           _resamplingFactor;   // pyramid reduction factor relative to the highest-res image

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
        tf::VirtualPyramid* pyramid(){return _parent;}
        tf::xyz<int> resamplingFactor(){return _resamplingFactor;}

        // inherited pure virtual methods, to implement
        virtual void initChannels ( ) throw (iim::IOException);
        virtual iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);
        virtual iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);
        virtual float getVXL_1(){return VXL_H;}
        virtual float getVXL_2(){return VXL_V;}
        virtual float getVXL_3(){return VXL_D;}
        virtual iim::axis getAXS_1(){return _parent->_highresVol->getAXS_1();}
        virtual iim::axis getAXS_2(){return _parent->_highresVol->getAXS_2();}
        virtual iim::axis getAXS_3(){return _parent->_highresVol->getAXS_3();}
        virtual std::string getPrintableFormat(){return std::string("Virtual Pyramid on ") + _parent->_highresVol->getPrintableFormat();}
        static  std::string name(){return "Virtual Pyramid";}

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
    public:

        class CacheBlock;                           // forward-declaration

    private:

        // object members
        std::string _path;                          // where cache files are stored / have to be stored
        CacheBlock ******_hypergrid;                // 5D array of <CacheBlock>, follows T-C-Z-Y-Z order
        tf::xyzct<size_t> _block_dim;               // desired dimensions of each <CacheBlock>
        tf::xyzct<size_t> _nBlocks;                 // hypergrid dimension along X, Y, Z, C (channel), and T (time)
        tf::xyzct<size_t> _dims;                    // image space dimensions along X, Y, Z, C (channel), and T (time)

        // object methods
        HyperGridCache(){}                      // disable default constructor
        void load() throw (iim::IOException, tf::RuntimeException, iom::exception);		// load from disk
        void save() throw (iim::IOException, tf::RuntimeException, iom::exception);		// save to disk
        void init() throw (iim::IOException, tf::RuntimeException, iom::exception);		// init object
        void initHyperGrid() throw (iim::IOException, tf::RuntimeException);            // init 'hypergrid' 5D matrix

    public:

        // constructor 1
        HyperGridCache(
                std::string path,										// where cache files are stored / have to be stored
                tf::xyzct<size_t> image_dim = tf::xyzct<size_t>(),		// image dimensions along X, Y, Z, C (channel), and T (time)
                tf::xyzct<size_t> block_dim = tf::xyzct<size_t>())      // hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
        throw (iim::IOException, tf::RuntimeException, iom::exception);

        // destructor
        ~HyperGridCache() throw (iim::IOException);


        // get methods
        tf::xyzct<size_t> blockDim(){return _block_dim;}
        tf::xyzct<size_t> nBlocks(){return _nBlocks;}
        std::vector<CacheBlock*> blocks_modified();
        float currentRamUsageGB();
        float maximumRamUsageGB();

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


        // clear all data
        void clear();


    public:

        // Cache Block nested class
        // - stores individual portions (blocks) of cached data and the corresponding visit counts
        class CacheBlock
        {
            private:

                // object members
                HyperGridCache*   _parent;                  // container
                tf::xyzct<size_t> _origin;                  // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
                tf::xyzct<size_t> _dims;                    // block dimensions - block has coordinates [origin, origin + dims)
                tf::uint8*        _imdata;                  // cached image data
                tf::xyzct<size_t> _index;                   // 5D index in the parent hypergrid
                std::string       _path;                    // path of file where this block is stored
                bool              _modified;                // whether the data of this block has been modified w.r.t. its original version stored on the disk
                int               _visits;                  // #times this block has been visited (for both loading and storing of image data)

                // object utility methods
                CacheBlock(){}                          // disable default constructor
                void load() throw (iim::IOException, iom::exception, tf::RuntimeException);   // load from disk


            public:


                // contructor 1
                CacheBlock(
                        HyperGridCache* parent,
                        tf::xyzct<size_t> origin,           // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
                        tf::xyzct<size_t> dims,             // dimensions of the block
                        tf::xyzct<size_t> index)            // 5D index in the parent hypergrid
                throw (iim::IOException, iom::exception);

                // destructor
                ~CacheBlock() throw (iim::IOException, iom::exception);

                // GET and SET methods
                tf::xyzct<size_t> origin(){return _origin;}
                tf::xyzct<size_t> dims()  {return _dims;}
                int visits()              {return _visits;}
                void setVisits(int visits)  {_visits=visits;}
                bool modified(){return _modified;}
                size_t bytesPerPixel(){return sizeof(unsigned char);}
                float currentRamUsageGB(){return _imdata ? _dims.size() * bytesPerPixel() * 1.0e-9 : 0.0f;}
                float maximumRamUsageGB(){return _dims.size() * bytesPerPixel() * 1.0e-9;}

                // calculate XYZT and C intersection
                template <typename T>
                tf::voi4D<T> intersection(tf::voi4D<T> xyzt)
                {
                    return tf::voi4D<T>(
                                tf::xyzt<T>(std::max(xyzt.start.x, T(_origin.x)),
                                            std::max(xyzt.start.y, T(_origin.y)),
                                            std::max(xyzt.start.z, T(_origin.z)),
                                            std::max(xyzt.start.t, T(_origin.t))),
                                tf::xyzt<T>(std::min(xyzt.end.x,   T(_origin.x + _dims.x)),
                                            std::min(xyzt.end.y,   T(_origin.y + _dims.y)),
                                            std::min(xyzt.end.z,   T(_origin.z + _dims.z)),
                                            std::min(xyzt.end.t,   T(_origin.t + _dims.t))));
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

                // save to disk
                void save() throw (iim::IOException, iom::exception, tf::RuntimeException);

                // clear data
                void clear(){
                    if(_imdata)
                        delete[] _imdata;
                    _imdata = 0;
                    _modified = 0;
                }
        };
};



#endif // VIRTUALPYRAMID_H
