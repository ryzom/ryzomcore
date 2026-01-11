// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERZONEREGION_H
#define BUILDERZONEREGION_H

// ***************************************************************************

namespace NLLIGO
{
	class CZoneBank;
	class CZoneBankElement;
	struct SPiece;
}

namespace NLMISC
{
	class IStream;
}

class CBuilderZone;

// ***************************************************************************

// CZoneRegion contains information about the zones painted
class CBuilderZoneRegion
{

public:
	uint				RegionId;

public:

	CBuilderZoneRegion (uint regionId);


	// New interface
	bool				init (NLLIGO::CZoneBank *pBank, CBuilderZone *pBuilder, std::string &error);
	void				add (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt);
	void				invertCutEdge (sint32 x, sint32 y, uint8 cePos);
	void				cycleTransition (sint32 x, sint32 y);
	bool				addNotPropagate (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt);
	void				addForce (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt);
	void				del (sint32 x, sint32 y, bool transition=false, void *pInternal=NULL);
	void				move (sint32 x, sint32 y);
	uint32				countZones ();
	void				reduceMin ();


	// Accessors
	
private:

	// An element of the graph
	struct SMatNode
	{
		std::string			Name;
		std::vector<uint32>	Arcs; // Position in the tree (vector of nodes)
	};

private:

	NLLIGO::CZoneBank			*_ZeBank;
	CBuilderZone				*_Builder; // To use the global mask

	std::vector<SMatNode>		_MatTree; // The tree of transition between materials

private:

	void				addTransition (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt);
	
	void				addToUpdateAndCreate (CBuilderZoneRegion* pBZRfrom, sint32 sharePos, sint32 x, sint32 y, const std::string &sNewMat, void *pInt1, void *pInt2);

	void				putTransitions (sint32 x, sint32 y, const NLLIGO::SPiece &rMask, const std::string &MatName, void *pInternal);
	//void				putTransition (sint32 x, sint32 y, const std::string &MatName);
	void				updateTrans (sint32 x, sint32 y, NLLIGO::CZoneBankElement *pElt = NULL);

	std::string			getNextMatInTree(const std::string &sMatA, const std::string &sMatB);
	void				tryPath (uint32 posA, uint32 posB, std::vector<uint32> &vPath);

	void				set (sint32 x, sint32 y, sint32 nPosX, sint32 nPosY, const std::string &ZoneName, bool transition=false);
	void				setRot (sint32 x, sint32 y, uint8 rot);
	void				setFlip (sint32 x, sint32 y, uint8 flip);
	void				resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY);

};

#endif // BUILDERZONEREGION_H