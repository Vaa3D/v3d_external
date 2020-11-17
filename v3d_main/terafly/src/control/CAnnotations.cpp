#include "renderer_gl1.h"
//#include "renderer.h"
#include <algorithm>
#include <vector>
#include <list>
#include "locale.h"
#include <math.h>
#include <algorithm>
#include <set>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <tinyxml.h>
#include "CAnnotations.h"
#include "CSettings.h"
#include "COperation.h"
#include "CImageUtils.h"
#include "../presentation/PLog.h"
//#include "renderer_gl1.h"
//#include "renderer.h"
#include "../../../../3drenderer/v3dr_surfaceDialog.h"

double SOMA_X = -1.1;
double SOMA_Y = -1.1;
double SOMA_Z = -1.1;
V3DLONG SOMA_FOUND = 0;
//#endif
//#ifndef SOMA_Y
//#define SOMA_Y -1.1
//#endif
//#ifndef SOMA_Z
//#define SOMA_Z -1.1
//#endif

using namespace terafly;
using namespace std;

CAnnotations* CAnnotations::uniqueInstance = 0;
long long annotation::last_ID = -1;
tf::uint64 annotation::instantiated = 0;
tf::uint64 annotation::destroyed = 0;

bool isMarker (annotation* ano) { return ano->type == 0;}

annotation::annotation() throw (tf::RuntimeException){
    type = subtype  = -1;
    r = x = y = z = -1;
    parent = 0;
    vaa3d_n = -1;
    name = comment = "";
    color.r = color.g = color.b = color.a = 0;
    container = 0;
    smart_delete = true;

    // assign first usable ID
    if(last_ID == (std::numeric_limits<long long>::max)())
        throw tf::RuntimeException("Reached the maximum number of annotation instances. Please signal this issue to the developer");
    ID = ++last_ID;

    instantiated++;

    #ifdef terafly_enable_debug_annotations
    tf::debug(tf::LEV_MAX, strprintf("%lld(%.0f, %.0f, %.0f) born", ID, x, y, z).c_str(), 0, true);
    #endif
}

annotation::~annotation()
{
    //"smart" deletion
    if(smart_delete)
    {
        // if this is a tree-like structure, destroy children first
        if(type == 1)
            for(std::set<annotation*>::iterator it = children.begin(); it != children.end(); it++)
                delete *it;

        // remove annotation from the Octree
        static_cast<CAnnotations::Octree::octant*>(container)->container->remove(this);
    }

    destroyed++;

    #ifdef terafly_enable_debug_annotations
    tf::debug(tf::LEV_MAX, strprintf("%lld(%.0f, %.0f, %.0f) DESTROYED (smart_delete = %s)", ID, x, y, z, smart_delete ? "true" : "false").c_str(), 0, true);
    #endif
}

void annotation::ricInsertIntoTree(annotation* node, QList<NeuronSWC> &tree)
{
    // create NeuronSWC node
    NeuronSWC p;
    p.type = node->subtype;
    p.n = node->ID;
    p.x = node->x;
    p.y = node->y;
    p.z = node->z;
    p.r = node->r;
    p.level = node->level;
    p.creatmode = node->creatmode; //for timestamping and quality control LMG 8/10/2018
    p.timestamp = node->timestamp; //for timestamping and quality control LMG 8/10/2018
    p.tfresindex = node->tfresindex; //for keepin TeraFly resolution index LMG 13/12/2018
    p.pn = node->parent ? node->parent->ID : -1;
    // add node to list
    #ifdef terafly_enable_debug_annotations
    tf::debug(tf::LEV_MAX, strprintf("Add node %lld(%.0f, %.0f, %.0f) to list", p.n, p.x, p.y, p.z).c_str(), 0, true);
    #endif
    tree.push_back(p);

    // recur on children nodes
    for(std::set<annotation*>::const_iterator it = node->children.begin(); it != node->children.end(); it++)
        ricInsertIntoTree((*it), tree);
}

void annotation::insertIntoTree(QList<NeuronSWC> &tree)
{
    ricInsertIntoTree(this, tree);
}

void CAnnotations::uninstance()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

CAnnotations::~CAnnotations()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    if(octree)
        delete octree;
    octree = 0;

    /**/tf::debug(tf::LEV1, "object successfully destroyed", __itm__current__function__);
}

//recursive support method of 'clear' method
void CAnnotations::Octree::_rec_clear(const Poctant& p_octant) throw(RuntimeException)
{
    if(p_octant)
    {
        _rec_clear(p_octant->child1);
        _rec_clear(p_octant->child2);
        _rec_clear(p_octant->child3);
        _rec_clear(p_octant->child4);
        _rec_clear(p_octant->child5);
        _rec_clear(p_octant->child6);
        _rec_clear(p_octant->child7);
        _rec_clear(p_octant->child8);

        while(!p_octant->annotations.empty())
        {
            annotation *ano = p_octant->annotations.back();
            p_octant->annotations.pop_back();
            ano->smart_delete = false;  // turn "smart" delete off before calling the decontructor
            delete ano;
        }

        delete p_octant;
    }
}

//recursive support method of 'insert' method
void CAnnotations::Octree::_rec_insert(const Poctant& p_octant, annotation& neuron) throw(RuntimeException)
{
    //if the octant is greater than a point, the insert is recursively postponed until a 1x1x1 leaf octant is reached
    if(p_octant->V_dim > 1 || p_octant->H_dim > 1 || p_octant->D_dim > 1)
    {
//        printf("\nIn octant V[%d-%d),H[%d-%d),D[%d-%d), neuron V[%.1f],H[%.1f],D[%.1f]\n",
//                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                p_octant->D_start, p_octant->D_start+p_octant->D_dim,
//                neuron.y, neuron.x, neuron.z);
        p_octant->n_annotations++;
        uint32 V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
        uint32 H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
        uint32 D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));

        //child1: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        if	   (neuron.y >= p_octant->V_start               && neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start               && neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start               && neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child1)
                p_octant->child1 = new octant(p_octant->V_start,                V_dim_halved,
                                                                                p_octant->H_start,				H_dim_halved,
                                                                                p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child1\n");
            _rec_insert(p_octant->child1, neuron);
        }

        //child2: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child2)
                p_octant->child2 = new octant(p_octant->V_start,              V_dim_halved,
                                                                              p_octant->H_start,				H_dim_halved,
                                                                              p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child2\n");
            _rec_insert(p_octant->child2, neuron);
        }


        //child3: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child3)
                    p_octant->child3 = new octant(p_octant->V_start,          V_dim_halved,
                                                                              p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                              p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child3\n");
            _rec_insert(p_octant->child3, neuron);
        }

        //child4: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start				&& neuron.y < p_octant->V_start+V_dim_halved		&&
                neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child4)
                p_octant->child4 = new octant(p_octant->V_start,              V_dim_halved,
                                                                              p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                              p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child4\n");
            _rec_insert(p_octant->child4, neuron);
        }


        //child5: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start+V_dim_halved	&& neuron.y < p_octant->V_start+p_octant->V_dim		&&
                neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child5)
                p_octant->child5 = new octant(p_octant->V_start+V_dim_halved, V_dim_halved,
                                                                              p_octant->H_start,				H_dim_halved,
                                                                              p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child5\n");
            _rec_insert(p_octant->child5, neuron);
        }

        //child6: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start+V_dim_halved	&& neuron.y < p_octant->V_start+p_octant->V_dim		&&
                neuron.x >= p_octant->H_start				&& neuron.x < p_octant->H_start+H_dim_halved		&&
                neuron.z >= p_octant->D_start+D_dim_halved	&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child6)
                p_octant->child6 = new octant(p_octant->V_start+V_dim_halved,	V_dim_halved,
                                                                              p_octant->H_start,				H_dim_halved,
                                                                              p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child6\n");
            _rec_insert(p_octant->child6, neuron);
        }


        //child7: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron.y >= p_octant->V_start+V_dim_halved	&& neuron.y < p_octant->V_start+p_octant->V_dim		&&
                neuron.x >= p_octant->H_start+H_dim_halved	&& neuron.x < p_octant->H_start+p_octant->H_dim		&&
                neuron.z >= p_octant->D_start				&& neuron.z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child7)
                p_octant->child7 = new octant(p_octant->V_start+V_dim_halved,	V_dim_halved,
                                                                                p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                                p_octant->D_start,				D_dim_halved, p_octant->container);
            //printf("inserting in child7\n");
            _rec_insert(p_octant->child7, neuron);
        }


        //child8: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron.y >= p_octant->V_start+V_dim_halved			&& neuron.y < p_octant->V_start+p_octant->V_dim   &&
                neuron.x >= p_octant->H_start+H_dim_halved			&& neuron.x < p_octant->H_start+p_octant->H_dim   &&
                neuron.z >= p_octant->D_start+D_dim_halved			&& neuron.z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child8)
                p_octant->child8 = new octant(p_octant->V_start+V_dim_halved,	V_dim_halved,
                                                                                p_octant->H_start+H_dim_halved,	H_dim_halved,
                                                                                p_octant->D_start+D_dim_halved,	D_dim_halved, p_octant->container);
            //printf("inserting in child8\n");
            _rec_insert(p_octant->child8, neuron);
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::Octree::insert(...): Out of bounds neuron [%.0f,%.0f,%.0f] (vaa3d n = %d).\n\n"
                                             "To activate out of bounds neuron visualization, please go to \"Options\"->\"3D annotation\"->\"Virtual space size\" and select the option \"Unlimited\".",
                                             neuron.x, neuron.y, neuron.z, neuron.vaa3d_n));
    }
    else
    {
//        printf("\nIn octant V[%d-%d),H[%d-%d),D[%d-%d), neuron V[%.1f],H[%.1f],D[%.1f] INSERTED\n",
//                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                p_octant->D_start, p_octant->D_start+p_octant->D_dim,
//                neuron.y, neuron.x, neuron.z);

        p_octant->n_annotations++;
        neuron.container = static_cast<void*>(p_octant);
        p_octant->annotations.push_back(&neuron);

        #ifdef terafly_enable_debug_annotations
        tf::debug(tf::LEV_MAX, strprintf("Added neuron %lld(%lld) {%.0f, %.0f, %.0f} to annotations list of octant X[%d,%d] Y[%d,%d] Z[%d,%d]",
                                           neuron.ID, neuron.parent ? neuron.parent->ID : -1, neuron.x, neuron.y, neuron.z,
                                           p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                           p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                           p_octant->D_start, p_octant->D_start+p_octant->D_dim).c_str(), 0, true);
        #endif
    }
}

//recursive support method of 'remove' method
void CAnnotations::Octree::_rec_remove(const Poctant& p_octant, annotation *neuron) throw(RuntimeException)
{
    //if the octant is greater than a point, the remove operation is recursively postponed until a 1x1x1 leaf octant is reached
    if(p_octant->V_dim > 1 || p_octant->H_dim > 1 || p_octant->D_dim > 1)
    {
        p_octant->n_annotations--;
        uint32 V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
        uint32 H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
        uint32 D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));

        //child1: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        if	   (neuron->y >= p_octant->V_start	&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start	&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start    && neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child1)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child1, neuron);
        }

        //child2: [V_start,			V_start+V_dim/2),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start				&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start				&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start+D_dim_halved	&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child2)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child2, neuron);
        }


        //child3: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron->y >= p_octant->V_start				&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start+H_dim_halved	&& neuron->x < p_octant->H_start+p_octant->H_dim		&&
                neuron->z >= p_octant->D_start				&& neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child3)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child3, neuron);
        }

        //child4: [V_start,			V_start+V_dim/2),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start				&& neuron->y < p_octant->V_start+V_dim_halved		&&
                neuron->x >= p_octant->H_start+H_dim_halved	&& neuron->x < p_octant->H_start+p_octant->H_dim		&&
                neuron->z >= p_octant->D_start+D_dim_halved	&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child4)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child4, neuron);
        }


        //child5: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start,			D_start+D_dim/2)
        else if(neuron->y >= p_octant->V_start+V_dim_halved	&& neuron->y < p_octant->V_start+p_octant->V_dim		&&
                neuron->x >= p_octant->H_start				&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start				&& neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child5)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child5, neuron);
        }

        //child6: [V_start+V_dim/2, V_start+V_dim  ),[H_start,			H_start+H_dim/2),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start+V_dim_halved	&& neuron->y < p_octant->V_start+p_octant->V_dim		&&
                neuron->x >= p_octant->H_start				&& neuron->x < p_octant->H_start+H_dim_halved		&&
                neuron->z >= p_octant->D_start+D_dim_halved	&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child6)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child6, neuron);
        }


        //child7: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start,			D_start+D_dim/2)
        else if(neuron->y >= p_octant->V_start+V_dim_halved	&& neuron->y < p_octant->V_start+p_octant->V_dim		&&
                neuron->x >= p_octant->H_start+H_dim_halved	&& neuron->x < p_octant->H_start+p_octant->H_dim		&&
                neuron->z >= p_octant->D_start				&& neuron->z < p_octant->D_start+D_dim_halved)
        {
            if(!p_octant->child7)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child7, neuron);
        }


        //child8: [V_start+V_dim/2, V_start+V_dim  ),[H_start+H_dim/2,	H_start+H_dim  ),[D_start+D_dim/2,	D_start+D_dim  )
        else if(neuron->y >= p_octant->V_start+V_dim_halved			&& neuron->y < p_octant->V_start+p_octant->V_dim   &&
                neuron->x >= p_octant->H_start+H_dim_halved			&& neuron->x < p_octant->H_start+p_octant->H_dim   &&
                neuron->z >= p_octant->D_start+D_dim_halved			&& neuron->z < p_octant->D_start+p_octant->D_dim)
        {
            if(!p_octant->child8)
                throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

            _rec_remove(p_octant->child8, neuron);
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::Octree::remove(...): Cannot find the proper region wherein to remove the given neuron [%.0f,%.0f,%.0f]", neuron->y, neuron->x, neuron->z));
    }
    else
    {
        p_octant->n_annotations--;

        if(std::find(p_octant->annotations.begin(), p_octant->annotations.end(), neuron) == p_octant->annotations.end())
            throw RuntimeException(strprintf("Cannot remove node (%.0f, %.0f, %.0f) from octree: unable to find the node", neuron->y, neuron->x, neuron->z));

        p_octant->annotations.remove(neuron);

        #ifdef terafly_enable_debug_annotations
        tf::debug(tf::LEV_MAX, strprintf("REMOVED neuron %lld(%lld) {%.0f, %.0f, %.0f} from annotations list of octant X[%d,%d] Y[%d,%d] Z[%d,%d]",
                                           neuron->ID, neuron->parent ? neuron->parent->ID : -1, neuron->x, neuron->y, neuron->z,
                                           p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                           p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                           p_octant->D_start, p_octant->D_start+p_octant->D_dim).c_str(), 0, true);
        #endif
    }
}

//recursive support method of 'deep_count' method
tf::uint32 CAnnotations::Octree::_rec_deep_count(const Poctant& p_octant) throw(RuntimeException)
{
    if(p_octant)
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
             return 1;
        else
            return  _rec_deep_count(p_octant->child1)+_rec_deep_count(p_octant->child2)+_rec_deep_count(p_octant->child3)+
                    _rec_deep_count(p_octant->child4)+_rec_deep_count(p_octant->child5)+_rec_deep_count(p_octant->child6)+
                    _rec_deep_count(p_octant->child7)+_rec_deep_count(p_octant->child8);
    else return 0;
}

//recursive support method of 'toNeuronTree' method
void CAnnotations::Octree::_rec_to_neuron_tree(const Poctant& p_octant, QList<NeuronSWC> &segments) throw(tf::RuntimeException)
{
    if(p_octant && p_octant->V_dim > 1 && p_octant->H_dim > 1 && p_octant->D_dim > 1)
//    if(p_octant)
    {
        // 12 octant edges (24 corners) to be created...
        NeuronSWC corners[24];
        for(int i=0; i<24; i++)
        {
            corners[i].type = 2;
            corners[i].n = (i == 0 ? (segments.empty()? 0 : segments.back().n) : corners[i-1].n) + 1;
        }


        // assign coordinates to the 24 corners
        // --- top-left-front corner --------------------------
        corners[0].y = corners[8].y = corners[16].y = p_octant->V_start;
        corners[0].x = corners[8].x = corners[16].x = p_octant->H_start;
        corners[0].z = corners[8].z = corners[16].z = p_octant->D_start;
        // --- top-right-front corner -------------------------
        corners[1].y = corners[9].y = corners[17].y = p_octant->V_start;
        corners[1].x = corners[9].x = corners[17].x = p_octant->H_start + p_octant->H_dim -1;
        corners[1].z = corners[9].z = corners[17].z = p_octant->D_start;
        // --- top-right-back corner --------------------------
        corners[2].y = corners[10].y = corners[18].y = p_octant->V_start;
        corners[2].x = corners[10].x = corners[18].x = p_octant->H_start + p_octant->H_dim - 1;
        corners[2].z = corners[10].z = corners[18].z = p_octant->D_start + p_octant->D_dim - 1;
        // --- top-left-back corner ---------------------------
        corners[3].y = corners[11].y = corners[19].y = p_octant->V_start;
        corners[3].x = corners[11].x = corners[19].x = p_octant->H_start;
        corners[3].z = corners[11].z = corners[19].z = p_octant->D_start + p_octant->D_dim - 1;
        // --- bottom-left-front corner -----------------------
        corners[4].y = corners[12].y = corners[20].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[4].x = corners[12].x = corners[20].x = p_octant->H_start;
        corners[4].z = corners[12].z = corners[20].z = p_octant->D_start;
        // --- bottom-right-front corner ----------------------
        corners[5].y = corners[13].y = corners[21].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[5].x = corners[13].x = corners[21].x = p_octant->H_start + p_octant->H_dim - 1;
        corners[5].z = corners[13].z = corners[21].z =  p_octant->D_start;
        // --- bottom-right-back corner -----------------------
        corners[6].y = corners[14].y = corners[22].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[6].x = corners[14].x = corners[22].x = p_octant->H_start + p_octant->H_dim - 1;
        corners[6].z = corners[14].z = corners[22].z = p_octant->D_start + p_octant->D_dim - 1;
        // --- bottom-left-back corner ------------------------
        corners[7].y = corners[15].y = corners[23].y = p_octant->V_start + p_octant->V_dim - 1;
        corners[7].x = corners[15].x = corners[23].x = p_octant->H_start;
        corners[7].z = corners[15].z = corners[23].z = p_octant->D_start + p_octant->D_dim - 1;

        // create links
        corners[0].pn = -1;
        corners[1].pn = corners[0].n;
        corners[2].pn = -1;
        corners[3].pn = corners[2].n;
        corners[4].pn = -1;
        corners[5].pn = corners[4].n;
        corners[6].pn = -1;
        corners[7].pn = corners[6].n;
        corners[8].pn = -1;
        corners[11].pn = corners[8].n;
        corners[9].pn = -1;
        corners[10].pn = corners[9].n;
        corners[12].pn = -1;
        corners[15].pn = corners[12].n;
        corners[13].pn = -1;
        corners[14].pn = corners[13].n;
        corners[16].pn = -1;
        corners[20].pn = corners[16].n;
        corners[17].pn = -1;
        corners[21].pn = corners[17].n;
        corners[18].pn = -1;
        corners[22].pn = corners[18].n;
        corners[19].pn = -1;
        corners[23].pn = corners[19].n;

        // add links to NeuronTree
        for(int i=0; i<24; i++)
        {
            // apply trims
            corners[i].x = tf::saturate_trim<float>(corners[i].x, DIM_H - 1);
            corners[i].y = tf::saturate_trim<float>(corners[i].y, DIM_V - 1);
            corners[i].z = tf::saturate_trim<float>(corners[i].z, DIM_D - 1);

            segments.push_back(corners[i]);
        }

        // process children
        _rec_to_neuron_tree(p_octant->child1, segments);
        _rec_to_neuron_tree(p_octant->child2, segments);
        _rec_to_neuron_tree(p_octant->child3, segments);
        _rec_to_neuron_tree(p_octant->child4, segments);
        _rec_to_neuron_tree(p_octant->child5, segments);
        _rec_to_neuron_tree(p_octant->child6, segments);
        _rec_to_neuron_tree(p_octant->child7, segments);
        _rec_to_neuron_tree(p_octant->child8, segments);
    }
}

//recursive support method of 'height' method
tf::uint32 CAnnotations::Octree::_rec_height(const Poctant& p_octant) throw(RuntimeException)
{
    if(p_octant)
    {
        uint32 height_1 = _rec_height(p_octant->child1);
        uint32 height_2 = _rec_height(p_octant->child2);
        uint32 height_3 = _rec_height(p_octant->child3);
        uint32 height_4 = _rec_height(p_octant->child4);
        uint32 height_5 = _rec_height(p_octant->child5);
        uint32 height_6 = _rec_height(p_octant->child6);
        uint32 height_7 = _rec_height(p_octant->child7);
        uint32 height_8 = _rec_height(p_octant->child8);
        return 1+ ( MAX(MAX(MAX(height_1,height_2),MAX(height_3,height_4)),MAX(MAX(height_5,height_6),MAX(height_7,height_8))) );
    }
    else return 0;
}

//recursive support method of 'height' method
void CAnnotations::Octree::_rec_print(const Poctant& p_octant)
{
    if(p_octant)
    {
        printf("V[%d-%d),H[%d-%d),D[%d-%d)\n",p_octant->V_start, p_octant->V_start+p_octant->V_dim,
                                                                                  p_octant->H_start, p_octant->H_start+p_octant->H_dim,
                                                                                  p_octant->D_start, p_octant->D_start+p_octant->D_dim);
        for(std::list<terafly::annotation*>::iterator i = p_octant->annotations.begin(); i!= p_octant->annotations.end(); i++)
            printf("|===> %.2f %.2f %.2f\n", (*i)->y, (*i)->x, (*i)->z);
        _rec_print(p_octant->child1);
        _rec_print(p_octant->child2);
        _rec_print(p_octant->child3);
        _rec_print(p_octant->child4);
        _rec_print(p_octant->child5);
        _rec_print(p_octant->child6);
        _rec_print(p_octant->child7);
        _rec_print(p_octant->child8);
    }
}

//recursive support method of 'prune' method
void CAnnotations::Octree::_rec_prune(const Poctant& p_octant) throw(tf::RuntimeException)
{
    if(p_octant)
    {
        // prune goes here
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
        {
            std::list<annotation*>::iterator i = p_octant->annotations.begin();
            while (i != p_octant->annotations.end() && p_octant->annotations.size() > 1)
            {
                annotation &ai  = **i;
                std::list<annotation*>::iterator j = i;
                while (j != p_octant->annotations.end() && p_octant->annotations.size() > 1)
                {
                    annotation &aj = **j;
                    if(*i != *j                        &&  // two different nodes
                       ai.type    == 1 && aj.type == 1 &&  // both have neuron type
                       ai         ==      aj           &&  // both have same coordinates
                       ai.parent  &&      aj.parent    &&  // both have a valid parent
                     *(ai.parent) ==    *(aj.parent))      // both have the same parent's coordinates (otherwise this is a crossroad and MUST not be pruned)
                    {
                        // aj is considered as duplicate --> ai takes its place in 4 actions:
                        printf("found duplicate neuron at (%d, %d, %d)\n", p_octant->H_start, p_octant->V_start, p_octant->D_start);

                        // 1) children of aj are assigned new parent ai
                        for(std::set<annotation*>::iterator ajc = aj.children.begin(); ajc != aj.children.end(); ajc++)
                            (*ajc)->parent = &ai;

                        // 2) children of aj become children of ai
                        ai.children.insert(aj.children.begin(), aj.children.end());

                        // 3) remove aj from it's parent children list
                        aj.parent->children.erase(&aj);

                        // 4) remove aj from the octree
                        p_octant->annotations.erase(j);

                        // 5) deallocate memory for aj
                        aj.smart_delete = false;
                        delete &aj;
                    }
                    else
                        ++j;
                }
                if(i != p_octant->annotations.end())
                    ++i;
            }
        }
        else
        {
            _rec_prune(p_octant->child1);
            _rec_prune(p_octant->child2);
            _rec_prune(p_octant->child3);
            _rec_prune(p_octant->child4);
            _rec_prune(p_octant->child5);
            _rec_prune(p_octant->child6);
            _rec_prune(p_octant->child7);
            _rec_prune(p_octant->child8);
        }
    }
}

//recursive support method of 'find' method
void CAnnotations::Octree::_rec_search(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int, std::list<annotation*>& neurons)  throw(RuntimeException)
{
    if(p_octant)
    {
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
        {
            for(std::list<terafly::annotation*>::iterator i = p_octant->annotations.begin(); i!= p_octant->annotations.end(); i++)
                neurons.push_back((*i));
        }
        else
        {
            int V_dim_halved	= static_cast<int>(round((float)p_octant->V_dim/2));
            int V_halved		= p_octant->V_start+V_dim_halved;
            int H_dim_halved	= static_cast<int>(round((float)p_octant->H_dim/2));
            int H_halved		= p_octant->H_start+H_dim_halved;
            int D_dim_halved	= static_cast<int>(round((float)p_octant->D_dim/2));
            int D_halved		= p_octant->D_start+D_dim_halved;

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child1, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child2, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child3, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child4, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child5, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child6, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                _rec_search(p_octant->child7, V_int, H_int, D_int, neurons);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                _rec_search(p_octant->child8, V_int, H_int, D_int, neurons);
        }
    }
}

//recursive support method of 'rec_find' method
CAnnotations::Octree::Poctant CAnnotations::Octree::_rec_find(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(RuntimeException)
{
    //printf("\n_rec_find(p_octant=%d, V_int=[%d-%d), H_int=[%d-%d), D_int=[%d-%d))\n", p_octant, V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end);

    if(p_octant)
    {
//        printf("_rec_find(): in octant V[%d-%d),H[%d-%d),D[%d-%d)\n",
//                p_octant->V_start, p_octant->V_start+p_octant->V_dim,
//                p_octant->H_start, p_octant->H_start+p_octant->H_dim,
//                p_octant->D_start, p_octant->D_start+p_octant->D_dim);
        if(p_octant->V_dim == 1 && p_octant->H_dim == 1 && p_octant->D_dim == 1)
            return p_octant;
        else
        {
            //printf("_rec_find(): smisting...\n");
            int V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
            int V_halved	= p_octant->V_start+V_dim_halved;
            int H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
            int H_halved	= p_octant->H_start+H_dim_halved;
            int D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));
            int D_halved	= p_octant->D_start+D_dim_halved;

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   p_octant->H_start,  p_octant->H_start+H_dim_halved, p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start,  H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child1, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   p_octant->H_start,  p_octant->H_start+H_dim_halved, D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start,  H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child2, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   H_halved,           H_halved+H_dim_halved,          p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,           H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child3, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, p_octant->V_start, p_octant->V_start+V_dim_halved,   H_halved,           H_halved+H_dim_halved,          D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,           H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child4, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            p_octant->H_start,  p_octant->H_start+H_dim_halved, p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,	  V_dim_halved,	p_octant->H_start,  H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child5, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            p_octant->H_start,  p_octant->H_start+H_dim_halved, D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,          V_dim_halved,	p_octant->H_start,  H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child6, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            H_halved,           H_halved+H_dim_halved,          p_octant->D_start,	p_octant->D_start+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,	  V_dim_halved,	H_halved,           H_dim_halved, p_octant->D_start,	D_dim_halved))
                return _rec_find(p_octant->child7, V_int, H_int, D_int);
            //printf("no.\n");

            //printf("V[%d-%d),H[%d-%d),D[%d-%d) intersects V[%d-%d),H[%d-%d),D[%d-%d)?...", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end, V_halved,          V_halved+V_dim_halved,            H_halved,           H_halved+H_dim_halved,          D_halved,		D_halved+D_dim_halved);
            if(intersects(V_int, H_int, D_int, V_halved,	  V_dim_halved,	H_halved,           H_dim_halved, D_halved,		D_dim_halved))
                return _rec_find(p_octant->child8, V_int, H_int, D_int);
            //printf("no.\n");

            return 0;
        }
    }
    else
        return 0;
}

//recursive support method of 'count' method
tf::uint32 CAnnotations::Octree::_rec_count(const Poctant& p_octant, const interval_t& V_int, const interval_t& H_int, const interval_t& D_int) throw(RuntimeException)
{
    if(p_octant)
    {
        if(contains(V_int, H_int, D_int, p_octant->V_start, p_octant->V_dim, p_octant->H_start, p_octant->H_dim, p_octant->D_start, p_octant->D_dim))
           return p_octant->n_annotations;
        else
        {
            uint32 neuron_count = 0;
            int V_dim_halved = static_cast<int>(round((float)p_octant->V_dim/2));
            int V_halved		= p_octant->V_start+V_dim_halved;
            int H_dim_halved = static_cast<int>(round((float)p_octant->H_dim/2));
            int H_halved		= p_octant->H_start+H_dim_halved;
            int D_dim_halved = static_cast<int>(round((float)p_octant->D_dim/2));
            int D_halved		= p_octant->D_start+D_dim_halved;

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child1, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                neuron_count+= _rec_count(p_octant->child2, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child3, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, p_octant->V_start, V_dim_halved, H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                neuron_count+= _rec_count(p_octant->child4, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, p_octant->D_start,	D_dim_halved))
                neuron_count+= _rec_count(p_octant->child5, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	p_octant->H_start, H_dim_halved, D_halved,			D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child6, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, p_octant->D_start,	D_dim_halved))
                 neuron_count+= _rec_count(p_octant->child7, V_int, H_int, D_int);

            if(intersects(V_int, H_int, D_int, V_halved,		  V_dim_halved,	H_halved,		   H_dim_halved, D_halved,			D_dim_halved))
                neuron_count+= _rec_count(p_octant->child8, V_int, H_int, D_int);

            return neuron_count;
        }
    }
    else return 0;
}

//returns true if two given volumes intersect each other
bool inline CAnnotations::Octree::intersects(const interval_t& V1_int,		 const interval_t& H1_int,		   const interval_t& D1_int,
                                                           int& V2_start, int& V2_dim, int& H2_start, int& H2_dim, int& D2_start, int& D2_dim) throw(RuntimeException)
{
    return 	( V1_int.start  < (V2_start + V2_dim)	&&
              V1_int.end    >  V2_start             &&
              H1_int.start  < (H2_start + H2_dim)	&&
              H1_int.end    >  H2_start             &&
              D1_int.start  < (D2_start + D2_dim)	&&
              D1_int.end    >  D2_start	 );
}

//returns true if first volume contains second volume
bool inline CAnnotations::Octree::contains  (const interval_t& V1_int,		 const interval_t& H1_int,		   const interval_t& D1_int,
                                             int& V2_start, int& V2_dim, int& H2_start, int& H2_dim, int& D2_start, int& D2_dim) throw(RuntimeException)
{
    return (  V1_int.start  <=  V2_start            &&
              V1_int.end    >=  (V2_start+V2_dim)	&&
              H1_int.start  <=  H2_start            &&
              H1_int.end    >=  (H2_start+H2_dim)	&&
              D1_int.start  <=  D2_start            &&
              D1_int.end    >=  (D2_start+D2_dim));
}

CAnnotations::Octree::Octree(tf::uint32 _DIM_V, tf::uint32 _DIM_H, tf::uint32 _DIM_D)
{
    /**/tf::debug(tf::LEV1, strprintf("dimV = %d, dimH = %d, dimD = %d", _DIM_V, _DIM_H, _DIM_D).c_str(), __itm__current__function__);

    if(CSettings::instance()->getAnnotationSpaceUnlimited())
    {
        DIM_V = std::numeric_limits<int>::max();
        DIM_H = std::numeric_limits<int>::max();
        DIM_D = std::numeric_limits<int>::max();

        /**/tf::debug(tf::LEV1, strprintf("unbounded annotation space activated", _DIM_V, _DIM_H, _DIM_D).c_str(), __itm__current__function__);
    }
    else
    {
        DIM_V = _DIM_V;
        DIM_H = _DIM_H;
        DIM_D = _DIM_D;
    }
    root = new octant(0,DIM_V,0,DIM_H,0,DIM_D, this);
}

CAnnotations::Octree::~Octree(void)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    clear();

    /**/tf::debug(tf::LEV1, "object successfully DESTROYED", __itm__current__function__);
}

//clears octree content and deallocates used memory
void CAnnotations::Octree::clear() throw(RuntimeException)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    _rec_clear(root);
    root = 0;
}

//prunes the octree by removing all nodes duplicates while maintaining the same branched structure
void CAnnotations::Octree::prune() throw(tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    _rec_prune(root);
}

//insert given neuron in the octree
void CAnnotations::Octree::insert(annotation& neuron)  throw(RuntimeException)
{
    // @FIXED by Alessandro on 2017-09-03: set negative coordinates to zero so that annotation can be stored in the octree
    neuron.x = std::max(neuron.x, 0.f);
    neuron.y = std::max(neuron.y, 0.f);
    neuron.z = std::max(neuron.z, 0.f);

    _rec_insert(root,neuron);
}

//remove given neuron from the octree (returns 1 if succeeds)
bool CAnnotations::Octree::remove(annotation *neuron) throw(tf::RuntimeException)
{
    std::list<annotation*>* matching_nodes = this->find(neuron->x, neuron->y, neuron->z);
    if(std::find(matching_nodes->begin(), matching_nodes->end(), neuron) == matching_nodes->end())
        return false;
    else
    {
        _rec_remove(root, neuron);
        return true;
    }
}

//search for the annotations at the given coordinate. If found, returns the address of the annotations list
std::list<annotation*>* CAnnotations::Octree::find(float x, float y, float z) throw(RuntimeException)
{
    interval_t V_range(static_cast<int>(floor(y)), static_cast<int>(ceil(y)));
    interval_t H_range(static_cast<int>(floor(x)), static_cast<int>(ceil(x)));
    interval_t D_range(static_cast<int>(floor(z)), static_cast<int>(ceil(z)));
    if(V_range.end == V_range.start)
        V_range.end++;
    if(H_range.end == H_range.start)
        H_range.end++;
    if(D_range.end == D_range.start)
        D_range.end++;
    Poctant oct = _rec_find(root, V_range, H_range, D_range);
    if(oct)
        return &(oct->annotations);
    else
        return 0;
}

//returns the number of neurons (=leafs) in the octree by exploring the entire data structure
tf::uint32 CAnnotations::Octree::deep_count() throw(RuntimeException)
{
    return _rec_deep_count(root);
}

//returns the octree height
tf::uint32 CAnnotations::Octree::height() throw(RuntimeException)
{
    return _rec_height(root);
}

//print the octree content
void CAnnotations::Octree::print()
{
    printf("\n\nOCTREE start printing...\n\n");
    _rec_print(root);
    printf("\n\nOCTREE end printing...\n\n");
}

//search for neurons in the given 3D volume and puts found neurons into 'neurons'
void CAnnotations::Octree::find(interval_t V_int, interval_t H_int, interval_t D_int, std::list<annotation*>& neurons) throw(RuntimeException)
{
	// check interval validity
	if( H_int.start < 0 || H_int.end < 0 || (H_int.end-H_int.start < 0) || 
		V_int.start < 0 || V_int.end < 0 || (V_int.end-V_int.start < 0) || 
		D_int.start < 0 || D_int.end < 0 || (D_int.end-D_int.start < 0))
		throw tf::RuntimeException( tf::strprintf("invalid interval X[%d,%d), Y[%d,%d), Z[%d,%d)",
		H_int.start, H_int.end, V_int.start, V_int.end, D_int.start, D_int.end), tf::shortFuncName(__itm__current__function__));


    /**/tf::debug(tf::LEV2, strprintf("interval = [%d,%d](V) x [%d,%d](H) x [%d,%d](D)", V_int.start, V_int.end, H_int.start, H_int.end, D_int.start, D_int.end).c_str(), __itm__current__function__);
    _rec_search(root, V_int, H_int, D_int, neurons);
    /**/tf::debug(tf::LEV2, strprintf("found %d neurons", neurons.size()).c_str(), __itm__current__function__);
}

//returns the number of neurons (=leafs) in the given volume without exploring the entire data structure
tf::uint32 CAnnotations::Octree::count(interval_t V_int, interval_t H_int, interval_t D_int) throw(RuntimeException)
{
    //adjusting default parameters
    V_int.start = V_int.start == -1 ? 0		: V_int.start;
    V_int.end   = V_int.end   == -1 ? DIM_V : V_int.end;
    H_int.start = H_int.start == -1 ? 0		: H_int.start;
    H_int.end   = H_int.end   == -1 ? DIM_H : H_int.end;
    D_int.start = D_int.start == -1 ? 0		: D_int.start;
    D_int.end   = D_int.end   == -1 ? DIM_D : D_int.end;

    return _rec_count(root, V_int, H_int, D_int);
}


/*********************************************************************************
* Adds the given annotation(s)
**********************************************************************************/
void CAnnotations::addLandmarks(tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range, LandmarkList &markers) throw (RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d), markers.size = %d",
                                        X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end, markers.size()).c_str(), __itm__current__function__);

	// check interval validity
	if( X_range.start < 0 || X_range.end < 0 || (X_range.end-X_range.start < 0) || 
		Y_range.start < 0 || Y_range.end < 0 || (Y_range.end-Y_range.start < 0) || 
		Z_range.start < 0 || Z_range.end < 0 || (Z_range.end-Z_range.start < 0))
		throw tf::RuntimeException( tf::strprintf("invalid interval X[%d,%d), Y[%d,%d), Z[%d,%d)",
		X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end), tf::shortFuncName(__itm__current__function__));


    /**/tf::debug(tf::LEV3, strprintf("%d markers before clearLandmarks", count()).c_str(), __itm__current__function__);
    clearLandmarks(X_range, Y_range, Z_range);
    /**/tf::debug(tf::LEV3, strprintf("%d markers after clearLandmarks", count()).c_str(), __itm__current__function__);


    QElapsedTimer timer;
    timer.start();
    for(int i=0; i<markers.size(); i++)
    {
       annotation* node = new annotation();
       node->type = 0;
       node->subtype = markers[i].category;
       node->parent = 0;
       node->r = markers[i].radius;
       node->x = markers[i].x;
       node->y = markers[i].y;
       node->z = markers[i].z;
       node->name = markers[i].name;
       node->comment = markers[i].comments;
       node->color = markers[i].color;
       octree->insert(*node);
    }
    PLog::instance()->appendOperation(new AnnotationOperation("store annotations: add landmarks", tf::CPU, timer.elapsed()));

    /**/tf::debug(tf::LEV3, strprintf("%d markers after insertions", count()).c_str(), __itm__current__function__);
}

void CAnnotations::clearCurves(tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range) throw (tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

	// check interval validity
	if( X_range.start < 0 || X_range.end < 0 || (X_range.end-X_range.start < 0) || 
		Y_range.start < 0 || Y_range.end < 0 || (Y_range.end-Y_range.start < 0) || 
		Z_range.start < 0 || Z_range.end < 0 || (Z_range.end-Z_range.start < 0))
		throw tf::RuntimeException( tf::strprintf("invalid interval X[%d,%d), Y[%d,%d), Z[%d,%d)",
		X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end), tf::shortFuncName(__itm__current__function__));


    QElapsedTimer timer;
    timer.start();
    std::list<annotation*> nodes;
    std::set <annotation*> roots;
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("clear curves: find curve nodes in the given range", tf::CPU, timer.elapsed()));

    // retrieve root nodes from the nodes founds so far
    timer.restart();
    for(std::list<annotation*>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
    {
        // is a neuron node (type = 1)
        if((*it)->type == 1)
        {
            annotation *p = *it;
            while(p->parent != 0)
                p = p->parent;
            roots.insert(p);
        }
    }
    PLog::instance()->appendOperation(new AnnotationOperation("clear curves: retrieve root nodes from the nodes founds so far", tf::CPU, timer.elapsed()));

    // clear all segments starting from the retrieved root nodes
    timer.restart();
    for(std::set<annotation*>::const_iterator it = roots.begin(); it != roots.end(); it++)
        delete *it;
    PLog::instance()->appendOperation(new AnnotationOperation("clear curves: clear all segments starting from the retrieved root nodes", tf::CPU, timer.elapsed()));
}

void CAnnotations::clearLandmarks(tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range) throw (tf::RuntimeException)
{
    /**/tf::debug(tf::LEV3, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

	// check interval validity
	if( X_range.start < 0 || X_range.end < 0 || (X_range.end-X_range.start < 0) || 
		Y_range.start < 0 || Y_range.end < 0 || (Y_range.end-Y_range.start < 0) || 
		Z_range.start < 0 || Z_range.end < 0 || (Z_range.end-Z_range.start < 0))
		throw tf::RuntimeException( tf::strprintf("invalid interval X[%d,%d), Y[%d,%d), Z[%d,%d)",
		X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end), tf::shortFuncName(__itm__current__function__));


    QElapsedTimer timer;
    timer.start();
    std::list<annotation*> nodes;
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("clear landmarks: find landmarks in the given range", tf::CPU, timer.elapsed()));

    /**/tf::debug(tf::LEV3, strprintf("found %d nodes", nodes.size()).c_str(), __itm__current__function__);
    timer.restart();
    for(std::list<annotation*>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
        if((*it)->type == 0)
            delete *it;
    PLog::instance()->appendOperation(new AnnotationOperation("clear landmarks: remove landmarks", tf::CPU, timer.elapsed()));
}

void CAnnotations::addCurves(tf::interval_t X_range, tf::interval_t Y_range, tf::interval_t Z_range, NeuronTree& nt) throw (tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("X[%d,%d), Y[%d,%d), Z[%d,%d)", X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

	// check interval validity
	if( X_range.start < 0 || X_range.end < 0 || (X_range.end-X_range.start < 0) || 
		Y_range.start < 0 || Y_range.end < 0 || (Y_range.end-Y_range.start < 0) || 
		Z_range.start < 0 || Z_range.end < 0 || (Z_range.end-Z_range.start < 0))
		throw tf::RuntimeException( tf::strprintf("invalid interval X[%d,%d), Y[%d,%d), Z[%d,%d)",
		X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end), tf::shortFuncName(__itm__current__function__));


    // first clear curves in the given range
    tf::uint64 deletions = annotation::destroyed;
    clearCurves(X_range, Y_range, Z_range);
    deletions = annotation::destroyed - deletions;
    /**/tf::debug(tf::LEV3, strprintf("nt.size() = %d, deleted = %llu", nt.listNeuron.size(), deletions).c_str(), __itm__current__function__);

    // then allocate and initialize curve nodes
    QElapsedTimer timer;
    timer.start();
    std::map<int, annotation*> annotationsMap;
    std::map<int, NeuronSWC*> swcMap;
    for(int i=0; i<nt.listNeuron.size(); i++)
    {
        annotation* ann = new annotation();
        ann->type = 1;
        ann->name = nt.name.toStdString();
        ann->comment = nt.comment.toStdString();
        ann->color = nt.color;
        ann->subtype = nt.listNeuron[i].type;
        ann->r = nt.listNeuron[i].r;
        ann->x = nt.listNeuron[i].x;
        ann->y = nt.listNeuron[i].y;
        ann->z = nt.listNeuron[i].z;
        ann->level = nt.listNeuron[i].level;
        ann->creatmode = nt.listNeuron[i].creatmode; //for timestamping and quality control LMG 8/10/2018
        ann->timestamp = nt.listNeuron[i].timestamp; //for timestamping and quality control LMG 8/10/2018
        ann->tfresindex = nt.listNeuron[i].tfresindex; //for TeraFly resolution index       LMG 13/12/2018

        #ifdef terafly_enable_debug_annotations
        tf::debug(tf::LEV_MAX, strprintf("inserting curve point %lld(%.1f,%.1f,%.1f), n=(%d), pn(%d)\n", ann->ID, ann->x, ann->y, ann->z, nt.listNeuron[i].n, nt.listNeuron[i].pn).c_str(), 0, true);
        #endif

        octree->insert(*ann);
        annotationsMap[nt.listNeuron[i].n] = ann;
        swcMap[nt.listNeuron[i].n] = &(nt.listNeuron[i]);
    }

    PLog::instance()->appendOperation(new AnnotationOperation("store annotations: allocate and initialize curve nodes", tf::CPU, timer.elapsed()));

    // finally linking nodes
    timer.restart();
    for(std::map<int, annotation*>::iterator it = annotationsMap.begin(); it!= annotationsMap.end(); it++)
    {
        it->second->parent = swcMap[it->first]->pn == -1 ? 0 : annotationsMap[swcMap[it->first]->pn];
        if(it->second->parent)
        {
            #ifdef terafly_enable_debug_annotations
            tf::debug(tf::LEV_MAX, strprintf("Add %lld(%.0f, %.0f, %.0f) to %lld(%.0f, %.0f, %.0f)'s children list\n",
                                               it->second->ID, it->second->x, it->second->y, it->second->z, it->second->parent->ID,
                                               it->second->parent->x, it->second->parent->y, it->second->parent->z).c_str(), 0, true);
            #endif

            it->second->parent->children.insert(it->second);
        }
    }
    PLog::instance()->appendOperation(new AnnotationOperation("store annotations: link curve nodes", tf::CPU, timer.elapsed()));
//    printf("--------------------- teramanager plugin >> inserted %d curve points\n", annotationsMap.size());
}

/*********************************************************************************
* Retrieves the annotation(s) in the given volume space
**********************************************************************************/
void CAnnotations::findLandmarks(interval_t X_range, interval_t Y_range, interval_t Z_range, QList<LocationSimple> &markers) throw (RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("X_range = [%d,%d), Y_range = [%d,%d), Z_range = [%d,%d)",
                                        X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

	// check interval validity
	if( X_range.start < 0 || X_range.end < 0 || (X_range.end-X_range.start < 0) || 
		Y_range.start < 0 || Y_range.end < 0 || (Y_range.end-Y_range.start < 0) || 
		Z_range.start < 0 || Z_range.end < 0 || (Z_range.end-Z_range.start < 0))
		throw tf::RuntimeException( tf::strprintf("invalid interval X[%d,%d), Y[%d,%d), Z[%d,%d)",
		X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end), tf::shortFuncName(__itm__current__function__));


    std::list<annotation*> nodes;
    QElapsedTimer timer;
    timer.start();

    /**/tf::debug(tf::LEV3, "find all nodes in the given range", __itm__current__function__);
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("find landmarks: find all annotations in the given range", tf::CPU, timer.elapsed()));


    /**/tf::debug(tf::LEV3, "select markers only", __itm__current__function__);
    timer.restart();
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if((*i)->type == 0) //selecting markers
        {
            LocationSimple marker;
            marker.x = (*i)->x;
            marker.y = (*i)->y;
            marker.z = (*i)->z;
            marker.radius = (*i)->r;
            marker.category = (*i)->subtype;
            marker.color = (*i)->color;
            marker.name = (*i)->name;
            marker.comments = (*i)->comment;
            markers.push_back(marker);
        }
    }
    PLog::instance()->appendOperation(new AnnotationOperation("find landmarks: select landmarks only", tf::CPU, timer.elapsed()));
}

void CAnnotations::findCurves(interval_t X_range, interval_t Y_range, interval_t Z_range, QList<NeuronSWC> &curves) throw (RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("X_range = [%d,%d), Y_range = [%d,%d), Z_range = [%d,%d)",
                                        X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end).c_str(), __itm__current__function__);

	// check interval validity
	if( X_range.start < 0 || X_range.end < 0 || (X_range.end-X_range.start < 0) || 
		Y_range.start < 0 || Y_range.end < 0 || (Y_range.end-Y_range.start < 0) || 
		Z_range.start < 0 || Z_range.end < 0 || (Z_range.end-Z_range.start < 0))
		throw tf::RuntimeException( tf::strprintf("invalid interval X[%d,%d), Y[%d,%d), Z[%d,%d)",
		X_range.start, X_range.end, Y_range.start, Y_range.end, Z_range.start, Z_range.end), tf::shortFuncName(__itm__current__function__));


    std::list<annotation*> nodes;
    QElapsedTimer timer;
    timer.start();

    /**/tf::debug(tf::LEV3, "find all nodes in the given range", __itm__current__function__);
    octree->find(Y_range, X_range, Z_range, nodes);
    PLog::instance()->appendOperation(new AnnotationOperation("find curves: find all annotations in the given range", tf::CPU, timer.elapsed()));

    // find roots
    timer.restart();
    /**/tf::debug(tf::LEV3, "find roots", __itm__current__function__);
    std::set<annotation*> roots;
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if((*i)->type == 1) //selecting curve points
        {
            annotation* p = (*i);
            while(p->parent != 0)
                p = p->parent;
            roots.insert(p);
        }
    }
    PLog::instance()->appendOperation(new AnnotationOperation("find curves: find roots", tf::CPU, timer.elapsed()));

    /**/tf::debug(tf::LEV3, strprintf("%d roots found, now inserting all nodes", roots.size()).c_str(), __itm__current__function__);
    timer.restart();
    for(std::set<annotation*>::const_iterator it = roots.begin(); it != roots.end(); it++)
        (*it)->insertIntoTree(curves);
    PLog::instance()->appendOperation(new AnnotationOperation("find curves: insert all linked nodes starting from roots", tf::CPU, timer.elapsed()));
    /**/tf::debug(tf::LEV3, strprintf("%d nodes inserted", curves.size()).c_str(), __itm__current__function__);
}

/*********************************************************************************
* Save/load methods
**********************************************************************************/
//This function was added by shengdian to remove duplicated nodes (not include branch root nodes).2018-12-05
void CAnnotations::removeDuplicatedNode(QList<NeuronSWC> &neuron,QList<NeuronSWC> &result)
{
    vector<long> parents0,rootnodes,tipnodes,duplicatednodes;
    vector<long> ids0;
    vector<long> parentschild;
    //QList<NeuronSWC> neuron = neurons;

    //Remove duplicated nodes
    //get ids and reorder tree with ids following list
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids0.push_back(neuron.at(i).n);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        V3DLONG parentid=neuron.at(i).parent;
        neuron[i].n=i;
        if(neuron.at(i).parent !=-1)
        {
            neuron[i].parent=find(ids0.begin(), ids0.end(),neuron.at(i).parent) - ids0.begin();
            parents0.push_back(neuron.at(i).parent);
        }
    }
//    cout<<"Neuron size is "<<neuron.size()<<endl;
//    cout<<"parents size is "<<parents0.size()<<endl;
    //QList<V3DLONG>  child_num0;//vector<bool> parentexist;
    for(V3DLONG i=0;i<neuron.size();i++)// 0 or 1? check!
    {
        V3DLONG parentid=neuron.at(i).parent;
        //parentexist.push_back(true);
        //check root nodes,nodes' parent node is root and (no child) nodes
        if(parentid!=-1)
        {
            if(neuron.at(parentid).parent==-1)
            {
                parentschild.push_back(neuron.at(i).n);
            }
            //nodes' parent node is root node

        }//if root nodes
        else
        {
            rootnodes.push_back(neuron.at(i).n);//cout<<"root node id is "<<neuron.at(i).n<<endl;
        }//if tip nodes
        if(std::count(parents0.begin(),parents0.end(),neuron.at(i).n)==0)
        {
            tipnodes.push_back(neuron.at(i).n);//cout<<"tip node id is "<<neuron.at(i).n<<endl;
        }
        //child_num0.push_back(count(parents0.begin(),parents0.end(),neuron.at(i).n));
    }
    cout<<"tip nodes size is "<<tipnodes.size()<<endl;
    cout<<"root nodes size is "<<rootnodes.size()<<endl;
    cout<<"parent child node is "<<parentschild.size()<<endl;

    //
    if(tipnodes.size()==0||rootnodes.size()==0||parentschild.size()==0)
    {
        cout<<"size worng"<<endl;
    }
    else
    {
        //remove duplicated root nodes
//        for(V3DLONG r=0;r<rootnodes.size();r++)
//        {
//            for(V3DLONG r1=r+1;r1<rootnodes.size();r1++)
//            {
//                if(/*r!=r1&&*/neuron.at(rootnodes.at(r)).x==neuron.at(rootnodes.at(r1)).x
//                        &&neuron.at(rootnodes.at(r)).y==neuron.at(rootnodes.at(r1)).y
//                        &&neuron.at(rootnodes.at(r)).z==neuron.at(rootnodes.at(r1)).z)
//                {
//                    duplicatednodes.push_back(rootnodes.at(r1));
//                    for(V3DLONG p=0;p<parentschild.size();p++)
//                    {
//                        if(rootnodes.at(r1)==neuron.at(parentschild.at(p)).parent)
//                        {
//                            neuron[parentschild.at(p)].parent=neuron.at(rootnodes.at(r)).n;
//    //                        cout<<"change "<<parentschild.at(p)<<"'s parent id to "<<tipnodes.at(tip)<<"'s "<<neuron.at(tipnodes.at(tip)).n<<endl;
//                            break;
//                        }
//                    }
//                    break;
//                }

//            }
//        }
        for(V3DLONG tip=0;tip<tipnodes.size();tip++)
        {
            for(V3DLONG r=0;r<rootnodes.size();r++)
            {
                //if same position
    //            cout<<"tip id is "<<tipnodes.at(tip)<<" and "<<neuron.at(tipnodes.at(tip)).n<<endl;
    //            cout<<"root id is "<<rootnodes.at(r)<<" and "<<neuron.at(rootnodes.at(r)).n<<endl;
                if(neuron.at(tipnodes.at(tip)).x==neuron.at(rootnodes.at(r)).x
                        &&neuron.at(tipnodes.at(tip)).y==neuron.at(rootnodes.at(r)).y
                        &&neuron.at(tipnodes.at(tip)).z==neuron.at(rootnodes.at(r)).z)
                {
                    //cout<<"duplicated id is "<<rootnodes.at(r)<<endl;
                    duplicatednodes.push_back(rootnodes.at(r));
                    for(V3DLONG p=0;p<parentschild.size();p++)
                    {
                        if(rootnodes.at(r)==neuron.at(parentschild.at(p)).parent)
                        {
                            neuron[parentschild.at(p)].parent=neuron.at(tipnodes.at(tip)).n;
    //                        cout<<"change "<<parentschild.at(p)<<"'s parent id to "<<tipnodes.at(tip)<<"'s "<<neuron.at(tipnodes.at(tip)).n<<endl;
                            break;
                        }
                    }
                    break;
                }
            }

        }
    }
    cout<<"duplicated node size is "<<duplicatednodes.size()<<endl;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        bool needtoremove=false;
        for(V3DLONG d=0;d<duplicatednodes.size();d++)
        {
            if(neuron.at(i).n==neuron.at(duplicatednodes.at(d)).n)
            {
                needtoremove=true;
                break;
            }
        }
        if(!needtoremove)
            result.append(neuron.at(i));
    }
}
#define VOID 1000000000

QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT)
{
    QHash<V3DLONG, V3DLONG> cp;
    for (V3DLONG i=0;i<neurons.size(); i++)
    {
        if (neurons.at(i).pn==-1)
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else if(idlist.indexOf(LUT.value(neurons.at(i).pn)) == 0 && neurons.at(i).pn != neurons.at(0).n)
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), idlist.indexOf(LUT.value(neurons.at(i).pn)));
    }
        return cp;
}

QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons)
{
    // Range of LUT values: [0, # deduplicated neuron list)
    QHash<V3DLONG,V3DLONG> LUT;
    V3DLONG cur_id=0;
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        V3DLONG j=0;
        for (j=0;j<i;j++) // Check whether this node is a duplicated with the previous ones
        {
            if (neurons.at(i).x==neurons.at(j).x && neurons.at(i).y==neurons.at(j).y && neurons.at(i).z==neurons.at(j).z)	break;
        }
        if(i==j){  // not a duplicate of the previous ones
            LUT.insertMulti(neurons.at(i).n, cur_id);
            cur_id++;
        }
        else{
            LUT.insertMulti(neurons.at(i).n, LUT.value(neurons.at(j).n));
        }

    }
    return (LUT);
}

void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group)
{
    if (!numbered[node]){
        numbered[node] = *group;
        neworder[*id] = node;
        (*id)++;
        for (V3DLONG v=0;v<siz;v++)
            if (!numbered[v] && matrix[v][node])
            {
                DFS(matrix, neworder, v, id, siz,numbered,group);
            }
    }
}
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2, double xscale=1, double yscale=1, double zscale=1)
{
    double xx = (s1.x-s2.x)*xscale;
    double yy = (s1.y-s2.y)*yscale;
    double zz = (s1.z-s2.z)*zscale;
    return (xx*xx+yy*yy+zz*zz);
}
bool CAnnotations::Sort_SWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid)
{
    double thres=0;
    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

    //create a new id list to give every different neuron a new id
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

    //create a child-parent table, both child and parent id refers to the index of idlist
    QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);


    V3DLONG siz = idlist.size();

    bool** matrix = new bool*[siz];
    for (V3DLONG i = 0;i<siz;i++)
    {
        matrix[i] = new bool[siz];
        for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
    }


    //generate the adjacent matrix for undirected matrix
    for (V3DLONG i = 0;i<siz;i++)
    {
        QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
        for (V3DLONG j=0;j<parentSet.size();j++)
        {
            V3DLONG v2 = (V3DLONG) (parentSet.at(j));
            if (v2==-1) continue;
            matrix[i][v2] = true;
            matrix[v2][i] = true;
        }
    }


    //do a DFS for the the matrix and re-allocate ids for all the nodes
    V3DLONG root = 0;
    if (newrootid==-1)
    {
        for (V3DLONG i=0;i<neurons.size();i++)
            if (neurons.at(i).pn==-1){
                root = idlist.indexOf(LUT.value(neurons.at(i).n));
                break;
            }
    }
    else{
        root = idlist.indexOf(LUT.value(newrootid));
//        QList<V3DLONG> test = LUT.keys();
//        for(int i=0; i<test.size();i++){
//            qDebug()<<i<<test.at(i)<<test.indexOf(test.at(i));
//        }
//        qDebug()<<test.size()<<test.at(0)<<newrootid<<test.indexOf(newrootid);

        if (LUT.keys().indexOf(newrootid)==-1)
        {
            v3d_msg("The new root id you have chosen does not exist in the SWC file.");
            return(false);
        }
    }


    V3DLONG* neworder = new V3DLONG[siz];
    int* numbered = new int[siz];
    for (V3DLONG i=0;i<siz;i++) numbered[i] = 0;

    V3DLONG id[] = {0};

    int group[] = {1};
    DFS(matrix,neworder,root,id,siz,numbered,group);

    while (*id<siz)
    {
        V3DLONG iter;
        (*group)++;
        for (iter=0;iter<siz;iter++)
            if (numbered[iter]==0) break;
        DFS(matrix,neworder,iter,id,siz,numbered,group);
    }


    //find the point in non-group 1 that is nearest to group 1,
    //include the nearest point as well as its neighbors into group 1, until all the nodes are connected
    while((*group)>1)
    {
        double min = VOID;
        double dist2 = 0;
        int mingroup = 1;
        V3DLONG m1,m2;
        for (V3DLONG ii=0;ii<siz;ii++){
            if (numbered[ii]==1)
                for (V3DLONG jj=0;jj<siz;jj++)
                    if (numbered[jj]!=1)
                    {
                        dist2 = computeDist2(neurons.at(idlist.at(ii)),neurons.at(idlist.at(jj)));
                        if (dist2<min)
                        {
                            min = dist2;
                            mingroup = numbered[jj];
                            m1 = ii;
                            m2 = jj;
                        }
                    }
        }
        for (V3DLONG i=0;i<siz;i++)
            if (numbered[i]==mingroup)
                numbered[i] = 1;
        if (min<=thres*thres)
        {
            matrix[m1][m2] = true;
            matrix[m2][m1] = true;
        }
        (*group)--;
    }

    id[0] = 0;
    for (int i=0;i<siz;i++)
    {
        numbered[i] = 0;
        neworder[i]= VOID;
    }

    *group = 1;

    V3DLONG new_root=root;
    V3DLONG offset=0;
    while (*id<siz)
    {
        V3DLONG cnt = 0;
        DFS(matrix,neworder,new_root,id,siz,numbered,group);
        (*group)++;
        NeuronSWC S;
        S.n = offset+1;
        S.pn = -1;
        V3DLONG oripos = idlist.at(new_root);
        S.x = neurons.at(oripos).x;
        S.y = neurons.at(oripos).y;
        S.z = neurons.at(oripos).z;
        S.r = neurons.at(oripos).r;
        S.type = neurons.at(oripos).type;
        S.seg_id = neurons.at(oripos).seg_id;
        S.level = neurons.at(oripos).level;
        S.creatmode = neurons.at(oripos).creatmode;
        S.timestamp = neurons.at(oripos).timestamp;
        S.tfresindex = neurons.at(oripos).tfresindex;
        result.append(S);
        cnt++;

        for (V3DLONG ii=offset+1;ii<(*id);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
            {
                if (neworder[ii]!=VOID && neworder[jj]!=VOID && matrix[neworder[ii]][neworder[jj]] )
                {
                        NeuronSWC S;
                        S.n = ii+1;
                        S.pn = jj+1;
                        V3DLONG oripos = idlist.at(neworder[ii]);
                        S.x = neurons.at(oripos).x;
                        S.y = neurons.at(oripos).y;
                        S.z = neurons.at(oripos).z;
                        S.r = neurons.at(oripos).r;
                        S.type = neurons.at(oripos).type;
                        S.seg_id = neurons.at(oripos).seg_id;
                        S.level = neurons.at(oripos).level;
                        S.creatmode = neurons.at(oripos).creatmode;
                        S.timestamp = neurons.at(oripos).timestamp;
                        S.tfresindex = neurons.at(oripos).tfresindex;
                        result.append(S);
                        cnt++;

                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
                }
            }
        }
        for (new_root=0;new_root<siz;new_root++)
            if (numbered[new_root]==0) break;
        offset += cnt;
    }

    if ((*id)<siz) {
        v3d_msg("Error!");
        return false;
    }

    //free space by Yinan Wan 12-02-02
    if (neworder) {delete []neworder; neworder=NULL;}
    if (numbered) {delete []numbered; numbered=NULL;}
    if (matrix){
        for (V3DLONG i=0;i<siz;i++) {delete matrix[i]; matrix[i]=NULL;}
        if (matrix) {delete []matrix; matrix=NULL;}
    }


    return(true);

}
QVector< QVector<V3DLONG> > get_neighbors(QList<NeuronSWC> &neurons, const QHash<V3DLONG,V3DLONG> & LUT)
{
    // generate neighbor lists for each node, using new ids.
    // LUT (look-up table): old name -> new ids
    // ids are the line numbers
    // names are the node names (neurons.name)
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();
    int siz = idlist.size();
    QList<int> nlist;
    for(V3DLONG i=0; i<neurons.size(); i++){nlist.append(neurons.at(i).n);}

//    qDebug()<<"Before defining qvector";
    QVector< QVector<V3DLONG> > neighbors = QVector< QVector<V3DLONG> >(siz, QVector<V3DLONG>() );
//    qDebug()<<"After defining qvector";
//    system("pause");
    for (V3DLONG i=0;i<neurons.size();i++)
    {
        // Find parent node
//        qDebug()<<i;
        int pid_old = nlist.lastIndexOf(neurons.at(i).pn);
        if(pid_old<0){
            continue;  // Skip root nodes
        }
        else{
            int pname_old = neurons.at(pid_old).n;
            int cname_old = neurons.at(i).n;
            int pid_new = LUT.value(pname_old);
            int cid_new = LUT.value(cname_old);
            if((pid_new>=siz) || (cid_new>=siz)){
                v3d_msg(QString("Out of range [0, %1]: pid:%2; cid:%3").arg(siz).arg(pid_new).arg(cid_new));
            }
            // add a new neighbor for the child node
            if(!neighbors.at(cid_new).contains(pid_new)){
//                qDebug()<<QString("Adding edge between %1 and %2").arg(cid_new).arg(pid_new);
                neighbors[cid_new].push_back(pid_new);
            }
            // add a new neighbor for the parent node
            if(!neighbors.at(pid_new).contains(cid_new)){
//                qDebug()<<QString("Adding edge between %1 and %2").arg(pid_new).arg(cid_new);
                neighbors[pid_new].push_back(cid_new);
            }
        }
    }
    return neighbors;
}
QList<V3DLONG> DFS(QVector< QVector<V3DLONG> > neighbors, V3DLONG newrootid, V3DLONG siz)
{
    // siz: size of the whole neuronlist
    // The neuronlist may include multiple components
    // A component is a connected tree
    // Sorted components: other components that have already been sorted.
    // Current component: the component where newroot resides. We will sort it and append it to the sorted components

    // sorted_size: size of sorted components
    // neworder: new order of the sored components
    // *group: id of the current component

    QList<V3DLONG> neworder;

    // DFS to sort current component;

    // Initialization
    QStack<int> pstack;
    QList<int> visited;
    for(int i=0;i<siz; i++){visited.append(0);}
    visited[newrootid]=1;
    pstack.push(newrootid);
    neworder.append(newrootid);

    // Tree traverse
    bool is_push;
    int pid;
    while(!pstack.isEmpty()){
        is_push = false;
        pid = pstack.top();
        // whether exist unvisited neighbors of pid
        // if yes, push neighbor to stack;
        QVector<V3DLONG>::iterator it;
        QVector<V3DLONG> cur_neighbors = neighbors.at(pid);
        for(it=cur_neighbors.begin(); it!=cur_neighbors.end(); ++it)
        {
            if(visited.at(*it)==0)
            {
                pstack.push(*it);
                is_push=true;
                visited[*it]=1;
                neworder.append(*it);
                break;
            }
        }
        // else, pop pid
        if(!is_push){
            pstack.pop();
        }
    }
    return neworder;
}
bool CAnnotations::Sort_SWC_NewVersion(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result,V3DLONG newrootid)
{

//    newrootid=VOID;
    double thres=VOID;
    // node name list of
    QList<V3DLONG> nlist;
    for(int i=0; i<neurons.size(); i++){
        nlist.append(neurons.at(i).n);
    }

    //create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

    //create a new id list to give every different neuron a new id
    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();
    V3DLONG siz = idlist.size();

    // create a vector to keep neighbors of each node
    QVector< QVector<V3DLONG> > neighbors = get_neighbors(neurons, LUT);

    // Find the new id of the new root
    V3DLONG root = 0;
    if (newrootid==VOID)  // If unspecified, use the 1st root as new root.
    {
        for (V3DLONG i=0;i<neurons.size();i++)
            if (neurons.at(i).pn==-1){
                root = idlist.indexOf(LUT.value(neurons.at(i).n));
                break;
            }
    }
    else{
        root = idlist.indexOf(LUT.value(newrootid));
        if (LUT.keys().indexOf(newrootid)==-1)
        {
            v3d_msg("The new root id you have chosen does not exist in the SWC file.");
            return(false);
        }
    }

    //Major steps
    //do a DFS for the the matrix and re-allocate ids for all the nodes
    QList<V3DLONG> neworder;
    QList<V3DLONG> cur_neworder;
    QList<V3DLONG> component_id;
    V3DLONG sorted_size = 0;
    int cur_group = 1;

    // Begin with the new root node and
    // generate the 1st sorted tree.
    cur_neworder= DFS(neighbors, root, siz);
    sorted_size += cur_neworder.size();
    neworder.append(cur_neworder);
    for(int i=0; i<cur_neworder.size(); i++){
        component_id.append(cur_group);
    }
    cout<<"Done 1st DFS fr"<<endl;

    // Continue to sort the rest of the tree
    while (sorted_size <siz)
    {
        V3DLONG new_root;
        cur_group++;
        for (V3DLONG iter=0;iter<siz;iter++)
        {
            if (!neworder.contains(iter))
            {
                new_root = iter;
                break;
            }
        }
        cur_neworder= DFS(neighbors, new_root, siz);
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id.append(cur_group);
        }
    }
    qDebug()<<"Number of components before making connections"<<cur_group;

    QList<V3DLONG> output_newroot_list;
    if((thres != 1000000000) && (thres>0)){  // If distance threshold > 1: make new connections
        qDebug()<<"find the point in non-group 1 that is nearest to group 1";
        //find the point in non-group 1 that is nearest to group 1,
        //include the nearest point as well as its neighbors into group 1, until all the nodes are connected
        output_newroot_list.append(root);
        while(cur_group>1)
        {
            double min = VOID;
            double dist2 = 0;
            int mingroup = 1;

            // Find the closest pair of nodes between group 1 and the rest.
            V3DLONG m1,m2;
            for (V3DLONG ii=0;ii<siz;ii++)
            {
                qDebug()<<QString("Distance check: %1").arg(ii);
                if (component_id[ii]==1)
                {
                    for (V3DLONG jj=0;jj<siz;jj++)
                        if (component_id[jj]!=1)
                        {
                            dist2 = computeDist2(neurons.at(nlist.indexOf(LUT.key(ii))),
                                                 neurons.at(nlist.indexOf(LUT.key(jj))));
                            if (dist2<min)
                            {
                                min = dist2;
                                mingroup = component_id[jj];
                                m1 = ii;
                                m2 = jj;
                            }
                        }
                }
            }
            for (V3DLONG i=0;i<siz;i++)
                if (component_id[i]==mingroup)
                    component_id[i] = 1;
            if (min<=thres*thres)
            {
                qDebug()<<QString("New connection is made between %1 and %2").arg(m1).arg(m2);
                if(!neighbors.at(m1).contains(m2)){neighbors[m1].push_back(m2);}
                if(!neighbors.at(m2).contains(m1)){neighbors[m2].push_back(m1);}
            }
            else{  // set component the node closest to group 1 is root
                output_newroot_list.append(m2);
            }
            cur_group--;
        }
        qDebug()<<"Number of components after making connections"<<output_newroot_list.size();
    }
    else{
        int tp_group = 0;
        for(int i=0; i<siz; i++){
            if(component_id.at(i) != tp_group){
                output_newroot_list.append(neworder.at(i));
                tp_group = component_id.at(i);
            }
        }
    }

    // DFS sort of the neuronlist after new connections
    for (int i=0;i<siz;i++)
    {
        component_id[i] = 0;
        neworder[i]= VOID;
    }
    component_id.clear();
    neworder.clear();
    sorted_size = 0;
    cur_group = 1;

    V3DLONG offset=0;
    for(V3DLONG i=0; i<output_newroot_list.size(); i++)
    {
        V3DLONG new_root = output_newroot_list.at(i);
        qDebug()<<QString("Output component %1, root id is %2").arg(i).arg(new_root);
        V3DLONG cnt = 0;
        // Sort current component;
        cur_neworder= DFS(neighbors, new_root, siz);
        sorted_size += cur_neworder.size();
        neworder.append(cur_neworder);
        for(int i=0; i<cur_neworder.size(); i++){
            component_id.append(cur_group);
        }
        NeuronSWC S;
        S.n = offset+1;
        S.pn = -1;
        V3DLONG oriname = LUT.key(new_root);
        V3DLONG oripos = nlist.indexOf(oriname);
        S.x = neurons.at(oripos).x;
        S.y = neurons.at(oripos).y;
        S.z = neurons.at(oripos).z;
        S.r = neurons.at(oripos).r;
        S.type = neurons.at(oripos).type;
        S.seg_id = neurons.at(oripos).seg_id;
        S.level = neurons.at(oripos).level;
        S.creatmode = neurons.at(oripos).creatmode;
        S.timestamp = neurons.at(oripos).timestamp;
        S.tfresindex = neurons.at(oripos).tfresindex;
        result.append(S);
        cnt++;
        qDebug()<<QString("New root %1:").arg(i)<<S.x<<S.y<<S.z;

        for (V3DLONG ii=offset+1;ii<(sorted_size);ii++)
        {
            for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
            {
                V3DLONG cid = neworder[ii];
                V3DLONG pid = neworder[jj];
                // If there's an edge between the two nodes, then pid is the parent of cid
                if (pid!=VOID && cid!=VOID && neighbors.at(pid).contains(cid))
                {
                        NeuronSWC S;
                        S.n = ii+1;
                        oriname = LUT.key(cid);
                        oripos = nlist.indexOf(oriname);
                        S.pn = jj+1;
                        S.x = neurons.at(oripos).x;
                        S.y = neurons.at(oripos).y;
                        S.z = neurons.at(oripos).z;
                        S.r = neurons.at(oripos).r;
                        S.type = neurons.at(oripos).type;
                        S.seg_id = neurons.at(oripos).seg_id;
                        S.level = neurons.at(oripos).level;
                        S.creatmode = neurons.at(oripos).creatmode;
                        S.timestamp = neurons.at(oripos).timestamp;
                        S.tfresindex = neurons.at(oripos).tfresindex;
                        result.append(S);
                        cnt++;
                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
                }
            }
        }
        offset += cnt;
    }

    if ((sorted_size)<siz) {
        v3d_msg("Error!");
        return false;
    }

    // free space.
    neighbors.clear();
    return(true);
}
void CAnnotations::save(const char* filepath, bool removedupnode, bool as_swc) throw (RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("filepath = \"%s\"", filepath).c_str(), __itm__current__function__);

    //measure elapsed time
    QElapsedTimer timer;
    timer.start();

    //retrieving annotations
    std::list<annotation*> annotations;
    if(octree)
        octree->find(interval_t(0, octree->DIM_V), interval_t(0, octree->DIM_H), interval_t(0, octree->DIM_D), annotations);

    FILE* f;

    //saving ano file
//    v3d_msg(QString(filepath));
    QString input_ano = QString(filepath);
    if(input_ano.indexOf("/") != (-1)){
        input_ano.remove(0, input_ano.lastIndexOf("/")+1);
    }

    QString output_ano = input_ano;
    QString output_apo = output_ano + ".apo";
    QString output_swc = as_swc? output_ano+".swc":output_ano+".eswc";

//    if(filename.endsWith(".ano")){
//        filename.remove(filename.lastIndexOf(".ano"), filename.size());
//    }
//    else{
//        v3d_msg("Input is not an ano file.");
//        return;
//    }

////    v3d_msg(QString("fileprefix: %1").arg(fileprefix));

//    QString output_ano = filename;
//    QString output_apo = filename;
//    QString output_swc = filename;
//    output_ano.append(".ano");
//    output_apo.append(".apo");

//    if(as_swc){
//        output_swc.append(".swc");
//    }
//    else{
//        output_swc.append(".eswc");
//    }
    QString fileprefix(filepath);
    if(fileprefix.indexOf("/") != -1){
        fileprefix.remove(fileprefix.lastIndexOf("/")+1, fileprefix.size());
    }
    cout<<endl<<"output_ano: "<<qPrintable(fileprefix + output_ano)<<endl;
    cout<<"output_apo: "<<qPrintable(fileprefix + output_apo)<<endl;
    cout<<"output_swc: "<<qPrintable(fileprefix + output_swc)<<endl;

    f = fopen(qPrintable(fileprefix + output_ano), "w");
    if(!f)
    {
        throw RuntimeException(strprintf("in CAnnotations::save(): cannot save to path \"%s\"", qPrintable(fileprefix + output_ano)));
    }
    else{
        fprintf(f, "APOFILE=%s\n", qPrintable(output_apo));
        fprintf(f, "SWCFILE=%s\n", qPrintable(output_swc));
        fclose(f);
    }

#ifdef _YUN_
	//if (V3dR_GLWidget::surfaceDlg) cout << V3dR_GLWidget::surfaceDlg->getMarkerNum() << endl;
	myRenderer_gl1* thisRenderer = myRenderer_gl1::cast(static_cast<Renderer_gl1*>(CViewer::getCurrent()->view3DWidget->getRenderer()));
	for (QList<ImageMarker>::iterator it = thisRenderer->listMarker.begin(); it != thisRenderer->listMarker.end(); ++it)
	{
		ImageMarker currMarker;
		float convertedX = CViewer::getCurrent()->coord2global<float>(it->x, iim::horizontal, false, -1, false, false, __itm__current__function__);
		float convertedY = CViewer::getCurrent()->coord2global<float>(it->y, iim::vertical, false, -1, false, false, __itm__current__function__);
		float convertedZ = CViewer::getCurrent()->coord2global<float>(it->z, iim::depth, false, -1, false, false, __itm__current__function__);
		std::list<annotation*>* annoPtrList = octree->find(convertedX, convertedY, convertedZ);
		if (annoPtrList != nullptr)
		{
			if (annoPtrList->size() == 1) annoPtrList->front()->name = it->name.toStdString();
			else
			{
				if (it->name == "duplicated") continue;
				else
				{
					for (std::list<annotation*>::iterator annoIt = annoPtrList->begin(); annoIt != annoPtrList->end(); ++annoIt)
						(*annoIt)->name = "duplicated";
					annoPtrList->front()->name = it->name.toStdString();
				}
			}
		}
	}
#endif

    //saving apo (point cloud) file
    QList<CellAPO> points;
    for(std::list<annotation*>::iterator i = annotations.begin(); i!= annotations.end(); i++)
        if((*i)->type == 0)     //selecting markers
        {
            CellAPO cell;
            cell.n = (*i)->ID;
            cell.name = (*i)->name.c_str();
            cell.comment = (*i)->comment.c_str();
            cell.x = (*i)->x;
            cell.y = (*i)->y;
            cell.z = (*i)->z;
            cell.volsize = (*i)->r*(*i)->r*4*terafly::pi;
            cell.color = (*i)->color;
            points.push_back(cell);
        }
    writeAPO_file(fileprefix + output_apo, points);

    //saving SWC file
    f = fopen(qPrintable(fileprefix + output_swc), "w");
    fprintf(f, "#name undefined\n");
    fprintf(f, "#comment terafly_annotations\n");

	cout << "Annotation size: " << annotations.size() << endl;
    if(removedupnode)
    {
        // Peng Xie 2019-04-23
        // Find new soma node in the current annotation
        long soma_found_new = 0;
        double soma_x_new = -1.1;
        double soma_y_new = -1.1;
        double soma_z_new = -1.1;
        for(std::list<annotation*>::iterator i = annotations.begin(); i != annotations.end(); i++)
        {
            if((*i)->type == 1) //selecting NeuronSWC
            {
                if((*i)->subtype == 1){ // soma found in current annotation
                    if(soma_found_new == 0){
                        soma_found_new ++;
                        soma_x_new = (*i)->x;
                        soma_y_new = (*i)->y;
                        soma_z_new = (*i)->z;
                    }
                    else{
                        if(((*i)->x != soma_x_new) || ((*i)->y != soma_y_new) || ((*i)->x != soma_y_new)){
                            soma_found_new ++;
                        }
                    }
                }
            }
        }

        // Decide which soma location to use.
        if(soma_found_new == 0){     // Case 1: If soma_found_new == 0, try using the old soma location
            qDebug()<<"'Remove dup and save': Using soma from the 'original' annotation.";
        }
        else{
            qDebug()<<"'Remove dup and save': Using soma from the 'edited' annotation.";
            SOMA_FOUND = soma_found_new;
            SOMA_X = soma_x_new;
            SOMA_Y = soma_y_new;
            SOMA_Z = soma_z_new;
        }
        if(SOMA_FOUND>1){      // Case 2: If soma_found_new > 0, use the new soma location
            v3d_msg("Multiple node locations typed as soma in your input swc.\n"
                    "If root of output swc does not match with the real soma,\n"
                    "please double check your input swc.\n"
                    );
        }

        // Create the neuron tree to be sorted.
        QList<NeuronSWC> nt,nt_sort;
        long countNode=0;
        long soma_ct = 0;
        long soma_name = -1;
        for(std::list<annotation*>::iterator i = annotations.begin(); i != annotations.end(); i++)
        {
            if((*i)->type == 1) //selecting NeuronSWC
            {
                countNode++;
                NeuronSWC temp;
                temp.n=(*i)->ID;
                temp.type=(*i)->subtype;
                temp.x=(*i)->x;
                temp.y=(*i)->y;
                temp.z=(*i)->z;
                temp.r=(*i)->r;
                temp.parent=(*i)->parent ? (*i)->parent->ID : -1;
                temp.pn=(*i)->parent ? (*i)->parent->ID : -1;
                temp.seg_id=0;
                temp.level=(*i)->level;
                temp.creatmode=(*i)->creatmode;
                temp.timestamp=(*i)->timestamp;
                temp.tfresindex=(*i)->tfresindex;
                if((temp.x == SOMA_X) && (temp.y == SOMA_Y) && (temp.z == SOMA_Z))  // Use soma node as root, if any.
                {
                    soma_ct++;
                    if(soma_ct==1)
                    {
                        soma_name = (*i)->ID;
                    }
                }
                nt.append(temp);
            }
        }
        cout<<"nt size is "<<nt.size()<<endl;
        NeuronTree tp;
        tp.listNeuron = nt;

        // Sorting
        if(soma_name == -1){
            v3d_msg("No soma detected in the input swc.\n"
                    "If root of output swc does not match with the real soma,\n"
                    "please double check your input swc.\n"
                    );
            Sort_SWC_NewVersion(nt,nt_sort,VOID);
        }
        else{
            Sort_SWC_NewVersion(nt,nt_sort,soma_name);
        }
        cout<<"nt_sort size is "<<nt_sort.size()<<endl;

        // Saving
        if(as_swc){
            fprintf(f, "#n type x y z radius parent\n");
            for(V3DLONG countNode=0;countNode<nt_sort.size();countNode++)
            {
                NeuronSWC cur_node = nt_sort.at(countNode);
                fprintf(f, "%lld %d %.3f %.3f %.3f %.3f %lld\n",
                        cur_node.n, cur_node.type,
                        cur_node.x, cur_node.y, cur_node.z,
                        cur_node.r, cur_node.parent);
            }
        }
        else{
            fprintf(f, "#n type x y z radius parent seg_id level mode timestamp TFresindex\n");
            for(V3DLONG countNode=0;countNode<nt_sort.size();countNode++)
            {
                NeuronSWC cur_node = nt_sort.at(countNode);
                fprintf(f, "%lld %d %.3f %.3f %.3f %.3f %lld %lld %lld %d %.0f %d\n",
                        cur_node.n, cur_node.type,
                        cur_node.x, cur_node.y, cur_node.z,
                        cur_node.r, cur_node.parent,
                        cur_node.level, cur_node.creatmode,
                        cur_node.timestamp, cur_node.tfresindex
                        );
            }
        }
        v3d_msg(qPrintable("De-duplicated swc saved: " + fileprefix + output_swc));
    }
    else
    {
        if(as_swc){
            fprintf(f, "#n type x y z radius parent seg_id level mode timestamp TFresindex\n");
            for(std::list<annotation*>::iterator i = annotations.begin(); i != annotations.end(); i++)
            {
                if((*i)->type == 1) //selecting NeuronSWC
                {
                    fprintf(f, "%lld %d %.3f %.3f %.3f %.3f %lld %lld %lld %d %.0f %d\n", (*i)->ID, (*i)->subtype, (*i)->x, (*i)->y, (*i)->z, (*i)->r,
                            (*i)->parent ? (*i)->parent->ID : -1, 0, (*i)->level, (*i)->creatmode, (*i)->timestamp, (*i)->tfresindex);
                }
            }
        }
        else{
            fprintf(f, "#n type x y z radius parent seg_id level mode timestamp TFresindex\n");
            for(std::list<annotation*>::iterator i = annotations.begin(); i != annotations.end(); i++)
            {
                if((*i)->type == 1) //selecting NeuronSWC
                {
                    fprintf(f, "%lld %d %.3f %.3f %.3f %.3f %lld %lld %lld %d %.0f %d\n", (*i)->ID, (*i)->subtype, (*i)->x, (*i)->y, (*i)->z, (*i)->r,
                            (*i)->parent ? (*i)->parent->ID : -1, 0, (*i)->level, (*i)->creatmode, (*i)->timestamp, (*i)->tfresindex);
                }
            }
        }

    }

    fclose(f);//file closing

    PLog::instance()->appendOperation(new AnnotationOperation("save annotations: save .ano to disk", tf::IO, timer.elapsed()));
}
void CAnnotations::deleteOldAnnotations(const char *filepath)throw (RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("filepath = \"%s\"", filepath).c_str(), __itm__current__function__);
    std::ifstream f(filepath);
    if(!f.is_open())
        throw RuntimeException(strprintf("in CAnnotations::load(): cannot load file \"%s\"", filepath));

    // read line by line
    for (std::string line; std::getline(f, line); )
    {
        std::vector < std::string > tokens;
        terafly::split(line, "=", tokens);
        if(tokens.size() != 2)
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): cannot parse line \"%s\"",filepath,line.c_str()));

        QDir dir(filepath);
        dir.cdUp();
        if(tokens[0].compare("APOFILE") == 0)
        {
            QString apofilepath=dir.absolutePath().append("/").append(tf::clcr(tokens[1]).c_str());
            std::remove(apofilepath.toStdString().c_str());
            //cout<<"test apo file path "<<apofilepath.toStdString()<<endl;
        }
        else if(tokens[0].compare("SWCFILE") == 0)
        {
            QString swcfilepath=dir.absolutePath().append("/").append(tf::clcr(tokens[1]).c_str());
            std::remove(swcfilepath.toStdString().c_str());
            //cout<<"test swc file path "<<swcfilepath.toStdString()<<endl;
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): unable to recognize file type \"%s\"", filepath, tokens[0].c_str()));
    }
    f.close();
    std::remove(filepath);
}

void CAnnotations::load(const char* filepath) throw (RuntimeException)
{
    SOMA_FOUND = 0;
    /**/tf::debug(tf::LEV1, strprintf("filepath = \"%s\"", filepath).c_str(), __itm__current__function__);

    //precondition checks
    if(!octree)
        throw RuntimeException("CAnnotations::load(): octree not yet initialized");

    //measure elapsed time
    QElapsedTimer timer;
    timer.start();

    //clearing annotations
    this->clear();

    // open ANO file
    std::ifstream f(filepath);
    if(!f.is_open())
        throw RuntimeException(strprintf("in CAnnotations::load(): cannot load file \"%s\"", filepath));

    // read line by line
    for (std::string line; std::getline(f, line); )
    {
        std::vector < std::string > tokens;
        terafly::split(line, "=", tokens);
        if(tokens.size() != 2)
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): cannot parse line \"%s\"",filepath,line.c_str()));

        QDir dir(filepath);
        dir.cdUp();
        if(tokens[0].compare("APOFILE") == 0)
        {
            QList <CellAPO> cells = readAPO_file(dir.absolutePath().append("/").append(tf::clcr(tokens[1]).c_str()));
            for(QList <CellAPO>::iterator i = cells.begin(); i!= cells.end(); i++)
            {
                annotation* ann = new annotation();
                ann->type = 0;
                ann->name = i->name.toStdString();
                ann->comment = i->comment.toStdString();
                ann->color = i->color;
                ann->r = sqrt(i->volsize / (4*terafly::pi));
                ann->x = i->x;
                ann->y = i->y;
                ann->z = i->z;
                octree->insert(*ann);
            }
            //printf("--------------------- teramanager plugin >> inserted %d markers\n", cells.size());
        }
        else if(tokens[0].compare("SWCFILE") == 0)
        {
            NeuronTree nt = readSWC_file(dir.absolutePath().append("/").append(tf::clcr(tokens[1]).c_str()));
//            v3d_msg(QString("Test_1.1. Input swc is: %1").arg(dir.absolutePath().append("/").append(tf::clcr(tokens[1]).c_str())));
            std::map<int, annotation*> annotationsMap;
            std::map<int, NeuronSWC*> swcMap;
            for(QList <NeuronSWC>::iterator i = nt.listNeuron.begin(); i!= nt.listNeuron.end(); i++)
            {
                annotation* ann = new annotation();
                ann->type = 1;
                ann->name = nt.name.toStdString();
                // Peng Xie added, 2019-01-24
//                if((!soma_found) && (i->pn == -1) && (i->type == 1)){
                if(i->type == 1){
//                    v3d_msg(QString("Soma found: %1 %2 %3 %4").arg(i->n).arg(i->x).arg(i->y).arg(i->z));
                    if(SOMA_FOUND == 0){
                        SOMA_X = i->x;  // If multiple soma found, throw a warning and use the 1st one for sorting.
                        SOMA_Y = i->y;
                        SOMA_Z = i->z;
                        SOMA_FOUND = 1;
                    }
                    else if ((SOMA_X != i->x) || (SOMA_Y != i->y) || (SOMA_Z != i->z))  // Multiple soma found
                    {
                        SOMA_FOUND ++;
                    }
                }
                ann->comment = nt.comment.toStdString();
                ann->color = nt.color;
                ann->subtype = i->type;
                ann->r = i->r;
                ann->x = i->x;
                ann->y = i->y;
                ann->z = i->z;
                ann->level = i->level;
                ann->creatmode = i->creatmode;
                ann->timestamp = i->timestamp;
                ann->tfresindex = i->tfresindex;
                ann->vaa3d_n = i->n;
                octree->insert(*ann);
                annotationsMap[i->n] = ann;
                swcMap[i->n] = &(*i);
            }
            for(std::map<int, annotation*>::iterator i = annotationsMap.begin(); i!= annotationsMap.end(); i++)
            {
				if(i->second == NULL)
				{
					qDebug()<<"i->second is NULL";
					continue;
				}

                i->second->parent = swcMap[i->first]->pn == -1 ? 0 : annotationsMap[swcMap[i->first]->pn];
                if(i->second->parent)
                {
                    #ifdef terafly_enable_debug_annotations
                    tf::debug(tf::LEV_MAX, strprintf("Add %lld(%.0f, %.0f, %.0f) to %lld(%.0f, %.0f, %.0f)'s children list\n",
                                                       i->second->ID, i->second->x, i->second->y, i->second->z, i->second->parent->ID,
                                                       i->second->parent->x, i->second->parent->y, i->second->parent->z).c_str(), 0, true);
                    #endif

                    i->second->parent->children.insert(i->second);
                }
            }
        }
        else
            throw RuntimeException(strprintf("in CAnnotations::load(const char* filepath = \"%s\"): unable to recognize file type \"%s\"", filepath, tokens[0].c_str()));
    }
    f.close();

    PLog::instance()->appendOperation(new AnnotationOperation("load annotations: read .ano from disk", tf::IO, timer.elapsed()));
}

/*********************************************************************************
* Conversion from VTK to APO files
**********************************************************************************/
void CAnnotations::convertVtk2APO(std::string vtkPath, std::string apoPath) throw (tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("vtkPath = \"%s\", apoPath = \"%s\"", vtkPath.c_str(), apoPath.c_str()).c_str(), __itm__current__function__);

    // open file and check header
    FILE* f = fopen(vtkPath.c_str(), "r");
    if(!f)
        throw tf::RuntimeException(tf::strprintf("in CAnnotations::convertVtk2APO(): cannot open file at \"%s\"", vtkPath.c_str()));
    char lineBuffer[1024];
    char stringBuffer[1024];
    int points_n;
    if(!fgets(lineBuffer, 1024, f))
        throw tf::RuntimeException(tf::strprintf("Cannot read 1st line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw tf::RuntimeException(tf::strprintf("Cannot read 2nd line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw tf::RuntimeException(tf::strprintf("Cannot read 3rd line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw tf::RuntimeException(tf::strprintf("Cannot read 4th line in \"%s\"", vtkPath.c_str()));
    if(!fgets(lineBuffer, 1024, f))
        throw tf::RuntimeException(tf::strprintf("Cannot read 5th line in \"%s\"", vtkPath.c_str()));
    if(sscanf(lineBuffer, "%s %d", stringBuffer, &points_n) != 2)
        throw tf::RuntimeException(tf::strprintf("5th line (\"%s\") is not in the format <string> <number> <string> as expected, in \"%s\"", lineBuffer, vtkPath.c_str()));
    if(strcmp(stringBuffer, "POINTS") != 0)
        throw tf::RuntimeException(tf::strprintf("expected \"POINTS\" at 5th line (\"%s\"), in \"%s\"", lineBuffer, vtkPath.c_str()));

    // read cells
    QList<CellAPO> cells;
    for(int i=0; i<points_n; i++)
    {
        if(!fgets(lineBuffer, 1024, f))
            throw tf::RuntimeException(tf::strprintf("Cannot read %d-th line in \"%s\"", i, vtkPath.c_str()));
        float x=0,y=0,z=0;
        if(sscanf(lineBuffer, "%f %f %f", &x, &y, &z) != 3)
            throw tf::RuntimeException(tf::strprintf("%d-th line (\"%s\") is not in the format <float> <float> <float> as expected, in \"%s\"", i, lineBuffer, vtkPath.c_str()));


        CellAPO cell;
        cell.n = i;
        cell.name = "na";
        cell.comment = "na";
        cell.x = x;
        cell.y = y;
        cell.z = z;
        cell.volsize = 4*terafly::pi;
        cell.color.r = cell.color.g = 0;
        cell.color.b = 255;
        cells.push_back(cell);
    }
    fclose(f);

    // save to APO file
    writeAPO_file(apoPath.c_str(), cells);

    // save output ano
    std::string anoPath = tf::cdUp(apoPath) + "/" + tf::getFileName(apoPath, false) + ".ano";
    f = fopen(anoPath.c_str(), "w");
    if(!f)
        throw RuntimeException(tf::strprintf("cannot save .ano to path \"%s\"", anoPath.c_str()));
    fprintf(f, "APOFILE=%s\n",tf::getFileName(apoPath).c_str());
    fclose(f);
}

/*********************************************************************************
* Conversion from VTK to APO files
**********************************************************************************/
void CAnnotations::convertMaMuT2APO(std::string MaMuTPath, std::string apoPath) throw (tf::RuntimeException)
{
    QList<CellAPO> cells;
    int count = 0;

    // check xml
    TiXmlDocument xml(MaMuTPath.c_str());
    if(!xml.LoadFile())
        throw tf::RuntimeException(tf::strprintf("unable to open MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
    TiXmlElement *pElem = xml.FirstChildElement("TrackMate");
    if(!pElem)
        throw tf::RuntimeException(tf::strprintf("cannot find node <TrackMate> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
    pElem = pElem->FirstChildElement("Model");
    if(!pElem)
        throw tf::RuntimeException(tf::strprintf("cannot find node <Model> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
    pElem = pElem->FirstChildElement("AllSpots");
    if(!pElem)
        throw tf::RuntimeException(tf::strprintf("cannot find node <AllSpots> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
    int nspots = 0;
    pElem->QueryIntAttribute("nspots", &nspots);
    if(nspots == 0)
        throw tf::RuntimeException(tf::strprintf("no spots found in <AllSpots> node in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));

    // parse spots
    TiXmlElement *frameElem = pElem->FirstChildElement("SpotsInFrame");
    while(frameElem)
    {
        int frame_id = 0;
        if(frameElem->QueryIntAttribute("frame", &frame_id) != TIXML_SUCCESS)
            throw tf::RuntimeException(tf::strprintf("failed to query node <SpotsInFrame> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
        if(frame_id != 0)
        {
            static bool first_time = true;
            if(first_time)
            {
                v3d_msg("Spots within frame != 0 will be ignored");
                first_time = false;
            }
            continue;
        }

        pElem = frameElem->FirstChildElement("Spot");
        if(!pElem)
            throw tf::RuntimeException(tf::strprintf("failed to find first node <Spot> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));

        while(pElem)
        {
            CellAPO c;
            if(pElem->QueryFloatAttribute("POSITION_X", &c.x) != TIXML_SUCCESS)
                throw tf::RuntimeException(tf::strprintf("failed to query attribute \"POSITION_X\" from node <Spot> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
            if(pElem->QueryFloatAttribute("POSITION_Y", &c.y) != TIXML_SUCCESS)
                throw tf::RuntimeException(tf::strprintf("failed to query attribute \"POSITION_Y\" from node <Spot> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
            if(pElem->QueryFloatAttribute("POSITION_Z", &c.z) != TIXML_SUCCESS)
                throw tf::RuntimeException(tf::strprintf("failed to query attribute \"POSITION_Z\" from node <Spot> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
            if(pElem->QueryFloatAttribute("RADIUS", &c.volsize) != TIXML_SUCCESS)
                throw tf::RuntimeException(tf::strprintf("failed to query attribute \"RADIUS\" from node <Spot> in MaMuT xml file at \"%s\"", MaMuTPath.c_str()));
            c.volsize = (4.0/3.0)*tf::pi*c.volsize*c.volsize*c.volsize;
            c.color.r = c.color.g = 0;
            c.color.b = 255;
            cells.push_back(c);
            pElem = pElem->NextSiblingElement("Spot");
            count++;
        }
        frameElem = frameElem->NextSiblingElement("SpotsInFrame");
    }

    // save output apo
    writeAPO_file(apoPath.c_str(), cells);

    // save output ano
    std::string anoPath = tf::cdUp(apoPath) + "/" + tf::getFileName(apoPath, false) + ".ano";
    FILE* f = fopen(anoPath.c_str(), "w");
    if(!f)
        throw RuntimeException(tf::strprintf("cannot save .ano to path \"%s\"", anoPath.c_str()));
    fprintf(f, "APOFILE=%s\n",tf::getFileName(apoPath).c_str());
    fclose(f);

    QMessageBox::information(0, "Info", tf::strprintf("Successfully written %d markers from %d entries", cells.size(), count).c_str());
}

/*********************************************************************************
* Compute type I and type II errors between two APO files
**********************************************************************************/
std::pair<int, int>                                             // return pair<false positives, false negatives>
    tf::CAnnotations::typeIandIIerrorAPO(std::string apo1Path, // first apo file path (assumed as TRUTH)
                                          std::string apo2Path, // second apo file to be compared
                                          int d,                // maximum distance between a finding that matches with a truth
                                          std::string filter,   // filter cells in apo2 by name
                                          const std::string & outputPath /*=*/)
throw (tf::RuntimeException)
{
    // read cells
    QList<CellAPO> truth = readAPO_file(apo1Path.c_str());
    QList<CellAPO> findings = readAPO_file(apo2Path.c_str());

    QList<CellAPO> errors;

    // false positives: cells in 'findings' but not in 'truth'
    std::pair<int, int> out;
    out.first = out.second = 0;
    for(int j=0; j<findings.size(); j++)
    {
        //QMessageBox::information(0, "Title", tf::strprintf("findings[j].name = \"%s\", filter = \"%s\"", tf::cls(findings[j].name.toStdString()).c_str(), filter.c_str()).c_str());
        if(filter.compare("none") != 0 && tf::cls(findings[j].name.toStdString()).compare(filter) != 0)
            continue;

        bool found = false;
        for(int i=0; i<truth.size() && !found; i++)
        {
            if(distance(truth[i], findings[j]) <= static_cast<float>(d))
                found = true;
        }
        if(!found)
        {
            out.first++;
            findings[j].color.r = 255;
            findings[j].color.g = 0;
            findings[j].color.b = 0;
            findings[j].comment = "false_positive";
            errors.push_back(findings[j]);
        }
    }

    // false negatives: cells in 'truth' whose distance from all cells in 'findings' is higher than d
    for(int i=0; i<truth.size(); i++)
    {
        bool found = false;
        for(int j=0; j<findings.size() && !found; j++)
        {
            if(filter.compare("none") != 0 && tf::cls(findings[j].name.toStdString()).compare(filter) != 0)
                continue;

            if(distance(truth[i], findings[j]) <= static_cast<float>(d))
                found = true;
        }
        if(!found)
        {
            out.second++;

            truth[i].color.r = 0;
            truth[i].color.g = 0;
            truth[i].color.b = 255;
            truth[i].comment = "false_negative";
            errors.push_back(truth[i]);
        }
    }

    // save output apo
    if(outputPath.size())
    {
        writeAPO_file(outputPath.c_str(), errors);

        // save output ano
        std::string anoPath = tf::cdUp(outputPath) + "/" + tf::getFileName(outputPath, false) + ".ano";
        FILE* f = fopen(anoPath.c_str(), "w");
        if(!f)
            throw RuntimeException(tf::strprintf("cannot save .ano to path \"%s\"", anoPath.c_str()));
        fprintf(f, "APOFILE=%s\n",tf::getFileName(outputPath).c_str());
        fclose(f);
    }

    return out;
}

/*********************************************************************************
* Diff between two APO files
**********************************************************************************/
void CAnnotations::diffAPO( std::string apo1Path,               // first apo file path (assumed as truth)
                            std::string apo2Path,               // second apo file path
                            int x0 /*=0*/, int x1 /*=-1*/,      // VOI [x0, x1) in the global reference sys
                            int y0 /*=0*/, int y1 /*=-1*/,      // VOI [y0, y1) in the global reference sys
                            int z0 /*=0*/, int z1 /*=-1*/,      // VOI [z0, z1) in the global reference sys
                            std::string diffPath)               // path where the difference apo file (containing only FPs and FNs) has to be stored (optional)
throw (tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, strprintf("apo1Path = \"%s\", apo2Path = \"%s\", x0 = %d, x1 =%d, y0 = %d, y1 = %d, z0 = %d, z1 = %d",
                                        apo1Path.c_str(), apo2Path.c_str(), x0, x1, y0, y1, z0, z1).c_str(), __itm__current__function__);

    // parse default parameters
    x1 = x1 > 0 ? x1 : std::numeric_limits<int>::max();
    y1 = y1 > 0 ? y1 : std::numeric_limits<int>::max();
    z1 = z1 > 0 ? z1 : std::numeric_limits<int>::max();

    // read cells
    QList<CellAPO> cells1 = readAPO_file(apo1Path.c_str());
    QList<CellAPO> cells2 = readAPO_file(apo2Path.c_str());

    // detect false negatives (in BLUE)
    QList<CellAPO> diff_cells;
    for(int i=0; i<cells1.size(); i++)
    {
        bool found = false;
        for(int j=0; j<cells2.size() && !found; j++)
        {
            if(cells1[i].x == cells2[j].x && cells1[i].y == cells2[j].y && cells1[i].z == cells2[j].z)
                found = true;
        }
        if(!found)
        {
            CellAPO cell;
            cell.x = cells1[i].x;
            cell.y = cells1[i].y;
            cell.z = cells1[i].z;
            cell.color.b = 255;
            cell.color.r = cell.color.g = 0;
            diff_cells.push_back(cell);
        }
    }

    // detect false positives (in RED)
    for(int j=0; j<cells2.size(); j++)
    {
        bool found = false;
        for(int i=0; i<cells1.size() && !found; i++)
        {
            if(cells1[i].x == cells2[j].x && cells1[i].y == cells2[j].y && cells1[i].z == cells2[j].z)
                found = true;
        }
        if(!found)
        {
            CellAPO cell;
            cell.x = cells2[j].x;
            cell.y = cells2[j].y;
            cell.z = cells2[j].z;
            cell.color.r = 255;
            cell.color.b = cell.color.g = 0;
            diff_cells.push_back(cell);
        }
    }

    // write output
    if(!diffPath.empty())
        writeAPO_file(diffPath.c_str(), diff_cells);

    // insert cells into the same octree. In this way, it is much faster to compute TPs, FPs and FNs
    /*Octree* diff_octree = new Octree(std::numeric_limits<uint32>::max(), std::numeric_limits<uint32>::max(), std::numeric_limits<uint32>::max());
    std::vector<annotation*> ano1, ano2;
    tf::uint64 cell1count = 0;
    for(int i=0; i<cells1.size(); i++)
    {
        if( cells1[i].x >= x0 && cells1[i].x < x1 &&    //
            cells1[i].y >= y0 && cells1[i].y < y1 &&    // cell is within selected VOI
            cells1[i].z >= z0 && cells1[i].z < z1)      //
        {

            annotation* cell = new annotation;
            cell->type = 0;
            cell->subtype = 0;
            cell->x = cells1[i].x;
            cell->y = cells1[i].y;
            cell->z = cells1[i].z;
            ano1.push_back(cell);
            diff_octree->insert(*cell);
            cell1count++;
        }
    }
    tf::uint64 cell2count = 0;
    for(int i=0; i<cells2.size(); i++)
    {
        if( cells2[i].x >= x0 && cells2[i].x < x1 &&    //
            cells2[i].y >= y0 && cells2[i].y < y1 &&    // cell is within selected VOI
            cells2[i].z >= z0 && cells2[i].z < z1)      //
        {
            annotation* cell = new annotation;
            cell->type = 0;
            cell->subtype = 0;
            cell->x = cells2[i].x;
            cell->y = cells2[i].y;
            cell->z = cells2[i].z;
            ano2.push_back(cell);
            diff_octree->insert(*cell);
            cell2count++;
        }
    }

    // count false positives, true positives, and false negatives    
    QList<CellAPO> diff_cells;
    tf::uint64 FPs = 0;
    tf::uint64 TPs = 0;
    tf::uint64 FNs = 0;
    for(int i=0; i<ano2.size(); i++)
        if( static_cast<CAnnotations::Octree::octant*>(ano2[i]->container)->annotations.size() == 1)
        {
            CellAPO cell;
            cell.n = ano2[i]->ID;
            cell.name = ano2[i]->name.c_str();
            cell.comment = ano2[i]->comment.c_str();
            cell.x = ano2[i]->x;
            cell.y = ano2[i]->y;
            cell.z = ano2[i]->z;
            cell.volsize = ano2[i]->r*ano2[i]->r*4*teramanager::pi;
            cell.color.r = 255;
            cell.color.g = 0;
            cell.color.b = 0;
            diff_cells.push_back(cell);
            FPs++;
        }
        else
            TPs++;
    for(int i=0; i<ano1.size(); i++)
        if( static_cast<CAnnotations::Octree::octant*>(ano1[i]->container)->annotations.size() == 1)
        {
            CellAPO cell;
            cell.n = ano2[i]->ID;
            cell.name = ano2[i]->name.c_str();
            cell.comment = ano2[i]->comment.c_str();
            cell.x = ano2[i]->x;
            cell.y = ano2[i]->y;
            cell.z = ano2[i]->z;
            cell.volsize = ano2[i]->r*ano2[i]->r*4*teramanager::pi;
            cell.color.r = 0;
            cell.color.g = 0;
            cell.color.b = 255;
            diff_cells.push_back(cell);
            FNs++;
        }

    // release memory for octree
    delete diff_octree;

    // save diff apo (point cloud) file
//    diffPath = "C:/diff.apo";
    if(!diffPath.empty())
        writeAPO_file(diffPath.c_str(), diff_cells);

    // display result
    std::string message = tf::strprintf(   "VOI = X[%d,%d), Y[%d,%d), Z[%d,%d)\n"
                                            "#Cells (from truth): %d\n"
                                            "#Cells (from .apo) : %d\n"
                                            "#TPs: %lld\n"
                                            "#FPs: %lld\n"
                                            "#FNs: %lld\n"
                                            "TPR: %.3f\n"
                                            "FPR: %.3f\n",
                                            x0, x1, y0, y1, z0, z1,
                                            cell1count,
                                            cell2count,
                                            TPs,
                                            FPs,
                                            FNs,
                                            (TPs+0.1f)/(TPs+FNs),
                                            (FPs+0.1f)/(cell2count));
    QMessageBox::information(0, "Result", message.c_str());*/
}


/*********************************************************************************
* Trim APO files
**********************************************************************************/
void CAnnotations::trimAPO( std::string inputPath,  // input apo file path
                            std::string outputPath,                   // where output apo file is saved
                            int x0 /*=0*/, int x1 /*=-1*/,    // VOI [x0, x1) in the global reference sys
                            int y0 /*=0*/, int y1 /*=-1*/,    // VOI [y0, y1) in the global reference sys
                            int z0 /*=0*/, int z1 /*=-1*/)    // VOI [z0, z1) in the global reference sys
throw (tf::RuntimeException)
{
    // parse default parameters
    x1 = x1 > 0 ? x1 : std::numeric_limits<int>::max();
    y1 = y1 > 0 ? y1 : std::numeric_limits<int>::max();
    z1 = z1 > 0 ? z1 : std::numeric_limits<int>::max();

    // read cells
    QList<CellAPO> cells = readAPO_file(inputPath.c_str());

    // select only cells within VOI
    QList<CellAPO> cells_VOI;
    for(int i=0; i<cells_VOI.size(); i++)
        if(cells_VOI[i].x >= x0 && cells_VOI[i].x < x1 && cells_VOI[i].y >= y0 && cells_VOI[i].y < y1 && cells_VOI[i].z >= z0 && cells_VOI[i].z < z1)
            cells_VOI.push_back(cells_VOI[i]);

    // save output apo
    writeAPO_file(outputPath.c_str(), cells_VOI);

    // save output ano
    std::string anoPath = tf::cdUp(outputPath) + "/" + tf::getFileName(outputPath, false) + ".ano";
    FILE* f = fopen(anoPath.c_str(), "w");
    if(!f)
        throw RuntimeException(tf::strprintf("cannot save .ano to path \"%s\"", anoPath.c_str()));
    fprintf(f, "APOFILE=%s\n",tf::getFileName(outputPath).c_str());
    fclose(f);
}

/*********************************************************************************
* Label duplicates in APO files
**********************************************************************************/
void tf::CAnnotations::labelDuplicates(
                            std::string inputPath,  // input apo file path
                            std::string outputPath, // where output apo file is saved
                            int d,                  // maximum distance between 2 duplicates
                            RGBA8 color)            // VOI [y0, y1) in the global reference sys
throw (tf::RuntimeException)
{
    // read cells
    QList<CellAPO> cells = readAPO_file(inputPath.c_str());

    // label duplicates with the given color
    for(int i=0; i<cells.size(); i++)
        for(int j=0; j<cells.size(); j++)
            if(i != j && distance(cells[i], cells[j]) <= d)
                cells[i].color = cells[j].color = color;

    // write cells
    writeAPO_file(outputPath.c_str(), cells);

    // save output ano
    std::string anoPath = tf::cdUp(outputPath) + "/" + tf::getFileName(outputPath, false) + ".ano";
    FILE* f = fopen(anoPath.c_str(), "w");
    if(!f)
        throw RuntimeException(tf::strprintf("cannot save .ano to path \"%s\"", anoPath.c_str()));
    fprintf(f, "APOFILE=%s\n",tf::getFileName(outputPath).c_str());
    fclose(f);
}

/*********************************************************************************
* Counts markers having distance <= d from each other
**********************************************************************************/
tf::uint32 CAnnotations::countDuplicateMarkers(int d) throw (tf::RuntimeException)
{
    std::list<annotation*> nodes;
    octree->find(tf::interval_t(0, std::numeric_limits<int>::max()),
                 tf::interval_t(0, std::numeric_limits<int>::max()),
                 tf::interval_t(0, std::numeric_limits<int>::max()), nodes);

    tf::uint32 count = 0;
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
        for(std::list<annotation*>::iterator j = nodes.begin(); j != nodes.end(); j++)
            if(i!=j && distance(*i, *j) <= ((float)d))
            {
                (*i)->color.r = 255;
                (*i)->color.b = (*i)->color.g = 0;
                count++;
                break;
            }
    return count;
}

/*********************************************************************************
*
**********************************************************************************/
void CAnnotations::diffnAPO(QStringList apos,         // inputs
                    std::string outputPath)         // where output apo file is saved
throw (tf::RuntimeException)
{
    // get distinct colors
    std::vector<RGBA8> colors = tf::CImageUtils::distinctColors(apos.size());

    // insert cells into the same octree. In this way, it is much faster to compute the XOR
    Octree* xor_octree = new Octree(std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    for(int f=0; f<apos.size(); f++)
    {
        QList<CellAPO> cells = readAPO_file(apos[f]);
        for(int i=0; i<cells.size(); i++)
        {
            annotation* cell = new annotation(cells[i]);
            cell->name = tf::getFileName(apos[f].toStdString(), false);
            cell->color = colors[f];
            xor_octree->insert(*cell);
        }
    }

    // retrieve all nodes
    std::list<annotation*> nodes;
    xor_octree->find(tf::interval_t(0, std::numeric_limits<int>::max()),
                 tf::interval_t(0, std::numeric_limits<int>::max()),
                 tf::interval_t(0, std::numeric_limits<int>::max()), nodes);

    // only take cells from nodes where at least one .apo disagrees, i.e. nodes that do not contain apos.size() cells
    QList<CellAPO> output_cells;
    for(std::list<annotation*>::iterator i = nodes.begin(); i != nodes.end(); i++)
        if( static_cast<CAnnotations::Octree::octant*>((*i)->container)->annotations.size() != apos.size())
            output_cells.push_back((*i)->toCellAPO());

    // save output apo
    writeAPO_file(outputPath.c_str(), output_cells);

    // save output ano
    std::string anoPath = tf::cdUp(outputPath) + "/" + tf::getFileName(outputPath, false) + ".ano";
    FILE* f = fopen(anoPath.c_str(), "w");
    if(!f)
        throw RuntimeException(tf::strprintf("cannot save .ano to path \"%s\"", anoPath.c_str()));
    fprintf(f, "APOFILE=%s\n",tf::getFileName(outputPath).c_str());
    fclose(f);
}

/*********************************************************************************
* Merge .xml ImageJ Cell Counter markers files into .APO
**********************************************************************************/
void CAnnotations::mergeImageJCellCounterXMLs(QStringList xmls,  // inputs
                    std::string apoPath, // where output apo file is saved
                    int xS, int yS, int zS, // blocks size
                    int overlap /*=0*/,     // blocks overlap
                    int x0 /*=0*/,          // (0,0,0) block X-coordinate
                    int y0 /*=0*/,          // (0,0,0) block Y-coordinate
                    int z0 /*=0*/)          // (0,0,0) block Z-coordinate
throw (tf::RuntimeException)
{
    // parse xmls
    QList<CellAPO> cells;
    int count = 0;
    for(int f=0; f<xmls.size(); f++)
    {
        // get unique block ID
        std::string fname = tf::getFileName(xmls[f].toStdString(), true);
        fname.erase(fname.find_first_of("."));
        fname = fname.substr(fname.size()-3);
        if(fname.size() != 3)
            throw tf::RuntimeException(tf::strprintf("\"%s\" does not end with 3 digits xyz", xmls[f].toStdString().c_str()));
        int ix = fname[0] - '0';
        int iy = fname[1] - '0';
        int iz = fname[2] - '0';

        // lot of checks
        TiXmlDocument xml(xmls[f].toStdString().c_str());
        if(!xml.LoadFile())
            throw tf::RuntimeException(tf::strprintf("unable to load xml file at \"%s\"", xmls[f].toStdString().c_str()));
        TiXmlElement *pElem = xml.FirstChildElement("CellCounter_Marker_File");
        if(!pElem)
            throw tf::RuntimeException(tf::strprintf("cannot find node <CellCounter_Marker_File> in xml file at \"%s\"", xmls[f].toStdString().c_str()));
        pElem = pElem->FirstChildElement("Image_Properties");
        if(!pElem)
            throw tf::RuntimeException(tf::strprintf("cannot find node <Image_Properties> in xml file at \"%s\"", xmls[f].toStdString().c_str()));
        pElem = pElem->FirstChildElement("Image_Filename");
        if(!pElem)
            throw tf::RuntimeException(tf::strprintf("cannot find node <Image_Filename> in xml file at \"%s\"", xmls[f].toStdString().c_str()));
        if(fname.compare(pElem->GetText()) != 0)
            throw tf::RuntimeException(tf::strprintf("<Image_Filename> mismatch (found \"%s\", expected \"%s\") in xml file at \"%s\"", pElem->GetText(), fname.c_str(), xmls[f].toStdString().c_str()));

        pElem = xml.FirstChildElement("CellCounter_Marker_File");
        pElem = pElem->FirstChildElement("Marker_Data");
        if(!pElem)
            throw tf::RuntimeException(tf::strprintf("cannot find node <Marker_Data> in xml file at \"%s\"", xmls[f].toStdString().c_str()));
        pElem = pElem->FirstChildElement("Marker_Type");
        if(!pElem)
            throw tf::RuntimeException(tf::strprintf("cannot find node <Marker_Type> in xml file at \"%s\"", xmls[f].toStdString().c_str()));
        pElem = pElem->FirstChildElement("Marker");
        while(pElem)
        {
            CellAPO c;
            c.x = x0 + ix*xS - ix*overlap + tf::str2num<int>(pElem->FirstChildElement("MarkerX")->GetText());
            c.y = y0 + iy*yS - iy*overlap + tf::str2num<int>(pElem->FirstChildElement("MarkerY")->GetText());
            c.z = z0 + iz*zS - iz*overlap + tf::str2num<int>(pElem->FirstChildElement("MarkerZ")->GetText());
            c.color.r = c.color.g = 0;
            c.color.b = 255;
            if(cells.indexOf(c) == -1)
                cells.push_back(c);
            pElem = pElem->NextSiblingElement("Marker");
            count++;
        }
    }

    // save output apo
    writeAPO_file(apoPath.c_str(), cells);

    // save output ano
    std::string anoPath = tf::cdUp(apoPath) + "/" + tf::getFileName(apoPath, false) + ".ano";
    FILE* f = fopen(anoPath.c_str(), "w");
    if(!f)
        throw RuntimeException(tf::strprintf("cannot save .ano to path \"%s\"", anoPath.c_str()));
    fprintf(f, "APOFILE=%s\n",tf::getFileName(apoPath).c_str());
    fclose(f);

    QMessageBox::information(0, "Info", tf::strprintf("Successfully written %d markers from %d entries", cells.size(), count).c_str());
}

/*********************************************************************************
* Converts the octree to a NeuronTree. This is actually a draw method.
**********************************************************************************/
NeuronTree CAnnotations::Octree::toNeuronTree() throw (tf::RuntimeException)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    // unlimited octrees are not supported
    if(DIM_V == std::numeric_limits<int>::max() || DIM_H == std::numeric_limits<int>::max() || DIM_D == std::numeric_limits<int>::max())
        throw tf::RuntimeException("Cannot draw unbounded octree. Please deactivate the \"Unlimited\" option for the annotation space size and re-open the image");

    NeuronTree nt;
    nt.editable = false;
    nt.linemode = true;
    _rec_to_neuron_tree(root, nt.listNeuron);

    /**/tf::debug(tf::LEV2, "Successfully generated the Neuron Tree", __itm__current__function__);

    return nt;
}
