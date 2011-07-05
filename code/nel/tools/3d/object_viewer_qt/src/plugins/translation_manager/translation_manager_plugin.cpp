// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Project includes
#include "translation_manager_plugin.h"
#include "translation_manager_settings_page.h"
#include "translation_manager_main_window.h"
// Project system includes
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/imenu_manager.h"
#include "../../extension_system/iplugin_spec.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QErrorMessage>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

namespace Plugin
{
TranslationManagerPlugin::~TranslationManagerPlugin()
{
	Q_FOREACH(QObject *obj, _autoReleaseObjects)
	{
		_plugMan->removeObject(obj);
	}
	qDeleteAll(_autoReleaseObjects);
	_autoReleaseObjects.clear();
}

bool TranslationManagerPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;

	addAutoReleasedObject(new CTranslationManagerSettingsPage(this));
	addAutoReleasedObject(new CTranslationManagerContext(this));
	addAutoReleasedObject(new CCoreListener(this));
        
	return true;
}

void TranslationManagerPlugin::extensionsInitialized()
{
	Core::ICore *core = Core::ICore::instance();
	Core::IMenuManager *menuManager = core->menuManager();
	//menuManager = _plugMan->getObject<Core::IMenuManager>();
	// Menu Actions for plugin
	QAction *aboutTManPlugin = new QAction("Translation Manager", this);
	// Locations
	QMenu *helpMenu = menuManager->menu(Core::Constants::M_HELP);
	QAction *aboutQtAction = menuManager->action(Core::Constants::ABOUT_QT);
	helpMenu->addSeparator();
	helpMenu->insertAction(aboutQtAction, aboutTManPlugin);
}

void TranslationManagerPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

QString TranslationManagerPlugin::name() const
{
	return "Translation Manager";
}

QString TranslationManagerPlugin::version() const
{
	return "0.1";
}

QString TranslationManagerPlugin::vendor() const
{
	return "cemycc";
}

QString TranslationManagerPlugin::description() const
{
	return "OVQT plugin for translation files.";
}

QStringList TranslationManagerPlugin::dependencies() const
{
	QStringList list;
	list.append(Core::Constants::OVQT_CORE_PLUGIN);
	//list.append("ObjectViewer");
	return list;
}

void TranslationManagerPlugin::addAutoReleasedObject(QObject *obj)
{
	_plugMan->addObject(obj);
	_autoReleaseObjects.prepend(obj);
}

QObject* TranslationManagerPlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
	if (qobj->objectName() == name)
		return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *TranslationManagerPlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
	if (spec->name() == name)
		return spec;
	return 0;
}

}

Q_EXPORT_PLUGIN(Plugin::TranslationManagerPlugin)
