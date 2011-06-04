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

// NeL includes
#include <nel/ligo/zone_bank.h>

// STL includes
#include <string>
#include <vector>

// Qt includes
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtGui/QPixmap>

namespace LandscapeEditor
{

/**
@class PixmapDatabase
@brief PixmapDatabase contains the image database
@details
*/
class PixmapDatabase
{
public:
	PixmapDatabase();
	~PixmapDatabase();

	/// Load all images(png) from zonePath, list images gets from zoneBank
	bool loadPixmaps(const QString &zonePath, NLLIGO::CZoneBank &zoneBank);

	/// Unload all images
	void reset();

	/// Get list names all loaded pixmaps
	QStringList listPixmaps() const;

	/// Get original pixmap
	/// @return QPixmap* if the image is in the database ; otherwise returns 0.
	QPixmap *pixmap(const QString &zoneName) const;
private:

	QMap<QString, QPixmap*> m_pixmapMap;
};


/**
@class ZoneBuilder
@brief ZoneBuilder contains all the shared data between the tools and the engine
@details ZoneBank contains the macro zones that is composed of several zones plus a mask
PixmapDatabase contains the graphics for the zones
*/
class ZoneBuilder
{
public:
	ZoneBuilder();
	~ZoneBuilder();

	// Init zoneBank and init zone pixmap database
	bool init(const QString &pathName, bool bMakeAZone);

	void newZone(bool bDisplay=true);

	// Accessors
	NLLIGO::CZoneBank &getZoneBank()
	{
		return m_zoneBank;
	}

	PixmapDatabase *pixmapDatabase() const;

	QString dataPath() const;

private:

	// Scan ./zoneligos dir and add all *.ligozone files to zoneBank
	bool initZoneBank (const QString &path);

	sint32 m_minX, m_maxX, m_minY, m_maxY;
	QString m_lastPathName;

	PixmapDatabase *m_pixmapDatabase;
	NLLIGO::CZoneBank m_zoneBank;
	std::vector<NLLIGO::CZoneBankElement*> m_currentSelection;
};

} /* namespace LandscapeEditor */

#endif // BUILDER_ZONE_H
