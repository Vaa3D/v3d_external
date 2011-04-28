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
	
	ns__V3DMSG *v3dmessage;
	
	ns__V3DMSG_ROTATION *v3dmsgrot;
	ns__V3DMSG_ZOOM *v3dmsgzoom;
	ns__V3DMSG_SHIFT *v3dmsgshift;
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
	
	virtual int v3dopenfile3d(ns__V3DMSG *input, ns__V3DMSG *output); // open a file in V3D and set 3dview position
	
	virtual int v3dopenfile3dwrot(ns__V3DMSG_ROTATION *input, ns__V3DMSG_ROTATION *output); // open a file in V3D and set 3dview rotation
	
	virtual int v3dopenfile3dwzoom(ns__V3DMSG_ZOOM *input, ns__V3DMSG_ZOOM *output); // open a file in V3D and set 3dview zoom
	
	virtual int v3dopenfile3dwshift(ns__V3DMSG_SHIFT *input, ns__V3DMSG_SHIFT *output); // open a file in V3D and set 3dview shift
	
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

/**
 *
 *	Response Handler Class through plugin interface
 *
 */

class ResponseHandler : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
	//QStringList menulist() const;
	//void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				V3DPluginCallback2 & v3d, QWidget * parent) {return true;}
	float getPluginVersion() const {return 1.01f;} // version info 
};


#endif // __V3DWEBSERVICE_HPP__
