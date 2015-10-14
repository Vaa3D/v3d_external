#ifndef FOODEBUG_H
#define FOODEBUG_H

#include <QObject>
#include <QMutex>
#include <QDebug>
#include <iostream>


// Class to print out all debug messages in the main thread.
class FooDebugWriter : public QObject
{
    Q_OBJECT

public:
    static QMutex mutex;
    static FooDebugWriter debugWriter;

public slots:
    void printMessage(QString message);
};


class FooDebugString : public QObject
{
    Q_OBJECT

public:
    FooDebugString();
    void emitMessage();
    QString& string();

signals:
    void messageCompleted(QString message);

protected:
    QString messageString;
};


// Attempt at a thread safe qDebug()
class FooDebug
{
public:
    FooDebug();
    ~FooDebug();
    FooDebug & 	operator<< ( QChar t );
    FooDebug & 	operator<< ( bool t );
    FooDebug & 	operator<< ( char t );
    FooDebug & 	operator<< ( signed short i );
    FooDebug & 	operator<< ( unsigned short i );
    FooDebug & 	operator<< ( signed int i );
    FooDebug & 	operator<< ( unsigned int i );
    FooDebug & 	operator<< ( signed long l );
    FooDebug & 	operator<< ( unsigned long l );
    FooDebug & 	operator<< ( qint64 i );
    FooDebug & 	operator<< ( quint64 i );
    FooDebug & 	operator<< ( float f );
    FooDebug & 	operator<< ( double f );
    FooDebug & 	operator<< ( const char * s );
    FooDebug & 	operator<< ( const QString & s );
    FooDebug & 	operator<< ( const QStringRef & s );
    FooDebug & 	operator<< ( const QLatin1String & s );
    FooDebug & 	operator<< ( const QByteArray & b );
    FooDebug & 	operator<< ( const void * p );

protected:
    FooDebugString* fooDebugString;
    QDebug qdebug;
};


// possibly less thread-unsafe replacement for qDebug()
FooDebug fooDebug();





#endif // FOODEBUG_H
