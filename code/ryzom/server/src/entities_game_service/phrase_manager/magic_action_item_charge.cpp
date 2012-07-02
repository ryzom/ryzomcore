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
#include "magic_action.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_manager.h"

#include "phrase_manager/magic_phrase.h"

using namespace NLNET;
using namespace NLMISC;
using namespace std;


class CMagicActionItemCharge : public IMagicAction
{
public:
	CMagicActionItemCharge()
		:_SapLoad(0){}

protected:
	/// add brick
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd, CBuildParameters &buildParams )
	{
		for ( uint i=0 ; i< brick.Params.size() ; ++i)
		{
			const TBrickParam::IId* param = brick.Params[i];

			switch(param->id())
			{
			case TBrickParam::MA_END:
				INFOLOG("MA_END Found: end of effect");
				effectEnd = true;
				return true;
			case TBrickParam::MA_RECHARGE:
				_SapLoad = ((CSBrickParamMagicRecharge *)param)->SapLoad;
				INFOLOG("MA_RECHARGE: %u",_SapLoad);
				break;					
			default:
				// unused param, can be useful in the phrase
				phrase->applyBrickParam( param, brick, buildParams );
				break;
			}
		}
		return true;
	}
	virtual bool validate(CMagicPhrase * phrase, std::string &errorCode)
	{
		CCharacter * user = PlayerManager.getChar(phrase->getActor());
		if ( !user )
			return false;
		if ( user->forageProgress() ) // can't be extracting into temp inventory
			return false; // for now, gives the message "target missing or invalid"
		uint moneyCost = uint( phrase->getSabrinaCost() * RechargeMoneyFactor );
		if ( ! moneyCost )
			moneyCost = 1;
		if ( user->getMoney() < moneyCost )
		{
			errorCode = "EGS_MAGIC_LACK_MONEY";
			return false;
		}
		return true;
	}

	virtual void launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						 const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						 const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport )
	{
	}

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp )
	{
		H_AUTO(CMagicActionItemCharge_apply);

		CCharacter * user = PlayerManager.getChar(phrase->getActor());
		if ( !user )
			return;

		uint moneyCost = uint( phrase->getSabrinaCost() * RechargeMoneyFactor );
		if ( ! moneyCost )
			moneyCost = 1;
		if ( user->getMoney() < moneyCost )
			return;
		user->spendMoney( moneyCost );
		if(successFactor <= 0.0f)
			return;

		user->createRechargeItem( _SapLoad );
	}

	uint32 _SapLoad;
};

BEGIN_MAGIC_ACTION_FACTORY(CMagicActionItemCharge)
	ADD_MAGIC_ACTION_TYPE( "reload" )	
END_MAGIC_ACTION_FACTORY(CMagicActionItemCharge)

