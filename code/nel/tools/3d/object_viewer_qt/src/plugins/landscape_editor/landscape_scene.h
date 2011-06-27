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

// NeL includes
#include <nel/ligo/zone_region.h>

// Qt includes
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>

namespace LandscapeEditor
{

class LandscapeScene : public QGraphicsScene
{
	Q_OBJECT

public:
	LandscapeScene(QObject *parent = 0);
	virtual ~LandscapeScene();

	int cellSize() const;
	void setZoneBuilder(ZoneBuilder *zoneBuilder);

	QGraphicsItem *createZoneItem(const LigoData &data, const ZonePosition &zonePos);
	QGraphicsItem *createEmptyZoneItem(const ZonePosition &zonePos);
	void deleteZoneItem(const ZonePosition &zonePos);
	void processZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);

	void snapshot(const QString &fileName, int sizeSource);
	void snapshot(const QString &fileName, int width, int height);

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
	void checkUnderZone(const LigoData &data, const ZonePosition &zonePos);

	int m_cellSize;
	ZoneBuilder *m_zoneBuilder;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_SCENE_H
