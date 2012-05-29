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

#error "deprecated"

//#include "pd_string_manager.h"
//#include "pd_lib.h"
//
//#include <string.h>
//
//#include <nel/misc/file.h>
//#include <nel/misc/command.h>
//#include <nel/net/unified_network.h>
//#include <nel/net/service.h>
//
//#include "pd_lib.h"
//#include "timestamp.h"
//
//using namespace std;
//using namespace NLMISC;
//using namespace NLNET;
//
//
//namespace RY_PDS 
//{
//
////
////
//
////void	cbStoreStringResult(CMessage& msgin, const string &serviceName, uint16 serviceId);
//
////TUnifiedCallbackItem	PDStringManagerCbArray[] =
////{
////	{ "STORE_STRING_RESULT", cbStoreStringResult },
////};
//
////void	cbStoreStringResult(CMessage& msgin, const string &serviceName, uint16 serviceId)
////{
////	if (CPDStringManager::_NbProcessingStoreStringRequests)
////	{
////		CPDStringManager::_NbProcessingStoreStringRequests--;
////	}
////	else
////		nlwarning("<cbStoreStringResult> received more STORE_STRING_RESULT messages than sent STORE_STRING requests");
////
////	ucstring			str;
////	uint32				stringId;
////
////	msgin.serial(str);
////	msgin.serial(stringId);
////
////	uint	i;
////	for (i=0; i<CPDSLib::_Libs.size(); ++i)
////	{
////		CPDSLib*	lib = CPDSLib::_Libs[i];
////		if (lib != NULL)
////			lib->getStringManager().setStringId(str, stringId);
////	}
////}
//
//
//// Is net callback ready?
//bool				CPDStringManager::_InitCb = false;
//
///// Null String
//ucstring			CPDStringManager::_NullStr;
//
///// Number of store string requests that have not been processed by IOS yet
//uint				CPDStringManager::_NbProcessingStoreStringRequests = 0;
//
//
///*
// * Constructor
// */
//CPDStringManager::CPDStringManager()
//{
//	_PDSLib = NULL;
//	_Callback = NULL;
//}
//
//
///*
// * Destructor
// */
//CPDStringManager::~CPDStringManager()
//{
//}
//
//
///*
// * Init
// */
//void	CPDStringManager::init(CPDSLib* lib)
//{
//	//
//	if (!_InitCb)
//	{
//		_InitCb = true;
//		CUnifiedNetwork::getInstance()->addCallbackArray(PDStringManagerCbArray, sizeof(PDStringManagerCbArray)/sizeof(PDStringManagerCbArray[0]));
//	}
//
//	//
//	_PDSLib = lib;
//}
//
//
///*
// * Get string entry (allocate a new entry if not yet in container)
// */
//CPDStringManager::TEntryId	CPDStringManager::getEntryIdNonConst(const ucstring& str)
//{
//	THash		hash = getHash(str);
//	TEntryId	entryId = InvalidEntryId;
//
//	if (_HashTable.size() > hash)
//		entryId = _HashTable[hash];
//
//	while (entryId != InvalidEntryId)
//	{
//		const CStringEntry&	entry = _StringEntries[entryId];
//
//		if (entry.String == str)
//		{
//			// DONE BY NICO : this forces the lib to update the string id value of the entry. Thus, EGS callbacks are called.
//			// The real problem is that entries are never removed from _StringEntries. So when we add a string that should have been previously removed, setStringId is never called... )
//			setStringId(str, entry.StringId);
//			return entryId;
//		}
//
//		entryId = entry.NextInHash;
//	}
//
//	// allocate new entry
//	entryId = _StringEntries.size();
//	_StringEntries.resize(entryId+1);
//
//	// check hash table size
//	if (_HashTable.size() <= hash)
//		_HashTable.resize(PD_STRING_HASHTABLE_SIZE, InvalidEntryId);
//
//	// fill new entry
//	CStringEntry&	entry = _StringEntries[entryId];
//
//	entry.String = str;
//	entry.Hash = hash;
//	entry.NextInHash = _HashTable[hash];
//
//	// link entry in hash table
//	_HashTable[hash] = entryId;
//
//	if (_PDSLib != NULL)
//	{
//		storeStringInIOS(str);
//	}
//
//	return entryId;
//}
//
//
///*
// * Compares 2 ucchar strings
// */
//bool	CPDStringManager::compare(const ucchar* a, const ucchar* b)
//{
//	while (*a == *b && *a != 0)
//	{
//		++a;
//		++b;
//	}
//	return *a == *b;
//}
//
//
//
//
///*
// * Explicitly add string to manager
// * \param eid is the associated entity id
// * \param string is the string to add
// * \return the persistant string id
// */
//void	CPDStringManager::addString(NLMISC::CEntityId eid, const ucstring& str, bool addToLog)
//{
//	TEntryId		entry = getEntryIdNonConst(str);
//
//	if (_PDSLib != NULL)
//	{
//		// client mode, send mapping to PDS
//		_PDSLib->addString(eid, str);
//	}
//
//	if ((_PDSLib == NULL && addToLog) || (_PDSLib != NULL && !_PDSLib->PDSUsed() && PDEnableStringLog))
//	{
//		// PDS mode, log mapping
//		_Log.Actions.push_back(CLogAction());
//		_Log.Actions.back().MapId = true;
//		_Log.Actions.back().EntityId = eid;
//		_Log.Actions.back().String = str;
//	}
//
//	mapEid(eid, entry);
//}
//
///*
// * Unmap EntityId
// */
//void	CPDStringManager::unmap(NLMISC::CEntityId eid, bool addToLog)
//{
//	TEIdMap::iterator	it = _EIdMap.find(eid);
//	if (it == _EIdMap.end())
//		return;
//
//	if (_PDSLib != NULL)
//	{
//		// client mode, send unmapping to PDS
//		_PDSLib->unmapString(eid);
//	}
//
//	if ((_PDSLib == NULL && addToLog) || (_PDSLib != NULL && !_PDSLib->PDSUsed() && PDEnableStringLog))
//	{
//		// PDS mode, log unmapping
//		_Log.Actions.push_back(CLogAction());
//		_Log.Actions.back().MapId = false;
//		_Log.Actions.back().EntityId = eid;
//	}
//
//	CStringEntry&	entry = _StringEntries[(*it).second];
//
//	_EIdMap.erase(it);
//
//	// remove from mapped eids
//	std::vector<NLMISC::CEntityId>::iterator	ite;
//	for (ite=entry.MappedIds.begin(); ite!=entry.MappedIds.end(); )
//	{
//		if (*ite == eid)
//			ite = entry.MappedIds.erase(ite);
//		else
//			++ite;
//	}
//}
//
//
///*
// * Set string callback
// * This callback is called when the string id is received from the IOS.
// * That is when string id is ready to be used
// */
//void	CPDStringManager::setCallback(TStringCallback callback)
//{
//	_Callback = callback;
//}
//
//
//
//
///*
// * Display Manager content
// */
//void	CPDStringManager::display(NLMISC::CLog* log) const
//{
//}
//
//
//
///*
// * Serial String Manager
// */
//void	CPDStringManager::serial(NLMISC::IStream& f)
//{
//	H_AUTO( PDSSM_SERIAL );
//	f.serialCheck((uint32)'PDSM');
//	uint	version = f.serialVersion(0);
//
//	{
//		H_AUTO( PDSSM_SERIAL_ENTRIES );
//		f.serialCont(_StringEntries);
//	}
//	{
//		H_AUTO( PDSSM_SERIAL_EIDMAP );
//		f.serialCont(_EIdMap);
//	}
//
//	if (f.isReading())
//	{
//		H_AUTO( PDSSM_SERIAL_BUILDVOLATILEDATA );
//		buildVolatileData();
//	}
//}
//
//
///*
// * Set String (PDS side)
// */
///*
//bool	CPDStringManager::setString(const NLMISC::CEntityId& eid, TEntryId id, const ucstring& str)
//{
//	bool	success = true;
//
//	if (id >= _StringEntries.size())
//	{
//		// alloc string entry
//		_StringEntries.resize(id+1);
//
//		// fill up entry
//		CStringEntry&	entry = _StringEntries[id];
//
//		entry.Hash = getHash(str);
//		entry.String = str;
//
//		// link in hash table
//		if (_HashTable.size() <= entry.Hash)
//			_HashTable.resize(PD_STRING_HASHTABLE_SIZE, InvalidEntryId);
//
//		entry.NextInHash = _HashTable[entry.Hash];
//	}
//	else if (_StringEntries[id].String != str)
//	{
//		// entry was alread filled !
//		nlwarning("CPDStringManager::setString(): entry '%d' already set, entry is kept unchanged", id);
//		success = false;
//	}
//
//	_EIdMap[eid] = id;
//
//	return success;
//}
//*/
//
//
//
///*
// * Set string Id
// */
//void	CPDStringManager::setStringId(const ucstring& str, TStringId id)
//{
//	THash		hash = getHash(str);
//	TEntryId	entryId = InvalidEntryId;
//
//	if (_HashTable.size() >= hash)
//	{
//		entryId = _HashTable[hash];
//
//		while (entryId != InvalidEntryId)
//		{
//			CStringEntry&	entry = _StringEntries[entryId];
//
//			if (entry.String == str)
//			{
//				entry.StringId = id;
//				_StringIdMap[id] = entryId;
//				break;
//			}
//			entryId = entry.NextInHash;
//		}
//	}
//
//	// callback
//	if (_Callback != NULL)
//		_Callback(str, id);
//}
//
//
//
///*
// * Build All String Associations
// */
//void	CPDStringManager::buildStringAssociation()
//{
//	uint	i;
//	for (i=0; i<CPDSLib::_Libs.size(); ++i)
//	{
//		CPDSLib*	lib = CPDSLib::_Libs[i];
//		if (lib != NULL)
//			lib->getStringManager().askAssociations();
//	}
//}
//
//
///*
// * Ask Associations
// */
//void	CPDStringManager::askAssociations()
//{
//	uint	i;
//	for (i=0; i<_StringEntries.size(); ++i)
//	{
//		storeStringInIOS(_StringEntries[i].String);
//	}
//}
//
//
//
//
///*
// * Rebuild volatile data
// */
//void	CPDStringManager::buildVolatileData()
//{
//	static bool alreadyBuilt = false;
//
////#ifdef NL_DEBUG
////	if(alreadyBuilt)
////		nlstopex(("buildVolatileData() called 2 times, BEN, please fix me!!!"));
////#else
//	if(alreadyBuilt)
//		nlwarning("buildVolatileData() called 2 times, BEN, please fix me!!!");
////#endif
//
//	_HashTable.clear();
//
//	uint	i;
//	for (i=0; i<_StringEntries.size(); ++i)
//	{
//		CStringEntry&	entry = _StringEntries[i];
//
//		entry.Hash = getHash(entry.String);
//
//		if (_HashTable.size() <= entry.Hash)
//			_HashTable.resize(PD_STRING_HASHTABLE_SIZE, InvalidEntryId);
//
//		entry.NextInHash = _HashTable[entry.Hash];
//		_HashTable[entry.Hash] = i;
//
//		entry.MappedIds.clear();
//	}
//
//	TEIdMap::const_iterator	it;
//	for (it=_EIdMap.begin(); it!=_EIdMap.end(); ++it)
//	{
//		CStringEntry&	entry = _StringEntries[(*it).second];
//		entry.MappedIds.push_back((*it).first);
//	}
//
//	alreadyBuilt = true;
//}
//
//
///*
// * Load String manager default file (to be used by client when PDS is not connected)
// */
//bool	CPDStringManager::load()
//{
//	if (_PDSLib == NULL)
//		return false;
//
//	std::string	logDir = _PDSLib->getLogDirectory();
//
//	if (!CFile::isDirectory(logDir))
//	{
//		if (!CFile::createDirectoryTree(logDir))
//		{
//			nlwarning("Failed to create log root directory '%s'", logDir.c_str());
//		}
//
//		if (!CFile::setRWAccess(logDir))
//		{
//			nlwarning("Failed, can't set RW access to directory '%s'", logDir.c_str());
//		}
//	}
//
//	return load(logDir);
//}
//
///*
// * Save String manager default file (to be used by client when PDS is not connected)
// */
//bool	CPDStringManager::save()
//{
//	if (_PDSLib == NULL)
//		return false;
//
//	std::string	logDir = _PDSLib->getLogDirectory();
//
//	if (!CFile::isDirectory(logDir))
//	{
//		if (!CFile::createDirectoryTree(logDir))
//		{
//			nlwarning("Failed to create log root directory '%s'", logDir.c_str());
//		}
//
//		if (!CFile::setRWAccess(logDir))
//		{
//			nlwarning("Failed, can't set RW access to directory '%s'", logDir.c_str());
//		}
//	}
//
//	return save(logDir);
//}
//
///*
// * Load String manager file
// */
//bool	CPDStringManager::load(const std::string& path)
//{
//	std::string	filename = getFile(path);
//
//	if (!CFile::fileExists(filename))
//	{
//		nlinfo("CPDStringManager::load(): file '%s' doesn't exist, assumes string manager empty", filename.c_str());
//		return true;
//	}
//
//	CIFile	file;
//	if (!file.open(filename))
//	{
//		nlwarning("CPDStringManager::load(): failed to load file '%s', string manager left as is", filename.c_str());
//		return false;
//	}
//
//	try
//	{
//		serial(file);
//	}
//	catch (Exception& e)
//	{
//		nlwarning("CPDStringManager::load(): failed to load file '%s', exception in serial '%s'", filename.c_str(), e.what());
//		return false;
//	}
//
//	return true;
//}
//
///*
// * Save String manager file
// */
//bool	CPDStringManager::save(const std::string& path)
//{
//	std::string	filename = getFile(path);
//
//	COFile	file;
//	if (!file.open(filename))
//	{
//		nlwarning("CPDStringManager::save(): failed to save file '%s', string manager left as is", filename.c_str());
//		return false;
//	}
//
//	try
//	{
//		serial(file);
//	}
//	catch (Exception& e)
//	{
//		nlwarning("CPDStringManager::save(): failed to save file '%s', exception in serial '%s'", filename.c_str(), e.what());
//		return false;
//	}
//
//	return true;
//}
//
///*
// * Get String manager filename
// */
//std::string	CPDStringManager::getFile(const std::string& path)
//{
//	return CPath::standardizePath(path) + "string_manager.bin";
//}
//
//
///*
// * Apply log
// */
//bool	CPDStringManager::applyLog(NLMISC::IStream& s)
//{
//	try
//	{
//		s.serial(_Log);
//	}
//	catch (Exception &e)
//	{
//		nlwarning("CPDStringManager::applyLog(): failed to apply log from stream, exception in serial '%s'", e.what());
//		return false;
//	}
//
//	uint	i;
//	for (i=0; i<_Log.Actions.size(); ++i)
//	{
//		CLogAction&	action = _Log.Actions[i];
//
//		if (action.MapId)
//		{
//			addString(action.EntityId, action.String, false);
//		}
//		else
//		{
//			unmap(action.EntityId, false);
//		}
//	}
//
//	_Log.Actions.clear();
//
//	return true;
//}
//
///*
// * Store log
// */
//bool	CPDStringManager::storeLog(NLMISC::IStream& s)
//{
//	try
//	{
//		s.serial(_Log);
//	}
//	catch (Exception &e)
//	{
//		nlwarning("CPDStringManager::storeLog(): failed to store log to stream, exception in serial '%s'", e.what());
//		return false;
//	}
//
//	_Log.Actions.clear();
//
//	return true;
//}
//
///*
// * Is log file
// */
//bool	CPDStringManager::isLogFileName(const std::string& filename, CTimestamp& timestamp)
//{
//	char	buffer[32];
//	if (NLMISC::CFile::getExtension(filename) != "xml" ||
//		sscanf(NLMISC::CFile::getFilenameWithoutExtension(filename).c_str(), "string_%s", &buffer) != 1)
//		return false;
//	timestamp.fromString(buffer);
//	return true;
//}
//
///*
// * Store string in IOS
// */
//void	CPDStringManager::storeStringInIOS(const ucstring& str)
//{
//	CMessage msgios("STORE_STRING");
//	msgios.serial( const_cast<ucstring&>(str) );
//	CUnifiedNetwork::getInstance()->send("IOS", msgios);
//
//	_NbProcessingStoreStringRequests++;
//}
//
///*
// * Returns true if IOS has not processed all store string requests
// */
//bool	CPDStringManager::isWaitingIOSStoreStringResult()
//{
//	return (_NbProcessingStoreStringRequests != 0);
//}
//
//}
//
