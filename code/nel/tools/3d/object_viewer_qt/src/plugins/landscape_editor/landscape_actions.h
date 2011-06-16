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

#ifndef LANDSCAPE_ACTIONS_H
#define LANDSCAPE_ACTIONS_H

// Project includes
#include "builder_zone.h"

// NeL includes

// Qt includes
#include <QtGui/QUndoCommand>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsPixmapItem>

namespace LandscapeEditor
{
class ZoneBuilder;

class OpenLandscapeCommand: public QUndoCommand
{
public:
	OpenLandscapeCommand(const QString &fileName, QUndoCommand *parent = 0);
	~OpenLandscapeCommand();
	virtual void undo();
	virtual void redo();
private:

	QString m_fileName;
};

class NewLandscapeCommand: public QUndoCommand
{
public:
	NewLandscapeCommand(QUndoCommand *parent = 0);
	~NewLandscapeCommand();
	virtual void undo();
	virtual void redo();
private:
};

class LigoTileCommand: public QUndoCommand
{
public:
	LigoTileCommand(const LigoData &data, ZoneBuilder *zoneBuilder, QGraphicsScene *scene, QUndoCommand *parent = 0);
	~LigoTileCommand();

	virtual void undo();
	virtual void redo();

private:

	LigoData m_ligoData;
	QGraphicsPixmapItem *m_item;
	ZoneBuilder *m_zoneBuilder;
	QGraphicsScene *m_scene;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_ACTIONS_H
