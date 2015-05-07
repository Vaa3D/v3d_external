//by Hanchuan Peng
// 2015-05-07. Requested and encouraged by Greg Jefferis @ Cambridge

#ifndef __NRRD_H__
#define __NRRD_H__

class Image4DSimple;
bool read_nrrd(char *filename, Image4DSimple *&img);
bool write_nrrd(char *filename, Image4DSimple *img);

#endif

