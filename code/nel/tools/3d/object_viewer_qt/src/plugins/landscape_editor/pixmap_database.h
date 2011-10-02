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

#ifndef PIXMAP_DATABASE_H
#define PIXMAP_DATABASE_H

// Project includes
#include "landscape_editor_global.h"

// NeL includes
#include <nel/ligo/zone_bank.h>

// Qt includes
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtGui/QPixmap>

namespace LandscapeEditor
{

/**
@class PixmapDatabase
@brief PixmapDatabase contains the image database
@details
*/
class LANDSCAPE_EDITOR_EXPORT PixmapDatabase
{
public:
	explicit PixmapDatabase(int textureSize = 256);
	~PixmapDatabase();

	/// Load all images(png) from zonePath, list images gets from zoneBank
	bool loadPixmaps(const QString &zonePath, NLLIGO::CZoneBank &zoneBank, bool displayProgress = false);

	/// Unload all images
	void reset();

	/// Get list names all loaded pixmaps
	QStringList listPixmaps() const;

	/// Get original pixmap
	/// @return QPixmap* if the image is in the database ;
	/// otherwise returns pixmap which contains error message.
	QPixmap *pixmap(const QString &zoneName) const;

	int textureSize() const;

private:

	int m_textureSize;
	QPixmap *m_errorPixmap;
	QMap<QString, QPixmap *> m_pixmapMap;
};

} /* namespace LandscapeEditor */

#endif // PIXMAP_DATABASE_H
