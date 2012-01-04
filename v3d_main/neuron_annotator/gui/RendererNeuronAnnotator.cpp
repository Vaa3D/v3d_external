#include "../3drenderer/renderer_gl2.h"
#include "RendererNeuronAnnotator.h"
#include "../DataFlowModel.h"
#include "../3drenderer/v3dr_common.h"
#include "../3drenderer/v3dr_glwidget.h"
#include "../geometry/Rotation3D.h"

#define NA_USE_VOLUME_TEXTURE_CMB 1

RendererNeuronAnnotator::RendererNeuronAnnotator(void* w)
    : QObject(NULL)
    , Renderer_gl2(w)
    , stereo3DMode(STEREO_OFF)
    , bStereoSwapEyes(false)
    , bShowCornerAxes(true)
    , volumeTexture(NULL)
    , neuronLabelTexture(NULL)
    , neuronVisibilityTexture(NULL)
{
    // qDebug() << "RendererNeuronAnnotator constructor" << this;
    /*
    neuronMask=0;
    texture3DSignal=0;
    texture3DBackground=0;
    texture3DReference=0;
    texture3DBlank=0;
    texture3DCurrent=0;
    textureSetAlreadyLoaded=false;
    masklessSetupStackTexture=false;
    */

    // qDebug() << this << texture3DCurrent << texture3DSignal << texture3DBackground << texture3DReference << texture3DBlank;

    // black background for consistency with other viewers
    RGBA32f bg_color;
    bg_color.r = bg_color.g = bg_color.b = 0.0f;
    bg_color.a = 1.0f;
    color_background = bg_color;
    color_background2 = bg_color;

    // remove decorations from 3D viewer
    bShowAxes = false;
    bShowBoundingBox = false;
    bShowBoundingBox2 = false;

    setRenderTextureLast(true); // Everyone draws opaque geometry first.  Why is this not the default?

    loadObj(); // create display lists for markers

    // initial alpha blending mode
    setAlphaBlending(false);

    // VOLUME_FILTER = 0; // Use sharp voxel boundaries (at least for testing)
    tryTexNPT = true; // Don't use Vaa3D rescale of image dimensions (because we are taking care of that...)
}

static void turn_off_specular()
{
    // attempt to turn off specular lighting
    float black_color[4] = {0,0,0,0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black_color);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, black_color);
    glLightfv(GL_LIGHT1, GL_SPECULAR, black_color);
    glLightfv(GL_LIGHT2, GL_SPECULAR, black_color);
}

RendererNeuronAnnotator::~RendererNeuronAnnotator()
{
    // TODO - what about all those texture3Dwhatever?  When are those deleted?
    // qDebug() << "RendererNeuronAnnotator destructor" << this;
    /*
    cleanExtendedTextures(); // might delete texture3DCurrent
    if (neuronMask) {delete [] neuronMask; neuronMask = NULL;}
    if (texture3DReference) {delete [] texture3DReference; texture3DReference = NULL;}
    if (texture3DBlank) {delete [] texture3DBlank; texture3DBlank = NULL;}
    if (texture3DBackground) {delete [] texture3DBackground; texture3DBackground = NULL;}
     */
    // NOTE that deleting texture3DSignal is handled by something else

    // I manage the texture memory, not you.  So set to NULL before you get a chance to clear it.
    Xslice_data = Yslice_data = Zslice_data = NULL;
    Xtex_list = Ytex_list = Ztex_list = NULL;
}

/* slot */
void RendererNeuronAnnotator::setStereoMode(int m)
{
    stereo3DMode = (Stereo3DMode) m;
}

/* slot */
void RendererNeuronAnnotator::setAlphaBlending(bool b)
{
    // qDebug() << "RendererNeuronAnnotator::setAlphaBlending()" << b;
    if (b == (renderMode == Renderer::rmAlphaBlendingProjection))
        return; // no change
    if (b)
    {
        makeCurrent();
        renderMode = Renderer::rmAlphaBlendingProjection;
        equAlphaBlendingProjection();
    }
    else
        renderMode = Renderer::rmMaxIntensityProjection;
    emit alphaBlendingChanged(b);
}

// helper for loadShader(), copied from Renderer_gl2.cpp
static QString resourceTextFile(QString filename)
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

// helper for loadShader(), copied from Renderer_gl2.cpp
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

/* virtual */
void RendererNeuronAnnotator::loadShader()
{
    qDebug() << "RendererNeuronAnnotator::loadShader()";
    cleanShader(); //090705
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
            QString texShaderName(":/neuron_annotator/resources/tex_fragment_cmb.txt");
            bool bUseClassicV3dShader = false;
            if (bUseClassicV3dShader)
                 texShaderName = ":/shader/tex_fragment.txt";
            linkGLShader(SMgr, shaderTex2D,
                            0, //Q_CSTR(resourceTextFile(":/shader/color_vertex.txt")),
                            Q_CSTR(QString("#undef TEX3D \n") + deftexlod + resourceTextFile(texShaderName)));

            qDebug("+++++++++ shader for Volume texture3D");
            linkGLShader(SMgr, shaderTex3D,
                            0, //Q_CSTR(resourceTextFile(":/shader/color_vertex.txt")),
                            Q_CSTR(QString("#define TEX3D \n") + deftexlod + resourceTextFile(texShaderName)));

    }
    catch (...) {
        qDebug() << "ERROR: shader loading failed";
    }

    qDebug("+++++++++ GLSL shader setup finished.");

    glGenTextures(1, &texColormap);
    initColormap();

    if (neuronLabelTexture)
        neuronLabelTexture->uploadPixels();
    else
        qDebug() << "NeuronLabelTexture not set" << __FILE__ << __LINE__;

    if (neuronVisibilityTexture)
        neuronVisibilityTexture->uploadPixels();
    else
        qDebug() << "NeuronVisibilityTexture not set" << __FILE__ << __LINE__;
}

void RendererNeuronAnnotator::shaderTexBegin(bool stream)
{
        shader = (texture_unit0_3D && !stream)? shaderTex3D : shaderTex2D;

        int format_bgra = (stream && pbo_image_format==GL_BGRA)? 1:0;

        if (tryVolShader && shader && !b_selecting)
        {
                shader->begin(); //must before setUniform
                shader->setUniform1i("volume",   0); //GL_TEXTURE0
                shader->setUniform1i("colormap", 1); //GL_TEXTURE1, 2D
                shader->setUniform1i("neuronVisibility", 2); // GL_TEXTURE2, 1D
                shader->setUniform1i("neuronLabel", 3);

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
                // qDebug() << "Uploading color map" << __FILE__ << __LINE__;
                glTexSubImage2D(GL_TEXTURE_2D, // target
                                0, // level
                                0,0, // offset
                                256, // width
                                FILL_CHANNEL,   // height
                                GL_RGBA, // image format
                                GL_UNSIGNED_BYTE, // image type
                                &colormap[0][0]);
                CHECK_GLError_print();

                if (neuronVisibilityTexture && neuronVisibilityTexture->bNeedsUpload)
                {
                    neuronVisibilityTexture->uploadPixels();
                }

                // switch back to volume data texture
                glActiveTextureARB(GL_TEXTURE0_ARB);
        }
}

void RendererNeuronAnnotator::shaderTexEnd()
{
        if (tryVolShader && shader && !b_selecting)
        {
                // off colormap
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE2_ARB);
                glDisable(GL_TEXTURE_1D);
                glActiveTextureARB(GL_TEXTURE3_ARB);
                glDisable(GL_TEXTURE_3D);
                glActiveTextureARB(GL_TEXTURE0_ARB);

                shader->end();
        }
        shader = 0;
}

void RendererNeuronAnnotator::equAlphaBlendingProjection()
{
    // qDebug() << "RendererNeuronAnnotator::equAlphaBlendingProjection()";

    // Renderer_gl2::equAlphaBlendingProjection();

    glBlendEquationEXT(GL_FUNC_ADD_EXT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RendererNeuronAnnotator::setDepthClip(float totalDepthInGlUnits)
{
    if (totalDepthInGlUnits <= 0) return;
    viewNear = viewDistance - 0.3 * totalDepthInGlUnits;
    viewNear = viewNear < 1.0 ? 1.0 : viewNear;
    viewFar = viewDistance + 0.7 * totalDepthInGlUnits;
}

// mouse left click to select neuron
// copied from Renderer_gl1::selectPosition(x, y)
/* virtual */
XYZ RendererNeuronAnnotator::screenPositionToVolumePosition(const QPoint& screenPos)
{
    int x = screenPos.x();
    int y = screenPos.y();

        // _appendMarkerPos
        MarkerPos pos;
        pos.x = x;
        pos.y = y;
        for (int i=0; i<4; i++)
                pos.view[i] = viewport[i];
        for (int i=0; i<16; i++)
        {
                pos.P[i]  = projectionMatrix[i];
                pos.MV[i] = markerViewMatrix[i];
        }

        // getCenterOfMarkerPos
        XYZ P1, P2;

        //_MarkerPos_to_NearFarPoint
        Matrix P(4,4);		P << pos.P;   P = P.t();    // OpenGL is row-inner / C is column-inner
        Matrix M(4,4);		M << pos.MV;  M = M.t();
        Matrix PM = P * M;

        double xd = (pos.x             - pos.view[0])*2.0/pos.view[2] -1;
        double yd = (pos.view[3]-pos.y - pos.view[1])*2.0/pos.view[3] -1; // OpenGL is bottom to top
        //double z = 0,1;                              // the clip space depth from 0 to 1

        ColumnVector pZ0(4); 	pZ0 << xd << yd << 0 << 1;
        ColumnVector pZ1(4); 	pZ1 << xd << yd << 1 << 1;
        if (bOrthoView)
        {
                pZ0(3) = -1;  //100913
        }
        ColumnVector Z0 = PM.i() * pZ0;       //cout << "Z0 \n" << Z0 << endl;
        ColumnVector Z1 = PM.i() * pZ1;       //cout << "Z1 \n" << Z1 << endl;
        Z0 = Z0 / Z0(4);
        Z1 = Z1 / Z1(4);

        P1 = XYZ(Z0(1), Z0(2), Z0(3));
        P2 = XYZ(Z1(1), Z1(2), Z1(3));

        // getCenterOfLineProfile
        XYZ loc = (P1+P2)*.5;

        //
        int chno;

        V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
        My4DImage* curImg = 0;

        if (w)
        {
                curImg = v3dr_getImage4d(_idep);

//		chno = w->getNumKeyHolding()-1; // #channel info got from keyboard
//		if (chno<0 || chno>dim4) chno = curChannel; // default channel set by user
        }

        double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
        // [0, 1] ==> [+1, -1]*(s)
        clipplane[3] = viewClip;
        ViewPlaneToModel(markerViewMatrix, clipplane);

        float selectval = 0;
        int selectchno = 0;
        XYZ selectloc;

        XYZ P2ori = P2;
        XYZ P1ori = P1;
        for(chno=0; chno<dim4; chno++)
        {
           P2 = P2ori;
           P1 = P1ori;

            if (curImg && data4dp)
            {
                double f = 0.8; // must be LESS 1 to converge, close to 1 is better

                XYZ D = P2-P1; normalize(D);

                unsigned char* vp = 0;
                switch (curImg->getDatatype())
                {
                        case V3D_UINT8:
                                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
                                break;
                        case V3D_UINT16:
                                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(short int);
                                break;
                        case V3D_FLOAT32:
                                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float);
                                break;
                        default:
                                v3d_msg("Unsupported data type found. You should never see this.", 0);
                                return loc;
                }

                qDebug()<<"iter ..."<<chno<<"vp ..."<<vp;

                float sum = 0;
                for (int i=0; i<200; i++) // iteration, (1/f)^200 is big enough
                {
                        double length = norm(P2-P1);
                        if (length < 0.5) // pixel
                                break; //////////////////////////////////

                        int nstep = int(length + 0.5);
                        double step = length/nstep;

                        XYZ sumloc(0,0,0);
                        sum = 0;
                        for (int i=0; i<=nstep; i++)
                        {
                                XYZ P = P1 + D*step*(i);
                                float value;
                                switch (curImg->getDatatype())
                                {
                                        case V3D_UINT8:
                                                value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                                                break;
                                        case V3D_UINT16:
                                                value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                                                break;
                                        case V3D_FLOAT32:
                                                value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                                                break;
                                        default:
                                                v3d_msg("Unsupported data type found. You should never see this.", 0);
                                                return loc;
                                }

                                sumloc = sumloc + P*(value);
                                sum = sum + value;
                        }

                        if (sum)
                        {
                            loc = sumloc / sum;
                        }
                        else
                                break; //////////////////////////////////

                        P1 = loc - D*(length*f/2);
                        P2 = loc + D*(length*f/2);
                }

                float curval;
                V3DLONG x = loc.x + 0.5;
                V3DLONG y = loc.y + 0.5;
                V3DLONG z = loc.z + 0.5;
                V3DLONG offsets = z*dim2*dim1 + y*dim1 + x;

                // Avoid seg fault on linux.
                // I do not understand this whole method, so I am just dodging
                // this one segfault.
                V3DLONG maxOffsets = dim1*dim2*dim3 - 1;
                V3DLONG minOffsets = 0;
                if (  (offsets < minOffsets)
                   || (offsets > maxOffsets) )
                {
                    qDebug() << "Avoided segmentation fault in RendererNeuronAnnotator::screenPositionToVolumePosition()"
                             << loc.x << loc.y << loc.z << ";" << dim1 << dim2 << dim3
                             << ";" << offsets
                             << __FILE__ << __LINE__;
                    return loc;
                }

                switch (curImg->getDatatype())
                {
                        case V3D_UINT8:
                                curval = *(vp + offsets);
                                break;
                        case V3D_UINT16:
                                curval = *((short int *)vp + offsets);
                                break;
                        case V3D_FLOAT32:
                                curval = *((float *)vp + offsets);
                                break;
                        default:
                                v3d_msg("Unsupported data type found. You should never see this.", 0);
                                return loc;
                }

                if(curval>selectval)
                {
                    selectval = curval;
                    selectchno = chno;
                    selectloc = loc;

                    qDebug()<<"select channel no ..."<<selectchno;
                }

            }

            qDebug()<<"chno ..."<<chno<<"dim4 ..."<<dim4;

        }
        qDebug()<<"0-based pos ... "<<selectloc.x<<selectloc.y<<selectloc.z;

        return selectloc;
}

void RendererNeuronAnnotator::loadVol()
{
    // Renderer_gl1::loadVol();
    cleanVol(); // 081006: move to before setting imageX/Y/Z, 090705 move to first line
    cleanTexStreamBuffer(); //091012

    qDebug("  Renderer_gl1::loadVol");
    makeCurrent(); //ensure right context when multiple views animation or mouse drop, 081105

    if (! volumeTexture || bufSize[3]<1 ) return; // no image data, 081002

    ////////////////////////////////////////////////////////////////
    // set coordinate frame size
    sampleScaleX = sampleScale[0];
    sampleScaleY = sampleScale[1];
    sampleScaleZ = sampleScale[2];
    imageX = bufSize[0];
    imageY = bufSize[1];
    imageZ = bufSize[2];
    imageT = bufSize[4];

    bool ok;
    if ( !(ok = supported_TexNPT()) )
            tryTexNPT = 0;
    qDebug()<< QString("	ARB_texture_non_power_of_two          %1 supported ").arg(ok?"":"NOT");

    if ( !(ok = supported_TexCompression()) )
            tryTexCompress = 0;
    qDebug()<< QString("	ARB_texture_compression               %1 supported ").arg(ok?"":"NOT");

    if ( !(ok = supported_Tex3D()) )
            tryTex3D = 0;
    qDebug()<< QString("	EXT_texture3D (or OpenGL 2.0)         %1 supported ").arg(ok?"":"NOT");

    if ( !(ok = supported_TexStream()) )
            if (tryTexStream != -1)
                    tryTexStream = 0;
    qDebug()<< QString("	texture stream (need PBO and GLSL)    %1 supported ").arg(ok?"":"NOT");

    ok = supported_GL2();
    qDebug()<< QString("	GLSL (and OpenGL 2.0)                 %1 supported ").arg(ok?"":"NOT");


    if (imageT>1) //090802: TexSubImage conflicts against compressed texture2D, but is good for compressed texture3D
    {
            //tryTex3D = 1; 			qDebug("	Turn on tryTex3D for Time series");
            tryTexCompress = 0;		qDebug("		Turn off tryTexCompress for time series");
            tryTexStream = 0;		qDebug("		Turn off tryTexStream for time series");
    }

//	// comment for easy test on small volume
//	if (IS_FITTED_VOLUME(imageX,imageY,imageZ))
//	{
//		if (tryTexStream==1)
//		{
//			qDebug("	No need texture stream for small volume (fitted in %dx%dx%d)", LIMIT_VOLX,LIMIT_VOLY,LIMIT_VOLZ);
//			tryTexStream = 0;  // no need stream, because volume can be fitted in video memory
//		}
//	}

    ////////////////////////////////////////////////////////////////
    // coordinate system
    //
    //     y_slice[z][x]
    //      |
    //      |
    //      |_______ x_slice[z][y]
    //     /
    //    /z_slice[y][x]
    //
    ////////////////////////////////////////////////////////////////
    QTime qtime;  qtime.start();
    qDebug("   setupStack start --- try %s", try_vol_state());

    // fillX = _getTexFillSize(imageX);
    // fillY = _getTexFillSize(imageY);
    // fillZ = _getTexFillSize(imageZ);
    qDebug("   sampleScale = %gx%gx%g""   sampledImage = %dx%dx%d""   fillTexture = %dx%dx%d",
                    sampleScaleX, sampleScaleY, sampleScaleZ,  imageX, imageY, imageZ,  fillX, fillY, fillZ);

    if (tryTex3D && supported_Tex3D())
    {
        qDebug() << "Renderer_gl1::loadVol() - creating 3D texture ID\n";
            glGenTextures(1, &tex3D);		//qDebug("	tex3D = %u", tex3D);
    }
    if (!tex3D || tryTexStream !=0) //stream = -1/1/2
    {
            //tryTex3D = 0; //091015: no need, because tex3D & tex_stream_buffer is not related now.

        qDebug() << "Renderer_gl1::loadVol() - creating data structures for managing 2D texture slice set\n";

            // Ztex_list = new GLuint[imageZ+1]; //+1 for pbo tex
            // Ytex_list = new GLuint[imageY+1];
            // Xtex_list = new GLuint[imageX+1];
            // memset(Ztex_list, 0, sizeof(GLuint)*(imageZ+1));
            // memset(Ytex_list, 0, sizeof(GLuint)*(imageY+1));
            // memset(Xtex_list, 0, sizeof(GLuint)*(imageX+1));
            // /glGenTextures(imageZ+1, Ztex_list);
            // glGenTextures(imageY+1, Ytex_list);
            // glGenTextures(imageX+1, Xtex_list);

            CHECK_GLErrorString_throw(); // can throw const char* exception, RZC 080925

            int X = _getBufFillSize(imageX);
            int Y = _getBufFillSize(imageY);
            int Z = _getBufFillSize(imageZ);
            // Zslice_data = new RGBA8 [Y * X];//[Z][y][x] //base order
            // Yslice_data = new RGBA8 [Z * X];//[Y][z][x]
            // Xslice_data = new RGBA8 [Z * Y];//[X][z][y]
            // memset(Zslice_data, 0, sizeof(RGBA8)* (Y * X));
            // memset(Yslice_data, 0, sizeof(RGBA8)* (Z * X));
            // memset(Xslice_data, 0, sizeof(RGBA8)* (Z * Y));

            // optimized copy slice data in setupStackTexture, by RZC 2008-10-04
    }

    qDebug("   setupStack: id & buffer ....................... cost time = %g sec", qtime.elapsed()*0.001);


    ///////////////////////////////////////
    if (texture_format==-1)
    {
            texture_format = GL_RGBA;
            //Call TexImage with a generic compressed internal format. The texture image will be compressed by the GL, if possible.
            //Call CompressedTexImage to Load pre-compressed image.
            //S3TC: DXT1(1bit alpha), DXT3(sharp alpha), DXT5(smooth alpha)
            //glHint(GL_TEXTURE_COMPRESSION_HINT_ARB, GL_NICEST); // seems no use, choice DXT3, but DXT5 is better, 081020
            if (tryTexCompress && GLEE_ARB_texture_compression)
                    texture_format = GL_COMPRESSED_RGBA_ARB;
            if (texture_format==GL_COMPRESSED_RGBA_ARB && GLEE_EXT_texture_compression_s3tc)
                    texture_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
    if (image_format==-1)
    {
            image_format = GL_RGBA;
    }
    if (image_type==-1)
    {
            image_type = GL_UNSIGNED_BYTE;
    }

    subloadTex(volTimePoint, true);   // first loading
    ///////////////////////////////////////

    //091013
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w)  w->updateControl();

    if (volumeTexture)
        updateSettingsFromVolumeTexture(*volumeTexture);
}

// This function resamples the original mask to the same dimensions
// as the texture buffer. It returns false if the source image is smaller
// than the target buffer because this isn't implemented.

/*
bool RendererNeuronAnnotator::populateNeuronMaskAndReference(NaVolumeData::Reader& volumeReader)
{
    QTime stopwatch;
    stopwatch.start();
    if (! volumeReader.hasReadLock()) return false;
    const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();
    const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();

    // qDebug() << "Start RendererNeuronAnnotator::populateNeuronMask()";

    // Clean previous data
    if (neuronMask!=0) {
        delete [] neuronMask;
        neuronMask = NULL;
    }

    if (neuronProxy.sz == 0) {
        qDebug() << "RendererNeuronAnnotator::populateNeuronMask() can't process empty my4Dmask";
        return false;
    }

    int sourceX = neuronProxy.sx;
    int sourceY = neuronProxy.sy;
    int sourceZ = neuronProxy.sz;

    float sx, sy, sz;
    // V3DLONG dx, dy, dz;
    sx = float(sourceX)/realX;
    sy = float(sourceY)/realY;
    sz = float(sourceZ)/realZ;

    if (!(sx>=1.0 && sy>=1.0 && sz>=1.0)) {
        qDebug() << "RendererNeuronAnnotator::populateNeuronMask() source image must be equal or larger dimension than texture with current implementation";
        return false;
    }

    neuronMask=new unsigned char[realX*realY*realZ];

    memset(neuronMask, 0, realX*realY*realZ);

    V3DLONG ox, oy, oz;
    V3DLONG ix, iy, iz;

    if (texture3DReference!=0) {
        delete [] texture3DReference;
        texture3DReference = NULL;
    }

    texture3DReference=new RGBA8[realZ*realY*realX];

    // Because we are mapping a mask, we do NOT want to 'sample' the data,
    // but rather condense it without missing any of the mask elements. Therefore,
    // we will iterate over the source mask, and then direct the contents to the
    // equivalent target voxel.

    // Note that the first mask entry is not overwritten by subsequent entries.

    for (iz = 0; iz < sourceZ; iz++) {
        for (iy = 0; iy < sourceY; iy++) {
            for (ix = 0; ix < sourceX; ix++) {
                ox = CLAMP(0,realX-1, IROUND(ix/sx));
                oy = CLAMP(0,realY-1, IROUND(iy/sy));
                oz = CLAMP(0,realZ-1, IROUND(iz/sz));

                V3DLONG offset = oz*realX*realY + oy*realX + ox;
                if (neuronMask[offset]==0) {
                    neuronMask[offset] = neuronProxy.value_at(ix, iy, iz, 0);
                }
                RGBA8 referenceVoxel;
                double referenceIntensity = refProxy.value_at(ix,iy,iz,0);
                referenceVoxel.r=referenceIntensity;
                referenceVoxel.g=referenceIntensity;
                referenceVoxel.b=referenceIntensity;
                referenceVoxel.a=255;
                texture3DReference[offset] = referenceVoxel;
            }
        }
        // Stay interactive by flushing event queue every 20 milliseconds.
        if (stopwatch.elapsed() > 20) {
            volumeReader.unlock();
            emit progressMessageChanged("Populating neuron mask and reference");
            emit progressValueChanged(int((100.0 * iz) / sourceZ));
            QCoreApplication::processEvents();
            if (! volumeReader.refreshLock()) {
                emit progressAborted("");
                return false; // volume is no longer available for reading
            }
            stopwatch.restart();
        }
    }

    // qDebug() << "RendererNeuronAnnotator::populateNeuronMask() done";
    // qDebug() << "RendererNeuronAnnotator::populateNeuronMaskAndReference() took" << stopwatch.elapsed() / 1000.0 << "seconds";

    return true;
}


bool RendererNeuronAnnotator::populateBaseTextures() {

    // qDebug() << "RendererNeuronAnnotator::populateBaseTextures() start";

    // Sanity check
    if (texture3DSignal==0) {
        qDebug() << "RendererNeuronAnnotator::populateBaseTextures() requires that texture3DSignal be pre-populated";
        return false;
    }

    // Clean
    if (texture3DBlank!=0) {
        delete [] texture3DBlank;
        texture3DBlank = NULL;
    }
    if (texture3DBackground!=0) {
        delete [] texture3DBackground;
        texture3DBackground = NULL;
    }

    try {
        texture3DBlank=new RGBA8[realZ*realY*realX];
        texture3DBackground=new RGBA8[realZ*realY*realX];
    } catch (std::exception exc) {
        qDebug() << "Ran out of memory in RendererNeuronAnnotator::populateBaseTextures()"
                    << __FILE__ << __LINE__;
        return false;
    }

    RGBA8 blankRgba;
    blankRgba.r = 0;
    blankRgba.g = 0;
    blankRgba.b = 0;
    blankRgba.a = 0;
    int z,y,x;

    for (z = 0; z < realZ; z++)
            for (y = 0; y < realY; y++)
                    for (x = 0; x < realX; x++)
                    {
                        V3DLONG index=z*realY*realX + y*realX + x;


                        // Blank case
                        texture3DBlank[index] = blankRgba;

                        unsigned char maskValue=neuronMask[index];

                        // Background case
                        if (maskValue==0) {
                            texture3DBackground[index] = texture3DSignal[index];
                        } else {
                            texture3DBackground[index] = blankRgba;
                        }
                    }

    // qDebug() << "RendererNeuronAnnotator::populateBaseTextures() end";

    return true;
}
*/

// This is originally based on version from Renderer_gl1
void RendererNeuronAnnotator::setupStackTexture(bool bfirst)
{
#ifdef NA_USE_VOLUME_TEXTURE_CMB
    if (!volumeTexture) return;
    updateSettingsFromVolumeTexture(*volumeTexture);
#else
    // qDebug() << "RendererNeuronAnnotator::setupStackTexture() - start";

    if (masklessSetupStackTexture) {

        Renderer_gl1::setupStackTexture(true);

    } else {

        /* _safeReference3DBuf does its own "delete"; this extra one causes a crash.
        if (texture3DSignal!=0)
            delete [] texture3DSignal;
        */

        texture3DSignal = _safeReference3DBuf(rgbaBuf, imageX, imageY, imageZ,  safeX, safeY, safeZ); //081008
        realX = safeX;
        realY = safeY;
        realZ = safeZ;

        fillX = _getTexFillSize(realX);
        fillY = _getTexFillSize(realY);
        fillZ = _getTexFillSize(realZ);

        // qDebug("	texture:   real = %dx%dx%d   fill = %dx%dx%d",  realX, realY, realZ,  fillX, fillY, fillZ);
    }

    // qDebug() << "RendererNeuronAnnotator::setupStackTexture() - end";
#endif
}

void RendererNeuronAnnotator::updateSettingsFromVolumeTexture(const vaa3d::VolumeTexture& volumeTexture)
{
    realX = volumeTexture.usedTextureSize.x();
    realY = volumeTexture.usedTextureSize.y();
    realZ = volumeTexture.usedTextureSize.z();
    safeX = realX; // necessary
    safeY = realY;
    safeZ = realZ;

    // imageX = volumeTexture.originalImageSize.x();
    // imageY = volumeTexture.originalImageSize.y();
    // imageZ = volumeTexture.originalImageSize.z();
    imageX = volumeTexture.usedTextureSize.x();
    imageY = volumeTexture.usedTextureSize.y();
    imageZ = volumeTexture.usedTextureSize.z();
    bufSize[0] = imageX;
    bufSize[1] = imageY;
    bufSize[2] = imageZ;

    // sampleScaleX = sampleScaleY = sampleScaleZ = sampleScale[0] = sampleScale[1] = sampleScale[2] = 1.0;
    sampleScaleX = sampleScale[0] = (float)volumeTexture.usedTextureSize.x() / (float)volumeTexture.originalImageSize.x();
    sampleScaleY = sampleScale[1] = (float)volumeTexture.usedTextureSize.y() / (float)volumeTexture.originalImageSize.y();
    sampleScaleZ = sampleScale[2] = (float)volumeTexture.usedTextureSize.z() / (float)volumeTexture.originalImageSize.z();

    boundingBox.x0 = boundingBox.y0 = boundingBox.z0 = 0.0;
    // boundingBox.x1 = volumeTexture.usedTextureSize.x();
    // boundingBox.y1 = volumeTexture.usedTextureSize.y();
    // boundingBox.z1 = volumeTexture.usedTextureSize.z();
    boundingBox.x1 = volumeTexture.originalImageSize.x();
    boundingBox.y1 = volumeTexture.originalImageSize.y();
    boundingBox.z1 = volumeTexture.originalImageSize.z();

    // TODO VOL_[XYZ][01]?
    VOL_X0 = VOL_Y0 = VOL_Z0 = 0;
    VOL_X1 = VOL_Y1 = VOL_Z1 = 1;

    // HACK: Postpone propagating const correctness into legacy texture code...
    Xslice_data = (RGBA8*)(volumeTexture.getDataPtr(vaa3d::VolumeTexture::Stack::X));
    Yslice_data = (RGBA8*)(volumeTexture.getDataPtr(vaa3d::VolumeTexture::Stack::Y));
    Zslice_data = (RGBA8*)(volumeTexture.getDataPtr(vaa3d::VolumeTexture::Stack::Z));
    Xtex_list = (GLuint*)(volumeTexture.getTexIDPtr(vaa3d::VolumeTexture::Stack::X));
    Ytex_list = (GLuint*)(volumeTexture.getTexIDPtr(vaa3d::VolumeTexture::Stack::Y));
    Ztex_list = (GLuint*)(volumeTexture.getTexIDPtr(vaa3d::VolumeTexture::Stack::Z));

    image_format = GL_BGRA;
    image_type = GL_UNSIGNED_INT_8_8_8_8_REV;
    tryTexCompress = false;
}

/* virtual */
int RendererNeuronAnnotator::_getBufFillSize(int w)
{
    return vaa3d::Dimension::padToMultipleOf(w, 8);
}

/* virtual */
int RendererNeuronAnnotator::_getTexFillSize(int w)
{
    return vaa3d::Dimension::padToMultipleOf(w, 8);
}

/* virtual */
void RendererNeuronAnnotator::cleanVol()
{
    // I manage the texture memory, not you.  So set to NULL before you get a chance to clear it.
    Xslice_data = Yslice_data = Zslice_data = NULL;
    Xtex_list = Ytex_list = Ztex_list = NULL;
    Renderer_gl1::cleanVol();
}

// copied from renderer_tex.cpp
void RendererNeuronAnnotator::_drawStack( double ts, double th, double tw,
                double s0, double s1, double h0, double h1, double w0, double w1,
                double ds, int slice0, int slice1, int thickness,
                GLuint tex3D, GLuint texs[], int stack_i,
                float direction, int section, bool b_tex3d, bool b_stream)
{
        //qDebug("		s(%g-%g)h(%g-%g)w(%g-%g)", s0,s1, h0,h1, w0,w1);
        if ((s1-s0<0)||(h1-h0<0)||(w1-w0<0)) return; // no draw
        if (thickness <1) return; // only support thickness>=1

        if (section >0) { // cross-section
                h0 = 0;		h1 = 1;
                w0 = 0;		w1 = 1;
                s1 = s0;
                slice1 = slice0;
        }

//	double moreslice = ((tex3D)? 4 : 1); // 081009: more slice for tex3D
        double step, slice, s;
        if (direction <0) {
                step = (+1); ///moreslice);
                slice = slice0;
                s = s0;
        } else {
                step = (-1); ///moreslice);
                slice = slice1;
                s = s1;
        }

        double tw0 = tw*w0;  double tw1 = tw*w1;
        double th0 = th*h0;  double th1 = th*h1;

        for (;
                slice0 <= slice && slice <= slice1;
                slice += step, s += step * ds
                )
        {
                if (!b_tex3d)
                {
                        if (b_stream)
                        {
                                _streamTex(stack_i, int(slice), int(step), slice0, slice1);
                        }
                        else
                        {
                                glBindTexture(GL_TEXTURE_2D, texs[int(slice)+1]); //[0] reserved for pbo tex
                                setTexParam2D();
                        }
                }
                else // (btex_3d && !btex_stream)
                {
                        glBindTexture(GL_TEXTURE_3D, tex3D);
                        setTexParam3D();
                }

                double tss = ts*s;
                int k_repeat = thickness;
                if ( (step>0 && slice==slice1)
                        ||(step<0 && slice==slice0)
                        )  k_repeat = 1; // 081106

                for (int k=0; k<k_repeat; k++) // 081105
                {
                        double ids = step * k*ds/thickness;
                        double idts = ts*ids;

                        // temporary test to show boundaries of quads
                        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);

                        glBegin(GL_QUADS);

                        // Store different texture coordinates in texture units 0 and 3,
                        // so we can use both 2D and 3D textures, until we move everything
                        // to 3D textures.  I'm using 3D texture for the neuron labels because
                        // its easier; I'm using 2D textures for the volume because that is what
                        // is working already.
                        glMultiTexCoord2dARB(GL_TEXTURE0_ARB, tw0, th0);
                        if (stack_i==1) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw0, th0, tss +idts);
                        else if (stack_i==2) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw0, tss +idts, th0);
                        else if (stack_i==3) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tss +idts, tw0, th0);
                        if      (stack_i==1) glVertex3d(w0, h0, s +ids);
                        else if (stack_i==2) glVertex3d(w0, s +ids, h0);
                        else if (stack_i==3) glVertex3d(s +ids, w0, h0);

                        if (!b_tex3d)  glMultiTexCoord2dARB(GL_TEXTURE0_ARB, tw1, th0);
                        if (stack_i==1) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw1, th0, tss +idts);
                        else if (stack_i==2) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw1, tss +idts, th0);
                        else if (stack_i==3) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tss +idts, tw1, th0);
                        if      (stack_i==1) glVertex3d(w1, h0, s +ids);
                        else if (stack_i==2) glVertex3d(w1, s +ids, h0);
                        else if (stack_i==3) glVertex3d(s +ids, w1, h0);

                        if (!b_tex3d)  glMultiTexCoord2dARB(GL_TEXTURE0_ARB, tw1, th1);
                        if (stack_i==1) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw1, th1, tss +idts);
                        else if (stack_i==2) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw1, tss +idts, th1);
                        else if (stack_i==3) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tss +idts, tw1, th1);
                        if      (stack_i==1) glVertex3d(w1, h1, s +ids);
                        else if (stack_i==2) glVertex3d(w1, s +ids, h1);
                        else if (stack_i==3) glVertex3d(s +ids, w1, h1);

                        if (!b_tex3d)  glMultiTexCoord2dARB(GL_TEXTURE0_ARB, tw0, th1);
                        if (stack_i==1) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw0, th1, tss +idts);
                        else if (stack_i==2) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tw0, tss +idts, th1);
                        else if (stack_i==3) glMultiTexCoord3dARB(GL_TEXTURE3_ARB, tss +idts, tw0, th1);
                        if      (stack_i==1) glVertex3d(w0, h1, s +ids);
                        else if (stack_i==2) glVertex3d(w0, s +ids, h1);
                        else if (stack_i==3) glVertex3d(s +ids, w0, h1);

                        // qDebug() << "texture coordinates =" << tw0 << tw1 << th0 << th1;

                        glEnd();
                }
        }
        if (b_stream) _streamTex_end();
}

/*
bool RendererNeuronAnnotator::initializeTextureMasks() {

    if (!populateBaseTextures()) {
        return false;
    }

    QList<int> initialMaskList;
    for (int i=0;i<256;i++) {
        initialMaskList.append(i);
    }

    emit progressMessageChanged(QString("Setting up textures"));

    QList<RGBA8*> initialOverlayList;
    initialOverlayList.append(texture3DBackground);
    rebuildFromBaseTextures(initialMaskList, initialOverlayList);

    return true;
}


// This function assumes the size realX,Y,Z should be used
void RendererNeuronAnnotator::load3DTextureSet(RGBA8* tex3DBuf)
{
    // On subsequent load, some parameters might not be set yet.
    if (! (realX && realY && realZ && imageX && imageY && imageZ) ) return;

    // makeCurrent();
    int sliceCount=0;
    QTime stopwatch;
    stopwatch.start();

    for (int stack_i = 1; stack_i <= 3; ++stack_i)
    {
        // qDebug() << "RendererNeuronAnnotator::load3DTextureSet()" << stack_i << __FILE__ << __LINE__;

            int n_slice = 0;
            RGBA8* p_slice = 0;
            GLuint* p_tex = 0;
            int w = 0, h =0;
            int sw = 0, sh = 0;

            switch (stack_i)
            {
            case 1: //Z[y][x]
                    n_slice = realZ;
                    p_slice = Zslice_data;
                    p_tex = Ztex_list;
                    w = fillX, h = fillY;
                    sw = COPY_X, sh = COPY_Y;
                    break;
            case 2: //Y[z][x]
                    n_slice = realY;
                    p_slice = Yslice_data;
                    p_tex = Ytex_list;
                    w = fillX, h = fillZ;
                    sw = COPY_X, sh = COPY_Z;
                    break;
            case 3: //X[z][y]
                    n_slice = realX;
                    p_slice = Xslice_data;
                    p_tex = Xtex_list;
                    w = fillY, h = fillZ;
                    sw = COPY_Y, sh = COPY_Z;
                    break;
            }

            // On Linux, glTexSubImage2D() silently fails if a dimension is not
            // divisible by 4.
#ifdef __linux__
            if (textureSetAlreadyLoaded)
            {
                // Ensure subtexture sizes are divisible by 4 on linux
                // TODO - this is a hack
                // This approach can leave some wrong pixels at the edge of the volume.
                // But its better than having all of the pixels wrong.
                if ((sw%4)||(sh%4))
                {
                    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // this does not help
                    // glPixelStorei(GL_PACK_ALIGNMENT, 1);  // this does not help
                    sw = sw - (sw%4); // truncate texture update to multiple of 4
                    sh = sh - (sh%4); // truncate texture update to multiple of 4
                    qDebug() << "Applying hack to work around linux non-multiple-of-4 texture bug";
                }
            }
#endif

            MESSAGE_ASSERT(imageX>=realX && imageY>=realY && imageZ>=realZ);
            MESSAGE_ASSERT(COPY_X>=realX && COPY_Y>=realY && COPY_Z>=realZ);
//		sw+=1, sh+=1;  // to get rid of artifacts at sub-tex border // or use NPT tex
//		if (sw>w) sw = w;
//		if (sh>h) sh = h;

            for (int i = 0; i < n_slice; i++)

            {
                // qDebug() << "   " << i;
                    glBindTexture(GL_TEXTURE_2D, p_tex[i+1]); //[0] reserved for pbo tex
                    RGBA8* p_first = NULL;

                    if (!textureSetAlreadyLoaded)
                    {
                            p_first = p_slice;
                            if (p_first) _copySliceFromStack(tex3DBuf, realX,realY,realZ,  p_first, w,  stack_i, i);

                            setTexParam2D();
                            glTexImage2D(GL_TEXTURE_2D, // target
                                    0, // level
                                    texture_format, // texture format
                                    w, // width
                                    h, // height
                                    0, // border
                                    image_format, // image format
                                    image_type, // image type
                                    p_first);
                            CHECK_GLErrorString_throw(); // can throw const char* exception, RZC 080925
                    }
                    // Notice: compressed texture is corrupted with TexSubImage2D but TexSubImage3D !!!
                    else
                    {
                        // qDebug() << "    textureSetAlreadyLoaded" << i << stopwatch.elapsed();
                            _copySliceFromStack(tex3DBuf, realX,realY,realZ,  p_slice, sw,  stack_i, i);

                            glTexSubImage2D(GL_TEXTURE_2D, // target
                                    0, // level
                                    0,0,  // offset
                                    sw, // sub width
                                    sh, // sub height
                                    image_format, // image format
                                    image_type, // image type
                                    p_slice);
                            CHECK_GLErrorString_throw(); // can throw const char* exception, RZC 080925
                    }
                    sliceCount++;
                    int prog = sliceCount*100 / (realX + realY + realZ);
                    // updateProgressDialog(dialog, prog);

                    if (! (sliceCount % 20)) {
                        emit progressValueChanged(prog);
                        // processEvents() kludge as long as texture updates are in GUI thread
                        // TODO Is ExcludeUserInputEvents really necessary here?
                        QCoreApplication::processEvents(); //  QEventLoop::ExcludeUserInputEvents );
                        makeCurrent();
                    }
            }

    }//tex2D

    if (!textureSetAlreadyLoaded) {
        textureSetAlreadyLoaded=true;
    }

    emit progressComplete();
}

const RGBA8* RendererNeuronAnnotator::getTexture3DCurrent() const {
    // qDebug() << "getTexture3DCurrent()";
    // qDebug() << this << texture3DCurrent << texture3DSignal << texture3DBackground << texture3DReference << texture3DBlank;
    // qDebug() << __FILE__ << __LINE__;
    return texture3DCurrent;
}

void RendererNeuronAnnotator::cleanExtendedTextures() {
    if (texture3DCurrent!=0 &&
        texture3DCurrent!=texture3DSignal &&
        texture3DCurrent!=texture3DBackground &&
        texture3DCurrent!=texture3DReference &&
        texture3DCurrent!=texture3DBlank)
    {
        // qDebug() << "RendererNeuronAnnotator::cleanExtendedTextures()";
        // qDebug() << __FILE__ << __LINE__;
        // qDebug() << this;
        // qDebug() << this << texture3DCurrent << texture3DSignal << texture3DBackground << texture3DReference << texture3DBlank;
        delete [] texture3DCurrent;
        texture3DCurrent = NULL;
    }
}

void RendererNeuronAnnotator::rebuildFromBaseTextures(const QList<int>& maskIndexList, QList<RGBA8*>& overlayList) {
    cleanExtendedTextures();
    if (overlayList.size()==1 && overlayList.at(0)==texture3DSignal) {
        // Then we don't need to add masks since these are implicit
        texture3DCurrent=texture3DSignal;
    } else if (overlayList.size()==0) {
        overlayList.append(texture3DBlank);
        texture3DCurrent=extendTextureFromMaskList(overlayList, maskIndexList);
    } else {
        texture3DCurrent=extendTextureFromMaskList(overlayList, maskIndexList);
    }
    load3DTextureSet(texture3DCurrent);
    // qDebug() << "RendererNeuronAnnotator::rebuildFromBaseTextures()";
    // qDebug() << this << texture3DCurrent << texture3DSignal << texture3DBackground << texture3DReference << texture3DBlank;
    // qDebug() << __FILE__ << __LINE__;
}

RGBA8* RendererNeuronAnnotator::extendTextureFromMaskList(const QList<RGBA8*> & sourceTextures, const QList<int> & maskIndexList) {
    // qDebug() << "RendererNeuronAnnotator::extendTextureFromMaskList() start";
    RGBA8* newTexture=new RGBA8[realZ*realY*realX];
    int* quickList=new int[256];
    for (int m=0;m<256;m++) {
        quickList[m]=-1;
    }
    for (int m=0;m<maskIndexList.size();m++) {
        int value=maskIndexList.at(m);
        if (value>254) {
            qDebug() << "Error: ignoring mask entry greater than 254:" << value;
        } else {
            quickList[value+1]=value+1; // we want the array to be indexed by 1...n+1 rather than 0...n like maskIndexList
        }
    }
    // Move to arrays for performance
    int numSourceTextures=sourceTextures.size();
    RGBA8** sourceTextureArray=new RGBA8* [numSourceTextures];
    for (int i=0;i<numSourceTextures;i++) {
        sourceTextureArray[i]=sourceTextures.at(i);
    }
    for (int z=0;z<realZ;z++) {
        int zRyRx=z*realY*realX;
        for (int y=0;y<realY;y++) {
            int yRx=y*realX;
            for (int x=0;x<realX;x++) {
                int offset=zRyRx + yRx + x;
                unsigned char maskValue=neuronMask[offset];
                if (maskValue!=0 && quickList[maskValue]==maskValue) { // !=0 prevents background from always being added
                    // Add from mask
                    newTexture[offset]=texture3DSignal[offset];
                } else {
                    // Add up overlays
                    RGBA8 total;
                    total.a=0;
                    total.r=0;
                    total.g=0;
                    total.b=0;
                    for (int i=0;i<numSourceTextures;i++) {
                        RGBA8 rgba=sourceTextureArray[i][offset];
                        int capA=total.a+rgba.a;
                        if (capA>255) {
                            capA=255;
                        }
                        int capR=total.r+rgba.r;
                        if (capR>255) {
                            capR=255;
                        }
                        int capG=total.g+rgba.g;
                        if (capG>255) {
                            capG=255;
                        }
                        int capB=total.b+rgba.b;
                        if (capB>255) {
                            capB=255;
                        }
                        total.a=capA;
                        total.r=capR;
                        total.g=capG;
                        total.b=capB;
                    }
                    newTexture[offset]=total;
                }
            }
        }
    }
    delete [] sourceTextureArray;
    delete [] quickList;
    // qDebug() << "RendererNeuronAnnotator::extendTextureFromMaskList() done";
    return newTexture;
}

// What we want to do in this function is constrained by the requirement that texture
// updates must be of size power-of-2, beginning with size 4. This is an empirical
// finding from the current Mac Book Pro. We will choose size 16 and see how this performs.
//
// We will iterate through 16x16 tiles of the current texture set. For each tile, we will scan
// for mask membership. If a match is seen, we will first modify the local texture3DCurrent,
// and then copy this updated tile to the graphics system.

void RendererNeuronAnnotator::updateCurrentTextureMask(int neuronIndex, int state) {
    if(!texture3DCurrent) return;

    // qDebug() << "RendererNeuronAnnotator::updateCurrentTextureMask() start";
    makeCurrent();

    int maskIndex = neuronIndex+1;

    QTime timer;

    timer.start();

    const int TILE_DIMENSION=8;
    int tileBufferSize=TILE_DIMENSION*TILE_DIMENSION;
    RGBA8* tileBuffer = new RGBA8[tileBufferSize];
    int glTilesIncluded=0;
    int glTilesExcluded=0;
    int sliceCount=0;

    for (int stack_i=1; stack_i<=3; stack_i++)
    {
            GLuint* p_tex = 0;
            int sw = 0, sh = 0;
            int n_slice = 0;

            switch (stack_i)
            {
            case 1: //Z[y][x]
                    n_slice = realZ;
                    p_tex = Ztex_list;
                    sw = COPY_X, sh = COPY_Y;
                    break;
            case 2: //Y[z][x]
                    n_slice = realY;
                    p_tex = Ytex_list;
                    sw = COPY_X, sh = COPY_Z;
                    break;
            case 3: //X[z][y]
                    n_slice = realX;
                    p_tex = Xtex_list;
                    sw = COPY_Y, sh = COPY_Z;
                    break;
            }

            // Temporary hack to imperfectly deal with non-multiple-of-8 texture sizes
            // TODO - this is a hack
            // This approach can leave some wrong pixels at the edge of the volume.
            // But its better than having all of the pixels wrong.
            if ((sw%8)||(sh%8))
            {
                sw = sw - (sw%8); // truncate texture update to multiple of 8
                sh = sh - (sh%8); // truncate texture update to multiple of 8
                qDebug() << "Applying hack to work around non-multiple-of-8 texture dimension issue";
            }

            if (sw%TILE_DIMENSION!=0 || sh%TILE_DIMENSION!=0) {
                qDebug() << "RendererNeuronAnnotator::updateCurrentTextureMask() ERROR: width and height of texture buffer must both be divisible by "
                        << TILE_DIMENSION << "! width=" << sw << " height=" << sh << " realX=" << realX << " realY=" << realY << " realZ=" << realZ;
                emit progressAborted("Error - texture size is not a multiple of 8");
                return;
            }

            RGBA8 blank;
            blank.r=0;
            blank.g=0;
            blank.b=0;
            blank.a=0;

            int RxRy = realX* realY;

            for (int i = 0; i < n_slice; i++)

            {
                    glBindTexture(GL_TEXTURE_2D, p_tex[i+1]); //[0] reserved for pbo tex

                    // Let's consider each tile
                    for (int h=0;h<sh;h+=TILE_DIMENSION) {
                        for (int w=0;w<sw;w+=TILE_DIMENSION) {
                            bool updateMaskTile=false;
                            // While we are traversing each pixel of the tile, we will update
                            // any position in texture3DCurrent which is affected by the mask
                            // change, and also flag the tile for graphics update.

                            // Update matrix:
                            // stack_i     i   :  ho   :  wo

                            //    1       [z]     [y]     [x]

                            //    2       [y]     [z]     [x]

                            //    3       [x]     [z]     [y]

                            int bufX, bufY, bufZ;
                            if (stack_i==1) {
                                bufZ=i;
                                int m1=bufZ*RxRy;
                                for (int ho=h;ho<(h+TILE_DIMENSION);ho++) {
                                    bufY=ho;
                                    int m2=bufY*realX;
                                    for (int wo=w;wo<(w+TILE_DIMENSION);wo++) {
                                        bufX=wo;
                                        int offset = m1 + m2 + bufX;
                                        int tileOffset = (ho-h)*TILE_DIMENSION + (wo-w);
                                        tileBuffer[tileOffset]=texture3DCurrent[offset];
                                        int maskValue=neuronMask[offset];
                                        if (maskValue==maskIndex) {
                                            updateMaskTile=true;
                                            if (state==0) {
                                                // Turn off mask
                                                texture3DCurrent[offset] = blank;
                                                tileBuffer[tileOffset] = blank;
                                            } else {
                                                // Turn on mask
                                                texture3DCurrent[offset] = texture3DSignal[offset];
                                                tileBuffer[tileOffset] = texture3DSignal[offset];
                                            }
                                        }
                                    }
                                }
                            } else if (stack_i==2) {
                                bufY=i;
                                int m2=bufY*realX;
                                for (int ho=h;ho<(h+TILE_DIMENSION);ho++) {
                                    bufZ=ho;
                                    int m1=bufZ*RxRy;
                                    for (int wo=w;wo<(w+TILE_DIMENSION);wo++) {
                                        bufX=wo;
                                        int offset = m1 + m2 + bufX;
                                        int tileOffset = (ho-h)*TILE_DIMENSION + (wo-w);
                                        tileBuffer[tileOffset]=texture3DCurrent[offset];
                                        int maskValue=neuronMask[offset];
                                        if (maskValue==maskIndex) {
                                            updateMaskTile=true;
                                            if (state==0) {
                                                // Turn off mask
                                                texture3DCurrent[offset] = blank;
                                                tileBuffer[tileOffset] = blank;
                                            } else {
                                                // Turn on mask
                                                texture3DCurrent[offset] = texture3DSignal[offset];
                                                tileBuffer[tileOffset] = texture3DSignal[offset];
                                            }
                                        }
                                    }
                                }
                            } else if (stack_i==3) {
                                bufX=i;
                                for (int ho=h;ho<(h+TILE_DIMENSION);ho++) {
                                    bufZ=ho;
                                    int m1=bufZ*RxRy;
                                    for (int wo=w;wo<(w+TILE_DIMENSION);wo++) {
                                        bufY=wo;
                                        int offset = m1 + bufY*realX + bufX;
                                        int tileOffset = (ho-h)*TILE_DIMENSION + (wo-w);
                                        tileBuffer[tileOffset]=texture3DCurrent[offset];
                                        int maskValue=neuronMask[offset];
                                        if (maskValue==maskIndex) {
                                            updateMaskTile=true;
                                            if (state==0) {
                                                // Turn off mask
                                                texture3DCurrent[offset] = blank;
                                                tileBuffer[tileOffset] = blank;
                                            } else {
                                                // Turn on mask
                                                texture3DCurrent[offset] = texture3DSignal[offset];
                                                tileBuffer[tileOffset] = texture3DSignal[offset];
                                            }
                                        }
                                    }
                                }
                            }
                            if (updateMaskTile) {
                                glTexSubImage2D(GL_TEXTURE_2D, // target
                                                0, // level
                                                w,h,  // offset
                                                TILE_DIMENSION, // sub width
                                                TILE_DIMENSION, // sub heighttexture3DCurrent
                                                image_format, // image format
                                                image_type, // image type
                                                tileBuffer);
                                CHECK_GLErrorString_throw();
                                glTilesIncluded++;
                            } else {
                                glTilesExcluded++;
                            }
                        }
                    }
                    sliceCount++;
                    int prog = sliceCount*100 / (realX + realY + realZ);
                    // updateProgressDialog(dialog, prog);

                    // attempt to get away from that horrible horrible hyper-modal stack-drilling unthreadable progress dialog
                    if (! (sliceCount % 20)) {
                        emit progressValueChanged(prog);
                        // processEvents() kludge as long as texture updates are in GUI thread
                        // TODO Is ExcludeUserInputEvents really necessary here?
                        QCoreApplication::processEvents(); //  QEventLoop::ExcludeUserInputEvents);
                        makeCurrent();
                    }
            }
    }
    int mSec=timer.elapsed();
    qDebug() << "RendererNeuronAnnotator::updateCurrentTextureMask() finished in " << mSec << " milliseconds  tilesIncluded=" << glTilesIncluded <<" tilesExcluded=" << glTilesExcluded;
    emit progressComplete();
}

RGBA8* RendererNeuronAnnotator::getOverlayTextureByAnnotationIndex(int index)
{
    if (index==DataFlowModel::REFERENCE_MIP_INDEX) {
        return texture3DReference;
    } else if (index==DataFlowModel::BACKGROUND_MIP_INDEX) {
        return texture3DBackground;
    }
}

*/

float RendererNeuronAnnotator::glUnitsPerImageVoxel() const
{
    if (0 == imageZ)
        return 2.0 / boundingBox.Dmax();

    if ( (boundingBox.Dx() >= boundingBox.Dy()) && (boundingBox.Dx() >= boundingBox.Dz()) )
        return 2.0 * sampleScaleX / imageX;
    else if (boundingBox.Dy() >= boundingBox.Dz())
        return 2.0 * sampleScaleY / imageY;
    else
        return 2.0 * sampleScaleZ / imageZ;
}

bool RendererNeuronAnnotator::hasBadMarkerViewMatrix() const // to help avoid a crash
{
    return (! (markerViewMatrix[0] == markerViewMatrix[0]));
}

void RendererNeuronAnnotator::clearLandmarks()
{
    if (0 == listMarker.size()) return; // already clear
    listMarker.clear();
}

void RendererNeuronAnnotator::setLandmarks(const QList<ImageMarker>& landmarks)
{
    if (landmarks == listMarker) return; // no change
    listMarker = landmarks;
}

// StereoEyeView sets either left or right eye view, depending on constructor argument.
class StereoEyeView
{
public:
    enum Eye {LEFT, RIGHT};

    StereoEyeView(Eye eyeActual, Eye eyeGeom)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        // Set camera for left eye
        // Rotate by about 4 degrees about Y axis
        // TODO - a shear operation on the projection matrix might clip better.
        // We'll solve that problem when it appears...
        // Need to PRE-multiply rotation matrix, so some tedious manipulation
        GLdouble viewMat[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, viewMat); // remember current modelview
        glLoadIdentity();
        double angle = 1.7; // left eye
        if (eyeGeom == RIGHT) angle = -angle; // right eye is opposite direction
        glRotated(angle, 0, 1, 0); // put rotation in modelview
        glMultMatrixd(viewMat); // end result is premultiply by Rotation
        glDrawBuffer(GL_BACK); // for non-quad modes
    }

    virtual ~StereoEyeView()
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
};

class AnaglyphRedCyanEyeView : public StereoEyeView
{
public:
    AnaglyphRedCyanEyeView(Eye eye, Eye eyeGeom) : StereoEyeView(eye, eyeGeom)
    {
        if (eye == LEFT)
            glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE); // red only
        else
            glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE); // cyan only
    }

    ~AnaglyphRedCyanEyeView()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Show all colors
    }
};

class AnaglyphGreenMagentaEyeView : public StereoEyeView
{
public:
    AnaglyphGreenMagentaEyeView(Eye eye, Eye eyeGeom) : StereoEyeView(eye, eyeGeom)
    {
        if (eye == LEFT)
            glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE); // green only
        else
            glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE); // magenta only
    }

    ~AnaglyphGreenMagentaEyeView()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Show all colors
    }
};

class QuadBufferView : public StereoEyeView
{
public:
    QuadBufferView(Eye eye, Eye eyeGeom) : StereoEyeView(eye, eyeGeom)
    {
        if (eye == LEFT)
            glDrawBuffer(GL_BACK_LEFT);
        else
            glDrawBuffer(GL_BACK_RIGHT);
    }

    ~QuadBufferView()
    {
    }
};

/* virtual */
void RendererNeuronAnnotator::paint()
{
    makeCurrent();
    switch(stereo3DMode)
    {
    case STEREO_OFF:
        paint_mono();
        break;
    case STEREO_LEFT_EYE:
        {
            StereoEyeView v(StereoEyeView::LEFT, bStereoSwapEyes? StereoEyeView::RIGHT : StereoEyeView::LEFT);
            paint_mono();
        }
        break;
    case STEREO_RIGHT_EYE:
        {
            StereoEyeView v(StereoEyeView::RIGHT, bStereoSwapEyes? StereoEyeView::LEFT : StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    case STEREO_ANAGLYPH_RED_CYAN:
        {
            AnaglyphRedCyanEyeView v(StereoEyeView::LEFT, bStereoSwapEyes? StereoEyeView::RIGHT : StereoEyeView::LEFT);
            paint_mono();
        }
        {
            AnaglyphRedCyanEyeView v(StereoEyeView::RIGHT, bStereoSwapEyes? StereoEyeView::LEFT : StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    case STEREO_ANAGLYPH_GREEN_MAGENTA:
        {
            AnaglyphGreenMagentaEyeView v(StereoEyeView::LEFT, bStereoSwapEyes? StereoEyeView::RIGHT : StereoEyeView::LEFT);
            paint_mono();
        }
        {
            AnaglyphGreenMagentaEyeView v(StereoEyeView::RIGHT, bStereoSwapEyes? StereoEyeView::LEFT : StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    case STEREO_QUAD_BUFFERED:
        {
            QuadBufferView v(StereoEyeView::LEFT, bStereoSwapEyes? StereoEyeView::RIGHT : StereoEyeView::LEFT);
            paint_mono();
        }
        {
            QuadBufferView v(StereoEyeView::RIGHT, bStereoSwapEyes? StereoEyeView::LEFT : StereoEyeView::RIGHT);
            paint_mono();
        }
        break;
    default:
        qDebug() << "Error: Unsupported Stereo mode" << stereo3DMode;
        paint_mono();
        break;
    }
}

// Copied from Renderer_gl1::paint() 27 Sep 2011 CMB
void RendererNeuronAnnotator::paint_mono()
{
        // turn_off_specular();
        //qDebug(" Renderer_gl1::paint(renderMode=%i)", renderMode);

        if (b_error) return; //080924 try to catch the memory error

        glClearColor(color_background.r, color_background.g, color_background.b, 0);
        glClearStencil(0);
        glClearDepth(1);
        glDepthRange(0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
                                //GL_LEQUAL);

        glMatrixMode(GL_MODELVIEW);
        // here, MUST BE normalized space of [-1,+1]^3;

        glGetDoublev(GL_MODELVIEW_MATRIX, volumeViewMatrix); //no scale here, used for drawUnitVolume()

        glPushMatrix();
                setMarkerSpace(); // space to define marker & curve
                glGetIntegerv(GL_VIEWPORT,         viewport);            // used for selectObj(smMarkerCreate)
                glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);    // used for selectObj(smMarkerCreate)
                glGetDoublev(GL_MODELVIEW_MATRIX,  markerViewMatrix);    // used for selectObj(smMarkerCreate)
        glPopMatrix();

        bShowCSline = bShowAxes;
        bShowFSline = bShowBoundingBox;

        prepareVol();

        if (!b_renderTextureLast) {
            renderVol();
        }

        if (sShowMarkers>0 || sShowSurfObjects>0)
        {
                if (polygonMode==1)	      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                else if (polygonMode==2)  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                else                      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                turn_off_specular();
                setObjLighting();
                turn_off_specular();

                if (sShowSurfObjects>0)
                {
                        glPushMatrix(); //================================================= SurfObject {

                        // original surface object space ==>fit in [-1,+1]^3
                        setSurfaceStretchSpace();
                        glPushName(dcSurface);
                                drawObj();  // neuron-swc, cell-apo, label-surf, etc
                        glPopName();

                        glPopMatrix(); //============================================================= }
                }

                if (sShowMarkers>0)
                {
                        glPushMatrix(); //===================================================== Marker {

                        // marker defined in original image space ==>fit in [-1,+1]^3
                        setMarkerSpace();
                        glPushName(dcSurface);
                                drawMarker();  // just markers
                        glPopName();

                        glPopMatrix(); //============================================================= }
                }

                disObjLighting();
        }

        if (! b_selecting)
        {
                if (bShowBoundingBox || bShowAxes)
                {
                        glPushMatrix(); //========================== default bounding frame & axes {

                        // bounding box space ==>fit in [-1,+1]^3
                        setObjectSpace();
                        drawBoundingBoxAndAxes(boundingBox, 1, 3);

                        glPopMatrix(); //========================================================= }
                }

                if (bShowBoundingBox2 && has_image() && !surfBoundingBox.isNegtive() )
                {
                        glPushMatrix(); //============================ surface object bounding box {

                        setSurfaceStretchSpace();
                        drawBoundingBoxAndAxes(surfBoundingBox, 1, 0);

                        glPopMatrix(); //========================================================= }
                }

            if (bOrthoView)
            {
                        glPushMatrix(); //============================================== scale bar {

                        drawScaleBar();

                        glPopMatrix(); //========================================================= }
            }
        }

     if (b_renderTextureLast) {
          renderVol();
     }

    if (bShowCornerAxes)
    {
        paint_corner_axes();
    }

    // must be at last
    if (! b_selecting && sShowTrack)
    {
            blendTrack();
    }

    return;
}


void RendererNeuronAnnotator::setupData(void* idep)
{
    qDebug("  RendererNeuronAnnotator::setupData");
    cleanData();

    // Begin assuming emptiness
    start1 = 0;
    start2 = 0;
    start3 = 0;
    start4 = 0;
    start5 = 0;
    size1 = dim1 = 0;
    size2 = dim2 = 0;
    size3 = dim3 = 0;
    size4 = dim4 = 0;
    size5 = dim5 = 0;
    sampleScaleX = sampleScaleY = sampleScaleZ = sampleScale[0] = sampleScale[1] = sampleScale[2] = sampleScale[3] = sampleScale[4] = 1;
    data4dp = NULL;

    // Try to get existing VolumeTexture data - otherwise exit
    vaa3d::VolumeTexture const * vt_ptr = volumeTexture;
    if (NULL == vt_ptr)
    {
        qDebug() << "setupData() called with NULL volumeTexture";
        return;
    }

    size1 = dim1 = vt_ptr->originalImageSize.x();
    size2 = dim2 = vt_ptr->originalImageSize.y();
    size3 = dim3 = vt_ptr->originalImageSize.z();
    size4 = dim4 = 4; // RGBA
    size5 = dim5 = 1;

    bufSize[0] = size1;
    bufSize[1] = size2;
    bufSize[2] = size3;
    bufSize[3] = size4;
    bufSize[4] = size5;

    b_limitedsize = (vt_ptr->originalImageSize != vt_ptr->usedTextureSize);
    if (b_limitedsize)
    {
        bufSize[0] = vt_ptr->paddedTextureSize.x();
        bufSize[1] = vt_ptr->paddedTextureSize.y();
        bufSize[2] = vt_ptr->paddedTextureSize.z();
        sampleScaleX = sampleScale[0] = (float)vt_ptr->usedTextureSize.x() / (float)vt_ptr->originalImageSize.x();
        sampleScaleY = sampleScale[1] = (float)vt_ptr->usedTextureSize.y() / (float)vt_ptr->originalImageSize.y();
        sampleScaleZ = sampleScale[2] = (float)vt_ptr->usedTextureSize.z() / (float)vt_ptr->originalImageSize.z();
        qDebug() << "  Down sampling to" << bufSize[0] << bufSize[1] << bufSize[2];
    }
    // TODO - implement local subvolume
    isSimulatedData = false;


    dataViewProcBox = dataBox = BoundingBox(start1, start2, start3, start1+(size1-1), start2+(size2-1), start3+(size3-1));

    // Pretend there is vaa3d-classic data
    // TODO - set to NULL
    My4DImage* image4d = v3dr_getImage4d(_idep);
    if (image4d)
        data4dp = image4d->getRawData();
    else
        data4dp = NULL;
    rgbaBuf = total_rgbaBuf = NULL;

    this->_idep = idep;
        rgbaBuf = total_rgbaBuf = new RGBA8[1]; // TODO token memory allocation until I can clear things up
    updateSettingsFromVolumeTexture(*volumeTexture);
}


void RendererNeuronAnnotator::setShowCornerAxes(bool b)
{
    if (b == bShowCornerAxes) return;
    bShowCornerAxes = b;
    emit showCornerAxesChanged(bShowCornerAxes);
}

void RendererNeuronAnnotator::paint_corner_axes()
{
    // Keep rotation of scene, but not scale nor translation.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT); // save color and depth test
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        double aspect = double(screenW)/MAX(screenH,1);
        gluPerspective(30.0, aspect, 1.0, 10.0);
        glMatrixMode(GL_MODELVIEW);
        GLdouble modelviewMatrix[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
        Rotation3D rotation(modelviewMatrix);
        Vector3D eulerAngles = rotation.convertBodyFixedXYZRotationToThreeAngles();
        eulerAngles *= 180.0 / 3.14159; // radians to degrees
        for (int i = 0; i < 3; ++i) {
            while (eulerAngles[i] < 0) eulerAngles[i] += 360.0;
            while (eulerAngles[i] >= 360.0) eulerAngles[i] -= 360.0;
        }
        glLoadIdentity();
        glTranslated(-1.1 * aspect, -1.1, 0); // move to corner
        glTranslated(0, 0, -viewDistance); // place axes in front of camera
        glScaled(0.2, 0.2, 0.2); // keep axes small in relation to screen
        // Apply scene rotation
        glRotated(eulerAngles[0], 1, 0, 0);
        glRotated(eulerAngles[1], 0, 1, 0);
        glRotated(eulerAngles[2], 0, 0, 1);
        glClear(GL_DEPTH_BUFFER_BIT); // draw over the existing scene
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(5.0);
        glBegin(GL_LINES);
            // thicker black lines
            glColor3f(0.1, 0.1, 0.1); // black
            glVertex3f(0, 0, 0); // x0
            glVertex3f(1, 0, 0); // x1
            glVertex3f(0, 0, 0); // y0
            glVertex3f(0, -1, 0); // y1
            glVertex3f(0, 0, 0); // z0
            glVertex3f(0, 0, -1); // z1
        glEnd();
        glClear(GL_DEPTH_BUFFER_BIT); // draw over the black lines
        glLineWidth(3.0);
        glBegin(GL_LINES);
            // Thinner colored lines
            glColor3f(0.8, 0.3, 0.3); // red
            glVertex3f(0, 0, 0); // x0
            glVertex3f(1, 0, 0); // x1
            glColor3f(0.3, 0.7, 0.3); // green
            glVertex3f(0, 0, 0); // y0
            glVertex3f(0, -1, 0); // y1
            glColor3f(0.3, 0.3, 0.9); // blue
            glVertex3f(0, 0, 0); // z0
            glVertex3f(0, 0, -1); // z1
        glEnd();
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}



