
#ifndef __IO_BIOFORMATS_H__
#define __IO_BIOFORMATS_H__

#include <QString>

QString getAppPath();

bool call_bioformats_io(QString infilename, QString & outfilename);

#endif

