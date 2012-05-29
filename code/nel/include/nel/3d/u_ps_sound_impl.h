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

#ifndef NL_PS_SOUND_IMPL_H
#define NL_PS_SOUND_IMPL_H

#include "nel/misc/types_nl.h"
#include <string>
#include "nel/misc/debug.h"
#include "nel/misc/rgba.h"
#include "nel/sound/u_audio_mixer.h"
#include "u_particle_system_sound.h"


// WARNING : this file is not intended to be directly included by the client.
// It is just used to avoid a dependencie between NL3D and NLSOUND


namespace NL3D
{


inline void SpawnedSourceEndedCallback(NLSOUND::USource *source, void *userParam);


class CPSSoundServImpl;


/// This class implement a sound instance (a sound source)
class CPSSoundInstanceImpl : public UPSSoundInstance
{
public:
	/// construct this object from a nel sound source
	/** The system will call this method to set the parameters of the sound
	  */
	CPSSoundInstanceImpl()
		: _Source(NULL), _Spawned(false), _SoundServImpl(NULL)
	{
	}

	/// init this sound instance parameters
	void init(NLSOUND::USource *source, CPSSoundServImpl *soundServImp, bool spawned)
	{
		nlassert(source);
		_Source = source;
		_Spawned    = spawned;
		_SoundServImpl = soundServImp;
	}

	/// change this sound source parameters
	virtual void setSoundParams(float gain,
								const NLMISC::CVector &pos,
								const NLMISC::CVector &velocity,
								float pitch
							  )
	{
		if (!_Source) return;
		if (gain < 0) gain = 0;
		if (gain > 1) gain = 1;
		if (pitch < 0.0001f) pitch = 0.0001f;
		_Source->setPos(pos);
		_Source->setVelocity(velocity);
		_Source->setRelativeGain(gain);
		_Source->setPitch(pitch);
	}

	/// start to play the sound
	virtual void play(void)
	{
		if (!_Source) return;
		_Source->play();
	}


	virtual bool isPlaying(void) const
	{
		if (!_Source) return false;
		return _Source->isPlaying();
	}

	/// stop the sound
	virtual void stop(void)
	{
		if (!_Source) return;
		_Source->stop();
	}

	// get pitch
	virtual float getPitch() const
	{
		if (!_Source) return 0.f;
		return _Source->getPitch();
	}

	// set sound looping
	virtual void setLooping(bool looping)
	{
		if (_Source) _Source->setLooping(looping);
	}

	virtual bool isLooping() const
	{
			return _Source ? _Source->getLooping() : false;
	}


	/// release the sound source
	virtual void release(void);

protected:
	friend inline void SpawnedSourceEndedCallback(NLSOUND::USource *source, void *userParam);
	NLSOUND::USource *_Source;
	bool			 _Spawned;
	CPSSoundServImpl   *_SoundServImpl;
};







/**
 * This class implements PS sound server. It warps the calls to NEL sound. Everything is in a .h file to avoid dependency
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSSoundServImpl : public UPSSoundServer
{
public:
	/// construct this sound server; You must init it then
	CPSSoundServImpl() : _AudioMixer(NULL)
	{
	}

	virtual ~CPSSoundServImpl() {}

	/// init this particle system sound server, using the given audio mixer
	void init(NLSOUND::UAudioMixer *audioMixer)
	{
		_AudioMixer = audioMixer;
	}


	/// get the audio mixer associated with that server
	NLSOUND::UAudioMixer *getAudioMixer(void) { return _AudioMixer;}
	const NLSOUND::UAudioMixer *getAudioMixer(void) const { return _AudioMixer;}


	/// inherited from IPSSoundServer
	UPSSoundInstance *createSound(const NLMISC::CSheetId &soundName, bool spawned = true)
	{
		if (!_AudioMixer)
			return NULL;
		CPSSoundInstanceImpl *sound = new CPSSoundInstanceImpl;
		NLSOUND::USource *source = _AudioMixer->createSource(soundName, spawned, SpawnedSourceEndedCallback, sound );
		if (source)
		{
			/*
			if (spawned)
			{
				source->setLooping(false);
			}
			*/
			sound->init(source, this, spawned);
			return sound;
		}
		else
		{
			// should usually not happen
			delete sound;
			return NULL;
		}
	}

protected:

	NLSOUND::UAudioMixer  *_AudioMixer;

};


/// this callback is called when a spawned source has ended, so that we know that the pointer to it is invalid...
inline void SpawnedSourceEndedCallback(NLSOUND::USource *source, void *userParam)
{
	nlassert(((CPSSoundInstanceImpl *) userParam)->_Source == source);
	((CPSSoundInstanceImpl *) userParam)->_Source = NULL;
}



} // NL3D


#endif // NL_PS_SOUND_IMPL_H

/* End of ps_sound_impl.h */
