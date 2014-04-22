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

/////////////
// INCLUDE //
/////////////

#include "animation_state.h"
#include "debug_client.h"
#include "client_cfg.h"
// misc
#include "nel/misc/debug.h"
// Georges
#include "nel/georges/u_form_elm.h"

///////////
// USING //
///////////

using namespace NLGEORGES;
using namespace std;

////////////
// METHOD //
////////////

//-----------------------------------------------
//-----------------------------------------------
EGSPD::CPeople::TPeople		CAnimationState::_FallBackToDefaultRace[2][CAnimationStateSheet::StaticStateCount];
bool						CAnimationState::_FallBackToDefaultRaceInited= false;

//-----------------------------------------------
//-----------------------------------------------
// Yoyo: ugly: because of players, we have to disable racial animation if they don't want them
void	CAnimationState::initFallBackToDefaultRace()
{
	_FallBackToDefaultRaceInited= true;

	// Default
	for(uint i=0;i<CAnimationStateSheet::StaticStateCount;i++)
	{
		_FallBackToDefaultRace[0][i]= EGSPD::CPeople::Matis;
		_FallBackToDefaultRace[1][i]= EGSPD::CPeople::Matis;
	}

	// Special for Sit mode. Use fyros for female, and tryker for male
	_FallBackToDefaultRace[0][CAnimationStateSheet::SitMode]= EGSPD::CPeople::Tryker;
	_FallBackToDefaultRace[1][CAnimationStateSheet::SitMode]= EGSPD::CPeople::Fyros;
	_FallBackToDefaultRace[0][CAnimationStateSheet::SitEnd]= EGSPD::CPeople::Tryker;
	_FallBackToDefaultRace[1][CAnimationStateSheet::SitEnd]= EGSPD::CPeople::Fyros;
	_FallBackToDefaultRace[0][CAnimationStateSheet::Idle]= EGSPD::CPeople::Tryker;
	_FallBackToDefaultRace[1][CAnimationStateSheet::Idle]= EGSPD::CPeople::Fyros;
}

//-----------------------------------------------
//-----------------------------------------------
// Yoyo: ugly: because of players, we have to disable some racial animation since they don't want them AT ALL
bool	CAnimationState::isOldRaceAnimationForced(EGSPD::CPeople::TPeople race, GSGENDER::EGender gender) const
{
	TAnimStateId animState= state();
	// consider Idle as sit.... This is false if the animation MODE is not SIT, but don't care,
	// since idle is never a run/walk state....
	bool	isSit= animState==CAnimationStateSheet::SitMode ||
		animState==CAnimationStateSheet::SitEnd ||
		animState==CAnimationStateSheet::Idle;
	// disable Tryker male run/walk
	if(race==EGSPD::CPeople::Tryker && gender==GSGENDER::male && !isSit)
		return true;
	// disable Fyros male Sit
	if(race==EGSPD::CPeople::Fyros && gender==GSGENDER::male && isSit)
		return true;
	// disable Matis (male/female) Sit
	if(race==EGSPD::CPeople::Matis && isSit)
		return true;

	// otherwise, allow every other racial animation
	return false;
}


//-----------------------------------------------
// CAnimationState :
// Constructor.
//-----------------------------------------------
CAnimationState::CAnimationState()
{
	_Sheet = NULL;
}// CAnimationState //


//-----------------------------------------------
// recursMarkTraverseNext
//-----------------------------------------------
void CAnimationState::recursMarkTraverseNext(sint idAnim, std::vector<bool> &traversedAnims, bool rootCall)
{
	if(idAnim<0 || idAnim>=(sint)_Animations.size())
		return;

	// if this animation has already been traversed, then don't need to recurse (already done)
	if(traversedAnims[idAnim])
		return;

	// mark this anim as traversed (if not from rootcall)
	if(!rootCall)
		traversedAnims[idAnim]= true;

	// mark, and traverse recurs
	CAnimation &anim= _Animations[idAnim];
	const std::vector<sint8>	&nextAnim= anim.getNextAnimList();
	for(uint i=0;i<nextAnim.size();i++)
		recursMarkTraverseNext(nextAnim[i], traversedAnims, false);
}

//-----------------------------------------------
// init
//-----------------------------------------------
void CAnimationState::init(CAnimationStateSheet *sheet, NL3D::UAnimationSet *animationSet)
{
	uint32 i;

	_Sheet = sheet;

	// **** init if needed _FallBackToDefaultRace
	if(!_FallBackToDefaultRaceInited)
		initFallBackToDefaultRace();

	// **** build animation list
	_Animations.resize(sheet->Animations.size());
	for (i = 0; i < _Animations.size(); ++i)
	{
		_Animations[i].init(&sheet->Animations[i], animationSet);
	}

	// **** build list of root animation
	// flag each animation to know if it is reachable through "NextAnim" system
	std::vector<bool> traversedAnims;
	traversedAnims.clear();
	traversedAnims.resize(_Animations.size(), false);
	for (i = 0; i < _Animations.size(); ++i)
	{
		recursMarkTraverseNext(i, traversedAnims, true);
	}
	// Build the final list of root animaions
	_RootAnimations.clear();
	for (i = 0; i < _Animations.size(); ++i)
	{
		// root animations are not reachable. NB: also consider always the 0th animation as a root one
		if(i==0 || !traversedAnims[i])
			_RootAnimations.push_back(i);
	}

}// init//

//-----------------------------------------------
// buildAnimFilter
//-----------------------------------------------
void CAnimationState::buildAnimFilter(vector<uint> &filteredRootAnimList, vector<bool>	&animFilterStates, uint32 jobSpecialisation, EGSPD::CPeople::TPeople race, GSGENDER::EGender gender) const
{
	uint i;

	// If the user doesn't want Racial Animation, force player race according to old animations
	// Plus force old race animation for some anim/race/gender case
	if(!ClientCfg.EnableRacialAnimation || isOldRaceAnimationForced(race, gender) )
	{
		// avoid problem with Gender= Neutral (beast).
		uint	uGender= gender;
		uint	uState= state();
		NLMISC::clamp(uGender, 0U, 1U);
		NLMISC::clamp(uState, 0U, uint(CAnimationStateSheet::StaticStateCount-1));
		// According to the state, and the sex of user, the choice may differ
		race= _FallBackToDefaultRace[uGender][uState];
	}

	// Mark each animation if ok or not
	animFilterStates.resize(_Animations.size());
	for(i=0;i<_Animations.size();i++)
	{
		animFilterStates[i]= _Animations[i].filterOk(jobSpecialisation, race);
	}

	// build list of filtered root animation
	filteredRootAnimList.clear();
	for(i=0;i<_RootAnimations.size();i++)
	{
		uint	idAnim= _RootAnimations[i];
		// if this root animation is filtered, add it
		if(idAnim<_Animations.size() && animFilterStates[idAnim])
			filteredRootAnimList.push_back(idAnim);
	}

}// buildAnimFilter //

//-----------------------------------------------
// chooseAnimationIndex :
// Choose a valid animation index in state.
// (there are more chances for the first animation)
// \warning This method does not check if _Animations is empty.
//-----------------------------------------------
uint CAnimationState::chooseAnimationIndex(const vector<uint> &filteredRootAnimList) const
{
	// error, none match filter, fallback to the first one
	if(filteredRootAnimList.empty())
		return 0;

	// 1 chance by 2 to choose the first animation
	if(filteredRootAnimList.size()==1 || (rand()%2))
		return filteredRootAnimList[0];
	else
		return filteredRootAnimList[rand()%filteredRootAnimList.size()];
}// chooseAnimationIndex //

//-----------------------------------------------
// chooseAnim :
// Choose an animation in the list.
// \return TAnimId : Id of the animation.
//-----------------------------------------------
CAnimation::TAnimId CAnimationState::chooseAnim(uint32 jobSpecialisation, EGSPD::CPeople::TPeople race, GSGENDER::EGender gender, double angToDest, CAnimation::TAnimId currentAnimIndex) const
{
	// If there is not animation return CAnimation::UnknownAnim.
	if(_Animations.empty())
		return CAnimation::UnknownAnim;

	// The animation to choose is not a rotation, no need to choose according to an angle.
	if(angToDest == 0.0)
	{
		// static to avoid reallocation each time
		static	vector<uint>	filteredRootAnimList;
		static	vector<bool>	animFilterStates;
		buildAnimFilter(filteredRootAnimList, animFilterStates, jobSpecialisation, race, gender);

		// Do not check if there is a sequence
		if(currentAnimIndex == CAnimation::UnknownAnim)
			return (CAnimation::TAnimId) chooseAnimationIndex(filteredRootAnimList);
		// Check the current animation index given is valid.
		if((uint)currentAnimIndex >= _Animations.size())
		{
			nlwarning("CAnimationState:chooseAnim: currentAnimIndex(%d) >= size", currentAnimIndex);
			return (CAnimation::TAnimId) chooseAnimationIndex(filteredRootAnimList);
		}
		// Get the next animation to play from the animation given.
		sint8 nextAnim = _Animations[(uint)currentAnimIndex].getNextAnim(animFilterStates);
		// Check the is a next animation defined
		if(nextAnim >= 0)
		{
			// Check the next animation is valid
			if((uint)nextAnim<_Animations.size())
				return (CAnimation::TAnimId) nextAnim;
			else
			{
				nlwarning("CAnimationState:chooseAnim: next animation index(%d) is invalid", nextAnim);
				return (CAnimation::TAnimId) chooseAnimationIndex(filteredRootAnimList);
			}
		}
		else
			return (CAnimation::TAnimId) chooseAnimationIndex(filteredRootAnimList);
	}
	// This is a rotation.
	else
	{
		uint i;
		uint best = 0;
		const uint count = (uint)_Animations.size ();
		double bestAng = 1000.0;	// Big value to be > to the first element.
		for (i=0; i<count; i++)
		{
			const CAnimation &anim = _Animations[i];
			double angTmp;
			if(anim.virtualRot() != 0.0)
				angTmp = fabs(fabs(angToDest)-anim.virtualRot());
			else
				angTmp = fabs(fabs(angToDest)-anim.getRot());
			if(angTmp < bestAng)
			{
				bestAng = angTmp;
				best = i;
			}
		}

		// Return the id for the closest animation for this angle.
		return (CAnimation::TAnimId)best;
	}
}// chooseAnim //

//-----------------------------------------------
// getAnimationByIndex
//-----------------------------------------------
CAnimation *CAnimationState::getAnimationByIndex(uint index)
{
	if (index >= _Animations.size()) return NULL;
	return &_Animations[index];
}// getAnimationByIndex
