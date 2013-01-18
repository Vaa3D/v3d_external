/* clonalselect_gui.h
 * 2013-01-13: create this program by Yang Yu
 */


#ifndef __CLONALSELECT_GUI_H__
#define __CLONALSELECT_GUI_H__

//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <v3d_interface.h>

//
QStringList importSeriesFileList(const QString & curFilePath, char* suffix);

// clonal selecting class
class ColonalSelectWidget : public QWidget
{
    Q_OBJECT

public:
    ColonalSelectWidget(V3DPluginCallback &callback, QWidget *parentWidget);

signals:
    void textChanged(const QString&);

public slots:
    void update();

private slots:
    void updateDir(const QString &dir);
    void getMaskDir();

public:
    QString maskfolder;

    QLabel* label_mask;
    QLineEdit *edit_mask;
    QPushButton *pb_browse_mask;

    QLabel *label_subject;
    QComboBox *combo_subject;

    QLabel *label_cmlist;
    QListWidget *listWidget;

    QScrollArea * scrollAreaEdit;
    QScrollArea * scrollAreaMask;

    QLabel *label_select;
    QPushButton *button_select;

    QGridLayout *settingGroupLayout;

    //
    QStringList cmFileList;
    QStringList cmNameList;

};

#endif // __CLONALSELECT_GUI_H__



