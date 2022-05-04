#include "httputilsuser.h"

#include <QJsonObject>

HttpUtilsUser::HttpUtilsUser()
{

}

void HttpUtilsUser::loginWithHttp(QJsonObject userInfo)
{
    QJsonObject body;
    body.insert("user", userInfo);
    asyncPostRequest(URL_LOGIN, body);
}


