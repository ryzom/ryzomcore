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
* Static Implementation of CGuildContainerPD
* ----------------------------------------- */
TGuildId						CGuildContainerPD::getGMGuild() const
{
	return _GMGuild;
}
void							CGuildContainerPD::setGMGuild(TGuildId __v, bool forceWrite)
{
	if ((_GMGuild != __v) || forceWrite)
	{
		PDSLib.set(5, __BaseRow, (RY_PDS::TColumnIndex)(0), __v);
	}
	_GMGuild = __v;
}
CGuildPD*						CGuildContainerPD::getGuilds(const TGuildId& __k)
{
	std::map<TGuildId, CGuildPD*>::iterator _it = _Guilds.find(__k);
	return (_it==_Guilds.end() ? NULL : (*_it).second);
}
const CGuildPD*					CGuildContainerPD::getGuilds(const TGuildId& __k) const
{
	std::map<TGuildId, CGuildPD*>::const_iterator _it = _Guilds.find(__k);
	return (_it==_Guilds.end() ? NULL : (*_it).second);
}
std::map<TGuildId, CGuildPD*>::iterator	CGuildContainerPD::getGuildsBegin()
{
	return _Guilds.begin();
}
std::map<TGuildId, CGuildPD*>::iterator	CGuildContainerPD::getGuildsEnd()
{
	return _Guilds.end();
}
std::map<TGuildId, CGuildPD*>::const_iterator	CGuildContainerPD::getGuildsBegin() const
{
	return _Guilds.begin();
}
std::map<TGuildId, CGuildPD*>::const_iterator	CGuildContainerPD::getGuildsEnd() const
{
	return _Guilds.end();
}
const std::map<TGuildId, CGuildPD*> &	CGuildContainerPD::getGuilds() const
{
	return _Guilds;
}
void							CGuildContainerPD::setGuilds(CGuildPD* __v)
{
	if (__v == NULL)	return;
	TGuildId	__k = __v->getId();
	std::map<TGuildId, CGuildPD*>::iterator	_it = _Guilds.find(__k);
	if (_it != _Guilds.end())
	{
		CGuildPD*	__prev = (*_it).second;
		if (__prev == __v)	return;
		__prev->pds__setParent(NULL);
		__prev->pds__unregister();
		__prev->pds__destroy();
		delete __prev;
	}
	__v->pds__setParent(this);
	_Guilds[__k] = __v;
}
void							CGuildContainerPD::deleteFromGuilds(const TGuildId &__k)
{
	std::map<TGuildId, CGuildPD*>::iterator	__it = _Guilds.find(__k);
	if (__it == _Guilds.end())	return;
	CGuildPD*	__o = (*__it).second;
	__o->pds__unregister();
	__o->pds__destroy();
	delete __o;
}
uint8							CGuildContainerPD::getDummy() const
{
	return _Dummy;
}
void							CGuildContainerPD::clear()
{
	_GMGuild = (TGuildId)0;
	PDSLib.set(5, __BaseRow, (RY_PDS::TColumnIndex)(0), (TGuildId)0);
	for (std::map<TGuildId, CGuildPD*>::iterator __it=_Guilds.begin(); __it!=_Guilds.end(); )
	{
		std::map<TGuildId, CGuildPD*>::iterator __itr=__it++;
		CGuildPD*	__o = (*__itr).second;
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
	_Guilds.clear();
}
CGuildContainerPD*				CGuildContainerPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 5) ? static_cast<CGuildContainerPD*>(obj) : NULL;
}
const CGuildContainerPD*		CGuildContainerPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 5) ? static_cast<const CGuildContainerPD*>(obj) : NULL;
}
CGuildContainerPD*				CGuildContainerPD::create(const uint8 &Dummy)
{
	CGuildContainerPD	*__o = static_cast<CGuildContainerPD*>(pds_static__factory());
	__o->pds__init(Dummy);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CGuildContainerPD::remove(const uint8& Dummy)
{
	std::map<uint8,CGuildContainerPD*>::iterator	it = _Map.find(Dummy);
	if (it != _Map.end())
	{
		CGuildContainerPD*	__o = (*it).second;
		__o->pds__notifyRelease();
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
}
void							CGuildContainerPD::load(const uint8& Dummy)
{
	PDSLib.load(5, (uint64)Dummy);
}
void							CGuildContainerPD::setLoadCallback(void (*callback)(const uint8& key, CGuildContainerPD* object))
{
	__pds__LoadCallback = callback;
}
void							CGuildContainerPD::unload(const uint8 &Dummy)
{
	std::map<uint8,CGuildContainerPD*>::iterator	it = _Map.find(Dummy);
	if (it != _Map.end())
	{
		CGuildContainerPD*	__o = (*it).second;
		__o->pds__notifyRelease();
		__o->pds__destroy();
		delete __o;
	}
}
CGuildContainerPD*				CGuildContainerPD::get(const uint8 &Dummy)
{
	std::map<uint8, CGuildContainerPD*>::iterator	__it = _Map.find(Dummy);
	return (__it != _Map.end()) ? (*__it).second : NULL;
}
std::map<uint8, CGuildContainerPD*>::iterator	CGuildContainerPD::begin()
{
	return _Map.begin();
}
std::map<uint8, CGuildContainerPD*>::iterator	CGuildContainerPD::end()
{
	return _Map.end();
}
void							CGuildContainerPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokGMGuild = __pdr.addString("GMGuild");
	uint16	__TokGuilds = __pdr.addString("Guilds");
	uint16	__TokDummy = __pdr.addString("Dummy");
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokGMGuild)
		{
			__pdr.pop(__TokGMGuild, _GMGuild);
		}
		// apply Guilds
		else if (__pdr.peekNextToken() == __TokGuilds)
		{
			__pdr.popStructBegin(__TokGuilds);
			while (!__pdr.isEndOfStruct())
			{
				TGuildId	key;
				__pdr.pop(__Tok_MapKey, key);
				__pdr.popStructBegin(__Tok_MapVal);
				CGuildPD*	obj;
				obj = NULL;
				if (__pdr.peekNextToken() == __Tok_ClassName)
				{
					std::string	__className;
					__pdr.pop(__Tok_ClassName, __className);
					obj = CGuildPD::cast(PDSLib.create(__className));
					if (obj != NULL)
					{
						__pdr.popStructBegin(__TokGuilds);
						obj->apply(__pdr);
						obj->pds__setParentUnnotified(this);
						__pdr.popStructEnd(__TokGuilds);
					}
					else
					{
						__pdr.skipStruct();
					}
				}
				if (obj !=  NULL)
				{
					_Guilds[key] = obj;
				}
				__pdr.popStructEnd(__Tok_MapVal);
			}
			__pdr.popStructEnd(__TokGuilds);
		}
		// end of apply Guilds
		else if (__pdr.peekNextToken() == __TokDummy)
		{
			__pdr.pop(__TokDummy, _Dummy);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
	pds__notifyInit();
}
void							CGuildContainerPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokGMGuild = __pdr.addString("GMGuild");
	uint16	__TokGuilds = __pdr.addString("Guilds");
	uint16	__TokDummy = __pdr.addString("Dummy");
	__pdr.push(__TokGMGuild, _GMGuild);
	// store Guilds
	__pdr.pushStructBegin(__TokGuilds);
	for (std::map<TGuildId, CGuildPD*>::const_iterator it=_Guilds.begin(); it!=_Guilds.end(); ++it)
	{
		TGuildId	key = (*it).first;
		__pdr.push(__Tok_MapKey, key);
		__pdr.pushStructBegin(__Tok_MapVal);
		if ((*it).second != NULL)
		{
			std::string	__className = PDSLib.getClassName((*it).second);
			__pdr.push(__Tok_ClassName, __className);
			__pdr.pushStructBegin(__TokGuilds);
			(*it).second->store(__pdr);
			__pdr.pushStructEnd(__TokGuilds);
		}
		__pdr.pushStructEnd(__Tok_MapVal);
	}
	__pdr.pushStructEnd(__TokGuilds);
	// end of store Guilds
	__pdr.push(__TokDummy, _Dummy);
}
void							CGuildContainerPD::pds__init(const uint8 &Dummy)
{
	_GMGuild = (TGuildId)0;
	_Dummy = Dummy;
	_Map[getDummy()] = this;
}
void							CGuildContainerPD::pds__destroy()
{
	for (std::map<TGuildId, CGuildPD*>::iterator __it=_Guilds.begin(); __it!=_Guilds.end(); )
	{
		std::map<TGuildId, CGuildPD*>::iterator __itr=__it++;
		CGuildPD*	__o = ((*__itr).second);
		if (__o != NULL)
		{
			__o->pds__destroy();
			delete __o;
		}
	}
	_Guilds.clear();
	_Map.erase(getDummy());
}
void							CGuildContainerPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_GMGuild);
	RY_PDS::TTableIndex	tableIndex;
	RY_PDS::TRowIndex	rowIndex;
	do
	{
		// read table and row, create an object, affect to the ref, and fetch it
		data.serial(tableIndex, rowIndex);
		if (rowIndex == RY_PDS::INVALID_ROW_INDEX || tableIndex == RY_PDS::INVALID_TABLE_INDEX)	break;
		TGuildId	__k;
		data.serial(__k);
		CGuildPD*	__o = static_cast<CGuildPD*>(PDSLib.create(tableIndex));
		_Guilds.insert(std::make_pair<TGuildId,CGuildPD*>(__k, __o));
		PDSLib.setRowIndex(rowIndex, __o);
		__o->pds__fetch(data);
		__o->pds__setParentUnnotified(this);
	}
	while (true);
	data.serial(_Dummy);
	_Map[getDummy()] = this;
}
void							CGuildContainerPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(5, __BaseRow, (uint64)_Dummy);
	pds__registerAttributes();
}
void							CGuildContainerPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildContainerPD: registerAttributes %u:%u", 5, __BaseRow);
	PDSLib.set(5, __BaseRow, (RY_PDS::TColumnIndex)(2), _Dummy);
}
void							CGuildContainerPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(5, __BaseRow);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CGuildContainerPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildContainerPD: unregisterAttributes %u:%u", 5, __BaseRow);
	for (std::map<TGuildId, CGuildPD*>::iterator __it=_Guilds.begin(); __it!=_Guilds.end(); )
	{
		std::map<TGuildId, CGuildPD*>::iterator __itr=__it++;
		CGuildPD*	__o = (*__itr).second;
		__o->pds__unregister();
		__o->pds__destroy();
		delete __o;
	}
}
void							CGuildContainerPD::pds__notifyInit()
{
	for (std::map<TGuildId, CGuildPD*>::iterator __it=_Guilds.begin(); __it!=_Guilds.end(); )
	{
		std::map<TGuildId, CGuildPD*>::iterator __itr=__it++;
		(*__itr).second->pds__notifyInit();
	}
}
void							CGuildContainerPD::pds__notifyRelease()
{
	PDSLib.release(5, __BaseRow);
	for (std::map<TGuildId, CGuildPD*>::iterator __it=_Guilds.begin(); __it!=_Guilds.end(); )
	{
		std::map<TGuildId, CGuildPD*>::iterator __itr=__it++;
		(*__itr).second->pds__notifyRelease();
	}
}
void							CGuildContainerPD::pds__unlinkGuilds(TGuildId __k)
{
	_Guilds.erase(__k);
}
void							CGuildContainerPD::pds_static__init()
{
	PDSLib.setIndexAllocator(5, _IndexAllocator);
	pds_static__setFactory(pds_static__factory);
}
std::map<uint8,CGuildContainerPD*>	CGuildContainerPD::_Map;
void							CGuildContainerPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(5, userFactory, pds_static__fetch, pds_static__notifyFailure);
		_FactoryInitialised = true;
	}
}
bool							CGuildContainerPD::_FactoryInitialised;
void							CGuildContainerPD::pds_static__notifyFailure(uint64 key)
{
	if (__pds__LoadCallback != NULL)
	{
		__pds__LoadCallback((uint8)key, NULL);
	}
}
void							(*CGuildContainerPD::__pds__LoadCallback)(const uint8& key, CGuildContainerPD* object) = NULL;
RY_PDS::CIndexAllocator			CGuildContainerPD::_IndexAllocator;
RY_PDS::IPDBaseData*			CGuildContainerPD::pds_static__factory()
{
	return new CGuildContainerPD();
}
void							CGuildContainerPD::pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data)
{
	CGuildContainerPD	*__o = static_cast<CGuildContainerPD*>(object);
	__o->pds__fetch(data);
	if (__pds__LoadCallback != NULL)
	{
		__pds__LoadCallback(__o->getDummy(), __o);
	}
	__o->pds__notifyInit();
}
// End of static implementation of CGuildContainerPD

	
} // End of EGSPD
