//=======================================================================
// Hang Xiao, Augest 1, 2010
//=======================================================================


#ifndef CELLTRACK_H_H
#define CELLTRACK_H_H
#include "../component_tree.h"
#include <vector>
#include <stack>
#include <map>
using namespace std;

class AT3D
{
	// types
public:
	class Track;
	
	class Cell;
	
	typedef vector<Cell*> Frame;
	
	class rgb_pixel;
	
	typedef map<rgb_pixel, int> Palette;
	
	//AT3D
public:
	AT3D();
	
	AT3D(vector<string>&);
	
	void clear();
	
	bool isTreeLoad();
	
	bool empty();
	
	void push_state();
	
	void pop_state();

//slots	
	void create(vector<string>&);
	
	void createTracks(vector<string>& filenames);
	
	bool setTrees(string);
	
	void saveFrames(string);
	
	void choose(vector<Track*>&);
	
	void remove(vector<Track*>&);
	
	void undo();
	
//access
	Frame& frame(int);
	
	Track& track(int);

	ComponentTree& tree(int);
		
	int frameCount();
	
	int trackCount();
	
	int width();
	
	int height();
	
	int depth();
	
// filter functions
	enum{CURRENTVOLUME = 0, VOLUME = 1, LIFE = 2, SPEED = 3, DEFORM = 4};
	double maxMeanVolume();
	double maxMeanSpeed();
	double maxMeanDeform();
	void filtering(int time, const vector< pair<double, double> >&);
	
private:
	
	void init();
	
	void setFrames(); //use trackIds to set frames
	
	void sortFrame(Frame& frame);
	
	void sortTracks(vector<Track>&);
	

	
public:
	vector<string> m_filenames;
	
	vector<Frame> m_frames;
	
	vector<ComponentTree> m_trees;
	
	int m_numFrames;
	
	Palette m_palette; 
	
	vector<Track> m_tracks; // store the main dat
	
	vector<Track*> m_ptracks;  //directly control Frame's content
	
	int m_numTracks;
	
	int m_width;
	
	int m_height;
	
	int m_depth;
	
	stack< vector<Track*> > m_states;
	
	// class Cell , Track, rgb_pixel
public:
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
		
		bool operator<(const rgb_pixel& P) const; //const tail is very in map
				
		bool operator!=(const rgb_pixel& P);
		
	};
	// =====================================================
	
	
	//===========================================
	class Cell
	//===========================================
	{
	public:
		Cell();
		
		Cell(Track*, int, rgb_pixel);
				
		int size() const;
				
		void setCenters(int , int);
		
		double deformation(Cell& prev, int width, int height);
		
	public:
		Track* track;
		
		int time;

		ComponentTree::Node * node;
		
		rgb_pixel color;

		double centerX;
		
		double centerY;
		
		double centerZ;
		
		double borderX;
		
		double borderY;
		
		double borderZ;
		
		double lengthX;
		
		double lengthY;
		
		double lengthZ;
		
		vector<int> points;
		
		vector<int> centers;
		
		bool needSign;
	};
	//===========================================
	
	//===========================================
	class Track
	//===========================================
	{
	public:
		
		Track();
		
		Track(int _start);
		
		Cell& operator[](int frameId);
		
		bool operator<(const Track &) const;
		
		Cell& first();
		
		Cell& last();
		
		int startId() const;
		
		int endId() const;
		
		int size() const;
				
		int realSize() const;
		
		void clear();
		
		void insert(Cell&);
		
	public:
		
		int trackId;
		
		int start;
		
		int realNum;
		
		vector<Cell> cells;
		
		// filter variables
		double meanVolume;
		double meanSpeed;
		double meanDeform;
	};
	//===========================================
	
	class Vertex
	{
	public:
		Vertex();
		Vertex(unsigned short,unsigned short,unsigned short);
		Vertex(int p, int w, int h);
		bool operator<(const Vertex&) const;
		void operator+=(const Vertex&);
	public:
		unsigned short x;
		unsigned short y;
		unsigned short z;
	};
		
};

#endif /*Frame_H_*/

