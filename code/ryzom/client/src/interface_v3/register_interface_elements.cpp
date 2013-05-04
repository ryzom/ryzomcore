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
#include "interface_3d_scene.h"
#include "view_radar.h"
#include "dbctrl_sheet.h"
#include "dbgroup_list_sheet.h"
#include "sphrase_manager.h"
#include "../r2/displayer_visual.h"
#include "../r2/displayer_visual_entity.h"
#include "../r2/displayer_visual_group.h"
#include "../r2/instance.h"
#include "../r2/tool.h"
#include "../r2/tool_pick.h"
#include "view_pointer_ryzom.h"
#include "nel/gui/reflect_register.h"

void registerInterfaceElements()
{
	CViewPointerRyzom::forceLinking();

	REGISTER_REFLECTABLE_CLASS(CViewRadar, CViewBase);
	REGISTER_REFLECTABLE_CLASS(CDBCtrlSheet, CCtrlDraggable);
	REGISTER_REFLECTABLE_CLASS(IListSheetBase, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CInterface3DScene, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CInterface3DCharacter, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DIG, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DShape, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DCamera, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DLight, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DFX, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(R2::CInstance, CReflectable);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerBase, CReflectable);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerVisual, R2::CDisplayerBase);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerVisualEntity, R2::CDisplayerVisual);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerVisualGroup, R2::CDisplayerVisual);
	REGISTER_REFLECTABLE_CLASS(R2::CTool, CReflectable);
	REGISTER_REFLECTABLE_CLASS(R2::CToolPick, R2::CTool);
	REGISTER_REFLECTABLE_CLASS(CSPhraseComAdpater, CReflectable);	
}



