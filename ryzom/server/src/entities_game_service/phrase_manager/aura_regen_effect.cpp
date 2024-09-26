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
// net
#include "nel/net/message.h"
//
#include "aura_regen_effect.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_utilities_functions.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;


//--------------------------------------------------------------
//		CRegenAuraEffect::update()
//--------------------------------------------------------------
bool CRegenAuraEffect::update(CTimerEvent * event, bool applyEffect)
{
	if (!TheDataset.isAccessible(_TargetRowId))
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

#ifdef NL_DEBUG
	nlassert(_AffectedScore != SCORES::unknown);
#endif
	
	CCharacter *player = PlayerManager.getChar(_TargetRowId);
	if (!player)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
		
	player->getScores()._PhysicalScores[_AffectedScore].RegenerateModifier += _RegenModifier;
	
	// now only one update is needed, but the end methos must reset the modifier !
	// must update this every ticks NO !!!!
	//_UpdateTimer.setRemaining(1, event);

	return false;
} // update //

//--------------------------------------------------------------
//		CAuraRootEffect::removed()
//--------------------------------------------------------------
void CRegenAuraEffect::removed()
{
	// reset modifier to it's old value
	CCharacter *player = PlayerManager.getChar(_TargetRowId);
	if (!player)
	{
		nlwarning("Cannot find target entity %s", _TargetRowId.toString().c_str());
		return;
	}
	
	player->getScores()._PhysicalScores[_AffectedScore].RegenerateModifier -= _RegenModifier;

	// call base class method for aura removal management
	CAuraBaseEffect::removed();
}


CAuraEffectTFactory<CRegenAuraEffect> *CLifeAuraEffectFactoryInstance = new CAuraEffectTFactory<CRegenAuraEffect>(POWERS::LifeAura);
CAuraEffectTFactory<CRegenAuraEffect> *CStaminaAuraEffectFactoryInstance = new CAuraEffectTFactory<CRegenAuraEffect>(POWERS::StaminaAura);
CAuraEffectTFactory<CRegenAuraEffect> *CSapAuraEffectFactoryInstance = new CAuraEffectTFactory<CRegenAuraEffect>(POWERS::SapAura);
