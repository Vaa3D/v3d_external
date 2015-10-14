/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




// head file FL_treeMatching.h
// F. Long
// 20090112

#ifndef __TREE_MATCHING__
#define __TREE_MATCHING__

#include <string>

#include "FL_swcTree.h"
#include "../atlas_builder/FL_registerAffine.h"

using namespace std;

//----------------------
// similarity functions
//----------------------
float calsim_node_distance(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2); //calculate similarity between two nodes in terms of Eucldiean distance
float calsim_branch_length_ratio(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2); // compute the branching length ratio of two nodes in two trees and compute the distance  
float *calsim_allbranch_length_ratio(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2, unsigned char method); // compute the similarity vector of branching length ratio of two nodes in two trees  
float *calsim_allbranch_length_ratio(float *lengthratio1, float *lengthratio2, unsigned char lengthratioNum1, unsigned char lengthratioNum2); // compute the similarity vector of branching length ratio of two nodes in two trees  
float calsim_length_weighted_distance(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2); // compute the similatiry between nodeid1 and nodeid2, in terms of the length-weighted-Euclidean distance

//----------------------------------------------------------------------------------------------
// temporary, still has bugs in it, removed from FL_treeMatching.cpp on 20090401
//----------------------------------------------------------------------------------------------
void sigBranchingPointMatching(swcTree *Tree1, swcTree *Tree2, V3DLONG branchSizeThre, V3DLONG *&matchingList,  V3DLONG &sigBranchPointNum, float &simMeasure); //match significant branching points in two trees

//-----------------------
// registration function
//-----------------------
void affineTransformTrees(swcTree *subjectTree, swcTree *targetTree, V3DLONG *controlPoints, V3DLONG controlPointsNum); //register (global affine) subjectTree against targetTree using control points

//--------------------------------------------------------------------------------------------
// find all combination of possible matches of sub-branches of two nodes in Tree1 and Tree2
//--------------------------------------------------------------------------------------------

void findPossibleMatchList(V3DLONG childrenNum1_Tree1, V3DLONG childrenNum2_Tree2, bool submatchTag, V3DLONG *&possibleMatchList, V3DLONG &num, V3DLONG &N_num, V3DLONG &R_num);

//------------------------------------------
// generate tree matching result for Graphviz 
//------------------------------------------
void genMatchingGraphvizFile(swcTree *Tree1, swcTree *Tree2, V3DLONG *matchingList, char *outfilename);

//-----------------------
// latest tree matching functions
//-----------------------

void updateRemoveBranchTag(swcTree *Tree_old_sn, swcTree *Tree_sn, swcTree *Tree_old, V3DLONG deleteNodeID, V3DLONG ancesterID, V3DLONG **remveBranchTag, V3DLONG **&removeBranchTagNew);

// latest version of treeMatching_noContinualNodes_lengthRatioAngle_simDependent
// see function description in FL_treeMatching.cpp for details
bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, bool b_scale, V3DLONG *&matchingList, float &simMeasure, float *lengthRatioThre, float subtreeRatioThre, float weight_lengthRatio, float weight_angle);

// matching leaf nodes, this function is called by treeMatching_hierarchical, and work together with treeMatching_noContinualNodes_lengthRatioAngle_simDependent
bool treeMatching_leafNodes(swcTree *Tree1, swcTree *Tree2, float *parentVector1, float *parentVector2, 
							V3DLONG *falseLeafNodeList, V3DLONG falseLeafNodeNum, float *parentPathLength, float lengthDiffThre, unsigned char method, 
							float weight_lengthRatio, float weight_angle,
							V3DLONG *&matchedLeafNodeList, unsigned char &matchedLeafNodeNum, float &simMeasure);

// tree matching using hierarchical DP scheme 
bool treeMatching_hierarchical(swcTree *Tree1, swcTree *Tree2, bool b_scale, V3DLONG *&matchingList, float &simMeasure, string outfilenamedir); 

//-----------------------
// OLD FUNCTIONS
//-----------------------

//// matching leaf nodes, this function is called by treeMatching_hierarchical, and work together with treeMatching_noContinualNodes_lengthRatioAngle_simDependent
//bool treeMatching_leafNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *falseLeafNodeList, V3DLONG falseLeafNodeNum, float *parentPathLength, float thre, V3DLONG *&matchedLeafNodeList, unsigned char &matchedLeafNodeNum, float &simMeasure); 
//
//bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent_superNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure, 
//																			float *lengthRatioThre, float subtreeRatioThre, float weight_lengthRatio, 
//																			float weight_angle, float branchingNodeDisRatioThre);
//
// earlier version of treeMatching_noContinualNodes_lengthRatioAngle_simDependent, which is no longer kept
// match two trees, the similarity metrics are both length ratio and angles
//bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, V3DLONG *&branchMatchingList, float &simMeasure, float lengthThre1, float lengthThre2); 

// earlier version of treeMatching_noContinualNodes_lengthRatioAngle_simDependent, which is no longer kept
//match two trees, the similarity metrics are both length ratio and angles
//bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure, float lengthThre1, float lengthThre2); 

// earlier version of treeMatching_noContinualNodes_lengthRatioAngle_simDependent, which is no longer kept
//bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure, float *lengthRatioThre, V3DLONG depthThre);

//
////tree matching using dynamic programming, the very early version that works, nodeNumThre=0 for the same neurons, nodeNumThre=2 for different neurons
//bool treeMatching(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure); 
//
//// match two trees, every node (root, continual nodes, branching nodes, leaf nodes will all find matches), suitable for matching neurons of the same structures
//bool treeMatching_allNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure); 
//
//// match two trees, root, branching nodes, leaf nodes will find matches, continual nodes will not find matches, suitable for matching neurons of different structures
//bool treeMatching_noContinualNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure); 
//
////match two trees, the similarity metrics (length-ratio) are dependent between parent and children, continual and leaf nodes are removed from matching
//bool treeMatching_noContinualLeafNodes_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure); 
//
////match two trees, the similarity metrics (length-ratio) are dependent between parent and children, continual nodes are removed from matching
//// use legnth ratio as similarity score
//bool treeMatching_noContinualNodes_lengthRatio_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure); 


#endif
