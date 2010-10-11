/* 
 * File:   DownloadManager.cpp
 * Author: Christopher M. Bruns <brunsc@janelia.hhmi.org>
 * 
 * Created on October 6, 2010, 4:30 PM
 */

// TODO - Use v3d logging system for messages

#include "DownloadManager.h"
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressDialog>
#include <QInputDialog>
#include <QLineEdit>

using namespace std;

void DownloadManager::startDownload(const QUrl &url, QString fileName)
{
    localFileName = fileName;
    reply = nam->get(QNetworkRequest(url));

    progressDialog->setMaximum(0); // zero means "unknown"
    progressDialog->setValue(0);
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(downloadProgressSlot(qint64, qint64)));
    progressDialog->show();
}

void DownloadManager::downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal)
{
    progressDialog->setMaximum(bytesTotal);
    progressDialog->setValue(bytesReceived);
}

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

    // message.setText("Finished downloading");
    // message.exec(); // exec method make it modal

    // Reading attributes of the reply
    // e.g. the HTTP status code
    QVariant statusCodeV =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    // Or the target URL if it was a redirect:
    QVariant redirectionTargetUrl =
        reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    // see CS001432 on how to handle this

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
        fprintf(stderr, "Http error\n");
    }

    // We receive ownership of the reply object
    // and therefore need to handle deletion.
    reply->disconnect();
    reply->deleteLater();

    emit downloadFinishedSignal(localFileName);
}
