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



#ifndef CL_ID_TO_STRING_ARRAY_H
#define CL_ID_TO_STRING_ARRAY_H

#include "entity_sheet.h"


/**
  * An ID and its matching string. Intended to be used in an array (class CIDToStringArraySheet)
  */
class CIDToString
{
public:
	uint32 ID;
	std::string String;
public:
	virtual void build(const NLGEORGES::UFormElm &item);
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};


/** Map a list of IDs to a list of string
  */
class CIDToStringArraySheet : public CEntitySheet
{
public:
	std::vector<CIDToString> Array;
public:
	// ctor
	CIDToStringArraySheet();
	/// Build the entity from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);
	/// Serialize character sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};



#endif
