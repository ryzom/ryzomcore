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
#include "nel/gui/view_base.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace NLGUI
{

	CViewBase::~CViewBase()
	{
		CWidgetManager::getInstance()->removeRefOnView (this);
	}

	// ***************************************************************************
	void CViewBase::dumpSize(uint depth /*=0*/) const
	{
		std::string result;
		result.resize(depth * 4, ' ');
		std::string::size_type pos = _Id.find_last_of(':');
		std::string shortID = pos == std::string::npos ? _Id : _Id.substr(pos);
		result += NLMISC::toString("id=%s, w=%d, h=%d, x=%d, y=%d, wreal=%d, hreal=%d, xreal=%d, yreal=%d", shortID.c_str(), (int) _W, (int) _H, (int) _X, (int) _Y, (int) _WReal, (int) _HReal, (int) _XReal, (int) _YReal);
		nlinfo(result.c_str());
	}

	// ***************************************************************************
	void CViewBase::visit(CInterfaceElementVisitor *visitor)
	{
		nlassert(visitor);
		visitor->visitView(this);
		CInterfaceElement::visit(visitor);
	}

}

