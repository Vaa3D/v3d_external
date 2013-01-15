
/*





 */



#include "VolumePatternIndex.h"
#include "../utility/ImageLoader.h"

const int VolumePatternIndex::MODE_UNDEFINED=-1;
const int VolumePatternIndex::MODE_INDEX=0;
const int VolumePatternIndex::MODE_SEARCH=1;

class SleepThread : QThread {
public:
    SleepThread() {}
    void msleep(int miliseconds) {
        QThread::msleep(miliseconds);
    }
};

VolumePatternIndex::VolumePatternIndex()
{
    mode=MODE_UNDEFINED;
    fastSearch=false;
}

VolumePatternIndex::~VolumePatternIndex()
{
}

bool VolumePatternIndex::execute()
{
    if (mode==MODE_UNDEFINED) {
        return false;
    } else if (mode==MODE_INDEX) {
        return createIndex();
    } else if (mode==MODE_SEARCH) {
        return doSearch();
    }
    return false;
}

int VolumePatternIndex::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-mode") {
            modeString=(*argList)[++i];
        } else if (arg=="-inputList") {
            inputFileList=(*argList)[++i];
        } else if (arg=="-outputIndex") {
            outputIndexFile=(*argList)[++i];
        } else if (arg=="-subVolume") {
            subVolumeString=(*argList)[++i];
        } else if (arg=="-query") {
            queryImageFile=(*argList)[++i];
        } else if (arg=="-maxHits") {
            QString maxHitsString=(*argList)[++i];
            maxHits=maxHitsString.toInt();
        } else if (arg=="-fast") {
            fastSearch=true;
        }
    }
    if (modeString.size()>0) {
        if (modeString=="index") {
            mode=MODE_INDEX;
        } else if (modeString=="search") {
            mode=MODE_SEARCH;
        }
    }
    return 0;
}

bool VolumePatternIndex::createSubVolume() {
    qDebug() << "createSubVolume: subVolume=" << subVolumeString;
    return true;
}

bool VolumePatternIndex::createIndex()
{
    qDebug() << "createIndex() start";
}

bool VolumePatternIndex::doSearch()
{
    qDebug() << "doSearch() start";
}
