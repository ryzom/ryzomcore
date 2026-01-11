// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2010  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#ifndef NLSOUND_STREAM_SOUND_H
#define NLSOUND_STREAM_SOUND_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "nel/sound/sound.h"

namespace NLSOUND {

/**
 * CStreamSound
 * \brief CStreamSound
 * \date 2010-01-28 07:29GMT
 * \author Jan Boon (Kaetemi)
 */
class CStreamSound : public CSound
{
public:
	CStreamSound();
	virtual ~CStreamSound();

	/// Get the type of the sound.
	virtual TSOUND_TYPE getSoundType()						{ return SOUND_STREAM; }

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &/* subsounds */) const { }

	/// Serialize the sound data.
	virtual void		serial(NLMISC::IStream &s);

	/// Return the length of the sound in ms
	virtual uint32		getDuration()						{ return 0; }

	/// Return the alpha attenuation value.
	float				getAlpha() const					{ return m_Alpha; }

private:
	CStreamSound(const CStreamSound &);
	CStreamSound &operator=(const CStreamSound &);

private:
	float				m_Alpha;

}; /* class CStreamSound */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_STREAM_SOUND_H */

/* end of file */
