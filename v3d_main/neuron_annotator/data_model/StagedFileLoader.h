#ifndef STAGEDFILELOADER_H
#define STAGEDFILELOADER_H

#include <QObject>
#include <QString>

class ProgressiveLoadItem : public QObject
{
    Q_OBJECT

signals:
    void completed(bool succeeded);

public slots:
    virtual void load() {}
    virtual void cancel() {}
};

class ProgressiveLoadVolume : public ProgressiveLoadItem
{
public:
    QString fileName;
};

class StagedFileLoader
{
public:
    StagedFileLoader();
};

#endif // STAGEDFILELOADER_H
