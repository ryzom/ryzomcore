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
#include "egs_pd.h"

namespace EGSPD
{
	
static const struct { const char* Name; CFameTrend::TFameTrend Value; } TFameTrendConvert[] =
{
	{ "FameUpward", CFameTrend::FameUpward },
	{ "FameDownward", CFameTrend::FameDownward },
	{ "FameSteady", CFameTrend::FameSteady },
};
/* -----------------------------------------
* Static Implementation of CFameTrend
* ----------------------------------------- */
void							CFameTrend::init()
{
	_StrTable.clear();
	_ValueMap.clear();
	_StrTable.resize(3);
	uint	i;
	for (i=0; i<3; ++i)
	{
		_StrTable[TFameTrendConvert[i].Value] = TFameTrendConvert[i].Name;
		_ValueMap[NLMISC::toLower(std::string(TFameTrendConvert[i].Name))] = TFameTrendConvert[i].Value;
	}
	_Initialised = true;
}
bool							CFameTrend::_Initialised = false;
std::string						CFameTrend::_UnknownString = "Unknown";
std::vector<std::string>		CFameTrend::_StrTable;
std::map<std::string, CFameTrend::TFameTrend>	CFameTrend::_ValueMap;
// End of static implementation of CFameTrend

/* -----------------------------------------
* Static Implementation of CFameContainerEntryPD
* ----------------------------------------- */
NLMISC::CSheetId				CFameContainerEntryPD::getSheet() const
{
	return _Sheet;
}
sint32							CFameContainerEntryPD::getFame() const
{
	return _Fame;
}
void							CFameContainerEntryPD::setFame(sint32 __v, bool forceWrite)
{
	if ((_Fame != __v) || forceWrite)
	{
		PDSLib.set(0, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_Fame = __v;
}
sint32							CFameContainerEntryPD::getFameMemory() const
{
	return _FameMemory;
}
void							CFameContainerEntryPD::setFameMemory(sint32 __v, bool forceWrite)
{
	if ((_FameMemory != __v) || forceWrite)
	{
		PDSLib.set(0, __BaseRow, (RY_PDS::TColumnIndex)(2), __v);
	}
	_FameMemory = __v;
}
CFameTrend::TFameTrend			CFameContainerEntryPD::getLastFameChangeTrend() const
{
	return _LastFameChangeTrend;
}
void							CFameContainerEntryPD::setLastFameChangeTrend(CFameTrend::TFameTrend __v, bool forceWrite)
{
	nlassert(__v<CFameTrend::___TFameTrend_useSize);
	if ((_LastFameChangeTrend != __v) || forceWrite)
	{
		PDSLib.set(0, __BaseRow, (RY_PDS::TColumnIndex)(3), (uint32)__v);
	}
	_LastFameChangeTrend = __v;
}
CFameContainerPD*				CFameContainerEntryPD::getParent()
{
	return _Parent;
}
const CFameContainerPD*			CFameContainerEntryPD::getParent() const
{
	return _Parent;
}
void							CFameContainerEntryPD::clear()
{
	_Fame = 0;
	PDSLib.set(0, __BaseRow, (RY_PDS::TColumnIndex)(1), 0);
	_FameMemory = 0;
	PDSLib.set(0, __BaseRow, (RY_PDS::TColumnIndex)(2), 0);
	_LastFameChangeTrend = (CFameTrend::TFameTrend)0;
	PDSLib.set(0, __BaseRow, (RY_PDS::TColumnIndex)(3), (uint32)(CFameTrend::TFameTrend)0);
}
CFameContainerEntryPD*			CFameContainerEntryPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 0) ? static_cast<CFameContainerEntryPD*>(obj) : NULL;
}
const CFameContainerEntryPD*	CFameContainerEntryPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 0) ? static_cast<const CFameContainerEntryPD*>(obj) : NULL;
}
void							CFameContainerEntryPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokSheet = __pdr.addString("Sheet");
	uint16	__TokFame = __pdr.addString("Fame");
	uint16	__TokFameMemory = __pdr.addString("FameMemory");
	uint16	__TokLastFameChangeTrend = __pdr.addString("LastFameChangeTrend");
	_Parent = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokSheet)
		{
			__pdr.pop(__TokSheet, _Sheet);
		}
		else if (__pdr.peekNextToken() == __TokFame)
		{
			__pdr.pop(__TokFame, _Fame);
		}
		else if (__pdr.peekNextToken() == __TokFameMemory)
		{
			__pdr.pop(__TokFameMemory, _FameMemory);
		}
		else if (__pdr.peekNextToken() == __TokLastFameChangeTrend)
		{
			{
				std::string	valuename;
				__pdr.pop(__TokLastFameChangeTrend, valuename);
				_LastFameChangeTrend = CFameTrend::fromString(valuename);
			}
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CFameContainerEntryPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokSheet = __pdr.addString("Sheet");
	uint16	__TokFame = __pdr.addString("Fame");
	uint16	__TokFameMemory = __pdr.addString("FameMemory");
	uint16	__TokLastFameChangeTrend = __pdr.addString("LastFameChangeTrend");
	__pdr.push(__TokSheet, _Sheet);
	__pdr.push(__TokFame, _Fame);
	__pdr.push(__TokFameMemory, _FameMemory);
	{
		std::string	valuename = CFameTrend::toString(_LastFameChangeTrend);
		__pdr.push(__TokLastFameChangeTrend, valuename);
	}
}
void							CFameContainerEntryPD::pds__init(const NLMISC::CSheetId &Sheet)
{
	_Sheet = Sheet;
	_Fame = 0;
	_FameMemory = 0;
	_LastFameChangeTrend = (CFameTrend::TFameTrend)0;
	_Parent = NULL;
}
void							CFameContainerEntryPD::pds__destroy()
{
}
void							CFameContainerEntryPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Sheet);
	data.serial(_Fame);
	data.serial(_FameMemory);
	data.serialEnum(_LastFameChangeTrend);
	_Parent = NULL;
}
void							CFameContainerEntryPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(0, __BaseRow, 0);
	pds__registerAttributes();
}
void							CFameContainerEntryPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CFameContainerEntryPD: registerAttributes %u:%u", 0, __BaseRow);
	PDSLib.set(0, __BaseRow, (RY_PDS::TColumnIndex)(0), _Sheet);
}
void							CFameContainerEntryPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(0, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CFameContainerEntryPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CFameContainerEntryPD: unregisterAttributes %u:%u", 0, __BaseRow);
	pds__setParent(NULL);
}
void							CFameContainerEntryPD::pds__setParent(CFameContainerPD* __parent)
{
	NLMISC::CEntityId	prevId;
	if (_Parent != NULL)
	{
		prevId = _Parent->_ContId;
	}
	_Parent = __parent;
	PDSLib.setParent(0, getRow(), (RY_PDS::TColumnIndex)(4), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()), (_Parent != NULL ? _Parent->getContId() : NLMISC::CEntityId::Unknown), prevId);
}
void							CFameContainerEntryPD::pds__setParentUnnotified(CFameContainerPD* __parent)
{
	_Parent = __parent;
}
void							CFameContainerEntryPD::pds__notifyInit()
{
}
void							CFameContainerEntryPD::pds__notifyRelease()
{
	PDSLib.release(0, __BaseRow);
}
void							CFameContainerEntryPD::pds_static__init()
{
	PDSLib.setIndexAllocator(0, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CFameContainerEntryPD::_IndexAllocator;
// End of static implementation of CFameContainerEntryPD

/* -----------------------------------------
* Static Implementation of CFameContainerPD
* ----------------------------------------- */
NLMISC::CEntityId				CFameContainerPD::getContId() const
{
	return _ContId;
}
CFameContainerEntryPD*			CFameContainerPD::getEntries(const NLMISC::CSheetId& __k)
{
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator _it = _Entries.find(__k);
	return (_it==_Entries.end() ? NULL : &((*_it).second));
}
const CFameContainerEntryPD*	CFameContainerPD::getEntries(const NLMISC::CSheetId& __k) const
{
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator _it = _Entries.find(__k);
	return (_it==_Entries.end() ? NULL : &((*_it).second));
}
std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator	CFameContainerPD::getEntriesBegin()
{
	return _Entries.begin();
}
std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator	CFameContainerPD::getEntriesEnd()
{
	return _Entries.end();
}
std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator	CFameContainerPD::getEntriesBegin() const
{
	return _Entries.begin();
}
std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator	CFameContainerPD::getEntriesEnd() const
{
	return _Entries.end();
}
const std::map<NLMISC::CSheetId, CFameContainerEntryPD> &	CFameContainerPD::getEntries() const
{
	return _Entries;
}
CFameContainerEntryPD*			CFameContainerPD::addToEntries(const NLMISC::CSheetId &__k)
{
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator	__it = _Entries.find(__k);
	if (__it == _Entries.end())
	{
		__it = _Entries.insert(std::map<NLMISC::CSheetId, CFameContainerEntryPD>::value_type(__k, CFameContainerEntryPD())).first;
		CFameContainerEntryPD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CFameContainerPD::deleteFromEntries(const NLMISC::CSheetId &__k)
{
	std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator	__it = _Entries.find(__k);
	if (__it == _Entries.end())	return;
	CFameContainerEntryPD&	__o = (*__it).second;
	__o.pds__unregister();
	_Entries.erase(__it);
}
uint32							CFameContainerPD::getLastGuildStatusChange() const
{
	return _LastGuildStatusChange;
}
void							CFameContainerPD::setLastGuildStatusChange(uint32 __v, bool forceWrite)
{
	if ((_LastGuildStatusChange != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(2), __v, _ContId);
	}
	_LastGuildStatusChange = __v;
}
uint32							CFameContainerPD::getLastFameChangeDate() const
{
	return _LastFameChangeDate;
}
void							CFameContainerPD::setLastFameChangeDate(uint32 __v, bool forceWrite)
{
	if ((_LastFameChangeDate != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(3), __v, _ContId);
	}
	_LastFameChangeDate = __v;
}
void							CFameContainerPD::clear()
{
	for (std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __it=_Entries.begin(); __it!=_Entries.end(); )
	{
		std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __itr=__it++;
		CFameContainerEntryPD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_Entries.clear();
	_LastGuildStatusChange = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(2), 0);
	_LastFameChangeDate = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(3), 0);
}
CFameContainerPD*				CFameContainerPD::cast(RY_PDS::IPDBaseData* obj)
{
	switch (obj->getTable())
	{
		case 1:
		case 2:
		return static_cast<CFameContainerPD*>(obj);
	}
	return NULL;
}
const CFameContainerPD*			CFameContainerPD::cast(const RY_PDS::IPDBaseData* obj)
{
	switch (obj->getTable())
	{
		case 1:
		case 2:
		return static_cast<const CFameContainerPD*>(obj);
	}
	return NULL;
}
CFameContainerPD*				CFameContainerPD::create(const NLMISC::CEntityId &ContId)
{
	CFameContainerPD	*__o = static_cast<CFameContainerPD*>(pds_static__factory());
	__o->pds__init(ContId);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CFameContainerPD::remove(const NLMISC::CEntityId& ContId)
{
	std::map<NLMISC::CEntityId,CFameContainerPD*>::iterator	it = _Map.find(ContId);
	if (it != _Map.end())
	{
		CFameContainerPD*	__o = (*it).second;
		__o->pds__notifyRelease();
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
}
void							CFameContainerPD::load(const NLMISC::CEntityId& ContId)
{
	PDSLib.load(1, ContId.asUint64());
}
void							CFameContainerPD::setLoadCallback(void (*callback)(const NLMISC::CEntityId& key, CFameContainerPD* object))
{
	__pds__LoadCallback = callback;
}
void							CFameContainerPD::unload(const NLMISC::CEntityId &ContId)
{
	std::map<NLMISC::CEntityId,CFameContainerPD*>::iterator	it = _Map.find(ContId);
	if (it != _Map.end())
	{
		CFameContainerPD*	__o = (*it).second;
		__o->pds__notifyRelease();
		__o->pds__destroy();
		delete __o;
	}
}
CFameContainerPD*				CFameContainerPD::get(const NLMISC::CEntityId &ContId)
{
	std::map<NLMISC::CEntityId, CFameContainerPD*>::iterator	__it = _Map.find(ContId);
	return (__it != _Map.end()) ? (*__it).second : NULL;
}
std::map<NLMISC::CEntityId, CFameContainerPD*>::iterator	CFameContainerPD::begin()
{
	return _Map.begin();
}
std::map<NLMISC::CEntityId, CFameContainerPD*>::iterator	CFameContainerPD::end()
{
	return _Map.end();
}
void							CFameContainerPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokContId = __pdr.addString("ContId");
	uint16	__TokEntries = __pdr.addString("Entries");
	uint16	__TokLastGuildStatusChange = __pdr.addString("LastGuildStatusChange");
	uint16	__TokLastFameChangeDate = __pdr.addString("LastFameChangeDate");
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokContId)
		{
			__pdr.pop(__TokContId, _ContId);
		}
		// apply Entries
		else if (__pdr.peekNextToken() == __TokEntries)
		{
			__pdr.popStructBegin(__TokEntries);
			while (!__pdr.isEndOfStruct())
			{
				NLMISC::CSheetId	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CFameContainerEntryPD&	obj = _Entries[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokEntries);
		}
		// end of apply Entries
		else if (__pdr.peekNextToken() == __TokLastGuildStatusChange)
		{
			__pdr.pop(__TokLastGuildStatusChange, _LastGuildStatusChange);
		}
		else if (__pdr.peekNextToken() == __TokLastFameChangeDate)
		{
			__pdr.pop(__TokLastFameChangeDate, _LastFameChangeDate);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
	pds__notifyInit();
}
void							CFameContainerPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokContId = __pdr.addString("ContId");
	uint16	__TokEntries = __pdr.addString("Entries");
	uint16	__TokLastGuildStatusChange = __pdr.addString("LastGuildStatusChange");
	uint16	__TokLastFameChangeDate = __pdr.addString("LastFameChangeDate");
	__pdr.push(__TokContId, _ContId);
	// store Entries
	__pdr.pushStructBegin(__TokEntries);
	for (std::map<NLMISC::CSheetId, CFameContainerEntryPD>::const_iterator it=_Entries.begin(); it!=_Entries.end(); ++it)
	{
		NLMISC::CSheetId	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokEntries);
	// end of store Entries
	__pdr.push(__TokLastGuildStatusChange, _LastGuildStatusChange);
	__pdr.push(__TokLastFameChangeDate, _LastFameChangeDate);
}
void							CFameContainerPD::pds__init(const NLMISC::CEntityId &ContId)
{
	_ContId = ContId;
	_LastGuildStatusChange = 0;
	_LastFameChangeDate = 0;
	_Map[getContId()] = this;
}
void							CFameContainerPD::pds__destroy()
{
	for (std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __it=_Entries.begin(); __it!=_Entries.end(); )
	{
		std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_Entries.clear();
	_Map.erase(getContId());
}
void							CFameContainerPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_ContId);
	RY_PDS::TTableIndex	tableIndex;
	RY_PDS::TRowIndex	rowIndex;
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		NLMISC::CSheetId	__k;
		data.serial(__k);
		_Entries.insert(std::make_pair<NLMISC::CSheetId,CFameContainerEntryPD>(__k, CFameContainerEntryPD()));
		CFameContainerEntryPD*	__o = &(_Entries[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	data.serial(_LastGuildStatusChange);
	data.serial(_LastFameChangeDate);
	_Map[getContId()] = this;
}
void							CFameContainerPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(__BaseTable, __BaseRow, 0, _ContId);
	pds__registerAttributes();
}
void							CFameContainerPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CFameContainerPD: registerAttributes %u:%u", __BaseTable, __BaseRow);
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(0), _ContId);
}
void							CFameContainerPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(__BaseTable, __BaseRow, _ContId);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CFameContainerPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CFameContainerPD: unregisterAttributes %u:%u", __BaseTable, __BaseRow);
	for (std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __it=_Entries.begin(); __it!=_Entries.end(); )
	{
		std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __itr=__it++;
		CFameContainerEntryPD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
}
void							CFameContainerPD::pds__notifyInit()
{
	for (std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __it=_Entries.begin(); __it!=_Entries.end(); )
	{
		std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
}
void							CFameContainerPD::pds__notifyRelease()
{
	PDSLib.release(__BaseTable, __BaseRow);
	for (std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __it=_Entries.begin(); __it!=_Entries.end(); )
	{
		std::map<NLMISC::CSheetId, CFameContainerEntryPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
}
void							CFameContainerPD::pds_static__init()
{
	PDSLib.setIndexAllocator(1, _IndexAllocator);
	pds_static__setFactory(pds_static__factory);
}
std::map<NLMISC::CEntityId,CFameContainerPD*>	CFameContainerPD::_Map;
void							CFameContainerPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(1, userFactory, pds_static__fetch, pds_static__notifyFailure);
		_FactoryInitialised = true;
	}
}
bool							CFameContainerPD::_FactoryInitialised;
void							CFameContainerPD::pds_static__notifyFailure(uint64 key)
{
	if (__pds__LoadCallback != NULL)
	{
		__pds__LoadCallback((NLMISC::CEntityId)key, NULL);
	}
}
void							(*CFameContainerPD::__pds__LoadCallback)(const NLMISC::CEntityId& key, CFameContainerPD* object) = NULL;
RY_PDS::CIndexAllocator			CFameContainerPD::_IndexAllocator;
RY_PDS::IPDBaseData*			CFameContainerPD::pds_static__factory()
{
	return new CFameContainerPD();
}
void							CFameContainerPD::pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data)
{
	CFameContainerPD	*__o = static_cast<CFameContainerPD*>(object);
	__o->pds__fetch(data);
	if (__pds__LoadCallback != NULL)
	{
		__pds__LoadCallback(__o->getContId(), __o);
	}
	__o->pds__notifyInit();
}
// End of static implementation of CFameContainerPD

/* -----------------------------------------
* Static Implementation of CGuildFameContainerPD
* ----------------------------------------- */
CGuildPD*						CGuildFameContainerPD::getParent()
{
	return _Parent;
}
const CGuildPD*					CGuildFameContainerPD::getParent() const
{
	return _Parent;
}
void							CGuildFameContainerPD::clear()
{
	CFameContainerPD::clear();
}
CGuildFameContainerPD*			CGuildFameContainerPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 2) ? static_cast<CGuildFameContainerPD*>(obj) : NULL;
}
const CGuildFameContainerPD*	CGuildFameContainerPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 2) ? static_cast<const CGuildFameContainerPD*>(obj) : NULL;
}
CGuildFameContainerPD*			CGuildFameContainerPD::create(const NLMISC::CEntityId &ContId)
{
	CGuildFameContainerPD	*__o = static_cast<CGuildFameContainerPD*>(pds_static__factory());
	__o->pds__init(ContId);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CGuildFameContainerPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	_Parent = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __Tok_Parent)
		{
			__pdr.popStructBegin(__Tok_Parent);
			CFameContainerPD::apply(__pdr);
			__pdr.popStructEnd(__Tok_Parent);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CGuildFameContainerPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	__pdr.pushStructBegin(__Tok_Parent);
	CFameContainerPD::store(__pdr);
	__pdr.pushStructEnd(__Tok_Parent);
}
void							CGuildFameContainerPD::pds__init(const NLMISC::CEntityId &ContId)
{
	CFameContainerPD::pds__init(ContId);
	_Parent = NULL;
}
void							CGuildFameContainerPD::pds__destroy()
{
	CFameContainerPD::pds__destroy();
}
void							CGuildFameContainerPD::pds__fetch(RY_PDS::CPData &data)
{
	CFameContainerPD::pds__fetch(data);
	_Parent = NULL;
}
void							CGuildFameContainerPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(__BaseTable, __BaseRow, 0, _ContId);
	pds__registerAttributes();
}
void							CGuildFameContainerPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildFameContainerPD: registerAttributes %u:%u", __BaseTable, __BaseRow);
	CFameContainerPD::pds__registerAttributes();
}
void							CGuildFameContainerPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(__BaseTable, __BaseRow, _ContId);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CGuildFameContainerPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildFameContainerPD: unregisterAttributes %u:%u", __BaseTable, __BaseRow);
	CFameContainerPD::pds__unregisterAttributes();
	pds__setParent(NULL);
}
void							CGuildFameContainerPD::pds__setParent(CGuildPD* __parent)
{
	if (_Parent != NULL)
	{
		_Parent->pds__unlinkFameContainer(_ContId);
	}
	_Parent = __parent;
	PDSLib.setParent(__BaseTable, getRow(), (RY_PDS::TColumnIndex)(4), (__parent != NULL ? RY_PDS::CObjectIndex(4, __parent->getRow()) : RY_PDS::CObjectIndex::null()), _ContId);
}
void							CGuildFameContainerPD::pds__setParentUnnotified(CGuildPD* __parent)
{
	_Parent = __parent;
}
void							CGuildFameContainerPD::pds__notifyInit()
{
	CFameContainerPD::pds__notifyInit();
}
void							CGuildFameContainerPD::pds__notifyRelease()
{
	CFameContainerPD::pds__notifyRelease();
}
void							CGuildFameContainerPD::pds_static__init()
{
	PDSLib.setIndexAllocator(2, _IndexAllocator);
	pds_static__setFactory(pds_static__factory);
}
void							CGuildFameContainerPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(2, userFactory, pds_static__fetch, NULL);
		_FactoryInitialised = true;
	}
}
bool							CGuildFameContainerPD::_FactoryInitialised;
RY_PDS::CIndexAllocator			CGuildFameContainerPD::_IndexAllocator;
RY_PDS::IPDBaseData*			CGuildFameContainerPD::pds_static__factory()
{
	return new CGuildFameContainerPD();
}
// End of static implementation of CGuildFameContainerPD

	
} // End of EGSPD
