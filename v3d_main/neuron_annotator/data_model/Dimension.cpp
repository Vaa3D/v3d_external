#include "Dimension.h"

namespace jfrc {


////////////////////////
// Dimension methods //
///////////////////////

Dimension::Dimension()
{
    data[0] = 0; data[1] = 0; data[2] = 0;
}

Dimension::Dimension(size_t x, size_t y, size_t z)
{
    data[0] = x; data[1] = y; data[2] = z;
}

const size_t& Dimension::operator[](size_t ix) const
{return data[ix];}

size_t& Dimension::operator[](size_t ix)
{return data[ix];}

const size_t& Dimension::x() const
{return data[X];}

const size_t& Dimension::y() const
{return data[Y];}

const size_t& Dimension::z() const
{return data[Z];}

size_t& Dimension::x()
{return data[X];}

size_t& Dimension::y()
{return data[Y];}

size_t& Dimension::z()
{return data[Z];}

bool Dimension::operator!=(const Dimension& rhs) const {
    const Dimension& lhs = *this;
    for (int i = 0; i < 3; ++i)
        if (lhs[i] != rhs[i]) return true;
    return false;
}

bool Dimension::operator==(const Dimension& rhs) const {
    const Dimension& lhs = *this;
    return ! (lhs != rhs);
}

/// Intermediate value used in sampledSizeMethod
double Dimension::computeLinearSubsampleScale(size_t memoryLimit) const
{
    if (memoryLimit <= 0) return 1.0; // zero means no limit
    size_t memoryUse = x() * y() * z() * 6; // 6 bytes per voxel
    if (memoryUse <= memoryLimit) return 1.0; // already fits into memory as-is
    double sampleFactor = std::pow((double)memoryLimit / (double)memoryUse, 1.0/3.0);
    return sampleFactor;
}

/// Compute a possibly smaller size that would fit in a particular GPU memory limit,
/// assuming 32 bit pixels.
Dimension Dimension::sampledSize(size_t memoryLimit) const
{
    double sampleFactor = computeLinearSubsampleScale(memoryLimit);
    return Dimension((int)(x() * sampleFactor), // scale all dimensions
                     (int)(y() * sampleFactor),
                     (int)(z() * sampleFactor));
}

/// Expand dimensions, if necessary, to be a multiple of 8
Dimension Dimension::padToMultipleOf(unsigned int factor)
{
    return Dimension(padToMultipleOf(x(), factor),
                     padToMultipleOf(y(), factor),
                     padToMultipleOf(z(), factor));
}

/// Compute next multiple of factor greater than or equal to coord
/* static */
size_t Dimension::padToMultipleOf(size_t coord, unsigned int factor)
{
    const int remainder = coord % factor;
    if (0 == remainder) return coord;
    size_t result = coord + factor - remainder;
    assert(0 == (result % factor));
    return result;
}


} // namespace jfrc

