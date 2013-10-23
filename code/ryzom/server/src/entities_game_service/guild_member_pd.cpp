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
* Static Implementation of CGuildMemberPD
* ----------------------------------------- */
TCharacterId					CGuildMemberPD::getId() const
{
	return _Id;
}
CGuildGrade::TGuildGrade		CGuildMemberPD::getGrade() const
{
	return _Grade;
}
void							CGuildMemberPD::setGrade(CGuildGrade::TGuildGrade __v, bool forceWrite)
{
	nlassert(__v<CGuildGrade::___TGuildGrade_useSize);
	if (_Grade != __v)
	{
		// callback the manager 
		IGuildManager::getInstance().guildMemberChanged(this);
	}
	if ((_Grade != __v) || forceWrite)
	{
		PDSLib.set(3, __BaseRow, (RY_PDS::TColumnIndex)(1), (uint32)__v, _Id);
	}
	_Grade = __v;
}
uint32							CGuildMemberPD::getEnterTime() const
{
	return _EnterTime;
}
void							CGuildMemberPD::setEnterTime(uint32 __v, bool forceWrite)
{
	if ((_EnterTime != __v) || forceWrite)
	{
		PDSLib.set(3, __BaseRow, (RY_PDS::TColumnIndex)(2), __v, _Id);
	}
	_EnterTime = __v;
}
CGuildPD*						CGuildMemberPD::getGuild()
{
	return _Guild;
}
const CGuildPD*					CGuildMemberPD::getGuild() const
{
	return _Guild;
}
void							CGuildMemberPD::clear()
{
	_Grade = (CGuildGrade::TGuildGrade)0;
	PDSLib.set(3, __BaseRow, (RY_PDS::TColumnIndex)(1), (uint32)(CGuildGrade::TGuildGrade)0);
	_EnterTime = 0;
	PDSLib.set(3, __BaseRow, (RY_PDS::TColumnIndex)(2), 0);
}
CGuildMemberPD*					CGuildMemberPD::cast(RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 3) ? static_cast<CGuildMemberPD*>(obj) : NULL;
}
const CGuildMemberPD*			CGuildMemberPD::cast(const RY_PDS::IPDBaseData* obj)
{
	return (obj->getTable() == 3) ? static_cast<const CGuildMemberPD*>(obj) : NULL;
}
void							CGuildMemberPD::setFactory(RY_PDS::TPDFactory userFactory)
{
	pds_static__setFactory(userFactory);
}
CGuildMemberPD*					CGuildMemberPD::create(const TCharacterId &Id)
{
	CGuildMemberPD	*__o = static_cast<CGuildMemberPD*>(PDSLib.create(3));
	__o->pds__init(Id);
	__o->pds__register();
	__o->pds__notifyInit();
	return __o;
}
void							CGuildMemberPD::apply(CPersistentDataRecord &__pdr)
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokId = __pdr.addString("Id");
	uint16	__TokGrade = __pdr.addString("Grade");
	uint16	__TokEnterTime = __pdr.addString("EnterTime");
	_Guild = NULL;
	while (!__pdr.isEndOfStruct())
	{
		if (false) {}
		else if (__pdr.peekNextToken() == __TokId)
		{
			__pdr.pop(__TokId, _Id);
		}
		else if (__pdr.peekNextToken() == __TokGrade)
		{
			{
				std::string	valuename;
				__pdr.pop(__TokGrade, valuename);
				_Grade = CGuildGrade::fromString(valuename);
			}
		}
		else if (__pdr.peekNextToken() == __TokEnterTime)
		{
			__pdr.pop(__TokEnterTime, _EnterTime);
		}
		else
		{
			nlwarning("Skipping unrecognised token: %s", __pdr.peekNextTokenName().c_str());
			__pdr.skipData();
		}
	}
}
void							CGuildMemberPD::store(CPersistentDataRecord &__pdr) const
{
	uint16	__Tok_MapKey = __pdr.addString("__Key__");
	uint16	__Tok_MapVal = __pdr.addString("__Val__");
	uint16	__Tok_ClassName = __pdr.addString("__Class__");
	uint16	__TokId = __pdr.addString("Id");
	uint16	__TokGrade = __pdr.addString("Grade");
	uint16	__TokEnterTime = __pdr.addString("EnterTime");
	__pdr.push(__TokId, _Id);
	{
		std::string	valuename = CGuildGrade::toString(_Grade);
		__pdr.push(__TokGrade, valuename);
	}
	__pdr.push(__TokEnterTime, _EnterTime);
}
void							CGuildMemberPD::init()
{
}
void							CGuildMemberPD::release()
{
}
void							CGuildMemberPD::pds__init(const TCharacterId &Id)
{
	_Id = Id;
	_Grade = (CGuildGrade::TGuildGrade)0;
	_EnterTime = 0;
	_Guild = NULL;
}
void							CGuildMemberPD::pds__destroy()
{
}
void							CGuildMemberPD::pds__fetch(RY_PDS::CPData &data)
{
	data.serial(_Id);
	data.serialEnum(_Grade);
	data.serial(_EnterTime);
	_Guild = NULL;
}
void							CGuildMemberPD::pds__register()
{
	__BaseRow = _IndexAllocator.allocate();
	PDSLib.allocateRow(3, __BaseRow, 0, _Id);
	pds__registerAttributes();
}
void							CGuildMemberPD::pds__registerAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildMemberPD: registerAttributes %u:%u", 3, __BaseRow);
	PDSLib.set(3, __BaseRow, (RY_PDS::TColumnIndex)(0), _Id);
}
void							CGuildMemberPD::pds__unregister()
{
	pds__unregisterAttributes();
	PDSLib.deallocateRow(3, __BaseRow, _Id);
	_IndexAllocator.deallocate(__BaseRow);
}
void							CGuildMemberPD::pds__unregisterAttributes()
{
	if (RY_PDS::PDVerbose)	nldebug("CGuildMemberPD: unregisterAttributes %u:%u", 3, __BaseRow);
	pds__setParent(NULL);
}
void							CGuildMemberPD::pds__setParent(CGuildPD* __parent)
{
	if (_Guild != NULL)
	{
		_Guild->pds__unlinkMembers(_Id);
	}
	_Guild = __parent;
	PDSLib.setParent(3, getRow(), (RY_PDS::TColumnIndex)(3), (__parent != NULL ? RY_PDS::CObjectIndex(4, __parent->getRow()) : RY_PDS::CObjectIndex::null()), _Id);
}
void							CGuildMemberPD::pds__setParentUnnotified(CGuildPD* __parent)
{
	_Guild = __parent;
}
void							CGuildMemberPD::pds__notifyInit()
{
	init();
}
void							CGuildMemberPD::pds__notifyRelease()
{
	release();
	PDSLib.release(3, __BaseRow);
}
void							CGuildMemberPD::pds_static__init()
{
	PDSLib.setIndexAllocator(3, _IndexAllocator);
	nlassertex(_FactoryInitialised, ("User Factory for class CGuildMemberPD not set!"));
	// factory must have been set by user before database init called!
	// You must provide a factory for the class CGuildMemberPD as it is marked as derived
	// Call EGSPD::CGuildMemberPD::setFactory() with a factory before any call to EGSPD::init()!
}
void							CGuildMemberPD::pds_static__setFactory(RY_PDS::TPDFactory userFactory)
{
	if (!_FactoryInitialised)
	{
		PDSLib.registerClass(3, userFactory, pds_static__fetch, NULL);
		_FactoryInitialised = true;
	}
}
bool							CGuildMemberPD::_FactoryInitialised;
RY_PDS::CIndexAllocator			CGuildMemberPD::_IndexAllocator;
void							CGuildMemberPD::pds_static__fetch(RY_PDS::IPDBaseData *object, RY_PDS::CPData &data)
{
	CGuildMemberPD	*__o = static_cast<CGuildMemberPD*>(object);
	__o->pds__fetch(data);
	__o->pds__notifyInit();
}
// End of static implementation of CGuildMemberPD

	
} // End of EGSPD
