#include "plugin1.h"

#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>

#include "../../extension_system/plugin_spec.h"

#include "nel/misc/debug.h"

using namespace Plugin;

bool MyPlugin::initialize(NLQT::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;
	QString str;
	
	QList<NLQT::CPluginSpec *>  listPlug = pluginManager->plugins();
	
	Q_FOREACH (NLQT::CPluginSpec *plugSpec, listPlug)
		str += plugSpec->name();

	QMessageBox msgBox;
	msgBox.setText(str);
	msgBox.exec();

	nlinfo("test message");

	return true;
}

void MyPlugin::extensionsInitialized()
{
	QString str;
	QList<QObject *> listObjects = _plugMan->allObjects();

	Q_FOREACH (QObject *qobj, listObjects)
	{
		if (qobj->objectName() == "CMainWindow")
		{
			QMainWindow *wnd = qobject_cast< QMainWindow* >(qobj);
			str += qobj->objectName() + QString(": width=%1,height=%2").arg(wnd->width()).arg(wnd->height());
		}
	}

	QMessageBox msgBox;
	msgBox.setText(str);
	msgBox.exec();
}

void MyPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
	nlassert(!NLMISC::INelContext::isContextInitialised());
	_LibContext = static_cast<NLMISC::CLibraryContext *>(nelContext);
}

QString MyPlugin::name() const
{
	return "ExamplePlugin";
}

QString MyPlugin::version() const
{
	return "0.1";
}

QString MyPlugin::vendor() const
{
	return "dnk";
}

QString MyPlugin::description() const
{
	return "Example plugin";
}

Q_EXPORT_PLUGIN(MyPlugin)
