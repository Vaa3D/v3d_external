#include "../basic_c_fun/v3d_interface.h"
#include "../v3d/v3d_version_info.h"
#include <iostream>
#include <QDir>
#include <QString>
#include <limits>

using namespace std;

void usage(const char* argv[]) {
    cerr << "Usage:" << endl;
    cerr << "    % " << argv[0] << " (plugins directory) >> plugin_versions.xml" << endl;
}

void parsePluginVersion(QObject *plugin, QString fileName)
{
    float version = std::numeric_limits<float>::quiet_NaN();

    if (qobject_cast<V3DPluginInterface2_1 *>(plugin))
    {
        V3DPluginInterface2_1 *iface =
                qobject_cast<V3DPluginInterface2_1 *>(plugin);
        version = iface->getPluginVersion();
    }
    else if (qobject_cast<V3DSingleImageInterface2_1 *>(plugin))
    {
        V3DSingleImageInterface2_1 *iface =
                qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
        version = iface->getPluginVersion();
    }
    else
    {
        // Plugin does not have version-aware interface...
    }

    if (version == version) 
    { // i.e. not NaN
        // Here is where we produce the program output
        cout << "    "; // indent
        cout << "<v3d_plugin";
        cout << " name=\"" << fileName.toStdString() << "\"";
        cout << " version=\"" << version << "\"";
        cout << " platform=\"" << BUILD_OS_INFO << "\"";
        cout << "/>" << endl;
    }

    return;
}

// Adapted from method in v3d_plugin_loader.cpp
void searchPluginFiles(const QDir& pluginsDir)
{
    QStringList fileList = pluginsDir.entryList(QDir::Files);
    foreach (QString fileName, fileList)
    {
        QString fullpath = pluginsDir.absoluteFilePath(fileName);
        QPluginLoader* loader = new QPluginLoader(fullpath);
        if (! loader) return;
        QObject *plugin = loader->instance(); //a new instance
        if (plugin) parsePluginVersion(plugin, fileName);
        loader->unload();
    }
}

void searchPluginDirs(const QDir& pluginsDir)
{
    QStringList dirList = pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString dirName, dirList)
    {
        QDir subDir = pluginsDir;
        subDir.cd(dirName);
        searchPluginDirs(subDir);
        searchPluginFiles(subDir);
    }
}

int main(int argc, const char* argv[])
{
    // Need to have directory argument
    if (argc < 2)
    {
        usage(argv);
        exit(1);
    }

    // First argument is directory name
    QDir pluginDir(argv[1]);
    if (! pluginDir.exists())
    {
        cerr << "ERROR: Directory '" << pluginDir.path().toStdString() << "' does not exist." << endl;
        exit(1);
    }

    searchPluginDirs(pluginDir);

    return 0;
}
