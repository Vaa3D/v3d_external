#ifndef PRIVATENEURONFRAGMENTDATA_H
#define PRIVATENEURONFRAGMENTDATA_H

#include "NeuronSeparationResult.h"
#include <QSharedData>
#include <vector>
#include <iostream>

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
    // TODO obsolete fragmentHues and fragmentVoxelCount in favor of fragments
    jfrc::NeuronSeparationResult neuronSeparationResult;
};

#endif // PRIVATENEURONFRAGMENTDATA_H
