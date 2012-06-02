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
#include "interface_element.h"
#include "interface_3d_scene.h"
#include "view_base.h"
#include "view_text.h"
#include "view_text_id.h"
#include "view_bitmap.h"
#include "view_radar.h"
#include "group_menu.h"
#include "ctrl_base.h"
#include "interface_group.h"
#include "group_frame.h"
#include "group_container.h"
#include "group_list.h"
#include "dbgroup_select_number.h"
#include "ctrl_button.h"
#include "ctrl_text_button.h"
#include "ctrl_col_pick.h"
#include "dbctrl_sheet.h"
#include "dbgroup_list_sheet.h"
#include "group_editbox.h"
#include "group_tree.h"
#include "reflect.h"
#include "dbview_bar.h"
#include "dbview_bar3.h"
#include "group_list.h"
#include "ctrl_scroll.h"
#include "dbgroup_combo_box.h"
#include "ctrl_scroll.h"
#include "group_tab.h"
#include "group_html.h"
#include "group_header.h"
#include "sphrase_manager.h"
//
#include "../r2/displayer_visual.h"
#include "../r2/displayer_visual_entity.h"
#include "../r2/displayer_visual_group.h"
#include "../r2/instance.h"
#include "../r2/tool.h"
#include "../r2/tool_pick.h"



void registerInterfaceElements()
{
	REGISTER_REFLECTABLE_CLASS(CInterfaceElement, CReflectable);
	REGISTER_REFLECTABLE_CLASS(CViewBase, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CViewText, CViewBase);
	REGISTER_REFLECTABLE_CLASS(CViewTextID, CViewText);
	REGISTER_REFLECTABLE_CLASS(CViewBitmap, CViewBase);
	REGISTER_REFLECTABLE_CLASS(CViewRadar, CViewBase);
	REGISTER_REFLECTABLE_CLASS(CViewTextMenu, CViewText);
	REGISTER_REFLECTABLE_CLASS(CDBViewBar, CViewBitmap);
	REGISTER_REFLECTABLE_CLASS(CDBViewBar3, CViewBitmap);
	REGISTER_REFLECTABLE_CLASS(CCtrlBase, CViewBase);
	REGISTER_REFLECTABLE_CLASS(CCtrlBaseButton, CCtrlBase);
	REGISTER_REFLECTABLE_CLASS(CCtrlButton, CCtrlBaseButton);
	REGISTER_REFLECTABLE_CLASS(CCtrlTextButton, CCtrlBaseButton);
	REGISTER_REFLECTABLE_CLASS(CCtrlColPick, CCtrlBase);
	REGISTER_REFLECTABLE_CLASS(CDBCtrlSheet, CCtrlBase);
	REGISTER_REFLECTABLE_CLASS(CInterfaceGroup, CCtrlBase);
	REGISTER_REFLECTABLE_CLASS(CGroupFrame, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CGroupModal, CGroupFrame);
	REGISTER_REFLECTABLE_CLASS(CGroupContainer, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CDBGroupSelectNumber, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(IListSheetBase, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CGroupEditBox, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CGroupTree, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CDBGroupComboBox, CInterfaceGroup)
	REGISTER_REFLECTABLE_CLASS(CCtrlScroll, CCtrlBase);
	REGISTER_REFLECTABLE_CLASS(CGroupMenu, CGroupModal)
	REGISTER_REFLECTABLE_CLASS(CGroupSubMenu, CGroupFrame)
	REGISTER_REFLECTABLE_CLASS(CGroupTab, CInterfaceGroup)
	REGISTER_REFLECTABLE_CLASS(CGroupScrollText, CInterfaceGroup)
	REGISTER_REFLECTABLE_CLASS(CGroupHTML, CGroupScrollText)


	REGISTER_REFLECTABLE_CLASS(CInterface3DScene, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CInterface3DCharacter, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DIG, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DShape, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DCamera, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DLight, CInterfaceElement);
	REGISTER_REFLECTABLE_CLASS(CInterface3DFX, CInterfaceElement);

	REGISTER_REFLECTABLE_CLASS(CGroupTree::SNode, CReflectable);
	REGISTER_REFLECTABLE_CLASS(CGroupList, CInterfaceGroup);
	REGISTER_REFLECTABLE_CLASS(CGroupHeader, CGroupList);


	REGISTER_REFLECTABLE_CLASS(R2::CInstance, CReflectable);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerBase, CReflectable);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerVisual, R2::CDisplayerBase);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerVisualEntity, R2::CDisplayerVisual);
	REGISTER_REFLECTABLE_CLASS(R2::CDisplayerVisualGroup, R2::CDisplayerVisual);
	REGISTER_REFLECTABLE_CLASS(R2::CTool, CReflectable);
	REGISTER_REFLECTABLE_CLASS(R2::CToolPick, R2::CTool);

	REGISTER_REFLECTABLE_CLASS(CSPhraseComAdpater, CReflectable);	

}



