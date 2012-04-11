/**
 * \file stream_file_source.cpp
 * \brief CStreamFileSource
 * \date 2012-04-11 09:57GMT
 * \author Jan Boon (Kaetemi)
 * CStreamFileSource
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
#include <nel/sound/stream_file_source.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include <nel/sound/audio_mixer_user.h>
#include <nel/sound/audio_decoder.h>

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

CStreamFileSource::CStreamFileSource(CStreamFileSound *streamFileSound, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster, CGroupController *groupController)
: CStreamSource(streamFileSound, spawn, cb, cbUserParam, cluster, groupController), m_AudioDecoder(NULL), m_Paused(false)
{
	m_Thread = NLMISC::IThread::create(this);
}

CStreamFileSource::~CStreamFileSource()
{
	stop();
	m_Thread->wait(); // thread must have stopped for delete!
	delete m_Thread;
	m_Thread = NULL;
	delete m_AudioDecoder;
	m_AudioDecoder = NULL;
}

void CStreamFileSource::play()
{
	// note: CStreamSource will assert crash if already physically playing!


	if (m_Thread->isRunning() && m_WaitingForPlay)
	{
		nldebug("play waiting %s", getStreamFileSound()->getFilePath().c_str());
		if (m_NextBuffer || !m_FreeBuffers)
		{
			CStreamSource::play();
		}
		else
		{
			m_WaitingForPlay = true;
			CAudioMixerUser *mixer = CAudioMixerUser::instance();
			mixer->addSourceWaitingForPlay(this);
		}
	}
	else if (!_Playing)
	{
		nldebug("play waiting %s", getStreamFileSound()->getFilePath().c_str());
		if (!m_WaitingForPlay)
		{
			// thread may be stopping from stop call
			m_Thread->wait();
		}
		else
		{
			nlwarning("Already waiting for play");
		}
		if (!getStreamFileSound()->getAsync())
			prepareDecoder();
		// else load audiodecoder in thread
		m_WaitingForPlay = true;
		m_Thread->start();
		m_Thread->setPriority(NLMISC::ThreadPriorityHighest);
		if (!getStreamFileSound()->getAsync())
		{
			// wait until at least one buffer is ready
			while (!(m_NextBuffer || !m_FreeBuffers) && m_WaitingForPlay)
				NLMISC::nlSleep(10);
			CStreamSource::play();
			if (!_Playing)
			{
				nlwarning("Failed to synchronously start playing a file stream source. This happens when all physical tracks are in use. Use a Highest Priority sound");
			}
		}
		else
		{
			CAudioMixerUser *mixer = CAudioMixerUser::instance();
			mixer->addSourceWaitingForPlay(this);
		}
	}
	else
	{
		nlwarning("Already playing");
	}

	
	/*if (!m_WaitingForPlay)
	{
		m_WaitingForPlay = true;

		m_Thread->wait(); // thread must have stopped to restart it!

		m_Thread->start();
		m_Thread->setPriority(NLMISC::ThreadPriorityHighest);
	}

	CStreamSource::play();*/
}

void CStreamFileSource::stop()
{
	nldebug("stop %s", getStreamFileSound()->getFilePath().c_str());

	CStreamSource::stop();

	// thread will check _Playing to stop
}

bool CStreamFileSource::isPlaying()
{
	nldebug("isPlaying");

	return m_Thread->isRunning();
}

void CStreamFileSource::pause()
{
	nldebug("pause");

	if (!m_Paused)
	{
		// thread checks for this to not delete the audio decoder
		m_Paused = true;
		
		// stop the underlying system
		CStreamSource::stop();

		// thread will check _Playing to stop
	}
	else
	{
		nlwarning("Already paused");
	}
}

void CStreamFileSource::resume()
{
	nldebug("resume");

	if (m_Paused)
	{
		m_Thread->wait(); // thread must have stopped to restart it!
		
		play();
	}
	else
	{
		nlwarning("Not paused");
	}
}

bool CStreamFileSource::isEnded()
{
	return (!m_Thread->isRunning() && !_Playing && !m_WaitingForPlay && !m_Paused);
}

float CStreamFileSource::getLength()
{
	return m_AudioDecoder->getLength();
}

bool CStreamFileSource::isLoadingAsync()
{
	return m_WaitingForPlay;
}

void CStreamFileSource::prepareDecoder()
{
	// creates a new decoder or keeps going with the current decoder if the stream was paused

	if (m_Paused)
	{
		// handle paused!
		m_Paused = false;
	}
	else if (m_AudioDecoder) // audio decoder should normally not exist when not paused and starting the thread
	{
		nlwarning("CAudioDecoder already exists, possible thread race bug with pause");			
		delete m_AudioDecoder;
		m_AudioDecoder = NULL;
	}
	if (!m_AudioDecoder)
	{
		// load the file
		m_AudioDecoder = IAudioDecoder::createAudioDecoder(getStreamFileSound()->getFilePath(), getStreamFileSound()->getAsync(), getStreamFileSound()->getLooping());
		if (!m_AudioDecoder)
		{
			nlwarning("Failed to create IAudioDecoder, likely invalid format");
			return;
		}
		this->setFormat(m_AudioDecoder->getChannels(), m_AudioDecoder->getBitsPerSample(), (uint32)m_AudioDecoder->getSamplesPerSec());
	}
	uint samples, bytes;
	this->getRecommendedBufferSize(samples, bytes);
	this->preAllocate(bytes * 2);
}

void CStreamFileSource::bufferMore(uint bytes) // buffer from bytes (minimum) to bytes * 2 (maximum)
{
	uint8 *buffer = this->lock(bytes * 2);
	if (buffer)
	{
		uint32 result = m_AudioDecoder->getNextBytes(buffer, bytes, bytes * 2);
		this->unlock(result);
	}
}

void CStreamFileSource::run()
{
	nldebug("run");

	bool looping = _Looping;
	if (getStreamFileSound()->getAsync())
		prepareDecoder();
	uint samples, bytes;
	this->getRecommendedBufferSize(samples, bytes);
	bufferMore(bytes);
	while (_Playing || m_WaitingForPlay)
	{
		if (!m_AudioDecoder->isMusicEnded())
		{
			bool newLooping = _Looping;
			if (looping != newLooping)
			{
				m_AudioDecoder->setLooping(looping);
				looping = newLooping;
			}
			
			bufferMore(bytes);
			NLMISC::nlSleep(this->getRecommendedSleepTime());
		}
		else
		{
			// wait until done playing buffers
			while (this->hasFilledBuffersAvailable())
				NLMISC::nlSleep(40);
			// stop the physical source
			// if (hasPhysicalSource())
			// 	getPhysicalSource()->stop();
			// the audio mixer will call stop on the logical source
			break;
		}
	}
	if (m_Paused)
	{
		// don't delete anything
	}
	else
	{
		delete m_AudioDecoder;
		m_AudioDecoder = NULL;
	}
}

} /* namespace NLSOUND */

/* end of file */
