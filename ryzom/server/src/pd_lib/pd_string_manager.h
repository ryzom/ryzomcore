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

#ifndef NL_PD_STRING_MANAGER_H
#define NL_PD_STRING_MANAGER_H

#error "Deprecated"

//#include <nel/misc/types_nl.h>
//#include <nel/misc/entity_id.h>
//#include <nel/misc/ucstring.h>
//#include <nel/misc/stream.h>
//#include <nel/misc/debug.h>
//#include <nel/misc/path.h>
//#include "nel/misc/hierarchical_timer.h"


//class CTimestamp;
//
//namespace NLNET
//{
//	class CMessage;
//}
//
//namespace RY_PDS 
//{
//
//// must a a power of 2!!
//const uint32	PD_STRING_HASHTABLE_SIZE = 65536;
//
//// invalid string id
//const uint32	PD_INVALID_STRING_ID = 0xffffffff;
//const uint32	PD_INVALID_STRING_PID = 0xffffffff;
//class CPDSLib;
//
///**
// * Persistant Data String Manager
// * \author Benjamin Legros
// * \author Nevrax France
// * \date 2003
// */
//class CPDStringManager
//{
//public:
//
//	/// Constructor
//	CPDStringManager();
//
//	/// Destructor
//	~CPDStringManager();
//
//
//	/// Persistant string entry id
//	typedef uint32			TEntryId;
//
//	/// String Id type -- session Id
//	typedef uint32			TStringId;
//
//	/// String Ready callback
//	typedef void			(*TStringCallback)(const ucstring& string, TStringId id);
//
//
//	/**
//	 * Init
//	 */
//	void					init(CPDSLib* lib);
//
//	/**
//	 * Build All String Associations
//	 */
//	static void				buildStringAssociation();
//
//
//
//	/**
//	 * Explicitly add string to manager
//	 * \param eid is the associated entity id
//	 * \param string is the string to add
//	 * \return the persistant string id
//	 */
//	void					addString(NLMISC::CEntityId eid, const ucstring& str, bool addToLog = true);
//
//	/**
//	 * Unmap EntityId
//	 */
//	void					unmap(NLMISC::CEntityId eid, bool addToLog = true);
//
//
//
//
//	/**
//	 * Set string callback
//	 * This callback is called when the string id is received from the IOS.
//	 * That is when string id is ready to be used
//	 */
//	void					setCallback(TStringCallback callback);
//
//	/**
//	 * Get String
//	 */
//	const ucstring&			getString(const NLMISC::CEntityId &eid) const;
//
//	/**
//	 * Get String
//	 */
//	const ucstring&			getString(TStringId id) const;
//
//	/**
//	 * Get Session String Id (from the ucstring itself)
//	 */
//	TStringId				getStringId(const ucstring& str) const;
//
//	/**
//	 * Get Session String Id (from the string entity id)
//	 */
//	TStringId				getStringId(const NLMISC::CEntityId& eid) const;
//
//
//	/**
//	 * Get Mapped Ids
//	 */
//	bool					getMappedIds(const ucstring& str, std::vector<NLMISC::CEntityId>& ids) const;
//
//	/**
//	 * Does string exists with given type
//	 */
//	bool					stringExists(const ucstring& str, uint8 type, NLMISC::CEntityId* foundEid = NULL) const;
//
//
//
//
//	/**
//	 * Display Manager content
//	 */
//	void					display(NLMISC::CLog* log = NLMISC::InfoLog) const;
//
//
//
//
//	/**
//	 * Serial String Manager
//	 */
//	void					serial(NLMISC::IStream& f);
//
//
//
//
//
//
//
//	enum
//	{
//		InvalidStringId = 0xffffffff, 
//		InvalidEntryId = 0xffffffff
//	};
//
//
//	/**
//	 * Set string Id
//	 */
//	void					setStringId(const ucstring& str, TStringId id);
//
//
//	/// Load String manager default file (to be used by client when PDS is not connected)
//	bool					load();
//
//	/// Save String manager default file (to be used by client when PDS is not connected)
//	bool					save();
//
//
//	/// Load String manager file
//	bool					load(const std::string& path);
//
//	/// Save String manager file
//	bool					save(const std::string& path);
//
//	/// Is Log Empty?
//	bool					logEmpty() const					{ return _Log.empty(); }
//
//	/// Apply log
//	bool					applyLog(NLMISC::IStream& s);
//
//	/// Store log
//	bool					storeLog(NLMISC::IStream& s);
//
//	/// Get String manager filename
//	std::string				getFile(const std::string& path);
//
//	/// Is log file
//	static bool				isLogFileName(const std::string& filename, CTimestamp& timestamp);
//
//	/// Store string in IOS
//	static void				storeStringInIOS(const ucstring& str);
//
//	/// Returns true if IOS has not processed all store string requests
//	static bool				isWaitingIOSStoreStringResult();
//
//private:
//
//	friend void	cbStoreStringResult(NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId);
//
//	/// Is net callback  ready?
//	static bool				_InitCb;
//
//	/// Null String
//	static ucstring			_NullStr;
//
//	/// Number of store string requests that have not been processed by IOS yet
//	static uint				_NbProcessingStoreStringRequests;
//
//
//	/// Master Lib
//	CPDSLib*				_PDSLib;
//
//	/// Use PDS
//	bool					_UsePDS;
//
//	/// Callback for StringId
//	TStringCallback			_Callback;
//
//
//
//
//
//
//
//	/// \name String Container Management
//	// @{
//
//	typedef uint16		THash;
//
//	class CStringEntry
//	{
//	public:
//
//		CStringEntry() : Hash(0), NextInHash(InvalidEntryId), StringId(InvalidStringId)		{ }
//
//		/// String in entry (this value is persistent)
//		ucstring						String;
//
//		// following values are not persistent and can be rebuilt from persistent values
//		/// Hash code of the string (see getHash() static method)
//		THash							Hash;
//		/// Next string entry in the Hashtable entry (e.g. the next string that has the same hash code)
//		TEntryId						NextInHash;
//		/// IOS string id
//		TStringId						StringId;
//		/// list of EntityIds mapped to this string
//		std::vector<NLMISC::CEntityId>	MappedIds;
//
//		void		serial(NLMISC::IStream& s)
//		{
//			H_AUTO( PDSSM_STRING_ENTRY );
//			if (s.isReading())
//			{
//				std::string	utf8;
//				s.serial(utf8);
//				String.fromUtf8(utf8);
//			}
//			else
//			{
//				std::string	utf8;
//				{
//					H_AUTO( PDSSM_STRING_ENTRY_TO_UTF8 );
//					utf8 = String.toUtf8();
//				}
//				s.serial(utf8);
//			}
//		}
//	};
//
//	/// Mapped Strings
//	std::vector<CStringEntry>		_StringEntries;
//
//	/// HashTable
//	std::vector<TEntryId>			_HashTable;
//
//	/// Get string entry (allocate a new entry if not yet in container)
//	TEntryId		getEntryIdNonConst(const ucstring& str);
//
//	/// Get string entry only if exists
//	TEntryId		getEntryId(const ucstring& str) const;
//
//	// @}
//
//
//
//
//
//
//	/// \name CEntityId Mapping
//	// @{
//
//	typedef std::map<NLMISC::CEntityId, TEntryId>	TEIdMap;
//
//	/// Mapping of entities to string
//	TEIdMap							_EIdMap;
//
//	/// Map eid to string entry
//	void			mapEid(const NLMISC::CEntityId& eid, TEntryId id);
//
//	// @}
//
//
//
//
//
//
//	/// \name StringId Mapping
//	// @{
//
//	typedef std::map<TStringId, TEntryId>	TStringIdMap;
//
//	/// Mapping of entities to string
//	TStringIdMap					_StringIdMap;
//
//	// @}
//
//
//
//
//
//	/// \name Persistence logging
//	// @{
//
//	/// Atomic Action in the string manager
//	class CLogAction
//	{
//	public:
//
//		CLogAction() : MapId(false)	{ }
//
//		/// Is Log a mapping
//		bool						MapId;
//
//		/// Mapped Entity Id
//		NLMISC::CEntityId			EntityId;
//
//		/// Mapped String
//		ucstring					String;
//
//		void		serial(NLMISC::IStream& f)
//		{
//			f.xmlPush("logaction");
//
//				f.xmlPush("mapid");
//				f.serial(MapId);
//				f.xmlPop();
//
//				f.xmlPush("entityid");
//				f.serial(EntityId);
//				f.xmlPop();
//
//				if (MapId)
//				{
//					f.xmlPush("string");
//					if (f.isReading())
//					{
//						std::string	utf8;
//						f.serial(utf8);
//						String.fromUtf8(utf8);
//					}
//					else
//					{
//						std::string	utf8 = String.toUtf8();
//						f.serial(utf8);
//					}
//					f.xmlPop();
//				}
//
//			f.xmlPop();
//		}
//	};
//
//	/// Log of the string manager state
//	class CLog
//	{
//	public:
//
//		/// Is Log empty?
//		bool		empty() const				{ return Actions.empty(); }
//
//		/// List of atomic actions
//		std::vector<CLogAction>		Actions;
//
//		void		serial(NLMISC::IStream& f)
//		{
//			f.xmlPush("stringlog");
//			f.serialCont(Actions);
//			f.xmlPop();
//		}
//	};
//
//	CLog			_Log;
//
//	// @}
//
//
//
//
//
//	/// Get String Hash
//	static THash	getHash(const ucstring& str);
//
//	/// Get String Hash
//	static THash	getHash(const ucchar* str);
//
//	/// Compares 2 ucchar strings
//	static bool		compare(const ucchar* a, const ucchar* b);
//
//
//	/// Ask Associations
//	void			askAssociations();
//
//
//	/// Rebuild volatile data
//	void			buildVolatileData();
//};
//
//
///*
// * Get string entry only if exists
// */
//inline CPDStringManager::TEntryId	CPDStringManager::getEntryId(const ucstring& str) const
//{
//	THash		hash = getHash(str);
//	TEntryId	entryId = InvalidEntryId;
//
//	if (_HashTable.size() < hash)
//		return InvalidEntryId;
//
//	for (entryId=_HashTable[hash];
//		 entryId != InvalidEntryId && _StringEntries[entryId].String != str;
//		 entryId = _StringEntries[entryId].NextInHash)
//		;
//
//	return entryId;
//}
//
//
///*
// * Get String Hash
// */
//inline CPDStringManager::THash	CPDStringManager::getHash(const ucstring& str)
//{
//	return getHash(str.c_str());
//	//return str.empty() ? (THash)0 : getHash(str.c_str());
//}
//
///*
// * Get String Hash
// */
//inline CPDStringManager::THash	CPDStringManager::getHash(const ucchar* str)
//{
//	// classic hash formula sum(i=0..n, str[i]*31^(n-i))
//	uint32		hash;
//	for (hash=0; *str != 0; ++str)
//		hash = ((hash << 5) - hash) + *str;
//
//	return (THash)(hash ^ (hash >> 16));
//}
//
//
///*
// * Get String
// */
//inline const ucstring&	CPDStringManager::getString(const NLMISC::CEntityId &eid) const
//{
//	TEIdMap::const_iterator	it = _EIdMap.find(eid);
//
//	return (it == _EIdMap.end()) ? _NullStr : _StringEntries[(*it).second].String;
//}
//
///*
// * Get String
// */
//inline const ucstring&	CPDStringManager::getString(TStringId id) const
//{
//	TStringIdMap::const_iterator	it = _StringIdMap.find(id);
//
//	return (it == _StringIdMap.end()) ? _NullStr : _StringEntries[(*it).second].String;
//}
//
///*
// * Get Session String Id (from the ucstring itself)
// */
//inline CPDStringManager::TStringId	CPDStringManager::getStringId(const ucstring& str) const
//{
//	TEntryId	entry = getEntryId(str);
//
//	return (entry == InvalidEntryId) ? PD_INVALID_STRING_ID : _StringEntries[entry].StringId;
//}
//
///*
// * Get Session String Id (from the string entity id)
// */
//inline CPDStringManager::TStringId	CPDStringManager::getStringId(const NLMISC::CEntityId& eid) const
//{
//	TEIdMap::const_iterator	it = _EIdMap.find(eid);
//
//	return (it == _EIdMap.end()) ? PD_INVALID_STRING_ID : _StringEntries[(*it).second].StringId;
//}
//
//
///*
// * Does string exists with given type
// */
//inline bool	CPDStringManager::stringExists(const ucstring& str, uint8 type, NLMISC::CEntityId* foundEid) const
//{
//	TEntryId	entry = getEntryId(str);
//
//	if (entry == InvalidEntryId)
//		return false;
//
//	const CStringEntry&	e = _StringEntries[entry];
//
//	uint	i;
//	for (i=0; i<e.MappedIds.size(); ++i)
//	{
//		if (e.MappedIds[i].getType() == type)
//		{
//			if (foundEid != NULL)
//				*foundEid = e.MappedIds[i];
//			return true;
//		}
//	}
//
//	return false;
//}
//
///*
// * Get Mapped Ids
// */
//inline bool	CPDStringManager::getMappedIds(const ucstring& str, std::vector<NLMISC::CEntityId>& ids) const
//{
//	TEntryId	entry = getEntryId(str);
//
//	if (entry == InvalidEntryId || _StringEntries[entry].MappedIds.empty())
//		return false;
//
//	ids = _StringEntries[entry].MappedIds;
//	return true;
//}
//
///*
// * Map eid to string entry
// */
//inline void	CPDStringManager::mapEid(const NLMISC::CEntityId& eid, TEntryId id)
//{
//	TEIdMap::iterator	it = _EIdMap.find(eid);
//	if (it != _EIdMap.end())
//	{
//		// unmap first
//		CStringEntry&	prev = _StringEntries[(*it).second];
//
//		// remove from mapped eids
//		std::vector<NLMISC::CEntityId>::iterator	ite;
//		for (ite=prev.MappedIds.begin(); ite!=prev.MappedIds.end(); )
//		{
//			if (*ite == eid)
//				ite = prev.MappedIds.erase(ite);
//			else
//				++ite;
//		}
//
//		// map to new entry
//		(*it).second = id;
//	}
//	else
//	{
//		// map directly in eid map
//		_EIdMap[eid] = id;
//	}
//
//	_StringEntries[id].MappedIds.push_back(eid);
//}
//
//
//}

#endif // NL_PD_STRING_MANAGER_H

/* End of pd_string_manager.h */
