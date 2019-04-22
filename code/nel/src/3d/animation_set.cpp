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

#include "nel/3d/animation_set.h"
#include "nel/3d/driver.h"
#include "nel/3d/shape_bank.h"
#include "nel/misc/stream.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"



using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************
CAnimationSet::CAnimationSet (bool headerOptim)
{
	_SampleDivisor= 1;
	_AnimHeaderOptimisation= headerOptim;
	_Built= false;
}

// ***************************************************************************
CAnimationSet::~CAnimationSet ()
{
	// Erase all animations.
	for (uint a=0; a<_Animation.size(); a++)
		delete _Animation[a];
	for (uint s=0; s<_SkeletonWeight.size(); s++)
		delete _SkeletonWeight[s];
}

// ***************************************************************************
uint CAnimationSet::getNumChannelId () const
{
	return (uint)_ChannelIdByName.size ();
}

// ***************************************************************************
uint CAnimationSet::addAnimation (const char* name, CAnimation* animation)
{
	// error to add an animation after a build() if the animation set is in HeaderCompress mode
	nlassert(! (_Built && _AnimHeaderOptimisation) );

	// if sampleDivisor, apply to the animation
	if(_SampleDivisor>1)
		animation->applySampleDivisor(_SampleDivisor);

	// compress CTrackSampledQuat header
	if(_AnimHeaderOptimisation)
		animation->applyTrackQuatHeaderCompression();

	// Add an animation
	_Animation.push_back (animation);
	_AnimationName.push_back (name);

	// Add an entry name / animation
	_AnimationIdByName.insert (std::map <std::string, uint32>::value_type (name, (uint32)_Animation.size()-1));

	// Return animation id
	return (uint)_Animation.size()-1;
}

// ***************************************************************************
uint CAnimationSet::addSkeletonWeight (const char* name, CSkeletonWeight* skeletonWeight)
{
	// Add an animation
	_SkeletonWeight.push_back (skeletonWeight);
	_SkeletonWeightName.push_back (name);

	// Add an entry name / animation
	_SkeletonWeightIdByName.insert (std::map <std::string, uint32>::value_type (name, (uint32)_SkeletonWeight.size()-1));

	// Return animation id
	return (uint)_SkeletonWeight.size()-1;
}

// ***************************************************************************
void CAnimationSet::reset ()
{
	_Animation.clear();
	_SkeletonWeight.clear();
	_ChannelName.clear();
	_AnimationName.clear();
	_SkeletonWeightName.clear();
	_ChannelIdByName.clear();
	_AnimationIdByName.clear();
	_SkeletonWeightIdByName.clear();
	_SSSShapes.clear();
}

// ***************************************************************************
void CAnimationSet::build ()
{
	// error to rebuild in if already done while _AnimHeaderOptimisation,
	// cause applyAnimHeaderCompression() won't work
	if(_Built && _AnimHeaderOptimisation)
		return;
	_Built= true;

	// Clear the channel map
	_ChannelName.clear();
	_ChannelIdByName.clear ();

	// Set of names
	std::set<std::string> channelNames;

	// For each animation in the set
	uint a;
	for (a=0; a<_Animation.size(); a++)
	{
		// Fill the set of channel names
		getAnimation (a)->getTrackNames (channelNames);
	}

	// Add this name in the map with there iD
	uint id=0;
	std::set<std::string>::iterator ite=channelNames.begin ();
	while (ite!=channelNames.end ())
	{
		// Insert an entry
		_ChannelIdByName.insert (std::map <std::string, uint32>::value_type (*ite, id++));

		// Next entry
		ite++;
	}

	// build ChannelName From Map
	buildChannelNameFromMap();

	// If the animation set is in HeaderOptim mode, reduce memory load by removing map<string, trackId>
	if(_AnimHeaderOptimisation)
	{
		for (uint a=0; a<_Animation.size(); a++)
		{
			_Animation[a]->applyAnimHeaderCompression (this, _ChannelIdByName);
		}
	}

	// Build the set of SSS Shapes from each animation
	for (a=0; a<_Animation.size(); a++)
	{
		const std::vector<std::string>	&shapes= _Animation[a]->getSSSShapes();
		for(uint s=0;s<shapes.size();s++)
		{
			// insert (may be already done)
			_SSSShapes.insert(shapes[s]);
		}
	}


	// TestYoyo
	/*nlinfo("ANIMYOYO: %d channels", _ChannelIdByName.size());
	std::map <std::string, uint32>::iterator	it;
	for(it= _ChannelIdByName.begin();it!=_ChannelIdByName.end();it++)
	{
		nlinfo("ANIMYOYO: %3d: %s", it->second, it->first.c_str());
	}*/
}

// ***************************************************************************
void CAnimationSet::serial (NLMISC::IStream& f)
{
	// serial not possible if header optimisation enabled
	nlassert(!_AnimHeaderOptimisation);

	// Serial an header
	f.serialCheck (NELID("_LEN"));
	f.serialCheck (NELID("MINA"));
	f.serialCheck (NELID("TES_"));

	// Serial a version
	uint	ver= f.serialVersion (1);

	// Serial the class
	f.serialContPtr (_Animation);
	f.serialContPtr (_SkeletonWeight);
	f.serialCont (_AnimationName);
	f.serialCont (_SkeletonWeightName);
	f.serialCont(_ChannelIdByName);
	f.serialCont(_AnimationIdByName);
	f.serialCont(_SkeletonWeightIdByName);
	if(ver>=1)
		f.serialCont(_ChannelName);
	else
		buildChannelNameFromMap();
}

// ***************************************************************************
bool CAnimationSet::loadFromFiles(const std::string &path, bool recurse /* = true*/, const char *ext /*= "anim"*/, bool wantWarningMessage /*= true*/)
{
	bool everythingOk = true;
	std::vector<std::string> anims;
	NLMISC::CPath::getPathContent(path, recurse, false, true, anims);
	for (uint k = 0; k < anims.size(); ++k)
	{
		std::string fileExt = NLMISC::CFile::getExtension(anims[k]);
		if (fileExt == ext) // an animation file ?
		{
			try
			{
				NLMISC::CIFile	iFile;
				iFile.open(anims[k]);
				CUniquePtr<CAnimation> anim(new CAnimation);
				anim->serial(iFile);
				addAnimation(NLMISC::CFile::getFilenameWithoutExtension(anims[k]).c_str(), anim.release());
				iFile.close();

			}
			catch (const NLMISC::EStream &e)
			{
				if (wantWarningMessage)
				{
					nlinfo("Unable to load an automatic animation : %s", e.what());
				}
				everythingOk = false;
			}
		}
	}
	build();
	return everythingOk;
}

// ***************************************************************************
void CAnimationSet::setAnimationSampleDivisor(uint sampleDivisor)
{
	_SampleDivisor= sampleDivisor;
	// 0 is invalid
	if(_SampleDivisor==0)
		_SampleDivisor= 1;
}

// ***************************************************************************
uint CAnimationSet::getAnimationSampleDivisor() const
{
	return _SampleDivisor;
}

// ***************************************************************************
void	CAnimationSet::buildChannelNameFromMap()
{
	contReset(_ChannelName);
	_ChannelName.resize(_ChannelIdByName.size());
	std::map <std::string, uint32>::iterator	it;
	for(it= _ChannelIdByName.begin();it!=_ChannelIdByName.end();it++)
	{
		_ChannelName[it->second]= it->first;
	}
}

// ***************************************************************************
void	CAnimationSet::preloadSSSShapes(IDriver &drv, CShapeBank &shapeBank)
{
	const	std::string		shapeCacheName= "SSS_PreLoad";

	// Create the Animation Set Shape cache if do not exist
	if(!shapeBank.isShapeCache(shapeCacheName))
	{
		// allow "inifinite" number of preloaded shapes
		shapeBank.addShapeCache(shapeCacheName);
		shapeBank.setShapeCacheSize(shapeCacheName, 1000000);
	}

	// For all files
	std::set<std::string>::iterator		it;
	for(it=_SSSShapes.begin();it!=_SSSShapes.end();it++)
	{
		string	fileName= toLower(*it);

		// link the shape to the shapeCache
		shapeBank.linkShapeToShapeCache(fileName, shapeCacheName);

		// If !present in the shapeBank
		if( shapeBank.getPresentState(fileName)==CShapeBank::NotPresent )
		{
			// Don't load it if no more space in the cache
			if( shapeBank.getShapeCacheFreeSpace(shapeCacheName)>0 )
			{
				// load it.
				shapeBank.load(fileName);

				// If success
				if( shapeBank.getPresentState(fileName)==CShapeBank::Present )
				{
					// When a shape is first added to the bank, it is not in the cache.
					// add it and release it to force it to be in the cache.
					IShape	*shp= shapeBank.addRef(fileName);
					if(shp)
					{
						//nlinfo("Loading %s", CPath::lookup(fileName.c_str(), false, false).c_str());
						shp->flushTextures(drv, 0);
						shapeBank.release(shp);
					}
				}
			}
		}
	}
}


} // NL3D
