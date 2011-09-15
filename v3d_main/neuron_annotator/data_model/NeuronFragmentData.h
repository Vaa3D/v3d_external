#ifndef NEURONFRAGMENTDATA_H
#define NEURONFRAGMENTDATA_H

#include "NaSharedDataModel.h"
#include "NaVolumeData.h"

class PrivateNeuronFragmentData;

// NeuronFragmentData contains information about Neuron Fragments
class NeuronFragmentData : public NaSharedDataModel<PrivateNeuronFragmentData>
{
    Q_OBJECT

public:
    NeuronFragmentData();
    explicit NeuronFragmentData(const NeuronFragmentData&);
    explicit NeuronFragmentData(const NaVolumeData&);

public slots:
    virtual void update();

protected:
    const NaVolumeData* naVolumeData;


public:
    class Reader; friend class Reader;
    class Reader : public BaseReader
    {
    public:
        Reader(const NeuronFragmentData&);
        int getNumberOfFragments() const;
        int getNumberOfVoxels(int fragmentIndex) const;
        int getNumberOfDataChannels() const;
        // TODO - how to handle color?
    };


    class Writer; friend class Writer;
    class Writer : public BaseWriter
    {};

};

#endif // NEURONFRAGMENTDATA_H
