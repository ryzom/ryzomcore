/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef ENTITY_H
#define ENTITY_H

#include <nel/misc/types_nl.h>

// STL includes
#include <map>
#include <string>
#include <vector>

// NeL includes
#include <nel/misc/vector.h>
#include <nel/misc/vectord.h>
#include <nel/misc/quat.h>
#include <nel/3d/animation_time.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/channel_mixer.h>

namespace NL3D
{
class UPlayList;
class UAnimationSet;
}

namespace NLQT
{

class CSlotInfo
{
public:
	CSlotInfo();

	std::string		Animation;
	std::string		Skeleton;
	float			Offset;
	float			StartTime;
	float			EndTime;

	float			StartBlend;
	float			EndBlend;
	float			Smoothness;
	float			SpeedFactor;
	sint32			ClampMode;
	bool			SkeletonInverted;
	bool			Enable;

	CSlotInfo &operator=(const CSlotInfo &);
};


/**
@class CEntity
@brief Class manage animated shape.
@details
Allows you to load animations for shape and skeleton weight.
Contains a built-in playlist. Has management and playback Playlists or Mixer.
*/
class CEntity
{
public:
	struct Mode
	{
		enum List
		{
			PlayList = 1,
			Mixer
		};
	};

	/// Will need for a single or multiple animation shape
	struct SAnimationStatus
	{
		bool	LoopAnim;
		bool	PlayAnim;
		float	CurrentTimeAnim;
		float	StartAnim;
		float	EndAnim;
		float	SpeedAnim;
		int	Mode;

		SAnimationStatus()
			: LoopAnim(false), PlayAnim(false),
			  CurrentTimeAnim(0), StartAnim(0),
			  EndAnim(0), SpeedAnim(1), Mode(Mode::PlayList) {}
	};

	/// Destructor
	~CEntity(void);

	/// Loads a file animations
	/// @param fileName - name animation file
	void loadAnimation(const std::string &fileName);

	/// Loads a file skeleton weight
	void loadSWT(const std::string &fileName);

	/// Adds an animation to a playlist
	/// @param name - name loaded animations
	void addAnimToPlayList(const std::string &name);

	/// Removes the animation from a playlist
	/// @param row - number of animations in the playlist
	void removeAnimToPlayList(uint row);

	/// Swaps animations to a playlist
	/// @param row1 - first number of animations in the playlist
	/// @param row2 - second number of animations in the playlist
	void swapAnimToPlayList(uint row1, uint row2);

	/// Playback animation
	void playbackAnim(bool play);

	/// Reset playlist and animation
	void reset();

	/// Get the total time of animation playlist
	/// @return total time of animation
	float getPlayListLength() const;

	/// get time length single animation
	float getAnimLength(const std::string &name) const;

	/// Get slot infomation
	void setSlotInfo(uint num, CSlotInfo &slotInfo)
	{
		_SlotInfo[num] = slotInfo;
	}

	/// Set use mode playlist or mixer
	void setMode(int mode)
	{
		_AnimationStatus.Mode = mode;
	}

	/// Set in place mode animation
	void setInPlace(bool enabled)
	{
		_inPlace = enabled;
	}

	/// Get in place mode
	bool getInPlace() const
	{
		return _inPlace;
	}

	/// Set inc position
	void setIncPos(bool enabled)
	{
		_incPos = enabled;
	}

	/// Get inc position
	bool getIncPos() const
	{
		return _incPos;
	}

	/// Get information about the current status of playing a playlist
	/// @return struct containing current information playback
	SAnimationStatus getStatus() const
	{
		return _AnimationStatus;
	}

	/// Get name entity
	/// @return name entity
	std::string getName() const
	{
		return _Name;
	}

	/// Get file name shape
	/// @return file name shape
	std::string getFileNameShape() const
	{
		return _FileNameShape;
	}

	/// Get file name skeleton
	/// @return file name skeleton
	std::string getFileNameSkeleton() const
	{
		return _FileNameSkeleton;
	}

	/// Get slot information
	CSlotInfo getSlotInfo(uint num)
	{
		return _SlotInfo[num];
	}

	/// Get list loaded animations files
	std::vector<std::string>& getAnimationList()
	{
		return _AnimationList;
	}

	/// Get playlist animations
	std::vector<std::string>& getPlayListAnimation()
	{
		return _PlayListAnimation;
	}

	/// Get list loaded skeleton weight template files
	std::vector<std::string>& getSWTList()
	{
		return _SWTList;
	}

	/// Get game interface for manipulating Skeleton.
	NL3D::UInstance getInstance() const
	{
		return _Instance;
	}

	/// Get game interface for manipulating Skeleton.
	NL3D::USkeleton getSkeleton() const
	{
		return _Skeleton;
	}

private:
	/// Constructor
	CEntity(void);

	/// Update the animate from the playlist or channel mixer
	/// @param time - current time in second
	void update(NL3D::TAnimationTime time);

	void resetChannel();

	/// Update the animate from the playlist
	void animatePlayList(NL3D::TAnimationTime time);

	/// Update the animate from the mixer
	void animateChannelMixer();
	void addTransformation (NLMISC::CMatrix &current, NL3D::UAnimation *anim,
							float begin, float end,
							NL3D::UTrack *posTrack, NL3D::UTrack *rotquatTrack,
							NL3D::UTrack *nextPosTrack, NL3D::UTrack *nextRotquatTrack,
							bool removeLast);

	// The name of the entity
	std::string _Name;
	std::string	_FileNameShape;
	std::string	_FileNameSkeleton;

	SAnimationStatus _AnimationStatus;

	bool _inPlace;
	bool _incPos;

	float _CharacterScalePos;

	// The mesh instance associated to this entity
	NL3D::UInstance _Instance;

	// The skeleton binded to the instance
	NL3D::USkeleton _Skeleton;

	NL3D::UPlayList *_PlayList;

	NL3D::UAnimationSet *_AnimationSet;

	// Animation input file
	std::vector<std::string> _AnimationList;

	// Skeleton weight input file
	std::vector<std::string> _SWTList;

	// Play list animation
	std::vector<std::string> _PlayListAnimation;

	// Slot info for this object
	CSlotInfo _SlotInfo[NL3D::CChannelMixer::NumAnimationSlot];

	friend class CObjectViewer;
}; /* class CEntity */

typedef std::map<std::string, CEntity> CEntities;
typedef CEntities::iterator	EIT;

} /* namespace NLQT */

#endif // ENTITY_H
