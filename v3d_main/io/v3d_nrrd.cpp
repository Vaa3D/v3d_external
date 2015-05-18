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
        
        // Figure out what kind of data we have
        // Set the number of image dimensions and bail if needed
        unsigned int domainAxisNum, domainAxisIdx[NRRD_DIM_MAX],
        rangeAxisNum, rangeAxisIdx[NRRD_DIM_MAX]={-1},
          spaceAxisNum, spaceAxisIdx[NRRD_DIM_MAX];
        
        // domain should be space-time axes
        domainAxisNum = nrrdDomainAxesGet(nrrd, domainAxisIdx);
        // colour only
        rangeAxisNum = nrrdRangeAxesGet(nrrd, rangeAxisIdx);
        // definitely space, but not always set, so may need to use domain axes
        spaceAxisNum = nrrdSpatialAxesGet(nrrd, spaceAxisIdx);
        
        int timeAxis=-1;
        
        if(rangeAxisNum>1) {
            v3d_msg("nrrds can only have 1 colour axis!");
            return false;
        }
        int colourAxis = rangeAxisNum>=0 ? rangeAxisIdx[0] : -1;
        
        if(domainAxisNum>4) {
            v3d_msg("nrrds can only have 4 domain (spatio-temporal) axes!");
            return false;
        } else if (domainAxisNum<2) {
            v3d_msg("nrrds must have at least 2 domain (spatio-temporal) axes!");
            return false;
        }

        if(spaceAxisNum==0) {
            // explicit space axes have not been set - we need to look more carefully to see which
            // of the domain axes are space (and not time)
            unsigned int ax;
            for (unsigned int i=0; i<domainAxisNum; i++) {
                ax=domainAxisIdx[i];
                switch ( nrrd->axis[ax].kind )
                {
                    case nrrdKindUnknown:
                    case nrrdKindDomain:
                    case nrrdKindSpace: spaceAxisIdx[spaceAxisNum++]=ax; break;
                    case nrrdKindTime: 
                        if(timeAxis>0) 
                        {
                            v3d_msg("nrrds cannot have multiple time axes!");
                            return false;
                        }
                        timeAxis=ax; break;
                    default: printf("WARNING: unknown domain axis type %d in nrrd", nrrd->axis[ax].kind);
                }
            }
        }
        
        if(spaceAxisNum == 4) {
            // assume the last space axis is a time axis
            if(timeAxis<0) {
                timeAxis=spaceAxisIdx[spaceAxisNum-1];
                printf("WARNING: Found 4 space-like axes in nrrd. Assuming that last space axis is a time axis");
            } else {
                if(rangeAxisNum>0) {
                    v3d_msg("Found 4 space axes in nrrd, one time and one colour axis. Should be impossible!");
                    return false;
                }
                printf("WARNING: ambiguous nrrd kind field. Assuming that last unmarked axis in nrrd is colour.");
                // FIXME should this be the smallest axis?!
                rangeAxisIdx[0]=spaceAxisIdx[spaceAxisNum-1];
                rangeAxisNum=1;
            }
            --spaceAxisNum;
        }
        if(spaceAxisNum==2) {
            // insert an axis immediately after last spatial axis
            unsigned int last_space_axis=spaceAxisIdx[spaceAxisNum-1];
            // nb nrrdAxesInsert take as input the position to insert the new axis
            nrrdAxesInsert(nrrd, nrrd, last_space_axis+1);
            // record that extra space axis id
            spaceAxisIdx[spaceAxisNum++]=last_space_axis+1;
            
            // recalculate time ...
            if( timeAxis>=0 && timeAxis > last_space_axis) {
                timeAxis++;
            }
            /// and colour axes
            if( colourAxis>=0 && colourAxis > last_space_axis) {
                colourAxis++;
            }
        } else if (spaceAxisNum !=3) {
            v3d_msg("nrrds must have 2 or 3 spatial axes!");
            return false;
        }
        
        // FIXME
        if(timeAxis>=0) {
            v3d_msg("No support yet for nrrds with time information!");
            return false;
        }
        
        // Fetch axis spacing
        double spacing[3] = { 1.0, 1.0, 1.0 };
        for ( unsigned int i = 0; i < spaceAxisNum; ++i )
        {
            unsigned int ax=spaceAxisIdx[i];
            switch ( nrrdSpacingCalculate( nrrd, ax, spacing+i, nrrd->axis[ax].spaceDirection ) )
            {
                case nrrdSpacingStatusScalarNoSpace:
                case nrrdSpacingStatusDirection:
                    break;
                case nrrdSpacingStatusScalarWithSpace:
                    printf("WARNING: nrrdSpacingCalculate returned nrrdSpacingStatusScalarWithSpace\n");
                    break;
                case nrrdSpacingStatusNone:
                default:
                    printf("WARNING: no pixel spacings in Nrrd for spatial axis %d (absolute axis id %d); setting to 1.0\n", i, ax);
                    spacing[i] = 1.0;
                    break;
            }
        }

        pixelsz[0]=(float) spacing[0];
        pixelsz[1]=(float) spacing[1];
        pixelsz[2]=(float) spacing[2];
        
        spaceorigin[0]=isnan(nrrd->spaceOrigin[0])?0.0f:(float) nrrd->spaceOrigin[0];
        spaceorigin[1]=isnan(nrrd->spaceOrigin[1])?0.0f:(float) nrrd->spaceOrigin[1];
        spaceorigin[2]=isnan(nrrd->spaceOrigin[2])?0.0f:(float) nrrd->spaceOrigin[2];
        
        if ( colourAxis >= 0 && colourAxis != (nrrd->dim-1) ) {
            // colour is not last axis - permute to bump it to last
            Nrrd *ntmp = nrrdNew();
            // make a map in which the ith entry contains the *old* position of the axis
            unsigned int axmap[NRRD_DIM_MAX];
            // colour channel (formerly first) must go into last channel
            // FIXME need to handle time/5d data in due course
            axmap[nrrd->dim-1] = colourAxis;
            for (unsigned int axi=0; axi<(nrrd->dim-1); axi++)
            {
                axmap[axi] = axi + (axi >= colourAxis);
            }
            if (nrrdCopy(ntmp, nrrd)
                || nrrdAxesPermute(nrrd, ntmp, axmap))
            {
                throw(biffGetDone(NRRD));
            }
            nrrdNuke(ntmp);
        }

        // store image dimensions 
        // FIXME - still no support for time
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



