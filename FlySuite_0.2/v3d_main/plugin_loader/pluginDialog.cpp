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





// 2009-08-14 Zongcai Ruan: modified for general plugin interface dialog

/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "pluginDialog.h"


PluginDialog::PluginDialog(const QString &appname,
		const QList<QDir> &paths, const QStringList &fileNames,
                           QWidget *parent) :
    QDialog(parent),
    label(new QLabel),
    treeWidget(new QTreeWidget),
    okButton(new QPushButton(tr("OK")))
{
    this->appName = appname;///

    treeWidget->setAlternatingRowColors(false);
    treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    treeWidget->setColumnCount(1);
    treeWidget->header()->hide();

    okButton->setDefault(true);

    connect(okButton, SIGNAL(clicked()), this, SLOT(close()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->addWidget(label, 0, 0, 1, 3);
    mainLayout->addWidget(treeWidget, 1, 0, 1, 3);
    mainLayout->addWidget(okButton, 2, 1);
    setLayout(mainLayout);
    setWindowTitle(tr("Plug-in Information"));
    resize(500,500);

    pluginIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirHomeIcon));
    interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
                            QIcon::Normal, QIcon::On);
    interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
                            QIcon::Normal, QIcon::Off);
    menuIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
    funcIcon.addPixmap(style()->standardPixmap(QStyle::SP_MessageBoxInformation));

    QString labelText = appName; ///appNmae==V3D
    labelText += tr(" found the following plug-ins\n");
    foreach (const QDir& dir, paths) {
        labelText += QString("(%1):\n").arg(QDir::toNativeSeparators(dir.path()));
        visitPlugins(dir.path(), fileNames);
    }
    label->setText(labelText);
}

void PluginDialog::visitPlugins(const QString &path, const QStringList &fileNameList)
{
    const QDir dir(path);

    foreach (QString fileName, fileNameList)
    {
        QPluginLoader loader(dir.absoluteFilePath(fileName));

        QObject *plugin = loader.instance(); // a new instance

        if (plugin)
            populateTreeWidget(plugin, dir.relativeFilePath(fileName));//relativeFilePath

        loader.unload(); // unload this instance
        //qDebug() << "unload: " <<fileName;
    }
}

void PluginDialog::populateTreeWidget(QObject *plugin, const QString &fileName)
{
    QTreeWidgetItem *pluginItem = new QTreeWidgetItem(treeWidget);
//    treeWidget->setItemExpanded(pluginItem, true);  //100804 expanded off
    pluginItem->setText(0, fileName);
    pluginItem->setIcon(0, pluginIcon);
//    QFont f = pluginItem->font(0); f.setBold(true);  pluginItem->setFont(0, f);

    if (plugin)
    {
		addTreeItems(pluginItem, (plugin));
    }
}

void PluginDialog::addTreeItems(QTreeWidgetItem *pluginItem, QObject *plugin)
{
	QString interfaceName = v3d_getInterfaceName(plugin);
    if (interfaceName.size()<=0)
    	return;

    // Add version number, if available
    double version = -100.0;
    V3DSingleImageInterface2_1 *sif21 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
    if (sif21) version = sif21->getPluginVersion();
    V3DPluginInterface2_1 *pif21 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
    if (pif21) version = pif21->getPluginVersion();
    if (version != -100.0) {
        QTreeWidgetItem *versionItem = new QTreeWidgetItem(pluginItem);
        versionItem->setText(0, QString("Plugin version %1").arg(version, 1, 'f', 1));
        // TODO add a line about the version number
    }

    QTreeWidgetItem *interfaceItem = new QTreeWidgetItem(pluginItem);
    interfaceItem->setText(0, interfaceName);
    interfaceItem->setIcon(0, interfaceIcon);
    QFont f = interfaceItem->font(0); f.setItalic(true);  interfaceItem->setFont(0, f);

    QStringList menulist = v3d_getInterfaceMenuList(plugin);
    foreach (QString feature, menulist)
    {
        //if (feature.endsWith("..."))  feature.chop(3);
        QTreeWidgetItem *featureItem = new QTreeWidgetItem(interfaceItem);
        featureItem->setText(0, feature);
        featureItem->setIcon(0, menuIcon);
    }

    QStringList funclist = v3d_getInterfaceFuncList(plugin);
    foreach (QString feature, funclist)
    {
        //if (feature.endsWith("..."))  feature.chop(3);
        QTreeWidgetItem *featureItem = new QTreeWidgetItem(interfaceItem);
        featureItem->setText(0, feature);
        featureItem->setIcon(0, funcIcon);
    }
}
