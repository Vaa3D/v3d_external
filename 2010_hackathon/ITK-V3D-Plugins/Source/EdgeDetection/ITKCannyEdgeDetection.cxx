/* ITKCannyEdgeDetection.cpp
 * 2010-06-02: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKCannyEdgeDetection.h"

// ITK Header Files
#include "itkImage.h"
#include "itkCannyEdgeDetectionImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"

/** \class CannyEdgeDetectionImageFilter
 *
 * This filter is an implementation of a Canny edge detector for scalar-valued
 * images.  Based on John Canny's paper "A Computational Approach to Edge 
 * Detection"(IEEE Transactions on Pattern Analysis and Machine Intelligence, 
 * Vol. PAMI-8, No.6, November 1986),  there are four major steps used in the 
 * edge-detection scheme:
 * (1) Smooth the input image with Gaussian filter.
 * (2) Calculate the second directional derivatives of the smoothed image. 
 * (3) Non-Maximum Suppression: the zero-crossings of 2nd derivative are found,
 *     and the sign of third derivative is used to find the correct extrema. 
 * (4) The hysteresis thresholding is applied to the gradient magnitude
 *      (multiplied with zero-crossings) of the smoothed image to find and 
 *      link edges.
 *
 * \par Inputs and Outputs
 * The input to this filter should be a scalar, real-valued Itk image of
 * arbitrary dimension.  The output should also be a scalar, real-value Itk
 * image of the same dimensionality.
 *
 * \par Parameters
 * There are four parameters for this filter that control the sub-filters used
 * by the algorithm.
 *
 * \par 
 * Variance and Maximum error are used in the Gaussian smoothing of the input
 * image.  See  itkDiscreteGaussianImageFilter for information on these
 * parameters.
 *
 * \par
 * Threshold is the lowest allowed value in the output image.  Its data type is 
 * the same as the data type of the output image. Any values below the
 * Threshold level will be replaced with the OutsideValue parameter value, whose
 * default is zero.
 * 
 * \todo Edge-linking will be added when an itk connected component labeling
 * algorithm is available.
 *
 * \sa DiscreteGaussianImageFilter
 * \sa ZeroCrossingImageFilter
 * \sa ThresholdImageFilter
 */

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKCannyEdgeDetection, ITKCannyEdgeDetectionPlugin)

void itkCannyEdgeDetectionPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK CannyEdgeDetection";
QStringList ITKCannyEdgeDetectionPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK CannyEdgeDetection")
						 << QObject::tr("about this plugin");
}

void ITKCannyEdgeDetectionPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK CannyEdgeDetection"))
    {
    	itkCannyEdgeDetectionPlugin(callback, parent);
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Canny Edge Detction 1.0 (2010-June-02): this plugin is developed by Yang Yu.");
	}
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKCannyEdgeDetectionImageFilterSpecializaed
{
public:
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandle curwin = callback.currentImageWindow();
		
		V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
		Image4DSimple *p4DImage = callback.getImage(curwin);
		
		typedef TInputPixelType  PixelType;
		
		PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );
		unsigned long int numberOfPixels = p4DImage->getTotalBytes();
		
		long pagesz = p4DImage->getTotalUnitNumberPerChannel();
		
		long nx = p4DImage->getXDim();
		long ny = p4DImage->getYDim();
		long nz = p4DImage->getZDim();
		long nc = p4DImage->getCDim();  // Number of channels
		
		int channelToFilter = globalSetting.iChannel_for_plugin;
		
		if( channelToFilter >= nc )
		{
			v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
			return;
		}
		
		long offsets = channelToFilter*pagesz;
		
		const unsigned int Dimension = 3;
		
		typedef itk::Image< TInputPixelType, Dimension > InputImageType;
		typedef itk::Image< TOutputPixelType, Dimension > OutputImageType;
		typedef itk::ImportImageFilter< TInputPixelType, Dimension > ImportFilterType;
		
		typename ImportFilterType::Pointer importFilter = ImportFilterType::New();
		
		typename ImportFilterType::SizeType size;
		size[0] = nx;
		size[1] = ny;
		size[2] = nz;
		
		typename ImportFilterType::IndexType start;
		start.Fill( 0 );
		
		typename ImportFilterType::RegionType region;
		region.SetIndex( start );
		region.SetSize(  size  );
		
		importFilter->SetRegion( region );
		
		region.SetSize( size );
		
		typename InputImageType::PointType origin;
		origin.Fill( 0.0 );
		
		importFilter->SetOrigin( origin );
		
		
		typename ImportFilterType::SpacingType spacing;
		spacing.Fill( 1.0 );
		
		importFilter->SetSpacing( spacing );
		
		const bool importImageFilterWillOwnTheBuffer = false;
		
		typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
		typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
		
		typedef itk::CannyEdgeDetectionImageFilter<OutputImageType, OutputImageType> CannyEdgeDetectionType;
		typename CannyEdgeDetectionType::Pointer cannyedgedetectionFilter = CannyEdgeDetectionType::New();
		
		//set Parameters
		// m_Variance m_MaximumError m_Threshold m_UpperThreshold m_LowerThreshold m_OutsideValue
		
		// DiscreteGaussianImageFilter: Variance and Maximum error
		// ZeroCrossingImageFilter
		// ThresholdImageFilter
		
		float var = 10.0;
		float maxerr = 0.5; //0.5  //Maximum Error Must be in the range [ 0.0 , 1.0 ]

		//input
		//update the pixel value
		ITKCannyEdgeDetectionDialog d(callback, parent);
		
		if (d.exec()!=QDialog::Accepted)
		{
			return;
		}
		else
		{
			
                  unsigned char lowerTh = d.sbLowerThreshold->value();
                  unsigned char upperTh = d.sbUpperThreshold->value();
                  printf("Thresholds: lower=%d, upper=%d\n", (int)lowerTh, (int)upperTh);
		

			//consider multiple channels
			if(channelToFilter==-1)
			{
				TOutputPixelType *output1d;
				try
				{
					output1d = new TOutputPixelType [numberOfPixels];
				}
				catch(...)
				{
					std::cerr << "Error memroy allocating." << std::endl;
					return;
				}
				
				const bool filterWillDeleteTheInputBuffer = false;
				
				for(long ch=0; ch<nc; ch++)
				{
					offsets = ch*pagesz;
					
					TOutputPixelType *p = output1d+offsets;
					
					importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
					
					castImageFilter->SetInput( importFilter->GetOutput() );
					try
					{
						castImageFilter->Update();
					}
					catch( itk::ExceptionObject & excp)
					{
						std::cerr << "Error run this filter." << std::endl;
						std::cerr << excp << std::endl;
						return;
					}
					
					cannyedgedetectionFilter->SetInput( castImageFilter->GetOutput() );
					
					cannyedgedetectionFilter->SetVariance(var);
					cannyedgedetectionFilter->SetMaximumError(maxerr);
					cannyedgedetectionFilter->SetLowerThreshold(lowerTh);
					cannyedgedetectionFilter->SetUpperThreshold(upperTh);
					
					cannyedgedetectionFilter->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
					try
					{
						cannyedgedetectionFilter->Update();
					}
					catch( itk::ExceptionObject & excp)
					{
						std::cerr << "Error run this filter." << std::endl;
						std::cerr << excp << std::endl;
						return;
					}
					
				}
				
				setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, nc, callback);
			}
			else if(channelToFilter<nc)
			{
				importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
				
				castImageFilter->SetInput( importFilter->GetOutput() );
				try
				{
					castImageFilter->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}
				
				cannyedgedetectionFilter->SetInput( castImageFilter->GetOutput() );
				
				cannyedgedetectionFilter->SetVariance(var);
				cannyedgedetectionFilter->SetMaximumError(maxerr);
				cannyedgedetectionFilter->SetLowerThreshold(lowerTh);
				cannyedgedetectionFilter->SetUpperThreshold(upperTh);
				
				try
				{
					cannyedgedetectionFilter->Update();
				}
				catch( itk::ExceptionObject & excp)
				{
					std::cerr << "Error run this filter." << std::endl;
					std::cerr << excp << std::endl;
					return;
				}
				
				// output
				typename OutputImageType::PixelContainer * container;
				
				container =cannyedgedetectionFilter->GetOutput()->GetPixelContainer();
				container->SetContainerManageMemory( false );
				
				typedef TOutputPixelType OutputPixelType;
				OutputPixelType * output1d = container->GetImportPointer();
				
				setPluginOutputAndDisplayUsingGlobalSetting(output1d, nx, ny, nz, 1, callback);
			}
			
			
			
		}
			

	} // excute
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKCannyEdgeDetectionImageFilterSpecializaed< input_pixel_type, output_pixel_type > runner; \
		runner.Execute( callback, parent ); \
		break; \
	} 

#define EXECUTE_ALL_PIXEL_TYPES \
	if (! p4DImage) return; \
	ImagePixelType pixelType = p4DImage->getDatatype(); \
	switch( pixelType )  \
	{  \
		EXECUTE( V3D_UINT8, unsigned char, float );  \
		EXECUTE( V3D_UINT16, unsigned short int, float );  \
		EXECUTE( V3D_FLOAT32, float, float );  \
		case V3D_UNKNOWN:  \
		{  \
		}  \
	}   

void itkCannyEdgeDetectionPlugin(V3DPluginCallback &callback, QWidget *parent)
{
	Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
	if (!p4DImage)
    {
		v3d_msg(QObject::tr("You don't have any image open in the main window."));
		return;
    }
	
	EXECUTE_ALL_PIXEL_TYPES; 
}


