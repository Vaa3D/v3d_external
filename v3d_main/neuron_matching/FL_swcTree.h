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

#ifndef __SWC_TREE__
#define __SWC_TREE__

#define INVALID_VALUE -9999
#define MAX_CHILDREN_NUM 6 

class swcNode
{
public:
	V3DLONG nid; //node id
	unsigned char ntype; //node type, soma: 1; axon: 2; basal dendrite: 3; apical dendrite: 4
	float x,y,z; //x,y,z coordinate
	float radius; //radius 
	V3DLONG parentid;	// parent node id, root:-1
};

class swcTree
{
public:
	swcNode *node;
	V3DLONG treeNodeNum;
	
public:
	swcTree()
	{
		treeNodeNum = 0;
		node = 0;
	}
	
	swcTree(V3DLONG nodeNum)
	{
		treeNodeNum = nodeNum;
		node = new swcNode [nodeNum];
		
		for (int i=0; i<treeNodeNum; i++)
		{ 
			node[i].nid = 0; 
			node[i].ntype = 0;
			node[i].x = 0;
			node[i].y = 0;
			node[i].z = 0;
			node[i].radius = 0;
			node[i].parentid = 0;
		}
	}
	
	~swcTree()
	{
		if (node) {delete []node; node = 0;}
		treeNodeNum = 0;
	}

	bool readSwcFile(char *infilename, swcTree *&Tree, bool tagprint);
	bool writeSwcFile(const char *outfilename);	
	void printSwcTree();
	void genGraphvizFile(char *outfilename); //output the tree as the dot file to visualize in Graphviz

	void decompose(V3DLONG *seedNodeID, V3DLONG seedNodeNum, swcTree **&subTrees, V3DLONG &subTreeNum, V3DLONG *&branchIdx); // decompose trees into multiple sub-trees based on give seedNodeId
	void decompose(V3DLONG *seedNodeID, V3DLONG seedNodeNum, swcTree **&subTrees, V3DLONG &subTreeNum, V3DLONG *&branchIdx, V3DLONG *&subTreeSeeds); //20090911, add subTreeSeeds for matlab display code
	 
	void copyTree(swcTree *&newTree); //copy tree
	
	void getRootID(V3DLONG &rootid, V3DLONG &rootidx); // get the id and index of root of the tree
	void getIndex(V3DLONG nodeid, V3DLONG &nodeidx); // get the index of the nodeid
	void findPath(V3DLONG nodeid1, V3DLONG nodeid2, V3DLONG *&pathNodeList, V3DLONG &pathNodeNum, unsigned char &ancestorTag); // find a path between nodeid1, nodeid2
	
	void computeDepth(V3DLONG *&depth); // compute depth for each node in the tree
	void computeDepth(V3DLONG &depth, V3DLONG nodeid); //compute depth for a particular node with id value equals nodeid
	void computeDistance(V3DLONG nodeid1, V3DLONG nodeid2, float &dist); //compute physical distance between two nodes of the tree in the current coordinate system
	void computeLength(V3DLONG nodeid, unsigned char parentTag, float *&length, unsigned char &lengthNum); //compute the length between two nodes that are directly parenet and child
	void computeLength(V3DLONG nodeid1, V3DLONG nodeid2, float &length); //compute the length between any two nodes
	void computeTreeDiameter(float &treeDiameter, V3DLONG &leafNodeID1, V3DLONG &leafNodeID2); // compute the diameter of the tree
	void computeTreeCenter(float *&treeCenter, float &averageRadius); // compute the center of the tree
	
	void computeLengthWeightedDistance(V3DLONG nodeid, V3DLONG *NodeList, V3DLONG NodeListNum, float *&dist); // compute the sum of the length weighted distance from nodeid to all the nodes in Nodelist
	void computeLengthWeightedDistanceNormalized(V3DLONG nodeid, float &dist, unsigned char method); // compute the normalized length weighted distance of a node in the tree
	
	void computeBranchLengthRatio(V3DLONG nodeid, float *&lengthratio, unsigned char &lengthratioNum, unsigned char method); // compute the branch length ratio of nodeid
	void computeOrientation(V3DLONG parentNodeid, V3DLONG childNodeid, float &angle1, float &angle2); // compute the orientation of the brunch between parentNodeid and childNodeid in the current coordinate system, they need to be direct parent and child 
//	void computeBranchingAngles(V3DLONG nodeid, float *&childrenAngles, unsigned char childrenAnglesNum, float *&parentChildAngles, unsigned char parentChildAnglesNum); // compute branching angles of the node
	void computeAngle(V3DLONG nodeid, V3DLONG nodeid1, V3DLONG nodeid2, float &angle); // compute the angle between the line connecting nodeid, nodeid1, and nodeid, nodeid2
	void computeAngle(float *vec1, float *vec2, float &angle); // compute the angle between two vectors	
	void computeBranchingAngles(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, float *&angles); // compute the angles between pair-wise vectors formed by direct children and parent
	void computeBranchingAngles(V3DLONG nodeid, V3DLONG &branchNum, float *&angles); // compute the angles between pair-wise vectors formed by the average vector of children path and the average vector of parent path
	void computeLeafNodeAngles(V3DLONG nodeid, float *vec, V3DLONG *leafNodeListID, V3DLONG leafNodeNum, float *&angles); // compute the angles between the vectors formed by the leaf ndoes, current tree root, and anterior tree root, this function is called by leaf node matching
	
//	void computeBranchSizeAngles(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, V3DLONG *&branchNodeNum, float *&angles); // compute the angles and branch size by combining functions computeBranchingAngles and getBranchSize to remove some redunt compuation if both size and angles need to compute
	void computeBranchNodeNumAngles(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, V3DLONG *&branchNodeNum, float *&angles); // compute the angles and branch size by combining functions computeBranchingAngles and getBranchSize to remove some redunt compuation if both size and angles need to compute
	void computeBranchLengthSumAngles(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, float *&lengthSum, float *&angles); // compute the angles, and sum of sub-branch lengths
		
	void sortTree(V3DLONG *&nodeidx); //sort the tree in depth
	
//	void sigBranchingNodeThre(swcTree *Tree, float ratio1, float ratio2, float &lengthThre1, float &lengthThre2); //adpatively determine thresholds for keeping significant branching nodes
	
	void removeContinualNodes(swcTree *&newTree, unsigned char *&removeTag);//remove all continual nodes
	void removeContinuaLeaflNodes(swcTree *&newTree, unsigned char *&removeTag); //remove all continual nodes and leaf nodes
	void removeInsignificantNodes(float lengthThre1, float lengthThre2, swcTree *&newTree, bool keepLeaf, bool *&removeNodeTag, V3DLONG **&removeBranchTag); //remove all continual nodes,  and subtrees of insignificant branching nodes, leaf nodes of significant branching points are kept
	void removeInsignificantNodes(float *lengthRatioThre, swcTree *&newTree, bool keepLeaf, bool *&removeNodeTag, V3DLONG **&removeBranchTag); //remove all continual nodes,  and subtrees of insignificant branching nodes, leaf nodes of significant branching points are kept
	void detectCriticalBranchNodes(float *threVector, unsigned char leafMethod, unsigned char criticalNodeMethod, bool *&removeNodeTag, V3DLONG **&removeBranchTag, swcTree *&newTree, swcTree *&newTreeWithPath); //detect critical branch nodes and output new trees defined on critical branch nodes (and leaf nodes)
//	void detectCriticalBranchNodes(float *threVector, unsigned char leafMethod, unsigned char criticalNodeMethod, swcTree *Tree, bool *&removeNodeTag, V3DLONG **&removeBranchTag, swcTree *&newTree, swcTree *&newTreeWithPath); //detect critical branch nodes and output new trees defined on critical branch nodes (and leaf nodes)
	
	void getSubTree(V3DLONG nodeid, swcTree *&subtree, V3DLONG *&subtreeNodeIdx); // get the sub tree rooted at a particular node	
	void getSubTree(V3DLONG nodeid, V3DLONG depthThre, swcTree *&subtree, V3DLONG *&subtreeNodeIdx); // get the parital sub tree rooted at a particular node, nodes within the subtree have a depth no bigger then depthThre with respect to the root nodeid
	void getSubTree(V3DLONG nodeid, float subtreeRatioThre, swcTree *&subtree, V3DLONG *&subtreeNodeIdx); // get the parital sub tree rooted at a particular node, nodes within the subtree have a length ratio less that lengthRatioThre with respect to the root nodeid

	void getSubTreeNodeNum(V3DLONG *&subTreeNodeNum); // get the number of nodes of the sub tree rooted at each node
	void getSubTreeNodeNum(V3DLONG nodeid, V3DLONG &subTreeNodeNum); // get the number of nodes of the sub tree rooted at a particular node
	void sortSubTreeNodeNum(V3DLONG *subTreeNodeNum, float *&sortval, float *&sortidx); //sort nodes in a tree according to their sub tree node number 
	void getSubTreeLength(V3DLONG nodeid, float &maxlength, V3DLONG &maxLengthNodeid, float &minlength, V3DLONG &minLengthNodeid, float &totalLength); // get the maximum, minimum length between nodeid and a node, and the total length of the subtree rooted at nodeid 
	void getSubTreeLength(V3DLONG nodeid, float &maxlength, V3DLONG &maxLengthNodeid, float &secondmaxlength, V3DLONG &secondmaxLengthNodeid, float &minlength, V3DLONG &minLengthNodeid, float &totalLength); // get the maximum, second maximum, minimum length between nodeid and a node, and the total length of the subtree rooted at nodeid

//	void getSubTreeMaxLength(V3DLONG nodeid, float &maxlength, V3DLONG &maxLengthNodeid,  float *&totalLength); // get the maximum length between nodeid and a node in its subtree	
//	void getSubTreeTotalLength(V3DLONG nodeid, float &subTreeTotalLength); // get the total length of the subtree rooted at nodeid
//	void getChildrenBranchNodeNum(V3DLONG nodeid, V3DLONG *&directChildrenID, V3DLONG *&directChildrenIdx, V3DLONG &directChildrenNum, V3DLONG *&childrenBranchNodeNum); // get the number of nodes of each children branch  
//	void getBranchSize(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, V3DLONG *&branchNodeNum); // get the number of nodes of each children branch and the remaining branch not belong to children 
	void getBranchNodeNum(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, V3DLONG *&branchNodeNum); // get the number of nodes of branch split from nodeid, i.e., each children branch and the remaining branch not belong to children 

	void getDirectChildren(V3DLONG **&childrenList, V3DLONG *&childrenNum, unsigned char maxChildrenNum ); //get the direct children for each node in the tree
	void getDirectChildren(V3DLONG nodeid, V3DLONG *&childrenID, V3DLONG *&childrenNodeIdx, V3DLONG &childrenNum); // get the direct children of nodeid
	void getAllChildren(V3DLONG nodeid, V3DLONG *&childrenID, V3DLONG *&childrenNodeIdx, V3DLONG &childrenNum); // get all the children of nodeid
	void getAllAncestors(V3DLONG nodeid, V3DLONG *&ancestorID, V3DLONG *&ancestorIdx, V3DLONG &ancestorNum); // get all ancestors of nodeid
	void getAncestors(V3DLONG nodeid, V3DLONG *&ancestorsID, V3DLONG *&ancestorsIdx, V3DLONG &ancestorsNum); // get all the ancestors of nodeid, should combine with the above function, the same
	
	void getNodeStructuralType(unsigned char *&stype, unsigned char &maxfurcation); //get the structural type of each node in the tree, -1: root; 0: leaf; 1: continuation; 2: bifurcation; 3: trifurcation ...
	void getLeafNode(V3DLONG *&leafNodeList, V3DLONG *&leafNodeIdx, V3DLONG &leafNodeNum); // get leaf nodes from the tree
	void getLeafNode(float lengthThre, V3DLONG *&leafNodeList, V3DLONG *&leafNodeIdx, V3DLONG &leafNodeNum); // get significant leaf nodes, ignoring those on small branches determiend by lengthThre
	void getBifurcation(V3DLONG *&bifurNodeList, V3DLONG *&bifurfNodeIdx, V3DLONG &bifurNodeNum); //get birfurcation node from the tree
	void getBranchingNode(V3DLONG *&branchingNodeList, V3DLONG *&branchingNodeIdx, V3DLONG &branchingNodeNum); //get branching nodes (can be bifurcation and multi-furcation) from the tree
	void getTypedNodes(V3DLONG *&typedNodeList, V3DLONG *&typedNodeIdx, V3DLONG &typedNodeNum, unsigned char nodeType); //combine the function of getLeafNode, getBifurcation, getBranchingNode into one function
	void getSignificantBranchingNode(V3DLONG branchSizeThre, V3DLONG *&branchingNodeList, V3DLONG *&branchingNodeIdx, V3DLONG &branchingNodeNum); //get significant branching points, each branch needs to be bigger than branchSizeThre	
	
//	void deleteNode(V3DLONG nodeid, swcTree *&newTree); // delete a node in the tree to geneate a new tree, note that when nodeid is a branching point, what delected is infact a randomly selected branch
//	void deleteBranch(V3DLONG nodeid, swcTree *&newTree); //delete a branch of nodes rooted at nodeid	
	void randomDeleteNodes(V3DLONG branchSizeThre, float rate, swcTree *&newTree, V3DLONG *&deletedTag); //randomly delete some nodes in the tree according to some probability
	void randomDeleteBranch(V3DLONG branchSizeThre, float rate, swcTree *&newTree, V3DLONG *&deletedTag); //randomly delete some branches in the tree according to some probability
	void randomPerturbCoordinates(float rate, float maxdis, swcTree *&newTree, V3DLONG *&perturbedTag);// randomly select some nodes to perturb their 3d coordinates slightly 
	void deleteTreeNode(V3DLONG *nodeidlist, V3DLONG deleteNodeNum, swcTree *&newTree); //delete a list of nodes in the tree and generate a new tree
	void deleteSubTreeNode(V3DLONG *nodeIDList, V3DLONG nodeIDNum, V3DLONG &deleteNodeNum, V3DLONG *&deleteTag, swcTree *&newTree); // delete subtree rooted at each nodeid in nodeIDList, include nodeid

	void buildSuperNodeTree(V3DLONG *deleteNodeIDlist, V3DLONG *ancesterIDList, V3DLONG deleteNodeNum, swcTree *&newTree); //Build super node tree by deleting a list of nodes specified by deleteNodeIDlist, making the children of each node the children of its ancester node listed in ancesterIDList
	void buildSuperNodeTree(V3DLONG deleteNodeID, V3DLONG ancesterID, swcTree *&newTree); //Build super node tree, delete only one node 
	
	void pruneBranch(V3DLONG branchNodeNum, swcTree *&newTree, V3DLONG *&deletedTag); //prune small branches based on branch nodes
	void pruneBranch(float branchMaxLength, swcTree *&newTree, V3DLONG *&deletedTag); //prune small branches based on branch length

	void reformTree(V3DLONG rootid, swcTree *&newTree); // reform tree structure using a node (different from that of the original) as the root)
	void tree2UndirectedGraph(unsigned char *&graph);  //turn tree into an undirected graph

//	not implemented yet
	void affineTransform(float *T, swcTree *&newTree); //apply global affine transformation to each node in the tree 
	void getBranchTotalLength(V3DLONG nodeid, V3DLONG *&branchRootID, V3DLONG *&branchRootIdx, V3DLONG &branchNum, float *&totalLength); // get the total length of each branch split from nodeid;

};
	
#endif





