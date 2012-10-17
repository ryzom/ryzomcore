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

#include "stdsound.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/sound/async_file_manager_sound.h"
#include "nel/misc/async_file_manager.h"
#include "nel/sound/driver/buffer.h"
#include "nel/sound/audio_mixer_user.h"

using namespace NLMISC;

namespace NLSOUND
{

//CAsyncFileManagerSound	*CAsyncFileManagerSound::_Singleton;
NLMISC_SAFE_SINGLETON_IMPL(CAsyncFileManagerSound);


/*CAsyncFileManagerSound &CAsyncFileManagerSound::getInstance()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CAsyncFileManagerSound();
	}
	return *_Singleton;
}
*/

void	CAsyncFileManagerSound::terminate()
{
	if (_Instance != NULL)
	{
		INelContext::getInstance().releaseSingletonPointer("CAsyncFileManagerSound", _Instance);
		delete _Instance;
		_Instance = NULL;
	}
}

void	CAsyncFileManagerSound::loadWavFile(IBuffer *pdestBuffer, const std::string &filename)
{
	CAsyncFileManager::getInstance().addLoadTask(new CLoadWavFile(pdestBuffer, filename));
}


class CCancelLoadWavFile : public CAsyncFileManager::ICancelCallback
{
	std::string	_Filename;

	bool callback(const NLMISC::IRunnable *prunnable) const
	{
		const CAsyncFileManagerSound::CLoadWavFile *pLWF = dynamic_cast<const CAsyncFileManagerSound::CLoadWavFile*>(prunnable);

		if (pLWF != NULL)
		{
			if (pLWF->_Filename == _Filename)
				return true;
		}
		return false;
	}

public:
	CCancelLoadWavFile (const std::string &filename)
		: _Filename (filename)
	{}
};

void	CAsyncFileManagerSound::cancelLoadWaveFile(const std::string &/* filename */)
{
	nlwarning("CAsyncFileManagerSound::cancelLoadWaveFile : not implemented yet !");
//	CAsyncFileManager::getInstance().cancelLoadTask(CCancelLoadWavFile(filename));
}


// Do not use these methods with the bigfile manager
void CAsyncFileManagerSound::loadFile (const std::string &fileName, uint8 **pPtr)
{
	CAsyncFileManager::getInstance().loadFile(fileName, pPtr);
}

void CAsyncFileManagerSound::loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs)
{
	if (vFileNames.size() != vPtrs.size())
	{
		nlwarning("CAsyncFileManagerSound::loadFiles : number of filenames and pointer differ ! (%u file, %u ptr)", vFileNames.size(), vPtrs.size());
		// ignore load request...
		return;
	}
	CAsyncFileManager::getInstance().loadFiles(vFileNames, vPtrs);
}

void CAsyncFileManagerSound::signal (bool *pSgn)
{
	if (pSgn == 0)
	{
		nlwarning("CAsyncFileManagerSound::signal : trying to signal with a null pointer !");
		return;
	}
	CAsyncFileManager::getInstance().signal(pSgn);
}

void CAsyncFileManagerSound::cancelSignal (bool *pSgn)
{
	if (pSgn == 0)
	{
		nlwarning("CAsyncFileManagerSound::cancelSignal : trying to remove a signal with a null pointer !");
		return;
	}
	CAsyncFileManager::getInstance().cancelSignal(pSgn);
}


// Load task.
CAsyncFileManagerSound::CLoadWavFile::CLoadWavFile (IBuffer *pdestBuffer, const std::string &filename)
: _pDestbuffer(pdestBuffer), _Filename(filename)
{
	if (_Filename.empty())
	{
		nlwarning("CAsyncFileManagerSound::CLoadWavFile::CLoadWavFile : file name is empty !");
	}
	if (_pDestbuffer == 0)
	{
		nlwarning("CAsyncFileManagerSound::CLoadWavFile::CLoadWavFile : dest buffer ptr is null!");
	}
}

void CAsyncFileManagerSound::CLoadWavFile::run (void)
{
	nldebug("Loading sample %s...", _Filename.c_str());
//	nlSleep(500);
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	if (mixer == 0)
	{
		nlwarning("CAsyncFileManagerSound::CLoadWavFile::run : mixer is not avalable !");
		return;
	}

	ISoundDriver *sndDrv = mixer->getSoundDriver();
	if (sndDrv == 0)
	{
		nlwarning("CAsyncFileManagerSound::CLoadWavFile::run : sound driver is null !");
		return;
	}

	if (_pDestbuffer == 0)
	{
		nlwarning("CAsyncFileManagerSound::CLoadWavFile::run : dest buffer is null !");
		return;
	}

	if (_Filename.empty())
	{
		nlwarning("CAsyncFileManagerSound::CLoadWavFile::run : filename is empty !");
		return;
	}
	try
	{
		static NLMISC::TStringId empty(CStringMapper::map(""));
		NLMISC::TStringId nameId = CStringMapper::map(CFile::getFilenameWithoutExtension(_Filename));
		if (nameId != empty) nlassertex(nameId == _pDestbuffer->getName(), ("The preset buffer name doesn't match!"));
		_pDestbuffer->setName(nameId);

		NLMISC::CIFile ifile(NLMISC::CPath::lookup(_Filename));
		uint size = ifile.getFileSize();
		std::vector<uint8> buffer;
		buffer.resize(size);
		ifile.serialBuffer(&buffer[0], size);

		std::vector<uint8> result;
		IBuffer::TBufferFormat bufferFormat;
		uint8 channels;
		uint8 bitsPerSample;
		uint32 frequency;

		if (!IBuffer::readWav(&buffer[0], size, result, bufferFormat, channels, bitsPerSample, frequency))
		{
			nlwarning("CAsyncFileManagerSound::CLoadWavFile::run : IBuffer::readWav returned false !");
			return;
		}

		_pDestbuffer->setFormat(bufferFormat, channels, bitsPerSample, frequency);
		if (!_pDestbuffer->fill(&result[0], (uint)result.size()))
		{
			nlwarning("CAsyncFileManagerSound::CLoadWavFile::run : _pDestbuffer->fill returned false !");
			return;
		}
	}
	catch(...)
	{
		nlwarning("CAsyncFileManagerSound::CLoadWavFile::run : Exeption detected during IDriver::loadWavFile(%p, %s)", _pDestbuffer, _Filename.c_str());
	}
}



} // NLSOUND
