#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "ui_PreferencesNa.h"
#include <QDialog>

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    int getVideoMegabytes() const;
    void setVideoMegabytes(int mb);

signals:

public slots:
    void savePreferences();
    void loadPreferences();

protected:
    Ui::PreferencesDialog ui;
};

#endif // PREFERENCESDIALOG_H
