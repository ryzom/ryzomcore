// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef __ZONE_REGION_H__
#define __ZONE_REGION_H__

// ***************************************************************************

#include "nel/misc/stream.h"
#include <vector>
#include <string>

namespace NLLIGO
{

// ***************************************************************************

#define STRING_OUT_OF_BOUND "< OOB >"
#define STRING_UNUSED		"< UNUSED >"

// ***************************************************************************

struct SPiece
{
	sint32				w, h;			// Max 255x255
	std::vector<uint8>	Tab;

	void			rotFlip (uint8 rot, uint8 flip);
};

// ***************************************************************************

class CZoneRegion
{

public:

	CZoneRegion ();

	void				serial (NLMISC::IStream &f);
	void				resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY);
	void				basicSet (sint32 x, sint32 y, sint32 PosX, sint32 PosY,  const std::string &ZoneName);


	// Accessors
	const std::string	&getName (sint32 x, sint32 y) const;
	uint8				getPosX (sint32 x, sint32 y) const;
	uint8				getPosY (sint32 x, sint32 y) const;
	uint8				getRot (sint32 x, sint32 y) const;
	uint8				getFlip (sint32 x, sint32 y) const;
	uint8				getCutEdge (sint32 x, sint32 y, uint8 pos) const; // pos==0 -> getUpCE, pos==1 -> getDownCE, ...
	uint32				getDate (sint32 x, sint32 y, uint8 lowOrHigh) const; // lowOrHigh == 0 -> low
	std::string			getSharingMatNames (sint32 x, sint32 y, uint edge);
	uint8				getSharingCutEdges (sint32 x, sint32 y, uint edge);

	// Accessors
	bool				setName (sint32 x, sint32 y, const std::string &newValue);
	bool				setPosX (sint32 x, sint32 y, uint8 newValue);
	bool				setPosY (sint32 x, sint32 y, uint8 newValue);
	bool				setRot (sint32 x, sint32 y, uint8 newValue);
	bool				setFlip (sint32 x, sint32 y, uint8 newValue);
	bool				setSharingMatNames (sint32 x, sint32 y, uint edge, const std::string &newValue);
	bool				setSharingCutEdges (sint32 x, sint32 y, uint edge, uint8 newValue);

	sint32				getMinX () const { return _MinX; };
	sint32				getMaxX () const { return _MaxX; };
	sint32				getMinY () const { return _MinY; };
	sint32				getMaxY () const { return _MaxY; };

	void				setMinX (sint32 newValue) { _MinX = newValue; };
	void				setMaxX (sint32 newValue) { _MaxX = newValue; };
	void				setMinY (sint32 newValue) { _MinY = newValue; };
	void				setMaxY (sint32 newValue) { _MaxY = newValue; };

protected:

	// An element of the grid
	struct SZoneUnit
	{
		std::string			ZoneName;
		uint8				PosX, PosY; // Position in a large piece
		uint8				Rot, Flip; // Rot 0-0deg, 1-90deg, 2-180deg, 3-270deg, Flip 0-false, 1-true

		// Work Data : For transition				[2 3]
		std::string			SharingMatNames[4];	//  [0 1]
		uint8				SharingCutEdges[4]; // 0-Up, 1-Down, 2-Left, 3-Right (value [0-2])

		SZoneUnit ();
		void			serial (NLMISC::IStream &f);
		const SZoneUnit&operator= (const SZoneUnit&zu);
	};

	struct SZoneUnit2 : public SZoneUnit
	{
		uint32				DateLow;
		uint32				DateHigh;

		SZoneUnit2 ();
		void			serial (NLMISC::IStream &f);
		const SZoneUnit2&operator= (const SZoneUnit2&zu);
		const SZoneUnit2&operator= (const SZoneUnit&zu);
	};

	static std::string			_StringOutOfBound;

protected:

	std::vector<SZoneUnit2>		_Zones;
	sint32						_MinX, _MinY;
	sint32						_MaxX, _MaxY;

};


} // namespace NLLIGO

#endif // __ZONE_REGION_H__
