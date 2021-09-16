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



#ifndef RY_PREREQUISITS_H
#define RY_PREREQUISITS_H

#include "nel/misc/types_nl.h"
#include <vector>


struct CPrerequisitDesc
{
	inline CPrerequisitDesc() : Description(0), Validated(false),IsMandatory(true)
	{}

	inline CPrerequisitDesc( uint32 desc, bool ok, bool mandatory ) : Description(desc), Validated(ok),IsMandatory(mandatory)
	{}

	/// serial
	void serial(NLMISC::IStream & s)
	{
		s.serial(Description);
		s.serial(Validated);
		s.serial(IsMandatory);
	}

	/// string id used to describe the prerequisit
	uint32	Description;
	/// status (true = ok, false = prerequisit not met)
	bool	Validated;
	/// is mandatory or not
	bool	IsMandatory;
};

/**
 * class used to store the mission prerequisit infos and to send it to the client
 * \author David Fleury
 * \author Nevrax France
 * \date 2005
 */
class CPrerequisitInfos
{
public:
	/// ctor
	CPrerequisitInfos()
	{}

	/// serial
	void serial(NLMISC::IStream & s)
	{
		s.serialCont(Prerequisits);
	}


	std::vector<CPrerequisitDesc>	Prerequisits;

};

#endif // RY_PREREQUISITS_H //

