#include "VirtualPyramid.h"
#include "VirtualVolume.h"
#include <fstream>
#include "IOPluginAPI.h"

/********************************
*   VIRTUAL PYRAMID definitions *
*********************************
---------------------------------------------------------------------------------------------------------------------------*/
// VirtualPyramid constructor 1
itm::VirtualPyramid::VirtualPyramid(
        std::string           _highresPath,       // highest-res (unconverted) volume path
        int                 reduction_factor,     // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
        float lower_bound /*= 100*/,              // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
        iim::VirtualVolume* _highresVol /*= 0*/)  // highest-res (unconverted) volume, if null will be instantiated on-the-fly
throw (iim::IOException, iom::exception, itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // set object members
    highresVol = _highresVol;
    highresPath = _highresPath;

    // instance highest-res volume
    instanceHighresVol();

    // iteratively add a new layer until the lowest-res layer has size > lower_bound
    int i = 0;
    do
    {
        xyz<int> rf = xyz<int>(pow(reduction_factor, i),pow(reduction_factor, i),pow(reduction_factor, i));

        // instance only nonempty layers
        if(VirtualPyramidLayer::isEmpty(highresVol, rf))
            break;
        else
        {
            virtualPyramid.push_back(new VirtualPyramidLayer(i, this, rf));
            i++;
        }
    }
    while (virtualPyramid.back()->getMVoxels() > lower_bound);

    // at least two layers are needed
    if(virtualPyramid.size() < 2)
        throw iim::IOException("Cannot instance Virtual Pyramid with the given settings: at least 2 layers needed. Please check resampling/reduction options or your image size.");

    // create/check metadata
    init();
}


// VirtualPyramid constructor 2
itm::VirtualPyramid::VirtualPyramid(
        std::string                                       _highresPath,         // highest-res (unconverted) volume path
        std::vector< xyz<int> >                           reduction_factors,    // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
        iim::VirtualVolume* _highresVol /*= 0*/)                                // highest-res (unconverted) volume, if null will be instantiated on-the-fly
throw (iim::IOException, iom::exception, itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // set object members
    highresVol = _highresVol;
    highresPath = _highresPath;

    // instance highest-res volume
    instanceHighresVol();

    // generate layers according to the given reduction factors
    for(int i=0; i<reduction_factors.size(); i++)
    {
        // instance only nonempty layers
        if(!VirtualPyramidLayer::isEmpty(highresVol, reduction_factors[i]))
            virtualPyramid.push_back(new VirtualPyramidLayer(i, this, reduction_factors[i]));
    }

    // at least two layers are needed
    if(virtualPyramid.size() < 2)
        throw iim::IOException("Cannot instance Virtual Pyramid with the given settings: at least 2 layers needed. Please check resampling/reduction options or your image size.");

    // create/check metadata
    init();
}

// VirtualPyramid deconstructor
itm::VirtualPyramid::~VirtualPyramid() throw(iim::IOException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // virtual layers should automatically remove themselves from virtualPyramid once they are destroyed
    if(virtualPyramid.size())
        throw iim::IOException("Cannot destroy VirtualPyramid: not all layers have been destroyed");

    for(int i=0; i<pyramid.size(); i++)
        delete pyramid[i];

    delete highresVol;
}

void itm::VirtualPyramid::instanceHighresVol() throw (iim::IOException, iom::exception, itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // create terafly local folder in the executable directory
    if(!iim::check_and_make_dir(".terafly"))
        throw iim::IOException("Cannot create local folder \".terafly\" in the executable folder");

    // create subfolder with the MD5 id generated from the image filepath
    std::string MD5_path = std::string(".terafly/.") + QString(QCryptographicHash::hash(highresPath.c_str(),QCryptographicHash::Md5).toHex()).toStdString();
    if(!iim::check_and_make_dir(MD5_path.c_str()))
        throw iim::IOException(itm::strprintf("Cannot create local folder for unconverted volume at \"%s\"", MD5_path.c_str()));

    // create .virtualpyramid subfolder
    localPath = MD5_path + "/.virtualpyramid";
    if(!iim::check_and_make_dir(localPath.c_str()))
        throw iim::IOException(itm::strprintf("Cannot create local folder for Virtual Pyramid at \"%s\"", localPath.c_str()));

    // create (or check) metadata for Virtual Pyramid
    std::string image_path = MD5_path + "/.volume.txt";
    if(iim::isFile(image_path))
    {
        std::ifstream f(image_path.c_str());
        if(!f.is_open())
            throw iim::IOException("Cannot open .volume.txt file at \"%s\"", image_path.c_str());
        std::string line;
        std::getline(f, line);
        std::vector<std::string> tokens = itm::parse(line, ":", 2, image_path);
        if(tokens[1].compare(highresPath) != 0)
            throw iim::IOException(itm::strprintf("Unconverted image path mismatch at \"%s\": expected \"%s\", found \"%s\"", image_path.c_str(), highresPath.c_str(), tokens[1].c_str()));
        std::getline(f, line);
        tokens = itm::parse(line, ":", 2, image_path);
        if(highresVol && tokens[1].compare(highresVol->getPrintableFormat()) != 0)
            throw iim::IOException(itm::strprintf("Unconverted image path mismatch at \"%s\": expected \"%s\", found \"%s\"", image_path.c_str(), highresPath.c_str(), tokens[1].c_str()));
        else
            highresVol = iim::VirtualVolume::instance_format(highresPath.c_str(), tokens[1]);
        f.close();
    }
    {
        std::ofstream f(image_path.c_str());
        if(!f.is_open())
            throw iim::IOException("Cannot open .volume.txt file at \"%s\"", image_path.c_str());
        f << "imagepath:" << highresPath << std::endl;

        // if volume is not instantiated, and we haven't done it before, we have to use the (time-consuming) auto instantiator
        if(highresVol == 0)
            highresVol = iim::VirtualVolume::instance(highresPath.c_str());

        // then we can store the image format
        f << "format:" << highresVol->getPrintableFormat() << std::endl;
        f.close();
    }
}

void itm::VirtualPyramid::init()  throw (iim::IOException, iom::exception, itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // create (or check) metadata for Virtual Pyramid Layers
    for(int k=0; k<virtualPyramid.size(); k++)
    {
        // create subfolder
        std::string subfolder = localPath + itm::strprintf("/.layer%02d", k);
        if(!iim::check_and_make_dir(subfolder.c_str()))
                throw iim::IOException(itm::strprintf("Cannot create local folder at \"%s\"", subfolder.c_str()));

        // create / check reduction factor file
        std::string reduction_factor_file = subfolder + "/.reduction_factor.txt";
        if(iim::isFile(reduction_factor_file))
        {
            std::ifstream f(reduction_factor_file.c_str());
            if(!f.is_open())
                throw iim::IOException(itm::strprintf("Cannot open reduction factor file at \"%s\"", reduction_factor_file.c_str()));
            std::string line;
            std::getline(f, line);
            std::vector<std::string> tokens = itm::parse(line, " ", 3, reduction_factor_file);
            if(itm::str2num<int>(tokens[0]) != virtualPyramid[k]->reductionFactor.x)
                throw iim::IOException(itm::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along X (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), itm::str2num<int>(tokens[0]), virtualPyramid[k]->reductionFactor.x ));
            if(itm::str2num<int>(tokens[1]) != virtualPyramid[k]->reductionFactor.y)
                throw iim::IOException(itm::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along Y (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), itm::str2num<int>(tokens[1]), virtualPyramid[k]->reductionFactor.y ));
            if(itm::str2num<int>(tokens[2]) != virtualPyramid[k]->reductionFactor.z)
                throw iim::IOException(itm::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along Z (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), itm::str2num<int>(tokens[2]), virtualPyramid[k]->reductionFactor.z ));
            f.close();
        }
        else
        {
            std::ofstream f(reduction_factor_file.c_str());
            if(!f.is_open())
                throw iim::IOException(itm::strprintf("Cannot open reduction factor file at \"%s\"", reduction_factor_file.c_str()));
            f << virtualPyramid[k]->reductionFactor.x << " " << virtualPyramid[k]->reductionFactor.y << " " << virtualPyramid[k]->reductionFactor.z << std::endl;
            f.close();
        }

        // instance cache
        pyramid.push_back(new HyperGridCache(subfolder,
                                             xyzct<size_t>(
                                                 virtualPyramid[k]->getDIM_H(),
                                                 virtualPyramid[k]->getDIM_V(),
                                                 virtualPyramid[k]->getDIM_D(),
                                                 virtualPyramid[k]->getDIM_C(),
                                                 virtualPyramid[k]->getDIM_T())));
    }
}

// load volume of interest from the given resolution layer
itm::image_5D<itm::uint8>
itm::VirtualPyramid::loadVOI(
        itm::xyz<size_t> start,  // xyz range [start, end)
        itm::xyz<size_t> end,    // xyz range [start, end)
        int level)               // pyramid level (0=highest resolution, the higher the lower the resolution)
throw (iim::IOException, iom::exception, itm::RuntimeException)
{
    // checks
    if(level < 0 || level > pyramid.size())
        throw itm::RuntimeException(itm::strprintf("Invalid pyramid level %d", level));

    iim::uint8* data = 0;
    if(level == 0)
    {
        // load data from highest-res image
        data = highresVol->loadSubvolume_to_UINT8(start.y, end.y, start.x, end.x, start.z, end.z);

        // put data into CACHE
        //parent->cache->putData(image_5D<uint8>(data, xyzct<size_t>(0,0,0,0,0)), xyzct<size_t>(0,0,0,0,0), reductionFactor);

        // return data
        return itm::image_5D<uint8>(
                    data,
                    xyzt<size_t>(end.x-start.x, end.y-start.y, end.z-start.z, highresVol->getNActiveFrames()),
                    active_channels<>(highresVol->getActiveChannels(), highresVol->getNACtiveChannels()));
    }
    else
    {
        data = pyramid[level]->readData(
                    xyzt<size_t>(start.x, start.y, start.z, highresVol->getT0()),
                    xyzt<size_t>(end.x, end.y, end.z, highresVol->getT1()+1),
                    active_channels<>(highresVol->getActiveChannels(), highresVol->getNACtiveChannels())).data;
    }
    return itm::image_5D<uint8>(
                data,
                xyzt<size_t>(end.x-start.x, end.y-start.y, end.z-start.z, highresVol->getNActiveFrames()),
                active_channels<>(highresVol->getActiveChannels(), highresVol->getNACtiveChannels()));
}

/*----END VIRTUAL PYRAMID section -----------------------------------------------------------------------------------------*/






/**************************************
*   VIRTUAL PYRAMID LAYER definitions *
***************************************
---------------------------------------------------------------------------------------------------------------------------*/
// return true if 'highestVol' downsampled by 'reduction_factor' generates a 0-sized image
bool itm::VirtualPyramidLayer::isEmpty(iim::VirtualVolume* highresVol, xyz<int> _reduction_factor)
{
    int dim_v = itm::round(highresVol->getDIM_V()/static_cast<float>(_reduction_factor.y));
    int dim_h = itm::round(highresVol->getDIM_H()/static_cast<float>(_reduction_factor.x));
    int dim_d = itm::round(highresVol->getDIM_D()/static_cast<float>(_reduction_factor.z));

    return dim_v == 0 || dim_h == 0 || dim_d == 0;
}

// VirtualPyramidLayer constructor
itm::VirtualPyramidLayer::VirtualPyramidLayer(
        int _level,                         // pyramid level (0 for the highest-res, the coarser the resolution the higher)
        VirtualPyramid* _parent,            // container
        xyz<int> _reduction_factor)         // reduction factor relative to the highest-res image
throw (iim::IOException) : VirtualVolume()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);


    level = _level;
    parent = _parent;
    reductionFactor = _reduction_factor;

    VXL_V = parent->highresVol->getVXL_V()*reductionFactor.y;
    VXL_H = parent->highresVol->getVXL_H()*reductionFactor.x;
    VXL_D = parent->highresVol->getVXL_D()*reductionFactor.z;
    ORG_V = parent->highresVol->getORG_V();
    ORG_H = parent->highresVol->getORG_H();
    ORG_D = parent->highresVol->getORG_D();
    DIM_C = parent->highresVol->getDIM_C();
    BYTESxCHAN = parent->highresVol->getBYTESxCHAN();
    initChannels();
    DIM_T = parent->highresVol->getDIM_T();
    DIM_V = itm::round(parent->highresVol->getDIM_V()/static_cast<float>(reductionFactor.y));
    DIM_H = itm::round(parent->highresVol->getDIM_H()/static_cast<float>(reductionFactor.x));
    DIM_D = itm::round(parent->highresVol->getDIM_D()/static_cast<float>(reductionFactor.z));


}

void itm::VirtualPyramidLayer::initChannels() throw (iim::IOException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    static bool first_time = true;
    if(first_time)
        parent->highresVol->initChannels();
    else
        first_time = false;
    active = parent->highresVol->getActiveChannels();
    n_active = parent->highresVol->getNACtiveChannels();
}

// VirtualPyramidLayer deconstructor
itm::VirtualPyramidLayer::~VirtualPyramidLayer() throw (iim::IOException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);
    active = 0;
    n_active = 0;

    // remove layer from parent, if present
    std::vector<itm::VirtualPyramidLayer*>::iterator position = std::find(parent->virtualPyramid.begin(), parent->virtualPyramid.end(), this);
    if (position != parent->virtualPyramid.end())
        parent->virtualPyramid.erase(position);

    // if parent is empty, destroy it
    if(parent->virtualPyramid.empty())
        delete parent;
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

    throw iim::IOException("load to real32 from VirtualPyramidLayer not yet implemented");
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


    // check image subset coordinates
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED
    if(V1-V0 <=0 || H1-H0 <= 0 || D1-D0 <= 0)
        throw iim::IOException("in VirtualPyramidLayer::loadSubvolume_to_UINT8: invalid subvolume intervals");

    // return outputs
    if(channels)
        *channels = (int)n_active;

    return parent->loadVOI(xyz<size_t>(H0,V0,D0), xyz<size_t>(H1,V1,D1), level).data;
}
/*---- END VIRTUAL PYRAMID LAYER section ----------------------------------------------------------------------------------*/






/**************************************
*   HYPER GRID  CACHE definitions     *
***************************************
---------------------------------------------------------------------------------------------------------------------------*/
float itm::HyperGridCache::maximumSizeGB = 1;

// constructor 1
itm::HyperGridCache::HyperGridCache(
        std::string _path,                                                      // where cache files are stored / have to be stored
        xyzct<size_t> image_dim,                                                // image dimensions along X, Y, Z, C (channel), and T (time)
        xyzct<size_t> _block_dim /*= xyzct<size_t>(256,256,256,inf,inf)*/)       // hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
throw (iim::IOException, itm::RuntimeException)
{
    // set object members
    path = _path;
    dimX = image_dim.x;
    dimY = image_dim.y;
    dimZ = image_dim.z;
    dimC = image_dim.c;
    dimT = image_dim.t;
    block_dim = _block_dim;

    // @TODO
    if(dimC > 3)
        throw iim::IOException("HyperGridCache does not support > 3 channels yet");
    if(dimT > 1)
        throw iim::IOException("HyperGridCache does not support 5D (xyz + channel + time) data yet");

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
    hypergrid = new CacheBlock*****[nT];
    for(size_t t=0, t_acc=0; t<nT; t_acc+=nTs[t++])
    {
        hypergrid[t] = new CacheBlock****[nC];
        for(size_t c=0, c_acc=0; c<nC; c_acc+=nCs[c++])
        {
            hypergrid[t][c] = new CacheBlock***[nZ];
            for(size_t z=0, z_acc=0; z<nZ; z_acc+=nZs[z++])
            {
                hypergrid[t][c][z] = new CacheBlock**[nY];
                for(size_t y=0, y_acc=0; y<nY; y_acc+=nYs[y++])
                {
                    hypergrid[t][c][z][y] = new CacheBlock*[nY];
                    for(size_t x=0, x_acc=0; x<nX; x_acc+=nXs[x++])
                    {
                        hypergrid[t][c][z][y][x] = new CacheBlock(this,
                                                                  xyzct<size_t>(x_acc, y_acc, z_acc, c_acc, t_acc),
                                                                  xyzct<size_t>(nXs[x], nYs[y], nZs[z], nCs[c], nTs[t]),
                                                                  xyzct<size_t>(x,y,z,c,t));
                    }
                }
            }
        }
    }

    init();
}

// destructor
itm::HyperGridCache::~HyperGridCache() throw (iim::IOException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    for(size_t t=0; t<nT; t++)
    {
        for(size_t c=0; c<nC; c++)
        {
            for(size_t z=0; z<nZ; z++)
            {
                for(size_t y=0; y<nY; y++)
                {
                    for(size_t x=0; x<nX; x++)
                        delete hypergrid[t][c][z][y][x];
                    delete hypergrid[t][c][z][y];
                }
                delete hypergrid[t][c][z];
            }
            delete hypergrid[t][c];
        }
        delete hypergrid[t];
    }
    delete hypergrid;
}

// init persistency files
void itm::HyperGridCache::init() throw (iim::IOException, itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // create (hidden) folder in given path
    std::string directory = path + "/.hypergridcache";
    if(!iim::check_and_make_dir(directory.c_str()))
        throw iim::IOException(itm::strprintf("Cannot create local folder \"%s\" in the vaa3d executable folder", directory.c_str()));


    // save or check hypergrid to file
    std::string hypergridFilePath = directory + "/.hypergrid.txt";
    if(iim::isFile(hypergridFilePath))
        load();
    else
        save();
}

// load from disk
void itm::HyperGridCache::load() throw (iim::IOException, itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // load metadata
    std::string hypergridFilePath = path + "/.hypergridcache/.hypergrid.txt";
    std::ifstream f(hypergridFilePath.c_str());
    if(!f.is_open())
        throw iim::IOException("Cannot open .hypergrid file at \"%s\"", hypergridFilePath.c_str());
    std::string line;
    std::vector <std::string> tokens;
    while(std::getline(f, line))
    {
        tokens = itm::parse(line, ":", 2, hypergridFilePath);
        if(tokens[0].compare("image.dimX") == 0 && itm::str2num<size_t>(tokens[1]) != dimX)
            throw iim::IOException(itm::strprintf("dimX mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), dimX, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("image.dimY") == 0 && itm::str2num<size_t>(tokens[1]) != dimY)
            throw iim::IOException(itm::strprintf("dimY mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), dimY, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("image.dimZ") == 0 && itm::str2num<size_t>(tokens[1]) != dimZ)
            throw iim::IOException(itm::strprintf("dimZ mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), dimZ, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("image.dimC") == 0 && itm::str2num<size_t>(tokens[1]) != dimC)
            throw iim::IOException(itm::strprintf("dimC mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), dimC, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("image.dimT") == 0 && itm::str2num<size_t>(tokens[1]) != dimT)
            throw iim::IOException(itm::strprintf("dimT mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), dimT, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.nX") == 0 && itm::str2num<size_t>(tokens[1]) != nX)
            throw iim::IOException(itm::strprintf("nX mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), nX, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.nY") == 0 && itm::str2num<size_t>(tokens[1]) != nY)
            throw iim::IOException(itm::strprintf("nY mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), nY, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.nZ") == 0 && itm::str2num<size_t>(tokens[1]) != nZ)
            throw iim::IOException(itm::strprintf("nZ mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), nZ, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.nC") == 0 && itm::str2num<size_t>(tokens[1]) != nC)
            throw iim::IOException(itm::strprintf("nC mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), nC, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.nT") == 0 && itm::str2num<size_t>(tokens[1]) != nT)
            throw iim::IOException(itm::strprintf("nT mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), nT, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.dimX") == 0 && itm::str2num<size_t>(tokens[1]) != block_dim.x)
            throw iim::IOException(itm::strprintf("blockX mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), block_dim.x, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.dimY") == 0 && itm::str2num<size_t>(tokens[1]) != block_dim.y)
            throw iim::IOException(itm::strprintf("blockY mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), block_dim.y, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.dimZ") == 0 && itm::str2num<size_t>(tokens[1]) != block_dim.z)
            throw iim::IOException(itm::strprintf("blockZ mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), block_dim.z, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.dimC") == 0 && itm::str2num<size_t>(tokens[1]) != block_dim.c)
            throw iim::IOException(itm::strprintf("blockC mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), block_dim.c, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.dimT") == 0 && itm::str2num<size_t>(tokens[1]) != block_dim.t)
            throw iim::IOException(itm::strprintf("blockT mismatch in .hypergrid file at \"%s\": expected %llu, found %llu", hypergridFilePath.c_str(), block_dim.t, itm::str2num<size_t>(tokens[1])));
        if(tokens[0].compare("blocks.visits") == 0)
        {
            std::vector <std::string> visits;
            itm::parse(tokens[1], ";", nX*nY*nZ*nC*nT+1, hypergridFilePath, visits);
            size_t counter = 0;
            for(size_t t=0; t<nT; t++)
                for(size_t c=0; c<nC; c++)
                    for(size_t z=0; z<nZ; z++)
                        for(size_t y=0; y<nY; y++)
                            for(size_t x=0; x<nX; x++)
                                hypergrid[t][c][z][y][x]->setVisits(itm::str2num<int>(visits[counter++]));
        }
    }
    f.close();
}

// save to disk
void itm::HyperGridCache::save() throw (iim::IOException, itm::RuntimeException)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    // save metadata
    std::string hypergridFilePath = path + "/.hypergridcache/.hypergrid.txt";
    std::ofstream f(hypergridFilePath.c_str());
    if(!f.is_open())
        throw iim::IOException("Cannot open .hypergrid file at \"%s\"", hypergridFilePath.c_str());
    f << "image.dimX:" << dimX << std::endl;
    f << "image.dimY:" << dimY << std::endl;
    f << "image.dimZ:" << dimZ << std::endl;
    f << "image.dimC:" << dimC << std::endl;
    f << "image.dimT:" << dimT << std::endl;
    f << "blocks.nX:" << nX << std::endl;
    f << "blocks.nY:" << nY << std::endl;
    f << "blocks.nZ:" << nZ << std::endl;
    f << "blocks.nC:" << nC << std::endl;
    f << "blocks.nT:" << nT << std::endl;
    f << "blocks.dimX:" << block_dim.x << std::endl;
    f << "blocks.dimY:" << block_dim.y << std::endl;
    f << "blocks.dimZ:" << block_dim.z << std::endl;
    f << "blocks.dimC:" << block_dim.c << std::endl;
    f << "blocks.dimT:" << block_dim.t << std::endl;
    f << "blocks.visits:";
    for(size_t t=0; t<nT; t++)
        for(size_t c=0; c<nC; c++)
            for(size_t z=0; z<nZ; z++)
                for(size_t y=0; y<nY; y++)
                    for(size_t x=0; x<nX; x++)
                        f << hypergrid[t][c][z][y][x]->getVisits() << ";" ;
    f.close();
}


// read data from the cache (downsampling on-the-fly supported)
itm::image_5D<itm::uint8>                                       // <image data, image data size> output
itm::HyperGridCache::readData(xyzt<size_t> start,               // start coordinate in the current X,Y,Z,T image space
        xyzt<size_t> end,                                       // end coordinate in the current X,Y,Z,T image space
        active_channels<> channels,                             // active channels
        itm::xyz<int> downsamplingFactor)                       // downsampling factors along X,Y and Z
throw (iim::IOException)
{
    // allocate data
    size_t sbv_dim    = (end.x-start.x) * (end.y-start.y) * (end.z-start.z) * channels.dim * (end.t-start.t);
    uint8* data  = new itm::uint8[sbv_dim];

    for(size_t i=0; i<sbv_dim; i++)
        data[i] = 0;

    return itm::image_5D<itm::uint8>(data, xyzt<size_t>(end.x-start.x, end.y-start.y, end.z-start.z, end.t-start.t), channels);
}

/*---- END HYPER GRID CACHE section ---------------------------------------------------------------------------------------*/






/********************************************
*   HYPER GRID CACHE BLOCK definitions      *
*********************************************
---------------------------------------------------------------------------------------------------------------------------*/
// constructor 1
itm::HyperGridCache::CacheBlock::CacheBlock(
        HyperGridCache* _parent,
        xyzct<size_t> _origin,          // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
        xyzct<size_t> _dims,            // dimensions of the block
        xyzct<size_t> _index)           // 5D index in the parent hypergrid
throw (iim::IOException)
{
    /**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__);

    parent = _parent;
    origin = _origin;
    imdata.dims.x = _dims.x;
    imdata.data = 0;
    idx = _index;
    visits = 0;

    path = parent->path + itm::strprintf("t%s_c%s_z%s_y%s_x%s.tif",
                                                             itm::num2str(idx.t).c_str(),
                                                             itm::num2str(idx.c).c_str(),
                                                             itm::num2str(idx.z).c_str(),
                                                             itm::num2str(idx.y).c_str(),
                                                             itm::num2str(idx.x).c_str());
}


// destructor
itm::HyperGridCache::CacheBlock::~CacheBlock() throw (iim::IOException)
{
    /**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__);

    if(imdata.data)
        delete imdata.data;
}

// load from disk
void itm::HyperGridCache::CacheBlock::load() throw (iim::IOException, iom::exception)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    if(iim::isFile(path))
    {
        if(!imdata.data)
            imdata.data = new uint8[imdata.dims.x * imdata.dims.y * imdata.dims.z * imdata.chans.dim];
        int dimX, dimY, dimZ, dimC, bytes;
        iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->readData(path, dimX, dimY, dimZ, bytes, dimC, imdata.data);
    }
}

// save to disk
void itm::HyperGridCache::CacheBlock::save() throw (iim::IOException, iom::exception)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    if(imdata.data)
    {
        if(!iim::isFile(path))
            iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->create3Dimage(path, imdata.dims.y, imdata.dims.x, imdata.dims.z, 1, imdata.chans.dim);
        iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->writeData(path, imdata.data, imdata.dims.y, imdata.dims.x, imdata.dims.z, 1, imdata.chans.dim);
    }
}
/*---- END HYPER GRID CACHE BLOCK section --------------------------------------------------------------------------------*/
