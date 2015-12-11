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

#ifndef NL_DYNLIBLOAD_H
#define NL_DYNLIBLOAD_H

#include "types_nl.h"
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
struct HINSTANCE__;
typedef struct HINSTANCE__ *HINSTANCE;
typedef HINSTANCE HMODULE;      /* HMODULEs can be used in place of HINSTANCEs */
#else
#	include <dlfcn.h>
#endif

namespace NLMISC
{

/// Define the os specific type for dynamic library module handler
#if defined (NL_OS_WINDOWS)
typedef HMODULE		NL_LIB_HANDLE;
#elif defined (NL_OS_UNIX)
typedef void*		NL_LIB_HANDLE;
#else
# error "You must define the module type on this platform"
#endif

#ifdef NL_OS_WINDOWS
// MSCV need explicit tag to export or import symbol for a code module
#define NL_LIB_EXPORT	__declspec(dllexport)
#define NL_LIB_IMPORT	__declspec(dllimport)
#else
// other systems don't bother with this kind of detail, they export any 'extern' symbol (almost all functions)
#define NL_LIB_EXPORT
#define NL_LIB_IMPORT
#endif

/// Generic dynamic library loading function.
NL_LIB_HANDLE	nlLoadLibrary(const std::string &libName);
/// Generic dynamic library unloading function.
bool			nlFreeLibrary(NL_LIB_HANDLE libHandle);
/// Generic dynamic library symbol address lookup function.
void			*nlGetSymbolAddress(NL_LIB_HANDLE libHandle, const std::string &symbolName);

// Compilation mode specific suffixes
#ifdef NL_OS_WINDOWS
#	ifdef NL_DEBUG_INSTRUMENT
const std::string nlLibSuffix("_di");
#	elif defined(NL_DEBUG)
const std::string nlLibSuffix("_d");
#	elif defined(NL_RELEASE)
const std::string nlLibSuffix("_r");
#	else
#		error "Unknown compilation mode, can't build suffix"
#	endif
#elif defined (NL_OS_UNIX)
const std::string nlLibSuffix; // empty
#else
#	error "You must define the lib suffix for your platform"
#endif


// Utility macro to export a module entry point as a C pointer to a C++ class or function
#define NL_LIB_EXPORT_SYMBOL(symbolName, classOrFunctionName, instancePointer) \
extern "C" \
{ \
	NL_LIB_EXPORT classOrFunctionName	*symbolName = (classOrFunctionName*)instancePointer; \
};

/*
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2004
 */
class CLibrary
{
	/// Dynamic library handle
	NL_LIB_HANDLE	_LibHandle;
	/// Loaded library name
	std::string		_LibFileName;

	/** When a module handle is assigned to the instance, this
	 *	flag state whether the CLibrary will free the library or not
	 *	when it is deleted.
	 */
	bool			_Ownership;
	/** This points to the 'pure' nel library interface.
	 *	This means that the library export an entry point called
	 *	'NeLLibraryEntry' that points to a INelLibrary interface.
	 *	This interface is used by the CLibrary class to
	 *	manage efficiently the library and to give
	 *	library implementors some hooks on library management.
	 */
	class INelLibrary		*_PureNelLibrary;

	/// Lib paths
	static std::vector<std::string>	_LibPaths;

	/// Private copy constructor, not authorized
	CLibrary (const CLibrary &other);

	// Private assignment operator
	CLibrary &operator =(const CLibrary &other);

public:
	CLibrary();
	/** Assign a existing module handler to a new dynamic library instance
	 *	Note that you cannot use 'pure nel library' this way.
	 */
	CLibrary(NL_LIB_HANDLE libHandle, bool ownership);
	/// Load the specified library and take ownership by default
	CLibrary(const std::string &libName, bool addNelDecoration, bool tryLibPath, bool ownership = true);
	/// Destructor, free the library if the object have ownership
	virtual ~CLibrary();

	/** Load the specified library.
	*	The method assert if a module is already assigned or loaded
	*	If addNelDecoration is true, the standard Nel suffix and library extention or prefix are
	*	added to the lib name (with is just a base name).
	*	If tryLibPath is true, then the method will try to find the required
	*	library in the added library files (in order of addition).
	*	Return true if the library load ok.
	*/
	bool loadLibrary(const std::string &libName, bool addNelDecoration, bool tryLibPath, bool ownership = true);

	/** Unload (free) the assigned/loaded library.
	*	The object must have ownership over the library or the call will assert.
	*	After this call, you can recall loadLibrary.
	*/
	void freeLibrary();

	/** Get the address a the specified procedure in the library
	*	Return NULL is the proc is not found,
	*	Assert if the library is not load or assigned.
	*/
	void *getSymbolAddress(const std::string &symbolName);

	/// Get the name of the loaded library
	std::string getLibFileName()
	{
		return _LibFileName;
	}

	/// Check whether a library is effectively loaded
	bool isLibraryLoaded();

	/** Check if the loaded library is a pure library
	 *	Return false if the library is not pure
	 *	or if the library is not loaded.
	 */
	bool isLibraryPure();

	/** Return the pointer on the pure nel library interface.
	 *	Return NULL if isLibraryPure() is false.
	 */
	class INelLibrary *getNelLibraryInterface();

	/** Build a NeL standard library name according to platform and compilation mode setting.
	 *	aka : adding decoration one base lib name.
	 *	e.g : 'mylib' become	'mylib_rd.dll' on Windows ReleaseDebug mode,
	 *							'libmylib.dylib' under OS X or
	 *							'libmylib.so' on unix system.
	 */
	static std::string makeLibName(const std::string &baseName);
	/** Remove NeL standard library name decoration according to platform and compilation mode setting.
	 *	e.g : 'mylib_rd.dll' on windows ReleaseDebug mode become 'mylib'
	 */
	static std::string cleanLibName(const std::string &decoratedName);
	/** Add a list of library path
	 *	@see loadLibrary for a discusion about lib path
	 */
	static void addLibPaths(const std::vector<std::string> &paths);
	/** Add a library path
	 *	@see loadLibrary for a discusion about lib path
	 */
	static void addLibPath(const std::string &path);

};

/** Interface class for 'pure Nel' library module.
 *	This interface is used to expose a standard
 *	entry point for dynamically loaded library
 *	and to add some hooks for library implementor
 *	over library loading/unloading.
 */
class INelLibrary
{
	friend class CLibrary;

	/// this counter keep tracks of how many time the library have been loaded in the same process.
	uint32						_LoadingCounter;
	/// The library context
	class CLibraryContext		*_LibContext;

	/** Called by CLibrary after a successful loadLibrary
	 *	Note : this methods MUST be virtual to ensure
	 *	that the code executed is the one from
	 *	the loaded library.
	 */
	virtual void _onLibraryLoaded(class INelContext &nelContext);
	/** Called by the CLibrary class before unloading the library.
	 *	Note : this methods MUST be virtual to ensure
	 *	that the code executed is the one from
	 *	the loaded library.
	 */
	virtual void _onLibraryUnloaded();
public:

	INelLibrary()
		: _LoadingCounter(0),
		_LibContext(NULL)
	{}

	virtual ~INelLibrary();

	/// Return the loading counter value
	uint32	getLoadingCounter();

	/** Called after each loading of the library.
	 *	firstTime is true if this is the first time that
	 *	this library is loaded in the current process.
	 *	If the process load more than once the same
	 *	library, subsequent call to this method will
	 *	have firstTime false.
	 *	Implement this method to initialize think or whatever.
	 */
	virtual void onLibraryLoaded(bool firstTime) =0;
	/** Called after before each unloading of the library.
	 *	lastTime is true if the library will be effectively
	 *	unmapped from the process memory.
	 *	If the library have been loaded more than once,
	 *	then lastTime is false, indicating that the
	 *	library will still be in the process memory
	 *	space after the call.
	 */
	virtual void onLibraryUnloaded(bool lastTime) =0;
};

#define NLMISC_PURE_LIB_ENTRY_POINT	NelLibraryEntry

#define NLMISC_DECL_PURE_LIB(className) \
	className	_PureLibraryEntryInstance; \
	NL_LIB_EXPORT_SYMBOL( NLMISC_PURE_LIB_ENTRY_POINT, NLMISC::INelLibrary, &_PureLibraryEntryInstance)

} // NLMISC

#endif // NL_DYNLIBLOAD_H
