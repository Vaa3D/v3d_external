﻿#include "./v3dr_gl_vr.h"
#include "VRFinger.h"
#include"./spline.h"
//#include <GL/glew.h>
#include <SDL_opengl.h>

#include "../v3d/vr_vaa3d_call.h"
#include "../neuron_tracing/fastmarching_linker.h"



#if defined( OSX )
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <OpenGL/glu.h>
// Apple's version of glut.h #undef's APIENTRY, redefine it
#define APIENTRY
#else
#include <GL/glu.h>
#endif


#include <stdio.h>
#include <string>
#include <cstdlib>

#include "shader_m.h"
#include "Sphere.h"
#include "Cylinder.h"

#if defined(POSIX)
#include "unistd.h"
#endif

#ifndef _WIN32
#define APIENTRY
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif
#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;
#define MAX_UNDO_COUNT 5

#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif
int checkForOpenGLError(const char* file, int line)
{
    // return 1 if an OpenGL error occured, 0 otherwise.
    GLenum glErr;
    int retCode = 0;

    glErr = glGetError();
    while(glErr != GL_NO_ERROR)
    {
	cout << "glError in file " << file
	     << "@line " << line << gluErrorString(glErr) << endl;
	retCode = 1;
	break;
	//exit(EXIT_FAILURE);
    }
    return retCode;
}

float CMainApplication::fContrast = 1;
bool CMainApplication::m_bFrozen = false;
bool CMainApplication::m_bVirtualFingerON = false;
float CMainApplication::iLineWid = 1;
float CMainApplication::iscaleZ =1;
float CMainApplication::fBrightness = 0.0;
int CMainApplication::m_curMarkerColorType = 6;
int CMainApplication::m_modeControlGrip_L = 0;
glm::mat4 CMainApplication::m_globalMatrix = glm::mat4();
My4DImage *CMainApplication::img4d_replace = nullptr;
ModelControlR CMainApplication::m_modeGrip_R = m_drawMode;
ModeControlSettings  CMainApplication::m_modeGrip_L = _donothing;
ModeTouchPadR CMainApplication::m_modeTouchPad_R = tr_contrast;
SecondeMenu CMainApplication::m_secondMenu = _nothing;
RGBImageChannel CMainApplication::m_rgbChannel = channel_rgb;
bool CMainApplication::showshootingPad = false;

#define dist_thres 0.01
#define connection_rigourous 0.5
#define default_radius 0.618
#define drawing_step_size 2  //the larger, the fewer SWC nodes
//LMG for Windows UTC Timestamp 15/10/2018
#define timegm _mkgmtime
//the following table is copied from renderer_obj.cpp and should be eventually separated out as a single neuron drawing routine. Boted by PHC 20170616

const GLubyte neuron_type_color_heat[ ][3] = { //whilte---> yellow ---> red ----> black  (hotness increases)
{ 255.0 , 255.0 , 255.0 }, //white
{ 255.0 , 255.0 , 251.062496062 },
{ 255.0 , 255.0 , 247.124992125 },
{ 255.0 , 255.0 , 243.187488187 },
{ 255.0 , 255.0 , 239.24998425 },
{ 255.0 , 255.0 , 235.312480312 },
{ 255.0 , 255.0 , 231.374976375 },
{ 255.0 , 255.0 , 227.437472437 },
{ 255.0 , 255.0 , 223.4999685 },
{ 255.0 , 255.0 , 219.562464562 },
{ 255.0 , 255.0 , 215.624960625 },
{ 255.0 , 255.0 , 211.687456687 },
{ 255.0 , 255.0 , 207.74995275 },
{ 255.0 , 255.0 , 203.812448812 },
{ 255.0 , 255.0 , 199.874944875 },
{ 255.0 , 255.0 , 195.937440937 },
{ 255.0 , 255.0 , 191.999937 },
{ 255.0 , 255.0 , 188.062433062 },
{ 255.0 , 255.0 , 184.124929125 },
{ 255.0 , 255.0 , 180.187425187 },
{ 255.0 , 255.0 , 176.24992125 },
{ 255.0 , 255.0 , 172.312417312 },
{ 255.0 , 255.0 , 168.374913375 },
{ 255.0 , 255.0 , 164.437409437 },
{ 255.0 , 255.0 , 160.4999055 },
{ 255.0 , 255.0 , 156.562401562 },
{ 255.0 , 255.0 , 152.624897625 },
{ 255.0 , 255.0 , 148.687393687 },
{ 255.0 , 255.0 , 144.74988975 },
{ 255.0 , 255.0 , 140.812385812 },
{ 255.0 , 255.0 , 136.874881875 },
{ 255.0 , 255.0 , 132.937377937 },
{ 255.0 , 255.0 , 128.999874 },
{ 255.0 , 255.0 , 125.062370062 },
{ 255.0 , 255.0 , 121.124866125 },
{ 255.0 , 255.0 , 117.187362187 },
{ 255.0 , 255.0 , 113.24985825 },
{ 255.0 , 255.0 , 109.312354312 },
{ 255.0 , 255.0 , 105.374850375 },
{ 255.0 , 255.0 , 101.437346437 },
{ 255.0 , 255.0 , 97.4998424998 },
{ 255.0 , 255.0 , 93.5623385623 },
{ 255.0 , 255.0 , 89.6248346248 },
{ 255.0 , 255.0 , 85.6873306873 },
{ 255.0 , 255.0 , 81.7498267498 },
{ 255.0 , 255.0 , 77.8123228123 },
{ 255.0 , 255.0 , 73.8748188748 },
{ 255.0 , 255.0 , 69.9373149373 },
{ 255.0 , 255.0 , 65.9998109998 },
{ 255.0 , 255.0 , 62.0623070623 },
{ 255.0 , 255.0 , 58.1248031248 },
{ 255.0 , 255.0 , 54.1872991873 },
{ 255.0 , 255.0 , 50.2497952498 },
{ 255.0 , 255.0 , 46.3122913123 },
{ 255.0 , 255.0 , 42.3747873748 },
{ 255.0 , 255.0 , 38.4372834373 },
{ 255.0 , 255.0 , 34.4997794998 },
{ 255.0 , 255.0 , 30.5622755623 },
{ 255.0 , 255.0 , 26.6247716248 },
{ 255.0 , 255.0 , 22.6872676873 },
{ 255.0 , 255.0 , 18.7497637498 },
{ 255.0 , 255.0 , 14.8122598123 },
{ 255.0 , 255.0 , 10.8747558748 },
{ 255.0 , 255.0 , 6.93725193725 },
{ 255.0 , 255.0 , 2.99974799975 },
{ 255.0 , 254.374831016 , 0.0 },
{ 255.0 , 251.749835282 , 0.0 },
{ 255.0 , 249.124839547 , 0.0 },
{ 255.0 , 246.499843813 , 0.0 },
{ 255.0 , 243.874848078 , 0.0 },
{ 255.0 , 241.249852344 , 0.0 },
{ 255.0 , 238.62485661 , 0.0 },
{ 255.0 , 235.999860875 , 0.0 },
{ 255.0 , 233.374865141 , 0.0 },
{ 255.0 , 230.749869406 , 0.0 },
{ 255.0 , 228.124873672 , 0.0 },
{ 255.0 , 225.499877938 , 0.0 },
{ 255.0 , 222.874882203 , 0.0 },
{ 255.0 , 220.249886469 , 0.0 },
{ 255.0 , 217.624890735 , 0.0 },
{ 255.0 , 214.999895 , 0.0 },
{ 255.0 , 212.374899266 , 0.0 },
{ 255.0 , 209.749903531 , 0.0 },
{ 255.0 , 207.124907797 , 0.0 },
{ 255.0 , 204.499912063 , 0.0 },
{ 255.0 , 201.874916328 , 0.0 },
{ 255.0 , 199.249920594 , 0.0 },
{ 255.0 , 196.624924859 , 0.0 },
{ 255.0 , 193.999929125 , 0.0 },
{ 255.0 , 191.374933391 , 0.0 },
{ 255.0 , 188.749937656 , 0.0 },
{ 255.0 , 186.124941922 , 0.0 },
{ 255.0 , 183.499946188 , 0.0 },
{ 255.0 , 180.874950453 , 0.0 },
{ 255.0 , 178.249954719 , 0.0 },
{ 255.0 , 175.624958984 , 0.0 },
{ 255.0 , 172.99996325 , 0.0 },
{ 255.0 , 170.374967516 , 0.0 },
{ 255.0 , 167.749971781 , 0.0 },
{ 255.0 , 165.124976047 , 0.0 },
{ 255.0 , 162.499980313 , 0.0 },
{ 255.0 , 159.874984578 , 0.0 },
{ 255.0 , 157.249988844 , 0.0 },
{ 255.0 , 154.624993109 , 0.0 },
{ 255.0 , 151.999997375 , 0.0 },
{ 255.0 , 149.375001641 , 0.0 },
{ 255.0 , 146.750005906 , 0.0 },
{ 255.0 , 144.125010172 , 0.0 },
{ 255.0 , 141.500014437 , 0.0 },
{ 255.0 , 138.875018703 , 0.0 },
{ 255.0 , 136.250022969 , 0.0 },
{ 255.0 , 133.625027234 , 0.0 },
{ 255.0 , 131.0000315 , 0.0 },
{ 255.0 , 128.375035766 , 0.0 },
{ 255.0 , 125.750040031 , 0.0 },
{ 255.0 , 123.125044297 , 0.0 },
{ 255.0 , 120.500048562 , 0.0 },
{ 255.0 , 117.875052828 , 0.0 },
{ 255.0 , 115.250057094 , 0.0 },
{ 255.0 , 112.625061359 , 0.0 },
{ 255.0 , 110.000065625 , 0.0 },
{ 255.0 , 107.375069891 , 0.0 },
{ 255.0 , 104.750074156 , 0.0 },
{ 255.0 , 102.125078422 , 0.0 },
{ 255.0 , 99.5000826874 , 0.0 },
{ 255.0 , 96.875086953 , 0.0 },
{ 255.0 , 94.2500912186 , 0.0 },
{ 255.0 , 91.6250954842 , 0.0 },
{ 255.0 , 89.0000997498 , 0.0 },
{ 255.0 , 86.3751040155 , 0.0 },
{ 255.0 , 83.7501082811 , 0.0 },
{ 255.0 , 81.1251125467 , 0.0 },
{ 255.0 , 78.5001168123 , 0.0 },
{ 255.0 , 75.8751210779 , 0.0 },
{ 255.0 , 73.2501253435 , 0.0 },
{ 255.0 , 70.6251296092 , 0.0 },
{ 255.0 , 68.0001338748 , 0.0 },
{ 255.0 , 65.3751381404 , 0.0 },
{ 255.0 , 62.750142406 , 0.0 },
{ 255.0 , 60.1251466716 , 0.0 },
{ 255.0 , 57.5001509373 , 0.0 },
{ 255.0 , 54.8751552029 , 0.0 },
{ 255.0 , 52.2501594685 , 0.0 },
{ 255.0 , 49.6251637341 , 0.0 },
{ 255.0 , 47.0001679997 , 0.0 },
{ 255.0 , 44.3751722653 , 0.0 },
{ 255.0 , 41.750176531 , 0.0 },
{ 255.0 , 39.1251807966 , 0.0 },
{ 255.0 , 36.5001850622 , 0.0 },
{ 255.0 , 33.8751893278 , 0.0 },
{ 255.0 , 31.2501935934 , 0.0 },
{ 255.0 , 28.6251978591 , 0.0 },
{ 255.0 , 26.0002021247 , 0.0 },
{ 255.0 , 23.3752063903 , 0.0 },
{ 255.0 , 20.7502106559 , 0.0 },
{ 255.0 , 18.1252149215 , 0.0 },
{ 255.0 , 15.5002191871 , 0.0 },
{ 255.0 , 12.8752234528 , 0.0 },
{ 255.0 , 10.2502277184 , 0.0 },
{ 255.0 , 7.625231984 , 0.0 },
{ 255.0 , 5.00023624962 , 0.0 },
{ 255.0 , 2.37524051523 , 0.0 },
{ 254.750226751 , 0.0 , 0.0 },
{ 252.125041517 , 0.0 , 0.0 },
{ 249.499856283 , 0.0 , 0.0 },
{ 246.874671049 , 0.0 , 0.0 },
{ 244.249485815 , 0.0 , 0.0 },
{ 241.624300582 , 0.0 , 0.0 },
{ 238.999115348 , 0.0 , 0.0 },
{ 236.373930114 , 0.0 , 0.0 },
{ 233.74874488 , 0.0 , 0.0 },
{ 231.123559646 , 0.0 , 0.0 },
{ 228.498374412 , 0.0 , 0.0 },
{ 225.873189178 , 0.0 , 0.0 },
{ 223.248003944 , 0.0 , 0.0 },
{ 220.62281871 , 0.0 , 0.0 },
{ 217.997633477 , 0.0 , 0.0 },
{ 215.372448243 , 0.0 , 0.0 },
{ 212.747263009 , 0.0 , 0.0 },
{ 210.122077775 , 0.0 , 0.0 },
{ 207.496892541 , 0.0 , 0.0 },
{ 204.871707307 , 0.0 , 0.0 },
{ 202.246522073 , 0.0 , 0.0 },
{ 199.621336839 , 0.0 , 0.0 },
{ 196.996151606 , 0.0 , 0.0 },
{ 194.370966372 , 0.0 , 0.0 },
{ 191.745781138 , 0.0 , 0.0 },
{ 189.120595904 , 0.0 , 0.0 },
{ 186.49541067 , 0.0 , 0.0 },
{ 183.870225436 , 0.0 , 0.0 },
{ 181.245040202 , 0.0 , 0.0 },
{ 178.619854968 , 0.0 , 0.0 },
{ 175.994669734 , 0.0 , 0.0 },
{ 173.369484501 , 0.0 , 0.0 },
{ 170.744299267 , 0.0 , 0.0 },
{ 168.119114033 , 0.0 , 0.0 },
{ 165.493928799 , 0.0 , 0.0 },
{ 162.868743565 , 0.0 , 0.0 },
{ 160.243558331 , 0.0 , 0.0 },
{ 157.618373097 , 0.0 , 0.0 },
{ 154.993187863 , 0.0 , 0.0 },
{ 152.36800263 , 0.0 , 0.0 },
{ 149.742817396 , 0.0 , 0.0 },
{ 147.117632162 , 0.0 , 0.0 },
{ 144.492446928 , 0.0 , 0.0 },
{ 141.867261694 , 0.0 , 0.0 },
{ 139.24207646 , 0.0 , 0.0 },
{ 136.616891226 , 0.0 , 0.0 },
{ 133.991705992 , 0.0 , 0.0 },
{ 131.366520759 , 0.0 , 0.0 },
{ 128.741335525 , 0.0 , 0.0 },
{ 126.116150291 , 0.0 , 0.0 },
{ 123.490965057 , 0.0 , 0.0 },
{ 120.865779823 , 0.0 , 0.0 },
{ 118.240594589 , 0.0 , 0.0 },
{ 115.615409355 , 0.0 , 0.0 },
{ 112.990224121 , 0.0 , 0.0 },
{ 110.365038887 , 0.0 , 0.0 },
{ 107.739853654 , 0.0 , 0.0 },
{ 105.11466842 , 0.0 , 0.0 },
{ 102.489483186 , 0.0 , 0.0 },
{ 99.864297952 , 0.0 , 0.0 },
{ 97.2391127181 , 0.0 , 0.0 },
{ 94.6139274842 , 0.0 , 0.0 },
{ 91.9887422503 , 0.0 , 0.0 },
{ 89.3635570164 , 0.0 , 0.0 },
{ 86.7383717825 , 0.0 , 0.0 },
{ 84.1131865487 , 0.0 , 0.0 },
{ 81.4880013148 , 0.0 , 0.0 },
{ 78.8628160809 , 0.0 , 0.0 },
{ 76.237630847 , 0.0 , 0.0 },
{ 73.6124456131 , 0.0 , 0.0 },
{ 70.9872603793 , 0.0 , 0.0 },
{ 68.3620751454 , 0.0 , 0.0 },
{ 65.7368899115 , 0.0 , 0.0 },
{ 63.1117046776 , 0.0 , 0.0 },
{ 60.4865194437 , 0.0 , 0.0 },
{ 57.8613342099 , 0.0 , 0.0 },
{ 55.236148976 , 0.0 , 0.0 },
{ 52.6109637421 , 0.0 , 0.0 },
{ 49.9857785082 , 0.0 , 0.0 },
{ 47.3605932743 , 0.0 , 0.0 },
{ 44.7354080405 , 0.0 , 0.0 },
{ 42.1102228066 , 0.0 , 0.0 },
{ 39.4850375727 , 0.0 , 0.0 },
{ 36.8598523388 , 0.0 , 0.0 },
{ 34.2346671049 , 0.0 , 0.0 },
{ 31.609481871 , 0.0 , 0.0 },
{ 28.9842966372 , 0.0 , 0.0 },
{ 26.3591114033 , 0.0 , 0.0 },
{ 23.7339261694 , 0.0 , 0.0 },
{ 21.1087409355 , 0.0 , 0.0 },
{ 18.4835557016 , 0.0 , 0.0 },
{ 15.8583704678 , 0.0 , 0.0 },
{ 13.2331852339 , 0.0 , 0.0 },
{ 10.608 , 0.0 , 0.0 } // black
};
const GLubyte neuron_type_color[ ][3] = {///////////////////////////////////////////////////////
        {255, 255, 255},  // white,   0-undefined
        {20,  20,  20 },  // black,   1-soma
        {200, 20,  0  },  // red,     2-axon
        {0,   20,  200},  // blue,    3-dendrite
        {200, 0,   200},  // purple,  4-apical dendrite
        //the following is Hanchuan's extended color. 090331
        {0,   200, 200},  // cyan,    5
        {220, 200, 0  },  // yellow,  6
        {0,   200, 20 },  // green,   7
        {188, 94,  37 },  // coffee,  8
        {180, 200, 120},  // asparagus,	9
        {250, 100, 120},  // salmon,	10
        {120, 200, 200},  // ice,		11
        {100, 120, 200},  // orchid,	12
    //the following is Hanchuan's further extended color. 111003
    {255, 128, 168},  //	13
    {128, 255, 168},  //	14
    {128, 168, 255},  //	15
    {168, 255, 128},  //	16
    {255, 168, 128},  //	17
    {168, 128, 255}, //	18
    {0, 0, 0}, //19 //totally black. PHC, 2012-02-15
    //the following (20-275) is used for matlab heat map. 120209 by WYN
    {0,0,131}, //20
    {0,0,135},
    {0,0,139},
    {0,0,143},
    {0,0,147},
    {0,0,151},
    {0,0,155},
    {0,0,159},
    {0,0,163},
    {0,0,167},
    {0,0,171},
    {0,0,175},
    {0,0,179},
    {0,0,183},
    {0,0,187},
    {0,0,191},
    {0,0,195},
    {0,0,199},
    {0,0,203},
    {0,0,207},
    {0,0,211},
    {0,0,215},
    {0,0,219},
    {0,0,223},
    {0,0,227},
    {0,0,231},
    {0,0,235},
    {0,0,239},
    {0,0,243},
    {0,0,247},
    {0,0,251},
    {0,0,255},
    {0,3,255},
    {0,7,255},
    {0,11,255},
    {0,15,255},
    {0,19,255},
    {0,23,255},
    {0,27,255},
    {0,31,255},
    {0,35,255},
    {0,39,255},
    {0,43,255},
    {0,47,255},
    {0,51,255},
    {0,55,255},
    {0,59,255},
    {0,63,255},
    {0,67,255},
    {0,71,255},
    {0,75,255},
    {0,79,255},
    {0,83,255},
    {0,87,255},
    {0,91,255},
    {0,95,255},
    {0,99,255},
    {0,103,255},
    {0,107,255},
    {0,111,255},
    {0,115,255},
    {0,119,255},
    {0,123,255},
    {0,127,255},
    {0,131,255},
    {0,135,255},
    {0,139,255},
    {0,143,255},
    {0,147,255},
    {0,151,255},
    {0,155,255},
    {0,159,255},
    {0,163,255},
    {0,167,255},
    {0,171,255},
    {0,175,255},
    {0,179,255},
    {0,183,255},
    {0,187,255},
    {0,191,255},
    {0,195,255},
    {0,199,255},
    {0,203,255},
    {0,207,255},
    {0,211,255},
    {0,215,255},
    {0,219,255},
    {0,223,255},
    {0,227,255},
    {0,231,255},
    {0,235,255},
    {0,239,255},
    {0,243,255},
    {0,247,255},
    {0,251,255},
    {0,255,255},
    {3,255,251},
    {7,255,247},
    {11,255,243},
    {15,255,239},
    {19,255,235},
    {23,255,231},
    {27,255,227},
    {31,255,223},
    {35,255,219},
    {39,255,215},
    {43,255,211},
    {47,255,207},
    {51,255,203},
    {55,255,199},
    {59,255,195},
    {63,255,191},
    {67,255,187},
    {71,255,183},
    {75,255,179},
    {79,255,175},
    {83,255,171},
    {87,255,167},
    {91,255,163},
    {95,255,159},
    {99,255,155},
    {103,255,151},
    {107,255,147},
    {111,255,143},
    {115,255,139},
    {119,255,135},
    {123,255,131},
    {127,255,127},
    {131,255,123},
    {135,255,119},
    {139,255,115},
    {143,255,111},
    {147,255,107},
    {151,255,103},
    {155,255,99},
    {159,255,95},
    {163,255,91},
    {167,255,87},
    {171,255,83},
    {175,255,79},
    {179,255,75},
    {183,255,71},
    {187,255,67},
    {191,255,63},
    {195,255,59},
    {199,255,55},
    {203,255,51},
    {207,255,47},
    {211,255,43},
    {215,255,39},
    {219,255,35},
    {223,255,31},
    {227,255,27},
    {231,255,23},
    {235,255,19},
    {239,255,15},
    {243,255,11},
    {247,255,7},
    {251,255,3},
    {255,255,0},
    {255,251,0},
    {255,247,0},
    {255,243,0},
    {255,239,0},
    {255,235,0},
    {255,231,0},
    {255,227,0},
    {255,223,0},
    {255,219,0},
    {255,215,0},
    {255,211,0},
    {255,207,0},
    {255,203,0},
    {255,199,0},
    {255,195,0},
    {255,191,0},
    {255,187,0},
    {255,183,0},
    {255,179,0},
    {255,175,0},
    {255,171,0},
    {255,167,0},
    {255,163,0},
    {255,159,0},
    {255,155,0},
    {255,151,0},
    {255,147,0},
    {255,143,0},
    {255,139,0},
    {255,135,0},
    {255,131,0},
    {255,127,0},
    {255,123,0},
    {255,119,0},
    {255,115,0},
    {255,111,0},
    {255,107,0},
    {255,103,0},
    {255,99,0},
    {255,95,0},
    {255,91,0},
    {255,87,0},
    {255,83,0},
    {255,79,0},
    {255,75,0},
    {255,71,0},
    {255,67,0},
    {255,63,0},
    {255,59,0},
    {255,55,0},
    {255,51,0},
    {255,47,0},
    {255,43,0},
    {255,39,0},
    {255,35,0},
    {255,31,0},
    {255,27,0},
    {255,23,0},
    {255,19,0},
    {255,15,0},
    {255,11,0},
    {255,7,0},
    {255,3,0},
    {255,0,0},
    {251,0,0},
    {247,0,0},
    {243,0,0},
    {239,0,0},
    {235,0,0},
    {231,0,0},
    {227,0,0},
    {223,0,0},
    {219,0,0},
    {215,0,0},
    {211,0,0},
    {207,0,0},
    {203,0,0},
    {199,0,0},
    {195,0,0},
    {191,0,0},
    {187,0,0},
    {183,0,0},
    {179,0,0},
    {175,0,0},
    {171,0,0},
    {167,0,0},
    {163,0,0},
    {159,0,0},
    {155,0,0},
    {151,0,0},
    {147,0,0},
    {143,0,0},
    {139,0,0},
    {135,0,0},
    {131,0,0},
    {127,0,0} //275
        };//////////////////////////////////////////////////////////////////////////////////
const int neuron_type_color_num = sizeof(neuron_type_color)/(sizeof(GLubyte)*3);
//


void ThreadSleep( unsigned long nMilliseconds )
{
#if defined(_WIN32)
	::Sleep( nMilliseconds );
#elif defined(POSIX)
	usleep( nMilliseconds * 1000 );
#endif
}


static bool g_bPrintf = true;



//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf_s( buffer, fmt, args );
	va_end( args );

	if ( g_bPrintf )
		printf( "%s", buffer );

	OutputDebugStringA( buffer );
}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication( int argc, char *argv[] )
	: m_pCompanionWindow(NULL)
	, m_pContext(NULL)
	, m_nCompanionWindowWidth(3200)//( 1600 )//(640)//
	, m_nCompanionWindowHeight(1600)//(800)//( 320 )//
	, morphologyShader ( NULL )
	, raycastingShader ( NULL )
	, clipPatchShader (NULL)
	, backfaceShader ( NULL )
	, m_unCompanionWindowProgramID( 0 )
	, m_unControllerTransformProgramID( 0 )
	, m_unRenderModelProgramID( 0 )
	, m_unControllerRayProgramID( 0 )
	, m_pHMD( NULL )
	, m_pRenderModels( NULL )
	, m_pChaperone( NULL )
	, m_bDebugOpenGL( false )
	, m_bVerbose( false )
	, m_bPerf( false )
	, m_bVblank( false )
	, m_bGlFinishHack( true )
	, m_glControllerVertBuffer( 0 )
	, m_unControllerVAO( 0 )
	, m_unMorphologyLineModeVAO( 0 )
	, m_uiMorphologyLineModeVertcount(0)
	, m_unSketchMorphologyLineModeVAO( 0 )
	, m_uiSketchMorphologyLineModeVertcount(0)
	, m_VolumeImageVAO (0)
	, m_clipPatchVAO (0)
	, m_nControllerMatrixLocation( -1 )
	, m_nControllerRayMatrixLocation( -1 )
	, m_nRenderModelMatrixLocation( -1 )
	, m_iTrackedControllerCount( 0 )
	, m_iTrackedControllerCount_Last( -1 )
	, m_iValidPoseCount( 0 )
	, m_iValidPoseCount_Last( -1 )
	, m_strPoseClasses("")
	, m_bShowMorphologyLine(true)
	, m_bShowMorphologySurface(false)
	, m_bShowMorphologyMarker(true)
	, m_bControllerModelON(true)
	, m_modeControlTouchPad_R(0)
	, m_modeControlGrip_R(0)
	, m_contrastMode (true)//right touch_pad only control contrast
	, m_rotateMode (false)
	, m_zoomMode (false)
	, m_autoRotateON (false)
	, m_TouchFirst (true)
	, m_fTouchOldX( 0 )
	, m_fTouchOldY( 0 )
	, m_pickUpState(false)
	, pick_point_index_A (-1)
	, pick_point_index_B (-1)
	, m_ControllerTexVAO( 0 )
	, m_iControllerRayVAO( 0 )
	, m_nCtrTexMatrixLocation( -1 )
	, m_unCtrTexProgramID( 0 )
	, m_bHasImage4D( false)
	, mainwindow(NULL)
	, img4d(NULL)
	, READY_TO_SEND(false)
	, sketchNum(0)
	, bIsUndoEnable (false)
	, bIsRedoEnable (false)
	, _call_assemble_plugin(false)
	, _startdragnode(false)
	, postVRFunctionCallMode (0)
	, curveDrawingTestStatus (-1)
	, showshootingray(false)
	, replacetexture(false)
	//, font_VR (NULL)

{
	leftEyeDesc.m_nDepthBufferId = leftEyeDesc.m_nRenderFramebufferId = leftEyeDesc.m_nRenderTextureId = leftEyeDesc.m_nResolveFramebufferId = leftEyeDesc.m_nResolveTextureId = 0;
	rightEyeDesc.m_nDepthBufferId = rightEyeDesc.m_nRenderFramebufferId = rightEyeDesc.m_nRenderTextureId = rightEyeDesc.m_nResolveFramebufferId = rightEyeDesc.m_nResolveTextureId = 0;

	for( int i = 1; i < argc; i++ )
	{
		if( !stricmp( argv[i], "-gldebug" ) )
		{
			m_bDebugOpenGL = true;
		}
		else if( !stricmp( argv[i], "-verbose" ) )
		{
			m_bVerbose = true;
		}
		else if( !stricmp( argv[i], "-novblank" ) )
		{
			m_bVblank = false;
		}
		else if( !stricmp( argv[i], "-noglfinishhack" ) )
		{
			m_bGlFinishHack = false;
		}
		else if( !stricmp( argv[i], "-noprintf" ) )
		{
			g_bPrintf = false;
		}
	}
	// other initialization tasks are done in BInit
	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	dprintf( "Shutdown" );
	this->mainwindow->show();
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[ unRequiredBufferLen ];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete [] pchBuffer;
	return sResult;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 )
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

	if ( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}



	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );
	//question: difference between IVRRenderModels and the CGLRenderModel?
	//is IVRRenderModels necessary for VR? comment it and observe the change
	if( !m_pRenderModels )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}
	m_pChaperone = (vr::IVRChaperone *)vr::VR_GetGenericInterface( vr::IVRChaperone_Version, &eError );
	//question: difference between IVRRenderModels and the CGLRenderModel?
	//is IVRRenderModels necessary for VR? comment it and observe the change
	if( !m_pChaperone )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to get chaper interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}


	int nWindowPosX = 100;
	int nWindowPosY = 100;
	//fps test liqi
	 countsPerSecond = 0.0;
	 CounterStart = 0;

	 frameCount = 0;
	 fps = 0;

	 frameTimeOld = 0;
	 frameTime;

	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	/*m_flashtype = noflash;
	m_Flashcolor = 0;
	m_FlashCount = 0;*/
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );
	if( m_bDebugOpenGL )
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );






	m_pCompanionWindow = SDL_CreateWindow( "TeraVR", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags );
	if (m_pCompanionWindow == NULL)
	{
		printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
	if (m_pContext == NULL)
	{
		printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf( "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString( nGlewError ) );
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if ( SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0 )
	{
		printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
	m_strDisplay = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

	//std::string strWindowTitle = "Vaa3D VR - " + m_strDriver + " " + m_strDisplay;
	std::string strWindowTitle = "TeraVR - Initializing";
	SDL_SetWindowTitle( m_pCompanionWindow, strWindowTitle.c_str() );

	
	m_fNearClip = 0.1f;

 	m_fFarClip = 30.0f;
	m_iTexture = 0;
	m_uiControllerTexIndexSize = 0;
	m_oldGlobalMatrix = m_ctrlChangeMatrix = m_oldCtrlMatrix= glm::mat4();
	//m_oldGlobalMatrix = m_ctrlChangeMatrix = m_oldCtrlMatrix= glm::mat4();

	// m_modeGrip_R = global_padm_modeGrip_R;
	// m_modeGrip_L = global_padm_modeGrip_L;
	delName = "";
	dragnodePOS="";
	loadedNTCenter = glm::vec3(0);
	vertexcount = swccount = 0;
	pick_node = 0;

	fSlabwidth = 0.05;
	
	if (!BInitGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!BInitCompositor())
	{
		printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
		return false;
	}

	//if(font_VR)
	//	delete font_VR;
	//font_VR = 0;
	//// init font
	//font_VR= new gltext::Font("data/DroidSans.ttf", 32, 512, 512);
	//// specify the screen size for perfect pixel rendering
	//font_VR->setDisplaySize(m_nCompanionWindowWidth, m_nCompanionWindowHeight);
	//// NOTE: cache is hard to use properly; you need to calculate the right cache size in the above gltext::Font() constructor
	//font_VR->cacheCharacters("1234567890!@#$%^&*()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,./;'[]\\<>?:\"{}|-=_+");


	ctrSphere = new Sphere(0.01f);
	ctrSphereColor = glm::vec3();
	int color_id = (m_curMarkerColorType>=0 && m_curMarkerColorType<neuron_type_color_num)? m_curMarkerColorType : 0;
	ctrSphereColor[0] =  neuron_type_color[color_id][0] /255.0;
	ctrSphereColor[1] =  neuron_type_color[color_id][1] /255.0;
	ctrSphereColor[2] =  neuron_type_color[color_id][2] /255.0;

	u_clipnormal = glm::vec3(0,-1.0,0);
	u_clippoint = glm::vec3(0.0,0.0,2.0);
	teraflyPOS = 0;
	CollaborationCreatorPos = 0;
	SDL_StartTextInput();
	SDL_ShowCursor( SDL_DISABLE );
	currentNT.listNeuron.clear();
	currentNT.hashNeuron.clear();

	tempNT.listNeuron.clear();
	tempNT.hashNeuron.clear();
	return true;
}

//*/
//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	dprintf( "GL Error: %s\n", message );
}


//-----------------------------------------------------------------------------
// Purpose: Initialize OpenGL. Returns true if OpenGL has been successfully
//          initialized, false if shaders could not be created.
//          If failure occurred in a module other than shaders, the function
//          may return true or throw an error. 
//-----------------------------------------------------------------------------
bool CMainApplication::BInitGL()
{
	if( m_bDebugOpenGL )
	{
		glDebugMessageCallback( (GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
	if( !CreateAllShaders() )
		return false;
	loadedNT_merged.listNeuron.clear();
	loadedNT_merged.hashNeuron.clear();

	//merge all loaded Neuron (as non-editable Neuron) into one single NeuronTree ,then display
	qDebug()<<"loadedNTList->size()"<<loadedNTList->size()<<"  editableNTL size is"<<this->editableLoadedNTL.size()<<" non editable NTL size is "<<this->nonEditableLoadedNTL.size();
	 if( nonEditableLoadedNTL.size()>0) 
	 	loadedNT_merged =  nonEditableLoadedNTL.at(0); //because the first loaded NeuronTree may be not in order, so just push it into loadedNT_merged
	 if( nonEditableLoadedNTL.size()>1)
	 {
	 	for(int i=1;i< nonEditableLoadedNTL.size();i++)
	 	{
	 		NTL curNTL;
	 		NeuronTree curNT =  nonEditableLoadedNTL.at(i);
	 		curNTL.push_back(curNT);
	 		qDebug()<<"curNTL->size()"<<curNTL.size();
	 		MergeNeuronTrees(loadedNT_merged,&curNTL);
	 		curNTL.clear();
	 	}
	 }

	//convert vaa3d_traced_neuron (as editable neuron) to V_NeuronSWC_list with all segments
	for(int j=0;j<editableLoadedNTL.size();j++)
	{
		NeuronTree SingleTree = editableLoadedNTL.at(j);
		V_NeuronSWC_list testVNL = NeuronTree__2__V_NeuronSWC_list(SingleTree);
		for(int i= 0;i<testVNL.seg.size();i++)
		{
			NeuronTree SS;
			//convert each segment to NeuronTree with one single path
			V_NeuronSWC seg_temp =  testVNL.seg.at(i);
			seg_temp.reverse();
			SS = V_NeuronSWC__2__NeuronTree(seg_temp);
			//append to editable sketchedNTList
			SS.name = "loaded_" + QString("%1").arg(i);
			if (SS.listNeuron.size()>0)
				sketchedNTList.push_back(SS);
		}
	}
	// set up each NT in sketchedNTList morphology line VAO&VBO
	SetupAllMorphologyLine();

	SetupGlobalMatrix();
	
	SetupMorphologyLine(0);
	//SetupMorphologyLine(2);
	//SetupMorphologyLine(loadedNT_merged,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,0);
	//SetupMorphologySurface(loadedNT_merged,loaded_spheres,loaded_cylinders,loaded_spheresPos);

	SetupTexturemaps();
	SetupCameras();
	SetupCamerasForMorphology();
	SetupStereoRenderTargets();

	if (m_bHasImage4D) 
	{

		SetupVolumeRendering();

		QList <LocationSimple> & listLoc = img4d->listLandmarks;
		qDebug()<<"Init! NOW listLandmarks.size is "<<img4d->listLandmarks.size();
		drawnMarkerList.clear();
		markerVisibility.clear();
		for(int i=0;i<listLoc.size();i++)
		{
			LocationSimple S_tmp = listLoc.at(i);
			SetupMarkerandSurface(S_tmp.x,S_tmp.y,S_tmp.z,S_tmp.color.r,S_tmp.color.g,S_tmp.color.b);
		}
	}
	SetupCompanionWindow();

	SetupRenderModels();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
bool CMainApplication::BInitCompositor()//note: VRCompositor is responsible for getting pose and submitting framebuffers
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if ( !vr::VRCompositor() )
	{
		printf( "Compositor initialization failed. See log file for details\n" );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	// float trans_x = 0.6 ;
	// float trans_y = 1.5 ;
	// float trans_z = 0.4 ;
	
	//remeber last Grip choice
	// global_padm_modeGrip_L = m_modeGrip_L;
	// global_padm_modeGrip_R = m_modeGrip_R;
	//reverse the normalization (so that the next VR session can normalize correctly)
	m_globalMatrix = glm::translate(m_globalMatrix,glm::vec3(loadedNTCenter.x,loadedNTCenter.y,loadedNTCenter.z) ); 
	m_globalMatrix = glm::scale(m_globalMatrix,glm::vec3(1.0f/m_globalScale,1.0f/m_globalScale,1.0f/m_globalScale));
	//m_globalMatrix = glm::translate(m_globalMatrix,glm::vec3(-trans_x,-trans_y,-trans_z) ); //fine tune
	m_globalMatrix = glm::translate(m_globalMatrix,glm::vec3(- HmdQuadImageOffset.v[0],- HmdQuadImageOffset.v[1],- HmdQuadImageOffset.v[2]));
	//replace "vaa3d_traced_neuron" with VR_drawn_curves
	////////update glwidget->listneurontree
	if(m_bHasImage4D&&(sketchedNTList.size()>0))
	{
		NeuronTree SS;
		SS.n = -1;
		SS.on = true;
		SS.name = "vaa3d_traced_neuron";
		SS.file = "vaa3d_traced_neuron";
		SS.color = XYZW(0,0,0,0);
		// add or replace into listNeuronTree
		bool contained = false;
		for (int i=0; i<loadedNTList->size(); i++)
		{
			if (SS.file == loadedNTList->at(i).file) // same file to replace it
			{
				contained = true;
				SS.n = 1+i;
				//SS = loadedNTList->at(i);
				MergeNeuronTrees(SS,&sketchedNTList);
				loadedNTList->replace(i, SS); 
				break;
			}
		}
		if (!contained ) 
		{
			// no this file then add it
			SS.n = 1+loadedNTList->size();
			MergeNeuronTrees(SS,&sketchedNTList);
			loadedNTList->append(SS);
		}
		////////update image4d->tracedneuron
		img4d->tracedNeuron_old = img4d->tracedNeuron; 
		img4d->tracedNeuron = NeuronTree__2__V_NeuronSWC_list(SS);
		img4d->tracedNeuron.name = "vaa3d_traced_neuron";
		img4d->tracedNeuron.file = "vaa3d_traced_neuron";
		img4d->proj_trace_history_append();
		// img4d->update_3drenderer_neuron_view();

	}
	//if(drawnMarkerList.size()>0)//In all cases ,we should copy markerlist into img4d,liqi 2018_12_9
		if (m_bHasImage4D)
		{
			img4d->listLandmarks.clear();
			for(int i=0;i<drawnMarkerList.size();i++)
			{
				ImageMarker mk = drawnMarkerList.at(i);
				LocationSimple S_tmp = LocationSimple(mk.x,mk.y,mk.z);
				S_tmp.color = mk.color;
				img4d->listLandmarks.append(S_tmp);		
			}
		}

	bIsUndoEnable = false;
	bIsRedoEnable = false;
	vUndoList.clear();
	vRedoList.clear();

	if( m_pHMD ) 
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		delete (*i);
	}
	m_vecRenderModels.clear();

	if( m_pContext )
	{
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE );
		glDebugMessageCallback(nullptr, nullptr);

		if (morphologyShader != NULL)
		{
			delete morphologyShader;
			morphologyShader =NULL;
		}
		if (raycastingShader != NULL)
		{
			delete raycastingShader;
			raycastingShader =NULL;
		}
		if (backfaceShader != NULL)
		{
			delete backfaceShader;
			backfaceShader =NULL;
		}
		if (clipPatchShader != NULL)
		{
			delete clipPatchShader;
			clipPatchShader =NULL;
		}
		if ( m_unControllerTransformProgramID )
		{
			glDeleteProgram( m_unControllerTransformProgramID );
		}
		if ( m_unRenderModelProgramID )
		{
			glDeleteProgram( m_unRenderModelProgramID );
		}
		if ( m_unCompanionWindowProgramID )
		{
			glDeleteProgram( m_unCompanionWindowProgramID );
		}
		if ( m_unControllerRayProgramID )
		{
			glDeleteProgram( m_unControllerRayProgramID );
		}

		glDeleteRenderbuffers( 1, &leftEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nResolveFramebufferId );

		glDeleteRenderbuffers( 1, &rightEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nResolveFramebufferId );

		glDeleteFramebuffers( 1, &g_frameBufferBackface );
		glDeleteTextures( 1, &g_tffTexObj );
		glDeleteTextures( 1, &g_bfTexObj );
		glDeleteTextures( 1, &g_volTexObj );

		if(m_ControllerTexVAO!=0)
		{
			glDeleteVertexArrays(1, &m_ControllerTexVAO);
			glDeleteBuffers(1, &m_ControllerTexVBO);
		}

		if(m_iTexture!=0)
			glDeleteTextures(1, &m_iTexture );

		if ( m_unCtrTexProgramID )
		{
			glDeleteProgram( m_unCtrTexProgramID );
		}

		
		if (iSketchNTLMorphologyVAO.size()>0)
		{
			for(int i=0;i<iSketchNTLMorphologyVAO.size();i++)
			{
				glDeleteVertexArrays( 1, &iSketchNTLMorphologyVAO.at(i) );
				glDeleteBuffers(1, &iSketchNTLMorphologyVertBuffer.at(i));
				glDeleteBuffers(1, &iSketchNTLMorphologyIndexBuffer.at(i));
			}
			iSketchNTLMorphologyVAO.clear();
			iSketchNTLMorphologyVertBuffer.clear();
			iSketchNTLMorphologyIndexBuffer.clear();
			iSketchNTLMorphologyVertcount.clear();
		}

		//for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
		//{
		//	delete (*i);
		//}
		qDebug()<<"Start to delete sphere....";
		int j;
		j=0;
		for(auto i = loaded_spheres.begin();i!=loaded_spheres.end();i++) 
		{
				delete (*i);
				if(j%500==0) qDebug()<<"now delete 500 *"<<j;
				j++;
		}
		j=0;
		for(auto i = loaded_cylinders.begin();i!=loaded_cylinders.end();i++) 
		{
				delete (*i);
				if(j%500==0) qDebug()<<"now delete 500 *"<<j;
				j++;
		}
		j=0;
		for(auto i = sketch_spheres.begin();i!=sketch_spheres.end();i++) 
		{
				delete (*i);
				if(j%500==0) qDebug()<<"now delete 500 *"<<j;
				j++;
		}
		j=0;
		for(auto i = sketch_cylinders.begin();i!=sketch_cylinders.end();i++)
		{
				delete (*i);
				if(j%500==0) qDebug()<<"now delete 500 *"<<j;
				j++;
		}
		j=0;
		for(auto i = Agents_spheres.begin();i!=Agents_spheres.end();i++)
		{
				delete (*i);
				if(j%500==0) qDebug()<<"now delete 500 *"<<j;
				j++;
		}
		j=0;
		for(auto i = Markers_spheres.begin();i!=Markers_spheres.end();i++) 
		{
				delete (*i);
				if(j%500==0) qDebug()<<"now delete 500 *"<<j;
				j++;
		}

		if(ctrSphere)
			delete ctrSphere;

			
		//for (int i=0;i<loaded_spheres.size();i++) delete loaded_spheres[i];
		//for (int i=0;i<loaded_cylinders.size();i++) delete loaded_cylinders[i];
		//for (int i=0;i<sketch_spheres.size();i++) delete sketch_spheres[i];
		//for (int i=0;i<sketch_cylinders.size();i++) delete sketch_cylinders[i];

		//for (int i=0;i<Agents_spheres.size();i++) delete Agents_spheres[i];

		//for (int i=0;i<Markers_spheres.size();i++) delete Markers_spheres[i];

		loaded_spheres.clear();
		loaded_spheresPos.clear();
		loaded_spheresColor.clear();
		loaded_cylinders.clear();
		sketch_spheres.clear();
		sketch_spheresPos.clear();
		sketch_cylinders.clear();

		Agents_spheres.clear();
		Agents_spheresPos.clear();
		Agents_spheresColor.clear();

		Markers_spheres.clear();
		Markers_spheresPos.clear();
		Markers_spheresColor.clear();

		sketchedNTList.clear();
		drawnMarkerList.clear();
		markerVisibility.clear();

		if(pick_node)  pick_node = 0;
		if( m_unMorphologyLineModeVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unMorphologyLineModeVAO );
			glDeleteBuffers(1, &m_glMorphologyLineModeVertBuffer);
			glDeleteBuffers(1, &m_glMorphologyLineModeIndexBuffer);
		}
		
		if( m_unSketchMorphologyLineModeVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unSketchMorphologyLineModeVAO );
			glDeleteBuffers(1, &m_glSketchMorphologyLineModeVertBuffer);
			glDeleteBuffers(1, &m_glSketchMorphologyLineModeIndexBuffer);
		}

		if( m_VolumeImageVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_VolumeImageVAO );
			//glDeleteBuffers(1, &m_imageVBO);
		}

		if( m_clipPatchVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_clipPatchVAO );
			//glDeleteBuffers(1, &m_imageVBO);
		}

		if( m_unCompanionWindowVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unCompanionWindowVAO );
			glDeleteBuffers(1, &m_glCompanionWindowIDVertBuffer);
			glDeleteBuffers(1, &m_glCompanionWindowIDIndexBuffer);
		}
		if( m_unControllerVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unControllerVAO );
			glDeleteBuffers(1, &m_glControllerVertBuffer);
		} 
	}

	if( m_pCompanionWindow )
	{
	//	if(font_VR->self)
	//	{
	//		delete font_VR->self;
	//		qDebug()<<"delete self ";
	//		font_VR->self = 0;
	//	}
	//	delete font_VR;
	//	qDebug()<<"delete font ";
	//	font_VR = NULL;
	//	qDebug()<<"deleted font of VR";
		SDL_DestroyWindow(m_pCompanionWindow);
		m_pCompanionWindow = NULL;
	}
	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	while ( SDL_PollEvent( &sdlEvent ) != 0 )
	{
		if ( sdlEvent.type == SDL_QUIT )
		{
			bRet = true;
		}
		else if ( sdlEvent.type == SDL_KEYDOWN )
		{
			if ( sdlEvent.key.keysym.sym == SDLK_ESCAPE 
			     || sdlEvent.key.keysym.sym == SDLK_q )
			{
				bRet = true;
			}
			if( sdlEvent.key.keysym.sym == SDLK_c )

			{
				m_bShowMorphologyLine = !m_bShowMorphologyLine;
				m_bShowMorphologySurface = !m_bShowMorphologySurface;
				m_bShowMorphologyMarker = !m_bShowMorphologyMarker;
			}
		}
	}//*/

	m_iControllerIDLeft = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);	
	m_iControllerIDRight = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

	//qDebug("Left=%d, Right= %d\n",m_iControllerIDLeft,m_iControllerIDRight);
	// Process SteamVR events
	vr::VREvent_t event;
	while( m_pHMD->PollNextEvent( &event, sizeof( event ) ) )
	{
		ProcessVREvent( event );
		// if(_call_assemble_plugin)
		// {
		// 	qDebug()<<"run here";
		// 	return true;
		// }
		if(postVRFunctionCallMode!=0)
		{
			qDebug()<<"there is a post-VR function call "<<postVRFunctionCallMode;
			return true;
		}
		if((event.trackedDeviceIndex==m_iControllerIDLeft)&&(event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_ApplicationMenu))
		{
			bRet = true;
			return bRet;
		}
	}


	// Process SteamVR RIGHT controller state 
	//including draw lines
	{
		vr::VRControllerState_t state;		
		//if( (unDevice==m_iControllerIDRight)&&(m_pHMD->GetControllerState( unDevice, &state, sizeof(state) ) ))
		if( m_pHMD->GetControllerState( m_iControllerIDRight, &state, sizeof(state) ) )
		{
			if(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)&&(!showshootingray))
			{
				if(m_modeGrip_R==m_drawMode)
				{

					//this part is for building a neuron tree to further save as SWC file
					if (vertexcount%drawing_step_size ==0)//use vertexcount to control point counts in a single line 
					//each #drawing_step_size frames render a SWC node
					{
						const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos 
						glm::mat4 mat = glm::mat4();
						for (size_t i = 0; i < 4; i++)
						{
							for (size_t j = 0; j < 4; j++)
							{
								mat[i][j] = *(mat_M.get() + i * 4 + j);
							}
						}
						mat=glm::inverse(m_globalMatrix) * mat;
						glm::vec4 m_v4DevicePose = mat * glm::vec4( 0, 0, 0, 1 );//change the world space(with the globalMatrix) to the initial world space

						swccount++;//for every 10(#drawing_step_size) frames we store a point as a neuronswc point,control with swccount
						NeuronSWC SL0;
						SL0.x = m_v4DevicePose.x ;
						SL0.y = m_v4DevicePose.y ;
						SL0.z = m_v4DevicePose.z ;
						SL0.r = default_radius; //set default radius 1
						SL0.type = m_curMarkerColorType;//set default type 11:ice
						SL0.n = currentNT.listNeuron.size()+1;
						SL0.creatmode = 618;
						if((swccount==1)||(vertexcount == 0))
						{
							SL0.pn = -1;//for the first point of each lines , it's parent must be -1	
						}
						else
						{
							SL0.pn = currentNT.listNeuron.at(SL0.n-1-1).n;//for the others, their parent should be the last one
						}
						currentNT.listNeuron.append(SL0);
						currentNT.hashNeuron.insert(SL0.n, currentNT.listNeuron.size()-1);//store NeuronSWC SL0 into currentNT
						if(img4d&&m_bVirtualFingerON) //if an image exist, call virtual finger functions for curve drawing
						{
							// for each 10 nodes/points, call virtual finger once aiming to see the line's mid-result
							if((currentNT.listNeuron.size()>0)&&(currentNT.listNeuron.size()%10==0))
							{	
								qDebug()<<"size%10==0 goto charge isAnyNodeOutBBox";
								tempNT.listNeuron.clear();
								tempNT.hashNeuron.clear();
								for(int i=0;i<currentNT.listNeuron.size();i++)
								{
									NeuronSWC S_node = currentNT.listNeuron.at(i);//swcBB
									if(!isAnyNodeOutBBox(S_node))
									{
										S_node.n=tempNT.listNeuron.size();
										if(S_node.pn!=-1)
											S_node.pn = tempNT.listNeuron.last().n;
										tempNT.listNeuron.append(S_node);
										tempNT.hashNeuron.insert(S_node.n, tempNT.listNeuron.size()-1);
									}
									else if(i==0)
									{
										vertexcount=swccount=0;
										break;
									}
										
								}
								qDebug()<<"charge isAnyNodeOutBBox done  goto virtual finger";
								// improve curve shape
								NeuronTree InputNT;
								InputNT = tempNT;
								int iter_number=1;
								//for(int i=0;i<iter_number;i++)  // liqi  Find matching line segments more than once, resulting in inaccurate results
								//{
								//	NeuronTree OutputNT;
								//	RefineSketchCurve(i%3,InputNT, OutputNT); //ver. 2b
								//	//convergent = CompareDist(InputNT, OutputNT);
								//	InputNT.listNeuron.clear();
								//	InputNT.hashNeuron.clear();
								//	InputNT = OutputNT;
								//}
								currentNT.listNeuron.clear();
								currentNT.hashNeuron.clear();
								currentNT = InputNT;
								tempNT.listNeuron.clear();
								tempNT.hashNeuron.clear();	
								qDebug()<<"virtual finger done  goto next frame";
							}
						}
					}	
					vertexcount++;
				}
				else if(m_modeGrip_R==m_dragMode) 
				{
					const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos
					glm::mat4 mat = glm::mat4();
					for (size_t i = 0; i < 4; i++)
					{
						for (size_t j = 0; j < 4; j++)
						{
							mat[i][j] = *(mat_M.get() + i * 4 + j);
						}
					}
					mat=glm::inverse(m_globalMatrix) * mat;
					glm::vec4 ctrlRightPos = mat * glm::vec4( 0, 0, 0, 1 );
					//qDebug("ctrlRightPos = %.2f,%.2f,%.2f\n",ctrlRightPos.x,ctrlRightPos.y,ctrlRightPos.z);
					if(!_startdragnode)
					{
						if (isOnline == false)
						{
							bIsUndoEnable = true;
							if (vUndoList.size() == MAX_UNDO_COUNT)
							{
								vUndoList.erase(vUndoList.begin());
							}
							vUndoList.push_back(sketchedNTList);
							if (vRedoList.size() > 0)
								vRedoList.clear();
							bIsRedoEnable = false;
							vRedoList.clear();
						}
					}
					_startdragnode = true;
					if(m_pickUpState == false)
					{
						float dist = 0;
						float minvalue = 10.f;
						for(int i=0;i<sketchedNTList.size();i++)
						{
							NeuronTree nt = sketchedNTList.at(i);
							for(int j=0;j<nt.listNeuron.size();j++)
							{
								NeuronSWC SS0;
								SS0 = nt.listNeuron.at(j);
								dist = glm::sqrt((ctrlRightPos.x-SS0.x)*(ctrlRightPos.x-SS0.x)+(ctrlRightPos.y-SS0.y)*(ctrlRightPos.y-SS0.y)+(ctrlRightPos.z-SS0.z)*(ctrlRightPos.z-SS0.z));
								//qDebug("SS0 = %.2f,%.2f,%.2f\n",SS0.x,SS0.y,SS0.z);
								if(dist > (dist_thres/m_globalScale*5))
									continue;
								minvalue = glm::min(minvalue,dist);
								if(minvalue==dist)
								{
									pick_node = &sketchedNTList[i].listNeuron[j];
									pick_point_index_A = i;
									pick_point_index_B = j;
								}
							}

						}
						if(pick_point_index_B!=-1){
							m_pickUpState = true;
							qDebug("pick up %d, %d point.",pick_point_index_A,pick_point_index_B);}
					}
					else if(pick_point_index_B!=-1)//when pick up a node
					{
						dragnodePOS="";
						dragnodePOS = QString("%1 %2").arg(pick_point_index_A).arg(pick_point_index_B);
						dragnodePOS += QString(" %1 %2 %3").arg(ctrlRightPos.x).arg(ctrlRightPos.y).arg(ctrlRightPos.z);

						pick_node->x = ctrlRightPos.x;
						pick_node->y = ctrlRightPos.y;
						pick_node->z = ctrlRightPos.z;

						SetupSingleMorphologyLine(pick_point_index_A,1);

						if(isOnline)
							pick_node->type = 0;
							// sketchedNT_merged.listNeuron[pick_point].type = 0;
						qDebug()<<"finish update nearest node location";
					}
				}
				else if(m_modeGrip_R==m_ConnectMode)
				{
					//this part is for building a neuron tree to further save as SWC file
					if (vertexcount%drawing_step_size ==0)//use vertexcount to control point counts in a single line 
					//each #drawing_step_size frames render a SWC node
					{
						const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos 
						glm::mat4 mat = glm::mat4();
						for (size_t i = 0; i < 4; i++)
						{
							for (size_t j = 0; j < 4; j++)
							{
								mat[i][j] = *(mat_M.get() + i * 4 + j);
							}
						}
						mat=glm::inverse(m_globalMatrix) * mat;
						glm::vec4 m_v4DevicePose = mat * glm::vec4( 0, 0, 0, 1 );//change the world space(with the globalMatrix) to the initial world space

						swccount++;//for every 10(#drawing_step_size) frames we store a point as a neuronswc point,control with swccount
						NeuronSWC SL0;
						SL0.x = m_v4DevicePose.x ;
						SL0.y = m_v4DevicePose.y ;
						SL0.z = m_v4DevicePose.z ;
						SL0.r = default_radius; //set default radius 1
						SL0.type = m_curMarkerColorType;//set default type 11:ice
						SL0.n = currentNT.listNeuron.size()+1;
						SL0.creatmode = 618;
						if((swccount==1)||(vertexcount == 0))
						{
							SL0.pn = -1;//for the first point of each lines , it's parent must be -1	
						}
						else
						{
							SL0.pn = currentNT.listNeuron.at(SL0.n-1-1).n;//for the others, their parent should be the last one
						}
						currentNT.listNeuron.append(SL0);
						currentNT.hashNeuron.insert(SL0.n, currentNT.listNeuron.size()-1);//store NeuronSWC SL0 into currentNT
						// if(img4d&&m_bVirtualFingerON) //if an image exist, call virtual finger functions for curve drawing
						// {
						// 	// for each 10 nodes/points, call virtual finger once aiming to see the line's mid-result
						// 	if((currentNT.listNeuron.size()>0)&&(currentNT.listNeuron.size()%10==0))
						// 	{	
						// 		qDebug()<<"size%10==0 goto charge isAnyNodeOutBBox";
						// 		tempNT.listNeuron.clear();
						// 		tempNT.hashNeuron.clear();
						// 		for(int i=0;i<currentNT.listNeuron.size();i++)
						// 		{
						// 			NeuronSWC S_node = currentNT.listNeuron.at(i);//swcBB
						// 			if(!isAnyNodeOutBBox(S_node))
						// 			{
						// 				S_node.n=tempNT.listNeuron.size();
						// 				if(S_node.pn!=-1)
						// 					S_node.pn = tempNT.listNeuron.last().n;
						// 				tempNT.listNeuron.append(S_node);
						// 				tempNT.hashNeuron.insert(S_node.n, tempNT.listNeuron.size()-1);
						// 			}
						// 			else if(i==0)
						// 			{
						// 				vertexcount=swccount=0;
						// 				break;
						// 			}
										
						// 		}
						// 		qDebug()<<"charge isAnyNodeOutBBox done  goto virtual finger";
						// 		// improve curve shape
						// 		NeuronTree InputNT;
						// 		InputNT = tempNT;
						// 		int iter_number=1;
						// 		//for(int i=0;i<iter_number;i++)  // liqi  Find matching line segments more than once, resulting in inaccurate results
						// 		//{
						// 		//	NeuronTree OutputNT;
						// 		//	RefineSketchCurve(i%3,InputNT, OutputNT); //ver. 2b
						// 		//	//convergent = CompareDist(InputNT, OutputNT);
						// 		//	InputNT.listNeuron.clear();
						// 		//	InputNT.hashNeuron.clear();
						// 		//	InputNT = OutputNT;
						// 		//}
						// 		currentNT.listNeuron.clear();
						// 		currentNT.hashNeuron.clear();
						// 		currentNT = InputNT;
						// 		tempNT.listNeuron.clear();
						// 		tempNT.hashNeuron.clear();	
						// 		qDebug()<<"virtual finger done  goto next frame";
						// 	}
						// }
					}	
					vertexcount++;
				}
				//else if(m_modeGrip_R==m_slabplaneMode)
				//{
				//	glm::mat4 model;
				//	model = glm::scale(glm::mat4(), glm::vec3(img4d->getXDim(), img4d->getYDim(), img4d->getZDim()));
				//	model = m_globalMatrix * model;
				//	const Matrix4 &mat_M = m_rmat4DevicePose[m_iControllerIDRight]; // mat means current controller pos
				//	glm::mat4 mat = glm::mat4();
				//	for (size_t i = 0; i < 4; i++)
				//	{
				//		for (size_t j = 0; j < 4; j++)
				//		{
				//			mat[i][j] = *(mat_M.get() + i * 4 + j);
				//		}
				//	}

				//	mat = glm::inverse(model) * mat;
				//	glm::vec4 m_v4DevicePose = mat * glm::vec4(0, 0, 0, 1);
				//	mat = glm::inverse(mat);
				//	mat = glm::transpose(mat);
				//	glm::vec4 clipnormal = mat * glm::vec4(0.0, 1.0, 0.0, 1);
				//	u_clipnormal = glm::vec3(clipnormal.x, clipnormal.y, clipnormal.z);
				//	u_clippoint = glm::vec3(m_v4DevicePose.x, m_v4DevicePose.y, m_v4DevicePose.z);
				//}
			}

			if(!(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)))
			{
				m_pickUpState = false;
				pick_point_index_A = -1;
				pick_point_index_B = -1;
			}//whenever the touchpad is unpressed, reset m_pickUpState and pick_point

			//whenever touchpad is unpressed, set bool flag  m_TouchFirst = true;
			if(!(state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)))
			{
				m_TouchFirst=true;
			}//
			//whenever touchpad is pressed, get detX&detY,return to one function according to the mode
			if((state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad))&&
				!(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)))//&&!(showshootingPad))
			{
				float m_fTouchPosY;
				float m_fTouchPosX;
				if(m_TouchFirst==true)
				{//for every first touch,store the axis(x&y) on touchpad as old(means the initial ) POS 
					m_TouchFirst=false;
					m_fTouchOldY = state.rAxis[0].y;
					m_fTouchOldX = state.rAxis[0].x;
					//qDebug("1m_TouchFirst= %d,m_fTouchOldX= %f,m_fTouchOldY= %f.\n",m_TouchFirst,m_fTouchOldX,m_fTouchOldY);
				}
				m_fTouchPosY = state.rAxis[0].y;
				m_fTouchPosX = state.rAxis[0].x;
				//qDebug("2m_TouchFirst= %d,m_fTouchPosXR= %f,m_fTouchPosYR= %f.\n",m_TouchFirst,m_fTouchPosXR,m_fTouchPosYR);
				detX = m_fTouchPosX - m_fTouchOldX;
				detY = m_fTouchPosY - m_fTouchOldY;
				if((detX<0.3f)&&(detX>-0.3f))detX=0;
				if((detY<0.3f)&&(detY>-0.3f))detY=0;
				/*if(detY>1.7||detY<-1.7)
				{
				bRet = true;
				return bRet;
				}//*/
				//if(m_contrastMode==true)//into translate mode
				if(m_modeTouchPad_R == tr_contrast)
				{
					if (m_fTouchPosY > 0)
					{
						fContrast += 0.5;
						if (fContrast > 50)
							fContrast = 50;
						//fBrightness+= 0.01f;
						//if(fBrightness>0.8f)
						//	fBrightness = 0.8f;
					}
					else
					{
						fContrast -= 0.5;
						if (fContrast < 1)
							fContrast = 1;
						//fBrightness-= 0.01f;
						//if(fBrightness<0)
						//	fBrightness = 0;
					}

				}
				else if(m_modeTouchPad_R == tr_clipplane)
				{
					glm::mat4 model;
					model = glm::scale(glm::mat4(), glm::vec3(img4d->getXDim(), img4d->getYDim(), img4d->getZDim()));
					model = m_globalMatrix * model;
					const Matrix4 &mat_M = m_rmat4DevicePose[m_iControllerIDRight]; // mat means current controller pos
					glm::mat4 mat = glm::mat4();
					for (size_t i = 0; i < 4; i++)
					{
						for (size_t j = 0; j < 4; j++)
						{
							mat[i][j] = *(mat_M.get() + i * 4 + j);
						}
					}

					mat = glm::inverse(model) * mat;
					glm::vec4 m_v4DevicePose = mat * glm::vec4(0, 0, 0, 1);
					mat = glm::inverse(mat);
					mat = glm::transpose(mat);
					glm::vec4 clipnormal = mat * glm::vec4(0.0, 0.0, 0.05, 1);
					u_clipnormal = glm::vec3(clipnormal.x, clipnormal.y, clipnormal.z);
					u_clippoint = glm::vec3(m_v4DevicePose.x, m_v4DevicePose.y, m_v4DevicePose.z);
				}
				// else if(0&&m_rotateMode==true)//into ratate mode
				// {
				// 	m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
				// 	m_globalMatrix = glm::rotate(m_globalMatrix,m_fTouchPosX/100,glm::vec3(1,0,0));
				// 	m_globalMatrix = glm::rotate(m_globalMatrix,m_fTouchPosY/100,glm::vec3(0,1,0));
				// 	m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
				// }
				// else if(0&&m_zoomMode==true)//into zoom mode
				// {
				// 	m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
				// 	m_globalMatrix = glm::scale(m_globalMatrix,glm::vec3(1,1,1+m_fTouchPosY/15));
				// 	m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
				// }
			}
		}
	}//
	 //Process SteamVR LEFT controller state
	//inlcuding 
	{
		vr::VRControllerState_t state;
		if( m_pHMD->GetControllerState( m_iControllerIDLeft, &state, sizeof(state)))
		{
			if(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger))
			{
				const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];
				glm::mat4 mat = glm::mat4();
				for (size_t i = 0; i < 4; i++)
				{
					for (size_t j = 0; j < 4; j++)
					{
						mat[i][j] = *(mat_M.get() + i * 4 + j);
					}
				}
				// mat = m_ctrlChangeMatrix * m_oldCtrlMatrix 
				// mat * inverse(m_oldCtrlMatrix) = m_ctrlChangeMatrix
				m_ctrlChangeMatrix = mat * glm::inverse(m_oldCtrlMatrix);
				m_globalMatrix = m_ctrlChangeMatrix * m_oldGlobalMatrix;
			}

		}

	}
	//qDebug()<<"run here 1.02";
	// Process SteamVR LEFT controller state
	//inlcuding translate,rotate and zoom with touchpad; pull a specific point to another POS with trigger
	//{
	//	vr::VRControllerState_t state;

	//		if( m_pHMD->GetControllerState( m_iControllerIDLeft, &state, sizeof(state) ) )
	//		{
	//		//	//whenever touchpad is unpressed, set bool flag  m_TouchFirst = true;
	//		//	if(!(state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)))
	//		//	{
	//		//		m_TouchFirst=true;
	//		//	}//
	//		//	//whenever touchpad is pressed, get detX&detY,return to one function according to the mode
	//		//	if((state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad))&&
	//		//		!(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)))
	//		//	{
	//		//		float m_fTouchPosY;
	//		//		float m_fTouchPosX;
	//		//		if(m_TouchFirst==true)
	//		//		{//for every first touch,store the axis(x&y) on touchpad as old(means the initial ) POS 
	//		//			m_TouchFirst=false;
	//		//			m_fTouchOldY = state.rAxis[0].y;
	//		//			m_fTouchOldX = state.rAxis[0].x;
	//		//			//qDebug("1m_TouchFirst= %d,m_fTouchOldX= %f,m_fTouchOldY= %f.\n",m_TouchFirst,m_fTouchOldX,m_fTouchOldY);
	//		//		}
	//		//		m_fTouchPosY = state.rAxis[0].y;
	//		//		m_fTouchPosX = state.rAxis[0].x;
	//		//		//qDebug("2m_TouchFirst= %d,m_fTouchPosXR= %f,m_fTouchPosYR= %f.\n",m_TouchFirst,m_fTouchPosXR,m_fTouchPosYR);
	//		//		detX = m_fTouchPosX - m_fTouchOldX;
	//		//		detY = m_fTouchPosY - m_fTouchOldY;
	//		//		if((detX<0.3f)&&(detX>-0.3f))detX=0;
	//		//		if((detY<0.3f)&&(detY>-0.3f))detY=0;
	//		//		/*if(detY>1.7||detY<-1.7)
	//		//		{
	//		//			bRet = true;
	//		//			return bRet;
	//		//		}//*/
	//		//		if(m_translationMode==true)//into translate mode
	//		//		{
	//		//			const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];
	//		//			Vector4 direction(0,0,0,1);

	//		//			Vector4 start_Y = mat_M * Vector4( 0, 0, 0, 1 );
	//		//			Vector4 end_Y = mat_M * Vector4( 0, 0, -1.0f, 1 );
	//		//			Vector4 direction_Y = end_Y - start_Y;

	//		//			Vector4 start_X = mat_M * Vector4( 0, 0, 0, 1 );
	//		//			Vector4 end_X = mat_M * Vector4( 1.0f, 0, 0, 1 );
	//		//			Vector4 direction_X = end_X - start_X;

	//		//			if(fabs(m_fTouchPosX) > fabs(m_fTouchPosY)) //move across axis
	//		//			{
	//		//				if(m_fTouchPosX<0) direction = direction_X * -1;
	//		//				else direction = direction_X;
	//		//			} else //move along axis
	//		//			{
	//		//				if(m_fTouchPosY<0) direction = direction_Y * -1;
	//		//				else direction = direction_Y;
	//		//			}
	//		//			direction = direction.normalize() * 0.01;

	//		//			glm::mat4 temp_mat = glm::translate(glm::mat4(),glm::vec3(direction.x,direction.y,direction.z));
	//		//			//glm::mat4 temp_mat = glm::translate(glm::mat4(),glm::vec3(detX/300,0,detY/300));
	//		//			m_globalMatrix = temp_mat * m_globalMatrix;
	//		//		}
	//		//		else if(m_rotateMode==true)//into ratate mode
	//		//		{
	//		//			m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
	//		//			m_globalMatrix = glm::rotate(m_globalMatrix,m_fTouchPosX/300,glm::vec3(1,0,0));
	//		//			m_globalMatrix = glm::rotate(m_globalMatrix,m_fTouchPosY/300,glm::vec3(0,1,0));
	//		//			m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
	//		//		}
	//		//		else if(m_zoomMode==true)//into zoom mode
	//		//		{
	//		//			m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
	//		//			m_globalMatrix = glm::scale(m_globalMatrix,glm::vec3(1+m_fTouchPosY/300,1+m_fTouchPosY/300,1+m_fTouchPosY/300));
	//		//			m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
	//		//		}
	//		//	}

	//			//pick up the nearest node and pull it to new locations
	//			//note: this part of code only serves as a demonstration, and does not handle complicated cases well.
	//			//also, can only pull drawn neurons, not loaded ones.
	//			if(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger))
	//			{
	//				qDebug()<<"start to find nearest node";
	//				const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];// mat means current controller pos
	//				glm::mat4 mat = glm::mat4();
	//				for (size_t i = 0; i < 4; i++)
	//				{
	//					for (size_t j = 0; j < 4; j++)
	//					{
	//						mat[i][j] = *(mat_M.get() + i * 4 + j);
	//					}
	//				}
	//				mat=glm::inverse(m_globalMatrix) * mat;
	//				glm::vec4 ctrlLeftPos = mat * glm::vec4( 0, 0, 0, 1 );
	//				//qDebug("ctrlLeftPos = %.2f,%.2f,%.2f\n",ctrlLeftPos.x,ctrlLeftPos.y,ctrlLeftPos.z);
	//				if(m_pickUpState == false)
	//				{
	//					float dist = 0;
	//					float minvalue = 10.f;
	//					for(int i = 0; i<sketchedNT_merged.listNeuron.size();i++)
	//					{
	//						NeuronSWC SS0;
	//						SS0 = sketchedNT_merged.listNeuron.at(i);
	//						dist = glm::sqrt((ctrlLeftPos.x-SS0.x)*(ctrlLeftPos.x-SS0.x)+(ctrlLeftPos.y-SS0.y)*(ctrlLeftPos.y-SS0.y)+(ctrlLeftPos.z-SS0.z)*(ctrlLeftPos.z-SS0.z));
	//						//qDebug("SS0 = %.2f,%.2f,%.2f\n",SS0.x,SS0.y,SS0.z);
	//						if(dist > (dist_thres/m_globalScale*5))
	//							continue;
	//						minvalue = glm::min(minvalue,dist);
	//						if(minvalue==dist)
	//							pick_point = i;

	//					}
	//					if(pick_point!=-1){
	//						m_pickUpState = true;
	//						qDebug("pick up %d point.",pick_point);}
	//				}
	//				else if(pick_point!=-1)
	//				{
	//					m_pickUpState = true;
	//					NeuronSWC SS1;
	//					SS1 = sketchedNT_merged.listNeuron.at(pick_point);
	//					SS1.x = ctrlLeftPos.x;
	//					SS1.y = ctrlLeftPos.y;
	//					SS1.z = ctrlLeftPos.z;
	//					sketchedNT_merged.listNeuron[pick_point] = SS1;
	//					qDebug()<<"finish update nearest node location";
	//					//sketch_spheresPos[pick_point] = glm::vec3(SS1.x,SS1.y,SS1.z);
	//					//if(SS1.pn!=-1)
	//					//{
	//					//	NeuronSWC SS2 = sketchedNT_merged.listNeuron.at(SS1.pn-1);//SS2 is the parent of SS1
	//					//	//change cylinder state
	//					//	float dist = glm::sqrt((SS2.x-SS1.x)*(SS2.x-SS1.x)+(SS2.y-SS1.y)*(SS2.y-SS1.y)+(SS2.z-SS1.z)*(SS2.z-SS1.z));
	//					//	delete sketch_cylinders[pick_point];
	//					//	sketch_cylinders[pick_point]= new Cylinder(SS1.r,SS2.r,dist);
	//					//}
	//					//if(SS1.n!=sketchedNT_merged.listNeuron.size())
	//					//{
	//					//	NeuronSWC SS0 = sketchedNT_merged.listNeuron.at(SS1.n);//SS0 is the child of SS1
	//					//	float dist = glm::sqrt((SS0.x-SS1.x)*(SS0.x-SS1.x)+(SS0.y-SS1.y)*(SS0.y-SS1.y)+(SS0.z-SS1.z)*(SS0.z-SS1.z));
	//					//	delete sketch_cylinders[pick_point+1];
	//					//	sketch_cylinders[pick_point+1]= new Cylinder(SS0.r,SS1.r,dist);
	//					//}
	//				}
	//			}


	//			if(!(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)))
	//			{
	//				m_pickUpState = false;
	//				pick_point = -1;
 //               }//whenever the touchpad is unpressed, reset m_pickUpState and pick_point
	//		}
	//}
	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	while ( !bQuit )
	{
		bQuit = HandleInput();
		if (bQuit) break;
		RenderFrame();
	}
	SDL_StopTextInput();
}
// using in VR_MainWindow.cpp
bool CMainApplication::HandleOneIteration()
{
	bool bQuit = false;
	bQuit = HandleInput();
	RenderFrame();
	if(bQuit==true) Shutdown();
	
	return bQuit;

}

void CMainApplication::SetupCurrentUserInformation(string name, int typeNumber)
{
	current_agent_name = name;

	qDebug("typeNumber: %d",typeNumber);

	switch(typeNumber)
	{
	case 2:
		current_agent_color = "red";
		break;
	case 3:
		current_agent_color = "blue";
		break;
	case 4:
		current_agent_color = "purple";
		break;
	case 5:
		current_agent_color = "cyan";
		break;
	case 6:
		current_agent_color = "yellow";
		break;
	case 7:
		current_agent_color = "green";
		break;
	case 8:
		current_agent_color = "coffee";
		break;
	case 9:
		current_agent_color = "asparagus";
		break;
	case 10:
		current_agent_color = "salmon";
		break;
	case 11:
		current_agent_color = "ice";
		break;
	case 12:
		current_agent_color = "orchid";
		break;
	default:
		current_agent_color = "other color";
		break;
	}
}

void CMainApplication::SetupAgentModels(vector<Agent> &curAgents)
{

	for (int i=0;i<Agents_spheres.size();i++) delete Agents_spheres[i];
	Agents_spheres.clear();//clear old spheres
	Agents_spheresPos.clear();//clear old spheres pos
	Agents_spheresColor.clear();//clear old spheres color

	if(curAgents.size()<2) return;//size<2 means there is no other users,no need to generate sphere

	for(int i=0;i<curAgents.size();i++)//generate new spheres as other users
	{
		if(curAgents.at(i).isItSelf==true) continue;//come to itself, skip
		Agents_spheres.push_back(new Sphere((0.05f / m_globalScale),5,5));

		int type =curAgents.at(i).colorType;//the color of sphere's surface should be the same as Agent's colortype
		glm::vec3 agentclr=glm::vec3();
		agentclr[0] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][0];
		agentclr[1] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][1];
		agentclr[2] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][2];
		for(int i=0;i<3;i++) agentclr[i] /= 255.0;//range should be in [0,1]
		Agents_spheresColor.push_back(agentclr);

		glm::mat4 mat_HMD = glm::mat4();
		for (int k = 0; k < 4; k++)
		{
			for (int j = 0; j < 4; j++)
			{
				mat_HMD[k][j]=curAgents.at(i).position[k*4+j];
				//qDebug()<<"mat_HMD"<<k<<j<<mat_HMD[k][j]<<" ";
			}
		}
		glm::vec4 posss = mat_HMD * glm::vec4( 0, 0, 0, 1 );
		XYZ convertPOS = ConvertGlobaltoLocalCoords(posss.x,posss.y,posss.z);
		//qDebug()<<" get agent POS "<<convertPOS.x<<" "<<convertPOS.y<<" "<<convertPOS.z;
		glm::vec3 agentsPos=glm::vec3(convertPOS.x,convertPOS.y,convertPOS.z);
		//agentsPos  means user's position in world, posss[3] will always be 1.
		//later may need orientation information
		if(curAgents[i].name == collaboration_creator_name)
		{
			//qDebug()<<collaboration_creator_name<<"converted pos";
			CollaborationCreatorPos = convertPOS;
		}
		Agents_spheresPos.push_back(agentsPos);
	}

}

void CMainApplication::SetupMarkerandSurface(double x,double y,double z,int type)
{
	ImageMarker mk(x,y,z);
	mk.type = type;
	mk.radius = 0.02f / m_globalScale;

	glm::vec3 agentclr=glm::vec3();
	agentclr[0] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][0];
	agentclr[1] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][1];
	agentclr[2] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][2];

	mk.color.r = agentclr[0];
	mk.color.g = agentclr[1];
	mk.color.b = agentclr[2];
	drawnMarkerList.push_back(mk);
	markerVisibility.push_back(1);

	Markers_spheres.push_back(new Sphere(mk.radius,10,10));
	Markers_spheresPos.push_back(glm::vec3(mk.x,mk.y,mk.z));


	for(int i=0;i<3;i++) agentclr[i] /= 255.0;//range should be in [0,1]
	Markers_spheresColor.push_back(agentclr);


}

void CMainApplication::SetupMarkerandSurface(double x,double y,double z,int colorR, int colorG, int colorB)
{
	ImageMarker mk(x,y,z);
	mk.radius = 0.02f / m_globalScale;
	mk.color.r = colorR;
	mk.color.g = colorG;
	mk.color.b = colorB;
	drawnMarkerList.push_back(mk);
	markerVisibility.push_back(1);

	Markers_spheres.push_back(new Sphere(mk.radius,10,10));
	Markers_spheresPos.push_back(glm::vec3(mk.x,mk.y,mk.z));

	glm::vec3 agentclr=glm::vec3(colorR,colorG,colorB);

	for(int i=0;i<3;i++) agentclr[i] /= 255.0;//range should be in [0,1]
	Markers_spheresColor.push_back(agentclr);


}

bool CMainApplication::RemoveMarkerandSurface(double x,double y,double z,int type)
{
	// bool deletedmarker=false;
	//remove the marker in list 
		for(int i=0;i<drawnMarkerList.size();i++)
		{
			ImageMarker markertemp = drawnMarkerList.at(i);
			float dist;
			if(isOnline == false )
				dist = glm::sqrt((markertemp.x- x)*(markertemp.x- x)+(markertemp.y- y)*(markertemp.y- y)+(markertemp.z- z)*(markertemp.z- z));
			else 
			{
				XYZ TargetResx = ConvertLocaltoGlobalCoords(x,y,z,CollaborationTargetMarkerRes);
				XYZ TaegetMarkx = ConvertLocaltoGlobalCoords(markertemp.x,markertemp.y,markertemp.z,CollaborationTargetMarkerRes);
				dist = glm::sqrt((TaegetMarkx.x- TargetResx.x)*(TaegetMarkx.x- TargetResx.x)+(TaegetMarkx.y- TargetResx.y)*(TaegetMarkx.y- TargetResx.y)+(TaegetMarkx.z- TargetResx.z)*(TaegetMarkx.z- TargetResx.z));
			}
			//cal the dist between pos & current node'position, then compare with the threshold
			if(dist < (dist_thres/m_globalScale*5))
			{
				drawnMarkerList.removeAt(i);
				markerVisibility.erase(markerVisibility.begin()+i);
				qDebug()<<"remove marker at "<<i;
				if(Markers_spheres[i]) delete Markers_spheres[i];
				Markers_spheres.erase(Markers_spheres.begin()+i);
				Markers_spheresPos.erase(Markers_spheresPos.begin()+i);
				Markers_spheresColor.erase(Markers_spheresColor.begin()+i);
				// deletedmarker = true;
				return true;
			}
		}

	return false;
	//if(deletedmarker == true)//if deleted a marker in drawnMarkerList, then
	//{
	//	//empty all Markers_spheres,Markers_spheresPos,Markers_spheresColor
	//	for (int i=0;i<Markers_spheres.size();i++) delete Markers_spheres[i];
	//	Markers_spheres.clear();
	//	Markers_spheresPos.clear();
	//	Markers_spheresColor.clear();

	//	//reset Markers_spheres,Markers_spheresPos,Markers_spheresColor
	//	for(int i=0;i<drawnMarkerList.size();i++)
	//	{
	//		ImageMarker mk = drawnMarkerList.at(i);
	//		Markers_spheres.push_back(new Sphere(mk.radius,10,10));
	//		Markers_spheresPos.push_back(glm::vec3(mk.x,mk.y,mk.z));

	//		glm::vec3 agentclr=glm::vec3();
	//		agentclr[0] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][0];
	//		agentclr[1] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][1];
	//		agentclr[2] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][2];
	//		for(int i=0;i<3;i++) agentclr[i] /= 255.0;//range should be in [0,1]
	//		Markers_spheresColor.push_back(agentclr);
	//	}
	//}
	//else
	//{
	//	//cannot find marker, do nothing
	//	qDebug()<<"Cannot find any marker nearby.Please retry.";
	//}

}
//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void CMainApplication::ProcessVREvent( const vr::VREvent_t & event )
{
	/*if((event.eventtype==vr::vrevent_buttonpress)&&(event.data.controller.button==vr::k_ebutton_steamvr_trigger))
	{
		sdl_showsimplemessagebox( sdl_messagebox_error, "trigger pressed! ", "trigger pressed!", null );
	}//*/
	//if(event.data.controller.button==vr::k_ebutton_steamvr_trigger)

	
	////////////////////////////////LEFT
	if((event.trackedDeviceIndex==m_iControllerIDLeft)&&(event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_Grip))
	{	
		// m_modeControlGrip_L++;
		// m_modeControlGrip_L%=11;
		// switch(m_modeControlGrip_L)
		// {
		// case 0:
		// 	m_modeGrip_L = _donothing;
		// 	break;
		// case 1:
		// 	m_modeGrip_L = _TeraShift;
		// 	break;
		// case 2:
		// 	m_modeGrip_L = _TeraZoom;
		// 	break;
		// case 3:
		// 	m_modeGrip_L = _Contrast;
		// 	break;
		// case 4:
		// 	m_modeGrip_L = _UndoRedo;
		// 	break;
		// case 5:
		// 	m_modeGrip_L = _ColorChange;
		// 	break;
		// case 6:
		// 	m_modeGrip_L = _Surface;
		// 	break;
		// case 7:
		// 	m_modeGrip_L = _VirtualFinger;
		// 	break;
		// case 8:
		// 	m_modeGrip_L = _Freeze;
		// 	break;
		// case 9:
		// 	m_modeGrip_L = _LineWidth;
		// 	break;
		// case 10:
		// 	m_modeGrip_L = _AutoRotate;
		// 	break;
		// default:
		// 	break;
		// }
		if(isOnline == false)
		{
			m_globalMatrix = glm::mat4();
			SetupGlobalMatrix();
		}
	}

    if((event.trackedDeviceIndex==m_iControllerIDLeft)&&(event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_SteamVR_Touchpad)&&(!showshootingray))
	{	
		vr::VRControllerState_t state;	
		m_pHMD->GetControllerState( m_iControllerIDLeft, &state, sizeof(state));
		float temp_x  = state.rAxis[0].x;
		// bool ONorOFF=false;
		// if(temp_x>0)
		// {
		// 	ONorOFF = false;
		// }
		// else
		// {
		// 	ONorOFF = true;
		// }
		switch(m_modeGrip_L)
		{
		case _donothing:
			break;
		case _Surface:
			{
				m_bShowMorphologySurface = !m_bShowMorphologySurface;
				m_bShowMorphologyLine = !m_bShowMorphologyLine;
				m_bShowMorphologyMarker = !m_bShowMorphologyMarker;
				if(m_bShowMorphologySurface)
					qDebug()<<"m_bShowMorphologySurface ON";
				else
					qDebug()<<"m_bShowMorphologySurface OFF";
				break;
			}
		case _VirtualFinger:
			{
				qDebug()<<"Run virtual finger chooose";
				m_bVirtualFingerON = !m_bVirtualFingerON;	
				if(m_bVirtualFingerON)
					qDebug()<<"virtual finger ON";
				else
					qDebug()<<"virtual finger OFF";
				break;
			}
		case _Freeze: // now temporarily used for brightness supression
			{
				//m_bControllerModelON = !m_bControllerModelON;
				//m_bFrozen is used to control texture
                m_bFrozen = !m_bFrozen;


                    if (fBrightness >= -0.9) fBrightness = -1.0;
                    else fBrightness = 0.0;

                break;
			}
		case _Contrast://contrast func is moved to right controller touch pad , grip button+/-
			{
				 if(temp_x>0)
				 {
					fBrightness+= 0.02f;
					if(fBrightness>0.9f)
						fBrightness = 0.9f;
				}
				else
				{
					fBrightness-= 0.02f;
					if(fBrightness<-0.9)
						fBrightness = -0.9;
				}


				break;
			}
		case _UndoRedo:
			{
				qDebug()<<"Undo/Redo Operation. Lines Only.";
				if(temp_x>0)
				{
					 RedoLastSketchedNT();
					 SetupAllMorphologyLine();
				}
				else
				{
					 UndoLastSketchedNT();
					 SetupAllMorphologyLine();
				}
				break;
			}
		case _LineWidth: //line width
			{
				qDebug()<<"Clear all sketch Neuron";
				if(temp_x>0)
				{
					iLineWid+=1;
					if (iLineWid>10)
						iLineWid = 10;
				}
				else
				{
					iLineWid-=1;
					if (iLineWid<1)
						iLineWid = 1;
				}
			
				

				break;
			}
		case _AutoRotate: //actually for auto-rotation
			{
				qDebug()<<"Auto rotation";
				if (m_autoRotateON)
				{
					m_autoRotateON = false;
				}
				else 
				{
					m_autoRotateON = true;

					const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];// mat means current controller pos
					glm::mat4 mat = glm::mat4();
					for (size_t i = 0; i < 4; i++)
					{
						for (size_t j = 0; j < 4; j++)
						{
							mat[i][j] = *(mat_M.get() + i * 4 + j);
						}
					}
					mat=glm::inverse(m_globalMatrix) * mat;
					glm::vec4 ctrlLeftPos = mat * glm::vec4( 0, 0, 0, 1 );

					autoRotationCenter.x = ctrlLeftPos.x;
					autoRotationCenter.y = ctrlLeftPos.y;
					autoRotationCenter.z = ctrlLeftPos.z;
				}
								
				break;
			}
		case _TeraShift:
			{
				const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];// mat means current controller pos
					glm::mat4 mat = glm::mat4();
					for (size_t i = 0; i < 4; i++)
					{
						for (size_t j = 0; j < 4; j++)
						{
							mat[i][j] = *(mat_M.get() + i * 4 + j);
						}
					}
					mat=glm::inverse(m_globalMatrix) * mat;
					glm::vec4 ctrlLeftPos = mat * glm::vec4( 0, 0, 0, 1 );
					teraflyPOS = XYZ(ctrlLeftPos.x,ctrlLeftPos.y,ctrlLeftPos.z);
					float _deltaX = fabs(ctrlLeftPos.x - loadedNTCenter.x);
					float _deltaY = fabs(ctrlLeftPos.y - loadedNTCenter.y);
					float _deltaZ = fabs(ctrlLeftPos.z - loadedNTCenter.z);
					float _maxDelta = MAX(MAX(_deltaX, _deltaY), _deltaZ);
					if(teraflyPOS.x<swcBB.x0)
						teraflyPOS.x=swcBB.x0;
					else if (teraflyPOS.x>swcBB.x1)
						teraflyPOS.x=swcBB.x1;
					if(teraflyPOS.y<swcBB.y0)
						teraflyPOS.y=swcBB.y0;
					else if (teraflyPOS.y>swcBB.y1)
						teraflyPOS.y=swcBB.y1;
					if(teraflyPOS.z<swcBB.z0)
						teraflyPOS.z=swcBB.z0;
					else if (teraflyPOS.y>swcBB.y1)
						teraflyPOS.z=swcBB.z1;
					if (_maxDelta == _deltaX)
					{
						postVRFunctionCallMode = (ctrlLeftPos.x - loadedNTCenter.x) > 0 ? 1 : 2;
						teraflyPOS.x = loadedNTCenter.x;
					}
					else if (_maxDelta == _deltaY)
					{
						postVRFunctionCallMode = (ctrlLeftPos.y - loadedNTCenter.y) > 0 ? 3 : 4;
						teraflyPOS.y = loadedNTCenter.y;
					}
					else if (_maxDelta == _deltaZ)
					{
						postVRFunctionCallMode = (ctrlLeftPos.z - loadedNTCenter.z) > 0 ? 5 : 6;
						teraflyPOS.z = loadedNTCenter.z;
					}
					else
						qDebug() << "oh no! Something wrong!Please check!";
					cout<<"postVRFunctionCallMode"<<postVRFunctionCallMode<<endl;
					break;
			}
		case _TeraZoom:
			{
				if(temp_x>0)
				{
					// zoom in

					const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];// mat means current controller pos
					glm::mat4 mat = glm::mat4();
					for (size_t i = 0; i < 4; i++)
					{
						for (size_t j = 0; j < 4; j++)
						{
							mat[i][j] = *(mat_M.get() + i * 4 + j);
						}
					}
					mat=glm::inverse(m_globalMatrix) * mat;
					glm::vec4 ctrlLeftPos = mat * glm::vec4( 0, 0, 0, 1 );
					teraflyPOS = XYZ(ctrlLeftPos.x,ctrlLeftPos.y,ctrlLeftPos.z);

					if((teraflyPOS.x<swcBB.x0)||(teraflyPOS.y<swcBB.y0)||(teraflyPOS.z<swcBB.z0)
						||(teraflyPOS.x>swcBB.x1)||(teraflyPOS.y>swcBB.y1)||(teraflyPOS.z>swcBB.z1))
						qDebug()<<"Out of the bounding box.Ignored!";
					else
						postVRFunctionCallMode = 7;
				}
				else // zoom out
					postVRFunctionCallMode = 8;	
				break;
			}
		case _ColorChange:
			{
				// if(isOnline == false)
				// {
				// 	// range 0 ~ 7  neuron_type_color_num=276 
				// 	m_curMarkerColorType = (++m_curMarkerColorType)%8;
				// 	int color_id = (m_curMarkerColorType>=0 && m_curMarkerColorType<neuron_type_color_num)? m_curMarkerColorType : 0;
				// 	ctrSphereColor[0] =  neuron_type_color[color_id][0] /255.0;
				// 	ctrSphereColor[1] =  neuron_type_color[color_id][1] /255.0;
				// 	ctrSphereColor[2] =  neuron_type_color[color_id][2] /255.0;
				// 	cout<<"neuron_type_color_num"<<neuron_type_color_num<<endl;
				// }
				break;
			}
		case _ResetImage:
			{
				if(isOnline == false)
				{
					m_globalMatrix = glm::mat4();
					SetupGlobalMatrix();
				}
				break;
			}
		case _MovetoCreator:
			{
				if(isOnline == true)
				{
					if(CollaborationCreatorPos.x!=0&&CollaborationCreatorPos.y!=0&&CollaborationCreatorPos.z!=0)
					{
						qDebug()<<"get creator pos";						
						teraflyPOS = XYZ(CollaborationCreatorPos.x,CollaborationCreatorPos.y,CollaborationCreatorPos.z);//liqi
						postVRFunctionCallMode = 9;
					}
				}
			}
		case _RGBImage: //line width
			{
				if(temp_x>0)
				{
					switch(m_rgbChannel)
					{
					case channel_rgb:
						{m_rgbChannel = channel_r;
						break;}
					case channel_r:
						{m_rgbChannel = channel_g;break;}
					case channel_g:
						{m_rgbChannel = channel_b;break;}
					case channel_b:
						{m_rgbChannel = channel_rgb;break;}
					default:
						break;
					}
				}
				else
				{
					switch(m_rgbChannel)
					{
					case channel_rgb:
						{m_rgbChannel = channel_b;
						break;}
					case channel_r:
						{m_rgbChannel = channel_rgb;break;}
					case channel_g:
						{m_rgbChannel = channel_r;break;}
					case channel_b:
						{m_rgbChannel = channel_g;break;}
					default:
						break;}
				}
				break;
			}
			break;
		case _StretchImage:
			{
				if(temp_x>0&&iscaleZ<=5)
				{				
				iscaleZ++;
				m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
				m_globalMatrix = glm::scale(m_globalMatrix,glm::vec3(1,1,5.0/4.0));
				m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
				}
				else if(temp_x<0&&iscaleZ>1)
				{
				iscaleZ--;
				m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
				m_globalMatrix = glm::scale(m_globalMatrix,glm::vec3(1,1,4.0/5.0));
				m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
				}
			}
		default:
			break;
		}
	}
    if((event.trackedDeviceIndex==m_iControllerIDLeft)&&(event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_SteamVR_Trigger))
    {

            const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];// mat means current controller pos
            glm::mat4 mat = glm::mat4();
            for (size_t i = 0; i < 4; i++)
            {
                for (size_t j = 0; j < 4; j++)
                {
                    mat[i][j] = *(mat_M.get() + i * 4 + j);
                }
            }
            m_oldCtrlMatrix = mat;
			m_oldGlobalMatrix = m_globalMatrix;
	}
//    if((event.trackedDeviceIndex==m_iControllerIDLeft)&&(event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_SteamVR_Trigger))
//    {
//
//            const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDLeft];// mat means current controller pos
//            glm::mat4 mat = glm::mat4();
//            for (size_t i = 0; i < 4; i++)
//            {
//                for (size_t j = 0; j < 4; j++)
//                {
//                    mat[i][j] = *(mat_M.get() + i * 4 + j);
//                }
//            }
//            mat=glm::inverse(m_globalMatrix) * mat;
//
//			glm::vec4 start = mat * glm::vec4( 0, 0, -0.02f, 1 );
//            //Vector4 start = mat_M * Vector4( 0, 0, -0.02f, 1 );
//
//
//            NeuronSWC SL0;
//            NeuronSWC SL1;
//            SL0.x = start.x ;SL0.y = start.y ;SL0.z = start.z ;SL0.r = default_radius;SL0.type = 200; SL0.n = swccount+1;SL0.pn = -1;
//
////            float dist = 0;
////            if(swccount>=2)
////            {
////                double point_x = sketchNT.listNeuron.at(swccount-2).x;
////                double point_y = sketchNT.listNeuron.at(swccount-2).y;
////                double point_z = sketchNT.listNeuron.at(swccount-2).z;
////                dist = glm::sqrt((point_x-start.x)*(point_x-start.x)+(point_y-start.y)*(point_y-start.y)+(point_z-start.z)*(point_z-start.z));
////            }
//
//            if(bool_ray)
//            {
//                ray_ratio = 1;
//                double updated_z = -0.29*ray_ratio;
//				glm::vec4 end = mat * glm::vec4( 0, 0, updated_z, 1 );
//                //Vector4 end = mat_M * Vector4( 0, 0, updated_z, 1 );
//                SL1.x = end.x ;SL1.y = end.y ;SL1.z = end.z ;SL1.r = default_radius;SL1.type = 200; SL1.n = swccount+2;SL1.pn = swccount+1;
//                sketchNT.listNeuron.append(SL0);
//                sketchNT.hashNeuron.insert(SL0.n, sketchNT.listNeuron.size()-1);//store NeuronSWC SL0 into sketchNT
//                sketchNT.listNeuron.append(SL1);
//                sketchNT.hashNeuron.insert(SL1.n, sketchNT.listNeuron.size()-1);//store NeuronSWC SL1 into sketchNT
//                swccount +=2;
//                bool_ray = false;
//            }else
//            {
//                ray_ratio++;
//                double updated_z = -0.29*ray_ratio;
//				glm::vec4 end = mat * glm::vec4( 0, 0, updated_z, 1 );
//                //Vector4 end = mat_M * Vector4( 0, 0, updated_z, 1 );
//                SL1.x = end.x ;SL1.y = end.y ;SL1.z = end.z ;SL1.r = default_radius;SL1.type = 200; SL1.n = swccount+1;SL1.pn = swccount;
//                sketchNT.listNeuron.append(SL1);
//                sketchNT.hashNeuron.insert(SL1.n, sketchNT.listNeuron.size()-1);//store NeuronSWC SL1 into sketchNT
//                swccount++;
//            }
//
//    }



	//////////////////////////////////////////RIGHT
	//if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_SteamVR_Touchpad)&&(event.eventType==vr::VREvent_ButtonUnpress))
	//{	
	//	vr::VRControllerState_t state;	
	//	m_pHMD->GetControllerState( m_iControllerIDRight, &state, sizeof(state));
	//	float temp_x  = state.rAxis[0].x;
	//	float temp_y  = state.rAxis[0].y;

	//	if(fabs(temp_x) > fabs(temp_y))
	//	{
	//		if((loadedNT.listNeuron.size()>0)&&(sketchNT.listNeuron.size()>0)) //both original_vr_neuron and areaofinterest must be non-empty.
	//		{
	//			//call feature search function, and update display

	//			//save current neurons
	//			QString outfilename1 = QCoreApplication::applicationDirPath()+"/original_vr_neuron.swc";
	//			writeSWC_file(outfilename1, loadedNT);
	//			qDebug("Successfully write original_vr_neuron");

	//			QString outfilename2 = QCoreApplication::applicationDirPath()+"/areaofinterest.swc";
	//			writeSWC_file(outfilename2, sketchNT);
	//			qDebug("Successfully write areaofinterest");

	//			//calculate
	//			if(temp_x<0) 
	//			{
	//				qDebug("Search bjut");
	//				if(!neuron_subpattern_search(1,mainwindow))
	//				{
	//					qDebug("Search failed!");
	//					return;	
	//				}
	//			}
	//			else 
	//			{
	//				qDebug("Search shu");
	//				if(!neuron_subpattern_search(2,mainwindow))
	//				{
	//					qDebug("Search failed!");
	//					return;	
	//				}
	//			}

	//			//load again
	//			QString filename = QCoreApplication::applicationDirPath()+"/updated_vr_neuron.swc";
	//			NeuronTree nt_tmp = readSWC_file(filename);
	//			qDebug("Successfully read tagged SWC file");

	//			for (int i=0; i<loadedNT.listNeuron.size(); i++)
	//				loadedNT.listNeuron[i].type = nt_tmp.listNeuron[i].type;

	//			SetupMorphologyLine(0);
	//		}
	//		else
	//		{
	//			qDebug("Area of interest is empty!");
	//		}
	//	} 
	//	else 
	//	{
	//		if(temp_y<0) 
	//		{
	//			qDebug("none");
	//		}
	//		else 
	//		{
	//			qDebug("freeze");
	//			m_bFrozen = !m_bFrozen;
	//		}
	//	}
	//}

    if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_SteamVR_Touchpad)/*&&(!showshootingray)*/)
	{		//use touchpad press to change the processing mode for touchpad, only contrast now
		// m_modeControlTouchPad_R++;
		// m_modeControlTouchPad_R%=4;

		// switch(m_modeControlTouchPad_R)
		// {
		// case 0:
		// 	m_contrastMode=m_rotateMode=m_zoomMode = false;
		// 	break;
		// case 1:
		// 	m_contrastMode = true;
		// 	m_rotateMode=m_zoomMode = false;
		// 	break;
		// case 2:
		// 	m_rotateMode = true;
		// 	m_contrastMode=m_zoomMode = false;
		// 	break;
		// case 3:
		// 	m_zoomMode = true;
		// 	m_contrastMode=m_rotateMode = false;
		// 	break;
		// default:
		// 	break;
		// }	
		//qDebug("m_modeControlTouchPad_R=%d,m_contrastMode=%d,m_rotateMode=%d,m_zoomMode=%d",m_modeControlTouchPad_R,m_contrastMode,m_rotateMode,m_zoomMode);
		vr::VRControllerState_t state;	
		m_pHMD->GetControllerState( m_iControllerIDRight, &state, sizeof(state));
		float temp_x  = state.rAxis[0].x;
		// case m_slabplaneMode:
		// {
		// 	if(temp_x>0)
		// 	{
		// 		fSlabwidth += 0.1;
		// 		if (fSlabwidth > 0.5)
		// 			fSlabwidth = 0.5;
		// 	}
		// 	if(temp_x<0)
		// 	{
		// 		fSlabwidth -= 0.1;
		// 		if (fSlabwidth < 0.05)
		// 			fSlabwidth = 0.05;
		// 	}
		// 	break;
		// }

	}
		if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_SteamVR_Trigger)&&(event.eventType==vr::VREvent_ButtonUnpress))	//detect trigger when menu show
		{
				glm::vec2 ShootingPadUV = calculateshootingPadUV();
				if(showshootingPad)
				MenuFunctionChoose(ShootingPadUV);
				else if(m_secondMenu!=_nothing&&(isOnline == false))
				{
					//todo liqi
					ColorMenuChoose(ShootingPadUV);
				}
				//qDebug()<<showshootingPad;
		}


	if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_SteamVR_Trigger)&&(event.eventType==vr::VREvent_ButtonUnpress)&&(!showshootingray))	//detect trigger when menu don't show
	{	

		qDebug()<<"current mode is "<<m_modeGrip_R;
		switch(m_modeGrip_R)
		{
		case m_drawMode:
			//if(m_modeGrip_R==m_drawMode)
			{
				if(img4d&&m_bVirtualFingerON) //if an image exist, call virtual finger functions for curve drawing
				{	
					//bool isAnyNodeOutBBox = false;
					tempNT.listNeuron.clear();
					tempNT.hashNeuron.clear();
					for(int i=0;i<currentNT.listNeuron.size();i++)
					{
						NeuronSWC S_node = currentNT.listNeuron.at(i);//swcBB
						if(!isAnyNodeOutBBox(S_node))
						{
							S_node.n=tempNT.listNeuron.size();
							if(S_node.pn!=-1)
								S_node.pn = tempNT.listNeuron.last().n;
							tempNT.listNeuron.append(S_node);
							tempNT.hashNeuron.insert(S_node.n, tempNT.listNeuron.size()-1);
						}
						else if(i==0)
							break;
					}

					if (tempNT.listNeuron.size()>0)
					{
						
						// improve curve shape
						NeuronTree InputNT;
						InputNT = tempNT;
						int iter_number=1;
						bool convergent = false;//todo:future may add this convergent func
						for(int i=0;(convergent==false)&&(i<iter_number);i++)
						{
							NeuronTree OutputNT;
							RefineSketchCurve(i%3,InputNT, OutputNT); //ver. 2b
							//convergent = CompareDist(InputNT, OutputNT);
							InputNT.listNeuron.clear();
							InputNT.hashNeuron.clear();
							InputNT = OutputNT;
						}
						currentNT.listNeuron.clear();
						currentNT.hashNeuron.clear();
						currentNT = InputNT;
						tempNT.listNeuron.clear();
						tempNT.hashNeuron.clear();	

						//change radius to mark the curved traced in VR.
						for(int i=0;i<currentNT.listNeuron.size();i++)
						{
							currentNT.listNeuron[i].r = 0.666;
							currentNT.listNeuron[i].creatmode = 666;
						}
					}
				}

				// if one of the endpoints of the newly drawn curve are near some existing curve with the given threshold,
				// change the endpoint position slightly so that the new curve connects with the existing curve.
				int autoConnected = -1;
				time_t timer2;// set timestamp to all segments created,with or without virtual finger on
				struct tm y2k = { 0 };
				double seconds;
				y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
				y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1; // seconds since January 1, 2000 in UTC
				time(&timer2);  /* get current time; same as: timer = time(NULL)  */
				seconds = difftime(timer2, timegm(&y2k)); //Timestamp LMG 27/9/2018
				qDebug("Timestamp at m_drawMode (VR) (seconds since January 1, 2000 in UTC): %.0f", seconds);
				for(int i=0;i<currentNT.listNeuron.size();i++)
				{
					if (currentNT.listNeuron[i].timestamp == 0) currentNT.listNeuron[i].timestamp = seconds;
				}
				if(currentNT.listNeuron.size()>0)
				{
					NeuronSWC* beginNode = &currentNT.listNeuron.first();
					NeuronSWC* endNode = &currentNT.listNeuron.last();
					// qDebug()<<"found begin&end node";
					// float endPointThres = 0.01;
					bool bNodeChanged = false;

					// try to match begin node
					for(int i = 0; i<sketchedNTList.size();i++)
					{
						NeuronTree nt0 = sketchedNTList.at(i);
						float dist = 0;
						float min_dist = 999999;
						NeuronSWC SS0;
						NeuronSWC min_node;

						//find the nearest point on the current curve
						//there could be more than one p on the curve, such that dist(p, begin)<thres. we always want to find the closest p to connect.
						for (int j=0;j<nt0.listNeuron.size();j++)
						{
							
							SS0 = nt0.listNeuron.at(j);
							dist = glm::sqrt((beginNode->x-SS0.x)*(beginNode->x-SS0.x)+(beginNode->y-SS0.y)*(beginNode->y-SS0.y)+(beginNode->z-SS0.z)*(beginNode->z-SS0.z));
						
							if(dist < min_dist)
							{
								min_dist = dist;
								min_node = SS0;
							}
						}

						//check if the candidate is qualified
						if (min_dist < connection_rigourous*(dist_thres/m_globalScale*5)
							//|| ((min_dist < 2*(dist_thres/m_globalScale*5) ) && (i == sketchedNTList.size()-1)    )
							)//todo: threshold to be refined
						{
							bNodeChanged = true; //no need to try to match end node
							beginNode->x = min_node.x;
							beginNode->y = min_node.y;
							beginNode->z = min_node.z;
							autoConnected = min_node.type;
							break;
						}
					}

					// try to match end node
					if (1||bNodeChanged == false) //2018-08-27: try to connect both ends if possible
					{
						// qDebug()<<"start to find begin nearest node";
						for(int i = 0; i<sketchedNTList.size();i++)
						 {
							NeuronTree nt0 = sketchedNTList.at(i);
							float dist = 0;
							float min_dist = 999999;
							NeuronSWC SS0;
							NeuronSWC min_node;
						
							for (int j=0;j<nt0.listNeuron.size();j++)
							{
								SS0 = nt0.listNeuron.at(j);
								dist = glm::sqrt((endNode->x-SS0.x)*(endNode->x-SS0.x)+(endNode->y-SS0.y)*(endNode->y-SS0.y)+(endNode->z-SS0.z)*(endNode->z-SS0.z));
								if(dist < min_dist)
								{
									min_dist = dist;
									min_node = SS0;
								}
							}

							if (min_dist < connection_rigourous*(dist_thres/m_globalScale*5) 
							//	|| ((min_dist < 3*(dist_thres/m_globalScale*5) ) && (i == sketchedNTList.size()-1)    )
								)//todo: threshold to be refined
							{
								endNode->x = min_node.x;
								endNode->y = min_node.y;
								endNode->z = min_node.z;
								autoConnected = min_node.type;
								break;
							}
						}						
					}					
				}

				if (autoConnected != -1 && m_curMarkerColorType == 0) //current color is white, allow auto color change after auto connection
				{
					for(int i=0;i<currentNT.listNeuron.size();i++)
					{
						currentNT.listNeuron[i].type = autoConnected;
					}
				}
				if (isOnline==false)
				{
					if(currentNT.listNeuron.size()>0)
					{
						bIsUndoEnable = true;
						if(vUndoList.size()==MAX_UNDO_COUNT)
						{
							vUndoList.erase(vUndoList.begin());
						}
						vUndoList.push_back(sketchedNTList);
						if (vRedoList.size()> 0)
							vRedoList.clear();
						bIsRedoEnable = false;
						vRedoList.clear();
						
						currentNT.name = "sketch_"+QString("%1").arg(sketchNum++);
						qDebug()<<currentNT.name;
						sketchedNTList.push_back(currentNT);
						int lIndex = sketchedNTList.size() - 1;
						qDebug()<<"index = "<<lIndex;
						SetupSingleMorphologyLine(lIndex,0);
						currentNT.listNeuron.clear();
						currentNT.hashNeuron.clear();		
					}
					swccount=0;
				}
				break;
			}
		case m_deleteMode:
			//else if(m_modeGrip_R==m_deleteMode)
			{
				const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos
				glm::mat4 mat = glm::mat4();
				for (size_t i = 0; i < 4; i++)
				{
					for (size_t j = 0; j < 4; j++)
					{
						mat[i][j] = *(mat_M.get() + i * 4 + j);
					}
				}
				mat=glm::inverse(m_globalMatrix) * mat;
				glm::vec4 m_v4DevicePose = mat * glm::vec4( 0, 0, 0, 1 );//change the world space(with the globalMatrix) to the initial world space
				delName = "";
				delcurvePOS  ="";
				delcurvePOS = QString("%1 %2 %3").arg(m_v4DevicePose.x).arg(m_v4DevicePose.y).arg(m_v4DevicePose.z);
				delName = FindNearestSegment(glm::vec3(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z));
				if(isOnline==false)	
				{
					NTL temp_NTL = sketchedNTList;
					bool delerror = DeleteSegment(delName);
					if(delerror==true)
					{
						bIsUndoEnable = true;
						if(vUndoList.size()==MAX_UNDO_COUNT)
						{
							vUndoList.erase(vUndoList.begin());
						}
						vUndoList.push_back(temp_NTL);
						if (vRedoList.size()> 0)
							vRedoList.clear();
						bIsRedoEnable = false;
						vRedoList.clear();					
						qDebug()<<"Segment Deleted.";
					}
					else
						qDebug()<<"Cannot Find the Segment ";
				}
				break;
			}
		case m_ConnectMode:
		{
			//do timestamp when create new segments
			time_t timer2; // set timestamp to all segments created,with or without virtual finger on
			struct tm y2k = {0};
			double seconds;
			y2k.tm_hour = 0;
			y2k.tm_min = 0;
			y2k.tm_sec = 0;
			y2k.tm_year = 100;
			y2k.tm_mon = 0;
			y2k.tm_mday = 1;						  // seconds since January 1, 2000 in UTC
			time(&timer2);							  /* get current time; same as: timer = time(NULL)  */
			seconds = difftime(timer2, timegm(&y2k)); //Timestamp LMG 27/9/2018
			qDebug("Timestamp at m_drawMode (VR) (seconds since January 1, 2000 in UTC): %.0f", seconds);
			for (int i = 0; i < currentNT.listNeuron.size(); i++)
			{
				if (currentNT.listNeuron[i].timestamp == 0)
					currentNT.listNeuron[i].timestamp = seconds;
			}

			//do simple connect
			float tolerance = 20;
			std::vector<int> segInfo;//prevent connect same segment
			std::vector<int> nodeIndexInfo;//check connected node index ,prevent connect node both in middle of segment .only allow connect Branch to begin/end or begin/end to end/begin
			NeuronTree connectTree;
			for(V3DLONG  i = 0;i<sketchedNTList.size();i++)
			{
				cout<<"sketchedNTList"<<"   "<<i<<endl;
				NeuronTree* p_tree = (NeuronTree*)(&(sketchedNTList.at(i)));
				QList<NeuronSWC>* p_listneuron = &(p_tree->listNeuron);

				long minX=currentNT.listNeuron.at(0).x;
				long minY=currentNT.listNeuron.at(0).y;
				long minZ=currentNT.listNeuron.at(0).z;
				long maxX=minX;
				long maxY=minY;
				long maxZ=minZ;
				for(size_t curi = 0;curi<currentNT.listNeuron.size();++curi)
				{
					if(currentNT.listNeuron.at(curi).x<=minX) minX = currentNT.listNeuron.at(curi).x;
					if(currentNT.listNeuron.at(curi).y<=minY) minY = currentNT.listNeuron.at(curi).y;
					if(currentNT.listNeuron.at(curi).z<=minZ) minZ = currentNT.listNeuron.at(curi).z;
					if(currentNT.listNeuron.at(curi).x>=maxX) maxX = currentNT.listNeuron.at(curi).x;
					if(currentNT.listNeuron.at(curi).y>=maxY) maxY = currentNT.listNeuron.at(curi).y;
					if(currentNT.listNeuron.at(curi).z>=maxZ) maxZ = currentNT.listNeuron.at(curi).z;
				}

				minX = minX - 5;minY = minY - 5;minZ = minZ - 5;
				maxX = maxX + 5;maxY = maxY + 5;maxZ = maxZ + 5;
				cout<<"minX is "<<minX<<"minY is "<<minY<<"minZ is "<<minZ<<endl;
				cout<<"maxX is "<<maxX<<"maxY is "<<maxY<<"maxZ is "<<maxZ<<endl;

				QList<NeuronSWC> nodeOnStroke;
				float matchdistance = 9999;
				NeuronSWC matchestNode;
				int matchestIndex = -1;
				for(size_t j = 0;j<p_listneuron->size();j++)
				{
					GLdouble  ix, iy, iz;
					ix = p_listneuron->at(j).x;
					iy = p_listneuron->at(j).y;
					iz = p_listneuron->at(j).z;
					cout<<"ix is "<<ix<<"iy is "<<iy<<"iz is "<<iz<<endl;
					if ((ix >= minX && ix<= maxX) && (iy >= minY &&iy <= maxY) && (iz >= minZ &&iz <= maxZ))
					{
						nodeOnStroke.push_back(p_listneuron->at(j));//把当前考虑的已经画好的线 并且在stroke的矩形范围内的节点Push进入nodeonstroke
						//cout << p.x() << " " << p.y() << endl;
					}
				}

				cout<<"node on stroke size in skelist"<<" "<<i<<"is"<<nodeOnStroke.size();
				for(V3DLONG j = 0;j<currentNT.listNeuron.size();j++)
				{
					
					for(V3DLONG k = 0;k<nodeOnStroke.size();++k)
					{
						GLdouble  ix, iy, iz;
						ix = nodeOnStroke.at(k).x;
						iy = nodeOnStroke.at(k).y;
						iz = nodeOnStroke.at(k).z;
						XYZ p2(ix,iy,iz);
						XYZ p(currentNT.listNeuron.at(j).x,currentNT.listNeuron.at(j).y,currentNT.listNeuron.at(j).z);
						float currentdistance = std::sqrt((p.x - p2.x)*(p.x - p2.x) + (p.y - p2.y)*(p.y - p2.y)+(p.z - p2.z)*(p.z - p2.z));
						if(currentdistance <= tolerance)
						{
							if(currentdistance<matchdistance) 
							{
								matchestNode = nodeOnStroke.at(k);
								
								matchdistance = currentdistance;
							}
						}
						
					}

				}
				//push matchest node on the same segment (distance is smallest
				if(segInfo.size()==0&&matchdistance<=tolerance)
				{
					segInfo.push_back(i);//push current segment ID
					nodeIndexInfo.push_back(matchestNode.n);
					NeuronSWC SL0 = matchestNode;
					//SL0.pn = -1;
					//SL0.n = 1;
					connectTree.listNeuron.append(SL0);
					connectTree.hashNeuron.insert(SL0.n,connectTree.listNeuron.size()-1);
					cout<<"segInfo.size == 1 now"<<endl;
				}							
				else
				{
					cout<<"come into else "<<endl;
					bool repeat = false;
					if(std::find(segInfo.begin(),segInfo.end(),i)!=segInfo.end())repeat = true;
					cout<<"step 1"<<endl;
					if(repeat ==false&&matchdistance<=tolerance)
					{
						segInfo.push_back(i);
						nodeIndexInfo.push_back(matchestNode.n);
						NeuronSWC SL0 = matchestNode;
						//SL0.pn = 1;
						//SL0.n = 2;
						connectTree.listNeuron.append(SL0);
						connectTree.hashNeuron.insert(SL0.n,connectTree.listNeuron.size()-1);
						cout<<"segInfo.size == 2 now"<<endl;
					}
				}//for each sketchlist

				cout<<"come out of for"<<endl;
				if(segInfo.size()==2) break;
			}
			NeuronTree curNT;
			//resort the number of connected segment
			if(segInfo.size()==2)
			{

				int index1 = connectTree.listNeuron.at(0).n;
				int index2 = connectTree.listNeuron.at(1).n;
				cout<<"index1 is "<<index1<<endl;
				cout<<"index2 is "<<index2<<endl;
				
				NeuronTree * p_connectNT1 = &sketchedNTList[segInfo[0]];
				NeuronTree * p_connectNT2 = &sketchedNTList[segInfo[1]];
				cout<<"p_connectNT1.list size is "<<p_connectNT1->listNeuron.size();
				cout<<"p_connectNT2.list size is "<<p_connectNT2->listNeuron.size();
				if((index1==1&&index2==p_connectNT2->listNeuron.size())//head to tail
					||(index1==p_connectNT1->listNeuron.size()&&index2==1)//tail to head 
					||(index1==1&&index2==1)//head to head
					||(index1==p_connectNT1->listNeuron.size()&&index2 ==p_connectNT2->listNeuron.size()))//tail to tail
				{
					cout<<"connect head to tail !!! come into here"<<endl;
					for(int j = 0;j<2;j++)
					{
						cout<<"pos 1 "<<j<<endl;
						int index = connectTree.listNeuron.at(j).n;
						NeuronTree * connectedNT = &sketchedNTList[segInfo[j]];
						if((index==1&&j==0)||(index==connectedNT->listNeuron.size()&&j==1))
						{
							cout<<"pos 3 "<<j<<endl;
							for(int i = connectedNT->listNeuron.size()-1;i>=0;i--)//add first segment to connectNT
							{
								cout<<"pos 4 "<<i<<endl;
								NeuronSWC SL0  = connectedNT->listNeuron.at(i);
								SL0.n = curNT.listNeuron.size()+1;
								if(i==connectedNT->listNeuron.size()-1&&j==0)
								{
									SL0.pn = -1;
								}
								else
									SL0.pn = curNT.listNeuron.at(SL0.n-1-1).n;
								curNT.listNeuron.append(SL0);
								curNT.hashNeuron.insert(SL0.n,curNT.listNeuron.size()-1);
							}
						}	
						else if((index==connectedNT->listNeuron.size()&&j==0)||(index==1&&j==1))
						{
							cout<<"pos 2 "<<j<<endl;
							for(int i = 0;i<connectedNT->listNeuron.size();i++)//add first segment to connectNT
							{

								NeuronSWC SL0  = connectedNT->listNeuron.at(i);
								SL0.n = curNT.listNeuron.size()+1;
								if(i==0&&j==0)
								{
									SL0.pn = -1;
								}
								else
									SL0.pn = curNT.listNeuron.at(SL0.n-1-1).n;
								curNT.listNeuron.append(SL0);
								curNT.hashNeuron.insert(SL0.n,curNT.listNeuron.size()-1);
							}
						}

						}
					if(isOnline==false)
					{
						if(currentNT.listNeuron.size()>0)
						{
							bIsUndoEnable = true;
							if(vUndoList.size()==MAX_UNDO_COUNT)
							{
								vUndoList.erase(vUndoList.begin());
							}
							vUndoList.push_back(sketchedNTList);
							if (vRedoList.size()> 0)
								vRedoList.clear();
							bIsRedoEnable = false;
							vRedoList.clear();

							//sketchedNTList.removeAt(segInfo[0]);
							//sketchedNTList.removeAt(segInfo[1]);
							//SetupSingleMorphologyLine(segInfo[1],2);
							DeleteSegment(sketchedNTList.at(segInfo[0]).name);
							DeleteSegment(sketchedNTList.at(segInfo[1]).name);

							curNT.name = "sketch_"+QString("%1").arg(sketchNum++);
							qDebug()<<curNT.name;
							sketchedNTList.push_back(curNT);
							int lIndex = sketchedNTList.size() - 1;
							qDebug()<<"index = "<<lIndex;
							SetupSingleMorphologyLine(lIndex,0);
							currentNT.listNeuron.clear();
							currentNT.hashNeuron.clear();		
						}
						swccount=0;
					}
				}
				else if(index1==1||index1==p_connectNT1->listNeuron.size()
						||index2==1||index2==p_connectNT2->listNeuron.size())
				{
					cout<<"come into connect branch to begin/end"<<endl;
					/*
					|	|
					|	|
					|\	|
					| \	|
					|  \|
					|	NT1
					NT2
					*/
					//begin/end connect to branch
					
					if(index2==1||index2==p_connectNT2->listNeuron.size())
					{
						cout<<"tranverse index1 and index2"<<endl;
						NeuronTree* p_tempNT = p_connectNT2;
						p_connectNT2 = p_connectNT1;
						p_connectNT1 = p_tempNT;
						int tempindex = index2;
						index2 = index1;
						index1 = tempindex;
					}
					NeuronTree result_NT1,result_NT2,result_NT3;
					cout<<"begin to add result NT1"<<endl;
					for(int i=0;i<index2;i++)
					{
						NeuronSWC SL0  = p_connectNT2->listNeuron.at(i);
						SL0.n = result_NT1.listNeuron.size()+1;
						if(i==0)
						{
							SL0.pn = -1;
						}
						else
							SL0.pn = result_NT1.listNeuron.at(SL0.n-1-1).n;
						result_NT1.listNeuron.append(SL0);
						result_NT1.hashNeuron.insert(SL0.n,result_NT1.listNeuron.size()-1);
					}
					cout<<"begin to add result NT2"<<endl;
					for(int i=index2-1;i<p_connectNT2->listNeuron.size();i++)
					{
						NeuronSWC SL0  = p_connectNT2->listNeuron.at(i);
						SL0.n = result_NT2.listNeuron.size()+1;
						if(i==index2-1)
						{
							SL0.pn = -1;
						}
						else
							SL0.pn = result_NT2.listNeuron.at(SL0.n-1-1).n;
						result_NT2.listNeuron.append(SL0);
						result_NT2.hashNeuron.insert(SL0.n,result_NT2.listNeuron.size()-1);
					}
					//add third segment
					NeuronSWC SL0  = p_connectNT2->listNeuron.at(index2-1);
					SL0.n = result_NT3.listNeuron.size()+1;
					SL0.pn = -1;
					result_NT3.listNeuron.append(SL0);
					result_NT3.hashNeuron.insert(SL0.n,result_NT3.listNeuron.size()-1);
					cout<<"begin to add result NT3"<<endl;
					if(index1 == 1)
					{
						cout<<"connect to NT3begin"<<endl;
						for(int i=0;i<p_connectNT1->listNeuron.size();i++)
						{
							NeuronSWC SL0  = p_connectNT1->listNeuron.at(i);
							SL0.n = result_NT3.listNeuron.size()+1;
							SL0.pn = result_NT3.listNeuron.at(SL0.n-1-1).n;
							result_NT3.listNeuron.append(SL0);
							result_NT3.hashNeuron.insert(SL0.n,result_NT3.listNeuron.size()-1);
						}
					}
					else if (index1==p_connectNT1->listNeuron.size())
					{
						cout<<"connect to NT3end"<<endl;
						for(int i=p_connectNT1->listNeuron.size()-1;i>=0;i--)
						{
							NeuronSWC SL0  = p_connectNT1->listNeuron.at(i);
							SL0.n = result_NT3.listNeuron.size()+1;
							SL0.pn = result_NT3.listNeuron.at(SL0.n-1-1).n;
							result_NT3.listNeuron.append(SL0);
							result_NT3.hashNeuron.insert(SL0.n,result_NT3.listNeuron.size()-1);
						}
					}
					if(isOnline==false)
					{
						cout<<"add result NT 1 2 3 to sketchlist"<<endl;
						if(currentNT.listNeuron.size()>0)
						{
							bIsUndoEnable = true;
							if(vUndoList.size()==MAX_UNDO_COUNT)
							{
								vUndoList.erase(vUndoList.begin());
							}
							vUndoList.push_back(sketchedNTList);
							if (vRedoList.size()> 0)
								vRedoList.clear();
							bIsRedoEnable = false;
							vRedoList.clear();

							//sketchedNTList.removeAt(segInfo[0]);
							//sketchedNTList.removeAt(segInfo[1]);
							//SetupSingleMorphologyLine(segInfo[0],2);
							//SetupSingleMorphologyLine(segInfo[1],2);
							qDebug()<<"sketchedNTList.at(segInfo[0]).name "<<sketchedNTList.at(segInfo[0]).name<<endl;
							qDebug()<<"sketchedNTList.at(segInfo[1]).name "<<sketchedNTList.at(segInfo[1]).name<<endl;
							QString tempdelname = sketchedNTList.at(segInfo[0]).name;
							QString tempdelname1 = sketchedNTList.at(segInfo[1]).name;
							bool delerror1 = DeleteSegment(tempdelname);
							if(delerror1)cout<<"delete 1 success"<<endl;
							cout<<"pos 1 get seginfo[1] name"<<endl;
							bool delerror2 = DeleteSegment(tempdelname1);
							if(delerror2)cout<<"delete 2 success"<<endl;
							
							result_NT1.name = "sketch_"+QString("%1").arg(sketchNum++);
							qDebug()<<result_NT1.name;
							sketchedNTList.push_back(result_NT1);
							int lIndex = sketchedNTList.size() - 1;
							qDebug()<<"index = "<<lIndex;
							SetupSingleMorphologyLine(lIndex,0);

							result_NT2.name = "sketch_"+QString("%1").arg(sketchNum++);
							qDebug()<<result_NT2.name;
							sketchedNTList.push_back(result_NT2);
							lIndex = sketchedNTList.size() - 1;
							qDebug()<<"index = "<<lIndex;
							SetupSingleMorphologyLine(lIndex,0);

							result_NT3.name = "sketch_"+QString("%1").arg(sketchNum++);
							qDebug()<<result_NT3.name;
							sketchedNTList.push_back(result_NT3);
							lIndex = sketchedNTList.size() - 1;
							qDebug()<<"index = "<<lIndex;
							SetupSingleMorphologyLine(lIndex,0);
							currentNT.listNeuron.clear();
							currentNT.hashNeuron.clear();		
						}
						swccount=0;
					}

				}
				else
				{
					//other situation clear currentNT listNeuron
					currentNT.listNeuron.clear();
					currentNT.hashNeuron.clear();
					swccount=0;
				}
			cout<<"add curNT done"<<endl;
			}
			
			break;
		}

		
		// case m_markMode:
		// 	{
		// 		const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos
		// 		glm::mat4 mat = glm::mat4();
		// 		for (size_t i = 0; i < 4; i++)
		// 		{
		// 			for (size_t j = 0; j < 4; j++)
		// 			{
		// 				mat[i][j] = *(mat_M.get() + i * 4 + j);
		// 			}
		// 		}
		// 		mat=glm::inverse(m_globalMatrix) * mat;
		// 		glm::vec4 m_v4DevicePose = mat * glm::vec4( 0, 0, 0, 1 );//change the world space(with the globalMatrix) to the initial world space
		// 		markerPOS="";
		// 		markerPOS = QString("%1 %2 %3").arg(m_v4DevicePose.x).arg(m_v4DevicePose.y).arg(m_v4DevicePose.z);
		// 		if(isOnline==false)	
		// 		{
		// 			ClearUndoRedoVectors();
		// 			SetupMarkerandSurface(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z,m_curMarkerColorType);
		// 		}

		// 		break;
		// 	}
		case m_dragMode:
			{
				_startdragnode = false;
				//ClearUndoRedoVectors();
			}
			
			break;
		case m_markMode://when there is a maker ,delete it ,otherwise add a marker
			{
				const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos
				glm::mat4 mat = glm::mat4();
				for (size_t i = 0; i < 4; i++)
				{
					for (size_t j = 0; j < 4; j++)
					{
						mat[i][j] = *(mat_M.get() + i * 4 + j);
					}
				}
				mat=glm::inverse(m_globalMatrix) * mat;
				glm::vec4 m_v4DevicePose = mat * glm::vec4( 0, 0, 0, 1 );//change the world space(with the globalMatrix) to the initial world space
				/*delmarkerPOS="";
				delmarkerPOS = QString("%1 %2 %3").arg(m_v4DevicePose.x).arg(m_v4DevicePose.y).arg(m_v4DevicePose.z);*/
				markerPOS="";
				markerPOS = QString("%1 %2 %3").arg(m_v4DevicePose.x).arg(m_v4DevicePose.y).arg(m_v4DevicePose.z);
				bool IsmarkerValid = false;
				if(isOnline==false)	
				{
					ClearUndoRedoVectors();
					IsmarkerValid = RemoveMarkerandSurface(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z);
					cout<<"m_v4DevicePose.x"<<m_v4DevicePose.x<<"m_v4DevicePose.y"<<m_v4DevicePose.y<<"m_v4DevicePose.z"<<m_v4DevicePose.z<<endl;
					cout<<"img4d->getYDim()"<<img4d->getYDim()<<"img4d->getXDim()"<<img4d->getXDim()<<"img4d->getZDim()"<<img4d->getZDim()<<endl;

					bool IsOutofBounds = ((m_v4DevicePose.x>img4d->getXDim()) || (m_v4DevicePose.x<=0))
						||((m_v4DevicePose.y>img4d->getYDim()) || (m_v4DevicePose.y<=0))
						||((m_v4DevicePose.z>img4d->getZDim()) || (m_v4DevicePose.z<=0));

					if((!IsmarkerValid)&&(!IsOutofBounds))
					{
						SetupMarkerandSurface(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z,m_curMarkerColorType);
					}
				}
				break;
			}
		case m_insertnodeMode:
			{
				if(isOnline == false)
				{
				const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos
				glm::mat4 mat = glm::mat4();
				for (size_t i = 0; i < 4; i++)
				{
					for (size_t j = 0; j < 4; j++)
					{
						mat[i][j] = *(mat_M.get() + i * 4 + j);
					}
				}
				mat=glm::inverse(m_globalMatrix) * mat;
				glm::vec4 m_v4DevicePose = mat * glm::vec4( 0, 0, 0, 1 );//change the world space(with the globalMatrix) to the initial world space		
				delName = "";
				delName = FindNearestSegment(glm::vec3(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z));
				NeuronTree nearestNT;
				NeuronSWC nearestNode;
				if (delName == "") break; //segment not found

				for(int i=0;i<sketchedNTList.size();i++)//get split NT,nearest node
				{
					QString NTname="";
					NTname = sketchedNTList.at(i).name;
					if(NTname==delName)
					{
						nearestNT=sketchedNTList.at(i);
						nearestNode = FindNearestNode(sketchedNTList.at(i),glm::vec3(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z));
						break;
					}
				}
				int switchinsertnode = 0;
				NeuronTree NewNT;
				int NewNTsize = nearestNT.listNeuron.size();
				if(nearestNode==*nearestNT.listNeuron.begin())
				{
					switchinsertnode = 1;
					NewNTsize += 1;
				}
				else if(nearestNode==nearestNT.listNeuron.back())
				{
					switchinsertnode = 2;
					NewNTsize += 1;
				}
				else
				{
					switchinsertnode = 3;
					NewNTsize += 2;
				}
				if(isOnline==false)	
				{
					NTL temp_NTL = sketchedNTList;
					bool delerror = DeleteSegment(delName);
					if(delerror==true)
					{
						bIsUndoEnable = true;
						if(vUndoList.size()==MAX_UNDO_COUNT)
						{
							vUndoList.erase(vUndoList.begin());
						}
						vUndoList.push_back(temp_NTL);
						if (vRedoList.size()> 0)
							vRedoList.clear();
						bIsRedoEnable = false;
						vRedoList.clear();					
						qDebug()<<"Segment Deleted.";
					}
					else
						qDebug()<<"Cannot Find the Segment ";
				}

				//set newNTsize after insert node
				//crete NewNT's topology
				for(int k=0;k<NewNTsize;k++)
				{

					NeuronSWC tempSWC;
					if(k==0)//insert first node
						{
							tempSWC.n=1;
							tempSWC.pn=-1;
							tempSWC.x=0;
							tempSWC.y=0;
							tempSWC.z=0;
							tempSWC.type = 2;
							tempSWC.creatmode = nearestNT.listNeuron[0].creatmode;
							tempSWC.timestamp = nearestNT.listNeuron[0].timestamp;
							NewNT.listNeuron.append(tempSWC);
        					NewNT.hashNeuron.insert(tempSWC.n, NewNT.listNeuron.size()-1);
							continue;
						}
					tempSWC.n=k+1;
					tempSWC.pn=NewNT.listNeuron.back().n;
					tempSWC.x=0;
					tempSWC.y=0;
					tempSWC.z=0;
					tempSWC.type = 2;
					tempSWC.creatmode = nearestNT.listNeuron[0].creatmode;
					tempSWC.timestamp = nearestNT.listNeuron[0].timestamp;
					NewNT.listNeuron.append(tempSWC);
        			NewNT.hashNeuron.insert(tempSWC.n, NewNT.listNeuron.size()-1);
				}
				//add data to New NT
				for(int k=0,j=0;k<NewNT.listNeuron.size();j++,k++)
				{
					switch (switchinsertnode)
					{
					case 1://insert node behind first
						{
							int temp = j;
							if(j+1<nearestNT.listNeuron.size())
							temp = j+1;
							if( nearestNT.listNeuron[j].n ==nearestNode.n+1&& k<=j)
							{
								NewNT.listNeuron[k].x = 1.0/2 * nearestNT.listNeuron[j-1].x + 3.0/8*nearestNT.listNeuron[j].x+1.0/8*nearestNT.listNeuron[temp].x;
								NewNT.listNeuron[k].y = 1.0/2 * nearestNT.listNeuron[j-1].y + 3.0/8*nearestNT.listNeuron[j].y+1.0/8*nearestNT.listNeuron[temp].y;
								NewNT.listNeuron[k].z = 1.0/2 * nearestNT.listNeuron[j-1].z + 3.0/8*nearestNT.listNeuron[j].z+1.0/8*nearestNT.listNeuron[temp].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;
								j--;
							}
							else
							{
								NewNT.listNeuron[k].x = nearestNT.listNeuron[j].x;
								NewNT.listNeuron[k].y = nearestNT.listNeuron[j].y;
								NewNT.listNeuron[k].z = nearestNT.listNeuron[j].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;	
							}
						}
						break;
					case 2://insert node before last
						{
							int temp = j;
							if(j-1>0)
							temp = j-2;
							if( nearestNT.listNeuron[j].n ==nearestNode.n&& k<=j)
							{
								
								NewNT.listNeuron[k].x = 1.0/2 * nearestNT.listNeuron[j].x + 3.0/8*nearestNT.listNeuron[j-1].x+1.0/8*nearestNT.listNeuron[temp].x;
								NewNT.listNeuron[k].y = 1.0/2 * nearestNT.listNeuron[j].y + 3.0/8*nearestNT.listNeuron[j-1].y+1.0/8*nearestNT.listNeuron[temp].y;
								NewNT.listNeuron[k].z = 1.0/2 * nearestNT.listNeuron[j].z + 3.0/8*nearestNT.listNeuron[j-1].z+1.0/8*nearestNT.listNeuron[temp].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;
								j--;
							}
							else
							{
								NewNT.listNeuron[k].x = nearestNT.listNeuron[j].x;
								NewNT.listNeuron[k].y = nearestNT.listNeuron[j].y;
								NewNT.listNeuron[k].z = nearestNT.listNeuron[j].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;	
							}
						}
						break;
					case 3://insert node in middle
						{
							int balancenode1 =j-1;
							int balancenode2 =j-1;
							int balancenode3 =j+1;
							int balancenode4 =j+1;
							if(j-1>0)
								balancenode1 = j-2;
							if(j+2<nearestNT.listNeuron.size())
								balancenode4 = j+2;
							if( nearestNT.listNeuron[j].n ==nearestNode.n&& k<=j)
							{
								NewNT.listNeuron[k].x = 1.0/8 * nearestNT.listNeuron[balancenode1].x+ 3.0/8 * nearestNT.listNeuron[balancenode2].x + 3.0/8*nearestNT.listNeuron[j].x+1.0/8*nearestNT.listNeuron[balancenode3].x;
								NewNT.listNeuron[k].y = 1.0/8*nearestNT.listNeuron[balancenode1].y+ 3.0/8 * nearestNT.listNeuron[balancenode2].y + 3.0/8*nearestNT.listNeuron[j].y+1.0/8*nearestNT.listNeuron[balancenode3].y;
								NewNT.listNeuron[k].z = 1.0/8*nearestNT.listNeuron[balancenode1].z+ 3.0/8 * nearestNT.listNeuron[balancenode2].z + 3.0/8*nearestNT.listNeuron[j].z+1.0/8*nearestNT.listNeuron[balancenode3].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;
								k++;
								NewNT.listNeuron[k].x = nearestNT.listNeuron[j].x;
								NewNT.listNeuron[k].y = nearestNT.listNeuron[j].y;
								NewNT.listNeuron[k].z = nearestNT.listNeuron[j].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;
								k++;
								NewNT.listNeuron[k].x =1.0/8*nearestNT.listNeuron[balancenode2].x+ 3.0/8 * nearestNT.listNeuron[j].x + 3.0/8*nearestNT.listNeuron[balancenode3].x+1.0/8*nearestNT.listNeuron[balancenode4].x;
								NewNT.listNeuron[k].y = 1.0/8*nearestNT.listNeuron[balancenode2].y+ 3.0/8 * nearestNT.listNeuron[j].y + 3.0/8*nearestNT.listNeuron[balancenode3].y+1.0/8*nearestNT.listNeuron[balancenode4].y;
								NewNT.listNeuron[k].z = 1.0/8*nearestNT.listNeuron[balancenode2].z+ 3.0/8 * nearestNT.listNeuron[j].z + 3.0/8*nearestNT.listNeuron[balancenode3].z+1.0/8*nearestNT.listNeuron[balancenode4].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;

							}
							else
							{
								NewNT.listNeuron[k].x = nearestNT.listNeuron[j].x;
								NewNT.listNeuron[k].y = nearestNT.listNeuron[j].y;
								NewNT.listNeuron[k].z = nearestNT.listNeuron[j].z;
								NewNT.listNeuron[k].r = nearestNT.listNeuron[j].r;
								NewNT.listNeuron[k].type = nearestNT.listNeuron[j].type;
							}

						}
						break;
					default:
						break;


					}
				}
				qDebug()<<"insert node finished!";
				//add the  new NT to NTList
				NewNT.name = "sketch_"+QString("%1").arg(sketchNum++);
				qDebug()<<NewNT.name;
				sketchedNTList.push_back(NewNT);
				int lIndex = sketchedNTList.size() - 1;
				qDebug()<<"index = "<<lIndex;
				SetupSingleMorphologyLine(lIndex,0);
				break;
				}
			}
		case m_splitMode:
			{	

				if(isOnline == false)
				{
				const Matrix4 & mat_M = m_rmat4DevicePose[m_iControllerIDRight];// mat means current controller pos
				glm::mat4 mat = glm::mat4();
				for (size_t i = 0; i < 4; i++)
				{
					for (size_t j = 0; j < 4; j++)
					{
						mat[i][j] = *(mat_M.get() + i * 4 + j);
					}
				}
				mat=glm::inverse(m_globalMatrix) * mat;
				glm::vec4 m_v4DevicePose = mat * glm::vec4( 0, 0, 0, 1 );//change the world space(with the globalMatrix) to the initial world space		
				delName = "";
				delName = FindNearestSegment(glm::vec3(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z));
				NeuronTree nearestNT;
				NeuronSWC nearestNode;
				if (delName == "") break; //segment not found	

				for(int i=0;i<sketchedNTList.size();i++)//get split NT,nearest node
				{
					QString NTname="";
					NTname = sketchedNTList.at(i).name;
					if(NTname==delName)
					{
						nearestNT=sketchedNTList.at(i);
						nearestNode = FindNearestNode(sketchedNTList.at(i),glm::vec3(m_v4DevicePose.x,m_v4DevicePose.y,m_v4DevicePose.z));
						break;
					}
				}
				//special situation for splitnode in head or end
				if((nearestNode==*nearestNT.listNeuron.begin())||(nearestNode==nearestNT.listNeuron.back())||nearestNT.listNeuron.size()<=3||nearestNode==*(nearestNT.listNeuron.begin()+1)||nearestNode==*(nearestNT.listNeuron.end()-2))
				{
					qDebug()<<"split node in hear/end or neuron is too short";
					break;
				}
				//delete NT first
				if(isOnline==false)	
				{
					NTL temp_NTL = sketchedNTList;
					bool delerror = DeleteSegment(delName);
					if(delerror==true)
					{
						bIsUndoEnable = true;
						if(vUndoList.size()==MAX_UNDO_COUNT)
						{
							vUndoList.erase(vUndoList.begin());
						}
						vUndoList.push_back(temp_NTL);
						if (vRedoList.size()> 0)
							vRedoList.clear();
						bIsRedoEnable = false;
						vRedoList.clear();					
						qDebug()<<"Segment Deleted.";
					}
					else
						qDebug()<<"Cannot Find the Segment ";
				}
				//add two NT
				NeuronTree splitNT1,splitNT2;
				glm::vec3 directionsplit;
				//split first half get splitdirection
				int splitNT2beginindex=0;
				int splitNT2size=1;
				for(int k=0;k<nearestNT.listNeuron.size();k++)
				{
					 if(nearestNT.listNeuron.at(k).n<=nearestNode.n)
					 { 
					 	splitNT1.listNeuron.append(nearestNT.listNeuron.at(k));
        			 	splitNT1.hashNeuron.insert(nearestNT.listNeuron.at(k).n, splitNT1.listNeuron.size()-1);
					 }
					 else
					 {	
						qDebug()<<"splitNT1 completed";
						NeuronSWC secnode=nearestNT.listNeuron.at(k);
					 	if(nearestNT.listNeuron.at(k).n==nearestNode.n+1)
					 	{	
							splitNT2beginindex=k-1;
							splitNT2size=nearestNT.listNeuron.size()-k+1;
					 		NeuronSWC secHeadNS=nearestNode;
							glm::vec3 tempdirection(secHeadNS.x-secnode.x,secHeadNS.y-secnode.y,secHeadNS.z-secnode.z);
							tempdirection=glm::normalize(tempdirection);
							directionsplit=tempdirection;
							break;
					 	}
					 }
				}
					
				//create NT2's Topology
				for(int j=splitNT2beginindex,k=1;j<nearestNT.listNeuron.size();j++,k++)
				{
					NeuronSWC tempSWC;
					if(j==splitNT2beginindex)
						{
							tempSWC.n=1;
							tempSWC.pn=-1;
							tempSWC.x=0;
							tempSWC.y=0;
							tempSWC.z=0;
							tempSWC.type=2;
							splitNT2.listNeuron.append(tempSWC);
        					splitNT2.hashNeuron.insert(tempSWC.n, splitNT2.listNeuron.size()-1);
							continue;
						}
					tempSWC.n=k;
					tempSWC.pn=splitNT2.listNeuron.back().n;
					tempSWC.x=0;
					tempSWC.y=0;
					tempSWC.z=0;
					tempSWC.type=2;
					splitNT2.listNeuron.append(tempSWC);
        			splitNT2.hashNeuron.insert(tempSWC.n, splitNT2.listNeuron.size()-1);
					splitNT2size++;
				}
				time_t timer2;
					struct tm y2k = { 0 };
					double seconds;
					y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
					y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1; // seconds since January 1, 2000 in UTC
					time(&timer2);  /* get current time; same as: timer = time(NULL)  */
					seconds = difftime(timer2, timegm(&y2k)); //Timestamp LMG 27/9/2018
					qDebug("Timestamp at m_drawMode (VR) (seconds since January 1, 2000 in UTC): %.0f", seconds);
				//copy data from second half into NT2
				for(int j=splitNT2beginindex,k=0;j<nearestNT.listNeuron.size();j++,k++)
				{
					if(j==splitNT2beginindex)
					{	
						splitNT2.listNeuron[k].x=nearestNode.x - directionsplit.x;
						splitNT2.listNeuron[k].y=nearestNode.y - directionsplit.y;
						splitNT2.listNeuron[k].z=nearestNode.z - directionsplit.z;
						splitNT2.listNeuron[k].r=nearestNT.listNeuron[j].r;
						splitNT2.listNeuron[k].creatmode = nearestNT.listNeuron[j].creatmode;
						splitNT2.listNeuron[k].timestamp = seconds;
						splitNT2.listNeuron[k].type=nearestNT.listNeuron[j].type;
						continue;
					}
					splitNT2.listNeuron[k].x=nearestNT.listNeuron[j].x;
					splitNT2.listNeuron[k].y=nearestNT.listNeuron[j].y;
					splitNT2.listNeuron[k].z=nearestNT.listNeuron[j].z;
					splitNT2.listNeuron[k].r=nearestNT.listNeuron[j].r;
					splitNT2.listNeuron[k].creatmode = nearestNT.listNeuron[j].creatmode;
					splitNT2.listNeuron[k].timestamp = seconds;
					splitNT2.listNeuron[k].type=nearestNT.listNeuron[j].type;
				}

				//add the two new NTs to NTList
				splitNT1.name = "sketch_"+QString("%1").arg(sketchNum++);
				qDebug()<<splitNT1.name;
				sketchedNTList.push_back(splitNT1);
				int lIndex = sketchedNTList.size() - 1;
				qDebug()<<"index = "<<lIndex;
				SetupSingleMorphologyLine(lIndex,0);
				splitNT2.name = "sketch_"+QString("%1").arg(sketchNum++);
				qDebug()<<splitNT2.name;
				sketchedNTList.push_back(splitNT2);
				lIndex = sketchedNTList.size() - 1;
				qDebug()<<"index = "<<lIndex;
				SetupSingleMorphologyLine(lIndex,0);

				break;
			}
			}
		
		default :
			break;
		}

		//every time the trigger(right) is unpressd ,set the vertexcount to zero preparing for the next line
		vertexcount=0;
		READY_TO_SEND=true;
		qDebug()<<"READY_TO_SEND=true;";
	}
	if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_Grip)&&(event.eventType==vr::VREvent_ButtonUnpress))
	{	//use grip button(right) to change mode draw/delelte/drag/drawmarker/deletemarker

		// m_modeControlGrip_R++;
		// m_modeControlGrip_R%=6;
		// switch(m_modeControlGrip_R)
		// {
		// case 0:
		// 	m_modeGrip_R = m_drawMode;
		// 	break;
		// case 1:
		// 	m_modeGrip_R = m_deleteMode;
		// 	break;
		// case 2:
		// 	m_modeGrip_R = m_dragMode;
		// 	break;
		// case 3:
		// 	m_modeGrip_R = m_markMode;
		// 	break;
		// case 4:
		// 	m_modeGrip_R = m_delmarkMode;
		// 	break;
		// case 5:
		// 	m_modeGrip_R = m_splitMode;
		// 	break;
		// default:
		// 	break;
		// }	
		// qDebug("m_modeGrip_R=%d",m_modeGrip_R);
		//grip right button is used to control linewidth for now
		iLineWid+=2;
		if(iLineWid>9){iLineWid = 1;}

				
	}
	if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_ApplicationMenu)&&(event.eventType==vr::VREvent_ButtonPress))
	{
				//this func can 1.call top menu
				//2.return to top menu from second menu and hide second menu meantime
				//3.hide top menu
				//4.when no menu show ,hide ray
				showshootingPad = !showshootingPad; //show virtualPad
				if (showshootingPad)
					m_secondMenu = _nothing;
				if (!showshootingPad && m_secondMenu == _nothing) //when top menu and second menu dont show ,then hide ray
					showshootingray = false;
	}

		//save swc
		//if(sketchedNTList.size()>0)
		//{   //save swc to file
		//	QDateTime mytime = QDateTime::currentDateTime();
		//	QString imageName = "FILE";
		//	if (img4d) imageName = img4d->getFileName();
		//	QStringList qsl = imageName.trimmed().split("/",QString::SkipEmptyParts);
		//	QString name = qsl.back();
		//	QString filename = QCoreApplication::applicationDirPath()+"/annotations_VR_" + name + "_" + mytime.toString("yyyy_MM_dd_hh_mm") + ".swc";
		//	//shift the neuron nodes to get global coordinates
		//	NeuronTree mergedSketchNTL;
		//	MergeNeuronTrees(mergedSketchNTL,&sketchedNTList);
		//	writeSWC_file(filename, mergedSketchNTL);	
		//	qDebug("Successfully writeSWC_file");
		//}

		//save markers
		//if(!drawnMarkerList.empty())
		//{
		//	//save marker to file
		//	QDateTime mytime = QDateTime::currentDateTime();
		//	QString imageName = "FILE";
		//	if (img4d) imageName = img4d->getFileName();
		//	QStringList qsl = imageName.trimmed().split("/",QString::SkipEmptyParts);
		//	QString name = qsl.back();
		//	QString filename = QCoreApplication::applicationDirPath()+"/annotations_VR_" + name + "_" + mytime.toString("yyyy_MM_dd_hh_mm") + ".marker";

		//	writeMarker_file(filename,drawnMarkerList);
		//	qDebug("Successfully writeMarker_file");
		//}
		// qDebug("before call");
		// if(!call_neuron_assembler_live_plugin(mainwindow)) return;
		// qDebug("after call");
		// _call_assemble_plugin = true;
		// postVRFunctionCallMode = 1;


		//2018-06-22 now this button is used for showing next two markers in sequence.
		//2018-07-27 now this button is used for virtual button
		// if (curveDrawingTestStatus == -1) //test not started;
		// {
		// 	if (markerVisibility.size() >= 2)
		// 	{
		// 		for(int i=2; i<markerVisibility.size();i++) markerVisibility[i] = 0;
		// 		curveDrawingTestStatus = 1;//test started. //id of the second current visible marker.
		// 		elapsedTimes.clear();
		// 		timer1.start();
		// 	}
		// }
		// else
		// {
		// 	qint64 etime = timer1.elapsed();
		// 	elapsedTimes.push_back(etime);
		// 	timer1.restart();
		// 	markerVisibility[curveDrawingTestStatus] = markerVisibility[curveDrawingTestStatus-1] = 0;
		// 	if (markerVisibility.size() > curveDrawingTestStatus+2)
		// 	{
		// 		curveDrawingTestStatus += 2;
		// 		markerVisibility[curveDrawingTestStatus] = markerVisibility[curveDrawingTestStatus-1] = 1;
		// 	}
		// 	else // no more enough markers, test termination
		// 	{
		// 		for(int i=0; i<markerVisibility.size();i++) markerVisibility[i] = 1;
		// 		curveDrawingTestStatus = -1;//test not started;
		// 		for(int i=0; i<elapsedTimes.size();i++) qDebug()<<elapsedTimes[i]<< " milliseconds";

        //         //output time
        //         if(1)
        //         {
        //             QDateTime mytime = QDateTime::currentDateTime();
        //             QString imageName = "FILE";
        //             if (img4d) imageName = img4d->getFileName();
        //             QStringList qsl = imageName.trimmed().split("/",QString::SkipEmptyParts);
        //             QString name = qsl.back();
        //             QString filename = imageName + "_VR_" + mytime.toString("yyyy_MM_dd_hh_mm") + ".txt";
        //             //QString filename = "["+ name +  "]_VR_" + mytime.toString("yyyy_MM_dd_hh_mm") + ".txt"; //QCoreApplication::applicationDirPath()+
        //             ofstream outFile(filename.toStdString(), ofstream::out);
        //             for(int i=0; i<elapsedTimes.size();i++) outFile << elapsedTimes[i] / 1000 << endl;
        //             outFile.close();
        //         }

        //         //output swc
        //         if(sketchedNTList.size()>0)
        //         {   //save swc to file
        //             QDateTime mytime = QDateTime::currentDateTime();
        //             QString imageName = "FILE";
        //             if (img4d) imageName = img4d->getFileName();
        //             QStringList qsl = imageName.trimmed().split("/",QString::SkipEmptyParts);
        //             QString name = qsl.back();
        //             QString filename = imageName + "_VR_" + mytime.toString("yyyy_MM_dd_hh_mm") + ".swc";
        //             //QString filename = "["+ name +  "]_VR_" + mytime.toString("yyyy_MM_dd_hh_mm") + ".swc"; //QCoreApplication::applicationDirPath()+
        //             //shift the neuron nodes to get global coordinates
        //             NeuronTree mergedSketchNTL;
        //             MergeNeuronTrees(mergedSketchNTL,&sketchedNTList);
        //             writeSWC_file(filename, mergedSketchNTL);
        //             qDebug("Successfully writeSWC_file");
        //         }
		// 	}
		// }

				

	
	//////////////////
}
bool CMainApplication::isAnyNodeOutBBox(NeuronSWC S_temp)
{
	if((S_temp.x<swcBB.x0)||(S_temp.y<swcBB.y0)||(S_temp.z<swcBB.z0)
		||(S_temp.x>swcBB.x1)||(S_temp.y>swcBB.y1)||(S_temp.z>swcBB.z1))
		return true;
	else
		return false;
}

//merge NTlist to Neuron Tree
void CMainApplication::MergeNeuronTrees(NeuronTree &ntree, const QList<NeuronTree> * NTlist)
{


	for(int i=0;i<NTlist->size();i++)
	{
		NeuronTree  nt = NTlist->at(i);
		
		for(int j=0;j<nt.listNeuron.size();j++)
		{
			int treeNum = ntree.listNeuron.size();
			NeuronSWC ss = nt.listNeuron.at(j);
			
			if(ss.pn!=-1) ss.pn = ss.pn-ss.n+treeNum+1;
			ss.n = treeNum+1;

			ntree.listNeuron.append(ss);
			ntree.hashNeuron.insert(ss.n, ntree.listNeuron.size()-1);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderFrame()
{
	// for now as fast as possible 
	if ( m_pHMD )
	{
		QString AgentsNum = QString("%1").arg(Agents_spheres.size()+1);
		std::string strWindowTitle = "TeraVR [Username: "+current_agent_name+
			"][Color: "+current_agent_color+"][#Online users: "+AgentsNum.toStdString() + "]";
		SDL_SetWindowTitle( m_pCompanionWindow, strWindowTitle.c_str() );
		RenderControllerAxes();
		SetupControllerTexture();
		SetupControllerRay();
		SetupMorphologyLine(1);//for currently drawn stroke(currentNT)
		//FlashStuff(m_flashtype,FlashCoords);
		//for all (synchronized) sketched strokes
		//SetupMorphologySurface(currentNT,sketch_spheres,sketch_cylinders,sketch_spheresPos);
		//SetupMorphologyLine(currentNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,1);
		 
		if (m_autoRotateON) //auto rotation is on
		{
			m_globalMatrix = glm::translate(m_globalMatrix,autoRotationCenter);
			m_globalMatrix = glm::rotate(m_globalMatrix,0.01f,glm::vec3(1,0,0));
			m_globalMatrix = glm::rotate(m_globalMatrix,0.01f,glm::vec3(0,1,0));
			m_globalMatrix = glm::translate(m_globalMatrix,-autoRotationCenter);
		}

		RenderStereoTargets();
		RenderCompanionWindow();
		vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
		vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
	}

	if ( m_bVblank && m_bGlFinishHack )
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}
	// SwapWindow
	{
		SDL_GL_SwapWindow( m_pCompanionWindow );
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor( 0, 0, 0, 1 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	// Flush and wait for swap.
	if ( m_bVblank )
	{
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if ( m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last )
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;//question: what are these? why keep count and count_last? just to observe change?
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;//note: maybe used for drawing controllers
		
		dprintf( "PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
	}
	UpdateHMDMatrixPose();
}


//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason.
//-----------------------------------------------------------------------------
GLuint CMainApplication::CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader )
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource( nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader( nSceneVertexShader );

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if ( vShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneVertexShader );
		return 0;
	}
	glAttachShader( unProgramID, nSceneVertexShader);
	glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader( nSceneFragmentShader );

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader );
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneFragmentShader );
		return 0;	
	}

	glAttachShader( unProgramID, nSceneFragmentShader );
	glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

	glLinkProgram( unProgramID );

	GLint programSuccess = GL_TRUE;
	glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
	if ( programSuccess != GL_TRUE )
	{
		dprintf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram( unProgramID );
		return 0;
	}

	glUseProgram( unProgramID );
	glUseProgram( 0 );

	return unProgramID;
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
bool CMainApplication::CreateAllShaders()//todo: change shader code
{
	QString qappDirPath = QCoreApplication::applicationDirPath(); 
	morphologyShader = new Shader(string(qappDirPath.toStdString()+"/materials/basic.vert").c_str(), string(qappDirPath.toStdString()+"/materials/basic.frag").c_str());

	if (m_bHasImage4D) CreateVolumeRenderingShaders();

	m_unCtrTexProgramID = CompileGLShader( 
		"Scene",

		// Vertex Shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVcoordsIn;\n"
		"layout(location = 2) in vec3 v3NormalIn;\n"
		"out vec2 v2UVcoords;\n"
		"void main()\n"
		"{\n"
		"	v2UVcoords = v2UVcoordsIn;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// Fragment Shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"in vec2 v2UVcoords;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture(mytexture, v2UVcoords);\n"
		"}\n"
		);
	m_nCtrTexMatrixLocation = glGetUniformLocation( m_unCtrTexProgramID, "matrix" );
	if( m_nCtrTexMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in scene shader\n" );
		return false;
	}
	m_unControllerTransformProgramID = CompileGLShader(
		"Controller",//note: for axes

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
		);
	m_nControllerMatrixLocation = glGetUniformLocation( m_unControllerTransformProgramID, "matrix" );
	if( m_nControllerMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in controller shader\n" );
		return false;
	}

	m_unRenderModelProgramID = CompileGLShader( 
		"render model",//note: for 3D controller model

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matrix * vec4(position.xyz, 1);\n"
		"}\n",

		//fragment shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture( diffuse, v2TexCoord);\n"
		"}\n"

		);
	m_nRenderModelMatrixLocation = glGetUniformLocation( m_unRenderModelProgramID, "matrix" );
	if( m_nRenderModelMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in render model shader\n" );
		return false;
	}

	m_unCompanionWindowProgramID = CompileGLShader(
		"CompanionWindow",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main()\n"
		"{\n"
		"	v2UV = v2UVIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"		outputColor = texture(mytexture, v2UV);\n"
		"}\n"
		);

		//for  shootingrayshader
	m_unControllerRayProgramID = CompileGLShader(
		"ControllerRay",
		// vertex shader
		"#version 410 core\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec3 color ;\n"
		"out vec4 fcolor;\n"
		"void main()\n"
		"{\n"
		"	fcolor.xyz = color; fcolor.a = 1.0;\n"
		"   vec4 middlepos = vec4(position,1.0);\n"
		"	gl_Position = matrix * middlepos;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"in vec4 fcolor;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"		outputColor = fcolor;\n"
		"}\n"
		);
	m_nControllerRayMatrixLocation = glGetUniformLocation( m_unControllerRayProgramID, "matrix" );
		if( m_nControllerRayMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in controllerRay shader\n" );
		return false;
	}
	return m_unControllerTransformProgramID != 0
		&& m_unRenderModelProgramID != 0
		&& m_unCompanionWindowProgramID != 0
		&& m_unControllerRayProgramID!= 0;
}
bool CMainApplication::SetupTexturemaps()
{
	//std::string sExecutableDirectory = Path_StripFilename( Path_GetExecutablePath() );
	//std::string strFullPath = Path_MakeAbsolute( "../cube_texture.png", sExecutableDirectory );
	//qDebug()<<"current applicationDirPath: "<<QCoreApplication::applicationDirPath();  
	//qDebug()<<"current currentPath: "<<QDir::currentPath();
	QString qstrFullPath = QCoreApplication::applicationDirPath() +"/materials/controller_texture.png";

	//std::string strFullPath ="../materials/controller_texture.png";//C:/Users/penglab/Documents/GitHub/v3d_external/v3d_main/v3d/release/
	std::vector<unsigned char> imageRGBA;
	unsigned nImageWidth, nImageHeight;
	unsigned nError = lodepng::decode( imageRGBA, nImageWidth, nImageHeight, qstrFullPath.toStdString());//strFullPath.c_str() );
	
	if ( nError != 0 )
		return false;
	for(int i = 0;i < nImageWidth; i ++)
	{
		for(int j = 0; j < nImageHeight; j ++)
		{
			if(imageRGBA[(j*nImageWidth + i)*4+1] > 225 && imageRGBA[(j*nImageWidth + i)*4+2] > 225)
			{
				imageRGBA[(j*nImageWidth + i)*4+3] = 1;
			}
			else
			{
				imageRGBA[(j*nImageWidth + i)*4+3] = 255;
			}
		}
	}
	glGenTextures(1, &m_iTexture );
	glBindTexture( GL_TEXTURE_2D, m_iTexture );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nImageWidth, nImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, &imageRGBA[0] );

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	 	
	glBindTexture( GL_TEXTURE_2D, 0 );

	return ( m_iTexture != 0 );
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::AddVertex( float fl0, float fl1, float fl2, float fl3, float fl4, std::vector<float> &vertdata )
{
	vertdata.push_back( fl0 );
	vertdata.push_back( fl1 );
	vertdata.push_back( fl2 );
	vertdata.push_back( fl3 );
	vertdata.push_back( fl4 );
}
void CMainApplication::SetupControllerTexture()
{
	if ( !m_pHMD )
		return;

	std::vector<float> vcVerts;

	const Matrix4 & mat_L = m_rmat4DevicePose[m_iControllerIDLeft];
	const Matrix4 & mat_R = m_rmat4DevicePose[m_iControllerIDRight];
	// save the right controller position to ctrSpherePos
	Vector4 ctrPOS = mat_R * Vector4(0,0,0,1);
	ctrSpherePos = glm::vec3(ctrPOS.x,ctrPOS.y,ctrPOS.z);

	//left controller
	{
		Vector4 point_A(-0.023f,-0.009f,0.065f,1);//grip no.1 dispaly "Mode Switch"
		Vector4 point_B(-0.023f,-0.009f,0.105f,1);
		Vector4 point_C(-0.02f,-0.025f,0.065f,1);
		Vector4 point_D(-0.02f,-0.025f,0.105f,1);
		point_A = mat_L * point_A;
		point_B = mat_L * point_B;
		point_C = mat_L * point_C;
		point_D = mat_L * point_D;
		//Update PadFunc
		// AddVertex(point_A.x,point_A.y,point_A.z,0,0.25f,vcVerts);
		// AddVertex(point_B.x,point_B.y,point_B.z,0.34f,0.25f,vcVerts);
		// AddVertex(point_C.x,point_C.y,point_C.z,0,0.375f,vcVerts);
		// AddVertex(point_C.x,point_C.y,point_C.z,0,0.375f,vcVerts);
		// AddVertex(point_D.x,point_D.y,point_D.z,0.34f,0.375f,vcVerts);
		// AddVertex(point_B.x,point_B.y,point_B.z,0.34f,0.25f,vcVerts);

		Vector4 point_A2(0.023f,-0.009f,0.065f,1);//grip no.2 dispaly "Mode Switch"
		Vector4 point_B2(0.023f,-0.009f,0.105f,1);
		Vector4 point_C2(0.02f,-0.025f,0.065f,1);
		Vector4 point_D2(0.02f,-0.025f,0.105f,1);
		point_A2 = mat_L * point_A2;
		point_B2 = mat_L * point_B2;
		point_C2 = mat_L * point_C2;
		point_D2 = mat_L * point_D2;
		// AddVertex(point_A2.x,point_A2.y,point_A2.z,0.34f,0.25f,vcVerts);
		// AddVertex(point_B2.x,point_B2.y,point_B2.z,0,0.25f,vcVerts);
		// AddVertex(point_C2.x,point_C2.y,point_C2.z,0.34f,0.375f,vcVerts);
		// AddVertex(point_C2.x,point_C2.y,point_C2.z,0.34f,0.375f,vcVerts);
		// AddVertex(point_D2.x,point_D2.y,point_D2.z,0,0.375f,vcVerts);
		// AddVertex(point_B2.x,point_B2.y,point_B2.z,0,0.25f,vcVerts);//*/

		Vector4 point_E(-0.02f,0.01f,0.03f,1);// for the touchpad dispaly "ON/OFF"
		Vector4 point_F(0.02f,0.01f,0.03f,1);
		Vector4 point_G(-0.02f,0.01f,0.07f,1);
		Vector4 point_H(0.02f,0.01f,0.07f,1);
		point_E = mat_L * point_E;
		point_F = mat_L * point_F;
		point_G = mat_L * point_G;
		point_H = mat_L * point_H;
		switch(m_modeGrip_L)
		{
		case _donothing:
			break;
		case _Surface:
			{
				if(m_bShowMorphologySurface)
				{

					AddVertex(point_E.x,point_E.y,point_E.z,0.085f,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.165f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);
				}
				else
				{
					AddVertex(point_E.x,point_E.y,point_E.z,0,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.085f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);

				}
				break;
			}
		case _VirtualFinger:
			{
				if(m_bVirtualFingerON)
				{
					AddVertex(point_E.x,point_E.y,point_E.z,0,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.085f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);
				}
				else
				{
					AddVertex(point_E.x,point_E.y,point_E.z,0.085f,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.165f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);

				}
				break;
			}
		case _ColorChange:// display "ok"
		case _ResetImage://display"ok
		case _MovetoCreator:
		case _TeraShift:
			{//_TeraShift
				AddVertex(point_E.x,point_E.y,point_E.z,0.17f,0.5f,vcVerts);
				AddVertex(point_F.x,point_F.y,point_F.z,0.25f,0.5f,vcVerts);
				AddVertex(point_G.x,point_G.y,point_G.z,0.17f,0.625f,vcVerts);
				AddVertex(point_G.x,point_G.y,point_G.z,0.17f,0.625f,vcVerts);
				AddVertex(point_H.x,point_H.y,point_H.z,0.25f,0.625f,vcVerts);
				AddVertex(point_F.x,point_F.y,point_F.z,0.25f,0.5f,vcVerts);
				break;
			}	
		case _Freeze:
			{
				if(m_bFrozen)
				{

					AddVertex(point_E.x,point_E.y,point_E.z,0.085f,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.165f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);

				}
				else
				{

					AddVertex(point_E.x,point_E.y,point_E.z,0,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.085f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);


				}
				break;
			}
		case _TeraZoom:
		case _LineWidth:
		case _StretchImage:
		case _RGBImage:
		case _Contrast:
			{
				AddVertex(point_E.x,point_E.y,point_E.z,0.25,0.125f,vcVerts);
				AddVertex(point_F.x,point_F.y,point_F.z,0.335f,0.125f,vcVerts);
				AddVertex(point_G.x,point_G.y,point_G.z,0.25,0.25f,vcVerts);
				AddVertex(point_G.x,point_G.y,point_G.z,0.25,0.25f,vcVerts);
				AddVertex(point_H.x,point_H.y,point_H.z,0.335f,0.25f,vcVerts);
				AddVertex(point_F.x,point_F.y,point_F.z,0.335f,0.125f,vcVerts);
				break;
			}
		case _AutoRotate:
			{
				if(m_autoRotateON)
				{
					AddVertex(point_E.x,point_E.y,point_E.z,0,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.085f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.085f,0.375f,vcVerts);
				}
				else
				{
					AddVertex(point_E.x,point_E.y,point_E.z,0.085f,0.375f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_G.x,point_G.y,point_G.z,0.085f,0.5f,vcVerts);
					AddVertex(point_H.x,point_H.y,point_H.z,0.165f,0.5f,vcVerts);
					AddVertex(point_F.x,point_F.y,point_F.z,0.165f,0.375f,vcVerts);

				}
				break;
			}
		case _UndoRedo:
			{
				AddVertex(point_E.x,point_E.y,point_E.z,0.17f,0.125f,vcVerts);
				AddVertex(point_F.x,point_F.y,point_F.z,0.25,0.125f,vcVerts);
				AddVertex(point_G.x,point_G.y,point_G.z,0.17f,0.25f,vcVerts);
				AddVertex(point_G.x,point_G.y,point_G.z,0.17f,0.25f,vcVerts);
				AddVertex(point_H.x,point_H.y,point_H.z,0.25,0.25f,vcVerts);
				AddVertex(point_F.x,point_F.y,point_F.z,0.25,0.125f,vcVerts);
				break;			
			}

		default:
			break;
		}

		Vector4 point_I(-0.01f,0.01f,0.01f,1);//for the menu button dispaly "QUIT"
		Vector4 point_J(0.01f,0.01f,0.01f,1);
		Vector4 point_K(-0.01f,0.01f,0.03f,1);
		Vector4 point_L(0.01f,0.01f,0.03f,1);//*/
		point_I = mat_L * point_I;
		point_J = mat_L * point_J;
		point_K = mat_L * point_K;
		point_L = mat_L * point_L;
		AddVertex(point_I.x,point_I.y,point_I.z,0.085f,0,vcVerts);
		AddVertex(point_J.x,point_J.y,point_J.z,0.17f,0,vcVerts);
		AddVertex(point_K.x,point_K.y,point_K.z,0.085f,0.125f,vcVerts);
		AddVertex(point_K.x,point_K.y,point_K.z,0.085f,0.125f,vcVerts);
		AddVertex(point_L.x,point_L.y,point_L.z,0.17f,0.125f,vcVerts);
		AddVertex(point_J.x,point_J.y,point_J.z,0.17f,0,vcVerts);


		Vector4 point_M(-0.02f,0.005f,0.1f,1);//for the current mode dispaly "Surface/line  Virtual finger  search ....."
		Vector4 point_N(0.02f,0.005f,0.1f,1);
		Vector4 point_O(-0.02f,0.002f,0.14f,1);
		Vector4 point_P(0.02f,0.002f,0.14f,1);//*/
		point_M = mat_L * point_M;
		point_N = mat_L * point_N;
		point_O = mat_L * point_O;
		point_P = mat_L * point_P;


		switch(m_modeGrip_L)
		{
		case _donothing:
			{//donothing
				break;
			}
		case _Surface:
			{//surface
				AddVertex(point_M.x,point_M.y,point_M.z,0.085f,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.17f,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.085f,0.25f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.085f,0.25f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.17f,0.25f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.17f,0.125f,vcVerts);
				break;
			}
		case _TeraShift:
			{//_TeraShift
				AddVertex(point_M.x,point_M.y,point_M.z,0.42f,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5f,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42f,0.25f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42f,0.25f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.5,0.25f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5,0.125f,vcVerts);
				break;
			}
		case _VirtualFinger:
			{//virtual finger
				AddVertex(point_M.x,point_M.y,point_M.z,0.17f,0,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.25f,0,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.17f,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.17f,0.125f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.25f,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.25f,0,vcVerts);
				break;
			}
		case _Freeze:
			{//freeze
				AddVertex(point_M.x,point_M.y,point_M.z,0.335f,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.25f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.25f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.42f,0.25f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.125f,vcVerts);


				
				break;
			}
		case _LineWidth:
			{
				AddVertex(point_M.x,point_M.y,point_M.z,0.335f,0,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.125f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.42f,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0,vcVerts);
				break;
			}
		case _StretchImage:
		{
				AddVertex(point_M.x,point_M.y,point_M.z,0.335f,0.375f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.375f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.5f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.5f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.42f,0.5f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.375f,vcVerts);

			break;
		}
		case _AutoRotate:
			{
				AddVertex(point_M.x,point_M.y,point_M.z,0.42f,0,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5f,0,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42f,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42f,0.125f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.5f,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5f,0,vcVerts);
				break;
			}
		case _Contrast:
			{//_Contrast
				AddVertex(point_M.x,point_M.y,point_M.z,0,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.085f,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0,0.25f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0,0.25f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.085f,0.25f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.085f,0.125f,vcVerts);
				break;
			}
		case _TeraZoom:
			{//_TeraZoom
				AddVertex(point_M.x,point_M.y,point_M.z,0.42f,0.5f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5,0.5f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42f,0.625f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42f,0.625f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.5,0.625f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5,0.5f,vcVerts);
				break;
			}	
		case _ColorChange:
			{//_ColorChange
				AddVertex(point_M.x,point_M.y,point_M.z,0.335f,0.5f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.5f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.625f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.625f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.42f,0.625f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.5f,vcVerts);
				break;
			}
		case _ResetImage:
			{
				AddVertex(point_M.x,point_M.y,point_M.z,0.085,0.625f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.17,0.625f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.085,0.75f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.085,0.75f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.17,0.75f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.17,0.625f,vcVerts);
				break;
			}
		case _RGBImage:
			{
				AddVertex(point_M.x,point_M.y,point_M.z,0.17,0.625f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.25,0.625f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.17,0.75f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.17,0.75f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.25,0.75f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.25,0.625f,vcVerts);
				break;
			}
		case _MovetoCreator:
			{
				AddVertex(point_M.x,point_M.y,point_M.z,0.335,0.635f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42,0.635f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335,0.75f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.335,0.75f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.42,0.75f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.42,0.635f,vcVerts);
			}
		case _UndoRedo:
		default:
			break;

		}
	}
	// right controller
	{
		Vector4 point_A(-0.023f,-0.009f,0.065f,1);//grip dispaly "Mode Switch: draw /delete /marker /pull"
		Vector4 point_B(-0.023f,-0.009f,0.105f,1);
		Vector4 point_C(-0.02f,-0.025f,0.065f,1);
		Vector4 point_D(-0.02f,-0.025f,0.105f,1);
		point_A = mat_R * point_A;
		point_B = mat_R * point_B;
		point_C = mat_R * point_C;
		point_D = mat_R * point_D;//*/

		// AddVertex(point_A.x,point_A.y,point_A.z,0,0.25f,vcVerts);
		// AddVertex(point_B.x,point_B.y,point_B.z,0.34f,0.25f,vcVerts);
		// AddVertex(point_C.x,point_C.y,point_C.z,0,0.375f,vcVerts);
		// AddVertex(point_C.x,point_C.y,point_C.z,0,0.375f,vcVerts);
		// AddVertex(point_D.x,point_D.y,point_D.z,0.34f,0.375f,vcVerts);
		// AddVertex(point_B.x,point_B.y,point_B.z,0.34f,0.25f,vcVerts);

		Vector4 point_A2(0.023f,-0.009f,0.065f,1);//grip no.2 display "Mode Switch: draw /delete /marker /pull"
		Vector4 point_B2(0.023f,-0.009f,0.105f,1);
		Vector4 point_C2(0.02f,-0.025f,0.065f,1);
		Vector4 point_D2(0.02f,-0.025f,0.105f,1);
		point_A2 = mat_R * point_A2;
		point_B2 = mat_R * point_B2;
		point_C2 = mat_R * point_C2;
		point_D2 = mat_R * point_D2;//*/

		// AddVertex(point_A2.x,point_A2.y,point_A2.z,0.34f,0.25f,vcVerts);
		// AddVertex(point_B2.x,point_B2.y,point_B2.z,0,0.25f,vcVerts);
		// AddVertex(point_C2.x,point_C2.y,point_C2.z,0.34f,0.375f,vcVerts);
		// AddVertex(point_C2.x,point_C2.y,point_C2.z,0.34f,0.375f,vcVerts);
		// AddVertex(point_D2.x,point_D2.y,point_D2.z,0,0.375f,vcVerts);
		// AddVertex(point_B2.x,point_B2.y,point_B2.z,0,0.25f,vcVerts);//*/

		Vector4 point_E(-0.02f,0.01f,0.03f,1);// for the touchpad switch & display "translate /rotate /scale /nothing
		Vector4 point_F(0.02f,0.01f,0.03f,1);
		Vector4 point_G(-0.02f,0.01f,0.07f,1);
		Vector4 point_H(0.02f,0.01f,0.07f,1);
		point_E = mat_R * point_E;
		point_F = mat_R * point_F;
		point_G = mat_R * point_G;
		point_H = mat_R * point_H;
		// switch (m_modeControlTouchPad_R)
		// {
		// case 0://nothing
		// 	{
		// 		AddVertex(point_E.x,point_E.y,point_E.z,0.42f,0.375f,vcVerts);
		// 		AddVertex(point_F.x,point_F.y,point_F.z,0.5f,0.375f,vcVerts);
		// 		AddVertex(point_G.x,point_G.y,point_G.z,0.42f,0.5f,vcVerts);
		// 		AddVertex(point_G.x,point_G.y,point_G.z,0.42f,0.5f,vcVerts);
		// 		AddVertex(point_H.x,point_H.y,point_H.z,0.5f,0.5f,vcVerts);
		// 		AddVertex(point_F.x,point_F.y,point_F.z,0.5f,0.375f,vcVerts);
		// 		break;
		// 	}
		//case 1://translate
		
			//terazoom
	switch(m_modeTouchPad_R)
	{
		case tr_contrast:
		{
			AddVertex(point_E.x, point_E.y, point_E.z, 0.17f, 0.375f, vcVerts);
			AddVertex(point_F.x, point_F.y, point_F.z, 0.25, 0.375f, vcVerts);
			AddVertex(point_G.x, point_G.y, point_G.z, 0.17, 0.5f, vcVerts);
			AddVertex(point_G.x, point_G.y, point_G.z, 0.17, 0.5f, vcVerts);
			AddVertex(point_H.x, point_H.y, point_H.z, 0.25f, 0.5f, vcVerts);
			AddVertex(point_F.x, point_F.y, point_F.z, 0.25f, 0.375f, vcVerts);
			break;
		}
		case tr_clipplane:
		{
			AddVertex(point_E.x, point_E.y, point_E.z, 0.42f,0.635f, vcVerts);
			AddVertex(point_F.x, point_F.y, point_F.z, 0.5,0.635f, vcVerts);
			AddVertex(point_G.x, point_G.y, point_G.z, 0.42f,0.765f, vcVerts);
			AddVertex(point_G.x, point_G.y, point_G.z, 0.42f,0.765f, vcVerts);
			AddVertex(point_H.x, point_H.y, point_H.z, 0.5,0.765f, vcVerts);
			AddVertex(point_F.x, point_F.y, point_F.z, 0.5,0.635f, vcVerts);
			break;
		}
		default:
		break;
	}
				
				//break;
		// case 2://rotate
		// 	{
		// 		AddVertex(point_E.x,point_E.y,point_E.z,0.25f,0.375f,vcVerts);
		// 		AddVertex(point_F.x,point_F.y,point_F.z,0.335f,0.375f,vcVerts);
		// 		AddVertex(point_G.x,point_G.y,point_G.z,0.25f,0.5f,vcVerts);
		// 		AddVertex(point_G.x,point_G.y,point_G.z,0.25f,0.5f,vcVerts);
		// 		AddVertex(point_H.x,point_H.y,point_H.z,0.335f,0.5f,vcVerts);
		// 		AddVertex(point_F.x,point_F.y,point_F.z,0.335f,0.375f,vcVerts);
		// 		break;
		// 	}
		// case 3://scale
		// 	{
		// 		AddVertex(point_E.x,point_E.y,point_E.z,0.335f,0.375f,vcVerts);
		// 		AddVertex(point_F.x,point_F.y,point_F.z,0.42f,0.375f,vcVerts);
		// 		AddVertex(point_G.x,point_G.y,point_G.z,0.335f,0.5f,vcVerts);
		// 		AddVertex(point_G.x,point_G.y,point_G.z,0.335f,0.5f,vcVerts);
		// 		AddVertex(point_H.x,point_H.y,point_H.z,0.42f,0.5f,vcVerts);
		// 		AddVertex(point_F.x,point_F.y,point_F.z,0.42f,0.375f,vcVerts);
		// 		break;
		// 	}
		// default:
		// 	break;
		// }


		Vector4 point_I(-0.01f,0.01f,0.01f,1);//for the menu button dispaly "SAVE"
		Vector4 point_J(0.01f,0.01f,0.01f,1);
		Vector4 point_K(-0.01f,0.01f,0.03f,1);
		Vector4 point_L(0.01f,0.01f,0.03f,1);//*/
		point_I = mat_R * point_I;
		point_J = mat_R * point_J;
		point_K = mat_R * point_K;
		point_L = mat_R * point_L;
		AddVertex(point_I.x,point_I.y,point_I.z,0,0,vcVerts);
		AddVertex(point_J.x,point_J.y,point_J.z,0.085f,0,vcVerts);
		AddVertex(point_K.x,point_K.y,point_K.z,0,0.125f,vcVerts);
		AddVertex(point_K.x,point_K.y,point_K.z,0,0.125f,vcVerts);
		AddVertex(point_L.x,point_L.y,point_L.z,0.085f,0.125f,vcVerts);
		AddVertex(point_J.x,point_J.y,point_J.z,0.085f,0,vcVerts);


		//For NewPadtest
		Vector4 point_lefttop(-0.2f,0.1f,-0.3f,1);
		Vector4 point_righttop(0.2f,0.1f,-0.3f,1);
		Vector4 point_leftbottom(-0.2f,0.02f,0.03f,1);
		Vector4 point_rightbottom(0.2f,0.02f,0.03f,1);
		point_lefttop = mat_L * point_lefttop;
		point_righttop = mat_L * point_righttop;
		point_leftbottom = mat_L * point_leftbottom;
		point_rightbottom = mat_L * point_rightbottom;

		Vector4 colormenu_point_lefttop(-0.1f,0.05f,-0.05f,1);
		Vector4 colormenu_point_righttop(0.1f,0.05f,-0.05f,1);
		Vector4 colormenu_point_leftbottom(-0.1f,0.02f,0.03f,1);
		Vector4 colormenu_point_rightbottom(0.1f,0.02f,0.03f,1);

		colormenu_point_lefttop = mat_L * colormenu_point_lefttop;
		colormenu_point_righttop = mat_L * colormenu_point_righttop;
		colormenu_point_leftbottom = mat_L * colormenu_point_leftbottom;
		colormenu_point_rightbottom = mat_L * colormenu_point_rightbottom;
		if(showshootingPad)
		{
				AddVertex(point_lefttop.x,point_lefttop.y,point_lefttop.z,0.5,0.0f,vcVerts);
				AddVertex(point_righttop.x,point_righttop.y,point_righttop.z,1.0,0.0f,vcVerts);
				AddVertex(point_leftbottom.x,point_leftbottom.y,point_leftbottom.z,0.5,0.5f,vcVerts);
				AddVertex(point_leftbottom.x,point_leftbottom.y,point_leftbottom.z,0.5,0.5f,vcVerts);
				AddVertex(point_rightbottom.x,point_rightbottom.y,point_rightbottom.z,1.0,0.5f,vcVerts);
				AddVertex(point_righttop.x,point_righttop.y,point_righttop.z,1.0,0.0f,vcVerts);
		}
		//add vertex
		//1------2/6
		// |    /|
		// |   / |
		// |  /  |
		// | /   |
		// -------
		// 3/4   5
		else if(m_secondMenu==_colorPad)
		{
				AddVertex(colormenu_point_lefttop.x,colormenu_point_lefttop.y,colormenu_point_lefttop.z,0.5518f,0.5487f,vcVerts);
				AddVertex(colormenu_point_righttop.x,colormenu_point_righttop.y,colormenu_point_righttop.z,0.7174f,0.5487f,vcVerts);
				AddVertex(colormenu_point_leftbottom.x,colormenu_point_leftbottom.y,colormenu_point_leftbottom.z,0.5518f,0.6458f,vcVerts);
				AddVertex(colormenu_point_leftbottom.x,colormenu_point_leftbottom.y,colormenu_point_leftbottom.z,0.5518f,0.6458f,vcVerts);
				AddVertex(colormenu_point_rightbottom.x,colormenu_point_rightbottom.y,colormenu_point_rightbottom.z,0.7174,0.6458f,vcVerts);
				AddVertex(colormenu_point_righttop.x,colormenu_point_righttop.y,colormenu_point_righttop.z,0.7174,0.5487f,vcVerts);
		}

		Vector4 point_M(-0.02f,0.005f,0.1f,1);//for the current interact mode dispaly "draw / delete / marker /pull"
		Vector4 point_N(0.02f,0.005f,0.1f,1);
		Vector4 point_O(-0.02f,0.002f,0.13f,1);
		Vector4 point_P(0.02f,0.002f,0.13f,1);//*/
		point_M = mat_R * point_M;
		point_N = mat_R * point_N;
		point_O = mat_R * point_O;
		point_P = mat_R * point_P;


		switch (m_modeGrip_R)
		{
		case m_drawMode:
			{//draw line
				AddVertex(point_M.x,point_M.y,point_M.z,0.17f,0.25f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.25,0.25f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.17f,0.375f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.17f,0.375f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.25,0.375f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.25,0.25f,vcVerts);
				break;
			}
		case m_deleteMode:
			{//delete segment
				AddVertex(point_M.x,point_M.y,point_M.z,0.25,0.25f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.335f,0.25f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.25f,0.375f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.25f,0.375f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.335f,0.375f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.335f,0.25f,vcVerts);
				break;
			}
		// case m_markMode:
		// 	{//draw marker
		// 		AddVertex(point_M.x,point_M.y,point_M.z,0.335,0.25f,vcVerts);
		// 		AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.25f,vcVerts);
		// 		AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.375f,vcVerts);
		// 		AddVertex(point_O.x,point_O.y,point_O.z,0.335f,0.375f,vcVerts);
		// 		AddVertex(point_P.x,point_P.y,point_P.z,0.42f,0.375f,vcVerts);
		// 		AddVertex(point_N.x,point_N.y,point_N.z,0.42f,0.25f,vcVerts);
		// 		break;
		// 	}
		case m_dragMode:
			{//pull node
				AddVertex(point_M.x,point_M.y,point_M.z,0.42,0.25f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5,0.25f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42,0.375f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.42,0.375f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.5,0.375f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.5,0.25f,vcVerts);
				break;
			}
		case m_markMode:
			{// marker mode
				AddVertex(point_M.x,point_M.y,point_M.z,0.25,0,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.335,0,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.25,0.125f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.25,0.125f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.335,0.125f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.335,0,vcVerts);
				break;
			}
		case m_splitMode:
			{//split line mode
				AddVertex(point_M.x,point_M.y,point_M.z,0.0,0.625f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.08,0.625f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.0,0.75f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.0,0.75f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.08,0.75f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.08,0.625f,vcVerts);
			}
		case m_insertnodeMode:
			{//split line mode
				AddVertex(point_M.x,point_M.y,point_M.z,0.25,0.625f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.335,0.625f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.25,0.75f,vcVerts);
				AddVertex(point_O.x,point_O.y,point_O.z,0.25,0.75f,vcVerts);
				AddVertex(point_P.x,point_P.y,point_P.z,0.335,0.75f,vcVerts);
				AddVertex(point_N.x,point_N.y,point_N.z,0.335,0.625f,vcVerts);
			}

		default:
			break;
		}
	}

	m_uiControllerTexIndexSize = vcVerts.size()/5;
	// Setup the VAO the first time through.

	if(m_ControllerTexVAO==0)

	{
		//setup vao and vbo stuff

		glGenVertexArrays(1, &m_ControllerTexVAO);

		glGenBuffers(1, &m_ControllerTexVBO);



		//now allocate buffers

		glBindVertexArray(m_ControllerTexVAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_ControllerTexVBO);


		glEnableVertexAttribArray(0);
		GLsizei stride = sizeof(VertexDataScene);
		uintptr_t offset = 0;

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset =  sizeof( Vector3 );

		glEnableVertexAttribArray( 1 );

		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}
		glBindBuffer(GL_ARRAY_BUFFER, m_ControllerTexVBO);

	if( vcVerts.size() > 0 )

	{
		glBufferData(GL_ARRAY_BUFFER,  sizeof(float) * vcVerts.size(), &vcVerts[0], GL_STREAM_DRAW );
	}	
}
void CMainApplication::AddrayVertex(float fl0, float fl1, float fl2, float fl3, float fl4,float fl5, std::vector<float> &vertdata)
{
	vertdata.push_back( fl0 );
	vertdata.push_back( fl1 );
	vertdata.push_back( fl2 );
	vertdata.push_back( fl3 );
	vertdata.push_back( fl4 );
	vertdata.push_back( fl5 );
}
void CMainApplication::SetupControllerRay()
{	
	if ( !m_pHMD )
		return;

	std::vector<float> vcVerts;
	const Matrix4 & mat_R = m_rmat4DevicePose[m_iControllerIDRight];
	const Matrix4 & mat_L = m_rmat4DevicePose[m_iControllerIDLeft];
	std::vector<float> vertdataarray;

	Vector4 start = mat_R * Vector4( 0, 0, -0.02f, 1 );
	Vector4 end = mat_R * Vector4( 0, 0, -39.f, 1 );
	Vector3 color( 1.0f, 0.0f, 0.0f );
	m_uiControllerRayVertcount = 0;


	//update ray endpos if ray intersect the pad
	glm::vec2 ShootingPadUV = calculateshootingPadUV();// update shootingraycutPos
	// Vector4 point_lefttop(-0.2f,0.1f,-0.3f,1);
	// Vector4 point_righttop(0.2f,0.1f,-0.3f,1);
	// Vector4 point_leftbottom(-0.2f,0.02f,0.03f,1);
	// Vector4 point_rightbottom(0.2f,0.02f,0.03f,1);
	// point_lefttop = mat_L * point_lefttop;
	// point_righttop = mat_L * point_righttop;
	// point_leftbottom = mat_L * point_leftbottom;
	// point_rightbottom = mat_L * point_rightbottom;

	if((ShootingPadUV.x> 0)&&(ShootingPadUV.y>0)&&(showshootingPad||(m_secondMenu!=_nothing)))
	showshootingray = true;
	else
	showshootingray = false;
	if(((m_secondMenu!=_nothing)||showshootingPad)&&(ShootingPadUV.x> 0)&&(ShootingPadUV.y>0))
	{
		AddrayVertex(start.x,start.y,start.z,color.x,color.y,color.z,vertdataarray);
		AddrayVertex(shootingraycutPos.x,shootingraycutPos.y,shootingraycutPos.z,color.x,color.y,color.z,vertdataarray);
		m_uiControllerRayVertcount += 2;
	}
	//update  red logo on where you choose

	if(m_iControllerRayVAO == 0)
	{
		glGenVertexArrays(1, &m_iControllerRayVAO);

		glGenBuffers(1, &m_iControllerRayVBO);

		glBindVertexArray(m_iControllerRayVAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_iControllerRayVBO);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void *)(3* sizeof(float)));
		glBindVertexArray(0);
		qDebug()<<"set up controllerray VAO ";
	}
		glBindBuffer(GL_ARRAY_BUFFER, m_iControllerRayVBO);

	if( vertdataarray.size() > 0 )
	{
		glBufferData(GL_ARRAY_BUFFER,  sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW );
	}	
}
//void CMainApplication::SetupMorphologySurface(NeuronTree neurontree,vector<Sphere*>& spheres,vector<Cylinder*>& cylinders,vector<glm::vec3>& spheresPos)
//{
//	NeuronSWC S0,S1;
//	for(int i = spheres.size();i<neurontree.listNeuron.size();i++)
//	{
//		S0=neurontree.listNeuron.at(i);
//
//		//draw sphere
//		spheres.push_back(new Sphere((float)(S0.r)));
//		spheresPos.push_back(glm::vec3(S0.x,S0.y,S0.z));
//		if(S0.pn!=-1){
//			S1 = neurontree.listNeuron.at(S0.pn-1);
//			//draw cylinder
//			float dist = glm::sqrt((S1.x-S0.x)*(S1.x-S0.x)+(S1.y-S0.y)*(S1.y-S0.y)+(S1.z-S0.z)*(S1.z-S0.z));
//			cylinders.push_back(new Cylinder(S0.r,S1.r,dist));
//		}
//		//else {//todo-yimin: temp, to be deleted
//		//	cylinders.push_back(new Cylinder(S0.r,S0.r,0));//generate a degenerated cylinder
//		//}
//	}
//}
//
////-----------------------------------------------------------------------------
//// Purpose: prepare data for line mode
////-----------------------------------------------------------------------------
//void CMainApplication::SetupMorphologyLine( int drawMode)//pass 3 parameters: &neuronTree, VAO, VertVBO, IdxVBO
//{
//	if(drawMode==0){
//		SetupMorphologyLine(loadedNT_merged,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,drawMode);
//	}
//	else{
//		SetupMorphologyLine(currentNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,drawMode);
//	}
//
//}//*/
//
////-----------------------------------------------------------------------------
//void CMainApplication::SetupMorphologyLine(NeuronTree neuron_Tree,
//                                           GLuint& LineModeVAO,
//                                           GLuint& LineModeVBO,
//                                           GLuint& LineModeIndex,
//                                           unsigned int& Vertcount,
//                                           int drawMode)
//{
//	vector <glm::vec3> vertices;
//	vector<GLuint> indices;
//
//	NeuronSWC S0,S1;
//    if (neuron_Tree.listNeuron.size()<1)
//		return;
//
//    // try to be consistent with the 3D viewer window, by PHC 20170616
//    const QList <NeuronSWC> & listNeuron = neuron_Tree.listNeuron;
//    const QHash <int, int> & hashNeuron = neuron_Tree.hashNeuron;
//    RGBA8 rgba = neuron_Tree.color;
//    bool on    = neuron_Tree.on;
//    bool editable = neuron_Tree.editable;
//    int cur_linemode = neuron_Tree.linemode;
//    //
//
//    for(int i=0; i<listNeuron.size(); i++)
//	{
//        //S0 = listNeuron.at(i);
//        //by PHC 20170616. also try to fix the bug of nonsorted neuron display
//
//        S1 = listNeuron.at(i);   // at(i) faster than [i]
//        bool valid = false;
//        if (S1.pn == -1) // root end, 081105
//        {
//            S0 = S1;
//            valid = true;
//        }
//        else if (S1.pn >= 0) //change to >=0 from >0, PHC 091123
//        {
//            // or using hash for finding parent node
//            int j = hashNeuron.value(S1.pn, -1);
//            if (j>=0 && j <listNeuron.size())
//            {
//                S0 = listNeuron.at(j);
//                valid = true;
//            }
//        }
//        if (! valid)
//            continue;
//
//        //
//
//		vertices.push_back(glm::vec3(S0.x,S0.y,S0.z));
//
//        glm::vec3 vcolor_load(0,1,0);//green for loaded neuron tree
//
//        //by PHC 20170616. Try to draw as consistent as possible as the 3D viewer
//
//        if (rgba.a==0 || editable) //make the skeleton be able to use the default color by adjusting alpha value
//        {
//            int type = S0.type;
//            if (editable)
//            {
//                int ncolorused = neuron_type_color_num;
//                if (neuron_type_color_num>19)
//                    ncolorused = 19;
//                type = S0.seg_id %(ncolorused -5)+5; //segment color using hanchuan's neuron_type_color
//            }
//            if (type >= 300 && type <= 555 )  // heat colormap index starts from 300 , for sequencial feature scalar visaulziation
//            {
//                vcolor_load[0] =  neuron_type_color_heat[ type - 300][0];
//                vcolor_load[1] =  neuron_type_color_heat[ type - 300][1];
//                vcolor_load[2] =  neuron_type_color_heat[ type - 300][2];
//            }
//            else
//            {
//                vcolor_load[0] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][0];
//                vcolor_load[1] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][1];
//                vcolor_load[2] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][2];
//            }
//        }
//        else
//        {
//            vcolor_load[0] = rgba.c[0];
//            vcolor_load[1] = rgba.c[1];
//            vcolor_load[2] = rgba.c[2];
//        }
//
//        //
//
//        glm::vec3 vcolor_draw(1,0,0);//red for drawing neuron tree
//
//        vertices.push_back((drawMode==0) ? vcolor_load : vcolor_draw);
//
//        //Yimin's original code which does not display nonsorted neuron correctly
///*
//		if (S0.pn != -1)
//		{
//			S1 = neuron_Tree.listNeuron.at(S0.pn-1);
//			indices.push_back(S0.n-1);
//			indices.push_back(S1.n-1);
//		}
//		else
//		{
//			indices.push_back(S0.n-1);
//            indices.push_back(S0.n-1);
//		}
// */
//
//
//        {
//            indices.push_back(S0.n-1);
//            indices.push_back(S1.n-1);
//        }
//	}
//
//	Vertcount = vertices.size();
//
//    if(LineModeVAO == 0)
//    {
//        //setup vao and vbo stuff
//        glGenVertexArrays(1, &LineModeVAO);
//        glGenBuffers(1, &LineModeVBO);
//        glGenBuffers(1, &LineModeIndex);
//
//        //now allocate buffers
//        glBindVertexArray(LineModeVAO);
//
//        glBindBuffer(GL_ARRAY_BUFFER, LineModeVBO);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LineModeIndex);
//
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (GLvoid*)0);
//        uintptr_t offset =  sizeof( glm::vec3 );
//        glEnableVertexAttribArray( 1 );
//        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (const void *)offset);
//
//        glBindVertexArray(0);
//    }
//    glBindBuffer(GL_ARRAY_BUFFER, LineModeVBO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LineModeIndex);
//    if( vertices.size() > 0 )
//    {
//        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0],(drawMode==0)?GL_STATIC_DRAW:GL_DYNAMIC_DRAW);
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0],(drawMode==0)?GL_STATIC_DRAW: GL_DYNAMIC_DRAW);
//    }
//}


void CMainApplication::SetupMorphologySurface(NeuronTree neurontree,vector<Sphere*>& spheres,vector<Cylinder*>& cylinders,vector<glm::vec3>& spheresPos)
{
	const QList <NeuronSWC> & listNeuron = neurontree.listNeuron;
	const QHash <int, int> & hashNeuron = neurontree.hashNeuron;
	
	NeuronSWC S0,S1;
	if(neurontree.listNeuron.size()<1)
	{
		return;
	}//*/
	for(int i = spheres.size();i<listNeuron.size();i++)
	{
		S0=listNeuron.at(i);

		//draw sphere
		spheres.push_back(new Sphere((float)(S0.r)));
		spheresPos.push_back(glm::vec3(S0.x,S0.y,S0.z));
		//draw cylinder
		if(S0.pn == -1){
			cylinders.push_back(new Cylinder(S0.r,S0.r,0,3,2));
		}
		else if (S0.pn >= 0)
		{
			int j = hashNeuron.value(S0.pn, -1);
			if (j>=0 && j <listNeuron.size())
			{
				S1 = listNeuron.at(j);
			} else 
			{
				qDebug("continue hit.\n");
				continue;
			}

			float dist = glm::sqrt((S1.x-S0.x)*(S1.x-S0.x)+(S1.y-S0.y)*(S1.y-S0.y)+(S1.z-S0.z)*(S1.z-S0.z));
			cylinders.push_back(new Cylinder(S0.r,S1.r,dist));
			//qDebug("surface edge---- %d,%d\n",S0.n,S1.n);
		}
		
	}
}

//-----------------------------------------------------------------------------
// Purpose: prepare data for line mode
//-----------------------------------------------------------------------------
void CMainApplication::SetupMorphologyLine( int drawMode)//pass 3 parameters: &neuronTree, VAO, VertVBO, IdxVBO
{
	if(drawMode==0)// 0 means drawmode = 0, which means this function is called out of mainloop
	{
		SetupMorphologyLine(loadedNT_merged,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,drawMode);
	}
	else if(drawMode==1)//1 means drawmode = 1, set up currentNT's VAO&VBO
	{
		SetupMorphologyLine(currentNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,drawMode);
	}
	// else if(drawMode==2)// 2 for setting up sketchNTL's VAO&VBO
	// {
	// 	SetupMorphologyLine(sketchedNT_merged,m_unRemoteMorphologyLineModeVAO,m_glRemoteMorphologyLineModeVertBuffer,m_glRemoteMorphologyLineModeIndexBuffer,m_uiRemoteMorphologyLineModeVertcount,drawMode);
	// }

}//*/


//-----------------------------------------------------------------------------
void CMainApplication::SetupMorphologyLine(NeuronTree neuron_Tree,
											GLuint& LineModeVAO, 
											GLuint& LineModeVBO, 
											GLuint& LineModeIndex,
											unsigned int& Vertcount,
											int drawMode)
{
	loaded_spheresColor.clear();
	
	vector <glm::vec3> vertices;
	vector<GLuint> indices;
	map<int,int> id2loc;//map neuron node id to location in vertices

	NeuronSWC S0,S1;
	//if(neuron_Tree.listNeuron.size()<1)
	//{
	//	vertices.clear();
	//	indices.clear();
	//	//return;
	//}

	// try to be consistent with the 3D viewer window, by PHC 20170616
	const QList <NeuronSWC> & listNeuron = neuron_Tree.listNeuron;
	const QHash <int, int> & hashNeuron = neuron_Tree.hashNeuron;
	RGBA8 rgba = neuron_Tree.color;
	bool on    = neuron_Tree.on;
	bool editable = neuron_Tree.editable;
	int cur_linemode = neuron_Tree.linemode;

	if (rgba.a == 255) rgba.a = 0; //special case

	//qDebug("rgba color---- %d,%d,%d,%d\n",rgba.c[0],rgba.c[1],rgba.c[2],rgba.a);
	//if (editable) qDebug("editable\n"); else qDebug("not editable\n");
	//rgba.a = 0;

	//handle nodes
	for(int i=0; i<listNeuron.size(); i++)
	{
		S1 = listNeuron.at(i);

		//vertices[2*(S1.n-1)] = glm::vec3(S1.x,S1.y,S1.z);
		vertices.push_back(glm::vec3(S1.x,S1.y,S1.z));
		id2loc[S1.n] = i;
		//qDebug("i=%d,S1.n=%d\n",i,S1.n);

		glm::vec3 vcolor_load(0,1,0);//green for loaded neuron tree

		//by PHC 20170616. Try to draw as consistent as possible as the 3D viewer

		if (rgba.a==0 || editable) //make the skeleton be able to use the default color by adjusting alpha value
		{
			int type = S1.type;
			//if (editable) //yw 2018-05-03, this code block assigns single color (pink) to editable neuron, now disabled.
			//{
			//	int ncolorused = neuron_type_color_num;
			//	if (neuron_type_color_num>19)
			//		ncolorused = 19;
			//	type = S0.seg_id %(ncolorused -5)+5; //segment color using hanchuan's neuron_type_color
			//}
			if (type >= 300 && type <= 555 )  // heat colormap index starts from 300 , for sequencial feature scalar visaulziation
			{
				vcolor_load[0] =  neuron_type_color_heat[ type - 300][0];
				vcolor_load[1] =  neuron_type_color_heat[ type - 300][1];
				vcolor_load[2] =  neuron_type_color_heat[ type - 300][2];
			}
			else
			{
				vcolor_load[0] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][0];
				vcolor_load[1] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][1];
				vcolor_load[2] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][2];
				//qDebug("set in 2,type=%d\n",type);
			}
		}
		else
		{
			vcolor_load[0] = rgba.c[0];
			vcolor_load[1] = rgba.c[1];
			vcolor_load[2] = rgba.c[2];
			//qDebug("set in 3\n");
		}
		for(int i=0;i<3;i++) vcolor_load[i] /= 255.0;
		//qDebug("color---- %f,%f,%f\n",vcolor_load[0],vcolor_load[1],vcolor_load[2]);

		glm::vec3 vcolor_draw(0.5,0.5,0.5);// for locally drawn structures that has not been synchronized yet.
		// glm::vec3 vcolor_remote(1,0,0);//red
		//vertices[2*(S1.n-1)+1] = (drawMode==0) ? vcolor_load : vcolor_draw;
		//vertices.push_back((drawMode==0) ? vcolor_load : vcolor_draw);
		if(drawMode==0)
			vertices.push_back(vcolor_load);
		else if(drawMode==1)
			vertices.push_back(vcolor_draw);
		else if(drawMode==2)
			vertices.push_back(vcolor_load);
			//vertices.push_back(vcolor_remote);
		if (drawMode==0) loaded_spheresColor.push_back(vcolor_load);
	}

	//map<int,int>::iterator iter;
	//for (iter = id2loc.begin();iter != id2loc.end();++iter)
	//	qDebug("edge---- %d,%d\n",iter->first,iter->second);

	//handle edges
	for(int i=0; i<listNeuron.size(); i++)
	{
		//by PHC 20170616. also try to fix the bug of nonsorted neuron display

		S1 = listNeuron.at(i);   // at(i) faster than [i]
		bool valid = false;
		if (S1.pn == -1) // root end, 081105
		{
			S0 = S1;
			valid = true;
		}
		else if (S1.pn >= 0) //change to >=0 from >0, PHC 091123
		{
			// or using hash for finding parent node
			int j = hashNeuron.value(S1.pn, -1);
			if (j>=0 && j <listNeuron.size())
			{
				S0 = listNeuron.at(j);
				valid = true;
			}
		} 
		if (! valid)
			continue;

		int loc0, loc1;
		map<int,int>::iterator iter;
		iter = id2loc.find(S0.n);
		loc0 = iter->second;
		indices.push_back(loc0);
		iter = id2loc.find(S1.n);
		loc1 = iter->second;
		indices.push_back(loc1);
	}

	Vertcount = vertices.size();

	if(LineModeVAO ==0)
	{
		//setup vao and vbo stuff
		glGenVertexArrays(1, &LineModeVAO);
		glGenBuffers(1, &LineModeVBO);
		glGenBuffers(1, &LineModeIndex);

		//now allocate buffers
		glBindVertexArray(LineModeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, LineModeVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LineModeIndex);
	 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (GLvoid*)0);
		uintptr_t offset =  sizeof( glm::vec3 );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (const void *)offset);

		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, LineModeVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LineModeIndex);
	if( vertices.size() > 0 )
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0],(drawMode==0)?GL_STATIC_DRAW:GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0],(drawMode==0)?GL_STATIC_DRAW: GL_DYNAMIC_DRAW);

	}		
}

//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void CMainApplication::RenderControllerAxes() //note: note render, actually setup VAO and VBO for axes
{
	// don't draw controllers if somebody else has input focus
	if( m_pHMD->IsInputFocusCapturedByAnotherProcess() )
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;
	m_iTrackedControllerCount = 0;

	for ( vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice )
	{
		if ( !m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		if( m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) != vr::TrackedDeviceClass_Controller )
			continue;

		m_iTrackedControllerCount += 1;

		if( !m_rTrackedDevicePose[ unTrackedDevice ].bPoseIsValid )
			continue;

		const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];

		Vector4 center = mat * Vector4( 0, 0, 0, 1 );

		for ( int i = 0; i < 2; ++i )//note: prepare data for axes and ray
		{
			Vector3 color( 0, 0, 0 );
			Vector4 point( 0, 0, 0, 1 );
			point[i] += 0.05f;  // offset in X, Y, Z
			color[0] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back( center.x );
			vertdataarray.push_back( center.y );
			vertdataarray.push_back( center.z );

			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			vertdataarray.push_back( point.x );
			vertdataarray.push_back( point.y );
			vertdataarray.push_back( point.z );
		
			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			m_uiControllerVertcount += 2;
		}
		
		//prepare the shooting ray

		// if(showshootingPad && (unTrackedDevice ==vr::k_unTrackedDeviceIndex_Hmd + 3))  //only right controller render ray
		// {		Vector4 start = mat * Vector4( 0, 0, -0.02f, 1 );
		// 		Vector4 end = mat * Vector4( 0, 0, -39.f, 1 );
		// 		Vector3 color( .92f, .92f, .71f );

		// 		vertdataarray.push_back( start.x );vertdataarray.push_back( start.y );vertdataarray.push_back( start.z );//note: ray?
		// 		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );

		// 		vertdataarray.push_back( end.x );vertdataarray.push_back( end.y );vertdataarray.push_back( end.z );
		// 		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );
		// 		m_uiControllerVertcount += 2;
		// }
		

		// Setup the VAO the first time through.
		if (m_unControllerVAO == 0)
		{
			glGenVertexArrays(1, &m_unControllerVAO);
			glBindVertexArray(m_unControllerVAO);

			glGenBuffers(1, &m_glControllerVertBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

			GLuint stride = 2 * 3 * sizeof(float);
			uintptr_t offset = 0;

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

			offset += sizeof(Vector3);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

			glBindVertexArray(0);
	}

	glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

	// set vertex data if we have some
	if( vertdataarray.size() > 0 )
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW );
	}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );
}

void CMainApplication::SetupCamerasForMorphology()
{
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			m_EyeTransLeft[i][j]   = *(m_mat4eyePosLeft.get() + i * 4 + j);
			m_EyeTransRight[i][j]  = *(m_mat4eyePosRight.get() + i * 4 + j);
			m_ProjTransLeft[i][j]  = *(m_mat4ProjectionLeft.get() + i * 4 + j);
			m_ProjTransRight[i][j] = *(m_mat4ProjectionRight.get() + i * 4 + j);
		}
	}

	for (size_t i = 0; i < 3; i++)
	{
		m_EyePosLeft[i] = *(m_mat4eyePosLeft.get() + i * 4 + 3);
		m_EyePosRight[i] = *(m_mat4eyePosRight.get() + i * 4 + 3);
	}

}

void CMainApplication::SetupGlobalMatrix()
{
	float r_max = -1;
	swcBB = NULL_BoundingBox;

	

	//if (    !((img4d->getXDim()==0)  &&  (img4d->getYDim()==0)  &&  (img4d->getZDim()==0))    ) 
	if (m_bHasImage4D)
	{
		//have an image in the scene, further adjust the bounding box
		swcBB.expand(BoundingBox(XYZ(0,0,0), XYZ(img4d->getXDim(),img4d->getYDim(),img4d->getZDim())));
	}
	else
	{
		for(int i = 0;i<loadedNT_merged.listNeuron.size();i++)
		{
			NeuronSWC S=loadedNT_merged.listNeuron.at(i);
			float d = S.r * 2;
			swcBB.expand(BoundingBox(XYZ(S) - d, XYZ(S) + d));
			if (S.r > r_max)  r_max = S.r;
		}
	}

	float DX = swcBB.Dx();
	float DY = swcBB.Dy();
	float DZ = swcBB.Dz();
	cout <<"bounding box(swc+image): dx = "<< DX << ", dy = " << DY << ", dz =  " << DZ <<endl;

	
	

	float maxD = swcBB.Dmax();

	//original center location
	loadedNTCenter.x = (swcBB.x0 + swcBB.x1)/2;
	loadedNTCenter.y = (swcBB.y0 + swcBB.y1)/2;
	loadedNTCenter.z = (swcBB.z0 + swcBB.z1)/2;
	qDebug("old: center.x = %f,center.y = %f,center.z = %f\n",loadedNTCenter.x,loadedNTCenter.y,loadedNTCenter.z);

	m_globalScale = 1 / maxD * 2; // these numbers are related to room size
	
	float trans_x = 0.6 ;
	float trans_y = 1.5 ;
	float trans_z = 0.4 ;
	//printf("transform: scale = %f, translate = (%f,%f,%f)\n", scale,trans_x,trans_y,trans_z );
	if(!m_pChaperone)
	{
		cout<<"m_pChaperone is NULL"<<endl;
	}
	vr::HmdQuad_t rect;
	m_pChaperone->GetPlayAreaRect(&rect);
		vr::HmdVector3_t HmdQuadmin = {100.0f,100.0f,100.0f};
		vr::HmdVector3_t HmdQuadmax = {-100.0f,-100.0f,-100.0f};
	for(size_t i=0;i<4;++i)
	{
		vr::HmdVector3_t currentHmdQuadPos = rect.vCorners[i];
		HmdQuadmin.v[0] = HmdQuadmin.v[0]<currentHmdQuadPos.v[0]?HmdQuadmin.v[0]:currentHmdQuadPos.v[0];
		HmdQuadmin.v[1] = HmdQuadmin.v[1]<currentHmdQuadPos.v[1]?HmdQuadmin.v[1]:currentHmdQuadPos.v[1];
		HmdQuadmin.v[2] = HmdQuadmin.v[2]<currentHmdQuadPos.v[2]?HmdQuadmin.v[2]:currentHmdQuadPos.v[2];
		HmdQuadmax.v[0] = HmdQuadmax.v[0]>currentHmdQuadPos.v[0]?HmdQuadmax.v[0]:currentHmdQuadPos.v[0];
		HmdQuadmax.v[1] = HmdQuadmax.v[1]>currentHmdQuadPos.v[1]?HmdQuadmax.v[1]:currentHmdQuadPos.v[1];
		HmdQuadmax.v[2] = HmdQuadmax.v[2]>currentHmdQuadPos.v[2]?HmdQuadmax.v[2]:currentHmdQuadPos.v[2];
	}
	HmdQuadImageOffset.v[0] = 0;
	HmdQuadImageOffset.v[1] = 1.5f;
	HmdQuadImageOffset.v[2] = (HmdQuadmax.v[1]-HmdQuadmin.v[1])/4;
	// cout<<"HmdQuadmin x == "<<HmdQuadmin.v[0]<<"HmdQuadmin y == "<<HmdQuadmin.v[1]<<"HmdQuadmin z == "<<HmdQuadmin.v[2]<<endl;
	// cout<<"HmdQuadmax x == "<<HmdQuadmax.v[0]<<"HmdQuadmax y == "<<HmdQuadmax.v[1]<<"HmdQuadmax z == "<<HmdQuadmax.v[2]<<endl;	

		
	m_globalMatrix = glm::translate(m_globalMatrix,glm::vec3(HmdQuadImageOffset.v[0],HmdQuadImageOffset.v[1],HmdQuadImageOffset.v[2])); 
	//m_globalMatrix = glm::translate(m_globalMatrix,glm::vec3(trans_x,trans_y,trans_z) ); //fine tune
		//glm::vec4 cntr = m_globalMatrix * glm::vec4(loadedNTCenter.x,loadedNTCenter.y,loadedNTCenter.z,1);
		//qDebug("after scaling: center.x = %f,center.y = %f,center.z = %f\n",cntr.x,cntr.y,cntr.z);
	m_globalMatrix = glm::scale(m_globalMatrix,glm::vec3(m_globalScale,m_globalScale,m_globalScale));
	m_globalMatrix = glm::translate(m_globalMatrix,glm::vec3(- loadedNTCenter.x,- loadedNTCenter.y,- loadedNTCenter.z) ); 
	
		//cntr = m_globalMatrix * glm::vec4(loadedNTCenter.x,loadedNTCenter.y,loadedNTCenter.z,1);
		//qDebug("after translation: center.x = %f,center.y = %f,center.z = %f\n",cntr.x,cntr.y,cntr.z);

	

}


//-----------------------------------------------------------------------------
// Purpose: Creates a frame buffer. Returns true if the buffer was set up.
//          Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool CMainApplication::CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc )//note: maybe no need to change
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	framebufferDesc.m_nDepthBufferId );

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId );
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId );
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupStereoRenderTargets()
{
	if ( !m_pHMD )
		return false;

	m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );

	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, leftEyeDesc );
	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, rightEyeDesc );
	
	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCompanionWindow()//question: what's the content here?
{
	if ( !m_pHMD )
		return;

	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back( VertexDataWindow( Vector2(-1, -1), Vector2(0, 0)) );
	vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(1, 0)) );
	vVerts.push_back( VertexDataWindow( Vector2(-1, 1), Vector2(0, 1)) );//fix the problem of scene of the desktop window upside down
	vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(1, 1)) );

	// right eye verts
	vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(0, 0)) );
	vVerts.push_back( VertexDataWindow( Vector2(1, -1), Vector2(1, 0)) );
	vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(0, 1)) );
	vVerts.push_back( VertexDataWindow( Vector2(1, 1), Vector2(1, 1)) );

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6};
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays( 1, &m_unCompanionWindowVAO );
	glBindVertexArray( m_unCompanionWindowVAO );

	glGenBuffers( 1, &m_glCompanionWindowIDVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW );

	glGenBuffers( 1, &m_glCompanionWindowIDIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, position ) );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, texCoord ) );

	glBindVertexArray( 0 );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderStereoTargets()
{
	if (m_bHasImage4D)
		glClearColor( 25.0f/255,  25.0f/255,  64.0f/255,  1.0f );
	else
		glClearColor( 204.0f/255, 217.0f/255, 229.0f/255, 1.0f );
	
	glEnable( GL_MULTISAMPLE );
	// Left Eye
	glBindFramebuffer( GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId ); //render scene to m_nRenderFramebufferId
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
	frameCount++;
	if(GetTime() > 1.0f)
	{
		fps = frameCount;
		frameCount = 0;
		StartTimer();
		//cout<<fps<<endl;
		//used for fps tess liqi
	}	
	
	frameTime = GetFrameTime();
 	RenderScene( vr::Eye_Left );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glDisable( GL_MULTISAMPLE );
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId); //copy m_nRenderFramebufferId to m_nResolveFramebufferId, which is later passed to VR.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId );

    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );	

	glEnable( GL_MULTISAMPLE );
	// Right Eye
	glBindFramebuffer( GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Right );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
 	
	glDisable( GL_MULTISAMPLE );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId );
	
    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR  );
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );
}


//-----------------------------------------------------------------------------
// Purpose: Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void CMainApplication::RenderScene( vr::Hmd_Eye nEye )
{
	glm::vec3 surfcolor(0.95f,0.95f,0.95f);
	glm::vec3 wireframecolor(0.545f, 0.361f, 0.855f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//if(nEye==vr::Eye_Left )
	//{
	//	// render text on Companion Window

	//	// Enable and configure blending for font antialiasing and transparency
	//	glEnable(GL_BLEND);
	//	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	//	// Draw some text with the loaded font
	//	font_VR->setPenPosition(650, 200);
	//	font_VR->setPenColor(0.5,0.5,0.5);
	//	char temp_str[10];
	//	sprintf(temp_str,"%d",Agents_spheres.size()+1);
	//	//char user_str[30]="#CURRENT USERS = ";
	//	//std::strcat(user_str,temp_str);
	//	string user_str="#CURRENT USERS = ";
	//	user_str+=temp_str;
	//	font_VR->draw(user_str);

	//	glDisable(GL_BLEND);
	//}

///*
	//=================== draw volume image ======================
	if (m_bHasImage4D)
	{	
		// render to texture
		glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_frameBufferBackface); 
		backfaceShader->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderImage4D(backfaceShader,nEye,GL_BACK); 
		glUseProgram(0);
		///*
		// bind to previous framebuffer again
		if (nEye == vr::Eye_Left)
		{
			glBindFramebuffer( GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId );
		}
		else if (nEye == vr::Eye_Right)
		{
			glBindFramebuffer( GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
		}
		// ray casting
		raycastingShader->use();
		SetUinformsForRayCasting();
		RenderImage4D(raycastingShader,nEye,GL_FRONT); //*/
		//to make the image not block the morphology surface
		glEnable(GL_DEPTH_TEST);
		//glClear(GL_DEPTH_BUFFER_BIT);
	}
	//=================== draw agent postion with sphere ====================
	{
		morphologyShader->use();
		
		morphologyShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		morphologyShader->setVec3("lightPos", 1.2f, 1.0f, 2.0f);
		//morphologyShader->setVec3("lightPos",glm::vec3(m_EyeTransLeft * m_HMDTrans* glm::vec4( 0, 0, 0, 1 )));

		glm::mat4 projection,view;
		if (nEye == vr::Eye_Left)
		{
			morphologyShader->setVec3("viewPos", m_EyePosLeft);
			projection = m_ProjTransLeft;
			view = m_EyeTransLeft * m_HMDTrans;
		}
		else if (nEye == vr::Eye_Right)
		{
			morphologyShader->setVec3("viewPos", m_EyePosRight); 
			projection = m_ProjTransRight;
			view = m_EyeTransRight * m_HMDTrans;
		}
		morphologyShader->setMat4("projection", projection);
		morphologyShader->setMat4("view", view);
		
		for(int i = 0;i<Agents_spheres.size();i++)// sketch neuron tree
		{	//draw sphere
			glm::mat4 model;
			Sphere* sphr = Agents_spheres[i];
			glm::vec3 sPos = Agents_spheresPos[i];
			//glm::vec3 sPos2 = glm::vec3(10,100,50);
			//qDebug("%f    %f    %f",sPos.x,sPos.y,sPos.z);
			model = glm::translate(glm::mat4(), sPos);

			model = m_globalMatrix * model;

			morphologyShader->setMat4("model", model);
			morphologyShader->setVec3("objectColor", Agents_spheresColor[i]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sphr->Render();
			morphologyShader->setVec3("objectColor", surfcolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphr->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		// draw a sphere on right controller center
		{
			glLineWidth(1);
			glm::mat4 model;
			model = glm::translate(glm::mat4(), ctrSpherePos);

			morphologyShader->setMat4("model", model);
			morphologyShader->setVec3("objectColor", ctrSphereColor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			ctrSphere->Render();
			morphologyShader->setVec3("objectColor", surfcolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			ctrSphere->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	//=================== draw markers with sphere ====================
	if(m_bShowMorphologyMarker)
	{
		morphologyShader->use();
		
		morphologyShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		morphologyShader->setVec3("lightPos", 1.2f, 1.0f, 2.0f);
		//morphologyShader->setVec3("lightPos",glm::vec3(m_EyeTransLeft * m_HMDTrans* glm::vec4( 0, 0, 0, 1 )));

		glm::mat4 projection,view;
		if (nEye == vr::Eye_Left)
		{
			morphologyShader->setVec3("viewPos", m_EyePosLeft);
			projection = m_ProjTransLeft;
			view = m_EyeTransLeft * m_HMDTrans;
		}
		else if (nEye == vr::Eye_Right)
		{
			morphologyShader->setVec3("viewPos", m_EyePosRight); 
			projection = m_ProjTransRight;
			view = m_EyeTransRight * m_HMDTrans;
		}
		morphologyShader->setMat4("projection", projection);
		morphologyShader->setMat4("view", view);
		
		for(int i = 0;i<Markers_spheres.size();i++)// sketch neuron tree
		{	//draw sphere
			if (!markerVisibility[i]) continue;
			glm::mat4 model;
			Sphere* sphr = Markers_spheres[i];
			glm::vec3 sPos = Markers_spheresPos[i];
			//glm::vec3 sPos2 = glm::vec3(10,100,50);
			//qDebug("%f    %f    %f",sPos.x,sPos.y,sPos.z);
			model = glm::translate(glm::mat4(), sPos);

			model = m_globalMatrix * model;

			morphologyShader->setMat4("model", model);
			morphologyShader->setVec3("objectColor", Markers_spheresColor[i]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sphr->Render();
			morphologyShader->setVec3("objectColor", surfcolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphr->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	//=================== draw morphology in tube mode ======================
	if (m_bShowMorphologySurface)
	{
		morphologyShader->use();
		
		morphologyShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		morphologyShader->setVec3("lightPos", 1.2f, 1.0f, 2.0f);
		//morphologyShader->setVec3("lightPos",glm::vec3(m_EyeTransLeft * m_HMDTrans* glm::vec4( 0, 0, 0, 1 )));

		glm::mat4 projection,view;
		if (nEye == vr::Eye_Left)
		{
			morphologyShader->setVec3("viewPos", m_EyePosLeft);
			projection = m_ProjTransLeft;
			view = m_EyeTransLeft * m_HMDTrans;
		}
		else if (nEye == vr::Eye_Right)
		{
			morphologyShader->setVec3("viewPos", m_EyePosRight); 
			projection = m_ProjTransRight;
			view = m_EyeTransRight * m_HMDTrans;
		}
		morphologyShader->setMat4("projection", projection);
		morphologyShader->setMat4("view", view);


		// world transformation
		const QList <NeuronSWC> & loaded_listNeuron = loadedNT_merged.listNeuron;
		const QHash <int, int> & loaded_hashNeuron = loadedNT_merged.hashNeuron;
		NeuronSWC S0,S1;

		//freeze is used for brightness suppression
		 //if (!m_bFrozen) {
		 //	m_frozen_globalMatrix = m_globalMatrix;
		 //}
			//else 
		// {
		// 	m_globalMatrix = m_frozen_globalMatrix;
		// }

		int cy_count = 0;
		for(int i = 0;i<loaded_spheres.size();i++)//loaded neuron tree
		{
			//draw sphere
			glm::mat4 model;
			Sphere* sphr = loaded_spheres[i];
			glm::vec3 sPos = loaded_spheresPos[i];
			model = glm::translate(glm::mat4(), sPos);

			model = m_globalMatrix * model;

			morphologyShader->setMat4("model", model);
			morphologyShader->setVec3("objectColor", surfcolor);//loaded_spheresColor[i]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sphr->Render();
			morphologyShader->setVec3("objectColor", loaded_spheresColor[i]);// wireframecolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphr->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//draw cylinder
			//int pn = loadedNT_merged.listNeuron[i].pn;
			S0 = loaded_listNeuron.at(i);
			if (S0.pn != -1)
			{
				int j = loaded_hashNeuron.value(S0.pn, -1);
				S1 = loaded_listNeuron.at(j);

				//cylinder between node i and (pn-1)
				glm::vec3 v1(0.0f, -1.0f, 0.0f);
				glm::vec3 v2;// = loaded_spheresPos[pn-1] - loaded_spheresPos[i];
				v2.x = S1.x-S0.x;v2.y = S1.y-S0.y;v2.z = S1.z-S0.z;
				float dist = glm::length(v2); //dprintf("dist= %f\n", dist);
				//v1 = glm::normalize(v1);
				v2 = glm::normalize(v2);
				float cos_angle = glm::dot(v1, v2);
				glm::vec3 axis = glm::cross(v1, v2);//todo-yimin: optimizable: these things can be pre-calculated

				//handle degenerated axis. todo: verify
				if (glm::length(axis) < 0.0001)//(cos_angle < -1 + 0.001f) { //if the angle is 180 degree, any axis will do.
					axis = glm::cross(glm::vec3(1.0f,0.0f,0.0f),v1);

				Cylinder* cy = loaded_cylinders[cy_count++];
				model = glm::translate(glm::mat4(),loaded_spheresPos[i]);
				model = glm::rotate(model, glm::acos(cos_angle), axis);
				model = glm::translate(model, glm::vec3(.0f, -dist/2, .0f));

				model = m_globalMatrix * model;

				morphologyShader->setMat4("model", model);
				morphologyShader->setVec3("objectColor", surfcolor);//loaded_spheresColor[i]);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				cy->Render();
				morphologyShader->setVec3("objectColor", loaded_spheresColor[i]);// wireframecolor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				cy->Render();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
			}
			else//when a node's pn = -1, do not render it's cylinder 
			{
				cy_count++;
			}
		}

		cy_count = 0;
		for(int i = 0;i<sketch_spheres.size();i++)// sketch neuron tree
		{	//draw sphere
			glm::mat4 model;
			Sphere* sphr = sketch_spheres[i];
			glm::vec3 sPos = sketch_spheresPos[i];
			model = glm::translate(glm::mat4(), sPos);

			model = m_globalMatrix * model;

			morphologyShader->setMat4("model", model);
			morphologyShader->setVec3("objectColor", surfcolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sphr->Render();
			morphologyShader->setVec3("objectColor", wireframecolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphr->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//draw cylinder
			int pn = currentNT.listNeuron[i].pn;
			if (pn != -1)
			{
				//cylinder between node i and (pn-1)
				glm::vec3 v1(0.0f, -1.0f, 0.0f);
				glm::vec3 v2 = sketch_spheresPos[pn-1] - sketch_spheresPos[i];//+glm::vec3(0.0001,0.0001,0.0001);
				float dist = glm::length(v2); //dprintf("dist= %f\n", dist);
				//v1 = glm::normalize(v1);
				v2 = glm::normalize(v2);
				float cos_angle = glm::dot(v1, v2);
				glm::vec3 axis = glm::cross(v1, v2);

				//handle degenerated axis. todo: verify
				if (glm::length(axis) < 0.0001)//(cos_angle < -1 + 0.001f) { //if the angle is 180 degree, any axis will do.
					axis = glm::cross(glm::vec3(1.0f,0.0f,0.0f),v1);

				Cylinder* cy = sketch_cylinders[cy_count++];
				model = glm::translate(glm::mat4(),sketch_spheresPos[i]);
				model = glm::rotate(model, glm::acos(cos_angle), axis);
				model = glm::translate(model, glm::vec3(.0f, -dist/2, .0f));

				model = m_globalMatrix * model;

				morphologyShader->setMat4("model", model);
				morphologyShader->setVec3("objectColor", surfcolor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				cy->Render();
				morphologyShader->setVec3("objectColor", wireframecolor);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				cy->Render();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else//when a node's pn = -1, do not render it's cylinder 
			{
				cy_count++;
			}
		}
	}
	//=================== draw morphology in line mode ======================
	if (m_bShowMorphologyLine)
	{	

		Matrix4 globalMatrix_M = Matrix4();
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				globalMatrix_M[i*4+j]=m_globalMatrix[i][j];
			}
		}
		Matrix4 model_M =  GetCurrentViewProjectionMatrix(nEye) * globalMatrix_M;//eaquls m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose * global;
		//draw loaded lines (nonEditableLoadedNTL)
		glUseProgram(m_unControllerTransformProgramID);
		glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE,model_M.get());//GetCurrentViewProjectionMatrix(nEye).get());// model = globalmatrix * model;?

		// .get() is a const float * m[16], globalmatrix must be a glm::mat4  
		glBindVertexArray(m_unMorphologyLineModeVAO);
		glLineWidth(iLineWid);
		glDrawElements(GL_LINES, m_uiMorphologyLineModeVertcount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//draw currently sketch lines(currentNT)
		glBindVertexArray(m_unSketchMorphologyLineModeVAO);
		glDrawElements(GL_LINES, m_uiSketchMorphologyLineModeVertcount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//draw all sketch lines(sketchedNTList)
		for(int i=0;i<sketchedNTList.size();i++)
		{
			glBindVertexArray(iSketchNTLMorphologyVAO.at(i));
			glDrawElements(GL_LINES, iSketchNTLMorphologyVertcount.at(i), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
	//=================== draw the controller axis lines ======================
	bool bIsInputCapturedByAnotherProcess = m_pHMD->IsInputFocusCapturedByAnotherProcess();
	if( !bIsInputCapturedByAnotherProcess&&m_modeTouchPad_R == tr_clipplane)
	{
		glUseProgram( m_unControllerTransformProgramID );
		glUniformMatrix4fv( m_nControllerMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix( nEye ).get() );
		glBindVertexArray( m_unControllerVAO );
		glLineWidth(9);
		glDrawArrays( GL_LINES, 0, m_uiControllerVertcount );
		glBindVertexArray( 0 );
		glLineWidth(iLineWid);
	}
	//=================== draw the controller shooting ray ======================
	if( !bIsInputCapturedByAnotherProcess )
	{
		glUseProgram( m_unControllerRayProgramID );
		glUniformMatrix4fv( m_nControllerRayMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix( nEye ).get() );
		glBindVertexArray( m_iControllerRayVAO );
		glEnable(GL_DEPTH_TEST);
		glLineWidth(6);
		glDrawArrays( GL_LINES, 0, m_uiControllerRayVertcount );
		glBindVertexArray( 0 );
		glLineWidth(iLineWid);
	}
	//=================== Render Model rendering ======================
	if (m_bControllerModelON) 
	{
	
	glUseProgram( m_unRenderModelProgramID );

	for( uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )
	{
		if( !m_rTrackedDeviceToRenderModel[ unTrackedDevice ] || !m_rbShowTrackedDevice[ unTrackedDevice ] )
			continue;

		const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[ unTrackedDevice ];
		
		if( !pose.bPoseIsValid )
			continue;

		if( bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) == vr::TrackedDeviceClass_Controller )
			continue;
		const Matrix4 & matDeviceToTracking = m_rmat4DevicePose[ unTrackedDevice ];
		Matrix4 matMVP = GetCurrentViewProjectionMatrix( nEye ) * matDeviceToTracking;
		glUniformMatrix4fv( m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get() );

		m_rTrackedDeviceToRenderModel[ unTrackedDevice ]->Draw();
	}

	//=================== draw controller tags ======================
	{	
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram( m_unCtrTexProgramID );
		glBindVertexArray( m_ControllerTexVAO );
		glUniformMatrix4fv( m_nCtrTexMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix( nEye ).get() );
		glBindTexture( GL_TEXTURE_2D, m_iTexture );
		glDrawArrays( GL_TRIANGLES, 0, m_uiControllerTexIndexSize );
		glBindTexture( GL_TEXTURE_2D, 0 );
		glBindVertexArray( 0 );
		glDisable(GL_BLEND);
	}

	}//if (m_bControllerModelON) 

	glUseProgram( 0 );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport( 0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight );

	glBindVertexArray( m_unCompanionWindowVAO );
	glUseProgram( m_unCompanionWindowProgramID );

	// render left eye (first half of index array )
	if (leftEyeDesc.m_nResolveTextureId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, 0 );

	}
	// render right eye (second half of index array )
	if (rightEyeDesc.m_nResolveTextureId != 0)
	{
		glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId  );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize) );
	}
	glBindVertexArray( 0 );
	glUseProgram( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip );

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}


//-----------------------------------------------------------------------------
// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform( nEye );
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, 
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.invert();
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Current View Projection Matrix with respect to nEye,
//          which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye )
{
	Matrix4 matMVP;
	if( nEye == vr::Eye_Left )
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if( nEye == vr::Eye_Right )
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
	}

	return matMVP;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::UpdateHMDMatrixPose()
{
	if ( !m_pHMD )
		return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for ( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if ( m_rTrackedDevicePose[nDevice].bPoseIsValid )
		{
			m_iValidPoseCount++;//note: count all the valid devices? but the value is never used.
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
			if (m_rDevClassChar[nDevice]==0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		//freeze is used for brightness suppression
		//if(!m_bFrozen) 
		{
			m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
			m_mat4HMDPose.invert();
			for (size_t i = 0; i < 4; i++)
			{
				for (size_t j = 0; j < 4; j++)
				{
					m_HMDTrans[i][j] = *(m_mat4HMDPose.get() + i * 4 + j);
				}
			}
			m_frozen_mat4HMDPose = m_mat4HMDPose;
			m_frozen_HMDTrans = m_HMDTrans;
		} 
		// else
		// {
		// 	m_mat4HMDPose = m_frozen_mat4HMDPose;
		// 	m_HMDTrans = m_frozen_HMDTrans;
		// }
	}
	
}

QString  CMainApplication::getHMDPOSstr()
{
	const Matrix4 & mat_HMD = m_rmat4DevicePose[0];
	glm::mat4 mat = glm::mat4();
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			mat[i][j] = *(mat_HMD.get() + i * 4 + j);
		}
	}
	mat=glm::inverse(m_globalMatrix) * mat;
	//qDebug()<<" convertedHMD SEND POS  1 "<<mat[3][0]<<" "<<mat[3][1]<<" "<<mat[3][2];
	XYZ convertPOS = ConvertLocaltoGlobalCoords(mat[3][0],mat[3][1],mat[3][2],CollaborationMaxResolution);
	mat[3][0] = convertPOS.x;
	mat[3][1] = convertPOS.y;
	mat[3][2] = convertPOS.z;
	QString positionStr;
	/*for(int i=0;i<16;i++)
	{
		positionStr+=QString("%1").arg(mat_HMD[i]);
		positionStr+=" ";
	}*/
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			positionStr+=QString("%1").arg(mat[i][j]);
			positionStr+=" ";
		}
	}
	//qDebug()<<"getHMDPOSstr"<<positionStr;
	return positionStr;
}



QString CMainApplication::NT2QString()
{
	//char messageBuff[8000]="";
	//for(int i=0;(i<sketchNT.listNeuron.size())&&(i<120);i++)
	//{
	//	char packetbuff[300];
	//	NeuronSWC S_temp;
	//	S_temp=sketchNT.listNeuron.at(i);
	//	sprintf(packetbuff,"%ld %d %5.3f %5.3f %5.3f %5.3f %ld ",S_temp.n,S_temp.type,S_temp.x,S_temp.y,S_temp.z,S_temp.r,S_temp.pn);
	//	std::strcat(messageBuff,packetbuff);
	//}
	//string str_temp=messageBuff;
	//QString str=QString::fromStdString(str_temp);
	////QString str="hello world";
	//return str;
	string messageBuff="";
	for(int i=0;(i<currentNT.listNeuron.size())&&(i<120);i++)
	{
		char packetbuff[300];
		NeuronSWC S_temp;
		S_temp=currentNT.listNeuron.at(i);
		XYZ tempconvertedxyz = ConvertLocaltoGlobalCoords(S_temp.x,S_temp.y,S_temp.z,CollaborationMaxResolution);
		sprintf(packetbuff,"%ld %d %5.3f %5.3f %5.3f %5.3f %ld ",S_temp.n,S_temp.type,tempconvertedxyz.x,tempconvertedxyz.y,tempconvertedxyz.z,S_temp.r,S_temp.pn);
		messageBuff +=packetbuff;
	}

	QString str=QString::fromStdString(messageBuff);
	return str;
}

void CMainApplication::UpdateNTList(QString &msg, int type)//may need to be changed to AddtoNTList( , )
{	
	QStringList qsl = QString(msg).trimmed().split(" ",QString::SkipEmptyParts);
	int str_size = qsl.size()-(qsl.size()%7);//to make sure that the string list size always be 7*N;
	//qDebug()<<"qsl.size()"<<qsl.size()<<"str_size"<<str_size;
	NeuronSWC S_temp;
	NeuronTree newTempNT;
	newTempNT.listNeuron.clear();
	newTempNT.hashNeuron.clear();
    qDebug()<<"type"<<type<<endl;
	//each segment has a unique ID storing as its name
	newTempNT.name  = "sketch_"+ QString("%1").arg(sketchNum++);
	for(int i=0;i<str_size;i++)
	{
		qsl[i].truncate(99);
		//qDebug()<<qsl[i];
		int iy = i%7;
		if (iy==0)
		{
			S_temp.n = qsl[i].toInt();
		}
		else if (iy==1)
		{
			S_temp.type = type;
		}
		else if (iy==2)
		{
			S_temp.x = qsl[i].toFloat();

		}
		else if (iy==3)
		{
			S_temp.y = qsl[i].toFloat();

		}
		else if (iy==4)
		{
			S_temp.z = qsl[i].toFloat();

		}
		else if (iy==5)
		{
			S_temp.r = qsl[i].toFloat();

		}
		else if (iy==6)
		{
			S_temp.pn = qsl[i].toInt();
			//converted received NT XYZ coords
			XYZ tempxyz = ConvertGlobaltoLocalCoords(S_temp.x,S_temp.y,S_temp.z);
			S_temp.x = tempxyz.x;
			S_temp.y = tempxyz.y;
			S_temp.z = tempxyz.z;
			newTempNT.listNeuron.append(S_temp);
			newTempNT.hashNeuron.insert(S_temp.n, newTempNT.listNeuron.size()-1);
		}
	}//*/
	sketchedNTList.push_back(newTempNT);
	qDebug()<<"receieved nt name is "<<newTempNT.name;
	SetupSingleMorphologyLine(sketchedNTList.size()-1,0);
}
void CMainApplication::ClearCurrentNT()
{
	if(currentNT.listNeuron.size()>0)
	{
		for (int i=0;i<sketch_spheres.size();i++) delete sketch_spheres[i];
		sketch_spheres.clear();
		for (int i=0;i<sketch_cylinders.size();i++) delete sketch_cylinders[i];
		sketch_cylinders.clear();
		sketch_spheresPos.clear();

		currentNT.listNeuron.clear();
		currentNT.hashNeuron.clear();
	}
	vertexcount=swccount=0;
}
QString CMainApplication::FindNearestSegment(glm::vec3 dPOS)
{
	QString ntnametofind="";
	//qDebug()<<sketchedNTList.size();
	if(sketchedNTList.size()<1) return ntnametofind;

	for(int i=0;i<sketchedNTList.size();i++)
	{
		NeuronTree nt=sketchedNTList.at(i);
		for(int j=0;j<nt.listNeuron.size();j++)
		{
			NeuronSWC SS0;
			SS0 = nt.listNeuron.at(j);
			float dist;
			if(isOnline == false)
			dist = glm::sqrt((dPOS.x-SS0.x)*(dPOS.x-SS0.x)+(dPOS.y-SS0.y)*(dPOS.y-SS0.y)+(dPOS.z-SS0.z)*(dPOS.z-SS0.z));
			else 
			{
				XYZ TargetresdPOS = ConvertLocaltoGlobalCoords(dPOS.x,dPOS.y,dPOS.z,collaborationTargetdelcurveRes);
				XYZ TargetresSS0POS = ConvertLocaltoGlobalCoords(SS0.x,SS0.y,SS0.z,collaborationTargetdelcurveRes);
				dist = glm::sqrt((TargetresdPOS.x-TargetresSS0POS.x)*(TargetresdPOS.x-TargetresSS0POS.x)+(TargetresdPOS.y-TargetresSS0POS.y)*(TargetresdPOS.y-TargetresSS0POS.y)+(TargetresdPOS.z-TargetresSS0POS.z)*(TargetresdPOS.z-TargetresSS0POS.z));
			}
			//cal the dist between pos & current node'position, then compare with the threshold
			if(dist < (dist_thres/m_globalScale*5))
			{
				//once dist between pos & node < threshold, return the segment/neurontree' name that current node belong to 
				ntnametofind = nt.name;
				return ntnametofind;
			}
		}
	}
	//if cannot find any matches, return ""
	return ntnametofind;
}
NeuronSWC CMainApplication::FindNearestNode(NeuronTree NT,glm::vec3 dPOS)//lq
{		
	NeuronSWC SS0;
	for(int i=0;i<NT.listNeuron.size();i++)
	{

		SS0 = NT.listNeuron.at(i);
		float dist = glm::sqrt((dPOS.x-SS0.x)*(dPOS.x-SS0.x)+(dPOS.y-SS0.y)*(dPOS.y-SS0.y)+(dPOS.z-SS0.z)*(dPOS.z-SS0.z));
		//cal the dist between pos & current node'position, then compare with the threshold
		if(dist < (dist_thres/m_globalScale*5))
		{
			//once dist between pos & node < threshold, return the the node
			return SS0;
		}
	}
}
void CMainApplication::UpdateDragNodeinNTList(int ntnum,int swcnum,float nodex,float nodey,float nodez)
{
	if((ntnum<0)||(ntnum>=sketchedNTList.size())) return;
	if((swcnum<0)||(swcnum>=sketchedNTList.at(ntnum).listNeuron.size())) return;
	NeuronSWC ss = sketchedNTList.at(ntnum).listNeuron.at(swcnum);
	ss.x = nodex;
	ss.y = nodey;
	ss.z = nodez;
	sketchedNTList[ntnum].listNeuron[swcnum]=ss;
	qDebug()<<"Successfully update node location.";
	SetupSingleMorphologyLine(ntnum,1);
}

bool CMainApplication::DeleteSegment(QString segName)
{
	//delete the segment that match with segName
	cout<<"segname is "<<endl;
	if(segName=="") return false;//segName="" will do nothing
	for(int i=0;i<sketchedNTList.size();i++)
	{
		QString NTname="";
		NTname = sketchedNTList.at(i).name;
		if(segName==NTname)
		{
			cout<<"find delete seg!@!!@"<<endl;
			//delete the segment in NTList,then return
			sketchedNTList.removeAt(i);
			SetupSingleMorphologyLine(i,2);
			return true;
		}
	}
	//if cannot find any matches,return false
	return false;
}

void CMainApplication::UndoLastSketchedNT()
{
	if(!bIsUndoEnable) return;
	if (vUndoList.size()<1) 
	{
		qDebug()<<"Reach Maximum Undo Counts! Cannot Undo anymore!";
		bIsUndoEnable = false;
		return;
	}
	NTL prevNTL = vUndoList.back();

	vRedoList.push_back(sketchedNTList);
	bIsRedoEnable = true;

	sketchedNTList.clear();
	sketchedNTList = prevNTL;

	vUndoList.pop_back();
}

void CMainApplication::RedoLastSketchedNT()
{
	if(!bIsRedoEnable) return;
	if(vRedoList.size()<1)
	{
		qDebug()<<"Reach Maximum Redo Counts! Cannot Redo anymore!";
		bIsRedoEnable = false;
		return;
	}
	NTL nextNTL = vRedoList.back();

	vUndoList.push_back(sketchedNTList);
	bIsUndoEnable = true;

	sketchedNTList.clear();
	sketchedNTList = nextNTL;

	vRedoList.pop_back();
}

void CMainApplication::ClearUndoRedoVectors()
{
	bIsUndoEnable = false;
	bIsRedoEnable = false;
	if(vUndoList.size()>0) vUndoList.clear();
	if(vRedoList.size()>0) vRedoList.clear();
}

//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel *CMainApplication::FindOrLoadRenderModel( const char *pchRenderModelName )
{
	CGLRenderModel *pRenderModel = NULL;
	for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		if( !stricmp( (*i)->GetName().c_str(), pchRenderModelName ) )
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if( !pRenderModel )
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while ( 1 )
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async( pchRenderModelName, &pModel );
			if ( error != vr::VRRenderModelError_Loading )
				break;

			ThreadSleep( 1 );
		}

		if ( error != vr::VRRenderModelError_None )
		{
			dprintf( "Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum( error ) );
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while ( 1 )
		{
			error = vr::VRRenderModels()->LoadTexture_Async( pModel->diffuseTextureId, &pTexture );
			if ( error != vr::VRRenderModelError_Loading )
				break;

			ThreadSleep( 1 );
		}

		if ( error != vr::VRRenderModelError_None )
		{
			dprintf( "Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName );
			vr::VRRenderModels()->FreeRenderModel( pModel );
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel( pchRenderModelName );
		if ( !pRenderModel->BInit( *pModel, *pTexture ) )
		{
			dprintf( "Unable to create GL model from render model %s\n", pchRenderModelName );
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back( pRenderModel );
		}
		vr::VRRenderModels()->FreeRenderModel( pModel );
		vr::VRRenderModels()->FreeTexture( pTexture );
	}
	return pRenderModel;
}


float CMainApplication::GetGlobalScale()
{
	return m_globalScale;
}

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex )
{
	if( unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount )
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String );
	CGLRenderModel *pRenderModel = FindOrLoadRenderModel( sRenderModelName.c_str() );
	if( !pRenderModel )
	{
		std::string sTrackingSystemName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String );
		dprintf( "Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str() );
	}
	else
	{
		m_rTrackedDeviceToRenderModel[ unTrackedDeviceIndex ] = pRenderModel;
		m_rbShowTrackedDevice[ unTrackedDeviceIndex ] = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModels()//question: purpose? for the 16 VR devices?
{
	memset( m_rTrackedDeviceToRenderModel, 0, sizeof( m_rTrackedDeviceToRenderModel ) );

	if( !m_pHMD )
		return;

	for( uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )//note: don't include the hmd
	{
		if( !m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		SetupRenderModelForTrackedDevice( unTrackedDevice );
	}

}






void CMainApplication::RefineSketchCurve(int direction, NeuronTree &oldNTree, NeuronTree &newNTree)
{
	unsigned char* data1d = 0;
	V3DLONG N,M,P,C;
	if(img4d)
	{
		data1d = img4d->getRawData();

		N = img4d->getXDim();
		M = img4d->getYDim();
		P = img4d->getZDim();
		C = img4d->getCDim();
	}
	vector<MyMarker*> outswc_final;
	V3DLONG siz = oldNTree.listNeuron.size(); 
	Tree tree;
	for (V3DLONG i=0;i<siz;i++)
	{
		NeuronSWC s = oldNTree.listNeuron[i];
		Point* pt = new Point;
		pt->x = s.x;
		pt->y = s.y;
		pt->z = s.z;
		pt->r = s.r;
		pt ->type = s.type;
		pt->p = NULL;
		pt->childNum = 0;
		tree.push_back(pt);
	}
	for (V3DLONG i=0;i<siz;i++)
	{
		if (oldNTree.listNeuron[i].pn<0) continue;
		V3DLONG pid = oldNTree.hashNeuron.value(oldNTree.listNeuron[i].pn);
		tree[i]->p = tree[pid];
		tree[pid]->childNum++;
	}
	//	printf("tree constructed.\n");
	vector<Segment*> seg_list;
	for (V3DLONG i=0;i<siz;i++)
	{
		if (tree[i]->childNum!=1)//tip or branch point
		{
			Segment* seg = new Segment;
			Point* cur = tree[i];
			do
			{
				seg->push_back(cur);
				cur = cur->p;
			}
			while(cur && cur->childNum==1);
			seg_list.push_back(seg);
		}
	}
	vector<MyMarker*> outswc;

	for (V3DLONG i=0;i<seg_list.size();i++)
	{
		vector<MyMarker> nearpos_vec, farpos_vec; // for near/far locs testing
		nearpos_vec.clear();
		farpos_vec.clear();
		if(seg_list[i]->size() > 2)
		{
			for (V3DLONG j=0;j<seg_list[i]->size();j++)
			{
				Point* node = seg_list[i]->at(j);
				XYZ loc0_t, loc1_t;
				switch (direction)// with a 50* 50* 50 bounding box 
				{
				case 0:  loc0_t = XYZ(node->x, node->y, (node->z-10>0)?(node->z-10):0); loc1_t = XYZ(node->x, node->y, (node->z+10<P)?(node->z+10):P-1); break;
				case 1:  loc0_t = XYZ(node->x, (node->y-10>0)?(node->y-10):0, node->z); loc1_t = XYZ(node->x, (node->y+10<M)?(node->y+10):M-1, node->z); break;
				case 2:  loc0_t = XYZ((node->x-10>0)?(node->x-10):0, node->y, node->z); loc1_t = XYZ((node->x+10<M)?(node->x+10):N-1, node->y, node->z); break;
				default:
					return;
				}


				XYZ loc0 = loc0_t;
				XYZ loc1 = loc1_t;

				nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
				farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
			}

			//fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
			fastmarching_drawing_serialbboxes(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
			qDebug()<<"done fm function start smooth";
			smooth_sketch_curve(outswc,5);
			qDebug()<<"done smooth";

			for(V3DLONG d = 0; d <outswc.size(); d++)
			{
				outswc[d]->radius = 2;
				outswc[d]->type = 11;
				outswc_final.push_back(outswc[d]);

			}
			outswc.clear();
		}
		else if(seg_list[i]->size() == 2)
		{
			Point* node1 = seg_list[i]->at(0);
			Point* node2 = seg_list[i]->at(1);

			for (V3DLONG j=0;j<3;j++)
			{
				XYZ loc0_t, loc1_t;
				if(j ==0)
				{
					loc0_t = XYZ(node1->x, node1->y,  node1->z);
					switch (direction)
					{
					case 0:  loc1_t = XYZ(node1->x, node1->y,  P-1); break;
					case 1:  loc1_t = XYZ(node1->x, M-1,  node1->z); break;
					case 2:  loc1_t = XYZ(N-1, node1->y,  node1->z); break;
					default:
						return;
					}
				}
				else if(j ==1)
				{
					loc0_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  0.5*(node1->z + node2->z));
					switch (direction)
					{
					case 0:  loc1_t = XYZ(0.5*(node1->x + node2->x),  0.5*(node1->y + node2->y),  P-1); break;
					case 1:  loc1_t = XYZ(0.5*(node1->x + node2->x), M-1,   0.5*(node1->z + node2->z)); break;
					case 2:  loc1_t = XYZ(N-1,  0.5*(node1->y + node2->y),   0.5*(node1->z + node2->z)); break;
					default:
						return;
					}
				}
				else
				{
					loc0_t = XYZ(node2->x, node2->y,  node2->z);
					switch (direction)
					{
					case 0:  loc1_t = XYZ(node2->x, node2->y,  P-1); break;
					case 1:  loc1_t = XYZ(node2->x, M-1,  node2->z); break;
					case 2:  loc1_t = XYZ(N-1, node2->y,  node2->z); break;
					default:
						return;
					}               }

				XYZ loc0 = loc0_t;
				XYZ loc1 = loc1_t;

				nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
				farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
			}

			fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
			smooth_sketch_curve(outswc,5);


			for(V3DLONG d = 0; d <outswc.size(); d++)
			{
				outswc[d]->radius = 2;
				outswc[d]->type = 11;
				outswc_final.push_back(outswc[d]);

			}
			outswc.clear();
		}
	}
	vector <XYZ> outswc_final_vec,outswc_final_vec_resampled;
	for(int i=0;i<outswc_final.size();i++)
	{
		MyMarker * mark_temp = outswc_final.at(i);
		XYZ vec_temp = XYZ(mark_temp->x,mark_temp->y,mark_temp->z);
		outswc_final_vec.push_back(vec_temp);
	}
	
	VectorResampling(outswc_final_vec, outswc_final_vec_resampled, 0.5f);
	reverse(outswc_final_vec_resampled.begin(),outswc_final_vec_resampled.end());
    VectorToNeuronTree(newNTree,outswc_final_vec_resampled,m_curMarkerColorType);
	//can change current neurontree's color by using 
	//VectorToNeuronTree(newNTree,outswc_final_vec_resampled,type);
	outswc_final_vec.clear();
	outswc_final_vec_resampled.clear();//*/


	outswc_final.clear();
	qDebug("Successfully done fastmarching.");	
}



//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose )//note: just called once.
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
	return matrixObj;
}

/***********************************
***    volume image rendering    ***
***********************************/
void CMainApplication::SetupCubeForImage4D()
{
   //qDebug("SetupCubeForImage4D() is called.");
	
	GLfloat vertices[24] = {
	0.0, 0.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 1.0,
	1.0, 0.0, 0.0,
	1.0, 0.0, 1.0,
	1.0, 1.0, 0.0,
	1.0, 1.0, 1.0
    };

    GLuint indices[36] = {
	1,5,7,
	7,3,1,
	0,2,6,
    6,4,0,
	0,1,3,
	3,2,0,
	7,5,4,
	4,6,7,
	2,3,7,
	7,6,2,
	1,0,4,
	4,5,1
    };
    GLuint gbo[2];
    /*
	图像块box是1*1*1   对每个体素都进行一次取样 只需要256次或者512 为什么循环次数需要1600
	因为stepsize 是0.001 那么走完一个块需要1000次 走最大对角线需要1414次
	ray casting 是在图像空间 1*1*1实现的
	*/
    glGenBuffers(2, gbo);
    GLuint vertexdat = gbo[0];
    GLuint veridxdat = gbo[1];
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // used in glDrawElement()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    // vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); // for vertexloc
    glEnableVertexAttribArray(1); // for vertexcol

    // the vertex location is the same as the vertex color
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    // glBindVertexArray(0);
    m_VolumeImageVAO = vao;

	// a custom clip surface that is infinitely close to the near frustum clipping surface
	float frustum_vertices[] = {
    // first triangle
     1.0f, -1.0f, -0.99f,  // bottom right
	 1.0f,  1.0f, -0.99f,  // top right
    -1.0f,  1.0f, -0.99f,  // top left 
    // second triangle
     -1.0f, -1.0f, -0.99f,  // bottom left
    1.0f, -1.0f, -0.99f,  // bottom right
    -1.0f,  1.0f, -0.99f   // top left
	}; 

	GLuint frustum_vbo;
	glGenVertexArrays(1, &m_clipPatchVAO);
    glGenBuffers(1, &frustum_vbo);

	glBindVertexArray(m_clipPatchVAO);

    glBindBuffer(GL_ARRAY_BUFFER, frustum_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frustum_vertices), frustum_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 
}

// init the 1 dimentional texture for transfer function
GLuint CMainApplication::initTFF1DTex()
{
   //qDebug("initTFF1DTex() is called.");

	// read in the user defined data of transfer function
//    ifstream inFile(filename, ifstream::in);
//        if (!inFile)
//    {
// 	cerr << "Error openning file: " << filename << endl;
// 	exit(EXIT_FAILURE);
//    }
   
//    const int MAX_CNT = 10000;
//    GLubyte *tff = (GLubyte *) calloc(MAX_CNT, sizeof(GLubyte));
//    inFile.read(reinterpret_cast<char *>(tff), MAX_CNT);
//    if (inFile.eof())
//    {
// 	size_t bytecnt = inFile.gcount();
// 	*(tff + bytecnt) = '\0';
// 	cout << "bytecnt " << bytecnt << endl;
//    }
//    else if(inFile.fail())
//    {
// 	cout << filename << "read failed " << endl;
//    }
//    else
//    {
// 	cout << filename << "is too large" << endl;
//    }    
	
   GLubyte *transferfunc = new GLubyte[256*3];


//    tk::spline spline_r,spline_g,spline_b,spline_a;
//    std::vector<double> vec_cx,vec_ry,vec_gy,vec_by,vec_ax,vec_ay;
//    vec_cx.push_back(0);vec_cx.push_back(80);vec_cx.push_back(82);vec_cx.push_back(256);
//    vec_ry.push_back(0.91);vec_ry.push_back(0.91);vec_ry.push_back(1.0);vec_ry.push_back(1.0);
//    vec_gy.push_back(0.7);vec_gy.push_back(0.7);vec_gy.push_back(1.0);vec_gy.push_back(1.0);
//    vec_by.push_back(0.61);vec_by.push_back(0.61);
   
// 	spline_r.set_points(vec_cx,vec_ry);
// 	spline_g.set_points(vec_cx,vec_gy);
// 	spline_b.set_points(vec_cx,vec_by);
	
// 	cout<<"come into spline_r"<<endl;
//    for(int i=0;i<256;++i)
//    {
// 	   transferfunc[i*3] = spline_r(i);
// 	   transferfunc[i*3+1] = spline_g(i);	
// 	   transferfunc[i*3+2] = spline_b(i);	   
//    }
// 	cout<<"come out spline_r"<<endl;
    GLuint tff1DTex;
//    glGenTextures(1, &tff1DTex);
//    glBindTexture(GL_TEXTURE_1D, tff1DTex);
//    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, transferfunc);
//    free(transferfunc);    
    return tff1DTex;
}

// init the 2D texture for render backface 'bf' stands for backface
GLuint CMainApplication::initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
    //qDebug("initFace2DTex() is called.");
	
	GLuint backFace2DTex;
    glGenTextures(1, &backFace2DTex);
    glBindTexture(GL_TEXTURE_2D, backFace2DTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bfTexWidth, bfTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    return backFace2DTex;
}

// init 3D texture to store the volume data used fo ray casting
GLuint CMainApplication::initVol3DTex()
{

    {GLuint w = img4d->getXDim(); GLuint h = img4d->getYDim(); GLuint d= img4d->getZDim();
	cout << "(w,h,d) of image =("<<w<<","<<h<<","<<d <<")"<< endl;
	
    glGenTextures(1, &g_volTexObj);
    // bind 3D texture target
    glBindTexture(GL_TEXTURE_3D, g_volTexObj);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GL_ERROR();
	cout << "img4d->getCDim()=" << img4d->getCDim() << endl;
	switch (img4d->getCDim())
	{
	case 1:
	{
		//GLubyte * testData = new GLubyte[img4d->getTotalUnitNumberPerChannel() * 3];
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, w, h, d, 0, GL_RED, GL_UNSIGNED_BYTE, (GLubyte *)img4d->getRawData());
	}
	break;
	case 3:
	{
		GLubyte *RData = img4d->getRawDataAtChannel(0);
		GLubyte *GData = img4d->getRawDataAtChannel(1);
		GLubyte *BData = img4d->getRawDataAtChannel(2);
		GLubyte *RBGData = new GLubyte[img4d->getTotalUnitNumberPerChannel() * 3];
		for (int i = 0; i < img4d->getTotalUnitNumberPerChannel() * 3;)
		{
			RBGData[i] = RData[i / 3];
			RBGData[i + 1] = GData[i / 3];
			RBGData[i + 2] = BData[i / 3];
			i += 3;
		}
		//	RGBImageTexData = RBGData;
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, w, h, d, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte *)RBGData);
	}
	break;

	default:
		break;
	}

    //glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, w, h, d, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (GLvoid*)data); 
	/*GLubyte * RData = img4d->getRawDataAtChannel(0);
	GLubyte * GData = img4d->getRawDataAtChannel(1);
	GLubyte * BData = img4d->getRawDataAtChannel(2);
	GLubyte * RBGData = new GLubyte[img4d->getTotalUnitNumberPerChannel()*3];
	cout<<"step 2"<<endl;
	for(int i = 0;i<img4d->getTotalUnitNumberPerChannel()*3;)
	{	
		RBGData[i] = RData[i/3];
		RBGData[i+1] = GData[i/3];
		RBGData[i+2] = BData[i/3];
		i+=3;
	}
	cout<<"step 3"<<endl;
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, w, h, d, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte *)RBGData);*/

	GL_ERROR();
    cout << "volume texture created" << endl;
    return g_volTexObj;
	}

}

GLuint CMainApplication::initVolOctree3DTex(int step,GLuint g_volTexObj_octree)
{
if(!replacetexture)
    {
		GLuint w = img4d->getXDim(); GLuint h = img4d->getYDim(); GLuint d= img4d->getZDim();
	cout << "(w,h,d) of image =("<<w<<","<<h<<","<<d <<")"<< endl;
		glGenTextures(1, &g_volTexObj_octree);
    	glBindTexture(GL_TEXTURE_3D, g_volTexObj_octree);
    	bindTexturePara();
    	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		GL_ERROR();
		switch(img4d->getDatatype())
		{
			case V3D_UINT8:
			{
				HelpFunc_createOctreetexture<GLubyte>(step);
			}break;
			case V3D_UINT16:
			{
				HelpFunc_createOctreetexture<GLushort>(step);
				
			}break;
			case V3D_FLOAT32:
			{

				HelpFunc_createOctreetexture<GLuint>(step);
			}break;
			default:
			break;
		}	
	}
	
	 GL_ERROR();
     cout << "volume octree texture created" << endl;
     return g_volTexObj_octree;
	

}
void CMainApplication::initFrameBufferForVolumeRendering(GLuint texObj, GLuint texWidth, GLuint texHeight)
{
	//qDebug("initFrameBufferForVolumeRendering() is called.");
	 
	// create a depth buffer for our framebuffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);

	// attach the texture and the depth buffer to the framebuffer
	glGenFramebuffers(1, &g_frameBufferBackface);
	glBindFramebuffer(GL_FRAMEBUFFER, g_frameBufferBackface);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	//check Framebuffer Status
	GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (complete != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "framebuffer is not complete" << endl;
		exit(EXIT_FAILURE);
	}

	glEnable(GL_DEPTH_TEST);
}

//setup container cube, 1D, 2D, 3D textures, and framebuffer for volume rendering
void CMainApplication::SetupVolumeRendering()
{
	//qDebug("SetupVolumeRendering() is called.");

	g_texWidth = g_winWidth = m_nRenderWidth;
    g_texHeight = g_winHeight = m_nRenderHeight;

	qDebug("g_texWidth = %d, g_texHeight = %d",g_texWidth,g_texHeight);

	SetupCubeForImage4D();

    //g_tffTexObj = initTFF1DTex();
    g_bfTexObj = initFace2DTex(g_texWidth, g_texHeight);
    g_volTexObj = initVol3DTex();
	//g_volTexObj_octree_8 = initVolOctree3DTex(8);
	//g_volTexObj_octree_16 = initVolOctree3DTex(16,g_volTexObj_octree_16);
	//g_volTexObj_octree_64 = initVolOctree3DTex(64,g_volTexObj_octree_64);	
	//g_volTexObj_octree_32 = initVolOctree3DTex(32);
    initFrameBufferForVolumeRendering(g_bfTexObj, g_texWidth, g_texHeight);
}

bool CMainApplication::CreateVolumeRenderingShaders()
{
	//qDebug("CreateVolumeRenderingShaders() is called.");
	QString qappDirPath = QCoreApplication::applicationDirPath();
	raycastingShader = new Shader(string(qappDirPath.toStdString()+"/materials/raycasting.vert").c_str(), string(qappDirPath.toStdString()+"/materials/raycasting.frag").c_str());
	backfaceShader = new Shader(string(qappDirPath.toStdString()+"/materials/backface.vert").c_str(), string(qappDirPath.toStdString()+"/materials/backface.frag").c_str());
	clipPatchShader = new Shader(string(qappDirPath.toStdString()+"/materials/clippatch.vert").c_str(), string(qappDirPath.toStdString()+"/materials/backface.frag").c_str());
	return true;
}

void CMainApplication::RenderImage4D(Shader* shader, vr::Hmd_Eye nEye, GLenum cullFace)
{
	// setup projection, view, model
	glm::mat4 projection, view, model;
	if (nEye == vr::Eye_Left)
	{
		projection = m_ProjTransLeft;
		view = m_EyeTransLeft * m_HMDTrans;
	}
	else if (nEye == vr::Eye_Right)
	{
		projection = m_ProjTransRight;
		view = m_EyeTransRight * m_HMDTrans;
	}	
	model = glm::scale(glm::mat4(),glm::vec3(img4d->getXDim(),img4d->getYDim(),img4d->getZDim()));
	model = m_globalMatrix * model;
	glm::mat4 mvp = projection * view * model;

	// render
	glEnable(GL_CULL_FACE);
    glCullFace(cullFace);
	
	if (cullFace == GL_BACK)
	{
		//for the patch, tranform the coordinates from NDC space back to world space
		clipPatchShader->use();
		clipPatchShader->setMat4("MVP",mvp);
		glBindVertexArray(m_clipPatchVAO); 
		glDrawArrays(GL_TRIANGLES, 0, 6);
		shader->use();
	}
	shader->setMat4("MVP",mvp);
	
    glBindVertexArray(m_VolumeImageVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glDisable(GL_CULL_FACE); 
}

void CMainApplication::SetUinformsForRayCasting()
{
	// setting uniforms such as
	// ScreenSize 
	// StepSize
	// TransferFunc
	// ExitPoints i.e. the backface, the backface hold the ExitPoints of ray casting
	// VolumeTex the texture that hold the volume data
	
	raycastingShader->setVec2("ScreenSize",(float)g_winWidth, (float)g_winHeight);
	raycastingShader->setFloat("StepSize",0.001f);
	//raycastingShader->setFloat("Stepsizeoctree8",0.01f);
	//raycastingShader->setFloat("Stepsizeoctree16",0.02f);
	//raycastingShader->setFloat("Stepsizeoctree32",0.04f);
	//raycastingShader->setFloat("Stepsizeoctree64",0.08f);
	raycastingShader->setVec2("ImageSettings",fContrast, fBrightness);
	//raycastingShader->setFloat("contrast ",fContrast);
	//raycastingShader->setFloat("brightness ",fBrightness);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, g_tffTexObj);
	raycastingShader->setInt("TransferFunc",0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_bfTexObj);
	raycastingShader->setInt("ExitPoints", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, g_volTexObj);
	raycastingShader->setInt("VolumeTex", 2);

	// glActiveTexture(GL_TEXTURE3);
	// glBindTexture(GL_TEXTURE_3D, g_volTexObj_octree_8);
	// raycastingShader->setInt("VolumeTexoctree8", 3);

	// glActiveTexture(GL_TEXTURE4);
	// glBindTexture(GL_TEXTURE_3D, g_volTexObj_octree_16);
	// raycastingShader->setInt("VolumeTexoctree16", 4);
	// glActiveTexture(GL_TEXTURE5);
	// glBindTexture(GL_TEXTURE_3D, g_volTexObj_octree_32);
	// raycastingShader->setInt("VolumeTexoctree32", 5);
	
	// glActiveTexture(GL_TEXTURE6);
	// glBindTexture(GL_TEXTURE_3D, g_volTexObj_octree_64);
	// raycastingShader->setInt("VolumeTexoctree64", 6);

	if(img4d->getCDim()== 1 )//single channel image
	// to do
	raycastingShader->setInt("channel", 0);

	else	
		raycastingShader->setInt("channel", m_rgbChannel);
		
	if(m_modeTouchPad_R == tr_clipplane)
	{
		raycastingShader->setVec3("clippoint", u_clippoint);
		raycastingShader->setVec3("clipnormal", u_clipnormal);
		raycastingShader->setInt("clipmode", 1);
	}
	// else if(m_modeGrip_R == m_slabplaneMode)
	// {
	// 	raycastingShader->setVec3("clippoint", u_clippoint);
	// 	raycastingShader->setVec3("clipnormal", u_clipnormal);
	// 	raycastingShader->setInt("clipmode", 2);
	// 	raycastingShader->setFloat("clipwidth",fSlabwidth);
	// }
	// else 
	// {
	// 	raycastingShader->setInt("clipmode", 0);
	// }
}



//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
CGLRenderModel::CGLRenderModel( const std::string & sRenderModelName )//todo: set vao vbo to zero
	: m_sModelName( sRenderModelName )
{
	m_glIndexBuffer = 0;
	m_glVertArray = 0;
	m_glVertBuffer = 0;
	m_glTexture = 0;
}



CGLRenderModel::~CGLRenderModel()
{
	Cleanup();
}


//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool CGLRenderModel::BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture )
{
	// create and bind a VAO to hold state for this model
	glGenVertexArrays( 1, &m_glVertArray );
	glBindVertexArray( m_glVertArray );

	// Populate a vertex buffer
	glGenBuffers( 1, &m_glVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vr::RenderModel_Vertex_t ) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW );

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vPosition ) );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vNormal ) );
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, rfTextureCoord ) );

	// Create and populate the index buffer
	glGenBuffers( 1, &m_glIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( uint16_t ) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW );

	glBindVertexArray( 0 );

	// create and populate the texture
	glGenTextures(1, &m_glTexture );
	glBindTexture( GL_TEXTURE_2D, m_glTexture );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData );

	// If this renders black ask McJohn what's wrong.
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	GLfloat fLargest;
	glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest );

	glBindTexture( GL_TEXTURE_2D, 0 );

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Cleanup()//todo: delete vao, vbo
{
	if( m_glVertBuffer )
	{
		glDeleteBuffers(1, &m_glIndexBuffer);
		glDeleteVertexArrays( 1, &m_glVertArray );
		glDeleteBuffers(1, &m_glVertBuffer);
		m_glIndexBuffer = 0;
		m_glVertArray = 0;
		m_glVertBuffer = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draws the render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Draw()
{
	glBindVertexArray( m_glVertArray );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_glTexture );

	glDrawElements( GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0 );

	glBindVertexArray( 0 );
}

void CMainApplication::SetupSingleMorphologyLine(int ntIndex, int processMode)
{
	//qDebug()<<"index = "<< ntIndex<<" and mode ="<<processMode;
	if (processMode == 0)
	// add a new VAO&VBO for new segment in sketchedNTList
	{
		iSketchNTLMorphologyVAO.push_back(0);
		iSketchNTLMorphologyVertBuffer.push_back(0);
		iSketchNTLMorphologyIndexBuffer.push_back(0);	
		iSketchNTLMorphologyVertcount.push_back(0);
		SetupMorphologyLine(sketchedNTList.at(ntIndex),iSketchNTLMorphologyVAO.at(ntIndex),iSketchNTLMorphologyVertBuffer.at(ntIndex),iSketchNTLMorphologyIndexBuffer.at(ntIndex),iSketchNTLMorphologyVertcount.at(ntIndex),2);
	}
	else if (processMode == 1)
	// changed a node's pos in dragmode ,then reset the VAO&VBO at ntIndex 
	{
		SetupMorphologyLine(sketchedNTList.at(ntIndex),iSketchNTLMorphologyVAO.at(ntIndex),iSketchNTLMorphologyVertBuffer.at(ntIndex),iSketchNTLMorphologyIndexBuffer.at(ntIndex),iSketchNTLMorphologyVertcount.at(ntIndex),2);
	}
	else if (processMode == 2)
	// deleted a segment in sketchedNTList, then delete the related VAO&VBO at ntIndex
	{
		if( iSketchNTLMorphologyVAO.at(ntIndex) != 0 )
		{
			
			glDeleteVertexArrays( 1, &iSketchNTLMorphologyVAO.at(ntIndex) );
			glDeleteBuffers(1, &iSketchNTLMorphologyVertBuffer.at(ntIndex));
			glDeleteBuffers(1, &iSketchNTLMorphologyIndexBuffer.at(ntIndex));

			iSketchNTLMorphologyVAO.erase(iSketchNTLMorphologyVAO.begin()+ntIndex);
			iSketchNTLMorphologyVertBuffer.erase(iSketchNTLMorphologyVertBuffer.begin()+ntIndex);
			iSketchNTLMorphologyIndexBuffer.erase(iSketchNTLMorphologyIndexBuffer.begin()+ntIndex);
			iSketchNTLMorphologyVertcount.erase(iSketchNTLMorphologyVertcount.begin()+ntIndex);
			qDebug()<<"Delete VAO done";
		}
	}
}

void CMainApplication::SetupAllMorphologyLine()
{
	if (iSketchNTLMorphologyVAO.size()>0)
	// >0 means needs to reset all the buffers
	{
		for(int i=0;i<iSketchNTLMorphologyVAO.size();i++)
		{
			glDeleteVertexArrays( 1, &iSketchNTLMorphologyVAO.at(i) );
			glDeleteBuffers(1, &iSketchNTLMorphologyVertBuffer.at(i));
			glDeleteBuffers(1, &iSketchNTLMorphologyIndexBuffer.at(i));
		}
		iSketchNTLMorphologyVAO.clear();
		iSketchNTLMorphologyVertBuffer.clear();
		iSketchNTLMorphologyIndexBuffer.clear();
		iSketchNTLMorphologyVertcount.clear();
	}

	// for each segment in sketchedNTL ,add its VAO&VBO
	for(int i=0;i<sketchedNTList.size();i++)
	{
		SetupSingleMorphologyLine(i,0);
	}
	qDebug()<<"Set up all NTs' VAO&VBO done";
}

//-----------------------------------------------------------------------------
// Purpose: Detect if the ray intersects the function panel
//-----------------------------------------------------------------------------
glm::vec2 CMainApplication::calculateshootingPadUV()
{

	const Matrix4 & mat_L = m_rmat4DevicePose[m_iControllerIDLeft];
	const Matrix4 & mat_R = m_rmat4DevicePose[m_iControllerIDRight];

	Vector4 point_lefttop ;
	Vector4 point_righttop;
	Vector4 point_leftbottom;
	Vector4 point_rightbottom;
	if(showshootingPad)
	{
		point_lefttop = Vector4(-0.2f,0.1f,-0.3f,1);
		point_righttop = Vector4(0.2f,0.1f,-0.3f,1);
		point_leftbottom = Vector4(-0.2f,0.02f,0.03f,1);
		point_rightbottom = Vector4(0.2f,0.02f,0.03f,1);
	}
	else if(m_secondMenu==_colorPad)
	{
		point_lefttop = Vector4(-0.1f,0.05f,-0.05f,1);
		point_righttop = Vector4(0.1f,0.05f,-0.05f,1);
		point_leftbottom = Vector4(-0.1f,0.02f,0.03f,1);
		point_rightbottom = Vector4(0.1f,0.02f,0.03f,1);
	}
	point_lefttop = mat_L * point_lefttop;
	point_righttop = mat_L * point_righttop;
	point_leftbottom = mat_L * point_leftbottom;
	point_rightbottom = mat_L * point_rightbottom;


	//update shootingray data and Pad pos
	glm::vec3 FuncPadPosV1 = glm::vec3(point_lefttop.x,point_lefttop.y,point_lefttop.z);
	glm::vec3 FuncPadPosV0 = glm::vec3(point_leftbottom.x,point_leftbottom.y,point_leftbottom.z);
	glm::vec3 FuncPadPosV2 = glm::vec3(point_rightbottom.x,point_rightbottom.y,point_rightbottom.z);
	Vector4 startpoint(0.0f,0.0f,-0.02f,1);
	Vector4 shootingDir(0.0f,0.0f,-39.0f,1);
	startpoint = mat_R * startpoint;
	shootingDir = mat_R * shootingDir;
	shootingraystartPos = glm::vec3(startpoint.x,startpoint.y,startpoint.z);
	shootingrayDir = glm::vec3(shootingDir.x,shootingDir.y,shootingDir.z);




	glm::vec3 E1 = FuncPadPosV1-FuncPadPosV0;
	glm::vec3  E2 = FuncPadPosV2-FuncPadPosV0;
	glm::vec3  P = glm::cross(shootingrayDir,E2);
	
	float det = glm::dot(E1, P);
	glm::vec3  T;
	 if( det >0 )
    {
        T = shootingraystartPos - FuncPadPosV0;
    }
    else
     {
         T = FuncPadPosV0 - shootingraystartPos;
         det = -det;
     }
	if(det < 0.0001f)
		{return glm::vec2(-1,-1);}
	float u = glm::dot(T, P);
	if(u<0.0f||u>det)
		{return glm::vec2(-1,-1);}
	glm::vec3  Q = glm::cross(T,E1);
	float v=glm::dot(shootingrayDir, Q);
	if(v<0 || u>det || v>det)
		{return glm::vec2(-1,-1);}
	//t means shooting Distance from the starting point of the ray to the point of intersection
	float t = glm::dot(E2, Q);
	float fInvDet = 1.0f/det;
	t *= fInvDet;
	u *= fInvDet;
	v *= fInvDet;

	shootingraycutPos = FuncPadPosV0+u*E1 +v* E2;
	return glm::vec2(v,1-u);
}
void CMainApplication::ColorMenuChoose(glm::vec2 UV)
{
	float panelpos_x=UV.x;
	float panelpos_y=UV.y;


	if(panelpos_y>0&&panelpos_y<0.5)
	{
		if(panelpos_x>0&&panelpos_x <= 0.2)
		{
			m_curMarkerColorType = 0;//white
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;
		}
		if(panelpos_x<=0.4&&panelpos_x >= 0.2)
		{
			m_curMarkerColorType = 1;//black
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;
		}
		if(panelpos_x<=0.6&&panelpos_x>=0.4)
		{
			m_curMarkerColorType = 2;//red
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;			
		}
		if(panelpos_x<=0.8&&panelpos_x>=0.6)
		{
			m_curMarkerColorType = 3;//blue
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;
		}
		if(panelpos_x<=1&&panelpos_x>=0.8)
		{
			m_curMarkerColorType = 4;//purple
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;
		}
	}
	else if(panelpos_y>=0.5&&panelpos_y<=1)
	{
		if(panelpos_x>0&&panelpos_x <= 0.2)
		{
			m_curMarkerColorType =5;//cyan
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;
		}
		if(panelpos_x<=0.4&&panelpos_x >= 0.2)
		{
			m_curMarkerColorType = 6;//yellow
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;
		}
		if(panelpos_x<=0.6&&panelpos_x>=0.4)
		{
			m_curMarkerColorType = 7;//green
			ctrSphereColor[0] =  neuron_type_color[m_curMarkerColorType][0] /255.0;
			ctrSphereColor[1] =  neuron_type_color[m_curMarkerColorType][1] /255.0;
			ctrSphereColor[2] =  neuron_type_color[m_curMarkerColorType][2] /255.0;
		}
	}
}
void CMainApplication::MenuFunctionChoose(glm::vec2 UV)
{
	float panelpos_x=UV.x;
	float panelpos_y=UV.y;


//choose left controllerFunction
	if(panelpos_x <= 0.436)
	{
		if((panelpos_x <= 0.26) && (panelpos_y<= 0.25)&&(panelpos_y >= 0.075)&&(panelpos_x >= 0.1))
		{
			m_modeGrip_L = _TeraShift;
		}
		 if((panelpos_x <= 0.436) && (panelpos_y<= 0.25)&&(panelpos_y >= 0.075)&&(panelpos_x >= 0.26))
		{
			m_modeGrip_L = _TeraZoom;
		}
		 if((panelpos_x <= 0.26) && (panelpos_y<= 0.44)&&(panelpos_y >= 0.25)&&(panelpos_x >= 0.1))
		{
			m_modeGrip_L = _UndoRedo;
		}
		 if((panelpos_x <= 0.436) && (panelpos_y<= 0.44)&&(panelpos_y >= 0.25)&&(panelpos_x >= 0.27))
		{
			m_modeGrip_L = _AutoRotate;

		}
		 if((panelpos_x <= 0.26) && (panelpos_y<= 0.617)&&(panelpos_y >= 0.44)&&(panelpos_x >= 0.1))
		{
			m_modeGrip_L = _Surface;
		}
		 if((panelpos_x <= 0.436) && (panelpos_y<= 0.617)&&(panelpos_y >= 0.44)&&(panelpos_x >= 0.27))
		{
			m_modeGrip_L = _ColorChange;
			m_secondMenu = _colorPad;
			showshootingPad = false;
			//hide top menu,show color menu
		}
		 if((panelpos_x <= 0.26) && (panelpos_y<= 0.8)&&(panelpos_y >= 0.617)&&(panelpos_x >= 0.1))
		{
			m_modeGrip_L = _VirtualFinger;
		}
		 if((panelpos_x <= 0.436) && (panelpos_y<= 0.8)&&(panelpos_y >= 0.617)&&(panelpos_x >= 0.27))
		{
            m_modeGrip_L = _Freeze;
		}
		 if((panelpos_x <= 0.26) && (panelpos_y<= 1)&&(panelpos_y >= 0.8)&&(panelpos_x >= 0.1))
		{
			m_modeGrip_L = _RGBImage;
			//m_modeGrip_L = _LineWidth;
		}
		 if((panelpos_x <= 0.436) && (panelpos_y<= 1)&&(panelpos_y >= 0.8)&&(panelpos_x >= 0.27))
		{
			//m_modeGrip_L = _Contrast;
			m_modeGrip_L = _StretchImage;
		}
		// else if((panelpos_x <= 0.26) && (panelpos_y<= 0.59)&&(panelpos_y >= 0.5)&&(panelpos_x >= 0.1))
		// {
		// 	m_modeGrip_L = _ResetImage;
		// }
		// else if((panelpos_x <= 0.436) && (panelpos_y<= 0.59)&&(panelpos_y >= 0.5)&&(panelpos_x >= 0.27))
		// {
			//m_modeGrip_L = _LineWidth;
		// }
		// else if((panelpos_x <= 0.26) && (panelpos_y<= 0.68)&&(panelpos_y >= 0.59)&&(panelpos_x >= 0.1))
		// {
		// 	m_modeGrip_L = _MovetoCreator;
		// }
		//
		//these function may be added in the future
	}
	//choose Right controllerFunction
	if(panelpos_x >= 0.437)
	{
		if((panelpos_x >= 0.437)&&(panelpos_x <= 0.626)&&(panelpos_y >= 0.07)&&(panelpos_y <= 0.25))
		{
			m_modeGrip_R = m_drawMode;
		}
		else if((panelpos_x >= 0.626)&&(panelpos_x <= 0.806)&&(panelpos_y >= 0.07)&&(panelpos_y <= 0.25))
		{
			m_modeGrip_R = m_deleteMode;
		}
		if((panelpos_x >= 0.437)&&(panelpos_x <= 0.626)&&(panelpos_y >= 0.25)&&(panelpos_y <= 0.44))
		{
			m_modeGrip_R = m_insertnodeMode;
			//m_modeGrip_R = m_markMode;
		}
		else if((panelpos_x >= 0.626)&&(panelpos_x <= 0.806)&&(panelpos_y >= 0.25)&&(panelpos_y <= 0.44))
		{
			m_modeGrip_R = m_markMode;
			//m_modeGrip_R = m_ConnectMode;
		}
		if((panelpos_x >= 0.437)&&(panelpos_x <= 0.626)&&(panelpos_y >= 0.44)&&(panelpos_y <= 0.617))
		{
			m_modeGrip_R = m_dragMode;
		}
		else if((panelpos_x >= 0.626)&&(panelpos_x <= 0.806)&&(panelpos_y >= 0.44)&&(panelpos_y <= 0.617))
		{
			m_modeGrip_R = m_splitMode;
		}
		if((panelpos_x >= 0.437)&&(panelpos_x <= 0.626)&&(panelpos_y >= 0.617)&&(panelpos_y <=0.8))
		{
			m_modeGrip_L = _MovetoCreator;
		}
		else if((panelpos_x >= 0.626)&&(panelpos_x <= 0.806)&&(panelpos_y >= 0.617)&&(panelpos_y <= 0.8))
		{
			m_modeGrip_L = _Contrast;
		}
		//else if((panelpos_x >= 0.437)&&(panelpos_x <= 0.626)&&(panelpos_y >= 0.617)&&(panelpos_y <= 0.8))
		//{
		//	m_modeGrip_R = m_clipplaneMode;
		//}
		//else if((panelpos_x >= 0.626)&&(panelpos_x <= 0.806)&&(panelpos_y >= 0.617)&&(panelpos_y <= 0.8))
		//{
		//	m_modeGrip_R = m_slabplaneMode;
		//}
	}
	if(panelpos_x >= 0.806)//switch touchpad right function
	{
		if((panelpos_y >= 0.09)&&(panelpos_y <= 0.305))
		{
			m_modeTouchPad_R = tr_contrast;
		}
		else if((panelpos_y >= 0.32)&&(panelpos_y <= 0.528))
		{
			m_modeTouchPad_R = tr_clipplane;
		}
	}

}
XYZ CMainApplication::ConvertLocaltoGlobalCoords(float x,float y,float z,XYZ targetRes)//localtogolbal
{
	x+= (CmainVRVolumeStartPoint.x-1);
	y+= (CmainVRVolumeStartPoint.y-1);
	z+= (CmainVRVolumeStartPoint.z-1);
	x*=(targetRes.x/CollaborationCurrentRes.x);
	y*=(targetRes.y/CollaborationCurrentRes.y);
	z*=(targetRes.z/CollaborationCurrentRes.z);
	return XYZ(x,y,z);
}
XYZ CMainApplication::ConvertGlobaltoLocalCoords(float x,float y,float z)
{
	x/=(CollaborationMaxResolution.x/CollaborationCurrentRes.x);
	y/=(CollaborationMaxResolution.y/CollaborationCurrentRes.y);
	z/=(CollaborationMaxResolution.z/CollaborationCurrentRes.z);
	x-= (CmainVRVolumeStartPoint.x-1);
	y-= (CmainVRVolumeStartPoint.y-1);
	z-= (CmainVRVolumeStartPoint.z-1);

	return XYZ(x,y,z);
}
//bool CMainApplication::FlashStuff(FlashType type,XYZ coords)
//{
//	switch (type)
//	{
//	case noflash:
//		break;
//	case line:
//		{
//			m_FlashCount++;
//			if(m_FlashCount>=300)
//				{m_FlashCount == 0;m_flashtype = noflash;}
//			qDebug()<<"get into case line";
//			delName = "";
//			NeuronTree nearestNT;
//			delName = FindNearestSegment(glm::vec3(coords.x,coords.y,coords.z));
//			if (delName == "") return false; //segment not found
//			for(int i=0;i<sketchedNTList.size();i++)//get split NT,nearest node
//			{
//				QString NTname="";
//				NTname = sketchedNTList.at(i).name;
//				if(NTname==delName)
//				{
//					nearestNT=sketchedNTList.at(i);
//					break;
//				}
//			}
//			if((m_FlashCount%100)==0 && (m_FlashCount/100)%2 == 0)
//			{
//				for(int i = 0;i<nearestNT.listNeuron.size();i++)
//				{
//					nearestNT.color.c[0] = 0;
//		nearestNT.color.c[1] = 0;
//					nearestNT.color.c[2] = 0;
//					qDebug()<<"case even";
//				}
//			}
//			if((m_FlashCount%100)==0 && (m_FlashCount/100)%2 == 1)			
//			{
//				for(int i = 0;i<nearestNT.listNeuron.size();i++)
//				{
//					qDebug()<<"case odd";
//					nearestNT.listNeuron[i].type = m_Flashoricolor;
//				}
//			}
//				
//		}
//		break;
//	default:
//		break;
//	}
//	return true;
//}
template<class T>
MinMaxOctree<T>::MinMaxOctree(int width, int height, int depth,int step) {

	this->width = width/step;
	if(!width%step) this->width+=1;
	this->height = height/step;
	if(!height%step) this->height+=1;
	this->depth = depth/step;
	if(!depth%step) this->depth+=1;
	this->step = step;
	data = (T*)malloc(this->width * this->height * this->depth*3); 

	for(int voxel = 0; voxel < this->width * this->height * this->depth*3;) {
		data[voxel] = 0;
		data[voxel+ 1] = 0;
		data[voxel+ 2] = 0;
		voxel+=3;
	}

}
template<class T>
MinMaxOctree<T>::~MinMaxOctree() {
	delete [] data;
}
template<class T>
void MinMaxOctree<T>::build(T* volumeData, int volumeWidth, int volumeHeight, int volumeDepth) {
	cout<<"octree width height depth = "<<this->width<<" "<<this->height<<" "<<this->depth<<" end"<<endl;
	int octreeIndex, volumeIndex;


	//v3d image data  stored order : width , height ,depth .see m3dimage.cpp row 273
	for(int w = 0; w < this->width; w++) {
		for(int h = 0; h < this->height; h++) {
			for(int d = 0; d < this->depth; d++) {
				//cout<<"row   "<<w<<"column   "<<h<<"depth "<<d<<endl;
				int volumeindexDep,volumeindexWid,volumeindexHeight;
				for(int vw = 0; vw < step; vw++) {
					volumeindexWid = (w * step + vw)>=volumeWidth?(volumeWidth-1):(w * step + vw);
					if(volumeindexWid==volumeWidth-1)break;
					for(int vh = 0; vh < step; vh++) {
						volumeindexHeight = (h * step + vh)>=volumeHeight?(volumeHeight-1):(h * step + vh);
						if(volumeindexHeight==volumeHeight-1)break;
						for(int vd = 0; vd < step; vd++) {
							//cout<<"volume row   "<<w+vw<<"column   "<<h+vh<<"depth "<<d+vd<<endl;
							volumeindexDep = (d * step + vd)>=volumeDepth?(volumeDepth-1):(d * step + vd);
							if(volumeindexDep==volumeDepth-1)break;
							
							volumeIndex = volumeindexDep * volumeHeight * volumeWidth + volumeindexHeight * volumeWidth + volumeindexWid;
							octreeIndex = d * height * width + h * width + w;
							// if(volumeData[volumeIndex *3] < data[octreeIndex * 4 + 0])
							// 	data[octreeIndex * 4 + 0] = volumeData[volumeIndex * 4];
							if(volumeData[volumeIndex * 3] > data[octreeIndex * 3])
								{
									data[octreeIndex * 3] = volumeData[volumeIndex * 3];
								}
								
							if(volumeData[volumeIndex * 3+1] > data[octreeIndex * 3+1])
								data[octreeIndex * 3+1] = volumeData[volumeIndex * 3+1];
							if(volumeData[volumeIndex * 3+2] > data[octreeIndex * 3+2])
								data[octreeIndex * 3+2] = volumeData[volumeIndex * 3+2];	
						}
					}
				}

			}
		}
	}
	int datasize=0;
	for(int w = 0; w <  this->width; w++) {
		for(int h = 0; h < this->height; h++) {
			for(int d = 0; d < this->depth; d++) {
				int index = d * height * width + h * width + w;

			if((int)data[index*3]>1)datasize++;
			}}}
			cout<<datasize<<endl;
	cout<<"BUILD DONE"<<endl;

}
template<class T>
void CMainApplication::HelpFunc_createOctreetexture(int step)
{
	MinMaxOctree<T>* minmaxOctree = new MinMaxOctree<T>(img4d->getXDim(),img4d->getYDim(),img4d->getZDim(),step);
	minmaxOctree->build((T*)RGBImageTexData,img4d->getXDim(),img4d->getYDim(),img4d->getZDim());
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, minmaxOctree->getWidth(), minmaxOctree->getHeight(), minmaxOctree->getDepth(), 0, GL_RGB, GL_UNSIGNED_BYTE, (T *)minmaxOctree->GetData());

}
void CMainApplication::StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	countsPerSecond = double(frequencyCount.QuadPart);


	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double CMainApplication::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart-CounterStart)/countsPerSecond;
}

double CMainApplication::GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart-frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if(tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount)/countsPerSecond;
}
void CMainApplication::bindTexturePara()
{
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
}
