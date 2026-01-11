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



#ifndef NL_BRICK_RECEPTACLE_H
#define NL_BRICK_RECEPTACLE_H


/////////////
// Include //
/////////////
// Misc
#include "nel/misc/types_nl.h"
// Client
#include "control.h"
#include "bitmap_base.h"
#include "interfaces_manager.h"
#include "brick_control.h"
// Interface 3D
#include "nel/3d/u_driver.h"


////////////
// Extern //
////////////
extern NL3D::UDriver *Driver;

/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CBrickReceptacle : public CControl, public CBitmapBase
{
public:

	/**
	 * constructor
	 * \param uint id the id of the control (default = 0)
	 */
	CBrickReceptacle(uint id = 0);

	/**
	 * constructor
	 */
	CBrickReceptacle(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint texture, const CRGBA &rgba, uint16 family, uint numFunc = 0);

	/// destructor
	~CBrickReceptacle();

	/**
	 * set the brick this control "contains", only usable on a unlocked object (_Locked == false)
	 * \param CBrickControl* brick the brick that will be in this receptacle
	 * \return bool true if the brick is compatible, false otherwise
	 */
	bool brick( CBrickControl *brick);

	/**
	 * get the brick if any
	 * \return CBrickControl * the brick or NULL if receptacle is empty
	 */
	CBrickControl *brick() { return _Brick; }

	/**
	 * set the brick family accepted by this control
	 * \param uint16 family the new family accepted
	 */
	bool family( uint16 family);


	/**
	 * get the brick family this control can accept
	 * \return uint16 the family this control can accept
	 */
	uint16 family() const { return _Family; }

	/**
	 * test if the receptacle is empty (same as (brick() == NULL) )
	 * \return bool true if empty, false if full
	 */
	bool isEmpty() const { return ( _Brick == NULL); }

	/**
	 * emty the slot, delete _Brick if allocated
	 */
	void clear();

	/// Display the Bitmap.
	virtual void display();

	/// Set some references for the display.
	virtual void ref(float x, float y, float w, float h);


	/// Manage the click for the control.
	virtual void click(float x, float y, bool &taken);


	/// lock the control (the brick cannot be changed/removed/added )
	inline void lock() { _Locked = true; }

	/// unlock the control
	inline void unlock() { _Locked = false; }

	/// return the locked state of the control
	inline bool isLocked() const { return _Locked; }

private:
	/// init the control
	void init( uint16 family=0, uint numFunc = 0);

private:
	/// the brick contained by this control, or NULL if empty
	CBrickControl	*_Brick;

	/// the brick family this control can accept
	uint16			_Family;

	/// lock the associated brick control or not (if locked == true, the brick cannot be changed/removed/added )
	bool			_Locked;

	/// function called with a left click on the button
	uint		_NumFuncLeftClick;

};


#endif // NL_BRICK_RECEPTACLE_H

/* End of brick_receptacle.h */
