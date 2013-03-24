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
#include <string>
#include <map>
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/gui/ctrl_sheet_selection.h"
#include "nel/gui/view_renderer.h"

namespace NLGUI
{

	//=============================================================
	void CSheetSelectionGroup::setTexture(const std::string &texName)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TextureIndex = rVR.getTextureIdFromName(texName);
		rVR.getTextureSizeFromId(_TextureIndex, _TextureWidth, _TextureHeight);
	}

	//=============================================================
	void CCtrlSheetSelection::deleteGroups()
	{
		_Groups.clear();
		_GroupNameToIndex.clear();
	}

	//=============================================================
	sint  CCtrlSheetSelection::addGroup(const std::string &name)
	{
		if (getGroupIndex(name) != -1)
		{
			nlwarning("<CCtrlSheetSelection::addGroup> Group inserted twice : %s", name.c_str());
			return - 1;
		}
		_Groups.push_back(CSheetSelectionGroup(name));
		_GroupNameToIndex[name] = (uint)_Groups.size() - 1;
		return (sint)_Groups.size() - 1;
	}

	//=============================================================
	sint CCtrlSheetSelection::getGroupIndex(const std::string &name) const
	{
		TGroupNameToIndex::const_iterator it = _GroupNameToIndex.find(name);
		return it == _GroupNameToIndex.end() ? - 1 : (sint) it->second;
	}

	//=============================================================
	CSheetSelectionGroup *CCtrlSheetSelection::getGroup(const std::string &name)
	{
		return getGroup(getGroupIndex(name));
	}

	//=============================================================
	const CSheetSelectionGroup *CCtrlSheetSelection::getGroup(const std::string &name) const
	{
		return getGroup(getGroupIndex(name));
	}

	//=============================================================
	CSheetSelectionGroup *CCtrlSheetSelection::getGroup(uint index)
	{
		if (index > _Groups.size())
		{
			// nlwarning("<CCtrlSheetSelection::getGroup> invalid group index");
			return NULL;
		}
		return &_Groups[index];
	}

	//=============================================================
	const CSheetSelectionGroup *CCtrlSheetSelection::getGroup(uint index) const
	{
		if (index > _Groups.size())
		{
			nlwarning("<CCtrlSheetSelection::getGroup> invalid group index");
			return NULL;
		}
		return &_Groups[index];
	}

}


