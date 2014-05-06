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

#ifndef ICORE_H
#define ICORE_H

#include "core_global.h"

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QMainWindow;
class QSettings;
QT_END_NAMESPACE

namespace ExtensionSystem
{
class IPluginManager;
}

namespace Core
{
class MenuManager;
class ContextManager;

class CORE_EXPORT ICore : public QObject
{
	Q_OBJECT

public:
	ICore() {}
	virtual ~ICore() {}

	static ICore *instance();

	virtual bool showOptionsDialog(const QString &group = QString(),
								   const QString &page = QString(),
								   QWidget *parent = 0) = 0;

	virtual MenuManager *menuManager() const = 0;
	virtual ContextManager *contextManager() const = 0;

	virtual QSettings *settings() const = 0;
	virtual QMainWindow *mainWindow() const = 0;

	virtual ExtensionSystem::IPluginManager *pluginManager() const = 0;

Q_SIGNALS:
	void changeSettings();
	void closeMainWindow();
};

} // namespace Core

#endif // ICORE_H
