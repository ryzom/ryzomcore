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


#ifndef SIMPLE_VIEWER_H
#define SIMPLE_VIEWER_H

// Project includes
#include "qnel_widget.h"
#include "../core/icore_listener.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QUndoStack>
class QWidget;

namespace Plugin
{

class SimpleViewer : public QWidget
{
	Q_OBJECT
public:
	SimpleViewer(QWidget *parent = 0);
	virtual ~SimpleViewer() {}

	QUndoStack *m_undoStack;
};

class ExampleCoreListener : public Core::ICoreListener
{
	Q_OBJECT
public:
	ExampleCoreListener(QObject *parent = 0): ICoreListener(parent) {}
	virtual ~ExampleCoreListener() {}

	virtual bool closeMainWindow() const;
};

} // namespace Plugin

#endif // SIMPLE_VIEWER_H
