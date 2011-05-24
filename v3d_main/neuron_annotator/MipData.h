#ifndef NA_MIP_DATA_H
#define NA_MIP_DATA_H

#include <QObject>
#include <vector>
#include "../v3d/v3d_core.h"

class MipPixel : public std::vector<float>
{
public:
    MipPixel(size_t nChannels = 1)
        : std::vector<float>(nChannels, 0.0f)
        , z(-1), intensity(0.0f), neuronIndex(-1)
    {}

    int z;
    float intensity;
    int neuronIndex;
};

typedef std::vector<MipPixel> MipColumn;

// MipData is a generic container for float representation of original MIP data.
// MipDisplayImage can contain a binary tree of MipDisplayImages,
// for efficient update when toggling different components (e.g. neurons).
class MipData : public QObject
{
    Q_OBJECT

public:
    explicit MipData(QObject * parent = NULL) : QObject(parent) {}
    // Initialize from either a 3D data set...
    explicit MipData(const My4DImage* img, const My4DImage* maskImg = NULL, QObject * parent = NULL);
    // ...or a pair of intermediate images in a binary tree
    // explicit MipData(const MipData* child1, const MipData* child2, QObject * parent = NULL);

    const MipColumn& operator[](int index) const {
        return data[index];
    }
    MipColumn& operator[](int index) {
        return data[index];
    }

    bool loadMy4DImage(const My4DImage* img, const My4DImage* maskImg = NULL);

    int nColumns() const;
    int nRows() const;
    int nChannels() const;

    float dataMin; // actual minimum value
    float dataMax; // actual maximum value

signals:
    void processedXColumn(int);
    void intensitiesUpdated();
    void neuronMasksUpdated();

public slots:
    void updateData() {}

private:
    std::vector<MipColumn> data; // Image pixels
    bool bToggledOn; // Whether this MIP image is on or off (within binary tree, say)
};

#endif // NA_MIP_DATA_H
