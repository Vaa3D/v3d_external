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




/*
 * renderer_gl2.cpp
 *
 *  Created on: Nov 20, 2008
 *      Author: ruanzongcai
 */


// #include "version_control.h"
// #if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
// #include <QtGui>
// //#include <QtANGLE\GLES2\gl2.h>
// #include <QtANGLE\GLES3\gl3.h>
// #endif // USE_Qt5_VS2015_Win7_81 || USE_Qt5_VS2015_Win10_10_14393

//#include "GLee2glew.h" ////2020-2-10
//#include "glsl_r.h"

#include "renderer_gl2.h"



// if error then just warning
#define ERROR_MessageBox(title, type, what) { \
    QMessageBox::critical( 0, title, QObject::tr("%1: OUT OF MEMORY or GL ERROR.\n---%2 exception: %3")\
            .arg(title).arg(type).arg(what) + "\n\n" + \
        QObject::tr("3D View: Please close some images or views to release memory, then try again.\n\n") ); \
}

#define CATCH_handler( func_name ) \
    catch (std::exception& e) { \
        \
        qDebug("    *** std exception occurred in %s", func_name); \
        ERROR_MessageBox(func_name, "std", e.what()); \
        \
    } catch (const char* str) { \
        \
        qDebug("    *** GLSL exception occurred in %s", func_name); \
        ERROR_MessageBox(func_name, "GLSL", str); \
        \
    } catch (...) { \
        \
        ERROR_MessageBox( func_name, "UNKOWN", "unknown exception" ); \
        \
    }



Renderer_gl2::Renderer_gl2(void* widget)
    : Renderer_gl1(widget)
{
    qDebug("   Renderer_gl2::Renderer_gl2");
    init_members();

    tryTexStream = (supported_PBO()? 1 : 0);
    tryVolShader = (supported_GLSL()? 1 : 0);

    //tryObjShader = (supported_GLSL()? 1 : 0);
}

Renderer_gl2::~Renderer_gl2()
{
    qDebug("   Renderer_gl2::~Renderer_gl2");

    cleanShader();
    cleanTexStreamBuffer();
}

//////////////////////////////////////////////////////////////


void Renderer_gl2::initialize(int version)
{
    qDebug("   Renderer_gl2::initialize (%d)", version);

    try {

        Renderer_gl1::initialize();
        if (version<2) return;

        loadShader();

    } CATCH_handler( "Renderer_gl2::initialize" );
}


QString resourceTextFile(QString filename)
{
    //QFile inputFile(":/subdir/input.txt");
    qDebug() << "Load shader: " << filename;

    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly)==false)
        qDebug() << "   *** ERROR in Load shader: " << filename;

    QTextStream in(&inputFile);
    QString line = in.readAll();
    inputFile.close();
    return line;
}

static void linkGLShader(cwc::glShaderManager& SMgr,
        cwc::glShader*& shader, //output
        const char* vertex, const char* fragment)
{
    glGetError(); // clear error
    shader = SMgr.loadfromMemory(vertex, fragment);
    if (shader==0)
    {
       qDebug() << "Renderer_gl2::init:  Error Loading, compiling or linking shader\n";
       throw SMgr.getInfoLog();
    }
}

//#define TEX_LOD  1 // do not works in pixel shader, only works in vertex shader
#define IF_VOL_SHADER  (tryVolShader && shader && !b_selecting)
#define IF_OBJ_SHADER  (tryObjShader && shader && !b_selecting)


void Renderer_gl2::drawObj()
{
    shader = shaderObj;
    if (IF_OBJ_SHADER )// && polygonMode==3)
    {
        shader->begin(); //must before setUniform
    }

    ///////////////////////////
    Renderer_gl1::drawObj();
    ///////////////////////////

    if (IF_OBJ_SHADER )//&& polygonMode==3)
    {
        shader->end();
    }
    shader = 0;
}

void Renderer_gl2::loadShader()
{
    cleanShader(); //090705
    qDebug("   Renderer_gl2::loadShader");
    makeCurrent(); //ensure right context when multiple views animation or mouse drop

    try {

        qDebug("+++++++++ shader for Surface Object");
        linkGLShader(SMgr, shaderObj, //0,0);
                //0,
                Q_CSTR(resourceTextFile(":/shader/lighting.txt") + resourceTextFile(":/shader/color_vertex.txt")),
                //Q_CSTR(resourceTextFile(":/shader/vertex_normal.txt")),
                //0);
                Q_CSTR(resourceTextFile(":/shader/lighting.txt") + resourceTextFile(":/shader/obj_fragment.txt")));

        #ifdef TEX_LOD
            QString deftexlod = "#define TEX_LOD \n";
        #else
            QString deftexlod = "#undef TEX_LOD \n";
        #endif

        qDebug("+++++++++ shader for Volume texture2D");
        linkGLShader(SMgr, shaderTex2D,
                0, //Q_CSTR(resourceTextFile(":/shader/color_vertex.txt")),
                Q_CSTR(QString("#undef TEX3D \n") + deftexlod + resourceTextFile(":/shader/tex_fragment.txt")));

        qDebug("+++++++++ shader for Volume texture3D");
        linkGLShader(SMgr, shaderTex3D,
                0, //Q_CSTR(resourceTextFile(":/shader/color_vertex.txt")),
                Q_CSTR(QString("#define TEX3D \n") + deftexlod + resourceTextFile(":/shader/tex_fragment.txt")));

    } CATCH_handler("Renderer_gl2::initialze");
    qDebug("+++++++++ GLSL shader setup finished.");


    glGenTextures(1, &texColormap);
    initColormap();
}

void Renderer_gl2::cleanShader()
{
    qDebug("    Renderer_gl2::cleanShader");
    makeCurrent(); //ensure right context when multiple views animation or mouse drop

    DELETE_AND_ZERO(shaderTex2D);
    DELETE_AND_ZERO(shaderTex3D);
    DELETE_AND_ZERO(shaderObj);

    if (texColormap) {
        glDeleteTextures(1, &texColormap);
        texColormap = 0;
    }
}

void Renderer_gl2::toggleShader()
{
    bool b = (tryVolShader || tryObjShader);
    tryVolShader = !b;
    tryObjShader = !b;
    if (! supported_GLSL())
    {
        qDebug( "	No GL shading language support");
        tryVolShader = 0;
        tryObjShader = 0;
    }
    //qDebug( "	tryShader(vol obj) = %d %d", tryVolShader, tryObjShader);
}
void Renderer_gl2::toggleObjShader()
{
    tryObjShader = !tryObjShader;
    if (! supported_GLSL())
    {
        qDebug( "	No GL shading language support");
        tryObjShader = 0;
    }
    //qDebug( "	tryShader(vol obj) = %d %d", tryVolShader, tryObjShader);
}

void Renderer_gl2::shaderTexBegin(bool stream)
{
    shader = (texture_unit0_3D && !stream)? shaderTex3D : shaderTex2D;

    int format_bgra = (stream && pbo_image_format==GL_BGRA)? 1:0;

    if (IF_VOL_SHADER)
    {
        shader->begin(); //must before setUniform
        shader->setUniform1i("volume",   0); //GL_TEXTURE0
        shader->setUniform1i("colormap", 1); //GL_TEXTURE1

        float n = FILL_CHANNEL-1; // 0-based
        shader->setUniform3f("channel", 0/n, 1/n, 2/n);
        shader->setUniform1i("blend_mode", renderMode);
        shader->setUniform1i("format_bgra", format_bgra);

        // switch to colormap texture
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texColormap);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GLSL will replace TexEnv
        CHECK_GLError_print();

//		glTexImage2D(GL_TEXTURE_2D, // target
//				0, // level
//				GL_RGBA, // texture format
//				256, // width
//				FILL_CHANNEL,   // height
//				0, // border
//				GL_RGBA, // image format
//				GL_UNSIGNED_BYTE, // image type
//				&colormap[0][0]);
        glTexSubImage2D(GL_TEXTURE_2D, // target
                0, // level
                0,0, // offset
                256, // width
                FILL_CHANNEL,   // height
                GL_RGBA, // image format
                GL_UNSIGNED_BYTE, // image type
                &colormap[0][0]);
        CHECK_GLError_print();

        // switch back to volume data texture
        glActiveTextureARB(GL_TEXTURE0_ARB);
    }
}

void Renderer_gl2::shaderTexEnd()
{
    if (IF_VOL_SHADER)
    {
        // off colormap
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);
        glActiveTextureARB(GL_TEXTURE0_ARB);

        shader->end();
    }
    shader = 0;
}

void Renderer_gl2::equAlphaBlendingProjection()
{
    if (! tryVolShader)
    {
        Renderer_gl1::equAlphaBlendingProjection();
        return;
    }
    glBlendEquationEXT(GL_FUNC_ADD_EXT);/////////////////////
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);	// alpha multiplied in shader
}
void Renderer_gl2::equMaxIntensityProjection()
{
    if (! tryVolShader)
    {
        Renderer_gl1::equMaxIntensityProjection();
        return;
    }
    glBlendEquationEXT(GL_MAX_EXT);//////////////////////////
}

void Renderer_gl2::equMinIntensityProjection()
{
    if (! tryVolShader)
    {
        Renderer_gl1::equMinIntensityProjection();
        return;
    }
    glBlendEquationEXT(GL_MIN_EXT);//////////////////////////
}

void Renderer_gl2::equCrossSection()
{
    if (! tryVolShader)
    {
        Renderer_gl1::equCrossSection();
        return;
    }
    glBlendEquationEXT(GL_FUNC_ADD_EXT);/////////////////////
    glBlendColorEXT(1, 1, 1, 1-CSbeta);
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA); // constant Alpha
}

void Renderer_gl2::initColormap()
{
    qDebug("   Renderer_gl2::initColormap");

    for (int ch=0; ch<N_CHANNEL; ch++)
    {
        for (int i=0; i<256; i++) // intensity
        {
            for (int j=0; j<3; j++)  colormap[ch][i].c[j] = (ch==j)*255;	colormap[ch][i].a = i;
            //for (int j=0; j<3; j++)  colormap[ch][i].c[j] = (ch!=j)*i;	colormap[ch][i].a = i;
        }

        for (int j=0; j<4; j++) // RGBA
        {
            colormap_curve[ch][j].clear();
            int y;
            y = colormap[ch][0].c[j];	   set_colormap_curve(colormap_curve[ch][j],  0.0,  y);
            y = colormap[ch][255].c[j];    set_colormap_curve(colormap_curve[ch][j],  1.0,  y);

        //	qDebug() << QString("[%1][%2]").arg(ch).arg(j) <<  colormap_curve[ch][j];
        }
    }

    glBindTexture(GL_TEXTURE_2D, texColormap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter

    // load texture
    glTexImage2D(GL_TEXTURE_2D, // target
            0, // level
            GL_RGBA, // texture format
            256, // width
            FILL_CHANNEL,   // height
            0, // border
            GL_RGBA, // image format
            GL_UNSIGNED_BYTE, // image type
            &colormap[0][0]);
            //NULL); ////////////////////////////////////// load ON DEMAND when drawing
    CHECK_GLError_print();

}

void Renderer_gl2::applyColormapToImage() // at most the first 3 channels
{
    qDebug("   Renderer_gl2::applyColormapToImage");

#ifndef test_main_cpp
    My4DImage* curImg = v3dr_getImage4d(_idep);
    if (curImg && data4dp && dim4>0)
    {
        qDebug("	dim[%dx%dx%d_%d]", dim1,dim2,dim3,dim4);
        int ix, iy, iz;
        for (iz = 0; iz < dim3; iz++)
            for (iy = 0; iy < dim2; iy++)
                for (ix = 0; ix < dim1; ix++)
                {
                    RGB8 oldC = getRGB3dUINT8 (data4dp, dim1, dim2, dim3, dim4,  ix, iy, iz);

                    RGB8 newC = lookupColormap(oldC, 1); //OP_ADD

                    setRGB3dUINT8 (data4dp, dim1, dim2, dim3, dim4,  ix, iy, iz,  newC);
                }

        curImg->updateViews();
    }
#endif
}

////////////////////////////////////////////////////
// shader for cross-section mode in tex_fragment.txt
//
//vec3 aC1 = C1.rgb * C1.a;
//vec3 aC2 = C2.rgb * C2.a;
//vec3 aC3 = C3.rgb * C3.a;
//float Amean = (C1.a + C2.a + C3.a)/3.0;
//			// pow((C1.a * C2.a * C3.a), 1.0/3.0);
//float Amax	= max(C1.a, max(C2.a, C3.a));
//
//vec4 oColor;
//if (mode==0) // cross-section
//{
//	float Axsec = Amean;
//	oColor.rgb = (aC1 + aC2 + aC3);
//	oColor.a = Axsec;
//}

#define OP_MAX	0
#define OP_ADD	1

RGB8 Renderer_gl2::lookupColormap(RGB8 inC, int op)
{
    #define R(k,j)	(colormap[k][j].r/255.0)
    #define G(k,j)	(colormap[k][j].g/255.0)
    #define B(k,j)	(colormap[k][j].b/255.0)
    #define A(k,j)	(colormap[k][j].a/255.0)

    #define AR(k,j)	(A(k,j)*R(k,j))
    #define AG(k,j)	(A(k,j)*G(k,j))
    #define AB(k,j)	(A(k,j)*B(k,j))

    int i1 = inC.r;
    int i2 = inC.g;
    int i3 = inC.b;

    float o1,o2,o3; // o1=o2=o3=0;

    if (op==OP_MAX)
    {
        o1 = MAX(AR(1,i1), MAX(AR(2,i2), AR(3,i3)));
        o2 = MAX(AG(1,i1), MAX(AG(2,i2), AG(3,i3)));
        o3 = MAX(AB(1,i1), MAX(AB(2,i2), AB(3,i3)));

    }
    else if (op==OP_ADD)
    {
        o1 = AR(1,i1) + AR(2,i2) + AR(3,i3);
        o2 = AG(1,i1) + AG(2,i2) + AG(3,i3);
        o3 = AB(1,i1) + AB(2,i2) + AB(3,i3);
    }

    RGB8 oC;
    oC.r = o1*255;
    oC.g = o2*255;
    oC.b = o3*255;
    return oC;
}

bool Renderer_gl2::supported_TexStream()
{
    if (imageT>1)
    {
        qDebug( "		Time series is NOT supported by texture stream!");
        tryTexStream = 0;
        return false;
    }
//	if (sizeof(void*)<8)
//	{
//		qDebug( "		32-bit system is NOT supported by texture stream!");
//		tryTexStream = 0;
//		return false;
//	}
    if (!supported_PBO())
    {
        qDebug( "		ARB_pixel_buffer_object 	NOT supported !");
        tryTexStream = 0;
        return false;
    }
    if (!supported_GLSL())
    {
        qDebug( "		ARB_shading_language_100 	NOT supported !");
        tryTexStream = 0;
        return false;
    }
    return true;
}

#define BIND_UNPACK_PBO(pbo)  glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo)


void Renderer_gl2::cleanTexStreamBuffer()
{
    qDebug("    Renderer_gl2::cleanStreamBuffer");
    makeCurrent(); //ensure right context when multiple views animation or mouse drop

    // release PBO object
    BIND_UNPACK_PBO(0);
    if (pboZ) {
        glDeleteBuffersARB(1, &pboZ);
        pboZ = 0;
    }
    if (pboY) {
        glDeleteBuffersARB(1, &pboY);
        pboY = 0;
    }
    if (pboX) {
        glDeleteBuffersARB(1, &pboX);
        pboX = 0;
    }
    tex_stream_buffer = pboZ>0; //#################

    DELETE_AND_ZERO(rgbaBuf_Yzx);
    DELETE_AND_ZERO(rgbaBuf_Xzy);
}

const bool use_pboZ_only = false;

void  Renderer_gl2::setupTexStreamBuffer()
{
    cleanTexStreamBuffer();
    qDebug("   Renderer_gl2::setupStreamBuffer");
    makeCurrent(); //ensure right context when multiple views animation or mouse drop

    //MESSAGE_ASSERT( !tryTexCompress && !tryTex3D && imageT<=1 );
    try
    {
        DELETE_AND_ZERO(rgbaBuf_Yzx);
        DELETE_AND_ZERO(rgbaBuf_Xzy);
    //	rgbaBuf_Yzx = new RGBA8 [imageX*imageY*imageZ];  // this direction copy does not help for speed
    //	_copyYzxFromZyx( rgbaBuf_Yzx, rgbaBuf,  imageX, imageY, imageZ);
        rgbaBuf_Xzy = new RGBA8 [imageX*imageY*imageZ];
        _copyXzyFromZyx( rgbaBuf_Xzy, rgbaBuf,  imageX, imageY, imageZ);
    }
    catch(...)
    {
        qDebug("	setupTexStreamBuffer: Out of memory. It may cause rendering speed down!");
    }
    fillX = _getTexFillSize(imageX);
    fillY = _getTexFillSize(imageY);
    fillZ = _getTexFillSize(imageZ);


    //091012: 1 common PBO is not faster than switch 3 PBOs
    BIND_UNPACK_PBO(0);
    glGenBuffersARB(1, &pboZ);
    glGenBuffersARB(1, &pboY);
    glGenBuffersARB(1, &pboX);
    tex_stream_buffer = pboZ>0; //##################

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      //// 4-byte pixel alignment image for good speed
    pbo_texture_format = GL_RGBA8;
    //pbo_texture_format = GL_RGB8;				//// no help for speed
    //pbo_image_format = GL_RGBA;
    pbo_image_format = GL_BGRA;  					//// only BGRA format is speeded up by PBO
    pbo_image_type = GL_UNSIGNED_BYTE;				//// no care about big-endian or little-endian
    //pbo_image_type = GL_UNSIGNED_INT_8_8_8_8_REV;
    // When using GL_UNSIGNED_INT_8_8_8_8_REV, the OpenGL implementation expects to find data in byte order ARGB on big-endian systems, but BGRA on little-endian systems.
    // for GeForce 8800 GT 512M PCIE x16, 1GB RGB8 image, speed is 0.3hz(no compression, no pbo), 0.6hz(pbo RGBA), 1.1hz(pbo BGRA), 2.2hz(compression, no pbo)

    int max_w = MAX(fillX, MAX(fillY,fillZ));
    for (int stack_i=1; stack_i<=3; stack_i++)
    {
        GLuint pbo = 0, tex = 0;
        int w = 0, h =0;
        int size = 0;

        switch (stack_i)
        {
        case 1: //Z[y][x]
            pbo = pboZ;
            tex = Ztex_list[0];
            w = fillX, h = fillY;
            break;
        case 2: //Y[z][x]
            pbo = pboY;
            tex = Ytex_list[0];
            w = fillX, h = fillZ;
            break;
        case 3: //X[z][y]
            pbo = pboX;
            tex = Xtex_list[0];
            w = fillY, h = fillZ;
            break;
        }

        //MUST pre-allocated PBO memory size not less than used, otherwise report invalid operation
        size = w*h*4;
        if (use_pboZ_only)
        {
            size = max_w*max_w*4;
            pbo = pboZ; // use 1 common PBO
        }
        ///////////////////////////////////////////
        BIND_UNPACK_PBO(pbo);
        glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, size, NULL, GL_STREAM_DRAW);

        glBindTexture(GL_TEXTURE_2D, tex);
        setTexParam2D();
        glTexImage2D(GL_TEXTURE_2D, // target
            0, // level
            pbo_texture_format, // texture format
            w, // width
            h, // height
            0, // border
            pbo_image_format, // image format
            pbo_image_type, // image type
            NULL);
        CHECK_GLErrorString_throw(); // can throw const char* exception, RZC 080925
    }
    BIND_UNPACK_PBO(0);
}

void Renderer_gl2::setupStackTexture(bool bfirst) {
    qDebug() << "Renderer_gl2::setupStackTexture() start";
    Renderer_gl1::setupStackTexture(bfirst);
}


void Renderer_gl2::_streamTex(int stack_i, int slice_i, int step, int slice0, int slice1)
{
    GLuint pbo = 0, tex = 0;
    RGBA8 *pbo_mem = 0, *p_slice = 0;
    int w = 0, h =0;
    int sw = 0, sh = 0;
    int size = 0;

    switch (stack_i)
    {
    case 1: //Z[y][x]
        pbo = pboZ;
        tex = Ztex_list[0];
        p_slice = Zslice_data;
        w = fillX, h = fillY;
        sw = COPY_X, sh = COPY_Y;
        break;
    case 2: //Y[z][x]
        pbo = pboY;
        tex = Ytex_list[0];
        p_slice = Yslice_data;
        w = fillX, h = fillZ;
        sw = COPY_X, sh = COPY_Z;
        break;
    case 3: //X[z][y]
        pbo = pboX;
        tex = Xtex_list[0];
        p_slice = Xslice_data;
        w = fillY, h = fillZ;
        sw = COPY_Y, sh = COPY_Z;
        break;
    }

    //sw = w; sh =h;
    size = sw*sh*4;
    if (use_pboZ_only)
    {
        pbo = pboZ; // use 1 common PBO
    }
    if (!pbo || !tex) return;

    MESSAGE_ASSERT(imageX>=realX && imageY>=realY && imageZ>=realZ);
    MESSAGE_ASSERT(COPY_X>=realX && COPY_Y>=realY && COPY_Z>=realZ);
//	sw+=1, sh+=1;  // to get rid of artifacts
//	if (sw>w) sw = w;
//	if (sh>h) sh = h;

    glBindTexture(GL_TEXTURE_2D, tex);
    BIND_UNPACK_PBO(pbo);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, size, NULL, GL_STREAM_DRAW);
    CHECK_GLError_print();

    pbo_mem = (RGBA8*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
    CHECK_GLError_print();
    if (pbo_mem)
    {
        //memset(pbo_mem, slice_i, size);
        //memcpy(pbo_mem, rgbaBuf + slice_i, size);
        _copySliceFromStack(rgbaBuf, imageX,imageY,imageZ,  pbo_mem, sw,  stack_i, slice_i,  rgbaBuf_Yzx, rgbaBuf_Xzy);

        // Unmap the texture image buffer & Start DMA transfer
        glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
        CHECK_GLError_print();
    }
    else
    {
        //memset(p_slice, 0, size);
        _copySliceFromStack(rgbaBuf, imageX,imageY,imageZ,  p_slice, sw,  stack_i, slice_i,  rgbaBuf_Yzx, rgbaBuf_Xzy);
    }

    setTexParam2D(); //100809
    glTexSubImage2D(GL_TEXTURE_2D, // target
        0, // level
        0,0,  // offset
        sw, // width
        sh, // height
        pbo_image_format, // image format
        pbo_image_type, // image type
        (pbo_mem)? 0    // PBO offset
            : p_slice); // CPU memory
    CHECK_GLError_print();
}

void Renderer_gl2::_streamTex_end()
{
    BIND_UNPACK_PBO(0);
}

bool Renderer_gl2::_streamTex_ready()
{
    // tex_stream_buffer is set only when createStreamBuffer is successful.
    // tryTexStream: -1--resident, 0--off, 1--mixed, 2--force
    // beStill: means no user input a while
    bool need_stream = (tryTexStream==2)
                    || (tryTexStream==1 && beStill())
                    || (renderMode==rmCrossSection); //20110709
    return  (tex_stream_buffer && need_stream);
}

void Renderer_gl2::toggleTexStream()
{
    tryTexStream = !(tryTexStream >0); // -1--resident, 0--off, 1--mixed, 2--force
    //qDebug( "	tryTexStream = %d", tryTexStream);
    try	{
        PROGRESS_DIALOG( ((tryTexStream >0)? "Try Texture Stream": "No Texture Stream"), widget);
        PROGRESS_PERCENT(30);

        if (tryTexStream<=0)
        {
            cleanTexStreamBuffer();
            qDebug("	toggleTexStream: %s", try_vol_state());
        }
        else loadVol();

        PROGRESS_PERCENT(100);
    } CATCH_handler( "Renderer_gl2::toggleTexStream" );
}



