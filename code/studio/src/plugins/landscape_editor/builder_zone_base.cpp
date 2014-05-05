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
#include "builder_zone_base.h"
#include "landscape_scene_base.h"
#include "zone_region_editor.h"
#include "pixmap_database.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>

namespace LandscapeEditor
{
int NewLandId = 0;

ZoneBuilderBase::ZoneBuilderBase(LandscapeSceneBase *landscapeScene)
	: m_pixmapDatabase(0),
	  m_landscapeSceneBase(landscapeScene)
{
	nlassert(m_landscapeSceneBase);
	m_pixmapDatabase = new PixmapDatabase();
	m_lastPathName = "";
}

ZoneBuilderBase::~ZoneBuilderBase()
{
	delete m_pixmapDatabase;
}

bool ZoneBuilderBase::init(const QString &pathName, bool displayProgress)
{
	if (pathName.isEmpty())
		return false;
	if (pathName != m_lastPathName)
	{
		m_lastPathName = pathName;
		QString zoneBankPath = pathName;
		zoneBankPath += "/zoneligos/";

		// Init the ZoneBank
		m_zoneBank.reset();
		if (!initZoneBank (zoneBankPath))
		{
			m_zoneBank.reset();
			return false;
		}
		// Construct the DataBase from the ZoneBank
		QString zoneBitmapPath = pathName;
		zoneBitmapPath += "/zonebitmaps/";
		m_pixmapDatabase->reset();
		if (!m_pixmapDatabase->loadPixmaps(zoneBitmapPath, m_zoneBank, displayProgress))
		{
			m_zoneBank.reset();
			return false;
		}
	}
	return true;
}

int ZoneBuilderBase::loadZoneRegion(const QString &fileName, int defaultId)
{
	LandscapeItem landItem;
	landItem.zoneRegionObject = new ZoneRegionObject();
	landItem.zoneRegionObject->load(fileName.toUtf8().constData());

	if (!checkOverlaps(landItem.zoneRegionObject->ligoZoneRegion()))
	{
		delete landItem.zoneRegionObject;
		return -1;
	}
	int id = defaultId;
	if (id == -1)
		id = NewLandId++;
//	landItem.builderZoneRegion = new BuilderZoneRegion(LandCounter);
//	landItem.builderZoneRegion->init(this);

	m_landscapeSceneBase->addZoneRegion(landItem.zoneRegionObject->ligoZoneRegion());
//	landItem.rectItem = m_landscapeScene->createLayerBlackout(landItem.zoneRegionObject->ligoZoneRegion());
	m_landscapeMap.insert(id, landItem);

	calcMask();
	return id;
}

void ZoneBuilderBase::deleteZoneRegion(int id)
{
	if (m_landscapeMap.contains(id))
	{
		m_landscapeSceneBase->delZoneRegion(m_landscapeMap.value(id).zoneRegionObject->ligoZoneRegion());
		delete m_landscapeMap.value(id).zoneRegionObject;
//		delete m_landscapeMap.value(id).builderZoneRegion;
		m_landscapeMap.remove(id);
		calcMask();
	}
	else
		nlwarning("Landscape (id %i) not found", id);
}

int ZoneBuilderBase::countZoneRegion() const
{
	return m_landscapeMap.size();
}

ZoneRegionObject *ZoneBuilderBase::zoneRegion(int id) const
{
	return m_landscapeMap.value(id).zoneRegionObject;
}

bool ZoneBuilderBase::initZoneBank (const QString &pathName)
{
	QDir *dir = new QDir(pathName);
	QStringList filters;
	filters << "*.ligozone";

	// Find all ligozone files in dir
	QStringList listFiles = dir->entryList(filters, QDir::Files);

	std::string error;
	Q_FOREACH(QString file, listFiles)
	{
		//nlinfo(file.toUtf8().constData());
		if (!m_zoneBank.addElement((pathName + file).toUtf8().constData(), error))
			QMessageBox::critical(0, QObject::tr("Landscape editor"), QString(error.c_str()), QMessageBox::Ok);
	}
	delete dir;
	return true;
}

PixmapDatabase *ZoneBuilderBase::pixmapDatabase() const
{
	return m_pixmapDatabase;
}

QString ZoneBuilderBase::dataPath() const
{
	return m_lastPathName;
}

void ZoneBuilderBase::calcMask()
{
	m_minY = m_minX = 1000000;
	m_maxY = m_maxX = -1000000;

	if (m_landscapeMap.size() == 0)
		return;

	QMapIterator<int, LandscapeItem> i(m_landscapeMap);
	while (i.hasNext())
	{
		i.next();
		const NLLIGO::CZoneRegion &region = i.value().zoneRegionObject->ligoZoneRegion();

		if (m_minX > region.getMinX())
			m_minX = region.getMinX();
		if (m_minY > region.getMinY())
			m_minY = region.getMinY();
		if (m_maxX < region.getMaxX())
			m_maxX = region.getMaxX();
		if (m_maxY < region.getMaxY())
			m_maxY = region.getMaxY();
	}
}

bool ZoneBuilderBase::checkOverlaps(const NLLIGO::CZoneRegion &newZoneRegion)
{
	QMapIterator<int, LandscapeItem> it(m_landscapeMap);
	while (it.hasNext())
	{
		it.next();
		const NLLIGO::CZoneRegion &zoneRegion = it.value().zoneRegionObject->ligoZoneRegion();
		for (sint32 y = zoneRegion.getMinY(); y <= zoneRegion.getMaxY(); ++y)
			for (sint32 x = zoneRegion.getMinX(); x <= zoneRegion.getMaxX(); ++x)
			{
				const std::string &refZoneName = zoneRegion.getName(x, y);
				if (refZoneName != STRING_UNUSED)
				{
					const std::string &zoneName = newZoneRegion.getName(x, y);
					if ((zoneName != STRING_UNUSED) && (zoneName != STRING_OUT_OF_BOUND))
						return false;
				}
			}
	}
	return true;
}

} /* namespace LandscapeEditor */
