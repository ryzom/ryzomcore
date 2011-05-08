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
#include <QtGui/QFileDialog>

namespace Plugin
{
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
	QAction *loadZoneAction = new QAction("Load Zone", this);
	QAction *saveZoneAction = new QAction("Save Zone", this);

	QMenu *toolsMenu = menuManager->menu(Core::Constants::M_TOOLS);
	QMenu *zoneMenu = toolsMenu->addMenu("Zone Painter");
	zoneMenu->addAction(loadZoneAction);
	connect(loadZoneAction, SIGNAL(triggered()), this, SLOT(clickLoadZoneAction()));
	zoneMenu->addAction(saveZoneAction);
}

/****** SLOTS ******/
void ZonePainterPlugin::clickLoadZoneAction() {
	QString zoneFile = QFileDialog::getOpenFileName(NULL, tr("Open Zone File"), ".", tr("Zone Files (*.zone);;"));
}

void ZonePainterPlugin::clickSaveZoneAction() {

}
/****** END SLOTS ******/

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