#include "ConsoleObserverServiceImpl.h"
#include "../console/obsConsoleObserverService.h"
#include "../console/cdsConsoleDataServiceProxy.h"
#include <QtGui>
#include <sstream>

#define CLIENT_NAME "NeuronAnnotator"

namespace obs {

ConsoleObserverServiceImpl::ConsoleObserverServiceImpl(QObject *parent) :
    QThread(parent),
    _port(-1),
    _errorMessage(NULL),
    _running(false)
{
    cds::ConsoleDataServiceProxy proxy;
    cds::fw__reservePortResponse reservePortResponse;
    if (proxy.reservePort(NULL, NULL, CLIENT_NAME, reservePortResponse) != SOAP_OK) {
        _errorMessage = new QString(proxy.soap_fault_string());
    }
    else {
        _port = reservePortResponse.return_;
    }

    QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

ConsoleObserverServiceImpl::~ConsoleObserverServiceImpl() {
    qDebug() << "delete ConsoleObserverServiceImpl on port"<<_port;
    if (_errorMessage!=0) delete _errorMessage;
}

// Reimplemented run(port) to support stopping the thread
void ConsoleObserverServiceImpl::run()
{
    _running = true;
    for (;;)
    {
        accept_timeout = CONSOLE_OBSERVER_ACCEPT_TIMEOUT;
        if (!soap_valid_socket(accept()))
        {
            if (errnum != 0) {
                qDebug() << "Error accepting connection, code:" << error << "errnum:"<<errnum;
            }
        }
        else {
            (void)serve();
            soap_destroy(this);
            soap_end(this);
        }
        if (!_running) {
            return;
        }
    }
}

// Like start() but returns without starting the thread if its not able to bind to the port
void ConsoleObserverServiceImpl::startServer()
{
    if (soap_valid_socket(bind(NULL, _port, 100)))
    {
        start(QThread::NormalPriority);
    }
}

// Asyncronously tell the server to stop. It will die on its next accept loop.
void ConsoleObserverServiceImpl::stopServer()
{
    _running = false;
}

void ConsoleObserverServiceImpl::registerWithConsole()
{
    if (_port < 0) return;

    std::stringstream ss;
    std::string endpointUrl;
    ss << "http://localhost:" << _port;
    ss >> endpointUrl;

    cds::ConsoleDataServiceProxy proxy;
    cds::fw__registerClientResponse registerClientResponse;
    if (proxy.registerClient(NULL, NULL, _port, endpointUrl, registerClientResponse) != SOAP_OK) {
        _errorMessage = new QString(proxy.soap_fault_string());
    }
}

int ConsoleObserverServiceImpl::ontologySelected(LONG64 rootId, struct fw__ontologySelectedResponse &response)
{
    emit ontologySelected(rootId);
    return SOAP_OK;
}

int ConsoleObserverServiceImpl::entitySelected(LONG64 entityId, struct fw__entitySelectedResponse &response)
{
    emit entitySelected(entityId);
    return SOAP_OK;
}

int ConsoleObserverServiceImpl::entityViewRequested(LONG64 entityId, struct fw__entityViewRequestedResponse &response) {
    emit entityViewRequested(entityId);
    return SOAP_OK;
}

int ConsoleObserverServiceImpl::annotationsChanged(LONG64 entityId, struct fw__annotationsChangedResponse &response)
{
    emit annotationsChanged(entityId);
    return SOAP_OK;
}

ConsoleObserverService *ConsoleObserverServiceImpl::copy()
{
    // Would need to implement the other constructors to do this, but who needs it anyway?
    return NULL;
}

}

