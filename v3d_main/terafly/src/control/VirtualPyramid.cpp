#include "VirtualPyramid.h"
#include "VirtualVolume.h"
#include <fstream>
#include "IOPluginAPI.h"

/********************************
*   VIRTUAL PYRAMID definitions *
*********************************
---------------------------------------------------------------------------------------------------------------------------*/
// VirtualPyramid constructor 1
tf::VirtualPyramid::VirtualPyramid(
        std::string           _highresPath,       // highest-res (unconverted) volume path
        int                 reduction_factor,     // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
        float lower_bound /*= 100*/,              // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
        iim::VirtualVolume* _highresVol /*= 0*/)  // highest-res (unconverted) volume, if null will be instantiated on-the-fly
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

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
tf::VirtualPyramid::VirtualPyramid(
        std::string                                       _highresPath,         // highest-res (unconverted) volume path
        std::vector< xyz<int> >                           reduction_factors,    // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
        iim::VirtualVolume* _highresVol /*= 0*/)                                // highest-res (unconverted) volume, if null will be instantiated on-the-fly
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

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
tf::VirtualPyramid::~VirtualPyramid() throw(iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // virtual layers should automatically remove themselves from virtualPyramid once they are destroyed
    if(virtualPyramid.size())
        throw iim::IOException("Cannot destroy VirtualPyramid: not all layers have been destroyed");

    for(int i=0; i<pyramid.size(); i++)
        delete pyramid[i];

    delete highresVol;
}

void tf::VirtualPyramid::instanceHighresVol() throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // create terafly local folder in the executable directory
    if(!iim::check_and_make_dir(".terafly"))
        throw iim::IOException("Cannot create local folder \".terafly\" in the executable folder");

    // create subfolder with the MD5 id generated from the image filepath
    std::string MD5_path = std::string(".terafly/.") + QString(QCryptographicHash::hash(highresPath.c_str(),QCryptographicHash::Md5).toHex()).toStdString();
    if(!iim::check_and_make_dir(MD5_path.c_str()))
        throw iim::IOException(tf::strprintf("Cannot create local folder for unconverted volume at \"%s\"", MD5_path.c_str()));

    // create .virtualpyramid subfolder
    localPath = MD5_path + "/.virtualpyramid";
    if(!iim::check_and_make_dir(localPath.c_str()))
        throw iim::IOException(tf::strprintf("Cannot create local folder for Virtual Pyramid at \"%s\"", localPath.c_str()));

    // create (or check) metadata for Virtual Pyramid
    std::string volumetxtPath = MD5_path + "/.volume.txt";
    if(iim::isFile(volumetxtPath))
    {
        std::ifstream f(volumetxtPath.c_str());
        if(!f.is_open())
            throw iim::IOException("Cannot open .volume.txt file at \"%s\"", volumetxtPath.c_str());
        std::string line;
        std::getline(f, line);
        std::vector<std::string> tokens = tf::parse(line, ":", 2, volumetxtPath);
        if(tokens[1].compare(highresPath) != 0)
            throw iim::IOException(tf::strprintf("Unconverted image path mismatch at \"%s\": expected \"%s\", found \"%s\"", volumetxtPath.c_str(), highresPath.c_str(), tokens[1].c_str()));
        std::getline(f, line);
        tokens = tf::parse(line, ":", 2, volumetxtPath);
        if(highresVol && tokens[1].compare(highresVol->getPrintableFormat()) != 0)
            throw iim::IOException(tf::strprintf("Unconverted image path mismatch at \"%s\": expected \"%s\", found \"%s\"", volumetxtPath.c_str(), highresPath.c_str(), tokens[1].c_str()));
        else
            highresVol = iim::VirtualVolume::instance_format(highresPath.c_str(), tokens[1]);
        f.close();
    }
    {
        // if volume is not instantiated, and we haven't done it before, we have to use the (time-consuming) auto instantiator
        // @FIXED by Alessandro on 2016/03/25: this should be done BEFORE creating the volume.txt file
        // so that in case it throws, no (incomplete) volume.txt is created
        if(highresVol == 0)
            highresVol = iim::VirtualVolume::instance(highresPath.c_str());

        std::ofstream f(volumetxtPath.c_str());
        if(!f.is_open())
            throw iim::IOException("Cannot open .volume.txt file at \"%s\"", volumetxtPath.c_str());
        f << "imagepath:" << highresPath << std::endl;

        // then we can store the image format
        f << "format:" << highresVol->getPrintableFormat() << std::endl;
        f.close();
    }
}

void tf::VirtualPyramid::init()  throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // create (or check) metadata for Virtual Pyramid Layers
    for(int k=0; k<virtualPyramid.size(); k++)
    {
        // create subfolder
        std::string subfolder = localPath + tf::strprintf("/.layer%02d", k);
        if(!iim::check_and_make_dir(subfolder.c_str()))
                throw iim::IOException(tf::strprintf("Cannot create local folder at \"%s\"", subfolder.c_str()));

        // create / check reduction factor file
        std::string reduction_factor_file = subfolder + "/.reduction_factor.txt";
        if(iim::isFile(reduction_factor_file))
        {
            std::ifstream f(reduction_factor_file.c_str());
            if(!f.is_open())
                throw iim::IOException(tf::strprintf("Cannot open reduction factor file at \"%s\"", reduction_factor_file.c_str()));
            std::string line;
            std::getline(f, line);
            std::vector<std::string> tokens = tf::parse(line, " ", 3, reduction_factor_file);
            if(tf::str2num<int>(tokens[0]) != virtualPyramid[k]->reductionFactor.x)
                throw iim::IOException(tf::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along X (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), tf::str2num<int>(tokens[0]), virtualPyramid[k]->reductionFactor.x ));
            if(tf::str2num<int>(tokens[1]) != virtualPyramid[k]->reductionFactor.y)
                throw iim::IOException(tf::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along Y (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), tf::str2num<int>(tokens[1]), virtualPyramid[k]->reductionFactor.y ));
            if(tf::str2num<int>(tokens[2]) != virtualPyramid[k]->reductionFactor.z)
                throw iim::IOException(tf::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along Z (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), tf::str2num<int>(tokens[2]), virtualPyramid[k]->reductionFactor.z ));
            f.close();
        }
        else
        {
            std::ofstream f(reduction_factor_file.c_str());
            if(!f.is_open())
                throw iim::IOException(tf::strprintf("Cannot open reduction factor file at \"%s\"", reduction_factor_file.c_str()));
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
tf::image5D<tf::uint8>
tf::VirtualPyramid::loadVOI(
        tf::xyz<size_t> start,  // xyz range [start, end)
        tf::xyz<size_t> end,    // xyz range [start, end)
        int level)               // pyramid level (0=highest resolution, the higher the lower the resolution)
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // checks
    if(level < 0 || level > pyramid.size())
        throw tf::RuntimeException(tf::strprintf("Invalid pyramid level %d", level));

    iim::uint8* data = 0;
    if(level == 0)
    {
        // load data from highest-res image
        data = highresVol->loadSubvolume_to_UINT8(start.y, end.y, start.x, end.x, start.z, end.z);

        // put data into CACHE
        //parent->cache->putData(image_5D<uint8>(data, xyzct<size_t>(0,0,0,0,0)), xyzct<size_t>(0,0,0,0,0), reductionFactor);
    }
    else
    {
        data = pyramid[level]->readData(
                    voi4D<size_t>(
                        xyzt<size_t>(start.x, start.y, start.z, highresVol->getT0()),
                        xyzt<size_t>(end.x, end.y, end.z, highresVol->getT1()+1)),
                    active_channels<>(highresVol->getActiveChannels(), highresVol->getNACtiveChannels())).data;
    }
    return tf::image5D<uint8>(
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
bool tf::VirtualPyramidLayer::isEmpty(iim::VirtualVolume* highresVol, xyz<int> _reduction_factor)
{
    int dim_v = tf::round(highresVol->getDIM_V()/static_cast<float>(_reduction_factor.y));
    int dim_h = tf::round(highresVol->getDIM_H()/static_cast<float>(_reduction_factor.x));
    int dim_d = tf::round(highresVol->getDIM_D()/static_cast<float>(_reduction_factor.z));

    return dim_v == 0 || dim_h == 0 || dim_d == 0;
}

// VirtualPyramidLayer constructor
tf::VirtualPyramidLayer::VirtualPyramidLayer(
        int _level,                         // pyramid level (0 for the highest-res, the coarser the resolution the higher)
        VirtualPyramid* _parent,            // container
        xyz<int> _reduction_factor)         // reduction factor relative to the highest-res image
throw (iim::IOException) : VirtualVolume()
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);


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
    DIM_V = parent->highresVol->getDIM_V()/static_cast<float>(reductionFactor.y);
    DIM_H = parent->highresVol->getDIM_H()/static_cast<float>(reductionFactor.x);
    DIM_D = parent->highresVol->getDIM_D()/static_cast<float>(reductionFactor.z);
}

void tf::VirtualPyramidLayer::initChannels() throw (iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    static bool first_time = true;
    if(first_time)
        parent->highresVol->initChannels();
    else
        first_time = false;
    active = parent->highresVol->getActiveChannels();
    n_active = parent->highresVol->getNACtiveChannels();
}

// VirtualPyramidLayer deconstructor
tf::VirtualPyramidLayer::~VirtualPyramidLayer() throw (iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);
    active = 0;
    n_active = 0;

    // remove layer from parent, if present
    std::vector<tf::VirtualPyramidLayer*>::iterator position = std::find(parent->virtualPyramid.begin(), parent->virtualPyramid.end(), this);
    if (position != parent->virtualPyramid.end())
        parent->virtualPyramid.erase(position);

    // if parent is empty, destroy it
    if(parent->virtualPyramid.empty())
        delete parent;
}


// VirtualPyramidLayer load method to uint32
iim::real32* tf::VirtualPyramidLayer::loadSubvolume_to_real32(
        int V0 /* = -1 */,
        int V1 /* = -1 */,
        int H0 /* = -1 */,
        int H1 /* = -1 */,
        int D0 /* = -1 */,
        int D1 /* = -1 */)
throw (iim::IOException)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    throw iim::IOException("load to real32 from VirtualPyramidLayer not yet implemented");
}


// VirtualPyramidLayer load method to uint8
iim::uint8* tf::VirtualPyramidLayer::loadSubvolume_to_UINT8(
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
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);


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
float tf::HyperGridCache::maximumSizeGB = 1;

// constructor 1
tf::HyperGridCache::HyperGridCache(
        std::string _path,                                                      // where cache files are stored / have to be stored
        xyzct<size_t> image_dim,                                                // image dimensions along X, Y, Z, C (channel), and T (time)
        xyzct<size_t> _block_dim /*= xyzct<size_t>(256,256,256,inf,inf)*/)       // hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
throw (iim::IOException, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

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
    std::vector<size_t> nXs = tf::partition(dimX, block_dim.x);
    std::vector<size_t> nYs = tf::partition(dimY, block_dim.y);
    std::vector<size_t> nZs = tf::partition(dimZ, block_dim.z);
    std::vector<size_t> nCs = tf::partition(dimC, block_dim.c);
    std::vector<size_t> nTs = tf::partition(dimT, block_dim.t);
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
tf::HyperGridCache::~HyperGridCache() throw (iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

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
void tf::HyperGridCache::init() throw (iim::IOException, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // create (hidden) folder in given path
    std::string directory = path + "/.hypergridcache";
    if(!iim::check_and_make_dir(directory.c_str()))
        throw iim::IOException(tf::strprintf("Cannot create local folder \"%s\" in your executable folder", directory.c_str()));


    // save or check hypergrid to file
    std::string hypergridFilePath = directory + "/.hypergrid.txt";
    if(iim::isFile(hypergridFilePath))
        load();
    else
        save();
}

// load from disk
void tf::HyperGridCache::load() throw (iim::IOException, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // load metadata
    std::string hypergridFilePath = path + "/.hypergridcache/.hypergrid.txt";
    std::ifstream f(hypergridFilePath.c_str());
    if(!f.is_open())
        throw iim::IOException("Cannot open .hypergrid file at \"%s\"", hypergridFilePath.c_str());
    std::string line;
    std::vector <std::string> tokens;
    while(std::getline(f, line))
    {
        tokens = tf::parse(line, ":", 2, hypergridFilePath);
        if(tokens[0].compare("image.dimX") == 0 && tf::str2num<size_t>(tokens[1]) != dimX)
            throw iim::IOException(tf::strprintf("dimX mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), dimX, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("image.dimY") == 0 && tf::str2num<size_t>(tokens[1]) != dimY)
            throw iim::IOException(tf::strprintf("dimY mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), dimY, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("image.dimZ") == 0 && tf::str2num<size_t>(tokens[1]) != dimZ)
            throw iim::IOException(tf::strprintf("dimZ mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), dimZ, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("image.dimC") == 0 && tf::str2num<size_t>(tokens[1]) != dimC)
            throw iim::IOException(tf::strprintf("dimC mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), dimC, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("image.dimT") == 0 && tf::str2num<size_t>(tokens[1]) != dimT)
            throw iim::IOException(tf::strprintf("dimT mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), dimT, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.nX") == 0 && tf::str2num<size_t>(tokens[1]) != nX)
            throw iim::IOException(tf::strprintf("nX mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), nX, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.nY") == 0 && tf::str2num<size_t>(tokens[1]) != nY)
            throw iim::IOException(tf::strprintf("nY mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), nY, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.nZ") == 0 && tf::str2num<size_t>(tokens[1]) != nZ)
            throw iim::IOException(tf::strprintf("nZ mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), nZ, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.nC") == 0 && tf::str2num<size_t>(tokens[1]) != nC)
            throw iim::IOException(tf::strprintf("nC mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), nC, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.nT") == 0 && tf::str2num<size_t>(tokens[1]) != nT)
            throw iim::IOException(tf::strprintf("nT mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), nT, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.dimX") == 0 && tf::str2num<size_t>(tokens[1]) != block_dim.x)
            throw iim::IOException(tf::strprintf("blockX mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), block_dim.x, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.dimY") == 0 && tf::str2num<size_t>(tokens[1]) != block_dim.y)
            throw iim::IOException(tf::strprintf("blockY mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), block_dim.y, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.dimZ") == 0 && tf::str2num<size_t>(tokens[1]) != block_dim.z)
            throw iim::IOException(tf::strprintf("blockZ mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), block_dim.z, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.dimC") == 0 && tf::str2num<size_t>(tokens[1]) != block_dim.c)
            throw iim::IOException(tf::strprintf("blockC mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), block_dim.c, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.dimT") == 0 && tf::str2num<size_t>(tokens[1]) != block_dim.t)
            throw iim::IOException(tf::strprintf("blockT mismatch in .hypergrid file at \"%s\": expected %llu, found %llu. \n\nPlease delete folder \"<your executable path>/%s\" and reopen the image (metadata will be reinitialized automatically).", hypergridFilePath.c_str(), block_dim.t, tf::str2num<size_t>(tokens[1]), path.c_str()));
        if(tokens[0].compare("blocks.visits") == 0)
        {
            std::vector <std::string> visits;
            tf::parse(tokens[1], ";", nX*nY*nZ*nC*nT+1, hypergridFilePath, visits);
            size_t counter = 0;
            for(size_t t=0; t<nT; t++)
                for(size_t c=0; c<nC; c++)
                    for(size_t z=0; z<nZ; z++)
                        for(size_t y=0; y<nY; y++)
                            for(size_t x=0; x<nX; x++)
                                hypergrid[t][c][z][y][x]->setVisits(tf::str2num<int>(visits[counter++]));
        }
    }
    f.close();
}

// save to disk
void tf::HyperGridCache::save() throw (iim::IOException, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

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
tf::image5D<tf::uint8>                                          // <image data, image data size> output
tf::HyperGridCache::readData(
        tf::voi4D<size_t> voi,                                  // range in X,Y,Z,T space
        active_channels<> channels,                             // active channels
        tf::xyz<int> downsamplingFactor)                        // downsampling factors along X,Y and Z
throw (iim::IOException)
{
    /**/tf::debug(tf::LEV2, tf::strprintf("voi = [%d,%d),{%s},[%d,%d),[%d,%d),[%d,%d), downsampling = {%d,%d,%d}",
                                          voi.start.t, voi.end.t, channels.toString().c_str(), voi.start.z, voi.end.z,
                                          voi.start.y, voi.end.y, voi.start.x, voi.end.x, downsamplingFactor.x, downsamplingFactor.y, downsamplingFactor.z).c_str(), __itm__current__function__);

    // check xyzt data selection
    if(voi.start.x >= voi.end.x || voi.start.y >= voi.end.y || voi.start.z >= voi.end.z || voi.start.t >= voi.end.t)
        throw iim::IOException(tf::strprintf("Invalid xyzt data selection x=[%d,%d) y=[%d,%d) z=[%d,%d) t=[%d,%d)",
                                              voi.start.x, voi.end.x, voi.start.y, voi.end.y, voi.start.z, voi.end.z, voi.start.t, voi.end.t), __itm__current__function__);
    if(voi.end.x > dimX || voi.end.y > dimY || voi.end.z > dimZ || voi.end.t > dimT)
        throw iim::IOException(tf::strprintf("Out of range xyzt data selection x=[%d,%d) y=[%d,%d) z=[%d,%d) t=[%d,%d), range is "
                                              "x=[0,%d) y=[0,%d) z=[0,%d) t=[0,%d)",
                                              voi.start.x, voi.end.x, voi.start.y, voi.end.y, voi.start.z, voi.end.z, voi.start.t, voi.end.t, dimX, dimY, dimZ, dimT), __itm__current__function__);

    // check channel selection
    for(size_t i=0; i<channels.dim; i++)
        if(channels.table[i] >= dimC)
            throw iim::IOException(tf::strprintf("Out of range channel selection %d, number of channels is %d",
                                                  channels.table[i], dimC), __itm__current__function__);

    // check downsampling factor
    if(downsamplingFactor.x < 1 || downsamplingFactor.y < 1 || downsamplingFactor.z < 1)
        throw iim::IOException(tf::strprintf("Invalid downsampling factor {%d,%d%d}, minimum is {1,1,1}",
                                              downsamplingFactor.x, downsamplingFactor.y, downsamplingFactor.z), __itm__current__function__);


    // allocate and initialize data
    tf::image5D<tf::uint8> img;
    img.chans = channels;
    img.dims.x = voi.dims().x / downsamplingFactor.x;
    img.dims.y = voi.dims().y / downsamplingFactor.y;
    img.dims.z = voi.dims().z / downsamplingFactor.z;
    img.dims.t = voi.dims().t;
    img.data  = new tf::uint8[img.size()];
    for(size_t i=0; i<img.size(); i++)
        img.data[i] = 0;


    // detecting intersecting blocks
    printf("\n");
    for(size_t t=0; t<nT; t++)
        for(size_t c=0; c<nC; c++)
            for(size_t z=0; z<nZ; z++)
                for(size_t y=0; y<nY; y++)
                    for(size_t x=0; x<nX; x++)
                        if(hypergrid[t][c][z][y][x]->xyzt_intersection(voi).size()   > 0 &&
                           hypergrid[t][c][z][y][x]->c_intersection(channels).size() > 0)
                        {
                            voi4D<size_t> iv = hypergrid[t][c][z][y][x]->xyzt_intersection(voi);
                            std::vector<unsigned int> ic = hypergrid[t][c][z][y][x]->c_intersection(channels);
                            printf("hypergrid[%02d][%02d][%02d][%02d][%02d] intersection WITH [%d,%d),{%s},[%d,%d),[%d,%d),[%d,%d) IS [%d,%d),{%s},[%d,%d),[%d,%d),[%d,%d)\n",
                                   t,c,z,y,x,
                                   voi.start.t, voi.end.t, channels.toString().c_str(),             voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x,
                                    iv.start.t,  iv.end.t,  active_channels<>::toString(ic).c_str(), iv.start.z,  iv.end.z,  iv.start.y,  iv.end.y,  iv.start.x,  iv.end.x);
                        }
    printf("\n");

    return img;
}

/*---- END HYPER GRID CACHE section ---------------------------------------------------------------------------------------*/






/********************************************
*   HYPER GRID CACHE BLOCK definitions      *
*********************************************
---------------------------------------------------------------------------------------------------------------------------*/
// constructor 1
tf::HyperGridCache::CacheBlock::CacheBlock(
        HyperGridCache* _parent,
        xyzct<size_t> _origin,          // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
        xyzct<size_t> _dims,            // dimensions of the block
        xyzct<size_t> _index)           // 5D index in the parent hypergrid
throw (iim::IOException)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    parent = _parent;
    origin = _origin;
    dims   = _dims;
    imdata = 0;
    index  = _index;
    visits = 0;

    path = parent->path + tf::strprintf("t%s_c%s_z%s_y%s_x%s.tif",
                                                             tf::num2str(index.t).c_str(),
                                                             tf::num2str(index.c).c_str(),
                                                             tf::num2str(index.z).c_str(),
                                                             tf::num2str(index.y).c_str(),
                                                             tf::num2str(index.x).c_str());
}


// destructor
tf::HyperGridCache::CacheBlock::~CacheBlock() throw (iim::IOException)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    if(imdata)
        delete imdata;
}

// calculate intersection with current block
tf::voi4D<size_t> tf::HyperGridCache::CacheBlock::xyzt_intersection(tf::voi4D<size_t> range)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    return tf::voi4D<size_t>(
                tf::xyzt<size_t>(std::max(range.start.x, origin.x),
                                 std::max(range.start.y, origin.y),
                                 std::max(range.start.z, origin.z),
                                 std::max(range.start.t, origin.t)),
                tf::xyzt<size_t>(std::min(range.end.x,   origin.x + dims.x),
                                 std::min(range.end.y,   origin.y + dims.y),
                                 std::min(range.end.z,   origin.z + dims.z),
                                 std::min(range.end.t,   origin.t + dims.t)));
}

// calculate channel intersection with current block
std::vector<unsigned int> tf::HyperGridCache::CacheBlock::c_intersection(tf::active_channels<> chans)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    std::vector<unsigned int> chs;
    for(size_t i=0; i<chans.dim; i++)
        if(chans.table[i] >= origin.c && chans.table[i] < origin.c + dims.c)
            chs.push_back(chans.table[i]);
    return chs;
}

// load from disk
void tf::HyperGridCache::CacheBlock::load() throw (iim::IOException, iom::exception)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    if(dims.c > 3)
        throw iim::IOException("I/O functions with c (channels) > 3 not yet implemented", __itm__current__function__);
    if(dims.t > 1)
        throw iim::IOException("I/O functions with t (time) > 1 not yet implemented", __itm__current__function__);

    if(iim::isFile(path))
    {
        if(imdata)
            imdata = new uint8[dims.x * dims.y * dims.z * dims.c];
        int dimX, dimY, dimZ, dimC, bytes;
        iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->readData(path, dimX, dimY, dimZ, bytes, dimC, imdata);
    }
}

// save to disk
void tf::HyperGridCache::CacheBlock::save() throw (iim::IOException, iom::exception)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    if(dims.c > 3)
        throw iim::IOException("I/O functions with c (channels) > 3 not yet implemented", __itm__current__function__);
    if(dims.t > 1)
        throw iim::IOException("I/O functions with t (time) > 1 not yet implemented", __itm__current__function__);

    if(imdata)
    {
        if(!iim::isFile(path))
            iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->create3Dimage(path, dims.y, dims.x, dims.z, 1, dims.c);
        iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->writeData(path, imdata, dims.y, dims.x, dims.z, 1, dims.c);
    }
}
/*---- END HYPER GRID CACHE BLOCK section --------------------------------------------------------------------------------*/
