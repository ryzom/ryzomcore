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



#ifndef CL_BRICK_CONTROL_H
#define CL_BRICK_CONTROL_H

// Misc
#include "nel/misc/types_nl.h"
// Game_share

// Client
#include "control.h"
#include "bitmap_base.h"
#include "base_spell_client.h"



class CBrickControl;


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CBrickControl : public CControl
{
public:

	/// default Constructor
	CBrickControl(uint id = 0);

	/// Constructor
	CBrickControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn = 0,  uint numFuncR = 0);

	// copy constructor
//	CBrickControl( const CBrickControl &b);

	/// Destructor
	~CBrickControl()
	{}

	/**
	 * get the brick associated to this control
	 * \return CBrickClient* the brick or NULL if no brick is associated with this control
	 */
	inline CBrickClient *getAssociatedBrick() { return _AssociatedBrick; }

	/**
	 * set the brick associated to this control
	 * \param CBrickClient* brick the brick associated to this control
	 */
	inline void setAssociatedBrick(CBrickClient *brick)
	{
		//nlassert( brick) ;
		if ( !brick )
		{
			nlwarning("<CBrickClient::setAssociatedBrick> : param in NULL");
			return;
		}
		_AssociatedBrick = brick;
	}

	/// display the control
	virtual void display();

	/**
	 * set the color of the control
	 * \param CRGBA &color new color of the control
	 */
	inline void rgba( const CRGBA &color) { _RGBA = color; }

	/**
	 * get the color of the control
	 * \return CRGBA& color of the control
	 */
	inline const CRGBA &rgba() const { return _RGBA; }

	/// Manage the click for the control.
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click for the control.
	virtual void clickRight(float x, float y, bool &taken);


private:
	/**
	 * init the control
	 */
	void init(uint numFuncOn = 0,  uint numFuncR = 0);

private:
	/// the brick associated to this control
	CBrickClient *_AssociatedBrick;

	// color applied on the texture of the brick, default = 255 255 255 255
	CRGBA		_RGBA;

	/// function called with a left click on the control
	uint		_NumFuncOn;
	/// function called with a right click on the control
	uint		_NumFuncR;

};


#endif // CL_BRICK_CONTROL_H

/* End of brick_control.h */
