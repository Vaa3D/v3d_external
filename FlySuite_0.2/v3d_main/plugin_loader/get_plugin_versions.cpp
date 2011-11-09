// "get_plugin_versions" is a command line tool to populate
// V3D plugin versions for use by the automatic updater.

#include "../basic_c_fun/v3d_interface.h"
#include "../v3d/v3d_version_info.h"
#include <iostream>
#include <QDir>
#include <QString>
#include <QUrl>
#include <limits>

using namespace std;

namespace v3d {

class PluginVersionFinder
{
public:
    void findPlugins(const QDir& pluginDir, const QUrl& pluginUrl)
    {
        if (! pluginDir.exists())
        {
            cerr << "ERROR: Directory '" << pluginDir.path().toStdString() << "' does not exist." << endl;
            exit(1);
        }

        if (!pluginUrl.isValid())
        {
            cerr << "ERROR: URL '" << pluginUrl.toString().toStdString() << "' is invalid." << endl;
            exit(1);
        }

        searchPluginDirs(pluginDir, pluginUrl, QString(""));
    }

private:
    void parsePluginVersion(
            QObject *plugin,
            const QString& fileName,
            const QUrl& baseUrl,
            const QString& relativeUrl)
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
            QUrl fileUrl = baseUrl.resolved(relativeUrl).resolved(fileName);

            // Here is where we produce the program output
            const std::string indent("    ");
            cout << indent << "<v3d_plugin" << endl;
            cout << indent << "    name=\"" << fileName.toStdString() << "\"" << endl;
            cout << indent << "    dir=\"" << relativeUrl.toStdString() << "\"" << endl;
            cout << indent << "    version=\"" << version << "\"" << endl;
            cout << indent << "    platform=\"" << BUILD_OS_INFO << "\"" << endl;
            cout << indent << "    href=\"" << fileUrl.toString().toStdString() << "\"" << endl;
            // TODO - need directory to put plugin into
            cout << indent << "/>" << endl;
        }

        return;
    }

    // Adapted from method in v3d_plugin_loader.cpp
    void searchPluginFiles(const QDir& pluginsDir, const QUrl& baseUrl, const QString& relativeUrl)
    {
        QStringList fileList = pluginsDir.entryList(QDir::Files);
        foreach (QString fileName, fileList)
        {
            QString fileUrl = relativeUrl;
            if (relativeUrl.length() > 0) // no slash if empty
                fileUrl += "/";
            fileUrl += fileName;
            QString fullpath = pluginsDir.absoluteFilePath(fileName);
            QPluginLoader* loader = new QPluginLoader(fullpath);
            if (! loader) return;
            QObject *plugin = loader->instance(); //a new instance
            if (plugin) {
                parsePluginVersion(plugin, fileName, baseUrl, relativeUrl);
            }
            loader->unload();
        }
    }

    void searchPluginDirs(const QDir& pluginsDir, const QUrl& baseUrl, const QString& relativeUrl)
    {
        QStringList dirList = pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString dirName, dirList)
        {
            QDir subDir = pluginsDir;
            subDir.cd(dirName);
            QString subUrl = relativeUrl;
            if (relativeUrl.length() > 0) // no slash if empty
                subUrl += "/";
            subUrl += dirName;
            searchPluginDirs(subDir, baseUrl, subUrl);
            searchPluginFiles(subDir, baseUrl, subUrl);
        }
    }
}; // PluginVersionFinder

} // namespace v3d

void usage(const char* argv[]) {
    cerr << "Usage:" << endl;
    cerr << "    % " << argv[0] << " (plugins directory) (http://url.path.to/plugins/directory) >> plugin_versions.xml" << endl;
}

int main(int argc, const char* argv[])
{
    // Need to have directory and url arguments
    if (argc < 3)
    {
        usage(argv);
        exit(1);
    }

    // First argument is directory name
    QDir pluginDir(argv[1]);
    // Second argument is URL corresponding to plugin directory
    QUrl pluginUrl(argv[2]);

    v3d::PluginVersionFinder finder;
    finder.findPlugins(pluginDir, pluginUrl);

    return 0;
}
