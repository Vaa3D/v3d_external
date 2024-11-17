#include "v3dr_gl_vr.h"
#include "VR_MainWindow.h"


#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>
#include <math.h>
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <qjson/serializer.h>
#include <qjson/parser.h>
#include <qjson/qobjecthelper.h>



//extern std::vector<Agent> Agents;
//std::vector<Agent> Agents;
VR_MainWindow::VR_MainWindow(V3dR_Communicator * TeraflyCommunicator) :
    QWidget()
{
    userId="";
    VR_Communicator = TeraflyCommunicator;


    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), this, SLOT(TVProcess(QString)));
    disconnect(VR_Communicator, SIGNAL(msgtowarn(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtowarn(QString)), this, SLOT(processWarnMsg(QString)));
    disconnect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), this, SLOT(processAnalyzeMsg(QString)));
    //    connect(this,SIGNAL(sendPoolHead()),this,SLOT(onReadySendSeg()));

    disconnect(VR_Communicator,SIGNAL(updateBCIstate(QString)), 0, 0);
    connect(VR_Communicator,SIGNAL(updateBCIstate(QString)),this,SLOT(updateBCIstate(QString)));


    userId = TeraflyCommunicator->userId;
    qDebug()<<"userId "<<userId<<" "<<VR_Communicator->userId;
    CURRENT_DATA_IS_SENT=false;
    transferTimer = new QTimer(this);
    connect(transferTimer, SIGNAL(timeout()), this, SLOT(performFileTransfer()));


    QString recordDirPath = QCoreApplication::applicationDirPath() + "/record.csv";

    // 检查目录是否存在，如果不存在则创建
    QDir directory(recordDirPath);
    if (!directory.exists()) {
        if (!QDir().mkdir(recordDirPath)) {
            qDebug() << "Failed to create directory:" << recordDirPath;
            return;
        }
    }
    // 设置要过滤的文件类型
    QStringList files = directory.entryList(QDir::Files | QDir::NoDotAndDotDot);

    previousFileCount = files.length();

    qDebug()<<"VR_MainWindowVR_MainWindowVR_MainWindow";
}

VR_MainWindow::~VR_MainWindow() {
    disconnect(VR_Communicator, SIGNAL(msgtoprocess(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtoprocess(QString)), VR_Communicator, SLOT(TFProcess(QString)));
    disconnect(VR_Communicator, SIGNAL(msgtowarn(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtowarn(QString)), this, SLOT(processWarnMsg(QString)));
    disconnect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), 0, 0);
    connect(VR_Communicator, SIGNAL(msgtoanalyze(QString)), this, SLOT(processAnalyzeMsg(QString)));
    if (transferTimer->isActive()) {
        transferTimer->stop();
    }
    delete transferTimer;
}
//void VR_MainWindow::sendDataToServer(const QByteArray &jsonData) {
//    // Prepare HTTP request
//    QUrl url("http://127.0.0.1:8000/data_analysis/analysisR/");
//    QNetworkRequest request(url);
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

//    QNetworkAccessManager manager;
//    QNetworkReply *reply = manager.post(request, jsonData);
//    QEventLoop eventLoop;
//    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
//    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
//    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//       qDebug()<<"getAllProject: "<<code;
//       if (code == 200) {
//           QByteArray json = reply->readAll();
//           qDebug() << "Response:" << json;

//           QJson::Parser parser;
//           bool ok;
//           QVariantMap result = parser.parse(json, &ok).toMap();

//           if (!ok) {
//               qDebug() << "Failed to parse JSON";
//               return;
//           }

//           bool status = result["status"].toBool();
//           QString message = result["message"].toString();
//           QString resultString = result["result_string"].toString();
//           double resultDouble = result["result_double"].toDouble();

//           if (!status) {
//               QString msg = "Operation Failed! " + message;
//               qDebug() << msg;
//               QMessageBox::information(0, "Message", msg, QMessageBox::Ok);
//               return;
//           }

//           qDebug() << "Status:" << status;
//           qDebug() << "Message:" << message;
//           qDebug() << "Result String:" << resultString;
//           qDebug() << "Result Double:" << resultDouble;

//       } else {
//           QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
//           QMessageBox::information(0, "Message", reason, QMessageBox::Ok);
//       }

//       reply->deleteLater();
//}
void VR_MainWindow::sendDataToServer(const QByteArray &jsonData) {
    // Prepare HTTP request
    QUrl url("http://127.0.0.1:8000/data_analysis/analysisR/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request, jsonData);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "getAllProject: " << code;

    if (code == 200) {
        QByteArray json = reply->readAll();
        qDebug() << "Response:" << json;

        QJson::Parser parser;
        bool ok;
        QVariantMap result = parser.parse(json, &ok).toMap();

        if (!ok) {
            qDebug() << "Failed to parse JSON";
            return;
        }

        bool status = result["status"].toBool();
        QString message = result["message"].toString();
        QString resultString = result["result_string"].toString();
        double resultDouble = result["result_double"].toDouble();

        if (!status) {
            QString msg = "Operation Failed! " + message;
            qDebug() << msg;
            QMessageBox::information(0, "Message", msg, QMessageBox::Ok);
            return;
        }

        // Print the result values
        qDebug() << "Status:" << status;
        qDebug() << "Message:" << message;
        qDebug() << "Result String:" << resultString;
        qDebug() << "Result Double:" << resultDouble;

        // 处理接收到的图像数据 (Base64)
        QString base64Image = result["image"].toString(); // 读取图像的Base64编码
        if (!base64Image.isEmpty()) {
            // Decode the Base64 string to QByteArray
//            QByteArray imageData = QByteArray::fromBase64(base64Image.toUtf8());


            // Get the path to save the image (same directory as the .exe file)
            QString exePath = QCoreApplication::applicationDirPath();
            QString imagePath = exePath + "/received_image.jpg"; // You can modify the image file name as needed
            // 读取图片文件
            QFile imageFile(imagePath);
            if (!imageFile.exists()) {
                qWarning() << "Image file does not exist at" << imagePath;
                return;
            }

            if (!imageFile.open(QIODevice::ReadOnly)) {
                qWarning() << "Failed to open image file:" << imagePath;
                return;
            }

            QByteArray imageData = imageFile.readAll();
            imageFile.close();
            pMainApplication->onNewVolumeDataReceived(imageData);
            qDebug() << "onNewVolumeDataReceived";

//            // Save the image to the path
//            QFile file(imagePath);
//            if (file.open(QIODevice::WriteOnly)) {
//                file.write(imageData);
//                file.close();
//                // 发射信号，传递图像数据

//                qDebug() << "Image saved to:" << imagePath;
//            } else {
//                qDebug() << "Failed to save the image.";
//            }
        }

    } else {
        QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QMessageBox::information(0, "Message", reason, QMessageBox::Ok);
    }

    reply->deleteLater();
}

void VR_MainWindow::onReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Request successful";
            qDebug() << "Response:" << reply->readAll();
        } else {
            qDebug() << "Request failed:" << reply->errorString();
        }
        reply->deleteLater();

    }
}
void VR_MainWindow::sendRealDataAndGetResult(const QList<double>& curSingle, const QString& dataName) {
    qDebug() << "sendRealData";

    // Validate the input parameters
    if (curSingle.isEmpty()) {
        qWarning() << "Error: The data list cannot be empty.";
        return;
    }

    if (dataName.isEmpty()) {
        qWarning() << "Error: Data name cannot be empty.";
        return;
    }

    // Convert the data to JSON using the original method
    QJson::Serializer serializer;
    QVariantMap jsonMap;
    jsonMap["data_name"] = dataName;

    // Convert QList<double> to QVariantList
    QVariantList dataArray;
    for (double value : curSingle) {
        dataArray.append(value);
    }
    jsonMap["data_value"] = dataArray;

    // Serialize the QVariantMap to JSON
    QByteArray jsonData = serializer.serialize(jsonMap);

    // Send JSON data to server
    sendDataToServer(jsonData);
}
void VR_MainWindow::generateAndSendData() {
    const int numChannels = 32;
    const int numSamples = 1000;
    const double sfreq = 1000.0;  // Sampling frequency in Hz
    const double freq = 10.0;     // Frequency of the sinusoidal component in Hz
    const double amplitude = 1.0; // Amplitude of the sinusoidal component
    const double noiseLevel = 0.5; // Standard deviation of the Gaussian noise
    qDebug()<<"generateAndSendData";
    // Generate mock data (10x10 double array)
    std::vector<std::vector<double>> data(numChannels, std::vector<double>(numSamples));
    double t, signal, noise;

    std::srand(static_cast<unsigned>(std::time(0)));

    for (int i = 0; i < numChannels; ++i) {
        for (int j = 0; j < numSamples; ++j) {
            t = static_cast<double>(j) / sfreq;
            signal = amplitude * std::sin(2 * M_PI * freq * t);
            noise = noiseLevel * (static_cast<double>(std::rand()) / RAND_MAX - 0.5);
            data[i][j] = signal + noise;
        }
    }

    // Convert to JSON string using QJson::Serializer
    QJson::Serializer serializer;
    QVariantMap jsonMap;
    jsonMap["data_name"] = "data_matrix"; // Example data name




    QVariantList dataArray;
    for (int i = 0; i < numChannels; ++i) {
        QVariantList rowArray;
        for (int j = 0; j < numSamples; ++j) {
            rowArray.append(data[i][j]);
        }
        dataArray.append(rowArray);
    }
    jsonMap["data_value"] = dataArray;

    QByteArray jsonData = serializer.serialize(jsonMap);

    // Send JSON data to server
    sendDataToServer(jsonData);
}

void VR_MainWindow::performFileTransfer() {

    QString recordDirPath = QCoreApplication::applicationDirPath() + "/record.csv";
    qDebug() << "performFileTransfer.";
    // 检查目录是否存在，如果不存在则创建
    QDir directory(recordDirPath);
    if (!directory.exists()) {
        if (!QDir().mkdir(recordDirPath)) {
            qDebug() << "Failed to create directory:" << recordDirPath;
            return;
        }
    }
    // 设置要过滤的文件类型
    QStringList files = directory.entryList(QDir::Files | QDir::NoDotAndDotDot);
//    generateAndSendData();
    int newFileCount = files.length() - previousFileCount;

    // 如果有新文件
    if (newFileCount > 0) {
        QStringList newFiles;
        QStringList newFiles_name;
        for (const QString &file : files) {
            if (!previousFiles.contains(file)) {
                QString filePath = directory.filePath(file);
                newFiles.append(filePath);
                newFiles_name.append(file);

                qDebug() << "Detected new file:" << filePath;
                qDebug() << "Detected new file:" << file;
            }
        }
        // 发送新文件
        if (VR_Communicator) {
            VR_Communicator->sendfiles(newFiles, newFiles_name);
        } else {
            qDebug() << "VR_Communicator is null.";
        }

        // 更新上一次检查时的文件列表和文件数量
        previousFiles = files;
        previousFileCount = files.length();
    }
    if(pMainApplication->eegDevice.isRecording)
    {
        QList<double> sumData = pMainApplication->getSumData();
        QList<double> CurSingle = pMainApplication->getCurSingleData();
        if (!CurSingle.isEmpty() && CurSingle.size() > 1) {
            qDebug() << "RunVRMainloop";
            qDebug() << "Latest data in CurSingle:" << CurSingle.last();
            qDebug() << CurSingle.length();
             qDebug() << sumData.length();

            if (VR_Communicator && VR_Communicator->socket && VR_Communicator->socket->state() == QAbstractSocket::ConnectedState) {
                if (CurSingle.size() >= 32*1000) {

                    QString currentDateTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                    QString eegdata_paradigm = pMainApplication->getModeControlSettingsDescription(pMainApplication->m_modeGrip_L);;
                    QString data_label = "eegdata_paradigm_" + VR_Communicator->userName + "_" + currentDateTime;
//                    QString data_label = eegdata_paradigm+"_" + VR_Communicator->userName + "_" + currentDateTime;
                    //VR_Communicator->send2DArrayBinary(CurSingle,data_label);
                    // Optionally handle the result
                    if (pMainApplication->isClosedLoop) {

                    }
                    qDebug() << "Data sent successfully and received result: " ;
                    sendRealDataAndGetResult(CurSingle, data_label);
                    // Send the data using the provided function



                } else {
                    qDebug() << "CurSingle size is out of expected range.";
                }
            } else {
                qDebug() << "VR_Communicator is not connected or socket is null.";
            }
        } else {
            qDebug() << "Data is empty or index out of range.";
        }

    }

}

void VR_MainWindow::startFileTransferTask() {
    // 设置定时器间隔为1小时（3600000毫秒）
    transferTimer->start(5000);
}
//void VR_MainWindow::onReadySendSeg()
//{
//	if(!CollaborationSendPool.empty())
//	{
//		cout<<"CollaborationSendPool.size()"<<CollaborationSendPool.size()<<endl;
//		QString send_MSG = *CollaborationSendPool.begin();
//		CollaborationSendPool.erase(CollaborationSendPool.begin());
//		if((send_MSG!="exit")&&(send_MSG!="quit"))
//		{
//            VR_Communicator->sendMsg("/drawline:" + send_MSG);
//		}
//	}
//	else
//    {
//        cout<<"CollaborationSendPool is empty";
//	}
//}
//void VR_MainWindow::onReadyRead()
void VR_MainWindow::TVProcess(QString line)
{
    QRegExp msgreg("/(.*)_(.*):(.*)");
    line=line.trimmed();
    qDebug()<<line;
    if(msgreg.indexIn(line)!=-1)
    {
        QString operationtype=msgreg.cap(1).trimmed();
        bool isNorm=msgreg.cap(2).trimmed()=="norm";
        QString operatorMsg=msgreg.cap(3).trimmed();
        if(operationtype == "drawline" )
        {
            QString msg=operatorMsg;
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);
            if(listwithheader.size()<1) return;

            QString user=listwithheader[0].split(" ").at(1).trimmed();
            //QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            int type=-1;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(listwithheader[i]=="$")
                        break;
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_back(converted);

                    type=nodeinfo[0].toInt();
                }
                if(pMainApplication&&!coords.isEmpty())
                {
                    qDebug()<<type<<" "<<coords.size();
                    pMainApplication->UpdateNTList(coords,type);
                    //需要判断线是否在图像中，如果不在则调用全局处理
                }
            }

            if(user==VR_Communicator->userId)
            {
                qDebug()<<"release lock";
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug()<<"user = "<<user<<" "<<userId;
            }
        }else if(operationtype == "delline")
        {
            QString msg = operatorMsg;
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);
            //        qDebug()<<"list with header:"<<listwithheader;
            if(listwithheader.size()<1) return;
            // QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            QStringList infos=listwithheader[0].split(" ");
            QString user=infos.at(1).trimmed();
            unsigned int isMany=0;
            if(infos.size()>=6)
                isMany=infos.at(5).trimmed().toInt();

            QStringList coordsInSeg;

            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(i==listwithheader.size()-1 || listwithheader[i]=="$"){
                        if(listwithheader[i]!="$"){
                            auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                            auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                            coords.push_front(converted);
                            coordsInSeg.append(listwithheader[i]);
                        }
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            if(!pMainApplication->DeleteSegment(coords,0.2*VRVolumeCurrentRes.x/VRvolumeMaxRes.x));
                            {
                                qDebug()<<"delete in block failed";
                                VR_Communicator->emitDelSeg(coordsInSeg.join(";"), 0);
                                //全局删线处理
                            }
                        }
                        coords.clear();
                        coordsInSeg.clear();
                        continue;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_front(converted);
                    coordsInSeg.append(listwithheader[i]);
                }

            }


            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;

            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }else if(operationtype == "splitline"){
            QString msg = operatorMsg;
            QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
            qDebug()<<"splitline_msg_count"<<listwithheader.count();
            if(listwithheader.size()<1)
            {
                qDebug()<<"msg only contains header:"<<msg;
                return;
            }
            QStringList infos=listwithheader[0].split(" ");
            QString user=infos.at(1).trimmed();
            QStringList coordsInDelSeg;
            int index=0;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(listwithheader[i]=="$"){
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            if(!pMainApplication->DeleteSegment(coords,0.2*VRVolumeCurrentRes.x/VRvolumeMaxRes.x));
                            {
                                qDebug()<<"delete in block failed";
                                VR_Communicator->emitDelSeg(coordsInDelSeg.join(";"), 0);
                                //全局删线处理
                            }
                        }
                        coords.clear();
                        coordsInDelSeg.clear();
                        index = i+1;
                        break;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_front(converted);
                    coordsInDelSeg.append(listwithheader[i]);
                }

            }
            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;

            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }

            int type=-1;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=index;i<listwithheader.size();i++)
                {
                    if(i==listwithheader.size()-1 || listwithheader[i]=="$"){
                        if(listwithheader[i]!="$"){
                            auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                            auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                            coords.push_front(converted);
                        }
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            qDebug()<<type<<" "<<coords.size();
                            pMainApplication->UpdateNTList(coords,type);
                            //需要判断线是否在图像中，如果不在则调用全局处理
                        }
                        coords.clear();
                        continue;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_back(converted);
                    type=nodeinfo[0].toInt();
                }
            }

            if(user==VR_Communicator->userId)
            {
                qDebug()<<"release lock";
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug()<<"user = "<<user<<" "<<userId;
            }

        }else if(operationtype == "addmarker")
        {
            QString msg = operatorMsg;
            qDebug()<<"TeraVR add marker";
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);

            if(listwithheader.size()<1) return;
            QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            if(listwithheader.size()>1)
            {
                auto node=listwithheader[1].split(" ");
                RGB8 color;
                color.r = node.at(0).toUInt();
                color.g = node.at(1).toUInt();
                color.b = node.at(2).toUInt();
                float mx = node.at(3).toFloat();
                float my = node.at(4).toFloat();
                float mz = node.at(5).toFloat();
                XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                //需要判断点是否在图像中，如果不在则全局处理
            }
            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }else if(operationtype == "delmarker")
        {
            qDebug()<<"TeraVR del marker";
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);

            if(listwithheader.size()<1) return;
            QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();

            if(listwithheader.size()>1)
            {
                for(int i=1; i<listwithheader.size(); i++){
                    auto node=listwithheader[i].split(" ");
                    RGB8 color;
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    if(!pMainApplication->RemoveMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z))
                    {
                        VR_Communicator->emitDelMarker(listwithheader[i]);
                    }
                    //需要判断点是否在图像中，如果不在则全局处理
                }
            }
            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }else if(operationtype == "retypeline")
        {
            QString msg =operatorMsg;
            QStringList listwithheader=operatorMsg.split(",",QString::SkipEmptyParts);
            if(listwithheader.size()<1) return;
            QStringList infos=listwithheader[0].split(" ");


            QString user=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[1].trimmed();
            int type=listwithheader[0].trimmed().split(' ',QString::SkipEmptyParts)[2].trimmed().toInt();
            unsigned int isMany=0;
            if(infos.size()>=7)
                isMany=infos.at(6).trimmed().toInt();

            QStringList coordsInSeg;
            if(listwithheader.size()>1)
            {
                QVector<XYZ> coords;
                for(int i=1;i<listwithheader.size();i++)
                {
                    if(i==listwithheader.size()-1 || listwithheader[i]=="$"){
                        if(listwithheader[i]!="$"){
                            auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                            auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                            coords.push_front(converted);
                            coordsInSeg.append(listwithheader[i]);
                        }
                        if(pMainApplication&&!coords.isEmpty())
                        {
                            if(!pMainApplication->retypeSegment(
                                    coords,
                                    1.0*VRVolumeCurrentRes.x/VRvolumeMaxRes.x,
                                    type))
                            {
                                qDebug()<<"Vr call fly retype ";
                                VR_Communicator->emitRetypeSeg(coordsInSeg.join(";"),type,0);
                            }
                        }
                        coords.clear();
                        coordsInSeg.clear();
                        continue;
                    }
                    auto nodeinfo=listwithheader[i].split(" ",QString::SkipEmptyParts);
                    auto converted=ConvertMaxGlobal2LocalBlock(nodeinfo[1].toFloat(),nodeinfo[2].toFloat(),nodeinfo[3].toFloat());
                    coords.push_front(converted);
                    coordsInSeg.append(listwithheader[i]);
                }

            }

            if(user==VR_Communicator->userId)
            {
                pMainApplication->READY_TO_SEND=false;
                CURRENT_DATA_IS_SENT=false;
            }else{
                qDebug() << "user = " << user << " " << VR_Communicator->userId << " " << userId;
            }
        }else {

        }
    }
}
void VR_MainWindow::updateBCIstate(QString receivedString){
    qDebug() << "updateBCIstate Received message from client:" << receivedString;

    // 使用空格分割字符串
    QStringList parts = receivedString.split(" ");

    // 检查是否有足够的部分
    if (parts.size() >= 5) {
        // 解析各个部分
        QString userIdD = parts[1];
        QString BCI_paradigm = parts[2];
        bool BCI_ssvep_mode = (parts[3] == "true"); // 将字符串转换为bool
        float BCI_parameter = parts[4].toFloat();
        BCIParameters param;
        // 现在可以使用解析出来的数据进行处理
        qDebug() << "User ID:" << userIdD;
        qDebug() << "BCI Paradigm:" << BCI_paradigm;
        qDebug() << "BCI SSVEP Mode:" << BCI_ssvep_mode;
        qDebug() << "BCI Parameter:" << BCI_parameter;

        // 使用setParams函数设置成员变量的值
        param.setParams(userIdD, BCI_paradigm, BCI_ssvep_mode, BCI_parameter);


        params.setParams(userIdD, BCI_paradigm, BCI_ssvep_mode, BCI_parameter);

            if(pMainApplication->m_modeGrip_L==_m_cobci){

                pMainApplication->setIsReady(BCI_ssvep_mode);
                pMainApplication->setFSSVEPHz(BCI_parameter);
                // 设置定时器的间隔
                int interval = (int)1000 / BCI_parameter;
                pMainApplication->m_timer->setInterval(interval);

                std::cout << " handle Increased fSSVEPHz: " << BCI_parameter << std::endl;
                if(BCI_ssvep_mode){


                     qDebug() <<VR_Communicator->userId <<"will join co-bci from "<<userIdD;
                     if (!pMainApplication->timer_eegGet->isActive()) {

                         bool success = pMainApplication->startBCIparadigm();  // 调用 startBCIparadigm 函数

                         if (success) {
                             // BCI 范式启动成功
                             qDebug() << "BCI paradigm started successfully.";


                         } else {
                             // 启动失败，可能需要处理失败情况
                             qDebug() << "Failed to start BCI paradigm.";
                             pMainApplication->setIsSSVEP(false);
                         }


                     }
                }
                 else
                {
                    qDebug() <<VR_Communicator->userId <<"will leave co-bci from "<<userIdD;

                     pMainApplication->stopBCIparadigm();
                     pMainApplication->setIsSSVEP(false);


                }


            }




        // 进行进一步的处理或者回复客户端
    } else {
        // 如果部分数量不够，处理格式错误情况
        qDebug() << "Received message format error!";
        // 可以返回错误信息给客户端或者进行其他处理
    }
}
void VR_MainWindow::processWarnMsg(QString line){
    QRegExp warnreg("/WARN_(.*):(.*)");
    line=line.trimmed();
    if(warnreg.indexIn(line)!=-1)
    {
        QString reason=warnreg.cap(1).trimmed();
        QString operatorMsg=warnreg.cap(2).trimmed();
        QString msg = operatorMsg;
        QStringList listwithheader=msg.split(',',QString::SkipEmptyParts);
        //        if(listwithheader.size()<2)
        //        {
        //            qDebug()<<"msg only contains header:"<<msg;
        //            return;
        //        }

        QString header = listwithheader[0];
        QString sender=header.split(" ").at(0).trimmed();
        listwithheader.removeAt(0);

        if(sender=="server"){
            if(reason=="TipUndone" || reason=="CrossingError" || reason=="MulBifurcation" || reason=="BranchingError")
            {
                if(listwithheader.size()>=1)
                {
                    for(int i=0; i<listwithheader.size(); i++){
                        auto node=listwithheader[i].split(" ");
                        RGB8 color;
                        color.r = node.at(0).toUInt();
                        color.g = node.at(1).toUInt();
                        color.b = node.at(2).toUInt();
                        float mx = node.at(3).toFloat();
                        float my = node.at(4).toFloat();
                        float mz = node.at(5).toFloat();
                        XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                        pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                        //需要判断点是否在图像中，如果不在则全局处理
                    }
                }
            }
            else if(reason=="Loop")
            {
                int result = header.split(" ").at(1).trimmed().toUInt();
                if(result == 1){
                    //                emit setDefineSomaActionState(true);
                }
                if(result == 0){
                    //                emit setDefineSomaActionState(false);
                    for(int i=0; i<listwithheader.size(); i++){
                        auto node=listwithheader[i].split(" ");
                        RGB8 color;
                        color.r = node.at(0).toUInt();
                        color.g = node.at(1).toUInt();
                        color.b = node.at(2).toUInt();
                        float mx = node.at(3).toFloat();
                        float my = node.at(4).toFloat();
                        float mz = node.at(5).toFloat();
                        XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                        pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                        //需要判断点是否在图像中，如果不在则全局处理
                    }
                }
            }
            else if(reason=="DisconnectError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("Disconnect from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="GetSwcMetaInfoError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetSwcMetaInfoError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="ApoFileNotFoundError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("ApoFileNotFoundError!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="GetApoDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetApoDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="GetSwcFullNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("GetSwcFullNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="AddSwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("AddSwcNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="DeleteSwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("DeleteSwcNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="ModifySwcNodeDataError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("ModifySwcNodeDataError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="UpdateSwcAttachmentApoError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("UpdateSwcAttachmentApoError from DBMS!"),
                                         QMessageBox::Ok);
            }
            else if(reason=="FullNumberError"){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("The number of collaborating users has been full about this swc!"),
                                         QMessageBox::Ok);
            }
            else {
                if(listwithheader.size()>=1)
                {
                    auto node=listwithheader[0].split(" ");
                    RGB8 color;
                    color.r = node.at(0).toUInt();
                    color.g = node.at(1).toUInt();
                    color.b = node.at(2).toUInt();
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
            }
        }

    }
}

void VR_MainWindow::processAnalyzeMsg(QString line){
    qDebug()<<line;
    QRegExp analyzereg("/FEEDBACK_ANALYZE_(.*):(.*)");
    QRegExp definereg("/FEEDBACK_DEFINE_(.*):(.*)");
    line=line.trimmed();
    if(analyzereg.indexIn(line) != -1)
    {
        QString type=analyzereg.cap(1).trimmed();
        QString operatorMsg=analyzereg.cap(2).trimmed();

        qDebug()<<"type:"<<type;
        qDebug()<<"operatormsg:"<<operatorMsg;

        if(type=="ColorMutation"){
            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
            QString msgHeader=listWithHeader[0];
            QStringList msgList=listWithHeader;
            msgList.removeAt(0);

            QString sender=msgHeader.split(" ").at(0).trimmed();
            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
            int result=msgHeader.split(" ").at(2).trimmed().toInt();
            if (sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         tr("no error"),
                                         QMessageBox::Ok);
            }
            else if(sender=="server" && result==0){
                for(int i=0; i<msgList.size(); i++){
                    auto node=msgList[i].split(" ");
                    RGB8 color;
                    color.r = node.at(0).toUInt();
                    color.g = node.at(1).toUInt();
                    color.b = node.at(2).toUInt();
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
                if( request_senderid==userId )
                    QMessageBox::information(0,tr("Infomation "),
                                             tr("error: color mutation exists! notice the soma nearby and the red markers!"),
                                             QMessageBox::Ok);
            }else if(sender=="server" && result==-1){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("error: soma not detected!"),
                                         QMessageBox::Ok);
            }
        }
        if(type=="Dissociative"){
            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
            QString msgHeader=listWithHeader[0];
            QStringList msgList=listWithHeader;
            msgList.removeAt(0);

            QString sender=msgHeader.split(" ").at(0).trimmed();
            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
            int result=msgHeader.split(" ").at(2).trimmed().toInt();
            if (sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         tr("no error"),
                                         QMessageBox::Ok);
            }
            else if(sender=="server" && result==0){
                for(int i=0; i<msgList.size(); i++){
                    auto node=msgList[i].split(" ");
                    RGB8 color;
                    color.r = node.at(0).toUInt();
                    color.g = node.at(1).toUInt();
                    color.b = node.at(2).toUInt();
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
                if(request_senderid==userId)
                    QMessageBox::information(0,tr("Infomation "),
                                             tr("error: dissociative seg exists! notice the red markers!"),
                                             QMessageBox::Ok);
            }
        }
        if(type=="Angle"){
            QStringList listWithHeader=operatorMsg.split(",",QString::SkipEmptyParts);
            QString msgHeader=listWithHeader[0];
            QStringList msgList=listWithHeader;
            msgList.removeAt(0);

            QString sender=msgHeader.split(" ").at(0).trimmed();
            QString request_senderid=msgHeader.split(" ").at(1).trimmed();
            int result=msgHeader.split(" ").at(2).trimmed().toInt();
            if (sender=="server" && result==1)
            {
                QMessageBox::information(0,tr("Infomation "),
                                         tr("no error"),
                                         QMessageBox::Ok);
            }
            else if(sender=="server" && result==0){
                for(int i=0; i<msgList.size(); i++){
                    auto node=msgList[i].split(" ");
                    RGB8 color;
                    color.r = node.at(0).toUInt();
                    color.g = node.at(1).toUInt();
                    color.b = node.at(2).toUInt();
                    float mx = node.at(3).toFloat();
                    float my = node.at(4).toFloat();
                    float mz = node.at(5).toFloat();
                    XYZ  converreceivexyz = ConvertMaxGlobal2LocalBlock(mx, my, mz);
                    pMainApplication->SetupMarkerandSurface(converreceivexyz.x, converreceivexyz.y, converreceivexyz.z, color.r, color.g, color.b);
                    //需要判断点是否在图像中，如果不在则全局处理
                }
                if(request_senderid==userId)
                    QMessageBox::information(0,tr("Infomation "),
                                             tr("error: incorrect angle exists! notice the red markers!"),
                                             QMessageBox::Ok);
            }else if(sender=="server" && result==-1){
                QMessageBox::information(0,tr("Infomation "),
                                         tr("error: soma not detected!"),
                                         QMessageBox::Ok);
            }
        }
    }
}

int VR_MainWindow::StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,
                                bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,V3dR_Communicator* TeraflyCommunicator,
                                XYZ* zoomPOS,XYZ *CreatorPos,XYZ MaxResolution) {

    qDebug()<<"StartVRScene-lddddddddddddd1114";

    pMainApplication = new CMainApplication(  0, 0 ,TeraflyCommunicator->CreatorMarkerPos);

    pMainApplication->mainwindow =pmain;

    pMainApplication->isOnline = isLinkSuccess;
    GetResindexandStartPointfromVRInfo(ImageVolumeInfo,MaxResolution);

    if(ntlist != NULL)
    {
        if((ntlist->size()==1)&&(ntlist->at(0).name.isEmpty()))
        {
            // means there is only a reloaded annotation in terafly
            // we rename it as vaa3d_traced_neuron
            qDebug()<<"means this is terafly special condition.do something";
            NeuronTree newS;
            newS.color = XYZW(0,0,255,255);
            newS = ntlist->at(0);
            newS.n = 1;
            newS.on = true;
            newS.name = "vaa3d_traced_neuron";
            newS.file = "vaa3d_traced_neuron";
            pMainApplication->editableLoadedNTL.append(newS);
        }
        else
        {
            for(int i=0;i<ntlist->size();i++)
            {
                if((ntlist->at(i).name == "vaa3d_traced_neuron")&&(ntlist->at(i).file == "vaa3d_traced_neuron"))
                {
                    // means there is a NT named "vaa3d_traced_neuron", we only need to edit this NT.
                    pMainApplication->editableLoadedNTL.append(ntlist->at(i));
                }
                else if (!ntlist->at(0).name.isEmpty())
                {
                    // means it is a loaded Neuron in 3D View,currently we do not allow to edit this neuron in VR
                    pMainApplication->nonEditableLoadedNTL.append(ntlist->at(i));
                }
                // else if (ntlist->at(0).name.isEmpty())
                // means it is an reloaded annotation in terafly, currently we do not show this neuron in VR
            }
        }
    }
    pMainApplication->loadedNTList = ntlist;
    if(i4d->valid())
    {
        pMainApplication->img4d = i4d;
        pMainApplication->m_bHasImage4D=true;
    }
    if (!pMainApplication->BInit())
    {
        pMainApplication->Shutdown();
        qDebug()<<"init failed";
        return 0;
    }

    // 尝试初始化应用程序
    if (!pMainApplication->eegDevice.initdevice()) {
    qDebug() << "eegDevice Initialization failed. Shutting down.";
    QMessageBox::critical(nullptr, "Initialization Failed", "EEG Device initialization failed. Shutting down.");
    }
    qDebug() << "eegDevice Initialization successful.";


    SendVRconfigInfo();
    pMainApplication->SetupCurrentUserInformation(VR_Communicator->userId.toStdString(),0);
    RunVRMainloop(zoomPOS);
    pMainApplication->Shutdown();
    qDebug()<<"Now quit VR";
    int _call_that_function = pMainApplication->postVRFunctionCallMode;
    zoomPOS->x = pMainApplication->teraflyPOS.x;
    zoomPOS->y = pMainApplication->teraflyPOS.y;
    zoomPOS->z = pMainApplication->teraflyPOS.z;

    CreatorPos->x = VR_Communicator->CreatorMarkerPos.x;
    CreatorPos->y = VR_Communicator->CreatorMarkerPos.y;
    CreatorPos->z = VR_Communicator->CreatorMarkerPos.z;
    CreatorRes = VR_Communicator->CreatorMarkerRes;
    qDebug()<<"call that function is"<<_call_that_function;
    delete pMainApplication;
    pMainApplication=0;
    return _call_that_function;
}

//void VR_MainWindow::SendHMDPosition()
//{
//	if(!pMainApplication) return;
//	//get hmd position
//	QString PositionStr=pMainApplication->getHMDPOSstr();

//	//send hmd position
//    //VR_Communicator->onReadySend(QString("/hmdpos:" + PositionStr));
//	//QTimer::singleShot(2000, this, SLOT(SendHMDPosition()));
//	//cout<<"socket resindex"<<ResIndex<<endl;
//	//qDebug()<<"QString resindex"<< QString("%1").arg(ResIndex);
//    //VR_Communicator->onReadySend(QString("/ResIndex:" + QString("%1").arg(ResIndex) ));
//}
void VR_MainWindow::RunVRMainloop(XYZ* zoomPOS)
{
    qDebug()<<"get into RunMainloop";
    bool bQuit = false;
    while(!bQuit)
    {
        //handle one rendering loop, and handle user interaction
        //        qDebug()<<"VR_MainWindow"
        bQuit=pMainApplication->HandleOneIteration();
        //READY_TO_SEND is set to true by the "trigger button up" event;
        //client sends data to server (using onReadySend());
        //server sends the same data back to client;
        //READY_TO_SEND is set to false in onReadyRead();
        //CURRENT_DATA_IS_SENT is used to ensure that each data is only sent once.
        if(!transferTimer->isActive())
        {
            startFileTransferTask(); // 启动定时器任务
        }

       // qDebug()<<pMainApplication->READY_TO_SEND<<" "<<CURRENT_DATA_IS_SENT;
        if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
        {
            if(pMainApplication->undo)
            {
                if(VR_Communicator->undoDeque.size())
                {
                    VR_Communicator->UpdateUndoDeque();
                    CURRENT_DATA_IS_SENT=true;
                }
                else{
                    pMainApplication->TriggerHapticPluse();
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }
                pMainApplication->undo=false;
            }else if(pMainApplication->redo)
            {
                if(VR_Communicator->redoDeque.size())
                {
                    VR_Communicator->UpdateRedoDeque();
                    CURRENT_DATA_IS_SENT=true;
                }
                else{
                    pMainApplication->TriggerHapticPluse();
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                }

                pMainApplication->redo=false;
            }

        }
        if(pMainApplication->m_modeGrip_L==_m_cobci)
        {


                        QString BCI_paradigm="ssvep";
                            // 使用setParams函数设置成员变量的值
                        params.setParams(VR_Communicator->userId, BCI_paradigm,pMainApplication->getIsReady(), pMainApplication->getFSSVEPHz());

                        // 构建待发送的QString
                        QString waitsend = QString("1 %1 %2 %3 %4")
                                .arg(params.userId)
                                .arg(params.BCI_paradigm)
                                .arg(params.BCI_ssvep_mode ? "true" : "false") // 将bool转换为字符串
                                .arg(params.BCI_parameter);
                        if(waitsend!=lastBCIstate)
                        {


                            if(VR_Communicator&&
                                VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                            {


                                VR_Communicator->UpdateBCIMsg(waitsend);

                                lastBCIstate=waitsend;
                                CURRENT_DATA_IS_SENT=true;
                            }

                        }else
                        {
                            pMainApplication->READY_TO_SEND=false;
                            CURRENT_DATA_IS_SENT=false;

                        }
     }
        if((pMainApplication->READY_TO_SEND==true)&&(CURRENT_DATA_IS_SENT==false))
        {
            if(pMainApplication->m_modeGrip_R==m_drawMode)
            {
                qDebug()<<"TeraVR add seg";
                QStringList waitsend=pMainApplication->NT2QString(pMainApplication->currentNT);
                if(waitsend.size())
                {

                    pMainApplication->ClearCurrentNT();
                    if(VR_Communicator&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        waitsend.push_front(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                        waitsend.append("$");
                        VR_Communicator->UpdateAddSegMsg(waitsend.join(","));
                        CURRENT_DATA_IS_SENT=true;
                    }
                }else
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                }
            }
            else if(pMainApplication->m_modeGrip_R==m_deleteMode)
            {
                if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0)
                {
                    QStringList result;
                    result.push_back(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                    {
                        result.push_back(ConvertToMaxGlobalForSeg(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                                                                      .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    if(VR_Communicator&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        VR_Communicator->UpdateDelSegMsg(QString(result.join(",")));
                        CURRENT_DATA_IS_SENT=true;
                        pMainApplication->SegNode_tobedeleted.x = 0;
                        pMainApplication->SegNode_tobedeleted.y = 0;
                        pMainApplication->SegNode_tobedeleted.z = 0;
                        qDebug()<<"TeraVR del seg sucess";
                    }
                }else{
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                    qDebug()<<"TeraVR del seg failed";
                }
            }
            else if(pMainApplication->m_modeGrip_R==m_markMode)
            {
                if(pMainApplication->markerPosTobeDeleted!="")
                {
                    QStringList result;
                    QString ConvertedmarkerPOS = ConvertToMaxGlobalForMarker(pMainApplication->markerPosTobeDeleted);
                    result.push_back(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    result.push_back(ConvertedmarkerPOS);
                    if(ConvertedmarkerPOS.split(" ")[0]=="-1")
                    {
                        qDebug()<<"TeraVR del marker";
                        if(VR_Communicator&&
                            VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                        {
                            VR_Communicator->UpdateDelMarkerMsg(QString(result.join(",")));
                        }
                    }else
                    {
                        qDebug()<<"TeraVR add marker";
                        if(VR_Communicator&&
                            VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                        {
                            VR_Communicator->UpdateAddMarkerMsg(QString(result.join(",") ));
                        }
                    }
                    pMainApplication->markerPosTobeDeleted.clear();
                    CURRENT_DATA_IS_SENT=true;
                }else
                {
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->markerPosTobeDeleted="";
                }
            }else if(pMainApplication->m_modeGrip_R==m_retypeMode)
            {
                if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0)
                {
                    QStringList result;
                    result.push_back(QString("1 %1 %2 %3 %4 %5").arg(VR_Communicator->userId).arg(pMainApplication->m_curMarkerColorType).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                    {
                        result.push_back(ConvertToMaxGlobalForSeg(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                                                                      .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    if(VR_Communicator&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        VR_Communicator->UpdateRetypeSegMsg(QString(result.join(",")));
                        CURRENT_DATA_IS_SENT=true;
                        pMainApplication->SegNode_tobedeleted.x = 0;
                        pMainApplication->SegNode_tobedeleted.y = 0;
                        pMainApplication->SegNode_tobedeleted.z = 0;
                        qDebug()<<"TeraVR retype seg sucess";
                    }
                }else{
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                    qDebug()<<"TeraVR retype seg failed";
                }
            }else if(pMainApplication->m_modeGrip_R == m_splitMode)
            {
                if (pMainApplication->SegNode_tobedeleted.x >0 || pMainApplication->SegNode_tobedeleted.y > 0 || pMainApplication->SegNode_tobedeleted.z > 0 )
                {
                    if(pMainApplication->segaftersplit.size()!=2)
                    {
                        pMainApplication->READY_TO_SEND=false;
                        CURRENT_DATA_IS_SENT=false;
                        pMainApplication->ClearCurrentNT();
                        pMainApplication->segaftersplit.clear();
                        qDebug()<<"TeraVR del seg failed";
                    }
                    QStringList result;
                    result.push_back(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                    for(int i=0;i<pMainApplication->segtobedeleted.listNeuron.size();i++)
                    {
                        result.push_back(ConvertToMaxGlobalForSeg(QString("%1 %2 %3 %4").arg(pMainApplication->segtobedeleted.listNeuron[i].x)
                                                                      .arg(pMainApplication->segtobedeleted.listNeuron[i].y).arg(pMainApplication->segtobedeleted.listNeuron[i].z).arg(pMainApplication->segtobedeleted.listNeuron[i].type)));
                    }
                    qDebug()<<"result = "<<result;
                    QStringList waitsends;
                    for(auto nt:pMainApplication->segaftersplit)
                    {
                        QStringList waitsend=pMainApplication->NT2QString(nt);
                        waitsend.push_front(QString("1 %1 %2 %3 %4").arg(VR_Communicator->userId).arg(VRVolumeCurrentRes.x).arg(VRVolumeCurrentRes.y).arg(VRVolumeCurrentRes.z));
                        waitsend.append("$");
                        waitsends.push_back(waitsend.join(","));
                    }
                    pMainApplication->segaftersplit.clear();
                    qDebug()<<"waitsends = "<<waitsends;


                    if(VR_Communicator&&
                        VR_Communicator->socket->state()==QAbstractSocket::ConnectedState)
                    {
                        //                    VR_Communicator->UpdateDelSegMsg(QString(result.join(";")));
                        //                    for(auto addmsg:waitsends)
                        //                    {
                        //                        VR_Communicator->UpdateAddSegMsg(addmsg);
                        //                    }
                        //
                        VR_Communicator->UpdateSplitSegMsg(QString(result.join(",")),waitsends.at(0),waitsends.at(1));
                        CURRENT_DATA_IS_SENT=true;
                        pMainApplication->SegNode_tobedeleted.x = 0;
                        pMainApplication->SegNode_tobedeleted.y = 0;
                        pMainApplication->SegNode_tobedeleted.z = 0;
                        qDebug()<<"TeraVR del seg sucess";
                    }
                }else{
                    pMainApplication->READY_TO_SEND=false;
                    CURRENT_DATA_IS_SENT=false;
                    pMainApplication->ClearCurrentNT();
                    pMainApplication->segaftersplit.clear();
                    qDebug()<<"TeraVR del seg failed";
                }
            }
        }
    }

}

//-----------------------------------------------------------------------------
// Purpose: for standalone VR.
//-----------------------------------------------------------------------------
int startStandaloneVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain, XYZ* zoomPOS)
// bool startStandaloneVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain)
{
qDebug()<<"startStandaloneVRScene-lddddddddddddd888";
    CMainApplication *pMainApplication = new CMainApplication( 0, 0 );
    //pMainApplication->setnetworkmodefalse();//->NetworkModeOn=false;
    pMainApplication->mainwindow = pmain;
    pMainApplication->isOnline = false;

qDebug()<<"startStandaloneVRScene-lddddddddddddd";
    if(ntlist != NULL)
    {
        if((ntlist->size()==1)&&(ntlist->at(0).name.isEmpty()))
        {
            // means there is only a reloaded annotation in terafly
            // we rename it as vaa3d_traced_neuron
            qDebug()<<"means this is terafly special condition.do something";
            NeuronTree newS;
            newS.color = XYZW(0,0,255,255);
            newS = ntlist->at(0);
            newS.n = 1;
            newS.on = true;
            newS.name = "vaa3d_traced_neuron";
            newS.file = "vaa3d_traced_neuron";
            pMainApplication->editableLoadedNTL.append(newS);
        }
        else
        {
            for(int i=0;i<ntlist->size();i++)
            {
                if((ntlist->at(i).name == "vaa3d_traced_neuron")&&(ntlist->at(i).file == "vaa3d_traced_neuron"))
                {
                    // means there is a NT named "vaa3d_traced_neuron", we only need to edit this NT.
                    pMainApplication->editableLoadedNTL.append(ntlist->at(i));
                }
                else if (!ntlist->at(0).name.isEmpty())
                {
                    // means it is a loaded Neuron in 3D View,currently we do not allow to edit this neuron in VR
                    pMainApplication->nonEditableLoadedNTL.append(ntlist->at(i));
                }
                // else if (ntlist->at(0).name.isEmpty())
                // means it is an reloaded annotation in terafly, currently we do not show this neuron in VR
            }
        }
    }
    pMainApplication->loadedNTList = ntlist;
qDebug()<<"startStandaloneVRScene-i4d->valid()";
    if(i4d->valid())
    {
        pMainApplication->img4d = i4d;
        pMainApplication->m_bHasImage4D=true;
    }
    qDebug()<<"startStandaloneVRScene-!pMainApplication->BInit())";
    if (!pMainApplication->BInit())
    {
        pMainApplication->Shutdown();
        return 0;
    }
qDebug()<<"startStandaloneVRScene-lddddddddddddd";
    // 尝试初始化应用程序
    if (!pMainApplication->eegDevice.initdevice()) {
    qDebug() << "eegDevice Initialization failed. Shutting down.";
    QMessageBox::critical(nullptr, "Initialization Failed", "EEG Device initialization failed. Shutting down.");

    }
    qDebug() << "eegDevice Initialization successful.";




    pMainApplication->SetupCurrentUserInformation("local user", 13);

    pMainApplication->RunMainLoop();

    pMainApplication->Shutdown();

    // bool _call_that_plugin = pMainApplication->_call_assemble_plugin;
    int _call_that_function = pMainApplication->postVRFunctionCallMode;
    zoomPOS->x = pMainApplication->teraflyPOS.x;
    zoomPOS->y = pMainApplication->teraflyPOS.y;
    zoomPOS->z = pMainApplication->teraflyPOS.z;
    delete pMainApplication;
    pMainApplication = NULL;

    // return _call_that_plugin;
    return _call_that_function;
}
void VR_MainWindow::GetResindexandStartPointfromVRInfo(QString VRinfo,XYZ CollaborationMaxResolution)
{
    qDebug() << "GetResindexandStartPointfromVRInfo........";
    qDebug()<<"VRinfo"<<VRinfo;
    QRegExp rx("Res\\((\\d+)\\s.\\s(\\d+)\\s.\\s(\\d+)\\),Volume\\sX.\\[(\\d+),(\\d+)\\],\\sY.\\[(\\d+),(\\d+)\\],\\sZ.\\[(\\d+),(\\d+)\\]");
    if (rx.indexIn(VRinfo) != -1 && (ResIndex != -1)) {
        qDebug()<<"get  VRResindex and VRVolume Start point ";
        VRVolumeStartPoint = XYZ(rx.cap(4).toInt(),rx.cap(6).toInt(),rx.cap(8).toInt());
        VRVolumeEndPoint = XYZ(rx.cap(5).toInt(),rx.cap(7).toInt(),rx.cap(9).toInt());
        VRVolumeCurrentRes = XYZ(rx.cap(1).toInt(),rx.cap(2).toInt(),rx.cap(3).toInt());
        VRvolumeMaxRes = CollaborationMaxResolution;

    }
    else
    {
        VRVolumeStartPoint = XYZ(1,1,1);
        VRVolumeEndPoint = CollaborationMaxResolution;
        VRVolumeCurrentRes = CollaborationMaxResolution;
        VRvolumeMaxRes = CollaborationMaxResolution;

    }
    //pass Resindex and VRvolumeStartPoint to PMAIN  to  offer parameter to NT2QString
    pMainApplication->collaborationTargetdelcurveRes = VRvolumeMaxRes;
    pMainApplication->CmainResIndex = ResIndex;
    pMainApplication->CmainVRVolumeStartPoint = VRVolumeStartPoint;
    pMainApplication->CmainVRVolumeEndPoint=VRVolumeEndPoint;


    pMainApplication->CollaborationMaxResolution = CollaborationMaxResolution;
    pMainApplication->CollaborationCurrentRes = VRVolumeCurrentRes;

}

QString VR_MainWindow::ConvertToMaxGlobalForMarker(QString coords)
{
    float x = coords.section(' ',0, 0).toFloat();  // str == "bin/myapp"
    float y = coords.section(' ',1, 1).toFloat();  // str == "bin/myapp"
    float z = coords.section(' ',2, 2).toFloat();  // str == "bin/myapp"
    int r=coords.section(' ',3, 3).toInt();
    int g=coords.section(' ',4, 4).toInt();
    int b=coords.section(' ',5, 5).toInt();
    x+=(VRVolumeStartPoint.x-1);
    y+=(VRVolumeStartPoint.y-1);
    z+=(VRVolumeStartPoint.z-1);
    x*=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
    y*=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
    z*=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
    return QString("%1 %2 %3 %4 %5 %6").arg(r).arg(g).arg(b).arg(x).arg(y).arg(z);
}

QString VR_MainWindow::ConvertToMaxGlobalForSeg(QString coords){
    float x = coords.section(' ',0, 0).toFloat();  // str == "bin/myapp"
    float y = coords.section(' ',1, 1).toFloat();  // str == "bin/myapp"
    float z = coords.section(' ',2, 2).toFloat();  // str == "bin/myapp"
    int type = coords.section(' ',3, 3).toInt();
    x+=(VRVolumeStartPoint.x-1);
    y+=(VRVolumeStartPoint.y-1);
    z+=(VRVolumeStartPoint.z-1);
    x*=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
    y*=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
    z*=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
    return QString("%1 %2 %3 %4").arg(type).arg(x).arg(y).arg(z);
}

void VR_MainWindow::SendVRconfigInfo()
{
    //float globalscale = pMainApplication->GetGlobalScale();
    //QString QSglobalscale = QString("%1").arg(globalscale);
    //   VR_Communicator->onReadySend(QString("/scale:" +  QSglobalscale ));
}

XYZ VR_MainWindow:: ConvertBlock2GloabelInRES(XYZ local)
{
    return XYZ(local.x+VRVolumeStartPoint.x,local.y+VRVolumeStartPoint.y,local.z+VRVolumeStartPoint.z);
}
XYZ VR_MainWindow:: ConvertMaxGlobal2LocalBlock(float x,float y,float z)
{
    //QString str1 = coords.section(' ',0, 0);  // str == "bin/myapp"
    //QString str2 = coords.section(' ',1, 1);  // str == "bin/myapp"
    //QString str3 = coords.section(' ',2, 2);  // str == "bin/myapp"
    x/=(VRvolumeMaxRes.x/VRVolumeCurrentRes.x);
    y/=(VRvolumeMaxRes.y/VRVolumeCurrentRes.y);
    z/=(VRvolumeMaxRes.z/VRVolumeCurrentRes.z);
    x-=(VRVolumeStartPoint.x-1);
    y-=(VRVolumeStartPoint.y-1);
    z-=(VRVolumeStartPoint.z-1);
    return XYZ(x,y,z);
}
