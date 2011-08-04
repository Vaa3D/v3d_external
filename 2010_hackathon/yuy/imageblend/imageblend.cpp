/* imageblend.cpp
 * 2011-07-30: the program is created by Yang Yu
 */


#include <QtGui>

#include <cmath>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <iostream>

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

extern "C" {
#include "tiffio.h"
};

#include "imageblend.h"

using namespace std;

#define EMPTY 1

// handle .lsm file
// following the rules from ITK .lsm reader
// Structure with LSM-specific data ( only in the first image directory).
#define TIF_CZ_LSMINFO 34412
#define TIF_CZ_LSMINFO_SIZE_RESERVED 90+6
#define TIF_CZ_LSMINFO_SIZE 512

typedef int			Int32_t;
typedef unsigned	UInt32_t;

typedef float       Float32_t;
typedef double      Float64_t;
typedef long double Float96_t;

typedef struct {
	UInt32_t    U32MagicNumber;
	Int32_t     S32StructureSize;
	Int32_t     S32DimensionX;
	Int32_t     S32DimensionY;
	Int32_t     S32DimensionZ;
	Int32_t     S32DimensionChannels;
	Int32_t     S32DimensionTime;
	Int32_t     S32DataType;
	Int32_t     S32ThumbnailX;
	Int32_t     S32ThumbnailY;
	Float64_t   F64VoxelSizeX;
	Float64_t   F64VoxelSizeY;
	Float64_t   F64VoxelSizeZ;
	UInt32_t    u32ScanType;
	UInt32_t    u32DataType;
	UInt32_t    u32OffsetVectorOverlay;
	UInt32_t    u32OffsetInputLut;
	UInt32_t    u32OffsetOutputLut;
	UInt32_t    u32OffsetChannelColors;
	Float64_t   F64TimeIntervall;
	UInt32_t    u32OffsetChannelDataTypes;
	UInt32_t    u32OffsetScanInformation;
	UInt32_t    u32OffsetKsData;
	UInt32_t    u32OffsetTimeStamps;
	UInt32_t    u32OffsetEventList;
	UInt32_t    u32OffsetRoi;
	UInt32_t    u32OffsetBleachRoi;
	UInt32_t    u32OffsetNextRecording;
	UInt32_t    u32Reserved [ TIF_CZ_LSMINFO_SIZE_RESERVED ];
} zeiss_info; // itkLSMImageIO.cxx

bool loadLSM(const char *filename, unsigned char *&p1dImg, V3DLONG *&szImg, int &datatypeImg)
{    
    // load header
    TIFF  *tif=NULL;
    int   depth, width, height, kind, datatype;
    
    // 
    TIFFSetWarningHandler(NULL);
    if( (tif = TIFFOpen(const_cast<char *>(filename),"r")) == NULL )
    {
        printf("Could not open incoming image\n");
        return false;
    }
    
    depth = 1;
    while(TIFFReadDirectory(tif))
        depth += 1;
    TIFFClose(tif);
    depth = depth / 2;		/* half the dirs are thumbnails */
    
    TIFFSetWarningHandler(NULL);
    if( (tif = TIFFOpen(const_cast<char *>(filename),"r")) == NULL )
    {
        printf("Could not open incoming image\n");
        return false;
    }
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    
    short bits, channels;
    
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);
    
    if (bits<=8 && bits>0) datatype=1;
    else if (bits<=16 && bits>0) datatype=2;
    else
    {
        printf("LSM file should not support more than 16 bits data.\n");
        return false;
    }
    
    // resolutions
    ttag_t tag = TIF_CZ_LSMINFO;
    void *praw = NULL;
    short value_count = 512; // TIF_CZ_LSMINFO_SIZE
    const TIFFFieldInfo *fld = NULL;
    
    if( (fld = TIFFFieldWithTag( tif, tag )) == NULL )
    {
        printf("Fail in reading .lsm header info!\n");
        return false;
    }
    else
    {
        if( fld->field_passcount )
        {
            if( TIFFGetField( tif, tag, &value_count, &praw ) != 1 )
            {
                printf("Tag of .lsm cannot be found!\n");
                return false;
            }
            else
            {
                if( fld->field_type != TIFF_BYTE )
                {
                    printf("Tag of .lsm is not of type TIFF_BYTE!\n");
                    return false;
                }
            }
        }
    }
    
    const zeiss_info *zi = reinterpret_cast<zeiss_info*>(praw);
    
    if( sizeof(*zi) != TIF_CZ_LSMINFO_SIZE)
    {
        printf("Problem of alignement of reading header of .lsm file on your platform.\n");
        return false;
    }
    double res_x, res_y, res_z;
    res_x = zi->F64VoxelSizeX;
    res_y = zi->F64VoxelSizeY;
    // TIFF only support 2 or 3 dimension:
    if ( depth > 1 )
    {
        res_z = zi->F64VoxelSizeZ;
    }
    
    //
    TIFFClose(tif);
    
    // assign
    szImg = new V3DLONG [4];
    szImg[0] = width; szImg[1] = height; szImg[2] = depth; szImg[3] = channels; datatypeImg = datatype;
    
    // loading
    V3DLONG dims_x, dims_y, dims_z, dims_c, sz_frame, sz_channel;
    dims_x = width; dims_y = height; dims_z = depth;
    sz_frame = dims_x*dims_y;
    sz_channel = dims_x*dims_y*dims_z;
    
    try {
        p1dImg = new unsigned char [sz_channel*channels*datatype];
    } catch (...) {
        cout<<"Unable to allocate memory for lsm image!"<<endl;
        return false;
    }
    
    TIFFSetWarningHandler(NULL);
    if( (tif = TIFFOpen(const_cast<char *>(filename),"r")) == NULL )
    {
        printf("Could not open incoming image\n");
        return false;
    }
    
    for(V3DLONG nChannel=0; nChannel<channels; nChannel++)
    {
        V3DLONG offset_c = nChannel*sz_channel;
        for(V3DLONG nFrame=0; nFrame<dims_z; nFrame++)
        {
            V3DLONG offsets = offset_c + nFrame*sz_frame;
            
            if (!TIFFReadDirectory(tif)) break;	  // skip the one we just read, it's a thumbnail 
            if (!TIFFReadDirectory(tif)) break;	  // get the next slice
            
            // load frame
            if (TIFFIsTiled(tif))
            {
                // File is tiled
                uint32 *bytecounts;
                TIFFGetField(tif, TIFFTAG_TILEBYTECOUNTS, &bytecounts);
                
                ttile_t t, nt = TIFFNumberOfTiles(tif);
                for (t = 0; t < nt; t++)
                {
                    if(TIFFReadEncodedTile(tif, t, (unsigned char *)p1dImg + offsets*datatype + (V3DLONG)t*sz_channel, bytecounts[t])<0)
                        return -1 ;
                }
            }
            else
            {
                // File is striped 
                tstrip_t s, ns = TIFFNumberOfStrips(tif);
                for (s = 0; s < ns; s++)
                {
                    if(TIFFReadEncodedStrip(tif, s, (unsigned char *)p1dImg + offsets*datatype + (V3DLONG)s*sz_channel*datatype, sz_frame*datatype)<0)
                        continue;
                }
            }
        }
    }
    
    //
    TIFFClose(tif);
    
    return true;
}

//
Q_EXPORT_PLUGIN2(imageBlend, ImageBlendPlugin);

// func mutual information for pair images with the same size
template <class Tdata>
double mi_computing(Tdata *pImg1, Tdata *pImg2, V3DLONG szImg, int datatype)
{
    size_t start_t = clock();
    
    // joint histogram
    double **jointHistogram = NULL;
    double *img1Hist=NULL;
    double *img2Hist=NULL;
    
    V3DLONG szHist;
    
    if(datatype==1) // 8-bit UINT8
    {
        szHist = 256;
    }
    else if(datatype==2) // 12-bit UINT16
    {
        szHist = 4096;
    }
    else
    {
        cout<<"Datatype is not supported!"<<endl;
        return -1;
    }
    
    V3DLONG denomHist = szHist*szHist;
    
    try
    {
        jointHistogram = new double * [szHist];
        for(int i=0; i<szHist; i++)
        {
            jointHistogram[i] = new double [szHist];
            memset(jointHistogram[i], 0, sizeof(double)*szHist);
        }
        
        img1Hist = new double [szHist];
        img2Hist = new double [szHist];
        
        memset(img1Hist, 0, sizeof(double)*szHist);
        memset(img2Hist, 0, sizeof(double)*szHist);
    }
    catch(...)
    {
        qDebug()<<"Fail to allocate memory for joint histogram!";
        return -1;
    }
    
    //
    for(V3DLONG i=0; i<szImg; i++)
    {
        jointHistogram[ (V3DLONG)pImg1[i] ][ (V3DLONG)pImg2[i] ] ++;  
    }
    
    double jointEntropy=0, img1Entropy=0, img2Entropy=0;
    
    // normalized joint histogram
    for(V3DLONG i=0; i<szHist; i++)
    {
        for(V3DLONG j=0; j<szHist; j++)
        {
            jointHistogram[i][j] /= (double)(denomHist);
            
            double val = jointHistogram[i][j]?jointHistogram[i][j]:1;
            
            jointEntropy += val * log2(val);
        }
    }

    // marginal histogram
    for(V3DLONG i=0; i<szHist; i++)
    {
        for(V3DLONG j=0; j<szHist; j++)
        {
            img1Hist[i] += jointHistogram[i][j];
            img2Hist[i] += jointHistogram[j][i];
        } 
    }
    
    for(V3DLONG i=0; i<szHist; i++)
    {
        double val1 = img1Hist[i]?img1Hist[i]:1;
        double val2 = img2Hist[i]?img2Hist[i]:1;
        
        img1Entropy += val1 * log2(val1);
        img2Entropy += val2 * log2(val2);
    }
    
    // de-alloc
    if(jointHistogram) {
        for(int i=0; i<szHist; i++)
        {
            delete[] jointHistogram[i];
        }     
        delete []jointHistogram; jointHistogram=NULL;
    }
    if(img1Hist) {delete []img1Hist; img1Hist=NULL;}
    if(img2Hist) {delete []img2Hist; img2Hist=NULL;}
    
    qDebug() << "time elapse for computing mutual information ... " <<clock()-start_t;
    
    // MI
    return (jointEntropy - img1Entropy - img2Entropy);    
    
}

//plugin
const QString title = "Image Blending";

// funcs
QStringList ImageBlendPlugin::funclist() const
{
	return QStringList() << "imageblend";
}

bool ImageBlendPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent)
{
    //
    if (func_name == tr("imageblend"))
	{
        if(input.size()<1) return false; // no inputs
        
        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty()) 
        {
            //print Help info
            printf("\nUsage: v3d -x imageBlend.dylib -f imageblend -i <input_images> -o <output_image> -p \"#s <save_blending_result zero(false)/nonzero(true)>\" \n");
            
            return true;
        }
        
        if(infilelist->size()<2)
        {
            printf("\nThe image blending program needs two images as an input!\n");
            
            return false;
        }
        
        qDebug()<<"input files ..."<<infilelist->at(0)<<infilelist->at(1);
        
        char * infile = infilelist->at(0); // input images
        char * paras = NULL; // parameters
        char * outfile = NULL; // outputs
        
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        
        bool b_saveimage = true; // save the blended image by default
        
        qDebug()<<"parameters ..."<<paras;
        
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[200];
            char * myparas = new char[len];
            strcpy(myparas, paras);
            for(int i = 0; i < len; i++)
            {
                if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            char ** argv = new char* [argc];
            for(int i = 0; i < argc; i++)
            {
                argv[i] = myparas + posb[i];
            }
            for(int i = 0; i < len; i++)
            {
                if(myparas[i]==' ' || myparas[i]=='\t') 
                    myparas[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];
                    
                    qDebug()<<">>key ..."<<key;
                    
                    if (*key == '#')
                    {
                        while(*++key)
                        {                            
                            if (!strcmp(key, "s"))
                            {                                
                                b_saveimage = atoi( argv[i+1] )?true:false;                                
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                            
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                    
                }
            }
        }
        
        QString blendImageName;
        QString m_InputFileName(infile);
        m_InputFileName.chop(4);
        if(!outfile) 
            blendImageName = m_InputFileName + "_blended.raw";
        else
            blendImageName = QString(outfile);
        
        if(QFileInfo(blendImageName).suffix().toUpper() != "RAW")
        {
            blendImageName.append(".raw"); // force to save as .raw file
        }
        
        // image blending
        QString m_InputFileName1(infilelist->at(0));
        QString m_InputFileName2(infilelist->at(1));
        
        // load images
        V3DLONG *sz_img1 = 0; 
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;
        
        if(QFileInfo(m_InputFileName1).suffix().toUpper().compare("LSM") == 0)
        {
            if (loadLSM(const_cast<char *>(m_InputFileName1.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1)!=true)
            {
                fprintf (stderr, "Error happens in reading the image1 file [%s]. Exit. \n",m_InputFileName1.toStdString().c_str());
                return false;
            }
        }
        else
        {
            if (loadImage(const_cast<char *>(m_InputFileName1.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1)!=true)
            {
                fprintf (stderr, "Error happens in reading the image1 file [%s]. Exit. \n",m_InputFileName1.toStdString().c_str());
                return false;
            }
        }
        
        V3DLONG *sz_img2 = 0; 
        int datatype_img2 = 0;
        unsigned char* p1dImg2 = 0;
        
        if(QFileInfo(m_InputFileName2).suffix().toUpper().compare("LSM") == 0)
        {
            if (loadLSM(const_cast<char *>(m_InputFileName2.toStdString().c_str()), p1dImg2, sz_img2, datatype_img2)!=true)
            {
                fprintf (stderr, "Error happens in reading the image1 file [%s]. Exit. \n",m_InputFileName2.toStdString().c_str());
                return false;
            }
        }
        else
        {
            if (loadImage(const_cast<char *>(m_InputFileName2.toStdString().c_str()), p1dImg2, sz_img2, datatype_img2)!=true)
            {
                fprintf (stderr, "Error happens in reading the image1 file [%s]. Exit. \n",m_InputFileName2.toStdString().c_str());
                return false;
            }
        }
        
        // check dims datatype
        if(datatype_img1 != datatype_img2)
        {
            cout<<"Images are different data types! Do nothing!"<<endl;
            return false;
        }
        
        if(sz_img1[0] != sz_img2[0] || sz_img1[1] != sz_img2[1] || sz_img1[2] != sz_img2[2] ) // x, y, z
        {
            cout<<"Images are different dimensions! Do nothing!"<<endl;
            return false;
        }
        
        //
        V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
        
        // find reference : suppose reference color channels similar enough
        V3DLONG ref1=0, ref2=0, nullcolor1 = -1, nullcolor2 = -1;
        bool b_img1existNULL=false, b_img2existNULL=false;
        
        // step 1: find null color channel
        for(V3DLONG c=0; c<sz_img1[3]; c++) // image 1
        {
            V3DLONG offset_c = c*pagesz;
            V3DLONG sumint1 = 0;
            for (V3DLONG k=0; k<sz_img1[2]; k++) 
            {
                V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                for(V3DLONG j=0; j<sz_img1[1]; j++)
                {
                    V3DLONG offset_j = offset_k + j*sz_img1[0];
                    for(V3DLONG i=0; i<sz_img1[0]; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        
                        if(datatype_img1 == V3D_UINT8)
                        {
                            sumint1 += p1dImg1[idx];
                        }
                        else if(datatype_img1 == V3D_UINT16)
                        {
                            sumint1 += ((unsigned short *)p1dImg1)[idx];
                        }
                        else if(datatype_img1 == V3D_FLOAT32)
                        {
                            sumint1 += ((float *)p1dImg1)[idx];
                        }
                        else
                        {
                            cout<<"Your image datatype is not supported!"<<endl;
                            return false;
                        }
                    }
                }
            }
            
            qDebug()<<"sum ..."<<sumint1<<c;
            
            if(sumint1<EMPTY)
            {
                b_img1existNULL = true;
                nullcolor1 = c;
            }        
        }
        
        for(V3DLONG c=0; c<sz_img2[3]; c++) // image 2
        {
            V3DLONG offset_c = c*pagesz;
            V3DLONG sumint2 = 0;
            for (V3DLONG k=0; k<sz_img1[2]; k++) 
            {
                V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                for(V3DLONG j=0; j<sz_img1[1]; j++)
                {
                    V3DLONG offset_j = offset_k + j*sz_img1[0];
                    for(V3DLONG i=0; i<sz_img1[0]; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        
                        if(datatype_img1 == V3D_UINT8)
                        {
                            sumint2 += p1dImg2[idx];
                        }
                        else if(datatype_img1 == V3D_UINT16)
                        {
                            sumint2 += ((unsigned short *)p1dImg2)[idx];
                        }
                        else if(datatype_img1 == V3D_FLOAT32)
                        {
                            sumint2 += ((float *)p1dImg2)[idx];
                        }
                        else
                        {
                            cout<<"Your image datatype is not supported!"<<endl;
                            return false;
                        }
                    }
                }
            }
            
            qDebug()<<"sum ..."<<sumint2<<c;
            
            if(sumint2<EMPTY)
            {
                b_img2existNULL = true;
                nullcolor2 = c;
            }
        }
        
        // step 2: find ref color channel by compute MI
        double scoreMI = -1e10; // -INF
        for(V3DLONG c1=0; c1<sz_img1[3]; c1++)
        {
            if(b_img1existNULL)
            {
                if(c1==nullcolor1) continue;
            }
            
            for(V3DLONG c2=0; c2<sz_img2[3]; c2++)
            {
                if(b_img2existNULL)
                {
                    if(c2==nullcolor2) continue;
                }
                
                if(datatype_img1 == V3D_UINT8)
                {
                    unsigned char* pImg1Proxy = p1dImg1 + c1*pagesz;
                    unsigned char* pImg2Proxy = p1dImg2 + c2*pagesz;
                    
                    double valMI = mi_computing<unsigned char>(pImg1Proxy, pImg2Proxy, pagesz, 1);
                    
                    if(valMI>scoreMI)
                    {
                        scoreMI = valMI;
                        
                        ref1 = c1;
                        ref2 = c2;
                    }
                }
                else if(datatype_img1 == V3D_UINT16)
                {
                    unsigned short* pImg1Proxy = ((unsigned short *)p1dImg1) + c1*pagesz;
                    unsigned short* pImg2Proxy = ((unsigned short *)p1dImg2) + c2*pagesz;
                    
                    double valMI = mi_computing<unsigned short>(pImg1Proxy, pImg2Proxy, pagesz, 2);
                    
                    qDebug()<<"mi ..."<<valMI<<c1<<c2;
                    
                    if(valMI>scoreMI)
                    {
                        scoreMI = valMI;
                        
                        ref1 = c1;
                        ref2 = c2;
                    }
                }
                else if(datatype_img1 == V3D_FLOAT32)
                {
                    printf("Currently this program dose not support FLOAT32.\n"); // temporary
                    return false;
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return false;
                }
                
            }
        }
        
        qDebug()<<"ref ..."<<ref1<<ref2<<"null color ..."<<b_img1existNULL<<nullcolor1<<b_img2existNULL<<nullcolor2;
        
        // image blending	
        // suppose image1 and image2 have a common reference
        // the blended image color dim = image1 color dim + image2 color dim - 1
        V3DLONG colordim = sz_img1[3]+sz_img2[3]-1;
        
        if(b_img1existNULL) colordim--;
        if(b_img2existNULL) colordim--;
        
        V3DLONG sz_blend[4];
        sz_blend[0] = sz_img1[0]; sz_blend[1] = sz_img1[1]; sz_blend[2] = sz_img1[2]; sz_blend[3] = colordim;
        
        V3DLONG totalplxs = colordim * pagesz;
        
        if(datatype_img1 == V3D_UINT8)
        {
            //
            unsigned char* data1d = NULL;
            try
            {
                data1d = new unsigned char [totalplxs];
                
                memset(data1d, 0, sizeof(unsigned char)*totalplxs);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            
            //
            V3DLONG c1=0, c2=0;
            
            for(V3DLONG c=0; c<colordim-1; c++)
            {
                V3DLONG offset_c = c*pagesz;
                
                V3DLONG offset_c1, offset_c2;
                bool b_img1;
                
                if(c1<sz_img1[3])
                {
                    b_img1 = true;
                    
                    if(b_img1existNULL)
                    {
                        if(c1!=nullcolor1)
                        {
                            offset_c1 = c1*pagesz;
                        }
                        else
                        {
                            c1++;
                            
                            if(c1<sz_img1[3])
                                offset_c1 = c1*pagesz;
                            else
                                b_img1 = false;
                        }
                    }
                    
                    if(c1!=ref1)
                    {
                        offset_c1 = c1*pagesz;
                    }
                    else
                    {
                        c1++;
                        
                        if(c1<sz_img1[3])
                            offset_c1 = c1*pagesz;
                        else
                            b_img1 = false;
                    }
                    
                    qDebug()<<"color 1 ..."<<c1<<c;
                    
                    c1++;
                }
                else
                {
                    b_img1 = false;
                }
                
                if(!b_img1)
                {                
                    if(b_img2existNULL)
                    {
                        if(c2!=nullcolor2)
                        {
                            offset_c2 = c2*pagesz;
                        }
                        else
                        {
                            c2++;
                            
                            if(c2<sz_img2[3])
                                offset_c2 = c2*pagesz;
                            else
                                continue;
                        }
                    }
                    
                    if(c2!=ref2)
                    {
                        offset_c2 = c2*pagesz;
                    }
                    else
                    {
                        c2++;
                        
                        if(c2<sz_img2[3])
                            offset_c2 = c2*pagesz;
                        else
                            continue;
                    }
                    
                    qDebug()<<"color 2 ..."<<c2<<c;
                    
                    c2++;
                }
                
                for (V3DLONG k=0; k<sz_img1[2]; k++) 
                {
                    V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                    
                    V3DLONG offset_k1 = offset_k - offset_c + offset_c1;
                    V3DLONG offset_k2 = offset_k - offset_c + offset_c2;
                    
                    for(V3DLONG j=0; j<sz_img1[1]; j++)
                    {
                        V3DLONG offset_j = offset_k + j*sz_img1[0];
                        
                        V3DLONG offset_j1 = offset_k1 + j*sz_img1[0];
                        V3DLONG offset_j2 = offset_k2 + j*sz_img1[0];
                        
                        for(V3DLONG i=0; i<sz_img1[0]; i++)
                        {
                            V3DLONG idx = offset_j + i;
                            
                            if (b_img1) 
                            {
                                data1d[idx] = p1dImg1[offset_j1 + i];
                            }
                            else
                            {
                                data1d[idx] = p1dImg2[offset_j2 + i];
                            }
                        }
                    }
                }
            }
            
            V3DLONG offset = (colordim-1)*pagesz;
            V3DLONG offset1 = ref1*pagesz;
            V3DLONG offset2 = ref2*pagesz;
            
            for(V3DLONG i=0; i<pagesz; i++)
            {
                data1d[offset + i] = 0.5*p1dImg1[offset1+i] + 0.5*p1dImg2[offset2+i];
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (saveImage(blendImageName.toStdString().c_str(), (const unsigned char *)data1d, sz_blend, 1)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(data1d) {delete []data1d; data1d=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(data1d); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = sz_img1[0];
                metaImg[1] = sz_img1[1];
                metaImg[2] = sz_img1[2];
                metaImg[3] = colordim;
                metaImg[4] = datatype_img1;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
            
        }
        else if(datatype_img1 == V3D_UINT16)
        {
            //
            unsigned short* data1d = NULL;
            try
            {
                data1d = new unsigned short [totalplxs];
                
                memset(data1d, 0, sizeof(unsigned short)*totalplxs);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            
            //
            V3DLONG c1=0, c2=0;
            
            for(V3DLONG c=0; c<colordim-1; c++)
            {
                V3DLONG offset_c = c*pagesz;
                
                V3DLONG offset_c1, offset_c2;
                bool b_img1;
                
                if(c1<sz_img1[3])
                {
                    b_img1 = true;
                    
                    if(b_img1existNULL)
                    {
                        if(c1!=nullcolor1)
                        {
                            offset_c1 = c1*pagesz;
                        }
                        else
                        {
                            c1++;
                            
                            if(c1<sz_img1[3])
                                offset_c1 = c1*pagesz;
                            else
                                b_img1 = false;
                        }
                    }
                    
                    if(c1!=ref1)
                    {
                        offset_c1 = c1*pagesz;
                    }
                    else
                    {
                        c1++;
                        
                        if(c1<sz_img1[3])
                            offset_c1 = c1*pagesz;
                        else
                            b_img1 = false;
                    }
                    
                    qDebug()<<"color 1 ..."<<c1<<c;
                    
                    c1++;
                }
                else
                {
                    b_img1 = false;
                }
                
                if(!b_img1)
                {                
                    if(b_img2existNULL)
                    {
                        if(c2!=nullcolor2)
                        {
                            offset_c2 = c2*pagesz;
                        }
                        else
                        {
                            c2++;
                            
                            if(c2<sz_img2[3])
                                offset_c2 = c2*pagesz;
                            else
                                continue;
                        }
                    }
                    
                    if(c2!=ref2)
                    {
                        offset_c2 = c2*pagesz;
                    }
                    else
                    {
                        c2++;
                        
                        if(c2<sz_img2[3])
                            offset_c2 = c2*pagesz;
                        else
                            continue;
                    }
                    
                    qDebug()<<"color 2 ..."<<c2<<c;
                    
                    c2++;
                }
                
                for (V3DLONG k=0; k<sz_img1[2]; k++) 
                {
                    V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                    
                    V3DLONG offset_k1 = offset_k - offset_c + offset_c1;
                    V3DLONG offset_k2 = offset_k - offset_c + offset_c2;
                    
                    for(V3DLONG j=0; j<sz_img1[1]; j++)
                    {
                        V3DLONG offset_j = offset_k + j*sz_img1[0];
                        
                        V3DLONG offset_j1 = offset_k1 + j*sz_img1[0];
                        V3DLONG offset_j2 = offset_k2 + j*sz_img1[0];
                        
                        for(V3DLONG i=0; i<sz_img1[0]; i++)
                        {
                            V3DLONG idx = offset_j + i;
                            
                            if (b_img1) 
                            {
                                data1d[idx] = ((unsigned short *)p1dImg1)[offset_j1 + i];
                            }
                            else
                            {
                                data1d[idx] = ((unsigned short *)p1dImg2)[offset_j2 + i];
                            }
                        }
                    }
                }
            }
            
            V3DLONG offset = (colordim-1)*pagesz;
            V3DLONG offset1 = ref1*pagesz;
            V3DLONG offset2 = ref2*pagesz;
            
            for(V3DLONG i=0; i<pagesz; i++)
            {
                data1d[offset + i] = 0.5*((unsigned short *)p1dImg1)[offset1+i] + 0.5*((unsigned short *)p1dImg2)[offset2+i];
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (saveImage(blendImageName.toStdString().c_str(), (const unsigned char *)data1d, sz_blend, 2)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(data1d) {delete []data1d; data1d=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(data1d); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = sz_img1[0];
                metaImg[1] = sz_img1[1];
                metaImg[2] = sz_img1[2];
                metaImg[3] = colordim;
                metaImg[4] = datatype_img1;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
        }
        else if(datatype_img1 == V3D_FLOAT32)
        {
            //
            float* data1d = NULL;
            try
            {
                data1d = new float [totalplxs];
                
                memset(data1d, 0, sizeof(float)*totalplxs);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            
            //
            V3DLONG c1=0, c2=0;
            
            for(V3DLONG c=0; c<colordim-1; c++)
            {
                V3DLONG offset_c = c*pagesz;
                
                V3DLONG offset_c1, offset_c2;
                bool b_img1;
                
                if(c1<sz_img1[3])
                {
                    b_img1 = true;
                    
                    if(b_img1existNULL)
                    {
                        if(c1!=nullcolor1)
                        {
                            offset_c1 = c1*pagesz;
                        }
                        else
                        {
                            c1++;
                            
                            if(c1<sz_img1[3])
                                offset_c1 = c1*pagesz;
                            else
                                b_img1 = false;
                        }
                    }
                    
                    if(c1!=ref1)
                    {
                        offset_c1 = c1*pagesz;
                    }
                    else
                    {
                        c1++;
                        
                        if(c1<sz_img1[3])
                            offset_c1 = c1*pagesz;
                        else
                            b_img1 = false;
                    }
                    
                    qDebug()<<"color 1 ..."<<c1<<c;
                    
                    c1++;
                }
                else
                {
                    b_img1 = false;
                }
                
                if(!b_img1)
                {                
                    if(b_img2existNULL)
                    {
                        if(c2!=nullcolor2)
                        {
                            offset_c2 = c2*pagesz;
                        }
                        else
                        {
                            c2++;
                            
                            if(c2<sz_img2[3])
                                offset_c2 = c2*pagesz;
                            else
                                continue;
                        }
                    }
                    
                    if(c2!=ref2)
                    {
                        offset_c2 = c2*pagesz;
                    }
                    else
                    {
                        c2++;
                        
                        if(c2<sz_img2[3])
                            offset_c2 = c2*pagesz;
                        else
                            continue;
                    }
                    
                    qDebug()<<"color 2 ..."<<c2<<c;
                    
                    c2++;
                }
                
                for (V3DLONG k=0; k<sz_img1[2]; k++) 
                {
                    V3DLONG offset_k = offset_c + k*sz_img1[0]*sz_img1[1];
                    
                    V3DLONG offset_k1 = offset_k - offset_c + offset_c1;
                    V3DLONG offset_k2 = offset_k - offset_c + offset_c2;
                    
                    for(V3DLONG j=0; j<sz_img1[1]; j++)
                    {
                        V3DLONG offset_j = offset_k + j*sz_img1[0];
                        
                        V3DLONG offset_j1 = offset_k1 + j*sz_img1[0];
                        V3DLONG offset_j2 = offset_k2 + j*sz_img1[0];
                        
                        for(V3DLONG i=0; i<sz_img1[0]; i++)
                        {
                            V3DLONG idx = offset_j + i;
                            
                            if (b_img1) 
                            {
                                data1d[idx] = ((float *)p1dImg1)[offset_j1 + i];
                            }
                            else
                            {
                                data1d[idx] = ((float *)p1dImg2)[offset_j2 + i];
                            }
                        }
                    }
                }
            }
            
            V3DLONG offset = (colordim-1)*pagesz;
            V3DLONG offset1 = ref1*pagesz;
            V3DLONG offset2 = ref2*pagesz;
            
            for(V3DLONG i=0; i<pagesz; i++)
            {
                data1d[offset + i] = 0.5*((float *)p1dImg1)[offset1+i] + 0.5*((float *)p1dImg2)[offset2+i];
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (saveImage(blendImageName.toStdString().c_str(), (const unsigned char *)data1d, sz_blend, 4)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(data1d) {delete []data1d; data1d=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(data1d); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = sz_img1[0];
                metaImg[1] = sz_img1[1];
                metaImg[2] = sz_img1[2];
                metaImg[3] = colordim;
                metaImg[4] = datatype_img1;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
        }
        else 
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return -1;
        }
        
        // de-alloc
        if(p1dImg1) {delete []p1dImg1; p1dImg1=NULL;}
        if(p1dImg2) {delete []p1dImg2; p1dImg2=NULL;}

	}
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
    
    return true;
}

// menu
QStringList ImageBlendPlugin::menulist() const
{
    return QStringList() << tr("Image_Blend")
                         << tr("About");
}

void ImageBlendPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Image_Blend"))
    {
        ImageBlendingDialog dialog(callback, parent, NULL);
        if (dialog.exec()!=QDialog::Accepted)
            return;
        
        QString m_InputFileName1 = dialog.fn_img1;
        QString m_InputFileName2 = dialog.fn_img2;
        
        if ( !QFile::exists(m_InputFileName1) )
        {
            cout<<"Image 1 does not exist!"<<endl;
            return;
        }
        if ( !QFile::exists(m_InputFileName2) )
        {
            cout<<"Image 2 does not exist!"<<endl;
            return;
        }
        
        // call dofunc
        V3DPluginArgItem arg;
        V3DPluginArgList pluginfunc_input;
        V3DPluginArgList pluginfunc_output;
        
        vector<char*> fileList;
        vector<char*> paraList;
        
        fileList.clear();
        paraList.clear();
        
        QByteArray bytes1 = m_InputFileName1.toAscii();
        QByteArray bytes2 = m_InputFileName2.toAscii();
        
        fileList.push_back(bytes1.data());        
        fileList.push_back(bytes2.data());
        
        paraList.push_back("#s 0");
        
        arg.type = ""; arg.p = (void *)(&fileList); pluginfunc_input << arg;
        arg.type = ""; arg.p = (void *)(&paraList); pluginfunc_input << arg;
        
        bool success = dofunc("imageblend", pluginfunc_input, pluginfunc_output, callback, parent);
        
        if(!success)
        {
            QMessageBox::information(parent, "Warning: Image Blending", QString("Fail to run image blending program."));
            return;
        }
        
        V3DLONG *metaImg = (V3DLONG *)(pluginfunc_output.at(1).p);
        
        V3DLONG sx = metaImg[0];
        V3DLONG sy = metaImg[1];
        V3DLONG sz = metaImg[2];
        
        V3DLONG colordim = metaImg[3];
        V3DLONG datatype = metaImg[4];
        
        // show result in v3d
        if(datatype == V3D_UINT8)
        {
            //
            unsigned char* data1d = (unsigned char *)pluginfunc_output.at(0).p;
            
            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)data1d, sx, sy, sz, colordim, V3D_UINT8); //
            
            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Blended Image");
            callback.updateImageWindow(newwin);
            
        }
        else if(datatype == V3D_UINT16)
        {
            //
            unsigned short* data1d = (unsigned short *)pluginfunc_output.at(0).p;
            
            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)data1d, sx, sy, sz, colordim, V3D_UINT16); //
            
            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Blended Image");
            callback.updateImageWindow(newwin);
        }
        else if(datatype == V3D_FLOAT32)
        {
            //
            float* data1d = (float *)pluginfunc_output.at(0).p;;
            
            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)data1d, sx, sy, sz, colordim, V3D_FLOAT32); //
            
            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Blended Image");
            callback.updateImageWindow(newwin);
        }
        else 
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return;
        }
        
    }
	else if (menu_name == tr("About"))
	{
		QMessageBox::information(parent, "Version info", QString("ImageBlend Plugin %1 (July 30, 2011) developed by Yang Yu. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()));
		return;
	}
}

