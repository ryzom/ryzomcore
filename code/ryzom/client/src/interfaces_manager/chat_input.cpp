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

//////////////
// Includes //
//////////////
// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
// Client
#include "client_cfg.h"
#include "chat_input.h"
#include "chat_control.h"
#include "interfaces_manager.h"
// Misc.
#include "nel/misc/command.h"

////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


/////////////
// Externs //
/////////////
extern UDriver		   *Driver;


//-----------------------------------------------
// CChatInput :
// Constructor.
//-----------------------------------------------
CChatInput::CChatInput(uint id)
: CCapture(id)
{
	init();
}// CChatInput //

//-----------------------------------------------
// CChatInput :
// Constructor.
//-----------------------------------------------
CChatInput::CChatInput(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, const CPen &pen)
: CCapture(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, numFunc, pen)
{
	init();
}// CChatInput //

//-----------------------------------------------
// CChatInput :
// Constructor.
//-----------------------------------------------
CChatInput::CChatInput(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, uint32 fontSize, CRGBA color, bool shadow)
: CCapture(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, numFunc, fontSize, color, shadow)
{
	init();
}// CChatInput //

//-----------------------------------------------
// ~CChatInput :
// Destructor.
//-----------------------------------------------
CChatInput::~CChatInput()
{
	// Remove the listener.
	Driver->EventServer.removeListener(EventCharId, this);
}// ~CChatInput //

//-----------------------------------------------
// init :
// Initialize the button (1 function called for all constructors -> easier).
//-----------------------------------------------
void CChatInput::init()
{
	_Prompt		= "Say : ";
}// init //


//---------------------------------------------------
// operator() :
// Function that receives events.
//---------------------------------------------------
void CChatInput::operator()(const CEvent& event)
{
	CEventChar &ec = (CEventChar&)event;
	switch(ec.Char)
	{
	// RETURN : Send the chat message
	case KeyRETURN:
		if(insert())
			execute();
		else
			insert(true);
		break;

	// BACKSPACE
	case KeyBACK:
		if(insert())
		{
			if(_Str.size()!= 0)
			{
				_Str.erase( _Str.end()-1 );
			}
		}
		break;

	// TAB
	case KeyTAB:
		if(insert())
		{
			if(!_Str.empty() && _Str[0] == '/')
			{
				string command = _Str.toString().substr(1);
				ICommand::expand(command);
				_Str = '/' + command;
			}
		}
		break;

	// ESCAPE
	case KeyESCAPE:
		if(insert())
		{
			_Str.clear();
			insert(false);
		}
		break;

	// OTHER
	default:
		if(insert())
			if(_Str.size() < _MaxChar)
				_Str += ec.Char;
		break;
	}
}// operator() //


//---------------------------------------------------
// execute:
//---------------------------------------------------
void CChatInput::execute()
{

	CInterfMngr::runFuncCtrl(_NumFunc, id());
	_Str.clear();
	_Insert = false;
}// execute //
