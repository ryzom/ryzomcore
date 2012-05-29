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

#ifndef NL_PS_SOUND_INTERFACE_H
#define NL_PS_SOUND_INTERFACE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/sheet_id.h"
#include <string>

namespace NLMISC
{
	class CVector;
}

namespace NL3D
{

struct UPSSoundInstance;


/**
 * This class is an interface which allow the particle system to create a sound. When a sound is created,
 * the system get an interface on a sound instance. The interface must be registered to the particle system.
 * when it has been created.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
struct UPSSoundServer
{
	virtual ~UPSSoundServer() {}

	/** Querry the implementer to create a sound instance, and retrieve an interface to it.
	  * NULL means that the server can't create the sound, so it is ignored
	  * \param soundName the name of the sound in the sound bank
	  * \param spawn     true if the sound must be spawned e.g it continues after this interface is removed
	  * \param cb		 useful only for spawned sound, it tells when a spawned sound has been removed
	  */
	virtual UPSSoundInstance *createSound(const NLMISC::CSheetId &soundName, bool spawn = false) = 0;
};


/**
  * This is an interface between the particle system and a sound instance. When the system call 'release' on this interface
  * , the sound must be detroyed. If a sound stop before relese is called, this interface must remains valid, however.
  */

struct UPSSoundInstance
{
	virtual ~UPSSoundInstance() {}

	/** The system will call this method to set the parameters of the sound
	  * Values are clamped
	  */
	virtual void setSoundParams(float gain,
								const NLMISC::CVector &pos,
								const NLMISC::CVector &velocity,
								float frequency
							   ) = 0;

	/// start to play the sound
	virtual void play(void) = 0;

	/// tells whether the sound is playing
	virtual bool isPlaying(void) const = 0;

	/// stop the sound
	virtual void stop(void) = 0;

	/// when this method is called, the sound is not needed anymore by the system
	virtual void release(void) = 0;

	/// get pitch
	virtual float getPitch() const = 0;

	// set sound looping
	virtual void setLooping(bool looping) = 0;

	// test if sound is looping
	virtual bool isLooping() const = 0;
};



} // NL3D


#endif // NL_PS_SOUND_INTERFACE_H

/* End of ps_sound_interface.h */
