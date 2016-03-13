#ifndef VIRTUALPYRAMID_H
#define VIRTUALPYRAMID_H

#include "CPlugin.h"
#include "VirtualVolume.h"


class teramanager::VirtualPyramid
{
    private:

        // object members
        iim::VirtualVolume*                 highresVol;     // highest-res (unconverted) volume
        std::string                         highresPath;    // highest-res (unconverted) volume path
        std::vector< itm::VirtualPyramidLayer* > layers;    // virtual (mostly empty) pyramid layers, from lowest-to-highest res

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
        std::vector<iim::VirtualVolume*> getLayers(){
            return std::vector<iim::VirtualVolume*>(layers.begin(), layers.end());
        }
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


class teramanager::VirtualPyramidLayer : public :: iim::VirtualVolume
{
    private:

        // object members
        iim::VirtualVolume*     highresVol;         // highest-res (unconverted) volume
        xyz<int>                reductionFactor;    // reduction factor relative to highresVol
        VirtualPyramid*         pyramid;            // container

        // disable default constructor
        VirtualPyramidLayer(){}

    public:

        // constructor
        VirtualPyramidLayer(iim::VirtualVolume* _highresVol, xyz<int> _reduction_factor, VirtualPyramid* _pyramid);

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




#endif // VIRTUALPYRAMID_H
