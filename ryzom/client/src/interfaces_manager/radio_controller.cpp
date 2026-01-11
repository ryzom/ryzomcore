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
// Misc.
// Net.
// 3D Interface.
#include "nel/3d/u_driver.h"
// Client.
#include "radio_controller.h"


//-----------------------------------------------
// CRadioController :
// Constructor.
//-----------------------------------------------
CRadioController::CRadioController(uint id)
: CControl(id)
{
//	_Selected = -1;
}// CRadioController //


//-----------------------------------------------
// ~CRadioController :
// Destructor.
//-----------------------------------------------
CRadioController::~CRadioController()
{
	_Buttons.clear();
}// ~CRadioController //

//-----------------------------------------------
// add :
// Add a button to the group.
// Return true or false if the button do not have been inserted.
//-----------------------------------------------
bool CRadioController::add(CRadioButton *button)
{
	// Test if the pointer is allocated.
	if(button)
	{
		nlassert(button);

		_Buttons.push_back(button);
		button->setController( this );

		return true;
	}
	else
	{
		return false;
	}
}// add //

//-----------------------------------------------
// select :
// Select a button.
//-----------------------------------------------
void CRadioController::select(uint s)
{
	//nlassert( s < _Buttons.size() );
	if (s >= _Buttons.size() )
	{
		nlwarning("<CRadioController::select> : trying to select a button out of range");
		return;
	}

	_Buttons[s]->select();
}// select //


//-----------------------------------------------
// click :
// Manage the click of the mouse for the Buttons.
//-----------------------------------------------
void CRadioController::click(float x, float y, bool &taken)
{

	TVectButtons::iterator it;
	const TVectButtons::iterator itEnd = _Buttons.end();

	for (it = _Buttons.begin() ; it != itEnd ; ++it)
	{
		(*it)->radioClick(x, y, taken );
	}
}// click //



//-----------------------------------------------
// unselectAll :
//-----------------------------------------------
void CRadioController::unselectAll()
{
	TVectButtons::iterator it;
	const TVectButtons::iterator itEnd = _Buttons.end();

	for (it = _Buttons.begin() ; it != itEnd ; ++it)
	{
		(*it)->unSelect();
	}
}// unselectAll //