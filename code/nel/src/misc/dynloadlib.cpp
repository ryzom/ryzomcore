// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/dynloadlib.h"
#include "nel/misc/path.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

NL_LIB_HANDLE nlLoadLibrary(const std::string &libName)
{
	NL_LIB_HANDLE res = 0;
#ifdef NL_OS_WINDOWS
	res = LoadLibraryW(nlUtf8ToWide(libName));
#elif defined(NL_OS_UNIX)
	res = dlopen(libName.c_str(), RTLD_NOW);
#else
#	error "You must code nlLoadLibrary() for your platform"
#endif
	if(res == 0) nlwarning("Load library '%s' failed: %s", libName.c_str(), NLMISC::formatErrorMessage(NLMISC::getLastError()).c_str());
	return res;
}

bool nlFreeLibrary(NL_LIB_HANDLE libHandle)
{
#ifdef NL_OS_WINDOWS
	return FreeLibrary(libHandle) > 0;
#elif defined(NL_OS_UNIX)
	return dlclose(libHandle) == 0;
#else
#	error "You must code nlFreeLibrary() for your platform"
#endif
}

void *nlGetSymbolAddress(NL_LIB_HANDLE libHandle, const std::string &procName)
{
	void *res = 0;
#ifdef NL_OS_WINDOWS
	res = (void *)GetProcAddress(libHandle, procName.c_str());
#elif defined(NL_OS_UNIX)
	res = dlsym(libHandle, procName.c_str());
#else
#	error "You must code nlGetProcAddress() for your platform"
#endif
	if(res == 0) nlwarning("Getting symbol address of '%s' failed: %s", procName.c_str(), NLMISC::formatErrorMessage(NLMISC::getLastError()).c_str());
	return res;
}

// Again some OS specifics stuff
#ifdef NL_OS_WINDOWS
  const string	nlLibPrefix;	// empty
  const string	nlLibExt(".dll");
#elif defined(NL_OS_MAC)
  const string	nlLibPrefix("lib");
  const string	nlLibExt(".dylib");
#elif defined(NL_OS_UNIX)
  const string	nlLibPrefix("lib");
  const string	nlLibExt(".so");
#else
#	error "You must define the default dynamic lib extention"
#endif

std::vector<std::string>	CLibrary::_LibPaths;


CLibrary::CLibrary (const CLibrary &/* other */)
{
	// Nothing to do has it is forbidden.
	// Allowing copy require to manage reference count from CLibrary to the module resource.
	nlassert(false);
}

CLibrary &CLibrary::operator =(const CLibrary &/* other */)
{
	// Nothing to do has it is forbidden.
	// Allowing assignment require to manage reference count from CLibrary to the module resource.
	nlassert(false);
	return *this;
}

std::string CLibrary::makeLibName(const std::string &baseName)
{
	return nlLibPrefix+baseName+nlLibSuffix+nlLibExt;
}

std::string CLibrary::cleanLibName(const std::string &decoratedName)
{
	// remove path and extension
	string ret = CFile::getFilenameWithoutExtension(decoratedName);

	if (!nlLibPrefix.empty())
	{
		// remove prefix
		if (ret.find(nlLibPrefix) == 0)
			ret = ret.substr(nlLibPrefix.size());
	}
	if (!nlLibSuffix.empty())
	{
		// remove suffix
		if (ret.substr(ret.size()-nlLibSuffix.size()) == nlLibSuffix)
			ret = ret.substr(0, ret.size() - nlLibSuffix.size());
	}

	return ret;
}

void CLibrary::addLibPaths(const std::vector<std::string> &paths)
{
	for (uint i=0; i<paths.size(); ++i)
	{
		string newPath = CPath::standardizePath(paths[i]);

		// only add new path
		if (std::find(_LibPaths.begin(), _LibPaths.end(), newPath) == _LibPaths.end())
		{
			_LibPaths.push_back(newPath);
		}
	}
}

void CLibrary::addLibPath(const std::string &path)
{
	string newPath = CPath::standardizePath(path);

	// only add new path
	if (std::find(_LibPaths.begin(), _LibPaths.end(), newPath) == _LibPaths.end())
	{
		_LibPaths.push_back(newPath);
	}
}

CLibrary::CLibrary()
:	_LibHandle(NULL),
	_Ownership(true),
	_PureNelLibrary(NULL)
{
}

CLibrary::CLibrary(NL_LIB_HANDLE libHandle, bool ownership)
: _PureNelLibrary(NULL)
{
	_LibHandle = libHandle;
	_Ownership = ownership;
	_LibFileName = "unknown";
}

CLibrary::CLibrary(const std::string &libName, bool addNelDecoration, bool tryLibPath, bool ownership)
: _PureNelLibrary(NULL)
{
	loadLibrary(libName, addNelDecoration, tryLibPath, ownership);
	// Assert here !
	nlassert(_LibHandle != NULL);
}


CLibrary::~CLibrary()
{
	if (_LibHandle != NULL && _Ownership)
	{
		nlFreeLibrary(_LibHandle);
	}
}

bool CLibrary::loadLibrary(const std::string &libName, bool addNelDecoration, bool tryLibPath, bool ownership)
{
	_Ownership = ownership;
	string libPath = libName;

	if (addNelDecoration)
		libPath = makeLibName(libPath);

	if (tryLibPath)
	{
		// remove any directory spec
		string filename = CFile::getFilename(libPath);

		for (uint i=0; i<_LibPaths.size(); ++i)
		{
			string pathname = _LibPaths[i]+filename;
			if (CFile::isExists(pathname))
			{
				// we found it, replace libPath
				libPath = pathname;
				break;
			}
		}
	}

	nldebug("Loading dynamic library '%s'", libPath.c_str());
	// load the lib now
	_LibHandle = nlLoadLibrary(libPath);
	_LibFileName = libPath;
	// MTR: some new error handling. Just logs if it couldn't load the handle.
	if(_LibHandle == NULL)
	{
#ifdef NL_OS_UNIX
		const char *errormsg = dlerror();
#else
		const char *errormsg = "Verify DLL existence";
#endif
		nlwarning("Loading library %s failed: %s", libPath.c_str(), errormsg);
	}
	else
	{
		// check for 'pure' NeL library
		void *entryPoint = getSymbolAddress(NL_MACRO_TO_STR(NLMISC_PURE_LIB_ENTRY_POINT));
		if (entryPoint != NULL)
		{
			// rebuild the interface pointer
			_PureNelLibrary = *(reinterpret_cast<INelLibrary**>(entryPoint));
			// call the private initialization method.
			_PureNelLibrary->_onLibraryLoaded(INelContext::getInstance());
		}
	}

	return _LibHandle != NULL;
}

void CLibrary::freeLibrary()
{
	if (_LibHandle)
	{
		nlassert(_Ownership);

		if (_PureNelLibrary)
		{
			// call the private finalization method.
			_PureNelLibrary->_onLibraryUnloaded();
		}

		nldebug("Freeing dynamic library '%s'", _LibFileName.c_str());
		nlFreeLibrary(_LibHandle);

		_PureNelLibrary = NULL;
		_LibHandle = NULL;
		_Ownership = false;
		_LibFileName.clear();
	}
}

void *CLibrary::getSymbolAddress(const std::string &symbolName)
{
	nlassert(_LibHandle != NULL);

	return nlGetSymbolAddress(_LibHandle, symbolName);
}

bool CLibrary::isLibraryLoaded()
{
	return _LibHandle != NULL;
}

bool CLibrary::isLibraryPure()
{
	return _LibHandle != NULL && _PureNelLibrary != NULL;
}

INelLibrary *CLibrary::getNelLibraryInterface()
{
	if (!isLibraryPure())
		return NULL;

	return _PureNelLibrary;
}

INelLibrary::~INelLibrary()
{
	// cleanup ram
	if (_LibContext != NULL)
		delete _LibContext;
}

void INelLibrary::_onLibraryLoaded(INelContext &nelContext)
{
	++_LoadingCounter;

	if (_LoadingCounter == 1)
	{
		// Linux relocates all symbols, so this is unnecessary.
#ifdef NL_OS_WINDOWS
		// initialize NeL context
		nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS

		_LibContext = new CLibraryContext(nelContext);
	}
	else
	{
		nlassert(NLMISC::INelContext::isContextInitialised());
	}

	onLibraryLoaded(_LoadingCounter==1);
}

void INelLibrary::_onLibraryUnloaded()
{
	nlassert(_LoadingCounter > 0);

	onLibraryUnloaded(_LoadingCounter == 1);

	--_LoadingCounter;
}

uint32	INelLibrary::getLoadingCounter()
{
	return _LoadingCounter;
}


}	// namespace NLMISC
