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
#include <algorithm>

#include "FL_treeMatching.h"
#include "mymatrix.cpp"
#include "munkres.cpp"
#include "./combination/combination.h" 
using namespace std;
using namespace stdcomb;

// generate tree matching result for Graphviz
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


// function to calculate similarity between two nodes, one in Tree1 and the other in Tree2, in terms of Eucldiean distance
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


// compute the branching length ratio of two nodes in two trees and compute the distance 
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

// override function calsim_branch_length_ratio above
// compute the similarity vector of branching length ratio of two nodes in two trees 
float *calsim_allbranch_length_ratio(swcTree *Tree1, swcTree *Tree2, V3DLONG nodeid1, V3DLONG nodeid2)
{
	V3DLONG i,j;
	
	float *lengthratio1 =0, *lengthratio2 = 0;
	unsigned char lengthratioNum1, lengthratioNum2;
	float thre = 10;
	
	// compute branch length ratio
	
	Tree1->computeBranchLengthRatio(nodeid1, lengthratio1, lengthratioNum1, 4);
	Tree2->computeBranchLengthRatio(nodeid2, lengthratio2, lengthratioNum2, 4);
	
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

// override function calsim_branch_length_ratio above
// compute the similarity vector of branching length ratio of two nodes in two trees 
float *calsim_allbranch_length_ratio(float *lengthratio1, float *lengthratio2, unsigned char lengthratioNum1, unsigned char lengthratioNum2)
{
	V3DLONG i,j;
	float thre = 1;
	
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

// compute the similarity vector of branching angles of two nodes in two trees
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
// compute the similatiry between nodeid1 and nodeid2, in terms of the length-weighted-Euclidean distance 
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

//match significant branching points in two trees, not debugged yet
void sigBranchingPointMatching(swcTree *Tree1, swcTree *Tree2, V3DLONG branchSizeThre, V3DLONG *&matchingList, V3DLONG &sigBranchPointNum, float &simMeasure)
{
	
	V3DLONG i,j, m, n;
	
	// find significant branching points in two trees
	V3DLONG *branchingNodeList1 =0, *branchingNodeIdx1 = 0, branchingNodeNum1;
	V3DLONG *branchingNodeList2 =0, *branchingNodeIdx2 = 0, branchingNodeNum2;
	
//	//get significant branching points of Tree1 and Tree2
//	Tree1->getSignificantBranchingNode(branchSizeThre, branchingNodeList1, branchingNodeIdx1, branchingNodeNum1); 
//	Tree2->getSignificantBranchingNode(branchSizeThre, branchingNodeList2, branchingNodeIdx2, branchingNodeNum2); 	
//
//   //for debug purpose
//	FILE *file;	
//	file = fopen("sigPoints1", "wt");
//	if (file == NULL)
//	{
//		printf("error to open file\n");
//	}
//		
//	for (int i=0; i<branchingNodeNum1; i++)
//	{
////		fprintf(file, "%d\n", branchingNodeIdx1[i]);
//		fprintf(file, "%d\n", branchingNodeList1[i]);
//
//	}
//	fclose(file);
//	
//	
//	file = fopen("sigPoints2", "wt");
//	if (file == NULL)
//	{
//		printf("error to open file\n");
//	}
//		
//	for (int i=0; i<branchingNodeNum2; i++)
//	{
////		fprintf(file, "%d\n", branchingNodeIdx2[i]);
//		fprintf(file, "%d\n", branchingNodeList2[i]);
//
//	}
//	fclose(file);

	//get significant branching points of Tree1 and Tree2
	Tree1->getSignificantBranchingNode(branchSizeThre, branchingNodeList1, branchingNodeIdx1, branchingNodeNum1); 

	FILE *file;	
	file = fopen("sigPoints1", "wt");
	if (file == NULL)
	{
		printf("error to open file\n");
	}
		
	for (int i=0; i<branchingNodeNum1; i++)
	{
		fprintf(file, "%d\n", branchingNodeList1[i]);

	}
	

//	Tree1->swcTree::getBranchingNode(branchingNodeList1, branchingNodeIdx1, branchingNodeNum1); //find all branching points
		
	//find all leaf nodes
	V3DLONG *leafNodeList1 =0, *leafNodeIdx1=0, leafNodeNum1;
	Tree1->swcTree::getLeafNode(leafNodeList1, leafNodeIdx1, leafNodeNum1);

	
	//for debug purpose
	for (int i=0; i<leafNodeNum1; i++)
	{
		fprintf(file, "%d\n", leafNodeList1[i]);
	}

	if (leafNodeList1) {delete []leafNodeList1; leafNodeList1=0;}
	if (leafNodeIdx1) {delete []leafNodeList1; leafNodeList1=0;}
	
	V3DLONG rootid, rootidx;
	Tree1->getRootID(rootid, rootidx);
	fprintf(file, "%d\n", rootid);
	
	fclose(file);


	Tree2->getSignificantBranchingNode(branchSizeThre, branchingNodeList2, branchingNodeIdx2, branchingNodeNum2); 	
	
	file = fopen("sigPoints2", "wt");
	if (file == NULL)
	{
		printf("error to open file\n");
	}
		
	for (int i=0; i<branchingNodeNum2; i++)
	{
		fprintf(file, "%d\n", branchingNodeList2[i]);

	}

//	Tree2->swcTree::getBranchingNode(branchingNodeList2, branchingNodeIdx2, branchingNodeNum2); 
	
	//find all leaf nodes
	V3DLONG *leafNodeList2 =0, *leafNodeIdx2=0, leafNodeNum2;	
	Tree2->swcTree::getLeafNode(leafNodeList2, leafNodeIdx2, leafNodeNum2);
	
	//for debug purpose
	for (int i=0; i<leafNodeNum2; i++)
	{
		fprintf(file, "%d\n", leafNodeList2[i]);
	}

	if (leafNodeList2) {delete []leafNodeList2; leafNodeList2=0;}
	if (leafNodeIdx2) {delete []leafNodeIdx2; leafNodeIdx2=0;}

	Tree2->getRootID(rootid, rootidx);
	fprintf(file, "%d\n", rootid);

	fclose(file);
//	
	
	sigBranchPointNum = branchingNodeNum1;		

	//initialize
	V3DLONG wid1 = (MAX_CHILDREN_NUM+1);// +1 because add parent node
	V3DLONG len1 = branchingNodeNum1*wid1;
	V3DLONG len2 = branchingNodeNum2*wid1;
	
	V3DLONG *branchSize1 = new V3DLONG [len1];
	V3DLONG *branchSize2 = new V3DLONG [len2];
	V3DLONG *branchNum1 = new V3DLONG [branchingNodeNum1]; // number of branches for each signficant branching points (including parent branch)
	V3DLONG *branchNum2 = new V3DLONG [branchingNodeNum2];
	
	V3DLONG wid2 = (MAX_CHILDREN_NUM+1)*MAX_CHILDREN_NUM/2;
	V3DLONG len3 = branchingNodeNum1*wid2; //pair-wise angles
	V3DLONG len4 = branchingNodeNum2*wid2; //pair-wise angles
	
	float *branchAngle1 = new float [len3];
	float *branchAngle2 = new float [len4];
	
	
	for (i=0; i<len1; i++)
		branchSize1[i] = INVALID_VALUE; //INVALID_VALUE defined in FL_swcTree.h

	for (i=0; i<len3; i++)
		branchAngle1[i] = INVALID_VALUE; //INVALID_VALUE defined in FL_swcTree.h

	for (i=0; i<len2; i++)
		branchSize2[i] = INVALID_VALUE; //INVALID_VALUE defined in FL_swcTree.h

	for (i=0; i<len4; i++)
		branchAngle2[i] = INVALID_VALUE; //INVALID_VALUE defined in FL_swcTree.h

	// compute subbranch sizes (number of nodes) and angles features of each significant branching points in tree1 		
	for (i=0; i<branchingNodeNum1; i++)
	{

		// compute subbranch sizes and angles
		V3DLONG *branchRootID=0, *branchRootIdx=0, branchNum, *branchNodeNum = 0;
		float *angles =0;
		
		Tree1->computeBranchNodeNumAngles(branchingNodeList1[i], branchRootID, branchRootIdx, branchNum, branchNodeNum, angles);
		
		for (j=0; j<branchNum; j++)
		{
			branchSize1[i*wid1+j+1] = branchNodeNum[j];	// the first element is saved for parent	
			branchAngle1[i*wid2+j+1] = angles[j];	
		}
		branchNum1[i] = branchNum;
		
		
		if (branchRootID) {delete []branchRootID; branchRootID=0;}
		if (branchRootIdx) {delete []branchRootIdx; branchRootIdx=0;}
		if (branchNodeNum) {delete []branchNodeNum; branchNodeNum=0;}
		if (angles) {delete []angles; angles=0;} 
		
	};
		
	// compute subbranch sizes and angles features of each significant branching points in tree2 		
	for (i=0; i<branchingNodeNum2; i++)
	{

		// compute subbranch sizes and angles
		V3DLONG *branchRootID=0, *branchRootIdx=0, branchNum, *branchNodeNum = 0;
		float *angles =0;
		Tree2->computeBranchNodeNumAngles(branchingNodeList2[i], branchRootID, branchRootIdx, branchNum, branchNodeNum, angles);
		
		for (j=0; j<branchNum; j++)
		{
			branchSize2[i*wid1+j+1] = branchNodeNum[j];		
			branchAngle2[i*wid2+j+1] = angles[j];
		}
		branchNum2[i] = branchNum;
		
		if (branchRootID) {delete []branchRootID; branchRootID=0;}
		if (branchRootIdx) {delete []branchRootIdx; branchRootIdx=0;}
		if (branchNodeNum) {delete []branchNodeNum; branchNodeNum=0;}
		if (angles) {delete []angles; angles=0;} 
		
	};

	
	// compute similarity matrix of signficant braning points in two trees
	
	float thre = 2;
	float *simAll = new float [branchingNodeNum1*branchingNodeNum2]; //similarity matrix between significant branching points in Tree1 and Tree2
	
	for (i=0; i<branchingNodeNum1; i++)
	for (j=0; j<branchingNodeNum2; j++)
	{
		
		simAll[i*branchingNodeNum2 +j] = 0;
		
		if ((branchNum1[i]<2) || (branchNum2[j]<2)) // should never be satisfied, otherwise, should not be significant branching points
		{
			simAll[i*branchingNodeNum2+j] = -1;
		}
		else
		{
		
			float *sim = new float [(branchNum1[i]-1)*(branchNum2[j]-1)]; //similarity matrix between branches of two considered branching points, used to determine which branch in one point match to which in the other
		
			// compute size similarity of each pairing branch
			for (m=1; m<branchNum1[i]; m++) // do not test on parent path, assuming the root is correct, otherwise, need to align root first
			for (n=1; n<branchNum2[j]; n++) // do not test on parent path, assuming the root is correct, otherwise, need to align root first
			{
				float r, r1;
				V3DLONG idx;
				
				r1 = (float)branchSize1[i*wid1+m] / (float)branchSize2[j*wid1+n];
				r = (r1>(1.0/r1))? r1:1.0/r1; // take bigger one
				idx = (m-1)*(branchNum2[j]-1)+ (n-1);
				if (r>thre)
					sim[idx] = -1;
	//				sim[m-1][n-1];
				else
					sim[idx]= -2*r/(thre-1) + (thre+1)/(thre-1);
	//				sim([m-1][n-1]= -2*r/(thre-1) + (thre+1)/(thre-1);

			}
			
			// bipartite matching find the best match
			Matrix<double> dismatrix(branchNum1[i]-1, branchNum2[j]-1);
			for (m=0; m<branchNum1[i]-1; m++) // do not test on parent path, assuming the root is correct, otherwise, need to align root first
			for (n=0; n<branchNum2[j]-1; n++) // do not test on parent path, assuming the root is correct, otherwise, need to align root first
				dismatrix(m, n) = 1-sim[m*(branchNum2[j]-1)+n]; //range is 0~2
			
			Munkres branchMatch;
			branchMatch.solve(dismatrix); 
			
			V3DLONG *branchMatchList = new V3DLONG [branchNum1[i]-1];
			unsigned char matchedBranchNum=0;
			
			for (m=0; m<branchNum1[i]-1; m++) // do not test on parent path, assuming the root is correct, otherwise, need to align root first
			{
				branchMatchList[m] = INVALID_VALUE;
				
				for (n=0; n<branchNum2[j]-1; n++) // do not test on parent path, assuming the root is correct, otherwise, need to align root first
				{
					printf("%f ", dismatrix(m,n));
					
					if (dismatrix(m,n)==0) // find a match
					{
						branchMatchList[m] = n;
						simAll[i*branchingNodeNum2 +j] += sim[m*(branchNum2[j]-1)+n]; 
						matchedBranchNum++;
						break;
					}
				}
				printf("\n");
				
			}
						
			// compute angle similarity
			float simAngle = 0;
			int *cnt2 = new int [(branchNum2[i]-1)*(branchNum2[i]-1)]; // look up table of index
			int cnt = 0;
			
			for (m=0; m<branchNum2[j]-2; m++)
			for (n=m+1; n<branchNum2[j]-1; n++)
			{
				cnt2[m*(branchNum2[j]-1)+n] = cnt;
				cnt2[n*(branchNum2[j]-1)+m] = cnt; // make it symetrical				
				cnt++;
			}

			
			int cntidx1 = -1;
			int cntidx2;
			
			for (m=0; m<branchNum1[i]-2; m++)
			for (n=m+1; n<branchNum1[i]-1; n++)
			{
				cntidx1++;

				if ((branchMatchList[m]!=INVALID_VALUE) && (branchMatchList[n]!=INVALID_VALUE)) // the m and n branch of the node in tree1 both find match, unless angle is meaningless
				{
					
					// 
					cntidx2 = cnt2[branchMatchList[m]*(branchNum2[j]-1)+branchMatchList[n]];
					
					float r, r1;
					V3DLONG idx;
					
					r1 = branchAngle1[i*wid2+branchNum1[i]+cntidx1] /branchAngle2[j*wid2+branchNum2[j]+cntidx2]; // the first branchNum1 elemnts in branchAngle1 of a tree1 node relate to parent
					
					r = (r1>(1.0/r1))? r1:1.0/r1; // take bigger one
//					idx = (m-1)*(<branchNum2[j]-1)+ (n-1);
					
					if (r>thre)
						simAngle += -1;
					else
						simAngle += -2*r/(thre-1) + (thre+1)/(thre-1);
						
				}
				
			}
			simAngle /=  matchedBranchNum*(matchedBranchNum-1)/2;
			
			simAll[i*branchingNodeNum2 +j] += simAngle;
			
			if (cnt2) {delete []cnt2; cnt2=0;}
			if (branchMatchList) {delete []branchMatchList; branchMatchList=0;}
			if (sim) {delete []sim; sim=0;}
		}//if ((branchNum1[i]<2) || (branchNum2[j]<2))
	} // for i,j
	
	// bipartite matching based on the similarity matrix, get the matching list and score
	
	Matrix<double> dismatrix(branchingNodeNum1, branchingNodeNum2);
	
	for (i=0; i<branchingNodeNum1; i++)
	for (j=0; j<branchingNodeNum2; j++)
		dismatrix(m, n) = 1-simAll[i*branchingNodeNum2 +j]; //range is 0~2
	
	Munkres match;
	match.solve(dismatrix); 
	
	matchingList = new V3DLONG [branchingNodeNum1*2];
	
	for (i=0; i<branchingNodeNum1; i++)
	{
		matchingList[i*2] = branchingNodeList1[i];
		matchingList[i*2+1] = INVALID_VALUE;
		
		for (j=0; j<branchingNodeNum2; j++)
		{
			if (dismatrix(i,j)==0)
			{
				matchingList[i*2+1] = branchingNodeList2[j];
				
				simMeasure += simAll[i*branchingNodeNum2 +j]; 
				break;
			}
			
		}
	}
				
	
	// delete pointers
	if (simAll) {delete []simAll; simAll=0;}
	if (branchingNodeList1) {delete []branchingNodeList1; branchingNodeList1=0;}
	if (branchingNodeList2) {delete []branchingNodeList2; branchingNodeList2=0;}
	if (branchingNodeIdx1) {delete []branchingNodeIdx1; branchingNodeIdx1=0;}
	if (branchingNodeIdx2) {delete []branchingNodeIdx2; branchingNodeIdx2=0;}
	
	if (branchSize1) {delete []branchSize1; branchSize1=0;}
	if (branchSize2) {delete []branchSize2; branchSize2=0;}
	if (branchAngle1) {delete []branchAngle1; branchAngle1=0;}
	if (branchAngle2) {delete []branchAngle2; branchAngle2=0;}
	if (branchNum1) {delete []branchNum1; branchNum1=0;}
	if (branchNum2) {delete []branchNum2; branchNum2=0;}
	
}

//register (global affine) subjectTree against targetTree using control points
// controlNodeIDs: P pairs of corresponding points, with P*2 elements, the first element 
// of each pair is the nodeid serving as the control point in the subjectTree, 
// and the second element is the nodeid severing as the control point of the targetTree
// num_cpts is the number of control point pairs
// The result override subjectTree

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


// tree matching using dynamic programming
// the very early version that works, nodeNumThre=0 for the same neurons, nodeNumThre=2 for different neurons
bool treeMatching(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
{
	V3DLONG i, j, m, n, p,q;
	V3DLONG nodeNum1, nodeNum2;
	nodeNum1 = Tree1->treeNodeNum;
	nodeNum2 = Tree2->treeNodeNum;
	float nodeNumThre = 2; // nodes with subtree node number no less than this value will be considered in match, 2 for matching neurons by structures, 0 for matching neurons by geometrical features
//	float nodeNumThre = 0; // nodes with subtree node number no less than this value will be considered in match, 2 for matching neurons by structures, 0 for matching neurons by geometrical features

	// -----------------------------------------------------------------
	// get the number of nodes in the subtree rooted at each node
	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
	// -----------------------------------------------------------------	
	V3DLONG *subTreeNodeNum1 = 0;
	V3DLONG *subTreeNodeNum2 = 0;
	
	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 

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

	V3DLONG *childrenList1=0, *childrenNodeIdx1=0;
	V3DLONG *childrenList2 =0, *childrenNodeIdx2 = 0;	
	V3DLONG childrenNum1, childrenNum2;
	V3DLONG nodeidx1, nodeidx2;
	V3DLONG depthThre = 4;

	// allocate memory for similarity1d and intialize
	float *similarity1d = new float [nodeNum1*nodeNum2]; // optimal similarity
	if (!similarity1d)
	{
		printf("Fail to allocate memory for similarity1d. \n");
		return false;
	}
	
	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	{
////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
		similarity1d[m*nodeNum2 + n] =  -1;
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
	
	// initialize matching list
	 matchingList = new V3DLONG [nodeNum1]; //indicating for each node in tree1, the id# of the matching point in tree2
	
	for (i=0; i<nodeNum1; i++)
		matchingList[i] = -1; //need to check i
	
	// start matching
	for (i=0; i<nodeNum1; i++)
	{
	
		
		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1, childrenNodeIdx1, childrenNum1); // get direct children for the current node in Tree1

		if (childrenNum1 < nodeNumThre)
		{
			for (m=0; m<nodeNum2; m++)
				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[m+1]] = 1;		
			continue;
		}
		
//		// construct super node set
//		V3DLONG *superNodeIdx1 = new V3DLONG [choldrenNum1];
//		superNodeIdx1[0] = sortidx1[i+1]];
//		for (int tt = 0; tt< childrenNum1; tt++)
//			superNodeIdx1[tt+1] = childrenNodeIdx1[tt];
//		

#ifdef DEBUG

		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenList1[m]);
		printf("\n");
		
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenNodeIdx1[m]);
		printf("\n\n");
		
#endif
		// find candidate nodes in Tree2 that can be matched to the current node in tree1
		
		for (j=0; j<nodeNum2; j++) 
		{

			
			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for a node in Tree2
//			Tree2->getAllChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); //get all children for the current node in Tree2

			if (childrenNum2 < nodeNumThre)
			{
				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 1;
				continue;
			}
//			// construct super node set
//			V3DLONG *superNodeIdx2 = new V3DLONG [choldrenNum2];
//			superNodeIdx2[0] = sortidx2[i+1]];
//			for (int tt = 0; tt< childrenNum2; tt++)
//				superNodeIdx2[tt+1] = childrenNodeIdx2[tt];
			
#ifdef DEBUG

			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenList2[m]);
			printf("\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenNodeIdx2[m]);
			printf("\n\n");
#endif			
			
			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
			if (nodeNumThre==0) // match neurons with the same underlying strucutre
			{
				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
			}
			
//			printf("%d, %d, %d, %d\n", i,j, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
			
			if (nodeNumThre==2) // match neurons with different underlying strucutre
			{
				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);

			}
//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
			
//			// compute the dissimilarity between the two nodes, without considering their subtree similarity yet
//			dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 
//				caldist(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
	
			for (int p = 0; p<nodeNum1; p++)
				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
				
			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];

//#ifdef DEBUG	
//			printf("The initial similarity:\n");
//			printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1dtmp);
//
//#endif			
			
			// find the best matching for subtree using bipartite matching			
			if ((childrenNum1!=0)&&(childrenNum2!=0)) // if one of the current nodes in tree1 and tree2 are leaf nodes, do not do the following
			{
				
				Matrix<double> bpMatrix(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix, need to free?
				
				for (m=0; m<childrenNum1+childrenNum2; m++)
				for (n=0; n<childrenNum1+childrenNum2; n++)
//						bpMatrix(m,n) = 9999;
						bpMatrix(m,n) = -INVALID_VALUE;
						
				// build the matrix
				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1*childrenNum2*2];

				for (m=0; m<childrenNum1; m++)
				for (n=0; n<childrenNum2; n++)
				{
					bestSubMatch[m*childrenNum2*2+n*2] = INVALID_VALUE;
					bestSubMatch[m*childrenNum2*2+n*2+1] = INVALID_VALUE;
				}
				
				for (m=0; m<childrenNum1; m++)
				{
					Tree1->getIndex(childrenList1[m], nodeidx1);

					//get all children rooted at childrenList1[m], 
	
					V3DLONG *subTreeNodeIdx1 = 0;
					swcTree *subTree1 = 0; 
//					Tree1->getSubTree(childrenList1[m], subTree1, subTreeNodeIdx1); // include root of the subtree, i.e., nodeid
					Tree1->getSubTree(childrenList1[m], depthThre, subTree1, subTreeNodeIdx1);

#ifdef DEBUG
					printf("subTreeNodeIdx1: ");
					
					for (p=0; p<subTree1->treeNodeNum; p++)
						printf("%d, ", subTreeNodeIdx1[p]);
					printf("\n");
#endif					
					for (n=0; n<childrenNum2; n++)
					{
						Tree2->getIndex(childrenList2[n], nodeidx2);
						
					    // find the best one in the subtree rooted at nodeidx2
						V3DLONG *subTreeNodeIdx2 = 0;
						swcTree *subTree2 = 0; 
//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
						Tree2->getSubTree(childrenList2[n], depthThre, subTree2, subTreeNodeIdx2);
#ifdef DEBUG
						printf("subTreeNodeIdx2: ");
						
						for (p=0; p<subTree2->treeNodeNum; p++)
							printf("%d, ", subTreeNodeIdx2[p]);
						printf("\n");
#endif						
						// find the best match with highest similarity in the subtree rooted at childrenList1[m] and childreList2[m]
						
						float bestval = INVALID_VALUE;
						for (p=0; p<subTree1->treeNodeNum; p++)
						for (q=0; q<subTree2->treeNodeNum; q++)
							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
							{
								bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
								bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
								bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
								
							}
//						bpMatrix(m,n) = 9999 - bestval; // hungarian method use distance 
						bpMatrix(m,n) = -bestval; // hungarian method use distance 

//						// find the best match with lowest dissimilarity in the subtree rooted at childrenList1[m] and childreList2[m]
//	
//						float bestval = -INVALID_VALUE;
//						
//						for (p=0; p<subTree1->treeNodeNum; p++)
//						for (q=0; q<subTree2->treeNodeNum; q++)
//							if (bestval>dissimilarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//							{
//								bestval = dissimilarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//								bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
//								bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
//								
//							}
//								
//						bpMatrix(m,n) = bestval; // hungarian method use distance 
						
						if (subTree2) {delete subTree2; subTree2=0;}
						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
								
					} // for n
					
					if (subTree1) {delete subTree1; subTree1=0;}
					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}

				} // for m
								
				// Hungarian algorithm for bipartite matching, find the minimum cost
				Matrix<double> bpMatrixOut(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix

				for (m=0; m<childrenNum1+childrenNum2; m++)
				for (n=0; n<childrenNum1+childrenNum2; n++)
					bpMatrixOut(m,n) = bpMatrix(m,n);
					
				Munkres branchMatch;
				branchMatch.solve(bpMatrixOut); 

//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				{
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						printf("%f ", bpMatrixOut(m,n));
//					printf("\n");
//			    }
//				printf("\n");

				
				// get temporary simiarlity1d, and matchingfunc1d
				
				for (m=0; m<childrenNum1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
				{
					Tree1->getIndex(childrenList1[m], nodeidx1);
				
					for (n=0; n<childrenNum2; n++) 
					{
	//					printf("%f\n ", bpMatrix(m,n));
//						printf("%f\n ", bpMatrixOut(m,n));
					
						Tree2->getIndex(childrenList2[n], nodeidx2);

						if (bpMatrixOut(m,n)==0) // find a match
						{
							// update similarity1d and mappingfunc1d						
// //							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += similarity1d[nodeidx1*nodeNum2+nodeidx2];
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += (9999-bpMatrix(m,n));

//							printf("%f ", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);

							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
							
//							// update dissimilarity1d and mappingfunc1d						
//							dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += bpMatrix(m,n);

							V3DLONG ind1 = bestSubMatch[m*childrenNum2*2+n*2];
							V3DLONG ind2 = bestSubMatch[m*childrenNum2*2+n*2+1];
							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
								
							//expand matching list of the children
							for (p = 0; p<nodeNum1; p++)
							{	
								
								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
										mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
										mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
								
							} //for p
						} //if (bpMatrixOut(m,n)==0)
					} // for n
//					printf("\n");
				} // for m	
				
//				// normalize similarity1d, make it irrelavant to the number of matching
//				V3DLONG matchcnt = 0;
//				for (p = 0; p<nodeNum1; p++)
//					if (mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]!=INVALID_VALUE)
//						matchcnt++;
//						
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] /= matchcnt;	
				
						
//				if (bestnodeid) {delete []bestnodeid; bestnodeid = 0;}

				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
				
#ifdef DEBUG				
				printf("The final similarity involving the similarities of the children:\n"); 
				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
#endif			
			} //if ((childrenNum1!=0)&&(childrenNum2!=0))


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

//			//print dissimilarity matrix
//			printf("Dissimilarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current dissimilarity %f = \n", dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", dissimilarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
			
#endif	

							
			if (childrenList2) {delete childrenList2; childrenList2=0;}
			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
		} //for (j=0; j<nodeNum2; j++)
		

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


//			//print dissimilarity matrix
//			printf("dissimilarity matrix\n");
//			printf("sortidx1 = %f, sortidx2 = %f\n", sortidx1[i+1],sortidx2[j+1]);
//			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("current dissimilarity %f = \n", dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//			
//			for (m=0; m<nodeNum1; m++)
//			{
//				for (n=0; n<nodeNum2; n++)
//					printf("%f ", dissimilarity1d[m*nodeNum2+n]);
//				printf("\n");
//			}
//			
//			printf("\n");
			
#endif	

	if (childrenList1) {delete childrenList1; childrenList1=0;}
	if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
		
	}// for i
	
	
	// reconstruct the optimal match from stored best matches
	
	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);

	// temporary
//	rootid1 = 209;	rootid2 = 64;
//	rootid1 = 176;	rootid2 = 57;
//	rootid1 = 131;	rootid2 = 50;
//	rootid1 = 134;	rootid2 = 50;
	
	Tree1->getIndex(rootid1, rootidx1);
	Tree2->getIndex(rootid2, rootidx2);
	
	if (nodeNumThre==0)
		simMeasure = similarity1d[rootidx1*nodeNum2+rootidx2];
		
	if (nodeNumThre ==2)
		simMeasure = similarity1d[Tree1->node[rootidx1].nid*nodeNum2+Tree2->node[rootidx2].nid];
	
	printf("simMeasure = %f\n", simMeasure);
	
	for (m=0; m<nodeNum1; m++)
	{
		V3DLONG idx;
		if (nodeNumThre == 0)
			idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
	
		if (nodeNumThre == 2)
		{   
			V3DLONG tmpidx1, tmpidx2;
			V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
			V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
			
			if (m==rootidx1) //add root
				idx = rootidx2;
			else
			{
				Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
				Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
				
				if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
					printf("Root should have only one child\n");
				
	//			Tree1->getIndex(Tree1->node[rootidx1].nid, tmpidx1);
	//			Tree2->getIndex(Tree2->node[rootidx2].nid, tmpidx2);
				
				idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
			}
		}
		
		if (idx>=0)
		{
			matchingList[m] = Tree2->node[idx].nid;
//			printf("%f, %d, %d\n", calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[m].nid, Tree2->node[idx].nid), Tree1->node[m].nid, Tree2->node[idx].nid);
			
		}	
		else
			matchingList[m] = INVALID_VALUE;
	}



//#ifdef DEBUG	
	
	//print similarity matrix
	printf("Similarity matrix\n");
	
	for (i=0; i<nodeNum1; i++)
	{
		for (j=0; j<nodeNum2; j++)
			printf("%f ", similarity1d[i*nodeNum2+j]);
		printf("\n");
	}

//	//print similarity matrix
//	printf("Dissimilarity matrix\n");
//	
//	for (i=0; i<nodeNum1; i++)
//	{
//		for (j=0; j<nodeNum2; j++)
//			printf("%f ", dissimilarity1d[i*nodeNum2+j]);
//		printf("\n");
//	}

//#endif	
	
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
//	if (dissimilarity1d) {delete []dissimilarity1d; dissimilarity1d=0;}
	
	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
	
}


// match two trees using dynamic programming, 
// every node (root, continual nodes, branching nodes, leaf nodes) will all find matches
bool treeMatching_allNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
{
	V3DLONG i, j, m, n, p,q;
	V3DLONG nodeNum1, nodeNum2;
	nodeNum1 = Tree1->treeNodeNum;
	nodeNum2 = Tree2->treeNodeNum;

	// -----------------------------------------------------------------
	// get the number of nodes in the subtree rooted at each node
	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
	// -----------------------------------------------------------------	
	V3DLONG *subTreeNodeNum1 = 0;
	V3DLONG *subTreeNodeNum2 = 0;
	
	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 

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

	V3DLONG *childrenList1=0, *childrenNodeIdx1=0;
	V3DLONG *childrenList2 =0, *childrenNodeIdx2 = 0;	
	V3DLONG childrenNum1, childrenNum2;
	V3DLONG nodeidx1, nodeidx2;
	V3DLONG depthThre = 4;

	// allocate memory for similarity1d and intialize
	float *similarity1d = new float [nodeNum1*nodeNum2]; // optimal similarity
	if (!similarity1d)
	{
		printf("Fail to allocate memory for similarity1d. \n");
		return false;
	}
	
	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	{
////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
		similarity1d[m*nodeNum2 + n] =  -1;
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
	
	// initialize matching list
	 matchingList = new V3DLONG [nodeNum1]; //indicating for each node in tree1, the id# of the matching point in tree2
	
	for (i=0; i<nodeNum1; i++)
		matchingList[i] = -1; //need to check i
	
	// start matching
	for (i=0; i<nodeNum1; i++)
	{
	
		
		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1, childrenNodeIdx1, childrenNum1); // get direct children for the current node in Tree1

//		if (childrenNum1 < nodeNumThre)
//		{
//			for (m=0; m<nodeNum2; m++)
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[m+1]] = 1;		
//			continue;
//		}
//		
////		// construct super node set
////		V3DLONG *superNodeIdx1 = new V3DLONG [choldrenNum1];
////		superNodeIdx1[0] = sortidx1[i+1]];
////		for (int tt = 0; tt< childrenNum1; tt++)
////			superNodeIdx1[tt+1] = childrenNodeIdx1[tt];
////		

#ifdef DEBUG

		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenList1[m]);
		printf("\n");
		
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenNodeIdx1[m]);
		printf("\n\n");
		
#endif
		// find candidate nodes in Tree2 that can be matched to the current node in tree1
		
		for (j=0; j<nodeNum2; j++) 
		{

			
			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for a node in Tree2
//			Tree2->getAllChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); //get all children for the current node in Tree2

//			if (childrenNum2 < nodeNumThre)
//			{
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 1;
//				continue;
//			}
//
////			// construct super node set
////			V3DLONG *superNodeIdx2 = new V3DLONG [choldrenNum2];
////			superNodeIdx2[0] = sortidx2[i+1]];
////			for (int tt = 0; tt< childrenNum2; tt++)
////				superNodeIdx2[tt+1] = childrenNodeIdx2[tt];
			
#ifdef DEBUG

			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenList2[m]);
			printf("\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenNodeIdx2[m]);
			printf("\n\n");
#endif			
			
			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);			
//			printf("%d, %d, %d, %d\n", i,j, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
							
			for (int p = 0; p<nodeNum1; p++)
				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
				
			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];

//#ifdef DEBUG	
//			printf("The initial similarity:\n");
//			printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1dtmp);
//
//#endif			
			
			// find the best matching for subtree using bipartite matching			
			if ((childrenNum1!=0)&&(childrenNum2!=0)) // if one of the current nodes in tree1 and tree2 are leaf nodes, do not do the following
			{
				
				Matrix<double> bpMatrix(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix, need to free?
				
				for (m=0; m<childrenNum1+childrenNum2; m++)
				for (n=0; n<childrenNum1+childrenNum2; n++)
//						bpMatrix(m,n) = 9999;
						bpMatrix(m,n) = -INVALID_VALUE;
						
				// build the matrix
				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1*childrenNum2*2];

				for (m=0; m<childrenNum1; m++)
				for (n=0; n<childrenNum2; n++)
				{
					bestSubMatch[m*childrenNum2*2+n*2] = INVALID_VALUE;
					bestSubMatch[m*childrenNum2*2+n*2+1] = INVALID_VALUE;
				}
				
				for (m=0; m<childrenNum1; m++)
				{
					Tree1->getIndex(childrenList1[m], nodeidx1);

					//get all children rooted at childrenList1[m], 
	
					V3DLONG *subTreeNodeIdx1 = 0;
					swcTree *subTree1 = 0; 
//					Tree1->getSubTree(childrenList1[m], subTree1, subTreeNodeIdx1); // include root of the subtree, i.e., nodeid
					Tree1->getSubTree(childrenList1[m], depthThre, subTree1, subTreeNodeIdx1);

#ifdef DEBUG
					printf("subTreeNodeIdx1: ");
					
					for (p=0; p<subTree1->treeNodeNum; p++)
						printf("%d, ", subTreeNodeIdx1[p]);
					printf("\n");
#endif					
					for (n=0; n<childrenNum2; n++)
					{
						Tree2->getIndex(childrenList2[n], nodeidx2);
						
					    // find the best one in the subtree rooted at nodeidx2
						V3DLONG *subTreeNodeIdx2 = 0;
						swcTree *subTree2 = 0; 
//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
						Tree2->getSubTree(childrenList2[n], depthThre, subTree2, subTreeNodeIdx2);
#ifdef DEBUG
						printf("subTreeNodeIdx2: ");
						
						for (p=0; p<subTree2->treeNodeNum; p++)
							printf("%d, ", subTreeNodeIdx2[p]);
						printf("\n");
#endif						
						// find the best match with highest similarity in the subtree rooted at childrenList1[m] and childreList2[m]
						
						float bestval = INVALID_VALUE;
						for (p=0; p<subTree1->treeNodeNum; p++)
						for (q=0; q<subTree2->treeNodeNum; q++)
							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
							{
								bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
								bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
								bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
								
							}
//						bpMatrix(m,n) = 9999 - bestval; // hungarian method use distance 
						bpMatrix(m,n) = -bestval; // hungarian method use distance 
						
						if (subTree2) {delete subTree2; subTree2=0;}
						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
								
					} // for n
					
					if (subTree1) {delete subTree1; subTree1=0;}
					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}

				} // for m
								
				// Hungarian algorithm for bipartite matching, find the minimum cost
				Matrix<double> bpMatrixOut(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix

				for (m=0; m<childrenNum1+childrenNum2; m++)
				for (n=0; n<childrenNum1+childrenNum2; n++)
					bpMatrixOut(m,n) = bpMatrix(m,n);
					
				Munkres branchMatch;
				branchMatch.solve(bpMatrixOut); 

//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				{
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						printf("%f ", bpMatrixOut(m,n));
//					printf("\n");
//			    }
//				printf("\n");

				
				// get temporary simiarlity1d, and matchingfunc1d
				
				for (m=0; m<childrenNum1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
				{
					Tree1->getIndex(childrenList1[m], nodeidx1);
				
					for (n=0; n<childrenNum2; n++) 
					{
	//					printf("%f\n ", bpMatrix(m,n));
//						printf("%f\n ", bpMatrixOut(m,n));
					
						Tree2->getIndex(childrenList2[n], nodeidx2);

						if (bpMatrixOut(m,n)==0) // find a match
						{
							// update similarity1d and mappingfunc1d						
// //							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += similarity1d[nodeidx1*nodeNum2+nodeidx2];
//							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += (9999-bpMatrix(m,n));

//							printf("%f ", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);

							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
							
//							// update dissimilarity1d and mappingfunc1d						
//							dissimilarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += bpMatrix(m,n);

							V3DLONG ind1 = bestSubMatch[m*childrenNum2*2+n*2];
							V3DLONG ind2 = bestSubMatch[m*childrenNum2*2+n*2+1];
							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
								
							//expand matching list of the children
							for (p = 0; p<nodeNum1; p++)
							{	
								
								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
										mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
										mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
								
							} //for p
						} //if (bpMatrixOut(m,n)==0)
					} // for n
//					printf("\n");
				} // for m	
				
//				// normalize similarity1d, make it irrelavant to the number of matching
//				V3DLONG matchcnt = 0;
//				for (p = 0; p<nodeNum1; p++)
//					if (mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]!=INVALID_VALUE)
//						matchcnt++;
//						
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] /= matchcnt;	
				
						
//				if (bestnodeid) {delete []bestnodeid; bestnodeid = 0;}

				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
				
#ifdef DEBUG				
				printf("The final similarity involving the similarities of the children:\n"); 
				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
#endif			
			} //if ((childrenNum1!=0)&&(childrenNum2!=0))


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

							
			if (childrenList2) {delete childrenList2; childrenList2=0;}
			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
		} //for (j=0; j<nodeNum2; j++)
		

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

	if (childrenList1) {delete childrenList1; childrenList1=0;}
	if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
		
	}// for i
	
	
	// reconstruct the optimal match from stored best matches
	
	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);
	
	Tree1->getIndex(rootid1, rootidx1);
	Tree2->getIndex(rootid2, rootidx2);
	
//	if (nodeNumThre==0)
		simMeasure = similarity1d[rootidx1*nodeNum2+rootidx2];
		
//	if (nodeNumThre ==2)
//		simMeasure = similarity1d[Tree1->node[rootidx1].nid*nodeNum2+Tree2->node[rootidx2].nid];
	
	printf("simMeasure = %f\n", simMeasure);
	
	for (m=0; m<nodeNum1; m++)
	{
		V3DLONG idx;
//		if (nodeNumThre == 0)
			idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
	
		
		if (idx>=0)
		{
			matchingList[m] = Tree2->node[idx].nid;
//			printf("%f, %d, %d\n", calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[m].nid, Tree2->node[idx].nid), Tree1->node[m].nid, Tree2->node[idx].nid);
			
		}	
		else
			matchingList[m] = INVALID_VALUE;
	}



//#ifdef DEBUG	
	
	//print similarity matrix
	printf("Similarity matrix\n");
	
	for (i=0; i<nodeNum1; i++)
	{
		for (j=0; j<nodeNum2; j++)
			printf("%f ", similarity1d[i*nodeNum2+j]);
		printf("\n");
	}

//#endif	
	
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
//	if (dissimilarity1d) {delete []dissimilarity1d; dissimilarity1d=0;}
	
	if (mappingfunc1d) {delete []mappingfunc1d; mappingfunc1d=0;}
	
}



// match two trees using dynamic programming
// root, continual nodes, leaf nodes will find matches, continual nodes will not find matches
// suitable for matching neurons of the same structures
bool treeMatching_noContinualNodes(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
{
	V3DLONG i, j, m, n, p,q;

//	float nodeNumThre = 2; // nodes with subtree node number no less than this value will be considered in match, 2 for matching neurons by structures, 0 for matching neurons by geometrical features


	// -----------------------------------------------------------------
	// generate trees only containing root, branching points, and leaf nodes
	// remove continual points
	// the input tree pointers are backed up in Tree1_old, Tree2_old
	// the trees after removing continual nodes are Tree1 and Tree2
	// -----------------------------------------------------------------	
	swcTree *Tree1_old =0, *Tree2_old = 0;
	swcTree *newTree1 =0, *newTree2 = 0;
	unsigned char *removeTag1=0, *removeTag2=0;
	
	Tree1_old = Tree1;
	Tree2_old = Tree2;
	
//	Tree1->removeContinualNodes(newTree1, removeTag1);
	Tree1->removeContinuaLeaflNodes(newTree1, removeTag1);
	Tree1 = newTree1;
	newTree1 = 0;

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		printf("%d ", Tree1->node[i].nid);
	}
	printf("\n");
		
//	if (Tree1) {delete Tree1; Tree1=newTree1; newTree1=0;} 

//	Tree2->removeContinualNodes(newTree2, removeTag2);
	Tree2->removeContinuaLeaflNodes(newTree2, removeTag2);
	Tree2 = newTree2;
	newTree2 = 0;
	
	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		printf("%d ", Tree2->node[i].nid);
	}
	printf("\n");
	
//	if (Tree2) {delete Tree2; Tree2=newTree2; newTree2=0;} 
	
	// -----------------------------------------------------------------
	// get the number of nodes in the subtree rooted at each node
	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
	// -----------------------------------------------------------------	
	V3DLONG *subTreeNodeNum1 = 0;
	V3DLONG *subTreeNodeNum2 = 0;
	
	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 

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
	
	V3DLONG *childrenList1=0, *childrenNodeIdx1=0;
	V3DLONG *childrenList2 =0, *childrenNodeIdx2 = 0;	
	V3DLONG childrenNum1, childrenNum2;
	V3DLONG nodeidx1, nodeidx2;
	V3DLONG depthThre = 4;

	// allocate memory for similarity1d and intialize
	float *similarity1d = new float [nodeNum1*nodeNum2]; // optimal similarity
	if (!similarity1d)
	{
		printf("Fail to allocate memory for similarity1d. \n");
		return false;
	}
	
	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	{
////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
		similarity1d[m*nodeNum2 + n] =  -1;
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
	 matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
	
	for (i=0; i<Tree1_old->treeNodeNum; i++)
		matchingList[i] = INVALID_VALUE; //need to check i
	
	// start matching
	for (i=0; i<nodeNum1; i++)
	{
	
		
		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1, childrenNodeIdx1, childrenNum1); // get direct children for the current node in Tree1

//		if (childrenNum1 == 0) // leaf node, note that Tree1 and Tree2 now only contain root, branching nodes, and leaf nodes
//		{
//			for (m=0; m<nodeNum2; m++)
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[m+1]] = 1;		
//			continue;
//		}
//		
////		// construct super node set
////		V3DLONG *superNodeIdx1 = new V3DLONG [choldrenNum1];
////		superNodeIdx1[0] = sortidx1[i+1]];
////		for (int tt = 0; tt< childrenNum1; tt++)
////			superNodeIdx1[tt+1] = childrenNodeIdx1[tt];
//		

#ifdef DEBUG

		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenList1[m]);
		printf("\n");
		
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenNodeIdx1[m]);
		printf("\n\n");
		
#endif
		// find candidate nodes in Tree2 that can be matched to the current node in tree1
		
		for (j=0; j<nodeNum2; j++) 
		{

			
			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for a node in Tree2
//			Tree2->getAllChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2, childrenNodeIdx2, childrenNum2); //get all children for the current node in Tree2

//			if (childrenNum2 == 0) //leaf nodes
//			{
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 1;
//				continue;
//			}
////			// construct super node set
////			V3DLONG *superNodeIdx2 = new V3DLONG [choldrenNum2];
////			superNodeIdx2[0] = sortidx2[i+1]];
////			for (int tt = 0; tt< childrenNum2; tt++)
////				superNodeIdx2[tt+1] = childrenNodeIdx2[tt];
			
#ifdef DEBUG

			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenList2[m]);
			printf("\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenNodeIdx2[m]);
			printf("\n\n");
#endif			
			
			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
//			
////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_length_weighted_distance(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);

//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
				
			for (int p = 0; p<nodeNum1; p++)
				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
				
			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];

//#ifdef DEBUG	
//			printf("The initial similarity:\n");
//			printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1dtmp);
//
//#endif			
			
			// find the best matching for subtree using bipartite matching			
			if ((childrenNum1!=0)&&(childrenNum2!=0)) // if one of the current nodes in tree1 and tree2 are leaf nodes, do not do the following
			{
				
				Matrix<double> bpMatrix(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix, need to free?
				
				for (m=0; m<childrenNum1+childrenNum2; m++)
				for (n=0; n<childrenNum1+childrenNum2; n++)
						bpMatrix(m,n) = -INVALID_VALUE;
						
				// build the matrix
				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1*childrenNum2*2];

				for (m=0; m<childrenNum1; m++)
				for (n=0; n<childrenNum2; n++)
				{
					bestSubMatch[m*childrenNum2*2+n*2] = INVALID_VALUE;
					bestSubMatch[m*childrenNum2*2+n*2+1] = INVALID_VALUE;
				}
				
				for (m=0; m<childrenNum1; m++)
				{
					Tree1->getIndex(childrenList1[m], nodeidx1);

					//get all children rooted at childrenList1[m], 
	
					V3DLONG *subTreeNodeIdx1 = 0;
					swcTree *subTree1 = 0; 
//					Tree1->getSubTree(childrenList1[m], subTree1, subTreeNodeIdx1); // include root of the subtree, i.e., nodeid
					Tree1->getSubTree(childrenList1[m], depthThre, subTree1, subTreeNodeIdx1);

#ifdef DEBUG
					printf("subTreeNodeIdx1: ");
					
					for (p=0; p<subTree1->treeNodeNum; p++)
						printf("%d, ", subTreeNodeIdx1[p]);
					printf("\n");
#endif					
					for (n=0; n<childrenNum2; n++)
					{
						Tree2->getIndex(childrenList2[n], nodeidx2);
						
					    // find the best one in the subtree rooted at nodeidx2
						V3DLONG *subTreeNodeIdx2 = 0;
						swcTree *subTree2 = 0; 
//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
						Tree2->getSubTree(childrenList2[n], depthThre, subTree2, subTreeNodeIdx2);
#ifdef DEBUG
						printf("subTreeNodeIdx2: ");
						
						for (p=0; p<subTree2->treeNodeNum; p++)
							printf("%d, ", subTreeNodeIdx2[p]);
						printf("\n");
#endif						
						// find the best match with highest similarity in the subtree rooted at childrenList1[m] and childreList2[m]
						
						float bestval = INVALID_VALUE;
						for (p=0; p<subTree1->treeNodeNum; p++)
						for (q=0; q<subTree2->treeNodeNum; q++)
							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
							{
								bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
								bestSubMatch[m*childrenNum2*2+n*2] = subTreeNodeIdx1[p];
								bestSubMatch[m*childrenNum2*2+n*2+1] = subTreeNodeIdx2[q];
								
							}
//						bpMatrix(m,n) = 9999 - bestval; // hungarian method use distance 
						bpMatrix(m,n) = -bestval; // hungarian method use distance 
						
						if (subTree2) {delete subTree2; subTree2=0;}
						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
								
					} // for n
					
					if (subTree1) {delete subTree1; subTree1=0;}
					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}

				} // for m
								
				// Hungarian algorithm for bipartite matching, find the minimum cost
				Matrix<double> bpMatrixOut(childrenNum1+childrenNum2, childrenNum1+childrenNum2);// bipartite matrix

				for (m=0; m<childrenNum1+childrenNum2; m++)
				for (n=0; n<childrenNum1+childrenNum2; n++)
					bpMatrixOut(m,n) = bpMatrix(m,n);
					
				Munkres branchMatch;
				branchMatch.solve(bpMatrixOut); 

//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				{
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						printf("%f ", bpMatrixOut(m,n));
//					printf("\n");
//			    }
//				printf("\n");

				
				// get temporary simiarlity1d, and matchingfunc1d
				
				for (m=0; m<childrenNum1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
				{
					Tree1->getIndex(childrenList1[m], nodeidx1);
				
					for (n=0; n<childrenNum2; n++) 
					{
	//					printf("%f\n ", bpMatrix(m,n));
//						printf("%f\n ", bpMatrixOut(m,n));
					
						Tree2->getIndex(childrenList2[n], nodeidx2);

						if (bpMatrixOut(m,n)==0) // find a match
						{
							// update similarity1d and mappingfunc1d						
							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
							
							V3DLONG ind1 = bestSubMatch[m*childrenNum2*2+n*2];
							V3DLONG ind2 = bestSubMatch[m*childrenNum2*2+n*2+1];
							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
								
							//expand matching list of the children
							for (p = 0; p<nodeNum1; p++)
							{	
								
								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
										mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
										mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
								
							} //for p
						} //if (bpMatrixOut(m,n)==0)
					} // for n
//					printf("\n");
				} // for m	
				
//				// normalize similarity1d, make it irrelavant to the number of matching
//				V3DLONG matchcnt = 0;
//				for (p = 0; p<nodeNum1; p++)
//					if (mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]!=INVALID_VALUE)
//						matchcnt++;
//						
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] /= matchcnt;	
				
						
//				if (bestnodeid) {delete []bestnodeid; bestnodeid = 0;}

				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
				
#ifdef DEBUG				
				printf("The final similarity involving the similarities of the children:\n"); 
				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
#endif			
			} //if ((childrenNum1!=0)&&(childrenNum2!=0))


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

							
			if (childrenList2) {delete childrenList2; childrenList2=0;}
			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}
//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
		} //for (j=0; j<nodeNum2; j++)
		

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

	if (childrenList1) {delete childrenList1; childrenList1=0;}
	if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}
//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
		
	}// for i
	
	
	// reconstruct the optimal match from stored best matches
	
	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);

	Tree1->getIndex(rootid1, rootidx1);
	Tree2->getIndex(rootid2, rootidx2);

	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;

	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
	
	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
		printf("Root should have only one child\n");

	for (m=0; m<nodeNum1; m++)
	{
		V3DLONG idx;
	
		
		if (m==rootidx1) //add root
			idx = rootidx2;
		else
			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];

		V3DLONG tmpidx1;
		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
		
		if (idx>=0)
		{
			matchingList[tmpidx1] = Tree2->node[idx].nid;		
//			matchingList[m] = Tree2->node[idx].nid;
			
		}	
		else
			matchingList[tmpidx1] = INVALID_VALUE;
//			matchingList[m] = INVALID_VALUE;
	}

//	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes

	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
	printf("simMeasure = %f\n", simMeasure);
	
	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}

//	for (m=0; m<nodeNum1; m++)
//	{
//		V3DLONG idx;
//	
//		V3DLONG tmpidx1, tmpidx2;
//		V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
//		V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
//		
//		if (m==rootidx1) //add root
//			idx = rootidx2;
//		else
//		{
//			Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
//			Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
//			
//			if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
//				printf("Root should have only one child\n");
//							
//			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
//		}
//		
//		
//		if (idx>=0)
//		{
//			matchingList[m] = Tree2->node[idx].nid;
////			printf("%f, %d, %d\n", calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[m].nid, Tree2->node[idx].nid), Tree1->node[m].nid, Tree2->node[idx].nid);
//			
//		}	
//		else
//			matchingList[m] = INVALID_VALUE;
//	}
//
//	simMeasure = similarity1d[Tree1->node[rootidx1].nid*nodeNum2+Tree2->node[rootidx2].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	

	


//#ifdef DEBUG	
	
	//print similarity matrix
	printf("Similarity matrix\n");
	
	for (i=0; i<nodeNum1; i++)
	{
		for (j=0; j<nodeNum2; j++)
			printf("%f ", similarity1d[i*nodeNum2+j]);
		printf("\n");
	}
	
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
	
	if (removeTag1) {delete removeTag1; removeTag1=0;}
	if (removeTag2) {delete removeTag2; removeTag2=0;}

}

// match two trees using dynamic programming
// Different from treeMatching_noContinualNodes, the similarity metrics (length-ratio) are dependent between parent and 
// children, i.e., which branch match to which branch when computing legnth-ratio should be consistent with the bipartite matching
// result of children nodes
// root, branching nodes, continual and leaf nodes are removed before matching, thus will not find matches

bool treeMatching_noContinualLeafNodes_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
{
	V3DLONG i, j, m, n, p,q;


	// -----------------------------------------------------------------
	// generate trees only containing root, branching points, and leaf nodes
	// remove continual points
	// the input tree pointers are backed up in Tree1_old, Tree2_old
	// the trees after removing continual nodes are Tree1 and Tree2
	// -----------------------------------------------------------------	
	swcTree *Tree1_old =0, *Tree2_old = 0;
	swcTree *newTree1 =0, *newTree2 = 0;
	unsigned char *removeTag1=0, *removeTag2=0;
	
	Tree1_old = Tree1;
	Tree2_old = Tree2;
	
//	Tree1->removeContinualNodes(newTree1, removeTag1);
	Tree1->removeContinuaLeaflNodes(newTree1, removeTag1);
	Tree1 = newTree1;
	newTree1 = 0;

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		printf("%d ", Tree1->node[i].nid);
	}
	printf("\n");
		
//	if (Tree1) {delete Tree1; Tree1=newTree1; newTree1=0;} 

//	Tree2->removeContinualNodes(newTree2, removeTag2);
	Tree2->removeContinuaLeaflNodes(newTree2, removeTag2);
	Tree2 = newTree2;
	newTree2 = 0;
	
	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		printf("%d ", Tree2->node[i].nid);
	}
	printf("\n");
	
//	if (Tree2) {delete Tree2; Tree2=newTree2; newTree2=0;} 
	
	// -----------------------------------------------------------------
	// get the number of nodes in the subtree rooted at each node
	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
	// -----------------------------------------------------------------	
	V3DLONG *subTreeNodeNum1 = 0;
	V3DLONG *subTreeNodeNum2 = 0;
	
	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 

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
	V3DLONG depthThre = 4;

	// allocate memory for similarity1d and intialize
	float *similarity1d = new float [nodeNum1*nodeNum2]; // #(branching nodes + root in Tree 1) *#(branching nodes + root in Tree 2)
	if (!similarity1d)
	{
		printf("Fail to allocate memory for similarity1d. \n");
		return false;
	}
	
	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	{
////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
//		similarity1d[m*nodeNum2 + n] =  -1;
		similarity1d[m*nodeNum2 + n] =  0;
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
	 matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
	
	for (i=0; i<Tree1_old->treeNodeNum; i++)
		matchingList[i] = INVALID_VALUE; //need to check i
	
	// start matching
	for (i=0; i<nodeNum1; i++)
	{
	
		V3DLONG *childrenList1_Tree1=0, *childrenNodeIdx1_Tree1=0, childrenNum1_Tree1;
		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1, childrenNodeIdx1_Tree1, childrenNum1_Tree1); // get direct children for the current node in Tree1
		
		V3DLONG *childrenList1_Tree1_old=0, *childrenNodeIdx1_Tree1_old=0, childrenNum1_Tree1_old;
		Tree1_old->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1_old, childrenNodeIdx1_Tree1_old, childrenNum1_Tree1_old); // get direct children for the current node in Tree1
		

#ifdef DEBUG

		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenList1[m]);
		printf("\n");
		
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenNodeIdx1[m]);
		printf("\n\n");
		
#endif

		float *lengthratio1=0;
		unsigned char lengthratioNum1;
		Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4);

		// find candidate nodes in Tree2 that can be matched to the current node in tree1
		
		for (j=0; j<nodeNum2; j++) 
		{

			printf("i=%d,j=%d\n", i,j);	
			V3DLONG *childrenList2_Tree2=0, *childrenNodeIdx2_Tree2=0, childrenNum2_Tree2;
			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2, childrenNodeIdx2_Tree2, childrenNum2_Tree2); // get direct children for the current node in Tree1
			
			V3DLONG *childrenList2_Tree2_old=0, *childrenNodeIdx2_Tree2_old=0, childrenNum2_Tree2_old;
			Tree2_old->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2_old, childrenNodeIdx2_Tree2_old, childrenNum2_Tree2_old); // get direct children for the current node in Tree1

								
			
#ifdef DEBUG

			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenList2[m]);
			printf("\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenNodeIdx2[m]);
			printf("\n\n");
#endif			
			
//			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
////			
//////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_length_weighted_distance(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);

//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
				
//			for (int p = 0; p<nodeNum1; p++)
//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//				
//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];

			// compute metrics of each branch based on which similarity is further computed
			float *lengthratio2=0;
			unsigned char lengthratioNum2;
			Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4);
			
			float *simVector = 0;
//			simVector = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
			simVector = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
			
#ifdef DEBUG
			for (m=0; m<lengthratioNum1; m++)
			{
				for (n=0; n<lengthratioNum2; n++)
					printf("%f ", simVector[m*lengthratioNum2+n]);
				printf("\n");
			}
			printf("\n");
#endif			
			
			for (int p = 0; p<nodeNum1; p++)
				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;

			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
				

			// find the best matching for subtree using bipartite matching	
			
			if ((childrenNum1_Tree1==0)||(childrenNum2_Tree2==0)) 
			// only compute the local similarit, i.e., the best similarity of length-ratio (no subtree nodes similarity) of the two nodes without
			// adding the similarity of subtree nodes
			{
				Matrix<double> bpMatrix(childrenNum1_Tree1_old+childrenNum2_Tree2_old, childrenNum1_Tree1_old+childrenNum2_Tree2_old);// lengthratioNum1 and lengthratioNum2 are the number of children before pruning 
				
				printf("print bpMatrix\n");
				
				for (m=0; m<childrenNum1_Tree1_old+childrenNum2_Tree2_old; m++)
				{
					for (n=0; n<childrenNum1_Tree1_old+childrenNum2_Tree2_old; n++)
					{
						if ((m<childrenNum1_Tree1_old)&&(n<childrenNum2_Tree2_old))
							bpMatrix(m,n) = -INVALID_VALUE-simVector[m*childrenNum2_Tree2_old+n];
						else // dummy nodes
							bpMatrix(m,n) = -INVALID_VALUE;
						printf("%f ", bpMatrix(m,n));
					}
					printf("\n");
				}

				Matrix<double> bpMatrixOut(childrenNum1_Tree1_old+childrenNum2_Tree2_old, childrenNum1_Tree1_old+childrenNum2_Tree2_old);// bipartite matrix

				for (m=0; m<childrenNum1_Tree1_old+childrenNum2_Tree2_old; m++)
				for (n=0; n<childrenNum1_Tree1_old+childrenNum2_Tree2_old; n++)
					bpMatrixOut(m,n) = bpMatrix(m,n);
					
				Munkres branchMatch;
				branchMatch.solve(bpMatrixOut); 
				
				// compute simiarlity1d
				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 0;

				printf("print bpMatrixOut\n");
				
				for (m=0; m<childrenNum1_Tree1_old; m++) // note not necessary to consider m<childrenNum1+childrenNum2
				{
					for (n=0; n<childrenNum2_Tree2_old; n++) 
					{	
		   				printf("%f ", bpMatrixOut(m,n));
						
						if (bpMatrixOut(m,n)==0) // find a match
							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += simVector[m*childrenNum2_Tree2_old+n]; 	// update similarity1d and mappingfunc1d						
					}
					printf("\n");
				}
				
				printf("%d, %d, %f\n", i,j,similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
				
			}
			else // none of the two nodes are leaf nodes in the pruned tree, they may be nodes that have only one child in the pruned trees though
			{
				
				Matrix<double> bpMatrix(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// consider branches before pruning
				
				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
				for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
						bpMatrix(m,n) = -INVALID_VALUE;

//				for (m=0; m<lengthratioNum1+lengthratioNum2; m++)
//				for (n=0; n<lengthratioNum1+lengthratioNum2; n++)
//				{
//					if ((m<lengthratioNum1)&&(n<lengthratioNum2))
//						bpMatrix(m,n) = -INVALID_VALUE-simVector[m*lengthratioNum2+n];
//					else
//						bpMatrix(m,n) = -INVALID_VALUE;
//				}
					
//				initialize bestSubMatch
				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1_Tree1*childrenNum2_Tree2*2];

				for (m=0; m<childrenNum1_Tree1; m++)
				for (n=0; n<childrenNum2_Tree2; n++)
				{
					bestSubMatch[m*childrenNum2_Tree2*2+n*2] = INVALID_VALUE;
					bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = INVALID_VALUE;
				}
				
				// assign values to bpMatrix				
				for (m=0; m<childrenNum1_Tree1; m++)
				{
					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1); 

					//get all children rooted at childrenList1_Tree1[m], 	
					V3DLONG *subTreeNodeIdx1 = 0;
					swcTree *subTree1 = 0; 
					Tree1->getSubTree(childrenList1_Tree1[m], depthThre, subTree1, subTreeNodeIdx1);
					
#ifdef DEBUG
					printf("subTreeNodeIdx1: ");
					
					for (p=0; p<subTree1->treeNodeNum; p++)
						printf("%d, ", subTreeNodeIdx1[p]);
					printf("\n");
#endif					

					// find the corresponding branches in the Tree1_old
//					V3DLONG s = 0;
//					while (s<childrenNum1_Tree1_old)
//					{
//						if (childrenList1_Tree1_old[s] == childrenList1_Tree1[m])
//							break;
//						else
//							s++;
//					}

					V3DLONG s = 0;
					while (s<childrenNum1_Tree1_old)
					{
						V3DLONG *pathNodeList =0;
						V3DLONG pathNodeNum;
						unsigned char ancestorTag;
						
						Tree1_old->findPath(childrenList1_Tree1_old[s], childrenList1_Tree1[m], pathNodeList, pathNodeNum, ancestorTag); 
						printf("ancestorTag = %d\n", ancestorTag);
						if (pathNodeList) {delete pathNodeList; pathNodeList=0;}
						
						if (ancestorTag == 1)
							break;
						else
							s++;
					}
				
											
					for (n=0; n<childrenNum2_Tree2; n++)
					{
						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);
						
						// find the best one in the subtree rooted at nodeidx2
						V3DLONG *subTreeNodeIdx2 = 0;
						swcTree *subTree2 = 0; 
//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
						Tree2->getSubTree(childrenList2_Tree2[n], depthThre, subTree2, subTreeNodeIdx2);
#ifdef DEBUG
						printf("subTreeNodeIdx2: ");
						
						for (p=0; p<subTree2->treeNodeNum; p++)
							printf("%d, ", subTreeNodeIdx2[p]);
						printf("\n");
#endif						
						// find the best match with highest similarity in the subtree rooted at childrenList1_Tree1[m] and childreList2_Tree2[n]						
						float bestval = INVALID_VALUE;
						for (p=0; p<subTree1->treeNodeNum; p++)
						for (q=0; q<subTree2->treeNodeNum; q++)
							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]] + simVector[p*lengthratioNum2+q])
							{
								bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
								bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p];
								bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q];
								
							}

						// find the corresponding branches in Tree2_old						
//						V3DLONG t = 0;
//						while (t<childrenNum2_Tree2_old)
//						{
//							if (childrenList2_Tree2_old[t] == childrenList2_Tree2[n])
//								break;
//							else
//								t++;
//						}

					V3DLONG t = 0;
					while (t<childrenNum2_Tree2_old)
					{
						V3DLONG *pathNodeList =0;
						V3DLONG pathNodeNum;
						unsigned char ancestorTag;
						
						Tree2_old->findPath(childrenList2_Tree2_old[t], childrenList2_Tree2[n], pathNodeList, pathNodeNum, ancestorTag);
						if (pathNodeList) {delete pathNodeList; pathNodeList=0;}
						
						if (ancestorTag == 1)
							break;
						else
							t++;
					}
						
						// assign value to bpMatrix(m,n)							
						bpMatrix(m,n) = -bestval-simVector[s*lengthratioNum2+t]; // hungarian method use distance 
												
						if (subTree2) {delete subTree2; subTree2=0;}
						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
								
					} // for n
				
					if (subTree1) {delete subTree1; subTree1=0;}
					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}

				} // for m
						
														
				// Hungarian algorithm for bipartite matching, find the minimum cost
				Matrix<double> bpMatrixOut(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// bipartite matrix

				printf("printf bpMatrixOut(m,n)\n");
				
				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
				{
					for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
					{
						bpMatrixOut(m,n) = bpMatrix(m,n);
						printf("%f ", bpMatrixOut(m,n));
					}
					printf("\n");
				}
				printf("\n");
				
					
				Munkres branchMatch;
				branchMatch.solve(bpMatrixOut); 

//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				{
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						printf("%f ", bpMatrixOut(m,n));
//					printf("\n");
//			    }
//				printf("\n");

				
				// get temporary simiarlity1d, and matchingfunc1d
				
				for (m=0; m<childrenNum1_Tree1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
				{
					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1);
				
					for (n=0; n<childrenNum2_Tree2; n++) 
					{
	//					printf("%f\n ", bpMatrix(m,n));
//						printf("%f\n ", bpMatrixOut(m,n));
					
						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);

						if (bpMatrixOut(m,n)==0) // find a match
						{
							// update similarity1d and mappingfunc1d						
							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
							
							V3DLONG ind1 = bestSubMatch[m*childrenNum2_Tree2*2+n*2];
							V3DLONG ind2 = bestSubMatch[m*childrenNum2_Tree2*2+n*2+1];
							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
								
							//expand matching list of the children
							for (p = 0; p<nodeNum1; p++)
							{	
								
								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
										mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
										mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
								
							} //for p
						} //if (bpMatrixOut(m,n)==0)
					} // for n
//					printf("\n");
				} // for m	
				
				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
				
#ifdef DEBUG				
				printf("The final similarity involving the similarities of the children:\n"); 
				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
#endif			
			} //if ((childrenNum1!=0)&&(childrenNum2!=0))


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
//			if (childrenList2) {delete childrenList2; childrenList2=0;}
//			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}

			if (childrenList2_Tree2) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
			if (childrenNodeIdx2_Tree2) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
			
			if (childrenList2_Tree2_old) {delete childrenList2_Tree2_old; childrenList2_Tree2_old=0;}
			if (childrenNodeIdx2_Tree2_old) {delete childrenNodeIdx2_Tree2_old; childrenNodeIdx2_Tree2_old=0;}
			
//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
		} //for (j=0; j<nodeNum2; j++)
		

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
//	if (childrenList1) {delete childrenList1; childrenList1=0;}
//	if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}

	if (childrenList1_Tree1) {delete childrenList1_Tree1; childrenList1_Tree1=0;}
	if (childrenNodeIdx1_Tree1) {delete childrenNodeIdx1_Tree1; childrenNodeIdx1_Tree1=0;}
	
	if (childrenList1_Tree1_old) {delete childrenList1_Tree1_old; childrenList1_Tree1_old=0;}
	if (childrenNodeIdx1_Tree1_old) {delete childrenNodeIdx1_Tree1_old; childrenNodeIdx1_Tree1_old=0;}
	
//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
		
	}// for i
	
	
	// reconstruct the optimal match from stored best matches
	
	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);

//	Tree1->getIndex(rootid1, rootidx1);
//	Tree2->getIndex(rootid2, rootidx2);

	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;

	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
	
	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
		printf("Root should have only one child\n");

	for (m=0; m<nodeNum1; m++)
	{
		V3DLONG idx;
	
		
		if (m==rootidx1) //add root, require root must be matched to root
			idx = rootidx2;
		else
			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];

		V3DLONG tmpidx1;
		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
		
		if (idx>=0)
		{
			matchingList[tmpidx1] = Tree2->node[idx].nid;		
//			matchingList[m] = Tree2->node[idx].nid;
			
		}	
		else
			matchingList[tmpidx1] = INVALID_VALUE;
//			matchingList[m] = INVALID_VALUE;
	}

//	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes

	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
	printf("simMeasure = %f\n", simMeasure);
	
	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}


//#ifdef DEBUG	
	
	//print similarity matrix
	printf("Similarity matrix\n");
	
	for (i=0; i<nodeNum1; i++)
	{
		for (j=0; j<nodeNum2; j++)
			printf("%f ", similarity1d[i*nodeNum2+j]);
		printf("\n");
	}
	
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
	
	if (removeTag1) {delete removeTag1; removeTag1=0;}
	if (removeTag2) {delete removeTag2; removeTag2=0;}
	
	
}

// match two trees using dynamic programming
// Different from treeMatching_noContinualNodes, the similarity metrics (length-ratio) are dependent between parent and 
// children, i.e., which branch match to which branch when computing legnth-ratio should be consistent with the bipartite matching
// result of children nodes
// root, branching nodes, leaf nodes will find matches, continual nodes will not find matches
bool treeMatching_noContinualNodes_lengthRatio_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
{
	V3DLONG i, j, m, n, p,q;

	// -----------------------------------------------------------------
	// generate trees only containing root, branching points, and leaf nodes
	// remove continual points
	// the input tree pointers are backed up in Tree1_old, Tree2_old
	// the trees after removing continual nodes are Tree1 and Tree2
	// -----------------------------------------------------------------	
	swcTree *Tree1_old =0, *Tree2_old = 0;
	swcTree *newTree1 =0, *newTree2 = 0;
	unsigned char *removeTag1=0, *removeTag2=0;
	
	Tree1_old = Tree1;
	Tree2_old = Tree2;
	
	Tree1->removeContinualNodes(newTree1, removeTag1);
//	Tree1->removeContinuaLeaflNodes(newTree1, removeTag1);
	Tree1 = newTree1;
	newTree1 = 0;

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		printf("%d ", Tree1->node[i].nid);
	}
	printf("\n");
		
	Tree2->removeContinualNodes(newTree2, removeTag2);
//	Tree2->removeContinuaLeaflNodes(newTree2, removeTag2);
	Tree2 = newTree2;
	newTree2 = 0;
	
	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		printf("%d ", Tree2->node[i].nid);
	}
	printf("\n");
	
	
	// -----------------------------------------------------------------
	// get the number of nodes in the subtree rooted at each node
	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
	// -----------------------------------------------------------------	
	V3DLONG *subTreeNodeNum1 = 0;
	V3DLONG *subTreeNodeNum2 = 0;
	
	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 

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
	V3DLONG depthThre = 4;

	// allocate memory for similarity1d and intialize
	float *similarity1d = new float [nodeNum1*nodeNum2]; // #(branching nodes + root in Tree 1) *#(branching nodes + root in Tree 2)
	if (!similarity1d)
	{
		printf("Fail to allocate memory for similarity1d. \n");
		return false;
	}
	
	for (m=0; m<nodeNum1; m++)
	for (n=0; n<nodeNum2; n++)
	{
////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
//		similarity1d[m*nodeNum2 + n] =  -1;
		similarity1d[m*nodeNum2 + n] =  0;
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
	 matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
	
	for (i=0; i<Tree1_old->treeNodeNum; i++)
		matchingList[i] = INVALID_VALUE; //need to check i
	
	// start matching
	for (i=0; i<nodeNum1; i++)
	{
	
		V3DLONG *childrenList1_Tree1=0, *childrenNodeIdx1_Tree1=0, childrenNum1_Tree1;
		Tree1->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1, childrenNodeIdx1_Tree1, childrenNum1_Tree1); // get direct children for the current node in Tree1
		
//		V3DLONG *childrenList1_Tree1_old=0, *childrenNodeIdx1_Tree1_old=0, childrenNum1_Tree1_old;
//		Tree1_old->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1_old, childrenNodeIdx1_Tree1_old, childrenNum1_Tree1_old); // get direct children for the current node in Tree1
		

#ifdef DEBUG

		printf("print childrenList1[m] and childrenNodeIdx1[m]\n");
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenList1[m]);
		printf("\n");
		
		for (int m=0; m<childrenNum1; m++)
			printf("%d ", childrenNodeIdx1[m]);
		printf("\n\n");
		
#endif

		float *lengthratio1=0;
		unsigned char lengthratioNum1;
		Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4);

		// find candidate nodes in Tree2 that can be matched to the current node in tree1
		
		for (j=0; j<nodeNum2; j++) 
		{

			printf("i=%d,j=%d\n", i,j);	
			V3DLONG *childrenList2_Tree2=0, *childrenNodeIdx2_Tree2=0, childrenNum2_Tree2;
			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2, childrenNodeIdx2_Tree2, childrenNum2_Tree2); // get direct children for the current node in Tree1
			
//			V3DLONG *childrenList2_Tree2_old=0, *childrenNodeIdx2_Tree2_old=0, childrenNum2_Tree2_old;
//			Tree2_old->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2_old, childrenNodeIdx2_Tree2_old, childrenNum2_Tree2_old); // get direct children for the current node in Tree1

								
			
#ifdef DEBUG

			printf("print childrenList2[n] and childrenNodeIdx2[n]\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenList2[m]);
			printf("\n");
			
			for (m=0; m<childrenNum2; m++)
				printf("%d ", childrenNodeIdx2[m]);
			printf("\n\n");
#endif			
			
//			// compute the tempoary best similarity between the two nodes, without considering their subtree similarity yet
////			
//////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
////			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_branch_length_ratio(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = calsim_length_weighted_distance(Tree1_old, Tree2_old, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);

//			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += calsim_node_distance(Tree1, Tree2, Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid);
//			printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
				
//			for (int p = 0; p<nodeNum1; p++)
//				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;
//				
//			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];

			// compute metrics of each branch based on which similarity is further computed
			float *lengthratio2=0;
			unsigned char lengthratioNum2;
			Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4);
			
			float *simVector = 0;
			
			printf("i=%d, j=%d, lengthratioNum1 = %d, lengthratioNum2=%d\n", i,j,lengthratioNum1, lengthratioNum2);
			simVector = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
			
#ifdef DEBUG
			for (m=0; m<lengthratioNum1; m++)
			{
				for (n=0; n<lengthratioNum2; n++)
					printf("%f ", simVector[m*lengthratioNum2+n]);
				printf("\n");
			}
			printf("\n");
#endif			
			
			for (int p = 0; p<nodeNum1; p++)
				mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = INVALID_VALUE;

			mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 +(V3DLONG)sortidx1[i+1]] = sortidx2[j+1];
				

			// find the best matching for subtree using bipartite matching	
			
			if ((childrenNum1_Tree1==0)||(childrenNum2_Tree2==0)) 
			// only compute the local similarity, i.e., the best similarity of length-ratio (no subtree nodes similarity) of the two nodes without
			// adding the similarity of subtree nodes
			{
				// compute length ratio similarity
				for (m=0; m<lengthratioNum1*lengthratioNum2; m++) // note that lengthratioNum =1 for leaf nodes
						similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += simVector[m]; 	// update similarity1d										
				printf("%d, %d, %f\n", i,j,similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
				
				
			}
			else // none of the two nodes are leaf nodes in the pruned tree, they are either root or branching nodes
			{
				
				Matrix<double> bpMatrix(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// consider branches before pruning
				
				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
				for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
						bpMatrix(m,n) = -INVALID_VALUE;

//				for (m=0; m<lengthratioNum1+lengthratioNum2; m++)
//				for (n=0; n<lengthratioNum1+lengthratioNum2; n++)
//				{
//					if ((m<lengthratioNum1)&&(n<lengthratioNum2))
//						bpMatrix(m,n) = -INVALID_VALUE-simVector[m*lengthratioNum2+n];
//					else
//						bpMatrix(m,n) = -INVALID_VALUE;
//				}
					
//				initialize bestSubMatch
				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1_Tree1*childrenNum2_Tree2*2];

				for (m=0; m<childrenNum1_Tree1; m++)
				for (n=0; n<childrenNum2_Tree2; n++)
				{
					bestSubMatch[m*childrenNum2_Tree2*2+n*2] = INVALID_VALUE;
					bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = INVALID_VALUE;
				}
				
				// assign values to bpMatrix				
				for (m=0; m<childrenNum1_Tree1; m++)
				{
					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1); 

					//get all children rooted at childrenList1_Tree1[m], 	
					V3DLONG *subTreeNodeIdx1 = 0;
					swcTree *subTree1 = 0; 
					Tree1->getSubTree(childrenList1_Tree1[m], depthThre, subTree1, subTreeNodeIdx1);
					
#ifdef DEBUG
					printf("subTreeNodeIdx1: ");
					
					for (p=0; p<subTree1->treeNodeNum; p++)
						printf("%d, ", subTreeNodeIdx1[p]);
					printf("\n");
#endif					

//					// find the corresponding branches in the Tree1_old
////					V3DLONG s = 0;
////					while (s<childrenNum1_Tree1_old)
////					{
////						if (childrenList1_Tree1_old[s] == childrenList1_Tree1[m])
////							break;
////						else
////							s++;
////					}
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
				
											
					for (n=0; n<childrenNum2_Tree2; n++)
					{
						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);
						
						// find the best one in the subtree rooted at nodeidx2
						V3DLONG *subTreeNodeIdx2 = 0;
						swcTree *subTree2 = 0; 
//						Tree2->getSubTree(childrenList2[n], subTree2, subTreeNodeIdx2); // include root of the subtree, i.e., nodeid
						Tree2->getSubTree(childrenList2_Tree2[n], depthThre, subTree2, subTreeNodeIdx2);
#ifdef DEBUG
						printf("subTreeNodeIdx2: ");
						
						for (p=0; p<subTree2->treeNodeNum; p++)
							printf("%d, ", subTreeNodeIdx2[p]);
						printf("\n");
#endif						
						// find the best match with highest similarity in the subtree rooted at childrenList1_Tree1[m] and childreList2_Tree2[n]						
						float bestval = INVALID_VALUE;
						V3DLONG p1, q1;
						
						for (p=0; p<subTree1->treeNodeNum; p++)
						for (q=0; q<subTree2->treeNodeNum; q++)
							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]] + simVector[p*lengthratioNum2+q])
							{
								bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
								p1 = p; q1 = q;
//								bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p];
//								bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q];
								
							}

						bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p1];
						bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q1];

//						// find the corresponding branches in Tree2_old						
////						V3DLONG t = 0;
////						while (t<childrenNum2_Tree2_old)
////						{
////							if (childrenList2_Tree2_old[t] == childrenList2_Tree2[n])
////								break;
////							else
////								t++;
////						}
//
//					V3DLONG t = 0;
//					while (t<childrenNum2_Tree2_old)
//					{
//						V3DLONG *pathNodeList =0;
//						V3DLONG pathNodeNum;
//						unsigned char ancestorTag;
//						
//						Tree2_old->findPath(childrenList2_Tree2_old[t], childrenList2_Tree2[n], pathNodeList, pathNodeNum, ancestorTag);
//						if (pathNodeList) {delete pathNodeList; pathNodeList=0;}
//						
//						if (ancestorTag == 1)
//							break;
//						else
//							t++;
//					}
						
						// assign value to bpMatrix(m,n)	
//						bpMatrix(m,n) = -bestval-simVector[s*lengthratioNum2+t]; // hungarian method use distance 						
						bpMatrix(m,n) = -bestval-simVector[m*lengthratioNum2+n]; // hungarian method use distance 
												
												
						if (subTree2) {delete subTree2; subTree2=0;}
						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
								
					} // for n
				
					if (subTree1) {delete subTree1; subTree1=0;}
					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}

				} // for m
						
														
				// Hungarian algorithm for bipartite matching, find the minimum cost
				Matrix<double> bpMatrixOut(childrenNum1_Tree1+childrenNum2_Tree2, childrenNum1_Tree1+childrenNum2_Tree2);// bipartite matrix

				printf("printf bpMatrixOut(m,n)\n");
				
				for (m=0; m<childrenNum1_Tree1+childrenNum2_Tree2; m++)
				{
					for (n=0; n<childrenNum1_Tree1+childrenNum2_Tree2; n++)
					{
						bpMatrixOut(m,n) = bpMatrix(m,n);
						printf("%f ", bpMatrixOut(m,n));
					}
					printf("\n");
				}
				printf("\n");
				
					
				Munkres branchMatch;
				branchMatch.solve(bpMatrixOut); 

//				for (m=0; m<childrenNum1+childrenNum2; m++)
//				{
//					for (n=0; n<childrenNum1+childrenNum2; n++)
//						printf("%f ", bpMatrixOut(m,n));
//					printf("\n");
//			    }
//				printf("\n");

				
				// get temporary simiarlity1d, and matchingfunc1d
				
				for (m=0; m<childrenNum1_Tree1; m++) // note not necessary to consider m<childrenNum1+childrenNum2
				{
					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1);
				
					for (n=0; n<childrenNum2_Tree2; n++) 
					{
	//					printf("%f\n ", bpMatrix(m,n));
//						printf("%f\n ", bpMatrixOut(m,n));
					
						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);

						if (bpMatrixOut(m,n)==0) // find a match
						{
							// update similarity1d and mappingfunc1d						
							similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += -bpMatrix(m,n);
//							printf("%f, %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]], bpMatrix(m,n));
							
							V3DLONG ind1 = bestSubMatch[m*childrenNum2_Tree2*2+n*2];
							V3DLONG ind2 = bestSubMatch[m*childrenNum2_Tree2*2+n*2+1];
							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
								
							//expand matching list of the children
							for (p = 0; p<nodeNum1; p++)
							{	
								
								if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
								    (mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
										mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
										mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
								
							} //for p
						} //if (bpMatrixOut(m,n)==0)
					} // for n
//					printf("\n");
				} // for m	
				
				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
				
#ifdef DEBUG				
				printf("The final similarity involving the similarities of the children:\n"); 
				printf("%d, %d, %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid, Tree2->node[(V3DLONG)sortidx2[j+1]].nid, similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
#endif			
			} //if ((childrenNum1!=0)&&(childrenNum2!=0))


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
//			if (childrenList2) {delete childrenList2; childrenList2=0;}
//			if (childrenNodeIdx2) {delete childrenNodeIdx2; childrenNodeIdx2=0;}

			if (childrenList2_Tree2) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
			if (childrenNodeIdx2_Tree2) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
			
//			if (childrenList2_Tree2_old) {delete childrenList2_Tree2_old; childrenList2_Tree2_old=0;}
//			if (childrenNodeIdx2_Tree2_old) {delete childrenNodeIdx2_Tree2_old; childrenNodeIdx2_Tree2_old=0;}
			
//			if (superNodeIdx2) {delete superNodeIdx2; superNodeIdx2=0;}
		} //for (j=0; j<nodeNum2; j++)
		

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
//	if (childrenList1) {delete childrenList1; childrenList1=0;}
//	if (childrenNodeIdx1) {delete childrenNodeIdx1; childrenNodeIdx1=0;}

	if (childrenList1_Tree1) {delete childrenList1_Tree1; childrenList1_Tree1=0;}
	if (childrenNodeIdx1_Tree1) {delete childrenNodeIdx1_Tree1; childrenNodeIdx1_Tree1=0;}
	
//	if (childrenList1_Tree1_old) {delete childrenList1_Tree1_old; childrenList1_Tree1_old=0;}
//	if (childrenNodeIdx1_Tree1_old) {delete childrenNodeIdx1_Tree1_old; childrenNodeIdx1_Tree1_old=0;}
	
//	if (superNodeIdx1) {delete superNodeIdx1; superNodeIdx1=0;}
		
	}// for i
	
	
	// reconstruct the optimal match from stored best matches
	
	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
	Tree1->getRootID(rootid1, rootidx1);
	Tree2->getRootID(rootid2, rootidx2);

//	Tree1->getIndex(rootid1, rootidx1);
//	Tree2->getIndex(rootid2, rootidx2);

	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;

	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
	
	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
		printf("Root should have only one child\n");

	for (m=0; m<nodeNum1; m++)
	{
		V3DLONG idx;
	
		
		if (m==rootidx1) //add root, require root must be matched to root
			idx = rootidx2;
		else
			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];

		V3DLONG tmpidx1;
		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
		
		if (idx>=0)
		{
			matchingList[tmpidx1] = Tree2->node[idx].nid;		
//			matchingList[m] = Tree2->node[idx].nid;
			
		}	
		else
			matchingList[tmpidx1] = INVALID_VALUE;
//			matchingList[m] = INVALID_VALUE;
	}

//	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes

	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
	printf("simMeasure = %f\n", simMeasure);
	
	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}


//#ifdef DEBUG	
	
	//print similarity matrix
	printf("Similarity matrix\n");
	
	for (i=0; i<nodeNum1; i++)
	{
		for (j=0; j<nodeNum2; j++)
			printf("%f ", similarity1d[i*nodeNum2+j]);
		printf("\n");
	}
	
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
	
	if (removeTag1) {delete removeTag1; removeTag1=0;}
	if (removeTag2) {delete removeTag2; removeTag2=0;}
	
	
}



// match two trees using dynamic programming
// In an earlier version, lengthThre1 and lengthThre2 are the threshold of lengths the maximum and second maximum branches of node. 
// They determine which branching points will be kept in Tree1 and Tree2 for matching, if lengthThre1
// and lengthThre2 are big, only significant branching points will be matched; Otherwise small branching nodes or even all
// branching nodes will be matched
// In this version, lengthRatioThre is used to determie which branching nodes are kept as significant branching nodes. 
// properties of this function:
// 1. when lengthThre1 and lengthThre2 (lengthRatioThre in this version) are big, only match significant branching points, no continual points, leaf nodes 
//    will be matched (should set keepLeaf = 0)
// 2. however, features are computed based on original trees (not pruned), best matched sub-trees are computed on pruned 
//    trees, i.e., only those nodes belonging to the significant branching points are considered
// 3. to compensate the number of matches which will influence the similarity1d, use similarity1d normalized by the number
//    of matches to select the best submatch in sub-branches, but the overall similarity is still computed by accumulating
//    all sub-branches matches
// 4. The trick to make this function work are:
// 1) the trees need to appropriately pruned by setting appropriate lengthThre1 and lengthThre2 values, so that only significant 
//	  branching points are kept, those branching points in tiny sub-branches are removed
// 2) depthThre can not be set to too big, preferrablely 2, otherwise, the algorithm will favor matches close to leaf nodes,
//    making the number of matching nodes smaller (but are correct)
// 3) leaf nodes need to be removed, because the way the compute the lengthratio of leaf nodes are not appropriately defined (set to 0),
//	  so that leaf nodes have too much freedom to match to other nodes, which will influence the global matching, leaf nodes will be matched
//    after the significant branching points have been matched
// note that if keepLeaf is set to 0, leaf node will not be matched correctly, because the length ratio of any leaf node is
// set to 0, no matter where it sits on the tree, leaf nodes need to be matched after significant branching points are matched

//bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure, float lengthThre1, float lengthThre2)
bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, V3DLONG *&branchMatchingList, float &simMeasure, float *lengthRatioThre)
{
	V3DLONG i, j, m, n, p,q;
	float weight_lengthRatio = 1.0;
//	float weight_angle = 1.0;
//	float weight_lengthRatio = 0;
	float weight_angle = 0;
//	float lengthThre1 = 50;
//	float lengthThre2 = 10;
////	float lengthThre1 = 5;
////	float lengthThre2 = 5;
	
	// -----------------------------------------------------------------
	// generate trees only containing root, branching points, and leaf nodes
	// remove continual points
	// the input tree pointers are backed up in Tree1_old, Tree2_old
	// the trees after removing continual nodes are Tree1 and Tree2
	// -----------------------------------------------------------------	
	swcTree *Tree1_old =0, *Tree2_old = 0;
	swcTree *newTree1 =0, *newTree2 = 0;
	bool *removeNodeTag1=0, *removeNodeTag2=0;
	V3DLONG **removeBranchTag1=0, **removeBranchTag2=0;	
	bool keepLeaf=0;
	
	Tree1_old = Tree1;
	Tree2_old = Tree2;
	
////	Tree1->removeContinualNodes(newTree1, removeTag1);
//	Tree1->removeInsignificantNodes(lengthThre1, lengthThre2, newTree1, keepLeaf, removeNodeTag1, removeBranchTag1); // remove continual nodes, leaf nodes (if keepLeaf=0) , and small branching points, note that features will be still computed on the Tree1_old
	Tree1->removeInsignificantNodes(lengthRatioThre, newTree1, keepLeaf, removeNodeTag1, removeBranchTag1); // remove continual nodes, leaf nodes (if keepLeaf=0) , and small branching points, note that features will be still computed on the Tree1_old

	Tree1 = newTree1;
	newTree1 = 0;

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		printf("%d ", Tree1->node[i].nid);
	}
	printf("\n");

	for (i=0; i<Tree1->treeNodeNum; i++)
	{
		printf("%d ", Tree1->node[i].nid);
	
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			printf("%d ", removeBranchTag1[i][j]);
		printf("\n");
	}
	printf("\n");
	
	Tree1->writeSwcFile("Tree1.swc"); 
	Tree1->genGraphvizFile("Tree1.dot");
	Tree1_old->genGraphvizFile("Tree1_old.dot");
	
	
////	Tree2->removeContinualNodes(newTree2, removeTag2);
//	Tree2->removeInsignificantNodes(lengthThre1, lengthThre2, newTree2, keepLeaf, removeNodeTag2, removeBranchTag2); // remove continual nodes, leaf nodes (if keepLeaf=0), and small branching points
	Tree2->removeInsignificantNodes(lengthRatioThre, newTree2, keepLeaf, removeNodeTag2, removeBranchTag2); // remove continual nodes, leaf nodes (if keepLeaf=0), and small branching points

	Tree2 = newTree2;
	newTree2 = 0;
	
	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		printf("%d ", Tree2->node[i].nid);
	}
	printf("\n");

	for (i=0; i<Tree2->treeNodeNum; i++)
	{
		printf("%d ", Tree2->node[i].nid);
	
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			printf("%d ", removeBranchTag2[i][j]);
		printf("\n");
	}
	printf("\n");
	
	Tree2->writeSwcFile("Tree2.swc"); 
	Tree2->genGraphvizFile("Tree2.dot");
	Tree2_old->genGraphvizFile("Tree2_old.dot");

	
	// -----------------------------------------------------------------
	// get the number of nodes in the subtree rooted at each node
	// the memory of subTreeNodeNum is allocated inside getSubTreeNodeNum
	// -----------------------------------------------------------------	
	V3DLONG *subTreeNodeNum1 = 0;
	V3DLONG *subTreeNodeNum2 = 0;
	
	Tree2->getSubTreeNodeNum(subTreeNodeNum2); 	// these two lines can not reverse order?
	Tree1->getSubTreeNodeNum(subTreeNodeNum1); 

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
	V3DLONG depthThre = 2;
//	V3DLONG depthThre = 3;

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
	 matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
	
	for (i=0; i<Tree1_old->treeNodeNum; i++)
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
//		Tree1->getDirectChildren(61, childrenList1_Tree1, childrenNodeIdx1_Tree1, childrenNum1_Tree1); // get direct children for the current node in Tree1
		
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
		Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4);
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
			Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4);
			childrenNum2_Tree2_old = lengthratioNum2; // number of children in old Tree1
			
			float *simVector_lengthRatio = 0;			
//			printf("i=%d, j=%d, lengthratioNum1 = %d, lengthratioNum2=%d\n", i,j,lengthratioNum1, lengthratioNum2);
			simVector_lengthRatio = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
			
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
			
			V3DLONG *childrenOrder1_Tree1 = new V3DLONG [childrenNum1_Tree1_old];
			V3DLONG *childrenOrder2_Tree2 = new V3DLONG [childrenNum2_Tree2_old];
			
			for (m=0; m<childrenNum1_Tree1_old; m++)
				childrenOrder1_Tree1[m] = m;
			
			for (m=0; m<childrenNum2_Tree2_old; m++)
				childrenOrder2_Tree2[m] = m;
								
			//enumerate possible combinations
			V3DLONG N_num, R_num, *N_list, *R_list;
			
			if (childrenNum1_Tree1_old>childrenNum2_Tree2_old)
			{
				N_num = childrenNum1_Tree1_old;
				R_num = childrenNum2_Tree2_old;					
				N_list = childrenOrder1_Tree1;
				R_list = childrenOrder2_Tree2;
			}
			else
			{
				N_num = childrenNum2_Tree2_old;
				R_num = childrenNum1_Tree1_old;
				N_list = childrenOrder2_Tree2;
				R_list = childrenOrder1_Tree1;
			}
								
			V3DLONG num = 1; // number of possible combinations
			//compute num: C(m,n)*P(n) = m!/((m-n)!*n!)*n! = m!/(m-n)!;
			for (m=N_num; m>0; m--)
				num *= m;
			for (m=(N_num-R_num); m>0; m--)
				num /=m;
				
			V3DLONG *possibleMatchList = new V3DLONG [num*(R_num*2)]; 

			// assign values to possibleMatchList				
			if (childrenNum1_Tree1_old>childrenNum2_Tree2_old)
			{	
				for (n=0; n<num; n++)
				for (m=0; m<R_num; m++)
					possibleMatchList[n*(R_num*2)+2*m+1] = m;
			}
			else
			{
				for (n=0; n<num; n++)
				for (m=0; m<R_num; m++)
					possibleMatchList[n*(R_num*2)+2*m] = m;
			}
			
			// generate C(m,n) and then p(n) to assign remaining values in possibleMatchList
			V3DLONG *R2_list=new V3DLONG [R_num];
			V3DLONG cnt = -1;
			
			do
			{
				for (m=0; m<R_num; m++)
				{	
					//printf("%d ", R_list[m]);
					R2_list[m] = R_list[m];
				}
				//printf("\n");

				do
				{
					cnt++;
					if (childrenNum1_Tree1_old>childrenNum2_Tree2_old)
					{
						for (m=0; m<R_num; m++)
							possibleMatchList[cnt*(R_num*2)+2*m] = R2_list[m];
					}
					else
					{
						for (m=0; m<R_num; m++)
							possibleMatchList[cnt*(R_num*2)+2*m+1] = R2_list[m];
					}
					
				}while(next_permutation(R2_list,R2_list+R_num)); //next_permutation is defined in <algorithm.h>

			}
			while(next_combination(N_list,N_list+N_num,R_list, R_list+R_num)); // next_combination is defined in "./combination/combination.h"
			
			printf("nodeid1 = %d, nodeid2 = %d\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid);					
			printf("possibleMatchList:\n");
			
			for (m=0;m<num; m++)
			{
				for (n=0; n<R_num; n++)
					printf("%d, %d, ", possibleMatchList[m*(R_num*2) +2*n], possibleMatchList[m*(R_num*2) +2*n+1]);
				printf("\n");
			}
				
																																																				
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
					Tree1->getSubTree(childrenList1_Tree1[m], depthThre, subTree1, subTreeNodeIdx1);
					
					for (n=0; n<childrenNum2_Tree2; n++)
					{
						// find the best one in the subtree rooted at nodeidx2
						V3DLONG *subTreeNodeIdx2 = 0;
						swcTree *subTree2 = 0; 
						Tree2->getSubTree(childrenList2_Tree2[n], depthThre, subTree2, subTreeNodeIdx2);

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
							
							similarity1d_norm /= cntt;
							printf("similarity1d_norm = %f\n", similarity1d_norm);
							
							
							if (bestval<=similarity1d_norm)
							{
								bestval = similarity1d_norm;
								p1 = p; q1 = q;
							}
							
						}
						

						bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p1];
						bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q1];
						
						if (subTree2) {delete subTree2; subTree2=0;}
						if (subTreeNodeIdx2) {delete []subTreeNodeIdx2; subTreeNodeIdx2=0;}
								
					} // for n
				
					if (subTree1) {delete subTree1; subTree1=0;}
					if (subTreeNodeIdx1) {delete []subTreeNodeIdx1; subTreeNodeIdx1=0;}

				} // for m

				//print bestSbuMatch
				printf("bestSubMatch:\n");
				for (m=0; m<childrenNum1_Tree1; m++)
				for (n=0; n<childrenNum2_Tree2; n++)
					printf("m = %d, n = %d, bestSubMatch1 = %d, bestSubMatch2 = %d, nodeid1 = %d, nodeid2 = %d\n", m,n, bestSubMatch[m*childrenNum2_Tree2*2+n*2],  bestSubMatch[m*childrenNum2_Tree2*2+n*2+1],
					      Tree1->node[bestSubMatch[m*childrenNum2_Tree2*2+n*2]].nid, Tree2->node[bestSubMatch[m*childrenNum2_Tree2*2+n*2+1]].nid);
						  
						  
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
							tmpSimVal += similarity1d[ind1*nodeNum2+ind2];

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
				for (n=0; n<R_num; n++)
				{
					V3DLONG tmpidx =m*(R_num*2)+2*n; 
					V3DLONG idx1 = possibleMatchList[tmpidx];
					V3DLONG idx2 = possibleMatchList[tmpidx+1];
					tmpSimLengthRatio += simVector_lengthRatio[idx1*childrenNum2_Tree2_old+idx2];
				}
				
				tmpSimLengthRatio /= R_num; // normalize to balance length-ratio and angle
//					if (weight_lengthRatio!=0)
//						printf("local length ratio similarity1d = %f\n ", tmpSimLengthRatio);
				
				tmpSimVal += (weight_lengthRatio*tmpSimLengthRatio);
				
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
				}

				
			} //for (m=0; m<num; m++)


			//assign the best similarity 			
			similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]= bestSimVal;
			printf("bestSimVal = %f\n", bestSimVal);
			
			for (p=0; p<R_num; p++)
			{
				bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2] = possibleMatchList[best_m*R_num*2+p*2];
				bestBranchMatch[(V3DLONG)sortidx1[i+1]*nodeNum2*(MAX_CHILDREN_NUM*2)+(V3DLONG)sortidx2[j+1]*(MAX_CHILDREN_NUM*2)+p*2+1] = possibleMatchList[best_m*R_num*2+p*2+1];
				
//				printf("possibleMatchList = %d, %d\n",possibleMatchList[best_m*R_num*2+p*2], possibleMatchList[best_m*R_num*2+p*2+1]);
			}
			
			//assign the best mapping list if both nodes are non-leaf nodes, otherwise, mappingfunc1d has already been set
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
						
						mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
		//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
							
						//expand matching list of the children
						for (p = 0; p<nodeNum1; p++)
						{	
							
							if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
								(mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
									mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
									mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
							
						} //for p
					}  // if removeBranchTag1 ...
				} // for m 
			}
			
			if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
			if (possibleMatchList) {delete []possibleMatchList; possibleMatchList=0;}
			if (R2_list) {delete []R2_list; R2_list=0;}
			if (childrenOrder1_Tree1) {delete []childrenOrder1_Tree1; childrenOrder1_Tree1=0;}
			if (childrenOrder2_Tree2) {delete []childrenOrder2_Tree2; childrenOrder2_Tree2=0;}
				


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
		
		if (idx>=0)
		{
			matchingList[tmpidx1] = Tree2->node[idx].nid;		
			matchedNodeNum ++;			
		}	
		else
			matchingList[tmpidx1] = INVALID_VALUE;
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
		
	branchMatchingList = new V3DLONG [matchedNodeNum*(MAX_CHILDREN_NUM)*2];
	
	for (i=0; i<matchedNodeNum*(MAX_CHILDREN_NUM)*2; i++)
		branchMatchingList[i] = INVALID_VALUE;
		
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
		
			printf("nodeid1 = %d, nodeid2 = %d,           ", Tree1->node[m].nid, Tree2->node[idx].nid);

			V3DLONG *childrenList1=0, *childrenNodeIdx1=0, childrenNum1;			
			V3DLONG *childrenList2=0, *childrenNodeIdx2=0, childrenNum2;
			
			Tree1_old->getDirectChildren(Tree1->node[m].nid, childrenList1, childrenNodeIdx1, childrenNum1); 
			Tree2_old->getDirectChildren(Tree2->node[idx].nid, childrenList2, childrenNodeIdx2, childrenNum2); // get direct children for the current node in Tree1
			
			if ((childrenNum1>0) && (childrenNum2>0))
			{
				for (p = 0; p<MAX_CHILDREN_NUM; p++)
				{
						V3DLONG tmpidx1 = bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2)+idx*(MAX_CHILDREN_NUM*2)+p*2];
						V3DLONG tmpidx2 = bestBranchMatch[m*nodeNum2*(MAX_CHILDREN_NUM*2)+idx*(MAX_CHILDREN_NUM*2)+p*2+1];
						
						if ((tmpidx1!=INVALID_VALUE)&&(tmpidx2!=INVALID_VALUE))
						{
							branchMatchingList[cnt*MAX_CHILDREN_NUM*2+p*2+0] = tmpidx1;
							branchMatchingList[cnt*MAX_CHILDREN_NUM*2+p*2+1] = tmpidx2;
							
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
			
		}		
	}	
	


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
		
		if (matchingList[idx]>0)
			matchingList_new[i] = matchingList[idx];
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
//	if (removeBranchTag1) {delete2dpointer(removeBranchTag1, MAX_CHILDREN_NUM, Tree1->treeNodeNum);}
	if (removeBranchTag1_1d) {delete []removeBranchTag1; removeBranchTag1=0;}
	delete []removeBranchTag1_1d; removeBranchTag1_1d=0;

	V3DLONG *removeBranchTag2_1d = removeBranchTag2[0];
	if (removeBranchTag2_1d) {delete []removeBranchTag2; removeBranchTag2=0;}
	delete []removeBranchTag2_1d; removeBranchTag2_1d=0;
	
	if (bestBranchMatch) {delete []bestBranchMatch; bestBranchMatch=0;}
	
}

// tree matching using hierarchical DP scheme 
bool treeMatching_hierarchical(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
{


	// significant branching nodes matching
	
	float *lengthRatioThre = new float [2];
	V3DLONG i, j, k, m; 
	
	lengthRatioThre[0] = 0.2;
	lengthRatioThre[1] = 0.1;
	
	V3DLONG *branchMatchingList = 0;
	treeMatching_noContinualNodes_lengthRatioAngle_simDependent(Tree1, Tree2, matchingList, branchMatchingList, simMeasure, lengthRatioThre);
	
	
	// decompse each tree according to matched branching nodes
	V3DLONG *seedNodeID1 = 0, *seedNodeID2 =0;
	swcTree **subTrees1 =0, **subTrees2 = 0;
	V3DLONG *branchIdx1 = 0, *branchIdx2 = 0;
	V3DLONG subTreeNum1, subTreeNum2;
	
	// compute the number of matched branching ndoes
	V3DLONG seedNodeNum=0;	
	for(i=0; i<Tree1->treeNodeNum; i++)
	{	
		if ( matchingList[i]>0)
			seedNodeNum++;
	}
	
	seedNodeID1 = new V3DLONG [seedNodeNum];
	seedNodeID2 = new V3DLONG [seedNodeNum];
	
	j = 0;
	for(i=0; i<Tree1->treeNodeNum; i++)
	{	
		if ( matchingList[i]>0)
		{
			seedNodeID1[j] = Tree1->node[i].nid;
			seedNodeID2[j] = matchingList[i];
			printf("%d, %d\n", seedNodeID1[j], seedNodeID2[j]);
			j++;
		}
	}	
	
	printf("seedNodeNum = %d\n", seedNodeNum);
	
	Tree1->decompose(seedNodeID1, seedNodeNum, subTrees1, subTreeNum1, branchIdx1); // decompose the tree using matched significant branching nodes
	Tree2->decompose(seedNodeID2, seedNodeNum, subTrees2, subTreeNum2, branchIdx2);

	printf("subTreeNum1 = %d, subTreeNum2 = %d\n", subTreeNum1, subTreeNum2);


	// pair trees to be matched
	V3DLONG cnt = -1;
	V3DLONG *idx1 = new V3DLONG [seedNodeNum*(MAX_CHILDREN_NUM+1)]; 
	V3DLONG *idx2 = new V3DLONG [seedNodeNum*(MAX_CHILDREN_NUM+1)];
	
	for (i=0; i<seedNodeNum; i++)
	{
		idx1[i*(MAX_CHILDREN_NUM+1)] = -9999;
		
		j= 1;
		while ( branchIdx1[i*(MAX_CHILDREN_NUM+1)+j]!=INVALID_VALUE)
		{
			cnt++;
			idx1[i*(MAX_CHILDREN_NUM+1) + j] = cnt;
			j++;
		}
		for (k =j; k<MAX_CHILDREN_NUM+1; k++)
			idx1[i*(MAX_CHILDREN_NUM+1)+k] = -9999;
	}
		
	cnt = -1;
	for (i=0; i<seedNodeNum; i++)
	{
		idx2[i*(MAX_CHILDREN_NUM+1)] = -9999;
		
		j= 1;
		while ( branchIdx2[i*(MAX_CHILDREN_NUM+1)+j]!=INVALID_VALUE)
		{
			cnt++;
			idx2[i*(MAX_CHILDREN_NUM+1) + j] = cnt;
			j++;
		}
		for (k =j; k<MAX_CHILDREN_NUM+1; k++)
			idx2[i*(MAX_CHILDREN_NUM+1)+k] = -9999;
	}
	
	V3DLONG subTrees1_idx, subTrees2_idx;

	for (i=0; i<seedNodeNum; i++)
	{
//		j= 0;
//		while (branchMatchingList[i*MAX_CHILDREN_NUM*2+j]!=INVALID_VALUE)

		j= 1;
		while (branchIdx1[i*(MAX_CHILDREN_NUM+1)+j]!=INVALID_VALUE)
		{
			printf("i=%d, j=%d\n", i,j);
			subTrees1_idx = idx1[i*(MAX_CHILDREN_NUM+1)+j]; //get index of the tree
			subTrees2_idx = INVALID_VALUE;
			
			k = 0;
			while (k<MAX_CHILDREN_NUM)
			{
				if (branchMatchingList[i*MAX_CHILDREN_NUM*2+2*k] == branchIdx1[i*(MAX_CHILDREN_NUM+1)+j])
				{
					V3DLONG tmp = branchMatchingList[i*MAX_CHILDREN_NUM*2+2*k+1];
					
					m = 0;
					while (m<MAX_CHILDREN_NUM)
					{
						if (tmp == branchIdx2[i*(MAX_CHILDREN_NUM+1)+m])
						{
							subTrees2_idx = idx2[i*(MAX_CHILDREN_NUM+1)+m];
							break;
						}
						else 
							m++;
					}
					
					break;
				}
				else
					k++;
			} // while k
			
			printf("subTrees1_idx = %d, subTrees2_idx = %d\n",  subTrees1_idx, subTrees2_idx);

			// sub-branch matching			
			if (subTrees2_idx!=INVALID_VALUE)
			{
			  	V3DLONG *branchMatchingList_local = 0;
				V3DLONG *matchingList_local = 0;
				float simMeasure_local;
				
				if ((subTrees1[subTrees1_idx]->treeNodeNum<5) || (subTrees2[subTrees2_idx]->treeNodeNum<5))
					lengthRatioThre[0] = lengthRatioThre[1] = 0;
					
			 	treeMatching_noContinualNodes_lengthRatioAngle_simDependent(subTrees1[subTrees1_idx], subTrees2[subTrees2_idx], matchingList_local, branchMatchingList_local, simMeasure_local, lengthRatioThre);

				// combine global matching list and local matching list
				
				for (m=0; m<subTrees1[subTrees1_idx]->treeNodeNum; m++)
				{
					if (matchingList_local[m]>0)
					{
						V3DLONG idx1, idx2;
						Tree1->getIndex(subTrees1[subTrees1_idx]->node[m].nid, idx1);
						Tree2->getIndex(matchingList_local[m], idx2);
						
//						matchingList[idx] = subTrees2[subTrees2_idx]->node[matchingList_local[m]].nid;
						matchingList[idx1] = Tree2->node[idx2].nid;

					}
				}
				
				// delete pointers
				if (branchMatchingList_local) {delete []branchMatchingList_local; branchMatchingList_local=0;}
				if (matchingList_local) {delete []matchingList_local; matchingList_local = 0;}
			}
				
			j++;
			printf("j=%d\n",j);
		} // while 
	}// for i
	
	
	//delete pointers related to decompose function
	if (seedNodeID1) {delete []seedNodeID1; seedNodeID1=0;}
	if (seedNodeID2) {delete []seedNodeID2; seedNodeID2=0;}
	if (branchIdx1) {delete []branchIdx1; branchIdx1=0;}
	if (branchIdx2) {delete []branchIdx2; branchIdx2=0;}
	
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
	
//	swcTree *subTrees1_1d = subTrees1[0];
//	if (subTrees1) {delete subTrees1;subTrees1=0;}
//	delete subTrees1_1d; subTrees1_1d=0;
//
//	swcTree *subTrees2_1d = subTrees2[0];
//	if (subTrees2) {delete subTrees2;subTrees2=0;}
//	delete subTrees2_1d; subTrees2_1d=0;

	// delete pointer related to treeMatching_noContinualNodes_lengthRatioAngle_simDependent
	if (lengthRatioThre) {delete []lengthRatioThre; lengthRatioThre=0;}
	if (branchMatchingList) {delete []branchMatchingList; branchMatchingList=0;}
	
}

//	Tree1->treeDecompose(matchingList, );


//
//// match two trees using dynamic programming
//// Different from treeMatching_noContinualNodes_lengthRatio_simDependent, angles are also considered
//// root, branching nodes, leaf nodes will find matches, continual nodes will not find matches
//
//bool treeMatching_noContinualNodes_lengthRatioAngle_simDependent(swcTree *Tree1, swcTree *Tree2, V3DLONG *&matchingList, float &simMeasure)
//{
//	V3DLONG i, j, m, n, p,q;
//	float weight_lengthRatio = 1.0;
////	float weight_angle = 1.0;
////	float weight_lengthRatio = 0;
//	float weight_angle = 0;
//	float lengthThre1 = 50;
//	float lengthThre2 = 10;
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
////	Tree1->removeContinualNodes(newTree1, removeTag1);
//	Tree1->removeInsignificantNodes(lengthThre1, lengthThre2, newTree1, removeTag1); // remove continual nodes, leaf nodes, and small branching points, note that features will be still computed on the Tree1_old
//	Tree1 = newTree1;
//	newTree1 = 0;
//
//	for (i=0; i<Tree1->treeNodeNum; i++)
//	{
//		printf("%d ", Tree1->node[i].nid);
//	}
//	printf("\n");
//	Tree1->writeSwcFile("Tree1.swc"); 
//	Tree1->genGraphvizFile("Tree1.dot");
//	Tree1_old->genGraphvizFile("Tree1_old.dot");
//	
////	Tree2->removeContinualNodes(newTree2, removeTag2);
//	Tree2->removeInsignificantNodes(lengthThre1, lengthThre2, newTree2, removeTag2); // remove continual nodes, leaf nodes, and small branching points
//	Tree2 = newTree2;
//	newTree2 = 0;
//	
//	for (i=0; i<Tree2->treeNodeNum; i++)
//	{
//		printf("%d ", Tree2->node[i].nid);
//	}
//	printf("\n");
//	
//	Tree2->writeSwcFile("Tree2.swc"); 
//	Tree2->genGraphvizFile("Tree2.dot");
//	Tree2_old->genGraphvizFile("Tree2_old.dot");
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
//	float *similarity1d = new float [nodeNum1*nodeNum2]; // #(branching nodes + root +leaf nodes in Tree 1) *#(branching nodes + root + leaf nodes in Tree 2)
//	if (!similarity1d)
//	{
//		printf("Fail to allocate memory for similarity1d. \n");
//		return false;
//	}
//	
//	for (m=0; m<nodeNum1; m++)
//	for (n=0; n<nodeNum2; n++)
//	{
//////		similarity1d[m*nodeNum2 + n] =  INVALID_VALUE;
////		similarity1d[m*nodeNum2 + n] =  -1;
//		similarity1d[m*nodeNum2 + n] =  0;
//	}
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
//	for (n=0; n<nodeNum2; n++)
//	for (p=0; p<nodeNum1; p++)
//		mappingfunc1d[m*nodeNum1*nodeNum2 + n*nodeNum1 + p] = INVALID_VALUE;
//	
//	// initialize matching list, the size is determined by the original Tree1, i.e., Tree1_old
//	 matchingList = new V3DLONG [Tree1_old->treeNodeNum]; //indicating for each node in tree1, the id# of the matching point in tree2
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
////		V3DLONG *childrenList1_Tree1_old=0, *childrenNodeIdx1_Tree1_old=0, childrenNum1_Tree1_old;
////		Tree1_old->getDirectChildren(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, childrenList1_Tree1_old, childrenNodeIdx1_Tree1_old, childrenNum1_Tree1_old); // get direct children for the current node in Tree1
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
//		// compute branch length ratio
//		float *lengthratio1=0;
//		unsigned char lengthratioNum1;
//		Tree1_old->computeBranchLengthRatio(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, lengthratio1, lengthratioNum1, 4);
//
//		// compute the angle metrics between branches 
//		V3DLONG branchNum1; //branchNum1 should have the same value as lengthratioNum1
//		float *angles1;
//		Tree1_old->computeBranchingAngles(Tree1->node[(V3DLONG)sortidx1[i+1]].nid, branchNum1, angles1);
//
//#ifdef DEBUG
//		printf("branchNum1 = %d, lengthratioNum1 = %d\n", branchNum1, lengthratioNum1);
//		for (m=0; m<branchNum1*(branchNum1+1)/2; m++)
//			printf("%f ", angles1[m]);
//		printf("\n");
//#endif
//
//
//		// find candidate nodes in Tree2 that can be matched to the current node in tree1
//		
//		for (j=0; j<nodeNum2; j++) 
//		{
//
////			printf("i=%d,j=%d\n", i,j);	
////			printf("nodeid1 = %d, nodeid2 = %d, sortidx1 = %f, sortidx2 = %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid, sortidx1[i+1], sortidx2[j+1]);
//			
//			V3DLONG *childrenList2_Tree2=0, *childrenNodeIdx2_Tree2=0, childrenNum2_Tree2;
//			Tree2->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2, childrenNodeIdx2_Tree2, childrenNum2_Tree2); // get direct children for the current node in Tree1
//			
////			V3DLONG *childrenList2_Tree2_old=0, *childrenNodeIdx2_Tree2_old=0, childrenNum2_Tree2_old;
////			Tree2_old->getDirectChildren(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, childrenList2_Tree2_old, childrenNodeIdx2_Tree2_old, childrenNum2_Tree2_old); // get direct children for the current node in Tree1
//
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
//			// compute the legnth ratio metrics of each branch based on which similarity is further computed
//			float *lengthratio2=0;
//			unsigned char lengthratioNum2;
//			Tree2_old->computeBranchLengthRatio(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, lengthratio2, lengthratioNum2, 4);
//			
//			float *simVector_lengthRatio = 0;			
////			printf("i=%d, j=%d, lengthratioNum1 = %d, lengthratioNum2=%d\n", i,j,lengthratioNum1, lengthratioNum2);
//			simVector_lengthRatio = calsim_allbranch_length_ratio(lengthratio1, lengthratio2, lengthratioNum1, lengthratioNum2);
//			
//			// compute the angle metrics between branches based on which similarity is further computed
//			V3DLONG branchNum2; //branchNum2 should have the same value as lengthratioNum2
//			float *angles2;
//			Tree2_old->computeBranchingAngles(Tree2->node[(V3DLONG)sortidx2[j+1]].nid, branchNum2, angles2);
//
//#ifdef DEBUG
//		printf("branchNum2 = %d, lengthratioNum2 = %d\n", branchNum2, lengthratioNum2);
//		for (m=0; m<branchNum2*(branchNum2+1)/2; m++)
//			printf("angles2 = %f ", angles2[m]);
//		printf("\n");
//#endif
//			
//			float *simVector_angle_parent = 0, *simVector_angle_children = 0;			
////			printf("i=%d, j=%d, branchNum1 = %d, branchNum2=%d\n", i,j,branchNum1, branchNum2);
//			calsim_allbranch_angles(angles1, angles2, branchNum1, branchNum2, simVector_angle_parent, simVector_angle_children);		
//			
////			printf("simVector_angle_parent=%d, simVector_angle_children= %d\n", simVector_angle_parent, simVector_angle_children);
//			
//#ifdef DEBUG
//			for (m=0; m<lengthratioNum1; m++)
//			{
//				for (n=0; n<lengthratioNum2; n++)
//					printf("%f ", simVector_angle_parent[m*lengthratioNum2+n]);
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
//			// find the best matching for subtree by enumeration	
//			
//			if ((childrenNum1_Tree1==0)||(childrenNum2_Tree2==0)) // if one of the nodes is leaf node, only compute the local similarity of the two nodes without adding the similarity of subtree nodes 
//			  
//			{
//				// length-ratio similarity
//				float tmp=0;
//				V3DLONG len = lengthratioNum1*lengthratioNum2;
//				for (m=0; m<len; m++) // note lengthratioNum=1 for leaf nodes
//					tmp += simVector_lengthRatio[m];
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] += (weight_lengthRatio*tmp/len); 	// update similarity1d						
//		
////				printf("nodeid1 = %d, nodeid2 = %d, similarity1d = %f\n", Tree1->node[(V3DLONG)sortidx1[i+1]].nid,Tree2->node[(V3DLONG)sortidx2[j+1]].nid,similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//
//				// compute angle similarity, no need to figure out the mapping of sub-branches of the two ndoes, as at least one of the nodes is leaf				
//				V3DLONG lenp = 0, lenc=0;
//				if ((childrenNum1_Tree1==0)&&(childrenNum2_Tree2==0))
//				{
//					lenp = 1;
//					lenc = 1;
//				}	
//				
//				if ((childrenNum1_Tree1==0)&&(childrenNum2_Tree2!=0))
//				{	
//					lenp = branchNum2;
//					lenc = branchNum2*(branchNum2-1)/2;
//				}
//					
//				if ((childrenNum1_Tree1!=0)&&(childrenNum2_Tree2==0))
//				{
//					lenp = branchNum1;
//					lenc = branchNum1*(branchNum1-1)/2;
//				}
//					
//				tmp =0;
//				for (m=0; m<lenp; m++)
//					tmp +=simVector_angle_parent[m];
//
//				for (m=0; m<lenc; m++)
//					tmp += simVector_angle_children[m];
//					
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] +=(weight_angle*tmp/(lenp+lenc));		
//
////				float nn = (rand() % 201-100.0)/100.0; //generate a random number between [-1,1]
////				printf("***nn=%f\n", nn);
////				
//////				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = 100*nn; 
////				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]] = nn; 
//
//
////				printf("At least one of the node is leaf, ");
////				printf("similarity1d = %f\n", similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]);
//												
//			}
//			else // none of the two nodes are leaf nodes in the pruned tree, they are either root or branching nodes
//			{
//				
//				// compute all possible combinations and save it in possibleMatchList
//				
//				V3DLONG *childrenOrder1_Tree1 = new V3DLONG [childrenNum1_Tree1];
//				V3DLONG *childrenOrder2_Tree2 = new V3DLONG [childrenNum2_Tree2];
//				
//				for (m=0; m<childrenNum1_Tree1; m++)
//					childrenOrder1_Tree1[m] = m;
//				
//				for (m=0; m<childrenNum2_Tree2; m++)
//					childrenOrder2_Tree2[m] = m;
//									
//				//enumerate possible combinations
//				V3DLONG N_num, R_num, *N_list, *R_list;
//				
//				if (childrenNum1_Tree1>childrenNum2_Tree2)
//				{
//					N_num = childrenNum1_Tree1;
//					R_num = childrenNum2_Tree2;					
//					N_list = childrenOrder1_Tree1;
//					R_list = childrenOrder2_Tree2;
//				}
//				else
//				{
//					N_num = childrenNum2_Tree2;
//					R_num = childrenNum1_Tree1;
//					N_list = childrenOrder2_Tree2;
//					R_list = childrenOrder1_Tree1;
//				}
//									
//				V3DLONG num = 1; // number of possible combinations
//				//compute num: C(m,n)*P(n) = m!/((m-n)!*n!)*n! = m!/(m-n)!;
//				for (m=N_num; m>0; m--)
//					num *= m;
//				for (m=(N_num-R_num); m>0; m--)
//					num /=m;
//					
//				V3DLONG *possibleMatchList = new V3DLONG [num*(R_num*2)]; 
//
//				// assign values to possibleMatchList				
//				if (childrenNum1_Tree1>childrenNum2_Tree2)
//				{	
//					for (n=0; n<num; n++)
//					for (m=0; m<R_num; m++)
//						possibleMatchList[n*(R_num*2)+2*m+1] = m;
//				}
//				else
//				{
//					for (n=0; n<num; n++)
//					for (m=0; m<R_num; m++)
//						possibleMatchList[n*(R_num*2)+2*m] = m;
//				}
//				
//				// generate C(m,n) and then p(n) to assign remaining values in possibleMatchList
//				V3DLONG *R2_list=new V3DLONG [R_num];
//				V3DLONG cnt = -1;
//				
//				do
//				{
//					for (m=0; m<R_num; m++)
//					{	
//						//printf("%d ", R_list[m]);
//						R2_list[m] = R_list[m];
//					}
//					//printf("\n");
//
//					do
//					{
//						cnt++;
//						if (childrenNum1_Tree1>childrenNum2_Tree2)
//						{
//							for (m=0; m<R_num; m++)
//								possibleMatchList[cnt*(R_num*2)+2*m] = R2_list[m];
//						}
//						else
//						{
//							for (m=0; m<R_num; m++)
//								possibleMatchList[cnt*(R_num*2)+2*m+1] = R2_list[m];
//						}
//						
//					}while(next_permutation(R2_list,R2_list+R_num)); //next_permutation is defined in <algorithm.h>
//
//				}
//				while(next_combination(N_list,N_list+N_num,R_list, R_list+R_num)); // next_combination is defined in "./combination/combination.h"
//								
//		//		find the best matching nodes in the subtree rooted at each pair-wise children nodes in tree1 and tree2
//		//		initialize bestSubMatch
//				V3DLONG *bestSubMatch = new V3DLONG [childrenNum1_Tree1*childrenNum2_Tree2*2];
//
//				for (m=0; m<childrenNum1_Tree1; m++)
//				for (n=0; n<childrenNum2_Tree2; n++)
//				{
//					bestSubMatch[m*childrenNum2_Tree2*2+n*2] = INVALID_VALUE;
//					bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = INVALID_VALUE;
//				}
//				
//				// find the best match
//				for (m=0; m<childrenNum1_Tree1; m++)
//				{
////					Tree1->getIndex(childrenList1_Tree1[m], nodeidx1); 
//
//					//get all children rooted at childrenList1_Tree1[m], 	
//					V3DLONG *subTreeNodeIdx1 = 0;
//					swcTree *subTree1 = 0; 
//					Tree1->getSubTree(childrenList1_Tree1[m], depthThre, subTree1, subTreeNodeIdx1);
//					
//					for (n=0; n<childrenNum2_Tree2; n++)
//					{
////						Tree2->getIndex(childrenList2_Tree2[n], nodeidx2);
//						
//						// find the best one in the subtree rooted at nodeidx2
//						V3DLONG *subTreeNodeIdx2 = 0;
//						swcTree *subTree2 = 0; 
//						Tree2->getSubTree(childrenList2_Tree2[n], depthThre, subTree2, subTreeNodeIdx2);
//
//						// find the best match with highest similarity in the subtree rooted at childrenList1_Tree1[m] and childreList2_Tree2[n]						
//						float bestval = INVALID_VALUE;
//						V3DLONG p1, q1;
//						
//						for (p=0; p<subTree1->treeNodeNum; p++)
//						for (q=0; q<subTree2->treeNodeNum; q++)
//							if (bestval<similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]])
//							{
//								bestval = similarity1d[subTreeNodeIdx1[p]*nodeNum2+subTreeNodeIdx2[q]];
//								p1 = p; q1 = q;
//							}
//						
//						
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2] = subTreeNodeIdx1[p1];
//						bestSubMatch[m*childrenNum2_Tree2*2+n*2+1] = subTreeNodeIdx2[q1];
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
//				// compute for each match the best similarity score and corresponding match
//				
//				float bestSimVal = INVALID_VALUE;
//				V3DLONG best_m = 0;
//				
//				for (m=0; m<num; m++) // for all possible combinations
//				{
//					float tmpSimVal=0;
//					
//					// subtree similarity
//					for (n=0; n<R_num; n++)
//					{
//						V3DLONG tmpidx =m*(R_num*2)+2*n; 
////						V3DLONG idx1 = childrenNodeIdx1_Tree1[possibleMatchList[tmpidx]];
////						V3DLONG idx2 = childrenNodeIdx2_Tree2[possibleMatchList[tmpidx+1]];
////						tmpSimVal += similarity1d[idx1*nodeNum2+idx2];
//
//						V3DLONG idx1 = possibleMatchList[tmpidx];
//						V3DLONG idx2 = possibleMatchList[tmpidx+1];
//
//						V3DLONG ind1 = bestSubMatch[idx1*childrenNum2_Tree2*2+idx2*2];
//						V3DLONG ind2 = bestSubMatch[idx1*childrenNum2_Tree2*2+idx2*2+1];
//						
//						tmpSimVal += similarity1d[ind1*nodeNum2+ind2];
//
//					}
//					
////					printf("subtree similarity1d = %f, ", tmpSimVal);
//					
//					// plus length-ratio similarity
//					float tmpSimLengthRatio=0;
//					for (n=0; n<R_num; n++)
//					{
//						V3DLONG tmpidx =m*(R_num*2)+2*n; 
//						V3DLONG idx1 = possibleMatchList[tmpidx];
//						V3DLONG idx2 = possibleMatchList[tmpidx+1];
//						tmpSimLengthRatio += simVector_lengthRatio[idx1*childrenNum2_Tree2+idx2];
//					}
//					
//					tmpSimLengthRatio /= R_num; // normalize to balance length-ratio and angle
////					if (weight_lengthRatio!=0)
////						printf("local length ratio similarity1d = %f\n ", tmpSimLengthRatio);
//					
//					tmpSimVal += (weight_lengthRatio*tmpSimLengthRatio);
//					
//					// plus angle (parent-path) similarity
//					float tmpSimAngle = 0;
//					for (n=0; n<R_num; n++)
//					{
//						V3DLONG tmpidx =m*(R_num*2)+2*n; 
//						V3DLONG idx1 = possibleMatchList[tmpidx];
//						V3DLONG idx2 = possibleMatchList[tmpidx+1];
//						
//						tmpSimAngle += simVector_angle_parent[idx1*childrenNum2_Tree2 + idx2];
//					}
//					
//					// plus angle (children-path) similarity	
//
//					V3DLONG len = childrenNum2_Tree2*(childrenNum2_Tree2-1)/2;
//					V3DLONG ttmp;
//									
//					for (p=0; p<R_num-1; p++)
//					for (q=p+1; q<R_num; q++)
//					{
//						V3DLONG tmpidxp =m*(R_num*2)+2*p; 
//						V3DLONG tmpidxq =m*(R_num*2)+2*q; 
//						
//						// compute idx1 and idx2
//						V3DLONG idx1 = 0, idx2 = 0;
//						for (V3DLONG s=0; s< possibleMatchList[tmpidxp]; s++)
//							idx1 += (childrenNum1_Tree1-(s+1));
//
////						idx1 +=  (possibleMatchList[tmpidxq] - possibleMatchList[tmpidxp]-1);
//
//						ttmp = possibleMatchList[tmpidxq] - possibleMatchList[tmpidxp];
//						if (ttmp<0)
//							ttmp = -ttmp;							
//						idx1 +=  (ttmp-1);
//							
//	
//						for (V3DLONG s=0; s< possibleMatchList[tmpidxp+1]; s++)
//							idx2 += (childrenNum2_Tree2-(s+1));
//						
//						ttmp = possibleMatchList[tmpidxq+1] - possibleMatchList[tmpidxp+1];
//						if (ttmp<0)
//							ttmp = -ttmp;							
//						idx2 +=  (ttmp-1);
//												
//						tmpSimAngle += simVector_angle_children[idx1*len + idx2];
//					}
//					
//					tmpSimAngle /= (R_num*(R_num+1)/2); // normalize to balance length-ratio and angle
////					printf("tmpSimLengthRatio = %f, tmpSimAngle = %f, R_num= %d\n", tmpSimLengthRatio, tmpSimAngle, R_num);
//
////					if (weight_angle!=0)
////						printf("local angle similarity1d = %f\n ", tmpSimAngle);
//					
//					tmpSimVal += (weight_angle*tmpSimAngle);
//					
////					float nn = (rand() % 201-100.0)/100.0;
////					printf("nn = %f\n", nn);
////					
//////					tmpSimVal += (100*nn);	
////					tmpSimVal += nn;	
//									
//					if (bestSimVal<tmpSimVal)
//					{
//						bestSimVal = tmpSimVal;
//						best_m = m;
//					}
//					
//				} //for (m=0; m<num; m++)
//
//				//assign the best similarity 			
//				similarity1d[(V3DLONG)sortidx1[i+1]*nodeNum2+(V3DLONG)sortidx2[j+1]]= bestSimVal;
//
//				//assign the best mapping list
//				for (m=0; m<R_num; m++)
//				{
//					
//					p = possibleMatchList[best_m*(R_num*2)+2*m];
//					q = possibleMatchList[best_m*(R_num*2)+2*m+1];
//					
//					// and the best matched sub-branch
//					V3DLONG ind1 = bestSubMatch[p*childrenNum2_Tree2*2+q*2];
//					V3DLONG ind2 = bestSubMatch[p*childrenNum2_Tree2*2+q*2+1];
//					
//					mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + ind1] = ind2;							
//	//							mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + bestSubMatch[m*childrenNum2*2+n*2]] = bestSubMatch[m*childrenNum2*2+n*2+1];
//						
//					//expand matching list of the children
//					for (p = 0; p<nodeNum1; p++)
//					{	
//						
//						if ((mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p]==INVALID_VALUE)&&
//							(mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p]>=0))
//								mappingfunc1d[(V3DLONG)sortidx1[i+1]*nodeNum2*nodeNum1+(V3DLONG)sortidx2[j+1]*nodeNum1 + p] = 
//								mappingfunc1d[ind1*nodeNum2*nodeNum1+ind2*nodeNum1 + p];
//						
//					} //for p
//				} // for m
//				
//				if (bestSubMatch) {delete []bestSubMatch; bestSubMatch = 0;}
//				if (possibleMatchList) {delete []possibleMatchList; possibleMatchList=0;}
//				if (R2_list) {delete []R2_list; R2_list=0;}
//				if (childrenOrder1_Tree1) {delete []childrenOrder1_Tree1; childrenOrder1_Tree1=0;}
//				if (childrenOrder2_Tree2) {delete []childrenOrder2_Tree2; childrenOrder2_Tree2=0;}
//				
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
//
//			if (childrenList2_Tree2) {delete childrenList2_Tree2; childrenList2_Tree2=0;}
//			if (childrenNodeIdx2_Tree2) {delete childrenNodeIdx2_Tree2; childrenNodeIdx2_Tree2=0;}
//			
//		} //for (j=0; j<nodeNum2; j++)
//		
//
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
////					printf("       nodeid1 = %d is matched to nodeid2 = %d\n", Tree1->node[p].nid, Tree2->node[q].nid);
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
//
//	if (lengthratio1) {delete lengthratio1; lengthratio1=0;}
//
//
//	if (childrenList1_Tree1) {delete childrenList1_Tree1; childrenList1_Tree1=0;}
//	if (childrenNodeIdx1_Tree1) {delete childrenNodeIdx1_Tree1; childrenNodeIdx1_Tree1=0;}
//			
////	// reconstruct the optimal match from stored best matches up to now
////
////	for (i=0; i<Tree1_old->treeNodeNum; i++)
////		matchingList[i] = INVALID_VALUE; //need to check i
////	
////	V3DLONG rootidx1, rootidx2, rootid1, rootid2;
////	Tree1->getRootID(rootid1, rootidx1);
////	Tree2->getRootID(rootid2, rootidx2);
////
//////	Tree1->getIndex(rootid1, rootidx1);
//////	Tree2->getIndex(rootid2, rootidx2);
////
////	V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
////	V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
////
////	Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
////	Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
////	
////	if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
////		printf("Root should have only one child\n");
////
////	V3DLONG best_n = 0;
////
////	float bestval = INVALID_VALUE;
////	
////	for (n=1; n<nodeNum2; n++) // do not include root
////		if (bestval < similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + n])
////		{
////			bestval = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + n];
////			best_n = n;
////		}
////	printf("best_n = %d\n", best_n);
////
////	for (m=0; m<nodeNum1; m++)
////	{
////		V3DLONG idx;
////	
////		
////		if (m==rootidx1) //add root, require root must be matched to root
////			idx = rootidx2;
////		else
////		{
//////			idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];				
////		   idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + best_n*nodeNum1 + m];
////		}
////		V3DLONG tmpidx1;
////		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
////		
////		if (idx>=0)
////		{
////			matchingList[tmpidx1] = Tree2->node[idx].nid;		
//////			matchingList[m] = Tree2->node[idx].nid;
////			
////		}	
////		else
////			matchingList[tmpidx1] = INVALID_VALUE;
//////			matchingList[m] = INVALID_VALUE;
////	}
////
////
////	FILE *file;	
////	string outfilename;	
////	
//////	file = fopen((char *)(outfilename+).c_str(), "wt");
////	file = fopen("res.txt", "wt");
////	
////	if (file == NULL)
////	{
////		printf("error to open file\n");
////		return 0; 
////	}
////		
////	for (int i=0; i<Tree1->treeNodeNum; i++)
////	{
//////		fprintf(file, "%d %d\n", i, matchingList[i]);
////		fprintf(file, "%d %d\n", Tree1->node[i].nid, matchingList[i]);
////
////	}
////	fclose(file);
//	
////	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
////	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
////	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
////	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}			
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
////	simMeasure = similarity1d[Tree1->node[rootChildrenNodeIdx1[0]].nid*nodeNum2+Tree2->node[rootChildrenNodeIdx2[0]].nid]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//
////	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + rootChildrenNodeIdx2[0]]; //note that the similarity between root is not involved, similarity1d is computed based on branchng nodes
//	simMeasure = similarity1d[rootChildrenNodeIdx1[0]*nodeNum2 + best_n];
//	printf("simMeasure = %f\n", simMeasure);
//
//	if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
//	if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
//	if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
//	if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}
//
////
////	// temporary
////	V3DLONG rootid1, rootid2, rootidx1, rootidx2;
////	rootid1 = 229;	
////	rootid2 = 37;
////	
////	Tree1->getIndex(rootid1, rootidx1);
////	Tree2->getIndex(rootid2, rootidx2);
////	
////	printf("rootidx1 = %d, rootidx2 = %d\n", rootidx1, rootidx2);
////	
////	simMeasure = similarity1d[rootidx1*nodeNum2+rootidx2];
////	printf("simMeasure = %f\n", simMeasure);
////	
////	for (m=0; m<nodeNum1; m++)
////	{
////		V3DLONG idx;
//////		if (nodeNumThre == 0)
//////			idx = mappingfunc1d[rootidx1*nodeNum2*nodeNum1 + rootidx2*nodeNum1 + m];
////	
//////		if (nodeNumThre == 2)
//////		{   
////			V3DLONG *rootChildrenID1, *rootChildrenNodeIdx1, rootChildrenNum1;
////			V3DLONG *rootChildrenID2, *rootChildrenNodeIdx2, rootChildrenNum2;
////			
////			if (m==rootidx1) //add root
////				idx = rootidx2;
////			else
////			{
////				Tree1->getDirectChildren(rootid1, rootChildrenID1, rootChildrenNodeIdx1, rootChildrenNum1);
////				Tree2->getDirectChildren(rootid2, rootChildrenID2, rootChildrenNodeIdx2, rootChildrenNum2);
////				
////				if ((rootChildrenNum1!=1)||(rootChildrenNum2!=1))
////					printf("Root should have only one child\n");
////				
////	//			Tree1->getIndex(Tree1->node[rootidx1].nid, tmpidx1);
////	//			Tree2->getIndex(Tree2->node[rootidx2].nid, tmpidx2);
////				
////				idx = mappingfunc1d[rootChildrenNodeIdx1[0]*nodeNum2*nodeNum1 + rootChildrenNodeIdx2[0]*nodeNum1 + m];
////			}
////			
////			if (rootChildrenID1) {delete []rootChildrenID1; rootChildrenID1 = 0;}
////			if (rootChildrenID2) {delete []rootChildrenID2; rootChildrenID2 = 0;}
////			if (rootChildrenNodeIdx1) {delete []rootChildrenNodeIdx1; rootChildrenNodeIdx1 = 0;}
////			if (rootChildrenNodeIdx2) {delete []rootChildrenNodeIdx2; rootChildrenNodeIdx2 = 0;}
////
//////		}
////
////		V3DLONG tmpidx1;
////		Tree1_old->getIndex(Tree1->node[m].nid, tmpidx1);
////		
////		if (idx>=0)
////		{
////			matchingList[tmpidx1] = Tree2->node[idx].nid;
//////			printf("%f, %d, %d\n", calsim_branch_length_ratio(Tree1, Tree2, Tree1->node[m].nid, Tree2->node[idx].nid), Tree1->node[m].nid, Tree2->node[idx].nid);
////			
////		}	
////		else
////			matchingList[tmpidx1] = INVALID_VALUE;
////	}
////
//
//	
//
//
////#ifdef DEBUG	
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




