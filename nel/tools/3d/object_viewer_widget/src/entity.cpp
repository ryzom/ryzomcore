/*
Object Viewer Qt Widget
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

#include "stdpch.h"
#include "entity.h"

#include <QtCore/QString>

// NeL includes
#include <nel/misc/path.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_animation.h>
#include <nel/3d/u_play_list_manager.h>
#include <nel/3d/u_play_list.h>
#include <nel/3d/u_track.h>

// Project includes
#include "object_viewer_widget.h"

using namespace NLMISC;
using namespace NL3D;

namespace NLQT 
{

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
	_Name("<Unknown>"), _FileNameShape(""),
		_FileNameSkeleton(""), _inPlace(false), _incPos(false),
		_Instance(NULL), _Skeleton(NULL), 
		_PlayList(NULL), _AnimationSet(NULL)
	{
		_CharacterScalePos = 1;
	}

	CEntity::~CEntity(void)
	{
		if (_PlayList != NULL)
		{
			_PlayList->resetAllChannels();
			CObjectViewerWidget::objViewWid().getPlayListManager()->deletePlayList (_PlayList);
			_PlayList = NULL;
		}
		if (_AnimationSet != NULL)
		{
			CObjectViewerWidget::objViewWid().getDriver()->deleteAnimationSet(_AnimationSet);
			_AnimationSet = NULL;
		}
		if (!_Skeleton.empty())
		{
			_Skeleton.detachSkeletonSon(_Instance);
			CObjectViewerWidget::objViewWid().getScene()->deleteSkeleton(_Skeleton);
			_Skeleton = NULL;
		}
		if (!_Instance.empty())
		{
			CObjectViewerWidget::objViewWid().getScene()->deleteInstance(_Instance);
			_Instance = NULL;
		}
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

		_Instance.start();
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

		if (play)
			_Instance.start();
		else
			_Instance.freezeHRC();
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
		for(uint i = 0; i < NL3D::CChannelMixer::NumAnimationSlot; i++)
			_PlayList->setAnimation(i, UPlayList::empty);
	}

	void CEntity::addTransformation (CMatrix &current, UAnimation *anim, float begin, float end, UTrack *posTrack, UTrack *rotquatTrack, 
		UTrack *nextPosTrack, UTrack *nextRotquatTrack, bool removeLast)
	{
		// In place ?
		if (_inPlace)
		{
			// Just identity
			current.identity();
		}
		else
		{
			// Remove the start of the animation
			CQuat rotEnd (0,0,0,1);
			CVector posEnd (0,0,0);
			if (rotquatTrack)
			{
				// Interpolate the rotation
				rotquatTrack->interpolate (end, rotEnd);
			}
			if (posTrack)
			{
				// Interpolate the position
				posTrack->interpolate (end, posEnd);
			}

			// Add the final rotation and position
			CMatrix tmp;
			tmp.identity ();
			tmp.setRot (rotEnd);
			tmp.setPos (posEnd);

			// Incremental ?
			if (_incPos)
				current *= tmp;
			else
				current = tmp;

			if (removeLast)
			{
				CQuat rotStart (0,0,0,1);
				CVector posStart (0,0,0);
				if (nextRotquatTrack)
				{
					// Interpolate the rotation
					nextRotquatTrack->interpolate (begin, rotStart);
				}
				if (nextPosTrack)
				{
					// Interpolate the position
					nextPosTrack->interpolate (begin, posStart);
				}
				// Remove the init rotation and position of the next animation
				tmp.identity ();
				tmp.setRot (rotStart);
				tmp.setPos (posStart);
				tmp.invert ();
				current *= tmp;

				// Normalize the mt
				CVector I = current.getI ();
				CVector J = current.getJ ();
				I.z = 0;
				J.z = 0;
				J.normalize ();
				CVector K = I^J;
				K.normalize ();
				I = J^K;
				I.normalize ();
				tmp.setRot (I, J, K);
				tmp.setPos (current.getPos ());
				current = tmp;
			}
		}
	}

	void CEntity::animatePlayList(NL3D::TAnimationTime time)
	{
		if (!_PlayListAnimation.empty())
		{
			// Animation index
			uint id = _AnimationSet->getAnimationIdByName(_PlayListAnimation[0].c_str());

			// Try channel AnimationSet
			NL3D::UAnimation *anim = _AnimationSet->getAnimation(id);

			bool there = false;

			UTrack *posTrack = NULL;
			UTrack *rotQuatTrack = NULL;

			// Current matrix
			CMatrix current;
			current.identity();

			// read an animation for init matrix
			rotQuatTrack = anim->getTrackByName("rotquat");
			posTrack = anim->getTrackByName("pos");

			there = posTrack || rotQuatTrack;

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
					NL3D::UAnimation *newAnim = _AnimationSet->getAnimation(id);

					UTrack *newPosTrack = newAnim->getTrackByName ("pos");
					UTrack *newRotquatTrack = newAnim->getTrackByName ("rotquat");

					// Add the transformation
					addTransformation (current, anim, newAnim->getBeginTime(), anim->getEndTime(), posTrack, rotQuatTrack, newPosTrack, newRotquatTrack, true);


					anim = newAnim;
					posTrack = newPosTrack;
					rotQuatTrack = newRotquatTrack;

					// Add start time
					startTime = endTime;
					endTime = startTime + (anim->getEndTime() - anim->getBeginTime());
				}
				else 
				{
					// Add the transformation
					addTransformation (current, anim, 0, anim->getEndTime(), posTrack, rotQuatTrack, NULL, NULL, false);
					break;
				}
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

				// Add the transformation
				addTransformation (current, anim, 0, anim->getBeginTime() + time - startTime, posTrack, rotQuatTrack, NULL, NULL, false);

				id = _AnimationSet->getAnimationIdByName(_PlayListAnimation[index].c_str());
				anim = _AnimationSet->getAnimation(id);

				// Final time
				startTime -= anim->getBeginTime();
			}

			// Set the slot
			_PlayList->setAnimation(0, id);
			_PlayList->setTimeOrigin(0, startTime);
			_PlayList->setSpeedFactor(0, 1.0f);
			_PlayList->setWeightSmoothness(0, 1.0f);
			_PlayList->setStartWeight(0, 1, 0);
			_PlayList->setEndWeight(0, 1, 1);
			_PlayList->setWrapMode(0, UPlayList::Clamp);

			// Setup the pos and rot for this shape
			if (there)
			{
				CVector pos = current.getPos();

				// If a  skeleton model
				if(!_Skeleton.empty())
				{
					// scale animated pos value with the CFG scale
					pos *= _CharacterScalePos;
					_Skeleton.setPos(pos);
					_Skeleton.setRotQuat(current.getRot());
				}
				else
				{
					_Instance.setPos(pos);
					_Instance.setRotQuat(current.getRot());
				}
			}
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
