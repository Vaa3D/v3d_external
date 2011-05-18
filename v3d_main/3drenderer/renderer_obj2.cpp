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




/*
 * renderer_obj2.cpp
 *
 *  Created on: Aug 29, 2008
 *      Author: ruanzongcai
 * Last update: 090220: by Hanchuan Peng, add neuron coordinate manipulation functions
 * Last update: 090331, by Hanchuan Peng, extend several more colors for neuron display
 * Last update: 090716: by Hanchuan Peng, now use the apo file loader writer in the basic_c_fun directory
 * Last update: by PHC, 2010-06-02, separate the content of function copyToEditableNeuron() to NeuronTree__2__V_NeuronSWC_list()
 */

#include "renderer_tex2.h"
#include "v3dr_glwidget.h"

#include "freeglut_geometry_r.c"
#define CALL_glutSolidTorus glutSolidTorus
#define CALL_glutSolidDode  glutSolidDodecahedron


// if error then just warning
// clean memory before MessageBox, otherwise MessageBox maybe could not be created correctly
#define ERROR_MessageBox(title, type, what) { \
	cleanObj(); \
	QMessageBox::critical( 0, title, QObject::tr("%1: OUT OF MEMORY or FILE IO ERROR.\n---%2 exception: %3")\
			.arg(title).arg(type).arg(what) + "\n\n" + \
		QObject::tr("3D View: Please close some images or views to release memory, then try again.\n\n") ); \
}

#define CATCH_handler( func_name ) \
	catch (std::exception& e) { \
		\
		qDebug("    *** std exception occurred in "func_name); \
		ERROR_MessageBox(func_name, "std", e.what()); \
		\
	} catch (const char* str) { \
		\
		qDebug("    *** IO exception occurred in "func_name); \
		ERROR_MessageBox(func_name, "IO", str); \
		\
	} catch (...) { \
		\
		ERROR_MessageBox( func_name, "UNKOWN", "unknown exception" ); \
		\
	} \
	b_error = false; // clear b_error to continue running



////////////////////////////////////////////////////////////////////


void Renderer_tex2::loadObjectFromFile(const char* url)
{
	qDebug("   Renderer_tex2::loadObjectFromFile (url)");

	QString filename;
	if (url)
		filename = QString(url);
	else
	    filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
	    		"",
	    		QObject::tr("Supported file (*.swc *.apo *.raw *.tif *.tiff *.v3ds *.obj *.marker *.csv)"
	    				";;Neuron structure	(*.swc)"
	    				";;Point Cloud		(*.apo)"
	    				";;Label field		(*.raw *.tif *.tiff)"
	    				";;Label Surface	(*.v3ds *.obj)"
						";;Landmarks		(*.marker *.csv)"
	    				));
    qDebug()<< "open file: " << filename;

	if (filename.size()>0)
		loadObjectFilename(filename);
}

void Renderer_tex2::loadObjectListFromFile()
{
	qDebug("   Renderer_tex2::loadObjectListFromFile");

	QStringList qsl;
	qsl.clear();

#ifndef test_main_cpp
	if (_idep==0) return;
	iDrawExternalParameter* ep = (iDrawExternalParameter*) _idep;
	//Q_ASSERT( ep->image4d==0 );

//	for (int i=0;i<ep->swc_file_list.size();i++)		qsl.append(ep->swc_file_list[i]);
//	for (int i=0;i<ep->pointcloud_file_list.size();i++)		qsl.append(ep->pointcloud_file_list[i]);
//	qsl.append(ep->surface_file);
//	qsl.append(ep->labelfield_file);
	qsl << ep->swc_file_list;
	qsl << ep->pointcloud_file_list;
	qsl << ep->surface_file;
	qsl << ep->labelfield_file;
#endif

	((QWidget*)widget)->hide(); //101024 to avoid busy updateGL
	foreach (QString filename, qsl)
	{
	    if (filename.size()>0)
	    	loadObjectFilename(filename);
	}
	((QWidget*)widget)->show();
}

void Renderer_tex2::loadObjectFilename(const QString& filename)
{
	makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105, 081122

	int type = 0;
	try {

		// if create labelfield
		if (filename.endsWith(".tif", Qt::CaseInsensitive) || filename.endsWith(".tiff", Qt::CaseInsensitive)
				|| filename.endsWith(".raw", Qt::CaseInsensitive))
		{
			loadLabelfieldSurf(filename);
		}
		// if label surface obj
		else if (filename.endsWith(".obj", Qt::CaseInsensitive))
		{
			type = stLabelSurface;
			loadWavefrontOBJ(filename);
		}
		// if label surface v3ds --binary format obj
		else if (filename.endsWith(".v3ds", Qt::CaseInsensitive))
		{
			type = stLabelSurface;
			loadV3DSurface(filename);
		}
		else if (filename.endsWith(".marker", Qt::CaseInsensitive) || filename.endsWith(".csv", Qt::CaseInsensitive))
		{
			type = stImageMarker;
			loadLandmarks_from_file(filename);
		}
		// if swc
		else if (filename.endsWith(".swc", Qt::CaseInsensitive))
		{
			type = stNeuronStructure;
			loadNeuronTree(filename);
		}
		// if apo
		else if (filename.endsWith(".apo", Qt::CaseInsensitive))
		{
			type = stPointCloud;
			loadCellAPO(filename);
		}

	} CATCH_handler( "Renderer_tex2::loadObjectFilename" );

    updateBoundingBox(); ///// 081121, all of loaded bounding-box are updated here

	if (widget) //090522 RZC
	{
		((V3dR_GLWidget*)widget)->surfaceSelectTab(type-1);
	}
}

void Renderer_tex2::saveSurfFile()
{
	qDebug("   Renderer_tex2::saveSurfFile");

	if (list_listTriangle.size()==0)
	{
		QMessageBox::information(0, QObject::tr("save file"), QObject::tr("NO surface to save!"));
		return;
	}

	extern QString lf_data_title;
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save Surface File"),
    		lf_data_title+".v3ds",
    		QObject::tr("V3D Surface Object (*.v3ds)"
    				";;Wavefront Object (*.obj)"
    				));
    qDebug()<< "save file: " << filename;

    try {

		// if obj
		if (filename.endsWith(".obj", Qt::CaseInsensitive))
		{
			saveWavefrontOBJ(filename);
		}
		// if v3ds
		if (filename.endsWith(".v3ds", Qt::CaseInsensitive))
		{
			saveV3DSurface(filename);
		}

    } CATCH_handler( "Renderer_tex2::saveSurfFile" );

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Renderer_tex2::loadObj()
{
	cleanObj(); //070905
	qDebug("  Renderer_tex2::loadObj");
	makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105

	createMarker_atom(); 		//qDebug("	 glGetError(createMarker_atom) = %u", glGetError());
	createNeuron_tube();     //qDebug("	 glGetError(createNeuron_tube) = %u", glGetError());
	createCell_atom();     		//qDebug("	 glGetError(createCell_atom) = %u", glGetError());

	CHECK_GLErrorString_throw();
}

void Renderer_tex2::cleanObj()
{
	qDebug("   Renderer_tex2::cleanObj");
	makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105

	// marker
	for (int i=0; i<NTYPE_MARKER; i++)
	{
		glDeleteLists(glistMarker[i], 1);
		glistMarker[i] = 0;
	}
	listMarker.clear();

	// cell
	glDeleteLists(glistCell, 1);
	glistCell = 0;
	listCell.clear();
	map_APOFile_IndexList.clear();
	map_CellIndex_APOFile.clear();

	// neuron
	glDeleteLists(glistTube, 1);
	glDeleteLists(glistTubeEnd, 1);
	for (int i=0; i<listNeuronTree.size(); i++)
	{
		listNeuronTree[i].listNeuron.clear();
		listNeuronTree[i].hashNeuron.clear();
	}
	listNeuronTree.clear();
	glistTube=glistTubeEnd = 0;

	// label field
	cleanLabelfieldSurf();
}

void Renderer_tex2::updateBoundingBox()
{
	BoundingBox& sBB =surfBoundingBox;
	BoundingBox& BB  =boundingBox;

	sBB = NULL_BoundingBox;		//qDebug("	 BoundingBox (%g %g %g)--(%g %g %g)", sBB.x0,sBB.y0,sBB.z0, sBB.x1,sBB.y1,sBB.z1 );
	sBB.expand(swcBB);			//qDebug("	 BoundingBox (%g %g %g)--(%g %g %g)", sBB.x0,sBB.y0,sBB.z0, sBB.x1,sBB.y1,sBB.z1 );
	sBB.expand(apoBB);			//qDebug("	 BoundingBox (%g %g %g)--(%g %g %g)", sBB.x0,sBB.y0,sBB.z0, sBB.x1,sBB.y1,sBB.z1 );
	sBB.expand(labelBB);		//qDebug("	 BoundingBox (%g %g %g)--(%g %g %g)", sBB.x0,sBB.y0,sBB.z0, sBB.x1,sBB.y1,sBB.z1 );

	if (!has_image())// only surface object
	{
		//qDebug("	!have_image");
		boundingBox = surfBoundingBox; //081031
	}
	qDebug("  Renderer_tex2::updateBoundingBox surface (%g %g %g)--(%g %g %g)", sBB.x0,sBB.y0,sBB.z0, sBB.x1,sBB.y1,sBB.z1 );
	qDebug("  Renderer_tex2::updateBoundingBox default (%g %g %g)--(%g %g %g)", BB.x0,BB.y0,BB.z0, BB.x1,BB.y1,BB.z1 );

	updateThicknessBox(); //090806
}

void Renderer_tex2::setThickness(double t)
{
	//qDebug("  Renderer_tex2::setThickness");
	thicknessZ = t;
	updateThicknessBox();
}

void Renderer_tex2::updateThicknessBox()
{
	if (has_image())
	{
		BoundingBox& BB = boundingBox;
		BB.x0 = 0;
		BB.y0 = 0;
		BB.z0 = 0;
		BB.x1 = imageX / sampleScaleX * thicknessX;
		BB.y1 = imageY / sampleScaleY * thicknessY;
		BB.z1 = imageZ / sampleScaleZ * thicknessZ;
		//qDebug("	 BoundingBox (%g %g %g)--(%g %g %g)", BB.x0,BB.y0,BB.z0, BB.x1,BB.y1,BB.z1 );
	}
	else // no image
	{
		BoundingBox& BB = boundingBox;
		BoundingBox& sBB = surfBoundingBox;
		BB.x0 = sBB.x0 * thicknessX;
		BB.y0 = sBB.y0 * thicknessY;
		BB.z0 = sBB.z0 * thicknessZ;
		BB.x1 = sBB.x1 * thicknessX;
		BB.y1 = sBB.y1 * thicknessY;
		BB.z1 = sBB.z1 * thicknessZ;
		//qDebug("	 BoundingBox (%g %g %g)--(%g %g %g)", BB.x0,BB.y0,BB.z0, BB.x1,BB.y1,BB.z1 );
	}
}

void Renderer_tex2::setMarkerSpace()
{
	Renderer::setObjectSpace(); //// object put in original image space, 090715 off
	glTranslated(-start1,-start2,-start3); //090715
	glScaled(thicknessX, thicknessY, thicknessZ);
}
void Renderer_tex2::drawMarker()
{
	glPushName(stImageMarker);
		drawMarkerList();
	glPopName();
}

void Renderer_tex2::setSurfaceStretchSpace()
{
	Renderer::setObjectSpace(); //// object put in original image space, 090715 off
	glTranslated(-start1,-start2,-start3); //090715
	if (//have_image() &&
			b_surfStretch)  // 090423 RZC: stretch surface object with image thickness
	{
		glScaled(thicknessX, thicknessY, thicknessZ);
	}
}
void Renderer_tex2::drawObj()
{
//	if (sShowMarkers==2 || sShowSurfObjects==2) // draw float over volume
//	{
//		// save depth buffer
//		// glReadPixels(0,0,screenW,screenH, GL_DEPTH_COMPONENT, GL_FLOAT, depth_buffer); //very very slowly!!!
//		// using fast stencil, by RZC 080902
//	}

	if (b_useClipBoxforSubjectObjs)  enableClipBoundingBox(surfBoundingBox, true, 0.001); //081031,081231, 090503 RZC


	glPushName(stLabelSurface);  //081222: when no image data, label surface became the background of float draw.
		drawLabelfieldSurf();
	glPopName();

	glPushName(stNeuronStructure);
		drawNeuronTreeList();
	glPopName();

	glPushName(stPointCloud);
		drawCellList();
	glPopName();


	if (b_useClipBoxforSubjectObjs)  disableClipBoundingBox(); // surface clip do not include markers, and labelText
}

#define IS_TRANSPARENT  (polygonMode>=3 && !b_selecting)

void Renderer_tex2::disObjLighting()
{
	glDisable( GL_COLOR_MATERIAL );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );
	glDisable( GL_CULL_FACE );

	glDisable(GL_ALPHA_TEST);
	if (IS_TRANSPARENT)
	{
		glDisable(GL_BLEND); //090429 RZC: no effect to glBlendEquationEXT(GL_MAX_EXT), must set to GL_FUNC_ADD_EXT
		glBlendEquationEXT(GL_FUNC_ADD_EXT);
		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LESS);//more artifacts ???
	}
}
void Renderer_tex2::setObjLighting()
{
	glEnable(GL_ALPHA_TEST);  glAlphaFunc(GL_GREATER, 0); //for outline mode
	if (IS_TRANSPARENT)
	{
		glEnable(GL_BLEND); //090429 RZC: no effect to glBlendEquationEXT(GL_MAX_EXT), must set to GL_FUNC_ADD_EXT
		glBlendEquationEXT(GL_FUNC_ADD_EXT);
		//glBlendColorEXT(1, 1, 1, 1-CSbeta);
		glBlendColorEXT(1, 1, 1, 0.2);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA); // constant Alpha
		glDisable(GL_DEPTH_TEST);
		//glDepthFunc(GL_ALWAYS);//more artifacts ???
	}

	glPushMatrix();
    glLoadIdentity(); // set light in camera coordinate ///(x-right,y-up,z-out)///
    {
		XYZW light1_ambient  = XYZW( .2, .2, .2, 1 );
		XYZW light1_diffuse  = XYZW( .8, .8, .8, 1 );
		XYZW light1_specular = XYZW( .9, .9, .9, 1 );
		int i;
		float s;

		i = GL_LIGHT1;
		s = 0.4;
		glLightfv( i, GL_AMBIENT,  (s*light1_ambient).v);
		glLightfv( i, GL_DIFFUSE,  (s*light1_diffuse).v);
		glLightfv( i, GL_SPECULAR, (0 *light1_specular).v); //specular off
		///////////////////////// from-eye-forward, default direction

		i = GL_LIGHT0;
		s = 0.8;
		glLightfv( i, GL_AMBIENT,  (s*light1_ambient).v);
		glLightfv( i, GL_DIFFUSE,  (s*light1_diffuse).v);
		glLightfv( i, GL_SPECULAR, (s*light1_specular).v);
		//glLightf( i, GL_SPOT_CUTOFF, 90.0);
		//glLightf( i, GL_SPOT_EXPONENT, 20.0);
		XYZW light1_position  = XYZW(  1,  1, -1,  0 );  ////////////// right-up-backward
		XYZW light1_direction = XYZW( -1, -1,  1,  0 );
		glLightfv( i, GL_POSITION, light1_position.v);
		glLightfv( i, GL_SPOT_DIRECTION, light1_direction.v);

		i = GL_LIGHT2;
		s = 1.0;
		glLightfv( i, GL_AMBIENT,  (s*light1_ambient).v);
		glLightfv( i, GL_DIFFUSE,  (s*light1_diffuse).v);
		glLightfv( i, GL_SPECULAR, (s*light1_specular).v);
		XYZW light2_position  = XYZW( -1,  0,   1,  0 ); ////////////// left-level-forward
		XYZW light2_direction = XYZW(  1,  0,  -1,  0 );
		glLightfv( i, GL_POSITION, light2_position.v);
		glLightfv( i, GL_SPOT_DIRECTION, light2_direction.v);

		XYZW mater_no_emission = XYZW(  0, 0, 0, 1 );
		//XYZW mater_emission    = XYZW(  0.2, 0.2, 0.2, 1 ); // emission used for selected indicator
		XYZW mater_no_ambient  = XYZW(  0, 0, 0, 1 );
		XYZW mater_ambient     = XYZW(  .1, .1, .1, 1 );    // No ambient used for cell intensity
		XYZW mater_specular    = XYZW(  1, 1, 1, 1 );
		//XYZW mater_specular    = XYZW(  .8, .8, .8, 1 );
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mater_no_emission.v);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mater_ambient.v);
		glMaterialfv(GL_FRONT,    GL_SPECULAR, mater_specular.v);
		glMaterialf(GL_FRONT,     GL_SHININESS, 40.0);  //default OpenGL does not permit a shininess or spot exponent over 128

		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0); //081220, MUST set 0
		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE); // important for lighting when using scale transformation
		glDisable(GL_CULL_FACE); // because surface cut
    }
    glPopMatrix();
}

void Renderer_tex2::beginHighlight()
{
	XYZW mater_emission = XYZW( .3, .3, .3,  1 );
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mater_emission.v);
}
void Renderer_tex2::endHighlight()
{
	XYZW mater_no_emission = XYZW( 0, 0, 0,  1 );
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mater_no_emission.v);
}


///////////////////////////////////////////////////////

static GLuint _createOcta() //8-face sphere
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	{
		GLUquadric* Q = gluNewQuadric();
		gluQuadricOrientation( Q, GLU_OUTSIDE);
		gluQuadricNormals(Q, GLU_FLAT);

		gluSphere( Q, .55,  4, 2);

		gluDeleteQuadric(Q);
	}
	glEndList();
	return g;
}

static GLuint _createDiamond()
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	{
		GLUquadric* Q = gluNewQuadric();
		gluQuadricOrientation( Q, GLU_OUTSIDE);
		gluQuadricNormals(Q, GLU_FLAT);

		gluSphere( Q, .55,  8, 4);

		gluDeleteQuadric(Q);
	}
	glEndList();
	return g;
}

static GLuint _createSphere(int m=36)
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	{
		GLUquadric* Q = gluNewQuadric();
		gluQuadricOrientation( Q, GLU_OUTSIDE);
		gluQuadricNormals(Q, GLU_SMOOTH);

		gluSphere( Q, .5,  m, m/2+1);

		gluDeleteQuadric(Q);
	}
	glEndList();
	return g;
}

static GLuint _createCylinder(int m=36, int n=1)
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	glPushMatrix();
	{
		GLUquadric* Q = gluNewQuadric();
		gluQuadricOrientation( Q, GLU_OUTSIDE);
		gluQuadricNormals(Q, GLU_SMOOTH);

		gluCylinder( Q, .5, .5,  +0.5,  m, n);
		glScalef( 1, 1, -1); // flip z, origin at center
		gluCylinder( Q, .5, .5,  +0.5,  m, n);

		gluDeleteQuadric(Q);
	}
	glPopMatrix();
	glEndList();
	return g;
}

static GLuint _createCone(int m=36, int n=1)
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	{
		GLUquadric* Q = gluNewQuadric();
		gluQuadricOrientation( Q, GLU_OUTSIDE);
		gluQuadricNormals(Q, GLU_SMOOTH);

		gluCylinder( Q, 0, 0.5,  +1,  m, n);  // origin at tip

		gluDeleteQuadric(Q);
	}
	glEndList();
	return g;
}

static GLuint _createDisk(int m=36, int n=1)
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	{
		GLUquadric* Q = gluNewQuadric();
		gluQuadricOrientation( Q, GLU_OUTSIDE);
		gluQuadricNormals(Q, GLU_SMOOTH);

		gluDisk( Q, .2, .5,  m, n);

		gluDeleteQuadric(Q);
	}
	glEndList();
	return g;
}

static GLuint _createDualCone(int m=36, int n=1)
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	glPushMatrix();
	{
		GLUquadric* Q = gluNewQuadric();
		gluQuadricOrientation( Q, GLU_OUTSIDE);
		gluQuadricNormals(Q, GLU_SMOOTH);

		gluCylinder( Q, 0.5, 0, +0.5,  m, n);
		glScalef( 1, 1, -1); // flip z, origin at center
		gluCylinder( Q, 0.5, 0, +0.5,  m, n);

		gluDeleteQuadric(Q);
	}
	glPopMatrix();
	glEndList();
	return g;
}

static GLuint _createCube()
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	glPushMatrix();
	glScalef(.7f, .7f, .7f); // ~ sqrt(2)/2
	glTranslatef(-.5f, -.5f, -.5f);
	glBegin(GL_QUADS);

	//yx0
	//glColor3f(.7f, .7f, .9f);
	glNormal3f(0, 0, -1);	glVertex3f(0, 0, 0);	glVertex3f(0, 1, 0);	glVertex3f(1, 1, 0);	glVertex3f(1, 0, 0);
	//x0z
	//glColor3f(.7f, .9f, .7f);
	glNormal3f(0, -1, 0);	glVertex3f(0, 0, 0);	glVertex3f(1, 0, 0);	glVertex3f(1, 0, 1);	glVertex3f(0, 0, 1);
	//0zy
	//glColor3f(.9f, .7f, .7f);
	glNormal3f(-1, 0, 0);	glVertex3f(0, 0, 0);	glVertex3f(0, 0, 1);	glVertex3f(0, 1, 1);	glVertex3f(0, 1, 0);
	//xy1
	//glColor3f(.9f, .9f, .5f);
	glNormal3f(0, 0, +1);	glVertex3f(0, 0, 1);	glVertex3f(1, 0, 1);	glVertex3f(1, 1, 1);	glVertex3f(0, 1, 1);
	//z1x
	//glColor3f(.9f, .5f, .9f);
	glNormal3f(0, +1, 0);	glVertex3f(0, 1, 0);	glVertex3f(0, 1, 1);	glVertex3f(1, 1, 1);	glVertex3f(1, 1, 0);
	//1yz
	//glColor3f(.5f, .9f, .9f);
	glNormal3f(+1, 0, 0);	glVertex3f(1, 0, 0);	glVertex3f(1, 1, 0);	glVertex3f(1, 1, 1);	glVertex3f(1, 0, 1);

	glEnd();
	glPopMatrix();
	glEndList();
	return g;
}

#ifdef CALL_glutSolidTorus
static GLuint _createRing(int m=36, int n=36)
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	{
		CALL_glutSolidTorus( .2, (.55-.2),  m, n);
	}
	glEndList();
	return g;
}
#endif

#ifdef CALL_glutSolidDode
static GLuint _createDode()
{
	GLuint g = glGenLists(1);
	glNewList(g, GL_COMPILE);
	glPushMatrix();
	{
		glScalef(.3f, .3f, .3); //
		CALL_glutSolidDode();
	}
	glPopMatrix();
	glEndList();
	return g;
}
#endif


#define __image_marker__

void Renderer_tex2::createMarker_atom()
{
	//	{pxUnknown, pxLocaNotUseful, pxLocaUseful, pxLocaUnsure, pxTemp};

	for (int i=0; i<NTYPE_MARKER; i++)
		marker_color[i] = random_rgba8(255);

	int i=0;

	//0
#ifdef CALL_glutSolidDode
	if (i<NTYPE_MARKER) glistMarker[i++] = _createDode(); // soccer
#else
	if (i<NTYPE_MARKER) glistMarker[i++] = _createDiamond();
#endif

	//1
	if (i<NTYPE_MARKER) glistMarker[i++] = _createCube();
	//2
	if (i<NTYPE_MARKER) glistMarker[i++] = _createSphere();

	//3
#ifdef CALL_glutSolidTorus
	if (i<NTYPE_MARKER) glistMarker[i++] = _createRing();
#else
	if (i<NTYPE_MARKER) glistMarker[i++] = _createCylinder();
#endif

	//4
	if (i<NTYPE_MARKER) glistMarker[i++] = _createDualCone();

}


///////////////////////////////////////////////////////

void Renderer_tex2::updateLandmark()
{
	//qDebug("  Renderer_tex2::updateLandmark");

#ifndef test_main_cpp
	My4DImage* image4d = v3dr_getImage4d(_idep);
	if (image4d)
	{
		QList <LocationSimple> listLoc = image4d->listLandmarks;
		qDebug("\t number of markers = %d", listLoc.size());

		listMarker.clear();
		for (int i=0; i<listLoc.size(); i++)
		{
			ImageMarker S;
	        //memset(&S, 0, sizeof(S)); //this will make QString member to crash . 090219 by PHC.

			S.n = i;
			S.type = listLoc[i].inputProperty;
			S.shape = listLoc[i].shape;
			S.x = listLoc[i].x;
			S.y = listLoc[i].y;
			S.z = listLoc[i].z;
			S.color = listLoc[i].color;  //random_rgba8(255);
			S.on = true; //listLoc[i].on;        //090713 RZC: the state synchronization is hard
			S.selected = false; //added because the memset() is got commented
			//S.pn = 0; //added because the memset() is got commented

			S.name = listLoc[i].name.c_str();
			S.comment = listLoc[i].comments.c_str();

			listMarker.append(S);
		}
	}
#endif
}

void Renderer_tex2::drawMarkerList()
{
	//qDebug("    Renderer_tex2::drawMarkerList");
	if (sShowMarkers==0) return;

	float maxD = boundingBox.Dmax();
				//MAX(dim1,MAX(dim2,dim3));//090726: this will be 0 when no image
	float marker_size = //markerSize; // 090423 RZC
					maxD * markerSize/1000.f;

	for (int pass=0; pass<numPassFloatDraw(sShowMarkers); pass++)
	{
		setFloatDrawOp(pass, sShowMarkers);

		for (int i=0; i<listMarker.size(); i++)
		{
			const ImageMarker& S = listMarker[i];
			if (! S.on)	continue;
			if (S.selected) HIGHLIGHT_ON();

			int type = //i % NTYPE_MARKER;
						//S.type % NTYPE_MARKER;
						(S.type<0 || S.type>= NTYPE_MARKER)? 0 : S.type; //081230
			RGBA8 color = //marker_color[type];
						S.color;
			glColor3ubv(color.c);

			glPushMatrix();
			glTranslated(S.x-1, S.y-1, S.z-1); // 090505 RZC : marker position is 1-based
			//glScaled(marker_size, marker_size, marker_size);
			glScaled(marker_size/thicknessX, marker_size/thicknessY, marker_size/thicknessZ); // 090421 RZC: shape adjusted with image thickness

			glPushName(1+i);
				glCallList(glistMarker[type]);
			glPopName();

			glPopMatrix();

			if (S.selected) HIGHLIGHT_OFF();
		}
	}
	setFloatDrawOp(-1, sShowMarkers);

	// marker label
	// qDebug("b_showMarkerLabel = %i", (b_showMarkerLabel));
	// qDebug("widget = 0x%p", widget);
	if (b_showMarkerLabel && !b_showMarkerName)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND); // no effect to glBlendEquationEXT(GL_MAX_EXT)
		glDisable(GL_LIGHTING);
		disableClipBoundingBox(); //090726

		RGBA32f c = XYZW(1) - color_background;
					//color_line;
		glColor4f(c.r, c.g, c.b, 1);

		for (int i=0; i<listMarker.size(); i++)
		{
			const ImageMarker& S = listMarker[i];
			if (! S.on)	continue;

			glPushMatrix();
			glTranslated(S.x-1, S.y-1, S.z-1); // 090505 RZC : marker position is 1-based

			((QGLWidget*)widget)->renderText(0., 0., 0., QString("%1").arg(i+1));
			//char sbuf[20];	sprintf(sbuf, "%d", i+1);	drawString(0, 0, 0, sbuf);

			glPopMatrix();
		}

		glPopAttrib();
	}

	// toggle marker name. by Lei Qu, 110425
	// revised again by Hanchuan Peng, 110426
	if (b_showMarkerName)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND); // no effect to glBlendEquationEXT(GL_MAX_EXT)
		glDisable(GL_LIGHTING);
		disableClipBoundingBox(); //090726

		RGBA32f c = XYZW(1) - color_background;
					//color_line;
		glColor4f(c.r, c.g, c.b, 1);

		for (int i=0; i<listMarker.size(); i++)
		{
			const ImageMarker& S = listMarker[i];
			if (! S.on)	continue;

			glPushMatrix();
			glTranslated(S.x-1, S.y-1, S.z-1);

			QFont font;
			font.setPointSize(10);

			QString mystr = S.name.trimmed();
			if (b_showMarkerLabel)
			{
				if (S.name.size()>0)
					mystr = QString("%1").arg(i+1) + " : " + mystr;
				else {
					mystr = QString("%1").arg(i+1);
				}
			}
			
			((QGLWidget*)widget)->renderText(0., 0., 0., (mystr)); //do not use font for now. by PHC, 110426
			//((QGLWidget*)widget)->renderText(0., 0., 0., (mystr), font); 

			glPopMatrix();
		}

		glPopAttrib();
	}

}

#define __cell_apo__

void Renderer_tex2::createCell_atom()
{
	glistCell = _createSphere();
	//qDebug("Renderer_tex2::createCell_atom -- createSphere = %u", glistCell);
}

void Renderer_tex2::saveCellAPO(const QString& filename)
{
#ifndef test_main_cpp
	writeAPO_file(filename, listCell);
#endif
}


QList <CellAPO> Renderer_tex2::listFromAPO_file(const QString& filename)
{
	PROGRESS_DIALOG("Loading Point cloud", widget);
	PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102

	 QList <CellAPO> mylist;
#ifndef test_main_cpp
	 mylist = readAPO_file(filename);
#endif
	 return mylist;
}


void Renderer_tex2::loadCellAPO(const QString& filename)  //090521 RZC: merge reading code to listFromAPO_file
{
	if (map_APOFile_IndexList.contains(filename))
	{
		qDebug()<< "There is a same file in memory, do nothing.";
		return; // do nothing
	}

    QList <CellAPO> mylist = listFromAPO_file(filename);

    // add to cell set
    int nexist = listCell.size();
    listCell << mylist;

    // update map of file_name and cell_index
    QList<int> ind_list;
    ind_list.clear();
    for (int i=0; i<mylist.size(); i++)
    {
    	int index = nexist + i;

        map_CellIndex_APOFile.insert(index, filename);
    	ind_list.append(index);
    }
    map_APOFile_IndexList.insert(filename, ind_list);

    // update bounding box
    apoBB = NULL_BoundingBox;
    foreach(CellAPO S, listCell)
    {
		//apoBB.expand(XYZ(S));
		float d = 2.0*pow(S.volsize/3.1415926*0.75, 1/3.0);
		apoBB.expand(BoundingBox(XYZ(S)-d/2, XYZ(S)+d/2));
    }
}

void Renderer_tex2::drawCellList()
{
	//qDebug("    Renderer_tex2::drawCellList");
	if (sShowSurfObjects==0) return;

	for (int pass=0; pass<numPassFloatDraw(sShowSurfObjects); pass++)
	{
		setFloatDrawOp(pass, sShowSurfObjects);

		for (int i=0; i<listCell.size(); i++)
		{
			const CellAPO& S = listCell[i];
			if (! S.on) continue;
			if (S.selected) HIGHLIGHT_ON();

			float sh = S.intensity/2.0;  //default OpenGL does not permit a shininess or spot exponent over 128
			XYZW ms = S.intensity/255.0;
			//RGBA8 color = XYZW(S.color)*ms;  // 081213
			float d = 2.0*pow(S.volsize/3.1415926*0.75, 1/3.0);

			glPushMatrix();

			glTranslatef(S.x, S.y, S.z);
			glScalef(d, d, d);

			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, sh);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ms.v);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (1/4.0 *ma).v); // 081206, change EMISSION to AMBIENT
			glColor3ubv(S.color.c);

			glPushName(1+i);
				glCallList(glistCell);
			glPopName();

			glPopMatrix();

			if (S.selected) HIGHLIGHT_OFF();
		}

	}
	setFloatDrawOp(-1, sShowSurfObjects);

	// cell name
	//qDebug(" b_showCellName = %i", (b_showCellName));
	//qDebug("widget = 0x%p", widget);
	if (b_showCellName)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND); // no effect to glBlendEquationEXT(GL_MAX_EXT)
		glDisable(GL_LIGHTING);
		disableClipBoundingBox(); //090726

		RGBA32f c = XYZW(1) - color_background;
					//color_line;
		glColor4f(c.r, c.g, c.b, 1);

		for (int i=0; i<listCell.size(); i++)
		{
			const CellAPO& S = listCell[i];
			if (! S.on)	continue;
			if (S.name.isEmpty()) continue;

			glPushMatrix();
			glTranslatef(S.x, S.y, S.z);

			//qDebug()<<" cellName = "<<S.name <<"\n";
			((QGLWidget*)widget)->renderText(0., 0., 0., (S.name));
			//((QGLWidget*)widget)->renderText(0., 0., 0., QString("%1").arg(i+1));

			glPopMatrix();
		}

		glPopAttrib();
	}
}

#define __neuron_swc__

const int mNeuron = 36; // less than 18 make tube and sphere can not connect smoothly

void Renderer_tex2::createNeuron_tube()
{
	glistTube    = _createCylinder(mNeuron,1);
	glistTubeEnd = _createSphere(mNeuron);
}

void Renderer_tex2::drawDynamicNeuronTube(float rb, float rt, float length)
{
	GLUquadric* Q = gluNewQuadric();
	gluQuadricOrientation( Q, GLU_OUTSIDE);
	gluQuadricNormals(Q, GLU_SMOOTH);

	gluCylinder( Q, .5*rb, .5*rt, length,  mNeuron, 1);

	gluDeleteQuadric(Q);
}

void Renderer_tex2::saveNeuronTree(int kk, const QString& filename) //kk is the cur number of the tree to save
{
	if (kk<0 || kk>=listNeuronTree.size())
	{
		qDebug()<< "Invalid tree index number in saveNeuronTree()";
		return;
	}

#ifndef test_main_cpp
	writeSWC_file(filename, listNeuronTree[kk]);
#endif
}


void Renderer_tex2::loadNeuronTree(const QString& filename)
{
    bool contained = false;
    int idx = -1;

    for (int i=0; i<listNeuronTree.size(); i++)
		if (filename == listNeuronTree[i].file) // same file
		{
			contained = true;
			idx = i;
			break;
		}
	if (contained)
	{
		qDebug()<< "There is a same file in memory, do nothing.";
		return; // do nothing
	}

	PROGRESS_DIALOG("Loading Neuron structure "+filename, widget);
	PROGRESS_PERCENT(50); // 0 or 100 not be displayed. 081102

    NeuronTree SS;
#ifndef test_main_cpp
    SS = readSWC_file(filename);
#endif

     // add to neuron_tree set
    if (contained && idx>=0 && idx<listNeuronTree.size())
	{
		listNeuronTree.replace(idx, SS); //090117 use overwrite  by PHC
	}
    if (! contained) //listNeuronTree.contains(SS)) // because NeuronTree contains template, so listNeuronTree.contains() cannot work, 0811115
    {
		listNeuronTree.append(SS);
    }
    updateNeuronBoundingBox();
}

void Renderer_tex2::updateNeuronBoundingBox()
{
    swcBB = NULL_BoundingBox;
    foreach(NeuronTree SS, listNeuronTree)
    //for(int i=0; i<listNeuronTree.size(); i++)
    { //const QList<NeuronSWC> & listNeuron = listNeuronTree.at(i).listNeuron;

    	foreach(NeuronSWC S, SS.listNeuron)
    	//for (int j=0; j<listNeuron.size(); j++)
 		{ //const NeuronSWC & S = listNeuron.at(j);

			//swcBB.expand(XYZ(S));
			float d = S.r *2;
			swcBB.expand(BoundingBox(XYZ(S)-d, XYZ(S)+d));
		}
    }
}

#define CURVE_NAME "curve_segment"
#define CURVE_FILE "curve_segment"

void Renderer_tex2::addCurveSWC(vector<XYZ> &loc_list, int chno)
{
#ifndef test_main_cpp

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg =  v3dr_getImage4d(_idep);
	if (w && curImg)
	{		
		curImg->proj_trace_add_curve_segment(loc_list, chno);
		curImg->update_3drenderer_neuron_view(w, this);
	}

#else

	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

	try {
		int count = 0;

		qDebug("-------------------------------------------------------");
		for (int k=0;k<loc_list.size();k++)
		{
			count++;
			NeuronSWC S;

			S.n 	= 1+k;
			S.type 	= 0;
			S.x 	= loc_list.at(k).x;
			S.y 	= loc_list.at(k).y;
			S.z 	= loc_list.at(k).z;
			S.r 	= 1;
			S.pn 	= (k==0)? -1 : k;

			//qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);
			{
				listNeuron.append(S);
				hashNeuron.insert(S.n, listNeuron.size()-1);
			}
		}
		qDebug("---------------------add %d lines, %d remained lines", count, listNeuron.size());

		NeuronTree SS;

		SS.n = -1;
		SS.color = random_rgba8(255);
		SS.on = true;
		SS.listNeuron = listNeuron;
		SS.hashNeuron = hashNeuron;

		//091028: this is important
		{
			SS.n = 1+listNeuronTree.size();
			QString snum = QString("_%1").arg(SS.n);
			SS.name = CURVE_NAME +snum;
			SS.file = CURVE_FILE +snum;
			listNeuronTree.append(SS);
		}

	} CATCH_handler( "Renderer_tex2::addCurveSWC" );

    updateNeuronBoundingBox();
    updateBoundingBox(); // all of loaded bounding-box are updated here

#endif
}

#ifndef test_main_cpp

void Renderer_tex2::updateNeuronTree(V_NeuronSWC & seg)
{
	qDebug("  Renderer_tex2::updateNeuronTree( V_NeuronSWC_list )");

//	PROGRESS_DIALOG("Updating Neuron structure", widget);
//	PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102

	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

	try {
		int count = 0;

		qDebug("-------------------------------------------------------");
		for (int k=0;k<seg.row.size();k++)
		{
			count++;
			NeuronSWC S;

			S.n 	= seg.row.at(k).data[0];
			S.type 	= seg.row.at(k).data[1];
			S.x 	= seg.row.at(k).data[2];
			S.y 	= seg.row.at(k).data[3];
			S.z 	= seg.row.at(k).data[4];
			S.r 	= seg.row.at(k).data[5];
			S.pn 	= seg.row.at(k).data[6];

			//for hit & editing
			S.seg_id       = seg.row.at(k).seg_id;
			S.nodeinseg_id = seg.row.at(k).nodeinseg_id;

			//qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);

			//if (! listNeuron.contains(S)) // 081024
			{
				listNeuron.append(S);
				hashNeuron.insert(S.n, listNeuron.size()-1);
			}
		}
		qDebug("---------------------read %d lines, %d remained lines", count, listNeuron.size());

		if (listNeuron.size()<1) //this is used to remove a neuron with the same name if the size is <=0
		{
			for (int i=0; i<listNeuronTree.size(); i++)
			{
				if (listNeuronTree[i].file == QString(seg.file.c_str())) // same file. try to remove all instances with the same name
				{
					listNeuronTree.removeAt(i);
					qDebug()<<"find name matched and remove an empty neuron";
				}
			}
		    updateNeuronBoundingBox();

		    qDebug()<<"remove an empty neuron";
			return; //////////////////////////////
		}

		NeuronTree SS;

		SS.n = -1;
		SS.color = XYZW(seg.color_uc[0],seg.color_uc[1],seg.color_uc[2],seg.color_uc[3]);
		SS.on = true;
		SS.listNeuron = listNeuron;
		SS.hashNeuron = hashNeuron;

		//090914 RZC
		SS.name = seg.name.c_str();
		SS.file = seg.file.c_str();

	    // add or replace into listNeuronTree
		bool contained = false;
		for (int i=0; i<listNeuronTree.size(); i++)
			if (SS.file == listNeuronTree[i].file) // same file to replace it
			{
				contained = true;
				SS.n = 1+i;
				listNeuronTree.replace(i, SS); //090117 use overwrite  by PHC
				break;
			}
		if (! contained) //listNeuronTree.contains(SS)) // because NeuronTree contains template, so listNeuronTree.contains() cannot work, 081115
		{
			SS.n = 1+listNeuronTree.size();
			listNeuronTree.append(SS);
		}

		// make sure only one current editing neuron has editable flag
		qDebug("	lastEditingNeuron = %d, NeuronTree.n = %d", curEditingNeuron, SS.n);
		//qDebug("-------------------------------------------------------");
		for (int i=0; i<listNeuronTree.size(); i++)
		{
			listNeuronTree[i].editable = (1+i==SS.n); //090923
		}
		curEditingNeuron = SS.n;

	} CATCH_handler( "Renderer_tex2::updateNeuronTree( V_NeuronSWC )" );

    updateNeuronBoundingBox();
    updateBoundingBox(); // all of loaded bounding-box are updated here
}

V_NeuronSWC_list Renderer_tex2::copyToEditableNeuron(NeuronTree * ptree)
{
	qDebug("  Renderer_tex2::copyToEditableNeuron");

	return NeuronTree__2__V_NeuronSWC_list(ptree); // by PHC, 2010-06-10 as I separate this function to NeuronTree__2__V_NeuronSWC_list()
}

void Renderer_tex2::finishEditingNeuronTree()
{
	qDebug("  Renderer_tex2::finishEditingNeuronTree");
	for (int i=0; i<listNeuronTree.size(); i++)
	{
		listNeuronTree[i].editable = false; //090928
	}
	//090929
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	if (w)	w->updateTool();
}

#endif


void Renderer_tex2::toggleLineType()
{
	lineType = (lineType +1) %2;
	if (lineType)	tryObjShader = 0; //091019
	//qDebug("    Renderer_tex2::toggleLineType = %d", lineType);
	//compileNeuronTreeList();
}

//void Renderer_tex2::compileNeuronTreeList()
//{
//	if (compiledNeuron)
//		for (int i=0; i<listNeuronTree.size(); i++)
//	{
//		compileNeuronTree(i); /// make re-compiled list_glistNeuron[i]
//	}
//}
//
//void Renderer_tex2::compileNeuronTree(int index)
//{
//	if (! compiledNeuron) return;
//	makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081117
//
//	if (index>=0 && index<listNeuronTree.size())
//	{
//		glistNeuron = list_glistNeuron[index];
//		glDeleteLists(glistNeuron, 1);
//	}
//
//	GLuint g = glGenLists(1);
//	glNewList(g, GL_COMPILE);
//
//	drawNeuronTree(index);
//
//	glEndList();
//	glistNeuron = g;
//
//	if (index>=0 && index<listNeuronTree.size())
//	{
//		list_glistNeuron[index] = glistNeuron;
//	}
//}


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
		};//////////////////////////////////////////////////////////////////////////////////
const int neuron_type_color_num = sizeof(neuron_type_color)/(sizeof(GLubyte)*3);

void Renderer_tex2::drawNeuronTree(int index)
{
	if (listNeuronTree.size() <1)  return;
	if (index<0 || index>=listNeuronTree.size()) return;

	const QList <NeuronSWC> & listNeuron = listNeuronTree.at(index).listNeuron;
	const QHash <int, int> & hashNeuron = listNeuronTree.at(index).hashNeuron;
	RGBA8 rgba = listNeuronTree.at(index).color;
	bool on    = listNeuronTree.at(index).on;
	bool editable = listNeuronTree.at(index).editable;
	NeuronSWC S0, S1;

	if (! on) return;

//  for debug ////////////////////////////
//	if (listNeuron.size()<=0) return;
//	S1 = listNeuron.last();
//	S0 = listNeuron.at(0);
//	//qDebug("last-0  (%g %g %g) - (%g %g %g)",  S1.x,S1.y,S1.z, S0.x,S0.y,S0.z);
//	glColor3ub(0, 0, 255);
//	glBegin(GL_LINES);
//	{
//		glVertex3f(S0.x, S0.y, S0.z);
//		glVertex3f(S1.x, S1.y, S1.z);
//	}
//	glEnd(); ////////////////////////////

	for (int i=0; i<listNeuron.size(); i++)
	{
		S1 = listNeuron.at(i);   // at(i) faster than [i]
		//if (S1.pn <1)	continue; 	// skip the first point

		bool valid = false;
		if (S1.pn == -1) // root end, 081105
		{
			S0 = S1;
			valid = true;
		}
		else if (S1.pn >=0) //change to >=0 from >0, PHC 091123
		{
//			for (int j=0; j<listNeuron.size(); j++)
//			{
//				S0 = listNeuron.at(j);
//				if (S0.n==S1.pn)
//				{
//					valid = true;
//					break;
//				}
//			}
			// or using hash for finding parent node
			int j = hashNeuron.value(S1.pn, -1);
			if (j>=0 && j <listNeuron.size())
			{
				S0 = listNeuron.at(j);
				valid = true;
			}
		}
		if (! valid)  continue;


		//drawNeuronTube + TubeEnd
		glPushMatrix();
		{
			//qDebug("%i-%i  (%g %g %g) - (%g %g %g)", i,j,  S1.x,S1.y,S1.z, S0.x,S0.y,S0.z);

			//if (rgba.a==0 || lineType==1)
			if (rgba.a==0 || editable) //make the skeleton be able to use the default color by adjusting alpha value
			{
				int type = S1.type; 			 // 090925
				if (editable)
				{
					type = S1.seg_id %(neuron_type_color_num -5)+5; //090829, 091027 RZC: segment color using hanchuan's neuron_type_color
				}
				glColor3ubv( neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ] ); // 081230, 090331
			}
			else
				glColor3ubv(rgba.c);


			// (0,0,0)--(0,0,1) ==> S0--S1
			XYZ D = S0 - S1;
			float length = norm(D);
			float r1 = S1.r;
			float r0 = S0.r;
			//if (r1*length<1) qDebug("length, r1, r0 = (%g %g %g)", length, r1, r0);
			float rf = 2;
			r1 *= rf;
			r0 *= rf;

			if (lineType==0)
			{
				GLfloat m[4][4];
				XYZ A, B, C;
				C = //XYZ(0,0,1);
					D; normalize(C);	 if (norm(C)<.9) C = XYZ(0,0,1);
				B = //XYZ(0,1,0);
					cross(C, XYZ(0,0,1)); normalize(B);		 if (norm(B)<.9) B = XYZ(0,1,0);
				A = //XYZ(1,0,0);
					cross(C, B); //normalize(A);
				m[0][0] = A.x;	m[1][0] = B.x;	m[2][0] = C.x;	m[3][0] = S1.x;
				m[0][1] = A.y;	m[1][1] = B.y;	m[2][1] = C.y;	m[3][1] = S1.y;
				m[0][2] = A.z;	m[1][2] = B.z;	m[2][2] = C.z;	m[3][2] = S1.z;
				m[0][3] = 0;	m[1][3] = 0;	m[2][3] = 0;	m[3][3] = 1;
				glMultMatrixf(&m[0][0]);

				if (length >0)
				{
					glPushMatrix();

//					float s,a,b,c;
//					s = length;
//					c = (r1-r0)*s / ((r1-r0)*s - r1);		if (r1==r0) c = 0;
//					b = (s-1)*(c+1);
//					a = b;
//					m[0][0] = r1*(a+1);		m[1][0] = 0;			m[2][0] = 0;			m[3][0] = 0;
//					m[0][1] = 0;			m[1][1] = r1*(b+1);		m[2][1] = 0;			m[3][1] = 0;
//					m[0][2] = 0;			m[1][2] = 0;			m[2][2] = s*(c+1);		m[3][2] = 0;
//					m[0][3] = a;			m[1][3] = b;			m[2][3] = c;			m[3][3] = 1;
//					glMultMatrixf(&m[0][0]); // OpenGL Matrix stack cannot support 3D projective transform division, by RZC 080901
					//glScalef(r1, r1, length);
					//glCallList(glistTube);

					drawDynamicNeuronTube(r1, r0, length); // dynamically create tube, slowly

					glPopMatrix();
				}

				glPushMatrix();
				{
					glScaled(r1, r1, r1);
					//glScaled(r1/thicknessX, r1/thicknessY, r1/thicknessZ); // 090421 RZC: adjusted with image thickness
					glCallList(glistTubeEnd);
				}
				glPopMatrix();

			}
			else if (lineType==1)
			{
				if (length >0)  // branch line
				{
					glLineWidth(lineWidth);
					glBegin(GL_LINES);
						glVertex3f(S0.x, S0.y, S0.z);	glVertex3f(S1.x, S1.y, S1.z);
					glEnd();
					if (nodeSize)
					{
						glPointSize(nodeSize);
						glBegin(GL_POINTS);
							glVertex3f(S1.x, S1.y, S1.z);
						glEnd();
					}
				}
				else if (rootSize)// root point
				{
					glPointSize(rootSize);
					glBegin(GL_POINTS);
						glVertex3f(S1.x, S1.y, S1.z);
					glEnd();
				}
				glLineWidth(1);
				glPointSize(1);
			}
		}
		glPopMatrix();
		valid = false;
	}//for
}

void Renderer_tex2::drawNeuronTreeList()
{
	//qDebug("    Renderer_tex2::drawNeuronTree");
	if (sShowSurfObjects==0) return;
	if (listNeuronTree.size()<1)  return;

	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
	if (lineType==1) // float line
	{
		glDisable(GL_LIGHTING);
	}
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	//	if (lineType==0 && sShowSurfObjects==1) // restore depth buffer // very very slowly!!! discard, by RZC 080902
	//	{
	//		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	//		glStencilMask(0);
	//		glDepthMask(GL_TRUE);
	//		glRasterPos2i(0,0);
	//		glDrawPixels(screenW,screenH, GL_DEPTH_COMPONENT, GL_FLOAT, depth_buffer);
	//		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	//		glStencilMask(-1);
	//		//glDepthMask(GL_TRUE);
	//	}

	for (int pass=0; pass<numPassFloatDraw(sShowSurfObjects); pass++)
	{
		setFloatDrawOp(pass, sShowSurfObjects);

		for (int i=0; i<listNeuronTree.size(); i++)
		{
			const NeuronTree& S = listNeuronTree[i];
			if (! S.on)  continue;
			if (S.selected)  HIGHLIGHT_ON();

			glPushName(1+i);
				drawNeuronTree(i); // only instantly draw, seems good for multiple neurons, 081115
			glPopName();

			if (S.selected) HIGHLIGHT_OFF();
		}
	}
	setFloatDrawOp(-1, sShowSurfObjects);

//	glEnable(GL_LIGHTING);
//	glDisable(GL_CULL_FACE);
	glPopAttrib();
}

////////////////////////////////////////////////////////

//#include "labelfield.cpp"




