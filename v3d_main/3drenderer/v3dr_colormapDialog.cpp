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
 * V3dr_colormapDialog.cpp
 *
 *  Created on: Dec 14, 2008
 *      Author: ruanzongcai
 *  last edit: by Hanchuan Peng, 2010-01-30
 */

#include "v3dr_colormapDialog.h"


///////////////////////////////////////////////////////////
#define UPDATE_VIEW(w)   if (w)  w->update();
///////////////////////////////////////////////////////////


V3dr_colormapDialog::V3dr_colormapDialog(V3dR_GLWidget* w, QWidget *parent)
    :SharedToolDialog(w, parent)
{
    qDebug("V3dr_colormapDialog::V3dr_colormapDialog");

    setWindowTitle(tr("Volume Colormap")); //Color Map Options")); // 090423 RZC: changed

    glwidget = 0;
    renderer = 0;
    bCanUndo = bMod = false;

    undoButton=loadButton=saveButton=applyButton = 0;

    createFirst();
    linkTo(w);/////

   // shortkeyClose = new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(hide()));
}

V3dr_colormapDialog::~V3dr_colormapDialog()
{
    qDebug("V3dr_colormapDialog::~V3dr_colormapDialog");
    if (glwidget) glwidget->clearColormapDialog();
}


void V3dr_colormapDialog::linkTo(QWidget* w) //point to new view
{
    if (! w)  return;

    IncRef(w); //DecRef(w);
    qDebug("  V3dr_colormapDialog::linkTo ( %p )  ref=%d", w, ref);

    glwidget = (V3dR_GLWidget*)w;
    renderer = (Renderer_gl2*)(glwidget->getRenderer());

    if (renderer && (renderer->data_unitbytes >1)) applyButton->setEnabled(false);
    bCanUndo = bMod = false;

    saveOldcurve();
    //updateStops();
    QTimer::singleShot(100, this, SLOT( updateStops() )); // So STRANGE!, may because Resize event
}


void V3dr_colormapDialog::createFirst()
{
    QGroupBox *editorGroup[N_CHANNEL];
    for(int i=0; i<N_CHANNEL; i++)
    {
        m_editor[i] = 0;
        editorGroup[i] = 0;

        editorGroup[i] = new QGroupBox(this);
        editorGroup[i]->setTitle(tr("Channel %1").arg(i+1));
        m_editor[i] = new GradientEditor(editorGroup[i]);

        //editorGroup[i]->setMinimumSize(180,400);
    }


    QGroupBox *presetGroup = new QGroupBox(this);
    presetGroup->setTitle("Presets");
//   QVBoxLayout *presetLayout = new QVBoxLayout(presetGroup);
    QGridLayout *presetLayout = new QGridLayout(presetGroup);

    QPushButton *default0Button = new QPushButton("Default");
    QPushButton *default1Button = new QPushButton("Gray");
    QPushButton *default2Button = new QPushButton("Red -> Gray");
    QPushButton *default3Button = new QPushButton("Green -> Gray");
    QPushButton *default4Button = new QPushButton("Blue -> Gray");
    QPushButton *default5Button = new QPushButton("cm1");
    QPushButton *default6Button = new QPushButton("cm2");
//	QComboBox *defaultList = new QComboBox(btnGroup);
//	QStringList qsl;
//	qsl << "Default" << "Preset 1" << "Preset 2" << "Preset 3" << "Preset 4";
//	defaultList->addItems(qsl);

    presetLayout->addWidget(default0Button,		1,0, 1,1);
    presetLayout->addWidget(default1Button,		2,0, 1,1);
    presetLayout->addWidget(default2Button,		3,0, 1,1);
    presetLayout->addWidget(default3Button,		4,0, 1,1);
    presetLayout->addWidget(default4Button,		5,0, 1,1);
    presetLayout->addWidget(default5Button,		6,0, 1,1);
    presetLayout->addWidget(default6Button,		7,0, 1,1);
//    presetLayout->addWidget(defaultList);


    QGroupBox *btnGroup = new QGroupBox(this);
    QVBoxLayout *btnLayout = new QVBoxLayout(btnGroup);

    undoButton = new QPushButton("Undo");
    loadButton = new QPushButton("Load");
    saveButton = new QPushButton("Save");
    applyButton = new QPushButton("Apply to image");

    btnLayout->addWidget(undoButton);
    btnLayout->addWidget(loadButton);
    btnLayout->addWidget(saveButton);
    //btnLayout->addSpacing(15);
    btnLayout->addWidget(applyButton);


    QGroupBox *controlGroup = new QGroupBox(this);
    controlGroup->setFixedWidth(150);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    controlLayout->addWidget(presetGroup);
    controlLayout->addWidget(btnGroup);
    controlLayout->addStretch(0);
    controlLayout->setContentsMargins(0,0,0,0);


    QHBoxLayout *allLayout = new QHBoxLayout(this);
    for(int i=0; i<N_CHANNEL; i++)
    {
        allLayout->addWidget(editorGroup[i]);
    }
    allLayout->addWidget(controlGroup);
    HALF_MARGINS(allLayout);


    // Signal
    connect(default0Button, SIGNAL(clicked()), this, SLOT(setDefault0()));
    connect(default1Button, SIGNAL(clicked()), this, SLOT(setDefault1()));
    connect(default2Button, SIGNAL(clicked()), this, SLOT(setDefault2()));
    connect(default3Button, SIGNAL(clicked()), this, SLOT(setDefault3()));
    connect(default4Button, SIGNAL(clicked()), this, SLOT(setDefault4()));
    connect(default5Button, SIGNAL(clicked()), this, SLOT(setDefault5()));
    connect(default6Button, SIGNAL(clicked()), this, SLOT(setDefault6()));
//    if (defaultList) connect(defaultList, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefault(int)));

    if (undoButton)  connect(undoButton, SIGNAL(clicked()), this, SLOT(undo()));
    if (loadButton)  connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));
    if (saveButton)  connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));

    if (applyButton)  connect(applyButton, SIGNAL(clicked()), this, SLOT(applyToImage()));

    for(int i=0; i<N_CHANNEL; i++)
    {
        connect(m_editor[i], SIGNAL(gradientStopsChanged(const QGradientStops &)), this, SLOT(updateColormap()));
    }


    // Initial after connect updateColormap
    saveOldcurve();
    //updateStops();
    QTimer::singleShot(100, this, SLOT( updateStops() )); // So STRANGE!,, may because Resize event
    //QTimer::singleShot(100, this, SLOT( setDefault() )); // So STRANGE!

}

void V3dr_colormapDialog::setDefault(int ich, int jch, int k)
// k= 0(0-0), 1(1-1), 2(0.5-0.5), 3,4(0-1), 5(1-0), 6(0-1-0), 7(0-1-1-0), 8(0-1-0-1)
{
//	QGradientStops stops;
//	#define PRESET(x, a)  {int b = int((a)*255+0.5); stops << QGradientStop((x), QColor(b,b,b,b)); }

    QPolygonF curve;
    #define PRESET(x, a)  { curve << QPointF((x), (a)); }

    switch (k)
    {
    case 0: //0-0
        PRESET(0.00, 0);
        PRESET(1.00, 0);
        break;
    case 1: //1-1
        PRESET(0.00, 1);
        PRESET(1.00, 1);
        break;
    case 2: //0.5-0.5
        PRESET(0.00, 0.5);
        PRESET(1.00, 0.5);
        break;
    default:
    case 4: //0-1
        PRESET(0.00, 0);
        PRESET(1.00, 1);
        break;
    case 5: //1-0
        PRESET(0.00, 1);
        PRESET(1.00, 0);
        break;
    case 6: //0-1-0
        PRESET(0.00, 0);
        PRESET(0.50, 1);
        PRESET(1.00, 0);
        break;
    case 7: //0-1-1-0
        PRESET(0.00, 0);
        PRESET(0.33, 1);
        PRESET(0.66, 1);
        PRESET(1.00, 0);
        break;
    case 8: //0-1-0-1
        PRESET(0.00, 0);
        PRESET(0.33, 1);
        PRESET(0.66, 0);
        PRESET(1.00, 1);
        break;
    }

    if (m_editor[ich])
    {
//    	bool mask[4];  	for (int j=0; j<4; j++)  mask[j] = (j==jch);
//    	m_editor[ich]->setGradientStops(mask, stops);

        m_editor[ich]->setNormalCurve(jch, curve);
    }
}

void V3dr_colormapDialog::updateStops()
{
    //qDebug() << "V3dr_colormapDialog::updateStops";
    if (! renderer)  return;

    for(int i=0; i<N_CHANNEL; i++)
    {
        if (! m_editor[i])  continue;

        for (int j=0; j<4; j++)
        {
            m_editor[i]->setNormalCurve(j, renderer->colormap_curve[i][j]);

        //	qDebug() << QString("[%1][%2]").arg(i).arg(j) << renderer->colormap_curve[i][j];
        }
    }
    updateColormap();  //081220
}

void V3dr_colormapDialog::updateColormap()
{
    //qDebug() << "V3dr_colormapDialog::updateColormap";

    if (! renderer)  return;

    bMod = true;
    undoButton->setEnabled(bCanUndo && bMod);

    for(int i=0; i<N_CHANNEL; i++)
    {
        if (! m_editor[i])  continue;

        for (int j=0; j<4; j++)
        {
            renderer->colormap_curve[i][j] = m_editor[i]->normalCurve(j); //081219

        //	qDebug() << QString("[%1][%2]").arg(i).arg(j) << renderer->colormap_curve[i][j];
        }

        for (int k=0; k<=255; k++)
        {
            QRgb argb = m_editor[i]->colorF( k/255.0 );
            (renderer->colormap[i][k]).r = (unsigned char)qRed(argb);
            (renderer->colormap[i][k]).g = (unsigned char)qGreen(argb);
            (renderer->colormap[i][k]).b = (unsigned char)qBlue(argb);
            (renderer->colormap[i][k]).a = (unsigned char)qAlpha(argb);
            //printf("{[%d][%d]%08x}",0,k, (renderer->colormap[0][k]).i);
        }
    }

    UPDATE_VIEW(glwidget);
}

void V3dr_colormapDialog::saveOldcurve()
{
    if (! renderer)  return;

    // save renderer's stops
    for(int i=0; i<N_CHANNEL; i++)
    for(int j=0; j<4; j++)
    {
        oldcurve[i][j] = renderer->colormap_curve[i][j];
    }
    bCanUndo = true;
    undoButton->setEnabled(bCanUndo && bMod);
}

void V3dr_colormapDialog::undo()
{
    qDebug("  V3dr_colormapDialog::undo");

    if (bCanUndo && bMod && renderer)
    {
        // restore the old state
        for(int i=0; i<N_CHANNEL; i++)
        for (int j=0; j<4; j++)
        {
            renderer->colormap_curve[i][j] = oldcurve[i][j];
        }

        updateStops();
    }
    //bCanUndo = false; // 090707 to be solved later
    undoButton->setEnabled(bCanUndo && bMod);

    UPDATE_VIEW(glwidget);
}

void V3dr_colormapDialog::load()
{
    qDebug("Vaa3dr_colormapDialog::load");
    hide();

    QString filename = QFileDialog::getOpenFileName(0, QObject::tr("Open Color Map File"),
            ".vaa3dcm",
            QObject::tr("Vaa3D Color Map File (*.vaa3dcm);;(*.*)"
                    ));
    qDebug()<< "load file: " << filename;

    try {

        if (filename.size())   loadColormapFile(filename);

    } catch (...) { }

    show();
}

void V3dr_colormapDialog::save()
{
    qDebug("Vaa3dr_colormapDialog::save");
    hide();

    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save Color Map File"),
            ".vaa3dcm",
            QObject::tr("Vaa3D Color Map File (*.vaa3dcm);;(*.*)"
                    ));
    qDebug()<< "save file: " << filename;

    try {

        if (filename.size())   saveColormapFile(filename);

    } catch (...) { }

    show();
}

void V3dr_colormapDialog::loadColormapFile(const QString& filename)
{
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(0, QObject::tr("open file"), QObject::tr("open file [%1] failed!").arg(filename));
        return;
    }

    PROGRESS_DIALOG("Loading Colormap", this);
    PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102


    QPolygonF curve[N_CHANNEL][4];

    int count = 0;
    qDebug("-------------------------------------------------------");
    while (! qf.atEnd())
    {
        char _buf[200], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++);
        if (buf[0]=='#' ||buf[0]=='\0')	continue;

        count++;
        int ich, jch;
        qreal x, y;

        QStringList qsl = QString(buf).split(" ", Qt::SkipEmptyParts); // 090528 RZC: add QString::SkipEmptyParts
        if (qsl.size()==0)   continue;

        for (int i=0; i<qsl.size(); i++)
        {
            qsl[i].truncate(99);
            if (i==0) ich = qsl[i].toInt();
            if (i==1) jch = qsl[i].toInt();
            if (i==2) x = qsl[i].toFloat();
            if (i==3) y = qsl[i].toFloat();
        }
        //qDebug("%s  ///  %d %d (%g %g)", buf,  ich, jch, x, y);
        {
            ich = qBound(0, ich, N_CHANNEL);
            jch = qBound(0, jch, 3);
            x = qBound(0.0, x, 1.0);
            y = qBound(0.0, y, 1.0);

            curve[ich][jch] << QPointF(x, y);
        }
    }
    qDebug("---------------------read %d lines", count);


    for(int i=0; i<N_CHANNEL; i++)
    {
        if (! m_editor[i])  continue;

        for (int j=0; j<4; j++)
        {
            m_editor[i]->setNormalCurve(j, curve[i][j]);
        }
    }
    updateColormap();
}

void V3dr_colormapDialog::saveColormapFile(const QString& filename)
{
    QFile qf(filename);
    if (! qf.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(0, QObject::tr("open file"), QObject::tr("open file [%1] failed!").arg(filename));
        return;
    }

    PROGRESS_DIALOG("Saving Colormap", this);
    PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102


    QPolygonF curve[N_CHANNEL][4];
    for(int i=0; i<N_CHANNEL; i++)
    {
        if (! m_editor[i])  continue;

        for (int j=0; j<4; j++)
        {
            curve[i][j] = m_editor[i]->normalCurve(j);
        }
    }

    int count = 0;
    qDebug("-------------------------------------------------------");

    char buf[200];
    sprintf(buf, "# ich(0--n-1) jch(0--3) x y \n");	//qDebug("%s", buf);
    qf.write(buf, strlen(buf));

    for(int i=0; i<N_CHANNEL; i++)
    for (int j=0; j<4; j++)
    {
        for (int k=0; k<curve[i][j].size(); k++)
        {
            qreal x = curve[i][j].at(k).x();
            qreal y = curve[i][j].at(k).y();

            sprintf(buf, "curve %d %d %g %g\n", i,j, x,y);	//qDebug("%s", buf);
            qf.write(buf, strlen(buf));

            count++;
        }
    }
    qDebug("---------------------write %d lines", count);
}

void V3dr_colormapDialog::applyToImage()
{
    //qDebug("V3dr_colormapDialog::applyToImage");

    if (QMessageBox::question(0, QObject::tr("Applying Colormap to Image"),  //100810: Cmd-V instead of Cmd-R
                        tr("Are you sure to APPLY current colormap to the image stack in Tri-view ? \n\n"
                           "(You can press Ctrl/Cmd-V to update the modified data into 3D-view after applying colormap)."),
                        QMessageBox::No | QMessageBox::Yes,
                        QMessageBox::Yes)
        ==QMessageBox::No)
        return;

    PROGRESS_DIALOG("Applying colormap to the image stack in tri-view", this);
    PROGRESS_PERCENT(1); // 0 or 100 not be displayed. 081102

    if (renderer)	renderer->applyColormapToImage();

    PROGRESS_PERCENT(100);
}
