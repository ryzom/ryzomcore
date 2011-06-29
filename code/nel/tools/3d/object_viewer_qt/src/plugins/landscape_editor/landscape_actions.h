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
#include "landscape_scene.h"

// NeL includes

// Qt includes
#include <QtGui/QUndoCommand>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

namespace LandscapeEditor
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

class NewLandscapeCommand: public QUndoCommand
{
public:
	NewLandscapeCommand(QUndoCommand *parent = 0);
	virtual ~NewLandscapeCommand();

	virtual void undo();
	virtual void redo();
private:
};

// Modify the landscape
class LigoTileCommand: public QUndoCommand
{
public:
	LigoTileCommand(const LigoData &data, const ZonePosition &zonePos,
					ZoneBuilder *zoneBuilder, LandscapeScene *scene,
					QUndoCommand *parent = 0);
	virtual ~LigoTileCommand();

	virtual void undo();
	virtual void redo();

private:
	ZonePosition m_zonePos;
	LigoData m_newLigoData;
	LigoData m_oldLigoData;
	ZoneBuilder *m_zoneBuilder;
	LandscapeScene *m_scene;
};

class UndoScanRegionCommand: public QUndoCommand
{
public:
	UndoScanRegionCommand(ZoneBuilder *zoneBuilder, LandscapeScene *scene, QUndoCommand *parent = 0);
	virtual ~UndoScanRegionCommand();

	void setScanList(const QList<ZonePosition> &zonePositionList);
	virtual void undo();
	virtual void redo();

private:

	QList<ZonePosition> m_zonePositionList;
	ZoneBuilder *m_zoneBuilder;
	LandscapeScene *m_scene;
};

class RedoScanRegionCommand: public QUndoCommand
{
public:
	RedoScanRegionCommand(ZoneBuilder *zoneBuilder, LandscapeScene *scene, QUndoCommand *parent = 0);
	virtual ~RedoScanRegionCommand();

	void setScanList(const QList<ZonePosition> &zonePositionList);
	virtual void undo();
	virtual void redo();

private:

	QList<ZonePosition> m_zonePositionList;
	ZoneBuilder *m_zoneBuilder;
	LandscapeScene *m_scene;
};

/*
// Move the landscape
class LigoMoveCommand: public QUndoCommand
{
public:

	LigoMoveCommand(int index, sint32 deltaX, sint32 deltaY, ZoneBuilder *zoneBuilder, QUndoCommand *parent = 0);
	virtual ~LigoMoveCommand();

	virtual void undo();
	virtual void redo();
private:

	int m_index;
	sint32 m_deltaX;
	sint32 m_deltaY;
	ZoneBuilder *m_zoneBuilder;
};
*/
// Modify the landscape
class LigoResizeCommand: public QUndoCommand
{
public:
	LigoResizeCommand(int index, sint32 newMinX, sint32 newMaxX,
					  sint32 newMinY, sint32 newMaxY, ZoneBuilder *zoneBuilder,
					  QUndoCommand *parent = 0);
	virtual ~LigoResizeCommand();

	virtual void undo();
	virtual void redo();

private:
	int m_index;
	sint32 m_newMinX;
	sint32 m_newMaxX;
	sint32 m_newMinY;
	sint32 m_newMaxY;
	NLLIGO::CZoneRegion m_oldZoneRegion;
	ZoneBuilder *m_zoneBuilder;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_ACTIONS_H
