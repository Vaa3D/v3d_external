#ifndef TIMEBASEDIDENTIFIERGENERATOR_H
#define TIMEBASEDIDENTIFIERGENERATOR_H

#include <QDateTime>

class TimebasedIdentifierGenerator
{
public:
    TimebasedIdentifierGenerator();

    static long getSingleId() {
        long currentTimeId=QDateTime::currentMSecsSinceEpoch();
        if (currentTimeId<=lastId) {
            currentTimeId=lastId+1;
            lastId=currentTimeId;
        }
        return currentTimeId;
    }

private:
    static long lastId;
};

#endif // TIMEBASEDIDENTIFIERGENERATOR_H
