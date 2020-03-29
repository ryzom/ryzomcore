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

#ifndef STAT_GLOBALS_H
#define STAT_GLOBALS_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/sheet_id.h"


//-----------------------------------------------------------------------------
// Methods
//-----------------------------------------------------------------------------

namespace STAT_GLOBALS
{
	NLMISC::CSString getInputFilePath(const NLMISC::CSString& path= NLMISC::CSString());
	NLMISC::CSString getScriptFilePath(const NLMISC::CSString& path= NLMISC::CSString());
	NLMISC::CSString getOutputFilePath(const NLMISC::CSString& path= NLMISC::CSString());

	void clearAccountNames();
	void clearAllCharacterNames();
	void clearCharacterNames(const NLMISC::CSString& shard);
	void clearSheetNames();

	void addAccountNameMapping(uint32 account,const NLMISC::CSString& name);
	void addCharacterNameMapping(const NLMISC::CSString& shard,uint32 account,uint32 slot,const NLMISC::CSString& name);
	void addSheetNameMapping(NLMISC::CSheetId sheetId,const NLMISC::CSString& name);

	const NLMISC::CSString& getAccountName(uint32 accountId);
	const NLMISC::CSString& getCharacterName(const NLMISC::CSString& shard,uint32 account,uint32 slot);
	const NLMISC::CSString& getSheetName(NLMISC::CSheetId sheetId);
}


//-----------------------------------------------------------------------------
#endif
