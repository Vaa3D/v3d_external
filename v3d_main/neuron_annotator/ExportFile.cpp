#include "ExportFile.h"
#include "DataFlowModel.h"
#include "geometry/CameraModel.h"
#include "analysis/AnalysisTools.h"
#include <QMutexLocker>

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

        // Do initial zero-fill
        for (int i=0;i<totalplx;i++) {
            output1d[i]=0;
        }

        //
        V3DLONG numcolor = sc;
        if(overlayStatusList.at(0)) // reference
        {
            //numcolor = sc-1; // add to fouth color channel
            //V3DLONG offset_c = numcolor*pagesz;

            V3DLONG offset_c = 2*pagesz;

            for(V3DLONG k=0; k<sz; k++)
            {
                //V3DLONG offset_k = offset_c + k*sx*sy;
                V3DLONG offset_k = k*sx*sy;
                for(V3DLONG j=0; j<sy; j++)
                {
                    V3DLONG offset_j = offset_k + j*sx;
                    for(V3DLONG i=0; i<sx; i++)
                    {
                        V3DLONG idx = offset_j + i;

                        //V3DLONG idxref = idx - offset_c;

                        output1d[idx] = (Toutput) (ref1d[idx]); // r
                        output1d[idx + pagesz] = (Toutput) (ref1d[idx]); // g
                        output1d[idx + offset_c] = (Toutput) (ref1d[idx]); // b
                    }
                }
            }
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
                                //output1d[idx] = 0;
                            }
                        }
                        else // background
                        {
                            if(overlayStatusList.at(1))
                            {
//                                int total=((Toutput)input1d[idx])+output1d[idx];
//                                if (total<0) {
//                                    total=0;
//                                } else if (total>255) { // only works for 8-bit data
//                                    total=255;
//                                }
//                                output1d[idx] = total; // may have problem convert int to unsigned char

                                output1d[idx] += ((Toutput)input1d[idx]); // output type is always higher than input
                            }
                            else
                            {
                                //output1d[idx] = 0;
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
ExportFile::ExportFile(
		QString fileName,
		const NaVolumeData& volumeData,
		const NeuronSelectionModel& selectionModel,
        const DataColorModel& colorModel,
        const CameraModel& cameraModel,
        bool is2D)
	: filename(fileName)
	, volumeData(volumeData)
	, selectionModel(selectionModel)
    , colorModel(colorModel)
    , cameraModel(cameraModel)
    , is2D(is2D)
    , stopped(true)
{}

ExportFile::~ExportFile()
{
}

template<class TSRC, class TDEST, class TLABEL>
void copyChannelType(My4DImage& dest, int destChannel,
		const Image4DProxy<My4DImage>& src, int srcChannel,
		const Image4DProxy<My4DImage>& label,
		const QList<bool>& maskStatusList,
		bool bShowBackground,
		bool bDoMask)
{
	const V3DLONG sx = src.sx;
	const V3DLONG sy = src.sy;
	const V3DLONG sz = src.sz;
	const V3DLONG su = dest.getUnitBytes();

	// copy channel data
	// pixel by pixel
	size_t line_size = sx;
	size_t slice_size = sx*sy;
	size_t channel_size = sx*sy*sz;
	TSRC* source_channel = ((TSRC*)src.data_p) + srcChannel*channel_size;
	TDEST* dest_channel = ((TDEST*)dest.getRawData()) + destChannel*channel_size;
	for (int z = 0; z < sz; ++z)
	{
		TSRC* source_slice = source_channel + z*slice_size;
		TDEST* dest_slice = dest_channel + z*slice_size;
		TLABEL* label_slice = ((TLABEL*)label.data_p) + z*slice_size;
		for (int y = 0; y < sy; ++y)
		{
			TSRC* source_line = source_slice + y*line_size;
			TDEST* dest_line = dest_slice + y*line_size;
			TLABEL* label_line = label_slice + y*line_size;
			for (int x = 0; x < sx; ++x)
			{
				if (bDoMask) {
				    TLABEL neuronIndex = label_line[x];
				    if (0 == neuronIndex) {
				        if (bShowBackground)
				            dest_line[x] = source_line[x];
				        else
				        	    dest_line[x] = 0;
				    }
				    else {
				    	    if (maskStatusList[neuronIndex-1])
				    	        dest_line[x] = source_line[x];
				    	    else
				    	    	    dest_line[x] = 0;
				    }
				}
				else
				    dest_line[x] = source_line[x];
			}
		}
	}
}

void copyChannel(My4DImage& dest, int destChannel,
		const Image4DProxy<My4DImage>& src, int srcChannel,
		const Image4DProxy<My4DImage>& label,
		const QList<bool>& maskStatusList,
		bool bShowBackground,
		bool bDoMask)
{
	typedef unsigned char T1;
	typedef unsigned short T2;
	int s = src.su;
	int d = dest.getUnitBytes();
	int l = label.su;
	if (s==1 && d == 1 && l == 1)
		copyChannelType<T1,T1,T1>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
	if (s==1 && d == 1 && l == 2)
		copyChannelType<T1,T1,T2>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
	if (s==1 && d == 2 && l == 1)
		copyChannelType<T1,T2,T1>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
	if (s==1 && d == 2 && l == 2)
		copyChannelType<T1,T2,T2>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
	if (s==2 && d == 1 && l == 1)
		copyChannelType<T2,T1,T1>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
	if (s==2 && d == 1 && l == 2)
		copyChannelType<T2,T1,T2>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
	if (s==2 && d == 2 && l == 1)
		copyChannelType<T2,T2,T1>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
	if (s==2 && d == 2 && l == 2)
		copyChannelType<T2,T2,T2>(dest, destChannel, src, srcChannel,
				label, maskStatusList, bShowBackground, bDoMask);
}

void ExportFile::run()
{

    qDebug() << "ExportFile::run()";

    if(stopped)
    {
        stopped = false;
        // return;
    }

    QString message;
    bool bSucceeded = true;

    while (!stopped) // so I can use "break"
    { // acquire read locks in this block
        QMutexLocker locker(&mutex);

		// fetch volume dimensions
		NaVolumeData::Reader volumeReader(volumeData);
		if (! volumeReader.hasReadLock()) {
			message = "Could not access Volume data";
			stopped = true;
			bSucceeded = false;
			break;
		}
		const Image4DProxy<My4DImage>& originalStack =
				volumeReader.getOriginalImageProxy();
		const Image4DProxy<My4DImage>& referenceStack =
				volumeReader.getReferenceImageProxy();
		const Image4DProxy<My4DImage>& labelStack =
				volumeReader.getNeuronMaskProxy();
		V3DLONG sx = originalStack.sx;
		V3DLONG sy = originalStack.sy;
		V3DLONG sz = originalStack.sz;
		// done fetching volume dimensions

		// count color channels
		DataColorModel::Reader colorReader(colorModel);
		if (colorModel.readerIsStale(colorReader))
		{
			message = "Could not access color reader";
			stopped = true;
			bSucceeded = false;
			break;
		}
		// Number of color channels depends on input size and toggle state
		V3DLONG sc = 0;
		for (int c = 0; c < originalStack.sc; ++c) {
			// Only export visible color channels
			if (colorReader.getChannelVisibility(c))
				++sc;
		}
		bool bIncludeSignal = (sc > 0);
		NeuronSelectionModel::Reader selectionReader(selectionModel);
		if (! selectionReader.hasReadLock()) {
			message = "Could not access Neuron masks";
			stopped = true;
			bSucceeded = false;
			break;
		}
		const QList<bool>& maskStatusList = selectionReader.getMaskStatusList();
		const QList<bool>& overlayStatusList = selectionReader.getOverlayStatusList();
		// Plus perhaps reference channel
		bool bIncludeReference = selectionReader.overlayIsChecked(DataFlowModel::REFERENCE_MIP_INDEX);
		if (bIncludeReference)
			++sc;
		// done counting color channels

		// choose pixel format, e.g. 8bit vs. 16bit
		ImagePixelType pixelType = V3D_UINT8;
		if (bIncludeSignal && (originalStack.su > pixelType))
			pixelType = (ImagePixelType)originalStack.su;
		if (bIncludeReference && (referenceStack.su > pixelType))
			pixelType = (ImagePixelType)referenceStack.su;

		// Create output image
		My4DImage outputImage;
		outputImage.createImage(sx, sy, sz, sc, pixelType);
	    // Write each channel
	    int cOut = 0;
	    bool bShowBackground = selectionReader.overlayIsChecked(DataFlowModel::BACKGROUND_MIP_INDEX);
	    if (bIncludeSignal) {
			for (int c = 0; c < originalStack.sc; ++c) {
				// Only export visible color channels
				if (colorReader.getChannelVisibility(c))
				{
					copyChannel(outputImage, cOut, originalStack, c,
							labelStack, maskStatusList, bShowBackground, true);
					++cOut; // prepare next channel
				}
			}
	    }
	    if (bIncludeReference) {
			copyChannel(outputImage, cOut, referenceStack, 0,
					labelStack, maskStatusList, bShowBackground, false);
			++cOut; // prepare next channel
	    }

        if (is2D) {

            qDebug() << "is2D==true";

            qDebug() << "Calling padAndRotateImage";
            My4DImage* rotatedImage = padAndRotateImage(&outputImage, cameraModel.rotation(), true /* flipY */);

            QString rotatedString(filename);
            rotatedString.append("_rotate.v3dpbd");
            ImageLoader rotatedDebugger;
            rotatedDebugger.saveImage(rotatedImage, rotatedString);

            qDebug() << "Calling createMIPFromImage";
            My4DImage* rotatedMip = AnalysisTools::createMIPFromImage(rotatedImage);

            qDebug() << "Deleting rotatedImage";
            delete rotatedImage;

            qDebug() << "Calling imageLoader.saveImage";
            if (imageLoader.saveImage(rotatedMip, filename)) {
                stopped = true;
            } else {
                bSucceeded = false;
                message = "Failed to write to file";
                stopped = true;
            }

            qDebug() << "Calling delete rotatedMip";
            delete rotatedMip;

        } else {

            qDebug() << "is2D==false";

            if (imageLoader.saveImage(&outputImage, filename)) {
                stopped = true;
            }
            else {
                bSucceeded = false;
                message = "Failed to write to file";
                stopped = true;
            }

        }
    } // release read locks

	if (bSucceeded)
	{
		emit exportFinished(filename);
	}
	else
	{
		emit exportFailed(filename, message);
	}

    //
    return;
}

My4DImage* ExportFile::padAndRotateImage(My4DImage* image, const Rotation3D& rotation, bool flipY)
{
    // To allocate space for a rotated version of an image, we need to account for a
    // larger size, which is the largest diagonal length.

    long xDim=image->getXDim();
    long yDim=image->getYDim();
    long zDim=image->getZDim();
    int cDim=image->getCDim();

    qDebug() << "Export xDim=" << xDim << " yDim=" << yDim << " zDim= " << zDim;

    double xyLength = sqrt((double)(xDim*xDim + yDim*yDim)); // major axis=Z, 2
    double yzLength = sqrt((double)(yDim*yDim + zDim*zDim)); // major axis=X, 0
    double zxLength = sqrt((double)(zDim*zDim + xDim*xDim)); // major axis=Y, 1

    int majorAxis=0;
    double majorLengthD=0.0;

    if (xyLength > yzLength && xyLength > zxLength) {
        majorAxis=2;
        majorLengthD=xyLength;
    } else if (yzLength > xyLength && yzLength > zxLength)  {
        majorAxis=0;
        majorLengthD=yzLength;
    } else {
        majorAxis=1;
        majorLengthD=zxLength;
    }

    long majorLength=majorLengthD;
    majorLength++;

    // We need to create a cube of this dimension
    My4DImage* rotatedImage = new My4DImage();
    rotatedImage->loadImage(majorLength, majorLength, majorLength, image->getCDim(), image->getDatatype());
    long xTim=majorLength;
    long yTim=majorLength;
    long zTim=majorLength;

    rotatedImage->loadImage(xTim, yTim, zTim, image->getCDim(), image->getDatatype());

    // Step through the original image, and for each voxel make an intensity contribution according to
    // subvoxel proximity, such that the entire intensity value is transferred.
    v3d_uint8** data8=0L;
    v3d_uint16** data16=0L;

    v3d_uint8** target8=0L;
    v3d_uint16** target16=0L;

    qDebug() << "Image datatype=" << image->getDatatype();

    if (image->getDatatype()==V3D_UINT8) {
        qDebug() << "Using 8-bit data";
        data8=new v3d_uint8*[cDim];
        target8=new v3d_uint8*[cDim];
        for (int c=0;c<cDim;c++) {
            data8[c]=image->getRawDataAtChannel(c);
            target8[c]=rotatedImage->getRawDataAtChannel(c);
        }
    } else {
        qDebug() << "Using 16-bit data";
        data16=new v3d_uint16*[cDim];
        target16=new v3d_uint16*[cDim];
        for (int c=0;c<cDim;c++) {
            data16[c]=(v3d_uint16*)image->getRawDataAtChannel(c);
            target16[c]=(v3d_uint16*)rotatedImage->getRawDataAtChannel(c);
        }
    }

    Vector3D vec = rotation.convertBodyFixedXYZRotationToThreeAngles();

    double rotationX=vec.m_x;
    double rotationY=vec.m_y;
    double rotationZ=vec.m_z;

    Rotation3D rotationT = rotation;

    rotationT.setRotationFromBodyFixedXYZAngles(-1.0 * rotationX, -1.0 * rotationY, rotationZ);

    for (long z=0;z<zDim;z++) {
        long zOffset=z*yDim*xDim;
        for (long y=0;y<yDim;y++) {
            long fY=y;
            if (flipY) {
                fY=yDim-y-1;
            }
            long yOffset=fY*xDim;
            for (long x=0;x<xDim;x++) {

                long cX=x-xDim/2;
                long cY=y-yDim/2;
                long cZ=z-zDim/2;

                Vector3D imagePosition=Vector3D(cX,cY,cZ);
                Vector3D rotationPosition=rotationT*imagePosition;

                long rX=rotationPosition.m_x+xDim/2;
                long rY=rotationPosition.m_y+yDim/2;
                long rZ=rotationPosition.m_z+zDim/2;

                long offset=zOffset+yOffset+x;

                long targetX=rX+(majorLength-xDim)/2;
                long targetY=rY+(majorLength-yDim)/2;
                long targetZ=rZ+(majorLength-zDim)/2;

                if (targetX >= xTim) {
                    targetX=xTim-1;
                } else if (targetX < 0) {
                    targetX=0;
                }
                if (targetY >= yTim) {
                    targetY=yTim-1;
                } else if (targetY < 0) {
                    targetY=0;
                }
                if (targetZ >= zTim) {
                    targetZ=zTim-1;
                } else if (targetZ < 0) {
                    targetZ=0;
                }

                long tOffset=targetZ*yTim*xTim + targetY*xTim + targetX;

                for (int c=0;c<cDim;c++) {
                    if (data8 != 0L) {
                        v3d_uint8* d8=image->getRawDataAtChannel(c);
                        v3d_uint8* t8=rotatedImage->getRawDataAtChannel(c);
                        t8[tOffset]=d8[offset];
                    } else {
                        target16[c][tOffset]=data16[c][offset];
                    }
                }



            }
        }
    }

    return rotatedImage;

}
