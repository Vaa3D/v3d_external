#include "ConsoleObserverServiceImpl.h"
#include "../console/obsConsoleObserverService.h"
#include "../console/cdsConsoleDataServiceProxy.h"
#include <QtGui>
#include <sstream>

#define CLIENT_NAME "NeuronAnnotator"

namespace obs {

ConsoleObserverServiceImpl::ConsoleObserverServiceImpl(QObject *parent) :
    QThread(parent)
{
    cds::ConsoleDataServiceProxy proxy;
    cds::fw__reservePortResponse reservePortResponse;
    proxy.reservePort(NULL, NULL, CLIENT_NAME, reservePortResponse);
    port = reservePortResponse.return_;

    qDebug() << "Received console approval to run on port "<<port;
}

void ConsoleObserverServiceImpl::run()
{
    ConsoleObserverService::run(port);
}

void ConsoleObserverServiceImpl::registerWithConsole()
{
    std::stringstream ss;
    std::string endpointUrl;
    ss << "http://localhost:" << port;
    ss >> endpointUrl;

    cds::ConsoleDataServiceProxy proxy;
    cds::fw__registerClientResponse registerClientResponse;
    proxy.registerClient(NULL, NULL, port, endpointUrl, registerClientResponse);

    qDebug() << "Registered with console and listening for events";
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

