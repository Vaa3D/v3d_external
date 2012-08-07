#include "NeuronSelector.h"
#include <QtAlgorithms>
#include <iostream>
#include <cassert>

using namespace std;

NeuronSelector::NeuronSelector(QObject * parentParam)
    : QObject(parentParam)
    , index(-1)
    , dataFlowModel(NULL)
    , sx(0), sy(0), sz(0)
    , curNeuronBDxb(0), curNeuronBDyb(0), curNeuronBDzb(0)
    , curNeuronBDxe(0), curNeuronBDye(0), curNeuronBDze(0)
{
    // allow passing of QList<ImageMarker> through signals/slots
    qRegisterMetaType<QList<ImageMarker> >("QList<ImageMarker>");
}

// NeuronSelector init func.
void NeuronSelector::onVolumeDataChanged()
{
    if (! dataFlowModel) return;
    NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();

    sx = neuronProxy.sx;
    sy = neuronProxy.sy;
    sz = neuronProxy.sz;

    curNeuronBDxb = sx-1;
    curNeuronBDxe = 0;

    curNeuronBDyb = sy-1;
    curNeuronBDye = 0;

    curNeuronBDzb = sz-1;
    curNeuronBDze = 0;
}

// get the index of selected neuron
int NeuronSelector::getIndexSelectedNeuron()
{
    // qDebug() << "NeuronSelector::getIndexSelectedNeuron()" << __FILE__ << __LINE__;
    int numNeuron = 0;
    if (NULL == dataFlowModel) return -1;
    // sum of pixels of each neuron mask in the cube
    std::vector<int> sum;
    {
            NeuronSelectionModel::Reader selectionReader(
                    dataFlowModel->getNeuronSelectionModel());
            if (! selectionReader.hasReadLock()) return -1;

            NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
            if (! volumeReader.hasReadLock()) return -1;
            const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();

            numNeuron = selectionReader.getMaskStatusList().size();

            // qDebug()<<"how many neurons ... "<<numNeuron;

            sum.assign(numNeuron, 0);

            //
            V3DLONG xb = xlc-NB; if(xb<0) xb = 0;
            V3DLONG xe = xlc+NB; if(xe>sx) xe = sx-1;
            V3DLONG yb = ylc-NB; if(yb<0) yb = 0;
            V3DLONG ye = ylc+NB; if(ye>sy) ye = sy-1;
            V3DLONG zb = zlc-NB; if(zb<0) zb = 0;
            V3DLONG ze = zlc+NB; if(ze>sz) ze = sz-1;

            // const unsigned char *neuronMask = neuronProxy.img0->getRawData();
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

                                    // int cur_idx = neuronMask[idx] - 1; // value of mask stack - convert to 0...n-1 neuron index
                                    int cur_idx = neuronProxy.value_at(i, j, k, 0) - 1;

                                    if(cur_idx>=0 && maskStatusList.at(cur_idx)) // active masks
                                    {
                                            sum[cur_idx]++;
                                    }
                            }
                    }
            }
        } // release locks
	
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
	
	//
        // qDebug() << "NeuronSelector::getIndexSelectedNeuron index=" << index << __FILE__ << __LINE__;

        if(index < 0) index = -1; // Index zero is a real fragment
	
	return index;
}

void NeuronSelector::getCurIndexNeuronBoundary()
{
        if(index<0) return;

        {
            NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
            if (! volumeReader.hasReadLock()) return;
            const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();

            // const unsigned char *neuronMask = neuronProxy.img0->getRawData();

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

                                    // if(index == (neuronMask[idx]-1))
                                    if (index == (neuronProxy.value_at(i, j, k, 0) - 1) )
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
        } // release lock
	
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
void NeuronSelector::setDataFlowModel(const DataFlowModel* dataFlowModelParam)
{
    dataFlowModel = dataFlowModelParam;
    if (NULL == dataFlowModel)
        return; // no connections possible
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(selectionChanged()),
            this, SLOT(onSelectionModelChanged()));
    connect(this, SIGNAL(neuronSelected(int)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(selectExactlyOneNeuron(int)));
    connect(this, SIGNAL(selectionClearNeeded()),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));
    connect(&dataFlowModel->getVolumeData(), SIGNAL(dataChanged()),
            this, SLOT(onVolumeDataChanged()));
    // init();
}

// 
void NeuronSelector::updateSelectedPosition(double x, double y, double z)
{
    // qDebug() << "NeuronSelector::updateSelectedPosition()" << x << y << z << __FILE__ << __LINE__;
        xlc = x + 0.5;
        ylc = y + 0.5;
        zlc = z + 0.5;
	
        // qDebug()<<"test signal/slot passing parameters ..."<<xlc<<ylc<<zlc;
	
	//
        index = getIndexSelectedNeuron();
        if (index < 0) return;
        // reaction depends on whether neuron is already selected
        bool alreadySelected;
        {
            NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
            if (! selectionReader.hasReadLock()) return;
            alreadySelected = (selectionReader.getNeuronSelectList()[index]);
        } // release lock before emit
        if (alreadySelected) {
            // qDebug() << "selectionClearNeeded()";
            emit selectionClearNeeded();
        }
        else {
            // const QList<ImageMarker> landmarks = highlightIndexNeuron();
            // qDebug() << "updateSelectedPosition" << x << y << z;
            emit neuronSelected(index);
            // if (landmarks.size() > 0) {
            //     // qDebug() << landmarks[0].radius << __FILE__ << __LINE__;
            //     emit landmarksUpdateNeeded(landmarks);
            // }
        }
}

// highlight selected neuron
QList<ImageMarker> NeuronSelector::highlightIndexNeuron()
{
    // qDebug() << "NeuronSelector::highlightIndexNeuron" << index << __FILE__ << __LINE__;
    // list of markers
    QList<ImageMarker> listLandmarks;

    getCurIndexNeuronBoundary();
	
    if(index<0) return listLandmarks;
    if(curNeuronBDxb>curNeuronBDxe || curNeuronBDyb>curNeuronBDye || curNeuronBDzb>curNeuronBDze)
        return listLandmarks;

    bool bIsSelected;
    { // demarcate read lock
        NeuronSelectionModel::Reader selectionReader(
                dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return listLandmarks;
        bIsSelected = selectionReader.getNeuronSelectList().at(index);
    } // release lock

    // index neuron selected status is true
    if(! bIsSelected)
    {
        return listLandmarks;
    }

    // get neuron color
    RGBA8 color;
    color.r = 0; color.g = 255; color.b = 255; color.a = 128;// cyan
    {
       NeuronFragmentData::Reader fragmentReader(dataFlowModel->getNeuronFragmentData());
       if (! dataFlowModel->getNeuronFragmentData().readerIsStale(fragmentReader)) {
           qreal hue = (fragmentReader.getFragmentHues())[index];
           qreal saturation = 0.8;
           qreal value = 0.7;
           qreal alpha = 1.0;
           QColor c;
           c.setHsvF(hue, saturation, value, alpha);
           color.r = c.red();
           color.g = c.green();
           color.b = c.blue();
           color.a = c.alpha();
           // qDebug() << "neuron color =" << c << hue << saturation << value << alpha;
       }
    }

    // qDebug() << "NeuronSelector::highlightIndexNeuron" << __FILE__ << __LINE__;
    { // read lock stanza
        // const unsigned char *neuronMask = dataFlowModel->getNeuronMaskAsMy4DImage()->getRawData();
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return listLandmarks;
        const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();
        sx = neuronProxy.sx;
        sy = neuronProxy.sy;
        sz = neuronProxy.sz;

        // mesh grids
        for(V3DLONG k=0; k<neuronProxy.sz; k+=STEP){
            for(V3DLONG j=0; j<neuronProxy.sy; j+=STEP){
                for(V3DLONG i=0; i<neuronProxy.sx; i+=STEP){

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
                        V3DLONG xe = i+NB; if(xe>=sx) xe = sx-1;
                        V3DLONG yb = j-NB; if(yb<0) yb = 0;
                        V3DLONG ye = j+NB; if(ye>=sy) ye = sy-1;
                        V3DLONG zb = k-NB; if(zb<0) zb = 0;
                        V3DLONG ze = k+NB; if(ze>=sz) ze = sz-1;

                        for(V3DLONG kk=zb; kk<=ze; kk++)
                        {
                            V3DLONG offset_kk = kk*sx*sy;
                            for(V3DLONG jj=yb; jj<=ye; jj++)
                            {
                                V3DLONG offset_jj = offset_kk + jj*sx;
                                for(V3DLONG ii=xb; ii<=xe; ii++)
                                {
                                    V3DLONG idx = offset_jj + ii;

                                    // qDebug() << index << neuronProxy.value_at(ii, jj, kk, 0);
                                    if(index == ((int)neuronProxy.value_at(ii, jj, kk, 0)) - 1) // off by one
                                    {
                                        // qDebug() << "found landmark";
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
                            ImageMarker p;
                            p.x = (V3DLONG)(sumx/(float)count+1.5); // 1-based
                            p.y = (V3DLONG)(sumy/(float)count+1.5);
                            p.z = (V3DLONG)(sumz/(float)count+1.5);
                            // RGBA8 c;
                            // c.r = 0; c.g = 255; c.b = 255; c.a = 128;// cyan
                            p.color = color; // instead of random_rgba8(255);
                            p.radius = 1; // instead of 5
                            p.on = true;
                            p.shape = 2; // 2: dodecahedron

                            // QString qstr = QString("Neuron %1").arg(index);
                            // p.name = qstr.toStdString().c_str();

                            // qDebug() << "Appending landmark";
                            listLandmarks.append(p);
                        }
                    }
                }
            }
        }
    } // release read lock
    // qDebug() << listLandmarks.size() << "landmarks found";
    if (listLandmarks.size() > 0) {
        // qDebug() << listLandmarks[0].radius << __FILE__ << __LINE__;
    }
    return listLandmarks;
}

void NeuronSelector::onSelectionModelChanged()
{
    // qDebug() << "NeuronSelector::onSelectionModelChanged";
    QList<int> selectedIndices;
    {
        NeuronSelectionModel::Reader selectionReader(
                dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return;
        const QList<bool>& neuronSelectList = selectionReader.getNeuronSelectList();
        for (int n = 0; n < neuronSelectList.size(); ++n)
            if (neuronSelectList[n])
                selectedIndices << n;
    } // release lock before emit
    // nothing selected?
    if (selectedIndices.size() == 0)
    {
        // qDebug() << "NeuronSelector: no selections";
        if (index < 0) return; // nothing is highlighted already
        index = -1;
        emit landmarksClearNeeded();
        return;
    }
    else // something is selected
    {
        // qDebug() << "NeuronSelector: single neuron selected";
        assert(selectedIndices.size() > 0);
        // if (index == selectedIndices[0]) return; // no change; NO - Selection might have originated from NeuronSelector.
        index = selectedIndices[0]; // set "selected" index to first item in list
        // accumulate little spheres to cover each selected neuron
        // TODO - highlight multiple neurons
        // Just highlight the first neuron in the list for now
        const QList<ImageMarker> landmarks = highlightIndexNeuron();
        // qDebug() << "number of landmarks =" << landmarks.size();
        if (landmarks.size() > 0) {
            // qDebug() << landmarks[0].radius << __FILE__ << __LINE__;
            emit landmarksUpdateNeeded(landmarks);
        }
        return;
    }
}
