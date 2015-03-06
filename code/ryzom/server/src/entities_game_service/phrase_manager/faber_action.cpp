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

#include "entity_structure/statistic.h"
#include "egs_sheets/egs_static_brick.h"
#include "game_item_manager/game_item.h"
#include "faber_action.h"
#include "phrase_manager/faber_phrase.h"
#include "entity_manager/entity_manager.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "progression/progression_pve.h"
#include "progression/progression_pvp.h"
#include "game_item_manager/game_item_manager.h"
#include "entities_game_service.h"
#include "phrase_manager/s_effect.h"
#include "mission_manager/mission_event.h"
#include "server_share/log_item_gen.h"


extern NLMISC::CRandom RandomGenerator;

using namespace std;
using namespace NLMISC;

std::vector< std::pair< ITEM_TYPE::TItemType , IFaberActionFactory* > >IFaberActionFactory::Factories;

CVariable<uint32> NBMeanCraftRawMaterials("egs","NBMeanCraftRawMaterials", "Mean of raw material used for craft an item, it's used for scale xp win when crafting an item with effective raw material used", 10, 0, true );

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
	static bool checkSentenceValidity( CFaberPhrase * phrase )
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Roll success factor
	static float rollSuccessFactor( CCharacter* c, CFaberPhrase * phrase, sint& deltaLvl )
	{
		const CStaticBrick	*faberPlan= phrase->getRootFaberPlan();
		if(!faberPlan)
			return 0.f;
		
		uint8 roll =(uint8) RandomGenerator.rand(99);

		// add spire effect ( craft )
		const CSEffect* pEffect = c->lookForActiveEffect( EFFECT_FAMILIES::TotemCraftSuc );
		if ( pEffect != NULL )
		{
			if ( roll <= pEffect->getParamValue() )
				roll = 0;
			else
				roll -= (uint8)pEffect->getParamValue();
		}

		sint32 skillValue = c->getSkillValue( faberPlan->getSkill(0) );
		
/*		TODO: get effect on skill if exist
		const CSEffectPtr debuff = c->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillFaber );
		if ( debuff) skillValue -= debuff->getParamValue();
		if ( skillValue < 0 ) skillValue = 0;
*/		
		const CSEffect* buffOutpost = c->lookForActiveEffect( EFFECT_FAMILIES::OutpostCraft );
		if ( buffOutpost) 
			skillValue += buffOutpost->getParamValue();

		deltaLvl = skillValue + c->craftSuccessModifier() - (sint32) min( (sint32)phrase->getLowerRmQuality(), (sint32) phrase->getRecommendedSkill() );

		float	sf= CStaticSuccessTable::getSuccessFactor( SUCCESS_TABLE_TYPE::Craft, deltaLvl, roll );

		// If the faber plan does not allow any partial success
		if( faberPlan->Faber && !faberPlan->Faber->AllowPartialSuccess )
		{
			// consider a partial success as a full success
			if(sf>0.f)
				sf= 1.f;
		}

		return sf;
	}
	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// get success factor and send message to client
	static float getSuccessFactor( CCharacter* c, CFaberPhrase * phrase, sint& deltaLvl )
	{
		// test forced failure
		if (PHRASE_UTILITIES::forceActionFailure(c) )
		{
			return 0.0f;
		}

		float successFactor = rollSuccessFactor( c, phrase, deltaLvl );
		if( successFactor == 0.0f )
		{
			//Failure
			PHRASE_UTILITIES::sendDynamicSystemMessage(phrase->getActor(), "FABER_MISS");
		}
		else
		{
			if( successFactor < 1.0f )
			{
				//Partial success
				PHRASE_UTILITIES::sendDynamicSystemMessage(phrase->getActor(), "FABER_PARTIAL_SUCCESS");
			}
			else
			{
				//Normal success
				PHRASE_UTILITIES::sendDynamicSystemMessage(phrase->getActor(), "FABER_SUCCESS");
			}
		}
		return successFactor;
	}

	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// create a system crafted item
	static CGameItemPtr createSystemCraftedItem( uint32 quantity, const NLMISC::CSheetId& sheet )
	{
		if (quantity == 0) return NULL;
		
		CGameItemPtr item = GameItemManager.createItem( sheet, (uint16)1, true,true, CEntityId::Unknown  );
		if( item == NULL)
		{
			nlwarning("<CFaberActionCommon::createACraftedItem> Error while creating item %s -> returned a NULL pointer", sheet.toString().c_str() );
			return NULL;
		}
		quantity = min(quantity, item->getMaxStackSize());
		item->setStackSize(quantity);
		return item;
	} // createSystemCraftedItem //
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create crafted item, consume Mps and character xp gain
	static void createCraftedItem( CFaberPhrase * phrase, CCharacter * c, SKILLS::ESkills skill, sint16 deltaLvl, const CSheetId& sheet, uint16 requiredLevel, uint32 nbItemsPerUnit, const CCraftParameters& params, float successFactor )
	{
		vector<CGameItemPtr> itemStacks; // not a stack if only 1 element
		if( c != 0 )
		{
			uint32 nbItemsRemaining = nbItemsPerUnit;
			while ( nbItemsRemaining > 0 )
			{
				CGameItemPtr item = GameItemManager.createItem( sheet , requiredLevel, true, true, c->getId()  );
				if( item == NULL)
				{
					nlwarning("<createCraftedItem> Error while creating item %s -> returned a NULL pointer", sheet.toString().c_str() );
					return;
				}
				uint32 maxStackSize = min(nbItemsPerUnit, item->getMaxStackSize());
				item->setStackSize(min(maxStackSize, nbItemsRemaining));
				itemStacks.push_back( item );
				nbItemsRemaining -= item->getStackSize();
			}
			// Test result
			if( itemStacks.empty() )
			{
				PHRASE_UTILITIES::sendDynamicSystemMessage(phrase->getActor(), "FABER_CREATE_ITEM_FAIL");
				c->sendCloseTempInventoryImpulsion();
				return;
			}
		}
		else
		{
			// System item
			CGameItemPtr item = createSystemCraftedItem( nbItemsPerUnit, sheet );
			itemStacks.push_back( item );
		}
		
		if( ! itemStacks.empty() )
		{
			// Set craft parameters
			uint nbItems = 0;
			for ( vector<CGameItemPtr>::iterator itSt=itemStacks.begin(); itSt!=itemStacks.end(); ++itSt )
			{
				CGameItemPtr& item = *itSt;
				nbItems += 1;
				item->setCraftParameters( params );
				
				for( uint j = 0; j < phrase->getMps().size(); ++j )
				{
					item->addRmUsedForCraft( phrase->getMps()[ j ]->SheetId );
				}
			}
			
			if( c != 0 )
			{
				// Add item(s) to Temp Inventory
				c->enterTempInventoryMode(TEMP_INV_MODE::Craft);

				for ( vector<CGameItemPtr>::iterator itSt=itemStacks.begin(); itSt!=itemStacks.end(); ++itSt )
				{
					CGameItemPtr& item = *itSt;

					if ( !c->addItemToInventory(INVENTORIES::temporary, item) )
					{
						if( item != 0 ) item.deleteItem();
						PHRASE_UTILITIES::sendDynamicSystemMessage(phrase->getActor(), "FABER_TEMP_INVENTORY_FULL");

						// Delete remaining stacks so that the vector 'items' contains only valid items
						uint sizeOk = (uint)itemStacks.size();
						if ( sizeOk == 0 )
							c->sendCloseTempInventoryImpulsion();
						++itSt;
						while ( itSt!=itemStacks.end() )
						{
							if( (*itSt) != 0 ) (*itSt).deleteItem();
							++itSt;
						}
						itemStacks.resize( sizeOk );
						break;
					}
				}

				//Consume Mps
				c->consumeFaberRms();

				// Try to unblock a mission step for the player c and if it does not work try for team mates
				CMissionEventCraft event( sheet, nbItemsPerUnit, requiredLevel );
				c->processMissionEventWithTeamMate(event);

				// action report for xp gain
				if( skill != SKILLS::unknown )
				{
					TReportAction report;
					report.ActorRowId = c->getEntityRowId();
					report.ActionNature = ACTNATURE::CRAFT;
					report.DeltaLvl = deltaLvl;
					report.Skill = skill;
					report.factor = successFactor;
					report.Focus = (uint32) phrase->getFocusCost();

					// as this is a craft action, the player must have a crafting tool in right hand
					CGameItemPtr tool = c->getRightHandItem();
					if (tool == NULL)
					{
						nlwarning("For player %s Faber action ends without a tool in hand ! error", c->getId().toString().c_str());
					}
					else
					{
						if ( tool->getItemWornState() == ITEM_WORN_STATE::Worned )
						{
							const CStaticItem * form = tool->getStaticForm();
							if( form )
							{
								const string msgName = ITEM_WORN_STATE::getMessageForState(ITEM_WORN_STATE::Worned);
								if ( !msgName.empty())
								{
									SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
									params[0].SheetId = tool->getSheetId();
									PHRASE_UTILITIES::sendDynamicSystemMessage( report.ActorRowId, msgName, params);
								}
							}
						}
						else
						{
							c->wearRightHandItem(phrase->getMps().size()/10);

							// report Xp Gain unless used tool is worned
							PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( report, true, false );
							PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(report);
						}

						// wear armor, shield and jewels
						c->wearArmor();
						c->wearShield();
						c->wearJewels();
					}
				}
			}
			else
			{
				// System item
				phrase->setSystemCraftedItem( itemStacks.front() );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Display stat
	static void displayStat(CCharacter *character, float successFactor, SKILLS::ESkills skill)
	{
		if(!character)
			return;
		if(skill==SKILLS::unknown)
			return;

		CSheetId hl, hr;
		uint32 qualityl, qualityr;

		CGameItemPtr item = character->getItem( INVENTORIES::handling, INVENTORIES::left );
		if( item == 0 )
		{
			qualityl = 0;
		}
		else
		{
			hl = item->getSheetId();
			qualityl = item->quality();
		}
		item = character->getItem( INVENTORIES::handling, INVENTORIES::right );
		if( item == 0 )
		{
			qualityr = 0;
		}
		else
		{
			hr = item->getSheetId();
			qualityr = item->quality();
		}
		/*Bsi.append( 
			StatPath, 
			NLMISC::toString("[EAA] %s %s %d %s %d %1.2f %s %d", 
				character->getId().toString().c_str(), 
				hl.toString().c_str(), 
				qualityl, 
				hr.toString().c_str(), 
				qualityr
				, successFactor, SKILLS::toString(skill).c_str(), character->getSkills()._Skills[skill].Current )
			);*/

		/*
		EgsStat.displayNL("[EAA] %s %s %d %s %d %1.2f %s %d", 
				character->getId().toString().c_str(), 
				hl.toString().c_str(), 
				qualityl, 
				hr.toString().c_str(), 
				qualityr
				, successFactor, SKILLS::toString(skill).c_str(), character->getSkills()._Skills[skill].Current);
		*/
//		EGSPD::executeActionFaber(character->getId(), hl.toString(), qualityl, hr.toString(), qualityr, successFactor, SKILLS::toString(skill), character->getSkills()._Skills[skill].Current);
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

	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, CCraftParameters& params, uint64 &statBF ) = 0;
	
protected:
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void apply(CFaberPhrase * phrase)
	{
		CCharacter* character = ( CCharacter * ) CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if( character )
		{
			// compute success factor
			sint deltaLvl;
			float successFactor = CFaberActionCommon::getSuccessFactor( character, phrase, deltaLvl );

			// final item quality, depending of recommended skill of action used and lower quality of raw materials used and success factor of action
			uint16 finalItemQuality = max( (uint16)1, (uint16) (min(phrase->getLowerRmQuality(),(uint16)phrase->getRecommendedSkill()) * successFactor) );
			
			// partial, normal or critical success
			if( successFactor > 0.0f )
			{
				CCraftParameters	Params;
				uint64				statBitField= 0;

				uint16 recommendedSkill = 0;

				uint32 mpOccurence = 0;
				
				// parsing faber plan 
				uint32 neededMp = (uint32)phrase->getRootFaberPlan()->Faber->NeededMps.size();
				for( uint mp = 0; mp < neededMp; ++mp )
				{
					//for each type of Mp needed
					for( uint k = 0; k < phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].Quantity; ++k )
					{
						// for each Mp of one type (we have Quantity by type)
						uint32 NumMpParameters = (uint32)phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters.size();
						// for each Faber parameters in Mp
						for( uint j = 0; j < NumMpParameters; ++j )
						{
							// check if Mp Type match with Faber waiting Type
							if( phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].MpType == phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ j ].MpFaberType )
							{
								specializedApply( phrase, mpOccurence, j, Params, statBitField);
								break;
							}
						}
						++mpOccurence;
					}
				}
				
				// Scalling parameters to 0 - 1.0 according to number raw material used for each params
				if( Params.nbStatEnergy > 0 ) Params.StatEnergy = Params.StatEnergy / Params.nbStatEnergy;
				if( Params.nbDurability > 0 ) Params.Durability = Params.Durability / Params.nbDurability;
				if( Params.nbWeight > 0 ) Params.Weight = Params.Weight / Params.nbWeight;
				if( Params.nbDmg > 0 ) Params.Dmg = Params.Dmg / Params.nbDmg;
				if( Params.nbSpeed > 0 ) Params.Speed = Params.Speed / Params.nbSpeed;
				if( Params.nbSapLoad > 0 ) Params.SapLoad = Params.SapLoad / Params.nbSapLoad;
				if( Params.nbRange > 0 ) Params.Range = Params.Range / Params.nbRange;

				if( Params.nbDodgeModifier > 0 ) Params.DodgeModifier = Params.DodgeModifier / Params.nbDodgeModifier;
				if( Params.nbParryModifier > 0 ) Params.ParryModifier = Params.ParryModifier / Params.nbParryModifier;
				if( Params.nbAdversaryDodgeModifier > 0 ) Params.AdversaryDodgeModifier = Params.AdversaryDodgeModifier / Params.nbAdversaryDodgeModifier;
				if( Params.nbAdversaryParryModifier > 0 ) Params.AdversaryParryModifier = Params.AdversaryParryModifier / Params.nbAdversaryParryModifier;
				
				if( Params.nbProtectionFactor > 0 ) Params.ProtectionFactor = Params.ProtectionFactor / Params.nbProtectionFactor;
				if( Params.nbMaxSlashingProtection > 0 ) Params.MaxSlashingProtection = Params.MaxSlashingProtection / Params.nbMaxSlashingProtection;
				if( Params.nbMaxBluntProtection > 0 ) Params.MaxBluntProtection = Params.MaxBluntProtection / Params.nbMaxBluntProtection;
				if( Params.nbMaxPiercingProtection > 0 ) Params.MaxPiercingProtection = Params.MaxPiercingProtection / Params.nbMaxPiercingProtection;

				if( Params.nbAcidProtectionFactor > 0 ) Params.AcidProtectionFactor = Params.AcidProtectionFactor / Params.nbAcidProtectionFactor;
				if( Params.nbColdProtectionFactor > 0 ) Params.ColdProtectionFactor = Params.ColdProtectionFactor / Params.nbColdProtectionFactor;
				if( Params.nbFireProtectionFactor > 0 ) Params.FireProtectionFactor = Params.FireProtectionFactor / Params.nbFireProtectionFactor;
				if( Params.nbRotProtectionFactor > 0 ) Params.RotProtectionFactor = Params.RotProtectionFactor / Params.nbRotProtectionFactor;
				if( Params.nbShockWaveProtectionFactor > 0 ) Params.ShockWaveProtectionFactor = Params.ShockWaveProtectionFactor / Params.nbShockWaveProtectionFactor;
				if( Params.nbPoisonProtectionFactor > 0 ) Params.PoisonProtectionFactor = Params.PoisonProtectionFactor / Params.nbPoisonProtectionFactor;
				if( Params.nbElectricityProtectionFactor > 0 ) Params.ElectricityProtectionFactor = Params.ElectricityProtectionFactor / Params.nbElectricityProtectionFactor;

				if( Params.nbDesertResistanceFactor > 0 ) Params.DesertResistanceFactor = Params.DesertResistanceFactor / Params.nbDesertResistanceFactor;
				if( Params.nbForestResistanceFactor > 0 ) Params.ForestResistanceFactor = Params.ForestResistanceFactor / Params.nbForestResistanceFactor;
				if( Params.nbLacustreResistanceFactor > 0 ) Params.LacustreResistanceFactor = Params.LacustreResistanceFactor / Params.nbLacustreResistanceFactor;
				if( Params.nbJungleResistanceFactor > 0 ) Params.JungleResistanceFactor = Params.JungleResistanceFactor / Params.nbJungleResistanceFactor;
				if( Params.nbPrimaryRootResistanceFactor > 0 ) Params.PrimaryRootResistanceFactor = Params.PrimaryRootResistanceFactor / Params.nbPrimaryRootResistanceFactor;

				if( Params.nbElementalCastingTimeFactor > 0 ) Params.ElementalCastingTimeFactor /= Params.nbElementalCastingTimeFactor;
				if( Params.nbElementalPowerFactor > 0 ) Params.ElementalPowerFactor /= Params.nbElementalPowerFactor;
				if( Params.nbOffensiveAfflictionCastingTimeFactor > 0 ) Params.OffensiveAfflictionCastingTimeFactor /= Params.nbOffensiveAfflictionCastingTimeFactor;
				if( Params.nbOffensiveAfflictionPowerFactor > 0 ) Params.OffensiveAfflictionPowerFactor /= Params.nbOffensiveAfflictionPowerFactor;
				if( Params.nbHealCastingTimeFactor > 0 ) Params.HealCastingTimeFactor /= Params.nbHealCastingTimeFactor;
				if( Params.nbHealPowerFactor > 0 ) Params.HealPowerFactor /= Params.nbHealPowerFactor;
				if( Params.nbDefensiveAfflictionCastingTimeFactor > 0 ) Params.DefensiveAfflictionCastingTimeFactor /= Params.nbDefensiveAfflictionCastingTimeFactor;
				if( Params.nbDefensiveAfflictionPowerFactor > 0 ) Params.DefensiveAfflictionPowerFactor /= Params.nbDefensiveAfflictionPowerFactor;
				
				// Apply success factor and MBO modifiers factor
				Params.StatEnergy = Params.StatEnergy; 
				Params.Durability = Params.Durability + phrase->getMBODurability();
				Params.Weight = Params.Weight + phrase->getMBOWeight();
				Params.Dmg = Params.Dmg + phrase->getMBODmg();
				Params.Speed = Params.Speed + phrase->getMBOSpeed();
				Params.SapLoad = Params.SapLoad + phrase->getMBOSapLoad();
				Params.Range = Params.Range + phrase->getMBORange();
				
				Params.MaxSlashingProtection = Params.MaxSlashingProtection + phrase->getMBOProtection();
				Params.MaxBluntProtection = Params.MaxBluntProtection + phrase->getMBOProtection();
				Params.MaxPiercingProtection = Params.MaxPiercingProtection + phrase->getMBOProtection();

				float buffFactor = successFactor * ((float)((sint32)min((sint32)phrase->getLowerRmQuality(),(sint32)phrase->getRecommendedSkill()))) / (float)phrase->getRecommendedSkill();

				Params.HpBuff = (sint32)( ( Params.HpBuff + phrase->getMBOHitPoint() ) *  buffFactor );
				Params.SapBuff = (sint32)( ( Params.SapBuff + phrase->getMBOSap() ) * buffFactor );
				Params.StaBuff = (sint32)( ( Params.StaBuff + phrase->getMBOStamina() ) * buffFactor );
				Params.FocusBuff = (sint32)( ( Params.FocusBuff + phrase->getMBOFocus() ) * buffFactor );

				// apply plan stat bonus
				Params.HpBuff += sint32(phrase->getRootFaberPlan()->Faber->HpBonusPerLevel * finalItemQuality);
				Params.SapBuff += sint32(phrase->getRootFaberPlan()->Faber->SapBonusPerLevel * finalItemQuality);
				Params.StaBuff += sint32(phrase->getRootFaberPlan()->Faber->StaBonusPerLevel * finalItemQuality);
				Params.FocusBuff += sint32(phrase->getRootFaberPlan()->Faber->FocusBonusPerLevel * finalItemQuality);
				
				// apply procs
				{
					std::vector<SItemSpecialEffect> effects = character->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_STAT_BONUS);
					std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
					for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
					{
						float rnd = RandomGenerator.frand();
						if (rnd<it->EffectArgFloat[0])
						{
							sint32* pbuff = NULL;

							// build random filters
							vector<bool> scoresAllowed;
							scoresAllowed.resize(SCORES::NUM_SCORES,true);
							ITEMFAMILY::EItemFamily family	= phrase->getCraftedItemStaticForm()->Family;
							ITEM_TYPE::TItemType	type	= phrase->getCraftedItemStaticForm()->Type;
							if( type == ITEM_TYPE::MAGICIAN_STAFF )
							{
								scoresAllowed[SCORES::stamina]=false;
								scoresAllowed[SCORES::focus]=false;
							}
							else if( type==ITEM_TYPE::HEAVY_BOOTS || type==ITEM_TYPE::HEAVY_GLOVES || type==ITEM_TYPE::HEAVY_PANTS ||
									 type==ITEM_TYPE::HEAVY_SLEEVES || type==ITEM_TYPE::HEAVY_VEST || type==ITEM_TYPE::HEAVY_HELMET )
							{
								scoresAllowed[SCORES::focus]=false;
								scoresAllowed[SCORES::sap]=false;
							}
							else if( type==ITEM_TYPE::MEDIUM_BOOTS || type==ITEM_TYPE::MEDIUM_GLOVES || type==ITEM_TYPE::MEDIUM_PANTS ||
									 type==ITEM_TYPE::MEDIUM_SLEEVES || type==ITEM_TYPE::MEDIUM_VEST )
							{
								scoresAllowed[SCORES::focus]=false;
							}
							else if( family==ITEMFAMILY::MELEE_WEAPON || family==ITEMFAMILY::RANGE_WEAPON || family==ITEMFAMILY::SHIELD )
							{
								scoresAllowed[SCORES::focus]=false;
								scoresAllowed[SCORES::sap]=false;
							}
							else if( family==ITEMFAMILY::CRAFTING_TOOL || family==ITEMFAMILY::HARVEST_TOOL )
							{
								scoresAllowed[SCORES::hit_points]=false;
								scoresAllowed[SCORES::stamina]=false;
							}
							// choose score using filtered random
							uint32 randomScoreIndex = (uint32)RandomGenerator.rand((uint16)(SCORES::NUM_SCORES-1));
							while( scoresAllowed[randomScoreIndex]==false )
							{
								randomScoreIndex = (randomScoreIndex+1)%SCORES::NUM_SCORES;
							}
							SCORES::TScores stat = (SCORES::TScores)randomScoreIndex;

							switch (stat)
							{
							case SCORES::hit_points: pbuff = &Params.HpBuff; break;
							case SCORES::stamina: pbuff = &Params.StaBuff; break;
							case SCORES::sap: pbuff = &Params.SapBuff; break;
							case SCORES::focus: pbuff = &Params.FocusBuff; break;
							}
							if (pbuff)
								*pbuff += (sint32)it->EffectArgFloat[1] + (sint32)max((uint16)1,(uint16)(finalItemQuality/10));
							PHRASE_UTILITIES::sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_STAT_BONUS, character, NULL, (sint32)stat, *pbuff);
						}
					}
				}
				
				// Clamp factors
				clamp(Params.StatEnergy,0.f, 1.f);
				clamp(Params.Durability,0.f, 1.f);
				clamp(Params.Weight,0.f, 1.f);
				clamp(Params.Dmg,0.f, 1.f);
				clamp(Params.Speed,0.f, 1.f);
				clamp(Params.SapLoad,0.f, 1.f);
				clamp(Params.Range,0.f, 1.f);
				
				clamp(Params.DodgeModifier,0.f, 1.f);
				clamp(Params.ParryModifier,0.f, 1.f);
				clamp(Params.AdversaryDodgeModifier,0.f, 1.f);
				clamp(Params.AdversaryParryModifier,0.f, 1.f);
				
				clamp(Params.ProtectionFactor,0.f, 1.f);
				clamp(Params.MaxSlashingProtection,0.f, 1.f);
				clamp(Params.MaxBluntProtection,0.f, 1.f);
				clamp(Params.MaxPiercingProtection,0.f, 1.f);
				
				clamp(Params.AcidProtectionFactor, 0.f, 1.f);
				clamp(Params.ColdProtectionFactor, 0.f, 1.f);
				clamp(Params.FireProtectionFactor, 0.f, 1.f);
				clamp(Params.RotProtectionFactor, 0.f, 1.f);
				clamp(Params.ShockWaveProtectionFactor, 0.f, 1.f);
				clamp(Params.PoisonProtectionFactor, 0.f, 1.f);
				clamp(Params.ElectricityProtectionFactor, 0.f, 1.f);

				clamp(Params.DesertResistanceFactor, 0.f, 1.f);
				clamp(Params.ForestResistanceFactor, 0.f, 1.f);
				clamp(Params.LacustreResistanceFactor, 0.f, 1.f);
				clamp(Params.JungleResistanceFactor, 0.f, 1.f);
				clamp(Params.PrimaryRootResistanceFactor, 0.f, 1.f);
				
				clamp(Params.ElementalCastingTimeFactor, 0.f, 1.f);
				clamp(Params.ElementalPowerFactor, 0.f, 1.f);
				clamp(Params.OffensiveAfflictionCastingTimeFactor, 0.f, 1.f);
				clamp(Params.OffensiveAfflictionPowerFactor, 0.f, 1.f);
				clamp(Params.HealCastingTimeFactor, 0.f, 1.f);
				clamp(Params.HealPowerFactor, 0.f, 1.f);
				clamp(Params.DefensiveAfflictionCastingTimeFactor, 0.f, 1.f);
				clamp(Params.DefensiveAfflictionPowerFactor, 0.f, 1.f);
				
				// Yoyo: Stretch the stats for more interesting items
				stretchItemStats(Params, statBitField);

				// apply plan factor, result factors are unclamped, because can be greater than 1.0f
				Params.Durability = Params.Durability * phrase->getRootFaberPlan()->Faber->Durability;
				Params.Weight = Params.Weight * phrase->getRootFaberPlan()->Faber->Weight;
				Params.Dmg = Params.Dmg * phrase->getRootFaberPlan()->Faber->Dmg;
				Params.Speed = Params.Speed * phrase->getRootFaberPlan()->Faber->Speed;
				Params.SapLoad = Params.SapLoad * phrase->getRootFaberPlan()->Faber->SapLoad;
				Params.Range = Params.Range * phrase->getRootFaberPlan()->Faber->Range;
				
				Params.DodgeModifier = Params.DodgeModifier * phrase->getRootFaberPlan()->Faber->DodgeModifier;
				Params.ParryModifier = Params.ParryModifier * phrase->getRootFaberPlan()->Faber->ParryModifier;
				Params.AdversaryDodgeModifier = Params.AdversaryDodgeModifier * phrase->getRootFaberPlan()->Faber->AdversaryDodgeModifier;
				Params.AdversaryParryModifier = Params.AdversaryParryModifier * phrase->getRootFaberPlan()->Faber->AdversaryParryModifier;
				
				Params.ProtectionFactor = Params.ProtectionFactor * phrase->getRootFaberPlan()->Faber->ProtectionFactor;
				Params.MaxSlashingProtection = Params.MaxSlashingProtection * phrase->getRootFaberPlan()->Faber->MaxSlashingProtection;
				Params.MaxBluntProtection = Params.MaxBluntProtection * phrase->getRootFaberPlan()->Faber->MaxBluntProtection;
				Params.MaxPiercingProtection = Params.MaxPiercingProtection * phrase->getRootFaberPlan()->Faber->MaxPiercingProtection;
				
				Params.AcidProtectionFactor = Params.AcidProtectionFactor * phrase->getRootFaberPlan()->Faber->AcidProtectionFactor;
				Params.ColdProtectionFactor = Params.ColdProtectionFactor * phrase->getRootFaberPlan()->Faber->ColdProtectionFactor;
				Params.FireProtectionFactor = Params.FireProtectionFactor * phrase->getRootFaberPlan()->Faber->FireProtectionFactor;
				Params.RotProtectionFactor = Params.RotProtectionFactor * phrase->getRootFaberPlan()->Faber->RotProtectionFactor;
				Params.ShockWaveProtectionFactor = Params.ShockWaveProtectionFactor * phrase->getRootFaberPlan()->Faber->ShockWaveProtectionFactor;
				Params.PoisonProtectionFactor = Params.PoisonProtectionFactor * phrase->getRootFaberPlan()->Faber->PoisonProtectionFactor;
				Params.ElectricityProtectionFactor = Params.ElectricityProtectionFactor * phrase->getRootFaberPlan()->Faber->ElectricityProtectionFactor;

				Params.DesertResistanceFactor = Params.DesertResistanceFactor * phrase->getRootFaberPlan()->Faber->DesertResistanceFactor;
				Params.ForestResistanceFactor = Params.ForestResistanceFactor * phrase->getRootFaberPlan()->Faber->ForestResistanceFactor;
				Params.LacustreResistanceFactor = Params.LacustreResistanceFactor * phrase->getRootFaberPlan()->Faber->LacustreResistanceFactor;
				Params.JungleResistanceFactor = Params.JungleResistanceFactor * phrase->getRootFaberPlan()->Faber->JungleResistanceFactor;
				Params.PrimaryRootResistanceFactor = Params.PrimaryRootResistanceFactor * phrase->getRootFaberPlan()->Faber->PrimaryRootResistanceFactor;
				
				Params.ElementalCastingTimeFactor = Params.ElementalCastingTimeFactor * phrase->getRootFaberPlan()->Faber->ElementalCastingTimeFactor;
				Params.ElementalPowerFactor = Params.ElementalPowerFactor * phrase->getRootFaberPlan()->Faber->ElementalPowerFactor;
				Params.OffensiveAfflictionCastingTimeFactor = Params.OffensiveAfflictionCastingTimeFactor * phrase->getRootFaberPlan()->Faber->OffensiveAfflictionCastingTimeFactor;
				Params.OffensiveAfflictionPowerFactor = Params.OffensiveAfflictionPowerFactor * phrase->getRootFaberPlan()->Faber->OffensiveAfflictionPowerFactor;
				Params.HealCastingTimeFactor = Params.HealCastingTimeFactor * phrase->getRootFaberPlan()->Faber->HealCastingTimeFactor;
				Params.HealPowerFactor = Params.HealPowerFactor * phrase->getRootFaberPlan()->Faber->HealPowerFactor;
				Params.DefensiveAfflictionCastingTimeFactor = Params.DefensiveAfflictionCastingTimeFactor * phrase->getRootFaberPlan()->Faber->DefensiveAfflictionCastingTimeFactor;
				Params.DefensiveAfflictionPowerFactor = Params.DefensiveAfflictionPowerFactor * phrase->getRootFaberPlan()->Faber->DefensiveAfflictionPowerFactor;

				// Apply character bonus, do NOT clamp afterwards
				{
					CCharacter::CBrickProperties properties;
					CCharacter::CBrickPropertyValues::const_iterator it, end;
					// Repeat following block for each modifiable Param
					// Durability[A]
					character->getPropertiesFromKnownBricks(BRICK_FAMILIES::BPBCA, properties);
					for (it=properties[TBrickParam::BONUS_CR_DURABILITY].begin(), end = properties[TBrickParam::BONUS_CR_DURABILITY].end(); it!=end; ++it)
						Params.Durability += ((CSBrickParamBonusCrDurability*)(TBrickParam::IId*)(*it))->Bonus;
					// End Repeat
				}
				
				// apply procs, do NOT clamp afterwards
				{
					std::vector<SItemSpecialEffect> effects = character->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_LIMIT);
					std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
					for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
					{
						float rnd = RandomGenerator.frand();
						double effect0 = it->EffectArgFloat[0];
						double effect1 = it->EffectArgFloat[1];
						// Clamp boost when player do not have required privs
						if (!character->havePriv(":DEV:") && !character->havePriv(":SGM:"))
						{
							clamp(effect0, 0.0, 0.25);
							clamp(effect1, 0.0, 0.25);
						}

						if (rnd < effect0)
						{
							applyProcAddLimit(Params, statBitField, effect1);
							PHRASE_UTILITIES::sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::ISE_CRAFT_ADD_LIMIT, character, NULL, (sint32)(effect1*100.0));
						}
					}
				}
				
				// Compute the delta level for the XP based on the Base skill, not the buffed/unbuffed one.
				sint32 skillValue = character->getSkillBaseValue( phrase->getRootFaberPlan()->getSkill(0) );
				deltaLvl = skillValue - (sint32)min((sint32)phrase->getLowerRmQuality(),(sint32)phrase->getRecommendedSkill());
				// Craft the item and Get XP
				TLogContext_Item_Craft logContext(character->getId());
				CFaberActionCommon::createCraftedItem( phrase, character, phrase->getRootFaberPlan()->getSkill(0), deltaLvl, phrase->getCraftedItemStaticForm()->SheetId, finalItemQuality, phrase->getRootFaberPlan()->Faber->NbItemsPerUnit, Params, successFactor * mpOccurence / NBMeanCraftRawMaterials );

				// Display stat
				CFaberActionCommon::displayStat(character, successFactor, phrase->getRootFaberPlan()->getSkill(0));
			}
			else
			{
				character->consumeFaberRms(true);
				character->sendCloseTempInventoryImpulsion();
			}
		}
	}

	// Apply params for system craft / compute result item (exemple selling item, not crafted by a player)
	virtual void systemApply(CFaberPhrase * phrase)
	{
		// compute success factor
		CCraftParameters	Params;
		uint64				statBitField= 0;
		
		uint16 Quality = 0;
		
		// parsing faber plan 
		uint32 neededMp = (uint32)phrase->getRootFaberPlan()->Faber->NeededMps.size();
		for( uint mp = 0; mp < neededMp; ++mp )
		{
			//for each type of Mp needed
			for( uint k = 0; k < phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].Quantity; ++k )
			{
				// for each Mp of one type (we have Quantity by type)
				uint32 NumMpParameters = (uint32)phrase->getMps()[ mp ]->Mp->MpFaberParameters.size();
				// for each Faber parameters in Mp
				for( uint j = 0; j < NumMpParameters; ++j )
				{
					// check if Mp Type match with Faber waiting Type
					if( phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].MpType == phrase->getMps()[ mp ]->Mp->MpFaberParameters[ j ].MpFaberType )
					{
						specializedApply( phrase, mp, j, Params, statBitField);
						break;
					}
				}
			}
		}
			
		// Scalling parameters to 0 - 100 according to number raw material used for each params
		if( Params.StatEnergy > 0.0f ) Params.StatEnergy = Params.StatEnergy / Params.nbStatEnergy;
		if( Params.nbDurability > 0.0f ) Params.Durability = Params.Durability / Params.nbDurability;
		if( Params.nbWeight > 0.0f ) Params.Weight = Params.Weight / Params.nbWeight;
		if( Params.nbDmg > 0.0f ) Params.Dmg = Params.Dmg / Params.nbDmg;
		if( Params.nbSpeed > 0.0f ) Params.Speed = Params.Speed / Params.nbSpeed;
		if( Params.nbSapLoad > 0.0f ) Params.SapLoad = Params.SapLoad / Params.nbSapLoad;
		if( Params.nbRange > 0.0f ) Params.Range = Params.Range / Params.nbRange;
		
		if( Params.nbDodgeModifier > 0.0f ) Params.DodgeModifier = Params.DodgeModifier / Params.nbDodgeModifier;
		if( Params.nbParryModifier > 0.0f ) Params.ParryModifier = Params.ParryModifier / Params.nbParryModifier;
		if( Params.nbAdversaryDodgeModifier > 0.0f ) Params.AdversaryDodgeModifier = Params.AdversaryDodgeModifier / Params.nbAdversaryDodgeModifier;
		if( Params.nbAdversaryParryModifier > 0.0f ) Params.AdversaryParryModifier = Params.AdversaryParryModifier / Params.nbAdversaryParryModifier;
		
		if( Params.nbProtectionFactor > 0.0f ) Params.ProtectionFactor = Params.ProtectionFactor / Params.nbProtectionFactor;
		if( Params.nbMaxSlashingProtection > 0.0f ) Params.MaxSlashingProtection = Params.MaxSlashingProtection / Params.nbMaxSlashingProtection;
		if( Params.nbMaxBluntProtection > 0.0f ) Params.MaxBluntProtection = Params.MaxBluntProtection / Params.nbMaxBluntProtection;
		if( Params.nbMaxPiercingProtection > 0.0f ) Params.MaxPiercingProtection = Params.MaxPiercingProtection / Params.nbMaxPiercingProtection;
		
		if( Params.nbAcidProtectionFactor > 0 ) Params.AcidProtectionFactor = Params.AcidProtectionFactor / Params.nbAcidProtectionFactor;
		if( Params.nbColdProtectionFactor > 0 ) Params.ColdProtectionFactor = Params.ColdProtectionFactor / Params.nbColdProtectionFactor;
		if( Params.nbFireProtectionFactor > 0 ) Params.FireProtectionFactor = Params.FireProtectionFactor / Params.nbFireProtectionFactor;
		if( Params.nbRotProtectionFactor > 0 ) Params.RotProtectionFactor = Params.RotProtectionFactor / Params.nbRotProtectionFactor;
		if( Params.nbShockWaveProtectionFactor > 0 ) Params.ShockWaveProtectionFactor = Params.ShockWaveProtectionFactor / Params.nbShockWaveProtectionFactor;
		if( Params.nbPoisonProtectionFactor > 0 ) Params.PoisonProtectionFactor = Params.PoisonProtectionFactor / Params.nbPoisonProtectionFactor;
		if( Params.nbElectricityProtectionFactor > 0 ) Params.ElectricityProtectionFactor = Params.ElectricityProtectionFactor / Params.nbElectricityProtectionFactor;

		if( Params.nbDesertResistanceFactor > 0 ) Params.DesertResistanceFactor = Params.DesertResistanceFactor / Params.nbDesertResistanceFactor;
		if( Params.nbForestResistanceFactor > 0 ) Params.ForestResistanceFactor = Params.ForestResistanceFactor / Params.nbForestResistanceFactor;
		if( Params.nbLacustreResistanceFactor > 0 ) Params.LacustreResistanceFactor = Params.LacustreResistanceFactor / Params.nbLacustreResistanceFactor;
		if( Params.nbJungleResistanceFactor > 0 ) Params.JungleResistanceFactor = Params.JungleResistanceFactor / Params.nbJungleResistanceFactor;
		if( Params.nbPrimaryRootResistanceFactor > 0 ) Params.PrimaryRootResistanceFactor = Params.PrimaryRootResistanceFactor / Params.nbPrimaryRootResistanceFactor;

		if( Params.nbElementalCastingTimeFactor > 0 ) Params.ElementalCastingTimeFactor /= Params.nbElementalCastingTimeFactor;
		if( Params.nbElementalPowerFactor > 0 ) Params.ElementalPowerFactor /= Params.nbElementalPowerFactor;
		if( Params.nbOffensiveAfflictionCastingTimeFactor > 0 ) Params.OffensiveAfflictionCastingTimeFactor /= Params.nbOffensiveAfflictionCastingTimeFactor;
		if( Params.nbOffensiveAfflictionPowerFactor > 0 ) Params.OffensiveAfflictionPowerFactor /= Params.nbOffensiveAfflictionPowerFactor;
		if( Params.nbHealCastingTimeFactor > 0 ) Params.HealCastingTimeFactor /= Params.nbHealCastingTimeFactor;
		if( Params.nbHealPowerFactor > 0 ) Params.HealPowerFactor /= Params.nbHealPowerFactor;
		if( Params.nbDefensiveAfflictionCastingTimeFactor > 0 ) Params.DefensiveAfflictionCastingTimeFactor /= Params.nbDefensiveAfflictionCastingTimeFactor;
		if( Params.nbDefensiveAfflictionPowerFactor > 0 ) Params.DefensiveAfflictionPowerFactor /= Params.nbDefensiveAfflictionPowerFactor;
		
		// Clamp factors
		clamp(Params.StatEnergy,0.f, 1.f);
		clamp(Params.Durability,0.f, 1.f);
		clamp(Params.Weight,0.f, 1.f);
		clamp(Params.Dmg,0.f, 1.f);
		clamp(Params.Speed,0.f, 1.f);
		clamp(Params.SapLoad,0.f, 1.f);
		clamp(Params.Range,0.f, 1.f);
		
		clamp(Params.DodgeModifier,0.f, 1.f);
		clamp(Params.ParryModifier,0.f, 1.f);
		clamp(Params.AdversaryDodgeModifier,0.f, 1.f);
		clamp(Params.AdversaryParryModifier,0.f, 1.f);
		
		clamp(Params.ProtectionFactor,0.f, 1.f);
		clamp(Params.MaxSlashingProtection,0.f, 1.f);
		clamp(Params.MaxBluntProtection,0.f, 1.f);
		clamp(Params.MaxPiercingProtection,0.f, 1.f);
		
		clamp(Params.AcidProtectionFactor, 0.f, 1.f);
		clamp(Params.ColdProtectionFactor, 0.f, 1.f);
		clamp(Params.FireProtectionFactor, 0.f, 1.f);
		clamp(Params.RotProtectionFactor, 0.f, 1.f);
		clamp(Params.ShockWaveProtectionFactor, 0.f, 1.f);
		clamp(Params.PoisonProtectionFactor, 0.f, 1.f);
		clamp(Params.ElectricityProtectionFactor, 0.f, 1.f);
		
		clamp(Params.DesertResistanceFactor, 0.f, 1.f);
		clamp(Params.ForestResistanceFactor, 0.f, 1.f);
		clamp(Params.LacustreResistanceFactor, 0.f, 1.f);
		clamp(Params.JungleResistanceFactor, 0.f, 1.f);
		clamp(Params.PrimaryRootResistanceFactor, 0.f, 1.f);
		
		clamp(Params.ElementalCastingTimeFactor, 0.f, 1.f);
		clamp(Params.ElementalPowerFactor, 0.f, 1.f);
		clamp(Params.OffensiveAfflictionCastingTimeFactor, 0.f, 1.f);
		clamp(Params.OffensiveAfflictionPowerFactor, 0.f, 1.f);
		clamp(Params.HealCastingTimeFactor, 0.f, 1.f);
		clamp(Params.HealPowerFactor, 0.f, 1.f);
		clamp(Params.DefensiveAfflictionCastingTimeFactor, 0.f, 1.f);
		clamp(Params.DefensiveAfflictionPowerFactor, 0.f, 1.f);
		
		// Yoyo: Stretch the stats for more interesting items
		stretchItemStats(Params, statBitField);
		
		// apply plan factor, result factors are unclamped, because can be greater than 1.0f
		Params.Durability = Params.Durability * phrase->getRootFaberPlan()->Faber->Durability;
		Params.Weight = Params.Weight * phrase->getRootFaberPlan()->Faber->Weight;
		Params.Dmg = Params.Dmg * phrase->getRootFaberPlan()->Faber->Dmg;
		Params.Speed = Params.Speed * phrase->getRootFaberPlan()->Faber->Speed;
		Params.SapLoad = Params.SapLoad * phrase->getRootFaberPlan()->Faber->SapLoad;
		Params.Range = Params.Range * phrase->getRootFaberPlan()->Faber->Range;
		
		Params.DodgeModifier = Params.DodgeModifier * phrase->getRootFaberPlan()->Faber->DodgeModifier;
		Params.ParryModifier = Params.ParryModifier * phrase->getRootFaberPlan()->Faber->ParryModifier;
		Params.AdversaryDodgeModifier = Params.AdversaryDodgeModifier * phrase->getRootFaberPlan()->Faber->AdversaryDodgeModifier;
		Params.AdversaryParryModifier = Params.AdversaryParryModifier * phrase->getRootFaberPlan()->Faber->AdversaryParryModifier;
		
		Params.ProtectionFactor = Params.ProtectionFactor * phrase->getRootFaberPlan()->Faber->ProtectionFactor;
		Params.MaxSlashingProtection = Params.MaxSlashingProtection * phrase->getRootFaberPlan()->Faber->MaxSlashingProtection;
		Params.MaxBluntProtection = Params.MaxBluntProtection * phrase->getRootFaberPlan()->Faber->MaxBluntProtection;
		Params.MaxPiercingProtection = Params.MaxPiercingProtection * phrase->getRootFaberPlan()->Faber->MaxPiercingProtection;
		
		Params.AcidProtectionFactor = Params.AcidProtectionFactor * phrase->getRootFaberPlan()->Faber->AcidProtectionFactor;
		Params.ColdProtectionFactor = Params.ColdProtectionFactor * phrase->getRootFaberPlan()->Faber->ColdProtectionFactor;
		Params.FireProtectionFactor = Params.FireProtectionFactor * phrase->getRootFaberPlan()->Faber->FireProtectionFactor;
		Params.RotProtectionFactor = Params.RotProtectionFactor * phrase->getRootFaberPlan()->Faber->RotProtectionFactor;
		Params.ShockWaveProtectionFactor = Params.ShockWaveProtectionFactor * phrase->getRootFaberPlan()->Faber->ShockWaveProtectionFactor;
		Params.PoisonProtectionFactor = Params.PoisonProtectionFactor * phrase->getRootFaberPlan()->Faber->PoisonProtectionFactor;
		Params.ElectricityProtectionFactor = Params.ElectricityProtectionFactor * phrase->getRootFaberPlan()->Faber->ElectricityProtectionFactor;
		
		Params.DesertResistanceFactor = Params.DesertResistanceFactor * phrase->getRootFaberPlan()->Faber->DesertResistanceFactor;
		Params.ForestResistanceFactor = Params.ForestResistanceFactor * phrase->getRootFaberPlan()->Faber->ForestResistanceFactor;
		Params.LacustreResistanceFactor = Params.LacustreResistanceFactor * phrase->getRootFaberPlan()->Faber->LacustreResistanceFactor;
		Params.JungleResistanceFactor = Params.JungleResistanceFactor * phrase->getRootFaberPlan()->Faber->JungleResistanceFactor;
		Params.PrimaryRootResistanceFactor = Params.PrimaryRootResistanceFactor * phrase->getRootFaberPlan()->Faber->PrimaryRootResistanceFactor;

		Params.ElementalCastingTimeFactor = Params.ElementalCastingTimeFactor * phrase->getRootFaberPlan()->Faber->ElementalCastingTimeFactor;
		Params.ElementalPowerFactor = Params.ElementalPowerFactor * phrase->getRootFaberPlan()->Faber->ElementalPowerFactor;
		Params.OffensiveAfflictionCastingTimeFactor = Params.OffensiveAfflictionCastingTimeFactor * phrase->getRootFaberPlan()->Faber->OffensiveAfflictionCastingTimeFactor;
		Params.OffensiveAfflictionPowerFactor = Params.OffensiveAfflictionPowerFactor * phrase->getRootFaberPlan()->Faber->OffensiveAfflictionPowerFactor;
		Params.HealCastingTimeFactor = Params.HealCastingTimeFactor * phrase->getRootFaberPlan()->Faber->HealCastingTimeFactor;
		Params.HealPowerFactor = Params.HealPowerFactor * phrase->getRootFaberPlan()->Faber->HealPowerFactor;
		Params.DefensiveAfflictionCastingTimeFactor = Params.DefensiveAfflictionCastingTimeFactor * phrase->getRootFaberPlan()->Faber->DefensiveAfflictionCastingTimeFactor;
		Params.DefensiveAfflictionPowerFactor = Params.DefensiveAfflictionPowerFactor * phrase->getRootFaberPlan()->Faber->DefensiveAfflictionPowerFactor;
		
		TLogContext_Item_Craft logContext(TheDataset.getEntityId(phrase->getActor()));
		CFaberActionCommon::createCraftedItem( phrase, 0, SKILLS::unknown, 0, phrase->getCraftedItemStaticForm()->SheetId, 0, 1, Params, 1.0f );
	}


	// Yoyo: Stretch the stats for more interesting items
	void	stretchItemStats(CCraftParameters &params, uint64 statBitField);
	
	void applyProcAddLimit(CCraftParameters& params, uint64 statBitField, double bonus);
};

namespace NCraftParametersHelpers // this prevents namespace pollution
{
	typedef CCraftParameters TStruct;
	typedef float TArray[RM_FABER_STAT_TYPE::NumRMStatType];
	static void convStructToArray(TArray& a, TStruct const& s)
	{
		a[RM_FABER_STAT_TYPE::Durability]							= s.Durability;
		a[RM_FABER_STAT_TYPE::Weight]								= s.Weight;
		a[RM_FABER_STAT_TYPE::SapLoad]								= s.SapLoad;
		a[RM_FABER_STAT_TYPE::DMG]									= s.Dmg;
		a[RM_FABER_STAT_TYPE::Speed]								= s.Speed;
		a[RM_FABER_STAT_TYPE::Range]								= s.Range;
		a[RM_FABER_STAT_TYPE::DodgeModifier]						= s.DodgeModifier;
		a[RM_FABER_STAT_TYPE::ParryModifier]						= s.ParryModifier;
		a[RM_FABER_STAT_TYPE::AdversaryDodgeModifier]				= s.AdversaryDodgeModifier;
		a[RM_FABER_STAT_TYPE::AdversaryParryModifier]				= s.AdversaryParryModifier;
		a[RM_FABER_STAT_TYPE::ProtectionFactor]						= s.ProtectionFactor;
		a[RM_FABER_STAT_TYPE::MaxSlashingProtection]				= s.MaxSlashingProtection;
		a[RM_FABER_STAT_TYPE::MaxBluntProtection]					= s.MaxBluntProtection;
		a[RM_FABER_STAT_TYPE::MaxPiercingProtection]				= s.MaxPiercingProtection;
		a[RM_FABER_STAT_TYPE::AcidProtection]						= s.AcidProtectionFactor;
		a[RM_FABER_STAT_TYPE::ColdProtection]						= s.ColdProtectionFactor;
		a[RM_FABER_STAT_TYPE::FireProtection]						= s.FireProtectionFactor;
		a[RM_FABER_STAT_TYPE::RotProtection]						= s.RotProtectionFactor;
		a[RM_FABER_STAT_TYPE::ShockWaveProtection]					= s.ShockWaveProtectionFactor;
		a[RM_FABER_STAT_TYPE::PoisonProtection]						= s.PoisonProtectionFactor;
		a[RM_FABER_STAT_TYPE::ElectricityProtection]				= s.ElectricityProtectionFactor;
		a[RM_FABER_STAT_TYPE::DesertResistance]						= s.DesertResistanceFactor;
		a[RM_FABER_STAT_TYPE::ForestResistance]						= s.ForestResistanceFactor;
		a[RM_FABER_STAT_TYPE::LacustreResistance]					= s.LacustreResistanceFactor;
		a[RM_FABER_STAT_TYPE::JungleResistance]						= s.JungleResistanceFactor;
		a[RM_FABER_STAT_TYPE::PrimaryRootResistance]				= s.PrimaryRootResistanceFactor;
		a[RM_FABER_STAT_TYPE::ElementalCastingTimeFactor]			= s.ElementalCastingTimeFactor;
		a[RM_FABER_STAT_TYPE::ElementalPowerFactor]					= s.ElementalPowerFactor;
		a[RM_FABER_STAT_TYPE::OffensiveAfflictionCastingTimeFactor]	= s.OffensiveAfflictionCastingTimeFactor;
		a[RM_FABER_STAT_TYPE::OffensiveAfflictionPowerFactor]		= s.OffensiveAfflictionPowerFactor;
		a[RM_FABER_STAT_TYPE::DefensiveAfflictionCastingTimeFactor]	= s.DefensiveAfflictionCastingTimeFactor;
		a[RM_FABER_STAT_TYPE::DefensiveAfflictionPowerFactor]		= s.DefensiveAfflictionPowerFactor;
		a[RM_FABER_STAT_TYPE::HealCastingTimeFactor]				= s.HealCastingTimeFactor;
		a[RM_FABER_STAT_TYPE::HealPowerFactor]						= s.HealPowerFactor;
	}
	static void convArrayToStruct(TStruct& s, TArray const& a)
	{
		s.Durability							= a[RM_FABER_STAT_TYPE::Durability];
		s.Weight								= a[RM_FABER_STAT_TYPE::Weight];
		s.SapLoad								= a[RM_FABER_STAT_TYPE::SapLoad];
		s.Dmg									= a[RM_FABER_STAT_TYPE::DMG];
		s.Speed									= a[RM_FABER_STAT_TYPE::Speed];
		s.Range									= a[RM_FABER_STAT_TYPE::Range];
		s.DodgeModifier							= a[RM_FABER_STAT_TYPE::DodgeModifier];
		s.ParryModifier							= a[RM_FABER_STAT_TYPE::ParryModifier];
		s.AdversaryDodgeModifier				= a[RM_FABER_STAT_TYPE::AdversaryDodgeModifier];
		s.AdversaryParryModifier				= a[RM_FABER_STAT_TYPE::AdversaryParryModifier];
		s.ProtectionFactor						= a[RM_FABER_STAT_TYPE::ProtectionFactor];
		s.MaxSlashingProtection					= a[RM_FABER_STAT_TYPE::MaxSlashingProtection];
		s.MaxBluntProtection					= a[RM_FABER_STAT_TYPE::MaxBluntProtection];
		s.MaxPiercingProtection					= a[RM_FABER_STAT_TYPE::MaxPiercingProtection];
		s.AcidProtectionFactor					= a[RM_FABER_STAT_TYPE::AcidProtection];
		s.ColdProtectionFactor					= a[RM_FABER_STAT_TYPE::ColdProtection];
		s.FireProtectionFactor					= a[RM_FABER_STAT_TYPE::FireProtection];
		s.RotProtectionFactor					= a[RM_FABER_STAT_TYPE::RotProtection];
		s.ShockWaveProtectionFactor				= a[RM_FABER_STAT_TYPE::ShockWaveProtection];
		s.PoisonProtectionFactor				= a[RM_FABER_STAT_TYPE::PoisonProtection];
		s.ElectricityProtectionFactor			= a[RM_FABER_STAT_TYPE::ElectricityProtection];
		s.DesertResistanceFactor				= a[RM_FABER_STAT_TYPE::DesertResistance];
		s.ForestResistanceFactor				= a[RM_FABER_STAT_TYPE::ForestResistance];
		s.LacustreResistanceFactor				= a[RM_FABER_STAT_TYPE::LacustreResistance];
		s.JungleResistanceFactor				= a[RM_FABER_STAT_TYPE::JungleResistance];
		s.PrimaryRootResistanceFactor			= a[RM_FABER_STAT_TYPE::PrimaryRootResistance];
		s.ElementalCastingTimeFactor			= a[RM_FABER_STAT_TYPE::ElementalCastingTimeFactor];
		s.ElementalPowerFactor					= a[RM_FABER_STAT_TYPE::ElementalPowerFactor];
		s.OffensiveAfflictionCastingTimeFactor	= a[RM_FABER_STAT_TYPE::OffensiveAfflictionCastingTimeFactor];
		s.OffensiveAfflictionPowerFactor		= a[RM_FABER_STAT_TYPE::OffensiveAfflictionPowerFactor];
		s.DefensiveAfflictionCastingTimeFactor	= a[RM_FABER_STAT_TYPE::DefensiveAfflictionCastingTimeFactor];
		s.DefensiveAfflictionPowerFactor		= a[RM_FABER_STAT_TYPE::DefensiveAfflictionPowerFactor];
		s.HealCastingTimeFactor					= a[RM_FABER_STAT_TYPE::HealCastingTimeFactor];
		s.HealPowerFactor						= a[RM_FABER_STAT_TYPE::HealPowerFactor];
	}
}

void CFaberActionMakeCompute::stretchItemStats(CCraftParameters& params, uint64 statBitField)
{
	// Transform to Float array
	float array[RM_FABER_STAT_TYPE::NumRMStatType];
	nlctassert(RM_FABER_STAT_TYPE::NumRMStatType==34);
	NCraftParametersHelpers::convStructToArray(array, params);
	
	// Apply same formula from EGS and client
	RM_FABER_STAT_TYPE::stretchItemStats(array, statBitField);
	
	// back to raw struct
	NCraftParametersHelpers::convArrayToStruct(params, array);
}

void CFaberActionMakeCompute::applyProcAddLimit(CCraftParameters& params, uint64 statBitField, double bonus)
{
	// Transform to Float array
	float array[RM_FABER_STAT_TYPE::NumRMStatType];
	nlctassert(RM_FABER_STAT_TYPE::NumRMStatType==34);
	NCraftParametersHelpers::convStructToArray(array, params);
	
	for (size_t i=0; i<RM_FABER_STAT_TYPE::NumRMStatType; ++i)
	{
		// if the item use this stat
		if(statBitField&((uint64)(1)<<i))
			array[i] *= 1.f + (float)bonus;
	}
	
	// back to raw struct
	NCraftParametersHelpers::convArrayToStruct(params, array);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Generic Item Generation
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


class CFaberActionMakeGeneric : public CFaberActionMakeCompute
{
public:
	// ctor
	CFaberActionMakeGeneric() {};
	
	// dtor
	virtual ~CFaberActionMakeGeneric() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, CCraftParameters& params, uint64 &statBF )
	{
		// the faber parameter of this MP in the current item part
		float mpStatEnergy = phrase->getMps()[ mpOccurence ]->Mp->StatEnergy / 100.f;
		CMP::TMpFaberParameters		&fp= phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ];
		RM_FABER_TYPE::TRMFType		ft= fp.MpFaberType;
		
		// **** Set The BitField of impacted stats
		nlctassert(RM_FABER_STAT_TYPE::NumRMStatType<=64);
		for(uint i=0;i<RM_FABER_STAT_TYPE::NumRMStatType;i++)
		{
			if(RM_FABER_STAT_TYPE::isStatRelevant(ft, (RM_FABER_STAT_TYPE::TRMStatType)i))
				statBF|= (uint64)(1)<<i;
		}

		// **** For each Stat, increment it if the ItemPart affect this Stat
		nlctassert(RM_FABER_STAT_TYPE::NumRMStatType==34);
		
		// Durability
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::Durability))
		{
			params.Durability += fp.Durability;
			params.nbDurability += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// Weight
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::Weight))
		{
			params.Weight += fp.Weight;
			params.nbWeight += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// SapLoad
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::SapLoad))
		{
			params.SapLoad += fp.SapLoad;
			params.nbSapLoad += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// DMG
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::DMG))
		{
			params.Dmg += fp.Dmg;
			params.nbDmg += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// Speed
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::Speed))
		{
			params.Speed += fp.Speed;
			params.nbSpeed += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// Range
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::Range))
		{
			params.Range += fp.Range;
			params.nbRange += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// DodgeModifier
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::DodgeModifier))
		{
			params.DodgeModifier += fp.DodgeModifier;
			params.nbDodgeModifier += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ParryModifier
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ParryModifier))
		{
			params.ParryModifier += fp.ParryModifier;
			params.nbParryModifier += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// AdversaryDodgeModifier
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::AdversaryDodgeModifier))
		{
			params.AdversaryDodgeModifier += fp.AdversaryDodgeModifier;
			params.nbAdversaryDodgeModifier += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// AdversaryParryModifier
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::AdversaryParryModifier))
		{
			params.AdversaryParryModifier += fp.AdversaryParryModifier;
			params.nbAdversaryParryModifier += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ProtectionFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ProtectionFactor))
		{
			params.ProtectionFactor += fp.ProtectionFactor;
			params.nbProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// MaxSlashingProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::MaxSlashingProtection))
		{
			params.MaxSlashingProtection += fp.MaxSlashingProtection;
			params.nbMaxSlashingProtection += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// MaxBluntProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::MaxBluntProtection))
		{
			params.MaxBluntProtection += fp.MaxBluntProtection;
			params.nbMaxBluntProtection += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// MaxPiercingProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::MaxPiercingProtection))
		{
			params.MaxPiercingProtection += fp.MaxPiercingProtection;
			params.nbMaxPiercingProtection += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// AcidProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::AcidProtection))
		{
			params.AcidProtectionFactor += fp.AcidProtection;
			params.nbAcidProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ColdProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ColdProtection))
		{
			params.ColdProtectionFactor += fp.ColdProtection;
			params.nbColdProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// FireProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::FireProtection))
		{
			params.FireProtectionFactor += fp.FireProtection;
			params.nbFireProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// RotProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::RotProtection))
		{
			params.RotProtectionFactor += fp.RotProtection;
			params.nbRotProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ShockWaveProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ShockWaveProtection))
		{
			params.ShockWaveProtectionFactor += fp.ShockWaveProtection;
			params.nbShockWaveProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// PoisonProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::PoisonProtection))
		{
			params.PoisonProtectionFactor += fp.PoisonProtection;
			params.nbPoisonProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ElectricityProtection
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ElectricityProtection))
		{
			params.ElectricityProtectionFactor += fp.ElectricityProtection;
			params.nbElectricityProtectionFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// DesertResistance
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::DesertResistance))
		{
			params.DesertResistanceFactor += fp.DesertResistance;
			params.nbDesertResistanceFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ForestResistance
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ForestResistance))
		{
			params.ForestResistanceFactor += fp.ForestResistance;
			params.nbForestResistanceFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// LacustreResistance
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::LacustreResistance))
		{
			params.LacustreResistanceFactor += fp.LacustreResistance;
			params.nbLacustreResistanceFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// JungleResistance
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::JungleResistance))
		{
			params.JungleResistanceFactor += fp.JungleResistance;
			params.nbJungleResistanceFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// PrimaryRootResistance
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::PrimaryRootResistance))
		{
			params.PrimaryRootResistanceFactor += fp.PrimaryRootResistance;
			params.nbPrimaryRootResistanceFactor += 1;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ElementalCastingTimeFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ElementalCastingTimeFactor))
		{
			params.ElementalCastingTimeFactor += fp.ElementalCastingTimeFactor;
			++params.nbElementalCastingTimeFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// ElementalPowerFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::ElementalPowerFactor))
		{
			params.ElementalPowerFactor += fp.ElementalPowerFactor;
			++params.nbElementalPowerFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// OffensiveAfflictionCastingTimeFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::OffensiveAfflictionCastingTimeFactor))
		{
			params.OffensiveAfflictionCastingTimeFactor += fp.OffensiveAfflictionCastingTimeFactor;
			++params.nbOffensiveAfflictionCastingTimeFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// OffensiveAfflictionPowerFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::OffensiveAfflictionPowerFactor))
		{
			params.OffensiveAfflictionPowerFactor += fp.OffensiveAfflictionPowerFactor;
			++params.nbOffensiveAfflictionPowerFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// DefensiveAfflictionCastingTimeFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::DefensiveAfflictionCastingTimeFactor))
		{
			params.DefensiveAfflictionCastingTimeFactor += fp.DefensiveAfflictionCastingTimeFactor;
			++params.nbDefensiveAfflictionCastingTimeFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// DefensiveAfflictionPowerFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::DefensiveAfflictionPowerFactor))
		{
			params.DefensiveAfflictionPowerFactor += fp.DefensiveAfflictionPowerFactor;
			++params.nbDefensiveAfflictionPowerFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// HealCastingTimeFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::HealCastingTimeFactor))
		{
			params.HealCastingTimeFactor += fp.HealCastingTimeFactor;
			++params.nbHealCastingTimeFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
		// HealPowerFactor
		if(RM_FABER_STAT_TYPE::isStatRelevant(ft, RM_FABER_STAT_TYPE::HealPowerFactor))
		{
			params.HealPowerFactor += fp.HealPowerFactor;
			++params.nbHealPowerFactor;
			params.StatEnergy += mpStatEnergy;
			params.nbStatEnergy += 1;
		}
			
		// Add always impact on HpBuff (should always be NULL in MPs for now)
		params.HpBuff += fp.HpBuff;
		params.SapBuff += fp.SapBuff;
		params.StaBuff += fp.StaBuff;
		params.FocusBuff += fp.FocusBuff;
	}
	
};
	



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Blade weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeBladeMeleeWeapon : public CFaberActionMakeGeneric
{
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

// Axe weapon specialized class
class CFaberActionMakeAxe : public CFaberActionMakeBladeMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMakeAxe, ITEM_TYPE::AXE )

// 2HAxe weapon specialized class
class CFaberActionMake2HAxe : public CFaberActionMakeBladeMeleeWeapon
{
};
FABER_ACTION_FACTORY( CFaberActionMake2HAxe, ITEM_TYPE::TWO_HAND_AXE )


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Hammer weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeHammerMeleeWeapon : public CFaberActionMakeGeneric
{
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


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//					Point weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakePointMeleeWeapon : public CFaberActionMakeGeneric
{
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
class CFaberActionMakeStaffMeleeWeapon : public CFaberActionMakeGeneric
{
};
FABER_ACTION_FACTORY( CFaberActionMakeStaffMeleeWeapon, ITEM_TYPE::STAFF )


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				Magician Staff weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeMagicianStaff: public CFaberActionMakeGeneric
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMagicianStaff, ITEM_TYPE::MAGICIAN_STAFF )


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Trigger range weapon specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeTriggerRangeWeapon : public CFaberActionMakeGeneric
{
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



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Ammo specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeAmmo : public CFaberActionMakeGeneric
{
protected:
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, CCraftParameters& params, uint64 &statBF )
	{
		// Generic
		CFaberActionMakeGeneric::specializedApply(phrase, mpOccurence, mpParameters, params, statBF);

		// Ensure no HpBuff etc...
		params.HpBuff = 0;
		params.SapBuff = 0;
		params.StaBuff = 0;
		params.FocusBuff = 0;
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




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Explosive Ammo specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeExplosiveAmmo : public CFaberActionMakeGeneric
{
protected:
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, CCraftParameters& params, uint64 &statBF )
	{
		// Generic
		CFaberActionMakeGeneric::specializedApply(phrase, mpOccurence, mpParameters, params, statBF);
		
		// Ensure no HpBuff etc...
		params.HpBuff = 0;
		params.SapBuff = 0;
		params.StaBuff = 0;
		params.FocusBuff = 0;
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



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Armor specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeArmor : public CFaberActionMakeGeneric
{
protected:
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, CCraftParameters& params, uint64 &statBF )
	{
		// Generic
		CFaberActionMakeGeneric::specializedApply(phrase, mpOccurence, mpParameters, params, statBF);
		
		// Choose color
		{
			if( phrase->getMps()[ mpOccurence ]->Mp->MpColor <= 7 )
			{
				params.Color[ phrase->getMps()[ mpOccurence ]->Mp->MpColor ] += 1;
			}
		}
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


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Jewel specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeJewelry : public CFaberActionMakeGeneric
{
protected:
	virtual void specializedApply( CFaberPhrase * phrase, uint32 mpOccurence, uint mpParameters, CCraftParameters& params, uint64 &statBF )
	{
		// Generic
		CFaberActionMakeGeneric::specializedApply(phrase, mpOccurence	, mpParameters, params, statBF);
		
		// Choose Color and Protection
		{
			// the faber parameter of this MP in the current item part
			CMP::TMpFaberParameters		&fp= phrase->getMps()[ mpOccurence ]->Mp->MpFaberParameters[ mpParameters ];
			RM_FABER_TYPE::TRMFType		ft= fp.MpFaberType;
			
			if( phrase->getMps()[ mpOccurence ]->Mp->MpColor <= 7 )
			{
				params.Color[ phrase->getMps()[ mpOccurence ]->Mp->MpColor ] += 1;
			}
		}
	}
};

// Anklet specialized class
class CFaberActionMakeAnklet : public CFaberActionMakeJewelry
{
};
FABER_ACTION_FACTORY( CFaberActionMakeAnklet, ITEM_TYPE::ANKLET )

// Bracelet specialized class
class CFaberActionMakeBracelet : public CFaberActionMakeJewelry
{
};
FABER_ACTION_FACTORY( CFaberActionMakeBracelet, ITEM_TYPE::BRACELET )

// Diadem specialized class
class CFaberActionMakeDiadem : public CFaberActionMakeJewelry
{
};
FABER_ACTION_FACTORY( CFaberActionMakeDiadem, ITEM_TYPE::DIADEM )

// Earing specialized class
class CFaberActionMakeEaring : public CFaberActionMakeJewelry
{
};
FABER_ACTION_FACTORY( CFaberActionMakeEaring, ITEM_TYPE::EARING )

// Pendant specialized class
class CFaberActionMakePendant : public CFaberActionMakeJewelry
{
};
FABER_ACTION_FACTORY( CFaberActionMakePendant, ITEM_TYPE::PENDANT )

// Ring specialized class
class CFaberActionMakeRing : public CFaberActionMakeJewelry
{
};
FABER_ACTION_FACTORY( CFaberActionMakeRing, ITEM_TYPE::RING )



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Tool specialized class
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class CFaberActionMakeTool : public CFaberActionMakeGeneric
{
};

// ammo tool specialized class
class CFaberActionMakeAmmoTool : public CFaberActionMakeTool
{
};
FABER_ACTION_FACTORY( CFaberActionMakeAmmoTool, ITEM_TYPE::AmmoTool )

// armor tool specialized class
class CFaberActionMakeArmorTool : public CFaberActionMakeTool
{
};
FABER_ACTION_FACTORY( CFaberActionMakeArmorTool, ITEM_TYPE::ArmorTool )

// jewelry tool specialized class
class CFaberActionMakeJewelryTool : public CFaberActionMakeTool
{
};
FABER_ACTION_FACTORY( CFaberActionMakeJewelryTool, ITEM_TYPE::JewelryTool )

// melee weapon tool specialized class
class CFaberActionMakeMeleeWeaponTool : public CFaberActionMakeTool
{
};
FABER_ACTION_FACTORY( CFaberActionMakeMeleeWeaponTool, ITEM_TYPE::MeleeWeaponTool )

// range weapon tool specialized class
class CFaberActionMakeRangeWeaponTool : public CFaberActionMakeTool
{
};
FABER_ACTION_FACTORY( CFaberActionMakeRangeWeaponTool, ITEM_TYPE::RangeWeaponTool )

// SHEARS = pick for forage
class CFaberActionMakeForageTool : public CFaberActionMakeTool
{
};
FABER_ACTION_FACTORY( CFaberActionMakeForageTool, ITEM_TYPE::SHEARS )

// tool maker specialized class
class CFaberActionMakeToolMaker : public CFaberActionMakeTool
{
};
FABER_ACTION_FACTORY( CFaberActionMakeToolMaker, ITEM_TYPE::ToolMaker )


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//				  Mission Item specialized class (episode2 trunk etc...)
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


// Make Mission Item specialized class. Directly derive from IFaberAction, since simplest one
class CFaberActionMakeMissionItem : public IFaberAction
{
public:
	CFaberActionMakeMissionItem() {};
	virtual ~CFaberActionMakeMissionItem() {};
	
protected:
	//////////////////////////////////////////
	// check sentence validity
	virtual bool checkSentenceValidity( CFaberPhrase * phrase )
	{
		return CFaberActionCommon::checkSentenceValidity( phrase );
	}
	
	//////////////////////////////////////////
	// Apply all params / compute result item
	virtual void apply(CFaberPhrase * phrase)
	{
		CCharacter* character = ( CCharacter * ) CEntityBaseManager::getEntityBasePtr( phrase->getActor() );
		if( character )
		{
			// compute success factor
			sint deltaLvl;
			float successFactor = CFaberActionCommon::getSuccessFactor( character, phrase, deltaLvl );
			
			// final item quality, depending of recommended skill of action used and lower quality of raw materials used and success factor of action
			uint16 finalItemQuality = max( (uint16)1, (uint16) (min(phrase->getLowerRmQuality(),(uint16)phrase->getRecommendedSkill()) * successFactor) );
			
			// partial, normal or critical success
			if( successFactor > 0.0f )
			{
				// Use an empty craft parameters
				CCraftParameters	Params;
				
				// Count number of MPs required (for possible XP gain)
				uint32 mpOccurence = 0;
				uint32 neededMp = (uint32)phrase->getRootFaberPlan()->Faber->NeededMps.size();
				for( uint mp = 0; mp < neededMp; ++mp )
				{
					mpOccurence+= phrase->getRootFaberPlan()->Faber->NeededMps[ mp ].Quantity;
				}
				
				// Compute the delta level for the XP based on the Base skill, not the buffed/unbuffed one.
				sint32 skillValue = character->getSkillBaseValue( phrase->getRootFaberPlan()->getSkill(0) );
				deltaLvl = skillValue - (sint32)min((sint32)phrase->getLowerRmQuality(),(sint32)phrase->getRecommendedSkill());
				// Craft the item and Get XP
				CFaberActionCommon::createCraftedItem( phrase, character, phrase->getRootFaberPlan()->getSkill(0), deltaLvl, phrase->getCraftedItemStaticForm()->SheetId, finalItemQuality, phrase->getRootFaberPlan()->Faber->NbItemsPerUnit, Params, successFactor * mpOccurence / NBMeanCraftRawMaterials );
				
				// Display stat
				CFaberActionCommon::displayStat(character, successFactor, phrase->getRootFaberPlan()->getSkill(0));
			}
			else
			{
				character->consumeFaberRms(true);
				character->sendCloseTempInventoryImpulsion();
			}
		}
	}
	
	// Apply params for system craft / compute result item (exemple selling item, not crafted by a player)
	virtual void systemApply(CFaberPhrase * phrase)
	{
		CCraftParameters	Params;
		CFaberActionCommon::createCraftedItem(phrase, 0, SKILLS::unknown, 0, phrase->getCraftedItemStaticForm()->SheetId, 0, 1, Params, 1.0f );
	}
	
};
// Work for all Undefined item type
FABER_ACTION_FACTORY( CFaberActionMakeMissionItem, ITEM_TYPE::UNDEFINED )


