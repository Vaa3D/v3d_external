#include "FileTreeSearcher.h"

#include <QString>
#include <QRegExp>
#include <QDir>
#include <iostream>

using namespace std;

FileTreeSearcher::FileTreeSearcher(const QString & regex)
{
    regexObject.setPattern(regex);
}

FileTreeSearcher::FileTreeSearcher(const QRegExp & regex)
{
    regexObject = regex;
}

QStringList FileTreeSearcher::findFilesInDirectory(QDir directory) {
    QStringList matchList;
    if (!directory.exists()) {
        cerr << "FileTreeSearcher::findFilesInDirectory() error : directory=" << directory.absolutePath().toStdString() << " does not exist" << endl;
        return matchList;
    }
    directory.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QStringList entryList = directory.entryList();
    for (int i=0; i<entryList.size(); ++i) {
        QString filename = entryList.at(i);
        QString fullpath = directory.absolutePath() + "/" + filename;
        if (filename.contains(regexObject)) {
            matchList.append(fullpath);
        } else {
            QFileInfo fileInfo(fullpath);
            if (fileInfo.isDir()) {
                QDir dir(fileInfo.absoluteFilePath());
                matchList.append(findFilesInDirectory(dir));
            }
        }
    }
    return matchList;
}

