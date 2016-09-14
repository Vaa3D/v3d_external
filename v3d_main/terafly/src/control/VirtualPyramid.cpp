#include "VirtualPyramid.h"
#include "VirtualVolume.h"
#include <fstream>
#include "IOPluginAPI.h"
#include "CImageUtils.h"

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
        xyz<int> rf = xyz<int>(pow(reduction_factor*1.0, i),pow(reduction_factor*1.0, i),pow(reduction_factor*1.0, i));

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

// get pyramid method
std::vector<iim::VirtualVolume*> tf::VirtualPyramid::getVirtualPyramid()
{
    std::vector<iim::VirtualVolume*> tmp(virtualPyramid.begin(), virtualPyramid.end());
    std::reverse(tmp.begin(), tmp.end());
    return tmp;
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
        std::vector<std::string> tokens = tf::parse(line, "=", 2, volumetxtPath);
        if(tokens[1].compare(highresPath) != 0)
            throw iim::IOException(tf::strprintf("Unconverted image path mismatch at \"%s\": expected \"%s\", found \"%s\"", volumetxtPath.c_str(), highresPath.c_str(), tokens[1].c_str()));
        std::getline(f, line);
        tokens = tf::parse(line, "=", 2, volumetxtPath);
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
        f << "imagepath=" << highresPath << std::endl;

        // then we can store the image format
        f << "format=" << highresVol->getPrintableFormat() << std::endl;
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

        // propagate data to higher layers
		for(size_t l = 1; l <pyramid.size(); l++)
			pyramid[l]->putData
			(
				tf::image5D<uint8>
				(
					data, 
					xyzt<size_t>(end.x - start.x, end.y - start.y, end.z - start.z, highresVol->getNActiveFrames()), 
					active_channels<>(highresVol->getActiveChannels(), highresVol->getNACtiveChannels())
				),
				tf::xyzt<size_t>( start.x, start.y, start.z, highresVol->getT0()), 
				virtualPyramid[l]->reductionFactor * (-1)
			);
    }
    else
    {
        data = pyramid[level]->readData(
                    voi4D<int>(
                        xyzt<int>(start.x, start.y, start.z, highresVol->getT0()),
                        xyzt<int>(end.x, end.y, end.z, highresVol->getT1()+1)),
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
    int dim_v = highresVol->getDIM_V()/static_cast<float>(_reduction_factor.y);
    int dim_h = highresVol->getDIM_H()/static_cast<float>(_reduction_factor.x);
    int dim_d = highresVol->getDIM_D()/static_cast<float>(_reduction_factor.z);

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

    try
    {
        return parent->loadVOI(xyz<size_t>(H0,V0,D0), xyz<size_t>(H1,V1,D1), level).data;
    }
    catch(iom::exception e)
    {
        throw iim::IOException(e.what());
    }
    catch(tf::RuntimeException e)
    {
        throw iim::IOException(e.what());
    }
}
/*---- END VIRTUAL PYRAMID LAYER section ----------------------------------------------------------------------------------*/






/**************************************
*   HYPER GRID  CACHE definitions     *
***************************************
---------------------------------------------------------------------------------------------------------------------------*/
float tf::HyperGridCache::maximumSizeGB = 1;

// constructor 1
tf::HyperGridCache::HyperGridCache(
        std::string _path,                              // where cache files are stored / have to be stored
        xyzct<size_t> _image_dim,                       // image dimensions along X, Y, Z, C (channel), and T (time)
        xyzct<size_t> _block_dim)						// hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
throw (iim::IOException, tf::RuntimeException, iom::exception)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // set object members
    path = _path;
    dimX = _image_dim.x;
    dimY = _image_dim.y;
    dimZ = _image_dim.z;
    dimC = _image_dim.c;
    dimT = _image_dim.t;
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
void tf::HyperGridCache::init() throw (iim::IOException, tf::RuntimeException, iom::exception)
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
void tf::HyperGridCache::load() throw (iim::IOException, tf::RuntimeException, iom::exception)
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
void tf::HyperGridCache::save() throw (iim::IOException, tf::RuntimeException, iom::exception)
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


// read data from the cache (rescaling on-the-fly supported)
tf::image5D<tf::uint8>                          // 5D image output
tf::HyperGridCache::readData(
        tf::voi4D<int> voi,                     // 4D VOI in X,Y,Z,T space
        active_channels<> channels,             // channels to load
        tf::xyz<int> scaling)					// scaling along X,Y and Z (> 0 upscaling, < 0 downscaling) 
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, tf::strprintf("voi = [%d,%d),{%s},[%d,%d),[%d,%d),[%d,%d), downsampling = {%f,%f,%f}",
                                          voi.start.t, voi.end.t, channels.toString().c_str(), voi.start.z, voi.end.z,
                                          voi.start.y, voi.end.y, voi.start.x, voi.end.x, scaling.x, scaling.y, scaling.z).c_str(), __itm__current__function__);

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

    // check scaling
    if(scaling.x != 1 || scaling.y != 1 || scaling.z != 1)
        throw iim::IOException(tf::strprintf("Invalid scaling {%d,%d,%d}: scaling not yet supported when read data from HyperGridCache",
                                              scaling.x, scaling.y, scaling.z), __itm__current__function__);


    // allocate and initialize data
    tf::image5D<tf::uint8> img;
    img.chans = channels;
    img.dims = voi.dims();
    img.data  = new tf::uint8[img.size()];
    for(size_t i=0; i<img.size(); i++)
        img.data[i] = 0;


    // detecting intersecting blocks
    //printf("\n");
    for(size_t t=0; t<nT; t++)
        for(size_t c=0; c<nC; c++)
            for(size_t z=0; z<nZ; z++)
                for(size_t y=0; y<nY; y++)
                    for(size_t x=0; x<nX; x++)
                        if(hypergrid[t][c][z][y][x]->intersection(voi).size()   > 0 &&
                           hypergrid[t][c][z][y][x]->intersection(channels).size() > 0)
                        {
                            voi4D<float> iv = hypergrid[t][c][z][y][x]->intersection(voi);
                            std::vector<unsigned int> ic = hypergrid[t][c][z][y][x]->intersection(channels);

                            // local VOI = intersecting ROI - origin shift
                            voi4D<int> loc_VOI = iv.rounded();
                            loc_VOI -= hypergrid[t][c][z][y][x]->getOrigin();

                            voi4D<int> img_VOI = iv.rounded();
                            img_VOI -= voi.start;


                            printf("readData: hypergrid[%02d][%02d][%02d][%02d][%02d]: block [%d,%d),[%d,%d),[%d,%d),[%d,%d),[%d,%d) intersection WITH [%d,%d),{%s},[%d,%d),[%d,%d),[%d,%d) IS [%.0f,%.0f),{%s},[%.0f,%.0f),[%.0f,%.0f),[%.0f,%.0f)\n",
                                t,c,z,y,x,
                                hypergrid[t][c][z][y][x]->getOrigin().t, hypergrid[t][c][z][y][x]->getOrigin().t + hypergrid[t][c][z][y][x]->getDims().t,
                                hypergrid[t][c][z][y][x]->getOrigin().c, hypergrid[t][c][z][y][x]->getOrigin().c + hypergrid[t][c][z][y][x]->getDims().c,
                                hypergrid[t][c][z][y][x]->getOrigin().z, hypergrid[t][c][z][y][x]->getOrigin().z + hypergrid[t][c][z][y][x]->getDims().z,
                                hypergrid[t][c][z][y][x]->getOrigin().y, hypergrid[t][c][z][y][x]->getOrigin().y + hypergrid[t][c][z][y][x]->getDims().y,
                                hypergrid[t][c][z][y][x]->getOrigin().x, hypergrid[t][c][z][y][x]->getOrigin().x + hypergrid[t][c][z][y][x]->getDims().x,
                                voi.start.t, voi.end.t, img.chans.toString().c_str(), voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x,
                                iv.start.t,  iv.end.t,  active_channels<>::toString(ic).c_str(), iv.start.z,  iv.end.z,  iv.start.y,  iv.end.y,  iv.start.x,  iv.end.x);

                            printf("readData: local VOI is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", loc_VOI.start.t, loc_VOI.end.t, loc_VOI.start.z, loc_VOI.end.z, loc_VOI.start.y, loc_VOI.end.y, loc_VOI.start.x, loc_VOI.end.x);
                            printf("readData: image VOI is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", img_VOI.start.t, img_VOI.end.t, img_VOI.start.z, img_VOI.end.z, img_VOI.start.y, img_VOI.end.y, img_VOI.start.x, img_VOI.end.x);


                            hypergrid[t][c][z][y][x]->readData(img, img_VOI, loc_VOI, scaling);
                        }
    //printf("\n");

    return img;
}

// put data into the cache (rescaling on-the-fly supported)
void tf::HyperGridCache::putData(
    const tf::image5D<uint8> & img,		// 5D image input
	tf::xyzt<size_t> offset,			// offset relative to (0,0,0,0), it is up(or down)scaled if needed
	tf::xyz<int> scaling)				// scaling along X,Y and Z (powers of 2 only)
    throw (iim::IOException, iom::exception, tf::RuntimeException)
{
	/**/tf::debug(tf::LEV1, strprintf("path = \"%s\", img dims = (%d x %d x %d x %d x %d), offset = (%d, %d, %d, %d), scaling = (%d,%d,%d)",
		path.c_str(), img.dims.x, img.dims.y, img.dims.z, img.chans.dim, img.dims.t, offset.x, offset.y, offset.z, offset.t, scaling.x, scaling.y, scaling.z).c_str(), __itm__current__function__);

	// check scaling
	if(!scaling.x || !scaling.y || !scaling.z)
		throw iim::IOException(tf::strprintf("Invalid scaling {%d,%d,%d}",
		scaling.x, scaling.y, scaling.z), __itm__current__function__);
    if( !(scaling.x > 0 && scaling.y > 0 && scaling.z > 0) && !(scaling.x < 0 && scaling.y < 0 && scaling.z < 0))
		throw iim::IOException(tf::strprintf("Invalid scaling {%d,%d,%d}: all three scaling factors must be > 0 (upscaling) or < 0 (downscaling)",
		scaling.x, scaling.y, scaling.z), __itm__current__function__);
	bool upscaling = scaling.x > 0;
	if(!upscaling)
        scaling = scaling * (-1);

	// determine voi
    voi4D<float> voi(
        tf::xyzt<float>(offset.x, offset.y, offset.z, offset.t),
        tf::xyzt<float>(offset.x + img.dims.x, offset.y + img.dims.y, offset.z + img.dims.z, offset.t + img.dims.t));

	// scale voi
	if(upscaling)
        voi *= tf::xyz<float>(scaling);
	else
        voi /= tf::xyz<float>(scaling);


	// put data into intersecting blocks
	printf("put data into intersecting blocks \n");
	for(size_t t=0; t<nT; t++)
		for(size_t c=0; c<nC; c++)
			for(size_t z=0; z<nZ; z++)
				for(size_t y=0; y<nY; y++)
					for(size_t x=0; x<nX; x++)
					{
                        voi4D<float>              iv = hypergrid[t][c][z][y][x]->intersection(voi);
						std::vector<unsigned int> ic = hypergrid[t][c][z][y][x]->intersection(img.chans);

						bool intersects = iv.size() > 0 && ic.size() > 0;
						if(intersects)
						{
                            printf("hypergrid[%02d][%02d][%02d][%02d][%02d]: block [%d,%d),[%d,%d),[%d,%d),[%d,%d),[%d,%d) %s [%.1f,%.1f),{%s},[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f) IS [%.1f,%.1f),{%s},[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f)\n",
								t,c,z,y,x, 
								hypergrid[t][c][z][y][x]->getOrigin().t, hypergrid[t][c][z][y][x]->getOrigin().t + hypergrid[t][c][z][y][x]->getDims().t,
								hypergrid[t][c][z][y][x]->getOrigin().c, hypergrid[t][c][z][y][x]->getOrigin().c + hypergrid[t][c][z][y][x]->getDims().c,
								hypergrid[t][c][z][y][x]->getOrigin().z, hypergrid[t][c][z][y][x]->getOrigin().z + hypergrid[t][c][z][y][x]->getDims().z,
								hypergrid[t][c][z][y][x]->getOrigin().y, hypergrid[t][c][z][y][x]->getOrigin().y + hypergrid[t][c][z][y][x]->getDims().y,
								hypergrid[t][c][z][y][x]->getOrigin().x, hypergrid[t][c][z][y][x]->getOrigin().x + hypergrid[t][c][z][y][x]->getDims().x,
								intersects ? "intersects with" : "DOES NOT intersect with",
								voi.start.t, voi.end.t, img.chans.toString().c_str(),             voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x,
								iv.start.t,  iv.end.t,  active_channels<>::toString(ic).c_str(), iv.start.z,  iv.end.z,  iv.start.y,  iv.end.y,  iv.start.x,  iv.end.x);


                            // local VOI = intersecting ROI - origin shift
                            voi4D<int> loc_VOI = iv.rounded();
                            loc_VOI -= hypergrid[t][c][z][y][x]->getOrigin();

                            // image VOI = scaling(intersecting ROI)
                            voi4D<float> img_VOIf = iv;
							if(upscaling)
                                img_VOIf /= tf::xyz<float>(scaling);
							else
                                img_VOIf *= tf::xyz<float>(scaling);
                            img_VOIf -= tf::xyzt<float>(offset);
                            voi4D<int> img_VOI = img_VOIf.rounded();

							printf("local VOI is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", loc_VOI.start.t, loc_VOI.end.t, loc_VOI.start.z, loc_VOI.end.z, loc_VOI.start.y, loc_VOI.end.y, loc_VOI.start.x, loc_VOI.end.x);
							printf("image VOI is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", img_VOI.start.t, img_VOI.end.t, img_VOI.start.z, img_VOI.end.z, img_VOI.start.y, img_VOI.end.y, img_VOI.start.x, img_VOI.end.x);

							hypergrid[t][c][z][y][x]->putData(img, img_VOI, loc_VOI, upscaling ? scaling : scaling * (-1));
						}
					}
	printf("\n");
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
throw (iim::IOException, iom::exception)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    parent = _parent;
    origin = _origin;
    dims   = _dims;
    imdata = 0;
    index  = _index;
    visits = 0;
    modified = false;

    path = parent->path + "/" + tf::strprintf("t%s_c%s_z%s_y%s_x%s.tif",
                                                             tf::num2str(index.t).c_str(),
                                                             tf::num2str(index.c).c_str(),
                                                             tf::num2str(index.z).c_str(),
                                                             tf::num2str(index.y).c_str(),
                                                             tf::num2str(index.x).c_str());
}


// destructor
tf::HyperGridCache::CacheBlock::~CacheBlock() throw (iim::IOException, iom::exception)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    if(imdata)
	{
        //save();
        delete imdata;
	}
}


// calculate channel intersection with current block
std::vector<unsigned int> tf::HyperGridCache::CacheBlock::intersection(tf::active_channels<> chans)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    std::vector<unsigned int> chs;
    for(size_t i=0; i<chans.dim; i++)
        if(chans.table[i] >= origin.c && chans.table[i] < origin.c + dims.c)
            chs.push_back(chans.table[i]);
    return chs;
}

// load from disk
void tf::HyperGridCache::CacheBlock::load() throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // precondition checks
    if(dims.c > 3)
        throw iim::IOException("I/O functions with c (channels) > 3 not yet implemented", __itm__current__function__);
    if(dims.t > 1)
        throw iim::IOException("I/O functions with t (time) > 1 not yet implemented", __itm__current__function__);
    if(!dims.x || !dims.y || !dims.z || !dims.c ||!dims.t)
        throw iim::IOException("dims <= 0", __itm__current__function__);

    // allocate memory for data if needed
    if(!imdata)
        imdata = new uint8[dims.x * dims.y * dims.z * dims.c * dims.t];

    // try to load data from disk
    if(iim::isFile(path))
    {
        int dimX = dims.x, dimY=dims.y, dimZ=dims.z, dimC=dims.c, bytes = 1;

        if(iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->isChansInterleaved())
        {
            tf::uint8 *interleaved = new tf::uint8[dims.x*dims.y*dims.z*dims.c];
            iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->readData(path, dimX, dimY, dimZ, bytes, dimC, interleaved);
            tf::uint8 *iptr = interleaved;
            size_t dims_zyx = dims.z*dims.y*dims.x;
            for(size_t z=0; z<dims.z; z++)
            {
                size_t zyx = z*dims.y*dims.x;
                for(size_t y=0; y<dims.y; y++)
                {
                    size_t stride  = zyx + y*dims.x;
                    for(size_t x=0; x<dims.x; x++)
                    {
                        for(size_t c=0; c<dims.c; c++, iptr++)
                            imdata[c*dims_zyx + stride + x] = *iptr;
                    }
                }
            }
            delete[] interleaved;
        }
        else
            iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->readData(path, dimX, dimY, dimZ, bytes, dimC, imdata);
    }
    else // otherwise initialize to black
    {
        size_t imdata_size = dims.x * dims.y * dims.z * dims.c * dims.t;
        for(size_t i = 0; i<imdata_size; i++)
            imdata[i] = 0;
    }
}

// save to disk
void tf::HyperGridCache::CacheBlock::save() throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // precondition checks
    if(dims.c > 3)
        throw iim::IOException("I/O functions with c (channels) > 3 not yet implemented", __itm__current__function__);
    if(dims.t > 1)
        throw iim::IOException("I/O functions with t (time) > 1 not yet implemented", __itm__current__function__);
    if(!dims.x || !dims.y || !dims.z || !dims.c ||!dims.t)
        throw iim::IOException("dims <= 0", __itm__current__function__);

    // only save when data has been modified
    if(imdata && modified)
    {
		double t0 = -TIME(0);
        if(!iim::isFile(path))
            iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->create3Dimage(path, dims.y, dims.x, dims.z, 1, dims.c);
        if(iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->isChansInterleaved())
        {
            tf::uint8 *interleaved = new tf::uint8[dims.x*dims.y*dims.z*dims.c];
            tf::uint8 *write_ptr = interleaved;
            size_t dims_zyx = dims.z*dims.y*dims.x;
            for(size_t z=0; z<dims.z; z++)
            {
                size_t zyx = z*dims.y*dims.x;
                for(size_t y=0; y<dims.y; y++)
                {
                    size_t stride  = zyx + y*dims.x;
                    for(size_t x=0; x<dims.x; x++)
                    {
                        for(size_t c=0; c<dims.c; c++, write_ptr++)
                            *write_ptr = imdata[c*dims_zyx + stride + x];
                    }
                }
            }
            iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->writeData(path, interleaved, dims.y, dims.x, dims.z, 1, dims.c);
            delete[] interleaved;
        }
        else
            iom::IOPluginFactory::instance()->getPlugin3D("tiff3D")->writeData(path, imdata, dims.y, dims.x, dims.z, 1, dims.c);
		printf("\n\nblock \"%s\" saved in %.1f seconds\n\n", path.c_str(), t0 + TIME(0));
    }
}

// put data into current block (rescaling on-the-fly supported)
void tf::HyperGridCache::CacheBlock::putData(
    const tf::image5D<uint8> &image,        // 5D image input
	tf::voi4D<int> image_voi,				// image VOI (already scaled)
	tf::voi4D<int> block_voi,				// current block VOI (already scaled)
	tf::xyz<int> scaling					// scaling along X,Y and Z (> 0 upscaling, < 0 downscaling)
    ) throw (iim::IOException, iom::exception, tf::RuntimeException)
{
	/**/tf::debug(tf::LEV1, strprintf("path = \"%s\", img dims = (%d x %d x %d x %d x %d), image voi = [%d,%d),[%d,%d),[%d,%d),[%d,%d) block voi = [%d,%d),[%d,%d),[%d,%d),[%d,%d), scaling = (%d,%d,%d)",
		path.c_str(), 
		image.dims.x, image.dims.y, image.dims.z, image.chans.dim, image.dims.t, 
		image_voi.start.x, image_voi.end.x, image_voi.start.y, image_voi.end.y, image_voi.start.z, image_voi.end.z, image_voi.start.t, image_voi.end.t, 
		block_voi.start.x, block_voi.end.x, block_voi.start.y, block_voi.end.y, block_voi.start.z, block_voi.end.z, block_voi.start.t, block_voi.end.t, 
		scaling.x, scaling.y, scaling.z).c_str(), __itm__current__function__);


	// check scaling
	if(!scaling.x || !scaling.y || !scaling.z)
		throw iim::IOException(tf::strprintf("Invalid scaling {%d,%d,%d}",
		scaling.x, scaling.y, scaling.z), __itm__current__function__);
	if( !(scaling.x > 0 && scaling.y > 0 && scaling.z > 0) && !(scaling.x < 0 && scaling.y < 0 && scaling.z < 0))
		throw iim::IOException(tf::strprintf("Invalid scaling {%d,%d,%d}: all three scaling factors must be > 0 (upscaling) or < 0 (downscaling)",
		scaling.x, scaling.y, scaling.z), __itm__current__function__);

	// check voi
	if(image_voi.start.t < 0 || image_voi.end.t - image_voi.start.t <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along t-axis", image_voi.start.t, image_voi.end.t));
	if(image_voi.start.z < 0 || image_voi.end.z - image_voi.start.z <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along z-axis", image_voi.start.z, image_voi.end.z));
	if(image_voi.start.y < 0 || image_voi.end.y - image_voi.start.y <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along y-axis", image_voi.start.y, image_voi.end.y));
	if(image_voi.start.x < 0 || image_voi.end.x - image_voi.start.x <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along x-axis", image_voi.start.x, image_voi.end.x));
	if(block_voi.start.t < 0 || block_voi.end.t - block_voi.start.t <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along t-axis", block_voi.start.t, block_voi.end.t));
	if(block_voi.start.z < 0 || block_voi.end.z - block_voi.start.z <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along z-axis", block_voi.start.z, block_voi.end.z));
	if(block_voi.start.y < 0 || block_voi.end.y - block_voi.start.y <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along y-axis", block_voi.start.y, block_voi.end.y));
	if(block_voi.start.x < 0 || block_voi.end.x - block_voi.start.x <= 0)
		throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along x-axis", block_voi.start.x, block_voi.end.x));

	// load block if needed
	if(!imdata)
		load();

	// prepare metadata
	unsigned int src_dims[5], src_offset[5], src_count[5], dst_dims[5], dst_offset[5];
	for(size_t i=0; i<5; i++)
	{
		src_dims[i] = image.getDims()[i];
		dst_dims[i] = dims[i];
	}
	src_offset[0] = image_voi.start.x;
	src_offset[1] = image_voi.start.y;
	src_offset[2] = image_voi.start.z;
	src_offset[4] = image_voi.start.t;
	src_count[0]  = image_voi.end.x - image_voi.start.x;
	src_count[1]  = image_voi.end.y - image_voi.start.y;
	src_count[2]  = image_voi.end.z - image_voi.start.z;
	src_count[4]  = image_voi.end.t - image_voi.start.t;
	dst_offset[0] = block_voi.start.x;
	dst_offset[1] = block_voi.start.y;
	dst_offset[2] = block_voi.start.z;
	dst_offset[4] = block_voi.start.t;

	// copy and rescale VOI
    tf::CImageUtils::copyRescaleVOI(image.data, src_dims, src_offset, src_count, imdata, dst_dims, dst_offset, scaling);

	// set this block as modified
	modified = true;
}

// read data from block and put into image buffer (rescaling on-the-fly supported)
void tf::HyperGridCache::CacheBlock::readData(
    tf::image5D<uint8> & image,             // 5D image buffer (preallocated)
    tf::voi4D<int> image_voi,				// image VOI (already scaled)
    tf::voi4D<int> block_voi,				// current block VOI (already scaled)
    tf::xyz<int> scaling					// scaling along X,Y and Z (> 0 upscaling, < 0 downscaling)
) throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("path = \"%s\", img dims = (%d x %d x %d x %d x %d), image voi = [%d,%d),[%d,%d),[%d,%d),[%d,%d) block voi = [%d,%d),[%d,%d),[%d,%d),[%d,%d), scaling = (%d,%d,%d)",
        path.c_str(),
        image.dims.x, image.dims.y, image.dims.z, image.chans.dim, image.dims.t,
        image_voi.start.x, image_voi.end.x, image_voi.start.y, image_voi.end.y, image_voi.start.z, image_voi.end.z, image_voi.start.t, image_voi.end.t,
        block_voi.start.x, block_voi.end.x, block_voi.start.y, block_voi.end.y, block_voi.start.z, block_voi.end.z, block_voi.start.t, block_voi.end.t,
        scaling.x, scaling.y, scaling.z).c_str(), __itm__current__function__);


    // check scaling
    if(!scaling.x || !scaling.y || !scaling.z)
        throw iim::IOException(tf::strprintf("Invalid scaling {%d,%d,%d}",
        scaling.x, scaling.y, scaling.z), __itm__current__function__);
    if( !(scaling.x > 0 && scaling.y > 0 && scaling.z > 0) && !(scaling.x < 0 && scaling.y < 0 && scaling.z < 0))
        throw iim::IOException(tf::strprintf("Invalid scaling {%d,%d,%d}: all three scaling factors must be > 0 (upscaling) or < 0 (downscaling)",
        scaling.x, scaling.y, scaling.z), __itm__current__function__);

    // check voi
    if(image_voi.start.t < 0 || image_voi.end.t - image_voi.start.t <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along t-axis", image_voi.start.t, image_voi.end.t));
    if(image_voi.start.z < 0 || image_voi.end.z - image_voi.start.z <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along z-axis", image_voi.start.z, image_voi.end.z));
    if(image_voi.start.y < 0 || image_voi.end.y - image_voi.start.y <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along y-axis", image_voi.start.y, image_voi.end.y));
    if(image_voi.start.x < 0 || image_voi.end.x - image_voi.start.x <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along x-axis", image_voi.start.x, image_voi.end.x));
    if(block_voi.start.t < 0 || block_voi.end.t - block_voi.start.t <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along t-axis", block_voi.start.t, block_voi.end.t));
    if(block_voi.start.z < 0 || block_voi.end.z - block_voi.start.z <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along z-axis", block_voi.start.z, block_voi.end.z));
    if(block_voi.start.y < 0 || block_voi.end.y - block_voi.start.y <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along y-axis", block_voi.start.y, block_voi.end.y));
    if(block_voi.start.x < 0 || block_voi.end.x - block_voi.start.x <= 0)
        throw iim::IOException(tf::strprintf("Invalid image VOI [%d,%d) along x-axis", block_voi.start.x, block_voi.end.x));

    // load block if needed
    if(!imdata)
        load();

    // prepare metadata
    unsigned int src_dims[5], src_offset[5], src_count[5], dst_dims[5], dst_offset[5];
    for(size_t i=0; i<5; i++)
    {
        src_dims[i] = dims[i];
        dst_dims[i] = image.getDims()[i];
    }
    dst_offset[0] = image_voi.start.x;
    dst_offset[1] = image_voi.start.y;
    dst_offset[2] = image_voi.start.z;
    dst_offset[4] = image_voi.start.t;
    src_count[0]  = block_voi.end.x - block_voi.start.x;
    src_count[1]  = block_voi.end.y - block_voi.start.y;
    src_count[2]  = block_voi.end.z - block_voi.start.z;
    src_count[4]  = block_voi.end.t - block_voi.start.t;
    src_offset[0] = block_voi.start.x;
    src_offset[1] = block_voi.start.y;
    src_offset[2] = block_voi.start.z;
    src_offset[4] = block_voi.start.t;

    // copy and rescale VOI
    tf::CImageUtils::copyRescaleVOI(imdata, src_dims, src_offset, src_count, image.data, dst_dims, dst_offset, scaling);
}

/*---- END HYPER GRID CACHE BLOCK section --------------------------------------------------------------------------------*/
