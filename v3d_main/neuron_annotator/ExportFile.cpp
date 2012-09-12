#include "ExportFile.h"
#include "DataFlowModel.h"
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
		const DataColorModel& colorModel)
	: filename(fileName)
	, volumeData(volumeData)
	, selectionModel(selectionModel)
    , colorModel(colorModel)
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

	    if (imageLoader.saveImage(&outputImage, filename)) {
	    	    stopped = true;
	    }
	    else {
	    	    bSucceeded = false;
	    	    message = "Failed to write to file";
	    	    stopped = true;
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
