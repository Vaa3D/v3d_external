#ifndef TIFF_ROTATE_H
#define TIFF_ROTATE_H

bool rotate_along_xaxis(double theta, unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, bool keep_size = true);
bool rotate_along_yaxis(double theta, unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, bool keep_size = true);
bool rotate_along_zaxis(double theta, unsigned char* inmg1d, V3DLONG * in_sz, unsigned char* & outimg1d, V3DLONG * & out_sz, bool keep_size = true);

#endif
