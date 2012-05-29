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

#include "group_modal.h"
#include "interface_manager.h"
#include "interface_element.h"
#include "game_share/xml_auto_ptr.h"

using namespace std;

#include "nel/misc/i_xml.h"

NLMISC_REGISTER_OBJECT(CViewBase, CGroupModal, std::string, "modal");

// ***************************************************************************
CGroupModal::CGroupModal(const TCtorParam &param)
: CGroupFrame(param)
{
	SpawnOnMousePos= true;
	ExitClickOut= true;
	ExitClickL= false;
	ExitClickR= false;
	ExitKeyPushed = false;
	ForceInsideScreen= false;
	SpawnMouseX= SpawnMouseY= 0;
	_MouseDeltaX= _MouseDeltaY= 0;
	//By default, modal are escapable
	_Escapable= true;
}


// ***************************************************************************
bool CGroupModal::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CGroupFrame::parse(cur, parentGroup))
		return false;

	// read modal option
	CXMLAutoPtr	prop;
	prop = xmlGetProp (cur, (xmlChar*)"mouse_pos");
	if ( prop )	SpawnOnMousePos= convertBool(prop);
	prop = xmlGetProp (cur, (xmlChar*)"exit_click_out");
	if ( prop )	ExitClickOut= convertBool(prop);
	prop = xmlGetProp (cur, (xmlChar*)"exit_click_l");
	if ( prop )	ExitClickL= convertBool(prop);
	prop = xmlGetProp (cur, (xmlChar*)"exit_click_r");
	if ( prop )	ExitClickR= convertBool(prop);
	prop = xmlGetProp (cur, (xmlChar*)"exit_click_b");
	if ( prop )	ExitClickR= ExitClickL= convertBool(prop);
	prop = xmlGetProp (cur, (xmlChar*)"force_inside_screen");
	if ( prop )	ForceInsideScreen= convertBool(prop);
	prop = xmlGetProp (cur, (xmlChar*)"category");
	if ( prop )	Category= (const char *) prop;
	prop = xmlGetProp (cur, (xmlChar*)"onclick_out");
	if ( prop )	OnClickOut = (const char *) prop;
	prop = xmlGetProp (cur, (xmlChar*)"onclick_out_params");
	if ( prop )	OnClickOutParams = (const char *) prop;
	prop = xmlGetProp (cur, (xmlChar*)"onpostclick_out");
	if ( prop )	OnPostClickOut = (const char *) prop;
	prop = xmlGetProp (cur, (xmlChar*)"onpostclick_out_params");
	if ( prop )	OnPostClickOutParams = (const char *) prop;
	prop = xmlGetProp (cur, (xmlChar*)"exit_key_pushed");
	if ( prop )	ExitKeyPushed= convertBool(prop);

	// Force parent hotspot for spawn on mouse
	if(SpawnOnMousePos)
		setParentPosRef(Hotspot_BL);

	// bkup x/y as the deltas.
	_MouseDeltaX= getX();
	_MouseDeltaY= getY();

	// Modals are disabled by default
	_Active = false;

	return true;
}

// ***************************************************************************
void CGroupModal::updateCoords ()
{
	// if snap to mouse pos.
	if(SpawnOnMousePos)
	{
		// Special for menu for instance: If the size is bigger or equal to the screen, keep 0, because will be clipped just after
		CViewRenderer &rVR = CInterfaceManager::getInstance()->getViewRenderer();
		uint32 w,h;
		rVR.getScreenSize(w,h);
		if(_W>=(sint32)w && _H>=(sint32)h)
		{
			_X= _Y= 0;
		}
		else
		{
			// change coords
			_X= SpawnMouseX+_MouseDeltaX;
			_Y= SpawnMouseY+_MouseDeltaY;
		}
	}

	// update group
	CGroupFrame::updateCoords();

	// if snap to mouse pos or ForceInsideScreen
	if(SpawnOnMousePos || ForceInsideScreen)
	{
		bool clipped = false;
		// repos the group according to real size. clip against screen
		if(_XReal<0)
		{
			_X+= -_XReal;
			clipped = true;
		}
		else
		{
			if (!SpawnOnMousePos)
				_X = _MouseDeltaX;
		}

		if(_XReal+_WReal>_Parent->getW())
		{
			_X-= _XReal+_WReal-_Parent->getW();
			clipped =true;
		}
		else
		{
			if ((!SpawnOnMousePos) && (_XReal>=0))
				_X = _MouseDeltaX;
		}

		if(_YReal<0)
		{
			_Y+= -_YReal;
			clipped =true;
		}
		else
		{
			if (!SpawnOnMousePos)
				_Y = _MouseDeltaY;
		}

		if(_YReal+_HReal>_Parent->getH())
		{
			_Y-= _YReal+_HReal-_Parent->getH();
			clipped =true;
		}
		else
		{
			if ((!SpawnOnMousePos) && (_YReal>=0))
				_Y = _MouseDeltaY;
		}

		if (clipped)
		{
			CGroupFrame::updateCoords();
		}
	}
}


