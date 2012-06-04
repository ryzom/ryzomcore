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

#ifndef NL_SOUND_BANK_H
#define NL_SOUND_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "nel/sound/audio_mixer_user.h"
#include "nel/misc/sheet_id.h"
#include <string>

namespace NLSOUND {

class CSound;
class CSimpleSound;


/**
 * A set of sounds.
 * \author Peter Hanappe
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2001
 */
class CSoundBank
{
public:

	void registerBufferAssoc(CSimpleSound *sound, IBuffer *buffer);
	void unregisterBufferAssoc(CSimpleSound *sound, IBuffer *buffer);

	/** Called by CSampleBank when a sample(buffer) is unloaded.
	 *	Remove link from CSound to unloaded IBuffer.
	 */
	void	bufferUnloaded(const NLMISC::TStringId &bufferName);
	/** Called by CSampleBank when a sample(buffer) is loaded.
	 *	Regenerate link between CSound and IBuffer.
	 */
	void	bufferLoaded(const NLMISC::TStringId &bufferName, IBuffer *buffer);

	/// Constructor
	CSoundBank() : _Loaded(false) { }

	/// Destructor
	virtual ~CSoundBank();

	/// Load all the sounds.
	void				load(const std::string &packedSheetDir, bool packedSheetUpdate);

	/// Remove all the sounds in this bank.
	void				unload();

	/// Returns true if the sounds in this bank have been loaded.
	bool				isLoaded();

	/// Return a sound corresponding to a name.
	CSound				*getSound(const NLMISC::CSheetId &sheetId);

	/// Return the names of the sounds
	void				getNames( std::vector<NLMISC::CSheetId> &sheetIds );

	/// Return the number of sounds in this bank.
	uint				countSounds();

	void				addSound(CSound *sound);
	void				removeSound(const NLMISC::CSheetId &sheetId);


private:
	/// CSoundBank singleton instance.
	//static CSoundBank		*_Instance;

	typedef CHashSet<class CSimpleSound*, THashPtr<CSimpleSound*> >	TSimpleSoundContainer;
//	typedef std::hash_map<std::string, TSimpleSoundContainer >				TBufferAssocContainer;
	typedef CHashMap<NLMISC::TStringId, TSimpleSoundContainer, NLMISC::CStringIdHashMapTraits>		TBufferAssocContainer;
	/// Sound names hash map
//	typedef std::hash_map<std::string, CSound*>								TSoundTable;
//	typedef CHashMap<NLMISC::CSheetId, CSound*, NLMISC::CSheetIdHashMapTraits>						TSoundTable;
	typedef std::vector<CSound *> TSoundTable; // list the sheets by shortId of the sheetId

	/// Assoc from buffer to sound. Used for sound unloading.
	TBufferAssocContainer		_BufferAssoc;

	// Buffer
	TSoundTable					_Sounds;

	// Did we load the buffers.
	bool				_Loaded;

};


/**
 * ESoundFileNotFound
 */

class ESoundBankNotFound : public NLMISC::Exception
{
public:
	ESoundBankNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sound bank not found: ")+filename).c_str() ) {}
};

} // NLSOUND


#endif // NL_SOUND_BANK_H

/* End of sound.h */

