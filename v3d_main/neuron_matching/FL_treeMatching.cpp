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




// isomorphism tree matching using dynamic programming
// F. Long
// 20090108

#undef DEBUG

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string>

#include "FL_treeMatching.h"
#include "mymatrix.cpp"
#include "munkres.cpp"
#include "./combination/combination.h" 
using namespace std;
using namespace stdcomb;

// **************************************************
// generate tree matching result for Graphviz
// **************************************************

void genMatchingGraphvizFile(swcTree *Tree1, swcTree *Tree2, V3DLONG *matchingList, char *outfilename)
{
	V3DLONG i;
	
	FILE *file;
	file = fopen(outfilename, "wt");
	
	fprintf(file,"digraph untitled\n");
	fprintf(file,"{\n");
	
	// plot Tree1	
	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		if (Tree1->node[i].parentid!=-1)
		{
			fprintf(file,"a%d->a%d;\n", Tree1->node[i].parentid, Tree1->node[i].nid);
		}
	}
	
	// plot Tree2	
	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		if (Tree2->node[i].parentid!=-1)
		{
			fprintf(file,"b%d->b%d;\n", Tree2->node[i].parentid, Tree2->node[i].nid);
		}
	}	
	
	// plot the matching lines between Tree1 and Tree2
	for (i = 0; i<Tree1->treeNodeNum; i++)
	{
		if (matchingList[i]>0)
			//			fprintf(file, "a%d-b%d\n", Tree1->node[i].nid, matchingList[i]);
			//			fprintf(file, "a%d-b%d [color=\"red\", len=3]\n", Tree1->node[i].nid, matchingList[i]);
			fprintf(file, "a%d->b%d [style=dotted, dir=both, minlen=5]\n", Tree1->node[i].nid, matchingList[i]);
		
	}
	
	fprintf(file, "}\n");
	
	fclose(file);
	
}


// ******************************************************************
// function to calculate similarity between two nodes, 
// one in Tree1 and the other in Tree2, in terms of Eucldiean distance
// ******************************************************************

float calsim_node_distance(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2)
{
	float sim;
	float thre = 30; 
//	float thre = 15; 

	float dist; 
	
	V3DLONG nodeidx1, nodeidx2;

	Tree1->getIndex(nodeid1, nodeidx1);
	Tree2->getIndex(nodeid2, nodeidx2);
		
	dist = sqrt((Tree1->node[nodeidx1].x - Tree2->node[nodeidx2].x)*(Tree1->node[nodeidx1].x - Tree2->node[nodeidx2].x) +
			    (Tree1->node[nodeidx1].y - Tree2->node[nodeidx2].y)*(Tree1->node[nodeidx1].y - Tree2->node[nodeidx2].y) +
			    (Tree1->node[nodeidx1].z - Tree2->node[nodeidx2].z)*(Tree1->node[nodeidx1].z - Tree2->node[nodeidx2].z));
//	printf("dist = %f \n", dist);

//	sim = 1.0/dist; // not good

//	sim = 9999-dist; // not good
	
	if (dist>thre)
		sim = -1;
	else
		sim = 1- 2*dist/thre;
	
	return sim;


	// compute various spatial features	
	// 1. distance between nodes for the simplest case
	// 2. length between the current node and its children
	// 3. paralleness
	// refer to retina paper for their similarity measures
	// need to normalize among different terms
	// how to normalize in different layers
	
//	// distance between nodes in tree1 and tree2
//	float sim, thre, tmp;
//	float thre; 
//	V3DLONG idx1, idx2;
//	V3DLONG i,j;
// 	V3DLONG nodeNum1, nodeNum2;
	
}


// *************************************************************************************
// compute the branching length ratio of two nodes in two trees and compute the distance 
// *************************************************************************************

float calsim_branch_length_ratio(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2)
{
	float *lengthratio1 =0, *lengthratio2 = 0;
	unsigned char lengthratioNum1, lengthratioNum2;
	float sim = -1, dist;
//	float thre = 200;
//	float thre = 1;
	float thre = 10;
	
	// compute branch length ratio
	
	unsigned char method = 0; // method should be less than 4
	Tree1->computeBranchLengthRatio(nodeid1, lengthratio1, lengthratioNum1, method);
	Tree2->computeBranchLengthRatio(nodeid2, lengthratio2, lengthratioNum2, method);
	
	if (((lengthratio1[0]==0) && (lengthratio1[1]==0)) || ((lengthratio2[0]==0) && (lengthratio2[1]==0)))
	{
		printf("nodeid1 %d or nodeid2 %d or both are not branching points\n", nodeid1, nodeid2);
		sim = -1;
	}
	else
	{
		if ((method==0)||(method ==2))
		{
			dist = sqrt((lengthratio1[0]-lengthratio2[0])*(lengthratio1[0]-lengthratio2[0]) +
						(lengthratio1[1]-lengthratio2[1])*(lengthratio1[1]-lengthratio2[1]));
		}
		
		if (method == 1)
		{
			dist = sqrt((lengthratio1[0]-lengthratio2[0])*(lengthratio1[0]-lengthratio2[0]) +
						(lengthratio1[1]-lengthratio2[1])*(lengthratio1[1]-lengthratio2[1]) +
						(lengthratio1[2]-lengthratio2[2])*(lengthratio1[2]-lengthratio2[2]) +
						(lengthratio1[3]-lengthratio2[3])*(lengthratio1[3]-lengthratio2[3]));

		}
		
		if (method ==3)
		{
			dist = fabs(lengthratio1[0]-lengthratio2[0]);
		}
		
//		printf("dist= %f\n", dist);
		 
		if (dist>thre)
			sim = -1;
		else
			sim = 1- 2*dist/thre;
			
//		printf("sim= %f, dist=%f, lengthratio1[0]=%f, lengthratio1[1]=%f,lengthratio2[0]=%f, lengthratio2[1]=%f ", sim, dist, lengthratio1[0], lengthratio1[1], lengthratio1[0], lengthratio1[1]);				
	}	
	
	return sim;

}


// *************************************************************************************
// override function calsim_branch_length_ratio above
// compute the similarity vector of branching length ratio of two nodes in two trees 
// *************************************************************************************

float *calsim_allbranch_length_ratio(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2, unsigned char method)
{
	V3DLONG i,j;
	
	float *lengthratio1 =0, *lengthratio2 = 0;
	unsigned char lengthratioNum1, lengthratioNum2;
	float thre = 10;
	
	// compute branch length ratio
	
	Tree1->computeBranchLengthRatio(nodeid1, lengthratio1, lengthratioNum1, method);
	Tree2->computeBranchLengthRatio(nodeid2, lengthratio2, lengthratioNum2, method);
	
	// get the numbeer of children of nodeid1, nodeid2
	V3DLONG *childrenList1=0, *childrenList2=0, *childrenNodeIdx1=0, *childrenNodeIdx2=0, childrenNum1, childrenNum2;
	 
	Tree1->getDirectChildren(nodeid1, childrenList1, childrenNodeIdx1, childrenNum1);
	Tree2->getDirectChildren(nodeid2, childrenList2, childrenNodeIdx2, childrenNum2);
	
	if (childrenList1) {delete []childrenList1; childrenList1=0;}
	if (childrenList2) {delete []childrenList2; childrenList2=0;}
	if (childrenNodeIdx1) {delete []childrenNodeIdx1; childrenNodeIdx1=0;}
	if (childrenNodeIdx2) {delete []childrenNodeIdx2; childrenNodeIdx2=0;}

	float *sim = 0, *dist=0;
	sim = new float [childrenNum1*childrenNum2];
	dist = new float [childrenNum1*childrenNum2];
	
	for (i=0; i<childrenNum1; i++)
	for (j=0; j<childrenNum2; j++)
	{
		dist[i*childrenNum2+j] = fabs(lengthratio1[i]-lengthratio2[j]);
		
		if (dist[i*childrenNum2+j]>thre)
			sim[i*childrenNum2+j] = -1;
		else
			sim[i*childrenNum2+j] = 1- 2*dist[i*childrenNum2+j]/thre;
		
	}
	
	if (dist) {delete []dist; dist = 0;}
	
	return sim;

}


// *************************************************************************************
// override function calsim_branch_length_ratio above
// compute the similarity vector of branching length ratio of two nodes in two trees 
// *************************************************************************************

float *calsim_allbranch_length_ratio(float *lengthratio1, float *lengthratio2, unsigned char lengthratioNum1, unsigned char lengthratioNum2)
{
	V3DLONG i,j;
//	float thre = 1; // before 20090415	
//	float thre = 0.3; // 20090415, match leaf nodes, consider all possibilities

	float thre = 20; // 20100128, match leaf nodes, consider all possibilities

	float *sim = 0, *dist=0;
	
	sim = new float [lengthratioNum1*lengthratioNum2];
	dist = new float [lengthratioNum1*lengthratioNum2];
	
	for (i=0; i<lengthratioNum1; i++)
	for (j=0; j<lengthratioNum2; j++)
	{
		dist[i*lengthratioNum2+j] = fabs(lengthratio1[i]-lengthratio2[j]);
		
		if (dist[i*lengthratioNum2+j]>thre)
			sim[i*lengthratioNum2+j] = -1;
		else
			sim[i*lengthratioNum2+j] = 1- 2*dist[i*lengthratioNum2+j]/thre;
		
	}
	
	if (dist) {delete []dist; dist = 0;}
	
	return sim;

}

// *************************************************************************************
// compute the similarity vector of branching angles of two nodes in two trees
// *************************************************************************************

void calsim_allbranch_angles(float *angles1, float *angles2, V3DLONG branchNum1, V3DLONG branchNum2, float *&sim_parent, float *&sim_children)
{
	V3DLONG i,j;
	float thre = 180;
	
	if ((branchNum1==0)&&(branchNum2==0)) // if the two nodes are both leaf nodes
	{
		sim_parent = new float [1];
		sim_children = new float [1];
		sim_parent[0] = 1;
		sim_children[0] = 1;
	}
	
	if ((branchNum1==0)&&(branchNum2!=0)) // if ndoe1 is leaf node
	{
		sim_parent = new float [branchNum2];
		float *dist_parent = new float [branchNum2];
		
		
		// compute similarities between parent angles
		for (j=0; j<branchNum2; j++)
		{
			dist_parent[j] = angles2[j];
				
			if (dist_parent[j]>thre)
				sim_parent[j] = -1;
			else
				sim_parent[j] = 1- 2*dist_parent[j]/thre;	
		}
		
		//compute similarities between children angles
		V3DLONG branchlen2 = branchNum2*(branchNum2-1)/2;
		sim_children = new float [branchlen2];
		float *dist_children = new float [branchlen2];
				
		for (j=0; j<branchlen2; j++)
		{
			dist_children[j] = angles2[j+branchNum2];
			
			if (dist_children[j]>thre)
				sim_children[j] = -1;
			else
				sim_children[j] = 1- 2*dist_children[j]/thre;
			
		}	

		// delete pointers		
		if (dist_parent) {delete []dist_parent; dist_parent=0;}
		if (dist_children) {delete []dist_children; dist_children=0;}
	}
	
	if ((branchNum1!=0)&&(branchNum2==0)) // if ndoe2 is leaf node
	{
		sim_parent = new float [branchNum1];
		float *dist_parent = new float [branchNum1];
		
		
		// compute similarities between parent angles
		for (j=0; j<branchNum1; j++)
		{
			dist_parent[j] = angles1[j];
				
			if (dist_parent[j]>thre)
				sim_parent[j] = -1;
			else
				sim_parent[j] = 1- 2*dist_parent[j]/thre;	
		}
		
		//compute similarities between children angles
		V3DLONG branchlen1 = branchNum1*(branchNum1-1)/2;
		sim_children = new float [branchlen1];
		float *dist_children = new float [branchlen1];
				
		for (j=0; j<branchlen1; j++)
		{
			dist_children[j] = angles1[j+branchNum1];
			
			if (dist_children[j]>thre)
				sim_children[j] = -1;
			else
				sim_children[j] = 1- 2*dist_children[j]/thre;
			
		}		

		// delete pointers
		if (dist_parent) {delete []dist_parent; dist_parent=0;}
		if (dist_children) {delete []dist_children; dist_children=0;}

	}
	
	if ((branchNum1!=0)&&(branchNum2!=0)) // if the two nodes are not leaf nodes
	{
	
//		if (branchNum1 ==0)
//			branchNum1 = 1;
//		
//		if (branchNum2 ==0)
//			branchNum2 = 1;

		float *dist_parent=0;			
		V3DLONG parent_len = branchNum1*branchNum2;
		sim_parent = new float [parent_len];
		dist_parent = new float [parent_len];
		
		float *dis_children=0;
		V3DLONG children_len = (branchNum1*(branchNum1-1)/2)*(branchNum2*(branchNum2-1)/2);
		sim_children = new float [children_len];
		float *dist_children = new float [children_len];
		
		// compute similarities between parent angles
		for (i=0; i<branchNum1; i++)
		for (j=0; j<branchNum2; j++)
		{
			dist_parent[i*branchNum2+j] = fabs(angles1[i]-angles2[j]);
				
			if (dist_parent[i*branchNum2+j]>thre)
				sim_parent[i*branchNum2+j] = -1;
			else
				sim_parent[i*branchNum2+j] = 1- 2*dist_parent[i*branchNum2+j]/thre;	
		}
		
		//compute similarities between children angles
		V3DLONG branchlen1 = branchNum1*(branchNum1-1)/2;
		V3DLONG branchlen2 = branchNum2*(branchNum2-1)/2;
		
		for (i=0; i<branchlen1; i++)
		for (j=0; j<branchlen2; j++)
		{
			dist_children[i*branchlen2+j] = fabs(angles1[i+branchNum1]-angles2[j+branchNum2]);
			
			if (dist_children[i*branchlen2+j]>thre)
				sim_children[i*branchlen2+j] = -1;
			else
				sim_children[i*branchlen2+j] = 1- 2*dist_children[i*branchlen2+j]/thre;
			
		}
		
		// delete pointers
		if (dist_parent) {delete []dist_parent; dist_parent = 0;}
		if (dist_children) {delete []dist_children; dist_children = 0;}
	}
	

}


// *************************************************************************************
// compute the similatiry between nodeid1 and nodeid2, in terms of the length-weighted-Euclidean distance 
// *************************************************************************************

float calsim_length_weighted_distance(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2)
{
	float lengthWeightedDis1 =0, lengthWeightedDis2 = 0;
	float sim = -1, dist;
//	float thre = 200;
//	float thre = 1;
	float thre = 10;
	
	// compute branch length ratio
	
	unsigned char method = 0;
	
	Tree1->computeLengthWeightedDistanceNormalized(nodeid1, lengthWeightedDis1, method);
	Tree2->computeLengthWeightedDistanceNormalized(nodeid2, lengthWeightedDis2, method);

	dist = fabs(lengthWeightedDis1-lengthWeightedDis2);
			
//		printf("dist= %f\n", dist);
		 
	if (dist>thre)
		sim = -1;
	else
		sim = 1- 2*dist/thre;
			
//		printf("sim= %f, dist=%f, lengthratio1[0]=%f, lengthratio1[1]=%f,lengthratio2[0]=%f, lengthratio2[1]=%f ", sim, dist, lengthratio1[0], lengthratio1[1], lengthratio1[0], lengthratio1[1]);				
	
	return sim;

}


// *************************************************************************************
// register (global affine) subjectTree against targetTree using control points
// controlNodeIDs: P pairs of corresponding points, with P*2 elements, the first element 
// of each pair is the nodeid serving as the control point in the subjectTree, 
// and the second element is the nodeid severing as the control point of the targetTree
// num_cpts is the number of control point pairs
// The result override subjectTree
// *************************************************************************************

void affineTransformTrees(swcTree *subjectTree, swcTree *targetTree, V3DLONG *controlNodeIDs, V3DLONG num_cpts)
{

	unsigned char ndims = 3;
	float *subject = new float [subjectTree->treeNodeNum*ndims];
	float *target  = new float [targetTree->treeNodeNum*ndims];
	float *cpt_subject = new float [num_cpts*ndims];
	float *cpt_target = new float [num_cpts*ndims];
	
	V3DLONG i;
	
	// assign values to subject, target, cpt_subject, cpt_target
	for (i=0; i<subjectTree->treeNodeNum; i++)
	{
		V3DLONG j = i*ndims;
		subject[j] = subjectTree->node[i].x;
		subject[j+1] = subjectTree->node[i].y;
		subject[j+2] = subjectTree->node[i].z;
	}
	
//	for (i=0; i<targetTree->treeNodeNum; i++)
//	{
//		V3DLONG j = i*ndims;
//		targetTree[j] = targetTree->node[i].x;
//		targetTree[j+1] = targetTree->node[i].y;
//		targetTree[j+2] = targetTree->node[i].z;
//	}

	for (i=0; i<num_cpts; i++)
	{
		V3DLONG idx;
		V3DLONG j = i*ndims;
		V3DLONG p = i*2;
		
		subjectTree->getIndex(controlNodeIDs[p], idx);
		
		cpt_subject[j] = subjectTree->node[idx].x;
		cpt_subject[j+1] = subjectTree->node[idx].y;
		cpt_subject[j+2] = subjectTree->node[idx].z;

		targetTree->getIndex(controlNodeIDs[p+1], idx);

		cpt_target[j] = targetTree->node[idx].x;
		cpt_target[j+1] = targetTree->node[idx].y;
		cpt_target[j+2] = targetTree->node[idx].z;
	}
	
	// do affine transformation
	registerAffine(subject, subjectTree->treeNodeNum, cpt_subject, cpt_target, num_cpts, ndims);
//	registerAffine(cpt_subject, num_cpts, cpt_subject, cpt_target, num_cpts, ndims);

	// assign values to subjectTree
	for (i=0; i<subjectTree->treeNodeNum; i++)
	{
		V3DLONG j = i*ndims;
		subjectTree->node[i].x = subject[j];
		subjectTree->node[i].y = subject[j+1];
		subjectTree->node[i].z = subject[j+2];
	}
	
	if (subject) {delete []subject; subject=0;}
	if (target) {delete []target; target=0;}
	if (cpt_subject) {delete []cpt_subject; cpt_subject=0;}
	if (cpt_target) {delete []cpt_target; cpt_target=0;}
	
}

// *************************************************************************************
// This program is called by treeMatching_noContinualNodes_lengthRatioAngle_simDependent_superNodes()
// to update removeBranchTag generated by removeInsignificantNodes(), when generating supernodes
// note that remveBranchTagNew and remveBranchTag correspond to Trees before generating supernodes and after 
// (i.e., nodes with deleteNodeID is deleted and and its children become children of its ancester indicated
// by ancesterID). For simpliciaty, remveBranchTagNew and remveBranchTag have the same number of nodes
// one of the node in remveBranchTagNew has entries all equal to INVALID_VALUE
// *************************************************************************************

void updateRemoveBranchTag(swcTree *Tree_old_sn, swcTree *Tree_sn, swcTree *Tree_old, V3DLONG deleteNodeID, V3DLONG ancesterID, V3DLONG **removeBranchTag, V3DLONG **&removeBranchTagNew)
{
	V3DLONG i,j,m,n;
	
	V3DLONG *removeBranchTagNew1d = new V3DLONG [Tree_old->treeNodeNum*MAX_CHILDREN_NUM];
	
	if (!removeBranchTagNew1d)
	{ 
		printf("fail to allocate memory for removeBranchTagNew1d in updateRemoveBranchTag()\n");
		return;
	}
	
//	if (!new2dpointer(removeBranchTagNew, MAX_CHILDREN_NUM, Tree_old->treeNodeNum, removeBranchTagNew1d))
//	{
//		printf("fail to allocate memory for removeBranchTagNew in updateRemoveBranchTag()\n");
//		return;		 
//	}

	// cannot call new2dpointer here, thus rewrite the function here
	if (removeBranchTagNew!=0) {printf("removeBranchTagNew is not 0, return\n"); exit (1);}
	removeBranchTagNew = new V3DLONG * [Tree_old->treeNodeNum];
	if (!removeBranchTagNew) {exit (1);}
	
	for (i=0; i<Tree_old->treeNodeNum;i++)
	{
		removeBranchTagNew[i] = (V3DLONG *)removeBranchTagNew1d + i*MAX_CHILDREN_NUM;
	}
  
	// copy removeBranchTag to removeBranchTagNew
	for (i=0; i<Tree_old->treeNodeNum; i++)
	for (j=0; j<MAX_CHILDREN_NUM; j++)
	{
		removeBranchTagNew[i][j] = removeBranchTag[i][j];
	}
	
	if ((deleteNodeID != INVALID_VALUE) && (ancesterID != INVALID_VALUE)) // otherwise, no need to update
	{
		
		// update the values of removeBranchTagNew at deleteNodeID and ancesterID
		V3DLONG ancesteridx;
		Tree_old->getIndex(ancesterID,ancesteridx);
		
		V3DLONG *childrenID_Tree_sn =0, *childrenNodeIdx_Tree_sn =0, childrenNum_Tree_sn;
		V3DLONG *childrenID_Tree_old_sn =0, *childrenNodeIdx_Tree_old_sn =0, childrenNum_Tree_old_sn;
		
		Tree_sn->getDirectChildren(ancesterID, childrenID_Tree_sn, childrenNodeIdx_Tree_sn, childrenNum_Tree_sn); // get the direct children of nodeid in new tree
		Tree_old_sn->getDirectChildren(ancesterID, childrenID_Tree_old_sn, childrenNodeIdx_Tree_old_sn, childrenNum_Tree_old_sn); // get the direct children of nodeid in old tree

		
		for (j=0; j<childrenNum_Tree_old_sn; j++)
		{
		
			swcTree *subtree;
			V3DLONG *subtreeNodeIdx;
			
			Tree_old_sn->getSubTree(childrenID_Tree_old_sn[j], subtree, subtreeNodeIdx);
			
			m= 0;
			while (m<subtree->treeNodeNum)
			{
				n = 0;
				while (n< childrenNum_Tree_sn)
				{
					if (subtree->node[m].nid == childrenID_Tree_sn[n])
					{	
						removeBranchTag[ancesteridx][j] = n; // not removed
						break;
					}
					n++;
				}
				
				if (removeBranchTag[ancesteridx][j] >= 0)
					break;
				else
					m++;
			}

			if (removeBranchTag[ancesteridx][j] == INVALID_VALUE)
				removeBranchTag[ancesteridx][j] = -1; // the branch is removed
			
			if (subtree) {delete subtree; subtree =0;}
			if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx =0;}
				
		}
		
		if (childrenID_Tree_old_sn) {delete []childrenID_Tree_old_sn; childrenID_Tree_old_sn=0;}
		if (childrenNodeIdx_Tree_old_sn) {delete []childrenNodeIdx_Tree_old_sn; childrenNodeIdx_Tree_old_sn=0;}		
		if (childrenID_Tree_sn) {delete []childrenID_Tree_sn; childrenID_Tree_sn=0;}
		if (childrenNodeIdx_Tree_sn) {delete []childrenNodeIdx_Tree_sn; childrenNodeIdx_Tree_sn=0;}
	}
}


// *************************************************************************************
// compute all possible combinations of matched branches of Tree1 and Tree2 at a pair of nodes
// Input: childrenNum1_Tree1 and childrenNum2_Tree2, the number of children of the particular node
//        in Tree1 and of the particular node in Tree2
//	      submatchTag: a boolean tag indicating whether submatch is allowed. For instance, 
//        branch 0,1,2,3 in Tree1, branch 0',1',2' in Tree2. submatchTag = 0 means 0',1',2' all
//	      need to find corresponding match in Tree1. submatchTag =1 means sub-combinations of 
//        0',1',2', e.g. (0',1') can be matched while leaving 2' out
// output: possibleMatchList, all possible combintations of matched branches. The total number is
//         sum(C(n,n-k)C(m,n-k)P(n-k))
//         num: number of all possible combinations
//         N_num: number of branches in the tree with bigger number of branches
//         R_num: number of branches in the tree with smaller number of branches
// *************************************************************************************

void findPossibleMatchList(V3DLONG childrenNum1_Tree1, V3DLONG childrenNum2_Tree2, bool submatchTag, V3DLONG *&possibleMatchList,V3DLONG &num, V3DLONG &N_num, V3DLONG &R_num)
{

	V3DLONG m,n,p,k;
	V3DLONG *childrenOrder1_Tree1, *childrenOrder2_Tree2;	
	V3DLONG *N_list=0, *R_list=0;
	V3DLONG r_num, *r_list=0, *r1_list=0, *r2_list=0;
	V3DLONG cnt=-1, cnt2;	

	
	// get N_num and R_num, which are the maximum number of branches in the two trees,
	// note that N_num>=R_num
	if ((childrenNum1_Tree1==0)||(childrenNum2_Tree2==0))
	{
		num = 0; N_num = 0; R_num = 0; possibleMatchList = 0;
		return;
	}
	if ((childrenNum1_Tree1>0)&&(childrenNum2_Tree2>0)) //otherwise, it is not necessary to match
	{
		if (childrenNum1_Tree1>childrenNum2_Tree2)
		{
		
			N_num = childrenNum1_Tree1;
			R_num = childrenNum2_Tree2;		
		}
		else
		{
			N_num = childrenNum2_Tree2;
			R_num = childrenNum1_Tree1;
		}

		N_list = new V3DLONG [N_num];
		R_list = new V3DLONG [R_num];
		
		for (m=0; m<N_num; m++)
			N_list[m] = m;
		
		for (m=0; m<R_num; m++)
			R_list[m] = m;
						

		// compute total number of possible combintations
		// num: sum(C(m,n-k)*C(n,n-k)*P(n-k)) =sum(m!/((m-n+k)!(n-k)!)*n!/(k!(n-k)!)*(n-k)!) = sum(m!n!/((m-n+k)!(n-k)!k!));
		// where m=N_num, n=R_num, k takes value from 0 to n-1
							
		num = 0; // number of possible combinations
		V3DLONG maxk;
		
		if (submatchTag==1)
			maxk = R_num;
		else
			maxk = 1;
		
		for (k=0; k<maxk; k++)
		{
		
			V3DLONG subnum = 1;
			
			for (m=N_num; m>0; m--)
				subnum *= m;
			
			for (n=R_num; n>0; n--)
				subnum *=n;
				
			for (p=N_num-R_num+k; p>0; p--)
				subnum /=p;
				
			for (p=R_num-k; p>0; p--)
				subnum /=p;
				
			for (p=k; p>0; p--)
				subnum /=p;
			
			num += subnum;
		}
			
		// allocate memory and initialize possibleMatchList
		V3DLONG len = num*(R_num*2);
		possibleMatchList = new V3DLONG [len]; 
		for (k=0; k<len; k++)
			possibleMatchList[k] = INVALID_VALUE;

							
		//enumerate possible combinations
		for (k=0; k<maxk; k++)
		{
			r_num = R_num-k;
			r_list = new V3DLONG [r_num];

			for (m=0; m<r_num; m++)
				r_list[m] = m;
				
			do
			{
			
				// assign some values to possibleMatchList		

				V3DLONG tmpnum = 1;
				for (p=N_num; p>0; p--)
					tmpnum *= p;
				for (p=N_num-r_num; p>0; p--)
					tmpnum /=p;
			
//				printf("tmpnum = %d\n", tmpnum);
				
				cnt2 = cnt;
//				printf("cnt=%d\n", cnt);
							
				if (childrenNum1_Tree1>childrenNum2_Tree2)
				{	
					n = 0;
					while (n<tmpnum)
					{
						cnt++;
						n++;
						for (m=0; m<r_num; m++)
							possibleMatchList[cnt*(R_num*2)+2*m+1] = r_list[m];
					}
				}
				else
				{	
					n = 0;
					while(n<tmpnum)
					{
						cnt++;
						n++;
						for (m=0; m<r_num; m++)
							possibleMatchList[cnt*(R_num*2)+2*m] = r_list[m];
					}
				}

				// assign remaining values in possibleMatchList
				r1_list = new V3DLONG [r_num];
				r2_list = new V3DLONG [r_num];
				
				for (m=0; m<r_num; m++)
//					r1_list[m] = r_list[m];
					r1_list[m] = m;
					
				cnt = cnt2;
//				printf("cnt=%d\n", cnt);

				V3DLONG tmpcnt = 0;
				do
				{
					tmpcnt++;
					for (m=0; m<r_num; m++)
					{	
						r2_list[m] = r1_list[m];
					}

					do
					{
						cnt++;
						if (childrenNum1_Tree1>childrenNum2_Tree2)
						{
							for (m=0; m<r_num; m++)
//								possibleMatchList[cnt*(R_num*2)+2*m] = r2_list[m];
//								possibleMatchList[cnt*(R_num*2)+2*m] = r_list[r2_list[m]];
								possibleMatchList[cnt*(R_num*2)+2*m] = N_list[r2_list[m]];

						}
						else
						{
							for (m=0; m<r_num; m++)
//								possibleMatchList[cnt*(R_num*2)+2*m+1] = r2_list[m];
//								possibleMatchList[cnt*(R_num*2)+2*m+1] = r_list[r2_list[m]];
								possibleMatchList[cnt*(R_num*2)+2*m+1] = N_list[r2_list[m]];

						}
						
					}while(next_permutation(r2_list,r2_list+r_num)); //next_permutation is defined in <algorithm.h>

				}while(next_combination(N_list,N_list+N_num,r1_list, r1_list+r_num)); // next_combination is defined in "./combination/combination.h"

//				printf("tmpcnt=%d\n", tmpcnt);
//				printf("cnt=%d\n", cnt);

				// delete pointers
				if (r2_list) {delete []r2_list; r2_list=0;}
				if (r1_list) {delete []r1_list; r1_list=0;}
				
			}while(next_combination(R_list, R_list+R_num, r_list, r_list+r_num)); // next_combination is defined in "./combination/combination.h"

			if (r_list) {delete []r_list; r_list=0;}
			
		} // for k end
		 
		 
		// print possibleMatchList
		printf("possibleMatchList:\n");
		
		for (m=0;m<num; m++)
		{
			for (n=0; n<R_num; n++)
				printf("%d, %d, ", possibleMatchList[m*(R_num*2) +2*n], possibleMatchList[m*(R_num*2) +2*n+1]);
			printf("\n");
		}
		
		printf("\n");
	} // if end
				
}			
				
				


// // *************************************************************************************
// match two trees using dynamic programming
// Input: swcTree *Tree1, swcTree *Tree2, bool b_scale, float *lengthRatioThre, float subtreeRatioThre
// Output: V3DLONG *&matchingList, float &simMeasure
// matchingList: a 1d array that saves the data of a 2d Matrix: #tree node * (MAX_CHILDREN_NUM+1) 
// each row corresponds to one node in tree1, ordered in the same order of swcTree
// the first element of each node is the nodeid of the matched node in Tree2
// the second to the last elements are the corresponding branches in Tree2 that match
// respectively each branch (0,1,2,..) of the current node in Tree1

// properties of this function:
// 1. lengthRatioThre is used to determie which branching nodes are kept as significant branching nodes. 
//    when lengthRatioThre[0] and lengthRatioThre[1] are big, only match significant branching points, 
//    no continual points, leaf nodes will be matched (should set keepLeaf = 0). When lengthRatioThre[0] and 
//     lengthRatioThre[1] are 0, all branching nodes are matched
// 2. Features are computed based on original trees (not pruned), best matched sub-trees are computed on pruned 
//    trees, i.e., only those nodes belonging to the significant branching points are considered
// 3. to compensate the number of matches which will influence the similarity1d, use similarity1d normalized
//    by the number of matches to select the best submatch in sub-branches, but the overall similarity is still
//    computed by accumulating all sub-branches matches
// 4. subtreeRatioThre can not be set to too big, preferrablely 2, otherwise, the algorithm will favor matches 
//    close to leaf nodes, making the number of matching nodes smaller (but are correct)
// 5. leaf nodes need to be removed, because the way the compute the lengthratio of leaf nodes are not 
//    appropriately defined (set to 0), so that leaf nodes have too much freedom to match to other nodes, 
//    which will influence the global matching, leaf nodes can be matched by routine treeMatching_leafNodes,
//    after the significant branching points have been matched. note that if keepLeaf is set to 0, leaf node
//    will not be matched correctly, because the length ratio of any leaf node is set to 0, no matter where 
//    it sits on the tree, leaf nodes need to be matched after significant branching points are matched

// earlier versions are of the following forms:
// bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure, float lengthThre1, float lengthThre2)
// bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure, float *lengthRatioThre, V3DLONG depthThre)
// note the different input parameters: lengthThre1 and lengthThre2 in the first version, and lengthRatioThre and depthThre in the second version
// lengthThre1 and lengthThre2 are the threshold of lengths the maximum and second maximum branches of node. 
// They determine which branching points will be kept in Tree1 and Tree2 for matching, if lengthThre1
// and lengthThre2 are big, only significant branching points will be matched; Otherwise small branching nodes or even all
// branching nodes will be matched
// in the latest version below, subtreeRatioThre (not depthThre, since the same depth difference may lead to big length differences) is used to determine how far subtree nodes 
// can be considered when matching subtrees
// lengthRatioThre replaces lengthThre1 and lengthThre2
//// *************************************************************************************

bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, bool b_scale, V3DLONG *&matchingList, float &simMeasure, float *lengthRatioThre, 
																 float subtreeRatioThre, float weight_lengthRatio, float weight_angle)
{
	V3DLONG i, j, m, n, p,q;
//	float weight_lengthRatio = 1.0; //weight_lengthRatio and weight_angle determine how lengthRatio and angle are combined
////	float weight_angle = 1.0;
////	float weight_lengthRatio = 0;
//	float weight_angle = 0;
////	float lengthThre1 = 50;
////	float lengthThre2 = 10;
//////	float lengthThre1 = 5;
//////	float lengthThre2 = 5;
	
	// -----------------------------------------------------------------
	// generate trees only containing root and significant branching points
	// remove continual points, small branching points, and leaf nodes
	// the input tree pointers are backed up in Tree1_old, Tree2_old
	// the trees after removing continual nodes are Tree1 and Tree2
	// -----------------------------------------------------------------	
	swcTree *Tree1_old =0, *Tree2_old = 0;
	swcTree *newTree1 =0, *newTree2 = 0;
	swcTree *newTreeWithPath1 =0, *newTreeWithPath2 = 0;
	bool *removeNodeTag1=0, *removeNodeTag2=0;
	V3DLONG **removeBranchTag1=0, **removeBranchTag2=0;	
	bool keepLeaf=0;
	
	Tree1_old = Tree1;
	Tree2_old = Tree2;
	
	 
//	Tree1->removeInsignificantNodes(lengthRatioThre, newTree1, keepLeaf, removeNodeTag1, removeBranchTag1); // remove continual nodes, leaf nodes (if keepLeaf=0) , and small branching points, note that features will be still computed on the Tree1_old

	// 20091014, use new way to detect critical branch node (or significant branch nodes)
	float *threVector = new float [2];
	threVector[0] = 0.1; threVector[0] = 0.1;
//	unsigned char leafMethod = 1;
	unsigned char leafMethod = 0;
	unsigned char criticalNodeMethod = 2;
	
	Tree1->detectCriticalBranchNodes(threVector, leafMethod, criticalNodeMethod, removeNodeTag1, removeBranchTag1, newTree1, newTreeWithPath1);
//	Tree1->detectCriticalBranchNodes(threVector, leafMethod, criticalNodeMethod, Tree1, removeNodeTag1, removeBranchTag1, newTree1, newTreeWithPath1);
	
	Tree1 = newTree1;
	newTree1 = 0;

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		printf("%d ", Tree1->node[i].nid);
	}
	printf("\n");

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		printf("%d %d ", Tree1->node[i].nid, Tree1->node[i].parentid);
	
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			printf("%d ", removeBranchTag1[i][j]);
		printf("\n");
	}
	printf("\n");
	
	Tree1->writeSwcFile("Tree1.swc"); 
	Tree1->genGraphvizFile("Tree1.dot");
	Tree1_old->genGraphvizFile("Tree1_old.dot");
	
	newTreeWithPath1->writeSwcFile("TreeWithPath1.swc");
	if (newTreeWithPath1) {delete newTreeWithPath1; newTreeWithPath1=0;}
	
	
//	Tree2->removeInsignificantNodes(lengthRatioThre, newTree2, keepLeaf, removeNodeTag2, removeBranchTag2); // remove continual nodes, leaf nodes (if keepLeaf=0), and small branching points

	// 20091014, use new way to detect critical branch node (or significant branch nodes)
	Tree2->detectCriticalBranchNodes(threVector, leafMethod, criticalNodeMethod, removeNodeTag2, removeBranchTag2, newTree2, newTreeWithPath2);	
//	Tree2->detectCriticalBranchNodes(threVector, leafMethod, criticalNodeMethod, Tree1, removeNodeTag2, removeBranchTag2, newTree2, newTreeWithPath2);	
	if (threVector) {delete []threVector; threVector=0;}
	
	Tree2 = newTree2;
	newTree2 = 0;
	
	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		printf("%d ", Tree2->node[i].nid);
	}
	printf("\n");

	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		printf("%d %d ", Tree2->node[i].nid, Tree2->node[i].parentid);
	
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			printf("%d ", removeBranchTag2[i][j]);
		printf("\n");
	}
	printf("\n");
	
	Tree2->writeSwcFile("Tree2.swc"); 
	Tree2->genGraphvizFile("Tree2.dot");
	Tree2_old->genGraphvizFile("Tree2_old.dot");

	newTreeWithPath2->writeSwcFile("TreeWithPath2.swc");
	if (newTreeWithPath2) {delete newTreeWithPath2; newTreeWithPath2=0;}
	
	
	// -----------------------------------------------------------------
	// get the number of nodes in the subtree rooted at each node
	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
	// -----------------------------------------------------------------	
	V3DLONG *subTreeNodeNum1 = 0;
	V3DLONG *subTreeNodeNum2 = 0;
	
	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 
	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?

#ifdef DEBUG
	
	printf("print subTreeNodeNum1 returned from Tree1->getSubTreeNodeNum(subTreeNodeNum1)\n");	
	for (i=0; i<Tree1->treeNodeNum; i++)
		printf("%d, %d\n ", i, subTreeNodeNum1[i]);
	printf("\n");

	printf("print subTreeNodeNum2 returned from Tree2->getSubTreeNodeNum(subTreeNodeNum2)\n");	
	for (i=0; i<Tree2->treeNodeNum; i++)
		printf("%d, %d\n ", i, subTreeNodeNum2[i]);
	printf("\n");

#endif

	// ------------------------------------------------------------
	// sort subTreeNodeNum1 and subTreeNodeNum2 from small to large
	// ------------------------------------------------------------
	float *sortval1, *sortidx1;
	float *sortval2, *sortidx2;
	
	Tree1->sortSubTreeNodeNum(subTreeNodeNum1, sortval1, sortidx1);
	Tree2->sortSubTreeNodeNum(subTreeNodeNum2, sortval2, sortidx2);
	
#ifdef DEBUG

	printf("print sortval1 and sortidx1\n");
	
	for (i=0; i<Tree1->treeNodeNum; i++)
		printf("%d, %f, %f\n ", i, sortval1[i+1], sortidx1[i+1]);
	printf("\n");
	
	printf("print sortval2 and sortidx2\n");
	
	for (i=0; i<Tree2->treeNodeNum; i++)
		printf("%d, %f, %f\n ", i, sortval2[i+1], sortidx2[i+1]);
	printf("\n");
	
#endif


	// ---------------------------------------
	// tree matching using dynamic programming
	// ---------------------------------------
	V3DLONG nodeNum1, nodeNum2;
	nodeNum1 = Tree1->treeNodeNum;
	nodeNum2 = Tree2->treeNodeNum;
	
	V3DLONG nodeidx1, nodeidx2;
//	V3DLONG depthThre = 2;
//	V3DLONG depthThre = 4;

	// allocate memory for similarity1d and intialize
	float *similarity1d = new float [nodeNum1*nodeNum2]; // #(branching nodes + root +leaf nodes in Tree 1) *#(branching nodes + root + leaf nodes in Tree 2)
	if (!similarity1d)
	{
		printf("Fail to allocate memory for similarity1d. \n");
		return false;
	}
	
	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	{
		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE; // leaf nodes will not be matched in DP, thus they will keep this value
//		similarity1d[m*nodeNum2 + n] =  0;
	}

		
	// allocate memory for mappingfunc1d and intialize	
	V3DLONG *mappingfunc1d = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching
	if (!mappingfunc1d)
	{
		printf("Fail to allocate memory for mappingfunc1d. \n");
		return false;
	}

	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	for (p=0; p<nodeNum1; p++)
		mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
	
	// initialize matching list, the size is determined by the original Tree1, i.e., Tree1_old
	V3DLONG len = Tree1_old->treeNodeNum*(MAX_CHILDREN_NUM+1);
	 matchingList = new V3DLONG [len]; //indicating for each node in tree1, the id# of the matching point in tree2, and the matching branches
	
	for (i=0; i<len; i++)
		matchingList[i] = INVALID_VALUE; //need to check i
	
	// allocate memory and initiate bestBranchMatch, which indicates that for each pair of matching nodes in tree1 and tree2, 
	// how their branches are matched. This information is used in matching leaf nodes and hierarchical matching
	V3DLONG *bestBranchMatch = new V3DLONG [nodeNum1*nodeNum2*(MAX_CHILDREN_NUM*2)];
	
//	V3DLONG cnttt = 0;	
	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	for (p=0; p<MAX_CHILDREN_NUM*2; p++) 
	{
//		cnttt++;
		bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2) + n*(MAX_CHILDREN_NUM*2) + p] = INVALID_VALUE;
	}
		
	// start matching
	for (i=0; i<nodeNum1; i++)
	{
	
		V3DLONG *childrenList1_Tree1=0, *childrenNodeIdx1_Tree1=0, childrenNum1_Tree1;
		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1, childrenNodeIdx1_Tree1, childrenNum1_Tree1); // get direct children for the current node in Tree1
		
#ifdef DEBUG

		printf("print childrenList1_Tree1 and childrenNodeIdx1_Tree1\n");
		for (int m=0; m<childrenNum1_Tree1; m++)
			printf("%d ", childrenList1_Tree1[m]);
		printf("\n");
		
		for (int m=0; m<childrenNum1_Tree1; m++)
			printf("%d ", childrenNodeIdx1_Tree1[m]);
		printf("\n\n");
		
#endif
	
		// compute branch length ratio
		float *lengthratio1=0;
		unsigned char lengthratioNum1, childrenNum1_Tree1_old;
		if (b_scale==true)
			Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4); // use length ratio
		else
			Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 5); // use length instead of length ratio
			
		childrenNum1_Tree1_old = lengthratioNum1; // number of children in old Tree1
		
		// compute the angle metrics between branches 
		V3DLONG branchNum1; //branchNum1 should have the same value as lengthratioNum1
		float *angles1;
		Tree1_old->computeBranchingAngles(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, branchNum1, angles1);

#ifdef DEBUG
		printf("branchNum1 = %d, lengthratioNum1 = %d\n", branchNum1, lengthratioNum1);
		for (m=0; m<branchNum1*(branchNum1+1)/2; m++)
			printf("%f ", angles1[m]);
		printf("\n");
#endif


		// find candidate nodes in Tree2 that can be matched to the current node in tree1
		
		for (j=0; j<nodeNum2; j++) 
		{

//			printf("i=%d,j=%d\n", i,j);	
//			printf("nodeid1 = %d, nodeid2 = %d, sortidx1 = %f, sortidx2 = %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid, sortidx1[i+1], sortidx2[j+1]);
			
			V3DLONG *childrenList2_Tree2=0, *childrenNodeIdx2_Tree2=0, childrenNum2_Tree2;
			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2, childrenNodeIdx2_Tree2, childrenNum2_Tree2); // get direct children for the current node in Tree1
			
			
#ifdef DEBUG

			printf("print childrenList2_Tree2 and childrenNodeIdx2_Tree2\n");
			
			for (m=0; m<childrenNum2_Tree2; m++)
				printf("%d ", childrenList2_Tree2[m]);
			printf("\n");
			
			for (m=0; m<childrenNum2_Tree2; m++)
				printf("%d ", childrenNodeIdx2_Tree2[m]);
			printf("\n\n");
#endif			
			
			// compute the legnth ratio metrics of each branch based on which similarity is further computed
			float *lengthratio2=0;
			unsigned char lengthratioNum2, childrenNum2_Tree2_old;
			
			if (b_scale == true)
				Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4); // use length ratio
			else
				Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 5); // use length instead of length ratio
				
			childrenNum2_Tree2_old = lengthratioNum2; // number of children in old Tree1
			
			float *simVector_lengthRatio = 0;			
//			printf("i=%d, j=%d, lengthratioNum1 = %d, lengthratioNum2=%d\n", i,j,lengthratioNum1, lengthratioNum2);
			simVector_lengthRatio = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
			
			printf("simVector_lengthRatio\n");
			for (m=0; m<lengthratioNum1; m++)
			{
				for (n=0; n<lengthratioNum2; n++)
				{
					printf("%f ", simVector_lengthRatio[m*lengthratioNum2 + n]);
				}
				printf("\n");
			}
			
			// compute the angle metrics between branches based on which similarity is further computed
			V3DLONG branchNum2; //branchNum2 should have the same value as lengthratioNum2
			float *angles2;
			Tree2_old->computeBranchingAngles(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, branchNum2, angles2);

#ifdef DEBUG
		printf("branchNum2 = %d, lengthratioNum2 = %d\n", branchNum2, lengthratioNum2);
		for (m=0; m<branchNum2*(branchNum2+1)/2; m++)
			printf("angles2 = %f ", angles2[m]);
		printf("\n");
#endif
			
			float *simVector_angle_parent = 0, *simVector_angle_children = 0;			
//			printf("i=%d, j=%d, branchNum1 = %d, branchNum2=%d\n", i,j,branchNum1, branchNum2);
			calsim_allbranch_angles(angles1, angles2, branchNum1, branchNum2, simVector_angle_parent, simVector_angle_children);		
			
//			printf("simVector_angle_parent=%d, simVector_angle_children= %d\n", simVector_angle_parent, simVector_angle_children);
			
#ifdef DEBUG
			for (m=0; m<lengthratioNum1; m++)
			{
				for (n=0; n<lengthratioNum2; n++)
					printf("%f ", simVector_angle_parent[m*lengthratioNum2+n]);
				printf("\n");
			}
			printf("\n");
#endif			
			
			for (int p = 0; p<nodeNum1; p++)
				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;

			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
				

			// find the best matching for subtree by enumeration	
			
				
			// compute all possible combinations of matched branches with respect to the old tree and save it in possibleMatchList

			V3DLONG *possibleMatchList;
			V3DLONG num, N_num, R_num;
			findPossibleMatchList(childrenNum1_Tree1_old, childrenNum2_Tree2_old, 0, possibleMatchList, num, N_num, R_num);
//			old code
//			V3DLONG *childrenOrder1_Tree1 = new V3DLONG [childrenNum1_Tree1_old];
//			V3DLONG *childrenOrder2_Tree2 = new V3DLONG [childrenNum2_Tree2_old];
//			
//			for (m=0; m<childrenNum1_Tree1_old; m++)
//				childrenOrder1_Tree1[m] = m;
//			
//			for (m=0; m<childrenNum2_Tree2_old; m++)
//				childrenOrder2_Tree2[m] = m;
//								
//			//enumerate possible combinations
//			V3DLONG N_num, R_num, *N_list, *R_list;
//			
//			if (childrenNum1_Tree1_old>childrenNum2_Tree2_old)
//			{
//				N_num = childrenNum1_Tree1_old;
//				R_num = childrenNum2_Tree2_old;					
//				N_list = childrenOrder1_Tree1;
//				R_list = childrenOrder2_Tree2;
//			}
//			else
//			{
//				N_num = childrenNum2_Tree2_old;
//				R_num = childrenNum1_Tree1_old;
//				N_list = childrenOrder2_Tree2;
//				R_list = childrenOrder1_Tree1;
//			}
//								
//			V3DLONG num = 1; // number of possible combinations
//			//compute num: C(m,n)*P(n) = m!/((m-n)!*n!)*n! = m!/(m-n)!;
//			for (m=N_num; m>0; m--)
//				num *= m;
//			for (m=(N_num-R_num); m>0; m--)
//				num /=m;
//				
//			V3DLONG *possibleMatchList = new V3DLONG [num*(R_num*2)]; 
//
//			// assign values to possibleMatchList				
//			if (childrenNum1_Tree1_old>childrenNum2_Tree2_old)
//			{	
//				for (n=0; n<num; n++)
//				for (m=0; m<R_num; m++)
//					possibleMatchList[n*(R_num*2)+2*m+1] = m;
//			}
//			else
//			{
//				for (n=0; n<num; n++)
//				for (m=0; m<R_num; m++)
//					possibleMatchList[n*(R_num*2)+2*m] = m;
//			}
//			
//			// generate C(m,n) and then p(n) to assign remaining values in possibleMatchList
//			V3DLONG *R2_list=new V3DLONG [R_num];
//			V3DLONG cnt = -1;
//			
//			do
//			{
//				for (m=0; m<R_num; m++)
//				{	
//					//printf("%d ", R_list[m]);
//					R2_list[m] = R_list[m];
//				}
//				//printf("\n");
//
//				do
//				{
//					cnt++;
//					if (childrenNum1_Tree1_old>childrenNum2_Tree2_old)
//					{
//						for (m=0; m<R_num; m++)
//							possibleMatchList[cnt*(R_num*2)+2*m] = R2_list[m];
//					}
//					else
//					{
//						for (m=0; m<R_num; m++)
//							possibleMatchList[cnt*(R_num*2)+2*m+1] = R2_list[m];
//					}
//					
//				}while(next_permutation(R2_list,R2_list+R_num)); //next_permutation is defined in <algorithm.h>
//
//			}
//			while(next_combination(N_list,N_list+N_num,R_list, R_list+R_num)); // next_combination is defined in "./combination/combination.h"
//			
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);					
//			printf("possibleMatchList:\n");
//			
//			for (m=0;m<num; m++)
//			{
//				for (n=0; n<R_num; n++)
//					printf("%d, %d, ", possibleMatchList[m*(R_num*2) +2*n], possibleMatchList[m*(R_num*2) +2*n+1]);
//				printf("\n");
//			}
				
																																																				
		//		find the best matching nodes in the subtree rooted at each pair-wise children nodes in Tree1 and Tree2,
		//		not in Tree1_old, Tree2_old

			V3DLONG *bestSubMatch =0;		
			if ((childrenNum1_Tree1!=0)&&(childrenNum2_Tree2!=0)) // if one of the nodes is leaf node, do not compute similarity of subtrees, only compute local similarity  		  
			{

		//		initialize bestSubMatch
				bestSubMatch = new V3DLONG [childrenNum1_Tree1*childrenNum2_Tree2*2];

				for (m=0; m<childrenNum1_Tree1; m++)
				for (n=0; n<childrenNum2_Tree2; n++)
				{
					bestSubMatch[m*childrenNum2_Tree2*2+n*2] = INVALID_VALUE;
					bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = INVALID_VALUE;
				}
				
				// find the best match
				for (m=0; m<childrenNum1_Tree1; m++)
				{
					//get all children rooted at childrenList1_Tree1[m], 	
					V3DLONG *subTreeNodeIdx1 = 0;
					swcTree *subTree1 = 0; 
//					Tree1->getSubTree(childrenList1_Tree1[m], depthThre, subTree1, subTreeNodeIdx1);
					Tree1->getSubTree(childrenList1_Tree1[m], subtreeRatioThre, subTree1, subTreeNodeIdx1); // get the nodes in the subtree whose lenghtratio is constrained by subtreeRatioThre
					
					for (n=0; n<childrenNum2_Tree2; n++)
					{
						// find the best one in the subtree rooted at nodeidx2
						V3DLONG *subTreeNodeIdx2 = 0;
						swcTree *subTree2 = 0; 
//						Tree2->getSubTree(childrenList2_Tree2[n], depthThre, subTree2, subTreeNodeIdx2);
						Tree2->getSubTree(childrenList2_Tree2[n], subtreeRatioThre, subTree2, subTreeNodeIdx2);

						// find the best match with highest similarity in the subtree rooted at childrenList1_Tree1[m] and childreList2_Tree2[n]						
						float bestval = INVALID_VALUE;
						V3DLONG p1, q1;
												
//						for (p=0; p<subTree1->treeNodeNum; p++)
//						for (q=0; q<subTree2->treeNodeNum; q++)
//							if (bestval<=similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//							{
//								bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//								p1 = p; q1 = q;
//							}
						
						for (p=0; p<subTree1->treeNodeNum; p++)
						for (q=0; q<subTree2->treeNodeNum; q++)
						{

							float similarity1d_norm = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
							int cntt=0;
						
							printf("subTreeNodeIdx1[p] = %d, subTreeNodeIdx2[q] = %d\n", Tree1->node[subTreeNodeIdx1[p]].nid, Tree2->node[subTreeNodeIdx2[q]].nid);
							printf("mappingfunc1d:\n");
								
							for (V3DLONG pp = 0; pp<nodeNum1; pp++)
							{
								if (mappingfunc1d[subTreeNodeIdx1[p]*nodeNum2*nodeNum1+subTreeNodeIdx2[q]*nodeNum1+pp]!=INVALID_VALUE)
								{
									cntt++;
									printf("%d %d\n", Tree1->node[pp].nid,Tree2->node[mappingfunc1d[subTreeNodeIdx1[p]*nodeNum2*nodeNum1+subTreeNodeIdx2[q]*nodeNum1+pp]].nid);
								}
							}
							
//							similarity1d_norm /= cntt;

							// 20090402 begin
							if (cntt!=0)
								similarity1d_norm /= cntt;
							else
								similarity1d_norm = INVALID_VALUE;									
							//20090402 end
							
							printf("similarity1d_norm = %f\n", similarity1d_norm);
							
							
							if (bestval<=similarity1d_norm)
							{
								bestval = similarity1d_norm;
								p1 = p; q1 = q;
							}
							
						}
						

//						bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p1];
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q1];

						//20090402 begin
						if (bestval!=INVALID_VALUE)
						{
							bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p1];
							bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q1];
						}
						//20090402 end
						
						if (subTree2) {delete subTree2; subTree2=0;}
						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
								
					} // for n
				
					if (subTree1) {delete subTree1; subTree1=0;}
					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}

				} // for m

//				//print bestSbuMatch
//				printf("bestSubMatch:\n");
//				for (m=0; m<childrenNum1_Tree1; m++)
//				for (n=0; n<childrenNum2_Tree2; n++)
//					printf("m = %d, n = %d, bestSubMatch1 = %d, bestSubMatch2 = %d, nodeid1 = %d, nodeid2 = %d\n", m,n, bestSubMatch[m*childrenNum2_Tree2*2+n*2],  bestSubMatch[m*childrenNum2_Tree2*2+n*2+1],
//					      Tree1->node[bestSubMatch[m*childrenNum2_Tree2*2+n*2]].nid, Tree2->node[bestSubMatch[m*childrenNum2_Tree2*2+n*2+1]].nid);
						  
						  
			}// if ((childrenNum1_Tree1!=0)&&(childrenNum2_Tree2!=0))
				
					
			// compute for each match the best similarity score and corresponding match
				
			float bestSimVal = INVALID_VALUE;
//			float bestSimVal_norm = INVALID_VALUE;

			V3DLONG best_m = 0;
			
			for (m=0; m<num; m++) // for all possible combinations in old trees
			{
				float tmpSimVal=0;
//				float tmpSimVal_norm;			
				
				unsigned char matchedNodeNum = 0; // for normalizing similarity purpose
				
				if ((childrenNum1_Tree1!=0)&&(childrenNum2_Tree2!=0)) // only compute subtree similarity for non-leaf nodes
				{
					// subtree similarity
					
					for (n=0; n<R_num; n++) // mappings in old trees
					{
						V3DLONG tmpidx =m*(R_num*2)+2*n; 

						V3DLONG idx1 = possibleMatchList[tmpidx];
						V3DLONG idx2 = possibleMatchList[tmpidx+1];

						
						// test if idx1, idx2 which are in Tree1_old and Tree2_old are still in Tree1 and Tree2
						
						if ((removeBranchTag1[(V3DLONG)sortidx1[i+1]][idx1]==-1)||(removeBranchTag2[(V3DLONG)sortidx2[j+1]][idx2]==-1)) // at least one of the mapped branches is not in Tree1 or Tree2
							continue; // do not count that match
						else
						{
							// compute the new index of branches in Tree1 and Tree2, so that bestSubMatch can use the correct subscripts
							
							V3DLONG idx1_new = removeBranchTag1[(V3DLONG)sortidx1[i+1]][idx1];
							V3DLONG idx2_new = removeBranchTag2[(V3DLONG)sortidx2[j+1]][idx2];
							
							V3DLONG ind1 = bestSubMatch[idx1_new*childrenNum2_Tree2*2+idx2_new*2];
							V3DLONG ind2 = bestSubMatch[idx1_new*childrenNum2_Tree2*2+idx2_new*2+1];		
												
//							tmpSimVal += similarity1d[ind1*nodeNum2+ind2];
							
							//20090402 begin
							if ((ind1!=INVALID_VALUE)&&(ind2!=INVALID_VALUE)) // otherwise, no subtree nodes in that branch pair can be matched, ignore that branch pair
								tmpSimVal += similarity1d[ind1*nodeNum2+ind2];
							//20090402 end
							
//							// compute number of matched nodes in subtree
//							for (p = 0; p<nodeNum1; p++)
//							{
//								if (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]!=INVALID_VALUE)
//									matchedNodeNum++;
//							}
							
						}

					}
					
//					printf("subtree similarity1d = %f, ", tmpSimVal);
				} // if ((childrenNum1_Tree1!=0)&&(childrenNum2_Tree2!=0)) 

//				printf("matchedNodeNum = %d\n", matchedNodeNum);						
					
				// plus length-ratio similarity, computed based on old trees
				float tmpSimLengthRatio=0;
				printf("m=%d, ", m);
				
				for (n=0; n<R_num; n++)
				{
					V3DLONG tmpidx =m*(R_num*2)+2*n; 
					V3DLONG idx1 = possibleMatchList[tmpidx];
					V3DLONG idx2 = possibleMatchList[tmpidx+1];
					
					//20090402 begin
					if (simVector_lengthRatio[idx1*childrenNum2_Tree2_old+idx2]==-1) // if one pair of branch is far too different, then that branching pair should not be considered
					{
						tmpSimLengthRatio = -R_num;
						break;
					}
					else
						tmpSimLengthRatio += simVector_lengthRatio[idx1*childrenNum2_Tree2_old+idx2];
					printf("%f, ", simVector_lengthRatio[idx1*childrenNum2_Tree2_old+idx2]); 
					//20090402 end
					
				}
				
				printf("\n");
				
				tmpSimLengthRatio /= R_num; // normalize to balance length-ratio and angle
//					if (weight_lengthRatio!=0)
//						printf("local length ratio similarity1d = %f\n ", tmpSimLengthRatio);
				
				tmpSimVal += (weight_lengthRatio*tmpSimLengthRatio);
				
				printf("tmpSimVal=%f\n", tmpSimVal);
				
				// plus angle (parent-path) similarity, computed based on old trees
				float tmpSimAngle = 0;
				for (n=0; n<R_num; n++)
				{
					V3DLONG tmpidx =m*(R_num*2)+2*n; 
					V3DLONG idx1 = possibleMatchList[tmpidx];
					V3DLONG idx2 = possibleMatchList[tmpidx+1];
					
					tmpSimAngle += simVector_angle_parent[idx1*childrenNum2_Tree2_old + idx2];
				}
				
				// plus angle (children-path) similarity	

				V3DLONG len = childrenNum2_Tree2_old*(childrenNum2_Tree2_old-1)/2;
				V3DLONG ttmp;
								
				for (p=0; p<R_num-1; p++)
				for (q=p+1; q<R_num; q++)
				{
					V3DLONG tmpidxp =m*(R_num*2)+2*p; 
					V3DLONG tmpidxq =m*(R_num*2)+2*q; 
					
					// compute idx1 and idx2
					V3DLONG idx1 = 0, idx2 = 0;
					for (V3DLONG s=0; s< possibleMatchList[tmpidxp]; s++)
						idx1 += (childrenNum1_Tree1_old-(s+1));

//						idx1 +=  (possibleMatchList[tmpidxq] - possibleMatchList[tmpidxp]-1);

					ttmp = possibleMatchList[tmpidxq] - possibleMatchList[tmpidxp];
					if (ttmp<0)
						ttmp = -ttmp;							
					idx1 +=  (ttmp-1);
						

					for (V3DLONG s=0; s< possibleMatchList[tmpidxp+1]; s++)
						idx2 += (childrenNum2_Tree2_old-(s+1));
					
					ttmp = possibleMatchList[tmpidxq+1] - possibleMatchList[tmpidxp+1];
					if (ttmp<0)
						ttmp = -ttmp;							
					idx2 +=  (ttmp-1);
											
					tmpSimAngle += simVector_angle_children[idx1*len + idx2];
				}
				
				tmpSimAngle /= (R_num*(R_num+1)/2); // normalize to balance length-ratio and angle
//					printf("tmpSimLengthRatio = %f, tmpSimAngle = %f, R_num= %d\n", tmpSimLengthRatio, tmpSimAngle, R_num);

//					if (weight_angle!=0)
//						printf("local angle similarity1d = %f\n ", tmpSimAngle);
				
				tmpSimVal += (weight_angle*tmpSimAngle);
				
//					float nn = (rand() % 201-100.0)/100.0;
//					printf("nn = %f\n", nn);
//					
////					tmpSimVal += (100*nn);	
//					tmpSimVal += nn;	
				

//				tmpSimVal_norm = tmpSimVal/(matchedNodeNum+1); // normalize by the number of matched nodes in the subtree, +1 is the add the node itsself
				
//				if (bestSimVal_norm<tmpSimVal_norm)
//				{
//					bestSimVal_norm = tmpSimVal_norm;
//					bestSimVal = tmpSimVal;
//					best_m = m;
//				}

				if (bestSimVal<tmpSimVal)
				{
					bestSimVal = tmpSimVal;
					best_m = m;
					printf("best_m=%d\n", best_m);
				}

				
			} //for (m=0; m<num; m++)


			//update the best similarity and bestBranchMatch			
			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]= bestSimVal;
			printf("bestSimVal = %f\n", bestSimVal);

//			for (p=0; p<R_num; p++)
//			{
//				bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2] = possibleMatchList[best_m*R_num*2+p*2];
//				bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2+1] = possibleMatchList[best_m*R_num*2+p*2+1];
//			}
			
			// 20090402 begin
			if (bestSimVal!=-1) // otherwise, do not set bestBranchMatch since the pair of nodes should not be matched
			{
				for (p=0; p<R_num; p++)
				{
					bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2] = possibleMatchList[best_m*R_num*2+p*2];
					bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2+1] = possibleMatchList[best_m*R_num*2+p*2+1];					
				}
			}
			else
			{
				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = INVALID_VALUE; // make the pair of nodes unmatched
				continue; // do not do the following either
			}
			// 20090402 end
			
			//assign the best mapping list if both nodes are non-leaf nodes
			if ((childrenNum1_Tree1!=0)&&(childrenNum2_Tree2!=0))
			{
				for (m=0; m<R_num; m++)
				{
					
					p = possibleMatchList[best_m*(R_num*2)+2*m];
					q = possibleMatchList[best_m*(R_num*2)+2*m+1];
					
					// not all R_num have matching, need to test
//					if ((removeBranchTag1[(V3DLONG)sortidx1[i+1]][p]==0) && (removeBranchTag2[(V3DLONG)sortidx2[j+1]][q]==0)) // the branches in each tree should exist
					if ((removeBranchTag1[(V3DLONG)sortidx1[i+1]][p]>=0) && (removeBranchTag2[(V3DLONG)sortidx2[j+1]][q]>=0)) // the branches in each tree should exist

					{

						V3DLONG p_new = removeBranchTag1[(V3DLONG)sortidx1[i+1]][p];
						V3DLONG q_new = removeBranchTag2[(V3DLONG)sortidx2[j+1]][q];
						
						// add the best matched sub-branch
						V3DLONG ind1 = bestSubMatch[p_new*childrenNum2_Tree2*2+q_new*2];
						V3DLONG ind2 = bestSubMatch[p_new*childrenNum2_Tree2*2+q_new*2+1];
						
					
//						mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
						
						//20090402 begin
						if ((ind1!=INVALID_VALUE)&&(ind2!=INVALID_VALUE))
							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;
						else
							continue;
						//20090402 end
													
						//expand matching list of the children
						for (p = 0; p<nodeNum1; p++)
						{	
							
							if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
								(mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
						    {
//									printf("%d ", mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]); //20090402

									mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
									mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
							}
							
						} //for p
						
						printf("\n");
						
					}  // if removeBranchTag1 ...
				} // for m 
			}
			
			if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
			if (possibleMatchList) {delete []possibleMatchList; possibleMatchList=0;}
//			if (R2_list) {delete []R2_list; R2_list=0;}
//			if (childrenOrder1_Tree1) {delete []childrenOrder1_Tree1; childrenOrder1_Tree1=0;}
//			if (childrenOrder2_Tree2) {delete []childrenOrder2_Tree2; childrenOrder2_Tree2=0;}
				


#ifdef DEBUG	
	
			//print similarity matrix
			printf("Similarity matrix\n");
			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
			
			for (m=0; m<nodeNum1; m++)
			{
				for (n=0; n<nodeNum2; n++)
					printf("%f ", similarity1d[m*nodeNum2+n]);
				printf("\n");
			}
			
#endif	

			if (lengthratio2) {delete lengthratio2; lengthratio2=0;}

			if (childrenList2_Tree2) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
			if (childrenNodeIdx2_Tree2) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
			
		} //for (j=0; j<nodeNum2; j++)
		

			
			float bestsim = INVALID_VALUE;
			V3DLONG best_j=0;
			
			for (j=0; j<nodeNum2; j++)
				if (similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+j]>bestsim)
				{
					bestsim = similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+j];
					best_j = j;
				}

			printf("nodeid1 = %d, bestmatched nodeid2 = %d, best similarity = %f \n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[best_j].nid, bestsim);

			for (p=0; p<nodeNum1; p++)
			{
				V3DLONG q = mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+best_j*nodeNum1+p];
				if (q!=INVALID_VALUE)
//					printf("       nodeid1 = %d is matched to nodeid2 = %d\n", Tree1->node[p].nid, Tree2->node[q].nid);
					printf("%d %d\n", Tree1->node[p].nid, Tree2->node[q].nid);

			}
			
#ifdef DEBUG	
	
			//print similarity matrix
			printf("Similarity matrix\n");
			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
			
			for (m=0; m<nodeNum1; m++)
			{
				for (n=0; n<nodeNum2; n++)
					printf("%f ", similarity1d[m*nodeNum2+n]);
				printf("\n");
			}
			
			printf("\n");


#endif	

	if (lengthratio1) {delete lengthratio1; lengthratio1=0;}


	if (childrenList1_Tree1) {delete childrenList1_Tree1; childrenList1_Tree1=0;}
	if (childrenNodeIdx1_Tree1) {delete childrenNodeIdx1_Tree1; childrenNodeIdx1_Tree1=0;}
						
			
	}// for i
	
	
	// reconstruct the optimal match from stored best matches
	
	// method 2: reconstruct from the root
	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);
	V3DLONG matchedNodeNum = 0;
	
	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;

	for (m=0; m<nodeNum1; m++)
	{
		V3DLONG idx;
	
		
		if (m==rootidx1) //add root, require root must be matched to root
			idx = rootidx2;
		else
		{
		   idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
		}
		
		V3DLONG tmpidx1;
		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
		V3DLONG offset = tmpidx1*(MAX_CHILDREN_NUM+1);
		
		if (idx>=0)
		{
			printf("idx=%d, offset=%d, nodeid=%d\n", idx, offset, Tree2->node[idx].nid);
			
			matchedNodeNum ++;					
			matchingList[offset] = Tree2->node[idx].nid;		
		}	
//		else
//		{
//			for (n=0; n<MAX_CHILDREN+1; n++)
//			{
//				matchingList[offset+n] = INVALID_VALUE;
//			
//			}
//		}
	}
	
	simMeasure = similarity1d[rootidx1*nodeNum2 + rootidx2];
	printf("simMeasure = %f\n", simMeasure);
	
	
//	KEEP THE FOLLOWING

//	// method 1: reconstruct from the child of root
//	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
//	Tree1->getRootID(rootid1, rootidx1);
//	Tree2->getRootID(rootid2, rootidx2);
//
////	Tree1->getIndex(rootid1, rootidx1);
////	Tree2->getIndex(rootid2, rootidx2);
//
//	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//
//	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
//	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
//	
//	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
//		printf("Root should have only one child\n");
//
//	V3DLONG best_n = 0;
//
//	float bestval = INVALID_VALUE;
//	
//	for (n=1; n<nodeNum2; n++) // do not include root
//		if (bestval < similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + n])
//		{
//			bestval = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + n];
//			best_n = n;
//		}
//	printf("best_n = %d\n", best_n);
//
//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//	
//		
//		if (m==rootidx1) //add root, require root must be matched to root
//			idx = rootidx2;
//		else
//		{
////			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];				
//		   idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + best_n*nodeNum1 + m];
//		}
//		V3DLONG tmpidx1;
//		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
//		
//		if (idx>=0)
//		{
//			matchingList[tmpidx1] = Tree2->node[idx].nid;		
////			matchingList[m] = Tree2->node[idx].nid;
//			
//		}	
//		else
//			matchingList[tmpidx1] = INVALID_VALUE;
////			matchingList[m] = INVALID_VALUE;
//	}
//	
//
//	
//
////	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//
////	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + best_n];
//	printf("simMeasure = %f\n", simMeasure);

//	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
//	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
//	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
//	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}
	


	
	//print similarity matrix
	printf("Similarity matrix\n");
	
	for (i=0; i<nodeNum1; i++)
	{
		for (j=0; j<nodeNum2; j++)
			printf("%f ", similarity1d[i*nodeNum2+j]);
		printf("\n");
	}
	
	printf("\n best branch matching: \n");
		
////	branchMatchingList = new V3DLONG [matchedNodeNum*(MAX_CHILDREN_NUM)*2];

//	V3DLONG len1 = MAX_CHILDREN_NUM*2+1;
//	V3DLONG len = matchedNodeNum*len1;	
//	branchMatchingList = new V3DLONG [len]; // take it as a 2d matrix, matchedNodeNum rows, (MAX_CHILDREN_NUM*2+1) columns, the first element of each row is node id of tree1
//	
//	for (i=0; i<len; i++)
//		branchMatchingList[i] = INVALID_VALUE;
		
	// add best matching branches to matchingList	
	V3DLONG cnt = -1;
	
	for (m=0; m<nodeNum1; m++)
	{
	

		V3DLONG idx;
		
		if (m==rootidx1) //add root, require root must be matched to root
			idx = rootidx2;
		else
		{
		   idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
		}

		if (idx!=INVALID_VALUE)
		{
			cnt++;
			
			V3DLONG nodeidx1;
			
			Tree1_old->getIndex(Tree1->node[m].nid, nodeidx1);
	
			printf("nodeid1 = %d, nodeid2 = %d,           ", Tree1->node[m].nid, Tree2->node[idx].nid);

			V3DLONG *childrenList1=0, *childrenNodeIdx1=0, childrenNum1;			
			V3DLONG *childrenList2=0, *childrenNodeIdx2=0, childrenNum2;
			
			Tree1_old->getDirectChildren(Tree1->node[m].nid, childrenList1, childrenNodeIdx1, childrenNum1); 
			Tree2_old->getDirectChildren(Tree2->node[idx].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for the current node in Tree1
			
			if ((childrenNum1>0) && (childrenNum2>0))
			{
//				branchMatchingList[cnt*len1] = Tree1->node[m].nid;						

				for (p = 0; p<MAX_CHILDREN_NUM; p++)
				{
						V3DLONG tmpidx1 = bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2)+idx*(MAX_CHILDREN_NUM*2)+p*2];
						V3DLONG tmpidx2 = bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2)+idx*(MAX_CHILDREN_NUM*2)+p*2+1];
						
						if ((tmpidx1!=INVALID_VALUE)&&(tmpidx2!=INVALID_VALUE))
						{
//							branchMatchingList[cnt*len1+1+p*2] = tmpidx1;
//							branchMatchingList[cnt*len1+1+p*2+1] = tmpidx2;
							
							matchingList[nodeidx1*(MAX_CHILDREN_NUM+1)+tmpidx1+1] = tmpidx2;
							
							printf("tmpidx1 = %d, tmpidx2 = %d; ", tmpidx1, tmpidx2);
//							printf("%d, %d; ", Tree1_old->node[childrenNodeIdx1[tmpidx1]].nid, Tree2_old->node[childrenNodeIdx2[tmpidx2]].nid);
						}	
				}
			}
			printf("\n");
			
			if (childrenList1) {delete []childrenList1; childrenList1 = 0;}			
			if (childrenList2) {delete []childrenList2; childrenList2 = 0;}
			if (childrenNodeIdx1) {delete []childrenNodeIdx1; childrenNodeIdx1 = 0;}			
			if (childrenNodeIdx2) {delete []childrenNodeIdx2; childrenNodeIdx2 = 0;}
			
		} //if (idx!=INVALID_VALUE)
		printf("\n");
		
	} // for m
	


	// ------------------------------------------------
	// find matching between significant leaf nodes
	// ------------------------------------------------

	// ------------------------------------------------
	// generate matching result for Graphviz
	// ------------------------------------------------

//	// generate matching result file for Graphviz
//	swcTree *Tree1_new, *Tree2_new;
//	Tree1_new->readSwcFile("Tree1.swc", Tree1_new, 0); // allocate memory for Tree1 inside readSWCfile
//	Tree2_new->readSwcFile("Tree2.swc", Tree2_new, 0); // allocate memory for Tree1 inside readSWCfile

	V3DLONG *matchingList_new =new V3DLONG [Tree1->treeNodeNum];

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		V3DLONG idx;
		Tree1_old->getIndex(Tree1->node[i].nid, idx);
//		printf("%d\n", idx);
		
		if (matchingList[idx*(MAX_CHILDREN_NUM+1)]>0)
			matchingList_new[i] = matchingList[idx*(MAX_CHILDREN_NUM+1)];
		else
			matchingList_new[i] = INVALID_VALUE;
	};
	
	genMatchingGraphvizFile(Tree1, Tree2, matchingList_new, "matching.dot");

	if (matchingList_new) {delete []matchingList_new; matchingList_new=0;}	

	// ---------------------------------------
	// delete pointers
	// ---------------------------------------
	
	if (subTreeNodeNum1) {delete []subTreeNodeNum1; subTreeNodeNum1=0;}
	if (subTreeNodeNum2) {delete []subTreeNodeNum2; subTreeNodeNum2=0;}
	
	if (sortval1) {delete []sortval1; sortval1=0;}
    if (sortval2) {delete []sortval2; sortval2=0;}
	if (sortidx1) {delete []sortidx1; sortidx1=0;}
	if (sortidx2) {delete []sortidx2; sortidx2=0;}
	
	if (similarity1d) {delete []similarity1d; similarity1d=0;}
	
	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
	
	//recover pointer
	if (Tree1) {delete Tree1; Tree1=Tree1_old; Tree1_old=0;} 	
	if (Tree2) {delete Tree2; Tree2=Tree2_old; Tree2_old=0;} 
	
	if (removeNodeTag1) {delete removeNodeTag1; removeNodeTag1=0;}
	if (removeNodeTag2) {delete removeNodeTag2; removeNodeTag2=0;}

	V3DLONG *removeBranchTag1_1d = removeBranchTag1[0];
	if (removeBranchTag1_1d) {delete []removeBranchTag1; removeBranchTag1=0;}
	delete []removeBranchTag1_1d; removeBranchTag1_1d=0;

	V3DLONG *removeBranchTag2_1d = removeBranchTag2[0];
	if (removeBranchTag2_1d) {delete []removeBranchTag2; removeBranchTag2=0;}
	delete []removeBranchTag2_1d; removeBranchTag2_1d=0;
	
	if (bestBranchMatch) {delete []bestBranchMatch; bestBranchMatch=0;}

//	printf("print nodes at the end of treeMatching_noContinualNodes_lengthRatio_simDependent\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %d\n", Tree1->node[i].nid, Tree1->node[i].parentid);
//		 
//	forw (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %d\n", Tree2->node[i].nid, Tree2->node[i].parentid);
//	
//	printf("\n");
}


// *************************************************************************************
// matching leaf nodes, this function is called by treeMatching_hierarchical, 
// it works together with treeMatching_noContinualNodes_lengthRatioAngle_simDependent
// which only matches branching nodes. 
//
// override the above function, add more input parameters:
// 1) falseLeafNodeList, falseLeafNodeNum, indicating which nodes should be exclude from being taking as
// leaf nodes. This is needed when matching leaf nodes on subtrees. Since tree decomposition make some
// continual nodes in the original tree into leaf nodes in the sub tree
// 2) parentVector1 and parentVector2 are the vectors formed by the root of the current tree and
// a reference node (e.g., the parent of the current tree root, or the root of its anterior tree) 
// for which angles of leafnodes are computed
// 3) method: which method to use when matching leaf nodes. method = 0, use bipartite matching; method =1
// use enumeration
// *************************************************************************************

bool treeMatching_leafNodes(swcTree *Tree1, swcTree *Tree2, float *parentVector1, float *parentVector2, 
							V3DLONG *falseLeafNodeList, V3DLONG falseLeafNodeNum, float *parentPathLength, float lengthDiffThre, unsigned char method, 
							float weight_lengthRatio, float weight_angle,
							V3DLONG *&matchedLeafNodeList, unsigned char &matchedLeafNodeNum, float &simMeasure)
{
	V3DLONG i,j,m,n,p,q;

	// get root of tree1 and tree2
	V3DLONG rootid1, rootid2;
	V3DLONG rootidx1, rootidx2;
	
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);
	
	// find all leaf nodes in Tree1 and Tree2
	V3DLONG *tmpleafNodeList1=0, *tmpleafNodeIdx1 = 0, tmpleafNodeNum1;
	V3DLONG *tmpleafNodeList2=0, *tmpleafNodeIdx2 = 0, tmpleafNodeNum2;
	float lengthThre = 0; // prune small leaf nodes, lengthThre is the threshold of the absolution length from the leaf node to its nearest branching point
	
	Tree1->getLeafNode(lengthThre, tmpleafNodeList1, tmpleafNodeIdx1, tmpleafNodeNum1); //ignore leaf nodes in very small branches
	Tree2->getLeafNode(lengthThre, tmpleafNodeList2, tmpleafNodeIdx2, tmpleafNodeNum2);	//ignore leaf nodes in very small branches
	
	bool *falsetag1 = new bool [tmpleafNodeNum1];
	bool *falsetag2 = new bool [tmpleafNodeNum2];
	
	// compute the number of real leaf nodes
	V3DLONG leafNodeNum1=0, leafNodeNum2=0;
	
	for (i=0; i<tmpleafNodeNum1; i++)
	{

		falsetag1[i] = 0;
		
		j=0;
		while (j<falseLeafNodeNum)
		{
			if (falseLeafNodeList[j*2]!=tmpleafNodeList1[i])
				j++;
			else
			{
				falsetag1[i] = 1;
				break;
			}
		}
		
		if (falsetag1[i]==0)
			leafNodeNum1++;
	}
		
	for (i=0; i<tmpleafNodeNum2; i++)
	{

		falsetag2[i] = 0;
		
		j=0;
		while (j<falseLeafNodeNum)
		{
			if (falseLeafNodeList[j*2+1]!=tmpleafNodeList2[i])
				j++;
			else
			{
				falsetag2[i] = 1;
				break;
			}
		}
		
		if (falsetag2[i]==0)
			leafNodeNum2++;
	}
			
	// assign values to leafNodeList1, leafNodeList2 and compute length ratio of each leaf node
	float *length1 = new float [leafNodeNum1];
	float *length2 = new float [leafNodeNum2];
	V3DLONG *leafNodeList1 = new V3DLONG [leafNodeNum1];
	V3DLONG *leafNodeList2 = new V3DLONG [leafNodeNum2];
	
	V3DLONG cnt=-1;
	for (i=0; i<tmpleafNodeNum1; i++)
	{
		if (falsetag1[i] ==0)
		{
			cnt++;
			leafNodeList1[cnt] = tmpleafNodeList1[i];
			Tree1->computeLength(leafNodeList1[cnt], rootid1, length1[cnt]); // compute length of each leafnode to the root of the current tree
//			length1[cnt] /=  parentPathLength[0];
		}
	}

	cnt=-1;
	for (i=0; i<tmpleafNodeNum2; i++)
	{
		if (falsetag2[i] ==0)
		{
			cnt++;
			leafNodeList2[cnt] = tmpleafNodeList2[i];
			Tree2->computeLength(leafNodeList2[cnt], rootid2, length2[cnt]); // compute length of each leafnode to the root of the tree
//			length2[cnt] /=  parentPathLength[1];
		}
	}
		
	// compute the length ratio similarity
	unsigned char num1 = (unsigned char) leafNodeNum1;
	unsigned char num2 = (unsigned char) leafNodeNum2;
	float *simVector_lengthRatio = new float [leafNodeNum1*leafNodeNum2];
	simVector_lengthRatio = calsim_allbranch_length_ratio(length1, length2, num1, num2);	

	for (i=0; i<leafNodeNum1; i++)
	{
		for (j=0; j<leafNodeNum2; j++)
			printf("%f ", simVector_lengthRatio[i*leafNodeNum2+j]);
		printf("\n");
	}
		
	// compute angle and angle similarity
	float *angles1; 
	Tree1->computeLeafNodeAngles(rootid1, parentVector1, leafNodeList1, leafNodeNum1, angles1);
		
	float *angles2;
	Tree2->computeLeafNodeAngles(rootid2, parentVector2, leafNodeList2, leafNodeNum2, angles2);
		
	float *simVector_angle_parent = 0, *simVector_angle_children = 0;			
	calsim_allbranch_angles(angles1, angles2, leafNodeNum1, leafNodeNum2, simVector_angle_parent, simVector_angle_children);		

	// match leaf nodes using either bipartite graph matching or enumeration
	if (method == 0) // use bipartite graph matching to match leaf nodes. does not work well
	{
	
		//compute similarity between leaf ndoes, sim_vector
		float *sim_vector = new float [num1*num2];
		
		Matrix<double> bpMatrix(num1+num2, num1+num2);// bipartite matrix, need to free?	
		for (i=0; i<num1+num2; i++)
		for (j=0; j<num1+num2; j++)
			bpMatrix(i,j) = -INVALID_VALUE;

//		printf("%d, %d\n", num1, num2);
		
		for (i=0; i<num1; i++)
		for (j=0; j<num2; j++)
		{
			sim_vector[i*num2+j] = weight_lengthRatio * simVector_lengthRatio[i*num2+j] + weight_angle * simVector_angle_parent[i*num2+j]; 
			
			bpMatrix(i, j) = 1-sim_vector[i*num2+j]; //range is 0~2
//			printf("%f, %f, %f, %f, %f\n", simVector_lengthRatio[i*num2+j], simVector_angle_parent[i*num2+j], sim_vector[i*num2+j], 1-sim_vector[i*num2+j], bpMatrix(i, j));

		}
		
		Matrix<double> bpMatrixOut(num1+num2, num1+num2);// bipartite matrix, need to free?	

//		printf("print bpMatrixOut before BM \n");

		for (i=0; i<num1+num2; i++)
		{
			for (j=0; j<num1+num2; j++)
			{
				bpMatrixOut(i,j) = bpMatrix(i,j);
//				printf("%f ", bpMatrixOut(i,j));
			}
//			printf("\n");
		}
		
		
		Munkres branchMatch;
		branchMatch.solve(bpMatrixOut); 
		
//		printf("print bpMatrixOut\n");
		
		for (i=0; i<num1+num2; i++)
		{
			for (j=0; j<num1+num2; j++)
			{
//				printf("%f ", bpMatrixOut(i,j));
			}
//			printf("\n");
		}

//		printf("print matched (i,j)\n");
		
		// assign values of matchedLeafNodeList
		matchedLeafNodeNum = 0;	
		for (i=0; i<num1; i++)
		for (j=0; j<num2; j++)
		{
			if (bpMatrixOut(i,j)==0) // find a match
			{
				matchedLeafNodeNum++;
//				printf("%d, %d\n", i,j);
			}
		}
		
//		printf("\n");
		
		
		matchedLeafNodeList = new V3DLONG [matchedLeafNodeNum*2];
		simMeasure = 0;
		
		cnt = 0;
		for (i=0; i<num1; i++)
		for (j=0; j<num2; j++)
		{
			if (bpMatrixOut(i,j)==0) // find a match
			{
				matchedLeafNodeList[cnt*2] = leafNodeList1[i];
				matchedLeafNodeList[cnt*2+1] = leafNodeList2[j];
				simMeasure += bpMatrix(i,j);
				cnt++;
			}
		}
	
	}
	else // if (method !=0)
	{
		// find all possible combinations
		V3DLONG *possibleMatchList=0;	
		V3DLONG num, N_num, R_num;
		findPossibleMatchList(leafNodeNum1, leafNodeNum2, 1, possibleMatchList, num, N_num, R_num);
		
		// find the best match by enumeration
		float tmpSimVal, bestSimVal = INVALID_VALUE;
		V3DLONG best_m, best_r_num;
		
		for (m=0; m<num; m++) // test all possible combinations, and select the maximum
		{
			float tmpSimLengthRatio=0;
			V3DLONG r_num = 0;
			tmpSimVal = 0;
			for (n=0; n<R_num; n++)
			{
				V3DLONG tmpidx =m*(R_num*2)+2*n; 
				V3DLONG idx1 = possibleMatchList[tmpidx];
				V3DLONG idx2 = possibleMatchList[tmpidx+1];

				if ((idx1!=INVALID_VALUE)&&(idx2!=INVALID_VALUE))
				{
					tmpSimLengthRatio += simVector_lengthRatio[idx1*leafNodeNum2+idx2];
					r_num++;
				}
				
			} // for n end
				
			tmpSimVal += (weight_lengthRatio*tmpSimLengthRatio);
			
//			printf("tmpSimVal=%f\n", tmpSimVal);
			
			// plus angle (parent-path) similarity
			float tmpSimAngle = 0;
			for (n=0; n<r_num; n++)
			{
				V3DLONG tmpidx =m*(R_num*2)+2*n; 
				V3DLONG idx1 = possibleMatchList[tmpidx];
				V3DLONG idx2 = possibleMatchList[tmpidx+1];
				
				tmpSimAngle += simVector_angle_parent[idx1*leafNodeNum2 + idx2];
//				printf("tmpidx = %d, idx1 = %d, idx2 = %d \n", tmpidx, idx1, idx2);
			}

//			printf("r_num = %d, num = %d, N_num = %d, R_num = %d, best_r_num = %d, tmpSimVal = %f, tmpSimAngle = %f, bestSimVal = %f \n", r_num, num, N_num, R_num, best_r_num, tmpSimVal, tmpSimAngle, bestSimVal);	
			
			// plus angle (children-path) similarity	
			V3DLONG len = leafNodeNum2*(leafNodeNum2-1)/2;
			V3DLONG ttmp;
							
			for (p=0; p<r_num-1; p++)
			for (q=p+1; q<r_num; q++)
			{
				V3DLONG tmpidxp =m*(R_num*2)+2*p; 
				V3DLONG tmpidxq =m*(R_num*2)+2*q; 
				
				// compute idx1 and idx2
				V3DLONG idx1 = 0, idx2 = 0;
				for (V3DLONG s=0; s< possibleMatchList[tmpidxp]; s++)
					idx1 += (leafNodeNum1-(s+1));

				ttmp = possibleMatchList[tmpidxq] - possibleMatchList[tmpidxp];
				
				if (ttmp<0)
					ttmp = -ttmp;							
				idx1 +=  (ttmp-1);
					

				for (V3DLONG s=0; s< possibleMatchList[tmpidxp+1]; s++)
					idx2 += (leafNodeNum2-(s+1));
				
				ttmp = possibleMatchList[tmpidxq+1] - possibleMatchList[tmpidxp+1];
				if (ttmp<0)
					ttmp = -ttmp;							
				idx2 +=  (ttmp-1);
										
				tmpSimAngle += simVector_angle_children[idx1*len + idx2];
			}

			
			tmpSimAngle /= (r_num*(r_num+1)/2); // normalize to balance length-ratio and angle
			tmpSimAngle *= r_num;
			
			tmpSimVal += (weight_angle*tmpSimAngle);	
			
			if (bestSimVal < tmpSimVal)
			{
				bestSimVal = tmpSimVal;
				best_m = m;
				best_r_num = r_num;
			}
				
		} // for m end
		
		// assign values to output parameters
		if (possibleMatchList!=0)
//		if ((possibleMatchList!=0)&&(bestSimVal/best_m>=0))	//maybe shoud use this
		{		
			matchedLeafNodeNum = best_r_num;			
			matchedLeafNodeList = new V3DLONG [matchedLeafNodeNum*2];
			simMeasure = 0;

			
			for (n=0; n<best_r_num; n++)
			{
				V3DLONG tmpidx =best_m*(R_num*2)+2*n; 
				V3DLONG idx1 = possibleMatchList[tmpidx];
				V3DLONG idx2 = possibleMatchList[tmpidx+1];
			
				matchedLeafNodeList[n*2] = leafNodeList1[idx1];
				matchedLeafNodeList[n*2+1] = leafNodeList2[idx2];
				simMeasure = bestSimVal;
			}
		}
		else // no match
		{
			matchedLeafNodeNum = 0;			
			matchedLeafNodeList = 0;
			simMeasure = 0;

		}
		if (possibleMatchList) {delete []possibleMatchList; possibleMatchList=0;}
	}


	//delete pointers
	if (leafNodeList1) {delete []leafNodeList1; leafNodeList1 =0;}
	if (leafNodeList2) {delete []leafNodeList2; leafNodeList2 =0;}
	if (tmpleafNodeList1) {delete []tmpleafNodeList1; tmpleafNodeList1 =0;}
	if (tmpleafNodeList2) {delete []tmpleafNodeList2; tmpleafNodeList2 =0;}
	if (tmpleafNodeIdx1) {delete []tmpleafNodeIdx1; tmpleafNodeIdx1 =0;}
	if (tmpleafNodeIdx2) {delete []tmpleafNodeIdx2; tmpleafNodeIdx2 =0;}
	if (simVector_lengthRatio) {delete []simVector_lengthRatio; simVector_lengthRatio = 0;}
	if (simVector_angle_parent) {delete []simVector_angle_parent; simVector_angle_parent = 0;}
	if (simVector_angle_children) {delete []simVector_angle_children; simVector_angle_children = 0;}
	if (falsetag1) {delete []falsetag1; falsetag1 = 0;}
	if (falsetag2) {delete []falsetag2; falsetag2 = 0;}
	if (length1) {delete []length1; length1 = 0;}
	if (length2) {delete []length2; length2 = 0;}
	if (angles1) {delete []length1; length1 = 0;}
	if (angles2) {delete []length2; length2 = 0;}

} 


// *************************************************************************************
// Tree matching using hierarchical DP scheme 
// Input: swcTree *Tree1, swcTree *Tree2, b_scale (incidate if scale matters, use lengthratio if
// 
// output: V3DLONG *&matchingList, float &simMeasure

// some important array variables used in the program
// matchingList: (Tree1->treeNodeNum)*(MAX_CHILDREN_NUM+2) elements, 
// matchingList_level: (subTree1->treeNodeNum)*(MAX_CHILDREN_NUM+1) elements,
// matchingList_leafNode: (Tree1->treeNodeNum*2) elements
// matchedLeafNodeList: (# matched leaves in a subtree *2) elements
// branchIdx1, branchIdx2: 

// some parameters used in the program
// lengthRatioThre: determine which nodes should be selected as significant branching nodes
// subtreeRatioThre: determine how far decendent should be considered in best subtree matching
// these two variables may change with the hierarchy of the tree

// the trees need to appropriately pruned by setting appropriate lengthRatioThre values at
// each hierarchy, so that only significant branching points are kept, those branching points
// in tiny sub-branches are removed. They can be matched in later hierarchy
// *************************************************************************************

bool treeMatching_hierarchical(swcTree *Tree1, swcTree *Tree2, bool b_scale, V3DLONG *&matchingList, float &simMeasure, string outfilenamedir)
{


	// parameter initialization
		
	V3DLONG i, j, k, m,n; 
	float weight_lengthRatio = 1; 
	float weight_angle = 0;

			
	float *lengthRatioThre = new float [2];	
	lengthRatioThre[0] = 0.2;
	lengthRatioThre[1] = 0.1;

//	lengthRatioThre[0] = 0.1;
//	lengthRatioThre[1] = 0.05;

	float branchingNodeDisRatioThre = 0.2; // for generating supernode purpose
	
	V3DLONG *seedNodeID1 = 0, *seedNodeID2 =0;
	swcTree **subTrees1 =0, **subTrees2 = 0;
	V3DLONG *branchIdx1 = 0, *branchIdx2 = 0;
	V3DLONG subTreeNum1, subTreeNum2;
	V3DLONG seedNodeNum=0, seedNodeNumOld=0; // determine if the process should stop

	V3DLONG *matchingList_level = 0;	// matchingList of each level, which will be accumulated into matchingList
	float simMeasure_level; //similarity measure of each level, which will be accumulated into simMeasure_level
	unsigned char level = 0;
//	V3DLONG depthThre;	
	float subtreeRatioThre = 0.2; 
	// determine how far nodes in subtree can be considered when determining the best subtree in DP process, 
	//this value can be different for differnt levels, 0.2~0.4 are ok for the first level
	
	matchingList = new V3DLONG [(Tree1->treeNodeNum)*(MAX_CHILDREN_NUM+2)]; 
	// different from matchingList_level, matchingList contains all nodes in Tree1, it also adds a column indicating which level the node is matched
	// the first element of each node is its matching node in Tree2, the next MAX_CHILDREN_NUM are its matching branches in Tree2, the last element 
	// indicates which level it is matched

	V3DLONG *matchingList_leafNode = new V3DLONG [Tree1->treeNodeNum*2]; 
	// different from matchingList, this records how leaf nodes are matched, each matched leaf nodes has two columns, the first column is the nodeid in Tree2, the second is level indicator
	// note that matchingList_leafNode and matchingList will be combined into matchingList for output
	float simMeasure_leafNode = 0;
	
	// initial (level 0) significant branching nodes matching
//	two earlier verions:
////	treeMatching_noContinualNodes_lengthRatioAngle_simDependent(Tree1, Tree2, matchingList_level, branchMatchingList_level, simMeasure_level, lengthRatioThre);
//	depthThre = 2;
//	treeMatching_noContinualNodes_lengthRatioAngle_simDependent(Tree1, Tree2, matchingList_level, simMeasure_level, lengthRatioThre, depthThre);

//	before 20090914
	treeMatching_noContinualNodes_lengthRatioAngle_simDependent(Tree1, Tree2, b_scale, matchingList_level, simMeasure_level, lengthRatioThre, subtreeRatioThre, weight_lengthRatio, weight_angle);

//
////	20090914, consider super nodes to handle branching order difference between two trees
//	treeMatching_noContinualNodes_lengthRatioAngle_simDependent_superNodes(Tree1, Tree2, matchingList_level, simMeasure_level, lengthRatioThre, subtreeRatioThre, weight_lengthRatio, weight_angle, branchingNodeDisRatioThre);

	// initialize matchingList, and simMeasure
	int cnt =-1;
	int len = MAX_CHILDREN_NUM+1;
	
	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		if (matchingList_level[i*len]>0) //there is a match, otherwise its value is INVALID_VALUE=-9999
		{
			for (j=0; j<len; j++)
				matchingList[i*(len+1)+j] = matchingList_level[i*len+j];
			
			matchingList[i*(len+1)+len] = level;
		}
		else
		{
		
			for (j=0; j<len+1; j++)
				matchingList[i*(len+1)+j] = INVALID_VALUE;
		
		}
	}
	
	simMeasure = simMeasure_level;

	if (matchingList_level) {delete []matchingList_level; matchingList_level = 0;} 
	
	// initialzie matchingList_leafNode and simMeasure_leafNode
	for (i=0; i<Tree1->treeNodeNum*2; i++)
		matchingList_leafNode[i] = INVALID_VALUE;

	// get roots of the original tree (for matching leaf node purpose)
	V3DLONG rootid1, rootid2, rootidx1, rootidx2;
	
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);
	
	V3DLONG *subTreeMatchingList = 0;
	
	// hierarchical matching	
	while (1)
	{
					
//		depthThre = 3;
		subtreeRatioThre = 0.6; // note this value is different from level 0 matching to allow detailed matching as level increases
				
		// compute the number of matched branching ndoes
		seedNodeNumOld = seedNodeNum;
			
		// compute seedNodeNum
		seedNodeNum=0;	
		for(i=0; i<Tree1->treeNodeNum; i++)
		{	
			if ( matchingList[i*(len+1)]>0)
				seedNodeNum++;
		}

		printf("seedNodeNumOld = %d, seedNodeNum = %d\n", seedNodeNumOld,  seedNodeNum);

		if ((seedNodeNum-seedNodeNumOld)<=1) // if number of seeded nodes does not change significantly, stop hierarchical matching process
			break;

		char branchmatchfiletmp[200]; // files record how branches are matched in each level for debugging and displaying purpose
		string branchmatchfile; // files record how branches are matched in each level for debugging and displaying purpose
		
		sprintf(branchmatchfiletmp, "branchmatch_level%d.txt", level);
		branchmatchfile = outfilenamedir + branchmatchfiletmp;
		
		FILE *file;
		file  = fopen(branchmatchfile.c_str(), "wt");

		if (file == NULL)
		{
			printf("error to open file\n");
			return 0; 
		}

		level++;
		printf("level = %d\n", level);

		seedNodeID1 = new V3DLONG [seedNodeNum];
		seedNodeID2 = new V3DLONG [seedNodeNum];
		
		// print matched branching nodes in Tree1 and Tree2
		j = 0;
		for(i=0; i<Tree1->treeNodeNum; i++)
		{	
			if ( matchingList[i*(len+1)]>0)
			{
				seedNodeID1[j] = Tree1->node[i].nid;
				seedNodeID2[j] = matchingList[i*(len+1)];
				printf("%d, %d\n", seedNodeID1[j], seedNodeID2[j]);
				j++;
			}
		}	
				
		// decompse each tree according to matched branching nodes

		if (subTrees1) 
		{
			for (i=0; i<subTreeNum1; i++)
				if (subTrees1[i]) {delete subTrees1[i]; subTrees1[i]=0;}
			delete subTrees1; subTrees1=0;
		}

		if (subTrees2) 
		{
			for (i=0; i<subTreeNum2; i++)
				if (subTrees2[i]) {delete subTrees2[i]; subTrees2[i]=0;}
			delete subTrees2; subTrees2=0;
		}	
				
//		Tree1->decompose(seedNodeID1, seedNodeNum, subTrees1, subTreeNum1, branchIdx1); //decompose the orginal tree, not sub-branches
//		Tree2->decompose(seedNodeID2, seedNodeNum, subTrees2, subTreeNum2, branchIdx2); //decompose the orginal tree, not sub-branches


		V3DLONG *subTreeSeeds1 = new V3DLONG [seedNodeNum*(MAX_CHILDREN_NUM+1)];
		V3DLONG *subTreeSeeds2 = new V3DLONG [seedNodeNum*(MAX_CHILDREN_NUM+1)];
		
		Tree1->decompose(seedNodeID1, seedNodeNum, subTrees1, subTreeNum1, branchIdx1, subTreeSeeds1); //decompose the orginal tree, not sub-branches, 20090911 revise
		Tree2->decompose(seedNodeID2, seedNodeNum, subTrees2, subTreeNum2, branchIdx2, subTreeSeeds2); //decompose the orginal tree, not sub-branches, , 20090911 revise
		
		printf("subTreeNum1 = %d, subTreeNum2 = %d\n", subTreeNum1, subTreeNum2);

//#ifdef DEBUG
			// output matched branches, for displaying purpose
			char branchfiletmp[200];
			string branchfile; // files record how branches are matched in each level for debugging and displaying purpose
		
					
			for (i=0; i<subTreeNum1; i++)
			{
				sprintf(branchfiletmp, "Tree1_level%d_branch%d.swc", level-1, i);
				branchfile = outfilenamedir + branchfiletmp;
				subTrees1[i]->writeSwcFile(branchfile.c_str());
			}
			
			for (i=0; i<subTreeNum2; i++)
			{
				sprintf(branchfiletmp, "Tree2_level%d_branch%d.swc", level-1, i);
				branchfile = outfilenamedir + branchfiletmp;
				subTrees2[i]->writeSwcFile(branchfile.c_str());
			}

			char branchseedfiletmp[200];
			string branchseedfile;
			
			sprintf(branchseedfiletmp, "Tree1_branchseeds_level%d.txt", level-1);
			branchseedfile = outfilenamedir + branchseedfiletmp;
			
			FILE *fileseed;
			fileseed  = fopen(branchseedfile.c_str(), "wt");

			if (fileseed == NULL)
			{
				printf("error to open file\n");
				return 0; 
			}
		
								
			for (i=0; i<subTreeNum1; i++)
			{
				for (j=0; j<=MAX_CHILDREN_NUM; j++)
				{
					fprintf(fileseed, "%d ", subTreeSeeds1[i*(MAX_CHILDREN_NUM+1)+j]);
				}
				fprintf(fileseed, "\n");
			}
			fclose(fileseed);
			
			sprintf(branchseedfiletmp, "Tree2_branchseeds_level%d.txt", level-1);
			branchseedfile = outfilenamedir + branchseedfiletmp;
			
			fileseed  = fopen(branchseedfile.c_str(), "wt");

			if (fileseed == NULL)
			{
				printf("error to open file\n");
				return 0; 
			}
		
								
			for (i=0; i<subTreeNum2; i++)
			{
				for (j=0; j<=MAX_CHILDREN_NUM; j++)
				{
					fprintf(fileseed, "%d ", subTreeSeeds2[i*(MAX_CHILDREN_NUM+1)+j]);
				}
				fprintf(fileseed, "\n");
			}
			fclose(fileseed);
			
//#endif


		// determine pairs of subtrees to be matched
		V3DLONG cnt = -1;
		V3DLONG *idx1 = new V3DLONG [seedNodeNum*len]; // order each matched branch in tree1
		// the same size as branchIdx1, idx1 set the first element of each seed node to INVALID_VALUE, 
		// different from branchIdx1, idx1 number the branches of branchIdx1 that have been matched, it has a format like -9999 0 1 -9999 2 -9999 -9999
		V3DLONG *idx2 = new V3DLONG [seedNodeNum*len]; //order each matched branch in tree2
		// the same size as branchIdx2, idx2 set the first element of each seed node to INVALID_VALUE
		//  different from branchIdx2, idx2 number the branches of branchIdx1 that have been matched, it has a format like -9999 0 1 -9999 2 -9999 -9999
				
		for (i=0; i<seedNodeNum; i++)
		{
			idx1[i*len] = INVALID_VALUE;
						
			for (j=1; j<len; j++)
			{
				if (branchIdx1[i*len+j]!=INVALID_VALUE)
				{
					cnt++;
					idx1[i*len + j] = cnt;					
				}
				else
					idx1[i*len + j] = INVALID_VALUE;
			}
		}
			
		cnt = -1;
		for (i=0; i<seedNodeNum; i++)
		{
			idx2[i*len] = INVALID_VALUE;
			
			for (j=1; j<len; j++)
			{
				if (branchIdx2[i*len+j]!=INVALID_VALUE)
				{
					cnt++;
					idx2[i*len + j] = cnt;					
				}
				else
					idx2[i*len + j] = INVALID_VALUE;
			}

		}
	
		//match subtree pairs, note branchIdx1 and branchIdx2 are of different sizes of matchingList
		V3DLONG subTrees1_idx, subTrees2_idx;
		V3DLONG subTree1_branchCnt = -1;
		
		if (subTreeMatchingList) {delete []subTreeMatchingList; subTreeMatchingList = 0;}
		
		subTreeMatchingList = new V3DLONG [subTreeNum1];
		
		for (i=0; i<subTreeNum1; i++)
			subTreeMatchingList[i] = INVALID_VALUE;
			
		for (i=0; i<seedNodeNum; i++)
		{
	
			printf("i=%d\n", i);
						
			for (j=1; j<len; j++)
			{
				if (branchIdx1[i*len+j]!=INVALID_VALUE)
				{
					printf("i=%d, j=%d\n", i,j);
					subTrees1_idx = idx1[i*len+j]; //get index of the tree
					subTrees2_idx = INVALID_VALUE; // subTree2_idx needs to be searched for
					
//					subTreeMatchingList[subTrees1_idx] = subTree1_idx;
					
					V3DLONG seedNodeIdx1;
					Tree1->getIndex(seedNodeID1[i], seedNodeIdx1);
					
					V3DLONG tmp = matchingList[seedNodeIdx1*(len+1)+branchIdx1[i*len+j]+1];
						
					//search from branchIdx2
					m = 0;
					while (m<MAX_CHILDREN_NUM)
					{
						if (tmp == branchIdx2[i*len+m+1])
						{
							subTrees2_idx = idx2[i*len+m+1];
							break;
						}
						else 
							m++;
					}
					
					printf("subTrees1_idx = %d, subTrees2_idx = %d\n",  subTrees1_idx, subTrees2_idx);


					// sub-branch matching			
					if (subTrees2_idx!=INVALID_VALUE)
					{

					
//						subTree1_branchCnt++;
//						subTreeMatchingList[subTree1_branchCnt*2] = subTree1_idx;
//						subTreeMatchingList[subTree1_branchCnt*2+1] = subTree2_idx;
						subTreeMatchingList[subTrees1_idx] = subTrees2_idx;
						
						fprintf(file, "%d %d\n", subTrees1_idx, subTrees2_idx); 
						
						printf("subTrees1[subTrees1_idx]->treeNodeNum = %d, subTrees2[subTrees2_idx]->treeNodeNum = %d\n", subTrees1[subTrees1_idx]->treeNodeNum, subTrees2[subTrees2_idx]->treeNodeNum);
						
						if ((subTrees1[subTrees1_idx]->treeNodeNum<50) || (subTrees2[subTrees2_idx]->treeNodeNum<50)) // if the number of nodes in the trees are small, directly match all nodes, no need to do further hierarchical matching
							lengthRatioThre[0] = lengthRatioThre[1] = 0;
						
//				for (m=0; m<subTrees2[8]->treeNodeNum; m++)
//			printf("%d, %d\n", subTrees2[8]->node[m].nid, subTrees2[8]->node[m].parentid); 
//	

						// match branching nodes
						
//	//					treeMatching_noContinualNodes_lengthRatioAngle_simDependent(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], matchingList_level, branchMatchingList_level, simMeasure_level, lengthRatioThre);
//						treeMatching_noContinualNodes_lengthRatioAngle_simDependent(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], matchingList_level, simMeasure_level, lengthRatioThre, depthThre);
//						if ((subTrees1_idx==7)&&(subTrees2_idx==8))
//						{
//							for (m=0; m<subTrees2[subTrees2_idx]->treeNodeNum; m++)
//								printf("%d, %d\n", subTrees2[subTrees2_idx]->node[m].nid, subTrees2[subTrees2_idx]->node[m].parentid); 
//						}

//				for (m=0; m<subTrees2[8]->treeNodeNum; m++)
//			printf("%d, %d\n", subTrees2[8]->node[m].nid, subTrees2[8]->node[m].parentid); 
						
						treeMatching_noContinualNodes_lengthRatioAngle_simDependent(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], b_scale, matchingList_level, simMeasure_level, lengthRatioThre, subtreeRatioThre,  weight_lengthRatio, weight_angle);
//						treeMatching_noContinualNodes_lengthRatioAngle_simDependent_superNodes(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], matchingList_level, simMeasure_level, lengthRatioThre, subtreeRatioThre,  weight_lengthRatio, weight_angle, branchingNodeDisRatioThre);

//				for (m=0; m<subTrees2[8]->treeNodeNum; m++)
//			printf("%d, %d\n", subTrees2[8]->node[m].nid, subTrees2[8]->node[m].parentid); 

						
						// update matchingList
						
						cnt = -1;
						for (m=0; m<subTrees1[subTrees1_idx]->treeNodeNum; m++)
						{

							V3DLONG idx1;
							Tree1->getIndex(subTrees1[subTrees1_idx]->node[m].nid, idx1);

							if ((matchingList_level[m*len]>0) &&(matchingList[idx1*(len+1)+len]==INVALID_VALUE)) 
							// the second condition is to make sure that if a node has been matched in a previous level, it will take that value, do not reasign, as later levels are prune to errors
							{
								for (n=0; n<len; n++)
									matchingList[idx1*(len+1)+n] = matchingList_level[m*len+n];
								
								matchingList[idx1*(len+1)+len] = level;
							}
						}

						
						//update simMeasure
						simMeasure += simMeasure_level;
						
						
						// delete pointers
	//					if (branchMatchingList_level) {delete []branchMatchingList_level; branchMatchingList_level=0;}					
						if (matchingList_level) {delete []matchingList_level; matchingList_level = 0;}
					}
				}
//				j++;
//				printf("j=%d\n",j);

			} // while 
		}// for i, finish matching subtrees
		
				
		//delete pointers related to decompose function
		if (idx1) {delete []idx1, idx1 = 0;}
		if (idx2) {delete []idx2, idx2 = 0;}
		if (seedNodeID1) {delete []seedNodeID1; seedNodeID1=0;}
		if (seedNodeID2) {delete []seedNodeID2; seedNodeID2=0;}
		if (branchIdx1) {delete []branchIdx1; branchIdx1=0;}
		if (branchIdx2) {delete []branchIdx2; branchIdx2=0;}
		if (subTreeSeeds1) {delete []subTreeSeeds1; subTreeSeeds1=0;}
		if (subTreeSeeds2) {delete []subTreeSeeds2; subTreeSeeds2=0;}
		
		// note subTree1 and subTree2 are deleted before calling decompose
		// subTreeMatchingList is also released before
		
		fclose(file);
		
	} // while end
	
	// match leaf node of each branch

	// find the parent node of the matching nodes, exclude them from being taken as leaf nodes in leaf node matching for each subtree
	// so that real leaf nodes are matched
	
	V3DLONG *falseLeafNodeList = new V3DLONG [(seedNodeNum-1)*2]; //the parents of pairs of matching nodes should not be taken as leaf nodes in the subtrees
	
	cnt = -1;
	
	for (int i=0; i<Tree1->treeNodeNum; i++)
	{

		if (matchingList[i*(len+1)]>0)
		{
		
			V3DLONG idxx2, tmp1, tmp2;
		
			tmp1 = Tree1->node[i].parentid;
			Tree2->getIndex(matchingList[i*(len+1)], idxx2);
			tmp2 = Tree2->node[idxx2].parentid;
			
			if ((tmp1!=-1)&&(tmp2!=-1))
			{
				cnt++;
				falseLeafNodeList[cnt*2] = tmp1;
				falseLeafNodeList[cnt*2+1] = tmp2;
			}
		}
	}
	
	

	for (i=0; i<subTreeNum1; i++)
	{
		float lengthDiffThre = 0;
		float *parentPathLength = new float [2];
		V3DLONG subtree1_rootid1, subtree1_rootidx1, subtree2_rootid2, subtree2_rootidx2;
		unsigned char matchedLeafNodeNum;
		V3DLONG *matchedLeafNodeList = 0;
		float simMeasure_leafNode_level;
		V3DLONG subTrees1_idx = i;
		V3DLONG subTrees2_idx = subTreeMatchingList[i];
		
		if (subTrees2_idx!=INVALID_VALUE)
		{
			subTrees1[subTrees1_idx]->getRootID(subtree1_rootid1, subtree1_rootidx1);
			subTrees2[subTrees2_idx]->getRootID(subtree2_rootid2, subtree2_rootidx2);
			
			// first compute parentPathLength

			Tree1->computeLength(rootid1, subtree1_rootid1, parentPathLength[0]);
			Tree2->computeLength(rootid2, subtree2_rootid2, parentPathLength[1]);


			if ((parentPathLength[0]!=0)&&(parentPathLength[1]!=0)) // otherwise, it means one or both of the global tree roots are the same as the subtree roots, then do not match
			{
				// match leaf ndoes
								
//				treeMatching_leafNodes(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], parentPathLength, lengthDiffThre, matchedLeafNodeList, matchedLeafNodeNum, simMeasure_leafNode_level);
//				// note matchedLeafNodeList contains number of matched leaves *2, the first is the nodeid in tree1, the second is the nodeid in tree2
//				// matchingList_leafNode contains number of nodes in the original Tree1 *2, the first is the nodeid in tree2, the second is the level at which the nodes are matched
				
//				treeMatching_leafNodes(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], falseLeafNodeList, seedNodeNum-1, parentPathLength, lengthDiffThre, matchedLeafNodeList, matchedLeafNodeNum, simMeasure_leafNode_level);
////				// note matchedLeafNodeList contains number of matched leaves *2, the first is the nodeid in tree1, the second is the nodeid in tree2
////				// matchingList_leafNode contains number of nodes in the original Tree1 *2, the first is the nodeid in tree2, the second is the level at which the nodes are matched
					
				// compute parentVector1 and parentVector2, which are the vectors connecting the root of the current subtrees and their direct parent,
				// they are parameters transfered to function treeMatching_leafNodes for angle matching of leaf nodes
				float parentVector1[3], parentVector2[3]; // used to compute angle
				V3DLONG nidx1, nidx2; //the indexes of the root of the subtree in the original tree
				V3DLONG pidx1, pidx2; //the indexes of the nidx1, nidx2's parents

				Tree1->getIndex(subTrees1[subTrees1_idx]->node[subtree1_rootidx1].nid, nidx1); 
				
				if (Tree1->node[nidx1].parentid == -1)
				{
					parentVector1[0] = 0; 
					parentVector1[1] = 0; 
					parentVector1[2] = 0; 
					
				}
				else
				{
					Tree1->getIndex(Tree1->node[nidx1].parentid, pidx1);
					parentVector1[0] =  subTrees1[subTrees1_idx]->node[subtree1_rootidx1].x - Tree1->node[pidx1].x;
					parentVector1[1] =  subTrees1[subTrees1_idx]->node[subtree1_rootidx1].y - Tree1->node[pidx1].y;
					parentVector1[2] =  subTrees1[subTrees1_idx]->node[subtree1_rootidx1].z - Tree1->node[pidx1].z;
					
				}

				Tree2->getIndex(subTrees2[subTrees2_idx]->node[subtree2_rootidx2].nid, nidx2); 
				
				if (Tree2->node[nidx2].parentid == -1)
				{
					parentVector2[0] = 0; 
					parentVector2[1] = 0; 
					parentVector2[2] = 0; 
				}
				else
				{
					Tree2->getIndex(Tree2->node[nidx2].parentid, pidx2);
					parentVector2[0] =  subTrees2[subTrees2_idx]->node[subtree2_rootidx2].x - Tree2->node[pidx2].x;
					parentVector2[1] =  subTrees2[subTrees2_idx]->node[subtree2_rootidx2].y - Tree2->node[pidx2].y;
					parentVector2[2] =  subTrees2[subTrees2_idx]->node[subtree2_rootidx2].z - Tree2->node[pidx2].z;
				}


				unsigned char method = 1; //0: bipartite matching; 1: enumeration
				treeMatching_leafNodes(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], parentVector1, parentVector2, 
									   falseLeafNodeList, seedNodeNum-1, parentPathLength, lengthDiffThre, method,
									   weight_lengthRatio,  weight_angle,
									   matchedLeafNodeList, matchedLeafNodeNum, simMeasure_leafNode_level);				
																																								
				//update matchingList_leafNode

				cnt = -1;
				for (m=0; m<matchedLeafNodeNum; m++)
				{

					V3DLONG idx1, idx2;
					Tree1->getIndex(matchedLeafNodeList[m*2], idx1);
					Tree2->getIndex(matchedLeafNodeList[m*2+1], idx2);

		//								if (matchingList_leafNode[idx1*2+1]==INVALID_VALUE)
		//								{
						matchingList_leafNode[idx1*2] = matchedLeafNodeList[m*2+1];
						matchingList_leafNode[idx1*2+1] = level;
		//								}
				}

				//update simMeasure_leafNode
				simMeasure_leafNode += simMeasure_leafNode_level;
				
				if (parentPathLength) {delete []parentPathLength; parentPathLength=0;}
				if (matchedLeafNodeList) {delete []matchedLeafNodeList; matchedLeafNodeList = 0;}
			}
		}
	}

	
	// combine matching of branching nodes and leaf nodes, i.e., combine matchingList and matchingList_leafNode, 
	// simMeasure and simMeasure_leafNode		
	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		if (matchingList_leafNode[i*2]!=INVALID_VALUE)
		{
			matchingList[i*(len+1)] = matchingList_leafNode[i*2];
			matchingList[i*(len+1)+len] = matchingList_leafNode[i*2+1];
		}
	}

	simMeasure += simMeasure_leafNode;
	
	// delete pointer related to treeMatching_noContinualNodes_lengthRatioAngle_simDependent
	if (falseLeafNodeList) {delete []falseLeafNodeList; falseLeafNodeList=0;}
	if (lengthRatioThre) {delete []lengthRatioThre; lengthRatioThre=0;}
	if (matchingList_leafNode) {delete []matchingList_leafNode; matchingList_leafNode=0;}
	
//	if (branchMatchingList) {delete []branchMatchingList; branchMatchingList=0;}
	
}


//************************************************************************************
// OLD FUNCTIONS
//************************************************************************************


//************************************************************************************
//// matching leaf nodes, this function is called by treeMatching_hierarchical, 
//// it works together with treeMatching_noContinualNodes_lengthRatioAngle_simDependent
//// which only matches branching nodes. It uses bipartite graph matching 
//// note that if this function is called before sub-branch matching, it may make the matched 
//// branches inconsistent with matched leaves.
//// parentPathLength: the length of the parent path which is the path from the root of the current tree 
//// (a subtree of the global tree) to the global tree root
//// if parentPathLength[0] = 1,  parentPathLength[1] = 1, then absolute length comparison is applied to leaf nodes
//// matchedLeafNodeList consists of matchedLeafNodeNum*2 elements, storing nodeid of each matched leaf pair
//************************************************************************************

//bool treeMatching_leafNodes(swcTree *Tree1, swcTree *Tree2, float *parentPathLength, float lengthDiffThre, V3DLONG *&matchedLeafNodeList, unsigned char &matchedLeafNodeNum, float &simMeasure)
//{
//	V3DLONG i,j;
//	
//	// get root of tree1 and tree2
//	V3DLONG rootid1, rootid2;
//	V3DLONG rootidx1, rootidx2;
//	
//	Tree1->getRootID(rootid1, rootidx1);
//	Tree2->getRootID(rootid2, rootidx2);
//	
//	// find all leaf nodes in Tree1 and Tree2
//	V3DLONG *leafNodeList1=0, *leafNodeIdx1 = 0, leafNodeNum1;
//	V3DLONG *leafNodeList2=0, *leafNodeIdx2 = 0, leafNodeNum2;
//	float lengthThre = 0; // prune small leaf nodes
//	
//	Tree1->getLeafNode(lengthThre, leafNodeList1, leafNodeIdx1, leafNodeNum1); //ignore leaf nodes in very small branches
//	Tree2->getLeafNode(lengthThre, leafNodeList2, leafNodeIdx2, leafNodeNum2);	//ignore leaf nodes in very small branches
//	
//	// compute length ratio of each leaf node
//	float *length1 = new float [leafNodeNum1];
//	float *length2 = new float [leafNodeNum2];
//	
//	for (i=0; i<leafNodeNum1; i++)
//	{
//		Tree1->computeLength(leafNodeList1[i], rootid1, length1[i]); // compute length of each leafnode to the root of the tree
//		//		length1[i] /=  parentPathLength[0]; //comment on 20100128, for leaf nodes, normalized by parentPathLength[0] may diminish the differences
//		//		printf("leafnodeid = %d, subtree root = %d, length = %f\n", leafNodeList1[i], rootid1, length1[i]);
//	}
//	
//	for (i=0; i<leafNodeNum2; i++)
//	{
//		Tree2->computeLength(leafNodeList2[i], rootid2, length2[i]); // compute length of each leafnode to the root of the tree		
//		//		length2[i] /=  parentPathLength[1];//comment on 20100128, for leaf nodes, normalized by parentPathLength[0] may diminish the differences
//		//		printf("leafnodeid = %d, subtree root = %d, length = %f\n", leafNodeList2[i], rootid2, length2[i]);
//		
//	}
//	
//	// bipartite matching of leaf nodes
//	unsigned char num1 = (unsigned char) leafNodeNum1;
//	unsigned char num2 = (unsigned char) leafNodeNum2;
//	float *sim_vector = new float [leafNodeNum1*leafNodeNum2];
//	sim_vector = calsim_allbranch_length_ratio(length1, length2, num1, num2);	
//	
//	// bipartite matching
//	
//	Matrix<double> bpMatrix(num1+num2, num1+num2);// bipartite matrix, need to free?	
//	for (i=0; i<num1+num2; i++)
//		for (j=0; j<num1+num2; j++)
//			bpMatrix(i,j) = -INVALID_VALUE;
//	
//	for (i=0; i<num1; i++)
//		for (j=0; j<num2; j++)
//		{
//			//		if (sim_vector[i*num2+j]>-1)
//			bpMatrix(i, j) = 1-sim_vector[i*num2+j]; //range is 0~2
//		}
//	
//	Matrix<double> bpMatrixOut(num1+num2, num1+num2);// bipartite matrix, need to free?	
//	for (i=0; i<num1+num2; i++)
//		for (j=0; j<num1+num2; j++)
//			bpMatrixOut(i,j) = bpMatrix(i,j);
//	
//	Munkres branchMatch;
//	branchMatch.solve(bpMatrixOut); 
//	
//	// assign values of matchedLeafNodeList
//	matchedLeafNodeNum = 0;	
//	for (i=0; i<num1; i++)
//		for (j=0; j<num2; j++)
//		{
//			if (bpMatrixOut(i,j)==0) // find a match
//				matchedLeafNodeNum++;
//		}
//	
//	matchedLeafNodeList = new V3DLONG [matchedLeafNodeNum*2];
//	simMeasure = 0;
//	
//	unsigned char cnt = 0;
//	for (i=0; i<num1; i++)
//		for (j=0; j<num2; j++)
//		{
//			if (bpMatrixOut(i,j)==0) // find a match
//			{
//				matchedLeafNodeList[cnt*2] = leafNodeList1[i];
//				matchedLeafNodeList[cnt*2+1] = leafNodeList2[j];
//				simMeasure += bpMatrix(i,j);
//				cnt++;
//			}
//		}
//	
//	//delete pointers
//	if (leafNodeList1) {delete []leafNodeList1; leafNodeList1 =0;}
//	if (leafNodeList2) {delete []leafNodeList2; leafNodeList2 =0;}
//	if (leafNodeIdx1) {delete []leafNodeIdx1; leafNodeIdx1 =0;}
//	if (leafNodeIdx2) {delete []leafNodeIdx2; leafNodeIdx2 =0;}
//	if (length1) {delete []length1; length1 = 0;}
//	if (length2) {delete []length2; length2 = 0;}
//	
//} 


////************************************************************************************
//// 20090914, consider super nodes to handle branching order difference between two trees
//// so that matching can be more flexible and won't collapse when the branching order of
//// the two trees are different
//// revised from function treeMatching_noContinualNodes_lengthRatioAngle_simDependent()
////
//// NOT DEBUGGED YET
////************************************************************************************

//bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent_superNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure, 
//																			float *lengthRatioThre, float subtreeRatioThre, float weight_lengthRatio, 
//																			float weight_angle, float branchingNodeDisRatioThre)
//{
//	V3DLONG i, j, m, n, p,q;
//	
//	// -----------------------------------------------------------------
//	// generate trees only containing root and significant branching points
//	// remove continual points, small branching points, and leaf nodes
//	// the input tree pointers are backed up in Tree1_old, Tree2_old
//	// the trees after removing continual nodes are Tree1 and Tree2
//	// -----------------------------------------------------------------	
//	swcTree *Tree1_old =0, *Tree2_old = 0; 
//	swcTree *newTree1 =0, *newTree2 = 0;
//	
//	// trees for super node 
//	swcTree *Tree1_sn =0, *Tree1_sn_old = 0; // allocate within for sni loop, released before the end of the loop
//	swcTree *Tree2_sn =0, *Tree2_sn_old = 0; // allocate within for snj loop, released before the end of the loop
//	
//	bool *removeNodeTag1=0, *removeNodeTag2=0; //released at the end of the function
//	V3DLONG **removeBranchTag1=0, **removeBranchTag2=0; //released at the end of the function	
//	V3DLONG **removeBranchTag1_sn=0, **removeBranchTag2_sn=0; // released before the end of for sni and for snj loop
//	
//	bool keepLeaf=0;
//	
//	Tree1_old = Tree1;
//	Tree2_old = Tree2;
//	
//	// remove continual nodes, leaf nodes (if keepLeaf=0) , and small branching points, note that features will be still computed on the Tree1_old
//	// and call the new tree Tree1
//	Tree1->removeInsignificantNodes(lengthRatioThre, newTree1, keepLeaf, removeNodeTag1, removeBranchTag1); 
//	
//	Tree1 = newTree1; // the pruned tree is deleted and Tree1 point to old tree before the end of the function
//	newTree1 = 0;
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//	{
//		printf("%d ", Tree1->node[i].nid);
//	}
//	printf("\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//	{
//		printf("%d ", Tree1->node[i].nid);
//		
//		for (j=0; j<MAX_CHILDREN_NUM; j++)
//			printf("%d ", removeBranchTag1[i][j]);
//		printf("\n");
//	}
//	printf("\n");
//	
//	// save to swc and dot files
//	Tree1->writeSwcFile("Tree1.swc"); 
//	Tree1->genGraphvizFile("Tree1.dot");
//	Tree1_old->genGraphvizFile("Tree1_old.dot");
//	
//	// remove continual nodes, leaf nodes (if keepLeaf=0), and small branching points in Tree2 
//	// and call the new tree Tree2
//	Tree2->removeInsignificantNodes(lengthRatioThre, newTree2, keepLeaf, removeNodeTag2, removeBranchTag2); 
//	
//	Tree2 = newTree2; // the pruned tree is deleted and Tree1 point to old tree before the end of the function
//	newTree2 = 0;
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//	{
//		printf("%d ", Tree2->node[i].nid);
//	}
//	printf("\n");
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//	{
//		printf("%d ", Tree2->node[i].nid);
//		
//		for (j=0; j<MAX_CHILDREN_NUM; j++)
//			printf("%d ", removeBranchTag2[i][j]);
//		printf("\n");
//	}
//	printf("\n");
//	
//	// save to swc and dot files
//	Tree2->writeSwcFile("Tree2.swc"); 
//	Tree2->genGraphvizFile("Tree2.dot");
//	Tree2_old->genGraphvizFile("Tree2_old.dot");
//	
//	
//	// -----------------------------------------------------------------
//	// get the number of nodes in the subtree rooted at each node
//	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
//	// -----------------------------------------------------------------	
//	V3DLONG *subTreeNodeNum1 = 0; // released before the end of the function
//	V3DLONG *subTreeNodeNum2 = 0; // released before the end of the function
//	
//	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
//	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 
//	
//#ifdef DEBUG
//	
//	printf("print subTreeNodeNum1 returned from Tree1->getSubTreeNodeNum(subTreeNodeNum1)\n");	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum1[i]);
//	printf("\n");
//	
//	printf("print subTreeNodeNum2 returned from Tree2->getSubTreeNodeNum(subTreeNodeNum2)\n");	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum2[i]);
//	printf("\n");
//	
//#endif
//	
//	// ------------------------------------------------------------
//	// sort subTreeNodeNum1 and subTreeNodeNum2 from small to large
//	// ------------------------------------------------------------
//	float *sortval1, *sortidx1; // released before the end of the function
//	float *sortval2, *sortidx2; // released before the end of the function
//	
//	Tree1->sortSubTreeNodeNum(subTreeNodeNum1, sortval1, sortidx1);
//	Tree2->sortSubTreeNodeNum(subTreeNodeNum2, sortval2, sortidx2);
//	
//#ifdef DEBUG
//	
//	printf("print sortval1 and sortidx1\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval1[i+1], sortidx1[i+1]);
//	printf("\n");
//	
//	printf("print sortval2 and sortidx2\n");
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval2[i+1], sortidx2[i+1]);
//	printf("\n");
//	
//#endif
//	
//	// ---------------------------------------
//	// initialize parameters
//	// ---------------------------------------
//	
//	V3DLONG nodeNum1, nodeNum2;
//	nodeNum1 = Tree1->treeNodeNum;
//	nodeNum2 = Tree2->treeNodeNum;
//	
//	V3DLONG nodeidx1, nodeidx2;
//	
//	// allocate memory for similarity1d and intialize
//	float *similarity1d = new float [nodeNum1*nodeNum2]; // similarity matrix between pair-wise nodes of Tree1 and Tree2 (after prune)
//	if (!similarity1d)
//	{
//		printf("Fail to allocate memory for similarity1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//		{
//			similarity1d[m*nodeNum2 + n] =  INVALID_VALUE; // leaf nodes will not be matched in DP, thus they will keep this value
//		}
//	
//	
//	// allocate memory for mappingfunc1d and intialize	
//	V3DLONG *mappingfunc1d = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching between pair-wise nodes in Tree1 and Tree2, 
//	// for each pair-wise nodes, matching function is a vector containing nodeNum1 elements, indicating for each node in Tree1
//	// what is its correspondence node in Tree2
//	
//	if (!mappingfunc1d)
//	{
//		printf("Fail to allocate memory for mappingfunc1d. \n");
//		return false;
//	}
//	
//	//	V3DLONG *mappingfunc1d_tmp = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching
//	//	if (!mappingfunc1d_tmp)
//	//	{
//	//		printf("Fail to allocate memory for mappingfunc1d_tmp. \n");
//	//		return false;
//	//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//			for (p=0; p<nodeNum1; p++)
//				mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
//	
//	// initialize matching list, the size is determined by the original Tree1, i.e., Tree1_old
//	V3DLONG len = Tree1_old->treeNodeNum*(MAX_CHILDREN_NUM+1);
//	matchingList = new V3DLONG [len]; //indicating for each node in Tree1, the id# of the matching point in tree2, and the matching branches corresponding to each branch of the node in Tree1
//	
//	for (i=0; i<len; i++)
//		matchingList[i] = INVALID_VALUE; //need to check i
//	
//	// allocate memory and initiate bestBranchMatch, which indicates that for each pair of matching nodes in tree1 and tree2, 
//	// how their branches are matched. This information is used in matching leaf nodes and hierarchical matching
//	V3DLONG *bestBranchMatch = new V3DLONG [nodeNum1*nodeNum2*(MAX_CHILDREN_NUM*2)];
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//			for (p=0; p<MAX_CHILDREN_NUM*2; p++) 
//			{
//				bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2) + n*(MAX_CHILDREN_NUM*2) + p] = INVALID_VALUE;
//			}
//	
//	// compute the maximum length of the trees, for the purpose of supernode generation
//	float Tree1_old_maxlength, Tree1_old_minlength, Tree1_old_totalLength;	
//	V3DLONG Tree1_old_maxLengthNodeid, Tree1_old_minLengthNodeid;		
//	V3DLONG rootid1, rootidx1;
//	
//	Tree1_old->getRootID(rootid1, rootidx1);		
//	Tree1_old->getSubTreeLength(rootid1, Tree1_old_maxlength, Tree1_old_maxLengthNodeid, Tree1_old_minlength, Tree1_old_minLengthNodeid, Tree1_old_totalLength);
//	printf("The maximum length of Tree1_old is %f\n", Tree1_old_maxlength);
//	
//	float Tree2_old_maxlength, Tree2_old_minlength, Tree2_old_totalLength;
//	V3DLONG Tree2_old_maxLengthNodeid, Tree2_old_minLengthNodeid;
//	V3DLONG rootid2, rootidx2;	
//	
//	Tree2_old->getRootID(rootid2, rootidx2);				
//	Tree2_old->getSubTreeLength(rootid1, Tree2_old_maxlength, Tree2_old_maxLengthNodeid, Tree2_old_minlength, Tree2_old_minLengthNodeid, Tree2_old_totalLength);
//	printf("The maximum length of Tree2_old is %f\n", Tree2_old_maxlength);
//	
//	// ---------------------------------------
//	// tree matching using dynamic programming
//	// ---------------------------------------
//	
//	for (i=0; i<nodeNum1; i++) // nodes in Tree1 
//	{
//		
//		V3DLONG *childrenList1_Tree1=0, *childrenNodeIdx1_Tree1=0, childrenNum1_Tree1;
//		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1, childrenNodeIdx1_Tree1, childrenNum1_Tree1); // get direct children for the current node in Tree1
//		
//#ifdef DEBUG
//		
//		printf("print childrenList1_Tree1 and childrenNodeIdx1_Tree1\n");
//		for (int m=0; m<childrenNum1_Tree1; m++)
//			printf("%d ", childrenList1_Tree1[m]);
//		printf("\n");
//		
//		for (int m=0; m<childrenNum1_Tree1; m++)
//			printf("%d ", childrenNodeIdx1_Tree1[m]);
//		printf("\n\n");
//		
//#endif
//		
//		for (j=0; j<nodeNum2; j++) // nodes in Tree2
//		{
//			
//			//			printf("i=%d,j=%d\n", i,j);	
//			//			printf("nodeid1 = %d, nodeid2 = %d, sortidx1 = %f, sortidx2 = %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid, sortidx1[i+1], sortidx2[j+1]);
//			
//			V3DLONG *childrenList2_Tree2=0, *childrenNodeIdx2_Tree2=0, childrenNum2_Tree2;
//			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2, childrenNodeIdx2_Tree2, childrenNum2_Tree2); // get direct children for the current node in Tree2
//			
//			
//#ifdef DEBUG
//			
//			printf("print childrenList2_Tree2 and childrenNodeIdx2_Tree2\n");
//			
//			for (m=0; m<childrenNum2_Tree2; m++)
//				printf("%d ", childrenList2_Tree2[m]);
//			printf("\n");
//			
//			for (m=0; m<childrenNum2_Tree2; m++)
//				printf("%d ", childrenNodeIdx2_Tree2[m]);
//			printf("\n\n");
//#endif			
//			
//			
//			//			//initialize mappingfunc1d between i and j
//			//			for (int p = 0; p<nodeNum1; p++)
//			//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			float bestSimVal_global = INVALID_VALUE; // bestSimVal_global record for each pair-wise nodes sortdix1[i+1] and sortidx2[j+1], the best matching score when considering all possible case of super nodes
//			
//			for (int sni=0; sni<=childrenNum1_Tree1; sni++) // combine non, or each child into a super node 
//			{
//				
//				if (sni==0)
//				{
//					Tree1->copyTree(Tree1_sn);
//					Tree1_old->copyTree(Tree1_sn_old);
//					
//					// compute removeBranchTag1_sn
//					updateRemoveBranchTag(Tree1_sn_old, Tree1_sn, Tree1_old, INVALID_VALUE, INVALID_VALUE, removeBranchTag1, removeBranchTag1_sn);							
//					
//				}
//				else
//				{
//					// delete the childrenList1_Tree1[i] and make the children of childrenList1_Tree1[i] the direct children of sortidx1[i+1].nid
//					// this is done on both old tree and tree after removing insignificant points, the result trees are Tree1_sn, and Tree1_sn_old	
//					// note that each node in Tree1 is a branching node in Tree1_old after calling removeInsignificantNodes(). Thus there is no
//					// need to test if the sni child is a branching node or not (here supernodes are only generated by combining branching nodes)
//					
//					float branchingNodeDis;
//					Tree1_old->computeLength(childrenList1_Tree1[sni-1], Tree1->node[(V3DLONG)sortidx1[i+1]].nid, branchingNodeDis);	
//					
//					if ((float)branchingNodeDis/(float)Tree1_old_maxlength<branchingNodeDisRatioThre) // the distance between the two branching ndoes is short compared to the maximum length of the tree
//					{
//						printf("sni=%d, childrenList1_Tree1 = %d, Tree1->node[(V3DLONG)sortidx1[i+1]].nid = %d\n", sni, childrenList1_Tree1[sni-1], Tree1->node[(V3DLONG)sortidx1[i+1]].nid);
//						Tree1->buildSuperNodeTree(childrenList1_Tree1[sni-1], Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree1_sn);	// Tree1_sn is deleted before the end of for sni loop		
//						Tree1_old->buildSuperNodeTree(childrenList1_Tree1[sni-1], Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree1_sn_old);	// Tree1_sn_old is deleted before the end of for sni loop
//						
//						// compute removeBranchTag1_sn
//						updateRemoveBranchTag(Tree1_sn_old, Tree1_sn, Tree1_old, childrenList1_Tree1[sni-1], Tree2->node[(V3DLONG)sortidx1[i+1]].nid, removeBranchTag1, removeBranchTag1_sn);							
//						
//					}
//				}
//				
//				//save files
//				Tree1_sn->writeSwcFile("Tree1_sn.swc"); 
//				Tree1_sn_old->writeSwcFile("Tree1_sn_old.swc"); 
//				
//				// get direct children of the node in Tree1_sn
//				V3DLONG childrenNum1_Tree1_sn;
//				V3DLONG *childrenList1_Tree1_sn=0;
//				V3DLONG *childrenNodeIdx1_Tree1_sn = 0;
//				
//				Tree1_sn->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1_sn, childrenNodeIdx1_Tree1_sn, childrenNum1_Tree1_sn); // get direct children for the current node in Tree1
//				
//				// compute branch length ratio
//				float *lengthratio1=0; // delete before the end of sni loop
//				unsigned char lengthratioNum1, childrenNum1_Tree1_sn_old;
//				Tree1_sn_old->computeBranchLengthRatio(Tree1_sn->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4);
//				childrenNum1_Tree1_sn_old = lengthratioNum1; // number of children in Tree1_sn_old
//				
//				// compute the angle metrics between branches 
//				V3DLONG branchNum1; //branchNum1 should have the same value as lengthratioNum1
//				float *angles1; //delete before the end of sni loop
//				Tree1_sn_old->computeBranchingAngles(Tree1_sn->node[(V3DLONG)sortidx1[i+1]].nid, branchNum1, angles1);
//				
//				//	#ifdef DEBUG
//				//			printf("branchNum1 = %d, lengthratioNum1 = %d\n", branchNum1, lengthratioNum1);
//				//			for (m=0; m<branchNum1*(branchNum1+1)/2; m++)
//				//				printf("%f ", angles1[m]);
//				//			printf("\n");
//				//	#endif
//				//
//				//
//				// find candidate nodes in Tree2 that can be matched to the current node in tree1
//				
//				
//				for (int snj=0; snj<=childrenNum2_Tree2; snj++) // combine non, or each child into a super node 
//				{
//					if (snj==0)
//					{
//						Tree2->copyTree(Tree2_sn);
//						Tree2_old->copyTree(Tree2_sn_old);
//						
//						// compute removeBranchTag2_sn
//						updateRemoveBranchTag(Tree2_sn_old, Tree2_sn, Tree2_old, INVALID_VALUE, INVALID_VALUE, removeBranchTag2, removeBranchTag2_sn);
//						
//					}
//					else
//					{
//						// delete the childrenList2_Tree2[i] and make the children of childrenList2_Tree2[i] the direct children of sortidx2[j+1].nid
//						// this is done on both old tree and tree after removing insignificant points, the result trees are Tree2_sn, and Tree2_sn_old
//						// note that each node in Tree1 is a branching node in Tree1_old after calling removeInsignificantNodes(). Thus there is no
//						// need to test if the sni child is a branching node or not (here supernodes are only generated by combining branching nodes)
//						float branchingNodeDis;
//						Tree2_old->computeLength(childrenList2_Tree2[snj-1], Tree2->node[(V3DLONG)sortidx2[j+1]].nid, branchingNodeDis);	
//						
//						if ((float)branchingNodeDis/(float)Tree2_old_maxlength<branchingNodeDisRatioThre) // the distance between the two branching ndoes is short compared to the maximum length of the tree
//						{			
//							
//							printf("snj=%d, childrenList2_Tree2 = %d, Tree2->node[(V3DLONG)sortidx2[j+1]].nid = %d\n", snj, childrenList2_Tree2[snj-1], Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//							
//							Tree2->buildSuperNodeTree(childrenList2_Tree2[snj-1], Tree2->node[(V3DLONG)sortidx2[j+1]].nid, Tree2_sn);	// Tree2_sn is deleted before the end of for snj loop		
//							Tree2_old->buildSuperNodeTree(childrenList2_Tree2[snj-1], Tree2->node[(V3DLONG)sortidx2[j+1]].nid, Tree2_sn_old); // Tree2_sn_old is deleted before the end of for snj loop
//							
//							// compute removeBranchTag2_sn
//							updateRemoveBranchTag(Tree2_sn_old, Tree2_sn, Tree2_old, childrenList2_Tree2[snj-1], Tree2->node[(V3DLONG)sortidx2[j+1]].nid, removeBranchTag2, removeBranchTag2_sn);							
//						}
//					}
//					
//					//save files
//					Tree2_sn->writeSwcFile("Tree2_sn.swc"); 
//					Tree2_sn_old->writeSwcFile("Tree2_sn_old.swc"); 
//					
//					// get direct children of the node in Tree1_sn
//					V3DLONG childrenNum2_Tree2_sn;
//					V3DLONG *childrenList2_Tree2_sn=0;  //delete before the end of snj loop
//					V3DLONG *childrenNodeIdx2_Tree2_sn = 0;  //delete before the end of snj loop
//					
//					Tree2_sn->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2_sn, childrenNodeIdx2_Tree2_sn, childrenNum2_Tree2_sn); // get direct children for the current node in Tree1
//					
//					// compute the legnth ratio metrics of each branch based on which similarity is further computed
//					float *lengthratio2=0; //delete before the end of snj loop
//					unsigned char lengthratioNum2, childrenNum2_Tree2_sn_old;
//					Tree2_sn_old->computeBranchLengthRatio(Tree2_sn->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4);
//					childrenNum2_Tree2_sn_old = lengthratioNum2; // number of children in old Tree2_sn
//					
//					float *simVector_lengthRatio = 0;  //delete before the end of snj loop			
//					//			printf("i=%d, j=%d, lengthratioNum1 = %d, lengthratioNum2=%d\n", i,j,lengthratioNum1, lengthratioNum2);
//					simVector_lengthRatio = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
//					
//					printf("simVector_lengthRatio\n");
//					for (m=0; m<lengthratioNum1; m++)
//					{
//						for (n=0; n<lengthratioNum2; n++)
//						{
//							printf("%f ", simVector_lengthRatio[m*lengthratioNum2 + n]);
//						}
//						printf("\n");
//					}
//					
//					// compute the angle metrics between branches based on which similarity is further computed
//					V3DLONG branchNum2; //branchNum2 should have the same value as lengthratioNum2
//					float *angles2;  //delete before the end of snj loop
//					Tree2_sn_old->computeBranchingAngles(Tree2_sn->node[(V3DLONG)sortidx2[j+1]].nid, branchNum2, angles2);
//					
//#ifdef DEBUG
//					printf("branchNum2 = %d, lengthratioNum2 = %d\n", branchNum2, lengthratioNum2);
//					for (m=0; m<branchNum2*(branchNum2+1)/2; m++)
//						printf("angles2 = %f ", angles2[m]);
//					printf("\n");
//#endif
//					
//					float *simVector_angle_parent = 0, *simVector_angle_children = 0;	 //delete before the end of snj loop		
//					//			printf("i=%d, j=%d, branchNum1 = %d, branchNum2=%d\n", i,j,branchNum1, branchNum2);
//					calsim_allbranch_angles(angles1, angles2, branchNum1, branchNum2, simVector_angle_parent, simVector_angle_children);							
//					//			printf("simVector_angle_parent=%d, simVector_angle_children= %d\n", simVector_angle_parent, simVector_angle_children);
//					
//#ifdef DEBUG
//					for (m=0; m<lengthratioNum1; m++)
//					{
//						for (n=0; n<lengthratioNum2; n++)
//							printf("%f ", simVector_angle_parent[m*lengthratioNum2+n]);
//						printf("\n");
//					}
//					printf("\n");
//#endif			
//					
//					//					//initialize mappingfunc1d_tmp between i and j
//					//					for (int p = 0; p<nodeNum1; p++)
//					//						mappingfunc1d_tmp[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//					//					mappingfunc1d_tmp[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//					
//					// find the best matching for subtree by enumeration	
//					
//					// compute all possible combinations of matched branches with respect to the old tree and save it in possibleMatchList
//					
//					V3DLONG *possibleMatchList;  //delete before the end of snj loop
//					V3DLONG num, N_num, R_num;
//					findPossibleMatchList(childrenNum1_Tree1_sn_old, childrenNum2_Tree2_sn_old, 0, possibleMatchList, num, N_num, R_num);
//					
//					//		find the best matching nodes in the subtree rooted at each pair-wise children nodes in Tree1_sn and Tree2_sn,
//					//		not in Tree1_sn_old, Tree2_sn_old
//					
//					V3DLONG *bestSubMatch =0;	 //delete before the end of snj loop	
//					if ((childrenNum1_Tree1_sn!=0)&&(childrenNum2_Tree2_sn!=0)) // if one of the nodes is leaf node, do not compute similarity of subtrees, only compute local similarity  		  
//					{
//						
//						//		initialize bestSubMatch
//						bestSubMatch = new V3DLONG [childrenNum1_Tree1_sn*childrenNum2_Tree2_sn*2]; // the best matching node pairs in the subtree rooted at 
//						//each pair-wise children of the ith and jth nodes in Tree1_sn and Tree2_sn
//						
//						for (m=0; m<childrenNum1_Tree1_sn; m++)
//							for (n=0; n<childrenNum2_Tree2_sn; n++)
//							{
//								bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2] = INVALID_VALUE;
//								bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2+1] = INVALID_VALUE;
//							}
//						
//						// find the best match
//						for (m=0; m<childrenNum1_Tree1_sn; m++)
//						{
//							//get all children rooted at childrenList1_Tree1_sn[m], 	
//							V3DLONG *subTreeNodeIdx1 = 0; //delete before the end of m loop
//							swcTree *subTree1_sn = 0; //delete before the end of m loop
//							//					Tree1_sn->getSubTree(childrenList1_Tree1_sn[m], depthThre, subTree1_sn, subTreeNodeIdx1);
//							Tree1_sn->getSubTree(childrenList1_Tree1_sn[m], subtreeRatioThre, subTree1_sn, subTreeNodeIdx1); // get the nodes in the subtree whose lenghtratio is constrained by subtreeRatioThre
//							
//							for (n=0; n<childrenNum2_Tree2_sn; n++)
//							{
//								// find the best one in the subtree rooted at nodeidx2
//								V3DLONG *subTreeNodeIdx2 = 0; //delete before the end of n loop
//								swcTree *subTree2_sn = 0; //delete before the end of n loop
//								//						Tree2_sn->getSubTree(childrenList2_Tree2_sn[n], depthThre, subTree2_sn, subTreeNodeIdx2);
//								Tree2_sn->getSubTree(childrenList2_Tree2_sn[n], subtreeRatioThre, subTree2_sn, subTreeNodeIdx2);
//								
//								// find the best match with highest similarity in the subtree rooted at childrenList1_Tree1_sn[m] and childreList2_Tree2_sn[n]						
//								float bestval = INVALID_VALUE;
//								V3DLONG p1, q1;
//								
//								// for each pair-wise nodes in the two subtrees
//								for (p=0; p<subTree1_sn->treeNodeNum; p++)
//									for (q=0; q<subTree2_sn->treeNodeNum; q++)
//									{
//										
//										float similarity1d_norm = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//										int cntt=0; //conuter of how many nodes in the subtree of [subTreeNodeIdx1[p] and subTreeNodeIdx2[q] matches, used to normalize similarity score, to exclude the
//										// influence of the subtree size on the similarity score
//										
//										printf("subTreeNodeIdx1[p] = %d, subTreeNodeIdx2[q] = %d\n", Tree1_sn->node[subTreeNodeIdx1[p]].nid, Tree2_sn->node[subTreeNodeIdx2[q]].nid);
//										printf("mappingfunc1d:\n");
//										
//										for (V3DLONG pp = 0; pp<nodeNum1; pp++)
//										{
//											if (mappingfunc1d[subTreeNodeIdx1[p]*nodeNum2*nodeNum1+subTreeNodeIdx2[q]*nodeNum1+pp]!=INVALID_VALUE)
//											{
//												cntt++; 
//												printf("%d %d\n", Tree1_sn->node[pp].nid,Tree2_sn->node[mappingfunc1d[subTreeNodeIdx1[p]*nodeNum2*nodeNum1+subTreeNodeIdx2[q]*nodeNum1+pp]].nid);
//											}
//										}
//										
//										// 20090402 begin
//										if (cntt!=0)
//											similarity1d_norm /= cntt;
//										else
//											similarity1d_norm = INVALID_VALUE;									
//										//20090402 end
//										
//										printf("similarity1d_norm = %f\n", similarity1d_norm);
//										
//										
//										if (bestval<=similarity1d_norm)
//										{
//											bestval = similarity1d_norm;
//											p1 = p; q1 = q;
//										}
//										
//									} // for p,q end
//								
//								
//								//						bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2] = subTreeNodeIdx1[p1];
//								//						bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2+1] = subTreeNodeIdx2[q1];
//								
//								//20090402 begin
//								if (bestval!=INVALID_VALUE)
//								{
//									bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2] = subTreeNodeIdx1[p1];
//									bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2+1] = subTreeNodeIdx2[q1];
//								}
//								//20090402 end
//								
//								if (subTree2_sn) {delete subTree2_sn; subTree2_sn=0;}
//								if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
//								
//							} // for n
//							
//							if (subTree1_sn) {delete subTree1_sn; subTree1_sn=0;}
//							if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}
//							
//						} // for m
//						
//						//				//print bestSbuMatch
//						//				printf("bestSubMatch:\n");
//						//				for (m=0; m<childrenNum1_Tree1_sn; m++)
//						//				for (n=0; n<childrenNum2_Tree2_sn; n++)
//						//					printf("m = %d, n = %d, bestSubMatch1 = %d, bestSubMatch2 = %d, nodeid1 = %d, nodeid2 = %d\n", m,n, bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2],  bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2+1],
//						//					      Tree1_sn->node[bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2]].nid, Tree2_sn->node[bestSubMatch[m*childrenNum2_Tree2_sn*2+n*2+1]].nid);
//						
//						
//					}// if ((childrenNum1_Tree1_sn!=0)&&(childrenNum2_Tree2_sn!=0))
//					
//					
//					// compute for each match the best similarity score and corresponding match
//					
//					float bestSimVal = INVALID_VALUE;
//					
//					V3DLONG best_m = 0;
//					
//					for (m=0; m<num; m++) // for all possible combinations in old trees
//					{
//						float tmpSimVal=0;
//						
//						unsigned char matchedNodeNum = 0; // for normalizing similarity purpose
//						
//						if ((childrenNum1_Tree1_sn!=0)&&(childrenNum2_Tree2_sn!=0)) // only compute subtree similarity for non-leaf nodes
//						{
//							// subtree similarity
//							
//							for (n=0; n<R_num; n++) // mappings in old trees
//							{
//								V3DLONG tmpidx =m*(R_num*2)+2*n; 
//								
//								V3DLONG idx1 = possibleMatchList[tmpidx];
//								V3DLONG idx2 = possibleMatchList[tmpidx+1];
//								
//								
//								// test if idx1, idx2 which are in Tree1_sn_old and Tree2_sn_old are still in Tree1_sn and Tree2_sn								
//								
//								if ((removeBranchTag1_sn[(V3DLONG)sortidx1[i+1]][idx1]==-1)||(removeBranchTag2_sn[(V3DLONG)sortidx2[j+1]][idx2]==-1)) // at least one of the mapped branches is not in Tree1_sn or Tree2_sn
//									continue; // do not count that match
//								else
//								{
//									// compute the new index of branches in Tree1 and Tree2_sn, so that bestSubMatch can use the correct subscripts
//									
//									V3DLONG idx1_new = removeBranchTag1_sn[(V3DLONG)sortidx1[i+1]][idx1];
//									V3DLONG idx2_new = removeBranchTag2_sn[(V3DLONG)sortidx2[j+1]][idx2];
//									
//									V3DLONG ind1 = bestSubMatch[idx1_new*childrenNum2_Tree2_sn*2+idx2_new*2];
//									V3DLONG ind2 = bestSubMatch[idx1_new*childrenNum2_Tree2_sn*2+idx2_new*2+1];		
//									
//									//							tmpSimVal += similarity1d[ind1*nodeNum2+ind2];
//									
//									//20090402 begin
//									if ((ind1!=INVALID_VALUE)&&(ind2!=INVALID_VALUE)) // otherwise, no subtree nodes in that branch pair can be matched, ignore that branch pair
//										tmpSimVal += similarity1d[ind1*nodeNum2+ind2];
//									//20090402 end
//									
//								}
//								
//							}
//							
//							//					printf("subtree similarity1d = %f, ", tmpSimVal);
//						} // if ((childrenNum1_Tree1_sn!=0)&&(childrenNum2_Tree2_sn!=0)) 
//						
//						//				printf("matchedNodeNum = %d\n", matchedNodeNum);						
//						
//						// plus length-ratio similarity, computed based on old trees
//						float tmpSimLengthRatio=0;
//						printf("m=%d, ", m);
//						
//						for (n=0; n<R_num; n++)
//						{
//							V3DLONG tmpidx =m*(R_num*2)+2*n; 
//							V3DLONG idx1 = possibleMatchList[tmpidx];
//							V3DLONG idx2 = possibleMatchList[tmpidx+1];
//							
//							//20090402 begin
//							if (simVector_lengthRatio[idx1*childrenNum2_Tree2_sn_old+idx2]==-1) // if one pair of branch is far too different, then that branching pair should not be considered
//							{
//								tmpSimLengthRatio = -R_num;
//								break;
//							}
//							else
//								tmpSimLengthRatio += simVector_lengthRatio[idx1*childrenNum2_Tree2_sn_old+idx2];
//							printf("%f, ", simVector_lengthRatio[idx1*childrenNum2_Tree2_sn_old+idx2]); 
//							//20090402 end
//							
//						}
//						
//						printf("\n");
//						
//						tmpSimLengthRatio /= R_num; // normalize to balance length-ratio and angle
//						//					if (weight_lengthRatio!=0)
//						//						printf("local length ratio similarity1d = %f\n ", tmpSimLengthRatio);
//						
//						tmpSimVal += (weight_lengthRatio*tmpSimLengthRatio);
//						
//						printf("tmpSimVal=%f\n", tmpSimVal);
//						
//						// plus angle (parent-path) similarity, computed based on old trees
//						float tmpSimAngle = 0;
//						for (n=0; n<R_num; n++)
//						{
//							V3DLONG tmpidx =m*(R_num*2)+2*n; 
//							V3DLONG idx1 = possibleMatchList[tmpidx];
//							V3DLONG idx2 = possibleMatchList[tmpidx+1];
//							
//							tmpSimAngle += simVector_angle_parent[idx1*childrenNum2_Tree2_sn_old + idx2];
//						}
//						
//						// plus angle (children-path) similarity	
//						
//						V3DLONG len = childrenNum2_Tree2_sn_old*(childrenNum2_Tree2_sn_old-1)/2;
//						V3DLONG ttmp;
//						
//						for (p=0; p<R_num-1; p++)
//							for (q=p+1; q<R_num; q++)
//							{
//								V3DLONG tmpidxp =m*(R_num*2)+2*p; 
//								V3DLONG tmpidxq =m*(R_num*2)+2*q; 
//								
//								// compute idx1 and idx2
//								V3DLONG idx1 = 0, idx2 = 0;
//								for (V3DLONG s=0; s< possibleMatchList[tmpidxp]; s++)
//									idx1 += (childrenNum1_Tree1_sn_old-(s+1));
//								
//								//						idx1 +=  (possibleMatchList[tmpidxq] - possibleMatchList[tmpidxp]-1);
//								
//								ttmp = possibleMatchList[tmpidxq] - possibleMatchList[tmpidxp];
//								if (ttmp<0)
//									ttmp = -ttmp;							
//								idx1 +=  (ttmp-1);
//								
//								
//								for (V3DLONG s=0; s< possibleMatchList[tmpidxp+1]; s++)
//									idx2 += (childrenNum2_Tree2_sn_old-(s+1));
//								
//								ttmp = possibleMatchList[tmpidxq+1] - possibleMatchList[tmpidxp+1];
//								if (ttmp<0)
//									ttmp = -ttmp;							
//								idx2 +=  (ttmp-1);
//								
//								tmpSimAngle += simVector_angle_children[idx1*len + idx2];
//							}
//						
//						tmpSimAngle /= (R_num*(R_num+1)/2); // normalize to balance length-ratio and angle
//						//					printf("tmpSimLengthRatio = %f, tmpSimAngle = %f, R_num= %d\n", tmpSimLengthRatio, tmpSimAngle, R_num);
//						
//						//					if (weight_angle!=0)
//						//						printf("local angle similarity1d = %f\n ", tmpSimAngle);
//						
//						tmpSimVal += (weight_angle*tmpSimAngle);
//						
//						if (tmpSimVal>bestSimVal)
//						{
//							bestSimVal = tmpSimVal;
//							best_m = m;
//							printf("best_m=%d\n", best_m);
//						}
//					} //for (m=0; m<num; m++)
//					
//					if (bestSimVal>bestSimVal_global) // bestSimVal_global record for each pair-wise nodes sortdix1[i+1] and sortidx2[j+1], the best matching score when considering all possible case of super nodes
//					{
//						//update the best similarity and bestBranchMatch			
//						similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]= bestSimVal;
//						printf("bestSimVal = %f\n", bestSimVal);
//						
//						// 20090402 begin
//						//						if (bestSimVal!=-1) // otherwise, do not set bestBranchMatch since the pair of nodes should not be matched
//						if (bestSimVal>-1) // otherwise, do not set bestBranchMatch since the pair of nodes should not be matched, 20090918
//						{
//							for (p=0; p<R_num; p++)
//							{
//								bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2] = possibleMatchList[best_m*R_num*2+p*2];
//								bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2+1] = possibleMatchList[best_m*R_num*2+p*2+1];					
//							}
//						}
//						else
//						{
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = INVALID_VALUE; // make the pair of nodes unmatched
//							continue; // do not do the following either
//						}
//						// 20090402 end
//						
//						//assign the best mapping list if both nodes are non-leaf nodes
//						if ((childrenNum1_Tree1_sn!=0)&&(childrenNum2_Tree2_sn!=0))
//						{
//							
//							//reset mappingfunc1d_tmp between i and j
//							for (int p = 0; p<nodeNum1; p++)
//								mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//							
//							for (m=0; m<R_num; m++)
//							{
//								
//								p = possibleMatchList[best_m*(R_num*2)+2*m];
//								q = possibleMatchList[best_m*(R_num*2)+2*m+1];
//								
//								// not all R_num have matching, need to test
//								//					if ((removeBranchTag1_sn[(V3DLONG)sortidx1[i+1]][p]==0) && (removeBranchTag2_sn[(V3DLONG)sortidx2[j+1]][q]==0)) // the branches in each tree should exist
//								if ((removeBranchTag1_sn[(V3DLONG)sortidx1[i+1]][p]>=0) && (removeBranchTag2_sn[(V3DLONG)sortidx2[j+1]][q]>=0)) // the branches in each tree should exist
//									
//								{
//									
//									V3DLONG p_new = removeBranchTag1_sn[(V3DLONG)sortidx1[i+1]][p];
//									V3DLONG q_new = removeBranchTag2_sn[(V3DLONG)sortidx2[j+1]][q];
//									
//									// add the best matched sub-branch
//									V3DLONG ind1 = bestSubMatch[p_new*childrenNum2_Tree2_sn*2+q_new*2];
//									V3DLONG ind2 = bestSubMatch[p_new*childrenNum2_Tree2_sn*2+q_new*2+1];
//									
//									//20090402 begin
//									if ((ind1!=INVALID_VALUE)&&(ind2!=INVALID_VALUE))
//										mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;
//									else
//										continue;
//									//20090402 end
//									
//									//expand matching list of the children
//									for (p = 0; p<nodeNum1; p++)
//									{	
//										
//										if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
//											(mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0)) // the condition here avoid override of values already set
//										{
//											//									printf("%d ", mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]); //20090402
//											
//											mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//											mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
//										}
//										
//									} //for p
//									
//									printf("\n");
//									
//								}  // if removeBranchTag1_sn ...
//							} // for m 
//							
//							//							// set values for mappingfunc1d
//							//							for (p = 0; p<nodeNum1; p++)
//							//							{	
//							//								
//							//								mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//							//								mappingfunc1d_tmp[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p];
//							//							} //for p
//							
//						} //if ((childrenNum1_Tree1_sn!=0)&&(childrenNum2_Tree2_sn!=0))
//						
//						
//					} //if (bestSimVal>bestSimVal_global)
//					
//					// delete pointers generated within for snj loop
//					
//					if (lengthratio2) {delete lengthratio2; lengthratio2=0;}
//					if (simVector_lengthRatio) {delete []simVector_lengthRatio; simVector_lengthRatio=0;}
//					if (angles2) {delete []angles2; angles2=0;}
//					if (simVector_angle_parent) {delete []simVector_angle_parent; simVector_angle_parent=0;}
//					if (simVector_angle_children) {delete []simVector_angle_children; simVector_angle_children=0;}
//					
//					if (Tree2_sn) {delete []Tree2_sn; Tree2_sn=0;}
//					if (Tree2_sn_old) {delete []Tree2_sn_old; Tree2_sn_old=0;}
//					
//					if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
//					if (possibleMatchList) {delete []possibleMatchList; possibleMatchList=0;}
//					
//					V3DLONG *removeBranchTag2_sn_1d = removeBranchTag2_sn[0];
//					if (removeBranchTag2_sn_1d) {delete []removeBranchTag2_sn; removeBranchTag2_sn=0;}
//					delete []removeBranchTag2_sn_1d; removeBranchTag2_sn_1d=0;
//					
//					if (childrenList2_Tree2_sn) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
//					if (childrenNodeIdx2_Tree2_sn) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
//					
//				} //for snj end
//				
//				// delete pointers generated within for sni loop
//				
//				if (lengthratio1) {delete lengthratio1; lengthratio1=0;}
//				if (Tree1_sn) {delete []Tree1_sn; Tree1_sn=0;}
//				if (Tree1_sn_old) {delete []Tree1_sn_old; Tree1_sn_old=0;}
//				
//				V3DLONG *removeBranchTag1_sn_1d = removeBranchTag1_sn[0];
//				if (removeBranchTag1_sn_1d) {delete []removeBranchTag1_sn; removeBranchTag1_sn=0;}
//				delete []removeBranchTag1_sn_1d; removeBranchTag1_sn_1d=0;
//				
//				if (childrenList1_Tree1_sn) {delete childrenList1_Tree1_sn; childrenList1_Tree1_sn=0;}
//				if (childrenNodeIdx1_Tree1_sn) {delete childrenNodeIdx1_Tree1_sn; childrenNodeIdx1_Tree1_sn=0;}
//				
//			} // for sni end
//			
//			
//			
//			//print temporary result for debug purpose
//			
//			float bestsim = INVALID_VALUE;
//			V3DLONG best_j=0;
//			
//			for (j=0; j<nodeNum2; j++)
//				if (similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+j]>bestsim)
//				{
//					bestsim = similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+j];
//					best_j = j;
//				}
//			
//			printf("nodeid1 = %d, bestmatched nodeid2 = %d, best similarity = %f \n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[best_j].nid, bestsim);
//			
//			for (p=0; p<nodeNum1; p++)
//			{
//				V3DLONG q = mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+best_j*nodeNum1+p];
//				if (q!=INVALID_VALUE)
//					//					printf("       nodeid1 = %d is matched to nodeid2 = %d\n", Tree1_sn->node[p].nid, Tree2->node[q].nid);
//					printf("%d %d\n", Tree1->node[p].nid, Tree2->node[q].nid);
//				
//			}
//			
//#ifdef DEBUG	
//			
//			//print similarity matrix
//			printf("Similarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", similarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
//			
//			printf("\n");
//			
//			
//#endif	
//			if (childrenList2_Tree2) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
//			if (childrenNodeIdx2_Tree2) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
//			
//		}//for (j=0; j<nodeNum2; j++)
//		
//		if (childrenList1_Tree1) {delete childrenList1_Tree1; childrenList1_Tree1=0;}
//		if (childrenNodeIdx1_Tree1) {delete childrenNodeIdx1_Tree1; childrenNodeIdx1_Tree1=0;}
//		
//	}// for i
//	
//	
//	// reconstruct the optimal match from stored best matches
//	
//	// method 2: reconstruct from the root
//	//	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
//	//	Tree1->getRootID(rootid1, rootidx1);
//	//	Tree2->getRootID(rootid2, rootidx2);
//	V3DLONG matchedNodeNum = 0;
//	
//	//	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//	//	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//	
//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//		
//		if (m==rootidx1) //add root, require root must be matched to root
//			idx = rootidx2;
//		else
//		{
//			idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
//		}
//		
//		V3DLONG tmpidx1;
//		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
//		V3DLONG offset = tmpidx1*(MAX_CHILDREN_NUM+1);
//		
//		if (idx>=0)
//		{
//			printf("idx=%d, offset=%d, nodeid=%d\n", idx, offset, Tree2->node[idx].nid);
//			
//			matchedNodeNum ++;					
//			matchingList[offset] = Tree2->node[idx].nid;		
//		}	
//		//		else
//		//		{
//		//			for (n=0; n<MAX_CHILDREN+1; n++)
//		//			{
//		//				matchingList[offset+n] = INVALID_VALUE;
//		//			
//		//			}
//		//		}
//	}
//	
//	simMeasure = similarity1d[rootidx1*nodeNum2 + rootidx2];
//	printf("simMeasure = %f\n", simMeasure);
//	
//	
//	
//	//print similarity matrix
//	printf("Similarity matrix\n");
//	
//	for (i=0; i<nodeNum1; i++)
//	{
//		for (j=0; j<nodeNum2; j++)
//			printf("%f ", similarity1d[i*nodeNum2+j]);
//		printf("\n");
//	}
//	
//	printf("\n best branch matching: \n");
//	
//	// add best matching branches to matchingList	
//	V3DLONG cnt = -1;
//	
//	for (m=0; m<nodeNum1; m++)
//	{
//		
//		
//		V3DLONG idx;
//		
//		if (m==rootidx1) //add root, require root must be matched to root
//			idx = rootidx2;
//		else
//		{
//			idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
//		}
//		
//		if (idx!=INVALID_VALUE)
//		{
//			cnt++;
//			
//			V3DLONG nodeidx1;
//			
//			Tree1_old->getIndex(Tree1->node[m].nid, nodeidx1);
//			
//			printf("nodeid1 = %d, nodeid2 = %d,           ", Tree1->node[m].nid, Tree2->node[idx].nid);
//			
//			V3DLONG *childrenList1=0, *childrenNodeIdx1=0, childrenNum1;			
//			V3DLONG *childrenList2=0, *childrenNodeIdx2=0, childrenNum2;
//			
//			Tree1_old->getDirectChildren(Tree1->node[m].nid, childrenList1, childrenNodeIdx1, childrenNum1); 
//			Tree2_old->getDirectChildren(Tree2->node[idx].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for the current node in Tree1
//			
//			if ((childrenNum1>0) && (childrenNum2>0))
//			{
//				//				branchMatchingList[cnt*len1] = Tree1->node[m].nid;						
//				
//				for (p = 0; p<MAX_CHILDREN_NUM; p++)
//				{
//					V3DLONG tmpidx1 = bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2)+idx*(MAX_CHILDREN_NUM*2)+p*2];
//					V3DLONG tmpidx2 = bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2)+idx*(MAX_CHILDREN_NUM*2)+p*2+1];
//					
//					if ((tmpidx1!=INVALID_VALUE)&&(tmpidx2!=INVALID_VALUE))
//					{
//						matchingList[nodeidx1*(MAX_CHILDREN_NUM+1)+tmpidx1+1] = tmpidx2;
//						
//						printf("tmpidx1 = %d, tmpidx2 = %d; ", tmpidx1, tmpidx2);
//						//							printf("%d, %d; ", Tree1_old->node[childrenNodeIdx1[tmpidx1]].nid, Tree2_old->node[childrenNodeIdx2[tmpidx2]].nid);
//					}	
//				}
//			}
//			printf("\n");
//			
//			if (childrenList1) {delete []childrenList1; childrenList1 = 0;}			
//			if (childrenList2) {delete []childrenList2; childrenList2 = 0;}
//			if (childrenNodeIdx1) {delete []childrenNodeIdx1; childrenNodeIdx1 = 0;}			
//			if (childrenNodeIdx2) {delete []childrenNodeIdx2; childrenNodeIdx2 = 0;}
//			
//		} //if (idx!=INVALID_VALUE)
//		printf("\n");
//		
//	} // for m
//	
//	
//	// ------------------------------------------------
//	// generate matching result for Graphviz
//	// ------------------------------------------------
//	
//	//	// generate matching result file for Graphviz
//	
//	V3DLONG *matchingList_new =new V3DLONG [Tree1->treeNodeNum];
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//	{
//		V3DLONG idx;
//		Tree1_old->getIndex(Tree1->node[i].nid, idx);
//		//		printf("%d\n", idx);
//		
//		if (matchingList[idx*(MAX_CHILDREN_NUM+1)]>0)
//			matchingList_new[i] = matchingList[idx*(MAX_CHILDREN_NUM+1)];
//		else
//			matchingList_new[i] = INVALID_VALUE;
//	};
//	
//	genMatchingGraphvizFile(Tree1, Tree2, matchingList_new, "matching.dot");
//	
//	if (matchingList_new) {delete []matchingList_new; matchingList_new=0;}	
//	
//	// ---------------------------------------
//	// delete pointers
//	// ---------------------------------------
//	
//	if (subTreeNodeNum1) {delete []subTreeNodeNum1; subTreeNodeNum1=0;}
//	if (subTreeNodeNum2) {delete []subTreeNodeNum2; subTreeNodeNum2=0;}
//	
//	if (sortval1) {delete []sortval1; sortval1=0;}
//    if (sortval2) {delete []sortval2; sortval2=0;}
//	if (sortidx1) {delete []sortidx1; sortidx1=0;}
//	if (sortidx2) {delete []sortidx2; sortidx2=0;}
//	
//	if (similarity1d) {delete []similarity1d; similarity1d=0;}
//	
//	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
//	//	if (mappingfunc1d_tmp) {delete []mappingfunc1d_tmp; mappingfunc1d_tmp=0;}
//	
//	//recover tree pointer
//	if (Tree1) {delete Tree1; Tree1=Tree1_old; Tree1_old=0;} 	
//	if (Tree2) {delete Tree2; Tree2=Tree2_old; Tree2_old=0;} 
//	
//	if (removeNodeTag1) {delete removeNodeTag1; removeNodeTag1=0;}
//	if (removeNodeTag2) {delete removeNodeTag2; removeNodeTag2=0;}
//	
//	V3DLONG *removeBranchTag1_1d = removeBranchTag1[0];
//	if (removeBranchTag1_1d) {delete []removeBranchTag1; removeBranchTag1=0;}
//	delete []removeBranchTag1_1d; removeBranchTag1_1d=0;
//	
//	V3DLONG *removeBranchTag2_1d = removeBranchTag2[0];
//	if (removeBranchTag2_1d) {delete []removeBranchTag2; removeBranchTag2=0;}
//	delete []removeBranchTag2_1d; removeBranchTag2_1d=0;
//	
//	if (bestBranchMatch) {delete []bestBranchMatch; bestBranchMatch=0;}
//}
//


////************************************************************************************
//// tree matching using dynamic programming
//// the very early version that works, nodeNumThre=0 for the same neurons, nodeNumThre=2 for different neurons
////************************************************************************************

//bool treeMatching(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
//{
//	V3DLONG i, j, m, n, p,q;
//	V3DLONG nodeNum1, nodeNum2;
//	nodeNum1 = Tree1->treeNodeNum;
//	nodeNum2 = Tree2->treeNodeNum;
//	float nodeNumThre = 2; // nodes with subtree node number no less than this value will be considered in match, 2 for matching neurons by structures, 0 for matching neurons by geometrical features
//	//	float nodeNumThre = 0; // nodes with subtree node number no less than this value will be considered in match, 2 for matching neurons by structures, 0 for matching neurons by geometrical features
//	
//	// -----------------------------------------------------------------
//	// get the number of nodes in the subtree rooted at each node
//	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
//	// -----------------------------------------------------------------	
//	V3DLONG *subTreeNodeNum1 = 0;
//	V3DLONG *subTreeNodeNum2 = 0;
//	
//	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
//	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 
//	
//#ifdef DEBUG
//	
//	printf("print subTreeNodeNum1 returned from Tree1->getSubTreeNodeNum(subTreeNodeNum1)\n");	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum1[i]);
//	printf("\n");
//	
//	printf("print subTreeNodeNum2 returned from Tree2->getSubTreeNodeNum(subTreeNodeNum2)\n");	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum2[i]);
//	printf("\n");
//	
//#endif
//	
//	// ------------------------------------------------------------
//	// sort subTreeNodeNum1 and subTreeNodeNum2 from small to large
//	// ------------------------------------------------------------
//	float *sortval1, *sortidx1;
//	float *sortval2, *sortidx2;
//	
//	Tree1->sortSubTreeNodeNum(subTreeNodeNum1, sortval1, sortidx1);
//	Tree2->sortSubTreeNodeNum(subTreeNodeNum2, sortval2, sortidx2);
//	
//#ifdef DEBUG
//	
//	printf("print sortval1 and sortidx1\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval1[i+1], sortidx1[i+1]);
//	printf("\n");
//	
//	printf("print sortval2 and sortidx2\n");
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval2[i+1], sortidx2[i+1]);
//	printf("\n");
//	
//#endif
//	
//	
//	// ---------------------------------------
//	// tree matching using dynamic programming
//	// ---------------------------------------
//	
//	V3DLONG *childrenList1=0, *childrenNodeIdx1=0;
//	V3DLONG *childrenList2 =0, *childrenNodeIdx2 = 0;	
//	V3DLONG childrenNum1, childrenNum2;
//	V3DLONG nodeidx1, nodeidx2;
//	V3DLONG depthThre = 4;
//	
//	// allocate memory for similarity1d and intialize
//	float *similarity1d = new float [nodeNum1*nodeNum2]; // optimal similarity
//	if (!similarity1d)
//	{
//		printf("Fail to allocate memory for similarity1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//		{
//			////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
//			similarity1d[m*nodeNum2 + n] =  -1;
//			//		similarity1d[m*nodeNum2 + n] =  0;
//		}
//	
//	
//	// allocate memory for mappingfunc1d and intialize	
//	V3DLONG *mappingfunc1d = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching
//	if (!mappingfunc1d)
//	{
//		printf("Fail to allocate memory for mappingfunc1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//			for (p=0; p<nodeNum1; p++)
//				mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
//	
//	// initialize matching list
//	matchingList = new V3DLONG [nodeNum1]; //indicating for each node in tree1, the id# of the matching point in tree2
//	
//	for (i=0; i<nodeNum1; i++)
//		matchingList[i] = -1; //need to check i
//	
//	// start matching
//	for (i=0; i<nodeNum1; i++)
//	{
//		
//		
//		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1, childrenNodeIdx1, childrenNum1); // get direct children for the current node in Tree1
//		
//		if (childrenNum1 < nodeNumThre)
//		{
//			for (m=0; m<nodeNum2; m++)
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[m+1]] = 1;		
//			continue;
//		}
//		
//		//		// construct super node set
//		//		V3DLONG *superNodeIdx1 = new V3DLONG [choldrenNum1];
//		//		superNodeIdx1[0] = sortidx1[i+1]];
//		//		for (int tt = 0; tt< childrenNum1; tt++)
//		//			superNodeIdx1[tt+1] = childrenNodeIdx1[tt];
//		//		
//		
//#ifdef DEBUG
//		
//		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenList1[m]);
//		printf("\n");
//		
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenNodeIdx1[m]);
//		printf("\n\n");
//		
//#endif
//		// find candidate nodes in Tree2 that can be matched to the current node in tree1
//		
//		for (j=0; j<nodeNum2; j++) 
//		{
//			
//			
//			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for a node in Tree2
//			//			Tree2->getAllChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); //get all children for the current node in Tree2
//			
//			if (childrenNum2 < nodeNumThre)
//			{
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 1;
//				continue;
//			}
//			//			// construct super node set
//			//			V3DLONG *superNodeIdx2 = new V3DLONG [choldrenNum2];
//			//			superNodeIdx2[0] = sortidx2[i+1]];
//			//			for (int tt = 0; tt< childrenNum2; tt++)
//			//				superNodeIdx2[tt+1] = childrenNodeIdx2[tt];
//			
//#ifdef DEBUG
//			
//			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenList2[m]);
//			printf("\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenNodeIdx2[m]);
//			printf("\n\n");
//#endif			
//			
//			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
//			if (nodeNumThre==0) // match neurons with the same underlying strucutre
//			{
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//				//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			}
//			
//			//			printf("%d, %d, %d, %d\n", i,j, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			
//			if (nodeNumThre==2) // match neurons with different underlying strucutre
//			{
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//				//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//				
//			}
//			//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			//			// compute the dissimilarity between the two nodes, without considering their subtree similarity yet
//			//			dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 
//			//				caldist(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			
//			for (int p = 0; p<nodeNum1; p++)
//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			
//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			//#ifdef DEBUG	
//			//			printf("The initial similarity:\n");
//			//			printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1dtmp);
//			//
//			//#endif			
//			
//			// find the best matching for subtree using bipartite matching			
//			if ((childrenNum1!=0)&&(childrenNum2!=0)) // if one of the current nodes in tree1 and tree2 are leaf nodes, do not do the following
//			{
//				
//				Matrix<double> bpMatrix(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix, need to free?
//				
//				for (m=0; m<childrenNum1+childrenNum2; m++)
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						//						bpMatrix(m,n) = 9999;
//						bpMatrix(m,n) = -INVALID_VALUE;
//				
//				// build the matrix
//				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1*childrenNum2*2];
//				
//				for (m=0; m<childrenNum1; m++)
//					for (n=0; n<childrenNum2; n++)
//					{
//						bestSubMatch[m*childrenNum2*2+n*2] = INVALID_VALUE;
//						bestSubMatch[m*childrenNum2*2+n*2+1] = INVALID_VALUE;
//					}
//				
//				for (m=0; m<childrenNum1; m++)
//				{
//					Tree1->getIndex(childrenList1[m], nodeidx1);
//					
//					//get all children rooted at childrenList1[m], 
//					
//					V3DLONG *subTreeNodeIdx1 = 0;
//					swcTree *subTree1 = 0; 
//					//					Tree1->getSubTree(childrenList1[m], subTree1, subTreeNodeIdx1); // include root of the subtree, i.e., nodeid
//					Tree1->getSubTree(childrenList1[m], depthThre, subTree1, subTreeNodeIdx1);
//					
//#ifdef DEBUG
//					printf("subTreeNodeIdx1: ");
//					
//					for (p=0; p<subTree1->treeNodeNum; p++)
//						printf("%d, ", subTreeNodeIdx1[p]);
//					printf("\n");
//#endif					
//					for (n=0; n<childrenNum2; n++)
//					{
//						Tree2->getIndex(childrenList2[n], nodeidx2);
//						
//					    // find the best one in the subtree rooted at nodeidx2
//						V3DLONG *subTreeNodeIdx2 = 0;
//						swcTree *subTree2 = 0; 
//						//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
//						Tree2->getSubTree(childrenList2[n], depthThre, subTree2, subTreeNodeIdx2);
//#ifdef DEBUG
//						printf("subTreeNodeIdx2: ");
//						
//						for (p=0; p<subTree2->treeNodeNum; p++)
//							printf("%d, ", subTreeNodeIdx2[p]);
//						printf("\n");
//#endif						
//						// find the best match with highest similarity in the subtree rooted at childrenList1[m] and childreList2[m]
//						
//						float bestval = INVALID_VALUE;
//						for (p=0; p<subTree1->treeNodeNum; p++)
//							for (q=0; q<subTree2->treeNodeNum; q++)
//								if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//								{
//									bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//									bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
//									bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
//									
//								}
//						//						bpMatrix(m,n) = 9999 - bestval; // hungarian method use distance 
//						bpMatrix(m,n) = -bestval; // hungarian method use distance 
//						
//						//						// find the best match with lowest dissimilarity in the subtree rooted at childrenList1[m] and childreList2[m]
//						//	
//						//						float bestval = -INVALID_VALUE;
//						//						
//						//						for (p=0; p<subTree1->treeNodeNum; p++)
//						//						for (q=0; q<subTree2->treeNodeNum; q++)
//						//							if (bestval>dissimilarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//						//							{
//						//								bestval = dissimilarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//						//								bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
//						//								bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
//						//								
//						//							}
//						//								
//						//						bpMatrix(m,n) = bestval; // hungarian method use distance 
//						
//						if (subTree2) {delete subTree2; subTree2=0;}
//						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
//						
//					} // for n
//					
//					if (subTree1) {delete subTree1; subTree1=0;}
//					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}
//					
//				} // for m
//				
//				// Hungarian algorithm for bipartite matching, find the minimum cost
//				Matrix<double> bpMatrixOut(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix
//				
//				for (m=0; m<childrenNum1+childrenNum2; m++)
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						bpMatrixOut(m,n) = bpMatrix(m,n);
//				
//				Munkres branchMatch;
//				branchMatch.solve(bpMatrixOut); 
//				
//				//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				//				{
//				//					for (n=0; n<childrenNum1+childrenNum2; n++)
//				//						printf("%f ", bpMatrixOut(m,n));
//				//					printf("\n");
//				//			    }
//				//				printf("\n");
//				
//				
//				// get temporary simiarlity1d, and matchingfunc1d
//				
//				for (m=0; m<childrenNum1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
//				{
//					Tree1->getIndex(childrenList1[m], nodeidx1);
//					
//					for (n=0; n<childrenNum2; n++) 
//					{
//						//					printf("%f\n ", bpMatrix(m,n));
//						//						printf("%f\n ", bpMatrixOut(m,n));
//						
//						Tree2->getIndex(childrenList2[n], nodeidx2);
//						
//						if (bpMatrixOut(m,n)==0) // find a match
//						{
//							// update similarity1d and mappingfunc1d						
//							// //							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += similarity1d[nodeidx1*nodeNum2+nodeidx2];
//							//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += (9999-bpMatrix(m,n));
//							
//							//							printf("%f ", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//							
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
//							
//							//							// update dissimilarity1d and mappingfunc1d						
//							//							dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += bpMatrix(m,n);
//							
//							V3DLONG ind1 = bestSubMatch[m*childrenNum2*2+n*2];
//							V3DLONG ind2 = bestSubMatch[m*childrenNum2*2+n*2+1];
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
//							
//							//expand matching list of the children
//							for (p = 0; p<nodeNum1; p++)
//							{	
//								
//								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
//								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
//									mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//									mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
//								
//							} //for p
//						} //if (bpMatrixOut(m,n)==0)
//					} // for n
//					//					printf("\n");
//				} // for m	
//				
//				//				// normalize similarity1d, make it irrelavant to the number of matching
//				//				V3DLONG matchcnt = 0;
//				//				for (p = 0; p<nodeNum1; p++)
//				//					if (mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]!=INVALID_VALUE)
//				//						matchcnt++;
//				//						
//				//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] /= matchcnt;	
//				
//				
//				//				if (bestnodeid) {delete []bestnodeid; bestnodeid = 0;}
//				
//				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
//				
//#ifdef DEBUG				
//				printf("The final similarity involving the similarities of the children:\n"); 
//				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//#endif			
//			} //if ((childrenNum1!=0)&&(childrenNum2!=0))
//			
//			
//#ifdef DEBUG	
//			
//			//print similarity matrix
//			printf("Similarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", similarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
//			
//			//			//print dissimilarity matrix
//			//			printf("Dissimilarity matrix\n");
//			//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//			printf("current dissimilarity %f = \n", dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			//			
//			//			for (m=0; m<nodeNum1; m++)
//			//			{
//			//				for (n=0; n<nodeNum2; n++)
//			//					printf("%f ", dissimilarity1d[m*nodeNum2+n]);
//			//				printf("\n");
//			//			}
//			
//#endif	
//			
//			
//			if (childrenList2) {delete childrenList2; childrenList2=0;}
//			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
//		} //for (j=0; j<nodeNum2; j++)
//		
//		
//#ifdef DEBUG	
//		
//		//print similarity matrix
//		printf("Similarity matrix\n");
//		printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//		printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//		printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//		
//		for (m=0; m<nodeNum1; m++)
//		{
//			for (n=0; n<nodeNum2; n++)
//				printf("%f ", similarity1d[m*nodeNum2+n]);
//			printf("\n");
//		}
//		
//		printf("\n");
//		
//		
//		//			//print dissimilarity matrix
//		//			printf("dissimilarity matrix\n");
//		//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//		//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//		//			printf("current dissimilarity %f = \n", dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//		//			
//		//			for (m=0; m<nodeNum1; m++)
//		//			{
//		//				for (n=0; n<nodeNum2; n++)
//		//					printf("%f ", dissimilarity1d[m*nodeNum2+n]);
//		//				printf("\n");
//		//			}
//		//			
//		//			printf("\n");
//		
//#endif	
//		
//		if (childrenList1) {delete childrenList1; childrenList1=0;}
//		if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//		//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
//		
//	}// for i
//	
//	
//	// reconstruct the optimal match from stored best matches
//	
//	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
//	Tree1->getRootID(rootid1, rootidx1);
//	Tree2->getRootID(rootid2, rootidx2);
//	
//	// temporary
//	//	rootid1 = 209;	rootid2 = 64;
//	//	rootid1 = 176;	rootid2 = 57;
//	//	rootid1 = 131;	rootid2 = 50;
//	//	rootid1 = 134;	rootid2 = 50;
//	
//	Tree1->getIndex(rootid1, rootidx1);
//	Tree2->getIndex(rootid2, rootidx2);
//	
//	if (nodeNumThre==0)
//		simMeasure = similarity1d[rootidx1*nodeNum2+rootidx2];
//	
//	if (nodeNumThre ==2)
//		simMeasure = similarity1d[Tree1->node[rootidx1].nid*nodeNum2+Tree2->node[rootidx2].nid];
//	
//	printf("simMeasure = %f\n", simMeasure);
//	
//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//		if (nodeNumThre == 0)
//			idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
//		
//		if (nodeNumThre == 2)
//		{   
//			V3DLONG tmpidx1, tmpidx2;
//			V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//			V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//			
//			if (m==rootidx1) //add root
//				idx = rootidx2;
//			else
//			{
//				Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
//				Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
//				
//				if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
//					printf("Root should have only one child\n");
//				
//				//			Tree1->getIndex(Tree1->node[rootidx1].nid, tmpidx1);
//				//			Tree2->getIndex(Tree2->node[rootidx2].nid, tmpidx2);
//				
//				idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
//			}
//		}
//		
//		if (idx>=0)
//		{
//			matchingList[m] = Tree2->node[idx].nid;
//			//			printf("%f, %d, %d\n", calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[m].nid, Tree2->node[idx].nid), Tree1->node[m].nid, Tree2->node[idx].nid);
//			
//		}	
//		else
//			matchingList[m] = INVALID_VALUE;
//	}
//	
//	
//	
//	//#ifdef DEBUG	
//	
//	//print similarity matrix
//	printf("Similarity matrix\n");
//	
//	for (i=0; i<nodeNum1; i++)
//	{
//		for (j=0; j<nodeNum2; j++)
//			printf("%f ", similarity1d[i*nodeNum2+j]);
//		printf("\n");
//	}
//	
//	//	//print similarity matrix
//	//	printf("Dissimilarity matrix\n");
//	//	
//	//	for (i=0; i<nodeNum1; i++)
//	//	{
//	//		for (j=0; j<nodeNum2; j++)
//	//			printf("%f ", dissimilarity1d[i*nodeNum2+j]);
//	//		printf("\n");
//	//	}
//	
//	//#endif	
//	
//	// ---------------------------------------
//	// delete pointers
//	// ---------------------------------------
//	
//	if (subTreeNodeNum1) {delete []subTreeNodeNum1; subTreeNodeNum1=0;}
//	if (subTreeNodeNum2) {delete []subTreeNodeNum2; subTreeNodeNum2=0;}
//	
//	if (sortval1) {delete []sortval1; sortval1=0;}
//    if (sortval2) {delete []sortval2; sortval2=0;}
//	if (sortidx1) {delete []sortidx1; sortidx1=0;}
//	if (sortidx2) {delete []sortidx2; sortidx2=0;}
//	
//	if (similarity1d) {delete []similarity1d; similarity1d=0;}
//	//	if (dissimilarity1d) {delete []dissimilarity1d; dissimilarity1d=0;}
//	
//	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
//	
//}
//
//

////************************************************************************************
//// match two trees using dynamic programming, 
//// every node (root, continual nodes, branching nodes, leaf nodes) will all find matches
////************************************************************************************

//bool treeMatching_allNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
//{
//	V3DLONG i, j, m, n, p,q;
//	V3DLONG nodeNum1, nodeNum2;
//	nodeNum1 = Tree1->treeNodeNum;
//	nodeNum2 = Tree2->treeNodeNum;
//	
//	// -----------------------------------------------------------------
//	// get the number of nodes in the subtree rooted at each node
//	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
//	// -----------------------------------------------------------------	
//	V3DLONG *subTreeNodeNum1 = 0;
//	V3DLONG *subTreeNodeNum2 = 0;
//	
//	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
//	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 
//	
//#ifdef DEBUG
//	
//	printf("print subTreeNodeNum1 returned from Tree1->getSubTreeNodeNum(subTreeNodeNum1)\n");	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum1[i]);
//	printf("\n");
//	
//	printf("print subTreeNodeNum2 returned from Tree2->getSubTreeNodeNum(subTreeNodeNum2)\n");	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum2[i]);
//	printf("\n");
//	
//#endif
//	
//	// ------------------------------------------------------------
//	// sort subTreeNodeNum1 and subTreeNodeNum2 from small to large
//	// ------------------------------------------------------------
//	float *sortval1, *sortidx1;
//	float *sortval2, *sortidx2;
//	
//	Tree1->sortSubTreeNodeNum(subTreeNodeNum1, sortval1, sortidx1);
//	Tree2->sortSubTreeNodeNum(subTreeNodeNum2, sortval2, sortidx2);
//	
//#ifdef DEBUG
//	
//	printf("print sortval1 and sortidx1\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval1[i+1], sortidx1[i+1]);
//	printf("\n");
//	
//	printf("print sortval2 and sortidx2\n");
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval2[i+1], sortidx2[i+1]);
//	printf("\n");
//	
//#endif
//	
//	
//	// ---------------------------------------
//	// tree matching using dynamic programming
//	// ---------------------------------------
//	
//	V3DLONG *childrenList1=0, *childrenNodeIdx1=0;
//	V3DLONG *childrenList2 =0, *childrenNodeIdx2 = 0;	
//	V3DLONG childrenNum1, childrenNum2;
//	V3DLONG nodeidx1, nodeidx2;
//	V3DLONG depthThre = 4;
//	
//	// allocate memory for similarity1d and intialize
//	float *similarity1d = new float [nodeNum1*nodeNum2]; // optimal similarity
//	if (!similarity1d)
//	{
//		printf("Fail to allocate memory for similarity1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//		{
//			////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
//			similarity1d[m*nodeNum2 + n] =  -1;
//		}
//	
//	
//	// allocate memory for mappingfunc1d and intialize	
//	V3DLONG *mappingfunc1d = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching
//	if (!mappingfunc1d)
//	{
//		printf("Fail to allocate memory for mappingfunc1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//			for (p=0; p<nodeNum1; p++)
//				mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
//	
//	// initialize matching list
//	matchingList = new V3DLONG [nodeNum1]; //indicating for each node in tree1, the id# of the matching point in tree2
//	
//	for (i=0; i<nodeNum1; i++)
//		matchingList[i] = -1; //need to check i
//	
//	// start matching
//	for (i=0; i<nodeNum1; i++)
//	{
//		
//		
//		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1, childrenNodeIdx1, childrenNum1); // get direct children for the current node in Tree1
//		
//		//		if (childrenNum1 < nodeNumThre)
//		//		{
//		//			for (m=0; m<nodeNum2; m++)
//		//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[m+1]] = 1;		
//		//			continue;
//		//		}
//		//		
//		////		// construct super node set
//		////		V3DLONG *superNodeIdx1 = new V3DLONG [choldrenNum1];
//		////		superNodeIdx1[0] = sortidx1[i+1]];
//		////		for (int tt = 0; tt< childrenNum1; tt++)
//		////			superNodeIdx1[tt+1] = childrenNodeIdx1[tt];
//		////		
//		
//#ifdef DEBUG
//		
//		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenList1[m]);
//		printf("\n");
//		
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenNodeIdx1[m]);
//		printf("\n\n");
//		
//#endif
//		// find candidate nodes in Tree2 that can be matched to the current node in tree1
//		
//		for (j=0; j<nodeNum2; j++) 
//		{
//			
//			
//			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for a node in Tree2
//			//			Tree2->getAllChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); //get all children for the current node in Tree2
//			
//			//			if (childrenNum2 < nodeNumThre)
//			//			{
//			//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 1;
//			//				continue;
//			//			}
//			//
//			////			// construct super node set
//			////			V3DLONG *superNodeIdx2 = new V3DLONG [choldrenNum2];
//			////			superNodeIdx2[0] = sortidx2[i+1]];
//			////			for (int tt = 0; tt< childrenNum2; tt++)
//			////				superNodeIdx2[tt+1] = childrenNodeIdx2[tt];
//			
//#ifdef DEBUG
//			
//			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenList2[m]);
//			printf("\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenNodeIdx2[m]);
//			printf("\n\n");
//#endif			
//			
//			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);			
//			//			printf("%d, %d, %d, %d\n", i,j, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			
//			for (int p = 0; p<nodeNum1; p++)
//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			
//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			//#ifdef DEBUG	
//			//			printf("The initial similarity:\n");
//			//			printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1dtmp);
//			//
//			//#endif			
//			
//			// find the best matching for subtree using bipartite matching			
//			if ((childrenNum1!=0)&&(childrenNum2!=0)) // if one of the current nodes in tree1 and tree2 are leaf nodes, do not do the following
//			{
//				
//				Matrix<double> bpMatrix(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix, need to free?
//				
//				for (m=0; m<childrenNum1+childrenNum2; m++)
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						//						bpMatrix(m,n) = 9999;
//						bpMatrix(m,n) = -INVALID_VALUE;
//				
//				// build the matrix
//				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1*childrenNum2*2];
//				
//				for (m=0; m<childrenNum1; m++)
//					for (n=0; n<childrenNum2; n++)
//					{
//						bestSubMatch[m*childrenNum2*2+n*2] = INVALID_VALUE;
//						bestSubMatch[m*childrenNum2*2+n*2+1] = INVALID_VALUE;
//					}
//				
//				for (m=0; m<childrenNum1; m++)
//				{
//					Tree1->getIndex(childrenList1[m], nodeidx1);
//					
//					//get all children rooted at childrenList1[m], 
//					
//					V3DLONG *subTreeNodeIdx1 = 0;
//					swcTree *subTree1 = 0; 
//					//					Tree1->getSubTree(childrenList1[m], subTree1, subTreeNodeIdx1); // include root of the subtree, i.e., nodeid
//					Tree1->getSubTree(childrenList1[m], depthThre, subTree1, subTreeNodeIdx1);
//					
//#ifdef DEBUG
//					printf("subTreeNodeIdx1: ");
//					
//					for (p=0; p<subTree1->treeNodeNum; p++)
//						printf("%d, ", subTreeNodeIdx1[p]);
//					printf("\n");
//#endif					
//					for (n=0; n<childrenNum2; n++)
//					{
//						Tree2->getIndex(childrenList2[n], nodeidx2);
//						
//					    // find the best one in the subtree rooted at nodeidx2
//						V3DLONG *subTreeNodeIdx2 = 0;
//						swcTree *subTree2 = 0; 
//						//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
//						Tree2->getSubTree(childrenList2[n], depthThre, subTree2, subTreeNodeIdx2);
//#ifdef DEBUG
//						printf("subTreeNodeIdx2: ");
//						
//						for (p=0; p<subTree2->treeNodeNum; p++)
//							printf("%d, ", subTreeNodeIdx2[p]);
//						printf("\n");
//#endif						
//						// find the best match with highest similarity in the subtree rooted at childrenList1[m] and childreList2[m]
//						
//						float bestval = INVALID_VALUE;
//						for (p=0; p<subTree1->treeNodeNum; p++)
//							for (q=0; q<subTree2->treeNodeNum; q++)
//								if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//								{
//									bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//									bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
//									bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
//									
//								}
//						//						bpMatrix(m,n) = 9999 - bestval; // hungarian method use distance 
//						bpMatrix(m,n) = -bestval; // hungarian method use distance 
//						
//						if (subTree2) {delete subTree2; subTree2=0;}
//						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
//						
//					} // for n
//					
//					if (subTree1) {delete subTree1; subTree1=0;}
//					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}
//					
//				} // for m
//				
//				// Hungarian algorithm for bipartite matching, find the minimum cost
//				Matrix<double> bpMatrixOut(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix
//				
//				for (m=0; m<childrenNum1+childrenNum2; m++)
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						bpMatrixOut(m,n) = bpMatrix(m,n);
//				
//				Munkres branchMatch;
//				branchMatch.solve(bpMatrixOut); 
//				
//				//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				//				{
//				//					for (n=0; n<childrenNum1+childrenNum2; n++)
//				//						printf("%f ", bpMatrixOut(m,n));
//				//					printf("\n");
//				//			    }
//				//				printf("\n");
//				
//				
//				// get temporary simiarlity1d, and matchingfunc1d
//				
//				for (m=0; m<childrenNum1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
//				{
//					Tree1->getIndex(childrenList1[m], nodeidx1);
//					
//					for (n=0; n<childrenNum2; n++) 
//					{
//						//					printf("%f\n ", bpMatrix(m,n));
//						//						printf("%f\n ", bpMatrixOut(m,n));
//						
//						Tree2->getIndex(childrenList2[n], nodeidx2);
//						
//						if (bpMatrixOut(m,n)==0) // find a match
//						{
//							// update similarity1d and mappingfunc1d						
//							// //							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += similarity1d[nodeidx1*nodeNum2+nodeidx2];
//							//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += (9999-bpMatrix(m,n));
//							
//							//							printf("%f ", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//							
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
//							
//							//							// update dissimilarity1d and mappingfunc1d						
//							//							dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += bpMatrix(m,n);
//							
//							V3DLONG ind1 = bestSubMatch[m*childrenNum2*2+n*2];
//							V3DLONG ind2 = bestSubMatch[m*childrenNum2*2+n*2+1];
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
//							
//							//expand matching list of the children
//							for (p = 0; p<nodeNum1; p++)
//							{	
//								
//								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
//								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
//									mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//									mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
//								
//							} //for p
//						} //if (bpMatrixOut(m,n)==0)
//					} // for n
//					//					printf("\n");
//				} // for m	
//				
//				//				// normalize similarity1d, make it irrelavant to the number of matching
//				//				V3DLONG matchcnt = 0;
//				//				for (p = 0; p<nodeNum1; p++)
//				//					if (mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]!=INVALID_VALUE)
//				//						matchcnt++;
//				//						
//				//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] /= matchcnt;	
//				
//				
//				//				if (bestnodeid) {delete []bestnodeid; bestnodeid = 0;}
//				
//				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
//				
//#ifdef DEBUG				
//				printf("The final similarity involving the similarities of the children:\n"); 
//				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//#endif			
//			} //if ((childrenNum1!=0)&&(childrenNum2!=0))
//			
//			
//#ifdef DEBUG	
//			
//			//print similarity matrix
//			printf("Similarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", similarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
//			
//#endif	
//			
//			
//			if (childrenList2) {delete childrenList2; childrenList2=0;}
//			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
//		} //for (j=0; j<nodeNum2; j++)
//		
//		
//#ifdef DEBUG	
//		
//		//print similarity matrix
//		printf("Similarity matrix\n");
//		printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//		printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//		printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//		
//		for (m=0; m<nodeNum1; m++)
//		{
//			for (n=0; n<nodeNum2; n++)
//				printf("%f ", similarity1d[m*nodeNum2+n]);
//			printf("\n");
//		}
//		
//		printf("\n");
//		
//#endif	
//		
//		if (childrenList1) {delete childrenList1; childrenList1=0;}
//		if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//		//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
//		
//	}// for i
//	
//	
//	// reconstruct the optimal match from stored best matches
//	
//	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
//	Tree1->getRootID(rootid1, rootidx1);
//	Tree2->getRootID(rootid2, rootidx2);
//	
//	Tree1->getIndex(rootid1, rootidx1);
//	Tree2->getIndex(rootid2, rootidx2);
//	
//	//	if (nodeNumThre==0)
//	simMeasure = similarity1d[rootidx1*nodeNum2+rootidx2];
//	
//	//	if (nodeNumThre ==2)
//	//		simMeasure = similarity1d[Tree1->node[rootidx1].nid*nodeNum2+Tree2->node[rootidx2].nid];
//	
//	printf("simMeasure = %f\n", simMeasure);
//	
//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//		//		if (nodeNumThre == 0)
//		idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
//		
//		
//		if (idx>=0)
//		{
//			matchingList[m] = Tree2->node[idx].nid;
//			//			printf("%f, %d, %d\n", calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[m].nid, Tree2->node[idx].nid), Tree1->node[m].nid, Tree2->node[idx].nid);
//			
//		}	
//		else
//			matchingList[m] = INVALID_VALUE;
//	}
//	
//	
//	
//	//#ifdef DEBUG	
//	
//	//print similarity matrix
//	printf("Similarity matrix\n");
//	
//	for (i=0; i<nodeNum1; i++)
//	{
//		for (j=0; j<nodeNum2; j++)
//			printf("%f ", similarity1d[i*nodeNum2+j]);
//		printf("\n");
//	}
//	
//	//#endif	
//	
//	// ---------------------------------------
//	// delete pointers
//	// ---------------------------------------
//	
//	if (subTreeNodeNum1) {delete []subTreeNodeNum1; subTreeNodeNum1=0;}
//	if (subTreeNodeNum2) {delete []subTreeNodeNum2; subTreeNodeNum2=0;}
//	
//	if (sortval1) {delete []sortval1; sortval1=0;}
//    if (sortval2) {delete []sortval2; sortval2=0;}
//	if (sortidx1) {delete []sortidx1; sortidx1=0;}
//	if (sortidx2) {delete []sortidx2; sortidx2=0;}
//	
//	if (similarity1d) {delete []similarity1d; similarity1d=0;}
//	//	if (dissimilarity1d) {delete []dissimilarity1d; dissimilarity1d=0;}
//	
//	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
//	
//}
//
//

////************************************************************************************
//// match two trees using dynamic programming
//// root, continual nodes, leaf nodes will find matches, continual nodes will not find matches
//// suitable for matching neurons of the same structures
////************************************************************************************

//bool treeMatching_noContinualNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
//{
//	V3DLONG i, j, m, n, p,q;
//	
//	//	float nodeNumThre = 2; // nodes with subtree node number no less than this value will be considered in match, 2 for matching neurons by structures, 0 for matching neurons by geometrical features
//	
//	
//	// -----------------------------------------------------------------
//	// generate trees only containing root, branching points, and leaf nodes
//	// remove continual points
//	// the input tree pointers are backed up in Tree1_old, Tree2_old
//	// the trees after removing continual nodes are Tree1 and Tree2
//	// -----------------------------------------------------------------	
//	swcTree *Tree1_old =0, *Tree2_old = 0;
//	swcTree *newTree1 =0, *newTree2 = 0;
//	unsigned char *removeTag1=0, *removeTag2=0;
//	
//	Tree1_old = Tree1;
//	Tree2_old = Tree2;
//	
//	//	Tree1->removeContinualNodes(newTree1, removeTag1);
//	Tree1->removeContinuaLeaflNodes(newTree1, removeTag1);
//	Tree1 = newTree1;
//	newTree1 = 0;
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//	{
//		printf("%d ", Tree1->node[i].nid);
//	}
//	printf("\n");
//	
//	//	if (Tree1) {delete Tree1; Tree1=newTree1; newTree1=0;} 
//	
//	//	Tree2->removeContinualNodes(newTree2, removeTag2);
//	Tree2->removeContinuaLeaflNodes(newTree2, removeTag2);
//	Tree2 = newTree2;
//	newTree2 = 0;
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//	{
//		printf("%d ", Tree2->node[i].nid);
//	}
//	printf("\n");
//	
//	//	if (Tree2) {delete Tree2; Tree2=newTree2; newTree2=0;} 
//	
//	// -----------------------------------------------------------------
//	// get the number of nodes in the subtree rooted at each node
//	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
//	// -----------------------------------------------------------------	
//	V3DLONG *subTreeNodeNum1 = 0;
//	V3DLONG *subTreeNodeNum2 = 0;
//	
//	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
//	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 
//	
//#ifdef DEBUG
//	
//	printf("print subTreeNodeNum1 returned from Tree1->getSubTreeNodeNum(subTreeNodeNum1)\n");	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum1[i]);
//	printf("\n");
//	
//	printf("print subTreeNodeNum2 returned from Tree2->getSubTreeNodeNum(subTreeNodeNum2)\n");	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum2[i]);
//	printf("\n");
//	
//#endif
//	
//	// ------------------------------------------------------------
//	// sort subTreeNodeNum1 and subTreeNodeNum2 from small to large
//	// ------------------------------------------------------------
//	float *sortval1, *sortidx1;
//	float *sortval2, *sortidx2;
//	
//	Tree1->sortSubTreeNodeNum(subTreeNodeNum1, sortval1, sortidx1);
//	Tree2->sortSubTreeNodeNum(subTreeNodeNum2, sortval2, sortidx2);
//	
//#ifdef DEBUG
//	
//	printf("print sortval1 and sortidx1\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval1[i+1], sortidx1[i+1]);
//	printf("\n");
//	
//	printf("print sortval2 and sortidx2\n");
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval2[i+1], sortidx2[i+1]);
//	printf("\n");
//	
//#endif
//	
//	
//	// ---------------------------------------
//	// tree matching using dynamic programming
//	// ---------------------------------------
//	V3DLONG nodeNum1, nodeNum2;
//	nodeNum1 = Tree1->treeNodeNum;
//	nodeNum2 = Tree2->treeNodeNum;
//	
//	V3DLONG *childrenList1=0, *childrenNodeIdx1=0;
//	V3DLONG *childrenList2 =0, *childrenNodeIdx2 = 0;	
//	V3DLONG childrenNum1, childrenNum2;
//	V3DLONG nodeidx1, nodeidx2;
//	V3DLONG depthThre = 4;
//	
//	// allocate memory for similarity1d and intialize
//	float *similarity1d = new float [nodeNum1*nodeNum2]; // optimal similarity
//	if (!similarity1d)
//	{
//		printf("Fail to allocate memory for similarity1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//		{
//			////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
//			similarity1d[m*nodeNum2 + n] =  -1;
//			//		similarity1d[m*nodeNum2 + n] =  0;
//		}
//	
//	
//	// allocate memory for mappingfunc1d and intialize	
//	V3DLONG *mappingfunc1d = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching
//	if (!mappingfunc1d)
//	{
//		printf("Fail to allocate memory for mappingfunc1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//			for (p=0; p<nodeNum1; p++)
//				mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
//	
//	// initialize matching list, the size is determined by the original Tree1, i.e., Tree1_old
//	matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
//	
//	for (i=0; i<Tree1_old->treeNodeNum; i++)
//		matchingList[i] = INVALID_VALUE; //need to check i
//	
//	// start matching
//	for (i=0; i<nodeNum1; i++)
//	{
//		
//		
//		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1, childrenNodeIdx1, childrenNum1); // get direct children for the current node in Tree1
//		
//		//		if (childrenNum1 == 0) // leaf node, note that Tree1 and Tree2 now only contain root, branching nodes, and leaf nodes
//		//		{
//		//			for (m=0; m<nodeNum2; m++)
//		//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[m+1]] = 1;		
//		//			continue;
//		//		}
//		//		
//		////		// construct super node set
//		////		V3DLONG *superNodeIdx1 = new V3DLONG [choldrenNum1];
//		////		superNodeIdx1[0] = sortidx1[i+1]];
//		////		for (int tt = 0; tt< childrenNum1; tt++)
//		////			superNodeIdx1[tt+1] = childrenNodeIdx1[tt];
//		//		
//		
//#ifdef DEBUG
//		
//		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenList1[m]);
//		printf("\n");
//		
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenNodeIdx1[m]);
//		printf("\n\n");
//		
//#endif
//		// find candidate nodes in Tree2 that can be matched to the current node in tree1
//		
//		for (j=0; j<nodeNum2; j++) 
//		{
//			
//			
//			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for a node in Tree2
//			//			Tree2->getAllChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); //get all children for the current node in Tree2
//			
//			//			if (childrenNum2 == 0) //leaf nodes
//			//			{
//			//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 1;
//			//				continue;
//			//			}
//			////			// construct super node set
//			////			V3DLONG *superNodeIdx2 = new V3DLONG [choldrenNum2];
//			////			superNodeIdx2[0] = sortidx2[i+1]];
//			////			for (int tt = 0; tt< childrenNum2; tt++)
//			////				superNodeIdx2[tt+1] = childrenNodeIdx2[tt];
//			
//#ifdef DEBUG
//			
//			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenList2[m]);
//			printf("\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenNodeIdx2[m]);
//			printf("\n\n");
//#endif			
//			
//			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
//			//			
//			////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_length_weighted_distance(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			
//			//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (int p = 0; p<nodeNum1; p++)
//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			
//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			//#ifdef DEBUG	
//			//			printf("The initial similarity:\n");
//			//			printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1dtmp);
//			//
//			//#endif			
//			
//			// find the best matching for subtree using bipartite matching			
//			if ((childrenNum1!=0)&&(childrenNum2!=0)) // if one of the current nodes in tree1 and tree2 are leaf nodes, do not do the following
//			{
//				
//				Matrix<double> bpMatrix(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix, need to free?
//				
//				for (m=0; m<childrenNum1+childrenNum2; m++)
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						bpMatrix(m,n) = -INVALID_VALUE;
//				
//				// build the matrix
//				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1*childrenNum2*2];
//				
//				for (m=0; m<childrenNum1; m++)
//					for (n=0; n<childrenNum2; n++)
//					{
//						bestSubMatch[m*childrenNum2*2+n*2] = INVALID_VALUE;
//						bestSubMatch[m*childrenNum2*2+n*2+1] = INVALID_VALUE;
//					}
//				
//				for (m=0; m<childrenNum1; m++)
//				{
//					Tree1->getIndex(childrenList1[m], nodeidx1);
//					
//					//get all children rooted at childrenList1[m], 
//					
//					V3DLONG *subTreeNodeIdx1 = 0;
//					swcTree *subTree1 = 0; 
//					//					Tree1->getSubTree(childrenList1[m], subTree1, subTreeNodeIdx1); // include root of the subtree, i.e., nodeid
//					Tree1->getSubTree(childrenList1[m], depthThre, subTree1, subTreeNodeIdx1);
//					
//#ifdef DEBUG
//					printf("subTreeNodeIdx1: ");
//					
//					for (p=0; p<subTree1->treeNodeNum; p++)
//						printf("%d, ", subTreeNodeIdx1[p]);
//					printf("\n");
//#endif					
//					for (n=0; n<childrenNum2; n++)
//					{
//						Tree2->getIndex(childrenList2[n], nodeidx2);
//						
//					    // find the best one in the subtree rooted at nodeidx2
//						V3DLONG *subTreeNodeIdx2 = 0;
//						swcTree *subTree2 = 0; 
//						//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
//						Tree2->getSubTree(childrenList2[n], depthThre, subTree2, subTreeNodeIdx2);
//#ifdef DEBUG
//						printf("subTreeNodeIdx2: ");
//						
//						for (p=0; p<subTree2->treeNodeNum; p++)
//							printf("%d, ", subTreeNodeIdx2[p]);
//						printf("\n");
//#endif						
//						// find the best match with highest similarity in the subtree rooted at childrenList1[m] and childreList2[m]
//						
//						float bestval = INVALID_VALUE;
//						for (p=0; p<subTree1->treeNodeNum; p++)
//							for (q=0; q<subTree2->treeNodeNum; q++)
//								if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//								{
//									bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//									bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
//									bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
//									
//								}
//						//						bpMatrix(m,n) = 9999 - bestval; // hungarian method use distance 
//						bpMatrix(m,n) = -bestval; // hungarian method use distance 
//						
//						if (subTree2) {delete subTree2; subTree2=0;}
//						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
//						
//					} // for n
//					
//					if (subTree1) {delete subTree1; subTree1=0;}
//					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}
//					
//				} // for m
//				
//				// Hungarian algorithm for bipartite matching, find the minimum cost
//				Matrix<double> bpMatrixOut(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix
//				
//				for (m=0; m<childrenNum1+childrenNum2; m++)
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						bpMatrixOut(m,n) = bpMatrix(m,n);
//				
//				Munkres branchMatch;
//				branchMatch.solve(bpMatrixOut); 
//				
//				//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				//				{
//				//					for (n=0; n<childrenNum1+childrenNum2; n++)
//				//						printf("%f ", bpMatrixOut(m,n));
//				//					printf("\n");
//				//			    }
//				//				printf("\n");
//				
//				
//				// get temporary simiarlity1d, and matchingfunc1d
//				
//				for (m=0; m<childrenNum1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
//				{
//					Tree1->getIndex(childrenList1[m], nodeidx1);
//					
//					for (n=0; n<childrenNum2; n++) 
//					{
//						//					printf("%f\n ", bpMatrix(m,n));
//						//						printf("%f\n ", bpMatrixOut(m,n));
//						
//						Tree2->getIndex(childrenList2[n], nodeidx2);
//						
//						if (bpMatrixOut(m,n)==0) // find a match
//						{
//							// update similarity1d and mappingfunc1d						
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
//							
//							V3DLONG ind1 = bestSubMatch[m*childrenNum2*2+n*2];
//							V3DLONG ind2 = bestSubMatch[m*childrenNum2*2+n*2+1];
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
//							
//							//expand matching list of the children
//							for (p = 0; p<nodeNum1; p++)
//							{	
//								
//								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
//								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
//									mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//									mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
//								
//							} //for p
//						} //if (bpMatrixOut(m,n)==0)
//					} // for n
//					//					printf("\n");
//				} // for m	
//				
//				//				// normalize similarity1d, make it irrelavant to the number of matching
//				//				V3DLONG matchcnt = 0;
//				//				for (p = 0; p<nodeNum1; p++)
//				//					if (mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]!=INVALID_VALUE)
//				//						matchcnt++;
//				//						
//				//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] /= matchcnt;	
//				
//				
//				//				if (bestnodeid) {delete []bestnodeid; bestnodeid = 0;}
//				
//				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
//				
//#ifdef DEBUG				
//				printf("The final similarity involving the similarities of the children:\n"); 
//				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//#endif			
//			} //if ((childrenNum1!=0)&&(childrenNum2!=0))
//			
//			
//#ifdef DEBUG	
//			
//			//print similarity matrix
//			printf("Similarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", similarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
//			
//#endif	
//			
//			
//			if (childrenList2) {delete childrenList2; childrenList2=0;}
//			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
//		} //for (j=0; j<nodeNum2; j++)
//		
//		
//#ifdef DEBUG	
//		
//		//print similarity matrix
//		printf("Similarity matrix\n");
//		printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//		printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//		printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//		
//		for (m=0; m<nodeNum1; m++)
//		{
//			for (n=0; n<nodeNum2; n++)
//				printf("%f ", similarity1d[m*nodeNum2+n]);
//			printf("\n");
//		}
//		
//		printf("\n");
//		
//		
//#endif	
//		
//		if (childrenList1) {delete childrenList1; childrenList1=0;}
//		if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//		//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
//		
//	}// for i
//	
//	
//	// reconstruct the optimal match from stored best matches
//	
//	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
//	Tree1->getRootID(rootid1, rootidx1);
//	Tree2->getRootID(rootid2, rootidx2);
//	
//	Tree1->getIndex(rootid1, rootidx1);
//	Tree2->getIndex(rootid2, rootidx2);
//	
//	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//	
//	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
//	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
//	
//	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
//		printf("Root should have only one child\n");
//	
//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//		
//		
//		if (m==rootidx1) //add root
//			idx = rootidx2;
//		else
//			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
//		
//		V3DLONG tmpidx1;
//		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
//		
//		if (idx>=0)
//		{
//			matchingList[tmpidx1] = Tree2->node[idx].nid;		
//			//			matchingList[m] = Tree2->node[idx].nid;
//			
//		}	
//		else
//			matchingList[tmpidx1] = INVALID_VALUE;
//		//			matchingList[m] = INVALID_VALUE;
//	}
//	
//	//	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	
//	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	printf("simMeasure = %f\n", simMeasure);
//	
//	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
//	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
//	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
//	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}
//	
//	//	for (m=0; m<nodeNum1; m++)
//	//	{
//	//		V3DLONG idx;
//	//	
//	//		V3DLONG tmpidx1, tmpidx2;
//	//		V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//	//		V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//	//		
//	//		if (m==rootidx1) //add root
//	//			idx = rootidx2;
//	//		else
//	//		{
//	//			Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
//	//			Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
//	//			
//	//			if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
//	//				printf("Root should have only one child\n");
//	//							
//	//			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
//	//		}
//	//		
//	//		
//	//		if (idx>=0)
//	//		{
//	//			matchingList[m] = Tree2->node[idx].nid;
//	////			printf("%f, %d, %d\n", calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[m].nid, Tree2->node[idx].nid), Tree1->node[m].nid, Tree2->node[idx].nid);
//	//			
//	//		}	
//	//		else
//	//			matchingList[m] = INVALID_VALUE;
//	//	}
//	//
//	//	simMeasure = similarity1d[Tree1->node[rootidx1].nid*nodeNum2+Tree2->node[rootidx2].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	//	
//	
//	
//	
//	
//	//#ifdef DEBUG	
//	
//	//print similarity matrix
//	printf("Similarity matrix\n");
//	
//	for (i=0; i<nodeNum1; i++)
//	{
//		for (j=0; j<nodeNum2; j++)
//			printf("%f ", similarity1d[i*nodeNum2+j]);
//		printf("\n");
//	}
//	
//	// ---------------------------------------
//	// delete pointers
//	// ---------------------------------------
//	
//	if (subTreeNodeNum1) {delete []subTreeNodeNum1; subTreeNodeNum1=0;}
//	if (subTreeNodeNum2) {delete []subTreeNodeNum2; subTreeNodeNum2=0;}
//	
//	if (sortval1) {delete []sortval1; sortval1=0;}
//    if (sortval2) {delete []sortval2; sortval2=0;}
//	if (sortidx1) {delete []sortidx1; sortidx1=0;}
//	if (sortidx2) {delete []sortidx2; sortidx2=0;}
//	
//	if (similarity1d) {delete []similarity1d; similarity1d=0;}
//	
//	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
//	
//	//recover pointer
//	if (Tree1) {delete Tree1; Tree1=Tree1_old; Tree1_old=0;} 	
//	if (Tree2) {delete Tree2; Tree2=Tree2_old; Tree2_old=0;} 
//	
//	if (removeTag1) {delete removeTag1; removeTag1=0;}
//	if (removeTag2) {delete removeTag2; removeTag2=0;}
//	
//}
//

////************************************************************************************
//// match two trees using dynamic programming
//// Different from treeMatching_noContinualNodes, the similarity metrics (length-ratio) are dependent between parent and 
//// children, i.e., which branch match to which branch when computing legnth-ratio should be consistent with the bipartite matching
//// result of children nodes
//// root, branching nodes, continual and leaf nodes are removed before matching, thus will not find matches
////************************************************************************************

//bool treeMatching_noContinualLeafNodes_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
//{
//	V3DLONG i, j, m, n, p,q;
//	
//	
//	// -----------------------------------------------------------------
//	// generate trees only containing root, branching points, and leaf nodes
//	// remove continual points
//	// the input tree pointers are backed up in Tree1_old, Tree2_old
//	// the trees after removing continual nodes are Tree1 and Tree2
//	// -----------------------------------------------------------------	
//	swcTree *Tree1_old =0, *Tree2_old = 0;
//	swcTree *newTree1 =0, *newTree2 = 0;
//	unsigned char *removeTag1=0, *removeTag2=0;
//	
//	Tree1_old = Tree1;
//	Tree2_old = Tree2;
//	
//	//	Tree1->removeContinualNodes(newTree1, removeTag1);
//	Tree1->removeContinuaLeaflNodes(newTree1, removeTag1);
//	Tree1 = newTree1;
//	newTree1 = 0;
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//	{
//		printf("%d ", Tree1->node[i].nid);
//	}
//	printf("\n");
//	
//	//	if (Tree1) {delete Tree1; Tree1=newTree1; newTree1=0;} 
//	
//	//	Tree2->removeContinualNodes(newTree2, removeTag2);
//	Tree2->removeContinuaLeaflNodes(newTree2, removeTag2);
//	Tree2 = newTree2;
//	newTree2 = 0;
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//	{
//		printf("%d ", Tree2->node[i].nid);
//	}
//	printf("\n");
//	
//	//	if (Tree2) {delete Tree2; Tree2=newTree2; newTree2=0;} 
//	
//	// -----------------------------------------------------------------
//	// get the number of nodes in the subtree rooted at each node
//	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
//	// -----------------------------------------------------------------	
//	V3DLONG *subTreeNodeNum1 = 0;
//	V3DLONG *subTreeNodeNum2 = 0;
//	
//	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
//	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 
//	
//#ifdef DEBUG
//	
//	printf("print subTreeNodeNum1 returned from Tree1->getSubTreeNodeNum(subTreeNodeNum1)\n");	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum1[i]);
//	printf("\n");
//	
//	printf("print subTreeNodeNum2 returned from Tree2->getSubTreeNodeNum(subTreeNodeNum2)\n");	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum2[i]);
//	printf("\n");
//	
//#endif
//	
//	// ------------------------------------------------------------
//	// sort subTreeNodeNum1 and subTreeNodeNum2 from small to large
//	// ------------------------------------------------------------
//	float *sortval1, *sortidx1;
//	float *sortval2, *sortidx2;
//	
//	Tree1->sortSubTreeNodeNum(subTreeNodeNum1, sortval1, sortidx1);
//	Tree2->sortSubTreeNodeNum(subTreeNodeNum2, sortval2, sortidx2);
//	
//#ifdef DEBUG
//	
//	printf("print sortval1 and sortidx1\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval1[i+1], sortidx1[i+1]);
//	printf("\n");
//	
//	printf("print sortval2 and sortidx2\n");
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval2[i+1], sortidx2[i+1]);
//	printf("\n");
//	
//#endif
//	
//	
//	// ---------------------------------------
//	// tree matching using dynamic programming
//	// ---------------------------------------
//	V3DLONG nodeNum1, nodeNum2;
//	nodeNum1 = Tree1->treeNodeNum;
//	nodeNum2 = Tree2->treeNodeNum;
//	
//	V3DLONG nodeidx1, nodeidx2;
//	V3DLONG depthThre = 4;
//	
//	// allocate memory for similarity1d and intialize
//	float *similarity1d = new float [nodeNum1*nodeNum2]; // #(branching nodes + root in Tree 1) *#(branching nodes + root in Tree 2)
//	if (!similarity1d)
//	{
//		printf("Fail to allocate memory for similarity1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//		{
//			////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
//			//		similarity1d[m*nodeNum2 + n] =  -1;
//			similarity1d[m*nodeNum2 + n] =  0;
//		}
//	
//	
//	// allocate memory for mappingfunc1d and intialize	
//	V3DLONG *mappingfunc1d = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching
//	if (!mappingfunc1d)
//	{
//		printf("Fail to allocate memory for mappingfunc1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//			for (p=0; p<nodeNum1; p++)
//				mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
//	
//	// initialize matching list, the size is determined by the original Tree1, i.e., Tree1_old
//	matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
//	
//	for (i=0; i<Tree1_old->treeNodeNum; i++)
//		matchingList[i] = INVALID_VALUE; //need to check i
//	
//	// start matching
//	for (i=0; i<nodeNum1; i++)
//	{
//		
//		V3DLONG *childrenList1_Tree1=0, *childrenNodeIdx1_Tree1=0, childrenNum1_Tree1;
//		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1, childrenNodeIdx1_Tree1, childrenNum1_Tree1); // get direct children for the current node in Tree1
//		
//		V3DLONG *childrenList1_Tree1_old=0, *childrenNodeIdx1_Tree1_old=0, childrenNum1_Tree1_old, childrenNum1_Tree1_sn_old;
//		Tree1_old->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1_old, childrenNodeIdx1_Tree1_old, childrenNum1_Tree1_old); // get direct children for the current node in Tree1
//		
//		
//#ifdef DEBUG
//		
//		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenList1[m]);
//		printf("\n");
//		
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenNodeIdx1[m]);
//		printf("\n\n");
//		
//#endif
//		
//		float *lengthratio1=0;
//		unsigned char lengthratioNum1;
//		Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4);
//		
//		// find candidate nodes in Tree2 that can be matched to the current node in tree1
//		
//		for (j=0; j<nodeNum2; j++) 
//		{
//			
//			printf("i=%d,j=%d\n", i,j);	
//			V3DLONG *childrenList2_Tree2=0, *childrenNodeIdx2_Tree2=0, childrenNum2_Tree2;
//			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2, childrenNodeIdx2_Tree2, childrenNum2_Tree2); // get direct children for the current node in Tree1
//			
//			V3DLONG *childrenList2_Tree2_old=0, *childrenNodeIdx2_Tree2_old=0, childrenNum2_Tree2_old, childrenNum2_Tree2_sn_old;
//			Tree2_old->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2_old, childrenNodeIdx2_Tree2_old, childrenNum2_Tree2_old); // get direct children for the current node in Tree1
//			
//			
//			
//#ifdef DEBUG
//			
//			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenList2[m]);
//			printf("\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenNodeIdx2[m]);
//			printf("\n\n");
//#endif			
//			
//			//			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
//			////			
//			//////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_length_weighted_distance(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			
//			//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			//			for (int p = 0; p<nodeNum1; p++)
//			//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			//				
//			//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			// compute metrics of each branch based on which similarity is further computed
//			float *lengthratio2=0;
//			unsigned char lengthratioNum2;
//			Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4);
//			
//			float *simVector = 0;
//			//			simVector = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			simVector = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
//			
//#ifdef DEBUG
//			for (m=0; m<lengthratioNum1; m++)
//			{
//				for (n=0; n<lengthratioNum2; n++)
//					printf("%f ", simVector[m*lengthratioNum2+n]);
//				printf("\n");
//			}
//			printf("\n");
//#endif			
//			
//			for (int p = 0; p<nodeNum1; p++)
//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			
//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			
//			// find the best matching for subtree using bipartite matching	
//			
//			if ((childrenNum1_Tree1==0)||(childrenNum2_Tree2==0)) 
//				// only compute the local similarit, i.e., the best similarity of length-ratio (no subtree nodes similarity) of the two nodes without
//				// adding the similarity of subtree nodes
//			{
//				Matrix<double> bpMatrix(childrenNum1_Tree1_old+childrenNum2_Tree2_old, childrenNum1_Tree1_old+childrenNum2_Tree2_old);// lengthratioNum1 and lengthratioNum2 are the number of children before pruning 
//				
//				printf("print bpMatrix\n");
//				
//				for (m=0; m<childrenNum1_Tree1_old+childrenNum2_Tree2_old; m++)
//				{
//					for (n=0; n<childrenNum1_Tree1_old+childrenNum2_Tree2_old; n++)
//					{
//						if ((m<childrenNum1_Tree1_old)&&(n<childrenNum2_Tree2_old))
//							bpMatrix(m,n) = -INVALID_VALUE-simVector[m*childrenNum2_Tree2_old+n];
//						else // dummy nodes
//							bpMatrix(m,n) = -INVALID_VALUE;
//						printf("%f ", bpMatrix(m,n));
//					}
//					printf("\n");
//				}
//				
//				Matrix<double> bpMatrixOut(childrenNum1_Tree1_old+childrenNum2_Tree2_old, childrenNum1_Tree1_old+childrenNum2_Tree2_old);// bipartite matrix
//				
//				for (m=0; m<childrenNum1_Tree1_old+childrenNum2_Tree2_old; m++)
//					for (n=0; n<childrenNum1_Tree1_old+childrenNum2_Tree2_old; n++)
//						bpMatrixOut(m,n) = bpMatrix(m,n);
//				
//				Munkres branchMatch;
//				branchMatch.solve(bpMatrixOut); 
//				
//				// compute simiarlity1d
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 0;
//				
//				printf("print bpMatrixOut\n");
//				
//				for (m=0; m<childrenNum1_Tree1_old; m++) // note not necessary to consider m<childrenNum1+childrenNum2
//				{
//					for (n=0; n<childrenNum2_Tree2_old; n++) 
//					{	
//		   				printf("%f ", bpMatrixOut(m,n));
//						
//						if (bpMatrixOut(m,n)==0) // find a match
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += simVector[m*childrenNum2_Tree2_old+n]; 	// update similarity1d and mappingfunc1d						
//					}
//					printf("\n");
//				}
//				
//				printf("%d, %d, %f\n", i,j,similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//				
//			}
//			else // none of the two nodes are leaf nodes in the pruned tree, they may be nodes that have only one child in the pruned trees though
//			{
//				
//				Matrix<double> bpMatrix(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// consider branches before pruning
//				
//				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
//					for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
//						bpMatrix(m,n) = -INVALID_VALUE;
//				
//				//				for (m=0; m<lengthratioNum1+lengthratioNum2; m++)
//				//				for (n=0; n<lengthratioNum1+lengthratioNum2; n++)
//				//				{
//				//					if ((m<lengthratioNum1)&&(n<lengthratioNum2))
//				//						bpMatrix(m,n) = -INVALID_VALUE-simVector[m*lengthratioNum2+n];
//				//					else
//				//						bpMatrix(m,n) = -INVALID_VALUE;
//				//				}
//				
//				//				initialize bestSubMatch
//				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1_Tree1*childrenNum2_Tree2*2];
//				
//				for (m=0; m<childrenNum1_Tree1; m++)
//					for (n=0; n<childrenNum2_Tree2; n++)
//					{
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2] = INVALID_VALUE;
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = INVALID_VALUE;
//					}
//				
//				// assign values to bpMatrix				
//				for (m=0; m<childrenNum1_Tree1; m++)
//				{
//					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1); 
//					
//					//get all children rooted at childrenList1_Tree1[m], 	
//					V3DLONG *subTreeNodeIdx1 = 0;
//					swcTree *subTree1 = 0; 
//					Tree1->getSubTree(childrenList1_Tree1[m], depthThre, subTree1, subTreeNodeIdx1);
//					
//#ifdef DEBUG
//					printf("subTreeNodeIdx1: ");
//					
//					for (p=0; p<subTree1->treeNodeNum; p++)
//						printf("%d, ", subTreeNodeIdx1[p]);
//					printf("\n");
//#endif					
//					
//					// find the corresponding branches in the Tree1_old
//					//					V3DLONG s = 0;
//					//					while (s<childrenNum1_Tree1_old)
//					//					{
//					//						if (childrenList1_Tree1_old[s] == childrenList1_Tree1[m])
//					//							break;
//					//						else
//					//							s++;
//					//					}
//					
//					V3DLONG s = 0;
//					while (s<childrenNum1_Tree1_old)
//					{
//						V3DLONG *pathNodeList =0;
//						V3DLONG pathNodeNum;
//						unsigned char ancestorTag;
//						
//						Tree1_old->findPath(childrenList1_Tree1_old[s], childrenList1_Tree1[m], pathNodeList, pathNodeNum, ancestorTag); 
//						printf("ancestorTag = %d\n", ancestorTag);
//						if (pathNodeList) {delete pathNodeList; pathNodeList=0;}
//						
//						if (ancestorTag == 1)
//							break;
//						else
//							s++;
//					}
//					
//					
//					for (n=0; n<childrenNum2_Tree2; n++)
//					{
//						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);
//						
//						// find the best one in the subtree rooted at nodeidx2
//						V3DLONG *subTreeNodeIdx2 = 0;
//						swcTree *subTree2 = 0; 
//						//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
//						Tree2->getSubTree(childrenList2_Tree2[n], depthThre, subTree2, subTreeNodeIdx2);
//#ifdef DEBUG
//						printf("subTreeNodeIdx2: ");
//						
//						for (p=0; p<subTree2->treeNodeNum; p++)
//							printf("%d, ", subTreeNodeIdx2[p]);
//						printf("\n");
//#endif						
//						// find the best match with highest similarity in the subtree rooted at childrenList1_Tree1[m] and childreList2_Tree2[n]						
//						float bestval = INVALID_VALUE;
//						for (p=0; p<subTree1->treeNodeNum; p++)
//							for (q=0; q<subTree2->treeNodeNum; q++)
//								if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//									//							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]] + simVector[p*lengthratioNum2+q])
//								{
//									bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//									bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p];
//									bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q];
//									
//								}
//						
//						// find the corresponding branches in Tree2_old						
//						//						V3DLONG t = 0;
//						//						while (t<childrenNum2_Tree2_old)
//						//						{
//						//							if (childrenList2_Tree2_old[t] == childrenList2_Tree2[n])
//						//								break;
//						//							else
//						//								t++;
//						//						}
//						
//						V3DLONG t = 0;
//						while (t<childrenNum2_Tree2_old)
//						{
//							V3DLONG *pathNodeList =0;
//							V3DLONG pathNodeNum;
//							unsigned char ancestorTag;
//							
//							Tree2_old->findPath(childrenList2_Tree2_old[t], childrenList2_Tree2[n], pathNodeList, pathNodeNum, ancestorTag);
//							if (pathNodeList) {delete pathNodeList; pathNodeList=0;}
//							
//							if (ancestorTag == 1)
//								break;
//							else
//								t++;
//						}
//						
//						// assign value to bpMatrix(m,n)							
//						bpMatrix(m,n) = -bestval-simVector[s*lengthratioNum2+t]; // hungarian method use distance 
//						
//						if (subTree2) {delete subTree2; subTree2=0;}
//						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
//						
//					} // for n
//					
//					if (subTree1) {delete subTree1; subTree1=0;}
//					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}
//					
//				} // for m
//				
//				
//				// Hungarian algorithm for bipartite matching, find the minimum cost
//				Matrix<double> bpMatrixOut(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// bipartite matrix
//				
//				printf("printf bpMatrixOut(m,n)\n");
//				
//				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
//				{
//					for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
//					{
//						bpMatrixOut(m,n) = bpMatrix(m,n);
//						printf("%f ", bpMatrixOut(m,n));
//					}
//					printf("\n");
//				}
//				printf("\n");
//				
//				
//				Munkres branchMatch;
//				branchMatch.solve(bpMatrixOut); 
//				
//				//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				//				{
//				//					for (n=0; n<childrenNum1+childrenNum2; n++)
//				//						printf("%f ", bpMatrixOut(m,n));
//				//					printf("\n");
//				//			    }
//				//				printf("\n");
//				
//				
//				// get temporary simiarlity1d, and matchingfunc1d
//				
//				for (m=0; m<childrenNum1_Tree1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
//				{
//					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1);
//					
//					for (n=0; n<childrenNum2_Tree2; n++) 
//					{
//						//					printf("%f\n ", bpMatrix(m,n));
//						//						printf("%f\n ", bpMatrixOut(m,n));
//						
//						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);
//						
//						if (bpMatrixOut(m,n)==0) // find a match
//						{
//							// update similarity1d and mappingfunc1d						
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
//							
//							V3DLONG ind1 = bestSubMatch[m*childrenNum2_Tree2*2+n*2];
//							V3DLONG ind2 = bestSubMatch[m*childrenNum2_Tree2*2+n*2+1];
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
//							
//							//expand matching list of the children
//							for (p = 0; p<nodeNum1; p++)
//							{	
//								
//								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
//								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
//									mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//									mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
//								
//							} //for p
//						} //if (bpMatrixOut(m,n)==0)
//					} // for n
//					//					printf("\n");
//				} // for m	
//				
//				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
//				
//#ifdef DEBUG				
//				printf("The final similarity involving the similarities of the children:\n"); 
//				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//#endif			
//			} //if ((childrenNum1!=0)&&(childrenNum2!=0))
//			
//			
//#ifdef DEBUG	
//			
//			//print similarity matrix
//			printf("Similarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", similarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
//			
//#endif	
//			
//			if (lengthratio2) {delete lengthratio2; lengthratio2=0;}
//			//			if (childrenList2) {delete childrenList2; childrenList2=0;}
//			//			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			
//			if (childrenList2_Tree2) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
//			if (childrenNodeIdx2_Tree2) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
//			
//			if (childrenList2_Tree2_old) {delete childrenList2_Tree2_old; childrenList2_Tree2_old=0;}
//			if (childrenNodeIdx2_Tree2_old) {delete childrenNodeIdx2_Tree2_old; childrenNodeIdx2_Tree2_old=0;}
//			
//			//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
//		} //for (j=0; j<nodeNum2; j++)
//		
//		
//#ifdef DEBUG	
//		
//		//print similarity matrix
//		printf("Similarity matrix\n");
//		printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//		printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//		printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//		
//		for (m=0; m<nodeNum1; m++)
//		{
//			for (n=0; n<nodeNum2; n++)
//				printf("%f ", similarity1d[m*nodeNum2+n]);
//			printf("\n");
//		}
//		
//		printf("\n");
//		
//		
//#endif	
//		
//		if (lengthratio1) {delete lengthratio1; lengthratio1=0;}
//		//	if (childrenList1) {delete childrenList1; childrenList1=0;}
//		//	if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//		
//		if (childrenList1_Tree1) {delete childrenList1_Tree1; childrenList1_Tree1=0;}
//		if (childrenNodeIdx1_Tree1) {delete childrenNodeIdx1_Tree1; childrenNodeIdx1_Tree1=0;}
//		
//		if (childrenList1_Tree1_old) {delete childrenList1_Tree1_old; childrenList1_Tree1_old=0;}
//		if (childrenNodeIdx1_Tree1_old) {delete childrenNodeIdx1_Tree1_old; childrenNodeIdx1_Tree1_old=0;}
//		
//		//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
//		
//	}// for i
//	
//	
//	// reconstruct the optimal match from stored best matches
//	
//	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
//	Tree1->getRootID(rootid1, rootidx1);
//	Tree2->getRootID(rootid2, rootidx2);
//	
//	//	Tree1->getIndex(rootid1, rootidx1);
//	//	Tree2->getIndex(rootid2, rootidx2);
//	
//	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//	
//	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
//	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
//	
//	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
//		printf("Root should have only one child\n");
//	
//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//		
//		
//		if (m==rootidx1) //add root, require root must be matched to root
//			idx = rootidx2;
//		else
//			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
//		
//		V3DLONG tmpidx1;
//		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
//		
//		if (idx>=0)
//		{
//			matchingList[tmpidx1] = Tree2->node[idx].nid;		
//			//			matchingList[m] = Tree2->node[idx].nid;
//			
//		}	
//		else
//			matchingList[tmpidx1] = INVALID_VALUE;
//		//			matchingList[m] = INVALID_VALUE;
//	}
//	
//	//	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	
//	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	printf("simMeasure = %f\n", simMeasure);
//	
//	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
//	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
//	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
//	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}
//	
//	
//	//#ifdef DEBUG	
//	
//	//print similarity matrix
//	printf("Similarity matrix\n");
//	
//	for (i=0; i<nodeNum1; i++)
//	{
//		for (j=0; j<nodeNum2; j++)
//			printf("%f ", similarity1d[i*nodeNum2+j]);
//		printf("\n");
//	}
//	
//	// ---------------------------------------
//	// delete pointers
//	// ---------------------------------------
//	
//	if (subTreeNodeNum1) {delete []subTreeNodeNum1; subTreeNodeNum1=0;}
//	if (subTreeNodeNum2) {delete []subTreeNodeNum2; subTreeNodeNum2=0;}
//	
//	if (sortval1) {delete []sortval1; sortval1=0;}
//    if (sortval2) {delete []sortval2; sortval2=0;}
//	if (sortidx1) {delete []sortidx1; sortidx1=0;}
//	if (sortidx2) {delete []sortidx2; sortidx2=0;}
//	
//	if (similarity1d) {delete []similarity1d; similarity1d=0;}
//	
//	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
//	
//	//recover pointer
//	if (Tree1) {delete Tree1; Tree1=Tree1_old; Tree1_old=0;} 	
//	if (Tree2) {delete Tree2; Tree2=Tree2_old; Tree2_old=0;} 
//	
//	if (removeTag1) {delete removeTag1; removeTag1=0;}
//	if (removeTag2) {delete removeTag2; removeTag2=0;}
//	
//	
//}
//

////************************************************************************************
//// match two trees using dynamic programming
//// Different from treeMatching_noContinualNodes, the similarity metrics (length-ratio) are dependent between parent and 
//// children, i.e., which branch match to which branch when computing legnth-ratio should be consistent with the bipartite matching
//// result of children nodes
//// root, branching nodes, leaf nodes will find matches, continual nodes will not find matches
////************************************************************************************

//bool treeMatching_noContinualNodes_lengthRatio_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
//{
//	V3DLONG i, j, m, n, p,q;
//	
//	
//	// -----------------------------------------------------------------
//	// generate trees only containing root, branching points, and leaf nodes
//	// remove continual points
//	// the input tree pointers are backed up in Tree1_old, Tree2_old
//	// the trees after removing continual nodes are Tree1 and Tree2
//	// -----------------------------------------------------------------	
//	swcTree *Tree1_old =0, *Tree2_old = 0;
//	swcTree *newTree1 =0, *newTree2 = 0;
//	unsigned char *removeTag1=0, *removeTag2=0;
//	
//	Tree1_old = Tree1;
//	Tree2_old = Tree2;
//	
//	
//	Tree1->removeContinualNodes(newTree1, removeTag1);
//	//	Tree1->removeContinuaLeaflNodes(newTree1, removeTag1);
//	Tree1 = newTree1;
//	newTree1 = 0;
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//	{
//		printf("%d ", Tree1->node[i].nid);
//	}
//	printf("\n");
//	
//	Tree2->removeContinualNodes(newTree2, removeTag2);
//	//	Tree2->removeContinuaLeaflNodes(newTree2, removeTag2);
//	Tree2 = newTree2;
//	newTree2 = 0;
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//	{
//		printf("%d ", Tree2->node[i].nid);
//	}
//	printf("\n");
//	
//	
//	// -----------------------------------------------------------------
//	// get the number of nodes in the subtree rooted at each node
//	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
//	// -----------------------------------------------------------------	
//	V3DLONG *subTreeNodeNum1 = 0;
//	V3DLONG *subTreeNodeNum2 = 0;
//	
//	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
//	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 
//	
//#ifdef DEBUG
//	
//	printf("print subTreeNodeNum1 returned from Tree1->getSubTreeNodeNum(subTreeNodeNum1)\n");	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum1[i]);
//	printf("\n");
//	
//	printf("print subTreeNodeNum2 returned from Tree2->getSubTreeNodeNum(subTreeNodeNum2)\n");	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %d\n ", i, subTreeNodeNum2[i]);
//	printf("\n");
//	
//#endif
//	
//	// ------------------------------------------------------------
//	// sort subTreeNodeNum1 and subTreeNodeNum2 from small to large
//	// ------------------------------------------------------------
//	float *sortval1, *sortidx1;
//	float *sortval2, *sortidx2;
//	
//	Tree1->sortSubTreeNodeNum(subTreeNodeNum1, sortval1, sortidx1);
//	Tree2->sortSubTreeNodeNum(subTreeNodeNum2, sortval2, sortidx2);
//	
//#ifdef DEBUG
//	
//	printf("print sortval1 and sortidx1\n");
//	
//	for (i=0; i<Tree1->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval1[i+1], sortidx1[i+1]);
//	printf("\n");
//	
//	printf("print sortval2 and sortidx2\n");
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//		printf("%d, %f, %f\n ", i, sortval2[i+1], sortidx2[i+1]);
//	printf("\n");
//	
//#endif
//	
//	
//	// ---------------------------------------
//	// tree matching using dynamic programming
//	// ---------------------------------------
//	V3DLONG nodeNum1, nodeNum2;
//	nodeNum1 = Tree1->treeNodeNum;
//	nodeNum2 = Tree2->treeNodeNum;
//	
//	V3DLONG nodeidx1, nodeidx2;
//	V3DLONG depthThre = 4;
//	
//	// allocate memory for similarity1d and intialize
//	float *similarity1d = new float [nodeNum1*nodeNum2]; // #(branching nodes + root in Tree 1) *#(branching nodes + root in Tree 2)
//	if (!similarity1d)
//	{
//		printf("Fail to allocate memory for similarity1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//		{
//			////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
//			//		similarity1d[m*nodeNum2 + n] =  -1;
//			similarity1d[m*nodeNum2 + n] =  0;
//		}
//	
//	
//	// allocate memory for mappingfunc1d and intialize	
//	V3DLONG *mappingfunc1d = new V3DLONG [nodeNum1*nodeNum2*nodeNum1]; // optimal matching
//	if (!mappingfunc1d)
//	{
//		printf("Fail to allocate memory for mappingfunc1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//		for (n=0; n<nodeNum2; n++)
//			for (p=0; p<nodeNum1; p++)
//				mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
//	
//	// initialize matching list, the size is determined by the original Tree1, i.e., Tree1_old
//	matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
//	
//	for (i=0; i<Tree1_old->treeNodeNum; i++)
//		matchingList[i] = INVALID_VALUE; //need to check i
//	
//	// start matching
//	for (i=0; i<nodeNum1; i++)
//	{
//		
//		V3DLONG *childrenList1_Tree1=0, *childrenNodeIdx1_Tree1=0, childrenNum1_Tree1;
//		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1, childrenNodeIdx1_Tree1, childrenNum1_Tree1); // get direct children for the current node in Tree1
//		
//		//		V3DLONG *childrenList1_Tree1_old=0, *childrenNodeIdx1_Tree1_old=0, childrenNum1_Tree1_old;
//		//		Tree1_old->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1_old, childrenNodeIdx1_Tree1_old, childrenNum1_Tree1_old); // get direct children for the current node in Tree1
//		
//		
//#ifdef DEBUG
//		
//		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenList1[m]);
//		printf("\n");
//		
//		for (int m=0; m<childrenNum1; m++)
//			printf("%d ", childrenNodeIdx1[m]);
//		printf("\n\n");
//		
//#endif
//		
//		float *lengthratio1=0;
//		unsigned char lengthratioNum1;
//		Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4);
//		
//		// find candidate nodes in Tree2 that can be matched to the current node in tree1
//		
//		for (j=0; j<nodeNum2; j++) 
//		{
//			
//			printf("i=%d,j=%d\n", i,j);	
//			V3DLONG *childrenList2_Tree2=0, *childrenNodeIdx2_Tree2=0, childrenNum2_Tree2;
//			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2, childrenNodeIdx2_Tree2, childrenNum2_Tree2); // get direct children for the current node in Tree1
//			
//			//			V3DLONG *childrenList2_Tree2_old=0, *childrenNodeIdx2_Tree2_old=0, childrenNum2_Tree2_old;
//			//			Tree2_old->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2_old, childrenNodeIdx2_Tree2_old, childrenNum2_Tree2_old); // get direct children for the current node in Tree1
//			
//			
//			
//#ifdef DEBUG
//			
//			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenList2[m]);
//			printf("\n");
//			
//			for (m=0; m<childrenNum2; m++)
//				printf("%d ", childrenNodeIdx2[m]);
//			printf("\n\n");
//#endif			
//			
//			//			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
//			////			
//			//////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_length_weighted_distance(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			
//			//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			//			for (int p = 0; p<nodeNum1; p++)
//			//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			//				
//			//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			// compute metrics of each branch based on which similarity is further computed
//			float *lengthratio2=0;
//			unsigned char lengthratioNum2;
//			Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4);
//			
//			float *simVector = 0;
//			
//			printf("i=%d, j=%d, lengthratioNum1 = %d, lengthratioNum2=%d\n", i,j,lengthratioNum1, lengthratioNum2);
//			simVector = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
//			
//#ifdef DEBUG
//			for (m=0; m<lengthratioNum1; m++)
//			{
//				for (n=0; n<lengthratioNum2; n++)
//					printf("%f ", simVector[m*lengthratioNum2+n]);
//				printf("\n");
//			}
//			printf("\n");
//#endif			
//			
//			for (int p = 0; p<nodeNum1; p++)
//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//			
//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
//			
//			
//			// find the best matching for subtree using bipartite matching	
//			
//			if ((childrenNum1_Tree1==0)||(childrenNum2_Tree2==0)) 
//				// only compute the local similarity, i.e., the best similarity of length-ratio (no subtree nodes similarity) of the two nodes without
//				// adding the similarity of subtree nodes
//			{
//				// compute length ratio similarity
//				for (m=0; m<lengthratioNum1*lengthratioNum2; m++) // note that lengthratioNum =1 for leaf nodes
//					similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += simVector[m]; 	// update similarity1d										
//				printf("%d, %d, %f\n", i,j,similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//				
//				
//			}
//			else // none of the two nodes are leaf nodes in the pruned tree, they are either root or branching nodes
//			{
//				
//				Matrix<double> bpMatrix(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// consider branches before pruning
//				
//				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
//					for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
//						bpMatrix(m,n) = -INVALID_VALUE;
//				
//				//				for (m=0; m<lengthratioNum1+lengthratioNum2; m++)
//				//				for (n=0; n<lengthratioNum1+lengthratioNum2; n++)
//				//				{
//				//					if ((m<lengthratioNum1)&&(n<lengthratioNum2))
//				//						bpMatrix(m,n) = -INVALID_VALUE-simVector[m*lengthratioNum2+n];
//				//					else
//				//						bpMatrix(m,n) = -INVALID_VALUE;
//				//				}
//				
//				//				initialize bestSubMatch
//				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1_Tree1*childrenNum2_Tree2*2];
//				
//				for (m=0; m<childrenNum1_Tree1; m++)
//					for (n=0; n<childrenNum2_Tree2; n++)
//					{
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2] = INVALID_VALUE;
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = INVALID_VALUE;
//					}
//				
//				// assign values to bpMatrix				
//				for (m=0; m<childrenNum1_Tree1; m++)
//				{
//					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1); 
//					
//					//get all children rooted at childrenList1_Tree1[m], 	
//					V3DLONG *subTreeNodeIdx1 = 0;
//					swcTree *subTree1 = 0; 
//					Tree1->getSubTree(childrenList1_Tree1[m], depthThre, subTree1, subTreeNodeIdx1);
//					
//#ifdef DEBUG
//					printf("subTreeNodeIdx1: ");
//					
//					for (p=0; p<subTree1->treeNodeNum; p++)
//						printf("%d, ", subTreeNodeIdx1[p]);
//					printf("\n");
//#endif					
//					
//					//					// find the corresponding branches in the Tree1_old
//					////					V3DLONG s = 0;
//					////					while (s<childrenNum1_Tree1_old)
//					////					{
//					////						if (childrenList1_Tree1_old[s] == childrenList1_Tree1[m])
//					////							break;
//					////						else
//					////							s++;
//					////					}
//					//
//					//					V3DLONG s = 0;
//					//					while (s<childrenNum1_Tree1_old)
//					//					{
//					//						V3DLONG *pathNodeList =0;
//					//						V3DLONG pathNodeNum;
//					//						unsigned char ancestorTag;
//					//						
//					//						Tree1_old->findPath(childrenList1_Tree1_old[s], childrenList1_Tree1[m], pathNodeList, pathNodeNum, ancestorTag); 
//					//						printf("ancestorTag = %d\n", ancestorTag);
//					//						if (pathNodeList) {delete pathNodeList; pathNodeList=0;}
//					//						
//					//						if (ancestorTag == 1)
//					//							break;
//					//						else
//					//							s++;
//					//					}
//					
//					
//					for (n=0; n<childrenNum2_Tree2; n++)
//					{
//						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);
//						
//						// find the best one in the subtree rooted at nodeidx2
//						V3DLONG *subTreeNodeIdx2 = 0;
//						swcTree *subTree2 = 0; 
//						//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
//						Tree2->getSubTree(childrenList2_Tree2[n], depthThre, subTree2, subTreeNodeIdx2);
//#ifdef DEBUG
//						printf("subTreeNodeIdx2: ");
//						
//						for (p=0; p<subTree2->treeNodeNum; p++)
//							printf("%d, ", subTreeNodeIdx2[p]);
//						printf("\n");
//#endif						
//						// find the best match with highest similarity in the subtree rooted at childrenList1_Tree1[m] and childreList2_Tree2[n]						
//						float bestval = INVALID_VALUE;
//						V3DLONG p1, q1;
//						
//						for (p=0; p<subTree1->treeNodeNum; p++)
//							for (q=0; q<subTree2->treeNodeNum; q++)
//								if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//									//							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]] + simVector[p*lengthratioNum2+q])
//								{
//									bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//									p1 = p; q1 = q;
//									//								bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p];
//									//								bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q];
//									
//								}
//						
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p1];
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q1];
//						
//						//						// find the corresponding branches in Tree2_old						
//						////						V3DLONG t = 0;
//						////						while (t<childrenNum2_Tree2_old)
//						////						{
//						////							if (childrenList2_Tree2_old[t] == childrenList2_Tree2[n])
//						////								break;
//						////							else
//						////								t++;
//						////						}
//						//
//						//					V3DLONG t = 0;
//						//					while (t<childrenNum2_Tree2_old)
//						//					{
//						//						V3DLONG *pathNodeList =0;
//						//						V3DLONG pathNodeNum;
//						//						unsigned char ancestorTag;
//						//						
//						//						Tree2_old->findPath(childrenList2_Tree2_old[t], childrenList2_Tree2[n], pathNodeList, pathNodeNum, ancestorTag);
//						//						if (pathNodeList) {delete pathNodeList; pathNodeList=0;}
//						//						
//						//						if (ancestorTag == 1)
//						//							break;
//						//						else
//						//							t++;
//						//					}
//						
//						// assign value to bpMatrix(m,n)	
//						//						bpMatrix(m,n) = -bestval-simVector[s*lengthratioNum2+t]; // hungarian method use distance 						
//						bpMatrix(m,n) = -bestval-simVector[m*lengthratioNum2+n]; // hungarian method use distance 
//						
//						
//						if (subTree2) {delete subTree2; subTree2=0;}
//						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
//						
//					} // for n
//					
//					if (subTree1) {delete subTree1; subTree1=0;}
//					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}
//					
//				} // for m
//				
//				
//				// Hungarian algorithm for bipartite matching, find the minimum cost
//				Matrix<double> bpMatrixOut(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// bipartite matrix
//				
//				printf("printf bpMatrixOut(m,n)\n");
//				
//				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
//				{
//					for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
//					{
//						bpMatrixOut(m,n) = bpMatrix(m,n);
//						printf("%f ", bpMatrixOut(m,n));
//					}
//					printf("\n");
//				}
//				printf("\n");
//				
//				
//				Munkres branchMatch;
//				branchMatch.solve(bpMatrixOut); 
//				
//				//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				//				{
//				//					for (n=0; n<childrenNum1+childrenNum2; n++)
//				//						printf("%f ", bpMatrixOut(m,n));
//				//					printf("\n");
//				//			    }
//				//				printf("\n");
//				
//				
//				// get temporary simiarlity1d, and matchingfunc1d
//				
//				for (m=0; m<childrenNum1_Tree1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
//				{
//					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1);
//					
//					for (n=0; n<childrenNum2_Tree2; n++) 
//					{
//						//					printf("%f\n ", bpMatrix(m,n));
//						//						printf("%f\n ", bpMatrixOut(m,n));
//						
//						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);
//						
//						if (bpMatrixOut(m,n)==0) // find a match
//						{
//							// update similarity1d and mappingfunc1d						
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
//							
//							V3DLONG ind1 = bestSubMatch[m*childrenNum2_Tree2*2+n*2];
//							V3DLONG ind2 = bestSubMatch[m*childrenNum2_Tree2*2+n*2+1];
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
//							
//							//expand matching list of the children
//							for (p = 0; p<nodeNum1; p++)
//							{	
//								
//								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
//								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
//									mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//									mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
//								
//							} //for p
//						} //if (bpMatrixOut(m,n)==0)
//					} // for n
//					//					printf("\n");
//				} // for m	
//				
//				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
//				
//#ifdef DEBUG				
//				printf("The final similarity involving the similarities of the children:\n"); 
//				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//#endif			
//			} //if ((childrenNum1!=0)&&(childrenNum2!=0))
//			
//			
//#ifdef DEBUG	
//			
//			//print similarity matrix
//			printf("Similarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", similarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
//			
//#endif	
//			
//			if (lengthratio2) {delete lengthratio2; lengthratio2=0;}
//			//			if (childrenList2) {delete childrenList2; childrenList2=0;}
//			//			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			
//			if (childrenList2_Tree2) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
//			if (childrenNodeIdx2_Tree2) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
//			
//			//			if (childrenList2_Tree2_old) {delete childrenList2_Tree2_old; childrenList2_Tree2_old=0;}
//			//			if (childrenNodeIdx2_Tree2_old) {delete childrenNodeIdx2_Tree2_old; childrenNodeIdx2_Tree2_old=0;}
//			
//			//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
//		} //for (j=0; j<nodeNum2; j++)
//		
//		
//#ifdef DEBUG	
//		
//		//print similarity matrix
//		printf("Similarity matrix\n");
//		printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//		printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//		printf("current similarity %f = \n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//		
//		for (m=0; m<nodeNum1; m++)
//		{
//			for (n=0; n<nodeNum2; n++)
//				printf("%f ", similarity1d[m*nodeNum2+n]);
//			printf("\n");
//		}
//		
//		printf("\n");
//		
//		
//#endif	
//		
//		if (lengthratio1) {delete lengthratio1; lengthratio1=0;}
//		//	if (childrenList1) {delete childrenList1; childrenList1=0;}
//		//	if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//		
//		if (childrenList1_Tree1) {delete childrenList1_Tree1; childrenList1_Tree1=0;}
//		if (childrenNodeIdx1_Tree1) {delete childrenNodeIdx1_Tree1; childrenNodeIdx1_Tree1=0;}
//		
//		//	if (childrenList1_Tree1_old) {delete childrenList1_Tree1_old; childrenList1_Tree1_old=0;}
//		//	if (childrenNodeIdx1_Tree1_old) {delete childrenNodeIdx1_Tree1_old; childrenNodeIdx1_Tree1_old=0;}
//		
//		//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
//		
//	}// for i
//	
//	
//	// reconstruct the optimal match from stored best matches
//	
//	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
//	Tree1->getRootID(rootid1, rootidx1);
//	Tree2->getRootID(rootid2, rootidx2);
//	
//	//	Tree1->getIndex(rootid1, rootidx1);
//	//	Tree2->getIndex(rootid2, rootidx2);
//	
//	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//	
//	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
//	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
//	
//	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
//		printf("Root should have only one child\n");
//	
//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//		
//		
//		if (m==rootidx1) //add root, require root must be matched to root
//			idx = rootidx2;
//		else
//			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
//		
//		V3DLONG tmpidx1;
//		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
//		
//		if (idx>=0)
//		{
//			matchingList[tmpidx1] = Tree2->node[idx].nid;		
//			//			matchingList[m] = Tree2->node[idx].nid;
//			
//		}	
//		else
//			matchingList[tmpidx1] = INVALID_VALUE;
//		//			matchingList[m] = INVALID_VALUE;
//	}
//	
//	//	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	
//	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	printf("simMeasure = %f\n", simMeasure);
//	
//	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
//	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
//	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
//	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}
//	
//	
//	//#ifdef DEBUG	
//	
//	//print similarity matrix
//	printf("Similarity matrix\n");
//	
//	for (i=0; i<nodeNum1; i++)
//	{
//		for (j=0; j<nodeNum2; j++)
//			printf("%f ", similarity1d[i*nodeNum2+j]);
//		printf("\n");
//	}
//	
//	// ---------------------------------------
//	// delete pointers
//	// ---------------------------------------
//	
//	if (subTreeNodeNum1) {delete []subTreeNodeNum1; subTreeNodeNum1=0;}
//	if (subTreeNodeNum2) {delete []subTreeNodeNum2; subTreeNodeNum2=0;}
//	
//	if (sortval1) {delete []sortval1; sortval1=0;}
//    if (sortval2) {delete []sortval2; sortval2=0;}
//	if (sortidx1) {delete []sortidx1; sortidx1=0;}
//	if (sortidx2) {delete []sortidx2; sortidx2=0;}
//	
//	if (similarity1d) {delete []similarity1d; similarity1d=0;}
//	
//	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
//	
//	//recover pointer
//	if (Tree1) {delete Tree1; Tree1=Tree1_old; Tree1_old=0;} 	
//	if (Tree2) {delete Tree2; Tree2=Tree2_old; Tree2_old=0;} 
//	
//	if (removeTag1) {delete removeTag1; removeTag1=0;}
//	if (removeTag2) {delete removeTag2; removeTag2=0;}
//	
//	
//}

//
