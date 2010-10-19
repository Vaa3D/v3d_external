/* 
 * File:   DownloadManager.h
 * Author: Christopher M. Bruns <brunsc@janelia.hhmi.org>
 *
 * Created on October 6, 2010, 4:30 PM
 */

#ifndef V3D_DOWNLOADMANAGER_H
#define	V3D_DOWNLOADMANAGER_H

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QString>
#include <QDialog>
class QPushButton;
class QCheckBox;
class QLabel;
class QLineEdit;

#include "ui_dialog_url_entry.h";

// Custom dialog asks user for URL to download
// This is a work in progress, and is not yet used in the V3D GUI
class V3dUrlDialog : public QDialog, public Ui::V3dUrlDialog
{
    Q_OBJECT

public:
    V3dUrlDialog(QWidget *parent = 0);
    QUrl getUrl();
    bool getKeepLocalCache();

private slots:
    void on_lineEdit_textChanged();
};

// Download manager downloads a file from a URL
class DownloadManager : public QObject
{
    Q_OBJECT

public:
    // The parent argument will be used as the GUI parent
    // of message dialogs and the progress dialog.
    DownloadManager(QWidget* parent = 0);
    // static QString chooseLocalFileName(const QUrl& url);
    void startDownloadCheckCache(QUrl url, QString fileName);
    void startDownload(QUrl url, QString fileName);
    QUrl askUserForUrl();
    bool b_cacheFile; // whether to keep local copy of file

signals:
    void downloadFinishedSignal(QUrl url, QString fileName, bool b_cacheFile);

public slots:
    void cancelDownloadSlot();
    void downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal);

protected slots:
    void finishedDownloadSlot(QNetworkReply* reply);
    void gotHeaderSlot(QNetworkReply* headerReply);

private:
    QProgressDialog *progressDialog;
    QString localFileName;
    QNetworkReply *replyFromGet; // remember to delete
    QWidget *guiParent; // parent for message and progress dialogs
};

#endif	/* V3D_DOWNLOADMANAGER_H */

