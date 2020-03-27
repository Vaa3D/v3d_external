#ifndef CANNOTATIONS_H
#define CANNOTATIONS_H

#include "CViewer.h"

#include <set>
#include "v3d_interface.h"
#include "CPlugin.h"
#include "math.h"
//#include "CViewer.h"

//annotation structure
struct terafly::annotation
{
    long long ID;                   //unique identifier
    int type;                       //-1 = undefined, 0 = LocationSimple, 1 = NeuronSWC
    int subtype;                    //see Vaa3D LocationSimple and NeuronSWC types
    float x, y, z;                  //point coordinates
    float r;                        //radius
    std::string name;               //name
    std::string comment;            //comment
    RGBA8 color;                    //color
    annotation* parent;             //parent node pointer (used in case of linked structures)
    std::set<annotation*> children; //children nodes pointers (used in case of linked structures)
    void* container;                //address of the container object
    int vaa3d_n;                    //Vaa3D's swc/apo index
    bool smart_delete;              // = true by default, enables "smart" deletion (see decontructor code)
    long level;
    int creatmode;                  //for timestamping and quality control LMG 8/10/2018
    double timestamp;               //for timestamping and quality control LMG 8/10/2018
    int tfresindex;                 //for keeping TeraFly resolution index LMG 13/12/2018
    // constructor and deconstructor
    annotation() throw (tf::RuntimeException);
    annotation(const CellAPO & c)
    {
        x = c.x;
        y = c.y;
        z = c.z;
        color = c.color;
        name = c.name.toStdString();
        comment = c.comment.toStdString();
        r = pow(c.volsize*(3.0/4), 1.0/3);
        type = 0;
        subtype = 0;
    }

    ~annotation();

    inline bool operator==(const annotation& r) const{
        return  x       == r.x &&
                y       == r.y &&
                z       == r.z &&
                type    == r.type &&
                subtype == r.subtype;
    }

    inline CellAPO toCellAPO() const{
        CellAPO c;
        c.x = x;
        c.y = y;
        c.z = z;
        c.color = color;
        c.name = name.c_str();
        c.comment = comment.c_str();
        c.n = ID;
        c.volsize = (4.0/3)*tf::pi*r*r*r;
        return c;
    }

    void ricInsertIntoTree(annotation* node, QList<NeuronSWC> &tree);
    void insertIntoTree(QList<NeuronSWC> &tree);

    static long long last_ID;       //last ID assigned
    static tf::uint64 instantiated;//statistic "instantiated": counts the number of instantiated objects
    static tf::uint64 destroyed;   //statistic "destroyed"   : counts the number of destroyed objects
};

class terafly::CAnnotations
{

    private:

        //octree structure
        class Octree
        {
            private:

                struct octant
                {
                    //VHD intervals that delimit the octant
                    int V_start, V_dim;
                    int H_start, H_dim;
                    int D_start, D_dim;

                    //number of neurons in the octant
                    tf::uint32 n_annotations;

                    //annotations stored in the octant (only in a leaf)
                    std::list<terafly::annotation*> annotations;

                    //pointers to children octants
                    octant *child1;	//[V_start,         V_start+V_dim/2),[H_start,		H_start+H_dim/2),[D_start,		D_start+D_dim/2)
                    octant *child2;	//[V_start,         V_start+V_dim/2),[H_start,		H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
                    octant *child3;	//[V_start,         V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,		D_start+D_dim/2)
                    octant *child4;	//[V_start,         V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
                    octant *child5;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start,		H_start+H_dim/2),[D_start,		D_start+D_dim/2)
                    octant *child6;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start,		H_start+H_dim/2),[D_start+D_dim/2,      D_start+D_dim  )
                    octant *child7;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,		D_start+D_dim/2)
                    octant *child8;	//[V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )

                    //octree container handle
                    Octree* container;

                    octant(tf::uint32 _V_start, tf::uint32 _V_dim, tf::uint32 _H_start, tf::uint32 _H_dim, tf::uint32 _D_start, tf::uint32 _D_dim, Octree* _container)
                    {
                        child1  = child2 = child3 = child4 = child5 = child6 = child7 = child8 = 0;
                        V_start = _V_start;
                        V_dim   = _V_dim;
                        H_start = _H_start;
                        H_dim   = _H_dim;
                        D_start = _D_start;
                        D_dim   = _D_dim;
                        n_annotations = 0;
                        container = _container;
                    }
                };
                typedef octant* Poctant;

                tf::uint32 DIM_V, DIM_H, DIM_D;		//volume dimensions (in voxels) along VHD axes
                octant *root;				//pointer to root octant
                Octree(void){}				//default constructor is not available

                /*** SUPPORT methods ***/

                //recursive support methods
                void        _rec_clear(const Poctant& p_octant) throw(tf::RuntimeException);
                void        _rec_insert(const Poctant& p_octant, annotation& neuron) throw(tf::RuntimeException);
                void        _rec_remove(const Poctant& p_octant, annotation* neuron) throw(tf::RuntimeException);
                tf::uint32 _rec_deep_count(const Poctant& p_octant) throw(tf::RuntimeException);
                tf::uint32 _rec_height(const Poctant& p_octant) throw(tf::RuntimeException);
                void        _rec_print(const Poctant& p_octant);
                void        _rec_search(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int, std::list<annotation*>& neurons) throw(tf::RuntimeException);
                Poctant     _rec_find(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(tf::RuntimeException);
                tf::uint32 _rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(tf::RuntimeException);
                void        _rec_prune(const Poctant& p_octant) throw(tf::RuntimeException);
                void        _rec_to_neuron_tree(const Poctant& p_octant, QList<NeuronSWC> &segments) throw(tf::RuntimeException);

                //returns true if two given volumes intersect each other
                bool inline intersects(const interval_t& V1_int,const interval_t& H1_int,const interval_t& D1_int,
                                       int& V2_start, int& V2_dim, int& H2_start, int& H2_dim, int& D2_start, int& D2_dim)  throw(tf::RuntimeException);

                //returns true if first volume contains second volume
                bool inline contains  (const interval_t& V1_int, const interval_t& H1_int, const interval_t& D1_int,
                                       int& V2_start, int& V2_dim, int& H2_start, int& H2_dim, int& D2_start, int& D2_dim)  throw(tf::RuntimeException);

            public:

                //CONSTRUCTOR and DECONSTRUCTOR
                Octree(tf::uint32 _DIM_V, tf::uint32 _DIM_H, tf::uint32 _DIM_D);
                ~Octree(void);

                //clears octree content and deallocates used memory
                void clear() throw(tf::RuntimeException);

                //insert given neuron in the octree
                void insert(annotation& neuron) throw(tf::RuntimeException);

                //remove given neuron from the octree (returns 1 if succeeds)
                bool remove(annotation* neuron) throw(tf::RuntimeException);

                //search for neurons in the given 3D volume and puts found neurons into 'neurons'
                void find(interval_t V_int, interval_t H_int, interval_t D_int, std::list<annotation*>& neurons) throw(tf::RuntimeException);

                //search for the annotations at the given coordinate. If found, returns the address of the annotations list
                std::list<annotation*>* find(float x, float y, float z) throw(tf::RuntimeException);

                //returns the number of neurons (=leafs) in the given volume without exploring the entire data structure
                tf::uint32 count(interval_t V_int = interval_t(-1,-1), interval_t H_int = interval_t(-1,-1), interval_t D_int = interval_t(-1,-1))  throw(tf::RuntimeException);

                //returns the number of neurons (=leafs) in the octree by exploring the entire data structure
                tf::uint32 deep_count()  throw(tf::RuntimeException);

                //returns the octree height
                tf::uint32 height()  throw(tf::RuntimeException);

                //prunes the octree by removing all nodes duplicates while maintaining the same branched structure
                void prune() throw(tf::RuntimeException);

                //print the octree content
                void print();

                /*********************************************************************************
                * Converts the octree to a NeuronTree. This is actually a draw method.
                **********************************************************************************/
                NeuronTree toNeuronTree() throw (tf::RuntimeException);

                friend class CAnnotations;
                friend struct annotation;

                static inline double round(double val){
                    return floor(val + 0.5);
                }
                static inline double round(float val){
                    return floor(val + 0.5f);
                }
        };


        //members
        Octree* octree;         //octree associated to the 3D image space where annotations are stored
        int octreeDimX;
        int octreeDimY;
        int octreeDimZ;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        CAnnotations() : octree(0), octreeDimX(-1), octreeDimY(-1), octreeDimZ(-1){}
        static CAnnotations* uniqueInstance;
        CAnnotations(tf::uint32 volHeight, tf::uint32 volWidth, tf::uint32 volDepth) : octreeDimX(volWidth), octreeDimY(volHeight), octreeDimZ(volDepth)
        {
            /**/tf::debug(tf::LEV1, strprintf("volHeight = %d, volWidth = %d, volDepth = %d", volHeight, volWidth, volDepth).c_str(), __itm__current__function__);

            octree = new Octree(octreeDimY, octreeDimX, octreeDimZ);

            /**/tf::debug(tf::LEV1, "object successfully constructed", __itm__current__function__);
        }

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CAnnotations* instance(tf::uint32 volHeight, tf::uint32 volWidth, tf::uint32 volDepth)
        {
            if (uniqueInstance == 0)
                uniqueInstance = new CAnnotations(volHeight, volWidth, volDepth);
            return uniqueInstance;
        }
        static CAnnotations* getInstance() throw (tf::RuntimeException)
        {
            if(uniqueInstance)
                return uniqueInstance;
            else
                throw tf::RuntimeException("in CAnnotations::getInstance(): no object has been instantiated yet");
        }
        static void uninstance();
        ~CAnnotations();

        /*********************************************************************************
        * Find/Add/Clear annotation(s)
        **********************************************************************************/
        void findLandmarks (tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range, QList<LocationSimple> &markers) throw (tf::RuntimeException);
        void addLandmarks  (tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range, LandmarkList& markers) throw (tf::RuntimeException);
        void clearLandmarks(tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range) throw (tf::RuntimeException);

        void findCurves (tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range, QList<NeuronSWC> &curves) throw (tf::RuntimeException);
        void addCurves  (tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range, NeuronTree& nt) throw (tf::RuntimeException);
        void clearCurves(tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range) throw (tf::RuntimeException);

        // @obsolete void removeCurves(std::list<NeuronSWC> &curves) throw (RuntimeException);
        // @obsolote void removeLandmarks(std::list<LocationSimple> &markers) throw (RuntimeException);

        Octree* getOctree(){return octree;}

        /*********************************************************************************
        * Save/load method
        **********************************************************************************/
        void save(const char* filepath,bool removedupnode, bool as_swc) throw (tf::RuntimeException);
        void load(const char* filepath) throw (tf::RuntimeException);
        void deleteOldAnnotations(const char* filepath) throw (tf::RuntimeException);

        void removeDuplicatedNode(QList<NeuronSWC> &saveSWC,QList<NeuronSWC> &result);
        bool Sort_SWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid=-1);
        bool Sort_SWC_NewVersion(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result,V3DLONG newrootid=-1);
        /*********************************************************************************
        * Removes all the annotations from the octree
        **********************************************************************************/
        void clear()  throw (tf::RuntimeException)
        {
            /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

            delete octree;
            octree = new Octree(octreeDimY, octreeDimX, octreeDimZ);
        }

        /*********************************************************************************
        * Checks whether the octree is empty
        **********************************************************************************/
        bool isEmpty()
        {
            if(octree)
                return octree->deep_count() == 0;
            else
                return true;
        }

        /*********************************************************************************
        * Counts the number of stored annotations
        **********************************************************************************/
        tf::uint32 count()
        {
            if(octree)
                return octree->count();
            else
                return 0;
        }

        /*********************************************************************************
        * Counts markers having distance <= d from each other
        **********************************************************************************/
        tf::uint32 countDuplicateMarkers(int d=0) throw (tf::RuntimeException);


        /*********************************************************************************
        * Prunes the octree by removing duplicate nodes w/o altering the branching structure
        **********************************************************************************/
        void prune() throw (tf::RuntimeException)
        {
            if(octree)
                octree->prune();
        }



        /*********************************************************************************
        * Conversion from VTK to APO files
        **********************************************************************************/
        static void convertVtk2APO(std::string vtkPath, std::string apoPath) throw (tf::RuntimeException);

        /*********************************************************************************
        * Conversion from MaMuT to APO files
        **********************************************************************************/
        static void convertMaMuT2APO(std::string MaMuTPath, std::string apoPath) throw (tf::RuntimeException);

        /*********************************************************************************
        * Diff between two APO files
        **********************************************************************************/
        static void diffAPO(std::string apo1Path,   // first apo file path
                            std::string apo2Path,   // second apo file path
                            int x0=0, int x1=-1,    // VOI [x0, x1) in the global reference sys
                            int y0=0, int y1=-1,    // VOI [y0, y1) in the global reference sys
                            int z0=0, int z1=-1,    // VOI [z0, z1) in the global reference sys
                            std::string diffPath="")// path where the difference apo file (containing only FPs and FNs) has to be stored (optional)
        throw (tf::RuntimeException);


        /*********************************************************************************
        * Compute type I and type II errors between two APO files
        **********************************************************************************/
        static std::pair<int, int>                  // return pair<false positives, false negatives>
            typeIandIIerrorAPO(std::string apo1Path,// first apo file path (assumed as TRUTH)
                               std::string apo2Path,// second apo file to be compared
                               int d,               // maximum distance between a finding that matches with a truth
                               std::string filter,  // filter cells in apo2 by name
                               const std::string & outputPath = "")
        throw (tf::RuntimeException);


        /*********************************************************************************
        * Trim APO files
        **********************************************************************************/
        static void trimAPO(std::string inputPath,  // input apo file path
                            std::string outputPath, // where output apo file is saved
                            int x0=0, int x1=-1,    // VOI [x0, x1) in the global reference sys
                            int y0=0, int y1=-1,    // VOI [y0, y1) in the global reference sys
                            int z0=0, int z1=-1)    // VOI [z0, z1) in the global reference sys
        throw (tf::RuntimeException);


        /*********************************************************************************
        * Label duplicates in APO files
        **********************************************************************************/
        static void labelDuplicates(std::string inputPath,  // input apo file path
                                    std::string outputPath, // where output apo file is saved
                                    int d,                  // maximum distance between 2 duplicates
                                    RGBA8 color)            // VOI [y0, y1) in the global reference sys
        throw (tf::RuntimeException);


        /*********************************************************************************
        * Merge .xml ImageJ Cell Counter markers files into .APO
        **********************************************************************************/
        static void mergeImageJCellCounterXMLs(QStringList xmls,  // inputs
                            std::string outputPath, // where output apo file is saved
                            int xS, int yS, int zS, // blocks size
                            int overlap=0,          // blocks overlap
                            int x0=0,               // (0,0,0) block X-coordinate
                            int y0=0,               // (0,0,0) block Y-coordinate
                            int z0=0)               // (0,0,0) block Z-coordinate
        throw (tf::RuntimeException);

        /*********************************************************************************
        *
        **********************************************************************************/
        static void diffnAPO(QStringList apos,        // inputs
                            std::string outputPath) // where output apo file is saved
        throw (tf::RuntimeException);


        static inline bool isMarkerOutOfRendererBounds(const LocationSimple& marker, tf::CViewer &w){
            return marker.x < 0 || marker.y < 0 || marker.z < 0 ||
                   marker.x  >= (w.volH1-w.volH0) || marker.y  >= (w.volV1-w.volV0) || marker.z >= (w.volD1-w.volD0);
        }
        static inline bool isMarkerOutOfRendererBounds(const ImageMarker& marker, tf::CViewer &w){
            return marker.x < 0 || marker.y < 0 || marker.z < 0 ||
                    marker.x  >= (w.volH1-w.volH0) || marker.y  >= (w.volV1-w.volV0) || marker.z >= (w.volD1-w.volD0);
        }

        static inline float distance(const LocationSimple& m1, const LocationSimple& m2){
            return sqrt((m1.x-m2.x)*(m1.x-m2.x) + (m1.y-m2.y)*(m1.y-m2.y) + (m1.z-m2.z)*(m1.z-m2.z));
        }

        static inline float distance(const CellAPO& m1, const CellAPO& m2){
            return sqrt((m1.x-m2.x)*(m1.x-m2.x) + (m1.y-m2.y)*(m1.y-m2.y) + (m1.z-m2.z)*(m1.z-m2.z));
        }

        static inline float distance(tf::annotation* m1, tf::annotation* m2){
            return sqrt((m1->x-m2->x)*(m1->x-m2->x) + (m1->y-m2->y)*(m1->y-m2->y) + (m1->z-m2->z)*(m1->z-m2->z));
        }

        friend struct annotation;
};

#endif // CANNOTATIONS_H
