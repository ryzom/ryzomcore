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

#ifndef R2_NPC_EDITOR_H
#define R2_NPC_EDITOR_H

// Misc
#include "nel/misc/singleton.h"

namespace NLGUI
{
	class CGroupContainer;
}

class CEntityCL;

namespace R2
{

class CDisplayerVisualEntity;

class CNPCEditor : public NLMISC::CSingleton<CNPCEditor>
{

public:

	//NLMISC_DECLARE_CLASS(R2::CNPCEditor);
	// ctor
	CNPCEditor();
	// dtor
	~CNPCEditor();

	virtual void updateNPCView(uint slot);

private:


private:

	CDisplayerVisualEntity *	_DisplayerVE;
	NLGUI::CGroupContainer *			_NPCWindow;

};


} // R2

#endif

