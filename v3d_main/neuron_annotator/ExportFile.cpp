#include "ExportFile.h"

ExportFile::ExportFile()
{
    p4Dimg = NULL;
    p3Dtex = NULL;
    
    stopped = true;
}

ExportFile::~ExportFile()
{
}

bool ExportFile::init(My4DImage *p4DimgInput, RGBA8 *p3DtexInput, QString filenameInput)
{
    stopped = false;
    
    p4Dimg = p4DimgInput;
    p3Dtex = p3DtexInput;
    
    filename = filenameInput;
    
    //check
    if(!p4Dimg)
    {
        stopped = true;
        v3d_msg("No image stack is specified.");
        return false;
    }
    
    if(!p3Dtex)
    {
        stopped = true;
        v3d_msg("No texture is specified.");
        return false;
    }
    
    if(filename.isEmpty())
    {
        stopped = true;
        v3d_msg("No file name is specified.");
        return false;
    }
    
    if(QFileInfo(filename).suffix().toUpper() != "TIF")
    {
        filename.append(".tif"); // force to save as .tif file
    }

    return true;
}

void ExportFile::run()
{
    mutex.lock();

    if(stopped)
    {
        stopped = false;
        mutex.unlock();
    }
    else
    {
        while(!stopped)
        {
            // save tif file
            V3DLONG sx = p4Dimg->getXDim();
            V3DLONG sy = p4Dimg->getYDim();
            V3DLONG sz = p4Dimg->getZDim();
            V3DLONG sc = 3;

            V3DLONG pagesz = sx*sy*sz;
            V3DLONG totalplx = pagesz*sc;

            V3DLONG pagesz2 = 2*pagesz;
            V3DLONG pagesz3 = 3*pagesz;

            if(sz>256) sz = 256; // compressed size realZ, realY, realX

            unsigned char * data1d = NULL;
            try {
                data1d = new unsigned char [totalplx];

                for(V3DLONG k=0; k<sz; k++)
                {
                    V3DLONG offset_k = k*sx*sy;
                    for(V3DLONG j=0; j<sy; j++)
                    {
                        V3DLONG offset_j = offset_k + j*sx;
                        for(V3DLONG i=0; i<sx; i++)
                        {
                            V3DLONG idx = offset_j + i;

                            data1d[idx] = p3Dtex[idx].r;
                            data1d[idx + pagesz] = p3Dtex[idx].g;
                            data1d[idx + pagesz2] = p3Dtex[idx].b;
                            //data1d[idx + pagesz3] = p3Dtex[idx].a;
                        }
                    }
                }
            } catch (...) {
                cout<<"Fail to allocate memory for Export tif file!"<<endl;
                stopped = true;
                mutex.unlock();
                return;
            }

            // saving
            V3DLONG savesz[4];
            savesz[0] = sx; savesz[1] = sy; savesz[2] = sz; savesz[3] = sc;

            if (saveImage(filename.toStdString().c_str(), (const unsigned char *)data1d, savesz, 1)!=true){
                cout<<"Fail to save file!"<<endl;
                stopped = true;
                mutex.unlock();
                return;
            }

            // done
            stopped = true;
        }

        mutex.unlock();
    }

    //
    return;
}
