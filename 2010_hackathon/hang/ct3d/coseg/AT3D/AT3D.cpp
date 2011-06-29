//===========================================
// Hang Xiao, Augest 1, 2010
//===========================================
#include "AT3D.h"
#include "../myalgorithms.h"   //bucketsort
#include <cassert>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
using namespace std;

AT3D::AT3D()
{
	init();
}

AT3D::AT3D(vector<string>& names)
{
	create(names);
}

void AT3D::create(vector<string>& names)
{
	assert(! names.empty());
	init();
	m_filenames = names;                               // m_filenames
	m_numFrames = names.size();                        // m_numFrames
	createTracks(names);                               // m_tracks, m_palette, m_width, m_height, m_depth
	m_numTracks = m_tracks.size();                     // m_numTracks
	m_ptracks.resize(m_numTracks);                    // m_ptracks
	for(int i = 0; i < m_numTracks; i++)
	{
		m_ptracks[i] = &(m_tracks[i]);
	}
	m_states.push(m_ptracks);                         // m_states
	setFrames();     // m_frames
}


void AT3D::init()
{
	clear();
}

void AT3D::clear()
{
	
	if(! m_trees.empty())
	{
		for(int i = 0; i < m_numFrames; i++)
		{
			m_trees[i].clear();
		}
	}
	m_trees.clear();
	
	m_frames.clear();
	m_filenames.clear();
	m_numFrames = 0;
	
	m_tracks.clear();
	m_palette.clear();
	m_numTracks = 0;
	
	m_width = 0;
	m_height = 0;
	m_depth = 0;
	
	//other
	while(! m_states.empty())m_states.pop();
}

bool AT3D::empty()
{
	if(m_numFrames == 0 || m_numTracks == 0) return true;
	return false;
}

bool AT3D::isTreeLoad()
{
	return (! m_trees.empty());
}

// just pop, woundn't re set frames

void AT3D::push_state()
{
	m_states.push(m_ptracks);
}

void AT3D::pop_state()
{
	if(!m_states.empty())m_states.pop();
}
/*******************************************
 * createTracks : m_tracks and m_palette will be set
 *                
 *                will affect m_width, m_height, m_depth
 
 *******************************************/
void AT3D::createTracks(vector<string>& filenames)
{
	int fileCount = filenames.size();
	
	int trackId = -1;
	for(int t = 0; t < fileCount; t++)
	{
		//1. read and check image
		int width = 0;
		int height = 0;
		int depth = 0;
		int channels = 0;
		string filename = filenames[t];
		unsigned char* img = readtiff((char*)filename.c_str(), &width, &height, &depth, &channels);
		if(m_width == 0 ) m_width = width;
		if(m_height == 0) m_height = height;
		if(m_depth == 0) m_depth = depth;
		
		if(m_width != width || m_height != height || m_depth != depth)
		{
			cerr<<"tiff image with different size"<<endl;
			return ;
		}
		int _size = width*height*depth;
		
		if(channels < 3)
		{
			cout<<"not rgb images"<<endl;
			unsigned char* _img = new unsigned char[_size*3];
			
			for(int i = 0; i < _size ; i++)
			{
				int index = 3*i;
				if(channels == 1) 
				{
					_img[index] = img[i];
					_img[index+1] = img[i];
					_img[index+2] = img[i];
				}
				if(channels == 2)
				{
					_img[index] = img[2*i];
					_img[index+1] = img[2*i+1];
					_img[index+2] = (unsigned char)(img[2*i] / 2.0 + img[2*i+1]/ 2.0);
				}
			}
			
			
			delete img;
			img = _img;
			channels = 3;
			return;
		}
		else if(channels == 4)
		{
			unsigned char* _img = new unsigned char[_size*3];
			for(int i = 0; i < _size; i++)
			{
				_img[3*i] = img[4*i];
				_img[3*i+1] = img[4*i+1];
				_img[3*i+2] = img[4*i+2];
			}
			delete img;
			img = _img;
			channels = 3;
		}
		
		//2. find all the cells
		
		for(int p = 0; p < _size; p++)
		{
			rgb_pixel color(img[3*p], img[3*p+1], img[3*p+2]);
			
			if(color != rgb_pixel(0,0,0))
			{
				if(m_palette.find(color) != m_palette.end()) trackId = m_palette[color];
				else trackId = -1;
				//color not exit
				if(m_palette.find(color) == m_palette.end())
				{
					trackId = m_palette.size();
					m_palette[color] = trackId;
					
					Track track;
					track.trackId = trackId;
					track.start = t;
					m_tracks.push_back(track);
					
					Cell cell(NULL,t,color);  //m_tracks.back() is not sure to be the content's real address
					cell.points.push_back(p);
					m_tracks.back().insert(cell);
					//cout<<"<"<<cell.track->trackId<<":"<<&track<<"> ";
				}
				//color exist
				else
				{
					Track& track = m_tracks[trackId];
					assert(track.endId() <= t);
					if(track.endId() < t)
					{
						Cell cell(NULL,t,color);
						cell.points.push_back(p);
						track.insert(cell);
					}
					else
					{
						Cell& cell = track.last();
						cell.points.push_back(p);
					} 
				} //if(trackId == -1)
			} //if(color != (0,0,0)
		}//for all the points
	}// for all the frames;
	
	//sortTracks(m_tracks); //sort Tracks and set cell's track and set cell's centers
	
	sort(m_tracks.begin(),m_tracks.end());
	
	vector<Track>::iterator it = m_tracks.begin();
	
	int i = 0;
	
	while(it != m_tracks.end())
	{
		Track& track = *it;
		track.trackId = i++;
		vector<Cell>::iterator itr = track.cells.begin();
		track.meanVolume = 0.0;
		track.meanSpeed = 0.0;
		track.meanDeform = 0.0;
		while(itr != (*it).cells.end())
		{
			int time = (*itr).time;
			Cell& cell = *itr;
			cell.track = &track;
			cell.setCenters(m_width,m_height);
			track.meanVolume += cell.size();
			
			if(time>track.start)
			{
				double dx = cell.centerX - track[time-1].centerX;
				double dy = cell.centerY - track[time-1].centerY;
				double dz = cell.centerZ - track[time-1].centerZ;
				track.meanSpeed += sqrt(dx*dx + dy*dy +dz*dz);
				track.meanDeform += cell.deformation(track[time-1], m_width, m_height);
			}
			itr++;
		}
		track.meanVolume = track.meanVolume/track.size();
		track.meanSpeed = track.meanSpeed/(track.size()-1);
		track.meanDeform = track.meanDeform/(track.size()-1);
		it++;
	}
	
	return;
}

/********************************************
 * setFrames: set m_frames according to 
 * m_numFrames, m_ptracks
 * the cells in each frame are sorted by their size
 ********************************************/

void AT3D::setFrames()
{
	m_frames.clear();
	m_frames.resize(m_numFrames);
	
	vector<Track*>::iterator it = m_ptracks.begin();
	while(it != m_ptracks.end())
	{
		Track* ptrack = *it;
		vector<Cell>::iterator itr= ptrack->cells.begin();
		while(itr != ptrack->cells.end())
		{
			assert((*itr).time < m_numFrames);
			m_frames[(*itr).time].push_back(&(*itr));
			itr++;
		}
		it++;
	}
	
	
	//sort frames
	for( int i = 0; i < m_numFrames; i++) sortFrame(m_frames[i]);
}

void AT3D::sortFrame(Frame& frame)        // sortCells
{
	vector<int> data;
	int _size = frame.size();
	data.resize(_size);
	
	for(int i = 0; i < _size; i++)
	{
		data[i] = frame[i]->size();
	}
	
	vector<int> orders = bucketSort(data);
	
	Frame sortedFrame;
	sortedFrame.resize(_size);
	
	for(int i = 0; i < _size; i++)
	{
		sortedFrame[i] = frame[orders[i]];
	}
	
	frame = sortedFrame;
}

/**********************************************************
 * sortTracks : sort m_tracks according to each track's 
 * average size. after sort the conntent of each track will change
 * we should re-assign each cell's track pointer. 
 * 
 * this could be done in createTracks
 **********************************************************/

void AT3D::sortTracks(vector<Track>& tracks)
{
	sort(tracks.begin(),tracks.end());
	
	for(int i = 0; i < m_numTracks; i++)
	{
		Track& track = tracks[i];
		track.trackId = i;
		for(int j = 0; j < track.size(); j++)
		{
			Cell& cell = track[j];
			cell.track = &track;
		}
	}
	
}

bool AT3D::setTrees(string treeFile)
{
	ifstream ifs(treeFile.c_str());
	if(ifs.fail())
	{
		cerr<<"\topen "<<treeFile.c_str()<<"failed"<<endl;
		return false;
	}
	int numFrames;
	int numTracks;
	readValue(ifs,numFrames);
	readValue(ifs,numTracks);
	
	if(numFrames != m_numFrames)
	{
		cerr<<"the tree's num is different from images"<<endl;
		return false;
	}
	if(numTracks != m_numTracks)
	{
		cerr<<"the tree file is not consponed to the images files"<<endl;
		return false;
	}
	
	m_trees.resize(m_numFrames);
	for(int i = 0; i < m_numFrames; i++)
	{
		m_trees[i].load(ifs);
		if(m_trees[i].width()!=m_width || m_trees[i].height() != m_height || m_trees[i].depth() != m_depth)
		{
			cerr<<"the tree file is not consponed to the images files"<<endl;
			return false; 
		}
	}
	return true;
}

void AT3D::saveFrames(string dir)
{
	for(int id = 0 ;id < m_numFrames; id++)
	{
		string filename = m_filenames[id];
		int lastslash = filename.find_last_of("/");
		filename = filename.substr(lastslash, filename.size() - lastslash);
		filename = dir + filename;
		Frame& frame = m_frames[id];
		// Step 1: Init Variables
		int channels=3;
		unsigned char red=0, green=0, blue=0;
		int width = m_width;
		int height = m_height;
		int layers = m_depth;
		
		// Step 2: Prepare data 
		unsigned char* image=new unsigned char[width*height*layers*channels];
		/*********************************
		 * I should say , the initialize of images is very very important
		 *********************************/
		for(int i=0;i<width*height*layers*channels;i++)
		{
			image[i]=0;
		}
		if(!frame.empty())
		{
			Frame::iterator it;
			for(it = frame.begin();it != frame.end(); it++)
			{
				Cell* cell = *it;
				vector<int>& points = cell->points;
				red= cell->color.r;
				green = cell->color.g;
				blue = cell->color.b;
				vector<int>::iterator itr;
				for(itr=points.begin();itr!=points.end();itr++)
				{
					int p = *itr;
					image[p*channels]= red;
					image[p*channels+1]= green;
					image[p*channels+2]= blue;
				}
			}
		}
		
		// Step 3: Start writing data
		writetiff((char*)filename.c_str(),image,channels,width,height,layers);
		
		// Step 4: Free Heap Data
		free(image);
	}
}

void AT3D::choose(vector<AT3D::Track*>& ptracks)
{
	m_states.push(m_ptracks);
	m_ptracks.clear();
	m_ptracks = ptracks;
	setFrames();
}

void AT3D::remove(vector<AT3D::Track*>& ptracks)
{
	m_states.push(m_ptracks);
	set<Track*> trackset;
	for(int i = 0; i < (int) m_ptracks.size(); i++)
	{
		trackset.insert(m_ptracks[i]);
	}
	
	for(int i = 0 ; i < (int) ptracks.size(); i++)
	{
		trackset.erase(ptracks[i]);
	}
	
	set<Track*>::iterator it = trackset.begin();
	
	m_ptracks.clear();
	m_ptracks.resize(trackset.size());
	int i = 0;
	while(it != trackset.end())
	{
		m_ptracks[i++] = *it;
		it++;
	}
	setFrames();
}


void AT3D::undo()
{
	if(! m_states.empty()) 
	{
		m_ptracks = m_states.top();
		m_states.pop();
	}
	setFrames();
}

AT3D::Frame& AT3D::frame(int time)
{
	return m_frames[time];
}

AT3D::Track& AT3D::track(int trackId)
{
	return m_tracks[trackId];
}

ComponentTree& AT3D::tree(int time)
{
	return m_trees[time];
}

int AT3D::frameCount()
{
	return m_numFrames;
}

int AT3D::trackCount()
{
	return m_numTracks;
}

int AT3D::width()
{
	return m_width;
}

int AT3D::height()
{
	return m_height;
}

int AT3D::depth()
{
	return m_depth;
}

double AT3D::maxMeanVolume()
{
	vector<double> data;
	vector<Track*>::iterator it = m_ptracks.begin();
	while(it != m_ptracks.end())
	{
		data.push_back((*it)->meanVolume);
		it++;
	}
	return *max_element(data.begin(), data.end());
}

double AT3D::maxMeanSpeed()
{
	vector<double> data;
	vector<Track*>::iterator it = m_ptracks.begin();
	while(it != m_ptracks.end())
	{
		data.push_back((*it)->meanSpeed);
		it++;
	}
	return *max_element(data.begin(), data.end());
	
}

double AT3D::maxMeanDeform()
{
	vector<double> data;
	vector<Track*>::iterator it = m_ptracks.begin();
	while(it != m_ptracks.end())
	{
		data.push_back((*it)->meanDeform);
		it++;
	}
	return *max_element(data.begin(), data.end());
	
}


void AT3D::filtering(int time, const vector< pair<double, double> >& filters)
{
	assert(filters.size() == 5);
	// 1. push the previous results
	//m_states.push(m_ptracks);   //push and pop will be done out side
	m_ptracks = m_states.top();
	setFrames();
	
	// 2. filter by current volume
	set<Track*> trackset;
	for(int i = 0; i < (int) m_ptracks.size(); i++)
	{
		trackset.insert(m_ptracks[i]);
	}
	
	for(int cellId = 0 ; cellId <(int)  m_frames[time].size(); cellId++)
	{
		Cell* cell = m_frames[time][cellId];
		if(cell->size() < filters[CURRENTVOLUME].first ||
		   cell->size() > filters[CURRENTVOLUME].second )
		trackset.erase(cell->track);
	}
	
	m_ptracks.clear();	
	m_ptracks.insert(m_ptracks.begin(),trackset.begin(),trackset.end());
	
	vector<Track*>::iterator it = m_ptracks.begin();
	vector<Track*> ptracks;
	while(it != m_ptracks.end())
	{
		if((*it)->meanVolume >= filters[VOLUME].first &&
		   (*it)->meanVolume <= filters[VOLUME].second &&
		   (*it)->size() >= filters[LIFE].first &&
		   (*it)->size() <= filters[LIFE].second &&
		   (*it)->meanSpeed >= filters[SPEED].first &&
		   (*it)->meanSpeed <= filters[SPEED].second &&
		   (*it)->meanDeform >= filters[DEFORM].first &&
		   (*it)->meanDeform <= filters[DEFORM].second)
		{
			ptracks.push_back(*it);
		}
		it++;
	}
	m_ptracks.clear();
	m_ptracks = ptracks;
	
	setFrames();
}

// =====================================================
AT3D::rgb_pixel::rgb_pixel(unsigned char r_, unsigned char g_, unsigned char b_)
// =====================================================
{
	r=r_; g=g_; b=b_; 
}

// =====================================================
AT3D::rgb_pixel::rgb_pixel(unsigned int rgb)
// =====================================================
{
	r=rgb%256;
	rgb/=256;
	g=rgb%256;
	rgb/=256;
	b=rgb%256;
}

// =====================================================
AT3D::rgb_pixel::rgb_pixel()
// =====================================================
{
}

// =====================================================
bool AT3D::rgb_pixel::operator==(const rgb_pixel& P)
// =====================================================
{
	return (r==P.r && g==P.g && b==P.b);
}

// =====================================================
bool AT3D::rgb_pixel::operator<(const rgb_pixel& P) const
// =====================================================
{
	// return < according to lexicographical order by (r,g,b)
	if (r<P.r)
		return true;
	if (r>P.r)
		return false;
	// r==P.r
	if (g<P.g)
		return true;
	if (g>P.g)
		return false;
	// r==P.r && g==P.g
	if (b<P.b)
		return true;
	return false;
}

// =====================================================
bool AT3D::rgb_pixel::operator!=(const rgb_pixel& P)
// =====================================================
{
	return (r!=P.r || g!=P.g || b!=P.b);
}

//===========================================
AT3D::Cell::Cell()
//===========================================
{
	     track = NULL;
	   time = -1;
	     color = rgb_pixel(0,0,0);
	needSign = false;
}
//===========================================

//===========================================
AT3D::Cell::Cell(Track* _track, int _t, rgb_pixel _color)
//===========================================
{
	track = _track;
	time = _t;
	color = _color;
	needSign = false;
}
//===========================================


int AT3D::Cell::size() const
{
	return points.size();
}

/*******************************************************
 * setCenters : will set centerX, centerY, centerZ
 *              borderX, borderY, borderZ
 *              lengthX, lengthY, lengthZ
 *******************************************************/

void AT3D::Cell::setCenters(int width, int height)
{
	//1. get mean point
	//double centerX,centerY,centerZ;
	int x, y, z;
	//double lengthX, lengthY, lengthZ;
	//double borderX, borderY, borderZ;
	double maxX, maxY, maxZ;
	vector<int>::iterator it = points.begin();
	if(it != points.end())
	{
		int p = *it;
		z = p/(width*height); p = p % (width*height);
		y = p/width; 
		x = p%width;
		centerX = borderX = maxX = x;
		centerY = borderY = maxY = y;
		centerZ = borderZ = maxZ = z;
		it++;
	}
	while(it != points.end())
	{
		int p = *it;
		z = p/(width*height); p = p % (width*height);
		y = p/width; 
		x = p%width;
		centerX += x;
		centerY += y;
		centerZ += z;
		borderX = min(borderX,(double)x);
		borderY = min(borderY,(double)y);
		borderZ = min(borderZ,(double)z);
		maxX = max(maxX,(double)x);
		maxY = max(maxY,(double)y);
		maxZ = max(maxZ,(double)z);
		it++;
	}
	
	centerX = centerX/points.size();
	centerY = centerY/points.size();
	centerZ = centerZ/points.size();
	
	lengthX = maxX - borderX + 0.5;
	lengthY = maxY - borderY + 0.5;
	lengthZ = maxZ - borderZ + 0.5;
	double ratio = 0.5;
	double rx = (lengthX*ratio+1)/2.0;
	double ry = (lengthY*ratio+1)/2.0;
	double rz = (lengthZ*ratio+1)/2.0;
	
	for(float i = -rx;i <= rx; i++)
	{
		for(float j = -ry; j <= ry; j++)
		{
			for(float k = -rz; k <= rz; k++)
			{
				if ((i*i)/(rx*rx)+(j*j)/(ry*ry)+(k*k)/(rz*rz)<=1.0)
				{
					int x = (int)(i + centerX);
					int y = (int)(j + centerY);
					int z = (int)(k + centerZ);
					int position = (int)(z*width*height+y*width+x);
					centers.push_back(position);
				}
			}
		}
	}
}

double AT3D::Cell::deformation(Cell& prev, int width, int height)
{
	set<Vertex> vertices;
	
	//Vertex this_center((unsigned short)(centerX+0.5),(unsigned short)(centerY+0.5),(unsigned short)(centerZ+0.5));
	//Vertex prev_center((unsigned short)(prev.centerX+0.5),(unsigned short)(prev.centerY+0.5),(unsigned short)(prev.centerZ+0.5));
	Cell * large;
	Cell * small;
	if(points.size() > prev.points.size()) 
	{
		large = this;
		small = &prev;
	}
	else
	{
		large = &prev;
		small = this;
	}

	vector<int>::iterator it = small->points.begin();

	while(it != (small->points).end())
	{
		Vertex vertex(*it,width,height);
		//vertex += this_center;
		vertices.insert(vertex);
		it++;
	}
	int overlap = 0;
	it = (large->points).begin();
	while(it != (large->points).end())
	{
		Vertex vertex(*it,width,height);
		//vertex += prev_center;
		if(vertices.find(vertex) != vertices.end()) overlap++;
		it++;
	}
	//double overlap = points.size() + prev.points.size() - vertices.size();
	return (double)overlap/prev.points.size();
}

//===========================================
AT3D::Track::Track()
//===========================================
{
	trackId = -1;
	start = -1;
	realNum= 0;
}
//===========================================

//===========================================
AT3D::Track::Track(int _start)
//===========================================
{
	trackId = -1;
	start = _start;
	realNum = 0;
}
//===========================================

//===========================================
AT3D::Cell& AT3D::Track::operator[](int t)
//===========================================
{
	assert(t >= start);
	assert(t < start+(int)cells.size());
	return cells[t - start];
}
//===========================================

bool AT3D::Track::operator<(const AT3D::Track& track) const
{
	double avg_size1 = 0.0, avg_size2 = 0.0;
	vector<Cell>::const_iterator it = this->cells.begin();
	while(it != this->cells.end())
	{
		avg_size1 += (*it).size();
		it++;
	}
	avg_size1 = avg_size1/this->size();
	
	it = track.cells.begin();
	while(it != track.cells.end())
	{
		avg_size2 += (*it).size();
		it++;
	}
	avg_size2 = avg_size2/track.size();
	return (avg_size1 > avg_size2);
}

//===========================================
AT3D::Cell& AT3D::Track::first()
//===========================================
{
	return *(cells.begin());
}
//===========================================

//===========================================
AT3D::Cell& AT3D::Track::last()
//===========================================
{
	return *(cells.rbegin());
}
//===========================================

//===========================================
int AT3D::Track::startId() const
//===========================================
{
	return start;
}

//===========================================
int AT3D::Track::endId() const
//===========================================
{
	return start + cells.size() - 1;
}

//===========================================
int AT3D::Track::size() const
//===========================================
{
	return cells.size();
}

//===========================================
int AT3D::Track::realSize() const
//===========================================
{
	return realNum;
}

//===========================================
void AT3D::Track::clear()
//===========================================
{
	trackId = -1;
	start = -1;
	cells.clear();
}
//===========================================


//===========================================
void AT3D::Track::insert(Cell& cell)
//===========================================
{
	assert(cell.time > endId());
	if(cell.time == endId() + 1)
	{
		cells.push_back(cell);
		realNum++;
	}
	else
	{
		for(int i = endId()+1; i < cell.time; i++)
		{
			Cell _cell(this,i,rgb_pixel(0,0,0));
			cells.push_back(_cell);
		}
		cells.push_back(cell);
		realNum++;
	}
	
}
//===========================================

AT3D::Vertex::Vertex()
{
	x = 0;
	y = 0;
	z = 0;
}

AT3D::Vertex::Vertex(unsigned short _x, unsigned short _y, unsigned short _z)
{
	x = _x;
	y = _y;
	z = _z;
}

AT3D::Vertex::Vertex(int p, int width, int height)
{
	z = p /(width*height);
	y = (p % (width*height))/width;
	x = p % width;
}

bool AT3D::Vertex::operator<(const Vertex& v) const
{
	if(x < v.x) return true;
	if(x > v.x) return false;
	if(y < v.y) return true;
	if(y > v.y) return false;
	if(z < v.z) return true;
	if(z > v.z) return false;
	return false;
}

void AT3D::Vertex::operator+=(const Vertex& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}



