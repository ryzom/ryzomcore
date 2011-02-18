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

// Project includes
#include "core_plugin.h"
#include "settings_dialog.h"
#include "core_constants.h"
#include "search_paths_settings_page.h"
#include "../../extension_system/iplugin_spec.h"
#include "qtwin.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

using namespace Core;

CorePlugin::CorePlugin()
{
}

CorePlugin::~CorePlugin()
{
	Q_FOREACH(QObject *obj, _autoReleaseObjects)
	{
		_plugMan->removeObject(obj);
	}
	qDeleteAll(_autoReleaseObjects);
	_autoReleaseObjects.clear();
}

bool CorePlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;
	_oldOVQT = false;

	// for old ovqt
	QMainWindow *wnd = qobject_cast<QMainWindow *>(_plugMan->objectByName("CMainWindow"));
	if (wnd)
	{
		_pluginView = new ExtensionSystem::CPluginView(_plugMan);
		QMenu *toolsMenu = qobject_cast<QMenu *>(_plugMan->objectByName("ovqt.Menu.Tools"));
		QMenu *helpMenu = qobject_cast<QMenu *>(_plugMan->objectByName("ovqt.Menu.Help"));
		nlassert(toolsMenu);
		nlassert(helpMenu);

		QAction *newAction = toolsMenu->addAction(tr("New settings"));
		QAction *newAction2 = helpMenu->addAction(tr("About plugins"));
		newAction->setIcon(QIcon(Constants::ICON_SETTINGS));

		connect(newAction, SIGNAL(triggered()), this, SLOT(execSettings()));
		connect(newAction2, SIGNAL(triggered()), _pluginView, SLOT(show()));
		_oldOVQT = true;
	}
	else
	{
		_mainWindow = new CMainWindow(this);
#ifdef Q_WS_X11
		_mainWindow->setAttribute(Qt::WA_TranslucentBackground);
		_mainWindow->setAttribute(Qt::WA_NoSystemBackground, false);
		QPalette pal = _mainWindow->palette();
		QColor bg = pal.window().color();
		bg.setAlpha(180);
		pal.setColor(QPalette::Window, bg);
		_mainWindow->setPalette(pal);
		_mainWindow->ensurePolished(); // workaround Oxygen filling the background
		_mainWindow->setAttribute(Qt::WA_StyledBackground, false);
#endif
		if (QtWin::isCompositionEnabled())
		{
			QtWin::extendFrameIntoClientArea(_mainWindow);
			_mainWindow->setContentsMargins(0, 0, 0, 0);
		}
		_mainWindow->show();
	}

	addAutoReleasedObject(new CSearchPathsSettingsPage(this));
	return true;
}

void CorePlugin::extensionsInitialized()
{
	_pluginView = new ExtensionSystem::CPluginView(_plugMan);
}

void CorePlugin::shutdown()
{
	if (!_oldOVQT)
	{
		delete _mainWindow;
		delete _pluginView;
	}
}

void CorePlugin::execSettings()
{
	CSettingsDialog settingsDialog(this);
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

void CorePlugin::addAutoReleasedObject(QObject *obj)
{
	_plugMan->addObject(obj);
	_autoReleaseObjects.prepend(obj);
}

Q_EXPORT_PLUGIN(CorePlugin)
