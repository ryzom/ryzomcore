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
#include "imenu_manager.h"

// Qt includes
#include <QtCore/QHash>

namespace Core
{

class MenuManager : public IMenuManager
{
	Q_OBJECT

public:
	MenuManager(QObject *parent = 0);
	virtual ~MenuManager();

	virtual void registerMenu(QMenu *menu, const QString &id);
	virtual void registerAction(QAction *action, const QString &id);

	virtual QMenu *menu(const QString &id) const;
	virtual QAction *action(const QString &id) const;

	virtual void unregisterMenu(const QString &id);
	virtual void unregisterAction(const QString &id);

	virtual QMenuBar *menuBar() const;
	void setMenuBar(QMenuBar *menuBar);
private:
	QMenuBar *_menuBar;
	typedef QHash<QString, QMenu *> IdMenuMap;
	IdMenuMap _menuMap;
	typedef QHash<QString, QAction *> IdActionMap;
	IdActionMap _actionMap;
};

} // namespace Core

#endif // MENU_MANAGER_H
