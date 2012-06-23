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

#ifndef NL_SOUND_ANIM_MARKER_H
#define NL_SOUND_ANIM_MARKER_H

#include "nel/misc/string_mapper.h"
#include "nel/3d/cluster.h"
#include "nel/sound/u_source.h"
#include "nel/misc/sheet_id.h"


namespace NLMISC
{
	class IStream;
}


namespace NLSOUND {


typedef std::set<NLMISC::CSheetId> TMarkerSoundSet;

class UAudioMixer;


class CSoundAnimMarker
{
public:

	CSoundAnimMarker(float time = 0.0f) : _Time(time) {}
	virtual ~CSoundAnimMarker();

	/** Set the time of this marker */
	virtual void			setTime(float time)		{ _Time = time; }

	/** Returns the time of this marker */
	virtual float			getTime()	const			{ return _Time; }

	/** Add a new sound in the set of to-be-played sounds for this marker */
	virtual void			addSound(const NLMISC::CSheetId &soundName);

	/** Remove a sound */
	virtual void			removeSound(const NLMISC::CSheetId &soundName);

	/** Return the set of sounds of this marker */
	virtual void			getSounds(std::vector<NLMISC::CSheetId> &sounds);

	/** Play all the sounds of this marker */
	virtual void			play(UAudioMixer* mixer, NL3D::CCluster *cluster, CSoundContext &context);


protected:

	/** The set of sounds to be played */
	TMarkerSoundSet			_Sounds;

	/** The time position of this marker */
	float					_Time;
};

} // namespace NLSOUND

#endif // NL_SOUND_ANIM_MARKER_H
