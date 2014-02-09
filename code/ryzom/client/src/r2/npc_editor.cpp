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
#include "npc_editor.h"
#include "../entity_cl.h"
#include "../interface_v3/interface_3d_scene.h"
#include "../interface_v3/character_3d.h"
#include "editor.h"
#include "../interface_v3/interface_manager.h"
#include "nel/gui/group_container.h"
#include "displayer_visual_entity.h"
#include "nel/gui/dbgroup_combo_box.h"

#include "../sheet_manager.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;


namespace R2
{

CNPCEditor::CNPCEditor()
{

}

//--------------------------------------------------------------------------------

CNPCEditor::~CNPCEditor()
{

}

//--------------------------------------------------------------------------------

void CNPCEditor::updateNPCView(uint slot)
{
	_NPCWindow = dynamic_cast<CGroupContainer *>(
		CWidgetManager::getInstance()->getElementFromId("ui:interface:r2ed_npc"));
	if (!_NPCWindow)
	{
		nlwarning("<CNPCEditor::updateNPCView> can't retrieve npc window, or bad type");
	}
	else
	{
		CInterface3DScene *sceneI = dynamic_cast<CInterface3DScene *>(_NPCWindow->getGroup("char3d"));
		if (!sceneI)
		{
			nlwarning("<CNPCEditor::updateNPCView> can't retrieve character 3d view, or bad type");
		}

		CInterface3DCharacter *char3DI = NULL;
		if (sceneI->getCharacter3DCount() != 0)
		{
			char3DI = sceneI->getCharacter3D(0);
		}
		if (char3DI == NULL)
		{
			nlwarning("<CNPCEditor::updateNPCView> Can't retrieve char 3D Interface");
		}
		else
		{
			CCharacter3D * char3D = NULL;
			char3D = char3DI->getCharacter3D();
			if (char3D == NULL)
			{
				nlwarning("<CNPCEditor::updateNPCView> Can't retrieve char3D");
			}
			else
			{
				SCharacter3DSetup c3Ds = char3D->getCurrentSetup();
				c3Ds.setupFromSERVERDataBase(slot);
				char3D->setup(c3Ds);
			}
		}
	}
}

//--------------------------------------------------------------------------------

}

