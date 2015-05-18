#include <teem/nrrd.h>

#include "../basic_c_fun/basic_4dimage.h"
#include "../basic_c_fun/basic_surf_objs.h"

#include "v3d_nrrd.h"

//isnan and isfinite is a part of the C and C++ standards, support for these has been removed
//from VC++ in the latest update of Visual Studio. (VS2012 Update 2)
#include <math.h>  //for isnan() 
#ifndef isnan 
#define isnan(x) ((x)!=(x)) 
#endif


bool read_nrrd(const char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype)
{
    float pixelsz[4];
    float spaceorigin[3];
    return read_nrrd_with_pxinfo(imgSrcFile, data1d, sz, datatype, pixelsz, spaceorigin);
}

bool read_nrrd_with_pxinfo(const char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype,
                           float pixelsz[4], float spaceorigin[3])
{
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
            v3d_msg("nrrd error: unsupported data type");
            dt = 0;
            break;
        }

        if (dt==0)
        {
            nrrdNuke(nrrd);
            return false;
        }

        data1d = (unsigned char *)(nrrd->data);
        datatype = dt;
        
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
        
        spaceorigin[0]=isnan(nrrd->spaceOrigin[0])?0.0f:(float) nrrd->spaceOrigin[0];
        spaceorigin[1]=isnan(nrrd->spaceOrigin[1])?0.0f:(float) nrrd->spaceOrigin[1];
        spaceorigin[2]=isnan(nrrd->spaceOrigin[2])?0.0f:(float) nrrd->spaceOrigin[2];
        
        printf("nonSpatialDimension: %d", nonSpatialDimension);
        // Record size of non-spatial dimension (ie vector, colour etc)
        int nDataVar = nonSpatialDimension>=0 ? 1 : nrrd->axis[nonSpatialDimension].size;
        
        if ( nonSpatialDimension >= 0 && nonSpatialDimension != (nrrd->dim-1) ) {
            // colour is not last axis - permute to bump it to last
            Nrrd *ntmp = nrrdNew();
            // make a map in which the ith entry contains the *old* position of the axis
            unsigned int axmap[NRRD_DIM_MAX];
            // colour channel (formerly first) must go into last channel
            // FIXME need to handle time/5d data in due course
            axmap[nrrd->dim-1] = nonSpatialDimension;
            for (unsigned int axi=0; axi<(nrrd->dim-1); axi++)
            {
                axmap[axi] = axi + (axi >= nonSpatialDimension);   
            }
            if (nrrdCopy(ntmp, nrrd)
                || nrrdAxesPermute(nrrd, ntmp, axmap))
            {
                throw(biffGetDone(NRRD));
            }
            nrrdNuke(ntmp);
        }

        // store image dimensions 
        // FIXME - what about when incoming nrrd was e.g. xyc
        // look at nrrdAxesInsert?
        sz = new V3DLONG[4];
        sz[0] = ((nrrd->dim > 0) ? nrrd->axis[0].size : 1);
        sz[1] = ((nrrd->dim > 1) ? nrrd->axis[1].size : 1);
        sz[2] = ((nrrd->dim > 2) ? nrrd->axis[2].size : 1);
        sz[3] = ((nrrd->dim > 3) ? nrrd->axis[3].size : 1);

        nrrdNix(nrrd); //free nrrd data structure w/o the actual data it points to. Added based on Greg's suggestion.

        return true;
    }

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
        // nb no way to check for 5d data at present
        size_t nsize[NRRD_DIM_MAX] = {sz[0], sz[1], sz[2], sz[3]};
        const unsigned int ndim = sz[3]>1 ? 4 : 3;

        // wrap incoming data into nrrd struct
        if ( nrrdWrap_nva( nrrd, data, nrrdType, ndim, nsize))
        {
            throw( biffGetDone(NRRD) );
        }

        // For now always set space dimensions to 3
        nrrdSpaceDimensionSet( nrrd, 3 );
        double origin[NRRD_DIM_MAX] = { spaceorigin[0], spaceorigin[1], spaceorigin[2]};
        if ( nrrdSpaceOriginSet( nrrd, origin ) )
        {
            throw( biffGetDone(NRRD) );
        }
        
        // Vaa3d Image4DSimple assumes xyzct ordering
        int kind[NRRD_DIM_MAX] = { nrrdKindSpace, nrrdKindSpace, nrrdKindSpace, nrrdKindList, nrrdKindTime};
        nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoKind, kind);
        nrrdAxisInfoSet_va(nrrd, nrrdAxisInfoLabel, "x", "y", "z", "c", "t");
        
        // Set up space directions to record voxel spacings.
        double spaceDir[NRRD_DIM_MAX][NRRD_SPACE_DIM_MAX];
        for ( int i = 0; i < ndim; ++i )
        {
            for ( int j = 0; j < 4; ++j )
            {
                if (i>2 || j>2) spaceDir[i][j] = NAN;
                else if (i == j) spaceDir[i][j] = (double) pixelsz[i];
                else spaceDir[i][j] = 0.0; // Can't assume that memory is zeroed
            }
        }
        nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoSpaceDirection, spaceDir);

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
    
    nrrdIoStateNix( nios );
    // Free nrrd struct but not data that it points to
    nrrdNix(nrrd);
    return true;
}



