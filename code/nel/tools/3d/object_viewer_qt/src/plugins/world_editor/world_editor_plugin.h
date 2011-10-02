// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef WORLD_EDITOR_PLUGIN_H
#define WORLD_EDITOR_PLUGIN_H

// Project includes
#include "world_editor_constants.h"
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"

// NeL includes
#include "nel/misc/app_context.h"
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace NLMISC
{
class CLibraryContext;
}

namespace WorldEditor
{
class WorldEditorWindow;

class WorldEditorPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~WorldEditorPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void shutdown();
	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

protected:
	NLMISC::CLibraryContext *m_libContext;

private:
	NLLIGO::CLigoConfig m_ligoConfig;
	ExtensionSystem::IPluginManager *m_plugMan;
	QList<QObject *> m_autoReleaseObjects;
};

class WorldEditorContext: public Core::IContext
{
	Q_OBJECT
public:
	WorldEditorContext(QObject *parent = 0);
	virtual ~WorldEditorContext() {}

	virtual QString id() const
	{
		return QLatin1String("WorldEditorContext");
	}
	virtual QString trName() const
	{
		return tr("World Editor");
	}
	virtual QIcon icon() const
	{
		return QIcon(Constants::ICON_WORLD_EDITOR);
	}

	virtual void open();

	virtual QUndoStack *undoStack();

	virtual QWidget *widget();

	WorldEditorWindow *m_worldEditorWindow;
};

} // namespace WorldEditor

#endif // WORLD_EDITOR_PLUGIN_H
