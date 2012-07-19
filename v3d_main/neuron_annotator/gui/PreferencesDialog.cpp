#include "PreferencesDialog.h"
#include <QSettings>
#include <QDebug>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
}

void PreferencesDialog::savePreferences()
{
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    // qDebug() << "Saving preferences";
    settings.setValue("NaMaxVideoMegabytes", getVideoMegabytes());
}

void PreferencesDialog::loadPreferences()
{
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QVariant val = settings.value("NaMaxVideoMegabytes");
    // qDebug() << "Loading preferences";
    if (val.isValid())
        setVideoMegabytes(val.toInt());
}

int PreferencesDialog::getVideoMegabytes() const
{
    return ui.maxVideoMemBox->value();
}

void PreferencesDialog::setVideoMegabytes(int mb)
{
    ui.maxVideoMemBox->setValue(mb);
}

