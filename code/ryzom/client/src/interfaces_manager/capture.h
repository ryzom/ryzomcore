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



#ifndef CL_CAPTURE_H
#define CL_CAPTURE_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/ucstring.h"
// Client.
#include "control.h"
#include "pen.h"


///////////
// Using //
///////////
using NLMISC::IEventListener;
using NLMISC::CEvent;


/**
 * Class to manage the keyboard capture.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CCapture : public CControl, public CPen, public IEventListener
{
private:
	/// Initialize the button (1 function called for all constructors -> easier).
	inline void init(uint numFunc);
	/// callback
	virtual void operator () (const CEvent& event);

protected:
	uint		_NumFunc;
	/// the max number of char on a line
	uint		_MaxChar;
	/// the memorized line
	ucstring	_Str;
	/// the prompt (displayed at the beginning of the control, before the edited line)
	ucstring	_Prompt;
	/// the mode, if insert==true, the line is being edited
	bool		_Insert;

public:
	/// Constructor.
	CCapture(uint id);
	CCapture(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, const CPen &pen);
	CCapture(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, uint32 fontSize, CRGBA color, bool shadow);

	/// Destructor.
	~CCapture();

	/// Display the Bitmap.
	virtual void display();
	/// Manage the click of the mouse for the Bitmap.
	virtual void click(float x, float y, bool &taken);


	/// set the string contained in the class : _Str.
	void setStr(const ucstring &str)
	{
		_Str = str;
	}

	/// Return the string contained in the class : _Str.
	const ucstring &getStr() const
	{
		return _Str;
	}

	void setPrompt(const ucstring &str)
	{
		_Prompt = str;
	}

	const ucstring &getPrompt() const { return _Prompt; }

	/// Set if the Control is in Insert mode.
	inline void insert(bool i) {_Insert = i;}
	/// Return if the control is in insert mode.
	inline bool insert() const {return _Insert;}
};


#endif // CL_CAPTURE_H

/* End of capture.h */
