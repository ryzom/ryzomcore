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



#ifndef CL_ANIMATION_STATE_H
#define CL_ANIMATION_STATE_H

/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
// GameShare
#include "game_share/gender.h"
// Client sheets
#include "client_sheets/animation_set_list_sheet.h"
// Client
#include "animation.h"
// std
#include <map>


///////////
// CLASS //
///////////
namespace NLGEORGES
{
	class UFormElm;
}

/*
 *	Documentation
 *
 *  TAnimStateId : there is one TAnimStateId assciated with one string.
 */

/**
 * <Class description>
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CAnimationState
{
friend class CAnimationSet;

private:
	/// Vector of animation.
	std::vector<CAnimation> _Animations;

	/** Root Animations (index point to _Animations).
	 *	A Root animation is an animation that is not referenced in the "Graph of Next Anim"
	 */
	std::vector<uint>		_RootAnimations;

	// State Sheet
	CAnimationStateSheet *_Sheet;

	/// mark each animation of traversed state
	void recursMarkTraverseNext(sint idAnim, std::vector<bool> &traversedAnims, bool rootCall);

	// Build list of possible animations, according to job specialisation and race
	void buildAnimFilter(std::vector<uint> &filteredRootAnimList, std::vector<bool>	&animFilterStates, uint32 jobSpecialisation, EGSPD::CPeople::TPeople race, GSGENDER::EGender gender) const;

	// Choose a valid animation index in state.
	// (there are more chances for the first animation)
	// \warning This method does not check if _Animations is empty.
	uint chooseAnimationIndex(const std::vector<uint> &filteredRootAnimList) const;

	/** Backward compatibility. FallBack to old animations if EnableRacialAnimation==false
	 *	The fallbakc depends on the StateId (run, sit...) and on sex of player:
	 *		0 male
	 *		1 female
	 */
	static EGSPD::CPeople::TPeople		_FallBackToDefaultRace[2][CAnimationStateSheet::StaticStateCount];
	static bool							_FallBackToDefaultRaceInited;
	static void							initFallBackToDefaultRace();
	bool								isOldRaceAnimationForced(EGSPD::CPeople::TPeople race, GSGENDER::EGender gender) const;

public:
	/// Constructor
	CAnimationState();

	void init(CAnimationStateSheet *sheet, NL3D::UAnimationSet *animationSet);

	/// Return the ID for the State.
	TAnimStateId state() const
	{
		return (TAnimStateId)_Sheet->State;
	}

	/*
	 * Choose an animation in the list.
	 * \return TAnimId : Id of the animation.
	 * \param jobSpecialisation. 0 if none, else an index of a job specialisation of the character (eg: CraftRolemaster)
	 * \param race. unknown => all selected
	 * \param gender. Special Hack for EnableRacialAnimation==false....
	*/
	CAnimation::TAnimId chooseAnim(uint32 jobSpecialisation, EGSPD::CPeople::TPeople race, GSGENDER::EGender gender, double angToDest=0.0, CAnimation::TAnimId currentAnimIndex=CAnimation::UnknownAnim) const;
	/// Should the objects in hands be displayed ?
	bool areObjectsVisible() const {return _Sheet->DisplayObjects;}

	/// return the name of the LodCharacter animation. Only one by AnimationState.
	std::string getLodCharacterAnimation() const
	{
		return _Sheet->LodCharAnim.get(_Sheet->IdLodCharacterAnimation);
	}

	/// return mean MeleeImpactDelay (relevant only for melee attack states)
	float	getMeleeImpactDelay() const {return _Sheet->MeleeImpactDelay;}

	/// Return a pointer on the animation according to the animId.
	const CAnimation *getAnimation(CAnimation::TAnimId animId) const
	{
		if((uint)animId >= _Animations.size())
		{
			nlwarning("CAnimationState:getAnimation: animId '%d' is invalid, state animation count = %d", animId,_Animations.size());
			if( _Sheet )
			{
				nlinfo("CAnimationState:getAnimation: animation sheet count = %d, State = %d, display objects : %d", _Sheet->Animations.size(), _Sheet->State, _Sheet->DisplayObjects?1:0);
			}
#if !FINAL_VERSION
			nlstop;
#endif // !FINAL_VERSION
			return 0;
		}
		return &(_Animations[animId]);
	}

	/** \name DEBUG
	 * Methods only here for the debug.
	 */
	//@{
	/// Method only Used for debug
	void check(CAnimation::TAnimId animId) const
	{
		if(animId != CAnimation::UnknownAnim)
			nlassertex(((uint)animId<_Animations.size()), ("CAnimationState:check: animId '%d' is invalid.", animId));
	}
	//@}


	uint getNumAnimation() const { return (uint)_Animations.size(); }
	CAnimation *getAnimationByIndex(uint index);

	// Transform a string in state id
	static TAnimStateId getAnimationStateId (const std::string &stateName)
	{
		return CAnimationStateSheet::getAnimationStateId(stateName);
	}

	// Transform a state id in string (for debug output)
	static const std::string &getAnimationStateName (TAnimStateId id)
	{
		return CAnimationStateSheet::getAnimationStateName(id);
	}

//	static void memoryCompress()	{ _LodCharAnim.memoryCompress(); }
//	static void memoryRelease()		{ _LodCharAnim.clear(); }
};


#endif // CL_ANIMATION_STATE_H

/* End of animation_state.h */
