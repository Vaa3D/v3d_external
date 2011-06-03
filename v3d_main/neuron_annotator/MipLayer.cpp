#include "MipLayer.h"
#include <cassert>
#include <QDebug>

MipLayer::MipLayer(QSize size, QObject *parent)
    : QObject(parent)
    , m_size(size)
    , child1(NULL)
    , child2(NULL)
    , m_pixelData(new MipLayer::Pixel[size.width() * size.height()])
    , m_isEnabled(false) // no data yet
{}

// MIPLayers can be combined in a binary tree
MipLayer::MipLayer(const MipLayer* child1, const MipLayer* child2, QObject *parent)
    : QObject(parent)
    , m_size(child1->size())
    , child1(child1)
    , child2(child2)
    , m_pixelData(new MipLayer::Pixel[m_size.width() * m_size.height()])
    , m_isEnabled(false) // no data yet
{
    updateData();
    connect(child1, SIGNAL(layerChanged()),
            this, SLOT(updateData()));
    // child2 can be NULL
    if (child2)
        connect(child2, SIGNAL(layerChanged()),
                this, SLOT(updateData()));
}

MipLayer::~MipLayer()
{
    if (m_pixelData) {
        delete [] m_pixelData;
        m_pixelData = NULL;
    }
}

QSize MipLayer::size() const {return m_size;}

// Remember to manually emit layerChanged() after you are done manually changing the image!
MipLayer::Pixel * MipLayer::scanLine(short y) {
    assert(y < m_size.height());
    return &(m_pixelData[y * m_size.width()]);
}

const MipLayer::Pixel * MipLayer::scanLine(short y) const {
    assert(y < m_size.height());
    return &(m_pixelData[y * m_size.width()]);
}

void MipLayer::enable(bool isEnabled) {
    if (isEnabled == m_isEnabled) return;
    m_isEnabled = isEnabled;
    // emit layerChanged();
}

void MipLayer::enableWithSignal(bool isEnabled) {
    if (isEnabled == m_isEnabled) return;
    m_isEnabled = isEnabled;
    emit layerChanged();
}

bool MipLayer::isEnabled() const {return m_isEnabled;}

void MipLayer::updateData() { // reexamine data from child images
    qDebug() << "updateData " << this;
    // Use an early "return()" statement if you don't want to emit layerChanged()
    // I don't know how to update without some child images
    if (! child1) return;
    assert(size() == child1->size());
    if (! child2) { // child2 can be NULL
        if (!child1->isEnabled()) {
            if (! isEnabled()) return; // no change (should not happen...)
            enable(false);
        }
        else {
            memcpy(m_pixelData, child1->m_pixelData, size().width() * size().height() * sizeof(MipLayer::Pixel) );
        }
        emit layerChanged();
        return;
    }
    else {
        assert(size() == child2->size());
    }
    // are both children disabled?
    if (  (! child1->isEnabled())
       && (! child2->isEnabled()) )
    {
        if (! isEnabled()) return; // no change (should not happen...)
        enable(false);
    }
    // Perhaps just one child is disabled, thus just copy the other one, *fast*
    else if (! child1->isEnabled()) {
        memcpy(m_pixelData, child2->m_pixelData, size().width() * size().height() * sizeof(MipLayer::Pixel) );
        enable(true);
    }
    else if (! child2->isEnabled()) {
        memcpy(m_pixelData, child1->m_pixelData, size().width() * size().height() * sizeof(MipLayer::Pixel) );
        enable(true);
    }
    else {
        // slower thoughtful combination of two child images
        int dimY = size().height();
        int dimX = size().width();
        for (int y = 0; y < dimY; ++y) {
            Pixel * sl = scanLine(y);
            const Pixel * sl1 = child1->scanLine(y);
            const Pixel * sl2 = child2->scanLine(y);
            for (int x = 0; x < dimX; ++x)
            {
                const MipLayer::Pixel& pixel1 = sl1[x];
                const MipLayer::Pixel& pixel2 = sl2[x];
                MipLayer::Pixel& pixel = sl[x];
                if (pixel1.intensity >= pixel2.intensity)
                    pixel = pixel1;
                else
                    pixel = pixel2;
            }
        }
        enable(true);
    }
    emit layerChanged();
}


