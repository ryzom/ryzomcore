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
#include "s_phrase_factory.h"

#include "egs_sheets/egs_sheets.h"

using namespace std;
using namespace NLMISC;

vector< std::pair< BRICK_TYPE::EBrickType , ISPhraseFactory* > > * ISPhraseFactory::Factories;

CSPhrasePtr ISPhraseFactory::buildPhrase( const TDataSetRow & actorRowId,const std::vector< NLMISC::CSheetId>& brickIds, bool execution )
{
	// the check to see if there is at least a brick is made before
	nlassert( !brickIds.empty() );
	nlassert( Factories );
	
	// transform sheet ids into forms
	std::vector< const CStaticBrick* > bricks( brickIds.size() );
	uint16 nbNull = 0;
	for ( uint i = 0; i < bricks.size(); ++i )
	{
		// skip unknown sheetIds
		if (  brickIds[i] == NLMISC::CSheetId::Unknown )
		{
			++nbNull;
			continue;
		}
		
		bricks[i-nbNull] = CSheets::getSBrickForm( brickIds[i] );
		
		if ( bricks[i-nbNull] == NULL )
		{
			nlwarning( "<ISPhraseFactory buildPhrase> invalid SBrick sheet %u, named %s",brickIds[i].asInt(),brickIds[i].toString().c_str() );
			return NULL;
		}
	}
	
	if (nbNull)
		bricks.resize( bricks.size() - nbNull);
	
	//get the root brick family and the associated factory
	for ( uint i = 0; i < Factories->size(); i++ )
	{
		if ( (*Factories)[i].first == BRICK_FAMILIES::brickType(bricks[0]->Family) )
		{
			CSPhrasePtr phrase = (*Factories)[i].second->buildPhrase( actorRowId, bricks, execution);
			if ( phrase )
			{
				phrase->setBrickSheets(brickIds);
				return phrase;
			}
			return NULL;
		}
	}
	nlwarning( "<ISPhraseFactory buildPhrase> the brick type %s has no corresponding phrase class", BRICK_TYPE::toString(BRICK_FAMILIES::brickType( bricks[0]->Family )).c_str() );
	return NULL;
}

