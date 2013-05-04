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



#ifndef RY_AREA_EFFECT_H
#define RY_AREA_EFFECT_H

#include "game_share/magic_fx.h"

#include "egs_sheets/egs_static_brick.h"

#include "range_selector.h"

class CStaticAiAction;
class CGameItemPtr;
	
/**
 * Area effect descriptor. The build function enables to build it from scripts
 * As it is a rather simple class, I used an union instead of a class hierarchy to avoid methods calls through the vftable
 * 
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CAreaEffect
{
	CAreaEffect();
	NL_INSTANCE_COUNTER_DECL(CAreaEffect);
public:

	CAreaEffect(const CAreaEffect &areaEffect);
	~CAreaEffect();

	struct SBomb
	{
		float Radius;
		float MinFactor;
		uint8 MaxTargets;
	};
	struct SSpray
	{
		float MinBase;
		float MaxBase;
		float Height;
		uint8 MaxTargets;
	};
	struct SChain
	{
		uint8 MaxTargets;
		float Range;
		float Factor;
	};
	union
	{
		SBomb  Bomb;
		SSpray Spray;
		SChain Chain;
	};

	static CAreaEffect * buildArea( const TBrickParam::IId * param )
	{
	
		CAreaEffect * area = NULL;
		switch (param->id() )
		{
			case TBrickParam::AREA_BOMB:
				area = CAreaEffect::buildBomb(param);
				break;
			case TBrickParam::AREA_SPRAY:
				area = CAreaEffect::buildSpray(param);
				break;
			case TBrickParam::AREA_CHAIN:
				area = CAreaEffect::buildChain(param);
				break;
			default:
				break;
		}
		return area;
	}

	static CAreaEffect * buildArea( CGameItemPtr &itemPtr );

	static CAreaEffect * buildArea( const CStaticAiAction *aiAction );

	MAGICFX::TSpellMode Type;

	/// convert to string for display
	std::string toString() const
	{
		std::string str;
		switch(Type)
		{
		case MAGICFX::Bomb:
			str = NLMISC::toString("Bomb, radius %f, Min factor %f", Bomb.Radius, Bomb.MinFactor);
			break;
		case MAGICFX::Chain:
			str = NLMISC::toString("Spray, max base %f, base %f height %f", Spray.MaxBase, Spray.MinBase, Spray.Height);
			break;
		case MAGICFX::Spray:
			str = NLMISC::toString("Chain, max target %d, factor %f range %f", Chain.MaxTargets, Chain.Factor, Chain.Range);
			break;
		default:
			str = "unkown area type";
		};

		return str;
	}

protected:

	static CAreaEffect * buildBomb( const TBrickParam::IId * param )
	{
		CAreaEffect * effect = new CAreaEffect;
		effect->Type = MAGICFX::Bomb;
		effect->Bomb.Radius = ((CSBrickParamAreaBomb *)param)->Radius;
		effect->Bomb.MinFactor = ((CSBrickParamAreaBomb *)param)->MinFactor;
		effect->Bomb.MaxTargets = ((CSBrickParamAreaBomb *)param)->MaxTarget;
		return effect;
	}
	
	static CAreaEffect * buildSpray( const TBrickParam::IId * param )
	{
		CAreaEffect * effect = new CAreaEffect;
		effect->Type = MAGICFX::Spray;
		effect->Spray.MinBase = ((CSBrickParamAreaSpray *)param)->Base;
		effect->Spray.Height = ((CSBrickParamAreaSpray *)param)->Height;
		effect->Spray.MaxTargets = ((CSBrickParamAreaSpray *)param)->MaxTarget;
		if ( ((CSBrickParamAreaSpray *)param)->Angle >= 180 )
		{
			nlwarning("<CAreaEffect build>: spray angle is %u (0 < angle < 180 )",((CSBrickParamAreaSpray *)param)->Angle );
			delete effect;
			return NULL;
		}
		// compute the spray demi-angle
		float angle = float( NLMISC::Pi * ((CSBrickParamAreaSpray *)param)->Angle / 360.0 );
		// get the max base from the angle
		effect->Spray.MaxBase = float( 2 * tan( angle ) * effect->Spray.Height );
		return effect;
	}
	
	static CAreaEffect * buildChain( const TBrickParam::IId * param )
	{
		CAreaEffect * effect = new CAreaEffect;
		effect->Type = MAGICFX::Chain;
		effect->Chain.MaxTargets = ((CSBrickParamAreaChain *)param)->MaxTargets;
		effect->Chain.Range = ((CSBrickParamAreaChain *)param)->Range;
		effect->Chain.Factor = ((CSBrickParamAreaChain *)param)->Factor;
		return effect;
	}


};


/**
 * Class used to select entities in the specified area
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CEntityRangeSelector : public CRangeSelector
{
	NL_INSTANCE_COUNTER_DECL(CEntityRangeSelector);
public:

	CEntityRangeSelector() : _PrevFactor(1.0f), _Area(NULL)
	{
	}

	void buildTargetList(const TDataSetRow & actorRow, const TDataSetRow & mainTargetRow, const CAreaEffect * area, ACTNATURE::TActionNature nature, bool ignoreMainTarget = false);

	void clampTargetCount(uint32 maxTargetCount )
	{
		if ( _Area && _Entities.size() > maxTargetCount )
		{
			_Entities.resize( maxTargetCount );
			_Distances.resize( maxTargetCount );

		}
	}

	float getFactor(uint entityIdx) const;

private:
	// NB : this object don't own this pointed object
	const CAreaEffect *	 _Area;
	mutable float		 _PrevFactor;
};


/**
* Class used to select entities in a disc centered on given entity (used by auras)
* \author David Fleury
* \author Nevrax France
* \date 2003
*/
class CAuraEntitySelector : public CRangeSelector
{
public:
	inline void buildTargetList(CEntityBase * actor, sint32 x, sint32 y, float radius, bool offensiveAction, bool ignoreMainTarget)
	{
		buildDisc( actor, x, y, radius, EntityMatrix, offensiveAction, ignoreMainTarget);
	}
};

#endif // RY_AREA_EFFECT_H

/* End of area_effect.h */





















