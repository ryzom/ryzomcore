/**
 * \file source_music_channel.cpp
 * \brief CSourceMusicChannel
 * \date 2012-04-11 16:08GMT
 * \author Jan Boon (Kaetemi)
 * CSourceMusicChannel
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "stdsound.h"
#include <nel/sound/source_music_channel.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/sound/stream_file_source.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

CSourceMusicChannel::CSourceMusicChannel() : m_Source(NULL), m_Gain(1.0f)
{
	
}

CSourceMusicChannel::~CSourceMusicChannel()
{
	nlassert(!m_Source);
	delete m_Source;
	m_Source = NULL;
}

bool CSourceMusicChannel::play(const std::string &filepath, bool async, bool loop)
{
	// delete previous source if any
	// note that this waits for the source's thread to finish if the source was still playing
	if (m_Source)
		delete m_Source;

	m_Sound.setMusicFilePath(filepath, async, loop);

	m_Source = new CStreamFileSource(&m_Sound, false, NULL, NULL, NULL, NULL);
	m_Source->setSourceRelativeMode(true);
	m_Source->setPos(NLMISC::CVector::Null);
	m_Source->setRelativeGain(m_Gain);

	m_Source->play();

	return m_Source->isPlaying();
}

void CSourceMusicChannel::stop()
{
	// stop but don't delete the source, deleting source may cause waiting for thread
	if (m_Source)
		m_Source->stop();
}

void CSourceMusicChannel::reset()
{
	// forces the source to be deleted, happens when audio mixer is reset
	delete m_Source;
	m_Source = NULL;
}

void CSourceMusicChannel::pause()
{
	if (m_Source)
		m_Source->pause();
}

void CSourceMusicChannel::resume()
{
	if (m_Source)
		m_Source->resume();
}

bool CSourceMusicChannel::isEnded()
{
	if (m_Source)
	{
		if (m_Source->isEnded())
		{
			// we can delete the source now without worrying about thread wait
			delete m_Source;
			m_Source = NULL;
			return true;
		}
		return false;
	}
	return true;
}

bool CSourceMusicChannel::isLoadingAsync()
{
	if (m_Source)
		return m_Source->isLoadingAsync();
	return false;
}

float CSourceMusicChannel::getLength()
{
	if (m_Source)
		return m_Source->getLength();
	return 0.0f;
}

void CSourceMusicChannel::setVolume(float gain)
{
	m_Gain = gain;
	if (m_Source)
		m_Source->setRelativeGain(gain);
}

} /* namespace NLSOUND */

/* end of file */
