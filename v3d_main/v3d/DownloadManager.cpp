/* 
 * File:   DownloadManager.cpp
 * Author: Christopher M. Bruns <brunsc@janelia.hhmi.org>
 * 
 * Created on October 6, 2010, 4:30 PM
 */

// TODO - Use v3d logging system for messages

#include "DownloadManager.h"
#include <QFileInfo>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QCheckBox>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRegExpValidator>

using namespace std;

V3dUrlDialog::V3dUrlDialog(QWidget *parent) 
    : QDialog(parent)
{
    setupUi(this);
    QRegExp urlRegExp("https?://\\S+");
    lineEdit->setValidator(new QRegExpValidator(urlRegExp, this));
    // connect(openButton, SIGNAL(clicked()), this, SLOT(accept()));
    // connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void V3dUrlDialog::on_lineEdit_textChanged() {
    QPushButton *openButton = buttonBox->button(QDialogButtonBox::Open);
    openButton->setEnabled(lineEdit->hasAcceptableInput());
}

QUrl V3dUrlDialog::getUrl()
{
    QUrl url(lineEdit->text());
    return url;
}

bool V3dUrlDialog::getKeepLocalCache() {
    return checkBox->isChecked();
}

DownloadManager::DownloadManager(QWidget *parent)
        : QObject(parent),
          guiParent(parent),
          b_cacheFile(true),
          b_forceopen3dviewer(false)
{
    progressDialog = new QProgressDialog(parent);
    // We don't allocate the QNetworkReply
    replyFromGet = 0;

    // When the user clicks "Cancel", stop downloading.
    progressDialog->setModal(true);
    progressDialog->setAutoClose(true);
    progressDialog->setMinimumDuration(500);
    connect(progressDialog, SIGNAL(canceled()),
            this, SLOT(cancelDownloadSlot()));
}

QUrl DownloadManager::askUserForUrl()
{
    QUrl url;
    V3dUrlDialog *urlDialog = new V3dUrlDialog(guiParent);
    int returnValue = urlDialog->exec();
    if (returnValue == QDialog::Accepted) {
        url = urlDialog->getUrl();
        b_cacheFile = urlDialog->checkBox->isChecked();
    }
    else {
        // fprintf(stderr, "URL dialog rejected\n");
    }
    return url;
}

void DownloadManager::startDownloadCheckCache(
        QUrl url, QString fileName)
{
    localFileName = fileName;
    QNetworkAccessManager *headerNam = new QNetworkAccessManager(this);
    QNetworkReply *headerReply = headerNam->head(QNetworkRequest(url));
    connect(headerNam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(gotHeaderSlot(QNetworkReply*)));
}

// Examine header to see whether local cached file can be used
void DownloadManager::gotHeaderSlot(QNetworkReply* headerReply)
{
    QFile localFile(localFileName);
    // Perhaps the file was removed since we last checked
    if (localFile.exists() && (localFile.size() > 0))
    {
        // Check file size
        qint64 localFileSize = localFile.size();
        // Check file date
        QDateTime localFileTime = QFileInfo(localFile).lastModified();

        // Need to compare to existing cached file.
        if (headerReply->error() == QNetworkReply::NoError) {
            // At what date/time was the remote file last modified?
            QVariant lmod = headerReply->header(QNetworkRequest::LastModifiedHeader);
            if (lmod.isValid()) {
                QDateTime remoteFileTime = lmod.toDateTime();
                // Compare time
                if (remoteFileTime < localFileTime) {
                    QVariant hsiz = headerReply->header(QNetworkRequest::ContentLengthHeader);
                    if (hsiz.isValid()) {
                        qint64 remoteFileSize = hsiz.toLongLong();
                        if ( (remoteFileSize == localFileSize)
                                && (remoteFileTime < localFileTime) ) 
                        {
                            // In this one case we can use the cached file.
                            emit downloadFinishedSignal(headerReply->url(),
                                    localFileName, b_cacheFile, b_forceopen3dviewer);
                            return;
                        }
                    }
                }
            }
        }
    }

    // If cached file does not match, need to warn about overwriting.
    if (localFile.exists()) {
        QMessageBox msgBox(guiParent);
        QString msg = "Warning: File\n "
                + localFileName + "\nalready exists.";
        msgBox.setText(msg);
        msgBox.setInformativeText(tr("Overwrite existing file?"));
        msgBox.setStandardButtons(QMessageBox::Cancel);
        msgBox.setIcon(QMessageBox::Warning);
        QPushButton *overwriteButton = msgBox.addButton(tr("Overwrite"),
                QMessageBox::DestructiveRole);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.exec();
        if (msgBox.clickedButton() != (QAbstractButton*)overwriteButton)
            return; // Do nothing unless user clicks "Overwrite"
    }

    // If we get this far, don't use cache file, download it.
    startDownload(headerReply->url(), localFileName);
}

void DownloadManager::startDownload(QUrl url, QString fileName)
{
    localFileName = fileName;
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    // When the download is complete, write the file to disk
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(finishedDownloadSlot(QNetworkReply*)));
    replyFromGet = nam->get(QNetworkRequest(url));
    connect(replyFromGet, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(downloadProgressSlot(qint64, qint64)));

    progressDialog->setMaximum(0); // zero means "unknown"
    progressDialog->setValue(0);
    QString title = tr("Downloading file") + " " + url.toString() + " ...";
    progressDialog->setWindowTitle(title);
    progressDialog->exec();
}

void DownloadManager::downloadProgressSlot(qint64 bytesReceived, qint64 bytesTotal)
{
    progressDialog->setMaximum(bytesTotal);
    progressDialog->setValue(bytesReceived);
    QString fileText = tr("Downloading file") + " " + localFileName;
    int percent = 0;
    QString percentText = tr("Computing time remaining...");
    if (bytesTotal > 0) {
        int percent = 100.0 * bytesReceived / bytesTotal;
        percentText = QString::number(percent) + "% " + tr("downloaded");
    }
    progressDialog->setLabelText(fileText + "\n" + percentText);
}

void DownloadManager::cancelDownloadSlot()
{
    progressDialog->hide();

    if (replyFromGet) {
        replyFromGet->abort();
        replyFromGet->disconnect();
        replyFromGet = 0;
    }
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
        emit downloadFinishedSignal(reply->url(), localFileName, b_cacheFile, b_forceopen3dviewer);
    }
    // Some http error received
    else
    {
        // TODO - how to report this...
        fprintf(stderr, "Http error\n");
        emit downloadFinishedSignal(reply->url(), "", false, false);
    }

    // We receive ownership of the reply object
    // and therefore need to handle deletion.
    reply->disconnect();
    reply->deleteLater();

}
