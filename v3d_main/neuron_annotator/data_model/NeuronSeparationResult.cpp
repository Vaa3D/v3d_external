#include "NeuronSeparationResult.h"
#include <sstream>
#include <stdexcept>

using namespace std;

namespace jfrc {


std::istream& NeuronSeparationResult::loadChk4File(std::istream& is, bool debug)
{
    // Read separation parameters
    is.read((char*)&c, sizeof(double)); // "c" is the intensity z-score for seeding neuron fragments
    is.read((char*)&e, sizeof(double)); // "e" is the intensity z-score for growing neuron fragments
    is.read((char*)&s, sizeof(int)); // "s" is the minimum voxel size for a neuron fragment
    if (debug) {
        cout << "Parameters: c = " << c << "; e = " << e << "; s = " << s << endl;
    }
    // Read input image file names, and which channel (reference) was eliminated from each.
    int numberOfInputImages;
    is.read((char*)&numberOfInputImages, sizeof(int));
    for (int i = 0; i < numberOfInputImages; ++i) {
        int stringLength;
        is.read((char*)&stringLength, sizeof(int));
        std::vector<char> temp_string(stringLength + 1, '\0');
        is.read(&temp_string[0], stringLength * sizeof(char));
        inputImageFileNames.push_back(std::string(&temp_string[0]));
        if (debug) cout << "Input: " << inputImageFileNames.back() << endl;
        int refChannelIndex;
        is.read((char*)&refChannelIndex, sizeof(int));
        if (debug) cout << "(channel " << refChannelIndex+1 << " eliminated)" << endl;
    }
    // Read channel data
    int numberOfChannels;
    is.read((char*)&numberOfChannels, sizeof(int));
    if (debug) cout << numberOfChannels << " channels" << endl;
    // Allocate array
    for (int c = 0; c < numberOfChannels; ++c)
    {
        MyLibArrayImposter array;
        array.loadFile(is, debug);
        channelArrays.push_back(array);
    }
    return is;
}


std::ostream& NeuronSeparationResult::saveChk4File(std::ostream& os)
{
    os.write((const char*)&c, sizeof(double));
    os.write((const char*)&e, sizeof(double));
    os.write((const char*)&s, sizeof(int));
    int numberOfInputImages = (int)inputImageFileNames.size();
    os.write((const char*)&numberOfInputImages, sizeof(int));
    for (int i = 0; i < numberOfInputImages; ++i) {
        int nameLength = (int)inputImageFileNames[i].size();
        os.write((const char*)&nameLength, sizeof(int));
        os.write(inputImageFileNames[0].c_str(), nameLength);
        // TODO - eliminated channel id
    }
    // TODO - rest of data
	return os;
}


////////////////////////////////
// MyLibArrayImposter members //
////////////////////////////////

/* static */
const int MyLibArrayImposter::Value_Type_Size[10] = { 1, 2, 4, 8, 1, 2, 4, 8, 4, 8 };

istream& MyLibArrayImposter::loadFile(istream& is, bool debug)
{
    // Verify "Array" token in file at head of array
    char arrayToken[6];
    arrayToken[5] = '\0';
    is.read(arrayToken, 5);
    if (debug) cout << arrayToken << endl;
    if (string("Array") != arrayToken) {
        ostringstream msg("Expected 'Array' in file read, found '");
        msg << arrayToken << "'";
        throw runtime_error(msg.str().c_str());
    }
    // MyLib writes Array struct memory directly into file.
    // So read it that way; each element in order
    is.read((char*)&kind, sizeof(Array_Kind));
    is.read((char*)&type, sizeof(Value_Type));
    is.read((char*)&scale, sizeof(int));
    is.read((char*)&ndims, sizeof(int));
    is.read((char*)&size, sizeof(Size_Type)); // correct
    // Next element is a pointer.  Don't want to save that...
    Dimn_Type* dims_ptr;
    is.read((char*)&dims_ptr, sizeof(Dimn_Type*)); // discard
    dims.clear();
    is.read((char*)&tlen, sizeof(int)); // length of "text" string
    // discard char* text pointer
    char* text_ptr = NULL;
    is.read((char*)&text_ptr, sizeof(char*));
    // discard void* data pointer
    void* data_ptr = NULL;
    is.read((char*)&data_ptr, sizeof(void*)); // discard
    data.reset((char *)NULL);
    // Read size in each spatial dimension
    // TODO - I somehow missed 4 more bytes that need to be consumed.
    // 32 bit int?
    char foo[100];
    is.read(foo, 4);
    dims.assign(ndims, 0);
    is.read((char*)&dims[0], ndims * sizeof(Dimn_Type));
    if (debug) {
        cout << "Kind = " << kind << endl;
        cout << "Type = " << type << endl;
        cout << "Scale = " << scale << endl;
        cout << "ndims = " << ndims << endl;
        cout << "size = " << size << endl;
        cout << "dims ptr = " << dims_ptr << endl;
        cout << "tlen = " << tlen << endl;
        cout << "text = '" << text << "'" << endl;
        cout << "data ptr = " << data_ptr << endl;
        for (int i = 0; i < ndims; ++i)
            cout << "dimension " << i << " size = " << dims[i] << endl;
    }
    // Load array data
    data.reset(new char[data_size()]);
    is.read(data.get(), data_size());
    // Load text data
    std::vector<char> temp_text(tlen + 2, '\0');
    is.read(&temp_text[0], tlen + 1);
    text = &temp_text[0]; // convert to std::string
    if(debug) cout << "text = '" << text << "'" << endl;

    return is;
}


} // namespace jfrc
