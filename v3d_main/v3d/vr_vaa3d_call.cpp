#include "vr_vaa3d_call.h"

#include "../imaging/v3d_imaging.h"
#include "../plugin_loader/v3d_plugin_loader.h"

#include "mainwindow.h"

bool neuron_subpattern_search(int option, MainWindow *pmain)
{
    v3d_msg(QString("Now try to do VR neuron search plugin functions [%1]").arg(option), 0);

    try
    {
        QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
        if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
            pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
        if (pluginsDir.dirName() == "MacOS") {
            pluginsDir.cdUp();
            pluginsDir.cdUp();
            pluginsDir.cdUp();
        }
#endif

        if (option==0)
        {
            if (pluginsDir.cd("plugins/vr_neuron_search/vr_nmotif_0")==false)
            {
                qDebug("Cannot find ./plugins/vr_neuron_search/vr_nmotif_0 directory!");
                return false;
            }
        }
        else if (option==1)
        {
            if (pluginsDir.cd("plugins/vr_neuron_search/vr_nmotif_bjut")==false)
            {
                qDebug("Cannot find ./plugins/vr_neuron_search/vr_nmotif_bjut directory!");
                return false;
            }
        }
        else if (option==2)
        {
            if (pluginsDir.cd("plugins/vr_neuron_search/vr_nmotif_shu")==false)
            {
                qDebug("Cannot find ./plugins/vr_neuron_search/vr_nmotif_shu directory!");
                return false;
            }
        }
        else
            return false;

        QStringList fileList = pluginsDir.entryList(QDir::Files);
        if (fileList.size()<1)
        {
            qDebug("Cannot find any file in the plugins directory!");
            return false;
        }

        QString fullpath = pluginsDir.absoluteFilePath(fileList.at(0)); //always just use the first file (assume it is the only one) found in the folder as the "correct" dll
        //the real dll name should be "xxxxx.dll"

        QPluginLoader* loader = new QPluginLoader(fullpath);
        if (!loader)
        {
            qDebug("ERROR in V3d_PluginLoader::searchPluginFiles the VR neuron search module(%s)", qPrintable(fullpath));
            return false;
        }

        v3d_msg(fullpath, 0);

        V3d_PluginLoader mypluginloader(pmain);
        QString datfilename = "tracing_menu";

        QTime t;
       // t.start();
        mypluginloader.runPlugin(loader, datfilename);
        //the data IO manager is specially designed to pass the datafilename parameter in this way.
        //2013-12-02. by PHC. what an ugly design here!
       // v3d_msg(QString("** invoke time for the plugin is [%1] ms.").arg(t.elapsed()*1000), 0);
    }
    catch (...)
    {
        v3d_msg("Catch a problem in v3d_imaging() wrapper function.", 0);
        return false;
    }

    return true;
}

bool call_neuron_assembler_live_plugin(MainWindow *pmain)
{
	try
	{
		QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
		if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
				pluginsDir.cdUp();
#endif

		if (pluginsDir.cd("plugins/neuron_utilities/assemble_neuron_live")==false)
		{
			qDebug("Cannot find ./plugins/neuron_utilities/assemble_neuron_live directory!");
			return false;
		}
        QStringList fileList =  pluginsDir.entryList(QDir::Files);
        if (fileList.size()<1)
        {
            qDebug("Cannot find any file in the plugins directory!");
            return false;
        }
		QString fullpath = pluginsDir.absoluteFilePath(fileList.at(0)); //always just use the first file (assume it is the only one) found in the folder as the "correct" dll
        //the real dll name should be "xxxxx.dll"
        QPluginLoader* loader = new QPluginLoader(fullpath);
        if(!loader) return false;
        V3d_PluginLoader mypluginloader(pmain);
        QString methodname = "load_new_stack";
        mypluginloader.runPlugin(loader, methodname);
    }
    catch (...)
    {
        v3d_msg("Catch a problem in v3d_imaging() wrapper function.", 0);
        return false;
    }
	return true;
}
