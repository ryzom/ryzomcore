// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef LANDSCAPE_EDITOR_PLUGIN_H
#define LANDSCAPE_EDITOR_PLUGIN_H

// Project includes
#include "landscape_editor_constants.h"
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"

// NeL includes
#include "nel/misc/app_context.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace NLMISC
{
class CLibraryContext;
}

namespace LandscapeEditor
{
class LandscapeEditorWindow;

class LandscapeEditorPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~LandscapeEditorPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void shutdown();
	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

protected:
	NLMISC::CLibraryContext *m_libContext;

private:
	ExtensionSystem::IPluginManager *m_plugMan;
	QList<QObject *> m_autoReleaseObjects;
};

class LandscapeEditorContext: public Core::IContext
{
	Q_OBJECT
public:
	explicit LandscapeEditorContext(QObject *parent = 0);
	virtual ~LandscapeEditorContext() {}

	virtual QString id() const
	{
		return QLatin1String("LandscapeEditorContext");
	}
	virtual QString trName() const
	{
		return tr("Landscape Editor");
	}
	virtual QIcon icon() const
	{
		return QIcon(Constants::ICON_LANDSCAPE_ITEM);
	}

	virtual void open();

	virtual QUndoStack *undoStack();

	virtual QWidget *widget();

	LandscapeEditorWindow *m_landEditorWindow;
};

} // namespace LandscapeEditor

#endif // LANDSCAPE_EDITOR_PLUGIN_H
