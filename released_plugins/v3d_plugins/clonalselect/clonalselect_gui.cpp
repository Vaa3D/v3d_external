/* clonalselect_gui.cpp
 * 2013-01-13: create this program by Yang Yu
 */


#ifndef __CLONALSELECT_GUI_CPP__
#define __CLONALSELECT_GUI_CPP__

//
#include "clonalselect_gui.h"

// Open a series of inputs
QStringList importSeriesFileList(const QString & curFilePath, char* suffix)
{
    QStringList myList;
    myList.clear();

    // get the files namelist in the directory
    QStringList fileSuffix;
    fileSuffix<<suffix;

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(fileSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    return myList;
}

// clonal selecting class
ColonalSelectWidget::ColonalSelectWidget(V3DPluginCallback &callback, QWidget *parentWidget)
{
    //
    m_callback = &callback;

    //
    /// create a dialog
    //

    // image
    m_winlist = callback.getImageWindowList();

    v3dhandle wincurr = callback.currentImageWindow(); // focused image
    QString itemcurr = callback.getImageName(wincurr);
    int idxcurr = 0;

    QStringList items;
    for (int i=0; i<m_winlist.size(); i++)
    {
        QString item = callback.getImageName(m_winlist[i]);

        items << item;

        if(item.compare(itemcurr) == 0)
            idxcurr = i;
    }

    combo_subject = new QComboBox(); combo_subject->addItems(items);
    combo_subject->setCurrentIndex(idxcurr);

    label_subject = new QLabel(QObject::tr("Image: "));

    // clonal masks
    label_mask = new QLabel(QObject::tr("Clonal mask directory: "));

    m_maskfolder = QString();
    QSettings settings("ClonalSelect", "dir");
    m_maskfolder = settings.value("path").toString();

    if(!m_maskfolder.isEmpty() && QDir(m_maskfolder).exists())
    {
        edit_mask = new QLineEdit(m_maskfolder);
    }
    else
    {
        edit_mask = new QLineEdit(QDir::currentPath());
    }

    pb_browse_mask = new QPushButton("Browse...");


    // list
    label_cmlist = new QLabel(QObject::tr("Clonals: "));

    listWidget = new QListWidget();

    updateDir(m_maskfolder);

    // threshold
    slider_threshold = new QSlider(Qt::Horizontal);
    slider_threshold->setTickPosition(QSlider::TicksBothSides);
    slider_threshold->setMinimum(0); slider_threshold->setMaximum(100);
    slider_threshold->setValue(10);
    m_threshold = 0.1;
    label_threshold = new QLabel(QObject::tr("Threshold:\t%1").arg(m_threshold));

    // select button
    label_select = new QLabel(QObject::tr("Annotation: "));
    button_select = new QPushButton(QObject::tr("Select"));


    // layout
    settingGroupLayout = new QGridLayout();

    settingGroupLayout->addWidget(label_subject, 0,0);
    settingGroupLayout->addWidget(combo_subject, 0,1);

    settingGroupLayout->addWidget(label_mask, 1,0);
    settingGroupLayout->addWidget(edit_mask, 1,1);
    settingGroupLayout->addWidget(pb_browse_mask, 1,2);

    settingGroupLayout->addWidget(label_cmlist, 2,0);
    settingGroupLayout->addWidget(listWidget, 2,1);

    settingGroupLayout->addWidget(label_threshold, 3,0);
    settingGroupLayout->addWidget(slider_threshold, 3,1);

    settingGroupLayout->addWidget(label_select, 4,0);
    settingGroupLayout->addWidget(button_select, 4,1);

    setLayout(settingGroupLayout);
    setWindowTitle(QString("Clonal Selecting"));

    // signal and slot
    connect(pb_browse_mask, SIGNAL(clicked()), this, SLOT(getMaskDir()));
    connect(edit_mask, SIGNAL(textChanged(QString)), this, SLOT(updateDir(QString)));
    connect(slider_threshold, SIGNAL(valueChanged(int)), this, SLOT(setThreshold(int)));
    connect(button_select, SIGNAL(clicked()), this, SLOT(selectClonal())); //

}

void ColonalSelectWidget::update()
{
    m_winlist = m_callback->getImageWindowList();
}

void ColonalSelectWidget::selectClonal()
{
    //
    if(!cmFileList.empty())
    {
        if(m_cmList.empty())
        {
            // load clonal masks
            for(int i = 0; i<cmFileList.count(); i++)
            {
                PointClouds pc;

                if(pc.read(cmFileList.at(i).toStdString()))
                {
                    cout << "Error in reading "<< cmFileList.at(i).toStdString().c_str() <<endl;
                    return;
                }

                m_cmList.push_back(pc);
            }
        }
        else
        {
            // check the m_cmList is the same with cmFileList
            // if not, reload clonal masks

        }

        //
        /// selecting
        //

        // subject image
        int curImg = combo_subject->currentIndex();
        Image4DSimple* subject = m_callback->getImage(m_winlist[curImg]);

        QString curImgName = m_callback->getImageName(m_winlist[curImg]);

        QString curImgNamePath=QFileInfo(curImgName).path();
        QString curImgNameBase = QFileInfo(curImgName).baseName();
        QString outImgNameSelected = curImgNamePath + "/" + curImgNameBase + "_selected.v3draw";
        QString outImgNameResidue = curImgNamePath + "/" + curImgNameBase + "_residue.v3draw";

        if (!subject)
        {
            QMessageBox::information(0, "Clonal Selecting", QObject::tr("Invalid image specified."));
            return;
        }

        ImagePixelType datatype_subject = subject->getDatatype();

        if(datatype_subject!=1)
        {
            cout<<"Your datatype is not supported"<<endl;
            return;
        }

        unsigned char* subject1d = subject->getRawData();

        V3DLONG sx = subject->getXDim();
        V3DLONG sy = subject->getYDim();
        V3DLONG sz = subject->getZDim();
        V3DLONG sc = subject->getCDim();

        bool b_select = false;
        unsigned char* pSel = NULL;
        unsigned char* pRes = NULL;

        V3DLONG sc_out = 2; // assuming only computing in the first 2 signals

        V3DLONG pagesz = sx*sy*sz;
        V3DLONG tolpxl = pagesz*sc;

        for(int i = 0; i<listWidget->count(); i++)
        {
            if(listWidget->item(i)->checkState())
            {
                b_select = true;

                if(!pSel)
                {
                    y_new<unsigned char, V3DLONG>(pSel,tolpxl);
                }

                if(!pRes)
                {
                    y_new<unsigned char, V3DLONG>(pRes,tolpxl);
                }

                PointClouds pc;

                pc = m_cmList.at(i);

                double threshold = m_threshold * (double)(pc.pcdheadinfo.maxv);

                for(long k=0; k<pc.points.size(); k++)
                {
                    Point<unsigned short, unsigned short> p = pc.points.at(k);

                    if(p.x<sx && p.y<sy && p.z<sz && p.v>threshold)
                    {
                        V3DLONG idx = p.z*sx*sy + p.y*sx + p.x;

                        for(long c=0; c<sc_out; c++)
                        {
                            pSel[ idx ] = subject1d[ idx ];
                            pSel[ idx + pagesz] = subject1d[ idx + pagesz];
                        }
                    }
                }
            }
        }

        if(!b_select)
        {
            // do nothing
            cout << "None select"<<endl;
        }
        else
        {
            // computing residue
            for(V3DLONG c=0; c<sc_out; c++)
            {
                V3DLONG offsets = c*pagesz;
                for(V3DLONG i=0; i<pagesz; i++)
                {
                    V3DLONG idx = offsets + i;

                    if(!pSel[idx])
                    {
                        pRes[idx] = subject1d[idx];
                    }
                }
            }

            for(V3DLONG c=sc_out; c<sc; c++)
            {
                V3DLONG offsets = c*pagesz;
                for(V3DLONG i=0; i<pagesz; i++)
                {
                    V3DLONG idx = offsets + i;

                    pSel[idx] = pRes[idx] = subject1d[idx];
                }
            }


            // display
            Image4DSimple p4DImgSel;
            p4DImgSel.setData((unsigned char*)pSel, sx, sy, sz, sc, subject->getDatatype()); //

            v3dhandle newwin = m_callback->newImageWindow();
            m_callback->setImage(newwin, &p4DImgSel);
            m_callback->setImageName(newwin, outImgNameSelected);
            m_callback->updateImageWindow(newwin);

            Image4DSimple p4DImgRes;
            p4DImgRes.setData((unsigned char*)pRes, sx, sy, sz, sc, subject->getDatatype()); //

            v3dhandle newwinRes = m_callback->newImageWindow();
            m_callback->setImage(newwinRes, &p4DImgRes);
            m_callback->setImageName(newwinRes, outImgNameResidue);
            m_callback->updateImageWindow(newwinRes);

        }
    }
    else
    {
        cout<<"Do nothing!";
    }

}

void ColonalSelectWidget::getMaskDir()
{
    if(!m_maskfolder.isEmpty() && QDir(m_maskfolder).exists())
    {
        m_maskfolder = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory containing all clonal masks "),
                                                         m_maskfolder,
                                                         QFileDialog::ShowDirsOnly);
    }
    else
    {
        m_maskfolder = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory containing all clonal masks "),
                                                         QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly);
    }
    edit_mask->setText(m_maskfolder);
}

void ColonalSelectWidget::updateDir(const QString &dir)
{
    if(QDir(dir).exists())
    {
        m_maskfolder = dir;
        cmFileList = importSeriesFileList(m_maskfolder, "*.pcd");

        foreach (QString cmFile, cmFileList)
        {
            QString cmName = QFileInfo(cmFile).baseName(); // .pcd
            cmName.chop(4); // _bin

            cmNameList << cmName;
        }

        if(!cmNameList.empty())
        {
            while(listWidget->count()>0)
            {
                listWidget->takeItem(0); // clear
            }

            QStringListIterator it(cmNameList);
            while (it.hasNext())
            {
                QListWidgetItem *listItem = new QListWidgetItem(it.next(),listWidget);
                listItem->setCheckState(Qt::Unchecked);
                listWidget->addItem(listItem);
            }

            QSettings settings("ClonalSelect", "dir");
            settings.setValue("path", m_maskfolder);
        }
    }
}

void ColonalSelectWidget::setThreshold(int threshold)
{
    threshold = slider_threshold->value();
    m_threshold = (double)threshold/100.0;

    label_threshold->setText(QString("Threshold:\t%1").arg(m_threshold));
}

#endif // __CLONALSELECT_GUI_CPP__



