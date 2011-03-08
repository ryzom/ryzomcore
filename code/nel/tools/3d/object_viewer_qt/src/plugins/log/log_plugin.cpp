/*
Log Plugin Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "log_plugin.h"
#include "log_settings_page.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>
#include <QtGui/QWidget>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

// NeL includes
#include <nel/misc/debug.h>

// Project includes
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/imenu_manager.h"
#include "qt_displayer.h"

using namespace Plugin;

namespace ExtensionSystem
{
	class IPluginSpec;
}

CLogPlugin::CLogPlugin(QWidget *parent): QDockWidget(parent)
{
	_ui.setupUi(this);
}

CLogPlugin::~CLogPlugin() 
{
	_plugMan->removeObject(_logSettingsPage);
	delete _logSettingsPage;

	NLMISC::ErrorLog->removeDisplayer(_displayer);
	NLMISC::WarningLog->removeDisplayer(_displayer);
	NLMISC::DebugLog->removeDisplayer(_displayer);
	NLMISC::AssertLog->removeDisplayer(_displayer);
	NLMISC::InfoLog->removeDisplayer(_displayer);
	delete _displayer;
}

bool CLogPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;
	_logSettingsPage = new CLogSettingsPage(this);
	_plugMan->addObject(_logSettingsPage);
	return true;
}

void CLogPlugin::extensionsInitialized()
{
	NLMISC::ErrorLog->addDisplayer(_displayer);
	NLMISC::WarningLog->addDisplayer(_displayer);
	NLMISC::DebugLog->addDisplayer(_displayer);
	NLMISC::AssertLog->addDisplayer(_displayer);
	NLMISC::InfoLog->addDisplayer(_displayer);

	Core::ICore *core = Core::ICore::instance();
	Core::IMenuManager *menuManager = core->menuManager();
	QMenu *viewMenu = menuManager->menu(Core::Constants::M_VIEW);

	QMainWindow *wnd = Core::ICore::instance()->mainWindow();
	wnd->addDockWidget(Qt::RightDockWidgetArea, this);
	hide();

	viewMenu->addAction(this->toggleViewAction());
}

void CLogPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS 
        // Ensure that a context doesn't exist yet.  
        // This only applies to platforms without PIC, e.g. Windows.  
        nlassert(!NLMISC::INelContext::isContextInitialised()); 
#endif // fdef NL_OS_WINDOWS^M
        _LibContext = new NLMISC::CLibraryContext(*nelContext); 

	_displayer = new NLQT::CQtDisplayer(_ui.plainTextEdit);

}

QString CLogPlugin::name() const
{
	return "LogPlugin";
}

QString CLogPlugin::version() const
{
	return "1.0";
}

QString CLogPlugin::vendor() const
{
	return "aquiles";
}

QString CLogPlugin::description() const
{
	return "DockWidget to display all log messages from NeL.";
}

QStringList CLogPlugin::dependencies() const
{
	QStringList list;
	list.append(Core::Constants::OVQT_CORE_PLUGIN);
	return list;
}

Q_EXPORT_PLUGIN(CLogPlugin)
