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



#include "stdpch.h"
#include "egs_static_rolemaster_phrase.h"
#include "egs_static_brick.h"
#include "egs_sheets.h"
#include "game_share/fame.h"

//Nel georges
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


// ***************************************************************************
void CStaticRolemasterPhrase::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	form->getRootNode().getValueByName( IsRolemasterPhrase, "usable_by_player" );
	form->getRootNode().getValueByName( IsValidPhrase, "castable" );

	string sbrickSheetName;
	uint i = 0;
	while ( i < 100 )
	{
		bool res = form->getRootNode().getValueByName( sbrickSheetName, toString( "brick %u", i ).c_str() );
		if ( (! res) || sbrickSheetName.empty() )
		{
			++i;
			continue;
		}
		Bricks.push_back( CSheetId( sbrickSheetName ) );
		++i;
	}

	computeMinFameValue();
}

// ***************************************************************************
void CStaticRolemasterPhrase::computeMinFameValue()
{
	MinFameValue = (sint32)0x80000000;
	for ( uint i = 0; i  < Bricks.size(); i++ )
	{
		const CStaticBrick * brick = CSheets::getSBrickForm( Bricks[i] );
		if( brick && brick->MinFameValue > MinFameValue )
			MinFameValue = brick->MinFameValue;
	}
}

// ***************************************************************************
void CStaticRolemasterPhrase::reloadSheet(const CStaticRolemasterPhrase &o)
{
	// nothing special
	*this= o;
}

