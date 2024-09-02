#include "eegdevice.h"


EEGdevice::EEGdevice()
{
    // Get the absolute path of the DLL file
    QString dllPath = QCoreApplication::applicationDirPath() + "/demo.dll";
    LPCWSTR dllPathW = (LPCWSTR)dllPath.utf16(); // Convert QString to LPCWSTR

    // Load the DLL from the specified absolute path
    hinstLib = LoadLibrary(dllPathW);
    if (hinstLib == NULL) {
        qDebug() << "Failed to load DLL";
        return;
    }
}

EEGdevice::~EEGdevice()
{
    // Free the DLL module
    if (hinstLib != NULL) {
        FreeLibrary(hinstLib);
    }
}
QList<double> EEGdevice::getDataFromAmp() {
    // 调用 getData 函数获取数据指针
    double* dataPtr = getData();
    if (!dataPtr) {
        qDebug() << "Failed to get data from amp.";
        return QList<double>(); // 返回一个空的 QList<double>
    }

    // 获取数据大小
    int dataSize = getDataSize();

    QList<double> results;
    for (int i = 0; i < dataSize; i++) {
        // 通过指针偏移访问数据，并将数据添加到结果列表中
        double value = *(dataPtr + i);
        results.append(value);
    }

    return results;
}
bool EEGdevice::InitAmp()
{
     qDebug() << "InitAmp: " << FindAmp;
    if (FindAmp)
    {
        disConnect();
    }

    int result = -1;
    bool success = findAmp(result);
    qDebug() << "result: " << result;
    if (success)
    {
        ch_num = getDataChannel();

        qDebug() << "Number of channels: " << ch_num;

        QList<double> newData=getDataFromAmp();

        qDebug() << "Number of newData empty: " << newData.size();
        FindAmp = true;
    }
    else
    {
        FindAmp = false;
        qDebug() << "Failed to initialize EEG device.";
    }
    return success;
}
bool EEGdevice::findAmp(int& ch_num)
{
    // Get function pointer
    typedef bool (*FindAmpFunc)(int&);
    FindAmpFunc findAmpFunc = (FindAmpFunc)GetProcAddress(hinstLib, "findAmp");

    // Check if function was loaded successfully
    if (!findAmpFunc) {
        qDebug() << "Failed to get function pointer";
        return false;
    }

    // Call the function
    return findAmpFunc(ch_num);
}

double* EEGdevice::getData()
{
    // Get function pointer
    typedef double* (*GetDataFunc)();
    GetDataFunc getDataFunc = (GetDataFunc)GetProcAddress(hinstLib, "getData");

    // Check if function was loaded successfully
    if (!getDataFunc) {
        qDebug() << "Failed to get function pointer";
        return nullptr;
    }

    // Call the function
    return getDataFunc();
}

int EEGdevice::test()
{
    // Get function pointer
    typedef int (*TestFunc)();
    TestFunc testFunc = (TestFunc)GetProcAddress(hinstLib, "test");

    // Check if function was loaded successfully
    if (!testFunc) {
        qDebug() << "Failed to get function pointer";
        return -1;
    }

    // Call the function
    return testFunc();
}
int EEGdevice::getDataSize()
{
    // Get function pointer
    typedef int (*GetDataSizeFunc)();
    GetDataSizeFunc getDataSizeFunc = (GetDataSizeFunc)GetProcAddress(hinstLib, "getDataSize");

    // Check if function was loaded successfully
    if (!getDataSizeFunc) {
        qDebug() << "Failed to get function pointer";
        return -1;
    }

    // Call the function
    return getDataSizeFunc();
}
int EEGdevice::getDataChannel()
{
    // 获取函数指针
    typedef int (*GetChannelCountFunc)();
    GetChannelCountFunc getChannelCountFunc = (GetChannelCountFunc)GetProcAddress(hinstLib, "getChannelCount");
    if (!getChannelCountFunc) {
        qDebug() << "Failed to get function pointer.";
        FreeLibrary(hinstLib);
        return -1;
    }

    return getChannelCountFunc();
}

void EEGdevice::delPtr()
{
    // Get function pointer
    typedef void (*DelPtrFunc)();
    DelPtrFunc delPtrFunc = (DelPtrFunc)GetProcAddress(hinstLib, "delPtr");

    // Check if function was loaded successfully
    if (!delPtrFunc) {
        qDebug() << "Failed to get function pointer";
        return;
    }

    // Call the function
    delPtrFunc();
}

void EEGdevice::disConnect()
{
    if (FindAmp)
    {
        getData(); // 清空数据
        delPtr();
        FindAmp = false;
    }
}
//start data collection
bool EEGdevice::StartRecording(QString _papadigmName) {
    qDebug() << "StartRecording"<<FindAmp;

        if (!FindAmp) {
            qDebug() << "InitAmp";
            if (!InitAmp()) {
                qDebug() << "Failed to initialize the amplifier.";
                return false;
            }
        }
    papadigmName= _papadigmName;
    timer = 60.0;
    sumdata.clear();
    isRecording = true;
    triggerList.clear();
    return true;
}

bool EEGdevice::initdevice() {
    qDebug() << "initdevice"<<FindAmp;

        if (!FindAmp) {
            qDebug() << "InitAmp";
            if (!InitAmp()) {
                qDebug() << "Failed to initialize the amplifier.";
                return false;
            }
        }


    return true;
}
// 触发工具
void EEGdevice::TriggerTool(QString description) {
    if (startTime == 0.0)
        return;
    QTime currentTime = QTime::currentTime();
    float CurTime = (currentTime.hour() * 3600 + currentTime.minute() * 60 + currentTime.second() + currentTime.msec() / 1000.0) - startTime;
    int sample = static_cast<int>(CurTime * 1000);
    qDebug() << "打点";
    qDebug() << sample;
    triggerList.append(sample);
}

// 停止记录数据
void EEGdevice::StopRecording() {
    QList<double> newData = getDataFromAmp();
    sumdata.append(newData);
    //qDebug() << "sumdata：" << sumdata.size();
    qDebug() << "sumdata" << sumdata.size();
    QTime currentTime = QTime::currentTime();
    endTime = (currentTime.hour() * 3600 + currentTime.minute() * 60 + currentTime.second() + currentTime.msec() / 1000.0) - startTime;
   // qDebug() << "ending time：" << endTime;

    isRecording = false;
    disConnect();

    // 构造记录文件路径
    recordFilePath = QCoreApplication::applicationDirPath() + "/record.csv";

    // 写入数据到文件
    WriteToCsv(AdjustData(), recordFilePath);
    curSingleData.clear();
    sumdata.clear();
    triggerList.clear();
    startTime = endTime = 0.0;
    qDebug() << "data saved in" << recordFilePath;
}

double** EEGdevice::AdjustData() {
    qDebug() << "sumdata.size():" << sumdata.size();
    qDebug() << "ch_num:" << ch_num;
    Sample_num = sumdata.size() / ch_num;

    // 创建二维数组，ch_num个通道，每个通道一个数组，最后一个数组存储触发器信息
    double** dataArray = new double*[ch_num + 1];
    for (int i = 0; i < ch_num + 1; ++i) {
        dataArray[i] = new double[Sample_num];
    }

    // 将数据存储到数组中
    for (int i = 0; i < ch_num; ++i) {
        for (int j = 0; j < Sample_num; ++j) {
            dataArray[i][j] = sumdata.at(j * ch_num + i);
        }
    }

    qDebug() << "Data stored in dataArray:";
    for (int i = 0; i < ch_num + 1; ++i) {
        for (int j = 0; j < Sample_num; ++j) {
           // qDebug() << "dataArray[" << i << "][" << j << "]: " << dataArray[i][j];
        }
    }

    // 在最后一个数组中存储触发器信息
    dataArray[ch_num] = new double[triggerList.size()];
    for (int i = 0; i < triggerList.size(); ++i) {
        dataArray[ch_num][i] = 100; // 这里的 100 是示例值，你可以根据需求修改
    }

    qDebug() << "Sample_num:" << Sample_num;
    return dataArray;
}


void EEGdevice::WriteToCsv(double** dataArray, QString filename,QString fullPath) {
    // 确保目录存在
    QDir dir(fullPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();
    QString currentTimeString = currentTime.toString("yyyy-MM-dd-HH-mm-ss");

    // 创建文件并写入数据
    QString filePath = fullPath + "/" + filename+ currentTimeString + ".csv";
    qDebug() << "File path:" << filePath; // 输出文件路径
    qDebug() << "dataArray:" << dataArray[1][1]; // 输出文件路径
        qDebug() << "ch_num:" << ch_num;
        qDebug() << "Sample_num:" << Sample_num;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < ch_num; ++i) {
            for (int j = 0; j < Sample_num; ++j) {
                // 检查dataArray是否为空
                if (dataArray == nullptr) {
                    qDebug() << "dataArray is null!";
                    file.close();
                    return;
                }
                // 检查dataArray[i]是否为空
                if (dataArray[i] == nullptr) {
                    qDebug() << "dataArray[" << i << "] is null!";
                    file.close();
                    return;
                }
                out << dataArray[i][j];
                if (j < Sample_num - 1) {
                    out << ",";
                }
            }
            out << "\n"; // 添加换行符
        }
        file.close();
        qDebug() << "Data successfully written to file.";
    } else {
        qDebug() << "Failed to open file for writing.";
    }
}


