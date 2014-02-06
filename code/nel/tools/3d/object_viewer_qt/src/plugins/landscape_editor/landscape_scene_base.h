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

#ifndef LANDSCAPE_SCENE_BASE_H
#define LANDSCAPE_SCENE_BASE_H

// Project includes
#include "landscape_editor_global.h"
#include "builder_zone_base.h"
#include "zone_region_editor.h"

// NeL includes
#include <nel/ligo/zone_region.h>

// Qt includes
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>

namespace LandscapeEditor
{

/**
@class LandscapeSceneBase
@brief
@details
*/
class LANDSCAPE_EDITOR_EXPORT LandscapeSceneBase : public QGraphicsScene
{
	Q_OBJECT

public:
	LandscapeSceneBase(int sizeCell = 160, QObject *parent = 0);
	virtual ~LandscapeSceneBase();

	int cellSize() const;
	void setZoneBuilder(ZoneBuilderBase *zoneBuilder);

	QGraphicsItem *createItemZone(const LigoData &data, const ZonePosition &zonePos);
	QGraphicsItem *createItemEmptyZone(const ZonePosition &zonePos);
	void deleteItemZone(const ZonePosition &zonePos);

	void addZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);
	void delZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);

	void snapshot(const QString &fileName, int width, int height, const QRectF &landRect);

	QString zoneNameFromMousePos() const;

public Q_SLOTS:

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
	bool checkUnderZone(const int posX, const int posY);

	int m_cellSize;
	qreal m_mouseX, m_mouseY;
	sint32 m_posX, m_posY;
	ZoneBuilderBase *m_zoneBuilderBase;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_SCENE_BASE_H
