// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
// Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#include "core.h"
#include "context_manager.h"
#include "main_window.h"
#include "../../extension_system/iplugin_manager.h"

static Core::CoreImpl *m_coreInstance = 0;

namespace Core
{

ICore *ICore::instance()
{
	return m_coreInstance;
}

CoreImpl::CoreImpl(MainWindow *mainWindow)
{
	m_mainWindow = mainWindow;
	m_coreInstance = this;
}

CoreImpl::~CoreImpl()
{
	m_coreInstance = 0;
}

bool CoreImpl::showOptionsDialog(const QString &group,
								 const QString &page,
								 QWidget *parent)
{
	return m_mainWindow->showOptionsDialog(group, page, parent);
}

MenuManager *CoreImpl::menuManager() const
{
	return m_mainWindow->menuManager();
}

ContextManager *CoreImpl::contextManager() const
{
	return m_mainWindow->contextManager();
}

QSettings *CoreImpl::settings() const
{
	return m_mainWindow->settings();
}

QMainWindow *CoreImpl::mainWindow() const
{
	return m_mainWindow;
}

ExtensionSystem::IPluginManager *CoreImpl::pluginManager() const
{
	return m_mainWindow->pluginManager();
}

} // namespace Core
