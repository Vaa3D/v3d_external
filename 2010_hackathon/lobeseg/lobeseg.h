//lobeseg.h
//created by Hanchuan Peng
//080822


#ifndef __LOBESEG__H__
#define __LOBESEG__H__

class BDB_Minus_ConfigParameter;
bool do_lobeseg_bdbminus(unsigned char *inimg1d, const V3DLONG sz[4], unsigned char *outimg1d, int in_channel_no, int out_channel_no, const BDB_Minus_ConfigParameter & mypara);

#endif


