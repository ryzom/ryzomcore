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

#ifndef NL_MUSIC_SOUND_H
#define NL_MUSIC_SOUND_H

#include "nel/misc/string_mapper.h"
#include "nel/sound/sound.h"


namespace NLSOUND {


// ***************************************************************************
/**
 * A sound describing a streamable music to play
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CMusicSound : public CSound
{
public:

	/// Constructor
	CMusicSound();
	/// Destructor
	virtual ~CMusicSound();

	/// \name From CSound
	//@{
	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);
	virtual TSOUND_TYPE getSoundType() {return SOUND_MUSIC;}
	virtual uint32		getDuration();
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const;
	virtual void		serial(NLMISC::IStream &s);
	virtual float		getMaxDistance() const;
	virtual bool		isDetailed() const;
	//@}

	/// Return the filename
	NLMISC::TStringId	getFileName() const					{ return _FileName; }
	sint32				getFadeInLength() const {return _FadeInLength;}
	sint32				getFadeOutLength() const {return _FadeOutLength;}
	sint32				getMinimumPlayTime() const {return _MinimumPlayTime;}
	sint32				getTimeBeforeCanReplay() const {return _TimeBeforeCanReplay;}

public:

	// For CMusicSoundManager. Mark the last time (in ms) this music was stopped, after a play. INT_MIN by default
	NLMISC::TTime		LastStopTime;

private:
	// Music FileName
	NLMISC::TStringId	_FileName;
	// time in ms
	sint32			_FadeInLength;
	sint32			_FadeOutLength;
	sint32			_MinimumPlayTime;
	sint32			_TimeBeforeCanReplay;
};


} // NLSOUND


#endif // NL_MUSIC_SOUND_H

/* End of music_sound.h */
