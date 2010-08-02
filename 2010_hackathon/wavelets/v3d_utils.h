#include <v3d_interface.h>
#include <v3d_basicdatatype.h>

class v3d_utils
{
public:
	static unsigned char* doubleArrayToCharArray(double* data1dD, int numVoxels, ImagePixelType dataType);
	double* channelToDoubleArray(Image4DSimple* inputImage, int channel);

};