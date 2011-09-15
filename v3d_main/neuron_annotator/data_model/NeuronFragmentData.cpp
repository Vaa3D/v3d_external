#include "NeuronFragmentData.h"
#include "PrivateNeuronFragmentData.h"
#include "NaSharedDataModel.cpp"

NeuronFragmentData::NeuronFragmentData()
    : naVolumeData(NULL)
{}

/* explicit */
NeuronFragmentData::NeuronFragmentData(const NeuronFragmentData& rhs)
    : naVolumeData(rhs.naVolumeData)
{
    if (naVolumeData)
        connect(naVolumeData, SIGNAL(dataChanged()),
                this, SLOT(update()));
}

/* explicit */
NeuronFragmentData::NeuronFragmentData(const NaVolumeData& naVolumeDataParam)
    : naVolumeData(&naVolumeDataParam)
{
    connect(naVolumeData, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

/* slot */
void NeuronFragmentData::update()
{
    qDebug() << "NeuronFragmentData::update()" << __FILE__ << __LINE__;
}


