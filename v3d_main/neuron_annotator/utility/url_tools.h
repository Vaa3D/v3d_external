/*
 * url_tools.h
 *
 *  Created on: Jan 15, 2013
 *      Author: Christopher M. Bruns
 */

#ifndef URL_TOOLS_H_
#define URL_TOOLS_H_

#include <QUrl>
#include <QString>
#include <QObject>
#include <QIODevice>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Analogous to QDir.filePath(fileName)
QUrl appendPath(const QUrl& parent, const QString& fileName);
bool exists(const QUrl& url);
QUrl parent(const QUrl& child);


// Stack allocated wrapper for reading from files or URLs
class UrlStream : QObject
{
public:
    UrlStream(QUrl url);
    QIODevice* io();
    virtual ~UrlStream();

protected:
    QNetworkReply* reply;
    QNetworkAccessManager networkManager;
    long fileSize;
};

#endif /* URL_TOOLS_H_ */
