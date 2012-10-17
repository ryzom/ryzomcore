// Object Viewer Qt - BNP Manager Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Roland Winklmeier <roland.m.winklmeier@googlemail.com>
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

#ifndef BNP_MANAGER_PLUGIN_H
#define BNP_MANAGER_PLUGIN_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "bnp_manager_window.h"

// NeL includes
#include "nel/misc/app_context.h"
#include <nel/misc/debug.h>

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

namespace BNPManager
{
class m_BnpManagerWindow;

class BNPManagerPlugin : public QObject, public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(ExtensionSystem::IPlugin)

public:
	BNPManagerPlugin();
    virtual ~BNPManagerPlugin();

    virtual bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
    virtual void extensionsInitialized();
    virtual void shutdown();
    virtual void setNelContext(NLMISC::INelContext *nelContext);

    void addAutoReleasedObject(QObject *obj);

protected:

    NLMISC::CLibraryContext *m_libContext;

private:

    ExtensionSystem::IPluginManager *m_plugMan;
    QList<QObject *> m_autoReleaseObjects;
};

/**
 * Implementation of the IContext interface
 *
 * \date 2011
 */

class BNPManagerContext : public Core::IContext
{
    Q_OBJECT

public:
	// Constructor
	BNPManagerContext(QObject *parent = 0) : IContext(parent)
	{
		// run new manager window app
		m_BnpManagerWindow = new BNPManagerWindow();
	}

	// Destructor
    virtual ~BNPManagerContext() {}

    virtual QString id() const
    {
        return QLatin1String("BNPManagerContext");
    }
    virtual QString trName() const
    {
        return tr("BNP Manager");
    }
    virtual QIcon icon() const
    {
        return QIcon(":/images/ic_nel_bnp_make.png");
    }

    virtual void open()
	{
		m_BnpManagerWindow->open();
	}

    virtual QUndoStack *undoStack()
	{
		return m_BnpManagerWindow->m_undoStack;
	}

    virtual QWidget *widget()
	{
		return m_BnpManagerWindow;
	}

    BNPManagerWindow *m_BnpManagerWindow;

};

} // namespace Plugin



#endif // BNP_MANAGER_PLUGIN_H
