/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




#include <stdio.h>
#include <stdlib.h>

#include <QtGui>
#include <QGLWidget>
#include <QGLFormat>

#include "image_window.h"

#define ZOOM_MAX_FACTOR   16
#define ZOOM_MIN_WIDTH   150

static int scrollThickness;
static int scrollMarginWithBar;
static int scrollMarginWithoutBar;

#define MAC_SCROLL_AREA_MARGIN 2

static int deltaW;
static int deltaH;
static int controlWidth;

static int frameWidth;
static int frameHeight;

ImageWindow::~ImageWindow()
{ if (loader != 0)
    { loader->terminate();
      loader->wait();
      if (loader != 0)    // finished signal could have been issued while waiting!
        delete loader;
    }

  for (int i = 0; i < stack.count(); i++)
    Free_Tiff_Image(stack.at(i));

  if (buffer != 0)
    free(buffer);
  if (colorBox != 0)
    delete [] colorBox;
  if (vizCheck != 0)
    delete [] vizCheck;
  if (colors != 0)
    delete [] colors;
  if (tables != 0)
    { free(tables[0]);
      free(tables);
    }
}

ImageWindow::ImageWindow()
{ colorBox = 0;
  vizCheck = 0;
  colors   = 0;
  tables   = 0;
  buffer   = 0;
  loader   = 0;
  setWindowTitle(tr("My-Image-Viewer"));
  hide();
}

int ImageWindow::openImage(const QString &fileName)
{ int          islsm, endian;
  Tiff_Reader *reader;

  islsm = 1; // Eventually a real test for .lsm suffix

  reader = Open_Tiff_Reader(fileName.toAscii().data(),&endian,islsm);
  if (reader == NULL)
    { fprintf(stderr,"Error opening tif:\n  %s\n",Tiff_Error_String());
      //  Eventually a dialog saying can't open file.
      return (1);
    }

  while ( ! End_Of_Tiff(reader))
    { Tiff_IFD *ifd = Read_Tiff_IFD(reader);
      if (ifd == NULL)
        { fprintf(stderr,"Error reading IFD:\n  %s\n",Tiff_Error_String());
           //  Eventually a dialog saying can't open file.
           return (1);
        }
      else
        { int *tag, type, count;
          tag = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
          if (tag == NULL || (*tag & TIFF_VALUE_REDUCED_RESOLUTION) == 0)
            { Tiff_Image *timage = Get_Image_From_IFD(ifd);
              stack.append(timage);
              Free_Tiff_IFD(ifd);
              break;
            }
          Free_Tiff_IFD(ifd);
        }
    }

  if (stack.count() == 0)
    { fprintf(stderr,"Nothing in tiff file\n");
      // Eventually a dialog;
      return (1);
    }

  loader = new LoaderThread(&stack,reader);

  first  = stack.at(0);
  buffer = (unsigned int *)
              Guarded_Malloc(sizeof(unsigned int)*first->width*first->height,"Image Window");

  vizCheck = new QCheckBox * [first->number_channels];
  colorBox = new QToolButton * [first->number_channels];

  colors   = new QColor [first->number_channels];
  for (int i = 0; i < first->number_channels; i++)
    { colors[i++].setRgb(255,0,0);
      colors[i].setRgb(0,255,0);
    }

  tables = (unsigned int **)
              Guarded_Malloc(sizeof(unsigned int *)*first->number_channels,"Image Window");
  tables[0] = (unsigned int *)
                 Guarded_Malloc(sizeof(unsigned int)*first->number_channels*256,"Image Window");
  for (int i = 1; i < first->number_channels; i++)
    tables[i] = tables[i-1] + 256;

  for (int i = 0; i < first->number_channels; i++)
    resetColorTable(i);

  zNom = zDen = 1;

  display = new StackWidget(first->width,first->height);

  scroller = new ImageScrollArea();
    scroller->setWidget(display);
    scroller->setWidgetResizable(true);
    scroller->setFocusPolicy(Qt::NoFocus);
    scroller->setMaximumWidth(first->width);
    scroller->setMaximumHeight(first->height);

  zplane = new QScrollBar(Qt::Horizontal);
    zplane->setMinimum(0);
    zplane->setMaximum(0);
    zplane->setMaximumWidth(first->width);

  QVBoxLayout *imageLayout = new QVBoxLayout;
    imageLayout->addWidget(scroller,1);
    imageLayout->addWidget(zplane,0);
    imageLayout->addStretch(0);
    imageLayout->setSpacing(0);
    imageLayout->setMargin(0);

  QWidget *imagePart = new QWidget;
    imagePart->setLayout(imageLayout);
    imagePart->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);

  QWidget *botFiller = new QWidget;
    botFiller->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  QVBoxLayout *controlLayout = new QVBoxLayout;

  planeLabel = new QLabel();
    planeLabel->setText(tr("Z-Plane: 1/1"));
    planeLabel->setFixedHeight(20);
    planeLabel->setMinimumWidth(100);
    planeLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    planeLabel->setFrameStyle(QFrame::Raised|QFrame::Panel);
    planeLabel->setLineWidth(1);
    planeLabel->setAlignment(Qt::AlignHCenter);

  controlLayout->addWidget(planeLabel);

  for (int i = 0; i < first->number_channels; i++)
    { QCheckBox *button = new QCheckBox(tr("Channel %1").arg(i+1));
        button->setFixedWidth(100);
        button->setCheckState(Qt::Checked);

      QPixmap blob = QPixmap(16,16);
        blob.fill(colors[i]);

      QLabel *label = new QLabel(tr("Color: "));
      QToolButton *color = new QToolButton();
        color->setIcon(QIcon(blob));
        color->setIconSize(QSize(16,16));
        color->setCheckable(true);
        color->setFixedSize(20,20);
      QHBoxLayout *colorLayout = new QHBoxLayout;
        colorLayout->addWidget(label);
        colorLayout->addWidget(color);
        colorLayout->addStretch(1);

      QVBoxLayout *channelLayout = new QVBoxLayout;
        channelLayout->addWidget(button);
        channelLayout->addLayout(colorLayout);

      QGroupBox *channelBox = new QGroupBox;
        channelBox->setLayout(channelLayout);
        channelBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

      controlLayout->addWidget(channelBox);

      vizCheck[i] = button;
      colorBox[i] = color;
    }

  QToolButton *zoomIn = new QToolButton();
    zoomIn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    zoomIn->setText(tr("-"));
    zoomIn->setFixedSize(20,20);

  zoomLabel = new QLabel();
    zoomLabel->setText(tr("Zoom: 1"));
    zoomLabel->setFixedHeight(20);
    zoomLabel->setMinimumWidth(100);
    zoomLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    zoomLabel->setFrameStyle(QFrame::Raised|QFrame::Panel);
    zoomLabel->setLineWidth(1);
    zoomLabel->setAlignment(Qt::AlignHCenter);

  QToolButton *zoomOut = new QToolButton();
    zoomOut->setToolButtonStyle(Qt::ToolButtonTextOnly);
    zoomOut->setText(tr("+"));
    zoomOut->setFixedSize(20,20);

  QHBoxLayout *zoomLayout = new QHBoxLayout;
    zoomLayout->addWidget(zoomIn);
    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addWidget(zoomOut);
    zoomLayout->setSpacing(0);

  controlLayout->addLayout(zoomLayout);
  controlLayout->addWidget(botFiller);

  QWidget *controlPart = new QWidget;
    controlPart->setLayout(controlLayout);
    controlPart->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->addWidget(imagePart,1);
    toolLayout->addWidget(controlPart,0);
    toolLayout->setSpacing(0);
    toolLayout->setMargin(0);

  QWidget *windowFrame = new QWidget();
    windowFrame->setLayout(toolLayout);

  setCentralWidget(windowFrame);

  connect(zplane,SIGNAL(valueChanged(int)),this,SLOT(newPlane(int)));
  for (int s = 0; s < first->number_channels; s++)
    { connect(vizCheck[s],SIGNAL(stateChanged(int)),this,SLOT(vizChange()));
      connect(colorBox[s],SIGNAL(toggled(bool)),this,SLOT(colorChange(bool)));
    }
  connect(loader,SIGNAL(loadedTo(int)),this,SLOT(morePlanes(int)));
  connect(loader,SIGNAL(finished()),this,SLOT(quitLoader()));
  connect(zoomIn,SIGNAL(pressed()),this,SLOT(zoomUp()));
  connect(zoomOut,SIGNAL(pressed()),this,SLOT(zoomDown()));

  newPlane(0);

  scrollThickness        = zplane->sizeHint().height();
  scrollMarginWithoutBar = 2*MAC_SCROLL_AREA_MARGIN;
  scrollMarginWithBar    = 2*MAC_SCROLL_AREA_MARGIN + scrollThickness;

  controlWidth  = controlPart->minimumSizeHint().width();
  controlHeight = controlPart->minimumSizeHint().height();

  frameWidth  = frameGeometry().width() - geometry().width();
  frameHeight = frameGeometry().height() - geometry().height();

  deltaW = scrollMarginWithoutBar + controlWidth;
  deltaH = scrollMarginWithoutBar + scrollThickness;

  QRect screen = QApplication::desktop()->availableGeometry(this);

  while ((first->width*zNom)/zDen > screen.width()-(deltaW+frameWidth))
    zoomDown();
  while ((first->height*zNom)/zDen > screen.height()-(deltaH+frameHeight))
    zoomDown();

  int windowWidth  = (first->width*zNom)/zDen + deltaW;
  int windowHeight = (first->height*zNom)/zDen + deltaH;
  if (windowHeight < controlHeight)
    windowHeight = controlHeight;

  resize( windowWidth, windowHeight );
  setMaximumSize( windowWidth, windowHeight );

  show();

  return (0);
}

void ImageWindow::zoomAdjustments()
{
  if (zDen == 1)
    zoomLabel->setText(tr("Zoom: %1").arg(zNom));
  else
    zoomLabel->setText(tr("Zoom: %1/%2").arg(zNom).arg(zDen));

  int zoomWidth = (first->width*zNom)/zDen;
  int zoomHeight = (first->height*zNom)/zDen;

  QRect screen = QApplication::desktop()->availableGeometry(this);

  int windowWidth  = zoomWidth + deltaW;
  int windowHeight = zoomHeight + deltaH;
  int widthAdjust  = scrollMarginWithoutBar;
  int heightAdjust = scrollMarginWithoutBar;

  if (windowWidth <= screen.width() - frameWidth && windowHeight <= screen.height() - frameHeight)
    { if (windowHeight < controlHeight)
        windowHeight = controlHeight;
      setMaximumSize( windowWidth, windowHeight );
      scroller->setFitFlag(true);
    }
  else
    { windowWidth  += (scrollMarginWithBar - scrollMarginWithoutBar);
      windowHeight += (scrollMarginWithBar - scrollMarginWithoutBar);
      if (windowWidth > screen.width() - frameWidth)
        windowWidth = screen.width() - frameWidth;
      if (windowHeight > screen.height() - frameHeight)
        windowHeight = screen.height() - frameHeight;
      setMaximumSize( windowWidth, windowHeight );

      widthAdjust  = scrollMarginWithBar;
      heightAdjust = scrollMarginWithBar;

      if (geometry().width() < windowWidth)
        windowWidth = geometry().width();
      if (geometry().height() < windowHeight)
        windowHeight = geometry().height();
      scroller->setFitFlag(false);
    }

  scroller->setMaximumWidth(zoomWidth + widthAdjust);
  scroller->setMaximumHeight(zoomHeight + heightAdjust);
  zplane->setMaximumWidth(zoomWidth + widthAdjust);

  resize( windowWidth, windowHeight );

  scroller->scrollbarCheck();
}

void ImageWindow::zoomUp()
{ if (zNom >= ZOOM_MAX_FACTOR)         //  Really don't want to see a pixel as a 32x32 do you?
    return;

  if (zNom % 3 == 0)
    { zNom = ((zNom/3)*4) / zDen;
      zDen = 1;
    }
  else if (zDen % 3 == 0)
    zDen = (zDen/3)*2;   
  else if (zDen % 4 == 0)
    zDen = (zDen/4)*3;
  else if (zNom % 2 == 0)
    zNom = (zNom/2)*3;
  else
    { zNom  = 3;
      zDen *= 2;
    }

  zoomAdjustments();

  newPlane(current);
}

void ImageWindow::zoomDown()
{ if ((first->width*zNom)/zDen < ZOOM_MIN_WIDTH)  //  Limit zoomout width
    return;
  
  if (zNom % 3 == 0)
    { zNom = (zNom/3)*2;
      if (zDen > 1)
        { zNom =  1;
          zDen /= 2;
        }
    }
  else if (zDen % 3 == 0)
    zDen = (zDen/3)*4;
  else if (zNom % 4 == 0)
    zNom = (zNom/4)*3;
  else if (zDen % 2 == 0)
    zDen = (zDen/2)*3;
  else
    { zDen = 4/zNom;
      zNom = 3;
    }

  zoomAdjustments();

  newPlane(current);
}

void ImageWindow::vizChange()
{ newPlane(current); }

void ImageWindow::resetColorTable(int i)
{ int r, g, b;
  double bf, gf, rf;

  colors[i].getRgb(&r,&g,&b);
  bf = b/255.;
  gf = g/255.;
  rf = r/255.;
  for (int j = 0; j < 256; j++)
    tables[i][j] = ((int) (j*bf)) | (((int) (j*gf)) << 8) | (((int) (j*rf)) << 16);
}

void ImageWindow::colorChange(bool checked)
{ int s;

  if (!checked) return;

  for (s = 0; s < first->number_channels; s++)
    if (colorBox[s]->isChecked())
      break;

  colors[s] = QColorDialog::getColor(colors[s]);
  colorBox[s]->setChecked(false);

  QPixmap blob = QPixmap(16,16);
     blob.fill(colors[s]);
  colorBox[s]->setIcon(QIcon(blob));
  resetColorTable(s);
  newPlane(current);
}

void ImageWindow::newPlane(int index)
{ int disjoint;
  Tiff_Image *image = stack.at(index);
  current = index;

  { int R, G, B;
    int r, g, b;

    R = G = B = 0;
    for (int s = 0; s < first->number_channels; s++)
      { colors[s].getRgb(&r,&g,&b);
        if (r > 0) R += 1;
        if (g > 0) G += 1;
        if (b > 0) B += 1;
      }
    disjoint = (R <= 1 && B <= 1 && G <= 1);
  }

  int p = 0;
  for (int s = 0; s < first->number_channels; s++)
    { unsigned int *table = tables[s];
      unsigned char *data = (unsigned char *) image->channels[s].plane;

      if (vizCheck[s]->checkState() != Qt::Checked)
        continue;

      if (p == 0)
        for (int i = 0; i < image->width*image->height; i++)
          buffer[i] = table[data[i]];
      else if (disjoint)
        for (int i = 0; i < image->width*image->height; i++)
          buffer[i] += table[data[i]];
      else
        for (int i = 0; i < image->width*image->height; i++)
          { unsigned int d = table[data[i]];
            unsigned int b = buffer[i];
            unsigned int x;

            x = (d & 0xff0000);
            if (x > (b & 0xff0000))
              b |= x;
            x = (d & 0xff00);
            if (x > (b & 0xff00))
              b |= x;
            x = (d & 0xff);
            if (x > (b & 0xff))
              b |= x;
            buffer[i] = b;
          }
      p += 1;
    }

  if (p == 0)
    for (int i = 0; i < image->width*image->height; i++)
      buffer[i] = 0;

  display->setImageTo((uchar *) buffer,zNom,zDen);

  planeLabel->setText(tr("Z-Plane: %1/%2").arg(current+1).arg(stack.count()));
}


/***************************************************************************************/
/*                                                                                     */
/*   CUSTOM SCROLL BAR BEHAVIOR                                                        */
/*                                                                                     */
/***************************************************************************************/

ImageScrollArea::ImageScrollArea(QWidget *parent) : QScrollArea(parent)
{ fits      = true;
  midUpdate = false;
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ImageScrollArea::scrollbarCheck()
{ QSize viewSize(viewport()->geometry().size());
  QResizeEvent dummy(viewSize,viewSize);
  resizeEvent(&dummy);
}

void ImageScrollArea::setFitFlag(bool flag)
{ fits = flag; }

void ImageScrollArea::resizeEvent(QResizeEvent *event)
{
  //  event->size() is the size of the viewport, not the scrollbar widget

  if (midUpdate) return;
  midUpdate = true;
  if (fits)
    { if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOn)
        { if (event->size().width() >= maximumWidth()-scrollMarginWithBar &&
              event->size().height() >= maximumHeight()-scrollMarginWithBar)
            { setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
              setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            }
        }
      else
        { if (event->size().width() < maximumWidth()-scrollMarginWithoutBar ||
              event->size().height() < maximumHeight()-scrollMarginWithoutBar)
            { setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
              setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            }
        }
    }
  else
    { if (event->size().width() >= maximumWidth()-scrollMarginWithBar)
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      else
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      if (event->size().height() >= maximumHeight()-scrollMarginWithBar)
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      else
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
  midUpdate = false;
  QScrollArea::resizeEvent(event);
}


/***************************************************************************************/
/*                                                                                     */
/*   LOADER THREAD                                                                     */
/*                                                                                     */
/***************************************************************************************/

LoaderThread::LoaderThread(QList<Tiff_Image *> *s, Tiff_Reader *r)
{ stack  = s;
  reader = r;
  start(NormalPriority);
}

void ImageWindow::morePlanes(int stackCount)   // connected to signal LoaderThread::loadedTo
{ zplane->setMaximum(stackCount-1);
  planeLabel->setText(tr("Z-Plane: %1/%2").arg(current+1).arg(stackCount));
}

void ImageWindow::quitLoader()   // connected to signal LoaderThread::finished();
{ if (loader != 0)
    { loader->wait();
      delete loader;
      loader = 0;
    }
}

void LoaderThread::run()
{ int loadCount; 

  loadCount = 0;
  while ( ! End_Of_Tiff(reader))
    { Tiff_IFD *ifd = Read_Tiff_IFD(reader);
      if (ifd == NULL)
        { fprintf(stderr,"Error reading IFD:\n  %s\n",Tiff_Error_String());
           //  Eventually a dialog saying can't read to end of file
          emit finished();
          return;
        }
      else
        { int *tag, type, count;
          tag = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
          if (tag == NULL || (*tag & TIFF_VALUE_REDUCED_RESOLUTION) == 0)
            { Tiff_Image *timage = Get_Image_From_IFD(ifd);
              stack->append(timage);
              loadCount += 1;
            }
          Free_Tiff_IFD(ifd);
          if (loadCount == 10)
            { emit loadedTo(stack->count());
              loadCount = 0;
            }
        }
    }
  emit loadedTo(stack->count());
  Free_Tiff_Reader(reader);
  emit finished();
}


/***************************************************************************************/
/*                                                                                     */
/*   STACK WIDGET                                                                      */
/*                                                                                     */
/***************************************************************************************/

static unsigned int *zoomBuf = 0;

StackWidget::~StackWidget()
{ if (dimage != cimage && dimage != 0)
    delete dimage;
  if (cimage != 0)
    delete cimage;
}

StackWidget::StackWidget(int w, int h, QWidget *parent) : QWidget(parent)
{ setFixedSize(w,h);
  width  = w;
  height = h;
  cimage = 0;
  dimage = 0;
  zoom   = 1;
  if (zoomBuf == 0)
    { QRect screen = QApplication::desktop()->availableGeometry(this);
      zoomBuf = (unsigned int *)
                     Guarded_Malloc((screen.width()+(ZOOM_MAX_FACTOR-1))*
                                    (screen.height()+(ZOOM_MAX_FACTOR-1))*sizeof(unsigned int), 
                                       "Image Window");
    }
}

void StackWidget::setImageTo(uchar *data, int zNom, int zDen)
{ if (dimage != cimage && dimage != 0)
    delete dimage;
  if (cimage != 0)
    delete cimage;

  if (geometry().width() != (width*zNom)/zDen)
    setFixedSize((width*zNom)/zDen,(height*zNom)/zDen);

  cimage = new QImage(data,width,height,QImage::Format_RGB32);
  if (zDen > zNom || zDen > 1)
    dimage = new QImage(cimage->scaled((width*zNom)/zDen,(height*zNom)/zDen));
  else
    dimage = cimage;
  if (zDen == 1 && zNom >= 2)
    zoom = zNom;
  else
    zoom = 1;

  update();
}

void StackWidget::paintEvent(QPaintEvent *event)
{ QPainter painter;

  QRect rect = event->rect();
  painter.begin(this);
  if (zoom == 1)
    painter.drawImage(rect,*dimage,rect);
  else
    { const uchar *data  = cimage->bits();  
      int          width = cimage->width();

      int lx = rect.x()/zoom;
      int ly = rect.y()/zoom;
      int hx = lx + ((rect.width()-1)/zoom+1);
      int hy = ly + ((rect.height()-1)/zoom+1);

      QRect source( QPoint( rect.x() % zoom, rect.y() % zoom), rect.size());

      unsigned int *p, *q;
      int i, j;
      int t, u;

      q = zoomBuf;
      p = ((unsigned int *) data) + ly*width;
      for (j = ly; j < hy; j++)
        { for (t = 0; t < zoom; t++)
            for (i = lx; i < hx; i++)
              { unsigned int d = p[i];
                for (u = 0; u < zoom; u++)
                  *q++ = d;
              }
          p += width;
        }
      
//      QImage shallow((const uchar *) zoomBuf,(hx-lx)*zoom,(hy-ly)*zoom,QImage::Format_RGB32);
      QImage shallow(( uchar *) zoomBuf,(hx-lx)*zoom,(hy-ly)*zoom,QImage::Format_RGB32);
      painter.drawImage(rect,shallow,source);  // upgrade to actually do it
    }
  painter.end();
}
