#include "infocache.h"


void InfoCache::setAccount(QString account)
{
//    InfoCache& instance = InfoCache::getInstance();
//    instance.account = account;
    InfoCache::getInstance().account = account;
}

void InfoCache::setToken(QString token)
{
//    InfoCache& instance = InfoCache::getInstance();
//    instance.token = token;
    InfoCache::getInstance().token = token;
}
