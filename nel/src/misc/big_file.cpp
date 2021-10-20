// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdmisc.h"

#include "nel/misc/file.h"
#include "nel/misc/big_file.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

//CBigFile *CBigFile::_Singleton = NULL;
NLMISC_SAFE_SINGLETON_IMPL(CBigFile);

// ***************************************************************************
void CBigFile::releaseInstance()
{
	if (_Instance)
	{
		NLMISC::INelContext::getInstance().releaseSingletonPointer("CBigFile", _Instance);
		delete _Instance;
		_Instance = NULL;
	}
}
// ***************************************************************************
CBigFile::CThreadFileArray::CThreadFileArray()
{
	_CurrentId = 0;
}

// ***************************************************************************
CBigFile::CThreadFileArray::~CThreadFileArray()
{
	vector<CHandleFile>		*ptr = (vector<CHandleFile>*)_TDS.getPointer();
	if (ptr) delete ptr;
}

// ***************************************************************************
uint32						CBigFile::CThreadFileArray::allocate()
{
	return _CurrentId++;
}

// ***************************************************************************
CBigFile::CHandleFile		&CBigFile::CThreadFileArray::get(uint32 index)
{
	// If the thread struct ptr is NULL, must allocate it.
	vector<CHandleFile>		*ptr= (vector<CHandleFile>*)_TDS.getPointer();
	if(ptr==NULL)
	{
		ptr= new vector<CHandleFile>;
		_TDS.setPointer(ptr);
	}

	// if the vector is not allocated, allocate it (empty entries filled with NULL => not opened FILE* in this thread)
	if(index>=ptr->size())
	{
		ptr->resize((ptrdiff_t)index + 1);
	}

	return (*ptr)[index];
}


// ***************************************************************************
void CBigFile::currentThreadFinished()
{
	_ThreadFileArray.currentThreadFinished();
}

// ***************************************************************************
void CBigFile::CThreadFileArray::currentThreadFinished()
{
	vector<CHandleFile>		*ptr= (vector<CHandleFile>*)_TDS.getPointer();
	if (ptr==NULL) return;
	for (uint k = 0; k < ptr->size(); ++k)
	{
		if ((*ptr)[k].File)
		{
			fclose((*ptr)[k].File);
			(*ptr)[k].File = NULL;
		}
	}
	delete ptr;
	_TDS.setPointer(NULL);
}


// ***************************************************************************
//CBigFile::CBigFile ()
//{
//}
//
//// ***************************************************************************
//CBigFile &CBigFile::getInstance ()
//{
//	if (_Singleton == NULL)
//	{
//		_Singleton = new CBigFile();
//	}
//	return *_Singleton;
//}

// ***************************************************************************
bool CBigFile::add (const std::string &sBigFileName, uint32 nOptions)
{
	// Is already the same bigfile name ?
	string bigfilenamealone = toLowerAscii(CFile::getFilename (sBigFileName));
	if (_BNPs.find(bigfilenamealone) != _BNPs.end())
	{
		nlwarning ("CBigFile::add : bigfile %s already added.", bigfilenamealone.c_str());
		return false;
	}

	// Create the new bnp entry
	BNP &bnp = _BNPs[bigfilenamealone];

	bnp.BigFileName= sBigFileName;

	// Allocate a new ThreadSafe FileId for this bnp.
	bnp.ThreadFileId= _ThreadFileArray.allocate();

	// Get a ThreadSafe handle on the file
	CHandleFile		&handle= _ThreadFileArray.get(bnp.ThreadFileId);

	// Open the big file.
	handle.File = nlfopen (sBigFileName, "rb");
	if (handle.File == NULL)
		return false;

	// Used internally by CBigFile, use optimizations and lower case of filenames
	bnp.InternalUse = true;

	// read BNP header
	if (!bnp.readHeader(handle.File))
	{
		fclose (handle.File);
		handle.File = NULL;
		return false;
	}
	if (nOptions&BF_CACHE_FILE_ON_OPEN)
		bnp.CacheFileOnOpen = true;
	else
		bnp.CacheFileOnOpen = false;

	if (!(nOptions&BF_ALWAYS_OPENED))
	{
		fclose (handle.File);
		handle.File = NULL;
		bnp.AlwaysOpened = false;
	}
	else
	{
		bnp.AlwaysOpened = true;
	}

	//nldebug("BigFile : added bnp '%s' to the collection", bigfilenamealone.c_str());

	return true;
}

// ***************************************************************************
void CBigFile::remove (const std::string &sBigFileName)
{
	if (_BNPs.find (sBigFileName) != _BNPs.end())
	{
		map<string, BNP>::iterator it = _BNPs.find (sBigFileName);
		BNP &rbnp = it->second;
		// Get a ThreadSafe handle on the file
		CHandleFile		&handle= _ThreadFileArray.get(rbnp.ThreadFileId);
		// close it if needed
		if (handle.File != NULL)
		{
			fclose (handle.File);
			handle.File= NULL;
		}

		_BNPs.erase (it);
	}
}

CBigFile::BNP::BNP() : FileNames(NULL), ThreadFileId(0), CacheFileOnOpen(false), AlwaysOpened(false), InternalUse(false), OffsetFromBeginning(0)
{
}

CBigFile::BNP::~BNP()
{
	if (FileNames)
	{
		delete[] FileNames;
		FileNames = NULL;
	}
}

//// ***************************************************************************
bool CBigFile::BNP::readHeader()
{
	// Only external use
	if (InternalUse || BigFileName.empty()) return false;

	FILE *f = nlfopen (BigFileName, "rb");
	if (f == NULL) return false;

	bool res = readHeader(f);
	fclose (f);

	return res;
}

//// ***************************************************************************
bool CBigFile::BNP::readHeader(FILE *file)
{
	if (file == NULL) return false;

	uint32 nFileSize=CFile::getFileSize (file);

	// Result
	if (nlfseek64 (file, nFileSize-4, SEEK_SET) != 0)
	{
		return false;
	}

	if (fread (&OffsetFromBeginning, sizeof(uint32), 1, file) != 1)
	{
		return false;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(OffsetFromBeginning);
#endif

	if (nlfseek64 (file, OffsetFromBeginning, SEEK_SET) != 0)
	{
		return false;
	}

	// Read the file count
	uint32 nNbFile;
	if (fread (&nNbFile, sizeof(uint32), 1, file) != 1)
	{
		return false;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nNbFile);
#endif

	map<string, BNPFile> tempMap;

	if (!InternalUse) SFiles.clear();

	for (uint32 i = 0; i < nNbFile; ++i)
	{
		uint8 nStringSize;
		if (fread (&nStringSize, 1, 1, file) != 1)
		{
			return false;
		}

		char sFileName[256];
		if (nStringSize)
		{
			if (fread(sFileName, 1, nStringSize, file) != nStringSize)
			{
				return false;
			}
		}
		sFileName[nStringSize] = 0;

		uint32 nFileSize2;
		if (fread (&nFileSize2, sizeof(uint32), 1, file) != 1)
		{
			return false;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nFileSize2);
#endif

		uint32 nFilePos;
		if (fread (&nFilePos, sizeof(uint32), 1, file) != 1)
		{
			return false;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nFilePos);
#endif

		if (InternalUse)
		{
			BNPFile bnpfTmp;
			bnpfTmp.Pos = nFilePos;
			bnpfTmp.Size = nFileSize2;
			tempMap.insert (make_pair(toLowerAscii(string(sFileName)), bnpfTmp));
		}
		else
		{
			SBNPFile bnpfTmp;
			bnpfTmp.Name = sFileName;
			bnpfTmp.Pos = nFilePos;
			bnpfTmp.Size = nFileSize2;
			SFiles.push_back(bnpfTmp);
		}
	}

	if (nlfseek64 (file, 0, SEEK_SET) != 0)
	{
		return false;
	}

	// Convert temp map
	if (InternalUse && nNbFile > 0)
	{
		uint nSize = 0, nNb = 0;
		map<string,BNPFile>::iterator it = tempMap.begin();
		while (it != tempMap.end())
		{
			nSize += (uint)it->first.size() + 1;
			nNb++;
			it++;
		}

		if (FileNames)
			delete[] FileNames;

		FileNames = new char[nSize];
		memset(FileNames, 0, nSize);
		Files.resize(nNb);

		it = tempMap.begin();
		nSize = 0;
		nNb = 0;
		while (it != tempMap.end())
		{
			strcpy(FileNames+nSize, it->first.c_str());

			Files[nNb].Name = FileNames+nSize;
			Files[nNb].Size = it->second.Size;
			Files[nNb].Pos = it->second.Pos;

			nSize += (uint)it->first.size() + 1;
			nNb++;
			it++;
		}
	}
	// End of temp map conversion

	return true;
}

bool CBigFile::BNP::appendHeader()
{
	// Only external use
	if (InternalUse || BigFileName.empty()) return false;

	FILE *f = nlfopen (BigFileName, "ab");
	if (f == NULL) return false;

	uint32 nNbFile = (uint32)SFiles.size();

	// value to be serialized
	uint32 nNbFile2 = nNbFile;

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nNbFile2);
#endif

	if (fwrite (&nNbFile2, sizeof(uint32), 1, f) != 1)
	{
		fclose(f);
		return false;
	}

	for (uint32 i = 0; i < nNbFile; ++i)
	{
		uint8 nStringSize = (uint8)SFiles[i].Name.length();
		if (fwrite (&nStringSize, 1, 1, f) != 1)
		{
			fclose(f);
			return false;
		}

		if (fwrite (SFiles[i].Name.c_str(), 1, nStringSize, f) != nStringSize)
		{
			fclose(f);
			return false;
		}

		uint32 nFileSize = SFiles[i].Size;

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nFileSize);
#endif

		if (fwrite (&nFileSize, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}

		uint32 nFilePos = SFiles[i].Pos;

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nFilePos);
#endif

		if (fwrite (&nFilePos, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}
	}

	uint32 nOffsetFromBeginning = OffsetFromBeginning;

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nOffsetFromBeginning);
#endif

	if (fwrite (&nOffsetFromBeginning, sizeof(uint32), 1, f) != 1)
	{
		fclose(f);
		return false;
	}

	fclose (f);
	return true;
}

// ***************************************************************************
bool CBigFile::BNP::appendFile(const std::string &filename)
{
	// Only external use
	if (InternalUse || BigFileName.empty()) return false;

	// Check if we can read the source file
	if (!CFile::fileExists(filename)) return false;

	SBNPFile ftmp;
	ftmp.Name = CFile::getFilename(filename);
	ftmp.Size = CFile::getFileSize(filename);
	ftmp.Pos = OffsetFromBeginning;
	SFiles.push_back(ftmp);
	OffsetFromBeginning += ftmp.Size;

	FILE *f1 = nlfopen(BigFileName, "ab");
	if (f1 == NULL) return false;

	FILE *f2 = nlfopen(filename, "rb");
	if (f2 == NULL)
	{
		fclose(f1);
		return false;
	}
	
	uint8 *ptr = new uint8[ftmp.Size];

	if (fread (ptr, ftmp.Size, 1, f2) != 1)
	{
		nlwarning("%s read error", filename.c_str());
	}
	else if (fwrite (ptr, ftmp.Size, 1, f1) != 1)
	{
		nlwarning("%s write error", BigFileName.c_str());
	}

	delete [] ptr;
	
	fclose(f2);
	fclose(f1);

	return true;
}

// ***************************************************************************
bool CBigFile::BNP::unpack(const std::string &sDestDir, TUnpackProgressCallback *callback)
{
	// Only external use
	if (InternalUse || BigFileName.empty()) return false;

	FILE *bnp = nlfopen (BigFileName, "rb");
	if (bnp == NULL)
		return false;

	// only read header is not already read
	if (SFiles.empty() && !readHeader(bnp))
	{
		fclose (bnp);
		return false;
	}

	CFile::createDirectory(sDestDir);

	uint32 totalUncompressed = 0, total = 0;

	for (uint32 i = 0; i < SFiles.size(); ++i)
	{
		total += SFiles[i].Size;
	}

	FILE *out = NULL;

	for (uint32 i = 0; i < SFiles.size(); ++i)
	{
		const SBNPFile &rBNPFile = SFiles[i];
		string filename = CPath::standardizePath(sDestDir) + rBNPFile.Name;

		if (callback && !(*callback)(filename, totalUncompressed, total))
		{
			fclose (bnp);
			return false;
		}

		out = nlfopen (filename, "wb");
		if (out != NULL)
		{
			nlfseek64 (bnp, rBNPFile.Pos, SEEK_SET);
			uint8 *ptr = new uint8[rBNPFile.Size];
			bool readError = fread (ptr, rBNPFile.Size, 1, bnp) != 1;
			if (readError)
			{
				nlwarning("%s read error errno = %d: %s", filename.c_str(), errno, strerror(errno));
			}
			bool writeError = fwrite (ptr, rBNPFile.Size, 1, out) != 1;
			if (writeError)
			{
				nlwarning("%s write error errno = %d: %s", filename.c_str(), errno, strerror(errno));
			}
			bool diskFull = ferror(out) && errno == 28 /* ENOSPC*/;
			fclose (out);
			delete [] ptr;
			if (diskFull)
			{
				fclose (bnp);
				throw NLMISC::EDiskFullError(filename);
			}
			if (writeError)
			{
				fclose (bnp);
				throw NLMISC::EWriteError(filename);
			}
			if (readError)
			{
				fclose (bnp);
				throw NLMISC::EReadError(filename);
			}
		}

		totalUncompressed += rBNPFile.Size;

		if (callback && !(*callback)(filename, totalUncompressed, total))
		{
			fclose (bnp);
			return false;
		}
	}

	fclose (bnp);
	return true;
}

// ***************************************************************************
bool CBigFile::isBigFileAdded(const std::string &sBigFileName) const
{
	// Is already the same bigfile name ?
	string bigfilenamealone = CFile::getFilename (sBigFileName);
	return _BNPs.find(bigfilenamealone) != _BNPs.end();
}

// ***************************************************************************
std::string CBigFile::getBigFileName(const std::string &sBigFileName) const
{
	string bigfilenamealone = CFile::getFilename (sBigFileName);
	map<string, BNP>::const_iterator it = _BNPs.find(bigfilenamealone);
	if (it != _BNPs.end())
		return it->second.BigFileName;
	else
		return "";
}


// ***************************************************************************
void CBigFile::list (const std::string &sBigFileName, std::vector<std::string> &vAllFiles)
{
	string lwrFileName = toLowerAscii(sBigFileName);
	if (_BNPs.find (lwrFileName) == _BNPs.end())
		return;
	vAllFiles.clear ();
	BNP &rbnp = _BNPs.find (lwrFileName)->second;
	vector<BNPFile>::iterator it = rbnp.Files.begin();
	while (it != rbnp.Files.end())
	{
		vAllFiles.push_back (string(it->Name)); // Add the name of the file to the return vector
		++it;
	}
}

// ***************************************************************************
void CBigFile::removeAll ()
{
	while (_BNPs.begin() != _BNPs.end())
	{
		remove (_BNPs.begin()->first);
	}
}

struct CBNPFileComp
{
	bool operator()(const CBigFile::BNPFile &f, const CBigFile::BNPFile &s )
	{
		return strcmp( f.Name, s.Name ) < 0;
	}
};

// ***************************************************************************
bool CBigFile::getFileInternal (const std::string &sFileName, BNP *&zeBnp, BNPFile *&zeBnpFile)
{
	string zeFileName, zeBigFileName, lwrFileName = toLowerAscii(sFileName);
	string::size_type i, nPos = sFileName.find ('@');
	if (nPos == string::npos)
	{
		return false;
	}

	for (i = 0; i < nPos; ++i)
		zeBigFileName += lwrFileName[i];
	++i; // Skip @
	for (; i < lwrFileName.size(); ++i)
		zeFileName += lwrFileName[i];

	if (_BNPs.find (zeBigFileName) == _BNPs.end())
	{
		return false;
	}

	BNP &rbnp = _BNPs.find (zeBigFileName)->second;
	if (rbnp.Files.empty())
	{
		return false;
	}

	vector<BNPFile>::iterator itNBPFile;

	BNPFile temp_bnp_file;
	temp_bnp_file.Name = (char*)zeFileName.c_str();
	itNBPFile = lower_bound(rbnp.Files.begin(), rbnp.Files.end(), temp_bnp_file, CBNPFileComp());

	if (itNBPFile != rbnp.Files.end())
	{
		if (strcmp(itNBPFile->Name, zeFileName.c_str()) != 0)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	BNPFile &rbnpfile = *itNBPFile;

	// set ptr on found bnp/bnpFile
	zeBnp= &rbnp;
	zeBnpFile= &rbnpfile;

	return true;
}

// ***************************************************************************
FILE* CBigFile::getFile (const std::string &sFileName, uint32 &rFileSize,
						 uint32 &rBigFileOffset, bool &rCacheFileOnOpen, bool &rAlwaysOpened)
{
	BNP		*bnp= NULL;
	BNPFile	*bnpFile= NULL;
	if(!getFileInternal(sFileName, bnp, bnpFile))
	{
		nlwarning ("BF: Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}
	nlassert(bnp && bnpFile);

	// Get a ThreadSafe handle on the file
	CHandleFile		&handle= _ThreadFileArray.get(bnp->ThreadFileId);
	/* If not opened, open it now. There is 2 reason for it to be not opened:
		rbnp.AlwaysOpened==false, or it is a new thread which use it for the first time.
	*/
	if(handle.File== NULL)
	{
		handle.File = nlfopen (bnp->BigFileName, "rb");
		if (handle.File == NULL)
		{
			nlwarning ("bnp: can't fopen big file '%s' error %d '%s'", bnp->BigFileName.c_str(), errno, strerror(errno));
			return NULL;
		}
	}

	rCacheFileOnOpen = bnp->CacheFileOnOpen;
	rAlwaysOpened = bnp->AlwaysOpened;
	rBigFileOffset = bnpFile->Pos;
	rFileSize = bnpFile->Size;
	return handle.File;
}

// ***************************************************************************
bool CBigFile::getFileInfo (const std::string &sFileName, uint32 &rFileSize, uint32 &rBigFileOffset)
{
	BNP		*bnp= NULL;
	BNPFile	*bnpFile= NULL;
	if(!getFileInternal(sFileName, bnp, bnpFile))
	{
		nlwarning ("BF: Couldn't find '%s' for info", sFileName.c_str());
		return false;
	}
	nlassert(bnp && bnpFile);

	// get infos
	rBigFileOffset = bnpFile->Pos;
	rFileSize = bnpFile->Size;
	return true;
}

// ***************************************************************************
char *CBigFile::getFileNamePtr(const std::string &sFileName, const std::string &sBigFileName)
{
	string bigfilenamealone = CFile::getFilename (sBigFileName);
	if (_BNPs.find(bigfilenamealone) != _BNPs.end())
	{
		BNP &rbnp = _BNPs.find (bigfilenamealone)->second;
		vector<BNPFile>::iterator itNBPFile;
		if (rbnp.Files.empty())
			return NULL;
		string lwrFileName = toLowerAscii(sFileName);

		BNPFile temp_bnp_file;
		temp_bnp_file.Name = (char*)lwrFileName.c_str();
		itNBPFile = lower_bound(rbnp.Files.begin(), rbnp.Files.end(), temp_bnp_file, CBNPFileComp());

		if (itNBPFile != rbnp.Files.end())
		{
			if (strcmp(itNBPFile->Name, lwrFileName.c_str()) == 0)
			{
				return itNBPFile->Name;
			}
		}
	}

	return NULL;
}

// ***************************************************************************
void CBigFile::getBigFilePaths(std::vector<std::string> &bigFilePaths)
{
	bigFilePaths.clear();
	for(std::map<std::string, BNP>::iterator it = _BNPs.begin(); it != _BNPs.end(); ++it)
	{
		bigFilePaths.push_back(it->second.BigFileName);
	}
}

// ***************************************************************************
bool CBigFile::unpack(const std::string &sBigFileName, const std::string &sDestDir, TUnpackProgressCallback *callback)
{
	BNP bnpFile;
	bnpFile.BigFileName = sBigFileName;
	return bnpFile.unpack(sDestDir, callback);
}

} // namespace NLMISC
