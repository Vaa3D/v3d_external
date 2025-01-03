//wkernel.h: the program to define kernels
// by Hanchuan Peng
// 2006-2011

#include "wkernel.h"

#include <math.h>
#include <stdio.h>

WeightKernel3D::WeightKernel3D(V3DLONG xw, V3DLONG yw, V3DLONG zw)
{
	initData();
	createInternalData(xw, yw, zw, (xw-1)/2, (yw-1)/2, (zw-1)/2);
}

WeightKernel3D::WeightKernel3D(V3DLONG r)
{
	initData();
	V3DLONG xw, yw, zw;
	rwid = r;
	xw = yw = zw = 2*rwid+1;
	createInternalData(xw, yw, zw, (xw-1)/2, (yw-1)/2, (zw-1)/2);
}

WeightKernel3D::~WeightKernel3D()
{
	cleanData();
}

void WeightKernel3D::generateKernel(KernelType t, int pp, int qq, int rr)
{
	if (!data3d || !data1d)
	{
	    printf("Invalid data pointers. Do nothing. [%ld]\n", totalLen);
		return;
	}
	
	V3DLONG i,j,k;
	V3DLONG ie, ib, je, jb, ke, kb;
	double dz, dy, dx;
	V3DLONG r2 = rwid*rwid;
	
	switch (t)
	{
		case KT_CUBE_ALL1:
			ib = 0; ie = xwid-1;
			jb = 0; je = ywid-1;
			kb = 0; ke = zwid-1;
			
			for (k=kb;k<=ke;k++)
			{
				for (j=jb;j<=je;j++)
				{
					for (i=ib;i<=ie;i++)
					{
						data3d[k][j][i] = 1;
					}
				}
			}
				
			break;	  
			
		case KT_SPHERE_ALL1:
			ib = 0; ie = xwid-1;
			jb = 0; je = ywid-1;
			kb = 0; ke = zwid-1;
			
			for (k=kb;k<=ke;k++)
			{
				dz = double(k-z0)/((zwid-1)/2);
				for (j=jb;j<=je;j++)
				{
					dy = double(j-y0)/((ywid-1)/2);
					for (i=ib;i<=ie;i++)
					{
						dx = double(i-x0)/((xwid-1)/2);
						if (dx*dx+dy*dy+dz*dz>1) 
                            data3d[k][j][i] = 0;
						else
							data3d[k][j][i] = 1;
					}
				}
			}
				
			break;	  
			
		default:
			printf("Unsupported Kernel Type! Do nothing.\n");
			break;
	}
}

int WeightKernel3D::createInternalData(V3DLONG xw, V3DLONG yw, V3DLONG zw, V3DLONG xw0, V3DLONG yw0, V3DLONG zw0)
{
	if (data1d || data3d)
	{
		printf("Data already exist. Do nothing.\n");
		return 1; 
	}
	
	if (xw<=0 || yw <=0 || zw <=0)
	{
		printf("Invalid width parameters. Do nothing.\n");
		return 1; 
	}
	
	if (xw0<0 || xw0 >= xw || yw0<0 || yw0 >= yw || zw0<0 || zw0 >= zw)
	{
		printf("Invalid center parameters. Do nothing.\n");
		return 1; 
	}
	
	totalLen = xw * yw * zw;
	data1d = new double [totalLen];
	if (!data1d)
	{
		printf("Fail to allocate memory for the kernel (1d vector).\n");
		return 1; 
	}
	
	data3d = new double ** [zw];
	if (!data3d)
	{
		printf("Fail to allocate memory for the kernel (3d array).\n");
		if (data1d)
		{
			delete []data1d;
			data1d = 0;
		}
		return 1; 
	}
	else
	{
		for (V3DLONG k=0;k<zw; k++)
		{
			data3d[k] = 0;
			data3d[k] = new double * [yw];
			if (!data3d[k])
			{
				for (V3DLONG j=0;j<k;j++)
				{
					if (data3d[j])
					{
						delete [] (data3d[j]);
						data3d[j] = 0;
					}
				}
				
				printf("Fail to allocate memory for the kernel (3d array).\n");
				
				if (data3d)
				{
					delete [] data3d;
					data3d = 0;
				}
				
				if (data1d)
				{
					delete []data1d;
					data1d = 0;
				}
				
				return 1;
			}
			else
			{
				for (V3DLONG j=0;j<yw; j++)
				{
					data3d[k][j] = data1d + k*yw*xw + j*xw;
				}
			}
		}
	}
	
	x0 = xw0;
	y0 = yw0;
	z0 = zw0;
	
	xwid = xw;
	ywid = yw;
	zwid = zw;
	
	return 0; 
}

void WeightKernel3D::cleanData() 
{
    if (data3d)
	{
		for (V3DLONG k=0; k<zwid;k++)
		{
			if (data3d[k])
			{
				delete [] (data3d[k]);
				data3d[k] = 0;
			}
		}
		delete [] data3d;
		data3d = 0;
	}
	if (data1d)
	{
		delete [] data1d;
		data1d = 0;
	}
	x0=y0=z0=0; 
	rwid=0;
	xwid=ywid=zwid=0; 
	totalLen = xwid*ywid*zwid;
}

void WeightKernel3D::initData() 
{
	data1d = 0;
	data3d = 0; 
	x0=y0=z0=0; 
	rwid=xwid=ywid=zwid=0; 
	totalLen = xwid*ywid*zwid; 
}




