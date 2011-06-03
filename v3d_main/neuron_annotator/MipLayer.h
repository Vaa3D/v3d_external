#ifndef MIPLAYER_H
#define MIPLAYER_H

#include <QObject>
#include <QSize>

// Intermediate image type for creating combined MIP projections from individual neuron MIP projections.
class MipLayer : public QObject
{
    Q_OBJECT

public:

    // This is the minimum information needed to track all of the MIP information
    class Pixel {
    public:
        Pixel() : intensity(0.0f), neuronIndex(-1), zCoordinate(-1) {}
        // eight bytes; might result in decent memory alignment
        float intensity; // (weighted?) sum of all relevant data channels
        short neuronIndex; // index of leaf node MipLayer
        short zCoordinate; // to help look up original color in the volume
    };

    explicit MipLayer(QSize size, QObject *parent = 0);
    // MIPLayers can be combined in a binary tree
    explicit MipLayer(const MipLayer* child1, const MipLayer* child2, QObject *parent = 0);
    virtual ~MipLayer();
    QSize size() const;
    // Remember to manually emit layerChanged() after you are done manually changing the image!
    MipLayer::Pixel * scanLine(short y);
    const Pixel * scanLine(short y) const;
    void enable(bool isEnabled);
    void enableWithSignal(bool isEnabled);
    bool isEnabled() const;
    MipLayer::Pixel& getPixel(int x, int y) {return m_pixelData[x + y * m_size.width()];}

signals:
    void layerChanged();

public slots:
    void updateData();

protected:
    QSize m_size;
    MipLayer::Pixel * m_pixelData;
    bool m_isEnabled; // efficiency variable indicating disabled or blank image
    const MipLayer * child1;
    const MipLayer * child2;
};

#endif // MIPLAYER_H
