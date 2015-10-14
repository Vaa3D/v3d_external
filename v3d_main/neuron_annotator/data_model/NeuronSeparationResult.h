#ifndef NEURONSEPARATIONRESULT_H
#define NEURONSEPARATIONRESULT_H

#include <boost/shared_array.hpp>
#include <vector>
#include <iostream>

extern "C" {
#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
}

namespace jfrc {

/// Contiguous run of voxels in a volume, used to compactly represent part of a neuron fragment
struct VoxelRun
{
    uint64_t start;
    uint64_t end;
};


/// Subset of voxels in a volume identified as a neuron fragment
struct NeuronFragment
{
    uint64_t numberOfVoxels; /// Size of this neuron fragment in voxels
    std::vector<VoxelRun> voxels; /// Voxel comprising this neuron fragment expressed as a series of contiguous runs.
    int clusterIndex; /// Clusters are spatially contiguous sets of neuron fragments, not all necessarily the same color.
    float hue; /// Generalized hue of this neuron fragment from multichannel volume data.
};


/// Class the mimics some aspects of Array class in Gene Myers' mylib
/// but in C++
/// without global variables
/// with more conventional reference-counted memory management using boost::shared_array
struct MyLibArrayImposter
{
    enum Array_Kind {
        PLAIN_KIND    = 0,  //    The outermost dimension encodes each channel (R, G, B, or A)
        RGB_KIND      = 1,  //  Others may encode COMPLEX data (see below):
        RGBA_KIND     = 2,  //    The innermost dimension encodes the real and imaginary parts.
        COMPLEX_KIND  = 3
    };

    enum Value_Type {
        UINT8_TYPE   = 0,
        UINT16_TYPE  = 1,
        UINT32_TYPE  = 2,
        UINT64_TYPE  = 3,
        INT8_TYPE    = 4,
        INT16_TYPE   = 5,
        INT32_TYPE   = 6,
        INT64_TYPE   = 7,
        FLOAT32_TYPE = 8,
        FLOAT64_TYPE = 9
    };

    static const int Value_Type_Size[10];

    typedef signed long long Size_Type;
    typedef unsigned int Dimn_Type;

public:
    MyLibArrayImposter() : data((char *)NULL) {}
    std::istream& loadFile(std::istream& is, bool debug = false);
    int64_t data_size() const {return size * Value_Type_Size[type];}

protected:
    Array_Kind  kind;    //  Interpreation of the array: one of the four enum constants above
    Value_Type  type;    //  Type of values, one of the eight enum constants above
    int         scale;   //  # of bits in integer values
    int         ndims;   //  Number of dimensions of the array
    Size_Type   size;    //  Total number of elements in the array (= PROD_i dims[i])
    std::vector<Dimn_Type> dims;    //  dims[i] = length of dimension i
    int         tlen;    //  Length of the text string text
    std::string text;    //  An arbitrary string label
    // Use char as closest thing to void* in modern C++
    // Reference counted shared_array causes shallow copies.
    boost::shared_array<char> data;
    // void       *data;    //  A block of size sizeof(type) * size bytes holding the array.
};


/// Collection of neuron fragments in a volume
struct NeuronSeparationResult
{
    std::ostream& saveChk4File(std::ostream& os);
    std::istream& loadChk4File(std::istream& is, bool debug = false);

    std::vector<NeuronFragment> fragments;
    std::vector<std::string> inputImageFileNames;
    std::vector<MyLibArrayImposter> channelArrays;
    double c;
    double e;
    int s;
};


} // namespace jfrc

#endif // NEURONSEPARATIONRESULT_H
