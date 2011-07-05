/* recenterimageplugin.cpp
 * 2009-08-14: created by Yang Yu
 * 2010-11-23: supporting all kinds of datatypes, changed by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "recenterimageplugin.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(recenterimage, ReCenterImagePlugin)

template <class Tidx, class Tdata>
void recentering(Tdata *&p, Tdata *data, Tidx nx, Tidx ny, Tidx nz, Tidx ox, Tidx oy, Tidx oz, Tidx ncolor);


QStringList ReCenterImagePlugin::menulist() const
{
    return QStringList() << tr("ReCenterImage")
						<< tr("About this plugin");
}

void ReCenterImagePlugin::processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
{
    if (! p4DImage) return;

	Image4DProxy<Image4DSimple> p4DProxy(p4DImage);
	
    //void* data1d = p4DProxy.begin();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
	
	int datatype = p4DImage->getDatatype();
	qDebug()<<"datatype is "<<datatype;

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
			// For different datatype
			if(datatype == 1)
			{
				unsigned char *pImage = NULL;
				recentering<V3DLONG, unsigned char>( pImage, (unsigned char*)p4DProxy.begin(), ndimx, ndimy, ndimz, N, M, P, sc);
				
				p4DImage->setData((unsigned char*)pImage, ndimx, ndimy, ndimz, sc, p4DImage->getDatatype()); // update data in current window
					
			}
			else if(datatype == 2)
			{
				short int *pImage = NULL;
				recentering<V3DLONG, short int>( pImage, (short int*)p4DProxy.begin(), ndimx, ndimy, ndimz, N, M, P, sc);
				
				p4DImage->setData((unsigned char*)pImage, ndimx, ndimy, ndimz, sc, p4DImage->getDatatype()); // update data in current window
			}
			else if(datatype == 3)
			{
				float *pImage = NULL;
				recentering<V3DLONG, float>( pImage, (float*)p4DProxy.begin(), ndimx, ndimy, ndimz, N, M, P, sc);
				
				p4DImage->setData((unsigned char*)pImage, ndimx, ndimy, ndimz, sc, p4DImage->getDatatype()); // update data in current window
			}
			else 
			{
				return;
			}

		}
	}
	else if (arg == tr("About this plugin"))
	{
		QMessageBox::information(parent, "Version info", "Plugin Recenter version 1.0 developed by Yang Yu.");
	}
	else
		return;

}

template <class Tidx, class Tdata>
void recentering(Tdata *&p, Tdata *data, Tidx nx, Tidx ny, Tidx nz, Tidx ox, Tidx oy, Tidx oz, Tidx ncolor)
{
	
	if(p) {delete []p; p=NULL;}
	else
	{
		Tidx nplxs = nx*ny*nz*ncolor;
		Tidx pagesz = ox*oy*oz;
		
		//Initial New image
		try
		{
			p = new Tdata [nplxs];
			for(Tidx i=0; i<nplxs; i++)
			{
				p[i] = 0;
			}
		}
		catch(...)
		{
			printf("Error allocating memory for new image!\n");
			return;
		}
		
		//recenter
		Tidx centerx = ox/2;
		Tidx centery = oy/2;
		Tidx centerz = oz/2;
		
		Tidx ncenterx = nx/2;
		Tidx ncentery = ny/2;
		Tidx ncenterz = nz/2;
		
		//shift
		Tidx leftx = fabs(ncenterx-centerx);
		Tidx rightx = fabs(ox + leftx);
		if(ox>nx)
		{
			rightx = fabs(nx + leftx);
		}
		
		Tidx lefty = fabs(ncentery - centery);
		Tidx righty = fabs(oy + lefty);
		if(oy>ny)
		{
			righty = fabs(ny + lefty);
		}
		
		Tidx leftz = fabs(ncenterz - centerz);
		Tidx rightz = fabs(oz + leftz);
		if(oz>nz)
		{
			rightz = fabs(nz + leftz);
		}
		
		//simple 8 cases
		if(nx<=ox)
		{
			if(ny<=oy)
			{
				if(nz<=oz)
				{
					//case 1
					qDebug()<< "case 1 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk =  offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}
				}
				else
				{
					//case 2
					qDebug()<< "case 2 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}
					
				}
			}
			else
			{
				if(nz<=oz)
				{
					//case 3
					qDebug()<< "case 3 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}
					
				}
				else
				{
					//case 4
					qDebug()<< "case 4 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}
					
				}
			}
		}
		else
		{
			if(ny<=oy)
			{
				if(nz<=oz)
				{
					//case 5
					qDebug()<< "case 5 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}
					
				}
				else
				{
					//case 6
					qDebug()<< "case 6 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}
					
				}
			}
			else
			{
				if(nz<=oz)
				{
					//case 7
					qDebug()<< "case 7 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}
					
				}
				else
				{
					//case 8
					qDebug()<< "case 8 ...";
					
					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}
					
				}
			}
		}
		
	}

}

