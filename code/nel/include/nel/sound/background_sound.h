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

#ifndef NL_BACKGROUND_SOUND_H
#define NL_BACKGROUND_SOUND_H

#include "nel/misc/string_mapper.h"
#include "nel/sound/sound.h"
#include "nel/sound/u_audio_mixer.h"

namespace NLSOUND {

//class ISoundDriver;
//class IBuffer;
//class CSound;



/**
 * A background sound static properties
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2002
 */
class CBackgroundSound : public CSound
{
public:
	/// Constructor
	CBackgroundSound();
	/// Destructor
	~CBackgroundSound();

	TSOUND_TYPE getSoundType()							{ return SOUND_BACKGROUND;}

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Return true if cone is meaningful
	bool				isDetailed() const				{ return false;}
	/// Return the length of the sound in ms
	virtual uint32		getDuration();
	/// Return the name (must be unique)

	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const;

	virtual float		getMaxDistance() const;

	void				serial(NLMISC::IStream &s);

	/// Associtation clas for storage of sound / filter.
	struct TSoundInfo
	{
		NLMISC::CSheetId		SoundName;
		UAudioMixer::TBackgroundFlags		Filter;

		void serial(NLMISC::IStream &s)
		{
			SoundName.serialString(s, "sound");
			s.serial(Filter);
		}
	};

	const std::vector<TSoundInfo> &getSounds() const			{ return _Sounds;}

private:


	/// The list of sound composing the background sound with there repective filter.
	std::vector<TSoundInfo>		_Sounds;


	// Duration of sound.
	uint32						_Duration;
	// flag for validity of duration (after first evaluation).
	bool						_DurationValid;
};

} // NLSOUND

#endif // NL_BACKGROUND_SOUND_H

