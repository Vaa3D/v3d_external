#ifndef EEGDEVICE_H
#define EEGDEVICE_H

#include <QCoreApplication>

#include <QDebug>
#include <QtCore>
#include <windows.h>
class EEGdevice
{
public:
    EEGdevice();
    ~EEGdevice();

    bool findAmp(int& ch_num);
    double* getData();
    int test();
    void delPtr();
    void disConnect();
    QList<double> getDataFromAmp();
    bool InitAmp();
    int getDataSize();
    void TriggerTool(QString description);
    void StopRecording();
    bool StartRecording(QString papadigmName);
    double **AdjustData();
    void WriteToCsv(double **dataArray, QString filename="filename", QString fullPath=QCoreApplication::applicationDirPath() + "/record.csv");
    int Sample_num; // 假设你已经定义了 Sample_num 变量
    bool FindAmp=false;
    int ch_num;
    float timer;
    QList<double> sumdata;
    QList<double> curSingleData;
    QList<int> triggerList;
    bool isRecording=false;
    float startTime;
    float endTime;
    QString recordFilePath;
    QString papadigmName;
    int getDataChannel();
    bool initdevice();
private:
    HINSTANCE hinstLib;

};

#endif // EEGDEVICE_H
