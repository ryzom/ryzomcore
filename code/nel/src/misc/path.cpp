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


#include "stdmisc.h"

#include "nel/misc/path.h"
#include "nel/misc/big_file.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/progress_callback.h"
#include "nel/misc/file.h"
#include "nel/misc/xml_pack.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <direct.h>
#	include <io.h>
#	include <fcntl.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <shlobj.h>
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#	include <dirent.h>
#   include <unistd.h>
#	include <cstdio>
#   include <cerrno>
#   include <sys/types.h>
#   include <utime.h>
#endif // NL_OS_WINDOWS

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

//
// Macros
//

// Use this define if you want to display info about the CPath.
//#define	NL_DEBUG_PATH

#ifdef	NL_DEBUG_PATH
#	define	NL_DISPLAY_PATH	nlinfo
#else
#	ifdef __GNUC__
#		define	NL_DISPLAY_PATH(format, args...)
#	else // __GNUC__
#		define	NL_DISPLAY_PATH if(false)
#	endif // __GNUC__
#endif


//
// Variables
//

NLMISC_SAFE_SINGLETON_IMPL(CPath);


//
// Functions
//

CFileContainer::~CFileContainer()
{
	if( _AllFileNames )
	{
		delete[] _AllFileNames;
		_AllFileNames = NULL;
	}
}

void CPath::releaseInstance()
{
	if (_Instance)
	{
		NLMISC::INelContext::getInstance().releaseSingletonPointer("CPath", _Instance);
		delete _Instance;
		_Instance = NULL;
	}
}


void CPath::getFileList(const std::string &extension, std::vector<std::string> &filenames)
{
	getInstance()->_FileContainer.getFileList(extension, filenames);
}

void CFileContainer::getFileList(const std::string &extension, std::vector<std::string> &filenames)
{
	if (!_MemoryCompressed)
	{
		TFiles::iterator first(_Files.begin()), last(_Files.end());

		if( !extension.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = SSMext.get(first->second.idExt);
				if (ext == extension)
				{
					filenames.push_back(first->first);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
			{
				filenames.push_back(first->first);
			}
		}
	}
	else
	{
		// compressed memory version
		std::vector<CFileContainer::CMCFileEntry>::iterator first(_MCFiles.begin()), last(_MCFiles.end());

		if( !extension.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = SSMext.get(first->idExt);
				if (ext == extension)
				{
					filenames.push_back(first->Name);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
			{
				filenames.push_back(first->Name);
			}
		}
	}
}

void CPath::getFileListByName(const std::string &extension, const std::string &name, std::vector<std::string> &filenames)
{
	getInstance()->_FileContainer.getFileListByName(extension, name, filenames);
}

void CFileContainer::getFileListByName(const std::string &extension, const std::string &name, std::vector<std::string> &filenames)
{
	if (!_MemoryCompressed)
	{
		TFiles::iterator first(_Files.begin()), last(_Files.end());

		if( !name.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = SSMext.get(first->second.idExt);
				if (first->first.find(name) != string::npos && (ext == extension || extension.empty()))
				{
					filenames.push_back(first->first);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
			{
				filenames.push_back(first->first);
			}
		}
	}
	else
	{
		// compressed memory version
		std::vector<CFileContainer::CMCFileEntry>::iterator first(_MCFiles.begin()), last(_MCFiles.end());

		if( !name.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = SSMext.get(first->idExt);
				if (strstr(first->Name, name.c_str()) != NULL && (ext == extension || extension.empty()))
				{
					filenames.push_back(first->Name);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
		{
				filenames.push_back(first->Name);
			}
		}
	}
}

void CPath::getFileListByPath(const std::string &extension, const std::string &path, std::vector<std::string> &filenames)
{
	getInstance()->_FileContainer.getFileListByPath(extension, path, filenames);
}

void CFileContainer::getFileListByPath(const std::string &extension, const std::string &path, std::vector<std::string> &filenames)
{
	if (!_MemoryCompressed)
	{
		TFiles::iterator first(_Files.begin()), last(_Files.end());

		if( !path.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = SSMext.get(first->second.idExt);
				string p = SSMpath.get(first->second.idPath);
				if (p.find(path) != string::npos && (ext == extension || extension.empty()))
				{
					filenames.push_back(first->first);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
			{
				filenames.push_back(first->first);
			}
		}
	}
	else
	{
		// compressed memory version
		std::vector<CFileContainer::CMCFileEntry>::iterator first(_MCFiles.begin()), last(_MCFiles.end());

		if( !path.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = SSMext.get(first->idExt);
				string p = SSMpath.get(first->idPath);
				
				if (strstr(p.c_str(), path.c_str()) != NULL && (ext == extension || extension.empty()))
				{
					filenames.push_back(first->Name);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
		{
				filenames.push_back(first->Name);
			}
		}
	}
}

void CPath::clearMap ()
{
	getInstance()->_FileContainer.clearMap();
}

void CFileContainer::clearMap ()
{
	nlassert(!_MemoryCompressed);
	_Files.clear ();
	CBigFile::getInstance().removeAll ();
	NL_DISPLAY_PATH("PATH: CPath::clearMap(): map directory cleared");
}

CFileContainer::CMCFileEntry *CFileContainer::MCfind (const std::string &filename)
{
	nlassert(_MemoryCompressed);
	vector<CMCFileEntry>::iterator it;
	CMCFileEntry temp_cmc_file;
	temp_cmc_file.Name = (char*)filename.c_str();
	it = lower_bound(_MCFiles.begin(), _MCFiles.end(), temp_cmc_file, CMCFileComp());
	if (it != _MCFiles.end())
	{
		CMCFileComp FileComp;
		if (FileComp.specialCompare(*it, filename.c_str()) == 0)
			return &(*it);
	}
	return NULL;
}

sint CFileContainer::findExtension (const string &ext1, const string &ext2)
{
	for (uint i = 0; i < _Extensions.size (); i++)
	{
		if (_Extensions[i].first == ext1 && _Extensions[i].second == ext2)
		{
			return i;
		}
	}
	return -1;
}

void CPath::remapExtension (const string &ext1, const string &ext2, bool substitute)
{
	getInstance()->_FileContainer.remapExtension(ext1, ext2, substitute);
}

void CFileContainer::remapExtension (const string &ext1, const string &ext2, bool substitute)
{
	nlassert(!_MemoryCompressed);

	string ext1lwr = toLower(ext1);
	string ext2lwr = toLower(ext2);

	if (ext1lwr.empty() || ext2lwr.empty())
	{
		nlwarning ("PATH: CPath::remapExtension(%s, %s, %d): can't remap empty extension", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}

	if (ext1lwr == "bnp" || ext2lwr == "bnp")
	{
		nlwarning ("PATH: CPath::remapExtension(%s, %s, %d): you can't remap a big file", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}

	if (!substitute)
	{
		// remove the mapping from the mapping list
		sint n = findExtension (ext1lwr, ext2lwr);
		nlassert (n != -1);
		_Extensions.erase (_Extensions.begin() + n);

		// remove mapping in the map
		TFiles::iterator it = _Files.begin();
		TFiles::iterator nit = it;
		while (it != _Files.end ())
		{
			nit++;
			string ext = SSMext.get((*it).second.idExt);
			if ((*it).second.Remapped && ext == ext2lwr)
			{
				_Files.erase (it);
			}
			it = nit;
		}
		NL_DISPLAY_PATH("PATH: CPath::remapExtension(%s, %s, %d): extension removed", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}
	else
	{
		sint n = findExtension (ext1lwr, ext2lwr);
		if (n != -1)
		{
			nlwarning ("PATH: CPath::remapExtension(%s, %s, %d): remapping already set", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
			return;
		}

		// adding mapping into the mapping list
		_Extensions.push_back (make_pair (ext1lwr, ext2lwr));

		// adding mapping into the map
		vector<string> newFiles;
		TFiles::iterator it = _Files.begin();
		while (it != _Files.end ())
		{
			string ext = SSMext.get((*it).second.idExt);
			if (!(*it).second.Remapped && ext == ext1lwr)
			{
				// find if already exist
				string::size_type pos = (*it).first.find_last_of (".");
				if (pos != string::npos)
				{
					string file = (*it).first.substr (0, pos + 1);
					file += ext2lwr;

// TODO perhaps a problem because I insert in the current map that I process
					string path = SSMpath.get((*it).second.idPath);
					insertFileInMap (file, path+file, true, ext1lwr);
				}
			}
			it++;
		}
		NL_DISPLAY_PATH("PATH: CPath::remapExtension(%s, %s, %d): extension added", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}
}

// ***************************************************************************
void CPath::remapFile (const std::string &file1, const std::string &file2)
{
	getInstance()->_FileContainer.remapFile(file1, file2);
}

void CFileContainer::remapFile (const std::string &file1, const std::string &file2)
{
	if (file1.empty()) return;
	if (file2.empty()) return;
	_RemappedFiles[toLower(file1)] = toLower(file2);
}

// ***************************************************************************
static void removeAllUnusedChar(string &str)
{
	uint32 i = 0;
	while (!str.empty() && (i != str.size()))
	{
		if ((str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'))
			str.erase(str.begin()+i);
		else
			i++;
	}
}

// ***************************************************************************
void CPath::loadRemappedFiles (const std::string &file)
{
	getInstance()->_FileContainer.loadRemappedFiles(file);
}

void CFileContainer::loadRemappedFiles (const std::string &file)
{
	string fullName = lookup(file, false, true, true);
	CIFile f;
	f.setCacheFileOnOpen (true);

	if (!f.open (fullName))
		return;

	char sTmp[514];
	string str;

	while (!f.eof())
	{
		f.getline(sTmp, 512);
		str = sTmp;
		std::string::size_type pos = str.find(',');
		if (pos != string::npos)
		{
			removeAllUnusedChar(str);
			if (!str.empty())
			{
				pos = str.find(',');
				remapFile( str.substr(0,pos), str.substr(pos+1, str.size()) );
			}
		}
	}
}


// ***************************************************************************
string CPath::lookup (const string &filename, bool throwException, bool displayWarning, bool lookupInLocalDirectory)
{
	return getInstance()->_FileContainer.lookup(filename, throwException, displayWarning, lookupInLocalDirectory);
}

string CFileContainer::lookup (const string &filename, bool throwException, bool displayWarning, bool lookupInLocalDirectory)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	/*
		NB: CPath access static instance getInstance() of course, so user must ensure
		that no mutator is called while async loading
	*/


	// If the file already contains a @, it means that a lookup already proceed and returning a big file, do nothing
	if (filename.find ("@") != string::npos)
	{
		NL_DISPLAY_PATH("PATH: CPath::lookup(%s):	already found", filename.c_str());
		return filename;
	}

	// Try to find in the map directories

	// If filename contains a path, we get only the filename to look inside paths
	string str = CFile::getFilename(toLower(filename));

	// Remove end spaces
	while ((!str.empty()) && (str[str.size()-1] == ' '))
	{
		str.resize (str.size()-1);
	}

	map<string, string>::iterator itss = _RemappedFiles.find(str);
	if (itss != _RemappedFiles.end())
		str = itss->second;

	if (_MemoryCompressed)
	{
		CMCFileEntry *pMCFE = MCfind(str);
		// If found in the map, returns it
		if (pMCFE != NULL)
		{
			string fname, path = SSMpath.get(pMCFE->idPath);
			if (pMCFE->Remapped)
				fname = CFile::getFilenameWithoutExtension(pMCFE->Name) + "." + SSMext.get(pMCFE->idExt);
			else
				fname = pMCFE->Name;

			NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the map directory: '%s'", fname.c_str(), path.c_str());
			return path + fname;
		}
	}
	else // NOT memory compressed
	{

		TFiles::iterator it = _Files.find (str);
		// If found in the map, returns it
		if (it != _Files.end())
		{
			string fname, path = SSMpath.get((*it).second.idPath);
			if (it->second.Remapped)
				fname = CFile::getFilenameWithoutExtension((*it).second.Name) + "." + SSMext.get((*it).second.idExt);
			else
				fname = (*it).second.Name;

			NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the map directory: '%s'", fname.c_str(), path.c_str());
			return path + fname;
		}
	}


	// Try to find in the alternative directories
	for (uint i = 0; i < _AlternativePaths.size(); i++)
	{
		string s = _AlternativePaths[i] + str;
		if ( CFile::fileExists(s) )
		{
			NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the alternative directory: '%s'", str.c_str(), s.c_str());
			return s;
		}

		// try with the remapping
		for (uint j = 0; j < _Extensions.size(); j++)
		{
			if (toLower(CFile::getExtension (str)) == _Extensions[j].second)
			{
				string rs = _AlternativePaths[i] + CFile::getFilenameWithoutExtension (filename) + "." + _Extensions[j].first;
				if ( CFile::fileExists(rs) )
				{
					NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the alternative directory: '%s'", str.c_str(), rs.c_str());
					return rs;
				}
			}
		}
	}

	// Try to find in the current directory
	if ( lookupInLocalDirectory && CFile::fileExists(filename) )
	{
		NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the current directory: '%s'", str.c_str(), filename.c_str());
		return filename;
	}

	// Not found
	if (displayWarning)
	{
		if(filename.empty())
			nlwarning ("PATH: Try to lookup for an empty filename. TODO: check why.");
		else
			nlwarning ("PATH: File (%s) not found (%s)", str.c_str(), filename.c_str());
	}

	if (throwException)
		throw EPathNotFound (filename);

	return "";
}

bool CPath::exists (const std::string &filename)
{
	return getInstance()->_FileContainer.exists(filename);
}

bool CFileContainer::exists (const std::string &filename)
{
	// Try to find in the map directories
	string str = toLower(filename);

	// Remove end spaces
	while ((!str.empty()) && (str[str.size()-1] == ' '))
	{
		str.resize (str.size()-1);
	}

	if (_MemoryCompressed)
	{
		CMCFileEntry *pMCFE = MCfind(str);
		// If found in the vector, returns it
		if (pMCFE != NULL)
			return true;
	}
	else
	{
		TFiles::iterator it = _Files.find (str);
		// If found in the map, returns it
		if (it != _Files.end())
			return true;
	}

	return false;
}

string CPath::standardizePath (const string &path, bool addFinalSlash)
{
	return getInstance()->_FileContainer.standardizePath(path, addFinalSlash);
}

string CFileContainer::standardizePath (const string &path, bool addFinalSlash)
{
	// check empty path
	if (path.empty())
		return "";

	string newPath(path);

	for (uint i = 0; i < path.size(); i++)
	{
		// don't transform the first \\ for windows network path
		if (path[i] == '\\')
			newPath[i] = '/';
	}

	// add terminal slash
	if (addFinalSlash && newPath[path.size()-1] != '/')
		newPath += '/';

	return newPath;
}

// replace / with backslash
std::string	CPath::standardizeDosPath (const std::string &path)
{
	return getInstance()->_FileContainer.standardizeDosPath(path);
}

std::string	CFileContainer::standardizeDosPath (const std::string &path)
{
	string newPath;

	for (uint i = 0; i < path.size(); i++)
	{
		if (path[i] == '/')
			newPath += '\\';
		// Yoyo: supress toLower. Not useful!?!
		/*else if (isupper(path[i]))
			newPath += tolower(path[i]);*/
		else
			newPath += path[i];
	}

	if (CFile::isExists(path) && CFile::isDirectory(path) && newPath[newPath.size()-1] != '\\')
		newPath += '\\';

	return newPath;
}


std::string CPath::getCurrentPath ()
{
	return getInstance()->_FileContainer.getCurrentPath();
}

std::string CFileContainer::getCurrentPath ()
{
	char buffer [1024];

#ifdef NL_OS_WINDOWS
	return standardizePath(_getcwd(buffer, 1024), false);
#else
	return standardizePath(getcwd(buffer, 1024), false);
#endif
}

bool CPath::setCurrentPath (const std::string &path)
{
	return getInstance()->_FileContainer.setCurrentPath(path);
}

bool CFileContainer::setCurrentPath (const std::string &path)
{
	int res;
	//nldebug("Change current path to '%s' (current path is '%s')", path.c_str(), getCurrentPath().c_str());
#ifdef NL_OS_WINDOWS
	res = _chdir(path.c_str());
#else
	res = chdir(path.c_str());
#endif
	if(res != 0) nlwarning("Cannot change current path to '%s' (current path is '%s') res: %d errno: %d (%s)", path.c_str(), getCurrentPath().c_str(), res, errno, strerror(errno));
	return res == 0;
}

std::string CPath::getFullPath (const std::string &path, bool addFinalSlash)
{
	return getInstance()->_FileContainer.getFullPath(path, addFinalSlash);
}

std::string CFileContainer::getFullPath (const std::string &path, bool addFinalSlash)
{
	string currentPath = standardizePath (getCurrentPath ());
	string sPath = standardizePath (path, addFinalSlash);

	// current path
	if (path.empty() || sPath == "." || sPath == "./")
	{
		return currentPath;
	}

	// windows full path
	if (path.size() >= 2 && path[1] == ':')
	{
		return sPath;
	}

	if (path.size() >= 2 && (path[0] == '/' || path[0] == '\\') && (path[1] == '/' || path[1] == '\\'))
	{
		return sPath;
	}


	// from root
	if (path [0] == '/' || path[0] == '\\')
	{
		if (currentPath.size() > 2 && currentPath[1] == ':')
		{
			return currentPath.substr(0,3) + sPath.substr(1);
		}
		else
		{
			return sPath;
		}
	}

	// default case
	return currentPath + sPath;
}



#ifdef NL_OS_WINDOWS
#	define dirent	WIN32_FIND_DATA
#	define DIR		void

static string sDir;
static WIN32_FIND_DATA findData;
static HANDLE hFind;

DIR *opendir (const char *path)
{
	nlassert (path != NULL);
	nlassert (path[0] != '\0');

	if (!CFile::isDirectory(path))
		return NULL;

	sDir = path;

	hFind = NULL;

	return (void *)1;
}

int closedir (DIR *dir)
{
	FindClose(hFind);
	return 0;
}

dirent *readdir (DIR *dir)
{
	// FIX : call to SetCurrentDirectory() and SetCurrentDirectory() removed to improve speed
	nlassert (!sDir.empty());

	// first visit in this directory : FindFirstFile()
	if (hFind == NULL)
	{
		string fullPath = CPath::standardizePath(sDir) + "*";
		hFind = FindFirstFileA (fullPath.c_str(), &findData);
	}
	// directory already visited : FindNextFile()
	else
	{
		if (!FindNextFileA (hFind, &findData))
			return NULL;
	}

	return &findData;
}


#endif // NL_OS_WINDOWS

#ifndef NL_OS_WINDOWS
string BasePathgetPathContent;
#endif

bool isdirectory (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return ((de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && ((de->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0);
#else
	//nlinfo ("isdirectory filename %s -> 0x%08x", de->d_name, de->d_type);
	// we can't use "de->d_type & DT_DIR" because it s always NULL on libc2.1
	//return (de->d_type & DT_DIR) != 0;

	return CFile::isDirectory (BasePathgetPathContent + de->d_name);

#endif // NL_OS_WINDOWS
}

bool isfile (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return ((de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) && ((de->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0);
#else
	// we can't use "de->d_type & DT_DIR" because it s always NULL on libc2.1
	//return (de->d_type & DT_DIR) == 0;

	return !CFile::isDirectory (BasePathgetPathContent + de->d_name);

#endif // NL_OS_WINDOWS
}

string getname (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return de->cFileName;
#else
	return de->d_name;
#endif // NL_OS_WINDOWS
}

void CPath::getPathContent (const string &path, bool recurse, bool wantDir, bool wantFile, vector<string> &result, class IProgressCallback *progressCallBack, bool showEverything)
{
	getInstance()->_FileContainer.getPathContent(path, recurse, wantDir, wantFile, result, progressCallBack, showEverything);
}

void CFileContainer::getPathContent (const string &path, bool recurse, bool wantDir, bool wantFile, vector<string> &result, class IProgressCallback *progressCallBack, bool showEverything)
{
	if(	path.empty() )
	{
		NL_DISPLAY_PATH("PATH: CPath::getPathContent(): Empty input Path");
		return;
	}

#ifndef NL_OS_WINDOWS
	BasePathgetPathContent = CPath::standardizePath (path);
#endif

	DIR *dir = opendir (path.c_str());

	if (dir == NULL)
	{
		NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): could not open the directory", path.c_str(), recurse, wantDir, wantFile);
		return;
	}

	// contains path that we have to recurs into
	vector<string> recursPath;

	for(;;)
	{
		dirent *de = readdir(dir);
		if (de == NULL)
		{
			NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): end of directory", path.c_str(), recurse, wantDir, wantFile);
			break;
		}

		string fn = getname (de);

		// skip . and ..
		if (fn == "." || fn == "..")
			continue;

		if (isdirectory(de))
		{
			// skip CVS and .svn directory
			if ((!showEverything) && (fn == "CVS" || fn == ".svn"))
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): skip CVS and .svn directory", path.c_str(), recurse, wantDir, wantFile);
				continue;
			}

			string stdName = standardizePath(standardizePath(path) + fn);
			if (recurse)
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): need to recurse into '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				recursPath.push_back (stdName);
			}

			if (wantDir)
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): adding path '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				result.push_back (stdName);
			}
		}

		if (wantFile && isfile(de))
		{
			if ( (!showEverything) && (fn.size() >= 4 && fn.substr (fn.size()-4) == ".log"))
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): skip *.log files (%s)", path.c_str(), recurse, wantDir, wantFile, fn.c_str());
				continue;
			}

			string stdName = standardizePath(path) + getname(de);

			NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): adding file '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
			result.push_back (stdName);
		}
	}

	closedir (dir);

#ifndef NL_OS_WINDOWS
	BasePathgetPathContent = "";
#endif

	// let s recurse
	for (uint i = 0; i < recursPath.size (); i++)
	{
		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->progress ((float)i/(float)recursPath.size ());
			progressCallBack->pushCropedValues ((float)i/(float)recursPath.size (), (float)(i+1)/(float)recursPath.size ());
		}

		getPathContent (recursPath[i], recurse, wantDir, wantFile, result, progressCallBack, showEverything);

		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->popCropedValues ();
		}
	}

	sort(result.begin(), result.end());
}

void CPath::removeAllAlternativeSearchPath ()
{
	getInstance()->_FileContainer.removeAllAlternativeSearchPath();
}

void CFileContainer::removeAllAlternativeSearchPath ()
{
	_AlternativePaths.clear ();
	NL_DISPLAY_PATH("PATH: CPath::RemoveAllAternativeSearchPath(): removed");
}


void CPath::addSearchPath (const string &path, bool recurse, bool alternative, class IProgressCallback *progressCallBack)
{
	getInstance()->_FileContainer.addSearchPath(path, recurse, alternative, progressCallBack);
}

void CFileContainer::addSearchPath (const string &path, bool recurse, bool alternative, class IProgressCallback *progressCallBack)
{
	//H_AUTO_INST(addSearchPath);

	nlassert(!_MemoryCompressed);

	// check empty directory
	if (path.empty())
	{
		nlwarning ("PATH: CPath::addSearchPath(%s, %s, %s): can't add empty directory, skip it",
			path.c_str(),
			recurse ? "recursive" : "not recursive",
			alternative ? "alternative" : "not alternative");
		return;
	}

	// check if it s a directory
	if (!CFile::isDirectory (path))
	{
		nlinfo ("PATH: CPath::addSearchPath(%s, %s, %s): '%s' is not a directory, I'll call addSearchFile()",
			path.c_str(),
			recurse ? "recursive" : "not recursive",
			alternative ? "alternative" : "not alternative",
			path.c_str());
		addSearchFile (path, false, "", progressCallBack);
		return;
	}

	string newPath = standardizePath(path);

	// check if it s a directory
	if (!CFile::isExists (newPath))
	{
		nlwarning ("PATH: CPath::addSearchPath(%s, %s, %s): '%s' is not found, skip it",
			path.c_str(),
			recurse ? "recursive" : "not recursive",
			alternative ? "alternative" : "not alternative",
			newPath.c_str());
		return;
	}

	nlinfo ("PATH: CPath::addSearchPath(%s, %d, %d): adding the path '%s'", path.c_str(), recurse, alternative, newPath.c_str());

	NL_DISPLAY_PATH("PATH: CPath::addSearchPath(%s, %d, %d): try to add '%s'", path.c_str(), recurse, alternative, newPath.c_str());

	if (alternative)
	{
		vector<string> pathsToProcess;

		// add the current path
		pathsToProcess.push_back (newPath);

		if (recurse)
		{
			// find all path and subpath
			getPathContent (newPath, recurse, true, false, pathsToProcess, progressCallBack);
		}

		for (uint p = 0; p < pathsToProcess.size(); p++)
		{
			// check if the path not already in the vector
			uint i;
			for (i = 0; i < _AlternativePaths.size(); i++)
			{
				if (_AlternativePaths[i] == pathsToProcess[p])
					break;
			}
			if (i == _AlternativePaths.size())
			{
				// add them in the alternative directory
				_AlternativePaths.push_back (pathsToProcess[p]);
				NL_DISPLAY_PATH("PATH: CPath::addSearchPath(%s, %s, %s): path '%s' added",
					newPath.c_str(),
					recurse ? "recursive" : "not recursive",
					alternative ? "alternative" : "not alternative",
					pathsToProcess[p].c_str());
			}
			else
			{
				nlwarning ("PATH: CPath::addSearchPath(%s, %s, %s): path '%s' already added",
					newPath.c_str(),
					recurse ? "recursive" : "not recursive",
					alternative ? "alternative" : "not alternative",
					pathsToProcess[p].c_str());
			}
		}
	}
	else
	{
		vector<string> filesToProcess;

		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->progress (0);
			progressCallBack->pushCropedValues (0, 0.5f);
		}

		// find all files in the path and subpaths
		getPathContent (newPath, recurse, false, true, filesToProcess, progressCallBack);

		// Progree bar
		if (progressCallBack)
		{
			progressCallBack->popCropedValues ();
			progressCallBack->progress (0.5);
			progressCallBack->pushCropedValues (0.5f, 1);
		}

		// add them in the map
		for (uint f = 0; f < filesToProcess.size(); f++)
		{
			// Progree bar
			if (progressCallBack)
			{
				progressCallBack->progress ((float)f/(float)filesToProcess.size());
				progressCallBack->pushCropedValues ((float)f/(float)filesToProcess.size(), (float)(f+1)/(float)filesToProcess.size());
			}

			string filename = CFile::getFilename (filesToProcess[f]);
			string filepath = CFile::getPath (filesToProcess[f]);
//			insertFileInMap (filename, filepath, false, CFile::getExtension(filename));
			addSearchFile (filesToProcess[f], false, "", progressCallBack);

			// Progress bar
			if (progressCallBack)
			{
				progressCallBack->popCropedValues ();
			}
		}

		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->popCropedValues ();
		}
	}
}

void CPath::addSearchFile (const string &file, bool remap, const string &virtual_ext, NLMISC::IProgressCallback *progressCallBack)
{
	getInstance()->_FileContainer.addSearchFile(file, remap, virtual_ext, progressCallBack);
}

void CFileContainer::addSearchFile (const string &file, bool remap, const string &virtual_ext, NLMISC::IProgressCallback *progressCallBack)
{
	nlassert(!_MemoryCompressed);

	string newFile = standardizePath(file, false);

	// check empty file
	if (newFile.empty())
	{
		nlwarning ("PATH: CPath::addSearchFile(%s, %d, '%s'): can't add empty file, skip it", file.c_str(), remap, virtual_ext.c_str());
		return;
	}

	// check if the file exists
	if (!CFile::isExists (newFile))
	{
		nlwarning ("PATH: CPath::addSearchFile(%s, %d, '%s'): '%s' is not found, skip it (current dir is '%s'",
			file.c_str(),
			remap,
			virtual_ext.c_str(),
			newFile.c_str(),
			CPath::getCurrentPath().c_str());
		return;
	}

	// check if it s a file
	if (CFile::isDirectory (newFile))
	{
		nlwarning ("PATH: CPath::addSearchFile(%s, %d, '%s'): '%s' is not a file, skip it",
			file.c_str(),
			remap,
			virtual_ext.c_str(),
			newFile.c_str());
		return;
	}

	// check if it s a big file
	if (CFile::getExtension(newFile) == "bnp")
	{
		NL_DISPLAY_PATH ("PATH: CPath::addSearchFile(%s, %d, '%s'): '%s' is a big file, add it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		addSearchBigFile(file, false, false, progressCallBack);
		return;
	}

	// check if it s an xml pack file
	if (CFile::getExtension(newFile) == "xml_pack")
	{
		NL_DISPLAY_PATH ("PATH: CPath::addSearchFile(%s, %d, '%s'): '%s' is an xml pack file, add it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		addSearchXmlpackFile(file, false, false, progressCallBack);
		return;
	}

	string filenamewoext = CFile::getFilenameWithoutExtension (newFile);
	string filename, ext;

	if (virtual_ext.empty())
	{
		filename = CFile::getFilename (newFile);
		ext = CFile::getExtension (filename);
	}
	else
	{
		filename = filenamewoext + "." + virtual_ext;
		ext = CFile::getExtension (newFile);
	}

	insertFileInMap (filename, newFile, remap, ext);

	if (!remap && !ext.empty())
	{
		// now, we have to see extension and insert in the map the remapped files
		for (uint i = 0; i < _Extensions.size (); i++)
		{
			if (_Extensions[i].first == toLower(ext))
			{
				// need to remap
				addSearchFile (newFile, true, _Extensions[i].second, progressCallBack);
			}
		}
	}
}

void CPath::addSearchListFile (const string &filename, bool recurse, bool alternative)
{
	getInstance()->_FileContainer.addSearchListFile(filename, recurse, alternative);
}

void CFileContainer::addSearchListFile (const string &filename, bool recurse, bool alternative)
{
	// check empty file
	if (filename.empty())
	{
		nlwarning ("PATH: CPath::addSearchListFile(%s, %d, %d): can't add empty file, skip it", filename.c_str(), recurse, alternative);
		return;
	}

	// check if the file exists
	if (!CFile::isExists (filename))
	{
		nlwarning ("PATH: CPath::addSearchListFile(%s, %d, %d): '%s' is not found, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// check if it s a file
	if (CFile::isDirectory (filename))
	{
		nlwarning ("PATH: CPath::addSearchListFile(%s, %d, %d): '%s' is not a file, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// TODO read the file and add files that are inside
}

// WARNING : recurse is not used
void CPath::addSearchBigFile (const string &sBigFilename, bool recurse, bool alternative, NLMISC::IProgressCallback *progressCallBack)
{
	getInstance()->_FileContainer.addSearchBigFile(sBigFilename, recurse, alternative, progressCallBack);
}

void CFileContainer::addSearchBigFile (const string &sBigFilename, bool recurse, bool alternative, NLMISC::IProgressCallback *progressCallBack)
{
	// Check if filename is not empty
	if (sBigFilename.empty())
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't add empty file, skip it", sBigFilename.c_str(), recurse, alternative);
		return;
	}
	// Check if the file exists
	if (!CFile::isExists (sBigFilename))
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): '%s' is not found, skip it", sBigFilename.c_str(), recurse, alternative, sBigFilename.c_str());
		return;
	}
	// Check if it s a file
	if (CFile::isDirectory (sBigFilename))
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): '%s' is not a file, skip it", sBigFilename.c_str(), recurse, alternative, sBigFilename.c_str());
		return;
	}
	// Open and read the big file header
	nlassert(!_MemoryCompressed);

	FILE *Handle = fopen (sBigFilename.c_str(), "rb");
	if (Handle == NULL)
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't open file, skip it", sBigFilename.c_str(), recurse, alternative);
		return;
	}

	// add the link with the CBigFile singleton
	if (CBigFile::getInstance().add (sBigFilename, BF_ALWAYS_OPENED | BF_CACHE_FILE_ON_OPEN))
	{
		// also add the bigfile name in the map to retrieve the full path of a .bnp when we want modification date of the bnp for example
		insertFileInMap (CFile::getFilename (sBigFilename), sBigFilename, false, CFile::getExtension(sBigFilename));

		// parse the big file to add file in the map
		uint32 nFileSize=CFile::getFileSize (Handle);
		//nlfseek64 (Handle, 0, SEEK_END);
		//uint32 nFileSize = ftell (Handle);
		nlfseek64 (Handle, nFileSize-4, SEEK_SET);
		uint32 nOffsetFromBeginning;
		if (fread (&nOffsetFromBeginning, sizeof(uint32), 1, Handle) != 1)
		{
			fclose(Handle);
			return;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nOffsetFromBeginning);
#endif

		nlfseek64 (Handle, nOffsetFromBeginning, SEEK_SET);
		uint32 nNbFile;
		if (fread (&nNbFile, sizeof(uint32), 1, Handle) != 1)
		{
			fclose(Handle);
			return;
		}

#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(nNbFile);
#endif

		for (uint32 i = 0; i < nNbFile; ++i)
		{
			// Progress bar
			if (progressCallBack)
			{
				progressCallBack->progress ((float)i/(float)nNbFile);
				progressCallBack->pushCropedValues ((float)i/(float)nNbFile, (float)(i+1)/(float)nNbFile);
			}

			char FileName[256];
			uint8 nStringSize;
			if (fread (&nStringSize, 1, 1, Handle) != 1)
			{
				fclose(Handle);
				return;
			}
			if (fread (FileName, 1, nStringSize, Handle) != nStringSize)
			{
				fclose(Handle);
				return;
			}
			FileName[nStringSize] = 0;
			uint32 nFileSize2;
			if (fread (&nFileSize2, sizeof(uint32), 1, Handle) != 1)
			{
				fclose(Handle);
				return;
			}

#ifdef NL_BIG_ENDIAN
			NLMISC_BSWAP32(nFileSize2);
#endif

			uint32 nFilePos;
			if (fread (&nFilePos, sizeof(uint32), 1, Handle) != 1)
			{
				fclose(Handle);
				return;
			}

#ifdef NL_BIG_ENDIAN
			NLMISC_BSWAP32(nFilePos);
#endif

			string sTmp = toLower(string(FileName));
			if (sTmp.empty())
			{
				nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't add empty file, skip it", sBigFilename.c_str(), recurse, alternative);
				continue;
			}
			string bigfilenamealone = CFile::getFilename (sBigFilename);
			string filenamewoext = CFile::getFilenameWithoutExtension (sTmp);
			string ext = toLower(CFile::getExtension(sTmp));

			insertFileInMap (sTmp, bigfilenamealone + "@" + sTmp, false, ext);

			for (uint j = 0; j < _Extensions.size (); j++)
			{
				if (_Extensions[j].first == ext)
				{
					// need to remap
					insertFileInMap (filenamewoext+"."+_Extensions[j].second,
									bigfilenamealone + "@" + sTmp,
									true,
									_Extensions[j].first);
				}
			}

			// Progress bar
			if (progressCallBack)
			{
				progressCallBack->popCropedValues ();
			}
		}
	}
	else
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't add the big file", sBigFilename.c_str(), recurse, alternative);
	}

	fclose (Handle);
}

// WARNING : recurse is not used
void CPath::addSearchXmlpackFile (const string &sXmlpackFilename, bool recurse, bool alternative, NLMISC::IProgressCallback *progressCallBack)
{
	getInstance()->_FileContainer.addSearchXmlpackFile(sXmlpackFilename, recurse, alternative, progressCallBack);
}

void CFileContainer::addSearchXmlpackFile (const string &sXmlpackFilename, bool recurse, bool alternative, NLMISC::IProgressCallback *progressCallBack)
{
	// Check if filename is not empty
	if (sXmlpackFilename.empty())
	{
		nlwarning ("PATH: CPath::addSearchXmlpackFile(%s, %d, %d): can't add empty file, skip it", sXmlpackFilename.c_str(), recurse, alternative);
		return;
	}
	// Check if the file exists
	if (!CFile::isExists (sXmlpackFilename))
	{
		nlwarning ("PATH: CPath::addSearchXmlpackFile(%s, %d, %d): '%s' is not found, skip it", sXmlpackFilename.c_str(), recurse, alternative, sXmlpackFilename.c_str());
		return;
	}
	// Check if it s a file
	if (CFile::isDirectory (sXmlpackFilename))
	{
		nlwarning ("PATH: CPath::addSearchXmlpackFile(%s, %d, %d): '%s' is not a file, skip it", sXmlpackFilename.c_str(), recurse, alternative, sXmlpackFilename.c_str());
		return;
	}
	// Open and read the xmlpack file header

	FILE *Handle = fopen (sXmlpackFilename.c_str(), "rb");
	if (Handle == NULL)
	{
		nlwarning ("PATH: CPath::addSearchXmlpackFile(%s, %d, %d): can't open file, skip it", sXmlpackFilename.c_str(), recurse, alternative);
		return;
	}

	// add the link with the CXMLPack singleton
	if (CXMLPack::getInstance().add (sXmlpackFilename))
	{
		// also add the xmlpack file name in the map to retrieve the full path of a .xml_pack when we want modification date of the xml_pack for example
		insertFileInMap (sXmlpackFilename, sXmlpackFilename, false, CFile::getExtension(sXmlpackFilename));


		vector<string>	filenames;
		CXMLPack::getInstance().list(sXmlpackFilename, filenames);

		for (uint i=0; i<filenames.size(); ++i)
		{
			// Progress bar
			if (progressCallBack)
			{
				progressCallBack->progress ((float)i/(float)filenames.size());
				progressCallBack->pushCropedValues ((float)i/(float)filenames.size(), (float)(i+1)/(float)filenames.size());
			}

			string packfilenamealone = sXmlpackFilename;
			string filenamewoext = CFile::getFilenameWithoutExtension (filenames[i]);
			string ext = toLower(CFile::getExtension(filenames[i]));

			insertFileInMap (filenames[i], packfilenamealone + "@@" + filenames[i], false, ext);

			for (uint j = 0; j < _Extensions.size (); j++)
			{
				if (_Extensions[j].first == ext)
				{
					// need to remap
					insertFileInMap (filenamewoext+"."+_Extensions[j].second,
									packfilenamealone + "@@" + filenames[i],
									true,
									_Extensions[j].first);
				}
			}

			// Progress bar
			if (progressCallBack)
			{
				progressCallBack->popCropedValues ();
			}
		}
	}
	else
	{
		nlwarning ("PATH: CPath::addSearchXmlpackFile(%s, %d, %d): can't add the xml pack file", sXmlpackFilename.c_str(), recurse, alternative);
	}

	fclose (Handle);
}

void CPath::addIgnoredDoubleFile(const std::string &ignoredFile)
{
	getInstance()->_FileContainer.addIgnoredDoubleFile(ignoredFile);
}

void CFileContainer::addIgnoredDoubleFile(const std::string &ignoredFile)
{
	IgnoredFiles.push_back(ignoredFile);
}

void CFileContainer::insertFileInMap (const string &filename, const string &filepath, bool remap, const string &extension)
{
	nlassert(!_MemoryCompressed);
	// find if the file already exist
	TFiles::iterator it = _Files.find (toLower(filename));
	if (it != _Files.end ())
	{
		string path = SSMpath.get((*it).second.idPath);
		if (path.find("@") != string::npos && filepath.find("@") == string::npos)
		{
			// if there's a file in a big file and a file in a path, the file in path wins
			// replace with the new one
			nlinfo ("PATH: CPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s' but special case so override it", filename.c_str(), filepath.c_str(), remap, extension.c_str(), path.c_str());
			string sTmp = filepath.substr(0,filepath.rfind('/')+1);
			(*it).second.idPath = SSMpath.add(sTmp);
			(*it).second.Remapped = remap;
			(*it).second.idExt = SSMext.add(extension);
			(*it).second.Name = filename;
		}
		else
		{
			for(uint i = 0; i < IgnoredFiles.size(); i++)
			{
				// if we don't want to display a warning, skip it
				if(filename == IgnoredFiles[i])
					return;
			}
			// if the path is the same, don't warn
			string path2 = SSMpath.get((*it).second.idPath);
			string sPathOnly;
			if(filepath.rfind("@@") != string::npos)
				sPathOnly = filepath.substr(0,filepath.rfind("@@")+2);
			else if(filepath.rfind('@') != string::npos)
				sPathOnly = filepath.substr(0,filepath.rfind('@')+1);
			else
				sPathOnly = CFile::getPath(filepath);

			if (path2 == sPathOnly)
				return;
			nlwarning ("PATH: CPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s', skip it",
				filename.c_str(),
				filepath.c_str(),
				remap,
				extension.c_str(),
				path2.c_str());
		}
	}
	else
	{
		CFileEntry fe;
		fe.idExt = SSMext.add(extension);
		fe.Remapped = remap;
		string sTmp;
		if (filepath.find("@") == string::npos)
			sTmp = filepath.substr(0,filepath.rfind('/')+1);
		else if (filepath.find("@@") != string::npos)
			sTmp = filepath.substr(0,filepath.rfind("@@")+2);
		else
			sTmp = filepath.substr(0,filepath.rfind('@')+1);

		fe.idPath = SSMpath.add(sTmp);
		fe.Name = filename;

		_Files.insert (make_pair(toLower(filename), fe));
		NL_DISPLAY_PATH("PATH: CPath::insertFileInMap(%s, %s, %d, %s): added", toLower(filename).c_str(), filepath.c_str(), remap, toLower(extension).c_str());
	}
}

void CPath::display ()
{
	getInstance()->_FileContainer.display();
}

void CFileContainer::display ()
{
	nlinfo ("PATH: Contents of the map:");
	nlinfo ("PATH: %-25s %-5s %-5s %s", "filename", "ext", "remap", "full path");
	nlinfo ("PATH: ----------------------------------------------------");
	if (_MemoryCompressed)
	{
		for (uint i = 0; i < _MCFiles.size(); ++i)
		{
			const CMCFileEntry &fe = _MCFiles[i];
			string ext = SSMext.get(fe.idExt);
			string path = SSMpath.get(fe.idPath);
			nlinfo ("PATH: %-25s %-5s %-5d %s", fe.Name, ext.c_str(), fe.Remapped, path.c_str());
		}
	}
	else
	{
		for (TFiles::iterator it = _Files.begin(); it != _Files.end (); it++)
		{
			string ext = SSMext.get((*it).second.idExt);
			string path = SSMpath.get((*it).second.idPath);
			nlinfo ("PATH: %-25s %-5s %-5d %s", (*it).first.c_str(), ext.c_str(), (*it).second.Remapped, path.c_str());
		}
	}
	nlinfo ("PATH: ");
	nlinfo ("PATH: Contents of the alternative directory:");
	for (uint i = 0; i < _AlternativePaths.size(); i++)
	{
		nlinfo ("PATH: '%s'", _AlternativePaths[i].c_str ());
	}
	nlinfo ("PATH: ");
	nlinfo ("PATH: Contents of the remapped entension table:");
	for (uint j = 0; j < _Extensions.size(); j++)
	{
		nlinfo ("PATH: '%s' -> '%s'", _Extensions[j].first.c_str (), _Extensions[j].second.c_str ());
	}
	nlinfo ("PATH: End of display");
}

void CPath::removeBigFiles(const std::vector<std::string> &bnpFilenames)
{
	getInstance()->_FileContainer.removeBigFiles(bnpFilenames);
}

void CFileContainer::removeBigFiles(const std::vector<std::string> &bnpFilenames)
{
	nlassert(!isMemoryCompressed());
	CHashSet<TSStringId> bnpStrIds;
	TFiles::iterator fileIt, fileCurrIt;
	for (uint k = 0; k < bnpFilenames.size(); ++k)
	{
		std::string completeBNPName = toLower(bnpFilenames[k]) + "@";
		if (SSMpath.isAdded(completeBNPName))
		{
			bnpStrIds.insert(SSMpath.add(completeBNPName));
		}
		CBigFile::getInstance().remove(bnpFilenames[k]);
		fileIt = _Files.find(toLower(bnpFilenames[k]));
		if (fileIt != _Files.end())
		{
			_Files.erase(fileIt);
		}
	}
	if (bnpStrIds.empty()) return;
	//	remove remapped files
	std::map<std::string, std::string>::iterator remapIt, remapCurrIt;
	for(remapIt = _RemappedFiles.begin(); remapIt != _RemappedFiles.end();)
	{
		remapCurrIt = remapIt;
		++ remapIt;
		const std::string &filename = remapCurrIt->second;
		fileIt = _Files.find(filename);
		if (fileIt != _Files.end())
		{
			if (bnpStrIds.count(fileIt->second.idPath))
			{
				_Files.erase(fileIt);
				_RemappedFiles.erase(remapCurrIt);
			}
		}
	}
	//	remove file entries
	for(fileIt = _Files.begin(); fileIt != _Files.end();)
	{
		fileCurrIt = fileIt;
		++ fileIt;
		if (bnpStrIds.count(fileCurrIt->second.idPath))
		{
			_Files.erase(fileCurrIt);
		}
	}


}


void CPath::memoryCompress()
{
	getInstance()->_FileContainer.memoryCompress();
}

void CFileContainer::memoryCompress()
{

	SSMext.memoryCompress();
	SSMpath.memoryCompress();
	uint nDbg = (uint)_Files.size();
	uint nDbg2 = SSMext.getCount();
	uint nDbg3 = SSMpath.getCount();
	nlinfo ("PATH: Number of file: %d, extension: %d, path: %d", nDbg, nDbg2, nDbg3);

	// Convert from _Files to _MCFiles
	uint nSize = 0, nNb = 0;
	TFiles::iterator it = _Files.begin();
	while (it != _Files.end())
	{
		string sTmp = SSMpath.get(it->second.idPath);
		if ((sTmp.find("@@") == string::npos) && (sTmp.find('@') != string::npos) && !it->second.Remapped)
		{
			// This is a file included in a bigfile (so the name is in the bigfile manager)
		}
		else
		{
			nSize += (uint)it->second.Name.size() + 1;
		}
		nNb++;
		it++;
	}

	_AllFileNames = new char[nSize];
	memset(_AllFileNames, 0, nSize);
	_MCFiles.resize(nNb);

	it = _Files.begin();
	nSize = 0;
	nNb = 0;
	while (it != _Files.end())
	{
		CFileEntry &rFE = it->second;
		string sTmp = SSMpath.get(rFE.idPath);
		if (sTmp.find("@") == string::npos || sTmp.find("@@") != string::npos || rFE.Remapped)
		{
			strcpy(_AllFileNames+nSize, rFE.Name.c_str());
			_MCFiles[nNb].Name = _AllFileNames+nSize;
			nSize += (uint)rFE.Name.size() + 1;
		}
		else
		{
			// This is a file included in a bigfile (so the name is in the bigfile manager)
			sTmp = sTmp.substr(0, sTmp.size()-1);
			_MCFiles[nNb].Name = CBigFile::getInstance().getFileNamePtr(rFE.Name, sTmp);
			if (_MCFiles[nNb].Name == NULL)
			{
				nlerror("memoryCompress: failed to find named file in big file: %s",SSMpath.get(rFE.idPath));
			}
		}

		_MCFiles[nNb].idExt = rFE.idExt;
		_MCFiles[nNb].idPath = rFE.idPath;
		_MCFiles[nNb].Remapped = rFE.Remapped;

		nNb++;
		it++;
	}

	contReset(_Files);
	_MemoryCompressed = true;
}

void CPath::memoryUncompress()
{
	getInstance()->_FileContainer.memoryUncompress();
}

void CFileContainer::memoryUncompress()
{
	SSMext.memoryUncompress();
	SSMpath.memoryUncompress();
	for(std::vector<CMCFileEntry>::iterator it = _MCFiles.begin(); it != _MCFiles.end(); ++it)
	{
		CFileEntry fe;
		fe.Name = it->Name;
		fe.idExt = it->idExt;
		fe.idPath = it->idPath;
		fe.Remapped = it->Remapped;

		_Files[toLower(CFile::getFilename(fe.Name))] = fe;
	}
	contReset(_MCFiles);
	_MemoryCompressed = false;
}

std::string CPath::getWindowsDirectory()
{
	return getInstance()->_FileContainer.getWindowsDirectory();
}

std::string CFileContainer::getWindowsDirectory()
{
#ifndef NL_OS_WINDOWS
	nlwarning("not a ms windows platform");
	return "";
#else
	char winDir[MAX_PATH];
	UINT numChar = ::GetWindowsDirectory(winDir, MAX_PATH);
	if (numChar > MAX_PATH || numChar == 0)
	{
		nlwarning("Couldn't retrieve windows directory");
		return "";
	}
	return CPath::standardizePath(winDir);
#endif
}

std::string CPath::getApplicationDirectory(const std::string &appName)
{
	return getInstance()->_FileContainer.getApplicationDirectory(appName);
}

std::string CFileContainer::getApplicationDirectory(const std::string &appName)
{
	static std::string appPath;
	if (appPath.empty())
	{
#ifdef NL_OS_WINDOWS
		wchar_t buffer[MAX_PATH];
		SHGetSpecialFolderPathW(NULL, buffer, CSIDL_APPDATA, TRUE);
		appPath = CPath::standardizePath(ucstring((ucchar*)buffer).toUtf8());
#elif defined(NL_OS_MAC)
		appPath = CPath::standardizePath(getenv("HOME"));
		appPath += "/Library/Application Support/";
#else
		appPath = CPath::standardizePath(getenv("HOME"));
#endif
	}

	std::string path = appPath;
#ifdef NL_OS_WINDOWS
	if (!appName.empty())
		path = CPath::standardizePath(path + appName);
#elif defined(NL_OS_MAC)
	path = CPath::standardizePath(path + appName);
#else
	if (!appName.empty())
		path = CPath::standardizePath(path + "." + toLower(appName));
#endif

	return path;
}

std::string CPath::getTemporaryDirectory()
{
	return getInstance()->_FileContainer.getTemporaryDirectory();
}

std::string CFileContainer::getTemporaryDirectory()
{
	static std::string path;
	if (path.empty())
	{
		const char *temp = getenv("TEMP");
		const char *tmp = getenv("TMP");

		std::string tempDir;

		if (temp)
			tempDir = temp;

		if (tempDir.empty() && tmp)
			tempDir = tmp;

#ifdef NL_OS_UNIX
		if (tempDir.empty())
			tempDir = "/tmp";
#else
		if (tempDir.empty())
			tempDir = ".";
#endif

		path = CPath::standardizePath(tempDir);
	}

	return path;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

int CFile::getLastSeparator (const string &filename)
{
	string::size_type pos = filename.find_last_of ('/');
	if (pos == string::npos)
	{
		pos = filename.find_last_of ('\\');
		if (pos == string::npos)
		{
			pos = filename.find_last_of ('@');
		}
	}
	return (int)pos;
}

string CFile::getFilename (const string &filename)
{
	string::size_type pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (pos + 1);
	else
		return filename;
}

string CFile::getFilenameWithoutExtension (const string &filename)
{
	string filename2 = getFilename (filename);
	string::size_type pos = filename2.find_last_of ('.');
	if (pos == string::npos)
		return filename2;
	else
		return filename2.substr (0, pos);
}

string CFile::getExtension (const string &filename)
{
	string::size_type pos = filename.find_last_of ('.');
	if (pos == string::npos)
		return "";
	else
		return filename.substr (pos + 1);
}

string CFile::getPath (const string &filename)
{
	string::size_type pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (0, pos + 1);
	else
		return "";
}

bool CFile::isDirectory (const string &filename)
{
#ifdef NL_OS_WINDOWS
	DWORD res = GetFileAttributes(filename.c_str());
	if (res == INVALID_FILE_ATTRIBUTES)
	{
		// nlwarning ("PATH: '%s' is not a valid file or directory name", filename.c_str ());
		return false;
	}
	return (res & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else // NL_OS_WINDOWS
	struct stat buf;
	int res = stat (filename.c_str (), &buf);
	if (res == -1)
	{
		// There was previously a warning message here but that was incorrect as it is defined that isDirectory returns false if the directory doesn't exist
		// nlwarning ("PATH: can't stat '%s' error %d '%s'", filename.c_str(), errno, strerror(errno));
		return false;
	}
	return (buf.st_mode & S_IFDIR) != 0;
#endif // NL_OS_WINDOWS
}

bool CFile::isExists (const string &filename)
{
#ifdef NL_OS_WINDOWS
	return (GetFileAttributes(filename.c_str()) != INVALID_FILE_ATTRIBUTES);
#else // NL_OS_WINDOWS
	struct stat buf;
	return stat (filename.c_str (), &buf) == 0;
#endif // NL_OS_WINDOWS
}

bool CFile::createEmptyFile (const std::string& filename)
{
	FILE *file = fopen (filename.c_str(), "wb");

	if (file)
	{
		fclose (file);
		return true;
	}

	return false;
}

bool CFile::fileExists (const string& filename)
{
	//H_AUTO(FileExists);
	return ! ! fstream( filename.c_str(), ios::in );
}


string CFile::findNewFile (const string &filename)
{
	string::size_type pos = filename.find_last_of ('.');
	if (pos == string::npos)
		return filename;

	string start = filename.substr (0, pos);
	string end = filename.substr (pos);

	uint num = 0;
	char numchar[4];
	string npath;
	do
	{
		npath = start;
		smprintf(numchar,4,"%03d",num++);
		npath += numchar;
		npath += end;
		if (!CFile::fileExists(npath)) break;
	}
	while (num<999);
	return npath;
}

// \warning doesn't work with big file
uint32	CFile::getFileSize (const std::string &filename)
{
	if (filename.find("@@") != string::npos)
	{
		uint32 fs = 0, bfo;
		bool c, d;
		CXMLPack::getInstance().getFile (filename, fs, bfo, c, d);
		return fs;
	}
	else if (filename.find('@') != string::npos)
	{
		uint32 fs = 0, bfo;
		bool c, d;
		CBigFile::getInstance().getFile (filename, fs, bfo, c, d);
		return fs;
	}
	else
	{
#if defined (NL_OS_WINDOWS)
		struct _stat buf;
		int result = _stat (filename.c_str (), &buf);
#elif defined (NL_OS_UNIX)
		struct stat buf;
		int result = stat (filename.c_str (), &buf);
#endif
		if (result != 0) return 0;
		else return buf.st_size;
	}
}

uint32	CFile::getFileSize (FILE *f)
{
#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _fstat (fileno(f), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = fstat (fileno(f), &buf);
#endif
	if (result != 0) return 0;
	else return buf.st_size;
}

uint32	CFile::getFileModificationDate(const std::string &filename)
{
	string::size_type pos;
	string fn;
	if ((pos=filename.find("@@")) != string::npos)
	{
		fn = filename.substr (0, pos);
	}
	else if ((pos=filename.find('@')) != string::npos)
	{
		fn = CPath::lookup(filename.substr (0, pos));
	}
	else
	{
		fn = filename;
	}

#if defined (NL_OS_WINDOWS)
//	struct _stat buf;
//	int result = _stat (fn.c_str (), &buf);
	// Changed 06-06-2007 : boris : _stat have an incoherent and hard to reproduce
	// on windows : if the system clock is adjusted according to daylight saving
	// time, the file date reported by _stat may (not always!) be adjusted by 3600s
	// This is a bad behavior because file time should always be reported as UTC time value

	// Use the WIN32 API to read the file times in UTC

	// create a file handle (this does not open the file)
	HANDLE h = CreateFile(fn.c_str(), 0, 0, NULL, OPEN_EXISTING, 0, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		nlwarning("Can't get modification date on file '%s' : %s", fn.c_str(), NLMISC::formatErrorMessage(NLMISC::getLastError()).c_str());
		return 0;
	}
	FILETIME creationTime;
	FILETIME accesstime;
	FILETIME modTime;

	// get the files times
	BOOL res = GetFileTime(h, &creationTime, &accesstime, &modTime);
	if (res == 0)
	{
		nlwarning("Can't get modification date on file '%s' : %s", fn.c_str(), NLMISC::formatErrorMessage(NLMISC::getLastError()).c_str());
		CloseHandle(h);
		return 0;
	}
	// close the handle
	CloseHandle(h);

	// win32 file times are in 10th of micro sec (100ns resolution), starting at jan 1, 1601
	// hey Mr Gates, why 1601 ?

	// first, convert it into second since jan1, 1601
	uint64 t = modTime.dwLowDateTime | (uint64(modTime.dwHighDateTime)<<32);

	// adjust time base to unix epoch base
	t -= CTime::getWindowsToUnixBaseTimeOffset();

	// convert the resulting time into seconds
	t /= 10;	// microsec
	t /= 1000;	// millisec
	t /= 1000;	// sec

	// return the resulting time
	return uint32(t);

#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = stat (fn.c_str (), &buf);
	if (result != 0)
	{
		nlwarning("Can't get modification date on file '%s' : %s", fn.c_str(), NLMISC::formatErrorMessage(NLMISC::getLastError()).c_str());
		return 0;
	}
	else
		return (uint32)buf.st_mtime;
#endif

}

bool	CFile::setFileModificationDate(const std::string &filename, uint32 modTime)
{
	string::size_type pos;
	string fn;
	if ((pos=filename.find('@')) != string::npos)
	{
		fn = CPath::lookup(filename.substr (0, pos));
	}
	else
	{
		fn = filename;
	}

#if defined (NL_OS_WINDOWS)

	// Use the WIN32 API to set the file times in UTC

	// create a file handle (this does not open the file)
	HANDLE h = CreateFile(fn.c_str(), GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		nlwarning("Can't set modification date on file '%s' (error accessing file) : %s", fn.c_str(), NLMISC::formatErrorMessage(NLMISC::getLastError()).c_str());
		return false;
	}
	FILETIME creationFileTime;
	FILETIME accessFileTime;
	FILETIME modFileTime;

	// read the current the files times
	if (GetFileTime(h, &creationFileTime, &accessFileTime, &modFileTime) == 0)
	{
		nlwarning("Can't set modification date on file '%s' : %s", fn.c_str(), formatErrorMessage(getLastError()).c_str());
		CloseHandle(h);
		return false;
	}

	// win32 file times are in 10th of micro sec (100ns resolution), starting at jan 1, 1601
	// hey Mr Gates, why 1601 ?

	// convert the unix time into a windows file time
	uint64 t = modTime;
	// convert to 10th of microsec
	t *= 1000;	// millisec
	t *= 1000;	// microsec
	t *= 10;	// 10th of micro sec (rez of windows file time is 100ns <=> 1/10 us

	// apply the windows to unix base time offset
	t += CTime::getWindowsToUnixBaseTimeOffset();

	// update the windows modTime structure
	modFileTime.dwLowDateTime = uint32(t & 0xffffffff);
	modFileTime.dwHighDateTime = uint32(t >> 32);

	// update the file time on disk
	BOOL rez = SetFileTime(h, &creationFileTime, &accessFileTime, &modFileTime);
	if (rez == 0)
	{
		nlwarning("Can't set modification date on file '%s': %s", fn.c_str(), formatErrorMessage(getLastError()).c_str());

		CloseHandle(h);
		return false;
	}

	// close the handle
	CloseHandle(h);

	return true;

#elif defined (NL_OS_UNIX)
	// first, read the current time of the file
	struct stat buf;
	int result = stat (fn.c_str (), &buf);
	if (result != 0)
		return false;

	// prepare the new time to apply
	utimbuf tb;
	tb.actime = buf.st_atime;
	tb.modtime = modTime;
	// set eh new time
	int res = utime(fn.c_str(), &tb);
	if (res == -1)
		nlwarning("Can't set modification date on file '%s': %s", fn.c_str(), formatErrorMessage(getLastError()).c_str());
	return res != -1;
#endif

}

uint32	CFile::getFileCreationDate(const std::string &filename)
{
	string::size_type pos;
	string fn;
	if ((pos=filename.find('@')) != string::npos)
	{
		fn = CPath::lookup(filename.substr (0, pos));
	}
	else
	{
		fn = filename;
	}

#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _stat (fn.c_str (), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = stat (fn.c_str (), &buf);
#endif

	if (result != 0) return 0;
	else return (uint32)buf.st_ctime;
}

struct CFileEntry
{
	CFileEntry (const string &filename, void (*callback)(const string &filename)) : FileName (filename), Callback (callback)
	{
		LastModified = CFile::getFileModificationDate(filename);
	}
	string FileName;
	void (*Callback)(const string &filename);
	uint32 LastModified;
};

static vector <CFileEntry> FileToCheck;

void CFile::removeFileChangeCallback (const std::string &filename)
{
	string fn = CPath::lookup(filename, false, false);
	if (fn.empty())
	{
		fn = filename;
	}
	for (uint i = 0; i < FileToCheck.size(); i++)
	{
		if(FileToCheck[i].FileName == fn)
		{
			nlinfo ("PATH: CFile::removeFileChangeCallback: '%s' is removed from checked files modification", fn.c_str());
			FileToCheck.erase(FileToCheck.begin()+i);
			return;
		}
	}
}

void CFile::addFileChangeCallback (const std::string &filename, void (*cb)(const string &filename))
{
	string fn = CPath::lookup(filename, false, false);
	if (fn.empty())
	{
		fn = filename;
	}
	nlinfo ("PATH: CFile::addFileChangeCallback: I'll check the modification date for this file '%s'", fn.c_str());
	FileToCheck.push_back(CFileEntry(fn, cb));
}

void CFile::checkFileChange (TTime frequency)
{
	static TTime lastChecked = CTime::getLocalTime();

	if (CTime::getLocalTime() > lastChecked + frequency)
	{
		for (uint i = 0; i < FileToCheck.size(); i++)
		{
			if(CFile::getFileModificationDate(FileToCheck[i].FileName) != FileToCheck[i].LastModified)
			{
				// need to reload it
				if(FileToCheck[i].Callback != NULL)
					FileToCheck[i].Callback(FileToCheck[i].FileName);

				FileToCheck[i].LastModified = CFile::getFileModificationDate(FileToCheck[i].FileName);
			}
		}

		lastChecked = CTime::getLocalTime();
	}
}

static bool CopyMoveFile(const std::string &dest, const std::string &src, bool copyFile, bool failIfExists = false, IProgressCallback *progress = NULL)
{
	if (dest.empty() || src.empty()) return false;
	std::string sdest = CPath::standardizePath(dest,false);
	std::string ssrc = CPath::standardizePath(src,false);

//	return copyFile  ? CopyFile(dossrc.c_str(), dosdest.c_str(), failIfExists) != FALSE
//					 : MoveFile(dossrc.c_str(), dosdest.c_str()) != FALSE;

	if (progress) progress->progress(0.f);
	if(copyFile)
	{
		uint32 totalSize = 0;
		uint32 readSize = 0;
		if (progress)
		{
			totalSize = CFile::getFileSize(ssrc);
		}
		FILE *fp1 = fopen(ssrc.c_str(), "rb");
		if (fp1 == NULL)
		{
			nlwarning ("PATH: CopyMoveFile error: can't fopen in read mode '%s'", ssrc.c_str());
			return false;
		}
		FILE *fp2 = fopen(sdest.c_str(), "wb");
		if (fp2 == NULL)
		{
			nlwarning ("PATH: CopyMoveFile error: can't fopen in read write mode '%s'", sdest.c_str());
			return false;
		}
		static char buffer [1000];
		size_t s;

		s = fread(buffer, 1, sizeof(buffer), fp1);
		while (s != 0)
		{
			if (progress)
			{
				readSize += (uint32)s;
				progress->progress((float) readSize / totalSize);
			}
			size_t ws = fwrite(buffer, s, 1, fp2);
			if (ws != 1)
			{
				nlwarning("Error copying '%s' to '%s', trying to write %u bytes failed.",
					ssrc.c_str(),
					sdest.c_str(),
					s);
				fclose(fp1);
				fclose(fp2);
				nlwarning("Errno = %d", errno);
				return false;
			}
			s = fread(buffer, 1, sizeof(buffer), fp1);
		}

		fclose(fp1);
		fclose(fp2);
		if (progress) progress->progress(1.f);
	}
	else
	{
#ifdef NL_OS_WINDOWS
		if (MoveFile(ssrc.c_str(), sdest.c_str()) == 0)
		{
			LPVOID lpMsgBuf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
					    FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL );
			nlwarning ("PATH: CopyMoveFile error: can't link/move '%s' into '%s', error %u (%s)",
				ssrc.c_str(),
				sdest.c_str(),
				GetLastError(),
				lpMsgBuf);

			LocalFree(lpMsgBuf);
			return false;
		}
#else
		if (rename (ssrc.c_str(), sdest.c_str()) == -1)
		{
			nlwarning ("PATH: CopyMoveFile error: can't rename '%s' into '%s', error %u",
				ssrc.c_str(),
				sdest.c_str(),
				errno);
			return false;
		}
#endif
	}
	if (progress) progress->progress(1.f);
	return true;
}

bool CFile::copyFile(const std::string &dest, const std::string &src, bool failIfExists /*=false*/, IProgressCallback *progress)
{
	return CopyMoveFile(dest, src, true, failIfExists, progress);
}

bool CFile::quickFileCompare(const std::string &fileName0, const std::string &fileName1)
{
	// make sure the files both exist
	if (!fileExists(fileName0.c_str()) || !fileExists(fileName1.c_str()))
		return false;

	// compare time stamps
	if (getFileModificationDate(fileName0.c_str()) != getFileModificationDate(fileName1.c_str()))
		return false;

	// compare file sizes
	if (getFileSize(fileName0.c_str()) != getFileSize(fileName1.c_str()))
		return false;

	// everything matched so return true
	return true;
}

bool CFile::thoroughFileCompare(const std::string &fileName0, const std::string &fileName1,uint32 maxBufSize)
{
	// make sure the files both exist
	if (!fileExists(fileName0.c_str()) || !fileExists(fileName1.c_str()))
		return false;

	// setup the size variable from file length of first file
	uint32 fileSize=getFileSize(fileName0.c_str());

	// compare file sizes
	if (fileSize != getFileSize(fileName1.c_str()))
		return false;

	// allocate a couple of data buffers for our 2 files
	uint32 bufSize= maxBufSize/2;
	nlassert(sint32(bufSize)>0);
	std::vector<uint8> buf0(bufSize);
	std::vector<uint8> buf1(bufSize);

	// open the two files for input
	CIFile file0(fileName0);
	CIFile file1(fileName1);

	for (uint32 i=0;i<fileSize;i+=bufSize)
	{
		// for the last block in the file reduce buf size to represent the amount of data left in file
		if (i+bufSize>fileSize)
		{
			bufSize= fileSize-i;
			buf0.resize(bufSize);
			buf1.resize(bufSize);
		}

		// read in the next data block from disk
		file0.serialBuffer(&buf0[0], bufSize);
		file1.serialBuffer(&buf1[0], bufSize);

		// compare the contents of the 2 data buffers
		if (buf0!=buf1)
			return false;
	}

	// everything matched so return true
	return true;
}

bool CFile::moveFile(const char *dest,const char *src)
{
	return CopyMoveFile(dest, src, false);
}

bool CFile::createDirectory(const std::string &filename)
{
#ifdef NL_OS_WINDOWS
	return _mkdir(filename.c_str())==0;
#else
	// Set full permissions....
	return mkdir(filename.c_str(), 0xFFFF)==0;
#endif
}

bool CFile::createDirectoryTree(const std::string &filename)
{
	bool lastResult=true;
	uint32 i=0;

	// skip dos drive name eg "a:"
	if (filename.size()>1 && filename[1]==':')
		i=2;

	// iterate over the set of directories in the routine's argument
	while (i<filename.size())
	{
		// skip passed leading slashes
		for (;i<filename.size();++i)
			if (filename[i]!='\\' && filename[i]!='/')
				break;

		// if the file name ended with a '/' then there's no extra directory to create
		if (i==filename.size())
			break;

		// skip forwards to next slash
		for (;i<filename.size();++i)
			if (filename[i]=='\\' || filename[i]=='/')
				break;

		// try to create directory
		std::string s= filename.substr(0,i);
		lastResult= createDirectory(s);
	}

	return lastResult;
}

bool CPath::makePathRelative (const char *basePath, std::string &relativePath)
{
	// Standard path with final slash
	string tmp = standardizePath (basePath, true);
	string src = standardizePath (relativePath, true);
	string prefix;

	for(;;)
	{
		// Compare with relativePath
		if (strncmp (tmp.c_str (), src.c_str (), tmp.length ()) == 0)
		{
			// Truncate
			uint size = (uint)tmp.length ();

			// Same path ?
			if (size == src.length ())
			{
				relativePath = ".";
				return true;
			}

			relativePath = prefix+relativePath.substr (size, relativePath.length () - size);
			return true;
		}

		// Too small ?
		if (tmp.length ()<2)
			break;

		// Remove last directory
		string::size_type lastPos = tmp.rfind ('/', tmp.length ()-2);
		string::size_type previousPos = tmp.find ('/');
		if ((lastPos == previousPos) || (lastPos == string::npos))
			break;

		// Troncate
		tmp = tmp.substr (0, lastPos+1);

		// New prefix
		prefix += "../";
	}

	return false;
}

std::string CPath::makePathAbsolute( const std::string &relativePath, const std::string &directory )
{
	if( relativePath.empty() )
		return "";
	if( directory.empty() )
		return "";

#ifdef NL_OS_WINDOWS
	// Windows network address. Eg.: \\someshare\path
	if( ( relativePath[ 0 ] == '\\' ) && ( relativePath[ 1 ] == '\\' ) )
		return relativePath;

	// Normal Windows absolute path. Eg.: C:\something
	//
	if( isalpha( relativePath[ 0 ] ) && ( relativePath[ 1 ] == ':' ) && ( ( relativePath[ 2 ] == '\\' ) || ( relativePath[ 2 ] == '/' ) ) )
		return relativePath;
#else
	// Unix filesystem absolute path
	if( relativePath[ 0 ] == '/' )
		return relativePath;

#endif

	// Add a slash to the directory if necessary.
	// If the relative path starts with dots we need a slash.
	// If the relative path starts with a slash we don't.
	// If it starts with neither, we need a slash.
	bool needSlash = true;
	char c = relativePath[ 0 ];
	if( ( c == '\\' ) || ( c == '/' ) )
		needSlash = false;
	
	bool hasSlash = false;
	std::string npath = directory;
	c = npath[ npath.size() - 1 ];
	if( ( c == '\\' ) || ( c == '/' ) )
		hasSlash = true;

	if( needSlash && !hasSlash )
		npath += '/';
	else
	if( hasSlash && !needSlash )
		npath.resize( npath.size() - 1 );
	
	// Now build the new absolute path
	npath += relativePath;
	npath = standardizePath( npath, false );

	return npath;
}

bool CFile::setRWAccess(const std::string &filename)
{
#ifdef NL_OS_WINDOWS
	// if the file exists and there's no write access
	if (_access (filename.c_str(), 00) == 0 && _access (filename.c_str(), 06) == -1)
	{
		// try to set the read/write access
		if (_chmod (filename.c_str(), _S_IREAD | _S_IWRITE) == -1)
		{
			if (INelContext::getInstance().getAlreadyCreateSharedAmongThreads())
			{
				nlwarning ("PATH: Can't set RW access to file '%s': %d %s", filename.c_str(), errno, strerror(errno));
			}
			return false;
		}
	}
#else
	// if the file exists and there's no write access
	if (access (filename.c_str(), F_OK) == 0)
	{
		// try to set the read/write access
		if (chmod (filename.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH) == -1)
		{
			if (INelContext::getInstance().getAlreadyCreateSharedAmongThreads())
			{
				nlwarning ("PATH: Can't set RW access to file '%s': %d %s", filename.c_str(), errno, strerror(errno));
			}
			return false;
		}
	}
	else
	{
		if (INelContext::getInstance().getAlreadyCreateSharedAmongThreads())
		{
			nlwarning("PATH: Can't access to file '%s'", filename.c_str());
		}
//		return false;
	}
#endif
	return true;
}


#ifdef NL_OS_WINDOWS
#define unlink _unlink
#endif

bool CFile::deleteFile(const std::string &filename)
{
	setRWAccess(filename);
	int res = unlink (filename.c_str());
	if (res == -1)
	{
		if (INelContext::getInstance().getAlreadyCreateSharedAmongThreads())
		{
			nlwarning ("PATH: Can't delete file '%s': (errno %d) %s", filename.c_str(), errno, strerror(errno));
		}
		return false;
	}
	return true;
}

#ifdef NL_OS_WINDOWS
#define rmdir _rmdir
#endif

bool CFile::deleteDirectory(const std::string &filename)
{
	setRWAccess(filename);
	int res = rmdir (filename.c_str());
	if (res == -1)
	{
		nlwarning ("PATH: Can't delete directory '%s': (errno %d) %s", filename.c_str(), errno, strerror(errno));
		return false;
	}
	return true;
}

void CFile::getTemporaryOutputFilename (const std::string &originalFilename, std::string &tempFilename)
{
	uint i = 0;
	do
		tempFilename = originalFilename+".tmp"+toString (i++);
	while (CFile::isExists(tempFilename));
}

} // NLMISC
