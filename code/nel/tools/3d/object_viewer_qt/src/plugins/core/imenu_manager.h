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

#ifndef IMENU_MANAGER_H
#define IMENU_MANAGER_H

#include "core_global.h"

#include <QtCore/QObject>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
class QString;
class QMenuBar;
QT_END_NAMESPACE

namespace Core
{
/*
@interface IMenuManager
@brief The IMenuManager is an interface for providing a registration of menus and menu item.
@details The IMenuManager provides centralized access to menus and menu items.
All menus and menu items should be registered in the IMenuManager.
*/
class CORE_EXPORT IMenuManager : public QObject
{
	Q_OBJECT
public:
	IMenuManager(QObject *parent = 0): QObject(parent) {}
	virtual ~IMenuManager() {}

	virtual void registerMenu(QMenu *menu, const QString &id) = 0;
	virtual void registerAction(QAction *action, const QString &id) = 0;

	virtual QMenu *menu(const QString &id) const = 0;
	virtual QAction *action(const QString &id) const = 0;

	virtual void unregisterMenu(const QString &id) = 0;
	virtual void unregisterAction(const QString &id) = 0;

	virtual QMenuBar *menuBar() const = 0;
};

} // namespace Core

#endif // IMENU_MANAGER_H
