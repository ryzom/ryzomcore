// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

#ifndef TRANSLATION_MANAGER_PLUGIN_H
#define TRANSLATION_MANAGER_PLUGIN_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "translation_manager_main_window.h"

// NeL includes
#include "nel/misc/app_context.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QIcon>

using namespace std;

namespace NLMISC
{
class CLibraryContext;
}

namespace ExtensionSystem
{
class IPluginSpec;
}

namespace TranslationManager
{

class CTranslationManagerContext;

class TranslationManagerPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:
	virtual ~TranslationManagerPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();

	void setNelContext(NLMISC::INelContext *nelContext);
	void addAutoReleasedObject(QObject *obj);

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	ExtensionSystem::IPluginManager *_plugMan;
	QList<QObject *> _autoReleaseObjects;
};

class CTranslationManagerContext: public Core::IContext
{
	Q_OBJECT

public:
	CTranslationManagerContext(CMainWindow *mainWindow, QObject *parent = 0): IContext(parent)
	{
		m_MainWindow = mainWindow;
	}

	virtual ~CTranslationManagerContext() {}

	virtual QString id() const
	{
		return QLatin1String("TranslationManagerContext");
	}
	virtual QString trName() const
	{
		return tr("Translation Manager");
	}
	virtual QIcon icon() const
	{
		return QIcon();
	}
	virtual QWidget *widget()
	{
		return m_MainWindow;
	}
	virtual QUndoStack *undoStack()
	{
		return m_MainWindow->m_undoStack;
	}
	virtual void open()
	{

	}

	CMainWindow *m_MainWindow;
};

}

#endif // TRANSLATION_MANAGER_PLUGIN_H