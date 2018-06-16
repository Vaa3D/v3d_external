#include "VirtualPyramid.h"
#include "VirtualVolume.h"
#include <fstream>
#include "IOPluginAPI.h"
#include "CImageUtils.h"
#include "basic_4dimage.h"


/********************************
*   VIRTUAL PYRAMID definitions *
*********************************
---------------------------------------------------------------------------------------------------------------------------*/
tf::VirtualPyramid::empty_filling tf::VirtualPyramid::_unexploredFillingMethod = tf::VirtualPyramid::RAW;
unsigned char tf::VirtualPyramid::_unexploredIntensityVal = 128;
float tf::VirtualPyramid::_unexploredSaltAndPepperPerc = 0.001;
bool tf::VirtualPyramid::_cacheHighestRes = false;
bool tf::VirtualPyramid::_freezeHighestRes = false;


// VirtualPyramid constructor 1
tf::VirtualPyramid::VirtualPyramid(
        std::string           highresPath,              // highest-res (unconverted) volume path
        int                 reduction_factor,           // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
        float lower_bound /*= 100*/,                    // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
        iim::VirtualVolume* highresVol /*= 0*/,         // highest-res (unconverted) volume, if null will be instantiated on-the-fly
        init_mode mode /*= DEFAULT*/,                   // initialization mode
        const std::string & lowResImagePath /*= ""*/,   // path of low-res image file (to be used when mode == GENERATE_LOW_RES_FROM_FILE)
        int sampling /*= 16*/,                          // sampling factor (to be used when mode == GENERATE_LOW_RES || GENERATE_ALL)
        int local /*= true*/,                           // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
        tf::xyz<size_t> block_dim,                      // x-y-z dimensions of virtual pyramid blocks
        const std::string block_format /*= ".tif"*/     // block file format
)
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // check first time instance
    if(iim::isDirectory( local ? pathLocal(_volPath) : pathRemote(_volPath)))
        throw iim::IOException(iim::strprintf("Cannot setup a new Virtual Pyramid in \"%s\": another Virtual Pyramid already exists at this path", ( local ? pathLocal(_volPath) : pathRemote(_volPath)).c_str()));

    // set object members
    _vol = highresVol;
    _volPath = highresPath;

    // init working folder
    initPath(local);

    // instance highest-res volume
    initVolume();

    // iteratively add a new layer until the lowest-res layer has size > lower_bound
    int i = 0;
    do
    {
        xyz<int> rf = xyz<int>(pow(reduction_factor*1.0, i),pow(reduction_factor*1.0, i),pow(reduction_factor*1.0, i));

        // instance only nonempty layers
        if(VirtualPyramidLayer::isEmpty(_vol, rf))
            break;
        else
        {
            _virtualPyramid.push_back(new VirtualPyramidLayer(i, this, rf));
            i++;
        }
    }
    while (_virtualPyramid.back()->getMVoxels() > lower_bound);

    // at least two layers are needed
    if(_virtualPyramid.size() < 2)
        throw iim::IOException("Cannot instance Virtual Pyramid with the given settings: at least 2 layers needed. Please check resampling/reduction options or your image size.");

    // create/check metadata
    initPyramid(block_dim, block_format);
}


// VirtualPyramid constructor 2
tf::VirtualPyramid::VirtualPyramid(
        std::string  highresPath,                       // highest-res (unconverted) volume path
        std::vector< xyz<int> > reduction_factors,      // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
        iim::VirtualVolume* highresVol /*= 0*/,         // highest-res (unconverted) volume, if null will be instantiated on-the-fly
        init_mode mode /*= DEFAULT*/,                   // initialization mode
        const std::string & lowResImagePath /*= ""*/,   // path of low-res image file (to be used when mode == GENERATE_LOW_RES_FROM_FILE)
        int sampling /*= 16*/,                          // sampling factor (to be used when mode == GENERATE_LOW_RES || GENERATE_ALL)
        int local /*= true*/,                           // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
        tf::xyz<size_t> block_dim,                      // x-y-z dimensions of virtual pyramid blocks
        const std::string block_format /*= ".tif"*/     // block file format
)
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // check first time instance
    if(iim::isDirectory( local ? pathLocal(_volPath) : pathRemote(_volPath)))
        throw iim::IOException(iim::strprintf("Cannot setup a new Virtual Pyramid in \"%s\": another Virtual Pyramid already exists at this path", ( local ? pathLocal(_volPath) : pathRemote(_volPath)).c_str()));

    // set object members
    _vol = highresVol;
    _volPath = highresPath;

    // init working folder
    initPath(local);

    // instance highest-res volume
    initVolume();

    // generate layers according to the given reduction factors
    for(int i=0; i<reduction_factors.size(); i++)
    {
        // instance only nonempty layers
        if(!VirtualPyramidLayer::isEmpty(_vol, reduction_factors[i]))
            _virtualPyramid.push_back(new VirtualPyramidLayer(i, this, reduction_factors[i]));
    }

    // at least two layers are needed
    if(_virtualPyramid.size() < 2)
        throw iim::IOException("Cannot instance Virtual Pyramid with the given settings: at least 2 layers needed. Please check resampling/reduction options or your image size.");

    // create/check metadata
    initPyramid(block_dim, block_format);
}

// pyramid size predictors
float tf::VirtualPyramid::predictGB(
    iim::VirtualVolume* highresVol,             // highest-res (unconverted) volume
    int reduction_factor,                       // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
    float lower_bound                           // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
)  throw (tf::RuntimeException)
{
    if(!highresVol)
        throw tf::RuntimeException("high res volume not instantiated");

    float GB = 0;
    float MB_i = 0;

    // iteratively add a new layer until the lowest-res layer has size > lower_bound
    int i = 1;
    do
    {
        xyz<int> rf = xyz<int>(pow(reduction_factor*1.0, i),pow(reduction_factor*1.0, i),pow(reduction_factor*1.0, i));

        // instance only nonempty layers
        if(VirtualPyramidLayer::isEmpty(highresVol, rf))
            break;
        else
        {
            int DIM_C = highresVol->getDIM_C();
            int BYTESxCHAN = highresVol->getBYTESxCHAN();
            int DIM_T = highresVol->getDIM_T();
            int DIM_V = int(highresVol->getDIM_V()/static_cast<float>(rf.y));
            int DIM_H = int(highresVol->getDIM_H()/static_cast<float>(rf.x));
            int DIM_D = int(highresVol->getDIM_D()/static_cast<float>(rf.z));
            GB   += (DIM_T/1000.0f) * (DIM_C/1000.0f) * (BYTESxCHAN/1000.0f) * DIM_V * DIM_H * DIM_D;
            MB_i  = (DIM_T/1000.0f) * (DIM_C/1000.0f) *  BYTESxCHAN          * DIM_V * DIM_H * DIM_D;
            i++;
        }
    }
    while (MB_i > lower_bound);

    // at least two layers are needed: return 0 (invalid pyramid)
    if(i < 2)
        return 0;
    else
        return GB;
}
float tf::VirtualPyramid::predictGB(
    iim::VirtualVolume* highresVol,             // highest-res (unconverted) volume
    std::vector< xyz<int> > reduction_factors   // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
)  throw (tf::RuntimeException)
{
    if(!highresVol)
        throw tf::RuntimeException("high res volume not instantiated");

    float GB = 0;

    // generate layers according to the given reduction factors
    int nonempty=0;
    for(int i=1; i<reduction_factors.size(); i++)
    {
        // instance only nonempty layers
        if(!VirtualPyramidLayer::isEmpty(highresVol, reduction_factors[i]))
        {
            int DIM_C = highresVol->getDIM_C();
            int BYTESxCHAN = highresVol->getBYTESxCHAN();
            int DIM_T = highresVol->getDIM_T();
            int DIM_V = int(highresVol->getDIM_V()/static_cast<float>(reduction_factors[i].y));
            int DIM_H = int(highresVol->getDIM_H()/static_cast<float>(reduction_factors[i].x));
            int DIM_D = int(highresVol->getDIM_D()/static_cast<float>(reduction_factors[i].z));
            GB += (DIM_T/1000.0f) * (DIM_C/1000.0f) * (BYTESxCHAN/1000.0f) * DIM_V * DIM_H * DIM_D;

            nonempty++;
        }
    }

    // at least two layers (high res + one resampled) needed: return 0 (invalid pyramid)
    if(nonempty < 1)
        return 0;
    else
        return GB;
}

// constructor 3 (Virtual Pyramid files already exist)
tf::VirtualPyramid::VirtualPyramid(
        std::string highresPath,                   // highest-res (unconverted) volume path
        iim::VirtualVolume* highresVol,            // highest-res (unconverted) volume, if null will be instantiated on-the-fly
        int local)                                  // store data on local drive (i.e. exe's folder) or remote storage (i.e. volume's folder)
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // set object members
    _vol = highresVol;
    _volPath = highresPath;

    // init working folder
    initPath(local);

    // instance highest-res volume
    initVolume();

    // create/check metadata
    initPyramid();
}

// VirtualPyramid deconstructor
tf::VirtualPyramid::~VirtualPyramid() throw(iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    try
    {
		this->clear(false);
    }
	catch (tf::RuntimeException & e)
    {
		throw iim::IOException(e.what());
	}
	catch (iom::exception & e)
	{
		throw iim::IOException(e.what());
	}

    // virtual layers should automatically remove themselves from virtualPyramid once they are destroyed
    if(_virtualPyramid.size())
        throw tf::RuntimeException("Cannot destroy VirtualPyramid: not all layers have been destroyed");

    // delete cache layers
    for(int i=0; i<_cachePyramid.size(); i++)
        delete _cachePyramid[i];

    // release highest res volume
    if(_vol)
        delete _vol;
}

// get pyramid method
std::vector<iim::VirtualVolume*> tf::VirtualPyramid::virtualPyramid()
{
    std::vector<iim::VirtualVolume*> tmp(_virtualPyramid.begin(), _virtualPyramid.end());
    std::reverse(tmp.begin(), tmp.end());
    return tmp;
}

// return path where Virtual Pyramid data are expected to be found on local storage (i.e. executable's folder)
std::string tf::VirtualPyramid::pathLocal(const std::string & _highresPath)
{
    return QDir::currentPath().toStdString() + "/.terafly/." + QString(QCryptographicHash::hash(tf::getFileName(_highresPath).c_str(),QCryptographicHash::Md5).toHex()).toStdString() + "/.virtualpyramid";
}

 // return path where Virtual Pyramid data are expected to be found on remote storage (i.e. volume's folder)
std::string tf::VirtualPyramid::pathRemote(const std::string & _highresPath)
{
    if(iim::isFile(_highresPath))
    {
        QDir remote_dir(_highresPath.c_str());
        remote_dir.cdUp();
        return remote_dir.absolutePath().toStdString() +  "/.terafly/." + QString(QCryptographicHash::hash(tf::getFileName(_highresPath).c_str(),QCryptographicHash::Md5).toHex()).toStdString() + "/.virtualpyramid";
    }
    else if (iim::isDirectory(_highresPath))
        return _highresPath + "/.terafly/.virtualpyramid";

    else
        return "<error: path does not exist>";
}

// return true if Virtual Pyramid files are found in either local or remote storage (see getLocalPath and getRemotePath)
bool tf::VirtualPyramid::exist(const std::string & _highresPath)
{
    return iim::isDirectory(pathLocal(_highresPath)) || iim::isDirectory(pathRemote(_highresPath));
}

// return true if Virtual Pyramid files are found in local AND remote storage (see getLocalPath and getRemotePath)
bool tf::VirtualPyramid::existTwice(const std::string & _highresPath)
{
    return iim::isDirectory(pathLocal(_highresPath)) && iim::isDirectory(pathRemote(_highresPath));
}

// return true if Virtual Pyramid files are found on local storage (see getLocalPath)
bool tf::VirtualPyramid::existOnLocal(const std::string & _highresPath)
{
    return iim::isDirectory(pathLocal(_highresPath));
}

// return true if Virtual Pyramid files are found on remote storage (see getRemotePath)
bool tf::VirtualPyramid::existOnRemote(const std::string & _highresPath)
{
    return iim::isDirectory(pathRemote(_highresPath));
}

// return path where low res image file is expected to be found (if any)
std::string tf::VirtualPyramid::pathLowRes(const std::string & _highresPath)
{
    if(iim::isDirectory(_highresPath))
        return _highresPath + "/lowres.tif";
    else
        return "";
}

void tf::VirtualPyramid::initPath(bool local) throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    if(local)
    {
        // create terafly local folder in the executable directory
        if(!iim::check_and_make_dir(".terafly"))
            throw iim::IOException("Cannot create local folder \".terafly\" in the executable folder");

        // create subfolder with the MD5 id generated from the image file name / folder name
        std::string MD5_path = QDir::currentPath().toStdString() + std::string("/.terafly/.") + QString(QCryptographicHash::hash(tf::getFileName(_volPath).c_str(),QCryptographicHash::Md5).toHex()).toStdString();
        if(!iim::check_and_make_dir(MD5_path.c_str()))
            throw iim::IOException(tf::strprintf("Cannot create local folder for unconverted volume at \"%s\"", MD5_path.c_str()));

        // create .virtualpyramid subfolder
        _path = MD5_path + "/.virtualpyramid";
        if(!iim::check_and_make_dir(_path.c_str()))
            throw iim::IOException(tf::strprintf("Cannot create local folder for Virtual Pyramid at \"%s\"", _path.c_str()));
    }
    else
    {
        if(iim::isFile(_volPath))
        {
            QDir remote_dir(_volPath.c_str());
            remote_dir.cdUp();
            std::string rpath = remote_dir.absolutePath().toStdString();
            rpath += "/.terafly";

            // create terafly local folder in the volume's file contaienr folder
            if(!iim::check_and_make_dir(rpath.c_str()))
                throw iim::IOException(iim::strprintf("Cannot create remote folder for Virtual Pyramid at \"%s\"", rpath.c_str()));

            // create subfolder with the MD5 id generated from the image file name / folder name
            std::string MD5_path = rpath + "/." + QString(QCryptographicHash::hash(tf::getFileName(_volPath).c_str(),QCryptographicHash::Md5).toHex()).toStdString();
            if(!iim::check_and_make_dir(MD5_path.c_str()))
                throw iim::IOException(iim::strprintf("Cannot create remote folder for Virtual Pyramid at \"%s\"", MD5_path.c_str()));

            // create .virtualpyramid subfolder
            _path = MD5_path + "/.virtualpyramid";
            if(!iim::check_and_make_dir(_path.c_str()))
                throw iim::IOException(tf::strprintf("Cannot create remote folder for Virtual Pyramid at \"%s\"", _path.c_str()));
        }
        else
        {
            std::string rpath = _volPath + "/.terafly";

            // create terafly local folder in the volume's file contaienr folder
            if(!iim::check_and_make_dir(rpath.c_str()))
                throw iim::IOException(iim::strprintf("Cannot create remote folder for Virtual Pyramid at \"%s\"", rpath.c_str()));

            // create .virtualpyramid subfolder
            _path = rpath + "/.virtualpyramid";
            if(!iim::check_and_make_dir(_path.c_str()))
                throw iim::IOException(tf::strprintf("Cannot create remote folder for Virtual Pyramid at \"%s\"", _path.c_str()));
        }
    }
}

void tf::VirtualPyramid::initVolume() throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // create (or check) metadata for Virtual Pyramid
    std::string volumetxtPath = _path + "/.volume.txt";
    if(iim::isFile(volumetxtPath))
    {
        std::ifstream f(volumetxtPath.c_str());
        if(!f.is_open())
            throw iim::IOException("Cannot open .volume.txt file at \"%s\"", volumetxtPath.c_str());
        std::string line;
        std::getline(f, line);
        std::vector<std::string> tokens = tf::parse(line, "=", 2, volumetxtPath);
        if(tokens[1].compare(tf::getFileName(_volPath)) != 0)
            throw iim::IOException(tf::strprintf("Unconverted image path mismatch at \"%s\": expected \"%s\", found \"%s\"", volumetxtPath.c_str(), _volPath.c_str(), tokens[1].c_str()));
        std::getline(f, line);
        tokens = tf::parse(line, "=", 2, volumetxtPath);
        if(_vol && tokens[1].compare(_vol->getPrintableFormat()) != 0)
            throw iim::IOException(tf::strprintf("Unconverted image path mismatch at \"%s\": expected \"%s\", found \"%s\"", volumetxtPath.c_str(), _volPath.c_str(), tokens[1].c_str()));
        else
            _vol = iim::VirtualVolume::instance_format(_volPath.c_str(), tokens[1]);
        f.close();
    }
    {
        // if volume is not instantiated, and we haven't done it before, we have to use the (time-consuming) auto instantiator
        // @FIXED by Alessandro on 2016/03/25: this should be done BEFORE creating the volume.txt file
        // so that in case it throws, no (incomplete) volume.txt is created
        if(_vol == 0)
            _vol = iim::VirtualVolume::instance(_volPath.c_str());

        std::ofstream f(volumetxtPath.c_str());
        if(!f.is_open())
            throw iim::IOException("Cannot open .volume.txt file at \"%s\"", volumetxtPath.c_str());
        f << "imagename=" << tf::getFileName(_volPath) << std::endl;

        // then we can store the image format
        f << "format=" << _vol->getPrintableFormat() << std::endl;
        f.close();
    }
}

void tf::VirtualPyramid::initPyramid(
        tf::xyz<size_t> block_dim,          // block dimensions
        const std::string block_format      // block file format
)  throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // init from .setup file if exists
    std::string setup_file_path = _path + "/.setup.txt";
    if(iim::isFile(setup_file_path))
    {
        if(_virtualPyramid.empty())
        {
            std::ifstream setup_file(setup_file_path.c_str());
            if(!setup_file.is_open())
                throw iim::IOException("Cannot open .setup.txt file at \"%s\"", setup_file_path.c_str());
            std::string line;
            while(std::getline(setup_file, line))
            {
                std::vector <std::string> factors;
                tf::parse(line, " ", 3, setup_file_path, factors);

                tf::xyz<int> reduction_factor(tf::str2num<int>(factors[0]), tf::str2num<int>(factors[1]), tf::str2num<int>(factors[2]));

                // instance only nonempty layers
                if(!VirtualPyramidLayer::isEmpty(_vol, reduction_factor))
                    _virtualPyramid.push_back(new VirtualPyramidLayer(_virtualPyramid.size(), this, reduction_factor));
            }
            setup_file.close();
        }
    }
    else
    {
        // create new .setup file
        if(_virtualPyramid.empty())
            throw iim::IOException("Cannot initialize virtual pyramid: .setup file does not exist AND virtual pyramid is empty");

        std::ofstream setup_file(setup_file_path.c_str());
        if(!setup_file.is_open())
            throw iim::IOException("Cannot write .setup.txt file at \"%s\"", setup_file_path.c_str());
        for(int k=0; k<_virtualPyramid.size(); k++)
            setup_file << _virtualPyramid[k]->resamplingFactor().x << " " << _virtualPyramid[k]->resamplingFactor().y << " " << _virtualPyramid[k]->resamplingFactor().z << "\n";
        setup_file.close();
    }


    // at least two layers are needed
    if(_virtualPyramid.size() < 2)
        throw iim::IOException("Cannot instance Virtual Pyramid with the given settings: at least 2 layers needed. Please check resampling/reduction options or your image size.");

    // create (or check) metadata for Virtual Pyramid Layers
    for(int k=0; k<_virtualPyramid.size(); k++)
    {
        // create subfolder
        std::string subfolder = _path + tf::strprintf("/.layer%02d", k);
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
            if(tf::str2num<int>(tokens[0]) != _virtualPyramid[k]->_resamplingFactor.x)
                throw iim::IOException(tf::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along X (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), tf::str2num<int>(tokens[0]), _virtualPyramid[k]->_resamplingFactor.x ));
            if(tf::str2num<int>(tokens[1]) != _virtualPyramid[k]->_resamplingFactor.y)
                throw iim::IOException(tf::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along Y (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), tf::str2num<int>(tokens[1]), _virtualPyramid[k]->_resamplingFactor.y ));
            if(tf::str2num<int>(tokens[2]) != _virtualPyramid[k]->_resamplingFactor.z)
                throw iim::IOException(tf::strprintf("Virtual pyramid layer at \"%s\" was generated with a different reduction factor along Z (%d) than the one currently selected (%d). "
                                                      "You can either adjust your local settings or delete the layer so it will be re-generated automatically.",
                                                      subfolder.c_str(), tf::str2num<int>(tokens[2]), _virtualPyramid[k]->_resamplingFactor.z ));
            f.close();
        }
        else
        {
            std::ofstream f(reduction_factor_file.c_str());
            if(!f.is_open())
                throw iim::IOException(tf::strprintf("Cannot open reduction factor file at \"%s\"", reduction_factor_file.c_str()));
            f << _virtualPyramid[k]->_resamplingFactor.x << " " << _virtualPyramid[k]->_resamplingFactor.y << " " << _virtualPyramid[k]->_resamplingFactor.z << std::endl;
            f.close();
        }

        // instance cache
        tf::xyzct<size_t> block5Ddim(block_dim.x, block_dim.y, block_dim.z, std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());
        _cachePyramid.push_back(new HyperGridCache(subfolder,
                                             xyzct<size_t>(
                                                 _virtualPyramid[k]->getDIM_H(),
                                                 _virtualPyramid[k]->getDIM_V(),
                                                 _virtualPyramid[k]->getDIM_D(),
                                                 _virtualPyramid[k]->getDIM_C(),
                                                 _virtualPyramid[k]->getDIM_T()), block5Ddim, block_format));
    }
}

// load volume of interest from the given resolution layer
tf::image5D<tf::uint8>
tf::VirtualPyramid::loadVOI(
        tf::xyz<size_t> start,  // xyz range [start, end)
        tf::xyz<size_t> end,    // xyz range [start, end)
        int level)              // pyramid level (0=highest resolution, the higher the lower the resolution)
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, tf::strprintf("VOI is x=[%d,%d) y=[%d,%d) z=[%d,%d), level = %d", start.x, end.x, start.y, end.y, start.z, end.z, level).c_str(), __itm__current__function__);

    // checks
    if(level < 0 || level > _cachePyramid.size())
        throw tf::RuntimeException(tf::strprintf("Pyramid level %d out of _cachePyramid bounds [0, %d)", level, _cachePyramid.size()));
    if(level < 0 || level > _virtualPyramid.size())
        throw tf::RuntimeException(tf::strprintf("Pyramid level %d out of _virtualPyramid bounds [0, %d)", level, _virtualPyramid.size()));
    if(end.x <= start.x || end.y <= start.y || end.z <= start.z)
        throw tf::RuntimeException(tf::strprintf("Invalid VOI [%d,%d) x [%d,%d) x [%d,%d)", start.x, end.x, start.y, end.y, start.z, end.z));

    // x,y,z truncation
    if(end.x > _virtualPyramid[level]->getDIM_H())
        end.x = _virtualPyramid[level]->getDIM_H();
    if(end.y > _virtualPyramid[level]->getDIM_V())
        end.y = _virtualPyramid[level]->getDIM_V();
    if(end.z > _virtualPyramid[level]->getDIM_D())
        end.z = _virtualPyramid[level]->getDIM_D();

    // convert 3D interval to voi type
    iim::voi3D<size_t> voi(iim::xyz<size_t>(start.x, start.y, start.z), iim::xyz<size_t>(end.x, end.y, end.z));

    // prepare output data array
    iim::uint8* data = 0;

    // highest-res layer = THE image or the lowest virtual layer
    if(level == 0)
    {
        // no data can be fethed (_freezeHighestRes = true)
        // OR
        // cache highest res option is ON and the VOI is available (either from cache or converted file)
        //  --> read data from lowest virtual pyramid layer
        if( _freezeHighestRes || (_cacheHighestRes && _cachePyramid[0]->completeness(voi) >= 1))
        {
            data = _cachePyramid[0]->readData(
                        voi4D<int>(
                            xyzt<int>(start.x, start.y, start.z, _vol->getT0()),
                            xyzt<int>(end.x, end.y, end.z, _vol->getT1()+1)),
                        active_channels<>(_vol->getActiveChannels(), _vol->getNACtiveChannels())).data;
        }
        // in all other cases
        //  --> fetch data from unconverted image and propagate data to virtual pyramid
        else
        {
            // fetch data
            data = _vol->loadSubvolume_to_UINT8(start.y, end.y, start.x, end.x, start.z, end.z);

            // replace 0 with 1 ('0' value is reserved for empty/nonloaded voxels)
            // ***WARNING*** : here, we are deliberately changing the meaning of the data for efficient representation, counting, and access of empty/nonloaded voxels
            size_t data_dim = (end.z - start.z)*(end.y - start.y)*(end.x - start.x)*_vol->getNActiveFrames()*_vol->getNACtiveChannels();
            for(size_t i=0; i<data_dim; i++)
             data[i] = std::max(iim::uint8(1), data[i]);

            // propagate data
            for(size_t l = !_cacheHighestRes; l <_cachePyramid.size(); l++)
                 _cachePyramid[l]->putData
                 (
                     tf::image5D<uint8>
                     (
                         data,
                         xyzt<size_t>(end.x - start.x, end.y - start.y, end.z - start.z, _vol->getNActiveFrames()),
                         active_channels<>(_vol->getActiveChannels(), _vol->getNACtiveChannels())
                     ),
                     tf::xyzt<size_t>( start.x, start.y, start.z, _vol->getT0()),
                     _virtualPyramid[l]->_resamplingFactor * (-1)
                 );
        }
    }
    // lower-res layers = virtual layers
    else
    {
        // read data
        data = _cachePyramid[level]->readData(
                    voi4D<int>(
                        xyzt<int>(start.x, start.y, start.z, _vol->getT0()),
                        xyzt<int>(end.x, end.y, end.z, _vol->getT1()+1)),
                    active_channels<>(_vol->getActiveChannels(), _vol->getNACtiveChannels())).data;

        // if required, replace empty voxels (0s) with the chosen visualization method for empty image regions
        if(_unexploredFillingMethod == tf::VirtualPyramid::SALT_AND_PEPPER)
        {
            size_t data_dim = (end.z - start.z)*(end.y - start.y)*(end.x - start.x)*_vol->getNActiveFrames()*_vol->getNACtiveChannels();
            for(size_t i=0; i<data_dim; i++)
                if(!data[i] && ((double)rand() / RAND_MAX) < _unexploredSaltAndPepperPerc)
                    data[i] = _unexploredIntensityVal;
        }
        if(_unexploredFillingMethod == tf::VirtualPyramid::SOLID)
        {
            size_t data_dim = (end.z - start.z)*(end.y - start.y)*(end.x - start.x)*_vol->getNActiveFrames()*_vol->getNACtiveChannels();
            for(size_t i=0; i<data_dim; i++)
                if(!data[i])
                    data[i] = _unexploredIntensityVal;
        }
    }
    return tf::image5D<uint8>(
                data,
                xyzt<size_t>(end.x-start.x, end.y-start.y, end.z-start.z, _vol->getNActiveFrames()),
                active_channels<>(_vol->getActiveChannels(), _vol->getNACtiveChannels()));
}

// refills pyramid by searching the first noncomplete region in the given cache layer and VOI and populating it with real image data
// - image data are taken from the unconverted image
// - block dim (if not provided) = tile dim if volume is tiled, otherwise block dim = block dim of highest-res cache layer
bool                                // return true if refill was successful, otherwise return false (no empty regions found)
tf::VirtualPyramid::refill(
        int cache_level,            // cache level where to search the 'emptiest' region (default: lowest-res cache layer)
        iim::voi3D<> VOI,           // volume of interest in the 'cache_level' coordinate system
        refill_strategy strategy,   // refill strategy
        tf::xyz<size_t> block_dim   // block dimension
) throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    // check preconditions
    if(!_vol)
        throw tf::RuntimeException("in VirtualPyramid::refill(): volume has not been initialized yet");
    if(_cachePyramid.empty())
        throw tf::RuntimeException("in VirtualPyramid::refill(): cache pyramid has not been initialized yet");
    if(_freezeHighestRes)
        return false;

    // check / adjust optional inputs
    if (cache_level < 0)
        cache_level = _cachePyramid.size()-1;   // default: lowest-res layer
    VOI.end.x = std::min(VOI.end.x, size_t(_vol->getDIM_H()));
    VOI.end.y = std::min(VOI.end.y, size_t(_vol->getDIM_V()));
    VOI.end.z = std::min(VOI.end.z, size_t(_vol->getDIM_D()));
    if(!VOI.isValid())
        throw tf::RuntimeException(tf::strprintf("in VirtualPyramid::refill(): invalid VOI x=[%d,%d) y=[%d,%d) z=[%d,%d)", VOI.start.x, VOI.end.x, VOI.start.y, VOI.end.y, VOI.start.z, VOI.end.z));

    // the set of tiles to be checked for completeness
    std::vector < iim::voi3D<size_t> > tiles;

    // block dim was not specified: we use volume tiles as blocks
    if(block_dim == tf::xyz<size_t>::biggest())
    {
        // get tiles from volume
        tiles = _vol->tilesXYZ();

        // volume is tiled only along x,y: we need to further partition along z
        if(tiles.size() && _vol->isTiled(iim::dimension_x) && _vol->isTiled(iim::dimension_y) && !_vol->isTiled(iim::dimension_z))
        {
            // we use block dim (along z) of highest-cache layer to partition
            std::vector<size_t> zparts = tf::partition(size_t(_vol->getDIM_D()), _cachePyramid[0]->blockDim().z);

            std::vector < iim::voi3D<size_t> > tiles_sub;
            for(size_t i=0; i<tiles.size(); i++)
            {
                size_t zcount = 0;
                for(size_t j=0; j<zparts.size(); j++)
                {
                    tiles_sub.push_back( iim::voi3D<size_t>(
                                             iim::xyz<size_t>(tiles[i].start.x, tiles[i].start.y, zcount),
                                             iim::xyz<size_t>(tiles[i].end.x  , tiles[i].end.y,   zcount + zparts[j]) ) );
                    zcount += zparts[j];
                }
            }
            tiles = tiles_sub;
        }
    }

    // no tiles available (or block dim specified): we need to partition along x,y,z
    if(tiles.empty())
    {
        // block dim not specified: set block dim = block dim of highest-res cache layer
        if(block_dim == tf::xyz<size_t>::biggest())
            block_dim = _cachePyramid[0]->blockDim().toXYZ();

        // we use block dim (along x,y,z) of highest-cache layer to partition
        std::vector<size_t> xparts = tf::partition(size_t(_vol->getDIM_H()), block_dim.x);
        std::vector<size_t> yparts = tf::partition(size_t(_vol->getDIM_V()), block_dim.y);
        std::vector<size_t> zparts = tf::partition(size_t(_vol->getDIM_D()), block_dim.z);

        size_t xcount = 0;
        for(size_t i=0; i<xparts.size(); i++)
        {
            size_t ycount = 0;
            for(size_t j=0; j<yparts.size(); j++)
            {
                size_t zcount = 0;
                for(size_t k=0; k<zparts.size(); k++)
                {
                    tiles.push_back( iim::voi3D<size_t>(
                                             iim::xyz<size_t>(xcount,             ycount,             zcount),
                                             iim::xyz<size_t>(xcount + xparts[i], ycount + yparts[j], zcount + zparts[k]) ) );
                    zcount += zparts[k];
                }
                ycount += yparts[j];
            }
            xcount += xparts[i];
        }
    }

    // apply scan strategy
    if(strategy == REFILL_RANDOM)
        std::random_shuffle(tiles.begin(), tiles.end());
    else if(strategy == REFILL_ZYX)
        std::sort(tiles.begin(), tiles.end());
    else if(strategy == REFILL_CENTER)
    {
        tf::xyz<float> VOI_center(VOI.start.x + (VOI.end.x-VOI.start.x)/2.0f, VOI.start.y + (VOI.end.y-VOI.start.y)/2.0f, VOI.start.z + (VOI.end.z-VOI.start.z)/2.0f);
        iim::xyz<size_t> VOI_center_mapped(
                    CImageUtils::mapCoord<float>(VOI_center.x, _cachePyramid[cache_level]->dims().x, _vol->getDIM_H(), true),
                    CImageUtils::mapCoord<float>(VOI_center.y, _cachePyramid[cache_level]->dims().y, _vol->getDIM_V(), true),
                    CImageUtils::mapCoord<float>(VOI_center.z, _cachePyramid[cache_level]->dims().z, _vol->getDIM_D(), true));
        std::sort(tiles.begin(), tiles.end(), iim::voi3D<size_t>::distanceFromPointFunctor(VOI_center_mapped));
    }

    // make a copy of tiles before changing coordinate system
    std::vector < iim::voi3D<size_t> > tiles_copy = tiles;

//    printf("tiles before mapping\n");
//    for(size_t i=0; i<tiles.size(); i++)
//         printf("[%d,%d) [%d,%d) [%d,%d)\n",
//             tiles[i].start.x, tiles[i].end.x,
//             tiles[i].start.y, tiles[i].end.y,
//             tiles[i].start.z, tiles[i].end.z);
//    printf("\n\n");

    // scale tile coordinates to the selected cache layer coordinate system
    for(size_t i=0; i<tiles.size(); i++)
    {
        tiles[i].start.x = round05(CImageUtils::mapCoord<float>(tiles[i].start.x, _vol->getDIM_H(), _cachePyramid[cache_level]->dims().x, false, true));
        tiles[i].end.x   = round(CImageUtils::mapCoord<float>(tiles[i].end.x,   _vol->getDIM_H(), _cachePyramid[cache_level]->dims().x, false, true));
        tiles[i].start.y = round05(CImageUtils::mapCoord<float>(tiles[i].start.y, _vol->getDIM_V(), _cachePyramid[cache_level]->dims().y, false, true));
        tiles[i].end.y   = round(CImageUtils::mapCoord<float>(tiles[i].end.y,   _vol->getDIM_V(), _cachePyramid[cache_level]->dims().y, false, true));
        tiles[i].start.z = round05(CImageUtils::mapCoord<float>(tiles[i].start.z, _vol->getDIM_D(), _cachePyramid[cache_level]->dims().z, false, true));
        tiles[i].end.z   = round(CImageUtils::mapCoord<float>(tiles[i].end.z,   _vol->getDIM_D(), _cachePyramid[cache_level]->dims().z, false, true));
    }


//    printf("tiles AFTER mapping\n");
//    for(size_t i=0; i<tiles.size(); i++)
//         printf("[%d,%d) [%d,%d) [%d,%d)\n",
//             tiles[i].start.x, tiles[i].end.x,
//             tiles[i].start.y, tiles[i].end.y,
//             tiles[i].start.z, tiles[i].end.z);
//    printf("\n\n");


    // remove tiles that do not intersect with the given VOI (also sync the result with tiles_copy)
    for( std::vector < iim::voi3D<size_t> >::iterator it = tiles.begin(), it_copy = tiles_copy.begin(); it != tiles.end(); )
    {
        if(VOI.intersection(*it).isValid() == false)
        {
            tiles.erase(it);
            tiles_copy.erase(it_copy);
        }
        else
        {
            it++;
            it_copy++;
        }
    }


//    printf("tiles after erasing not intersecting\n");
//    for(size_t i=0; i<tiles.size(); i++)
//        printf("[%d,%d) [%d,%d) [%d,%d)\n",
//               tiles[i].start.x, tiles[i].end.x,
//               tiles[i].start.y, tiles[i].end.y,
//               tiles[i].start.z, tiles[i].end.z);
//    printf("\n\n");


    // take first non-complete tile
    size_t k = 0;
    for(; k < tiles.size(); k++)
    {
        float c = _cachePyramid[cache_level]->completeness(tiles[k]);
        if(c < 1)
        {
//            printf("first noncomplete tile found (%f): [%d,%d) [%d,%d) [%d,%d)\n",
//                   c,
//                   tiles[k].start.x, tiles[k].end.x,
//                   tiles[k].start.y, tiles[k].end.y,
//                   tiles[k].start.z, tiles[k].end.z);
            break;
        }
    }


    // if k is out of bounds, it means all tiles are complete --> return false
    if(k == tiles.size())
        return false;
    // otherwise fetch data from highest-res (layer = 0) using the 'tiles_copy' tiles with coordinates in the highest-res image space
    else
        delete this->loadVOI(tf::xyz<size_t>(tiles_copy[k].start.x, tiles_copy[k].start.y, tiles_copy[k].start.z),
                             tf::xyz<size_t>(tiles_copy[k].end.x,   tiles_copy[k].end.y,   tiles_copy[k].end.z), 0).data;

//    float cnew = _cachePyramid[cache_level]->completeness(tiles[k]);
//    if(cnew != 1)
//        throw tf::RuntimeException(tf::strprintf("in VirtualPyramid::refill(): completeness != 1 (%f) after refill, VOI x=[%d,%d) y=[%d,%d) z=[%d,%d)", cnew, tiles[k].start.x, tiles[k].end.x, tiles[k].start.y, tiles[k].end.y, tiles[k].start.z, tiles[k].end.z));

    return true;
}


// clear cached data
void tf::VirtualPyramid::clear(
        bool ask_to_save,       // user is asked whether to save modified data (if any)
        int layer)              // layer selection (-1 = all layers)
throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // check preconditions
    if(layer != -1 && (layer < 0 || layer >= cachePyramid().size()))
        throw iim::IOException("Cannot clear pyramid cache: invalid layer selection");


    // get modified blocks (if any)
    std::vector <tf::HyperGridCache::CacheBlock*> blocks_modified;
    for(int i=0; i<cachePyramid().size(); i++)
    {
        if(layer == -1 || layer == i)
        {
            std::vector <tf::HyperGridCache::CacheBlock*> modified_i = cachePyramid()[i]->blocksChanged();
            blocks_modified.insert(blocks_modified.end(), modified_i.begin(), modified_i.end());
        }
    }

    // calculate overall image data size
    float GB = 0;
    for(int i=0; i<blocks_modified.size(); i++)
        GB += blocks_modified[i]->memoryMax();


    // save modified blocks
    if(blocks_modified.size())
    {
        int ret = QMessageBox::Yes;
        if(ask_to_save)
        {
            QMessageBox msgBox;
            msgBox.setText("Save image data");
            msgBox.setInformativeText(iim::strprintf("There are about %.2f GB of unsaved image data<br><br>"
                                      "Do you want to save them?", GB).c_str());
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            ret = msgBox.exec();
        }

        if(ret == QMessageBox::Yes)
        {
            QProgressDialog progress("Save virtual pyramid files...", "Cancel", 0, blocks_modified.size());
            progress.setWindowModality(Qt::WindowModal);
            progress.setMinimumDuration(0);

            for (int i = 0; i < blocks_modified.size(); i++)
            {
                progress.setValue(i);
                progress.setLabelText(iim::strprintf("Save virtual pyramid files %d of %d...", i+1, blocks_modified.size()).c_str());

                blocks_modified[i]->save();

                if (progress.wasCanceled())
                    break;
            }
            progress.setValue(blocks_modified.size());
        }
    }

    // clear all blocks
    for(int i=0; i<cachePyramid().size(); i++)
        if(layer == -1 || layer == i)
            _cachePyramid[i]->clear();
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
        int level,                         // pyramid level (0 for the highest-res, the coarser the resolution the higher)
        VirtualPyramid* parent,            // container
        xyz<int> _reduction_factor)         // reduction factor relative to the highest-res image
throw (iim::IOException) : VirtualVolume()
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);


    _level = level;
    _parent = parent;
    _resamplingFactor = _reduction_factor;

    VXL_V = _parent->_vol->getVXL_V()*_resamplingFactor.y;
    VXL_H = _parent->_vol->getVXL_H()*_resamplingFactor.x;
    VXL_D = _parent->_vol->getVXL_D()*_resamplingFactor.z;
    ORG_V = _parent->_vol->getORG_V();
    ORG_H = _parent->_vol->getORG_H();
    ORG_D = _parent->_vol->getORG_D();
    DIM_C = _parent->_vol->getDIM_C();
    BYTESxCHAN = _parent->_vol->getBYTESxCHAN();
    initChannels();
    DIM_T = _parent->_vol->getDIM_T();
    DIM_V = _parent->_vol->getDIM_V()/static_cast<float>(_resamplingFactor.y);
    DIM_H = _parent->_vol->getDIM_H()/static_cast<float>(_resamplingFactor.x);
    DIM_D = _parent->_vol->getDIM_D()/static_cast<float>(_resamplingFactor.z);
}

void tf::VirtualPyramidLayer::initChannels() throw (iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    static bool first_time = true;
    if(first_time)
        _parent->_vol->initChannels();
    else
        first_time = false;
    active = _parent->_vol->getActiveChannels();
    n_active = _parent->_vol->getNACtiveChannels();
}

// VirtualPyramidLayer deconstructor
tf::VirtualPyramidLayer::~VirtualPyramidLayer() throw (iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);
    active = 0;
    n_active = 0;

    // remove layer from parent, if present
    std::vector<tf::VirtualPyramidLayer*>::iterator position = std::find(_parent->_virtualPyramid.begin(), _parent->_virtualPyramid.end(), this);
    if (position != _parent->_virtualPyramid.end())
        _parent->_virtualPyramid.erase(position);

    // if parent is empty, destroy it
    if(_parent->_virtualPyramid.empty())
        delete _parent;
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
        return _parent->loadVOI(xyz<size_t>(H0,V0,D0), xyz<size_t>(H1,V1,D1), _level).data;
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

// constructor 1
tf::HyperGridCache::HyperGridCache(
        std::string path,                              // where cache files are stored / have to be stored
        xyzct<size_t> image_dim,                       // image dimensions along X, Y, Z, C (channel), and T (time)
        xyzct<size_t> block_dim,					// hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
        const std::string block_fmt)                   // hypergrid block file format
throw (iim::IOException, tf::RuntimeException, iom::exception)
{
    /**/tf::debug(tf::LEV2, iim::strprintf("_path = \"%s\", image_dim = {%d, %d, %d, %d, %d}, block_dim = {%d, %d, %d, %d, %d}, block_fmt = \"%s\"",
                                           _path.c_str(), image_dim.x, image_dim.y, image_dim.z, image_dim.c, image_dim.t, block_dim.x, block_dim.y, block_dim.z, block_dim.c, block_dim.t, block_fmt.c_str()).c_str(), __itm__current__function__);

    // set object members
    _path = path;
    _dims = image_dim;
    _block_dim = block_dim;
    _hypergrid = 0;
    _block_fmt = block_fmt;

    // adjust block dim if needed
    if(_block_dim.x == std::numeric_limits<size_t>::max())
        _block_dim.x = _dims.x;
    if(_block_dim.y == std::numeric_limits<size_t>::max())
        _block_dim.y = _dims.y;
    if(_block_dim.z == std::numeric_limits<size_t>::max())
        _block_dim.z = _dims.z;
    if(_block_dim.c == std::numeric_limits<size_t>::max())
        _block_dim.c = _dims.c;
    if(_block_dim.t == std::numeric_limits<size_t>::max())
        _block_dim.t = _dims.t;

    // check preconditions
    if(_dims.c > 3)
        throw iim::IOException("HyperGridCache does not support > 3 channels yet");
    if(_dims.t > 1)
        throw iim::IOException("HyperGridCache does not support 5D (xyz + channel + time) data yet");

    init();
}

// destructor
tf::HyperGridCache::~HyperGridCache() throw (iim::IOException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // save metadata
    save();

    for(size_t t=0; t<_nBlocks.t; t++)
    {
        for(size_t c=0; c<_nBlocks.c; c++)
        {
            for(size_t z=0; z<_nBlocks.z; z++)
            {
                for(size_t y=0; y<_nBlocks.y; y++)
                {
                    for(size_t x=0; x<_nBlocks.x; x++)
                        delete _hypergrid[t][c][z][y][x];
                    delete _hypergrid[t][c][z][y];
                }
                delete _hypergrid[t][c][z];
            }
            delete _hypergrid[t][c];
        }
        delete _hypergrid[t];
    }
    delete _hypergrid;
}

// init persistency files
void tf::HyperGridCache::init() throw (iim::IOException, tf::RuntimeException, iom::exception)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // create (hidden) folder in given path
    std::string directory = _path + "/.hypergridcache";
    if(!iim::check_and_make_dir(directory.c_str()))
        throw iim::IOException(tf::strprintf("Cannot create local folder \"%s\" in your executable folder", directory.c_str()));


    // save or check hypergrid to file
    std::string hypergridFilePath = directory + "/.hypergrid.txt";
    if(iim::isFile(hypergridFilePath))
        load();
    else
    {
        initHyperGrid();
        save();
    }
}

// init 'hypergrid' 5D matrix
void tf::HyperGridCache::initHyperGrid() throw (iim::IOException, tf::RuntimeException)
{
    // check preconditions
    if(! (_dims.x && _dims.y && _dims.z && _dims.c && _dims.t))
        throw iim::IOException(iim::strprintf("Cannot init hyper grid: invalid image dimensions %d x %d x %d x %d x %d", _dims.x, _dims.y, _dims.z, _dims.c, _dims.t));
    if(! (_block_dim.x && _block_dim.y && _block_dim.z && _block_dim.c && _block_dim.t))
        throw iim::IOException(iim::strprintf("Cannot init hyper grid: invalid block dimensions %d x %d x %d x %d x %d", _block_dim.x, _block_dim.y, _block_dim.z, _block_dim.c, _block_dim.t));
    if(_hypergrid)
        throw iim::IOException("Cannot init hyper grid: hyper grid already exists (you should never see this)");

    // generate hypergrid
    std::vector<size_t> nXs = tf::partition(_dims.x, _block_dim.x);
    std::vector<size_t> nYs = tf::partition(_dims.y, _block_dim.y);
    std::vector<size_t> nZs = tf::partition(_dims.z, _block_dim.z);
    std::vector<size_t> nCs = tf::partition(_dims.c, _block_dim.c);
    std::vector<size_t> nTs = tf::partition(_dims.t, _block_dim.t);
    _nBlocks.x = nXs.size();
    _nBlocks.y = nYs.size();
    _nBlocks.z = nZs.size();
    _nBlocks.c = nCs.size();
    _nBlocks.t = nTs.size();
    _hypergrid = new CacheBlock*****[_nBlocks.t];
    for(size_t t=0, t_acc=0; t<_nBlocks.t; t_acc+=nTs[t++])
    {
        _hypergrid[t] = new CacheBlock****[_nBlocks.c];
        for(size_t c=0, c_acc=0; c<_nBlocks.c; c_acc+=nCs[c++])
        {
            _hypergrid[t][c] = new CacheBlock***[_nBlocks.z];
            for(size_t z=0, z_acc=0; z<_nBlocks.z; z_acc+=nZs[z++])
            {
                _hypergrid[t][c][z] = new CacheBlock**[_nBlocks.y];
                for(size_t y=0, y_acc=0; y<_nBlocks.y; y_acc+=nYs[y++])
                {
                    _hypergrid[t][c][z][y] = new CacheBlock*[_nBlocks.x];
                    for(size_t x=0, x_acc=0; x<_nBlocks.x; x_acc+=nXs[x++])
                    {
                        _hypergrid[t][c][z][y][x] = new CacheBlock(this,
                                                                  xyzct<size_t>(x_acc, y_acc, z_acc, c_acc, t_acc),
                                                                  xyzct<size_t>(nXs[x], nYs[y], nZs[z], nCs[c], nTs[t]),
                                                                  xyzct<size_t>(x,y,z,c,t));
                    }
                }
            }
        }
    }
}

// load from disk
void tf::HyperGridCache::load() throw (iim::IOException, tf::RuntimeException, iom::exception)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // load metadata
    std::string hypergridFilePath = _path + "/.hypergridcache/.hypergrid.txt";
    std::ifstream f(hypergridFilePath.c_str());
    if(!f.is_open())
        throw iim::IOException("Cannot open .hypergrid file at \"%s\"", hypergridFilePath.c_str());
    std::string line;
    std::vector <std::string> tokens;
    std::string visits_str, emptycount_str;
    while(std::getline(f, line))
    {
        tokens = tf::parse(line, ":", 2, hypergridFilePath);
        if(tokens[0].compare("image.dimX") == 0)
            _dims.x = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("image.dimY") == 0)
            _dims.y = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("image.dimZ") == 0)
            _dims.z = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("image.dimC") == 0)
            _dims.c = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("image.dimT") == 0)
            _dims.t = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.format") == 0)
            _block_fmt = tf::cls(tokens[1]);
        if(tokens[0].compare("blocks.nX") == 0)
            _nBlocks.x = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.nY") == 0)
            _nBlocks.y = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.nZ") == 0)
            _nBlocks.z = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.nC") == 0)
            _nBlocks.c = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.nT") == 0)
            _nBlocks.t = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.dimX") == 0)
            _block_dim.x = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.dimY") == 0)
            _block_dim.y = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.dimZ") == 0)
            _block_dim.z = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.dimC") == 0)
            _block_dim.c = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.dimT") == 0)
            _block_dim.t = tf::str2num<size_t>(tokens[1]);
        if(tokens[0].compare("blocks.visits") == 0)
            visits_str = tokens[1];
        if(tokens[0].compare("blocks.emptycount") == 0)
            emptycount_str = tokens[1];
    }
    f.close();

    // init hyper grid
    initHyperGrid();

    // parse visits
    if(visits_str.empty())
        throw iim::IOException("Failed to load HyperGrid: visits not found");
    std::vector <std::string> visits;
    tf::parse(visits_str, ";", _nBlocks.x*_nBlocks.y*_nBlocks.z*_nBlocks.c*_nBlocks.t+1, hypergridFilePath, visits);
    size_t counter = 0;
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        _hypergrid[t][c][z][y][x]->setVisits(tf::str2num<int>(visits[counter++]));

    // parse empty counts
    if(!emptycount_str.empty())
    {
        std::vector <std::string> emptycounts;
        tf::parse(emptycount_str, ";", _nBlocks.x*_nBlocks.y*_nBlocks.z*_nBlocks.c*_nBlocks.t+1, hypergridFilePath, emptycounts);
        size_t counter = 0;
        for(size_t t=0; t<_nBlocks.t; t++)
            for(size_t c=0; c<_nBlocks.c; c++)
                for(size_t z=0; z<_nBlocks.z; z++)
                    for(size_t y=0; y<_nBlocks.y; y++)
                        for(size_t x=0; x<_nBlocks.x; x++)
                            _hypergrid[t][c][z][y][x]->setEmptyCount(tf::str2num<int>(emptycounts[counter++]));
    }
}

// save to disk
void tf::HyperGridCache::save() throw (iim::IOException, tf::RuntimeException, iom::exception)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // save metadata
    std::string hypergridFilePath = _path + "/.hypergridcache/.hypergrid.txt";
    std::ofstream f(hypergridFilePath.c_str());
    if(!f.is_open())
        throw iim::IOException("Cannot open .hypergrid file at \"%s\"", hypergridFilePath.c_str());
    f << "image.dimX:" << _dims.x << std::endl;
    f << "image.dimY:" << _dims.y << std::endl;
    f << "image.dimZ:" << _dims.z << std::endl;
    f << "image.dimC:" << _dims.c << std::endl;
    f << "image.dimT:" << _dims.t << std::endl;
    f << "blocks.format:" << _block_fmt << std::endl;
    f << "blocks.nX:" << _nBlocks.x << std::endl;
    f << "blocks.nY:" << _nBlocks.y << std::endl;
    f << "blocks.nZ:" << _nBlocks.z << std::endl;
    f << "blocks.nC:" << _nBlocks.c << std::endl;
    f << "blocks.nT:" << _nBlocks.t << std::endl;
    f << "blocks.dimX:" << _block_dim.x << std::endl;
    f << "blocks.dimY:" << _block_dim.y << std::endl;
    f << "blocks.dimZ:" << _block_dim.z << std::endl;
    f << "blocks.dimC:" << _block_dim.c << std::endl;
    f << "blocks.dimT:" << _block_dim.t << std::endl;
    f << "blocks.visits:";
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        f << _hypergrid[t][c][z][y][x]->visits() << ";" ;
    f << std::endl;
    f << "blocks.emptycount:";
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        f << _hypergrid[t][c][z][y][x]->emptyCount() << ";" ;
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
    if(voi.end.x > _dims.x || voi.end.y > _dims.y || voi.end.z > _dims.z || voi.end.t > _dims.t)
        throw iim::IOException(tf::strprintf("Out of range xyzt data selection x=[%d,%d) y=[%d,%d) z=[%d,%d) t=[%d,%d), range is "
                                              "x=[0,%d) y=[0,%d) z=[0,%d) t=[0,%d)",
                                              voi.start.x, voi.end.x, voi.start.y, voi.end.y, voi.start.z, voi.end.z, voi.start.t, voi.end.t, _dims.x, _dims.y, _dims.z, _dims.t), __itm__current__function__);

    // check channel selection
    for(size_t i=0; i<channels.dim; i++)
        if(channels.table[i] >= _dims.c)
            throw iim::IOException(tf::strprintf("Out of range channel selection %d, number of channels is %d",
                                                  channels.table[i], _dims.c), __itm__current__function__);

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
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        if(_hypergrid[t][c][z][y][x]->intersection(voi).size()   > 0 &&
                           _hypergrid[t][c][z][y][x]->intersection(channels).size() > 0)
                        {
                            voi4D<float> iv = _hypergrid[t][c][z][y][x]->intersection(voi);
                            std::vector<unsigned int> ic = _hypergrid[t][c][z][y][x]->intersection(channels);

                            // local VOI = intersecting ROI - origin shift
                            voi4D<int> loc_VOI = iv.rounded();
                            loc_VOI -= _hypergrid[t][c][z][y][x]->origin();

                            voi4D<int> img_VOI = iv.rounded();
                            img_VOI -= voi.start;


//                            printf("readData: hypergrid[%02d][%02d][%02d][%02d][%02d]: block [%d,%d),[%d,%d),[%d,%d),[%d,%d),[%d,%d) intersection WITH [%d,%d),{%s},[%d,%d),[%d,%d),[%d,%d) IS [%.0f,%.0f),{%s},[%.0f,%.0f),[%.0f,%.0f),[%.0f,%.0f)\n",
//                                t,c,z,y,x,
//                                hypergrid[t][c][z][y][x]->getOrigin().t, hypergrid[t][c][z][y][x]->getOrigin().t + hypergrid[t][c][z][y][x]->getDims().t,
//                                hypergrid[t][c][z][y][x]->getOrigin().c, hypergrid[t][c][z][y][x]->getOrigin().c + hypergrid[t][c][z][y][x]->getDims().c,
//                                hypergrid[t][c][z][y][x]->getOrigin().z, hypergrid[t][c][z][y][x]->getOrigin().z + hypergrid[t][c][z][y][x]->getDims().z,
//                                hypergrid[t][c][z][y][x]->getOrigin().y, hypergrid[t][c][z][y][x]->getOrigin().y + hypergrid[t][c][z][y][x]->getDims().y,
//                                hypergrid[t][c][z][y][x]->getOrigin().x, hypergrid[t][c][z][y][x]->getOrigin().x + hypergrid[t][c][z][y][x]->getDims().x,
//                                voi.start.t, voi.end.t, img.chans.toString().c_str(), voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x,
//                                iv.start.t,  iv.end.t,  active_channels<>::toString(ic).c_str(), iv.start.z,  iv.end.z,  iv.start.y,  iv.end.y,  iv.start.x,  iv.end.x);

//                            printf("readData: local VOI is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", loc_VOI.start.t, loc_VOI.end.t, loc_VOI.start.z, loc_VOI.end.z, loc_VOI.start.y, loc_VOI.end.y, loc_VOI.start.x, loc_VOI.end.x);
//                            printf("readData: image VOI is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", img_VOI.start.t, img_VOI.end.t, img_VOI.start.z, img_VOI.end.z, img_VOI.start.y, img_VOI.end.y, img_VOI.start.x, img_VOI.end.x);


                            _hypergrid[t][c][z][y][x]->readData(img, img_VOI, loc_VOI, scaling);
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
    /**/tf::debug(tf::LEV1, strprintf("path = \"%s\", img dims(t,c,z,y,x) = (%d x %d x %d x %d x %d), offset(t,z,y,x) = (%d, %d, %d, %d), scaling(z,y,x) = (%d,%d,%d)",
        _path.c_str(), img.dims.t, img.chans.dim, img.dims.z, img.dims.y, img.dims.x, offset.t, offset.z, offset.y, offset.x, scaling.z, scaling.y, scaling.x).c_str(), __itm__current__function__);

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

//    printf("before scaling, VOI(t,z,t,x) is [%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f)\n", voi.start.t, voi.end.t, voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x);

	// scale voi
	if(upscaling)
        voi *= tf::xyz<float>(scaling);
	else
        voi /= tf::xyz<float>(scaling);

//    printf("after scaling, VOI(t,z,t,x) is [%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f)\n", voi.start.t, voi.end.t, voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x);

	// put data into intersecting blocks
    //printf("put data into intersecting blocks \n");
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
					{
                        voi4D<float>              iv = _hypergrid[t][c][z][y][x]->intersection(voi);
                        std::vector<unsigned int> ic = _hypergrid[t][c][z][y][x]->intersection(img.chans);

                        bool intersects = iv.isValid() > 0 && ic.size() > 0;

//                        printf("hypergrid[%02d][%02d][%02d][%02d][%02d]: block [%d,%d),[%d,%d),[%d,%d),[%d,%d),[%d,%d) %s [%.1f,%.1f),{%s},[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f) IS [%.1f,%.1f),{%s},[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f)\n",
//                            t,c,z,y,x,
//                            _hypergrid[t][c][z][y][x]->origin().t, _hypergrid[t][c][z][y][x]->origin().t + _hypergrid[t][c][z][y][x]->dims().t,
//                            _hypergrid[t][c][z][y][x]->origin().c, _hypergrid[t][c][z][y][x]->origin().c + _hypergrid[t][c][z][y][x]->dims().c,
//                            _hypergrid[t][c][z][y][x]->origin().z, _hypergrid[t][c][z][y][x]->origin().z + _hypergrid[t][c][z][y][x]->dims().z,
//                            _hypergrid[t][c][z][y][x]->origin().y, _hypergrid[t][c][z][y][x]->origin().y + _hypergrid[t][c][z][y][x]->dims().y,
//                            _hypergrid[t][c][z][y][x]->origin().x, _hypergrid[t][c][z][y][x]->origin().x + _hypergrid[t][c][z][y][x]->dims().x,
//                            intersects ? "intersects with" : "DOES NOT intersect with",
//                            voi.start.t, voi.end.t, img.chans.toString().c_str(),             voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x,
//                            iv.start.t,  iv.end.t,  active_channels<>::toString(ic).c_str(), iv.start.z,  iv.end.z,  iv.start.y,  iv.end.y,  iv.start.x,  iv.end.x);


						if(intersects)
						{

                            // local VOI = intersecting ROI - origin shift
                            voi4D<int> loc_VOI = iv.rounded();
                            loc_VOI -= _hypergrid[t][c][z][y][x]->origin();

                            // image VOI = scaling(intersecting ROI)
                            voi4D<float> img_VOIf = iv;
							if(upscaling)
                                img_VOIf /= tf::xyz<float>(scaling);
							else
                                img_VOIf *= tf::xyz<float>(scaling);
//                            printf("after scaling: img_VOIf(t,z,t,x) is [%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f)\n", img_VOIf.start.t, img_VOIf.end.t, img_VOIf.start.z, img_VOIf.end.z, img_VOIf.start.y, img_VOIf.end.y, img_VOIf.start.x, img_VOIf.end.x);
                            img_VOIf -= tf::xyzt<float>(offset);
//                            printf("after shift  : img_VOIf(t,z,t,x) is [%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f),[%.1f,%.1f)\n", img_VOIf.start.t, img_VOIf.end.t, img_VOIf.start.z, img_VOIf.end.z, img_VOIf.start.y, img_VOIf.end.y, img_VOIf.start.x, img_VOIf.end.x);

                            voi4D<int> img_VOI = img_VOIf.rounded();

//                            printf("local VOI(t,z,t,x) is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", loc_VOI.start.t, loc_VOI.end.t, loc_VOI.start.z, loc_VOI.end.z, loc_VOI.start.y, loc_VOI.end.y, loc_VOI.start.x, loc_VOI.end.x);
//                            printf("image VOI(t,z,t,x) is [%d,%d),[%d,%d),[%d,%d),[%d,%d)\n", img_VOI.start.t, img_VOI.end.t, img_VOI.start.z, img_VOI.end.z, img_VOI.start.y, img_VOI.end.y, img_VOI.start.x, img_VOI.end.x);

                            if(img_VOI.isValid() && loc_VOI.isValid())
                                _hypergrid[t][c][z][y][x]->putData(img, img_VOI, loc_VOI, upscaling ? scaling : scaling * (-1));
						}
					}
    //printf("\n");
}


std::vector<tf::HyperGridCache::CacheBlock*> tf::HyperGridCache::blocksChanged()
{
    std::vector<tf::HyperGridCache::CacheBlock*> modified;
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        if(_hypergrid[t][c][z][y][x]->hasChanged())
                            modified.push_back(_hypergrid[t][c][z][y][x]);
    return modified;
}

// clear all data
void tf::HyperGridCache::clear()
{
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        _hypergrid[t][c][z][y][x]->clear();
}

float tf::HyperGridCache::memoryUsed()
{
    float sum = 0;
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        sum += _hypergrid[t][c][z][y][x]->memoryUsed();
    return sum;
}

float tf::HyperGridCache::memoryMax()
{
    float sum = 0;
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        sum += _hypergrid[t][c][z][y][x]->memoryMax();
    return sum;
}

// completeness index between 0 (0% explored) and 1 (100% explored)
// it is calculated by counting 'empty' voxels, i.e. that haven't been fetched yet
float tf::HyperGridCache::completeness(iim::voi3D<> voi, bool force_load_image) throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    // adjust voi if it has not been provided
    if(voi == iim::voi3D<>::biggest())
    {
        voi.end.x = _dims.x;
        voi.end.y = _dims.y;
        voi.end.z = _dims.z;
    }

    // check voi
    if(voi.intersection( iim::voi3D<>( iim::xyz<size_t>(0), iim::xyz<size_t>(_dims.x,_dims.y,_dims.z) ) ).isValid() == false)
        throw tf::RuntimeException(tf::strprintf("in completeness(): VOI x=[%d,%d) y=[%d,%d) z=[%d,%d) is out of bounds x=[0,%d) y=[0,%d) z=[0,%d)",
                                                 voi.start.x, voi.end.x, voi.start.y, voi.end.y, voi.start.z, voi.end.z, _dims.x, _dims.y, _dims.z));

    // sum empty counts among all blocks
    size_t sum_empty = 0, sum_total = 0;
    for(size_t t=0; t<_nBlocks.t; t++)
        for(size_t c=0; c<_nBlocks.c; c++)
            for(size_t z=0; z<_nBlocks.z; z++)
                for(size_t y=0; y<_nBlocks.y; y++)
                    for(size_t x=0; x<_nBlocks.x; x++)
                        if(_hypergrid[t][c][z][y][x]->intersection(voi).isValid())
                        {
                            sum_empty += _hypergrid[t][c][z][y][x]->emptyCountInVOI(voi, force_load_image);
                            sum_total += _hypergrid[t][c][z][y][x]->intersection(voi).size() * _hypergrid[t][c][z][y][x]->dims().c * _hypergrid[t][c][z][y][x]->dims().t;
                        }
    return 1.0f - float(sum_empty)/sum_total;
}


/*---- END HYPER GRID CACHE section ---------------------------------------------------------------------------------------*/






/********************************************
*   HYPER GRID CACHE BLOCK definitions      *
*********************************************
---------------------------------------------------------------------------------------------------------------------------*/
// constructor 1
tf::HyperGridCache::CacheBlock::CacheBlock(
        HyperGridCache* parent,
        xyzct<size_t> origin,          // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
        xyzct<size_t> dims,            // dimensions of the block
        xyzct<size_t> index)           // 5D index in the parent hypergrid
throw (iim::IOException, iom::exception)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    _parent = parent;
    _origin = origin;
    _dims   = dims;
    _imdata = 0;
    _index  = index;
    _visits = 0;
    _emptycount = _dims.size();
    _hasChanged = false;

    _path = _parent->_path + "/" + tf::strprintf("t%s_c%s_z%s_y%s_x%s%s",
                                                             tf::num2str(_index.t).c_str(),
                                                             tf::num2str(_index.c).c_str(),
                                                             tf::num2str(_index.z).c_str(),
                                                             tf::num2str(_index.y).c_str(),
                                                             tf::num2str(_index.x).c_str(),
                                                            _parent->blockFormat().c_str());
}


// destructor
tf::HyperGridCache::CacheBlock::~CacheBlock() throw (iim::IOException, iom::exception)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    if(_imdata)
        delete _imdata;
}

// calculate channel intersection with current block
std::vector<unsigned int> tf::HyperGridCache::CacheBlock::intersection(tf::active_channels<> chans)
{
    /**/tf::debug(tf::LEV_MAX, 0, __itm__current__function__);

    std::vector<unsigned int> chs;
    for(size_t i=0; i<chans.dim; i++)
        if(chans.table[i] >= _origin.c && chans.table[i] < _origin.c + _dims.c)
            chs.push_back(chans.table[i]);
    return chs;
}

// update empty voxel count
void tf::HyperGridCache::CacheBlock::updateEmptyCount()
{
    if(_imdata)
    {
        _emptycount = 0;
        size_t total = _dims.size();
        for(size_t i=0; i<total; i++)
            if(!_imdata[i])
                _emptycount++;
    }
}

// get empty voxel count within the given voi
// precondition 1: _emptycount is up-to-date
size_t tf::HyperGridCache::CacheBlock::emptyCountInVOI(iim::voi3D<> voi , bool force_load_image)
{
    /**/tf::debug(tf::LEV3, strprintf("block(z,y,x) = [%d,%d) [%d,%d) [%d,%d), voi(z,y,x) = [%d,%d) [%d,%d) [%d,%d)",
        _origin.z, _origin.z + _dims.z, _origin.y, _origin.y + _dims.y, _origin.x, _origin.x + _dims.x,
        voi.start.z, voi.end.z, voi.start.y, voi.end.y, voi.start.x, voi.end.x).c_str(), __itm__current__function__);

    // no voi provided --> just return empty count in this block
    if(voi == iim::voi3D<>::biggest())
        return _emptycount;
    else
    {
        // get intersection with current block
        iim::voi3D<> ivoi = this->intersection(voi);

        // empty intersection
        //  --> just return 0
        if(!ivoi.isValid())
            return 0;

        // intersection == current block
        //  --> just return empty count (precondition 1: _emptycount is kept up-to-date) in this block
        else if(ivoi.start.x == _origin.x && ivoi.end.x == _origin.x + _dims.x &&
                ivoi.start.y == _origin.y && ivoi.end.y == _origin.y + _dims.y &&
                ivoi.start.z == _origin.z && ivoi.end.z == _origin.z + _dims.z)
            return _emptycount;

        // intersection is a VOI of this block, and we have the image or we are forced to load the image
        //  --> count empty pixels in the image VOI
        else if(_imdata || force_load_image)
        {
            // load the image if needed
            if(!_imdata && force_load_image)
                this->load();

            // subtract origin --> get local image voi
            ivoi.start.x -= _origin.x;
            ivoi.start.y -= _origin.y;
            ivoi.start.z -= _origin.z;
            ivoi.end.x   -= _origin.x;
            ivoi.end.y   -= _origin.y;
            ivoi.end.z   -= _origin.z;

            size_t empty = 0;
            for(size_t t=0; t<_dims.t; t++)
            {
                size_t stride_t = t * _dims.c * _dims.z * _dims.y * _dims.x;
                for(size_t c=0; c<_dims.c; c++)
                {
                    size_t stride_c = stride_t + c * _dims.z * _dims.y * _dims.x;
                    for(size_t z=ivoi.start.z; z<ivoi.end.z; z++)
                    {
                        size_t stride_z = stride_c + z * _dims.y * _dims.x;
                        for(size_t y=ivoi.start.y; y<ivoi.end.y; y++)
                        {
                            size_t stride_y = stride_z + y * _dims.x;
                            for(size_t x=ivoi.start.x; x<ivoi.end.x; x++)
                                if(!_imdata[stride_y + x])
                                    empty++;
                        }
                    }
                }
            }
            return empty;
        }

        // intersection is a VOI of this block, we don't have neither image or the block file
        //  --> just return intersection size
        else if(!iim::isFile(_path))
            return ivoi.size() * _dims.c * _dims.t;

        // all other cases
        //  --> we make an estimate using the current _emptyCount and the ratio between the block size and the intersection VOI size
        else
        {
            //QMessageBox::information(0, "title", "you should never see this");
            return tf::round(_emptycount * ivoi.size() / double(_dims.x*_dims.y*_dims.z));
        }

    }
}

// load from disk
void tf::HyperGridCache::CacheBlock::load() throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // precondition checks
    if(_dims.c > 3)
        throw iim::IOException("I/O functions with c (channels) > 3 not yet implemented", __itm__current__function__);
    if(_dims.t > 1)
        throw iim::IOException("I/O functions with t (time) > 1 not yet implemented", __itm__current__function__);
    if(!_dims.x || !_dims.y || !_dims.z || !_dims.c ||!_dims.t)
        throw iim::IOException("dims <= 0", __itm__current__function__);

    // allocate memory for data if needed
    if(!_imdata)
        _imdata = new uint8[_dims.x * _dims.y * _dims.z * _dims.c * _dims.t];

    // try to load data from disk
    if(iim::isFile(_path))
    {
        Image4DSimple img;
        img.loadImage(_path.c_str());
        if(!img.valid())
            throw iim::IOException(tf::strprintf("Cannot read image block at \"%s\"", _path.c_str()));
        delete _imdata;
        _imdata = img.getRawData();
        img.setRawDataPointerToNull();
    }
    else // otherwise initialize to perfect black (0: value reserved for empty voxels)
    {
        size_t imdata_size = _dims.size();
        for(size_t i = 0; i<imdata_size; i++)
            _imdata[i] = 0;
    }

    // update empty voxel count
    updateEmptyCount();
}

// save to disk
void tf::HyperGridCache::CacheBlock::save() throw (iim::IOException, iom::exception, tf::RuntimeException)
{
    /**/tf::debug(tf::LEV2, 0, __itm__current__function__);

    // precondition checks
    if(_dims.c > 3)
        throw iim::IOException("I/O functions with c (channels) > 3 not yet implemented", __itm__current__function__);
    if(_dims.t > 1)
        throw iim::IOException("I/O functions with t (time) > 1 not yet implemented", __itm__current__function__);
    if(!_dims.x || !_dims.y || !_dims.z || !_dims.c ||!_dims.t)
        throw iim::IOException("dims <= 0", __itm__current__function__);

    // only save when data has been modified
    if(_imdata && _hasChanged)
    {
        Image4DSimple img;
        img.setDatatype(V3D_UINT8);
        img.setXDim(_dims.x);
        img.setYDim(_dims.y);
        img.setZDim(_dims.z);
        img.setCDim(_dims.c);
        img.setTDim(_dims.y);
        img.setRawDataPointer(_imdata);
        bool saved = img.saveImage(_path.c_str());
        img.setRawDataPointerToNull();
        if(!saved)
            throw iim::IOException(tf::strprintf("Cannot save image block at \"%s\"", _path.c_str()));

        _hasChanged = false;
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
        _path.c_str(),
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
        throw iim::IOException(tf::strprintf("Invalid block VOI [%d,%d) along t-axis", block_voi.start.t, block_voi.end.t));
	if(block_voi.start.z < 0 || block_voi.end.z - block_voi.start.z <= 0)
        throw iim::IOException(tf::strprintf("Invalid block VOI [%d,%d) along z-axis", block_voi.start.z, block_voi.end.z));
	if(block_voi.start.y < 0 || block_voi.end.y - block_voi.start.y <= 0)
        throw iim::IOException(tf::strprintf("Invalid block VOI [%d,%d) along y-axis", block_voi.start.y, block_voi.end.y));
	if(block_voi.start.x < 0 || block_voi.end.x - block_voi.start.x <= 0)
        throw iim::IOException(tf::strprintf("Invalid block VOI [%d,%d) along x-axis", block_voi.start.x, block_voi.end.x));

	// load block if needed
    if(!_imdata)
		load();

	// prepare metadata
	unsigned int src_dims[5], src_offset[5], src_count[5], dst_dims[5], dst_offset[5];
	for(size_t i=0; i<5; i++)
	{
		src_dims[i] = image.getDims()[i];
        dst_dims[i] = _dims[i];
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
    tf::CImageUtils::copyRescaleVOI(image.data, src_dims, src_offset, src_count, _imdata, dst_dims, dst_offset, scaling);

	// set this block as modified
    _hasChanged = true;

    // update empty voxel count
    updateEmptyCount();

    // increment visit counter
    _visits++;
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
        _path.c_str(),
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
    if(!_imdata)
        load();

    // prepare metadata
    unsigned int src_dims[5], src_offset[5], src_count[5], dst_dims[5], dst_offset[5];
    for(size_t i=0; i<5; i++)
    {
        src_dims[i] = _dims[i];
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
    tf::CImageUtils::copyRescaleVOI(_imdata, src_dims, src_offset, src_count, image.data, dst_dims, dst_offset, scaling);

    // increment visit counter
    _visits++;
}

/*---- END HYPER GRID CACHE BLOCK section --------------------------------------------------------------------------------*/
