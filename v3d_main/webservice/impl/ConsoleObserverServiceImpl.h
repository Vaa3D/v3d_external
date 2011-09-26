#ifndef CONSOLEOBSERVERSERVICEIMPL_H
#define CONSOLEOBSERVERSERVICEIMPL_H

#include <QThread>
#include "../console/obsConsoleObserverService.h"

#define CONSOLE_OBSERVER_ACCEPT_TIMEOUT 30

namespace obs {

class ConsoleObserverServiceImpl : public QThread, public ConsoleObserverService
{
    Q_OBJECT

public:
    explicit ConsoleObserverServiceImpl(QObject *parent = 0);
    ~ConsoleObserverServiceImpl();
    void run();
    void startServer();
    void stopServer();
    void registerWithConsole();
    inline QString * errorMessage() const { return _errorMessage; }
    inline int port() const { return _port; }
    virtual int ontologySelected(LONG64 rootId, struct fw__ontologySelectedResponse &response);
    virtual int ontologyChanged(LONG64 rootId, struct fw__ontologyChangedResponse &response);
    virtual int entitySelected(LONG64 entityId, struct fw__entitySelectedResponse &response);
    virtual int entityViewRequested(LONG64 entityId, struct fw__entityViewRequestedResponse &response);
    virtual int annotationsChanged(LONG64 entityId, struct fw__annotationsChangedResponse &response);
    virtual int sessionSelected(LONG64 sessionId, struct fw__sessionSelectedResponse &response);
    virtual ConsoleObserverService *copy();

signals:
    void ontologySelected(qint64 rootId);
    void ontologyChanged(qint64 rootId);
    void entitySelected(qint64 entityId);
    void entityViewRequested(qint64 entityId);
    void annotationsChanged(qint64 entityId);
    void sessionSelected(qint64 sessionId);

private:
    bool _running;
    int _port;
    QString *_errorMessage;

};

}

#endif // CONSOLEOBSERVERSERVICEIMPL_H
