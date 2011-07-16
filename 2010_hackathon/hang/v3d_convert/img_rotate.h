#ifndef TIFF_ROTATE_H
#define TIFF_ROTATE_H

bool rotate_along_xaxis(unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, float theta, bool keep_size = true);
bool rotate_along_yaxis(unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, float theta, bool keep_size = true);
bool rotate_along_zaxis(unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, float theta, bool keep_size = true);

#endif
