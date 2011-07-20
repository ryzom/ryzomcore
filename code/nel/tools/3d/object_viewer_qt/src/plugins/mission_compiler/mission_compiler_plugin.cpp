// Project includes
#include "mission_compiler_plugin.h"
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/imenu_manager.h"
#include "../../extension_system/iplugin_spec.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

#include "mission_compiler_settings_page.h"

namespace MissionCompiler
{

MissionCompilerPlugin::~MissionCompilerPlugin()
{
	Q_FOREACH(QObject *obj, _autoReleaseObjects)
	{
		_plugMan->removeObject(obj);
	}
	qDeleteAll(_autoReleaseObjects);
	_autoReleaseObjects.clear();
}

bool MissionCompilerPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;

	addAutoReleasedObject(new MissionCompilerSettingsPage(this));
	addAutoReleasedObject(new CMissionCompilerContext(this));
	//addAutoReleasedObject(new CCoreListener(this));
	return true;
}

void MissionCompilerPlugin::extensionsInitialized()
{
	Core::ICore *core = Core::ICore::instance();
	QSettings *settings = Core::ICore::instance()->settings();
	Core::IMenuManager *menuManager = core->menuManager();
	//menuManager = _plugMan->getObject<Core::IMenuManager>();
	//QAction *exampleAction1 = new QAction("Zone1", this);
	//QAction *exampleAction2 = new QAction("Zone2", this);
	//QMenu *toolsMenu = menuManager->menu(Core::Constants::M_TOOLS);
	//helpMenu->insertAction(aboutQtAction, exampleAction1);
	//helpMenu->addSeparator();
	//helpMenu->addAction(exampleAction2);
	//QMenu *zoneMenu = menuManager->menuBar()->addMenu("ZoneMenu");
	//zoneMenu->insertAction(aboutQtAction, exampleAction1);
	//zoneMenu->addSeparator();
	//zoneMenu->addAction(exampleAction2);

	// Initialize Ligo.
	//settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	//QString ligoConfigFile = settings->value(Core::Constants::DATA_PATH_SECTION).toString();
	//settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
}

void MissionCompilerPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

QString MissionCompilerPlugin::name() const
{
	return "MissionCompilerPlugin";
}

QString MissionCompilerPlugin::version() const
{
	return "0.1";
}

QString MissionCompilerPlugin::vendor() const
{
	return "Ryzom Core";
}

QString MissionCompilerPlugin::description() const
{
	return "Mission Compiler Plugin";
}

QStringList MissionCompilerPlugin::dependencies() const
{
	QStringList list;
	list.append(Core::Constants::OVQT_CORE_PLUGIN);
	//list.append("ObjectViewer");
	return list;
}

void MissionCompilerPlugin::addAutoReleasedObject(QObject *obj)
{
	_plugMan->addObject(obj);
	_autoReleaseObjects.prepend(obj);
}

QObject* MissionCompilerPlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
	if (qobj->objectName() == name)
		return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *MissionCompilerPlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
	if (spec->name() == name)
		return spec;
	return 0;
}

}

Q_EXPORT_PLUGIN(MissionCompiler::MissionCompilerPlugin)