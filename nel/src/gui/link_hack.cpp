// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2013-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/dbview_bar3.h"
#include "nel/gui/dbview_number.h"
#include "nel/gui/dbview_quantity.h"
#include "nel/gui/view_pointer.h"
#include "nel/gui/group_editbox_decor.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	void ifexprufct_forcelink();
	void force_link_dbgroup_select_number_cpp();
	void force_link_dbgroup_combo_box_cpp();
	void force_link_group_wheel_cpp();

	/// Necessary so the linker doesn't drop the code of these classes from the library
	void LinkHack()
	{
		CDBViewBar3::forceLink();
		CDBViewNumber::forceLink();
		CDBViewQuantity::forceLink();
		CViewPointer::forceLink();
		ifexprufct_forcelink();
		force_link_dbgroup_select_number_cpp();
		force_link_dbgroup_combo_box_cpp();
		force_link_group_wheel_cpp();
		CGroupEditBoxDecor::forceLink();
	}
}