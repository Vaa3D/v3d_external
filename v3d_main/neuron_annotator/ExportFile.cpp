#include "ExportFile.h"

template <class Tinput, class Tmask, class Tref, class Toutput>
Toutput* getCurrentStack(Tinput *input1d, Tmask *mask1d, Tref *ref1d, V3DLONG *szStack, QList<bool> maskStatusList, QList<bool> overlayStatusList, int datatype){
    if(!input1d || !mask1d || !ref1d){
        cout<<"Input image stack is NULL!"<<endl;
        return NULL;
    }

    //
    V3DLONG sx = szStack[0];
    V3DLONG sy = szStack[1];
    V3DLONG sz = szStack[2];
    V3DLONG sc = szStack[3]; // stack_sc + ref (0/1)

    V3DLONG pagesz = sx*sy*sz;
    V3DLONG totalplx = pagesz*sc;

    //
    Toutput *output1d = NULL;
    try{
        output1d = new Toutput [totalplx];

        //
        V3DLONG numcolor = 0;
        if(overlayStatusList.at(0)) // ref
        {
            numcolor = sc-1;

            V3DLONG offset_c = numcolor*pagesz;
            for(V3DLONG k=0; k<sz; k++)
            {
                V3DLONG offset_k = offset_c + k*sx*sy;
                for(V3DLONG j=0; j<sy; j++)
                {
                    V3DLONG offset_j = offset_k + j*sx;
                    for(V3DLONG i=0; i<sx; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        V3DLONG idxref = idx - offset_c;
                        output1d[idx] = (Toutput) (ref1d[idxref]);
                    }
                }
            }
        }
        else{
            numcolor = sc;
        }

        //
        for(V3DLONG c=0; c<numcolor; c++)
        {
            V3DLONG offset_c = c*pagesz;
            for(V3DLONG k=0; k<sz; k++)
            {
                V3DLONG offset_k = offset_c + k*sx*sy;
                for(V3DLONG j=0; j<sy; j++)
                {
                    V3DLONG offset_j = offset_k + j*sx;
                    for(V3DLONG i=0; i<sx; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        V3DLONG idxmask = idx - offset_c;

                        if(mask1d[idxmask]) // neuron
                        {
                            if(maskStatusList.at(mask1d[idxmask]-1))
                            {
                                output1d[idx] = (Toutput) (input1d[idx]);
                            }
                            else
                            {
                                output1d[idx] = 0;
                            }
                        }
                        else // background
                        {
                            if(overlayStatusList.at(1))
                            {
                                output1d[idx] = (Toutput) (input1d[idx]);
                            }
                            else
                            {
                                output1d[idx] = 0;
                            }
                        }

                    }
                }
            }
        }

    }
    catch(...){
        cout<<"fail to allocate memory!"<<endl;
        return NULL;
    }

    return output1d;
}

// export file class
ExportFile::ExportFile()
{
    pOriginal = NULL;
    pMask = NULL;
    pRef = NULL;
    
    stopped = true;
}

ExportFile::~ExportFile()
{
}

bool ExportFile::init(My4DImage *pOriginalInput, My4DImage *pMaskInput, My4DImage *pRefInput, QList<bool> maskStatusListInput, QList<bool> overlayStatusListInput, QString filenameInput)
{
    stopped = false;
    
    pOriginal = pOriginalInput;
    pMask = pMaskInput;
    pRef = pRefInput;

    maskStatusList = maskStatusListInput;
    overlayStatusList = overlayStatusListInput;

    filename = filenameInput;
    
    //check
    if(!pOriginal)
    {
        stopped = true;
        v3d_msg("No image stack is specified.");
        return false;
    }
    
    if(!pMask)
    {
        stopped = true;
        v3d_msg("No mask stack is specified.");
        return false;
    }

    if(!pRef)
    {
        stopped = true;
        v3d_msg("No reference stack is specified.");
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
            V3DLONG sx = pOriginal->getXDim();
            V3DLONG sy = pOriginal->getYDim();
            V3DLONG sz = pOriginal->getZDim();
            V3DLONG sc = pOriginal->getCDim();

            int datatype;

            if( pOriginal->getDatatype() > pRef->getDatatype()){
                datatype = pOriginal->getDatatype();
            }
            else{
                datatype = pRef->getDatatype();
            }

            V3DLONG szStack[4];

            szStack[0] = sx; szStack[1] = sy; szStack[2] = sz; szStack[3] = sc;

            if(overlayStatusList.at(0)) szStack[3] ++;

            //
            void * data1d = NULL;
            try {

                if(pOriginal->getDatatype()==V3D_UINT8 && pMask->getDatatype()==V3D_UINT8 && pRef->getDatatype()==V3D_UINT8)
                {
                    data1d = (void *) getCurrentStack<unsigned char, unsigned char, unsigned char, unsigned char>((unsigned char *)(pOriginal->getRawData()),
                                                                                                          (unsigned char *)(pMask->getRawData()),
                                                                                                          (unsigned char *)(pRef->getRawData()),
                                                                                                          szStack, maskStatusList, overlayStatusList, datatype);
                }
                else if(pOriginal->getDatatype()==V3D_UINT8 && pMask->getDatatype()==V3D_UINT8 && pRef->getDatatype()==V3D_UINT16)
                {
                    data1d = (void *) getCurrentStack<unsigned char, unsigned char, unsigned short, unsigned short>((unsigned char *)(pOriginal->getRawData()),
                                                                                                          (unsigned char *)(pMask->getRawData()),
                                                                                                          (unsigned short *)(pRef->getRawData()),
                                                                                                          szStack, maskStatusList, overlayStatusList, datatype);
                }
                else if(pOriginal->getDatatype()==V3D_UINT16 && pMask->getDatatype()==V3D_UINT8 && pRef->getDatatype()==V3D_UINT16)
                {
                    data1d = (void *) getCurrentStack<unsigned short, unsigned char, unsigned short, unsigned short>((unsigned short *)(pOriginal->getRawData()),
                                                                                                          (unsigned char *)(pMask->getRawData()),
                                                                                                          (unsigned short *)(pRef->getRawData()),
                                                                                                          szStack, maskStatusList, overlayStatusList, datatype);
                }
                else
                {
                    cout<<"Your datatype is currently not supported!"<<endl;
                    stopped = true;
                    mutex.unlock();
                    return;
                }

                if(!data1d) {
                    cout<<"Fail to generate current image stack!"<<endl;
                    stopped = true;
                    mutex.unlock();
                    return;
                }

            } catch (...) {
                cout<<"Fail to export image stack .tif file!"<<endl;
                stopped = true;
                mutex.unlock();
                return;
            }

            // save .tif image stack
            if (saveImage(filename.toStdString().c_str(), (const unsigned char *)data1d, szStack, datatype)!=true){
                cout<<"Fail to save file!"<<data1d<<datatype<<endl;
                stopped = true;
                mutex.unlock();
                return;
            }

            // de-alloc
            if(data1d) {delete []data1d; data1d=NULL;}

            // done
            stopped = true;
        }

        mutex.unlock();
    }

    //
    return;
}
