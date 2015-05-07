#include <teem/nrrd.h>

#include "../common_lib/include/teem/nrrd.h"
#include "../basic_c_fun/basic_4dimage.h"
#include "../basic_c_fun/basic_surf_objs.h"

bool read_nrrd(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype)
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
        v3d_msg("works nrrd");
        return true;
    }
    else
    {
        v3d_msg(QString("nrrd [%1] reading direct fail").arg(imgSrcFile));
        return false;
    }
}

bool read_nrrd(char *filename, Image4DSimple * &img)
{
    if (img)
    {
        delete img;
        img=0;
    }

    img = new Image4DSimple();

    Nrrd *nrrd = nrrdNew();
    if ( nrrdLoad( nrrd, filename, NULL ) )
    {
        if ( nrrd->dim > 4 )
        {
            v3d_msg("ERROR: for now, nrrd input can only handle data with dimension 4 or less.");
            return false;
        } else if ( nrrd->spaceDim > 3 ){
            v3d_msg("ERROR: for now, nrrd input can only handle data with space dimension 3 or less.");
            return false;
        }

        ImagePixelType p = V3D_UNKNOWN;
        switch ( nrrd->type )
        {
        case nrrdTypeUChar:  p = V3D_UINT8;  break;
        case nrrdTypeChar:   p = V3D_UINT8;  break;
        case nrrdTypeUShort: p = V3D_UINT16;  break;
        case nrrdTypeShort:  p = V3D_UINT16;  break;
        case nrrdTypeInt:    p = V3D_FLOAT32;  break;
        case nrrdTypeFloat:  p = V3D_FLOAT32;  break;
        default:
            v3d_msg("Error: unsupported type");
            img->setDatatype(V3D_UNKNOWN);
            break;
        }

        if (p==V3D_UNKNOWN)
        {
            delete nrrd; nrrd=0;
            return false;
        }

        img->setData((unsigned char *)(nrrd->data),
                     ((nrrd->dim > 0) ? nrrd->axis[0].size : 1),
                     ((nrrd->dim > 1) ? nrrd->axis[1].size : 1),
                     ((nrrd->dim > 2) ? nrrd->axis[2].size : 1),
                     ((nrrd->dim > 3) ? nrrd->axis[3].size : 1),
                     p);
        return true;
    }
    return false;

}

bool write_nrrd(char *filename,
                Image4DSimple *img)
{
    //    Nrrd *nrrd = nrrdNew();
    //    if ( nrrdSave( nrrd, filename, NULL ) )
    //    {
    //        return galse;
    //    }
    return true;
}


