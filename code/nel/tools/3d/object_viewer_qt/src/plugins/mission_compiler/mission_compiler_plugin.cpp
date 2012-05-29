// Project includes
#include "mission_compiler_plugin.h"
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/menu_manager.h"
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
	Q_FOREACH(QObject *obj, m_autoReleaseObjects)
	{
		m_plugMan->removeObject(obj);
	}
	qDeleteAll(m_autoReleaseObjects);
	m_autoReleaseObjects.clear();
}

bool MissionCompilerPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

	addAutoReleasedObject(new MissionCompilerSettingsPage(this));
	addAutoReleasedObject(new CMissionCompilerContext(this));
	//addAutoReleasedObject(new CCoreListener(this));
	return true;
}

void MissionCompilerPlugin::extensionsInitialized()
{
	Core::ICore *core = Core::ICore::instance();
	QSettings *settings = Core::ICore::instance()->settings();
	Core::MenuManager *menuManager = core->menuManager();

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
	m_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

void MissionCompilerPlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

}

Q_EXPORT_PLUGIN(MissionCompiler::MissionCompilerPlugin)