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




/*
 * V3dr_colormapDialog.h
 *
 *  Created on: Dec 14, 2008
 *      Author: ruanzongcai
 *  last edit: by Hanchuan Peng, 2010-01-30
 */

#ifndef V3DR_COLORMAPDIALOG_H_
#define V3DR_COLORMAPDIALOG_H_

#include "v3dr_common.h"
#include "renderer_gl2.h"
#include "v3dr_glwidget.h"
//class V3dR_GLWidget;
//class Renderer_gl2;

#include "qtr_widget.h"
#include "gradients.h"


class V3dr_colormapDialog : public SharedToolDialog
{
   // Q_OBJECT;

public:
    V3dr_colormapDialog(V3dR_GLWidget* w, QWidget *parent=0);
    virtual ~V3dr_colormapDialog();

protected:
    V3dR_GLWidget* glwidget;
    Renderer_gl2* renderer;
    GradientEditor *m_editor[N_CHANNEL];
    QPolygonF oldcurve[N_CHANNEL][4];
    bool bCanUndo, bMod;

    QShortcut *shortkeyClose;
    QPushButton *undoButton, *loadButton, *saveButton;
    QPushButton *applyButton;

    void createFirst();
    void saveOldcurve();
    void loadColormapFile(const QString& filename);
    void saveColormapFile(const QString& filename);

    //void setDefault(int ich, int jch, int k);
    // k= 0(0-0), 1(1-1), 2(0.5-0.5), 3,4(0-1), 5(1-0), 6(0-1-0), 7(0-1-1-0), 8(0-1-0-1)

//public slots:
//    virtual void linkTo(QWidget* w); //link to new view

//    void setDefault(int k)
//    {
//        switch(k)
//        {
//        default:
//        case 0: // RGB A(0-1)
//            setDefault(0,0, 1); setDefault(0,1, 0); setDefault(0,2, 0); setDefault(0,3, 4);
//            setDefault(1,0, 0); setDefault(1,1, 1); setDefault(1,2, 0); setDefault(1,3, 4);
//            setDefault(2,0, 0); setDefault(2,1, 0); setDefault(2,2, 1); setDefault(2,3, 4);
//            break;
//        case 1: // Gray A(0-1)
//            setDefault(0,0, 1); setDefault(0,1, 1); setDefault(0,2, 1); setDefault(0,3, 4);
//            setDefault(1,0, 1); setDefault(1,1, 1); setDefault(1,2, 1); setDefault(1,3, 4);
//            setDefault(2,0, 1); setDefault(2,1, 1); setDefault(2,2, 1); setDefault(2,3, 4);
//            break;
//        case 2: // Red2gray
//            setDefault(0,0, 1); setDefault(0,1, 1); setDefault(0,2, 1); setDefault(0,3, 4);
//            setDefault(1,0, 0); setDefault(1,1, 1); setDefault(1,2, 0); setDefault(1,3, 0);
//            setDefault(2,0, 0); setDefault(2,1, 0); setDefault(2,2, 1); setDefault(2,3, 0);
//            break;
//        case 3: // Green2gray
//            setDefault(0,0, 1); setDefault(0,1, 0); setDefault(0,2, 0); setDefault(0,3, 0);
//            setDefault(1,0, 1); setDefault(1,1, 1); setDefault(1,2, 1); setDefault(1,3, 4);
//            setDefault(2,0, 0); setDefault(2,1, 0); setDefault(2,2, 1); setDefault(2,3, 0);
//            break;
//        case 4: // Blue2gray
//            setDefault(0,0, 1); setDefault(0,1, 0); setDefault(0,2, 0); setDefault(0,3, 0);
//            setDefault(1,0, 0); setDefault(1,1, 1); setDefault(1,2, 0); setDefault(1,3, 0);
//            setDefault(2,0, 1); setDefault(2,1, 1); setDefault(2,2, 1); setDefault(2,3, 4);
//            break;
//        case 5: // CMY^2 A(0-1)
//            setDefault(0,0, 0); setDefault(0,1, 4); setDefault(0,2, 4); setDefault(0,3, 4);
//            setDefault(1,0, 4); setDefault(1,1, 0); setDefault(1,2, 4); setDefault(1,3, 4);
//            setDefault(2,0, 4); setDefault(2,1, 4); setDefault(2,2, 0); setDefault(2,3, 4);
//            break;
//        case 6: // GBR^2 A(0-1-1-0)
//            setDefault(0,0, 0); setDefault(0,1, 0); setDefault(0,2, 4); setDefault(0,3, 7);
//            setDefault(1,0, 4); setDefault(1,1, 0); setDefault(1,2, 0); setDefault(1,3, 7);
//            setDefault(2,0, 0); setDefault(2,1, 4); setDefault(2,2, 0); setDefault(2,3, 7);
//            break;
//		case 5: // CMY A(0-1-0-1)
//			setDefault(0,0, 0); setDefault(0,1, 2); setDefault(0,2, 2); setDefault(0,3, 8);
//			setDefault(1,0, 2); setDefault(1,1, 0); setDefault(1,2, 2); setDefault(1,3, 8);
//			setDefault(2,0, 2); setDefault(2,1, 2); setDefault(2,2, 0); setDefault(2,3, 8);
//			break;
//        }
//        updateColormap();
//    }
//    void setDefault0() {setDefault(0);}
//    void setDefault1() {setDefault(1);}
//    void setDefault2() {setDefault(2);}
//    void setDefault3() {setDefault(3);}
//    void setDefault4() {setDefault(4);}
//    void setDefault5() {setDefault(5);}
//    void setDefault6() {setDefault(6);}

//    void undo();
//    void updateStops();
//    void updateColormap();

//    void load();
//    void save();

//    void applyToImage();

};

#endif /* V3DR_COLORMAPDIALOG_H_ */
