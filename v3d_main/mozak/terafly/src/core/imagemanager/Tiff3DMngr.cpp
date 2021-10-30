//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
*******************
* 2015-03-03. Giulio.     @FIXED RGB photometric interprettion has to be set when there is more than one channel 
* 2015-02-06. Giulio.     @ADDED append operation that assume an already open and positioned file
* 2015-02-06. Giulio.     @ADDED open operation
* 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
* 2014-12-10. Giulio.     @ADDED added management of mismatch between machine/image endian
* 2014-12-06. Giulio.     @FIXED input file should NOT be closed at the end of 'loadTiff3D2Metadata'
* 2014-12-05. Giulio.     @ADDED input file should be closed at the end of 'loadTiff3D2Metadata'
*/

#include "Tiff3DMngr.h"
#include <stdlib.h> // needed by clang: defines size_t
#include <string.h>
#include "tiffio.h"

#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
#include <QElapsedTimer>
#include "PLog.h"
#include "COperation.h"
#endif




static
void swap2bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
}

static
void swap4bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+3);
    *(tp+3) = a;
    a = *(tp+1);
    *(tp+1) = *(tp+2);
    *(tp+2) = a;
}


char *loadTiff3D2Metadata ( char * filename, unsigned int &sz0, unsigned int  &sz1, unsigned int  &sz2, unsigned int  &sz3, int &datatype, int &b_swap, void * &fhandle, int &header_len ) {

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffLoadMetadata)
    #endif

	uint32 XSIZE;
	uint32 YSIZE;
	uint16 bpp;
	uint16 spp;
	uint16 Cpage;
	uint16 Npages;
    TIFF *input;
    int check;

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	input=TIFFOpen(filename,"r");
	if (!input)
    {
		//throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Cannot open the file %s",finName).c_str());
		return ((char *) "Cannot open the file.");
    }

	check=TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &XSIZE);
	if (!check)
	{
		TIFFClose(input);
		//throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Image length of %s undefined\n", finName).c_str());
		return ((char *) "Image width of undefined.");
	}		
    
	check=TIFFGetField(input, TIFFTAG_IMAGELENGTH, &YSIZE);
	if (!check)
	{
		TIFFClose(input);
		//throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Image length of %s undefined\n", finName).c_str());
		return ((char *) "Image length of undefined.");
	}		
    
	check=TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &bpp); 
	if (!check)
	{
		TIFFClose(input);
		//throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Undefined bits per sample in %s \n", finName).c_str());
		return ((char *) "Undefined bits per sample.");
	}

	check=TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &spp);
	if (!check)
	{
		TIFFClose(input);
		//throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Undefined bits per sample in %s \n", finName).c_str());
		return ((char *) "Undefined samples per pixel.");
	}

	// Onofri
	check=TIFFGetField(input, TIFFTAG_PAGENUMBER, &Cpage, &Npages);
	if (!check || Npages==0) { // the tag has not been read correctly
		// Add warning?
		Npages = 0;
		do {
			Npages++;
		} while ( TIFFReadDirectory(input) );
	}

	sz0 = XSIZE;
	sz1 = YSIZE;
	sz2 = Npages;
	sz3 = spp;
	datatype = bpp/8;

	//b_swap = 0;
	b_swap=TIFFIsByteSwapped(input);
	fhandle = (void *) input;
	header_len = -1;

	// the file must non be closed (it is responsibility of the caller)
	//TIFFClose(input);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffLoadMetadata, itm::IO, itm::strprintf("successfully loaded metadata from file \"%s\"", filename))
    #endif

	return ((char *) 0);
}

char *openTiff3DFile ( char *filename, char *mode, void *&fhandle ) {
	char *completeFilename = (char *) 0;
	int fname_len = (int) strlen(filename);
	char *suffix = strstr(filename,".tif");
	while ( suffix && (fname_len - (suffix-filename) > 5) )
		suffix = strstr(suffix+4,".tif");
	//if ( (suffix != 0) && (fname_len - (suffix-filename) <= 5) ) { // a substring ".tif is already at the end of the filename
	if ( suffix ) { // a substring ".tif is already at the very end of the filename
		completeFilename = new char[fname_len+1];
		strcpy(completeFilename,filename);
	}
	else {	
		completeFilename = new char[fname_len+4+1];
		strcpy(completeFilename,filename);
		strcat(completeFilename,".");
		strcat(completeFilename,TIFF3D_SUFFIX);
	}

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	fhandle = TIFFOpen(completeFilename,mode);
	if (!fhandle)
    {
		return ((char *) "Cannot open the file.");
    }
	return ((char *) 0);
}

void closeTiff3DFile ( void *fhandle ) {
	TIFFClose((TIFF *) fhandle);
}

char *initTiff3DFile ( char *filename, unsigned int sz0, unsigned int sz1, unsigned int sz2, unsigned int sz3, int datatype ) {
//int initTiff3DFile ( char *filename, uint32 XSIZE, uint32 YSIZE, uint16 spp, uint16 Npages, int datatype){

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffInitData)
    #endif

	uint32 XSIZE  = sz0;
	uint32 YSIZE  = sz1;
	uint16 Npages = sz2;
	uint16 spp    = sz3;

	uint16 bpp=8 * datatype;
	unsigned char *fakeData=new unsigned char[XSIZE * YSIZE];
	
	int check;

	if ( sz3 == 1 )
		spp = sz3; 
	else if ( sz3 < 4 )
		spp = 3;
	else
		return ((char *) "More than 3 channels in Tiff files.");

	char *completeFilename = (char *) 0;
	int fname_len = (int) strlen(filename);
	char *suffix = strstr(filename,".tif");
	while ( suffix && (fname_len - (suffix-filename) > 5) )
		suffix = strstr(suffix+4,".tif");
	//if ( (suffix != 0) && (fname_len - (suffix-filename) <= 5) ) { // a substring ".tif is already at the end of the filename
	if ( suffix ) { // a substring ".tif is already at the very end of the filename
		completeFilename = new char[fname_len+1];
		strcpy(completeFilename,filename);
	}
	else {	
		completeFilename = new char[fname_len+4+1];
		strcpy(completeFilename,filename);
		strcat(completeFilename,".");
		strcat(completeFilename,TIFF3D_SUFFIX);
	}

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffInitData, itm::CPU, itm::strprintf("generated fake data for 3D tiff \"%s\"", completeFilename))
    TERAFLY_TIME_RESTART(TiffInitData)
    #endif

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	TIFF *output;
	output = TIFFOpen(completeFilename,"w");
	if (!output) {
		return ((char *) "Cannot open the file.");
    }

	check = TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
	if (!check) {
		return ((char *) "Cannot set the image width.");
    }

	check = TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
	if (!check) {
		return ((char *) "Cannot set the image width.");
    }

	check = TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp); 
	if (!check) {
		return ((char *) "Cannot set the image width.");
    }

	check = TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
	if (!check) {
		return ((char *) "Cannot set the image width.");
    }

	check = TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, YSIZE); // one page per strip
	if (!check) {
		return ((char *) "Cannot set the image height.");
    }

	check = TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	if (!check) {
		return ((char *) "Cannot set the compression tag.");
    }

	check = TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	if (!check) {
		return ((char *) "Cannot set the planarconfig tag.");
    }

	if ( spp == 1 )
		check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	
	else // spp == 3
		check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);	
	if (!check) {
		return ((char *) "Cannot set the photometric tag.");
    }

	/* We are writing single page of the multipage file */
	check = TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
	if (!check) {
		return ((char *) "Cannot set the subfiletype tag.");
    }

	check = TIFFSetField(output, TIFFTAG_PAGENUMBER, 0, Npages); 
	if (!check) {
		return ((char *) "Cannot set the page number.");
    }


	check = (int)TIFFWriteEncodedStrip(output, 0, fakeData, XSIZE * YSIZE);
	if (!check) {
		return ((char *) "Cannot write encoded strip to file.");
    }

	delete[] fakeData;
	delete []completeFilename;

	check = TIFFWriteDirectory(output);
	if (!check) {
		return ((char *) "Cannot write a new directory.");
    }

	TIFFClose(output);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffInitData, itm::IO, itm::strprintf("written initialized 3D tiff \"%s\"", completeFilename))
    #endif

	return (char *) 0;
}

char *appendSlice2Tiff3DFile ( char *filename, int slice, unsigned char *img, unsigned int img_width, unsigned int img_height ) {
    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffAppendData)
    #endif

	TIFF *output;
	uint16 spp, bpp, NPages, pg0;

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	output=TIFFOpen(filename,"r");
	TIFFGetField(output, TIFFTAG_BITSPERSAMPLE, &bpp); 
	TIFFGetField(output, TIFFTAG_SAMPLESPERPIXEL, &spp);
	TIFFGetField(output, TIFFTAG_PAGENUMBER, &pg0, &NPages);
	TIFFClose(output);
	// since we are 
	output = (slice==0)? TIFFOpen(filename,"w") : TIFFOpen(filename,"a");

	TIFFSetDirectory(output,slice); // WARNING: slice must be the first page after the last, otherwise the file can be corrupted

	TIFFSetField(output, TIFFTAG_IMAGEWIDTH, img_width);
	TIFFSetField(output, TIFFTAG_IMAGELENGTH, img_height);
	TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp); 
	TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
	TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, img_height);
	TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	//TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	
	//TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);	
	// We are writing single page of the multipage file 
	TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
	TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, NPages); 

	TIFFWriteEncodedStrip(output, 0, img, img_width * img_height * spp * (bpp/8));
	//img +=  img_width * img_height;

	TIFFWriteDirectory(output);

	TIFFClose(output);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffAppendData, itm::IO, itm::strprintf("appended slice %d x %d to 3D tiff \"%s\"", img_width, img_height, filename))
    #endif

	return (char *) 0;
}

char *appendSlice2Tiff3DFile ( void *fhandler, int slice, unsigned char *img, unsigned int  img_width, unsigned int  img_height, int spp, int bpp, int NPages ) {
	TIFF *output = (TIFF *) fhandler;

	TIFFSetDirectory(output,slice); // WARNING: slice must be the first page after the last, otherwise the file can be corrupted

	TIFFSetField(output, TIFFTAG_IMAGEWIDTH, img_width);
	TIFFSetField(output, TIFFTAG_IMAGELENGTH, img_height);
	TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, (uint16)bpp); 
	TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, (uint16)spp);
	TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, img_height);
	TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	//TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	
	//TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);	
	// We are writing single page of the multipage file 
	TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
	TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, (uint16)NPages); 

	TIFFWriteEncodedStrip(output, 0, img, img_width * img_height * spp * (bpp/8));
	//img +=  img_width * img_height;

	TIFFWriteDirectory(output);

	return (char *) 0;
}

char *readTiff3DFile2Buffer ( char *filename, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last ) {

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffLoadData)
    #endif

    TIFF *input;

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	input=TIFFOpen(filename,"r");
	if (!input)
    {
		//throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Cannot open the file %s",finName).c_str());
		return ((char *) "Cannot open the file.");
    }
    
	int b_swap=TIFFIsByteSwapped(input);
	char *err_msg = readTiff3DFile2Buffer(input,img,img_width,img_height,first,last,b_swap);

	TIFFClose(input);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffLoadData, itm::IO, itm::strprintf("loaded block x(%d), y(%d), z(%d-%d) from 3D tiff \"%s\"", img_width, img_height, first, last, filename))
    #endif

	return err_msg;
}

char *readTiff3DFile2Buffer ( void *fhandler, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last, int b_swap ) {
	uint32 rps;
    uint16 spp, bpp, photo, comp, planar_config;
    int check, StripsPerImage,LastStripSize;

    TIFF *input = (TIFF *) fhandler;

	check=TIFFGetField(input, TIFFTAG_ROWSPERSTRIP, &rps);
	if (!check)
	{
		return ((char *) "Image length of undefined.");
	}	
	//rps=600;
    
	check=TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &bpp); 
	if (!check)
	{
		return ((char *) "Undefined bits per sample.");
	}

	check=TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &spp);
	if (!check)
	{
		return ((char *) "Undefined samples per pixel.");
	}

	check=TIFFGetField(input, TIFFTAG_PHOTOMETRIC, &photo);
	if (!check)
	{
		return ((char *) "Cannot determine photometric interpretation.");
	}

	check=TIFFGetField(input, TIFFTAG_COMPRESSION, &comp);
	if (!check)
	{
		return ((char *) "Cannot determine compression technique.");
	}

	check=TIFFGetField(input, TIFFTAG_PLANARCONFIG, &planar_config);
	if (!check)
	{
		return ((char *) "Cannot determine planar configuration.");
	}


	StripsPerImage =  (img_height + rps - 1) / rps;
	LastStripSize = img_height % rps;
	if (LastStripSize==0)
		LastStripSize=rps;

	check=TIFFSetDirectory(input, first);
	if (!check)
	{
		return ((char *) "Cannot open the requested first strip.");
	}

	unsigned char *buf = img;
	int page=0;
	do{

		for (int i=0; i < StripsPerImage-1; i++){
			if (comp==1) {
				TIFFReadRawStrip(input, i, buf, spp * rps * img_width * (bpp/8));
				buf = buf + spp * rps * img_width * (bpp/8);
			}
			else{
				TIFFReadEncodedStrip(input, i, buf, spp * rps * img_width * (bpp/8));
				buf = buf + spp * rps * img_width * (bpp/8);
			}
		}

		if (comp==1) {
			TIFFReadRawStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
		}
		else{
			TIFFReadEncodedStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
		}
		buf = buf + spp * LastStripSize * img_width * (bpp/8);

		page++;
	
	}while ( page < static_cast<int>(last-first+1) && TIFFReadDirectory(input));//while (TIFFReadDirectory(input));

	// input file is assumedo ti be already open and it is provided as an handler; the file should be closed by caller
	//TIFFClose(input);  

	if ( page < static_cast<int>(last-first+1) ){
		return ((char *) "Cannot read all the pages.");
	}

	// swap the data bytes if necessary 	
	if (b_swap)
	{
		int i;
		size_t total = img_width * img_height * spp * (last-first+1);
		if (bpp/8 == 2)
		{
			for (i=0;i<total; i++)
			{
				swap2bytes((void *)(img+2*i));
			}
		}
		else if (bpp/8 == 4)
		{
			for (i=0;i<total; i++)
			{
				swap4bytes((void *)(img+4*i));
			}
		}
	}

	return (char *) 0;
}
