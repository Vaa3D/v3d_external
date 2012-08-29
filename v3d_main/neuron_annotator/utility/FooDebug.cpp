#include "FooDebug.h"

/* slot */
void FooDebugWriter::printMessage(QString message)
{
    QMutexLocker locker(&mutex);
    std::cout << message.toStdString() << std::endl;
}


//////////////////

FooDebugString::FooDebugString()
{
    connect(this, SIGNAL(messageCompleted(QString)),
            &FooDebugWriter::debugWriter, SLOT(printMessage(QString)));
}

void FooDebugString::emitMessage()
{
    if (messageString.length() > 0)
        emit messageCompleted(messageString);
}

QString& FooDebugString::string()
{
    return messageString;
}


//////////////////

FooDebug::FooDebug()
    : fooDebugString(new FooDebugString())
    , qdebug(&fooDebugString->string())
{
}

FooDebug::~FooDebug()
{
    fooDebugString->emitMessage();
}

FooDebug & FooDebug::operator<< ( QChar t ) {qdebug << t; return *this;}
FooDebug & FooDebug::operator<< ( bool t ) {qdebug << t; return *this;}
FooDebug & FooDebug::operator<< ( char t ) {qdebug << t; return *this;}
FooDebug & FooDebug::operator<< ( signed short i ) {qdebug << i; return *this;}
FooDebug & FooDebug::operator<< ( unsigned short i ) {qdebug << i; return *this;}
FooDebug & FooDebug::operator<< ( signed int i ) {qdebug << i; return *this;}
FooDebug & FooDebug::operator<< ( unsigned int i ) {qdebug << i; return *this;}
FooDebug & FooDebug::operator<< ( signed long l ) {qdebug << l; return *this;}
FooDebug & FooDebug::operator<< ( unsigned long l ) {qdebug << l; return *this;}
FooDebug & FooDebug::operator<< ( qint64 i ) {qdebug << i; return *this;}
FooDebug & FooDebug::operator<< ( quint64 i ) {qdebug << i; return *this;}
FooDebug & FooDebug::operator<< ( float f ) {qdebug << f; return *this;}
FooDebug & FooDebug::operator<< ( double f ) {qdebug << f; return *this;}
FooDebug & FooDebug::operator<< ( const char * s ) {qdebug << s; return *this;}
FooDebug & FooDebug::operator<< ( const QString & s ) {qdebug << s; return *this;}
FooDebug & FooDebug::operator<< ( const QStringRef & s ) {qdebug << s; return *this;}
FooDebug & FooDebug::operator<< ( const QLatin1String & s ) {qdebug << s; return *this;}
FooDebug & FooDebug::operator<< ( const QByteArray & b ) {qdebug << b; return *this;}
FooDebug & FooDebug::operator<< ( const void * p ) {qdebug << p; return *this;}

////////////////////

FooDebug fooDebug() {
    return FooDebug();
}

QMutex FooDebugWriter::mutex;
FooDebugWriter FooDebugWriter::debugWriter;


