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
#include <nel/ligo/zone_region.h>

// Qt includes
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QUndoStack>

namespace LandscapeEditor
{
class ZoneBuilder;
class ListZonesWidget;

// Data
struct LigoData
{
	sint32			PosX;
	sint32			PosY;
	uint8			Rot;
	uint8			Flip;
	std::string		ZoneName;
	std::string		SharingMatNames[4];
	uint8			SharingCutEdges[4];
	bool operator!= (const LigoData& other) const
	{
		return (PosX != other.PosX) ||
			   (PosY != other.PosY) ||
			   (Rot != other.Rot) ||
			   (Flip != other.Flip) ||
			   (ZoneName != other.ZoneName) ||
			   (SharingMatNames[0] != other.SharingMatNames[0]) ||
			   (SharingMatNames[1] != other.SharingMatNames[1]) ||
			   (SharingMatNames[2] != other.SharingMatNames[2]) ||
			   (SharingMatNames[3] != other.SharingMatNames[3]) ||
			   (SharingCutEdges[0] != other.SharingCutEdges[0]) ||
			   (SharingCutEdges[1] != other.SharingCutEdges[1]) ||
			   (SharingCutEdges[2] != other.SharingCutEdges[2]) ||
			   (SharingCutEdges[3] != other.SharingCutEdges[3]);
	}
};

class LandscapeScene : public QGraphicsScene
{
	Q_OBJECT

public:
	LandscapeScene(QUndoStack *undoStack, ListZonesWidget *listZonesWidget, ZoneBuilder *zoneBuilder, QObject *parent = 0);
	virtual ~LandscapeScene();

	int cellSize() const;

	QGraphicsItem *createZoneItem(const LigoData &data);
	void processZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);
	void setCurrentZoneRegion(NLLIGO::CZoneRegion *zoneRegion);

	void snapshot(const QString &fileName, int sizeSource);
	void snapshot(const QString &fileName, int width, int height);

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:

	int m_cellSize;
	ListZonesWidget *m_listZonesWidget;
	QUndoStack *m_undoStack;
	ZoneBuilder *m_zoneBuilder;
	NLLIGO::CZoneRegion *m_zoneRegion;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_SCENE_H
