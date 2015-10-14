#include "PreferencesDialog.h"
#include <QSettings>
#include <QDebug>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
    bool showFastLoad = false;
#ifdef USE_FFMPEG
    showFastLoad = true;
#endif
    ui.fastLoadWidget->setVisible(showFastLoad);
}

void PreferencesDialog::savePreferences()
{
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    // qDebug() << "Saving preferences";
    settings.setValue("NaMaxVideoMegabytes", getVideoMegabytes());
    // qDebug() << "setting NaBUseFastLoad3D to" << ui.fastLoadCheckBox->isChecked();
    settings.setValue("NaBUseFastLoad3D", ui.fastLoadCheckBox->isChecked());
}

void PreferencesDialog::loadPreferences()
{
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QVariant val = settings.value("NaMaxVideoMegabytes");
    // qDebug() << "Loading preferences";
    if (val.isValid())
        setVideoMegabytes(val.toInt());
    val = settings.value("NaBUseFastLoad3D");
    if (val.isValid())
        ui.fastLoadCheckBox->setChecked(val.toBool());
}

int PreferencesDialog::getVideoMegabytes() const
{
    return ui.maxVideoMemBox->value();
}

void PreferencesDialog::setVideoMegabytes(int mb)
{
    ui.maxVideoMemBox->setValue(mb);
}

