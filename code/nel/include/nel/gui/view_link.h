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



#ifndef NL_VIEW_LINK_H
#define NL_VIEW_LINK_H

#include "nel/gui/view_text.h"

namespace NLGUI
{

	class CGroupHTML;

	/**
	 * class implementing a link view
	 * \author Cyril 'Hulud' Corvazier
	 * \author Nicolas Vizerie
	 * \author Nevrax France
	 * \date 2003
	 */
	class CViewLink : public CViewText
	{
	public:
        DECLARE_UI_CLASS( CViewLink )

		// Default constructor
		CViewLink (const TCtorParam &param);

		// The URI
		std::string		Link;

		std::string		LinkTitle;

		// Set the main group
		void	setHTMLView( CGroupHTML *html);
		bool	getMouseOverShape(std::string &texName, uint8 &rot, NLMISC::CRGBA &col);

		void setActionOnLeftClick(const std::string &actionHandler) { _AHOnLeftClick = actionHandler; };
		void setParamsOnLeftClick(const std::string &actionParams) { _AHOnLeftClickParams = actionParams; };

		const std::string &getActionOnLeftClick() const { return _AHOnLeftClick; }
		const std::string &getParamsOnLeftClick() const { return _AHOnLeftClickParams; }
	protected:

		// The main HTML group
		CGroupHTML		*HTML;

		// Left mouse click action
		// Don't use CStringShared as URLs change past values would be permanently remembered.
		std::string		_AHOnLeftClick;
		std::string		_AHOnLeftClickParams;

	};

}

#endif // NL_VIEW_LINK_H

/* End of view_link.h */
