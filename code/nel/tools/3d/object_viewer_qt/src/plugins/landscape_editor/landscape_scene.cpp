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

// Project includes
#include "landscape_scene.h"
#include "builder_zone.h"
#include "landscape_actions.h"
#include "list_zones_widget.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QGraphicsPixmapItem>

namespace LandscapeEditor
{

LandscapeScene::LandscapeScene(QUndoStack *undoStack, ListZonesWidget *listZonesWidget, ZoneBuilder *zoneBuilder, QObject *parent)
	: QGraphicsScene(parent),
	  m_undoStack(undoStack),
	  m_listZonesWidget(listZonesWidget),
	  m_zoneBuilder(zoneBuilder)
{
	m_cellSize = 160;
}

LandscapeScene::~LandscapeScene()
{
}

void LandscapeScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (mouseEvent->button() != Qt::LeftButton)
		return;

	qreal x = mouseEvent->scenePos().rx();
	qreal y = mouseEvent->scenePos().ry();
	if ((x < 0) || (y < 0))
		return;

	LigoData ligoData = m_listZonesWidget->currentLigoData();
	if (ligoData.ZoneName == "")
		return;

	ligoData.PosX = m_cellSize * int(x / m_cellSize);;
	ligoData.PosY = m_cellSize * int(y / m_cellSize);
	ligoData.Scale = m_cellSize / 256.0;

	LigoTileCommand *action = new LigoTileCommand(ligoData, m_zoneBuilder, this);
	m_undoStack->push(action);

	QGraphicsScene::mousePressEvent(mouseEvent);
}

} /* namespace LandscapeEditor */
