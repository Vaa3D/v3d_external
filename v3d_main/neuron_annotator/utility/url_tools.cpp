#include "url_tools.h"
#include <QEventLoop>
#include <QFileInfo>
#include <QDebug>

// Analogous to QDir.filePath(fileName)
QUrl appendPath(const QUrl& parent, const QString& fileName)
{
    QUrl result = parent;
    QString path = result.path();
    if (! path.endsWith("/"))
        path = path + "/";
    path = path + fileName;
    result.setPath(path);
    return result;
}

bool exists(const QUrl& url)
{
    // qDebug() << "exists?" << url << __FILE__ << __LINE__;
    if (! url.isValid())
        return false;
    if (url.isEmpty())
        return false;
    QString localPath = url.toLocalFile();
    if (! localPath.isEmpty()) {
        return QFileInfo(localPath).exists();
    }
    // Access URL to see if it exists...
    QNetworkAccessManager manager;
    // qDebug() << "manager" << __FILE__ << __LINE__;
    QEventLoop loop; // for synchronous url fetch http://stackoverflow.com/questions/5486090/qnetworkreply-wait-for-finished
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)),
            &loop, SLOT(quit()));
    QNetworkRequest request = QNetworkRequest(url);
    // Perhaps "head()" is faster than "get()"?
    QNetworkReply * reply = manager.head(request);
    // QObject::connect(reply, SIGNAL(readyRead()),
    //                  &loop, SLOT(quit()));
    loop.exec();
    bool result = (reply->error() == QNetworkReply::NoError);
    if (result) {
        long fileSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        // KLUDGE - some files of size 25 are corrupt...
        if ((fileSize > 0) && (fileSize < 43) && (url.path().contains(".v3dpbd"))) {
            qDebug() << "WARNING: Ignoring seemingly corrupt file " << url;
            result = false;
        }
    }
    else {
        // qDebug() << "No such url" << url;
    }
    reply->close();
    reply->deleteLater();
    reply = NULL;
    // qDebug() << url.scheme() << url.path() << localPath << result << __FILE__ << __LINE__;
    // qDebug() << "exists == " << result << __FILE__ << __LINE__;
    return result;
}

QUrl parent(const QUrl& child)
{
    QString path = child.path();
    if (path.isEmpty())
        return QUrl();
    int slashPos = path.lastIndexOf("/", -2); // search back from second-to-last character
    if (slashPos < 0)
        return QUrl();
    path.truncate(slashPos);
    QUrl result = child;
    result.setPath(path);
    return result;
}


UrlStream::UrlStream(QUrl url)
    : reply(NULL)
    , networkManager(this)
    , fileSize(0)
{
    QEventLoop loop; // for synchronous url fetch http://stackoverflow.com/questions/5486090/qnetworkreply-wait-for-finished
    QObject::connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            &loop, SLOT(quit()));
    // qDebug() << "networkManager" << __FILE__ << __LINE__;
    QNetworkRequest request = QNetworkRequest(url);
    reply = networkManager.get(request);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        reply->deleteLater();
        reply = NULL;
        return;
    }
    fileSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
}

QIODevice* UrlStream::io()
{
    return reply;
}

/* virtual */
UrlStream::~UrlStream()
{
    if (reply != NULL) {
        reply->close();
        reply->deleteLater();
        reply = NULL;
    }
}

