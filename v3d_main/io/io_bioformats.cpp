#include "io_bioformats.h"
#include "../basic_c_fun/v3d_message.h"

#include "version_control.h"
#if defined(USE_Qt5)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

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
    QString tmpfilePath = setting.value("bioformats_tmpfile_path").toByteArray();
    if (!(tmpfilePath.endsWith("v3draw")))
        tmpfilePath = ""; //reset the file name
    v3d_msg(QString("The last saved name of the tmp file or the reset name is [%1]\n").arg(tmpfilePath),0);


    if(infilename.isEmpty())
    {
        printf("\nError: Your image does not exist!\n");
        return false;
    }

    QString baseName = QFileInfo(infilename).baseName();

//    if (!QFile(tmpfilePath).exists())
//    {
//        tmpfilePath = QDir::tempPath().append("/").append("test1.v3draw");
//    }

    //test if the tmp folder is useable
    QFileInfo qtf(tmpfilePath);
    QFile tf(tmpfilePath);
    if (!(tf.open(QIODevice::ReadWrite))) //the test fail
    {
        v3d_msg(QString("The temporary folder [%1] for holding the intermediate file is not useable. "
                        "Please specify a folder that you can read/write and have enough space for "
                        "storing the intermediate file for using Bioformats library.\n").arg(qtf.dir().absolutePath()));
        QString tmpfilePathFolder = QFileDialog::getExistingDirectory(0, "Open Directory",
                                                              QDir::tempPath(),
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if(tmpfilePathFolder.isEmpty())
        {
            return false;
        }

        tmpfilePath = tmpfilePathFolder.append("/").append("test1.v3draw");
//        tmpfilePath = tmpfilePathFolder.append("/").append("test1.tif");
        v3d_msg(QString("The name of the newly selected tmp file is [%1]\n").arg(tmpfilePath), 0);
        QFileInfo qtf1(tmpfilePath);
        QFile tf1(tmpfilePath);
        if (!(tf1.open(QIODevice::ReadWrite))) //the test fail
        {
            v3d_msg(QString("The temporary folder [%1] is still not valid. Try to rerun and use a different folder.\n").arg(qtf1.dir().absolutePath()));
            return false;
        }
    }

    setting.setValue("bioformats_tmpfile_path", qPrintable(tmpfilePath));

    //

    outfilename = tmpfilePath;
    v3d_msg(QString("The name of the currently used tmp file is [%1]\n").arg(outfilename), 0);

    //need to add platform independent code here
    int res_syscall;
    if (QFile(outfilename).exists())
    {
#if defined(Q_OS_WIN)
        res_syscall = system(qPrintable(QString("del /F \"%1\"").arg(outfilename)));
#else
        res_syscall = system(qPrintable(QString("rm -f \"%1\"").arg(outfilename)));
#endif
    }

    v3d_msg(QString("The system call to delete the existing temp file returns a value [%1]").arg(res_syscall), 0);
    if (res_syscall<0)
    {
        v3d_msg(QString("Fail to delete the tmp file [%1].").arg(outfilename));
        return false;
    }

    //look for loci_tools.jar
    if (!QFile(lociLibPath).exists())
    {
        lociLibPath = getAppPath().append("/").append("loci_tools.jar");
    }

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
        v3d_msg("The temprary converted file does not exist. The conversion of format using Bioformats has failed. Please check you have Java properly installed and/or use another way to convert and load using Vaa3D.\n");
        outfilename = "";
        return false;
    }

    return true;
}


