/* 
 * File:   DownloadManager.h
 * Author: Christopher M. Bruns <brunsc@janelia.hhmi.org>
 *
 * Created on October 6, 2010, 4:30 PM
 */

#ifndef V3D_DOWNLOADMANAGER_H
#define	V3D_DOWNLOADMANAGER_H

#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QMainWindow>

// Download manager downloads a file from a URL
class DownloadManager : public QObject
{
    Q_OBJECT

public:
    // The parent argument will be used as the GUI parent
    // of message dialogs and the progress dialog.
    DownloadManager(QWidget* parent = 0);
    static QString chooseLocalFileName(const QUrl& url);
    void startDownload(const QUrl &url, QString fileName);

signals:
    void downloadFinishedSignal(QString fileName);

public slots:
    void cancelDownloadSlot();
    void downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal);

protected slots:
    void finishedDownloadSlot(QNetworkReply* reply);

private:
    QProgressDialog *progressDialog;
    QNetworkAccessManager *nam;
    QString localFileName;
    QNetworkReply *reply; // remember to delete
};

#endif	/* V3D_DOWNLOADMANAGER_H */

