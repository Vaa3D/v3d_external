//plugin_image_registration.h
//by Lei Qu
//2011-04-08

#include "plugin_image_registration.h"

#include <QtGui>
#include <math.h>
#include <stdlib.h>
#include "stackutil.h"
#include "common/q_imresize.cpp"
#include "common/q_convolve.h"
#include "histogram_matching/q_histogram_matching.h"

#include "q_paradialog_rigidaffine.h"
#include "q_registration_common.h"
#include "q_rigidaffine_registration.h"

Q_EXPORT_PLUGIN2(plugin_image_registration, ImageRegistrationPlugin);

const QString title = "ImageRegistrationPlugin demo";

void RigidAffineRegistration(V3DPluginCallback &callback, QWidget *parent,const int i_regtype);
void FFDNonrigidRegistration(V3DPluginCallback &callback, QWidget *parent);
void releasememory_rigidaffine(long *&,long *&,unsigned char *&,unsigned char *&,unsigned char *&,unsigned char *&,double *&,double *&,unsigned char *&);
void releasememory_nonrigid_FFD(long *&,long *&,unsigned char *&,unsigned char *&,unsigned char *&,unsigned char *&,double *&,double *&,unsigned char *&);

QStringList ImageRegistrationPlugin::menulist() const
{
    return QStringList()
	<< tr("rigid registration...");
}

void ImageRegistrationPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if(menu_name==tr("rigid registration..."))
	{
		RigidAffineRegistration(callback, parent,0);
	}
}

void RigidAffineRegistration(V3DPluginCallback &callback, QWidget *parent,const int i_regtype)
{
	CParaDialog_rigidaffine DLG_rigidaffine(callback,parent);
	if(DLG_rigidaffine.exec()!=QDialog::Accepted)
		return;

	for(long ind_file=0;ind_file<DLG_rigidaffine.m_qsSelectedFiles_sub.size();ind_file++)
	{
	QString qs_filename_input=DLG_rigidaffine.m_qsSelectedFiles_sub[ind_file];
	QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
	QString qs_filename_output=DLG_rigidaffine.lineEdit_img_sub2tar->text();
	QString qs_pathname_output=QFileInfo(qs_filename_output).path();

	QString qs_filename_img_sub=qs_filename_input;
	QString qs_filename_img_sub2tar,qs_filename_swc_grid;
	if(DLG_rigidaffine.m_qsSelectedFiles_sub.size()>1)
	{
		qs_filename_img_sub2tar=qs_pathname_output+"/"+qs_basename_input+".tif";
		qs_filename_swc_grid=qs_pathname_output+"/"+qs_basename_input+".swc";
	}
	else
	{
		qs_filename_img_sub2tar=DLG_rigidaffine.lineEdit_img_sub2tar->text();
		qs_filename_swc_grid=DLG_rigidaffine.lineEdit_swc_grid->text();
	}

	QString qs_filename_img_tar=DLG_rigidaffine.lineEdit_img_tar->text();
	long l_refchannel=DLG_rigidaffine.lineEdit_refchannel->text().toLong()-1;
	long l_downsampleratio=DLG_rigidaffine.lineEdit_downsampleratio->text().toLong();
	bool b_histomatch=DLG_rigidaffine.checkBox_histogrammatching->isChecked();
	bool b_alignedges=DLG_rigidaffine.checkBox_alignedges->isChecked();
	bool b_gausmooth=DLG_rigidaffine.groupBox_Gaussian->isChecked();
	long l_gauradius=DLG_rigidaffine.lineEdit_Gau_radius->text().toLong();
	double d_gausigma=DLG_rigidaffine.lineEdit_Gau_sigma->text().toDouble();

	CParas_reg paras;
	paras.i_regtype=i_regtype;
	paras.b_alignmasscenter=1;
	paras.l_iter_max=DLG_rigidaffine.lineEdit_iter_max->text().toLong();
	paras.d_step_inimultiplyfactor=DLG_rigidaffine.lineEdit_step_multiplyfactor->text().toDouble();
	paras.d_step_annealingratio=DLG_rigidaffine.lineEdit_step_annealingrate->text().toDouble();
	paras.d_step_min=DLG_rigidaffine.lineEdit_step_min->text().toDouble();

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	if(i_regtype==0) printf(">>Rigid registration\n");
	if(i_regtype==1) printf(">>Affine registration\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
	printf(">>  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
	printf(">>  input reference channel:      %ld\n",l_refchannel);
	printf(">>  input downsample ratio:       %ld\n",l_downsampleratio);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output sub2tar image:         %s\n",qPrintable(qs_filename_img_sub2tar));
	printf(">>  output meshgrid apo:          %s\n",qPrintable(qs_filename_swc_grid));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	if(i_regtype!=0 && i_regtype!=1)
	{
		printf("ERROR: invalid i_regtype!\n");
		return;
	}
	if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
	{
		printf("ERROR: invalid input file name (target or subject)!\n");
		return;
	}
	if(l_refchannel<0 || l_refchannel>=4)
	{
		printf("ERROR: invalid reference channel!\n");
		return;
	}

	long *sz_img_tar_input=0,*sz_img_sub_input=0;
	unsigned char *p_img_tar_input=0,*p_img_sub_input=0;
	unsigned char *p_img8u_tar=0,*p_img8u_sub=0;
	double *p_img64f_tar=0,*p_img64f_sub=0;
	unsigned char *p_img8u_sub_warp=0;

	printf("1. Read target and subject image. \n");
	int datatype_tar_input=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
		releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
		return;
	}
	printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
	printf("\t\tdatatype: %d\n",datatype_tar_input);

	int datatype_sub_input=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
		releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
		return;
	}
	printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
	printf("\t\tdatatype: %d\n",datatype_sub_input);

	if(datatype_tar_input!=datatype_sub_input || (datatype_tar_input!=1 
		// && datatype_tar_input!=2 
		// && datatype_tar_input!=4
		)) //by PHC 20111123
	{
		//printf("ERROR: input target and subject image have different datatype or datatype is not uint8/uint16/float32!\n");
		printf("ERROR: input target and subject image have different datatype or datatype is not uint8!\n");
		releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
		return;
	}
	if(sz_img_tar_input[3]<l_refchannel+1 || sz_img_sub_input[3]<l_refchannel+1)
	{
		printf("ERROR: invalid reference channel!\n");
		releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
		return;
	}

	printf("2. Extract reference channel. \n");
	{
		unsigned char *p_img_1c=0;
		if(!q_extractchannel(p_img_tar_input,sz_img_tar_input,l_refchannel,p_img_1c))
		{
			printf("ERROR: q_extractchannel() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
		if(p_img8u_tar) 			{delete []p_img8u_tar;			p_img8u_tar=0;}
		p_img8u_tar=p_img_1c;		p_img_1c=0;
		if(p_img_tar_input) 		{delete []p_img_tar_input;		p_img_tar_input=0;}
		if(!q_extractchannel(p_img_sub_input,sz_img_sub_input,l_refchannel,p_img_1c))
		{
			printf("ERROR: q_extractchannel() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
		if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
		p_img8u_sub=p_img_1c;		p_img_1c=0;
		if(qs_filename_img_sub2tar==NULL)
		if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}
	}

	printf("3. Downsample 1c images and make them have the same size. \n");
	long sz_img[4]={sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],1};
	double d_downsample_ratio_tar[3],d_downsample_ratio_sub[3];
	{
		for(int i=0;i<3;i++)
		{
			sz_img[i]=sz_img_tar_input[i]/double(l_downsampleratio)+0.5;
			d_downsample_ratio_tar[i]=sz_img_tar_input[i]/double(sz_img[i]);
			d_downsample_ratio_sub[i]=sz_img_sub_input[i]/double(sz_img[i]);
		}

		printf("\t>>downsample target image from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img[0],sz_img[1],sz_img[2]);
		if(sz_img_tar_input[0]!=sz_img[0] || sz_img_tar_input[1]!=sz_img[1] || sz_img_tar_input[2]!=sz_img[2])
		{
			unsigned char *p_img_tmp=0;
			long sz_img_old[4]={sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],1};
			if(!q_imresize_3D(p_img8u_tar,sz_img_old,0,sz_img,p_img_tmp))
			{
				printf("ERROR: q_imresize_3D() return false!\n");
				releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
				return;
			}
			if(p_img8u_tar) 			{delete []p_img8u_tar;			p_img8u_tar=0;}
			p_img8u_tar=p_img_tmp;	p_img_tmp=0;
		}

		printf("\t>>downsample subject image from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img[0],sz_img[1],sz_img[2]);
		if(sz_img_sub_input[0]!=sz_img[0] || sz_img_sub_input[1]!=sz_img[1] || sz_img_sub_input[2]!=sz_img[2])
		{
			unsigned char *p_img_tmp=0;
			long sz_img_old[4]={sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],1};
			if(!q_imresize_3D(p_img8u_sub,sz_img_old,0,sz_img,p_img_tmp))
			{
				printf("ERROR: q_imresize_3D() return false!\n");
				releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
				return;
			}
			if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
			p_img8u_sub=p_img_tmp;	p_img_tmp=0;
		}
	}

	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2];

	if(b_histomatch)
	{
		printf("[optional] Match the histogram of subject to that of target. \n");
		unsigned char *p_img_tmp=0;

		if(!q_histogram_matching_1c(p_img8u_tar,sz_img,
									p_img8u_sub,sz_img,
									p_img_tmp))
		{
			printf("ERROR: q_histogram_matching_1c() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
		if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
		p_img8u_sub=p_img_tmp;	p_img_tmp=0;
	}

	printf("4. Convert image data from uint8 to double and scale to [0~1]. \n");
	{
		p_img64f_tar=new double[l_npixels]();
		p_img64f_sub=new double[l_npixels]();
		if(!p_img64f_tar || !p_img64f_sub)
		{
			printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}

		long l_maxintensity_tar,l_maxintensity_sub;
		l_maxintensity_tar=l_maxintensity_sub=255;
		for(long i=0;i<l_npixels;i++)
		{
			p_img64f_tar[i]=p_img8u_tar[i]/(double)l_maxintensity_tar;
			p_img64f_sub[i]=p_img8u_sub[i]/(double)l_maxintensity_sub;
		}

		if(p_img8u_tar) 			{delete []p_img8u_tar;			p_img8u_tar=0;}
		if(p_img8u_sub) 			{delete []p_img8u_sub;			p_img8u_sub=0;}
	}

	if(b_alignedges)
	{
		printf("[optional] Generate gradient images. \n");
		double *p_img64f_gradnorm=0;
		//target
		if(!q_gradientnorm(p_img64f_tar,sz_img,1,p_img64f_gradnorm))
		{
			printf("ERROR: q_gradientnorm() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
		if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
		p_img64f_tar=p_img64f_gradnorm; p_img64f_gradnorm=0;
		//subject
		if(!q_gradientnorm(p_img64f_sub,sz_img,1,p_img64f_gradnorm))
		{
			printf("ERROR: q_gradientnorm() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
		if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}
		p_img64f_sub=p_img64f_gradnorm; p_img64f_gradnorm=0;
	}

	if(b_gausmooth)
	{
		printf("[optional] Gaussian smooth input target and subject images. \n");
		long l_kenelradius=l_gauradius;
		double d_sigma=d_gausigma;
		vector<double> vec1D_kernel;
		if(!q_kernel_gaussian_1D(l_kenelradius,d_sigma,vec1D_kernel))
		{
			printf("ERROR: q_kernel_gaussian_1D() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}

		printf("\tsmoothing target image.\n");
		if(!q_convolve_img64f_3D_fast(p_img64f_tar,sz_img,vec1D_kernel))
		{
			printf("ERROR: q_convolve64f_3D_fast() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
		printf("\tsmoothing subject image.\n");
		if(!q_convolve_img64f_3D_fast(p_img64f_sub,sz_img,vec1D_kernel))
		{
			printf("ERROR: q_convolve64f_3D_fast() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
	}


	//------------------------------------------------------------------------------------------------------------------------------------
	if(i_regtype==0) printf("5. Enter rigid registration iteration: \n");
	if(i_regtype==1) printf("5. Enter affine registration iteration: \n");

	vector< vector< vector< vector<double> > > > vec4D_grid;
	if(!q_rigidaffine_registration(paras,p_img64f_tar,p_img64f_sub,sz_img,vec4D_grid))
	{
		printf("ERROR: q_affine_registration_SSD() return false!\n");
		releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
		return;
	}
	if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
	if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}

	for(long x=0;x<2;x++)
		for(long y=0;y<2;y++)
			for(long z=0;z<2;z++)
			{
				vec4D_grid[y][x][z][0]=(vec4D_grid[y][x][z][0]+1)*d_downsample_ratio_tar[0]-1;
				vec4D_grid[y][x][z][1]=(vec4D_grid[y][x][z][1]+1)*d_downsample_ratio_tar[1]-1;
				vec4D_grid[y][x][z][2]=(vec4D_grid[y][x][z][2]+1)*d_downsample_ratio_tar[2]-1;
			}

	if(qs_filename_swc_grid!=NULL)
	{
		printf(">> Save deformed grid to swc file:[%s] \n",qPrintable(qs_filename_swc_grid));
		if(!q_rigidaffine_savegrid_swc(vec4D_grid,sz_img_tar_input,qPrintable(qs_filename_swc_grid)))
		{
			printf("ERROR: q_savegrid_swc() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
	}

	if(qs_filename_img_sub2tar!=NULL)
	{
		printf(">> Warp subject image according to the grid and save. \n");
		printf("\t>> resize the input subject image to the same size as target. \n");
		unsigned char *p_img_sub_resize=0;
		if(!q_imresize_3D(p_img_sub_input,sz_img_sub_input,0,sz_img_tar_input,p_img_sub_resize))
		{
			printf("ERROR: q_imresize_3D() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}

		printf("\t>> warped subject image according to deformed grid. \n");
		if(!q_rigidaffine_warpimage_baseongrid(paras.i_regtype,p_img_sub_resize,sz_img_tar_input,vec4D_grid,p_img8u_sub_warp))
		{
			printf("ERROR: q_warpimage_baseongrid() return false!\n");
			if(p_img_sub_resize) 			{delete []p_img_sub_resize;			p_img_sub_resize=0;}
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
		if(p_img_sub_resize) 			{delete []p_img_sub_resize;			p_img_sub_resize=0;}

		printf("\t>> Save warped subject image to file:[%s] \n",qPrintable(qs_filename_img_sub2tar));
		if(!saveImage(qPrintable(qs_filename_img_sub2tar),p_img8u_sub_warp,sz_img_tar_input,1))
		{
			printf("ERROR: q_save64f01_image() return false!\n");
			releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);
			return;
		}
	}

	printf(">>Free memory\n");
	releasememory_rigidaffine(sz_img_tar_input,sz_img_sub_input,p_img_tar_input,p_img_sub_input,p_img8u_tar,p_img8u_sub,p_img64f_tar,p_img64f_sub,p_img8u_sub_warp);

	}
	printf(">>Program exit success!\n\n");
}

void releasememory_rigidaffine(long *&sz_img_tar_input,long *&sz_img_sub_input,
		unsigned char *&p_img_tar_input,unsigned char *&p_img_sub_input,
		unsigned char *&p_img8u_tar,unsigned char *&p_img8u_sub,
		double *&p_img64f_tar,double *&p_img64f_sub,
		unsigned char *&p_img8u_output_sub)
{
	if(p_img_tar_input) 	{delete []p_img_tar_input;		p_img_tar_input=0;}
	if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}
	if(p_img8u_tar) 		{delete []p_img8u_tar;			p_img8u_tar=0;}
	if(p_img8u_sub) 		{delete []p_img8u_sub;			p_img8u_sub=0;}
	if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
	if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}
	if(p_img8u_output_sub)  {delete []p_img8u_output_sub;	p_img8u_output_sub=0;}
	if(sz_img_tar_input) 	{delete []sz_img_tar_input;		sz_img_tar_input=0;}
	if(sz_img_sub_input) 	{delete []sz_img_sub_input;		sz_img_sub_input=0;}
	printf("Release all memory done!\n");
}
