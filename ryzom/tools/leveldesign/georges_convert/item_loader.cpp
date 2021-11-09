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

#include "stdgeorgesconvert.h"
#include "item_loader.h"

namespace NLOLDGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemLoader::CItemLoader()
{
}

CItemLoader::~CItemLoader()
{
}

void CItemLoader::LoadItem( CItem& _item, const CStringEx& _sxfilename )
{
	_item.Load( _sxfilename );
}

void CItemLoader::LoadItem( CItem& _item, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	_item.Load( _sxfilename, _sxdate );
}

void CItemLoader::SaveItem( CItem& _item, const CStringEx& _sxfilename )
{
	_item.Save( _sxfilename );
}

}