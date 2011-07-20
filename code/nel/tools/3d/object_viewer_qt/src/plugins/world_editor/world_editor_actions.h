// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef WORLD_EDITOR_ACTIONS_H
#define WORLD_EDITOR_ACTIONS_H

// Project includes

// NeL includes

// Qt includes
#include <QtGui/QUndoCommand>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

namespace WorldEditor
{

class OpenLandscapeCommand: public QUndoCommand
{
public:
	OpenLandscapeCommand(const QString &fileName, QUndoCommand *parent = 0);
	virtual ~OpenLandscapeCommand();

	virtual void undo();
	virtual void redo();
private:

	QString m_fileName;
};

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_ACTIONS_H
