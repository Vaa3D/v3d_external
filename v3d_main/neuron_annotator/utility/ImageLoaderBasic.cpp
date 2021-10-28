/*
 * Imageloaderbasic.cpp
 *
 *  Created on: Jan 30, 2013
 *      Author: Christopher M. Bruns
 */

#include "ImageLoaderBasic.h"
#include "../../basic_c_fun/stackutil.h"
#include <iostream>
#include <sstream>
#include <string>

using std::cerr;
using std::endl;
using std::string;
using std::stringstream;

static bool DEBUG = false;
static bool DEBUG_SUMMARY = false;

static const unsigned char oooooool = 1;
static const unsigned char oooooolo = 2;
static const unsigned char ooooooll = 3;
static const unsigned char oooooloo = 4;
static const unsigned char ooooolol = 5;
static const unsigned char ooooollo = 6;
static const unsigned char ooooolll = 7;

static const unsigned char ooolllll = 63;
static const unsigned char olllllll = 127;
static const unsigned char looooooo = 128;
static const unsigned char lloooooo = 192;
static const unsigned char lllooooo = 224;

static const V3DLONG   PBD_3_REPEAT_MAX = 4096;
static const double    PBD_3_LITERAL_MAXEFF = 2.66;
static const double    PBD_3_DIFF_MAXEFF = 5.2;
static const V3DLONG   PBD_3_DIFF_REPEAT_THRESHOLD = 14;
static const V3DLONG   PBD_3_MIN_DIFF_LENGTH = 6;
static const V3DLONG   PBD_3_MAX_DIFF_LENGTH = 210;
static const int       PBD_3_REPEAT_STARTING_POSITION = 0;
static const int       PBD_3_REPEAT_COUNT = 1;
static const V3DLONG   PBD_3_MAX_LITERAL = 24;


ImageLoaderBasic::ImageLoaderBasic()
    : bIsCanceled(false)
    , fid(0)
    , compressionPosition(0)
    , decompressionPosition(0)
    , decompressionPrior(0)
{
}


ImageLoaderBasic::~ImageLoaderBasic()
{
}


/* virtual */
bool ImageLoaderBasic::loadImage(Image4DSimple * stackp, const char* filepath)
{
    // cerr << "loadImageBasic stack string " << filepath << __FILE__ << __LINE__ << endl;
    bool bSucceeded = false;
    std::string extension = getFileExtension(std::string(filepath));
    if (extension == "lsm") {
        // Vaa3d is not const-correct...
        stackp->loadImage(const_cast<char*>(filepath), true);
        bSucceeded = true;
    } else if ((extension == "tif") ||
            (extension == "v3draw") ||
            (extension == "raw")) {
        stackp->loadImage(const_cast<char*>(filepath));
        bSucceeded = true;
    } else if (hasPbdExtension(filepath)) {
        // Basic ImageLoader does not use threading, to avoid Qt linkage
        if (loadRaw2StackPBD(filepath, stackp, false) == 0)
            bSucceeded = true;
        else
            cerr << "Error with loadRaw2StackPBD";
    }
    return bSucceeded;
}


/*

 This function implements a hybrid of PackBits with difference-encoding, as follows:

 ==========================================================================================================
 8-bit

 <header byte>

 0 to 32   : Implies the following (n+1) bytes are literal
 33 to 127 : Implies the prior byte is to be followed by (n-32) 2-bit (4-value) differences, to be applied accumulatively.
             Any extra space to the remaining byte boundary is to be ignored. The 4 values are interpreted as as a progessive
             4D array incrementing the following values in order { 0, 1, 2, -1 }, such that,

             Value   Difference Sequence

             0    =  0,  0,  0,  0
             1    =  0,  0,  0,  1
             2    =  0,  0,  0,  2
             3    =  0,  0,  0, -1
             4    =  0,  0,  1,  0
             5    =  0,  0,  1,  1
             6    =  0,  0,  1,  2
             7    =  0,  0,  1, -1
             8    =  0,  0,  2,  0
            ...
            123   =  1, -1,  2, -1
            124   =  1, -1, -1,  0
            125   =  1, -1, -1,  1
            126   =  1, -1, -1,  2
            127   =  1, -1, -1, -1
            128   =  2,  0,  0,  0
            129   =  2,  0,  0,  1
            130   =  2,  0,  0,  2
            131   =  2,  0,  0, -1
            132   =  2,  0,  1,  0
            133   =  2,  0,  1,  1
            ...
            188   =  2, -1, -1,  0
            189   =  2, -1, -1,  1
            190   =  2, -1, -1,  2
            191   =  2, -1, -1, -1
            192   = -1,  0,  0,  0
            193   = -1,  0,  0,  1
            194   = -1,  0,  0,  2
            195   = -1,  0,  0, -1
            ...
            249   = -1, -1,  2,  1
            250   = -1, -1,  2,  2
            251   = -1, -1,  2, -1
            252   = -1, -1, -1,  0
            253   = -1, -1, -1,  1
            254   = -1, -1, -1,  2
            255   = -1, -1, -1, -1

128 to 255 : Implies the following single byte is to be repeated (n-127) times

 ==========================================================================================================
 16-bit

 <header byte>

 0 to 31   : Implies the following (n+1) positions are literal, which requires 2*(n+1) following bytes.

 32 to 79 : Implies the prior 2-byte value is to be followed by (n-31) 3-bit (8-value) differences, to be applied accumutively.
            Any extra space within a 1-byte boundary is to be ignored. The difference is encoded thus:

             3-bit

             0 - 4 (literal)
             5-7 (4-n)

 80 to 182 : Implies the prior 2-byte value is to be followed by (n-79) 4-bit (16-value) differences, to be applied accumutively.
             Any extra space within a 1-byte boundary is to be ignored. The difference is encoded thus:

             4-bit

             0 - 8 (literal)
             9-15 (8-n)


 183 to 222 : Implies the prior 2-byte value is to be followed by (n-182) 5-bit (32-value) differences, applied accumulatively.
              Any extra space within a 1-byte boundary is ignored. The difference is encoded as:

              5-bit

              0-16 (literal)
              17-31 (16-n)


223 to 255 : Implies the following two-byte value is to be repeated (n-222) times

====================================================================================================
3-bit lossy representation of 8-bit

In this case, 3-bits are used to represent the range between min/max of the 8-bit values in the pre-compressed 8-bit file.
<low byte>
Specifies the 8-bit value to use as the lower bound.
<high byte>
Specifies the 8-bit value to use as the upper bound.
<header byte>

A - Literal) 0 to 23 : Implies the following n+1 3-bit positions are literal, requiring ceil((n+1)*3/8) following bytes. The maximum literal efficiency is 2.66 positions/byte.

B - Difference) 24 to 127 : Implies the prior 3-bit value is to be followed by (n-22) difference-encoded doublet positions, for a maximum 127-22 = 105 3-bit-encoded accumulative two-step changes, to be interpreted as:

000 =  0,  0
001 =  0, -1
010 = -1,  0
011 = -1,  1
100 =  0,  1
101 =  1,  0
110 =  1, -1
111 =  1,  1

The maximum difference-encoding efficiency is (135*2)/52 = 5.2 positions/byte. NOTE: a sequence of { -1, -1 } can occur, as long as it is split by a 3-bit encoding boundary.

C - Repeat) 128 to 255 : The following byte should be divided into two sections, the first 3-bits, and then the following 5-bits.

The key byte should be interpreted as a 7-bit number by subtracting 128, giving a range of 0-127. The 5-bits in the following byte are added to
these 7 bits to give 12-bits of repeat spec, or 4096 max. The actual repeat value is this 12-bit value +1 (because a repeat of 0 doesn't make sense).

==============================

Cost Matrix:

#     Literal    Diff     Repeat

1       2        -        2
2       2        2        2
3       3        2        2
4       3        2        2
5       3        3        2
6       4        3        2
7       4        3        2
8       4        3        2
9       5        3        2
10      5        3        2
11      6        4        2
12      6        4        2

*/

int ImageLoaderBasic::saveStack2RawPBD(const char * filename, ImagePixelType datatype, unsigned char* data, const V3DLONG * sz)
{
    int berror=0;

        /* This function save a data stack to raw file */
                printf("size of [V3DLONG]=[%ld], [V3DLONG]=[%ld] [int]=[%ld], [short int]=[%ld], [double]=[%ld], [float]=[%ld]\n",
                       sizeof(V3DLONG), sizeof(V3DLONG), sizeof(int), sizeof(short int), sizeof(double), sizeof(float));
        V3DLONG i;

        fid = fopen(filename, "wb");
        if (!fid)
        {
            return exitWithError("Fail to open file for writing");
        }

        /* Write header */
                         // raw_image_stack_by_hpeng
        char formatkey[] = "v3d_volume_pkbitdf_encod";
        int lenkey = strlen(formatkey);

        V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
        if (nwrite!=lenkey)
        {
            return exitWithError("File write error");
        }

        char endianCodeMachine = checkMachineEndian();
        if (endianCodeMachine!='B' && endianCodeMachine!='L')
        {
                return exitWithError("This program only supports big- or little- endian but not other format. Cannot save data on this machine.");
        }

        nwrite = fwrite(&endianCodeMachine, 1, 1, fid);
        if (nwrite!=1)
        {
                return exitWithError("Error happened in file writing.");
        }

        //int b_swap = (endianCodeMachine==endianCodeData)?0:1;
        //int b_swap = 0; //for this machine itself, should not swap data.

        short int dcode = (short int)datatype;
	if (datatype==V3D_UNKNOWN) {
	  // This is a signal to use PBD_3_BIT_DTYPE
	  dcode=PBD_3_BIT_DTYPE;
	  datatype=V3D_UINT8;
	}
        if (!(dcode==1 || dcode==2 || dcode==PBD_3_BIT_DTYPE))
        {
            stringstream msg;
            msg << "Unrecognized data type code = [";
            msg << dcode;
            msg << "]. This code is not supported in this version.";
            return exitWithError(msg.str());
        }

        //if (b_swap) swap2bytes((void *)&dcode);
        nwrite=fwrite(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
        if (nwrite!=1)
        {
                return exitWithError("Writing file error.");
        }

        V3DLONG unitSize = datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */

        //short int mysz[4];
        BIT32_UNIT mysz[4];//060806
                                           //if (b_swap)  {
                                           //for (i=0;i<4;i++) mysz[i] = (short int) sz[i];
                for (i=0;i<4;i++) {
                    mysz[i] = (BIT32_UNIT) sz[i];
                    cerr << " size " << i << " = " << mysz[i] << endl;
                }
                //swap2bytes((void *)(mysz+i));
                //}
                nwrite = fwrite(mysz, 4, 4, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
        if (nwrite!=4)
        {
                return exitWithError("Writing file error.");
        }

        V3DLONG totalUnit = 1;
        for (i=0;i<4;i++)
        {
                totalUnit *= sz[i];
		pbd_sz[i]=sz[i];
		cerr << "Set pbd_sz " << i << " to " << pbd_sz[i] << "\n";
        }

	channelLength = pbd_sz[0] * pbd_sz[1] * pbd_sz[2];

        cerr << "Using totalUnit=" << totalUnit << " unitSize=" << unitSize << endl;

        V3DLONG maxSize = totalUnit*unitSize*2;                             // NOTE:
        // unsigned char * compressionBuffer = new unsigned char [maxSize]; // we give the compression buffer 2x room without throwing an error,
                                                                            // even though we hope it peforms well below 1, obviously
        std::vector<unsigned char> compressionBuffer(maxSize);

        printf("Allocated compression target with maxSize=%ld\n", maxSize);

        V3DLONG compressionSize = 0;

        if (dcode==1) {
	  compressionSize=compressPBD8(&compressionBuffer[0], data, totalUnit*unitSize, maxSize);
        } else if (dcode==2) {
	  compressionSize=compressPBD16(&compressionBuffer[0], data, totalUnit*unitSize, maxSize);
        } else if (dcode==PBD_3_BIT_DTYPE) {
	  compressionSize=compressPBD3(&compressionBuffer[0], data, totalUnit*unitSize, maxSize);
	}

        if (compressionSize==0) {
            return exitWithError("Error during compressPBD");
        }

        double finalCompressionRatio = (totalUnit*unitSize*1.0)/compressionSize;

        V3DLONG originalSize=totalUnit*unitSize;

        printf("Total original size=%ld  post-compression size=%ld  ratio=%f\n", originalSize, compressionSize, finalCompressionRatio);

        printf("Writing file...");

        nwrite = fwrite(&compressionBuffer[0], 1, compressionSize, fid);
        if (nwrite!=compressionSize)
        {
            stringstream msg;
            msg << "Something wrong in file writing. The program wrote ";
            msg << nwrite;
            msg << "data points but the file says there should be ";
            msg << totalUnit;
            msg << "data points.";
            return exitWithError(msg.str());
        }

        /* clean and return */
        fclose(fid);
        fid = 0;
        printf("done.\n");
        return berror;
}

V3DLONG ImageLoaderBasic::compressPBD8(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft) {
    // bool debug=false;
    V3DLONG p=0;

    if (sourceBufferLength==0) {
        printf("ImageLoaderBasic::compressPBD8 - unexpectedly received buffer of zero size\n");
        return 0;
    }

    unsigned char currentValue=0;
    int dbuffer[95];
    V3DLONG activeLiteralIndex=-1; // if -1 this means there is no literal mode started
    for (V3DLONG i=0;i<sourceBufferLength;i++) {

        if (p>=spaceLeft) {
            printf("ImageLoaderBasic::compressPBD8 ran out of space p=%ld\n", p);
            return 0;
        }

        // From this point we assume the result has been accumulating in compressionBuffer, and at this moment
        // we are searching for the best approach for the next segment. First, we will try reading
        // the next value, and testing what the runlength encoding efficiency would be. The
        // efficiency is simply the (number of bytes encoded / actual bytes). Next, we will test
        // the different encoding, and (depending on its reach), see what its efficiency is.
        // The minimum number of unencoded bytes to use run-length encoding for is 3 (code, value).
        // The minimum number of unencoded bytes to use difference encoding is 3 also (code, value).

        int reTest=1;
        currentValue=sourceBuffer[i];
        V3DLONG currentPosition=i+1;
        while(currentPosition<sourceBufferLength && reTest<128) { // 128 is the max number of repeats supported
            if (sourceBuffer[currentPosition++]==currentValue) {
                reTest++;
            } else {
                break;
            }
        }
        double reEfficiency = reTest*1.0 / 2.0; // 2-bytes are used for the encoding

        if (reEfficiency>=4.0) { // 4.0 is the max efficiency from the difference encoding
            // Then use RE
            compressionBuffer[p++]=reTest+127; // The code for number of repeats
            compressionBuffer[p++]=currentValue; // The repeated value
            i+=(reTest-1); // because will increment one more time at top of loop
            activeLiteralIndex=-1;
        } else {
            double dfEfficiency=0.0;
            V3DLONG c=i;
            if (i>0) { // I.e., if not first since we can't start with a difference encoding
                // We need to evaluate difference encoding starting with the prior value
                unsigned int priorValue=sourceBuffer[i-1];
                V3DLONG unitsToCheck=sourceBufferLength-i;
                if (unitsToCheck>95) {
                    unitsToCheck=95; // 95 is max supported number of differences
                }
                for (int j=0;j<95;j++) {
                    dbuffer[j]=0; // clear the difference buffer
                }
                for (;c<i+unitsToCheck;c++) {
                    int d=sourceBuffer[c] - priorValue;
                    if (d>2 || d<-1) {
                        break;
                    }
                    priorValue=sourceBuffer[c]; // since by definition the diff encoding is cumulative
                    if (d==-1) {
                        d=3; // we use this to represent -1 in the dbuffer for key lookup later
                    }
                    dbuffer[c-i]=d;
                }
                dfEfficiency = ((c-i)*1.0)/(((c-i)/4)+2); // The denominator includes the coding byte and the encoding bytes
            }
            // Now we can decide between RE and DF based on efficiency
            if (reEfficiency>dfEfficiency && reEfficiency>1.0) {
                // Then use RE
                compressionBuffer[p++]=reTest+127; // The code for number of repeats
                compressionBuffer[p++]=currentValue; // The repeated value
                i+=(reTest-1); // because will increment one more time at top of loop
                activeLiteralIndex=-1;
            } else if (dfEfficiency>1.0) {
                // First, encode the number of units we expect
                compressionBuffer[p++]=c-i+32;

                // Then use DF. We want to move forward in units of 4, and pick the correct encoding.
                // Note that is doesn't matter if we pad extra 0s because we know what the correct
                // length is from above.
                V3DLONG cp=i;
                unsigned char d0,d1,d2,d3;
                d0=d1=d2=d3=0;
                while(cp<c) {
                    V3DLONG start=cp-i;
                    d0=dbuffer[start];
                    if (cp+1<c) {
                        d1=dbuffer[start+1];
                        if (cp+2<c) {
                            d2=dbuffer[start+2];
                            if (cp+3<c) {
                                d3=dbuffer[start+3];
                            }
                        }
                    }
                    // In decompression, we want FIFO for speed, so we put last first.
                    unsigned char v=0;
                    v |= d3;
                    v <<= 2;
                    v |= d2;
                    v <<= 2;
                    v |= d1;
                    v <<= 2;
                    v |= d0;

                    compressionBuffer[p++]=v;
                    cp+=4; // Move ahead 4 steps at a time
                }

                activeLiteralIndex=-1;
                i=c-1; // will increment at top
            } else { // This will catch the case where dfEfficiency is 0.0 due to i==0
                // We need to add this value as a literal. If there is already a literal mode, then simply
                // add it. Otherwise, start a new one.
                if (activeLiteralIndex<0 || compressionBuffer[activeLiteralIndex]>=32) {
                    // We need a new index
                    compressionBuffer[p++]=0;
                    activeLiteralIndex=p-1; // Our new literal index
                    compressionBuffer[p++]=currentValue; // Add the current value onto current sequence of literals
                } else {
                    compressionBuffer[activeLiteralIndex] += 1; // Increment existing literal count
                    compressionBuffer[p++]=currentValue; // Add the current value onto current sequence of literals
                }
            }
        }
    }
    return p;
}

V3DLONG ImageLoaderBasic::compressPBD16(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft) {
    bool debug=false;
    V3DLONG p=0;
    const int THREE_BIT_DIFF_MAX_LENGTH=79-31;
    const int FOUR_BIT_DIFF_MAX_LENGTH=182-79;
    const int FIVE_BIT_DIFF_MAX_LENGTH=222-182;
    const int REPEAT_MAX_LENGTH=255-222;

    if (sourceBufferLength==0) {
        printf("ImageLoaderBasic::compressPBD16 - unexpectedly received buffer of zero size\n");
        return 0;
    }

    v3d_uint16 currentValue=0;
    v3d_uint16 * source16Buffer = (v3d_uint16*)sourceBuffer;
    V3DLONG source16BufferLength = sourceBufferLength/2;
    int* dbuffer = 0;
    int d3buffer[256];
    int d4buffer[256];
    int d5buffer[256];
    V3DLONG activeLiteralIndex=-1; // if -1 this means there is no literal mode started

    // Minimum number of values to accept as an encoding preferable to literal:
    //
    //      Run-length: 2 { code, value } = 3 bytes to encode 2 values as repeats, vs 4 bytes to encode 2 literal values (STARTS NEW VALUE)
    //
    //      Three-bit: 2 { code, diff } = 2 bytes to encode 2 or more values, superior to literal starting at 2 values (EXTENDS PRIOR VALUE)
    //
    //      Four-bit:  2 { code, diff } = 2 bytes to encode 2 values, superior to literal starting at 2 values (EXTENDS PRIOR VALUE)
    //
    //      Five-bit: 3 { code, diff } = 3 bytes to encode 3 values, superior to literal start at 3 values (EXTENDS PRIOR VALUE)

    const double THREE_BIT_MAX_EFF = 16.0/3.0;
    const double FOUR_BIT_MAX_EFF = 16.0/4.0;
    const double FIVE_BIT_MAX_EFF = 16.0/5.0;

    for (V3DLONG i=0;i<source16BufferLength;i++) {

        if (p>=spaceLeft) {
            cerr << "ImageLoaderBasic::compressPBD16 ran out of space p=" << p << endl;
            return 0;
        }

        // From this point we assume the result has been accumulating in compressionBuffer, and at this moment
        // we are searching for the best approach for the next segment. First, we will try reading
        // the next value, and testing what the runlength encoding efficiency would be. The
        // efficiency is simply the (number of bytes encoded / actual bytes). Next, we will test
        // the different encoding, and (depending on its reach), see what its efficiency is.

        // We will test the efficiency of the various methods and see how well we can do from the current position.

        int repeatTest=1;
        currentValue=source16Buffer[i];
        //qDebug() << "At position=" << i << " value=" << currentValue;
        V3DLONG currentPosition=i+1;
        while(currentPosition<source16BufferLength && repeatTest<REPEAT_MAX_LENGTH) {
            if (source16Buffer[currentPosition++]==currentValue) {
                repeatTest++;
            } else {
                break;
            }
        }
        double repeatEfficiency = repeatTest*1.0 / 3.0; // 3-bytes are used for the encoding

        if (repeatEfficiency>=THREE_BIT_MAX_EFF) { // we can't do better than this with difference encoding
            // Then go ahead and use repeat runlength encoding - this will work even at the start
            compressionBuffer[p++]=repeatTest+222; // The code for number of repeats
            v3d_uint16 * repeatValuePointer = (v3d_uint16*)(compressionBuffer + p);
            *repeatValuePointer=currentValue;
            p=p+2; // advance p by two bytes
            i+=(repeatTest-1); // because will increment one more time at top of loop
            activeLiteralIndex=-1; // we have no current literal context
        } else {
            // This means we don't have an impossible-to-beat repeat context, so we'll try a difference encoding
            // to see if we can do better. We will try them from most efficient to least.
            double dfEfficiency=0.0;
            int dfType=0; // 0 is unknown, 3=3-bit, 4=4-bit, etc.
            double df3Efficiency=0.0;
            double df4Efficiency=0.0;
            double df5Efficiency=0.0;
            V3DLONG c=0;
            V3DLONG d3c=0;
            V3DLONG d4c=0;
            V3DLONG d5c=0;
            if (i>0) { // We can only use a difference encoding if we are initialized to have a prior value.
                v3d_uint16 priorValue=source16Buffer[i-1];
                int unitsToCheck=source16BufferLength-i;
                for (dfType=3;dfType<6;dfType++) {
                    c=i;
                    if (dfType==3) {
                        if (unitsToCheck>THREE_BIT_DIFF_MAX_LENGTH) {
                            unitsToCheck=THREE_BIT_DIFF_MAX_LENGTH;
                        }
                        for (int j=0;j<256;j++) {
                            d3buffer[j]=0; // clear the difference buffer
                        }
                        for (;c<i+unitsToCheck;c++) {
                            //qDebug() << "debug: df3  c=" << c << " value=" << source16Buffer[c];
                            int d=source16Buffer[c] - priorValue;
                            if (d>4 || d<-3) {
                                break;
                            }
                            priorValue=source16Buffer[c]; // since by definition the diff encoding is cumulative
                            d3buffer[c-i]=d;
                        }
                        df3Efficiency=((c-i)*2.0)/(((c-i)*(3.0/8.0))+2.0); // 2.0 handles 1 byte boundary + length bias
                        d3c=c;
                    } else if (dfType==4) {
                        if (unitsToCheck>FOUR_BIT_DIFF_MAX_LENGTH) {
                            unitsToCheck=FOUR_BIT_DIFF_MAX_LENGTH;
                        }
                        for (int j=0;j<256;j++) {
                            d4buffer[j]=0; // clear the difference buffer
                        }
                        for (;c<i+unitsToCheck;c++) {
                            int d=source16Buffer[c] - priorValue;
                            if (d>8 || d<-7) {
                                break;
                            }
                            priorValue=source16Buffer[c]; // since by definition the diff encoding is cumulative
                            d4buffer[c-i]=d;
                        }
                        df4Efficiency=((c-i)*2.0)/(((c-i)/2.0)+2.0); // 2.0 handles 1 byte boundary + length bias
                        d4c=c;
                    } else if (dfType==5) {
                        if (unitsToCheck>FIVE_BIT_DIFF_MAX_LENGTH) {
                            unitsToCheck=FIVE_BIT_DIFF_MAX_LENGTH;
                        }
                        for (int j=0;j<256;j++) {
                            d5buffer[j]=0; // clear the difference buffer
                        }
                        for (;c<i+unitsToCheck;c++) {
                            int d=source16Buffer[c] - priorValue;
                            if (d>16 || d<-15) {
                                break;
                            }
                            priorValue=source16Buffer[c]; // since by definition the diff encoding is cumulative
                            d5buffer[c-i]=d;
                        }
                        df5Efficiency=((c-i)*2.0)/(((c-i)*(5.0/8.0))+2.0); // 2.0 handles 1 byte boundary + length bias
                        d5c=c;
                    }
                }

                // DEBUG - TYPE 3 ONLY FOR NOW
                dfEfficiency=df3Efficiency;
                c=d3c;
                dfType=3;
                dbuffer=d3buffer;

//                if (df4Efficiency>df3Efficiency) {
//                    dfEfficiency=df4Efficiency;
//                    c=d4c;
//                    dfType=4;
//                    dbuffer=d4buffer;
//                } else {
//                    dfEfficiency=df3Efficiency;
//                    c=d3c;
//                    dfType=3;
//                    dbuffer=d3buffer;
//                }
//                if (df5Efficiency>dfEfficiency) {
//                    dfEfficiency=df5Efficiency;
//                    c=d5c;
//                    dfType=5;
//                    dbuffer=d5buffer;
//                }
            }
            // Now we can decide between RE and DF based on efficiency
            if (repeatEfficiency>dfEfficiency && repeatEfficiency>1.0) {
                // Then use RE
                compressionBuffer[p++]=repeatTest+222; // The code for number of repeats
                v3d_uint16 * cbp = (v3d_uint16*)(compressionBuffer + p);
                *cbp=currentValue;
                p=p+2;
                i+=(repeatTest-1); // because will increment one more time at top of loop
                activeLiteralIndex=-1;
            } else if (dfEfficiency>1.0) {

                V3DLONG cp;
                unsigned char d0,d1,d2,d3,d4;

                if (dfType==3) {
                    // First, encode the number of units we expect
                    compressionBuffer[p++]=c-i+31;

                    // Then use DF. We want to move forward in units of 4, and pick the correct encoding.
                    // Note that is doesn't matter if we pad extra 0s because we know what the correct
                    // length is from above.
                    cp=i;
                    d0=d1=d2=d3=d4=0;

                    // byte partition sequence 332, 1331, 233

                    while(cp<c) {

                        // 332
                        int dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d0=4-dvalue;
                        } else {
                            d0=dvalue;
                        }
                        cp++;
                        if (cp==c) {
                            d0 <<= 5;
                            compressionBuffer[p++]=d0;
                            break;
                        }
                        d0 <<= 3;
                        dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d1=4-dvalue;
                        } else {
                            d1=dvalue;
                        }
                        d0 |= d1;
                        cp++;
                        d0 <<= 2;
                        if (cp==c) {
                            compressionBuffer[p++]=d0;
                            break;
                        }
                        dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d2=4-dvalue;
                        } else {
                            d2=dvalue;
                        }
                        d3 = d2;
                        d3 &= ooooollo;
                        d3 >>= 1;
                        d0 |= d3;
                        compressionBuffer[p++]=d0;
                        d4 = d2;
                        d4 &= oooooool;
                        unsigned char carryOver=d4;
                        d0=d1=d2=d3=d4=0;

                        // 1331
                        d0=carryOver;
                        cp++;
                        if (cp==c) {
                            d0 <<= 7;
                            compressionBuffer[p++]=d0;
                            break;
                        }
                        d0 <<= 3;
                        dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d1=4-dvalue;
                        } else {
                            d1=dvalue;
                        }
                        d0 |= d1;
                        cp++;
                        d0 <<= 3;
                        if (cp==c) {
                            d0 <<= 1;
                            compressionBuffer[p++]=d0;
                            break;
                        }
                        dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d2=4-dvalue;
                        } else {
                            d2=dvalue;
                        }
                        d0 |= d2;
                        cp++;
                        d0 <<= 1;
                        if (cp==c) {
                            compressionBuffer[p++]=d0;
                            break;
                        }
                        dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d3=4-dvalue;
                        } else {
                            d3=dvalue;
                        }
                        d4=d3;
                        d3 &= oooooloo;
                        d3 >>= 2;
                        d0 |= d3;
                        compressionBuffer[p++]=d0;
                        d4 &= ooooooll;
                        carryOver = d4;
                        d0=d1=d2=d3=d4=0;

                        // 233
                        d0=carryOver;
                        cp++;
                        if (cp==c) {
                            d0 <<= 6;
                            compressionBuffer[p++]=d0;
                            break;
                        }
                        d0 <<= 3;
                        dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d1=4-dvalue;
                        } else {
                            d1=dvalue;
                        }
                        d0 |= d1;
                        cp++;
                        d0 <<= 3;
                        if (cp==c) {
                            compressionBuffer[p++]=d0;
                            break;
                        }
                        dvalue=dbuffer[cp-i];
                        if (dvalue<0) {
                            d2=4-dvalue;
                        } else {
                            d2=dvalue;
                        }
                        d0 |= d2;
                        compressionBuffer[p++]=d0;
                        cp++;
                    }
                }

                activeLiteralIndex=-1;
                i=c-1; // will increment at top

            } else { // This will catch the case where dfEfficiency is 0.0 due to i==0
                // We need to add this value as a literal. If there is already a literal mode, then simply
                // add it. Otherwise, start a new one.
                if (activeLiteralIndex<0 || compressionBuffer[activeLiteralIndex]>=31) {
                    // We need a new index
                    compressionBuffer[p++]=0;
                    activeLiteralIndex=p-1; // Our new literal index
                    v3d_uint16 * cbp = (v3d_uint16*)(compressionBuffer + p); // Add the current value onto current sequence of literals
                    *cbp=currentValue;
                    //v3d_uint16 testValue=*((v3d_uint16*)(compressionBuffer + p));
                    if (debug)
                        cerr << "Assigned literal value for i=" << i << " index=0  p=" << p << " currentValue=" << currentValue << endl;
                    p+=2;
                } else {
                    compressionBuffer[activeLiteralIndex] += 1; // Increment existing literal count
                    v3d_uint16 * cbp = (v3d_uint16*)(compressionBuffer + p); // Add the current value onto current sequence of literals
                    *cbp=currentValue;
                    //v3d_uint16 testValue=*((v3d_uint16*)(compressionBuffer + p));
                    if (debug) cerr << "Assigned literal value for i=" << i << " index=" << compressionBuffer[activeLiteralIndex] << "  p=" << p << " currentValue=" << currentValue << endl;
                    p+=2;
                }
            }
        }
    }
    return p;
}

V3DLONG ImageLoaderBasic::decompressPBD8(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength) {

    // Decompress data
    V3DLONG cp=0;
    V3DLONG dp=0;
    const unsigned char mask=0x0003;
    unsigned char p0,p1,p2,p3;
    unsigned char value=0;
    unsigned char pva=0;
    unsigned char pvb=0;
    int leftToFill=0;
    int fillNumber=0;
    unsigned char * toFill=0;
    unsigned char sourceChar=0;
    while(cp<sourceLength) {

        if (isCanceled())
            return dp;

        value=sourceData[cp];

        if (value<33) {
            // Literal 0-32
            unsigned char count=value+1;
            for (V3DLONG j=cp+1;j<cp+1+count;j++) {
                targetData[dp++]=sourceData[j];
            }
            cp+=(count+1);
            decompressionPrior=targetData[dp-1];
        } else if (value<128) {
            // Difference 33-127
            leftToFill=value-32;
            while(leftToFill>0) {
                fillNumber=(leftToFill<4 ? leftToFill : 4);
                sourceChar=sourceData[++cp];
                toFill = targetData+dp;
                p0=sourceChar & mask;
                sourceChar >>= 2;
                p1=sourceChar & mask;
                sourceChar >>= 2;
                p2=sourceChar & mask;
                sourceChar >>= 2;
                p3=sourceChar & mask;
                pva=(p0==3?-1:p0)+decompressionPrior;

                *toFill=pva;
                if (fillNumber>1) {
                    toFill++;
                    pvb=pva+(p1==3?-1:p1);
                    *toFill=pvb;
                    if (fillNumber>2) {
                        toFill++;
                        pva=(p2==3?-1:p2)+pvb;
                        *toFill=pva;
                        if (fillNumber>3) {
                            toFill++;
                            *toFill=(p3==3?-1:p3)+pva;
                        }
                    }
                }

                decompressionPrior = *toFill;
                dp+=fillNumber;
                leftToFill-=fillNumber;
            }
            cp++;
        } else {
            // Repeat 128-255
            unsigned char repeatCount=value-127;
            unsigned char repeatValue=sourceData[++cp];

            for (int j=0;j<repeatCount;j++) {
                targetData[dp++]=repeatValue;
            }
            decompressionPrior=repeatValue;
            cp++;
        }

    }
    return dp;
}

V3DLONG ImageLoaderBasic::decompressPBD16(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength)
{
    // bool debug=false;

    // Decompress data
    V3DLONG cp=0;
    V3DLONG dp=0;
    unsigned char code=0;
    int leftToFill=0;
    unsigned char sourceChar=0;
    unsigned char carryOver=0;
    v3d_uint16* target16Data=(v3d_uint16*)targetData;

    unsigned char d0,d1,d2,d3,d4;

    while(cp<sourceLength) {

        if (isCanceled())
            return dp*2;

        code=sourceData[cp];

         //if (debug) qDebug() << "decompressPBD16  dPos=" << decompPos << " dBuf=" << decompBuf << " decompressionPrior=" << decompressionPrior << " debugThreshold=" << debugThreshold << " cp=" << cp << " code=" << code;

        // Literal 0-31
        if (code<32) {
            unsigned char count=code+1;
            //if (debug) qDebug() << "decompressPBD16  literal count=" << count;
            v3d_uint16 * initialOffset = (v3d_uint16*)(sourceData + cp + 1);
            for (int j=0;j<count;j++) {
                target16Data[dp++]=initialOffset[j];
                //if (debug) qDebug() << "decompressPBD16 added literal value=" << target16Data[dp-1] << " at position=" << ((decompressionPosition-decompressionBuffer) + 2*(dp-1));
            }
            cp+=(count*2+1);
            decompressionPrior=target16Data[dp-1];
            //if (debug) qDebug() << "debug: literal set decompressionPrior=" << decompressionPrior;
        }

        // NOTE: For the difference sections, we will unroll conditional
        // logic and explicitly go through the full cycle byte-boundary cycle.

        // Difference 3-bit 32-79
        else if (code<80) {
            leftToFill=code-31;
            //if (debug) qDebug() << "decompressPBD16 leftToFill start=" << leftToFill << " decompressionPrior=" << decompressionPrior;
            while(leftToFill>0) {

                // 332
                d0=d1=d2=d3=d4=0;
                sourceChar=sourceData[++cp];
                d0=sourceChar;
                d0 >>= 5;
                target16Data[dp++]=decompressionPrior+(d0<5?d0:4-d0);
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1] << " d0=" << d0;
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                d1=sourceChar;
                d1 >>= 2;
                d1 &= ooooolll;
                target16Data[dp]=target16Data[dp-1]+(d1<5?d1:4-d1);
                dp++;
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1];
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                d2=sourceChar;
                d2 &= ooooooll;
                carryOver=d2;

                // 1331
                d0=d1=d2=d3=d4=0;
                sourceChar=sourceData[++cp];
                d0=sourceChar;
                carryOver <<= 1;
                d0 >>= 7;
                d0 |= carryOver;
                target16Data[dp]=target16Data[dp-1]+(d0<5?d0:4-d0);
                dp++;
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1];
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                d1=sourceChar;
                d1 >>= 4;
                d1 &= ooooolll;
                target16Data[dp]=target16Data[dp-1]+(d1<5?d1:4-d1);
                dp++;
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1];
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                d2=sourceChar;
                d2 >>= 1;
                d2 &= ooooolll;
                target16Data[dp]=target16Data[dp-1]+(d2<5?d2:4-d2);
                dp++;
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1];
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                d3=sourceChar;
                d3 &= oooooool;
                carryOver=d3;

                // 233
                d0=d1=d2=d3=d4=0;
                sourceChar=sourceData[++cp];
                d0=sourceChar;
                d0 >>= 6;
                carryOver <<= 2;
                d0 |= carryOver;
                target16Data[dp]=target16Data[dp-1]+(d0<5?d0:4-d0);
                dp++;
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1];
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                d1=sourceChar;
                d1 >>= 3;
                d1 &= ooooolll;
                target16Data[dp]=target16Data[dp-1]+(d1<5?d1:4-d1);
                dp++;
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1];
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                d2=sourceChar;
                d2 &= ooooolll;
                target16Data[dp]=target16Data[dp-1]+(d2<5?d2:4-d2);
                dp++;
                //if (debug) qDebug() << "debug: position " << (dp-1) << " diff value=" << target16Data[dp-1];
                leftToFill--;
                if (leftToFill==0) {
                    break;
                }
                decompressionPrior=target16Data[dp-1];
            }
            decompressionPrior=target16Data[dp-1];
            //if (debug) qDebug() << "debug: diff set decompressionPrior=" << decompressionPrior;
            cp++;
        } else if (code<223) {
            cerr << "DEBUG: Mistakenly received unimplemented code of " << code << " at dp=" << dp << " cp=" << cp << " decompressionPrior=" << decompressionPrior << endl;
        }
        // Repeat 223-255
        else {
            unsigned char repeatCount=code-222;
            cp++;
            v3d_uint16 repeatValue=*((v3d_uint16*)(sourceData + cp));
            //if (debug) qDebug() << "decompressPBD16  repeatCount=" << repeatCount << " repeatValue=" << repeatValue;
            for (int j=0;j<repeatCount;j++) {
                target16Data[dp++]=repeatValue;
            }
            decompressionPrior=repeatValue;
            //if (debug) qDebug() << "debug: repeat set decompressionPrior=" << decompressionPrior;
            cp+=2;
            //if (debug) qDebug() << "decompressPBD16  finished adding repeats at dp=" << dp << " cp=" << cp;
        }

    }
    return dp*2;
}


// This is the main decompression function, which is basically a wrapper for the decompression function
// of ImageLoaderBasic, except it determines the boundary of acceptable processing given the contents of the
// most recently read block. This function assumes it is called sequentially (i.e., not in parallel)
// and so doesn't have to check that the prior processing step is finished. updatedCompressionBuffer
// points to the first invalid data position, i.e., all previous positions starting with compressionBuffer
// are valid.
void ImageLoaderBasic::updateCompressionBuffer8(unsigned char * updatedCompressionBuffer) {
    //printf("d1\n");
    if (compressionPosition==0) {
        // Just starting
        compressionPosition=&compressionBuffer[0];
    }
    unsigned char * lookAhead=compressionPosition;
    while(lookAhead<updatedCompressionBuffer) {
        unsigned char lav=*lookAhead;
        // We will keep going until we find nonsense or reach the end of the block
        if (lav<33) {
            // Literal values - the actual number of following literal values
            // is equal to the lav+1, so that if lav==32, there are 33 following
            // literal values.
            if ( lookAhead+lav+1 < updatedCompressionBuffer ) {
                // Then we can process the whole literal section - we can move to
                // the next position
                lookAhead += (lav+2);
            } else {
                break; // leave lookAhead in current maximum position
            }
        } else if (lav<128) {
            // Difference section. The number of difference entries is equal to lav-32, so that
            // if lav==33, the minimum, there will be 1 difference entry. For a given number of
            // difference entries, there are a factor of 4 fewer compressed entries. With the
            // equation below, lav-33 will be 4 when lav==37, which is 5 entries, requiring 2 bytes, etc.
            unsigned char compressedDiffEntries=(lav-33)/4 + 1;
            if ( lookAhead+compressedDiffEntries < updatedCompressionBuffer ) {
                // We can process this section, so advance to next position to evaluate
                lookAhead += (compressedDiffEntries+1);
            } else {
                break; // leave in current max position
            }
        } else {
            // Repeat section. Number of repeats is equal to lav-127, but the very first
            // value is the value to be repeated. The total number of compressed positions
            // is always == 2
            if ( lookAhead+1 < updatedCompressionBuffer ) {
                lookAhead += 2;
            } else {
                break; // leave in current max position
            }
        }
    }
    // At this point, lookAhead is in an invalid position, which if equal to updatedCompressionBuffer
    // means the entire compressed update can be processed.
    V3DLONG compressionLength=lookAhead-compressionPosition;
    if (decompressionPosition==0) {
        // Needs to be initialized
        decompressionPosition=decompressionBuffer;
    }
    //qDebug() << "updateCompressionBuffer calling decompressPBD compressionPosition=" << compressionPosition << " decompressionPosition=" << decompressionPosition
    //        << " size=" << compressionLength << " previousTotalDecompSize=" << getDecompressionSize() << " maxDecompSize=" << maxDecompressionSize;
    V3DLONG dlength=decompressPBD8(compressionPosition, decompressionPosition, compressionLength);
    compressionPosition=lookAhead;
    decompressionPosition+=dlength;
    //printf("d2\n");
}


// This is the main decompression function, which is basically a wrapper for the decompression function
// of ImageLoaderBasic, except it determines the boundary of acceptable processing given the contents of the
// most recently read block. This function assumes it is called sequentially (i.e., not in parallel)
// and so doesn't have to check that the prior processing step is finished. updatedCompressionBuffer
// points to the first invalid data position, i.e., all previous positions starting with compressionBuffer
// are valid.
void ImageLoaderBasic::updateCompressionBuffer16(unsigned char * updatedCompressionBuffer) {
    //printf("d1\n");
    if (compressionPosition==0) {
        // Just starting
        compressionPosition=&compressionBuffer[0];
    }
    unsigned char * lookAhead=compressionPosition;
    while(lookAhead<updatedCompressionBuffer) {
        // We assume at this point that lookAhead is at a code position
        unsigned char lav=*lookAhead;
        // We will keep going until we find nonsense or reach the end of the block
        if (lav<32) {
            // Literal values - the actual number of following literal values
            // is equal to the lav+1, so that if lav==31, there are 32 following
            // literal values.
            if ( lookAhead+((lav+1)*2) < updatedCompressionBuffer ) {
                // Then we can process the whole literal section - we can move to
                // the next position
                lookAhead += ( (lav+1)*2 + 1); // +1 is for code
            } else {
                break; // leave lookAhead in current maximum position
            }
        } else if (lav<80) {
            // Difference section, 3-bit encoding.
            // The number of difference entries is equal to lav-31, so that
            // if lav==32, the minimum, there will be 1 difference entry.
            unsigned char compressedDiffBytes=int(((((lav-31)*3)*1.0)/8.0)-0.0001) + 1;
//            int a=(lav-31)*3;
//            double b=a*1.0;
//            double c=b/8.0;
//            double d=c-0.0001;
//            int e=int(d);
//            int f=e+1;
//            qDebug() << "lav=" << lav << " lav-31=" << (lav-31) << " cdb=" << compressedDiffBytes;
//            qDebug() << "a=" << a << " b=" << b << " c=" << c << " d=" << d << " e=" << e << " f=" << f;
            if ( lookAhead+compressedDiffBytes < updatedCompressionBuffer ) {
                // We can process this section, so advance to next position to evaluate
                lookAhead += (compressedDiffBytes+1); // +1 is for code
            } else {
                break; // leave in current max position
            }
        } else if (lav<183) {
            // Difference section, 4-bit encoding.
            // The number of difference entries is equal to lav-79, so that
            // if lav==80, the minimum, there will be 1 difference entry.
            unsigned char compressedDiffBytes=int(((((lav-79)*4)*1.0)/8.0)-0.0001) + 1;
            if ( lookAhead+compressedDiffBytes < updatedCompressionBuffer ) {
                // We can process this section, so advance to next position to evaluate
                lookAhead += (compressedDiffBytes+1); // +1 is for code
            } else {
                break; // leave in current max position
            }
        } else if (lav<223) {
            // Difference section, 5-bit encoding.
            // The number of difference entries is equal to lav-182, so that
            // if lav==183, the minimum, there will be 1 difference entry.
            unsigned char compressedDiffBytes=int(((((lav-182)*5)*1.0)/8.0)-0.0001) + 1;
            if ( lookAhead+compressedDiffBytes < updatedCompressionBuffer ) {
                // We can process this section, so advance to next position to evaluate
                lookAhead += (compressedDiffBytes+1); // +1 is for code
            } else {
                break; // leave in current max position
            }
        } else {
            // Repeat section. Number of repeats is equal to lav-222, but the very first
            // value is the value to be repeated. The total number of compressed positions
            // is always == 3, one for the code and 2 for the 16-bit value
            if ( lookAhead+2 < updatedCompressionBuffer ) {
                lookAhead += 3;
            } else {
                break; // leave in current max position
            }
        }
    }
    // At this point, lookAhead is in an invalid position, which if equal to updatedCompressionBuffer
    // means the entire compressed update can be processed.
    V3DLONG compressionLength=lookAhead-compressionPosition;
    if (decompressionPosition==0) {
        // Needs to be initialized
        decompressionPosition=decompressionBuffer;
    }
    //qDebug() << "updateCompressionBuffer calling decompressPBD compressionPosition=" << compressionPosition << " decompressionPosition=" << decompressionPosition
    //        << " size=" << compressionLength << " previousTotalDecompSize=" << getDecompressionSize() << " maxDecompSize=" << maxDecompressionSize;
    V3DLONG dlength=decompressPBD16(compressionPosition, decompressionPosition, compressionLength);
    compressionPosition=lookAhead;
    decompressionPosition+=dlength;
    //printf("d2\n");
}

/* virtual */
int ImageLoaderBasic::exitWithError(std::string errorMessage) {
  cerr << errorMessage << endl;
  if (fid!=0) {
    fclose(fid);
    fid = 0;
  }
  int berror=1;
  return berror;
}

int ImageLoaderBasic::loadRaw2StackPBD(const char* filename, Image4DSimple * image, bool useThreading)
{
    fid = fopen(filename, "rb");
    if (! fid)
        return exitWithError(std::string("Fail to open file for reading."));
    fseek (fid, 0, SEEK_END);
    V3DLONG fileSize = ftell(fid);
    rewind(fid);
    FileStarStream fileStream(fid);
    return loadRaw2StackPBD(fileStream, fileSize, image, useThreading);
}

/* virtual */
int ImageLoaderBasic::loadRaw2StackPBD(DataStream& fileStream, V3DLONG fileSize, Image4DSimple * image, bool useThreading)
{
    if (useThreading) {
        cerr << "Error: attempt to use threading with ImageLoaderBasic" << __FILE__ << __LINE__ << endl;
    }

    decompressionPrior = 0;
    int berror = 0;

    /* Read header */
    char formatkey[] = "v3d_volume_pkbitdf_encod";
    V3DLONG lenkey = strlen(formatkey);

#ifndef _MSC_VER //added by PHC, 2010-05-21
    if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
    {
        stringstream msg;
        msg << "The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n";
        msg << "The fseek-ftell produces a file size = " << fileSize;
        return exitWithError(msg.str());
    }
#endif

    keyread.resize(lenkey+1);
    // V3DLONG nread = fread(&keyread[0], 1, lenkey, fid);
    V3DLONG nread = fileStream.read(&keyread[0], lenkey);
    if (nread!=lenkey)
    {
        return exitWithError(std::string("File unrecognized or corrupted file."));
    }
    keyread[lenkey] = '\0';

    V3DLONG i;
    if (strcmp(formatkey, &keyread[0])) /* is non-zero then the two strings are different */
    {
        return exitWithError("Unrecognized file format.");
    }

    char endianCodeData;
    // fread(&endianCodeData, 1, 1, fid);
    fileStream.read(&endianCodeData, 1);
    if (endianCodeData!='B' && endianCodeData!='L')
    {
        return exitWithError("This program only supports big- or little- endian but not other format. Check your data endian.");
    }

    char endianCodeMachine;
    endianCodeMachine = checkMachineEndian();
    if (endianCodeMachine!='B' && endianCodeMachine!='L')
    {
        return exitWithError("This program only supports big- or little- endian but not other format. Check your data endian.");
    }

    int b_swap = (endianCodeMachine==endianCodeData)?0:1;

    short int dcode = 0;
    // fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
    fileStream.read((char*)&dcode, 2); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
    if (b_swap)
        swap2bytes((void *)&dcode);

    int datatype;

    switch (dcode)
    {
    case PBD_3_BIT_DTYPE:
      datatype = PBD_3_BIT_DTYPE; // used for pbd3
      break;

    case 1:
        datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
        break;

    case 2:
        datatype = 2;
        break;

    case 4:
        datatype = 4;
        break;

    default:
        stringstream msg;
        msg << "Unrecognized data type code [";
        msg << dcode;
        msg << "The file type is incorrect or this code is not supported in this version.";
        return exitWithError(msg.str());
    }

    // qDebug() << "Setting datatype=" << datatype;

    if (datatype==1 || datatype==PBD_3_BIT_DTYPE) {
        image->setDatatype(V3D_UINT8);
    } else if (datatype==2) {
        image->setDatatype(V3D_UINT16);
    } else {
        return exitWithError("ImageLoader::loadRaw2StackPBD : only datatype=1 or datatype=2 supported");
    }
    loadDatatype=image->getDatatype(); // used for threaded loading

    // qDebug() << "Finished setting datatype=" << image->getDatatype();

    V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future.

    BIT32_UNIT mysz[4];
    mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
    // int tmpn=fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
    int tmpn=fileStream.read((char*)mysz, 16); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
    if (tmpn!=16)
    {
        stringstream msg;
        msg << "This program only reads [" << tmpn << "] units.";
        return exitWithError(msg.str());
    }

    if (b_swap && (unitSize==2 || unitSize==4)) {
        stringstream msg;
        msg << "b_swap true and unitSize > 1 - this is not implemented in current code";
        return exitWithError(msg.str());
    }

    if (b_swap)
    {
        for (i=0;i<4;i++)
        {
            //swap2bytes((void *)(mysz+i));
            printf("mysz raw read unit[%ld]: [%d] ", i, mysz[i]);
            swap4bytes((void *)(mysz+i));
            printf("swap unit: [%d][%0x] \n", mysz[i], mysz[i]);
        }
    }

    std::vector<V3DLONG> sz(4, 0); // avoid memory leak
    // V3DLONG * sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img.

    V3DLONG totalUnit = 1;
    for (i=0;i<4;i++)
    {
        sz[i] = (V3DLONG)mysz[i];
	pbd_sz[i]=sz[i];
	cerr << "Set pbd_sz " << i << " to " << pbd_sz[i] << "\n";
        totalUnit *= sz[i];
    }

    //mexPrintf("The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Exit.\n", fileSize, totalUnit*unitSize+4*4+2+1+lenkey);
    //mexPrintf("The read sizes are: %ld %ld %ld %ld\n", sz[0], sz[1], sz[2], sz[3]);

    V3DLONG headerSize=4*4+2+1+lenkey;
    V3DLONG compressedBytes=fileSize-headerSize;
    maxDecompressionSize=totalUnit*unitSize;
    channelLength=sz[0]*sz[1]*sz[2];
    compressionBuffer.resize(compressedBytes);
    pbd3_current_channel=0;

    V3DLONG remainingBytes = compressedBytes;
    //V3DLONG nBytes2G = V3DLONG(1024)*V3DLONG(1024)*V3DLONG(1024)*V3DLONG(2);
    V3DLONG readStepSizeBytes = V3DLONG(1024)*20000; // PREVIOUS
    totalReadBytes = 0;

    // done reading header
    // Transfer data to My4DImage

    // Allocating memory can take seconds.  So send a message
    short int blankImageDataType=datatype;
    if (datatype==PBD_3_BIT_DTYPE) {
      blankImageDataType=1;
    }
    image->createBlankImage(sz[0], sz[1], sz[2], sz[3], blankImageDataType);
    decompressionBuffer = image->getRawData();

    while (remainingBytes>0)
    {
        // qDebug() << "ImageLoader::loadRaw2StackPBD" << filename << stopwatch.elapsed() << __FILE__ << __LINE__;
        if (isCanceled()) {
            // clean and return
            // fclose(fid);
            return exitWithError(std::string("image load canceled"));
        }

        V3DLONG curReadBytes = (remainingBytes<readStepSizeBytes) ? remainingBytes : readStepSizeBytes;
	pbd3_current_channel=totalReadBytes/channelLength;
	cerr << "pbd3_current_channel " << pbd3_current_channel << "\n";
	V3DLONG bytesToChannelBoundary=(pbd3_current_channel+1)*channelLength - totalReadBytes;
	curReadBytes = (curReadBytes>bytesToChannelBoundary) ? bytesToChannelBoundary : curReadBytes;
        // nread = fread(&compressionBuffer[0]+totalReadBytes, 1, curReadBytes, fid);
        nread = fileStream.read((char*)(&compressionBuffer[0]+totalReadBytes), curReadBytes);
        totalReadBytes+=nread;

	cerr << "nread=" << nread << " curReadBytes=" << curReadBytes << " bytesToChannelBoundary=" << bytesToChannelBoundary << " totalReadBytes=" << totalReadBytes << "\n";

        if (nread!=curReadBytes)
        {
            stringstream msg;
            msg << "Something wrong in file reading. The program reads [";
            msg << nread << " data points] but the file says there should be [";
            msg << curReadBytes << " data points].";
            return exitWithError(msg.str());
        }
        remainingBytes -= nread;

        if (datatype==1) {
            updateCompressionBuffer8(&compressionBuffer[0]+totalReadBytes);
        } else if (datatype==PBD_3_BIT_DTYPE) {
	  updateCompressionBuffer3(&compressionBuffer[0]+totalReadBytes);
	} else {
            // assume datatype==2
            updateCompressionBuffer16(&compressionBuffer[0]+totalReadBytes);
        }

        if (isCanceled()) {
            return exitWithError(std::string("load canceled"));
        }
    }
    // qDebug() << "ImageLoader::loadRaw2StackPBD" << filename << stopwatch.elapsed() << __FILE__ << __LINE__;
    // qDebug() << "Total time elapsed after all reads is " << stopwatch.elapsed() / 1000.0 << " seconds";

    // clean and return
    // fclose(fid); //bug fix on 060412
    return berror;
}

V3DLONG ImageLoaderBasic::compressPBD3(unsigned char * compressionBuffer, unsigned char * sourceBuffer, V3DLONG sourceBufferLength, V3DLONG spaceLeft) {

    V3DLONG p=0;

    if (sourceBufferLength==0) {
        printf("ImageLoaderBasic::compressPBD3 - unexpectedly received buffer of zero size\n");
        return 0;
    }
    V3DLONG activeLiteralIndex=-1; // if -1 this means there is no literal mode started

    // Now that we have the min and max values, we can replace the source array in-place with the 3-bit version
    int channelCount=pbd_sz[3];
    cerr << "channelCount=" << channelCount << " channelLength=" << channelLength << "\n";
    pbd3_source_min=new unsigned char[channelCount];
    pbd3_source_max=new unsigned char[channelCount];
    unsigned char sourceMin=0;
    unsigned char sourceMax=0;
    for (int c=0;c<channelCount;c++) {
      sourceMin=255;
      sourceMax=0;
      V3DLONG cStart=c*channelLength;
      V3DLONG cEnd=cStart+channelLength;
      // Find the min and max values so that we know our 3-bit range within the 8-bit envelope
      for (V3DLONG cp=cStart;cp<cEnd;cp++) {
	unsigned char value=sourceBuffer[cp];
	if (value<sourceMin) {
	  sourceMin=value;
	}
	if (value>sourceMax) {
	  sourceMax=value;
	}
      }
      pbd3_source_min[c]=sourceMin;
      pbd3_source_max[c]=sourceMax;
      compressionBuffer[p++]=sourceMin;
      compressionBuffer[p++]=sourceMax;
      int iMin=sourceMin;
      int iMax=sourceMax;
      cerr << "channel " << c << " sourceMin=" << iMin << " sourceMax=" << iMax << "\n";
      int diffDivisor=(sourceMax - sourceMin);
      for (V3DLONG cp=cStart;cp<cEnd;cp++) {
	int diff = sourceBuffer[cp] - sourceMin;
	int newValue = (diff * 8) / diffDivisor;
	if (newValue >= 8) newValue=7;
	sourceBuffer[cp] = newValue;
      }
    }

    V3DLONG* firstRepeatBuffer=new V3DLONG[2];

    DEBUG_SUMMARY=false;
    DEBUG=false;

    V3DLONG i=0;

    cerr << "sourceBufferLength = " << sourceBufferLength << "\n";

    for (;i<sourceBufferLength;i++) {

      //      if (i>40400000) {
      ///	DEBUG_SUMMARY=true;
      //      }

      //      if (i>388514000) {
      //	DEBUG_SUMMARY=true;
      //	DEBUG=true;
      //	if (i>388516000) {
      //	  DEBUG_SUMMARY=false;
      //	  DEBUG=false;
      //	}
      //      }

      if (DEBUG_SUMMARY) cerr << "Top of main loop, i=" << i << "\n";

        if (p>=spaceLeft) {
            printf("ImageLoaderBasic::compressPBD3 ran out of space p=%ld\n", p);
            return 0;
        }

        // From this point we assume the result has been accumulating in compressionBuffer, and at this moment
        // we are searching for the best approach for the next segment. First, we will try reading
        // the next value, and testing what the runlength encoding efficiency would be. The
        // efficiency is simply the (number of bytes encoded / actual bytes). Next, we will test
        // the different encoding, and (depending on its reach), see what its efficiency is.

	// Here, we do a look-ahead to see if we can get a repeat, the most efficient representation.
	// PBD3 supports repeats up to 33279.

	if (DEBUG) cerr << "Main Loop: finding repeat count\n";

	V3DLONG currentRepeatTest=pbd3FindRepeatCountFromCurrentPosition(sourceBuffer, i, sourceBufferLength);

	// Check if it is obvious we should go with repeat
	if (currentRepeatTest>5) {
	  if (DEBUG) cerr << "Starting repeat\n";
	  // Repeat
	  pbd3FlushLiteral(compressionBuffer, sourceBuffer, &activeLiteralIndex, &p, i);
	  unsigned char keyByte;
	  unsigned char valueByte;
	  if (currentRepeatTest>(PBD_3_REPEAT_MAX-1)) {
	    currentRepeatTest=PBD_3_REPEAT_MAX-1;
	  }
	  pbd3EncodeRepeat(sourceBuffer[i], currentRepeatTest+1, &keyByte, &valueByte);
	  compressionBuffer[p++]=keyByte;
	  compressionBuffer[p++]=valueByte;
	  if (DEBUG_SUMMARY) {
	    int ik=keyByte;
	    int iv=valueByte;
	    int ic=currentRepeatTest+1;
	    cerr << "compress repeat, count=" << ic << " actual bytes: key=" << ik << " value=" << iv << "\n";
	  }
	  i+=currentRepeatTest;
	} else {
	  // Try to proceed with Diff encoding. If we make progress with Diff, we simultaneously need to
	  // watch for repeats. If we find a repeat, the worst case efficiency is if we have just invested
	  // in a new byte for Diff, leaving 7 unused bits, missing out on using 2 3-bit doublets, or 4
	  // diff positions, on the remaining first byte and then 3 more 3-bit doublets using another
	  // byte, for a total of 4+6=10 diff positions. But, it gets worse, because we actually need two
	  // completely fresh bytes for repeat, which means another byte with 2 more doublets = 14 positions.
	  // This means if the repeat is 14 or greater, we always want to use a repeat, but if the repeat is
	  // less than 14, we should use Diff if the Diff can cover the repeat.

	  if (DEBUG) cerr << "Starting diff test\n";

	  V3DLONG diffTest = pbd3FindDiffCountFromCurrentPosition(sourceBuffer, i, sourceBufferLength);

	  if (DEBUG) cerr << "diffTest=" << diffTest << "\n";

	  // Reset repeat buffer
	  firstRepeatBuffer[0]=0;
	  firstRepeatBuffer[1]=0;
	  // Check for repeats within the diff region
	  if (diffTest>PBD_3_DIFF_REPEAT_THRESHOLD) {
	    if (DEBUG) cerr << "Check1\n";
	    pbd3FindFirstRepeatOfMinLength(sourceBuffer, i, diffTest, PBD_3_DIFF_REPEAT_THRESHOLD, firstRepeatBuffer);
	    if (firstRepeatBuffer[PBD_3_REPEAT_STARTING_POSITION]>0) {
	      // Implies repeat found - encode the pre-repeat region.
	      diffTest=firstRepeatBuffer[PBD_3_REPEAT_STARTING_POSITION] - i;
	      // If diffTest is not even, this means the repeat starts off-stride, so grab the extra position
	      if (diffTest % 2 != 0) {
		diffTest+=1;
	      }
	    }
	  }
	  if (DEBUG) cerr << "Check3\n";
	  // If we found a qualifying repeat, then we need to encode the pre-repeat section, then let the outer loop
	  // detect the following repeat. If not, then we need to encode the diff sequence if 3 or greater, otherwise
	  // just accumulate this in the literal outer context.
	  if (diffTest > PBD_3_MIN_DIFF_LENGTH) {
	    if (DEBUG) cerr << "Check3.5 diffTest=" << diffTest << "\n";
	    // Handle max-size issue
	    if (diffTest>PBD_3_MAX_DIFF_LENGTH) {
	      diffTest=PBD_3_MAX_DIFF_LENGTH;
	    }
	    if (DEBUG) cerr << "Check4 diffTest=" << diffTest << "\n";

	    int diffByteCount=0;
	    pbd3FlushLiteral(compressionBuffer, sourceBuffer, &activeLiteralIndex, &p, i);
	    if (DEBUG) cerr << "Check5 - diffTest=" << diffTest << "\n";
	    unsigned char* diffEncoding=pbd3EncodeDiff(sourceBuffer, i, diffTest, &diffByteCount);
	    if (DEBUG) cerr << "Returned diffEncoding with " << diffByteCount << " bytes\n";
	    int di=0;
	    while(di<diffByteCount) {
	      if (DEBUG) {
		int dV=diffEncoding[di];
		cerr << "Adding compression for diff byte " << di << " =" << dV << "\n";
	      }
	      compressionBuffer[p++]=diffEncoding[di++];
	    }
	    if (DEBUG) cerr << "Done adding diff encoded compression bytes - clearing array\n";
	    delete [] diffEncoding;
	    if (DEBUG) cerr << "Done clearing diff result array - diffTest=" << diffTest << "\n";
	    i+=diffTest-1;
	    if (DEBUG) cerr << "========= Now i=" << i << " diffTest=" << diffTest << "\n";

	  } else {
	    // Neither repeat nor diff qualify, continue (or start if necessary) literal mode
	    if (activeLiteralIndex>-1) {
	      // we have an active literal mode - just let it continue - but check if we have to flush it
	      if ( (i-activeLiteralIndex) >= PBD_3_MAX_LITERAL) {
		pbd3FlushLiteral(compressionBuffer, sourceBuffer, &activeLiteralIndex, &p, i);
		double cRatio = (i * 1.0) / (p * 1.0);
		if (DEBUG) cerr << "Flushing literal at i=" << i << " cRatio=" << cRatio << "\n";
		activeLiteralIndex=i;
	      }
	    } else {
	      activeLiteralIndex=i;
	    }
	  }
	}
    }

    if (DEBUG) cerr << "Done main loop - deleting buffers\n";

    delete [] firstRepeatBuffer;

    cerr << "source buffer length=" << sourceBufferLength << " i=" << i << "\n";

    return p;
}

// This returns 0 if there are no repeats, 1 if two values are the same, etc.
V3DLONG ImageLoaderBasic::pbd3FindRepeatCountFromCurrentPosition(unsigned char* sourceBuffer, V3DLONG position, V3DLONG maxPosition)
{
  V3DLONG p=position;
  V3DLONG repeatLength=0;
  while(p<(maxPosition-1) && repeatLength<PBD_3_REPEAT_MAX) {
    unsigned char p0=sourceBuffer[p];
    unsigned char p1=sourceBuffer[p+1];
    if (p1!=p0) {
      break;
    } else {
      p++;
      repeatLength++;
    }
  }
  return repeatLength;
}

void ImageLoaderBasic::pbd3EncodeRepeat(unsigned char sourceValue, V3DLONG count, unsigned char* keyByte, unsigned char* valueByte)
{
  if (sourceValue > 7) {
    cerr << "Error: sourceValue byte should never have value greater than 7 for PBD 3";
    exit(1);
  }
  if (count>PBD_3_REPEAT_MAX) {
    cerr << "Error: can not encode repeat larger than " << PBD_3_REPEAT_MAX << "\n";
    exit(1);
  }
  // The actual number to encode is one less than 'count' so that 0 imlies a single instance of the value
  count--;
  if (count<0) {
    cerr << "Error: cannot encode repeat less than zero\n";
    exit(1);
  }
  if (count < 128) {
    // Can just use the 7-bits in the key
    *keyByte = (unsigned char)(count+128);
    sourceValue <<= 5;
    *valueByte=sourceValue;
  } else {
    // Need to use the extra 5-bits
    int ival=count;
    unsigned char* iarr = (unsigned char*)(&ival);
    unsigned char b0=iarr[0];
    unsigned char b1=iarr[1];
    unsigned char b0c=b0;
    b0c &= olllllll;
    *keyByte = b0c+128;
    b0 >>= 7;
    b1 <<= 1;
    b1 |= b0;
    b1 &= ooolllll;
    sourceValue <<= 5;
    *valueByte = sourceValue | b1;
  }
  if (DEBUG) cerr << "PBD3 Encoded repeat length=" << count << "\n";
}

V3DLONG ImageLoaderBasic::pbd3FindDiffCountFromCurrentPosition(unsigned char* sourceBuffer, V3DLONG position, V3DLONG maxPosition)
{
  // Here, we want to simply see how far we can get making single-step changes to the value. If we see an in-stride {-1, -1} then we must stop.
  if (DEBUG)  cerr << "ImageLoaderBasic::pbd3FindDiffCountFromCurrentPosition: position=" << position << " maxPosition=" << maxPosition << "\n";

  unsigned char priorValue=0;
  if (position>0) {
    priorValue=sourceBuffer[position-1];
  }
  V3DLONG r=0;
  int d1=0;
  int d2=0;
  if (maxPosition > (position + PBD_3_MAX_DIFF_LENGTH)) {
    maxPosition=position+PBD_3_MAX_DIFF_LENGTH;
  }
  bool pairComplete=true; // start in this state
  while(position<maxPosition) {
    if (DEBUG) cerr << "inner loop: position=" << position << " maxPosition=" << maxPosition << "\n";
    unsigned char currentValue=sourceBuffer[position];
    d2=d1;
    d1=currentValue-priorValue;
    if (d1==0 || d1==-1 || d1==1) {
      if (!pairComplete) {
	if (d1==-1 && d2==-1) {
	  // Have to bail without incrementing r
	  if (DEBUG) cerr << "Found -1 -1, returning r=" << r << "\n";
	  break;
	}
	r+=2;
	pairComplete=true;
      } else {
	pairComplete=false;
      }
      position++;
      priorValue=currentValue;
    } else {
      // Violated diff region
      break;
    }
  }
  if (DEBUG) cerr << "Done, returning r=" << r << "\n";
  if (DEBUG) fflush(stderr);
  return r;
}

// Returns [0] the index of the first repeat relative to the starting position, [1] the length of the first repeat. A length of 0 means there are no repeated values, a length of 1 means two values are the same, etc.
 void ImageLoaderBasic::pbd3FindFirstRepeatOfMinLength(unsigned char* sourceBuffer, V3DLONG position, V3DLONG searchLength, V3DLONG minLength, V3DLONG* returnBuffer)
{
  V3DLONG repeatCount=0;
  V3DLONG maxPosition=position+searchLength;
  bool foundQualifyingRepeat=false;
  returnBuffer[0]=0;
  returnBuffer[1]=0;
  if (searchLength<1) {
    return;
  } else {
    position++;
    while(position<maxPosition) {
      if (sourceBuffer[position]==sourceBuffer[position-1]) {
	repeatCount++;
	if (repeatCount>=minLength && !foundQualifyingRepeat) {
	  foundQualifyingRepeat=true;
	}
      } else {
	if (foundQualifyingRepeat) {
	  break;
	} else {
	  repeatCount=0;
	}
      }
      position++;
    }
    if (foundQualifyingRepeat) {
      returnBuffer[PBD_3_REPEAT_STARTING_POSITION]=position-repeatCount;
      returnBuffer[PBD_3_REPEAT_COUNT]=repeatCount;
    }
  }
  return;
}

 // When this method is called, we expect a diff region with the specified region to be viable.
unsigned char* ImageLoaderBasic::pbd3EncodeDiff(unsigned char* sourceBuffer, V3DLONG position, V3DLONG length, int* byteCount)
{
  if (DEBUG)  cerr << "ImageLoaderBasic::pbd3EncodeDiff start, length=" << length << "\n";
  unsigned char* resultBuffer=new unsigned char[PBD_3_MAX_DIFF_LENGTH]; // the actual length should be far less than this, since we are using 3 bits per 2 positions
  if (length<PBD_3_MIN_DIFF_LENGTH) {
    cerr << "Diff length cannot be encoded when less than " << PBD_3_MIN_DIFF_LENGTH << "\n";
    exit(1);
  }
  if (length % 2 != 0) {
    cerr << "pbd3 diff encoding must be an even length - length=" << length << "\n";
    exit(1);
  }
  V3DLONG doubletCount=length/2;
  unsigned char keyByte=doubletCount+22;
  unsigned char priorValue=0;
  if (position>0) {
    priorValue=sourceBuffer[position-1];
  }
  V3DLONG p=position;
  int d1=0;
  int d2=0;
  V3DLONG maxPosition=position+length;
  bool createEntry=false;
  int entryCount=0;
  resultBuffer[0]=keyByte;
  if (DEBUG_SUMMARY) cerr << "compress diff, doubletCount=" << doubletCount << "\n";
  if (DEBUG)  cerr << "Beginning main loop...\n";
  V3DLONG maxCurrentByte=0L;
  int currentByte=0;
  while (p < maxPosition) {
    int pVal=priorValue;
    unsigned char v=sourceBuffer[p];
    int cVal=v;
    if (DEBUG) cerr << "Top of diff loop, p=" << p << " priorValue=" << pVal << " currentValue=" << cVal << "\n";
    d2=d1;
    d1=v-priorValue;
    if (d1<-1 || d1>1) {
      cerr << "d1 out of bounds\n";
      exit(1);
    }
    priorValue=v;
    if (createEntry) {
      if (DEBUG) cerr << "Start createEntry\n";
      if (DEBUG) cerr << "d2=" << d2 << ", d1=" << d1 << "\n";
      currentByte=((entryCount*3)/8) + 1; // the +1 is for the keyByte
      if (currentByte>maxCurrentByte) {
	maxCurrentByte=currentByte;
	resultBuffer[currentByte]=0; // clear
      }
      int bitOffset=entryCount*3-8*(currentByte-1);
      if (bitOffset>7 || bitOffset<0) {
	cerr << "bitOffset should never be greater than 7 or less than 0: " << bitOffset << "\n";
	exit(1);
      }
      unsigned char entryByte=0;
      if (d2==0 && d1==0) {
	// zero-value
      } else if (d2==0 && d1==-1) {
	entryByte=oooooool;
      } else if (d2==-1 && d1==0) {
	entryByte=oooooolo;
      } else if (d2==-1 && d1==1) {
	entryByte=ooooooll;
      } else if (d2==0 && d1==1) {
	entryByte=oooooloo;
      } else if (d2==1 && d1==0) {
	entryByte=ooooolol;
      } else if (d2==1 && d1==-1) {
	entryByte=ooooollo;
      } else if (d2==1 && d1==1) {
	entryByte=ooooolll;
      } else if (d2==-1 && d1==-1) {
	cerr << "We should never see -1 -1 in the diff loop\n";
	exit(1);
      } else {
	cerr << "Diff values should be -1, 0, 1 : should never have d2=" << d2 << " and d1=" << d1 << "\n";
	exit(1);
      }
      if (DEBUG) cerr << "Done setting entryByte, currentByte=" << currentByte << " maxPosition=" << maxPosition << "\n";
      unsigned char eC=entryByte;
      entryByte <<= bitOffset;
      resultBuffer[currentByte] |= entryByte;
      if (bitOffset==6) {
	eC >>= 2;
	currentByte++;
	resultBuffer[currentByte]=0;
	maxCurrentByte=currentByte;
	resultBuffer[currentByte]=eC;
      } else if (bitOffset==7) {
	eC >>= 1;
	currentByte++;
	resultBuffer[currentByte]=0;
	maxCurrentByte=currentByte;
	resultBuffer[currentByte]=eC;
      }
      if (DEBUG) cerr << "Done setting resultBuffer\n";
      entryCount++;
      // The next position will be used to gather one difference, not to encode
      createEntry=false;
    } else {
      // When we add another difference with the next position, we will be ready to encode
      createEntry=true;
    }
    // Next: gradually pad-in the 3-bit representations of the differences
    p++;
  }
  if ((entryCount*3) % 8 == 0)  {
    *byteCount = (entryCount*3)/8 + 1;
  } else {
    *byteCount = (entryCount*3)/8 + 2; // of the +2, 1, is for the leftover diff byte, the other 1 is for the keyByte
  }
  if (currentByte!=(*byteCount-1)) {
    cerr << "currentByte " << currentByte << " does not equal byteCount-1: " << *byteCount << "\n";
    exit(1);
  }
  if (DEBUG) cerr << "pbd3 Encode Diff - entryCount=" << entryCount << ", byteCount=" << *byteCount << "\n";
  return resultBuffer;
}

void ImageLoaderBasic::pbd3FlushLiteral(unsigned char* compressionBuffer, unsigned char* sourceBuffer, V3DLONG* activeLiteralIndex, V3DLONG* pp, V3DLONG i) {
  //  cerr << "flush literal, i=" << i << "\n";
  if (*activeLiteralIndex>-1) {
    V3DLONG a=*activeLiteralIndex;
    V3DLONG aLength=i-a;
    if (aLength<1) {
      cerr << "Literal length must be at least 1\n";
      exit(1);
    }
    if (aLength>PBD_3_MAX_LITERAL) {
      cerr << "Can not support literal length greater than " << PBD_3_MAX_LITERAL;
      exit(1);
    }
    V3DLONG p = *pp;
    unsigned char keyByte=(unsigned char)aLength - 1;
    if (DEBUG_SUMMARY) cerr << "compress literal, count=" << aLength << "\n";
    compressionBuffer[p++]=keyByte; // key value, the minus one is for the implied -1
    V3DLONG bitOffset=0;
    unsigned char cByte=0;
    if (DEBUG) cerr << "Starting literal flush loop\n";
    while (a<i) {
      unsigned char v=sourceBuffer[a];
      if (v>7) {
	cerr << "Source value should never be greater than 7 since this is a 3-bit encoding\n";
	exit(1);
      }
      if (DEBUG) cerr << "a= " << a << " i=" << i << " v=" << v << "\n";
      if (bitOffset==0) {
	// Just assign
	cByte=v;
	bitOffset=3;
      } else if (bitOffset<6) {
	v <<= bitOffset;
	cByte |= v;
	bitOffset+=3;
	if (bitOffset==8) {

	  //	  if (DEBUG_SUMMARY) {
	  //	    int ci=cByte;
	  //	    cerr << " " << ci;
	  //	  }

	  compressionBuffer[p++]=cByte;
	  cByte=0;
	  bitOffset=0;
	}
      } else if (bitOffset==6) {
	unsigned char vCopy=v;
	v <<= bitOffset;
	v &= lloooooo;
	cByte |= v;

	//	if (DEBUG_SUMMARY) {
	//	  int ci=cByte;
	//	  cerr << " " << ci;
	//	}

	compressionBuffer[p++]=cByte;
	cByte=0;
	vCopy >>= 2;
	vCopy &= oooooool;
	cByte=vCopy;
	bitOffset=1;
      } else if (bitOffset==7) {
	unsigned char vCopy=v;
	v <<= bitOffset;
	v &= looooooo;
	cByte |= v;

	//	if (DEBUG_SUMMARY) {
	//	  int ci=cByte;
	//	  cerr << " " << ci;
	//	}

	compressionBuffer[p++]=cByte;
	cByte=0;
	vCopy >>= 1;
	vCopy &= ooooooll;
	cByte=vCopy;
	bitOffset=2;
      }
      a++;
    }
    if (bitOffset>0) {
      // Flush remainder

      //      if (DEBUG_SUMMARY) {
      //	int ci=cByte;
      //	cerr << " " << ci;
      //      }

      compressionBuffer[p++]=cByte;
    }
    *pp = p;
  }

  //  if (DEBUG_SUMMARY) {
  //    cerr << "\n";
  //  }

  *activeLiteralIndex=-1;
}


// This is the main decompression function, which is basically a wrapper for the decompression function
// of ImageLoaderBasic, except it determines the boundary of acceptable processing given the contents of the
// most recently read block. This function assumes it is called sequentially (i.e., not in parallel)
// and so doesn't have to check that the prior processing step is finished. updatedCompressionBuffer
// points to the first invalid data position, i.e., all previous positions starting with compressionBuffer
// are valid.
void ImageLoaderBasic::updateCompressionBuffer3(unsigned char * updatedCompressionBuffer) {

  DEBUG_SUMMARY=false;
  V3DLONG uP=(long long)updatedCompressionBuffer;
  //  cerr << "updateCompressionBuffer3 - updatedCompressionBuffer=" << uP << "\n";

    if (compressionPosition==0) {
      //      for (int j=0;j<50;j++) {
      //	int ci=compressionBuffer[j];
      //	cerr << "cp=" << j << " value=" << ci << " hex=" << std::hex << ci << std::dec << "\n";
      //      }
      // Just starting
      compressionPosition=&compressionBuffer[0];
      int channelCount=pbd_sz[3];
      cerr << "channelCount=" << channelCount << "\n";
      pbd3_source_min = new unsigned char[channelCount];
      pbd3_source_max = new unsigned char[channelCount];
      for (int i=0;i<channelCount;i++) {
	pbd3_source_min[i]=compressionBuffer[i*2];
	pbd3_source_max[i]=compressionBuffer[i*2+1];
	int iMin=pbd3_source_min[i];
	int iMax=pbd3_source_max[i];
	cerr << "Using channel " << i << " pbd3 min=" << iMin << " max=" << iMax << "\n";
      }
      compressionPosition+=(channelCount*2);
    }
    pbd3_current_min=pbd3_source_min[pbd3_current_channel];
    pbd3_current_max=pbd3_source_max[pbd3_current_channel];
    unsigned char * lookAhead=compressionPosition;
    while(lookAhead<updatedCompressionBuffer) {
      unsigned char lav=*lookAhead;
      // We will keep going until we find nonsense or reach the end of the block
      if (lav<24) {
	// Literal values - the actual number of following literal values
	// is equal to the lav+1, so that if lav==23, there are 24 following
	// literal values. For pbd3, the number of bytes encoded will be:
	int debugLiteralCount=(lav+1);
	if (DEBUG_SUMMARY) cerr << "pre: literal count=" << debugLiteralCount << "\n";
	int impliedBits=(lav+1)*3;
	unsigned char * trialPosition=0;
	if (impliedBits % 8 == 0) {
	  trialPosition = impliedBits/8 + lookAhead;
	} else {
	  trialPosition = (impliedBits/8 + 1) + lookAhead;
	}
	if ( trialPosition < updatedCompressionBuffer ) {
	  // Then we can process the whole literal section - we can move to next position
	  lookAhead = trialPosition+1;
	} else {
	  break; // leave lookAhead in current maximum position
	}
      } else if (lav<128) {
	// Difference section. The number of difference doublets is equal to lav-22, so that
	// if lav==24, the minimum, there will be 2 difference doublets.
	int doublets=lav-22;
	if (DEBUG_SUMMARY) cerr << "pre: doublet count=" << doublets << "\n";
	int impliedBits = doublets*3;
	unsigned char * trialPosition=0;
	int dBytes=0;
	if (impliedBits % 8 == 0) {
	  trialPosition = impliedBits/8 + lookAhead;
	  dBytes = impliedBits/8 + 1;
	} else {
	  trialPosition = (impliedBits/8 + 1) + lookAhead;
	  dBytes = impliedBits/8 + 2;
	}
	//	if (DEBUG_SUMMARY) {
	//	  if (doublets==31) {
	//	    unsigned char * dd = lookAhead;
	//	    int testDi1=*(dd+1);
	//	    int testDi2=*(dd+2);
	//	    int testDi3=*(dd+3);
	//	    if (testDi1==32 && testDi2==38 && testDi3==16) {
	//	      cerr << "=========== Found doublet 31 with first bytes 32, 38, 16\n";
	//	      for (; dd < (lookAhead + dBytes); dd++) {
	//		int di=*dd;
	//		cerr << "byte = " << di << "\n";
	//	      }
	//	      exit(1);
	//	    }
	//	  }
	  //	}
	if ( trialPosition < updatedCompressionBuffer ) {
	  // We can process this section, so advance to next position to evaluate
	  lookAhead = trialPosition+1;
	} else {
	  break; // leave in current max position
	}
      } else {
	// Repeat section. Takes two bytes.
	unsigned char* trialPosition = lookAhead + 1;
	if (DEBUG_SUMMARY) {
	  unsigned char key=*lookAhead;
	  unsigned char value=*(lookAhead+1);
	  unsigned char repeatValue=0;
	  int repeatCount=pbd3GetRepeatCountFromBytes(key, value, &repeatValue);
	  int rv=repeatValue;
	  cerr << "pre: repeatCount=" << repeatCount << " value=" << rv << "\n";
	}
	if (trialPosition < updatedCompressionBuffer) {
	  lookAhead = trialPosition + 1;
	} else {
	  break; // leave in current max position
	}
      }
    }
    // At this point, lookAhead is in an invalid position, which if equal to updatedCompressionBuffer
    // means the entire compressed update can be processed.
    V3DLONG compressionLength=lookAhead-compressionPosition;
    if (decompressionPosition==0) {
        // Needs to be initialized
        decompressionPosition=decompressionBuffer;
    }
    //qDebug() << "updateCompressionBuffer calling decompressPBD compressionPosition=" << compressionPosition << " decompressionPosition=" << decompressionPosition
    //        << " size=" << compressionLength << " previousTotalDecompSize=" << getDecompressionSize() << " maxDecompSize=" << maxDecompressionSize;

    if (DEBUG_SUMMARY) {
      V3DLONG cStart=(long long)&compressionBuffer[0];
      V3DLONG cCurrent=(long long)compressionPosition;
      V3DLONG cOffset=cCurrent-cStart;
      cerr << "Compression Buffer Offset=" << cOffset << " " << std::hex << cOffset << std::dec << "\n";
    }

    V3DLONG dlength=decompressPBD3(compressionPosition, decompressionPosition, compressionLength);
    compressionPosition=lookAhead;
    decompressionPosition+=dlength;
    //    if (DEBUG_SUMMARY) exit(1);
    //printf("d2\n");
}

V3DLONG ImageLoaderBasic::decompressPBD3(unsigned char * sourceData, unsigned char * targetData, V3DLONG sourceLength) {

  V3DLONG cp=0;
  V3DLONG dp=0;
  unsigned char value=0;
  int sMax=pbd3_current_max;
  int sMin=pbd3_current_min;
  int sourceRange = sMax - sMin;
  unsigned char bitOffset=0;
  unsigned char currentByte=sourceData[cp];

  V3DLONG dStart=(long long)decompressionBuffer;
  V3DLONG tStart=(long long)targetData;

  V3DLONG i=0L;

  V3DLONG sdStart=(long long)sourceData;
  V3DLONG sdEnd=sdStart + sourceLength;

  unsigned char valueByLevel[8];

  for (int ii=0;ii<8;ii++) {
    valueByLevel[ii]=(unsigned char)(sMin + (sourceRange*ii)/7);
    int pv=valueByLevel[ii];
  }

  int iv=0;

  if (DEBUG_SUMMARY) cerr << "==== decompressPBD3 ========================================\n";

  while(cp<sourceLength) {

    i = dp+(tStart-dStart);

    //    if (i>388514000) {
    //      DEBUG=true;
    //      DEBUG_SUMMARY=true;
    //    }

    if (DEBUG) {
      cerr << "i=" << i << "\n";
    }

        if (isCanceled())
            return dp;

	if (DEBUG) {
	  iv=sourceData[cp];
	  cerr << "Value Pre=" << iv << "\n";
	}

	if (bitOffset>0) {
	  cp++;
	  bitOffset=0;
	  if (cp>=sourceLength) {
	    break;
	  }
	}

        value=sourceData[cp++];

	if (DEBUG) {
	  iv = value;
	  cerr << "Value Post=" << iv << "\n";
	}

	// Debug
	int ic=0;

        if (value<24) {
            // Literal 0-23
	  bitOffset=0; // clear
            currentByte=sourceData[cp];

	    if (DEBUG) {
	        ic=currentByte;
		cerr << " " << ic;
	    }

            unsigned char count=value+1;
	    int iCount=count;

	    if (DEBUG_SUMMARY) {
	      cerr << "decompress literal, count=" << iCount << " i=" << i << "\n";
	    }

	    int l=0;
	    while (l<count) {
	      if (bitOffset>7) {
		cp++;
		currentByte=sourceData[cp];

		if (DEBUG) {
		  ic=currentByte;
		  cerr << " " << ic;
		}

		bitOffset-=8;
	      }
	      if (bitOffset<6) {
		unsigned char mask=ooooolll;
		mask <<= bitOffset;
		unsigned char v=currentByte & mask;
		v >>= bitOffset;
		targetData[dp++]=valueByLevel[v];
		bitOffset+=3;
		l++;
	      } else if (bitOffset==6) {
		unsigned char nextByte=sourceData[cp+1];
		unsigned char mask=lloooooo;
		unsigned char v=currentByte & mask;
		v >>= 6;
		mask=oooooool;
		unsigned char v2 = nextByte & mask;
		v2 <<= 2;
		v |= v2;
		targetData[dp++]=valueByLevel[v];
		bitOffset+=3;
		l++;
	      } else if (bitOffset==7) {
		unsigned char nextByte=sourceData[cp+1];
		unsigned char mask=looooooo;
		unsigned char v=currentByte & mask;
		v >>= 7;
		mask=ooooooll;
		unsigned char v2 = nextByte & mask;
		v2 <<= 1;
		v |= v2;
		targetData[dp++]=valueByLevel[v];
		bitOffset+=3;
		l++;
	      }
	    }

	    if (DEBUG) {
	      cerr << "\n";
	    }

	    if (bitOffset>8) {
	      cp++;
	    }

        } else if (value<128) {
	  // Difference 24-128
	  bitOffset=0; // clear
	  unsigned char previousValue=0;
	  if (!(dp==0 && (tStart==dStart))) {
	    for (int fl=0;fl<8;fl++) {
	      if (targetData[dp-1]==valueByLevel[fl]) {
		previousValue=fl;
	      }
	    }
	  }
	  //	  int pvi=previousValue;
	  //	  cerr << "Starting pv=" << pvi << "\n";

	  int doubletCount = value-22;

	  if (DEBUG_SUMMARY) {
	    cerr << "decompress diff, doubletCount=" << doubletCount << " i=" << i << "\n";
	    int totalBytes=0;
	    if ( (doubletCount*3)/8 % 8 == 0) {
	      totalBytes=(doubletCount*3)/8;
	    } else {
	      totalBytes=(doubletCount*3)/8+1;
	    }
	    for (int j=0;j<totalBytes;j++) {
	      int jv=sourceData[cp+j];
	      cerr << "byte " << j << " = " << jv << " " << std::hex << jv << std::dec << "\n";
	    }
	  }

	  unsigned char currentByte=sourceData[cp];

	  int v2c=0;
	  int vC=0;
	  unsigned char v=0;

	  for (int d=0;d<doubletCount;d++) {
	    int bo=bitOffset;
	    if (bitOffset>7) {
	      cp++;
	      bitOffset-=8;
	      currentByte=sourceData[cp];
	    }
	    int cB=currentByte;
	    int nB=sourceData[cp+1];
	    int bO=bitOffset;
	    if (DEBUG_SUMMARY) cerr << "   currentByte=" << cB << "  nextByte=" << nB << " bitOffset=" << bO << "\n";
	    if (bitOffset<6) {
	      unsigned char mask=ooooolll;
	      mask <<= bitOffset;
	      v=currentByte & mask;
	      v >>= bitOffset;
	    } else if (bitOffset==6) {
	      unsigned char nextByte=sourceData[cp+1];
	      unsigned char mask=lloooooo;
	      v=currentByte & mask;
	      v >>= 6;
	      mask=oooooool;
	      unsigned char v2 = nextByte & mask;
	      v2 <<= 2;
	      v |= v2;
	    } else if (bitOffset==7) {
	      unsigned char nextByte=sourceData[cp+1];
		unsigned char mask=looooooo;
		v=currentByte & mask;
		v >>= 7;
		mask=ooooooll;
		unsigned char v2 = nextByte & mask;
		vC=v;
		v2 <<= 1;
		v2c=v2;
		unsigned char vt=v;
		if (DEBUG_SUMMARY) cerr << " bitOffset 7 check: v2=" << v2c << " v=" << vC << "\n";
		v = vt | v2;
		vC=v;
		if (DEBUG_SUMMARY) cerr << " bitOffset 7 check: v=" << vC << "\n";
	    }

	    //	    targetData[dp++]=valueByLevel[0];
	    //	    targetData[dp++]=valueByLevel[0];

	    int pvC=previousValue;
	    if (DEBUG_SUMMARY) cerr << " previousValue= " << pvC << "\n";

	    if (v==0) {        // 0, 0
	      if (DEBUG_SUMMARY) {
		cerr << "0, 0\n";
	      }
	      targetData[dp++]=valueByLevel[previousValue];
	      targetData[dp++]=valueByLevel[previousValue];
	    } else if (v==1) { // 0, -1
	      if (DEBUG_SUMMARY) {
		cerr << "0, -1\n";
	      }
	      targetData[dp++]=valueByLevel[previousValue];
	      previousValue--;
	      targetData[dp++]=valueByLevel[previousValue];
	    } else if (v==2) { // -1, 0
	      if (DEBUG_SUMMARY) {
		cerr << "-1, 0\n";
	      }
	      previousValue--;
	      targetData[dp++]=valueByLevel[previousValue];
	      targetData[dp++]=valueByLevel[previousValue];
	    } else if (v==3) { // -1, 1
	      if (DEBUG_SUMMARY) {
		cerr << "-1, 1\n";
	      }
	      previousValue--;
	      targetData[dp++]=valueByLevel[previousValue];
	      previousValue++;
	      targetData[dp++]=valueByLevel[previousValue];
	    } else if (v==4) { // 0, 1
	      if (DEBUG_SUMMARY) {
		cerr << "0, 1\n";
	      }
	      targetData[dp++]=valueByLevel[previousValue];
	      previousValue++;
	      targetData[dp++]=valueByLevel[previousValue];
	    } else if (v==5) { // 1, 0
	      if (DEBUG_SUMMARY) {
		cerr << "1, 0\n";
	      }
	      previousValue++;
	      targetData[dp++]=valueByLevel[previousValue];
	      targetData[dp++]=valueByLevel[previousValue];
	    } else if (v==6) { // 1, -1
	      if (DEBUG_SUMMARY) {
		cerr << "1, -1\n";
	      }
	      previousValue++;
	      targetData[dp++]=valueByLevel[previousValue];
	      previousValue--;
	      targetData[dp++]=valueByLevel[previousValue];
	    } else if (v==7) { // 1, 1
	      if (DEBUG_SUMMARY) {
		cerr << "1, 1\n";
	      }
	      previousValue++;
	      targetData[dp++]=valueByLevel[previousValue];
	      previousValue++;
	      targetData[dp++]=valueByLevel[previousValue];
	    } else {
	      cerr << "v should never be out of the range of 0-7\n";
	      exit(1);
	    }

	    if (previousValue>7) {

	      i = dp+(tStart-dStart);

	      int pv=previousValue;
	      cerr << "Previous value went out of range, =" << pv << " i=" << i << "\n";
	      exit(1);
	    }

	    bitOffset+=3;
	  }

	  if (bitOffset>8) {
	    cp++;
	  }

        } else {
	  // Repeat 128-255
	  unsigned char repeatValue=0;
	  int repeatCount = pbd3GetRepeatCountFromBytes(value, sourceData[cp++], &repeatValue);
	  V3DLONG iCheck=dp+(tStart-dStart);

	  if (DEBUG_SUMMARY) {
	    int ik=value;
	    int iv=sourceData[cp];
	    cerr << "decompress repeat, count=" << repeatCount << " keyByte=" << ik << " valueByte=" << iv << " i=" << iCheck << "\n";
	  }

	  for (int ri=0;ri<repeatCount;ri++) {
	    targetData[dp++]=valueByLevel[repeatValue];
	  }
        }
    }

    i=dp+(tStart-dStart);

    //    if (DEBUG_SUMMARY) cerr << "Decompression size=" << i << "\n";

    return dp;
}

int ImageLoaderBasic::pbd3GetRepeatCountFromBytes(unsigned char keyByte, unsigned char valueByte, unsigned char* repeatValue) {
  // Repeat 128-255
  int ik=keyByte;
  int iv=valueByte;
  unsigned char b0=keyByte-128;
  unsigned char mask=oooooool;
  unsigned char b1=valueByte & mask;
  b1 <<= 7;
  b0 |= b1;
  mask = 30; // ooollllo
  b1 = valueByte & mask;
  b1 >>= 1;
  valueByte >>= 5;
  *repeatValue=valueByte;
  int repeatCount=0;
  unsigned char* rp = (unsigned char*)(&repeatCount);
  rp[0]=b0;
  rp[1]=b1;
  if (repeatCount<0 || repeatCount > 4095) {
    cerr << "Repeat count should not be " << repeatCount << "\n";
    exit(1);
  }
  repeatCount++;
  return repeatCount;
}

