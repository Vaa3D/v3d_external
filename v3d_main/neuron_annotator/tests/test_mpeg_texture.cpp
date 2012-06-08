
#include "../utility/loadV3dFFMpeg.h"
#include "../utility/MpegTexture.h"
#include <QCoreApplication>

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 2) {
        cerr << "Video file name not specified" << endl;
        return 1;
    }

    QCoreApplication theApp(argc, argv);

    MpegTexture mpegTexture;
    mpegTexture.loadFile(argv[1]);

    qDebug() << "starting app";
    return theApp.exec();
}

