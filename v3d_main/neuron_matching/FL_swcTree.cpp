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




// define swc tree class
// F. Long
// 20090108

#undef DEBUG_TREE

#include "../basic_c_fun/basic_memory.cpp" 
#include "../cellseg/FL_sort.h"
#include "FL_swcTree.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265
	
//read swc file into a swcTree object
// tagprint=1: print the read in content, tagprint =0: do not print

bool swcTree::readSwcFile(char *infilename, swcTree *&Tree, bool tagprint)
{
	FILE *file=0;
	char buffer[1001];// assumming each line has at most 1000 characters
	V3DLONG fieldnum = 7; //7 fields in swc file
	V3DLONG lineNum = 0;
	float *tmpVector = new float [7];
	
	swcTree *tmpTree = new swcTree(1000);
	
//    dataArray = new float [1000*fieldnum]; // assuming 1000 lines and each line has fieldnum of iterms
	
	file = fopen(infilename, "rt");
	
	if (file==NULL)
		printf("Error opening the file\n");
		
	if (fgets(buffer,1000,file) == NULL) //until 999 characters have been read or either a newline or a the End-of-File is reached, whichever comes first.
	{
		printf("The file is empty!\n");
			return false;
	}
	
	do
	{
		lineNum++;
		if (sscanf(buffer,"%g %g %g %g %g %g %g", tmpVector, tmpVector+1, tmpVector+2, tmpVector+3, tmpVector+4,tmpVector+5,tmpVector+6)<7)   			
		{
			printf("Line %d has less than %d iterms\n", lineNum, fieldnum);
			return false;
		}
						
		tmpTree->node[lineNum-1].nid = (V3DLONG)tmpVector[0];
		tmpTree->node[lineNum-1].ntype = (unsigned char)tmpVector[1];
		tmpTree->node[lineNum-1].x = (float)tmpVector[2];
		tmpTree->node[lineNum-1].y = (float)tmpVector[3];
		tmpTree->node[lineNum-1].z = (float)tmpVector[4];
		tmpTree->node[lineNum-1].radius = (float)tmpVector[5];
		tmpTree->node[lineNum-1].parentid = (V3DLONG)tmpVector[6];

	} while (fgets(buffer,1000,file) != NULL);
	
	// assign Tree
	Tree = new swcTree(lineNum);
	Tree->treeNodeNum = lineNum;
	
	for (int i=0; i<lineNum; i++)
	{
		Tree->node[i].nid = tmpTree->node[i].nid;
		Tree->node[i].ntype = tmpTree->node[i].ntype;
		Tree->node[i].x = tmpTree->node[i].x;
		Tree->node[i].y = tmpTree->node[i].y;
		Tree->node[i].z = tmpTree->node[i].z;
		Tree->node[i].radius = tmpTree->node[i].radius;
		Tree->node[i].parentid = tmpTree->node[i].parentid;
	}
	
	fclose(file);

	// print tree if tagprint = 1
	if (tagprint) Tree->printSwcTree();

	// free memory, delete pointers
	if (tmpTree) {delete tmpTree; tmpTree=0;}
	if (tmpVector) 	{delete []tmpVector; tmpVector=0;}
	
	return true;
}


//write a swcTree object into a swc file
bool swcTree::writeSwcFile(const char *outfilename)	
{
	int i,j;
	FILE *file;
	
	file = fopen(outfilename, "wt");
	
	for (i=0; i<treeNodeNum; i++)
	{
//		printf("%d %d %f %f %f %f %d\n", node[i].nid, node[i].ntype, node[i].x, node[i].y, node[i].z, node[i].radius, node[i].parentid);
		fprintf(file, "%d %d %f %f %f %f %d\n", node[i].nid, node[i].ntype, node[i].x, node[i].y, node[i].z, node[i].radius, node[i].parentid);
	}
	
	fclose(file);
	
	return true;
	
}


void swcTree::printSwcTree()
{
	for (int i=0; i<treeNodeNum; i++)
	{
		printf("%d %d %f %f %f %f %d\n", node[i].nid, node[i].ntype, node[i].x, node[i].y, node[i].z, node[i].radius, node[i].parentid);
	}
	printf("\n");
	
}


// decompose a tree into multiple sub-trees based on give seedNodeID
// note that if thre is no other nodes between two seed nodes (branching nodes), then the decomposed branch contains only
// one node which is the first node (from the root direction)
void swcTree::decompose(V3DLONG *seedNodeID, V3DLONG seedNodeNum, swcTree **&subTrees, V3DLONG &subTreeNum, V3DLONG *&branchIdx)
{

	V3DLONG int i, j, m,n;
	bool *seedTag = new bool [treeNodeNum]; // indicate if the node is a seed node (branching node)
	subTreeNum = -1;
	
	// set tag for seedNodes
	for (i=0; i<treeNodeNum; i++)
		seedTag[i] = 0;

	for (i=0; i<seedNodeNum; i++)
	{
		V3DLONG idx;
		getIndex(seedNodeID[i], idx);
		seedTag[idx] = 1;
	}
	
	subTrees = new swcTree *[seedNodeNum*MAX_CHILDREN_NUM]; // bigger than actual
	
	// set branchIdx
	branchIdx = new V3DLONG [seedNodeNum*(MAX_CHILDREN_NUM+1)]; 
	// imagine branchIdx is a 2d array, each row correspond to one branch, the first column of 
	// each row is the seed node id, the following MAX_CHILDREN_NUM columns are the index of the branch, e.g., 0,1,2,...
	// determined by which child it is of the parent seed node
	
	for (i=0; i<seedNodeNum; i++)
	{
		branchIdx[i*(MAX_CHILDREN_NUM+1)] = seedNodeID[i];
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			branchIdx[i*(MAX_CHILDREN_NUM+1)+j+1] = INVALID_VALUE;
	}
	
	// build subtree for each seedNode
	for (i=0; i<seedNodeNum; i++) 
	{
		V3DLONG *seedNodeChildrenID =0, *seedNodeChildrenNodeIdx=0, seedNodeChildrenNum;
		getDirectChildren(seedNodeID[i], seedNodeChildrenID, seedNodeChildrenNodeIdx, seedNodeChildrenNum);
		
	
		V3DLONG seedidx;
		getIndex(seedNodeID[i], seedidx);
	
		for (j=0; j<seedNodeChildrenNum; j++)
		{
			// build subtree
			V3DLONG subtreeNodeNum;
				
//			if (seedTag[seedNodeChildrenNodeIdx[j]] == 0) // if seedNodeChildrenID[j] is a seedNode no sub-branch is extracted		
			if (seedTag[seedNodeChildrenNodeIdx[j]] ==1) // if the child node is a seed node (branching point), then that branch only contains one node, i.e., the parent seed node
//			if ((seedTag[seedNodeChildrenNodeIdx[j]] ==1)&&(seedNodeChildrenNum==1)) // if the child node is a seed node (branching point) and it is the only child, then that branch only contains one node, i.e., the parent seed node. If it is not the only child, do not make a separate branch
			{
				subtreeNodeNum = 1;
				subTreeNum++;
				subTrees[subTreeNum] = new swcTree(subtreeNodeNum);
				
				subTrees[subTreeNum]->node[0].nid = node[seedidx].nid;
				subTrees[subTreeNum]->node[0].ntype = node[seedidx].ntype;
				subTrees[subTreeNum]->node[0].x = node[seedidx].x;
				subTrees[subTreeNum]->node[0].y = node[seedidx].y;
				subTrees[subTreeNum]->node[0].z = node[seedidx].z;
				subTrees[subTreeNum]->node[0].radius = node[seedidx].radius;
				subTrees[subTreeNum]->node[0].parentid = -1;

//				branchIdx[i*(MAX_CHILDREN_NUM+1)+1] = 0;
				branchIdx[i*(MAX_CHILDREN_NUM+1)+j+1] = j;
			}
			else // the branch should contain multiple nodes
			{
				
				if (seedTag[seedNodeChildrenNodeIdx[j]] == 0) //otherwise, the child is a seed node, do nothing
				{
					subtreeNodeNum = 0;
					V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];	

					tmpTreeNodeIdx[subtreeNodeNum] = seedidx; // put the parent seed node as the root in the current branch
					
					V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
					V3DLONG headptr, tailptr;
					V3DLONG curnodeid, curnodeidx;
					
					//initialize nodeQueue
					for (m=0; m<treeNodeNum; m++)
						nodeQueue[m]= INVALID_VALUE;
					
					nodeQueue[0] = seedNodeChildrenID[j];
					headptr = 0;
					tailptr = 0;
					
	//				tmpTreeNodeIdx[subtreeNodeNum] = seedNodeChildrenNodeIdx[j]; // add seedNodeChildrenNodeIdx[j] as root of the sub-tree
					
					//get the first element from the queue
					curnodeid = nodeQueue[headptr];	
				//	swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
	//				bool reachSeedNodeTag = 0;
					
					while ((curnodeid!=INVALID_VALUE)&&(subtreeNodeNum<treeNodeNum)) //there is still element in the queue
					{	
						subtreeNodeNum++;
						
						swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
						
	//					printf("%d, %d\n", curnodeid, curnodeidx);
						
						tmpTreeNodeIdx[subtreeNodeNum] = curnodeidx; // add the current node index
						
						// add children of the current node to the tail of the queue
						V3DLONG *childrenID = 0;
						V3DLONG *childrenNodeIdx = 0;
						V3DLONG childrenNum;

						swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);

				#ifdef DEBUG_TREE
						
						printf("childrenNum = %d, subtreeNodeNum=%d \n", childrenNum, subtreeNodeNum);
						
						for (n=0;n<treeNodeNum; n++)
							printf("%d ", nodeQueue[n]);
						printf("\n");
				#endif		

						n = 0;
						while (n<childrenNum) 
						{		
							if (seedTag[childrenNodeIdx[n]]==0) // add to the queue only when it is not a seed node
							{
								tailptr++;
								nodeQueue[tailptr]=childrenID[n];
							}
							n++;


						}// while end
						
							
						//get a node from the head of the queue
						headptr++;
						curnodeid = nodeQueue[headptr];

						if (childrenID) {delete []childrenID; childrenID=0;}
						if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}		
					} // while end
					
	//				if (reachSeedNodeTag == 0) // the last node is not seed node, it is a leaf node, should be included
	//					subtreeNodeNum++;

					subtreeNodeNum++;
					
					// assign output values
					if (subtreeNodeNum>0)
					{
						
						subTreeNum++;
						subTrees[subTreeNum] = new swcTree(subtreeNodeNum);
	//					subtreeNodeIdx = new V3DLONG [subtreeNodeNum];
						
						branchIdx[i*(MAX_CHILDREN_NUM+1)+j+1] = j;
						
						for (n=0; n<subtreeNodeNum; n++)
						{
							subTrees[subTreeNum]->node[n].nid = node[tmpTreeNodeIdx[n]].nid;
							subTrees[subTreeNum]->node[n].ntype = node[tmpTreeNodeIdx[n]].ntype;
							subTrees[subTreeNum]->node[n].x = node[tmpTreeNodeIdx[n]].x;
							subTrees[subTreeNum]->node[n].y = node[tmpTreeNodeIdx[n]].y;
							subTrees[subTreeNum]->node[n].z = node[tmpTreeNodeIdx[n]].z;
							subTrees[subTreeNum]->node[n].radius = node[tmpTreeNodeIdx[n]].radius;
							
							if (n==0)
								subTrees[subTreeNum]->node[n].parentid = -1;
							else
								subTrees[subTreeNum]->node[n].parentid = node[tmpTreeNodeIdx[n]].parentid;	
							
							subTrees[subTreeNum]->treeNodeNum = subtreeNodeNum;
	//						subtreeNodeIdx[n] = tmpTreeNodeIdx[i];

	//						printf("%d, %d, %f, %f,%f, %f, %d\n",subTrees[subTreeNum]->node[n].nid,
	//						subTrees[subTreeNum]->node[n].ntype, subTrees[subTreeNum]->node[n].x,
	//						subTrees[subTreeNum]->node[n].y, subTrees[subTreeNum]->node[n].z,
	//						subTrees[subTreeNum]->node[n].radius, subTrees[subTreeNum]->node[n].parentid);
							
						}

	//
	////				#ifdef DEBUG_TREE
	//					
	//					printf("The %d th subtree:\n", subTreeNum);
	//					
	//					printf("sub tree node nmber = %d\n", subtreeNodeNum);
	//					
	//					printf("sub tree nodes:\n");
	//					for (n=0; n<subtreeNodeNum; n++)
	////						printf("%d ", subtreeNodeIdx[i]);
	//						printf("%d ", subTrees[subTreeNum]->node[n].nid);
	//						
	//					printf("\n");
	//
	////				#endif
					
					
					} //if (subtreeNodeNum>0)
					
					// delete pointer
					if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
					if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
				} // if (seedTag[seedNodeChildrenNodeIdx[j]] == 0)

			} //else if (seedTag[seedNodeChildrenNodeIdx[j]] ==1 )
			

//				#ifdef DEBUG_TREE
			
			printf("The %d th subtree:\n", subTreeNum);			
			printf("sub tree node nmber = %d\n", subtreeNodeNum);
			
			printf("sub tree nodes:\n");
			for (n=0; n<subtreeNodeNum; n++)
				printf("%d ", subTrees[subTreeNum]->node[n].nid);
				
			printf("\n");

//				#endif
			
		} //for j
		
		if (seedNodeChildrenID) {delete []seedNodeChildrenID; seedNodeChildrenID=0;}
		if (seedNodeChildrenNodeIdx) {delete []seedNodeChildrenNodeIdx; seedNodeChildrenNodeIdx=0;}
	} // for i
	
	subTreeNum++; //since it starts from 0
	
}


// decompose a tree into multiple sub-trees based on give seedNodeID
// note that if thre is no other nodes between two seed nodes (branching nodes), then the decomposed branch contains only
// one node which is the first node (from the root direction)
// on 20090911, add array subTreeSeeds, recording the starting and ending seed nodes of a subtree, this is used in rendering
// matching results with matlab function gen_swc4v3d_hierarchical_matching
// it is a 1d array containing subTreeNum*(MAX_CHILDREN_NUM+1) elements, the MAX_CHILDREN_NUM*i th element is the starting seed of the ith subtree, 
// and the (MAX_CHILDREN_NUM+1)*i+1th element to (MAX_CHILDREN_NUM+1)*i+MAX_CHILDREN_NUM th is the ending seed of the ith subtree 

void swcTree::decompose(V3DLONG *seedNodeID, V3DLONG seedNodeNum, swcTree **&subTrees, V3DLONG &subTreeNum, V3DLONG *&branchIdx, V3DLONG *&subTreeSeeds)
{

	V3DLONG int i, j, m,n;
	bool *seedTag = new bool [treeNodeNum]; // indicate if the node is a seed node (branching node)
	subTreeNum = -1;
	
	// set tag for seedNodes
	for (i=0; i<treeNodeNum; i++)
		seedTag[i] = 0;

	for (i=0; i<seedNodeNum; i++)
	{
		V3DLONG idx;
		getIndex(seedNodeID[i], idx);
		seedTag[idx] = 1;
	}
	
	subTrees = new swcTree *[seedNodeNum*MAX_CHILDREN_NUM]; // bigger than actual
	
	// set branchIdx
	branchIdx = new V3DLONG [seedNodeNum*(MAX_CHILDREN_NUM+1)]; 
	// imagine branchIdx is a 2d array, each row correspond to one branch, the first column of 
	// each row is the seed node id, the following MAX_CHILDREN_NUM columns are the index of the branch, e.g., 0,1,2,...
	// determined by which child it is of the parent seed node
	
	for (i=0; i<seedNodeNum; i++)
	{
		branchIdx[i*(MAX_CHILDREN_NUM+1)] = seedNodeID[i];
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			branchIdx[i*(MAX_CHILDREN_NUM+1)+j+1] = INVALID_VALUE;
	}
	
	V3DLONG len = (MAX_CHILDREN_NUM+1);
//	subTreeSeeds = new V3DLONG [seedNodeNum*len];
	V3DLONG maxsubtreenum = 500; // at most 500 subtrees
	subTreeSeeds = new V3DLONG [maxsubtreenum*len]; 
	
	// initialize subTreeSeeds
	for (i=0; i<maxsubtreenum*len; i++)
	{
		subTreeSeeds[i] = INVALID_VALUE;
	}
	
	// build subtree for each seedNode
	for (i=0; i<seedNodeNum; i++) 
	{
		V3DLONG *seedNodeChildrenID =0, *seedNodeChildrenNodeIdx=0, seedNodeChildrenNum;
		getDirectChildren(seedNodeID[i], seedNodeChildrenID, seedNodeChildrenNodeIdx, seedNodeChildrenNum);
		
	
		V3DLONG seedidx;
		getIndex(seedNodeID[i], seedidx);
	
		for (j=0; j<seedNodeChildrenNum; j++)
		{
			// build subtree
			V3DLONG subtreeNodeNum;
				
//			if (seedTag[seedNodeChildrenNodeIdx[j]] == 0) // if seedNodeChildrenID[j] is a seedNode no sub-branch is extracted		
			if (seedTag[seedNodeChildrenNodeIdx[j]] ==1) // if the child node is a seed node (branching point), then that branch only contains one node, i.e., the parent seed node
//			if ((seedTag[seedNodeChildrenNodeIdx[j]] ==1)&&(seedNodeChildrenNum==1)) // if the child node is a seed node (branching point) and it is the only child, then that branch only contains one node, i.e., the parent seed node. If it is not the only child, do not make a separate branch
			{
				subtreeNodeNum = 1;
				subTreeNum++;
				
				subTreeSeeds[subTreeNum*len] = seedNodeID[i];
				subTreeSeeds[subTreeNum*len+1] = seedNodeChildrenID[j];
				
				subTrees[subTreeNum] = new swcTree(subtreeNodeNum);
				
				subTrees[subTreeNum]->node[0].nid = node[seedidx].nid;
				subTrees[subTreeNum]->node[0].ntype = node[seedidx].ntype;
				subTrees[subTreeNum]->node[0].x = node[seedidx].x;
				subTrees[subTreeNum]->node[0].y = node[seedidx].y;
				subTrees[subTreeNum]->node[0].z = node[seedidx].z;
				subTrees[subTreeNum]->node[0].radius = node[seedidx].radius;
				subTrees[subTreeNum]->node[0].parentid = -1;

//				branchIdx[i*(MAX_CHILDREN_NUM+1)+1] = 0;
				branchIdx[i*(MAX_CHILDREN_NUM+1)+j+1] = j;
			}
			else // the branch should contain multiple nodes
			{
				
				if (seedTag[seedNodeChildrenNodeIdx[j]] == 0) //otherwise, the child is a seed node, do nothing
				{
					subtreeNodeNum = 0;
					V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];	

					tmpTreeNodeIdx[subtreeNodeNum] = seedidx; // put the parent seed node as the root in the current branch
					
					V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
					V3DLONG headptr, tailptr;
					V3DLONG curnodeid, curnodeidx;
					
					//initialize nodeQueue
					for (m=0; m<treeNodeNum; m++)
						nodeQueue[m]= INVALID_VALUE;
					
					nodeQueue[0] = seedNodeChildrenID[j];
					headptr = 0;
					tailptr = 0;
					
	//				tmpTreeNodeIdx[subtreeNodeNum] = seedNodeChildrenNodeIdx[j]; // add seedNodeChildrenNodeIdx[j] as root of the sub-tree
					
					//get the first element from the queue
					curnodeid = nodeQueue[headptr];	
				//	swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
	//				bool reachSeedNodeTag = 0;
					
					V3DLONG endingSeedsNum = 0;
					V3DLONG *endingSeeds = new V3DLONG [MAX_CHILDREN_NUM];
					
					while ((curnodeid!=INVALID_VALUE)&&(subtreeNodeNum<treeNodeNum)) //there is still element in the queue
					{	
						subtreeNodeNum++;
						
						swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
						
	//					printf("%d, %d\n", curnodeid, curnodeidx);
						
						tmpTreeNodeIdx[subtreeNodeNum] = curnodeidx; // add the current node index
						
						// add children of the current node to the tail of the queue
						V3DLONG *childrenID = 0;
						V3DLONG *childrenNodeIdx = 0;
						V3DLONG childrenNum;

						swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);

				#ifdef DEBUG_TREE
						
						printf("childrenNum = %d, subtreeNodeNum=%d \n", childrenNum, subtreeNodeNum);
						
						for (n=0;n<treeNodeNum; n++)
							printf("%d ", nodeQueue[n]);
						printf("\n");
				#endif		

						n = 0;
						while (n<childrenNum) 
						{		
							if (seedTag[childrenNodeIdx[n]]==0) // add to the queue only when it is not a seed node
							{
								tailptr++;
								nodeQueue[tailptr]=childrenID[n];
							}
							else
							{	
								endingSeeds[endingSeedsNum] = childrenID[n];
								endingSeedsNum++;
								
							}
							n++;


						}// while end
						
							
						//get a node from the head of the queue
						headptr++;
						curnodeid = nodeQueue[headptr];

						if (childrenID) {delete []childrenID; childrenID=0;}
						if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}		
					} // while end
					
	//				if (reachSeedNodeTag == 0) // the last node is not seed node, it is a leaf node, should be included
	//					subtreeNodeNum++;

					subtreeNodeNum++;
					
					// assign output values
					if (subtreeNodeNum>0)
					{
						
						subTreeNum++;
						subTreeSeeds[len*subTreeNum] = seedNodeID[i];
						
						for (int v = 1; v<=endingSeedsNum; v++)
							subTreeSeeds[subTreeNum*len+v] = endingSeeds[v-1];
						
						subTrees[subTreeNum] = new swcTree(subtreeNodeNum);
	//					subtreeNodeIdx = new V3DLONG [subtreeNodeNum];
						
						branchIdx[i*(MAX_CHILDREN_NUM+1)+j+1] = j;
						
						for (n=0; n<subtreeNodeNum; n++)
						{
							subTrees[subTreeNum]->node[n].nid = node[tmpTreeNodeIdx[n]].nid;
							subTrees[subTreeNum]->node[n].ntype = node[tmpTreeNodeIdx[n]].ntype;
							subTrees[subTreeNum]->node[n].x = node[tmpTreeNodeIdx[n]].x;
							subTrees[subTreeNum]->node[n].y = node[tmpTreeNodeIdx[n]].y;
							subTrees[subTreeNum]->node[n].z = node[tmpTreeNodeIdx[n]].z;
							subTrees[subTreeNum]->node[n].radius = node[tmpTreeNodeIdx[n]].radius;
							
							if (n==0)
								subTrees[subTreeNum]->node[n].parentid = -1;
							else
								subTrees[subTreeNum]->node[n].parentid = node[tmpTreeNodeIdx[n]].parentid;	
							
							subTrees[subTreeNum]->treeNodeNum = subtreeNodeNum;
	//						subtreeNodeIdx[n] = tmpTreeNodeIdx[i];

	//						printf("%d, %d, %f, %f,%f, %f, %d\n",subTrees[subTreeNum]->node[n].nid,
	//						subTrees[subTreeNum]->node[n].ntype, subTrees[subTreeNum]->node[n].x,
	//						subTrees[subTreeNum]->node[n].y, subTrees[subTreeNum]->node[n].z,
	//						subTrees[subTreeNum]->node[n].radius, subTrees[subTreeNum]->node[n].parentid);
							
						}

	//
	////				#ifdef DEBUG_TREE
	//					
	//					printf("The %d th subtree:\n", subTreeNum);
	//					
	//					printf("sub tree node nmber = %d\n", subtreeNodeNum);
	//					
	//					printf("sub tree nodes:\n");
	//					for (n=0; n<subtreeNodeNum; n++)
	////						printf("%d ", subtreeNodeIdx[i]);
	//						printf("%d ", subTrees[subTreeNum]->node[n].nid);
	//						
	//					printf("\n");
	//
	////				#endif
					
					
					} //if (subtreeNodeNum>0)
					
					// delete pointer
					if (endingSeeds) {delete []endingSeeds; endingSeeds=0;}
					if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
					if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
				} // if (seedTag[seedNodeChildrenNodeIdx[j]] == 0)

			} //else if (seedTag[seedNodeChildrenNodeIdx[j]] ==1 )
			

//				#ifdef DEBUG_TREE
			
			printf("The %d th subtree:\n", subTreeNum);			
			printf("sub tree node nmber = %d\n", subtreeNodeNum);
			
			printf("sub tree nodes:\n");
			for (n=0; n<subtreeNodeNum; n++)
				printf("%d ", subTrees[subTreeNum]->node[n].nid);
				
			printf("\n");

//				#endif
			
		} //for j
		
		if (seedNodeChildrenID) {delete []seedNodeChildrenID; seedNodeChildrenID=0;}
		if (seedNodeChildrenNodeIdx) {delete []seedNodeChildrenNodeIdx; seedNodeChildrenNodeIdx=0;}
	} // for i
	
	subTreeNum++; //since it starts from 0
	
}


// get the id and index of root of the tree
void swcTree::getRootID(V3DLONG &rootid, V3DLONG &rootidx)
{
	for (int i=0; i<treeNodeNum; i++)
		if (node[i].parentid == -1)
		{
			rootidx = i;
			rootid = node[i].nid;
			break;
		}
}

	
// get the index of the nodeid
// note getIndex cannot be applied to parentid, since parentid is not unique
void swcTree::getIndex(V3DLONG nodeid, V3DLONG &nodeidx)
{

	nodeidx = INVALID_VALUE;
	
	if (nodeid<=0)
	{
		printf("nodeid should be bigger than 0, error in getIndex()\n");
		return;
	}
	
	for (V3DLONG int i=0; i<treeNodeNum; i++)
	{
		if (nodeid == node[i].nid)
		{
			nodeidx = i;
			break;
		}
	}
	
	if (nodeidx == INVALID_VALUE)
		printf("nodeid does not exist in the tree, error in getIndex()\n");
		
	return;
}

// find a path between nodeid1, nodeid2, 
// all nodes on the path are in the pathNodeList order in sequence
// ancestorTag indicates whether nodeid1 and nodeid2 have ancestor relationship, 1 if they do, 0 otherwise

void swcTree::findPath(V3DLONG nodeid1, V3DLONG nodeid2, V3DLONG *&pathNodeList, V3DLONG &pathNodeNum, unsigned char &ancestorTag)
{	
	V3DLONG i, j, k;
	V3DLONG nodeidx1, nodeidx2;
	
	if (nodeid1 == nodeid2) // the same node
	{
		pathNodeList = new V3DLONG [1];
		pathNodeList[0] = nodeid1;
		pathNodeNum = 1;
		ancestorTag = 0;
		return;
	}

	// get the depth of nodeid1, nodeid2
	V3DLONG depth1, depth2;
	swcTree::computeDepth(depth1, nodeid1);
	swcTree::computeDepth(depth2, nodeid2);
	
	// switch nodeid1 and nodeid2 if depth1<depth2
	if (depth1<depth2)
	{	
		V3DLONG tmpnodeid;
		tmpnodeid = nodeid1;
		nodeid1 = nodeid2;
		nodeid2 = tmpnodeid;
	}
	
	//get the index of nodeid1, nodeid2
	swcTree::getIndex(nodeid1, nodeidx1);
	swcTree::getIndex(nodeid2, nodeidx2);

	V3DLONG *tmp1 = new V3DLONG [500]; //up streaming searching for nodeid1	
    V3DLONG *tmp2 = new V3DLONG [500];  //up streaming searching for nodeid2	
//	printf("tmp1 address:%d, tmp2 address before:%d\n ", tmp1, tmp2);
			
//	printf("tmp1 address: %d\n", tmp1);

	//up streaming searching starting from nodeid1
	i = 0;
	tmp1[i] = nodeid1;
	
	do 
	{
//		printf("%d, %d***\n", nodeid1, nodeidx1);

		nodeid1 = node[nodeidx1].parentid;
//		printf("%d, %d\n", nodeid1, nodeidx1);
			
		if (nodeid1!=-1)
		{
			swcTree::getIndex(nodeid1, nodeidx1); // update index of nodeid1	
			i++;
			tmp1[i] = nodeid1;			
		}
	}while ((nodeid1!=nodeid2)&&(nodeid1!=-1));
	
//	printf("%d, %d\n", nodeid1, nodeid2);
	
	if (nodeid1==nodeid2) // hit nodeid2
	{
		ancestorTag = 1;
		pathNodeNum = i+1;
		pathNodeList = new V3DLONG [pathNodeNum];
		for (j=0; j<pathNodeNum; j++)
			pathNodeList[j] = tmp1[j];
		delete []tmp1; tmp1=0;
		delete []tmp2; tmp2=0;
		return;
	}
	
	if (nodeid1==-1) // come to root
	{

		ancestorTag = 0;
		
		//up streaming search starting from nodeid2, and compare with nodes along nodeid1's path
		unsigned char foundAncestor = 0;
			
		j = 0;
		tmp2[j] = nodeid2;
		
		do
		{
			nodeid2 = node[nodeidx2].parentid;
//			printf("%d %d %d\n", nodeid2, nodeidx2, tmp1[i]);
			
			if (nodeid2!=-1)
			{
				swcTree::getIndex(nodeid2, nodeidx2); // update index of nodeid2
			
				//test if this is the common ancestor
				for (k=0; k<i+1; k++)
				{
					if (nodeid2==tmp1[k]) // find common ancestor
					{
						foundAncestor = 1;
						break;
					}
				}
				
				if (foundAncestor == 1)
				{
					pathNodeNum = k+2+j;
					pathNodeList = new V3DLONG [pathNodeNum];
					
					for (i=0; i<=k; i++)
						pathNodeList[i] = tmp1[i];
						
					for (i=k+1; i<k+j+2; i++)
						pathNodeList[i] = tmp2[k+j+1-i];
						
//					for (i=k+j+1; i>k; i--)
//						pathNodeList[i] = tmp2[i];
					break;
						
				}
				else
				{
					j++;
					tmp2[j] = nodeid2;
				}
			}// if
		} while ((foundAncestor==0)&&(nodeid2!=-1));


		if (depth1<depth2)
		{
			V3DLONG *tmpPathNodeList = new V3DLONG [pathNodeNum];
			for (i=0; i<pathNodeNum; i++)
				tmpPathNodeList[pathNodeNum-1-i] = pathNodeList[i];
			for (i=0; i<pathNodeNum; i++)
				pathNodeList[i] = tmpPathNodeList[i];
			if (tmpPathNodeList) {delete tmpPathNodeList; tmpPathNodeList = 0;}
		}

//		if (nodeid2==-1)
		if (foundAncestor==0)
			printf("cannot find a path between nodeid1 and nodeid2, sth wrong!\n");
		
//		printf("tmp1 address:%d, tmp2 address:%d\n", tmp1, tmp2);
		
		delete []tmp1; tmp1=0;
		delete []tmp2; tmp2=0;
			
		return;
	} //	if (nodeid1==-1) // come to root
	
	printf("why here, program control wrong\n");
	
}	


// compute depth of each node
void swcTree::computeDepth(V3DLONG *&depth)
{
	
	depth = new V3DLONG [treeNodeNum];
	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid, curnodeidx;
	V3DLONG *childrenNum = 0;
	int i,j;
	
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
	{
		nodeQueue[i]= INVALID_VALUE;
		depth[i] = 0;
	}
	
	//find root
//	for (i=0; i<treeNodeNum; i++)
//	{
//		if (node[i].parentid==-1)
//		{
////			depth[i] = 0;
//			nodeQueue[0] = node[i].nid; // the default assumption is that the root node is numbered 1 in swc
//			headptr = 0;
//			tailptr = 0;
//			break;
//		}
//	}
	
	swcTree::getRootID(curnodeid, curnodeidx);
	nodeQueue[0] = node[curnodeidx].nid; // the default assumption is that the root node is numbered 1 in swc
	headptr = 0;
	tailptr = 0;
	
	V3DLONG **childrenList = 0;
	
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM);// MAX_CHILDREN_NUM is the maximum number of chilren for each node, the size of childrenList is treeNodeNum*MAX_CHILDREN_NUM

#ifdef DEBUG_TREE

	for (i=0; i<treeNodeNum; i++)
	{
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			printf("%d ", childrenList[i][j]);
		printf("\n");
	}
#endif	
	
	//assign depth of each node by breadth-first search

//	curnodeid = nodeQueue[headptr];	//get the first element from the queue
//	swcTree::getIndex(curnodeid, curnodeidx);
	
	while ((curnodeid!=INVALID_VALUE) && (headptr<treeNodeNum))//there is still element in the queue
	{	
		j = 0;
		while (j<childrenNum[curnodeidx]) // get children for each node

//		while (childrenList[curnodeid][j]!=INVALID_VALUE) // get children for each node
		{
			V3DLONG idx;
			swcTree::getIndex(childrenList[curnodeidx][j], idx);
			
//			printf("%d, %d, %d, %d\n", curnodeidx, j, childrenList[curnodeidx][j], idx);
			
			
//			depth[childrenList[curnodeidx][j]-1] = depth[curnodeidx]+1;
			depth[idx] = depth[curnodeidx]+1;
			
			//add children to the tail of the queue
			tailptr++;
			nodeQueue[tailptr]=childrenList[curnodeidx][j];
			j++;
		}
		
		
		//get a node from the head of the queue
		
		headptr++;
		curnodeid = nodeQueue[headptr];
		if (headptr<treeNodeNum) // when headptr ==treeNodeNum, getIndex will report error
			swcTree::getIndex(curnodeid, curnodeidx);
		
	}
	
#ifdef DEBUG_TREE
	
	printf("print depth in computeDepth\n");
	
	for (i=0; i<treeNodeNum; i++)
		printf("%d %d\n ", i, depth[i]);
		
	printf("\n");
#endif
	
	// delete pointer
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	if (childrenNum) {delete []childrenNum; childrenNum=0;}
	
}

// compute depth of a particular node
void swcTree::computeDepth(V3DLONG &depth, V3DLONG nodeid)
{
	depth = 0;	
	V3DLONG i, j, nodeidx;
	
	swcTree::getIndex(nodeid, nodeidx);
	
//	for (i=0; i<treeNodeNum; i++)
//	{
//		if (node[i].nid!==nodeid)
//		{
//			nodeidx = i;
//			break;
//		}
//	}

	while (node[nodeidx].parentid!=-1) // not reached root yet
	{
		depth++;
		j = node[nodeidx].parentid;

		swcTree::getIndex(j, nodeidx);
		
//		for (i=0; i<treeNodeNum; i++)
//		{
//			if (node[i].nid!==j)
//			{
//				nodeidx = i;
//				break;
//			}
//		}		
	}
	
}

//compute physical distance between two nodes of the tree in the current coordinate system
void swcTree::computeDistance(V3DLONG nodeid1, V3DLONG nodeid2, float &dist)
{
	V3DLONG nodeidx1, nodeidx2;

//	int i,j;
//	unsigned char found=0;

	swcTree::getIndex(nodeid1, nodeidx1);
	swcTree::getIndex(nodeid2, nodeidx2);
	
//	for (i=0; i<treeNodeNum; i++)
//	{
//		if (node[i].nid!==nodeid1)
//		{
//			nodeidx1 = i;
//			found++;
//		}
//		else 
//		{
//			if (node[i].nid!==nodeid2)
//			{
//				nodeidx2 = i;
//				found++;
//			}
//		}
//		
//		if (found==2) 
//			break;
//	}
	
	dist = sqrt((node[nodeidx1].x - node[nodeidx2].x)*(node[nodeidx1].x - node[nodeidx2].x) +
			 (node[nodeidx1].y - node[nodeidx2].y)*(node[nodeidx1].y - node[nodeidx2].y) +
			 (node[nodeidx1].z - node[nodeidx2].z)*(node[nodeidx1].z - node[nodeidx2].z));

}


//compute the length between two nodes that are directly parenet and child
// parentTag = 1, if compute the length between nodeid and its parent, in this case length is a single value, and lengthNum = 1
// parentTag = 0; if compute the length between nodeid and its children (can be multiple children), length can be a vector, and lengthNum = number of children
// 
void swcTree::computeLength(V3DLONG nodeid, unsigned char parentTag, float *&length, unsigned char &lengthNum)
{
	V3DLONG nodeidx;
	float dist;
	
	swcTree::getIndex(nodeid, nodeidx);
	
	if (parentTag == 1)
	{
		swcTree::computeDistance(nodeid, node[nodeidx].parentid, dist);
		length = new float [1];
		length[0] = dist;
		lengthNum = 1;
	}
	else
	{
		V3DLONG *childrenID = 0;
		V3DLONG *childrenNodeIdx =0;
		V3DLONG childrenNum;
		int i;
		
		swcTree::getDirectChildren(nodeid, childrenID, childrenNodeIdx, childrenNum);	
		
		lengthNum = childrenNum;
		length = new float [lengthNum];
		
		for (i=0; i<childrenNum; i++)
		{
			swcTree::computeDistance(nodeid, node[nodeidx].parentid, dist);
			length[i] = dist;
		}
		
		if (childrenID) {delete []childrenID; childrenID =0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx =0;}
		
	}

}
		
//compute the length between two nodes, may or may not have ancestor relationship
void swcTree::computeLength(V3DLONG nodeid1, V3DLONG nodeid2, float &length)
{
	int i, j;
	V3DLONG nodeidx1,nodeidx2;
	float dist=0;
	V3DLONG *pathNodeList = 0;
	unsigned char ancestorTag;
	V3DLONG pathNodeNum;
	
	if (nodeid1==nodeid2)
	{
		length = 0;
		return;
	}
	
	//compute length
	
	length = 0;
	
	swcTree:findPath(nodeid1, nodeid2, pathNodeList, pathNodeNum, ancestorTag);
	
	if (pathNodeList==0)
	{
		for (i=0; i<treeNodeNum; i++)
			printf("%d ", node[i].nid);
		printf("\n");	
		
		swcTree::findPath(nodeid1, nodeid2, pathNodeList, pathNodeNum, ancestorTag);
		
	}
	
//	printf("nodeid1= %d, nodeid2 = %d, pathNodeNum = %d\n", nodeid1, nodeid2, pathNodeNum);
	
	for (i=0; i<pathNodeNum-1; i++)
	{
		swcTree::computeDistance(pathNodeList[i], pathNodeList[i+1], dist);
		length += dist;
	}
	
	if (pathNodeList) {delete []pathNodeList; pathNodeList=0;}
	
}

// compute the diameter of the tree, which is defined as the longest path length
// between leaf nodes
void swcTree::computeTreeDiameter(float &treeDiameter, V3DLONG &leafNodeID1, V3DLONG &leafNodeID2)
{
	V3DLONG i,j;
	
	V3DLONG *leafNodeList = 0;
	V3DLONG *leafNodeIdx = 0;	
	V3DLONG leafNodeNum;
	
	float length=0;
	treeDiameter = 0;

	// find all leaf nodes
	swcTree::getLeafNode(leafNodeList, leafNodeIdx, leafNodeNum);
	
	// find the longest path length between leaf ndoes
	for (i=0; i<leafNodeNum-1; i++)
	for (j=i+1; j<leafNodeNum; j++)
	{
		swcTree:computeLength(leafNodeList[i], leafNodeList[j], length);
		if (length>treeDiameter)
		{
			treeDiameter = length;
			leafNodeID1 = node[i].nid;
			leafNodeID2 = node[j].nid;
		}
	}
	
	if (leafNodeList) {delete []leafNodeList; leafNodeList =0;}
	if (leafNodeIdx) {delete []leafNodeIdx; leafNodeIdx =0;}
}

// compute the center of the tree, which is defined as the longest path length
// between leaf nodes
void swcTree::computeTreeCenter(float *&treeCenter, float &averageRadius)
{
	V3DLONG i,j;
	
	V3DLONG *leafNodeList = 0;
	V3DLONG *leafNodeIdx = 0;	
	V3DLONG leafNodeNum;
	
	float length=0;
	float treeDiameter = 0;
	
	treeCenter = new float [3];
	treeCenter[0] = 0; 
	treeCenter[1] = 0; 
	treeCenter[2] = 0; 
	averageRadius = 0;
	
	// find all leaf nodes
	swcTree::getLeafNode(leafNodeList, leafNodeIdx, leafNodeNum);
	
	// find the longest path length between leaf ndoes
	for (i=0; i<leafNodeNum; i++)
	{
		treeCenter[0] += node[i].x;
		treeCenter[1] += node[i].y;
		treeCenter[2] += node[i].z;
		averageRadius += node[i].radius;
	}
	
	for (i=0; i<3; i++)
		treeCenter[i] /= leafNodeNum;
	
	averageRadius /= leafNodeNum;
	
	if (leafNodeList) {delete []leafNodeList; leafNodeList =0;}
	if (leafNodeIdx) {delete []leafNodeIdx; leafNodeIdx =0;}
}

// compute the sum of the length weighted distance from nodeid to all the nodes in Nodelist
void swcTree::computeLengthWeightedDistance(V3DLONG nodeid, V3DLONG *NodeList, V3DLONG NodeListNum, float *&dist)
{
	V3DLONG i;
	dist = new float [3];
	
	for (i=0; i<NodeListNum; i++)
	{
		float length;
		float dist1;
		
		if (NodeList[i]!=nodeid)
		{
			swcTree::computeLength(nodeid, NodeList[i], length);
			swcTree::computeDistance(nodeid, NodeList[i], dist1); 
			
			dist[0] += length*dist1;
			dist[1] += length;
			dist[2] += dist1;
		}
					
	}
}

// compute the normalized length weighted distance of a node in the tree
// method = 0: use length-weighted-distance from root to normalize
// method = 1: use sum of length from nodeid to normalize
// method = 2: use sum of distance from nodeid to normalize
// method = 3: use the multiplication of the sum of the distance and the sum of length from nodeid to normalize

void swcTree::computeLengthWeightedDistanceNormalized(V3DLONG nodeid, float &dist, unsigned char method)
{
	V3DLONG i;
	dist = 0;
		
	V3DLONG NodeListNum = treeNodeNum;
	V3DLONG *NodeList = new V3DLONG [NodeListNum];
	
	for (i=0; i<NodeListNum; i++)
		NodeList[i] = node[i].nid;
	
	//  compute the length-weighted-distance of nodeid
	float *dist1 = 0;
	swcTree::computeLengthWeightedDistance(nodeid, NodeList, NodeListNum, dist1);
	
	if (method ==0) // use length-weighted-distance from root to normalize
	{
		// compute the length-weighted-distance of rootid
		V3DLONG rootid, rootidx;
		swcTree::getRootID(rootid, rootidx);
		float *dist2 =0;
		swcTree::computeLengthWeightedDistance(rootid, NodeList, NodeListNum, dist2);
		
		dist = dist1[0]/dist2[0];
	}
	
	if (method == 1) // use sum of length from nodeid to normalize
		dist = dist1[0]/dist1[1];
	
	if (method == 2) // use sum of distance from nodeid to normalize
		dist = dist1[0]/dist1[2];
	
	if (method ==3) // use the multiplication of the sum of the distance and the sum of length from nodeid to normalize
		dist = dist1[0]/(dist1[1]*dist1[2]);
	
}


// compute the orientation of the brunch between parentNodeid and childNodeid in the current coordinate system, they need to be direct parent and child
void swcTree::computeOrientation(V3DLONG parentNodeid, V3DLONG childNodeid, float &angle1, float &angle2)
{
}

// compute the angle between the line connecting nodeid, nodeid1, and nodeid, nodeid2
void swcTree::computeAngle(V3DLONG nodeid, V3DLONG nodeid1, V3DLONG nodeid2, float &angle)
{
	struct {float x,y,z;} v1, v2;
	V3DLONG nodeidx, nodeidx1, nodeidx2;
	float costheta;
	
	//get index of the nodes
	swcTree::getIndex(nodeid, nodeidx);
	swcTree::getIndex(nodeid1, nodeidx1);
	swcTree::getIndex(nodeid2, nodeidx2);
	
	// compute vectors
	v1.x = node[nodeidx1].x-node[nodeidx].x;
	v1.y = node[nodeidx1].y-node[nodeidx].y;
	v1.z = node[nodeidx1].z-node[nodeidx].z;

	v2.x = node[nodeidx2].x-node[nodeidx].x;
	v2.y = node[nodeidx2].y-node[nodeidx].y;
	v2.z = node[nodeidx2].z-node[nodeidx].z;
	
	// compute inner product
	costheta = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z)/((v1.x*v1.x + v1.y*v1.y + v1.z*v1.z)*(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z));
	angle = acos(costheta)*180/PI;
	
}

// compute the angle between two vectors
void swcTree::computeAngle(float *vec1, float *vec2, float &angle)
{
	// compute inner product
	float costheta;
	
	costheta = (vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2])/((vec1[0]*vec1[0] + vec1[1]*vec1[1] + vec1[2]*vec1[2])*(vec2[0]*vec2[0] + vec2[1]*vec2[1] + vec2[2]*vec2[2]));
	if (costheta<-1)
		costheta = -1;
	if (costheta>1)
		costheta = 1;
		
	angle = acos(costheta)*180/PI;
//	printf("%f, %f\n", costheta, angle);
	
}

// compute the angles between pair-wise vectors formed by the average vector of children path and the average vector of parent path
// branchNum: number of branches, which is the number of direct children + 1
// angles: list of angles, if there are N children, then the number of angles are (N+1)*N/2,
// consisting of angles between any two pairs of children branches and the angles between the direct children branch and the direct parent
// angles[0]~angles[N-1] are the angles between the parent branch and each children branch, in the same order listed in branchRootID
// angles[N]~angles[(N+1)*N/2-1] are pair-wise angles between each children branch

void swcTree::computeBranchingAngles(V3DLONG nodeid, V3DLONG &branchNum, float *&angles)
{
	V3DLONG i, j;
	
	// find all nodes along the parent path and compute average vector along the parent path rooted at nodeid
	V3DLONG *ancestorsID=0, *ancestorsIdx=0, ancestorsNum;
	swcTree:getAncestors(nodeid, ancestorsID, ancestorsIdx, ancestorsNum);

	float *parentPathVec =0, *childrenPathVec=0;
	V3DLONG nodeidx;

	swcTree::getIndex(nodeid, nodeidx);
//	printf("nodeid = %d, nodeidx = %d\n", nodeid, nodeidx);
	
	if (ancestorsNum !=0) // nodeid is not root
	{
		
		float tmpx=0, tmpy=0, tmpz=0;
		
		for (i=0; i<ancestorsNum; i++)
		{
			tmpx += node[ancestorsIdx[i]].x;
			tmpy += node[ancestorsIdx[i]].y;
			tmpz += node[ancestorsIdx[i]].z;		
		}
		
		parentPathVec = new float [3];
		parentPathVec[0] = node[nodeidx].x-tmpx/ancestorsNum;
		parentPathVec[1] = node[nodeidx].y-tmpy/ancestorsNum;
		parentPathVec[2] = node[nodeidx].z-tmpz/ancestorsNum;
	}
	
	// find all nodes along the children path and compute average vector along each child path rooted at nodeid
	V3DLONG *directChildrenID=0, *directChildrenIdx=0, directChildrenNum;		
	swcTree::getDirectChildren(nodeid, directChildrenID, directChildrenIdx, directChildrenNum);
	
	if (directChildrenNum!=0) // nodeid is not leaf node
	{
		childrenPathVec = new float [directChildrenNum*3];
		
		for (i=0; i<directChildrenNum; i++)
		{
			V3DLONG *childrenID=0, *childrenNodeIdx=0, childrenNum;		
			getAllChildren(directChildrenID[i], childrenID, childrenNodeIdx, childrenNum); // does not include the node itself
			
			float tmpx=0, tmpy=0, tmpz=0;
			
			tmpx = node[directChildrenIdx[i]].x;
			tmpy = node[directChildrenIdx[i]].y;
			tmpz = node[directChildrenIdx[i]].z;
			
			for (j=0; j<childrenNum; j++)
			{
				tmpx += node[childrenNodeIdx[j]].x;
				tmpy += node[childrenNodeIdx[j]].y;
				tmpz += node[childrenNodeIdx[j]].z;		
			}
			
			childrenPathVec[i*3] = node[nodeidx].x-tmpx/(childrenNum+1);
			childrenPathVec[i*3+1] = node[nodeidx].y-tmpy/(childrenNum+1);
			childrenPathVec[i*3+2] = node[nodeidx].z-tmpz/(childrenNum+1);
			
			if (childrenID) {delete []childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		}
	}
	
	// compute angles
	
	if ((ancestorsNum!=0) && (directChildrenNum!=0)) //nodeid is not the root, nor the leaf node
	{

		// compute angles between parent path and each sub-branch
		angles = new float [directChildrenNum*(directChildrenNum+1)/2];
		float *childPathVec = new float [3];		
		for (i=0; i<directChildrenNum; i++)
		{
			childPathVec[0] = childrenPathVec[i*3];
			childPathVec[1] = childrenPathVec[i*3+1];
			childPathVec[2] = childrenPathVec[i*3+2];
			
			swcTree::computeAngle(parentPathVec, childPathVec, angles[i]);
		}
		if (childPathVec) {delete []childPathVec; childPathVec=0;}
		
		// compute angles between each sub-branch
		unsigned char cnt = directChildrenNum-1;
		float *childPathVec1 = new float [3];	
		float *childPathVec2 = new float [3];	
		
		
		for (i=0; i<directChildrenNum-1; i++)
		{
			childPathVec1[0] = childrenPathVec[i*3];
			childPathVec1[1] = childrenPathVec[i*3+1];
			childPathVec1[2] = childrenPathVec[i*3+2];
			
			for (j=i+1; j<directChildrenNum; j++)
			{
				childPathVec2[0] = childrenPathVec[j*3];
				childPathVec2[1] = childrenPathVec[j*3+1];
				childPathVec2[2] = childrenPathVec[j*3+2];
			
				cnt++;		
				swcTree::computeAngle(childPathVec1, childPathVec2, angles[cnt]);
			}
		}
		
		branchNum = directChildrenNum;
		
		if (childPathVec1) {delete []childPathVec1; childPathVec1=0;}
		if (childPathVec2) {delete []childPathVec2; childPathVec2=0;}
		
	}

	if (ancestorsNum==0) // nodeid is the root
	{
		angles = new float [directChildrenNum*(directChildrenNum+1)/2];
	
		for (i=0; i<directChildrenNum; i++)
			angles[i] = 0; // the angles between sub-branch and parent branch are set to 0
			
		// compute angles between each sub-branch
		unsigned char cnt = directChildrenNum;
		float *childPathVec1 = new float [3];	
		float *childPathVec2 = new float [3];	
		
		for (i=0; i<directChildrenNum-1; i++)
		{
			childPathVec1[0] = childrenPathVec[i*3];
			childPathVec1[1] = childrenPathVec[i*3+1];
			childPathVec1[2] = childrenPathVec[i*3+2];
			
			for (j=i+1; j<directChildrenNum; j++)
			{
				childPathVec2[0] = childrenPathVec[j*3];
				childPathVec2[1] = childrenPathVec[j*3+1];
				childPathVec2[2] = childrenPathVec[j*3+2];
			
				cnt++;		
				swcTree::computeAngle(childPathVec1, childPathVec2, angles[cnt]);
			}
		}
		
		branchNum = directChildrenNum;
		
		if (childPathVec1) {delete []childPathVec1; childPathVec1=0;}
		if (childPathVec2) {delete []childPathVec2; childPathVec2=0;}			
		
	}

	if (directChildrenNum==0) // nodeid is a leaf node
	{
		angles = new float [1];
		angles[0] = 0;
		branchNum = 0;
		
	}


	//delete pointers
	if (parentPathVec) {delete []parentPathVec; parentPathVec=0;}
	if (childrenPathVec) {delete []childrenPathVec; childrenPathVec=0;}
	
	if (ancestorsID) {delete []ancestorsID; ancestorsID=0;}
	if (ancestorsIdx) {delete []ancestorsIdx; ancestorsIdx=0;}
	
	if (directChildrenID) {delete []directChildrenID; directChildrenID=0;}
	if (directChildrenIdx) {delete []directChildrenIdx; directChildrenIdx=0;}
	
}

// compute the angles between pair-wise vectors formed by direct children and parent of nodeid
// branchRootID: root node of each branch, branchRootID[0] is the id of parent node
// branchRootIdx: index of root node of each branch, branchRootIdx[0] is the index of parent node
// branchNum: number of branches, which is the number of direct children + 1
// angles: list of angles, if there are N children, then the number of angles are (N+1)*N/2,
// consisting of angles between any two pairs of children branches and the angles between the direct children branch and the direct parent
// angles[0]~angles[N-1] are the angles between the parent branch and each children branch, in the same order listed in branchRootID
// angles[N]~angles[(N+1)*N/2-1] are pair-wise angles between each children branch

void swcTree::computeBranchingAngles(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, float *&angles)
{

		int i,j;
		
		V3DLONG *directChildrenID=0, *directChildrenIdx=0, directChildrenNum;		
		swcTree::getDirectChildren(nodeid, directChildrenID, directChildrenIdx, directChildrenNum);
		
		branchNum = directChildrenNum + 1; // add parent branch, which contains all the nodes not belonging to children branches
		branchRootID = new V3DLONG [branchNum];
		branchRootIdx = new V3DLONG [branchNum];
		angles = new float [branchNum*(branchNum-1)/2];
		
		// assign values to branchRootID, branchRootIdx
		for (j=0; j<directChildrenNum; j++)
		{
			branchRootID[j+1] =directChildrenID[j];
			branchRootIdx[j+1] = directChildrenIdx[j];
		}
		
		V3DLONG nodeidx;
		swcTree::getIndex(nodeid, nodeidx);
		branchRootID[0] = node[nodeidx].parentid;

		if (branchRootID[0]==-1)
			branchRootIdx[0] = -1;
		else
		{
			swcTree::getIndex(node[nodeidx].parentid, nodeidx);
			branchRootIdx[0] = nodeidx;
		}
	
		// compute angles
		
		if (branchRootIdx[0] == -1) //nodeid is the root
		{
			for (i=0; i<directChildrenNum; i++)
				angles[i] = INVALID_VALUE;
		}
		else
		{
			for (i=0; i<directChildrenNum; i++)
				swcTree::computeAngle(nodeid, node[nodeidx].parentid, directChildrenID[i], angles[i]);
		}
		
		unsigned char cnt = directChildrenNum;
		
		for (i=0; i<directChildrenNum-1; i++)
		for (j=i+1; j<directChildrenNum; j++)
		{
			cnt++;
			swcTree::computeAngle(nodeid, directChildrenID[i], directChildrenID[j], angles[cnt]);
		}

		
		if (directChildrenID) {delete []directChildrenID; directChildrenID=0;}
		if (directChildrenIdx) {delete []directChildrenIdx; directChildrenIdx=0;}

}


// compute the angles between the vectors formed by the leaf ndoes, current tree root (i.e., rootid), 
// and anterior tree root, this function is called by leaf node matching function: treeMatching_leafNodes()
// rootid is the root of the current tree
// vec is the vector pointing from rootid to the reference node (e.g., the anterior tree root, or the global root of the tree)
// if vec = [0,0,0], then nodeid is the root of global tree

void swcTree::computeLeafNodeAngles(V3DLONG rootid, float *vec, V3DLONG *leafNodeListID, V3DLONG leafNodeNum, float *&angles) 
{
	int i,j;
	V3DLONG rootidx;
	float vec1[3];
	
	swcTree::getIndex(rootid, rootidx);

	angles = new float [(leafNodeNum+1)*leafNodeNum/2];
		
	// compute parent angles
	
	if ((vec[0] == 0)&&(vec[1] == 0)&&(vec[2] == 0)) //rootid is the global root, do not compute parent angles
	{
		for (i=0; i<leafNodeNum; i++)
			angles[i] = INVALID_VALUE;
	}
	else
	{
		for (i=0; i<leafNodeNum; i++)
		{
			V3DLONG idx;
			swcTree::getIndex(leafNodeListID[i], idx);
			
//			vec1[0] = swcTree->node[rootidx].x - swcTree->node[idx].x;
//			vec1[1] = swcTree->node[rootidx].y - swcTree->node[idx].y;
//			vec1[2] = swcTree->node[rootidx].z - swcTree->node[idx].z;

			vec1[0] = node[rootidx].x - node[idx].x;
			vec1[1] = node[rootidx].y - node[idx].y;
			vec1[2] = node[rootidx].z - node[idx].z;
			
//			printf("vec1 = %f, %f, %f\n", vec1[0], vec1[1], vec1[2]);
//			printf("vec = %f, %f, %f\n", vec[0], vec[1], vec[2]);
			
			swcTree::computeAngle(vec1, vec, angles[i]);								
		}
	}
	
	// compute angles between each leaf branch
	unsigned char cnt = leafNodeNum;
	
	for (i=0; i<leafNodeNum-1; i++)
	for (j=i+1; j<leafNodeNum; j++)
	{
		cnt++;
		swcTree::computeAngle(rootid, leafNodeListID[i], leafNodeListID[j], angles[cnt]);
	}
}


// compute the angles and branch size by combining functions computeBranchingAngles and getBranchNodeNum to remove some redunt 
// compuation if both size and angles need to compute
// branchRootID: root node of each branch, branchRootID[0] is the id of parent node
// branchRootIdx: index of root node of each branch, branchRootIdx[0] is the index of parent node
// branchNum: number of branches, which is the number of direct children + 1
// branchNodeNum: number of nodes (or size) of each branch, branchNodeNum[0] is the number of nodes in parent branch, 
// which contains not only all ancestors but all nodes not belong to children
// angles: list of angles, if there are N children, then the number of angles are (N+1)*N/2,
// consisting of angles between any two pairs of children branches and the angles between the direct children branch and the direct parent
// angles[0]~angles[N-1] are the angles between the parent branch and each children branch, in the same order listed in branchRootID
// angles[N]~angles[(N+1)*N/2-1] are pair-wise angles between each children branch

void swcTree::computeBranchNodeNumAngles(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, V3DLONG *&branchNodeNum, float *&angles)
{

	int i,j;
	
	V3DLONG *directChildrenID=0, *directChildrenIdx=0, directChildrenNum;		
	swcTree::getDirectChildren(nodeid, directChildrenID, directChildrenIdx, directChildrenNum);
	
	branchNum = directChildrenNum + 1; // add parent branch, which contains all the nodes not belonging to children branches
	branchRootID = new V3DLONG [branchNum];
	branchRootIdx = new V3DLONG [branchNum];
	branchNodeNum = new V3DLONG [branchNum];
	angles = new float [branchNum*(branchNum-1)/2];


	// assign values to branchNodeNum
	for (j=0; j<directChildrenNum; j++)
		swcTree::getSubTreeNodeNum(directChildrenID[j], branchNodeNum[j+1]);

	//get the size of the parent branch
	V3DLONG sumval = 0;
	for (j=0; j<directChildrenNum; j++)
		sumval += branchNodeNum[j+1];

	//assign values to the first element of branchNodeNumx

	// assign values to branchRootID, branchRootIdx
	for (j=0; j<directChildrenNum; j++)
	{
		branchRootID[j+1] =directChildrenID[j];
		branchRootIdx[j+1] = directChildrenIdx[j];
	}

	//assign values to the first element of branchRootID, branchRootIdx
	V3DLONG nodeidx;
	swcTree::getIndex(nodeid, nodeidx);
	branchRootID[0] = node[nodeidx].parentid;

	if (branchRootID[0]==-1)
	{
		branchRootIdx[0] = -1;
		branchNodeNum[0] = 0;
	}
	else
	{
		swcTree::getIndex(node[nodeidx].parentid, nodeidx);
		branchRootIdx[0] = nodeidx;
		branchNodeNum[0] = treeNodeNum-sumval;

	}

	// compute angles
	
	if (branchRootIdx[0] == -1) //nodeid is the root
	{
		for (i=0; i<directChildrenNum; i++)
			angles[i] = INVALID_VALUE;
	}
	else
	{
		for (i=0; i<directChildrenNum; i++)
			swcTree::computeAngle(nodeid, node[nodeidx].parentid, directChildrenID[i], angles[i]);
	}
	
	unsigned char cnt = directChildrenNum;
	
	for (i=0; i<directChildrenNum-1; i++)
	for (j=i+1; j<directChildrenNum; j++)
	{
		cnt++;
		swcTree::computeAngle(nodeid, directChildrenID[i], directChildrenID[j], angles[cnt]);
	}

	
	if (directChildrenID) {delete []directChildrenID; directChildrenID=0;}
	if (directChildrenIdx) {delete []directChildrenIdx; directChildrenIdx=0;}
		
}

// compute the angles, and sum of sub-branch lengths
// branchRootID, branchRootIdx, branchNum, angles are defined in the same way as in member function computeBranchSizeAngles
// lengthSum is defined in a similar way as branchNodeNum in computeBranchSizeAngles 
// this function replace number of nodes in each sub-branch by the total length of the sub-branch
// since a V3DLONG branch may have a small number of nodes, and a short branch may have many nodes

void swcTree::computeBranchLengthSumAngles(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, float *&lengthSum, float *&angles)
{

	int i,j;
	
	V3DLONG *directChildrenID=0, *directChildrenIdx=0, directChildrenNum;		
	swcTree::getDirectChildren(nodeid, directChildrenID, directChildrenIdx, directChildrenNum);
	
	branchNum = directChildrenNum + 1; // add parent branch, which contains all the nodes not belonging to children branches
	branchRootID = new V3DLONG [branchNum];
	branchRootIdx = new V3DLONG [branchNum];
	lengthSum = new float [branchNum];
	angles = new float [branchNum*(branchNum-1)/2];

	float maxlength, minlength;
	V3DLONG maxLengthNodeid, minLengthNodeid;
	
	// assign values to lengthSum
	for (j=0; j<directChildrenNum; j++)
		swcTree::getSubTreeLength(directChildrenID[j], maxlength, maxLengthNodeid, minlength, minLengthNodeid, lengthSum[j+1]);

	//get the size of the parent branch
	V3DLONG sumval = 0;
	for (j=0; j<directChildrenNum; j++)
		sumval += lengthSum[j+1];

	V3DLONG rootid, rootidx;
	float treelength;
	
	// get the total length of branches in the tree
	swcTree::getRootID(rootid, rootidx);
//	swcTree::getSubTreeTotalLength(rootid, treelength); 
	swcTree::getSubTreeLength(rootid, maxlength, maxLengthNodeid,  minlength, minLengthNodeid, treelength);
	
	//assign values to the first element of lengthSum

	// assign values to branchRootID, branchRootIdx
	for (j=0; j<directChildrenNum; j++)
	{
		branchRootID[j+1] =directChildrenID[j];
		branchRootIdx[j+1] = directChildrenIdx[j];
	}

	//assign values to the first element of branchRootID, branchRootIdx
	V3DLONG nodeidx;
	swcTree::getIndex(nodeid, nodeidx);
	branchRootID[0] = node[nodeidx].parentid;

	if (branchRootID[0]==-1)
	{
		branchRootIdx[0] = -1;
		lengthSum[0] = 0;
	}
	
	else
	{
		swcTree::getIndex(node[nodeidx].parentid, nodeidx);
		branchRootIdx[0] = nodeidx;
		lengthSum[0] = treelength-sumval;
		
	}

	// compute angles
	
	if (branchRootIdx[0] == -1) //nodeid is the root of the tree
	{
		for (i=0; i<directChildrenNum; i++)
			angles[i] = INVALID_VALUE;
	}
	else
	{
		for (i=0; i<directChildrenNum; i++)
			swcTree::computeAngle(nodeid, node[nodeidx].parentid, directChildrenID[i], angles[i]);
	}
	
	unsigned char cnt = directChildrenNum;
	
	for (i=0; i<directChildrenNum-1; i++)
	for (j=i+1; j<directChildrenNum; j++)
	{
		cnt++;
		swcTree::computeAngle(nodeid, directChildrenID[i], directChildrenID[j], angles[cnt]);
	}

	
	if (directChildrenID) {delete []directChildrenID; directChildrenID=0;}
	if (directChildrenIdx) {delete []directChildrenIdx; directChildrenIdx=0;}

}

// compute the branch length ratio of nodeid
// nodeid needs to be a branching point
// method = 0, only lengthratio[0] and lengthratio[1] are computed
//			   lengthratio[0] = length of the maximum length subbranch / length of the parent branch
//			   lengthratio[1] = length of the second maximum length subbranch / length of the parent branch
// method = 1, lengthratio[2] and lengthratio[3] are also computed
//			   lengthratio[2] = length of the maximum length subbranch / length of the remaining branches
//             lengthratio[3] = length of the maximum length subbranch / length of the remaining branches
// method = 2, only lengthration[0] and lengthratio[1] are computed, but they are computed in
//             the following way: l1*d1/(l0*d0), l2*d2/(l0*d0), where l1, l2, l0 are the path length of the maximum
//             and second maximum length subbranch and parent branch, and d1, d2, d0 are the Euclidean distances
//             of the respective path
// method =3,  only lengthratio[0] is computed, where lengthration[0] = (length of the maximum length subbranch *
//             distance of the maximum length subbranch)/(length of the parent branch * distance of the parent branch)
// method = 4, lengthratio[0]~lengthratio[k-1] are computed, where k is the number of children of nodeid
//             lengthratio[i] (i<k) is the length ratio of ith sub-branch of nodeid with respect to the parent branch.
// method = 5, the same as method = 4, except it is not normalized by the length of the parent branch. In other words,
//             compute length not length ratio
// 
// note that node type can only be root, branching nodes, and leaf nodes, no continual nodes

void swcTree::computeBranchLengthRatio(V3DLONG nodeid, float *&lengthratio, unsigned char &lengthrationNum, unsigned char method)
{

	if (lengthratio!=0)	{delete []lengthratio; lengthratio=0;}
	// get children of nodeid 	 		

	V3DLONG i;
	float *branchlength=0;
	
	V3DLONG *childrenID=0, *childrenNodeIdx=0, childrenNum;
	swcTree::getDirectChildren(nodeid, childrenID, childrenNodeIdx, childrenNum);	
	
	if (childrenNum==0) // leaf node
	{
		lengthratio = new float [1];
		branchlength = new float [1];
		lengthrationNum = 1;
		lengthratio[0] = 0;
//		printf("nodeid is leaf node, length ratio is a scalar value set to zero by default.\n");
		
	}
	else if (childrenNum ==1) //root
	{
		lengthratio = new float [1];
		lengthratio[0] = -INVALID_VALUE; //the only node that has only one child is root since the tree has been pruned, set the lengthratio to be very large
		branchlength = new float [1];
		lengthrationNum = 1;
		
//		printf("nodeid is expected to be the root, not continual node.\n");
	}
	else if (childrenNum>=2) // branching points
	{
	
		switch (method)
		{
			case 0:
			case 2:
				lengthrationNum = 2;
				break;
			case 1:
				lengthrationNum = 4;
				break;
			case 3:
				lengthrationNum = 1;
				break;
			case 4:
			case 5:	
				lengthrationNum = childrenNum;
				break;
		}
		
	
//		lengthratio = new float [childrenNum];
//		for (i=0; i<childrenNum; i++)
//			lengthratio[i] = 0;
//			
//		// compute the length of the parent branch
//	//	float *branchlength = new float [MAX_CHILDREN_NUM];	
//		branchlength = new float [childrenNum];	

		lengthratio = new float [lengthrationNum];
		for (i=0; i<lengthrationNum; i++)
			lengthratio[i] = 0;
			
		// compute the length of the parent branch
		branchlength = new float [lengthrationNum];	
			
		V3DLONG rootid, rootidx;
		swcTree::getRootID(rootid, rootidx);
			
		if (nodeid==rootid)
		{
			printf("nodeid is the root\n");
			return;
		}

		swcTree::computeLength(nodeid, rootid, branchlength[0]);

		float maxBranchLength = INVALID_VALUE, secondMaxBranchLegnth = INVALID_VALUE;
		V3DLONG maxBranchNodeid, secondMaxBranchNodeid;
	
		for (i=0; i<childrenNum; i++)
		{
			float maxlength, minlength, totalLength;
			V3DLONG maxLengthNodeid, minLengthNodeid;	
			float length0;
			
			swcTree::getSubTreeLength(childrenID[i], maxlength, maxLengthNodeid, minlength, minLengthNodeid, totalLength);
			swcTree::computeLength(nodeid, childrenID[i], length0);

			//find the maximum and second maximum subranch length rooted at nodedid
			branchlength[i+1] = maxlength + length0;
			if (branchlength[i+1]>maxBranchLength)
			{
				secondMaxBranchLegnth = maxBranchLength;
				secondMaxBranchNodeid = maxBranchNodeid;
				
				maxBranchLength = branchlength[i+1];
				maxBranchNodeid = maxLengthNodeid;
				
			}	
			else
			{
				if (branchlength[i+1]>secondMaxBranchLegnth)
				{
					secondMaxBranchLegnth = branchlength[i+1];
					secondMaxBranchNodeid = maxLengthNodeid;
				}	
			}
		} // for (i=0; i<childrenNum; i++)
		
		if ((method == 0)||(method == 1))
		{
			lengthratio[0] = maxBranchLength / branchlength[0];
			lengthratio[1] = secondMaxBranchLegnth / branchlength[0];
		}
		
		if (method==1) // further compute remaining branch
		{
		
			float maxlength, minlength, treeTotalLength;
			V3DLONG maxLengthNodeid, minLengthNodeid;		
			
			swcTree::getSubTreeLength(rootid, maxlength, maxLengthNodeid, minlength, minLengthNodeid, treeTotalLength);
			
			float tmp = treeTotalLength-branchlength[0]; // length of the remaining branch
			
			if (tmp!=0)
			{
				lengthratio[2] = maxBranchLength / tmp;
				lengthratio[3] = secondMaxBranchLegnth / tmp;	
			}
		}
		else if (method == 2) // weighted ratio
		{
			float maxBranchDistance, secondMaxBranchDistance, parentBranchDistance;
			
			swcTree::computeDistance(nodeid, maxBranchNodeid, maxBranchDistance);
			swcTree::computeDistance(nodeid, secondMaxBranchNodeid, secondMaxBranchDistance);
			swcTree::computeDistance(nodeid, rootid, parentBranchDistance);
			
			float tmp = branchlength[0]*parentBranchDistance;
			
			lengthratio[0] = (maxBranchLength* maxBranchDistance)/tmp ;
			lengthratio[1] = (secondMaxBranchLegnth * secondMaxBranchDistance) /tmp;
		}
		else if (method == 3)
		{
			float maxBranchDistance, parentBranchDistance;
		
			swcTree::computeDistance(nodeid, maxBranchNodeid, maxBranchDistance);
			swcTree::computeDistance(nodeid, rootid, parentBranchDistance);
			
			lengthratio[0] = (maxBranchLength* maxBranchDistance)/(branchlength[0]*parentBranchDistance);
		}
		else if (method == 4)
		{
			for (i=0; i<childrenNum; i++)
				lengthratio[i] = branchlength[i+1]/branchlength[0];
		}
		else if (method == 5)
		{
			for (i=0; i<childrenNum; i++)
				lengthratio[i] = branchlength[i+1];
		}
		
	} //if (childrenNum >= 2)

	if (childrenID) {delete childrenID; childrenID=0;}
	if (branchlength) {delete branchlength; branchlength=0;}
	if (childrenNodeIdx) {delete childrenNodeIdx; childrenNodeIdx=0;}

}

//
////adpatively determine thresholds for keeping significant branching nodes
//// lengthThre1 and lengthThre2 will be used be removeInsignificantNodes
//// ratio1 and ratio2 is the given ratio of maximum length branch/parent length, and second maximum length branch/parent length
//// These two values determine the values of lengthThre1 and lengthThre2
//void swcTree::sigBranchingNodeThre(swcTree *Tree, float ratio1, float ratio2, float &lengthThre1, float &lengthThre2)
//{
//	
//	V3DLONG *branchingNodeList =0, *branchingNodeIdx=0, branchingNodeNum;
//	
//	swcTree::getBranchingNode(branchingNodeList, branchingNodeIdx, branchingNodeNum);
//	
//	V3DLONG i;
//	for (i=0; i<branchingNodeNum; i++)
//	{		
//		float *lengthratio =0;
//		unsigned char lengthratioNum;	
//		swcTree::computeBranchLengthRatio(branchingNodeList[i], lengthratio, lengthratioNum, 0);
//		
//		if (lengthratio) {delete []lengthratio; lengthratio=0;}
//		
//	}
//	
//	if (branchingNodeList) {delete []branchingNodeList; branchingNodeList=0;}
//	if (branchingNodeIdx) {delete []branchingNodeIdx; branchingNodeIdx=0;}
//}

// sort swc tree in depth, returns the index of node
// the node sequence is not necessarily in order, i.e., a smaller id# in swc file
// does not mean it has a small depth	
void swcTree::sortTree(V3DLONG *&nodeidx)
{

	nodeidx = new V3DLONG [treeNodeNum];
	V3DLONG *depth = 0;	
	float *depthtmp = new float [treeNodeNum+1];
	float *sortidx = new float [treeNodeNum+1];
	V3DLONG invalidVal = INVALID_VALUE;
	
	depthtmp[0] = invalidVal;
	sortidx[0] = invalidVal;
	
	swcTree::computeDepth(depth);
	
	for (int i=1; i<treeNodeNum+1; i++)
	{
		sortidx[i] = i-1;
		depthtmp[i] = (float)depth[i-1];
//		printf("i=%d, depth=%d\n", i-1, depth[i-1]);
	}
		
//	sort2(treeNodeNum+1, depthtmp, sortidx);
	sort2(treeNodeNum, depthtmp, sortidx);

#ifdef DEBUG_TREE	

	printf("sort tree result\n");
	
	for (int i=0; i<treeNodeNum; i++)
	{
		nodeidx[i] = (V3DLONG) sortidx[i+1];
		printf("%d ", nodeidx[i]);
	}
	printf("\n");
	
#endif
	
	if (depth) {delete []depth; depth=0;}
	if (depthtmp) {delete []depthtmp; depthtmp=0;}
	if (sortidx) {delete []sortidx; sortidx = 0;}
	
	return;
}


// remove all continual nodes,  and subtrees of insignificant branching nodes, leaf nodes of significant branching points are kept
// significant branching nodes is defined in this way: 
// 1) the length of the maximum-length-subbranch should be bigger than lengthThre1
// 2) the legnth of the second-maximum-length-subbranch should be bigger than lengthThre2 
//   (this is to prevent picking up some insignificant branching nodes which have small spurs on main branch) 
// This function is useful when trying to match the main structures with significant branching ndoes and leaf nodes of two trees 
// which is essential for hierarchical matching
// keepLeaf=1, keep leaf nodes, otherwise drop leaf nodes, which will make DP tree matching based on length ratio and angle easier
// removeNodeTag: indicate which node is removed from the original tree, 1: removed, 0: kept
// removeBranchTag: indicate for each node in the new tree, which sub-branch has been removed, 
//                  removeBranchTag is a 2d matrix newTreeNodeNum * MAX_CHILDREN_NUM, -1: removed, >=0: kept, value indicate which branch it is in new tree;
//					for removeBranchTag[i,j] where j> the number of children-1 of the node, values are set to INVALID_VALUE
//					e.g., 1, -1, -1, 0, -9999, -9999 means that for a node, it has 4 branches (4 values that unequal to -9999), its first and fourth branches 
//					are mapped to the second and the first branch of the new tree, its second and third branches are pruned.

void swcTree::removeInsignificantNodes(float lengthThre1, float lengthThre2, swcTree *&newTree, bool keepLeaf, bool *&removeNodeTag, V3DLONG **&removeBranchTag)
{
	V3DLONG **childrenList = 0;
	V3DLONG *childrenNum = 0;
	
	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	V3DLONG newTreeNodeNum = 0, newTreeNodeLeafNum=0;
	int i,j,m,n;
	V3DLONG curnodeidx, curnodeid, rootid, rootidx;
	removeNodeTag = new bool [treeNodeNum];

		
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); // get direct children for all nodes in the tree

	swcTree:getRootID(rootid, rootidx);
		
	for (i=0; i<treeNodeNum; i++)
	{
		removeNodeTag[i] = 0;
	
		if (i==rootidx) // do not remove root
		{
			newTreeNodeNum++;
			swcTree::getIndex(node[i].nid, curnodeidx);			
			tmpTreeNodeIdx[newTreeNodeNum-1] = curnodeidx;
		}
		else
		{
			
			if (childrenNum[i]<2)	// a continual node is removed, a leaf node is also temporarily removed, it will be added back if it is the leaf node of significant branching node
				removeNodeTag[i] = 1;
			else // a branching node
			{			
				// test if it is a significant branching nodes

				// find the maximum-length and second-maximum-length branches
				float maxBranchLength = INVALID_VALUE;
				float secondMaxBranchLength = INVALID_VALUE;
				float minBranchLength = -INVALID_VALUE;
				float totalLength = 0;
				V3DLONG maxBranchNodeid, secondMaxBranchNodeid, minBranchNodeid;
				
				getSubTreeLength(node[i].nid, maxBranchLength, maxBranchNodeid, secondMaxBranchLength, secondMaxBranchNodeid, minBranchLength, minBranchNodeid, totalLength); // get the maximum, second maximum, minimum length between nodeid and a node, and the total length of the subtree rooted at nodeid
				
//				for (j=0; j<childrenNum[i]; j++)
//				{
//					float maxlength, minlength, totalLength;
//					V3DLONG maxLengthNodeid, minLengthNodeid;	
//					float length0;
//					
//					swcTree::getSubTreeLength(childrenList[i][j], maxlength, maxLengthNodeid, minlength, minLengthNodeid, totalLength);
//					swcTree::computeLength(node[i].nid, childrenList[i][j], length0);
//
//					//find the maximum and second maximum subranch length rooted at nodedid
//					float branchlength = maxlength + length0;
//					if (branchlength>maxBranchLength)
//					{
//						secondMaxBranchLength = maxBranchLength;
//						secondMaxBranchNodeid = maxBranchNodeid;
//						
//						maxBranchLength = branchlength;
//						maxBranchNodeid = maxLengthNodeid;
//						
//					}	
//					else
//					{
//						if (branchlength>secondMaxBranchLength)
//						{
//							secondMaxBranchLength = branchlength;
//							secondMaxBranchNodeid = maxLengthNodeid;
//						}	
//					}
//				} // for j

				if ((maxBranchLength<lengthThre1)||(secondMaxBranchLength<lengthThre2)) // it's an insignficant branching node
					removeNodeTag[i]=1;
				else // it's a significant branching node
				{
					newTreeNodeNum++;
					swcTree::getIndex(node[i].nid, curnodeidx);			
					tmpTreeNodeIdx[newTreeNodeNum-1] = curnodeidx;
				} // if ((maxBranchLength<lengthThre1)||(maxBranchLength<lengthThre1))
				
			} // if (childrenNum[i]<2)
		} //if (i==rootidx)
	} // for (i=0; i<treeNodeNum; i++)

		
	// determine the leaf nodes in the new tree (not built yet), in order to add back the 
	// next branching nodes of the leaf nodes of the new tree to ensure the sub-branches of 
	// the new leaf nodes can be matched in tree matching process
	
	if (keepLeaf == 1) // add leaf node
	{
		V3DLONG *leafNodeTag = new V3DLONG [newTreeNodeNum];
		
		for (j=0; j<newTreeNodeNum; j++)
		{
			V3DLONG *childrenID=0, *childrenNodeIdx=0, cnum;
			getAllChildren(node[tmpTreeNodeIdx[j]].nid, childrenID, childrenNodeIdx, cnum); 
			
			m = 0;
			bool hitTag = 0;
			
			while ((m<cnum)&&(hitTag ==0))
			{
				n = 0;
				while ((n<newTreeNodeNum)&&(hitTag ==0))
				{
					if (childrenNodeIdx[m]==tmpTreeNodeIdx[n])
						hitTag = 1;
					else
						n++;
				}
				m++;
			}
			
			if (hitTag ==0) //subtree does not contain signficant branching nodes, thus is a leaf node in the new tree
				leafNodeTag[j] = 1;
			else
				leafNodeTag[j] = 0;
			
			if (childrenID) {delete []childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}					
				
		}
		

		// add back leaf nodes of the single branch of significant branching nodes
		for (j=0; j<newTreeNodeNum; j++)
		{
			V3DLONG *childrenID=0, *childrenNodeIdx=0, cnum;
			swcTree::getDirectChildren(node[tmpTreeNodeIdx[j]].nid, childrenID, childrenNodeIdx, cnum); 
			
			for (m=0; m<cnum; m++)
			{
				V3DLONG thisnode = childrenID[m];
				V3DLONG thisnodeidx;
				swcTree::getIndex(thisnode, thisnodeidx);
				
				while (1)
				{
					V3DLONG *childrenID_branch=0, *childrenNodeIdx_branch=0, cnum_branch;				
					getDirectChildren(thisnode, childrenID_branch, childrenNodeIdx_branch, cnum_branch); 
					
					if (cnum_branch==0) // reach a leaf node, add the leaf node
					{
						newTreeNodeLeafNum++;
						tmpTreeNodeIdx[newTreeNodeNum+newTreeNodeLeafNum-1] = thisnodeidx;
						break;
					}				
					else
					{
						if (cnum_branch==1) // reach a continual node, keep on traversing along the path
						{
							thisnode = childrenID_branch[0];
							swcTree::getIndex(thisnode, thisnodeidx);
						}
						else // branching node
						{
							if (leafNodeTag[j]==1) //branching node that is a leaf in the new tree
							{
								newTreeNodeLeafNum++;
								getIndex(thisnode, thisnodeidx);
								tmpTreeNodeIdx[newTreeNodeNum+newTreeNodeLeafNum-1] = thisnodeidx;
								break;
							}
							else 
							{
								// if the branching node does not belong to tmpTreeNodeIdx, keep on searching
								bool hitTag = 0;
								
								for (n=0; n<newTreeNodeNum; n++)
								{
									if (thisnodeidx == tmpTreeNodeIdx[n])
									{
										hitTag = 1;
										break;
									}
								}
								
								if (hitTag ==0)
								{
									//find the child with the longest path, no need to consider multiple paths, since the node is not in tmpTreeNodeIdx, it must be a node with its second-maximum-length path less than the threshold
									
									V3DLONG maxnum = INVALID_VALUE;
									V3DLONG max_n;
									
									for (n=0; n<cnum_branch; n++)
									{
										V3DLONG *allChildrenID=0, *allChildrenNodeIdx=0, allChildrenNum;
										getAllChildren(childrenID_branch[n], allChildrenID, allChildrenNodeIdx, allChildrenNum);
										if (maxnum<allChildrenNum)
										{
											maxnum = allChildrenNum; max_n = n;
										}
										
										if (allChildrenID) {delete []allChildrenID; allChildrenID=0;}
										if (allChildrenNodeIdx) {delete []allChildrenNodeIdx; allChildrenNodeIdx=0;}
										
									}
									thisnode = childrenID_branch[max_n];
									swcTree::getIndex(thisnode, thisnodeidx);
								}
								
								else // if hit a branching node that belong to tmpTreeNodeIdx, stop traversing, no node is added to tmpTreeNodeIdx 
									break;
							}
						}
					}
					if (childrenID_branch) {delete []childrenID_branch; childrenID_branch=0;}
					if (childrenNodeIdx_branch) {delete []childrenNodeIdx_branch; childrenNodeIdx_branch=0;}					
					
				} // while (1)			
			} // for m
			
			if (childrenID) {delete []childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}					
						
		} // for j

		if (leafNodeTag) {delete []leafNodeTag; leafNodeTag = 0;}
		
	}	

	// generate new tree
	newTreeNodeNum += newTreeNodeLeafNum;
	newTree = new swcTree(newTreeNodeNum); // allocat memory for nodes

	newTree->treeNodeNum = newTreeNodeNum;
//	printf("%d\n", newTreeNodeNum);
	
	for (int i=0; i<newTreeNodeNum; i++)
	{
			
		newTree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		newTree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		newTree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		newTree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		newTree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		newTree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		
		// find the right parenet
		curnodeidx = tmpTreeNodeIdx[i];
//			printf("%d\n", curnodeidx);
		
		if (curnodeidx == rootidx)
			newTree->node[i].parentid = -1;
		else
		{
			swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);	
//			printf("%d\n", curnodeidx);
					
			while (removeNodeTag[curnodeidx]==1)
			{
				swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);
//				printf("%d, %d, %d\n", i, tmpTreeNodeIdx[i], curnodeidx);
			}
			
			newTree->node[i].parentid = node[curnodeidx].nid;	
		}
		
//			printf("%d, %d, %d\n", i, newTree->node[i].nid, newTree->node[i].parentid);		
//			newTreeNodeIdx[i] = tmpTreeNodeIdx[i];
	}
	
	
// generate removeBranchTag

	//allocate memory
	V3DLONG *removeBranchTag1d = new V3DLONG [newTreeNodeNum*MAX_CHILDREN_NUM];
	
	if (!removeBranchTag1d)
	{ 
		printf("fail to allocate memory for removeBranchTag1d in swcTree::removeInsignificantNodes\n");
		return;
	}
	
	if (!new2dpointer(removeBranchTag, MAX_CHILDREN_NUM, newTreeNodeNum, removeBranchTag1d))
	{
		printf("fail to allocate memory for removeBranchTag1d in swcTree::removeInsignificantNodes\n");
		return;		   		   
	}

	for (i=0; i<newTreeNodeNum;i++)
	{
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			removeBranchTag[i][j] = INVALID_VALUE;
		
		V3DLONG *childrenID_newTree =0, *childrenNodeIdx_newTree =0, childrenNum_newTree;
		V3DLONG *childrenID =0, *childrenNodeIdx =0, childrenNum;
		
		newTree->getDirectChildren(newTree->node[i].nid, childrenID_newTree, childrenNodeIdx_newTree, childrenNum_newTree); // get the direct children of nodeid in new tree
		getDirectChildren(newTree->node[i].nid, childrenID, childrenNodeIdx, childrenNum); // get the direct children of nodeid in old tree
		
		for (j=0; j<childrenNum; j++)
		{
		
			swcTree *subtree;
			V3DLONG *subtreeNodeIdx;
			
			getSubTree(childrenID[j], subtree, subtreeNodeIdx);
			
			m= 0;
			while (m<subtree->treeNodeNum)
			{
				n = 0;
				while (n< childrenNum_newTree)
				{
					if (subtree->node[m].nid == childrenID_newTree[n])
					{	
//						removeBranchTag[i][j] = 0; // not removed
						removeBranchTag[i][j] = n; // not removed

						break;
					}
					n++;
				}
//				if (removeBranchTag[i][j] == 0)
				if (removeBranchTag[i][j] >= 0)
					break;
				else
					m++;
			}

			if (removeBranchTag[i][j] == INVALID_VALUE)
//				removeBranchTag[i][j] = 1; // the branch is removed
				removeBranchTag[i][j] = -1; // the branch is removed
			
			if (subtree) {delete subtree; subtree =0;}
			if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx =0;}
				
		}
		
		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}		
		if (childrenID_newTree) {delete []childrenID_newTree; childrenID_newTree=0;}
		if (childrenNodeIdx_newTree) {delete []childrenNodeIdx_newTree; childrenNodeIdx_newTree=0;}
		
	}
	
	// delete pointers
	
//	if (removeBranchTag1d) {delete []removeBranchTag1d; removeBranchTag1d=0;}
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	if (childrenNum) {delete []childrenNum; childrenNum=0;}

}



// The same as the above function except using lengthRatioThre to determine which branching nodes should be kept as significant branching nodes
void swcTree::removeInsignificantNodes(float *lengthRatioThre, swcTree *&newTree, bool keepLeaf, bool *&removeNodeTag, V3DLONG **&removeBranchTag)
{
	V3DLONG **childrenList = 0;
	V3DLONG *childrenNum = 0;
	
	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	V3DLONG newTreeNodeNum = 0, newTreeNodeLeafNum=0;
	int i,j,m,n;
	V3DLONG curnodeidx, curnodeid, rootid, rootidx;
	removeNodeTag = new bool [treeNodeNum];

		
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); // get direct children for all nodes in the tree

	swcTree:getRootID(rootid, rootidx);
		
	for (i=0; i<treeNodeNum; i++)
	{
		
		removeNodeTag[i] = 0;	
		if (i==rootidx) // do not remove root
		{
			newTreeNodeNum++;
			swcTree::getIndex(node[i].nid, curnodeidx);			
			tmpTreeNodeIdx[newTreeNodeNum-1] = curnodeidx;
		}
		else
		{
			
			if (childrenNum[i]<2)	// a continual node is removed, a leaf node is also temporarily removed, it will be added back if it is the leaf node of significant branching node
				removeNodeTag[i] = 1;
			else // a branching node
			{			
				// test if it is a significant branching nodes

//				// find the maximum-length and second-maximum-length branches
//				float maxBranchLength = INVALID_VALUE;
//				float secondMaxBranchLength = INVALID_VALUE;
//				float minBranchLength = -INVALID_VALUE;
//				float totalLength = 0;
//				V3DLONG maxBranchNodeid, secondMaxBranchNodeid, minBranchNodeid;
//				
//				getSubTreeLength(node[i].nid, maxBranchLength, maxBranchNodeid, secondMaxBranchLength, secondMaxBranchNodeid, minBranchLength, minBranchNodeid, totalLength); // get the maximum, second maximum, minimum length between nodeid and a node, and the total length of the subtree rooted at nodeid

				float *lengthratio =0;
				unsigned char lengthratioNum;	
				swcTree::computeBranchLengthRatio(node[i].nid, lengthratio, lengthratioNum, 0);
				printf("nodeid = %d, lengthratio[0]=%f, lengthratio[1]=%f, ", node[i].nid, lengthratio[0], lengthratio[1]);
				if ((lengthratio[0]<lengthRatioThre[0])||(lengthratio[1]<lengthRatioThre[1])) // it's an insignficant branching node				
//				if ((maxBranchLength<lengthThre1)||(secondMaxBranchLength<lengthThre2)) // it's an insignficant branching node
					removeNodeTag[i]=1;
				else // it's a significant branching node
				{
					newTreeNodeNum++;
					swcTree::getIndex(node[i].nid, curnodeidx);			
					tmpTreeNodeIdx[newTreeNodeNum-1] = curnodeidx;
				} // if ((maxBranchLength<lengthThre1)||(maxBranchLength<lengthThre1))
				printf("removeNodeTag=%d\n", removeNodeTag[i]);
				if (lengthratio!=0) {delete []lengthratio; lengthratio=0;}	
			} // if (childrenNum[i]<2)
		} //if (i==rootidx)
		
		
	} // for (i=0; i<treeNodeNum; i++)

		
	// determine the leaf nodes in the new tree (not built yet), in order to add back the 
	// next branching nodes of the leaf nodes of the new tree to ensure the sub-branches of 
	// the new leaf nodes can be matched in tree matching process
	
	if (keepLeaf == 1) // add leaf node
	{
		V3DLONG *leafNodeTag = new V3DLONG [newTreeNodeNum];
		
		for (j=0; j<newTreeNodeNum; j++)
		{
			V3DLONG *childrenID=0, *childrenNodeIdx=0, cnum;
			getAllChildren(node[tmpTreeNodeIdx[j]].nid, childrenID, childrenNodeIdx, cnum); 
			
			m = 0;
			bool hitTag = 0;
			
			while ((m<cnum)&&(hitTag ==0))
			{
				n = 0;
				while ((n<newTreeNodeNum)&&(hitTag ==0))
				{
					if (childrenNodeIdx[m]==tmpTreeNodeIdx[n])
						hitTag = 1;
					else
						n++;
				}
				m++;
			}
			
			if (hitTag ==0) //subtree does not contain signficant branching nodes, thus is a leaf node in the new tree
				leafNodeTag[j] = 1;
			else
				leafNodeTag[j] = 0;
			
			if (childrenID) {delete []childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}					
				
		}
		

		// add back leaf nodes of the single branch of significant branching nodes
		for (j=0; j<newTreeNodeNum; j++)
		{
			V3DLONG *childrenID=0, *childrenNodeIdx=0, cnum;
			swcTree::getDirectChildren(node[tmpTreeNodeIdx[j]].nid, childrenID, childrenNodeIdx, cnum); 
			
			for (m=0; m<cnum; m++)
			{
				V3DLONG thisnode = childrenID[m];
				V3DLONG thisnodeidx;
				swcTree::getIndex(thisnode, thisnodeidx);
				
				while (1)
				{
					V3DLONG *childrenID_branch=0, *childrenNodeIdx_branch=0, cnum_branch;				
					getDirectChildren(thisnode, childrenID_branch, childrenNodeIdx_branch, cnum_branch); 
					
					if (cnum_branch==0) // reach a leaf node, add the leaf node
					{
						newTreeNodeLeafNum++;
						tmpTreeNodeIdx[newTreeNodeNum+newTreeNodeLeafNum-1] = thisnodeidx;
						break;
					}				
					else
					{
						if (cnum_branch==1) // reach a continual node, keep on traversing along the path
						{
							thisnode = childrenID_branch[0];
							swcTree::getIndex(thisnode, thisnodeidx);
						}
						else // branching node
						{
							if (leafNodeTag[j]==1) //branching node that is a leaf in the new tree
							{
								newTreeNodeLeafNum++;
								getIndex(thisnode, thisnodeidx);
								tmpTreeNodeIdx[newTreeNodeNum+newTreeNodeLeafNum-1] = thisnodeidx;
								break;
							}
							else 
							{
								// if the branching node does not belong to tmpTreeNodeIdx, keep on searching
								bool hitTag = 0;
								
								for (n=0; n<newTreeNodeNum; n++)
								{
									if (thisnodeidx == tmpTreeNodeIdx[n])
									{
										hitTag = 1;
										break;
									}
								}
								
								if (hitTag ==0)
								{
									//find the child with the longest path, no need to consider multiple paths, since the node is not in tmpTreeNodeIdx, it must be a node with its second-maximum-length path less than the threshold
									
									V3DLONG maxnum = INVALID_VALUE;
									V3DLONG max_n;
									
									for (n=0; n<cnum_branch; n++)
									{
										V3DLONG *allChildrenID=0, *allChildrenNodeIdx=0, allChildrenNum;
										getAllChildren(childrenID_branch[n], allChildrenID, allChildrenNodeIdx, allChildrenNum);
										if (maxnum<allChildrenNum)
										{
											maxnum = allChildrenNum; max_n = n;
										}
										
										if (allChildrenID) {delete []allChildrenID; allChildrenID=0;}
										if (allChildrenNodeIdx) {delete []allChildrenNodeIdx; allChildrenNodeIdx=0;}
										
									}
									thisnode = childrenID_branch[max_n];
									swcTree::getIndex(thisnode, thisnodeidx);
								}
								
								else // if hit a branching node that belong to tmpTreeNodeIdx, stop traversing, no node is added to tmpTreeNodeIdx 
									break;
							}
						}
					}
					if (childrenID_branch) {delete []childrenID_branch; childrenID_branch=0;}
					if (childrenNodeIdx_branch) {delete []childrenNodeIdx_branch; childrenNodeIdx_branch=0;}					
					
				} // while (1)			
			} // for m
			
			if (childrenID) {delete []childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}					
						
		} // for j

		if (leafNodeTag) {delete []leafNodeTag; leafNodeTag = 0;}
		
	}	

	// generate new tree
	newTreeNodeNum += newTreeNodeLeafNum;
	newTree = new swcTree(newTreeNodeNum); // allocat memory for nodes

	newTree->treeNodeNum = newTreeNodeNum;
//	printf("%d\n", newTreeNodeNum);
	
	for (int i=0; i<newTreeNodeNum; i++)
	{
			
		newTree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		newTree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		newTree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		newTree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		newTree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		newTree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		
		// find the right parenet
		curnodeidx = tmpTreeNodeIdx[i];
//			printf("%d\n", curnodeidx);
		
		if (curnodeidx == rootidx)
			newTree->node[i].parentid = -1;
		else
		{
			swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);	
//			printf("%d\n", curnodeidx);
					
			while (removeNodeTag[curnodeidx]==1)
			{
				swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);
//				printf("%d, %d, %d\n", i, tmpTreeNodeIdx[i], curnodeidx);
			}
			
			newTree->node[i].parentid = node[curnodeidx].nid;	
		}
		
//			printf("%d, %d, %d\n", i, newTree->node[i].nid, newTree->node[i].parentid);		
//			newTreeNodeIdx[i] = tmpTreeNodeIdx[i];
	}
	
	
// generate removeBranchTag

	//allocate memory
	V3DLONG *removeBranchTag1d = new V3DLONG [newTreeNodeNum*MAX_CHILDREN_NUM];
	
	if (!removeBranchTag1d)
	{ 
		printf("fail to allocate memory for removeBranchTag1d in swcTree::removeInsignificantNodes\n");
		return;
	}
	
	if (!new2dpointer(removeBranchTag, MAX_CHILDREN_NUM, newTreeNodeNum, removeBranchTag1d))
	{
		printf("fail to allocate memory for removeBranchTag1d in swcTree::removeInsignificantNodes\n");
		return;		   		   
	}

	for (i=0; i<newTreeNodeNum;i++)
	{
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			removeBranchTag[i][j] = INVALID_VALUE;
		
		V3DLONG *childrenID_newTree =0, *childrenNodeIdx_newTree =0, childrenNum_newTree;
		V3DLONG *childrenID =0, *childrenNodeIdx =0, childrenNum;
		
		newTree->getDirectChildren(newTree->node[i].nid, childrenID_newTree, childrenNodeIdx_newTree, childrenNum_newTree); // get the direct children of nodeid in new tree
		getDirectChildren(newTree->node[i].nid, childrenID, childrenNodeIdx, childrenNum); // get the direct children of nodeid in old tree
		
		for (j=0; j<childrenNum; j++)
		{
		
			swcTree *subtree;
			V3DLONG *subtreeNodeIdx;
			
			getSubTree(childrenID[j], subtree, subtreeNodeIdx);
			
			m= 0;
			while (m<subtree->treeNodeNum)
			{
				n = 0;
				while (n< childrenNum_newTree)
				{
					if (subtree->node[m].nid == childrenID_newTree[n])
					{	
//						removeBranchTag[i][j] = 0; // not removed
						removeBranchTag[i][j] = n; // not removed

						break;
					}
					n++;
				}
//				if (removeBranchTag[i][j] == 0)
				if (removeBranchTag[i][j] >= 0)
					break;
				else
					m++;
			}

			if (removeBranchTag[i][j] == INVALID_VALUE)
//				removeBranchTag[i][j] = 1; // the branch is removed
				removeBranchTag[i][j] = -1; // the branch is removed
			
			if (subtree) {delete subtree; subtree =0;}
			if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx =0;}
				
		}
		
		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}		
		if (childrenID_newTree) {delete []childrenID_newTree; childrenID_newTree=0;}
		if (childrenNodeIdx_newTree) {delete []childrenNodeIdx_newTree; childrenNodeIdx_newTree=0;}
		
	}
	
	// delete pointers
	
//	if (removeBranchTag1d) {delete []removeBranchTag1d; removeBranchTag1d=0;}
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	if (childrenNum) {delete []childrenNum; childrenNum=0;}

}


// revised from removeInsignificantNodes()
// detect critical branch nodes in the tree, for hierarchical tree matching and decomposition purpose
// 
// Inpute parameters:
// threVector: threshold vectors, its value and dimension are determined by the mehtod that is 
//             used to detect critical branch nodes
// leafMethod: 0 if not add leaf
//             1 if add the leaf node of the longest sub-branch of a branch node to the returned tree, i.e., newTree
//             2 if add the leaf node of the momentum of the subtree of a branch node to the returned tree
// criticalNodeMethod: indicate which method to use to define a critical branch node
//         0 --- a branch node is critical if:  1) the path length of longest sub-branch is bigger 
//               than threVector[0]; and 2) the second longest sub-branch is bigger than threVector[1]
//         1 --- a branch node is critical if: 1) the path legnth of the longest sub-branch divided
//               by the path length from root to the branch node in consideratation is bigger than
//               threVector[0]; and 2) the path legnth of the second longest sub-branch divided
//               by the path length from root to the branch node in consideratation is bigger than
//               threVector[1]; 
//         2 --- a branch node is critical if: 1) the summed edge length of the biggest sub-branch
//               normmalized by the diameter of the tree (ie., the longest path path between any two leaf
//               nodes) is bigger than threVector[0]; 2) the summed edge length of the second biggest sub-branch
//               normalized by the diameter of the tree (ie., the longest path path between any two leaf
//               nodes) is bigger than threVector[0];
//         3 ---- the same as 2, use different normalize factor, which is the summed edge length of the entire tree
//
// Output parameters:
// removeNodeTag: indicate which node is removed from the original tree, 1: removed, 0: kept
// removeBranchTag: indicate for each node in the new tree, which sub-branch has been removed, 
//                  removeBranchTag is a 2d matrix newTreeNodeNum * MAX_CHILDREN_NUM, -1: removed, >=0: kept, value indicate which branch it is in new tree;
//					for removeBranchTag[i,j] where j> the number of children-1 of the node, values are set to INVALID_VALUE
//					e.g., 1, -1, -1, 0, -9999, -9999 means that for a node, it has 4 branches (4 values that unequal to -9999), its first and fourth branches 
//					are mapped to the second and the first branch of the new tree, its second and third branches are pruned.
// newTree: the new tree generated with the critical branch nodes, use straight lines to connect critical branch nodes
// newTreeWithPath: the new tree generated with the critical nodes, use path in the original tree to connect critical branch nodes
void swcTree::detectCriticalBranchNodes(float *threVector, unsigned char leafMethod, unsigned char criticalNodeMethod, 
										bool *&removeNodeTag, V3DLONG **&removeBranchTag, swcTree *&newTree, swcTree *&newTreeWithPath)
{

	V3DLONG **childrenList = 0;
	V3DLONG *childrenNum = 0;
	
	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	V3DLONG newTreeNodeNum = 0, newTreeNodeLeafNum=0;
	int i,j,m,n;
	V3DLONG curnodeidx, curnodeid, rootid, rootidx;
	removeNodeTag = new bool [treeNodeNum];
	
	float *leafNodeData = new float [treeNodeNum*7];
			
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); // get direct children for all nodes in the tree
	swcTree:getRootID(rootid, rootidx);
	
	// compute normalize factor for criticalNodeMethod 2
	float treeDiameter; 
	if (criticalNodeMethod == 2)
	{
		V3DLONG leafNodeID1, leafNodeID2;
		swcTree::computeTreeDiameter(treeDiameter, leafNodeID1, leafNodeID2);
		printf("treeDiameter = %f, leafNodeID1 = %d, leafNodeID2 = %d\n", treeDiameter, leafNodeID1, leafNodeID2);
	}
	// compute normalize factor for criticalNodeMethod 3
	float treeEdgeLength;
	if (criticalNodeMethod == 3)
	{
	    float maxlength, minlength;
	    V3DLONG maxLengthNodeid, minLengthNodeid;
		
		swcTree::getSubTreeLength(rootid, maxlength, maxLengthNodeid, minlength, minLengthNodeid, treeEdgeLength);
	}

	
	// ------------------------------
	// identify critical branch nodes
	// ------------------------------
	for (i=0; i<treeNodeNum; i++)
	{
		
		removeNodeTag[i] = 0;	
		if (i==rootidx) // do not remove root
		{
			tmpTreeNodeIdx[newTreeNodeNum] = rootidx;
			newTreeNodeNum++;
		}
		else
		{
			
			if (childrenNum[i]<2)	// a continual node is removed, a leaf node is also temporarily removed, 
			// which will be added back if addLeaf=1 and it is selected to be the leaf node of critical branch node in newTree
				removeNodeTag[i] = 1;
			else // a branching node
			{			
				// test if it is a significant branching nodes

				if (criticalNodeMethod ==0)  // the first definition of critical branch node, length of the longest and second longest branches
				{
					// find the maximum-length and second-maximum-length branches
					float maxBranchLength = INVALID_VALUE;
					float secondMaxBranchLength = INVALID_VALUE;
					float minBranchLength = -INVALID_VALUE;
					float totalLength = 0;
					V3DLONG maxBranchNodeid, secondMaxBranchNodeid, minBranchNodeid;
					
					getSubTreeLength(node[i].nid, maxBranchLength, maxBranchNodeid, secondMaxBranchLength, secondMaxBranchNodeid, minBranchLength, minBranchNodeid, totalLength); 
					// get the maximum, second maximum, minimum length between nodeid and a node, and the total length of the subtree rooted at nodeid
					if ((maxBranchLength<threVector[0])||(secondMaxBranchLength<threVector[1])) // it's not a critical branch node
						removeNodeTag[i]=1;
					else // it's a critical branch node
					{
						swcTree::getIndex(node[i].nid, curnodeidx);			
						tmpTreeNodeIdx[newTreeNodeNum] = curnodeidx;
						newTreeNodeNum++;
					} 
//					printf("removeNodeTag=%d\n", removeNodeTag[i]);
						
				}
				else if (criticalNodeMethod ==1) // the second definition of critical branch node: length ratio of the longest and second longest branches
				{
				
					float *lengthratio =0;
					unsigned char lengthratioNum;	
					swcTree::computeBranchLengthRatio(node[i].nid, lengthratio, lengthratioNum, 0); // criticalNodeMethod = 0 means only compute the longest and second longest branches
	//				printf("nodeid = %d, lengthratio[0]=%f, lengthratio[1]=%f, ", node[i].nid, lengthratio[0], lengthratio[1]);

					if ((lengthratio[0]<threVector[0])||(lengthratio[1]<threVector[1])) // it's not a critical branch node				
						removeNodeTag[i]=1;
					else // it's a critical branch node
					{
						swcTree::getIndex(node[i].nid, curnodeidx);			
						tmpTreeNodeIdx[newTreeNodeNum] = curnodeidx;
						newTreeNodeNum++;
					} 
	//				printf("removeNodeTag=%d\n", removeNodeTag[i]);
					if (lengthratio!=0) {delete []lengthratio; lengthratio=0;}	
				}
				else if (criticalNodeMethod ==2) // the third definition of critical branch node: summed edge length in the subtree/diamter of tree
				{
				
					float *totalEdgeLengthInSubTree = new float [childrenNum[i]];
					unsigned char bigBranchNum = 0;
					float *childrenEdgeLength = 0;
					unsigned char lengthNum;
					swcTree::computeLength(node[i].nid, 0, childrenEdgeLength, lengthNum);
					
					for (j=0; j<childrenNum[i]; j++)
					{
					
						float maxlength, minlength, totallength;
						V3DLONG maxLengthNodeid, minLengthNodeid;
						
						swcTree::getSubTreeLength(childrenList[i][j], maxlength, maxLengthNodeid, minlength, minLengthNodeid, totallength); //note totallength =0 if childrenList[i][j] is a leaf node
						totalEdgeLengthInSubTree[j] = totallength;
	
		//				void getSubTreeLength(V3DLONG nodeid, float &maxlength, V3DLONG &maxLengthNodeid, float &minlength, V3DLONG &minLengthNodeid, float &totalLength); // get the maximum, minimum length between nodeid and a node, and the total length of the subtree rooted at nodeid 
//						
						if ((totalEdgeLengthInSubTree[j]+childrenEdgeLength[j])/treeDiameter>threVector[0])
							bigBranchNum++;
						
						if (bigBranchNum==2)
							break;
					}
					
					if (bigBranchNum<2) 
						removeNodeTag[i]=1;
					else
					{
						swcTree::getIndex(node[i].nid, curnodeidx);			
						tmpTreeNodeIdx[newTreeNodeNum] = curnodeidx;
						newTreeNodeNum++;
						
					} 
					
					if (totalEdgeLengthInSubTree) {delete []totalEdgeLengthInSubTree; totalEdgeLengthInSubTree=0;}	
					if (childrenEdgeLength) {delete []childrenEdgeLength; childrenEdgeLength=0;}
				}
				else if (criticalNodeMethod ==3) // the forth definition of critical branch node: summed edge length in the subtree/summed edge length of tree
				{
				
					float *totalEdgeLengthInSubTree = new float [childrenNum[i]];
					unsigned char bigBranchNum = 0;
					float *childrenEdgeLength = 0;
					unsigned char lengthNum;
					swcTree::computeLength(node[i].nid, 0, childrenEdgeLength, lengthNum);
					
					for (j=0; j<childrenNum[i]; j++)
					{
											
						float maxlength, minlength,totallength;
						V3DLONG maxLengthNodeid, minLengthNodeid;
						
						swcTree::getSubTreeLength(childrenList[i][j], maxlength, maxLengthNodeid, minlength, minLengthNodeid, totallength); //note that totallength = 0 if childrenList[i][j] is a leaf node 
						totalEdgeLengthInSubTree[j] = totallength;
						if ((totalEdgeLengthInSubTree[j]+childrenEdgeLength[j])/treeEdgeLength>threVector[0])
						{
							bigBranchNum++;
//							printf("i=%d, totalEdgeLengthInSubTree = %f, childrenEdgeLength=%f, treeEdgeLength=%f\n", i, totalEdgeLengthInSubTree[j], childrenEdgeLength[j], treeEdgeLength);
						}
						if (bigBranchNum==2)
							break;
					}
					
					if (bigBranchNum<2) 
						removeNodeTag[i]=1;
					else
					{
						swcTree::getIndex(node[i].nid, curnodeidx);			
						tmpTreeNodeIdx[newTreeNodeNum] = curnodeidx;
						newTreeNodeNum++;
					} 
					
					if (totalEdgeLengthInSubTree) {delete []totalEdgeLengthInSubTree; totalEdgeLengthInSubTree=0;}	
					if (childrenEdgeLength) {delete []childrenEdgeLength; childrenEdgeLength=0;}
					
				}		
				else
				{
					printf("criticalNodeMethod>3 not defined\n");
					exit(1);
				}
					
			} // if (childrenNum[i]<2)
		} //if (i==rootidx)
//		printf("i=%d\n", i);
	} // for (i=0; i<treeNodeNum; i++)

	// ---------------------------------------------------------------	
    // add some leaf nodes into the critical point list if leafMethod>0		
	// ---------------------------------------------------------------	

	V3DLONG leafNodeNum = 0;
	
	if (leafMethod >0) // add leaf node
	{
	
		
		for (j=0; j<newTreeNodeNum; j++)
		{
			V3DLONG *childrenID=0, *childrenNodeIdx=0, cnum;
			swcTree::getDirectChildren(node[tmpTreeNodeIdx[j]].nid, childrenID, childrenNodeIdx, cnum); 
			
			for (m=0; m<cnum; m++) // check each branch
			{
				V3DLONG thisnode = childrenID[m];
				V3DLONG thisnodeidx;
				swcTree::getIndex(thisnode, thisnodeidx);
				
				swcTree *subtree = 0;
				V3DLONG *subtreeNodeIdx = 0;
				swcTree::getSubTree(thisnode, subtree, subtreeNodeIdx);
				
				bool hitTag = 0; //hitTag = 1 if a critical point is hit in the subtree
				
				for (int p =0; p<subtree->treeNodeNum; p++)
				{
					if (hitTag == 0)
					{
						for (int q =0; q<newTreeNodeNum; q++)
						{
							if (subtree->node[p].nid == node[tmpTreeNodeIdx[q]].nid)
							{
								hitTag = 1;
								break;
							}
						}
					}
					else
						break;
				}
				
				if (hitTag == 1) // do nothing to the current critical branch node 
					continue;
				else // add leaf node to the current critical branch node
				{
					if (leafMethod==1) // use the leaf node in the longest path of the subtree
					{
						float maxlength, minlength, totallength;
						V3DLONG maxLengthNodeid, minLengthNodeid;
						
						swcTree::getSubTreeLength(childrenID[m], maxlength, maxLengthNodeid, minlength, minLengthNodeid, totallength);
						
						V3DLONG leafnodeidx;
						
						if (totallength==0) // childrenID[m] is a leaf node
						{
							
							leafnodeidx = childrenNodeIdx[m];
							leafNodeData[leafNodeNum*7] = node[leafnodeidx].nid;
							leafNodeData[leafNodeNum*7+1] = node[leafnodeidx].ntype;
							leafNodeData[leafNodeNum*7+2] = node[leafnodeidx].x;
							leafNodeData[leafNodeNum*7+3] = node[leafnodeidx].y;
							leafNodeData[leafNodeNum*7+4] = node[leafnodeidx].z;
							leafNodeData[leafNodeNum*7+5] = node[leafnodeidx].radius;
//							leafNodeData[leafNodeNum*7+6] = node[leafnodeidx].parentid; //parent
							leafNodeData[leafNodeNum*7+6] = node[tmpTreeNodeIdx[j]].nid; //parent

						
						}
						else
						{
							swcTree::getIndex(maxLengthNodeid, leafnodeidx);			

							leafNodeData[leafNodeNum*7] = node[leafnodeidx].nid;
							leafNodeData[leafNodeNum*7+1] = node[leafnodeidx].ntype;
							leafNodeData[leafNodeNum*7+2] = node[leafnodeidx].x;
							leafNodeData[leafNodeNum*7+3] = node[leafnodeidx].y;
							leafNodeData[leafNodeNum*7+4] = node[leafnodeidx].z;
							leafNodeData[leafNodeNum*7+5] = node[leafnodeidx].radius;
							leafNodeData[leafNodeNum*7+6] = node[tmpTreeNodeIdx[j]].nid; //parent
						}
						
						leafNodeNum++;

					}
					else if (leafMethod==2) // use center of subtree node coordiante
					{
					
						float *center=0;
						float averageRadius;
						subtree->computeTreeCenter(center, averageRadius);

						leafNodeData[leafNodeNum*7] = treeNodeNum+leafNodeNum+1; //id						
						leafNodeData[leafNodeNum*7+1] = node[thisnodeidx].ntype;
						leafNodeData[leafNodeNum*7+2] = center[0];
						leafNodeData[leafNodeNum*7+3] = center[1];
						leafNodeData[leafNodeNum*7+4] = center[2];
						leafNodeData[leafNodeNum*7+5] = averageRadius;
						leafNodeData[leafNodeNum*7+6] = node[tmpTreeNodeIdx[j]].nid; //parent

						leafNodeNum++;
						if (center) {delete []center; center=0;}
						
					}
						
				}
				
				if (subtree) {delete subtree; subtree = 0;}
				if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx = 0;}
			}
			
			if (childrenID) {delete [] childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete [] childrenNodeIdx; childrenNodeIdx= 0;}
		}
	}
	
	// ----------------------------------------------------------------------------------------
	// generate new tree that contains only critical branch nodes (and leaf nodes, if leafNodeNum>0)
	// continual nodes between critical branch nodes are removed. As the result,
	// critical branch nodes are connected by straight lines
	// ----------------------------------------------------------------------------------------
	
//	newTreeNodeNum += newTreeNodeLeafNum;
	newTree = new swcTree(newTreeNodeNum+leafNodeNum); // allocat memory for nodes
	newTree->treeNodeNum = newTreeNodeNum+leafNodeNum;
	
	for (int i=0; i<newTreeNodeNum; i++)
	{
			
		newTree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		newTree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		newTree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		newTree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		newTree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		newTree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		
		// find the right parenet
		curnodeidx = tmpTreeNodeIdx[i];
//			printf("%d\n", curnodeidx);
		
		if (curnodeidx == rootidx)
			newTree->node[i].parentid = -1;
		else
		{
			swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);	
//			printf("%d\n", curnodeidx);
					
			while (removeNodeTag[curnodeidx]==1)
			{
				swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);
//				printf("%d, %d, %d\n", i, tmpTreeNodeIdx[i], curnodeidx);
			}
			
			newTree->node[i].parentid = node[curnodeidx].nid;	
		}
		
//			printf("%d, %d, %d\n", i, newTree->node[i].nid, newTree->node[i].parentid);		
//			newTreeNodeIdx[i] = tmpTreeNodeIdx[i];
	}
	
	if (leafNodeNum>0)
	{
		for (i=0; i<leafNodeNum; i++)
		{
						
			newTree->node[newTreeNodeNum+i].nid = leafNodeData[i*7];
			newTree->node[newTreeNodeNum+i].ntype = leafNodeData[i*7+1];
			newTree->node[newTreeNodeNum+i].x = leafNodeData[i*7+2];
			newTree->node[newTreeNodeNum+i].y = leafNodeData[i*7+3];
			newTree->node[newTreeNodeNum+i].z = leafNodeData[i*7+4];
			newTree->node[newTreeNodeNum+i].radius = leafNodeData[i*7+5];
			newTree->node[newTreeNodeNum+i].parentid = leafNodeData[i*7+6];
		}
	}
	
	newTreeNodeNum += leafNodeNum;
	
	// ----------------------------------------------------------------------------------------
	// generate new tree that contains not only critical branch nodes (and leaf nodes, if leafNodeNum>0)
	// but also continual nodes between critical nodes. This is to better visualize the tree 
	// defined on critical branch nodes. In other words, critical nodes are not connected simply by
	// straight lines, but by the path defined by a series of continual nodes in the original tree
	// ----------------------------------------------------------------------------------------
	
	
	V3DLONG newTreeWithPathNodeNum = 0;
	
	for (int i=0; i<treeNodeNum; i++)
		tmpTreeNodeIdx[i] = INVALID_VALUE;
	
	bool *addedTag = new bool [treeNodeNum];
	
	for (int i=0; i<treeNodeNum; i++)
		addedTag[i] = 0;
	
	for (int i=0; i<newTreeNodeNum; i++) // note now newTreeNodeNum include leafNodeNum
	{

		// get the index of the critical branch node in the original tree
		V3DLONG idx;		
		getIndex(newTree->node[i].nid, idx);
		// add this node into the tree, which is either a critical branch node or a branch node
		tmpTreeNodeIdx[newTreeWithPathNodeNum] = idx;
		newTreeWithPathNodeNum++;
		addedTag[idx]=1;
		//get the parent of the critical branch node in the original tree
		V3DLONG parentNodeid = node[idx].parentid; 
		
		if (parentNodeid!=-1)
		{
			V3DLONG parentNodeIdx;

			while (parentNodeid!=-1)  
			{
				getIndex(parentNodeid, parentNodeIdx);

				if ((removeNodeTag[parentNodeIdx]==1)&&(addedTag[parentNodeIdx]==0)) // not reached root or a critical branch node, and the node has not been added into the tree
				{
					parentNodeid = node[parentNodeIdx].parentid;
					tmpTreeNodeIdx[newTreeWithPathNodeNum] = parentNodeIdx;
					newTreeWithPathNodeNum++;
					addedTag[parentNodeIdx]=1;
				}
				else
				{
					if (addedTag[parentNodeIdx]==0) //have not add the critical node into the tree yet
					{
						tmpTreeNodeIdx[newTreeWithPathNodeNum] = parentNodeIdx;
						newTreeWithPathNodeNum++;
						addedTag[parentNodeIdx]=1;
					}
					break;
				}
			}
		}
//		printf("%d, %d\n", i, Tree1->node[0].nid);		
	}
	
	if (addedTag) {delete []addedTag; addedTag = 0;}
	
    newTreeWithPath = new swcTree(newTreeWithPathNodeNum);
	for (i=0; i<newTreeWithPathNodeNum; i++)
	{
		newTreeWithPath->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		newTreeWithPath->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		newTreeWithPath->node[i].x = node[tmpTreeNodeIdx[i]].x;
		newTreeWithPath->node[i].y = node[tmpTreeNodeIdx[i]].y;
		newTreeWithPath->node[i].z = node[tmpTreeNodeIdx[i]].z;
		newTreeWithPath->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		newTreeWithPath->node[i].parentid = node[tmpTreeNodeIdx[i]].parentid;
		
	}
			   
	// ----------------------------
	// generate removeBranchTag
	// ----------------------------

	//allocate memory
	V3DLONG *removeBranchTag1d = new V3DLONG [newTree->treeNodeNum*MAX_CHILDREN_NUM];
	
	if (!removeBranchTag1d)
	{ 
		printf("fail to allocate memory for removeBranchTag1d in swcTree::removeInsignificantNodes\n");
		return;
	}
	
	if (!new2dpointer(removeBranchTag, MAX_CHILDREN_NUM, newTree->treeNodeNum, removeBranchTag1d))
	{
		printf("fail to allocate memory for removeBranchTag1d in swcTree::removeInsignificantNodes\n");
		return;		   		   
	}

	for (i=0; i<newTree->treeNodeNum;i++)
	{
		for (j=0; j<MAX_CHILDREN_NUM; j++)
			removeBranchTag[i][j] = INVALID_VALUE;
		
		V3DLONG *childrenID_newTree =0, *childrenNodeIdx_newTree =0, childrenNum_newTree;
		V3DLONG *childrenID =0, *childrenNodeIdx =0, childrenNum;
		
		newTree->getDirectChildren(newTree->node[i].nid, childrenID_newTree, childrenNodeIdx_newTree, childrenNum_newTree); // get the direct children of nodeid in new tree
		getDirectChildren(newTree->node[i].nid, childrenID, childrenNodeIdx, childrenNum); // get the direct children of nodeid in old tree
		
		for (j=0; j<childrenNum; j++)
		{
		
			swcTree *subtree;
			V3DLONG *subtreeNodeIdx;
			
			getSubTree(childrenID[j], subtree, subtreeNodeIdx);
			
			m= 0;
			while (m<subtree->treeNodeNum)
			{
				n = 0;
				while (n< childrenNum_newTree)
				{
					if (subtree->node[m].nid == childrenID_newTree[n])
					{	
//						removeBranchTag[i][j] = 0; // not removed
						removeBranchTag[i][j] = n; // not removed

						break;
					}
					n++;
				}
//				if (removeBranchTag[i][j] == 0)
				if (removeBranchTag[i][j] >= 0)
					break;
				else
					m++;
			}

			if (removeBranchTag[i][j] == INVALID_VALUE)
//				removeBranchTag[i][j] = 1; // the branch is removed
				removeBranchTag[i][j] = -1; // the branch is removed
			
			if (subtree) {delete subtree; subtree =0;}
			if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx =0;}
				
		}
		
		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}		
		if (childrenID_newTree) {delete []childrenID_newTree; childrenID_newTree=0;}
		if (childrenNodeIdx_newTree) {delete []childrenNodeIdx_newTree; childrenNodeIdx_newTree=0;}
		
	}

	// ----------------------------	
	// delete pointers
	// ----------------------------
	
//	if (removeBranchTag1d) {delete []removeBranchTag1d; removeBranchTag1d=0;}
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	if (childrenNum) {delete []childrenNum; childrenNum=0;}
	if (leafNodeData) {delete []leafNodeData; leafNodeData =0;}

}


//remove all continual nodes
// removeTag indicates which node in the original tree is removed

void swcTree::removeContinualNodes(swcTree *&newTree, unsigned char *&removeTag)
{
	V3DLONG **childrenList = 0;
	V3DLONG *childrenNum = 0;
	
//	unsigned char maxChildrenNum =6;
//	swcTree *tmpTree = new swcTree [1];
	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	V3DLONG newTreeNodeNum = 0;
	int i,j;
	V3DLONG curnodeidx, curnodeid, rootid, rootidx;
	removeTag = new unsigned char [treeNodeNum];
	
//	// initialize tmpTree
//	tmpTree->treeNodeNum = 0;
//	tmpTree->node = new swcNode [treeNodeNum];
	
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM);

	swcTree:getRootID(rootid, rootidx);
		
	for (i=0; i<treeNodeNum; i++)
	{
		removeTag[i] = 0;
	
		if ((childrenNum[i]!=1) || (i==rootidx)) // do not remove root
		{
			newTreeNodeNum++;
			swcTree::getIndex(node[i].nid, curnodeidx);			
			tmpTreeNodeIdx[newTreeNodeNum-1] = curnodeidx;
		}
		else
			removeTag[i] = 1;
	}
	
	
//	//generate newTree
//	newTreeNodeIdx = new V3DLONG [newTreeNodeNum];
	
//	newTree = new swcTree [1];
//	newTree->swcTree(newTreeNodeNum); // allocat memory for nodes
	newTree = new swcTree(newTreeNodeNum); // allocat memory for nodes

	newTree->treeNodeNum = newTreeNodeNum;
//	printf("%d\n", newTreeNodeNum);
	
	for (int i=0; i<newTreeNodeNum; i++)
	{
			
		newTree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		newTree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		newTree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		newTree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		newTree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		newTree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		
		// find the right parenet
		curnodeidx = tmpTreeNodeIdx[i];
//			printf("%d\n", curnodeidx);
		
		if (curnodeidx == rootidx)
			newTree->node[i].parentid = -1;
		else
		{
			swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);	
//			printf("%d\n", curnodeidx);
					
			while (removeTag[curnodeidx]==1)
			{
				swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);
//				printf("%d, %d, %d\n", i, tmpTreeNodeIdx[i], curnodeidx);
			}
			
			newTree->node[i].parentid = node[curnodeidx].nid;	
		}
		
//			printf("%d, %d, %d\n", i, newTree->node[i].nid, newTree->node[i].parentid);		
//			newTreeNodeIdx[i] = tmpTreeNodeIdx[i];
	}
	
	
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	if (childrenNum) {delete []childrenNum; childrenNum=0;}

}


//remove all continual nodes and leaf nodes
// this function is specificly needed when matchig two trees based on branching points,
// and similarity score is only defined on branching points
// removeTag indicates which node in the original tree is removed

void swcTree::removeContinuaLeaflNodes(swcTree *&newTree, unsigned char *&removeTag)
{
	V3DLONG **childrenList = 0;
	V3DLONG *childrenNum = 0;
	
//	unsigned char maxChildrenNum =6;
//	swcTree *tmpTree = new swcTree [1];
	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	V3DLONG newTreeNodeNum = 0;
	int i,j;
	V3DLONG curnodeidx, curnodeid, rootid, rootidx;
	removeTag = new unsigned char [treeNodeNum];
	
//	// initialize tmpTree
//	tmpTree->treeNodeNum = 0;
//	tmpTree->node = new swcNode [treeNodeNum];
	
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM);

	swcTree:getRootID(rootid, rootidx);
		
	for (i=0; i<treeNodeNum; i++)
	{
		removeTag[i] = 0;
	
		if ((childrenNum[i]>=2) || (i==rootidx)) // keep branching nodes and root
		{
			newTreeNodeNum++;
			swcTree::getIndex(node[i].nid, curnodeidx);			
			tmpTreeNodeIdx[newTreeNodeNum-1] = curnodeidx;
		}
		else // remove continual nodes and leaf nodes
			removeTag[i] = 1;
	}
	
	
	newTree = new swcTree(newTreeNodeNum); // allocat memory for nodes

	newTree->treeNodeNum = newTreeNodeNum;
	
	for (int i=0; i<newTreeNodeNum; i++)
	{
			
		newTree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		newTree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		newTree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		newTree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		newTree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		newTree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		
		// find the right parenet
		curnodeidx = tmpTreeNodeIdx[i];
//			printf("%d\n", curnodeidx);
		
		if (curnodeidx == rootidx)
			newTree->node[i].parentid = -1;
		else
		{
			swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);	
//			printf("%d\n", curnodeidx);
					
			while (removeTag[curnodeidx]==1)
			{
				swcTree::getIndex(node[curnodeidx].parentid, curnodeidx);
//				printf("%d, %d, %d\n", i, tmpTreeNodeIdx[i], curnodeidx);
			}
			
			newTree->node[i].parentid = node[curnodeidx].nid;	
		}
		
//			printf("%d, %d, %d\n", i, newTree->node[i].nid, newTree->node[i].parentid);		
//			newTreeNodeIdx[i] = tmpTreeNodeIdx[i];
	}
	
	
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	if (childrenNum) {delete []childrenNum; childrenNum=0;}

}


// get subtree rooted at nodeid, including nodeid
// subtree is the returned sub tree
// subtreeNodeidx is the index of the sub tree nodes in the orginal tree
void swcTree::getSubTree(V3DLONG nodeid, swcTree *&subtree, V3DLONG *&subtreeNodeIdx)
{

	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	
	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid;
	int i,j;
	V3DLONG childrenNum;
	V3DLONG curnodeidx;
	V3DLONG subtreeNodeNum = 0;
	
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
		nodeQueue[i]= INVALID_VALUE;
	
	nodeQueue[0] = nodeid;
	headptr = 0;
	tailptr = 0;
	
	//get the first element from the queue
	curnodeid = nodeQueue[headptr];	
	
	while ((curnodeid!=INVALID_VALUE)&&(subtreeNodeNum<treeNodeNum)) //there is still element in the queue
	{	
		subtreeNodeNum++;
		
		swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
		tmpTreeNodeIdx[subtreeNodeNum-1] = curnodeidx; // add the current node index
		
		// add children of the current node to the tail of the queue
		V3DLONG *childrenID = 0;
		V3DLONG *childrenNodeIdx = 0;

		swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);

#ifdef DEBUG_TREE
		
		printf("childrenNum = %d, subtreeNodeNum=%d \n", childrenNum, subtreeNodeNum);
		
		for (j=0;j<treeNodeNum; j++)
			printf("%d ", nodeQueue[j]);
		printf("\n");
#endif		
		j = 0;
		while (j<childrenNum) 
		{			
			tailptr++;
			nodeQueue[tailptr]=childrenID[j];
			j++;
		}
		
		//get a node from the head of the queue
		headptr++;
		curnodeid = nodeQueue[headptr];

		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
		
	}
	
	// assign output values
//	subtree = new swcTree [1];
//	subtree->swcTree(subtreeNodeNum); // allocate memory for nodes
	subtree = new swcTree(subtreeNodeNum);
	
	subtreeNodeIdx = new V3DLONG [subtreeNodeNum];
	
	for (i=0; i<subtreeNodeNum; i++)
	{
		subtree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		subtree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		subtree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		subtree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		subtree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		subtree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		subtree->node[i].parentid = node[tmpTreeNodeIdx[i]].parentid;	
		
		subtree->treeNodeNum = subtreeNodeNum;
		subtreeNodeIdx[i] = tmpTreeNodeIdx[i];
		
	}

#ifdef DEBUG_TREE
	
	printf("sub tree node nmber = %d\n", subtreeNodeNum);
	
	printf("sub tree nodes:\n");
	for (i=0; i<subtreeNodeNum; i++)
		printf("%d ", subtreeNodeIdx[i]);
	printf("\n");

#endif
	
	// delete pointer
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}

}


// get the partial subtree rooted at nodeid, including nodeid
// nodes within the partial subtree have a depth no bigger then depthThre with respect to the root nodeid
// subtree is the returned sub tree
// subtreeNodeidx is the index of the sub tree nodes in the orginal tree
void swcTree::getSubTree(V3DLONG nodeid, V3DLONG depthThre, swcTree *&subtree, V3DLONG *&subtreeNodeIdx)
{

	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	
	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid;
	int i,j;
	V3DLONG childrenNum;
	V3DLONG curnodeidx;
	V3DLONG subtreeNodeNum = 0;
	V3DLONG *depth = new V3DLONG [treeNodeNum];
	
	V3DLONG curDepth = 0;
	
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
	{
		nodeQueue[i]= INVALID_VALUE;
		depth[i] = 0;
	}
	
	nodeQueue[0] = nodeid;
	headptr = 0;
	tailptr = 0;
	
	//get the first element from the queue
	curnodeid = nodeQueue[headptr];	
	
	while ((curnodeid!=INVALID_VALUE)&&(subtreeNodeNum<treeNodeNum)&&(curDepth<depthThre)) //there is still element in the queue
	{	
		subtreeNodeNum++;
		
		swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node		
		tmpTreeNodeIdx[subtreeNodeNum-1] = curnodeidx; // add the current node index
		
		// add children of the current node to the tail of the queue
		V3DLONG *childrenID = 0;
		V3DLONG *childrenNodeIdx = 0;

		swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);

#ifdef DEBUG_TREE
		
		printf("childrenNum = %d, subtreeNodeNum=%d \n", childrenNum, subtreeNodeNum);
		
		for (j=0;j<treeNodeNum; j++)
			printf("%d ", nodeQueue[j]);
		printf("\n");
#endif		
		j = 0;
		while (j<childrenNum) 
		{			
			tailptr++;
			nodeQueue[tailptr]=childrenID[j];			
			
			depth[childrenNodeIdx[j]] = depth[curnodeidx]+1;	
			curDepth = 	depth[childrenNodeIdx[j]];
//			printf("curDepth = %d, depthThre = %d\n", curDepth, depthThre);
			
			j++;
			
		}
		
		//get a node from the head of the queue
		headptr++;
		curnodeid = nodeQueue[headptr];

		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
	}
	
	// assign output values
//	subtree = new swcTree [1];
//	subtree->swcTree(subtreeNodeNum); // allocate memory for nodes
	subtree = new swcTree(subtreeNodeNum);
	
	subtreeNodeIdx = new V3DLONG [subtreeNodeNum];
	
	for (i=0; i<subtreeNodeNum; i++)
	{
		subtree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		subtree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		subtree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		subtree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		subtree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		subtree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		subtree->node[i].parentid = node[tmpTreeNodeIdx[i]].parentid;	
		
		subtree->treeNodeNum = subtreeNodeNum;
		subtreeNodeIdx[i] = tmpTreeNodeIdx[i];
		
	}

#ifdef DEBUG_TREE
	
	printf("sub tree node nmber = %d\n", subtreeNodeNum);
	
	printf("sub tree nodes:\n");
	for (i=0; i<subtreeNodeNum; i++)
		printf("%d ", subtreeNodeIdx[i]);
	printf("\n");

#endif
	
	// delete pointer
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
	if (depth) {delete []depth; depth = 0;}

}


// get the partial subtree rooted at nodeid, including nodeid
// nodes within the partial subtree have a lengthRatioThre (the length from the current node to the nodeid /
//  the maximum length of the tree from nodeid) is less then depthRatioThre with respect to the root nodeid
// subtree is the returned sub tree
// subtreeNodeidx is the index of the sub tree nodes in the orginal tree
void swcTree::getSubTree(V3DLONG nodeid, float subtreeRatioThre, swcTree *&subtree, V3DLONG *&subtreeNodeIdx)
{

	V3DLONG *tmpTreeNodeIdx = new V3DLONG [treeNodeNum];
	
	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid;
	int i,j;
	V3DLONG childrenNum;
	V3DLONG curnodeidx;
	V3DLONG subtreeNodeNum = 0;

//	V3DLONG *depth = new V3DLONG [treeNodeNum];	
//	V3DLONG curDepth = 0;	
	
	float curLengthRatio = 0;
	
	// compute the maximum length of the subtree rooted at nodeid, i.e., maxlength
	float maxlength, minlength, totalLength; 
	V3DLONG maxLengthNodeid, minLengthNodeid;
	
	swcTree::getSubTreeLength(nodeid, maxlength, maxLengthNodeid, minlength, minLengthNodeid, totalLength);	
	
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
	{
		nodeQueue[i]= INVALID_VALUE;
//		depth[i] = 0;
	}
	
	nodeQueue[0] = nodeid;
	headptr = 0;
	tailptr = 0;
	
	//get the first element from the queue
	curnodeid = nodeQueue[headptr];	
	
	while ((curnodeid!=INVALID_VALUE)&&(subtreeNodeNum<treeNodeNum)&&(curLengthRatio<subtreeRatioThre)) //there is still element in the queue
	{	
		subtreeNodeNum++;
		
		swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node		
		tmpTreeNodeIdx[subtreeNodeNum-1] = curnodeidx; // add the current node index
		
		// add children of the current node to the tail of the queue
		V3DLONG *childrenID = 0;
		V3DLONG *childrenNodeIdx = 0;

		swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);

#ifdef DEBUG_TREE
		
		printf("childrenNum = %d, subtreeNodeNum=%d \n", childrenNum, subtreeNodeNum);
		
		for (j=0;j<treeNodeNum; j++)
			printf("%d ", nodeQueue[j]);
		printf("\n");
#endif		
		j = 0;
		while (j<childrenNum) 
		{		
			// compute the length of childrenID[j] to nodeid
			float length;
			computeLength(nodeid, childrenID[j], length);
			curLengthRatio = length/maxlength;
			
			if (curLengthRatio<subtreeRatioThre)
			{
				tailptr++;
				nodeQueue[tailptr]=childrenID[j];			
				
//				depth[childrenNodeIdx[j]] = depth[curnodeidx]+1;	
//				curDepth = 	depth[childrenNodeIdx[j]];
//	//			printf("curDepth = %d, depthThre = %d\n", curDepth, depthThre);
			}
			
			j++;
			
		}
		
		//get a node from the head of the queue
		headptr++;
		curnodeid = nodeQueue[headptr];

		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
	}
	
	// assign output values
//	subtree = new swcTree [1];
//	subtree->swcTree(subtreeNodeNum); // allocate memory for nodes
	subtree = new swcTree(subtreeNodeNum);
	
	subtreeNodeIdx = new V3DLONG [subtreeNodeNum];
	
	for (i=0; i<subtreeNodeNum; i++)
	{
		subtree->node[i].nid = node[tmpTreeNodeIdx[i]].nid;
		subtree->node[i].ntype = node[tmpTreeNodeIdx[i]].ntype;
		subtree->node[i].x = node[tmpTreeNodeIdx[i]].x;
		subtree->node[i].y = node[tmpTreeNodeIdx[i]].y;
		subtree->node[i].z = node[tmpTreeNodeIdx[i]].z;
		subtree->node[i].radius = node[tmpTreeNodeIdx[i]].radius;
		subtree->node[i].parentid = node[tmpTreeNodeIdx[i]].parentid;	
		
		subtree->treeNodeNum = subtreeNodeNum;
		subtreeNodeIdx[i] = tmpTreeNodeIdx[i];
		
	}

#ifdef DEBUG_TREE
	
	printf("sub tree node nmber = %d\n", subtreeNodeNum);
	
	printf("sub tree nodes:\n");
	for (i=0; i<subtreeNodeNum; i++)
		printf("%d ", subtreeNodeIdx[i]);
	printf("\n");

#endif
	
	// delete pointer
	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
//	if (depth) {delete []depth; depth = 0;}

}


// get the number of nodes in the subtree rooted at each node
// instead of calling getSubTree on each node, here a more efficient approach using dynamic programming is taken
void swcTree::getSubTreeNodeNum(V3DLONG *&subTreeNodeNum)
{
	int i,j;
	subTreeNodeNum = new V3DLONG [treeNodeNum];
	V3DLONG *directChildrenNum = new V3DLONG [treeNodeNum];
	V3DLONG *sortednodeidx=0;
	
	swcTree::sortTree(sortednodeidx);

#ifdef DEBUG_TREE
	
	printf("sortednodeidx:");
	
	for (i=0; i<treeNodeNum; i++)
		printf("%d, %d\n ", i, sortednodeidx[i]);
	printf("\n");
#endif
	
	// compute the number of direct children for each node
	for (i=0; i<treeNodeNum; i++)
	{
		directChildrenNum[i] = 0;
		subTreeNodeNum[i] = 0;
	}
	
	for (i=0; i<treeNodeNum; i++)
	{
		if (node[i].parentid!=-1)
		{	V3DLONG idx;
			swcTree::getIndex(node[i].parentid, idx);
//			printf("%d, %d\n", node[i].parentid, idx);
			
			directChildrenNum[idx] += 1; // assuming that the starting id of the node is 1, not 0
//			directChildrenNum[node[i].parentid-1] += 1; // assuming that the starting id of the node is 1, not 0
		}
	}


#ifdef DEBUG_TREE

	printf("directChildrenNum printed in getSubTreeNodeNum(V3DLONG *&subTreeNodeNum):\n");
	for (i=0; i<treeNodeNum; i++)
		printf("%d %d\n ", i, directChildrenNum[i]);
	printf("\n");

#endif
	
	// compute the number of nodes in the subtree rooted at each point by 
	// accumulating the number of direct chidren of nodes in the subtree
	// this is done in dynamic programming style
	
	for (i=treeNodeNum-1; i>=0; i--) //assmuing smaller id has shallower depth
	{
		if (directChildrenNum[i] == 0) //leaf node
			subTreeNodeNum[i] = 1;
		else
		{
			for (j=treeNodeNum-1; j>=0; j--) 
			{
				if (node[j].parentid == node[i].nid) // i is j's parent
					subTreeNodeNum[i] += subTreeNodeNum[j];
			}
			subTreeNodeNum[i]++; // add itself
		}
	}
	

#ifdef DEBUG_TREE	

	printf("subTreeNodeNum printed in getSubTreeNodeNum(V3DLONG *&subTreeNodeNum):\n");

	for (i=0; i<treeNodeNum; i++)
		printf("%d, %d\n ", i,subTreeNodeNum[i]);
	printf("\n");
	
#endif	
	
	if (directChildrenNum) {delete []directChildrenNum; directChildrenNum=0;}
	
}


// get the number of nodes of the sub tree rooted at a particular node
void swcTree::getSubTreeNodeNum(V3DLONG nodeid, V3DLONG &subTreeNodeNum)
{
	swcTree *subtree = 0;
	V3DLONG *subtreeNodeIdx = 0;
	 
	swcTree::getSubTree(nodeid, subtree, subtreeNodeIdx);
	subTreeNodeNum = subtree->treeNodeNum;
	
}


// get the maximum and minimum length between nodeid and a node in its subtree
// also get the total length of the subtree rooted at nodeid
// maxlength: the maximum length
// maxLengthNodeid: the id of the node that has the maximum length from nodeid in the subtree rooted at nodeid
// minlength: the maximum length
// minLengthNodeid: the id of the node that has the maximum length from nodeid in the subtree rooted at nodeid
// totalLength is the sum of the length of branch between parenet and children, not between root and any node
// note if maxlength = INVALID_VALUE, minlength = -INVALID_VALUE, maxLengthNodeid = INVALID_VALUE, minLengthNodeid = INVALID_VALUE, then
// the subtree only contains one node, which is a leaf node
void swcTree::getSubTreeLength(V3DLONG nodeid, float &maxlength, V3DLONG &maxLengthNodeid, float &minlength, V3DLONG &minLengthNodeid, float &totalLength)
{
	
	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid;
	int i,j;
	V3DLONG childrenNum;
	V3DLONG curnodeidx;
	V3DLONG subtreeNodeNum = 0;
	float *length = new float [treeNodeNum];
	
	maxLengthNodeid = -1;
	minLengthNodeid = -1;

	totalLength = 0;
	maxlength = INVALID_VALUE; //INVALID VALUE is -9999
	minlength = -INVALID_VALUE;
	
	maxLengthNodeid = INVALID_VALUE;
	minLengthNodeid = INVALID_VALUE;
	
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
		nodeQueue[i]= INVALID_VALUE;
	
	nodeQueue[0] = nodeid;
	headptr = 0;
	tailptr = 0;

	totalLength = 0;
	
	length[headptr] = 0;
	maxlength = 0;
	minlength = 99999;
	
	//get the first element from the queue
	curnodeid = nodeQueue[headptr];	

//		printf("nodeid = %d, curnodeid = %d\n", nodeid,curnodeid);
	
	while ((curnodeid!=INVALID_VALUE)&&(subtreeNodeNum<treeNodeNum)) //there is still element in the queue
	{	
		subtreeNodeNum++;
		
		swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
//		tmpTreeNodeIdx[subtreeNodeNum-1] = curnodeidx; // add the current node index
		
		// add children of the current node to the tail of the queue
		V3DLONG *childrenID = 0;
		V3DLONG *childrenNodeIdx = 0;

		swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);
		
//		printf("curnodeid = %d, childrenNum = %d \n", curnodeid, childrenNum);
		
		
		j = 0;
		while (j<childrenNum) 
		{	

			tailptr++;
		
			// compute length
			
			float lengthtmp;
			swcTree::computeLength(curnodeid, childrenID[j], lengthtmp);
			length[tailptr] = length[headptr] + lengthtmp;
			
			
			totalLength += lengthtmp; // note totalLength only sum up the length of branch between parenet and children, not between root and any node
			
			if (length[tailptr]>maxlength)
			{
				maxlength = length[tailptr];
				maxLengthNodeid = childrenID[j];
			}

//			printf("length[headptr] = %f, lengthtmp = %f, length[tailptr] = %f, maxlength = %f\n", length[headptr], lengthtmp, length[tailptr], maxlength);
			
			if (length[tailptr]<minlength)
			{
				minlength = length[tailptr];
				minLengthNodeid = childrenID[j];
			}
			
			
			nodeQueue[tailptr]=childrenID[j];
			j++;
		} // while (j<childrenNum) 
		
//		if ((maxlength == 0)&&(minlength == 99999)) // the subtree only contain one node, which means this node is a leaf node 
//			minlength = 0;
//
		//get a node from the head of the queue
		headptr++;
		curnodeid = nodeQueue[headptr];

		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
	}
		
	// delete pointer
//	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
	if (length) {delete []length; length=0;}
}

// get the maximum, second maximum, and minimum length between nodeid and a node in its subtree
// also get the total length of the subtree rooted at nodeid
// note if maxlength = INVALID_VALUE, minlength = -INVALID_VALUE, maxLengthNodeid = INVALID_VALUE, minLengthNodeid = INVALID_VALUE, then
// the subtree only contains one node, which is a leaf node

void swcTree::getSubTreeLength(V3DLONG nodeid, float &maxlength, V3DLONG &maxLengthNodeid, float &secondmaxlength, V3DLONG &secondmaxLengthNodeid, float &minlength, V3DLONG &minLengthNodeid, float &totalLength)
{

	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid, curnodeidx;
	int i,j;
	V3DLONG subtreeNodeNum = 0;
	float *length = new float [treeNodeNum];

	totalLength = 0;
	maxlength = INVALID_VALUE; //INVALID VALUE is -9999
	secondmaxlength = INVALID_VALUE;
	minlength = -INVALID_VALUE;

	maxLengthNodeid = INVALID_VALUE;
	minLengthNodeid = INVALID_VALUE;
	
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
		nodeQueue[i]= INVALID_VALUE;
	
	nodeQueue[0] = nodeid;
	headptr = 0;
	tailptr = 0;

	length[headptr] = 0;

	
	//get the first element from the queue
	curnodeid = nodeQueue[headptr];	
	
	while ((curnodeid!=INVALID_VALUE)&&(subtreeNodeNum<treeNodeNum)) //there is still element in the queue
	{	
		subtreeNodeNum++;
		
		swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
		
		// add children of the current node to the tail of the queue
		V3DLONG *childrenID = 0;
		V3DLONG *childrenNodeIdx = 0;
		V3DLONG childrenNum;

		swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);
		
		
		j = 0;
		while (j<childrenNum) 
		{	

			tailptr++;
		
			// compute length
			
			float lengthtmp;
			swcTree::computeLength(curnodeid, childrenID[j], lengthtmp);
			length[tailptr] = length[headptr] + lengthtmp;
			
			
			totalLength += lengthtmp; // note totalLength only sum up the length of branch between parenet and children, not between root and any node
			
			if (length[tailptr]>maxlength)
			{
				secondmaxlength = maxlength;
				secondmaxLengthNodeid = maxLengthNodeid;
				
				maxlength = length[tailptr];
				maxLengthNodeid = childrenID[j];
			}
			else
			{
				if (length[tailptr]>secondmaxlength)
				{
					secondmaxlength = length[tailptr];
					secondmaxLengthNodeid = maxLengthNodeid;
				}
				else
				{
					if (length[tailptr]<minlength)
					{
						minlength = length[tailptr];
						minLengthNodeid = childrenID[j];
					}					
				}	
			}

			nodeQueue[tailptr]=childrenID[j];
			j++;
		} // while (j<childrenNum) 

//		if ((maxlength == 0)&&(minlength == 99999)) // the subtree only contain one node, which means this node is a leaf node 
//			minlength = 0;
		
		//get a node from the head of the queue
		headptr++;
		curnodeid = nodeQueue[headptr];

		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
	}
		
	// delete pointer
//	if (tmpTreeNodeIdx) {delete []tmpTreeNodeIdx; tmpTreeNodeIdx=0;}
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
	if (length) {delete []length; length=0;}

}


//sort nodes in a tree according to their sub tree node number 
void swcTree::sortSubTreeNodeNum(V3DLONG *subTreeNodeNum, float *&sortval, float *&sortidx)
{
	int i;
	
	sortval = new float [treeNodeNum+1]; //sort2 does not sort the first element, add an element so that everyone is sorted 
	sortidx = new float [treeNodeNum+1];
	
	if(!sortval || !sortidx) 
	{
		printf("Fail to allocate memory for sortval and sortidx.\n");
		return;
	}

	sortval[0] = INVALID_VALUE;	sortidx[0] =INVALID_VALUE;
		
	for (i=1; i<=treeNodeNum; i++)
	{
		sortval[i] = (float)subTreeNodeNum[i-1];
		sortidx[i] = (float)(i-1);
	}

#ifdef DEBUG_TREE

	printf("print sortval, sortidx before sort, in sortSubTreeNodeNum(V3DLONG *subTreeNodeNum, float *&sortval, float *&sortidx)\n");
	
//	for (i=0; i<=treeNodeNum; i++)
//		printf("%f ", sortval[i]);
//	printf("\n");
		
	for (i=0; i<=treeNodeNum; i++)
		printf("%f ", sortidx[i]);
	printf("\n");

#endif 

		
	sort2(treeNodeNum, sortval, sortidx); // sort2 does not sort the first element, note that it should be treeNodeNum, not treeNodeNum+1

#ifdef DEBUG_TREE

//	printf("print sortval, sortidx after sort, in sortSubTreeNodeNum(V3DLONG *subTreeNodeNum, float *&sortval, float *&sortidx)\n");
//	for (i=0; i<=treeNodeNum; i++)
//		printf("%f ", sortval[i]);
//	printf("\n");
		
	for (i=0; i<=treeNodeNum; i++)
		printf("%f ", sortidx[i]);
	printf("\n\n");

#endif

	return;
} 

// get the number of nodes of each children branch  
// directChildrenID: node ID of direct children rooted at nodeid
// directChildrenIdx: node idx (in the tree) of  direct children rooted at nodeid
// childrenBranchNodeNum: number of nodes in each children branch

//void swcTree::getChildrenBranchNodeNum(V3DLONG nodeid, V3DLONG *&directChildrenID, V3DLONG *&directChildrenIdx, V3DLONG &directChildrenNum, V3DLONG *&childrenBranchNodeNum)
//{
//
//	V3DLONG *childrenID=0, *childrenNodeIdx=0, childrenNum;		
//	swcTree::getDirectChildren(nodeid, directChildrenID, directChildrenIdx, directChildrenNum);
//	
//	childrenBranchNodeNum = new V3DLONG [directChildrenNum];
//	
//	for (int j=0; j<directChildrenNum; j++)
//		swcTree::getSubTreeNodeNum(directChildrenID[j], childrenBranchNodeNum[j]);
//
//}


// get the number of nodes of each children branch and the parent branch which contains all nodes not belong to children
// branchRootID: root node of each branch, branchRootID[0] is the id of parent node
// branchRootIdx: index of root node of each branch, branchRootIdx[0] is the index of parent node
// branchNodeNum: number of nodes (or size) of each branch, branchNodeNum[0] is the number of nodes in parent branch, which contains not only all ancestors but all nodes not belong to children
void swcTree::getBranchNodeNum(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, V3DLONG *&branchNodeNum) 
{
	
	int j;
	
	V3DLONG *directChildrenID=0, *directChildrenIdx=0, directChildrenNum;		
	swcTree::getDirectChildren(nodeid, directChildrenID, directChildrenIdx, directChildrenNum);

	branchNum = directChildrenNum + 1; // add parent branch, which contains all the nodes not belonging to children branches

	branchNodeNum = new V3DLONG [branchNum];
	branchRootID = new V3DLONG [branchNum];
	branchRootIdx = new V3DLONG [branchNum];
	
	
	for (j=0; j<directChildrenNum; j++)
//		swcTree::getSubTreeNodeNum(directChildrenID[j+1], branchNodeNum[j+1]);
		swcTree::getSubTreeNodeNum(directChildrenID[j], branchNodeNum[j+1]);


	//get the size of the parent branch
	V3DLONG sumval = 0;
	for (j=0; j<directChildrenNum; j++)
		sumval += branchNodeNum[j+1];

	for (j=0; j<directChildrenNum; j++)
	{
		branchRootID[j+1] =directChildrenID[j];
		branchRootIdx[j+1] = directChildrenIdx[j];
	}
	
	//assign values to the first element of branchNodeNum, branchRootID, branchRootIdx
	
	
	V3DLONG nodeidx;
	swcTree::getIndex(nodeid, nodeidx);
	branchRootID[0] = node[nodeidx].parentid;
	
	if (branchRootID[0]==-1)
	{
		branchRootIdx[0] = -1;
		branchNodeNum[0] = 0;
		
	}
	else
	{
		swcTree::getIndex(node[nodeidx].parentid, nodeidx);
		branchRootIdx[0] = nodeidx;
		branchNodeNum[0] = treeNodeNum-sumval;
		
	}
	
	if (directChildrenID) {delete []directChildrenID; directChildrenID=0;}
	if (directChildrenIdx) {delete []directChildrenIdx; directChildrenIdx=0;}
		
}

// get the total length of each branch split from nodeid, including parent branch
void swcTree::getBranchTotalLength(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, float *&totalLength)
{
}

//get the direct children for each node in the tree
void swcTree::getDirectChildren(V3DLONG **&childrenList, V3DLONG *&childrenNum, unsigned char maxChildrenNum )
{

	V3DLONG nodeidx; 
	V3DLONG *childrenList1d = new V3DLONG [treeNodeNum*maxChildrenNum];
	int i,j;
	V3DLONG *ptr = new V3DLONG [treeNodeNum]; // point to the current position of adding a child into the current node of childrenList
	
	if (!childrenList1d)
	{ 
		printf("fail to allocate memory for childrenList1d in swcTree::getDirectChilren\n");
		return;
	}
	
	if (!new2dpointer(childrenList, maxChildrenNum, treeNodeNum, childrenList1d))
	{
		printf("fail to allocate memory for childrenList in swcTree::getDirectChilren\n");
		return;		   		   
	}
		
	childrenNum = new V3DLONG [treeNodeNum];
	
	// initialize
	for (i=0; i<treeNodeNum; i++)
	{	
		childrenNum[i] = 0;
		
		for (j=0; j<maxChildrenNum; j++)
				childrenList[i][j] = INVALID_VALUE;
		
		ptr[i] = 0;
	}
	
	for (i=0; i<treeNodeNum; i++)
	{
		if (node[i].parentid!=-1) // not root
		{
			V3DLONG tmpNode = node[i].parentid;
			V3DLONG tmpNodeIdx;
			
			getIndex(tmpNode, tmpNodeIdx); //need this since node id might not be continous as we allow generating new trees by deleting some nodes
			
			childrenList[tmpNodeIdx][ptr[tmpNodeIdx]] = node[i].nid; 
			ptr[tmpNodeIdx]++;
			
			childrenNum[tmpNodeIdx] = ptr[tmpNodeIdx];
		}
	}
	
#ifdef DEBUG_TREE
	
	printf("print childrenList in getDirectChildren(V3DLONG **&childrenList, V3DLONG *&childrenNum, unsigned char maxChildrenNum)\n");
	
	for (i=0; i<treeNodeNum; i++)
	{
		for (j=0; j<maxChildrenNum; j++)
			printf("%d ", childrenList[i][j]);
		printf("\n");
	}

	printf("print childrenNum in getDirectChildren(V3DLONG **&childrenList, V3DLONG *&childrenNum, unsigned char maxChildrenNum)\n");
	
	for (i=0; i<treeNodeNum; i++)
	{
		printf("%d ", childrenNum[i]);
	}
	printf("\n");
	
	
#endif	
	
	return;

}
 
// get the direct children of nodeid
void swcTree::getDirectChildren(V3DLONG nodeid, V3DLONG *&childrenID, V3DLONG *&childrenNodeIdx, V3DLONG &childrenNum)
{
	int i;
	V3DLONG nodeidx;

	childrenNodeIdx = 0;	
	childrenID = 0;
	childrenNum = 0;
	
	V3DLONG *tmp = new V3DLONG [treeNodeNum];
	
	 	
	for (i=0; i<treeNodeNum; i++)
	{
		if (node[i].parentid==nodeid) 
		{
			childrenNum++; 
//			childrenNodeIdx[childrenNum-1] = i;
			tmp[childrenNum-1] = i;
		}
	}
	
	if (childrenNum>0) //not leaf node
	{
		childrenID = new V3DLONG [childrenNum];
		childrenNodeIdx = new V3DLONG [childrenNum];
		
		for (i=0; i<childrenNum; i++)
		{
			childrenNodeIdx[i] = tmp[i];		
			childrenID[i] = node[childrenNodeIdx[i]].nid;
		}
	}
	else
	{
		childrenID = new V3DLONG [1];
		childrenNodeIdx = new V3DLONG [1];
		childrenID[0] = INVALID_VALUE;
		childrenNodeIdx[0] = INVALID_VALUE;

	}
	
	//delete pointer
	if (tmp) {delete []tmp; tmp=0;}

	
}

// get all the children of nodeid
// remove nodeid from the children list on 20080121, otherwise, if may cause multiple to one mapping in DP
void swcTree::getAllChildren(V3DLONG nodeid, V3DLONG *&childrenID, V3DLONG *&childrenNodeIdx, V3DLONG &childrenNum)
{
	V3DLONG *tmpNodeIdx = 0;

	swcTree *subtree = 0; 
	swcTree::getSubTree(nodeid, subtree, tmpNodeIdx); // include root of the subtree, i.e., nodeid
	
	
//	childrenNum = subtree->treeNodeNum;
	childrenNum = subtree->treeNodeNum-1;
	
	childrenID = new V3DLONG [childrenNum];
	childrenNodeIdx = new V3DLONG [childrenNum];
	
	for (int i=0; i<childrenNum; i++)
	{
		childrenNodeIdx[i] = tmpNodeIdx[i+1]; // exlcude the root of the subtree, i.e., nodeid
		childrenID[i] = node[childrenNodeIdx[i]].nid;
	}	
	if (subtree) {delete subtree; subtree=0;}
	if (tmpNodeIdx) {delete tmpNodeIdx; tmpNodeIdx = 0;}
	
}

// get all ancestors of nodeid
void swcTree::getAllAncestors(V3DLONG nodeid, V3DLONG *&ancestorID, V3DLONG *&ancestorIdx, V3DLONG &ancestorNum)
{
	V3DLONG i;
	V3DLONG nodeidx;
	V3DLONG *tmpID  = new V3DLONG [treeNodeNum];
	V3DLONG *tmpIdx  = new V3DLONG [treeNodeNum];
	
	swcTree::getIndex(nodeid, nodeidx);
	
	nodeid = node[nodeidx].parentid;
	
	if (nodeid==-1) //input is root 
	{
		ancestorID = 0;
		ancestorIdx = 0;
		ancestorNum = 0;
		return;
	}
	
	ancestorNum =0;
	
	while (nodeid!=-1)
	{
		ancestorNum++;
	    tmpID[ancestorNum] = nodeid;
		tmpIdx[ancestorNum] = nodeidx;
		
		swcTree::getIndex(nodeid, nodeidx);
		nodeid = node[nodeidx].parentid;
	}
	
	//assign values to ancestorID, ancestorID
	ancestorID = new V3DLONG [ancestorNum];
	ancestorIdx = new V3DLONG [ancestorNum];
	
	for (i=0; i<ancestorNum; i++)
	{
		ancestorID[i] = tmpID[i];
		ancestorIdx[i] = tmpIdx[i];
	}
	
	if (tmpID) {delete []tmpID; tmpID=0;}
	if (tmpIdx) {delete []tmpIdx; tmpIdx=0;}

} 

// get all the ancestors of nodeid
void swcTree::getAncestors(V3DLONG nodeid, V3DLONG *&ancestorsID, V3DLONG *&ancestorsIdx, V3DLONG &ancestorsNum)
{
	V3DLONG *ancestorsID_tmp = new V3DLONG [1000];
	V3DLONG *ancestorsIdx_tmp = new V3DLONG [1000];
	
	V3DLONG nodeidx;
	ancestorsNum = -1;
	
	swcTree::getIndex(nodeid, nodeidx);
	
	// search for all ancestors
	do
	{
		ancestorsNum++;		
	
		nodeid = node[nodeidx].parentid;
		
		if (nodeid==-1)
			break;
			
		swcTree::getIndex(nodeid, nodeidx);
			
		ancestorsID_tmp[ancestorsNum] = nodeid;
		ancestorsIdx_tmp[ancestorsNum] = nodeidx;
		
		
	}while (nodeid!=-1);
	
	if (ancestorsNum == 0)
	{
		ancestorsID = 0;
		ancestorsIdx = 0;
	}
	else
	{
		//assign values to ancestorsID and ancestorsIdx
		ancestorsID = new V3DLONG [ancestorsNum];
		ancestorsIdx = new V3DLONG [ancestorsNum];
		
		for (int i=0; i<ancestorsNum; i++)
		{
			ancestorsID[i] = ancestorsID_tmp[i];
			ancestorsIdx[i] = ancestorsIdx_tmp[i];
		}
	}
	
	// delete pointers
	if (ancestorsID_tmp) {delete ancestorsID_tmp; ancestorsID_tmp=0;}
	if (ancestorsIdx_tmp) {delete ancestorsIdx_tmp; ancestorsIdx_tmp=0;}
	
}



//get the structural type of each node in the tree, -1: root; 0: leaf; 1: continuation; 2: bifurcation; 3: trifurcation ...
void swcTree::getNodeStructuralType(unsigned char *&stype, unsigned char &maxfurcation)
{
}

//output the tree as the dot file to visualize in Graphviz
void swcTree::genGraphvizFile(char *outfilename) 
{
	V3DLONG i;
	
	FILE *file;
	file = fopen(outfilename, "wt");
	
	fprintf(file,"digraph untitled\n");
	fprintf(file,"{\n");
	
	for (i=0; i<treeNodeNum; i++)
	{
		if (node[i].parentid!=-1)
		{
			fprintf(file,"%d->%d;\n", node[i].parentid,node[i].nid);
		}
	}
	fprintf(file, "}\n");
	
	fclose(file);
}

// get all leaf nodes in the tree
void swcTree::getLeafNode(V3DLONG *&leafNodeList, V3DLONG *&leafNodeIdx, V3DLONG &leafNodeNum)
{
   V3DLONG **childrenList = 0;
   V3DLONG *childrenNum = 0;
   V3DLONG *tmpidx = new V3DLONG [treeNodeNum];
   
   leafNodeNum = 0;
   
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); 
	for (int i=0; i<treeNodeNum; i++)
	{
		if (childrenNum[i]==0)
		{
			leafNodeNum++;
			tmpidx[leafNodeNum-1] = i;
		}
	}
	
	leafNodeList = new V3DLONG [leafNodeNum];
	leafNodeIdx = new V3DLONG [leafNodeNum];
	
	for (int i=0; i<leafNodeNum; i++)
	{
		leafNodeList[i] = node[tmpidx[i]].nid;
		leafNodeIdx[i] = tmpidx[i];
	}
	
	if (tmpidx) {delete [] tmpidx; tmpidx=0;}
	if (childrenNum) {delete [] childrenNum; childrenNum=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;

}

// get significant leaf nodes in the tree, ignore those in very small branches
// small branch is determined by lengthThre
void swcTree::getLeafNode(float lengthThre, V3DLONG *&leafNodeList, V3DLONG *&leafNodeIdx, V3DLONG &leafNodeNum)
{
   V3DLONG **childrenList = 0;
   V3DLONG *childrenNum = 0;
   V3DLONG *tmpidx = new V3DLONG [treeNodeNum];
   V3DLONG nodeid, nodeidx;
   
   leafNodeNum = 0;
   
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); 
	for (int i=0; i<treeNodeNum; i++)
	{
		if (childrenNum[i]==0)
		{
			// find its closet parent branching node
			nodeid = node[i].nid;
			float length;
			
			while(1)
			{
				swcTree::getIndex(nodeid, nodeidx);		
//				printf("nodeid = %d, nodeidx = %d\n", nodeid, nodeidx);
						
				if (node[nodeidx].parentid==-1) //come to root
					break;
					
				nodeid = node[nodeidx].parentid;
				
				V3DLONG *cnodeid, *cnodeidx, cnum;
				
				swcTree::getDirectChildren(nodeid, cnodeid, cnodeidx, cnum); 
				
				if (cnodeid) {delete []cnodeid; cnodeid=0;}
				if (cnodeidx) {delete []cnodeidx; cnodeidx=0;}
				
				if (cnum>1) //hit a branching point
					break;
			} // while
			
			//compute the length from the current leaf node to its closet branching node
			swcTree::computeLength(nodeid, node[i].nid,length);
			if (length>lengthThre)
			{
				leafNodeNum++;
				tmpidx[leafNodeNum-1] = i;
			}
		}
	}
	
	leafNodeList = new V3DLONG [leafNodeNum];
	leafNodeIdx = new V3DLONG [leafNodeNum];
	
	for (int i=0; i<leafNodeNum; i++)
	{
		leafNodeList[i] = node[tmpidx[i]].nid;
		leafNodeIdx[i] = tmpidx[i];
	}
	
	if (tmpidx) {delete [] tmpidx; tmpidx=0;}
	if (childrenNum) {delete [] childrenNum; childrenNum=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;

}

void swcTree::getBifurcation(V3DLONG *&bifurNodeList, V3DLONG *&bifurfNodeIdx, V3DLONG &bifurNodeNum)
{
   V3DLONG **childrenList = 0;
   V3DLONG *childrenNum = 0;
   V3DLONG *tmpidx = new V3DLONG [treeNodeNum];
   
   bifurNodeNum = 0;
   
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); 
	for (int i=0; i<treeNodeNum; i++)
	{
		if (childrenNum[i]==2)
		{
			bifurNodeNum++;
			tmpidx[bifurNodeNum-1] = i;
		}
	}
	
	bifurNodeList = new V3DLONG [bifurNodeNum];
	bifurfNodeIdx = new V3DLONG [bifurNodeNum];
	
	for (int i=0; i<bifurNodeNum; i++)
	{
		bifurNodeList[i] = node[tmpidx[i]].nid;
		bifurfNodeIdx[i] = tmpidx[i];
	}
	
	if (tmpidx) {delete [] tmpidx; tmpidx=0;}
	if (childrenNum) {delete [] childrenNum; childrenNum=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	
}


// get bifurcation and multi-fication nodes from the tree
void swcTree::getBranchingNode(V3DLONG *&branchingNodeList, V3DLONG *&branchingNodeIdx, V3DLONG &branchingNodeNum)
{
   V3DLONG **childrenList = 0;
   V3DLONG *childrenNum = 0;
   V3DLONG *tmpidx = new V3DLONG [treeNodeNum];
   
   branchingNodeNum = 0;
   
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); 
	for (int i=0; i<treeNodeNum; i++)
	{
		if (childrenNum[i]>=2)
		{
			tmpidx[branchingNodeNum] = i;
			branchingNodeNum++;
		}
	}
	
	branchingNodeList = new V3DLONG [branchingNodeNum];
	branchingNodeIdx = new V3DLONG [branchingNodeNum];
	
	for (int i=0; i<branchingNodeNum; i++)
	{
		branchingNodeList[i] = node[tmpidx[i]].nid;
		branchingNodeIdx[i] = tmpidx[i];
	}
	
	if (tmpidx) {delete [] tmpidx; tmpidx=0;}
	if (childrenNum) {delete [] childrenNum; childrenNum=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	
}

//get significant branching points, each branch needs to be bigger than branchSizeThre
void swcTree::getSignificantBranchingNode(V3DLONG branchSizeThre, V3DLONG *&sigBranchingNodeList, V3DLONG *&sigBranchingNodeIdx, V3DLONG &sigBranchingNodeNum)
{
	V3DLONG *branchingNodeList,*branchingNodeIdx=0, branchingNodeNum; 
	V3DLONG i,j;
	
	// get all branching nodes
	swcTree::getBranchingNode(branchingNodeList, branchingNodeIdx, branchingNodeNum); 

	V3DLONG *tmpList= new V3DLONG [branchingNodeNum];
	V3DLONG *tmpIdx = new V3DLONG [branchingNodeNum];
	V3DLONG cnt = -1;

	V3DLONG rootid, rootidx;
	swcTree::getRootID(rootid, rootidx);
	
	// for each braning node, compute size of each children branches
	for (i=0; i<branchingNodeNum; i++)
	{

		// get size of each children branch of a node
//		V3DLONG *directChildrenID=0, *directChildrenIdx=0, directChildrenNum, *childrenBranchNodeNum=0;				
//		swcTree::getBranchNodeNum(branchingNodeList[i], directChildrenID, directChildrenIdx, directChildrenNum, childrenBranchNodeNum);

		V3DLONG *branchRootID =0, *branchRootIdx=0, branchNum, *branchNodeNum =0;
		swcTree::getBranchNodeNum(branchingNodeList[i], branchRootID, branchRootIdx, branchNum, branchNodeNum);


		V3DLONG bigBranchNum=0;
		
		printf("%d: ", branchingNodeIdx[i]);
		
		for (j=1; j<branchNum; j++) // the first one is the size of parent branch
		{ 
			printf("%d, ", branchNodeNum[j]);
			if (branchNodeNum[j]>branchSizeThre)
				bigBranchNum++;
		}
		printf("\n");
		
//		printf("bigBranchNum = %d\n", bigBranchNum);
		
//		if ((bigBranchNum>1)||((bigBranchNum>1)&&(branchingNodeList[i]==rootid))) // it is a significant branch since there are at least 3 branch (including parent branch) that have big number of nodes, root can have only 2
		if (bigBranchNum>1) // it is a significant branch since there are at least 2 branch (including parent branch)

		{	
			cnt++;
//			printf("%d\n", cnt); 
			
			tmpList[cnt] = branchingNodeList[i];
			tmpIdx[cnt] = branchingNodeIdx[i];
		}
		
//		if (directChildrenID) {delete []directChildrenID; directChildrenID=0;}
//		if (directChildrenIdx) {delete []directChildrenIdx; directChildrenIdx=0;}	
//		if (childrenBranchNodeNum) {delete []childrenBranchNodeNum; childrenBranchNodeNum=0;}	

		if (branchRootID) {delete []branchRootID; branchRootID=0;}
		if (branchRootIdx) {delete []branchRootIdx; branchRootIdx=0;}	
		if (branchNodeNum) {delete []branchNodeNum; branchNodeNum=0;}	
			
	}
	
	// allocate memory and assign values to sigBranchingNodeList, sigBranchingNodeIdx, sigBranchingNodeNum
	sigBranchingNodeNum = cnt;
	sigBranchingNodeList = new V3DLONG [sigBranchingNodeNum];
	sigBranchingNodeIdx = new V3DLONG [sigBranchingNodeNum];
	
	for (i=0; i<cnt; i++)
	{
		sigBranchingNodeList[i] = tmpList[i];
		sigBranchingNodeIdx[i] = tmpIdx[i];
	}
	
	if (tmpList) {delete []tmpList; tmpList=0;}
	if (tmpIdx) {delete []tmpIdx; tmpIdx=0;}	
	if (branchingNodeList) {delete []branchingNodeList; branchingNodeList=0;}
	if (branchingNodeIdx) {delete []branchingNodeIdx; branchingNodeIdx=0;}

}

// get differnt types of node from the tree
// nodeType = 0 for 'leaf', 2 for 'bifurcation', 3 for 'trifurcation', 1 for 'branching', 4~10 for 'multifurcation'

void swcTree::getTypedNodes(V3DLONG *&typedNodeList, V3DLONG *&typedNodeIdx, V3DLONG &typedNodeNum, unsigned char nodeType)
{
   V3DLONG **childrenList = 0;
   V3DLONG *childrenNum = 0;
   V3DLONG *tmpidx = new V3DLONG [treeNodeNum];
   
   typedNodeNum = 0;
   
	swcTree::getDirectChildren(childrenList, childrenNum, MAX_CHILDREN_NUM); 
	
	switch (nodeType)
	{
		case 0: //'leaf'
			for (int i=0; i<treeNodeNum; i++)
			{
				if (childrenNum[i]==1)
				{
					typedNodeNum++;
					tmpidx[typedNodeNum-1] = i;
				}
			}
			break;
			
		case 2: //'bifurcation'
			for (int i=0; i<treeNodeNum; i++)
			{
				if (childrenNum[i]==2)
				{
					typedNodeNum++;
					tmpidx[typedNodeNum-1] = i;
				}
			}
			break;
		
		case 3: //'trifurcation':
			for (int i=0; i<treeNodeNum; i++)
			{
				if (childrenNum[i]==3)
				{
					typedNodeNum++;
					tmpidx[typedNodeNum-1] = i;
				}
			}
			break;

		case 1: //'branching':
			for (int i=0; i<treeNodeNum; i++)
			{
				if (childrenNum[i]>=2)
				{
					typedNodeNum++;
					tmpidx[typedNodeNum-1] = i;
				}
			}
			break;	

		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10: //'multifurcation':
			for (int i=0; i<treeNodeNum; i++)
			{
				if (childrenNum[i]>3)
				{
					typedNodeNum++;
					tmpidx[typedNodeNum-1] = i;
				}
			}
			break;


	};
	
	
	typedNodeList = new V3DLONG [typedNodeNum];
	typedNodeIdx = new V3DLONG [typedNodeNum];
	
	for (int i=0; i<typedNodeNum; i++)
	{
		typedNodeList[i] = node[tmpidx[i]].nid;
		typedNodeIdx[i] = tmpidx[i];
	}
	
	if (tmpidx) {delete [] tmpidx; tmpidx=0;}
	if (childrenNum) {delete [] childrenNum; childrenNum=0;}
	V3DLONG *childrenList1d = childrenList[0];
	if (childrenList) {delete2dpointer(childrenList, MAX_CHILDREN_NUM, treeNodeNum);}
	delete []childrenList1d; childrenList1d=0;
	
}


//copy tree
void swcTree::copyTree(swcTree *&newTree)
{
	newTree = new swcTree(treeNodeNum);
	
	newTree->treeNodeNum = treeNodeNum;
	
	for (int i=0; i<treeNodeNum; i++)
	{
		newTree->node[i].nid = node[i].nid;
		newTree->node[i].ntype = node[i].ntype;
		newTree->node[i].x = node[i].x;
		newTree->node[i].y = node[i].y;
		newTree->node[i].z = node[i].z;
		newTree->node[i].radius = node[i].radius;
		newTree->node[i].parentid = node[i].parentid;	
	}
} 



// delete a node in the tree to geneate a new tree, 
// note that when nodeid is a branching point, what delected is infact a randomly selected branc
// when nodeid is a leaf node, directly remove it
// when nodeid is a continual node, make its child to be the child of its parent

//void swcTree::deleteNode(V3DLONG nodeid, swcTree *&newTree)
//{
//	
//}
//
////delete a branch of nodes rooted at nodeid
//void swcTree::deleteBranch(V3DLONG nodeid, swcTree *&newTree)
//{
//
//}



// randomly delete some nodes in the tree according to some probability
// rate: rate of nodes that should be deleted
// branchSizeThre: if the deleted node is a branching point, only those with brach size less than branchThre are deleted
// this is to make sure that not significant changes are made to the original tree

void swcTree::randomDeleteNodes(V3DLONG branchSizeThre, float rate, swcTree *&newTree, V3DLONG *&deletedTag)
{
	V3DLONG num = (int) (treeNodeNum*rate); // number of nodes should be deleted
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	V3DLONG *checkedTag = new V3DLONG [treeNodeNum]; //do not check the same node twice, a node been checked means either it has been deleted or it does not satisfy the condition to be deleted
//	swcTree *tmpTree = 0;
	
	newTree = 0;
	deletedTag = new V3DLONG [treeNodeNum]; //record which node has been deleted
	
	printf("num = %d\n", num);

	if ((rate>1) || (rate<0))
	{
		printf("Rate should be falling into the range [0,1]. \n");
		return; 
	}
		
	if (num==0)
	{
		printf("No node is deleted. No new tree is generated. Check rate. \n");
		return;
	}
	
	if (num>treeNodeNum)
	{
		printf("All nodes are deleted. No new tree is generated. Check rate. \n");
		return; 
	}
	
//	//initialize tmpTree
//	swcTree::copyTree(tmpTree);
	
	//initialize deleteTag, meaning no node has been deleted yet
	
	for (i=0; i<treeNodeNum; i++)
	{
		deletedTag[i] = 0; 
		checkedTag[i] = 0;
	}
	
	
	// delete operation
	i = 0;
	while (i<num)
	{
		unsigned char exhausted= 1;
		V3DLONG *childrenID, *childrenNodeIdx, childrenNum;
		
		for (j=0; j<treeNodeNum; j++)
			if (checkedTag[j]==0)
			{
				exhausted = 0;
				break;
			}
		
		if (exhausted == 1) // no nodes can be further deleted, 
			break;
			
		// 
		// generate random number
//		unsigned char findtag = 0;
		srand(time(NULL));
		
//		while (findtag == 0)
		while (1)

		{   
			V3DLONG mytmp = rand();
			nodeidx = rand() % treeNodeNum; //generate a random number between 0 and treeNodeNum-1
			
			// note root can be deleted as well, in this case, some other node will become root			
			if (deletedTag[nodeidx] == 0) 
			{
				printf("deleted node = %d,  nodeidx = %d\n", mytmp, nodeidx);			
				break;
			}
		};
			
		swcTree::getDirectChildren(node[nodeidx].nid, childrenID, childrenNodeIdx, childrenNum);
		
		if (childrenNum==0) // a leaf node, directly delete
		{			
//			deleteNodesIdx[i] = nodeidx;
			deletedTag[nodeidx] = 1;
			checkedTag[nodeidx] = 1;
			i++;		
		}
		
		if (childrenNum==1) // a continual node
		{
//			deleteNodesIdx[i] = nodeidx;
			deletedTag[nodeidx] = 1;
			checkedTag[nodeidx] = 1;
			i++;
//			tmpTree->node[childrenNodeIdx[0]].parentid = tmpTree->node[nodeidx].parentid; //set child's parent to its own parent
			
		} 
		
		if (childrenNum>1) 	 // a branching point, including root
		{
			// test the branch size
			V3DLONG subTreeNodeNum;
			
			swcTree::getSubTreeNodeNum(node[nodeidx].nid, subTreeNodeNum);
			
			if (subTreeNodeNum>branchSizeThre)		
				checkedTag[nodeidx] = 1; // do not satisfy deleting condition, mark it so that not to check it next time
			else
			{
				// determin the branch with the maximum size
				
				float maxnum = -9999;
				unsigned char maxidx;
				
				for (j=0; j<childrenNum; j++)
				{
					V3DLONG subTreeNodeNum2;

					swcTree::getSubTreeNodeNum(childrenID[j], subTreeNodeNum2);			
								
					if (subTreeNodeNum>maxnum)
					{
						maxnum = subTreeNodeNum;
						maxidx = j;
					}
				}
				
				// prune other smaller branches
				for (j=0; j<childrenNum; j++)
				{
					swcTree *subtree = 0;
					V3DLONG *subtreeNodeIdx = 0;
					
					if (j!=maxidx)
					{
						swcTree::getSubTree(childrenID[j], subtree, subtreeNodeIdx); // get the subtree rooted at each children of the note to be deleted
						for (k=0; k<subtree->treeNodeNum; k++)
						{
							deletedTag[subtreeNodeIdx[k]] = 1;
							checkedTag[subtreeNodeIdx[k]] = 1;
						}
						
						i += subtree->treeNodeNum;
					}
					
					if (subtree) {delete subtree; subtree=0;}
					if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx=0;}
					
				} //for (j=0; j<childrenNum; j++)
			} //if
			
		} // if (childrenNum>1) 
		
		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
	} // while (i<num)
	
	//assign newTree
	V3DLONG idx = -1;
	
	newTree = new swcTree(treeNodeNum-i);
	newTree->treeNodeNum = treeNodeNum-i;
	
//	for (j=0; j<i; j++)
//	{
//		if (deletedTag[j] == 0) //node is kept
//		{
//			idx++;
//			newTree->node[idx].nid = node[j].nid;
//			newTree->node[idx].ntype = node[j].ntype;
//			newTree->node[idx].x = node[j].x;
//			newTree->node[idx].y = node[j].y;
//			newTree->node[idx].z = node[j].z;
//			newTree->node[idx].radius = node[j].radius;
//			newTree->node[idx].parentid = node[j].parentid;				
//		}
//	}

	for (j=0; j<treeNodeNum; j++)
	{
//		printf("%d ", deletedTag[j]);
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;
//			newTree->node[idx].nid = tmpTree->node[j].nid;
//			newTree->node[idx].ntype = tmpTree->node[j].ntype;
//			newTree->node[idx].x = tmpTree->node[j].x;
//			newTree->node[idx].y = tmpTree->node[j].y;
//			newTree->node[idx].z = tmpTree->node[j].z;
//			newTree->node[idx].radius = tmpTree->node[j].radius;
//			newTree->node[idx].parentid = tmpTree->node[j].parentid;				

			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
//			newTree->node[idx].parentid = node[j].parentid;
			
			// parent must be a node that exist
			V3DLONG pnode, pnodeidx;
			pnodeidx = j;
			
			do 
			{
				pnode = node[pnodeidx].parentid;
				swcTree::getIndex(pnode, pnodeidx);
//				printf("%d ", pnode);
				
			} while ((deletedTag[pnodeidx] == 1)&&(pnode!=-1)); // pnode has been deleted, find the next parent node

//			printf("\n");
			
			newTree->node[idx].parentid = pnode;
		}
	}
	
//	printf("\n");
	
	// delete pointers
	if (checkedTag) {delete []checkedTag; checkedTag=0;}
	
}


// delete nodes given by nodeid in a tree and generate a new tree
void swcTree::deleteTreeNode(V3DLONG *nodeidlist, V3DLONG deleteNodeNum, swcTree *&newTree)
{
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	
	newTree = 0;
	bool *deletedTag = new bool [treeNodeNum]; //record which node has been deleted
	
	//initialize deleteTag, meaning no node has been deleted yet	
	for (i=0; i<treeNodeNum; i++)
	{
		deletedTag[i] = 0; 
	}

	for (i=0; i<deleteNodeNum; i++)
	{
		swcTree::getIndex(nodeidlist[i], nodeidx);
		deletedTag[nodeidx]=1;
	}
	
	//assign newTree
	V3DLONG idx = -1;
	
	newTree = new swcTree(treeNodeNum-deleteNodeNum);
	newTree->treeNodeNum = treeNodeNum-deleteNodeNum;
	
	for (j=0; j<treeNodeNum; j++)
	{
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;

			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
			
			// parent must be a node that exist
			V3DLONG pnode, pnodeidx;
			pnodeidx = j;
			
			do 
			{
				pnode = node[pnodeidx].parentid;
				swcTree::getIndex(pnode, pnodeidx);
				
			} while ((deletedTag[pnodeidx] == 1)&&(pnode!=-1)); // pnode has been deleted, find the next parent node

			newTree->node[idx].parentid = pnode;
		}
	}
	
	if (deletedTag) {delete []deletedTag; deletedTag=0;}
	
}

// delete subtree rooted at nodeid and return a new tree
void swcTree::deleteSubTreeNode(V3DLONG *nodeIDList, V3DLONG nodeIDNum, V3DLONG &deleteNodeNum, V3DLONG *&deleteTag, swcTree *&newTree)
{
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	
	newTree = 0;
	bool *deletedTag = new bool [treeNodeNum]; //record which node has been deleted
	deleteNodeNum = 0;
	
	//initialize deleteTag, meaning no node has been deleted yet	
	for (i=0; i<treeNodeNum; i++)
	{
		deletedTag[i] = 0; 
	}

	V3DLONG newTreeNodeNum = treeNodeNum; 
	for (j=0; j<nodeIDNum; j++)
	{
		swcTree *subtree = 0;
		V3DLONG *subtreeNodeIdx = 0;
		getSubTree(nodeIDList[j], subtree, subtreeNodeIdx);
		
		for (i=0; i<subtree->treeNodeNum;i++)
		{
			deletedTag[subtreeNodeIdx[i]] = 1;
		}

		deleteNodeNum += subtree->treeNodeNum;
	   
	   if (subtree) {delete subtree; subtree=0;}
	   if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx=0;}
	}
	
	//assign newTree
	V3DLONG idx = -1;
   
    newTreeNodeNum = treeNodeNum -  deleteNodeNum;
	printf("treeNodeNum=%d, newTreeNodeNum = %d\n", treeNodeNum, newTreeNodeNum);
	
	newTree = new swcTree(newTreeNodeNum);
	newTree->treeNodeNum = newTreeNodeNum;
	
	for (j=0; j<treeNodeNum; j++)
	{
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;

			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
			newTree->node[idx].parentid = node[j].parentid;
			
//			// parent must be a node that exist
//			V3DLONG pnode, pnodeidx;
//			pnodeidx = j;
//			
//			do 
//			{
//				pnode = node[pnodeidx].parentid;
//				swcTree::getIndex(pnode, pnodeidx);
//				
//			} while ((deletedTag[pnodeidx] == 1)&&(pnode!=-1)); // pnode has been deleted, find the next parent node
//
//			newTree->node[idx].parentid = pnode;
		}
	}
	
//	if (deletedTag) {delete []deletedTag; deletedTag=0;}
	
}



 // Build super node tree by deleting a list of nodes specified by deleteNodeIDlist
 // making the children of each node the children of its ancester node listed in ancesterIDList
 // Usually the ancester node is not the direct parent 
 // Note that deleteNodeIDList and ancesterIDList have one-to-one corespondence
 // This function is called in tree matching when combining branching nodes into supernodes
 
void swcTree::buildSuperNodeTree(V3DLONG *deleteNodeIDList, V3DLONG *ancesterIDList, V3DLONG deleteNodeNum, swcTree *&newTree)
{
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	V3DLONG pnode, pnodeidx;
//	V3DLONG realDeleteNodeNum = 0; // this is bigger than deleteNodeNum since the nodes in the path from deleteNodeIDList[i] to ancesterIDList[i] are deleted too
	
	newTree = 0;
	bool *deletedTag = new bool [treeNodeNum]; //record which node has been deleted
	
	//initialize deleteTag, meaning no node has been deleted yet	
	for (i=0; i<treeNodeNum; i++)
	{
		deletedTag[i] = 0; 
	}

	for (i=0; i<deleteNodeNum; i++)
	{
		// set the deleteTag of nodes to be deleted to be 1	
		swcTree::getIndex(deleteNodeIDList[i], nodeidx);
		deletedTag[nodeidx]=1;
		
		// set the deleteTag of nodes on the path of the deleteNodeList[i] and ancesterIDList[i] to be 1
		pnodeidx = nodeidx;
		do 
		{
			pnode = node[pnodeidx].parentid;
			swcTree::getIndex(pnode, pnodeidx);			
			deletedTag[pnodeidx] = 1;
			deleteNodeNum++;
			
		} while ((pnode!=ancesterIDList[i])&&(pnode!=-1));
			
		if ((pnode!=ancesterIDList[i])&&(pnode==-1))
		{
			printf("%d is not an ancester of %d, check your input\n", ancesterIDList[i], deleteNodeIDList[i]);
			exit (1);
		}
	}
	

	//assign newTree
	V3DLONG idx = -1;
	
	newTree = new swcTree(treeNodeNum-deleteNodeNum);
	newTree->treeNodeNum = treeNodeNum-deleteNodeNum;
	
	for (j=0; j<treeNodeNum; j++)
	{
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;

			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
			
			// parent must be a node that exist
			V3DLONG pnode, pnodeidx;
			pnodeidx = j;

			do 
			{
				pnode = node[pnodeidx].parentid;
				swcTree::getIndex(pnode, pnodeidx);				
			} while ((deletedTag[pnodeidx] == 1) && (pnode!=-1)); // if pnode has been deleted, find the next parent node that is a branching node
			
//			do 
//			{
//				pnode = node[pnodeidx].parentid;
//				swcTree::getIndex(pnode, pnodeidx);
//				
//				V3DLONG *childrenID=0, *childrenNodeIdx=0;
//				swcTree::getDirectChildren(pnode, childrenID, childrenNodeIdx, childrenNum);
//				if (childrenID) {delete []childrenID; childrenID=0;}
//				if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
//				
//			} while (~((deletedTag[pnodeidx] == 0)&&(childrenNum>1)) && (pnode!=-1)); // if pnode has been deleted, find the next parent node that is a branching node

			newTree->node[idx].parentid = pnode;
		}
	}
	
	if (deletedTag) {delete []deletedTag; deletedTag=0;}
}

// Build super node tree
// Different from the above function, only delete a single node

void swcTree::buildSuperNodeTree(V3DLONG deleteNodeID, V3DLONG ancesterID, swcTree *&newTree)
{
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	V3DLONG pnode, pnodeidx;
	V3DLONG deleteNodeNum = 1;
	
	newTree = 0;
	bool *deletedTag = new bool [treeNodeNum]; //record which node has been deleted
	
	//initialize deleteTag, meaning no node has been deleted yet	
	for (i=0; i<treeNodeNum; i++)
	{
		deletedTag[i] = 0; 
	}

	// set the deleteTag of nodes to be deleted to be 1	
	swcTree::getIndex(deleteNodeID, nodeidx);
	deletedTag[nodeidx]=1;
	
	// set the deleteTag of nodes on the path of the deleteNodeList[i] and ancesterIDList[i] to be 1
	pnodeidx = nodeidx;
	do 
	{
		pnode = node[pnodeidx].parentid;
		swcTree::getIndex(pnode, pnodeidx);			
		deletedTag[pnodeidx] = 1;
		deleteNodeNum++;
		
	} while ((pnode!=ancesterID)&&(pnode!=-1));
		
	if ((pnode!=ancesterID)&&(pnode==-1))
	{
		printf("%d is not an ancester of %d, check your input\n", ancesterID, deleteNodeID);
		exit (1);
	}
	else
	{
		deletedTag[pnodeidx] = 0; //don't delete the node with ancesterID
		deleteNodeNum--;
	}

	//assign newTree
	V3DLONG idx = -1;
	
	newTree = new swcTree(treeNodeNum-deleteNodeNum);
	newTree->treeNodeNum = treeNodeNum-deleteNodeNum;
	
	for (j=0; j<treeNodeNum; j++)
	{
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;

			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
			
			// parent must be a node that exist
			V3DLONG pnode, pnodeidx;
			pnodeidx = j;

			do 
			{
				pnode = node[pnodeidx].parentid;
				if (pnode==-1)
					break;
				swcTree::getIndex(pnode, pnodeidx);				
			} while ((deletedTag[pnodeidx] == 1) && (pnode!=-1)); // if pnode has been deleted, find the next parent node that is a branching node
			
			newTree->node[idx].parentid = pnode;
		}
	}
	
	if (deletedTag) {delete []deletedTag; deletedTag=0;}
}

// randomly delete some branches in the tree according to rate
// rate: rate of nodes that should be deleted, the rate of branching points should be deleted is less than this number
// branchSizeThre: only those with brach size less than branchThre are deleted
// deletedTag indicate which nodes have been deleted
void swcTree::randomDeleteBranch(V3DLONG branchSizeThre, float rate, swcTree *&newTree, V3DLONG *&deletedTag)
{
	
	
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	V3DLONG *branchingNodeList = 0;
	V3DLONG *branchingdNodeIdx = 0;
	V3DLONG branchingdNodeNum;
	V3DLONG *checkedBranchingNodeTag = 0;
	V3DLONG *deletedBranchingNodeTag = 0; // record which branching nodes have been deleted
	V3DLONG nodeDeletedNum =0; // count how many nodes are deleted
	V3DLONG ind;

	V3DLONG *childrenID, *childrenNodeIdx, childrenNum;
	
	newTree = 0;

	// get all branching nodes
	swcTree::getTypedNodes(branchingNodeList, branchingdNodeIdx, branchingdNodeNum, 1);

#ifdef DEBUG_TREE

	for (i=0; i<branchingdNodeNum; i++)
	{
		swcTree::getAllChildren(node[branchingdNodeIdx[i]].nid, childrenID, childrenNodeIdx, childrenNum);	
		printf("branchingdNodeIdx = %d, childrenNum = %d\n", branchingdNodeIdx[i], childrenNum);
		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
		
	}
#endif
		
	V3DLONG deletedNum = (int) (branchingdNodeNum*rate); // number of branching points, rooted at which their subtrees should be deleted
	
	printf("deletedNum = %d\n", deletedNum);

	if ((rate>1) || (rate<0))
	{
		printf("Rate should be falling into the range [0,1]. \n");
		return; 
	}
		
	if (deletedNum==0)
	{
		printf("No node is deleted. No new tree is generated. Check rate. \n");
		return;
	}

	deletedTag = new V3DLONG [treeNodeNum]; //record which node has been deleted
	
	deletedBranchingNodeTag = new V3DLONG [branchingdNodeNum]; // record which branching nodes have been deleted
	checkedBranchingNodeTag = new V3DLONG [branchingdNodeNum]; //do not check the same node twice, a node been checked means either it has been deleted or it does not satisfy the condition to be deleted
	
	//initialize deleteTag, deletedBranchingNodeTag, checkedBranchingNodeTag meaning no node has been deleted or checked yet	
	for (i=0; i<treeNodeNum; i++)
		deletedTag[i] = 0; 
	
	//get branching points
	for (i=0; i<branchingdNodeNum; i++)
	{
		deletedBranchingNodeTag[i] = 0; 
		checkedBranchingNodeTag[i] = 0;
	}
	
	
	if (rate ==1)
	{
		for (i=0; i<branchingdNodeNum; i++)
		{
			swcTree::getAllChildren(node[branchingdNodeIdx[i]].nid, childrenID, childrenNodeIdx, childrenNum);	
			printf("branchingdNodeIdx = %d, childrenNum = %d\n", branchingdNodeIdx[i], childrenNum);
			
			checkedBranchingNodeTag[i] = 1;
				
			if (childrenNum <= branchSizeThre) 
			{	
				printf("branchingdNodeIdx =%d deleted. \n", branchingdNodeIdx[i]);
				
				deletedBranchingNodeTag[i] =  1;
				
				for (j=0; j<childrenNum; j++)
					deletedTag[childrenNodeIdx[j]] = 1;
					
				i++;
//				nodeDeletedNum += childrenNum;	
			}
			
			if (childrenID) {delete []childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		}	
	}
	else
	{
		// initialize random seed, otherwise each time rand() generate the same squence of random numbers
		srand(time(NULL));
		
		// delete operation
		i = 0;
		while (i<deletedNum)
		{
			unsigned char exhausted= 1;
	//		V3DLONG *childrenID, *childrenNodeIdx, childrenNum;
			
			for (j=0; j<deletedNum; j++)
				if (checkedBranchingNodeTag[j]==0)
				{
					exhausted = 0;
					break;
				}
			
			if (exhausted == 1) // no nodes can be further deleted, 
				break;
				
			// generate random number
			while (1)

			{   
	  
				V3DLONG mytmp = rand();
				ind = rand() % branchingdNodeNum; //generate a random number between 0 and treeNodeNum-1
				
	//			printf("mytmp = %d, ind = %d\n", mytmp, ind);
				
				// note root can be deleted as well, in this case, some other node will become root			
				if (deletedBranchingNodeTag[ind] == 0) 
				{
					nodeidx = branchingdNodeIdx[ind];

	//				printf("selected candidate branching node to be deleted = %d\n", nodeidx);			
					printf("selected candidate branching node to be deleted = %d\n", ind);			

					break;
				}
			};
			
				
	//		swcTree::getDirectChildren(node[nodeidx].nid, childrenID, childrenNodeIdx, childrenNum);

			swcTree::getAllChildren(node[nodeidx].nid, childrenID, childrenNodeIdx, childrenNum);	

	//		printf("childrenNum = %d\n", childrenNum);
			
			checkedBranchingNodeTag[ind] = 1;
				
			if (childrenNum <= branchSizeThre) 
			{	
				deletedBranchingNodeTag[ind] =  1;
				
				for (j=0; j<childrenNum; j++)
					deletedTag[childrenNodeIdx[j]] = 1;
					
				i++;
//				nodeDeletedNum += childrenNum;	
			}
			
			if (childrenID) {delete []childrenID; childrenID=0;}
			if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
			
		} // while (i<deletedNum)
	} //if (rate==1)
	
	for (j=0; j<treeNodeNum; j++)
		if (deletedTag[j]==1)
			nodeDeletedNum++;
			
	printf("Number of nodes deleted: %d\n", nodeDeletedNum);
	
	//assign newTree
	V3DLONG idx = -1;
	

	newTree = new swcTree(treeNodeNum-nodeDeletedNum);
	newTree->treeNodeNum = treeNodeNum-nodeDeletedNum;
	
	for (j=0; j<treeNodeNum; j++)
	{
//		printf("%d ", deletedTag[j]);
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;	

			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
//			newTree->node[idx].parentid = node[j].parentid;
			
			// parent must be a node that exist
			V3DLONG pnode, pnodeidx;
			pnodeidx = j;
			
			do 
			{
				pnode = node[pnodeidx].parentid;
				swcTree::getIndex(pnode, pnodeidx);
//				printf("%d ", pnode);
				
			} while ((deletedTag[pnodeidx] == 1)&&(pnode!=-1)); // pnode has been deleted, find the next parent node

//			printf("\n");
			
			newTree->node[idx].parentid = pnode;
		}
	}
	
//	printf("\n");
	
	// delete pointers
	if (checkedBranchingNodeTag) {delete []checkedBranchingNodeTag; checkedBranchingNodeTag=0;}
	if (deletedBranchingNodeTag) {delete []deletedBranchingNodeTag; deletedBranchingNodeTag=0;}
	if (branchingNodeList) {delete []branchingNodeList; branchingNodeList=0;}
	if (branchingdNodeIdx) {delete []branchingdNodeIdx; branchingdNodeIdx=0;}
	
}

// randomly select some nodes to perturb their 3d coordinates slightly 
// rate: rate of branches that should be perturbed
// maxdis: the maximum displacement along x, y, z are [-maxids, maxdis], x,y,z displacement can be different
// note the real maximum displacement in 3d is: sqrt(maxdis*maxdis + maxdis*maxdis + maxdis*maxdis)

void swcTree::randomPerturbCoordinates(float rate, float maxdis, swcTree *&newTree, V3DLONG *&perturbedTag)
{
	
	V3DLONG num = (int) (treeNodeNum*rate); // number of nodes should be disturbed
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	perturbedTag = new V3DLONG [treeNodeNum]; //record which node has been perturbed

	newTree = 0;

	if ((rate>1) || (rate<0))
	{
		printf("Rate should be falling into the range [0,1]. \n");
		return; 
	}
	
	printf("num = %d\n", num);
	
	if (num==0)
	{
		printf("No node has been perturbed. No new tree is generated. Check rate. \n");
		return;
	}
	
	if (num>treeNodeNum)
	{
		printf("All nodes will be perturbed. \n");
		num = treeNodeNum;
	}
	
	//initialize perturbedTag, meaning no node has been perturbed or checked yet
	
	for (i=0; i<treeNodeNum; i++)
		perturbedTag[i] = 0;
	
	// initialzie newTree
	swcTree::copyTree(newTree); // allocate memory for newTree inside copyTree
		
	// perturb operation
	
	srand(time(NULL));
	
	if (num==treeNodeNum) // no need to generate random number 
	{
		float xdis, ydis, zdis;
	
		for (i=0; i<treeNodeNum; i++)
		{
			
//			xdis = (float)((rand() % (int)(maxdis*100))-maxdis*50)/100;
//			ydis = (float)((rand() % (int)(maxdis*100))-maxdis*50)/100;
//			zdis = (float)((rand() % (int)(maxdis*100))-maxdis*50)/100;

			float a1,a2;
			a1 = (float)(rand()%100)*2*3.14159/100; //[0, 2pi]
			a2 = (float)(rand()%100)*2*3.14159/100; //[0, 2pi]
			
			xdis = maxdis*cos(a1)*sin(a2);
			ydis = maxdis*cos(a1)*cos(a2);
			zdis = maxdis*sin(a1);
			
			printf("xdis=%f, ydis=%f, zdis=%f\n", xdis, ydis, zdis);
			
			// revise the coordniate of the perturbed nodes
			
			newTree->node[i].x += xdis;
			newTree->node[i].y += ydis;
			newTree->node[i].z += zdis;
		}
	
	}
	else
	{
	
		i = 0;
		while (i<num)
		{
			
			// generate a node to perturb using random number generator
			while (1)
			{   
				V3DLONG mytmp = rand();
				nodeidx = rand() % treeNodeNum; //generate a random number between 0 and treeNodeNum-1
				
				if (perturbedTag[nodeidx] == 0) 
				{
//					printf("perturbed node = %d,  nodeidx = %d\n", mytmp, nodeidx);			
					break;
				}
			};
			
			// generate displacement magnitute along x, y, z of the node to be perturbed
			float xdis, ydis, zdis;
			
//			xdis = (float)((rand() % (int)(maxdis*100))-maxdis*50)/100;
//			ydis = (float)((rand() % (int)(maxdis*100))-maxdis*50)/100;
//			zdis = (float)((rand() % (int)(maxdis*100))-maxdis*50)/100;
	
			float a1,a2;
			a1 = (float)(rand()%100)*2*3.14159/100; //[0, 2pi]
			a2 = (float)(rand()%100)*2*3.14159/100; //[0, 2pi]
			
			xdis = maxdis*cos(a1)*sin(a2);
			ydis = maxdis*cos(a1)*cos(a2);
			zdis = maxdis*sin(a1);
					
			printf("perturbed node index = %d, xdis=%f, ydis=%f, zdis=%f\n", nodeidx, xdis, ydis, zdis);
			
			// revise the coordniate of the perturbed nodes
			
			newTree->node[nodeidx].x += xdis;
			newTree->node[nodeidx].y += ydis;
			newTree->node[nodeidx].z += zdis;
			
			perturbedTag[nodeidx] = 1;
			
			i++;
						
		} // while (i<num)
	}
}


// turn tree into an undirected graph, meaning that if there is a path from from a to b 
// (i.e., a is either b's parent or child), then set an undirected edge between a and b 
// graph: the adjacency matrix between any two nodes

void swcTree::tree2UndirectedGraph(unsigned char *&graph) 
{

	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid, curnodeidx;
	V3DLONG int i,j;
	V3DLONG childrenNum;
	
	//initialize graph
	graph = new unsigned char [treeNodeNum*treeNodeNum];
	for (i=0; i<treeNodeNum * treeNodeNum; i++)
		graph[i]= 0;
	
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
		nodeQueue[i]= INVALID_VALUE;

	// get root and put it into the queue
	swcTree::getRootID(curnodeid, curnodeidx);	
	nodeQueue[0] = curnodeid;
	headptr = 0;
	tailptr = 0;
	

//	//get the first element from the queue
//	curnodeid = nodeQueue[headptr];	
	
	while (headptr < treeNodeNum) //there is still element in the queue
	{	
				
		// get direct children of the current node 
		V3DLONG *childrenID = 0;
		V3DLONG *childrenNodeIdx = 0;

		swcTree::getDirectChildren(curnodeid, childrenID, childrenNodeIdx, childrenNum);

		// set value in graph, and add children of the current node to the tail of the queue
		j = 0;
		while (j<childrenNum) 
		{			
			graph[curnodeidx*treeNodeNum + childrenNodeIdx[j]] = 1;
			graph[childrenNodeIdx[j]*treeNodeNum + curnodeidx] = 1;
		
			tailptr++;
			nodeQueue[tailptr]=childrenID[j];
			j++;
		}

		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
		//get a node from the head of the queue
		headptr++;
		curnodeid = nodeQueue[headptr];
		swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
		
	}
	
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}

#ifdef DEBUG	
	for (i=0; i<treeNodeNum; i++)
	{
		for (j=0; j<treeNodeNum; j++)
			printf("%d ", graph[i*treeNodeNum+j]);
		printf("\n");
	}
#endif
}


// reform tree structure using a node (different from that of the original) as the root)
// rootid: the node id of the new root
// the output tree newTree is ordered naturally in depth from root to deepest depth
// in breth-first-search order

void swcTree::reformTree(V3DLONG rootid, swcTree *&newTree)
{

	V3DLONG *nodeQueue = new V3DLONG [treeNodeNum]; //used for breadth-first search of children
	V3DLONG headptr, tailptr;
	V3DLONG curnodeid, curnodeidx;
	int i,j;
	V3DLONG childrenNum;
	unsigned char *graph; //undirected graph
	
	newTree = new swcTree(treeNodeNum);
	
	// turn the tree into an undirected graph first
	swcTree::tree2UndirectedGraph(graph);
	 
	//initialize nodeQueue
	for (i=0; i<treeNodeNum; i++)
		nodeQueue[i]= INVALID_VALUE;
	
	// add element to newTree, and put the new root in the queue
	curnodeid = rootid;
	swcTree::getIndex(curnodeid, curnodeidx);
	
	V3DLONG newtreenodeidx = 0;
	
	newTree->node[newtreenodeidx].nid = node[curnodeidx].nid;
	newTree->node[newtreenodeidx].ntype = node[curnodeidx].ntype;
	newTree->node[newtreenodeidx].x = node[curnodeidx].x;
	newTree->node[newtreenodeidx].y = node[curnodeidx].y;
	newTree->node[newtreenodeidx].z = node[curnodeidx].z;
	newTree->node[newtreenodeidx].radius = node[curnodeidx].radius;
	newTree->node[newtreenodeidx].parentid = -1;
	
	nodeQueue[0] = rootid;
	headptr = 0;
	tailptr = 0;
	
		
	while (headptr<treeNodeNum) //there is still element in the queue
	{	
				
		// get the nodes that are connected to the curnodeid

		for (i=0; i<treeNodeNum; i++)
		{
			if (graph[curnodeidx*treeNodeNum + i] == 1) // there is an edge between the curnodeidx and i
			{
//				printf("%d, %d, %d\n", node[i].nid, headptr, newTree->node[headptr].parentid);
				if (node[i].nid != newTree->node[headptr].parentid) // i is the child of curnodeidx in the new tree
				
				{
					// add the node into newTree
					newtreenodeidx++;
					newTree->node[newtreenodeidx].nid = node[i].nid;
					newTree->node[newtreenodeidx].ntype = node[i].ntype;
					newTree->node[newtreenodeidx].x = node[i].x;
					newTree->node[newtreenodeidx].y = node[i].y;
					newTree->node[newtreenodeidx].z = node[i].z;
					newTree->node[newtreenodeidx].radius = node[i].radius;
					newTree->node[newtreenodeidx].parentid = curnodeid;
					
					// add the node into the queue
					tailptr++;
					nodeQueue[tailptr]=node[i].nid;
					
//					for (j=0; j<tailptr; j++)
//						printf("%d ", nodeQueue[j]);
//					printf("\n");
				}
			}
		}
		
		//get a node from the head of the queue
		headptr++;
		curnodeid = nodeQueue[headptr];
		swcTree::getIndex(curnodeid, curnodeidx); // get the index of the current node
	}
	
	// delete pointer
	if (nodeQueue) {delete []nodeQueue; nodeQueue=0;}
	if (graph) {delete []graph; graph=0;}
}
	

// prune small branches based on the number of nodes in the branch
// branchNodeNum: for any branching point, check the subtree rooted each of its children
// if the number of nodes is smaller than branchNodeNum, then that subbranch is pruned
// this function is useful if we want to do multi-resolution tree matching

void swcTree::pruneBranch(V3DLONG branchNodeNum, swcTree *&newTree, V3DLONG *&deletedTag)
{
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	
	newTree = 0;
	deletedTag = new V3DLONG [treeNodeNum]; //record which node has been deleted
		
	//initialize deleteTag, meaning no node has been deleted yet	
	for (i=0; i<treeNodeNum; i++)
		deletedTag[i] = 0; 
		
	// get branching points
	V3DLONG *branchingNodeList=0, *branchingNodeIdx =0;
	V3DLONG branchingNodeNum;	
	swcTree::getBranchingNode(branchingNodeList, branchingNodeIdx, branchingNodeNum);	
	V3DLONG deletedNum = 0;
	
	for (i=0; i<branchingNodeNum; i++)
	{
	
		V3DLONG *childrenID, *childrenNodeIdx, childrenNum;
		swcTree::getDirectChildren(node[branchingNodeIdx[i]].nid, childrenID, childrenNodeIdx, childrenNum);

		for (j=0; j<childrenNum; j++)
		{
			swcTree *subtree = 0;
			V3DLONG *subtreeNodeIdx = 0;
				
			V3DLONG subTreeNodeNum;			
			swcTree::getSubTreeNodeNum(node[childrenNodeIdx[j]].nid, subTreeNodeNum);
			
			if (subTreeNodeNum<branchNodeNum)		
			{
				
				swcTree::getSubTree(childrenID[j], subtree, subtreeNodeIdx); // get the subtree rooted at each children of the note to be deleted
				for (k=0; k<subtree->treeNodeNum; k++)
				{
					deletedTag[subtreeNodeIdx[k]] = 1;
					deletedNum++;
				}
				
				if (subtree) {delete subtree; subtree=0;}
				if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx=0;}
					
			} //if
		} //for (j=0; j<childrenNum; j++)
		
		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
	} // for (i=0; i<branchingNodeNum; i++)
	
	//assign newTree
	V3DLONG idx = -1;
	
	newTree = new swcTree(treeNodeNum-deletedNum);
	newTree->treeNodeNum = treeNodeNum-deletedNum;
	
	for (j=0; j<treeNodeNum; j++)
	{
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;
			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
			newTree->node[idx].parentid = node[j].parentid;
		}
	}
	
	// delete pointers
	if (branchingNodeList) {delete []branchingNodeList; branchingNodeList=0;}
	if (branchingNodeIdx) {delete []branchingNodeIdx; branchingNodeIdx=0;}
	
}

//prune small branches based on branch length
// branchLengthThre: for any branching point, check the subtree rooted at each of its children
// if the maximum length is smaller than branchMaxLength, then that subbranch is pruned
// this function is useful if we want to do multi-resolution tree matching

void swcTree::pruneBranch(float branchLengthThre, swcTree *&newTree, V3DLONG *&deletedTag)
{
	V3DLONG i, j, k;
	V3DLONG nodeidx;
	
	newTree = 0;
	deletedTag = new V3DLONG [treeNodeNum]; //record which node has been deleted
		
	//initialize deleteTag, meaning no node has been deleted yet	
	for (i=0; i<treeNodeNum; i++)
	{
		deletedTag[i] = 0; 
//		printf("%d\n", node[i].nid);
	}
	
	// get branching points
	V3DLONG *branchingNodeList=0, *branchingNodeIdx =0;
	V3DLONG branchingNodeNum;	
	swcTree::getBranchingNode(branchingNodeList, branchingNodeIdx, branchingNodeNum);	
//	V3DLONG deletedNum = 0;
	
	for (i=0; i<branchingNodeNum; i++)
	{
	
		V3DLONG *childrenID, *childrenNodeIdx, childrenNum;
//		swcTree::getDirectChildren(node[branchingNodeIdx[i]].nid, childrenID, childrenNodeIdx, childrenNum);
		swcTree::getDirectChildren(branchingNodeList[i], childrenID, childrenNodeIdx, childrenNum);
//		printf("branchingNodeList[i] = %d, childrenNum = %d\n",branchingNodeList[i], childrenNum); 
		
		for (j=0; j<childrenNum; j++)
		{
			swcTree *subtree = 0;
			V3DLONG *subtreeNodeIdx = 0;
				
//			V3DLONG subTreeNodeNum;			
//			swcTree::getSubTreeNodeNum(node[childrenNodeIdx[j]].nid, subTreeNodeNum);
			float maxlength, minlength;
			V3DLONG maxLengthNodeid, minLengthNodeid;
			float totalLength;
			
//			swcTree::getSubTreeLength(node[childrenNodeIdx[j]].nid, maxlength, maxLengthNodeid,  minlength, minLengthNodeid, totalLength);
			swcTree::getSubTreeLength(childrenID[j], maxlength, maxLengthNodeid,  minlength, minLengthNodeid, totalLength);
			
//			printf("i=%d, j=%d, branchingNodeList[i] = %d, childrenID[j] = %d, maxlength%f\n", i,j, branchingNodeList[i], childrenID[j], maxlength);
			float dist;			
			swcTree::computeDistance(branchingNodeList[i], childrenID[j], dist);
			maxlength += dist;
			
			if (maxlength<branchLengthThre)		
			{
				
				
//				printf("id = %d, maxlength = %10.8f\n", childrenID[j], maxlength);
				
				swcTree::getSubTree(childrenID[j], subtree, subtreeNodeIdx); // get the subtree rooted at each children of the note to be deleted
				for (k=0; k<subtree->treeNodeNum; k++)
				{
					deletedTag[subtreeNodeIdx[k]] = 1;
//					deletedNum++;
				}
				
			} //if
			
			if (subtree) {delete subtree; subtree=0;}
			if (subtreeNodeIdx) {delete []subtreeNodeIdx; subtreeNodeIdx=0;}
			
		} //for (j=0; j<childrenNum; j++)
		
		if (childrenID) {delete []childrenID; childrenID=0;}
		if (childrenNodeIdx) {delete []childrenNodeIdx; childrenNodeIdx=0;}
		
	} // for (i=0; i<branchingNodeNum; i++)
	
	//assign newTree
	V3DLONG idx = -1;
	V3DLONG newTreeNodeNum = 0;
	
	for (j=0; j<treeNodeNum; j++)
		if (deletedTag[j] == 0) //node is kept
			newTreeNodeNum++;
			
	newTree = new swcTree(newTreeNodeNum);
	newTree->treeNodeNum = newTreeNodeNum;
	
	for (j=0; j<treeNodeNum; j++)
	{
		
		if (deletedTag[j] == 0) //node is kept
		{
			idx++;
			newTree->node[idx].nid = node[j].nid;
			newTree->node[idx].ntype = node[j].ntype;
			newTree->node[idx].x = node[j].x;
			newTree->node[idx].y = node[j].y;
			newTree->node[idx].z = node[j].z;
			newTree->node[idx].radius = node[j].radius;
			newTree->node[idx].parentid = node[j].parentid;
		}
	}
	
	// delete pointers
	if (branchingNodeList) {delete []branchingNodeList; branchingNodeList=0;}
	if (branchingNodeIdx) {delete []branchingNodeIdx; branchingNodeIdx=0;}
	
}

