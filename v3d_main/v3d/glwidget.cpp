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






#include <QtGui>
#include <QtOpenGL>

#include <math.h>

#include "glwidget.h"

//#include "cellanotablemodel.h"

V3D_Base_GLWidget::V3D_Base_GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    gear1 = 0;
    gear2 = 0;
    gear3 = 0;

    //
    ball1 = 0;
    ball2 = 0;
    ball3 = 0;
	
    xRot = 0;
    yRot = 0;
    zRot = 0;
    gear1Rot = 0;

//    QTimer *animate_timer = new QTimer(this);
//    connect(animate_timer, SIGNAL(timeout()), this, SLOT(advanceGears()));
//    animate_timer->start(20);
}

V3D_Base_GLWidget::~V3D_Base_GLWidget()
{
    makeCurrent();
    glDeleteLists(gear1, 1);
    glDeleteLists(gear2, 1);
    glDeleteLists(gear3, 1);

    //
    glDeleteLists(ball1, 1);
    glDeleteLists(ball2, 1);
    glDeleteLists(ball3, 1);

}

#if 0
void V3D_Base_GLWidget::readAnoFile(QString file_anoText)
{
	printf("anofile=[%s]\n", qPrintable(file_anoText));
	
	QFile file(file_anoText);
	
	if(!file.open(QIODevice::ReadOnly))
	{
	  printf("*** Have problem to open the file [%s] ***\n", qPrintable(file_anoText)); 
	  return false;
	}  
	
	//if (anoData) {delete anoData; anoData=NULL;}
	if (anoTable) {delete anoTable; anoTable=NULL;}
	QString tmp = file.readAll();
	//printf("{%s}", qPrintable(tmp));
	
	//anoData = new TreeModel(tmp);
	anoTable = new CellAnoTableModel(tmp);
	file.close();
	
	if (!anoTable)
		return false;
	else
		return true;
}
#endif

void V3D_Base_GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void V3D_Base_GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void V3D_Base_GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void V3D_Base_GLWidget::initializeGL()
{
    static const GLfloat lightPos[4] = { 5.0f, 5.0f, 10.0f, 1.0f };
//    static const GLfloat reflectance1[4] = { 0.8f, 0.1f, 0.0f, 1.0f };
    static const GLfloat reflectance1[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
    static const GLfloat reflectance2[4] = { 0.0f, 0.8f, 0.2f, 1.0f };
    static const GLfloat reflectance3[4] = { 0.2f, 0.2f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    gear1 = makeGear(reflectance1, 1.0, 4.0, 1.0, 0.7, 20);
    gear2 = makeGear(reflectance2, 0.5, 2.0, 2.0, 0.7, 10);
    gear3 = makeGear(reflectance3, 1.3, 2.0, 0.5, 0.7, 10);

    //
    ball1 = makeSphere(reflectance1, 1.0);
    ball2 = makeSphere(reflectance2, 1.5);
    ball3 = makeSphere(reflectance3, 0.5);

    glEnable(GL_NORMALIZE);
}

void V3D_Base_GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);

/*
    drawGear(gear1, -3.0, -2.0, 0.0, gear1Rot / 16.0);
    drawGear(gear2, +3.1, -2.0, 0.0, -2.0 * (gear1Rot / 16.0) - 9.0);

    glRotated(+90.0, 1.0, 0.0, 0.0);
    drawGear(gear3, -3.1, -1.8, -2.2, +2.0 * (gear1Rot / 16.0) - 2.0);
*/
    //
    drawSphere(ball1, -4.0, -2.0, 0.0, gear1Rot / 16.0);
    drawSphere(ball2, +4.1, -2.0, 0.0, -2.0 * (gear1Rot / 16.0) - 9.0);

    glRotated(+90.0, 1.0, 0.0, 0.0);
    drawSphere(ball3, -4.1, -1.8, -2.2, +2.0 * (gear1Rot / 16.0) - 2.0);


    glPopMatrix();
}

void V3D_Base_GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, +1.0, -1.0, 1.0, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -40.0);
}

void V3D_Base_GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void V3D_Base_GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}

void V3D_Base_GLWidget::advanceGears()
{
    gear1Rot += 2 * 16;
    updateGL();
}

GLuint V3D_Base_GLWidget::makeGear(const GLfloat *reflectance, GLdouble innerRadius,
                          GLdouble outerRadius, GLdouble thickness,
                          GLdouble toothSize, GLint toothCount)
{
    const double Pi = 3.14159265358979323846;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, reflectance);

    GLdouble r0 = innerRadius;
    GLdouble r1 = outerRadius - toothSize / 2.0;
    GLdouble r2 = outerRadius + toothSize / 2.0;
    GLdouble delta = (2.0 * Pi / toothCount) / 4.0;
    GLdouble z = thickness / 2.0;
    int i, j;

    glShadeModel(GL_FLAT);

    for (i = 0; i < 2; ++i) {
        GLdouble sign = (i == 0) ? +1.0 : -1.0;

        glNormal3d(0.0, 0.0, sign);

        glBegin(GL_QUAD_STRIP);
        for (j = 0; j <= toothCount; ++j) {
            GLdouble angle = 2.0 * Pi * j / toothCount;
	    glVertex3d(r0 * cos(angle), r0 * sin(angle), sign * z);
	    glVertex3d(r1 * cos(angle), r1 * sin(angle), sign * z);
	    glVertex3d(r0 * cos(angle), r0 * sin(angle), sign * z);
	    glVertex3d(r1 * cos(angle + 3 * delta), r1 * sin(angle + 3 * delta),
                       sign * z);
        }
        glEnd();

        glBegin(GL_QUADS);
        for (j = 0; j < toothCount; ++j) {
            GLdouble angle = 2.0 * Pi * j / toothCount;
	    glVertex3d(r1 * cos(angle), r1 * sin(angle), sign * z);
	    glVertex3d(r2 * cos(angle + delta), r2 * sin(angle + delta),
                       sign * z);
	    glVertex3d(r2 * cos(angle + 2 * delta), r2 * sin(angle + 2 * delta),
                       sign * z);
	    glVertex3d(r1 * cos(angle + 3 * delta), r1 * sin(angle + 3 * delta),
                       sign * z);
        }
        glEnd();
    }

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < toothCount; ++i) {
        for (j = 0; j < 2; ++j) {
            GLdouble angle = 2.0 * Pi * (i + (j / 2.0)) / toothCount;
            GLdouble s1 = r1;
            GLdouble s2 = r2;
            if (j == 1)
                qSwap(s1, s2);

	    glNormal3d(cos(angle), sin(angle), 0.0);
	    glVertex3d(s1 * cos(angle), s1 * sin(angle), +z);
	    glVertex3d(s1 * cos(angle), s1 * sin(angle), -z);

	    glNormal3d(s2 * sin(angle + delta) - s1 * sin(angle),
                       s1 * cos(angle) - s2 * cos(angle + delta), 0.0);
	    glVertex3d(s2 * cos(angle + delta), s2 * sin(angle + delta), +z);
	    glVertex3d(s2 * cos(angle + delta), s2 * sin(angle + delta), -z);
        }
    }
    glVertex3d(r1, 0.0, +z);
    glVertex3d(r1, 0.0, -z);
    glEnd();

    glShadeModel(GL_SMOOTH);

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= toothCount; ++i) {
	GLdouble angle = i * 2.0 * Pi / toothCount;
	glNormal3d(-cos(angle), -sin(angle), 0.0);
	glVertex3d(r0 * cos(angle), r0 * sin(angle), +z);
	glVertex3d(r0 * cos(angle), r0 * sin(angle), -z);
    }
    glEnd();

    glEndList();

    return list;
}

void V3D_Base_GLWidget::drawGear(GLuint gear, GLdouble dx, GLdouble dy, GLdouble dz,
                        GLdouble angle)
{
    glPushMatrix();
    glTranslated(dx, dy, dz);
    glRotated(angle, 0.0, 0.0, 1.0);
    glCallList(gear);
    glPopMatrix();
}

void V3D_Base_GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}

GLuint V3D_Base_GLWidget::makeSphere(const GLfloat *reflectance, GLdouble radius)
{
//    const double Pi = 3.14159265358979323846;
//    int i, j;
    GLint slices=16;
	GLint stacks=16;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, reflectance);

    glShadeModel(GL_FLAT);

    GLUquadric * quad = gluNewQuadric();
    gluSphere(quad, radius, slices, stacks);
					
    glShadeModel(GL_SMOOTH);

    glEndList();

    return list;
}

void V3D_Base_GLWidget::drawSphere(GLuint gear, GLdouble dx, GLdouble dy, GLdouble dz, GLdouble angle)
{
    glPushMatrix();
    glTranslated(dx, dy, dz);
    glRotated(angle, 0.0, 0.0, 1.0);
    glCallList(gear);
    glPopMatrix();
}
