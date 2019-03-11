#ifndef VIRTUALPYRAMID_H
#define VIRTUALPYRAMID_H

#include "CPlugin.h"
#include "VirtualVolume.h"

#if defined(USE_Qt5)
#include <QtWidgets>
#endif

// Virtual Pyramid class: builds a virtual image pyramid on top of real (unconverted) image data
class terafly::VirtualPyramid
{
    // new types used by this class
    public:

        // different initializations for first-time open
        enum init_mode{
            DEFAULT,                                        // do nothing (default): all pyramid layers are initially empty (black)
            GENERATE_LOW_RES_FROM_FILE,                     // generate lowest-res pyramid layer from image file
            GENERATE_LOW_RES,                               // generate lowest-res pyramid layer from highest-res image
            GENERATE_ALL                                    // generate all pyramid layers from highest-res image
        };

        // different methods for filling empty=unseen=unexplored regions at visualization time
        enum empty_filling{
            RAW,                                            // display raw (0s) values (default)
            SALT_AND_PEPPER,                                // add salt and pepper
            SOLID                                           // display solid color
        };

        // different refill strategies
        enum refill_strategy{
            REFILL_RANDOM,                                  // random blocks
            REFILL_ZYX,                                     // sequential z-y-x blocks
            REFILL_CENTER                                   // center blocks first
        };


    private:

        // object members
        iim::VirtualVolume*                 _vol;                   // volume: multi-dimensional (unconverted) image
        std::string                         _volPath;               // volume path
        std::string                         _path;                  // where files should be stored
        std::vector< tf::VirtualPyramidLayer* > _virtualPyramid;    // virtual (=do NOT contain any data) pyramid layers (ordered by descending resolution)
        std::vector< tf::HyperGridCache*>  _cachePyramid;           // actual (=do contain data) pyramid 'cache' layers: cache data from/to disk and RAM at all resolution layers (ordered by descending resolution)

        // disable default constructor
        VirtualPyramid(){}

        // object utility methods
        void initPath(bool local = true) throw (iim::IOException, iom::exception, tf::RuntimeException);
        void initVolume() throw (iim::IOException, iom::exception, tf::RuntimeException);
        void initPyramid(
                tf::xyz<size_t> block_dim	= tf::xyz<size_t>(256), // block dimensions
                const std::string block_format = ".tif"             // block file format
        ) throw (iim::IOException, iom::exception, tf::RuntimeException);


    public:

        // constructor 1 (first time instance / Virtual Pyramid files do not exist)
        VirtualPyramid(
                std::string highresPath,                    // highest-res (unconverted) volume path
                int reduction_factor,                       // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
                float lower_bound = 100,                    // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
                iim::VirtualVolume* highresVol = 0,         // highest-res (unconverted) volume, if null will be instantiated on-the-fly
                init_mode mode = DEFAULT,                   // initialization mode
                const std::string & lowResImagePath = "",   // path of low-res image file (to be used when mode == GENERATE_LOW_RES_FROM_FILE)
                int sampling = 16,                          // sampling factor (to be used when mode == GENERATE_LOW_RES || GENERATE_ALL)
                int local = true,                           // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
                xyz<size_t> block_dim = xyz<size_t>(256),   // x-y-z dimensions of virtual pyramid blocks
                const std::string block_format = ".tif")    // block file format
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // constructor 2 (first time instance / Virtual Pyramid files do not exist)
        VirtualPyramid(
                std::string highresPath,                    // highest-res (unconverted) volume path
                std::vector< xyz<int> > reduction_factors,  // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
                iim::VirtualVolume* highresVol = 0,         // highest-res (unconverted) volume, if null will be instantiated on-the-fly
                init_mode mode = DEFAULT,                   // initialization mode
                const std::string & lowResImagePath = "",   // path of low-res image file (to be used when mode == GENERATE_LOW_RES_FROM_FILE)
                int sampling = 16,                          // sampling factor (to be used when mode == GENERATE_LOW_RES || GENERATE_ALL)
                int local = true,                           // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
                xyz<size_t> block_dim = xyz<size_t>(256),   // x-y-z dimensions of virtual pyramid blocks
                const std::string block_format = ".tif")    // block file format
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
        // - communicates with 'highresVol' (which contains highres data) and with 'pyramid' (which contains cached data)
        tf::image5D<uint8>
        loadVOI(
                xyz<size_t> start,  // xyz range [start, end)
                xyz<size_t> end,    // xyz range [start, end)
                int level)          // pyramid layer (0=highest resolution, the higher the lower the resolution)
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // refills pyramid by searching the first noncomplete block in the given cache layer and VOI and populating it with real image data
        // - image data are taken from the unconverted image
        // - block dim (if not provided) = tile dim if volume is tiled, otherwise block dim = block dim of highest-res cache layer
        bool                                                            // return true if refill was successful, otherwise return false (no empty regions found)
        refill(
                int cache_level = -1,                                   // cache level where to search the 'emptiest' region (default: lowest-res cache layer)
                iim::voi3D<> VOI = iim::voi3D<>::biggest(),             // volume of interest in the 'cache_level' coordinate system (default: the entire volume)
                refill_strategy strategy = REFILL_RANDOM,               // refill strategy
                tf::xyz<size_t> block_dim = tf::xyz<size_t>::biggest()  // block dimension
        )
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // clear cached data
        void clear(
                bool ask_to_save = true,    // user is asked whether to save modified data (if any)
                int layer = -1)             // layer selection (-1 = all layers)
        throw (iim::IOException, iom::exception, tf::RuntimeException);


        // *** PATH (local, remote, lowres image, ...) getters and checkers ***
        // return path where Virtual Pyramid data are expected to be found on local storage (i.e. executable's folder)
        static std::string pathLocal(const std::string & _volPath);
        // return path where Virtual Pyramid data are expected to be found on remote storage (i.e. volume's folder)
        static std::string pathRemote(const std::string & _volPath);
        // return true if Virtual Pyramid files are found in either local or remote storage (see getLocalPath and getRemotePath)
        static bool exist(const std::string & _volPath);
        // return true if Virtual Pyramid files are found in local AND remote storage (see getLocalPath and getRemotePath)
        static bool existTwice(const std::string & _volPath);
        // return true if Virtual Pyramid files are found on local storage (see getLocalPath)
        static bool existOnLocal(const std::string & _volPath);
        // return true if Virtual Pyramid files are found on remote storage (see getRemotePath)
        static bool existOnRemote(const std::string & _volPath);
        // return path where low res image file is expected to be found (if any)
        static std::string pathLowRes(const std::string & _volPath);


        // pyramid size predictors
        static float predictGB(
            iim::VirtualVolume* highresVol,             // highest-res (unconverted) volume
            int reduction_factor,                       // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
            float lower_bound                           // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
        ) throw (tf::RuntimeException);
        static float predictGB(
            iim::VirtualVolume* highresVol,             // highest-res (unconverted) volume
            std::vector< xyz<int> > reduction_factors   // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
        ) throw (tf::RuntimeException);


        // class options
        static empty_filling _unexploredFillingMethod;  // determines appearance of unexplored image space
        static unsigned char _unexploredIntensityVal;   // intensity level of unexplored voxels
        static float _unexploredSaltAndPepperPerc;      // salt & pepper percentage for salt & pepper filling of unexplored space
        static bool _freezeHighestRes;                  // if true, no new data are loaded/propagated from the highest res
        static bool _cacheHighestRes;                   // if true, also the highest res data are cached and saved

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
        virtual iim::axis getAXS_1(){return _parent->_vol->getAXS_1();}
        virtual iim::axis getAXS_2(){return _parent->_vol->getAXS_2();}
        virtual iim::axis getAXS_3(){return _parent->_vol->getAXS_3();}
        virtual std::string getPrintableFormat(){return std::string("Virtual Pyramid on ") + _parent->_vol->getPrintableFormat();}
        static  std::string name(){return "Virtual Pyramid";}
        virtual bool isTiled(iim::dimension d) {return false;}
        virtual std::vector< iim::voi3D<size_t> > tilesXYZ() {return std::vector< iim::voi3D<size_t> >();}

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
        std::string _block_fmt;                     // block file format (e.g. ".tif", ".v3draw", ...)
        tf::xyzct<size_t> _nBlocks;                 // hypergrid dimension along X, Y, Z, C (channel), and T (time)
        tf::xyzct<size_t> _dims;                    // image space dimensions along X, Y, Z, C (channel), and T (time)

        // object methods
        HyperGridCache(){}                          // disable default constructor
        void load() throw (iim::IOException, tf::RuntimeException, iom::exception);		// load from disk
        void save() throw (iim::IOException, tf::RuntimeException, iom::exception);		// save to disk
        void init() throw (iim::IOException, tf::RuntimeException, iom::exception);		// init object
        void initHyperGrid() throw (iim::IOException, tf::RuntimeException);            // init 'hypergrid' 5D matrix

    public:

        // constructor 1
        HyperGridCache(
                std::string path,										// where cache files are stored / have to be stored
                tf::xyzct<size_t> image_dim = tf::xyzct<size_t>(),		// image dimensions along X, Y, Z, C (channel), and T (time)
                tf::xyzct<size_t> block_dim = tf::xyzct<size_t>(),      // hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
                const std::string block_fmt = ".tif")                   // hypergrid block file format
        throw (iim::IOException, tf::RuntimeException, iom::exception);

        // destructor
        ~HyperGridCache() throw (iim::IOException);


        // get block size
        tf::xyzct<size_t> blockDim(){return _block_dim;}

        // get block format (e.g. ".tif", ".v3draw", ...)
        std::string blockFormat(){return _block_fmt;}

        // get number of blocks
        tf::xyzct<size_t> nBlocks(){return _nBlocks;}

        // get dimension
        tf::xyzct<size_t> dims(){return _dims;}

        // get blocks that have changed from pyramid startup
        std::vector<CacheBlock*> blocksChanged();

        // get current RAM usage in Gigabytes
        float memoryUsed();

        // get maximum RAM usage in Gigabytes
        float memoryMax();

        // completeness index between 0 (0% explored) and 1 (100% explored)
        // it is calculated by counting 'empty' voxels in the given VOI
        float completeness(
                iim::voi3D<> voi = iim::voi3D<>::biggest(),
                bool force_load_image = false
        ) throw (iim::IOException, iom::exception, tf::RuntimeException);


        // read data from the cache (rescaling on-the-fly supported)
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
        // - stores individual portions (blocks) of cached data and the corresponding visit and empty counts
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
                bool              _hasChanged;                // whether the data of this block has been modified w.r.t. its original version stored on the disk
                int               _visits;                  // # of times this block has been visited (load and/or store)
                size_t            _emptycount;              // # of empty voxels (= 0 with '0' reserved for empty voxels only / values start from 1)

                // object utility methods
                CacheBlock(){}                              // disable default constructor
                void load() throw (iim::IOException, iom::exception, tf::RuntimeException);   // load from disk
                void updateEmptyCount();                    // update empty voxel count


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

                // get origin
                tf::xyzct<size_t> origin(){return _origin;}

                // get dimensions
                tf::xyzct<size_t> dims()  {return _dims;}

                // get and set visits count
                int visits()              {return _visits;}
                void setVisits(int visits)  {_visits=visits;}

                // get and set empty count
                size_t emptyCount() { return _emptycount;}
                size_t emptyCountInVOI( iim::voi3D<> voi = iim::voi3D<>::biggest(), bool force_load_image = false);
                void setEmptyCount(size_t emptycount)  {_emptycount=emptycount;}

                // whether this block has changed (contains new data) since the last data fetch
                bool hasChanged(){return _hasChanged;}

                // get bytes per pixel
                size_t bytesPerPixel(){return sizeof(unsigned char);}

                // get current RAM usage in Gigabytes
                float memoryUsed(){return _imdata ? _dims.size() * bytesPerPixel() * 1.0e-9 : 0.0f;}

                // get maximum RAM usage in Gigabytes
                float memoryMax(){return _dims.size() * bytesPerPixel() * 1.0e-9;}

                // 3D intersection
                template <typename T>
                iim::voi3D<T> intersection( iim::voi3D<T> voi )
                {
                    return iim::voi3D<>(
                                iim::xyz<T>(
                                            std::max(voi.start.x, T(_origin.x) ),
                                            std::max(voi.start.y, T(_origin.y) ),
                                            std::max(voi.start.z, T(_origin.z) ) ),
                                iim::xyz<T>(
                                            std::min(voi.end.x,   T(_origin.x + _dims.x) ),
                                            std::min(voi.end.y,   T(_origin.y + _dims.y) ),
                                            std::min(voi.end.z,   T(_origin.z + _dims.z) ) ) );
                }

                // 4D intersection
                template <typename T>
                tf::voi4D<T> intersection(tf::voi4D<T> voi)
                {
                    return tf::voi4D<T>(
                                tf::xyzt<T>(
                                            std::max(voi.start.x, T(_origin.x)),
                                            std::max(voi.start.y, T(_origin.y)),
                                            std::max(voi.start.z, T(_origin.z)),
                                            std::max(voi.start.t, T(_origin.t))),
                                tf::xyzt<T>(
                                            std::min(voi.end.x,   T(_origin.x + _dims.x)),
                                            std::min(voi.end.y,   T(_origin.y + _dims.y)),
                                            std::min(voi.end.z,   T(_origin.z + _dims.z)),
                                            std::min(voi.end.t,   T(_origin.t + _dims.t))));
                }
                // channel intersection
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
                void clear()
                {
                    if(_imdata)
                        delete[] _imdata;
                    _imdata = 0;
                    _hasChanged = 0;
                }
        };
};



#endif // VIRTUALPYRAMID_H
