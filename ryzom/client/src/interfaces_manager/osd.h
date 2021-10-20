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



#ifndef CL_OSD_H
#define CL_OSD_H


/////////////
// Include //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/rgba.h"
// Client
#include "control.h"
#include "pen.h"
// Std
#include <map>
#include <fstream>


///////////
// Using //
///////////
using NLMISC::CRGBA;
using std::map;
using std::ifstream;


/**
 * Class to manage an OSD (On Screen Display = Kind of window) for the interfaces.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class COSD
{
public:
	enum ECursor
	{
		Cur_None = 0,
		Cur_Move,		// Move
		Cur_Resize,		// Resize
		Cur_Resize_TL,	// Resize Top Left
		Cur_Resize_T,	// Resize Top
		Cur_Resize_TR,	// Resize Top Right
		Cur_Resize_R,	// Resize Right
		Cur_Resize_BR,	// Resize Bottom Right
		Cur_Resize_B,	// Resize Bottom
		Cur_Resize_BL,	// Resize Bottom Left
		Cur_Resize_L,	// Resize Left
	};

	enum TMode
	{
		none = 0,
		selected,
		resizable,
		movable,
		locked,
		blocked
	};

	enum TResize
	{
		no_resize = 0,

		resize_B,
		resize_T,
		resize_R,
		resize_L,

		resize_BL,
		resize_TL,
		resize_BR,
		resize_TR,

		resize_move
	};

	enum TBG	// BG = Background.
	{
		BG_none = 0,	// NO BG.
		BG_plain,		// BG is only made with 1 color (RGBA).
		BG_stretch		// BG is a Bitmap and 1 color (RGBA).
	};

	enum TTB	// TB = Title Bar.
	{
		TB_none = 0,	// NO TB.
		TB_plain,		// TB is only made with 1 color (RGBA).
		TB_stretch		// TB is a Bitmap and 1 color (RGBA).
	};

private:
	typedef std::map<uint, CControl *>	TMapControls;
	typedef std::list<CControl *>		TListControl;

	/// Map to find a control with the Id.
	TMapControls	_Controls;
	/// The list of control in order to display.
	TListControl	_Children;


	/// Initialize the button (1 function called for all constructors -> easier).
	inline void init(uint id,float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, float minWidth, float minHeight, bool popUp = false);

	void getText(uint idCtrl);
	void getCapture(uint idCtrl);
	void getButton(uint idCtrl);
	void getRadioButton(uint idCtrl);
	void getRadioController(uint idCtrl);

	void getBitmap(uint idCtrl);
	void getList(uint idCtrl);
	void getMultiList(uint idCtrl);
	void getChatBox(uint idCtrl);
	void getChatInput(uint idCtrl);
	void getChoiceList(uint idCtrl);
	void getCandidateList(uint idCtrl);
	void getHorizontalList(uint idCtrl);
	void getControlList(uint idCtrl);
	void getSpellList(uint idCtrl);
	void getProgressBar(uint idCtrl);
	void getCastingBar(uint idCtrl);
	void getBrickControl(uint idCtrl);

	/// Resize the Left border of the OSD.
	void resizeL(float x, float y);
	/// Resize the Right border of the OSD.
	void resizeR(float x, float y);
	/// Resize the Bottom border of the OSD.
	void resizeB(float x, float y);
	/// Resize the Top border of the OSD.
	void resizeT(float x, float y);
	/// Move the OSD.
	void move(float x, float y);

	/// Function to test if a coordinate is in the rect.
	bool testInRect(float x, float y, float rectX0, float rectY0, float rectX1, float rectY1);

	/// Calculate a position according to the origin.
	void calculatePos(float &x, float &y, CControl::THotSpot origin);

	/// Calculate the Display X, Y, Width, Height.
	void calculateDisplay();

	/// Function to draw the background according to the mode.
	void drawBG();
	/// Function to draw the Title Bar according to the mode.
	void drawTB();
	/// Draw the resize borders.
	void drawBorders(float bSizeW, float bSizeH, float x0, float y0, float x1, float y1, const CRGBA &color);
	/// Draw a text with all information needed.
	void drawText(float x, float y, const ucstring &text, const CPen &pen);

	/// Test the mode of the OSD.
	void testMode(float x, float y, float rectX0, float rectY0, float rectX1, float rectY1);
	/// Determine the Resize Mode.
	bool resizeMode(float x, float y);

protected:
	/// the 'ID' fo the OSD (as defined for the interface manager)
	uint	_Id;

	/// boolean indicating if the OSD is a 'pop-up' window, in this case, a click outside the window close that window
	bool	_PopUp;

	/// Position of the OSD (between 0-1).
	float	_X;
	float	_Y;
	/// Position of the OSD (in Pixel).
	float	_X_Pixel;
	float	_Y_Pixel;

	/// Minimum values for window size
	float	_W_Min;
	float	_H_Min;


	/// Display Position of the Control (between 0-1).
	float	_X_Display;
	float	_Y_Display;

	/// Width and Height (between 0-1).
	float	_W;
	float	_H;
	/// Width and Height (in pixel).
	float	_W_Pixel;
	float	_H_Pixel;
	/// Display Width and Height of the Control (between 0-1).
	float	_W_Display;
	float	_H_Display;

	/// Variables to know the offset between the click position and the OSD borders.
	float	_OffsetX;
	float	_OffsetY;
	float	_OffsetW;
	float	_OffsetH;

	/// Is the OSD Visible.
	bool	_Show;

	/** \name OSD
	 * Variables to manage the OSD.
	 */
	//@{
	/// How to display the OSD.
	TMode		_OSD_Mode;
	/// Name of the OSD.
	ucstring	_OSD_Name;
	/// Function called each update (every frame in most case).
	uint		_OSD_Update_Func;
	//@}

	/** \name Background
	 * Variables to manage the background of the OSD.
	 */
	//@{
	/// Display mode for the background.
	TBG		_BG_Mode;
	/// Id of the texture for the background.
	uint	_BG;
	/// Color of the Background.
	CRGBA	_BG_Color;
	//@}

	/** \name Title Bar
	 * Variables to manage the Title Bar of the OSD.
	 */
	//@{
	/// Display mode for the Title Bar.
	TTB		_TB_Mode;
	/// Id of the texture for the Title BAr.
	uint	_TB;
	/// Color of the Title Bar.
	CRGBA	_TB_Color;
	/// Height of the Title Bar (in Pixel).
	float	_TB_H;
	/// Display Height of the Title Bar.
	float	_TB_H_Display;
	/// Pen of the Title Bar.
	CPen	_TB_Pen;
	//@}

	/** \name HighLight
	 * Variables to manage the HighLight of the OSD.
	 */
	//@{
	/// Color of the HighLight.
	CRGBA _HL_Color;
	/// HighLight Size (in Pixel).
	float _HL_Size;
	/// HighLight Size for the width (between 0-1).
	float _HL_W;
	/// HighLight Size for the height (between 0-1).
	float _HL_H;
	//@}

	/** \name Resize
	 * Variables to manage the Resize of the OSD.
	 */
	//@{
	/// Resize Mode.
	TResize	_RS_Mode;
	/// Resize borders Color
	CRGBA	_RS_Color;
	/// Resize size (in pixel).
	float	_RS_Size;
	/// Resize Size for the width (between 0-1).
	float	_RS_W;
	/// Resize Size for the height (between 0-1).
	float	_RS_H;
	//@}

	ECursor _Cursor;

public:
	/// Constructor
	COSD(bool popUp = false);
	COSD(uint Id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, float minWidth, float minHeight, bool popUp = false);
	/// Destructor
	~COSD();

	/// The OSD size has changed -> resize controls.
	void resize(uint32 width, uint32 height);
	/// Update the OSD (for timers, etc.).
	bool update(float x, float y, bool fullUse);
	/// Display the OSD.
	void display();

	/**
	 * Return the cursor used by the OSD at the moment.
	 * \return ECursor : 'Cur_None' if no cursor needed for the OSD.
	 * \warning This method should be called after the update one to be up to date.
	 */
	ECursor cursor();

	/// Manage the mouse click.
	void click(float x, float y, bool &taken);

	/// Manage the mouse right click.
	void clickRight(float x, float y, bool &taken);


	/// Add a control.
	void addChild(uint idCtrl, CControl *ctrl);
	/// Delete a control by the Id.
	void delChild(uint idCtrl);

	/// remove ctrl from children list (but not from control list)
	void removeFromChildren(uint idCtrl);

	/// Return the pointer of the control "id" or 0 if the control "id" doesn't exit.
	CControl *getCtrl(uint id);

	/// Load the OSD script.
	void open(ifstream &file);


	/** \name SHOW/HIDE
	 * Functions to show/hide the OSD.
	 */
	//@{
	/**
	 * This function return if the OSD is visible or not .
	 * \return bool : true if the OSD is visible.
	 */
	inline bool show() const {return _Show;}
	/**
	 * This function set if the OSD is visible or not .
	 * \param s : false to hide the OSD.
	 */
	inline void show(bool s) {_Show = s;}
	//@}


	/** \name OSD
	 * Functions to manage the OSD.
	 */
	//@{
	/// Change the OSD Position (between O-1).
	void osdSetPosition(float x, float y);
	/// Return the OSD Position (between O-1).
	void osdGetPosition(float &x, float &y) const ;
	/// Change the OSD Size (in Pixel).
	void osdSetSize(float w, float h);
	/// Return the OSD Size (in Pixel).
	void osdGetSize(float &w, float &h) const ;
	/// Change the OSD Mode (locked, resize, selected, etc.)
	void osdMode(TMode osdMode);
	/// Return the current OSD Mode.
	TMode osdMode() const ;
	/// Change the OSD Name.
	void osdName(const ucstring &osdName);
	/// Return the OSD Name.
	ucstring osdName() const;
	/// Set the update Function.
	void osdUpdateFunc(uint osdUpdateFunc);
	/// Get the update Function.
	uint osdUpdateFunc() const ;
	uint getId() const { return _Id; }
	//@}

	/** \name Background
	 * Functions to manage the background.
	 */
	//@{
	/// Set the Background display mode.
	void bgMode(TBG mode);
	/// Get the Background display mode.
	TBG bgMode() const ;
	/// Set the texture Id for the Background.
	void bg(uint b);
	/// Get background Id.
	uint bg() const ;
	/// Set the Background RGBA.
	void bgColor(const CRGBA &bRGBA);
	/// Get the background RGBA.
	CRGBA bgColor() const ;
	//@}

	/** \name Title Bar
	 * Functions to manage the Title Bar.
	 */
	//@{
	/// Set the Title Bar display mode.
	void tbMode(TTB mode);
	/// Get the Title Bar display mode.
	TTB tbMode() const ;
	/// Set the texture Id for the Title Bar.
	void tb(uint t);
	/// Get Title Bar Id.
	uint tb() const ;
	/// Set the Title Bar RGBA.
	void tbColor(const CRGBA &tRGBA);
	/// Get the Title Bar RGBA.
	CRGBA tbColor() const ;
	/// Set Title Bar Height (in Pixel)
	void tbHeight(float height);
	/// Get Title Bar Height (in Pixel)
	float tbHeight() const;
	/// Set the Pen for the Title Bar.
	void tbPen(const CPen &pen);
	/// Get the Pen of the Title Bar.
	CPen tbPen() const ;
	//@}

	/** \name HighLight
	 * Functions to manage the HighLight.
	 */
	//@{
	/// Set the HighLight Color.
	void hlColor(const CRGBA &hlColor);
	/// Get the HighLight Color.
	CRGBA hlColor() const ;
	/// Set the HighLight Size (in Pixel).
	void hlSize(float hlSize);
	/// Get the HighLight Size (in Pixel).
	float hlSize() const ;
	//@}

	/** \name Resize
	 * Functions to manage the Resize of the OSD.
	 */
	//@{
	/// Set the Resize Mode.
	void rsMode(TResize rsMode);
	/// Get the Resize Mode.
	TResize rsMode() const ;
	/// Set Resize borders Color
	void rsColor(CRGBA rsColor);
	/// Get Resize borders Color
	CRGBA rsColor() const ;
	/// Set Resize size (in pixel).
	void rsSize(float rsSize);
	/// Get Resize size (in pixel).
	float rsSize() const ;
	//@}


};


#endif // CL_OSD_H

/* End of osd.h */
