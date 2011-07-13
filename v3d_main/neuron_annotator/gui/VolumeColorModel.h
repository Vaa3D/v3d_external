#ifndef VOLUMECOLORMODEL_H
#define VOLUMECOLORMODEL_H

#include <QObject>
#include <QColor>

class VolumeColorModel : public QObject
{
    Q_OBJECT

public:
    typedef QRgb ColorType; // QRgb is a typedef for unsigned int #AARRGGBB


    class ScanLine
    {
    public:
        ScanLine(ColorType* data, size_t xdim)
            : m_Data(data), m_xDim(xdim)
        {}
        size_t xDim() const {return m_xDim;}
        size_t numPixels() const {return xDim();}
        ColorType& operator[](size_t x) {return m_Data[x];}
        ColorType operator[](size_t x) const {return m_Data[x];}
    private:
        ColorType * m_Data;
        size_t m_xDim;
    };


    class ZSlice
    {
    public:
        ZSlice(ColorType* data, size_t xdim, size_t ydim)
            : m_Data(data), m_xDim(xdim), m_yDim(ydim)
        {}
        size_t xDim() const {return m_xDim;}
        size_t yDim() const {return m_yDim;}
        size_t numPixels() const {return xDim() * yDim();}
        ScanLine scanLine(size_t y)
        {
            return ScanLine(m_Data + y * xDim(), xDim());
        }

    private:
        ColorType * m_Data;
        size_t m_xDim, m_yDim;
    };


    explicit VolumeColorModel(size_t xdim, size_t ydim, size_t zdim);
    explicit VolumeColorModel(const VolumeColorModel& rhs);
    virtual ~VolumeColorModel();

    size_t xDim() const {return m_xDim;}
    size_t yDim() const {return m_yDim;}
    size_t zDim() const {return m_zDim;}
    size_t numVoxels() const {return xDim() * yDim() * zDim();}
    ZSlice zSlice(size_t z)
    {
        return ZSlice(m_Data + z * xDim() * yDim(), xDim(), yDim());
    }
    VolumeColorModel& operator=(const VolumeColorModel& rhs);

    static void benchmarkDataCopy();
    static void benchmarkDataIncrement();
    static void benchmarkDataSum();
    static void benchmarkDataMip();
    static void benchmarkDataGamma();

signals:

public slots:

private:
    ColorType * m_Data;
    size_t m_xDim, m_yDim, m_zDim;
};

#endif // VOLUMECOLORMODEL_H
