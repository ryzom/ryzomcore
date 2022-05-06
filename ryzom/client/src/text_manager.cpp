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


#if 0
#include "stdpch.h"


/////////////
// INCLUDE //
/////////////
#include "text_manager.h"


#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

///////////
// USING //
///////////
using namespace std;


////////////
// GLOBAL //
////////////
CTextManager TextMngr;


/////////////
// METHODS //
/////////////
//-----------------------------------------------
// CTextManager :
// Constructor.
//-----------------------------------------------
CTextManager::CTextManager()
{
}// CTextManager //

//-----------------------------------------------
/// Return the text according to the Id.
//-----------------------------------------------
ucstring CTextManager::text(uint textId) // OLD
{
	map<uint, ucstring>::iterator it = _Texts.find(textId); // OLD
	if(it != _Texts.end())
		return (*it).second;

	// Return an empty string.
	return ucstring(); // OLD
}

//-----------------------------------------------
/// Set the text for 'textId'.
//-----------------------------------------------
void CTextManager::text(uint textId, const std::string &str)
{
	_Texts.insert(make_pair(textId, ucstring(str))); // OLD
}

//-----------------------------------------------
/// Set the text for 'textId'.
//-----------------------------------------------
void CTextManager::text(uint textId, const ucstring &str) // OLD
{
	_Texts.insert(make_pair(textId, str));
}
#endif
