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

#include "game_share/brick_families.h"
#include "game_share/entity_structure/statistic.h"
#include "game_share/egs_sheets/egs_static_brick.h"

#include "faber_action.h"
#include "faber_phrase.h"
#include "entity_manager.h"
#include "character.h"
#include "phrase_utilities_functions.h"

extern NLMISC::CRandom RandomGenerator;

using namespace std;
using namespace NLMISC;
using namespace RY_GAME_SHARE;

std::vector< std::pair< ITEM_TYPE::TItemType , IFaberActionFactory* > >IFaberActionFactory::Factories;




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Common class of faber action
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionCommon
{
public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//check sentence validity (grammar check => tool, Mp types and quantity, optionals bricks, credits bricks) 
	static bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		uint nbBricks = bricks.size();
		for( uint i = 0; i < nbBricks; ++i )
		{
			switch( bricks[i]->Family )
			{
			case BRICK_FAMILIES::RootFaber:
				if( i != 0 )
				{
					nlwarning("<CFaberActionCommon::checkSentenceValidity> RootFaber bricks %s is not in first position in sentence", bricks[i]->SheetId.toString().c_str() );
					return false; //plan must be first in sentence
				}
				break;
			case BRICK_FAMILIES::FARawMaterial:
				//TODO check type of Mp for each plan slot and quantity
				break;
			default:
				//TODO check compatibility of optional bricks and costs bricks
				break;
			};
		}
		return true;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Compute chance
	static uint8 computeChance( CCharacter* c, CFaberPhrase * phrase, sint& deltaLvl )
	{
		// compute success factor
		SSkill * skill = c->getSkills().getSkillStruct( phrase->getRootFaberPlan()->Faber->Skill );
		nlassert( skill ); //does never happen, checked in checkBrickValidity function
		sint32 skillValue = skill->Current;

/*		TODO: get effect on skill if exist
		const CSEffect * debuff = c->lookForSEffect( EFFECT_FAMILIES::DebuffSkillFaber );
		if ( debuff) skillValue -= debuff->getParamValue();
		if ( skillValue < 0 ) skillValue = 0;
*/			
		// get the success factor (divide delta level by 10 because a level is 10 skill points
		deltaLvl = ( skillValue + phrase->getSabrinaCredit() - (phrase->getSabrinaCost()<<1) ) / 10;
		return PHRASE_UTILITIES::getSuccessChance( deltaLvl );
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Roll success factor
	static float rollSuccessFactor( CCharacter* c, CFaberPhrase * phrase, sint& deltaLvl )
	{
		uint8 roll =(uint8) RandomGenerator.rand(99);
		return PHRASE_UTILITIES::getSucessFactor( computeChance( c, phrase, deltaLvl ), roll );
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// get success factor and send message to client
	static float getSuccessFactor( CCharacter* c, CFaberPhrase * phrase, sint& deltaLvl )
	{
		float successFactor = rollSuccessFactor( c, phrase, deltaLvl );
		if( successFactor < 0.0f )
		{
			//Fumble
			//TODO add message to client
		}
		else if( successFactor == 0.0f )
		{
			//Failure
			//TODO add message to client
		}
		else
		{
			if( successFactor < 1.0f )
			{
				//Partial success
				//TODO add message to client
			}
			else if( successFactor > 1.0f )
			{
				//Critical success
				//TODO add message to client
			}
			else
			{
				//Normal success
				//TODo add message to client
			}
		}
		return successFactor;
	}

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// create a system crafted item
	static CGameItemPtr createSystemCraftedItem( uint16 quantity, const NLMISC::CSheetId& sheet )
	{
		if (quantity == 0) return NULL;
		
		// if quantity > 1, check if item is stackable and check stack quantity
		if (quantity > 1)
		{
			const CStaticItem* form = CSheets::getForm( sheet );
			if( form )
			{
				if( form->Stackable < quantity )
				{
					quantity = (uint16) form->Stackable;
				}
			}
			else
			{
				nlwarning("<CFaberActionCommon::createACraftedItem> can't found form for item %s", sheet.toString().c_str());
			}
		}
		
		if (quantity > 1)
		{
			CSheetId idSheetStack("stack.sitem");
			CGameItemPtr stackItem = GameItemManager.createItem( idSheetStack, (uint16)1, CEntityId::Unknown, (sint16)0, false, CEntityId::Unknown );
			if( stackItem == NULL )
			{
				nlwarning("<CFaberActionCommon::createACraftedItem> Error while creating stack bag %s -> returned a NULL pointer", idSheetStack.toString().c_str() );
				return NULL;
			}
			else
			{
				uint32 hp = 0;
				uint32 hpmax = 0;
				for( int q = 0; q < quantity; ++q )
				{
					CGameItemPtr itemTmp = GameItemManager.createItem( const_cast< CSheetId& > ( sheet ), 1, const_cast<NLMISC::CEntityId&>(stackItem->getId()), (sint16)-1, true, CEntityId::Unknown );
					if (!hp && itemTmp != NULL)
					{
						hp = itemTmp->hp();
						hpmax = itemTmp->standardHP();
					}
				}
				return stackItem;	
			}
		}
		else // do not create a stack, as there is only one object
		{
			CGameItemPtr item = GameItemManager.createItem( const_cast< CSheetId& > ( sheet ), (uint16)1, CEntityId::Unknown, (sint16)0, true, CEntityId::Unknown  );
			if( item == NULL)
			{
				nlwarning("<CFaberActionCommon::createACraftedItem> Error while creating item %s -> returned a NULL pointer", sheet.toString().c_str() );
				return NULL;
			}
			return item;
		}
	} // createSystemCraftedItem //
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create crafted item, consume Mps and character xp gain
	static void createCraftedItem( CFaberPhrase * phrase, CCharacter * c, SKILLS::ESkills skill, sint16 deltaLvl, const CSheetId& sheet, uint16 quality, uint16 nbItemsPerUnit, uint32 durability, float weight, 
		uint16 dmg, float speed, uint16 sapLoad, float range, uint16 maxSlashingProtection, float slashingProtectionFactor, uint16 maxBluntProtection, float bluntProtectionFactor, 
		uint16 maxPiercingProtection, float piercingProtectionFactor, uint16 DodgeBonus, uint16 ParryBonus, uint8 color )
	{
		CGameItemPtr item;
		if( c != 0 )
		{
			item = c->createItemInBag( quality, nbItemsPerUnit, sheet, c->getId() );
		}
		else
		{
			item = createSystemCraftedItem( nbItemsPerUnit, sheet );
		}
		
		if( item != 0 )
		{
			if( item->getSheetId() == CSheetId("stack.sitem") )
			{
				const vector< CGameItemPtr >& items = item->getChildren();
				uint nbItems = items.size();

				CGameItemPtr itemEvent = NULL;
				uint realNb = nbItems;
				for( uint i = 0; i < nbItems; ++i )
				{
					if( items[ i ] != 0 )
					{
						itemEvent = items[ i ];
						items[ i ]->setHp( durability );
						items[ i ]->setWeight( weight );
						items[ i ]->setDamage( dmg );
						items[ i ]->setSpeed( speed );
						items[ i ]->setSapLoad( sapLoad );
						items[ i ]->setRange( range );

						items[ i ]->setProtection( DMGTYPE::BLUNT,maxBluntProtection,bluntProtectionFactor );
						items[ i ]->setProtection( DMGTYPE::PIERCING,maxSlashingProtection,slashingProtectionFactor );
						items[ i ]->setProtection( DMGTYPE::SLASHING,maxPiercingProtection,piercingProtectionFactor );

					/*	items[ i ]->setSlashingProtection( maxSlashingProtection );
						items[ i ]->setBluntProtection( maxBluntProtection );
						items[ i ]->setPiercingProtection( maxPiercingProtection );

						items[ i ]->setSlashingProtectionFactor( slashingProtectionFactor );
						items[ i ]->setBluntProtectionFactor( bluntProtectionFactor );
						items[ i ]->setPiercingProtectionFactor( piercingProtectionFactor );
						*/

						items[ i ]->setDodgeModifier( DodgeBonus );
						items[ i ]->setParryModifier( ParryBonus );
						
						items[ i ]->Color = color;
					}
					else
						realNb--;
				}
				if ( itemEvent != NULL )
				{
					CMissionEventCraft event(itemEvent->getSheetId(),realNb);
					if( c )	c->processMissionEvent(event);
				}
			}
			else
			{
				item->setHp( durability );
				item->setWeight( weight );
				item->setDamage( dmg );
				item->setSpeed( speed );
				item->setSapLoad( sapLoad );
				item->setRange( range );

				item->setProtection( DMGTYPE::BLUNT,maxBluntProtection,bluntProtectionFactor );
				item->setProtection( DMGTYPE::PIERCING,maxSlashingProtection,slashingProtectionFactor );
				item->setProtection( DMGTYPE::SLASHING,maxPiercingProtection,piercingProtectionFactor );
				/*
				item->setSlashingProtection( maxSlashingProtection );
				item->setBluntProtection( maxBluntProtection );
				item->setPiercingProtection( maxPiercingProtection );
				
				item->setSlashingProtectionFactor( slashingProtectionFactor );
				item->setBluntProtectionFactor( bluntProtectionFactor );
				item->setPiercingProtectionFactor( piercingProtectionFactor );
				*/
				
				item->setDodgeModifier( DodgeBonus );
				item->setParryModifier( ParryBonus );
				item->Color = color;

				CMissionEventCraft event(sheet,1);
				if( c ) c->processMissionEvent(event);
			}
			
			if( c != 0)
			{
				//Consume Mps
				c->consumeMp();
				
				// action report for xp gain
				c->actionReport( 0, deltaLvl, ACTNATURE::NEUTRAL, SKILLS::toString( skill ) );
			}
			phrase->setCraftedItem( item );
		}
	}
};




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				Common part of item statistics computation
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeCompute : public IFaberAction
{
public:
	// ctor
	CFaberActionMakeCompute() {};
	
	// dtor
	virtual ~CFaberActionMakeCompute() {};

	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color ) = 0;
	
protected:
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void apply(CFaberPhrase * phrase)
	{
		CCharacter* character = ( CCharacter * ) CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if( character )
		{
			sint deltaLvl;
			// compute success factor
			float successFactor = CFaberActionCommon::getSuccessFactor( character, phrase, deltaLvl );
			
			// partial, normal or critical success
			if( successFactor > 0.0f )
			{
				uint32 Durability = 0;
				float Weight = 0.0f;
				uint16 Dmg = 0;
				float Speed = 0.0f;
				uint16 Quality = 0;
				uint16 SapLoad = 0;
				float  Range = 0.0f;
				
				uint16 MaxSlashingProtection = 0;
				float SlashingProtectionFactor = 0.0f;
				
				uint16 MaxBluntProtection = 0;
				float BluntProtectionFactor = 0.0f;
				
				uint16 MaxPiercingProtection = 0;
				float PiercingProtectionFactor = 0.0f;
				
				uint16 DodgeModifier = 0;
				uint16 ParryModifier = 0;

				uint8  Color = 254; // -2 is not a valid value on uint8
				
				uint32 mpOccurence = 0;
				
				// parsing faber plan 
				uint32 neededMp = phrase->getRootFaberPlan()->Faber->NeededMps.size();
				for( uint mp = 0; mp < neededMp; ++mp )
				{
					//for each type of Mp needed
					for( uint k = 0; k < phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].Quantity; ++k )
					{
						// for each Mp of one type (we have Quantity by type)
						uint32 NumMpParameters = phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters.size();
						// for each Faber parameters in Mp
						for( uint j = 0; j < NumMpParameters; ++j )
						{
							// check if Mp Type match with Faber waiting Type
							if( phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].MpType == phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ j ].MpFaberType )
							{
								specializedApply( phrase, mpOccurence, j, Durability, Weight, Dmg, Speed, SapLoad, Range, MaxSlashingProtection, SlashingProtectionFactor, MaxBluntProtection, BluntProtectionFactor, MaxPiercingProtection, PiercingProtectionFactor, DodgeModifier, ParryModifier, Color );
								break;
							}
						}
						++mpOccurence;
					}
				}
				
				Durability = max( (sint32)1, (sint32)(Durability * successFactor * phrase->getRootFaberPlan()->Faber->DurabilityFactor + phrase->getMBODurability() * successFactor) );
				Weight = Weight * phrase->getRootFaberPlan()->Faber->WeightFactor + phrase->getMBOWeight();
				Dmg = max( (sint16)1, (sint16)(Dmg * successFactor * phrase->getRootFaberPlan()->Faber->DMGFactor + phrase->getMBODmg() * successFactor) );
				Speed = max( 1.5f, (( Speed * phrase->getRootFaberPlan()->Faber->SpeedFactor ) * 2 - ( Speed * phrase->getRootFaberPlan()->Faber->SpeedFactor ) * successFactor + phrase->getMBOSpeed() * successFactor )) / 8.0f;
				SapLoad = max( (uint16)0, (uint16) ( SapLoad * phrase->getRootFaberPlan()->Faber->SapLoadFactor * successFactor + phrase->getMBOSapLoad() * successFactor ) );
				Range = max( 0.0f, Range * phrase->getRootFaberPlan()->Faber->RangeFactor * successFactor + phrase->getMBORange() * successFactor );
				
				MaxSlashingProtection = max( (uint16)0, (uint16) ( MaxSlashingProtection * phrase->getRootFaberPlan()->Faber->SlashingProtectionFactor * successFactor + phrase->getMBOProtection() * successFactor ) );
				MaxBluntProtection = max( (uint16)0, (uint16) ( MaxBluntProtection * phrase->getRootFaberPlan()->Faber->BluntProtectionFactor * successFactor + phrase->getMBOProtection() * successFactor ) );
				MaxPiercingProtection = max( (uint16)0, (uint16) ( MaxPiercingProtection * phrase->getRootFaberPlan()->Faber->PiercingProtectionFactor * successFactor + phrase->getMBOProtection() * successFactor ) );
				SlashingProtectionFactor = max( 0.0f, SlashingProtectionFactor * phrase->getRootFaberPlan()->Faber->SlashingProtectionFactor * successFactor );
				BluntProtectionFactor = max( 0.0f, BluntProtectionFactor * phrase->getRootFaberPlan()->Faber->BluntProtectionFactor * successFactor );
				PiercingProtectionFactor = max( 0.0f, PiercingProtectionFactor * phrase->getRootFaberPlan()->Faber->PiercingProtectionFactor * successFactor );
				DodgeModifier = (uint16) ( DodgeModifier * phrase->getRootFaberPlan()->Faber->DodgeFactor * successFactor );
				ParryModifier = (uint16) ( ParryModifier * phrase->getRootFaberPlan()->Faber->ParryFactor * successFactor );

				// Quality of item
				Quality = max( (uint16)1, (uint16)(character->getLowerMpQualityOfFaberSentence() * successFactor + phrase->getMBOQuality() * successFactor ) );
				
				CFaberActionCommon::createCraftedItem( phrase, character, phrase->getRootFaberPlan()->Faber->Skill, deltaLvl, phrase->getRootFaberPlan()->SheetId, 
					Quality, phrase->getRootFaberPlan()->Faber->NbItemsPerUnit, Durability, Weight, Dmg, Speed, SapLoad, Range, MaxSlashingProtection, SlashingProtectionFactor, MaxBluntProtection, 
					BluntProtectionFactor, MaxPiercingProtection, PiercingProtectionFactor, DodgeModifier, ParryModifier, Color );
			}
		}
	}

	// Apply params for system craft / compute result item (exemple selling item, not crafted by a player)
	virtual void systemApply(CFaberPhrase * phrase)
	{
			// compute success factor
			float successFactor = 1.0f;
			
			uint32 Durability = 0;
			float Weight = 0.0f;
			uint16 Dmg = 0;
			float Speed = 0.0f;
			uint16 Quality = 0;
			uint16 SapLoad = 0;
			float  Range = 0.0f;
			
			uint16 MaxSlashingProtection = 0;
			float SlashingProtectionFactor = 0.0f;
			
			uint16 MaxBluntProtection = 0;
			float BluntProtectionFactor = 0.0f;
			
			uint16 MaxPiercingProtection = 0;
			float PiercingProtectionFactor = 0.0f;
			
			uint16 DodgeModifier = 0;
			uint16 ParryModifier = 0;
			
			uint8  Color = 254; // -2 is not ok for an uint8
			
			// parsing faber plan 
			uint32 neededMp = phrase->getRootFaberPlan()->Faber->NeededMps.size();
			for( uint mp = 0; mp < neededMp; ++mp )
			{
				//for each type of Mp needed
				for( uint k = 0; k < phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].Quantity; ++k )
				{
					// for each Mp of one type (we have Quantity by type)
					uint32 NumMpParameters = phrase->getMps()[ mp ]->Mp->MpFaberParameters.size();
					// for each Faber parameters in Mp
					for( uint j = 0; j < NumMpParameters; ++j )
					{
						// check if Mp Type match with Faber waiting Type
						if( phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].MpType == phrase->getMps()[ mp ]->Mp->MpFaberParameters[ j ].MpFaberType )
						{
							specializedApply( phrase, mp, j, Durability, Weight, Dmg, Speed, SapLoad, Range, MaxSlashingProtection, SlashingProtectionFactor, MaxBluntProtection, BluntProtectionFactor, MaxPiercingProtection, PiercingProtectionFactor, DodgeModifier, ParryModifier, Color );
							break;
						}
					}
				}
			}
				
			Durability = max( (sint32)1, (sint32)(Durability * successFactor * phrase->getRootFaberPlan()->Faber->DurabilityFactor + phrase->getMBODurability() * successFactor) );
			Weight = Weight * phrase->getRootFaberPlan()->Faber->WeightFactor + phrase->getMBOWeight();
			Dmg = max( (sint16)1, (sint16)(Dmg * successFactor * phrase->getRootFaberPlan()->Faber->DMGFactor + phrase->getMBODmg() * successFactor) );
			Speed = max( 0.5f, ( Speed * phrase->getRootFaberPlan()->Faber->SpeedFactor ) * 2 - ( Speed * phrase->getRootFaberPlan()->Faber->SpeedFactor ) * successFactor + phrase->getMBOSpeed() * successFactor );
			SapLoad = max( (uint16)0, (uint16) ( SapLoad * phrase->getRootFaberPlan()->Faber->SapLoadFactor * successFactor + phrase->getMBOSapLoad() * successFactor ) );
			Range = max( 0.0f, Range * phrase->getRootFaberPlan()->Faber->RangeFactor * successFactor + phrase->getMBORange() * successFactor );
			
			MaxSlashingProtection = max( (uint16)0, (uint16) ( MaxSlashingProtection * phrase->getRootFaberPlan()->Faber->SlashingProtectionFactor * successFactor + phrase->getMBOProtection() * successFactor ) );
			MaxBluntProtection = max( (uint16)0, (uint16) ( MaxBluntProtection * phrase->getRootFaberPlan()->Faber->BluntProtectionFactor * successFactor + phrase->getMBOProtection() * successFactor ) );
			MaxPiercingProtection = max( (uint16)0, (uint16) ( MaxPiercingProtection * phrase->getRootFaberPlan()->Faber->PiercingProtectionFactor * successFactor + phrase->getMBOProtection() * successFactor ) );
			SlashingProtectionFactor = max( 0.0f, SlashingProtectionFactor * phrase->getRootFaberPlan()->Faber->SlashingProtectionFactor * successFactor );
			BluntProtectionFactor = max( 0.0f, BluntProtectionFactor * phrase->getRootFaberPlan()->Faber->BluntProtectionFactor * successFactor );
			PiercingProtectionFactor = max( 0.0f, PiercingProtectionFactor * phrase->getRootFaberPlan()->Faber->PiercingProtectionFactor * successFactor );
			DodgeModifier = (uint16) ( DodgeModifier * phrase->getRootFaberPlan()->Faber->DodgeFactor * successFactor );
			ParryModifier = (uint16) ( ParryModifier * phrase->getRootFaberPlan()->Faber->ParryFactor * successFactor );
				
			// Quality of item
			Quality = 0;
				
			CFaberActionCommon::createCraftedItem( phrase, 0, SKILLS::unknown, 0, phrase->getRootFaberPlan()->SheetId, 
				Quality, phrase->getRootFaberPlan()->Faber->NbItemsPerUnit, Durability, Weight, Dmg, Speed, SapLoad, Range, MaxSlashingProtection, SlashingProtectionFactor, MaxBluntProtection, 
				BluntProtectionFactor, MaxPiercingProtection, PiercingProtectionFactor, DodgeModifier, ParryModifier, Color );
	}
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Blade weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeBladeMeleeWeapon : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeBladeMeleeWeapon() {};

	// dtor
	virtual ~CFaberActionMakeBladeMeleeWeapon() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}

	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPL )
		{
			Dmg += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
		}
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPL ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPM ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPG ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPC )
		{
			Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		}
	}
};


// Dagger weapon specialized class
class CFaberActionMakeDagger : public CFaberActionMakeBladeMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeDagger, ITEM_TYPE::DAGGER )

// Sword weapon specialized class
class CFaberActionMakeSword : public CFaberActionMakeBladeMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeSword, ITEM_TYPE::SWORD )

// 2HSword weapon specialized class
class CFaberActionMake2HSword : public CFaberActionMakeBladeMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMake2HSword, ITEM_TYPE::TWO_HAND_SWORD )




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Hammer weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeHammerMeleeWeapon : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeHammerMeleeWeapon() {};
	
	// dtor
	virtual ~CFaberActionMakeHammerMeleeWeapon() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPH )
		{
			Dmg += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
		}
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPH ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPM ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPG )
		{
			Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		}
	}
};


// Mace weapon specialized class
class CFaberActionMakeMace : public CFaberActionMakeHammerMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMace, ITEM_TYPE::MACE )

// 2HMace weapon specialized class
class CFaberActionMake2HMace : public CFaberActionMakeHammerMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMake2HMace, ITEM_TYPE::TWO_HAND_MACE )

// Axe weapon specialized class
class CFaberActionMakeAxe : public CFaberActionMakeHammerMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeAxe, ITEM_TYPE::AXE )

// 2HAxe weapon specialized class
class CFaberActionMake2HAxe : public CFaberActionMakeHammerMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMake2HAxe, ITEM_TYPE::TWO_HAND_AXE )




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Point weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakePointMeleeWeapon : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakePointMeleeWeapon() {};
	
	// dtor
	virtual ~CFaberActionMakePointMeleeWeapon() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPP )
		{
			Dmg += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
		}
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPP ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPM ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPG )
		{
			Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		}
	}
};


// Spear weapon specialized class
class CFaberActionMakeSpear : public CFaberActionMakePointMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeSpear, ITEM_TYPE::SPEAR )

// Pike weapon specialized class
class CFaberActionMakePike : public CFaberActionMakePointMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakePike, ITEM_TYPE::PIKE )




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Staff weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeStaffMeleeWeapon : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeStaffMeleeWeapon() {};
	
	// dtor
	virtual ~CFaberActionMakeStaffMeleeWeapon() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPM )
		{
			Dmg += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
		}
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPM ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPG )
		{
			Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		}
	}
};
FABER_ACTION_FACTORY( CFaberActionMakeStaffMeleeWeapon, ITEM_TYPE::STAFF )




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Trigger range weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeTriggerRangeWeapon : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeTriggerRangeWeapon() {};
	
	// dtor
	virtual ~CFaberActionMakeTriggerRangeWeapon() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& DmgModifier, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;
		
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPPE ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPCA )
		{
			DmgModifier += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
			Range += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Range;
		}
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPGA ||
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPPE || 
			phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPCA )
		{
			Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		}
	}
};


// Pistol range weapon specialized class
class CFaberActionMakePistol : public CFaberActionMakeTriggerRangeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakePistol, ITEM_TYPE::PISTOL )

// Bowpistol range weapon specialized class
class CFaberActionMakeBowpistol : public CFaberActionMakeTriggerRangeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeBowpistol, ITEM_TYPE::BOWPISTOL )

// Rifle range weapon specialized class
class CFaberActionMakeRifle : public CFaberActionMakeTriggerRangeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeRifle, ITEM_TYPE::RIFLE )

// Bowgun range weapon specialized class
class CFaberActionMakeBowrifle : public CFaberActionMakeTriggerRangeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeBowrifle, ITEM_TYPE::BOWRIFLE )

// Launcher range weapon specialized class
class CFaberActionMakeLauncher : public CFaberActionMakeTriggerRangeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeLauncher, ITEM_TYPE::LAUNCHER )

// Autolaunch range weapon specialized class
class CFaberActionMakeAutolaunch : public CFaberActionMakeTriggerRangeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeAutolaunch, ITEM_TYPE::AUTOLAUCH )

// Harpoon range weapon specialized class
class CFaberActionMakeHarpoon : public CFaberActionMakeTriggerRangeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHarpoon, ITEM_TYPE::HARPOON )




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Ammo specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeAmmo : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeAmmo() {};
	
	// dtor
	virtual ~CFaberActionMakeAmmo() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;
		
		if( phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MpFaberType == RM_FABER_TYPE::MPPR )
		{
			Dmg += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
		}
		Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		Range += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Range;
	}
};


// Pistol ammo specialized class
class CFaberActionMakePistolAmmo : public CFaberActionMakeAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakePistolAmmo, ITEM_TYPE::PISTOL_AMMO )

// Bowpistol ammo specialized class
class CFaberActionMakeBowpistolAmmo : public CFaberActionMakeAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakeBowpistolAmmo, ITEM_TYPE::BOWPISTOL_AMMO )

// Rifle ammo specialized class
class CFaberActionMakeRifleAmmo : public CFaberActionMakeAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakeRifleAmmo, ITEM_TYPE::RIFLE_AMMO )

// Bowrifle ammo specialized class
class CFaberActionMakeBowrifleAmmo : public CFaberActionMakeAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakeBowrifleAmmo, ITEM_TYPE::BOWRIFLE_AMMO )

// Harpoon ammo specialized class
class CFaberActionMakeHarpoonAmmo : public CFaberActionMakeAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHarpoonAmmo, ITEM_TYPE::HARPOON_AMMO )




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Explosive Ammo specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeExplosiveAmmo : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeExplosiveAmmo() {};
	
	// dtor
	virtual ~CFaberActionMakeExplosiveAmmo() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;
		
		Dmg += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
		Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		Range += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Range;
	}
};


// Launcher ammo specialized class
class CFaberActionMakeLauncherAmmo : public CFaberActionMakeExplosiveAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakeLauncherAmmo, ITEM_TYPE::LAUNCHER_AMMO )

// Autolaunch ammo specialized class
class CFaberActionMakeAutolaunchAmmo : public CFaberActionMakeExplosiveAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakeAutolaunchAmmo, ITEM_TYPE::AUTOLAUNCH_AMMO )

// Grenade ammo specialized class
class CFaberActionMakeGenadeAmmo : public CFaberActionMakeExplosiveAmmo
{
};
FABER_ACTION_FACTORY( CFaberActionMakeGenadeAmmo, ITEM_TYPE::GRENADE_AMMO )




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Armor specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeArmor : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeArmor() {};
	
	// dtor
	virtual ~CFaberActionMakeArmor() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( const std::vector< const CStaticBrick* >& bricks, CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( bricks, phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, uint32& Durability, float& Weight, uint16& Dmg, float& Speed, uint16& SapLoad, float& Range, uint16& MaxSlashingProtection, float& SlashingProtectionFactor, uint16& MaxBluntProtection, float& BluntProtectionFactor, uint16& MaxPiercingProtection, float& PiercingProtectionFactor, uint16& DodgeModifier, uint16& ParryModifier, uint8& Color )
	{
		Durability += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Durability;
		Weight += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Weight;
		SapLoad += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SapLoad;

		MaxSlashingProtection += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MaxSlashingProtection;
		SlashingProtectionFactor += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].SlashingProtectionFactor;
		MaxBluntProtection += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MaxBluntProtection;
		BluntProtectionFactor += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].BluntProtectionFactor;
		MaxPiercingProtection += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].MaxPiercingProtection;
		PiercingProtectionFactor += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].PiercingProtectionFactor;
		
		Dmg += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DMG;
		Speed += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Speed;
		Range += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].Range;

		DodgeModifier += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].DodgeBonus;
		ParryModifier += phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ].ParryBonus;
	}
};

///////////// Light Armor //////////////
// Light boots specialized class
class CFaberActionMakeLightBoots : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeLightBoots, ITEM_TYPE::LIGHT_BOOTS )

// Light gloves specialized class
class CFaberActionMakeLightGloves : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeLightGloves, ITEM_TYPE::LIGHT_GLOVES )

// Light pants specialized class
class CFaberActionMakeLightPants : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeLightPants, ITEM_TYPE::LIGHT_PANTS )

// Light sleeves specialized class
class CFaberActionMakeLightSleeves : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeLightSleeves, ITEM_TYPE::LIGHT_SLEEVES )

// Light vest specialized class
class CFaberActionMakeLightVest : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeLightVest, ITEM_TYPE::LIGHT_VEST )


///////////// Medium Armor //////////////
// Medium boots specialized class
class CFaberActionMakeMediumBoots : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMediumBoots, ITEM_TYPE::MEDIUM_BOOTS )

// Medium gloves specialized class
class CFaberActionMakeMediumGloves : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMediumGloves, ITEM_TYPE::MEDIUM_GLOVES )

// Medium pants specialized class
class CFaberActionMakeMediumPants : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMediumPants, ITEM_TYPE::MEDIUM_PANTS )

// Medium sleeves specialized class
class CFaberActionMakeMediumSleeves : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMediumSleeves, ITEM_TYPE::MEDIUM_SLEEVES )

// Medium vest specialized class
class CFaberActionMakeMediumVest : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMediumVest, ITEM_TYPE::MEDIUM_VEST )


///////////// Heavy Armor //////////////
// Heavy boots specialized class
class CFaberActionMakeHeavyBoots : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHeavyBoots, ITEM_TYPE::HEAVY_BOOTS )

// Heavy gloves specialized class
class CFaberActionMakeHeavyGloves : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHeavyGloves, ITEM_TYPE::HEAVY_GLOVES )

// Heavy pants specialized class
class CFaberActionMakeHeavyPants : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHeavyPants, ITEM_TYPE::HEAVY_PANTS )

// Heavy sleeves specialized class
class CFaberActionMakeHeavySleeves : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHeavySleeves, ITEM_TYPE::HEAVY_SLEEVES )

// Heavy vest specialized class
class CFaberActionMakeHeavyVest : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHeavyVest, ITEM_TYPE::HEAVY_VEST )

// Heavy helmet specialized class
class CFaberActionMakeHeavyHelmet : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeHeavyHelmet, ITEM_TYPE::HEAVY_HELMET )


///////////// Shield & buckler //////////////
// Shield specialized class
class CFaberActionMakeShield : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeShield, ITEM_TYPE::SHIELD )

// Buckler specialized class
class CFaberActionMakeBuckler : public CFaberActionMakeArmor
{
};
FABER_ACTION_FACTORY( CFaberActionMakeBuckler, ITEM_TYPE::BUCKLER )
