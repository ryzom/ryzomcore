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

#ifndef BUILDER_ZONE_REGION_H
#define BUILDER_ZONE_REGION_H

// Project includes

// NeL includes
#include <nel/ligo/zone_bank.h>
#include <nel/ligo/zone_region.h>

// STL includes
#include <string>
#include <vector>
#include <queue>

// Qt includes

namespace LandscapeEditor
{
class ZoneBuilder;
class ToUpdate;

// CZoneRegion contains informations about the zones painted.
// (Legacy class from old world editor. It needs to refactoring!)
class BuilderZoneRegion
{
public:

	explicit BuilderZoneRegion(uint regionId);

	// New interface
	bool init(ZoneBuilder *zoneBuilder);
	void add(sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement);
	void invertCutEdge(sint32 x, sint32 y, uint8 cePos);
	void cycleTransition(sint32 x, sint32 y);
	bool addNotPropagate(sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement);

	/// Brutal adding a zone over empty space do not propagate in any way -> can result
	/// in inconsistency when trying the propagation mode
	void addForce (sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement);
	void del(sint32 x, sint32 y, bool transition = false, ToUpdate *pUpdate = 0);
	void move(sint32 x, sint32 y);
	uint32 countZones();
	void reduceMin();
	uint getRegionId() const;

private:

	// An element of the graph
	struct SMatNode
	{
		std::string Name;
		// Position in the tree (vector of nodes)
		std::vector<uint32>	Arcs;
	};

	void addTransition(sint32 x, sint32 y, uint8 rot, uint8 flip, NLLIGO::CZoneBankElement *zoneBankElement);

	void addToUpdateAndCreate(BuilderZoneRegion *builderZoneRegion, sint32 sharePos, sint32 x, sint32 y,
							  const std::string &newMat, ToUpdate *ptCreate, ToUpdate *ptUpdate);

	void putTransitions(sint32 x, sint32 y, const NLLIGO::SPiece &mask, const std::string &matName, ToUpdate *ptUpdate);
	void updateTrans(sint32 x, sint32 y, NLLIGO::CZoneBankElement *zoneBankElement = 0);

	std::string getNextMatInTree(const std::string &matA, const std::string &matB);

	/// Find the fastest way between posA and posB in the MatTree (Dijkstra)
	void tryPath(uint32 posA, uint32 posB, std::vector<uint32> &path);

	void set(sint32 x, sint32 y, sint32 posX, sint32 posY, const std::string &zoneName, bool transition = false);
	void setRot(sint32 x, sint32 y, uint8 rot);
	void setFlip(sint32 x, sint32 y, uint8 flip);
	void resize(sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY);

	void placePiece(sint32 x, sint32 y, uint8 rot, uint8 flip,
					NLLIGO::SPiece &sMask, NLLIGO::SPiece &sPosX, NLLIGO::SPiece &sPosY,
					const std::string &eltName);

	uint m_regionId;

	// To use the global mask
	ZoneBuilder *m_zoneBuilder;

	// The tree of transition between materials
	std::vector<SMatNode> m_matTree;

	bool m_firstInit;
};

} /* namespace LandscapeEditor */

#endif // BUILDER_ZONE_REGION_H
