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

#ifndef NL_SAMPLE_BANK_H
#define NL_SAMPLE_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/string_mapper.h"
#include "nel/georges/u_form_elm.h"
#include "nel/sound/u_source.h"
#include "nel/sound/audio_mixer_user.h"
#include <string>

namespace NLSOUND {

class ISoundDriver;
class IBuffer;
class CSampleBankManager;
class CSampleBank;

/*
// Comparision for const char*
struct eqname
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};
*/

/// Sample names hash map
typedef CHashMap<NLMISC::TStringId, IBuffer*, NLMISC::CStringIdHashMapTraits> TSampleTable;

/**
 * A set of samples.
 * \author Peter Hanappe
 * \author Nevrax France
 * \date 2001
 */
class CSampleBank : public CAudioMixerUser::IMixerUpdate
{
public:
	/// Constructor
	CSampleBank(NLMISC::TStringId name, CSampleBankManager *sampleBankManager);

	/// Destructor
	virtual ~CSampleBank();

	/** Load all the samples.
	 *
	 * Can throw EPathNotFound or ESoundFileNotFound (check Exception)
	 * \param async If true, the samples are loaded in background.
	 */
	void				load(bool async);

	/** Unload all the samples in this bank. Return false is unload can't be done (if an async
	 *	loading is not terminated.
	 */
	bool				unload();


	/// Returns true if the samples in this bank have been loaded.
	bool				isLoaded();

	/// Return a samples corresponding to a name.
	IBuffer				*getSample(const NLMISC::TStringId &name);

	/// Return the number of samples in this bank.
	uint				countSamples();

	/// Return the size of this bank in bytes.
	uint				getSize();

	/// Return the name (must be unique)
	NLMISC::TStringId	getName() const { return _Name; }
	
private:
	/// The update method. Used when waiting for async sample loading.
	void onUpdate();

	// Sample bank manager
	CSampleBankManager	*_SampleBankManager;

	// Hashtable with samples
	TSampleTable		_Samples;

	// Sample bank name and path
	NLMISC::TStringId	_Name;

	// Did we load the buffers.
	bool				_Loaded;
	// Is the async load is done ?
	bool				_LoadingDone;
	// The size of the samples in the bank
	uint				_ByteSize;
	
	/// Flag for splitted load.
	bool				_SplitLoadDone;

	/// List of sample that need to be loaded asynchronously.
	std::list<std::pair<IBuffer *, NLMISC::TStringId> >	_LoadList;
	
};

/**
 * ESoundFileNotFound
 */
class ESampleBankNotFound : public NLMISC::Exception
{
public:
	ESampleBankNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sample bank not found: ")+filename).c_str() ) {}
};

} // NLSOUND


#endif // NL_SAMPLE_BANK_H

/* End of sound.h */

