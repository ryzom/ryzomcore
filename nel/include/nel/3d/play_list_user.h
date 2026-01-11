// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_PLAY_LIST_USER_H
#define NL_PLAY_LIST_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_play_list.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/animation_set.h"
#include "nel/3d/channel_mixer.h"
#include "nel/3d/animation_playlist.h"


namespace NL3D
{


// ***************************************************************************
/**
 * UPlayList implementation. Basicly, this is a playlist, plus a ChannelMixer.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPlayListUser : public UPlayList
{
private:
	CAnimationPlaylist					_PlayList;
	CChannelMixer						_ChannelMixer;
	// just take a refernece on a animation set, so _ChannelMixer has always a good pointer.
	NLMISC::CSmartPtr<CAnimationSet>	_AnimationSet;

	friend class CPlayListManagerUser;


public:

	/// Constructor
	CPlayListUser(NLMISC::CSmartPtr<CAnimationSet>		animationSet)
	{
		nlassert(animationSet!=NULL);
		_AnimationSet= animationSet;

		_ChannelMixer.setAnimationSet(_AnimationSet);

		nlassert((uint)UPlayList::empty == (uint)CAnimationPlaylist::empty);
		nlassert((uint)UPlayList::WrapModeCount == (uint)CAnimationPlaylist::WrapModeCount);
	}


	/// \name Animatable Link.
	// @{
	virtual	void	registerTransform(UTransform object, const char* prefix);
	virtual	void	resetAllChannels();
	// @}


	/// \name Animation Setup.
	// @{
	virtual	void emptyPlayList ();
	virtual	void setAnimation (uint8 slot, uint animation);
	virtual	uint getAnimation (uint8 slot) const;
	// @}


	/// \name Animation Time Setup.
	// @{
	virtual	void setTimeOrigin (uint8 slot, TGlobalAnimationTime timeOrigin);
	virtual	TGlobalAnimationTime getTimeOrigin (uint8 slot) const;
	virtual	void setSpeedFactor (uint8 slot, float speedFactor);
	virtual	float getSpeedFactor (uint8 slot) const;
	virtual	void setWrapMode (uint8 slot, TWrapMode wrapMode);
	virtual	TWrapMode getWrapMode (uint8 slot) const;
	// @}


	/// \name Animation Weight Setup.
	// @{
	virtual	void setStartWeight (uint8 slot, float startWeight, TGlobalAnimationTime time);
	virtual	float getStartWeight (uint8 slot, TGlobalAnimationTime& time) const;
	virtual	void setEndWeight (uint8 slot, float endWeight, TGlobalAnimationTime time);
	virtual	float getEndWeight (uint8 slot, TGlobalAnimationTime& time) const;
	virtual	void setWeightSmoothness (uint8 slot, float smoothness);
	virtual	float getWeightSmoothness (uint8 slot) const;
	virtual	void setWeight (uint8 slot, float weight);

	virtual TAnimationTime getLocalTime (uint8 slot, TGlobalAnimationTime globalTime, const UAnimationSet& animSet) const;
	virtual float getLocalWeight (uint8 slot, TGlobalAnimationTime globalTime) const;

	// @}



	/// \name Skeleton Weight Setup.
	// @{
	virtual	void setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted=false);
	virtual	uint getSkeletonWeight (uint8 slot, bool &inverted) const;
	// @}


	/// \name Special channel operation.
	// @{
	virtual	void enableChannel (uint channelId, bool enable);
	virtual	bool isChannelEnabled (uint channelId) const;
	// @}

public:
	/// Tool function. setup the mixer, and eval global channels
	void	evalPlayList(double playTime);

};


} // NL3D


#endif // NL_PLAY_LIST_USER_H

/* End of play_list_user.h */
