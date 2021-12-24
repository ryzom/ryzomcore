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


#ifndef FDC_CACHE_H
#define FDC_CACHE_H


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/singleton.h"
#include "game_share/file_description_container.h"


//-------------------------------------------------------------------------------------------------
// class CFdcCache
//-------------------------------------------------------------------------------------------------

class CFdcCache: public NLMISC::CSingleton<CFdcCache>
{
public:
	const CFileDescriptionContainer& getFdc() const;
	void addFileSpec(const NLMISC::CSString& fspec,bool recurse);
	void addFile(const NLMISC::CSString& fileName);
	void clear();

private:
	CFileDescriptionContainer _Fdc;
};


//-------------------------------------------------------------------------------------------------

#endif // FDC_CACHE_H
