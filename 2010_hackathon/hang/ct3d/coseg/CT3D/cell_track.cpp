//#include <io.h> //check file existence
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <deque>
#include <string>

#include <unistd.h>      //access
#include <dirent.h>
#include <time.h>
#include <math.h>
#include <sstream> //istringstream
#include <assert.h>
#include "lp_lib.h"
#include "cell_track.h"
#include "../component_tree.h"
#include "../myalgorithms.h"

using namespace std;

int MAXCELLNUM = 1000;

bool verbose = 0;

time_t start, end;

static void create_match(double * row, int num1, int num2, map<int,int>& match);
static void create_frames(map<int,int>&match, CellTrack::Frame &f1, CellTrack::Frame &f2);
static CellTrack::Frame merge_frames(ComponentTree &tree, double* row, CellTrack::Frame &f1, CellTrack::Frame &f2);

static void timeElapsed(time_t &start, time_t &end)
{
	double dif;
	dif = difftime (end,start);
	int hours = (int)(dif/3600.0);
	int minutes = (int)((dif - hours*3600.0)/60.0);
	int seconds = (int)(dif - hours*3600.0 - minutes*60.0);
	cout<<"time needed : "<<hours<<" hours, "<<minutes<<" minutes, "<<seconds<<" seconds"<<endl;
	return;
}

static bool is_suffix(string str, string suffix)
{
	size_t pos = str.find(suffix);
	if(pos !=(unsigned int) -1 && pos + suffix.size() == str.size()) return true;
	return false;
}

// =====================================================
CellTrack::rgb_pixel::rgb_pixel(unsigned char r_, unsigned char g_, unsigned char b_)
// =====================================================
{
	r=r_; g=g_; b=b_; 
}

// =====================================================
CellTrack::rgb_pixel::rgb_pixel(unsigned int rgb)
// =====================================================
{
	r=rgb%256;
	rgb/=256;
	g=rgb%256;
	rgb/=256;
	b=rgb%256;
}

// =====================================================
CellTrack::rgb_pixel::rgb_pixel()
// =====================================================
{
}

// =====================================================
bool CellTrack::rgb_pixel::operator==(const rgb_pixel& P)
// =====================================================
{
	return (r==P.r && g==P.g && b==P.b);
}

// =====================================================
bool CellTrack::rgb_pixel::operator<(const rgb_pixel& P) const
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
bool CellTrack::rgb_pixel::operator!=(const rgb_pixel& P)
// =====================================================
{
	return (r!=P.r || g!=P.g || b!=P.b);
}

// =====================================================
CellTrack::palette::int_set& CellTrack::palette::get_all_colors()
// =====================================================
{
	return all_colors;
}

// =====================================================
void CellTrack::palette::init(int size)
// =====================================================
{
	all_colors.clear();
	palette_size = size;
	this_palette.resize(palette_size);
	
	rgb_pixel black = rgb_pixel(0,0,0);
	this_palette[0] = black;
	reverse_palette[black] = 0;
	
	int i;
	for (i=1; i<palette_size; ++i)
	{
		all_colors.insert(i);
		// start from 1 to avoid black as a palette color
		int j=i+1;
		unsigned char r=0;
		unsigned char g=0;
		unsigned char b=0;
		int bit=sizeof(unsigned char)*CHAR_BIT-1;
		for (; j>0; bit--)
		{
			// lowest bit should be unused
			assert(bit>0);
			unsigned char add=1<<bit;
			if (j%2)
				r+=add;
			j/=2;
			if (j%2)
				g+=add;
			j/=2;
			if (j%2)
				b+=add;
			j/=2;
		}
		
		if (r==0)
			r=1;
		rgb_pixel col_i=rgb_pixel(r,g,b);
		this_palette[i]=col_i;
		//std::cout<<"palette["<<i<<"]=("<<(int)col_i.r<<","<<(int)col_i.g<<","<<(int)col_i.b<<")\n";
		reverse_palette[col_i]=i;
		//std::cerr<<"> "<<reverse_palette[col_i]<<"\n";
	}
}

// =====================================================
CellTrack::palette::palette()
// =====================================================
{
	init(256);
}

// =====================================================
CellTrack::palette::palette(int size)
// =====================================================
{
	init(size);
}

// =====================================================
int CellTrack::palette::operator()(const rgb_pixel& p)
// =====================================================
{
	if (reverse_palette.find(p)==reverse_palette.end())
		return -1;
	return reverse_palette[p];
}

// =====================================================
CellTrack::rgb_pixel CellTrack::palette::operator()(int i)
// =====================================================
{
	assert(i<size());
	return this_palette[i];
}

// =====================================================
int CellTrack::palette::size()
// =====================================================
{
	return palette_size;
}

CellTrack::Alignment::Alignment()
{
	m_numVars1 = 0;
	m_numVars2 = 0;
	m_row = NULL;
}

CellTrack::Alignment::~Alignment()
{
	clear();
}

void CellTrack::Alignment::clear()
{
	for(int i=0; i < m_numVars1; i++) m_weightMatrix.clear();
	m_numVars1 = 0;
	m_numVars2 = 0;
	if(m_row)
	{
		delete m_row;
		m_row = NULL;
	}
}

bool CellTrack::Alignment::align(ComponentTree& tree1, ComponentTree& tree2)
{
	/*
	 * 1. check the valiad of the tree
	 */
	if(tree1.width() != tree2.width() || tree1.height() != tree2.height() || tree1.depth() != tree2.depth())
	{
		cerr<<"The two trees with different size. Unalbe to align."<<endl;
		return false;
	}
	/*
	 * 2. clear the original data and getParams member variable
	 */
	clear();
	m_numVars1 = tree1.nodeCount();
	m_numVars2 = tree2.nodeCount();
	m_weightMatrix.resize(m_numVars1);
	for(int i = 0 ; i < m_numVars1; i++)
	{
		m_weightMatrix[i].resize(m_numVars2);
		for(int j=0; j < m_numVars2; j++)
			m_weightMatrix[i][j] = 0.0;
	}
	/*
	 * 3. set weight matrix
	 */
	int numVertices = tree1.pixelCount();
	int* matrix1 = tree1.getMappingMatrix();
	int* matrix2 = tree2.getMappingMatrix();
	// 3.1 get the overlap of each subset
	for(int v = 0; v < numVertices ; v++)
	{
		int label1 = matrix1[v];
		int label2 = matrix2[v];
		if(1)
		{
			if(label1 != m_numVars1-1 && label2 != m_numVars2-1 )m_weightMatrix[label1][label2]++;
		}
		else
		{
			m_weightMatrix[label1][label2]++;
		}
	}
	// 3.2 get the overlp of tree1's sub node with any of tree2's node 
	//     include the sub node and the merged component
	ComponentTree::Nodes::iterator it;
	for(int i = 0; i < m_numVars1- 1; i++)
	{
		for(int j = 0; j < m_numVars2- 1; j++)
		{
			ComponentTree::Node* node2 = tree2.getNode(j);
			if(! node2->childs.empty())
			{
				it = node2->childs.begin();
				//cout<<"node2 size = "<<node2->childs.size()<<endl;
				while( it != node2->childs.end())
				{
					int jj = (*it)->label;
					m_weightMatrix[i][j] += m_weightMatrix[i][jj];
					it++;
				}
			}
		}
	}
	
	// 3.3 merge tree1's match result for node with childs
	for(int i = 0; i < m_numVars1 - 1; i++)
	{
		for(int j = 0; j < m_numVars2 - 1; j++)
		{
			ComponentTree::Node* node1 = tree1.getNode(i);
			if(! node1->childs.empty())
			{
				it = node1->childs.begin();
				while( it != node1->childs.end())
				{
					int ii = (*it)->label;
					m_weightMatrix[i][j] += m_weightMatrix[ii][j];
					it++;
				}
			}
		}
	}
	// 3.4 get the 0 ~ 1 overlap value
	float intersection = 0;
	float joint = 1;
	for(int i = 0; i < m_numVars1; i++)
	{
		for(int j = 0; j < m_numVars2; j++)
		{
			intersection = m_weightMatrix[i][j];
			joint = tree1.getNode(i)->alpha_size + tree2.getNode(j)->alpha_size - intersection;
			assert(joint > 0.0001);
			m_weightMatrix[i][j] = (float)intersection/joint;
		}
	}
	
	if(verbose)
	{
		cout<<"weight matrix : "<<endl;
		cout.precision(3);
		cout<<" t1\\t2  ";
		for(int j = 0; j < m_numVars2; j++) cout<<setw(7)<<j;
		cout<<endl;
		for(int i = 0; i < m_numVars1; i++)
		{
			cout<<i<<" : ";
			for(int j = 0; j < m_numVars2; j++)
			{
				cout<<setw(7)<<m_weightMatrix[i][j]<<"  ";
			}
			cout<<endl;
		}
	}
	// 3.5 free matrix
	delete matrix1;
	delete matrix2;
	
	/*
	 * 4. build the linear model
	 */
	
	ComponentTree::Paths ps1 = tree1.getPaths();
	ComponentTree::Paths ps2 = tree2.getPaths();
	
	// 4.1 getParams linear variable
	lprec *lp;
	int Ncol, *colno=NULL, k;
	REAL * row = NULL;
	int i=0,j=0;
	Ncol = m_numVars1 * m_numVars2;
	lp = make_lp(0,Ncol);
	if(lp == NULL) return false;
	
	colno = (int *) malloc(Ncol * sizeof(*colno));
	row = (REAL *) malloc(Ncol * sizeof(*row));
	if((colno == NULL) || (row == NULL)) return false;
	for(i=0;i< Ncol;i++) row[i]=1.0; // assign all the content of row as 1
	
	set_add_rowmode(lp,TRUE); 
	set_binary(lp,Ncol,TRUE);
	
	
	// 4.2 add ps1 constraints
	//one path one constraint
	
	ComponentTree::Paths::iterator it2=ps1.begin();
	while(it2 != ps1.end())
	{
		k=0; 
		ComponentTree::Path::iterator itr = (*it2).begin();
		while(itr != (*it2).end())
		{
			i = (*itr);
			for(j=0;j<m_numVars2;j++)colno[k++] = i*m_numVars2+j+1; 
			itr++;
		}
		if(!add_constraintex(lp, k, row, colno, LE, 1))
			return false;
		it2++;
	}
	
	// 4.3 : add ps2 constraints
	it2=ps2.begin();
	while (it2 != ps2.end())
	{
		ComponentTree::Path::iterator itr = (*it2).begin();
		k=0; 
		while(itr != (*it2).end())
		{
			j = (*itr);
			for(i=0;i<m_numVars1;i++)colno[k++] = i*m_numVars2+j+1;
			itr++;
		}
		if(!add_constraintex(lp, k, row, colno, LE, 1))
			return false;
		it2++;
	}
	set_add_rowmode(lp,FALSE); 
	
	// 4.4 add the object
	k=0;
	for(i=0;i< m_numVars1; i++)
		for(j=0; j< m_numVars2; j++)
		{
			colno[k] = i*m_numVars2+j+1;
			row[k++] = m_weightMatrix[i][j];
		}
	if(!set_obj_fnex(lp, k, row, colno))return false;
	set_maxim(lp);
	set_verbose(lp,IMPORTANT);
	
	// 4.5 solve the linear problem
	if(::solve(lp) != OPTIMAL)
	{ 
		cout<<"Not optimized results"<<endl;
		return false;
	}
	// 4.6 save results to m_row
	// print out the match pairs
	get_variables(lp,row);
	m_row = row;
	
	if(verbose)
	{
		cout<<"mapping : ";
		for(int i = 0; i < m_numVars1; i++)
		{
			for(int j = 0; j < m_numVars2; j++)
			{
				if(fabs(row[i*m_numVars2+j] - 1.0)< 0.001)
				{
					cout<<i<<"<->"<<j<<"  ";
					break; // one i only match with one j
				}
			}
		}
		cout<<endl;
	}
	// 4.7 free heap space
	//if(row != NULL)
	//	free(row);
	if(colno != NULL)
		free(colno);
	
	if(lp != NULL) {
		delete_lp(lp);
	}
	
	return true;
}

bool CellTrack::Alignment::align(ComponentTree &tree, Frame& f1, Frame & f2)
{
	/*
	 * 1. getParams variables
	 */
	clear();
	m_numVars1 = f1.size();
	m_numVars2 = f2.size();
	m_weightMatrix.resize(m_numVars1);
	for(int i = 0 ; i < m_numVars1; i++)
	{
		m_weightMatrix[i].resize(m_numVars2);
		for(int j=0; j < m_numVars2; j++)
			m_weightMatrix[i][j] = 0.0;
	}
	
	vector<int> labels1;
	vector<int> labels2;
	vector<int> values1;
	vector<int> values2;
	int value = 0;
	//1.1 get labels1, labels2 and values1 , values2
	Frame::iterator it = f1.begin();
	while(it != f1.end())
	{
		labels1.push_back((*it)->label);
		values1.push_back(value++);
		it++;
	}
	value = 0;
	it = f2.begin();
	while(it != f2.end())
	{
		labels2.push_back((*it)->label);
		values2.push_back(value++);
		it++;
	}
	
	/*
	 * 2. set weight matrix
	 */
	//the matrix's value is the label's index in vector
	//which is more convenient than store label value
	int* matrix1 = tree.getMatrix(labels1,values1, -1);
	int* matrix2 = tree.getMatrix(labels2,values2, -1);
	int numVertices = tree.pixelCount();
	
	// 2.1 set the num of overlap points
	for(int v = 0; v < numVertices ; v++)
	{
		//find the nodes in each component tree
		int label1 = matrix1[v];
		int label2 = matrix2[v];
		if(label1 == -1 || label2 == -1) continue;
		m_weightMatrix[label1][label2]++;
	}
	
	// 2.2 set the overlap value
	float intersection = 0;
	float joint = 1;
	for(int i = 0; i < m_numVars1; i++)
	{
		for(int j = 0; j < m_numVars2; j++)
		{
			intersection = m_weightMatrix[i][j];
			joint = tree.getNode(labels1[i])->alpha_size + tree.getNode(labels2[j])->alpha_size - intersection;
			assert(joint > 0.0001);
			m_weightMatrix[i][j] = (float)intersection/joint;
		}
	}
	
	// 2.3 free space
	delete matrix1;
	delete matrix2;
	
	/*
	 * 3. build the linear model
	 */
	
	// 3.1 getParams variable
	lprec *lp;
	int Ncol, *colno=NULL, k;
	REAL * row = NULL;
	int i=0,j=0;
	Ncol = m_numVars1 * m_numVars2;
	lp = make_lp(0,Ncol);
	if(lp == NULL) return false;
	
	colno = (int *) malloc(Ncol * sizeof(*colno));
	row = (REAL *) malloc(Ncol * sizeof(*row));
	if((colno == NULL) || (row == NULL)) return false;
	for(i=0;i< Ncol;i++) row[i]=1.0; // assign all the content of row as 1
	
	set_add_rowmode(lp,TRUE); 
	set_binary(lp,Ncol,TRUE);
	// 3.2 the sum of each row is less than 1
	
	for(int i = 0; i < m_numVars1; i++)
	{
		k=0;
		for(j=0;j<m_numVars2;j++)colno[k++] = i*m_numVars2+j+1; 
		//one path on constraint
		if(!add_constraintex(lp, k, row, colno, LE, 1))
			return false;
	}
	// 3.3 the sum of each column is less than 1
	for(int j = 0; j < m_numVars2; j++)
	{
		k=0;
		for(i=0;i<m_numVars1;i++)colno[k++] = i*m_numVars2+j+1;
		if(!add_constraintex(lp, k, row, colno, LE, 1))
			return false;
	}
	set_add_rowmode(lp,FALSE); //I don't know why, just write it
	
	// 3.4 set the object
	
	k=0;
	for(i=0;i< m_numVars1; i++)
	{
		for(j=0; j< m_numVars2; j++)
		{
			colno[k] = i*m_numVars2+j+1;
			row[k++] = m_weightMatrix[i][j];
		}
	}
	if(!set_obj_fnex(lp, k, row, colno))return false;
	set_maxim(lp);
	set_verbose(lp,IMPORTANT);
	
	// 3.5 solve the problem
	if(::solve(lp) != OPTIMAL)
	{ 
		cout<<"Not optimized results"<<endl;
		return false;
	}
	
	// 3.6 save result to m_row
	get_variables(lp,row);
	m_row = row;
	// 3.7 print out matched result
	// 3.8 free space
	
	//if(row != NULL)
	//	free(row);
	if(colno != NULL)
	{
		free(colno);
		colno = NULL;
	}
	
	if(lp != NULL) {
		delete_lp(lp);
	}
	return true;
}

double* CellTrack::Alignment::result()
{
	return m_row;
}

CellTrack::CellTrack()
{
	m_numTracks = 0;
	m_numFrames = 0;
	m_threshold = -1.0;
	m_palette.init(MAXCELLNUM);
}

CellTrack::~CellTrack()
{
	clear();
}

//free the content of containers, m_frames, m_tracks
void CellTrack::clear()
{
	if(m_numFrames == 0 && m_numTracks == 0) return;
	cout<<"clearing data ... "<<endl;
	// step 1: free trees
	if(! m_trees.empty())
	{
		for(int i = 0; i <(int) m_trees.size(); i++)
			m_trees[i].clear();
	}
	m_frames.clear();
	m_names.clear();
	m_matches.clear();
	m_numFrames = 0;

	if(! m_tracks.empty())
	{
		vector<Track>::iterator it = m_tracks.begin();
		while(it != m_tracks.end())
		{
			Cell* p = (*it).entry;
			assert(p!=NULL);
			Cell* next = p;
			while(p->next)
			{
				next = p->next;
				delete p;
				p = next;
			}
			delete p;
			it++;
		}
		m_tracks.clear();
		m_numTracks = 0;
	}
	m_threshold = -1.0;
	//m_outdir = "out";

}


/********************************************
 * createTracking will do two things
 * 1. create trees
 * 2. tracking
 ********************************************/

void CellTrack::createTracking(int argc, char* argv[])
{
	time(&start);
	/*
	 * 1. set parameters
	 */
	int _minFilter = DEFAULT_MIN_FILTER;//0;
	int _maxFilter = DEFAULT_MAX_FILTER;//2147483647;
	int _singleFilter = DEFAULT_SINGLE_FILTER;//2147483647;  //or 1
	string _indir = DEFAULT_INPUTDIR;
	m_outdir = DEFAULT_OUTPUTDIR;
	
	bool mFlag = 0;
	bool MFlag = 0;
	bool sFlag = 0;
	bool oFlag = 0;
	bool iFlag = 0;
	
	int i = 0;
	while(i < argc)
	{
		if(strcmp(argv[i], "-m") == 0 && mFlag == 0)
		{
			assert(argc >= i+2);
			_minFilter = atoi(argv[++i]);
			mFlag = 1;
		}
		else if(strcmp(argv[i], "-M") == 0 && MFlag == 0)
		{
			assert(argc >= i+2);
			_maxFilter = atoi(argv[++i]);
			MFlag = 1;
		}
		else if(strcmp(argv[i], "-s") == 0 && sFlag == 0)
		{
			assert(argc >= i+2);
			_singleFilter = atoi(argv[++i]);
			sFlag = 1;
		}
		else if(strcmp(argv[i], "-i") == 0 && iFlag == 0)
		{
			assert(argc >= i+2);
			_indir = argv[++i];
			iFlag = 1;
			
			if(opendir(_indir.c_str()) == NULL)
			{
				cerr<<"Error : no input folder ("<<_indir.c_str()<<")"<<endl;
				return;
			}
		}
		else if(strcmp(argv[i], "-o") == 0 && oFlag == 0)
		{
			assert(argc >= i+2);
			m_outdir = argv[++i];
			oFlag = 1;
			if((access(m_outdir.c_str(),0)) == -1)
			{
				system((string("mkdir ")+m_outdir).c_str());
			}
			
		}
		else if(strcmp(argv[i],"-v") == 0)
		{
			verbose = 1;
		}
		else
		{
			cerr<<"Error : unknow parameter : "<<argv[i]<<endl;
			return;
		}
		i++;
	}
	
	cout<<"\t create -i "<<_indir.c_str()<<" -o "<<m_outdir.c_str()
	<<" -m "<<_minFilter<<" -M "<<_maxFilter<<" -s "
	<<_singleFilter;
	
	if(verbose) cout<<" -v ";
	cout<<endl;
	clear();
	/*
	 * 2. get names
	 */

	DIR* dir;
	struct dirent *DirEntry;
	dir = opendir(_indir.c_str());
	
	cout<<endl<<"reading images ... "<<endl;
	while((DirEntry=readdir(dir)))
	{
		if(DirEntry->d_name[0] == '.') continue;
		if(is_suffix(DirEntry->d_name,".tiff") || is_suffix(DirEntry->d_name,".tif"))
		{
			cout<<"\t"<<DirEntry->d_name<<endl;
			m_names.push_back(DirEntry->d_name);
		}
	}
	
	m_numFrames = m_names.size();
	
	if(m_numFrames == 0)
	{
		cerr<<"Error : no file input"<<endl;
		return;
	}
	
	m_trees.resize(m_numFrames);
	
	cout<<"\ncreating trees ..."<<endl;
	for(int i = 0; i < m_numFrames; i++)
	{
		cout<<"\ttree "<<(i+1)<<endl;
		m_trees[i].create((char*)(_indir + "/" + m_names[i]).c_str(),_minFilter, _maxFilter, _singleFilter);
	}
	tracking();
	return;
}



bool CellTrack::tracking()
{	
	assert(m_numFrames >= 2);
	/*
	 * 0. reset containers
	 */
	cout<<endl<<"start tracking ..."<<endl;
	m_frames.clear();
	m_frames.resize(m_numFrames);
	if(! m_tracks.empty())
	{
		vector<Track>::iterator it = m_tracks.begin();
		while(it != m_tracks.end())
		{
			Cell* p = (*it).entry;
			assert(p!=NULL);
			Cell* next = p;
			while(p->next)
			{
				next = p->next;
				delete p;
				p = next;
			}
			delete p;
			it++;
		}
		m_tracks.clear();
		m_numTracks = 0;
	}
	/*
	 * 1. local variables and member vaiable getParams
	 */
	vector<Frame> t_frames;
	Alignment alg;
	t_frames.resize(2*m_numFrames - 2);
	double * row = NULL; // the delete operation of row is done by alg

	/*
	 * 2. the first round of comparison
	 */
	
	// 2.1 load the first tree

	int i = 0; // the entry of t_frames
	int t = 1; // the entry of time
	// 2.2 go to the loop of m_numFrames - 1 times comparison
	//     and m_numFrames -1 matches will bed set
	
	m_matches.resize(m_numFrames - 1);
	cout<<"\tThe first round alignment "<<endl;
	for(int t = 1; t < m_numFrames; t++)
	{
		cout<<"\talign tree"<<t<<" and tree"<<(t+1)<<endl;
		
		alg.align(m_trees[t-1], m_trees[t]);
		
		row = alg.result();

		create_match(row, m_trees[t-1].nodeCount(), m_trees[t].nodeCount(), m_matches[t-1]);

		create_frames(m_matches[t-1],t_frames[i],t_frames[i+1]);
				
		alg.clear();
		
		i = i + 2;
	}
	
	/*
	 * 3. the second round of alignment to merge nodes from tim 1 ~ m_numFrames -2
	 */
	cout<<endl<<"\tThe second round alignment "<<endl;
	i = 0;
	t = 0;
	m_frames[t++] = t_frames[i++];
	
	while( t <= m_numFrames - 2)
	{
		cout<<"\talign tree"<<(t+1)<<endl;
		
		alg.align(m_trees[t],t_frames[i], t_frames[i+1]);
		
		row = alg.result();
		
		m_frames[t++] = merge_frames(m_trees[t], row , t_frames[i], t_frames[i+1]);
		
		alg.clear();
				
		i = i+2;
	}
	// now t = m_numFrames - 1,  i+1 = 2*m_numFrames -2
	m_frames[t] = t_frames[i];
	
	t_frames.clear();
	
	/*
	 * 4. set tracks and trackId for each cell
	 */
	t = 0;
	int trackId = 1;         //the cell id starts from 1, the cell id = 0 is the background
	while(t < m_numFrames)
	{
		Frame& f = m_frames[t];
		Frame::iterator it = f.begin();
		while(it != f.end())
		{
			if((*it)->prev == NULL)
			{
				Track tk;
				tk.trackId = trackId;
				tk.start = t;
				tk.end = t;           // to be set later
				tk.altitude = 0.00;
				tk.entry = (*it);
				//set the cell's cell id
				Cell* p = *it;
				while(p)
				{
					ComponentTree::Node* node = m_trees[(tk.end)++].getNode(p->label);
					tk.altitude += node->mean - node->level;
					p->trackId = trackId;
					p = p->next;
				}
				tk.altitude = tk.altitude/(tk.end - tk.start);
				m_tracks.push_back(tk);
				trackId++;
			}
			it++;
		}
		t++;
	}
	m_numTracks = m_tracks.size();
	cout<<"number of tracks : "<<m_numTracks<<endl;
	//out put tracks
	for(int i=0; i< m_numTracks; i++)
	{
		Cell* entry = m_tracks[i].entry;
		Cell* p = entry;
		while(p)
		{
			p=p->next;
		}
	}
	cout<<endl;
	/*
	 * 5. save results according to the trackId and labels 
	 */
	time(&end);
	timeElapsed(start,end);
	
	saveImages();
	saveGraph();
	saveGraph2();
	saveTracking();
	
	return true;
}

/********************************************************************************
 * saveTracking : save the results of tracking
 ********************************************************************************/
bool CellTrack::saveTracking(char* trackfile)
{
	string result_name = m_outdir + "/" + trackfile;
	cout<<"tracking results are saved to "<<result_name.c_str()<<endl;
	ofstream ofs(result_name.c_str());
	if(ofs.fail())
	{
		cerr<<"\topen "<<result_name.c_str()<<endl;
		return false;
	}
	/*
	 * 1. save m_numFrames, m_numTracks
	 */
	writeValue(ofs,m_numFrames);
	writeValue(ofs,m_numTracks);
	/*
	 * 2. save trees
	 */
	for(int i = 0; i < m_numFrames; i++)
	{
		m_trees[i].save(ofs);
	}
	
	/*
	 * 4. save m_tracks
	 */
	for(int i = 0; i < m_numTracks; i++)
	{
		writeValue(ofs,m_tracks[i].trackId);
		writeValue(ofs,m_tracks[i].altitude);
		writeValue(ofs,m_tracks[i].start);
		writeValue(ofs,m_tracks[i].end);
		
		Cell* entry = m_tracks[i].entry;
		Cell* p = entry;
		for(int j = m_tracks[i].start; j < m_tracks[i].end; j++)
		{
			assert(p != NULL);
			writeValue(ofs, p->label);
			writeValue(ofs, p->prevLabel);
			writeValue(ofs, p->nextLabel);
			//writeValue(ofs, p->trackId);
			p = p->next;
		}
	}
	/*
	 * 3. save m_matches 
	 */
	
	 for(int i = 0 ; i < m_numFrames - 1; i++)
	 {
	 map<int, int> & match = m_matches[i];
	 writeValue(ofs, (int)match.size());
	 map<int,int>::iterator it = match.begin();
	 while(it != match.end())
	 {
	 writeValue(ofs,(int)((*it).first));
	 writeValue(ofs,(int)((*it).second));
	 it++;
	 }
	 }
	/*
	 * 5. save m_threshold;
	 */
	writeValue(ofs,m_threshold);
	ofs.close();
	return true;
}

/********************************************************************************
 * loadTracking : load the results of tracking
 ********************************************************************************/
bool CellTrack::loadTracking(int argc, char* argv[])
{
	/*
	 * 0. free containers
	 */

	string result_name = DEFAULT_OUTPUTDIR;
	result_name += "/ct3d.tr";
	m_outdir = DEFAULT_OUTPUTDIR;
	int i = 0;
	bool lFlag = 0;
	bool oFlag = 0;
	while(i < argc )
	{
		if(strcmp(argv[i], "-i") == 0 && lFlag == 0)
		{
			assert(argc >= i+2);
			result_name = argv[++i];
			lFlag = 1;
			if((access(result_name.c_str(),0)) == -1)
			{
				cerr<<"Error : no input file ("<<result_name.c_str()<<")"<<endl;
				return false;
			}
			else if((access(result_name.c_str(),2)) == -1)
			{
				cerr<<"Error : "<<result_name.c_str()<<" has write permission "<<endl;
				return false;
			}
		}
		else if(strcmp(argv[i], "-o") == 0 && oFlag == 0)
		{
			assert(argc >= i+2);
			m_outdir = argv[++i];
			oFlag = 1;
			if((access(m_outdir.c_str(),0)) == -1)
			{
				system((string("mkdir ")+m_outdir).c_str());
			}
		}
		else if(strcmp(argv[i],"-v") == 0)
		{
			verbose = 1;
		}
		else
		{
			cerr<<"Error : unknow parameter : "<<argv[i]<<endl;
			return false;
		}
		i++;
	}
	
	//if(!oFlag && m_outdir.empty()) m_outdir = "out";
	
	if(verbose) cout<<"\tload -i "<<result_name.c_str()<<" -o "<<m_outdir.c_str()<<" -v"<<endl;
	else  cout<<"\tload -i "<<result_name.c_str()<<" -o "<<m_outdir.c_str()<<endl;
	cout<<endl;
	
	clear();  //clear data
	
	ifstream ifs(result_name.c_str());
	if(ifs.fail())
	{
		cerr<<"\topen "<<result_name.c_str()<<"  failed"<<endl;
		return false;
	}
	/*
	 * 1. load m_numFrames, m_numTracks
	 */
	readValue(ifs,m_numFrames);
	readValue(ifs,m_numTracks);
	/*
	 * 2. load m_trees
	 */
	cout<<"\nloading component trees ... "<<endl;
	m_trees.resize(m_numFrames);
	for(int i = 0; i < m_numFrames; i++)
	{
		cout<<"\ttree "<<i+1<<endl;
		m_trees[i].load(ifs);
	}
	
	 /*
	 * 4. load m_tracks and set m_frames
	 */
	cout<<"\nloading tracking results ... "<<endl;
	cout<<"\t"<<m_numTracks<<" tracks"<<endl;
	m_frames.resize(m_numFrames);
	m_tracks.resize(m_numTracks);
	for(int i = 0; i < m_numTracks; i++)
	{
		readValue(ifs,m_tracks[i].trackId);
		readValue(ifs,m_tracks[i].altitude);
		readValue(ifs,m_tracks[i].start);
		readValue(ifs,m_tracks[i].end);
		
		assert(m_tracks[i].end > m_tracks[i].start);
		
		Cell* entry = new Cell;
		readValue(ifs,entry->label);
		readValue(ifs,entry->prevLabel);
		readValue(ifs,entry->nextLabel);
		entry->trackId = m_tracks[i].trackId;
		entry->prev = NULL;
		m_tracks[i].entry = entry;
		Cell* prev = entry;
		
		m_frames[m_tracks[i].start].push_back(entry);
		Cell* p;
		for(int j = m_tracks[i].start + 1; j < m_tracks[i].end; j++)
		{
			p = new Cell;
			readValue(ifs, p->label);
			readValue(ifs, p->prevLabel);
			readValue(ifs, p->nextLabel);
			p->trackId = m_tracks[i].trackId;
			p->prev = prev;
			prev->next = p;
			prev = p;
			m_frames[j].push_back(p);
		}
	}
	/*
	 * 3. load m_matches
	 */

	 cout<<"\n loading alignment results ..."<<endl;
	 m_matches.resize(m_numFrames - 1);
	 for(int i = 0 ; i < m_numFrames - 1; i++)
	 {
		 int _size;
		 readValue(ifs,_size);
		 for(int j = 0; j < _size; j++)
		 {
				int key, value;
				readValue(ifs,key);
				readValue(ifs,value);
				m_matches[i][key] = value;
		 }
	 }
	 
	/*
	 * 5. read m_threshold and save frames
	 */
	readValue(ifs,m_threshold);
	ifs.close();
	
	m_names.resize(m_numFrames);
	for(int i = 0; i < m_numFrames; i++)
	{
		char index[255];
		char format[255];
		sprintf(format, "%c0%d%c",'%',(int)log10(m_numFrames)+2,'d');
		sprintf(index,format,i+1);
		m_names[i] = string("frame") + index + ".tif";
	}
	saveImages(); 
	saveGraph();
	saveGraph2();
	return true;
}

/******************************************************
 * saveImages : save the fid frame, fid starts from 0
 ******************************************************/

void CellTrack::saveImages()
{
	cout<<"saving images to folder: "<<m_outdir.c_str()<<endl;
	for(int i=0; i < m_numFrames; i++)
	{
		//cout<<"\timage"<<i+1<<": ";
		//==============================================================
		//saveFrame(i);
		if(m_numTracks > MAXCELLNUM) 
		{
			//cout<<"trackId > "<<MAXCELLNUM<<endl;
			MAXCELLNUM = 2*MAXCELLNUM;
			m_palette.init(MAXCELLNUM);
		}
		/*
		 * 1. get labels and values vector<int>
		 */
		vector<int> labels;
		vector<int> values;
		Frame::iterator it = m_frames[i].begin();
		while( it != m_frames[i].end())
		{
			int trackId = (*it)->trackId;
			if(m_tracks[trackId-1].altitude >= m_threshold)
			{
				//cout<<trackId<<" ";  // the trackId starts from 1
				labels.push_back((*it)->label);
				values.push_back((*it)->trackId);
			}
			it++;
		}
		/*
		 * 2. get value matrix
		 */
		int* matrix = m_trees[i].getMatrix(labels, values, 0);
		
		/*
		 * 3. output the results
		 */

		string out_name = m_outdir + "/" +  m_names[i];
		cout<<"\tsave image "<<i+1<<" to "<<out_name.c_str()<<endl;
		int width = m_trees[0].width();
		int height = m_trees[0].height();
		int depth = m_trees[0].depth();
		int channels = 3;
		int _size = width * height * depth;
		
		unsigned char * color_matrix = new unsigned char[_size * 3];
		
		for(int i=0; i< _size; i++)
		{
			rgb_pixel c = m_palette(matrix[i]);
			color_matrix[3*i] = c.r;
			color_matrix[3*i+1] = c.g;
			color_matrix[3*i+2] = c.b;
		}
		
		writetiff((char*)out_name.c_str(),color_matrix,channels, width, height, depth);
		delete matrix;
		delete color_matrix;
		//saveFrame(i) over
		//========================================================================
		//cout<<endl;
	}
}


/******************************************
 * saveGraph : save undirect graph
 *
 * Name rules:
 *
 * 1.  treeId : 1 to N
 * 2.  matchId : 1 to N-1
 *
 * 3. cluster name : "cluster_%1%2"
 *     %1 : "tree" or "match"
 *     %2 : treeId or matchId, 
 *
 * 4. tree cluster node name : "tree%1_%2"
 *        %1 : treeId
 *        %2 : node's label
 * 5. match cluster node name
 *      treeNode : "match%1tree%2_%3"
 *      matchNode : "match%1_%3"
 *            %1 : matchId
 *            %2 : treeId
 *            %3 : 1 to Max 
 * 6. frame cluster node name : "frame%1_%2_%3"
 *            %1 : treeId
 *            %2 : label in previous match
 *            %3 : label in next match * Display name rules:
 *
 *
 ******************************************/

void CellTrack::saveGraph() const
{
	string filename = m_outdir+"/topology.dot";
	ofstream ofs(filename.c_str());
	if(ofs.fail())
	{
		cerr<<"\tunable to open "<<filename<<endl;
		cerr<<"\tsave graph error"<<endl;
		return;
	}
	// 1. output the head
	ofs<<"graph G{"<<endl;
	
	//2. output each tree
	vector<ComponentTree>::const_iterator it = m_trees.begin();
	int treeId = 1;
	char prefix[100];
	char parent_node[100];
	char child_node[100];
	while(it != m_trees.end())
	{
		ComponentTree::Node* root = (*it).root();
		
		sprintf(prefix, "%s%d","tree",treeId);
		
		ofs<<"\tsubgraph cluster_tree"<<treeId<<"{"<<endl;
		
		deque<ComponentTree::Node*> queue;
		ComponentTree::Node* front;
		queue.push_back(root);
		
		sprintf(parent_node,"%s_%d",prefix,root->label);
		
		ofs<<"\t\t"<<parent_node<<" [label = "<<prefix<<" , color=lightblue, style=filled];"<<endl;
		while(! queue.empty())
		{
			front = queue.front();
			queue.pop_front();
			
			sprintf(parent_node,"%s_%d",prefix,front->label);
			
			vector<ComponentTree::Node*>::iterator itr = front->childs.begin();
			while(itr != front->childs.end())
			{
				sprintf(child_node,"%s_%d",prefix,(*itr)->label);
				ofs<<"\t\t"<<child_node<<" [label = "<<(*itr)->label<<"];"<<endl;
				ofs<<"\t\t"<<parent_node<<" -- "<<child_node<<";"<<endl;
				//ofs<<"\t\t"<<child_node<<" -> "<<parent_node<<";"<<endl;
				queue.push_back(*itr);
				itr++;
			}
		}
		
		//ofs<<"\t label = "<<prefix<<";"<<endl;
		ofs<<"\t }"<<endl;
		treeId++;
		it++;
	}
	ofs<<"}"<<endl;
	ofs.close();
	
	/*************************************************************
	 * save the second graph
	 *************************************************************/
	filename = m_outdir+"/connection.dot";
	ofs.open(filename.c_str());
	if(ofs.fail())
	{
		cerr<<"\tunable to open "<<filename<<endl;
		cerr<<"\tsave graph error"<<endl;
		return;
	}
	ofs<<"graph G{"<<endl;

	for(int i= 0; i < m_numFrames; i++)
	{
		ofs<<"\t subgraph cluster_frame"<<i+1<<"{"<<endl;
		ofs<<"\t\tlabel = "<<"frame"<<i+1<<endl;
		for(int j = m_frames[i].size() - 1 ; j >= 0; j--)
		//for(int j = 0 ; j < m_frames[i].size(); j++)
		{
			char frameNode[100];
			int prevLabel = m_frames[i][j]->prevLabel;
			int nextLabel = m_frames[i][j]->nextLabel;
			sprintf(frameNode,"frame%d_",i+1);
			if(prevLabel == -1) sprintf(frameNode,"%sNM_",frameNode);
			else sprintf(frameNode,"%s%d_",frameNode,prevLabel);
			if(nextLabel == -1) sprintf(frameNode,"%sNM",frameNode);
			else sprintf(frameNode,"%s%d",frameNode,nextLabel);
			
			ofs<<"\t\t"<<frameNode<<" [label = \""<<prevLabel<<"|"<<nextLabel<<"\"];"<<endl;
			
		}
		ofs<<"\t }"<<endl;
	}
	for(int i= 0; i < m_numFrames - 1; i++)
	{
		map<int,int> match = m_matches[i];
		for(int j = m_frames[i].size() - 1 ; j >= 0; j--)
			//for(int j = 0 ; j < m_frames[i].size(); j++)
		{
			char frameNode1[100];
			char frameNode2[100];
			int prevLabel = m_frames[i][j]->prevLabel;
			int nextLabel = m_frames[i][j]->nextLabel;
							 
			sprintf(frameNode1,"frame%d_",i+1);
					
			if(prevLabel == -1) sprintf(frameNode1,"%sNM_",frameNode1);
			else sprintf(frameNode1,"%s%d_",frameNode1,prevLabel);
			if(nextLabel == -1) sprintf(frameNode1,"%sNM",frameNode1);
			else sprintf(frameNode1,"%s%d",frameNode1,nextLabel);
			
			if(match.find(nextLabel) != match.end())
			{
				prevLabel = match[nextLabel];
				int jj;
				for( jj = 0; jj < m_frames[i+1].size(); jj++)
				{
					if(prevLabel == m_frames[i+1][jj]->prevLabel)break;
				}
				assert(jj != m_frames[i+1].size());
				
				nextLabel = m_frames[i+1][jj]->nextLabel;
				sprintf(frameNode2,"frame%d_%d_",i+2,prevLabel);
				if(nextLabel == -1) sprintf(frameNode2,"%sNM",frameNode2);
				else sprintf(frameNode2,"%s%d",frameNode2,nextLabel);
				ofs<<"\t\t"<<frameNode1<<" -- "<<frameNode2<<";"<<endl;
			}
			else
			{
				assert(nextLabel == -1);
			}
		}
	}

	ofs<<"}"<<endl;
	ofs.close();
}

void CellTrack::saveGraph2() const
{
	string relationName = m_outdir+"/topology.sif";
	string attributeName = m_outdir+"/attribute.att";
	ofstream ofs(relationName.c_str());
	ofstream ofs2(attributeName.c_str());
	if(ofs.fail() || ofs2.fail())
	{
		cerr<<"\tunable to open "<<relationName<<endl;
		cerr<<"\tsave graph error"<<endl;
		return;
	}
	
	//2. output each tree
	vector<ComponentTree>::const_iterator it = m_trees.begin();
	ofs<<"relation"<<endl;
	ofs2<<"attribute"<<endl;
	int treeId = 1;
	char prefix[100];
	char parent_node[100];
	char child_node[100];
	while(it != m_trees.end())
	{
		ComponentTree::Node* root = (*it).root();
		
		sprintf(prefix, "%s%d","tree",treeId);
				
		deque<ComponentTree::Node*> queue;
		ComponentTree::Node* front;
		queue.push_back(root);
		
		sprintf(parent_node,"%s_%d",prefix,root->label);
		ofs2<<parent_node<<" = "<<prefix<<endl;
		while(! queue.empty())
		{
			front = queue.front();
			queue.pop_front();
			
			sprintf(parent_node,"%s_%d",prefix,front->label);
			
			vector<ComponentTree::Node*>::reverse_iterator itr = front->childs.rbegin();
			while(itr != front->childs.rend())
			{
				sprintf(child_node,"%s_%d",prefix,(*itr)->label);
				ofs2<<child_node<<" = "<<(*itr)->label<<endl;
				ofs<<parent_node<<" pa "<<child_node<<endl;
				//ofs<<child_node<<" -> "<<parent_node<<";"<<endl;
				queue.push_back(*itr);
				itr++;
			}
		}
		
		treeId++;
		it++;
	}
	
	ofs.close();
	
	 // save the second graph
	relationName = m_outdir+"/connection.sif";
	ofs.open(relationName.c_str());
	if(ofs.fail())
	{
		cerr<<"\tunable to open "<<relationName<<endl;
		cerr<<"\tsave graph error"<<endl;
		return;
	}
	 
	
	for(int i= 0; i < m_numFrames - 1; i++)
	{
		map<int,int> match = m_matches[i];
		for(int j = m_frames[i].size() - 1 ; j >= 0; j--)
			//for(int j = 0 ; j < m_frames[i].size(); j++)
		{
			char frameNode1[100];
			char frameNode2[100];			
			sprintf(frameNode1,"tree%d_%d",i+1, m_frames[i][j]->label);
			
			int nextLabel = m_frames[i][j]->nextLabel;
			if(match.find(nextLabel) != match.end())
			{
				int prevLabel = match[nextLabel];
				int jj;
				for( jj = 0; jj < m_frames[i+1].size(); jj++)
				{
					if(prevLabel == m_frames[i+1][jj]->prevLabel)break;
				}
				assert(jj != m_frames[i+1].size());
				
				sprintf(frameNode2,"tree%d_%d",i+2,m_frames[i][jj]->label);
				ofs<<frameNode1<<" pb "<<frameNode2<<endl;
			}
		}
	}
	ofs.close();
}

int CellTrack::frameCount() const
{
	return m_numFrames;
}

int CellTrack::trackCount() const
{
	return m_numTracks;
}

ComponentTree& CellTrack::getTree(int fid)
{
	return m_trees[fid];
}

void CellTrack::filterCells(double threshold)
{
	vector<double> data;
	if(threshold <= 0)
	{
		cout.precision(3);
		for(int i = 0; i < m_numTracks; i++)
		{
			/*int start = m_tracks[i].start;
			Cell* c = m_tracks[i].entry;
			if(verbose) cout<<i+1<<": ";
			Cell* p= c;
			while(p)
			{
				ComponentTree& tree = m_trees[start++];
				double contrast = (tree.getNode(p->label))->mean -  (tree.getNode(p->label))->level;
				data.push_back(contrast);
				if(verbose) cout<<contrast<<" ";
				p = p->next;
			}
			if(verbose) cout<<"\t"<<m_tracks[i].start+1<<" ~ "<<m_tracks[i].end+1<<" altitude = "<<m_tracks[i].altitude<<endl; 
			 */
			data.push_back(m_tracks[i].altitude);
		}
		//if(verbose) cout<<endl;
		m_threshold = ostu_thresh(data);
	}
	else m_threshold = threshold;
	saveImages();
	cout<<"altitudes : "<<endl;
	vector<double>::iterator it = data.begin();
	while(it!= data.end())
	{
			cout<<(*it)<<" ";
			it++;
	}
	cout<<endl;
	cout<<"threshold = "<<m_threshold<<endl;
}

void CellTrack::resetCells()
{
	m_threshold = -1.0;
	saveImages();
}

void create_match(double * row, int num1, int num2, map<int,int>& matche)
{
	matche.clear();
	for(int i = 0; i < num1; i++)
	{
		for(int j = 0; j < num2; j++)
		{
			if(fabs(row[i*num2+j] - 1.0)< 0.001)
			{
				matche[i] = j;
			}
		}
	}
}

void create_frames(map<int, int>& match, CellTrack::Frame &f1, CellTrack::Frame &f2)
{
	int count = 0;
	cout.precision(3);
	map<int, int>::iterator it = match.begin();
	while(it != match.end())
	{
		int i = (*it).first;
		int j = (*it).second;
		
		CellTrack::Cell* c1 = new CellTrack::Cell;
		CellTrack::Cell* c2 = new CellTrack::Cell;
		c1->label = i;
		c1->prevLabel = -1;
		c1->nextLabel = i;
		c1->prev = NULL;
		c1->next = c2;
		
		c2->label = j;
		c2->prevLabel = j;
		c2->nextLabel = -1;
		c2->prev = c1;
		c2->next = NULL;
		f1.push_back(c1);
		f2.push_back(c2);
		
		it++;
	}
	cout<<"\t"<<match.size()<<" matches"<<endl;
}

CellTrack::Frame merge_frames(ComponentTree &tree, double* row, CellTrack::Frame &f1, CellTrack::Frame &f2)
{
	int count = 0;
	CellTrack::Frame merge_frame = f1;
	
	for(int j = 0; j < f2.size(); j++)
	{
		int match = 0;
		for(int i=0; i < f1.size();i++)
		{
			//merge two cell
			if(fabs(row[i*(f2.size())+j] - 1.0)< 0.001)
			{
				match = 1;
				count++;
				CellTrack::Cell* c1 = f1[i];
				CellTrack::Cell* c2 = f2[j];

				if((tree.getNode(c1->label))->alpha_size < (tree.getNode(c2->label))->alpha_size)
				{
					c1->nextLabel = c2->nextLabel;     // if c2 label is larger , assign c2's nextLabel to c1's label
					c1->label = c2->nextLabel;
					
				}
				c1->next = c2->next;
				assert(c2->next != NULL);
				(c2->next)->prev = c1;
				delete c2;
				c2 = NULL;
				break; // one j only match with one i
			}	
		}
		//deal with the unmatched cell
		if(match == 0)
		{
			merge_frame.push_back(f2[j]);
		}
	}
	f1.clear();
	f2.clear();
	cout<<"\t"<<count<<"matches"<<endl;
	return merge_frame;
}


