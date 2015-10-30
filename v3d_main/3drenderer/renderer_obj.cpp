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
 * renderer_obj.cpp
 *
 *  Created on: Aug 29, 2008
 *      Author: ruanzongcai
 * Last update: 090220: by Hanchuan Peng, add neuron coordinate manipulation functions
 * Last update: 090331, by Hanchuan Peng, extend several more colors for neuron display
 * Last update: 090716: by Hanchuan Peng, now use the apo file loader writer in the basic_c_fun directory
 * Last update: by PHC, 2010-06-02, separate the content of function copyToEditableNeuron() to NeuronTree__2__V_NeuronSWC_list()
 * Last update: 120209: by Yinan Wan, add matlab heat map to neuron_type_color list, it starts from type 19
 * Last update: 150506: by PHC. add asc reading support
 */
#include "renderer_gl1.h"
#include "v3dr_glwidget.h"
#include "freeglut_geometry_r.c"

#include "../io/asc_to_swc.h"
//#include "../../../vaa3d_tools/released_plugins/v3d_plugins/resample_swc/resampling.h"


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
void Renderer_gl1::loadObjectFromFile(const char* url)
{
	qDebug("   Renderer_gl1::loadObjectFromFile (url)");
	QString filename;
	if (url)
		filename = QString(url);
	else
	    filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
	    		"",
                QObject::tr("Supported file (*.swc *.eswc *.asc *.apo *.raw *.v3draw *.vaa3draw *.v3dpbd *.tif *.tiff *.v3ds *.vaa3ds *.obj *.marker *.csv)"
                        ";;Neuron structure	(*.swc *.eswc *.asc)"
	    				";;Point Cloud		(*.apo)"
                        ";;Label field		(*.raw *.v3draw *.vaa3draw *.v3dpbd *.tif *.tiff)"
	    				";;Label Surface	(*.vaa3ds *.v3ds *.obj)"
						";;Landmarks		(*.marker *.csv)"
	    				));
    qDebug()<< "open file: " << filename;
	if (filename.size()>0)
		loadObjectFilename(filename);
}
void Renderer_gl1::loadObjectListFromFile()
{
	qDebug("   Renderer_gl1::loadObjectListFromFile");
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
void Renderer_gl1::loadObjectFilename(const QString& filename)
{
#ifndef test_main_cpp
    makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105, 081122
	int type = 0;
	try {
        iDrawExternalParameter * ep = (iDrawExternalParameter*)_idep;
		// if create labelfield
		if (filename.endsWith(".tif", Qt::CaseInsensitive) ||
            filename.endsWith(".tiff", Qt::CaseInsensitive) ||
			filename.endsWith(".raw", Qt::CaseInsensitive) ||
			filename.endsWith(".v3draw", Qt::CaseInsensitive) ||
			filename.endsWith(".vaa3draw", Qt::CaseInsensitive)
            )
		{
			loadLabelfieldSurf(filename);
            ep->labelfield_file = filename;
		}
		// if label surface obj
		else if (filename.endsWith(".obj", Qt::CaseInsensitive))
		{
			type = stLabelSurface;
			loadWavefrontOBJ(filename);
            ep->surface_file = filename;
        }
		// if label surface v3ds --binary format obj
		else if (filename.endsWith(".v3ds", Qt::CaseInsensitive) || filename.endsWith(".vaa3ds", Qt::CaseInsensitive) )
		{
			type = stLabelSurface;
			loadV3DSurface(filename);
            ep->surface_file = filename;
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
            if (!(ep->swc_file_list.contains(filename)))
                ep->swc_file_list << filename;
        }
		// if eswc
		else if (filename.endsWith(".eswc", Qt::CaseInsensitive)) //PHC, 20120217
		{
			type = stNeuronStructure;
			loadNeuronTree(filename);
            if (!(ep->swc_file_list.contains(filename)))
                ep->swc_file_list << filename;
        }
        // if asc
        else if (filename.endsWith(".asc", Qt::CaseInsensitive)) //PHC, 20150506
        {
            type = stNeuronStructure;
            loadNeuronTree(filename);
            if (!(ep->swc_file_list.contains(filename)))
                ep->swc_file_list << filename;
        }
        // if apo
		else if (filename.endsWith(".apo", Qt::CaseInsensitive))
		{
			type = stPointCloud;
			loadCellAPO(filename);
            if (!(ep->pointcloud_file_list.contains(filename)))
                ep->pointcloud_file_list << filename;
		}
	} CATCH_handler( "Renderer_gl1::loadObjectFilename" );
    updateBoundingBox(); ///// 081121, all of loaded bounding-box are updated here
	if (widget) //090522 RZC
	{
		((V3dR_GLWidget*)widget)->surfaceSelectTab(type-1);
	}
#endif
}
void Renderer_gl1::saveSurfFile()
{
	qDebug("   Renderer_gl1::saveSurfFile");
	if (list_listTriangle.size()==0)
	{
		QMessageBox::information(0, QObject::tr("save file"), QObject::tr("NO surface to save!"));
		return;
	}
	extern QString lf_data_title;
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save Surface File"),
            lf_data_title+".vaa3ds",
    		QObject::tr("Vaa3D Surface Object (*.vaa3ds)"
                    ";;Vaa3D Surface Object (*.v3ds)"
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
        else if (filename.endsWith(".vaa3ds", Qt::CaseInsensitive) ||
            filename.endsWith(".v3ds", Qt::CaseInsensitive) )
		{
			saveV3DSurface(filename);
		}
        else
        {
            v3d_msg("You have supplied a file extension which is not offcially supported. This file name will be used, but to load it correctly into Vaa3D, you will need to change it to .vaa3ds or .v3ds extension(s).", 1);
            saveV3DSurface(filename);
        }
    } CATCH_handler( "Renderer_gl1::saveSurfFile" );
}
/////////////////////////////////////////////////////////////////////////////////////////////
void Renderer_gl1::loadObj()
{
	cleanObj(); //070905
	qDebug("  Renderer_gl1::loadObj");
	makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105
	createMarker_atom(); 		//qDebug("	 glGetError(createMarker_atom) = %u", glGetError());
	createNeuron_tube();     //qDebug("	 glGetError(createNeuron_tube) = %u", glGetError());
	createCell_atom();     		//qDebug("	 glGetError(createCell_atom) = %u", glGetError());
	CHECK_GLErrorString_throw();
}
void Renderer_gl1::cleanObj()
{
	qDebug("   Renderer_gl1::cleanObj");
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
void Renderer_gl1::updateBoundingBox()
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
	qDebug("  Renderer_gl1::updateBoundingBox surface (%g %g %g)--(%g %g %g)", sBB.x0,sBB.y0,sBB.z0, sBB.x1,sBB.y1,sBB.z1 );
	qDebug("  Renderer_gl1::updateBoundingBox default (%g %g %g)--(%g %g %g)", BB.x0,BB.y0,BB.z0, BB.x1,BB.y1,BB.z1 );
	updateThicknessBox(); //090806
}
void Renderer_gl1::setThickness(double t)
{
	//qDebug("  Renderer_gl1::setThickness");
	thicknessZ = t;
	updateThicknessBox();
}
void Renderer_gl1::updateThicknessBox()
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
void Renderer_gl1::setMarkerSpace()
{
	Renderer::setObjectSpace(); //// object put in original image space, 090715 off
	glTranslated(-start1,-start2,-start3); //090715
	glScaled(thicknessX, thicknessY, thicknessZ);
}
void Renderer_gl1::drawMarker()
{
	glPushName(stImageMarker);
		drawMarkerList();
	glPopName();
}
void Renderer_gl1::setSurfaceStretchSpace()
{
	Renderer::setObjectSpace(); //// object put in original image space, 090715 off
	glTranslated(-start1,-start2,-start3); //090715
	if (//have_image() &&
			b_surfStretch)  // 090423 RZC: stretch surface object with image thickness
	{
		glScaled(thicknessX, thicknessY, thicknessZ);
	}
}
void Renderer_gl1::drawObj()
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
void Renderer_gl1::disObjLighting()
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
void Renderer_gl1::setObjLighting()
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
void Renderer_gl1::beginHighlight()
{
	XYZW mater_emission = XYZW( .3, .3, .3,  1 );
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mater_emission.v);
}
void Renderer_gl1::endHighlight()
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
void Renderer_gl1::createMarker_atom()
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
void Renderer_gl1::updateLandmark()
{
	//qDebug("  Renderer_gl1::updateLandmark");
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
void Renderer_gl1::drawMarkerList()
{
	//qDebug("    Renderer_gl1::drawMarkerList");
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
void Renderer_gl1::createCell_atom()
{
	glistCell = _createSphere();
	//qDebug("Renderer_gl1::createCell_atom -- createSphere = %u", glistCell);
}
void Renderer_gl1::saveCellAPO(const QString& filename)
{
#ifndef test_main_cpp
	writeAPO_file(filename, listCell);
#endif
}
QList <CellAPO> Renderer_gl1::listFromAPO_file(const QString& filename)
{
	PROGRESS_DIALOG("Loading Point cloud", widget);
	PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102
	 QList <CellAPO> mylist;
#ifndef test_main_cpp
	 mylist = readAPO_file(filename);
#endif
	 return mylist;
}
void Renderer_gl1::loadCellAPO(const QString& filename)  //090521 RZC: merge reading code to listFromAPO_file
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
void Renderer_gl1::drawCellList()
{
	//qDebug("    Renderer_gl1::drawCellList");
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
void Renderer_gl1::createNeuron_tube()
{
	glistTube    = _createCylinder(mNeuron,1);
	glistTubeEnd = _createSphere(mNeuron);
}
void Renderer_gl1::drawDynamicNeuronTube(float rb, float rt, float length)
{
	GLUquadric* Q = gluNewQuadric();
	gluQuadricOrientation( Q, GLU_OUTSIDE);
	gluQuadricNormals(Q, GLU_SMOOTH);
	gluCylinder( Q, .5*rb, .5*rt, length,  mNeuron, 1);
	gluDeleteQuadric(Q);
}
void Renderer_gl1::saveNeuronTree(int kk, const QString& filename) //kk is the cur number of the tree to save
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
void Renderer_gl1::loadNeuronTree(const QString& filename)
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

    NeuronTree SS;

#ifndef test_main_cpp
    if (filename.endsWith(".swc", Qt::CaseInsensitive) || filename.endsWith(".eswc", Qt::CaseInsensitive))
    {
        SS = readSWC_file(filename);
//        if(SS.listNeuron.size()> 10000)
//        {
//            bool ok;
//            double ratio = QInputDialog::getDouble(0, "Large SWC File","Downsample by:            ",1,0,2147483647,0.1,&ok);
//            if(ok)
//            {
//                SS = resample_ratio(SS,ratio);
//                SS.color.r = 0;
//                SS.color.g = 0;
//                SS.color.b = 0;
//                SS.color.a = 0;
//            }
//        }
    }
    else if (filename.endsWith(".asc", Qt::CaseInsensitive))
        asc_to_swc::readASC_file(SS, (char *)(qPrintable(filename)));

#endif
    PROGRESS_DIALOG("Loading Neuron structure "+filename, widget);
    PROGRESS_PERCENT(50); // 0 or 100 not be displayed. 081102
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
void Renderer_gl1::updateNeuronBoundingBox()
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
void Renderer_gl1::addCurveSWC(vector<XYZ> &loc_list, int chno)
{
#ifndef test_main_cpp
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg =  v3dr_getImage4d(_idep);

    if (w && curImg)
    {
        if (0)  //should append the curves to the being-edited neuron directly
        {
            v3d_msg("NeuronTree oldtree = listNeuronTree.at(realCurEditingNeuron_inNeuronTree);");

            NeuronTree oldtree = listNeuronTree.at(realCurEditingNeuron_inNeuronTree);
            NeuronTree curTree  = curImg->proj_trace_add_curve_segment_append_to_a_neuron(loc_list, chno,
                                                                                          oldtree);
            listNeuronTree.replace(realCurEditingNeuron_inNeuronTree, curTree);
            curImg->update_3drenderer_neuron_view(w, this);
        }
        else
        {
            curImg->proj_trace_add_curve_segment(loc_list, chno);
            curImg->update_3drenderer_neuron_view(w, this);
        }
    }


#else
    v3d_msg("testmain addCurveSWC(vector<XYZ> &loc_list, int chno)");

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
        RGBA8 tt; tt.r = 255;tt.g = 0;tt.b = 0;tt.a = 0;
        SS.color = tt; //RGBA8(255, 0,0,0);//random_rgba8(255);
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
	} CATCH_handler( "Renderer_gl1::addCurveSWC" );
    updateNeuronBoundingBox();
    updateBoundingBox(); // all of loaded bounding-box are updated here
#endif
}
#ifndef test_main_cpp
void Renderer_gl1::updateNeuronTree(V_NeuronSWC & seg)
{
	qDebug("  Renderer_gl1::updateNeuronTree( V_NeuronSWC_list )");
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
        if (!contained
                //&& SS.file!=QString(TRACED_FILE)
                ) //listNeuronTree.contains(SS)) // because NeuronTree contains template, so listNeuronTree.contains() cannot work, 081115
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

        if (listNeuronTree.size()==1 && listNeuronTree[0].file=="vaa3d_traced_neuron" && listNeuronTree[0].name=="vaa3d_traced_neuron")
        {
            listNeuronTree[0].editable = true;
            curEditingNeuron = 1;
        }
	} CATCH_handler( "Renderer_gl1::updateNeuronTree( V_NeuronSWC )" );
    updateNeuronBoundingBox();
    updateBoundingBox(); // all of loaded bounding-box are updated here
}
V_NeuronSWC_list Renderer_gl1::copyToEditableNeuron(NeuronTree * ptree)
{
	qDebug("  Renderer_gl1::copyToEditableNeuron");
	return NeuronTree__2__V_NeuronSWC_list(ptree); // by PHC, 2010-06-10 as I separate this function to NeuronTree__2__V_NeuronSWC_list()
}
void Renderer_gl1::finishEditingNeuronTree()
{
	qDebug("  Renderer_gl1::finishEditingNeuronTree");
	for (int i=0; i<listNeuronTree.size(); i++)
	{
		listNeuronTree[i].editable = false; //090928
	}
    realCurEditingNeuron_inNeuronTree = -1;//150523

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

	//090929
	if (w)	w->updateTool();
}
#endif
void Renderer_gl1::toggleLineType()
{
	lineType = (lineType +1) %2;
	if (lineType)	tryObjShader = 0; //091019
	//qDebug("    Renderer_gl1::toggleLineType = %d", lineType);
	//compileNeuronTreeList();
}
//void Renderer_gl1::compileNeuronTreeList()
//{
//	if (compiledNeuron)
//		for (int i=0; i<listNeuronTree.size(); i++)
//	{
//		compileNeuronTree(i); /// make re-compiled list_glistNeuron[i]
//	}
//}
//
//void Renderer_gl1::compileNeuronTree(int index)
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
void Renderer_gl1::drawNeuronTree(int index)
{
	if (listNeuronTree.size() <1)  return;
	if (index<0 || index>=listNeuronTree.size()) return;
	const QList <NeuronSWC> & listNeuron = listNeuronTree.at(index).listNeuron;
	const QHash <int, int> & hashNeuron = listNeuronTree.at(index).hashNeuron;
	RGBA8 rgba = listNeuronTree.at(index).color;
	bool on    = listNeuronTree.at(index).on;
	bool editable = listNeuronTree.at(index).editable;
    int cur_linemode = listNeuronTree.at(index).linemode; //added by PHC 20130926

//    v3d_msg(QString("neuron mode %1").arg(cur_linemode), 0);

    int cur_lineType = (cur_linemode==0 || cur_linemode==1) ? cur_linemode : lineType;

	NeuronSWC S0, S1;
     // for neuron color: same as neuron label color (ZJL)
     GLubyte neuronColor[3];
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
					int ncolorused = neuron_type_color_num; if (neuron_type_color_num>19) ncolorused = 19; //added by PHC, 20120330
					type = S1.seg_id %(ncolorused -5)+5; //090829, 091027 RZC: segment color using hanchuan's neuron_type_color
				}
				if (type >= 300 && type <= 555 )  // heat colormap index starts from 300 , for sequencial feature scalar visaulziation
				{
					neuronColor[0] =  neuron_type_color_heat[ type - 300][0];
					neuronColor[1] =  neuron_type_color_heat[ type - 300][1];
					neuronColor[2] =  neuron_type_color_heat[ type - 300][2];
				} 
				else
				{
					neuronColor[0] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][0];
					neuronColor[1] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][1];
					neuronColor[2] =  neuron_type_color[ (type>=0 && type<neuron_type_color_num)? type : 0 ][2];
				}
				glColor3ubv(neuronColor); // 081230, 090331
			}
			else
			{
				glColor3ubv(rgba.c);
				neuronColor[0] = rgba.c[0];
				neuronColor[1] = rgba.c[1];
				neuronColor[2] = rgba.c[2];
			}
			// (0,0,0)--(0,0,1) ==> S0--S1
			XYZ D = S0 - S1;
			float length = norm(D);
			float r1 = S1.r;
			float r0 = S0.r;
			//if (r1*length<1) qDebug("length, r1, r0 = (%g %g %g)", length, r1, r0);
			float rf = 2;
			r1 *= rf;
			r0 *= rf;
			if (cur_lineType==0)
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
			else if (cur_lineType==1)
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
     // // display neuron label for testing comparison purpose ZJL 20120221
     // bool b_neuronLabel=true;
     // if(b_neuronLabel)
     // {
     //      XYZ label_loc; // it is located beside the first neuron
     //      label_loc.x = listNeuron.at(0).x;
     //      label_loc.y = listNeuron.at(0).y;
     //      label_loc.z = listNeuron.at(0).z;
     //      glPushAttrib(GL_ENABLE_BIT);
	// 	glDisable(GL_DEPTH_TEST);
	// 	glDisable(GL_STENCIL_TEST);
	// 	glDisable(GL_BLEND);
	// 	glDisable(GL_LIGHTING);
	// 	disableClipBoundingBox();
     //      //glColor4ub(neuronColor[0], neuronColor[1], neuronColor[2], 255);
     //      glColor4ub(255, 0, 0, 255);
     //      // move to the label position
     //      glPushMatrix();
     //      glTranslated(label_loc.x+index*5, label_loc.y+index*5, label_loc.z+index*5);
     //      ((QGLWidget*)widget)->renderText(0., 0., 0., QString("%1").arg(index));
     //      //char sbuf[20];	sprintf(sbuf, "%d", i+1);	drawString(0, 0, 0, sbuf);
     //      glPopMatrix();
	// 	glPopAttrib();
     // }
}
void Renderer_gl1::drawNeuronTreeList()
{
	//qDebug("    Renderer_gl1::drawNeuronTree");
	if (sShowSurfObjects==0) return;
	if (listNeuronTree.size()<1)  return;
	glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);

    {
        V3DLONG nlines_1=0, nlines_minus1=0;
        for (V3DLONG i=0; i<listNeuronTree.size();i++)
        {
            if (listNeuronTree[i].linemode==1)
                nlines_1++;
            else if (listNeuronTree[i].linemode!=0 && listNeuronTree[i].linemode!=1)
                nlines_minus1++;
        }

        if (nlines_1==listNeuronTree.size() ) // float line  // consider combination of tube and line mode displayed neuron now. by PHC 20130926
        {
            glDisable(GL_LIGHTING);
        }
        else if (nlines_minus1==listNeuronTree.size() && lineType==1)
        {
            glDisable(GL_LIGHTING);
        }
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
/*
//#include "labelfield.cpp"
*/
