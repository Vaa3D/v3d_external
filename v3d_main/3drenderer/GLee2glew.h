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
 *  GLee2glew.h
 *
 *  Created by Ruan Zongcai on 2020-2-7.
 *
 *  Translate GLee to GLEW because glee had stopped updating for a long time.
 */

#ifndef GLEE2GLEW_H_
#define GLEE2GLEW_H_

/************************************************************************************************************
 * if try to include <QtWidgets> it will also include gl.h with QtOGL, then GLEW throws error when including glew.h,
 * because gl.h has been included before GLEW.
 * If try to include glew.h before any Qt includes, then Qt undefines all GLEW macros (except *EXT, *ARB) and get undefined symbol errors.
 *
Qt5 seems to have set OpenGLES to a high priority than desktop openGL.
Don't think GLEW will work with the Angle based build of Qt5.

One solution to this is to build Qt5 from source code suing the setting -opengl desktop before you build.
Something like this:  configure -debug-and-release -opengl desktop
Then use nmake to build and it works fine.

Another approach with using OpenGL extension with Qt5 is to separate all OpenGL related part from Qt class implementation.
In the Qt part just call into the framework written OpenGL code through regular C or C++ interfaces using standard types.
Since the actual OpenGL code makes no references to Qt, then it doesn't have to include Qt headers, avoiding problems.
************************************************************************************************************/


//@2020-5-10 RZC: for crash using glew & Qt4 at linux
//@2020-10-31 RZC: fixed crash by USING (glewExperimental=true) BEFORE glewInit() AT LINUX

#if 0// ! defined( USE_Qt5 )
//
//#include <QtGui> ////for error: #error qdatastream.h must be included before any header file that defines Status
//#define GLEW_STATIC ////STATIC link by including glew.c into GLee2glew.c
//#include <glew/GL/glew.h>////STATIC link by including GLee_r.c into GLee2glew.c
////#undef GL_ARB_vertex_buffer_object
////#include "GLee_r.h"
//#include <GL/glu.h> ////for error: gluErrorString was not declared in this scope
//
//#if ! (defined(_WIN32) || defined(_WIN64))// old EXT only for WINDOWS
//#define glBlendEquationEXT	glBlendEquation
//#define glBlendColorEXT		glBlendColor
//#define glTexImage3DEXT		glTexImage3D
//#define glTexSubImage3DEXT	glTexSubImage3D
////#define glGenBuffersARB		glGenBuffers
////#define glBindBufferARB		glBindBuffer
////#define glDeleteBuffersARB	glDeleteBuffers
////#define glMapBufferARB		glMapBuffer
////#define glUnmapBufferARB	glUnmapBuffer
////#define glBufferDataARB		glBufferData
//#endif
//
#else

#define GLEW_STATIC ////STATIC link by including glew.c into GLee2glew.c
#include <glew/GL/glew.h>


#define GLEE_VERSION_2_0    GLEW_VERSION_2_0
#define GLEE_VERSION_1_5    GLEW_VERSION_1_5
#define GLEE_VERSION_1_4    GLEW_VERSION_1_4
#define GLEE_VERSION_1_3    GLEW_VERSION_1_3
#define GLEE_VERSION_1_2    GLEW_VERSION_1_2

//#define GLeeInit() (glewInit())
//// MUST USING (glewExperimental=true) BEFORE glewInit() AT LINUX
#define GLeeInit()  ((glewExperimental=true) && (glewInit()==GLEW_OK))

#define GLEE_EXT_blend_minmax               GLEW_EXT_blend_minmax
#define GLEE_EXT_blend_subtract             GLEW_EXT_blend_subtract
#define GLEE_EXT_blend_color                GLEW_EXT_blend_color

#define GLEE_ARB_texture_env_combine        GLEW_ARB_texture_env_combine
#define GLEE_ARB_multisample                GLEW_ARB_multisample
#define GLEE_ARB_multitexture               GLEW_ARB_multitexture
#define GLEE_EXT_texture3D                  GLEW_EXT_texture3D
#define GLEE_ARB_texture_compression        GLEW_ARB_texture_compression
#define GLEE_EXT_texture_compression_s3tc   GLEW_EXT_texture_compression_s3tc

#define GLEE_ARB_texture_non_power_of_two   GLEW_ARB_texture_non_power_of_two
#define GLEE_ARB_texture_rectangle          GLEW_ARB_texture_rectangle
#define GLEE_EXT_framebuffer_object         GLEW_EXT_framebuffer_object
#define GLEE_ARB_vertex_buffer_object       GLEW_ARB_vertex_buffer_object
#define GLEE_ARB_pixel_buffer_object        GLEW_ARB_pixel_buffer_object

#define GLEE_ARB_shading_language_100       GLEW_ARB_shading_language_100
#define GLEE_ARB_fragment_shader            GLEW_ARB_fragment_shader
#define GLEE_ARB_vertex_shader              GLEW_ARB_vertex_shader
#define GLEE_ARB_shader_objects             GLEW_ARB_shader_objects
#define GLEE_EXT_geometry_shader4           GLEW_EXT_geometry_shader4
#define GLEE_EXT_gpu_shader4                GLEW_EXT_gpu_shader4


#endif //USE_Qt5
#endif //GLEE2GLEW_H_
