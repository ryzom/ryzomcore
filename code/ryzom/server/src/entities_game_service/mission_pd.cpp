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
	
/* -----------------------------------------
* Static Implementation of CActiveStepStatePD
* ----------------------------------------- */
uint32							CActiveStepStatePD::getIndex() const
{
	return _Index;
}
uint32							CActiveStepStatePD::getState() const
{
	return _State;
}
void							CActiveStepStatePD::setState(uint32 __v, bool forceWrite)
{
	if ((_State != __v) || forceWrite)
	{
		PDSLib.set(6, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_State = __v;
}
CActiveStepPD*					CActiveStepStatePD::getStep()
{
	return _Step;
}
const CActiveStepPD*			CActiveStepStatePD::getStep() const
{
	return _Step;
}
void							CActiveStepStatePD::clear()
{
	_State = 0;
	PDSLib.set(6, __BaseRow, (RY_PDS::TColumnIndex)(1), 0);
}
CActiveStepStatePD*				CActiveStepStatePD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 6) ? static_cast<CActiveStepStatePD*>(obj) : NULL;
}
const CActiveStepStatePD*		CActiveStepStatePD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 6) ? static_cast<const CActiveStepStatePD*>(obj) : NULL;
}
void							CActiveStepStatePD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	uint16	__TokState = __pdr.addString("State");
	_Step = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokIndex)
		{
			__pdr.pop(__TokIndex, _Index);
		}
		else if (__pdr.peekNextToken() == __TokState)
		{
			__pdr.pop(__TokState, _State);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CActiveStepStatePD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	uint16	__TokState = __pdr.addString("State");
	__pdr.push(__TokIndex, _Index);
	__pdr.push(__TokState, _State);
}
void							CActiveStepStatePD::pds__init(const uint32 &Index)
{
	_Index = Index;
	_State = 0;
	_Step = NULL;
}
void							CActiveStepStatePD::pds__destroy()
{
}
void							CActiveStepStatePD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Index);
	data.serial(_State);
	_Step = NULL;
}
void							CActiveStepStatePD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(6, __BaseRow, 0);
	pds__registerAttributes();
}
void							CActiveStepStatePD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CActiveStepStatePD: registerAttributes %u:%u", 6, __BaseRow);
	PDSLib.set(6, __BaseRow, (RY_PDS::TColumnIndex)(0), _Index);
}
void							CActiveStepStatePD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(6, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CActiveStepStatePD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CActiveStepStatePD: unregisterAttributes %u:%u", 6, __BaseRow);
	pds__setParent(NULL);
}
void							CActiveStepStatePD::pds__setParent(CActiveStepPD* __parent)
{
	_Step = __parent;
	PDSLib.setParent(6, getRow(), (RY_PDS::TColumnIndex)(2), (__parent != NULL ? RY_PDS::CObjectIndex(7, __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CActiveStepStatePD::pds__setParentUnnotified(CActiveStepPD* __parent)
{
	_Step = __parent;
}
void							CActiveStepStatePD::pds__notifyInit()
{
}
void							CActiveStepStatePD::pds__notifyRelease()
{
	PDSLib.release(6, __BaseRow);
}
void							CActiveStepStatePD::pds_static__init()
{
	PDSLib.setIndexAllocator(6, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CActiveStepStatePD::_IndexAllocator;
// End of static implementation of CActiveStepStatePD

/* -----------------------------------------
* Static Implementation of CActiveStepPD
* ----------------------------------------- */
uint32							CActiveStepPD::getIndexInTemplate() const
{
	return _IndexInTemplate;
}
CActiveStepStatePD*				CActiveStepPD::getStates(const uint32& __k)
{
	std::map<uint32, CActiveStepStatePD>::iterator _it = _States.find(__k);
	return (_it==_States.end() ? NULL : &((*_it).second));
}
const CActiveStepStatePD*		CActiveStepPD::getStates(const uint32& __k) const
{
	std::map<uint32, CActiveStepStatePD>::const_iterator _it = _States.find(__k);
	return (_it==_States.end() ? NULL : &((*_it).second));
}
std::map<uint32, CActiveStepStatePD>::iterator	CActiveStepPD::getStatesBegin()
{
	return _States.begin();
}
std::map<uint32, CActiveStepStatePD>::iterator	CActiveStepPD::getStatesEnd()
{
	return _States.end();
}
std::map<uint32, CActiveStepStatePD>::const_iterator	CActiveStepPD::getStatesBegin() const
{
	return _States.begin();
}
std::map<uint32, CActiveStepStatePD>::const_iterator	CActiveStepPD::getStatesEnd() const
{
	return _States.end();
}
const std::map<uint32, CActiveStepStatePD> &	CActiveStepPD::getStates() const
{
	return _States;
}
CActiveStepStatePD*				CActiveStepPD::addToStates(const uint32 &__k)
{
	std::map<uint32, CActiveStepStatePD>::iterator	__it = _States.find(__k);
	if (__it == _States.end())
	{
		__it = _States.insert(std::map<uint32, CActiveStepStatePD>::value_type(__k, CActiveStepStatePD())).first;
		CActiveStepStatePD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CActiveStepPD::deleteFromStates(const uint32 &__k)
{
	std::map<uint32, CActiveStepStatePD>::iterator	__it = _States.find(__k);
	if (__it == _States.end())	return;
	CActiveStepStatePD&	__o = (*__it).second;
	__o.pds__unregister();
	_States.erase(__it);
}
CMissionPD*						CActiveStepPD::getMission()
{
	return _Mission;
}
const CMissionPD*				CActiveStepPD::getMission() const
{
	return _Mission;
}
void							CActiveStepPD::clear()
{
	for (std::map<uint32, CActiveStepStatePD>::iterator __it=_States.begin(); __it!=_States.end(); )
	{
		std::map<uint32, CActiveStepStatePD>::iterator __itr=__it++;
		CActiveStepStatePD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_States.clear();
}
CActiveStepPD*					CActiveStepPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 7) ? static_cast<CActiveStepPD*>(obj) : NULL;
}
const CActiveStepPD*			CActiveStepPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 7) ? static_cast<const CActiveStepPD*>(obj) : NULL;
}
void							CActiveStepPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndexInTemplate = __pdr.addString("IndexInTemplate");
	uint16	__TokStates = __pdr.addString("States");
	_Mission = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokIndexInTemplate)
		{
			__pdr.pop(__TokIndexInTemplate, _IndexInTemplate);
		}
		// apply States
		else if (__pdr.peekNextToken() == __TokStates)
		{
			__pdr.popStructBegin(__TokStates);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CActiveStepStatePD&	obj = _States[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokStates);
		}
		// end of apply States
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CActiveStepPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndexInTemplate = __pdr.addString("IndexInTemplate");
	uint16	__TokStates = __pdr.addString("States");
	__pdr.push(__TokIndexInTemplate, _IndexInTemplate);
	// store States
	__pdr.pushStructBegin(__TokStates);
	for (std::map<uint32, CActiveStepStatePD>::const_iterator it=_States.begin(); it!=_States.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokStates);
	// end of store States
}
void							CActiveStepPD::pds__init(const uint32 &IndexInTemplate)
{
	_IndexInTemplate = IndexInTemplate;
	_Mission = NULL;
}
void							CActiveStepPD::pds__destroy()
{
	for (std::map<uint32, CActiveStepStatePD>::iterator __it=_States.begin(); __it!=_States.end(); )
	{
		std::map<uint32, CActiveStepStatePD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_States.clear();
}
void							CActiveStepPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_IndexInTemplate);
	RY_PDS::TTableIndex	tableIndex;
	RY_PDS::TRowIndex	rowIndex;
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_States.insert(std::make_pair<uint32,CActiveStepStatePD>(__k, CActiveStepStatePD()));
		CActiveStepStatePD*	__o = &(_States[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	_Mission = NULL;
}
void							CActiveStepPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(7, __BaseRow, 0);
	pds__registerAttributes();
}
void							CActiveStepPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CActiveStepPD: registerAttributes %u:%u", 7, __BaseRow);
	PDSLib.set(7, __BaseRow, (RY_PDS::TColumnIndex)(0), _IndexInTemplate);
}
void							CActiveStepPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(7, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CActiveStepPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CActiveStepPD: unregisterAttributes %u:%u", 7, __BaseRow);
	pds__setParent(NULL);
	for (std::map<uint32, CActiveStepStatePD>::iterator __it=_States.begin(); __it!=_States.end(); )
	{
		std::map<uint32, CActiveStepStatePD>::iterator __itr=__it++;
		CActiveStepStatePD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
}
void							CActiveStepPD::pds__setParent(CMissionPD* __parent)
{
	_Mission = __parent;
	PDSLib.setParent(7, getRow(), (RY_PDS::TColumnIndex)(2), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CActiveStepPD::pds__setParentUnnotified(CMissionPD* __parent)
{
	_Mission = __parent;
}
void							CActiveStepPD::pds__notifyInit()
{
	for (std::map<uint32, CActiveStepStatePD>::iterator __it=_States.begin(); __it!=_States.end(); )
	{
		std::map<uint32, CActiveStepStatePD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
}
void							CActiveStepPD::pds__notifyRelease()
{
	PDSLib.release(7, __BaseRow);
	for (std::map<uint32, CActiveStepStatePD>::iterator __it=_States.begin(); __it!=_States.end(); )
	{
		std::map<uint32, CActiveStepStatePD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
}
void							CActiveStepPD::pds_static__init()
{
	PDSLib.setIndexAllocator(7, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CActiveStepPD::_IndexAllocator;
// End of static implementation of CActiveStepPD

/* -----------------------------------------
* Static Implementation of CDoneStepPD
* ----------------------------------------- */
uint32							CDoneStepPD::getIndex() const
{
	return _Index;
}
CMissionPD*						CDoneStepPD::getMission()
{
	return _Mission;
}
const CMissionPD*				CDoneStepPD::getMission() const
{
	return _Mission;
}
void							CDoneStepPD::clear()
{
}
CDoneStepPD*					CDoneStepPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 8) ? static_cast<CDoneStepPD*>(obj) : NULL;
}
const CDoneStepPD*				CDoneStepPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 8) ? static_cast<const CDoneStepPD*>(obj) : NULL;
}
void							CDoneStepPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	_Mission = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokIndex)
		{
			__pdr.pop(__TokIndex, _Index);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CDoneStepPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	__pdr.push(__TokIndex, _Index);
}
void							CDoneStepPD::pds__init(const uint32 &Index)
{
	_Index = Index;
	_Mission = NULL;
}
void							CDoneStepPD::pds__destroy()
{
}
void							CDoneStepPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Index);
	_Mission = NULL;
}
void							CDoneStepPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(8, __BaseRow, 0);
	pds__registerAttributes();
}
void							CDoneStepPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CDoneStepPD: registerAttributes %u:%u", 8, __BaseRow);
	PDSLib.set(8, __BaseRow, (RY_PDS::TColumnIndex)(0), _Index);
}
void							CDoneStepPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(8, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CDoneStepPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CDoneStepPD: unregisterAttributes %u:%u", 8, __BaseRow);
	pds__setParent(NULL);
}
void							CDoneStepPD::pds__setParent(CMissionPD* __parent)
{
	_Mission = __parent;
	PDSLib.setParent(8, getRow(), (RY_PDS::TColumnIndex)(1), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CDoneStepPD::pds__setParentUnnotified(CMissionPD* __parent)
{
	_Mission = __parent;
}
void							CDoneStepPD::pds__notifyInit()
{
}
void							CDoneStepPD::pds__notifyRelease()
{
	PDSLib.release(8, __BaseRow);
}
void							CDoneStepPD::pds_static__init()
{
	PDSLib.setIndexAllocator(8, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CDoneStepPD::_IndexAllocator;
// End of static implementation of CDoneStepPD

/* -----------------------------------------
* Static Implementation of CMissionCompassPD
* ----------------------------------------- */
uint32							CMissionCompassPD::getIndex() const
{
	return _Index;
}
uint32							CMissionCompassPD::getPlace() const
{
	return _Place;
}
void							CMissionCompassPD::setPlace(uint32 __v, bool forceWrite)
{
	if ((_Place != __v) || forceWrite)
	{
		PDSLib.set(9, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_Place = __v;
}
uint32							CMissionCompassPD::getBotId() const
{
	return _BotId;
}
void							CMissionCompassPD::setBotId(uint32 __v, bool forceWrite)
{
	if ((_BotId != __v) || forceWrite)
	{
		PDSLib.set(9, __BaseRow, (RY_PDS::TColumnIndex)(2), __v);
	}
	_BotId = __v;
}
CMissionPD*						CMissionCompassPD::getMission()
{
	return _Mission;
}
const CMissionPD*				CMissionCompassPD::getMission() const
{
	return _Mission;
}
void							CMissionCompassPD::clear()
{
	_Place = 0;
	PDSLib.set(9, __BaseRow, (RY_PDS::TColumnIndex)(1), 0);
	_BotId = 0;
	PDSLib.set(9, __BaseRow, (RY_PDS::TColumnIndex)(2), 0);
}
CMissionCompassPD*				CMissionCompassPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 9) ? static_cast<CMissionCompassPD*>(obj) : NULL;
}
const CMissionCompassPD*		CMissionCompassPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 9) ? static_cast<const CMissionCompassPD*>(obj) : NULL;
}
void							CMissionCompassPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	uint16	__TokPlace = __pdr.addString("Place");
	uint16	__TokBotId = __pdr.addString("BotId");
	_Mission = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokIndex)
		{
			__pdr.pop(__TokIndex, _Index);
		}
		else if (__pdr.peekNextToken() == __TokPlace)
		{
			__pdr.pop(__TokPlace, _Place);
		}
		else if (__pdr.peekNextToken() == __TokBotId)
		{
			__pdr.pop(__TokBotId, _BotId);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionCompassPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	uint16	__TokPlace = __pdr.addString("Place");
	uint16	__TokBotId = __pdr.addString("BotId");
	__pdr.push(__TokIndex, _Index);
	__pdr.push(__TokPlace, _Place);
	__pdr.push(__TokBotId, _BotId);
}
void							CMissionCompassPD::pds__init(const uint32 &Index)
{
	_Index = Index;
	_Place = 0;
	_BotId = 0;
	_Mission = NULL;
}
void							CMissionCompassPD::pds__destroy()
{
}
void							CMissionCompassPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Index);
	data.serial(_Place);
	data.serial(_BotId);
	_Mission = NULL;
}
void							CMissionCompassPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(9, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionCompassPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionCompassPD: registerAttributes %u:%u", 9, __BaseRow);
	PDSLib.set(9, __BaseRow, (RY_PDS::TColumnIndex)(0), _Index);
}
void							CMissionCompassPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(9, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionCompassPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionCompassPD: unregisterAttributes %u:%u", 9, __BaseRow);
	pds__setParent(NULL);
}
void							CMissionCompassPD::pds__setParent(CMissionPD* __parent)
{
	_Mission = __parent;
	PDSLib.setParent(9, getRow(), (RY_PDS::TColumnIndex)(3), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CMissionCompassPD::pds__setParentUnnotified(CMissionPD* __parent)
{
	_Mission = __parent;
}
void							CMissionCompassPD::pds__notifyInit()
{
	{
		// CMissionCompassPD init user code, defined at entities_game_service/pd_scripts/mission.pds:27
		NameStringId = 0;
		NameString.clear();
	}
}
void							CMissionCompassPD::pds__notifyRelease()
{
	PDSLib.release(9, __BaseRow);
}
void							CMissionCompassPD::pds_static__init()
{
	PDSLib.setIndexAllocator(9, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CMissionCompassPD::_IndexAllocator;
// End of static implementation of CMissionCompassPD

/* -----------------------------------------
* Static Implementation of CMissionTeleportPD
* ----------------------------------------- */
uint32							CMissionTeleportPD::getIndex() const
{
	return _Index;
}
CMissionPD*						CMissionTeleportPD::getMission()
{
	return _Mission;
}
const CMissionPD*				CMissionTeleportPD::getMission() const
{
	return _Mission;
}
void							CMissionTeleportPD::clear()
{
}
CMissionTeleportPD*				CMissionTeleportPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 10) ? static_cast<CMissionTeleportPD*>(obj) : NULL;
}
const CMissionTeleportPD*		CMissionTeleportPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 10) ? static_cast<const CMissionTeleportPD*>(obj) : NULL;
}
void							CMissionTeleportPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	_Mission = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokIndex)
		{
			__pdr.pop(__TokIndex, _Index);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionTeleportPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokIndex = __pdr.addString("Index");
	__pdr.push(__TokIndex, _Index);
}
void							CMissionTeleportPD::pds__init(const uint32 &Index)
{
	_Index = Index;
	_Mission = NULL;
}
void							CMissionTeleportPD::pds__destroy()
{
}
void							CMissionTeleportPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Index);
	_Mission = NULL;
}
void							CMissionTeleportPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(10, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionTeleportPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionTeleportPD: registerAttributes %u:%u", 10, __BaseRow);
	PDSLib.set(10, __BaseRow, (RY_PDS::TColumnIndex)(0), _Index);
}
void							CMissionTeleportPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(10, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionTeleportPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionTeleportPD: unregisterAttributes %u:%u", 10, __BaseRow);
	pds__setParent(NULL);
}
void							CMissionTeleportPD::pds__setParent(CMissionPD* __parent)
{
	_Mission = __parent;
	PDSLib.setParent(10, getRow(), (RY_PDS::TColumnIndex)(1), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CMissionTeleportPD::pds__setParentUnnotified(CMissionPD* __parent)
{
	_Mission = __parent;
}
void							CMissionTeleportPD::pds__notifyInit()
{
}
void							CMissionTeleportPD::pds__notifyRelease()
{
	PDSLib.release(10, __BaseRow);
}
void							CMissionTeleportPD::pds_static__init()
{
	PDSLib.setIndexAllocator(10, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CMissionTeleportPD::_IndexAllocator;
// End of static implementation of CMissionTeleportPD

/* -----------------------------------------
* Static Implementation of CMissionInsidePlacePD
* ----------------------------------------- */
uint32							CMissionInsidePlacePD::getAlias() const
{
	return _Alias;
}
uint32							CMissionInsidePlacePD::getDelay() const
{
	return _Delay;
}
void							CMissionInsidePlacePD::setDelay(uint32 __v, bool forceWrite)
{
	if ((_Delay != __v) || forceWrite)
	{
		PDSLib.set(11, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_Delay = __v;
}
CMissionPD*						CMissionInsidePlacePD::getMission()
{
	return _Mission;
}
const CMissionPD*				CMissionInsidePlacePD::getMission() const
{
	return _Mission;
}
void							CMissionInsidePlacePD::clear()
{
	_Delay = 0;
	PDSLib.set(11, __BaseRow, (RY_PDS::TColumnIndex)(1), 0);
}
CMissionInsidePlacePD*			CMissionInsidePlacePD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 11) ? static_cast<CMissionInsidePlacePD*>(obj) : NULL;
}
const CMissionInsidePlacePD*	CMissionInsidePlacePD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 11) ? static_cast<const CMissionInsidePlacePD*>(obj) : NULL;
}
void							CMissionInsidePlacePD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokAlias = __pdr.addString("Alias");
	uint16	__TokDelay = __pdr.addString("Delay");
	_Mission = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokAlias)
		{
			__pdr.pop(__TokAlias, _Alias);
		}
		else if (__pdr.peekNextToken() == __TokDelay)
		{
			__pdr.pop(__TokDelay, _Delay);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionInsidePlacePD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokAlias = __pdr.addString("Alias");
	uint16	__TokDelay = __pdr.addString("Delay");
	__pdr.push(__TokAlias, _Alias);
	__pdr.push(__TokDelay, _Delay);
}
void							CMissionInsidePlacePD::pds__init(const uint32 &Alias)
{
	_Alias = Alias;
	_Delay = 0;
	_Mission = NULL;
}
void							CMissionInsidePlacePD::pds__destroy()
{
}
void							CMissionInsidePlacePD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Alias);
	data.serial(_Delay);
	_Mission = NULL;
}
void							CMissionInsidePlacePD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(11, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionInsidePlacePD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionInsidePlacePD: registerAttributes %u:%u", 11, __BaseRow);
	PDSLib.set(11, __BaseRow, (RY_PDS::TColumnIndex)(0), _Alias);
}
void							CMissionInsidePlacePD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(11, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionInsidePlacePD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionInsidePlacePD: unregisterAttributes %u:%u", 11, __BaseRow);
	pds__setParent(NULL);
}
void							CMissionInsidePlacePD::pds__setParent(CMissionPD* __parent)
{
	_Mission = __parent;
	PDSLib.setParent(11, getRow(), (RY_PDS::TColumnIndex)(2), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CMissionInsidePlacePD::pds__setParentUnnotified(CMissionPD* __parent)
{
	_Mission = __parent;
}
void							CMissionInsidePlacePD::pds__notifyInit()
{
}
void							CMissionInsidePlacePD::pds__notifyRelease()
{
	PDSLib.release(11, __BaseRow);
}
void							CMissionInsidePlacePD::pds_static__init()
{
	PDSLib.setIndexAllocator(11, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CMissionInsidePlacePD::_IndexAllocator;
// End of static implementation of CMissionInsidePlacePD

/* -----------------------------------------
* Static Implementation of CMissionOutsidePlacePD
* ----------------------------------------- */
uint32							CMissionOutsidePlacePD::getAlias() const
{
	return _Alias;
}
uint32							CMissionOutsidePlacePD::getDelay() const
{
	return _Delay;
}
void							CMissionOutsidePlacePD::setDelay(uint32 __v, bool forceWrite)
{
	if ((_Delay != __v) || forceWrite)
	{
		PDSLib.set(12, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_Delay = __v;
}
CMissionPD*						CMissionOutsidePlacePD::getMission()
{
	return _Mission;
}
const CMissionPD*				CMissionOutsidePlacePD::getMission() const
{
	return _Mission;
}
void							CMissionOutsidePlacePD::clear()
{
	_Delay = 0;
	PDSLib.set(12, __BaseRow, (RY_PDS::TColumnIndex)(1), 0);
}
CMissionOutsidePlacePD*			CMissionOutsidePlacePD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 12) ? static_cast<CMissionOutsidePlacePD*>(obj) : NULL;
}
const CMissionOutsidePlacePD*	CMissionOutsidePlacePD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 12) ? static_cast<const CMissionOutsidePlacePD*>(obj) : NULL;
}
void							CMissionOutsidePlacePD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokAlias = __pdr.addString("Alias");
	uint16	__TokDelay = __pdr.addString("Delay");
	_Mission = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokAlias)
		{
			__pdr.pop(__TokAlias, _Alias);
		}
		else if (__pdr.peekNextToken() == __TokDelay)
		{
			__pdr.pop(__TokDelay, _Delay);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionOutsidePlacePD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokAlias = __pdr.addString("Alias");
	uint16	__TokDelay = __pdr.addString("Delay");
	__pdr.push(__TokAlias, _Alias);
	__pdr.push(__TokDelay, _Delay);
}
void							CMissionOutsidePlacePD::pds__init(const uint32 &Alias)
{
	_Alias = Alias;
	_Delay = 0;
	_Mission = NULL;
}
void							CMissionOutsidePlacePD::pds__destroy()
{
}
void							CMissionOutsidePlacePD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Alias);
	data.serial(_Delay);
	_Mission = NULL;
}
void							CMissionOutsidePlacePD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(12, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionOutsidePlacePD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionOutsidePlacePD: registerAttributes %u:%u", 12, __BaseRow);
	PDSLib.set(12, __BaseRow, (RY_PDS::TColumnIndex)(0), _Alias);
}
void							CMissionOutsidePlacePD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(12, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionOutsidePlacePD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionOutsidePlacePD: unregisterAttributes %u:%u", 12, __BaseRow);
	pds__setParent(NULL);
}
void							CMissionOutsidePlacePD::pds__setParent(CMissionPD* __parent)
{
	_Mission = __parent;
	PDSLib.setParent(12, getRow(), (RY_PDS::TColumnIndex)(2), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CMissionOutsidePlacePD::pds__setParentUnnotified(CMissionPD* __parent)
{
	_Mission = __parent;
}
void							CMissionOutsidePlacePD::pds__notifyInit()
{
}
void							CMissionOutsidePlacePD::pds__notifyRelease()
{
	PDSLib.release(12, __BaseRow);
}
void							CMissionOutsidePlacePD::pds_static__init()
{
	PDSLib.setIndexAllocator(12, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CMissionOutsidePlacePD::_IndexAllocator;
// End of static implementation of CMissionOutsidePlacePD

/* -----------------------------------------
* Static Implementation of CHandledAIGroupPD
* ----------------------------------------- */
uint32							CHandledAIGroupPD::getGroupAlias() const
{
	return _GroupAlias;
}
uint32							CHandledAIGroupPD::getDespawnTime() const
{
	return _DespawnTime;
}
void							CHandledAIGroupPD::setDespawnTime(uint32 __v, bool forceWrite)
{
	if ((_DespawnTime != __v) || forceWrite)
	{
		PDSLib.set(13, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_DespawnTime = __v;
}
CMissionPD*						CHandledAIGroupPD::getMission()
{
	return _Mission;
}
const CMissionPD*				CHandledAIGroupPD::getMission() const
{
	return _Mission;
}
void							CHandledAIGroupPD::clear()
{
	_DespawnTime = 0;
	PDSLib.set(13, __BaseRow, (RY_PDS::TColumnIndex)(1), 0);
}
CHandledAIGroupPD*				CHandledAIGroupPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 13) ? static_cast<CHandledAIGroupPD*>(obj) : NULL;
}
const CHandledAIGroupPD*		CHandledAIGroupPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 13) ? static_cast<const CHandledAIGroupPD*>(obj) : NULL;
}
void							CHandledAIGroupPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokGroupAlias = __pdr.addString("GroupAlias");
	uint16	__TokDespawnTime = __pdr.addString("DespawnTime");
	_Mission = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokGroupAlias)
		{
			__pdr.pop(__TokGroupAlias, _GroupAlias);
		}
		else if (__pdr.peekNextToken() == __TokDespawnTime)
		{
			__pdr.pop(__TokDespawnTime, _DespawnTime);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CHandledAIGroupPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokGroupAlias = __pdr.addString("GroupAlias");
	uint16	__TokDespawnTime = __pdr.addString("DespawnTime");
	__pdr.push(__TokGroupAlias, _GroupAlias);
	__pdr.push(__TokDespawnTime, _DespawnTime);
}
void							CHandledAIGroupPD::pds__init(const uint32 &GroupAlias)
{
	_GroupAlias = GroupAlias;
	_DespawnTime = 0;
	_Mission = NULL;
}
void							CHandledAIGroupPD::pds__destroy()
{
}
void							CHandledAIGroupPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_GroupAlias);
	data.serial(_DespawnTime);
	_Mission = NULL;
}
void							CHandledAIGroupPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(13, __BaseRow, 0);
	pds__registerAttributes();
}
void							CHandledAIGroupPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CHandledAIGroupPD: registerAttributes %u:%u", 13, __BaseRow);
	PDSLib.set(13, __BaseRow, (RY_PDS::TColumnIndex)(0), _GroupAlias);
}
void							CHandledAIGroupPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(13, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CHandledAIGroupPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CHandledAIGroupPD: unregisterAttributes %u:%u", 13, __BaseRow);
	pds__setParent(NULL);
}
void							CHandledAIGroupPD::pds__setParent(CMissionPD* __parent)
{
	_Mission = __parent;
	PDSLib.setParent(13, getRow(), (RY_PDS::TColumnIndex)(2), (__parent != NULL ? RY_PDS::CObjectIndex(__parent->getTable(), __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CHandledAIGroupPD::pds__setParentUnnotified(CMissionPD* __parent)
{
	_Mission = __parent;
}
void							CHandledAIGroupPD::pds__notifyInit()
{
}
void							CHandledAIGroupPD::pds__notifyRelease()
{
	PDSLib.release(13, __BaseRow);
}
void							CHandledAIGroupPD::pds_static__init()
{
	PDSLib.setIndexAllocator(13, _IndexAllocator);
}
RY_PDS::CIndexAllocator			CHandledAIGroupPD::_IndexAllocator;
// End of static implementation of CHandledAIGroupPD

/* -----------------------------------------
* Static Implementation of CMissionPD
* ----------------------------------------- */
uint32							CMissionPD::getTemplateId() const
{
	return _TemplateId;
}
uint32							CMissionPD::getMainMissionTemplateId() const
{
	return _MainMissionTemplateId;
}
void							CMissionPD::setMainMissionTemplateId(uint32 __v, bool forceWrite)
{
	if ((_MainMissionTemplateId != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_MainMissionTemplateId = __v;
}
uint32							CMissionPD::getGiver() const
{
	return _Giver;
}
void							CMissionPD::setGiver(uint32 __v, bool forceWrite)
{
	if ((_Giver != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(2), __v);
	}
	_Giver = __v;
}
float							CMissionPD::getHourLowerBound() const
{
	return _HourLowerBound;
}
void							CMissionPD::setHourLowerBound(float __v, bool forceWrite)
{
	if ((_HourLowerBound != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(3), __v);
	}
	_HourLowerBound = __v;
}
float							CMissionPD::getHourUpperBound() const
{
	return _HourUpperBound;
}
void							CMissionPD::setHourUpperBound(float __v, bool forceWrite)
{
	if ((_HourUpperBound != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(4), __v);
	}
	_HourUpperBound = __v;
}
CSeason::TSeason				CMissionPD::getSeason() const
{
	return _Season;
}
void							CMissionPD::setSeason(CSeason::TSeason __v, bool forceWrite)
{
	nlassert(__v<CSeason::___TSeason_useSize);
	if ((_Season != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(5), (uint32)__v);
	}
	_Season = __v;
}
uint32							CMissionPD::getMonoEndDate() const
{
	return _MonoEndDate;
}
void							CMissionPD::setMonoEndDate(uint32 __v, bool forceWrite)
{
	if ((_MonoEndDate != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(6), __v);
	}
	_MonoEndDate = __v;
}
uint32							CMissionPD::getEndDate() const
{
	return _EndDate;
}
void							CMissionPD::setEndDate(uint32 __v, bool forceWrite)
{
	if ((_EndDate != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(7), __v);
	}
	_EndDate = __v;
}
uint32							CMissionPD::getCriticalPartEndDate() const
{
	return _CriticalPartEndDate;
}
void							CMissionPD::setCriticalPartEndDate(uint32 __v, bool forceWrite)
{
	if ((_CriticalPartEndDate != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(8), __v);
	}
	_CriticalPartEndDate = __v;
}
uint32							CMissionPD::getBeginDate() const
{
	return _BeginDate;
}
void							CMissionPD::setBeginDate(uint32 __v, bool forceWrite)
{
	if ((_BeginDate != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(9), __v);
	}
	_BeginDate = __v;
}
uint32							CMissionPD::getFailureIndex() const
{
	return _FailureIndex;
}
void							CMissionPD::setFailureIndex(uint32 __v, bool forceWrite)
{
	if ((_FailureIndex != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(10), __v);
	}
	_FailureIndex = __v;
}
uint32							CMissionPD::getCrashHandlerIndex() const
{
	return _CrashHandlerIndex;
}
void							CMissionPD::setCrashHandlerIndex(uint32 __v, bool forceWrite)
{
	if ((_CrashHandlerIndex != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(11), __v);
	}
	_CrashHandlerIndex = __v;
}
uint32							CMissionPD::getPlayerReconnectHandlerIndex() const
{
	return _PlayerReconnectHandlerIndex;
}
void							CMissionPD::setPlayerReconnectHandlerIndex(uint32 __v, bool forceWrite)
{
	if ((_PlayerReconnectHandlerIndex != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(12), __v);
	}
	_PlayerReconnectHandlerIndex = __v;
}
bool							CMissionPD::getFinished() const
{
	return _Finished;
}
void							CMissionPD::setFinished(bool __v, bool forceWrite)
{
	if ((_Finished != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(13), __v);
	}
	_Finished = __v;
}
bool							CMissionPD::getMissionSuccess() const
{
	return _MissionSuccess;
}
void							CMissionPD::setMissionSuccess(bool __v, bool forceWrite)
{
	if ((_MissionSuccess != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(14), __v);
	}
	_MissionSuccess = __v;
}
uint32							CMissionPD::getDescIndex() const
{
	return _DescIndex;
}
void							CMissionPD::setDescIndex(uint32 __v, bool forceWrite)
{
	if ((_DescIndex != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(15), __v);
	}
	_DescIndex = __v;
}
uint32							CMissionPD::getWaitingQueueId() const
{
	return _WaitingQueueId;
}
void							CMissionPD::setWaitingQueueId(uint32 __v, bool forceWrite)
{
	if ((_WaitingQueueId != __v) || forceWrite)
	{
		PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(16), __v);
	}
	_WaitingQueueId = __v;
}
CActiveStepPD*					CMissionPD::getSteps(const uint32& __k)
{
	std::map<uint32, CActiveStepPD>::iterator _it = _Steps.find(__k);
	return (_it==_Steps.end() ? NULL : &((*_it).second));
}
const CActiveStepPD*			CMissionPD::getSteps(const uint32& __k) const
{
	std::map<uint32, CActiveStepPD>::const_iterator _it = _Steps.find(__k);
	return (_it==_Steps.end() ? NULL : &((*_it).second));
}
std::map<uint32, CActiveStepPD>::iterator	CMissionPD::getStepsBegin()
{
	return _Steps.begin();
}
std::map<uint32, CActiveStepPD>::iterator	CMissionPD::getStepsEnd()
{
	return _Steps.end();
}
std::map<uint32, CActiveStepPD>::const_iterator	CMissionPD::getStepsBegin() const
{
	return _Steps.begin();
}
std::map<uint32, CActiveStepPD>::const_iterator	CMissionPD::getStepsEnd() const
{
	return _Steps.end();
}
const std::map<uint32, CActiveStepPD> &	CMissionPD::getSteps() const
{
	return _Steps;
}
CActiveStepPD*					CMissionPD::addToSteps(const uint32 &__k)
{
	std::map<uint32, CActiveStepPD>::iterator	__it = _Steps.find(__k);
	if (__it == _Steps.end())
	{
		__it = _Steps.insert(std::map<uint32, CActiveStepPD>::value_type(__k, CActiveStepPD())).first;
		CActiveStepPD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CMissionPD::deleteFromSteps(const uint32 &__k)
{
	std::map<uint32, CActiveStepPD>::iterator	__it = _Steps.find(__k);
	if (__it == _Steps.end())	return;
	CActiveStepPD&	__o = (*__it).second;
	__o.pds__unregister();
	_Steps.erase(__it);
}
CMissionCompassPD*				CMissionPD::getCompass(const uint32& __k)
{
	std::map<uint32, CMissionCompassPD>::iterator _it = _Compass.find(__k);
	return (_it==_Compass.end() ? NULL : &((*_it).second));
}
const CMissionCompassPD*		CMissionPD::getCompass(const uint32& __k) const
{
	std::map<uint32, CMissionCompassPD>::const_iterator _it = _Compass.find(__k);
	return (_it==_Compass.end() ? NULL : &((*_it).second));
}
std::map<uint32, CMissionCompassPD>::iterator	CMissionPD::getCompassBegin()
{
	return _Compass.begin();
}
std::map<uint32, CMissionCompassPD>::iterator	CMissionPD::getCompassEnd()
{
	return _Compass.end();
}
std::map<uint32, CMissionCompassPD>::const_iterator	CMissionPD::getCompassBegin() const
{
	return _Compass.begin();
}
std::map<uint32, CMissionCompassPD>::const_iterator	CMissionPD::getCompassEnd() const
{
	return _Compass.end();
}
const std::map<uint32, CMissionCompassPD> &	CMissionPD::getCompass() const
{
	return _Compass;
}
CMissionCompassPD*				CMissionPD::addToCompass(const uint32 &__k)
{
	std::map<uint32, CMissionCompassPD>::iterator	__it = _Compass.find(__k);
	if (__it == _Compass.end())
	{
		__it = _Compass.insert(std::map<uint32, CMissionCompassPD>::value_type(__k, CMissionCompassPD())).first;
		CMissionCompassPD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CMissionPD::deleteFromCompass(const uint32 &__k)
{
	std::map<uint32, CMissionCompassPD>::iterator	__it = _Compass.find(__k);
	if (__it == _Compass.end())	return;
	CMissionCompassPD&	__o = (*__it).second;
	__o.pds__unregister();
	_Compass.erase(__it);
}
CDoneStepPD*					CMissionPD::getStepsDone(const uint32& __k)
{
	std::map<uint32, CDoneStepPD>::iterator _it = _StepsDone.find(__k);
	return (_it==_StepsDone.end() ? NULL : &((*_it).second));
}
const CDoneStepPD*				CMissionPD::getStepsDone(const uint32& __k) const
{
	std::map<uint32, CDoneStepPD>::const_iterator _it = _StepsDone.find(__k);
	return (_it==_StepsDone.end() ? NULL : &((*_it).second));
}
std::map<uint32, CDoneStepPD>::iterator	CMissionPD::getStepsDoneBegin()
{
	return _StepsDone.begin();
}
std::map<uint32, CDoneStepPD>::iterator	CMissionPD::getStepsDoneEnd()
{
	return _StepsDone.end();
}
std::map<uint32, CDoneStepPD>::const_iterator	CMissionPD::getStepsDoneBegin() const
{
	return _StepsDone.begin();
}
std::map<uint32, CDoneStepPD>::const_iterator	CMissionPD::getStepsDoneEnd() const
{
	return _StepsDone.end();
}
const std::map<uint32, CDoneStepPD> &	CMissionPD::getStepsDone() const
{
	return _StepsDone;
}
CDoneStepPD*					CMissionPD::addToStepsDone(const uint32 &__k)
{
	std::map<uint32, CDoneStepPD>::iterator	__it = _StepsDone.find(__k);
	if (__it == _StepsDone.end())
	{
		__it = _StepsDone.insert(std::map<uint32, CDoneStepPD>::value_type(__k, CDoneStepPD())).first;
		CDoneStepPD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CMissionPD::deleteFromStepsDone(const uint32 &__k)
{
	std::map<uint32, CDoneStepPD>::iterator	__it = _StepsDone.find(__k);
	if (__it == _StepsDone.end())	return;
	CDoneStepPD&	__o = (*__it).second;
	__o.pds__unregister();
	_StepsDone.erase(__it);
}
CMissionTeleportPD*				CMissionPD::getTeleports(const uint32& __k)
{
	std::map<uint32, CMissionTeleportPD>::iterator _it = _Teleports.find(__k);
	return (_it==_Teleports.end() ? NULL : &((*_it).second));
}
const CMissionTeleportPD*		CMissionPD::getTeleports(const uint32& __k) const
{
	std::map<uint32, CMissionTeleportPD>::const_iterator _it = _Teleports.find(__k);
	return (_it==_Teleports.end() ? NULL : &((*_it).second));
}
std::map<uint32, CMissionTeleportPD>::iterator	CMissionPD::getTeleportsBegin()
{
	return _Teleports.begin();
}
std::map<uint32, CMissionTeleportPD>::iterator	CMissionPD::getTeleportsEnd()
{
	return _Teleports.end();
}
std::map<uint32, CMissionTeleportPD>::const_iterator	CMissionPD::getTeleportsBegin() const
{
	return _Teleports.begin();
}
std::map<uint32, CMissionTeleportPD>::const_iterator	CMissionPD::getTeleportsEnd() const
{
	return _Teleports.end();
}
const std::map<uint32, CMissionTeleportPD> &	CMissionPD::getTeleports() const
{
	return _Teleports;
}
CMissionTeleportPD*				CMissionPD::addToTeleports(const uint32 &__k)
{
	std::map<uint32, CMissionTeleportPD>::iterator	__it = _Teleports.find(__k);
	if (__it == _Teleports.end())
	{
		__it = _Teleports.insert(std::map<uint32, CMissionTeleportPD>::value_type(__k, CMissionTeleportPD())).first;
		CMissionTeleportPD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CMissionPD::deleteFromTeleports(const uint32 &__k)
{
	std::map<uint32, CMissionTeleportPD>::iterator	__it = _Teleports.find(__k);
	if (__it == _Teleports.end())	return;
	CMissionTeleportPD&	__o = (*__it).second;
	__o.pds__unregister();
	_Teleports.erase(__it);
}
CMissionInsidePlacePD*			CMissionPD::getInsidePlaces(const uint32& __k)
{
	std::map<uint32, CMissionInsidePlacePD>::iterator _it = _InsidePlaces.find(__k);
	return (_it==_InsidePlaces.end() ? NULL : &((*_it).second));
}
const CMissionInsidePlacePD*	CMissionPD::getInsidePlaces(const uint32& __k) const
{
	std::map<uint32, CMissionInsidePlacePD>::const_iterator _it = _InsidePlaces.find(__k);
	return (_it==_InsidePlaces.end() ? NULL : &((*_it).second));
}
std::map<uint32, CMissionInsidePlacePD>::iterator	CMissionPD::getInsidePlacesBegin()
{
	return _InsidePlaces.begin();
}
std::map<uint32, CMissionInsidePlacePD>::iterator	CMissionPD::getInsidePlacesEnd()
{
	return _InsidePlaces.end();
}
std::map<uint32, CMissionInsidePlacePD>::const_iterator	CMissionPD::getInsidePlacesBegin() const
{
	return _InsidePlaces.begin();
}
std::map<uint32, CMissionInsidePlacePD>::const_iterator	CMissionPD::getInsidePlacesEnd() const
{
	return _InsidePlaces.end();
}
const std::map<uint32, CMissionInsidePlacePD> &	CMissionPD::getInsidePlaces() const
{
	return _InsidePlaces;
}
CMissionInsidePlacePD*			CMissionPD::addToInsidePlaces(const uint32 &__k)
{
	std::map<uint32, CMissionInsidePlacePD>::iterator	__it = _InsidePlaces.find(__k);
	if (__it == _InsidePlaces.end())
	{
		__it = _InsidePlaces.insert(std::map<uint32, CMissionInsidePlacePD>::value_type(__k, CMissionInsidePlacePD())).first;
		CMissionInsidePlacePD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CMissionPD::deleteFromInsidePlaces(const uint32 &__k)
{
	std::map<uint32, CMissionInsidePlacePD>::iterator	__it = _InsidePlaces.find(__k);
	if (__it == _InsidePlaces.end())	return;
	CMissionInsidePlacePD&	__o = (*__it).second;
	__o.pds__unregister();
	_InsidePlaces.erase(__it);
}
CMissionOutsidePlacePD*			CMissionPD::getOutsidePlaces(const uint32& __k)
{
	std::map<uint32, CMissionOutsidePlacePD>::iterator _it = _OutsidePlaces.find(__k);
	return (_it==_OutsidePlaces.end() ? NULL : &((*_it).second));
}
const CMissionOutsidePlacePD*	CMissionPD::getOutsidePlaces(const uint32& __k) const
{
	std::map<uint32, CMissionOutsidePlacePD>::const_iterator _it = _OutsidePlaces.find(__k);
	return (_it==_OutsidePlaces.end() ? NULL : &((*_it).second));
}
std::map<uint32, CMissionOutsidePlacePD>::iterator	CMissionPD::getOutsidePlacesBegin()
{
	return _OutsidePlaces.begin();
}
std::map<uint32, CMissionOutsidePlacePD>::iterator	CMissionPD::getOutsidePlacesEnd()
{
	return _OutsidePlaces.end();
}
std::map<uint32, CMissionOutsidePlacePD>::const_iterator	CMissionPD::getOutsidePlacesBegin() const
{
	return _OutsidePlaces.begin();
}
std::map<uint32, CMissionOutsidePlacePD>::const_iterator	CMissionPD::getOutsidePlacesEnd() const
{
	return _OutsidePlaces.end();
}
const std::map<uint32, CMissionOutsidePlacePD> &	CMissionPD::getOutsidePlaces() const
{
	return _OutsidePlaces;
}
CMissionOutsidePlacePD*			CMissionPD::addToOutsidePlaces(const uint32 &__k)
{
	std::map<uint32, CMissionOutsidePlacePD>::iterator	__it = _OutsidePlaces.find(__k);
	if (__it == _OutsidePlaces.end())
	{
		__it = _OutsidePlaces.insert(std::map<uint32, CMissionOutsidePlacePD>::value_type(__k, CMissionOutsidePlacePD())).first;
		CMissionOutsidePlacePD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CMissionPD::deleteFromOutsidePlaces(const uint32 &__k)
{
	std::map<uint32, CMissionOutsidePlacePD>::iterator	__it = _OutsidePlaces.find(__k);
	if (__it == _OutsidePlaces.end())	return;
	CMissionOutsidePlacePD&	__o = (*__it).second;
	__o.pds__unregister();
	_OutsidePlaces.erase(__it);
}
CHandledAIGroupPD*				CMissionPD::getHandledAIGroups(const uint32& __k)
{
	std::map<uint32, CHandledAIGroupPD>::iterator _it = _HandledAIGroups.find(__k);
	return (_it==_HandledAIGroups.end() ? NULL : &((*_it).second));
}
const CHandledAIGroupPD*		CMissionPD::getHandledAIGroups(const uint32& __k) const
{
	std::map<uint32, CHandledAIGroupPD>::const_iterator _it = _HandledAIGroups.find(__k);
	return (_it==_HandledAIGroups.end() ? NULL : &((*_it).second));
}
std::map<uint32, CHandledAIGroupPD>::iterator	CMissionPD::getHandledAIGroupsBegin()
{
	return _HandledAIGroups.begin();
}
std::map<uint32, CHandledAIGroupPD>::iterator	CMissionPD::getHandledAIGroupsEnd()
{
	return _HandledAIGroups.end();
}
std::map<uint32, CHandledAIGroupPD>::const_iterator	CMissionPD::getHandledAIGroupsBegin() const
{
	return _HandledAIGroups.begin();
}
std::map<uint32, CHandledAIGroupPD>::const_iterator	CMissionPD::getHandledAIGroupsEnd() const
{
	return _HandledAIGroups.end();
}
const std::map<uint32, CHandledAIGroupPD> &	CMissionPD::getHandledAIGroups() const
{
	return _HandledAIGroups;
}
CHandledAIGroupPD*				CMissionPD::addToHandledAIGroups(const uint32 &__k)
{
	std::map<uint32, CHandledAIGroupPD>::iterator	__it = _HandledAIGroups.find(__k);
	if (__it == _HandledAIGroups.end())
	{
		__it = _HandledAIGroups.insert(std::map<uint32, CHandledAIGroupPD>::value_type(__k, CHandledAIGroupPD())).first;
		CHandledAIGroupPD*	__o = &((*__it).second);
		__o->pds__init(__k);
		__o->pds__register();
		__o->pds__setParent(this);
	}
	return &((*__it).second);
}
void							CMissionPD::deleteFromHandledAIGroups(const uint32 &__k)
{
	std::map<uint32, CHandledAIGroupPD>::iterator	__it = _HandledAIGroups.find(__k);
	if (__it == _HandledAIGroups.end())	return;
	CHandledAIGroupPD&	__o = (*__it).second;
	__o.pds__unregister();
	_HandledAIGroups.erase(__it);
}
CMissionContainerPD*			CMissionPD::getContainer()
{
	return _Container;
}
const CMissionContainerPD*		CMissionPD::getContainer() const
{
	return _Container;
}
void							CMissionPD::clear()
{
	_MainMissionTemplateId = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(1), 0);
	_Giver = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(2), 0);
	_HourLowerBound = 0.0f;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(3), 0.0f);
	_HourUpperBound = 0.0f;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(4), 0.0f);
	_Season = (CSeason::TSeason)0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(5), (uint32)(CSeason::TSeason)0);
	_MonoEndDate = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(6), 0);
	_EndDate = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(7), 0);
	_CriticalPartEndDate = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(8), 0);
	_BeginDate = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(9), 0);
	_FailureIndex = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(10), 0);
	_CrashHandlerIndex = 0xFFFFFFFF;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(11), 0xFFFFFFFF);
	_PlayerReconnectHandlerIndex = 0xFFFFFFFF;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(12), 0xFFFFFFFF);
	_Finished = false;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(13), false);
	_MissionSuccess = true;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(14), true);
	_DescIndex = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(15), 0);
	_WaitingQueueId = 0;
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(16), 0);
	for (std::map<uint32, CActiveStepPD>::iterator __it=_Steps.begin(); __it!=_Steps.end(); )
	{
		std::map<uint32, CActiveStepPD>::iterator __itr=__it++;
		CActiveStepPD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_Steps.clear();
	for (std::map<uint32, CMissionCompassPD>::iterator __it=_Compass.begin(); __it!=_Compass.end(); )
	{
		std::map<uint32, CMissionCompassPD>::iterator __itr=__it++;
		CMissionCompassPD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_Compass.clear();
	for (std::map<uint32, CDoneStepPD>::iterator __it=_StepsDone.begin(); __it!=_StepsDone.end(); )
	{
		std::map<uint32, CDoneStepPD>::iterator __itr=__it++;
		CDoneStepPD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_StepsDone.clear();
	for (std::map<uint32, CMissionTeleportPD>::iterator __it=_Teleports.begin(); __it!=_Teleports.end(); )
	{
		std::map<uint32, CMissionTeleportPD>::iterator __itr=__it++;
		CMissionTeleportPD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_Teleports.clear();
	for (std::map<uint32, CMissionInsidePlacePD>::iterator __it=_InsidePlaces.begin(); __it!=_InsidePlaces.end(); )
	{
		std::map<uint32, CMissionInsidePlacePD>::iterator __itr=__it++;
		CMissionInsidePlacePD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_InsidePlaces.clear();
	for (std::map<uint32, CMissionOutsidePlacePD>::iterator __it=_OutsidePlaces.begin(); __it!=_OutsidePlaces.end(); )
	{
		std::map<uint32, CMissionOutsidePlacePD>::iterator __itr=__it++;
		CMissionOutsidePlacePD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_OutsidePlaces.clear();
	for (std::map<uint32, CHandledAIGroupPD>::iterator __it=_HandledAIGroups.begin(); __it!=_HandledAIGroups.end(); )
	{
		std::map<uint32, CHandledAIGroupPD>::iterator __itr=__it++;
		CHandledAIGroupPD*	__o = &((*__itr).second);
		__o->pds__unregister();
		__o->pds__destroy();
	}
	_HandledAIGroups.clear();
}
CMissionPD*						CMissionPD::cast(RY_PDS::IPDBaseData* obj)
{
	switch (obj->getTable())
	{
		case 14:
		case 15:
		case 16:
		case 17:
		return static_cast<CMissionPD*>(obj);
	}
	return NULL;
}
const CMissionPD*				CMissionPD::cast(const RY_PDS::IPDBaseData* obj)
{
	switch (obj->getTable())
	{
		case 14:
		case 15:
		case 16:
		case 17:
		return static_cast<const CMissionPD*>(obj);
	}
	return NULL;
}
void							CMissionPD::setFactory(RY_PDS::TPDFactory userFactory)
{
	pds_static__setFactory(userFactory);
}
CMissionPD*						CMissionPD::create(const uint32 &TemplateId)
{
	CMissionPD	*__o = static_cast<CMissionPD*>(PDSLib.create(14));
	__o->pds__init(TemplateId);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CMissionPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokTemplateId = __pdr.addString("TemplateId");
	uint16	__TokMainMissionTemplateId = __pdr.addString("MainMissionTemplateId");
	uint16	__TokGiver = __pdr.addString("Giver");
	uint16	__TokHourLowerBound = __pdr.addString("HourLowerBound");
	uint16	__TokHourUpperBound = __pdr.addString("HourUpperBound");
	uint16	__TokSeason = __pdr.addString("Season");
	uint16	__TokMonoEndDate = __pdr.addString("MonoEndDate");
	uint16	__TokEndDate = __pdr.addString("EndDate");
	uint16	__TokCriticalPartEndDate = __pdr.addString("CriticalPartEndDate");
	uint16	__TokBeginDate = __pdr.addString("BeginDate");
	uint16	__TokFailureIndex = __pdr.addString("FailureIndex");
	uint16	__TokCrashHandlerIndex = __pdr.addString("CrashHandlerIndex");
	uint16	__TokPlayerReconnectHandlerIndex = __pdr.addString("PlayerReconnectHandlerIndex");
	uint16	__TokFinished = __pdr.addString("Finished");
	uint16	__TokMissionSuccess = __pdr.addString("MissionSuccess");
	uint16	__TokDescIndex = __pdr.addString("DescIndex");
	uint16	__TokWaitingQueueId = __pdr.addString("WaitingQueueId");
	uint16	__TokSteps = __pdr.addString("Steps");
	uint16	__TokCompass = __pdr.addString("Compass");
	uint16	__TokStepsDone = __pdr.addString("StepsDone");
	uint16	__TokTeleports = __pdr.addString("Teleports");
	uint16	__TokInsidePlaces = __pdr.addString("InsidePlaces");
	uint16	__TokOutsidePlaces = __pdr.addString("OutsidePlaces");
	uint16	__TokHandledAIGroups = __pdr.addString("HandledAIGroups");
	_Container = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokTemplateId)
		{
			__pdr.pop(__TokTemplateId, _TemplateId);
		}
		else if (__pdr.peekNextToken() == __TokMainMissionTemplateId)
		{
			__pdr.pop(__TokMainMissionTemplateId, _MainMissionTemplateId);
		}
		else if (__pdr.peekNextToken() == __TokGiver)
		{
			__pdr.pop(__TokGiver, _Giver);
		}
		else if (__pdr.peekNextToken() == __TokHourLowerBound)
		{
			__pdr.pop(__TokHourLowerBound, _HourLowerBound);
		}
		else if (__pdr.peekNextToken() == __TokHourUpperBound)
		{
			__pdr.pop(__TokHourUpperBound, _HourUpperBound);
		}
		else if (__pdr.peekNextToken() == __TokSeason)
		{
			{
				std::string	valuename;
				__pdr.pop(__TokSeason, valuename);
				_Season = CSeason::fromString(valuename);
			}
		}
		else if (__pdr.peekNextToken() == __TokMonoEndDate)
		{
			__pdr.pop(__TokMonoEndDate, _MonoEndDate);
		}
		else if (__pdr.peekNextToken() == __TokEndDate)
		{
			__pdr.pop(__TokEndDate, _EndDate);
		}
		else if (__pdr.peekNextToken() == __TokCriticalPartEndDate)
		{
			__pdr.pop(__TokCriticalPartEndDate, _CriticalPartEndDate);
		}
		else if (__pdr.peekNextToken() == __TokBeginDate)
		{
			__pdr.pop(__TokBeginDate, _BeginDate);
		}
		else if (__pdr.peekNextToken() == __TokFailureIndex)
		{
			__pdr.pop(__TokFailureIndex, _FailureIndex);
		}
		else if (__pdr.peekNextToken() == __TokCrashHandlerIndex)
		{
			__pdr.pop(__TokCrashHandlerIndex, _CrashHandlerIndex);
		}
		else if (__pdr.peekNextToken() == __TokPlayerReconnectHandlerIndex)
		{
			__pdr.pop(__TokPlayerReconnectHandlerIndex, _PlayerReconnectHandlerIndex);
		}
		else if (__pdr.peekNextToken() == __TokFinished)
		{
			__pdr.pop(__TokFinished, _Finished);
		}
		else if (__pdr.peekNextToken() == __TokMissionSuccess)
		{
			__pdr.pop(__TokMissionSuccess, _MissionSuccess);
		}
		else if (__pdr.peekNextToken() == __TokDescIndex)
		{
			__pdr.pop(__TokDescIndex, _DescIndex);
		}
		else if (__pdr.peekNextToken() == __TokWaitingQueueId)
		{
			__pdr.pop(__TokWaitingQueueId, _WaitingQueueId);
		}
		// apply Steps
		else if (__pdr.peekNextToken() == __TokSteps)
		{
			__pdr.popStructBegin(__TokSteps);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CActiveStepPD&	obj = _Steps[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokSteps);
		}
		// end of apply Steps
		// apply Compass
		else if (__pdr.peekNextToken() == __TokCompass)
		{
			__pdr.popStructBegin(__TokCompass);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CMissionCompassPD&	obj = _Compass[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokCompass);
		}
		// end of apply Compass
		// apply StepsDone
		else if (__pdr.peekNextToken() == __TokStepsDone)
		{
			__pdr.popStructBegin(__TokStepsDone);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CDoneStepPD&	obj = _StepsDone[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokStepsDone);
		}
		// end of apply StepsDone
		// apply Teleports
		else if (__pdr.peekNextToken() == __TokTeleports)
		{
			__pdr.popStructBegin(__TokTeleports);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CMissionTeleportPD&	obj = _Teleports[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokTeleports);
		}
		// end of apply Teleports
		// apply InsidePlaces
		else if (__pdr.peekNextToken() == __TokInsidePlaces)
		{
			__pdr.popStructBegin(__TokInsidePlaces);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CMissionInsidePlacePD&	obj = _InsidePlaces[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokInsidePlaces);
		}
		// end of apply InsidePlaces
		// apply OutsidePlaces
		else if (__pdr.peekNextToken() == __TokOutsidePlaces)
		{
			__pdr.popStructBegin(__TokOutsidePlaces);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CMissionOutsidePlacePD&	obj = _OutsidePlaces[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokOutsidePlaces);
		}
		// end of apply OutsidePlaces
		// apply HandledAIGroups
		else if (__pdr.peekNextToken() == __TokHandledAIGroups)
		{
			__pdr.popStructBegin(__TokHandledAIGroups);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CHandledAIGroupPD&	obj = _HandledAIGroups[key];
				obj.apply(__pdr);
				obj.pds__setParentUnnotified(this);
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokHandledAIGroups);
		}
		// end of apply HandledAIGroups
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokTemplateId = __pdr.addString("TemplateId");
	uint16	__TokMainMissionTemplateId = __pdr.addString("MainMissionTemplateId");
	uint16	__TokGiver = __pdr.addString("Giver");
	uint16	__TokHourLowerBound = __pdr.addString("HourLowerBound");
	uint16	__TokHourUpperBound = __pdr.addString("HourUpperBound");
	uint16	__TokSeason = __pdr.addString("Season");
	uint16	__TokMonoEndDate = __pdr.addString("MonoEndDate");
	uint16	__TokEndDate = __pdr.addString("EndDate");
	uint16	__TokCriticalPartEndDate = __pdr.addString("CriticalPartEndDate");
	uint16	__TokBeginDate = __pdr.addString("BeginDate");
	uint16	__TokFailureIndex = __pdr.addString("FailureIndex");
	uint16	__TokCrashHandlerIndex = __pdr.addString("CrashHandlerIndex");
	uint16	__TokPlayerReconnectHandlerIndex = __pdr.addString("PlayerReconnectHandlerIndex");
	uint16	__TokFinished = __pdr.addString("Finished");
	uint16	__TokMissionSuccess = __pdr.addString("MissionSuccess");
	uint16	__TokDescIndex = __pdr.addString("DescIndex");
	uint16	__TokWaitingQueueId = __pdr.addString("WaitingQueueId");
	uint16	__TokSteps = __pdr.addString("Steps");
	uint16	__TokCompass = __pdr.addString("Compass");
	uint16	__TokStepsDone = __pdr.addString("StepsDone");
	uint16	__TokTeleports = __pdr.addString("Teleports");
	uint16	__TokInsidePlaces = __pdr.addString("InsidePlaces");
	uint16	__TokOutsidePlaces = __pdr.addString("OutsidePlaces");
	uint16	__TokHandledAIGroups = __pdr.addString("HandledAIGroups");
	__pdr.push(__TokTemplateId, _TemplateId);
	__pdr.push(__TokMainMissionTemplateId, _MainMissionTemplateId);
	__pdr.push(__TokGiver, _Giver);
	__pdr.push(__TokHourLowerBound, _HourLowerBound);
	__pdr.push(__TokHourUpperBound, _HourUpperBound);
	{
		std::string	valuename = CSeason::toString(_Season);
		__pdr.push(__TokSeason, valuename);
	}
	__pdr.push(__TokMonoEndDate, _MonoEndDate);
	__pdr.push(__TokEndDate, _EndDate);
	__pdr.push(__TokCriticalPartEndDate, _CriticalPartEndDate);
	__pdr.push(__TokBeginDate, _BeginDate);
	__pdr.push(__TokFailureIndex, _FailureIndex);
	__pdr.push(__TokCrashHandlerIndex, _CrashHandlerIndex);
	__pdr.push(__TokPlayerReconnectHandlerIndex, _PlayerReconnectHandlerIndex);
	__pdr.push(__TokFinished, _Finished);
	__pdr.push(__TokMissionSuccess, _MissionSuccess);
	__pdr.push(__TokDescIndex, _DescIndex);
	__pdr.push(__TokWaitingQueueId, _WaitingQueueId);
	// store Steps
	__pdr.pushStructBegin(__TokSteps);
	for (std::map<uint32, CActiveStepPD>::const_iterator it=_Steps.begin(); it!=_Steps.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokSteps);
	// end of store Steps
	// store Compass
	__pdr.pushStructBegin(__TokCompass);
	for (std::map<uint32, CMissionCompassPD>::const_iterator it=_Compass.begin(); it!=_Compass.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokCompass);
	// end of store Compass
	// store StepsDone
	__pdr.pushStructBegin(__TokStepsDone);
	for (std::map<uint32, CDoneStepPD>::const_iterator it=_StepsDone.begin(); it!=_StepsDone.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokStepsDone);
	// end of store StepsDone
	// store Teleports
	__pdr.pushStructBegin(__TokTeleports);
	for (std::map<uint32, CMissionTeleportPD>::const_iterator it=_Teleports.begin(); it!=_Teleports.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokTeleports);
	// end of store Teleports
	// store InsidePlaces
	__pdr.pushStructBegin(__TokInsidePlaces);
	for (std::map<uint32, CMissionInsidePlacePD>::const_iterator it=_InsidePlaces.begin(); it!=_InsidePlaces.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokInsidePlaces);
	// end of store InsidePlaces
	// store OutsidePlaces
	__pdr.pushStructBegin(__TokOutsidePlaces);
	for (std::map<uint32, CMissionOutsidePlacePD>::const_iterator it=_OutsidePlaces.begin(); it!=_OutsidePlaces.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokOutsidePlaces);
	// end of store OutsidePlaces
	// store HandledAIGroups
	__pdr.pushStructBegin(__TokHandledAIGroups);
	for (std::map<uint32, CHandledAIGroupPD>::const_iterator it=_HandledAIGroups.begin(); it!=_HandledAIGroups.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		(*it).second.store(__pdr);
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokHandledAIGroups);
	// end of store HandledAIGroups
}
void							CMissionPD::init()
{
}
void							CMissionPD::release()
{
}
void							CMissionPD::pds__init(const uint32 &TemplateId)
{
	_TemplateId = TemplateId;
	_MainMissionTemplateId = 0;
	_Giver = 0;
	_HourLowerBound = 0.0f;
	_HourUpperBound = 0.0f;
	_Season = (CSeason::TSeason)0;
	_MonoEndDate = 0;
	_EndDate = 0;
	_CriticalPartEndDate = 0;
	_BeginDate = 0;
	_FailureIndex = 0;
	_CrashHandlerIndex = 0xFFFFFFFF;
	_PlayerReconnectHandlerIndex = 0xFFFFFFFF;
	_Finished = false;
	_MissionSuccess = true;
	_DescIndex = 0;
	_WaitingQueueId = 0;
	_Container = NULL;
}
void							CMissionPD::pds__destroy()
{
	for (std::map<uint32, CActiveStepPD>::iterator __it=_Steps.begin(); __it!=_Steps.end(); )
	{
		std::map<uint32, CActiveStepPD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_Steps.clear();
	for (std::map<uint32, CMissionCompassPD>::iterator __it=_Compass.begin(); __it!=_Compass.end(); )
	{
		std::map<uint32, CMissionCompassPD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_Compass.clear();
	for (std::map<uint32, CDoneStepPD>::iterator __it=_StepsDone.begin(); __it!=_StepsDone.end(); )
	{
		std::map<uint32, CDoneStepPD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_StepsDone.clear();
	for (std::map<uint32, CMissionTeleportPD>::iterator __it=_Teleports.begin(); __it!=_Teleports.end(); )
	{
		std::map<uint32, CMissionTeleportPD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_Teleports.clear();
	for (std::map<uint32, CMissionInsidePlacePD>::iterator __it=_InsidePlaces.begin(); __it!=_InsidePlaces.end(); )
	{
		std::map<uint32, CMissionInsidePlacePD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_InsidePlaces.clear();
	for (std::map<uint32, CMissionOutsidePlacePD>::iterator __it=_OutsidePlaces.begin(); __it!=_OutsidePlaces.end(); )
	{
		std::map<uint32, CMissionOutsidePlacePD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_OutsidePlaces.clear();
	for (std::map<uint32, CHandledAIGroupPD>::iterator __it=_HandledAIGroups.begin(); __it!=_HandledAIGroups.end(); )
	{
		std::map<uint32, CHandledAIGroupPD>::iterator __itr=__it++;
		((*__itr).second).pds__destroy();
	}
	_HandledAIGroups.clear();
}
void							CMissionPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_TemplateId);
	data.serial(_MainMissionTemplateId);
	data.serial(_Giver);
	data.serial(_HourLowerBound);
	data.serial(_HourUpperBound);
	data.serialEnum(_Season);
	data.serial(_MonoEndDate);
	data.serial(_EndDate);
	data.serial(_CriticalPartEndDate);
	data.serial(_BeginDate);
	data.serial(_FailureIndex);
	data.serial(_CrashHandlerIndex);
	data.serial(_PlayerReconnectHandlerIndex);
	data.serial(_Finished);
	data.serial(_MissionSuccess);
	data.serial(_DescIndex);
	data.serial(_WaitingQueueId);
	RY_PDS::TTableIndex	tableIndex;
	RY_PDS::TRowIndex	rowIndex;
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_Steps.insert(std::make_pair<uint32,CActiveStepPD>(__k, CActiveStepPD()));
		CActiveStepPD*	__o = &(_Steps[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_Compass.insert(std::make_pair<uint32,CMissionCompassPD>(__k, CMissionCompassPD()));
		CMissionCompassPD*	__o = &(_Compass[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_StepsDone.insert(std::make_pair<uint32,CDoneStepPD>(__k, CDoneStepPD()));
		CDoneStepPD*	__o = &(_StepsDone[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_Teleports.insert(std::make_pair<uint32,CMissionTeleportPD>(__k, CMissionTeleportPD()));
		CMissionTeleportPD*	__o = &(_Teleports[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_InsidePlaces.insert(std::make_pair<uint32,CMissionInsidePlacePD>(__k, CMissionInsidePlacePD()));
		CMissionInsidePlacePD*	__o = &(_InsidePlaces[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_OutsidePlaces.insert(std::make_pair<uint32,CMissionOutsidePlacePD>(__k, CMissionOutsidePlacePD()));
		CMissionOutsidePlacePD*	__o = &(_OutsidePlaces[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		_HandledAIGroups.insert(std::make_pair<uint32,CHandledAIGroupPD>(__k, CHandledAIGroupPD()));
		CHandledAIGroupPD*	__o = &(_HandledAIGroups[__k]);
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	_Container = NULL;
}
void							CMissionPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(__BaseTable, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionPD: registerAttributes %u:%u", __BaseTable, __BaseRow);
	PDSLib.set(__BaseTable, __BaseRow, (RY_PDS::TColumnIndex)(0), _TemplateId);
}
void							CMissionPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(__BaseTable, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionPD: unregisterAttributes %u:%u", __BaseTable, __BaseRow);
	pds__setParent(NULL);
	for (std::map<uint32, CActiveStepPD>::iterator __it=_Steps.begin(); __it!=_Steps.end(); )
	{
		std::map<uint32, CActiveStepPD>::iterator __itr=__it++;
		CActiveStepPD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
	for (std::map<uint32, CMissionCompassPD>::iterator __it=_Compass.begin(); __it!=_Compass.end(); )
	{
		std::map<uint32, CMissionCompassPD>::iterator __itr=__it++;
		CMissionCompassPD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
	for (std::map<uint32, CDoneStepPD>::iterator __it=_StepsDone.begin(); __it!=_StepsDone.end(); )
	{
		std::map<uint32, CDoneStepPD>::iterator __itr=__it++;
		CDoneStepPD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
	for (std::map<uint32, CMissionTeleportPD>::iterator __it=_Teleports.begin(); __it!=_Teleports.end(); )
	{
		std::map<uint32, CMissionTeleportPD>::iterator __itr=__it++;
		CMissionTeleportPD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
	for (std::map<uint32, CMissionInsidePlacePD>::iterator __it=_InsidePlaces.begin(); __it!=_InsidePlaces.end(); )
	{
		std::map<uint32, CMissionInsidePlacePD>::iterator __itr=__it++;
		CMissionInsidePlacePD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
	for (std::map<uint32, CMissionOutsidePlacePD>::iterator __it=_OutsidePlaces.begin(); __it!=_OutsidePlaces.end(); )
	{
		std::map<uint32, CMissionOutsidePlacePD>::iterator __itr=__it++;
		CMissionOutsidePlacePD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
	for (std::map<uint32, CHandledAIGroupPD>::iterator __it=_HandledAIGroups.begin(); __it!=_HandledAIGroups.end(); )
	{
		std::map<uint32, CHandledAIGroupPD>::iterator __itr=__it++;
		CHandledAIGroupPD&	__o = (*__itr).second;
		__o.pds__unregister();
	}
}
void							CMissionPD::pds__setParent(CMissionContainerPD* __parent)
{
	NLMISC::CEntityId	prevId;
	if (_Container != NULL)
	{
		prevId = _Container->getCharId();
		_Container->pds__unlinkMissions(_TemplateId);
	}
	_Container = __parent;
	PDSLib.setParent(__BaseTable, getRow(), (RY_PDS::TColumnIndex)(24), (__parent != NULL ? RY_PDS::CObjectIndex(18, __parent->getRow()) : RY_PDS::CObjectIndex::null()), (_Container != NULL ? _Container->getCharId() : NLMISC::CEntityId::Unknown), prevId);
}
void							CMissionPD::pds__setParentUnnotified(CMissionContainerPD* __parent)
{
	_Container = __parent;
}
void							CMissionPD::pds__notifyInit()
{
	init();
	{
		// CMissionPD init user code, defined at entities_game_service/pd_scripts/mission.pds:75
		_Mission = this;
	}
	for (std::map<uint32, CActiveStepPD>::iterator __it=_Steps.begin(); __it!=_Steps.end(); )
	{
		std::map<uint32, CActiveStepPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
	for (std::map<uint32, CMissionCompassPD>::iterator __it=_Compass.begin(); __it!=_Compass.end(); )
	{
		std::map<uint32, CMissionCompassPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
	for (std::map<uint32, CDoneStepPD>::iterator __it=_StepsDone.begin(); __it!=_StepsDone.end(); )
	{
		std::map<uint32, CDoneStepPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
	for (std::map<uint32, CMissionTeleportPD>::iterator __it=_Teleports.begin(); __it!=_Teleports.end(); )
	{
		std::map<uint32, CMissionTeleportPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
	for (std::map<uint32, CMissionInsidePlacePD>::iterator __it=_InsidePlaces.begin(); __it!=_InsidePlaces.end(); )
	{
		std::map<uint32, CMissionInsidePlacePD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
	for (std::map<uint32, CMissionOutsidePlacePD>::iterator __it=_OutsidePlaces.begin(); __it!=_OutsidePlaces.end(); )
	{
		std::map<uint32, CMissionOutsidePlacePD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
	for (std::map<uint32, CHandledAIGroupPD>::iterator __it=_HandledAIGroups.begin(); __it!=_HandledAIGroups.end(); )
	{
		std::map<uint32, CHandledAIGroupPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyInit();
	}
}
void							CMissionPD::pds__notifyRelease()
{
	release();
	PDSLib.release(__BaseTable, __BaseRow);
	for (std::map<uint32, CActiveStepPD>::iterator __it=_Steps.begin(); __it!=_Steps.end(); )
	{
		std::map<uint32, CActiveStepPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
	for (std::map<uint32, CMissionCompassPD>::iterator __it=_Compass.begin(); __it!=_Compass.end(); )
	{
		std::map<uint32, CMissionCompassPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
	for (std::map<uint32, CDoneStepPD>::iterator __it=_StepsDone.begin(); __it!=_StepsDone.end(); )
	{
		std::map<uint32, CDoneStepPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
	for (std::map<uint32, CMissionTeleportPD>::iterator __it=_Teleports.begin(); __it!=_Teleports.end(); )
	{
		std::map<uint32, CMissionTeleportPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
	for (std::map<uint32, CMissionInsidePlacePD>::iterator __it=_InsidePlaces.begin(); __it!=_InsidePlaces.end(); )
	{
		std::map<uint32, CMissionInsidePlacePD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
	for (std::map<uint32, CMissionOutsidePlacePD>::iterator __it=_OutsidePlaces.begin(); __it!=_OutsidePlaces.end(); )
	{
		std::map<uint32, CMissionOutsidePlacePD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
	for (std::map<uint32, CHandledAIGroupPD>::iterator __it=_HandledAIGroups.begin(); __it!=_HandledAIGroups.end(); )
	{
		std::map<uint32, CHandledAIGroupPD>::iterator __itr=__it++;
		(*__itr).second.pds__notifyRelease();
	}
}
void							CMissionPD::pds_static__init()
{
	PDSLib.setIndexAllocator(14, _IndexAllocator);
	nlassertex(_FactoryInitialised, ("User Factory for class CMissionPD not set!"));
	// factory must have been set by user before database init called!
	// You must provide a factory for the class CMissionPD as it is marked as derived
	// Call EGSPD::CMissionPD::setFactory() with a factory before any call to EGSPD::init()!
}
void							CMissionPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(14, userFactory, pds_static__fetch, NULL);
		_FactoryInitialised = true;
	}
}
bool							CMissionPD::_FactoryInitialised;
RY_PDS::CIndexAllocator			CMissionPD::_IndexAllocator;
void							CMissionPD::pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data)
{
	CMissionPD	*__o = static_cast<CMissionPD*>(object);
	__o->pds__fetch(data);
	__o->pds__notifyInit();
}
// End of static implementation of CMissionPD

/* -----------------------------------------
* Static Implementation of CMissionGuildPD
* ----------------------------------------- */
void							CMissionGuildPD::clear()
{
	CMissionPD::clear();
}
CMissionGuildPD*				CMissionGuildPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 15) ? static_cast<CMissionGuildPD*>(obj) : NULL;
}
const CMissionGuildPD*			CMissionGuildPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 15) ? static_cast<const CMissionGuildPD*>(obj) : NULL;
}
void							CMissionGuildPD::setFactory(RY_PDS::TPDFactory userFactory)
{
	pds_static__setFactory(userFactory);
}
CMissionGuildPD*				CMissionGuildPD::create(const uint32 &TemplateId)
{
	CMissionGuildPD	*__o = static_cast<CMissionGuildPD*>(PDSLib.create(15));
	__o->pds__init(TemplateId);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CMissionGuildPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __Tok_Parent)
		{
			__pdr.popStructBegin(__Tok_Parent);
			CMissionPD::apply(__pdr);
			__pdr.popStructEnd(__Tok_Parent);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionGuildPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	__pdr.pushStructBegin(__Tok_Parent);
	CMissionPD::store(__pdr);
	__pdr.pushStructEnd(__Tok_Parent);
}
void							CMissionGuildPD::init()
{
}
void							CMissionGuildPD::release()
{
}
void							CMissionGuildPD::pds__init(const uint32 &TemplateId)
{
	CMissionPD::pds__init(TemplateId);
}
void							CMissionGuildPD::pds__destroy()
{
	CMissionPD::pds__destroy();
}
void							CMissionGuildPD::pds__fetch(RY_PDS::CPData &data)
{
	CMissionPD::pds__fetch(data);
}
void							CMissionGuildPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(__BaseTable, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionGuildPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionGuildPD: registerAttributes %u:%u", __BaseTable, __BaseRow);
	CMissionPD::pds__registerAttributes();
}
void							CMissionGuildPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(__BaseTable, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionGuildPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionGuildPD: unregisterAttributes %u:%u", __BaseTable, __BaseRow);
	CMissionPD::pds__unregisterAttributes();
}
void							CMissionGuildPD::pds__notifyInit()
{
	CMissionPD::pds__notifyInit();
	init();
}
void							CMissionGuildPD::pds__notifyRelease()
{
	release();
	CMissionPD::pds__notifyRelease();
}
void							CMissionGuildPD::pds_static__init()
{
	PDSLib.setIndexAllocator(15, _IndexAllocator);
	nlassertex(_FactoryInitialised, ("User Factory for class CMissionGuildPD not set!"));
	// factory must have been set by user before database init called!
	// You must provide a factory for the class CMissionGuildPD as it is marked as derived
	// Call EGSPD::CMissionGuildPD::setFactory() with a factory before any call to EGSPD::init()!
}
void							CMissionGuildPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(15, userFactory, pds_static__fetch, NULL);
		_FactoryInitialised = true;
	}
}
bool							CMissionGuildPD::_FactoryInitialised;
RY_PDS::CIndexAllocator			CMissionGuildPD::_IndexAllocator;
// End of static implementation of CMissionGuildPD

/* -----------------------------------------
* Static Implementation of CMissionTeamPD
* ----------------------------------------- */
void							CMissionTeamPD::clear()
{
	CMissionPD::clear();
}
CMissionTeamPD*					CMissionTeamPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 16) ? static_cast<CMissionTeamPD*>(obj) : NULL;
}
const CMissionTeamPD*			CMissionTeamPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 16) ? static_cast<const CMissionTeamPD*>(obj) : NULL;
}
void							CMissionTeamPD::setFactory(RY_PDS::TPDFactory userFactory)
{
	pds_static__setFactory(userFactory);
}
CMissionTeamPD*					CMissionTeamPD::create(const uint32 &TemplateId)
{
	CMissionTeamPD	*__o = static_cast<CMissionTeamPD*>(PDSLib.create(16));
	__o->pds__init(TemplateId);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CMissionTeamPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __Tok_Parent)
		{
			__pdr.popStructBegin(__Tok_Parent);
			CMissionPD::apply(__pdr);
			__pdr.popStructEnd(__Tok_Parent);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionTeamPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	__pdr.pushStructBegin(__Tok_Parent);
	CMissionPD::store(__pdr);
	__pdr.pushStructEnd(__Tok_Parent);
}
void							CMissionTeamPD::init()
{
}
void							CMissionTeamPD::release()
{
}
void							CMissionTeamPD::pds__init(const uint32 &TemplateId)
{
	CMissionPD::pds__init(TemplateId);
}
void							CMissionTeamPD::pds__destroy()
{
	CMissionPD::pds__destroy();
}
void							CMissionTeamPD::pds__fetch(RY_PDS::CPData &data)
{
	CMissionPD::pds__fetch(data);
}
void							CMissionTeamPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(__BaseTable, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionTeamPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionTeamPD: registerAttributes %u:%u", __BaseTable, __BaseRow);
	CMissionPD::pds__registerAttributes();
}
void							CMissionTeamPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(__BaseTable, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionTeamPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionTeamPD: unregisterAttributes %u:%u", __BaseTable, __BaseRow);
	CMissionPD::pds__unregisterAttributes();
}
void							CMissionTeamPD::pds__notifyInit()
{
	CMissionPD::pds__notifyInit();
	init();
}
void							CMissionTeamPD::pds__notifyRelease()
{
	release();
	CMissionPD::pds__notifyRelease();
}
void							CMissionTeamPD::pds_static__init()
{
	PDSLib.setIndexAllocator(16, _IndexAllocator);
	nlassertex(_FactoryInitialised, ("User Factory for class CMissionTeamPD not set!"));
	// factory must have been set by user before database init called!
	// You must provide a factory for the class CMissionTeamPD as it is marked as derived
	// Call EGSPD::CMissionTeamPD::setFactory() with a factory before any call to EGSPD::init()!
}
void							CMissionTeamPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(16, userFactory, pds_static__fetch, NULL);
		_FactoryInitialised = true;
	}
}
bool							CMissionTeamPD::_FactoryInitialised;
RY_PDS::CIndexAllocator			CMissionTeamPD::_IndexAllocator;
// End of static implementation of CMissionTeamPD

/* -----------------------------------------
* Static Implementation of CMissionSoloPD
* ----------------------------------------- */
void							CMissionSoloPD::clear()
{
	CMissionPD::clear();
}
CMissionSoloPD*					CMissionSoloPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 17) ? static_cast<CMissionSoloPD*>(obj) : NULL;
}
const CMissionSoloPD*			CMissionSoloPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 17) ? static_cast<const CMissionSoloPD*>(obj) : NULL;
}
void							CMissionSoloPD::setFactory(RY_PDS::TPDFactory userFactory)
{
	pds_static__setFactory(userFactory);
}
CMissionSoloPD*					CMissionSoloPD::create(const uint32 &TemplateId)
{
	CMissionSoloPD	*__o = static_cast<CMissionSoloPD*>(PDSLib.create(17));
	__o->pds__init(TemplateId);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CMissionSoloPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __Tok_Parent)
		{
			__pdr.popStructBegin(__Tok_Parent);
			CMissionPD::apply(__pdr);
			__pdr.popStructEnd(__Tok_Parent);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CMissionSoloPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__Tok_Parent = __pdr.addString("__Parent__");
	__pdr.pushStructBegin(__Tok_Parent);
	CMissionPD::store(__pdr);
	__pdr.pushStructEnd(__Tok_Parent);
}
void							CMissionSoloPD::init()
{
}
void							CMissionSoloPD::release()
{
}
void							CMissionSoloPD::pds__init(const uint32 &TemplateId)
{
	CMissionPD::pds__init(TemplateId);
}
void							CMissionSoloPD::pds__destroy()
{
	CMissionPD::pds__destroy();
}
void							CMissionSoloPD::pds__fetch(RY_PDS::CPData &data)
{
	CMissionPD::pds__fetch(data);
}
void							CMissionSoloPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(__BaseTable, __BaseRow, 0);
	pds__registerAttributes();
}
void							CMissionSoloPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionSoloPD: registerAttributes %u:%u", __BaseTable, __BaseRow);
	CMissionPD::pds__registerAttributes();
}
void							CMissionSoloPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(__BaseTable, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionSoloPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionSoloPD: unregisterAttributes %u:%u", __BaseTable, __BaseRow);
	CMissionPD::pds__unregisterAttributes();
}
void							CMissionSoloPD::pds__notifyInit()
{
	CMissionPD::pds__notifyInit();
	init();
}
void							CMissionSoloPD::pds__notifyRelease()
{
	release();
	CMissionPD::pds__notifyRelease();
}
void							CMissionSoloPD::pds_static__init()
{
	PDSLib.setIndexAllocator(17, _IndexAllocator);
	nlassertex(_FactoryInitialised, ("User Factory for class CMissionSoloPD not set!"));
	// factory must have been set by user before database init called!
	// You must provide a factory for the class CMissionSoloPD as it is marked as derived
	// Call EGSPD::CMissionSoloPD::setFactory() with a factory before any call to EGSPD::init()!
}
void							CMissionSoloPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(17, userFactory, pds_static__fetch, NULL);
		_FactoryInitialised = true;
	}
}
bool							CMissionSoloPD::_FactoryInitialised;
RY_PDS::CIndexAllocator			CMissionSoloPD::_IndexAllocator;
// End of static implementation of CMissionSoloPD

/* -----------------------------------------
* Static Implementation of CMissionContainerPD
* ----------------------------------------- */
NLMISC::CEntityId				CMissionContainerPD::getCharId() const
{
	return _CharId;
}
CMissionPD*						CMissionContainerPD::getMissions(const uint32& __k)
{
	std::map<uint32, CMissionPD*>::iterator _it = _Missions.find(__k);
	return (_it==_Missions.end() ? NULL : (*_it).second);
}
const CMissionPD*				CMissionContainerPD::getMissions(const uint32& __k) const
{
	std::map<uint32, CMissionPD*>::const_iterator _it = _Missions.find(__k);
	return (_it==_Missions.end() ? NULL : (*_it).second);
}
std::map<uint32, CMissionPD*>::iterator	CMissionContainerPD::getMissionsBegin()
{
	return _Missions.begin();
}
std::map<uint32, CMissionPD*>::iterator	CMissionContainerPD::getMissionsEnd()
{
	return _Missions.end();
}
std::map<uint32, CMissionPD*>::const_iterator	CMissionContainerPD::getMissionsBegin() const
{
	return _Missions.begin();
}
std::map<uint32, CMissionPD*>::const_iterator	CMissionContainerPD::getMissionsEnd() const
{
	return _Missions.end();
}
const std::map<uint32, CMissionPD*> &	CMissionContainerPD::getMissions() const
{
	return _Missions;
}
void							CMissionContainerPD::setMissions(CMissionPD* __v)
{
	if (__v == NULL)	return;
	uint32	__k = __v->getTemplateId();
	std::map<uint32, CMissionPD*>::iterator	_it = _Missions.find(__k);
	if (_it != _Missions.end())
	{
		CMissionPD*	__prev = (*_it).second;
		if (__prev == __v)	return;
		__prev->pds__setParent(NULL);
		__prev->pds__unregister();
		__prev->pds__destroy();
		delete __prev;
	}
	__v->pds__setParent(this);
	_Missions[__k] = __v;
}
void							CMissionContainerPD::deleteFromMissions(const uint32 &__k)
{
	std::map<uint32, CMissionPD*>::iterator	__it = _Missions.find(__k);
	if (__it == _Missions.end())	return;
	CMissionPD*	__o = (*__it).second;
	__o->pds__unregister();
	__o->pds__destroy();
	delete __o;
}
void							CMissionContainerPD::clear()
{
	for (std::map<uint32, CMissionPD*>::iterator __it=_Missions.begin(); __it!=_Missions.end(); )
	{
		std::map<uint32, CMissionPD*>::iterator __itr=__it++;
		CMissionPD*	__o = (*__itr).second;
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
	_Missions.clear();
}
CMissionContainerPD*			CMissionContainerPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 18) ? static_cast<CMissionContainerPD*>(obj) : NULL;
}
const CMissionContainerPD*		CMissionContainerPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 18) ? static_cast<const CMissionContainerPD*>(obj) : NULL;
}
CMissionContainerPD*			CMissionContainerPD::create(const NLMISC::CEntityId &CharId)
{
	CMissionContainerPD	*__o = static_cast<CMissionContainerPD*>(pds_static__factory());
	__o->pds__init(CharId);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CMissionContainerPD::remove(const NLMISC::CEntityId& CharId)
{
	std::map<NLMISC::CEntityId,CMissionContainerPD*>::iterator	it = _Map.find(CharId);
	if (it != _Map.end())
	{
		CMissionContainerPD*	__o = (*it).second;
		__o->pds__notifyRelease();
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
}
void							CMissionContainerPD::load(const NLMISC::CEntityId& CharId)
{
	PDSLib.load(18, CharId.asUint64());
}
void							CMissionContainerPD::setLoadCallback(void (*callback)(const NLMISC::CEntityId& key, CMissionContainerPD* object))
{
	__pds__LoadCallback = callback;
}
void							CMissionContainerPD::unload(const NLMISC::CEntityId &CharId)
{
	std::map<NLMISC::CEntityId,CMissionContainerPD*>::iterator	it = _Map.find(CharId);
	if (it != _Map.end())
	{
		CMissionContainerPD*	__o = (*it).second;
		__o->pds__notifyRelease();
		__o->pds__destroy();
		delete __o;
	}
}
CMissionContainerPD*			CMissionContainerPD::get(const NLMISC::CEntityId &CharId)
{
	std::map<NLMISC::CEntityId, CMissionContainerPD*>::iterator	__it = _Map.find(CharId);
	return (__it != _Map.end()) ? (*__it).second : NULL;
}
std::map<NLMISC::CEntityId, CMissionContainerPD*>::iterator	CMissionContainerPD::begin()
{
	return _Map.begin();
}
std::map<NLMISC::CEntityId, CMissionContainerPD*>::iterator	CMissionContainerPD::end()
{
	return _Map.end();
}
void							CMissionContainerPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokCharId = __pdr.addString("CharId");
	uint16	__TokMissions = __pdr.addString("Missions");
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokCharId)
		{
			__pdr.pop(__TokCharId, _CharId);
		}
		// apply Missions
		else if (__pdr.peekNextToken() == __TokMissions)
		{
			__pdr.popStructBegin(__TokMissions);
			while (!__pdr.isEndOfStruct())
			{
				uint32	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CMissionPD*	obj;
				obj = NULL;
				if (__pdr.peekNextToken() == __Tok_ClassName)
				{
					std::string	__className;
					__pdr.pop(__Tok_ClassName, __className);
					obj = CMissionPD::cast(PDSLib.create(__className));
					if (obj != NULL)
					{
						__pdr.popStructBegin(__TokMissions);
						obj->apply(__pdr);
						obj->pds__setParentUnnotified(this);
						__pdr.popStructEnd(__TokMissions);
					}
					else
					{
						__pdr.skipStruct();
					}
				}
				if (obj !=  NULL)
				{
					_Missions[key] = obj;
				}
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokMissions);
		}
		// end of apply Missions
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
	pds__notifyInit();
}
void							CMissionContainerPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokCharId = __pdr.addString("CharId");
	uint16	__TokMissions = __pdr.addString("Missions");
	__pdr.push(__TokCharId, _CharId);
	// store Missions
	__pdr.pushStructBegin(__TokMissions);
	for (std::map<uint32, CMissionPD*>::const_iterator it=_Missions.begin(); it!=_Missions.end(); ++it)
	{
		uint32	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		if ((*it).second != NULL)
		{
			std::string	__className = PDSLib.getClassName((*it).second);
			__pdr.push(__Tok_ClassName, __className);
			__pdr.pushStructBegin(__TokMissions);
			(*it).second->store(__pdr);
			__pdr.pushStructEnd(__TokMissions);
		}
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokMissions);
	// end of store Missions
}
void							CMissionContainerPD::pds__init(const NLMISC::CEntityId &CharId)
{
	_CharId = CharId;
	_Map[getCharId()] = this;
}
void							CMissionContainerPD::pds__destroy()
{
	for (std::map<uint32, CMissionPD*>::iterator __it=_Missions.begin(); __it!=_Missions.end(); )
	{
		std::map<uint32, CMissionPD*>::iterator __itr=__it++;
		CMissionPD*	__o = ((*__itr).second);
		if (__o != NULL)
		{
			__o->pds__destroy();
			delete __o;
		}
	}
	_Missions.clear();
	_Map.erase(getCharId());
}
void							CMissionContainerPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_CharId);
	RY_PDS::TTableIndex	tableIndex;
	RY_PDS::TRowIndex	rowIndex;
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		uint32	__k;
		data.serial(__k);
		CMissionPD*	__o = static_cast<CMissionPD*>(PDSLib.create(tableIndex));
		_Missions.insert(std::make_pair<uint32,CMissionPD*>(__k, __o));
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	_Map[getCharId()] = this;
}
void							CMissionContainerPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(18, __BaseRow, _CharId.asUint64(), _CharId);
	pds__registerAttributes();
}
void							CMissionContainerPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionContainerPD: registerAttributes %u:%u", 18, __BaseRow);
	PDSLib.set(18, __BaseRow, (RY_PDS::TColumnIndex)(0), _CharId);
}
void							CMissionContainerPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(18, __BaseRow, _CharId);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CMissionContainerPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CMissionContainerPD: unregisterAttributes %u:%u", 18, __BaseRow);
	for (std::map<uint32, CMissionPD*>::iterator __it=_Missions.begin(); __it!=_Missions.end(); )
	{
		std::map<uint32, CMissionPD*>::iterator __itr=__it++;
		CMissionPD*	__o = (*__itr).second;
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
}
void							CMissionContainerPD::pds__notifyInit()
{
	for (std::map<uint32, CMissionPD*>::iterator __it=_Missions.begin(); __it!=_Missions.end(); )
	{
		std::map<uint32, CMissionPD*>::iterator __itr=__it++;
		(*__itr).second->pds__notifyInit();
	}
}
void							CMissionContainerPD::pds__notifyRelease()
{
	PDSLib.release(18, __BaseRow);
	for (std::map<uint32, CMissionPD*>::iterator __it=_Missions.begin(); __it!=_Missions.end(); )
	{
		std::map<uint32, CMissionPD*>::iterator __itr=__it++;
		(*__itr).second->pds__notifyRelease();
	}
}
void							CMissionContainerPD::pds__unlinkMissions(uint32 __k)
{
	_Missions.erase(__k);
}
void							CMissionContainerPD::pds_static__init()
{
	PDSLib.setIndexAllocator(18, _IndexAllocator);
	pds_static__setFactory(pds_static__factory);
}
std::map<NLMISC::CEntityId,CMissionContainerPD*>	CMissionContainerPD::_Map;
void							CMissionContainerPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(18, userFactory, pds_static__fetch, pds_static__notifyFailure);
		_FactoryInitialised = true;
	}
}
bool							CMissionContainerPD::_FactoryInitialised;
void							CMissionContainerPD::pds_static__notifyFailure(uint64 key)
{
	if (__pds__LoadCallback != NULL)
	{
		__pds__LoadCallback((NLMISC::CEntityId)key, NULL);
	}
}
void							(*CMissionContainerPD::__pds__LoadCallback)(const NLMISC::CEntityId& key, CMissionContainerPD* object) = NULL;
RY_PDS::CIndexAllocator			CMissionContainerPD::_IndexAllocator;
RY_PDS::IPDBaseData*			CMissionContainerPD::pds_static__factory()
{
	return new CMissionContainerPD();
}
void							CMissionContainerPD::pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data)
{
	CMissionContainerPD	*__o = static_cast<CMissionContainerPD*>(object);
	__o->pds__fetch(data);
	if (__pds__LoadCallback != NULL)
	{
		__pds__LoadCallback(__o->getCharId(), __o);
	}
	__o->pds__notifyInit();
}
// End of static implementation of CMissionContainerPD

	
} // End of EGSPD
