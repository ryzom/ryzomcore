// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// Lost code

#if 0
#ifndef NL_TEXT_MANAGER_H
#define NL_TEXT_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h" // OLD
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
	std::map<uint, ucstring> _Texts; // OLD

public:
	/// Constructor
	CTextManager();

	/// Return the text according to the Id.
	ucstring text(uint textId); // OLD
	/// Set the text for 'textId'.
	void text(uint textId, const std::string &str);
	/// Set the text for 'textId'.
	void text(uint textId, const ucstring &str); // OLD
};


////////////
// EXTERN //
////////////
extern CTextManager TextMngr;



#endif // NL_TEXT_MANAGER_H
#endif

/* End of text_manager.h */
