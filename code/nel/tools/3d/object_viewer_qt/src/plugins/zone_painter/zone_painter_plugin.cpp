// Project includes
#include "zone_painter_plugin.h"
#include "zone_painter_settings_page.h"
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/imenu_manager.h"
#include "../../extension_system/iplugin_spec.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

namespace Plugin
{
	NLMISC_SAFE_SINGLETON_IMPL(CZoneManager)

ZonePainterPlugin::~ZonePainterPlugin()
{
	Q_FOREACH(QObject *obj, _autoReleaseObjects)
	{
		_plugMan->removeObject(obj);
	}
	qDeleteAll(_autoReleaseObjects);
	_autoReleaseObjects.clear();
}

bool ZonePainterPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;

	addAutoReleasedObject(new CZonePainterSettingsPage(this));
	addAutoReleasedObject(new CZonePainterContext(this));
	//addAutoReleasedObject(new CCoreListener(this));
	return true;
}

void ZonePainterPlugin::extensionsInitialized()
{
	Core::ICore *core = Core::ICore::instance();
	Core::IMenuManager *menuManager = core->menuManager();
	//menuManager = _plugMan->getObject<Core::IMenuManager>();
	QAction *exampleAction1 = new QAction("Zone1", this);
	QAction *exampleAction2 = new QAction("Zone2", this);
	QAction *aboutQtAction = menuManager->action(Core::Constants::ABOUT_QT);
	QMenu *helpMenu = menuManager->menu(Core::Constants::M_HELP);
	helpMenu->insertAction(aboutQtAction, exampleAction1);
	helpMenu->addSeparator();
	helpMenu->addAction(exampleAction2);
	QMenu *zoneMenu = menuManager->menuBar()->addMenu("ZoneMenu");
	zoneMenu->insertAction(aboutQtAction, exampleAction1);
	zoneMenu->addSeparator();
	zoneMenu->addAction(exampleAction2);
}

void ZonePainterPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

QString ZonePainterPlugin::name() const
{
	return "ZonePainterPlugin";
}

QString ZonePainterPlugin::version() const
{
	return "0.2";
}

QString ZonePainterPlugin::vendor() const
{
	return "Ryzom Core";
}

QString ZonePainterPlugin::description() const
{
	return "Zone Painter Plugin";
}

QStringList ZonePainterPlugin::dependencies() const
{
	QStringList list;
	list.append(Core::Constants::OVQT_CORE_PLUGIN);
	//list.append("ObjectViewer");
	return list;
}

void ZonePainterPlugin::addAutoReleasedObject(QObject *obj)
{
	_plugMan->addObject(obj);
	_autoReleaseObjects.prepend(obj);
}

QObject* ZonePainterPlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
	if (qobj->objectName() == name)
		return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *ZonePainterPlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
	if (spec->name() == name)
		return spec;
	return 0;
}

}

Q_EXPORT_PLUGIN(Plugin::ZonePainterPlugin)