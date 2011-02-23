#include "disp_sheet_id_plugin.h"
#include "dialog.h"
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

#include "../../extension_system/iplugin_spec.h"

#include "nel/misc/debug.h"

using namespace Plugin;

bool MyPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;
	QMainWindow *wnd = qobject_cast<QMainWindow *>(objectByName("CMainWindow"));
	if (!wnd)
	{
		*errorString = tr("Not found QMainWindow Object Viewer Qt.");
		return false;
	}
	return true;
}

void MyPlugin::extensionsInitialized()
{
	QMenu *toolsMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.Tools"));
	nlassert(toolsMenu);

	QAction *newAction = toolsMenu->addAction("Display sheet id");

	connect(newAction, SIGNAL(triggered()), this, SLOT(execMessageBox()));
}

void MyPlugin::execMessageBox()
{
	Dialog dialog;
	dialog.show();
	dialog.exec();
}

void MyPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

QString MyPlugin::name() const
{
	return "Display sheet id";
}

QString MyPlugin::version() const
{
	return "0.1";
}

QString MyPlugin::vendor() const
{
	return "pemeon";
}

QString MyPlugin::description() const
{
	return "Display sheet id";
}

QList<QString> MyPlugin::dependencies() const
{
	return QList<QString>();
}

QObject* MyPlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
	if (qobj->objectName() == name)
		return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *MyPlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
	if (spec->name() == name)
		return spec;
	return 0;
}

Q_EXPORT_PLUGIN(MyPlugin)
