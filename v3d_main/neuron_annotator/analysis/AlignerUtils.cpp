#include "AlignerUtils.h"
#include "../utility/ImageLoader.h"

const int AlignerUtils::MODE_UNDEFINED=-1;
const int AlignerUtils::MODE_SUBVOLUME=0;

#include "../neuron_annotator/analysis/SleepThread.h" //added by PHC, 20130521, to avoid a linking error on Windows
/*  //commented by PHC, 20130521, to avoid a linking error on Windows
class SleepThread : QThread {
public:
    SleepThread() {}
    void msleep(int miliseconds) {
        QThread::msleep(miliseconds);
    }
};
*/

AlignerUtils::AlignerUtils()
{
    mode=MODE_UNDEFINED;
}

AlignerUtils::~AlignerUtils()
{
}

bool AlignerUtils::execute()
{
    if (mode==MODE_UNDEFINED) {
        return false;
    } else if (mode==MODE_SUBVOLUME) {
        return createSubVolume();
    }
    return false;
}

int AlignerUtils::processArgs(vector<char*> *argList)
{
    for (int i=0;i<argList->size();i++) {
        QString arg=(*argList)[i];
        if (arg=="-sourceStack") {
            sourceStackFilepath=(*argList)[++i];
        } else if (arg=="-subVolume") {
            subVolumeString=(*argList)[++i];
        } else if (arg=="-outputStack") {
            outputStackFilepath=(*argList)[++i];
        }
    }
    if (subVolumeString.size()>0) {
        mode=MODE_SUBVOLUME;
    }
    return 0;
}

bool AlignerUtils::createSubVolume() {
    qDebug() << "createSubVolume: subVolume=" << subVolumeString;
	return true;
}
