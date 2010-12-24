
#include "PythonConsolePlugin.h"
#include "PythonConsoleWindow.h"
#include "V3DPythonModule.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(PythonConsole, PythonConsolePlugin);

PythonConsoleWindow* PythonConsolePlugin::pythonConsoleWindow = 0;

QStringList PythonConsolePlugin::menulist() const
{
    return QStringList()
		<< tr("Open V3D python console...")
		<< tr("About V3D python console...");
}

void PythonConsolePlugin::domenu(const QString & menu_name,
		V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("Open V3D python console..."))
    {
    	if (! pythonConsoleWindow) {
    		pythonConsoleWindow = new PythonConsoleWindow(parent);
    		v3d::initV3DPythonModule(&v3d);
    		pythonConsoleWindow->executeCommand("import v3d");
    	}
    	pythonConsoleWindow->show();
    }
	else
	{ // User selected "About..."
        QString msg = QString("V3D python console plugin version %1\n"
        		"Developed by Christopher M. Bruns. \n"
        		"(Janelia Farm Research Campus, HHMI)")
                .arg(getPluginVersion(), 1, 'f', 1);
		QMessageBox::information(parent, "About the V3D python console", msg);
	}
}
