/* recenterimageplugin.cpp
 * 2009-08-14: created by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "recenterimageplugin.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(recenterimage, ReCenterImagePlugin)


QStringList ReCenterImagePlugin::menulist() const
{
    return QStringList() << tr("ReCenterImage");
		//				<< tr("version of this plugin");
}

void ReCenterImagePlugin::processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
{
    if (! p4DImage) return;

    unsigned char* data1d = p4DImage->getRawData();
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    //define datatype here
    //


    if (arg == tr("ReCenterImage"))
    {
        bool ok;
        V3DLONG ndimx = QInputDialog::getInteger(parent, tr("Dimension x"),
                                                 tr("Enter dimx:"),
                                                 N, 0, 10000, 1, &ok);

        V3DLONG ndimy = QInputDialog::getInteger(parent, tr("Dimension y"),
                                                 tr("Enter dimy:"),
                                                 M, 0, 10000, 1, &ok);

        V3DLONG ndimz = QInputDialog::getInteger(parent, tr("Dimension z"),
                                                 tr("Enter dimz:"),
                                                 P, 0, 10000, 1, &ok);

		qDebug("dimx %ld dimy %ld dimz %ld \n", ndimx, ndimy, ndimz);

		V3DLONG ntotalpxls = sc*ndimx*ndimy*ndimz;

        if (ok)
        {
			//declare temporary pointer
			unsigned char *pImage = new unsigned char [ntotalpxls];
			if (!pImage)
			{
				printf("Fail to allocate memory.\n");
				return;
			 }
			 else
			 {
				 //Initial New image
				 for(V3DLONG c=0; c<sc; c++)
				 {
					 V3DLONG offsetc = c*ndimx*ndimy*ndimz;
					 for(V3DLONG k = 0; k<ndimz; k++)
					 {
						 V3DLONG offsetk = k*ndimx*ndimy;
						 for(V3DLONG j = 0; j<ndimy; j++)
						 {
							 V3DLONG offsetj = j*ndimx;
							 for(V3DLONG i=0; i<ndimx; i++)
							 {
								 pImage[offsetc + offsetk + offsetj + i] = 0;
							 }
						 }
					 }
				 }

				 //recenter
				 V3DLONG centerx = N/2;
				 V3DLONG centery = M/2;
				 V3DLONG centerz = P/2;

				 V3DLONG ncenterx = ndimx/2;
				 V3DLONG ncentery = ndimy/2;
				 V3DLONG ncenterz = ndimz/2;

				 //shift
				 V3DLONG leftx = fabs(ncenterx-centerx);
				 V3DLONG rightx = fabs(N + leftx);
				 if(N>ndimx)
				 {
					 rightx = fabs(ndimx + leftx);
				 }

				 V3DLONG lefty = fabs(ncentery - centery);
				 V3DLONG righty = fabs(M + lefty);
				 if(M>ndimy)
				 {
					 righty = fabs(ndimy + lefty);
				 }

				 V3DLONG leftz = fabs(ncenterz - centerz);
				 V3DLONG rightz = fabs(P + leftz);
				 if(P>ndimz)
				 {
					 rightz = fabs(ndimz + leftz);
				 }

				 //stupid 8 cases
				 if(ndimx<=N)
				 {
					 if(ndimy<=M)
					 {
						 if(ndimz<=P)
						 {
							 //case 1
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk =  k*N*M;
									 V3DLONG offsetnk = (k-leftz)*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = j*N;
										 V3DLONG offsetnj = (j-lefty)*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											 pImage[offsetnc + offsetnk + offsetnj + (i-leftx)] = data1d[offsetc + offsetk + offsetj + i];
										 }
									 }
								 }
							 }
						 }
						 else
						 {
							 //case 2
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk =  (k-leftz)*N*M;
									 V3DLONG offsetnk = k*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = j*N;
										 V3DLONG offsetnj = (j-lefty)*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											pImage[offsetnc + offsetnk + offsetnj + (i-leftx)] = data1d[offsetc + offsetk + offsetj + i];
										 }
									 }
								 }
							 }

						 }
					 }
					 else
					 {
						 if(ndimz<=P)
						 {
							 //case 3
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk =  k*N*M;
									 V3DLONG offsetnk = (k-leftz)*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = (j-lefty)*N;
										 V3DLONG offsetnj = j*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											 pImage[offsetnc + offsetnk + offsetnj + (i-leftx)] = data1d[offsetc + offsetk + offsetj + i];
										 }
									 }
								 }
							 }

						 }
						 else
						 {
							 //case 4
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk =  (k-leftz)*N*M;
									 V3DLONG offsetnk = k*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = (j-lefty)*N;
										 V3DLONG offsetnj = j*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											  pImage[offsetnc + offsetnk + offsetnj + (i-leftx)] = data1d[offsetc + offsetk + offsetj + i];
										 }
									 }
								 }
							 }

						 }
					 }
				 }
				 else
				 {
					 if(ndimy<=M)
					 {
						 if(ndimz<=P)
						 {
							 //case 5
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk =  k*N*M;
									 V3DLONG offsetnk = (k-leftz)*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = j*N;
										 V3DLONG offsetnj = (j-lefty)*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											  pImage[offsetnc + offsetnk + offsetnj + i] = data1d[offsetc + offsetk + offsetj + (i-leftx)];
										 }
									 }
								 }
							 }

						 }
						 else
						 {
							 //case 6
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk = (k-leftz)*N*M;
									 V3DLONG offsetnk = k*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = j*N;
										 V3DLONG offsetnj = (j-lefty)*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											 pImage[offsetnc + offsetnk + offsetnj + i] = data1d[offsetc + offsetk + offsetj + (i-leftx)];
										 }
									 }
								 }
							 }

						 }
					 }
					 else
					 {
						 if(ndimz<=P)
						 {
							 //case 7
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk =  k*N*M;
									 V3DLONG offsetnk = (k-leftz)*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = (j-lefty)*N;
										 V3DLONG offsetnj = j*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											 pImage[offsetnc + offsetnk + offsetnj + i] = data1d[offsetc + offsetk + offsetj + (i-leftx)];
										 }
									 }
								 }
							 }

						 }
						 else
						 {
							 //case 8
							 for(V3DLONG c=0; c<sc; c++)
							 {
								 V3DLONG offsetc = c*pagesz;
								 V3DLONG offsetnc = c*ndimx*ndimy*ndimz;
								 for(V3DLONG k=leftz; k<rightz; k++)
								 {
									 V3DLONG offsetk = (k-leftz)*N*M;
									 V3DLONG offsetnk = k*ndimx*ndimy;
									 for(V3DLONG j=lefty; j<righty; j++)
									 {
										 V3DLONG offsetj = (j-lefty)*N;
										 V3DLONG offsetnj = j*ndimx;
										 for(V3DLONG i=leftx; i<rightx; i++)
										 {
											pImage[offsetnc + offsetnk + offsetnj + i] = data1d[offsetc + offsetk + offsetj + (i-leftx)];
										 }
									 }
								 }
							 }

						 }
					 }
				 }

			 }

			//rescaling image and save to new 4dimage
			p4DImage->setData(pImage, ndimx, ndimy, ndimz, sc, p4DImage->getDatatype());

			//de-alloc
			//if(pImage) {delete []pImage; pImage=0;}

		}
	}
//	else if (arg == tr("version of this plugin"))
//	{
//		QMessageBox::information(parent, "Version info", "Plugin Recenter version 1.0 developed by Yang Yu.");
//	}
	else
		return;

}



