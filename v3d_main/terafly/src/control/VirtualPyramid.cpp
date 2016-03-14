#include "VirtualPyramid.h"


/********************************
*   VIRTUAL PYRAMID definitions *
*********************************
---------------------------------------------------------------------------------------------------------------------------*/
itm::VirtualPyramid* itm::VirtualPyramid::uniqueInstance = 0;

// VirtualPyramid constructor 1
itm::VirtualPyramid::VirtualPyramid(
        iim::VirtualVolume*   _highresVol,        // highest-res (unconverted) volume
        std::string           _highresPath,       // highest-res (unconverted) volume path
        int                 reduction_factor,     // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
        float lower_bound /*= 100*/)              // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    uniqueInstance = this;

    highresVol = _highresVol;
    highresPath = _highresPath;

    // iteratively add a new layer until the lowest-res layer has size > lower_bound
    int i = 0;
    do
    {
        layers.push_back(new VirtualPyramidLayer(highresVol, i, this, xyz<int>(pow(reduction_factor, i),pow(reduction_factor, i),pow(reduction_factor, i))));
        i++;
    }
    while (layers.back()->getMVoxels() > lower_bound);

    // eliminate lowest-res layer if size = 0
    if(layers.back()->getMVoxels() == 0)
        layers.pop_back();

    // order from lowest-res to highest-res
    std::reverse(layers.begin(), layers.end());
}


// VirtualPyramid constructor 2
itm::VirtualPyramid::VirtualPyramid(iim::VirtualVolume*   _highresVol,          // highest-res (unconverted) volume
        std::string                                       _highresPath,         // highest-res (unconverted) volume path
        std::vector< xyz<int> >                           reduction_factors)    // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
throw (itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    uniqueInstance = this;
    highresVol = _highresVol;
    highresPath = _highresPath;

    // generate layers according to the given reduction factors
    VirtualPyramidLayer* layer = 0;
    for(int i=0; i<reduction_factors.size(); i++)
        if( (layer=new VirtualPyramidLayer(highresVol, i, this, reduction_factors[i]))->getMVoxels() > 0)
            layers.push_back(layer);

    // order from lowest-res to highest-res
    std::reverse(layers.begin(), layers.end());
}

// VirtualPyramid deconstructor
itm::VirtualPyramid::~VirtualPyramid() throw(itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);
    for(size_t i=0; i<layers.size(); i++)
        delete layers[i];
    delete highresVol;
}
/*----END VIRTUAL PYRAMID section -----------------------------------------------------------------------------------------*/






/**************************************
*   VIRTUAL PYRAMID LAYER definitions *
***************************************
---------------------------------------------------------------------------------------------------------------------------*/
// VirtualPyramidLayer constructor
itm::VirtualPyramidLayer::VirtualPyramidLayer(
        iim::VirtualVolume* _highresVol,    // highest-res (unconverted) image
        int _level,                         // pyramid level (0 for the highest-res, the coarser the resolution the higher)
        VirtualPyramid* _pyramid,           // container
        xyz<int> reduction_factor)          // reduction factor relative to the highest-res image
throw (itm::RuntimeException) : VirtualVolume()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    highresVol = _highresVol;
    level = _level;
    pyramid = _pyramid;

    VXL_V = highresVol->getVXL_V()*reduction_factor.y;
    VXL_H = highresVol->getVXL_H()*reduction_factor.x;
    VXL_D = highresVol->getVXL_D()*reduction_factor.z;

    ORG_V = highresVol->getORG_V();
    ORG_H = highresVol->getORG_H();
    ORG_D = highresVol->getORG_D();

    DIM_C = highresVol->getDIM_C();
    BYTESxCHAN = highresVol->getBYTESxCHAN();
    initChannels();

    DIM_V = itm::round(highresVol->getDIM_V()/static_cast<float>(reduction_factor.y));
    DIM_H = itm::round(highresVol->getDIM_H()/static_cast<float>(reduction_factor.x));
    DIM_D = itm::round(highresVol->getDIM_D()/static_cast<float>(reduction_factor.z));
}

void itm::VirtualPyramidLayer::initChannels() throw (iim::IOException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    static bool first_time = true;
    if(first_time)
        highresVol->initChannels();
    else
        first_time = false;
    active = highresVol->getActiveChannels();
    n_active = highresVol->getNACtiveChannels();
}

// VirtualPyramidLayer deconstructor
itm::VirtualPyramidLayer::~VirtualPyramidLayer()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);
    active = 0;
    n_active = 0;
}


// VirtualPyramidLayer load method to uint32
iim::real32* itm::VirtualPyramidLayer::loadSubvolume_to_real32(
        int V0 /* = -1 */,
        int V1 /* = -1 */,
        int H0 /* = -1 */,
        int H1 /* = -1 */,
        int D0 /* = -1 */,
        int D1 /* = -1 */)
throw (iim::IOException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    throw itm::RuntimeException("load to real32 from VirtualPyramidLayer not yet implemented");
}


// VirtualPyramidLayer load method to uint8
iim::uint8* itm::VirtualPyramidLayer::loadSubvolume_to_UINT8(
        int V0 /* = -1 */,
        int V1 /* = -1 */,
        int H0 /* = -1 */,
        int H1 /* = -1 */,
        int D0 /* = -1 */,
        int D1 /* = -1 */,
        int *channels /* = 0 */,
        int ret_type /* =iim::DEF_IMG_DEPTH */)
throw (iim::IOException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // highest-res layer is just a wrapper for the highest-res image
    if(level == 0)
    {
        // load data from highest-res image
        iim::uint8* data = highresVol->loadSubvolume_to_UINT8(V0, V1, H0, H1, D0, D1, channels, ret_type);

        // put into CACHE here
        // @TODO

        // return data
        return data;
    }
    else
    {
        // check image subset coordinates
        V0 = V0 < 0 ? 0 : V0;
        H0 = H0 < 0 ? 0 : H0;
        D0 = D0 < 0 ? 0 : D0;
        V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
        H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
        D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED
        if(V1-V0 <=0 || H1-H0 <= 0 || D1-D0 <= 0)
            throw iim::IOException("in VirtualPyramidLayer::loadSubvolume_to_UINT8: invalid subvolume intervals");

        // allocate data
        size_t sbv_height = V1 - V0;
        size_t sbv_width  = H1 - H0;
        size_t sbv_depth  = D1 - D0;
        size_t sbv_dim    = sbv_height * sbv_width * sbv_depth * n_active;
        itm::uint8 *data  = new itm::uint8[sbv_dim];

        // fill data from cache
        for(size_t i=0; i<sbv_dim; i++)
            data[i] = 0;

        // return outputs
        if(channels)
            *channels = (int)n_active;
        return data;
    }
}
/*---- END VIRTUAL PYRAMID LAYER section ----------------------------------------------------------------------------------*/






/**************************************
*   VIRTUAL PYRAMID CACHE definitions *
***************************************
---------------------------------------------------------------------------------------------------------------------------*/
float itm::VirtualPyramidCache::maximumSizeGB = 1;

// constructor 1
itm::VirtualPyramidCache::VirtualPyramidCache(
        std::string _path,                                                      // where cache files are stored / have to be stored
        size_t _dimX, size_t _dimY, size_t _dimZ, size_t _dimC, size_t _dimT,   // image dimensions along X, Y, Z, C (channel), and T (time)
        xyzct<size_t> block_dim /*= xyzct<size_t>(256,256,256,inf,inf)*/)       // hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
throw (iim::IOException, itm::RuntimeException)
{
    path = _path;
    dimX = _dimX;
    dimY = _dimY;
    dimZ = _dimZ;
    dimC = _dimC;
    dimT = _dimT;

    // generate hypergrid
    std::vector<size_t> nXs = itm::partition(dimX, block_dim.x);
    std::vector<size_t> nYs = itm::partition(dimY, block_dim.y);
    std::vector<size_t> nZs = itm::partition(dimZ, block_dim.z);
    std::vector<size_t> nCs = itm::partition(dimC, block_dim.c);
    std::vector<size_t> nTs = itm::partition(dimT, block_dim.t);
    nX = nXs.size();
    nY = nYs.size();
    nZ = nZs.size();
    nC = nCs.size();
    nT = nTs.size();
    hypergrid = new CacheBlock****[nT];
    for(size_t t=0, t_acc=0; t<nT; t_acc+=nTs[t++])
    {
        hypergrid[t] = new CacheBlock***[nC];
        for(size_t c=0, c_acc=0; c<nC; c_acc+=nCs[c++])
        {
            hypergrid[t][c] = new CacheBlock**[nZ];
            for(size_t z=0, z_acc=0; z<nZ; z_acc+=nZs[z++])
            {
                hypergrid[t][c][z] = new CacheBlock*[nY];
                for(size_t y=0, y_acc=0; y<nY; y_acc+=nYs[y++])
                {
                    hypergrid[t][c][z][y] = new CacheBlock[nY];
                    for(size_t x=0, x_acc=0; x<nX; x_acc+=nXs[x++])
                    {
                        hypergrid[t][c][z][y][x].origin = xyzct<size_t>(x_acc, y_acc, z_acc, c_acc, t_acc);
                        hypergrid[t][c][z][y][x].imdata.size = xyzct<size_t>(nXs[x], nYs[y], nZs[z], nCs[c], nTs[t]);
                        printf("hypergrid[%02d][%02d][%02d][%02d][%02d] = (%02d-%02d, %02d-%02d, %02d-%02d, %02d-%02d, %02d-%02d)\n",
                               t, c, z, y, x,
                               hypergrid[t][c][z][y][x].origin.t, hypergrid[t][c][z][y][x].origin.t + hypergrid[t][c][z][y][x].imdata.size.t,
                               hypergrid[t][c][z][y][x].origin.c, hypergrid[t][c][z][y][x].origin.c + hypergrid[t][c][z][y][x].imdata.size.c,
                               hypergrid[t][c][z][y][x].origin.z, hypergrid[t][c][z][y][x].origin.z + hypergrid[t][c][z][y][x].imdata.size.z,
                               hypergrid[t][c][z][y][x].origin.y, hypergrid[t][c][z][y][x].origin.y + hypergrid[t][c][z][y][x].imdata.size.y,
                               hypergrid[t][c][z][y][x].origin.x, hypergrid[t][c][z][y][x].origin.z + hypergrid[t][c][z][y][x].imdata.size.x);
                    }
                }
            }
        }
    }

}

itm::VirtualPyramidCache::CacheBlock::CacheBlock() throw (iim::IOException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
    visits = 0;
}

/*---- END VIRTUAL PYRAMID CACHE section ----------------------------------------------------------------------------------*/
