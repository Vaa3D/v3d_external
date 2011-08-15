#ifndef _IMG_SAMPLING_H_
#define _IMG_SAMPLING_H_

#include "basic_memory.h"

template<class T> bool down_sampling(int factor, T* &inimg1d, V3DLONG * &in_sz, T* &outimg1d, V3DLONG * &out_sz)
{
	if(inimg1d == 0 || in_sz == 0 || in_sz[0] < factor || in_sz[1] < factor || in_sz[2] < factor) return false;
	if(out_sz == 0) out_sz = new V3DLONG[4];
	T *** inimg3d = 0, *** outimg3d = 0;
	V3DLONG tol_sz = in_sz[0] * in_sz[1] * in_sz[2];
	V3DLONG smp_sz = tol_sz / factor / factor / factor;
	out_sz[0] = in_sz[0] / factor;
	out_sz[1] = in_sz[1] / factor;
	out_sz[2] = in_sz[2] / factor;
	out_sz[3] = 1; //in_sz[3];
	try{
		if(outimg1d == 0) outimg1d = new T[smp_sz];
		new3dpointer(inimg3d, in_sz[0], in_sz[1], in_sz[2], inimg1d);
		new3dpointer(outimg3d, out_sz[0], out_sz[1], out_sz[2], outimg1d);
	}
	catch(...)
	{
		if(inimg3d)delete3dpointer(inimg3d, in_sz[0], in_sz[1], in_sz[2]);
		if(outimg3d)delete3dpointer(outimg3d, out_sz[0], out_sz[1], out_sz[2]);
		return false;
	}
	for(V3DLONG k = 0; k < out_sz[2]; k++)
	{
		V3DLONG kk = k * factor;
		for(V3DLONG j = 0; j < out_sz[1]; j++)
		{
			V3DLONG jj = j * factor;
			for(V3DLONG i = 0; i < out_sz[0]; i++)
			{
				V3DLONG ii = i * factor;
				outimg3d[k][j][i] = inimg3d[kk][jj][ii];
			}
		}
	}

	if(inimg3d)delete3dpointer(inimg3d, in_sz[0], in_sz[1], in_sz[2]);
	if(outimg3d)delete3dpointer(outimg3d, out_sz[0], out_sz[1], out_sz[2]);
	return true;
}
#endif
