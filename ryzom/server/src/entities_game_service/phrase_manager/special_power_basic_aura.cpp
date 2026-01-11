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
#include "game_share/visual_fx.h"
//
#include "special_power_basic_aura.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "special_power_phrase.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "aura_effect.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CSpecialPowerBasicAura::apply()
{
	if (!_Phrase)
		return;

	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerBasicAura::apply> Cannot find actor entity or not a player");
		return;
	}

	// disable auras
	if (!_ByPassTargetsDisableAuraTime)
	{
		actor->setForbidAuraUseDates(CTickEventHandler::getGameCycle(), CTickEventHandler::getGameCycle() + _DisablePowerTime);
	}

	const TGameCycle endDate = _Duration + CTickEventHandler::getGameCycle();

	// create effect and apply it on target
	CAuraRootEffect *effect = new CAuraRootEffect(_ActorRowId, endDate, _RootEffectFamily, _CreatedEffectFamily, _PowerType, _ParamValue, _AffectGuild);
	if (!effect)
	{		
		nlwarning("<CSpecialPowerBasicAura::apply> Failed to allocate new CShieldingEffect");
		return;
	}
	effect->setRadius(_AuraRadius);

	effect->setIsFromConsumable(_ByPassTargetsDisableAuraTime);

	effect->setTargetDisableTime(_TargetsDisableAuraTime);

	actor->addSabrinaEffect(effect);

	// add aura FX on the actor
	CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _ActorRowId, DSPropertyVISUAL_FX );
	CVisualFX fx;
	fx.unpack(visualFx.getValue());
	
	switch(_PowerType)
	{
	case POWERS::LifeAura:
		fx.Aura = MAGICFX::AuraHp;
		break;
	case POWERS::StaminaAura:
		fx.Aura = MAGICFX::AuraSta;
		break;
	case POWERS::SapAura:
		fx.Aura = MAGICFX::AuraSap;
		break;
	case POWERS::MeleeProtection:
		fx.Aura = MAGICFX::ProtectionMelee;
		break;
	case POWERS::Umbrella:
		fx.Aura = MAGICFX::ProtectionRange;
		break;
	case POWERS::AntiMagicShield:
		fx.Aura = MAGICFX::ProtectionMagic;
		break;
	case POWERS::WarCry:
		fx.Aura = MAGICFX::WarCry;
		break;
	case POWERS::FireWall:
		fx.Aura = MAGICFX::FireWall;
		break;
	case POWERS::ThornWall:
		fx.Aura = MAGICFX::ThornWall;
		break;
	case POWERS::WaterWall:
		fx.Aura = MAGICFX::WaterWall;
		break;
	case POWERS::LightningWall:
		fx.Aura = MAGICFX::LightningWall;
		break;
	default:
		return;
	};
	
	sint64 prop;
	fx.pack(prop);
	visualFx = (sint16)prop;
} // apply //
