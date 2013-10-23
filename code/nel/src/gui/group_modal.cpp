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
#include "nel/gui/group_modal.h"
#include "nel/gui/interface_element.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/view_renderer.h"
#include "nel/misc/i_xml.h"

using namespace std;

namespace NLGUI
{

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


	std::string CGroupModal::getProperty( const std::string &name ) const
	{
		if( name == "mouse_pos" )
		{
			return NLMISC::toString( SpawnOnMousePos );
		}
		else
		if( name == "exit_click_out" )
		{
			return NLMISC::toString( ExitClickOut );
		}
		else
		if( name == "exit_click_l" )
		{
			return NLMISC::toString( ExitClickL );
		}
		else
		if( name == "exit_click_r" )
		{
			return NLMISC::toString( ExitClickR );
		}
		else
		if( name == "exit_click_b" )
		{
			if( ExitClickL == ExitClickR )
				return NLMISC::toString( ExitClickL );
			else
				return "false";
		}
		else
		if( name == "force_inside_screen" )
		{
			return NLMISC::toString( ForceInsideScreen );
		}
		else
		if( name == "category" )
		{
			return Category;
		}
		else
		if( name == "onclick_out" )
		{
			return OnClickOut;
		}
		else
		if( name == "onclick_out_params" )
		{
			return OnClickOutParams;
		}
		else
		if( name == "onpostclick_out" )
		{
			return OnPostClickOut;
		}
		else
		if( name == "onpostclick_out_params" )
		{
			return OnPostClickOutParams;
		}
		else
		if( name == "exit_key_pushed" )
		{
			return NLMISC::toString( ExitKeyPushed );
		}
		else
			return CGroupFrame::getProperty( name );
	}

	void CGroupModal::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "mouse_pos" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				SpawnOnMousePos = b;
			return;
		}
		else
		if( name == "exit_click_out" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				ExitClickOut = b;
			return;
		}
		else
		if( name == "exit_click_l" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				ExitClickL = b;
			return;
		}
		else
		if( name == "exit_click_r" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				ExitClickR = b;
			return;
		}
		else
		if( name == "exit_click_b" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
			{
				ExitClickL = ExitClickR = b;
			}
			return;
		}
		else
		if( name == "force_inside_screen" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				ForceInsideScreen = b;
			return;
		}
		else
		if( name == "category" )
		{
			Category = value;
			return;
		}
		else
		if( name == "onclick_out" )
		{
			OnClickOut = value;
			return;
		}
		else
		if( name == "onclick_out_params" )
		{
			OnClickOutParams = value;
			return;
		}
		else
		if( name == "onpostclick_out" )
		{
			OnPostClickOut = value;
			return;
		}
		else
		if( name == "onpostclick_out_params" )
		{
			OnPostClickOutParams = value;
			return;
		}
		else
		if( name == "exit_key_pushed" )
		{
			bool b;
			if( NLMISC::fromString( value, b ) )
				ExitKeyPushed = b;
			return;
		}
		else
			CGroupFrame::setProperty( name, value );
	}



	xmlNodePtr CGroupModal::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CGroupFrame::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "modal" );
		xmlSetProp( node, BAD_CAST "mouse_pos", BAD_CAST NLMISC::toString( SpawnOnMousePos ).c_str() );
		xmlSetProp( node, BAD_CAST "exit_click_out", BAD_CAST NLMISC::toString( ExitClickOut ).c_str() );
		xmlSetProp( node, BAD_CAST "exit_click_l", BAD_CAST NLMISC::toString( ExitClickL ).c_str() );
		xmlSetProp( node, BAD_CAST "exit_click_r", BAD_CAST NLMISC::toString( ExitClickR ).c_str() );
		
		if( ExitClickL == ExitClickR )
			xmlSetProp( node, BAD_CAST "exit_click_b", BAD_CAST NLMISC::toString( ExitClickL ).c_str() );

		xmlSetProp( node, BAD_CAST "force_inside_screen", BAD_CAST NLMISC::toString( ForceInsideScreen ).c_str() );
		xmlSetProp( node, BAD_CAST "category", BAD_CAST Category.c_str() );
		xmlSetProp( node, BAD_CAST "onclick_out", BAD_CAST OnClickOut.c_str() );
		xmlSetProp( node, BAD_CAST "onclick_out_params", BAD_CAST OnClickOutParams.c_str() );
		xmlSetProp( node, BAD_CAST "onpostclick_out", BAD_CAST OnPostClickOut.c_str() );
		xmlSetProp( node, BAD_CAST "onpostclick_out_params", BAD_CAST OnPostClickOutParams.c_str() );
		xmlSetProp( node, BAD_CAST "exit_key_pushed", BAD_CAST NLMISC::toString( ExitKeyPushed ).c_str() );

		return node;
	}

	// ***************************************************************************
	bool CGroupModal::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if(!CGroupFrame::parse(cur, parentGroup))
			return false;

		// read modal option
		CXMLAutoPtr	prop;
		prop = xmlGetProp (cur, (xmlChar*)"mouse_pos");
		if (prop)	SpawnOnMousePos= convertBool(prop);
		prop = xmlGetProp (cur, (xmlChar*)"exit_click_out");
		if (prop)	ExitClickOut= convertBool(prop);
		prop = xmlGetProp (cur, (xmlChar*)"exit_click_l");
		if (prop)	ExitClickL= convertBool(prop);
		prop = xmlGetProp (cur, (xmlChar*)"exit_click_r");
		if (prop)	ExitClickR= convertBool(prop);
		prop = xmlGetProp (cur, (xmlChar*)"exit_click_b");
		if (prop)	ExitClickR= ExitClickL= convertBool(prop);
		prop = xmlGetProp (cur, (xmlChar*)"force_inside_screen");
		if (prop)	ForceInsideScreen= convertBool(prop);
		prop = xmlGetProp (cur, (xmlChar*)"category");
		if (prop)	Category= (const char *) prop;
		prop = xmlGetProp (cur, (xmlChar*)"onclick_out");
		if (prop)	OnClickOut = (const char *) prop;
		prop = xmlGetProp (cur, (xmlChar*)"onclick_out_params");
		if (prop)	OnClickOutParams = (const char *) prop;
		prop = xmlGetProp (cur, (xmlChar*)"onpostclick_out");
		if (prop)	OnPostClickOut = (const char *) prop;
		prop = xmlGetProp (cur, (xmlChar*)"onpostclick_out_params");
		if (prop)	OnPostClickOutParams = (const char *) prop;
		prop = xmlGetProp (cur, (xmlChar*)"exit_key_pushed");
		if (prop)	ExitKeyPushed= convertBool(prop);

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
			CViewRenderer &rVR = *CViewRenderer::getInstance();
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

}

