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
    /// create a dialog
    //

    // image
    v3dhandleList winlist = callback.getImageWindowList();

    v3dhandle wincurr = callback.currentImageWindow(); // focused image
    QString itemcurr = callback.getImageName(wincurr);
    int idxcurr = 0;

    QStringList items;
    for (int i=0; i<winlist.size(); i++)
    {
        QString item = callback.getImageName(winlist[i]);

        items << item;

        if(item.compare(itemcurr) == 0)
            idxcurr = i;
    }

    combo_subject = new QComboBox(); combo_subject->addItems(items);
    combo_subject->setCurrentIndex(idxcurr);

    label_subject = new QLabel(QObject::tr("Image: "));

    // clonal masks
    label_mask = new QLabel(QObject::tr("Clonal mask directory: "));

    maskfolder = QString();
    QSettings settings("ClonalSelect", "dir");
    maskfolder = settings.value("path").toString();

    if(!maskfolder.isEmpty() && QDir(maskfolder).exists())
    {
        edit_mask = new QLineEdit(maskfolder);
    }
    else
    {
        edit_mask = new QLineEdit(QDir::currentPath());
    }

    pb_browse_mask = new QPushButton("Browse...");


    // list
    label_cmlist = new QLabel(QObject::tr("Clonals: "));

    listWidget = new QListWidget();

    updateDir(maskfolder);


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

    settingGroupLayout->addWidget(label_select, 3,0);
    settingGroupLayout->addWidget(button_select, 3,1,1,1);

    setLayout(settingGroupLayout);
    setWindowTitle(QString("Clonal Selecting"));

    // signal and slot
    connect(pb_browse_mask, SIGNAL(clicked()), this, SLOT(getMaskDir()));
    connect(edit_mask, SIGNAL(textChanged(QString)), this, SLOT(updateDir(QString)) );
    connect(button_select, SIGNAL(clicked()), this, SLOT(update())); //

}

void ColonalSelectWidget::update()
{

    for(int i = 0; i<listWidget->count(); i++)
    {
        if(listWidget->item(i)->checkState())
        {
            qDebug()<<"checked "<<i<<cmNameList.at(i)<<cmFileList.at(i);
        }
    }
}

void ColonalSelectWidget::getMaskDir()
{
    if(!maskfolder.isEmpty() && QDir(maskfolder).exists())
    {
        maskfolder = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory containing all clonal masks "),
                                                       maskfolder,
                                                       QFileDialog::ShowDirsOnly);
    }
    else
    {
        maskfolder = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory containing all clonal masks "),
                                                       QDir::currentPath(),
                                                       QFileDialog::ShowDirsOnly);
    }
    edit_mask->setText(maskfolder);
}

void ColonalSelectWidget::updateDir(const QString &dir)
{
    if(QDir(dir).exists())
    {
        maskfolder = dir;
        cmFileList = importSeriesFileList(maskfolder, "*.pcd");

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
            settings.setValue("path", maskfolder);
        }
    }
}

#endif // __CLONALSELECT_GUI_CPP__



