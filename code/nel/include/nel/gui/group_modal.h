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



#ifndef NL_GROUP_MODAL_H
#define NL_GROUP_MODAL_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_frame.h"

namespace NLGUI
{

	// ***************************************************************************
	/**
	 * A group with special modal options
	 * \author Lionel Berenguier
	 * \author Nevrax France
	 * \date 2002
	 */
	class CGroupModal : public CGroupFrame
	{
	public:
        DECLARE_UI_CLASS( CGroupModal )

		bool		SpawnOnMousePos		: 1;
		bool		ExitClickOut		: 1;
		bool		ExitClickL			: 1;
		bool		ExitClickR			: 1;
		bool		ForceInsideScreen	: 1;
		bool		ExitKeyPushed		: 1;
		sint32		SpawnMouseX, SpawnMouseY;
		std::string Category;

		std::string OnClickOut;				// Launched when clicking out of the window, and BEFORE a new control has been cpatured
		std::string OnClickOutParams;
		std::string OnPostClickOut;			// Launched when clicking out of the window, and AFTER a new control has been captured
		std::string OnPostClickOutParams;
	public:

		/// Constructor
		CGroupModal(const TCtorParam &param);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
		virtual void updateCoords ();
		void setBaseX(sint32 x) { _MouseDeltaX = x;}
		void setBaseY(sint32 y) { _MouseDeltaY = y;}

		REFLECT_EXPORT_START(CGroupModal, CGroupFrame)
		REFLECT_EXPORT_END

	// ******************
	protected:
		sint32		_MouseDeltaX, _MouseDeltaY;
	};

}

#endif // NL_GROUP_MODAL_H

/* End of group_modal.h */
