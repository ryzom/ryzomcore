// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#ifndef NLSOUND_SAMPLE_BANK_MANAGER_H
#define NLSOUND_SAMPLE_BANK_MANAGER_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>

// NeL includes
#include <nel/misc/stream.h>
#include <nel/misc/string_mapper.h>
#include <nel/georges/u_form_elm.h>

// Project includes
#include "nel/sound/audio_mixer_user.h"

namespace NLSOUND {
	class ISoundDriver;
	class IBuffer;
	class CSampleBank;

/**
 * CSampleBankManager
 * \brief CSampleBankManager
 * \date 2010-03-08 21:09GMT
 */
class CSampleBankManager
{
	friend class CSampleBank;

public:
	CSampleBankManager(CAudioMixerUser *audioMixer);
	virtual ~CSampleBankManager();

	/** Initialise the sample bank with the mixer config file.
	 */
	void					init(NLGEORGES::UFormElm *mixerConfig);

	/// Delete all the loaded banks.
	void					releaseAll();
	
	/// Return an existing bank for the given file name, NULL if no bank for this file.
	CSampleBank				*findSampleBank(const NLMISC::TStringId &filename);

	/// Return the name corresponding to a name. The sample is searched
	// in all the loaded sample banks.
	IBuffer					*get(const NLMISC::TStringId &name);

	/// Reload all the sample bank.
	void					reload(bool async);

	/// Return the total loaded samples size.
	inline uint				getTotalByteSize() const { return m_LoadedSize; }

	/// Fill a vector with current loaded sample banks.
	void					getLoadedSampleBankInfo(std::vector<std::pair<std::string, uint> > &result);

private:
	typedef CHashMap<NLMISC::TStringId, CSampleBank*, NLMISC::CStringIdHashMapTraits> TSampleBankContainer;

	CAudioMixerUser			*m_AudioMixer;
	
	// The map off all loaded sample banks
	TSampleBankContainer	m_Banks;

	// The total size of loaded samples.
	uint					m_LoadedSize;
	
	struct TFilteredBank
	{
		uint32				Filter;
		NLMISC::TStringId	BankName;
	};
	
	/// List of virtual sample bank.
	typedef CHashMap<NLMISC::TStringId, std::vector<TFilteredBank>, NLMISC::CStringIdHashMapTraits>	TVirtualBankCont;
	TVirtualBankCont		m_VirtualBanks;
	
private:
	CSampleBankManager(const CSampleBankManager &);
	CSampleBankManager &operator=(const CSampleBankManager &);
	
}; /* class CSampleBankManager */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_SAMPLE_BANK_MANAGER_H */

/* end of file */
