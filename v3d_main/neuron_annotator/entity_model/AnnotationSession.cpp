#include "AnnotationSession.h"

AnnotationSession::AnnotationSession()
{
}

AnnotationSession::~AnnotationSession()
{
    if (sessionId != 0) delete sessionId;
    if (name != 0) delete name;
    if (owner != 0) delete owner;
}

