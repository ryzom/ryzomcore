// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#ifndef CTRL_DRAGGABLE_H
#define CTRL_DRAGGABLE_H

#include "nel/gui/ctrl_base.h"

namespace NLGUI
{

	class CCtrlDraggable : public CCtrlBase
	{
	public:
		DECLARE_UI_CLASS( CCtrlDraggable )

		CCtrlDraggable( const TCtorParam &param );
		virtual ~CCtrlDraggable(){}

		static CCtrlDraggable *getDraggedSheet(){ return _LastDraggedSheet; }
		bool isDragged() const{ return dragged; }
		void setDragged( bool dragged ){ this->dragged = dragged; }
		bool isDraggable() const{ return draggable; }
		void setDraggable( bool draggable ){ this->draggable = draggable; }
		
		void abortDragging()
		{
			dragged = false;
			_LastDraggedSheet = NULL;
		}

		// Necessary because of reflection, no other purpose
		void draw(){}

		REFLECT_EXPORT_START(CCtrlDraggable, CCtrlBase)
			REFLECT_BOOL("dragable", isDraggable, setDraggable);
		REFLECT_EXPORT_END

	protected:
		static void setDraggedSheet( CCtrlDraggable *draggable ){ _LastDraggedSheet = draggable; }

	private:
		static CCtrlDraggable *_LastDraggedSheet;
		bool dragged;
		bool draggable;
	};

}

#endif
