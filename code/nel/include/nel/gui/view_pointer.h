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
#include "nel/gui/view_pointer_base.h"

namespace NLGUI
{
	class CCtrlBase;
	class CGroupContainer;

	/**
	 * class describing the pointer
	 * \author Matthieu 'Trap' Besson
	 * \author Nevrax France
	 * \date 2002
	 */

	class CViewPointer : public CViewPointerBase
	{

	public:
		DECLARE_UI_CLASS( CViewPointer )
		
		CViewPointer( const TCtorParam &param );
		virtual ~CViewPointer(){}

		bool parse (xmlNodePtr cur,CInterfaceGroup * parentGroup);
		void draw();

		// Set cursor mode
		void setStringMode (bool stringCursor);
		bool getStringMode() const {return _StringMode;}

		// Set cursor string
		void setString (const ucstring &str);

		// TEMP PATCH
		void setCursor (const std::string &name)
		{
			_TxDefault = name;
			_TxIdDefault = -2;
		}
		// TEMP PATCH

		/// Show or hide the pointer. Please, use SetMouseMode (bool freelook) instead.
		void show(bool s) {_PointerVisible = s;}

		static void setHWMouse( bool hw ){ hwMouse = hw; }
		static void forceLink();

	private:

		/// Drawing helpers
		virtual bool drawResizer(CCtrlBase* pCB, NLMISC::CRGBA col){ return false; }
		virtual bool drawRotate(CCtrlBase* pCB, NLMISC::CRGBA col){ return false; }
		virtual bool drawScale(CCtrlBase* pCB, NLMISC::CRGBA col){ return false; }
		virtual bool drawColorPicker(CCtrlBase* pCB, NLMISC::CRGBA col){ return false; }
		virtual bool drawLink(CCtrlBase* pCB, NLMISC::CRGBA col){ return false; }
		virtual bool drawBrowse(CCtrlBase* pCB, NLMISC::CRGBA col){ return false; }
		virtual bool drawPan(CCtrlBase* pCB, NLMISC::CRGBA col){ return false; }
		virtual bool drawCustom(CCtrlBase* pCB);

	protected:

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

		CGroupContainer *_LastHightLight;

		// Cursor mode
		bool				_StringMode;
		bool				_ForceStringMode;
		CInterfaceGroup		*_StringCursor;
		CInterfaceGroup		*_StringCursorHardware;
		ucstring			_ContextString;

		// draw current cursor with the given texture, or, if in hardware mode, change the hardware cursor shape
		void drawCursor(sint32 texId, NLMISC::CRGBA col, uint8 rot);

	private:
		// set the string into frame for software or hardware version
		void setString (const ucstring &str, CInterfaceGroup *target);

		static bool hwMouse;

	};

}

#endif // RZ_VIEW_POINTER_H

/* End of view_pointer.h */
