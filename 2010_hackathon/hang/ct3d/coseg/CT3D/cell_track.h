//
//=======================================================================
// Copyright 2010 Institute PICB.
// Authors: Hang Xiao, Axel Mosig
// Data : July 14, 2010
//=======================================================================
//

#ifndef CELL_TRACK_H_H
#define CELL_TRACK_H_H
#include <vector>
#include <set>
#include <map>
#include "../component_tree.h"

#define DEFAULT_MIN_FILTER 800
#define DEFAULT_MAX_FILTER 10000
#define DEFAULT_SINGLE_FILTER 100
#define DEFAULT_INPUTDIR "./input"
#define DEFAULT_OUTPUTDIR "./output"

using namespace std;

class CellTrack
{	
public:
	
	class Cell;
	
	class Track;
	
	typedef vector<Cell*> Frame;
	
	class rgb_pixel;
	
	class palette;
	
	class Cell
	{
	public:
		Cell()
		{
			label = 0;
			trackId = 0;
			prev = NULL;
			next = NULL;
		}
	public:
		int label;
		
		int prevLabel;
		
		int nextLabel;
				
		int trackId;
		
		Cell* prev;
		
		Cell* next;
		
	};
	
	class Track 
	{
	public:
		Track()
		{
			trackId = 0;
			altitude = 0.0;
			start = 0;
			end = 0;
			entry = NULL;
		}
		public :
		int trackId;
		
		double altitude;
		
		int start; //the first cell's frame id, the minimul is 0
		
		int end; // the id after last cell's frame id, the maximul is m_numFrames
		// end - start is the num of cells in the track
		Cell* entry;
	};
	
	// =====================================================
	class rgb_pixel
	// =====================================================
	{
		
	public:
		
		unsigned char r,g,b;
		
	public:
		
		rgb_pixel(unsigned char r_, unsigned char g_, unsigned char b_);
		
		rgb_pixel(unsigned int rgb);
		
		rgb_pixel();
		
		bool operator==(const rgb_pixel& P);
		
		bool operator<(const rgb_pixel& P) const;
		
		bool operator!=(const rgb_pixel& P);
		
	};
	// =====================================================
	// =====================================================
	
	
	
	// =====================================================
	class palette
	// =====================================================
	{
		
	public:
		
		typedef set<int> int_set;
		
	private:
		
		typedef map<rgb_pixel,int> reverse_palette_map;
		
		int palette_size;
		
		int_set all_colors;
		
		vector<rgb_pixel> this_palette;
		reverse_palette_map reverse_palette;
		
	public:
		
		int_set& get_all_colors();
		
		palette();
		
		palette(int size);
		
		int size();
		
		rgb_pixel operator()(int i);
		
		int operator()(const rgb_pixel& p);
		
		void init(int size);
		
	};
	// =====================================================
	
	class Alignment
	{
	public:
		Alignment();
		~Alignment();
		bool align(ComponentTree&, ComponentTree&);
		bool align(ComponentTree&, Frame& ,Frame&);
		double* result();
		void clear();
		
	private:
		vector< vector<float> >  m_weightMatrix;  //use float instead of double to save space
		int m_numVars1;      // first is the num of nodes in tree1, another in tree2
		int m_numVars2;
		double* m_row;       // store the match result
		// are mapped one by one
	};
	
public:
	CellTrack();
			
	~CellTrack();
	
	void clear();  //clear all information except m_trees

	void createTracking(int argc, char* argv[]);
	
	bool saveTracking(char* trackfile = "ct3d.tr");
	
	bool loadTracking(int argc, char* argv[]);
	
	void saveImages();
	
	void saveGraph() const;
	
	void saveGraph2() const;
	
	bool tracking();
	
	void createPalette();
		
	int frameCount() const; 
	
	int trackCount() const;
	
	ComponentTree& getTree(int);
	
	void filterCells(double threshold = -1.0);
	
	void resetCells();
	
private:
	
	vector<ComponentTree> m_trees;
	
	vector<Frame> m_frames; 
	
	vector<Track> m_tracks; 
	
	vector< map<int,int> > m_matches;
		
	int m_numFrames;
	
	int m_numTracks;
	
	palette m_palette;
	
	vector<string> m_names;
	
	//string m_indir;
	
	string m_outdir;
	
	double m_threshold;
};

#endif
