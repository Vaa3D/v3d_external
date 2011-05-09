#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "NOT.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkNotImageFilter.h"
#include "itkImageFileWriter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(NOT, NOTPlugin)


QStringList NOTPlugin::menulist() const
{
    return QStringList() << QObject::tr("ITK NOT")
            << QObject::tr("about this plugin");
}

QStringList NOTPlugin::funclist() const
{
    return QStringList()<<QObject::tr("invImage")
		<<QObject::tr("about this plugin");
}


template <typename TPixelType>
class PluginSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType >
{
  typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;

  typedef itk::NotImageFilter< ImageType, ImageType > FilterType;

public:

  PluginSpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
    this->RegisterInternalFilter( this->m_Filter, 1.0 );
    }

  virtual ~PluginSpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
    this->Compute();
    }

  virtual void ComputeOneRegion()
    {
	void * p=NULL;
	p=(void*)this->GetInput3DImage();
    this->m_Filter->SetInput((const itk::Image<TPixelType,3>*) p );

    if( !this->ShouldGenerateNewWindow() )
      {
      this->m_Filter->InPlaceOn();
      }

    this->m_Filter->Update();
	p=(void*)m_Filter->GetOutput();
	itk::Image<TPixelType,3>* out=(itk::Image<TPixelType,3>*)p;

   // this->SetOutputImage( this->m_Filter->GetOutput() );
	this->SetOutputImage(out);
    }
void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {
       	V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

       	this->AddObserver( progressDialog.GetCommand() );
       	progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
       	progressDialog.show();
	this->RegisterInternalFilter( this->m_Filter, 1.0 );

	void * p=NULL;
	p=(void*)input.at(0).p;
	if(!p)perror("errro");
	
	this->m_Filter->SetInput((const itk::Image<TPixelType,3>*) p );

	this->m_Filter->Update();
	p=m_Filter->GetOutput();
	typename ImageType::SizeType size=((const itk::Image<TPixelType,3>*)p)->GetBufferedRegion().GetSize();	
	fprintf(stdout,"size %u\t %u \t %u\n",size[0],size[1],size[2]);
	V3DPluginArgItem arg;
	arg.p=m_Filter->GetOutput();
	arg.type="outputImage";
	output.replace(0,arg);
    }


private:

    typename FilterType::Pointer   m_Filter;
};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }


bool NOTPlugin::dofunc(const QString & func_name,
    const V3DPluginArgList & input, V3DPluginArgList & output,  V3DPluginCallback2 & v3d,QWidget * parent)
{
   if (func_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK NOT 1.0 (2010-May-12): this plugin is developed by Sophie Chen.");
    return false ;
    }
	PluginSpecialized<unsigned char> *runner=new PluginSpecialized<unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 
 
	return true;
}


void NOTPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK NOT 1.0 (2010-May-12): this plugin is developed by Sophie Chen.");
    return;
    }

  v3dhandle curwin = callback.currentImageWindow();
  if (!curwin)
    {
    v3d_msg(tr("You don't have any image open in the main window."));
    return;
    }

  Image4DSimple *p4DImage = callback.getImage(curwin);
  if (! p4DImage)
    {
    v3d_msg(tr("The input image is null."));
    return;
    }

  EXECUTE_PLUGIN_FOR_INTEGER_PIXEL_TYPES;
}

