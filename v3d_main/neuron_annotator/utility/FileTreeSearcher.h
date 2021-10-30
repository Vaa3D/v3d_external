#ifndef FILETREESEARCHER_H
#define FILETREESEARCHER_H

#include <QString>
#include <QDir>
#include <QRegExp>

class FileTreeSearcher
{
public:
    FileTreeSearcher(const QString & regex);
    FileTreeSearcher(const QRegExp & regex);
    QStringList findFilesInDirectory(QDir directory);

private:
    QRegExp regexObject;
};

#endif // FILETREESEARCHER_H
