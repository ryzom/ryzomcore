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

#include "nel/3d/animation.h"
#include "nel/3d/animation_set.h"
#include "nel/3d/track.h"
#include "nel/3d/track_sampled_quat_small_header.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/algo.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{

H_AUTO_DECL( NL3D_UI_Animation )

#define	NL3D_HAUTO_UI_ANIMATION						H_AUTO_USE( NL3D_UI_Animation )


// ***************************************************************************

CAnimation::CAnimation() : _BeginTimeTouched(true), _EndTimeTouched(true), _AnimLoopTouched(true)
{
	_MinEndTime = -FLT_MAX;
	_TrackSamplePack= NULL;
	_AnimationSetOwner= NULL;
}

// ***************************************************************************

CAnimation::~CAnimation ()
{
	// Delete all the pointers in the array
	for (uint i=0; i<_TrackVector.size(); i++)
		// Delete
		delete _TrackVector[i];

	// if created, release the _TrackSamplePack
	if(_TrackSamplePack)
		delete _TrackSamplePack;
	_TrackSamplePack= NULL;
}

// ***************************************************************************

void CAnimation::addTrack (const std::string& name, ITrack* pChannel)
{
	// must not already be HeaderOptimized
	nlassert(_IdByChannelId.empty());

	// Add an entry in the map
	_IdByName.insert (TMapStringUInt::value_type (name, (uint32)_TrackVector.size()));

	// Add an entry in the array
	_TrackVector.push_back (pChannel);

	//
	_BeginTimeTouched = _EndTimeTouched = _AnimLoopTouched= true;

}

// ***************************************************************************

void CAnimation::serial (NLMISC::IStream& f)
{
	// cannot save if anim header compressed
	nlassert(_IdByChannelId.empty());

	// Serial a header
	f.serialCheck (NELID("_LEN"));
	f.serialCheck (NELID("MINA"));

	// Serial a version
	sint version=f.serialVersion (2);

	// Serial the name
	f.serial (_Name);

	// Serial the name/id map
	f.serialCont(_IdByName);

	// Serial the vector
	f.serialContPolyPtr (_TrackVector);

	// Serial the min end time
	if (version>=1)
	{
		f.serial (_MinEndTime);
	}

	// Serial the SSS shapes
	if (version>=2)
	{
		f.serialCont (_SSSShapes);
	}

	// TestYoyo
	//nlinfo("ANIMYOYO: Anim NumTracks: %d", _TrackVector.size());
}


// ***************************************************************************
uint CAnimation::getIdTrackByName (const std::string& name) const
{

	// if not be HeaderOptimized
	if (_IdByChannelId.empty())
	{
		// Find an entry in the name/id map
		TMapStringUInt::const_iterator ite=_IdByName.find (name);

		// Not found ?
		if (ite==_IdByName.end ())
			// yes, error
			return NotFound;
		else
			// no, return track ID
			return (uint)ite->second;
	}
	else
	{
		nlassert(_AnimationSetOwner);
		// get the channel id from name
		uint	channelId= _AnimationSetOwner->getChannelIdByName(name);
		if(channelId==CAnimationSet::NotFound)
			return CAnimation::NotFound;
		else
			return getIdTrackByChannelId(channelId);
	}
}

// ***************************************************************************
void CAnimation::getTrackNames (std::set<std::string>& setString) const
{

	// if not be HeaderOptimized
	if (_IdByChannelId.empty())
	{
		// For each track name
		TMapStringUInt::const_iterator ite=_IdByName.begin();
		while (ite!=_IdByName.end())
		{
			// Add the name in the map
			setString.insert (ite->first);

			// Next track
			ite++;
		}
	}
	else
	{
		nlassert(_AnimationSetOwner);
		// For each track channel Id,
		for(uint i=0;i<_IdByChannelId.size();i++)
		{
			// Add in the map the channel name => same as track name
			setString.insert ( _AnimationSetOwner->getChannelName(_IdByChannelId[i]) );
		}
	}
}

// ***************************************************************************
TAnimationTime CAnimation::getBeginTime () const
{
	NL3D_HAUTO_UI_ANIMATION;

	if (_BeginTimeTouched)
	{
		// Track count
		uint trackCount=(uint)_TrackVector.size();

		// Track count empty ?
		if (trackCount==0)
			return 0.f;

		// Look for the lowest
		_BeginTime=_TrackVector[0]->getBeginTime ();

		// Scan all keys
		for (uint t=1; t<trackCount; t++)
		{
			if (_TrackVector[t]->getBeginTime ()<_BeginTime)
				_BeginTime=_TrackVector[t]->getBeginTime ();
		}

		_BeginTimeTouched = false;
	}

	return _BeginTime;
}

// ***************************************************************************

TAnimationTime CAnimation::getEndTime () const
{
	NL3D_HAUTO_UI_ANIMATION;

	if (_EndTimeTouched)
	{
		// Track count
		uint trackCount=(uint)_TrackVector.size();

		// Track count empty ?
		if (trackCount==0)
			return 0.f;

		// Look for the highest
		_EndTime=_TrackVector[0]->getEndTime ();

		// Scan tracks keys
		for (uint t=1; t<trackCount; t++)
		{
			if (_TrackVector[t]->getEndTime ()>_EndTime)
				_EndTime=_TrackVector[t]->getEndTime ();
		}

		// Check min end time
		if (_EndTime < _MinEndTime)
			_EndTime = _MinEndTime;

		_EndTimeTouched = false;
	}

	return _EndTime;
}

// ***************************************************************************
bool			CAnimation::allTrackLoop() const
{
	NL3D_HAUTO_UI_ANIMATION;

	if(_AnimLoopTouched)
	{
		// Track count
		uint trackCount=(uint)_TrackVector.size();

		// Default is true
		_AnimLoop= true;

		// Scan tracks keys
		for (uint t=0; t<trackCount; t++)
		{
			if (!_TrackVector[t]->getLoopMode())
			{
				_AnimLoop= false;
				break;
			}
		}
		_AnimLoopTouched = false;
	}

	return _AnimLoop;
}

// ***************************************************************************

UTrack* CAnimation::getTrackByName (const char* name)
{
	NL3D_HAUTO_UI_ANIMATION;

	// Get track id
	uint id=getIdTrackByName (name);

	// Not found ?
	if (id==CAnimation::NotFound)
		// Error, return NULL
		return NULL;
	else
		// No error, return the track
		return getTrack (id);
}

// ***************************************************************************

void CAnimation::releaseTrack (UTrack* /* track */)
{
	NL3D_HAUTO_UI_ANIMATION;

	// Nothing to do
}

// ***************************************************************************

void CAnimation::setMinEndTime (TAnimationTime minEndTime)
{
	_MinEndTime = minEndTime;
}

// ***************************************************************************

UAnimation* UAnimation::createAnimation (const char* sPath)
{
	NL3D_HAUTO_UI_ANIMATION;

	// Allocate an animation
	std::auto_ptr<CAnimation> anim (new CAnimation);

	// Read it
	NLMISC::CIFile file;
	if (file.open ( NLMISC::CPath::lookup( sPath ) ) )
	{
		// Serial the animation
		file.serial (*anim);

		// Return pointer
		CAnimation *ret=anim.release ();

		// Return the animation interface
		return ret;
	}
	else
		return NULL;
}

// ***************************************************************************

void UAnimation::releaseAnimation (UAnimation* animation)
{
	NL3D_HAUTO_UI_ANIMATION;

	// Cast the pointer
	CAnimation* release=(CAnimation*)animation;

	// Delete it
	delete release;
}

// ***************************************************************************
void	CAnimation::applySampleDivisor(uint sampleDivisor)
{
	NL3D_HAUTO_UI_ANIMATION;

	for(uint i=0;i<_TrackVector.size();i++)
	{
		ITrack	*track= _TrackVector[i];
		if(track)
			track->applySampleDivisor(sampleDivisor);
	}
}


// ***************************************************************************
void	CAnimation::applyTrackQuatHeaderCompression()
{
	NL3D_HAUTO_UI_ANIMATION;

	// if the header compression has already been donne, no op
	if(_TrackSamplePack)
		return;

	// **** First pass: count the number of keys to allocate
	CTrackSampleCounter		sampleCounter;
	bool					someTrackOK= false;
	for(uint i=0;i<_TrackVector.size();i++)
	{
		ITrack	*track= _TrackVector[i];
		if(track)
		{
			// return true only for CTrackSampledQuat
			if(track->applyTrackQuatHeaderCompressionPass0(sampleCounter))
				someTrackOK= true;
		}
	}

	// **** second pass: fill, onlmy if some track matchs (fails for instance for big animations)
	if(someTrackOK)
	{
		uint	i;

		// must create the Sample packer
		_TrackSamplePack= new CTrackSamplePack;

		// just copy the built track headers
		_TrackSamplePack->TrackHeaders.resize((uint32)sampleCounter.TrackHeaders.size());
		for(i=0;i<_TrackSamplePack->TrackHeaders.size();i++)
		{
			_TrackSamplePack->TrackHeaders[i]= sampleCounter.TrackHeaders[i];
		}

		// and allocate keys
		_TrackSamplePack->Times.resize(sampleCounter.NumKeys);
		_TrackSamplePack->Keys.resize(sampleCounter.NumKeys);

		// start the counter for Pass1 to work
		uint	globalKeyOffset= 0;

		// fill it for each track
		for(i=0;i<_TrackVector.size();i++)
		{
			ITrack	*track= _TrackVector[i];
			if(track)
			{
				ITrack	*newTrack= track->applyTrackQuatHeaderCompressionPass1(globalKeyOffset, *_TrackSamplePack);
				// if compressed
				if(newTrack)
				{
					// delete the old track, and replace with compressed one
					delete _TrackVector[i];
					_TrackVector[i]= newTrack;
				}
			}
		}

		nlassert(globalKeyOffset == _TrackSamplePack->Keys.size());
	}


}


// ***************************************************************************
struct CTempTrackInfo
{
	uint16	ChannelId;
	ITrack	*Track;

	bool operator<(const CTempTrackInfo &o) const
	{
		return ChannelId < o.ChannelId;
	}
};
void	CAnimation::applyAnimHeaderCompression(CAnimationSet *animationSetOwner, const std::map <std::string, uint32> &channelMap)
{
	uint	i;
	nlassert(animationSetOwner);
	// must not be already done
	nlassert(_IdByChannelId.empty());

	// fill the track info, with Track
	std::vector<CTempTrackInfo>		tempTrackInfo;
	tempTrackInfo.resize(_TrackVector.size());
	for(i=0;i<tempTrackInfo.size();i++)
	{
		tempTrackInfo[i].Track= _TrackVector[i];
	}

	// fill the track info, with ChannelId
	TMapStringUInt::iterator	it;
	for(it= _IdByName.begin();it!=_IdByName.end();it++)
	{
		// search this track in the channelMap
		std::map <std::string, uint32>::const_iterator	itChan= channelMap.find(it->first);
		nlassert(itChan!=channelMap.end());
		// store the channelId in the associated track
		tempTrackInfo[it->second].ChannelId= (uint16)itChan->second;
	}

	// sort by channelId
	sort(tempTrackInfo.begin(), tempTrackInfo.end());

	// refill the TrackVector (sorted)
	_IdByChannelId.resize( tempTrackInfo.size() );
	for(i=0;i<tempTrackInfo.size();i++)
	{
		_TrackVector[i]= tempTrackInfo[i].Track;
		_IdByChannelId[i]= tempTrackInfo[i].ChannelId;
	}

	// clear the no more needed track map
	contReset(_IdByName);

	// must keep the animSet
	_AnimationSetOwner= animationSetOwner;
}


// ***************************************************************************
uint	CAnimation::getIdTrackByChannelId (uint16 channelId) const
{
	if(_IdByChannelId.empty())
		return CAnimation::NotFound;
	else
	{
		uint	trackId= searchLowerBound(_IdByChannelId, channelId);
		// verify that the channel is really found
		if(_IdByChannelId[trackId]==channelId)
		{
			return trackId;
		}
		else
			return CAnimation::NotFound;
	}
}

// ***************************************************************************
void	CAnimation::addSSSShape(const std::string &shape)
{
	_SSSShapes.push_back(shape);
}


} // NL3D
