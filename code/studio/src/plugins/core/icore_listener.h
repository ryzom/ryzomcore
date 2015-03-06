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

#ifndef ICORE_LISTENER_H
#define ICORE_LISTENER_H

// Project includes
#include "core_global.h"

// Qt includes
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Core
{
/**
@interface ICoreListener
@brief The ICoreListener is an interface for providing a hook for plugins to veto on close event emitted from
the core plugin.
@details You implement this interface if you want to prevent the closing of the whole application.
If the application window requests a close, then first ICoreListener::closeMainWindow() is called
(in arbitrary order) on all registered objects implementing this interface.
If one if these calls returns false, the process is aborted and the event is ignored.  If all calls return
true, the corresponding signal is emitted and the event is accepted/performed.

You need to add your implementing object to the plugin managers objects:
PluginManager->addObject(yourImplementingObject);
Don't forget to remove the object again at deconstruction (e.g. in the destructor of
your plugin)
*/
class CORE_EXPORT ICoreListener: public QObject
{
	Q_OBJECT
public:
	ICoreListener(QObject *parent = 0): QObject(parent) {}
	virtual ~ICoreListener() {}

	/// Return false from the implemented method if you want to prevent the event.
	virtual bool closeMainWindow() const = 0;
};

} // namespace Core


#endif // ICORE_LISTENER_H
