#ifndef ANNOTATIONSESSION_H
#define ANNOTATIONSESSION_H

#include <QtCore>

class AnnotationSession
{
public:
    AnnotationSession();
    ~AnnotationSession();
public:
    qint64 *sessionId;
    QString *name;
    QString *owner;
};

#endif // ANNOTATIONSESSION_H
