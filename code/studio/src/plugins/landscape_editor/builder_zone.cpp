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
#include "builder_zone.h"
#include "list_zones_widget.h"
#include "landscape_actions.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>

namespace LandscapeEditor
{
int LandCounter = 0;

ZoneBuilder::ZoneBuilder(LandscapeScene *landscapeScene, ListZonesWidget *listZonesWidget, QUndoStack *undoStack)
	: m_currentZoneRegion(-1),
	  m_pixmapDatabase(0),
	  m_listZonesWidget(listZonesWidget),
	  m_landscapeScene(landscapeScene),
	  m_undoStack(undoStack)
{
	nlassert(m_landscapeScene);
	m_pixmapDatabase = new PixmapDatabase();
	m_lastPathName = "";
}

ZoneBuilder::~ZoneBuilder()
{
	delete m_pixmapDatabase;
}

bool ZoneBuilder::init(const QString &pathName, bool displayProgress)
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

void ZoneBuilder::actionLigoTile(const LigoData &data, const ZonePosition &zonePos)
{
	if (m_undoStack == 0)
		return;

	checkBeginMacro();
	// nlinfo(QString("%1 %2 %3 (%4 %5)").arg(data.zoneName.c_str()).arg(zonePos.x).arg(zonePos.y).arg(data.posX).arg(data.posY).toUtf8().constData());
	m_zonePositionList.push_back(zonePos);
	m_undoStack->push(new LigoTileCommand(data, zonePos, this, m_landscapeScene));
}

void ZoneBuilder::actionLigoMove(uint index, sint32 deltaX, sint32 deltaY)
{
	if (m_undoStack == 0)
		return;

	checkBeginMacro();
	//m_undoStack->push(new LigoMoveCommand(index, deltaX, deltaY, this));
}

void ZoneBuilder::actionLigoResize(uint index, sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	if (m_undoStack == 0)
		return;

	checkBeginMacro();
	// nlinfo(QString("minX=%1 maxX=%2 minY=%3 maxY=%4").arg(newMinX).arg(newMaxX).arg(newMinY).arg(newMaxY).toUtf8().constData());
	m_undoStack->push(new LigoResizeCommand(index, newMinX, newMaxX, newMinY, newMaxY, this));
}

void ZoneBuilder::addZone(sint32 posX, sint32 posY)
{
	// Read-only mode
	if ((m_listZonesWidget == 0) || (m_undoStack == 0))
		return;

	if (m_landscapeMap.empty())
		return;

	// Check zone name
	std::string zoneName = m_listZonesWidget->currentZoneName().toUtf8().constData();
	if (zoneName.empty())
		return;

	BuilderZoneRegion *builderZoneRegion = m_landscapeMap.value(m_currentZoneRegion).builderZoneRegion;
	builderZoneRegion->init(this);

	uint8 rot = uint8(m_listZonesWidget->currentRot());
	uint8 flip = uint8(m_listZonesWidget->currentFlip());

	NLLIGO::CZoneBankElement *zoneBankElement = getZoneBank().getElementByZoneName(zoneName);

	m_titleAction = QString("Add zone %1,%2").arg(posX).arg(posY);
	m_createdAction = false;
	m_zonePositionList.clear();
	if (m_listZonesWidget->isForce())
	{
		builderZoneRegion->addForce(posX, posY, rot, flip, zoneBankElement);
	}
	else
	{
		if (m_listZonesWidget->isNotPropogate())
			builderZoneRegion->addNotPropagate(posX, posY, rot, flip, zoneBankElement);
		else
			builderZoneRegion->add(posX, posY, rot, flip, zoneBankElement);
	}
	checkEndMacro();
}

void ZoneBuilder::addTransition(const sint32 posX, const sint32 posY)
{
	// Read-only mode
	if ((m_listZonesWidget == 0) || (m_undoStack == 0))
		return;

	if (m_landscapeMap.empty())
		return;

	m_titleAction = QString("Transition zone %1,%2").arg(posX).arg(posY);
	m_createdAction = false;
	m_zonePositionList.clear();

	nlinfo(QString("trans %1,%2").arg(posX).arg(posY).toUtf8().constData());

	sint32 x = (sint32)floor(float(posX) / m_landscapeScene->cellSize());
	sint32 y = (sint32)floor(float(posY) / m_landscapeScene->cellSize());
	sint32 k;

	// Detect if we are in a transition square to switch
	BuilderZoneRegion *builderZoneRegion = m_landscapeMap.value(m_currentZoneRegion).builderZoneRegion;
	builderZoneRegion->init(this);
	const NLLIGO::CZoneRegion &zoneRegion = currentZoneRegion()->ligoZoneRegion();
	bool bCutEdgeTouched = false;
	for (uint8 transPos = 0; transPos < 4; ++transPos)
	{
		uint ce = zoneRegion.getCutEdge(x, y, transPos);

		if ((ce > 0) && (ce < 3))
			for (k = 0; k < 2; ++k)
			{
				float xTrans, yTrans;

				if ((transPos == 0) || (transPos == 1))
				{
					if (ce == 1)
						xTrans = m_landscapeScene->cellSize() / 3.0f;
					else
						xTrans = 2.0f * m_landscapeScene->cellSize() / 3.0f;
				}
				else
				{
					if (transPos == 2)
						xTrans = 0;
					else
						xTrans = m_landscapeScene->cellSize();
				}
				xTrans += x * m_landscapeScene->cellSize();

				if ((transPos == 2) || (transPos == 3))
				{
					if (ce == 1)
						yTrans = m_landscapeScene->cellSize() / 3.0f;
					else
						yTrans = 2.0f * m_landscapeScene->cellSize() / 3.0f;
				}
				else
				{
					if (transPos == 1)
						yTrans = 0;
					else
						yTrans = m_landscapeScene->cellSize();
				}
				yTrans += y * m_landscapeScene->cellSize();

				if ((posX >= (xTrans - m_landscapeScene->cellSize() / 12.0f)) &&
						(posX <= (xTrans + m_landscapeScene->cellSize() / 12.0f)) &&
						(posY >= (yTrans - m_landscapeScene->cellSize() / 12.0f)) &&
						(posY <= (yTrans + m_landscapeScene->cellSize() / 12.0f)))
				{
					builderZoneRegion->invertCutEdge (x, y, transPos);
					bCutEdgeTouched = true;
				}
				ce = 3 - ce;
			}
	}

	// If not clicked to change the cutEdge so the user want to change the transition
	if (!bCutEdgeTouched)
	{
		builderZoneRegion->cycleTransition (x, y);
	}
	checkEndMacro();
}

void ZoneBuilder::delZone(const sint32 posX, const sint32 posY)
{
	if ((m_listZonesWidget == 0) || (m_undoStack == 0))
		return;

	if (m_landscapeMap.empty())
		return;

	m_titleAction = QString("Del zone %1,%2").arg(posX).arg(posY);
	m_createdAction = false;

	BuilderZoneRegion *builderZoneRegion = m_landscapeMap.value(m_currentZoneRegion).builderZoneRegion;

	builderZoneRegion->init(this);
	builderZoneRegion->del(posX, posY);
	checkEndMacro();
}

int ZoneBuilder::createZoneRegion()
{
	LandscapeItem landItem;
	landItem.zoneRegionObject = new ZoneRegionObject();
	landItem.builderZoneRegion = new BuilderZoneRegion(LandCounter);
	landItem.builderZoneRegion->init(this);
	landItem.rectItem = m_landscapeScene->createLayerBlackout(landItem.zoneRegionObject->ligoZoneRegion());

	m_landscapeMap.insert(LandCounter, landItem);
	if (m_currentZoneRegion == -1)
		setCurrentZoneRegion(LandCounter);

	calcMask();
	return LandCounter++;
}

int ZoneBuilder::createZoneRegion(const QString &fileName)
{
	LandscapeItem landItem;
	landItem.zoneRegionObject = new ZoneRegionObject();
	landItem.zoneRegionObject->load(fileName.toUtf8().constData());

	if (checkOverlaps(landItem.zoneRegionObject->ligoZoneRegion()))
	{
		delete landItem.zoneRegionObject;
		return -1;
	}
	landItem.builderZoneRegion = new BuilderZoneRegion(LandCounter);
	landItem.builderZoneRegion->init(this);

	m_landscapeScene->addZoneRegion(landItem.zoneRegionObject->ligoZoneRegion());
	landItem.rectItem = m_landscapeScene->createLayerBlackout(landItem.zoneRegionObject->ligoZoneRegion());
	m_landscapeMap.insert(LandCounter, landItem);

	if (m_currentZoneRegion == -1)
		setCurrentZoneRegion(LandCounter);

	calcMask();
	return LandCounter++;
}

void ZoneBuilder::deleteZoneRegion(int id)
{
	if (m_landscapeMap.contains(id))
	{
		if (m_landscapeMap.value(id).rectItem != 0)
			delete m_landscapeMap.value(id).rectItem;
		m_landscapeScene->delZoneRegion(m_landscapeMap.value(id).zoneRegionObject->ligoZoneRegion());
		delete m_landscapeMap.value(id).zoneRegionObject;
		delete m_landscapeMap.value(id).builderZoneRegion;
		m_landscapeMap.remove(id);
		calcMask();
	}
	else
		nlwarning("Landscape (id %i) not found", id);
}

void ZoneBuilder::setCurrentZoneRegion(int id)
{
	if (m_landscapeMap.contains(id))
	{
		if (currentIdZoneRegion() != -1)
		{
			NLLIGO::CZoneRegion &ligoRegion = m_landscapeMap.value(m_currentZoneRegion).zoneRegionObject->ligoZoneRegion();
			m_landscapeMap[m_currentZoneRegion].rectItem = m_landscapeScene->createLayerBlackout(ligoRegion);
		}
		delete m_landscapeMap.value(id).rectItem;
		m_landscapeMap[id].rectItem = 0;
		m_currentZoneRegion = id;
		calcMask();
	}
	else
		nlwarning("Landscape (id %i) not found", id);
}

int ZoneBuilder::currentIdZoneRegion() const
{
	return m_currentZoneRegion;
}

ZoneRegionObject *ZoneBuilder::currentZoneRegion() const
{
	ZoneRegionObject *result = 0;
	if (m_landscapeMap.contains(m_currentZoneRegion))
		result = m_landscapeMap.value(m_currentZoneRegion).zoneRegionObject;

	return result;
}

int ZoneBuilder::countZoneRegion() const
{
	return m_landscapeMap.size();
}

ZoneRegionObject *ZoneBuilder::zoneRegion(int id) const
{
	ZoneRegionObject *result = 0;
	if (m_landscapeMap.contains(id))
		result = m_landscapeMap.value(id).zoneRegionObject;

	return result;
}

bool ZoneBuilder::ligoData(LigoData &data, const ZonePosition &zonePos)
{
	if (m_landscapeMap.contains(zonePos.region))
	{
		m_landscapeMap.value(zonePos.region).zoneRegionObject->ligoData(data, zonePos.x, zonePos.y);
		return true;
	}
	return false;
}

void ZoneBuilder::setLigoData(LigoData &data, const ZonePosition &zonePos)
{
	if (m_landscapeMap.contains(zonePos.region))
		m_landscapeMap.value(zonePos.region).zoneRegionObject->setLigoData(data, zonePos.x, zonePos.y);
}

bool ZoneBuilder::initZoneBank (const QString &pathName)
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

PixmapDatabase *ZoneBuilder::pixmapDatabase() const
{
	return m_pixmapDatabase;
}

QString ZoneBuilder::dataPath() const
{
	return m_lastPathName;
}

bool ZoneBuilder::getZoneMask(sint32 x, sint32 y)
{
	if ((x < m_minX) || (x > m_maxX) ||
			(y < m_minY) || (y > m_maxY))
		return true;
	else
		return m_zoneMask[(x - m_minX) + (y - m_minY) * (1 + m_maxX - m_minX)];
}

void ZoneBuilder::calcMask()
{
	sint32 x, y;

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

	m_zoneMask.resize ((1 + m_maxX - m_minX) * (1 + m_maxY - m_minY));
	sint32 stride = (1 + m_maxX - m_minX);
	for (y = m_minY; y <= m_maxY; ++y)
		for (x = m_minX; x <= m_maxX; ++x)
		{
			m_zoneMask[x - m_minX + (y - m_minY) * stride] = true;

			QMapIterator<int, LandscapeItem> it(m_landscapeMap);
			while (it.hasNext())
			{
				it.next();
				if (int(it.key()) != m_currentZoneRegion)
				{
					const NLLIGO::CZoneRegion &region = it.value().zoneRegionObject->ligoZoneRegion();

					const std::string &rSZone = region.getName (x, y);
					if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
					{
						m_zoneMask[x - m_minX + (y - m_minY) * stride] = false;
					}
				}
			}
		}
}

bool ZoneBuilder::getZoneAmongRegions(ZonePosition &zonePos, BuilderZoneRegion *builderZoneRegionFrom, sint32 x, sint32 y)
{
	QMapIterator<int, LandscapeItem> it(m_landscapeMap);
	while (it.hasNext())
	{
		it.next();
		const NLLIGO::CZoneRegion &region = it.value().zoneRegionObject->ligoZoneRegion();
		if ((x < region.getMinX()) || (x > region.getMaxX()) ||
				(y < region.getMinY()) || (y > region.getMaxY()))
			continue;
		if (region.getName(x, y) != STRING_UNUSED)
		{
			builderZoneRegionFrom = it.value().builderZoneRegion;
			zonePos = ZonePosition(x, y, it.key());
			return true;
		}
	}

	// The zone is not present in other region so it is an empty or oob zone of the current region
	const NLLIGO::CZoneRegion &region = zoneRegion(builderZoneRegionFrom->getRegionId())->ligoZoneRegion();
	if ((x < region.getMinX()) || (x > region.getMaxX()) ||
			(y < region.getMinY()) || (y > region.getMaxY()))
		return false; // Out Of Bound

	zonePos = ZonePosition(x, y, builderZoneRegionFrom->getRegionId());
	return true;
}

void ZoneBuilder::checkBeginMacro()
{
	if (!m_createdAction)
	{
		m_createdAction = true;
		m_undoStack->beginMacro(m_titleAction);
		m_undoScanRegionCommand = new UndoScanRegionCommand(true, this, m_landscapeScene);
		m_undoStack->push(m_undoScanRegionCommand);
	}
}

void ZoneBuilder::checkEndMacro()
{
	if (m_createdAction)
	{
		UndoScanRegionCommand *redoScanRegionCommand = new UndoScanRegionCommand(false, this, m_landscapeScene);

		// Sets list positions in which need apply changes
		m_undoScanRegionCommand->setScanList(m_zonePositionList);
		redoScanRegionCommand->setScanList(m_zonePositionList);

		// Adds command in the stack
		m_undoStack->push(redoScanRegionCommand);
		m_undoStack->endMacro();
	}
}

bool ZoneBuilder::checkOverlaps(const NLLIGO::CZoneRegion &newZoneRegion)
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
						return true;
				}
			}
	}
	return false;
}

} /* namespace LandscapeEditor */
