#ifndef IMAGECOLORIZER_H
#define IMAGECOLORIZER_H

#include <QObject>
#include <QColor>
#include <QImage>

template<class ValueType>
class SimpleArray {
public:
    typedef ValueType value_type;
    typedef unsigned int size_type;
    typedef unsigned int index_type;
    typedef ValueType* iterator;
    typedef ValueType const * const_iterator;

    SimpleArray(size_type s)
        : m_size(s)
        , m_data(new ValueType[size])
    {}
    virtual ~SimpleArray() {delete m_data; m_data = NULL;}

    // Allow implicit conversion to pointer
    // This resolves index operator too.
    operator ValueType*() {return m_data;}
    operator const ValueType*() const {return m_data;}

    size_type size() const {return m_size;}

    iterator begin() {return &m_data[0];}
    const_iterator begin() const {return &m_data[0];}
    iterator end() {return &m_data[m_size];}
    const_iterator end() const {return &m_data[m_size];}

private:
    size_type m_size;
    ValueType * m_data;
};

template<class ValueType>
class MipDataPixel : public SimpleArray<ValueType> {};

template<class ValueType>
class MipDataColumn : public SimpleArray<MipDataPixel<ValueType> > {};

template<class ValueType>
class MipDataImage : public SimpleArray<MipDataColumn<ValueType> > {};


template<class ValueType>
class ImageColorizer : public QObject
{
public:
    explicit ImageColorizer(unsigned int numChannels, QObject *parent = 0)
        : channelColor(numChannels)
    {
        // Default color tables
        assert(numChannels > 0);
        // If there is just one color, make it white
        if (numChannels == 1) {
            channelColor[0] = Qt::white;
        }
        // First 3 channels default to red, green, blue; the rest are white
        else {
            for (int i = 0; i < numChannels; ++i) {
                if (i == 0) channelColor[i] = Qt::red;
                else if (i == 1) channelColor[i] = Qt::green;
                else if (i == 2) channelColor[i] = Qt::blue;
                else channelColor[i] = Qt::white;
            }
        }
    }

    void setChannelColor(unsigned int channelIndex, QRgb color)
    {
        if (color == channelColor[channelIndex]) return;
        channelColor[channelIndex] = color;
    }

    QRgb getCorrectedColor(ValueType value, unsigned int channelIndex) const;
    QRgb getCorrectedColor(ValueType* pixel) const;

    // TODO - inline conversion of <some 16-bit image type> to QImage
    void updateColors2D(const MipDataImage<ValueType>& input, QImage& output) const
    {
        const unsigned int dimX = output.width();
        const unsigned int dimY = output.height();
        const unsigned int dimC = input.numChannels();
        assert(dimC == channelColor.size());
        assert(dimX == input.numColumns());
        assert(dimY == input.numRows());
        for (int x = 0; x < dimX; ++x)
        {
            const MipDataColumn<ValueType>& column = input[x];
            for (int y = 0; y < dimY; ++y)
            {
                const MipDataPixel<ValueType>& pixel = column[y];
                short int red, green, blue;
                red = green = blue = 0;
                // Add the color contribution from each data channel
                for (unsigned int c = 0; c < dimC; ++c) {
                    QRgb col = channelColor[c];
                    float intensity = getCorrectedIntensity(pixel[c]);
                    red += qRed(col) * intensity;
                    green += qGreen(col) * intensity;
                    blue += qBlue(col) * intensity;
                }
                // Truncate to 8-bits
                red   = red   > 255 ? 255 : red;
                green = green > 255 ? 255 : green;
                blue  = blue  > 255 ? 255 : blue;
                output.setPixel(x, y, qRgb(red, green, blue));
            }
        }
    }

protected:
    std::vector<QRgb> channelColor; // each data channel can be a different color
};

#endif // IMAGECOLORIZER_H
