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

#include "stdsound.h"
#include "nel/sound/sample_bank_manager.h"

// STL includes

// NeL includes
#include "nel/misc/path.h"
#include "nel/misc/file.h"

// Project includes
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/buffer.h"
#include "nel/sound/sample_bank.h"
#include "nel/sound/async_file_manager_sound.h"
#include "nel/sound/background_sound_manager.h"
#include "nel/sound/sound_bank.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

CSampleBankManager::CSampleBankManager(CAudioMixerUser *audioMixer) : m_AudioMixer(audioMixer), m_LoadedSize(0)
{
	
}

CSampleBankManager::~CSampleBankManager()
{
	releaseAll();
}


void CSampleBankManager::init(NLGEORGES::UFormElm *mixerConfig)
{
	if (mixerConfig == 0)
		return;
	
	NLGEORGES::UFormElm	*virtualBanks;
	mixerConfig->getNodeByName(&virtualBanks, ".VirtualBanks");
	if (virtualBanks == 0)
		return;
	
	uint size;
	virtualBanks->getArraySize(size);
	
	for (uint i=0; i<size; ++i)
	{
		NLGEORGES::UFormElm	*virtualBank;
		virtualBanks->getArrayNode(&virtualBank, i);

		if (virtualBank != 0)
		{
			std::vector<TFilteredBank> vfb;
			std::string virtualName;
			virtualBank->getValueByName(virtualName, ".VirtualName");
			NLGEORGES::UFormElm	*realBanks;
			virtualBank->getNodeByName(&realBanks, ".FilteredBank");
			if (realBanks != 0)
			{
				uint size2;
				realBanks->getArraySize(size2);

				for (uint j=0; j<size2; ++j)
				{
					TFilteredBank fb;
					std::string	bankName;
					NLGEORGES::UFormElm	*realBank = NULL;
					realBanks->getArrayNode(&realBank, j);
					if (realBank != 0)
					{
						realBank->getValueByName(bankName, ".SampleBank");
						fb.BankName = CStringMapper::map(bankName);
						realBank->getValueByName(fb.Filter, ".Filter");
						vfb.push_back(fb);
					}
				}
			}

			if (!vfb.empty())
			{
				TStringId virtualNameId = CStringMapper::map(virtualName);
				m_VirtualBanks.insert(std::make_pair(virtualNameId, vfb));
				// create the sample bank
				CSampleBank *sampleBank = new CSampleBank(virtualNameId, this);
			}
		}
	}
}

void CSampleBankManager::releaseAll()
{
	// nldebug("SampleBanks: Releasing...");
	while (!m_Banks.empty())
	{
		delete m_Banks.begin()->second;
	}
	// nldebug("SampleBanks: Released");
}

CSampleBank *CSampleBankManager::findSampleBank(const NLMISC::TStringId &filename)
{
	TSampleBankContainer::iterator it(m_Banks.find(filename));

	if (it != m_Banks.end())
		return it->second;

	return NULL;
}

IBuffer *CSampleBankManager::get(const NLMISC::TStringId &name)
{
	IBuffer* buffer;
	TSampleBankContainer::iterator iter;

	for (iter = m_Banks.begin(); iter != m_Banks.end(); ++iter)
	{
		buffer = iter->second->getSample(name);
		if (buffer != 0)
		{
			return buffer;
		}
	}

	//nlwarning ("Try to get an unknown sample '%s'", name);
	return 0;
}

void CSampleBankManager::reload(bool async)
{
	TSampleBankContainer::iterator first(m_Banks.begin()), last(m_Banks.end());

	for (; first != last; ++first)
	{
		first->second->unload();
		first->second->load(async);
	}
}


void CSampleBankManager::getLoadedSampleBankInfo(std::vector<std::pair<std::string, uint> > &result)
{
	result.clear();

	TSampleBankContainer::iterator first(m_Banks.begin()), last(m_Banks.end());
	for (; first != last; ++first)
	{
		std::pair<std::string, uint> p;
		if (first->second->isLoaded())
		{
			p.first = NLMISC::CStringMapper::unmap(first->first);
			p.second = first->second->getSize();
			result.push_back(p);
		}
	}
}

} /* namespace NLSOUND */

/* end of file */
