#ifndef VIRTUALPYRAMID_H
#define VIRTUALPYRAMID_H

#include "CPlugin.h"
#include "VirtualVolume.h"

// Virtual Pyramid class
// - a container for virtual pyramid layers
// - an access point for virtual pyramid layers
class teramanager::VirtualPyramid
{
    private:

        // object members
        iim::VirtualVolume*                 highresVol;     // highest-res (unconverted) volume
        std::string                         highresPath;    // highest-res (unconverted) volume path
        std::vector< itm::VirtualPyramidLayer* > layers;    // virtual pyramid layers, from lowest-to-highest res

        // disable default constructor
        VirtualPyramid(){}

        // constructor 1
        VirtualPyramid(
                iim::VirtualVolume* _highresVol,            // highest-res (unconverted) volume
                std::string _highresPath,                   // highest-res (unconverted) volume path
                int reduction_factor,                       // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
                float lower_bound = 100)                    // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
        throw (itm::RuntimeException);

        // constructor 2
        VirtualPyramid(
                iim::VirtualVolume* _highresVol,            // highest-res (unconverted) volume
                std::string _highresPath,                   // highest-res (unconverted) volume path
                std::vector< xyz<int> > reduction_factors)  // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
        throw (itm::RuntimeException);

        // destructor
        ~VirtualPyramid() throw(itm::RuntimeException);

        // class members
        static VirtualPyramid* uniqueInstance;              // singleton design pattern

    public:


        // public singleton constructor 1
        static VirtualPyramid* instance(iim::VirtualVolume* _highresVol, std::string _highresPath, int reduction_factor, float lower_bound = 100) throw (itm::RuntimeException){
            if(uniqueInstance == 0)
                uniqueInstance = new VirtualPyramid(_highresVol, _highresPath, reduction_factor, lower_bound);
            return uniqueInstance;
        }

        // public singleton constructor 1
        static VirtualPyramid* instance(iim::VirtualVolume* _highresVol, std::string _highresPath, std::vector< xyz<int> > reduction_factors) throw (itm::RuntimeException){
            if(uniqueInstance == 0)
                uniqueInstance = new VirtualPyramid(_highresVol, _highresPath, reduction_factors);
            return uniqueInstance;
        }


        // GET methods
        std::vector<iim::VirtualVolume*> getLayers(){return std::vector<iim::VirtualVolume*>(layers.begin(), layers.end());}
        std::string getHighestResVolumePath(){return highresPath;}
        static VirtualPyramid* getInstance() throw (itm::RuntimeException)
        {
            if(uniqueInstance)
                return uniqueInstance;
            else
                throw itm::RuntimeException("Cannot get VirtualPyramid unique instance: not yet instantiated");
        }
        static bool isInstantiated(){ return uniqueInstance != 0;}


        // public singleton deconstructor
        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
};


// Virtual Pyramid Layer class
// - a wrapper built on the highest-res image to intercept its load methods
// - inherits from VirtualVolume, which makes using a Virtual Pyramid Image transparent to the client
// - communicates with VirtualPyramidCache
//     - the load() methods invoked on the highest-res layer  SEND    image data TO   the cache
//     - the load() methods invoked on the lower-res   layers RECEIVE image data FROM the cache
class teramanager::VirtualPyramidLayer : public :: iim::VirtualVolume
{
    private:

        // object members
        iim::VirtualVolume*     highresVol;         // highest-res (unconverted) volume
        int                     level;              // pyramid level (0 = highest-res, the coarser the resolution the higher)
        VirtualPyramid*         pyramid;            // container

        // disable default constructor
        VirtualPyramidLayer(){}

    public:

        // constructor
        VirtualPyramidLayer(
                iim::VirtualVolume* _highresVol,    // highest-res (unconverted) image
                int _level,                         // pyramid level (0 for the highest-res, the coarser the resolution the higher)
                VirtualPyramid* _pyramid,           // container
                xyz<int> reduction_factor)          // reduction factor relative to the highest-res image
        throw (itm::RuntimeException);

        // deconstructor
        virtual ~VirtualPyramidLayer() throw (itm::RuntimeException);

        // GET methods
        VirtualPyramid* getPyramid(){return pyramid;}

        // inherited pure virtual methods, to implement
        virtual void initChannels ( ) throw (iim::IOException);
        virtual iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);
        virtual iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);
        virtual float getVXL_1(){return VXL_H;}
        virtual float getVXL_2(){return VXL_V;}
        virtual float getVXL_3(){return VXL_D;}
        virtual iim::axis getAXS_1(){return highresVol->getAXS_1();}
        virtual iim::axis getAXS_2(){return highresVol->getAXS_2();}
        virtual iim::axis getAXS_3(){return highresVol->getAXS_3();}
        virtual std::string getPrintableFormat(){return std::string("VirtualPyramid on ") + highresVol->getPrintableFormat();}
};


// Virtual Pyramid Cache class
// - designed to cache data during visualization
// - send/receive image data to/from Virtual Pyramid Layers
// - store cached data permanently on the disk
class teramanager::VirtualPyramidCache
{
    public:

        // constructor 1
        VirtualPyramidCache(
                std::string _path,                                                      // where cache files are stored / have to be stored
                size_t _dimX, size_t _dimY, size_t _dimZ, size_t _dimC, size_t _dimT,   // image dimensions along X, Y, Z, C (channel), and T (time)
                xyzct<size_t> block_dim = xyzct<size_t>(256,256,256,inf<size_t>(),inf<size_t>())) // hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
        throw (iim::IOException, itm::RuntimeException);


        // read data from the cache (downsampling on-the-fly supported)
        image_5D<uint8>                                         // <image data, image data size> output
        readData(
                xyzct<size_t> start,                            // start coordinate in the current 5D image space
                xyzct<size_t> end,                              // end coordinate in the current 5D image space
                xyz<int> downsamplingFactor = xyz<int>(1,1,1))  // downsampling factors along X,Y and Z
        throw (iim::IOException);


        // put data into the cache (downsampling on-the-fly supported)
        void putData(
                const image_5D<uint8>,                          // image data array, follows T-C-Z-Y-Z order
                xyzct<size_t> shift,                               // shift relative to (0,0,0,0,0)
                xyz<int> downsamplingFactor = xyz<int>(1,1,1))  // downsampling factors along X,Y and Z
        throw (iim::IOException);


        // class members
        static float maximumSizeGB;             // maximum size (in Gigabytes) for this Cache (default: 1 GB)


    private:

        // object members
        class CacheBlock;                       // forward-declaration
        std::string path;                       // where cache files are stored / have to be stored
        CacheBlock *****hypergrid;              // 5D array of <CacheBlock>, follows T-C-Z-Y-Z order
        size_t nX, nY, nZ, nC, nT;              // hypergrid dimension along X, Y, Z, C (channel), and T (time)
        size_t dimX, dimY, dimZ, dimC, dimT;    // image space dimensions along X, Y, Z, C (channel), and T (time)


        // object methods
        VirtualPyramidCache(){}                 // disable default constructor
        void load() throw (iim::IOException);   // load from disk
        void save() throw (iim::IOException);   // save to disk


        // Cache Block class
        // - stores individual portions (blocks) of cached data and the corresponding visit counts
        class CacheBlock
        {
            public:

                // object members
                xyzct<size_t> origin;                   // origin coordinate of the block in the image5D (xyz+channel+time) space, start at (0,0,0,0,0)
                image_5D<uint8> imdata;                 // cached image data
                int visits;                             // #times this block has been visited (for both loading and storing of image data)

                // default constructor
                CacheBlock() throw (iim::IOException);

                // contructor 1
                CacheBlock(
                        xyzct<size_t> _origin,
                        xyzct<size_t> _size)
                throw (iim::IOException);

                // load/save methods
                void load() throw (iim::IOException);   // load from disk
                void save() throw (iim::IOException);   // save to disk
        };
};



#endif // VIRTUALPYRAMID_H
