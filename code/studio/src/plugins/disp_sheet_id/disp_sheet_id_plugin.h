// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef SHEET_ID_VIEW_PLUGIN_H
#define SHEET_ID_VIEW_PLUGIN_H

#include "../../extension_system/iplugin.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>

namespace NLMISC
{
class CLibraryContext;
}

namespace SheetIdViewPlugin
{

class DispSheetIdPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	DispSheetIdPlugin();
	~DispSheetIdPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void setNelContext(NLMISC::INelContext *nelContext);

private Q_SLOTS:
	void execMessageBox();

protected:
	NLMISC::CLibraryContext *m_LibContext;

private:
	ExtensionSystem::IPluginManager *m_plugMan;

};

} // namespace SheetIdViewPlugin

#endif // SHEET_ID_VIEW_PLUGIN_H
