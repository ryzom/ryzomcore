/*
    Georges Editor Qt
	Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

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

#include "entity.h"

// NeL includes
#include <nel/misc/path.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_animation.h>
#include <nel/3d/u_play_list_manager.h>
#include <nel/3d/u_play_list.h>
#include <nel/3d/u_track.h>

// Project includes
#include "modules.h"

using namespace NLMISC;
using namespace NL3D;

namespace NLQT {

CSlotInfo& CSlotInfo::operator=(const CSlotInfo & slotInfo)
{
	if ( this != &slotInfo)
	{
		Animation = slotInfo.Animation;
		ClampMode = slotInfo.ClampMode;
		Enable = slotInfo.Enable;
		EndBlend = slotInfo.EndBlend;
		EndTime = slotInfo.EndTime;
		Offset = slotInfo.Offset;
		Skeleton = slotInfo.Skeleton;
		SkeletonInverted = slotInfo.SkeletonInverted;
		Smoothness = slotInfo.Smoothness;
		SpeedFactor = slotInfo.SpeedFactor;
		StartBlend = slotInfo.StartBlend;
		StartTime = slotInfo.StartTime;
	}
	return *this;
}
  
CEntity::CEntity(void):
		_Name("<Unknown>"),
		_Instance(NULL), _Skeleton(NULL), 
		_PlayList(NULL), _AnimationSet(NULL)
{
}

CEntity::~CEntity(void)
{
}

void CEntity::loadAnimation(std::string &fileName)
{
	uint id = _AnimationSet->addAnimation(fileName.c_str(),CFile::getFilenameWithoutExtension(fileName).c_str());	
	_AnimationList.push_back(_AnimationSet->getAnimationName(id));
	_AnimationSet->build();
	if (!_Skeleton.empty()) _PlayList->registerTransform(_Skeleton);
	else _PlayList->registerTransform(_Instance);
}

void CEntity::loadSWT(std::string &fileName)
{
	uint id = _AnimationSet->addSkeletonWeight(fileName.c_str(),CFile::getFilenameWithoutExtension(fileName).c_str());
	_SWTList.push_back(_AnimationSet->getSkeletonWeightName(id));
}

void CEntity::addAnimToPlayList(std::string &name)
{
	_PlayListAnimation.push_back(name);
	
	_AnimationStatus.EndAnim = this->getPlayListLength();
}

void CEntity::removeAnimToPlayList(uint row)
{
	if (row < _PlayListAnimation.size())
	 _PlayListAnimation.erase(_PlayListAnimation.begin() + row);
	
	_AnimationStatus.EndAnim = this->getPlayListLength();
}

void CEntity::swapAnimToPlayList(uint row1, uint row2)
{
	if ((row1 < _PlayListAnimation.size()) && (row2 < _PlayListAnimation.size()))
	 std::swap(_PlayListAnimation[row1], _PlayListAnimation[row2]);
}

void CEntity::playbackAnim(bool play)
{
	_AnimationStatus.PlayAnim = play;
}

void CEntity::reset()
{
	_PlayListAnimation.clear();
	_AnimationList.clear();
	_SWTList.clear();
	
	_PlayList->resetAllChannels();
}

float CEntity::getPlayListLength()
{
	// Accumul all the time
	float time = 0;
	for(size_t i = 0; i < _PlayListAnimation.size(); ++i)
		time += getAnimLength(_PlayListAnimation[i]);
	return time;
}

float CEntity::getAnimLength(std::string name)
{
	uint id = _AnimationSet->getAnimationIdByName(name.c_str());
	NL3D::UAnimation *anim = _AnimationSet->getAnimation(id);
	return anim->getEndTime() - anim->getBeginTime();
}

void CEntity::update(NL3D::TAnimationTime time)
{
	this->resetChannel();
	switch (_AnimationStatus.Mode) 
	{
	case Mode::PlayList:  
		animatePlayList(time);
		break;
	case Mode::Mixer:
		animateChannelMixer();
		break;
	}
}


void CEntity::resetChannel()
{
	for(size_t i = 0; i < NL3D::CChannelMixer::NumAnimationSlot; i++)
	 _PlayList->setAnimation(i, UPlayList::empty);
}

void CEntity::animatePlayList(NL3D::TAnimationTime time)
{
	if (!_PlayListAnimation.empty())
	{
		// Animation index
		uint id = _AnimationSet->getAnimationIdByName(_PlayListAnimation[0].c_str());
		
		// Try channel AnimationSet
		NL3D::UAnimation *anim = _AnimationSet->getAnimation(id);
		
		// Accumul time
		float startTime = 0;
		float endTime = anim->getEndTime() - anim->getBeginTime();
		
		uint index = 0;
		while (time >= endTime)
		{
			index++;
			if (index < _PlayListAnimation.size())
			{
				id = _AnimationSet->getAnimationIdByName(_PlayListAnimation[index].c_str());
				anim = _AnimationSet->getAnimation(id);
			
				// Add start time
				startTime = endTime;
				endTime = startTime + (anim->getEndTime() - anim->getBeginTime());
			}
			else 
			  break;
		}
		
		// Time cropped ?
		if (index >= _PlayListAnimation.size())
		{
			// Yes
			index--;
			id = _AnimationSet->getAnimationIdByName(_PlayListAnimation[index].c_str());
			anim = _AnimationSet->getAnimation(id);
		
			// End time for last anim
			startTime = anim->getEndTime() - time;
		}
		else
		{
			// No 
			id = _AnimationSet->getAnimationIdByName(_PlayListAnimation[index].c_str());
			anim = _AnimationSet->getAnimation(id);
			
			// Final time
			startTime -= anim->getBeginTime();
		}
		
		// Set the slot
		_PlayList->setAnimation(0, id);
		_PlayList->setTimeOrigin(0, startTime);
		_PlayList->setWeightSmoothness(0, 1.0f);
		_PlayList->setStartWeight(0, 1, 0);
		_PlayList->setEndWeight(0, 1, 1);
		_PlayList->setWrapMode(0, UPlayList::Clamp);
	}
}

void CEntity::animateChannelMixer()
{
	for (uint i = 0; i < NL3D::CChannelMixer::NumAnimationSlot; i++)
	{
		if (_SlotInfo[i].Enable)
		{
			// Set the animation
			uint animId =  _AnimationSet->getAnimationIdByName(_SlotInfo[i].Animation);
			if (animId == UAnimationSet::NotFound) 
				_PlayList->setAnimation(i, UPlayList::empty);
			else 
				_PlayList->setAnimation(i, animId);
		
			// Set the skeleton weight
			uint skelId = _AnimationSet->getSkeletonWeightIdByName(_SlotInfo[i].Skeleton);
			if (skelId == UAnimationSet::NotFound)
				_PlayList->setSkeletonWeight(i, UPlayList::empty, false);
			else
				_PlayList->setSkeletonWeight(i, skelId, _SlotInfo[i].SkeletonInverted);

			// Set others values
			_PlayList->setTimeOrigin(i, _SlotInfo[i].Offset);
			_PlayList->setSpeedFactor(i, _SlotInfo[i].SpeedFactor);
			_PlayList->setStartWeight(i, _SlotInfo[i].StartBlend, _SlotInfo[i].StartTime);
			_PlayList->setEndWeight(i, _SlotInfo[i].EndBlend, _SlotInfo[i].EndTime);
			_PlayList->setWeightSmoothness(i, _SlotInfo[i].Smoothness);

			// Switch between wrap modes
			switch (_SlotInfo[i].ClampMode)
			{
			case 0:
				_PlayList->setWrapMode (i, UPlayList::Clamp);
				break;
			case 1:
				_PlayList->setWrapMode (i, UPlayList::Repeat);
				break;
			case 2:
				_PlayList->setWrapMode (i, UPlayList::Disable);
				break;
			}
		}
	}
}

} /* namespace NLQT */
