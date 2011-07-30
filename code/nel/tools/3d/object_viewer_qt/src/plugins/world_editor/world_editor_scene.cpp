// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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
#include "world_editor_scene.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QApplication>

namespace WorldEditor
{

WorldEditorScene::WorldEditorScene(int sizeCell, QObject *parent)
	: LandscapeEditor::LandscapeSceneBase(sizeCell, parent)
{
}

WorldEditorScene::~WorldEditorScene()
{
}
/*
void LandscapeSceneBase::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	QGraphicsScene::mousePressEvent(mouseEvent);

	qreal x = mouseEvent->scenePos().x();
	qreal y = mouseEvent->scenePos().y();
	m_posX = sint32(floor(x / m_cellSize));
	m_posY = sint32(-floor(y / m_cellSize));

	m_mouseButton = mouseEvent->button();
}

void LandscapeSceneBase::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	m_mouseX = mouseEvent->scenePos().x();
	m_mouseY = mouseEvent->scenePos().y() - m_cellSize;

	m_posX = sint32(floor(m_mouseX / m_cellSize));
	m_posY = sint32(-floor(m_mouseY / m_cellSize));

	QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void LandscapeSceneBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	QGraphicsScene::mouseReleaseEvent(mouseEvent);
	m_mouseButton = Qt::NoButton;
}
*/

} /* namespace WorldEditor */
