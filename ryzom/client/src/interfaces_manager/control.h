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



#ifndef CL_CONTROL_H
#define CL_CONTROL_H


/////////////
// Include //
/////////////
// Misc
#include "nel/misc/types_nl.h"
// Std
#include <list>


///////////
// Using //
///////////
//using std::list;


/**
 * basis class for all controls in interface.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CControl
{
private:
	/// Initialize the control (1 function called for all constructors -> easier).
	void init(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel);

public:
	enum THotSpot
	{
		HS_TL = 0,
		HS_TM,
		HS_TR,
		HS_ML,
		HS_MM,
		HS_MR,
		HS_BL,
		HS_BM,
		HS_BR
	};

	enum TMode
	{
		Mode_0_1 = 0,
		Mode_Pixel
	};

protected:
	typedef std::list<CControl *> TListControl;
	TListControl	_Children;

	// Id of the control.
	uint		_Id;

	/// Position of the Control (between 0-1).
	float		_X;
	float		_Y;
	/// Position of the Control (in Pixel).
	float		_X_Pixel;
	float		_Y_Pixel;
	/// Display Position of the Control (between 0-1).
	float		_X_Display;
	float		_Y_Display;
	/// The control position is relative to this Reference.
	float		_X_Ref;
	float		_Y_Ref;
	/// Delta to add to the position because of the Hot Spot.
	float		_X_HotSpot;
	float		_Y_HotSpot;

	/// Width and Height (between 0-1).
	float		_W;
	float		_H;
	/// Width and Height (in pixel).
	float		_W_Pixel;
	float		_H_Pixel;
	/// Display Width and Height of the Control (between 0-1).
	float		_W_Display;
	float		_H_Display;
	/// Reference Size (the OSD size).
	float		_W_Ref;
	float		_H_Ref;

	/// Do the control have to be displayed. true -> yes.
	bool		_Show;
	/// How to display the control in relation to the position of the control.
	THotSpot	_HotSpot;
	/// Useful for the Parent to know what reference position to give to the control.
	THotSpot	_Origin;
	/// Pointer on the Parent.
	CControl	*_Parent;

	/// Calculate the Display X, Y, Width, Height.
	virtual void calculateDisplay();
	/// Calculate the display position of the control in relation to the position of the control (Hot Spot).
	virtual void calculateHS();
	/// Function to calculate where to display a child.
	void calculateOrigin(float &x, float &y, THotSpot origin);

public:
	/// Constructor
	CControl(uint id = 0);
	CControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel);
	virtual ~CControl(){}

	/// get the display values of the control
	void getDisplayValues( float &x, float &y, float &h, float &w) const;

	/// get the size of the control
	void getSize( float &w, float &h, float &wPixel, float &hPixel) const;

	/// get the position of the control
	void getPosition( float &x, float &y, float &xPixel, float &yPixel) const;

	/// set the size of the control
	void setSize( float w, float h, float wPixel, float hPixel);

	/// set the position of the control
	void setPosition( float x, float y, float xPixel, float yPixel);


	/// Display the control.
	virtual void display() = 0;

	/// Manage the click for the control.
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click for the control.
	virtual void clickRight(float x, float y, bool &taken);

	/// The window size has changed -> resize the control.
	virtual void resize(uint32 width, uint32 height);

	/// Set some references for the display.
	virtual void ref(float x, float y, float w, float h);

	/// Hide or show the control. false -> hide, true -> show.
	virtual void show(bool show);

	/// Change the Hot Spot.
	virtual void hotSpot(THotSpot hs);

	/// Set Origin.
	virtual void origin(THotSpot o) {_Origin = o;}

	/// Set the parent of the control.
	virtual void parent(CControl *p) {_Parent = p;}

	/// Return the show of the control.
	bool show();

	/// Return the Hot Spot.
	THotSpot hotSpot();

	/// Get Origin.
	THotSpot origin() {return _Origin;}

	/// Get the parent of the control
	CControl *parent() {return _Parent;}

	/// Add a child to the control.
	void addChild(CControl *ctrl);

	/**
	* called when the mouse has moved
	* \param the x coordinate of the mouse
	* \param the y coordinate of the mouse
	*/
	virtual void mouseMove( float x, float y);

	/**
	 * Get the Id of the control.
	 * \return uint : control Id.
	 */
	inline uint id() const {return _Id;}
};


#endif // CL_CONTROL_H

/* End of control.h */
