/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//main_worm_straightener.cpp
//by Hanchuan Peng
//2008-03-05
//2008-07-12

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <fstream> 

//#define FREAL float

#include "../basic_c_fun/stackutil.h"
#include "../basic_c_fun/volimg_proc.h"

void parse_Radii_Paremeter(char * optarg, double * &radius, V3DLONG & len_Radii);
bool convert_type2uint8_3dimg_1dpt(void * &img, V3DLONG * sz, int datatype);

#include "bdb_minus.h"

vector<Coord2D> readControlPointFile2d(string posFile);
vector<Coord3D> readControlPointFile3d(string posFile);

void printHelp ();
void printHelp()
{
	printf("\nUsage: <main prog name> -i <input_image_file> -p <prior control pt file> -o <output_image_file> -c <channalNo_reference> -A <alpha: image force> -B <beta: length force> -G <gamma: smoothness force> -n <nloop> -w <local win radius>\n");
	printf("\t -i <input_image_file>              input 3D image (tif, or Hanchuan's RAW or LSM). \n");
	printf("\t -p <prior control pt file>         input prior location of control points (note: the order will matter!). If unspecified, then randomly initialized. \n");
	printf("\t -o <output_image_file>             output image where the third channel is a mask indicating the regions. \n");
	printf("\t -c <channalNo_reference>           the ID of channel for processing (starting from 0). If unspecified, then initialized as 0.\n");
	printf("\t -A <alpha>                         the alpha coefficient for the image force. Default = 1.0.\n");
	printf("\t -B <beta>                          the beta coefficient for the length force. Default = 0.5.\n");
	printf("\t -G <gamma>                         the gamma coefficient for the smoothness force Default = 0.5.\n");
	printf("\t -n <nloop>                         the number of maximum loops of optimization. If unspecified, then initialized as 500.\n");
	printf("\t -w <local win radius>              the radius of local window for center of mass estimation. The real win size is 2*radius+1. The default radius = 20. \n");
	printf("\t \n");
	printf("\t [-v]                               verbose printing enabled. \n");
	printf("\t [-h]                               print this message.\n");
	return;
}

#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

int main (int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}
	
	/* Read arguments */
	
	char *dfile_input = NULL;
	char *dfile_output = NULL;
	char *dfile_prior = NULL;
	int channelNo = 0; 
	float alpha = 1;
	float beta = 0.5;
	float gamma = 0.5;
	int radius = 20;
	int nloops = 500;
	bool b_verbose_print=false;
	
	//BasicWarpParameter my_para;
	
	int c;
	static char optstring[] = "hvi:p:o:c:A:B:G:n:w:";
	opterr = 0;
	while ((c = getopt (argc, argv, optstring)) != -1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;
				
			case 'v':
				b_verbose_print = 1;
				break;
				
			case 'i':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				dfile_input = optarg;
				break;
				
			case 'p':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -p.\n");
					return 1;
				}
				dfile_prior = optarg;
				break;
				
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_output = optarg;
				break;
				
			case 'c':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				channelNo = atoi (optarg);
				if (channelNo < 0)
				{
					fprintf (stderr, "Illegal channelNo found! It must be >=0.\n");
					return 1;
				}
				break;
				
			case 'A':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -A.\n");
					return 1;
				}
				alpha = atof (optarg);
				if (alpha<0)
				{
					fprintf (stderr, "alpha must not be less than than 0.\n");
					return 1;
				}
				break;

			case 'B':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -B.\n");
					return 1;
				}
				beta = atof (optarg);
				if (beta<0)
				{
					fprintf (stderr, "beta must not be less than than 0.\n");
					return 1;
				}
				break;

			case 'G':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -G.\n");
					return 1;
				}
				gamma = atof (optarg);
				if (gamma<0)
				{
					fprintf (stderr, "gamma must not be less than than 0.\n");
					return 1;
				}
				break;

			case 'n':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -n.\n");
					return 1;
				}
				nloops = atoi (optarg);
				if (nloops < 1)
				{
					fprintf (stderr, "The number of loop must be >= 1.\n");
					return 1;
				}
				break;
				
			case 'w':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -w.\n");
					return 1;
				}
				radius = atoi (optarg);
				if (radius < 0)
				{
					fprintf (stderr, "Illeagal radius ! It must be >=0.\n");
					return 1;
				}
				break;
				
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 0;
				
				// default:        abort (); 
        }
    }
	
	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);


	// display the parameter info 
	printf("\n-------------------------------------------------\n");
	printf("*** Parameters ***:\n");
	printf("Input  file: [%s]\n", dfile_input);
	printf("Output file: [%s]\n", dfile_output);
	printf("Channel  no: [%d]\n", channelNo);
	printf("alpha      : [%7.4f]\n", alpha);
	printf("beta       : [%7.4f]\n", beta);
	printf("gamma      : [%7.4f]\n", gamma);
	printf("radius     : [%d]\n", radius);
	printf("nloops     : [%d]\n", nloops);
	printf("Verbose    : [%d]\n", (int)b_verbose_print);
	printf("---------------------------------------------------\n\n");

	// Declare some necessary variables. 
	vector<Coord2D> mCoord;
	
	unsigned char * img_input = 0; // note that this variable must be initialized as NULL. 
	V3DLONG * sz_input = 0; // note that this variable must be initialized as NULL. 
	int datatype_input = 0;
	
	unsigned char * img_output = 0;
	V3DLONG * sz_output = 0;
	int datatype_output = 0;
	
	if (!dfile_input || !dfile_output)
	{
		fprintf (stderr, "You have not specified any input and output file!. Exit. \n");
		goto Label_exit;
	}
	else
	{
		FILE *tmp_fp = fopen(dfile_input, "r");
		if (!tmp_fp)
		{
			fprintf (stderr, "You have specified an input that does not exist!. Exit. \n");
			goto Label_exit;
		}
		else
		{
			fclose(tmp_fp);
		}
	}
	
	if (loadImage(dfile_input, img_input, sz_input, datatype_input)!=true)
	{
		fprintf (stderr, "Error happens in reading the input file [%s]. Exit. \n", dfile_input);
		goto Label_exit;
	}

	//check if any one need to be converted as uint8, if so, then do the conversion. 
	if (datatype_input!=1)
	{
		if (datatype_input==2 || datatype_input==4)
		{
			fprintf (stdout, "Now try to convert target image datatype from [%d]bytes per pixel to 1byte per pixel.\n", datatype_input);
			convert_type2uint8_3dimg_1dpt((void * &)img_input, sz_input, datatype_input);
			datatype_input=1;
		}
		else
		{
			fprintf (stderr, "The datatype of the target image cannot be converted to UINT8. Exit. \n");
			goto Label_exit;
		}
	}
	
	printf("Size of input_image = [%d %d %d %d].\n", sz_input[0], sz_input[1], sz_input[2], sz_input[3]);
	
	if (channelNo >= sz_input[3])
	{
		fprintf (stderr, "The reference channelNo of is invalid (bigger than the number of channels the images have). Exit. \n");
		goto Label_exit;
	}
	
	//read the initial control point file
	
	if (!dfile_prior)
	{
		fprintf (stderr, "You have not specified any prior control point file! Exit. \n");
		goto Label_exit;
	}
	else
	{
		FILE *tmp_fp = fopen(dfile_prior, "r");
		if (!tmp_fp)
		{
			fprintf (stderr, "You have specified a prior control point file that does not exist!. Exit. \n");
			goto Label_exit;
		}
		else
		{
			fclose(tmp_fp);
			mCoord = readControlPointFile2d(dfile_prior); //read the control point location file
			for (int tmpi=0; tmpi<mCoord.size(); tmpi++)
				printf("%d : %d, %d\n", tmpi, int(mCoord.at(tmpi).x), int(mCoord.at(tmpi).y));
		}
	}
	
	//do the computation
	{
		BDB_Minus_ConfigParameter mypara;
		mypara.f_image = alpha;
		mypara.f_length = beta;
		mypara.f_smooth = gamma; 
		mypara.b_adjust_tip = false;
		mypara.nloops = nloops;
		mypara.radius = radius;
		mypara.TH = 0.1;
	
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz_input[0], sz_input[1], sz_input[2], sz_input[3], img_input);
		
		//for (int k=0;k<sz_input[2];k++)
		int k=sz_input[2]/2;
		{
			point_bdb_minus_2d_localwinmass(inimg_4d[channelNo][k], sz_input[0], sz_input[1], mCoord, mypara);
		}
		
		delete4dpointer(inimg_4d, sz_input[0], sz_input[1], sz_input[2], sz_input[3]);	 
	}
	
	//now save the coordinates to a file
	{
		FILE *tmp_fp = fopen(dfile_output, "w");
		if (!tmp_fp)
		{
			fprintf (stderr, "You cannot open the file [%s] to write!. Exit. \n", dfile_output);
			goto Label_exit;
		}
		else
		{
			fprintf(tmp_fp, "x,y,z\n");
			for (int tmpi=0; tmpi<mCoord.size(); tmpi++)
				fprintf(tmp_fp, "%d, %d, 42\n", int(mCoord.at(tmpi).x+1), int(mCoord.at(tmpi).y+1));
			fclose(tmp_fp);
		}
	}

	// save to output file 
	
	if (img_output && sz_output)
	{
		switch (datatype_output)
		{
			case 1:
				if (saveImage(dfile_output, (const unsigned char *)img_output, sz_output, sizeof(unsigned char))!=true) 
				{
					fprintf(stderr, "Error happens in file writing. Exit. \n");
				}
				break;
				
			case 2:
				if (saveImage(dfile_output, (const unsigned char *)img_output, sz_output, 2)!=true) 
				{
					fprintf(stderr, "Error happens in file writing. Exit. \n");
				}
				break;
				
			case 4:
				if (saveImage(dfile_output, (const unsigned char *)img_output, sz_output, 4)!=true) 
				{
					fprintf(stderr, "Error happens in file writing. Exit. \n");
				}
				break;
				
			default:
				fprintf(stderr, "Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
				goto Label_exit;
				return 1;
		}
		printf("The warped image has been saved to the file [%s].\n", dfile_output);
	}
	
	// clean all workspace variables 

Label_exit:
		
	if (img_input) {delete [] img_input; img_input=0;}
	if (sz_input) {delete [] sz_input; sz_input=0;}
	
	if (img_output) {delete [] img_output; img_output=0;}
	if (sz_output) {delete [] sz_output; sz_output=0;}
	
	return 0;
}



void parse_Radii_Paremeter(char * optarg, double * &radius, V3DLONG & len_Radii)
{
	//the input must be set as 0
	if (!radius)
	{
		delete []radius;
		radius=0;
	}
	
	if (len_Radii!=0)
		len_Radii=0;
	
	//
	
	V3DLONG i=0, len, n;
	while (optarg[i++]!='\0') {}
	len=i-1;
	if (optarg[0]!='[' || optarg[len-1]!=']' || len>=1024)
	{
		printf("The radii parameter must have the format [xxx,xxx,...] where comma is the separator and no space is allowed, the first and last characters must be brackets.\n");
		return;
	}
	
	n=1;
	for (i=1;i<=len-2;i++)
	{
		n = (optarg[i]==',')?n+1:n;
		if ((optarg[i]>'9' || optarg[i] <'0') && optarg[i]!=',')
		{
			printf("Detected illegal characters in the radii parameters.\n");
			return;
		}
	}
	
	radius = new double [n];
	if (!radius)
	{
		printf("Fail to allocate memory for radii.\n");
		len_Radii=0;
		return;
	}
	
	char buf[1024]; //use a big 1K buffer
	
	if (n==1)
	{
		strncpy(buf, optarg+1, len-2); buf[len-2]='\0';
		radius[0] = atof(buf);
		len_Radii=1;
		return;
	}
	
	V3DLONG cb,ce,cb_new=1;
	int k=0;
	printf("n=%d\n",n);
	while (k<n)
	{
		cb=cb_new;
		int b_foundcomma=0;
		for (int i=cb;i<=len-2;i++)
		{
			if (optarg[i]==',')
			{
				ce = i-1;
				if (ce<cb) //detect a case like ",," or "[,"
				{
					b_foundcomma=1;
					cb_new=ce+2;
					break;
				}
				
				if (ce-cb+1>3)
				{
					printf("You have specified a number with more than 3 digits V3DLONG, which is unlikely the correct input. Exit!");
					exit(1);
				}
				else
				{
					strncpy(buf, optarg+cb, ce-cb+1); buf[ce-cb+1]='\0';
					radius[k] = atof(buf);
				}
				cb_new=ce+2;
				b_foundcomma=1;
				break;
			}
		}
		if (b_foundcomma==0) //this is the last value which has no comma left
		{
			ce=len-2;
			if (ce>=cb)
			{
				strncpy(buf, optarg+cb, ce-cb+1); buf[ce-cb+1]='\0';
				radius[k] = atof(buf);
			}
		}
		
		if (ce<cb) //i.e. ce-cb+1=1, or the case of ",,",",]","[,"
		{
			n--; //in this case, decease the total valid number
		}
		else
		{
			k++;
		}
	}
	len_Radii=n;
	printf("You have specified radius range to be %d number series {", len_Radii);
	for(i=0;i<len_Radii;i++)
	{
		printf("%5.2f",radius[i]);
		if (i<len_Radii-1) printf(",");
	}
	printf("}\n");
    
	return;
}

bool convert_type2uint8_3dimg_1dpt(void * &img, V3DLONG * sz, int datatype)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to convert_type2uint8_3dimg_1dpt() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
	
	if (datatype!=2 && datatype!=4)
	{
		fprintf(stderr, "This function convert_type2uint8_3dimg_1dpt() is designed to convert 16 bit and single-precision-float only.\n", __FILE__, __LINE__);
		return false;
	}
	
	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1 || sz[0]>2048 || sz[1]>2048 || sz[2]>300 || sz[3]>3)
	{
		fprintf(stderr, "Input image size is not valid .\n", __FILE__, __LINE__);
		return false;
	}

	V3DLONG totalunits = sz[0] * sz[1] * sz[2] * sz[3];
	unsigned char * outimg = new unsigned char [totalunits];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
	
	if (datatype==2)
	{
		unsigned short int * tmpimg = (unsigned short int *)img;
		for (V3DLONG i=0;i<totalunits;i++)
		{	
			outimg[i] = (unsigned char)(tmpimg[i]>>4); //as I knew it is 12-bit instead of 16 bit 
													   //note: 071120: seem because I did not read 12-bit data from LSM, the pointer operation tmpimg[i] will crash!!. 
			                                           //Thus as of today (071120 I temperarily pause to deal with 12-bit data)
													   //080302: change /16 to >>4
		}
	}
	else
	{
		float * tmpimg = (float *)img;
		for (V3DLONG i=0;i<totalunits;i++)
		{	
			outimg[i] = (unsigned char)(tmpimg[i]*255); //as I knew it is float between 0 and 1
		}
	}
	
	//copy to output data
	
	delete [] ((unsigned char *)img); //as I know img was originally allocated as (unsigned char *)
	img = outimg;
	
	return true;
}


vector<Coord2D> readControlPointFile2d(string posFile)
{
  vector<Coord2D> coordPos;
  Coord2D c2d;
  
  char curline[2000];
  ifstream file_op;
  file_op.open(posFile.c_str());
  if (!file_op)
  {
    fprintf(stderr, "Fail to open the pos file [%s]\n", posFile.c_str());
	return coordPos;
  }

  V3DLONG xpos, ypos;  xpos=ypos=-1;//set as default
  V3DLONG k=0;
  while(!file_op.eof())
  {
	file_op.getline(curline, 2000);
	//cout<<curline<<endl;
	k++;
	if (k>0) //ignore the first line
	{
	  sscanf(curline, "%ld,%ld", &xpos, &ypos);
	  if (xpos==-1 || ypos==-1)
	  {
	    continue;
	  }
	  else
	  {
	    c2d.x = xpos; 
		c2d.y = ypos;
	    coordPos.push_back(c2d);
	  }
	  xpos=ypos=-1; //reset to default
	}
  }
  file_op.close();

  return coordPos;
}

vector<Coord3D> readControlPointFile3d(string posFile)
{
  vector<Coord3D> coordPos;
  Coord3D c3d;
  
  char curline[2000];
  ifstream file_op;
  file_op.open(posFile.c_str());
  if (!file_op)
  {
    fprintf(stderr, "Fail to open the pos file [%s]\n", posFile.c_str());
	return coordPos;
  }

  V3DLONG xpos, ypos, zpos;  xpos=ypos=zpos=-1;//set as default
  V3DLONG k=0;
  while(!file_op.eof())
  {
	file_op.getline(curline, 2000);
	//cout<<curline<<endl;
	k++;
	if (k>0) //ignore the first line
	{
	  sscanf(curline, "%ld,%ld,%ld", &xpos, &ypos, &zpos);
	  if (xpos==-1 || ypos==-1 || zpos==-1)
	  {
	    continue;
	  }
	  else
	  {
	    c3d.x = xpos; 
		c3d.y = ypos;
		c3d.z = zpos;
	    coordPos.push_back(c3d);
	  }
	  xpos=ypos=zpos=-1; //reset to default
	}
  }
  file_op.close();

  return coordPos;
}

