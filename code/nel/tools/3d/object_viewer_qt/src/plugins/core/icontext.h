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

#ifndef ICONTEXT_H
#define ICONTEXT_H

// Project includes
#include "core_global.h"

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE
class QWidget;
class QUndoStack;
QT_END_NAMESPACE

namespace Core
{
/**
@interface IContext
@brief The IContext is an interface for providing tab pages in main window.
@details You need to subclass this interface and put an instance of your subclass
  into the plugin manager object pool.
*/
class CORE_EXPORT IContext: public QObject
{
	Q_OBJECT
public:
	IContext(QObject *parent = 0): QObject(parent) {}
	virtual ~IContext() {}

	/// id() is a unique identifier for referencing this page
	virtual QString id() const = 0;

	/// trName() is the (translated) name for display.
	virtual QString trName() const = 0;

	/// icon() is the icon for display
	virtual QIcon icon() const = 0;

	/// The widget will be destroyed by the widget hierarchy when the main window closes
	virtual QWidget *widget() = 0;

	virtual QUndoStack *undoStack() = 0;

	virtual void open() = 0;

	virtual void save(){}

	virtual void saveAs(){}

	virtual void newDocument(){}

	virtual void close(){}
};

} // namespace Core

#endif // ICONTEXT_H
