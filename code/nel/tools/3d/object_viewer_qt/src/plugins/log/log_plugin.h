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

#ifndef LOG_PLUGIN_H
#define LOG_PLUGIN_H

#include "../../extension_system/iplugin.h"

#include "nel/misc/app_context.h"

// Qt includes
#include <QDockWidget>

// Project includes
#include "ui_log_form.h"

namespace NLMISC
{
	class CLibraryContext;
}

namespace NLQT
{
	class IPluginSpec;
	class CQtDisplayer;
}

namespace Plugin 
{

	class CLogPlugin : public QDockWidget, public NLQT::IPlugin
	{
		Q_OBJECT
		Q_INTERFACES(NLQT::IPlugin)
	public:
		CLogPlugin(QWidget *parent = 0);
		~CLogPlugin();

		bool initialize(NLQT::IPluginManager *pluginManager, QString *errorString);
		void extensionsInitialized();

		void setNelContext(NLMISC::INelContext *nelContext);

		QString name() const;
		QString version() const;
		QString vendor() const;
		QString description() const;

		QObject *objectByName(const QString &name) const;
		NLQT::IPluginSpec *pluginByName(const QString &name) const;

		private Q_SLOTS:
			void createLogDock();

	protected:
		NLMISC::CLibraryContext *_LibContext;

	private:
		NLQT::IPluginManager *_plugMan;

		Ui::CLogPlugin _ui;

		NLQT::CQtDisplayer *_displayer;

	};

} // namespace Plugin

#endif // LOG_PLUGIN_H
