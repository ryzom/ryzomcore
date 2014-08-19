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
#include "landscape_actions.h"
#include "builder_zone.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes

namespace LandscapeEditor
{

LigoTileCommand::LigoTileCommand(const LigoData &data, const ZonePosition &zonePos,
								 ZoneBuilder *zoneBuilder, LandscapeScene *scene,
								 QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_zoneBuilder(zoneBuilder),
	  m_scene(scene)
{
	// Backup position
	m_zonePos = zonePos;

	// Backup new data
	m_newLigoData = data;

	// Backup old data
	m_zoneBuilder->ligoData(m_oldLigoData, m_zonePos);
}

LigoTileCommand::~LigoTileCommand()
{
}

void LigoTileCommand::undo ()
{
	m_zoneBuilder->setLigoData(m_oldLigoData, m_zonePos);
}

void LigoTileCommand::redo ()
{
	m_zoneBuilder->setLigoData(m_newLigoData, m_zonePos);
}

UndoScanRegionCommand::UndoScanRegionCommand(bool direction, ZoneBuilder *zoneBuilder, LandscapeScene *scene, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_direction(direction),
	  m_zoneBuilder(zoneBuilder),
	  m_scene(scene)
{
}

UndoScanRegionCommand::~UndoScanRegionCommand()
{
	m_zonePositionList.clear();
}

void UndoScanRegionCommand::setScanList(const QList<ZonePosition> &zonePositionList)
{
	m_zonePositionList = zonePositionList;
}

void UndoScanRegionCommand::undo()
{
	if (m_direction)
		applyChanges();
}

void UndoScanRegionCommand::redo()
{
	if (!m_direction)
		applyChanges();
}

void UndoScanRegionCommand::applyChanges()
{
	for (int i = 0; i < m_zonePositionList.size(); ++i)
		m_scene->deleteItemZone(m_zonePositionList.at(i));

	for (int i = 0; i < m_zonePositionList.size(); ++i)
	{
		LigoData data;
		m_zoneBuilder->ligoData(data, m_zonePositionList.at(i));
		m_scene->createItemZone(data, m_zonePositionList.at(i));
	}
}

LigoResizeCommand::LigoResizeCommand(int index, sint32 newMinX, sint32 newMaxX,
									 sint32 newMinY, sint32 newMaxY, ZoneBuilder *zoneBuilder,
									 QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_zoneBuilder(zoneBuilder)
{
	m_index = index;
	m_newMinX = newMinX;
	m_newMaxX = newMaxX;
	m_newMinY = newMinY;
	m_newMaxY = newMaxY;

	// Backup old region zone
	m_oldZoneRegion = m_zoneBuilder->zoneRegion(m_index)->ligoZoneRegion();
}

LigoResizeCommand::~LigoResizeCommand()
{
}

void LigoResizeCommand::undo ()
{
	// Restore old region zone
	m_zoneBuilder->zoneRegion(m_index)->setLigoZoneRegion(m_oldZoneRegion);
}

void LigoResizeCommand::redo ()
{
	// Get the zone region
	NLLIGO::CZoneRegion &region = m_zoneBuilder->zoneRegion(m_index)->ligoZoneRegion();

	sint32 i, j;
	std::vector<LigoData> newZones;
	newZones.resize((1 + m_newMaxX - m_newMinX) * (1 + m_newMaxY - m_newMinY));

	sint32 newStride = 1 + m_newMaxX - m_newMinX;
	sint32 Stride = 1 + region.getMaxX() - region.getMinX();

	for (j = m_newMinY; j <= m_newMaxY; ++j)
		for (i = m_newMinX; i <= m_newMaxX; ++i)
		{
			// Ref on the new value
			LigoData &data = newZones[(i - m_newMinX) + (j - m_newMinY) * newStride];

			// In the old array ?
			if ((i >= region.getMinX()) && (i <= region.getMaxX()) &&
					(j >= region.getMinY()) && (j <= region.getMaxY()))
			{
				// Backup values
				m_zoneBuilder->ligoData(data, ZonePosition(i, j, m_index));
			}
		}
	region.resize(m_newMinX, m_newMaxX, m_newMinY, m_newMaxY);

	for (j = m_newMinY; j <= m_newMaxY; ++j)
		for (i = m_newMinX; i <= m_newMaxX; ++i)
		{
			// Ref on the new value
			const LigoData &data = newZones[(i - m_newMinX) + (j - m_newMinY) * newStride];

			region.setName(i, j, data.zoneName);
			region.setPosX(i, j, data.posX);
			region.setPosY(i, j, data.posY);
			region.setRot(i, j, data.rot);
			region.setFlip(i, j, data.flip);
			uint k;
			for (k = 0; k < 4; k++)
			{
				region.setSharingMatNames(i, j, k, data.sharingMatNames[k]);
				region.setSharingCutEdges(i, j, k, data.sharingCutEdges[k]);
			}
		}
}

} /* namespace LandscapeEditor */
