#include <iostream>
#include <string>
#include <list>

using namespace std;

struct InputParas
{
	list<string> filelist;
	
	bool is_resize;                 
	string para_resize;            

	bool is_negate;               
	string para_negate;

	bool is_reverse;
	string para_reverse;

	bool is_crop;
	string para_crop;

	bool is_colors;
	string para_colors;

	InputParas()
	{
		is_resize = is_negate = is_reverse = is_crop = is_colors = 0;
	}
};

void printHelp();
bool parse_paras(int argc, char* argv[], InputParas &paras, string &s_error);
bool run_with_paras(InputParas paras);

int main(int argc, char* argv[])
{
	if(argc == 1) {printHelp(); return 0;}

	InputParas paras;
	string s_error("");

	if(! parse_paras(argc, argv, paras, s_error))
	{
		cout<<"Invalid paras : "<< s_error<<endl;
		return 0;
	}
	else
	{
	}
	return 0;
}

bool parse_paras(int argc, char* argv[], InputParas &paras, string &s_error)
{
	int i = 1;      // switch ind

	while(i < argc)
	{
		if(string(argv[i]) == "-resize")
		{
			if(!paras.is_resize)
			{
				paras.is_resize = true;
        		if(++i < argc && argv[i][0] != '-') 
				{
					paras.para_resize = argv[i];
				}
				else
				{
					s_error += "need paras for -resize";
					return false;
				}
			}
			else
			{
				s_error += "duplicate -resize";
				return false;
			}
		}
		else if(string(argv[i]) == "-crop")
		{
		}
		else
		{
			if(argv[i][0] != '-')paras.filelist.push_back(string(argv[i]));
			else 
			{
				s_error += "unknow para ";
				s_error += argv[i];
				return false;
			}
		}
		i++;
	}
	return true;
}

bool run_with_paras(InputParas paras)
{
	return true;
}

void printHelp()
{
	cout<<"Version: 1.0"<<endl;
	cout<<"Copyright: Opensource Licence"<<endl;
	cout<<""<<endl;
	cout<<"v3d_convert is the extension of imagemagic convert. It is designed to support  image operator on three dimension."<<endl;
	cout<<"Currently support .raw .tiff/.tif .lsm image format."<<endl;
	cout<<""<<endl;
	cout<<"Usage: v3d_convert [options ...] file [ [options ...] file ...] [options ...] file "<<endl;
	cout<<""<<endl;
	/*
	cout<<"Image Operations:"<<endl;
	cout<<"  -adaptive-blur geometry"<<endl;
	cout<<"                       adaptively blur pixels; decrease effect near edges"<<endl;
	cout<<"  -adaptive-resize geometry"<<endl;
	cout<<"                       adaptively resize image using 'mesh' interpolation"<<endl;
	cout<<"  -adaptive-sharpen geometry"<<endl;
	cout<<"                       adaptively sharpen pixels; increase effect near edges"<<endl;
	cout<<"  -alpha option        on, activate, off, deactivate, set, opaque, copy"<<endl;
	cout<<"                       transparent, extract, background, or shape"<<endl;
	cout<<"  -annotate geometry text"<<endl;
	cout<<"                       annotate the image with text"<<endl;
	cout<<"  -auto-gamma          automagically adjust gamma level of image"<<endl;
	cout<<"  -auto-level          automagically adjust color levels of image"<<endl;
	cout<<"  -auto-orient         automagically orient (rotate) image"<<endl;
	cout<<"  -bench iterations    measure performance"<<endl;
	cout<<"  -black-threshold value"<<endl;
	cout<<"                       force all pixels below the threshold into black"<<endl;
	cout<<"  -blue-shift factor   simulate a scene at nighttime in the moonlight"<<endl;
	cout<<"  -blur geometry       reduce image noise and reduce detail levels"<<endl;
	cout<<"  -border geometry     surround image with a border of color"<<endl;
	cout<<"  -bordercolor color   border color"<<endl;
	cout<<"  -brightness-contrast geometry"<<endl;
	cout<<"                       improve brightness / contrast of the image"<<endl;
	cout<<"  -cdl filename        color correct with a color decision list"<<endl;
	cout<<"  -charcoal radius     simulate a charcoal drawing"<<endl;
	cout<<"  -chop geometry       remove pixels from the image interior"<<endl;
	cout<<"  -clamp               restrict pixel range from 0 to the quantum depth"<<endl;
	cout<<"  -clip                clip along the first path from the 8BIM profile"<<endl;
	cout<<"  -clip-mask filename  associate a clip mask with the image"<<endl;
	cout<<"  -clip-path id        clip along a named path from the 8BIM profile"<<endl;
	cout<<"  -colorize value      colorize the image with the fill color"<<endl;
	cout<<"  -color-matrix matrix apply color correction to the image"<<endl;
	cout<<"  -contrast            enhance or reduce the image contrast"<<endl;
	cout<<"  -contrast-stretch geometry"<<endl;
	cout<<"                       improve contrast by `stretching' the intensity range"<<endl;
	cout<<"  -convolve coefficients"<<endl;
	cout<<"                       apply a convolution kernel to the image"<<endl;
	cout<<"  -cycle amount        cycle the image colormap"<<endl;
	cout<<"  -decipher filename   convert cipher pixels to plain pixels"<<endl;
	cout<<"  -deskew threshold    straighten an image"<<endl;
	cout<<"  -despeckle           reduce the speckles within an image"<<endl;
	cout<<"  -distort method args"<<endl;
	cout<<"                       distort images according to given method ad args"<<endl;
	cout<<"  -draw string         annotate the image with a graphic primitive"<<endl;
	cout<<"  -edge radius         apply a filter to detect edges in the image"<<endl;
	cout<<"  -encipher filename   convert plain pixels to cipher pixels"<<endl;
	cout<<"  -emboss radius       emboss an image"<<endl;
	cout<<"  -enhance             apply a digital filter to enhance a noisy image"<<endl;
	cout<<"  -equalize            perform histogram equalization to an image"<<endl;
	cout<<"  -evaluate operator value"<<endl;
	cout<<"                       evaluate an arithmetic, relational, or logical expression"<<endl;
	cout<<"  -extent geometry     set the image size"<<endl;
	cout<<"  -extract geometry    extract area from image"<<endl;
	cout<<"  -fft                 implements the discrete Fourier transform (DFT)"<<endl;
	cout<<"  -flip                flip image vertically"<<endl;
	cout<<"  -floodfill geometry color"<<endl;
	cout<<"                       floodfill the image with color"<<endl;
	cout<<"  -flop                flop image horizontally"<<endl;
	cout<<"  -frame geometry      surround image with an ornamental border"<<endl;
	cout<<"  -function name parameters"<<endl;
	cout<<"                       apply function over image values"<<endl;
	cout<<"  -gamma value         level of gamma correction"<<endl;
	cout<<"  -gaussian-blur geometry"<<endl;
	cout<<"                       reduce image noise and reduce detail levels"<<endl;
	cout<<"  -geometry geometry   preferred size or location of the image"<<endl;
	cout<<"  -identify            identify the format and characteristics of the image"<<endl;
	cout<<"  -ift                 implements the inverse discrete Fourier transform (DFT)"<<endl;
	cout<<"  -implode amount      implode image pixels about the center"<<endl;
	cout<<"  -lat geometry        local adaptive thresholding"<<endl;
	cout<<"  -layers method       optimize, merge,  or compare image layers"<<endl;
	cout<<"  -level value         adjust the level of image contrast"<<endl;
	cout<<"  -level-colors color,color"<<endl;
	cout<<"                       level image with the given colors"<<endl;
	cout<<"  -linear-stretch geometry"<<endl;
	cout<<"                       improve contrast by `stretching with saturation'"<<endl;
	cout<<"  -liquid-rescale geometry"<<endl;
	cout<<"                       rescale image with seam-carving"<<endl;
	cout<<"  -median radius       apply a median filter to the image"<<endl;
	cout<<"  -modulate value      vary the brightness, saturation, and hue"<<endl;
	cout<<"  -monochrome          transform image to black and white"<<endl;
	cout<<"  -morphology method kernel"<<endl;
	cout<<"                       apply a morphology method to the image"<<endl;
	cout<<"  -motion-blur geometry"<<endl;
	cout<<"                       simulate motion blur"<<endl;
	cout<<"  -negate              replace every pixel with its complementary color "<<endl;
	cout<<"  -noise radius        add or reduce noise in an image"<<endl;
	cout<<"  -normalize           transform image to span the full range of colors"<<endl;
	cout<<"  -opaque color        change this color to the fill color"<<endl;
	cout<<"  -ordered-dither NxN"<<endl;
	cout<<"                       add a noise pattern to the image with specific"<<endl;
	cout<<"                       amplitudes"<<endl;
	cout<<"  -paint radius        simulate an oil painting"<<endl;
	cout<<"  -polaroid angle      simulate a Polaroid picture"<<endl;
	cout<<"  -posterize levels    reduce the image to a limited number of color levels"<<endl;
	cout<<"  -print string        interpret string and print to console"<<endl;
	cout<<"  -profile filename    add, delete, or apply an image profile"<<endl;
	cout<<"  -quantize colorspace reduce colors in this colorspace"<<endl;
	cout<<"  -radial-blur angle   radial blur the image"<<endl;
	cout<<"  -raise value         lighten/darken image edges to create a 3-D effect"<<endl;
	cout<<"  -random-threshold low,high"<<endl;
	cout<<"                       random threshold the image"<<endl;
	cout<<"  -region geometry     apply options to a portion of the image"<<endl;
	cout<<"  -render              render vector graphics"<<endl;
	cout<<"  -repage geometry     size and location of an image canvas"<<endl;
	cout<<"  -resample geometry   change the resolution of an image"<<endl;
	cout<<"  -resize geometry     resize the image"<<endl;
	cout<<"  -roll geometry       roll an image vertically or horizontally"<<endl;
	cout<<"  -rotate degrees      apply Paeth rotation to the image"<<endl;
	cout<<"  -sample geometry     scale image with pixel sampling"<<endl;
	cout<<"  -scale geometry      scale the image"<<endl;
	cout<<"  -segment values      segment an image"<<endl;
	cout<<"  -selective-blur geometry"<<endl;
	cout<<"                       selectively blur pixels within a contrast threshold"<<endl;
	cout<<"  -sepia-tone threshold"<<endl;
	cout<<"                       simulate a sepia-toned photo"<<endl;
	cout<<"  -set property value  set an image property"<<endl;
	cout<<"  -shade degrees       shade the image using a distant light source"<<endl;
	cout<<"  -shadow geometry     simulate an image shadow"<<endl;
	cout<<"  -sharpen geometry    sharpen the image"<<endl;
	cout<<"  -shave geometry      shave pixels from the image edges"<<endl;
	cout<<"  -shear geometry      slide one edge of the image along the X or Y axis"<<endl;
	cout<<"  -sigmoidal-contrast geometry"<<endl;
	cout<<"                       increase the contrast without saturating highlights or shadows"<<endl;
	cout<<"  -sketch geometry     simulate a pencil sketch"<<endl;
	cout<<"  -solarize threshold  negate all pixels above the threshold level"<<endl;
	cout<<"  -sparse-color method args"<<endl;
	cout<<"                       fill in a image based on a few color points"<<endl;
	cout<<"  -splice geometry     splice the background color into the image"<<endl;
	cout<<"  -spread radius       displace image pixels by a random amount"<<endl;
	cout<<"  -strip               strip image of all profiles and comments"<<endl;
	cout<<"  -swirl degrees       swirl image pixels about the center"<<endl;
	cout<<"  -threshold value     threshold the image"<<endl;
	cout<<"  -thumbnail geometry  create a thumbnail of the image"<<endl;
	cout<<"  -tile filename       tile image when filling a graphic primitive"<<endl;
	cout<<"  -tint value          tint the image with the fill color"<<endl;
	cout<<"  -transform           affine transform image"<<endl;
	cout<<"  -transparent color   make this color transparent within the image"<<endl;
	cout<<"  -transpose           flip image vertically and rotate 90 degrees"<<endl;
	cout<<"  -transverse          flop image horizontally and rotate 270 degrees"<<endl;
	cout<<"  -trim                trim image edges"<<endl;
	cout<<"  -type type           image type"<<endl;
	cout<<"  -unique-colors       discard all but one of any pixel color"<<endl;
	cout<<"  -unsharp geometry    sharpen the image"<<endl;
	cout<<"  -vignette geometry   soften the edges of the image in vignette style"<<endl;
	cout<<"  -wave geometry       alter an image along a sine wave"<<endl;
	cout<<"  -white-threshold value"<<endl;
	cout<<"                       force all pixels above the threshold into white"<<endl;
	cout<<""<<endl;
	cout<<"Image Sequence Operators:"<<endl;
	cout<<"  -append              append an image sequence"<<endl;
	cout<<"  -clut                apply a color lookup table to the image"<<endl;
	cout<<"  -coalesce            merge a sequence of images"<<endl;
	cout<<"  -combine             combine a sequence of images"<<endl;
	cout<<"  -composite           composite image"<<endl;
	cout<<"  -crop geometry       cut out a rectangular region of the image"<<endl;
	cout<<"  -deconstruct         break down an image sequence into constituent parts"<<endl;
	cout<<"  -evaluate-sequence operator"<<endl;
	cout<<"                       evaluate an arithmetic, relational, or logical expression"<<endl;
	cout<<"  -flatten             flatten a sequence of images"<<endl;
	cout<<"  -fx expression       apply mathematical expression to an image channel(s)"<<endl;
	cout<<"  -hald-clut           apply a Hald color lookup table to the image"<<endl;
	cout<<"  -morph value         morph an image sequence"<<endl;
	cout<<"  -mosaic              create a mosaic from an image sequence"<<endl;
	cout<<"  -process arguments   process the image with a custom image filter"<<endl;
	cout<<"  -reverse             reverse image sequence"<<endl;
	cout<<"  -separate            separate an image channel into a grayscale image"<<endl;
	cout<<"  -write filename      write images to this file"<<endl;
	cout<<""<<endl;
	cout<<"Image Stack Operators:"<<endl;
	cout<<"  -clone index         clone an image"<<endl;
	cout<<"  -delete index        delete the image from the image sequence"<<endl;
	cout<<"  -insert index        insert last image into the image sequence"<<endl;
	cout<<"  -swap indexes        swap two images in the image sequence"<<endl;
	cout<<""<<endl;
	cout<<"Miscellaneous Options:"<<endl;
	cout<<"  -debug events        display copious debugging information"<<endl;
	cout<<"  -help                print program options"<<endl;
	cout<<"  -list type           print a list of supported option arguments"<<endl;
	cout<<"  -log format          format of debugging information"<<endl;
	cout<<"  -version             print version information"<<endl;
	*/
}
