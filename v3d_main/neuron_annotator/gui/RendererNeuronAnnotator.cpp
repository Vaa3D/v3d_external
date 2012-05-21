#include "../3drenderer/renderer_gl2.h"
#include "RendererNeuronAnnotator.h"
#include "../DataFlowModel.h"
#include "../3drenderer/v3dr_common.h"
#include "../3drenderer/v3dr_glwidget.h"
#include "../geometry/Rotation3D.h"

RendererNeuronAnnotator::RendererNeuronAnnotator(void* w)
    : QObject(NULL)
    , Renderer_gl2(w)
    , stereo3DMode(STEREO_OFF)
    , bStereoSwapEyes(false)
    , bShowCornerAxes(true)
    , bShowClipGuide(false)
    , slabThickness(1000)
    , slabDepth(0)
{
    // qDebug() << "RendererNeuronAnnotator constructor" << this;

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
    // VolumeTexture manages the texture memory, not base class.  So set to NULL before it gets a chance to clear it.
    Xslice_data = Yslice_data = Zslice_data = NULL;
    Xtex_list = Ytex_list = Ztex_list = NULL;
}

void RendererNeuronAnnotator::setUndoStack(QUndoStack& undoStackParam)
{
    customClipPlanes.setUndoStack(undoStackParam);
}

void RendererNeuronAnnotator::applyCustomCut(const CameraModel& cameraModel)
{
    Rotation3D R_obj_eye = cameraModel.rotation().transpose(); // rotation to convert eye coordinates to object coordinates
    Vector3D down_obj = R_obj_eye * Vector3D(0, 1, 0);
    applyCutPlaneInImageFrame(cameraModel.focus(), down_obj);
}

void RendererNeuronAnnotator::applyCutPlaneInImageFrame(Vector3D point, Vector3D direction)
{
    // cout << point << direction << endl;
    // convert focus to lie within unit cube, to match opengl transforms used.
    point.x() = point.x() / (dim1);
    point.y() = 1.0 - point.y() / (dim2);
    point.z() = 1.0 - point.z() / (dim3);
    // skew direction by scaled axes
    direction.x() *= dim1 * thicknessX;
    direction.y() *= dim2 * thicknessY;
    direction.z() *= dim3 * thicknessZ;
    direction /= direction.norm(); // convert to unit length
    direction.x() *= -1; // ?
    double distance = point.dot(direction);
    customClipPlanes.addPlane(direction.x(), direction.y(), direction.z(), -distance);
}

void RendererNeuronAnnotator::clipSlab(const CameraModel& cameraModel) // Apply clip plane to current slab
{
    Rotation3D R_obj_eye = cameraModel.rotation().transpose(); // rotation to convert eye coordinates to object coordinates
    Vector3D viewDirection = R_obj_eye * Vector3D(0, 0, 1);
    // cout << "view direction = " << viewDirection << endl;
    // Plane equation for front clip
    Vector3D direction1 = viewDirection;
    Vector3D point1 = cameraModel.focus() + viewDirection * (viewDistance - viewNear) / glUnitsPerImageVoxel();
    // Plane equation for back clip
    Vector3D direction2 = viewDirection * -1;
    Vector3D point2 = cameraModel.focus() - viewDirection * (viewFar - viewDistance) / glUnitsPerImageVoxel();
    // cout << cameraModel.focus() << ", " << viewDistance << ", " << viewNear << endl;
    // cout << point1 << direction1 << endl;
    // cout << point2 << direction2 << endl;

    applyCutPlaneInImageFrame(point1, direction1);
    applyCutPlaneInImageFrame(point2, direction2);
}

/* slot */
void RendererNeuronAnnotator::setStereoMode(int m)
{
    qDebug() << "Stereo mode = " << (Stereo3DMode) m << __FILE__ << __LINE__;
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
            // glGetIntegerv(GL_MAX_CLIP_PLANES, &maxGLClipPlanes);
            bool bUseClassicV3dShader = false;
            QString volVertexShaderName(":/neuron_annotator/resources/color_vertex_cmb.txt");
            QString volFragmentShaderName(":/neuron_annotator/resources/tex_fragment_cmb.txt");
            if (bUseClassicV3dShader)
                 volFragmentShaderName = ":/shader/tex_fragment.txt";
            QString defClip = QString("#version 120\n#define MaxClipPlanes %1\n").arg(customClipPlanes.size());
            // qDebug() << defClip;
            linkGLShader(SMgr, shaderTex2D,
                         Q_CSTR(
                                defClip +
                                resourceTextFile(volVertexShaderName)),
                         Q_CSTR(
                                defClip +
                                QString("#undef TEX3D \n") + deftexlod + resourceTextFile(volFragmentShaderName)));

            qDebug("+++++++++ shader for Volume texture3D");
            linkGLShader(SMgr, shaderTex3D,
                         Q_CSTR(
                                defClip +
                                resourceTextFile(volVertexShaderName)),
                         Q_CSTR(defClip +
                                QString("#define TEX3D \n") + deftexlod + resourceTextFile(volFragmentShaderName)));

    }
    catch (...) {
        qDebug() << "ERROR: shader loading failed";
    }

    qDebug("+++++++++ GLSL shader setup finished.");

    glGenTextures(1, &texColormap);
    initColormap();
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
                shader->setUniform1i("neuronVisibility", 2); // GL_TEXTURE2, 2D 256x256 neuron index
                shader->setUniform1i("neuronLabel", 3);

                // float n = FILL_CHANNEL-1; // 0-based
                float n = 4.0;
                shader->setUniform4f("channel", 0.0/n, 1.0/n, 2.0/n, 3.0/n);
                shader->setUniform1i("blend_mode", renderMode);
                shader->setUniform1i("format_bgra", format_bgra);

                // Hard coded first clip plane for testing
                for (int p = 0; p < customClipPlanes.size(); ++p)
                {
                    const double* v = &customClipPlanes[p][0];
                    QString varStr = QString("clipPlane[%1]").arg(p);
                    shader->setUniform4f(varStr.toStdString().c_str(), v[0], v[1], v[2], v[3]);
                }

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

                // TODO - does uploadPixels need to happen after shader->begin() like this?
                /*
                if (neuronVisibilityTexture && neuronVisibilityTexture->bNeedsUpload)
                {
                    neuronVisibilityTexture->uploadPixels();
                }
                 */

                glActiveTextureARB(GL_TEXTURE2_ARB); // neuron visibility
                glEnable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE3_ARB); // neuron label
                glEnable(GL_TEXTURE_3D);
                glActiveTextureARB(GL_TEXTURE0_ARB);

                // switch back to volume data texture
                glActiveTextureARB(GL_TEXTURE0_ARB); // volume
                glEnable(GL_TEXTURE_2D);
        }
}

void RendererNeuronAnnotator::shaderTexEnd()
{
        if (tryVolShader && shader && !b_selecting)
        {
                // off colormap
                glActiveTextureARB(GL_TEXTURE0_ARB); // volume
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE1_ARB); // color map
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE2_ARB); // neuron visibility
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE3_ARB); // neuron label
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

/*
void RendererNeuronAnnotator::setDepthClip(float totalDepthInGlUnits)
{
    if (totalDepthInGlUnits <= 0) return;
    double minFront = viewDistance - 0.3 * totalDepthInGlUnits;
    if (minFront < 0.5) minFront = 0.5;
    double maxBack = viewDistance + 0.7 * totalDepthInGlUnits;
    if (maxBack <= minFront) maxBack = minFront;
    double range = maxBack - minFront;

    viewNear = minFront + relativeFrontClip * range;
    viewFar = minFront + relativeBackClip * range;
}
*/

void RendererNeuronAnnotator::updateDepthClip()
{
    // qDebug() << "RendererNeuronAnnotator::updateDepthClip()";
    double minFront = viewDistance - 0.3 * slabThickness * glUnitsPerImageVoxel();
    if (minFront < 0.5) minFront = 0.5;
    double maxBack = viewDistance + 0.7 * slabThickness * glUnitsPerImageVoxel();
    if (maxBack <= minFront) maxBack = minFront;

    viewNear = minFront;
    viewFar = maxBack;
    // qDebug() << viewNear << viewFar;
}

/* slot */
bool RendererNeuronAnnotator::setSlabThickness(int val) // range 2-1000
{
    // qDebug() << "RendererNeuronAnnotator::setSlabThickness" << val;
    // check range
    if (val < 2) val = 2;
    if (val > 1000) val = 1000;
    // check whether value changed
    if (val == slabThickness)
        return false; // unchanged
    slabThickness = val;
    updateDepthClip();
    emit slabThicknessChanged(slabThickness);
    return true;
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
        // Do not include reference channel
        int maxChan = dim4 - 1;
        if (maxChan == 3) maxChan = 2;
        for(chno=0; chno <= maxChan; chno++)
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

            // qDebug()<<"chno ..."<<chno<<"dim4 ..."<<dim4;

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

    if ( bufSize[3]<1 ) return; // no image data, 081002

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


            CHECK_GLErrorString_throw(); // can throw const char* exception, RZC 080925

            int X = _getBufFillSize(imageX);
            int Y = _getBufFillSize(imageY);
            int Z = _getBufFillSize(imageZ);

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
}

// This is originally based on version from Renderer_gl1
void RendererNeuronAnnotator::setupStackTexture(bool bfirst)
{
}

// Sets various size-related internal variables
// Try to call this whenever setupData() is called
void RendererNeuronAnnotator::updateSettingsFromVolumeTexture(
        const jfrc::VolumeTexture::Reader& textureReader)
{
    // Set values as if empty volume
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
    sampleScaleX = sampleScaleY = sampleScaleZ = sampleScale[0] = sampleScale[1] = sampleScale[2] = sampleScale[3] = sampleScale[4] = 1.0;

    // Set (subset of) values as if full size unresampled volume
    size1 = dim1 = textureReader.originalImageSize().x();
    size2 = dim2 = textureReader.originalImageSize().y();
    size3 = dim3 = textureReader.originalImageSize().z();
    size4 = dim4 = 4; // RGBA
    size5 = dim5 = 1;
    bufSize[0] = size1;
    bufSize[1] = size2;
    bufSize[2] = size3;
    bufSize[3] = size4;
    bufSize[4] = size5;
    boundingBox.x0 = boundingBox.y0 = boundingBox.z0 = 0.0;
    boundingBox.x1 = textureReader.originalImageSize().x();
    boundingBox.y1 = textureReader.originalImageSize().y();
    boundingBox.z1 = textureReader.originalImageSize().z();
    VOL_X0 = VOL_Y0 = VOL_Z0 = 0;
    VOL_X1 = VOL_Y1 = VOL_Z1 = 1;
    dataViewProcBox = dataBox = BoundingBox(start1, start2, start3, start1+(size1-1), start2+(size2-1), start3+(size3-1));

    // Set (subset of) values using resampled size
    realX = textureReader.usedTextureSize().x();
    realY = textureReader.usedTextureSize().y();
    realZ = textureReader.usedTextureSize().z();
    safeX = realX; // necessary
    safeY = realY;
    safeZ = realZ;
    imageX = textureReader.usedTextureSize().x();
    imageY = textureReader.usedTextureSize().y();
    imageZ = textureReader.usedTextureSize().z();
    b_limitedsize = (textureReader.originalImageSize() != textureReader.usedTextureSize());
    if (b_limitedsize)
    {
        bufSize[0] = textureReader.paddedTextureSize().x();
        bufSize[1] = textureReader.paddedTextureSize().y();
        bufSize[2] = textureReader.paddedTextureSize().z();
        sampleScaleX = sampleScale[0] = (float)textureReader.usedTextureSize().x() / (float)textureReader.originalImageSize().x();
        sampleScaleY = sampleScale[1] = (float)textureReader.usedTextureSize().y() / (float)textureReader.originalImageSize().y();
        sampleScaleZ = sampleScale[2] = (float)textureReader.usedTextureSize().z() / (float)textureReader.originalImageSize().z();
        // qDebug() << "  Down sampling to" << bufSize[0] << bufSize[1] << bufSize[2];
    }

    // Copy pointers to openGL texture ID lists
    Xtex_list = const_cast<GLuint*>(textureReader.Xtex_list());
    Ytex_list = const_cast<GLuint*>(textureReader.Ytex_list());
    Ztex_list = const_cast<GLuint*>(textureReader.Ztex_list());

    // TODO - implement local subvolume
}

/* virtual */
int RendererNeuronAnnotator::_getBufFillSize(int w)
{
    return jfrc::Dimension::padToMultipleOf(w, 8);
}

/* virtual */
int RendererNeuronAnnotator::_getTexFillSize(int w)
{
    return jfrc::Dimension::padToMultipleOf(w, 8);
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

// Even numbered rows ON
static GLubyte rowStipple0[] = {
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};

// Odd numbered rows ON
static GLubyte rowStipple1[] = {
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff};

// Even numbered columns ON
static GLubyte colStipple0[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

// Odd numbered columns ON
static GLubyte colStipple1[] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

// Even/Even and Odd/Odd pixels ON
static GLubyte checkStipple0[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};

// Even/Odd and Odd/Even pixels ON
static GLubyte checkStipple1[] = {
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA};

class RowInterleavedStereoView : public StereoEyeView
{
public:
    // Remember to call fillStencil() once before rendering
    RowInterleavedStereoView(Eye eye, Eye eyeGeom, GLubyte* stipple=rowStipple0) : StereoEyeView(eye, eyeGeom)
        , stipple(stipple)
    {
        glPushAttrib(GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // don't modify the stencil buffer
        glEnable(GL_STENCIL_TEST);

        // Left image in the zero regions
        if (eye == LEFT)
            glStencilFunc( GL_EQUAL, 0, 1 );
        else
            glStencilFunc( GL_NOTEQUAL, 0, 1 );
        CHECK_GLErrorString_throw();
    }

    ~RowInterleavedStereoView()
    {
        glPopAttrib();
    }

    void fillStencil(RendererNeuronAnnotator& renderer)
    {
        int width = renderer.getScreenWidth();
        int height = renderer.getScreenHeight();

        // Remember current OpenGL state
        glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        // Modify only the stencil buffer
        glDisable(GL_LIGHTING); // no shading
        glDisable(GL_DEPTH_TEST); // don't modify depth buffer
        glDisable(GL_BLEND);
        glDrawBuffer(GL_BACK);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // don't modify color buffer
        glDepthMask(GL_FALSE);
        glEnable(GL_STENCIL_TEST);

        // Draw a rectangle over the full screen, into the stencil buffer
        // using the simplest possible OpenGL geometry
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glViewport(0, 0, width, height);
        glOrtho(0.0, width, 0.0, height, -1.0, 1.0); // 2D orthographic projection

        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); // Modify the stencil buffer everywhere
        glStencilFunc(GL_ALWAYS, 1, 1);
        glColor4f(1,1,1,0); // All ones
        // Use stippling trick to get the screen-space mask we need
        glEnable(GL_POLYGON_STIPPLE);
        glPolygonStipple(stipple);
        glRecti(0, 0, width, height);

        // Restore OpenGL state
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glPopAttrib();
    }

    const GLubyte* stipple;
};

class CheckerInterleavedStereoView : public RowInterleavedStereoView
{
public:
    CheckerInterleavedStereoView(Eye eye, Eye eyeGeom, GLubyte* stipple=checkStipple0)
        : RowInterleavedStereoView(eye, eyeGeom, stipple) {}
};

/* virtual */
void RendererNeuronAnnotator::paint()
{
    makeCurrent();
    switch(stereo3DMode)
    {
    case STEREO_OFF:
        glDrawBuffer(GL_BACK); // Avoid flicker on non-Quadro Mac
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
    case STEREO_ROW_INTERLEAVED:
        {
            {
                RowInterleavedStereoView v(StereoEyeView::LEFT, bStereoSwapEyes? StereoEyeView::RIGHT : StereoEyeView::LEFT);
                v.fillStencil(*this);
                paint_mono();
            }
            {
                RowInterleavedStereoView v(StereoEyeView::RIGHT, bStereoSwapEyes? StereoEyeView::LEFT : StereoEyeView::RIGHT);
                // DO NOT CLEAR
                paint_mono(false);
            }
            break;
        }    
    case STEREO_CHECKER_INTERLEAVED:
        {
            {
                CheckerInterleavedStereoView v(StereoEyeView::LEFT, bStereoSwapEyes? StereoEyeView::RIGHT : StereoEyeView::LEFT);
                v.fillStencil(*this);
                paint_mono();
            }
            {
                CheckerInterleavedStereoView v(StereoEyeView::RIGHT, bStereoSwapEyes? StereoEyeView::LEFT : StereoEyeView::RIGHT);
                // DO NOT CLEAR
                paint_mono(false);
            }
            break;
        }


    default:
        qDebug() << "Error: Unsupported Stereo mode" << stereo3DMode;
        paint_mono();
        break;
    }
}

// Copied from Renderer_gl1::paint() 27 Sep 2011 CMB
void RendererNeuronAnnotator::paint_mono(bool clearColorFirst)
{
        // turn_off_specular();
        //qDebug(" Renderer_gl1::paint(renderMode=%i)", renderMode);

        if (b_error) return; //080924 try to catch the memory error

        if (clearColorFirst) {
            glClearColor(color_background.r, color_background.g, color_background.b, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        glClearDepth(1);
        glDepthRange(0, 1);
        glClear(GL_DEPTH_BUFFER_BIT);

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
        paintCornerAxes();
    }

    if (bShowClipGuide)
    {
        paintClipGuide();
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
    // cleanData();

    isSimulatedData = false;

    // TODO - set image4d to NULL, after refactoring neuron clicking to not use image4d
    My4DImage* image4d = v3dr_getImage4d(_idep);
    if (image4d)
        data4dp = image4d->getRawData();

    this->_idep = idep;
    if (NULL == rgbaBuf)
        rgbaBuf = total_rgbaBuf = new RGBA8[1]; // TODO token memory allocation until I can clear things up

    image_format = GL_BGRA;
    image_type = GL_UNSIGNED_INT_8_8_8_8_REV;
    tryTexCompress = false;
}

void RendererNeuronAnnotator::setShowCornerAxes(bool b)
{
    if (b == bShowCornerAxes) return;
    bShowCornerAxes = b;
    emit showCornerAxesChanged(bShowCornerAxes);
}

// Draw a yellow line accross the screen for setting user clip plane
void RendererNeuronAnnotator::paintClipGuide()
{
    glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT); // save color and depth test
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST); // draw over the existing scene
    glBegin(GL_LINES);
        glColor3f(0.9, 0.9, 0.4); // yellow
        glVertex3f(-2, 0, 0);
        glVertex3f(+2, 0, 0);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void RendererNeuronAnnotator::paintCornerAxes()
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
            glVertex3f(0, 1, 0); // y1
            glVertex3f(0, 0, 0); // z0
            glVertex3f(0, 0, 1); // z1
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
            glVertex3f(0, 1, 0); // y1
            glColor3f(0.3, 0.3, 0.9); // blue
            glVertex3f(0, 0, 0); // z0
            glVertex3f(0, 0, 1); // z1
        glEnd();
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

