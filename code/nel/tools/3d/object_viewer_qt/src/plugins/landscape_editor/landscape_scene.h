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

#ifndef LANDSCAPE_SCENE_H
#define LANDSCAPE_SCENE_H

// Project includes

// NeL includes

// Qt includes
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QUndoStack>

namespace LandscapeEditor
{
class ZoneBuilder;
class ListZonesWidget;

class LandscapeScene : public QGraphicsScene
{
	Q_OBJECT

public:
	LandscapeScene(QUndoStack *undoStack, ListZonesWidget *listZonesWidget, ZoneBuilder *zoneBuilder, QObject *parent = 0);
	virtual ~LandscapeScene();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
	void createBackgroundPixmap();

	int m_cellSize;
	ListZonesWidget *m_listZonesWidget;
	QUndoStack *m_undoStack;
	ZoneBuilder *m_zoneBuilder;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_SCENE_H
