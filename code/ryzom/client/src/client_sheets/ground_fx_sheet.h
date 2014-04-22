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



#ifndef RY_GROUND_FX_SHEET_H
#define RY_GROUND_FX_SHEET_H

#include "client_sheets.h"

namespace NLGEORGES
{
	class UFormElm;
}

/** A ground fx and its associated ground material
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CGroundFXSheet
{
public:
	uint32					GroundID;
	NLMISC::TSStringId		IdFXName;

	std::string	getFXName() const {return ClientSheetsStrings.get(IdFXName);}
public:
	// build that sheet from a form
	bool build(const NLGEORGES::UFormElm &item);
	// serial this sheet
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};

// operator < sort ground fx by ground type
inline bool operator  < (const CGroundFXSheet &lhs, const CGroundFXSheet &rhs)
{
	return lhs.GroundID < rhs.GroundID;
}
//
inline bool operator == (const CGroundFXSheet &lhs, const CGroundFXSheet &rhs)
{
	return lhs.GroundID == rhs.GroundID;
}

#endif

