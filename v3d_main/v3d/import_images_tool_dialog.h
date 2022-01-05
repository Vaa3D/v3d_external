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




/****************************************************************************
**
import_images_tool_dialog.h
 by Hanchuan Peng
 2009-05-07
 revised by Yu Yang, 090508

**
****************************************************************************/

#ifndef __V3D_IMPORT_IMAGES_TOOL_DIALOG_H__
#define __V3D_IMPORT_IMAGES_TOOL_DIALOG_H__

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif

#include "ui_import_images_tool.h"

#include "../basic_c_fun/v3d_basicdatatype.h"

struct ImportImgPara
{
    V3DLONG startImg;
    V3DLONG endImg;
    V3DLONG countImg;
	QString filt;
    V3DLONG inc;
	int packType;
};

class import_images_tool_Dialog : public QDialog, public Ui_import_images_tool
{
    Q_OBJECT

public:
    import_images_tool_Dialog(QString folderName)
	{
		setupUi(this);

		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		//initialization codes for read directory
        numimgBox->setMaximum(10000000);
        startimgBox->setMaximum(10000000);
		startimgBox->setMinimum(1);
        incBox->setMaximum(10000000);
        incBox->setMinimum(1);
        endimgBox->setMaximum(10000000);
        endimgBox->setMinimum(1);

		startimgBox->setValue(1);
		incBox->setValue(1);
        endimgBox->setValue(1);
        filterEdit->setText("");

		comboPack->clear();
		comboPack->insertItem(0, "Pack images in 'Z' dimension");
		comboPack->insertItem(1, "Pack images in 'Channel' dimension");
		comboPack->setCurrentIndex(0);
	}

	void fetchData(ImportImgPara *p)
	{
		if (!p) return;
		p->countImg = numimgBox->value();
		p->startImg = startimgBox->value();
        p->endImg = endimgBox->value();
        p->inc = incBox->value();
		p->filt = filterEdit->text();
		p->packType = comboPack->currentIndex();
	}

private:
	QStringList mystringlist;

};


#endif
