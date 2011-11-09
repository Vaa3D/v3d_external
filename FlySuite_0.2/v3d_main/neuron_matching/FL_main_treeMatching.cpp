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




// main function of tree matching
// isomorphism tree matching using dynamic programming
// F. Long
// 20090109


//run -I 35_33LHregion_auto_align_branching_leaf_root_root10.swc -i 35_33LHregion_manual.swc -o res.txt
//run -I jneuron68_pruned_15.swc -i jneuron69_root406_pruned_15.swc -o res.txt 20090216
//run -I 35_33LHregion_auto_root10.swc -i 35_33LHregion_manual.swc -o res.txt
// run -I 35_33LHregion_auto_align_branching_leaf_root_root10_shift40.swc -i 35_33LHregion_manual.swc -o res.txt

//more than 2000 nodes, not tested, run -I ./data/two_trials_zeroed/auto_10/15_13LHregion.lsm.1.swc -R 270 -i ./data/two_trials_zeroed/trace_1_zero/15_13LHregion0.ASC.swc -o ./data/result/15_13LHregion0/res.txt

// systematic test
//run -I ./data/two_trials_zeroed/trace_1_zero/15_13LHregion0_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/15_13LHregion0_b.ASC.swc -o ./data/result/15_13LHregion0/
//run -I ./data/two_trials_zeroed/trace_1_zero/63_61LHregion_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/63_61LHregion_b.ASC.swc -o ./data/result/63_61LHregion/
//run -I ./data/two_trials_zeroed/trace_1_zero/59_57LHregion_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/59_57LHregion_b.ASC.swc -o ./data/result/59_57LHregion/
//run -I ./data/two_trials_zeroed/trace_1_zero/55_53LHregion_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/55_53LHregion_b.ASC.swc -o ./data/result/55_53LHregion/
//run -I ./data/two_trials_zeroed/trace_1_zero/51_49LHregion_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/51_49LHregion_b.ASC.swc -o ./data/result/51_49LHregion/
//run -I ./data/two_trials_zeroed/trace_1_zero/47_45LHregion_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/47_45LHregion_b.ASC.swc -o ./data/result/47_45LHregion/
//run -I ./data/two_trials_zeroed/trace_1_zero/43_41LHregion_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/43_41LHregion_b.ASC.swc -o ./data/result/43_41LHregion/ // sth wrong
//run -I ./data/two_trials_zeroed/trace_1_zero/31_29LHregion0_shift40.ASC.swc -i ./data/two_trials_zeroed/trace_2_zero/31_29LHregion0_b.ASC.swc -o ./data/result/31_29LHregion0/

// for recomb paper
// run -I ./data/result/recomb_paper/31_29LHregion0/31_29LHregion0_3x_rotate_xn5.ASC.swc -i ./data/result/recomb_paper/31_29LHregion0/31_29LHregion0_b_3x.ASC.swc -o ./data/result/recomb_paper/31_29LHregion0/
// run -I ./data/result/recomb_paper/47_45LHregion/47_45LHregion_3x_rotate_z5.ASC.swc -i ./data/result/recomb_paper/47_45LHregion/47_45LHregion_b_3x.ASC.swc -o ./data/result/recomb_paper/47_45LHregion/
// run -I ./data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc -i ./data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.5_dis15.0_shiftz40.swc -o ./data/result/recomb_paper/perturbation/31_29LHregion0/rate0.5_dis15.0/

//run -I ./data/result/recomb_paper/deletion/51_49LHregion/51_49LHregion_3x_shiftz40.ASC.swc -i ./data/result/recomb_paper/deletion/51_49LHregion/51_49LHregion_b_3x.ASC.swc_delete.swc -o ./data/result/recomb_paper/deletion/51_49LHregion/


//#include "FL_swcTree.h"
//#include "FL_readWriteTxtFile.cpp"
#include "FL_treeMatching.h"

//#include <string.h>
//#include <unistd.h>
//#include <stdio.h>

#include <string>
#include <unistd.h>
#include <math.h>
#include <string>
#include <stdlib.h>

using namespace std;

extern char *optarg;
extern int optind, opterr;

void printHelp();
void printHelp()
{
	printf("\nUsage: prog_name -I <tree1filename> -R <user_defined_root_node for tree1> -i <tree2filename> -r <user_defined_root_node for tree2> -p <length threshold of branch to be pruned> -c <remove continue points> -s <scale indicator> -o <outfilenamedir> \n");
	printf("[h]			help\n");
	return;
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}

	// ------------------
	// Read arguments 
	// ------------------

	V3DLONG int c;
	string infilename1, infilename2, outfilenamedir;	
//	static char optstring[] = "I:i:o:R:r:p:ch?";
	static char optstring[] = "I:i:o:R:r:p:o:sc";

	opterr = 0;
	bool b_tree1_custom_root=false, b_tree2_custom_root=false;
	V3DLONG tree1_cumtom_root, tree2_cumtom_root; 
	bool b_prune=false, b_removecontinuingnodes=false, b_scale = false; 
	float branchLengthThre = 15; //8,50

	while ((c = getopt (argc, argv, optstring)) != -1)
    {
		switch (c)
        {
			case 'h':
			case '?':
				printHelp ();
				return 0;
				break;

			case 'p':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -p.\n");
					return 1;
				}
				branchLengthThre = atof (optarg);
				if (branchLengthThre < 0)
				{
					fprintf (stdout, "Illeagal subject reference channelNo found! It must be >=0.\n");
					branchLengthThre=0;
				}
				b_prune=true;
				break;

			case 'c':
				b_removecontinuingnodes=true;
				break;

			case 's':
				b_scale=true; //scale matters, use length ratio instead of length， another way to consider is to normalize the scale beforehand
				break;
				
			case 'I': // file name of the input swc file of tree 1 (match tree1 against tree2)
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -I.\n");
					return 1;
				}
				infilename1 = optarg;
				break;
				
			case 'i': // file name of the input swc file of tree 2
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				infilename2 = optarg;
				break;
			case 'o': // output matching file, a text file indicating which node in tree 1 should be matched to which node in tree2
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				outfilenamedir = optarg;
				break;
				
			case 'R': //note there still could be bug if the specified rootnode is invalid (e.g. out of range)
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -R.\n");
					return 1;
				}
				tree1_cumtom_root = atoi (optarg);
				if (tree1_cumtom_root < 0)
				{
					fprintf (stderr, "Illeagal subject reference channelNo found! It must be >=0.\n");
					return 1;
				}
				b_tree1_custom_root=true;
				break;
				
			case 'r': //note there still could be bug if the specified rootnode is invalid (e.g. out of range)
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -r.\n");
					return 1;
				}
				tree2_cumtom_root = atoi (optarg);
				if (tree2_cumtom_root < 0)
				{
					fprintf (stderr, "Illeagal subject reference channelNo found! It must be >=0.\n");
					return 1;
				}
				b_tree2_custom_root=true;
				break;
				
			/* default:        abort (); */
        }
    }

	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);
				
	// ---------------------------------------
	// read input files into swc tree objects
	// ---------------------------------------

	swcTree *Tree1 = 0;
	swcTree *Tree2 = 0;
	
	Tree1->readSwcFile((char *)infilename1.c_str(), Tree1, 0); // allocate memory for Tree1 inside readSWCfile
	Tree2->readSwcFile((char *)infilename2.c_str(), Tree2, 0); // allocate memory for Tree1 inside readSWCfile
		
	// ---------------------------------------
	// test tree reforming using user defined root
	// ---------------------------------------	
	
	swcTree *newTree1 = 0;
	swcTree *newTree2 = 0;
	
	if (b_tree1_custom_root) 
	{
		Tree1->reformTree(tree1_cumtom_root, newTree1); 
		newTree1->writeSwcFile((char *)(infilename1+"_newroot.swc").c_str()); 
		if (Tree1) {delete Tree1; Tree1=newTree1; newTree1=0;} 
	}
	if (b_tree2_custom_root) 
	{
		Tree2->reformTree(tree2_cumtom_root, newTree2); 
		newTree2->writeSwcFile((char *)(infilename2+"_newroot.swc").c_str()); 
		if (Tree2) {delete Tree2; Tree2=newTree2; newTree2=0;} 
	}
	
	// ----------------------------------------------------------------------
	//  prune branch 
	// ----------------------------------------------------------------------
	
	// prune small branches
	
	V3DLONG *deletedTag = 0;
	
	if (b_prune)
	{
		Tree1->swcTree::pruneBranch(branchLengthThre, newTree1, deletedTag);
		newTree1->writeSwcFile((char *)(infilename1+"_pruned.swc").c_str()); 
		if (Tree1) {delete Tree1; Tree1=newTree1; newTree1=0;} 

		Tree2->swcTree::pruneBranch(branchLengthThre, newTree2, deletedTag);
		newTree2->writeSwcFile((char *)(infilename2+"_pruned.swc").c_str()); 
		if (Tree2) {delete Tree2; Tree2=newTree2; newTree2=0;} 
	}
//	newTree1->writeSwcFile("diffneuron1.swc");
//	newTree2->writeSwcFile("diffneuron2.swc");
	
	// ---------------------------------------------------
	// remove continual nodes in Tree1 and Tree2
	// ---------------------------------------------------
	
	// remove continual nodes  
	float maxlength, minlength, totalLength;
	V3DLONG maxLengthNodeid, minLengthNodeid;
	unsigned char *removeTag1=0, *removeTag2=0;
	
//	V3DLONG rootid, rootidx;
//	newTree1->getRootID(rootid, rootidx);
	
	if (b_removecontinuingnodes)
	{
		Tree1->removeContinualNodes(newTree1, removeTag1);
		newTree1->writeSwcFile((char *)(infilename1+"_cnoderemoved.swc").c_str()); 
		if (Tree1) {delete Tree1; Tree1=newTree1; newTree1=0;} 

		Tree2->removeContinualNodes(newTree2, removeTag2);	
		newTree2->writeSwcFile((char *)(infilename2+"_cnoderemoved.swc").c_str()); 
		if (Tree2) {delete Tree2; Tree2=newTree2; newTree2=0;} 
	}
	
	if (removeTag1) {delete removeTag1; removeTag1=0;}
	if (removeTag2) {delete removeTag2; removeTag2=0;}

	
// ##################################
// experiments for recomb paper, keep
// ##################################

// ------------------------------------------------------
//	// randomly perturb some nodes in the tree to generate results for recomb paper
//	// ------------------------------------------------------------------------------
//	
//	swcTree *newTree = 0;
//	V3DLONG *perturbedTag =0;
//	string filename;
//	
//	float rate = 0.5;
//	float maxdis = 15;
//	
//	Tree1->randomPerturbCoordinates(rate, maxdis, newTree, perturbedTag); // assuming Tree2 is the original tree without deleting any nodes
//
////	filename = outfilenamedir+infilename1+"Tree1_perturbed_rate1_dis2.swc";
//
//	char filenametmp[50];
//	sprintf(filenametmp, "_perturbed_rate%3.1f_dis%3.1f.swc", rate, maxdis);	
//
////	filename = infilename1+"_perturbed_rate1_dis2.swc";
//	filename = infilename1+filenametmp;
//	
//	newTree->writeSwcFile(filename.c_str());
//	
//	if (newTree) {delete newTree; newTree=0;}
//	if (perturbedTag) {delete []perturbedTag; perturbedTag=0;}	
//	if (Tree1) {delete Tree1; Tree1=0;}
//	if (Tree2) {delete Tree2; Tree2=0;}
//	
//	return 1;
//
//    // print number of leaf and branching nodes computing matching accurarcy of perturbation
//	V3DLONG *leafNodeList=0, *leafNodeIdx=0, leafNodeNum;
//	Tree1->getLeafNode(leafNodeList, leafNodeIdx, leafNodeNum);
//	printf("Number of leaf nodes = %d", leafNodeNum);
//	if (leafNodeList) {delete []leafNodeList; leafNodeList=0;}
//	if (leafNodeIdx) {delete []leafNodeIdx; leafNodeIdx=0;}
//
//	V3DLONG *branchingNodeList=0, *branchingNodeIdx=0, branchingNodeNum;
//    Tree1->getBranchingNode(branchingNodeList, branchingNodeIdx, branchingNodeNum);
//	printf("Number of branching nodes = %d", branchingNodeNum);
//
//	if (branchingNodeList) {delete []branchingNodeList; branchingNodeList=0;}
//	if (branchingNodeIdx) {delete []branchingNodeIdx; branchingNodeIdx=0;}
	
//	//------------------------------------------------------------------------------------------------
//	// delete branches in Trees to test program ability in handling missing or additional branches
// // for recomb paper
//	//------------------------------------------------------------------------------------------------
//	
//	V3DLONG nodeIDList[2]= {77, 169}; //51_49LHregion_b_3x.ASC.swc
//	V3DLONG nodeIDNum = 2;

//	V3DLONG nodeIDList[1]= {51}; //51_49LHregion_b_3x.ASC.swc
//	V3DLONG nodeIDNum = 1;
//
//	V3DLONG deleteNodeNum, *deleteTag = 0;
//	swcTree *newTree=0;
//	Tree1->deleteSubTreeNode(nodeIDList, nodeIDNum, deleteNodeNum, deleteTag, newTree);
//	
//	string filenametmp;
//	filenametmp = infilename1+"_delete.swc";	
//	newTree->writeSwcFile(filenametmp.c_str());
//	
//	if (deleteTag) {delete []deleteTag; deleteTag=0;}
//	if (newTree) {delete newTree; newTree=0;}


	// ---------------------------------------
	// match Tree1 against Tree2
	// ---------------------------------------


	V3DLONG *matchingList =0;
	float simMeasure;
	
////	treeMatching_allNodes(Tree1, Tree2, matchingList, simMeasure);
//	treeMatching_noContinualNodes(Tree1, Tree2, matchingList, simMeasure); //test structural matching
//	treeMatching_noContinualNodes_lengthRatio_simDependent(Tree1, Tree2, matchingList, simMeasure);

//	float lengthThre1 = 50;
//	float lengthThre2 = 10;
//	treeMatching_noContinualNodes_lengthRatioAngle_simDependent(Tree1, Tree2, matchingList, simMeasure, lengthThre1, lengthThre2);

	treeMatching_hierarchical(Tree1, Tree2, b_scale, matchingList, simMeasure, outfilenamedir);
	
	
//	// ---------------------------------------
//	// compute matching accuracy for test experiment
//    // useful for simulation experiments, such as 
//    // match a neuron and its pertubation
//	// ---------------------------------------
//	
//	V3DLONG misMatchNum = 0;
//	
//	for (int i=0; i<Tree1->treeNodeNum; i++)
//	{
//
//		if (Tree1->node[i].nid != matchingList[i])
//			misMatchNum++;
//
//	}
//	
//	printf("Number of mismatches are: %d, about %f percent of nodes. \n", misMatchNum, float(misMatchNum)/Tree1->treeNodeNum*100);
	
	
	// -------------------------------------------------
	// output matching result into the output file
	// -------------------------------------------------
	
	// old method
	FILE *file;	
//	file = fopen((char *)outfilenamedir.c_str(), "wt");
	file = fopen((char *)(outfilenamedir+"res.txt").c_str(), "wt");
	V3DLONG matchedNodeNum =0;
	V3DLONG *matchedNodeID = new V3DLONG [Tree1->treeNodeNum];
	
	if (file == NULL)
	{
		printf("error to open file\n");
		return 0; 
	}
		
	V3DLONG len = MAX_CHILDREN_NUM+2;
	
	for (int i=0; i<Tree1->treeNodeNum; i++)
	{
		fprintf(file, "%d %d %d\n", Tree1->node[i].nid, matchingList[i*len], matchingList[i*len+len-1]); // add at which hierarchy the node is matched
	}
	fclose(file);
	
	// write another file for display hierarchy purpose
//	file  = fopen((char *)(outfilenamedir+"_allterms.txt").c_str(), "wt");
	file  = fopen((char *)(outfilenamedir+"res_allterms.txt").c_str(), "wt");

	if (file == NULL)
	{
		printf("error to open file\n");
		return 0; 
	}
			
	for (int i=0; i<Tree1->treeNodeNum; i++)
	{
		if (matchingList[i*len]>0)
		{
		
			matchedNodeID[2*matchedNodeNum] = Tree1->node[i].nid;
			matchedNodeID[2*matchedNodeNum+1] = matchingList[i*len];
			matchedNodeNum++;
			
			fprintf(file, "%d ", Tree1->node[i].nid);
			
			for (int j=0; j<len; j++)
			{
				fprintf(file, "%d ",  matchingList[i*len+j]); // add at which hierarchy the node is matched
			}
			fprintf(file, "\n");
		}
	}
	
	fclose(file);
	
//	// ---------------------------------------
//	//  Register Tree1 to Tree2, important, keep
//	// ---------------------------------------
//	
//	affineTransformTrees(Tree1, Tree2, matchedNodeID, matchedNodeNum);
//	string filename;
//	filename = infilename1+"Tree1_registered.swc";
//	
//	Tree1->writeSwcFile(filename.c_str());

	// ---------------------------------------
	// delete pointers, free spaces
	// ---------------------------------------

	if (matchingList) {delete []matchingList; matchingList=0;}
	if (Tree1) {delete Tree1; Tree1=0;}
	if (Tree2) {delete Tree2; Tree2=0;}



// #########################################
// The followings are some early experiments
// keep them
// #########################################

//	// ---------------------------------------
//	//randomly delete some nodes in the tree
//	// ---------------------------------------	
//	
//	swcTree *newTree = 0;
//	V3DLONG *deletedTag;
//
//	Tree2->randomDeleteNodes(80, 0.2, newTree, deletedTag); // assuming Tree2 is the original tree without deleting any nodes
//	
//	newTree->writeSwcFile("testreal3.swc");
//	
//	if (newTree) {delete newTree; newTree=0;}
//	if (deletedTag) {delete []deletedTag; deletedTag=0;}

//	// ---------------------------------------
//	//randomly delete some branch nodes in the tree
//	// ---------------------------------------	
//	
//	swcTree *newTree = 0;
//	V3DLONG *deletedTag;
//	
//	Tree2->randomDeleteBranch(80, 0.2, newTree, deletedTag);  // the first argument should be at least 3, assuming Tree2 is the original tree without deleting any nodes
//	newTree->writeSwcFile("testreal3.swc");
//	
//	if (newTree) {delete newTree; newTree=0;}
//	if (deletedTag) {delete []deletedTag; deletedTag=0;}

	
//	// ---------------------------------------
//	//randomly perturb some nodes in the tree
//	// ---------------------------------------	
//	
//	swcTree *newTree = 0;
//	V3DLONG *perturbedTag =0;
//	
//	Tree2->randomPerturbCoordinates(1, 2, newTree, perturbedTag); // assuming Tree2 is the original tree without deleting any nodes
//	
//	newTree->writeSwcFile("testreal3.swc");
//	
//	if (newTree) {delete newTree; newTree=0;}
//	if (perturbedTag) {delete []perturbedTag; perturbedTag=0;}	
//	
//	return 1;

//	// ---------------------------------------
//	// match significant branching point in two trees
//	// ---------------------------------------	
//	
//	V3DLONG *sigBranchPointMatchingList = 0;
//	float simMeasure;
//	V3DLONG sigBranchPointNum;
//	
//	sigBranchingPointMatching(Tree1, Tree2, 3, sigBranchPointMatchingList, sigBranchPointNum, simMeasure);
//	
//	// ---------------------------------------
//	// output matching result into the output file
//	// ---------------------------------------
//	
//	FILE *file;	
//	file = fopen(outfilename, "wt");
//	if (file == NULL)
//	{
//		printf("error to open file\n");
//		return 0; 
//	}
//		
//	for (int i=0; i<sigBranchPointNum; i++)
//	{
//		fprintf(file, "%d %d\n", sigBranchPointMatchingList[i*sigBranchPointNum], sigBranchPointMatchingList[i*sigBranchPointNum+1]);
//
//	}
//	fclose(file);

// ---------------------------------------
//temporary for tree matching code debug, keep
// ---------------------------------------

//	swcTree *subtree1 = 0;
//	V3DLONG *subtreeNodeIdx1 = 0;
//
//	swcTree *subtree2 = 0;
//	V3DLONG *subtreeNodeIdx2 = 0;
//	
//	Tree1->getSubTree(131, subtree1, subtreeNodeIdx1);
//	Tree2->getSubTree(50, subtree2, subtreeNodeIdx2);
//	
//	for (int i=0; i<subtree1->treeNodeNum; i++)
//		printf("%d, ",  Tree1->node[subtreeNodeIdx1[i]].nid);
//	printf("\n");
//	
//	for (int i=0; i<subtree2->treeNodeNum; i++)
//		printf("%d, ",  Tree2->node[subtreeNodeIdx2[i]].nid);
//	printf("\n");
//
//	if (subtree1) {delete subtree1; subtree1=0;}
//	if (subtree2) {delete subtree2; subtree2=0;}
//	if (subtreeNodeIdx1) {delete subtreeNodeIdx1; subtreeNodeIdx1=0;}
//	if (subtreeNodeIdx2) {delete subtreeNodeIdx2; subtreeNodeIdx2=0;}



}



