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

#include "std3d.h"
#include "nel/misc/string_mapper.h"
#include "nel/3d/ps_sound.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/u_ps_sound_interface.h"
#include "nel/3d/ps_attrib_maker.h"

namespace NL3D
{


// we batch computation of Gains and frequencies. Here is the buffer size:
static const uint SoundBufSize = 1024;


// ***************************************************************************************************
CPSSound::CPSSound() : _Gain(1.f),
					   _GainScheme(NULL),
					   _Pitch(1.f),
					   _PitchScheme(NULL),
					   _EmissionPercent(1),
					   _SpawnSounds(false),
					   _Mute(false),
					   _SoundStopped(false),
					   _SoundReactivated(false),
					   _UseOriginalPitch(false)
{
	NL_PS_FUNC(CPSSound_CPSSound)
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("sound");
	_SoundName = NLMISC::CSheetId::Unknown /*NLMISC::CStringMapper::emptyId()*/;
}

// ***************************************************************************************************
void	CPSSound::stopSound()
{
	NL_PS_FUNC(CPSSound_stopSound)
	_SoundReactivated = false;
	if (_SoundStopped) return;
	CPSAttrib<UPSSoundInstance *>::iterator it = _Sounds.begin()
												, endIt = _Sounds.end();
	while (it != endIt)
	{
		if (*it)
		{
			(*it)->setLooping(false);
			(*it)->release();
			(*it) = NULL;
		}
		++it;
	}
	_SoundStopped = true;
}

// ***************************************************************************************************
void	CPSSound::reactivateSound()
{
	NL_PS_FUNC(CPSSound_reactivateSound)
	//if (!_SoundStopped) return;
	_SoundReactivated  = true;
}

// ***************************************************************************************************
void CPSSound::removeAllSources(void)
{
	NL_PS_FUNC(CPSSound_removeAllSources)
	const sint32 size = _Sounds.getSize();
	// delete all sounds, and rebuild them all
	for (sint32 k = size - 1; k >= 0; --k)
	{
		deleteElement(k);
	}
}

// ***************************************************************************************************
CPSSound::~CPSSound()
{
	NL_PS_FUNC(CPSSound_CPSSound)
	removeAllSources();
	delete _GainScheme;
	delete _PitchScheme;
}

// ***************************************************************************************************
uint32			CPSSound::getType(void) const
{
	NL_PS_FUNC(CPSSound_getType)
	return PSSound;
}



// ***************************************************************************************************
void			CPSSound::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSSound_step)
	if (pass != PSMotion) return;
	const uint32 size = _Owner->getSize();
	if (!size) return;
	if (_SoundStopped && !_SoundReactivated)
	{
		return;
	}
	if (_SoundReactivated)
	{
		_SoundStopped = false;
		_SoundReactivated = false;
		if (!_Mute)
		{
			sint32 k;
			// delete all sounds, and rebuild them all
			removeAllSources();
			for (k = 0; k < (sint32) size; ++k)
			{
				CPSEmitterInfo ei;
				ei.setDefaults();
				newElement(ei);
			}
		}
		// don't need to reupdate sound
		return;
	}
	nlassert(_Owner);
	uint32 toProcess, leftToDo = size;

	float   Gains[SoundBufSize];
	float   frequencies[SoundBufSize];

	uint	GainPtInc    = _GainScheme ? 1 : 0;
	uint	pitchPtInc = _PitchScheme ? 1 : 0;
	float   *currVol, *currPitch;


	CPSAttrib<UPSSoundInstance *>::iterator it = _Sounds.begin(),
												 endIt;
	CPSAttrib<NLMISC::CVector>::const_iterator posIt = _Owner->getPos().begin();
	CPSAttrib<NLMISC::CVector>::const_iterator speedIt = _Owner->getSpeed().begin();

	do
	{
		toProcess = leftToDo > SoundBufSize ? SoundBufSize : leftToDo;
		// compute Gain
		currVol = _GainScheme ? (float *) _GainScheme->make(getOwner(), size - leftToDo, Gains, sizeof(float), toProcess, true)
							  : &_Gain;
		if (!_UseOriginalPitch)
		{
			// compute pitch
			currPitch = _PitchScheme ? (float *) _PitchScheme->make(getOwner(), size - leftToDo, frequencies, sizeof(float), toProcess, true)
										 : &_Pitch;
			endIt = it + toProcess;
			const CMatrix &localToWorld = _Owner->getLocalToWorldMatrix();
			do
			{
				if (*it) // was this sound instanciated?
				{
					(*it)->setSoundParams(*currVol,
										  localToWorld * *posIt,
										  localToWorld.mulVector(*speedIt),
										  *currPitch);
					if ((*it)->isLooping() && !(*it)->isPlaying())
					{
						// looping sources do not play when they are clipped, so "reminds" the source to play when possible.
						(*it)->play();
					}
				}
				currVol += GainPtInc;
				currPitch += pitchPtInc;
				++posIt;
				++speedIt;
				++it;
			}
			while (it != endIt);
		}
		else
		{
			// keep original pitch
			endIt = it + toProcess;
			const CMatrix &localToWorld = _Owner->getLocalToWorldMatrix();
			do
			{
				if (*it) // was this sound instanciated?
				{
					(*it)->setSoundParams(*currVol,
						localToWorld * *posIt,
						localToWorld.mulVector(*speedIt),
						(*it)->getPitch());
				}
				currVol += GainPtInc;
				++posIt;
				++speedIt;
				++it;
			}
			while (it != endIt);
		}
		leftToDo -= toProcess;
	}
	while (leftToDo);
}

// ***************************************************************************************************
void	CPSSound::setGain(float Gain)
{
	NL_PS_FUNC(CPSSound_setGain)
	delete _GainScheme;
	_GainScheme = NULL;
	_Gain = Gain;
}

// ***************************************************************************************************
void	CPSSound::setGainScheme(CPSAttribMaker<float> *Gain)
{
	NL_PS_FUNC(CPSSound_setGainScheme)
	delete _GainScheme;
	_GainScheme = Gain;
	if (_Owner)
	{
		if (_GainScheme && _GainScheme->hasMemory()) _GainScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

// ***************************************************************************************************
void	CPSSound::setPitch(float pitch)
{
	NL_PS_FUNC(CPSSound_setPitch)
	delete _PitchScheme;
	_PitchScheme = NULL;
	_Pitch = pitch;
}

// ***************************************************************************************************
void	CPSSound::setPitchScheme(CPSAttribMaker<float> *pitch)
{
	NL_PS_FUNC(CPSSound_setPitchScheme)
	delete _PitchScheme;
	_PitchScheme = pitch;
	if (_Owner)
	{
		if (_PitchScheme && _PitchScheme->hasMemory()) _PitchScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

// ***************************************************************************************************
void			CPSSound::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSSound_serial)
	CPSLocatedBindable::serial(f);
	// version 3 : added option to keep original pitch from the .sound
	sint ver = f.serialVersion(3);

	// FIXME: CPSSound is reserialized from the _ParticleSystemProto
	// cache when a non-_Shared particle system is instanced, this 
	// causes unnecessary sheet id lookups from string.
	// SLN1: Serialize as uint32, but this requires the editor to know
	// the correct sheet id (and thus requires a built sheet_id.bin).
	// SLN2: Create a tool that reserializes all ps with sound sheet id 
	// instead of sheet names, based on a global flag, and serialize
	// a flag that specifies if the ps is serialized with id or name.
	_SoundName.serialString(f, "sound");

	sint32 nbSounds;
	bool hasScheme;
	if (f.isReading())
	{
		f.serial(nbSounds); // we are very unlikely to save a system with sounds being played in it,
							// but we need to keep datas coherency.
		if (_Owner)
		{
			_Sounds.resize(_Owner->getMaxSize());
		}
	}
	else
	{
		nbSounds = _Sounds.getSize(); // number of used sound
		f.serial(nbSounds);
	}


	if (f.isReading())
	{
		delete _GainScheme;
		_GainScheme = NULL;
		delete _PitchScheme;
		_PitchScheme = NULL;
	}
	// save Gain infos
	hasScheme = _GainScheme != NULL;
	f.serial(hasScheme);
	if (hasScheme)
	{
		f.serialPolyPtr(_GainScheme);
	}
	else
	{
		f.serial(_Gain);
	}
	// serial pitch infos
	if (ver >= 3)
	{
		bool useOriginalPitch = _UseOriginalPitch;
		f.serial(useOriginalPitch);
		_UseOriginalPitch = useOriginalPitch;
		if (!_UseOriginalPitch)
		{
			// serialize pitch infos (no needed otherwise)
			hasScheme = _PitchScheme != NULL;
			f.serial(hasScheme);
			if (hasScheme)
			{
				f.serialPolyPtr(_PitchScheme);
			}
			else
			{
				f.serial(_Pitch);
			}
		}
	}
	else
	{
		hasScheme = _PitchScheme != NULL;
		f.serial(hasScheme);
		if (hasScheme)
		{
			f.serialPolyPtr(_PitchScheme);
		}
		else
		{
			f.serial(_Pitch);
		}
	}

	if (ver > 1)
	{
		f.serial(_EmissionPercent);
		f.serial(_SpawnSounds);
	}

	if (f.isReading())
	{
		_SoundStopped = true;
		// insert blank sources
		for (sint k = 0; k < nbSounds; ++k)
		{
			CPSEmitterInfo ei;
			ei.setDefaults();
			newElement(ei);
		}
		_SoundStopped = false;
		_SoundReactivated = true;
	}
}


// ***************************************************************************************************
void			CPSSound::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSSound_newElement)
	nlassert(_Owner);
	if (_GainScheme && _GainScheme->hasMemory()) _GainScheme->newElement(info);
	if (_PitchScheme && _PitchScheme->hasMemory()) _PitchScheme->newElement(info);
	// if there's a sound server, we generate a new sound instance
	if (!_Mute && !_SoundStopped && CParticleSystem::getSoundServer())
	{
		if ((rand() % 99) * 0.01f < _EmissionPercent)
		{
			uint32 index = _Sounds.insert(CParticleSystem::getSoundServer()->createSound(_SoundName, _SpawnSounds));



			/// set position and activate the sound
			if (_Sounds[index])
			{
				const NLMISC::CMatrix &mat = getLocalToWorldMatrix();
				float pitch;
				if (_UseOriginalPitch)
				{
					pitch = _Sounds[index]->getPitch();
				}
				else
				{
					pitch = _PitchScheme ? _PitchScheme->get(getOwner(), 0) : _Pitch;
				}
				_Sounds[index]->setSoundParams(_GainScheme ? _GainScheme->get(getOwner(), 0) : _Gain,
											   mat * _Owner->getPos()[index],
											   _Owner->getSpeed()[index],
											   pitch
											  );
				_Sounds[index]->play();
			}
		}
		else
		{
			_Sounds.insert(NULL);
		}
	}
	else
	{
		_Sounds.insert(NULL);
	}
}

// ***************************************************************************************************
void	CPSSound::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSSound_deleteElement)
	if (_GainScheme && _GainScheme->hasMemory()) _GainScheme->deleteElement(index);
	if (_PitchScheme && _PitchScheme->hasMemory()) _PitchScheme->deleteElement(index);
	if (_Sounds[index])
	{
		_Sounds[index]->setLooping(false);
		_Sounds[index]->release();
	}
	_Sounds.remove(index);
}

// ***************************************************************************************************
void	CPSSound::resize(uint32 size)
{
	NL_PS_FUNC(CPSSound_resize)
	nlassert(size < (1 << 16));
	if (_GainScheme && _GainScheme->hasMemory()) _GainScheme->resize(size, getOwner()->getSize());
	if (_PitchScheme && _PitchScheme->hasMemory()) _PitchScheme->resize(size, getOwner()->getSize());
	if (size < _Sounds.getSize())
	{
		// if vector size has been shrunk, must delete sounds instances
		for(uint k = size; k < _Sounds.getSize(); ++k)
		{
			if (_Sounds[k])
			{
				_Sounds[k]->setLooping(false);
				_Sounds[k]->release();
			}
		}
	}
	_Sounds.resize(size);
}

// ***************************************************************************************************
void	CPSSound::setUseOriginalPitchFlag(bool useOriginalPitch)
{
	NL_PS_FUNC(CPSSound_setUseOriginalPitchFlag)
	if (_PitchScheme)
	{
		delete _PitchScheme;
		_PitchScheme = NULL;
	}
	_UseOriginalPitch = useOriginalPitch;
}


} // NL3D
