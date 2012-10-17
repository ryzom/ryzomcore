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
#include <windows.h>

#include "nel/misc/common.h"

// ---------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

// ---------------------------------------------------------------------------
void CTools::mkdir (const string &dirName)
{
	if (dirName == "")
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
	newDir = "";
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
}

// ---------------------------------------------------------------------------
void CTools::chdir (const std::string &newDir)
{
	if (!SetCurrentDirectory (newDir.c_str()))
		throwError ((newDir+" : ").c_str ());
}

// ---------------------------------------------------------------------------
std::string CTools::pwd ()
{
	char sTmp[512];
	if (GetCurrentDirectory (512, sTmp) == 0)
	{
		throwError ("Get current directory : ");
	}
	string sTmp2 = sTmp;
	return sTmp2;
}

// ---------------------------------------------------------------------------
std::string CTools::normalizePath (const std::string &path)
{
	// Convert slash to anti-slash
	string retPath = path;
	for (uint32 i = 0; i < retPath.size(); ++i)
		if (retPath[i] == '/')
			retPath[i] = '\\';
	return retPath;
}

// ---------------------------------------------------------------------------
bool CTools::fileExist (const std::string &sFileName)
{
	HANDLE hFile = CreateFile (sFileName.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	CloseHandle (hFile);
	return true;
}

// ---------------------------------------------------------------------------
int CTools::fileDateCmp (const std::string &file1, const std::string &file2)
{
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
}

// ---------------------------------------------------------------------------
int CTools::fileDateCmp (const std::string &file1, uint32 nDateLow, uint32 nDateHigh)
{
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
}

// ---------------------------------------------------------------------------
void CTools::dir (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
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
}

// ---------------------------------------------------------------------------
void CTools::dirSub (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
	sAllFiles.clear();
	string sTmp;
	for (uint32 i = 0; i < sFilter.size(); ++i)
	if (sFilter[i] != '*')
		sTmp += sFilter[i];
	dirSubRecurse (sTmp, sAllFiles, bFullPath);
}

// ---------------------------------------------------------------------------
void CTools::copy (const std::string &DstFile, const std::string &SrcFile)
{
	if (!CopyFile (SrcFile.c_str(), DstFile.c_str(), false))
	{
		throw Exception(string("Cannot copy ")+SrcFile+" to "+DstFile);
	}
}

// *******
// PRIVATE
// *******

// ---------------------------------------------------------------------------
void CTools::throwError (const char *message)
{
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
}

// ---------------------------------------------------------------------------
void CTools::dirSubRecurse (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
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
}

