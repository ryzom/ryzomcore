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

#ifndef CTRL_SCROLL_BASE_H
#define CTRL_SCROLL_BASE_H

#include "nel/gui/ctrl_base.h"

namespace NLGUI
{

	class CInterfaceGroup;

	class CCtrlScrollBase : public CCtrlBase
	{
	public:
		DECLARE_UI_CLASS( CCtrlScrollBase )

		CCtrlScrollBase( const TCtorParam &param );
		virtual ~CCtrlScrollBase();
		
		virtual void setTarget( CInterfaceGroup *pIG );
		CInterfaceGroup* getTarget(){ return _Target; }
		virtual sint32 moveTrackX( sint32 dx );
		virtual sint32 moveTrackY( sint32 dy );

		/** Move the Target Ofs with a Delta, and recompute TrackPos from this Ofs.
		 *	Useful for finer controled group scrolling when the list is very big (with mouseWheel or scroll buttons)
		 */
		virtual void moveTargetX( sint32 dx );
		virtual void moveTargetY( sint32 dy );


		// Necessary because of reflection, no other purpose
		void draw(){}

	protected:
		CInterfaceGroup *_Target; // If NULL the scroller is a value scroller

	private:

	};

}

#endif

