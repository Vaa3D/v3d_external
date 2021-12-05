#ifndef ANALYSISTOOLS_H
#define ANALYSISTOOLS_H

#include "../../v3d/v3d_core.h"


class BoundingBox3D
{
public:
    int x0;
    int x1;
    int y0;
    int y1;
    int z0;
    int z1;
};

class AnalysisTools
{
public:
    static const int CUBIFY_TYPE_AVERAGE;
    static const int CUBIFY_TYPE_MODE;

    AnalysisTools();
    static My4DImage* cubifyImageByChannel(My4DImage * sourceImage, int sourceChannel, int cubeSize, int type, V3DLONG* subregion,
                                           bool skipzeros, unsigned char** skipPositions);
    static My4DImage* cubifyImage(My4DImage * sourceImage, int cubeSize, int type);
    static My4DImage* getChannelSubImageFromMask(My4DImage * sourceImage, My4DImage* indexImage, int sourceChannel, int index, BoundingBox3D bb,
                                                                   bool normalize, double normalizationCutoff /* 0.0 - 1.0 */);
    static My4DImage* createMIPFromImage(My4DImage * image);
    static My4DImage* createMIPFromImageByLUT(My4DImage * image, v3d_uint8 * lut);
    static v3d_uint8  getReverse16ColorLUT(v3d_uint8 * lut, v3d_uint8 r, v3d_uint8 g, v3d_uint8 b);
    static v3d_uint8* create16Color8BitLUT_fiji();
    static v3d_uint8* create16Color8BitLUT();

};

#endif // ANALYSISTOOLS_H
