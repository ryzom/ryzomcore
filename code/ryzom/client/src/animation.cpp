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
// misc
#include "nel/misc/debug.h"
// Georges
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_loader.h"
// client
#include "animation.h"
#include "animation_misc.h"
#include "debug_client.h"
#include "client_cfg.h"
#include "entity_animation_manager.h"	// \todo GUIGUI : added to recompile animations at loading (to do better ?).


///////////
// USING //
///////////

using namespace NLGEORGES;
using namespace NLSOUND;
using namespace std;

////////////
// METHOD //
////////////
// STATIC
std::set<std::string> CAnimation::MissingAnim;

//-----------------------------------------------
// CAnimation :
// Constructor.
//-----------------------------------------------
CAnimation::CAnimation()
{
	_SoundAnim	= -1;
	_Animation	= UnknownAnim;
	_Rot		= 0.0;
	_Sheet		= NULL;

}// CAnimation //

//-----------------------------------------------
// Destructor
//-----------------------------------------------
CAnimation::~CAnimation()
{
}// ~CAnimation //


//-----------------------------------------------
// init
//-----------------------------------------------
void CAnimation::init(CAnimationSheet *sheet, NL3D::UAnimationSet *animationSet)
{
	_Sheet = sheet;

	_FXSet.init(&sheet->FXSet, animationSet);


	string animName = _Sheet->AnimNames.get(_Sheet->IdAnim);
	computeAnimation(animationSet, animName);
}// init //

//-----------------------------------------------
// computeAnimation :
//-----------------------------------------------
void CAnimation::computeAnimation(NL3D::UAnimationSet *animationSet, const std::string &animName)
{
	// Check animationSet.
	if(animationSet == 0)
	{
		nlwarning("CAnimation:computeAnimation: Animation Set is NULL.");
		return;
	}
	// ...
	uint id = animationSet->getAnimationIdByName(animName);
	if(id == NL3D::UAnimationSet::NotFound)
	{
		_Animation = animationSet->addAnimation(animName.c_str(), animName.c_str(), ClientCfg.DisplayMissingAnimFile);
		if(_Animation == NL3D::UAnimationSet::NotFound)
		{
			// Up to 100 missing animation kept.
			if(MissingAnim.size() < 100)
			{
				std::pair<std::set<std::string>::iterator, bool> result = MissingAnim.insert(animName);
				if(result.second)
					nlwarning("CAnimation::computeAnimation: Animation '%s' NOT FOUND.", animName.c_str());
			}
		}
		else
		{
			if(CAnimationMisc::getAnimationLength(animationSet, _Animation) == 0.0)
				nlwarning("CAnimation::computeAnimation: Animation '%s' LENGTH IS 0.", animName.c_str());
			// Must store name for reload (or if the sound is not currently enabled)
			_SoundAnimName= animName;
			// Sound Process.
			CSoundAnimManager *sndMngr = CSoundAnimManager::instance();
			if(sndMngr)
			{
				std::string animNameNotConst = animName;
				_SoundAnim = sndMngr->loadAnimation(animNameNotConst);
			}
		}
	}
	else
	{
		_Animation = id;
		// Must store name for reload (or if the sound is not currently enabled)
		_SoundAnimName= animName;
		// Sound Process.
		CSoundAnimManager *sndMngr = CSoundAnimManager::instance();
		if(sndMngr)
		{
			std::string animNameNotConst = animName;
			_SoundAnim = sndMngr->getAnimationFromName(animNameNotConst);
		}
	}
	// Get the rotation in the animation.
	_Rot = CAnimationMisc::getAnimationRotation(animationSet, _Animation);
}// computeAnimation //

//-----------------------------------------------
// Return the next animation to play or -1
//-----------------------------------------------
sint8 CAnimation::getNextAnim(const vector<bool>	&animFilterStates) const
{
	if(_Sheet)
	{
		if(_Sheet->Next.empty())
			return -1;
		else
		{
			static	vector<sint8>	filteredNextAnim;
			static	vector<uint16>	filteredNextWeight;
			filteredNextAnim.clear();
			filteredNextWeight.clear();

			// compute the filtered weight sum, and filtered next anim list
			uint32 weightSum = 0;
			uint i;
			for( i=0; i<_Sheet->NextWeight.size(); ++i)
			{
				sint8	nextAnim= _Sheet->Next[i];
				if(nextAnim<(sint)animFilterStates.size() && animFilterStates[nextAnim])
				{
					uint16	nw= _Sheet->NextWeight[i];
					weightSum += nw;
					filteredNextAnim.push_back(nextAnim);
					filteredNextWeight.push_back(nw);
				}
			}

			// if no filter match, abort
			if(filteredNextAnim.empty())
				return -1;

			// choose a random number between 1 and weightsum
			uint32 randWeight = rand()%weightSum + 1;

			// "concatenate" weights of each index, when the random value is reached we'll have the index to use
			uint32 w = 0;
			for( i=0; i<filteredNextWeight.size(); ++i)
			{
				w += filteredNextWeight[i];
				if( randWeight <= w )
				{
					return filteredNextAnim[i];
				}
			}
			nlwarning("<CAnimation::getNextAnim> can't choose next anim, rand=%d weigthSum=%d weightCount=%d",randWeight,weightSum,_Sheet->NextWeight.size());
		}
	}

	return -1;
}// getNextAnim //


//-----------------------------------------------
// resetSoundAnim
//-----------------------------------------------
void	CAnimation::resetSoundAnim()
{
	_SoundAnim=CSoundAnimationNoId;
}

//-----------------------------------------------
// reloadAnimation() with stored name
//-----------------------------------------------
void	CAnimation::reloadSoundAnim()
{
	CSoundAnimManager *sndMngr = CSoundAnimManager::instance();
	// don't need to reload if already loaded
	if(sndMngr && _SoundAnim==CSoundAnimationNoId)
	{
		// If some sound anim bound
		if(!_SoundAnimName.empty())
		{
			std::string animNameNotConst = _SoundAnimName;
			// if already exist, take
			_SoundAnim = sndMngr->getAnimationFromName(animNameNotConst);
			// else create
			if(_SoundAnim==CSoundAnimationNoId)
				_SoundAnim = sndMngr->loadAnimation(animNameNotConst);
		}
	}
}

//-----------------------------------------------
// getNextAnimList
//-----------------------------------------------
const std::vector<sint8>	&CAnimation::getNextAnimList() const
{
	static std::vector<sint8>	emptyList;
	if(_Sheet)
		return _Sheet->Next;
	else
		return emptyList;
}
