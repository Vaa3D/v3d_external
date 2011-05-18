/**
 *	v3dwebservice.cpp
 *	developed by Yang Yu, March 13, 2011
 */

#include "soapdep/v3dwebserver.nsmap" //ws namespace
#include "v3dwebservice.hpp"

/// Web service operation 'helloworld' (returns error code or SOAP_OK)
int v3dwebserverService::helloworld(char *name, char **response)
{
	return SOAP_OK;
}

/// Web service operation 'msghandler' (returns error code or SOAP_OK)
int v3dwebserverService::msghandler(ns__LOAD_MSG *lm, ns__LOAD_RES *lr)
{
	return SOAP_OK;
}

/// Web service operation 'v3dopenfile' (returns error code or SOAP_OK)
int v3dwebserverService::v3dopenfile(char *fn, char **v3dfn)
{
	return SOAP_OK;
}

/**
 *
 *	Server dummy methods to avoid link errors
 *
 */

int ns__helloworldResponse_(struct soap *soap, char *name, char **response)
{
  return SOAP_NO_METHOD;
}

int ns__msghandlerResponse_(struct soap *soap, ns__LOAD_MSG *lm, ns__LOAD_RES *lr)
{
	return SOAP_NO_METHOD;
}

int ns__v3dopenfileResponse_(struct soap *soap, char *fn)
{
  return SOAP_NO_METHOD;
}

/**
 * child class of soapv3dwebserviceService
 *
 */

soapv3dwsService::soapv3dwsService()
{
	v3dwebserverService_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
	
	try
	{
		pSoapPara = new soappara;
	}
	catch (...)
	{
		printf("Fail to init soap message pointer.\n");
		return;
	}
}

soapv3dwsService::~soapv3dwsService()
{
}

soappara* soapv3dwsService::getSoapPara()
{
	return pSoapPara;
}

void soapv3dwsService::setSoapPara(soappara *pSoapParaInput)
{
	pSoapPara = pSoapParaInput;
}

int soapv3dwsService::helloworld(char *name, char **response) // hello world
{
	QString qres = QString("Hello ").append(name);
	
	*response = (char *)qres.toStdString().c_str();
	
	pSoapPara->str_func = "helloworld";
	pSoapPara->str_message = (char *)malloc(strlen(*response) + 1);
	strcpy(pSoapPara->str_message, *response);
	
	printf("trigger a signal here in soapv3dwsService ...\n");
	emit wsRequests();
	
	fprintf(stderr, "%s\n", *response);
	
	return SOAP_OK;
}

int soapv3dwsService::msghandler(ns__LOAD_MSG *lm, ns__LOAD_RES *lr) // message resquest and response
{
	lr->loadtime = 1; // loading time
	
	int sz_unit = 0; // compute voxel size	
	switch (lm->dt) 
	{
		case UINT8:
			sz_unit = sizeof(unsigned char);
			break;
			
		case UINT16:
			sz_unit = sizeof(unsigned short);
			break;
			
		case FLOAT32:
			sz_unit = sizeof(float);
			break;
			
		default:
			break;
	}
	
	
	lr->memorysize = lm->x*lm->y*lm->z*lm->c*lm->t*sz_unit; // memory size
	
	fprintf(stderr, ">>loading time -- %f \n", lr->loadtime);
	fprintf(stderr, ">>image size in memory -- %ld \n", lr->memorysize);
	
	return SOAP_OK;
}

int soapv3dwsService::v3dopenfile(char *fn, char **v3dfn) // open a file in V3D
{
	printf("v3d open file ...\n");
	
	QString qres = QString(fn);
	
	*v3dfn = (char *)qres.toStdString().c_str();
	
	pSoapPara->str_func = "v3dopenfile";
	pSoapPara->str_message = (char *)malloc(strlen(*v3dfn) + 1);
	strcpy(pSoapPara->str_message, *v3dfn);
	
	printf("trigger a signal here in soapv3dwsService ...\n");
	emit wsRequests();
	
	return SOAP_OK;
	
}

soapv3dwsService* soapv3dwsService::soapv3dwsService::copy()
{	
	soapv3dwsService *dup = SOAP_NEW_COPY(soapv3dwsService(*(struct soap*)this));
	
	dup->setSoapPara(this->getSoapPara());
	
	return dup;
}

void soapv3dwsService::run()
{	
	mutex.lock();
	this->serve();
	mutex.unlock();
}

/**
 *
 *	Main Web Service Class
 *
 */

V3DWebService::V3DWebService(int nPort) // preferred class init
{
	port = nPort;
	
	stopped = false;
	
	init();
}


V3DWebService::V3DWebService()
{
	if (!port)
		port = 9125; // define port here
	
	stopped = false;
	
	init();
}

V3DWebService::~V3DWebService()
{
}

void V3DWebService::init()
{
	// init
	webserver = new soapv3dwsService();
	
	webserver->send_timeout = 60; // 60 seconds
	webserver->recv_timeout = 60; // 60 seconds
	webserver->accept_timeout = TIMEOUT; // server stops after TIMEOUT of inactivity
	webserver->max_keep_alive = 100; // max keep-alive sequence
	
	// bind a port
	master = webserver->bind(NULL, port, BACKLOG);
	if (!soap_valid_socket(master))
	{
		webserver->soap_stream_fault(std::cerr);
		exit(1);
	}
	printf("Socket connection successful %d\n", master);
	
	//connect(webserver, SIGNAL(wsRequests()), this, SLOT(webserviceResponse()) );
}

void V3DWebService::webserviceResponse()
{
	printf("trigger a signal here in V3DWebService ...\n");
	emit webserviceRequest();
}

soappara* V3DWebService::getSoapPara()
{
	return pSoapPara;
}

void V3DWebService::setSoapPara(soappara *pSoapParaInput)
{
	pSoapPara = pSoapParaInput;
}

void V3DWebService::run()
{
	//
	mutex.lock();
	
	if(stopped)
	{
		stopped = false;
		mutex.unlock();
	}
	else
	{
		while(!stopped)
		{
			if(!webserver) break;
			
			slave = webserver->accept(); 
			if (!soap_valid_socket(slave))
			{
				if (webserver->error)
				{
					webserver->soap_stream_fault(std::cerr);
					exit(1);
				}
				fprintf(stderr, "server timed out\n");
				break;
			}
			printf("Socket %d connection from IP %d.%d.%d.%d\n", slave, (webserver->ip >> 24)&0xFF, (webserver->ip >> 16)&0xFF, (webserver->ip >> 8)&0xFF, webserver->ip&0xFF);
			
			//this->setSoapPara( webserver->getSoapPara() );
			
//			connect(webserver, SIGNAL(finished()), webserver, SLOT(deleteLater()));
//			webserver->start();
//			connect(webserver, SIGNAL(wsRequests()), this, SLOT(webserviceResponse()) );
			
			printf("start web server thread ...\n");
		
			soapv3dwsService *t_webserver = new soapv3dwsService;
			t_webserver = webserver->copy();
			
			connect(t_webserver, SIGNAL(finished()), t_webserver, SLOT(deleteLater()));
			t_webserver->start();
			
			this->setSoapPara( t_webserver->getSoapPara() );
			connect(t_webserver, SIGNAL(wsRequests()), this, SLOT(webserviceResponse()));
		}
		
		mutex.unlock();	
	}
	
	//
	return;
}

