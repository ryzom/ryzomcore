// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/common.h"
#include "nel/misc/string_mapper.h"
#include "nel/sound/sound_anim_marker.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_source.h"

using namespace std;
using namespace NLSOUND;
using namespace NLMISC;

namespace NLSOUND {

// ********************************************************

CSoundAnimMarker::~CSoundAnimMarker()
{
}

// ********************************************************

void CSoundAnimMarker::play(UAudioMixer* mixer, NL3D::CCluster *cluster, CSoundContext &context)
{
	TMarkerSoundSet::iterator first(_Sounds.begin()), last(_Sounds.end());

	for (; first != last; ++first)
	{
		USource* source = mixer->createSource((*first), true, NULL, NULL, cluster, &context);
		if (source != NULL)
		{
			source->setRelativeGain(context.RelativeGain);
			source->setPos(context.Position);
			source->play();
		}
	}
}

// ********************************************************

void CSoundAnimMarker::addSound(const NLMISC::TStringId& soundName)
{
	pair<TMarkerSoundSet::iterator, bool> inserted;
	inserted = _Sounds.insert(soundName);
	if (inserted.second == false)
	{
		nlwarning("Duplicate sound (%s)", CStringMapper::unmap(soundName).c_str());
	}
}

// ********************************************************

void CSoundAnimMarker::removeSound(const NLMISC::TStringId &soundName)
{
	TMarkerSoundSet::iterator iter = _Sounds.find(soundName);
    if (iter != _Sounds.end())
	{
		_Sounds.erase(iter);
	}
	else
	{
		nlwarning("No sound was removed (%s)", CStringMapper::unmap(soundName).c_str());
	}
}

// ********************************************************

void CSoundAnimMarker::getSounds(vector<NLMISC::TStringId> &sounds)
{
	sounds.insert(sounds.end(), _Sounds.begin(), _Sounds.end());

/*	TMarkerSoundSet::iterator first(_Sounds.begin()), last(_Sounds.end());
	for (; first != last; ++first)
	{
		sounds.push_back((*first).c_str());
	}
*/
}



} // namespace NLSOUND
