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
            delete nrrd; nrrd=0;
            return false;
        }

        data1d = (unsigned char *)(nrrd->data);
        sz = new V3DLONG[4];
        sz[0] = ((nrrd->dim > 0) ? nrrd->axis[0].size : 1);
        sz[1] = ((nrrd->dim > 1) ? nrrd->axis[1].size : 1);
        sz[2] = ((nrrd->dim > 2) ? nrrd->axis[2].size : 1);
        sz[3] = ((nrrd->dim > 3) ? nrrd->axis[3].size : 1);
        datatype = dt;

        nrrdNix(nrrd); //free nrrd data structure w/o the actual data it points to. Added based on Greg's suggestion.

        return true;
    }
#endif

}

bool write_nrrd(char imgSrcFile[], unsigned char * data1d, V3DLONG sz[4], int datatype)
{
    float pixelsz[4];
    pixelsz[0] = pixelsz[1] = pixelsz[2] = pixelsz[3] = 1;
    float spaceorigin[3];
    spaceorigin[0] = spaceorigin[1] = spaceorigin[2] = 1;

    return write_nrrd_with_pxinfo(imgSrcFile, data1d, sz, datatype, pixelsz, spaceorigin);
}

bool write_nrrd_with_pxinfo(char imgSrcFile[], unsigned char * data1d, V3DLONG sz[4], int datatype,
                            float pixelsz[4], float spaceorigin[3])
{
    v3d_msg("To be implemented");
    //    Nrrd *nrrd = nrrdNew();
    //    if ( nrrdSave( nrrd, filename, NULL ) )
    //    {
    //        return galse;
    //    }
    return true;
}



