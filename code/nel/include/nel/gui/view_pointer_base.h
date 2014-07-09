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


#ifndef VIEW_POINTER_BASE_H
#define VIEW_POINTER_BASE_H

#include "nel/misc/events.h"
#include "nel/gui/view_base.h"

namespace NLGUI
{

	class CViewPointerBase : public CViewBase
	{
	public:
		DECLARE_UI_CLASS( CViewPointerBase )

		CViewPointerBase( const TCtorParam &param );
		virtual ~CViewPointerBase();

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

		void draw(){}

		/// set button state
		void setButtonState(NLMISC::TMouseButton state) { _Buttons = state; }
		/// get buttons state
		NLMISC::TMouseButton getButtonState() const { return _Buttons; }

		static const sint32 InvalidCoord = 0x80000000;

	protected:
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

	private:


	};

}

#endif

