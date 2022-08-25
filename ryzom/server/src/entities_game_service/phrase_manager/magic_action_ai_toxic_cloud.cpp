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
#include "magic_action_ai_toxic_cloud.h"
#include "phrase_manager/magic_phrase.h"
#include "phrase_manager/toxic_cloud.h"
#include "creature_manager/creature_manager.h"
#include "egs_sheets/egs_static_ai_action.h"


using namespace NLMISC;
using namespace std;

extern CCreatureManager CreatureManager;

//--------------------------------------------------------------
//					initFromAiAction
//--------------------------------------------------------------
bool CMagicAiActionToxicCloud::initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase )
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	
	if (aiAction->getType() != AI_ACTION::ToxicCloud )
		return false;
	
	// read parameters
	const COTSpellParams &data = aiAction->getData().OTSpell;

	CCreature *creature = CreatureManager.getCreature( phrase->getActor() );
	if (creature && creature->getForm())
		_Damage = (uint16)(data.SpellParamValue + data.SpellPowerFactor * creature->getForm()->getAttackLevel());
	else
		_Damage = (uint16)data.SpellParamValue;
	
	_EffectDuration = data.Duration;
	_UpdateFrequency = data.UpdateFrequency;
	_AffectedScore = data.AffectedScore;
	_DamageType = data.DamageType;
	
	// get effect radius
	const TAiArea &areaData = aiAction->getAreaData();
	_Radius = areaData.AreaRange;

	return true;
} // initFromAiAction //

//--------------------------------------------------------------
//					launch
//--------------------------------------------------------------
void CMagicAiActionToxicCloud::launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
									   const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
									   const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
{
} // launch //

//--------------------------------------------------------------
//					apply  
//--------------------------------------------------------------
void CMagicAiActionToxicCloud::apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
									  const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
									  const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
									  sint32 vamp, float vampRatio, bool reportXp )
{
	H_AUTO(CMagicAiActionToxicCloud_apply);

	if (!phrase || !_EffectDuration)
		return;
	
	// get acting entity position
	const TDataSetRow actorRowId = phrase->getActor();
	CEntityBase *actor = CEntityBaseManager::getEntityBasePtr(actorRowId);
	if (!actor)
		return;

	CVector pos;
	pos.x = actor->getX() / 1000.0f;
	pos.y = actor->getY() / 1000.0f;
	pos.z = actor->getZ() / 1000.0f;

	CToxicCloud *cloud = new CToxicCloud();
	if (!cloud)
	{
		nlwarning("MAGIC: failed to allocate new CToxicCloud object, memory full?");
		return;
	}

	cloud->init(pos, _Radius, _Damage, _UpdateFrequency, _EffectDuration);

	// fx radius
	uint8 fxRadius; // {0,1,2} for range (1,3,5 m)
	if (_Radius <= 1.5f)
	{
		fxRadius = 0;
	}
	else if (_Radius <= 4)
	{
		fxRadius = 1;
	}
	else
	{
		fxRadius = 2;
	}

	// spawn toxic cloud and add it to manager
	CSheetId sheet( toString( "toxic_cloud_%d.fx", fxRadius ));
	if ( cloud->spawn( sheet ) )
	{
		CEnvironmentalEffectManager::getInstance()->addEntity( cloud );
	}
	else
	{
		nlwarning( "MAGIC: Unable to spawn toxic cloud sheet %s", sheet.toString().c_str() );
		delete cloud;
	}
} // apply //

CMagicAiActionTFactory<CMagicAiActionToxicCloud> *CMagicActionToxicCloudAiFactoryInstance = new CMagicAiActionTFactory<CMagicAiActionToxicCloud>(AI_ACTION::ToxicCloud);
