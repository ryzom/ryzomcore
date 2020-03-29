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

#include "nel/misc/command.h"
#include "nel/misc/random.h"

#include "game_share/item_family.h"
#include "game_item_manager/game_item.h"

#include "egs_sheets/egs_sheets.h"
#include "phrase_manager/faber_phrase.h"

#include "shop_type/static_items.h"

#include <string>

using namespace std;
using namespace NLMISC;

extern NLMISC::CRandom RandomGenerator;
extern CVariable<bool> EGSLight;

std::vector< CGameItemPtr > CStaticItems::_StaticItems;
std::vector< CStaticItems::CRMForOneItemPart > CStaticItems::_RmForSystemCraft;


//----------------------------------------------------------------------------
// CStaticItems::buildStaticItem craft static item for sell it
// 
//----------------------------------------------------------------------------
void CStaticItems::buildStaticItem()
{
	if (EGSLight)
		return;

	// RM table and random Mp for build item are disabled until we decide if we build npc item with some random or not
//	initRmTable();

	uint nbbuilt = 0;
	for( CAllStaticItems::const_iterator it = CSheets::getItemMapForm().begin(); it != CSheets::getItemMapForm().end(); ++it )
	{
		// sheet starting with _ are parent, we can't sell them
		if( (*it).first.toString()[0] == '_' )
		{
//			nldebug("SI: skip because is parent '%s'", (*it).first.toString().c_str());
			continue;
		}

		// sheet starts *not* with ic (item craftable), we can't sell it because can't create it
		if( (*it).first.toString().substr(0,2) != "ic" )
		{
//			nldebug("SI: skip because is not start with ic '%s'", (*it).first.toString().c_str());
			continue;
		}

		{
			CGameItemPtr item = buildOnetItem( (*it).first );
			if( item != 0 )
			{
				_StaticItems.push_back( item );
				nbbuilt++;
//				nldebug("SI: Item %s built", (*it).first.toString().c_str() );
			}
		}
	}
	nlinfo("SI: %d Items built", nbbuilt);
}


//----------------------------------------------------------------------------
// CStaticItems::buildOntItem use faber system for make an item
// 
//----------------------------------------------------------------------------
CGameItemPtr CStaticItems::buildOnetItem( CSheetId sheet )
{
	const CStaticItem * staticItem = CSheets::getForm( sheet );
	if( staticItem != 0 )
	{	
		if( staticItem->Family != ITEMFAMILY::AMMO &&
			staticItem->Family != ITEMFAMILY::ARMOR  &&
			staticItem->Family != ITEMFAMILY::JEWELRY &&
			staticItem->Family != ITEMFAMILY::MELEE_WEAPON &&
			staticItem->Family != ITEMFAMILY::RANGE_WEAPON &&
			staticItem->Family != ITEMFAMILY::SHIELD )
			return 0;

		if( staticItem->CraftPlan == CSheetId::Unknown )
		{
			nlwarning("SI: Item %s have an unknown plan", sheet.toString().c_str() );
			return 0;
		}

		if( staticItem->Family == ITEMFAMILY::MELEE_WEAPON ||
			staticItem->Family == ITEMFAMILY::RANGE_WEAPON ||
			staticItem->Family == ITEMFAMILY::AMMO ||
			staticItem->Family == ITEMFAMILY::ARMOR ||
			staticItem->Family == ITEMFAMILY::SHIELD ||
			staticItem->Family == ITEMFAMILY::JEWELRY )
		{
			CFaberPhrase phrase;
			vector< CSheetId > Rm;
			vector< CSheetId > RmFormula;

			// select raw material used for craft item
			selectRmForCraft( staticItem->CraftPlan, Rm, RmFormula );
			
			// craft item and return it's pointer
			return phrase.systemCraftItem( staticItem->CraftPlan, Rm, RmFormula );
		}
		else
		{
			nlwarning("SI: Item %s has not a good family to be sell family='%s'", sheet.toString().c_str(), ITEMFAMILY::toString(staticItem->Family).c_str() );
			return 0;
		}
	}
	else
	{
		nlwarning("SI: Can't found static form for item %s", sheet.toString().c_str() );
		return 0;
	}
}


//----------------------------------------------------------------------------
// CStaticItems::initRmTable used for build static item
// 
//----------------------------------------------------------------------------
void CStaticItems::initRmTable()
{
	_RmForSystemCraft.clear();
	_RmForSystemCraft.resize( RM_FABER_TYPE::NUM_FABER_TYPE );

	for( CAllStaticItems::const_iterator it = CSheets::getItemMapForm().begin(); it != CSheets::getItemMapForm().end(); ++it )
	{
		if( (*it).second.Family == ITEMFAMILY::RAW_MATERIAL )
		{
			nlassert( (*it).second.Mp != 0 );
			if( (*it).second.Mp->StatEnergy <= 35 )
			{
				for( sint32 i = 0; i < RM_FABER_TYPE::NUM_FABER_TYPE; ++i )
				{
					if( (*it).second.Mp != 0 )
					{
						CMP * cmp = (*it).second.Mp;
						if( (*it).second.Mp->getMpFaberParameters( i ) != 0 )
						{
							_RmForSystemCraft[ i ].RawMaterial.push_back( (*it).first );
						}
					}
					else
					{
						nlwarning("Sheet %s type ITEMFAMILY::RAW_MATERIAL without Mp field data", (*it).first.toString().c_str() );
					}
				}
			}
		}
	}
}


//----------------------------------------------------------------------------
// CStaticItems::selectRmForCraft used for build static item
// 
//----------------------------------------------------------------------------
void CStaticItems::selectRmForCraft( const CSheetId& craftPlan, vector< CSheetId >& Rm, vector< CSheetId >& RmFormula )
{
	const CStaticBrick * rootPlan = CSheets::getSBrickForm( craftPlan );
	if( rootPlan != 0 )
	{
		nlassert( rootPlan->Faber != 0 );
		for( uint32 i = 0; i < rootPlan->Faber->NeededMps.size(); ++i )
		{
			for( uint32 nbRm = 0; nbRm != rootPlan->Faber->NeededMps[ i ].Quantity; ++nbRm )
			{
				// RM table and random Mp for build item are disabled until we decide if we build npc item with some random or not
//				nlassert( rootPlan->Faber->NeededMps[ i ].MpType < (sint32)_RmForSystemCraft.size() );
//				nlassert( _RmForSystemCraft[ rootPlan->Faber->NeededMps[ i ].MpType ].RawMaterial.size() > 0 );
//				Rm.push_back( _RmForSystemCraft[ rootPlan->Faber->NeededMps[ i ].MpType ].RawMaterial[ RandomGenerator.rand( _RmForSystemCraft[ rootPlan->Faber->NeededMps[ i ].MpType ].RawMaterial.size() - 1 ) ] );

				static CSheetId rmSheet("system_mp_fine.sitem");
				Rm.push_back(rmSheet);
			}
		}

		for( uint32 i = 0; i < rootPlan->Faber->NeededMpsFormula.size(); ++i )
		{
			for( uint32 nbRm = 0; nbRm != rootPlan->Faber->NeededMpsFormula[ i ].Quantity; ++nbRm )
			{
				RmFormula.push_back( rootPlan->Faber->NeededMpsFormula[ i ].MpType );
			}
		}
	}
}
