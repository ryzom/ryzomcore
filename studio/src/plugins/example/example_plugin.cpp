// Project includes
#include "example_plugin.h"
#include "example_settings_page.h"
#include "simple_viewer.h"

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

namespace Plugin
{

ExamplePlugin::ExamplePlugin()
{
}

ExamplePlugin::~ExamplePlugin()
{
	Q_FOREACH(QObject *obj, m_autoReleaseObjects)
	{
		m_plugMan->removeObject(obj);
	}
	qDeleteAll(m_autoReleaseObjects);
	m_autoReleaseObjects.clear();
}

bool ExamplePlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

	addAutoReleasedObject(new ExampleSettingsPage(this));
	addAutoReleasedObject(new ExampleContext(this));
	addAutoReleasedObject(new ExampleCoreListener(this));
	return true;
}

void ExamplePlugin::extensionsInitialized()
{
	Core::ICore *core = Core::ICore::instance();
	Core::MenuManager *menuManager = core->menuManager();
	QAction *exampleAction1 = new QAction("Example1", this);
	QAction *exampleAction2 = new QAction("Example2", this);
	QAction *aboutQtAction = menuManager->action(Core::Constants::ABOUT_QT);
	QMenu *helpMenu = menuManager->menu(Core::Constants::M_HELP);
	helpMenu->insertAction(aboutQtAction, exampleAction1);
	helpMenu->addSeparator();
	helpMenu->addAction(exampleAction2);
	menuManager->menuBar()->addMenu("ExampleMenu");
}

void ExamplePlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

void ExamplePlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

}

Q_EXPORT_PLUGIN(Plugin::ExamplePlugin)