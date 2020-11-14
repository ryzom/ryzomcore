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

#include "nel/sound/sample_bank.h"
#include "nel/sound/sample_bank_manager.h"
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/buffer.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/sound/async_file_manager_sound.h"
#include "nel/sound/background_sound_manager.h"
#include "nel/sound/sound_bank.h"


using namespace std;
using namespace NLMISC;


namespace NLSOUND {

/// Constante for the number of file to load asynchronously at a time.
uint32		ASYNC_LOADING_SPLIT = 10;		// 10 file by 10 file


// ********************************************************

CSampleBank::CSampleBank(NLMISC::TStringId name, CSampleBankManager *sampleBankManager)
	: _SampleBankManager(sampleBankManager), _Name(name), _Loaded(false), _LoadingDone(true), _ByteSize(0)
{
	_SampleBankManager->m_Banks.insert(make_pair(name, this));
}


// ********************************************************

CSampleBank::~CSampleBank()
{
	_SampleBankManager->m_AudioMixer->unregisterUpdate(this);
	while (!_LoadingDone)
	{
		// need to wait for loading end.
		nlSleep(100);
	}

	if (_Loaded)
		unload();

	// remove the bank from the list of known banks
	for (CSampleBankManager::TSampleBankContainer::iterator 
		it(_SampleBankManager->m_Banks.begin()), 
		end(_SampleBankManager->m_Banks.end()); it != end; ++it)
	{
		if (it->second == this)
		{
			_SampleBankManager->m_Banks.erase(it);
			break;
		}
	}


	// delete all the samples.
	while (!_Samples.empty())
	{
		delete _Samples.begin()->second;
		_Samples.erase(_Samples.begin());
	}

	_Samples.clear();
}


// ********************************************************

void CSampleBank::load(bool async)
{
	// TODO : add async loading support !

	CSampleBankManager::TVirtualBankCont::iterator it(_SampleBankManager->m_VirtualBanks.find(_Name));
	if (it != _SampleBankManager->m_VirtualBanks.end())
	{
		// this is a virtual sample bank !
		nlinfo("Loading virtual sample bank %s", CStringMapper::unmap(_Name).c_str());
		
		const CAudioMixerUser::TBackgroundFlags &flags = _SampleBankManager->m_AudioMixer->getBackgroundFlags();
		
		for (uint i=0; i<it->second.size(); ++i)
		{
			if (flags.Flags[it->second[i].Filter])
			{
				CSampleBank *bank = _SampleBankManager->findSampleBank(it->second[i].BankName);
				if (bank)
					bank->load(async);
			}
		}
	}

	//nlinfo("Loading sample bank %s %", CStringMapper::unmap(_Name).c_str(), async?"":"Asynchronously");

	vector<string> filenames;
//	vector<string>::iterator iter;

	if (_Loaded)
	{
		nlwarning("Trying to load an already loaded bank : %s", CStringMapper::unmap(_Name).c_str ());
		return;
	}


	// Load the sample bank from the builded sample_bank file.
	string bankName(CStringMapper::unmap(_Name)+".sample_bank");
	string filename = CPath::lookup(bankName, false);
	if (filename.empty())
	{
		nlwarning("Could not find sample bank [%s]", bankName.c_str());
		return;
	}

	try
	{

		CIFile	sampleBank(filename);

		CAudioMixerUser::TSampleBankHeader sbh;
		sampleBank.serial(sbh);
		_LoadingDone = false;

		sint32 seekStart = sampleBank.getPos();


		uint8	*data = 0;
		uint	i;
		for (i=0; i<sbh.Name.size(); ++i)
		{
			IBuffer *ibuffer = _SampleBankManager->m_AudioMixer->getSoundDriver()->createBuffer();
			nlassert(ibuffer);

			TStringId	nameId = CStringMapper::map(CFile::getFilenameWithoutExtension(sbh.Name[i]));
			ibuffer->setName(nameId);

	/*		{
				sint16 *data16 = new sint16[sbh.NbSample[i]];
				IBuffer::TADPCMState	state;
				state.PreviousSample = 0;
				state.StepIndex = 0;
				uint count =0;
				for (count=0; count+1024<sbh.NbSample[i]; count+=1024)
				{
					IBuffer::decodeADPCM(data+count/2, data16+count, 1024, state);
				}
				IBuffer::decodeADPCM(data+count/2, data16+count, sbh.NbSample[i]-count, state);

				state.PreviousSample = 0;
				state.StepIndex = 0;
				sint16	*data16_2 = new sint16[sbh.NbSample[i]];
 				IBuffer::decodeADPCM(data, data16_2, sbh.NbSample[i], state);

				for (uint j=0; j<sbh.NbSample[i]; ++j)
				{
					if (data16[j] != data16_2[j])
					{
						nlwarning("Sample differ at %u", j);
					}
				}

				_SoundDriver->readRawBuffer(ibuffer, sbh.Name[i], (uint8*)data16, sbh.NbSample[i]*2, Mono16, sbh.Freq[i]);
				delete [] data16;
				delete [] data16_2;
			}
	*/

			if (_SampleBankManager->m_AudioMixer->useAPDCM())
			{
				data = (uint8*) realloc(data, sbh.SizeAdpcm[i]);
				sampleBank.seek(seekStart + sbh.OffsetAdpcm[i], CIFile::begin);
				sampleBank.serialBuffer(data, sbh.SizeAdpcm[i]);
				ibuffer->setFormat(IBuffer::FormatDviAdpcm, 1, 16, sbh.Freq[i]);
				if (!ibuffer->fill(data, sbh.SizeAdpcm[i]))
					nlwarning("AM: ibuffer->fill returned false with FormatADPCM");
			}
			else
			{
				data = (uint8*) realloc(data, sbh.SizeMono16[i]);
				sampleBank.seek(seekStart + sbh.OffsetMono16[i], CIFile::begin);
				sampleBank.serialBuffer(data, sbh.SizeMono16[i]);
				ibuffer->setFormat(IBuffer::FormatPcm, 1, 16, sbh.Freq[i]);
				if (!ibuffer->fill(data, sbh.SizeMono16[i]))
					nlwarning("AM: ibuffer->fill returned false with FormatPCM");
			}

			_ByteSize += ibuffer->getSize();

			_Samples[nameId] = ibuffer;

			// Warn the sound bank that the sample are available.
			CAudioMixerUser::getInstance()->getSoundBank()->bufferLoaded(nameId, ibuffer);
		}
		free(data);

		_SampleBankManager->m_LoadedSize += _ByteSize;
	}
	catch(const Exception &e)
	{
		// loading failed !
		nlwarning("Exception %s during loading of sample bank %s", e.what(), filename.c_str());

		if (_SampleBankManager->m_AudioMixer->getPackedSheetUpdate())
		{
			nlinfo("Deleting offending sound bank, you need to restart to recreate it!");
			CFile::deleteFile(filename);
		}
	}

	_Loaded = true;
	_LoadingDone = true;



///////////////////////////////////////// OLD Version //////////////////////////////////////
/*

	std::string list = CPath::lookup(CStringMapper::unmap(_Name)+CAudioMixerUser::SampleBankListExt, false);
	if (list.empty())
	{
		nlwarning("File %s not found to load sample bank %s", (CStringMapper::unmap(_Name)+CAudioMixerUser::SampleBankListExt).c_str(), CStringMapper::unmap(_Name).c_str());
		return;
	}


	NLMISC::CIFile sampleBankList(list);
	sampleBankList.serialCont(filenames);

	for (iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		IBuffer* ibuffer = NULL;
		try
		{
			ibuffer = _SoundDriver->createBuffer();
			nlassert(ibuffer);

//			std::string sampleName(CFile::getFilenameWithoutExtension(*iter));
			NLMISC::TStringId sampleName(CStringMapper::map(CFile::getFilenameWithoutExtension(*iter)));

			if (async)
			{
				ibuffer->presetName(sampleName);
				nldebug("Preloading sample [%s]", CStringMapper::unmap(sampleName).c_str());
			}
			else
			{
				std::string fullName = NLMISC::CPath::lookup(*iter, false);
				if (!fullName.empty())
				{
					NLMISC::CIFile	ifile(fullName);
					uint size = ifile.getFileSize();
					uint8 *buffer = new uint8[ifile.getFileSize()];
					ifile.serialBuffer(buffer, size);

					_SoundDriver->readWavBuffer(ibuffer, fullName, buffer, size);
					_ByteSize += ibuffer->getSize();

					delete [] buffer;
				}
			}
			_Samples[sampleName] = ibuffer ;

			// Warn the sound bank that the sample are available.
			CSoundBank::instance()->bufferLoaded(sampleName, ibuffer);
		}
		catch (const ESoundDriver &e)
		{
			if (ibuffer != NULL) {
				delete ibuffer;
				ibuffer = NULL;
			}
			nlwarning("Problem with file '%s': %s", (*iter).c_str(), e.what());
		}
	}

	_Loaded = true;

	if (!async)
	{
		_LoadingDone = true;
		// compute the sample bank size.
		_LoadedSize += _ByteSize;
	}
	else
	{
		// fill the loading list.
		TSampleTable::iterator first(_Samples.begin()), last(_Samples.end());
		for (; first != last; ++first)
		{
			_LoadList.push_back(make_pair(first->second, first->first));
		}
		_SplitLoadDone = false;
		// send the first files
		for (uint i=0; i<ASYNC_LOADING_SPLIT && !_LoadList.empty(); ++i)
		{
			CAsyncFileManagerSound::getInstance().loadWavFile(_LoadList.front().first, CStringMapper::unmap(_LoadList.front().second)+".wav");
			_LoadList.pop_front();
		}
		// add a end loading event...
		CAsyncFileManagerSound::getInstance().signal(&_SplitLoadDone);
		// and register for update on the mixer
		CAudioMixerUser::instance()->registerUpdate(this);
	}
	*/
}

void CSampleBank::onUpdate()
{
	if (_SplitLoadDone)
	{
		nldebug("Some samples have been loaded");
		if (_LoadList.empty())
		{
			// all the samples are loaded, we can compute the bank size.
			TSampleTable::iterator	first(_Samples.begin()), last(_Samples.end());
			for (; first != last; ++first)
			{
				_ByteSize += first->second->getSize();
			}

			_SampleBankManager->m_LoadedSize += _ByteSize;

			// stop the update.
			_SampleBankManager->m_AudioMixer->unregisterUpdate(this);
			_LoadingDone = true;

			// Force an update in the background manager (can restar stopped sound).
			_SampleBankManager->m_AudioMixer->getBackgroundSoundManager()->updateBackgroundStatus();

			nlinfo("Sample bank %s loaded.", CStringMapper::unmap(_Name).c_str());
		}
		else
		{
			_SplitLoadDone = false;
			for (uint i=0; i<ASYNC_LOADING_SPLIT && !_LoadList.empty(); ++i)
			{
				CAsyncFileManagerSound::getInstance().loadWavFile(_LoadList.front().first, CStringMapper::unmap(_LoadList.front().second)+".wav");
				_LoadList.pop_front();
			}
			// add a end loading event...
			CAsyncFileManagerSound::getInstance().signal(&_SplitLoadDone);
		}
	}
}

// ********************************************************

bool				CSampleBank::unload()
{
	vector<IBuffer*> vec;
	TSampleTable::iterator it;

	if (!_Loaded)
	{
		nlwarning("Trying to unload an already unloaded bank : %s", CStringMapper::unmap(_Name).c_str ());
		return  true;
	}

	// need to wait end of load ?
	if (!_LoadingDone)
		return false;

	//nlinfo("Unloading sample bank %s", CStringMapper::unmap(_Name).c_str());

	for (it = _Samples.begin(); it != _Samples.end(); ++it)
	{
		IBuffer *buffer = it->second;
		if (buffer)
		{
			const NLMISC::TStringId & bufferName = buffer->getName();
			
			CAudioMixerUser *audioMixer = _SampleBankManager->m_AudioMixer;
			// Warn the mixer to stop any track playing this buffer.
			audioMixer->bufferUnloaded(buffer);

			// Warn the sound banks about this buffer.
			audioMixer->getSoundBank()->bufferUnloaded(bufferName);
			
			// delete
			it->second = NULL;
			delete buffer;
		}
	}

	_Loaded = false;

	_SampleBankManager->m_LoadedSize -= _ByteSize;
	_ByteSize = 0;

	return true;
}

// ********************************************************

bool				CSampleBank::isLoaded()
{
	return _Loaded;
}

// ********************************************************

IBuffer*			CSampleBank::getSample(const NLMISC::TStringId &name)
{
	{
/*		// dump the sample list.
		TSampleTable::iterator it (_Samples.begin()), last(_Samples.end());
		std::string s;

//		while (first != last)
		for (it = _Samples.begin(); it != _Samples.end(); ++it)
		{
			s += std::string(" [")+it->first+"] ";
			//first++;
		}

		nldebug("getSample(%s) : sample list = [%s]", name, s.c_str());
*/
	}

	// Find sound
	TSampleTable::iterator iter = _Samples.find(name);
	if ( iter == _Samples.end() )
	{
		return 0;
	}
	else
	{
		return (*iter).second;
	}
}

// ********************************************************

uint				CSampleBank::countSamples()
{
	return (uint)_Samples.size();
}

// ********************************************************

uint				CSampleBank::getSize()
{
	uint size = 0;

	TSampleTable::const_iterator iter;
	for (iter = _Samples.begin(); iter != _Samples.end(); iter++)
	{
		size +=	(*iter).second->getSize();
	}

	return size;
}



} // namespace NLSOUND

