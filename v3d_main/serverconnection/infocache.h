#ifndef INFOCACHE_H
#define INFOCACHE_H

#include <QString>

/**
 * @brief 该类需单例创建，用于存储连接网络的用户信息
 * - 使用懒汉式创建，多线程不安全！
 */
class InfoCache
{
private:

    InfoCache() {};
    QString account;
    QString token;

public:
    // best of all singleton(lazy)
    static InfoCache& getInstance() {
//        if(instance == nullptr) {
//            instance = new InfoCache();
//        }
        static InfoCache instance;
        return instance;
    }
    ~InfoCache() {};
    InfoCache& operator=(const InfoCache&)=delete;



    void clear() {
        account = "";
    }

    QString getAccount() {
        return account;
    }

    QString getToken() {
        return token;
    }

    void setAccount(QString account);
    void setToken(QString token);

};


#endif // INFOCACHE_H
