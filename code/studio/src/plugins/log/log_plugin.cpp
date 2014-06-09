// Object Viewer Qt - Log Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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
#include "log_plugin.h"
#include "log_settings_page.h"
#include "qt_displayer.h"

#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/menu_manager.h"
#include "../../extension_system/iplugin_spec.h"

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

namespace Plugin
{

	CLogPlugin::CLogPlugin(QWidget *parent): QDockWidget(parent)
	{
		m_ui.setupUi(this);
		logMenu = NULL;
	}

	CLogPlugin::~CLogPlugin()
	{
		Q_FOREACH(QObject *obj, m_autoReleaseObjects)
		{
			m_plugMan->removeObject(obj);
		}
		qDeleteAll(m_autoReleaseObjects);
		m_autoReleaseObjects.clear();

		NLMISC::ErrorLog->removeDisplayer(m_displayer);
		NLMISC::WarningLog->removeDisplayer(m_displayer);
		NLMISC::DebugLog->removeDisplayer(m_displayer);
		NLMISC::AssertLog->removeDisplayer(m_displayer);
		NLMISC::InfoLog->removeDisplayer(m_displayer);
		delete m_displayer;

		delete logMenu;
		logMenu = NULL;

		delete m_libContext;
		m_libContext = NULL;
	}

	bool CLogPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
	{
		Q_UNUSED(errorString);
		m_plugMan = pluginManager;
		m_logSettingsPage = new CLogSettingsPage(this, this);
		addAutoReleasedObject(m_logSettingsPage);
		return true;
	}

	void CLogPlugin::extensionsInitialized()
	{
		setDisplayers();

		Core::ICore *core = Core::ICore::instance();
		Core::MenuManager *menuManager = core->menuManager();

		QMainWindow *wnd = Core::ICore::instance()->mainWindow();
		wnd->addDockWidget(Qt::RightDockWidgetArea, this);
		hide();

		logMenu = menuManager->menuBar()->addMenu( "Log" );
		logMenu->addAction(toggleViewAction());
	}

	void CLogPlugin::setNelContext(NLMISC::INelContext *nelContext)
	{
#ifdef NL_OS_WINDOWS
		// Ensure that a context doesn't exist yet.
		// This only applies to platforms without PIC, e.g. Windows.
		nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // fdef NL_OS_WINDOWS^M
		m_libContext = new NLMISC::CLibraryContext(*nelContext);

		m_displayer = new NLQT::CQtDisplayer(m_ui.plainTextEdit);

	}

	QString CLogPlugin::name() const
	{
		return "LogPlugin";
	}

	QString CLogPlugin::version() const
	{
		return "1.1";
	}

	QString CLogPlugin::vendor() const
	{
		return "aquiles";
	}

	QString CLogPlugin::description() const
	{
		return tr("DockWidget to display all log messages from NeL.");
	}

	QStringList CLogPlugin::dependencies() const
	{
		QStringList list;
		list.append(Core::Constants::OVQT_CORE_PLUGIN);
		return list;
	}

	void CLogPlugin::addAutoReleasedObject(QObject *obj)
	{
		m_plugMan->addObject(obj);
		m_autoReleaseObjects.prepend(obj);
	}

	void CLogPlugin::setDisplayers()
	{
		QSettings *settings = Core::ICore::instance()->settings();

		settings->beginGroup(Core::Constants::LOG_SECTION);
		bool error = settings->value(Core::Constants::LOG_ERROR,     true).toBool();
		bool warning = settings->value(Core::Constants::LOG_WARNING, true).toBool();
		bool debug = settings->value(Core::Constants::LOG_DEBUG,     true).toBool();
		bool assert = settings->value(Core::Constants::LOG_ASSERT,   true).toBool();
		bool info = settings->value(Core::Constants::LOG_INFO,       true).toBool();
		settings->endGroup();

		if (error) {
			if (!NLMISC::ErrorLog->attached(m_displayer))
				NLMISC::ErrorLog->addDisplayer(m_displayer);
		} else {
			if (m_displayer) {
				NLMISC::ErrorLog->removeDisplayer(m_displayer);
			}
		}
		if (warning) {
			if (!NLMISC::WarningLog->attached(m_displayer))
				NLMISC::WarningLog->addDisplayer(m_displayer);
		} else {
			if (m_displayer) {
				NLMISC::WarningLog->removeDisplayer(m_displayer);
			}
		}
		if (debug) {
			if (!NLMISC::DebugLog->attached(m_displayer))
				NLMISC::DebugLog->addDisplayer(m_displayer);
		} else {
			if (m_displayer) {
				NLMISC::DebugLog->removeDisplayer(m_displayer);
			}
		}
		if (assert) {
			if (!NLMISC::AssertLog->attached(m_displayer))
				NLMISC::AssertLog->addDisplayer(m_displayer);
		} else {
			if (m_displayer) {
				NLMISC::AssertLog->removeDisplayer(m_displayer);
			}
		}
		if (info) {
			if (!NLMISC::InfoLog->attached(m_displayer))
				NLMISC::InfoLog->addDisplayer(m_displayer);
		} else {
			if (m_displayer) {
				NLMISC::InfoLog->removeDisplayer(m_displayer);
			}
		}
	}
}
Q_EXPORT_PLUGIN(Plugin::CLogPlugin)
