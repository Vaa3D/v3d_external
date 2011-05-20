#ifndef _Registration_H_
#define _Registration_H_

#include "header.h"

class Registration:public QObject,public V3DPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface)
public:
	Registration(){}
	QStringList menulist()const;
	QStringList funclist()const;
	void domenu(const QString &menu_name,V3DPluginCallback &callback,QWidget* parent);
	void dofunc(const QString & func_name,const V3DPluginArgList &input,V3DPluginArgList & output,QWidget *parent);

};

template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterDualImage< TPixelType, TPixelType>
{
	typedef V3DITKFilterDualImage< TPixelType,TPixelType >	                        				Superclass;
	typedef typename Superclass::Input3DImageType			                				ImageType;
	typedef float                                    								InternalPixelType;
  	typedef itk::Image< InternalPixelType, 3 > 									InternalImageType;
	//----------------------Registration--------------------------
	typedef itk::ImageRegistrationMethod< InternalImageType,InternalImageType > 	         			RegistrationType;
	typedef itk::MultiResolutionImageRegistrationMethod< InternalImageType, InternalImageType >   			MultiRegistrationType;
	typedef typename RegistrationType::ParametersType 								ParametersType;
	typedef typename MultiRegistrationType::ParametersType								MultiParametersType;
	typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType >   			FixedImagePyramidType;
  	typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType >   			MovingImagePyramidType;
	typedef itk::ResampleImageFilter< InternalImageType, InternalImageType >    					ResampleFilterType;
	//----------------------Transform for 3D----------------------
	typedef itk::AffineTransform< double, 3 > 			       						AffineTransformType;
	typedef itk::TranslationTransform< double, 3 > 									TranTransformType;
	typedef itk::VersorRigid3DTransform< double > 									Versor3DTransformType;
	typedef double CoordinateRepType;
	typedef itk::BSplineDeformableTransform< CoordinateRepType, 3, 3 >      					BsplineTransformType;
	//----------------------Interpolator--------------------------
	typedef itk::LinearInterpolateImageFunction< InternalImageType, double > 					LinearInterpolatorType;
	typedef itk::NearestNeighborInterpolateImageFunction< InternalImageType, double > 				NearestInterpolatorType;
	typedef itk::BSplineInterpolateImageFunction< InternalImageType, double > 					BSplineInterpolatorType;
	//-----------------------Metric-------------------------------
	typedef itk::MeanSquaresImageToImageMetric< InternalImageType, InternalImageType > 	        		MeanSqMetricType;
	typedef itk::MutualInformationImageToImageMetric< InternalImageType, InternalImageType >        		MutualMetricType;
  	typedef itk::MattesMutualInformationImageToImageMetric< InternalImageType, InternalImageType >  		MattesMetricType;
	typedef itk::MatchCardinalityImageToImageMetric< InternalImageType, InternalImageType >    			MatchMetricType;
	typedef itk::GradientDifferenceImageToImageMetric< InternalImageType, InternalImageType >  			GradientMetricType;
	typedef itk::NormalizedMutualInformationHistogramImageToImageMetric< InternalImageType, InternalImageType >     NormalizeMetricType;
	//----------------------Optimizer-----------------------------
	typedef itk::RegularStepGradientDescentOptimizer								ReStepOptimizerType;
	typedef itk::GradientDescentOptimizer										GrDeOptimizerType;
	typedef itk::VersorRigid3DTransformOptimizer           								Versor3DOptimizerType;
	typedef itk::AmoebaOptimizer       										AmoeOptimizerType;
	typedef itk::OnePlusOneEvolutionaryOptimizer           								PlusOptimizerType;

public:
	PluginSpecialized(V3DPluginCallback* callback,QString transform,QString interpolator,QString metric,QString optimizer,QString reg):Superclass(callback)
	{
		registration = RegistrationType::New();
		multiRegistration = MultiRegistrationType::New();
		fixedImagePyramid = FixedImagePyramidType::New();
  		movingImagePyramid = MovingImagePyramidType::New();		
		resample = ResampleFilterType::New();		
		registration_str = reg;
		transform_str = transform;
		metric_str = metric;
		interpolator_str = interpolator;
		optimizer_str = optimizer;
	}

	PluginSpecialized(V3DPluginCallback* callback):Superclass(callback)
	{
		resample = ResampleFilterType::New();		
	}

	void SetRegTransform(QString transform_str)
	{
		if(transform_str == "AffineTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			affTransform = AffineTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     affTransform    );
			else
				registration->SetTransform(     affTransform    );

			typedef itk::CenteredTransformInitializer< AffineTransformType, InternalImageType, InternalImageType >  TransformInitializerType;
			typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
			initializer->SetTransform( affTransform );
			initializer->SetFixedImage( fixedOut );
			initializer->SetMovingImage( movedOut );
			initializer->MomentsOn();
			initializer->InitializeTransform();
			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( affTransform->GetParameters() );
			else
				registration->SetInitialTransformParameters( affTransform->GetParameters() );
  		
			std::cout << "End the Initialize" << std::endl;
			numberOfParameters = affTransform->GetNumberOfParameters();
		}
		if(transform_str == "VersorRigid3DTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			versor3DTransform = Versor3DTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     versor3DTransform    );
			else
				registration->SetTransform(     versor3DTransform    );

			typedef itk::CenteredTransformInitializer< Versor3DTransformType, InternalImageType, InternalImageType >  TransformInitializerType;
			typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
			initializer->SetTransform( versor3DTransform );
			initializer->SetFixedImage(  fixedOut );
			initializer->SetMovingImage( movedOut );
			initializer->MomentsOn();
			initializer->InitializeTransform();

			typedef Versor3DTransformType::VersorType  VersorType;
			typedef VersorType::VectorType     	   VectorType;
			VersorType     rotation;
			VectorType     axis;
			axis[0] = 0.0;
			axis[1] = 0.0;
			axis[2] = 1.0;
			const double angle = 0;
			rotation.Set(  axis, angle  );
			versor3DTransform->SetRotation( rotation );			
			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( versor3DTransform->GetParameters() );
			else
				registration->SetInitialTransformParameters( versor3DTransform->GetParameters() );
			
			std::cout << "End the Initialize" << std::endl;
			numberOfParameters = versor3DTransform->GetNumberOfParameters();
			
			optimizer_str = "VersorRigid3DTransformOptimizer";
				
		}
		if(transform_str == "TranslationTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			tranTransform = TranTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     tranTransform    );
			else
				registration->SetTransform(     tranTransform    );
		
  			ParametersType initialParameters( tranTransform->GetNumberOfParameters() );

  			initialParameters[0] = 0.2;  	// Initial offset in mm along X
  			initialParameters[1] = 0.2;   	// Initial offset in mm along Y
  			initialParameters[2] = 0.2;  	// Initial offset in mm along Z

  			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( initialParameters );
			else
				registration->SetInitialTransformParameters( initialParameters );
			std::cout << "End the Initialize" << std::endl;			
			numberOfParameters = tranTransform->GetNumberOfParameters();
		}
		if(transform_str == "BSplineDeformableTransform")
		{
			std::cout << "You select "<<transform_str.toStdString()<<" ! "<<std::endl;
			std::cout << "Now Initialize the Transform..." << std::endl;
			bsplineTransform = BsplineTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetTransform(     bsplineTransform    );
			else
				registration->SetTransform(     bsplineTransform    );

			typedef BsplineTransformType::RegionType RegionType;
			RegionType bsplineRegion;
			RegionType::SizeType   gridSizeOnImage;
			RegionType::SizeType   gridBorderSize;
			RegionType::SizeType   totalGridSize;

			gridSizeOnImage.Fill( 10 );
			gridBorderSize.Fill( 3 );   
			totalGridSize = gridSizeOnImage + gridBorderSize;
			bsplineRegion.SetSize( totalGridSize );

			typedef BsplineTransformType::SpacingType 		SpacingType;
			typedef BsplineTransformType::OriginType 		OriginType;
			typedef itk::ImportImageFilter<float, 3>            	ImportFilterType;
			typename InternalImageType::PointType 			origin;
        		typename ImportFilterType::SpacingType 			spacing;
			typename ImportFilterType::RegionType 			region;
			
			origin.Fill(0.0);
			spacing.Fill(1.0);
			
			typename ImportFilterType::IndexType start;
			start.Fill(0);
			typename ImportFilterType::SizeType size;
			size[0] = p4DImage->getXDim();
			size[1] = p4DImage->getYDim();
			size[2] = p4DImage->getZDim();
			region.SetIndex(start);
			region.SetSize(size);
			typename InternalImageType::SizeType fixedImageSize = region.GetSize();

			for(unsigned int r=0; r<3; r++)
			{
			spacing[r] *= static_cast<double>(fixedImageSize[r] - 1)  /
						  static_cast<double>(gridSizeOnImage[r] - 1);
			}

			typename InternalImageType::DirectionType gridDirection = fixedOut->GetDirection();
			SpacingType gridOriginOffset = gridDirection * spacing;

			OriginType gridOrigin = origin - gridOriginOffset;

			bsplineTransform->SetGridSpacing( spacing );
			bsplineTransform->SetGridOrigin( gridOrigin );
			bsplineTransform->SetGridRegion( bsplineRegion );
			bsplineTransform->SetGridDirection( gridDirection );

			typedef BsplineTransformType::ParametersType     ParametersType;
			ParametersType parameters( bsplineTransform->GetNumberOfParameters() );
			parameters.Fill( 0.0 );
			bsplineTransform->SetParameters( parameters );
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInitialTransformParameters( bsplineTransform->GetParameters() );
			else
				registration->SetInitialTransformParameters( bsplineTransform->GetParameters() );
			
			std::cout << "End the Initialize" << std::endl;			
			numberOfParameters = tranTransform->GetNumberOfParameters();
		}

	}
	void SetRegInterpolator(QString interpolator_str)
	{
		if(interpolator_str == "LinearInterpolateImageFunction")
		{
			std::cout << "You select "<<interpolator_str.toStdString()<<" ! "<<std::endl;
			linInterpolator = LinearInterpolatorType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInterpolator( linInterpolator );
			else
   				registration->SetInterpolator( linInterpolator );

			resample->SetInterpolator( linInterpolator );
		}
		if(interpolator_str == "NearestNeighborInterpolateImageFunction")
		{
			std::cout << "You select "<<interpolator_str.toStdString()<<" ! "<<std::endl;
			neaInterpolator = NearestInterpolatorType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInterpolator( neaInterpolator );
			else
   				registration->SetInterpolator( neaInterpolator );

			resample->SetInterpolator( neaInterpolator );	
		}
		if(interpolator_str == "BSplineInterpolateImageFunction")
		{
			std::cout << "You select "<<interpolator_str.toStdString()<<" ! "<<std::endl;
			BspInterpolator = BSplineInterpolatorType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetInterpolator( BspInterpolator );
			else
   				registration->SetInterpolator( BspInterpolator );

			resample->SetInterpolator( BspInterpolator );
		}
		
	}
	void SetRegMetric(QString metric_str)
	{
		if(metric_str == "MeanSquaresImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			meanSqmetric = MeanSqMetricType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( meanSqmetric );
			else
   				registration->SetMetric( meanSqmetric );
		}	
		if(metric_str == "MutualInformationImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			mutualMetric = MutualMetricType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( mutualMetric );
			else
   				registration->SetMetric( mutualMetric );

			mutualMetric->SetFixedImageStandardDeviation(  0.4 );
  			mutualMetric->SetMovingImageStandardDeviation( 0.4 );
			std::cout<<"numberOfSamples: "<<numberOfSamples<<std::endl;
			mutualMetric->SetNumberOfSpatialSamples( numberOfSamples );

			optimizer_str = "GradientDescentOptimizer";
		}  
		if(metric_str == "MattesMutualInformationImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			mattesMetric = MattesMetricType::New();
			unsigned int numberOfBins = 24;
  			unsigned int numberOfSamples = 10000;
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
				multiRegistration->SetMetric( mattesMetric );
				mattesMetric->SetNumberOfHistogramBins( 128 );
  				mattesMetric->SetNumberOfSpatialSamples( 50000 );
				mattesMetric->ReinitializeSeed( 76926294 );
			}
			else
   			{
				registration->SetMetric( mattesMetric );
				mattesMetric->SetNumberOfHistogramBins( numberOfBins );
  				mattesMetric->SetNumberOfSpatialSamples( numberOfSamples );
			}			
		} 
		if(metric_str == "MatchCardinalityImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			matchMetric = MatchMetricType::New();
			matchMetric->MeasureMatchesOff();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( matchMetric );
			else
   				registration->SetMetric( matchMetric );
			matchMetric->MeasureMatchesOff();
		}
		if(metric_str == "NormalizedMutualInformationHistogramImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			normalizeMetric = NormalizeMetricType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( normalizeMetric );
			else
   				registration->SetMetric( normalizeMetric );
			unsigned int numberOfHistogramBins = 32;//Can be set by user
			typename NormalizeMetricType::HistogramType::SizeType histogramSize;

			#ifdef ITK_USE_REVIEW_STATISTICS
			  histogramSize.SetSize(3);
			#endif

			  histogramSize[0] = numberOfHistogramBins;
			  histogramSize[1] = numberOfHistogramBins;
			  histogramSize[2] = numberOfHistogramBins;
			  normalizeMetric->SetHistogramSize( histogramSize );
			 
			  typedef typename NormalizeMetricType::ScalesType ScalesType;
			  ScalesType scales( numberOfParameters );
			  scales.Fill( 1.0 );			    
			  normalizeMetric->SetDerivativeStepLengthScales(scales);
			  
		}
		if(metric_str == "GradientDifferenceImageToImageMetric")
		{
			std::cout << "You select "<<metric_str.toStdString()<<" ! "<<std::endl;
			gradientMetric = GradientMetricType::New();			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetMetric( gradientMetric );
			else
   				registration->SetMetric( gradientMetric );
			gradientMetric->SetDerivativeDelta( 0.5 );
		}
						
	}
	void SetRegOptimizer(QString optimizer_str)
	{
		if(optimizer_str == "RegularStepGradientDescentOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			reStepOptimizer = ReStepOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( reStepOptimizer );
			else
   				registration->SetOptimizer( reStepOptimizer );
			
			if(transform_str == "AffineTransform")
			{
				typedef ReStepOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
				optimizerScales[0] =  1.0;
				optimizerScales[1] =  1.0;
				optimizerScales[2] =  1.0;
				optimizerScales[3] =  1.0;
				optimizerScales[4] =  1.0;
				optimizerScales[5] =  1.0;
				optimizerScales[6] =  1.0;
				optimizerScales[7] =  1.0;
				optimizerScales[8] =  1.0;
				optimizerScales[9]  =  translationScale;
				optimizerScales[10] =  translationScale;
				optimizerScales[11] =  translationScale;
		
				reStepOptimizer->SetScales( optimizerScales );
				reStepOptimizer->SetMaximumStepLength( 4.0 );
				reStepOptimizer->SetMinimumStepLength( 0.001 );
				reStepOptimizer->SetNumberOfIterations( 200 );
				reStepOptimizer->SetRelaxationFactor( 0.8 );
			}
			else
			{
				reStepOptimizer->SetMaximumStepLength( 2.0 );
				reStepOptimizer->SetMinimumStepLength( 0.001 );
				reStepOptimizer->SetNumberOfIterations( 300 );
				reStepOptimizer->SetRelaxationFactor( 0.8 );
				
			}
			reStepOptimizer->MinimizeOn();

			ReCommandIterationUpdate::Pointer observer = ReCommandIterationUpdate::New();
			reStepOptimizer->AddObserver( itk::IterationEvent(), observer );

			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
				typedef RegistrationInterfaceCommand<MultiRegistrationType> CommandType;
				CommandType::Pointer command = CommandType::New();
				registration->AddObserver( itk::IterationEvent(), command );
			}
		}
		if(optimizer_str == "GradientDescentOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			grDeOptimizer = GrDeOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( grDeOptimizer );
			else
   				registration->SetOptimizer( grDeOptimizer );
			if(transform_str == "AffineTransform")
			{
				typedef GrDeOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
				optimizerScales[0] =  1.0;
				optimizerScales[1] =  1.0;
				optimizerScales[2] =  1.0;
				optimizerScales[3] =  1.0;
				optimizerScales[4] =  1.0;
				optimizerScales[5] =  1.0;
				optimizerScales[6] =  1.0;
				optimizerScales[7] =  1.0;
				optimizerScales[8] =  1.0;
				optimizerScales[9]  =  translationScale;
				optimizerScales[10] =  translationScale;
				optimizerScales[11] =  translationScale;
		
				grDeOptimizer->SetScales( optimizerScales );

			}
			grDeOptimizer->SetLearningRate( 15.0 );
 			grDeOptimizer->SetNumberOfIterations( 200 );
  			grDeOptimizer->MaximizeOn();
			
			GrCommandIterationUpdate::Pointer observer = GrCommandIterationUpdate::New();
			grDeOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
		if(optimizer_str == "VersorRigid3DTransformOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			versor3DOptimizer = Versor3DOptimizerType::New();
			std::cout<<"Test dialog14!"<<std::endl;
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( versor3DOptimizer );
			else
   				registration->SetOptimizer( versor3DOptimizer );
			std::cout<<"Test dialog15!"<<std::endl;
			typedef Versor3DOptimizerType::ScalesType       OptimizerScalesType;
			OptimizerScalesType optimizerScales( numberOfParameters );
			double translationScale = 1.0 / 1000.0;
			double maximumStepLength = 0.2;
			double minimumStepLength = 0.0001;
			int numberOfIterations = 200;

			/*V3DITKGenericDialog dialog("Set Up Parameters");
			dialog.AddDialogElement("TranslationScale", (double)0.0010, (double)0.0001, (double)0.0100);
			dialog.AddDialogElement("MaximumStepLength", (double)0.2, (double)0.1, (double)1.0);
			dialog.AddDialogElement("MinimumStepLength", (double)0.0010, (double)0.0001, (double)0.0100); 
			dialog.AddDialogElement("NumberOfIterations", 300.0, 100.0, 500.0);

			if( dialog.exec() == QDialog::Accepted )
			{
				translationScale = dialog.GetValue("TranslationScale");
				maximumStepLength = dialog.GetValue("MaximumStepLength");
				minimumStepLength = dialog.GetValue("MinimumStepLength");
				numberOfIterations = dialog.GetValue("NumberOfIterations");
			}*/
			versor3DOptimizer->SetMaximumStepLength( maximumStepLength );
			versor3DOptimizer->SetMinimumStepLength( minimumStepLength );
			versor3DOptimizer->SetNumberOfIterations( numberOfIterations );

    			optimizerScales[0] = 1.0;
			optimizerScales[1] = 1.0;
			optimizerScales[2] = 1.0;
			optimizerScales[3] = translationScale;
			optimizerScales[4] = translationScale;
			optimizerScales[5] = translationScale;
			versor3DOptimizer->SetScales( optimizerScales );
			
			//Create the Command observer and register it with the optimizer.
			VeCommandIterationUpdate::Pointer observer = VeCommandIterationUpdate::New();
			versor3DOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
		if(optimizer_str == "AmoebaOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			amoeOptimizer = AmoeOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( amoeOptimizer );
			else
   				registration->SetOptimizer( amoeOptimizer );
			if(transform_str == "AffineTransform")
			{
				typedef AmoeOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
				optimizerScales[0] =  1.0;
				optimizerScales[1] =  1.0;
				optimizerScales[2] =  1.0;
				optimizerScales[3] =  1.0;
				optimizerScales[4] =  1.0;
				optimizerScales[5] =  1.0;
				optimizerScales[6] =  1.0;
				optimizerScales[7] =  1.0;
				optimizerScales[8] =  1.0;
				optimizerScales[9]  =  translationScale;
				optimizerScales[10] =  translationScale;
				optimizerScales[11] =  translationScale;
		
				amoeOptimizer->SetScales( optimizerScales );

			}
			AmoeOptimizerType::ParametersType simplexDelta( numberOfParameters );
			simplexDelta.Fill( 5.0 );

			amoeOptimizer->AutomaticInitialSimplexOff();
			amoeOptimizer->SetInitialSimplexDelta( simplexDelta );			 
			amoeOptimizer->SetParametersConvergenceTolerance( 0.25 ); // quarter pixel
			amoeOptimizer->SetFunctionConvergenceTolerance(0.001); // 0.1%
			amoeOptimizer->SetMaximumNumberOfIterations( 200 );
		
			AmoeCommandIterationUpdate::Pointer observer = AmoeCommandIterationUpdate::New();
			amoeOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
		if(optimizer_str == "OnePlusOneEvolutionaryOptimizer")
		{
			std::cout << "You select "<<optimizer_str.toStdString()<<" ! "<<std::endl;
			plusOptimizer = PlusOptimizerType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
				multiRegistration->SetOptimizer( plusOptimizer );
			else
   				registration->SetOptimizer( plusOptimizer );
			if(transform_str == "AffineTransform")
			{
				typedef PlusOptimizerType::ScalesType OptimizerScalesType;
				OptimizerScalesType optimizerScales( numberOfParameters );
				double translationScale = 1.0 / 1000.0;
				optimizerScales[0] =  1.0;
				optimizerScales[1] =  1.0;
				optimizerScales[2] =  1.0;
				optimizerScales[3] =  1.0;
				optimizerScales[4] =  1.0;
				optimizerScales[5] =  1.0;
				optimizerScales[6] =  1.0;
				optimizerScales[7] =  1.0;
				optimizerScales[8] =  1.0;
				optimizerScales[9]  =  translationScale;
				optimizerScales[10] =  translationScale;
				optimizerScales[11] =  translationScale;
		
				plusOptimizer->SetScales( optimizerScales );

			}
			typedef itk::Statistics::NormalVariateGenerator  GeneratorType;
			GeneratorType::Pointer generator = GeneratorType::New();
			generator->Initialize(12345);
			plusOptimizer->MaximizeOff();

			plusOptimizer->SetNormalVariateGenerator( generator );
			plusOptimizer->Initialize( 10 );
			plusOptimizer->SetEpsilon( 1.0 );
			plusOptimizer->SetMaximumIteration( 4000 );

			PlusCommandIterationUpdate::Pointer observer = PlusCommandIterationUpdate::New();
			plusOptimizer->AddObserver( itk::IterationEvent(), observer );
		}
	}

	void GetFinalParameters(QString transform_str,QString optimizer_str)
	{
		std::cout << "Get Parameters:" << std::endl;
		unsigned int numberOfIterations;
		double bestValue;		
		if(transform_str == "AffineTransform")
		{
			double finalTranslationX ;
			double finalTranslationY ;
			double finalTranslationZ ;
			AffineTransformType::Pointer finalTransform = AffineTransformType::New();			
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				
				finalTranslationX = finalParameters[4];
				finalTranslationY = finalParameters[5];
				finalTranslationZ = finalParameters[6];	
				finalTransform->SetParameters( finalParameters );
			}
			else
			{
		
				ParametersType finalParameters = registration->GetLastTransformParameters();
				
				finalTranslationX = finalParameters[4];
				finalTranslationY = finalParameters[5];
				finalTranslationZ = finalParameters[6];	
				finalTransform->SetParameters( finalParameters );
			}
			if(optimizer_str == "RegularStepGradientDescentOptimizer")
			{
				numberOfIterations = reStepOptimizer->GetCurrentIteration();
				bestValue = reStepOptimizer->GetValue();
			}
			
			if(optimizer_str == "GradientDescentOptimizer")
			{
				numberOfIterations = grDeOptimizer->GetCurrentIteration();
				bestValue = grDeOptimizer->GetValue();
			}

			const double finalRotationCenterX = affTransform->GetCenter()[0];
			const double finalRotationCenterY = affTransform->GetCenter()[1];
			const double finalRotationCenterZ = affTransform->GetCenter()[2];
			// Print out results
			std::cout << "Result = " << std::endl;
			std::cout << " Center X      = " << finalRotationCenterX  << std::endl;
			std::cout << " Center Y      = " << finalRotationCenterY  << std::endl;
			std::cout << " Center Z      = " << finalRotationCenterZ  << std::endl;
			std::cout << " Translation X = " << finalTranslationX  << std::endl;
			std::cout << " Translation Y = " << finalTranslationY  << std::endl;
			std::cout << " Translation Z = " << finalTranslationZ  << std::endl;	
			std::cout << " Iterations    = " << numberOfIterations << std::endl;
			std::cout << " Metric value  = " << bestValue          << std::endl;

			
  			
  			finalTransform->SetFixedParameters( affTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "AffineTransform";
			resample->SetTransform( affTransform );
		}
		if(transform_str == "TranslationTransform")
		{
			double TranslationAlongX;
  			double TranslationAlongY;
			double TranslationAlongZ;
			TranTransformType::Pointer finalTransform = TranTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				
				TranslationAlongX = finalParameters[0];
				TranslationAlongY = finalParameters[1];
				TranslationAlongZ = finalParameters[2];	
				finalTransform->SetParameters( finalParameters );
			}
			else
			{
		
				ParametersType finalParameters = registration->GetLastTransformParameters();
				
				TranslationAlongX = finalParameters[0];
				TranslationAlongY = finalParameters[1];
				TranslationAlongZ = finalParameters[2];	
				finalTransform->SetParameters( finalParameters );
			}
			if(optimizer_str == "RegularStepGradientDescentOptimizer")
			{
				numberOfIterations = reStepOptimizer->GetCurrentIteration();
				bestValue = reStepOptimizer->GetValue();
			}

			if(optimizer_str == "GradientDescentOptimizer")
			{
				numberOfIterations = grDeOptimizer->GetCurrentIteration();
				bestValue = grDeOptimizer->GetValue();
			}

			std::cout << "Result = " << std::endl;
			std::cout << " Translation X = " << TranslationAlongX  << std::endl;
			std::cout << " Translation Y = " << TranslationAlongY  << std::endl;
			std::cout << " Translation Z = " << TranslationAlongZ  << std::endl;
			std::cout << " Iterations    = " << numberOfIterations << std::endl;
			std::cout << " Metric value  = " << bestValue          << std::endl;

			
  			
  			finalTransform->SetFixedParameters( tranTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "TranslationTransform";
			resample->SetTransform( finalTransform );

		}
		if(transform_str == "VersorRigid3DTransform")
		{
			double versorX ;
			double versorY ;
			double versorZ ;
			double finalTranslationX ;
			double finalTranslationY ;
			double finalTranslationZ ;
			Versor3DTransformType::Pointer finalTransform = Versor3DTransformType::New();
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				versorX 	   = finalParameters[0];
				versorY            = finalParameters[1];
				versorZ            = finalParameters[2];
				finalTranslationX  = finalParameters[3];
				finalTranslationY  = finalParameters[4];
				finalTranslationZ  = finalParameters[5];
				finalTransform->SetParameters( finalParameters );
			}
			else
			{		
				ParametersType finalParameters = registration->GetLastTransformParameters();
				
				versorX 	   = finalParameters[0];
				versorY            = finalParameters[1];
				versorZ            = finalParameters[2];
				finalTranslationX  = finalParameters[3];
				finalTranslationY  = finalParameters[4];
				finalTranslationZ  = finalParameters[5];
				finalTransform->SetParameters( finalParameters );
			}
			numberOfIterations = versor3DOptimizer->GetCurrentIteration();
			bestValue = versor3DOptimizer->GetValue();
			// Print out results
			//
			std::cout << std::endl << std::endl;
			std::cout << "Result = " << std::endl;
			std::cout << " versor X      = " << versorX  << std::endl;
			std::cout << " versor Y      = " << versorY  << std::endl;
			std::cout << " versor Z      = " << versorZ  << std::endl;
			std::cout << " Translation X = " << finalTranslationX  << std::endl;
			std::cout << " Translation Y = " << finalTranslationY  << std::endl;
			std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
			std::cout << " Iterations    = " << numberOfIterations << std::endl;
			std::cout << " Metric value  = " << bestValue          << std::endl;			

			Versor3DTransformType::MatrixType matrix = versor3DTransform->GetRotationMatrix();
			Versor3DTransformType::OffsetType offset = versor3DTransform->GetOffset();
			std::cout << "Matrix = " << std::endl << matrix << std::endl;
			std::cout << "Offset = " << std::endl << offset << std::endl;
			
  			
  			finalTransform->SetFixedParameters( versor3DTransform->GetFixedParameters() );
			tran_arg.p = (void*)finalTransform;
			tran_arg.type = "VersorRigid3DTransform";
			resample->SetTransform( finalTransform );

		}	
		if(transform_str == "BSplineDeformableTransform")
		{
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{		
				MultiParametersType finalParameters = multiRegistration->GetLastTransformParameters();
				std::cout << "Last Transform Parameters" << std::endl;
				std::cout << finalParameters << std::endl;

				BsplineTransformType::Pointer finalTransform = BsplineTransformType::New();
				finalTransform->SetParameters( finalParameters );
				finalTransform->SetFixedParameters( bsplineTransform->GetFixedParameters() );
				resample->SetTransform( finalTransform );
			
			}
			else
			{

				ParametersType finalParameters = registration->GetLastTransformParameters();
				std::cout << "Last Transform Parameters" << std::endl;
				std::cout << finalParameters << std::endl;

				BsplineTransformType::Pointer finalTransform = BsplineTransformType::New();
				finalTransform->SetParameters( finalParameters );
				finalTransform->SetFixedParameters( bsplineTransform->GetFixedParameters() );
				tran_arg.p = (void*)finalTransform;
				tran_arg.type = "BSplineDeformableTransform";
				resample->SetTransform( finalTransform );
			
			}
		}			
	}
	void Execute(QWidget *parent)
	{       
		this->SetImageSelectionDialogTitle("ITK-V3D Registration");
    		this->AddImageSelectionLabel("FixedImage: ");
    		this->AddImageSelectionLabel("MovedImage: ");
    		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);       
		this->Compute();
	}	
	void Compute()
	{
		this->Initialize();

		const V3DLONG x1 = 0;
		const V3DLONG y1 = 0;
		const V3DLONG z1 = 0;

		const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
		const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
		const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

		//get image pointers
		v3dhandleList wndlist = this->m_V3DPluginCallback->getImageWindowList();
		p4DImage=this->m_V3DPluginCallback->getImage(wndlist[0]);

		if(wndlist.size()<2)
		{
			v3d_msg(QObject::tr("Registration needs at least two images!"));
			return;
		}

		if( this->m_ImageSelectionDialog.exec() != QDialog::Accepted )
		{
			return;
		}   

		Image4DSimple* p4DImage_1 = this->GetInputImageFromIndex( 0 );
		Image4DSimple* p4DImage_2 = this->GetInputImageFromIndex( 1 );

		QList< V3D_Image3DBasic > inputImageList1 =
		getChannelDataForProcessingFromGlobalSetting( p4DImage_1, *(this->m_V3DPluginCallback) );
		QList< V3D_Image3DBasic > inputImageList2 =
		getChannelDataForProcessingFromGlobalSetting( p4DImage_2, *(this->m_V3DPluginCallback) );

		//Add the progress dialog 
		V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );
		this->AddObserver( progressDialog.GetCommand() );
		progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
		progressDialog.show();

		const unsigned int numberOfChannelsToProcess = inputImageList1.size();
		if (numberOfChannelsToProcess<=0)
			return;
		
		for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
		{
			const V3D_Image3DBasic inputImage1 = inputImageList1.at(channel);
			const V3D_Image3DBasic inputImage2 = inputImageList2.at(channel);

			this->TransferInputDualImages( inputImage1, inputImage2, x1, x2, y1, y2, z1, z2 );

			this->RegistrationOneRegion();

			this->AddOutputImageChannel( channel );
		}
	
		this->ComposeOutputImage();
		
	}

	void RegistrationOneRegion()
	{		
		std::cout << "Set Parameters!" << std::endl;
		V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );
		this->AddObserver( progressDialog.GetCommand() );
		progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
		progressDialog.show();
		//this->RegisterInternalFilter( registration, 1.0 );

		V3DPluginArgItem arg;
		V3DPluginArgList input;
		V3DPluginArgList output;
		
		arg.p=(void*)this->GetInput3DImage1();
		input<<arg;
		output<<arg;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastIn/CastIn.so";
		function_name="CastIn";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		fixedOut=(InternalImageType*)(output.at(0).p);

		arg.p=(void*)this->GetInput3DImage2();
		input.replace(0,arg);
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		movedOut=(InternalImageType*)(output.at(0).p);
		
		if(metric_str == "MutualInformationImageToImageMetric")
		{
			//Normalize
			arg.p =(void*)this->GetInput3DImage1();			
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/Normalize/Normalize.so";
		        function_name="Normalize";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* fixedTempOut=(InternalImageType*)(output.at(0).p);
			arg.p=(void*)fixedTempOut;
			input.replace(0,arg);
			//DisGauSmooth
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/DisGauSmooth/DisGauSmooth.so";
		        function_name="DisGauSmooth";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* fixedSmoothOut=(InternalImageType*)(output.at(0).p);
			
			arg.p=(void*)this->GetInput3DImage2();
			input.replace(0,arg);
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/Normalize/Normalize.so";
		        function_name="Normalize";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* movedTempOut=(InternalImageType*)(output.at(0).p);
			arg.p=(void*)movedTempOut;
			input.replace(0,arg);
			//DisGauSmooth
			plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/DisGauSmooth/DisGauSmooth.so";
		        function_name="DisGauSmooth";
			this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
			InternalImageType* movedSmoothOut=(InternalImageType*)(output.at(0).p);

			const unsigned int numberOfPixels = fixedOut->GetBufferedRegion().GetNumberOfPixels();  
    			numberOfSamples = static_cast< unsigned int >( numberOfPixels * 0.001 );  

			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
				multiRegistration->SetFixedImage(    fixedSmoothOut   );
				multiRegistration->SetMovingImage(   movedSmoothOut  );		
			}
			else
			{
				registration->SetFixedImage(    fixedSmoothOut   );
				registration->SetMovingImage(   movedSmoothOut  );
			}	
						
		}		

		//Cast unsigned char to float by calling CastIn
		else
		{
			if(registration_str == "MultiResolutionImageRegistrationMethod")
			{
				multiRegistration->SetFixedImage(    fixedOut   );
				multiRegistration->SetMovingImage(   movedOut  );		
			}
			else
			{
				registration->SetFixedImage(    fixedOut   );
				registration->SetMovingImage(   movedOut  );
			}	
		}

		SetRegTransform(transform_str);		
		SetRegInterpolator(interpolator_str);		
		SetRegMetric(metric_str);		
		SetRegOptimizer(optimizer_str);
		
		if(registration_str == "MultiResolutionImageRegistrationMethod")
		{
			multiRegistration->SetFixedImageRegion(fixedOut->GetBufferedRegion() );

			std::cout << "Parameters Set Finished" << std::endl;
			//参数在构造函数中设置完毕，开始单分辨率配准
			std::cout << "Start Registration..." << std::endl;
			try 
			{ 
			multiRegistration->StartRegistration(); 
			std::cout << "Optimizer stop condition: "
			<< multiRegistration->GetOptimizer()->GetStopConditionDescription()
			<< std::endl;
			} 
			catch( itk::ExceptionObject & err ) 
			{ 
			std::cerr << "ExceptionObject caught !" << std::endl; 
			std::cerr << err << std::endl; 
			} 
			std::cout << "\nMultiRegistration Finished! " << std::endl;
		}
		else
		{
			registration->SetFixedImageRegion(fixedOut->GetBufferedRegion() );
			std::cout << "Parameters Set Finished" << std::endl;
			//参数在构造函数中设置完毕，开始单分辨率配准
			std::cout << "Start Registration..." << std::endl;
			try 
			{ 
			registration->StartRegistration(); 
			std::cout << "Optimizer stop condition: "
			<< registration->GetOptimizer()->GetStopConditionDescription()
			<< std::endl;
			} 
			catch( itk::ExceptionObject & err ) 
			{ 
			std::cerr << "ExceptionObject caught !" << std::endl; 
			std::cerr << err << std::endl; 
			} 
			std::cout << "\nRegistration Finished! " << std::endl;
		}
		GetFinalParameters(transform_str,optimizer_str);
                //开始重采样
		std::cout << "Start Resample..." << std::endl;
		resample->SetInput( movedOut );
		resample->SetSize( fixedOut->GetLargestPossibleRegion().GetSize() );
		resample->SetOutputOrigin(  fixedOut->GetOrigin() );
		resample->SetOutputSpacing( fixedOut->GetSpacing() );
		resample->SetOutputDirection( fixedOut->GetDirection() );
		resample->SetDefaultPixelValue( 0 );
		resample->Update();
		std::cout << "\nResample Finished!" << std::endl;

		//call resample
		/*V3DPluginArgItem arg1;
		V3DPluginArgItem arg2;
		V3DPluginArgList resInput;
		arg1.p=(void*)fixedOut;
		arg2.p=(void*)movedOut;
		arg2.type=interpolator_str;
		resInput<<tran_arg<<arg1<<arg2;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/Resample/Resample.so";
                function_name="Resample";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,resInput,output);
		InternalImageType* resOut=(InternalImageType*)(output.at(0).p);*/

		//cast float to unsigned char 
		arg.p = (void*)resample->GetOutput();
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastOut/CastOut.so";
                function_name="CastOut";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		ImageType* unOut=(ImageType*)(output.at(0).p);
		this->SetOutputImage(unOut);		
	}	
	
	void ImageSubtract()
	{
		this->SetImageSelectionDialogTitle("ITK-V3D Subtract");
    		this->AddImageSelectionLabel("FixedImage: ");
    		this->AddImageSelectionLabel("MovedImage: ");
    		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);

		this->Initialize();
		
		const V3DLONG x1 = 0;
		const V3DLONG y1 = 0;
		const V3DLONG z1 = 0;

		const V3DLONG x2 = this->m_NumberOfPixelsAlongX;
		const V3DLONG y2 = this->m_NumberOfPixelsAlongY;
		const V3DLONG z2 = this->m_NumberOfPixelsAlongZ;

		//get image pointers
		v3dhandleList wndlist = this->m_V3DPluginCallback->getImageWindowList();
		p4DImage=this->m_V3DPluginCallback->getImage(wndlist[0]);

		if(wndlist.size()<2)
		{
			v3d_msg(QObject::tr("Subtract needs at least two images!"));
			return;
		}

		if( this->m_ImageSelectionDialog.exec() != QDialog::Accepted )
		{
			return;
		}   

		Image4DSimple* p4DImage_1 = this->GetInputImageFromIndex( 0 );
		Image4DSimple* p4DImage_2 = this->GetInputImageFromIndex( 1 );

		QList< V3D_Image3DBasic > inputImageList1 =
		getChannelDataForProcessingFromGlobalSetting( p4DImage_1, *(this->m_V3DPluginCallback) );
		QList< V3D_Image3DBasic > inputImageList2 =
		getChannelDataForProcessingFromGlobalSetting( p4DImage_2, *(this->m_V3DPluginCallback) );

		//Add the progress dialog 
		V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );
		this->AddObserver( progressDialog.GetCommand() );
		progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
		progressDialog.show();

		const unsigned int numberOfChannelsToProcess = inputImageList1.size();
		if (numberOfChannelsToProcess<=0)
			return;
		
		for( unsigned int channel = 0; channel < numberOfChannelsToProcess; channel++ )
		{
			const V3D_Image3DBasic inputImage1 = inputImageList1.at(channel);
			const V3D_Image3DBasic inputImage2 = inputImageList2.at(channel);

			this->TransferInputDualImages( inputImage1, inputImage2, x1, x2, y1, y2, z1, z2 );

			this->SubtractOneRegion();

			this->AddOutputImageChannel( channel );
		}
	
		this->ComposeOutputImage();	
		
	}
	void SubtractOneRegion()
	{
		//Cast unsigned char to float
		V3DPluginArgItem arg;
		V3DPluginArgList input;
		V3DPluginArgList output;
		
		arg.p=(void*)this->GetInput3DImage1();
		input<<arg;
		output<<arg;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastIn/CastIn.so";
		function_name="CastIn";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* fixedImage=(InternalImageType*)(output.at(0).p);
		
		
		std::cout << "Test G!" << std::endl;
		arg.p=(void*)this->GetInput3DImage2();
		input.replace(0,arg);
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* movedImage=(InternalImageType*)(output.at(0).p);

		resample->SetInput( movedImage );
		resample->SetSize( fixedImage->GetLargestPossibleRegion().GetSize() );
		resample->SetOutputOrigin(  fixedImage->GetOrigin() );
		resample->SetOutputSpacing( fixedImage->GetSpacing() );
		resample->SetOutputDirection( fixedImage->GetDirection() );
		resample->SetDefaultPixelValue( 1 );
		if(transform_str == "TranslationTransform")
		{
			TranTransformType::Pointer identityTransform = TranTransformType::New();
			identityTransform->SetIdentity();
			resample->SetTransform( identityTransform );
		}
		if(transform_str == "AffineTransform")
		{
			AffineTransformType::Pointer identityTransform = AffineTransformType::New();
			identityTransform->SetIdentity();
			resample->SetTransform( identityTransform );
		}

		if(transform_str == "VersorRigid3DTransform")
		{
			Versor3DTransformType::Pointer identityTransform = Versor3DTransformType::New();
			identityTransform->SetIdentity();
			resample->SetTransform( identityTransform );
		}

		if(transform_str == "BSplineDeformableTransform")
		{
			BsplineTransformType::Pointer identityTransform =BsplineTransformType::New();
			identityTransform->SetIdentity();
			resample->SetTransform( identityTransform );
		}
		//调用subtract插件
		arg.p = (void*)fixedImage;
		input.replace(0,arg);
		V3DPluginArgItem temp;
		temp.p = (void*)resample->GetOutput();
		input<<temp;
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/ImageSubtract/ImageSubtract.so";
		function_name="ImageSubtract";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* diffOut=(InternalImageType*)(output.at(0).p);
		
		//call Rescale
		arg.p = (void*)diffOut;
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/RescaleIntensity/RescaleIntensity.so";
		function_name="RescaleIntensity";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		InternalImageType* resOut=(InternalImageType*)(output.at(0).p);
		
		//cast float to unsigned char 
		arg.p = (void*)resOut;
		input.replace(0,arg);
		plugin_name="/home/liyun/V3D/v3d/plugins/ITK/RegistrationPlugin/CastOut/CastOut.so"; 
                function_name="CastOut";
		this->m_V3DPluginCallback->callPluginFunc(plugin_name,function_name,input,output);
		ImageType* unOut=(ImageType*)(output.at(0).p);
		
		this->SetOutputImage(unOut);		
	}
	virtual void ComputeOneRegion()
	{}

private:
	QString plugin_name;
        QString function_name;
	QString registration_str;
	QString transform_str;
	QString metric_str;
	QString interpolator_str;
	QString optimizer_str;

	typename AffineTransformType::Pointer  		affTransform;
	typename TranTransformType::Pointer  		tranTransform;
	typename Versor3DTransformType::Pointer  	versor3DTransform;
	typename BsplineTransformType::Pointer  	bsplineTransform;
	typename LinearInterpolatorType::Pointer      	linInterpolator;
	typename NearestInterpolatorType::Pointer      	neaInterpolator;
	typename BSplineInterpolatorType::Pointer      	BspInterpolator;
	typename MeanSqMetricType::Pointer  		meanSqmetric;
	typename MutualMetricType::Pointer  		mutualMetric;
	typename MattesMetricType::Pointer  		mattesMetric;
	typename MatchMetricType::Pointer  		matchMetric;
	typename GradientMetricType::Pointer            gradientMetric;
	typename NormalizeMetricType::Pointer  		normalizeMetric;
	typename ReStepOptimizerType::Pointer      	reStepOptimizer;
	typename GrDeOptimizerType::Pointer      	grDeOptimizer;
	typename Versor3DOptimizerType::Pointer      	versor3DOptimizer;
	typename AmoeOptimizerType::Pointer      	amoeOptimizer;
	typename PlusOptimizerType::Pointer      	plusOptimizer;
	
	unsigned int numberOfSamples;
	unsigned int numberOfParameters;
	typename RegistrationType::Pointer              registration;
	typename MultiRegistrationType::Pointer   	multiRegistration;
	typename FixedImagePyramidType::Pointer 	fixedImagePyramid;
	typename MovingImagePyramidType::Pointer 	movingImagePyramid;

	typename ResampleFilterType::Pointer 		resample;
	V3DPluginArgItem 				tran_arg;

	InternalImageType* 				fixedOut;
	InternalImageType* 				movedOut;
	Image4DSimple* 					p4DImage;

};
#endif
