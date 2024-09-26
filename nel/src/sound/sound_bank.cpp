// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2012-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/sound/sound_bank.h"
#include "nel/sound/simple_sound.h"
#include "nel/sound/complex_sound.h"
#include "nel/sound/context_sound.h"
#include "nel/sound/background_sound.h"
#include "nel/sound/music_sound.h"
#include "nel/sound/stream_sound.h"
#include "nel/sound/stream_file_sound.h"

#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/misc/path.h"
#include "nel/sound/driver/buffer.h"

#include "nel/georges/load_form.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


namespace NLSOUND {

void CSoundBank::bufferUnloaded(const NLMISC::TStringId  &bufferName)
{
	TBufferAssocContainer::iterator it(_BufferAssoc.find(bufferName));

	if (it != _BufferAssoc.end())
	{
		// ok, found some sound associated with this buffer.
		// update all sounds.
		TSimpleSoundContainer::iterator first(it->second.begin()), last(it->second.end());
		for (; first != last; ++first)
		{
			// remove the associated buffer.
			CSimpleSound *ss = const_cast<CSimpleSound*>(*(first));
			ss->setBuffer(NULL);
		}
	}
}

//void CSoundBank::bufferLoaded(const std::string bufferName, IBuffer *buffer)
void CSoundBank::bufferLoaded(const NLMISC::TStringId &/* bufferName */, IBuffer *buffer)
{
//	std::map<std::string, std::vector<TBufferAssoc> >::iterator it(_BufferAssoc.find(buffer->getName()));
	TBufferAssocContainer::iterator it(_BufferAssoc.find(buffer->getName()));

	if (it != _BufferAssoc.end())
	{
		// ok, found some sound associated with this buffer.
		// update all sounds.
		TSimpleSoundContainer::iterator first(it->second.begin()), last(it->second.end());
		for (; first != last; ++first)
		{
			CSimpleSound *ss = const_cast<CSimpleSound*>(*(it->second.begin()));
			// restore the associated buffer.
			ss->setBuffer(buffer);
		}
	}

}

void CSoundBank::registerBufferAssoc(CSimpleSound *sound, IBuffer *buffer)
{
	if (buffer != NULL)
	{
		const NLMISC::TStringId &bufferName = buffer->getName();
		_BufferAssoc[bufferName].insert(sound);
	}
}

void CSoundBank::unregisterBufferAssoc(CSimpleSound *sound, IBuffer * buffer)
{
	if (buffer != NULL)
	{
		const TStringId &bufferName = buffer->getName();
		TBufferAssocContainer::iterator it(_BufferAssoc.find(bufferName));

		if (it != _BufferAssoc.end())
		{
			TSimpleSoundContainer::iterator it2(it->second.find(sound));

			nlassert(it2 != it->second.end());
			it->second.erase(it2);

			if (it->second.empty())
			{
				// last sound refenrecing this buffer
				_BufferAssoc.erase(it);
			}

		}
	}
}


/*
 * Destructor
 */
CSoundBank::~CSoundBank()
{
	unload();
}

void CSoundBank::addSound(CSound *sound)
{
	std::pair<TSoundTable::iterator, bool> ret;
	ret = _Sounds.insert(make_pair(sound->getName(), sound));
	nlassert(ret.second);
}

void CSoundBank::removeSound(const NLMISC::TStringId &name)
{
	_Sounds.erase(name);
}


/** Pseudo serializer for packed sheet loading/saving.
 *	This class act as a wrapper to create sound from
 *	xml document being read, call the serialisation method
 *	to read/write packed sheet and to delete sounds that are
 *	no more needed.
 */
class CSoundSerializer
{
public:
	/// The sound beeing managed by this serializer.
	CSound *Sound;

	/// Default constructor.
	CSoundSerializer()
		: Sound(0)
	{}

	// load the values using the george sheet (called by GEORGE::loadForm)
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const std::string &name)
	{
		// just call the sound creation method with the xml form.
		Sound = CSound::createSound(name, form->getRootNode());

		// success ?
//		if (_Sound != 0)
//			CSoundBank::instance()->addSound(_Sound);
	}

	// load/save the values using the serial system (called by GEORGE::loadForm)
	void serial (NLMISC::IStream &s)
	{
		if (s.isReading())
		{
			// read the first item to find the type
			CSound::TSOUND_TYPE type = CSound::SOUND_SIMPLE;
			s.serialEnum(type);
			// read the sound name
//			std::string name;
//			s.serial(name);

			// Instantiate the corresponding sound.
			switch(CSound::TSOUND_TYPE(type))
			{
			case CSound::SOUND_SIMPLE:
				Sound = new CSimpleSound();
				break;
			case CSound::SOUND_COMPLEX:
				Sound = new CComplexSound();
				break;
			case CSound::SOUND_CONTEXT:
				Sound = new CContextSound();
				break;
			case CSound::SOUND_BACKGROUND:
				Sound = new CBackgroundSound();
				break;
			case CSound::SOUND_MUSIC:
				Sound = new CMusicSound();
				break;
			case CSound::SOUND_STREAM:
				Sound = new CStreamSound();
				break;
			case CSound::SOUND_STREAM_FILE:
				Sound = new CStreamFileSound();
				break;
			default:
				Sound = 0;
			}

//			nlassert(_Sound != 0);
			if (Sound)
			{
				// read the sound data
				Sound->serial(s);
//				CSoundBank::instance()->addSound(_Sound);
			}
		}
		else
		{
			if (Sound == 0)
			{
				// the sound doesn't exist
				uint32 i = std::numeric_limits<uint32>::max();
				s.serialEnum(i);
//				s.serial(std::string("bad sound"));
			}
			else
			{
				// write the sound type.
				CSound::TSOUND_TYPE type = Sound->getSoundType();
				s.serialEnum(type);
				// write the sound name
//				s.serial(const_cast<std::string&>(_Sound->getName()));

				// and write the sound data
				Sound->serial(s);
			}
		}
	}

	/** called by GEORGE::loadForm when a sheet read from the packed sheet is no more in
	 *	the directories.
	 */
	void removed()
	{
		if (Sound != 0)
		{
			// we remove the sound from the bank and delete it.
//			CSoundBank::instance()->removeSound(_Sound->getName());
			delete Sound;
		}
	}

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 3; }
};


/** Load all the sound samples.
 *
 * Can throw EPathNotFound or ESoundFileNotFound (check Exception)
 */
void CSoundBank::load(const std::string &packedSheetDir, bool packedSheetUpdate)
{
	// this structure is fill by the loadForm() function and will contain all you need
	std::map<std::string, CSoundSerializer> Container;
	nlassert(!_Loaded);
	// Just call the GEORGE::loadFrom method to read all available sounds
	::loadForm("sound", packedSheetDir + "sounds.packed_sheets", Container, packedSheetUpdate, false);
	_Loaded = true;

	// add all the loaded sound in the sound banks
	std::map<std::string, CSoundSerializer>::iterator first(Container.begin()), last(Container.end());
	for (; first != last; ++first)
	{
		if (first->second.Sound != 0)
			addSound(first->second.Sound);
	}

	Container.clear();
}


/*
 * Unload all the sound samples in this bank.
 */
void				CSoundBank::unload()
{
	nlassert(_Loaded);

	TSoundTable::iterator first(_Sounds.begin()), last(_Sounds.end());
	for (; first != last; ++first)
	{
		delete first->second;
	}

	_Sounds.clear();
	_Loaded = false;

/*	vector<CSound*> vec;


	TSoundTable::iterator map_iter;

	for (map_iter = _Sounds.begin(); map_iter != _Sounds.end(); ++map_iter)
	{
		// We can't delete directly second because the map is based on second->getName()
		vec.push_back( (*map_iter).second );
	}

	_Sounds.clear();

	vector<CSound*>::iterator vec_iter;

	for (vec_iter = vec.begin(); vec_iter != vec.end(); ++vec_iter)
	{
		CSound *sound = *vec_iter;
		delete sound;
	}

	_Loaded = false;
*/
}

/*
 * Returns true if the samples in this bank have been loaded.
 */
bool				CSoundBank::isLoaded()
{
	return _Loaded;
}

/*
 * Return a sound sample corresponding to a name.
 */
CSound*			CSoundBank::getSound(const NLMISC::TStringId &name)
{
	// Find sound
	TSoundTable::iterator iter = _Sounds.find(name);
	if ( iter == _Sounds.end() )
	{
		return 0;
	}
	else
	{
		return (*iter).second;
	}
}

/**
 *  Return the names of the sounds
 */
void				CSoundBank::getNames( std::vector<NLMISC::TStringId> &names )
{
	TSoundTable::const_iterator iter;
	for (iter = _Sounds.begin(); iter != _Sounds.end(); ++iter)
	{
		names.push_back((*iter).first);
		//nlwarning("getting sound %s", (*iter).first);
	}
}

/*
 * Return the number of buffers in this bank.
 */
uint				CSoundBank::countSounds()
{
	return (uint)_Sounds.size();
}


} // namespace NLSOUND

