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

#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

// Project includes
#include "core_global.h"

// Qt includes
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

namespace Core
{
struct MenuManagerPrivate;

/*
@interface MenuManager
@brief The MenuManager provide the interface for registration of menus and menu item.
@details The MenuManager provides centralized access to menus and menu items.
All menus and menu items should be registered in the MenuManager.
*/
class CORE_EXPORT MenuManager: public QObject
{
	Q_OBJECT

public:
	MenuManager(QMenuBar *menuBar, QObject *parent = 0);
	virtual ~MenuManager();

	void registerMenu(QMenu *menu, const QString &id);
	void registerAction(QAction *action, const QString &id);

	QMenu *menu(const QString &id) const;
	QAction *action(const QString &id) const;

	void unregisterMenu(const QString &id);
	void unregisterAction(const QString &id);

	QMenuBar *menuBar() const;
private:

	MenuManagerPrivate *d;
};

} // namespace Core

#endif // MENU_MANAGER_H
