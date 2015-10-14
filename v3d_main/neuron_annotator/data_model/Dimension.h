#ifndef JFRC_DIMENSION_H
#define JFRC_DIMENSION_H

#include <cstddef>  // size_t (might be in std namespace)
#include <cmath>  // std::pow
#include <cassert>

namespace jfrc {

/// X,Y,Z dimensions of a volume
class Dimension
{
public:
    enum Index {
        X = 0,
        Y = 1,
        Z = 2
    };

    Dimension();
    Dimension(size_t x, size_t y, size_t z);

    size_t numberOfVoxels() const {return x()*y()*z();}
    const size_t& operator[](size_t ix) const;
    size_t& operator[](size_t ix);
    const size_t& x() const;
    const size_t& y() const;
    const size_t& z() const;
    size_t& x();
    size_t& y();
    size_t& z();
    bool operator!=(const Dimension& rhs) const;
    bool operator==(const Dimension& rhs) const;
    /// Intermediate value used in sampledSizeMethod
    double computeLinearSubsampleScale(size_t memoryLimit) const;
    /// Compute a possibly smaller size that would fit in a particular GPU memory limit,
    /// assuming 32 bit pixels.
    Dimension sampledSize(size_t memoryLimit = 0) const;
    /// Expand dimensions, if necessary, to be a multiple of 8
    Dimension padToMultipleOf(unsigned int factor = 8);
    /// Compute next multiple of factor greater than or equal to coord
    static size_t padToMultipleOf(size_t coord, unsigned int factor);

protected:
    size_t data[3];
};

} // namespace jfrc

#endif
