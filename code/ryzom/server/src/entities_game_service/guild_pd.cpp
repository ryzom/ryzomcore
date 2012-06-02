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
* Static Implementation of CGuildPD
* ----------------------------------------- */
TGuildId						CGuildPD::getId() const
{
	return _Id;
}
uint64							CGuildPD::getMoney() const
{
	return _Money;
}
void							CGuildPD::setMoney(uint64 __v, bool forceWrite)
{
	if ((_Money != __v) || forceWrite)
	{
		PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(1), __v);
	}
	_Money = __v;
}
uint32							CGuildPD::getCreationDate() const
{
	return _CreationDate;
}
void							CGuildPD::setCreationDate(uint32 __v, bool forceWrite)
{
	if ((_CreationDate != __v) || forceWrite)
	{
		PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(2), __v);
	}
	_CreationDate = __v;
}
CPeople::TPeople				CGuildPD::getRace() const
{
	return _Race;
}
void							CGuildPD::setRace(CPeople::TPeople __v, bool forceWrite)
{
	nlassert(__v<CPeople::___TPeople_useSize);
	if ((_Race != __v) || forceWrite)
	{
		PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(3), (uint32)__v);
	}
	_Race = __v;
}
uint64							CGuildPD::getIcon() const
{
	return _Icon;
}
void							CGuildPD::setIcon(uint64 __v, bool forceWrite)
{
	if ((_Icon != __v) || forceWrite)
	{
		PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(4), __v);
	}
	_Icon = __v;
}
uint32							CGuildPD::getBuilding() const
{
	return _Building;
}
void							CGuildPD::setBuilding(uint32 __v, bool forceWrite)
{
	if ((_Building != __v) || forceWrite)
	{
		PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(5), __v);
	}
	_Building = __v;
}
uint32							CGuildPD::getVersion() const
{
	return _Version;
}
void							CGuildPD::setVersion(uint32 __v, bool forceWrite)
{
	if ((_Version != __v) || forceWrite)
	{
		PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(6), __v);
	}
	_Version = __v;
}
CGuildMemberPD*					CGuildPD::getMembers(const TCharacterId& __k)
{
	std::map<TCharacterId, CGuildMemberPD*>::iterator _it = _Members.find(__k);
	return (_it==_Members.end() ? NULL : (*_it).second);
}
const CGuildMemberPD*			CGuildPD::getMembers(const TCharacterId& __k) const
{
	std::map<TCharacterId, CGuildMemberPD*>::const_iterator _it = _Members.find(__k);
	return (_it==_Members.end() ? NULL : (*_it).second);
}
std::map<TCharacterId, CGuildMemberPD*>::iterator	CGuildPD::getMembersBegin()
{
	return _Members.begin();
}
std::map<TCharacterId, CGuildMemberPD*>::iterator	CGuildPD::getMembersEnd()
{
	return _Members.end();
}
std::map<TCharacterId, CGuildMemberPD*>::const_iterator	CGuildPD::getMembersBegin() const
{
	return _Members.begin();
}
std::map<TCharacterId, CGuildMemberPD*>::const_iterator	CGuildPD::getMembersEnd() const
{
	return _Members.end();
}
const std::map<TCharacterId, CGuildMemberPD*> &	CGuildPD::getMembers() const
{
	return _Members;
}
void							CGuildPD::setMembers(CGuildMemberPD* __v)
{
	if (__v == NULL)	return;
	TCharacterId	__k = __v->getId();
	std::map<TCharacterId, CGuildMemberPD*>::iterator	_it = _Members.find(__k);
	if (_it != _Members.end())
	{
		CGuildMemberPD*	__prev = (*_it).second;
		if (__prev == __v)	return;
		__prev->pds__setParent(NULL);
		__prev->pds__unregister();
		__prev->pds__destroy();
		delete __prev;
	}
	__v->pds__setParent(this);
	_Members[__k] = __v;
	{
		// callback the manager 
		IGuildManager::getInstance().guildMemberListChanged(this);
	}
}
void							CGuildPD::deleteFromMembers(const TCharacterId &__k)
{
	std::map<TCharacterId, CGuildMemberPD*>::iterator	__it = _Members.find(__k);
	if (__it == _Members.end())	return;
	CGuildMemberPD*	__o = (*__it).second;
	__o->pds__unregister();
	__o->pds__destroy();
	delete __o;
	{
		// callback the manager 
		IGuildManager::getInstance().guildMemberListChanged(this);
	}
}
CGuildFameContainerPD*			CGuildPD::getFameContainer()
{
	return _FameContainer;
}
const CGuildFameContainerPD*	CGuildPD::getFameContainer() const
{
	return _FameContainer;
}
void							CGuildPD::setFameContainer(CGuildFameContainerPD* __v)
{
	if (_FameContainer != NULL)
	{
		_FameContainer->pds__setParent(NULL);
	}
	__v->pds__setParent(this);
	_FameContainer = __v;
}
CGuildContainerPD*				CGuildPD::getParent()
{
	return _Parent;
}
const CGuildContainerPD*		CGuildPD::getParent() const
{
	return _Parent;
}
void							CGuildPD::clear()
{
	_Money = (uint64)0;
	PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(1), (uint64)0);
	_CreationDate = 0;
	PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(2), 0);
	_Race = (CPeople::TPeople)0;
	PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(3), (uint32)(CPeople::TPeople)0);
	_Icon = (uint64)0;
	PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(4), (uint64)0);
	_Building = 0;
	PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(5), 0);
	_Version = 0;
	PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(6), 0);
	for (std::map<TCharacterId, CGuildMemberPD*>::iterator __it=_Members.begin(); __it!=_Members.end(); )
	{
		std::map<TCharacterId, CGuildMemberPD*>::iterator __itr=__it++;
		CGuildMemberPD*	__o = (*__itr).second;
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
	_Members.clear();
	CGuildFameContainerPD*	__o = _FameContainer;
	__o->pds__unregister();
	__o->pds__destroy();
	delete __o;
}
CGuildPD*						CGuildPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 4) ? static_cast<CGuildPD*>(obj) : NULL;
}
const CGuildPD*					CGuildPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 4) ? static_cast<const CGuildPD*>(obj) : NULL;
}
void							CGuildPD::setFactory(RY_PDS::TPDFactory userFactory)
{
	pds_static__setFactory(userFactory);
}
CGuildPD*						CGuildPD::create(const TGuildId &Id)
{
	CGuildPD	*__o = static_cast<CGuildPD*>(PDSLib.create(4));
	__o->pds__init(Id);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CGuildPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokId = __pdr.addString("Id");
	uint16	__TokMoney = __pdr.addString("Money");
	uint16	__TokCreationDate = __pdr.addString("CreationDate");
	uint16	__TokRace = __pdr.addString("Race");
	uint16	__TokIcon = __pdr.addString("Icon");
	uint16	__TokBuilding = __pdr.addString("Building");
	uint16	__TokVersion = __pdr.addString("Version");
	uint16	__TokMembers = __pdr.addString("Members");
	uint16	__TokFameContainer = __pdr.addString("FameContainer");
	_Parent = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokId)
		{
			__pdr.pop(__TokId, _Id);
		}
		else if (__pdr.peekNextToken() == __TokMoney)
		{
			__pdr.pop(__TokMoney, _Money);
		}
		else if (__pdr.peekNextToken() == __TokCreationDate)
		{
			__pdr.pop(__TokCreationDate, _CreationDate);
		}
		else if (__pdr.peekNextToken() == __TokRace)
		{
			{
				std::string	valuename;
				__pdr.pop(__TokRace, valuename);
				_Race = CPeople::fromString(valuename);
			}
		}
		else if (__pdr.peekNextToken() == __TokIcon)
		{
			__pdr.pop(__TokIcon, _Icon);
		}
		else if (__pdr.peekNextToken() == __TokBuilding)
		{
			__pdr.pop(__TokBuilding, _Building);
		}
		else if (__pdr.peekNextToken() == __TokVersion)
		{
			__pdr.pop(__TokVersion, _Version);
		}
		// apply Members
		else if (__pdr.peekNextToken() == __TokMembers)
		{
			__pdr.popStructBegin(__TokMembers);
			while (!__pdr.isEndOfStruct())
			{
				TCharacterId	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CGuildMemberPD*	obj;
				obj = NULL;
				if (__pdr.peekNextToken() == __Tok_ClassName)
				{
					std::string	__className;
					__pdr.pop(__Tok_ClassName, __className);
					obj = CGuildMemberPD::cast(PDSLib.create(__className));
					if (obj != NULL)
					{
						__pdr.popStructBegin(__TokMembers);
						obj->apply(__pdr);
						obj->pds__setParentUnnotified(this);
						__pdr.popStructEnd(__TokMembers);
					}
					else
					{
						__pdr.skipStruct();
					}
				}
				if (obj !=  NULL)
				{
					_Members[key] = obj;
				}
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokMembers);
		}
		// end of apply Members
		// apply FameContainer
		else if (__pdr.peekNextToken() == __TokFameContainer)
		{
			__pdr.popStructBegin(__TokFameContainer);
			_FameContainer = NULL;
			if (__pdr.peekNextToken() == __Tok_ClassName)
			{
				std::string	__className;
				__pdr.pop(__Tok_ClassName, __className);
				_FameContainer = CGuildFameContainerPD::cast(PDSLib.create(__className));
				if (_FameContainer != NULL)
				{
					__pdr.popStructBegin(__TokFameContainer);
					_FameContainer->apply(__pdr);
					_FameContainer->pds__setParentUnnotified(this);
					__pdr.popStructEnd(__TokFameContainer);
				}
				else
				{
					__pdr.skipStruct();
				}
			}
			__pdr.popStructEnd(__TokFameContainer);
		}
		else
		{
// The following warning was removed because the pdr object for the guild contains both the stuff managed here and the stuff managed in guild_manager/guild.cpp
// - the latter is ignored by the code here. This is normal behaviour and doesn't deserve a warning
//			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CGuildPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokId = __pdr.addString("Id");
	uint16	__TokMoney = __pdr.addString("Money");
	uint16	__TokCreationDate = __pdr.addString("CreationDate");
	uint16	__TokRace = __pdr.addString("Race");
	uint16	__TokIcon = __pdr.addString("Icon");
	uint16	__TokBuilding = __pdr.addString("Building");
	uint16	__TokVersion = __pdr.addString("Version");
	uint16	__TokMembers = __pdr.addString("Members");
	uint16	__TokFameContainer = __pdr.addString("FameContainer");
	__pdr.push(__TokId, _Id);
	__pdr.push(__TokMoney, _Money);
	__pdr.push(__TokCreationDate, _CreationDate);
	{
		std::string	valuename = CPeople::toString(_Race);
		__pdr.push(__TokRace, valuename);
	}
	__pdr.push(__TokIcon, _Icon);
	__pdr.push(__TokBuilding, _Building);
	__pdr.push(__TokVersion, _Version);
	// store Members
	__pdr.pushStructBegin(__TokMembers);
	for (std::map<TCharacterId, CGuildMemberPD*>::const_iterator it=_Members.begin(); it!=_Members.end(); ++it)
	{
		TCharacterId	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		if ((*it).second != NULL)
		{
			std::string	__className = PDSLib.getClassName((*it).second);
			__pdr.push(__Tok_ClassName, __className);
			__pdr.pushStructBegin(__TokMembers);
			(*it).second->store(__pdr);
			__pdr.pushStructEnd(__TokMembers);
		}
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokMembers);
	// end of store Members
	// store FameContainer
	__pdr.pushStructBegin(__TokFameContainer);
	if (_FameContainer != NULL)
	{
		std::string	__className = PDSLib.getClassName(_FameContainer);
		__pdr.push(__Tok_ClassName, __className);
		__pdr.pushStructBegin(__TokFameContainer);
		_FameContainer->store(__pdr);
		__pdr.pushStructEnd(__TokFameContainer);
	}
	__pdr.pushStructEnd(__TokFameContainer);
}
void							CGuildPD::init()
{
}
void							CGuildPD::release()
{
}
void							CGuildPD::pds__init(const TGuildId &Id)
{
	_Id = Id;
	_Money = (uint64)0;
	_CreationDate = 0;
	_Race = (CPeople::TPeople)0;
	_Icon = (uint64)0;
	_Building = 0;
	_Version = 0;
	_FameContainer = NULL;
	_Parent = NULL;
}
void							CGuildPD::pds__destroy()
{
	for (std::map<TCharacterId, CGuildMemberPD*>::iterator __it=_Members.begin(); __it!=_Members.end(); )
	{
		std::map<TCharacterId, CGuildMemberPD*>::iterator __itr=__it++;
		CGuildMemberPD*	__o = ((*__itr).second);
		if (__o != NULL)
		{
			__o->pds__destroy();
			delete __o;
		}
	}
	_Members.clear();
	if (_FameContainer != NULL)
	{
		CGuildFameContainerPD*	__o = _FameContainer;
		__o->pds__destroy();
		delete __o;
	}
}
void							CGuildPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Id);
	data.serial(_Money);
	data.serial(_CreationDate);
	data.serialEnum(_Race);
	data.serial(_Icon);
	data.serial(_Building);
	data.serial(_Version);
	RY_PDS::TTableIndex	tableIndex;
	RY_PDS::TRowIndex	rowIndex;
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		TCharacterId	__k;
		data.serial(__k);
		CGuildMemberPD*	__o = static_cast<CGuildMemberPD*>(PDSLib.create(tableIndex));
		_Members.insert(std::make_pair<TCharacterId,CGuildMemberPD*>(__k, __o));
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	// read table and row, create an object, affect to the ref, and fetch it
	_FameContainer = NULL;
	data.serial(tableIndex, rowIndex);
	if (rowIndex != RY_PDS::INVALID_ROW_INDEX && tableIndex != RY_PDS::INVALID_TABLE_INDEX)
	{
		_FameContainer = static_cast<CGuildFameContainerPD*>(PDSLib.create(tableIndex));
		PDSLib.setRowIndex(rowIndex, _FameContainer);
		_FameContainer->pds__fetch(data);
		_FameContainer->pds__setParentUnnotified(this);
	}
	_Parent = NULL;
}
void							CGuildPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(4, __BaseRow, 0);
	pds__registerAttributes();
}
void							CGuildPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildPD: registerAttributes %u:%u", 4, __BaseRow);
	PDSLib.set(4, __BaseRow, (RY_PDS::TColumnIndex)(0), _Id);
}
void							CGuildPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(4, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CGuildPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildPD: unregisterAttributes %u:%u", 4, __BaseRow);
	pds__setParent(NULL);
	for (std::map<TCharacterId, CGuildMemberPD*>::iterator __it=_Members.begin(); __it!=_Members.end(); )
	{
		std::map<TCharacterId, CGuildMemberPD*>::iterator __itr=__it++;
		CGuildMemberPD*	__o = (*__itr).second;
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
	if (_FameContainer != NULL)
	{
		CGuildFameContainerPD*	__o = _FameContainer;
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
}
void							CGuildPD::pds__setParent(CGuildContainerPD* __parent)
{
	if (_Parent != NULL)
	{
		_Parent->pds__unlinkGuilds(_Id);
	}
	_Parent = __parent;
	PDSLib.setParent(4, getRow(), (RY_PDS::TColumnIndex)(9), (__parent != NULL ? RY_PDS::CObjectIndex(5, __parent->getRow()) : RY_PDS::CObjectIndex::null()));
}
void							CGuildPD::pds__setParentUnnotified(CGuildContainerPD* __parent)
{
	_Parent = __parent;
}
void							CGuildPD::pds__notifyInit()
{
	init();
	for (std::map<TCharacterId, CGuildMemberPD*>::iterator __it=_Members.begin(); __it!=_Members.end(); )
	{
		std::map<TCharacterId, CGuildMemberPD*>::iterator __itr=__it++;
		(*__itr).second->pds__notifyInit();
	}
	if (_FameContainer != NULL)
	{
		_FameContainer->pds__notifyInit();
	}
}
void							CGuildPD::pds__notifyRelease()
{
	release();
	PDSLib.release(4, __BaseRow);
	for (std::map<TCharacterId, CGuildMemberPD*>::iterator __it=_Members.begin(); __it!=_Members.end(); )
	{
		std::map<TCharacterId, CGuildMemberPD*>::iterator __itr=__it++;
		(*__itr).second->pds__notifyRelease();
	}
	if (_FameContainer != NULL)
	{
		_FameContainer->pds__notifyRelease();
	}
}
void							CGuildPD::pds__unlinkMembers(TCharacterId __k)
{
	_Members.erase(__k);
}
void							CGuildPD::pds__unlinkFameContainer(NLMISC::CEntityId dummy)
{
	{
		_FameContainer = NULL;
	}
}
void							CGuildPD::pds_static__init()
{
	PDSLib.setIndexAllocator(4, _IndexAllocator);
	nlassertex(_FactoryInitialised, ("User Factory for class CGuildPD not set!"));
	// factory must have been set by user before database init called!
	// You must provide a factory for the class CGuildPD as it is marked as derived
	// Call EGSPD::CGuildPD::setFactory() with a factory before any call to EGSPD::init()!
}
void							CGuildPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(4, userFactory, pds_static__fetch, NULL);
		_FactoryInitialised = true;
	}
}
bool							CGuildPD::_FactoryInitialised;
RY_PDS::CIndexAllocator			CGuildPD::_IndexAllocator;
void							CGuildPD::pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data)
{
	CGuildPD	*__o = static_cast<CGuildPD*>(object);
	__o->pds__fetch(data);
	__o->pds__notifyInit();
}
// End of static implementation of CGuildPD

	
} // End of EGSPD
