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

#include "stdsound.h"
#include "nel/sound/sound_anim_manager.h"
#include "nel/sound/sound_animation.h"
//#include "nel/sound/sound_anim_player.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLSOUND;
using namespace NLMISC;

namespace NLSOUND {

CSoundAnimManager*	CSoundAnimManager::_Instance = 0;

// ********************************************************

CSoundAnimManager::CSoundAnimManager(NLSOUND::UAudioMixer* mixer) : _Mixer(mixer)
{
	if (_Instance != 0)
	{
		throw Exception("Duplicate instanciation of CSoundAnimManager singleton");
	}

	_Instance = this;
	//_PlayerId = 0;
}

// ********************************************************

CSoundAnimManager::~CSoundAnimManager()
{
	if (_Instance == NULL)
		return;
	/*
	set<CSoundAnimPlayer*>::iterator iter;

	for (iter = _Players.begin(); iter != _Players.end(); iter++)
	{
		CSoundAnimPlayer* player = *iter;
		delete player;
	}

	_Players.clear();
	*/

	_Instance = NULL;
}

// ********************************************************

TSoundAnimId CSoundAnimManager::loadAnimation(std::string& name)
{
	nlassert(!name.empty());

	string filename;
	if (name.find(".anim") != name.npos)
	{
		filename = CFile::getFilenameWithoutExtension(name);
		filename.append(".sound_anim");
	}
	else
	{
		filename = name;
		filename.append(".sound_anim");
	}

	// throws exception if file not found
	filename = CPath::lookup(filename, false, false, true);
	if (filename.empty())
	{
		return CSoundAnimationNoId;
	}

	TSoundAnimId id = createAnimation(name);
	if (id == CSoundAnimationNoId)
	{
		return CSoundAnimationNoId;
	}

	CSoundAnimation* anim = _Animations[id];

	if (anim == NULL)
		return CSoundAnimationNoId;

	anim->setFilename(filename);
	anim->load();

	return id;
}

// ********************************************************

TSoundAnimId CSoundAnimManager::createAnimation(std::string& name)
{
	nlassert(!name.empty());

	// create and insert animations
	TSoundAnimId id = (TSoundAnimId)_Animations.size();
	CSoundAnimation* anim = new CSoundAnimation(name, id);
	_Animations.push_back(anim);

	// insert the name and id in the id table
	pair<TSoundAnimMap::iterator, bool> inserted;
	inserted =_IdMap.insert(make_pair(anim->getName().c_str(), id));
	if (!inserted.second)
	{
		nlwarning("Duplicate sound animation \"%s\"", name.c_str());
		delete anim;
		return CSoundAnimationNoId;
	}

	return id;
}

// ********************************************************

CSoundAnimation* CSoundAnimManager::findAnimation(std::string& name)
{
	TSoundAnimMap::iterator iter = _IdMap.find(name.c_str());
	return (iter == _IdMap.end())? 0 : _Animations[(*iter).second];
}

// ********************************************************

TSoundAnimId CSoundAnimManager::getAnimationFromName(std::string& name)
{
	TSoundAnimMap::iterator iter = _IdMap.find(name.c_str());
	return (iter == _IdMap.end())? CSoundAnimationNoId : (*iter).second;
}

// ********************************************************

void CSoundAnimManager::saveAnimation(CSoundAnimation* anim, std::string& filename)
{
	nlassert(anim);
	nlassert(!filename.empty());

	anim->setFilename(filename);
	anim->save ();
}

// ********************************************************
/*
TSoundAnimPlayId CSoundAnimManager::playAnimation(TSoundAnimId id, float time, CVector* position)
{
	nlassert(id != CSoundAnimationNoId);
	nlassert((uint32) id < _Animations.size());
	nlassert(position);

	CSoundAnimation* anim = _Animations[id];
	nlassert(anim);


	_PlayerId++;
	CSoundAnimPlayer* player = new CSoundAnimPlayer(anim, time, position, _Mixer, _PlayerId);
	nlassert(player);

	_Players.insert(player);

	return _PlayerId;
}
*/

struct TFindId : std::unary_function<TSoundAnimMap::value_type, bool>
{
	TSoundAnimId	Id;

	TFindId(TSoundAnimId id)
		: Id(id)
	{}

	bool operator () (const TSoundAnimMap::value_type &value) const
	{
		return value.second == Id;
	}
};

std::string	CSoundAnimManager::idToName(TSoundAnimId id)
{
	static string empty;
	TSoundAnimMap::iterator it(std::find_if(_IdMap.begin(), _IdMap.end(), TFindId(id)));

	if (it != _IdMap.end())
		return it->first;
	else
		return empty;
}


void CSoundAnimManager::playAnimation(TSoundAnimId id, float lastTime, float curTime, NL3D::CCluster *cluster, CSoundContext &context)
{
	//nlassert(id != CSoundAnimationNoId);
	if (id == CSoundAnimationNoId)
	{
		return;
	}

	if ((uint32) id >= _Animations.size())
		return;
	nlassert((uint32) id < _Animations.size());

	CSoundAnimation* anim = _Animations[id];
	nlassert(anim);

	anim->play(_Mixer, lastTime, curTime, cluster, context);
}


// ********************************************************
TSoundAnimPlayId		CSoundAnimManager::playAnimation(TSoundAnimId /* id */, float /* time */, NL3D::CCluster * /* cluster */, CSoundContext &/* context */)
{
	return 0;
}

// ********************************************************
TSoundAnimPlayId CSoundAnimManager::playAnimation(string& /* name */, float /* time */, NL3D::CCluster * /* cluster */, CSoundContext &/* context */)
{
/*	nlassert(position);

	TSoundAnimId id = getAnimationFromName(name);
	return (id == CSoundAnimation::NoId)? -1 : _PlayerId;*/
	return 0;
}


// ********************************************************
void CSoundAnimManager::stopAnimation(TSoundAnimPlayId /* playbackId */)
{
/*	nlassert(playbackId >= 0);

	set<CSoundAnimPlayer*>::iterator iter;

	for (iter = _Players.begin(); iter != _Players.end(); )
	{
		CSoundAnimPlayer* player = *iter;
		if (player->getId() == playbackId)
		{
			_Players.erase(iter);
			break;
		}
	}*/
}

// ********************************************************
bool CSoundAnimManager::isPlaying(TSoundAnimPlayId /* playbackId */)
{
	/*nlassert(playbackId >= 0);

	set<CSoundAnimPlayer*>::iterator iter;

	for (iter = _Players.begin(); iter != _Players.end(); )
	{
		CSoundAnimPlayer* player = *iter;
		if (player->getId() == playbackId)
		{
			return player->isPlaying();
		}
	}*/

	return false;
}

// ********************************************************
void CSoundAnimManager::update(float /* lastTime */, float /* curTime */)
{
/*	set<CSoundAnimPlayer*>::iterator iter;

	_Garbage.clear();

	for (iter = _Players.begin(); iter != _Players.end(); iter++)
	{
		CSoundAnimPlayer* player = *iter;
		if (player->isPlaying())
		{
			player->update(lastTime, curTime);
		}
		else
		{
			_Garbage.push_back(player);
		}
	}

	vector<CSoundAnimPlayer*>::iterator iter2;

	for (iter2 = _Garbage.begin(); iter2 != _Garbage.end(); iter2++)
	{
		iter = _Players.find(*iter2);
		if (iter != _Players.end())
		{
			_Players.erase(iter);
		}
	}*/
}


} // namespace NLSOUND


