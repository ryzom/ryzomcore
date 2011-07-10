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

#ifndef BUILDER_ZONE_H
#define BUILDER_ZONE_H

// Project includes
#include "builder_zone_region.h"
#include "zone_region_editor.h"
#include "pixmap_database.h"

// NeL includes
#include <nel/ligo/zone_bank.h>
#include <nel/ligo/zone_region.h>

// STL includes
#include <string>
#include <vector>

// Qt includes
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QPixmap>
#include <QtGui/QUndoStack>
#include <QtGui/QGraphicsRectItem>

namespace LandscapeEditor
{
class ListZonesWidget;
class LandscapeScene;
class UndoScanRegionCommand;

// Data
struct ZonePosition
{
	// Absolute position
	sint32 x;
	sint32 y;
	int region;

	ZonePosition()
	{
		x = 0xffffffff;
		y = 0xffffffff;
		region = -1;
	}

	ZonePosition(const sint32 posX, const sint32 posY, const int id)
	{
		x = posX;
		y = posY;
		region = id;
	}
};

/**
@class ZoneBuilder
@brief ZoneBuilder contains all the shared data between the tools and the engine.
@details ZoneBank contains the macro zones that is composed of several zones plus a mask.
PixmapDatabase contains the graphics for the zones
*/
class ZoneBuilder
{
public:
	ZoneBuilder(LandscapeScene *landscapeScene, ListZonesWidget *listZonesWidget = 0, QUndoStack *undoStack = 0);
	~ZoneBuilder();

	/// Init zoneBank and init zone pixmap database
	bool init(const QString &pathName, bool bMakeAZone);

	void calcMask();
	bool getZoneMask (sint32 x, sint32 y);
	bool getZoneAmongRegions(ZonePosition &zonePos, BuilderZoneRegion *builderZoneRegionFrom, sint32 x, sint32 y);

	/// Ligo Actions
	/// @{
	void actionLigoTile(const LigoData &data, const ZonePosition &zonePos);
	void actionLigoMove(uint index, sint32 deltaX, sint32 deltaY);
	void actionLigoResize(uint index, sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY);
	/// @}

	/// Zone Bricks
	/// @{
	void addZone(sint32 posX, sint32 posY);
	void addTransition(const sint32 posX, const sint32 posY);
	void delZone(const sint32 posX, const sint32 posY);
	/// @}

	/// Zone Region
	/// @{
	int createZoneRegion();
	int createZoneRegion(const QString &fileName);
	void deleteZoneRegion(int id);
	void setCurrentZoneRegion(int id);
	int currentIdZoneRegion() const;
	ZoneRegionObject *currentZoneRegion() const;
	int countZoneRegion() const;
	ZoneRegionObject *zoneRegion(int id) const;
	void ligoData(LigoData &data, const ZonePosition &zonePos);
	void setLigoData(LigoData &data, const ZonePosition &zonePos);
	/// @}

	// Accessors
	NLLIGO::CZoneBank &getZoneBank()
	{
		return m_zoneBank;
	}

	PixmapDatabase *pixmapDatabase() const;

	QString dataPath() const;

private:

	/// Scan ./zoneligos dir and add all *.ligozone files to zoneBank
	bool initZoneBank (const QString &path);

	void checkBeginMacro();
	void checkEndMacro();

	bool checkOverlaps(const NLLIGO::CZoneRegion &newZoneRegion);

	struct LandscapeItem
	{
		BuilderZoneRegion *builderZoneRegion;
		ZoneRegionObject *zoneRegionObject;
		QGraphicsRectItem *rectItem;
	};

	sint32 m_minX, m_maxX, m_minY, m_maxY;
	std::vector<bool> m_zoneMask;

	QString m_lastPathName;

	int m_currentZoneRegion;
	//std::vector<LandscapeItem> m_landscapeItems;
	QMap<int, LandscapeItem> m_landscapeMap;

	bool m_createdAction;
	QString m_titleAction;
	QList<ZonePosition> m_zonePositionList;
	UndoScanRegionCommand *m_undoScanRegionCommand;

	PixmapDatabase *m_pixmapDatabase;
	NLLIGO::CZoneBank m_zoneBank;
	ListZonesWidget *m_listZonesWidget;
	LandscapeScene *m_landscapeScene;
	QUndoStack *m_undoStack;
};

} /* namespace LandscapeEditor */

#endif // BUILDER_ZONE_H
