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

#ifndef BUILDER_ZONE_BASE_H
#define BUILDER_ZONE_BASE_H

// Project includes
#include "landscape_editor_global.h"

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
#include <QtGui/QGraphicsRectItem>

namespace LandscapeEditor
{
class LandscapeSceneBase;
class PixmapDatabase;
class ZoneRegionObject;

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
@class ZoneBuilderBase
@brief ZoneBuilderBase contains all the shared data between the tools and the engine.
@details ZoneBank contains the macro zones that is composed of several zones plus a mask.
PixmapDatabase contains the graphics for the zones
*/
class LANDSCAPE_EDITOR_EXPORT ZoneBuilderBase
{
public:
	explicit ZoneBuilderBase(LandscapeSceneBase *landscapeScene);
	virtual ~ZoneBuilderBase();

	/// Init zoneBank and init zone pixmap database
	bool init(const QString &pathName, bool displayProgress = false);

	/// Zone Region
	/// @{
	int loadZoneRegion(const QString &fileName, int defaultId = -1);
	void deleteZoneRegion(int id);
	int countZoneRegion() const;
	ZoneRegionObject *zoneRegion(int id) const;
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

	void calcMask();

	bool checkOverlaps(const NLLIGO::CZoneRegion &newZoneRegion);

	struct LandscapeItem
	{
		ZoneRegionObject *zoneRegionObject;
	};

	sint32 m_minX, m_maxX, m_minY, m_maxY;

	QString m_lastPathName;

	QMap<int, LandscapeItem> m_landscapeMap;

	PixmapDatabase *m_pixmapDatabase;
	NLLIGO::CZoneBank m_zoneBank;
	LandscapeSceneBase *m_landscapeSceneBase;
};

} /* namespace LandscapeEditor */

#endif // BUILDER_ZONE_BASE_H
