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



#ifndef CL_RADIO_BUTTON_H
#define CL_RADIO_BUTTON_H

//Misc
#include "nel/misc/types_nl.h"

//Client
#include "button.h"


// forward declaration of the CRadioController class (see radio_controler.h)
class CRadioController;

/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CRadioButton : public CButton
{
public:

	/// default constructor
	CRadioButton(uint id);

	/// constructors
	explicit CRadioButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, const CButtonBase &buttonBase);
	explicit CRadioButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, CRGBA on, CRGBA off, CRGBA disable);


	/// the click method, does nothing if linked to a radio controller, calls radioClick otherwise
	virtual void click(float x, float y, bool &taken);

	/**
	* method called by the radio controller of the radio button,, do like the click method for regular buttons
	* \param float x : the x position of the mouse
	* \param float y : the y position of the mouse
	* \param bool& taken : boolean indicating if the click has be taken by another control before or not
	*/
	void radioClick(float x, float y, bool &taken);

	/**
	* select this radio button (call unselectAll on the radio controller before calling the CButton::select method)
	*/
	virtual void select();

	/**
	* set the radio controller which controls this radio button
	* \param pointer on the CRadioController
	*/
	void setController(CRadioController *controller);


// attributes
private:
	/// the radio controller of this radio button
	CRadioController	*_Controller;
};


#endif // CL_RADIO_BUTTON_H

/* End of radio_button.h */
