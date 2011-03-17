// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// Project includes
#include "menu_manager.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

namespace Core
{
MenuManager::MenuManager(QObject *parent)
	: IMenuManager(parent),
	  _menuBar(0)
{
}

MenuManager::~MenuManager()
{
	_menuMap.clear();
}

void MenuManager::registerMenu(QMenu *menu, const QString &id)
{
	menu->setObjectName(id);
	_menuMap.insert(id, menu);
}

void MenuManager::registerAction(QAction *action, const QString &id)
{
	action->setObjectName(id);
	_actionMap.insert(id, action);
}

QMenu *MenuManager::menu(const QString &id) const
{
	QMenu *result = 0;
	if (!_menuMap.contains(id))
		nlwarning("QMenu %s not found", id.toStdString().c_str());
	else
		result = _menuMap.value(id);
	return result;
}

QAction *MenuManager::action(const QString &id) const
{
	QAction *result = 0;
	if (!_actionMap.contains(id))
		nlwarning("QAction %s not found", id.toStdString().c_str());
	else
		result = _actionMap.value(id);
	return result;
}

void MenuManager::unregisterMenu(const QString &id)
{
	_menuMap.remove(id);
}

void MenuManager::unregisterAction(const QString &id)
{
	_actionMap.remove(id);
}

QMenuBar *MenuManager::menuBar() const
{
	return _menuBar;
}

void MenuManager::setMenuBar(QMenuBar *menuBar)
{
	_menuBar = menuBar;
}

} /* namespace Core */