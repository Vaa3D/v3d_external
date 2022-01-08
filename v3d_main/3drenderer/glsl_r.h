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

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) 鈥淰3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,鈥� Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) 鈥淎utomatic reconstruction of 3D neuron structures using a graph-augmented deformable model,鈥� Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/******************************************************************************
glsl.h
Version: 1.0.0_rc5
Last update: 2006/11/14

(c) 2003-2006 by Martin Christen. All Rights reserved.
******************************************************************************/
// minor modified glsl.h for using GLee & pure GL 2.0
// by RZC 2008-11-23

/*!
\page libglslnews What's new in libglsl
\section s_libglslnews What's New
* New in RC5:
\item libglsl is actually a lib again. MT and MTDLL runtime is available. GLEW is included in the library.
\item Supporting Extension: GL_EXT_bindable_uniform (GeForce 8)

* New in RC4:
\item Support for Geometry Shader
\item Shader Model 4: Support for unsigned integers (Vertex Attributes are currently unsupported, but easy to integrate if you actually need them)
\item retrieve variable index with GetUniformLocation
\item All setUniformXXX can use index
\item Added GetProgramObject in class glShader to return the OpenGL Program Object

\todo Support all vertex attribute types
\todo Support glGetActiveAttrib
\todo Support glGetAttribLocation
\todo Support New Matrix Types (OpenGL 2.1)
\todo Make SetUniformXXX / VertexXXX calls template-based

\note
Make sure to check extension "GL_EXT_geometry_shader4" before using Geometry shaders!
*/



#ifndef A_GLSL_H
#define A_GLSL_H


#if (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
typedef char GLchar;
#endif
#include <GL/gl.h>

#include <vector>
#include <iostream>

#define GLSLAPI    // static build


namespace cwc
{
   class glShaderManager;

   //! Shader Object holds the Shader Source, the OpenGL "Shader Object" and provides some methods to load shaders.
   /*!
         \author Martin Christen
         \ingroup GLSL
         \bug This class should be an interface. (pure virtual)
   */
   class GLSLAPI glShaderObject
   {
      friend class glShader;

   public:
                  glShaderObject();
      virtual     ~glShaderObject();

      int         load(char* filename);                     //!< \brief Loads a shader file. \param filename The name of the ASCII file containing the shader. \return returns 0 if everything is ok.\n -1: File not found\n -2: Empty File\n -3: no memory
      void        loadFromMemory(const char* program);      //!< \brief Load program from null-terminated char array. \param program Address of the memory containing the shader program.

      bool        compile(void);                            //!< compile program
      const char*       getCompilerLog(void);                     //!< get compiler messages
      GLint       getAttribLocation(char* attribName);      //!< \brief Retrieve attribute location. \return Returns attribute location. \param attribName Specify attribute name.

   protected:
       int        program_type;  //!< The program type. 1=Vertex Program, 2=Fragment Program, 3=Geometry Progam, 0=none

       GLuint     ShaderObject;  //!< Shader Object >   GLhandleARB for old ARB extension
       GLubyte*   ShaderSource;  //!< ASCII Source-Code

       GLchar*    compiler_log;

       bool       is_compiled;   //!< true if compiled
       bool       _memalloc;     //!< true if memory for shader source was allocated
   };

//-----------------------------------------------------------------------------

   //! \brief Class holding Vertex Shader \ingroup GLSL \author Martin Christen
   class GLSLAPI aVertexShader : public glShaderObject
   {
   public:
                  aVertexShader();  //!< Constructor for Vertex Shader
      virtual     ~aVertexShader();
   };

//-----------------------------------------------------------------------------

   //! \brief Class holding Fragment Shader \ingroup GLSL \author Martin Christen
   class GLSLAPI aFragmentShader : public glShaderObject
   {
   public:
                  aFragmentShader(); //!< Constructor for Fragment Shader
      virtual     ~aFragmentShader();

   };

//-----------------------------------------------------------------------------

   //! \brief Class holding Geometry Shader \ingroup GLSL \author Martin Christen
   class GLSLAPI aGeometryShader : public glShaderObject
   {
   public:
                   aGeometryShader(); //!< Constructor for Geometry Shader
      virtual     ~aGeometryShader();
   };

//-----------------------------------------------------------------------------

   //! \brief Controlling compiled and linked GLSL program. \ingroup GLSL \author Martin Christen
   class GLSLAPI glShader
   {
      friend class glShaderManager;

   public:
                 glShader();
      virtual    ~glShader();
      void       addShader(glShaderObject* ShaderProgram); //!< add a Vertex or Fragment Program \param ShaderProgram The shader object.

      //!< Returns the OpenGL Program Object (only needed if you want to control everything yourself) \return The OpenGL Program Object
      GLuint     GetProgramObject(){return ProgramObject;}

      bool       link(void);                        //!< Link all Shaders
      const char*      getLinkerLog(void);                //!< Get Linker Messages \return char pointer to linker messages. Memory of this string is available until you link again or destroy this class.

      void       begin();                           //!< use Shader. OpenGL calls will go through vertex, geometry and/or fragment shaders.
      void       end();                             //!< Stop using this shader. OpenGL calls will go through regular pipeline.

      // Geometry Shader: Input Type, Output and Number of Vertices out
      void       SetInputPrimitiveType(int nInputPrimitiveType);   //!< Set the input primitive type for the geometry shader
      void       SetOutputPrimitiveType(int nOutputPrimitiveType); //!< Set the output primitive type for the geometry shader
      void       SetVerticesOut(int nVerticesOut);                 //!< Set the maximal number of vertices the geometry shader can output

      GLint       GetUniformLocation(const GLchar *name);  //!< Retrieve Location (index) of a Uniform Variable

      // Submitting Uniform Variables. You can set varname to 0 and specifiy index retrieved with GetUniformLocation (best performance)
      bool       setUniform1f(const GLchar* varname, GLfloat v0, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.
      bool       setUniform2f(const GLchar* varname, GLfloat v0, GLfloat v1, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.
      bool       setUniform3f(const GLchar* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.
      bool       setUniform4f(const GLchar* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, GLint index = -1);  //!< Specify value of uniform variable. \param varname The name of the uniform variable.

      bool       setUniform1i(const GLchar* varname, GLint v0, GLint index = -1);  //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.
      bool       setUniform2i(const GLchar* varname, GLint v0, GLint v1, GLint index = -1); //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.
      bool       setUniform3i(const GLchar* varname, GLint v0, GLint v1, GLint v2, GLint index = -1); //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.
      bool       setUniform4i(const GLchar* varname, GLint v0, GLint v1, GLint v2, GLint v3, GLint index = -1); //!< Specify value of uniform integer variable. \param varname The name of the uniform variable.

      // Note: unsigned integers require GL_EXT_gpu_shader4 (for example GeForce 8800)
      bool       setUniform1ui(const GLchar* varname, GLuint v0, GLint index = -1); //!< Specify value of uniform unsigned integer variable. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform2ui(const GLchar* varname, GLuint v0, GLuint v1, GLint index = -1); //!< Specify value of uniform unsigned integer variable. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform3ui(const GLchar* varname, GLuint v0, GLuint v1, GLuint v2, GLint index = -1); //!< Specify value of uniform unsigned integer variable. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform4ui(const GLchar* varname, GLuint v0, GLuint v1, GLuint v2, GLuint v3, GLint index = -1); //!< Specify value of uniform unsigned integer variable. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.

      // Arrays
      bool       setUniform1fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform2fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform3fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform4fv(const GLchar* varname, GLsizei count, GLfloat *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.

      bool       setUniform1iv(const GLchar* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform2iv(const GLchar* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform3iv(const GLchar* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.
      bool       setUniform4iv(const GLchar* varname, GLsizei count, GLint *value, GLint index = -1); //!< Specify values of uniform array. \param varname The name of the uniform variable.

      bool       setUniform1uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform2uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform3uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.
      bool       setUniform4uiv(const GLchar* varname, GLsizei count, GLuint *value, GLint index = -1); //!< Specify values of uniform array. \warning Requires GL_EXT_gpu_shader4. \param varname The name of the uniform variable.

      bool       setUniformMatrix2fv(const GLchar* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index = -1); //!< Specify values of uniform 2x2 matrix. \param varname The name of the uniform variable.
      bool       setUniformMatrix3fv(const GLchar* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index = -1); //!< Specify values of uniform 3x3 matrix. \param varname The name of the uniform variable.
      bool       setUniformMatrix4fv(const GLchar* varname, GLsizei count, GLboolean transpose, GLfloat *value, GLint index = -1); //!< Specify values of uniform 4x4 matrix. \param varname The name of the uniform variable.

      // Receive Uniform variables:
      void       getUniformfv(const GLchar* varname, GLfloat* values, GLint index = -1); //!< Receive value of uniform variable. \param varname The name of the uniform variable.
      void       getUniformiv(const GLchar* varname, GLint* values, GLint index = -1); //!< Receive value of uniform variable. \param varname The name of the uniform variable.
      void       getUniformuiv(const GLchar* varname, GLuint* values, GLint index = -1); //!< Receive value of uniform variable. \warning Requires GL_EXT_gpu_shader4 \param varname The name of the uniform variable.

      /*! This method simply calls glBindAttribLocation for the current ProgramObject
      \warning NVidia implementation is different than the GLSL standard: GLSL attempts to eliminate aliasing of vertex attributes but this is integral to NVIDIA锟絪 hardware approach and necessary for maintaining compatibility with existing OpenGL applications that NVIDIA customers rely on. NVIDIA锟絪 GLSL implementation therefore does not allow built-in vertex attributes to collide with a generic vertex attributes that is assigned to a particular vertex  attribute index with glBindAttribLocation. For example, you should not use gl_Normal (a built-in vertex attribute) and also use glBindAttribLocation to bind a generic vertex attribute named "whatever" to vertex attribute index 2 because gl_Normal aliases to index 2.
      \verbatim
      gl_Vertex                0
      gl_Normal                2
      gl_Color                 3
      gl_SecondaryColor        4
      gl_FogCoord              5
      gl_MultiTexCoord0        8
      gl_MultiTexCoord1        9
      gl_MultiTexCoord2       10
      gl_MultiTexCoord3       11
      gl_MultiTexCoord4       12
      gl_MultiTexCoord5       13
      gl_MultiTexCoord6       14
      gl_MultiTexCoord7       15
      \endverbatim

      \param index Index of the variable
      \param name Name of the attribute.
      */
      void        BindAttribLocation(GLint index, const GLchar* name);

      //GLfloat
      bool        setVertexAttrib1f(GLuint index, GLfloat v0); 				                         //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component
      bool        setVertexAttrib2f(GLuint index, GLfloat v0, GLfloat v1); 								 //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component
      bool        setVertexAttrib3f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);					 //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component \param v2 value of the attribute component
      bool        setVertexAttrib4f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);  //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component \param v2 value of the attribute component \param v3 value of the attribute component

      //GLdouble
      bool        setVertexAttrib1d(GLuint index, GLdouble v0);                                         //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component
      bool        setVertexAttrib2d(GLuint index, GLdouble v0, GLdouble v1);                            //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component
      bool        setVertexAttrib3d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2);               //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component \param v2 value of the attribute component
      bool        setVertexAttrib4d(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);  //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component \param v2 value of the attribute component \param v3 value of the attribute component

      //GLshort
      bool        setVertexAttrib1s(GLuint index, GLshort v0);                                      //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component
      bool        setVertexAttrib2s(GLuint index, GLshort v0, GLshort v1);                          //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component
      bool        setVertexAttrib3s(GLuint index, GLshort v0, GLshort v1, GLshort v2);              //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component \param v2 value of the attribute component
      bool        setVertexAttrib4s(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3);  //!< Specify value of attribute. \param index The index of the vertex attribute. \param v0 value of the attribute component \param v1 value of the attribute component \param v2 value of the attribute component \param v3 value of the attribute component

      // Normalized Byte (for example for RGBA colors)
      bool        setVertexAttribNormalizedByte(GLuint index, GLbyte v0, GLbyte v1, GLbyte v2, GLbyte v3); //!< Specify value of attribute. Values will be normalized.

      //GLint (Requires GL_EXT_gpu_shader4)
      bool        setVertexAttrib1i(GLuint index, GLint v0); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool        setVertexAttrib2i(GLuint index, GLint v0, GLint v1); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool        setVertexAttrib3i(GLuint index, GLint v0, GLint v1, GLint v2); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.
      bool        setVertexAttrib4i(GLuint index, GLint v0, GLint v1, GLint v2, GLint v3); //!< Specify value of attribute. Requires GL_EXT_gpu_shader4.

      //GLuint (Requires GL_EXT_gpu_shader4)
      bool        setVertexAttrib1ui(GLuint index, GLuint v0); //!< Specify value of attribute. \warning Requires GL_EXT_gpu_shader4. \param v0 value of the first component
      bool        setVertexAttrib2ui(GLuint index, GLuint v0, GLuint v1); //!< Specify value of attribute. \warning Requires GL_EXT_gpu_shader4.
      bool        setVertexAttrib3ui(GLuint index, GLuint v0, GLuint v1, GLuint v2); //!< Specify value of attribute. \warning Requires GL_EXT_gpu_shader4.
      bool        setVertexAttrib4ui(GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3); //!< Specify value of attribute. \warning Requires GL_EXT_gpu_shader4.

      //! Enable this Shader:
	   void        enable(void) //!< Enables Shader (Shader is enabled by default)
	   {
	      _bEnabled = true;
	   }

      //! Disable this Shader:
	   void        disable(void) //!< Disables Shader.
	   {
	      _bEnabled = false;
	   }

   protected:
      void        manageMemory(void){_bManageMemory = true;}
      void        UsesGeometryShader(bool bYesNo){ _bUsesGeometryShader = bYesNo;}

   private:
      GLuint      ProgramObject;                      // GLProgramObject


      GLchar*  	  linker_log;
      bool        is_linked;
      std::vector<glShaderObject*> ShaderList;       // List of all Shader Programs

      bool        _bManageMemory;
      bool        _bEnabled;

      bool        _bUsesGeometryShader;

      int         _nInputPrimitiveType;
      int         _nOutputPrimitiveType;
      int         _nVerticesOut;

   };

//-----------------------------------------------------------------------------

	//! To simplify the process loading/compiling/linking shaders this high level interface to simplify setup of a vertex/fragment shader was created. \ingroup GLSL \author Martin Christen
   class GLSLAPI glShaderManager
   {
   public:
       glShaderManager();
       virtual ~glShaderManager();

       // Regular GLSL (Vertex+Fragment Shader)
       glShader* loadfromFile(char* vertexFile, char* fragmentFile);    //!< load vertex/fragment shader from file. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part. \param vertexFile Vertex Shader File. \param fragmentFile Fragment Shader File.
       glShader* loadfromMemory(const char* vertexMem, const char* fragmentMem); //!< load vertex/fragment shader from memory. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part.

       // With Geometry Shader (Vertex+Geomentry+Fragment Shader)
       glShader* loadfromFile(char* vertexFile, char* geometryFile, char* fragmentFile); //!< load vertex/geometry/fragment shader from file. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part. \param vertexFile Vertex Shader File. \param geometryFile Geometry Shader File \param fragmentFile Fragment Shader File.
       glShader* loadfromMemory(const char* vertexMem, const char* geometryMem, const char* fragmentMem); //!< load vertex/geometry/fragment shader from memory. If you specify 0 for one of the shaders, the fixed function pipeline is used for that part.

       glShader* makeShader(aVertexShader* tVertexShader, aGeometryShader* tGeometryShader, aFragmentShader* tFragmentShader); // RZC 081125 add
       const char* getInfoLog(void) { return info_log; }; // RZC 081125 add

       void      SetInputPrimitiveType(int nInputPrimitiveType);    //!< Set the input primitive type for the geometry shader \param nInputPrimitiveType Input Primitive Type, for example GL_TRIANGLES
       void      SetOutputPrimitiveType(int nOutputPrimitiveType);  //!< Set the output primitive type for the geometry shader \param nOutputPrimitiveType Output Primitive Type, for example GL_TRIANGLE_STRIP
       void      SetVerticesOut(int nVerticesOut);                  //!< Set the maximal number of vertices the geometry shader can output \param nVerticesOut Maximal number of output vertices. It is possible to output less vertices!

       bool      free(glShader* o); //!< Remove the shader and free the memory occupied by this shader.

   private:
	   char                  info_log[1000]; // RZC 081125 add

       std::vector<glShader*>  _shaderObjectList;
       int                     _nInputPrimitiveType;
       int                     _nOutputPrimitiveType;
       int                     _nVerticesOut;
   };

//-----------------------------------------------------------------------------
// Global functions to initialize OpenGL Extensions and check for GLSL and
// OpenGL2. Also functions to check if Shader Model 4 is available and if
// Geometry Shaders are supported.
bool GLSLAPI InitOpenGLExtensions(void); //!< Initialize OpenGL Extensions (using glew) \ingroup GLSL
bool GLSLAPI HasGLSLSupport(void);       //!< Returns true if OpenGL Shading Language is supported. (This function will return a GLSL version number in a future release) \ingroup GLSL
bool GLSLAPI HasOpenGL2Support(void);    //!< Returns true if OpenGL 2.0 is supported. This function is deprecated and shouldn't be used anymore. \ingroup GLSL \deprecated
bool GLSLAPI HasGeometryShaderSupport(void); //!< Returns true if Geometry Shaders are supported. \ingroup GLSL
bool GLSLAPI HasShaderModel4(void); //!< Returns true if Shader Model 4 is supported. \ingroup GLSL

// these function names are deprecated, just here for backwards
// compatibility. It is very likely they will be removed in a future version
#define initGLExtensions InitOpenGLExtensions
#define checkGLSL HasGLSLSupport
#define checkGL2  HasOpenGL2Support
//----------------------------------------------------------------------------

};

#endif // A_GLSL_H

