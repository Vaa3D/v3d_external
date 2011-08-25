#include "../3drenderer/renderer_gl2.h"
#include "RendererNeuronAnnotator.h"
#include "../AnnotationSession.h"

RendererNeuronAnnotator::RendererNeuronAnnotator(void* w)
    : QObject(NULL), Renderer_gl2(w)
{
    neuronMask=0;
    texture3DSignal=0;
    texture3DBackground=0;
    texture3DReference=0;
    texture3DBlank=0;
    texture3DCurrent=0;
    textureSetAlreadyLoaded=false;
    masklessSetupStackTexture=false;

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

    // Using compressed textures makes the transitions between texture sets much worse looking.
    // especially at high zoom levels.  So turn off compression.
    // On the other hand, uncompressed textures seem to cause some kind of swapping, so compress...
    // tryTexCompress = 0;

    setRenderTextureLast(true);

    // create display lists for markers
    loadObj();
}

RendererNeuronAnnotator::~RendererNeuronAnnotator() {

}

void RendererNeuronAnnotator::loadVol() {
    Renderer_gl1::loadVol();
}

// This function resamples the original mask to the same dimensions
// as the texture buffer. It returns false if the source image is smaller
// than the target buffer because this isn't implemented.

bool RendererNeuronAnnotator::populateNeuronMaskAndReference(const My4DImage* my4Dmask, const My4DImage* referenceImage) {

    qDebug() << "Start RendererNeuronAnnotator::populateNeuronMask()";

    // Clean previous data
    if (neuronMask!=0) {
        delete [] neuronMask;
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

    if (texture3DReference!=0)
        delete [] texture3DReference;

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

    qDebug() << "RendererNeuronAnnotator::populateNeuronMask() done";

    return true;
}


bool RendererNeuronAnnotator::populateBaseTextures() {

    qDebug() << "RendererNeuronAnnotator::populateBaseTextures() start";

    // Sanity check
    if (texture3DSignal==0) {
        qDebug() << "RendererNeuronAnnotator::populateBaseTextures() requires that texture3DSignal be pre-populated";
        return false;
    }

    // Clean
    if (texture3DBlank!=0)
        delete [] texture3DBlank;
    if (texture3DBackground!=0)
        delete [] texture3DBackground;

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

    qDebug() << "RendererNeuronAnnotator::populateBaseTextures() end";

    return true;
}

// This is originally based on version from Renderer_gl1
void RendererNeuronAnnotator::setupStackTexture(bool bfirst)
{
    qDebug() << "RendererNeuronAnnotator::setupStackTexture() - start";

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

        qDebug("	texture:   real = %dx%dx%d   fill = %dx%dx%d",  realX, realY, realZ,  fillX, fillY, fillZ);
    }

    qDebug() << "RendererNeuronAnnotator::setupStackTexture() - end";
}

bool RendererNeuronAnnotator::initializeTextureMasks() {

    if (!populateBaseTextures()) {
        return false;
    }

    QList<int> initialMaskList;
    for (int i=1;i<256;i++) {
        initialMaskList.append(i);
    }

    emit progressMessage(QString("Setting up textures"));

    QList<RGBA8*> initialOverlayList;
    initialOverlayList.append(texture3DBackground);
    rebuildFromBaseTextures(initialMaskList, initialOverlayList);

    return true;
}


// This function assumes the size realX,Y,Z should be used
void RendererNeuronAnnotator::load3DTextureSet(RGBA8* tex3DBuf)
{
    makeCurrent();
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

                    // attempt to get away from that horrible horrible hyper-modal stack-drilling unthreadable progress dialog
                    if (! (sliceCount % 20)) {
                        emit progressAchieved(prog);
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

void RendererNeuronAnnotator::cleanExtendedTextures() {
    if (texture3DCurrent!=0 &&
        texture3DCurrent!=texture3DSignal &&
        texture3DCurrent!=texture3DBackground &&
        texture3DCurrent!=texture3DReference &&
        texture3DCurrent!=texture3DBlank) {
        delete [] texture3DCurrent;
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
}

RGBA8* RendererNeuronAnnotator::extendTextureFromMaskList(const QList<RGBA8*> & sourceTextures, const QList<int> & maskIndexList) {
    qDebug() << "RendererNeuronAnnotator::extendTextureFromMaskList() start";
    RGBA8* newTexture=new RGBA8[realZ*realY*realX];
    int* quickList=new int[256];
    for (int m=0;m<256;m++) {
        quickList[m]=-1;
    }
    for (int m=0;m<maskIndexList.size();m++) {
        int value=maskIndexList.at(m);
        if (value>254) {
            qDebug() << "Error: ignoring mask entry greater than 254";
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
    qDebug() << "RendererNeuronAnnotator::extendTextureFromMaskList() done";
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

    qDebug() << "RendererNeuronAnnotator::updateCurrentTextureMask() start";
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
                        emit progressAchieved(prog);
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

RGBA8* RendererNeuronAnnotator::getOverlayTextureByAnnotationIndex(int index) {
    if (index==AnnotationSession::REFERENCE_MIP_INDEX) {
        return texture3DReference;
    } else if (index==AnnotationSession::BACKGROUND_MIP_INDEX) {
        return texture3DBackground;
    }
}

