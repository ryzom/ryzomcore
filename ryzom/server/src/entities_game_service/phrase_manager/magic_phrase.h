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



#ifndef RY_MAGIC_PHRASE_H
#define RY_MAGIC_PHRASE_H

//
#include "server_share/target.h"
#include "game_share/magic_fx.h"
//
#include "s_phrase.h"
#include "spell_target.h"
#include "magic_action.h"
#include "phrase_manager/area_effect.h"
#include "magic_focus_item.h"


/**
 * This class represents a magic phrase in the Sabrina system. See CSPhrase for methods definitions.
 * A magic phrase contains the global parameters of the phrase (range, cost,...) And the magic actions
 * The real activity of the phrase is managed by these actions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CMagicPhrase : public CSPhrase
{
	// fordiden copy constructor
	CMagicPhrase(const CMagicPhrase &other)
	{
		nlassert(false);
	}
public:
	static void defaultCastingTime(float time) {CMagicPhrase::_DefaultCastingTime = time;}
	static float defaultCastingTime() {return CMagicPhrase::_DefaultCastingTime;}

	NL_INSTANCE_COUNTER_DECL(CMagicPhrase);
public:


	/// ctor
	inline CMagicPhrase() 
	: CSPhrase()
	, _SabrinaCost(0)
	, _SabrinaRelativeCost(1.f)
	, _SabrinaCredit(0)
	, _SabrinaRelativeCredit(1.f)
	, _SapCost(0)
	, _BrickMaxSabrinaCost(0)
	, _HPCost(0)
	, _RangeIndex(0)
	, _CastingTime(0)
	, _BaseCastingTime(0)
	, _Targets(1)
	, _Nature(ACTNATURE::UNKNOWN)
	, _Range(0)
	, _BreakResist(0)
	, _ArmorCompensation(0)
	, _MinCastTime(0)
	, _MaxCastTime(0xff)
	, _PostCastTime(0)
	, _CastingTimeCredit(0.f)
	, _MinRange(0), _MaxRange(0xff)
	, _RangeCredit(0.f)
	, _TargetRestriction(TARGET::EveryBody)
	, _CurrentFxPower(0)
	, _Area(NULL)
	, _EnchantPhrase(false)
	, _CreatureBehaviour(MBEHAV::UNKNOWN_BEHAVIOUR)
	, _CastBehaviour(MBEHAV::UNKNOWN_BEHAVIOUR)
	, _MagicFxType(MAGICFX::TMagicFx(0))
	, _Vampirise(0)
	, _VampiriseRatio(1)
	, _BehaviourWeight(0)
	, _MaxTargets(1)
	, _MultiTargetFactor(1.f)
	, _BreakNewLink(false)
	, _IsProc(false)
	, _DivineInterventionOccured(false)
	, _ShootAgainOccured(false)
	, _SpellLevel(0)
	{
		_PhraseType = BRICK_TYPE::MAGIC;
	}

	/// dtor
	virtual ~CMagicPhrase();

	inline void setEnchantMode(bool mode) { _EnchantPhrase = mode; }
	inline bool getEnchantMode() const { return _EnchantPhrase; }

	/// build the phrase from an ai action
	virtual bool initPhraseFromAiAction( const TDataSetRow & actorRowId, const CStaticAiAction *aiAction );

	/// \accessors
	//@{
	inline uint16	getSabrinaCost() const							{ return (uint16)(_SabrinaCost * _SabrinaRelativeCost);	}
	inline uint		getNbActions() const							{ return (uint)_Actions.size();	}
	inline sint32	getSapCost() const								{ return _SapCost;			}
	inline sint32	getHPCost() const								{ return _HPCost;			}
	inline ACTNATURE::TActionNature getNature()	const				{ return _Nature;			}

	inline const	std::vector<CSpellTarget> & getTargets() const	{ return _Targets;			}
	inline const	TDataSetRow & getActor() const					{ return _ActorRowId;		}
	inline sint32	getBreakResist() const							{ return _BreakResist;		}
	inline const	std::vector<SKILLS::ESkills>& getSkills() const	{ return _Skills;			}
	inline NLMISC::TGameCycle getCastingTime() const				{ return _DivineInterventionOccured||_ShootAgainOccured?_BaseCastingTime:_CastingTime;		}
	inline sint32	getSpellLevel() const							{ return _SpellLevel;		}
	//@}

	/**
	 * apply a brick parameter to this sentence
	 * \param param: parameter to take in account in this phrase
	 * \param brick the current brick 
	 * \param buildParams structure used to keep special params during build process
	 */
	void applyBrickParam( const TBrickParam::IId * param, const CStaticBrick &brick, CBuildParameters &buildParams );

	bool spendResources(CEntityBase* entity);

	///\name Overriden methods from CSPhrase
	//@{
	/// \warning The bricks vector MUST NOT be EMPTY.
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute = true );
	virtual bool evaluate();
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual bool launch();
	virtual void apply();
	virtual void stop();
	virtual void end();
	/// Change the primary target (if not a self only spell).
	virtual void setPrimaryTarget( const TDataSetRow &entityRowId );
	//@}

	inline uint16 currentFxPower() const { return _CurrentFxPower; }

	// set magic fx type
	inline void setMagicFxType(MAGICFX::TMagicFx type, uint16 fxPower) 
	{ 
		if (_CurrentFxPower <= fxPower)
		{
			_CurrentFxPower = fxPower;
			_MagicFxType = type; 
		}
	}

	///\return the area effect
	inline const CAreaEffect* getArea(){ return _Area;}

	virtual void setBrickSheets( const std::vector<NLMISC::CSheetId> & bricks)
	{
		_BrickSheets = bricks;
	}


	bool buildProc( const TDataSetRow & actorRowId, const std::vector<NLMISC::CSheetId>& brickIds );

	/// execute a proc item phrase. Does not use the phrase manager execution pipeline
	///\return true if the action could be executed
	/// it is the responsability of the caller to update the item.
	/// - getCastingTime() return the latency of the action
	/// - getSabrinaCost() can be used to compute the cost in item charge
	bool procItem();

	/// get used item stats
	inline const CMagicFocusItemFactor &getUsedItemStats() const { return _UsedItemStats; }

	/// get BrickMaxSabrinaCost
	inline uint16 getBrickMaxSabrinaCost() const { return _BrickMaxSabrinaCost; }

	/// get min spell power factor
	inline float getAreaSpellPowerFactor() const { return _MultiTargetFactor; }

	/// get spell max range
	inline uint32 getSpellRange() const { return _Range; }

	/// if true, new link created by this phrase will be broken
	inline bool breakNewLink() const { return _BreakNewLink; }

	/// if true, new link created by this phrase will be broken
	inline void breakNewLink(bool b) { _BreakNewLink = b; }

private:
	struct CActionApplyParams
	{
		sint					DeltaLevel;
		sint					SkillLevel;
		float					SuccessFactor;
		MBEHAV::CBehaviour		Behav;
		NLMISC::CBitSet			AffectedTargets;
		NLMISC::CBitSet			InvulnerabilityOffensive;
		NLMISC::CBitSet			InvulnerabilityAll;
		std::vector<float>		DistanceToTarget;
		std::vector<float>		TargetPowerFactor;
		bool					IsMad;
		NLMISC::CBitSet			Resists;
		TReportAction			ActionReport;
	};

private:
	static float	_DefaultCastingTime;

private:
	// active actions triggered by this phrase
	std::vector< IMagicAction* > _Actions;
	/// acting entity
	TDataSetRow					_ActorRowId;
	/// targets
	//std::vector<TDataSetRow>	_Targets;
	std::vector<CSpellTarget>	_Targets;

	// nature of the spell
	ACTNATURE::TActionNature	_Nature;
	
	// forced behaviour for creatures
	MBEHAV::EBehaviour			_CreatureBehaviour;
	// cast behaviour for players
	MBEHAV::EBehaviour			_CastBehaviour;
	// behaviour weight for players
	uint16						_BehaviourWeight;

	/// total cost (sabrina system)
	uint16						_SabrinaCost;
	/// total relative cost (sabrina system), must be added to total cost
	float						_SabrinaRelativeCost;
	/// brick max sabrina cost
	uint16						_BrickMaxSabrinaCost;
	/// total credit (sabrina system)
	uint16						_SabrinaCredit;
	/// total relative credit (sabrina system), must be added to total credit
	float						_SabrinaRelativeCredit;
	/// sap cost of the attack
	uint16						_SapCost;
	/// hp cost
	uint16						_HPCost;
	/// casting time in ticks
	NLMISC::TGameCycle			_CastingTime;
	/// casting time in ticks without time credits
	NLMISC::TGameCycle			_BaseCastingTime;
	/// post cast time in ticks
	NLMISC::TGameCycle			_PostCastTime;
	/// range index of the spell
	sint8						_RangeIndex;
	/// the skills used in this phrase
	std::vector<SKILLS::ESkills> _Skills;
	/// range in mm
	sint32						_Range;
	/// interrupt resistance bonus
	uint16						_BreakResist;
	/// number of skill malus points rhat are compensated
	uint16						_ArmorCompensation;
	/// 
	float						_CastingTimeCredit;
	/// Min casting time (1/10sec).
	uint8						_MinCastTime;
	/// Max casting time (1/10sec).
	uint8						_MaxCastTime;
	/// 
	float						_RangeCredit;
	/// Min Range (in meters).
	uint8						_MinRange;
	/// Max Range (in meters).
	uint8						_MaxRange;
	/// TargetType
	TARGET::TTargetRestriction	_TargetRestriction;
	/// magic main FX type
	MAGICFX::TMagicFx			_MagicFxType;
	/// current Fx "power"
	uint16						_CurrentFxPower;

	/// area effect description
	CAreaEffect *				_Area;

	/// flag indicating if the phrase is an enchant phrase
	bool						_EnchantPhrase;
	/// flag indicating if the phrase is the phrase is a 'proc' from an item
	bool						_IsProc;

	/// bricks composing the phrase
	std::vector<NLMISC::CSheetId> _BrickSheets;

	sint32						_Vampirise;
	float						_VampiriseRatio;

	/// stats of item in player right hand
	CMagicFocusItemFactor		_UsedItemStats;

	/// params needed by IMagicAction::apply()
	CActionApplyParams			_ApplyParams;

	/// area max affected targets
	uint8						_MaxTargets;
	///
	float						_MultiTargetFactor;

	/// if the phrase creates a new link, it will be broken
	bool						_BreakNewLink;

	bool						_DivineInterventionOccured;
	bool						_ShootAgainOccured;
	
	/// level of the spell, used to override caster's level when computing target resist value
	sint32						_SpellLevel;

private:

	/// Compute the range for current phrase.
	void computeRange(float rangeFactor, float wearMalus);
	/// Compute the casting time for current phrase.
	void computeCastingTime(float castingTimeFactor, float wearMalus);
	/// write target list mirror property
	void writeTargetList(bool isMad);
	/// enchant the phrase in a cristal
	void enchantPhrase(CCharacter * user,float successFactor);

	/// test targets invulnerabilities to magic
	void testTargetsInvulnerabilities( NLMISC::CBitSet &invulnerabilityOffensive, NLMISC::CBitSet &invulnerabilityAll);

	/// init used item stats if any
	void initUsedMagicFocusStats();
};

typedef NLMISC::CSmartPtr<CMagicPhrase> CMagicPhrasePtr;

#endif // RY_MAGIC_PHRASE_H

/* End of magic_phrase.h */
