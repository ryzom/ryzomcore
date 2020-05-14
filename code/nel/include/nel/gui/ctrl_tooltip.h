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



#ifndef RZ_CTRL_TOOLTIP_H
#define RZ_CTRL_TOOLTIP_H

#include "nel/gui/ctrl_base.h"
#include "nel/3d/u_texture.h"

namespace NLGUI
{
	class CEventDescriptor;


	/**
	 * \author Matthieu 'Mr TRAP' Besson
	 * \author Nevrax France
	 * \date 2003
	 */
	class CCtrlToolTip : public CCtrlBase
	{
	public:
		DECLARE_UI_CLASS(CCtrlToolTip)
		/// Constructor
		CCtrlToolTip(const TCtorParam &param) : CCtrlBase(param) {}

		virtual bool handleEvent (const NLGUI::CEventDescriptor& eventDesc);
		virtual void draw();
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		virtual bool		parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
		// Can do nothing with tooltip (but display it :) )
		virtual	bool		isCapturable() const {return false;}
		virtual void        serial(NLMISC::IStream &f);
	public:

	};


}

#endif // RZ_CTRL_TOOLTIP_H

/* End of ctrl_tooltip.h */

