#include <GL/glew.h>
#include "VRFinger.h"
#include "../neuron_tracing/fastmarching_linker.h"


//LMG for Windows UTC Timestamp 15/10/2018
#define timegm _mkgmtime

bool smooth_sketch_curve(std::vector<MyMarker *> & mCoord, int winsize)
{
	//std::cout<<" smooth_curve ";
	if (winsize<2) return true;

	std::vector<MyMarker *> mC = mCoord; // a copy
	V3DLONG N = mCoord.size();
	int halfwin = winsize/2;

	for (int i=1; i<N-1; i++) // don't move start & end point
	{
		std::vector<MyMarker*> winC;
		std::vector<double> winW;
		winC.clear();
		winW.clear();

		winC.push_back( mC[i] );
		winW.push_back( 1.+halfwin );
		for (int j=1; j<=halfwin; j++)
		{
			int k1 = i+j;	if(k1<0) k1=0;	if(k1>N-1) k1=N-1;
			int k2 = i-j;	if(k2<0) k2=0;	if(k2>N-1) k2=N-1;
			winC.push_back( mC[k1] );
			winC.push_back( mC[k2] );
			winW.push_back( 1.+halfwin-j );
			winW.push_back( 1.+halfwin-j );
		}
		//std::cout<<"winC.size = "<<winC.size()<<"\n";

		double s, x,y,z;
		s = x = y = z = 0;
        for (int i2=0; i2<winC.size(); i2++)
		{
                        x += winW[i2]* winC[i2]->x;
                        y += winW[i2]* winC[i2]->y;
                        z += winW[i2]* winC[i2]->z;
                        s += winW[i2];
		}
		if (s)
		{
			x /= s;
			y /= s;
			z /= s;
		}

		mCoord[i]->x = x; // output
		mCoord[i]->y = y; // output
		mCoord[i]->z = z; // output
	}
	return true;
}




void VectorResampling(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, float epsilon)
{
     int N = loc_vec.size();
     if (N<=0) return;

    VectorResamplinger(loc_vec, loc_vec_resampled, 0, N-1, epsilon);
}

void VectorResamplinger(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, int start_i, int end_i, float epsilon)
{
     // Recursive Ramer¨CDouglas¨CPeucker algorithm
     loc_vec_resampled.clear();
     XYZ & loc_start = loc_vec.at(start_i);
     XYZ & loc_final = loc_vec.at(end_i);

     if (start_i >= end_i)
     {
          loc_vec_resampled.push_back(loc_start);
          return;
     }
     else if (end_i - start_i == 1)
     {
          loc_vec_resampled.push_back(loc_start);
          loc_vec_resampled.push_back(loc_final);
          return;
     }

     float dx = loc_final.x-loc_start.x;
     float dy = loc_final.y-loc_start.y;
     float dz = loc_final.z-loc_start.z;
     // To be used in distance from point to line calculation below
     float dd = std::sqrt(dx*dx + dy*dy + dz*dz);
     // Find point with max distance between it and v1
     float max_dist_squared = -1.0f;
     int max_ind = -1;
     for (int j=start_i+1; j<end_i; j++)
     {
          XYZ & loc_this = loc_vec.at(j);
          // Compute distance from point at j to line between loc_start and loc_final
          Vector3D v1 = Vector3D(loc_this.x-loc_start.x, loc_this.y-loc_start.y, loc_this.z-loc_start.z);
          Vector3D v2 = Vector3D(loc_this.x-loc_final.x, loc_this.y-loc_final.y, loc_this.z-loc_final.z);
          Vector3D v3 = v1.cross(v2);
          float this_dist_sqaured = v3.normSquared();
          if (this_dist_sqaured > max_dist_squared)
          {
               max_dist_squared = this_dist_sqaured;
               max_ind = j;
          }
     }
     // Calculate actual max distance and compare to epsilon
     bool within_epsilon = false;
     if (dd > 0) // avoid divide by zero
     {
          float max_dist = std::sqrt(max_dist_squared) / dd;
          within_epsilon = (max_dist <= epsilon);
     }
     if (within_epsilon)
     {
          // If within epsilon, we can safely skip the points in between. Just return first and last points.
          loc_vec_resampled.push_back(loc_start);
          loc_vec_resampled.push_back(loc_final);
     }
     else
     {
          // If outside of epsilon, sample recursively between the two segments: start_i -> max_ind and max_indx -> end_i
          vector <XYZ> loc_vec_resampled1, loc_vec_resampled2;
          VectorResamplinger(loc_vec, loc_vec_resampled1, start_i, max_ind, epsilon);
          VectorResamplinger(loc_vec, loc_vec_resampled2, max_ind, end_i, epsilon);
          // Quickly join the two vectors, removing last element of vec1 (which would be duplicated)
          loc_vec_resampled1.pop_back();
          loc_vec_resampled.reserve(loc_vec_resampled1.size() + loc_vec_resampled2.size());
          loc_vec_resampled.insert(loc_vec_resampled.end(), loc_vec_resampled1.begin(), loc_vec_resampled1.end());
          loc_vec_resampled.insert(loc_vec_resampled.end(), loc_vec_resampled2.begin(), loc_vec_resampled2.end());
     }

}

void  VectorToNeuronTree(NeuronTree &SS, vector<XYZ> loc_list,int nttype, double creatmode)
{

	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

    // Add timestamp LMG 26/10/2018
    // Get current timestamp
    time_t timer2;
    struct tm y2k = {0};
    double seconds;

    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1; // seconds since January 1, 2000 in UTC

    time(&timer2);  /* get current time; same as: timer = time(NULL)  */

    seconds = difftime(timer2,timegm(&y2k)); //Timestamp LMG 27/9/2018
    qDebug("Timestamp at VectorToNeuronTree (VR) (seconds since January 1, 2000 in UTC): %.0f", seconds);

     int count = 0;

     qDebug("-------------------------------------------------------");
     for (int k=0;k<loc_list.size();k++)
     {
          count++;
          NeuronSWC S;

          S.n 	= 1+k;
          S.type 	= nttype;
          S.x 	= loc_list.at(k).x;
          S.y 	= loc_list.at(k).y;
          S.z 	= loc_list.at(k).z;
          S.r 	= 1;
          S.pn 	= (k==0)? -1 : k;

          S.creatmode = creatmode; //LMG 26/10/2018 for tracking curve cration modes
          if(S.timestamp == 0) S.timestamp = seconds; //LMG 26/10/2018 timestamp

          //qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);
          {
               listNeuron.append(S);
               hashNeuron.insert(S.n, listNeuron.size()-1);
          }
     }

     SS.n = -1;
     RGBA8 cc;
     cc.r=0; cc.g=20;cc.b=200;cc.a=0;
     SS.color = cc; //random_rgba8(255);//RGBA8(255, 0,0,0);
     SS.on = true;
     SS.listNeuron = listNeuron;
     SS.hashNeuron = hashNeuron;

}

