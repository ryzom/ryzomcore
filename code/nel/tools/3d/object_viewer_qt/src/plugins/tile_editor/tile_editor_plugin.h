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

#ifndef TILE_EDITOR_PLUGIN_H
#define TILE_EDITOR_PLUGIN_H

#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"

#include "nel/misc/app_context.h"

#include "tile_editor_main_window.h"

#include <QtCore/QObject>

namespace NLMISC
{
class CLibraryContext;
}

namespace TileEditorPluginQt
{

class TileEditorPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	~TileEditorPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

protected:
	NLMISC::CLibraryContext *m_LibContext;

private:
	ExtensionSystem::IPluginManager *m_plugMan;
	QList<QObject *> m_autoReleaseObjects;

};

class TileEditorContext: public Core::IContext
{
	Q_OBJECT
public:
	TileEditorContext(QObject *parent = 0) : IContext(parent)
	{
		m_tileEditorMainWindow = new TileEditorMainWindow();
	}

	virtual ~TileEditorContext() 
	{
		m_tileEditorMainWindow = NULL;
	}

	virtual QString id() const
	{
		return QLatin1String("TileEditor");
	}

	virtual QString trName() const
	{
		return tr("Tile Editor");
	}

	virtual QIcon icon() const
	{
		return QIcon(":/tileRotation/images/rotation0.png");
	}

	virtual QUndoStack *undoStack()
	{
		return m_tileEditorMainWindow->getUndoStack();
	}

	virtual void open()
	{
	}

	virtual QWidget *widget()
	{
		return m_tileEditorMainWindow;
	}

private:
	TileEditorMainWindow *m_tileEditorMainWindow;
};

} // namespace TileEditorPluginQt

#endif // TILE_EDITOR_PLUGIN_H
