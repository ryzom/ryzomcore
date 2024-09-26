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



#include "stdpch.h"

#include "list_sheet_base.h"
#include "nel/misc/common.h"


using namespace NLMISC;


// ***************************************************************************
IListSheetBase::IListSheetBase(const TCtorParam &param)
	: CInterfaceGroup(param)
{
	_Sectionable= false;
	_SectionEmptyScheme= NoEmptySection;
	// if fails one time to create the section group, abort for ever
	_SectionableError= false;
}


// ***************************************************************************
bool IListSheetBase::insertSectionGroup(uint sectionGroupIndex, uint sectionId, sint xGroup, sint yGroup)
{
	// if not exist, create new
	nlassert(sectionGroupIndex<=_SectionGroups.size());
	if(sectionGroupIndex==_SectionGroups.size())
	{
		CInterfaceGroup		*group= createSectionGroup(toString("section_group%d", sectionGroupIndex));
		if(!group)
			return false;
		_SectionGroups.push_back(group);
	}

	// change id
	CInterfaceGroup		*group= _SectionGroups[sectionGroupIndex];
	setSectionGroupId(group, sectionId);
	// change position
	group->setX(xGroup);
	group->setY(yGroup);

	return true;
}

// ***************************************************************************
void IListSheetBase::releaseSectionGroups(uint startSectionGroupIndex)
{
	// for any group >= startSectionGroupIndex
	for(uint i=startSectionGroupIndex;i<_SectionGroups.size();i++)
	{
		deleteSectionGroup(_SectionGroups[i]);
	}
	_SectionGroups.resize(startSectionGroupIndex);
}

