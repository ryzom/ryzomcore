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
#include <errno.h>

#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"

#include "game_share/loot_harvest_state.h"
#include "game_share/visual_slot_manager.h"
#include "game_share/brick_families.h"

#include "egs_sheets/egs_static_game_sheet.h"
#include "egs_sheets/egs_static_success_table.h"
#include "server_share/creature_size.h"
#include "game_share/roles.h"
#include "game_share/fame.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

const float MinDamage = 25;
const float DamageStep = 1.0f;
const float MeanSuccessFactor = 0.65f;
const float Offset = -0.5f;

extern CRandom RandomGenerator;

map< pair<uint16, uint16>, CStaticGameBrick *> CStaticGameBrick::_Bricks;

///////////////////////////////////////////////////////////////////////////
/////////////////////// Static Game Brick /////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//-----------------------------------------------
// readGeorges for CStaticGameBrick
//
//-----------------------------------------------
void CStaticGameBrick::readGeorges( const CSmartPtr<UForm> &form, const CSheetId &sheetId )
{
	//nlinfo ("<CStaticGameBrick::readGeorges> Ajoute la fiche %s %d", sheetId.toString().c_str(), sheetId.asInt());
	// load sheet
	if( !form )
	{
		return;
	}

	string value;
	UFormElm& root = form->getRootNode();

	// FamilyId
	if( root.getValueByName (value, "Basics.FamilyId") )
	{
		FamilyId = BRICK_FAMILIES::toSBrickFamily( value );	
	}
	else
	{		
		nlwarning("<CStaticGameBrick::readGeorges> can't get the value 'FamilyId' for sheet %s", sheetId.toString().c_str() );
	}
	

	// IndexInFamily
	if( ! root.getValueByName (IndexInFamily, "Basics.IndexInFamily") )	
	{
		nlwarning("<CStaticGameBrick::readGeorges> can't get the value 'IndexInFamily' for sheet %s", sheetId.toString().c_str() );
	}

	// Type
	if( root.getValueByName (value, "Basics.Type") )	
	{
		Type = BRICK_TYPE::toBrickType( value );
	}
	else
	{
		nlwarning("<CStaticGameBrick::readGeorges> can't get the value 'Type' for sheet %s", sheetId.toString().c_str() );
	}

	// Level
	if( ! root.getValueByName (Level, "Basics.Level") )	
	{
		nlwarning("<CStaticGameBrick::readGeorges> can't get the value 'Level' for sheet %s", sheetId.toString().c_str() );
	}

	// race
	if( root.getValueByName (value, "Learning.Race") )	
	{
		Race = EGSPD::CPeople::fromString( value );
	}
	else
	{
		nlwarning("<CStaticGameBrick::readGeorges> can't get the value 'Learning.Race' for sheet %s", sheetId.toString().c_str() );
	}

	// Price
	if( ! root.getValueByName( Price, "Learning.Price", UFormElm::Formula ) )
	{
		nlwarning("<CStaticGameBrick::readGeorges> can't get the value 'Learning.Price' for sheet %s", sheetId.toString().c_str() );
	}

	// Skill used
	if( root.getValueByName( value, "Basics.Skill" ) )
	{
		Skill = SKILLS::toSkill( value );
		if( Skill == SKILLS::unknown )
		{
			nlwarning("<CStaticGameBrick::readGeorges> Skill unknown for brick %s", sheetId.toString().c_str() );
		}
	}
	else
	{
		nlwarning("<CStaticGameBrick::readGeorges> can't get the value 'Basic.Skill' for sheet %s", sheetId.toString().c_str() );
		Skill = SKILLS::unknown;
	}
	
	const UFormElm *array = NULL;
	// mandatory families
	if (root.getNodeByName (&array, "MandatoryFamilies") && array)
	{
		 // Get an array size
		uint size;
		array->getArraySize (size);

		// Get a array value
		for (uint i=0; i<size; ++i)
		{
			uint16 code;
			std::string value;
			array->getArrayValue (value, i);
			code = BRICK_FAMILIES::toSBrickFamily( value );
			MandatoryFamilies.push_back( code );
		}
	}
	
	// optional families
	array = NULL;
	if (root.getNodeByName (&array, "OptionalFamilies") && array)
	{
		 // Get an array size
		uint size;
		array->getArraySize (size);

		// Get an array value
		for (uint i=0; i< size; ++i)
		{
			uint16 code;
			std::string value;
			array->getArrayValue (value, i);
			code = BRICK_FAMILIES::toSBrickFamily( value );
			OptionalFamilies.push_back( code );
		}
	}

	SheetId = sheetId;
//	_Bricks.insert( make_pair( make_pair(FamilyId,IndexInFamily), this) );
//	nlinfo("added brick %s, familyId = %u, index = %u",sheetId.toString().c_str(), FamilyId, IndexInFamily);

	std::map< std::pair<uint16, uint16>, CStaticGameBrick *>::const_iterator itFound = _Bricks.find( std::make_pair( FamilyId,IndexInFamily) );
	if ( itFound != _Bricks.end() )
	{
		if ( (*itFound).second )
			nlwarning("ALREADY ADDED brick %s, familyId = %u, index = %u in sheet %s",SheetId.toString().c_str(), FamilyId, IndexInFamily, (*itFound).second->SheetId.toString().c_str() );
		else
			nlwarning ("ALREADY ADDED brick %s, familyId = %u, index = %u : the inserted brick is a NULL pointer",SheetId.toString().c_str(), FamilyId, IndexInFamily, (*itFound).second->SheetId.toString().c_str() );
		return;
	}

	pair< map< std::pair<uint16, uint16>, CStaticGameBrick *>::iterator, bool > ret = _Bricks.insert( make_pair( make_pair(FamilyId,IndexInFamily), this) );
	if (ret.second == true)
	{
		//nlinfo("added brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), FamilyId, IndexInFamily);
		_InMap = true;
	}
	else
		nlwarning("FAILED TO INSERT brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), FamilyId, IndexInFamily);
}


//-----------------------------------------------
// serial CStaticGameBrick
//
//-----------------------------------------------
void CStaticGameBrick::serial( NLMISC::IStream &f ) throw(NLMISC::EStream)
{
	f.serial( FamilyId );
	f.serial( IndexInFamily );
	f.serial( SheetId );	
	f.serialEnum( Type );
	f.serialEnum( Race );
	f.serial( Level );
	f.serial( Price );
	f.serialEnum( Skill );

	/// the SheetId
	NLMISC::CSheetId		SheetId;
	f.serialCont( MandatoryFamilies );
	f.serialCont( OptionalFamilies );
	
	if (f.isReading() )
	{
		if ( _Bricks.find( make_pair(FamilyId,IndexInFamily) ) != _Bricks.end() )
		{
			nlwarning("ALREADY ADDED brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), FamilyId, IndexInFamily);
			return;
		}

		pair< map< std::pair<uint16, uint16>, CStaticGameBrick *>::iterator, bool > ret = _Bricks.insert( make_pair( make_pair(FamilyId,IndexInFamily), this) );
		if (ret.second == true)
		{
			//nlinfo("added brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), FamilyId, IndexInFamily);
			_InMap = true;
		}
		else
			nlwarning("FAILED TO INSERT brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), FamilyId, IndexInFamily);
	}
}


///////////////////////////////////////////////////////////////////////////
///////////////////// Static XpStagesTable form ///////////////////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet for role
void CStaticXpStagesTable::readGeorges( const CSmartPtr<UForm> &form, const CSheetId &sheetId )
{
	if( form )
	{
		UFormElm& root = form->getRootNode();

		// get pointer on array of Type stage table
		UFormElm* arrayTypeStageTable = 0;
		if( ! ( root.getNodeByName( &arrayTypeStageTable, "XpParPalier" ) && arrayTypeStageTable ) )
		{
			nlwarning("<CStaticXpStagesTable::readGeorges> can get node 'XpParPalier' in sheet %s", sheetId.toString().c_str() );
		}
		else
		{
			if( ! arrayTypeStageTable->isArray() )
			{
				nlwarning("<CStaticXpStagesTable::readGeorges> node 'XpParPalier' is not an array in sheet %s", sheetId.toString().c_str() );
			}
			else
			{
				uint size;
				arrayTypeStageTable->getArraySize(size);

				XpStagesTables.resize( size );

				for( uint i = 0; i < size; ++i )
				{
					// for each Type of stage xp table 
					UFormElm* TypeElt = 0;
					if( ! ( arrayTypeStageTable->getArrayNode( &TypeElt, i ) && TypeElt ) )
					{
						nlwarning("<CStaticXpStagesTable::readGeorges> node 'Type' is not in sheet %s", sheetId.toString().c_str() );
					}
					else
					{
						// for each Type of stage xp table 
						UFormElm* TypeStageElt = 0;
						if( ! ( TypeElt->getNodeByName( &TypeStageElt, "Type" ) && TypeStageElt ) )
						{
							nlwarning("<CStaticXpStagesTable::readGeorges> no array node type under 'Type' in sheet %s", sheetId.toString().c_str() );
						}
						else
						{
							if( ! TypeStageElt->isArray() )
							{
								nlwarning("<CStaticXpStagesTable::readGeorges> node Type Stage is not an array in sheet %s", sheetId.toString().c_str() );
							}
							else
							{
								uint size2;
								TypeStageElt->getArraySize(size2);
								
								XpStagesTables [ i ].StageTable.resize( size2);
								
								for( uint j = 0; j < size2; ++j )
								{
									// for each Type of stage xp table 
									UFormElm* StageElt;
									
									if( ! ( TypeStageElt->getArrayNode( &StageElt, j ) && StageElt ) )
									{
										nlwarning("<CStaticXpStagesTable::readGeorges> node Element of stage is not in sheet %s", sheetId.toString().c_str() );
									}
									else
									{
										if( ! StageElt->getValueByName( XpStagesTables[i].StageTable[j].SkillLevel, "SkillLevel" ) )
										{
											nlwarning("<CStaticXpStagesTable::readGeorges> can't get value for element 'SkillLevel' in sheet %s", sheetId.toString().c_str() );
										}
										if( ! StageElt->getValueByName( XpStagesTables[i].StageTable[j].XpForPointSkill, "XpPerPointSkill" ) )
										{
											nlwarning("<CStaticXpStagesTable::readGeorges> can't get value for element 'XpPerPointSkill' in sheet %s", sheetId.toString().c_str() );
										}
										if( ! StageElt->getValueByName( XpStagesTables[i].StageTable[j].SpPointMultiplier, "SpPointMultiplier" ) )
										{
											nlwarning("<CStaticXpStagesTable::readGeorges> can't get value for element 'SpPointMultiplier' in sheet %s", sheetId.toString().c_str() );
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// Serial SXpStage structure
void CStaticXpStagesTable::SXpStage::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial( SkillLevel );
	f.serial( XpForPointSkill );
	f.serial( SpPointMultiplier );
}

// Serial SStageTable structure
void CStaticXpStagesTable::SStageTable::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont( StageTable );
}

// Serial XpStagesTables structure
void CStaticXpStagesTable::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont( XpStagesTables );
}

// return a reference on Xp stage corresponding to level and stage table
const CStaticXpStagesTable::SXpStage* CStaticXpStagesTable::getXpStage( uint32 level, uint16 stage ) const
{
	nlassert( stage < XpStagesTables.size() );
	vector< SXpStage >::const_iterator it = XpStagesTables[ stage ].StageTable.begin();
	vector< SXpStage >::const_iterator itLast = XpStagesTables[ stage ].StageTable.end();
	while( it != XpStagesTables[ stage ].StageTable.end() )
	{
		if( (*it).SkillLevel >= level )
		{
			return &(*it);
		}
		itLast = it;
		++it;
	}
	return &(*itLast);
}

// reload
void CStaticXpStagesTable::reloadSheet(const CStaticXpStagesTable &o)
{
	this->operator=(o);
}

///////////////////////////////////////////////////////////////////////////
//////// Static form for stage type table associated to skill /////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet for role
void CStaticStagesTypeSkillTable::readGeorges( const CSmartPtr<UForm> &form, const CSheetId &sheetId )
{
//	nlinfo ("<CStaticStagesTypeSkillTable::readGeorges> Ajoute la fiche %s %d", sheetId.toString().c_str(), sheetId.asInt());
	// load sheet

	if( form )
	{
		UFormElm& root = form->getRootNode();

		// get pointer on array
		UFormElm* arraySkillStageType = 0;
		if( ! ( root.getNodeByName( &arraySkillStageType, "SkillStageTable" ) && arraySkillStageType ) )
		{
			nlwarning("<CStaticStagesTypeSkillTable::readGeorges> can get node 'SkillStageTable' in sheet %s", sheetId.toString().c_str() );
		}
		else
		{
			if( ! arraySkillStageType->isArray() )
			{
				nlwarning("<CStaticStagesTypeSkillTable::readGeorges> node 'SkillStageTable' in sheet %s is not an array", sheetId.toString().c_str() );
			}
			else
			{
				uint size;
				arraySkillStageType->getArraySize(size);

				string SkillName;
				
				SSkillStageTypeAndCoeff sstc;

				for( uint i = 0; i < size; ++i )
				{
					UFormElm* Elt;
					
					if( ! ( arraySkillStageType->getArrayNode( &Elt, i ) && Elt ) )
					{
						nlwarning("<CStaticStagesTypeSkillTable::readGeorges> array node is not in sheet %s", sheetId.toString().c_str() );
					}
					else
					{
						if( ! Elt->getValueByName( SkillName, "Skill" ) )
						{
							nlwarning("<CStaticStagesTypeSkillTable::readGeorges> can't get value for atom 'Skill' in sheet %s", sheetId.toString().c_str() );
						}
						else if( ! Elt->getValueByName( sstc.StageType, "Type of Stage" ) )
						{
							nlwarning("<CStaticStagesTypeSkillTable::readGeorges> can't get value for atom 'Type of Stage' in sheet %s", sheetId.toString().c_str() );
						}
						else if( ! Elt->getValueByName( sstc.Coeff, "TrainingGainCoef" ) )
						{
							nlwarning("<CStaticStagesTypeSkillTable::readGeorges> can't get value for atom 'Type of Stage' in sheet %s", sheetId.toString().c_str() );
						}
						else
						{
							SkillToStageType.insert( make_pair( SkillName, sstc ) );
						}
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////
///////////// Static form for Pact lose infos //////////////////
////////////////////////////////////////////////////////////////

/// read the sheet
void CStaticPacts::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
//	nlinfo ("<CStaticPacts::readGeorges> Ajoute la fiche %s %d", sheetId.toString().c_str(), sheetId.asInt());
	// load sheet

	if( form )
	{
		UFormElm& root = form->getRootNode();

		UFormElm *arrayDeathImpact = NULL;
		if( root.getNodeByName( &arrayDeathImpact, "death_impact" ) )
		{
			if( arrayDeathImpact )
			{
				uint size;
				nlverify( arrayDeathImpact->getArraySize(size) );
				PactLose.resize( size );

				UFormElm *node;

				NLMISC::TGameCycle CumulDuration = 0;
				float duration;
					
				// variable is used for calculate pact effect in differential between pacts type
				sint16 LoseHitPoints = 0;
				sint16 LoseStamina = 0;
				sint16 LoseSap = 0;
				sint16 LoseSkills = 0;

				sint16 value;
				
				for( uint i = 0; i < size; ++i )
				{
					node = NULL;
					arrayDeathImpact->getArrayNode( &node, i );
					
					if( node )
					{
						node->getValueByName( value, "HitPoints" );
						value = 0 - value - LoseHitPoints;
						LoseHitPoints += value;
						PactLose[ i ].LoseHitPointsLevel = value;

						node->getValueByName( value, "Stamina" );
						value = 0 - value - LoseStamina;
						LoseStamina += value;
						PactLose[ i ].LoseStaminaLevel = value;

						node->getValueByName( value, "Sap" );
						value = 0 - value - LoseSap;
						LoseSap += value;
						PactLose[ i ].LoseSapLevel = value;

						node->getValueByName( value, "Skills" );
						value = 0 - value - LoseSkills;
						LoseSkills += value;
						PactLose[ i ].LoseSkillsLevel = value;

						node->getValueByName( duration, "Duration" );
						double step = CTickEventHandler::getGameTimeStep();
						CumulDuration += (NLMISC::TGameCycle) (duration / step );
						PactLose[ i ].Duration = CumulDuration;
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////// Static Creatures ////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet
void CStaticCreatures::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
//	nlinfo ("<CStaticCreatures::readGeorges> Ajoute la fiche %s %d", sheetId.toString().c_str(), sheetId.asInt());
	// load sheet

	if( form )
	{
		UFormElm& root = form->getRootNode();

		// read harvest structure
		loadFromGeorges( *form, sheetId );

		string value;

		///////////////////////////////////////////////////////
		// Race
		///////////////////////////////////////////////////////
		root.getValueByName( value, "Basics.Race" );
		if (value.empty())
			_Race = EGSPD::CPeople::Unknown;
		else
		{
			_Race = EGSPD::CPeople::fromString( value );
			if (EGSPD::CPeople::toString(_Race) != value)
			{
				nlwarning("Error while reading race '%s', the race is set to '%s'",
					value.c_str(),
					EGSPD::CPeople::toString(_Race).c_str());
			}
		}

		///////////////////////////////////////////////////////
		// Gender
		///////////////////////////////////////////////////////
		root.getValueByName( _Gender, "Basics.Gender" );

		///////////////////////////////////////////////////////
		// Ecosystem
		///////////////////////////////////////////////////////
		root.getValueByName( value, "Basics.Ecosystem" );
		_Ecosystem = ECOSYSTEM::stringToEcosystem(value);

		///////////////////////////////////////////////////////
		// Creature Size
		///////////////////////////////////////////////////////
		root.getValueByName( value, "Basics.Size" );
		_Size = CREATURE_SIZE::stringToCreatureSize( value );

		///////////////////////////////////////////////////////
		// Level
		///////////////////////////////////////////////////////
		root.getValueByName( _Level, "creature_level" );

		///////////////////////////////////////////////////////
		// Caracteristics
		///////////////////////////////////////////////////////
		int i;
		for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
		{
			root.getValueByName( _Characteristics[ (CHARACTERISTICS::TCharacteristics) i ], ( string("Basics.Characteristics.") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i ) ).c_str() );
		}

		///////////////////////////////////////////////////////
		// Creature levels
		///////////////////////////////////////////////////////
		root.getValueByName( _AttackLevel, "Basics.AttackLevel" );
		root.getValueByName( _DefenseLevel, "Basics.DefenseLevel" );
		root.getValueByName( _XPLevel, "Basics.XPLevel" );
		root.getValueByName( _TauntLevel, "Basics.TauntLevel" );

		if ( !root.getValueByName( _XPGainOnCreature, "Basics.XPGainOnCreature" ) )
		{
			_XPGainOnCreature = 10;
			nlwarning("ERROR For creature sheet %s, cannot read Basics.XPGainOnCreature, XP may be inacurate !", sheetId.toString().c_str() );
		}
	

		///////////////////////////////////////////////////////
		// Derivated Scores
		///////////////////////////////////////////////////////
		uint32 playerSkillLevel = 1;
		if ( !root.getValueByName( playerSkillLevel, "Basics.PlayerSkillLevel" ) )
		{
			nlwarning("ERROR For creature sheet %s, cannot read Basics.PlayerSkillLevel, creature HP may be inacurate !", sheetId.toString().c_str() );
		}
		_NbPlayers = 1;
		if ( !root.getValueByName( _NbPlayers, "Basics.NbPlayers" ) )
		{
			nlwarning("ERROR For creature sheet %s, cannot read Basics.NbPlayers, creature HP may be inacurate !", sheetId.toString().c_str() );
		}

		sint32 hp;
		root.getValueByName( hp, "Basics.life" );
		float regen;
		root.getValueByName( regen, "Basics.LifeRegen" );

		if (hp == 0)
		{
			nlwarning("CStaticCreatures::readGeorges : spawning a creature with 0 Hit Points ! (SHEET %s), set the hp to 100", sheetId.toString().c_str());
			hp = 100;
		}

		for( i = 0; i < SCORES::NUM_SCORES; ++i )
		{
			 _Scores[ i ] = hp;
			 _Regen[ i ] = regen;
		}

		///////////////////////////////////////////////////////
		// Creature Damage Per Hit
		///////////////////////////////////////////////////////
		_PlayerHpLevel = 1;
		_NbHitToKillPlayer = 10.0f;
		if ( !root.getValueByName( _PlayerHpLevel, "Basics.PlayerHpLevel" ) )
		{
			nlwarning("ERROR For creature sheet %s, cannot read Basics.PlayerHpLevel, creature damage may be inacurate !", sheetId.toString().c_str() );
		}
		if ( !root.getValueByName( _NbHitToKillPlayer, "Basics.NbHitToKillPlayer" ) )
		{
			nlwarning("ERROR For creature sheet %s, cannot read Basics.NbHitToKillPlayer, creature damage may be inacurate !", sheetId.toString().c_str() );
		}

		if ( _NbHitToKillPlayer == 0.0f)
		{
			nlwarning("ERROR For creature sheet %s, Basics.NbHitToKillPlayer == 0, creature damage may be inacurate !", sheetId.toString().c_str() );
			_NbHitToKillPlayer = 3.0f;
		}
		
		_CreatureDamagePerHitWithoutAverageDodge = uint32( (100*_PlayerHpLevel) / _NbHitToKillPlayer );
		compileCreatureDamagePerHit();


		///////////////////////////////////////////////////////
		// Creature AttackLatency
		///////////////////////////////////////////////////////
		float latencyInSeconds;
		if ( root.getValueByName( latencyInSeconds, "Basics.AttackSpeed" ))
		{
			_AttackLatency = NLMISC::TGameCycle( (float)latencyInSeconds / CTickEventHandler::getGameTimeStep() );
			if (_AttackLatency < 10)
			{
				nlwarning("For creature sheet %s, AttackLatency = %u!", sheetId.toString().c_str(), (uint)_AttackLatency );
			}
		}
		else
		{
			nlwarning("ERROR For creature sheet %s, cannot read Basics.AttackSpeed, set it to 30 ticks!", sheetId.toString().c_str() );
			_AttackLatency = 30;
		}

		///////////////////////////////////////////////////////
		// Creature defense mode
		///////////////////////////////////////////////////////
		_DodgeAsDefense = true;
		if( ! root.getValueByName ( _DodgeAsDefense, "Basics.DodgeAsDefense") )
		{
			nlwarning( "<CStaticCreatures::readGeorges> can't get the value 'Basics.DodgeAsDefense' in sheet %s", sheetId.toString().c_str() );
		}

		///////////////////////////////////////////////////////
		// "Allonge" - the melee range value
		///////////////////////////////////////////////////////
		root.getValueByName( _MeleeReachValue, "Basics.MeleeReachValue" );

		///////////////////////////////////////////////////////
		// Items
		///////////////////////////////////////////////////////
		string sheetName;
		CSheetId sheet;

		for( i = 0; i < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++i )
		{
			root.getValueByName( sheetName, (string("Basics.Equipment.") + SLOT_EQUIPMENT::toString( (SLOT_EQUIPMENT::TSlotEquipment) i ) + string(".Item" )).c_str() );
			if( sheetName != string("") && sheetName.find(".item") != std::string::npos )
			{
				sheet = CSheetId( sheetName );
				_Items[ i ].IdSheet = sheet.asInt();
				root.getValueByName( _Items[ i ].Quality, (string("Basics.Equipment.") + SLOT_EQUIPMENT::toString( (SLOT_EQUIPMENT::TSlotEquipment) i ) + string(".Quality")).c_str() );
			}
			else
			{
				_Items[ i ].IdSheet = 0;
				_Items[ i ].Quality = 0;
			}
		}

		///////////////////////////////////////////////////////
		// Loot tables
		///////////////////////////////////////////////////////
		const UFormElm * lootTableArray = 0;
        if (root.getNodeByName (&lootTableArray, "Loot.Loot Tables") && lootTableArray)
		{
			// Get array size
			uint size;
			lootTableArray->getArraySize (size);

			_LootTables.resize(size);
			
			// Get a array value
			for (uint i=0; i<size; ++i)
			{
				lootTableArray->getArrayValue( _LootTables[i], i );
			}
		}
				
		///////////////////////////////////////////////////////
		// bag of animal if have pack animal capabilities
		///////////////////////////////////////////////////////
		root.getValueByName( _BagInventorySheet, "animal_bag.bag" );

		///////////////////////////////////////////////////////
		// Movement speed 
		///////////////////////////////////////////////////////
		root.getValueByName( _WalkSpeed, "Basics.MovementSpeeds.WalkSpeed" );
		root.getValueByName( _RunSpeed, "Basics.MovementSpeeds.RunSpeed" );

		///////////////////////////////////////////////////////
		// Creature properties (autorised actions)
		///////////////////////////////////////////////////////
		bool b;
		// Is the character Selectable ?
		root.getValueByName( b, "Properties.Selectable" );
		_Properties.selectable( b );
		// Is the character Talkable ?
		root.getValueByName( b, "Properties.Talkable" );
		_Properties.talkableTo( b );
		// Is the character Attackable ?
		root.getValueByName( b, "Properties.Attackable" );
		_Properties.attackable( b );
		// Is the character Givable ?
		root.getValueByName( b, "Properties.Givable" );
		_Properties.givable( b );
		// Is the creature is Harvestable/Lootable
		root.getValueByName( value, "Properties.LootHarvestState" );
		switch( LHSTATE::stringToLHState( value ) )
		{
			case LHSTATE::LOOTABLE:
				_Properties.lootable( true );
				_Properties.harvestable( false );
				break;
			case LHSTATE::HARVESTABLE:
				_Properties.lootable( false );
				_Properties.harvestable( true );
				break;
			case LHSTATE::LOOTABLE_HARVESTABLE:
				_Properties.lootable( true );
				_Properties.harvestable( true );
				break;
			case LHSTATE::NONE:
			default:
				_Properties.lootable( false );
				_Properties.harvestable( false );
				break;
		}
		// Is the character Mountable ?
		root.getValueByName( b, "Properties.Mountable");
		_Properties.mountable( b );

		// other properties
		_Properties.usable( false );
		_Properties.liftable( false );
		_Properties.lookableAt( false );
		_Properties.invitable( false );
		_Properties.canExchangeItem( false );

		// Xp coeff
//		root.getValueByName( XpCoeff, "Properties.XPGainCoef" );

		// protections
		root.getValueByName( _Protections[(uint)DMGTYPE::PIERCING].Max, "Protections.PiercingMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::PIERCING].Factor, "Protections.PiercingFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::SLASHING].Max, "Protections.SlashingMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::SLASHING].Factor, "Protections.SlashingFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::BLUNT].Max, "Protections.BluntMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::BLUNT].Factor, "Protections.BluntFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::ROT].Max, "Protections.RotMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::ROT].Factor, "Protections.RotFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::ACID].Max, "Protections.AcidMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::ACID].Factor, "Protections.AcidFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::COLD].Max, "Protections.ColdMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::COLD].Factor, "Protections.ColdFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::FIRE].Max, "Protections.FireMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::FIRE].Factor, "Protections.FireFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::POISON].Max, "Protections.PoisonMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::POISON].Factor, "Protections.PoisonFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::ELECTRICITY].Max, "Protections.ElectricityMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::ELECTRICITY].Factor, "Protections.ElectricityFactor" );
		root.getValueByName( _Protections[(uint)DMGTYPE::SHOCK].Max, "Protections.ShockMax" );
		root.getValueByName( _Protections[(uint)DMGTYPE::SHOCK].Factor, "Protections.ShockFactor" );

		// all factors must be divided by 100.0f as 12% is written 12 in sheets
		for (uint i = 0 ; i < _Protections.size() ; ++i)
			_Protections[i].Factor /= 100.0f;

		// resists
		root.getValueByName( _Resists.Fear , "Resists.Fear");
		root.getValueByName( _Resists.Sleep, "Resists.Sleep");
		root.getValueByName( _Resists.Stun, "Resists.Stun");
		root.getValueByName( _Resists.Root, "Resists.Root");
		root.getValueByName( _Resists.Snare, "Resists.Snare");
		root.getValueByName( _Resists.Slow, "Resists.Slow");
		root.getValueByName( _Resists.Madness, "Resists.Madness");
		root.getValueByName( _Resists.Blind, "Resists.Blind");
		root.getValueByName( _Resists.Acid, "Resists.Acid");
		root.getValueByName( _Resists.Cold, "Resists.Cold");
		root.getValueByName( _Resists.Electricity, "Resists.Electricity");
		root.getValueByName( _Resists.Fire, "Resists.Fire");
		root.getValueByName( _Resists.Poison, "Resists.Poison");
		root.getValueByName( _Resists.Rot, "Resists.Rot");
		root.getValueByName( _Resists.Shockwave, "Resists.Shockwave");

		// resist >= 10000 then set it to immune score
		if ( _Resists.Fear >= 10000)
			_Resists.Fear = CCreatureResists::ImmuneScore;
		if ( _Resists.Sleep >= 10000)
			_Resists.Sleep = CCreatureResists::ImmuneScore;
		if ( _Resists.Stun >= 10000)
			_Resists.Stun = CCreatureResists::ImmuneScore;
		if ( _Resists.Root >= 10000)
			_Resists.Root = CCreatureResists::ImmuneScore;
		if ( _Resists.Snare >= 10000)
			_Resists.Snare = CCreatureResists::ImmuneScore;
		if ( _Resists.Slow >= 10000)
			_Resists.Slow = CCreatureResists::ImmuneScore;
		if ( _Resists.Madness >= 10000)
			_Resists.Madness = CCreatureResists::ImmuneScore;
		if ( _Resists.Blind >= 10000)
			_Resists.Blind = CCreatureResists::ImmuneScore;
		if ( _Resists.Acid >= 10000)
			_Resists.Acid = CCreatureResists::ImmuneScore;
		if ( _Resists.Cold >= 10000)
			_Resists.Cold = CCreatureResists::ImmuneScore;
		if ( _Resists.Electricity >= 10000)
			_Resists.Electricity = CCreatureResists::ImmuneScore;
		if ( _Resists.Fire >= 10000)
			_Resists.Fire = CCreatureResists::ImmuneScore;
		if ( _Resists.Poison >= 10000)
			_Resists.Poison = CCreatureResists::ImmuneScore;
		if ( _Resists.Rot >= 10000)
			_Resists.Rot = CCreatureResists::ImmuneScore;
		if ( _Resists.Shockwave >= 10000)
			_Resists.Shockwave = CCreatureResists::ImmuneScore;
		
		// damage shield
		_DamageShieldDamage = 0;
		root.getValueByName( _DamageShieldDamage, "Damage Shield.Damage");
		_DamageShieldHpDrain = 0;
		root.getValueByName( _DamageShieldHpDrain, "Damage Shield.Drained HP");

		// action on death
		if ( root.getValueByName(value, "ActionOnDeath") )
		{
			_ActionOnDeath = value;
		}

		// Fame
		string s;
		if (root.getValueByName( s, "Basics.Fame"))
			_Faction = CStaticFames::getInstance().getFactionIndex(s);
		else
			_Faction = CStaticFames::INVALID_FACTION_INDEX;
		_FameByKillValid = false;
		_FameByKill = 0;
		if (root.getValueByName( s, "Basics.FameByKill") && !s.empty())
		{
			_FameByKillValid = true;
			NLMISC::fromString(s, _FameByKill);
		}
		// Get the entity collision radius
		if(root.getValueByName(_ColRadius, "Collision.CollisionRadius") == false)
			nlwarning("CStaticCreatures:readGeorges: cannot get the value from the key 'Collision.CollisionRadius' for the sheet '%u(%s)'.", sheetId.asInt(), sheetId.toString().c_str());
		root.getValueByName(_ColLength, "Collision.Length");
		root.getValueByName(_ColWidth, "Collision.Width");

		// character scale
		if(root.getValueByName(_Scale, "3d data.Scale") == false)
			nlwarning("Key '3d data.Scale' not found.");
		else
		{
			if(_Scale <= 0.0f)
			{
				nlwarning("CStaticCreatures::readGeorges> Scale(%f) <= 0.0 so fix scale to 1.0", _Scale);
				_Scale = 1.0f;
			}
		}
	}
}// CStaticCreatures::readGeorges //

///////////////////////////////////////////////////////////////////////////
uint CStaticCreatures::getVersion()
{
	return 50 + ( SKILLS::NUM_SKILLS << 16 );
}

///////////////////////////////////////////////////////////////////////////
void CStaticCreatures::serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
{
	CStaticHarvestable::serial(f);
	
	f.serialEnum( _Race );
	f.serial( _Gender );
	f.serial( _Size );
	f.serial( _Level );
	f.serial( _AttackLevel );
	f.serial( _DefenseLevel );
	f.serial( _XPLevel );
	f.serial( _XPGainOnCreature );
	f.serial( _TauntLevel );
	f.serial( _NbPlayers );
	for( int c = 0; c < CHARACTERISTICS::NUM_CHARACTERISTICS; ++c )
	{
		f.serial( _Characteristics[ c ] );
	}
	for( int sc = 0; sc < SCORES::NUM_SCORES; ++sc )
	{
		f.serial( _Scores[ sc ] );
		f.serial( _Regen[ sc ] );
	}
	for( int s = 0; s < SKILLS::NUM_SKILLS; ++s )
	{
		f.serial( _Skills[ s ] );
	}
	for( int e = 0; e < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++e )
	{
		f.serial( _Items[ e ] );
	}
	
	f.serialCont( _LootTables );
	f.serial( _MeleeReachValue );
	f.serial( _WalkSpeed );
	f.serial( _RunSpeed );
	f.serial( _Properties );
	f.serial( _CreatureDamagePerHitWithoutAverageDodge );
	f.serial( _CreatureDamagePerHit );
	f.serial( _AttackLatency );
	f.serialCont(_Protections);
	f.serial(_Faction);
	f.serial(_FameByKillValid);
	f.serial(_FameByKill);
	f.serial(_ColRadius);
	f.serial(_Scale);
	f.serial(_ColLength);
	f.serial(_ColWidth);

	f.serial(_Resists);
	f.serial(_ActionOnDeath);
	f.serial(_DodgeAsDefense);
	
	if (f.isReading())
	{
		std::string eco;
		f.serial(eco);
		_Ecosystem = ECOSYSTEM::stringToEcosystem(eco);
	}
	else
	{
		std::string eco = ECOSYSTEM::toString(_Ecosystem);
		f.serial(eco);
	}
	
	f.serial( _BagInventorySheet );
	
	f.serial(_DamageShieldDamage);
	f.serial(_DamageShieldHpDrain);
}

///////////////////////////////////////////////////////////////////////////
void CStaticCreatures::reloadSheet(const CStaticCreatures &o)
{
	// nothing special
	*this= o;
}

///////////////////////////////////////////////////////////////////////////
void CStaticCreatures::compileCreatureDamagePerHit()
{
	// NB yoyo: i think this is made for backward compatibility (make creature as powerfull as before dodge feature)
	if ( CStaticSuccessTable::getAverageDodgeFactor() < 1.0f )
		_CreatureDamagePerHit = (uint32) ( getCreatureDamagePerHitWithoutAverageDodge() / (1.0f - CStaticSuccessTable::getAverageDodgeFactor()) );
	else
		_CreatureDamagePerHit = getCreatureDamagePerHitWithoutAverageDodge();
}

/************************************************************************/
/* Class containing attributes map used for applyUserModel				*/
/************************************************************************/

typedef std::map<std::string, TAttributeType> TStaticAttributeMap;

class CAttributeMapping
{
public:
	CAttributeMapping();
	
	TAttributeType find(const std::string &key) const;

	TStaticAttributeMap _StaticAttributeMap;
};

CAttributeMapping::CAttributeMapping()
{	
	//BASICS
	_StaticAttributeMap.insert(make_pair(std::string("basics.race"),				at_race));
	_StaticAttributeMap.insert(make_pair(std::string("basics.gender"),				at_gender));
	_StaticAttributeMap.insert(make_pair(std::string("basics.size"),				at_size));
	_StaticAttributeMap.insert(make_pair(std::string("basics.level"),				at_level));
	_StaticAttributeMap.insert(make_pair(std::string("basics.playerskilllevel"),	at_player_skill_level));
	_StaticAttributeMap.insert(make_pair(std::string("basics.nbplayers"),			at_nb_players));
	_StaticAttributeMap.insert(make_pair(std::string("basics.playerhplevel"),		at_player_hp_level));
	_StaticAttributeMap.insert(make_pair(std::string("basics.nbhittokillplayer"),	at_nb_hit_to_kill_player));
	_StaticAttributeMap.insert(make_pair(std::string("basics.ecosystem"),			at_ecosystem));
	_StaticAttributeMap.insert(make_pair(std::string("basics.type"),				at_type));
	_StaticAttributeMap.insert(make_pair(std::string("basics.fame"),				at_fame));
	_StaticAttributeMap.insert(make_pair(std::string("basics.famebykill"),			at_fame_by_kill));
	_StaticAttributeMap.insert(make_pair(std::string("basics.life"),				at_life));
	_StaticAttributeMap.insert(make_pair(std::string("basics.liferegen"),			at_liferegen));
	_StaticAttributeMap.insert(make_pair(std::string("basics.attackspeed"),			at_attack_speed ));
	_StaticAttributeMap.insert(make_pair(std::string("basics.attacklevel"),			at_attack_level));
	_StaticAttributeMap.insert(make_pair(std::string("basics.defenselevel"),		at_defense_level ));
	_StaticAttributeMap.insert(make_pair(std::string("basics.xplevel"),				at_xp_level));
	_StaticAttributeMap.insert(make_pair(std::string("basics.tauntlevel"),			at_taunt_level));
	_StaticAttributeMap.insert(make_pair(std::string("basics.meleereachvalue"),		at_melee_reach_value));
	_StaticAttributeMap.insert(make_pair(std::string("basics.xpgainoncreature"),	at_xp_gain_on_creature));
	_StaticAttributeMap.insert(make_pair(std::string("basics.localcode"),			at_local_code));
	_StaticAttributeMap.insert(make_pair(std::string("basics.dodgeasdefense"),		at_dodge_as_defense));
	_StaticAttributeMap.insert(make_pair(std::string("basics.walkspeed"),			at_walk_speed));
	_StaticAttributeMap.insert(make_pair(std::string("basics.runspeed"),			at_run_speed));
	_StaticAttributeMap.insert(make_pair(std::string("basics.attackable"),			at_attackable));
	_StaticAttributeMap.insert(make_pair(std::string("basics.selectable"),			at_selectable));
	_StaticAttributeMap.insert(make_pair(std::string("basics.lhstate"),				at_lhstate));
	
	//PROTECTIONS
	_StaticAttributeMap.insert(make_pair(std::string("protections.piercing"),			at_protect_piercing));
	_StaticAttributeMap.insert(make_pair(std::string("protections.slashing"),			at_protect_slashing));
	_StaticAttributeMap.insert(make_pair(std::string("protections.blunt"),				at_protect_blunt));
	_StaticAttributeMap.insert(make_pair(std::string("protections.rot"),				at_protect_rot));
	_StaticAttributeMap.insert(make_pair(std::string("protections.acid"),				at_protect_acid));
	_StaticAttributeMap.insert(make_pair(std::string("protections.cold"),				at_protect_cold));
	_StaticAttributeMap.insert(make_pair(std::string("protections.fire"),				at_protect_fire));
	_StaticAttributeMap.insert(make_pair(std::string("protections.poison"),				at_protect_poison));
	_StaticAttributeMap.insert(make_pair(std::string("protections.electricity"),		at_protect_electricity));
	_StaticAttributeMap.insert(make_pair(std::string("protections.shock"),				at_protect_shock));
	
	//RESISTS
	_StaticAttributeMap.insert(make_pair(std::string("resists.fear"),			at_resists_fear));
	_StaticAttributeMap.insert(make_pair(std::string("resists.sleep"),			at_resists_sleep));
	_StaticAttributeMap.insert(make_pair(std::string("resists.stun"),			at_resists_stun));
	_StaticAttributeMap.insert(make_pair(std::string("resists.root"),			at_resists_root));
	_StaticAttributeMap.insert(make_pair(std::string("resists.snare"),			at_resists_snare));
	_StaticAttributeMap.insert(make_pair(std::string("resists.slow"),			at_resists_slow));
	_StaticAttributeMap.insert(make_pair(std::string("resists.madness"),		at_resists_madness));
	_StaticAttributeMap.insert(make_pair(std::string("resists.blind"),			at_resists_blind));
	_StaticAttributeMap.insert(make_pair(std::string("resists.acid"),			at_resists_acid));
	_StaticAttributeMap.insert(make_pair(std::string("resists.cold"),			at_resists_cold));
	_StaticAttributeMap.insert(make_pair(std::string("resists.electricity"),	at_resists_electricity));
	_StaticAttributeMap.insert(make_pair(std::string("resists.fire"),			at_resists_fire));
	_StaticAttributeMap.insert(make_pair(std::string("resists.poison"),			at_resists_poison));
	_StaticAttributeMap.insert(make_pair(std::string("resists.rot"),			at_resists_rot));
	_StaticAttributeMap.insert(make_pair(std::string("resists.shockwave"),		at_resists_shockwave));


}

TAttributeType CAttributeMapping::find(const std::string &key) const
{
	TStaticAttributeMap::const_iterator it = _StaticAttributeMap.find(key);
	if (it == _StaticAttributeMap.end())
		return at_unknown;

	return it->second;
}	

bool CStaticCreatures::applyProtectModification(uint index, const std::string &attr, const std::string &newValue)
{
	//Set protection factor
	if(attr == "factor")
	{
		char *ptr = NULL;
		float factor = static_cast<float>(strtod(newValue.c_str(), &ptr));
		if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
		{
			// all factors must be divided by 100.0f as 12% is written 12 in sheets
			//for (uint i = 0 ; i < _Protections.size() ; ++i)
			//_Protections[i].Factor /= 100.0f;
			_Protections[index].Factor = factor/100.0f;
			return false;
		}
		return true;
	}

	//Set protection max
	if (attr == "max")
	{
		uint16 max;
		NLMISC::fromString(newValue, max);
		_Protections[index].Max = max;
		return false;
	}
	return true;
	
}
/************************************************************************/
/* Apply attributes customization  defined in a UserModel				*/
/************************************************************************/
//bool CStaticCreatures::applyUserModel(const std::string &modelId, const std::vector<std::string> &scriptData)
bool CStaticCreatures::applyUserModel(CCustomElementId userModelId, const std::vector<std::string> &scriptData)
{
	bool errors = false;
	std::vector<std::string> scriptLine;
	std::string modelId = userModelId.Id;
	//uint32 primAlias = userModel.PrimAlias;

	CAttributeMapping attributeMap = CAttributeMapping();
	for (uint32 i = 1; i < scriptData.size(); ++i)
	{
		scriptLine.clear();	
		splitString(scriptData[i], " ", scriptLine);
		
		if (scriptLine.size() < 2
			|| toLower(scriptLine[0]) == "protect" && scriptLine.size() < 3)
		{
			nlwarning("<CStaticCreatures::applyUserModel> error while reading a script line (uncommented line %i), ignoring it.", i);
			errors = true;
			continue;
		}

		switch (attributeMap.find(toLower(scriptLine[0])))
		{

			//FIXME: test attributes value before applying, if error set to default value and errors=true



			/************************************************************************/
			/*					         BASICS                                     */
			/************************************************************************/
			case at_race :
			{		 
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting race to '%s'", modelId.c_str(), scriptLine[1].c_str());
				_Race = EGSPD::CPeople::fromString( scriptLine[1] );
				if (EGSPD::CPeople::toString(_Race) != scriptLine[1])
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s' : cannot read race '%s', the race is set to '%s'",
						modelId.c_str(), scriptLine[1].c_str(), EGSPD::CPeople::toString(_Race).c_str());
					errors = true;
				}				
				break;
			}
			case at_gender :
			{
				NLMISC::fromString(scriptLine[1], _Gender);
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting Gender to %u", modelId.c_str(), _Gender);
				break;
			}

			case at_size :
			{
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting size to '%s'", modelId.c_str(), scriptLine[1].c_str());
				_Size = CREATURE_SIZE::stringToCreatureSize( scriptLine[1] );
				break;
			}
			case at_level :
				//FIXME
				break;

			case at_player_skill_level :
				//FIXME
				break;

			case at_nb_players :
			{	
				sint n;
				NLMISC::fromString(scriptLine[1], n);
				if (n > 255 || n < 0)
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': invalid value for attribute 'NbPlayers', setting 'NbPlayers' to 1.", modelId.c_str());
					n = 1;
					errors = true;
				}
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting NbPlayers to '%s'", modelId.c_str(), scriptLine[1].c_str());
				_NbPlayers = static_cast<uint8>(n);
				break;
			}			

			case at_player_hp_level:
			{
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s': setting playerHpLevel to '%s'", modelId.c_str(), scriptLine[1].c_str());
				NLMISC::fromString(scriptLine[1], _PlayerHpLevel);
				_CreatureDamagePerHitWithoutAverageDodge = uint32( (100*_PlayerHpLevel) / _NbHitToKillPlayer );
				compileCreatureDamagePerHit();
				break;
			}
			case at_nb_hit_to_kill_player :
			{	
				char *ptr = NULL;
				float nbHits = static_cast<float>(strtod(scriptLine[1].c_str(), &ptr));
				if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
				{
					nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting NbHitToKillPlayer to '%f'", modelId.c_str(), nbHits);
					_NbHitToKillPlayer = nbHits;
				}
				else
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s' : cannot read Basics.NbHitToKillPlayer, setting it to 3.0f", modelId.c_str());
					_NbHitToKillPlayer = 3.0f;
					errors = true;
				}
								
				_CreatureDamagePerHitWithoutAverageDodge = uint32( (100*_PlayerHpLevel) / _NbHitToKillPlayer );
				compileCreatureDamagePerHit();
				break;
			}

			case at_ecosystem :
			{	
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting Ecosystem to'%s'", modelId.c_str(), scriptLine[1].c_str());
				_Ecosystem = ECOSYSTEM::stringToEcosystem(scriptLine[1]);
				break;
			}

			case at_type:
			{
				//FIXME
				break;
			}

			case at_fame:
			{
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting faction to '%s'", modelId.c_str(), scriptLine[1].c_str());
				_Faction = CStaticFames::getInstance().getFactionIndex(scriptLine[1]);
				//_Faction = CStaticFames::INVALID_FACTION_INDEX;
				break;
			}
			
			case at_fame_by_kill:
			{
				_FameByKill = 0;
				_FameByKillValid = true;
				NLMISC::fromString(scriptLine[1], _FameByKill);
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting FameByKill to '%d'", modelId.c_str(), _FameByKill);
				break;
			}

			case at_life:
			{
				sint32 hp;
				NLMISC::fromString(scriptLine[1], hp);
				
				if (hp == 0)
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s' : spawning a creature with 0 Hit Points ! set the hp to 100", modelId.c_str());
					hp = 100;
					errors = true;
				}

				nldebug("<CStaticCreatures::applyUserModel> Applying '%s': setting Hp to '%u'", modelId.c_str(), hp);

				_Scores[SCORES::hit_points] = hp;
				break;

			}

			case at_liferegen:
			{
				char *ptr = NULL;
				float lifeRegen = static_cast<float>(strtod(scriptLine[1].c_str(), &ptr));
				
				if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
				{
					_Regen[SCORES::hit_points] = lifeRegen;
					nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting lifeRegen to %f", modelId.c_str(), lifeRegen);
				}
				else
				{
					_Regen[SCORES::hit_points] = 1;
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.LifeRegen, set it to 1.0!", modelId.c_str() );
					errors = true;
				}
				break;

			}
			
			case at_attack_speed:
			{
				char *ptr = NULL;
				float latencyInSeconds = static_cast<float>(strtod(scriptLine[1].c_str(), &ptr));
				if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
				{
					_AttackLatency = NLMISC::TGameCycle( (float)latencyInSeconds / CTickEventHandler::getGameTimeStep() );
					if (_AttackLatency < 10)
					{
						nlwarning("While applying user model '%s', AttackLatency changed to %u!", modelId.c_str(), (uint)_AttackLatency );
					}
					nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting attackLatency to %f", modelId.c_str(), latencyInSeconds);
				}
				else
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.AttackSpeed, set it to 30 ticks!", modelId.c_str() );
					_AttackLatency = 30;
					errors = true;
				}
				break;
			}

			case at_attack_level:
			{
				NLMISC::fromString(scriptLine[1], _AttackLevel);
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting attackLevel to '%u'", modelId.c_str(), _AttackLevel);
				break;
			}

			case at_defense_level:
			{
				NLMISC::fromString(scriptLine[1], _DefenseLevel);
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting defenseLevel to %u", modelId.c_str(), _DefenseLevel);
				break;
			}

			case at_xp_level:
			{
				NLMISC::fromString(scriptLine[1], _XPLevel);
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting XpLevel to %u", modelId.c_str(), _XPLevel);
				break;
			}

			case at_taunt_level:
			{
				NLMISC::fromString(scriptLine[1], _TauntLevel);
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting TauntLevel to %u", modelId.c_str(), _TauntLevel);
				break;
			}

			case at_melee_reach_value:
			{
				NLMISC::fromString(scriptLine[1], _MeleeReachValue);
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting MeleeReachValue to %u", modelId.c_str(), _MeleeReachValue);
				break;
			}

			case at_xp_gain_on_creature:
			{	
				char *ptr = NULL;
				float xpGain = static_cast<float>(strtod(scriptLine[1].c_str(), &ptr));
				if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
				{
					nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting xpGainOnCreature to %f", modelId.c_str(), xpGain);
					_XPGainOnCreature = xpGain;
				}
				else
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.XPGainOnCreature, setting it to 10.0f");
					_XPGainOnCreature = 10.0f;
					errors = true;
				}
				break;
			}
			
			case at_local_code:
			{
				//used but directly from the georgesheet?
				break;
			}

			case at_dodge_as_defense:
			{	
				if (scriptLine[1] == "true" || scriptLine[1] == "1" || scriptLine[1] == "on")
				{
					_DodgeAsDefense = true;
				}
				else
				{
					if (scriptLine[1] == "false" || scriptLine[1] == "0" || scriptLine[1] == "off")
						_DodgeAsDefense = false;
					else
					{
						nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.DodgeAsDefense, setting it to 'false'");
						_DodgeAsDefense = false;
						errors = true;
						break;
					}
				}
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting DodgeAsDefense to %s", modelId.c_str(), scriptLine[1].c_str());
				break;
			}
			
			case at_walk_speed :
			{
				char *ptr = NULL;
				float walkSpeed = static_cast<float>(strtod(scriptLine[1].c_str(), &ptr));
				if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
				{
					_WalkSpeed = walkSpeed;
					nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting WalkSpeed to %f", modelId.c_str(), _WalkSpeed);
				}
				else
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.WalkSpeed, setting it to 1.66", modelId.c_str() );
					_WalkSpeed = 1.66f;
					errors = true;
				}
				break;
			}

			case at_run_speed :
			{
				char *ptr = NULL;
				float runSpeed = static_cast<float>(strtod(scriptLine[1].c_str(), &ptr));
				if (ptr != NULL && *ptr == '\0' && errno != ERANGE)
				{
					_RunSpeed = runSpeed;
					nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting RunSpeed to %f", modelId.c_str(), _RunSpeed);
				}
				else
				{
					nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.RunSpeed, setting it to 6.0", modelId.c_str() );
					_RunSpeed = 6.0;
					errors = true;
				}
				break;
			}

			case at_attackable:
			{	
				if (scriptLine[1] == "true" || scriptLine[1] == "1" || scriptLine[1] == "on")
				{
					_Properties.attackable( true );
				}
				else
				{
					if (scriptLine[1] == "false" || scriptLine[1] == "0" || scriptLine[1] == "off")
						_Properties.attackable( false );
					else
					{
						nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.Attackable, setting it to 'false'");
						_Properties.attackable( false );
						errors = true;
						break;
					}
				}
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting Attackable to %s", modelId.c_str(), scriptLine[1].c_str());
				break;
			}
			
			case at_selectable:
			{	
				if (scriptLine[1] == "true" || scriptLine[1] == "1" || scriptLine[1] == "on")
				{
					_Properties.selectable( true );
				}
				else
				{
					if (scriptLine[1] == "false" || scriptLine[1] == "0" || scriptLine[1] == "off")
						_Properties.selectable( false );
					else
					{
						nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': cannot read Basics.Selectable, setting it to 'false'");
						_Properties.selectable( false );
						errors = true;
						break;
					}
				}
				nldebug("<CStaticCreatures::applyUserModel> Applying '%s' : setting Selectable to %s", modelId.c_str(), scriptLine[1].c_str());
				break;
			}

			case at_lhstate:
			{
				switch( LHSTATE::stringToLHState( scriptLine[1] ) )
				{
					case LHSTATE::LOOTABLE:
						_Properties.lootable( true );
						_Properties.harvestable( false );
						break;
					case LHSTATE::HARVESTABLE:
						_Properties.lootable( false );
						_Properties.harvestable( true );
						break;
					case LHSTATE::LOOTABLE_HARVESTABLE:
						_Properties.lootable( true );
						_Properties.harvestable( true );
						break;
					case LHSTATE::NONE:
					default:
						_Properties.lootable( false );
						_Properties.harvestable( false );
						break;
				}	
			}

			/************************************************************************/
			/*                           PROTECT                                    */
			/************************************************************************/
			
			case at_protect_piercing :
			{
				errors = applyProtectModification((uint)DMGTYPE::PIERCING, scriptLine[1], scriptLine[2]);
				break;
			}

			case at_protect_slashing :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::SLASHING, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_blunt :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::BLUNT, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_rot :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::ROT, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_acid :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::ACID, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_cold :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::COLD, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_fire :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::FIRE, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_poison :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::POISON, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_electricity :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::ELECTRICITY, scriptLine[1], scriptLine[2] );
				break; 
			}

			case at_protect_shock :
			{ 
				errors = applyProtectModification((uint)DMGTYPE::SHOCK, scriptLine[1], scriptLine[2] );
				break; 
			}

			/************************************************************************/
			/*                          RESIST                                      */
			/************************************************************************/
			
			case at_resists_fear :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Fear = value;
				break; 
			}
			case at_resists_sleep :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Sleep = value;
				break; 
			}
			case at_resists_stun :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Stun = value;
				break; 
			}
			case at_resists_root :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Root = value;
				break; 
			}
			case at_resists_snare :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Snare = value;
				break; 
			}
			case at_resists_slow :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Slow = value;
				break; 
			}
			case at_resists_madness :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Madness = value;
				break; 
			}
			case at_resists_blind :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Blind = value;
				break; 
			}
			case at_resists_acid :
			{
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Acid = value;
				break; 
			}
			case at_resists_cold :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Cold = value;
				break; 
			}
			case at_resists_electricity :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Electricity = value;
				break; 
			}
			case at_resists_fire :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Fire = value;
				break; 
			}
			case at_resists_poison :
			{
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Poison = value;
				break; 
			}
			case at_resists_rot :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Rot = value;
				break; 
			}
			case at_resists_shockwave :
			{ 
				uint16 value;
				NLMISC::fromString(scriptLine[1], value);
				_Resists.Shockwave = value;
				break; 
			}

			case at_unknown:
			{	
				nlwarning("<CStaticCreatures::applyUserModel>Error while applying user model '%s': unknown token '%s', ignoring it.", modelId.c_str(), scriptLine[0].c_str());
				errors = true;
				break;
			}

		}
	}
	return errors;
}







///////////////////////////////////////////////////////////////////////////
//////////////////////// Static Characters ////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet
void CStaticCharacters::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
//	nlinfo ("<CStaticCharacters::readGeorges> Ajoute la fiche %s %d", sheetId.toString().c_str(), sheetId.asInt());
	// load sheet

	if( form )
	{
		UFormElm& root = form->getRootNode();		

		string value;

		///////////////////////////////////////////////////////
		// Race
		///////////////////////////////////////////////////////
		root.getValueByName( value, "Basics.Race" );
		Race = EGSPD::CPeople::fromString( value );

		///////////////////////////////////////////////////////
		// Gender
		///////////////////////////////////////////////////////
		root.getValueByName( Gender, "Basics.Gender" );

		///////////////////////////////////////////////////////
		// Character Size
		///////////////////////////////////////////////////////
		root.getValueByName( value, "Basics.Size" );
		Size = CREATURE_SIZE::stringToCreatureSize( value );

		///////////////////////////////////////////////////////
		// Level
		///////////////////////////////////////////////////////
		root.getValueByName( Level, "Basics.Level" );
		
		///////////////////////////////////////////////////////
		// First name
		///////////////////////////////////////////////////////
		//root.getValueByName( Surname, "Basics.Name" );
//		root.getValueByName( Name, "Basics.First Name" );

		///////////////////////////////////////////////////////
		// Surname
		///////////////////////////////////////////////////////
//		root.getValueByName( Surname, "Basics.CharacterName" );

		///////////////////////////////////////////////////////
		// Characteristics
		///////////////////////////////////////////////////////
		int i;
		for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
		{
			root.getValueByName( Characteristics[ (CHARACTERISTICS::TCharacteristics)i ], ( string("Basics.Characteristics.") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i ) ).c_str(), UFormElm::Formula );
		}

		///////////////////////////////////////////////////////
		// Derivated Scores
		///////////////////////////////////////////////////////
		for( i = 0; i < SCORES::NUM_SCORES; ++i )
		{
			root.getValueByName( Scores[ i ], ( string("Basics.Scores.") + SCORES::toString( i ) ).c_str(), UFormElm::Formula );
		}
		
		///////////////////////////////////////////////////////
		// Items
		///////////////////////////////////////////////////////
		string sheetName;
		CSheetId sheet;

		for( i = 0; i < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++i )
		{
			root.getValueByName( sheetName, (string("Basics.Equipment.") + SLOT_EQUIPMENT::toString( (SLOT_EQUIPMENT::TSlotEquipment) i ) + string(".Item" )).c_str() );
			if( sheetName != string("") )
			{
				sheet = CSheetId( sheetName );
				Items[ i ].IdSheet = sheet.asInt();
				root.getValueByName( Items[ i ].Quality, (string("Basics.Equipment.") + SLOT_EQUIPMENT::toString( (SLOT_EQUIPMENT::TSlotEquipment) i ) + string(".Quality")).c_str() );
			}
			else
			{
				Items[ i ].IdSheet = 0;
				Items[ i ].Quality = 0;
			}
		}

		///////////////////////////////////////////////////////
		// Sheaths
		///////////////////////////////////////////////////////
/*		for( i = 0; i < NB_SHEATH; ++i )
		{
			root.getValueByName( sheetName, (string("Basics.Equipment.Sheath") + toString( i ) + string("LeftHand.Item" )).c_str() );
			if( sheetName != string("") )
			{
				sheet = CSheetId( sheetName );
				Sheaths[ i ].Left.IdSheet = sheet.asInt();
				root.getValueByName( Sheaths[ i ].Left.Quality, (string("Basics.Equipment.Sheath") + toString( i ) + string("LeftHand.Quality")).c_str() );
			}
			else
			{
				Sheaths[ i ].Left.IdSheet = 0;
				Sheaths[ i ].Left.Quality = 0;
			}

			root.getValueByName( sheetName, (string("Basics.Equipment.Sheath") + toString( i ) + string("RightHand.Item" )).c_str() );
			if( sheetName != string("") )
			{
				sheet = CSheetId( sheetName );
				Sheaths[ i ].Right.IdSheet = sheet.asInt();
				root.getValueByName( Sheaths[ i ].Right.Quality, (string("Basics.Equipment.Sheath") + toString( i ) + string("RightHand.Quality")).c_str() );
			}
			else
			{
				Sheaths[ i ].Right.IdSheet = 0;
				Sheaths[ i ].Right.Quality = 0;
			}
		
			root.getValueByName( sheetName, (string("Basics.Equipment.Sheath") + toString( i ) + string("Ammo0.Item" )).c_str() );
			if( sheetName != string("") )
			{
				sheet = CSheetId( sheetName );
				Sheaths[ i ].Ammo0.IdSheet = sheet.asInt();
				root.getValueByName( Sheaths[ i ].Ammo0.Quality, (string("Basics.Equipment.Sheath") + toString( i ) + string("Ammo0.Quality")).c_str() );
			}
			else
			{
				Sheaths[ i ].Ammo0.IdSheet = 0;
				Sheaths[ i ].Ammo0.Quality = 0;
			}
		
			root.getValueByName( sheetName, (string("Basics.Equipment.Sheath") + toString( i ) + string("Ammo1.Item" )).c_str() );
			if( sheetName != string("") )
			{
				sheet = CSheetId( sheetName );
				Sheaths[ i ].Ammo1.IdSheet = sheet.asInt();
				root.getValueByName( Sheaths[ i ].Ammo1.Quality, (string("Basics.Equipment.Sheath") + toString( i ) + string("Ammo1.Quality")).c_str() );
			}
			else
			{
				Sheaths[ i ].Ammo1.IdSheet = 0;
				Sheaths[ i ].Ammo1.Quality = 0;
			}
		
			root.getValueByName( sheetName, (string("Basics.Equipment.Sheath") + toString( i ) + string("Ammo2.Item" )).c_str() );
			if( sheetName != string("") )
			{
				sheet = CSheetId( sheetName );
				Sheaths[ i ].Ammo2.IdSheet = sheet.asInt();
				root.getValueByName( Sheaths[ i ].Ammo2.Quality, (string("Basics.Equipment.Sheath") + toString( i ) + string("Ammo2.Quality")).c_str() );
			}
			else
			{
				Sheaths[ i ].Ammo2.IdSheet = 0;
				Sheaths[ i ].Ammo2.Quality = 0;
			}
		}
*/
		///////////////////////////////////////////////////////
		// Inventory
		///////////////////////////////////////////////////////
		const UFormElm *inventoryArray = 0;
        //if (root.getNodeByName (&inventoryArray, "Basics.Inventory") && inventoryArray)
		if (root.getNodeByName (&inventoryArray, "Inventory") && inventoryArray)
		{
			// Get array size
			uint size;
			inventoryArray->getArraySize (size);

			// Get a array value
			for (uint i=0; i<size; ++i)
			{
				inventoryArray->getArrayValue( value, i );
				CSheetId sheet(value);
				Inventory.push_back( sheet );
			}
		}

		///////////////////////////////////////////////////////
		// Pack Animal
		///////////////////////////////////////////////////////
		const UFormElm *packAnimalArray = 0;
        //if (root.getNodeByName (&packAnimalArray, "Basics.Pack Animal") && packAnimalArray )
		if (root.getNodeByName (&packAnimalArray, "Pack Animal") && packAnimalArray )
		{
			// Get array size
			uint size;
			packAnimalArray->getArraySize (size);

			// Get a array value
			for (uint i=0; i<size; ++i)
			{
				packAnimalArray->getArrayValue( value, i );
				CSheetId sheet( value );
				PackAnimal.push_back( sheet );
			}
		}

		///////////////////////////////////////////////////////
		// Known sentences
		///////////////////////////////////////////////////////
		const UFormElm *sentenceArray = NULL;
        if (root.getNodeByName (&sentenceArray, "PreMemorizedSentences") && sentenceArray)
		{
			 // Get array size
			uint size;
			sentenceArray->getArraySize (size);
			MemorizedSentences.resize( size );
			
//			nlinfo("<CStaticCharacters::readGeorges> static char role %d", Role);
			// Get a array value
			for (uint i=0; i<size; ++i)
			{
				const UFormElm *sentenceElt = NULL;
				if ( sentenceArray->getArrayNode( &sentenceElt, i) && sentenceElt)
				{
					sentenceElt->getValueByName( MemorizedSentences[ i ].Name, "Description" );

					const UFormElm *brickArray;
					if (sentenceElt->getNodeByName (&brickArray, "Bricks") && brickArray)
					{
						 // Get array size
						uint nbBricks;
						brickArray->getArraySize (nbBricks);

						if ( ! nbBricks )
						{
							nlwarning("<CStaticCharacters::readGeorges> sentence %s has no bricks, invalid sentence", MemorizedSentences[ i ].Name.c_str() );
						}

						// Get an array value
						for (uint j=0; j<nbBricks; ++j)
						{
							brickArray->getArrayValue ( value, j);
							CSheetId brickSheetId( value );
							MemorizedSentences[ i ].BricksIds.push_back( brickSheetId );
						}
						//nlinfo("			add sentence %s with %d bricks", MemorizedSentences[ i ].Name.c_str(), nbBricks );
					}					
				}
			}
		}

		// MoreKnownBricks
		const UFormElm *brickArray;
        if (root.getNodeByName (&brickArray, "KnownBricks") && brickArray)
		{
			// Get array size
			uint nbBricks;
			brickArray->getArraySize (nbBricks);

			// Get an array value
			for (uint i=0; i<nbBricks; ++i)
			{
				brickArray->getArrayValue ( value, i);
				CSheetId brickSheetId( value );
				KnownBricks.push_back( brickSheetId );
			}
		}

		///////////////////////////////////////////////////////
		// Movement speed 
		///////////////////////////////////////////////////////
		root.getValueByName( WalkSpeed, "Basics.MovementSpeeds.WalkSpeed" );
		root.getValueByName( RunSpeed, "Basics.MovementSpeeds.RunSpeed" );
	}
}


///////////////////////////////////////////////////////////////////////////
//////////////////////// Static Loot Set ////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet
void CStaticLootSet::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( form )
	{
		UFormElm& root = form->getRootNode();

		string value;

		const UFormElm *LootArray;
        if (root.getNodeByName (&LootArray, "Items") && LootArray)
		{
			// Get array size
			uint size;
			LootArray->getArraySize (size);
			ItemLoot.resize( size );
			
			// Get a array value
			for (uint i = 0; i < size; ++i)
			{
				const UFormElm *LootElt = NULL;
				if ( LootArray->getArrayNode( &LootElt, i) && LootElt )
				{
					///////////////////////////////////////////////////////
					// Item
					///////////////////////////////////////////////////////
					LootElt->getValueByName( value, "Item" );
					ItemLoot[ i ].Item = value;

					///////////////////////////////////////////////////////
					// Level
					///////////////////////////////////////////////////////
					LootElt->getValueByName( ItemLoot[ i ].Level, "Quality" );
					
					///////////////////////////////////////////////////////
					// Quantity
					///////////////////////////////////////////////////////
					LootElt->getValueByName( ItemLoot[ i ].Quantity, "Quantity" );
				}
			}
		}
	}	
	
} // CStaticLootSet::readGeorges //



///////////////////////////////////////////////////////////////////////////
//////////////////////// Static Loot Table ////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet
void CStaticLootTable::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( form )
	{
		UFormElm& root = form->getRootNode();

		string value;

		const UFormElm *LootArray;
        if (root.getNodeByName (&LootArray, "Items") && LootArray)
		{
			// Get array size
			uint size;
			LootArray->getArraySize (size);
						
			// Get a array value
			for (uint i = 0; i < size; ++i)
			{
				const UFormElm *LootElt = NULL;
				if ( LootArray->getArrayNode( &LootElt, i) && LootElt )
				{
					///////////////////////////////////////////////////////
					// Probability
					///////////////////////////////////////////////////////
					sint32 proba;
					LootElt->getValueByName( proba, "Probability" );
					
					if( proba <= 0 )
					{
						nlwarning("<CStaticLootTable::readGeorges> Negative probability or null for loot_set %s : %d",value.c_str(),proba);
					}
					else
					{
						///////////////////////////////////////////////////////
						// LootSet
						///////////////////////////////////////////////////////
						LootElt->getValueByName( value, "LootSet" );
						if( !value.empty() )
							LootSets.insert(make_pair(CSheetId(value),proba));
						else
							LootSets.insert(make_pair(CSheetId(),proba));
					}
				}
			}
		}

		const UFormElm *LootMoney = NULL;
		if (root.getNodeByName (&LootMoney, "Money") && LootMoney)
		{
			LootMoney->getValueByName( MoneyLvlFactor, "Lvl_Factor" );
			if( MoneyLvlFactor < 0.f || MoneyLvlFactor > 1.f )
			{
				nlwarning("<CStaticLootTable::readGeorges> Bad value for 'Lvl_Factor' in sheet %s: %d",sheetId.toString().c_str(), MoneyLvlFactor);
			}
		
			LootMoney->getValueByName( MoneyBase, "Base" );
			if( MoneyBase < 0 )
			{
				nlwarning("<CStaticLootTable::readGeorges> Bad value for 'Base' in sheet %s: %d",sheetId.toString().c_str(), MoneyBase);
			}

			LootMoney->getValueByName( MoneyDropProbability, "Probability" );
			if( MoneyDropProbability < 0.f || MoneyDropProbability > 1.f )
			{
				nlwarning("<CStaticLootTable::readGeorges> Bad value for 'Probability' in sheet %s: %d",sheetId.toString().c_str(), MoneyDropProbability);
			}
		}
	}
	
} // CStaticLootTable::readGeorges //


/// selectRandomLootSet
CSheetId CStaticLootTable::selectRandomLootSet() const
{
	if( LootSets.empty() )
		return CSheetId::Unknown;

	// compute the probability sum
	uint16 probabilitySum = 0;
	map<CSheetId,uint16>::const_iterator itconst;
	for( itconst = LootSets.begin(); itconst != LootSets.end(); ++itconst )
	{
		probabilitySum += (*itconst).second;
	}

	// choose a random number between and probabilitySum
	uint32 randWeight;
	if( probabilitySum == 0 )
		randWeight = 0;
	else
		randWeight = RandomGenerator.rand(probabilitySum-1) + 1;

	// "concatenate" weights of each index, when the random value is reached we'll have the index to use
	uint16 w = 0;
	for( itconst = LootSets.begin(); itconst != LootSets.end(); ++itconst )
	{
		w += (*itconst).second;
		if( randWeight <= w )
		{
			break;
		}
	}
	if( itconst != LootSets.end() )
	{
		return (*itconst).first;
	}

	nlwarning("<CStaticLootTable::selectRandomLootSet> can't find any lootset rand=%d probabilitySum=%d weightCount=%d",randWeight,probabilitySum,LootSets.size());
	return CSheetId::Unknown;
}

const CStaticLootSet *CStaticLootTable::selectRandomCustomLootSet() const
{
	if( CustomLootSets.empty() )
		return 0;

	// compute the probability sum
	uint16 probabilitySum = 0;
	multimap<uint16, CStaticLootSet>::const_iterator it = CustomLootSets.begin();
	for( ; it != CustomLootSets.end(); ++it )
	{
		probabilitySum += (*it).first;
	}
	
	// choose a random number between and probabilitySum
	uint32 randWeight;
	if( probabilitySum == 0 )
		randWeight = 0;
	else
		randWeight = RandomGenerator.rand(probabilitySum-1) + 1;
	
	// "concatenate" weights of each index, when the random value is reached we'll have the index to use
	uint16 w = 0;
	for (it = CustomLootSets.begin(); it != CustomLootSets.end(); ++it )
	{
		w += (*it).first;
		if( randWeight <= w )
		{
			break;
		}
	}

	if( it != CustomLootSets.end() )
	{
		return &(it->second);
	}

	nlwarning("Can't find any lootset rand=%d probabilitySum=%d weightCount=%d",randWeight,probabilitySum,CustomLootSets.size());
	return 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////// Static Race Statistics //////////////////////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet
void CStaticRaceStats::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	nlinfo ("<CStaticRaceStats::readGeorges> Ajoute la fiche %s %d", sheetId.toString().c_str(), sheetId.asInt());

	if( form )
	{
		UFormElm& root = form->getRootNode();		


		///////////////////////////////////////////////////////
		// Race
		///////////////////////////////////////////////////////
		string value;
		root.getValueByName( value, "Race" );
		Race = EGSPD::CPeople::fromString( value );

		///////////////////////////////////////////////////////
		// Caracteristics
		///////////////////////////////////////////////////////
		int i;
		for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
		{
			root.getValueByName( Characteristics[ (CHARACTERISTICS::TCharacteristics)i ], ( string("Characteristics.") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i ) ).c_str() );
		}

		///////////////////////////////////////////////////////
		// Derivated Scores
		///////////////////////////////////////////////////////
		for( i = 0; i < SCORES::NUM_SCORES; ++i )
		{
			root.getValueByName( Scores[ i ], ( string("Scores.") + SCORES::toString( i ) ).c_str(), UFormElm::Formula );
		}

		///////////////////////////////////////////////////////
		// Training progression speed for scores and regenerate
		///////////////////////////////////////////////////////
		root.getValueByName( ProgressionScore1, "TrainingProgression.TrainingScore1" );
		root.getValueByName( ProgressionScore2, "TrainingProgression.TrainingScore2" );
		root.getValueByName( ProgressionScore3, "TrainingProgression.TrainingScore3" );
		root.getValueByName( ProgressionScore4, "TrainingProgression.TrainingScore4" );

		root.getValueByName( ProgressionRegen1, "TrainingProgression.TrainingRegen1" );
		root.getValueByName( ProgressionRegen2, "TrainingProgression.TrainingRegen2" );
		root.getValueByName( ProgressionRegen3, "TrainingProgression.TrainingRegen3" );
		root.getValueByName( ProgressionRegen4, "TrainingProgression.TrainingRegen4" );

		///////////////////////////////////////////////////////
		// Default equipment
		///////////////////////////////////////////////////////
		// Male
		root.getValueByName( MaleDefaultEquipment.DefaultFace, "DefaultEquipment.Male equipment.DefaultFace" );
		root.getValueByName( MaleDefaultEquipment.DefaultChest, "DefaultEquipment.Male equipment.DefaultChest" );
		root.getValueByName( MaleDefaultEquipment.DefaultArms, "DefaultEquipment.Male equipment.DefaultArms" );
		root.getValueByName( MaleDefaultEquipment.DefaultLegs, "DefaultEquipment.Male equipment.DefaultLegs" );
		root.getValueByName( MaleDefaultEquipment.DefaultHands, "DefaultEquipment.Male equipment.DefaultHands" );
		root.getValueByName( MaleDefaultEquipment.DefaultFeet, "DefaultEquipment.Male equipment.DefaultFeet" );
		root.getValueByName( MaleDefaultEquipment.DefaultHair, "DefaultEquipment.Male equipment.DefaultHair" );
		// Female
		root.getValueByName( FemaleDefaultEquipment.DefaultFace, "DefaultEquipment.Female equipment.DefaultFace" );
		root.getValueByName( FemaleDefaultEquipment.DefaultChest, "DefaultEquipment.Female equipment.DefaultChest" );
		root.getValueByName( FemaleDefaultEquipment.DefaultArms, "DefaultEquipment.Female equipment.DefaultArms" );
		root.getValueByName( FemaleDefaultEquipment.DefaultLegs, "DefaultEquipment.Female equipment.DefaultLegs" );
		root.getValueByName( FemaleDefaultEquipment.DefaultHands, "DefaultEquipment.Female equipment.DefaultHands" );
		root.getValueByName( FemaleDefaultEquipment.DefaultFeet, "DefaultEquipment.Female equipment.DefaultFeet" );
		root.getValueByName( FemaleDefaultEquipment.DefaultHair, "DefaultEquipment.Female equipment.DefaultHair" );
	}
}


///////////////////////////////////////////////////////////////////////////
////////////////// Static Starting Role Statistics ////////////////////////
///////////////////////////////////////////////////////////////////////////

/// read the sheet
void CStaticRole::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( form )
	{
		string value;
		UFormElm& root = form->getRootNode();		

		///////////////////////////////////////////////////////
		// Role
		///////////////////////////////////////////////////////
		root.getValueByName( value, "Role" );
		Role = ROLES::toRoleId( value );

		nlinfo("<CStaticRole::readGeorges> Sheet %s Role %s RoleId %hu (%s)", sheetId.toString().c_str(), value.c_str(), (uint16)Role, ROLES::toString( (ROLES::ERole)Role ).c_str());

		///////////////////////////////////////////////////////
		// Race
		///////////////////////////////////////////////////////
		root.getValueByName( value, "Race" );
		Race = EGSPD::CPeople::fromString( value );

		///////////////////////////////////////////////////////
		// One point action and equipment
		///////////////////////////////////////////////////////
		string s1, s2;
		s1 = string( "PreMemorizedSentences1");
		s2 = string( "StartEquipment1");
		readGeorgesSentenceAndEquipment( root, sheetId, s1, MemorizedSentences1, s2, Items1 );

		///////////////////////////////////////////////////////
		// Two points action and equipment
		///////////////////////////////////////////////////////
		s1 = string( "PreMemorizedSentences2");
		s2 = string( "StartEquipment2");
		readGeorgesSentenceAndEquipment( root, sheetId, s1, MemorizedSentences2, s2, Items2 );
		
		///////////////////////////////////////////////////////
		// Three points action and equipment
		///////////////////////////////////////////////////////
		s1 = string( "PreMemorizedSentences3");
		s2 = string( "StartEquipment3");
		readGeorgesSentenceAndEquipment( root, sheetId, s1, MemorizedSentences3, s2, Items3 );
	}
}

///////////////////////////////////////////////////////
// load known sentence and equipment
///////////////////////////////////////////////////////
void CStaticRole::readGeorgesSentenceAndEquipment( UFormElm& root, const NLMISC::CSheetId &sheetId, const string& SentenceString, vector< TMemorizedSentence >& MemorizedSentences, const string& EquipmentString, SMirrorEquipment* Items )
{
	string value;
	const UFormElm *sentenceArray = NULL;
	if (root.getNodeByName (&sentenceArray, SentenceString.c_str()) && sentenceArray)
	{
		// Get array size
		uint size;
		sentenceArray->getArraySize (size);
		
		// Get a array value
		for (uint i=0; i<size; ++i)
		{
			const UFormElm *sentenceElt = NULL;
			if ( sentenceArray->getArrayNode( &sentenceElt, i) && sentenceElt)
			{
				sentenceElt->getValueByName( value, "Sabrina Phrase" );
//				string value2;
//				sentenceElt->getValueByName( value2, "Memory Type" );
				
				CSheetId sheet(value);
				if( sheet != CSheetId::Unknown )
				{
					TMemorizedSentence sentence;
					sentence.sentence = sheet;
					
//					sentence.memory = MEM_SET_TYPES::toMemSetType( value2 );
/*					if( sentence.memory == MEM_SET_TYPES::Unknown )
					{
						nlwarning("<CStaticRole::readGeorges> Unknown MEM_SET_TYPES for sheet %s, sentence %d of memorized sentence 1", sheetId.toString().c_str(), i );
					}
*/
					MemorizedSentences.push_back( sentence );
				}
				else
				{
					nlwarning("<CStaticRole::readGeorges> Sentence sheet not found for sheet %s, sentence %d of memorized sentence 1", sheetId.toString().c_str(), i );
				}
			}
		}
	}

	///////////////////////////////////////////////////////
	// Items
	///////////////////////////////////////////////////////
	string sheetName;
	CSheetId sheet;

	for( int i = 0; i < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++i )
	{
		root.getValueByName( sheetName, ( EquipmentString + string(".") + SLOT_EQUIPMENT::toString( (SLOT_EQUIPMENT::TSlotEquipment) i ) + string(".Item" )).c_str() );
		if( sheetName != string("") )
		{
			sheet = CSheetId( sheetName );
			Items[ i ].IdSheet = sheet.asInt();
			root.getValueByName( Items[ i ].Quality, ( EquipmentString + string(".") + SLOT_EQUIPMENT::toString( (SLOT_EQUIPMENT::TSlotEquipment) i ) + string(".Quality")).c_str() );
		}
		else
		{
			Items[ i ].IdSheet = 0;
			Items[ i ].Quality = 0;
		}
	}
}


///////////////////////////////////////////////////////////////////////////
///////////////////////// Static Skills Tree //////////////////////////////
///////////////////////////////////////////////////////////////////////////

//-----------------------------------------------
// readGeorges for CStaticSkillsTree
//
//-----------------------------------------------
void CStaticSkillsTree::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( form )
	{
		UFormElm& root = form->getRootNode();
		
		UFormElm *arraySkillElt = NULL;
		if( root.getNodeByName( &arraySkillElt, "SkillData" ) )
		{
			if( arraySkillElt )
			{
				uint NbSkills;
				nlverify( arraySkillElt->getArraySize( NbSkills ) );

				nlassertex( NbSkills == SKILLS::NUM_SKILLS, ("(%u != %u) Please synchronise game_share/skill.* with leveldesign/game_element/xp_table/skills.skill_tree (use skill_extractor.exe)", NbSkills, SKILLS::NUM_SKILLS));

				SkillsTree.resize( NbSkills );
				
				for( uint i = 0; i < NbSkills; ++i )
				{
					UFormElm* SkillElt = NULL;
					if( ! ( arraySkillElt->getArrayNode( &SkillElt, i ) && SkillElt ) )
					{
						nlwarning("<CStaticSkillsTree::readGeorges> can't get array node of SkillElt in sheet %s", sheetId.toString().c_str() );
					}
					else
					{
						// Skill
						string SkillName;
						SkillElt->getValueByName( SkillName, "Skill" );
						SKILLS::ESkills skill = SKILLS::toSkill( SkillName );
						nlassert( skill != SKILLS::unknown );
						if (skill == SKILLS::unknown)
						{
							continue;
						}
						SkillsTree[ skill ].Skill = skill;

						if( ! SkillElt->getValueByName( SkillsTree[ skill ].SkillCode, "SkillCode" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node SkillCode in sheet %s", sheetId.toString().c_str() );
						}

						// Skill Code
						if( ! SkillElt->getValueByName( SkillsTree[ skill ].SkillCode, "SkillCode" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node SkillCode in sheet %s", sheetId.toString().c_str() );
						}

						// Max skill value
						if( ! SkillElt->getValueByName( SkillsTree[ skill ].MaxSkillValue, "MaxSkillValue" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node MaxSkillValue in sheet %s", sheetId.toString().c_str() );
						}

						// Type of stage
						if( ! SkillElt->getValueByName( SkillsTree[ skill ].StageType, "Type of Stage" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node 'Type of Stage' in sheet %s", sheetId.toString().c_str() );
						}

						// ParentSkill
						if( ! SkillElt->getValueByName( SkillName, "ParentSkill" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node ParentSkills in sheet %s", sheetId.toString().c_str() );
						}
						else
						{	
							SkillsTree[ skill ].ParentSkill = SKILLS::toSkill( SkillName );
						}

						// ChildSkills
						UFormElm *arrayChildSkillElt = NULL;
						if( SkillElt->getNodeByName( &arrayChildSkillElt, "ChildSkills" ) )
						{
							if( arrayChildSkillElt )
							{
								uint NbChildSkills;
								nlverify( arrayChildSkillElt->getArraySize( NbChildSkills ) );
								
								SkillsTree[ skill ].ChildSkills.resize( NbChildSkills );
								
								for( uint i = 0; i < NbChildSkills; ++i )
								{
									string childSkillName;
									arrayChildSkillElt->getArrayValue( childSkillName, i );
									SKILLS::ESkills childSkill = SKILLS::toSkill( childSkillName );
									nlassert( childSkill != SKILLS::unknown );
									if (skill == SKILLS::unknown)
									  {
										continue;
									  }
									SkillsTree[ skill ].ChildSkills[ i ] = childSkill;
								}
							}
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------
// CStaticSkillsTree::getTreeSkillPointsUnderSkill
//-----------------------------------------------
uint32 CStaticSkillsTree::getTreeSkillPointsUnderSkill(SKILLS::ESkills skill) const
{
	if (skill >= 0 && skill < (sint32)SkillsTree.size())
	{
		CStaticSkillsTree::SSkillData skillData = SkillsTree[ skill ];
		
		const vector<SKILLS::ESkills> &childSkills = skillData.ChildSkills;
		uint16 ref = 0;
		if (skillData.ParentSkill != SKILLS::unknown)
		{
			ref = SkillsTree[ skillData.ParentSkill ].MaxSkillValue + 1;
		}
		else
			ref = 1; // min top skill value is 1
		
		uint32 ret = skillData.MaxSkillValue - ref;
		
		for (uint i = 0 ; i < childSkills.size() ; ++i)
		{
			ret += getTreeSkillPointsUnderSkill(childSkills[i]);
		}

		// if skill has childs, add 1 to total SP
		if (!childSkills.empty())
			++ret;
		
		return ret;
	}
	else
		return 0;
} // getTreeSkillPointsUnderSkill //


//-----------------------------------------------
// CStaticSkillsTree::getPlayerSkillPointsUnderSkill
//-----------------------------------------------
uint32 CStaticSkillsTree::getPlayerSkillPointsUnderSkill(const	CSkills *skills, SKILLS::ESkills skill) const
{
	if(!skills)
		return 0;
	
	if (skill >= 0 && skill < (sint32)SkillsTree.size())
	{
		// if skill is locked returns 0
		if ( skills->_Skills[skill].Base == 0)
			return 0;
		
		// skill unlocked, check children values
		CStaticSkillsTree::SSkillData skillData = SkillsTree[ skill ];
		const vector<SKILLS::ESkills> &childSkills = skillData.ChildSkills;
		uint16 ref = 0;
		if (skillData.ParentSkill != SKILLS::unknown)
		{
			ref = SkillsTree[ skillData.ParentSkill ].MaxSkillValue + 1;
		}
		else
			ref = 1; // min top skill value is 1
		
		uint32 ret = skills->_Skills[skill].Base - ref;

		bool childUnlocked = false;
		for (uint i = 0 ; i < childSkills.size() ; ++i)
		{
			// add unlocked child Sp
			if (skills->_Skills[childSkills[i]].Base > 0)
			{
				childUnlocked = true;
				ret += getPlayerSkillPointsUnderSkill(skills, childSkills[i]);
			}
		}

		// if any child is unlocked, add 1 to total SP
		if (childUnlocked)
			++ret;

		return ret;
	}
	else
		return 0;
} // getPlayerSkillPointsUnderSkill //
