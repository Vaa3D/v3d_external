/**
 *	v3dwebservice.hpp
 *	developed by Yang Yu, March 7, 2011
 */

#ifndef __V3DWEBSERVICE_HPP__
#define __V3DWEBSERVICE_HPP__

#include "soapH.h"

#include "soapv3dwebserverService.h" // server
//#include "soapv3dwebserverProxy.h" // client

#include <string.h>
#include <QString>
#include <QThread>
#include <QMutex>

#include "../v3d/mainwindow.h"

#define BACKLOG (100)
#define TIMEOUT (24*60*60) // timeout after 24hrs of inactivity

/**
 *
 *	Static
 *
 */

enum  { UINT8=1, UINT16=2, FLOAT32=4 };

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
};

/**
 * child class of soapv3dwebserviceService
 *
 */

class soapv3dwsService :  public QThread, public v3dwebserverService
{
	Q_OBJECT
	
public:
	soapv3dwsService(MainWindow *pMW);
	
	soapv3dwsService(const struct soap &_soap) : v3dwebserverService(_soap){}
	
	~soapv3dwsService();
	
public: // interface SOAP/XML handling
	
	virtual int helloworld(char *name, char **response); // hello world
	
	virtual int msghandler(ns__LOAD_MSG *lm, ns__LOAD_RES *lr); // message resquest and response
	
	virtual int v3dopenfile(char *fn, char **v3dfn); // open a file in V3D
	
	void setMainWin(MainWindow *pMW);
	
	MainWindow *getMainWin();
	
	soapv3dwsService *copy();
	
	void run();
	
signals:
	void wsRequests();
	
private:
	MainWindow *pMainWin;
};

/**
 *
 *	Main Web Service Class
 *
 */

class V3DWebService : public QThread
{

public:
	V3DWebService(MainWindow *pMW, int nPort); // preferred class init
	
	V3DWebService();
	
	~V3DWebService();
	
public:
	void init();
	
protected:
	void run();
	
public:
	MainWindow *pMainWin;
	
	SOAP_SOCKET master, slave;
	void *webserver; // soapv3dwsService class pointer
	int port;
	
	pthread_t tid;
	
	volatile bool stopped; // if true then abort. false by default 
	
private:
	QMutex mutex;

};

#endif // __V3DWEBSERVICE_HPP__
