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



#ifndef CL_RADIO_CONTROLLER_H
#define CL_RADIO_CONTROLLER_H

#include "nel/misc/types_nl.h"
// Client.
#include "control.h"
#include "radio_button.h"
#include <vector>

using std::vector;


/**
 * CRadioController : manage several CRadioButtons objects, ensuring only one is selected at a time
 * \author Fleury David
 * \author Nevrax France
 * \date 2001
 */
class CRadioController : public CControl
{
	typedef vector<CRadioButton *> TVectButtons;
	/// the radio buttons controled by this radio controller
	TVectButtons	_Buttons;

public:
	/// Constructor.
	CRadioController(uint id);
	/// Destructor.
	~CRadioController();

	/// Do nothing
	virtual void display() {}

	/// Manage the click of the mouse for the control
	virtual void click(float x, float y, bool &taken);

	/// does nothing
	virtual void resize(uint32 width, uint32 height) {}
	/// does nothing
	virtual void ref(float x, float y, float width, float height) {}

	/// Add a button to the group; Return true if the button have been inserted.
	bool add(CRadioButton *button);
	/// Select the button corresponding to the id.
	void select(uint id);
	/// Return the number of buttons.
	uint size() {return (uint)_Buttons.size();}

	/**
	* unselect all the radio buttons
	*/
	void unselectAll();
};


#endif // CL_RADIO_CONTROLLER_H

/* End of radio_button.h */
