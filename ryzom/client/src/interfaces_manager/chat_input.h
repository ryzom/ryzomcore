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



#ifndef NL_CHAT_INPUT_H
#define NL_CHAT_INPUT_H

#include "nel/misc/types_nl.h"
#include "capture.h"


/**
 * CChatInput class which manage the text input for chat boxes
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CChatInput : public CCapture
{
public:
	/// default Constructor
	CChatInput(uint id);

	///Constructor
	CChatInput(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, const CPen &pen);
	CChatInput(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, uint32 fontSize, CRGBA color, bool shadow);

	/// Destructor.
	~CChatInput();

	/// do like a 'RETURN' keypressed : force the analysis of the line and send the message/execute the command
	void execute();

private:
	/// Initialize the control (1 function called for all constructors -> easier).
	inline void init();
	/// callback
	virtual void operator () (const CEvent& event);
};


#endif // NL_CHAT_INPUT_H

/* End of chat_input.h */
