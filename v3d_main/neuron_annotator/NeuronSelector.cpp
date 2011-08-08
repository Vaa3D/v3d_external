#include "NeuronSelector.h"
#include <QtAlgorithms>
#include <iostream>

using namespace std;


// NeuronSelector init func
void NeuronSelector::init()
{
	//
	index = -1;
}

// get the index of selected neuron
int NeuronSelector::getIndexSelectedNeuron()
{
        NeuronSelectionModel::Reader selectionReader(
                annotationSession->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return -1;

	// find in mask stack
	sx = annotationSession->getNeuronMaskAsMy4DImage()->getXDim();
	sy = annotationSession->getNeuronMaskAsMy4DImage()->getYDim();
        sz = annotationSession->getNeuronMaskAsMy4DImage()->getZDim();
		
	// sum of pixels of each neuron mask in the cube 
	int *sum = NULL;
	
        int numNeuron = selectionReader.getMaskStatusList().size();
	
	qDebug()<<"how many neurons ... "<<numNeuron;
	
	try
	{
		sum = new int [numNeuron];		
		
		// init 0 
		//memset(sum, 0, numNeuron); 
		for(V3DLONG i=0; i<numNeuron; i++)
		{
			sum[i] = 0;
		}
	}
	catch (...) 
	{
		printf("Out of memory\n");
		return -1;
	}
	
	//
	V3DLONG xb = xlc-NB; if(xb<0) xb = 0;
	V3DLONG xe = xlc+NB; if(xe>sx) xe = sx-1;
	V3DLONG yb = ylc-NB; if(yb<0) yb = 0;
	V3DLONG ye = ylc+NB; if(ye>sy) ye = sy-1;
	V3DLONG zb = zlc-NB; if(zb<0) zb = 0;
	V3DLONG ze = zlc+NB; if(ze>sz) ze = sz-1;

        const unsigned char *neuronMask = annotationSession->getNeuronMaskAsMy4DImage()->getRawData();
        const QList<bool>& maskStatusList=selectionReader.getMaskStatusList();
	
	for(V3DLONG k=zb; k<=ze; k++)
	{
		V3DLONG offset_k = k*sx*sy;
		for(V3DLONG j=yb; j<=ye; j++)
		{
			V3DLONG offset_j = offset_k + j*sx;
			for(V3DLONG i=xb; i<=xe; i++)
			{
				V3DLONG idx = offset_j + i;
				
                                int cur_idx = neuronMask[idx] - 1; // value of mask stack - convert to 0...n-1 neuron index
				
                                if(cur_idx>=0 && maskStatusList.at(cur_idx)) // active masks
				{
					sum[cur_idx]++;
				}
			}
		}
	}
	
	//
        index = -1;
        int neuronSum=0;
        for(V3DLONG i=0; i<numNeuron; i++)
	{            

            if(sum[i]>0 && sum[i]>neuronSum) {
                neuronSum=sum[i];
                index = i;
            }

	}
	
	// de-alloc
	if(sum) {delete []sum; sum = NULL;}
	
	//
        qDebug() << "NeuronSelector::getIndexSelectedNeuron index=" << index;

        if(index>-1)
	{
                // switchSelectedNeuronUniquelyIfOn() modifies the
                // NeuronSelectionModel, and thus needs a Writer.
                // But before getting a Writer, all Readers, including
                // ours, must unlock.
                selectionReader.unlock();
                NeuronSelectionModel::Writer selectionWriter(
                        annotationSession->getNeuronSelectionModel());
                selectionWriter.switchSelectedNeuronUniquelyIfOn(index);
	}
	else
	{
            // Debug
            // LocationSimple p((V3DLONG)xlc, (V3DLONG)ylc, (V3DLONG)zlc);
            // RGBA8 c;
            // c.r = 255; c.g = 0; c.b = 0; c.a = 128;// cyan
            // p.color = c; // instead of random_rgba8(255);
            // p.radius = 1; // instead of 5
            // annotationSession->getOriginalImageStackAsMy4DImage()->listLandmarks.append(p);
            // emit neuronHighlighted(false);
            // end-Debug

            index = -1; // 0 is background
	}
	
	return index;
}

//
void NeuronSelector::getCurNeuronBoundary()
{
        index = getIndexSelectedNeuron();
	
        if(index<0) return;
	
	//
	curNeuronBDxb = sx-1;
	curNeuronBDxe = 0;
	
	curNeuronBDyb = sy-1;
	curNeuronBDye = 0;
	
	curNeuronBDzb = sz-1;
	curNeuronBDze = 0;

        const unsigned char *neuronMask = annotationSession->getNeuronMaskAsMy4DImage()->getRawData();
	
	//
	for(V3DLONG k=0; k<sz; k++)
	{
		V3DLONG offset_k = k*sx*sy;
		for(V3DLONG j=0; j<sy; j++)
		{
			V3DLONG offset_j = offset_k + j*sx;
			for(V3DLONG i=0; i<sx; i++)
			{
				V3DLONG idx = offset_j + i;
				
                                if(index == (neuronMask[idx]-1))
				{
					if(i<curNeuronBDxb) curNeuronBDxb = i;
					if(i>curNeuronBDxe) curNeuronBDxe = i;
					
					if(j<curNeuronBDyb) curNeuronBDyb = j;
					if(j>curNeuronBDye) curNeuronBDye = j;
					
                                        if(k<curNeuronBDzb) curNeuronBDzb = k;
                                        if(k>curNeuronBDze) curNeuronBDze = k;
				}
				
			}
		}
	}
	
	// x
	if( curNeuronBDxb-NB > 0 )
	{
		curNeuronBDxb -= NB;
	}
	else
	{
		curNeuronBDxb = 0;
	}
	
	if(curNeuronBDxe+NB < sx)
	{
		curNeuronBDxe += NB;
	}
	else
	{
		curNeuronBDxe = sx - 1;
	}
	
	// y
	if( curNeuronBDyb-NB > 0 )
	{
		curNeuronBDyb -= NB;
	}
	else
	{
		curNeuronBDyb = 0;
	}
	
	if(curNeuronBDye+NB < sy)
	{
		curNeuronBDye += NB;
	}
	else
	{
		curNeuronBDye = sy - 1;
	}
	
	// z
	if( curNeuronBDzb-NB > 0 )
	{
		curNeuronBDzb -= NB;
	}
	else
	{
		curNeuronBDzb = 0;
	}
	
	if(curNeuronBDze+NB < sz)
	{
		curNeuronBDze += NB;
	}
	else
	{
		curNeuronBDze = sz - 1;
	}
}

//
bool NeuronSelector::inNeuronMask(V3DLONG x, V3DLONG y, V3DLONG z)
{
	if(x<=curNeuronBDxe && x>=curNeuronBDxb && y<=curNeuronBDye && y>=curNeuronBDyb && z<=curNeuronBDze && z>=curNeuronBDzb)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
void NeuronSelector::setAnnotationSession(AnnotationSession* annotationSession) {
	this->annotationSession=annotationSession;
}

// 
void NeuronSelector::updateSelectedPosition(double x, double y, double z) {
        xlc = x + 0.5;
        ylc = y + 0.5;
        zlc = z + 0.5;
	
	qDebug()<<"test signal/slot passing parameters ..."<<xlc<<ylc<<zlc;
	
	//
	highlightSelectedNeuron();
}

void NeuronSelector::deselectCurrentNeuron()
{
    NeuronSelectionModel::Writer selectionWriter(
            annotationSession->getNeuronSelectionModel());
    if (selectionWriter.getNeuronSelectList().at(index)==true) {
        selectionWriter.getNeuronSelectList().replace(index, false);
    }
    selectionWriter.unlock();
    annotationSession->getOriginalImageStackAsMy4DImage()->listLandmarks.clear();
    emit neuronHighlighted(false);
}

// highlight selected neuron
void NeuronSelector::highlightSelectedNeuron()
{
    getCurNeuronBoundary();
	
    if(index<0) return;
    if(curNeuronBDxb>curNeuronBDxe || curNeuronBDyb>curNeuronBDye || curNeuronBDzb>curNeuronBDze) return;

    NeuronSelectionModel::Reader selectionReader(
            annotationSession->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) return;
    bool bIsSelected = selectionReader.getNeuronSelectList().at(index);
    selectionReader.unlock(); // unlock early because we are done with reader

    // index neuron selected status is true
    if(! bIsSelected)
    {
        // highlight result
        annotationSession->getOriginalImageStackAsMy4DImage()->listLandmarks.clear();

        // synchronize markers shown in 3d viewer
        emit neuronHighlighted(false);

        return;
    }

    // list of markers
    QList<LocationSimple> listLandmarks;

    const unsigned char *neuronMask = annotationSession->getNeuronMaskAsMy4DImage()->getRawData();

    // mesh grids
    for(V3DLONG k=0; k<sz; k+=STEP){
        for(V3DLONG j=0; j<sy; j+=STEP){
            for(V3DLONG i=0; i<sx; i+=STEP){

                if(inNeuronMask(i,j,k))
                {
                    // find avg position in cube around lattice point
                    float sumx, sumy, sumz;
                    V3DLONG count;
                    sumx = 0.0;
                    sumy = 0.0;
                    sumz = 0.0;
                    count = 0;

                    //
                    V3DLONG xb = i-NB; if(xb<0) xb = 0;
                    V3DLONG xe = i+NB; if(xe>sx) xe = sx-1;
                    V3DLONG yb = j-NB; if(yb<0) yb = 0;
                    V3DLONG ye = j+NB; if(ye>sy) ye = sy-1;
                    V3DLONG zb = k-NB; if(zb<0) zb = 0;
                    V3DLONG ze = k+NB; if(ze>sz) ze = sz-1;

                    for(V3DLONG kk=zb; kk<=ze; kk++)
                    {
                        V3DLONG offset_kk = kk*sx*sy;
                        for(V3DLONG jj=yb; jj<=ye; jj++)
                        {
                            V3DLONG offset_jj = offset_kk + jj*sx;
                            for(V3DLONG ii=xb; ii<=xe; ii++)
                            {
                                V3DLONG idx = offset_jj + ii;

                                if(index == (neuronMask[idx]-1))
                                {
                                    count++;
                                    sumx += ii;
                                    sumy += jj;
                                    sumz += kk;
                                }
                            }
                        }
                    }

                    // append a marker
                    if(count>0)
                    {
                        LocationSimple p((V3DLONG)(sumx/(float)count+1.5), (V3DLONG)(sumy/(float)count+1.5), (V3DLONG)(sumz/(float)count+1.5)); // 1-based
                        RGBA8 c;
                        c.r = 0; c.g = 255; c.b = 255; c.a = 128;// cyan
                        p.color = c; // instead of random_rgba8(255);
                        p.radius = 1; // instead of 5

                        // QString qstr = QString("Neuron %1").arg(index);
                        // p.name = qstr.toStdString().c_str();

                        listLandmarks.append(p);
                    }
                }
            }
        }
    }

    // highlight result
    annotationSession->getOriginalImageStackAsMy4DImage()->listLandmarks=listLandmarks;

    qDebug()<<"highlight selected neuron ..."<<listLandmarks.size();

    // synchronize markers shown in 3d viewer
    emit neuronHighlighted(false);

}

void NeuronSelector::updateSelectedNeurons()
{
    if(index<0) return;

    NeuronSelectionModel::Reader selectionReader(
            annotationSession->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) return;
    bool bIsVisible = selectionReader.getMaskStatusList().at(index);
    selectionReader.unlock();
    if(! bIsVisible)
        deselectCurrentNeuron();
}

void NeuronSelector::updateNeuronSelectList(int neuronIndex)
{
    index = neuronIndex;

    qDebug() << "NeuronSelector::updateNeuronSelectList() neuronIndex=" << neuronIndex;

    NeuronSelectionModel::Writer selectionWriter(
            annotationSession->getNeuronSelectionModel());
    selectionWriter.updateNeuronSelectList(index);
}

void NeuronSelector::clearAllSelections() {
    // Clear selection landmarks
    annotationSession->getOriginalImageStackAsMy4DImage()->listLandmarks.clear();
    emit neuronHighlighted(false);
    NeuronSelectionModel::Writer selectionWriter(
            annotationSession->getNeuronSelectionModel());
    selectionWriter.clearSelections();
}

