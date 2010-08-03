#ifndef __WAVELETCONFIGEXCEPTION_H__
#define __WAVELETCONFIGEXCEPTION_H__

#include <stdlib.h>

class WaveletConfigException
{
	public:
  	
  	const char* message;
  
  	WaveletConfigException();
  	WaveletConfigException(const char* exceptionMessage);
  	const char* what() const throw();
};

#endif
