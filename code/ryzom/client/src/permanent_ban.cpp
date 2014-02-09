// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "permanent_ban.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/big_file.h"
#include "nel/misc/random.h"


using namespace NLMISC;


#define REGKEY_RYZOM_PATH "Software\\Nevrax\\ryzom"
#define REGKEY_PERMANENT_BAN "PB"


// ************************************************************
static void setPermanentBanRegistryKey(bool on)
{
#ifndef NL_OS_WINDOWS
	nlinfo("Not implemented");
#else
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_RYZOM_PATH, &hKey)==ERROR_SUCCESS)
	{
		DWORD permanentBan = on ? 1 : 0;
		LONG result = RegSetValueEx(hKey, REGKEY_PERMANENT_BAN, 0, REG_DWORD, (LPBYTE)&permanentBan, 4);
		if (result != ERROR_SUCCESS)
		{
			nlwarning("pb key not created");
		}
	}
	else
	{
		nlwarning("pb key not created");
	}
#endif
}

// ************************************************************
static bool getPermanentBanRegistryKey()
{
#ifndef NL_OS_WINDOWS
	return false; // not implemented
#else
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_RYZOM_PATH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD permanentBan;
		DWORD type;
		DWORD dataSize = sizeof(DWORD);
		RegQueryValueEx (hKey, REGKEY_PERMANENT_BAN, 0, &type, (LPBYTE)&permanentBan, &dataSize);
		if (type == REG_DWORD && dataSize == sizeof(DWORD))
		{
			return permanentBan != 0;
		}
	}
	return false;
#endif
}

// ***********************************************************
static void setPermanentBanFileMarker(const std::string &path, bool on)
{
	if (on)
	{
		try
		{
			// simply touch a file
			COFile f(path);
			#ifdef NL_OS_WINDOWS
				SetFileAttributes(path.c_str(), FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM);
			#endif
		}
		catch(const EStream &e)
		{
			nlinfo(e.what());
		}
	}
	else
	{
		CFile::deleteFile(path);
	}
}

// ***********************************************************
static bool getPermanentBanFileMarker(const std::string &path)
{
	return CFile::isExists(path);
}

#define PERMANENT_BAN_FILE0 "c:\\3289763c1ecd044e"
#define PERMANENT_BAN_FILE1 "78d0732e50bf2bbd"

// ************************************************************
void setPermanentBanMarkers(bool on)
{
	setPermanentBanRegistryKey(on);
	setPermanentBanFileMarker(PERMANENT_BAN_FILE0, on);
	setPermanentBanFileMarker(CPath::getWindowsDirectory() + PERMANENT_BAN_FILE1, on);
}


// ************************************************************
bool testPermanentBanMarkers()
{
	/*#ifndef NL_OS_WINDOWS
		nlinfo("Not implemented");
		return false;
	#else
		if (getPermanentBanRegistryKey()) return true;
		if (getPermanentBanFileMarker(PERMANENT_BAN_FILE0)) return true;
		if (getPermanentBanFileMarker(CPath::getWindowsDirectory() + PERMANENT_BAN_FILE1)) return true;
	#endif
	return false;
	*/
	return false;
}


// mark a bnp file without corrupting its datas (this force the patch to be applied the next time the client is launched, thus delaying the trouble maker)
static void markBNPFile(std::string &path)
{
	CRandom rnd;
	rnd.srand((sint32) CTime::getLocalTime());

	uint32 nFileSize=CFile::getFileSize(path);
	if (!nFileSize) return;
	FILE *f = fopen(path.c_str(), "rb+");
	if (!f) return;
// Result
	if (nlfseek64 (f, nFileSize-4, SEEK_SET) != 0)
	{
		fclose (f);
		return;
	}

	uint32 nOffsetFromBeginning;
	if (fread (&nOffsetFromBeginning, sizeof(uint32), 1, f) != 1)
	{
		fclose (f);
		return;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nOffsetFromBeginning);
#endif

	if (nlfseek64 (f, nOffsetFromBeginning, SEEK_SET) != 0)
	{
		fclose (f);
		return;
	}

	// Read the file count
	uint32 nNbFile;
	if (fread (&nNbFile, sizeof(uint32), 1, f) != 1)
	{
		fclose (f);
		return;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nNbFile);
#endif

	for (uint32 i = 0; i < nNbFile; ++i)
	{
		char FileName[MAX_PATH];
		uint8 nStringSize;
		if (fread (&nStringSize, 1, 1, f) != 1)
		{
			fclose(f);
			return;
		}

		sint64 currPos = nlftell64(f);
		if (currPos < 0)
		{
			fclose(f);
			return;
		}
		if (fread (FileName, 1, nStringSize, f) != nStringSize)
		{
			fclose (f);
			return;
		}

		FileName[nStringSize] = 0;

		for(uint k = 0; k < nStringSize; ++k)
		{
			if (rnd.rand() & 1) FileName[k] = toupper(FileName[k]);
			else FileName[k] = tolower(FileName[k]);
		}


		if (nlfseek64 (f, currPos, SEEK_SET) != 0)
		{
			fclose (f);
			return;
		}


		// write shuffled version
		if (fwrite(FileName, 1, nStringSize, f) != nStringSize)
		{
			fclose(f);
			return;
		}
		fflush(f);

		uint32 nFileSize2;
		if (fread (&nFileSize2, sizeof(uint32), 1, f) != 1)
		{
			fclose (f);
			return;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nFileSize2);
#endif

		uint32 nFilePos;
		if (fread (&nFilePos, sizeof(uint32), 1, f) != 1)
		{
			fclose (f);
			return;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nFilePos);
#endif
	}

	fclose (f);
}

// ************************************************************
void applyPermanentBanPunishment()
{
	// go in the data directory & touch all bnp files so that the client should repatch all its datas
	std::vector<std::string> bigFilePaths;
	CBigFile::getInstance().getBigFilePaths(bigFilePaths);
	for(uint k = 0; k < bigFilePaths.size(); ++k)
	{
		markBNPFile(bigFilePaths[k]);
	}
}


