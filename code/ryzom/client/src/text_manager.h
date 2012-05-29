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



#ifndef NL_TEXT_MANAGER_H
#define NL_TEXT_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"
// Std
#include <map>


///////////
// CLASS //
///////////
/**
 * <Class description>
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CTextManager
{
protected:
	std::map<uint, ucstring> _Texts;

public:
	/// Constructor
	CTextManager();

	/// Return the text according to the Id.
	ucstring text(uint textId);
	/// Set the text for 'textId'.
	void text(uint textId, const std::string &str);
	/// Set the text for 'textId'.
	void text(uint textId, const ucstring &str);
};


////////////
// EXTERN //
////////////
extern CTextManager TextMngr;



#endif // NL_TEXT_MANAGER_H

/* End of text_manager.h */
