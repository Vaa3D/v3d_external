/* 
 * File:   DownloadManager.cpp
 * Author: Christopher M. Bruns <brunsc@janelia.hhmi.org>
 * 
 * Created on October 6, 2010, 4:30 PM
 */

// TODO - Use v3d logging system for messages

#include "DownloadManager.h"
#include <QFileInfo>

using namespace std;

DownloadManager::DownloadManager(QWidget *parent)
        : QObject(parent)
{
    progressDialog = new QProgressDialog(parent);
    nam = new QNetworkAccessManager(this);
    // We don't allocate the QNetworkReply
    reply = 0;

    // When the user clicks "Cancel", stop downloading.
    progressDialog->setModal(true);
    progressDialog->setAutoClose(true);
    progressDialog->setMinimumDuration(500);
    connect(progressDialog, SIGNAL(canceled()),
            this, SLOT(cancelDownloadSlot()));

    // When the download is complete, write the file to disk
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(finishedDownloadSlot(QNetworkReply*)));
}

void DownloadManager::getHeader(const QUrl& url)
{
    reply = nam->head(QNetworkRequest(url));
    connect(reply, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(gotHeaderSlot(QNetworkReply*)));
}

void DownloadManager::gotHeaderSlot(QNetworkReply* headerReply) {
    emit gotHeaderSignal(headerReply);
}

void DownloadManager::startDownload(const QUrl &url, QString fileName)
{
    localFileName = fileName;
    reply = nam->get(QNetworkRequest(url));

    progressDialog->setMaximum(0); // zero means "unknown"
    progressDialog->setValue(0);
    QString title = "Downloading file " + url.toString() + " ...";
    progressDialog->setWindowTitle(title);
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(downloadProgressSlot(qint64, qint64)));
    progressDialog->show();
}

void DownloadManager::downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal)
{
    progressDialog->setMaximum(bytesTotal);
    progressDialog->setValue(bytesReceived);
    QString fileText = QFileInfo(reply->url().path()).fileName();
    fileText = "Downloading file" + fileText;
    int percent = 0;
    QString percentText = "Computing time remaining...";
    if (bytesTotal > 0) {
        int percent = 100.0 * bytesReceived / bytesTotal;
        percentText = QString::number(percent) + "% downloaded.";
    }
    progressDialog->setLabelText(fileText + "\n" + percentText);
}

void DownloadManager::cancelDownloadSlot()
{
    progressDialog->hide();

    if (reply) {
        reply->abort();
        reply->disconnect();
        reply->deleteLater();
        reply = 0;
    }

    emit downloadFinishedSignal("");
}

// static
QString DownloadManager::chooseLocalFileName(const QUrl& url)
{
    // TODO - location should be in TEMPDIR, and should be
    // set thus here, not in mainwindow.cpp
    QString fileName = QFileInfo(url.path()).fileName();
    if (fileName.isEmpty())
        fileName = "download";
    if (QFile::exists(fileName)) {
        // already exists, don't overwrite
        QFileInfo fileInfo(fileName); // e.g. "foo.tar.gz"
        QString fileExtension = fileInfo.completeSuffix(); // e.g. "tar.gz"
        QString fileRoot = fileName;
        fileRoot.chop(fileExtension.length()); // e.g. "foo."
        int i = 0;
        fileName = fileRoot + QString::number(i) + "." + fileExtension;
        while (QFile::exists(fileName)) {
            ++i;
            // e.g. foo.1.tar.gz
            fileName = fileRoot + QString::number(i) + "." + fileExtension;
        }
    }
    return fileName;
}

void DownloadManager::finishedDownloadSlot(QNetworkReply* reply)
{
    progressDialog->hide();

    // no error received?
    if (reply->error() == QNetworkReply::NoError)
    {
        QFile localFile(localFileName);
        localFile.open(QIODevice::WriteOnly);
        localFile.write(reply->readAll());
        localFile.close();
    }
    // Some http error received
    else
    {
        // TODO - how to report this...
        fprintf(stderr, "Http error\n");
    }

    // We receive ownership of the reply object
    // and therefore need to handle deletion.
    reply->disconnect();
    reply->deleteLater();

    emit downloadFinishedSignal(localFileName);
}
