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
#include "animation_fx_id_array.h"
#include "sheet_manager.h"
#include "global.h"
//
#include "nel/3d/u_scene.h"

extern NL3D::UScene *Scene;

// *********************************************************************
CAnimationFXIDArray::CAnimationFXIDArray()
{
	_AnimSet = NULL;
}

// *********************************************************************
void CAnimationFXIDArray::release()
{
	_IDToFXArray.clear();
	if (Scene && _AnimSet)
	{
		Driver->deleteAnimationSet(_AnimSet);
	}
	_AnimSet = NULL;
}

// *********************************************************************
void CAnimationFXIDArray::init(const CIDToStringArraySheet &sheet, NL3D::UAnimationSet *animSet, bool mustDeleteAnimSet /* = false*/)
{
	release();
	// retrieve pointer on all fxs
	for(uint k = 0; k < sheet.Array.size(); ++k)
	{
		const CAnimationFXSheet *afs = dynamic_cast<const CAnimationFXSheet *>(SheetMngr.get(NLMISC::CSheetId(sheet.Array[k].String)));
		if (afs)
		{
			CIDToFX idToFX;
			idToFX.FX.init(afs, animSet);
			idToFX.ID = sheet.Array[k].ID;
			_IDToFXArray.push_back(idToFX);
		}
	}
	// sort by ids
	std::sort(_IDToFXArray.begin(), _IDToFXArray.end());
	if (mustDeleteAnimSet)
	{
		_AnimSet = animSet;
	}
}

// *********************************************************************
void CAnimationFXIDArray::init(const std::string &sheetName, NL3D::UAnimationSet *animSet, bool mustDeleteAnimSet /*= false*/)
{
	CIDToStringArraySheet *array = dynamic_cast<CIDToStringArraySheet *>(SheetMngr.get(NLMISC::CSheetId(sheetName)));
	if (array)
	{
		init(*array, animSet, mustDeleteAnimSet);
	}
}


// *********************************************************************
const CAnimationFX *CAnimationFXIDArray::getFX(uint32 id) const
{
	// after init, element are sorted by ids
	CIDToFX comp;
	comp.ID = id;
	std::vector<CIDToFX>::const_iterator it = std::lower_bound(_IDToFXArray.begin(), _IDToFXArray.end(), comp);
	if (it == _IDToFXArray.end()) return NULL;
	if (it->ID != id) return NULL;
	return &(it->FX);
}
