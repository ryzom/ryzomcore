// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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
#include "builder_zone_region.h"
#include "builder_zone.h"
#include "zone_region_editor.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes

namespace LandscapeEditor
{

BuilderZoneRegion::BuilderZoneRegion(uint regionId)
	: m_regionId(regionId),
	  m_zoneBuilder(0),
	  m_firstInit(false)
{
}

bool BuilderZoneRegion::init(ZoneBuilder *zoneBuilder)
{
	if (m_firstInit)
		return true;

	m_zoneBuilder = zoneBuilder;

	uint32 j, k;
	SMatNode mn;
	std::vector<std::string> AllValues;

	// Build the material tree
	m_zoneBuilder->getZoneBank().getCategoryValues("material", AllValues);
	for (uint32 i = 0; i < AllValues.size(); ++i)
	{
		mn.Name = AllValues[i];
		m_matTree.push_back(mn);
	}

	// Link between materials
	AllValues.clear ();
	m_zoneBuilder->getZoneBank().getCategoryValues("transname", AllValues);
	for (uint32 i = 0; i < AllValues.size(); ++i)
	{
		// Get the 2 materials linked together
		std::string matAstr, matBstr;
		for (j = 0; j < AllValues[i].size(); ++j)
		{
			if (AllValues[i][j] == '-')
				break;
			else
				matAstr += AllValues[i][j];
		}
		++j;
		for (; j < AllValues[i].size(); ++j)
			matBstr += AllValues[i][j];

		// Find matA
		for (j = 0; j < m_matTree.size(); ++j)
			if (m_matTree[j].Name == matAstr)
				break;

		if (j < m_matTree.size())
		{
			// Find matB
			for (k = 0; k < m_matTree.size(); ++k)
				if (m_matTree[k].Name == matBstr)
					break;

			if (k < m_matTree.size())
			{
				// Add a ref to matB in node matA
				m_matTree[j].Arcs.push_back(k);

				// Add a ref to matA in node matB
				m_matTree[k].Arcs.push_back(j);
			}
		}
	}

	m_firstInit = true;
	return true;
}

class ToUpdate
{
	struct SElement
	{
		sint32 x, y;

		// Material put into the cell to update
		std::string matPut;

		BuilderZoneRegion *builderZoneRegion;
	};

	std::vector<SElement> m_elements;

public:

	void add(BuilderZoneRegion *builderZoneRegion, sint32 x, sint32 y, const std::string &matName)
	{
		bool bFound = false;
		for (uint32 m = 0; m < m_elements.size(); ++m)
			if ((m_elements[m].x == x) && (m_elements[m].y == y))
			{
				bFound = true;
				break;
			}
		if (!bFound)
		{
			SElement newElement;
			newElement.x = x;
			newElement.y = y;
			newElement.matPut = matName;
			newElement.builderZoneRegion = builderZoneRegion;
			m_elements.push_back (newElement);
		}
	}

	void del(sint32 x, sint32 y)
	{
		bool bFound = false;
		uint32 m;
		for (m = 0; m < m_elements.size(); ++m)
			if ((m_elements[m].x == x) && (m_elements[m].y == y))
			{
				bFound = true;
				break;
			}
		if (bFound)
		{
			for (; m < m_elements.size() - 1; ++m)
				m_elements[m] = m_elements[m + 1];
			m_elements.resize (m_elements.size() - 1);
		}
	}

	uint32 size()
	{
		return m_elements.size();
	}

	sint32 getX(uint32 m)
	{
		return m_elements[m].x;
	}

	sint32 getY(uint32 m)
	{
		return m_elements[m].y;
	}

	BuilderZoneRegion *getBuilderZoneRegion(uint32 m)
	{
		return m_elements[m].builderZoneRegion;
	}

	const std::string &getMat (uint32 m)
	{
		return m_elements[m].matPut;
	}
};

void BuilderZoneRegion::add(sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement)
{
	sint32 sizeX = zoneBankElement->getSizeX(), sizeY = zoneBankElement->getSizeY();
	sint32 i, j;
	NLLIGO::SPiece sMask, sPosX, sPosY;
	ToUpdate tUpdate; // Transition to update

	if (!m_zoneBuilder->getZoneMask (x,y))
		return;

	if (zoneBankElement->getCategory("transname") != STRING_NO_CAT_TYPE)
	{
		addTransition (x, y, rot, flip, zoneBankElement);
		return;
	}

	// Create the mask in the good rotation and flip
	sMask.Tab.resize(sizeX * sizeY);
	sPosX.Tab.resize(sizeX * sizeY);
	sPosY.Tab.resize(sizeX * sizeY);

	for(j = 0; j < sizeY; ++j)
		for(i = 0; i < sizeX; ++i)
		{
			sPosX.Tab[i + j * sizeX] = (uint8)i;
			sPosY.Tab[i + j * sizeX] = (uint8)j;
			sMask.Tab[i + j * sizeX] = zoneBankElement->getMask()[i + j * sizeX];
		}
	sPosX.w = sPosY.w = sMask.w = sizeX;
	sPosX.h = sPosY.h = sMask.h = sizeY;
	sMask.rotFlip(rot, flip);
	sPosX.rotFlip(rot, flip);
	sPosY.rotFlip(rot, flip);

	// Test if the pieces can be put (due to mask)
	for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i + j * sMask.w])
			{
				if (m_zoneBuilder->getZoneMask(x + i, y + j) == false)
					return;
			}

	// Delete all pieces that are under the mask
	for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i + j * sMask.w])
			{
				del(x + i, y + j, true, &tUpdate);
			}

	// Delete all around all material that are not from the same as us
	const std::string &curMat = zoneBankElement->getCategory("material");

	if (curMat != STRING_NO_CAT_TYPE)
	{
		// This element is a valid material
		// Place the piece
		const std::string &eltName = zoneBankElement->getName();
		placePiece(x, y, rot, flip, sMask, sPosX, sPosY, eltName);

		// Put all transitions between different materials
		putTransitions (x, y, sMask, curMat, &tUpdate);
		placePiece(x, y, rot, flip, sMask, sPosX, sPosY, eltName);
	}
}

void BuilderZoneRegion::invertCutEdge(sint32 x, sint32 y, uint8 cePos)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);

	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	if ((x < zoneRegion.getMinX ()) || (x > zoneRegion.getMaxX ()) ||
			(y < zoneRegion.getMinY ()) || (y > zoneRegion.getMaxY ()))
		return;

	NLLIGO::CZoneBankElement *zoneBankElement = m_zoneBuilder->getZoneBank().getElementByZoneName(zoneRegion.getName(x, y));
	if (zoneBankElement == NULL)
		return;
	if (zoneBankElement->getCategory("transname") == STRING_NO_CAT_TYPE)
		return;


	ZonePosition zonePos(x, y, m_regionId);
	LigoData dataZone;

	m_zoneBuilder->ligoData(dataZone, zonePos);
	if (dataZone.sharingCutEdges[cePos] != 3 - dataZone.sharingCutEdges[cePos])
	{
		dataZone.sharingCutEdges[cePos] = 3 - dataZone.sharingCutEdges[cePos];

		m_zoneBuilder->actionLigoTile(dataZone, zonePos);
	}
	updateTrans(x, y);

	// If the transition number is not the same propagate the change
	// Propagate where the edge is cut (1/3 or 2/3) and update the transition
	if (cePos == 2)
		if (dataZone.sharingMatNames[0] != dataZone.sharingMatNames[2])
		{
			if (x > zoneRegion.getMinX ())
			{
				// [x-1][y].right = [x][y].left
				// _Zones[(x-1-zoneRegion->getMinX ())+(y-zoneRegion->getMinY ())*stride].SharingCutEdges[3] = dataZonePos.SharingCutEdges[2];
				ZonePosition zonePosTemp(x - 1, y, m_regionId);
				LigoData dataZoneTemp;
				m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
				if (dataZoneTemp.sharingCutEdges[3] != dataZone.sharingCutEdges[2])
				{
					dataZoneTemp.sharingCutEdges[3] = dataZone.sharingCutEdges[2];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
				}
			}
			updateTrans (x - 1, y);
		}
	if (cePos == 3)
		if (dataZone.sharingMatNames[1] != dataZone.sharingMatNames[3])
		{
			if (x < zoneRegion.getMaxX ())
			{
				// [x+1][y].left = [x][y].right
				ZonePosition zonePosTemp(x + 1, y, m_regionId);
				LigoData dataZoneTemp;
				m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
				if (dataZoneTemp.sharingCutEdges[2] != dataZone.sharingCutEdges[3])
				{
					dataZoneTemp.sharingCutEdges[2] = dataZone.sharingCutEdges[3];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
				}
			}
			updateTrans (x + 1, y);
		}
	if (cePos == 1)
		if (dataZone.sharingMatNames[0] != dataZone.sharingMatNames[1])
		{
			if (y > zoneRegion.getMinY ())
			{
				// [x][y-1].up = [x][y].down
				ZonePosition zonePosTemp(x, y - 1, m_regionId);
				LigoData dataZoneTemp;
				m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
				if (dataZoneTemp.sharingCutEdges[0] != dataZone.sharingCutEdges[1])
				{
					dataZoneTemp.sharingCutEdges[0] = dataZone.sharingCutEdges[1];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
				}
			}
			updateTrans (x, y-1);
		}
	if (cePos == 0)
		if (dataZone.sharingMatNames[2] != dataZone.sharingMatNames[3])
		{
			if (y < zoneRegion.getMaxY ())
			{
				// [x][y+1].down = [x][y].up
				ZonePosition zonePosTemp(x, y + 1, m_regionId);
				LigoData dataZoneTemp;
				m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
				if (dataZoneTemp.sharingCutEdges[1] != dataZone.sharingCutEdges[0])
				{
					dataZoneTemp.sharingCutEdges[1] = dataZone.sharingCutEdges[0];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
				}
			}
			updateTrans (x, y + 1);
		}
}

void BuilderZoneRegion::cycleTransition(sint32 x, sint32 y)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	if ((x < zoneRegion.getMinX ()) || (x > zoneRegion.getMaxX ()) ||
			(y < zoneRegion.getMinY ()) || (y > zoneRegion.getMaxY ()))
		return;

	NLLIGO::CZoneBankElement *zoneBankElement = m_zoneBuilder->getZoneBank().getElementByZoneName(zoneRegion.getName (x, y));
	if (zoneBankElement == NULL)
		return;
	if (zoneBankElement->getCategory("transname") == STRING_NO_CAT_TYPE)
		return;

	// \todo trap -> choose the good transition in function of the transition under the current location
	// Choose the next possible transition if not the same as the first one
	// Choose among all transition of the same number

	updateTrans (x, y);
}

bool BuilderZoneRegion::addNotPropagate (sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return false;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	sint32 sizeX = zoneBankElement->getSizeX(), sizeY = zoneBankElement->getSizeY();
	sint32 i, j;
	NLLIGO::SPiece sMask, sPosX, sPosY;
	ToUpdate tUpdate; // Transition to update

	if (!m_zoneBuilder->getZoneMask (x, y))
		return false;

	if (zoneBankElement->getCategory("transname") != STRING_NO_CAT_TYPE)
	{
		addTransition (x, y, rot, flip, zoneBankElement);
		return true;
	}

	// Create the mask in the good rotation and flip
	sMask.Tab.resize (sizeX * sizeY);
	sPosX.Tab.resize (sizeX * sizeY);
	sPosY.Tab.resize (sizeX * sizeY);

	for (j = 0; j < sizeY; ++j)
		for (i = 0; i < sizeX; ++i)
		{
			sPosX.Tab[i + j * sizeX] = (uint8)i;
			sPosY.Tab[i + j * sizeX] = (uint8)j;
			sMask.Tab[i + j * sizeX] = zoneBankElement->getMask()[i + j * sizeX];
		}
	sPosX.w = sPosY.w = sMask.w = sizeX;
	sPosX.h = sPosY.h = sMask.h = sizeY;
	sMask.rotFlip (rot, flip);
	sPosX.rotFlip (rot, flip);
	sPosY.rotFlip (rot, flip);

	// Test if the pieces can be put (due to mask)
	sint32 stride = (1 + zoneRegion.getMaxX () - zoneRegion.getMinX ());
	for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i+j*sMask.w])
			{
				if (m_zoneBuilder->getZoneMask(x + i, y + j) == false)
					return false;
				if (((x + i) < zoneRegion.getMinX ()) || ((x + i) > zoneRegion.getMaxX ()) ||
						((y + j) < zoneRegion.getMinY ()) || ((y + j) > zoneRegion.getMaxY ()))
					return false;
				NLLIGO::CZoneBankElement *zoneBankElementUnder = m_zoneBuilder->getZoneBank().getElementByZoneName(zoneRegion.getName (x + i, y + j));
				if (zoneBankElementUnder == NULL)
					return false;
				if (zoneBankElementUnder->getCategory("material") != zoneBankElement->getCategory("material"))
					return false;
			}

	// Delete all pieces that are under the mask
	for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i + j * sMask.w])
			{
				del(x + i, y + j, true, &tUpdate);
			}

	const std::string &curMat = zoneBankElement->getCategory("material");

	if (curMat != STRING_NO_CAT_TYPE)
	{
		// This element is a valid material
		// Place the piece
		const std::string &eltName = zoneBankElement->getName();
		placePiece(x, y, rot, flip, sMask, sPosX, sPosY, eltName);
	}

	return true;
}

void BuilderZoneRegion::addForce (sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	sint32 sizeX = zoneBankElement->getSizeX(), sizeY = zoneBankElement->getSizeY();
	sint32 i, j;
	NLLIGO::SPiece sMask, sPosX, sPosY;
	ToUpdate tUpdate; // Transition to update

	if (!m_zoneBuilder->getZoneMask (x, y))
		return;

	/*
	if (pElt->getCategory("transname") != STRING_NO_CAT_TYPE)
	{
		addTransition (x, y, nRot, nFlip, pElt);
		return;
	}*/

	// Create the mask in the good rotation and flip
	sMask.Tab.resize (sizeX * sizeY);
	sPosX.Tab.resize (sizeX * sizeY);
	sPosY.Tab.resize (sizeX * sizeY);

	for (j = 0; j < sizeY; ++j)
		for (i = 0; i < sizeX; ++i)
		{
			sPosX.Tab[i + j * sizeX] = (uint8)i;
			sPosY.Tab[i + j * sizeX] = (uint8)j;
			sMask.Tab[i + j * sizeX] = zoneBankElement->getMask()[i + j * sizeX];
		}
	sPosX.w = sPosY.w = sMask.w = sizeX;
	sPosX.h = sPosY.h = sMask.h = sizeY;
	sMask.rotFlip (rot, flip);
	sPosX.rotFlip (rot, flip);
	sPosY.rotFlip (rot, flip);

	// Test if the pieces can be put (due to mask)
	// All space under the mask must be empty
	sint32 stride = (1 + zoneRegion.getMaxX () - zoneRegion.getMinX ());
	for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i+j*sMask.w])
			{
				if (m_zoneBuilder->getZoneMask(x+i, y+j) == false)
					return;
				if (((x+i) < zoneRegion.getMinX ()) || ((x+i) > zoneRegion.getMaxX ()) ||
						((y+j) < zoneRegion.getMinY ()) || ((y+j) > zoneRegion.getMaxY ()))
					return;
				NLLIGO::CZoneBankElement *zoneBankElementUnder = m_zoneBuilder->getZoneBank().getElementByZoneName(zoneRegion.getName (x + i, y + i));
				if (zoneBankElementUnder != NULL)
					return;
			}

	// Delete all pieces that are under the mask
	for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i+j*sMask.w])
			{
				del(x+i, y+j, true, &tUpdate);
			}

	const std::string &curMat = zoneBankElement->getCategory ("material");
	const bool transition = zoneBankElement->getCategory("transname") != STRING_NO_CAT_TYPE;

	if (curMat != STRING_NO_CAT_TYPE || transition)
	{
		// This element is a valid material
		// Place the piece
		const std::string &eltName = zoneBankElement->getName();
		for (j = 0; j < sMask.h; ++j)
			for (i = 0; i < sMask.w; ++i)
				if (sMask.Tab[i + j * sMask.w])
				{
					set(x + i, y + j, sPosX.Tab[i + j * sPosX.w], sPosY.Tab[i + j * sPosY.w], eltName, transition);
					setRot(x + i, y + j, rot);
					setFlip(x + i, y + j, flip);
				}
	}
}

uint8 TransToEdge[72][4] =
{
	{ 0, 0, 1, 1 }, // TransNum = 0, Flip = 0, Rot = 0
	{ 2, 2, 0, 0 }, // TransNum = 0, Flip = 0, Rot = 1
	{ 0, 0, 2, 2 }, // TransNum = 0, Flip = 0, Rot = 2
	{ 1, 1, 0, 0 }, // TransNum = 0, Flip = 0, Rot = 3
	{ 0, 0, 1, 1 }, // TransNum = 0, Flip = 1, Rot = 0
	{ 2, 2, 0, 0 }, // TransNum = 0, Flip = 1, Rot = 1
	{ 0, 0, 2, 2 }, // TransNum = 0, Flip = 1, Rot = 2
	{ 1, 1, 0, 0 }, // TransNum = 0, Flip = 1, Rot = 3

	{ 0, 0, 1, 2 }, // TransNum = 1, Flip = 0, Rot = 0
	{ 1, 2, 0, 0 }, // TransNum = 1, Flip = 0, Rot = 1
	{ 0, 0, 1, 2 }, // TransNum = 1, Flip = 0, Rot = 2
	{ 1, 2, 0, 0 }, // TransNum = 1, Flip = 0, Rot = 3
	{ 0, 0, 2, 1 }, // TransNum = 1, Flip = 1, Rot = 0
	{ 2, 1, 0, 0 }, // TransNum = 1, Flip = 1, Rot = 1
	{ 0, 0, 2, 1 }, // TransNum = 1, Flip = 1, Rot = 2
	{ 2, 1, 0, 0 }, // TransNum = 1, Flip = 1, Rot = 3

	{ 0, 0, 2, 2 }, // TransNum = 2, Flip = 0, Rot = 0
	{ 1, 1, 0, 0 }, // TransNum = 2, Flip = 0, Rot = 1
	{ 0, 0, 1, 1 }, // TransNum = 2, Flip = 0, Rot = 2
	{ 2, 2, 0, 0 }, // TransNum = 2, Flip = 0, Rot = 3
	{ 0, 0, 2, 2 }, // TransNum = 2, Flip = 1, Rot = 0
	{ 1, 1, 0, 0 }, // TransNum = 2, Flip = 1, Rot = 1
	{ 0, 0, 1, 1 }, // TransNum = 2, Flip = 1, Rot = 2
	{ 2, 2, 0, 0 }, // TransNum = 2, Flip = 1, Rot = 3

	{ 0, 1, 1, 0 }, // TransNum = 3, Flip = 0, Rot = 0
	{ 0, 2, 0, 1 }, // TransNum = 3, Flip = 0, Rot = 1
	{ 2, 0, 0, 2 }, // TransNum = 3, Flip = 0, Rot = 2
	{ 1, 0, 2, 0 }, // TransNum = 3, Flip = 0, Rot = 3
	{ 0, 2, 0, 1 }, // TransNum = 3, Flip = 1, Rot = 0
	{ 2, 0, 0, 2 }, // TransNum = 3, Flip = 1, Rot = 1
	{ 1, 0, 2, 0 }, // TransNum = 3, Flip = 1, Rot = 2
	{ 0, 1, 1, 0 }, // TransNum = 3, Flip = 1, Rot = 3

	{ 0, 2, 1, 0 }, // TransNum = 4, Flip = 0, Rot = 0
	{ 0, 2, 0, 2 }, // TransNum = 4, Flip = 0, Rot = 1
	{ 1, 0, 0, 2 }, // TransNum = 4, Flip = 0, Rot = 2
	{ 1, 0, 1, 0 }, // TransNum = 4, Flip = 0, Rot = 3
	{ 0, 1, 0, 1 }, // TransNum = 4, Flip = 1, Rot = 0
	{ 2, 0, 0, 1 }, // TransNum = 4, Flip = 1, Rot = 1
	{ 2, 0, 2, 0 }, // TransNum = 4, Flip = 1, Rot = 2
	{ 0, 1, 2, 0 }, // TransNum = 4, Flip = 1, Rot = 3

	{ 0, 2, 2, 0 }, // TransNum = 5, Flip = 0, Rot = 0
	{ 0, 1, 0, 2 }, // TransNum = 5, Flip = 0, Rot = 1
	{ 1, 0, 0, 1 }, // TransNum = 5, Flip = 0, Rot = 2
	{ 2, 0, 1, 0 }, // TransNum = 5, Flip = 0, Rot = 3
	{ 0, 1, 0, 2 }, // TransNum = 5, Flip = 1, Rot = 0
	{ 1, 0, 0, 1 }, // TransNum = 5, Flip = 1, Rot = 1
	{ 2, 0, 1, 0 }, // TransNum = 5, Flip = 1, Rot = 2
	{ 0, 2, 2, 0 }, // TransNum = 5, Flip = 1, Rot = 3

	{ 0, 1, 1, 0 }, // TransNum = 6, Flip = 0, Rot = 0
	{ 0, 2, 0, 1 }, // TransNum = 6, Flip = 0, Rot = 1
	{ 2, 0, 0, 2 }, // TransNum = 6, Flip = 0, Rot = 2
	{ 1, 0, 2, 0 }, // TransNum = 6, Flip = 0, Rot = 3
	{ 0, 2, 0, 1 }, // TransNum = 6, Flip = 1, Rot = 0
	{ 2, 0, 0, 2 }, // TransNum = 6, Flip = 1, Rot = 1
	{ 1, 0, 2, 0 }, // TransNum = 6, Flip = 1, Rot = 2
	{ 0, 1, 1, 0 }, // TransNum = 6, Flip = 1, Rot = 3

	{ 0, 2, 1, 0 }, // TransNum = 7, Flip = 0, Rot = 0
	{ 0, 2, 0, 2 }, // TransNum = 7, Flip = 0, Rot = 1
	{ 1, 0, 0, 2 }, // TransNum = 7, Flip = 0, Rot = 2
	{ 1, 0, 1, 0 }, // TransNum = 7, Flip = 0, Rot = 3
	{ 0, 1, 0, 1 }, // TransNum = 7, Flip = 1, Rot = 0
	{ 2, 0, 0, 1 }, // TransNum = 7, Flip = 1, Rot = 1
	{ 2, 0, 2, 0 }, // TransNum = 7, Flip = 1, Rot = 2
	{ 0, 1, 2, 0 }, // TransNum = 7, Flip = 1, Rot = 3

	{ 0, 2, 2, 0 }, // TransNum = 8, Flip = 0, Rot = 0
	{ 0, 1, 0, 2 }, // TransNum = 8, Flip = 0, Rot = 1
	{ 1, 0, 0, 1 }, // TransNum = 8, Flip = 0, Rot = 2
	{ 2, 0, 1, 0 }, // TransNum = 8, Flip = 0, Rot = 3
	{ 0, 1, 0, 2 }, // TransNum = 8, Flip = 1, Rot = 0
	{ 1, 0, 0, 1 }, // TransNum = 8, Flip = 1, Rot = 1
	{ 2, 0, 1, 0 }, // TransNum = 8, Flip = 1, Rot = 2
	{ 0, 2, 2, 0 }, // TransNum = 8, Flip = 1, Rot = 3
};

void BuilderZoneRegion::addTransition (sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	uint32 i;
	// Check that we write in an already defined place
	if ((x < zoneRegion.getMinX ()) || (x > zoneRegion.getMaxX ()) ||
			(y < zoneRegion.getMinY ()) || (y > zoneRegion.getMaxY ()))
		return;

	// Check size
	if ((zoneBankElement->getSizeX() != 1) || (zoneBankElement->getSizeY() != 1))
		return;

	// Check that an element already exist at position we want put the transition
	NLLIGO::CZoneBankElement *zoneBankElementUnder = m_zoneBuilder->getZoneBank().getElementByZoneName(zoneRegion.getName(x, y));
	if (zoneBankElementUnder == NULL)
		return;

	// And check that this element is also a transition and the same transition
	if (zoneBankElementUnder->getCategory ("transname") == STRING_NO_CAT_TYPE)
		return;
	if (zoneBankElementUnder->getCategory ("transname") != zoneBankElement->getCategory("transname"))
		return;

	std::string underType = zoneBankElementUnder->getCategory("transtype");
	std::string overType = zoneBankElement->getCategory("transtype");
	std::string underNum = zoneBankElementUnder->getCategory("transnum");
	std::string overNum = zoneBankElement ->getCategory("transnum");

	ZonePosition zonePosTemp(x, y, m_regionId);
	LigoData dataZoneTemp;
	m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
	LigoData dataZoneOriginal = dataZoneTemp;

	bool bMustPropagate = false;
	// Same type of transition ?
	if (zoneBankElementUnder->getCategory("transtype") != zoneBankElement->getCategory("transtype"))
	{
		// No so random the cutEdges
		for (i = 0; i < 4; ++i)
		{
			uint8 nCut = (uint8)(1.0f + NLMISC::frand(2.0f));
			NLMISC::clamp(nCut, (uint8)1, (uint8)2);

			dataZoneTemp.sharingCutEdges[i] = nCut;
		}
		zoneBankElement = NULL;
		bMustPropagate = true;
	}
	else
	{
		// Put exactly the transition as given
		sint32 transnum = atoi(zoneBankElement->getCategory("transnum").c_str());
		sint32 flip = zoneRegion.getFlip(x, y);
		sint32 rot = zoneRegion.getRot(x, y);
		sint32 pos1 = -1, pos2 = -1;

		for (i = 0; i < 4; ++i)
		{
			if ((TransToEdge[transnum * 8 + flip * 4 + rot][i] != 0) &&
					(TransToEdge[transnum * 8 + flip * 4 + rot][i] != dataZoneTemp.sharingCutEdges[i]))
				bMustPropagate = true;

			dataZoneTemp.sharingCutEdges[i] = TransToEdge[transnum * 8 + flip * 4 + rot][i];

			if ((pos1 != -1) && (dataZoneTemp.sharingCutEdges[i] != 0))
				pos2 = i;
			if ((pos1 == -1) && (dataZoneTemp.sharingCutEdges[i] != 0))
				pos1 = i;
		}
		// Exchange cutedges != 0 one time /2 to permit all positions
		if ((transnum == 1) || (transnum == 4) || (transnum == 7))
			if (zoneBankElement->getName() == zoneBankElementUnder->getName())
			{
				bMustPropagate = true;

				dataZoneTemp.sharingCutEdges[pos1] = 3 - dataZoneTemp.sharingCutEdges[pos1];
				dataZoneTemp.sharingCutEdges[pos2] = 3 - dataZoneTemp.sharingCutEdges[pos2];
			}
	}
	if (dataZoneTemp != dataZoneOriginal)
	{
		// Add modification landscape
		m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
	}
	updateTrans (x, y, zoneBankElement);

	// If the transition number is not the same propagate the change
	if (bMustPropagate)
	{
		// Propagate where the edge is cut (1/3 or 2/3) and update the transition
		if (dataZoneTemp.sharingMatNames[0] != dataZoneTemp.sharingMatNames[2])
		{
			if (x > zoneRegion.getMinX ())
			{
				// [x-1][y].right = [x][y].left
				// _Zones[(x-1-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[3] = dataZoneTemp.SharingCutEdges[2];
				ZonePosition zonePosTemp2(x - 1, y, m_regionId);
				LigoData dataZoneTemp2;
				m_zoneBuilder->ligoData(dataZoneTemp2, zonePosTemp2);
				if (dataZoneTemp2.sharingCutEdges[3] != dataZoneTemp.sharingCutEdges[2])
				{
					dataZoneTemp2.sharingCutEdges[3] = dataZoneTemp.sharingCutEdges[2];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp2, zonePosTemp2);
				}
			}
			updateTrans (x - 1, y);
		}
		if (dataZoneTemp.sharingMatNames[1] != dataZoneTemp.sharingMatNames[3])
		{
			if (x < zoneRegion.getMaxX ())
			{
				// [x+1][y].left = [x][y].right
				//_Zones[(x+1-pBZR->getMinX ())+(y-pBZR->getMinY ())*stride].SharingCutEdges[2] = dataZoneTemp.SharingCutEdges[3];
				ZonePosition zonePosTemp2(x + 1, y, m_regionId);
				LigoData dataZoneTemp2;
				m_zoneBuilder->ligoData(dataZoneTemp2, zonePosTemp2);
				if (dataZoneTemp2.sharingCutEdges[2] != dataZoneTemp.sharingCutEdges[3])
				{
					dataZoneTemp2.sharingCutEdges[2] = dataZoneTemp.sharingCutEdges[3];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp2, zonePosTemp2);
				}
			}
			updateTrans (x + 1, y);
		}
		if (dataZoneTemp.sharingMatNames[0] != dataZoneTemp.sharingMatNames[1])
		{
			if (y > zoneRegion.getMinY ())
			{
				// [x][y-1].up = [x][y].down
				//_Zones[(x-pBZR->getMinX ())+(y-1-pBZR->getMinY ())*stride].SharingCutEdges[0] = dataZoneTemp.SharingCutEdges[1];
				ZonePosition zonePosTemp2(x, y - 1, m_regionId);
				LigoData dataZoneTemp2;
				m_zoneBuilder->ligoData(dataZoneTemp2, zonePosTemp2);
				if (dataZoneTemp2.sharingCutEdges[0] != dataZoneTemp.sharingCutEdges[1])
				{
					dataZoneTemp2.sharingCutEdges[0] = dataZoneTemp.sharingCutEdges[1];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp2, zonePosTemp2);
				}
			}
			updateTrans (x, y - 1);
		}
		if (dataZoneTemp.sharingMatNames[2] != dataZoneTemp.sharingMatNames[3])
		{
			if (y < zoneRegion.getMaxY ())
			{
				// [x][y+1].down = [x][y].up
				//_Zones[(x-pBZR->getMinX ())+(y+1-pBZR->getMinY ())*stride].SharingCutEdges[1] = dataZoneTemp.SharingCutEdges[0];
				ZonePosition zonePosTemp2(x, y + 1, m_regionId);
				LigoData dataZoneTemp2;
				m_zoneBuilder->ligoData(dataZoneTemp2, zonePosTemp2);
				if (dataZoneTemp2.sharingCutEdges[1] != dataZoneTemp.sharingCutEdges[0])
				{
					dataZoneTemp2.sharingCutEdges[1] = dataZoneTemp.sharingCutEdges[0];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp2, zonePosTemp2);
				}
			}
			updateTrans (x, y + 1);
		}
	}
}

void BuilderZoneRegion::addToUpdateAndCreate(BuilderZoneRegion *builderZoneRegion, sint32 sharePos, sint32 x, sint32 y,
		const std::string &newMat, ToUpdate *ptCreate, ToUpdate *ptUpdate)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	sint32 stride = (1 + zoneRegion.getMaxX() - zoneRegion.getMinX());

	ZonePosition zonePos;
	if (m_zoneBuilder->getZoneAmongRegions(zonePos, builderZoneRegion, x, y))
	{
		LigoData data;
		m_zoneBuilder->ligoData(data, zonePos);
		if (data.sharingMatNames[sharePos] != newMat)
		{
			data.sharingMatNames[sharePos] = newMat;

			// Add modification landscape
			m_zoneBuilder->actionLigoTile(data, zonePos);
		}
		builderZoneRegion->del(x, y, true, ptUpdate);
		ptCreate->add(builderZoneRegion, x, y, newMat);
	}
}

void BuilderZoneRegion::putTransitions (sint32 inX, sint32 inY, const NLLIGO::SPiece &mask, const std::string &matName,
										ToUpdate *ptUpdate)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	ToUpdate tCreate; // Transition to create

	sint32 i, j, k, l, m;
	sint32 x = inX, y = inY;
	for (j = 0; j < mask.h; ++j)
		for (i = 0; i < mask.w; ++i)
			if (mask.Tab[i + j * mask.w])
			{
				for (k = -1; k <= 1; ++k)
					for (l = -1; l <= 1; ++l)
					{
						BuilderZoneRegion *builderZoneRegion2 = this;
						ZonePosition zonePos;
						if (m_zoneBuilder->getZoneAmongRegions(zonePos, builderZoneRegion2, inX + i + l, inY + j + k))
							tCreate.add(builderZoneRegion2, inX + i + l, inY + j + k, matName);
					}
			}

	// Check coherency of the transition to update
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		BuilderZoneRegion *builderZoneRegion2 = tCreate.getBuilderZoneRegion(m);
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		std::string putMat = tCreate.getMat(m);

		//if ((x < pBZR->getMinX ())||(x > pBZR->getMaxX ())||(y < pBZR->getMinY ())||(y > pBZR->getMaxY ()))
		//	continue;

		ZonePosition zoneTemp(x, y, builderZoneRegion2->getRegionId());
		LigoData dataZoneTemp;
		m_zoneBuilder->ligoData(dataZoneTemp, zoneTemp);

		if (!((dataZoneTemp.sharingMatNames[0] == dataZoneTemp.sharingMatNames[1])&&
				(dataZoneTemp.sharingMatNames[1] == dataZoneTemp.sharingMatNames[2])&&
				(dataZoneTemp.sharingMatNames[2] == dataZoneTemp.sharingMatNames[3])))
			builderZoneRegion2->del(x, y, true, ptUpdate);

		// Check to see material can be posed
		uint corner;
		for (corner = 0; corner < 4; corner++)
		{
			std::string newMat = getNextMatInTree (putMat, dataZoneTemp.sharingMatNames[corner]);

			// Can't be posed ?
			if (newMat == STRING_UNUSED)
				break;
		}
		if ( (corner < 4) && (m != 0) )
		{
			// The material can't be paused
			for (int t = 0; t < 4; ++t)
				dataZoneTemp.sharingMatNames[t] = STRING_UNUSED;

			// Don't propagate any more
		}
		else
		{
			// Expand material for the 1st quarter
			std::string newMat = getNextMatInTree(putMat, dataZoneTemp.sharingMatNames[0]);
			if (newMat != dataZoneTemp.sharingMatNames[0])
			{
				// Update the quarter
				if (dataZoneTemp.sharingMatNames[0] != newMat)
				{
					dataZoneTemp.sharingMatNames[0] = newMat;

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
				}

				addToUpdateAndCreate(builderZoneRegion2, 1, x - 1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 3, x - 1, y - 1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 2, x, y - 1, newMat, &tCreate, ptUpdate);
			}

			// Expand material for the 2nd quarter
			newMat = getNextMatInTree(putMat, dataZoneTemp.sharingMatNames[1]);
			if (newMat != dataZoneTemp.sharingMatNames[1])
			{
				// Update the quarter
				//if (_Builder->getZoneMask(x,y))
				if (dataZoneTemp.sharingMatNames[1] != newMat)
				{
					dataZoneTemp.sharingMatNames[1] = newMat;

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
				}

				addToUpdateAndCreate(builderZoneRegion2, 0, x + 1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 2, x + 1, y - 1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 3, x, y - 1, newMat, &tCreate, ptUpdate);
			}

			// Expand material for the 3rd quarter
			newMat = getNextMatInTree(putMat, dataZoneTemp.sharingMatNames[2]);
			if (newMat != dataZoneTemp.sharingMatNames[2])
			{
				// Update the quarter
				//if (_Builder->getZoneMask(x,y))
				if (dataZoneTemp.sharingMatNames[2] != newMat)
				{
					dataZoneTemp.sharingMatNames[2] = newMat;

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
				}

				addToUpdateAndCreate(builderZoneRegion2, 3, x - 1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 1, x - 1, y + 1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 0, x, y + 1, newMat, &tCreate, ptUpdate);
			}

			// Expand material for the 4th quarter
			newMat = getNextMatInTree(putMat, dataZoneTemp.sharingMatNames[3]);
			if (newMat != dataZoneTemp.sharingMatNames[3])
			{
				// Update the quarter
				//if (_Builder->getZoneMask(x,y))
				if (dataZoneTemp.sharingMatNames[3] != newMat)
				{
					dataZoneTemp.sharingMatNames[3] = newMat;

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
				}

				addToUpdateAndCreate(builderZoneRegion2, 2, x + 1, y, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 0, x + 1, y + 1, newMat, &tCreate, ptUpdate);
				addToUpdateAndCreate(builderZoneRegion2, 1, x, y + 1, newMat, &tCreate, ptUpdate);
			}
		}

		// Add modification landscape
		m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
	}

	// Delete transitions that are inside the mask
	for (j = 0; j < mask.h; ++j)
		for (i = 0; i < mask.w; ++i)
			if (mask.Tab[i + j * mask.w])
			{
				tCreate.del(inX + i, inY + j);
			}

	// For all transition to update choose the cut edge
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		ZoneRegionObject *zoneRegionObj2 = m_zoneBuilder->zoneRegion(tCreate.getBuilderZoneRegion(m)->getRegionId());
		if (zoneRegionObj2 == 0)
			continue;

		const NLLIGO::CZoneRegion &zoneRegion2 = zoneRegionObj2->ligoZoneRegion();
		x = tCreate.getX(m);
		y = tCreate.getY(m);

		if ((x < zoneRegion.getMinX()) || (x > zoneRegion.getMaxX()) ||
				(y < zoneRegion.getMinY()) || (y > zoneRegion.getMaxY()))
			continue;

		ZonePosition zoneTemp(x, y, tCreate.getBuilderZoneRegion(m)->getRegionId());
		LigoData dataZoneTemp;
		m_zoneBuilder->ligoData(dataZoneTemp, zoneTemp);
		LigoData dataZoneTempOriginal = dataZoneTemp;

		for (i = 0; i < 4; ++i)
		{
			uint8 nCut = (uint8)(1.0f + NLMISC::frand(2.0f));
			NLMISC::clamp(nCut, (uint8)1, (uint8)2);
			dataZoneTemp.sharingCutEdges[i] = nCut;
		}

		// Add modification landscape
		if (dataZoneTempOriginal != dataZoneTemp)
			m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);

		// Propagate
		if (dataZoneTemp.sharingMatNames[0] != dataZoneTemp.sharingMatNames[2])
		{
			// [x-1][y].right = [x][y].left
			BuilderZoneRegion *builderZoneRegion3 = tCreate.getBuilderZoneRegion(m);
			ZonePosition zonePosU;
			if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion3, x - 1, y))
			{
				LigoData data;
				m_zoneBuilder->ligoData(data, zonePosU);
				if (data.sharingCutEdges[3] != dataZoneTemp.sharingCutEdges[2])
				{
					data.sharingCutEdges[3] = dataZoneTemp.sharingCutEdges[2];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(data, zonePosU);
				}
				ptUpdate->add(builderZoneRegion3, x - 1, y, "");
			}
		}
		else
		{
			m_zoneBuilder->ligoData(dataZoneTemp, zoneTemp);
			if (dataZoneTemp.sharingCutEdges[2] != 0)
			{
				dataZoneTemp.sharingCutEdges[2] = 0;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
			}
		}

		if (dataZoneTemp.sharingMatNames[0] != dataZoneTemp.sharingMatNames[1])
		{
			// [x][y-1].up = [x][y].down
			BuilderZoneRegion *builderZoneRegion3 = tCreate.getBuilderZoneRegion(m);
			ZonePosition zonePosU;
			if (m_zoneBuilder->getZoneAmongRegions (zonePosU, builderZoneRegion3, x, y - 1))
			{
				LigoData data;
				m_zoneBuilder->ligoData(data, zonePosU);
				if (data.sharingCutEdges[0] != dataZoneTemp.sharingCutEdges[1])
				{
					data.sharingCutEdges[0] = dataZoneTemp.sharingCutEdges[1];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(data, zonePosU);
				}
				ptUpdate->add (builderZoneRegion3, x, y - 1, "");
			}
		}
		else
		{
			m_zoneBuilder->ligoData(dataZoneTemp, zoneTemp);
			if (dataZoneTemp.sharingCutEdges[1] != 0)
			{
				dataZoneTemp.sharingCutEdges[1] = 0;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
			}
		}

		if (dataZoneTemp.sharingMatNames[3] != dataZoneTemp.sharingMatNames[1])
		{
			// [x+1][y].left = [x][y].right
			BuilderZoneRegion *builderZoneRegion3 = tCreate.getBuilderZoneRegion(m);
			ZonePosition zonePosU;
			if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion3, x + 1, y))
			{
				LigoData data;
				m_zoneBuilder->ligoData(data, zonePosU);
				if (data.sharingCutEdges[2] != dataZoneTemp.sharingCutEdges[3])
				{
					data.sharingCutEdges[2] = dataZoneTemp.sharingCutEdges[3];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(data, zonePosU);
				}
				ptUpdate->add(builderZoneRegion3, x + 1, y, "");
			}
		}
		else
		{
			m_zoneBuilder->ligoData(dataZoneTemp, zoneTemp);
			if (dataZoneTemp.sharingCutEdges[3] != 0)
			{
				dataZoneTemp.sharingCutEdges[3] = 0;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
			}
		}

		if (dataZoneTemp.sharingMatNames[2] != dataZoneTemp.sharingMatNames[3])
		{
			// [x][y+1].down = [x][y].up
			BuilderZoneRegion *builderZoneRegion3 = tCreate.getBuilderZoneRegion(m);
			ZonePosition  zonePosU;
			if (m_zoneBuilder->getZoneAmongRegions (zonePosU, builderZoneRegion3, x, y+1))
			{
				LigoData data;
				m_zoneBuilder->ligoData(data, zonePosU);
				if (data.sharingCutEdges[1] = dataZoneTemp.sharingCutEdges[0])
				{
					data.sharingCutEdges[1] = dataZoneTemp.sharingCutEdges[0];

					// Add modification landscape
					m_zoneBuilder->actionLigoTile(data, zonePosU);
				}
				ptUpdate->add (builderZoneRegion3, x, y + 1, "");
			}
		}
		else
		{
			m_zoneBuilder->ligoData(dataZoneTemp, zoneTemp);
			if (dataZoneTemp.sharingCutEdges[0] = 0)
			{
				dataZoneTemp.sharingCutEdges[0] = 0;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(dataZoneTemp, zoneTemp);
			}
		}
	}

	// Delete in tUpdate each element in common with tCreate
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		ptUpdate->del (x,y);
	}

	// Finally update all transition
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		const NLLIGO::CZoneRegion &zoneRegion2 = m_zoneBuilder->zoneRegion(tCreate.getBuilderZoneRegion(m)->getRegionId())->ligoZoneRegion();
		x = tCreate.getX(m);
		y = tCreate.getY(m);

		if ((x >= zoneRegion2.getMinX()) && (x <= zoneRegion2.getMaxX()) &&
				(y >= zoneRegion2.getMinY()) && (y <= zoneRegion2.getMaxY()))
			tCreate.getBuilderZoneRegion(m)->updateTrans(x, y);
	}

	// WARNING: TODO: check this for 
	for (m = 0; m < (sint32)ptUpdate->size(); ++m)
	{
		const NLLIGO::CZoneRegion &zoneRegion2 = m_zoneBuilder->zoneRegion(ptUpdate->getBuilderZoneRegion(m)->getRegionId())->ligoZoneRegion();
		x = ptUpdate->getX(m);
		y = ptUpdate->getY(m);
		if ((x >= zoneRegion2.getMinX()) && (x <= zoneRegion2.getMaxX()) &&
				(y >= zoneRegion2.getMinY()) && (y <= zoneRegion2.getMaxY()))
			ptUpdate->getBuilderZoneRegion(m)->updateTrans(x, y);
	}

	// Cross material
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		const NLLIGO::CZoneRegion &zoneRegion2 = m_zoneBuilder->zoneRegion(tCreate.getBuilderZoneRegion(m)->getRegionId())->ligoZoneRegion();
		x = tCreate.getX(m);
		y = tCreate.getY(m);


		ZonePosition zoneTemp(x, y, m_regionId);
		LigoData dataZoneTemp;
		m_zoneBuilder->ligoData(dataZoneTemp, zoneTemp);

		std::set<std::string> matNameSet;
		for (i = 0; i < 4; ++i)
			matNameSet.insert(dataZoneTemp.sharingMatNames[i]);

		if (((dataZoneTemp.sharingMatNames[0] == dataZoneTemp.sharingMatNames[3]) &&
				(dataZoneTemp.sharingMatNames[1] == dataZoneTemp.sharingMatNames[2]) &&
				(dataZoneTemp.sharingMatNames[0] != dataZoneTemp.sharingMatNames[1]))
				|| (matNameSet.size()>2))
		{
			NLLIGO::CZoneBank &zoneBank = m_zoneBuilder->getZoneBank();
			zoneBank.resetSelection();
			zoneBank.addOrSwitch("material", tCreate.getMat(m));
			zoneBank.addAndSwitch("size", "1x1");
			std::vector<NLLIGO::CZoneBankElement *> vElts;
			zoneBank.getSelection(vElts);
			if (vElts.size() == 0)
				return;
			sint32 nRan = (sint32)(NLMISC::frand((float)vElts.size()));
			NLMISC::clamp(nRan, (sint32)0, (sint32)(vElts.size() - 1));
			NLLIGO::CZoneBankElement *zoneBankElement = vElts[nRan];
			nRan = (uint32)(NLMISC::frand (1.0) * 4);
			NLMISC::clamp(nRan, (sint32)0, (sint32)3);
			uint8 rot = (uint8)nRan;
			nRan = (uint32)(NLMISC::frand (1.0) * 2);
			NLMISC::clamp (nRan, (sint32)0, (sint32)1);
			uint8 flip = (uint8)nRan;

			tCreate.getBuilderZoneRegion(m)->add(x, y, rot, flip, zoneBankElement);
		}
	}
}

struct STrans
{
	uint8 Num;
	uint8 Rot;
	uint8 Flip;
};

STrans TranConvTable[128] =
{
	{ 0,0,0 }, // Quart = 0, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 3, Np = 1 UNUSED

	{ 6,0,0 }, // Quart = 1, CutEdge = 0, Np = 0
	{ 6,3,1 }, // Quart = 1, CutEdge = 0, Np = 1
	{ 7,0,0 }, // Quart = 1, CutEdge = 1, Np = 0
	{ 7,0,0 }, // Quart = 1, CutEdge = 1, Np = 1
	{ 7,3,1 }, // Quart = 1, CutEdge = 2, Np = 0
	{ 7,3,1 }, // Quart = 1, CutEdge = 2, Np = 1
	{ 8,0,0 }, // Quart = 1, CutEdge = 3, Np = 0
	{ 8,3,1 }, // Quart = 1, CutEdge = 3, Np = 1

	{ 7,0,1 }, // Quart = 2, CutEdge = 0, Np = 0
	{ 7,0,1 }, // Quart = 2, CutEdge = 0, Np = 1
	{ 6,0,1 }, // Quart = 2, CutEdge = 1, Np = 0
	{ 6,1,0 }, // Quart = 2, CutEdge = 1, Np = 1
	{ 8,1,0 }, // Quart = 2, CutEdge = 2, Np = 0
	{ 8,0,1 }, // Quart = 2, CutEdge = 2, Np = 1
	{ 7,1,0 }, // Quart = 2, CutEdge = 3, Np = 0
	{ 7,1,0 }, // Quart = 2, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 3, CutEdge = 0, Np = 0
	{ 0,0,1 }, // Quart = 3, CutEdge = 0, Np = 1
	{ 1,0,1 }, // Quart = 3, CutEdge = 1, Np = 0
	{ 1,0,1 }, // Quart = 3, CutEdge = 1, Np = 1
	{ 1,0,0 }, // Quart = 3, CutEdge = 2, Np = 0
	{ 1,0,0 }, // Quart = 3, CutEdge = 2, Np = 1
	{ 2,0,0 }, // Quart = 3, CutEdge = 3, Np = 0
	{ 2,0,1 }, // Quart = 3, CutEdge = 3, Np = 1

	{ 7,3,0 }, // Quart = 4, CutEdge = 0, Np = 0
	{ 7,3,0 }, // Quart = 4, CutEdge = 0, Np = 1
	{ 8,3,0 }, // Quart = 4, CutEdge = 1, Np = 0
	{ 8,2,1 }, // Quart = 4, CutEdge = 1, Np = 1
	{ 6,3,0 }, // Quart = 4, CutEdge = 2, Np = 0
	{ 6,2,1 }, // Quart = 4, CutEdge = 2, Np = 1
	{ 7,2,1 }, // Quart = 4, CutEdge = 3, Np = 0
	{ 7,2,1 }, // Quart = 4, CutEdge = 3, Np = 1

	{ 0,3,0 }, // Quart = 5, CutEdge = 0, Np = 0
	{ 0,3,1 }, // Quart = 5, CutEdge = 0, Np = 1
	{ 1,3,1 }, // Quart = 5, CutEdge = 1, Np = 0
	{ 1,3,1 }, // Quart = 5, CutEdge = 1, Np = 1
	{ 1,3,0 }, // Quart = 5, CutEdge = 2, Np = 0
	{ 1,3,0 }, // Quart = 5, CutEdge = 2, Np = 1
	{ 2,3,0 }, // Quart = 5, CutEdge = 3, Np = 0
	{ 2,3,1 }, // Quart = 5, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 6, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 3, Np = 1 UNUSED

	{ 5,2,0 }, // Quart = 7, CutEdge = 0, Np = 0
	{ 5,1,1 }, // Quart = 7, CutEdge = 0, Np = 1
	{ 4,1,1 }, // Quart = 7, CutEdge = 1, Np = 0
	{ 4,1,1 }, // Quart = 7, CutEdge = 1, Np = 1
	{ 4,2,0 }, // Quart = 7, CutEdge = 2, Np = 0
	{ 4,2,0 }, // Quart = 7, CutEdge = 2, Np = 1
	{ 3,2,0 }, // Quart = 7, CutEdge = 3, Np = 0
	{ 3,1,1 }, // Quart = 7, CutEdge = 3, Np = 1

	{ 8,2,0 }, // Quart = 8, CutEdge = 0, Np = 0
	{ 8,1,1 }, // Quart = 8, CutEdge = 0, Np = 1
	{ 7,1,1 }, // Quart = 8, CutEdge = 1, Np = 0
	{ 7,1,1 }, // Quart = 8, CutEdge = 1, Np = 1
	{ 7,2,0 }, // Quart = 8, CutEdge = 2, Np = 0
	{ 7,2,0 }, // Quart = 8, CutEdge = 2, Np = 1
	{ 6,2,0 }, // Quart = 8, CutEdge = 3, Np = 0
	{ 6,1,1 }, // Quart = 8, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 9, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 3, Np = 1 UNUSED

	{ 2,1,0 }, // Quart = 10, CutEdge = 0, Np = 0
	{ 2,1,1 }, // Quart = 10, CutEdge = 0, Np = 1
	{ 1,1,1 }, // Quart = 10, CutEdge = 1, Np = 0
	{ 1,1,1 }, // Quart = 10, CutEdge = 1, Np = 1
	{ 1,1,0 }, // Quart = 10, CutEdge = 2, Np = 0
	{ 1,1,0 }, // Quart = 10, CutEdge = 2, Np = 1
	{ 0,1,0 }, // Quart = 10, CutEdge = 3, Np = 0
	{ 0,1,1 }, // Quart = 10, CutEdge = 3, Np = 1

	{ 4,3,0 }, // Quart = 11, CutEdge = 0, Np = 0
	{ 4,3,0 }, // Quart = 11, CutEdge = 0, Np = 1
	{ 5,3,0 }, // Quart = 11, CutEdge = 1, Np = 0
	{ 5,2,1 }, // Quart = 11, CutEdge = 1, Np = 1
	{ 3,3,0 }, // Quart = 11, CutEdge = 2, Np = 0
	{ 3,2,1 }, // Quart = 11, CutEdge = 2, Np = 1
	{ 4,2,1 }, // Quart = 11, CutEdge = 3, Np = 0
	{ 4,2,1 }, // Quart = 11, CutEdge = 3, Np = 1

	{ 2,2,0 }, // Quart = 12, CutEdge = 0, Np = 0
	{ 2,2,1 }, // Quart = 12, CutEdge = 0, Np = 1
	{ 1,2,1 }, // Quart = 12, CutEdge = 1, Np = 0
	{ 1,2,1 }, // Quart = 12, CutEdge = 1, Np = 1
	{ 1,2,0 }, // Quart = 12, CutEdge = 2, Np = 0
	{ 1,2,0 }, // Quart = 12, CutEdge = 2, Np = 1
	{ 0,2,0 }, // Quart = 12, CutEdge = 3, Np = 0
	{ 0,2,1 }, // Quart = 12, CutEdge = 3, Np = 1

	{ 4,0,1 }, // Quart = 13, CutEdge = 0, Np = 0
	{ 4,0,1 }, // Quart = 13, CutEdge = 0, Np = 1
	{ 3,1,0 }, // Quart = 13, CutEdge = 1, Np = 0
	{ 3,0,1 }, // Quart = 13, CutEdge = 1, Np = 1
	{ 5,1,0 }, // Quart = 13, CutEdge = 2, Np = 0
	{ 5,0,1 }, // Quart = 13, CutEdge = 2, Np = 1
	{ 4,1,0 }, // Quart = 13, CutEdge = 3, Np = 0
	{ 4,1,0 }, // Quart = 13, CutEdge = 3, Np = 1

	{ 3,0,0 }, // Quart = 14, CutEdge = 0, Np = 0
	{ 3,3,1 }, // Quart = 14, CutEdge = 0, Np = 1
	{ 4,0,0 }, // Quart = 14, CutEdge = 1, Np = 0
	{ 4,0,0 }, // Quart = 14, CutEdge = 1, Np = 1
	{ 4,3,1 }, // Quart = 14, CutEdge = 2, Np = 0
	{ 4,3,1 }, // Quart = 14, CutEdge = 2, Np = 1
	{ 5,0,0 }, // Quart = 14, CutEdge = 3, Np = 0
	{ 5,3,1 }, // Quart = 14, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 15, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }  // Quart = 15, CutEdge = 3, Np = 1 UNUSED
};

void BuilderZoneRegion::updateTrans (sint32 x, sint32 y, NLLIGO::CZoneBankElement *zoneBankElement)
{
	const NLLIGO::CZoneRegion &zoneRegion = m_zoneBuilder->zoneRegion(m_regionId)->ligoZoneRegion();
	if ((x < zoneRegion.getMinX()) || (x > zoneRegion.getMaxX()) ||
			(y < zoneRegion.getMinY()) || (y > zoneRegion.getMaxY()))
		return;

	//if (!_Builder->getZoneMask(x,y))
	//	return;

	// Interpret the transition info
	x -= zoneRegion.getMinX ();
	y -= zoneRegion.getMinY ();
	sint32 m;

	// Calculate the number of material around with transition info
	std::set<std::string> matNameSet;

	ZonePosition zonePosTemp(x + zoneRegion.getMinX(), y + zoneRegion.getMinY(), m_regionId);
	LigoData dataZoneTemp;
	m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
	LigoData dataZoneTempOriginal = dataZoneTemp;

	for (m = 0; m < 4; ++m)
		matNameSet.insert(dataZoneTemp.sharingMatNames[m]);

	if (matNameSet.size() == 1)
	{
		if (dataZoneTemp.sharingMatNames[0] == STRING_UNUSED)
		{
			del(x + zoneRegion.getMinX (), y + zoneRegion.getMinY ());
			// set (x+pBZR->getMinX (), y+pBZR->getMinY (), 0, 0, STRING_UNUSED, false);
			return;
		}
		else
		{
			NLLIGO::CZoneBankElement *zoneBankElement2 = m_zoneBuilder->getZoneBank().getElementByZoneName(dataZoneTemp.zoneName);
			if ((zoneBankElement != NULL) && (zoneBankElement2 != NULL) && (zoneBankElement2->getCategory("material") == dataZoneTemp.sharingMatNames[0]))
				return;

			NLLIGO::CZoneBank &zoneBank = m_zoneBuilder->getZoneBank();
			zoneBank.resetSelection ();
			zoneBank.addOrSwitch ("material", dataZoneTemp.sharingMatNames[0]);
			zoneBank.addAndSwitch ("size", "1x1");
			std::vector<NLLIGO::CZoneBankElement *> vElts;
			zoneBank.getSelection (vElts);
			if (vElts.size() == 0)
				return;
			sint32 nRan = (sint32)(NLMISC::frand((float)vElts.size()));
			NLMISC::clamp (nRan, (sint32)0, (sint32)(vElts.size()-1));
			zoneBankElement = vElts[nRan];
			nRan = (uint32)(NLMISC::frand(1.0) * 4);
			NLMISC::clamp (nRan, (sint32)0, (sint32)3);
			uint8 rot = (uint8)nRan;
			nRan = (uint32)(NLMISC::frand(1.0) * 2);
			NLMISC::clamp (nRan, (sint32)0, (sint32)1);
			uint8 flip = (uint8)nRan;

			set(x + zoneRegion.getMinX(), y + zoneRegion.getMinY(), 0, 0, zoneBankElement->getName(), false);
			setRot(x + zoneRegion.getMinX(), y + zoneRegion.getMinY(), rot);
			setFlip(x + zoneRegion.getMinX(), y + zoneRegion.getMinY(), flip);
			return;
		}
	}

	// No 2 materials so the transition system dont work
	if (matNameSet.size() != 2)
		return;

	std::set<std::string>::iterator it = matNameSet.begin();
	std::string matA = *it;
	++it;
	std::string matB = *it;

	NLLIGO::CZoneBank &zoneBank = m_zoneBuilder->getZoneBank();
	zoneBank.resetSelection ();
	zoneBank.addOrSwitch("transname", matA + "-" + matB);
	std::vector<NLLIGO::CZoneBankElement *> selection;
	zoneBank.getSelection(selection);
	if (selection.size() == 0)
	{
		std::string matTmp = matA;
		matA = matB;
		matB = matTmp;
		zoneBank.resetSelection ();
		zoneBank.addOrSwitch ("transname", matA + "-" + matB);
		zoneBank.getSelection (selection);
	}

	if (selection.size() == 0)
		return;

	// Convert the sharingCutEdges and SharingNames to the num and type of transition
	uint8 nQuart = 0; // 0-MatA 1-MatB
	for (m = 0; m < 4; ++m)
		if (dataZoneTemp.sharingMatNames[m] == matB)
			nQuart |= (1 << m);

	if ((nQuart == 0) || (nQuart == 6) ||
			(nQuart == 9) || (nQuart == 15))
		return; // No transition for those types

	uint8 nCutEdge = 0;
	uint8 nPosCorner = 0;

	// If up edge is cut write the cut position in nCutEdge bitfield (1->0, 2->1)
	if ((nQuart == 4) || (nQuart == 5) ||
			(nQuart == 7) || (nQuart == 8) ||
			(nQuart == 10) || (nQuart == 11))
	{
		if (dataZoneTemp.sharingCutEdges[0] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.sharingCutEdges[0] = 0;
	}

	// Same for down edge
	if ((nQuart == 1) || (nQuart == 2) ||
			(nQuart == 5) || (nQuart == 10) ||
			(nQuart == 13) || (nQuart == 14))
	{
		if (dataZoneTemp.sharingCutEdges[1] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.sharingCutEdges[1] = 0;
	}

	// Same for left edge
	if ((nQuart == 1) || (nQuart == 3) ||
			(nQuart == 4) ||(nQuart == 11) ||
			(nQuart == 12) || (nQuart == 14))
	{
		if (dataZoneTemp.sharingCutEdges[2] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.sharingCutEdges[2] = 0;
	}

	// Same for right edge
	if ((nQuart == 2) || (nQuart == 3) ||
			(nQuart == 7) || (nQuart == 8) ||
			(nQuart == 12) || (nQuart == 13))
	{
		if (dataZoneTemp.sharingCutEdges[3] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}
	else
	{
		dataZoneTemp.sharingCutEdges[3] = 0;
	}

	nlassert (nPosCorner == 2); // If not this means that more than 2 edges are cut which is not possible

	STrans Trans, TransTmp1, TransTmp2;

	TransTmp1 = TranConvTable[nQuart * 8 + 2 * nCutEdge + 0];
	TransTmp2 = TranConvTable[nQuart * 8 + 2 * nCutEdge + 1];

	// Choose one or the two
	sint32 nTrans = (sint32)(NLMISC::frand(2.0f));
	NLMISC::clamp(nTrans, (sint32)0, (sint32)1);
	if (nTrans == 0)
		Trans = TransTmp1;
	else
		Trans = TransTmp2;

	zoneBank.addAndSwitch ("transnum", NLMISC::toString(Trans.Num));
	zoneBank.getSelection (selection);

	if (selection.size() > 0)
	{
		if (zoneBankElement != NULL)
		{
			dataZoneTemp.zoneName = zoneBankElement->getName();
		}
		else
		{
			nTrans = (uint32)(NLMISC::frand (1.0) * selection.size());
			NLMISC::clamp(nTrans, (sint32)0, (sint32)(selection.size() - 1));
			dataZoneTemp.zoneName = selection[nTrans]->getName();
		}
		dataZoneTemp.posX = dataZoneTemp.posY = 0;
		dataZoneTemp.rot = Trans.Rot;
		dataZoneTemp.flip = Trans.Flip;
	}

	// Add modification landscape
	if (dataZoneTempOriginal != dataZoneTemp)
		m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
}

std::string BuilderZoneRegion::getNextMatInTree (const std::string &matA, const std::string &matB)
{
	const NLLIGO::CZoneRegion &zoneRegion = m_zoneBuilder->zoneRegion(m_regionId)->ligoZoneRegion();
	uint32 i, posA = 10000, posB = 10000;

	if (matA == matB)
		return matA;

	for (i = 0; i < m_matTree.size(); ++i)
	{
		if (m_matTree[i].Name == matA)
			posA = i;
		if (m_matTree[i].Name == matB)
			posB = i;
	}
	if ((posA == 10000) || (posB == 10000))
		return STRING_UNUSED;

	std::vector<uint32> vTemp;
	tryPath (posA, posB, vTemp);
	if (vTemp.size() <= 1)
		return STRING_UNUSED;
	else
		return m_matTree[vTemp[1]].Name;
}

struct SNode
{
	sint32 NodePos, Dist, PrevNodePos;

	SNode()
	{
		NodePos = Dist = PrevNodePos = -1;
	}
};

void BuilderZoneRegion::tryPath(uint32 posA, uint32 posB, std::vector<uint32> &path)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();

	// Build the adjascence matrix
	std::vector<sint32> matAdj;
	sint32 numNodes = m_matTree.size();
	sint32 i, j, cost;
	matAdj.resize(numNodes * numNodes, -1);
	for (i = 0; i < numNodes; ++i)
		for (j = 0; j < (sint32)m_matTree[i].Arcs.size(); ++j)
			matAdj[i + m_matTree[i].Arcs[j] * numNodes] = 1;

	std::vector<SNode> vNodes; // NodesPos == index
	vNodes.resize (numNodes);
	for (i = 0; i < numNodes; ++i)
		vNodes[i].NodePos = i;
	vNodes[posA].Dist = 0;

	std::queue<SNode> qNodes;
	qNodes.push (vNodes[posA]);

	while (qNodes.size() > 0)
	{
		SNode node = qNodes.front();
		qNodes.pop ();

		for (i = 0; i < numNodes; ++i)
		{
			cost = matAdj[node.NodePos + i * numNodes];
			if (cost != -1)
			{
				if ((vNodes[i].Dist == -1) || (vNodes[i].Dist > (cost + node.Dist)))
				{
					vNodes[i].Dist = cost + node.Dist;
					vNodes[i].PrevNodePos = node.NodePos;
					qNodes.push(vNodes[i]);
				}
			}
		}
	}

	// Get path length
	i = posB;
	j = 0;
	while (i != -1)
	{
		++j;
		i = vNodes[i].PrevNodePos;
	}

	// Write the path in the good order (from posA to posB)
	path.resize(j);
	i = posB;
	while (i != -1)
	{
		--j;
		path[j] = i;
		i = vNodes[i].PrevNodePos;
	}
}

void BuilderZoneRegion::del(sint32 x, sint32 y, bool transition, ToUpdate *pUpdate)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();
	if (!m_zoneBuilder->getZoneMask(x, y))
		return;

	const std::string &nameZone = zoneRegion.getName(x, y);
	NLLIGO::CZoneBankElement *zoneBankElement = m_zoneBuilder->getZoneBank().getElementByZoneName(nameZone);
	if (zoneBankElement != NULL)
	{
		sint32 sizeX = zoneBankElement->getSizeX(), sizeY = zoneBankElement->getSizeY();
		sint32 posX = zoneRegion.getPosX (x, y), posY = zoneRegion.getPosY (x, y);
		uint8 rot = zoneRegion.getRot (x, y);
		uint8 flip = zoneRegion.getFlip (x, y);
		sint32 i, j;
		sint32 deltaX, deltaY;

		if (flip == 0)
		{
			switch (rot)
			{
			case 0:
				deltaX = -posX;
				deltaY = -posY;
				break;
			case 1:
				deltaX = -(sizeY - 1 - posY);
				deltaY = -posX;
				break;
			case 2:
				deltaX = -(sizeX - 1 - posX);
				deltaY = -(sizeY - 1 - posY);
				break;
			case 3:
				deltaX = -posY;
				deltaY = -(sizeX - 1 - posX);
				break;
			}
		}
		else
		{
			switch (rot)
			{
			case 0:
				deltaX = -(sizeX - 1 - posX);
				deltaY = -posY;
				break;
			case 1:
				deltaX = -(sizeY - 1 - posY);
				deltaY = -(sizeX - 1 - posX);
				break;
			case 2:
				deltaX = -posX;
				deltaY = -(sizeY - 1 - posY);
				break;
			case 3:
				deltaX = -posY;
				deltaY = -posX;
				break;
			}
		}

		NLLIGO::SPiece mask;
		mask.Tab.resize (sizeX * sizeY);
		for(i = 0; i < sizeX * sizeY; ++i)
			mask.Tab[i] = zoneBankElement->getMask()[i];
		mask.w = sizeX;
		mask.h = sizeY;
		mask.rotFlip (rot, flip);

		for (j = 0; j < mask.h; ++j)
			for (i = 0; i < mask.w; ++i)
				if (mask.Tab[i + j * mask.w])
				{
					set(x + deltaX + i, y + deltaY + j, 0, 0, STRING_UNUSED, true);
					setRot(x + deltaX + i, y + deltaY + j, 0);
					setFlip(x + deltaX + i, y + deltaY + j, 0);
					if (pUpdate != NULL)
					{
						pUpdate->add(this, x + deltaX + i, y + deltaY + j, "");
					}
				}
		if (!transition)
			reduceMin ();
	}
	else
	{
		if ((x < zoneRegion.getMinX()) || (x > zoneRegion.getMaxX()) ||
				(y < zoneRegion.getMinY()) || (y > zoneRegion.getMaxY()))
			return;

		ZonePosition zonePosTemp(x, y, m_regionId);
		LigoData dataZoneTemp;
		m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
		LigoData dataZoneTempOriginal = dataZoneTemp;

		dataZoneTemp.zoneName = STRING_UNUSED;
		dataZoneTemp.posX = 0;
		dataZoneTemp.posY = 0;

		for (uint32 i = 0; i < 4; ++i)
		{
			dataZoneTemp.sharingMatNames[i] = STRING_UNUSED;
			dataZoneTemp.sharingCutEdges[i] = 0;
		}

		// Add modification landscape
		if (dataZoneTempOriginal != dataZoneTemp)
			m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);

	}
}

void BuilderZoneRegion::move (sint32 x, sint32 y)
{
	m_zoneBuilder->actionLigoMove(m_regionId, x, y);
}

uint32 BuilderZoneRegion::countZones ()
{
	const NLLIGO::CZoneRegion &zoneRegion = m_zoneBuilder->zoneRegion(m_regionId)->ligoZoneRegion();
	sint32 x, y;

	uint32 counter = 0;

	for (y = zoneRegion.getMinY (); y <= zoneRegion.getMaxY (); ++y)
		for (x = zoneRegion.getMinX (); x <= zoneRegion.getMaxX (); ++x)
			if (zoneRegion.getName (x, y) != STRING_UNUSED)
				++counter;

	return counter;
}

void BuilderZoneRegion::set(sint32 x, sint32 y, sint32 posX, sint32 posY,
							const std::string &zoneName, bool transition)
{
	ZoneRegionObject *zoneRegionObj = m_zoneBuilder->zoneRegion(m_regionId);
	if (zoneRegionObj == 0)
		return;

	const NLLIGO::CZoneRegion &zoneRegion = zoneRegionObj->ligoZoneRegion();

	// Do we need to resize ?
	if ((x < zoneRegion.getMinX()) || (x > zoneRegion.getMaxX()) ||
			(y < zoneRegion.getMinY()) || (y > zoneRegion.getMaxY()))
	{
		sint32 newMinX = (x < zoneRegion.getMinX() ? x: zoneRegion.getMinX()), newMinY = (y < zoneRegion.getMinY() ? y: zoneRegion.getMinY());
		sint32 newMaxX = (x > zoneRegion.getMaxX() ? x: zoneRegion.getMaxX()), newMaxY = (y > zoneRegion.getMaxY() ? y: zoneRegion.getMaxY());

		resize(newMinX, newMaxX, newMinY, newMaxY);
	}

	ZonePosition zonePosTemp(x, y, m_regionId);
	LigoData dataZoneTemp;
	if (!m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp))
		return;
	LigoData dataZoneTempOriginal = dataZoneTemp;

	dataZoneTemp.zoneName = zoneName;
	dataZoneTemp.posX = (uint8)posX;
	dataZoneTemp.posY = (uint8)posY;

	if (!transition)
	{
		NLLIGO::CZoneBankElement *zoneBankElem = m_zoneBuilder->getZoneBank().getElementByZoneName(zoneName);
		if (zoneBankElem == NULL)
			return;

		const std::string &matName = zoneBankElem->getCategory ("material");
		if (matName == STRING_NO_CAT_TYPE)
			return;
		for (uint32 i = 0; i < 4; ++i)
		{
			dataZoneTemp.sharingMatNames[i] = matName;
			dataZoneTemp.sharingCutEdges[i] = 0;
		}

		// Add modification landscape
		if (dataZoneTempOriginal != dataZoneTemp)
			m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);

		BuilderZoneRegion *builderZoneRegion = this;
		ZonePosition zonePosU;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x - 1, y - 1))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if (data.sharingMatNames[3] != matName)
			{
				data.sharingMatNames[3] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
		builderZoneRegion = this;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x, y - 1))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if ((data.sharingMatNames[2] != matName) || (data.sharingMatNames[3] != matName))
			{
				data.sharingMatNames[2] = matName;
				data.sharingMatNames[3] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
		builderZoneRegion = this;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x + 1, y - 1))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if (data.sharingMatNames[2] != matName)
			{
				data.sharingMatNames[2] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
		builderZoneRegion = this;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x - 1, y))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if ((data.sharingMatNames[1] != matName) || (data.sharingMatNames[3] != matName))
			{
				data.sharingMatNames[1] = matName;
				data.sharingMatNames[3] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
		builderZoneRegion = this;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x + 1, y))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if ((data.sharingMatNames[0] != matName) || (data.sharingMatNames[2] != matName))
			{
				data.sharingMatNames[0] = matName;
				data.sharingMatNames[2] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
		builderZoneRegion = this;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x - 1, y + 1))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if (data.sharingMatNames[1] != matName)
			{
				data.sharingMatNames[1] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
		builderZoneRegion = this;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x, y + 1))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if ((data.sharingMatNames[0] != matName) || (data.sharingMatNames[1] != matName))
			{
				data.sharingMatNames[0] = matName;
				data.sharingMatNames[1] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
		builderZoneRegion = this;
		if (m_zoneBuilder->getZoneAmongRegions(zonePosU, builderZoneRegion, x + 1, y + 1))
		{
			LigoData data;
			m_zoneBuilder->ligoData(data, zonePosU);
			if (data.sharingMatNames[0] != matName)
			{
				data.sharingMatNames[0] = matName;

				// Add modification landscape
				m_zoneBuilder->actionLigoTile(data, zonePosU);
			}
		}
	}
	else
	{
		// Add modification landscape
		if (dataZoneTempOriginal != dataZoneTemp)
			m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
	}
}

void BuilderZoneRegion::setRot (sint32 x, sint32 y, uint8 rot)
{
	ZonePosition zonePosTemp(x, y, m_regionId);
	LigoData dataZoneTemp;
	m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
	if (dataZoneTemp.rot != rot)
	{
		dataZoneTemp.rot = rot;

		// Add modification landscape
		m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
	}
}

void BuilderZoneRegion::setFlip(sint32 x, sint32 y, uint8 flip)
{
	ZonePosition zonePosTemp(x, y, m_regionId);
	LigoData dataZoneTemp;
	m_zoneBuilder->ligoData(dataZoneTemp, zonePosTemp);
	if (dataZoneTemp.flip != flip)
	{
		dataZoneTemp.flip = flip;

		// Add modification landscape
		m_zoneBuilder->actionLigoTile(dataZoneTemp, zonePosTemp);
	}
}


void BuilderZoneRegion::reduceMin ()
{
	const NLLIGO::CZoneRegion &zoneRegion = m_zoneBuilder->zoneRegion(m_regionId)->ligoZoneRegion();
	sint32 i, j;

	sint32 newMinX = zoneRegion.getMinX(), newMinY = zoneRegion.getMinY ();
	sint32 newMaxX = zoneRegion.getMaxX(), newMaxY = zoneRegion.getMaxY ();
	bool bCanSuppr;

	// Reduce the MinY
	while (true)
	{
		if (newMinY == newMaxY)
			break;
		j = newMinY;
		bCanSuppr = true;
		for (i = newMinX; i <= newMaxX; ++i)
		{
			std::string str = zoneRegion.getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			++newMinY;
		else
			break;
	}

	// Reduce the MaxY
	while (true)
	{
		if (newMinY == newMaxY)
			break;
		j = newMaxY;
		bCanSuppr = true;
		for (i = newMinX; i <= newMaxX; ++i)
		{
			std::string str = zoneRegion.getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			--newMaxY;
		else
			break;
	}

	// Reduce the MinX
	while (true)
	{
		if (newMinX == newMaxX)
			break;
		i = newMinX;
		bCanSuppr = true;
		for (j = newMinY; j <= newMaxY; ++j)
		{
			std::string str = zoneRegion.getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			++newMinX;
		else
			break;
	}

	// Reduce the MaxX
	while (true)
	{
		if (newMinX == newMaxX)
			break;
		i = newMaxX;
		bCanSuppr = true;
		for (j = newMinY; j <= newMaxY; ++j)
		{
			std::string str = zoneRegion.getName (i, j) ;
			if (!str.empty() && (str != STRING_UNUSED))
			{
				bCanSuppr = false;
				break;
			}
		}
		if (bCanSuppr)
			--newMaxX;
		else
			break;
	}

	if ((newMinX != zoneRegion.getMinX ()) ||
			(newMinY != zoneRegion.getMinY ()) ||
			(newMaxX != zoneRegion.getMaxX ()) ||
			(newMaxY != zoneRegion.getMaxY ()))
	{
		resize(newMinX, newMaxX, newMinY, newMaxY);
	}
}

uint BuilderZoneRegion::getRegionId() const
{
	return m_regionId;
}

void BuilderZoneRegion::resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	const NLLIGO::CZoneRegion &zoneRegion = m_zoneBuilder->zoneRegion(m_regionId)->ligoZoneRegion();
	if ((zoneRegion.getMinX ()!= newMinX) ||
			(zoneRegion.getMaxX ()!= newMaxX) ||
			(zoneRegion.getMinY ()!= newMinY) ||
			(zoneRegion.getMaxY ()!= newMaxY))
	{
		m_zoneBuilder->actionLigoResize(m_regionId, newMinX, newMaxX, newMinY, newMaxY);
	}
}

void BuilderZoneRegion::placePiece(sint32 x, sint32 y, uint8 rot, uint8 flip,
								   NLLIGO::SPiece &sMask, NLLIGO::SPiece &sPosX, NLLIGO::SPiece &sPosY,
								   const std::string &eltName)
{
	for (int j = 0; j < sMask.h; ++j)
		for (int i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i + j * sMask.w])
			{
				set(x + i, y + j, sPosX.Tab[i + j * sPosX.w], sPosY.Tab[i + j * sPosY.w], eltName);
				setRot(x + i, y + j, rot);
				setFlip(x + i, y + j, flip);
			}
}

} /* namespace LandscapeEditor */
