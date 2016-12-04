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
#include "nel/gui/reflect_register.h"

#include "nel/gui/interface_element.h"
#include "nel/gui/view_base.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/group_submenu_base.h"
#include "nel/gui/group_menu.h"
#include "nel/gui/ctrl_base.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_frame.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_list.h"
#include "nel/gui/dbgroup_select_number.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/ctrl_col_pick.h"
#include "nel/gui/ctrl_draggable.h"
#include "nel/gui/group_editbox_base.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_tree.h"
#include "nel/gui/reflect.h"
#include "nel/gui/dbview_bar.h"
#include "nel/gui/dbview_bar3.h"
#include "nel/gui/ctrl_scroll_base.h"
#include "nel/gui/ctrl_scroll.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/group_tab.h"
#include "nel/gui/group_html.h"
#include "nel/gui/group_header.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	void CReflectableRegister::registerClasses()
	{
		REGISTER_REFLECTABLE_CLASS(CInterfaceElement, CReflectable);
		REGISTER_REFLECTABLE_CLASS(CViewBase, CInterfaceElement);
		REGISTER_REFLECTABLE_CLASS(CViewText, CViewBase);
		REGISTER_REFLECTABLE_CLASS(CViewTextID, CViewText);
		REGISTER_REFLECTABLE_CLASS(CViewBitmap, CViewBase);
		REGISTER_REFLECTABLE_CLASS(CViewTextMenu, CViewText);
		REGISTER_REFLECTABLE_CLASS(CDBViewBar, CViewBitmap);
		REGISTER_REFLECTABLE_CLASS(CDBViewBar3, CViewBitmap);
		REGISTER_REFLECTABLE_CLASS(CCtrlBase, CViewBase);
		REGISTER_REFLECTABLE_CLASS(CCtrlBaseButton, CCtrlBase);
		REGISTER_REFLECTABLE_CLASS(CCtrlButton, CCtrlBaseButton);
		REGISTER_REFLECTABLE_CLASS(CCtrlTextButton, CCtrlBaseButton);
		REGISTER_REFLECTABLE_CLASS(CCtrlColPick, CCtrlBase);
		REGISTER_REFLECTABLE_CLASS(CCtrlDraggable, CCtrlBase);
		REGISTER_REFLECTABLE_CLASS(CInterfaceGroup, CCtrlBase);
		REGISTER_REFLECTABLE_CLASS(CGroupFrame, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CGroupModal, CGroupFrame);
		REGISTER_REFLECTABLE_CLASS(CGroupContainerBase, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CGroupContainer, CGroupContainerBase);
		REGISTER_REFLECTABLE_CLASS(CDBGroupSelectNumber, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CGroupEditBoxBase, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CGroupEditBox, CGroupEditBoxBase);
		REGISTER_REFLECTABLE_CLASS(CGroupTree, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CDBGroupComboBox, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CCtrlScrollBase, CCtrlBase);
		REGISTER_REFLECTABLE_CLASS(CCtrlScroll, CCtrlScrollBase);
		REGISTER_REFLECTABLE_CLASS(CGroupMenu, CGroupModal);
		REGISTER_REFLECTABLE_CLASS(CGroupSubMenuBase, CGroupFrame);
		REGISTER_REFLECTABLE_CLASS(CGroupSubMenu, CGroupSubMenuBase);
		REGISTER_REFLECTABLE_CLASS(CGroupTab, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CGroupScrollText, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CGroupHTML, CGroupScrollText);
		REGISTER_REFLECTABLE_CLASS(CGroupTree::SNode, CReflectable);
		REGISTER_REFLECTABLE_CLASS(CGroupList, CInterfaceGroup);
		REGISTER_REFLECTABLE_CLASS(CGroupHeader, CGroupList);
	}
}


