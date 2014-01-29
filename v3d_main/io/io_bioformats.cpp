#include "io_bioformats.h"
#include "../basic_c_fun/v3d_message.h"
#include <QtGui>

QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    // In a Mac app bundle, plugins directory could be either
    //  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
    //  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}

bool call_bioformats_io(QString infilename, QString & outfilename)
{
    QSettings setting("Vaa3D_tools", "bioformats");
    QString lociLibPath = setting.value("bioformats_binary_path").toByteArray();

    if(infilename.isEmpty())
    {
        printf("\nError: Your image does not exist!\n");
        return false;
    }

    QString baseName = QFileInfo(infilename).baseName();
    outfilename = QDir::tempPath().append("/").append(baseName).append(".tif");

    //need to add platform independent code here
    int res_syscall;
    if (!QFile(outfilename).exists())
    {
#if defined(Q_OS_WIN)
        res_syscall = system(qPrintable(QString("del /F \"%1\"").arg(outfilename)));
#else
        res_syscall = system(qPrintable(QString("rm -f \"%1\"").arg(outfilename)));
#endif
    }

    v3d_msg(QString("The system call to delete the existing temp file returns a value [%1]").arg(res_syscall), 0);
    if (res_syscall<0)
        return false;

    //look for loci_tools.jar
    if (!QFile(lociLibPath).exists())
    {
        v3d_msg(QString("Bioformats Java library is not detected specified correctly at [%1], so please specify the Bioformats Java library location.\n").arg(lociLibPath));
        lociLibPath = QFileDialog::getOpenFileName(0, QObject::tr("select the library of Bioformats Java library (loci_tools.jar)"),
                                               QDir::currentPath(),
                                               QObject::tr("Java Library File (*.jar)"));

        if(lociLibPath.isEmpty())
        {
            return false;
        }

        if (!QFile(lociLibPath).exists())
        {
            v3d_msg("Cannot find loci_tools.jar, please download it and make sure it is put under the Vaa3D executable folder, parallel to the Vaa3D executable and the plugins folder.");
            return false;
        }
    }

    setting.setValue("bioformats_binary_path", qPrintable(lociLibPath));

    QString cmd_loci = QString("java -cp %1 loci.formats.tools.ImageConverter \"%2\" \"%3\"").
            arg(lociLibPath.toStdString().c_str()).
            arg(infilename.toStdString().c_str()).
            arg(outfilename.toStdString().c_str());

    res_syscall = system(qPrintable(cmd_loci));
    v3d_msg(QString("The system call to the Bioformats returns a value [%1]").arg(res_syscall), 0);
    if (res_syscall<0)
        return false;

    if (!QFile(outfilename).exists())
    {
        v3d_msg("The temprary converted file does not exist. The conversion of format using Bioformats has failed. Please use another way to convert and load using Vaa3D.\n");
        outfilename = "";
        return false;
    }

    return true;
}


