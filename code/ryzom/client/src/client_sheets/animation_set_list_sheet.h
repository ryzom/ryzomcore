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



#ifndef CL_ANIMATION_SET_LIST_SHEET_H
#define CL_ANIMATION_SET_LIST_SHEET_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
#include "entity_sheet.h"
#include "animation_fx_set_sheet.h"
#include "game_share/people.h"

/////////////
// CLASSES //
/////////////


/**
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAnimationSheet
{

public:

	enum TAnimationId
	{
		UnknownAnim = 0xffffffff
	};

	enum { MaxNumFX = 4 };

public:

	/// Animation string Names (used when reading georges files)
	static NLMISC::CStaticStringMapper	AnimNames;
	/// Animation string Id
	NLMISC::TSStringId					IdAnim;

	/// Must apply the Character scale Pos factor to the "pos" channel" of this animation.
	bool					ApplyCharacterScalePosFactor: 1;
	/// Is the head controlable by the code.
	bool					HeadControlable				: 1;
	/// Do we have to play the animation from the strat or the end.
	bool					Reverse						: 1;
	/// Hide the entity at the end of the animation.
	bool					HideAtEndAnim				: 1;

	/// Useful for characters on a mount with rotation animation (because there is no rotation in the animation).
	double					VirtualRot;

	/// Name of the fx to launch with the animation (for backward compatibility, see _FXSet)
	static NLMISC::CStaticStringMapper	FxNames;
	/// Fx string Id
	NLMISC::TSStringId					IdFX;

	// FX set to launch with the animation
	CAnimationFXSetSheet				FXSet;

	std::vector<sint8>					Next;
	std::vector<uint16>					NextWeight;

	// Job/Race restriction (none by default)
	uint32								JobRestriction;
	EGSPD::CPeople::TPeople				RaceRestriction;

public:

	CAnimationSheet();

	/// Build the sheet from an external script.
	void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

};

/**
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAnimationStateSheet
{

public:

	enum TAnimStateSheetId
	{
		Idle			= 0,
		Run,
		Walk,
		TurnLeft,
		TurnRight,
		Emote,
		// Old Cast	// \todo GUIGUI : to remove when the new system will be used
		CastGoodBegin,
		CastGoodSuccess,
		CastGoodFail,
		CastGoodFumble,
		CastBadBegin,
		CastBadSuccess,
		CastBadFail,
		CastBadFumble,
		CastNeutralBegin,
		CastNeutralSuccess,
		CastNeutralFail,
		CastNeutralFumble,
		// CAST
		// Offensive Cast
		OffensiveCastInit,
		OffensiveCastBegin,
		OffensiveCastLoop,
		OffensiveCastFail,
		OffensiveCastFumble,
		OffensiveCastSuccess,
		OffensiveCastLink,
		// Curative Cast
		CurativeCastInit,
		CurativeCastBegin,
		CurativeCastLoop,
		CurativeCastFail,
		CurativeCastFumble,
		CurativeCastSuccess,
		CurativeCastLink,
		// Mixed Cast
		MixedCastInit,
		MixedCastBegin,
		MixedCastLoop,
		MixedCastFail,
		MixedCastFumble,
		MixedCastSuccess,
		MixedCastLink,
		// CAST INIT
		AcidCastInit,
		BlindCastInit,
		ColdCastInit,
		ElecCastInit,
		FearCastInit,
		FireCastInit,
		HealHPCastInit,
		MadCastInit,
		PoisonCastInit,
		RootCastInit,
		RotCastInit,
		ShockCastInit,
		SleepCastInit,
		SlowCastInit,
		StunCastInit,
		// CAST LOOP
		AcidCastLoop,
		BlindCastLoop,
		ColdCastLoop,
		ElecCastLoop,
		FearCastLoop,
		FireCastLoop,
		HealHPCastLoop,
		MadCastLoop,
		PoisonCastLoop,
		RootCastLoop,
		RotCastLoop,
		ShockCastLoop,
		SleepCastLoop,
		SlowCastLoop,
		StunCastLoop,
		// CAST FAIL
		AcidCastFail,
		BlindCastFail,
		ColdCastFail,
		ElecCastFail,
		FearCastFail,
		FireCastFail,
		HealHPCastFail,
		MadCastFail,
		PoisonCastFail,
		RootCastFail,
		RotCastFail,
		ShockCastFail,
		SleepCastFail,
		SlowCastFail,
		StunCastFail,
		// CAST END
		AcidCastEnd,
		BlindCastEnd,
		ColdCastEnd,
		ElecCastEnd,
		FearCastEnd,
		FireCastEnd,
		HealHPCastEnd,
		MadCastEnd,
		PoisonCastEnd,
		RootCastEnd,
		RotCastEnd,
		ShockCastEnd,
		SleepCastEnd,
		SlowCastEnd,
		StunCastEnd,
		// Attack
		DefaultAtkLow,
		DefaultAtkMiddle,
		DefaultAtkHigh,
		PowerfulAtkLow,
		PowerfulAtkMiddle,
		PowerfulAtkHigh,
		AreaAtkLow,
		AreaAtkMiddle,
		AreaAtkHigh,
		Attack1,
		Attack2,
		FirstPersonAttack,
		// OTHER
		Impact,
		Death,
		DeathIdle,
		LootInit,
		LootEnd,
		ProspectingInit,
		ProspectingEnd,
		CareInit,
		CareEnd,
		UseInit,
		UseBegin,
		UseLoop,
		UseEnd,
		StunBegin,
		StunLoop,
		StunEnd,
		SitMode,
		SitEnd,
		StrafeLeft,
		StrafeRight,
		StaticStateCount,
		UnknownState	= 0xffff
	};

protected:

	static const char *_AnimStateName[StaticStateCount];
	static std::map<std::string, TAnimStateSheetId>	_StringToAnimStateId;
	static std::vector<std::string>					_AnimStateIdToString;

public:

	/// Vector of animation.
	std::vector<CAnimationSheet> Animations;

	/// Animation State Id;
	uint16	State; // in fact it is a TAnimStateId

	// Are the objects in hand visible ?
	bool	DisplayObjects;

	/// The name of the animation for Lod Character.
	static NLMISC::CStaticStringMapper	LodCharAnim;
	/// The Id
	NLMISC::TSStringId					IdLodCharacterAnimation;

	/// MeleeImpactDelay (relevant only for melee attack states)
	float								MeleeImpactDelay;

public:

	// Transform a string in state id
	static TAnimStateSheetId getAnimationStateId (const std::string &stateName);

	// Transform a state id in string (for debug output)
	static const std::string &getAnimationStateName (TAnimStateSheetId id);

public:

	CAnimationStateSheet()
	{
		State = CAnimationStateSheet::UnknownState;
		DisplayObjects = true;
		IdLodCharacterAnimation = LodCharAnim.emptyId();
		MeleeImpactDelay= 0.5f;
	}

	/// Build the sheet from an external script.
	void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

};

/////////////
// For easiest use
typedef CAnimationStateSheet::TAnimStateSheetId TAnimStateKey;
typedef CAnimationStateSheet::TAnimStateSheetId TAnimStateId;


/**
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAnimationSetSheet
{
public:

	std::string							Name;
	std::vector<CAnimationStateSheet>	AnimationStates;

	bool	IsWalkEssential;
	bool	IsRunEssential;

public:

	/// Build the sheet from an external script.
	void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

};

/**
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAnimationSetListSheet : public CEntitySheet
{
public:

	std::vector<CAnimationSetSheet>	AnimSetList;

public:
	CAnimationSetListSheet();

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

};

#endif // CL_ANIMATION_SET_LIST_SHEET_H

/* End of animation_set_list_sheet.h */
