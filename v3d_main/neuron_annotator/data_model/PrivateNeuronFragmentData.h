#ifndef PRIVATENEURONFRAGMENTDATA_H
#define PRIVATENEURONFRAGMENTDATA_H

#include <QSharedData>
#include <vector>

class PrivateNeuronFragmentData : public QSharedData
{
public:
    PrivateNeuronFragmentData();
    PrivateNeuronFragmentData(const PrivateNeuronFragmentData&);
    virtual ~PrivateNeuronFragmentData();

    int getNumberOfFragments() const;
    const std::vector<int>& getFragmentSizes() const; // in voxels
    const std::vector<float>& getFragmentHues() const; // in range 0.0-1.0

    void setNumberOfFragments(int);
    void setFragmentSize(int index, int size);
    void setFragmentHue(int index, float hue);

protected:
    std::vector<int> fragmentVoxelCount;
    std::vector<float> fragmentHues;
};

#endif // PRIVATENEURONFRAGMENTDATA_H
