//
//=======================================================================
// Copyright 2010 Institute PICB.
// Authors: Hang Xiao, Axel Mosig
// Data : July 11, 2010
//=======================================================================
//

#ifndef COMPONENT_TREE_H_H
#define COMPONENT_TREE_H_H

#include <iostream>
#include <vector>
#include <list>
using namespace std;


class ComponentTree
{
	public:
		class Pixel;
		class Node;

		typedef int Vertex;
		typedef vector<Vertex> Vertices;
		typedef vector<int> Path;
		typedef vector<Path> Paths;

		class Pixel
		{
			public:
				Pixel();
				bool save(ofstream& ofs, bool saveType = true) const;
				bool load(ifstream& ifs, vector<Pixel>& pixels, vector<Node*>& nodes, bool saveType=true);
				void merge_entry(Pixel* entry);
			public:
				int  pos;
				Pixel * next;		 // the next pixel
				unsigned short level;
				Node* node;
		};
		typedef vector<Node*> Nodes;

		class Node
		{
			public:
				Node(){};
				bool save(ofstream& ofs, bool saveType=true) const;
				bool load(ifstream& ifs, vector<Pixel>& pixels, vector<Node*>& nodes, bool saveType=true);
				vector<Pixel*> alpha_pixels();
				vector<Pixel*> beta_pixels();
				vector<int> alpha_points();
				vector<int> beta_points();
				void merge_node(Node* node);  // node may be a child
				Nodes getPostOrderNodes(); //return all the node which stores in post order, equilivalent to m_root
				Nodes getPreOrderNodes(); //return all the node which stores in post order, equilivalent to m_root
				Nodes getBreadthFirstNodes(); //return all the node which stores in post order, equilivalent to m_root

			public:
				int label;          // the store index in m_nodes, start from 0
				int lowest_level;          // the lowest level
				int highest_alpha_level;   // the highest alpha level
				int highest_beta_level();    // the highest beta level
				double mean_level();
				vector<double> center();

				int alpha_size;  // the pixel in the component exclude the pixels in child nodes
				int beta_size;   // the total number of pixels 
				Node* parent;    // we will make Node as dynamic memory, for the label is not easy to 
				Pixel* entry_pixel; // pixel will set static, the entry shoud set as the one of the lowest pixel in this component 
				vector<Node*> childs;
		};

	public:
		ComponentTree();
		ComponentTree(char * imgfile, int _minSize, int _maxSize, int _singleSize);
		ComponentTree(char* treefile);
		~ComponentTree();
		bool create(char * imgfile, int _minSize, int _maxSize, int _singleSize);

		bool load(const char* from_tree_file);
		bool reload(const char* from_tree_file);
		bool load(ifstream& ifs, bool saveType = true);
		bool save(const char* to_tree_file) const;
		bool save(ofstream& ofs, bool saveType = true) const;
		void clear();

		int width() const;
		int height() const;
		int depth() const;
		Node* root() const;
		Node* getNode(int) const;  //node of label 

		Paths getPaths() const;

		int* getReverseAlphaMapping() const; //get the matrix of labels
		int* getMatrix(vector<int> labels , vector<int> values, int ini_value) const; 
		void setWeightMatrix(ComponentTree* tree2, vector<float> &weights);

		int nodeNum() const;
		int leafNum() const;
		int pixelNum() const;

		void printTree(int label = -1) const;
		void printReverseAlphaMapping() const;
		void printPaths() const;

	private:
		void printTreeRecursively(int , int) const;

	private:
		int m_width ;
		int m_height;
		int m_depth;	

		int m_minSize;
		int m_maxSize;
		int m_singleSize;

		int m_numPixels;
		int m_numNodes;
		int m_numLeafs;

		vector<Pixel> m_pixels;
		Nodes m_nodes; //store the nodes in post order
		Nodes m_leafs; //store all the leafs
		Node* m_root;  //the root Node
};

class DisjointSets
{
	public:

		// Create an empty DisjointSets data structure
		DisjointSets();
		// Create a DisjointSets data structure with a specified number of pixels (with pixel id's from 0 to count-1)
		DisjointSets(int count);
		// Copy constructor
		DisjointSets(const DisjointSets & s);
		// Destructor
		~DisjointSets();

		// Find the set identifier that an pixel currently belongs to.
		// Note: some internal data is modified for optimization even though this method is consant.
		int FindSet(int pixel) const;
		// Combine two sets into one. All pixels in those two sets will share the same set id that can be gotten using FindSet.
		int Union(int setId1, int setId2);
		// Add a specified number of pixels to the DisjointSets data structure. The pixel id's of the new pixels are numbered
		// consequitively starting with the first never-before-used pixelId.
		void AddPixels(int numToAdd);
		// Returns the number of pixels currently in the DisjointSets data structure.
		int NumPixels() const;
		// Returns the number of sets currently in the DisjointSets data structure.
		int NumSets() const;

	private:

		// Internal Node data structure used for representing an pixel
		struct Node
		{
			int rank; // This roughly represent the max height of the node in its subtree
			int index; // The index of the pixel the node represents
			Node* parent; // The parent node of the node
		};

		int m_numPixels; // the number of pixels currently in the DisjointSets data structure.
		int m_numSets; // the number of sets currently in the DisjointSets data structure.
		std::vector<Node*> m_nodes; // the list of nodes representing the pixels
};

#endif
