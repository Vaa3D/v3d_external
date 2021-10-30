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
    NeuronFragmentData& operator=(const NeuronFragmentData&);
    virtual ~NeuronFragmentData();

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
        Reader(const Reader& rhs);
        Reader& operator=(const Reader& rhs);
        virtual ~Reader();
        int getNumberOfFragments() const;
        const std::vector<int>& getFragmentSizes() const; // in voxels
        const std::vector<float>& getFragmentHues() const; // in range 0.0-1.0
    };


    class Writer; friend class Writer;
    class Writer : public BaseWriter
    {
    public:
        Writer(NeuronFragmentData& fragmentData) : BaseWriter(fragmentData) {}
        Writer(const Writer& rhs);
        Writer& operator=(const Writer&);
        virtual ~Writer();
    };

};

#endif // NEURONFRAGMENTDATA_H
