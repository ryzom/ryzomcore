/*
Log Plugin Qt
Copyright (C) 2011 Adrian Jaekel <aj at elane2k dot com>

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

#ifndef LOG_PLUGIN_H
#define LOG_PLUGIN_H

// Project includes
#include "ui_log_form.h"
#include "../../extension_system/iplugin.h"

// NeL includes
#include "nel/misc/app_context.h"

// Qt includes
#include <QDockWidget>

namespace NLMISC
{
	class CLibraryContext;
}

namespace ExtensionSystem
{
	class IPluginSpec;
}

namespace NLQT
{
	class CQtDisplayer;
}

namespace Plugin 
{
	class CLogSettingsPage;

	class CLogPlugin : public QDockWidget, public ExtensionSystem::IPlugin
	{
		Q_OBJECT
			Q_INTERFACES(ExtensionSystem::IPlugin)
	public:
		CLogPlugin(QWidget *parent = 0);
		~CLogPlugin();

		bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
		void extensionsInitialized();

		void setNelContext(NLMISC::INelContext *nelContext);
		NLQT::CQtDisplayer* displayer() { return m_displayer; }

		QString name() const;
		QString version() const;
		QString vendor() const;
		QString description() const;
		QStringList dependencies() const;

		void addAutoReleasedObject(QObject *obj);

		void setDisplayers();

	protected:
		NLMISC::CLibraryContext *m_libContext;

	private:
		ExtensionSystem::IPluginManager *m_plugMan;
		QList<QObject *> m_autoReleaseObjects;
		CLogSettingsPage *m_logSettingsPage;

		Ui::CLogPlugin m_ui;

		NLQT::CQtDisplayer *m_displayer;

		QMenu *logMenu;

	};

} // namespace Plugin

#endif // LOG_PLUGIN_H
