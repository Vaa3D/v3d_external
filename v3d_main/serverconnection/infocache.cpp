#include "infocache.h"


void InfoCache::setAccount(QString account)
{
    InfoCache::getInstance().account = account;
}

void InfoCache::setToken(QString token)
{
    InfoCache::getInstance().token = token;
}

void InfoCache::setPimgs(QVector<ArborInfo> *t_imgs)
{
    imgs=t_imgs;
}

int InfoCache::getready()
{
    int cnt=0;
    QHash<QString,ImgStatus>::iterator it=WidgetStatus.begin();
    while(it!=WidgetStatus.end()){
//        qDebug()<<"csz "<<it.value().isimgready<<it.value().isswcready<<it.value().ischecked;
        if(it.value().isimgready==true&&it.value().isswcready==true&&it.value().ischecked==false)
            cnt++;
        it++;
    }
    return cnt;
}

void InfoCache::resetws()
{
    QHash<QString,ImgStatus>::iterator it=WidgetStatus.begin();
    while(it!=WidgetStatus.end()){
        if(it.value().isopen==true&&it.value().ischecked==true)
            WidgetStatus.erase(it);
        it++;
    }
}
