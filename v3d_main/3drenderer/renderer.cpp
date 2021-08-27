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

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets, Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model, Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*
 *  renderer.cpp
 *
 *
 *  Created by Ruan Zongcai on 8/6/08.
 *  Copyright 2008-2009 Hanchuan Peng
 *  Last change: 2010-Dec-09. by Hanchuan Peng. remove a few functions and convert them as template functions and move to the header file
 */

#include "GLee2glew.h" ////2020-2-10

#include "renderer.h"
#include "v3dr_glwidget.h" //for makeCurrent, drawText
#include <sstream>
#include <string>
#include <cmath>
#include <QPainter>
#include <GL/gl.h>
#include <GL/glew.h>
Renderer::SelectMode Renderer::defaultSelectMode = Renderer::smObject;

using namespace std;

Renderer::Renderer(void* widget)
    : widget(widget)
{
    qDebug(" Renderer::Renderer >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    init_members();
}

Renderer::~Renderer()
{
    qDebug(" Renderer::~Renderer <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    cleanObj();
    cleanData();
    //qDebug(" ------------------------------------------------------ Renderer shutdown");
}

void Renderer::makeCurrent()
{
    if (! widget)  return;

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    w->makeCurrent();
}

void Renderer::drawString(float x, float y, float z, const char* text, int shadow, int fontsize)
{
    if (! widget)  return;

    if (shadow)
    {
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        glPushAttrib(GL_CURRENT_BIT);
            glColor3ub(50,50,50);
            //glColor3ub(200,200,200);

            // CMB MSVC debugger with Qt 4.7 triggers assert if font weight > 99
            // QFont f;  f.setPointSize(f.pointSize()+1); f.setWeight(f.weight()+200);
            QFont f;  f.setPointSize(f.pointSize()+1); f.setWeight(QFont::Thin);

           //((QOpenGLWidget_proxy*)widget)->renderText(x,y,z, QString(text), f); qt6


        glPopAttrib();
        glDepthFunc(GL_LEQUAL);
    }

    QFont f1;  f1.setPointSize((fontsize>0)?fontsize:30); //f1.setWeight(99);
//    if (fontsize>0) // qt6 still unsolve
//        ((V3dR_GLWidget *)widget)->renderText(x,y,z, QString(text));

    if (shadow)
    {
        glPopAttrib();
    }
}

bool Renderer::beStill()
{
    if (! widget)  return false;
    return ((V3dR_GLWidget*)widget)->getStill();
}

const char* Renderer::try_vol_state()
{
    static char sbuf[200];
    sprintf(sbuf, "(compress 3d npt stream shader = %d %d %d %d %d)", tryTexCompress,tryTex3D,tryTexNPT,tryTexStream,tryVolShader);
    return sbuf;
}

////////////////////////////////////////////////////////////////////////////////

void Renderer::setupView(int width, int height)
{
    //qDebug(" Renderer::setupView");
    makeCurrent(); // Qt seems not makeCurrent in resizeGL, 081029 by RZC

//	if (screenW != width || screenH != height)
//	{
//		if (depth_buffer)	delete[] depth_buffer;	depth_buffer = 0;
//		depth_buffer = new GLfloat[width*height];
//	}

    screenW = width;
    screenH = height;
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    setProjection();

    glMatrixMode(GL_MODELVIEW);
    // here please make object space fit in [-1, +1]^3
}

void Renderer::setProjection() // clipped space and projection matrix
{
    double aspect = double(screenW)/MAX(screenH,1); // width/height = aspect ratio

    if (bOrthoView)
    {
        double halfw = 1.3*aspect *zoomRatio;
        double halfh = 1.3        *zoomRatio;
        glOrtho(-halfw, halfw, -halfh, halfh, viewNear, viewFar);
    }
    else
    {
        gluPerspective(viewAngle*zoomRatio, aspect, viewNear, viewFar);
    }

    glTranslated(0, 0, -viewDistance);
}

#define PICK_BUFFER_SIZE 10240  //// CAUTION: pick buffer overflow may cause crash
#define PICK_TOLERANCE 5

int Renderer::selectObj(int x, int y, bool b_menu, char* pTip)
{
    makeCurrent(); //090715 make sure in correct OpenGL context

    qDebug(" Renderer::selectObj in renderer.cpp by jazz brain");
    if (b_selecting)  return 0;  // prevent re-enter

    if (selectMode >smObject && pTip) return 0; //prevent tool-tip when definition


    ////////////////////////////////////////////
    b_selecting = true;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLuint zDepth;
    glReadPixels(x,viewport[3]-y,1,1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &zDepth);

    GLuint *selectBuf = new GLuint[PICK_BUFFER_SIZE];
    glSelectBuffer(PICK_BUFFER_SIZE, selectBuf);
    glRenderMode(GL_SELECT);

    glInitNames();
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPickMatrix(x, viewport[3]-y,	PICK_TOLERANCE,PICK_TOLERANCE, viewport);
        setProjection();
        glMatrixMode(GL_MODELVIEW);

        paint(); //##########

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glFlush();
    }

    b_selecting = false;
    int hits = glRenderMode(GL_RENDER);
    if (hits==0)
    {
        delete[] selectBuf;

        if (b_menu && widget) // background menu
        {
            ((V3dR_GLWidget*)widget)->setBackgroundColor();
        }

        return 0;
    }


    ////////////////////////////////////////////
    GLuint *rec, *recNames, nameLength;
    int *hitNames = new int[PICK_BUFFER_SIZE];
    GLuint minZ, minDZ;

    //qDebug(" Renderer::selectObj hits = %d", hits);
    rec = (GLuint *) selectBuf;
    nameLength  = 0;
    minDZ    = GLuint(-1);
    recNames = (rec + 3);
    for (int i = 0; i < hits; i++)
    {
        //qDebug("   Z(%u) minDZ(%u) Hit[%i](%u [%u %u] %u %u %u)", zDepth,minDZ, i,rec[0],rec[1],rec[2], rec[3],rec[4],rec[5]);

        if (ABS((zDepth) - rec[1]) <= minDZ || i==0)// (closer) OR (yet no name)
            //************************************************************************
            if (rec[0] != 2) //090724: Ignore when nameLength==2 for Intel GL ICD bug
            //************************************************************************
        {
            nameLength  = rec[0];
            minZ        = rec[1];
            minDZ    = ABS((zDepth) - rec[1]); //081224
            recNames = (rec + 3);
            //qDebug("      --nameLength(%u) minDZ(%u) %u %u %u", nameLength, minDZ, recNames[0],recNames[1],recNames[2]);
        }

        rec += (3 + rec[0]); ///////////////////////////////////////// 081222

        if (((rec + 3) -selectBuf > PICK_BUFFER_SIZE)
        || (recNames + (nameLength) - selectBuf > PICK_BUFFER_SIZE))
        {
            qDebug("*** ERROR: pick buffer overflow !!!\n");
            return 0; //////////////////////////////////////////////// 081222
        }
    }
    //qDebug("      nameLength(%u) minDZ(%u) %u %u %u", nameLength, minDZ, recNames[0],recNames[1],recNames[2]);

    //printf("    The closest hitNames[%i]: ", nameLength);
    for (int j = 0; j < nameLength; j++)
    {
        //printf("%i ", recNames[j]);
        hitNames[j] = (int)recNames[j];
    }
    //printf("\n");

    //v3d_msg("hello before processHit",0);
    int ret = processHit((int)nameLength, hitNames, x, y, b_menu, pTip); //////////////////////
    //v3d_msg("hello after processHit",0);

    delete[] selectBuf;
    delete[] hitNames;
    return ret;
}


////////////////////////////////////////////////////////////////////////////////

void Renderer::initialize(int version)
{
    qDebug(" Renderer::initialize (%d)", version);
    if (version<0) return;

    ////////////////////////////////////////////////
    GLeeInit();
    makeCurrent();

    if (GLEE_ARB_multisample)
    {
        glEnable(GL_MULTISAMPLE_ARB); // default enabled by setSampleBuffers?
        GLint samples=0;
        glGetIntegerv(GL_SAMPLES_ARB, &samples);
        if (samples==2)  glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST); // 5tap for 2sample, 9tap for 4sample
    }

    // GL_*_SMOOTH affected by GL_MULTISAMPLE.
    // Otherwise combined with glEnable(GL_BLEND) & glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE), that is very limited, by RZC 081001
    // Polygon antialiasing is optimized using a blend function (GL_SRC_ALPHA_SATURATE, GL_ONE) with polygons sorted from nearest to farthest.
    //      Destination alpha bit planes, which must be present for this blend function to operate correctly, store the accumulated coverage.
    //glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_POLYGON_SMOOTH); // internal of quads, no effect?

    glDisable (GL_DITHER);
    glDisable (GL_FOG);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(0, +1); // deal z-fighting, 081121

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    if (rgbaBuf==0)   color_background = color_background2; // only geometric objects, 081023

    loadObj();
}

////////////////////////////////////////////////////////////////////////////////

void Renderer::paint()
{
    // normalized space of [-1,+1]^3;
    makeCurrent();
    //glClearColor(0.f, 0.f, 0.5f, 1.0f);
    glClearColor(color_background.r, color_background.g, color_background.b, color_background.a);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (polygonMode==1)	      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (polygonMode==2)  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else                      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //{
        glPushMatrix();

        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);

        setObjectSpace();
        //surfBoundingBox = boundingBox; // initialized in init()

        enableClipBoundingBox(surfBoundingBox, true, 0.001);
        //drawObj();
        //{
            GLUquadric* Q = gluNewQuadric();
            gluQuadricOrientation( Q, GLU_OUTSIDE);
            gluQuadricNormals(Q, GLU_SMOOTH);

            glTranslatef(.5f, .5f, .5f);
            glPushName(1);
            gluSphere( Q, .2,  36, 36);
            glPopName();

            gluDeleteQuadric(Q);
       // }
        disableClipBoundingBox();

        glDisable(GL_LIGHTING);

        glPopMatrix();
    //}

    if (!b_selecting) if (bShowBoundingBox || bShowAxes || bShowXYTranslateArrows)// a frame box [-1,+1]^3
    {
        setBoundingBoxSpace(boundingBox);
        drawBoundingBoxAndAxes(boundingBox);
    }

    return;
}

void Renderer::setObjectSpace()
{
    // bounding box space ==>fit in [-1,+1]^3
    setBoundingBoxSpace( boundingBox );
}

void Renderer::setBoundingBoxSpace(BoundingBox BB)
{
    float DX = BB.Dx();
    float DY = BB.Dy();
    float DZ = BB.Dz();
    float maxD = BB.Dmax();

    double s[3];
    s[0] = 1/maxD *2;
    s[1] = 1/maxD *2;
    s[2] = 1/maxD *2;
    double t[3];
    t[0] = -BB.x0 -DX /2;
    t[1] = -BB.y0 -DY /2;
    t[2] = -BB.z0 -DZ /2;

    // from boundingBox space ==> fit in [-1, +1]^3
    glScaled(s[0], s[1], s[2]);
    glTranslated(t[0], t[1], t[2]);

}

inline void box_quads(const BoundingBox & BB)
{
#define BB_VERTEX(xi,yi,zi)  glVertex3d(BB.x##xi, BB.y##yi, BB.z##zi)

    BB_VERTEX(0, 0, 0);	BB_VERTEX(0, 1, 0);	BB_VERTEX(1, 1, 0); BB_VERTEX(1, 0, 0); //z=0
    BB_VERTEX(0, 0, 1);	BB_VERTEX(0, 1, 1);	BB_VERTEX(1, 1, 1); BB_VERTEX(1, 0, 1); //z=1

    BB_VERTEX(0, 0, 0);	BB_VERTEX(1, 0, 0);	BB_VERTEX(1, 0, 1); BB_VERTEX(0, 0, 1); //y=0
    BB_VERTEX(0, 1, 0);	BB_VERTEX(1, 1, 0);	BB_VERTEX(1, 1, 1); BB_VERTEX(0, 1, 1); //y=1

    BB_VERTEX(0, 0, 0);	BB_VERTEX(0, 0, 1);	BB_VERTEX(0, 1, 1); BB_VERTEX(0, 1, 0); //x=0
    BB_VERTEX(1, 0, 0);	BB_VERTEX(1, 0, 1);	BB_VERTEX(1, 1, 1); BB_VERTEX(1, 1, 0); //x=1

}

inline void draw_tri(const XYZ P1, const XYZ P2, const XYZ P3, const XYZ offst)
{
    // front/back
    glBegin(GL_TRIANGLES);
    glVertex3f(P1.x, P1.y, P1.z);
    glVertex3f(P2.x, P2.y, P2.z);
    glVertex3f(P3.x, P3.y, P3.z);
    glVertex3f(P1.x+offst.x, P1.y+offst.y, P1.z+offst.z);
    glVertex3f(P2.x+offst.x, P2.y+offst.y, P2.z+offst.z);
    glVertex3f(P3.x+offst.x, P3.y+offst.y, P3.z+offst.z);
    glEnd();
    // sides
    glBegin(GL_QUADS);
    glVertex3f(P1.x, P1.y, P1.z);
    glVertex3f(P1.x+offst.x, P1.y+offst.y, P1.z+offst.z);
    glVertex3f(P2.x+offst.x, P2.y+offst.y, P2.z+offst.z);
    glVertex3f(P2.x, P2.y, P2.z);
    glVertex3f(P2.x, P2.y, P2.z);
    glVertex3f(P2.x+offst.x, P2.y+offst.y, P2.z+offst.z);
    glVertex3f(P3.x+offst.x, P3.y+offst.y, P3.z+offst.z);
    glVertex3f(P3.x, P3.y, P3.z);
    glVertex3f(P3.x, P3.y, P3.z);
    glVertex3f(P3.x+offst.x, P3.y+offst.y, P3.z+offst.z);
    glVertex3f(P1.x+offst.x, P1.y+offst.y, P1.z+offst.z);
    glVertex3f(P1.x, P1.y, P1.z);
    glEnd();
}

void Renderer::drawBoundingBoxAndAxes(BoundingBox BB, float BlineWidth, float AlineWidth)
{
    glPushAttrib(GL_LINE_BIT | GL_POLYGON_BIT);
            //| GL_DEPTH_BUFFER_BIT);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);

//	glPolygonOffset(0, -1); // deal z-fighting, 081120
//	glDepthFunc(GL_LEQUAL);
    if (posXTranslateBB != 0) delete posXTranslateBB;
    if (negXTranslateBB != 0) delete negXTranslateBB;
    if (posYTranslateBB != 0) delete posYTranslateBB;
    if (negYTranslateBB != 0) delete negYTranslateBB;
    posXTranslateBB=0, negXTranslateBB=0, posYTranslateBB=0, negYTranslateBB=0;
    if (bShowXYTranslateArrows && (iNegXTranslateArrowEnabled || iPosXTranslateArrowEnabled || iNegYTranslateArrowEnabled || iPosYTranslateArrowEnabled))
    {
        float D = (BB.Dmax());
        float td = D*0.015;
        XYZ A0 = BB.Vabsmin();
        XYZ A1 = BB.V1();// + D*0.05;
        XYZ Y0 = XYZ(0.5*(A0.x + A1.x) - td, A0.y - td, A0.z);
        XYZ Y1 = XYZ(0.5*(A0.x + A1.x) - td, A1.y + td, A0.z);
        XYZ X0 = XYZ(A0.x - td, 0.5*(A0.y + A1.y) - td, A0.z);
        XYZ X1 = XYZ(A1.x + td, 0.5*(A0.y + A1.y) - td, A0.z);

        glPolygonOffset(-0.002, -2); //(-0.002, -2) for good z-fighting with bounding box, 081120,100823

        glLineWidth(AlineWidth); // work only before glBegin(), by RZC 080827
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if (iNegXTranslateArrowEnabled)
        {
            if (iNegXTranslateArrowEnabled > 1)
                glColor3f(1, 0.7f, 0.7f);
            else
                glColor3f(0.7f, 0, 0);
            glBegin(GL_QUADS); box_quads( BoundingBox(X0, XYZ(X0.x-td, X0.y+2.0f*td, X0.z+td)) ); glEnd();
            draw_tri(XYZ(X0.x-td, X0.y-td, X0.z), XYZ(X0.x-td, X0.y+3.0f*td, X0.z), XYZ(X0.x-3.0f*td, X0.y+td, X0.z), XYZ(0.0f, 0.0f, td));
            negXTranslateBB = new BoundingBox(XYZ(X0.x-3.0f*td, X0.y-td, X0.z), XYZ(X0.x, X0.y+2.0f*td, X0.z+td));
        }
        if (iPosXTranslateArrowEnabled)
        {
            if (iPosXTranslateArrowEnabled > 1)
                glColor3f(1, 0.7f, 0.7f);
            else
                glColor3f(0.7f, 0, 0);
            glBegin(GL_QUADS); box_quads( BoundingBox(X1, XYZ(X1.x+td, X1.y+2.0f*td, X1.z+td)) ); glEnd();
            draw_tri(XYZ(X1.x+td, X1.y-td, X1.z), XYZ(X1.x+td, X1.y+3.0f*td, X1.z), XYZ(X1.x+3.0f*td, X1.y+td, X1.z), XYZ(0.0f, 0.0f, td));
            posXTranslateBB = new BoundingBox(XYZ(X1.x, X1.y-td, X1.z), XYZ(X1.x+3.0f*td, X1.y+3.0f*td, X1.z+td));
        }
        if (iNegYTranslateArrowEnabled)
        {
            if (iNegYTranslateArrowEnabled > 1)
                glColor3f(0.7f, 1, 0.7f);
            else
                glColor3f(0, 0.7f, 0);
            glBegin(GL_QUADS); box_quads( BoundingBox(Y0, XYZ(Y0.x+2.0f*td, Y0.y-td, Y0.z+td)) ); glEnd();
            draw_tri(XYZ(Y0.x-td, Y0.y-td, Y0.z), XYZ(Y0.x+3.0f*td, Y0.y-td, Y0.z), XYZ(Y0.x+td, Y0.y-3.0f*td, Y0.z), XYZ(0.0f, 0.0f, td));
            negYTranslateBB = new BoundingBox(XYZ(Y0.x-td, Y0.y-3.0f*td, Y0.z), XYZ(Y0.x+3.0f*td, Y0.y-td, Y0.z+td));
        }
        if (iPosYTranslateArrowEnabled)
        {
            if (iPosYTranslateArrowEnabled > 1)
                glColor3f(0.7f, 1, 0.7f);
            else
                glColor3f(0, 0.7f, 0);
            glBegin(GL_QUADS); box_quads( BoundingBox(Y1, XYZ(Y1.x+2.0f*td, Y1.y+td, Y1.z+td)) ); glEnd();
            draw_tri(XYZ(Y1.x-td, Y1.y+td, Y1.z), XYZ(Y1.x+3.0f*td, Y1.y+td, Y1.z), XYZ(Y1.x+td, Y1.y+3.0f*td, Y1.z), XYZ(0.0f, 0.0f, td));
            posYTranslateBB = new BoundingBox(XYZ(Y1.x-td, Y1.y+td, Y1.z), XYZ(Y1.x+3.0f*td, Y1.y+3.0f*td, Y1.z+td));
        }
    }

    // an indicator of coordinate direction
    if (bShowAxes && AlineWidth>0)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        float D = (BB.Dmax());
        float ld = D*0.0001; //1e-4 is best
        float td = D*0.015;
        XYZ A0 = BB.Vabsmin();
        XYZ A1 = BB.V1() + D*0.05;

        glPolygonOffset(-0.002, -2); //(-0.002, -2) for good z-fighting with bounding box, 081120,100823

        glLineWidth(AlineWidth); // work only before glBegin(), by RZC 080827
        glBegin(GL_QUADS);
        //glBegin(GL_LINES); // glPolygonOffset do NOT  influence GL_LINES
        {
            glColor3f(1, 0, 0);		box_quads( BoundingBox(A0, XYZ(A1.x, A0.y+ld, A0.z+ld)) );
            glColor3f(0, 1, 0);		box_quads( BoundingBox(A0, XYZ(A0.x+ld, A1.y, A0.z+ld)) );
            glColor3f(0, 0, 1);		box_quads( BoundingBox(A0, XYZ(A0.x+ld, A0.y+ld, A1.z)) );

        }
        glEnd();

        // this draw x,y,z coord
        glColor3f(1, 0, 0);		drawString(A1.x+td, A0.y, A0.z, "X", 1, 0);
        glColor3f(0, 1, 0);		drawString(A0.x, A1.y+td, A0.z, "Y", 1, 0);
        glColor3f(0, 0, 1);		drawString(A0.x, A0.y, A1.z+td, "Z", 1, 0);
    }

    if (bShowBoundingBox && BlineWidth>0)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glPolygonOffset(0, -1); // deal z-fighting with volume, 081120

        glLineWidth(BlineWidth); // work only before glBegin(), by RZC 080827
//        glBegin(GL_QUADS);qt6
//        //glBegin(GL_LINES);
//        {
//            glColor3fv(color_line.c);	box_quads(BB);
//        }
//        glEnd();
    }

    glPopAttrib();
}


void Renderer::drawVaa3DInfo(int fontsize)
{
    // no scale here
    GLdouble mRot[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    for (int i=0; i<3; i++) mRot[i*4 +3]=mRot[3*4 +i]=0; mRot[3*4 +3]=1; // only reserve rotation, remove translation in mRot

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    double aspect = double(screenW)/MAX(screenH,1);
    double halfw = 1.3*aspect;
    double halfh = 1.3;
    glOrtho(-halfw, halfw, -halfh, halfh, -1, 1000); // 1000 makes 0 at most front depth in z-buffer
    glTranslated(+0.8, -1.15, 0); // put at right-bottom corner

    double sbar = 0.1; // scale bar display size
    glScaled(sbar*2, sbar*2, sbar*2); //[0,1]-->[-1,+1]

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(mRot); // last rotation pose

    glPushAttrib(GL_LINE_BIT | GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glColor3fv(color_line.c);

    if (fontsize > 0)
    {
        BoundingBox BB = UNIT_BoundingBox;
        float D = (BB.Dmax());
        float ld = D*0.0001; //1e-4 is best
        float td = 0.02;
        XYZ A0 = BB.Vabsmin();
        XYZ A1 = BB.V1();

        char str[100];
        //sprintf(str, "%s", "BigNeuron.org");
        sprintf(str, "%s", "vaa3d.org");

        drawString(A0.x + td, A0.y, A0.z, str, 0, fontsize);  // since renderText is obsolate, the text cannot show. 20210825
//        drawString(A0.x + td, A0.y + td, A0.z, "bigneuron.org", 0, fontsize);
        //glColor3f(1, 0, 0);		drawString(A1.x + td, A0.y, A0.z, "X");
        //glColor3f(0, 1, 0);		drawString(A0.x, A1.y + td, A0.z, "Y");
        //glColor3f(0, 0, 1);		drawString(A0.x, A0.y, A1.z + td, "Z");
    }

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//added different edit modes display
void Renderer::drawEditInfo()
{
    // no scale here
    GLdouble mRot[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    for (int i=0; i<3; i++) mRot[i*4 +3]=mRot[3*4 +i]=0; mRot[3*4 +3]=1; // only reserve rotation, remove translation in mRot

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    double aspect = double(screenW)/MAX(screenH,1);
    double halfw = 1.3*aspect;
    double halfh = 1.3;
    glOrtho(-halfw, halfw, -halfh, halfh, -1, 1000); // 1000 makes 0 at most front depth in z-buffer
    glTranslated(+0.8, +1.15, 0); // put at right-bottom corner

    double sbar = 0.1; // scale bar display size
    glScaled(sbar*2, sbar*2, sbar*2); //[0,1]-->[-1,+1]

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(mRot); // last rotation pose

    glPushAttrib(GL_LINE_BIT | GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glColor3fv(color_line.c);

    if(editinput!=0)
    {
        BoundingBox BB = UNIT_BoundingBox;
        float D = (BB.Dmax());
        float ld = D*0.0001; //1e-4 is best
        float td = 0.02;
        XYZ A0 = BB.Vabsmin();
        XYZ A1 = BB.V1();

        char str[100];
        //sprintf(str, "%s", "BigNeuron.org");
        string editdisplay;
        switch (editinput)
        {
        case 1:  editdisplay = "Drawing BBox";break;
        case 2:
            if(neuronColorMode==0)
                editdisplay = "Retyping";
            else if (neuronColorMode==5)
                editdisplay = "Confidence Level";
            break;
        case 3:  editdisplay = "Deleting";break;
        case 4:  editdisplay = "Splitting";break;
        case 5:  editdisplay = "Drawing Global";break;
        case 6:  editdisplay = "Connecting";break;
        case 7:  editdisplay = "Defining Polyline";break;
        case 8:  editdisplay = "GD Tracing";break;
        case 9:  editdisplay = "Connecting (Loop Safe)"; break;
        case 10: editdisplay = "Highlight Subtree"; break;
        case 11: editdisplay = "Highlight Connected Segments"; break;
        case 12: editdisplay = "Fragment Tracing"; break;
        case 13: editdisplay = "Brain Atlas"; break;

        }

        sprintf(str, "%s", editdisplay.c_str());

        drawString(A0.x + td, A0.y, A0.z, str, 0, 18); // same renderText problem above, 20210825
//        drawString(A0.x + td, A0.y + td, A0.z, "bigneuron.org", 0, fontsize);
        //glColor3f(1, 0, 0);		drawString(A1.x + td, A0.y, A0.z, "X");
        //glColor3f(0, 1, 0);		drawString(A0.x, A1.y + td, A0.z, "Y");
        //glColor3f(0, 0, 1);		drawString(A0.x, A0.y, A1.z + td, "Z");
    }

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);


}


void Renderer::drawSegInfo()
{
    if (segInfoShow.empty())
    {
        //qDebug() << "No segmentation information updated.";
        return;
    }

    GLdouble mRot[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    for (int i=0; i<3; i++) mRot[i*4 +3]=mRot[3*4 +i]=0; mRot[3*4 +3]=1; // only reserve rotation, remove translation in mRot

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    double aspect = double(screenW)/MAX(screenH,1);
    double halfw = 1.3*aspect;
    double halfh = 1.3;
    glOrtho(-halfw, halfw, -halfh, halfh, -1, 1000); // 1000 makes 0 at most front depth in z-buffer
    glTranslated(-1.2, +1.15, 0); // put at right-bottom corner

    double sbar = 0.1; // scale bar display size
    glScaled(sbar*2, sbar*2, sbar*2); //[0,1]-->[-1,+1]

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(mRot); // last rotation pose

    glPushAttrib(GL_LINE_BIT | GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glColor3fv(color_line.c);

    BoundingBox BB = UNIT_BoundingBox;
    float D = (BB.Dmax());
    float ld = D*0.0001; //1e-4 is best
    float td = 0.02;
    XYZ A0 = BB.Vabsmin();
    XYZ A1 = BB.V1();
    XYZ A2 = BB.Vabsmax();

    if (connectEdit == segmentEdit)
    {
        stringstream totalSeg;
        stringstream segRemain;
        totalSeg << segInfoShow[0]; string segNum = totalSeg.str();
        string segEntry = "Segments connected: ";
        segRemain << segInfoShow[2]; string remainSegNum = segRemain.str();
        string unEntry = "Segments untouched: ";
        string segNumTex = segEntry + segNum + "   " + unEntry + remainSegNum;
        drawString(A0.x + td, A0.y, A0.z, &segNumTex[0], 0, 10);
    }
    else if (connectEdit == pointCloudEdit)
    {
        stringstream totalSeg;
        totalSeg << segInfoShow.size(); string segNum = totalSeg.str();
        string segEntry = "Number of segments created: ";
        string segNumTex = segEntry + segNum;
        drawString(A0.x + td, A0.y, A0.z, &segNumTex[0], 0, 10);
    }

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void Renderer::drawScaleBar(float AlineWidth)
{
    // no scale here
    GLdouble mRot[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    for (int i=0; i<3; i++) mRot[i*4 +3]=mRot[3*4 +i]=0; mRot[3*4 +3]=1; // only reserve rotation, remove translation in mRot

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    double aspect = double(screenW)/MAX(screenH,1);
    double halfw = 1.3*aspect;
    double halfh = 1.3;
    glOrtho(-halfw, halfw, -halfh, halfh, -1, 1000); // 1000 makes 0 at most front depth in z-buffer
    glTranslated(+0.8, -1.15, 0); // put at right-bottom corner

    double sbar = 0.1; // scale bar display size
    glScaled(sbar*2, sbar*2, sbar*2); //[0,1]-->[-1,+1]

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(mRot); // last rotation pose

    glPushAttrib(GL_LINE_BIT | GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glColor3fv(color_line.c);

    if (AlineWidth > 0)
    {
        BoundingBox BB = UNIT_BoundingBox;
        float D = (BB.Dmax());
        float ld = D*0.0001; //1e-4 is best
        float td = 0.02;
        XYZ A0 = BB.Vabsmin();
        XYZ A1 = BB.V1();

        glLineWidth(AlineWidth); // work only before glBegin(), by RZC 080827
        glBegin(GL_QUADS);
        {
            //glColor3f(1, 0, 0);
            box_quads(BoundingBox(A0, XYZ(A1.x, A0.y + ld, A0.z + ld)));
            //glColor3f(0, 1, 0);
            //box_quads(BoundingBox(A0, XYZ(A0.x + ld, A1.y, A0.z + ld)));
            //glColor3f(0, 0, 1);
            box_quads(BoundingBox(A0, XYZ(A0.x + ld, A0.y + ld, A1.z)));
        }
        glEnd();

        ////////////////////////////////////////
        double sizeX = bufSize[0]/sampleScale[0];
        double unitXscale = boundingBox.Dx()/boundingBox.Dmax();
        double sizeXunit = sizeX / unitXscale;
        char str[100];
        sprintf(str, "%g", sizeXunit * sbar * zoomRatio);
        qDebug("sizeX=%g unitXscale=%g sizeXunit=%g", sizeX, unitXscale, sizeXunit);

        drawString(A1.x + td, A0.y, A0.z, str, 0, 0);
        //glColor3f(1, 0, 0);		drawString(A1.x + td, A0.y, A0.z, "X");
        //glColor3f(0, 1, 0);		drawString(A0.x, A1.y + td, A0.z, "Y");
        //glColor3f(0, 0, 1);		drawString(A0.x, A0.y, A1.z + td, "Z");
    }

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// --------- MK, April, 2019, customized scale bar design --------- //
void Renderer::drawScaleBar_Yun(double voxDims[], int voxNums[], int VOIdims[], int resIndex, float zThickness, float AlineWidth)
{
    if (voxNums[0] == 0 || voxNums[1] == 0 || voxNums[2] == 0)
    {
        voxNums[0] = boundingBox.Dx();
        voxNums[1] = boundingBox.Dy();
        voxNums[2] = boundingBox.Dz();
    }

    int resLevel = std::pow(2.0, resIndex);
    //cout << resLevel << endl;
    // no scale here
    GLdouble mRot[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    for (int i = 0; i<3; i++) mRot[i * 4 + 3] = mRot[3 * 4 + i] = 0; mRot[3 * 4 + 3] = 1; // only reserve rotation, remove translation in mRot

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    double aspect = double(screenW) / MAX(screenH, 1);
    double halfw = 1.3*aspect;
    double halfh = 1.3;
    glOrtho(-halfw, halfw, -halfh, halfh, -1, 1000); // 1000 makes 0 at most front depth in z-buffer
    glTranslated(+0.8, -1.15, 0); // put at right-bottom corner

    double sbar = 0.1; // scale bar display size
    glScaled(sbar * 2, sbar * 2, sbar * 2); //[0,1]-->[-1,+1]

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixd(mRot); // last rotation pose

    glPushAttrib(GL_LINE_BIT | GL_POLYGON_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glColor3fv(color_line.c);

    if (AlineWidth > 0)
    {
        BoundingBox BB = UNIT_BoundingBox;
        float D = (BB.Dmax());
        float ld = D*0.0001; //1e-4 is best
        float td = 0.02;
        XYZ A0 = BB.Vabsmin();
        XYZ A1 = BB.V1();
        A0.x = A0.x - 1;
        A0.y = A0.y - 2.5;
        A0.z = A0.z - 2;
        A1.x = A1.x - 1;
        A1.y = A1.y - 2.5;
        A1.z = A1.z - 2;

        glLineWidth(AlineWidth); // work only before glBegin(), by RZC 080827
        glBegin(GL_QUADS);
        {
            //glColor3f(1, 0, 0);
            box_quads(BoundingBox(A0, XYZ(A1.x, A0.y + ld, A0.z + ld)));
            //glColor3f(0, 1, 0);
            box_quads(BoundingBox(A0, XYZ(A0.x + ld, A1.y, A0.z + ld)));
            //glColor3f(0, 0, 1);
            box_quads(BoundingBox(A0, XYZ(A0.x + ld, A0.y + ld, A1.z)));
        }
        glEnd();

        ////////////////////////////////////////
        double maxVOIdim;
        if (resLevel == 1) maxVOIdim = MAX(MAX(voxNums[0], voxNums[1]), voxNums[2]);
        else maxVOIdim = MAX(MAX(VOIdims[0], VOIdims[1]), VOIdims[2]);
        //cout << voxNums[0] << " " << voxNums[1] << " " << voxNums[2] << endl;
        //cout << VOIdims[0] << " " << VOIdims[1] << " " << VOIdims[2] << endl;
        //cout << maxVOIdim << endl << endl;

        int VOIdimRatio = 32 / resLevel;
        double xScale, yScale, zScale;
        if (VOIdimRatio == 32)
        {
            double unitXscale = boundingBox.Dx() / boundingBox.Dmax(); // scale bar ratio in 3 dimensions; the max among the 3 is 1
            double unitYscale = boundingBox.Dy() / boundingBox.Dmax();
            double unitZscale = boundingBox.Dz() / boundingBox.Dmax();
            xScale = round((sbar * zoomRatio) * (voxNums[0] * voxDims[0]) / unitXscale);
            yScale = round((sbar * zoomRatio) * (voxNums[1] * voxDims[1]) / unitYscale);
            zScale = round((sbar * zoomRatio) * (voxNums[2] * voxDims[2]) / unitZscale);
            zScale = round(zScale / double(zThickness));
        }
        else
        {
            double unitXscale = VOIdims[0] / maxVOIdim;
            double unitYscale = VOIdims[1] / maxVOIdim;
            double unitZscale = VOIdims[2] / maxVOIdim;
            xScale = round((VOIdims[0] * sbar * zoomRatio) * VOIdimRatio * voxDims[0] / unitXscale);
            yScale = round((VOIdims[1] * sbar * zoomRatio) * VOIdimRatio * voxDims[1] / unitYscale);
            zScale = round((VOIdims[2] * sbar * zoomRatio) * VOIdimRatio * voxDims[2] / unitZscale);
            zScale = round(zScale / double(zThickness));
        }

        char strX[100];
        char strY[100];
        char strZ[100];
        sprintf(strX, "%g", xScale);
        sprintf(strY, "%g", yScale);
        sprintf(strZ, "%g", zScale);

        //qDebug("sizeX=%g unitXscale=%g sizeXunit=%g", sizeX, unitXscale, sizeXunit);

        drawString(A1.x + td + 0.2, A0.y - 0.05, A0.z + 0.1, strX, 0, 8);
        drawString(A0.x + 0.2, A1.y + td + 0.1, A0.z + 0.1, strY, 0, 8);
        drawString(A0.x + 0.2, A0.y - 0.05, A1.z + td + 0.1, strZ, 0, 8);
        //drawString()
        glColor3f(0, 1, 0);		drawString(A1.x + td, A0.y - 0.05, A0.z, "X", 10);
        glColor3f(0, 1, 0);		drawString(A0.x, A1.y + td + 0.1, A0.z, "Y", 10);
        glColor3f(0, 1, 0);		drawString(A0.x, A0.y - 0.05, A1.z + td, "Z", 10);
    }

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
// ----------------------------------------------------------------- //

////////////////////////////////////////////////////////////////////////////////

void Renderer::setZoom(float ratio)
{
    if (ratio <-1) // [-inf,-1) zoom out..............
    {
        ratio = -ratio;
        zoomRatio = //1+(ratio);
                    //1+sqrt(ratio);
                    2+(150/viewAngle-1)*(1-2/(1+ratio));
    }
    else if (ratio < 0) // [-1, 0) zoom out
    {
        ratio = -ratio;
        zoomRatio = 1+(ratio);
                    //1+sqrt(ratio);
                    //1+pow(ratio, 1/3.0);
    }
    else	// [0+, +inf] zoom in.....................
    {
        zoomRatio = //1/(1+(ratio));
                    1/(1+(ratio)*(ratio));
    }
    setupView(screenW, screenH);
}

void Renderer::updateVolCutRange()
{
    int imageX = bufSize[0];
    int imageY = bufSize[1];
    int imageZ = bufSize[2];
    xCut0  = CLAMP(0, imageX-1, xCut0);
    yCut0  = CLAMP(0, imageY-1, yCut0);
    zCut0  = CLAMP(0, imageZ-1, zCut0);
    xCut1  = CLAMP(0, imageX-1, xCut1);
    yCut1  = CLAMP(0, imageY-1, yCut1);
    zCut1  = CLAMP(0, imageZ-1, zCut1);
}

void Renderer::updateSurfClipRange()
{
    xClip0  = CLAMP(0, 1, xClip0);
    yClip0  = CLAMP(0, 1, yClip0);
    zClip0  = CLAMP(0, 1, zClip0);
    xClip1  = CLAMP(0, 1, xClip1);
    yClip1  = CLAMP(0, 1, yClip1);
    zClip1  = CLAMP(0, 1, zClip1);
}

void Renderer::setSurfClipSpace() // no use this
{
    BoundingBox BB = surfBoundingBox;  // a copy
    float DX = BB.Dx();
    float DY = BB.Dy();
    float DZ = BB.Dz();
    float maxD = BB.Dmax();

    float s[3];
    s[0] = 1/maxD *2;
    s[1] = 1/maxD *2;
    s[2] = 1/maxD *2;
    float t[3];
    t[0] = -BB.x0 -DX /2;
    t[1] = -BB.y0 -DY /2;
    t[2] = -BB.z0 -DZ /2;

    // form surface object space ==> fit in [-1, +1]^3
    glScalef(s[0], s[1], s[2]);
    glTranslatef(t[0], t[1], t[2]);
}

void Renderer::enableClipBoundingBox(BoundingBox& bb, bool bDynamicClip, double border/*=0.001*/)
{
    BoundingBox BB = bb; // a copy for modifying
    //qDebug("	 enableClipBoundingBox  boundingBox (%g %g %g)--(%g %g %g)", BB.x0,BB.y0,BB.z0, BB.x1,BB.y1,BB.z1 );
    float maxD = BB.Dmax();
    if (maxD <= 0)  return; // no clip

    double dd = BB.Dmax()*border; // for a clip box border, 081102
    BB.x0 -= dd;   BB.x1 += dd;
    BB.y0 -= dd;   BB.y1 += dd;
    BB.z0 -= dd;   BB.z1 += dd;
    float DX = BB.Dx();
    float DY = BB.Dy();
    float DZ = BB.Dz();

    // in Object space
    GLdouble clip0[] = { +1.0, 0.0,  0.0, 1 }; // keep for dot(clip, pos)>=0
    GLdouble clip1[] = { -1.0, 0.0,  0.0, 1 };
    GLdouble clip2[] = { 0.0, +1.0,  0.0, 1 };
    GLdouble clip3[] = { 0.0, -1.0,  0.0, 1 };
    GLdouble clip4[] = { 0.0,  0.0, +1.0, 1 };
    GLdouble clip5[] = { 0.0,  0.0, -1.0, 1 };

    if (! bDynamicClip)
    {
        clip0[3] = -BB.x0;
        clip1[3] =  BB.x1;
        clip2[3] = -BB.y0;
        clip3[3] =  BB.y1;
        clip4[3] = -BB.z0;
        clip5[3] =  BB.z1;
    }
    else
    {
        clip0[3] = -BB.x0 - DX*(xClip0);
        clip1[3] =  BB.x1 - DX*(1-xClip1);
        clip2[3] = -BB.y0 - DY*(yClip0);
        clip3[3] =  BB.y1 - DY*(1-yClip1);
        clip4[3] = -BB.z0 - DZ*(zClip0);
        clip5[3] =  BB.z1 - DZ*(1-zClip1);
        //	clip0[3] = 1 - 2*(xClip0);
        //	clip1[3] = 1 - 2*(1-xClip1);
        //	clip2[3] = 1 - 2*(yClip0);
        //	clip3[3] = 1 - 2*(1-yClip1);
        //	clip4[3] = 1 - 2*(zClip0);
        //	clip5[3] = 1 - 2*(1-zClip1);
    }

    //configure the clip planes
    glClipPlane(GL_CLIP_PLANE0, clip0);
    glClipPlane(GL_CLIP_PLANE1, clip1);
    glClipPlane(GL_CLIP_PLANE2, clip2);
    glClipPlane(GL_CLIP_PLANE3, clip3);
    glClipPlane(GL_CLIP_PLANE4, clip4);
    glClipPlane(GL_CLIP_PLANE5, clip5);

    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
    glEnable(GL_CLIP_PLANE3);
    glEnable(GL_CLIP_PLANE4);
    glEnable(GL_CLIP_PLANE5);
}
//void Renderer::restoreClipBoundingBox()
//{
//	if (! b_useClipBoxforSubjectObjs) return;
//	glEnable(GL_CLIP_PLANE0);
//	glEnable(GL_CLIP_PLANE1);
//	glEnable(GL_CLIP_PLANE2);
//	glEnable(GL_CLIP_PLANE3);
//	glEnable(GL_CLIP_PLANE4);
//	glEnable(GL_CLIP_PLANE5);
//}
void Renderer::disableClipBoundingBox()
{
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
}


void Renderer::setViewClip(float f)
{
    // [0, 1] ==> [+1, -1]*(2)  // 2>sqrt(2)
    viewClip = (1-2*f)*(2);
    viewClip = CLAMP(-2, +2, viewClip);
}

void Renderer::enableViewClipPlane()
{
    //qDebug("	 enableViewClipPlane  %g", viewClip);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // in View space
    // keep for dot(clip, pos)>=0
    GLdouble clip0[] = { 0.0,  0.0, -1.0,  0 };
    // [0, 1] ==> [+1, -1]*(s)
    clip0[3] = viewClip;

    //configure the clip planes
    glClipPlane(GL_CLIP_PLANE0, clip0);
    glEnable(GL_CLIP_PLANE0);

    glPopMatrix();
}
void Renderer::disableViewClipPlane()
{
    glDisable(GL_CLIP_PLANE0);
}


////////////////////////////////////////////////////////////////////////////////
#define __data_buffer_functions__
//090731: memory offset change to V3DLONG for 64bit

unsigned char* createSimulatedData(V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4, V3DLONG dim5)
{
    // TIME_PACK_Z:  [c][t*z][y][x]

    V3DLONG dimx = dim1;
    V3DLONG dimy = dim2;
    V3DLONG dimz = dim3;
    V3DLONG dimc = dim4;
    V3DLONG dimt = dim5;

    V3DLONG size = dimx*dimy*dimz*dimc*dimt;
    unsigned char* data = new unsigned char[size];
    if (! data)
        return 0;

    V3DLONG x,y,z,c, t;
    for (t=0; t<dimt; t++)
        for (c=0; c<dimc; c++)
            for (z=0; z<dimz; z++)
            for (y=0; y<dimy; y++)
            for (x=0; x<dimx; x++)
                {
                    V3DLONG p = x + dimx*y + dimx*dimy*z + dimx*dimy*dimz*c + dimx*dimy*dimz*dimc*t;
                    data[p] = ( (x+y+z)*(c+1)*(t+1) )%180;
                }
    return data;
}

void getLimitedSampleScaleBufSize(V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4, V3DLONG dim5,
        float s[5], V3DLONG bs[5])
{
    MESSAGE_ASSERT(s);
    MESSAGE_ASSERT(bs);

    V3DLONG dimx = dim1;
    V3DLONG dimy = dim2;
    V3DLONG dimz = dim3;
    V3DLONG dimc = dim4;
    V3DLONG dimt = dim5;

    s[0] = (dimx>0)? MIN(LIMIT_VOLX, dimx)/float(dimx) : 1;
    s[1] = (dimy>0)? MIN(LIMIT_VOLY, dimy)/float(dimy) : 1;
    s[2] = (dimz>0)? MIN(LIMIT_VOLZ, dimz)/float(dimz) : 1;
    s[3] = 1;
    s[4] = 1;

    bs[0] = V3DLONG(s[0] * dim1);
    bs[1] = V3DLONG(s[1] * dim2);
    bs[2] = V3DLONG(s[2] * dim3);
    bs[3] = dim4;
    bs[4] = dim5;
}

void rgba3d_r2gray(RGBA8* rgbaBuf, V3DLONG bufSize[5])
{
    if (rgbaBuf==0 || bufSize==0)
        return;

    // copy R to G,B
    V3DLONG imageX, imageY, imageZ, imageC, imageT;
    {
        imageX = bufSize[0];
        imageY = bufSize[1];
        imageZ = bufSize[2];
        imageC = 4;
        imageT = bufSize[4];
    }
    if (imageX*imageY*imageZ==0)
        return;

    V3DLONG ot;
    V3DLONG ox, oy, oz;
    for (ot=0; ot<imageT; ot++)
    for (oz = 0; oz < imageZ; oz++)
    for (oy = 0; oy < imageY; oy++)
    for (ox = 0; ox < imageX; ox++)
        {
            RGBA8 rgba;
            V3DLONG p = ot*(imageZ*imageY*imageX) + oz*(imageY*imageX) + oy*(imageX) + ox;

            rgba = rgbaBuf[p];
            rgbaBuf[p].g = rgba.r;
            rgbaBuf[p].b = rgba.r;
        }
}

void data4dp_to_rgba3d(Image4DProxy<Image4DSimple>& img4dp, V3DLONG dim5,
        V3DLONG start1, V3DLONG start2, V3DLONG start3, V3DLONG start4,
        V3DLONG size1, V3DLONG size2, V3DLONG size3, V3DLONG size4,
        RGBA8* rgbaBuf, V3DLONG bufSize[5])
{
    if (rgbaBuf==0 || bufSize==0)
        return;

    //there may be a memory issue? by PHC 20110122
    //	if (img4dp.su!=1)
//	{
//		v3d_msg("Your data is not 8bit. Now this data4dp_to_rgba3d(0 function supports only 8bit data.");
//		return;
//	}

    V3DLONG dim1=img4dp.sx; V3DLONG dim2=img4dp.sy; V3DLONG dim3=img4dp.sz;
    V3DLONG dim4=img4dp.sc;
    #define SAMPLE(it, ic, ix,iy,iz, dx,dy,dz) \
                (unsigned char)sampling3dUINT8( img4dp, (it*dim4/imageT + ic), \
                                                ix, iy, iz, dx, dy, dz )

        #define SAMPLE2(si, ix,iy,iz, dx,dy,dz, dxyz) \
                        (unsigned char)sampling3dUINT8_2( img4dp, si, ix, iy, iz, dx, dy, dz, dxyz)

    // only convert 1<=dim4<=4 ==> RGBA
    V3DLONG imageX, imageY, imageZ, imageC, imageT;
    {
        imageX = bufSize[0];
        imageY = bufSize[1];
        imageZ = bufSize[2];
                imageC = MIN(4, size4); // <=4
        imageT = bufSize[4];
    }
    if (imageX*imageY*imageZ*imageC*imageT==0)
        return;

    float sx, sy, sz;
    V3DLONG dx, dy, dz;
    sx = float(size1) / imageX;
    sy = float(size2) / imageY;
    sz = float(size3) / imageZ;
    dx = V3DLONG(sx);
    dy = V3DLONG(sy);
    dz = V3DLONG(sz);
        V3DLONG dxyz = dx*dy*dz;
    MESSAGE_ASSERT(dx*dy*dz >=1); //down sampling

    V3DLONG ot;
    V3DLONG ox, oy, oz;
    V3DLONG ix, iy, iz;

        V3DLONG otOffset, ozOffset, oyOffset, oxOffset;

        V3DLONG SAM0, SAM1, SAM2, SAM3;


        for (ot=0; ot<imageT; ot++) {
            SAM0 = ot*dim4/imageT + 0;
            SAM1 = SAM0+1;
            SAM2 = SAM0+2;
            SAM3 = SAM0+3;
            otOffset=ot*(imageZ*imageY*imageX);
            for (oz = 0; oz < imageZ; oz++) {
                ozOffset=oz*(imageY*imageX);
                iz = start3+ CLAMP(0,dim3-1, IROUND(oz*sz));
                for (oy = 0; oy < imageY; oy++) {
                    oyOffset=oy*imageX;
                    oxOffset=otOffset+ozOffset+oyOffset;
                    iy = start2+ CLAMP(0,dim2-1, IROUND(oy*sy));

                    if (imageC==1) {
                        for (ox=0;ox<imageX;ox++) {
                            ix = start1+ CLAMP(0,dim1-1, IROUND(ox*sx));
                            RGBA8 rgba;
                            rgba.r = SAMPLE2(SAM0, ix,iy,iz, dx,dy,dz, dxyz);
                            rgba.g = 0;
                            rgba.b = 0;
                            float t = (0.f + rgba.r + rgba.g + rgba.b);
                            rgba.a = (unsigned char)t;
                            rgbaBuf[oxOffset++] = rgba;
                        }
                    }

                    if (imageC==2) {
                        for (ox=0;ox<imageX;ox++) {
                            ix = start1+ CLAMP(0,dim1-1, IROUND(ox*sx));
                            RGBA8 rgba;
                            rgba.r = SAMPLE2(SAM0, ix,iy,iz, dx,dy,dz, dxyz);
                            rgba.g = SAMPLE2(SAM1, ix,iy,iz, dx,dy,dz, dxyz);;
                            rgba.b = 0;
                            float t = (0.f + rgba.r + rgba.g + rgba.b)/2.0;
                            rgba.a = (unsigned char)t;
                            rgbaBuf[oxOffset++] = rgba;
                        }
                    }

                    if (imageC==3) {
                        for (ox=0;ox<imageX;ox++) {
                            ix = start1+ CLAMP(0,dim1-1, IROUND(ox*sx));
                            RGBA8 rgba;
                            rgba.r = SAMPLE2(SAM0, ix,iy,iz, dx,dy,dz, dxyz);
                            rgba.g = SAMPLE2(SAM1, ix,iy,iz, dx,dy,dz, dxyz);
                            rgba.b = SAMPLE2(SAM2, ix,iy,iz, dx,dy,dz, dxyz);
                            float t = (0.f + rgba.r + rgba.g + rgba.b)/3.0;
                            rgba.a = (unsigned char)t;
                            rgbaBuf[oxOffset++] = rgba;
                        }
                    }

                    if (imageC>=4) {
                        for (ox=0;ox<imageX;ox++) {
                            ix = start1+ CLAMP(0,dim1-1, IROUND(ox*sx));
                            RGBA8 rgba;
                            rgba.r = SAMPLE2(SAM0, ix,iy,iz, dx,dy,dz, dxyz);
                            rgba.g = SAMPLE2(SAM1, ix,iy,iz, dx,dy,dz, dxyz);
                            rgba.b = SAMPLE2(SAM2, ix,iy,iz, dx,dy,dz, dxyz);
                            rgba.a = SAMPLE2(SAM3, ix,iy,iz, dx,dy,dz, dxyz);
                            rgbaBuf[oxOffset++] = rgba;
                        }
                    }
                }
            }
        }
}

void data4dp_to_rgba3d(unsigned char* data4dp, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4, V3DLONG dim5,
        V3DLONG start1, V3DLONG start2, V3DLONG start3, V3DLONG start4,
        V3DLONG size1, V3DLONG size2, V3DLONG size3, V3DLONG size4,
        RGBA8* rgbaBuf, V3DLONG bufSize[5])
{
    if (data4dp==0 || rgbaBuf==0 || bufSize==0)
        return;

    //V3DLONG block_size = dim3*dim2*dim1;
    #define SAMPLE(it, ic, ix,iy,iz, dx,dy,dz) \
                (unsigned char)sampling3dAllTypes( data4dp+ (it*dim4 + ic)*(dim3*dim2*dim1), \
                                                dim1, dim2, dim3, ix, iy, iz, dx, dy, dz )

    // only convert 1<=dim4<=4 ==> RGBA
    V3DLONG imageX, imageY, imageZ, imageC, imageT;
    {
        imageX = bufSize[0];
        imageY = bufSize[1];
        imageZ = bufSize[2];
        imageC = MIN(4, size4); // <=4
        imageT = bufSize[4];
    }
    if (imageX*imageY*imageZ*imageC*imageT==0)
        return;

    float sx, sy, sz;
    V3DLONG dx, dy, dz;
    sx = float(size1) / imageX;
    sy = float(size2) / imageY;
    sz = float(size3) / imageZ;
    dx = V3DLONG(sx);
    dy = V3DLONG(sy);
    dz = V3DLONG(sz);
    MESSAGE_ASSERT(dx*dy*dz >=1); //down sampling

    V3DLONG ot;
    V3DLONG ox, oy, oz;
    V3DLONG ix, iy, iz;
    for (ot=0; ot<imageT; ot++)
    for (oz = 0; oz < imageZ; oz++)
    for (oy = 0; oy < imageY; oy++)
    for (ox = 0; ox < imageX; ox++)
        {
            ix = start1+ CLAMP(0,dim1-1, IROUND(ox*sx));
            iy = start2+ CLAMP(0,dim2-1, IROUND(oy*sy));
            iz = start3+ CLAMP(0,dim3-1, IROUND(oz*sz));

            RGBA8 rgba;

            if (imageC >= 1) {
                rgba.r = SAMPLE(ot, 0, ix,iy,iz, dx,dy,dz);
            } else {
                rgba.r = 0;
            }

            if (imageC >= 2) {
                rgba.g = SAMPLE(ot, 1, ix,iy,iz, dx,dy,dz);
            } else {
                rgba.g = 0;
            }

            if (imageC >= 3) {
                rgba.b = SAMPLE(ot, 2, ix,iy,iz, dx,dy,dz);
            } else {
                rgba.b = 0;
            }

            if (imageC >= 4) {
                rgba.a = SAMPLE(ot, 3, ix,iy,iz, dx,dy,dz);
            } else {
                float t = //MAX(rgba.r, MAX(rgba.g, rgba.b));
                            ((0.f + rgba.r + rgba.g + rgba.b) / imageC);
                rgba.a = (unsigned char)t;
                            //(unsigned char)(t*t/255);
                            //(unsigned char)(sqrt(t/255)*255);
            }

            rgbaBuf[ot*(imageZ*imageY*imageX) + oz*(imageY*imageX) + oy*(imageX) + ox] = rgba;
        }
}

// 090602 RZC: inline cannot be used in *.cpp but in *.h

// Please see the below comment for 'sampling3dUNIT8_2' in relation to this function.
float sampling3dUINT8(Image4DProxy<Image4DSimple>& img4dp,
        V3DLONG c,
        V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz)
{
    V3DLONG dim1=img4dp.sx; V3DLONG dim2=img4dp.sy; V3DLONG dim3=img4dp.sz;
    float avg = 0;
    float d = (dx*dy*dz);
    if (d>0 && x>=0 && y>=0 && z>=0 && x+dx<=dim1 && y+dy<=dim2 && z+dz<=dim3)
    {
        //float s = 0;
        V3DLONG xi,yi,zi;
        for (zi=0; zi<dz; zi++)
        for (yi=0; yi<dy; yi++)
        for (xi=0; xi<dx; xi++)
        {
            //float w = MAX( (2-ABS(xi-0.5*dx)*2.0/dx), MAX( (2-ABS(yi-0.5*dy)*2.0/dy), (2-ABS(zi-0.5*dz)*2.0/dz) )); //090712
            //s += w;
            avg += img4dp.value8bit_at(x,y,z,c);// *w;
        }
        //d = s;
        avg /= d;
    }
    return avg;
}

// There is a mystery as to why the above function, sampling3dUNIT8, uses a hard-coded 'img4dp.value8bit_at(x,y,z,c)' rather than
// sampling using the zi,yi,xi indices. Since it does not appear to use the indices, it is equivalent to the below implementation,
// which speeds-up the parent function quite a bit. Until we can resolve what is going on, we are temporarily using this
// fast version.
float sampling3dUINT8_2(Image4DProxy<Image4DSimple>& img4dp,
                V3DLONG c,
                V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz, V3DLONG dxyz)
{
        V3DLONG dim1=img4dp.sx; V3DLONG dim2=img4dp.sy; V3DLONG dim3=img4dp.sz;
        float avg = 0;
        if (dxyz>0 && x>=0 && y>=0 && z>=0 && x+dx<=dim1 && y+dy<=dim2 && z+dz<=dim3)
        {
                avg = img4dp.value8bit_at(x,y,z,c);
        }
        return avg;
}


//inline
RGBA32f sampling3dRGBA8(/*RGBA32f& sample,*/ RGBA8* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
        V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz)
{
    RGBA32f avg;
    avg.r = avg.g = avg.b = avg.a = 0;
    float d = (dx*dy*dz);
    if (d>0 && x>=0 && y>=0 && z>=0 && x+dx<=dim1 && y+dy<=dim2 && z+dz<=dim3)
    {
        V3DLONG xi,yi,zi;
        for (zi=0; zi<dz; zi++)
        for (yi=0; yi<dy; yi++)
        for (xi=0; xi<dx; xi++)
        {
            RGBA8 tmp = data[(z + zi)*dim2*dim1 + (y + yi)*dim1 + (x + xi)];
            avg.r += tmp.r;
            avg.g += tmp.g;
            avg.b += tmp.b;
            avg.a += tmp.a;
        }
        avg.r /= d;
        avg.g /= d;
        avg.b /= d;
        avg.a /= d;
        //sample = avg;
    }
    return avg;
}


//inline
RGBA32f sampling3dRGBA8at(RGBA8* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
        float x, float y, float z)
{
    #define SAMPLE(ix,iy,iz)	sampling3dRGBA8(data,dim1,dim2,dim3,  ix,iy,iz, 1,1,1)

    V3DLONG x0,x1, y0,y1, z0,z1;
    x0 = floor(x); 		x1 = ceil(x);
    y0 = floor(y); 		y1 = ceil(y);
    z0 = floor(z); 		z1 = ceil(z);
    float xf, yf, zf;
    xf = x-x0;
    yf = y-y0;
    zf = z-z0;
    RGBA32f is[2][2][2];
    is[0][0][0] = SAMPLE(x0, y0, z0);
    is[0][0][1] = SAMPLE(x0, y0, z1);
    is[0][1][0] = SAMPLE(x0, y1, z0);
    is[0][1][1] = SAMPLE(x0, y1, z1);
    is[1][0][0] = SAMPLE(x1, y0, z0);
    is[1][0][1] = SAMPLE(x1, y0, z1);
    is[1][1][0] = SAMPLE(x1, y1, z0);
    is[1][1][1] = SAMPLE(x1, y1, z1);
    float sf[2][2][2];
    sf[0][0][0] = (1-xf)*(1-yf)*(1-zf);
    sf[0][0][1] = (1-xf)*(1-yf)*(  zf);
    sf[0][1][0] = (1-xf)*(  yf)*(1-zf);
    sf[0][1][1] = (1-xf)*(  yf)*(  zf);
    sf[1][0][0] = (  xf)*(1-yf)*(1-zf);
    sf[1][0][1] = (  xf)*(1-yf)*(  zf);
    sf[1][1][0] = (  xf)*(  yf)*(1-zf);
    sf[1][1][1] = (  xf)*(  yf)*(  zf);

    RGBA32f* ip = &is[0][0][0];
    float* sp = &sf[0][0][0];
    RGBA32f sum;
    sum.r = sum.g = sum.b = sum.a = 0;
    for (V3DLONG i=0; i<8; i++) // weight sum
    {
        sum = sum + (ip[i]) * sp[i];
    }
    return (sum);
}

//inline
RGB8 getRGB3dUINT8(unsigned char* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4,
        V3DLONG x, V3DLONG y, V3DLONG z)
{
    RGB8 tmp;
    tmp.r=tmp.g=tmp.b= 0;
    if (x>=0 && y>=0 && z>=0 && x<dim1 && y<dim2 && z<dim3)
    {
        for (V3DLONG ci=0; ci<3 && ci<dim4; ci++)
        {
            tmp.c[ci] = (data + ci*(dim3*dim2*dim1)) [(z)*dim2*dim1 + (y)*dim1 + (x)];
        }
    }
    return tmp;
}

//inline
void setRGB3dUINT8(unsigned char* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4,
        V3DLONG x, V3DLONG y, V3DLONG z, RGB8 tmp)
{
    if (x>=0 && y>=0 && z>=0 && x<dim1 && y<dim2 && z<dim3)
    {
        for (V3DLONG ci=0; ci<3 && ci<dim4; ci++)
        {
            (data + ci*(dim3*dim2*dim1)) [(z)*dim2*dim1 + (y)*dim1 + (x)] = tmp.c[ci];
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
#define __drawing_operators__

int numPassFloatDraw(int sShow)
{
    if (sShow==1) return 1;
    else if (sShow==2) return 3;
    return 0;
}
void setFloatDrawOp(int pass, int sShow) // operator FloatDraw, by Ruan Zongcai 2008-09-03
{
    //		// debug
    //		int x=400, y=400; GLfloat depth, stencil;
    //		glReadPixels(x,y,1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    //		glReadPixels(x,y,1,1, GL_STENCIL_INDEX, GL_FLOAT, &stencil);
    //		qDebug("--- (%d %d) depth stencil = %g %g", x, y, depth, stencil);
    //return;//turn off for debug

    if (pass== -1)
    {
        glPopAttrib();//(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)
    }

    //==================================================================================

    if (pass==0 && sShow==1) // NOT floating pass 0
    {
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	//
        glDepthMask(GL_TRUE);								//
        glStencilMask(~0);									//
                                                            //
        glStencilFunc(GL_ALWAYS, 2, ~0);					// 2--object(surface) id
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 			// write object mask
        glDepthFunc(GL_LESS);								// when z <?
    }

    //==================================================================================
    //void void glStencilFunc( GLenum func, GLint ref, GLuint mask ) // ref func ?
    //void glStencilOp( GLenum sfail, GLenum spass_zfail, GLenum spass_zpass )
    //==================================================================================

    if (pass==0 && sShow==2) // HAVE floating pass 0
    {
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        /////////////////////////////////////////////
        glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT );
        glDisable(GL_CULL_FACE); // two-side face
        glDisable(GL_LIGHTING);  // no lighting
        glShadeModel(GL_FLAT);   // no interpolating
        /////////////////////////////////////////////

        glEnable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);// no color
        glDepthMask(GL_TRUE);								//
        glStencilMask(~0);          //~0?
                                                            //
        glStencilFunc(GL_GREATER, 1, ~0);					// 1>? 0--background(image) id
        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);			// write 1--drawable region mask, over image
        //glStencilOp(GL_KEEP, GL_KEEP,    GL_REPLACE);			// write 1--drawable region mask, over object group
        glDepthFunc(GL_GEQUAL);								// when zfar >=? (set drawable background region depth to zfar)
        glDepthRange(0.999, 1);				// write depth the farthest, .999 for compatible with polygonOffset

        //glStencilFunc(GL_NEVER, 0, 0);			// turn off part 1, for debug
        //090726 RZC:
        //glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE) for float over image
        //glStencilOp(GL_KEEP, GL_KEEP,    GL_REPLACE) for float over image & object group's render order

    }
    if (pass==1 && sShow==3) // floating pass 1 -----------------------------------------------------------------------
    {
        glStencilFunc(GL_LESS, 1, ~0);						// 1<? 2--object id
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);			// write 1--drawable region mask, just set mask part 2
        glDepthFunc(GL_LESS);								// when z <? (just write object normal depth)
        glDepthRange(0, 1);					// write depth normally

        glStencilFunc(GL_NEVER, 0, 0);			// turn off part 2, for debug

    }
    if (pass==1 && sShow==2) // floating pass 2 -----------------------------------------------------------------------
    {
        //qDebug("--- setFloatDrawOp(2,2)");

        ///////////////////////////////////////////////
        glPopAttrib(); //(GL_ENABLE_BIT | GL_LIGHTING_BIT )
        //glShadeModel(GL_SMOOTH); // include in GL_LIGHTING_BIT
        ///////////////////////////////////////////////
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    //
        glDepthMask(GL_TRUE);								//
        glStencilMask(0xFF);									// HAVE stencil
        glDepthRange(0, 1);					// write depth normally
                                                            //
        glStencilFunc(GL_EQUAL, 1, ~0);						// =1--drawable region id, real drawing here
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 			// no change stencil mask
        glDepthFunc(GL_LEQUAL);								// when z<=? draw object
    }
    if (pass==2 && sShow==2) // floating pass 3 -------------------------------------------------------------------------
    {
        //qDebug("--- setFloatDrawOp(3,2)");

        //////////////////////////////////////////////
        glDisable(GL_LIGHTING);  // no lighting
        //////////////////////////////////////////////
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);// no color
        glDepthMask(GL_FALSE);								// no depth
        glStencilMask(0x00);									//
                                                            //
        glStencilFunc(GL_EQUAL, 1, ~0);						// =1--drawable region mask
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); 			// increase mask to 2-object id, no drawing, just stencil
        glDepthFunc(GL_LEQUAL);								// when all z pass
    }
}

////////////////////////////////////////////////////////////

void blendBrighten(float fbright, float fcontrast) // fast, 8-bit precision
{
    //fcontrast = 1.5;
    if (fbright==0 && fcontrast==1) return;

    if (fbright>=-1 && fbright<=1)
    {
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_BLEND);
#define RECT_BLEND	glRecti(-100,-100, 100,100) // assume [-1,+1]^3 view

        if (fcontrast <=1)
        {
            if (fbright >=0)
            {
                glBlendEquationEXT(GL_FUNC_ADD_EXT);
            }
            else //fbright <0
            {
                glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT);
                fbright = -fbright;
            }
            glBlendFunc(GL_ONE, GL_SRC_ALPHA); // new_color = 1*fbright + fcontrast*old_color
            glColor4f(fbright, fbright, fbright, fcontrast);
            RECT_BLEND;
        }

        else //fcontrast >1
        {
            float res = fcontrast -1; // int(fcontrast);
            if (res)
            {
                glBlendEquationEXT(GL_FUNC_ADD_EXT);
                glBlendFunc(GL_DST_COLOR, GL_ONE); // new_color = res*old_color + 1*old_color;
                glColor4f(res, res, res, 1);
                RECT_BLEND;
            }

            if (fbright >=0)
            {
                glBlendEquationEXT(GL_FUNC_ADD_EXT);
            }
            else //fbright <0
            {
                glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT);
                fbright = -fbright;
            }
            glBlendFunc(GL_ONE, GL_ONE); // new_color = 1*fbright + 1*old_color
            glColor4f(fbright, fbright, fbright, 1);
            RECT_BLEND;
        }

        glPopMatrix();
        glPopAttrib();
    }
}

void accumBrighten(float fbright, float fcontrast)  // slow, 16-bit precision, and need extra accumulation buffer
{
    if (fbright>=-1 && fbright<=1)
    {
        glAccum(GL_LOAD, fcontrast);
        glAccum(GL_ADD, fbright);
        glAccum(GL_RETURN, 1);
    }
}

/////////////////////////////////////////////////////////////////////

void ModelRotToView(const double mRot[16], double& vx, double& vy, double& vz)
{
    double M[4][4];
    MAT16_TO_MAT4x4( mRot, M );
    double mx, my, mz;
    // vr = M * mr,  M is column-first-index
    mx = M[0][0]*vx + M[1][0]*vy + M[2][0]*vz;
    my = M[0][1]*vx + M[1][1]*vy + M[2][1]*vz;
    mz = M[0][2]*vx + M[1][2]*vy + M[2][2]*vz;

    vx =mx; vy =my; vz =mz;
}

void ViewRotToModel(const double mRot[16], double& vx, double& vy, double& vz)
{
    double M[4][4];
    MAT16_TO_MAT4x4( mRot, M );
    double  mx, my, mz;
    // mr = Mt * vr,  Mt = Mi for rotation, Mt is row-first-index
    mx = M[0][0]*vx + M[0][1]*vy + M[0][2]*vz;
    my = M[1][0]*vx + M[1][1]*vy + M[1][2]*vz;
    mz = M[2][0]*vx + M[2][1]*vy + M[2][2]*vz;

    vx =mx; vy =my; vz =mz;
}

/* inverse matrix
 * [a b]     [d  -b] /
 * [c d]i  = [-c  a]/(ad-bc)
 *
 * [A B]     [Ai + Ai*B* (D - C*Ai*B)i *C*Ai,    -Ai*B* (D - C*Ai*B)i]
 * [C D]i  = [         - (D - C*Ai*B)i *C*Ai,           (D - C*Ai*B)i]
 */

//MV = [u,v,w,t]

void ViewToModel(const double modelView[16], double& vx, double& vy, double& vz)
{
    ColumnVector X(4);		X << vx << vy << vz << 1;
    Matrix MV(4,4);		MV << modelView;   MV = MV.t();    // OpenGL is colume-first-index / C is row-first-index
    ColumnVector Y  = MV.i() * X;

    // MV is affine transformation
    vx =Y(1); vy =Y(2); vz =Y(3);
}

void ViewPlaneToModel(const double modelView[16], double plane[4])
{
    ColumnVector X(4);		X << plane[0] << plane[1] << plane[2] << plane[3];
    Matrix MV(4,4);		MV << modelView;   MV = MV.t();    // OpenGL is colume-first-index / C is row-first-index
    ColumnVector Y  = MV.t() * X;
    double len = sqrt(Y(1)*Y(1) + Y(2)*Y(2) + Y(3)*Y(3));
    if (len)
    {
        Y /= len;
    }
    plane[0] =Y(1); plane[1] =Y(2); plane[2] =Y(3), plane[3] =Y(4);
}

/////////////////////////////////////////////////////////////////////////////////////////////

#define format_type(components, data_type, compress_format) \
    int iformat, format, type; \
    switch(data_type) \
    { \
    case 2: type=GL_UNSIGNED_SHORT; break; \
    case 3: type=GL_FLOAT; break; \
    case 4: type=GL_FLOAT; break; \
    case 1: \
    default: type=GL_UNSIGNED_BYTE; \
    } \
    switch(components) \
    { \
    case 1: iformat=format=GL_LUMINANCE; \
                            if (data_type==2) iformat=GL_LUMINANCE16; \
                            if (data_type==3) iformat=GL_LUMINANCE16F_ARB; \
                            if (data_type==4) iformat=GL_LUMINANCE32F_ARB; break; \
    case 2: iformat=format=GL_LUMINANCE_ALPHA; \
                            if (data_type==2) iformat=GL_LUMINANCE16_ALPHA16; \
                            if (data_type==3) iformat=GL_LUMINANCE_ALPHA16F_ARB; \
                            if (data_type==4) iformat=GL_LUMINANCE_ALPHA32F_ARB; break; \
    case 3: iformat=format=GL_RGB; \
                            if (data_type==2) iformat=GL_RGB16; \
                            if (data_type==3) iformat=GL_RGB16F_ARB; \
                            if (data_type==4) iformat=GL_RGB32F_ARB; break; \
    case 4: \
    default: iformat=format=GL_RGBA; \
                            if (data_type==2) iformat=GL_RGBA16; \
                            if (data_type==3) iformat=GL_RGBA16F_ARB; \
                            if (data_type==4) iformat=GL_RGBA32F_ARB; \
    } \
    if (compress_format)	iformat = compress_format;
#define max_size_limit  (2UL*1024*1024*1024)
#define one_MB (1024*1024)

int getMaxTexSize1D(int components, int data_type, int compress_format)
{
    format_type(components, data_type, compress_format);

    int size;
    GLint dim;
    for (size=1; size<=max_size_limit; size *=2)
    {
        glTexImage1D(GL_PROXY_TEXTURE_1D,
                0, //level
                (GLint)iformat,
                size,
                0, //border
                (GLenum)format,
                (GLenum)type,
                0); //no data
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D,
                0, //level
                GL_TEXTURE_WIDTH,
                &dim);

        if (dim != size) break; //reach the limit
    }
    size /=2;
    return size;
}

int getMaxTexSize2D(int components, int data_type, int compress_format)
{
    format_type(components, data_type, compress_format);

    int size;
    GLint dim;
    for (size=1; size<=max_size_limit; size *=2)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D,
                0, //level
                (GLint)iformat,
                size,
                size,
                0, //border
                (GLenum)format,
                (GLenum)type,
                0); //no data
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D,
                0, //level
                GL_TEXTURE_WIDTH,
                &dim);

        if (dim != size) break; //reach the limit
    }
    size /=2;
    return size;
}

int getMaxTexSize3D(int components, int data_type, int compress_format)
{
    format_type(components, data_type, compress_format);

    int size;
    GLint dim;
    for (size=1; size<=max_size_limit; size *=2)
    {
        glTexImage3DEXT(GL_PROXY_TEXTURE_3D,
                0, //level
                (GLint)iformat,
                size,
                size,
                size,
                0, //border
                (GLenum)format,
                (GLenum)type,
                0); //no data
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D,
                0, //level
                GL_TEXTURE_WIDTH,
                &dim);

        if (dim != size) break; //reach the limit
    }
    size /=2;
    return size;
}

int getMaxTexSize2X(int components, int data_type, int compress_format)
{
    int max_2d = getMaxTexSize2D(components, data_type, compress_format);

    format_type(components, data_type, compress_format);

    int size;
    GLint dim;
    for (size=1; size<=max_size_limit; size *=2)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D,
                0, //level
                (GLint)iformat,
                size,
                max_2d,
                0, //border
                (GLenum)format,
                (GLenum)type,
                0); //no data
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D,
                0, //level
                GL_TEXTURE_WIDTH,
                &dim);

        if (dim != size) break; //reach the limit
    }
    size /=2;
    return size;
}

int getMaxTexSize2Y(int components, int data_type, int compress_format)
{
    int max_2d = getMaxTexSize2D(components, data_type, compress_format);

    format_type(components, data_type, compress_format);

    int size;
    GLint dim;
    for (size=1; size<=max_size_limit; size *=2)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D,
                0, //level
                (GLint)iformat,
                max_2d,
                size,
                0, //border
                (GLenum)format,
                (GLenum)type,
                0); //no data
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D,
                0, //level
                GL_TEXTURE_HEIGHT,
                &dim);

        if (dim != size) break; //reach the limit
    }
    size /=2;
    return size;
}

int getMaxTexSize3X(int components, int data_type, int compress_format)
{
    int max_3d = getMaxTexSize3D(components, data_type, compress_format);

    format_type(components, data_type, compress_format);

    int size;
    GLint dim;
    for (size=1; size<=max_size_limit; size *=2)
    {
        glTexImage3DEXT(GL_PROXY_TEXTURE_3D,
                0, //level
                (GLint)iformat,
                size,
                size,
                max_3d,
                0, //border
                (GLenum)format,
                (GLenum)type,
                0); //no data
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D,
                0, //level
                GL_TEXTURE_WIDTH,
                &dim);

        if (dim != size) break; //reach the limit
    }
    size /=2;
    return size;
}

int getMaxTexSize3Z(int components, int data_type, int compress_format)
{
    int max_3d = getMaxTexSize3D(components, data_type, compress_format);

    format_type(components, data_type, compress_format);

    int size;
    GLint dim;
    for (size=1; size<=max_size_limit; size *=2)
    {
        glTexImage3DEXT(GL_PROXY_TEXTURE_3D,
                0, //level
                (GLint)iformat,
                max_3d,
                max_3d,
                size,
                0, //border
                (GLenum)format,
                (GLenum)type,
                0); //no data
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D,
                0, //level
                GL_TEXTURE_DEPTH,
                &dim);

        if (dim != size) break; //reach the limit
    }
    size /=2;
    return size;
}

//////////////////////////////////////////////////////////////////////////////////

int err_printf(char* format, ...)
{
    va_list args;
    va_start (args, format);
    int ret = vfprintf(stderr, format, args);
    va_end (args);
    return ret;
}

#define BS_PRINTF_BUFSIZE 1024*4
static char bs_printf_buffer[BS_PRINTF_BUFSIZE];
char* bs_printf(const char* format, ...)
{
    va_list args;
    va_start (args, format);
    vsprintf(bs_printf_buffer, format, args);
    va_end (args);
    return bs_printf_buffer;
}

void GLinfoDetect(std::string* pinfo)
{
    GLeeInit();

    std::string info;
#define PRINTF  info += bs_printf
#define ENDL    info += bs_printf("\n")

    PRINTF(" -------------------------------------------------------------------- ");ENDL;

    PRINTF("    *** current bit_size_of(   int		) = %d ", sizeof(int)*8);ENDL;
    PRINTF("    *** current bit_size_of(   long		) = %d ", sizeof(long)*8);ENDL;
    PRINTF("    *** current bit_size_of( long long	) = %d ", sizeof(long long)*8);ENDL;
    PRINTF("    *** current bit_size_of(   void* 	) = %d ", sizeof(void*)*8);ENDL;

    PRINTF(" <<GL info----------------------------------------------------------- ");ENDL;

    PRINTF("    *** GL_VERSION = %s \t(%s, %s) ", glGetString(GL_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));ENDL;
    //PRINTF("    *** GL memory  = %d MB", getGLMemorySizeMB());ENDL;

    GLint i=0,j=0;	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &j);
    PRINTF("    GL_MAX_TEXTURE_SIZE = %d,  GL_MAX_3D_TEXTURE_SIZE = %d ", i,j);ENDL;
    PRINTF("       real max texture size: \t(rgba8) \t(rgba16) \t(rgba16f) \t(rgba32f) \n"
           "                          1d: \t%9d \t%9d \t%9d \t%9d \n"
           "                          2d: \t%9d \t%9d \t%9d \t%9d \n"
           "                          *y: \t%9d \t%9d \t%9d \t%9d \n"
           "                          3d: \t%9d \t%9d \t%9d \t%9d \n"
           "                         **z: \t%9d \t%9d \t%9d \t%9d \n"
           "                         xy*: \t%9d \t%9d \t%9d \t%9d ",
        getMaxTexSize1D(4,1), getMaxTexSize1D(4,2), getMaxTexSize1D(4,3), getMaxTexSize1D(4,4),
        getMaxTexSize2D(4,1), getMaxTexSize2D(4,2), getMaxTexSize2D(4,3), getMaxTexSize2D(4,4),
        getMaxTexSize2Y(4,1), getMaxTexSize2Y(4,2), getMaxTexSize2Y(4,3), getMaxTexSize2Y(4,4),
        getMaxTexSize3D(4,1), getMaxTexSize3D(4,2),	getMaxTexSize3D(4,3), getMaxTexSize3D(4,4),
        getMaxTexSize3Z(4,1), getMaxTexSize3Z(4,2),	getMaxTexSize3Z(4,3), getMaxTexSize3Z(4,4),
        getMaxTexSize3X(4,1), getMaxTexSize3X(4,2),	getMaxTexSize3X(4,3), getMaxTexSize3X(4,4));ENDL;
    GLint ict = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    PRINTF("       compressed s3tc_dxt5 : \t(rgba8) \t(rgba16) \t(rgba16f) \t(rgba32f) \n"
           "                          1d: \t%9d \t%9d \t%9d \t%9d \n"
           "                          2d: \t%9d \t%9d \t%9d \t%9d \n"
           "                          *y: \t%9d \t%9d \t%9d \t%9d \n"
           "                          3d: \t%9d \t%9d \t%9d \t%9d \n"
           "                         **z: \t%9d \t%9d \t%9d \t%9d \n"
           "                         xy*: \t%9d \t%9d \t%9d \t%9d ",
        getMaxTexSize1D(4,1,ict), getMaxTexSize1D(4,2,ict), getMaxTexSize1D(4,3,ict), getMaxTexSize1D(4,4,ict),
        getMaxTexSize2D(4,1,ict), getMaxTexSize2D(4,2,ict), getMaxTexSize2D(4,3,ict), getMaxTexSize2D(4,4,ict),
        getMaxTexSize2Y(4,1,ict), getMaxTexSize2Y(4,2,ict), getMaxTexSize2Y(4,3,ict), getMaxTexSize2Y(4,4,ict),
        getMaxTexSize3D(4,1,ict), getMaxTexSize3D(4,2,ict),	getMaxTexSize3D(4,3,ict), getMaxTexSize3D(4,4,ict),
        getMaxTexSize3Z(4,1,ict), getMaxTexSize3Z(4,2,ict),	getMaxTexSize3Z(4,3,ict), getMaxTexSize3Z(4,4,ict),
        getMaxTexSize3X(4,1,ict), getMaxTexSize3X(4,2,ict),	getMaxTexSize3X(4,3,ict), getMaxTexSize3X(4,4,ict));ENDL;

    GLfloat f[2]={0,0};	glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, f);
    PRINTF("    GL_SMOOTH_LINE_WIDTH_RANGE = %g -- %g ", f[0], f[1]);ENDL;

    //PRINTF("    EXT_blend_logic_op = %d ", GLEE_EXT_blend_logic_op);ENDL;			//v1.1e (2.0 inadvertently included)
    PRINTF("    EXT_blend_minmax = %d ", GLEE_EXT_blend_minmax);ENDL;				//v1.1e v1.2c
    PRINTF("    EXT_blend_subtract = %d ", GLEE_EXT_blend_subtract);ENDL;			//v1.1e v1.2c
    PRINTF("    EXT_blend_color = %d ", GLEE_EXT_blend_color);ENDL;					//v1.1e v1.4c
    //PRINTF("    ARB_texture_env_combine = %d ", GLEE_ARB_texture_env_combine);ENDL;			//v1.2e v1.3c
    PRINTF("    ARB_multisample = %d ", GLEE_ARB_multisample);ENDL;					        	//v1.2e v1.3c
    PRINTF("    ARB_multitexture = %d ", GLEE_ARB_multitexture);ENDL;					       	//v1.2e v1.3c
    PRINTF("    EXT_texture3D = %d ", GLEE_EXT_texture3D);ENDL;						        	//v1.2ec (2.0)
    PRINTF("    ARB_texture_compression = %d ", GLEE_ARB_texture_compression);ENDL;	        	//v1.2e v1.3c
    PRINTF("    EXT_texture_compression_s3tc = %d ", GLEE_EXT_texture_compression_s3tc);ENDL;	//v1.2e

    PRINTF(" --------------------------------------------------------------- GL>> ");ENDL;

    PRINTF(" <<GL2 info----------------------------------------------------------- ");ENDL;

    PRINTF("    *** GL_SHADING_LANGUAGE_VERSION = %s ", glGetString(GL_SHADING_LANGUAGE_VERSION_ARB));ENDL;
    // the ARB_shading_language_100, ARB_texture_non_power_of_two, ARB_draw_buffers, are core functions of GL 2.0

    GLint maxDrawBuffers=0;	 glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &maxDrawBuffers);
    GLint maxColorAttachs=0;   glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxColorAttachs);
    GLint maxAuxBuffers=0;   glGetIntegerv(GL_AUX_BUFFERS, &maxAuxBuffers);
    GLint drawBuffer=0;	glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
    PRINTF("    GL_MAX_DRAW_BUFFERS = %d,  GL_MAX_COLOR_ATTACHMENTS = %d,  GL_AUX_BUFFERS = %d ", maxDrawBuffers, maxColorAttachs, maxAuxBuffers);ENDL;

    const char* bufferName[9] = {"GL_FRONT_LEFT", "GL_FRONT_RIGHT", "GL_BACK_LEFT", "GL_BACK_RIGHT",
                                "GL_FRONT", "GL_BACK", "GL_LEFT", "GL_RIGHT", "GL_FRONT_AND_BACK",};
    GLint iname = drawBuffer-GL_FRONT_LEFT;
    if (iname>=0 && iname<9)
        {PRINTF("       current GL_DRAW_BUFFER = %s ", bufferName[iname]);ENDL;}
    else
        {PRINTF("       current GL_DRAW_BUFFER = GL_FRONT_LEFT+%d ", iname);ENDL;}

    PRINTF("    ARB_texture_non_power_of_two = %d ", GLEE_ARB_texture_non_power_of_two);ENDL;	//v1.4e v2.0c
    PRINTF("    ARB_texture_rectangle = %d ", GLEE_ARB_texture_rectangle);ENDL;	        		//v1.5e v3.1c
    PRINTF("    EXT_framebuffer_object = %d ", GLEE_EXT_framebuffer_object);ENDL;	    		//v1.5e v3.0c
    PRINTF("    ARB_vertex_buffer_object = %d ", GLEE_ARB_vertex_buffer_object);ENDL;			//v1.4e v1.5c
    PRINTF("    ARB_pixel_buffer_object = %d ", GLEE_ARB_pixel_buffer_object);ENDL;	    		//v2.0e v2.1c

    PRINTF(" --------------------------------------------------------------- GL2>> ");ENDL;

//	///////////////////////////////////////////////////////////////////
//	// GL_AUX_BUFFERS = 0 on GeForce 8800GT, 081126
//	// The constants FRONT, BACK, LEFT, RIGHT, and FRONT_AND_BACK that refer to multiple buffers are not valid for use in DrawBuffersARB
//	//	    and will result in the error INVALID_OPERATION.
//	// If DrawBuffersARB is supplied with a constant (other than NONE) that does not indicate any of the color buffers allocated to
//	//	    the GL context, the error INVALID_OPERATION will be generated.
//	// That means the referred buffer must be created when GL context creating, otherwise will result in the error INVALID_OPERATION.
//	// But QGLFormat do NOT support AUX_BUFFER !!!
//
//	GLenum buffers[] = { GL_FRONT_LEFT, GL_FRONT_RIGHT, GL_BACK_LEFT, GL_BACK_RIGHT, GL_AUX0 };
//	GLenum buffers_left[] = { GL_BACK_LEFT, GL_FRONT_LEFT };
//	GLenum buffers_back[] = { GL_BACK_LEFT, GL_BACK_RIGHT };
//	GLenum buffers_aux[]  = { GL_BACK_LEFT, GL_AUX0 };
//
//	//glDrawBuffersARB(2, buffers_left);
//	//printf("    glDrawBuffersARB{ GL_BACK_LEFT, GL_FRONT_LEFT } %s \n", (char*)gluErrorString(glGetError()));
////	glDrawBuffersARB(2, buffers_back);
////	printf("    glDrawBuffersARB{ GL_BACK_LEFT, GL_BACK_RIGHT } %s \n", (char*)gluErrorString(glGetError()));
////	glDrawBuffersARB(2, buffers_aux);
////	printf("    glDrawBuffersARB{ GL_BACK_LEFT, GL_AUX0 } %s \n", (char*)gluErrorString(glGetError()));

    if (pinfo)
        *pinfo = info;
    else
//		cerr << (info); //cerr is unbuffered
        v3d_msg("before fprintf(stderr, info.c_str());");
    fprintf(stderr, info.c_str());
}



//////////////////////////////////////////////////////////////////////////////////
