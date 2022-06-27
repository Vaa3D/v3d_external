#ifndef INFOCACHE_H
#define INFOCACHE_H

#include <QString>

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
