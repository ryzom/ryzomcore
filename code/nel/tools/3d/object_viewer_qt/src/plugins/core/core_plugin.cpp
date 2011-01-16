// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2010  Dzmitry Kamiahin <dnk-88@tut.by>
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

#include "core_plugin.h"

#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

#include "../../extension_system/iplugin_spec.h"

#include "settings_dialog.h"
#include "core_constants.h"
#include "search_paths_settings_page.h"
#include "nel/misc/debug.h"

using namespace Core;

bool CorePlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;
	// for old ovqt
	QMainWindow *wnd = qobject_cast<QMainWindow *>(objectByName("CMainWindow"));
	if (!wnd)
	{
		*errorString = tr("Not found QMainWindow Object Viewer Qt.");
		return false;
	}

	//_mainWindow = new CMainWindow(_plugMan);
	//_mainWindow->show();
	_plugMan->addObject(new CSearchPathsSettingsPage(wnd));
	return true;
}

void CorePlugin::extensionsInitialized()
{
	// for old ovqt
	_pluginView = new ExtensionSystem::CPluginView(_plugMan);

	QMenu *toolsMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.Tools"));
	QMenu *helpMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.Help"));
	nlassert(toolsMenu);
	nlassert(helpMenu);

	QAction *newAction = toolsMenu->addAction(tr("New settings"));
	QAction *newAction2 = helpMenu->addAction(tr("About plugins"));
	newAction->setIcon(QIcon(Constants::ICON_SETTINGS));

	connect(newAction, SIGNAL(triggered()), this, SLOT(execSettings()));
	connect(newAction2, SIGNAL(triggered()), _pluginView, SLOT(show()));
}

void CorePlugin::shutdown()
{
	//delete _mainWindow;
	delete _pluginView;
}

void CorePlugin::execSettings()
{
	CSettingsDialog settingsDialog(_plugMan);
	settingsDialog.show();
	settingsDialog.execDialog();
}

void CorePlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

QString CorePlugin::name() const
{
	return QLatin1String("Core");
}

QString CorePlugin::version() const
{
	return Constants::OVQT_VERSION_LONG;
}

QString CorePlugin::vendor() const
{
	return Constants::OVQT_VENDOR;
}

QString CorePlugin::description() const
{
	return "Core plugin.";
}

QList<QString> CorePlugin::dependencies() const
{
	return QList<QString>();
}

QObject* CorePlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
	if (qobj->objectName() == name)
		return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *CorePlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
	if (spec->name() == name)
		return spec;
	return 0;
}

Q_EXPORT_PLUGIN(CorePlugin)
