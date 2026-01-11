/**
 * \file stream_file_sound.h
 * \brief CStreamFileSound
 * \date 2012-04-11 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CStreamFileSound
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLSOUND_STREAM_FILE_SOUND_H
#define NLSOUND_STREAM_FILE_SOUND_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include <nel/sound/stream_sound.h>

namespace NLSOUND {
	class CSourceMusicChannel;

/**
 * \brief CStreamFileSound
 * \date 2012-04-11 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CStreamFileSound
 */
class CStreamFileSound : public CStreamSound
{
public:
	friend class CSourceMusicChannel;

public:
	CStreamFileSound();
	virtual ~CStreamFileSound();

	/// Get the type of the sound.
	virtual TSOUND_TYPE getSoundType()						{ return SOUND_STREAM_FILE; }

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &/* subsounds */) const { }

	/// Serialize the sound data.
	virtual void		serial(NLMISC::IStream &s);

	/// Return the length of the sound in ms
	virtual uint32		getDuration()						{ return 0; }

	inline bool			getAsync()							{ return m_Async; }

	inline const std::string &getFilePath()					{ return m_FilePath; }

private:
	/// Used by CSourceMusicChannel to set the filePath and default settings on other parameters.
	void setMusicFilePath(const std::string &filePath, bool async = true, bool loop = false);

private:
	CStreamFileSound(const CStreamFileSound &);
	CStreamFileSound &operator=(const CStreamFileSound &);

private:
	bool				m_Async;
	std::string			m_FilePath;

}; /* class CStreamFileSound */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_STREAM_FILE_SOUND_H */

/* end of file */
