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

#ifndef STRING_MAPPER_H
#define STRING_MAPPER_H

#include "types_nl.h"
#include "stream.h"
#include "mutex.h"

#include <vector>
#include <set>

namespace NLMISC
{

// const string *  as  uint (the TStringId returned by CStringMapper is a pointer to a string object)
//#ifdef HAVE_X86_64
//typedef uint64 TStringId;
//#else
//typedef uint TStringId;
//#endif

typedef	const std::string *TStringId;

// Traits for hash_map using CStringId
struct CStringIdHashMapTraits
{
	enum { bucket_size = 4, min_buckets = 8 };
	CStringIdHashMapTraits() { }
	size_t operator() (const NLMISC::TStringId &stringId) const
	{
		return	(size_t)stringId;
	}
	bool operator() (const NLMISC::TStringId &strId1, const NLMISC::TStringId &strId2) const
	{
		return (size_t)strId1 < (size_t)strId2;
	}
};

/** A static class that map string to integer and vice-versa
 * Each different string is tranformed into an unique integer identifier.
 * If the same string is submited twice, the same id is returned.
 * The class can also return the string associated with an id.
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2003
 */
class CStringMapper
{
	class CCharComp
	{
	public:
		bool operator()(std::string *x, std::string *y) const
		{
			return (*x) < (*y);
		}
	};

	class CAutoFastMutex
	{
		CFastMutex		*_Mutex;
	public:
		CAutoFastMutex(CFastMutex *mtx) : _Mutex(mtx)	{_Mutex->enter();}
		~CAutoFastMutex() {_Mutex->leave();}
	};

	// Local Data
	std::set<std::string*,CCharComp>	_StringTable;
	std::string*			_EmptyId;
	CFastMutex				_Mutex;		// Must be thread-safe (Called by CPortal/CCluster, each of them called by CInstanceGroup)

	// The 'singleton' for static methods
	static	CStringMapper	_GlobalMapper;

	// private constructor.
	CStringMapper();

public:

	~CStringMapper()
	{
		localClear();
	}

	/// Globaly map a string into a unique Id. ** This method IS Thread-Safe **
	static TStringId			map(const std::string &str) { return _GlobalMapper.localMap(str); }
	/// Globaly unmap a string. ** This method IS Thread-Safe **
	static const std::string	&unmap(const TStringId &stringId) { return _GlobalMapper.localUnmap(stringId); }
	/// Globaly helper to serial a string id. ** This method IS Thread-Safe **
	static void					serialString(NLMISC::IStream &f, TStringId &id) {_GlobalMapper.localSerialString(f, id);}
	/// Return the global id for the empty string (helper function). NB: Works with every instance of CStringMapper
	static TStringId			emptyId() { return 0; }

	// ** This method IS Thread-Safe **
	static void					clear() { _GlobalMapper.localClear(); }

	/// Create a local mapper. You can dispose of it by deleting it.
	static CStringMapper *	createLocalMapper();
	/// Localy map a string into a unique Id
	TStringId				localMap(const std::string &str);
	/// Localy unmap a string
	const std::string		&localUnmap(const TStringId &stringId) { return (stringId==0)?*_EmptyId:*((std::string*)stringId); }
	/// Localy helper to serial a string id
	void					localSerialString(NLMISC::IStream &f, TStringId &id);

	void					localClear();

};

// linear from 0 (0 is empty string) (The TSStringId returned by CStaticStringMapper
// is an index in the vector and begin at 0)
typedef uint TSStringId;

/**
 * After endAdd you cannot add strings anymore or it will assert
 * \author Matthieu Besson
 * \author Nevrax France
 * \date November 2003
 */
class CStaticStringMapper
{

	std::map<std::string, TSStringId>	_TempStringTable;
	std::map<TSStringId, std::string>	_TempIdTable;

	uint32	_IdCounter;
	char	*_AllStrings;
	std::vector<char*>	_IdToStr;
	bool _MemoryCompressed; // If false use the 2 maps

public:

	CStaticStringMapper()
	{
		_IdCounter = 0;
		_AllStrings = NULL;
		_MemoryCompressed = false;
		add("");
	}

	~CStaticStringMapper()
	{
		clear();
	}

	/// Globaly map a string into a unique Id
	TSStringId			add(const std::string &str);

	// see if a string is already present in the map
	bool				isAdded(const std::string &str) const;

	void				memoryCompress();
	// Uncompress the map.
	void				memoryUncompress();
	/// Globaly unmap a string
	const char *		get(TSStringId stringId);
	/// Return the global id for the empty string (helper function)
	static TSStringId	emptyId() { return 0; }

	void				clear();

	uint32 getCount() { return _IdCounter; }

	// helper serialize a string id as a string
	void				serial(NLMISC::IStream &f, TSStringId &strId);

	// helper serialize a string id vector
	void				serial(NLMISC::IStream &f, std::vector<TSStringId> &strIdVect);

};

} //namespace NLMISC

#endif // STRING_MAPPER_H

