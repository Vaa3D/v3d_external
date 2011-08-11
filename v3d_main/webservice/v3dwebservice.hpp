/**
 *	v3dwebservice.hpp
 *	developed by Yang Yu, March 7, 2011
 */

#ifndef __V3DWEBSERVICE_HPP__
#define __V3DWEBSERVICE_HPP__

#include "soapdep/soapH.h"

#include "soapdep/soapv3dwebserverService.h" // server
//#include "soapv3dwebserverProxy.h" // client

#include <string.h>
#include <QString>
#include <QThread>
#include <QMutex>

#include "../basic_c_fun/v3d_interface.h" // plugin interface

#define BACKLOG (100)
#define TIMEOUT (24*60*60) // timeout after 24hrs of inactivity

/**
 *
 *	Static
 *
 */

enum  { V3DWEB_UINT8=1, V3DWEB_UINT16=2, V3DWEB_FLOAT32=4 };

/**
 *
 *	para class
 *
 */
class soappara 
{
public:
	soappara(){}
	~soappara(){}
	
public:
	char *str_func;
	char *str_message; // e.g. fn
	
	float rot_x, rot_y, rot_z;
};

/**
 * web service class using soap
 *
 */

class soapv3dwsService :  public QThread, public v3dwebserverService
{
	Q_OBJECT
	
public:
	soapv3dwsService();
	
	soapv3dwsService(const struct soap &_soap) : v3dwebserverService(_soap){}
	
	~soapv3dwsService();
	
public: // interface SOAP/XML handling
	
	virtual int helloworld(char *name, char **response); // hello world
	
	virtual int msghandler(ns__LOAD_MSG *lm, ns__LOAD_RES *lr); // message resquest and response
	
	virtual int v3dopenfile(char *fn, char **v3dfn); // open a file in V3D
	
	soapv3dwsService *copy();
	
	void run();
	soappara *getSoapPara();
	void setSoapPara(soappara *pSoapParaInput);
	
signals:
	void wsRequests();
	
private:
	QMutex mutex;
	soappara *pSoapPara;
};

/**
 *
 *	Main Web Service Class
 *
 */

class V3DWebService : public QThread
{
	Q_OBJECT

public:
	V3DWebService(int nPort); // preferred class init
	
	V3DWebService();
	
	~V3DWebService();
	
public:
	void init();
	soappara *getSoapPara();
	void setSoapPara(soappara *pSoapParaInput);
	
signals:
	void webserviceRequest();
	
public slots:
	void webserviceResponse();
	
protected:
	void run();
	
public:
	
	SOAP_SOCKET master, slave;
	soapv3dwsService *webserver; // soapv3dwsService class pointer
	int port;
	
	volatile bool stopped; // if true then abort. false by default 
	
private:
	QMutex mutex;
	soappara *pSoapPara;

};

#endif // __V3DWEBSERVICE_HPP__
