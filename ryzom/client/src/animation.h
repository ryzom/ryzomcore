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



#ifndef CL_ANIMATION_H
#define CL_ANIMATION_H

/////////////
// INCLUDE //
/////////////

//
#include "game_share/magic_fx.h"
#include "client_sheets/animation_set_list_sheet.h"
//
#include "animation_fx.h"


///////////
// CLASS //
///////////
namespace NLGEORGES
{
	class UFormElm;
}

namespace NL3D
{
	class UTrack;
}

class CAnimationFXSet;


/**
 * Class to describe an animation.
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2002
 * \todo GUIGUI : change the animation name to remove ".anim".
 */
class CAnimation
{
public:
	/// Type of an animation.
	enum TAnimId
	{
		UnknownAnim = 0xffffffff
	};
public:
	/// Constructor
	CAnimation();

	// dtor
	~CAnimation();

	/// Build the structure from the georges file read in sheet manager
	void init(CAnimationSheet *sheet, NL3D::UAnimationSet *animationSet);

	/// Return the Animation Id or UnknownAnim.
	uint id() const {return _Animation;}
	/// Return the Animation Id or UnknownAnim.
	const NLSOUND::TSoundAnimId &soundId() const {return _SoundAnim;}
	/// Must apply the Character scale Pos factor to the "pos" channel" of this animation.
	bool applyCharacterScalePosFactor() const {return _Sheet->ApplyCharacterScalePosFactor;}
	/// Return the fx associated with the animation.
	std::string fx() const { return _Sheet->FxNames.get(_Sheet->IdFX); }
	/// Is the head controlable by the code.
	bool headControlable() const {return _Sheet->HeadControlable;}

	/// Is the head controlable by the code.
	bool isReverse() const {return _Sheet->Reverse;}

	/// Is the head controlable by the code.
	bool hideAtEndAnim() const {return _Sheet->HideAtEndAnim;}

	double getRot() const {return _Rot;}
	double virtualRot() const {return _Sheet->VirtualRot;}

	// get fx set
	CAnimationFXSet &getFXSet() { return _FXSet; }
	const CAnimationFXSet &getFXSet() const { return _FXSet; }

	/** Return the next animation to play or -1.
	 *	only filtered anim (in animFilterStates) are used. if none, return -1
	 */
	sint8 getNextAnim(const std::vector<bool>	&animFilterStates) const;

	static std::set<std::string> MissingAnim;

//	static void memoryCompress()	{ _FxNames.memoryCompress(); }
//	static void memoryRelease()		{ _FxNames.clear(); }

	void	resetSoundAnim();
	void	reloadSoundAnim();

	// true if this animation can be played according to the character job/race
	bool	filterOk(uint32 jobSpecialisation, EGSPD::CPeople::TPeople race) const
	{
		if(!_Sheet)
			return true;
		// Common case: No job restriction and no race restriction? => always ok
		if(_Sheet->JobRestriction==0 && _Sheet->RaceRestriction==EGSPD::CPeople::Unknown)
			return true;
		// if job restriction don't match
		if(_Sheet->JobRestriction!=0 && _Sheet->JobRestriction!=jobSpecialisation)
			return false;
		// if race restriction don't match
		if(_Sheet->RaceRestriction!=EGSPD::CPeople::Unknown && _Sheet->RaceRestriction!=race)
			return false;
		// else filter ok
		return true;
	}

	// return the list of Next Anim
	const std::vector<sint8>	&getNextAnimList() const;

private:
	/// Id of the sound animation.
	NLSOUND::TSoundAnimId	_SoundAnim;

	/// Name of the sound animation, for SoundAnimManager Reload
	std::string				_SoundAnimName;

	/// Animation Id
	uint					_Animation;

	/// Rotation in the animation.
	double					_Rot;

	// FX set to launch with the animation
	CAnimationFXSet			_FXSet;


	CAnimationSheet			*_Sheet;

private:
	void computeAnimation(NL3D::UAnimationSet *animationSet, const std::string &animName);
};


#endif // CL_ANIMATION_H

/* End of animation.h */































































