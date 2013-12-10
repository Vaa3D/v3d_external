#ifndef STEREO3DMODE_H
#define STEREO3DMODE_H

#include <stdint.h>

namespace jfrc
{

enum Stereo3DMode {
    STEREO_OFF,
    STEREO_LEFT_EYE,
    STEREO_RIGHT_EYE,
    STEREO_QUAD_BUFFERED,
    STEREO_ANAGLYPH_RED_CYAN,
    STEREO_ANAGLYPH_GREEN_MAGENTA,
    STEREO_ROW_INTERLEAVED,
    STEREO_COLUMN_INTERLEAVED,
    STEREO_CHECKER_INTERLEAVED
};

// StereoEyeView sets either left or right eye view, depending on constructor argument.
class StereoEyeView
{
public:
    enum Eye {LEFT, RIGHT};

    StereoEyeView(Eye eyeActual, Eye eyeGeom);
    virtual ~StereoEyeView();

protected:
    Stereo3DMode mode;
};

class AnaglyphRedCyanEyeView : public StereoEyeView
{
public:
    AnaglyphRedCyanEyeView(Eye eye, Eye eyeGeom);
    ~AnaglyphRedCyanEyeView();
};

class AnaglyphGreenMagentaEyeView : public StereoEyeView
{
public:
    AnaglyphGreenMagentaEyeView(Eye eye, Eye eyeGeom);
    ~AnaglyphGreenMagentaEyeView();
};

class QuadBufferView : public StereoEyeView
{
public:
    QuadBufferView(Eye eye, Eye eyeGeom);
    ~QuadBufferView();
};

class RowInterleavedStereoView : public StereoEyeView
{
public:
    static const uint8_t* rowStipple0;
    static const uint8_t* rowStipple1;
    static const uint8_t* colStipple0;
    static const uint8_t* colStipple1;
    static const uint8_t* checkStipple0;
    static const uint8_t* checkStipple1;

    // Remember to call fillStencil() once before rendering
    RowInterleavedStereoView(Eye eye, Eye eyeGeom, const uint8_t* stipple=rowStipple0);
    ~RowInterleavedStereoView();
    void fillStencil(int left, int top, int width, int height);

    const uint8_t* stipple;
};

class ColumnInterleavedStereoView : public RowInterleavedStereoView
{
public:

    ColumnInterleavedStereoView(Eye eye, Eye eyeGeom, const uint8_t* stipple=checkStipple0);
};

class CheckerInterleavedStereoView : public RowInterleavedStereoView
{
public:

    CheckerInterleavedStereoView(Eye eye, Eye eyeGeom, const uint8_t* stipple=checkStipple0);
};


} // namespace jfrc

#endif // STEREO3DMODE_H
