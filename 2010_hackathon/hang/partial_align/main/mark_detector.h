#ifndef __MARK_DETECT_H_
#define __MARK_DETECT_H_

#include "v3d_basicdatatype.h"
#include "mark_io.h"
#include <list>

// similary to readMarker_file function
bool detect_mark(std::list<MyImageMarker> & marklist, const unsigned char* inimg1d, V3DLONG sz[3]);

#endif
