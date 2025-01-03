#ifndef INFOCACHE_H
#define INFOCACHE_H

#include <QString>
#include <QDebug>

struct ArborInfo{
    int arborid;    //id
    QString image;  //brainid
    QString somaId; //somaid
    QString name;   //arborname
    double x;
    double y;
    double z;
    int status;
    QString ab_path;

    bool isvisable;
    long weight;
    ArborInfo(){
        arborid=-1;
        image="";
        somaId="";
        name="";
        x=-1;
        y=-1;
        z=-1;
        status=-2;
        ab_path="";

        isvisable=false;
        weight=100; //prepared for recommending to CheckWidget
    }

    void print(){
        qDebug()<<arborid<<x<<y<<z<<image<<somaId<<name;
    }
    bool operator==(const ArborInfo &img){
        return ((arborid==img.arborid)&&(x==img.x)&&(y==img.y)&&(z==img.z));
    }
};

struct ImgStatus{
    bool isopen=false;
    bool isimgready=false;
    bool isswcready=false;
    bool ischecked=false;
};

class InfoCache
{
public:
    // best of all singleton(lazy)
    static InfoCache& getInstance() {
        static InfoCache instance;
        return instance;
    }
    ~InfoCache() {imgs=nullptr;}
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

    void setPimgs(QVector<ArborInfo>* t_imgs);

    QHash<QString,ImgStatus> WidgetStatus;

    int getready();

    void resetws();
private:

    QVector<ArborInfo>* imgs=nullptr;
    InfoCache() {};
    QString account;
    QString token;



};


#endif // INFOCACHE_H
