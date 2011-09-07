/* ifusion.cpp
 * 2011-08-31: the program is created by Yang Yu
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

#include "../istitch/y_imglib.h"

#include "ifusion.h"

using namespace std;

//
// image normalization and image blending program
//
Q_EXPORT_PLUGIN2(ifusion, ImageFusionPlugin);

//plugin
const QString title = "Image Fusion";

//
class AdjustPara
{
public:
    AdjustPara(){k=0.0; b=0.0;}
    ~AdjustPara(){}
    
public:
    float k, b;
};

// linear blending
bool computeWeights(Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, V3DLONG i, V3DLONG j, V3DLONG k, V3DLONG tilei, float &weights)
{

    V3DLONG vx = vim.sz[0];
    V3DLONG vy = vim.sz[1];
    V3DLONG vz = vim.sz[2];
    V3DLONG vc = vim.sz[3];
    
    V3DLONG sz_img = vx*vy*vz;
    
    QList<float> listWeights;
    
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
        V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];
        
        V3DLONG x_start = (1 > tile2vi_xs) ? 1 : tile2vi_xs;
        V3DLONG x_end = (vx < tile2vi_xe) ? vx : tile2vi_xe;
        V3DLONG y_start = (1 > tile2vi_ys) ? 1 : tile2vi_ys;
        V3DLONG y_end = (vy < tile2vi_ye) ? vy : tile2vi_ye;
        V3DLONG z_start = (1 > tile2vi_zs) ? 1 : tile2vi_zs;
        V3DLONG z_end = (vz < tile2vi_ze) ? vz : tile2vi_ze;
        
        x_end++;
        y_end++;
        z_end++;
        
        V3DLONG rx = x_end - x_start;
        V3DLONG ry = y_end - y_start;
        V3DLONG rz = z_end - z_start;
        
        if(i>=x_start && i<=x_end && j>=y_start && j<=y_end && k>=z_start && k<=z_end)
        {
            float dist2xl = fabs(float(i-x_start));
            float dist2xr = fabs(float(x_end-i));
            float dist2yu = fabs(float(j-y_start));
            float dist2yd = fabs(float(y_end-j));
            float dist2zf = fabs(float(k-z_start));
            float dist2zb = fabs(float(z_end-k));
            
            float dist2boundary = dist2xl;
            if( dist2xr<dist2boundary ) dist2boundary = dist2xr;
            if( dist2yu<dist2boundary ) dist2boundary = dist2yu;
            if( dist2yd<dist2boundary ) dist2boundary = dist2yd;
            if( dist2zf<dist2boundary ) dist2boundary = dist2zf;
            if( dist2zb<dist2boundary ) dist2boundary = dist2zb;
            
            listWeights.push_back(dist2boundary);
        }
        
    }
    
    if (listWeights.size()<=1) 
    {
        weights=1.0;
    }
    else
    {
        float sumweights=0;
        for (int i=0; i<listWeights.size(); i++) {
            sumweights += listWeights.at(i);
        }
        weights = listWeights.at(tilei) / sumweights;
    }
    
    return true;
}



// obtain bg mean and var
template<class Tdata>
bool computeImgMeanVar(Tdata *pImg, V3DLONG szimg, float &imgmean, float &imgstdvar)
{
    //
    imgmean=0;
    
    for (V3DLONG i=0; i<szimg; i++) 
    {
        imgmean += pImg[i];
    }
    imgmean /= (float)szimg;
    
    imgstdvar = 0;
    for (V3DLONG i=0; i<szimg; i++) 
    {
        float val = (float)(pImg[i]);
        imgstdvar += (val - imgmean)*(val - imgmean);
    }
    imgstdvar /= (float)(szimg-1);
    
    imgstdvar = sqrt(imgstdvar);
    
    return true;
}

// reconstruct tiles into one stitched image
template <class Tdata> 
bool ireconstructing(Tdata *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, V3DLONG vx, V3DLONG vy, V3DLONG vz, V3DLONG vc, Tdata intensityrange, float **imgmean, float **imgstdvar)
{
    // for boundary counting
    V3DLONG n_swc=1;
    QString outputSWC, outputAPO;
    FILE * fp_swc=NULL, *fp_apo=NULL; // .swc showing boundary .apo showing tile's name
    
    //
    V3DLONG pagesz = vx*vy*vz;
    V3DLONG imgsz = pagesz*vc;
    float *pTmp = NULL;
    try {
        
        
        pTmp = new float [imgsz];
        memset(pTmp, 0.0, sizeof(float)*imgsz);
    } catch (...) {
        printf("Fail to allocate memory!\n");
        return false;
    }

    // fusion
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        // loading relative imagg files
        V3DLONG *sz_relative = 0;
        int datatype_relative = 0;
        unsigned char* relative1d = 0;
        
        if (loadImage(const_cast<char *>(vim.lut[ii].fn_img.c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.lut[ii].fn_img.c_str());
            return false;
        }
        V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
        
        //
        V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
        V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];
        
        V3DLONG x_start = (1 > tile2vi_xs) ? 1 : tile2vi_xs;
        V3DLONG x_end = (vx < tile2vi_xe) ? vx : tile2vi_xe;
        V3DLONG y_start = (1 > tile2vi_ys) ? 1 : tile2vi_ys;
        V3DLONG y_end = (vy < tile2vi_ye) ? vy : tile2vi_ye;
        V3DLONG z_start = (1 > tile2vi_zs) ? 1 : tile2vi_zs;
        V3DLONG z_end = (vz < tile2vi_ze) ? vz : tile2vi_ze;
        
        x_end++;
        y_end++;
        z_end++;
        
        //suppose all tiles with same color dimensions
        if(rc>vc)
            rc = vc;
        
        //
        Tdata *prelative = (Tdata *)relative1d;
        
        for(V3DLONG c=0; c<rc; c++)
        {
            V3DLONG o_c = c*vx*vy*vz;
            V3DLONG o_r_c = c*rx*ry*rz;
            for(V3DLONG k=z_start; k<z_end; k++)
            {
                V3DLONG o_k = o_c + k*vx*vy;
                V3DLONG o_r_k = o_r_c + (k-z_start)*rx*ry;
                
                for(V3DLONG j=y_start; j<y_end; j++)
                {
                    V3DLONG o_j = o_k + j*vx;
                    V3DLONG o_r_j = o_r_k + (j-y_start)*rx;
                    for(V3DLONG i=x_start; i<x_end; i++)
                    {
                        V3DLONG idx = o_j + i;
                        V3DLONG idx_r = o_r_j + (i-x_start);
                        
                        float val = (float)(prelative[idx_r]);
                        
                        val = (val - imgmean[c][ii]) / imgstdvar[c][ii]; // normalization
                        
                        //
//                        float coef;
//                        if(!computeWeights(vim, i, j, k, ii, coef) )
//                        {
//                            printf("Fail to call function computeWeights!\n");
//                            return false;
//                        }
//                        pVImg[idx] += (SDATATYPE) (val*coef); // linear blending
                        
                        if(pTmp[idx])
                            pTmp[idx] = (pTmp[idx] + val )/2; // Avg. Intensity
                        else
                            pTmp[idx] = val;
                        
                    }
                }
            }
        }
        
        //de-alloc
        if(relative1d) {delete []relative1d; relative1d=0;}
        if(sz_relative) {delete []sz_relative; sz_relative=0;}
    }
    
    float minval, maxval;
    for(V3DLONG c=0; c<vc; c++) 
    {
        V3DLONG offsets = c*pagesz;
        
        minval=1e9;
        maxval=-1e9;
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            V3DLONG idx = offsets+i;
            
            float val=pTmp[idx];
            
            if(minval>val) minval = val;
            if(maxval<val) maxval = val;
        }
        maxval -= minval;
        
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            
            V3DLONG idx = offsets+i;
            
            pVImg[idx] = (Tdata) (intensityrange * (pTmp[idx] - minval)/maxval);
        }
    }
    
    //de-alloc
    if(pTmp) {delete []pTmp; pTmp=NULL;}
    
    return true;
}


// funcs
QStringList ImageFusionPlugin::funclist() const
{
    return QStringList() << "ifusion";
}

bool ImageFusionPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent)
{
    //
    if (func_name == tr("ifusion"))
    {
        // parsing parameters
        if(input.size()<1) return false; // no inputs
        
        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty()) 
        {
            //print Help info
            printf("\nUsage: v3d -x ifusion.dylib -f ifusion -i <folder> -o <output_image> -p \"#s <save_blending_result zero(false)/nonzero(true)>\"\n");
            
            return true;
        }        
        
        char * infile = infilelist->at(0); // input images
        char * paras = NULL; // parameters
        char * outfile = NULL; // outputs
        
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        
        bool b_saveimage = true; // save the blended image by default
        
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
                                b_saveimage = (atoi( argv[i+1] ))?true:false;                                
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                            
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                    
                }
            }
        }
        
        QString blendImageName;
        
        // get stitch configuration        
        QDir myDir(infile);
        QStringList list = myDir.entryList(QStringList("*.tc"));
        
        if(list.size()!=1)
        {
            printf("Must have only one stitching configuration file!\n");
            return false;
        }
        
        if(!outfile) 
            blendImageName = QString(infile).append("/stitched.v3draw");
        else
            blendImageName = QString(outfile);
        
        if(QFileInfo(blendImageName).suffix().toUpper() != "V3DRAW")
        {
            blendImageName.append(".v3draw"); // force to save as .v3draw file
        }
        
        // load config
        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;
        
        QString tcfile = QString(infile).append("/").append(list.at(0));
        
        if( !vim.y_load(tcfile.toStdString()) )
        {
            printf("Wrong stitching configuration file to be load!\n");
            return false;
        }
        
        //
        float **bgmean = NULL;
        float **bgstdvar = NULL;
        
        try 
        {            
            bgmean = new float* [ vim.sz[3] ];
            for(int i = 0; i < vim.sz[3]; ++i)
                bgmean[i] = new float [vim.number_tiles];
            
            bgstdvar = new float* [ vim.sz[3] ];
            for(int i = 0; i < vim.sz[3]; ++i)
                bgstdvar[i] = new float [vim.number_tiles];
        } 
        catch (...) 
        {
            printf("Fail to allocate memory for threshold.\n");
            
            if(bgmean)
            {
                for (int i = 0; i < vim.sz[3]; ++i)
                {
                    delete [] bgmean[i];
                }
                delete []bgmean; bgmean=NULL;
            }
            if(bgstdvar)
            {
                for (int i = 0; i < vim.sz[3]; ++i)
                {
                    delete [] bgstdvar[i];
                }
                delete []bgstdvar; bgstdvar=NULL;
            }
            
            return false;
        }
        
        int datatype_tile = 0; // assume all tiles with the same datatype
        for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
        {
            // load tile
            V3DLONG *sz_relative = 0; 
            unsigned char* relative1d = 0;

            QString curPath = QFileInfo(tcfile).path();;

            string fn = curPath.append("/").append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();

            vim.lut[ii].fn_img = fn; // absolute path
            
            if (loadImage(const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_tile)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.lut[ii].fn_img.c_str());
                return -1;
            }
            V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
            
            V3DLONG szimg = rx*ry*rz;
            
            int start_t=clock();

            for (V3DLONG c=0; c<rc; c++)
            {
                V3DLONG offset = c*szimg;
                
                // compute re-scale parameters
                if(datatype_tile == V3D_UINT8)
                {
                    // 8-bit data
                    
                    // compute img mean and stdvar
                    if(!computeImgMeanVar<unsigned char>((unsigned char*)relative1d + offset, szimg, bgmean[c][ii], bgstdvar[c][ii]))
                    {
                        printf("Fail to call function computeBgMeanVar! \n");
                        return false;
                    }
                }
                else if(datatype_tile == V3D_UINT16)
                {
                    // 12-bit data
                    
                    // compute img mean and stdvar
                    if(!computeImgMeanVar<unsigned short>((unsigned short*)relative1d + offset, szimg, bgmean[c][ii], bgstdvar[c][ii]))
                    {
                        printf("Fail to call function computeBgMeanVar! \n");
                        return false;
                    }
                }
                else if(datatype_tile == V3D_FLOAT32)
                {
                    // current not supported
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, FLOAT32 datatype.\n");
                    return false;
                }
            }

            qDebug()<<"time elapse ..."<<clock()-start_t<<ii;
            
            // de-alloca
            if(relative1d) {delete []relative1d; relative1d=NULL;}
        }
        
        // do blending
        V3DLONG vx, vy, vz, vc;

        vx = vim.sz[0]; //
        vy = vim.sz[1];
        vz = vim.sz[2];
        vc = vim.sz[3];

        V3DLONG pagesz_vim = vx*vy*vz*vc;

        if(datatype_tile == V3D_UINT8)
        {
            // init
            unsigned char *pVImg = NULL;

            try
            {
                pVImg = new unsigned char [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            bool success = ireconstructing<unsigned char>((unsigned char *)pVImg, vim, vx, vy, vz, vc, 255, bgmean, bgstdvar);
            if(!success)
            {
                printf("Fail to call function ireconstructing! \n");
                return false;
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (saveImage(blendImageName.toStdString().c_str(), (const unsigned char *)pVImg, vim.sz, 1)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            // init
            unsigned short *pVImg = NULL;

            try
            {
                pVImg = new unsigned short [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            bool success = ireconstructing<unsigned short>((unsigned short *)pVImg, vim, vx, vy, vz, vc, 4096, bgmean, bgstdvar);
            if(!success)
            {
                printf("Fail to call function ireconstructing! \n");
                return false;
            }
            
            // output
            if(b_saveimage)
            {
                //save
                if (saveImage(blendImageName.toStdString().c_str(), (const unsigned char *)pVImg, vim.sz, 2)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
                
                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;
                
                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;
                
                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;
                
                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }

        }
        else if(datatype_tile == V3D_FLOAT32)
        {
            // current not supported
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }
        
        // de-alloc
        if(bgmean)
        {
            for (int i = 0; i < vim.sz[3]; ++i)
            {
                delete [] bgmean[i];
            }
            delete []bgmean; bgmean=NULL;
        }
        if(bgstdvar)
        {
            for (int i = 0; i < vim.sz[3]; ++i)
            {
                delete [] bgstdvar[i];
            }
            delete []bgstdvar; bgstdvar=NULL;
        }
    }
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
    
    return true;
}

// menu
QStringList ImageFusionPlugin::menulist() const
{
    return QStringList() << tr("Image Fusion")
                         << tr("About");
}

void ImageFusionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Image Fusion"))
    {
        ImageFusionDialog dialog(callback, parent, NULL);
        if (dialog.exec()!=QDialog::Accepted)
            return;
        
        QString m_InputFolder = dialog.foldername;
        
        if ( !QFile::exists(m_InputFolder) )
        {
            cout<<"Folder does not exist!"<<endl;
            return;
        }
        
        // call fusion dofunc 
        V3DPluginArgItem arg;
        V3DPluginArgList pluginfunc_input;
        V3DPluginArgList pluginfunc_output;
        
        vector<char*> fileList;
        vector<char*> paraList;
        
        fileList.clear();
        paraList.clear();
        
        QByteArray bytes = m_InputFolder.toAscii();
        
        fileList.push_back(bytes.data());
        
        paraList.push_back("#s 0");
        
        arg.type = ""; arg.p = (void *)(&fileList); pluginfunc_input << arg;
        arg.type = ""; arg.p = (void *)(&paraList); pluginfunc_input << arg;
        
        bool success = dofunc("ifusion", pluginfunc_input, pluginfunc_output, callback, parent);
        
        if(!success)
        {
            QMessageBox::information(parent, "Warning: Image Fusion", QString("Fail to run image fusion program."));
            return;
        }
        
        // output
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
            unsigned char* data1d = (unsigned char *)(pluginfunc_output.at(0).p);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)data1d, sx, sy, sz, colordim, V3D_UINT8); //

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched Image");
            callback.updateImageWindow(newwin);

        }
        else if(datatype == V3D_UINT16)
        {
            //
            unsigned short* data1d = (unsigned short *)(pluginfunc_output.at(0).p);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)data1d, sx, sy, sz, colordim, V3D_UINT16); //

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched Image");
            callback.updateImageWindow(newwin);
        }
        else if(datatype == V3D_FLOAT32)
        {
            // current not support
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return;
        }
        
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Image fusion. Version %1 (August 31, 2011) developed by Yang Yu and Sean Murphy. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()));
        return;
    }
}

