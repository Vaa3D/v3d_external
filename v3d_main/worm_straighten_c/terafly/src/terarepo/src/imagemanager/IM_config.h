//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
* 2019-10-24. Giulio.     @ADDED 'ComposedVolume' to the list of volume classes 
* 2019-10-22. Giulio      @ADDED VirtualVolume ID: MULTICHANNEL_FORMAT
* 2019-10-16. Giulio      @ADDED VirtualVolume ID: MULTISLICE_FORMAT
* 2018-07-05. Giulio.     @ADDED IDs for remapping algorithms from 8 bits values to rescaled 8 bits values
* 2018-06-30. Giulio.     @ADDED IDs for conversion algorithms from arbitrary depth to 8 bits
* 2017-04-01. Giulio.     @ADDED ID for 'volatile' format (not implemented yet)
* 2015-05-11. Giulio.     @ADDED 'Mapped Format' to the list of volume formats 
* 2015-03-17. Giulio.     @CHANGED includes of standard header files (stdlib.h and stdio.h) moved outside the directive #ifdef _WIN32 
* 2015-02-18. Giulio.     @ADDED Identifier for unstitched volume
*/


#ifndef _IM_CONFIG_H
#define _IM_CONFIG_H

#include <string>
#include <cstdarg>
#include <vector>
#include <sstream>
#include <limits>
#include <cstring>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <ctime>
#include <direct.h>
#else
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#endif

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

//#ifdef __MACH__
//#include <mach/clock.h>
//#include <mach/mach.h>
//#endif

typedef struct {int V0, V1, H0, H1;} Rect_t; // used by iim::Stack and iim::Block

namespace IconImageManager
{

    /*******************
    *    INTERFACES    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
	class Stack;
	class Block;
	class VirtualVolume;
	class ComposedVolume;
	class StackedVolume;
	class TiledVolume;
	class TiledMCVolume;
	class VirtualFmtMngr;
	class Tiff3DFmtMngr;
	class Vaa3DRawFmtMngr;
	class CacheBuffer;
    /*-------------------------------------------------------------------------------------------------------------------------*/

	
	/*******************
    *       TYPES      *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    typedef signed char	sint8;					//8-bit  signed   integers (-128                       -> +127)
    typedef short sint16;						//16-bit signed   integers (-32,768                    -> +32,767)
    typedef int sint32;							//32-bit signed   integers (-2,147,483,648             -> +2,147,483,647)
    typedef long long sint64;					//64-bit signed   integers (9,223,372,036,854,775,808 -> +9,223,372,036,854,775,807)
    typedef unsigned char uint8;				//8-bit  unsigned integers (0 -> +255)
    typedef unsigned short int uint16;			//16-bit unsigned integers (0 -> +65,535)
    typedef unsigned int uint32;				//32-bit unsigned integers (0 -> +4,294,967,295)
    typedef unsigned long long uint64;			//64-bit unsigned integers (0 -> +18,446,744,073,709,551,615
    typedef float real32;						//real single precision
    typedef double real64;						//real double precision
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const std::string MDATA_BIN_FILE_NAME  = "mdata.bin";       // name of binary metadata file
    const int         MDATA_BIN_FILE_VERSION = 2;               // version of binary metadata file
    const std::string MC_MDATA_BIN_FILE_NAME = "cmap.bin";      // name of binary metadata file for multichannel volumes
    const std::string FORMAT_MDATA_FILE_NAME = ".iim.format";   // name of format metadata file
    const std::string CHANNEL_PREFIX = "CH_";                   // prefix identifying a folder containing data of a certain channel
    const std::string TIME_FRAME_PREFIX = "T_";                 // prefix identifying a folder containing data of a certain time frame
    const int         DEF_IMG_DEPTH = 8;                        // default image depth
    const int         NUL_IMG_DEPTH = 0;                        // invalid image depth
    const int         NATIVE_RTYPE  = 0;                        // loadVolume returns the same bytes per channel as in the input image
    const std::string DEF_IMG_FORMAT = "tif";                   // default image format
    const int         STATIC_STRINGS_SIZE = 1024;               // size of static C-strings
    const std::string RAW_FORMAT            = "Vaa3D raw";                  // unique ID for the RawVolume class
    const std::string SIMPLE_RAW_FORMAT     = "Vaa3D raw (series, 2D)";     // unique ID for the SimpleVolumeRaw class
    const std::string STACKED_RAW_FORMAT    = "Vaa3D raw (tiled, 2D)";      // unique ID for the StackedVolume class
    const std::string TILED_FORMAT          = "Vaa3D raw (tiled, 3D)";      // unique ID for the TiledVolume class
    const std::string TILED_MC_FORMAT       = "Vaa3D raw (tiled, 4D)";      // unique ID for the TiledMCVolume class
    const std::string TIF3D_FORMAT          = "TIFF (3D)";                  // unique ID for multipage TIFF format (nontiled)
    const std::string SIMPLE_FORMAT         = "TIFF (series, 2D)";          // unique ID for the SimpleVolume class
    const std::string STACKED_FORMAT        = "TIFF (tiled, 2D)";           // unique ID for the StackedVolume class
    const std::string TILED_TIF3D_FORMAT    = "TIFF (tiled, 3D)";           // unique ID for multipage TIFF format (tiled)
    const std::string TILED_MC_TIF3D_FORMAT = "TIFF (tiled, 4D)";           // unique ID for multipage TIFF format (nontiled, 4D)
    const std::string UNST_TIF3D_FORMAT     = "TIFF (unstitched, 3D)";      // unique ID for multipage TIFF format (nontiled, 4D)
    const std::string BDV_HDF5_FORMAT       = "HDF5 (BigDataViewer)";       // unique ID for BDV HDF5
    const std::string IMS_HDF5_FORMAT       = "HDF5 (Imaris IMS)";          // unique ID for IMS HDF5
    const std::string MAPPED_FORMAT         = "Mapped Volume";              // unique ID for mapped volumes
    const std::string MULTISLICE_FORMAT     = "MultiSlice Volume";          // unique ID for multi-slice volumes
    const std::string MULTICYCLE_FORMAT     = "MultiCycle Volume";			// unique ID for multi-cycle volumes
    const std::string VOLATILE_FORMAT       = "Volatile Volume";            // unique ID for volatile volumes
    const std::string TIME_SERIES           = "Time series";                // unique ID for the TimeSeries class

    const double      PI = 3.14159265;                          // pi
    const int         TMITREE_MAX_HEIGHT  = 10;                 // maximum depth of the TMITREE
    const int         TMITREE_MIN_BLOCK_DIM = 250;              // minimum dimension of TMITREE block along X/Y/Z
    /*-------------------------------------------------------------------------------------------------------------------------*/

    /***************************
    * TRANSFORM ALGORITHMS IDs *
    ****************************
    ---------------------------------------------------------------------------------------------------------------------------*/
	/* ID of remap algorithms from 8 bits to 8 bits */
	const int REMAP_NULL                 = 0x0000;                   // null map (it ia also not reconized as an actually requested map)
	const int REMAP_6_TO_8_BITS          = 0x0001;                   // multply by 4 the voxel value and set to 255 all voxels with value >= 64
	const int REMAP_LOCAL_MAX            = 0x0002;                   // look for maximum values in each channel and rescale each channel separately
	
	const int N_REMAP_ALGORITHMS    = 3;	
	extern const char *remap_algorithms_strings[];
	extern const int remap_algorithms_IDs[];
	
	const int REMAP_8_BITS_DEFAULT   = REMAP_NULL;                   // set this constant to define the default remap algorithm

	const int MASK_REMAP_ALGORITHM       = 0x00ff;                   // remap algorithms have bits 0-7 not zero
	
	/* ID of conversion algorithms from 16 to 8 bits */
	const int DEPTH_CONVERSION_LINEAR    = 0x0100;                   // convert linearly from [0,2^bitdepth-1] to [0,2^8-1]
	const int DEPTH_CONVERSION_LOCAL_MAX = 0x0200;                   // look for maximum values in each channel and rescale each channel separately
	const int DEPTH_CONVERSION_4_11      = 0x0300;                   // maintain bits 4-11, set to 255 values larger than 4095
	
	const int N_CONVERSION_ALGORITHMS    = 3;	
	extern const char *conversion_algorithms_strings[];
	extern const int conversion_algorithms_IDs[];
	
	const int DEPTH_CONVERSION_DEFAULT   = DEPTH_CONVERSION_LOCAL_MAX; // set this constant to define the default conversion algorithm

	const int MASK_CONVERSION_ALGORITHM  = 0xff00;                     // conversion algorithms have only bits 8-15 not zero
	                                                                   // this mask can be used to extract the conversion algorithm ID 
	                                                                   // from a mixed remap/conversion algorithm
	                                                                   
	/* ID of mixed remap/conversion algorithms (can be applied to both conversion form 18 to 8 bits and rempa from 8 to 8 bits */
	const int REMAP_DEPTH_COVERSION_LOCAL_MAX = DEPTH_CONVERSION_LOCAL_MAX | REMAP_LOCAL_MAX;
	
	const int N_MIXED_ALGORITHMS    = 1;	
	extern const char *mixed_algorithms_strings[];
	extern const int mixed_algorithms_IDs[];
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    PARAMETERS    *
    ******************** - default values have to be set in IM_config.cpp.
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern int DEBUG;                                           // debug level of current module
    extern bool DEBUG_TO_FILE;                                  // whether debug messages should be printed on the screen or to a file (default: screen)
    extern std::string DEBUG_FILE_PATH;                         // filepath where to save debug information
    extern bool ADD_NOISE_TO_TIME_SERIES;                       // whether to mark individual frames of a time series with increasing gaussian noise
    extern int CHANNEL_SELECTION;								// channel to be used when image must be converted to an intensity image (default is ALL)
    extern std::string VERSION;                                 // version of current module
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *       ENUMS      *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    enum  axis        { vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0};
    enum  dimension   { dimension_x, dimension_y, dimension_z, dimension_channel, dimension_time};
    enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };
    enum  dimension_channel { ALL, R, G, B };
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *      STRUCTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    struct VHD_triple{int V, H, D;};
    struct interval_t{
        int start, end;
        interval_t(void) :				   start(-1),	  end(-1)  {}
        interval_t(int _start, int _end) : start(_start), end(_end){}
    };
    struct ref_sys{
        axis first, second, third;
        ref_sys(axis _first, axis _second, axis _third) : first(_first), second(_second), third(_third){}
        ref_sys(const ref_sys &_rvalue) : first(_rvalue.first), second(_rvalue.second), third(_rvalue.third){}
        ref_sys(): first(axis_invalid), second(axis_invalid), third(axis_invalid){}
    };
    // triplet (x + y + z)
    template<class T>
    struct xyz
    {
        T x,y,z;
        xyz(void) : x(0), y(0), z(0){}
        xyz(T _all) : x(_all), y(_all), z(_all){}
        xyz(T _x, T _y, T _z) : x(_x), y(_y), z(_z){}

        bool operator <  (const xyz<T> &p) const{
            return p.x < x && p.y < y && p.z < z;
        }

        bool operator == (const xyz<T> &p) const{
            return p.x == x && p.y == y && p.z == z;
        }
    };
    // 3D Volume Of Interest
    template<class T = size_t>
    struct voi3D
    {
        xyz<T> start;       // range [start, end)
        xyz<T> end;         // range [start, end)
        voi3D() : start(0,0,0,0), end(0,0,0,0){}
        voi3D(xyz<T> _start, xyz<T> _end) : start(_start), end(_end){}
        xyz<size_t> dims() const{
            return xyz<size_t>(
                        size_t(end.x > start.x ? end.x-start.x : 0),
                        size_t(end.y > start.y ? end.y-start.y : 0),
                        size_t(end.z > start.z ? end.z-start.z : 0));
        }
        size_t size() const{
            xyz<size_t> _dims=dims(); return _dims.x*_dims.y*_dims.z;
        }

        xyz<T> center() const{
            return xyz<T> ( (end.x-start.x) / 2 + start.x, (end.y-start.y) / 2 + start.y, (end.z-start.z) / 2 + start.z);
        }

        T distanceFromPoint(const xyz<T> & p) const{
            xyz<T> c = center();
            return std::sqrt( double((p.x-c.x)*(p.x-c.x) + (p.y-c.y)*(p.y-c.y) + (p.z-c.z)*(p.z-c.z)) );
        }

        bool operator <  (const voi3D<T> &p) const{
            xyz<T> center_a = center(), center_b = p.center();
            return  center_a.z <  center_b.z ||
                   (center_a.z == center_b.z && center_a.y <  center_b.y) ||
                   (center_a.z == center_b.z && center_a.y == center_b.y && center_a.x < center_b.x) ;
        }

        bool operator == (const voi3D<T> &p) const{
            return start == p.start && end == p.end;
        }

        bool isValid(){
            return end.x > start.x && end.y > start.y && end.z > start.z;
        }

        voi3D<T> intersection(const voi3D<T> & voi){
            return voi3D<T> (
                        xyz<T>( std::max(voi.start.x, start.x),
                                std::max(voi.start.y, start.y),
                                std::max(voi.start.z, start.z)),
                        xyz<T>( std::min(voi.end.x,   end.x),
                                std::min(voi.end.y,   end.y),
                                std::min(voi.end.z,   end.z))
                        );
        }

        static voi3D<T> biggest(){
            return voi3D<T> (
                        xyz<T>(0),
                        xyz<T>( std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max()) );
        }

        struct distanceFromPointFunctor
        {
            distanceFromPointFunctor(const xyz<T>& _p) : p(_p) {}

            bool operator()(const voi3D<T>& lhs, const voi3D<T>& rhs) const{
                return lhs.distanceFromPoint(p) < rhs.distanceFromPoint(p);
            }

            private:
                xyz<T> p;
        };
    };


    /********************************************
     * Cross-platform UTILITY inline functions	*
     ********************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    // round functions
    inline int round(float  x) { return static_cast<int>(x > 0.0f ? x + 0.5f : x - 0.5f);}
    inline int round(double x) { return static_cast<int>(x > 0.0  ? x + 0.5  : x - 0.5 );}

    // sign function
    template <typename T> int sgn ( T x ){ return x < 0 ? -1 : 1;}

    // integer pow
    inline int powInt(int base, int exp){ return static_cast<int>( pow(static_cast<float>(base), exp)); }

    // string-based sprintf function
    inline std::string strprintf(const std::string fmt, ...){
        int size = 100;
        std::string str;
        va_list ap;
        while (1) {
            str.resize(size);
            va_start(ap, fmt);
            int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
            va_end(ap);
            if (n > -1 && n < size) {
                str.resize(n);
                return str;
            }
            if (n > -1)
                size = n + 1;
            else
                size *= 2;
        }
        return str;
    }

    //returns true if the given path is a directory
    inline bool isDirectory(std::string path){
#ifdef _WIN32
        struct _stat64 s;
        if( _stat64(path.c_str(),&s) == 0 )
#else
		struct stat s;
		if( stat(path.c_str(),&s) == 0 )
#endif
        {
            if( s.st_mode & S_IFDIR )
                return true;
            else if( s.st_mode & S_IFREG )
                return false;
            else return false;
        }
        else return false;
    }

    //returns true if the given path is a file
    inline bool isFile(std::string path){
#ifdef _WIN32
		struct _stat64 s;
		if( _stat64(path.c_str(),&s) == 0 )
#else
		struct stat s;
		if( stat(path.c_str(),&s) == 0 )
#endif
        {
            if( s.st_mode & S_IFDIR )
                return false;
            else if( s.st_mode & S_IFREG )
                return true;
            else return false;
        }
        else return false;
    }

    //returns true if the given string <fullString> ends with <ending>
    inline bool hasEnding (std::string const &fullString, std::string const &ending){
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    //returns file extension, if any (otherwise returns "")
    inline std::string getFileExtension(const std::string& FileName){
        if(FileName.find_last_of(".") != std::string::npos)
            return FileName.substr(FileName.find_last_of(".")+1);
        return "";
    }

    //number to string conversion function and vice versa
    template <typename T>
    std::string num2str ( T Number ){
        std::stringstream ss;
        ss << Number;
        return ss.str();
    }
    template <typename T>
    T str2num ( const std::string &Text ){
        std::stringstream ss(Text);
        T result;
        return ss >> result ? result : 0;
    }

    // functions to swap 2-bytes and 4-bytes words
    inline void swap2bytes(void *targetp)
    {
        unsigned char * tp = (unsigned char *)targetp;
        unsigned char a = *tp;
        *tp = *(tp+1);
        *(tp+1) = a;
    }
    inline void swap4bytes(void *targetp)
    {
        unsigned char * tp = (unsigned char *)targetp;
        unsigned char a = *tp;
        *tp = *(tp+3);
        *(tp+3) = a;
        a = *(tp+1);
        *(tp+1) = *(tp+2);
        *(tp+2) = a;
    }

    //time computation
//    #ifdef _WIN32
//    inline double getTimeSeconds(){
//        return static_cast<double>(clock()) / CLOCKS_PER_SEC;
//    }
//    #else
//        #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
//        inline double getTimeSeconds()
//        {
//            clock_serv_t cclock;
//            mach_timespec_t mts;
//            host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
//            clock_get_time(cclock, &mts);
//            mach_port_deallocate(mach_task_self(), cclock);
//            return mts.tv_sec;
//        }
//        #else
//        inline double getTimeSeconds(){
//            timespec event;
//            clock_gettime(CLOCK_REALTIME, &event);
//            return (event.tv_sec*1000.0 + event.tv_nsec/1000000.0)/1000.0;
//        }
//        #endif
//    #endif


    //make dir
    #ifdef _WIN32
    #include <errno.h>
    inline bool makeDir(const char* arg){
//        printf("Creating directory \"%s\" ...", arg);
        bool done = _mkdir(arg) == 0;
        bool result = done || errno != ENOENT;
//        printf("%s\n", result? "DONE!" : "ERROR!");
        return result;
    }
    #else
    inline bool makeDir(const char* arg){
//        printf("Creating directory \"%s\" ...", arg);
        bool done = mkdir(arg, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
        bool result = done || errno == EEXIST;
//        printf("%s\n", result? "DONE!" : "ERROR!");
        return result;
    }
    #endif

     //remove dir
    #ifdef _WIN32
    #include <errno.h>
    inline bool remove_dir(const char* arg){
        bool done = _rmdir(arg) == 0;
        bool result = done;
        return result;
    }
    #else
    inline bool remove_dir(const char* arg){
        //bool done = rmdir(arg) == 0;
        //bool result = done;
        //return result;
        if(system(strprintf("rmdir \"%s\"", arg).c_str())!=0) {
            fprintf(stderr,"Can't remove directory \"%s\"\n", arg);
            return 0;
        }
        else
        	return 1;
    }
    #endif

	// check-and-makedir
    inline bool check_and_make_dir(const char *dirname){
        if(isDirectory(dirname))
            return true;
        else
            return makeDir(dirname);
    }



    //file deleting
    #ifdef _WIN32
    inline void delete_file( const char* arg ){
        if(system(strprintf("del /F /Q /S \"%s\"", arg).c_str())!=0)
            fprintf(stderr,"Can't delete file \"%s\"\n", arg);
    }
    #else
    inline void delete_file( const char* arg ){
        if(system(strprintf("rm -f \"%s\"", arg).c_str())!=0)
            fprintf(stderr,"Can't delete file \"%s\"\n", arg);
    }
    #endif

    //cross-platform current function macro
    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
    # define __iim__current__function__ __PRETTY_FUNCTION__
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
    # define __iim__current__function__ __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__)
    # define __iim__current__function__ __FUNCSIG__
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    # define __iim__current__function__ __FUNCTION__
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    # define __iim__current__function__ __FUNC__
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    # define __iim__current__function__ __func__
    #else
    # define __iim__current__function__ "(unknown)"
    #endif


    inline const char* axis_to_str(axis ax){
        if(ax == 1)
            return "Vertical";
        else if(ax == -1)
            return "Inverse Vertical";
        else if(ax == 2)
            return "Horizontal";
        else if(ax == -2)
            return "Inverse Horizontal";
        else if(ax == 3)
            return "Depth";
        else if(ax == -3)
            return "Inverse Depth";
        else return "<unknown>";
    }
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /***********************************************
    *    DEBUG, WARNING and EXCEPTION FUNCTIONS    *
    ************************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    inline void warning(const char* message, const char* source = 0)
    {
        if(DEBUG_TO_FILE)
        {
            FILE* f = fopen(DEBUG_FILE_PATH.c_str(), "a");
            if(source)
                fprintf(f, "\n**** WARNING (source: \"%s\") ****\n"
                "    |=> \"%s\"\n\n", source, message);
            else
                fprintf(f, "\n**** WARNING ****: %s\n", message);
            fclose(f);
        }
        else
        {
            if(source)
                printf("\n**** WARNING (source: \"%s\") ****\n"
                "    |=> \"%s\"\n\n", source, message);
            else
                printf("\n**** WARNING ****: %s\n", message);
        }
    }

    inline void debug(debug_level dbg_level, const char* message=0, const char* source=0)
    {
        if(DEBUG >= dbg_level){
            if(DEBUG_TO_FILE)
            {
                FILE* f = fopen(DEBUG_FILE_PATH.c_str(), "a");
                if(message && source)
                    fprintf(f, "\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\") ----\n"
                             "                      message: %s\n\n", dbg_level, source, message);
                else if(message)
                    fprintf(f, "\n--------------------- IconImageManager module: DEBUG (level %d) ----: %s\n", dbg_level, message);
                else if(source)
                    fprintf(f, "\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
                fclose(f);
            }
            else
            {
                if(message && source)
                    printf("\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\") ----\n"
                             "                      message: %s\n\n", dbg_level, source, message);
                else if(message)
                    printf("\n--------------------- IconImageManager module: DEBUG (level %d) ----: %s\n", dbg_level, message);
                else if(source)
                    printf("\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
            }
        }
    }


    class IOException
    {
        private:

            std::string source;
            std::string message;
            IOException(void);

        public:

            IOException(std::string _message, std::string _source = "unknown"){
                source = _source; message = _message;}
            ~IOException(void){}
            const char* what() const {return message.c_str();}
            const char* getSource() const {return source.c_str();}
    };
}
namespace iim = IconImageManager;	//a short alias for the current namespace

#endif //_IM_CONFIG_H


//time computation
#include <ctime>
#ifdef _WIN32
    #define TIME( arg ) (((double) clock()) / CLOCKS_PER_SEC)
    #define system_PAUSE() 		\
            system("PAUSE"); 		\
            cout<<endl;
    #define system_CLEAR() system("cls");
#else
    #define TIME( arg ) (time( arg ))
    #define system_CLEAR() system("clear");
#endif
