/**
 * \file stream_file_source.h
 * \brief CStreamFileSource
 * \date 2012-04-11 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CStreamFileSource
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2012-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLSOUND_STREAM_FILE_SOURCE_H
#define NLSOUND_STREAM_FILE_SOURCE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/thread.h>

// Project includes
#include <nel/sound/stream_source.h>
#include <nel/sound/stream_file_sound.h>

namespace NLSOUND {
	class IAudioDecoder;

/**
 * \brief CStreamFileSource
 * \date 2012-04-11 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CStreamFileSource
 */
class CStreamFileSource : public CStreamSource, private NLMISC::IRunnable
{
public:
	CStreamFileSource(CStreamFileSound *streamFileSound = NULL, bool spawn = false, TSpawnEndCallback cb = 0, void *cbUserParam = 0, NL3D::CCluster *cluster = 0, CGroupController *groupController = NULL);
	virtual ~CStreamFileSource();

	/// Return the source type
	TSOURCE_TYPE					getType() const								{ return SOURCE_STREAM_FILE; }

	/// \name Playback control
	//@{
	/// Play
	virtual void					play();
	/// Stop playing
	virtual void					stop();
	/// Get playing state. Return false even if the source has stopped on its own.
	virtual bool					isPlaying();
	/// Pause (following legacy music channel implementation)
	void pause();
	/// Resume (following legacy music channel implementation)
	void resume();
	/// check if song ended (following legacy music channel implementation)
	bool isEnded();
	/// (following legacy music channel implementation)
	float getLength();
	/// check if still loading (following legacy music channel implementation)
	bool isLoadingAsync();
	//@}

	/// \name Decoding thread
	//@{
	virtual void getName (std::string &result) const { result = "CStreamFileSource"; }
	virtual void run();
	//@}

	// TODO: getTime

private:
	bool prepareDecoder();
	inline bool bufferMore(uint bytes);

private:
	CStreamFileSource(const CStreamFileSource &);
	CStreamFileSource &operator=(const CStreamFileSource &);

private:
	inline CStreamFileSound *getStreamFileSound() { return static_cast<CStreamFileSound *>(m_StreamSound); }

	NLMISC::IThread *m_Thread;

	IAudioDecoder *m_AudioDecoder;

	std::string m_LookupPath;

	bool m_Paused;
	bool m_DecodingEnded;

}; /* class CStreamFileSource */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_STREAM_FILE_SOURCE_H */

/* end of file */
