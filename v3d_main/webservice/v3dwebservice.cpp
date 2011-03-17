/**
 *	v3dwebservice.cpp
 *	developed by Yang Yu, March 13, 2011
 */

#include "v3dwebserver.nsmap" //ws namespace
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

soapv3dwsService::soapv3dwsService(MainWindow *pMW)
{
	pMainWin = pMW;
	
	v3dwebserverService_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
	
	connect(this, SIGNAL(wsRequests()), pMainWin, SLOT(updateWebService(soappara*)), Qt::QueuedConnection);
}

soapv3dwsService::~soapv3dwsService()
{
}

int soapv3dwsService::helloworld(char *name, char **response) // hello world
{
	QString qres = QString("Hello ").append(name);
	
	*response = (char *)qres.toStdString().c_str();
	
	//QMessageBox::information((QWidget *)0, QString("title: hello"), QString(*response));
	
	if(this->getMainWin())
	{
		soappara *pSoapParaInput = new soappara;
		
		pSoapParaInput->str_func = "helloworld";
		pSoapParaInput->str_message = (char *)malloc(strlen(*response) + 1);
		strcpy(pSoapParaInput->str_message, *response);
		
		printf("trigger a signal here ...\n");
		//emit wsRequests();
		this->getMainWin()->updateWebService( pSoapParaInput );
	}
	else
	{
		printf("The pointer pMainWin does not passed correctly.\n");
		return -1;
	}
	
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
	QString qres = QString(fn);
	
	*v3dfn = (char *)qres.toStdString().c_str();
	
	if(this->getMainWin())
	{
		soappara *pSoapParaInput = new soappara;
		
		pSoapParaInput->str_func = "v3dopenfile";
		pSoapParaInput->str_message = (char *)malloc(strlen(*v3dfn) + 1);
		strcpy(pSoapParaInput->str_message, *v3dfn);
		
		printf("trigger a signal here ...\n");
		//emit wsRequests();
		this->getMainWin()->updateWebService( pSoapParaInput );
	}
	else
	{
		printf("The pointer pMainWin does not passed correctly.\n");
		return -1;
	}
	
	return SOAP_OK;
	
}

void soapv3dwsService::setMainWin(MainWindow *pMW)
{
	pMainWin = pMW;
}

MainWindow* soapv3dwsService::getMainWin()
{
	return pMainWin;
}

soapv3dwsService* soapv3dwsService::soapv3dwsService::copy()
{	
	soapv3dwsService *dup = SOAP_NEW_COPY(soapv3dwsService(*(struct soap*)this));
	
	dup->setMainWin( this->getMainWin() );
	
	return dup;
}

void soapv3dwsService::run()
{	
	this->serve();
}

/**
 *
 *	Main Web Service Class
 *
 */


V3DWebService::V3DWebService(MainWindow *pMW, int nPort) // preferred class init
{
	pMainWin = pMW;
	port = nPort;
	
	stopped = false;
	
	init();
}


V3DWebService::V3DWebService()
{
	pMainWin = NULL;
	
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
	// init a tmp instance
	soapv3dwsService tmp(pMainWin);
	
	webserver = (void *)(tmp.copy());
	
	// init
	((soapv3dwsService *)webserver)->send_timeout = 60; // 60 seconds
	((soapv3dwsService *)webserver)->recv_timeout = 60; // 60 seconds
	((soapv3dwsService *)webserver)->accept_timeout = TIMEOUT; // server stops after TIMEOUT of inactivity
	((soapv3dwsService *)webserver)->max_keep_alive = 100; // max keep-alive sequence
	
	//
	master = ((soapv3dwsService *)webserver)->bind(NULL, port, BACKLOG);
	if (!soap_valid_socket(master))
	{
		((soapv3dwsService *)webserver)->soap_stream_fault(std::cerr);
		exit(1);
	}
	printf("Socket connection successful %d\n", master);
}

void V3DWebService::run()
{
	//
	soapv3dwsService *t_webserver = new soapv3dwsService(pMainWin);
	
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
			slave = ((soapv3dwsService *)webserver)->accept(); 
			if (!soap_valid_socket(slave))
			{
				if (((soapv3dwsService *)webserver)->error)
				{
					((soapv3dwsService *)webserver)->soap_stream_fault(std::cerr);
					exit(1);
				}
				fprintf(stderr, "server timed out\n");
				break;
			}
			printf("Socket %d connection from IP %d.%d.%d.%d\n", slave, (((soapv3dwsService *)webserver)->ip >> 24)&0xFF, (((soapv3dwsService *)webserver)->ip >> 16)&0xFF, (((soapv3dwsService *)webserver)->ip >> 8)&0xFF, ((soapv3dwsService *)webserver)->ip&0xFF);
			t_webserver = ((soapv3dwsService *)webserver)->copy(); // make a safe copy
			
			if (!t_webserver)
				break;
			connect(t_webserver, SIGNAL(finished()), t_webserver, SLOT(deleteLater()));
			t_webserver->start();
		}
		
		mutex.unlock();	
	}
	
	//
	return;
}

