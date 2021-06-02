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



#ifndef CL_BUTTON_H
#define CL_BUTTON_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"
// Client.
#include "control.h"
#include "button_base.h"
#include "pen.h"
#include <string>


///////////
// Using //
///////////
using std::string;


/**
 * Manages "Button" control in Interface.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CButton : public CControl, public CButtonBase
{
public:
	enum TState
	{
		released,		// unselected
		left_clicked,	// selected with a single click with the left mouse button
		right_clicked,  // selected with a single click with the right mouse button
		double_clicked,	// selected with a double click with the left mouse button
	};

private:
	/// Initialize the button (1 function called for all constructors -> easier).
	inline void init(uint numFuncOn, uint numFuncR, uint numFuncD);

protected:
	/// Text of the button.
	ucstring	_Text;
	/// Pen used to display the text.
	CPen		_Pen;

	/// function called with a left click on the button
	uint		_NumFuncOn;

	/// function called with a right click on the button
	uint		_NumFuncRightClick;

	/// function called with a double (left) click on the button
	uint		_NumFuncDbleClick;

	/// state of the button
	TState		_State;

public:
	/// Constructor
	CButton(uint id = 0);
	explicit CButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, const CButtonBase &buttonBase);
	explicit CButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, CRGBA on, CRGBA off, CRGBA disable);

	/// Set the Pen to use for the Text of the button.
	void pen(const CPen &pen);

	/// unSelect the button.
	virtual void unSelect();

	/// Display the Bitmap.
	virtual void display();

	/// Manage the click of the mouse for the Button.
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click of the mouse for the Button.
	virtual void clickRight(float x, float y, bool &taken);

	/// get the state of the button (clicked, released (unselected), etc...)
	TState	getState() const { return _State; }

	/**
	 * set the text displayed in the button.
	 * \param text : the new text to display.
	 */
	void text(const ucstring &text) { _Text = text; }

	/**
	 * get the text displayed in the button
	 * \return cont ucstring& : the text displayed by the button.
	 */
	const ucstring &text() const { return _Text; }
};


#endif // CL_BUTTON_H

/* End of button.h */
