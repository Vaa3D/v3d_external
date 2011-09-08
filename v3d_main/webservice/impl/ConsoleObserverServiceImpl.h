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
    virtual ConsoleObserverService *copy();

signals:
    void ontologySelected(long rootId);
    void ontologyChanged(long rootId);
    void entitySelected(long entityId);
    void entityViewRequested(long entityId);
    void annotationsChanged(long entityId);

private:
    bool _running;
    int _port;
    QString *_errorMessage;

};

}

#endif // CONSOLEOBSERVERSERVICEIMPL_H
