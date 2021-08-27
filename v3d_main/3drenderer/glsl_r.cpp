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




/********************************************************************
glsl.cpp
Version: 1.0.0_rc5
Last update: 2006/11/12 (Geometry Shader Support)

(c) 2003-2006 by Martin Christen. All Rights reserved.
*********************************************************************/
// minor modified glsl.cpp for using GLee & pure GL 2.0, and turn off log display when no error occur.
// by RZC 2008-11-23

#include "GLee2glew.h" //Put here Because GL extension loading lib cannot work with the Angle based build of Qt5.

#include "glsl_r.h"


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <string.h>

using namespace std;
using namespace cwc;

bool useGLSL = false;
bool extensions_init = false;
bool bGeometryShader = false;
bool bGPUShader4 = false;

//-----------------------------------------------------------------------------
/*! \mainpage
\section s_intro Introduction
This is libglsl - a collection of helper classes to load, compile, link and activate shaders
written in the OpenGL Shading language. Vertex Shaders, Geometry Shaders and Fragment shaders
are supported (if the hardware is capable of supporting them, of course).

Version info: \ref s_libglslnews

\section s_examples Examples

\subsection Loading Vertex and Fragment Shader using Shader Manager.

\verbatim
Initialization:
   glShaderManager SM;
   glShader *shader = SM.loadfromFile("test.vert","test.frag");
   if (shader==0)
      cout << "Error Loading, compiling or linking shader\n";

Render:
   shader->begin();
   shader->setUniform1f("MyFloat", 1.123);
     glutDrawSolidSphere(1.0);
   shader->end();
\endverbatim

\subsection geom_shader Geometry Shader
The easiest way to use Geometry Shaders is through the
Shadermanager.
Initialization:
\verbatim
 SM.SetInputPrimitiveType(GL_TRIANGLES);
 SM.SetOutputPrimitiveType(GL_TRIANGLE_STRIP);
 SM.SetVerticesOut(3);
 glShader *shader = SM.loadfromFile("test.vert","test.geom","test.frag");
\endverbatim

*/

namespace cwc
{
//-----------------------------------------------------------------------------
// Error, Warning and Info Strings
const char* aGLSLStrings[] = {
        "[cwc e00] GLSL is not available!",
        "[cwc e01] Not a valid program object!",
        "[cwc e02] Not a valid object!",
        "[cwc e03] Out of memory!",
        "[cwc e04] Unknown compiler error!",
        "[cwc e05] Linker log is not available!",
        "[cwc e06] Compiler log is not available!",
        "[cwc Empty]"
        };
//-----------------------------------------------------------------------------

// GL ERROR CHECK
inline int CheckGLError(const char *file, int line)
{
	GLenum glErr;
	while ((glErr = glGetError()) != GL_NO_ERROR)
	{
		const GLubyte* sError = gluErrorString(glErr);
		if (sError)
		cerr << "GL Error #" << glErr << "(" << sError << ") " << " in File " << file << " at line: " << line << endl;
		else
		cerr << "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line << endl;
	}
	return glErr;
}
#define CHECK_GL_ERROR()  cwc::CheckGLError(__FILE__, __LINE__)


//-----------------------------------------------------------------------------
   bool InitOpenGLExtensions(void)
   {
      if (extensions_init) return true;
      extensions_init = true;

      GLeeInit();

      //cout << "OpenGL Vendor: " << (char*) glGetString(GL_VENDOR) << "\n";
      //cout << "OpenGL Renderer: " << (char*) glGetString(GL_RENDERER) << "\n";
      //cout << "OpenGL Version: " << (char*) glGetString(GL_VERSION) << "\n";
      //cout << "OpenGL Extensions:\n" << (char*) glGetString(GL_EXTENSIONS) << "\n\n";

      HasGLSLSupport();

      return true;
   }


   bool HasGLSLSupport(void)
   {
      bGeometryShader = HasGeometryShaderSupport();
      bGPUShader4     = HasShaderModel4();

      if (useGLSL) return true;  // already initialized and GLSL is available
      useGLSL = true;

      if (!extensions_init) InitOpenGLExtensions();  // extensions were not yet initialized!!


      if (GLEE_VERSION_2_0)
      {
         cout << "OpenGL 2.0 (or higher) is available!\n";
      }
      else if (GLEE_VERSION_1_5)
      {
         cout << "OpenGL 1.5 core functions are available\n";
      }
      else if (GLEE_VERSION_1_4)
      {
         cout << "OpenGL 1.4 core functions are available\n";
      }
      else if (GLEE_VERSION_1_3)
      {
         cout << "OpenGL 1.3 core functions are available\n";
      }
      else if (GLEE_VERSION_1_2)
      {
         cout << "OpenGL 1.2 core functions are available\n";
      }

      if (! GLEE_ARB_fragment_shader)
      {
          cout << "[WARNING] GL_ARB_fragment_shader extension is not available!\n";
          useGLSL = false;
      }

      if (! GLEE_ARB_vertex_shader)
      {
          cout << "[WARNING] GL_ARB_vertex_shader extension is not available!\n";
          useGLSL = false;
      }

      if (! GLEE_ARB_shader_objects)
      {
          cout << "[WARNING] GL_ARB_shader_objects extension is not available!\n";
          useGLSL = false;
      }

      if (useGLSL)
      {
          cout << "[OK] OpenGL Shading Language is available!\n";
      }
      else
      {
          cout << "[FAILED] OpenGL Shading Language is not available...\n";
      }

      return useGLSL;
   }


   bool HasOpenGL2Support(void)
   {
      if (!extensions_init) InitOpenGLExtensions();

      return (GLEE_VERSION_2_0 >0);
   }


   bool HasGeometryShaderSupport(void)
   {
      if (! GLEE_EXT_geometry_shader4)
         return false;

      return true;
   }

   bool HasShaderModel4(void)
   {
      if (! GLEE_EXT_gpu_shader4)
         return false;

      return true;
   }
}

//-----------------------------------------------------------------------------

// ************************************************************************
// Implementation of glShader class
// ************************************************************************

glShader::glShader()
{
  InitOpenGLExtensions();
  ProgramObject = 0;
  linker_log = 0;
  is_linked = false;
  _bManageMemory = false;
  _bEnabled = true;

  if (!useGLSL)
  {
     cout << "**ERROR: OpenGL Shading Language is NOT available!" << endl;
  }
  else
  {
      ProgramObject = glCreateProgram();
  }

}

//-----------------------------------------------------------------------------

glShader::~glShader()
{
    if (linker_log!=0) free(linker_log);
    if (useGLSL)
    {
       for (unsigned int i=0;i<ShaderList.size();i++)
       {
            glDetachShader(ProgramObject, ShaderList[i]->ShaderObject);
            CHECK_GL_ERROR(); // if you get an error here, you deleted the Program object first and then
                           // the ShaderObject! Always delete ShaderObjects last!
            if (_bManageMemory) delete ShaderList[i];
       }

       glDeleteShader(ProgramObject);
       CHECK_GL_ERROR();
    }

}

//-----------------------------------------------------------------------------

void glShader::addShader(glShaderObject* ShaderProgram)
{
if (!useGLSL) return;

   if (ShaderProgram==0) return;


   if (!ShaderProgram->is_compiled)
   {
        cout << "**warning** please compile program before adding object! trying to compile now...\n";
        if (!ShaderProgram->compile())
        {
            cout << "...compile ERROR!\n";
            return;
        }
        else
        {
            cout << "...ok!\n";
        }
   }

   ShaderList.push_back(ShaderProgram);

}

//-----------------------------------------------------------------------------

void glShader::SetInputPrimitiveType(int nInputPrimitiveType)
{
   _nInputPrimitiveType = nInputPrimitiveType;
}

void glShader::SetOutputPrimitiveType(int nOutputPrimitiveType)
{
   _nOutputPrimitiveType = nOutputPrimitiveType;
}

void glShader::SetVerticesOut(int nVerticesOut)
{
   _nVerticesOut = nVerticesOut;
}
//-----------------------------------------------------------------------------

bool glShader::link(void)
{
if (!useGLSL) return false;

unsigned int i;

    if (_bUsesGeometryShader)
    {
       glProgramParameteriEXT(ProgramObject, GL_GEOMETRY_INPUT_TYPE_EXT, _nInputPrimitiveType);
       glProgramParameteriEXT(ProgramObject, GL_GEOMETRY_OUTPUT_TYPE_EXT, _nOutputPrimitiveType);
       glProgramParameteriEXT(ProgramObject, GL_GEOMETRY_VERTICES_OUT_EXT, _nVerticesOut);
    }

    if (is_linked)  // already linked, detach everything first
    {
       cout << "**warning** Object is already linked, trying to link again" << endl;
       for (i=0;i<ShaderList.size();i++)
       {
           GLuint so = ShaderList[i]->ShaderObject;
    	   if (so && ProgramObject)  glDetachShader(ProgramObject, so );
            CHECK_GL_ERROR();
       }
    }

    for (i=0;i<ShaderList.size();i++)
    {
        GLuint so = ShaderList[i]->ShaderObject;
        if (so && ProgramObject)  glAttachShader(ProgramObject, so );
        CHECK_GL_ERROR();
        //cout << "attaching ProgramObj [" << i << "] @ 0x" << hex << ShaderList[i]->ProgramObject << " in ShaderObj @ 0x"  << ShaderObject << endl;
    }

    GLint linked = 0; // bugfix Oct-06-2006
    glLinkProgram(ProgramObject);
    CHECK_GL_ERROR();
    glGetProgramiv(ProgramObject, GL_LINK_STATUS, &linked);
    CHECK_GL_ERROR();

    is_linked = linked;
    if (! is_linked)
    {
        cout << "**linker error**\n";
    }

return is_linked;
}

//-----------------------------------------------------------------------------
// Compiler Log: Ausgabe der Compiler Meldungen in String

const char* glShader::getLinkerLog(void)
{
if (!useGLSL) return aGLSLStrings[0];

 GLint blen = 0;    // bugfix Oct-06-2006
 GLsizei slen = 0;  // bugfix Oct-06-2006

 if (ProgramObject==0) return aGLSLStrings[2];
 glGetProgramiv(ProgramObject, GL_INFO_LOG_LENGTH , &blen);
 CHECK_GL_ERROR();

 if (blen > 1)
 {
    if (linker_log!=0)
    {
        free(linker_log);
        linker_log =0;
    }
    if ((linker_log = (GLchar*)malloc(blen)) == NULL)
     {
        printf("ERROR: Could not allocate compiler_log buffer\n");
        return aGLSLStrings[3];
    }

    glGetProgramInfoLog(ProgramObject, blen, &slen, linker_log);
    CHECK_GL_ERROR();

 }
 if (linker_log!=0)
    return (char*) linker_log;
 else
    return aGLSLStrings[5];

 return aGLSLStrings[4];
}

void glShader::begin(void)
{
if (!useGLSL) return;
if (ProgramObject == 0) return;
if (!_bEnabled) return;

    if (is_linked)
    {        
        glUseProgram(ProgramObject);
        CHECK_GL_ERROR();
    }
}

//-----------------------------------------------------------------------------

void glShader::end(void)
{
if (!useGLSL) return;
if (!_bEnabled) return;


    glUseProgram(0);
    CHECK_GL_ERROR();
}

//-----------------------------------------------------------------------------

bool glShader::setUniform1f(const GLchar* varname, GLfloat v0, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index


    glUniform1f(loc, v0);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform2f(const GLchar* varname, GLfloat v0, GLfloat v1, GLint index)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform2f(loc, v0, v1);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform3f(const GLchar* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform3f(loc, v0, v1, v2);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform4f(const GLchar* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform4f(loc, v0, v1, v2, v3);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform1i(const GLchar* varname, GLint v0, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform1i(loc, v0);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform2i(const GLchar* varname, GLint v0, GLint v1, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform2i(loc, v0, v1);


    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform3i(const GLchar* varname, GLint v0, GLint v1, GLint v2, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform3i(loc, v0, v1, v2);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform4i(const GLchar* varname, GLint v0, GLint v1, GLint v2, GLint v3, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform4i(loc, v0, v1, v2, v3);

    return true;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool glShader::setUniform1ui(const GLchar* varname, GLuint v0, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform1uiEXT(loc, v0);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform2ui(const GLchar* varname, GLuint v0, GLuint v1, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform2uiEXT(loc, v0, v1);


    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform3ui(const GLchar* varname, GLuint v0, GLuint v1, GLuint v2, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform3uiEXT(loc, v0, v1, v2);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform4ui(const GLchar* varname, GLuint v0, GLuint v1, GLuint v2, GLuint v3, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform4uiEXT(loc, v0, v1, v2, v3);

    return true;
}
//-----------------------------------------------------------------------------

bool glShader::setUniform1fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform1fv(loc, count, value);

    return true;
}
bool glShader::setUniform2fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform2fv(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform3fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform3fv(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform4fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform4fv(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform1iv(const GLchar* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform1iv(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform2iv(const GLchar* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform2iv(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform3iv(const GLchar* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform3iv(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform4iv(const GLchar* varname, GLsizei count, GLint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform4iv(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform1uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform1uivEXT(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform2uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform2uivEXT(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform3uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform3uivEXT(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniform4uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!bGPUShader4) return false;
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniform4uivEXT(loc, count, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniformMatrix2fv(const GLchar* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniformMatrix2fv(loc, count, transpose, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniformMatrix3fv(const GLchar* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniformMatrix3fv(loc, count, transpose, value);

    return true;
}

//-----------------------------------------------------------------------------

bool glShader::setUniformMatrix4fv(const GLchar* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index)
{
    if (!useGLSL) return false; // GLSL not available
    if (!_bEnabled) return true;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return false;  // can't find variable / invalid index

    glUniformMatrix4fv(loc, count, transpose, value);

    return true;
}

//-----------------------------------------------------------------------------

GLint glShader::GetUniformLocation(const GLchar *name)
{
	GLint loc;

	loc = glGetUniformLocation(ProgramObject, name);
	if (loc == -1)
	{
        cout << "Error: can't find uniform variable \"" << name << "\"\n";
	}
    CHECK_GL_ERROR();
	return loc;
}

//-----------------------------------------------------------------------------

void glShader::getUniformfv(const GLchar* varname, GLfloat* values, GLint index)
{
if (!useGLSL) return;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return;  // can't find variable / invalid index

	glGetUniformfv(ProgramObject, loc, values);

}

//-----------------------------------------------------------------------------

void glShader::getUniformiv(const GLchar* varname, GLint* values, GLint index)
{
    if (!useGLSL) return;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return;  // can't find variable / invalid index

	glGetUniformiv(ProgramObject, loc, values);

}

//-----------------------------------------------------------------------------

void glShader::getUniformuiv(const GLchar* varname, GLuint* values, GLint index)
{
    if (!useGLSL) return;

    GLint loc;
    if (varname)
       loc = GetUniformLocation(varname);
    else
       loc = index;

    if (loc==-1)
       return;  // can't find variable / invalid index

	glGetUniformuivEXT(ProgramObject, loc, values);

}

//-----------------------------------------------------------------------------
void  glShader::BindAttribLocation(GLint index, const GLchar* name)
{
   glBindAttribLocation(ProgramObject, index, name);
}

//-----------------------------------------------------------------------------


bool glShader::setVertexAttrib1f(GLuint index, GLfloat v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib1f(index, v0);

   return true;
}

bool glShader::setVertexAttrib2f(GLuint index, GLfloat v0, GLfloat v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib2f(index, v0, v1);

   return true;
}

bool glShader::setVertexAttrib3f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

    glVertexAttrib3f(index, v0, v1, v2);

    return true;
}

bool glShader::setVertexAttrib4f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib4f(index, v0, v1, v2, v3);

   return true;
}

//-----------------------------------------------------------------------------

bool glShader::setVertexAttrib1d(GLuint index, GLdouble v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib1d(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool glShader::setVertexAttrib2d(GLuint index, GLdouble v0, GLdouble v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib2d(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib3d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib3d(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib4d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib4d(index, v0, v1, v2, v3);

   return true;
}

//-----------------------------------------------------------------------------

bool glShader::setVertexAttrib1s(GLuint index, GLshort v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib1s(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool glShader::setVertexAttrib2s(GLuint index, GLshort v0, GLshort v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib2s(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib3s(GLuint index, GLshort v0, GLshort v1, GLshort v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib3s(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib4s(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib4s(index, v0, v1, v2, v3);

   return true;
}
//----------------------------------------------------------------------------
bool glShader::setVertexAttribNormalizedByte(GLuint index, GLbyte v0, GLbyte v1, GLbyte v2, GLbyte v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!_bEnabled) return true;

   glVertexAttrib4Nub(index, v0, v1, v2, v3);

   return true;
}
//-----------------------------------------------------------------------------

bool glShader::setVertexAttrib1i(GLuint index, GLint v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI1iEXT(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool glShader::setVertexAttrib2i(GLuint index, GLint v0, GLint v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI2iEXT(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib3i(GLuint index, GLint v0, GLint v1, GLint v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI3iEXT(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib4i(GLuint index, GLint v0, GLint v1, GLint v2, GLint v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI4iEXT(index, v0, v1, v2, v3);

   return true;
}
//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib1ui(GLuint index, GLuint v0)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI1uiEXT(index, v0);

   return true;
}

//-----------------------------------------------------------------------------

bool glShader::setVertexAttrib2ui(GLuint index, GLuint v0, GLuint v1)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI2uiEXT(index, v0, v1);

   return true;
}

//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib3ui(GLuint index, GLuint v0, GLuint v1, GLuint v2)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI3uiEXT(index, v0, v1, v2);

   return true;
}
//-----------------------------------------------------------------------------
bool glShader::setVertexAttrib4ui(GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   if (!useGLSL) return false; // GLSL not available
   if (!bGPUShader4) return false;
   if (!_bEnabled) return true;

   glVertexAttribI4uiEXT(index, v0, v1, v2, v3);

   return true;
}
//-----------------------------------------------------------------------------
// ************************************************************************
// Shader Program : Manage Shader Programs (Vertex/Fragment)
// ************************************************************************
glShaderObject::glShaderObject()
{
    InitOpenGLExtensions();
    compiler_log = 0;
    is_compiled = false;
    program_type = 0;
    ShaderObject = 0;
    ShaderSource = 0;
    _memalloc = false;
}

//-----------------------------------------------------------------------------
glShaderObject::~glShaderObject()
{
   if (compiler_log!=0) free(compiler_log);
   if (ShaderSource!=0)
   {
        if (_memalloc)
            delete[] ShaderSource;  // free ASCII Source
   }

   if (is_compiled)
   {
        glDeleteShader(ShaderObject);
        CHECK_GL_ERROR();
   }
}

//-----------------------------------------------------------------------------
unsigned long getFileLength(ifstream& file)
{
    if(!file.good()) return 0;

    unsigned long pos=file.tellg();
    file.seekg(0,ios::end);
    unsigned long len = file.tellg();
    file.seekg(ios::beg);

    return len;
}


//-----------------------------------------------------------------------------
int glShaderObject::load(char* filename)
{
   ifstream file;
	file.open(filename, ios::in);
   if(!file) return -1;

   unsigned long len = getFileLength(file);

   if (len==0) return -2;   // "Empty File"

   if (ShaderSource!=0)    // there is already a source loaded, free it!
   {
      if (_memalloc)
      delete[] ShaderSource;
   }

   ShaderSource = (GLubyte*) new char[len+1];
   if (ShaderSource == 0) return -3;   // can't reserve memory
   _memalloc = true;

   ShaderSource[len] = 0;  // len isn't always strlen cause some characters are stripped in ascii read...
                           // it is important to 0-terminate the real length later, len is just max possible value...
   unsigned int i=0;
   while (file.good())
   {
       ShaderSource[i] = file.get();       // get character from file.
       if (!file.eof())
        i++;
   }

   ShaderSource[i] = 0;  // 0 terminate it.

   file.close();

   return 0;
}

//-----------------------------------------------------------------------------
void glShaderObject::loadFromMemory(const char* program)
{
    if (ShaderSource!=0)    // there is already a source loaded, free it!
    {
        if (_memalloc)
        delete[] ShaderSource;
    }
   _memalloc = false;
   ShaderSource = (GLubyte*) program;

}


// ----------------------------------------------------------------------------
// Compiler Log: Ausgabe der Compiler Meldungen in String
const char* glShaderObject::getCompilerLog(void)
{
if (!useGLSL) return aGLSLStrings[0];

 GLint blen = 0;
 GLsizei slen = 0;


 if (ShaderObject==0) return aGLSLStrings[1]; // not a valid program object

 glGetShaderiv(ShaderObject, GL_INFO_LOG_LENGTH , &blen);
 CHECK_GL_ERROR();

 if (blen > 1)
 {
    if (compiler_log!=0)
    {
        free(compiler_log);
        compiler_log =0;
    }
    if ((compiler_log = (GLchar*)malloc(blen)) == NULL)
     {
        printf("ERROR: Could not allocate compiler_log buffer\n");
        return aGLSLStrings[3];
    }

     glGetShaderInfoLog(ShaderObject, blen, &slen, compiler_log);
     CHECK_GL_ERROR();
     //cout << "compiler_log: \n", compiler_log);
 }
 if (compiler_log!=0)
    return (char*) compiler_log;
 else
   return aGLSLStrings[6];

 return aGLSLStrings[4];
}

// ----------------------------------------------------------------------------
bool glShaderObject::compile(void)
{
	if (!useGLSL) return false;

	is_compiled = false;

	GLint compiled = 0;

  if (ShaderSource==0) return false;

  GLint	length = (GLint) strlen((const char*)ShaderSource);
  glShaderSource(ShaderObject, 1, (const GLchar **)&ShaderSource, &length);
  CHECK_GL_ERROR();

  glCompileShader(ShaderObject);
  CHECK_GL_ERROR();
  glGetShaderiv(ShaderObject, GL_COMPILE_STATUS, &compiled);
  CHECK_GL_ERROR();

  if (compiled) is_compiled=true;

return is_compiled;
}

// ----------------------------------------------------------------------------
GLint glShaderObject::getAttribLocation(char* attribName)
{
   return glGetAttribLocation(ShaderObject, attribName);
}

// ----------------------------------------------------------------------------
aVertexShader::aVertexShader()
{
  program_type = 1;
   if (useGLSL)
   {
       ShaderObject = glCreateShader(GL_VERTEX_SHADER_ARB);
       CHECK_GL_ERROR();
   }
}

// ----------------------------------------------------
aVertexShader::~aVertexShader()
{
}
// ----------------------------------------------------

aFragmentShader::aFragmentShader()
{
    program_type = 2;
    if (useGLSL)
    {
        ShaderObject = glCreateShader(GL_FRAGMENT_SHADER_ARB);
        CHECK_GL_ERROR();
    }
}

// ----------------------------------------------------

aFragmentShader::~aFragmentShader()
{
}


// ----------------------------------------------------

aGeometryShader::aGeometryShader()
{
  program_type = 3;
   if (useGLSL && bGeometryShader)
   {
       ShaderObject = glCreateShader(GL_GEOMETRY_SHADER_EXT);
       CHECK_GL_ERROR();
   }
}

// ----------------------------------------------------

aGeometryShader::~aGeometryShader()
{
}

// ----------------------------------------------------------------------------
// ShaderManager: Easy use of (multiple) Shaders

glShaderManager::glShaderManager()
{
   InitOpenGLExtensions();
   _nInputPrimitiveType = GL_TRIANGLES;
   _nOutputPrimitiveType = GL_TRIANGLE_STRIP;
   _nVerticesOut = 3;

}

glShaderManager::~glShaderManager()
{
   // free objects
   vector<glShader*>::iterator  i=_shaderObjectList.begin();
   while (i!=_shaderObjectList.end())
   {
        //glShader* o = *i;
        i=_shaderObjectList.erase(i);
        //delete o;
   }
}

// ----------------------------------------------------------------------------

void glShaderManager::SetInputPrimitiveType(int nInputPrimitiveType)
{
      _nInputPrimitiveType = nInputPrimitiveType;

}

void glShaderManager::SetOutputPrimitiveType(int nOutputPrimitiveType)
{
      _nOutputPrimitiveType = nOutputPrimitiveType;

}

void glShaderManager::SetVerticesOut(int nVerticesOut)
{
   _nVerticesOut = nVerticesOut;
}

// ----------------------------------------------------------------------------

#define  INFOLOG(s) strncpy(info_log, s, sizeof(info_log))

glShader* glShaderManager::loadfromFile(char* vertexFile, char* fragmentFile)
{
	return loadfromFile(vertexFile, 0, fragmentFile);
}

glShader* glShaderManager::loadfromFile(char* vertexFile, char* geometryFile, char* fragmentFile)
{
   aVertexShader* tVertexShader = 0;
   aFragmentShader* tFragmentShader = 0;
   aGeometryShader* tGeometryShader = 0;

    // load vertex program
   if (vertexFile!=0)
   {
	  tVertexShader = new aVertexShader;
      if (tVertexShader->load(vertexFile) != 0)
      {
        cout << INFOLOG("error: can't load vertex shader!\n");
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
      }
   }

  // Load geometry program
  if (geometryFile!=0)
  {
	  tGeometryShader = new aGeometryShader;
	  if (tGeometryShader->load(geometryFile) != 0)
     {
        cout << INFOLOG("error: can't load geometry shader!\n");
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
     }
  }

  // Load fragment program
  if (fragmentFile!=0)
  {
	  tFragmentShader = new aFragmentShader;
	  if (tFragmentShader->load(fragmentFile) != 0)
     {
        cout << INFOLOG("error: can't load fragment shader!\n");
        delete tVertexShader;
        delete tFragmentShader;
        delete tGeometryShader;
        return 0;
     }
  }

	// Make shader program
	glShader* o;
	o = makeShader(tVertexShader, tGeometryShader, tFragmentShader);

	if (o==0)
	{
	  delete tVertexShader;
	  delete tFragmentShader;
	  delete tGeometryShader;
	  return 0;
	}
	return o;
}
// ----------------------------------------------------------------------------

glShader* glShaderManager::loadfromMemory(const char* vertexMem, const char* fragmentMem)
{
	return loadfromMemory(vertexMem, 0, fragmentMem);
}

glShader* glShaderManager::loadfromMemory(const char* vertexMem, const char* geometryMem, const char* fragmentMem)
{
	aVertexShader* tVertexShader = 0;
	aFragmentShader* tFragmentShader = 0;
	aGeometryShader* tGeometryShader = 0;

	// get vertex program
	if (vertexMem!=0)
	{
		tVertexShader = new aVertexShader;
	   tVertexShader->loadFromMemory(vertexMem);
	}

	// get geometry program
	if (geometryMem!=0)
	{
		tGeometryShader = new aGeometryShader;
	   tGeometryShader->loadFromMemory(geometryMem);
	}

	// get fragment program
	if (fragmentMem!=0)
	{
		tFragmentShader = new aFragmentShader;
	   tFragmentShader->loadFromMemory(fragmentMem);
	}

	// Make shader program
	glShader* o;
	o = makeShader(tVertexShader, tGeometryShader, tFragmentShader);

	if (o==0)
	{
	   delete tVertexShader;
	   delete tFragmentShader;
	   delete tGeometryShader;
	   return 0;
	}
	return o;
}


glShader* glShaderManager::makeShader(aVertexShader* tVertexShader, aGeometryShader* tGeometryShader, aFragmentShader* tFragmentShader)
{
	glShader* o = new glShader();

	if (tGeometryShader)
	{
	  o->UsesGeometryShader(true);
	  o->SetInputPrimitiveType(_nInputPrimitiveType);
	  o->SetOutputPrimitiveType(_nOutputPrimitiveType);
	  o->SetVerticesOut(_nVerticesOut);
	}

	// Compile vertex program
	if (tVertexShader)
		if (!tVertexShader->compile())
		{
			cout << "***COMPILER ERROR (Vertex Shader):\n";
			cout << INFOLOG(tVertexShader->getCompilerLog()) << endl;
			delete o;
			return 0;
		}
		else
		{
//			cout << "***GLSL Compiler Log (Vertex Shader):\n";
//			cout << INFOLOG(tVertexShader->getCompilerLog()) << "\n";
		}

	// Compile geometry program
	if (tGeometryShader)
		if (!tGeometryShader->compile())
		{
			cout << "***COMPILER ERROR (Geometry Shader):\n";
			cout << INFOLOG(tGeometryShader->getCompilerLog()) << endl;
			delete o;
			return 0;
		}
		else
		{
//			cout << "***GLSL Compiler Log (Geometry Shader):\n";
//			cout << INFOLOG(tVertexShader->getCompilerLog()) << "\n";
		}

	// Compile fragment program
	if (tFragmentShader)
		if (!tFragmentShader->compile())
		{
			cout << "***COMPILER ERROR (Fragment Shader):\n";
			cout << INFOLOG(tFragmentShader->getCompilerLog()) << endl;
			delete o;
			return 0;
		}
		else
		{
//			cout << "***GLSL Compiler Log (Fragment Shader):\n";
//			cout << INFOLOG(tFragmentShader->getCompilerLog()) << "\n";
		}

	// Add to object
	if (tVertexShader) o->addShader(tVertexShader);
	if (tGeometryShader) o->addShader(tGeometryShader);
	if (tFragmentShader) o->addShader(tFragmentShader);

	// link
	if (!o->link())
	{
	   cout << "**LINKER ERROR\n";
	   cout << INFOLOG(o->getLinkerLog()) << endl;
	   delete o;
	   return 0;
	}
	else
	{
//		cout << "***GLSL Linker Log:\n";
//		cout << INFOLOG(o->getLinkerLog()) << endl;
	}
	_shaderObjectList.push_back(o);
	o->manageMemory();

	return o;
}


// ----------------------------------------------------------------------------

 bool  glShaderManager::free(glShader* o)
 {
   vector<glShader*>::iterator  i=_shaderObjectList.begin();
   while (i!=_shaderObjectList.end())
   {
        if ((*i)==o)
        {
            _shaderObjectList.erase(i);
            delete o;
            return true;
        }
        i++;
   }
   return false;
 }


