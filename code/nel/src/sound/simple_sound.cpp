// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdsound.h"

#include "nel/misc/path.h"
#include "nel/sound/simple_sound.h"
#include "nel/sound/sound_bank.h"
#include "nel/sound/sample_bank_manager.h"
#include "nel/sound/sample_bank.h"
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/buffer.h"

using namespace std;
using namespace NLMISC;


namespace NLSOUND {


/*
 * Constructor
 */
CSimpleSound::CSimpleSound() :
	_Registered(false),
	_Buffer(NULL),
	// _Detailed(false), // not used?
	_Alpha(1.0),
	_NeedContext(false)
{
	// init with NULL in case of unexecpted access
	_Filename= NULL;
	_Buffername= NULL;
}


/*
 * Destructor
 */
CSimpleSound::~CSimpleSound()
{
	if (_Buffer != NULL)
		CAudioMixerUser::getInstance()->getSoundBank()->unregisterBufferAssoc(this, _Buffer);
}

void CSimpleSound::setBuffer(IBuffer *buffer)
{
	if (_Buffer != NULL && buffer != NULL && _Buffer->getName() != buffer->getName())
	{
		// if buffer name change, update the registration/
		CAudioMixerUser::getInstance()->getSoundBank()->unregisterBufferAssoc(this, _Buffer);
		CAudioMixerUser::getInstance()->getSoundBank()->registerBufferAssoc(this, buffer);
	}
	else if (!_Registered && buffer != NULL)
	{
		// creater initial registration.
		CAudioMixerUser::getInstance()->getSoundBank()->registerBufferAssoc(this, buffer);
		_Registered = true;
	}
	_Buffer = buffer;
}


void				CSimpleSound::getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const
{
	// A little hack, we use the reference vector to tag unavailable sample.
	if (!(_Buffername == CStringMapper::emptyId()) && const_cast<CSimpleSound*>(this)->getBuffer() == 0)
		subsounds.push_back(pair<string, CSound*>(CStringMapper::unmap(_Buffername)+" (sample)", (CSound*)NULL));
}


/*
 * Return the sample buffer of this sound
 */
IBuffer*			CSimpleSound::getBuffer()
{
	if (_Buffer == 0)
	{
		// try to find the sample buffer in the sample bank.
		CAudioMixerUser *audioMixer = CAudioMixerUser::instance();
		_Buffer = audioMixer->getSampleBankManager()->get(_Buffername);
		audioMixer->getSoundBank()->registerBufferAssoc(this, _Buffer);
		_Registered = true;
	}
	return _Buffer;
}


/*
 * Return the length of the sound in ms
 */
uint32				CSimpleSound::getDuration()
{
	IBuffer* buffer = getBuffer();

	if ( buffer == NULL )
	{
		return 0;
	}
	else
	{
		return (uint32)(buffer->getDuration());
	}
}


void				CSimpleSound::serial(NLMISC::IStream &s)
{
	std::string bufferName;
	CSound::serial(s);

	s.serial(_MinDist);
	s.serial(_Alpha);

	if (s.isReading())
	{
		s.serial(bufferName);
		_Buffername = CStringMapper::map(bufferName);
		setBuffer(NULL);

		// contain % so it need a context to play
		if (bufferName.find ("%") != string::npos)
		{
			_NeedContext = true;
		}
	}
	else
	{
		bufferName = CStringMapper::unmap(_Buffername);
		s.serial(bufferName);
	}
}


/**
 * 	Load the sound parameters from georges' form
 */
void				CSimpleSound::importForm(const std::string& filename, NLGEORGES::UFormElm& root)
{
	NLGEORGES::UFormElm *psoundType;
	std::string dfnName;

	// some basic checking.
	root.getNodeByName(&psoundType, ".SoundType");
	nlassert(psoundType != NULL);
	psoundType->getDfnName(dfnName);
	nlassert(dfnName == "simple_sound.dfn");

	// Call the base class
	CSound::importForm(filename, root);

	// Name
	_Filename = CStringMapper::map(filename);

	// Buffername
	std::string bufferName;
	root.getValueByName(bufferName, ".SoundType.Filename");
	bufferName = CFile::getFilenameWithoutExtension(bufferName);
	_Buffername = CStringMapper::map(bufferName);

	setBuffer(NULL);

	// contain % so it need a context to play
	if (bufferName.find ("%") != string::npos)
	{
		_NeedContext = true;
	}

	// MaxDistance
 	root.getValueByName(_MaxDist, ".SoundType.MaxDistance");

	// MinDistance
	root.getValueByName(_MinDist, ".SoundType.MinDistance");

	// Alpha
	root.getValueByName(_Alpha, ".SoundType.Alpha");

}

} // NLSOUND
