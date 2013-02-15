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

//////////////////////////////////////////////////////////////////////////////
// includes
//////////////////////////////////////////////////////////////////////////////

#include "stdpch.h"
#include "outpost_squad.h"
#include "outpost.h"
#include "outpost_manager.h"
#include "guild_manager/guild.h"
#include "egs_sheets/egs_sheets.h"
#include "primitives_parser.h"


//////////////////////////////////////////////////////////////////////////////
// namespaces
//////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace NLMISC;
using namespace NLNET;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


//////////////////////////////////////////////////////////////////////////////
// config vars
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// methods COutpostSpawnZone
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
COutpostSpawnZone::COutpostSpawnZone()
: _Alias(0), _Center(CVector::Null), _Radius(0.f)
{
}

//----------------------------------------------------------------------------
COutpostSpawnZone::COutpostSpawnZone(TAIAlias alias, const NLMISC::CVector & center, float radius)
: _Alias(alias), _Center(center), _Radius(radius)
{
}

//----------------------------------------------------------------------------
std::string COutpostSpawnZone::toString() const
{
	string desc;
	if (_Alias == 0)
	{
		desc = "uninitialized!";
	}
	else
	{
		desc = NLMISC::toString("Alias: %s, Center: (%d,%d), Radius: %g",
			CPrimitivesParser::aliasToString(_Alias).c_str(),
			sint32(_Center.x),
			sint32(_Center.y),
			_Radius
			);
	}
	return desc;
}

//////////////////////////////////////////////////////////////////////////////
// methods COutpostSquadDescriptor
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
COutpostSquadDescriptor::COutpostSquadDescriptor()
: _Sheet(NLMISC::CSheetId::Unknown)
, _Alias(0)
, _Form(NULL)
{
}

//----------------------------------------------------------------------------
COutpostSquadDescriptor::COutpostSquadDescriptor(NLMISC::CSheetId sheet, TAIAlias alias, const std::string& name)
{
	init(sheet, alias, name);
}

//----------------------------------------------------------------------------
void COutpostSquadDescriptor::init(NLMISC::CSheetId sheet, TAIAlias alias, const std::string& name)
{
	_Sheet = sheet;
	_Alias = alias;
	_Form = CSheets::getOutpostSquadForm(_Sheet);
	nlassertex(_Form, ("Squad sheet %s not found", _Sheet.toString().c_str()));
//	if (!(_Form))
// {
//		NLMISC::createDebug ();
//		NLMISC::AssertLog->setPosition (__LINE__, __FILE__, __FUNCTION__);
//		NLMISC::AssertLog->displayRawNL ("Squad sheet %s not found", _Sheet.toString().c_str());		// BUG with unsetPosition() called twice
//		NLMISC_BREAKPOINT;
//	}
}

//----------------------------------------------------------------------------
void COutpostSquadDescriptor::preStore() const
{
	CStaticOutpostSquad const* form = CSheets::getOutpostSquadForm(_Sheet);
	nlassertex(form, ("Squad sheet %s not found", _Sheet.toString().c_str()));
}

//----------------------------------------------------------------------------
void COutpostSquadDescriptor::postLoad()
{
	_Form = CSheets::getOutpostSquadForm(_Sheet);
	nlassertex(_Form, ("Squad sheet %s not found", _Sheet.toString().c_str()));
//	if (!_Form)
//		OUTPOST_WRN("Squad sheet %s not found", _Sheet.toString().c_str());
}

//----------------------------------------------------------------------------
std::string COutpostSquadDescriptor::toString() const
{
	string desc;
	if (_Form == NULL)
	{
		desc = "uninitialized!";
	}
	else
	{
		desc = NLMISC::toString("Sheet: '%s', Alias: %s, BuyPrice: %u",
			_Sheet.toString().c_str(),
			CPrimitivesParser::aliasToString(_Alias).c_str(),
			_Form->BuyPrice
			);
	}
	return desc;
}

//////////////////////////////////////////////////////////////////////////////
// static members COutpostSquad
//////////////////////////////////////////////////////////////////////////////

uint32 COutpostSquad::_LastCreateOrder = 0;
std::map<uint32, NLMISC::CRefPtr<COutpostSquad> > COutpostSquad::_CreateOrders;

//////////////////////////////////////////////////////////////////////////////
// methods COutpostSquad
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
COutpostSquad::COutpostSquad()
: _OwnerOutpostAlias(0)
, _Desc()
, _SpawnZone()
, _State(OUTPOSTENUMS::NotCreated)
, _CreateOrder(0)
, _GroupId(0)
, _Recreate(false)
{
	// Do not create since _Outpost is not valid yet
}

//----------------------------------------------------------------------------
COutpostSquad::COutpostSquad(TAIAlias outpost, const COutpostSquadDescriptor & desc, TAIAlias spawnZone, OUTPOSTENUMS::TPVPSide side)
: _OwnerOutpostAlias(outpost)
, _Desc(desc)
, _SpawnZone(spawnZone)
, _State(OUTPOSTENUMS::NotCreated)
, _CreateOrder(0)
, _GroupId(0)
, _Recreate(false)
, _Side(side)
{
	create();
}

//----------------------------------------------------------------------------
COutpostSquad::~COutpostSquad()
{
	destroy();
}

//----------------------------------------------------------------------------
// :KLUDGE: More than 2^32 simultaneous spawn orders would lead to an infinite loop below.
uint32 COutpostSquad::nextCreateOrder()
{
	uint32 order = _LastCreateOrder;
	while (_CreateOrders.find(order)!=_CreateOrders.end() || order==0)
		order = ++_LastCreateOrder;
	return order;
}

//----------------------------------------------------------------------------
NLMISC::CSmartPtr<COutpostSquad> COutpostSquad::getSquadFromCreateOrder(uint32 createOrder)
{
	map<uint32, NLMISC::CRefPtr<COutpostSquad> >::iterator it = _CreateOrders.find(createOrder);
	if (it != _CreateOrders.end())
		return (COutpostSquad *)it->second;

	return NULL;
}

//----------------------------------------------------------------------------
void COutpostSquad::create()
{
	OUTPOST_DBG("squad create asked");
	nlassert(_State==OUTPOSTENUMS::NotCreated);
	if (_CreateOrder==0)
	{
		COutpost *outpost = COutpostManager::getInstance().getOutpostFromAlias(_OwnerOutpostAlias);
		if ( outpost )
		{
			if (outpost->getAISId().get()!=0)
			{
				_CreateOrder = nextCreateOrder();
				_CreateOrders.insert(make_pair(_CreateOrder, this));
				
				COutpostCreateSquadMsg params;
				params.Outpost = _OwnerOutpostAlias;
				params.Group = _Desc.alias();
				params.Zone = _SpawnZone;
				params.CreateOrder = _CreateOrder;
				params.RespawnTimeS = 24*60*60; // respawn time is 24 hours because squads must not respawn
				params.Side = _Side;
				outpost->sendOutpostMessage("OUTPOST_CREATE_SQUAD", params);
				OUTPOST_DBG("A create order (%d) has been issued for this squad in outpost %s", _CreateOrder, CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
			}
			else
			{
				OUTPOST_DBG( "Outpost %s has no AIS Id", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str() );
			}
		}
		else
		{
			OUTPOST_WRN( "Outpost %s not found while creating squad", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str() );
		}
	}
	else
	{
		OUTPOST_WRN("A create order is already pending for this squad in outpost %s", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
	}
}

//----------------------------------------------------------------------------
void COutpostSquad::created(uint32 createOrder, uint32 groupId)
{
	OUTPOST_DBG("A creation confirmation (%d) has been received for this squad in outpost %s, group id is 0x%08x", _CreateOrder, CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str(), groupId);
	nlassert(_State==OUTPOSTENUMS::NotCreated);
	nlassert(_GroupId==0);
	nlassert(_CreateOrder==createOrder);
	_State = OUTPOSTENUMS::NotReady;
	_GroupId = groupId;
	_CreateOrders.erase(_CreateOrder);
	_CreateOrder = 0;

	// if squad was recreated, make an update to restore squads that were ready (training)
	if (_Recreate)
	{
		updateSquad(CTime::getSecondsSince1970());
		_Recreate = false;
	}
}

//----------------------------------------------------------------------------
void COutpostSquad::spawn(TAIAlias outpostAlias)
{
	OUTPOST_DBG("squad spawn asked");
	nlassert(_OwnerOutpostAlias==outpostAlias);
	nlassert(_State==OUTPOSTENUMS::NotSpawned);
	COutpostSpawnSquadMsg params;
	params.Outpost = _OwnerOutpostAlias;
	params.GroupId = _GroupId;
	COutpost* outpost = COutpostManager::getInstance().getOutpostFromAlias(_OwnerOutpostAlias);
	if (outpost)
	{
		outpost->sendOutpostMessage("OUTPOST_SPAWN_SQUAD", params);
		OUTPOST_DBG("A spawn order has been issued for this squad in outpost %s", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
		_State = OUTPOSTENUMS::Spawning;
	}
	else
	{
		OUTPOST_WRN("Outpost %s not found while spawning squad", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
	}
}

//----------------------------------------------------------------------------
void COutpostSquad::spawned()
{
	OUTPOST_DBG("A spawn confirmation has been received for this squad in outpost %s, group id is 0x%08x", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str(), _GroupId);
	nlassert(_State==OUTPOSTENUMS::Spawning);
	_State = OUTPOSTENUMS::Spawned;
}

//----------------------------------------------------------------------------
void COutpostSquad::despawn()
{
	OUTPOST_DBG("squad despawn asked");
	nlassert(_State==OUTPOSTENUMS::Spawned || _State==OUTPOSTENUMS::Spawning); // :NOTE: If we are spawning AIS will spawn, then despawn, EGS will handle spawned, despawned
	COutpostDespawnSquadMsg params;
	params.Outpost = _OwnerOutpostAlias;
	params.GroupId = _GroupId;
	COutpost* outpost = COutpostManager::getInstance().getOutpostFromAlias(_OwnerOutpostAlias);
	if (outpost)
	{
		outpost->sendOutpostMessage("OUTPOST_DESPAWN_SQUAD", params);
		OUTPOST_DBG("A despawn order has been issued for this squad in outpost %s", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
	}
	else
	{
		OUTPOST_WRN("Outpost %s not found while despawning squad", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
	}
}

//----------------------------------------------------------------------------
void COutpostSquad::despawned()
{
	OUTPOST_DBG("A despawn confirmation has been received for this squad in outpost %s, group id is 0x%08x", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str(), _GroupId);
	nlassert(_State==OUTPOSTENUMS::Spawned);
	_State = OUTPOSTENUMS::NotSpawned;
}

//----------------------------------------------------------------------------
void COutpostSquad::destroy()
{
	OUTPOST_DBG("squad destroy asked");
	if (_OwnerOutpostAlias!=0 && _GroupId!=0)
	{
		COutpostDeleteSquadMsg params;
		params.Outpost = _OwnerOutpostAlias;
		params.GroupId = _GroupId;
		COutpost* outpost = COutpostManager::getInstance().getOutpostFromAlias(_OwnerOutpostAlias);
		if (outpost)
		{
			outpost->sendOutpostMessage("OUTPOST_DELETE_SQUAD", params);
			OUTPOST_DBG("A destroy order has been issued for this squad in outpost %s", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
		}
		else
		{
			OUTPOST_WRN("Outpost %s not found while destroying squad", CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
		}
	}
}

//----------------------------------------------------------------------------
void COutpostSquad::recreate()
{
	_Recreate = true;

	destroy();
	_State = OUTPOSTENUMS::NotCreated;
	_CreateOrder = 0;
	_GroupId = 0;
	create();
}

//----------------------------------------------------------------------------
bool COutpostSquad::isSpawned()
{
	return _State==OUTPOSTENUMS::Spawned || _State==OUTPOSTENUMS::Spawning;
}

//----------------------------------------------------------------------------
bool COutpostSquad::isDead()
{
	return _State==OUTPOSTENUMS::Dead;
}

//----------------------------------------------------------------------------
bool COutpostSquad::isReady()
{
	return _State == OUTPOSTENUMS::NotSpawned;
}

//----------------------------------------------------------------------------
void COutpostSquad::died()
{
	OUTPOST_DBG("The squad with group id 0x%08x in outpost %s died", _GroupId, CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
	nlassert(_State!=OUTPOSTENUMS::Dead);
	nlassert(_GroupId!=0);
	_State = OUTPOSTENUMS::Dead;
}

//----------------------------------------------------------------------------
void COutpostSquad::leaderDied()
{
	OUTPOST_DBG("The leader of the squad with group id 0x%08x in outpost %s died", _GroupId, CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str());
	nlassert(_State!=OUTPOSTENUMS::Dead);
	nlassert(_GroupId!=0);
	//_State = OUTPOSTENUMS::Zombie;
}

//----------------------------------------------------------------------------
void COutpostSquad::AISDown()
{
	OUTPOST_DBG("AISDown for squad 0x%08x", _GroupId);
	switch (_State)
	{
	case OUTPOSTENUMS::NotCreated:
	case OUTPOSTENUMS::NotReady:
	case OUTPOSTENUMS::NotSpawned:
	case OUTPOSTENUMS::Spawning:
	case OUTPOSTENUMS::Spawned:
		_State = OUTPOSTENUMS::NotCreated;
		_CreateOrder = 0;
		_GroupId = 0;
		break;
	case OUTPOSTENUMS::Dead:
		break;
	default:
		nlerror("Undefined state in outpost squad");
	}
}

//----------------------------------------------------------------------------
void COutpostSquad::AISUp()
{
	OUTPOST_DBG("AISUp for squad 0x%08x", _GroupId);
	switch (_State)
	{
	case OUTPOSTENUMS::NotCreated:
		if (_CreateOrder==0)
			create();
		break;
	case OUTPOSTENUMS::NotReady:
	case OUTPOSTENUMS::NotSpawned:
	case OUTPOSTENUMS::Spawning:
	case OUTPOSTENUMS::Spawned:
	case OUTPOSTENUMS::Dead:
		break;
	default:
		nlerror("Undefined state in outpost squad");
	}
}

//----------------------------------------------------------------------------
bool COutpostSquad::updateSquad(uint32 currentTime)
{
	// Transitions
	switch (_State)
	{
	case OUTPOSTENUMS::NotCreated:
		OUTPOST_DBG("Squad 0x%08x: [NotCreated] after activation", _GroupId);
		break;
	case OUTPOSTENUMS::NotReady:
		_State = OUTPOSTENUMS::NotSpawned;
		OUTPOST_DBG("Squad 0x%08x: [NotReady] -> [NotSpawned]", _GroupId);
		break;
	case OUTPOSTENUMS::NotSpawned:
		spawn(_OwnerOutpostAlias);
		break;
	case OUTPOSTENUMS::Spawning:
	case OUTPOSTENUMS::Spawned:
	case OUTPOSTENUMS::Dead:
		break;
	default:
		nlerror("Undefined state in outpost squad");
	}
	// States
	{
		switch (_State)
		{
		case OUTPOSTENUMS::NotCreated:
		case OUTPOSTENUMS::NotReady:
		case OUTPOSTENUMS::NotSpawned:
		case OUTPOSTENUMS::Spawning:
		case OUTPOSTENUMS::Spawned:
		case OUTPOSTENUMS::Dead:
			break;
		default:
			nlerror("Undefined state in outpost squad");
		}
	}
	
	return true;
}

//----------------------------------------------------------------------------
bool COutpostSquad::setSpawnZone(TAIAlias spawnZone)
{
	if (_State == OUTPOSTENUMS::Spawned)
		return false;

	if (_SpawnZone == spawnZone)
		return true;

	_SpawnZone = spawnZone;
	if (_State == OUTPOSTENUMS::NotCreated)
		return true;

	// TODO: update the spawn zone in AIS without having to recreate the group?
	// recreate the group in AIS
	recreate();

	return true;
}

//----------------------------------------------------------------------------
std::string COutpostSquad::toString() const
{
	string desc;
	if (_OwnerOutpostAlias == 0)
	{
		desc = "uninitialized!";
	}
	else
	{
		desc = NLMISC::toString("OwnerOutpostAlias: %s, Desc: [%s], SpawnZone: %s, State: '%s', Side: '%s'",
			CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str(),
			_Desc.toString().c_str(),
			CPrimitivesParser::aliasToString(_SpawnZone).c_str(),
			OUTPOSTENUMS::toString(_State).c_str(),
			OUTPOSTENUMS::toString(_Side).c_str()
			);
	}
	return desc;
}

//////////////////////////////////////////////////////////////////////////////
// methods COutpostSquadData
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
COutpostSquadData::COutpostSquadData()
: _OwnerOutpostAlias(0)
, _Desc()
, _SpawnZone()
{
}

//----------------------------------------------------------------------------
COutpostSquadData::COutpostSquadData(TAIAlias outpost, const COutpostSquadDescriptor & desc, TAIAlias spawnZone)
: _OwnerOutpostAlias(outpost)
, _Desc(desc)
, _SpawnZone(spawnZone)
{
}

//----------------------------------------------------------------------------
std::string COutpostSquadData::toString() const
{
	string desc;
	if (_OwnerOutpostAlias == 0)
	{
		desc = "uninitialized!";
	}
	else
	{
		desc = NLMISC::toString("OwnerOutpostAlias: %s, Desc: [%s], SpawnZone: %s",
			CPrimitivesParser::aliasToString(_OwnerOutpostAlias).c_str(),
			_Desc.toString().c_str(),
			CPrimitivesParser::aliasToString(_SpawnZone).c_str()
			);
	}
	return desc;
}

//----------------------------------------------------------------------------

#define PERSISTENT_MACROS_AUTO_UNDEF

//////////////////////////////////////////////////////////////////////////////
// Persistent data for COutpostSquadDescriptor
//////////////////////////////////////////////////////////////////////////////

#define PERSISTENT_CLASS COutpostSquadDescriptor

#define PERSISTENT_PRE_APPLY\
	H_AUTO(COutpostSquadDescriptorApply);\

#define PERSISTENT_PRE_STORE\
	preStore();\
	
#define PERSISTENT_POST_APPLY\
	postLoad();\

#define PERSISTENT_DATA\
	PROP(CSheetId,_Sheet)\
	PROP(TAIAlias,_Alias)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//////////////////////////////////////////////////////////////////////////////
// Persistent data for COutpostSquadData
//////////////////////////////////////////////////////////////////////////////

#define PERSISTENT_CLASS COutpostSquadData

#define PERSISTENT_PRE_APPLY\
	H_AUTO(COutpostSquadDataApply);\

#define PERSISTENT_DATA\
	STRUCT2(_Desc,_Desc.store(pdr),_Desc.apply(pdr))\
	PROP(TAIAlias,_SpawnZone)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

