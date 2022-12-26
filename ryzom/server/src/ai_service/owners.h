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



#ifndef _OWNERS_
#define _OWNERS_

#include "ai_entity_physical.h"
#include "ai_entity_physical_inline.h"
#include "ai_instance.h"
#include "ai_grp_pet.h"

//	used to allow entity to handle pets.
class	CPetOwner
{
public:
	explicit	CPetOwner	()	:	_petGroup(NULL)
	{
	}
	virtual ~CPetOwner	();

	virtual	CAIEntityPhysical	&getPhysical()=0;
	inline	CGrpPet	*getPetGroup()
	{
		return	_petGroup;
	}
	inline	void	setPetGroup(CGrpPet	*petGroup)
	{
		nlassert(!getPetGroup());
		_petGroup=petGroup;
	}

	inline	void	removePetGroup()
	{
		_petGroup=NULL;
	}

private:
	NLMISC::CSmartPtr<CGrpPet>	_petGroup;
};

#endif
