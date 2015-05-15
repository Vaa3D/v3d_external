//by Hanchuan Peng
// 2015-05-07. Requested and encouraged by Greg Jefferis @ Cambridge

#ifndef __NRRD_H__
#define __NRRD_H__

class Image4DSimple;
bool read_nrrd(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype);
bool read_nrrd_with_pxinfo(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype, float pixelsz[4]);

bool write_nrrd(char imgSrcFile[], unsigned char * data1d, V3DLONG sz[4], int datatype);
bool write_nrrd_with_pxinfo(char imgSrcFile[], unsigned char * data1d, V3DLONG sz[4], int datatype, float pixelsz[4]);

#endif

