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



#ifndef RZ_VIEW_POINTER_H
#define RZ_VIEW_POINTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/events.h"
#include "view_base.h"

class CGroupContainer;
class CCtrlBase;

/**
 * class describing the pointer
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date 2002
 */

class CViewPointer : public CViewBase
{
	friend void	SetMouseFreeLook ();
	friend void	SetMouseCursor (bool updateMousePos);
public:
	CViewPointer (const TCtorParam &param);
	bool parse (xmlNodePtr cur,CInterfaceGroup * parentGroup);
	void draw ();

	// Set the pointer position.
	void setPointerPos (sint32 x, sint32 y);
	void setPointerDispPos (sint32 x, sint32 y);

	void resetPointerPos ();
	void setPointerDown (bool pd);
	void setPointerDownString (const std::string &s);

	void getPointerPos (sint32 &x, sint32 &y);
	void getPointerDispPos (sint32 &x, sint32 &y);

	void getPointerOldPos (sint32 &x, sint32 &y);
	void getPointerDownPos (sint32 &x, sint32 &y);
	bool getPointerDown ();
	std::string getPointerDownString ();
	bool getPointerDrag ();

	/// Is the pointer visible ?
	bool show() const {return _PointerVisible;}

	// Set cursor mode
	void setStringMode (bool stringCursor);
	bool getStringMode() const {return _StringMode;}

	// Set cursor string
	void setString (const ucstring &str);

public:
	// TEMP PATCH
	void setCursor (const std::string &name)
	{
		_TxDefault = name;
		_TxIdDefault = -2;
	}
	// TEMP PATCH

	/// set button state
	void setButtonState(NLMISC::TMouseButton state) { _Buttons = state; }
	/// get buttons state
	NLMISC::TMouseButton getButtonState() const { return _Buttons; }

private:

	/// Show or hide the pointer. Please, use SetMouseMode (bool freelook) instead.
	void show(bool s) {_PointerVisible = s;}

	/// Drawing helpers
	bool drawResizer	(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawMover		(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawRotate		(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawScale		(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawColorPicker (CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawLink		(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawBrowse		(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawPan		(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawCustom		(CCtrlBase* pCB);

private:

	// Look of the cursor in different situation
	std::string	_TxDefault;
	std::string	_TxMoveWindow;
	std::string _TxResizeBRTL;
	std::string _TxResizeBLTR;
	std::string _TxResizeTB;
	std::string _TxResizeLR;
	std::string _TxRotate;
	std::string _TxScale;
	std::string _TxColPick;
	std::string _TxPan;
	std::string _TxCanPan;
	std::string _TxPanR2;
	std::string _TxCanPanR2;

	sint32 _TxIdDefault;
	sint32 _TxIdMoveWindow;
	sint32 _TxIdResizeBRTL;
	sint32 _TxIdResizeBLTR;
	sint32 _TxIdResizeTB;
	sint32 _TxIdResizeLR;
	sint32 _TxIdRotate;
	sint32 _TxIdScale;
	sint32 _TxIdColPick;
	sint32 _TxIdPan;
	sint32 _TxIdCanPan;
	sint32 _TxIdPanR2;
	sint32 _TxIdCanPanR2;

	NLMISC::CRGBA _Color;

	sint32		_OffsetX;
	sint32		_OffsetY;

	// (x,y) is from the TopLeft corner of the window
	sint32		_PointerX;				// Current pointer position (raw, before snapping)
	sint32		_PointerY;
	sint32		_PointerOldX;			// Previous frame pointer position
	sint32		_PointerOldY;
	bool		_PointerDown;			// Is the pointer down ?
	sint32		_PointerDownX;			// Pointer down position
	sint32		_PointerDownY;
	std::string	_PointerDownString;		// What is under the pointer at the down position
	bool		_PointerDrag;			// Is the pointer down and we have moved ?
	bool		_PointerVisible;		// Is the pointer visible or hidden ?

	NLMISC::TMouseButton _Buttons;

	CGroupContainer *_LastHightLight;

	// Cursor mode
	bool				_StringMode;
	bool				_ForceStringMode;
	CInterfaceGroup		*_StringCursor;
	CInterfaceGroup		*_StringCursorHardware;
	ucstring			_ContextString;

private:
	// draw current cursor with the given texture, or, if in hardware mode, change the hardware cursor shape
	void drawCursor(sint32 texId, NLMISC::CRGBA col, uint8 rot);
	// set the string into frame for software or hardware version
	void setString (const ucstring &str, CInterfaceGroup *target);

};



#endif // RZ_VIEW_POINTER_H

/* End of view_pointer.h */
