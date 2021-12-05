#ifndef MANAGESOCKET_H
#define MANAGESOCKET_H

#include <QTcpSocket>
#include <QListWidgetItem>
#include "../terafly/src/presentation/PMain.h"

/**
 * @brief The ManageSocket class
 * 继承自QObject
 * 管理客户端的command请求：下载文件、加载文件和上传文件
 */

class ManageSocket:public QTcpSocket
{
    /**
     * @brief The DataInfo struct
     * 接收数据用的数据结构，控制判断该数据块是否已经读取完成，读取完成后重置该数据块
     */
    struct DataInfo
    {
        qint32 dataSize;    /*!<该数据块的长度，初始值为0*/
        qint32 stringOrFilenameSize;/*!<command的长度或者文件名的长度*/
        qint32 filedataSize;/*!<文件的长度，当传输的不是文件是command时为0*/
        qint32 dataReadedSize;/*!<已经读取的数据的长度，当数据块被完全读取是等于dataSize*/
    };

    Q_OBJECT
public:
    /**
     * @brief ManageSocket
     * @param handle socket的描述符
     * @param parent QT父类
     * 初始化类的各个属性，建立一个新的QTcpSocket，并设置其socket描述符
     * 建立该对象的信号和槽连接     *
     */
    ManageSocket(QObject *partent=nullptr);
    virtual ~ManageSocket(){

    }

    /**
     * @brief sendMsg 发送消息
     * @param msg :QString待发生的消息
     *
     */
    void sendMsg(QString msg);
    /**
     * @brief sendFiles 发送多个(>=1)文件
     * @param filePathList 文件的路径列表
     * @param fileNameList 文件名列表
     * 如果文件是tmp文件夹中的，发送后即将自动删除文件
     */
    void sendFiles(QStringList filePathList,QStringList fileNameList);
public slots:
    /**
     * @brief onreadyRead
     * 槽函数
     * 当QTcpSocket::readyRead信号被触发时，被调用，用于读取tcp传输的数据块
     * 读取完成后将根据数据块的种类调用command处理函数/文件处理函数
     * 冗余设计，可一次接受多条command或多个文件，以及command和文件的组合，但一般每一要么接受文件要么接受文件
     */
    void onreadyRead();
    void download(QListWidgetItem*);
    void load(QListWidgetItem*);
    void onMessageConnect();
//    void onMessageDisConnect();
private:
    /**
     * @brief resetDataInfo
     * 重置控制数据接受的结构
     */
    void resetDataInfo();

    /**
     * @brief processReaded 处理接受的数据包
     * @param list command和文件名队列
     * 每次从中读取command或文件名，直到遇到文件名或command，然后处理command列表或文件名列表，循环直到list空
     */
    void processReaded(QStringList list);
    /**
     * @brief processMsg 处理command
     * @param msglist command队列
     * command格式：
     * 下载："(.*):DownloadANO" （.*）为文件名列表
     * 加载神经元："(.*):LoadANO")
     */
    void processMsg( QString &msg);
    /**
     * @brief processFile 处理接受的文件队列
     * 冗余设计，可以及接受多个文件
     * @param filePaths 文件路径列表
     */
    void processFile( QStringList &filePaths);


public:
    QString ip;
    QString name;
    bool flag=false;/*!<判断是否是自己退出*/
    terafly::PMain *pmain=nullptr;
private:
    DataInfo dataInfo;/*!<用于控制数据接受的数据结构*/
    QStringList filepaths;/*!<文件路径队列*/
    QListWidget *listwidget=nullptr;
};
#endif // MANAGESOCKET_H

