/*
 * ImageLoaderBasic.h
 *
 *  Created on: Jan 30, 2013
 *      Author: Christopher M. Bruns
 */

#ifndef IMAGELOADERBASIC_H_
#define IMAGELOADERBASIC_H_

#include "../../basic_c_fun/basic_4dimage.h"
#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>

static const short int PBD_3_BIT_DTYPE = 33;

// Abstraction for possibly reading from sources that are not files.
class DataStream
{
public:
    virtual size_t read(void* dst, size_t numBytes) = 0;
};


// Simple implementation of ImageLoader that uses no Qt.
class ImageLoaderBasic
{
public:
    ImageLoaderBasic();
    virtual ~ImageLoaderBasic();

    static std::string getFileExtension(const std::string& filePath)
    {
        size_t dot = filePath.find_last_of(".");
        if (dot == std::string::npos)
            return std::string(""); // no extension found
        dot++; // remove the actual dot character
        // extract the file type extension
        std::string extension = filePath.substr(dot, filePath.size() - dot);
        // convert to lower case
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return extension;
    }

    static bool hasPbdExtension(const char* filename)
    {
        std::string extension = getFileExtension(std::string(filename));
        if (extension == "pbd")
            return true;
        if (extension == "v3dpbd")
            return true;
        if (extension == "vaa3pbd")
            return true;
        return false;
    }

    virtual bool loadImage(Image4DSimple * stackp, const char* filepath);
    int saveStack2RawPBD(const char * filename, ImagePixelType dataType, unsigned char* data, const V3DLONG * sz);
    virtual int loadRaw2StackPBD(DataStream& fileStream, V3DLONG fileSize, Image4DSimple * image, bool useThreading);
    virtual int loadRaw2StackPBD(const char * filename, Image4DSimple * image, bool useThreading);
    V3DLONG decompressPBD8(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength);
    V3DLONG decompressPBD16(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength);
    bool isCanceled() const {return bIsCanceled;}

protected:
    V3DLONG compressPBD3(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft);
    V3DLONG compressPBD8(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft);
    V3DLONG compressPBD16(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft);
    void updateCompressionBuffer8(unsigned char * updatedCompressionBuffer);
    void updateCompressionBuffer16(unsigned char * updatedCompressionBuffer);
    virtual int exitWithError(std::string errorMessage);
    virtual int exitWithError(const char* errorMessage){
        // avoid stack overflow from infinite virtual recursion by specifying this class method
        return ImageLoaderBasic::exitWithError(std::string(errorMessage));
    }

    V3DLONG pbd3FindRepeatCountFromCurrentPosition(unsigned char* sourceBuffer, V3DLONG position, V3DLONG maxPosition);
    void pbd3EncodeRepeat(unsigned char sourceValue, V3DLONG count, unsigned char* keyByte, unsigned char* valueByte);
    void pbd3EncodeLongRepeat(unsigned char sourceValue, V3DLONG count, unsigned char* returnBuffer);
    V3DLONG pbd3FindDiffCountFromCurrentPosition(unsigned char* sourceBuffer, V3DLONG position, V3DLONG maxPosition);
    void pbd3FindFirstRepeatOfMinLength(unsigned char* sourceBuffer, V3DLONG position, V3DLONG searchLength, V3DLONG minLength, V3DLONG* returnBuffer);
    unsigned char* pbd3EncodeDiff(unsigned char* sourceBuffer, V3DLONG position, V3DLONG length, int* byteCount);
    void pbd3FlushLiteral(unsigned char* compressionBuffer, unsigned char* sourceBuffer, V3DLONG* activeLiteralIndex, V3DLONG* p, V3DLONG i);
    void updateCompressionBuffer3(unsigned char * updatedCompressionBuffer);
    V3DLONG decompressPBD3(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength);
    int pbd3GetRepeatCountFromBytes(unsigned char keyByte, unsigned char valueByte, unsigned char* repeatValue);

    volatile bool bIsCanceled;
    FILE * fid;
    V3DLONG totalReadBytes;
    V3DLONG maxDecompressionSize;
    V3DLONG channelLength;
    std::vector<unsigned char> compressionBuffer;
    unsigned char * decompressionBuffer;
    unsigned char * compressionPosition;
    unsigned char * decompressionPosition;
    int decompressionPrior;
    std::vector<char> keyread;
    int loadDatatype;

    unsigned char* pbd3_source_min;
    unsigned char* pbd3_source_max;
    int pbd3_current_channel;
    unsigned char pbd3_current_min;
    unsigned char pbd3_current_max;
    V3DLONG pbd_sz[4];
};


// Implementation of DataStream for C file handles
class FileStarStream : public DataStream
{
public:
    FileStarStream(FILE* fid) : fid(fid) {}
    virtual size_t read(void* dst, size_t numBytes) {
        return fread(dst, 1, numBytes, fid);
    }

protected:
    FILE* fid;
};


#endif /* IMAGELOADERBASIC_H_ */
