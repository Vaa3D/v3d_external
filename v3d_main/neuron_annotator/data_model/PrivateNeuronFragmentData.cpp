#include "PrivateNeuronFragmentData.h"
#include <cassert>
#include <QDebug>

PrivateNeuronFragmentData::PrivateNeuronFragmentData()
{}

PrivateNeuronFragmentData::PrivateNeuronFragmentData(const PrivateNeuronFragmentData& rhs)
{
    fragmentVoxelCount = rhs.fragmentVoxelCount;
    fragmentHues = rhs.fragmentHues;
    qDebug() << "PrivateNeuronFragmentData copy constructor";
}

/* virtual */
PrivateNeuronFragmentData::~PrivateNeuronFragmentData()
{}

int PrivateNeuronFragmentData::getNumberOfFragments() const
{
    return (int)fragmentVoxelCount.size();
}

const std::vector<int>& PrivateNeuronFragmentData::getFragmentSizes() const // in voxels
{
    return fragmentVoxelCount;
}

const std::vector<float>& PrivateNeuronFragmentData::getFragmentHues() const // in range 0.0-1.0
{
    return fragmentHues;
}

void PrivateNeuronFragmentData::setNumberOfFragments(int n)
{
    // qDebug() << "PrivateNeuronFragmentData::setNumberOfFragments" << n;
    if (n != fragmentVoxelCount.size()) {
        fragmentVoxelCount.assign(n, 0);
        fragmentHues.assign(n, 0);
    }
    assert(fragmentHues.size() == n);
}

void PrivateNeuronFragmentData::setFragmentSize(int index, int size)
{
    // qDebug() << fragmentVoxelCount.size();
    if (fragmentVoxelCount.size() <= index) return;
    fragmentVoxelCount[index] = size;
}

void PrivateNeuronFragmentData::setFragmentHue(int index, float hue)
{
    // qDebug() << fragmentHues.size();
    // clamp to 0.0-1.0 range
    while (hue < 0) hue += 1.0;
    while (hue >= 1.0) hue -= 1.0;
    if (fragmentHues.size() <= index) return;
    fragmentHues[index] = hue;
}

