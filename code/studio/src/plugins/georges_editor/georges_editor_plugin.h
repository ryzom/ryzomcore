// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef GEORGES_EDITOR_PLUGIN_H
#define GEORGES_EDITOR_PLUGIN_H

// Project includes
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

namespace ExtensionSystem
{
class IPluginSpec;
}

namespace GeorgesQt
{
class GeorgesEditorForm;
class GeorgesEditorPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~GeorgesEditorPlugin();

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

class GeorgesEditorContext: public Core::IContext
{
	Q_OBJECT
public:
	GeorgesEditorContext(QObject *parent = 0);
	virtual ~GeorgesEditorContext() {}

	virtual QString id() const
	{
		return QLatin1String("GeorgesEditorContext");
	}
	virtual QString trName() const
	{
		return tr("Georges Editor");
	}
	virtual QIcon icon() const
	{
		return QIcon(":/images/ic_nel_georges_editor.png");
	}

	virtual void open();

	virtual QUndoStack *undoStack();

	virtual QWidget *widget();

	GeorgesEditorForm *m_georgesEditorForm;
};

} // namespace GeorgesQt

#endif // LANDSCAPE_EDITOR_PLUGIN_H
