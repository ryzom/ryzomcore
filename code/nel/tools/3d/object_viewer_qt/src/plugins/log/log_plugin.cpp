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
#include "../../extension_system/iplugin_spec.h"
#include "qt_displayer.h"

using namespace Plugin;

CLogPlugin::CLogPlugin(QWidget *parent): QDockWidget(parent)
{
	_ui.setupUi(this);
}

CLogPlugin::~CLogPlugin() 
{
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
	return true;
}

void CLogPlugin::extensionsInitialized()
{
	QMenu *helpMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.View"));
	helpMenu->addSeparator();

	NLMISC::ErrorLog->addDisplayer(_displayer);
	NLMISC::WarningLog->addDisplayer(_displayer);
	NLMISC::DebugLog->addDisplayer(_displayer);
	NLMISC::AssertLog->addDisplayer(_displayer);
	NLMISC::InfoLog->addDisplayer(_displayer);

	QMainWindow *wnd = qobject_cast<QMainWindow *>(objectByName("CMainWindow"));
	wnd->addDockWidget(Qt::RightDockWidgetArea, this);
	hide();
	helpMenu->addAction(this->toggleViewAction());
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

QList<QString> CLogPlugin::dependencies() const
{
	return QList<QString>();
}

QObject* CLogPlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
		if (qobj->objectName() == name)
			return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *CLogPlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
		if (spec->name() == name)
			return spec;
	return 0;
}

Q_EXPORT_PLUGIN(CLogPlugin)
