#ifndef PRIVATENEURONFRAGMENTDATA_H
#define PRIVATENEURONFRAGMENTDATA_H

#include <QSharedData>
#include <QColor>
#include <vector>

class PrivateNeuronFragmentData : public QSharedData
{
public:
    PrivateNeuronFragmentData();
    PrivateNeuronFragmentData(const PrivateNeuronFragmentData&);
    virtual ~PrivateNeuronFragmentData();

protected:
    std::vector<int> fragmentVoxelCount;
    std::vector<QRgb> fragmentColor;
};

#endif // PRIVATENEURONFRAGMENTDATA_H
