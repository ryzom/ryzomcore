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
#include "zone_region_editor.h"
#include "builder_zone.h"
#include "landscape_editor_global.h"

// NeL includes
#include <nel/ligo/zone_region.h>

// Qt includes
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>

namespace LandscapeEditor
{

/**
@class LandscapeScene
@brief
@details
*/
class LandscapeScene : public QGraphicsScene
{
	Q_OBJECT

public:
	LandscapeScene(int sizeCell = 160, QObject *parent = 0);
	virtual ~LandscapeScene();

	int cellSize() const;
	void setZoneBuilder(ZoneBuilder *zoneBuilder);

	QGraphicsItem *createItemZone(const LigoData &data, const ZonePosition &zonePos);
	QGraphicsItem *createItemEmptyZone(const ZonePosition &zonePos);
	QGraphicsRectItem *createLayerBlackout(const NLLIGO::CZoneRegion &zoneRegion);
	void deleteItemZone(const ZonePosition &zonePos);

	void addZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);
	void delZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);

	void snapshot(const QString &fileName, int width, int height, const QRectF &landRect);

	QString zoneNameFromMousePos() const;
	bool transitionMode() const;

public Q_SLOTS:
	void setTransitionMode(bool enabled);

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void drawForeground(QPainter *painter, const QRectF &rect);

	void drawTransition(QPainter *painter, const QRectF &rect);

private:
	bool checkUnderZone(const int posX, const int posY);

	int m_cellSize;
	bool m_transitionMode;
	qreal m_mouseX, m_mouseY;
	sint32 m_posX, m_posY;
	Qt::MouseButton m_mouseButton;
	ZoneBuilder *m_zoneBuilder;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_SCENE_H
