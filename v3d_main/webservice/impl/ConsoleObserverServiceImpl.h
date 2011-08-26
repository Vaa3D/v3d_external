#ifndef CONSOLEOBSERVERSERVICEIMPL_H
#define CONSOLEOBSERVERSERVICEIMPL_H

#include <QThread>
#include "../console/obsConsoleObserverService.h"

namespace obs {

class ConsoleObserverServiceImpl : public QThread, public ConsoleObserverService
{
    Q_OBJECT

public:
    explicit ConsoleObserverServiceImpl(QObject *parent = 0);
    void run();
    void registerWithConsole();
    virtual int ontologySelected(LONG64 _rootId, struct fw__ontologySelectedResponse &response);
    virtual int entitySelected(LONG64 _entityId, struct fw__entitySelectedResponse &response);
    virtual int entityViewRequested(LONG64 entityId, struct fw__entityViewRequestedResponse &response);
    virtual int annotationsChanged(LONG64 _entityId, struct fw__annotationsChangedResponse &response);
    virtual ConsoleObserverService *copy();

signals:
    void ontologySelected(long rootId);
    void entitySelected(long entityId);
    void entityViewRequested(long entityId);
    void annotationsChanged(long entityId);

private:
    int port;

};

}

#endif // CONSOLEOBSERVERSERVICEIMPL_H
