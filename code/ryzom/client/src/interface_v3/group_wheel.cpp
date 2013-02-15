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



// ----------------------------------------------------------------------------
#include "stdpch.h"
//
#include "group_wheel.h"
#include "interface_manager.h"



NLMISC_REGISTER_OBJECT(CViewBase, CInterfaceGroupWheel, std::string, "group_wheel");

// *****************************************************************************************************************
CInterfaceGroupWheel::CInterfaceGroupWheel(const TCtorParam &param) : CInterfaceGroup(param)
{
	_AHWheelUp = NULL;
	_AHWheelDown = NULL;
}

// *****************************************************************************************************************
bool CInterfaceGroupWheel::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	if (!CInterfaceGroup::parse(cur, parentGroup)) return false;
	parseAH(cur, "on_wheel_up", "on_wheel_up_params", _AHWheelUp, _AHWheelUpParams);
	parseAH(cur, "on_wheel_down", "on_wheel_down_params", _AHWheelDown, _AHWheelDownParams);
	return true;
}

// *****************************************************************************************************************
bool CInterfaceGroupWheel::handleEvent(const CEventDescriptor &event)
{
	if (CInterfaceGroup::handleEvent(event)) return true;
	if (event.getType() == CEventDescriptor::mouse)
	{
		const CEventDescriptorMouse &eventDesc = (const CEventDescriptorMouse &)event;
		if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mousewheel)
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			if (eventDesc.getWheel() > 0 && _AHWheelUp)
			{
				im->runActionHandler (_AHWheelUp, this, _AHWheelUpParams);
				return true;
			}
			else if (_AHWheelDown)
			{
				im->runActionHandler (_AHWheelDown, this, _AHWheelDownParams);
				return true;
			}
		}
	}
	return false;
}

