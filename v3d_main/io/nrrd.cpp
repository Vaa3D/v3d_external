#include <teem/nrrd.h>

#include "../basic_c_fun/basic_4dimage.h"
#include "../basic_c_fun/basic_surf_objs.h"

#include "nrrd.h"

bool read_nrrd(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype)
{
    float pixelsz[4];
    float spaceorigin[3];
    return read_nrrd_with_pxinfo(imgSrcFile, data1d, sz, datatype, pixelsz, spaceorigin);
}

bool read_nrrd_with_pxinfo(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype,
                           float pixelsz[4], float spaceorigin[3])
{

#if defined(Q_OS_WIN)
    v3d_msg("Direct NRRD file reading currently is not supported on Windows. You can use the Bioformats IO plugin to load instead.");
    return false;

#else

    if (data1d)
    {
        delete []data1d;
        data1d=0;
    }
    if (sz)
    {
        delete []sz;
        sz=0;
    }

    Nrrd *nrrd = nrrdNew();
    if ( nrrdLoad( nrrd, imgSrcFile, NULL ) )
    {
        v3d_msg(QString("nrrd [%1] reading direct fail").arg(imgSrcFile));
        return false;
    }
    else
    {
        if ( nrrd->dim > 4 )
        {
            v3d_msg("ERROR: for now, nrrd input can only handle data with dimension 4 or less.");
            return false;
        } else if ( nrrd->spaceDim > 3 ){
            v3d_msg("ERROR: for now, nrrd input can only handle data with space dimension 3 or less.");
            return false;
        }

        int dt = 0;
        switch ( nrrd->type )
        {
        case nrrdTypeUChar:  dt = 1;  break;
        case nrrdTypeChar:   dt = 1;  break;
        case nrrdTypeUShort: dt = 2;  break;
        case nrrdTypeShort:  dt = 2;  break;
        case nrrdTypeInt:    dt = 4;  break;
        case nrrdTypeFloat:  dt = 4;  break;
        default:
            v3d_msg("Error: unsupported type");
            dt = 0;
            break;
        }

        if (dt==0)
        {
            nrrdNuke(nrrd);
            return false;
        }

        data1d = (unsigned char *)(nrrd->data);
        sz = new V3DLONG[4];
        sz[0] = ((nrrd->dim > 0) ? nrrd->axis[0].size : 1);
        sz[1] = ((nrrd->dim > 1) ? nrrd->axis[1].size : 1);
        sz[2] = ((nrrd->dim > 2) ? nrrd->axis[2].size : 1);
        sz[3] = ((nrrd->dim > 3) ? nrrd->axis[3].size : 1);
        datatype = dt;
        
        spaceorigin[0]=isnan(nrrd->spaceOrigin[0])?0.0f:(float) nrrd->spaceOrigin[0];
        spaceorigin[1]=isnan(nrrd->spaceOrigin[1])?0.0f:(float) nrrd->spaceOrigin[1];
        spaceorigin[2]=isnan(nrrd->spaceOrigin[2])?0.0f:(float) nrrd->spaceOrigin[2];

        // Fetch axis spacing
        double spacing[3] = { 1.0, 1.0, 1.0 };
        int firstSpaceAxis = -1;
        int numSpaceAxesSoFar = 0;
        int nonSpatialDimension = -1;
        for ( unsigned int ax = 0; ax < nrrd->dim; ++ax )
        {
            switch ( nrrd->axis[ax].kind )
            {
                case nrrdKindUnknown:
                case nrrdKindDomain:
                case nrrdKindSpace:
                case nrrdKindTime: firstSpaceAxis=firstSpaceAxis<0?ax:firstSpaceAxis; break;
                default: nonSpatialDimension = ax; continue;
            }
            switch ( nrrdSpacingCalculate( nrrd, ax, spacing + numSpaceAxesSoFar, nrrd->axis[ax].spaceDirection ) )
            {
                case nrrdSpacingStatusScalarNoSpace:
                    break;
                case nrrdSpacingStatusDirection:
                    break;
                case nrrdSpacingStatusScalarWithSpace:
                    printf("WARNING: nrrdSpacingCalculate returned nrrdSpacingStatusScalarWithSpace\n");
                    spacing[numSpaceAxesSoFar] = (float) nrrd->axis[ax].spacing;
                    break;
                case nrrdSpacingStatusNone:
                default:
                    printf("WARNING: no pixel spacings in Nrrd for axis %d ; setting to 1.0\n",ax);
                    spacing[numSpaceAxesSoFar] = 1.0f;
                    break;
            }
            numSpaceAxesSoFar++;
        }
        if ( firstSpaceAxis < 0 || firstSpaceAxis >1 )
        {
            v3d_msg(QString("ERROR: Unable to identify first spatial axis in nrrd. Got [%1]").arg(firstSpaceAxis));
            return false;
        }
        pixelsz[0]=(float) spacing[0];
        pixelsz[1]=(float) spacing[1];
        pixelsz[2]=(float) spacing[2];
        
        // Figure out size of non-spatial dimension (ie vector, colour etc)
        int nDataVar = 1;
        if ( nonSpatialDimension > nrrd->spaceDim) nDataVar = sz[nonSpatialDimension];
        else if ( nrrd->spaceDim >= 0) {
            // At the moment we can't handle having the non-spatial dimension dimension coming first
            // that would require permuting the nrrd data block to prepare it for Vaa3d
            // See http://teem.sourceforge.net/nrrd/lib.html#overview nrrdAxesPermute
            v3d_msg(QString("ERROR: Only nrrds with vector values in the final dimension (not [%1]) are currently supported").arg(nonSpatialDimension));
            
            return false;
        }
        
        nrrdNix(nrrd); //free nrrd data structure w/o the actual data it points to. Added based on Greg's suggestion.

        return true;
    }
#endif

}

bool write_nrrd(const char imgSrcFile[], unsigned char * data1d, V3DLONG sz[4], int datatype)
{
    float pixelsz[4];
    pixelsz[0] = pixelsz[1] = pixelsz[2] = pixelsz[3] = 1;
    float spaceorigin[3];
    spaceorigin[0] = spaceorigin[1] = spaceorigin[2] = 0;

    return write_nrrd_with_pxinfo(imgSrcFile, data1d, sz, datatype, pixelsz, spaceorigin);
}

bool write_nrrd_with_pxinfo(const char imgSrcFile[], unsigned char * data1d, V3DLONG sz[4], int datatype,
                            float pixelsz[4], float spaceorigin[3])
{
    int nrrdType = nrrdTypeUnknown;
    switch ( datatype )
    {
        case V3D_UINT8: nrrdType = nrrdTypeUChar; break;
        case V3D_UINT16: nrrdType = nrrdTypeUShort; break;
        case V3D_FLOAT32: nrrdType = nrrdTypeFloat; break;
        default: 
            v3d_msg("Error: unsupported type");
            return false;
            break;
    }
    
    void* data = (void*) data1d;
    
    Nrrd *nrrd = nrrdNew();
    // write gzipped if available, raw otherwise
    NrrdIoState *nios = nrrdIoStateNew();
    if (nrrdEncodingGzip->available() )
    {
        nrrdIoStateEncodingSet( nios, nrrdEncodingGzip );
        nrrdIoStateSet( nios, nrrdIoStateZlibLevel, 9 );
    }

    try
    {
        if ( sz[3]>1 ) {
            throw("nrrd_write is presently unable to save 4D or 5D images");
        }

        if ( nrrdWrap_va( nrrd, data, (int) nrrdType, (unsigned int) 3,
                         (size_t) sz[0],
                         (size_t) sz[1],
                         (size_t) sz[2]))
        {
            throw( biffGetDone(NRRD) );
        }
        
        nrrdSpaceDimensionSet( nrrd, 3 );
        double origin[NRRD_DIM_MAX] = { spaceorigin[0], spaceorigin[1], spaceorigin[2]};
        if ( nrrdSpaceOriginSet( nrrd, origin ) )
        {
            throw( biffGetDone(NRRD) );
        }
        
        nrrdAxisInfoSet_va( nrrd, nrrdAxisInfoLabel, "x", "y", "z" );

        if ( nrrdSave( imgSrcFile, nrrd, nios ) )
        {
            throw( biffGetDone(NRRD) );
        }

    }
    catch ( char* err )
    {
        v3d_msg(QString("ERROR: write_nrrd returned error '[%1]'\n").arg(err));
        free( err );
        return false;
    }
    
    // Free nrrd struct but not data that it points to
    nrrdNix(nrrd);
    return true;
}



