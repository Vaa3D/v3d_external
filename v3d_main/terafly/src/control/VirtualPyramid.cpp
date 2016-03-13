#include "VirtualPyramid.h"

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
        layers.push_back(new VirtualPyramidLayer(highresVol, xyz<int>(pow(reduction_factor, i),pow(reduction_factor, i),pow(reduction_factor, i)), this));
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
        if( (layer=new VirtualPyramidLayer(highresVol, reduction_factors[i], this))->getMVoxels() > 0)
            layers.push_back(layer);

    // order from lowest-res to highest-res
    std::reverse(layers.begin(), layers.end());
}

// VirtualPyramid deconstructor
itm::VirtualPyramid::~VirtualPyramid()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);
    for(size_t i=0; i<layers.size(); i++)
        delete layers[i];
    delete highresVol;
}


// VirtualPyramidLayer constructor
itm::VirtualPyramidLayer::VirtualPyramidLayer(iim::VirtualVolume* _highresVol, xyz<int> _reduction_factor, VirtualPyramid* _pyramid) : VirtualVolume()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    highresVol = _highresVol;
    reductionFactor = _reduction_factor;
    pyramid = _pyramid;

    VXL_V = highresVol->getVXL_V()*reductionFactor.y;
    VXL_H = highresVol->getVXL_H()*reductionFactor.x;
    VXL_D = highresVol->getVXL_D()*reductionFactor.z;

    ORG_V = highresVol->getORG_V();
    ORG_H = highresVol->getORG_H();
    ORG_D = highresVol->getORG_D();

    DIM_C = highresVol->getDIM_C();
    BYTESxCHAN = highresVol->getBYTESxCHAN();
    initChannels();

    DIM_V = itm::round(highresVol->getDIM_V()/static_cast<float>(reductionFactor.y));
    DIM_H = itm::round(highresVol->getDIM_H()/static_cast<float>(reductionFactor.x));
    DIM_D = itm::round(highresVol->getDIM_D()/static_cast<float>(reductionFactor.z));
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

    // highest-res layer is just a wrapper for the highest-res volume
    if(reductionFactor == xyz<int>(1,1,1))
        return highresVol->loadSubvolume_to_UINT8(V0, V1, H0, H1, D0, D1, channels, ret_type);

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
    itm::uint8 *subvol     = new itm::uint8[sbv_dim];

    // fill data
    for(size_t i=0; i<sbv_dim; i++)
        subvol[i] = 0;

    // return outputs
    if(channels)
        *channels = (int)n_active;
    return subvol;
}

