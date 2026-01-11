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

#include "std3d.h"

#include "nel/3d/channel_mixer.h"
#include "nel/3d/track.h"
#include "nel/3d/animatable.h"
#include "nel/3d/skeleton_weight.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************

CChannelMixer::CChannelMixer()
{
	// No channel in the list
	_FirstChannelGlobal=NULL;
	_FirstChannelDetail=NULL;

	// No animation set
	_AnimationSet=NULL;

	// Mixer no dirty
	_Dirt=false;
	_ListToEvalDirt= false;

	// never evaluated.
	_LastEvalDetailDate= -1;
}

// ***************************************************************************

CChannelMixer::~CChannelMixer()
{
	resetChannels();
}

// ***************************************************************************

void CChannelMixer::setAnimationSet (const CAnimationSet* animationSet)
{
	// Set the animationSet Pointer
	_AnimationSet=animationSet;

	// clear the channels.
	resetChannels();
}

// ***************************************************************************

const CAnimationSet* CChannelMixer::getAnimationSet () const
{
	// Return the animationSet Pointer
	return _AnimationSet;
}

// ***************************************************************************
// Temp Data
static CAnimatedValueBlock	TempAnimatedValueBlock;


// ***************************************************************************
void CChannelMixer::evalSingleChannel(CChannel &chan, uint numActive, uint activeSlot[NumAnimationSlot])
{
	// If the refPtr of the object handled has been deleted, then no-op
	if(!chan._Object)
	{
		return;
	}

	// For Quat animated value only.
	CQuat	firstQuat;

	// First slot found
	bool bFirst=true;

	// Last blend factor
	float lastBlend=0.0;

	// Eval each slot
	for (uint a=0; a<numActive; a++)
	{
		// Slot number
		uint slot=activeSlot[a];

		// Current blend factor
		float blend=chan._Weights[slot]*_SlotArray[slot]._Weight;

		if(blend!=0.0f)
		{
			// Eval the track at this time
			const IAnimatedValue	&trackResult= ((ITrack*)chan._Tracks[slot])->eval (_SlotArray[slot]._Time, TempAnimatedValueBlock);

			// First track to be eval ?
			if (bFirst)
			{
				// If channel is a Quaternion animated Value, must store the first Quat.
				if (chan._IsQuat)
				{
					CAnimatedValueBlendable<NLMISC::CQuat>	&quatValue=(CAnimatedValueBlendable<NLMISC::CQuat>&)trackResult;
					firstQuat=quatValue.Value;
				}

				// Copy the interpolated value
				chan._Value->affect (trackResult);

				// First blend factor
				lastBlend=blend;

				// Not first anymore
				bFirst=false;
			}
			else
			{
				// If channel is a Quaternion animated Value, must makeClosest the ith result of the track, from firstQuat.
				if (chan._IsQuat)
				{
					CAnimatedValueBlendable<NLMISC::CQuat>	&quatValue=(CAnimatedValueBlendable<NLMISC::CQuat>&)trackResult;
					quatValue.Value.makeClosest (firstQuat);
				}

				// Blend with this value and the previous sum
				chan._Value->blend (trackResult, lastBlend/(lastBlend+blend));

				// last blend update
				lastBlend+=blend;
			}
		}

		// NB: if all weights are 0, the AnimatedValue is not modified...
	}

	// Touch the animated value and its owner to recompute them later.
	chan._Object->touch (chan._ValueId, chan._OwnerValueId);
}


// ***************************************************************************

void CChannelMixer::eval (bool detail, uint64 evalDetailDate)
{
	// eval the detail animation only one time per scene traversal.
	if(detail)
	{
		if((sint64)evalDetailDate== _LastEvalDetailDate)
			return;
		_LastEvalDetailDate= evalDetailDate;
	}

	// clean list according to anim setup
	if(_Dirt)
	{
		refreshList();
		cleanAll();
	}

	// clean eval list, according to channels enabled.
	if(_ListToEvalDirt)
	{
		refreshListToEval();
		nlassert(!_ListToEvalDirt);
	}

	// If the number of channels to draw is 0, quick quit.
	CChannel	**channelArrayPtr;
	uint		numChans;
	if(detail)
	{
		numChans= (uint)_DetailListToEval.size();
		if(numChans)
			channelArrayPtr= &_DetailListToEval[0];
		else
			return;
	}
	else
	{
		numChans= (uint)_GlobalListToEval.size();
		if(numChans)
			channelArrayPtr= &_GlobalListToEval[0];
		else
			return;
	}

	// Setup an array of animation that are not empty and stay. HTimer: 0.0% (because CLod skeletons not parsed here)
	uint numActive=0;
	uint activeSlot[NumAnimationSlot];
	for (uint s=0; s<NumAnimationSlot; s++)
	{
		// Dirt, not empty and has an influence? (add)
		if (!_SlotArray[s].isEmpty() && _SlotArray[s]._Weight>0)
			// Add a dirt slot
			activeSlot[numActive++]=s;
	}

	// no slot enabled at all?? skip
	if(numActive==0)
		return;

	// For each selected channel
	// fast 'just one slot Activated' version
	if(numActive==1)
	{
		// Slot number
		uint			slot=activeSlot[0];
		// Slot time
		TAnimationTime	slotTime= _SlotArray[slot]._Time;

		// For all channels
		for(;numChans>0; numChans--, channelArrayPtr++)
		{
			CChannel	&chan= **channelArrayPtr;

			// If the refPtr of the object handled has been deleted, then no-op
			if(!chan._Object)
			{
				continue;
			}

			// if Current blend factor is not 0
			if(chan._Weights[slot]!=0.0f)
			{
				// Eval the track and copy the interpolated value. HTimer: 1.4%
				chan._Value->affect (((ITrack*)chan._Tracks[slot])->eval (slotTime, TempAnimatedValueBlock));

				// Touch the animated value and its owner to recompute them later. HTimer: 0.6%
				chan._Object->touch (chan._ValueId, chan._OwnerValueId);
			}
		}
	}
	// little bit slower Blend version
	else
	{
		// For all channels
		for(;numChans>0; numChans--, channelArrayPtr++)
		{
			evalSingleChannel(**channelArrayPtr, numActive, activeSlot);
		}
	}
}


// ***************************************************************************
void CChannelMixer::evalChannels(sint *channelIdArray, uint numID)
{
	if (!channelIdArray) return;
	if (numID == 0) return;

	// Setup an array of animation that are not empty and stay
	uint numActive=0;
	uint activeSlot[NumAnimationSlot];

	// clean list according to anim setup
	if(_Dirt)
	{
		refreshList();
		cleanAll();
	}

	// clean eval list, according to channels enabled.
	if(_ListToEvalDirt)
	{
		refreshListToEval();
		nlassert(!_ListToEvalDirt);
	}

	// Setup it up
	for (uint s=0; s<NumAnimationSlot; s++)
	{
		// Dirt, not empty and has an influence? (add)
		if (!_SlotArray[s].isEmpty() && _SlotArray[s]._Weight>0)
			// Add a dirt slot
			activeSlot[numActive++]=s;
	}

	// no slot enabled at all?? skip
	if(numActive==0)
		return;

	for(uint k = 0; k < numID; ++k)
	{
		std::map<uint, CChannel>::iterator it = _Channels.find(channelIdArray[k]);
		if (it != _Channels.end())
		{
			evalSingleChannel(it->second, numActive, activeSlot);
		}
	}
}




// ***************************************************************************

sint CChannelMixer::addChannel (const string& channelName, IAnimatable* animatable, IAnimatedValue* value, ITrack* defaultValue, uint32 valueId, uint32 ownerValueId, bool detail)
{
	// Check the animationSet has been set
	nlassert (_AnimationSet);

	// Check args
	nlassert (animatable);
	nlassert (value);
	nlassert (defaultValue);

	// Get the channel Id having the same name than the tracks in this animation set.
	uint iDInAnimationSet=_AnimationSet->getChannelIdByName (channelName);

	// Tracks exist in this animation set?
	if (iDInAnimationSet!=CAnimationSet::NotFound)
	{
		// The channel entry
		CChannel	entry;

		// Set the channel name
		entry._ChannelName=channelName;

		// Set the object pointer
		entry._Object=animatable;

		// Set the pointer on the value in the object
		entry._Value=value;

		// Is this a CQuat animated value???
		entry._IsQuat= (typeid (*(entry._Value))==typeid (CAnimatedValueBlendable<NLMISC::CQuat>))!=0;


		// Set the default track pointer
		entry._DefaultTracks=defaultValue;

		// Set the value ID in the object
		entry._ValueId=valueId;

		// Set the First value ID in the object
		entry._OwnerValueId=ownerValueId;

		// in what mode is the channel?
		entry._Detail= detail;

		// All weights default to 1. All Tracks default to defaultTrack.
		for(sint s=0;s<NumAnimationSlot;s++)
		{
			entry._Weights[s]= 1.0f;
			entry._Tracks[s]= entry._DefaultTracks;
		}

		// add (if not already done) the entry in the map.
		_Channels[iDInAnimationSet]= entry;

		// Dirt all the slots
		dirtAll ();

		// Affect the default value in the animated value
		entry._Value->affect (((ITrack*)(entry._DefaultTracks))->eval(0, TempAnimatedValueBlock));

		// Touch the animated value and its owner to recompute them later.
		entry._Object->touch (entry._ValueId, entry._OwnerValueId);

		// return the id.
		return iDInAnimationSet;
	}
	else
	{
		// return Not found.
		return -1;
	}
}

// ***************************************************************************

void CChannelMixer::resetChannels ()
{
	// clear
	_Channels.clear();
	dirtAll ();
}


// ***************************************************************************
void CChannelMixer::enableChannel (uint channelId, bool enable)
{
	std::map<uint, CChannel>::iterator	it= _Channels.find(channelId);
	if(it!=_Channels.end())
	{
		it->second._EnableFlags &= ~CChannel::EnableUserFlag;
		if(enable)
			it->second._EnableFlags |= CChannel::EnableUserFlag;

		// Must recompute the channels to animate.
		_ListToEvalDirt= true;
	}
}


// ***************************************************************************
bool CChannelMixer::isChannelEnabled (uint channelId) const
{
	std::map<uint, CChannel>::const_iterator	it= _Channels.find(channelId);
	if(it!=_Channels.end())
	{
		return (it->second._EnableFlags & CChannel::EnableUserFlag) != 0;
	}
	else
		return false;
}


// ***************************************************************************
void CChannelMixer::lodEnableChannel (uint channelId, bool enable)
{
	std::map<uint, CChannel>::iterator	it= _Channels.find(channelId);
	if(it!=_Channels.end())
	{
		it->second._EnableFlags &= ~CChannel::EnableLodFlag;
		if(enable)
			it->second._EnableFlags |= CChannel::EnableLodFlag;

		// Must recompute the channels to animate.
		_ListToEvalDirt= true;
	}
}


// ***************************************************************************
bool CChannelMixer::isChannelLodEnabled (uint channelId) const
{
	std::map<uint, CChannel>::const_iterator	it= _Channels.find(channelId);
	if(it!=_Channels.end())
	{
		return (it->second._EnableFlags & CChannel::EnableLodFlag) != 0;
	}
	else
		return false;
}



// ***************************************************************************

void CChannelMixer::setSlotAnimation (uint slot, uint animation)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Check an animationSet as been set.
	nlassert (_AnimationSet);

	// Find the animation pointer for this animation
	const CAnimation* pAnimation=_AnimationSet->getAnimation (animation);

	// Does this animation change ?
	if (_SlotArray[slot]._Animation!=pAnimation)
	{
		// Change it
		_SlotArray[slot]._Animation=pAnimation;

		// Dirt it
		_SlotArray[slot]._Dirt=true;

		// Dirt the mixer
		_Dirt=true;
	}
}

// ***************************************************************************

const CAnimation	*CChannelMixer::getSlotAnimation(uint slot) const
{
	nlassert(slot < NumAnimationSlot);
	return _SlotArray[slot]._Animation;
}


// ***************************************************************************

void CChannelMixer::emptySlot (uint slot)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Does this animation already empty ?
	if (!_SlotArray[slot].isEmpty ())
	{
		// Change it
		_SlotArray[slot].empty ();

		// Dirt it
		_SlotArray[slot]._Dirt=true;

		// Dirt the mixer
		_Dirt=true;
	}
}

// ***************************************************************************

void CChannelMixer::resetSlots ()
{
	// Empty all slots
	for (uint s=0; s<NumAnimationSlot; s++)
		// Empty it
		emptySlot (s);
}

// ***************************************************************************

void CChannelMixer::applySkeletonWeight (uint slot, uint skeleton, bool invert)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Check the animationSet has been set
	nlassert (_AnimationSet);

	// Get the skeleton weight
	const CSkeletonWeight *pSkeleton=_AnimationSet->getSkeletonWeight (skeleton);

	// Something to change ?
	if ((pSkeleton!=_SlotArray[slot]._SkeletonWeight)||(invert!=_SlotArray[slot]._InvertedSkeletonWeight))
	{
		// Set the current skeleton
		_SlotArray[slot]._SkeletonWeight=pSkeleton;
		_SlotArray[slot]._InvertedSkeletonWeight=invert;

		// Get number of node in the skeleton weight
		uint sizeSkel=pSkeleton->getNumNode ();

		// For each entry of the skeleton weight
		for (uint n=0; n<sizeSkel; n++)
		{
			// Get the name of the channel for this node
			const string& channelName=pSkeleton->getNodeName (n);

			// Get the channel Id having the same name than the tracks in this animation set.
			uint channelId=_AnimationSet->getChannelIdByName (channelName);

			// Tracks exist in this animation set?
			if (channelId!=CAnimationSet::NotFound)
			{
				// Get the weight of the channel for this node
				float weight=pSkeleton->getNodeWeight (n);

				// Set the weight of this channel for this slot (only if channel setuped!!)
				std::map<uint, CChannel>::iterator ite=_Channels.find(channelId);
				if (ite!=_Channels.end())
					ite->second._Weights[slot]=invert?1.f-weight:weight;
			}
		}
	}
}

// ***************************************************************************

void CChannelMixer::resetSkeletonWeight (uint slot)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Something to change ?
	if (_SlotArray[slot]._SkeletonWeight!=NULL)
	{
		// Set skeleton
		_SlotArray[slot]._SkeletonWeight=NULL;
		_SlotArray[slot]._InvertedSkeletonWeight=false;

		// For each channels
		map<uint, CChannel>::iterator		itChannel;
		for(itChannel= _Channels.begin(); itChannel!=_Channels.end();itChannel++)
		{
			// Reset
			(*itChannel).second._Weights[slot]=1.f;
		}
	}
}

// ***************************************************************************

void CChannelMixer::cleanAll ()
{
	// For each slot
	for (uint s=0; s<NumAnimationSlot; s++)
	{
		// Clean it
		_SlotArray[s]._Dirt=false;
	}

	// Clean the mixer
	_Dirt=false;
}

// ***************************************************************************

void CChannelMixer::dirtAll ()
{
	// For each slot
	for (uint s=0; s<NumAnimationSlot; s++)
	{
		// Dirt
		if (!_SlotArray[s].isEmpty())
		{
			// Dirt it
			_SlotArray[s]._Dirt=true;

			// Dirt the mixer
			_Dirt=true;
		}
	}
}

// ***************************************************************************

void CChannelMixer::refreshList ()
{
	// Setup an array of animation to add
	uint numAdd=0;
	uint addSlot[NumAnimationSlot];

	// Setup an array of animation that are not empty and stay
	uint numStay=0;
	uint staySlot[NumAnimationSlot];

	// Setup it up
	uint s;
	for (s=0; s<NumAnimationSlot; s++)
	{
		// Dirt and not empty ? (add)
		if ((_SlotArray[s]._Dirt)&&(!_SlotArray[s].isEmpty()))
			// Add a dirt slot
			addSlot[numAdd++]=s;

		// Not empty and not dirt ? (stay)
		if ((!_SlotArray[s]._Dirt)&&(!_SlotArray[s].isEmpty()))
			// Add a dirt slot
			staySlot[numStay++]=s;
	}

	// Last channel pointer
	CChannel **lastPointerGlobal=&_FirstChannelGlobal;
	CChannel **lastPointerDetail=&_FirstChannelDetail;


	// Now scan each channel
	map<uint, CChannel>::iterator		itChannel;
	for(itChannel= _Channels.begin(); itChannel!=_Channels.end();itChannel++)
	{
		uint		channelId= itChannel->first;
		CChannel	&channel= (*itChannel).second;

		// Add this channel to the list if true
		bool add=false;

		// For each slot to add
		for (s=0; s<numAdd; s++)
		{
			uint iDTrack;

			// If the animation set is header compressed,
			if(_AnimationSet->isAnimHeaderOptimized())
				// can retrieve the animation trough the channel id (faster)
				iDTrack= _SlotArray[addSlot[s]]._Animation->getIdTrackByChannelId(channelId);
			else
				// get by name
				iDTrack= _SlotArray[addSlot[s]]._Animation->getIdTrackByName (channel._ChannelName);

			// If this track exist
			if (iDTrack!=CAnimation::NotFound)
			{
				// Set the track
				channel._Tracks[addSlot[s]]=_SlotArray[addSlot[s]]._Animation->getTrack (iDTrack);

				// Add this channel to the list
				add=true;
			}
			else
			{
				// Set the default track
				channel._Tracks[addSlot[s]]=channel._DefaultTracks;
			}
		}

		// Add this channel to the list ?
		if (!add)
		{
			// Was it in the list ?
			if (channel._InTheList)
			{
				// Check if this channel is still in use

				// For each slot in the stay list
				for (s=0; s<numStay; s++)
				{
					// Use anything interesting ?
					if (channel._Tracks[staySlot[s]]!=channel._DefaultTracks)
					{
						// Ok, add it to the list
						add=true;

						// Stop
						break;
					}
				}

				// Still in use?
				if (!add)
				{
					// Ensure first the object is not deleted
					if(channel._Object)
					{
						// Set it's value to default and touch it's object
						channel._Value->affect (((ITrack*)(channel._DefaultTracks))->eval(0, TempAnimatedValueBlock));
						channel._Object->touch (channel._ValueId, channel._OwnerValueId);
					}
				}
			}
		}

		// Do i have to add the channel to the list
		if (add)
		{
			// It is in the list
			channel._InTheList=true;

			if(channel._Detail)
			{
				// Set the last pointer value
				*lastPointerDetail=&channel;
				// Change last pointer
				lastPointerDetail=&channel._Next;
			}
			else
			{
				// Set the last pointer value
				*lastPointerGlobal=&channel;
				// Change last pointer
				lastPointerGlobal=&channel._Next;
			}
		}
		else
		{
			// It is not in the list
			channel._InTheList=false;
		}
	}

	// End of the list
	*lastPointerGlobal=NULL;
	*lastPointerDetail=NULL;

	// Must recompute the channels to animate.
	_ListToEvalDirt= true;
}


// ***************************************************************************
void CChannelMixer::refreshListToEval ()
{
	CChannel* pChannel;

	/* NB: this save if(), especially when Used with Skeleton, and CLod mode
	*/

	// Global list.
	_GlobalListToEval.clear();
	_GlobalListToEval.reserve(_Channels.size());
	pChannel=_FirstChannelGlobal;
	while(pChannel)
	{
		// if the channel is enabled (both user and lod), must eval all active slot.
		if(pChannel->_EnableFlags == CChannel::EnableAllFlag)
			_GlobalListToEval.push_back(pChannel);
		// next
		pChannel= pChannel->_Next;
	}

	// Global list.
	_DetailListToEval.clear();
	_DetailListToEval.reserve(_Channels.size());
	pChannel=_FirstChannelDetail;
	while(pChannel)
	{
		// if the channel is enabled (both user and lod), must eval all active slot.
		if(pChannel->_EnableFlags == CChannel::EnableAllFlag)
			_DetailListToEval.push_back(pChannel);
		// next
		pChannel= pChannel->_Next;
	}

	// done
	_ListToEvalDirt= false;
}

// ***************************************************************************
void CChannelMixer::resetEvalDetailDate()
{
	_LastEvalDetailDate= -1;
}


} // NL3D
