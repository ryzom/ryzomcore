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

#ifndef CORE_H
#define CORE_H

#include "icore.h"

namespace Core
{
class MainWindow;

class CoreImpl : public ICore
{
	Q_OBJECT

public:
	explicit CoreImpl(MainWindow *mainWindow);
	virtual ~CoreImpl();

	virtual bool showOptionsDialog(const QString &group = QString(),
								   const QString &page = QString(),
								   QWidget *parent = 0);

	virtual MenuManager *menuManager() const;
	virtual ContextManager *contextManager() const;

	virtual QSettings *settings() const;
	virtual QMainWindow *mainWindow() const;

	virtual ExtensionSystem::IPluginManager *pluginManager() const;
private:
	MainWindow *m_mainWindow;
	friend class MainWindow;
};

} // namespace Core

#endif // CORE_H
