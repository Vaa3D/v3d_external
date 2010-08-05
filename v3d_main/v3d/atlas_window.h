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




//atlas_window.h
//by Hanchuan Peng
//revised from Gene Myers' simple toy example, with all methods changed
//080313

#include <QtGui>
#include <QtOpenGL>

/*
extern "C" {
#include "tiff_io.h"
#include "tiff_image.h"
}
*/

#include "../elementfunc/basic_c_fun/stackutil.h"

class StackWidget : public QWidget
{
  Q_OBJECT

  public:
    StackWidget(int width, int height, QWidget *parent = 0);
    ~StackWidget();

  public slots:
    void setImageTo(uchar *data, int zNom, int zDen);

  protected:
    void paintEvent(QPaintEvent *event);

  private:
    int     width;
    int     height;
    int     zoom;
    QImage *cimage;
    QImage *dimage;
};

class LoaderThread : public QThread
{
  Q_OBJECT

public:
  LoaderThread(QList<Tiff_Image *> *stack, Tiff_Reader *reader);

signals:
  void loadedTo(int);
  void finished();

private:
  void run();

  QList<Tiff_Image *> *stack;
  Tiff_Reader         *reader;
};

class ImageScrollArea : public QScrollArea
{
  Q_OBJECT

public:
  ImageScrollArea(QWidget *parent = 0);
  void scrollbarCheck();
  void setFitFlag(bool flag);

protected:
  void resizeEvent(QResizeEvent *event);

private:
  bool midUpdate;
  bool fits;
};

class AtlasWidget : public QWidget
{
  Q_OBJECT

public:
  AtlasWidget();
  int openImage(const QString &fileName);
  ~AtlasWidget();

private slots:
  void newPlane(int index);
  void vizChange();
  void colorChange(bool checked);
  void morePlanes(int stackCount);
  void quitLoader();
  void zoomUp();
  void zoomDown();

private:
  void resetColorTable(int i);
  void zoomAdjustments();

  int  zNom, zDen;
  int  controlHeight;

  QList<Tiff_Image *> stack;
  Tiff_Image         *first;
  LoaderThread       *loader;

  QCheckBox         **vizCheck;
  QToolButton       **colorBox;
  QColor             *colors;
  unsigned int      **tables;
  QScrollBar         *zplane;
  QLabel             *zoomLabel;
  QLabel             *planeLabel;
  ImageScrollArea    *scroller;

  StackWidget        *display;
  int                 current;
  unsigned int       *buffer;
};
