//========= Copyright Valve Corporation ============//

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include "./v3dr_gl_vr.h"
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

#include <openvr.h>

#include "Matrices.h"//todo-yimin: this header is removable

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


NeuronTree loadedNT,sketchNT;
glm::vec3 loadedNTCenter;
long int vertexcount =0,swccount = 0;
#define dist_thres 0.01
#define default_radius 0.01

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
    {128, 255, 168},  //	16
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

class CGLRenderModel
{
public:
	CGLRenderModel( const std::string & sRenderModelName );
	~CGLRenderModel();

	bool BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture );
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};

static bool g_bPrintf = true;

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc = 0, char *argv[] = 0);
	virtual ~CMainApplication();

	bool BInit();
	bool BInitGL();
	bool BInitCompositor();

	void SetupRenderModels();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
	void ProcessVREvent( const vr::VREvent_t & event );
	void RenderFrame();

	void SetupMorphologyLine(int drawMode);
	void SetupMorphologyLine(NeuronTree neuron_Tree,GLuint& LineModeVAO, GLuint& LineModeVBO, GLuint& LineModeIndex,unsigned int& Vertcount,int drawMode);
	void SetupMorphologySurface(NeuronTree neurontree,vector<Sphere*>& spheres,vector<Cylinder*>& cylinders,vector<glm::vec3>& spheresPos);

	void SetupImage4D();

	void RenderControllerAxes();

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupCameras();
	void SetupCamerasForMorphology();

	void SetupBoundingBox();//2017-06-11 currently not used.
	void NormalizeNeuronTree(NeuronTree& nt);
	void RenderStereoTargets();
	void RenderCompanionWindow();
	void RenderScene( vr::Hmd_Eye nEye );

	Matrix4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye );
	Matrix4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye );
	Matrix4 GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye );
	void UpdateHMDMatrixPose();

	Matrix4 ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose );

	GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader );
	bool CreateAllShaders();

	void SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex );
	CGLRenderModel *FindOrLoadRenderModel( const char *pchRenderModelName );

private: 
	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;
	bool m_bVblank;
	bool m_bGlFinishHack;

	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;
	std::string m_strDriver;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ]; //note: contain everything: validity, matrix, ...
	Matrix4 m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ]; //note: store device transform matrices, copied from m_rTrackedDevicePose
	bool m_rbShowTrackedDevice[ vr::k_unMaxTrackedDeviceCount ];

private: // SDL bookkeeping
	SDL_Window *m_pCompanionWindow;
	uint32_t m_nCompanionWindowWidth;
	uint32_t m_nCompanionWindowHeight;

	SDL_GLContext m_pContext;

private: // OpenGL bookkeeping
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	bool m_bShowMorphologyLine;
	bool m_bShowMorphologySurface;

	///control edit mode
	int  m_modeControl;
	bool m_translationMode;
	bool m_rotateMode;
	bool m_zoomMode;
	bool m_TouchFirst;
	bool m_pickUpState;
	/////store the pos every first time touch on the touchpad
	float m_fTouchOldXL;
	float m_fTouchOldYL;

	int pick_point;

	float detX;
	float detY;
	

	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class


	
	float m_fNearClip;

	float m_fFarClip;
	// controller index , get them in HandleInput()
	int	m_iControllerIDLeft;
	int	m_iControllerIDRight;

	//unsigned int m_uiVertcount;

	//VAO/VBO for surface and line of loaded neuron
	vector<Sphere*> loaded_spheres;
	vector<Cylinder*> loaded_cylinders;
	vector<glm::vec3> loaded_spheresPos;

	GLuint m_unMorphologyLineModeVAO;
	GLuint m_glMorphologyLineModeVertBuffer;
	GLuint m_glMorphologyLineModeIndexBuffer;
	unsigned int m_uiMorphologyLineModeVertcount;

	//VAO/VBO for surface and line of loaded neuron
	vector<Sphere*> sketch_spheres;
	vector<Cylinder*> sketch_cylinders;
	vector<glm::vec3> sketch_spheresPos;

	GLuint m_unSketchMorphologyLineModeVAO;
	GLuint m_glSketchMorphologyLineModeVertBuffer;
	GLuint m_glSketchMorphologyLineModeIndexBuffer;
	unsigned int m_uiSketchMorphologyLineModeVertcount;

	//Neuron image
	GLuint m_imageVAO;
	GLuint m_imageVBO;

	GLuint m_unCompanionWindowVAO; //two 2D boxes
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;

	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;//note: axes for controller
	unsigned int m_uiControllerVertcount;

	Matrix4 m_mat4HMDPose;//note: m_rmat4DevicePose[hmd].invert()
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	//for morphology rendering
	glm::mat4 m_HMDTrans;
	glm::mat4 m_EyeTransLeft;//head to eye
	glm::mat4 m_EyeTransRight; 
	glm::vec3 m_EyePosLeft;
	glm::vec3 m_EyePosRight;
	glm::mat4 m_ProjTransLeft;
	glm::mat4 m_ProjTransRight;

	glm::mat4 m_globalMatrix;

	//BoundingBox swcBB;
	glm::mat4 m_GlobalTransformation;//move neuron to center of the world

	struct VertexDataScene//question: why define this? only used for sizeof()
	{
		Vector3 position;
		Vector2 texCoord;
	};

	struct VertexDataWindow//question: companion window just uses the projected data points from HMD?
	{
		Vector2 position;
		Vector2 texCoord;

		VertexDataWindow( const Vector2 & pos, const Vector2 tex ) :  position(pos), texCoord(tex) {	}
	};

	Shader* morphologyShader;
	GLuint m_unCompanionWindowProgramID;
	GLuint m_unControllerTransformProgramID;
	GLuint m_unRenderModelProgramID;

	GLint m_nControllerMatrixLocation;
	GLint m_nRenderModelMatrixLocation;

	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc );
	
	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	std::vector< CGLRenderModel * > m_vecRenderModels; //note: a duplicated access to below. used in shutdown destroy, check existence routines;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[ vr::k_unMaxTrackedDeviceCount ]; //note: maintain all the render models for VR devices; used in drawing
};

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
	, m_nCompanionWindowWidth( 640 )
	, m_nCompanionWindowHeight( 320 )
	, morphologyShader ( NULL )
	, m_unCompanionWindowProgramID( 0 )
	, m_unControllerTransformProgramID( 0 )
	, m_unRenderModelProgramID( 0 )
	, m_pHMD( NULL )
	, m_pRenderModels( NULL )
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
	, m_imageVAO( 0 )
	, m_nControllerMatrixLocation( -1 )
	, m_nRenderModelMatrixLocation( -1 )
	, m_iTrackedControllerCount( 0 )
	, m_iTrackedControllerCount_Last( -1 )
	, m_iValidPoseCount( 0 )
	, m_iValidPoseCount_Last( -1 )
	, m_strPoseClasses("")
	, m_bShowMorphologyLine(true)
	, m_bShowMorphologySurface(false)
	, m_modeControl(0)
	, m_translationMode (false)
	, m_rotateMode (false)
	, m_zoomMode (false)
	, m_TouchFirst (true)
	, m_fTouchOldXL( 0 )
	, m_fTouchOldYL( 0 )
	, m_pickUpState(false)
	, pick_point (-1)

{

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

	int nWindowPosX = 700;
	int nWindowPosY = 100;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );
	if( m_bDebugOpenGL )
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );






	m_pCompanionWindow = SDL_CreateWindow( "Vaa3D VR", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags );
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

	std::string strWindowTitle = "Vaa3D VR - " + m_strDriver + " " + m_strDisplay;
	SDL_SetWindowTitle( m_pCompanionWindow, strWindowTitle.c_str() );

	
	m_fNearClip = 0.1f;

 	m_fFarClip = 30.0f;

	m_globalMatrix =glm::mat4();


// 		m_MillisecondsTimer.start(1, this);
// 		m_SecondsTimer.start(1000, this);
	
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

	return true;
}


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

	
	//printf("before normalize\n");
	//for(int i = 0;i<loadedNT.listNeuron.size();i++)
	//{
	//	NeuronSWC S=loadedNT.listNeuron.at(i);
	//	printf("%f, %f, %f, %f\n", S.x,S.y,S.z,S.r);
	//}
	NormalizeNeuronTree(loadedNT);
	//printf("after normalize\n");
	//for(int i = 0;i<loadedNT.listNeuron.size();i++)
	//{
	//	NeuronSWC S=loadedNT.listNeuron.at(i);
	//	printf("%f, %f, %f, %f\n", S.x,S.y,S.z,S.r);
	//}

	SetupMorphologyLine(0);
	//SetupMorphologyLine(loadedNT,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,0);
	SetupMorphologySurface(loadedNT,loaded_spheres,loaded_cylinders,loaded_spheresPos);


	SetupCameras();
	SetupCamerasForMorphology();
	SetupBoundingBox();
	SetupStereoRenderTargets();

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



		for (int i=0;i<loaded_spheres.size();i++) delete loaded_spheres[i];
		for (int i=0;i<loaded_cylinders.size();i++) delete loaded_cylinders[i];

		for (int i=0;i<sketch_spheres.size();i++) delete sketch_spheres[i];
		for (int i=0;i<sketch_cylinders.size();i++) delete sketch_cylinders[i];

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

		if( m_imageVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_imageVAO );
			glDeleteBuffers(1, &m_imageVBO);
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


	/*while ( SDL_PollEvent( &sdlEvent ) != 0 )
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
			if(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger))
			{
				//this part is for building a neuron tree to further save as SWC file
				if (vertexcount%10 ==0)//use vertexcount to control point counts in a single line
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

					swccount++;//for every 10 frames we store a point as a neuronswc point,control with swccount
					NeuronSWC SL0;
					SL0.x = m_v4DevicePose.x ;
					SL0.y = m_v4DevicePose.y ;
					SL0.z = m_v4DevicePose.z ;
					SL0.r = default_radius; //set default radius 0.01
					SL0.type = 274;//set default type 274
					SL0.n = swccount;
					if(swccount==1)
					{
						SL0.pn = -1;//for the first point , it's parent must be -1
						//qDebug("Successfully run here.SL0.pn=%d\n",SL0.pn);	
					}
					else if(vertexcount == 0)
					{
						SL0.pn = -1;//for the first point of each lines, it's parent must be -1	
					}
					else
					{
						SL0.pn = sketchNT.listNeuron.at(SL0.n-1-1).n;//for the others, their parent should be the last one
					}
					sketchNT.listNeuron.append(SL0);
					sketchNT.hashNeuron.insert(SL0.n, sketchNT.listNeuron.size()-1);//store NeuronSWC SL0 into sketchNT
				}	
				vertexcount++;
			}
		}
	}//*/


	// Process SteamVR LEFT controller state
	//inlcuding translate,rotate and zoom with touchpad; pull a specific point to another POS with trigger
	{
		vr::VRControllerState_t state;

			if( m_pHMD->GetControllerState( m_iControllerIDLeft, &state, sizeof(state) ) )
			{
				//whenever touchpad is unpressed, set bool flag  m_TouchFirst = true;
				if(!(state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)))
				{
					m_TouchFirst=true;
				}//
				//whenever touchpad is pressed, get detX&detY,return to one function according to the mode
				if((state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad))&&
					!(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)))
				{
					float m_fTouchPosYL;
					float m_fTouchPosXL;
					if(m_TouchFirst==true)
					{//for every first touch,store the axis(x&y) on touchpad as old(means the initial ) POS 
						m_TouchFirst=false;
						m_fTouchOldYL = state.rAxis[0].y;
						m_fTouchOldXL = state.rAxis[0].x;
						//qDebug("1m_TouchFirst= %d,m_fTouchOldXL= %f,m_fTouchOldYL= %f.\n",m_TouchFirst,m_fTouchOldXL,m_fTouchOldYL);
					}
					m_fTouchPosYL = state.rAxis[0].y;
					m_fTouchPosXL = state.rAxis[0].x;
					//qDebug("2m_TouchFirst= %d,m_fTouchPosXR= %f,m_fTouchPosYR= %f.\n",m_TouchFirst,m_fTouchPosXR,m_fTouchPosYR);
					detX = m_fTouchPosXL - m_fTouchOldXL;
					detY = m_fTouchPosYL - m_fTouchOldYL;
					if((detX<0.3f)&&(detX>-0.3f))detX=0;
					if((detY<0.3f)&&(detY>-0.3f))detY=0;
					/*if(detY>1.7||detY<-1.7)
					{
						bRet = true;
						return bRet;
					}//*/
					if(m_translationMode==true)//into translate mode
					{
						//qDebug("TRANSLATION!detX= %f,detY= %f.\n",detX,detY);
						m_globalMatrix = glm::translate(m_globalMatrix,glm::vec3(detX/300,0,detY/300) ); 
						//translate_func(detX,detY);

					}
					else if(m_rotateMode==true)//into ratate mode
					{

						//qDebug("ROTATION!detX= %f,detY= %f.\n",detX,detY);
						//glm::vec3 globalRotation = 
						m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
						m_globalMatrix = glm::rotate(m_globalMatrix,detX/1000,glm::vec3(1,0,0));
						m_globalMatrix = glm::rotate(m_globalMatrix,detY/1000,glm::vec3(0,1,0));
						m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
						//rotate_func(detX,detY);
					}
					else if(m_zoomMode==true)//into zoom mode
					{
						m_globalMatrix = glm::translate(m_globalMatrix,-loadedNTCenter);
						m_globalMatrix = glm::scale(m_globalMatrix,glm::vec3(1+detY/1000,1+detY/1000,1+detY/1000));
						m_globalMatrix = glm::translate(m_globalMatrix,loadedNTCenter);
						//qDebug("ZOOM!detX= %f,detY= %f.\n",detX,detY);
						//zoom_func(detX,detY);
					}
				}

				//pick up the nearest node and pull it to new locations
				//note: this part of code only serves as a demonstration, and does not handle complicated cases well.
				//also, can only pull drawn neurons, not loaded ones.
				if(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger))
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
					//qDebug("ctrlLeftPos = %.2f,%.2f,%.2f\n",ctrlLeftPos.x,ctrlLeftPos.y,ctrlLeftPos.z);
					if(m_pickUpState == false)
					{
						float dist = 0;
						float minvalue = 10.f;
						for(int i = 0; i<sketchNT.listNeuron.size();i++)
						{
							NeuronSWC SS0;
							SS0 = sketchNT.listNeuron.at(i);
							dist = glm::sqrt((ctrlLeftPos.x-SS0.x)*(ctrlLeftPos.x-SS0.x)+(ctrlLeftPos.y-SS0.y)*(ctrlLeftPos.y-SS0.y)+(ctrlLeftPos.z-SS0.z)*(ctrlLeftPos.z-SS0.z));
							//qDebug("SS0 = %.2f,%.2f,%.2f\n",SS0.x,SS0.y,SS0.z);
							if(dist > dist_thres)
								continue;
							minvalue = glm::min(minvalue,dist);
							if(minvalue==dist)
								pick_point = i;

						}
						if(pick_point!=-1){
							m_pickUpState = true;
							qDebug("pick up %d point.",pick_point);}
					}
					else if(pick_point!=-1)
					{
						m_pickUpState = true;
						NeuronSWC SS1;
						SS1 = sketchNT.listNeuron.at(pick_point);
						SS1.x = ctrlLeftPos.x;
						SS1.y = ctrlLeftPos.y;
						SS1.z = ctrlLeftPos.z;
						sketchNT.listNeuron[pick_point] = SS1;
						sketch_spheresPos[pick_point] = glm::vec3(SS1.x,SS1.y,SS1.z);
						if(SS1.pn!=-1)
						{
							NeuronSWC SS2 = sketchNT.listNeuron.at(SS1.pn-1);//SS2 is the parent of SS1
							//change cylinder state
							float dist = glm::sqrt((SS2.x-SS1.x)*(SS2.x-SS1.x)+(SS2.y-SS1.y)*(SS2.y-SS1.y)+(SS2.z-SS1.z)*(SS2.z-SS1.z));
							delete sketch_cylinders[pick_point];
							sketch_cylinders[pick_point]= new Cylinder(SS1.r,SS2.r,dist);
						}
						if(SS1.n!=sketchNT.listNeuron.size())
						{
							NeuronSWC SS0 = sketchNT.listNeuron.at(SS1.n);//SS0 is the child of SS1
							float dist = glm::sqrt((SS0.x-SS1.x)*(SS0.x-SS1.x)+(SS0.y-SS1.y)*(SS0.y-SS1.y)+(SS0.z-SS1.z)*(SS0.z-SS1.z));
							delete sketch_cylinders[pick_point+1];
							sketch_cylinders[pick_point+1]= new Cylinder(SS0.r,SS1.r,dist);
						}
					}
				}


				if(!(state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)))
				{
					m_pickUpState = false;
					pick_point = -1;
				}//whenever the touchpad is unpressed, reset m_pickUpState and pick_point
			}
	}
	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();
	SDL_ShowCursor( SDL_DISABLE );
	sketchNT.listNeuron.clear();
	sketchNT.hashNeuron.clear();
	while ( !bQuit )
	{
		bQuit = HandleInput();

		RenderFrame();
	}

	SDL_StopTextInput();
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
	{		//use grip button to change the display mode linemode or surface mode
			m_bShowMorphologyLine = !m_bShowMorphologyLine;
			m_bShowMorphologySurface = !m_bShowMorphologySurface;		

	}
	if((event.trackedDeviceIndex==m_iControllerIDLeft)&&(event.eventType==vr::VREvent_ButtonPress)&&(event.data.controller.button==vr::k_EButton_SteamVR_Touchpad))
	{		//use menu button to change the processing mode for touchpad, nothing or translate or ratate or zoom mode
			m_modeControl++;
			m_modeControl%=4;
				switch(m_modeControl)
				{
				case 0:
					m_translationMode=m_rotateMode=m_zoomMode = false;
					break;
				case 1:
					m_translationMode = true;
					m_rotateMode=m_zoomMode = false;
					break;
				case 2:
					m_rotateMode = true;
					m_translationMode=m_zoomMode = false;
					break;
				case 3:
					m_zoomMode = true;
					m_translationMode=m_rotateMode = false;
					break;
				default:
					break;
				}	
			qDebug("m_modeControl=%d,m_translationMode=%d,m_rotateMode=%d,m_zoomMode=%d\n",m_modeControl,m_translationMode,m_rotateMode,m_zoomMode);
	}





	//////////////////////////////////////////RIGHT
	if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_SteamVR_Trigger)&&(event.eventType==vr::VREvent_ButtonUnpress))
	{	//every time the trigger(right) is unpressd ,set the vertexcount to zero preparing for the next line
		vertexcount=0;
		//qDebug("Successfully run here.vertexcount=%d\n",vertexcount);
	}
	if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_Grip)&&(event.eventType==vr::VREvent_ButtonUnpress))
	{	//use grip button(right) to clear all the lines been drawn on the HMD
		if(sketchNT.listNeuron.size()>0)
		{
			for (int i=0;i<sketch_spheres.size();i++) delete sketch_spheres[i];
			sketch_spheres.clear();
			for (int i=0;i<sketch_cylinders.size();i++) delete sketch_cylinders[i];
			sketch_cylinders.clear();
			sketch_spheresPos.clear();

			sketchNT.listNeuron.clear();
			sketchNT.hashNeuron.clear();
			vertexcount=swccount=0;
			//qDebug("Successfully run here.vertexcount=%d\n",vertexcount);
		}
	}
	if((event.trackedDeviceIndex==m_iControllerIDRight)&&(event.data.controller.button==vr::k_EButton_ApplicationMenu)&&(event.eventType==vr::VREvent_ButtonPress))
	{
		if(sketchNT.listNeuron.size()<1)
			return;
		QString filename = "swctofile.swc";
		writeSWC_file(filename, sketchNT);
		qDebug("Successfully writeSWC_file");
	}

	//////////////////
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderFrame()
{
	// for now as fast as possible
	if ( m_pHMD )
	{
		RenderControllerAxes();
		SetupMorphologyLine(1);
		SetupMorphologySurface(sketchNT,sketch_spheres,sketch_cylinders,sketch_spheresPos);
		//SetupMorphologyLine(sketchNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,1);

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
	morphologyShader = new Shader("basic.vert", "basic.frag");

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

	return m_unControllerTransformProgramID != 0
		&& m_unRenderModelProgramID != 0
		&& m_unCompanionWindowProgramID != 0;
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
//		SetupMorphologyLine(loadedNT,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,drawMode);
//	}
//	else{
//		SetupMorphologyLine(sketchNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,drawMode);
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
		SetupMorphologyLine(loadedNT,m_unMorphologyLineModeVAO,m_glMorphologyLineModeVertBuffer,m_glMorphologyLineModeIndexBuffer,m_uiMorphologyLineModeVertcount,drawMode);
	}
	else//1 means drawmode = 1, which means this function is called in mainloop
	{
		SetupMorphologyLine(sketchNT,m_unSketchMorphologyLineModeVAO,m_glSketchMorphologyLineModeVertBuffer,m_glSketchMorphologyLineModeIndexBuffer,m_uiSketchMorphologyLineModeVertcount,drawMode);
	}

}//*/


//-----------------------------------------------------------------------------
void CMainApplication::SetupMorphologyLine(NeuronTree neuron_Tree,
											GLuint& LineModeVAO, 
											GLuint& LineModeVBO, 
											GLuint& LineModeIndex,
											unsigned int& Vertcount,
											int drawMode)
{
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

	qDebug("rgba color---- %d,%d,%d,%d\n",rgba.c[0],rgba.c[1],rgba.c[2],rgba.a);
	if (editable) qDebug("editable\n"); else qDebug("not editable\n");
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
			int type = S0.type;
			if (editable)
			{
				int ncolorused = neuron_type_color_num;
				if (neuron_type_color_num>19)
					ncolorused = 19;
				type = S0.seg_id %(ncolorused -5)+5; //segment color using hanchuan's neuron_type_color
			}
			if (type >= 300 && type <= 555 )  // heat colormap index starts from 300 , for sequencial feature scalar visaulziation
			{
				vcolor_load[0] =  neuron_type_color_heat[ type - 300][0];
				vcolor_load[1] =  neuron_type_color_heat[ type - 300][1];
				vcolor_load[2] =  neuron_type_color_heat[ type - 300][2];
				qDebug("set in 1\n");
			}
			else
			{
				vcolor_load[0] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][0];
				vcolor_load[1] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][1];
				vcolor_load[2] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][2];
				qDebug("set in 2\n");
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

		glm::vec3 vcolor_draw(1,0,0);//red for drawing neuron tree

		//vertices[2*(S1.n-1)+1] = (drawMode==0) ? vcolor_load : vcolor_draw;
		vertices.push_back((drawMode==0) ? vcolor_load : vcolor_draw);
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


void CMainApplication::SetupImage4D()
{
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
// draw the six faces of the boundbox by drawwing triangles
// draw it contra-clockwise
// front: 1 5 7 3
// back: 0 2 6 4
// left£º0 1 3 2
// right:7 5 4 6    
// up: 2 3 7 6
// down: 1 0 4 5
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
    m_imageVAO = vao;
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

		for ( int i = 0; i < 3; ++i )//note: prepare data for axes and ray
		{
			Vector3 color( 0, 0, 0 );
			Vector4 point( 0, 0, 0, 1 );
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
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

		Vector4 start = mat * Vector4( 0, 0, -0.02f, 1 );
		Vector4 end = mat * Vector4( 0, 0, -39.f, 1 );
		Vector3 color( .92f, .92f, .71f );

		vertdataarray.push_back( start.x );vertdataarray.push_back( start.y );vertdataarray.push_back( start.z );//note: ray?
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );

		vertdataarray.push_back( end.x );vertdataarray.push_back( end.y );vertdataarray.push_back( end.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );
		m_uiControllerVertcount += 2;
	}

	// Setup the VAO the first time through.
	if ( m_unControllerVAO == 0 )
	{
		glGenVertexArrays( 1, &m_unControllerVAO );
		glBindVertexArray( m_unControllerVAO );

		glGenBuffers( 1, &m_glControllerVertBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

		GLuint stride = 2 * 3 * sizeof( float );
		uintptr_t offset = 0;

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof( Vector3 );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray( 0 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

	// set vertex data if we have some
	if( vertdataarray.size() > 0 )
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW );
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

void CMainApplication::SetupBoundingBox()
{
	BoundingBox swcBB = NULL_BoundingBox;
	for(int i = 0;i<loadedNT.listNeuron.size();i++)
	{
		NeuronSWC S=loadedNT.listNeuron.at(i);
		float d = S.r * 2;
		swcBB.expand(BoundingBox(XYZ(S) - d, XYZ(S) + d));
	}

	float DX = swcBB.Dx();
	float DY = swcBB.Dy();
	float DZ = swcBB.Dz();
	float maxD = swcBB.Dmax();

	float s[3];
	s[0] = 1 / maxD * 2;
	s[1] = 1 / maxD * 2;
	s[2] = 1 / maxD * 2;
	float t[3]; //swc center
	t[0] = -(swcBB.x0 + DX / 2);
	t[1] = -(swcBB.y0 + DY / 2);
	t[2] = -(swcBB.z0 + DZ / 2);

	m_GlobalTransformation = glm::translate(glm::mat4(), glm::vec3(t[0],t[1],t[2]));
	m_GlobalTransformation = glm::scale(m_GlobalTransformation, glm::vec3(s[0], s[1], s[2]));
	m_GlobalTransformation = glm::translate(m_GlobalTransformation, glm::vec3(-t[0]-0.6, -t[1]-1.0, -t[2]-0.4));
}

void CMainApplication::NormalizeNeuronTree(NeuronTree& nt)
{
	float r_max = -1;
	BoundingBox swcBB = NULL_BoundingBox;
	for(int i = 0;i<loadedNT.listNeuron.size();i++)
	{
		NeuronSWC S=loadedNT.listNeuron.at(i);
		float d = S.r * 2;
		swcBB.expand(BoundingBox(XYZ(S) - d, XYZ(S) + d));
		if (S.r > r_max)  r_max = S.r;
	}

	float DX = swcBB.Dx();
	float DY = swcBB.Dy();
	float DZ = swcBB.Dz();
	float maxD = swcBB.Dmax();

	//printf("boundingbox: (%f,%f,%f),(%f,%f,%f). size: (%f,%f,%f)\n", swcBB.x0,swcBB.y0,swcBB.z0,swcBB.x1,swcBB.y1,swcBB.z1,DX,DY,DZ );

	float scale = 1 / maxD * 2;
	float r_scale = 1 / r_max * 0.2;
	float trans_x = 0.6 - (swcBB.x0 + swcBB.x1)/2;
	float trans_y = 1.0 - (swcBB.y0 + swcBB.y1)/2;
	float trans_z = 0.4 - (swcBB.z0 + swcBB.z1)/2;

	//printf("transform: scale = %f, translate = (%f,%f,%f)\n", scale,trans_x,trans_y,trans_z );

	for(int i = 0;i<loadedNT.listNeuron.size();i++)
	{
		loadedNT.listNeuron[i].x = loadedNT.listNeuron[i].x * scale + trans_x * scale; 
		loadedNT.listNeuron[i].y = loadedNT.listNeuron[i].y * scale + trans_y * scale + 0.8;
		loadedNT.listNeuron[i].z = loadedNT.listNeuron[i].z * scale + trans_z * scale;
		loadedNT.listNeuron[i].r *= scale;
	}

	//calculate center after normalization
	swcBB = NULL_BoundingBox;
	for(int i = 0;i<loadedNT.listNeuron.size();i++)
	{
		NeuronSWC S=loadedNT.listNeuron.at(i);
		float d = S.r * 2;
		swcBB.expand(BoundingBox(XYZ(S) - d, XYZ(S) + d));
		if (S.r > r_max)  r_max = S.r;
	}
	loadedNTCenter.x = (swcBB.x0 + swcBB.x1)/2;
	loadedNTCenter.y = (swcBB.y0 + swcBB.y1)/2;
	loadedNTCenter.z = (swcBB.z0 + swcBB.z1)/2;
	qDebug("center.x = %f,center.y = %f,center.z = %f\n",loadedNTCenter.x,loadedNTCenter.y,loadedNTCenter.z);
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
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_MULTISAMPLE );
	// Left Eye
	glBindFramebuffer( GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Left );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	glDisable( GL_MULTISAMPLE );
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
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

	if (m_bShowMorphologySurface)
	{
		morphologyShader->use();
		
		morphologyShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		morphologyShader->setVec3("lightPos", 1.2f, 1.0f, 2.0f);
		glm::mat4 projection,view;
		if (nEye == vr::Eye_Left)
		{
			morphologyShader->setVec3("viewPos", m_EyePosLeft);
			projection = m_ProjTransLeft;
			view = m_EyeTransLeft * m_HMDTrans;// * m_GlobalTransformation;
		}
		else if (nEye == vr::Eye_Right)
		{
			morphologyShader->setVec3("viewPos", m_EyePosRight); 
			projection = m_ProjTransRight;
			view = m_EyeTransRight * m_HMDTrans;// * m_GlobalTransformation;
		}
		morphologyShader->setMat4("projection", projection);
		morphologyShader->setMat4("view", view);


		// world transformation
		const QList <NeuronSWC> & loaded_listNeuron = loadedNT.listNeuron;
		const QHash <int, int> & loaded_hashNeuron = loadedNT.hashNeuron;
		NeuronSWC S0,S1;

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
			morphologyShader->setVec3("objectColor", surfcolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sphr->Render();
			morphologyShader->setVec3("objectColor", wireframecolor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			sphr->Render();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//draw cylinder
			//int pn = loadedNT.listNeuron[i].pn;
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
			int pn = sketchNT.listNeuron[i].pn;
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
		//draw loaded lines
		glUseProgram(m_unControllerTransformProgramID);
		glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE,model_M.get());//GetCurrentViewProjectionMatrix(nEye).get());// model = globalmatrix * model;?
		// .get() is a const float * m[16], globalmatrix must be a glm::mat4  
		glBindVertexArray(m_unMorphologyLineModeVAO);
		glDrawElements(GL_LINES, m_uiMorphologyLineModeVertcount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//draw sketch lines
		glUseProgram(m_unControllerTransformProgramID);
		glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, model_M.get());//GetCurrentViewProjectionMatrix(nEye).get());// model = globalmatrix * model;?
		glBindVertexArray(m_unSketchMorphologyLineModeVAO);
		glDrawElements(GL_LINES, m_uiSketchMorphologyLineModeVertcount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	bool bIsInputCapturedByAnotherProcess = m_pHMD->IsInputFocusCapturedByAnotherProcess();

	if( !bIsInputCapturedByAnotherProcess )
	{
		// draw the controller axis lines
		glUseProgram( m_unControllerTransformProgramID );
		glUniformMatrix4fv( m_nControllerMatrixLocation, 1, GL_FALSE, GetCurrentViewProjectionMatrix( nEye ).get() );
		glBindVertexArray( m_unControllerVAO );
		glDrawArrays( GL_LINES, 0, m_uiControllerVertcount );
		glBindVertexArray( 0 );
	}

	// ----- Render Model rendering -----
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
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, 0 );

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize) );

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
	Matrix4 global(
		m_GlobalTransformation[0][0], m_GlobalTransformation[0][1], m_GlobalTransformation[0][2], m_GlobalTransformation[0][3],
		m_GlobalTransformation[1][0], m_GlobalTransformation[1][1], m_GlobalTransformation[1][2], m_GlobalTransformation[1][3],
		m_GlobalTransformation[2][0], m_GlobalTransformation[2][1], m_GlobalTransformation[2][2], m_GlobalTransformation[2][3],
		m_GlobalTransformation[3][0], m_GlobalTransformation[3][1], m_GlobalTransformation[3][2], m_GlobalTransformation[3][3]
	
	);
	
	Matrix4 matMVP;
	if( nEye == vr::Eye_Left )
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;// * global;
	}
	else if( nEye == vr::Eye_Right )
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;// * global;
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
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		m_mat4HMDPose.invert();
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				m_HMDTrans[i][j] = *(m_mat4HMDPose.get() + i * 4 + j);
			}
		}
	}
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


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
CGLRenderModel::CGLRenderModel( const std::string & sRenderModelName )//todo£º set vao vbo to zero
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


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool doimageVRViewer(NeuronTree nt)
{
	CMainApplication *pMainApplication = new CMainApplication( 0, 0 );

	loadedNT.listNeuron.clear();
	loadedNT.hashNeuron.clear();
	loadedNT.listNeuron = nt.listNeuron;
	loadedNT.hashNeuron = nt.hashNeuron;

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	pMainApplication->RunMainLoop();

	pMainApplication->Shutdown();

	return 0;
}

//bool doimageVRViewer()
//{
//	//NeuronTree nt;
//	//nt.listNeuron.clear();
//	//nt.hashNeuron.clear();
//	//return doimageVRViewer(nt);
//	QWidget* qtw = new QWidget();
//	return true;
//}
