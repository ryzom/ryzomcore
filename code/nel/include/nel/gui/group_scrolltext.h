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




#ifndef CL_GROUP_SCROLLTEXT_H
#define CL_GROUP_SCROLLTEXT_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"

namespace NLGUI
{
	class CCtrlBaseButton;
	class CCtrlScroll;
	class CGroupList;

	// Can be used to build a chat window or anything that displays sequences of strings
	/**
	 * Widget to have a resizable scrolltext and its scrollbar
	 * \author Nicolas Vizerie
	 * \author Nevrax France
	 * \date 2002
	 */
	class CGroupScrollText : public CInterfaceGroup
	{
	public:
        DECLARE_UI_CLASS( CGroupScrollText )

		/// Constructor
		CGroupScrollText(const TCtorParam &param);
		~CGroupScrollText();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		/// CInterfaceGroup Interface
		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
		virtual void updateCoords ();
		virtual void checkCoords ();
		virtual void draw ();
		virtual void clearViews ();
		virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

		// get the list associated to this group
		CGroupList	*getList() const { return _List; }

		// Get the scroll bar
		CCtrlScroll	 *getScrollBar() const { return	_ScrollBar; }

		// from CCtrlBase
		virtual	void		elementCaptured(CCtrlBase *capturedElement);

		// setup vertical scrolling event
		void	smoothScrollY (sint32 dy);

		REFLECT_EXPORT_START(CGroupScrollText, CInterfaceGroup)
		REFLECT_EXPORT_END

	private:
		CGroupList        *_List;
		CCtrlScroll	      *_ScrollBar;
		CCtrlBaseButton		  *_ButtonAdd;
		CCtrlBaseButton		  *_ButtonSub;
		bool            _Settuped;
		bool			_InvertScrollBar;
		sint32          _ListHeight;
		bool			_Scrolling;
		float			_ScrollDistance;
		bool			_ClockMsgEventRegistered;
	protected:
		void    setup();
		void    updateScrollBar();
	public:
		// private use for action handlers
		sint32			_StartHeight;
		sint64          _EllapsedTime;
	};

}

#endif
