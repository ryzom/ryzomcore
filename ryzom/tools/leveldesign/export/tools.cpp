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

#include "tools.h"

#include "nel/misc/common.h"

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#else
#	include <cerrno>
#	include <cstring>
#	include "nel/misc/path.h"
#	include "nel/misc/debug.h"
#endif

// ---------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

#ifndef NL_OS_WINDOWS
namespace
{
	// Matches name against a pattern containing at most one '*' wildcard
	// (the only kind of pattern used by every CTools::dir/dirSub caller).
	bool matchWildcard (const std::string &name, const std::string &pattern)
	{
		string::size_type star = pattern.find ('*');
		if (star == string::npos)
			return nlstricmp (name, pattern) == 0;

		string prefix = pattern.substr (0, star);
		string suffix = pattern.substr (star + 1);
		if (name.size () < prefix.size () + suffix.size ())
			return false;

		return nlstricmp (name.substr (0, prefix.size ()), prefix) == 0 &&
			nlstricmp (name.substr (name.size () - suffix.size ()), suffix) == 0;
	}
}
#endif // NL_OS_WINDOWS

// ---------------------------------------------------------------------------
void CTools::mkdir (const string &dirName)
{
#ifdef NL_OS_WINDOWS
	if (dirName.empty())
		return;
	// Does the directory exist ?
	string newDir = pwd();
	if (SetCurrentDirectory (dirName.c_str()))
	{
		SetCurrentDirectory (newDir.c_str());
		return;
	}
	SetCurrentDirectory (newDir.c_str());
	// Create upper levels
	newDir.clear();
	string::size_type pos = dirName.rfind('\\');
	if (pos != string::npos)
	{
		for (uint i = 0; i < pos; ++i)
			newDir += dirName[i];
		mkdir (newDir);
	}
	// Create Directory
	if (!CreateDirectory(dirName.c_str(),NULL))
		throw Exception(string("Cannot create directory ")+dirName);
#else
	if (dirName.empty())
		return;
	if (!CFile::createDirectoryTree (dirName))
		throw Exception (string ("Cannot create directory ") + dirName);
#endif
}

// ---------------------------------------------------------------------------
void CTools::chdir (const std::string &newDir)
{
#ifdef NL_OS_WINDOWS
	if (!SetCurrentDirectory (newDir.c_str()))
		throwError ((newDir+" : ").c_str ());
#else
	if (!CPath::setCurrentPath (newDir))
		throwError ((newDir + " : ").c_str ());
#endif
}

// ---------------------------------------------------------------------------
std::string CTools::pwd ()
{
#ifdef NL_OS_WINDOWS
	char sTmp[512];
	if (GetCurrentDirectory (512, sTmp) == 0)
	{
		throwError ("Get current directory : ");
	}
	string sTmp2 = sTmp;
	return sTmp2;
#else
	return CPath::getCurrentPath ();
#endif
}

// ---------------------------------------------------------------------------
std::string CTools::normalizePath (const std::string &path)
{
#ifdef NL_OS_WINDOWS
	// Convert slash to anti-slash
	string retPath = path;
	for (uint32 i = 0; i < retPath.size(); ++i)
		if (retPath[i] == '/')
			retPath[i] = '\\';
	return retPath;
#else
	// Convert anti-slash to slash
	string retPath = path;
	for (uint32 i = 0; i < retPath.size (); ++i)
		if (retPath[i] == '\\')
			retPath[i] = '/';
	return retPath;
#endif
}

// ---------------------------------------------------------------------------
bool CTools::fileExist (const std::string &sFileName)
{
#ifdef NL_OS_WINDOWS
	HANDLE hFile = CreateFile (sFileName.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	CloseHandle (hFile);
	return true;
#else
	return CFile::fileExists (sFileName);
#endif
}

// ---------------------------------------------------------------------------
int CTools::fileDateCmp (const std::string &file1, const std::string &file2)
{
#ifdef NL_OS_WINDOWS
	HANDLE hFile1 = CreateFile (file1.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hFile2 = CreateFile (file2.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((hFile1 == INVALID_HANDLE_VALUE) && (hFile2 == INVALID_HANDLE_VALUE))
		return 0;
	if (hFile1 == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile2);
		return -1;
	}
	if (hFile2 == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile1);
		return 1;
	}

	FILETIME CreationTime1, LastAccessTime1, LastWrite1;
	GetFileTime (hFile1, &CreationTime1, &LastAccessTime1, &LastWrite1);
	FILETIME CreationTime2, LastAccessTime2, LastWrite2;
	GetFileTime (hFile2, &CreationTime2, &LastAccessTime2, &LastWrite2);

	LONG nRet = CompareFileTime (&LastWrite1, &LastWrite2);

	CloseHandle(hFile1);
	CloseHandle(hFile2);

	return nRet;
#else
	bool exist1 = CFile::fileExists (file1);
	bool exist2 = CFile::fileExists (file2);
	if (!exist1 && !exist2)
		return 0;
	if (!exist1)
		return -1;
	if (!exist2)
		return 1;

	uint32 date1 = CFile::getFileModificationDate (file1);
	uint32 date2 = CFile::getFileModificationDate (file2);
	if (date1 < date2)
		return -1;
	if (date1 > date2)
		return 1;
	return 0;
#endif
}

// ---------------------------------------------------------------------------
int CTools::fileDateCmp (const std::string &file1, uint32 nDateLow, uint32 nDateHigh)
{
#ifdef NL_OS_WINDOWS
	HANDLE hFile1 = CreateFile (file1.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile1 == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	FILETIME CreationTime1, LastAccessTime1, LastWrite1;
	GetFileTime (hFile1, &CreationTime1, &LastAccessTime1, &LastWrite1);
	FILETIME Date;
	Date.dwLowDateTime = nDateLow;
	Date.dwHighDateTime = nDateHigh;
	LONG nRet = CompareFileTime (&LastWrite1, &Date);

	CloseHandle(hFile1);
	return nRet;
#else
	if (!CFile::fileExists (file1))
		return -1;

	// Convert the Windows FILETIME (100ns intervals since 1601-01-01) into
	// a Unix epoch (seconds since 1970-01-01) to compare against
	// getFileModificationDate(), since this method only ever receives raw
	// FILETIME components read back from persisted zone-region data.
	uint64 fileTime = ((uint64) nDateHigh << 32) | (uint64) nDateLow;
	uint32 date2 = (uint32) ((fileTime - 116444736000000000ULL) / 10000000ULL);
	uint32 date1 = CFile::getFileModificationDate (file1);

	if (date1 < date2)
		return -1;
	if (date1 > date2)
		return 1;
	return 0;
#endif
}

// ---------------------------------------------------------------------------
void CTools::dir (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
#ifdef NL_OS_WINDOWS
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	sAllFiles.clear ();
	GetCurrentDirectory (MAX_PATH, sCurDir);
	hFind = FindFirstFile (sFilter.c_str(), &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(GetFileAttributes(findData.cFileName)&FILE_ATTRIBUTE_DIRECTORY))
		{
			if (bFullPath)
				sAllFiles.push_back(string(sCurDir) + "\\" + findData.cFileName);
			else
				sAllFiles.push_back(findData.cFileName);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
#else
	sAllFiles.clear ();
	string curDir = CPath::getCurrentPath ();
	vector<string> content;
	CPath::getPathContent (curDir, false, false, true, content);
	for (uint i = 0; i < content.size (); ++i)
	{
		string name = CFile::getFilename (content[i]);
		if (matchWildcard (name, sFilter))
			sAllFiles.push_back (bFullPath ? content[i] : name);
	}
#endif
}

// ---------------------------------------------------------------------------
void CTools::dirSub (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
#ifdef NL_OS_WINDOWS
	sAllFiles.clear();
	string sTmp;
	for (uint32 i = 0; i < sFilter.size(); ++i)
	if (sFilter[i] != '*')
		sTmp += sFilter[i];
	dirSubRecurse (sTmp, sAllFiles, bFullPath);
#else
	sAllFiles.clear ();
	string curDir = CPath::getCurrentPath ();
	vector<string> content;
	CPath::getPathContent (curDir, true, false, true, content);
	for (uint i = 0; i < content.size (); ++i)
	{
		string name = CFile::getFilename (content[i]);
		if (matchWildcard (name, sFilter))
			sAllFiles.push_back (bFullPath ? content[i] : name);
	}
#endif
}

// ---------------------------------------------------------------------------
void CTools::copy (const std::string &DstFile, const std::string &SrcFile)
{
#ifdef NL_OS_WINDOWS
	if (!CopyFile (SrcFile.c_str(), DstFile.c_str(), false))
	{
		throw Exception(string("Cannot copy ")+SrcFile+" to "+DstFile);
	}
#else
	if (!CFile::copyFile (DstFile, SrcFile))
	{
		throw Exception (string ("Cannot copy ") + SrcFile + " to " + DstFile);
	}
#endif
}

// *******
// PRIVATE
// *******

// ---------------------------------------------------------------------------
void CTools::throwError (const char *message)
{
#ifdef NL_OS_WINDOWS
	LPVOID lpMsgBuf;
	FormatMessage (	FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL );
	string sTmp = (char*)lpMsgBuf;
	sTmp = message + sTmp;
	LocalFree (lpMsgBuf);
	throw Exception (sTmp);
#else
	string sTmp = message + string (strerror (errno));
	throw Exception (sTmp);
#endif
}

// ---------------------------------------------------------------------------
void CTools::dirSubRecurse (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
#ifdef NL_OS_WINDOWS
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];

	GetCurrentDirectory (MAX_PATH, sCurDir);
	hFind = FindFirstFile ("*.*", &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!(GetFileAttributes(findData.cFileName)&FILE_ATTRIBUTE_DIRECTORY))
		{
			string sTmp = findData.cFileName;
			if (sTmp.size() > sFilter.size())
			if (strcmp(sTmp.c_str()+sTmp.size()-sFilter.size(), sFilter.c_str()) == 0)
			{
				if (bFullPath)
					sAllFiles.push_back(string(sCurDir) + "\\" + findData.cFileName);
				else
					sAllFiles.push_back(findData.cFileName);
			}
		}
		else if ((strcmp(findData.cFileName, ".") != 0) && (strcmp(findData.cFileName, "..") != 0))
		{
			SetCurrentDirectory (findData.cFileName);
			dirSubRecurse (sFilter, sAllFiles, bFullPath);
			SetCurrentDirectory (sCurDir);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
#else
	// Unused on non-Windows: dirSub() implements its own recursion directly
	// via CPath::getPathContent(recurse=true) instead of calling this helper.
	nlwarning ("CTools::dirSubRecurse is a Windows-only implementation detail, not implemented on this platform");
#endif
}
