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


#ifndef GROUP_EDITBOX_BASE_H
#define GROUP_EDITBOX_BASE_H

#include "nel/gui/interface_group.h"

namespace NLGUI
{

	class CGroupEditBoxBase : public CInterfaceGroup
	{
	public:
		DECLARE_UI_CLASS( CGroupEditBoxBase )

		CGroupEditBoxBase( const TCtorParam &param );
		~CGroupEditBoxBase();

		// True if the editBox can recover the focus on enter. if not, it does not erase OldCapturedKeyboard when loose focus
		bool getRecoverFocusOnEnter() const{ return _RecoverFocusOnEnter; }
		void setRecoverFocusOnEnter( bool state ){ _RecoverFocusOnEnter = state; }

		std::string getAHOnFocus(){ return _AHOnFocus; }
		std::string getAHOnFocusParams(){ return _AHOnFocusParams; }

		// disable any current selection
		static void	disableSelection(){ _CurrSelection = NULL; }

		// Get / set current selection
		static CGroupEditBoxBase *getCurrSelection(){ return _CurrSelection; }
		static void setCurrSelection( CGroupEditBoxBase *selection ){ _CurrSelection = selection; }

		void draw(){}

		REFLECT_EXPORT_START( CGroupEditBoxBase, CInterfaceGroup )
			REFLECT_BOOL( "enter_recover_focus", getRecoverFocusOnEnter, setRecoverFocusOnEnter );
		REFLECT_EXPORT_END

	protected:
		bool _RecoverFocusOnEnter : 1;

		std::string _AHOnFocus;
		std::string _AHOnFocusParams;

		static CGroupEditBoxBase *_CurrSelection; // the edit box for which the selection is currently active, or NULL if there's none

	};

}

#endif

