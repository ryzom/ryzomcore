// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#ifndef DIALOG_STACK_H
#define DIALOG_STACK_H


/** this helps to record dynamically created window or dialog
  * Just deriv from this and call the method pushWnd to register a new window
  * They'll be deleted in the dtor
  */


class CDialogStack
{
	public:
		void pushWnd(CWnd *wnd)
		{
			// a window must be registered only once
			nlassert(std::find(_WndList.begin(), _WndList.end() , wnd) == _WndList.end()) ; 			
			_WndList.push_back(wnd) ;
		}

		~CDialogStack()
		{
			for (std::vector<CWnd *>::iterator it = _WndList.begin() ; it != _WndList.end() ; ++it)
			{
				(*it)->DestroyWindow() ;
				delete *it ;
			}
		}

	protected:

		std::vector<CWnd *> _WndList ;

} ;

#endif