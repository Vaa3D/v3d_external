#include "CheckManager.h"

CheckManager* CheckManager::uniqueInstance = 0;

QStringList CheckManager::getcachefiles()
{

    QDir dir(path);
    QStringList nameFilters;
    nameFilters <<"*.v3dpbd";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    return files;
}

void CheckManager::init_imgs()
{
    for(int i=0;i<file_count;i++){
//        ArborInfo img_temp;
//        QString img_loc=cache_files[i].split(".")[0];
//        imgs_managed.insert(img_loc);
//        img_temp.arborid=img_loc.split("_")[0].toInt();
//        img_temp.x=img_loc.split("_")[1].toLong();
//        img_temp.y=img_loc.split("_")[2].toLong();
//        img_temp.z=img_loc.split("_")[3].toLong();
//        img_temp.ab_path=path+cache_files[i];
////        QFileInfo f(path+cache_files[i]);
////        img_temp.birthtime=f.birthTime().toString();
//        imgs.emplace_back(img_temp);
//        imgs[i].print();
    }
}

void CheckManager::addimgs()
{

}

void CheckManager::deleteimgs()
{

}

void CheckManager::update()
{
    if(file_count<20){
        addimgs();
    }else if(file_count>20){
        deleteimgs();
    }
}

bool CheckManager::isExist(QString img_name)
{
    if(imgs_managed.find(img_name)!=imgs_managed.end())
        return true;
    else
        return false;
}

QStringList CheckManager::validimg(int n)
{
    QStringList result;
    for(int i=0;i<n;i++){
        result.emplace_back(imgs[i].ab_path);
    }
    return result;
}

void CheckManager::setLocXYZ(int id, QString image, int x, int y, int z)
{
    XYZ loc((float)x, (float)y, (float)z);
    xyzForLoc = loc;
    this->userid = id;
    this->brainId = image;
}

void CheckManager::setPotentialLocation(QString imageID, QString RES)
{
    resMap.insert(imageID, RES);
}

void CheckManager::downloadImage()
{
    QString res = resMap[this->brainId];
    qDebug() << "input download para, [brainId]:" << this->brainId << "[res]:" << res << "[x, y, z]" << (int)xyzForLoc.x <<"," << (int)xyzForLoc.y<< "," << (int)xyzForLoc.z;
    httpUtilsDownload->downLoadImage(this->brainId, res, (int)(xyzForLoc.x / pow(2, resIndex-1)), (int)(xyzForLoc.y / pow(2, resIndex-1)), (int)(xyzForLoc.z / pow(2, resIndex-1)), DEFAULT_IMAGE_SIZE);
}

void CheckManager::getPotentialLoaction()
{
    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());
    httpGetLocation->getPotentialLoaction(userInfo);
}

void CheckManager::getBrainList()
{
    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());
    httpGetBrainList->getBrainList(userInfo);
}

void CheckManager::setmainwidget(CheckWidget *mw)
{
    if(mw){
        mainwidget=mw;
        connect(mainwidget,SIGNAL(getPotential()),this,SLOT(getPotentialLoaction()));
    }
}

CheckManager::~CheckManager()
{

}
