#include "../3drenderer/renderer_gl2.h"
#include "RendererNeuronAnnotator.h"
#include "../DataFlowModel.h"
#include "../3drenderer/v3dr_common.h"
#include "../3drenderer/v3dr_glwidget.h"

RendererNeuronAnnotator::RendererNeuronAnnotator(void* w)
    : QObject(NULL), Renderer_gl2(w)
{
    // qDebug() << "RendererNeuronAnnotator constructor" << this;
    neuronMask=0;
    texture3DSignal=0;
    texture3DBackground=0;
    texture3DReference=0;
    texture3DBlank=0;
    texture3DCurrent=0;
    textureSetAlreadyLoaded=false;
    masklessSetupStackTexture=false;

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
    loadShader(); // create color map for fast gamma
}

RendererNeuronAnnotator::~RendererNeuronAnnotator()
{
    // TODO - what about all those texture3Dwhatever?  When are those deleted?
    // qDebug() << "RendererNeuronAnnotator destructor" << this;
    cleanExtendedTextures(); // might delete texture3DCurrent
    if (neuronMask) {delete [] neuronMask; neuronMask = NULL;}
    if (texture3DReference) {delete [] texture3DReference; texture3DReference = NULL;}
    if (texture3DBlank) {delete [] texture3DBlank; texture3DBlank = NULL;}
    if (texture3DBackground) {delete [] texture3DBackground; texture3DBackground = NULL;}
    // NOTE that deleting texture3DSignal is handled by something else
}

void RendererNeuronAnnotator::loadVol() {
    Renderer_gl1::loadVol();
}

// This function resamples the original mask to the same dimensions
// as the texture buffer. It returns false if the source image is smaller
// than the target buffer because this isn't implemented.

bool RendererNeuronAnnotator::populateNeuronMaskAndReference(const My4DImage* my4Dmask, const My4DImage* referenceImage)
{
    // qDebug() << "Start RendererNeuronAnnotator::populateNeuronMask()";

    // Clean previous data
    if (neuronMask!=0) {
        delete [] neuronMask;
        neuronMask = NULL;
    }

    if (my4Dmask==0) {  
        qDebug() << "RendererNeuronAnnotator::populateNeuronMask() can't process NULL my4Dmask";
        return false;
    }

    int sourceX=my4Dmask->getXDim();
    int sourceY=my4Dmask->getYDim();
    int sourceZ=my4Dmask->getZDim();

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

    for (iz = 0; iz < sourceZ; iz++)
        for (iy = 0; iy < sourceY; iy++)
            for (ix = 0; ix < sourceX; ix++) {
                ox = CLAMP(0,realX-1, IROUND(ix/sx));
                oy = CLAMP(0,realY-1, IROUND(iy/sy));
                oz = CLAMP(0,realZ-1, IROUND(iz/sz));

                V3DLONG offset = oz*realX*realY + oy*realX + ox;
                if (neuronMask[offset]==0) {
                    neuronMask[offset] = my4Dmask->at(ix,iy,iz);
                }
                RGBA8 referenceVoxel;
                double referenceIntensity=referenceImage->at(ix,iy,iz);
                referenceVoxel.r=referenceIntensity;
                referenceVoxel.g=referenceIntensity;
                referenceVoxel.b=referenceIntensity;
                referenceVoxel.a=255;
                texture3DReference[offset] = referenceVoxel;
    }

    // qDebug() << "RendererNeuronAnnotator::populateNeuronMask() done";

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

    texture3DBlank=new RGBA8[realZ*realY*realX];
    texture3DBackground=new RGBA8[realZ*realY*realX];

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

// This is originally based on version from Renderer_gl1
void RendererNeuronAnnotator::setupStackTexture(bool bfirst)
{
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
}

bool RendererNeuronAnnotator::initializeTextureMasks() {

    if (!populateBaseTextures()) {
        return false;
    }

    QList<int> initialMaskList;
    for (int i=1;i<256;i++) {
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

    for (int stack_i=1; stack_i<=3; stack_i++)
    {

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

            MESSAGE_ASSERT(imageX>=realX && imageY>=realY && imageZ>=realZ);
            MESSAGE_ASSERT(COPY_X>=realX && COPY_Y>=realY && COPY_Z>=realZ);
//		sw+=1, sh+=1;  // to get rid of artifacts at sub-tex border // or use NPT tex
//		if (sw>w) sw = w;
//		if (sh>h) sh = h;

            for (int i = 0; i < n_slice; i++)

            {
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
                        QCoreApplication::processEvents(/* QEventLoop::ExcludeUserInputEvents */);
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

            if (sw%TILE_DIMENSION!=0 || sh%TILE_DIMENSION!=0) {
                qDebug() << "RendererNeuronAnnotator::updateCurrentTextureMask() ERROR: width and height of texture buffer must both be divisible by "
                        << TILE_DIMENSION << "! width=" << sw << " height=" << sh << " realX=" << realX << " realY=" << realY << " realZ=" << realZ;
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
                        QCoreApplication::processEvents(/* QEventLoop::ExcludeUserInputEvents */);
                        makeCurrent();
                    }
            }
    }
    int mSec=timer.elapsed();
    qDebug() << "RendererNeuronAnnotator::updateCurrentTextureMask() finished in " << mSec << " milliseconds  tilesIncluded=" << glTilesIncluded <<" tilesExcluded=" << glTilesExcluded;
    emit progressComplete();
}


// neuron annotator mouse right click pop menu
/* virtual */
int RendererNeuronAnnotator::hitMenu(int x, int y, bool b_glwidget)
{
    makeCurrent(); // make sure in correct OpenGL context

    if (b_selecting)  return 0;  // prevent re-enter
    //if (selectMode >smObject && pTip) return 0; //prevent tool-tip when definition

    b_selecting = true;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLuint zDepth;
    glReadPixels(x,viewport[3]-y,1,1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &zDepth);

#define PICK_BUFFER_SIZE 10240  //// CAUTION: pick buffer overflow may cause crash
#define PICK_TOLERANCE 5

    GLuint *selectBuf = new GLuint[PICK_BUFFER_SIZE]; //
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

            paint(); //

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glFlush();
    }

    b_selecting = false;
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    // right click popup menu
    QList<QAction*> listAct;
    QAction* act=0;
    QAction* actViewNeuronOnly=0;
    QAction* actViewNeuronWithBackground=0;
    QAction* actViewNeuronWithReference=0;
    QAction* actViewNeuronWithBackgroundAndReference=0;

    QAction* actClearAllSelections=0;
    QAction* actViewAllNeurons=0;
    QAction* actClearAllNeurons=0;

    int hits = glRenderMode(GL_RENDER);
    if (hits==0 && b_glwidget)
    {

        listAct.append(actViewAllNeurons = new QAction("view all neurons in empty space", w));
        listAct.append(actClearAllSelections = new QAction("clear all", w));
        listAct.append(actClearAllNeurons = new QAction("clear all contents", w));

        if (w) w->updateGL(); //for highlight object

        QMenu menu;
        foreach (QAction* a, listAct) {  menu.addAction(a); }
        //menu.setWindowOpacity(POPMENU_OPACITY); // no effect on MAC? on Windows cause blink
        act = menu.exec(QCursor::pos());

        if (act==0) 	return 0;
        else if (act == actViewAllNeurons)
        {
            if(w)
            {
                QList<int> overlayList;
                emit w->triggerNeuronShownAll(overlayList); // overlayList should be empty
            }
        }
        else if (act == actClearAllSelections)
        {
            if (w)
            {
                emit w->triggerNeuronClearAllSelections();
            }
        }
        else if (act == actClearAllNeurons)
        {
            if(w)
            {
                emit w->triggerNeuronClearAll();
            }
        }

        //
        delete[] selectBuf;

        return 0;
    }

    //
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

    //int ret = processHit((int)nameLength, hitNames, x, y, b_menu, pTip); //

    // object name string
    QString qsName;
    QString qsInfo;

#define IS_VOLUME() 	(nameLength>=3 && hitNames[0]==dcVolume)
#define IS_SURFACE() 	(nameLength>=3 && hitNames[0]==dcSurface)
#define LIST_SELECTED(list, i, s) \
{ \
        if (i>=0 && i<list.size()) \
                list[i].selected = s; \
}
#define LIST_ON(list, i, s) \
{ \
        if (i>=0 && i<list.size()) \
                list[i].on = s; \
}
#define LIST_COLOR(list, i, w) \
{ \
        if (i>=0 && i<list.size()) \
        { \
                QColor qc = QColorFromRGBA8(list[i].color); \
                if (v3dr_getColorDialog( &qc, w))  list[i].color = RGBA8FromQColor( qc ); \
        } \
}

    lastSliceType = vsSliceNone;
    if (IS_VOLUME() || !b_glwidget) // volume
    {
        QString bound_info = QString("(%1 %2 %3 -- %4 %5 %6)").arg(start1+1).arg(start2+1).arg(start3+1).arg(start1+size1).arg(start2+size2).arg(start3+size3);
        QString data_title = "";	//if (w) data_title = QFileInfo(w->getDataTitle()).fileName();
        (qsName = QString("volume %1 ... ").arg(bound_info) +(data_title));
        //qsName += QString("%1").arg(names[2]);
        lastSliceType = hitNames[2]; //100731
        //qDebug()<<"sliceType:"<<currentSliceType;
    }
    if (IS_SURFACE()) // surface object
    {
            switch (hitNames[1])
            {
            case stImageMarker: {//marker
                    (qsName = QString("marker #%1 ... ").arg(hitNames[2]) + listMarker.at(hitNames[2]-1).name);
                    LIST_SELECTED(listMarker, hitNames[2]-1, true);

                    qsInfo = info_Marker(hitNames[2]-1);
            }break;

            case stLabelSurface: {//label surface
                    (qsName = QString("label surface #%1 ... ").arg(hitNames[2]) + listLabelSurf.at(hitNames[2]-1).name);
                    LIST_SELECTED(listLabelSurf, hitNames[2]-1, true);

                    int vertex_i=0;
                    Triangle * T = findNearestSurfTriangle_WinXY(x, y, vertex_i, (Triangle*)list_listTriangle.at(hitNames[2]-1));
                    qsInfo = info_SurfVertex(vertex_i, T, listLabelSurf.at(hitNames[2]-1).label);
            }break;

            case stNeuronStructure: {//swc
                    (qsName = QString("neuron/line #%1 ... ").arg(hitNames[2]) + listNeuronTree.at(hitNames[2]-1).name);
                    LIST_SELECTED(listNeuronTree, hitNames[2]-1, true);

                    if (listNeuronTree.at(hitNames[2]-1).editable) qsName += " (editing)";
                    NeuronTree *p_tree = (NeuronTree *)&(listNeuronTree.at(hitNames[2]-1));
                    qsInfo = info_NeuronNode(findNearestNeuronNode_WinXY(x, y, p_tree), p_tree);
            }break;

            case stPointCloud: {//apo
                    (qsName = QString("point cloud #%1 ... ").arg(hitNames[2]) + listCell.at(hitNames[2]-1).name);
                    LIST_SELECTED(listCell, hitNames[2]-1, true);
            }break;

            }
    }
    //qDebug() <<"\t Hit " << (qsName);

    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    // Menu Act Responses
    if (qsName.size()>0)
    {
            if (IS_VOLUME() || !b_glwidget)
            {
                if (b_glwidget) {
                    listAct.append(actViewAllNeurons = new QAction("view all neurons in empty space", w));
                }

                listAct.append(actViewNeuronOnly = new QAction("view only this neuron in empty space", w));

                actViewNeuronOnly->setIcon(QIcon(":/icons/neuronwobg.svg"));
                actViewNeuronOnly->setVisible(true);
                actViewNeuronOnly->setIconVisibleInMenu(true);

                listAct.append(actViewNeuronWithBackground = new QAction("view only this neuron with background", w));

                actViewNeuronWithBackground->setIcon(QIcon(":/icons/neuronwbg.svg"));
                actViewNeuronWithBackground->setVisible(true);
                actViewNeuronWithBackground->setIconVisibleInMenu(true);

                listAct.append(actViewNeuronWithReference = new QAction("view only this neuron with reference", w));

                actViewNeuronWithReference->setIcon(QIcon(":/icons/neuronwref.svg"));
                actViewNeuronWithReference->setVisible(true);
                actViewNeuronWithReference->setIconVisibleInMenu(true);

                listAct.append(actViewNeuronWithBackgroundAndReference = new QAction("view only this neuron with background and reference", w));

                actViewNeuronWithBackgroundAndReference->setIcon(QIcon(":/icons/neuronwbgref.svg"));
                actViewNeuronWithBackgroundAndReference->setVisible(true);
                actViewNeuronWithBackgroundAndReference->setIconVisibleInMenu(true);

                if (b_glwidget) {
                    listAct.append(actClearAllSelections = new QAction("clear all", w));
                }

            }

            if (w) w->updateGL(); //for highlight object

            QMenu menu;
            foreach (QAction* a, listAct) {  menu.addAction(a); }
            //menu.setWindowOpacity(POPMENU_OPACITY); // no effect on MAC? on Windows cause blink
            act = menu.exec(QCursor::pos());
    }

    // processing menu actions
    QList<int> overlayList;
    if (act==0) 	{
        return 0; // 081215: fix pop dialog when no choice of menu
    }
    else if (act == actViewNeuronWithBackground)
    {
        if(w)
        {
            overlayList.append(1); // 1 == background

            if(!b_glwidget && w->getNeuronIndex()>=0)
            {
                emit w->triggerNeuronIndexChanged(w->getNeuronIndex());
            }

            emit w->triggerNeuronShown(overlayList);
        }
    }
    else if (act == actViewNeuronWithReference)
    {
        if(w)
        {
            overlayList.append(0); // 0 == reference

            if(!b_glwidget && w->getNeuronIndex()>=0)
            {
                emit w->triggerNeuronIndexChanged(w->getNeuronIndex());
            }

            emit w->triggerNeuronShown(overlayList);
        }
    }
    else if (act == actViewNeuronWithBackgroundAndReference)
    {
        if(w)
        {
            overlayList.append(0); // 0 == reference
            overlayList.append(1); // 1 == background

            if(!b_glwidget && w->getNeuronIndex()>=0)
            {
                emit w->triggerNeuronIndexChanged(w->getNeuronIndex());
            }

            emit w->triggerNeuronShown(overlayList);
        }
    }
    else if (act == actViewNeuronOnly)
    {
        if(w)
        {
            if(!b_glwidget && w->getNeuronIndex()>=0)
            {
                emit w->triggerNeuronIndexChanged(w->getNeuronIndex());
            }
            emit w->triggerNeuronShown(overlayList); // overlayList should be empty
        }
    }
    else if (act == actViewAllNeurons)
    {
        if(w)
        {
            emit w->triggerNeuronShownAll(overlayList); // overlayList should be empty
        }
    }
    else if (act == actClearAllSelections)
    {
        if (w)
        {
            emit w->triggerNeuronClearAllSelections();
        }
    }


    //
    delete[] selectBuf;
    delete[] hitNames;

    return 0;
}


RGBA8* RendererNeuronAnnotator::getOverlayTextureByAnnotationIndex(int index) {
    if (index==DataFlowModel::REFERENCE_MIP_INDEX) {
        return texture3DReference;
    } else if (index==DataFlowModel::BACKGROUND_MIP_INDEX) {
        return texture3DBackground;
    }
}

