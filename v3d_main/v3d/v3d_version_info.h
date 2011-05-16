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




//by Hanchuan Peng
//081115: separated from the v3d_core.cpp

#ifndef __V3D_VERSION_INFO_H__
#define __V3D_VERSION_INFO_H__

#include <iomanip>
#include <sstream>
#include <cassert>
#include <QString>
#include <QObject>
#include <QDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QUrl>
#include <QValidator>
#include "ui_dialog_update_v3d.h"
#include "ui_dialog_update_list.h"
#include "ui_dialog_update_options.h"
#include "ui_dialog_update_downloading.h"
#include "ui_dialog_update_checking.h"
#include <limits>

#include "../basic_c_fun/v3d_message.h"

class QNetworkReply;
class QDomDocument;
class QTableWidget;
namespace v3d {
    class DownloadingUpdatesDialog;
}

namespace v3d {
    class VersionInfo;
    // Set current version in file v3d_version_info.cpp
    extern VersionInfo thisVersionOfV3D;
    extern QString versionnumber;
}

void v3d_aboutinfo();
void v3d_Lite_info();


//090908 RZC: build-time info
#define BUILD_TIME		current_time_stamp()
#include <QtGlobal>
#if defined(Q_WS_MAC)
	#define BUILD_OS_INFO	"Mac"
#elif defined(Q_WS_WIN)
	#define BUILD_OS_INFO	"Windows"
#elif defined(Q_WS_X11)
	#define BUILD_OS_INFO	"Linux"
#else
	#define BUILD_OS_INFO	"Unknown OS"
#endif
#define BUILD_QT_INFO 	"Qt "QT_VERSION_STR
#if QT_POINTER_SIZE==4
	#define BUILD_BITS		"32-bit"
#elif QT_POINTER_SIZE==8
	#define BUILD_BITS		"64-bit"
#elif QT_POINTER_SIZE==16
	#define BUILD_BITS		"128-bit"
#endif

namespace v3d {

class VersionInfo {
protected:
    void loadDataFromCString(const char* str) {
        std::istringstream ss(str);
        ss >> v3d_major_version;
        char dot;
        ss >> dot;
     //   assert(dot == '.'); // please to fix: always be failed assertion `dot == '.''
        ss >> v3d_minor_version;
        std::string letter;
        ss >> letter;
        specialLetter = QString(letter.c_str());
        loadMetaData();
    }
    void loadMetaData() {
        osPlatform = BUILD_OS_INFO;
        buildTime = BUILD_TIME;
    }

public:
    // Initialize from (2, 532, "a")
    VersionInfo(int v3d_major_version_param, int v3d_minor_version_param, const char* specialLetter_param = "")
        :       v3d_major_version(v3d_major_version_param),
                v3d_minor_version(v3d_minor_version_param),
                specialLetter(specialLetter_param)
    {
        osPlatform = BUILD_OS_INFO;
        buildTime = BUILD_TIME;
    }
    // Initialize from (2.532, "a")
    VersionInfo(double f, const char* specialLetter = "") {
        v3d_major_version = int(f);
        v3d_minor_version = int((f - v3d_major_version)*1000.01); // shift 3 digits to the left
        this->specialLetter = specialLetter;
        osPlatform = BUILD_OS_INFO;
        buildTime = BUILD_TIME;
    }
    // Initialize from ("2.532a")
    VersionInfo(const char* str) {
        loadDataFromCString(str);
    }
    VersionInfo(const QString& str) {
        loadDataFromCString(qPrintable(str));
    }

    QString toQString() const {
        std::ostringstream oss;
        this->print(oss);
        return QString(oss.str().c_str());
    }

    float toFloat() const {
        return v3d_major_version + 0.001 * v3d_minor_version;
    }

    bool operator!=(const VersionInfo& rhs) const {
        if (v3d_major_version != rhs.v3d_major_version) return true;
        if (v3d_minor_version != rhs.v3d_minor_version) return true;
        if (specialLetter != rhs.specialLetter) return true;
        return false;
    }

    bool operator==(const VersionInfo& rhs) const {
        return !(*this != rhs);
    }

    bool operator<(const VersionInfo& rhs) const {
        if (v3d_major_version < rhs.v3d_major_version) return true;
        if (v3d_major_version > rhs.v3d_major_version) return false;
        if (v3d_minor_version < rhs.v3d_minor_version) return true;
        if (v3d_minor_version > rhs.v3d_minor_version) return false;
        return false;
    }

    bool operator>(const VersionInfo& rhs) const {
        if (v3d_major_version > rhs.v3d_major_version) return true;
        if (v3d_major_version < rhs.v3d_major_version) return false;
        if (v3d_minor_version > rhs.v3d_minor_version) return true;
        if (v3d_minor_version < rhs.v3d_minor_version) return false;
        return false;
    }

    int v3d_major_version; // e.g. 2
    int v3d_minor_version; // always prints 3 digits, using leading zeros if needed
    QString specialLetter; // optional
    QString osPlatform;
    QString buildTime;

    std::ostream& print(std::ostream& os) const
    {
        os << v3d_major_version;
        os << ".";
        // Always print v3d_minor_version version with 3 digits, zero padded
        os << std::setw(3) << std::setfill('0') << v3d_minor_version;
        os << qPrintable(specialLetter);
        return os;
    }
};

extern VersionInfo thisVersionOfV3D;

// UpdateItem class is for storage of potentially updatable software component
// information within the V3DVersionChecker class.
class UpdateItem : public QObject
{
    Q_OBJECT

public:
    UpdateItem(QObject* parent)
            : QObject(parent),
              localVersion(std::numeric_limits<float>::quiet_NaN()),
              remoteVersion(std::numeric_limits<float>::quiet_NaN()),
              bDoInstall(false)
    {}

    QString relativeName; // e.g. "plugins/5dstackconverter/lib5dstackconverter.dylib"
    //
    QFileInfo localFile; // e.g. /Applications/v3d.app/Contents/MacOS/plugins/5dstackconverter/lib5dstackconverter.dylib
    float localVersion; // e.g. 1.1; or NaN, if undefined
    //
    QUrl remoteUrl; // e.g. http://penglab.janelia.org/proj/v3d/stable_version/mac/plugins/5dstackconverter/lib5dstackconverter.dylib
    float remoteVersion; // e.g. 1.2; or NaN, if undefined

    bool bDoInstall;

signals:
    void updateComplete(v3d::DownloadingUpdatesDialog*, bool succeeded);

public slots:
    void setInstall( int state );
    void startUpdate(v3d::DownloadingUpdatesDialog*);

private slots:
    void finishedDownloadSlot(QNetworkReply*);

private:
    v3d::DownloadingUpdatesDialog *progressDialog;
};

class CheckingForUpdatesDialog;

class V3DVersionChecker : public QObject
{
    Q_OBJECT

public:
    V3DVersionChecker(QWidget *guiParent);
    void checkForLatestVersion(bool b_verbose = false);
    bool shouldCheckNow();
    void populateQTableWidget(QTableWidget& tableWidget);

    static QString getPlatformString();
    static QString getDefaultV3DVersionXmlFileName();
    static QString getDefaultV3DVersionUrl();
    static void never_update();

public slots:
    void createVersionXml();

private slots:
    void gotVersion(QNetworkReply* reply);
    void show_update_list();
    void install_updates();
    void finishUpdates(v3d::DownloadingUpdatesDialog* progressDialog);

protected:
    bool userSelectedNeverUpdateInAnOlderVersionOfV3D();
    void createVersionXml(QString xmlFile);
    void processVersionXmlFile(const QDomDocument& versionDoc);
    // cache versions of local v3d and plugins, prior to checking update site.
    void populateLocalUpdateItems();
    void populateLocalPluginsDirs(const QDir& pluginsDir);
    void populateLocalPluginsFiles(const QDir& pluginsDir);
    std::vector<UpdateItem*> getUpdatableItems();
    std::vector<UpdateItem*> getFutureItems();

private:
    QWidget *guiParent;
    bool b_showAllMessages;
    typedef std::map<QString, UpdateItem*> UpdateItemsType;
    UpdateItemsType updateItems;
    bool bDownloadCanceled;
    QUrl xmlPathUrl; // location of update information xml file
    CheckingForUpdatesDialog *checkingDialog;
};

class VersionUrlValidator : public QValidator
{
    Q_OBJECT

public:
    VersionUrlValidator(QObject* parent) : QValidator(parent) {}
    
    State validate(QString& input, int& pos) const
    {
        return validate(input);
    }

    State validate(const QString& input) const
    {
        if (input.isEmpty()) return QValidator::Intermediate;
        QUrl url(input);
        if (url.isEmpty()) return QValidator::Intermediate;
        if (! url.isValid()) return QValidator::Intermediate;
        if (url.isRelative()) return QValidator::Intermediate;
        return QValidator::Acceptable;
    }
};


class CheckForUpdatesDialog : public QDialog, public Ui::dialog_update_v3d
{
    Q_OBJECT
public:
    CheckForUpdatesDialog(QWidget* guiParent);

signals:
    void yes_update();

private slots:
    void check_now();
    void show_options();
};

class UpdatesListDialog : public QDialog, public Ui::dialog_update_list
{
    Q_OBJECT
public:
    UpdatesListDialog(QWidget* guiParent, V3DVersionChecker *checker);

signals:
    void update_install();

private:
    V3DVersionChecker *versionChecker;
};

class UpdateOptionsDialog : public QDialog, public Ui::dialog_update_options
{
    Q_OBJECT

public:
    UpdateOptionsDialog(QWidget* guiParent);

private slots:
    void save_xml_file(); // create a local V3D version xml file
    void on_comboBox_currentIndexChanged(const QString& updateFrequency); // change update check frequency
    void on_lineEdit_editingFinished(); // user completes version xml url
    void on_lineEdit_textChanged(); // user changes version xml url
    void use_default_version_file();

private:
    VersionUrlValidator* validator;
    QPalette* redPalette;
    QPalette* blackPalette;
};


class CheckingForUpdatesDialog : public QDialog, public Ui::checking_for_updates_dialog
{
    Q_OBJECT
public:
    CheckingForUpdatesDialog(QWidget *parent);
    bool& wasCanceled() {return bWasCanceled;}

private slots:
    void canceled() {bWasCanceled = true;}

private:
    bool bWasCanceled;
};


class UpdatesAvailableDialog : public QMessageBox
{
    Q_OBJECT
public:
    UpdatesAvailableDialog(QWidget *parent);
    QPushButton* yesButton;

signals:
    void yes_update();

private slots:
    void never_update();
    void remind_me_later();
};


class DownloadingUpdatesDialog : public QDialog, public Ui::downloading_updates_dialog
{
    Q_OBJECT
public:
    DownloadingUpdatesDialog(QWidget *parent);
    bool wasCanceled() {return bWasCanceled;}

private slots:
    void canceled() {bWasCanceled = true;}

private:
    bool bWasCanceled;
};


} // namespace v3d


// inline to avoid multiply defined error
inline std::ostream& operator<<(std::ostream& os, const v3d::VersionInfo& v) {
    return v.print(os);
}

#endif
