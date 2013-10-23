// Project includes
#include "zone_painter_plugin.h"
#include "zone_painter_settings_page.h"
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/menu_manager.h"
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
//	NLMISC_SAFE_SINGLETON_IMPL(CZoneManager)

ZonePainterPlugin::~ZonePainterPlugin()
{
	Q_FOREACH(QObject *obj, m_autoReleaseObjects)
	{
		m_plugMan->removeObject(obj);
	}
	qDeleteAll(m_autoReleaseObjects);
	m_autoReleaseObjects.clear();
}

bool ZonePainterPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

	addAutoReleasedObject(new CZonePainterSettingsPage(this));
	addAutoReleasedObject(new CZonePainterContext(this));
	//addAutoReleasedObject(new CCoreListener(this));

	return true;
}

void ZonePainterPlugin::extensionsInitialized()
{
	Core::ICore *core = Core::ICore::instance();
	Core::MenuManager *menuManager = core->menuManager();
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
	m_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

void ZonePainterPlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

}

Q_EXPORT_PLUGIN(Plugin::ZonePainterPlugin)