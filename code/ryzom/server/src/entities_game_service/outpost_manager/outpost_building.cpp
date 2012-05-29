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
#include "server_share/log_item_gen.h"
#include "outpost.h"
#include "outpost_building.h"
#include "outpost_manager.h"

#include "world_instances.h"
#include "game_item_manager/game_item_manager.h"

#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"

#include "egs_sheets/egs_sheets.h"

#include "primitives_parser.h"

//----------------------------------------------------------------------------

using namespace NLLIGO;
using namespace NLNET;
using namespace NLMISC;
using namespace std;

//----------------------------------------------------------------------------

CVariable<float>	OutpostDrillerTimeUnit("egs", "OutpostDrillerTimeUnit", "Production time of mp in the driller (in seconds)", 3600.0f, 0, true );
CVariable<bool>		EnableOutpostDrillerMPGeneration("egs", "EnableOutpostDrillerMPGeneration", "Set if the outpost drillers generate mps or not", true, 0, true );
extern NLMISC::CRandom RandomGenerator;

//----------------------------------------------------------------------------
COutpostBuilding::COutpostBuilding(COutpost *parent, TAIAlias alias, const CVector &center, NLMISC::CSheetId defaultSheet)
{
	_Parent = parent;
	_Constructing = false;
	_Alias = alias;
	_Center = center;
	_DefaultSheet = defaultSheet;
	_CurrentSheet = CSheetId::Unknown;
	_StaticData = NULL;
	_BuiltDate = 0;
	_MPLeft = 0.0f;
	_MPLastTime = ~0;

	construct(defaultSheet);
}

//----------------------------------------------------------------------------
bool COutpostBuilding::construct(NLMISC::CSheetId sheetid)
{
	if (_CurrentSheet == sheetid)
		return false;

	if (_StaticData != NULL)
	{
		if ( ! _StaticData->CanBeDestroyedOrBuilt)
			return false;
	}

	_CurrentSheet = sheetid;
	_StaticData = CSheets::getOutpostBuildingForm(_CurrentSheet);
	if (!_StaticData)
		return false;
	
	_BuiltDate = CTime::getSecondsSince1970();

	_Constructing = true;
	if (_BotObject != NULL) // If bot object present disable player interaction on it
		_BotObject->setOutpostBuilding(NULL);

	if (_StaticData->ShapeWhenConstructing != CSheetId::Unknown)
		changeShape(_StaticData->ShapeWhenConstructing, _StaticData->NameWhenConstructing);

	// Specific init
	switch (_StaticData->Type)
	{
		case CStaticOutpostBuilding::TypeDriller:
		{
			_MPLeft = 0.0f;
			_MPLastTime = ~0;
		}
		break;

		default:
		break;
	}

	return true;
}

//----------------------------------------------------------------------------
bool COutpostBuilding::destroy()
{
	return construct(CSheetId("empty.outpost_building"));
}

//----------------------------------------------------------------------------
void COutpostBuilding::update(uint32 nCurrentTime)
{
	if (_StaticData == NULL)
		return;
	if (_Parent == NULL)
		return;
	if (!_Parent->isAISUp())
		return;

	// Check if the building is constructed
	if (_Constructing)
	{
		if (nCurrentTime > (_BuiltDate + _StaticData->CostTime))
		{
			_Constructing = false;

			// BotObject management (spawn + update)
			changeShape(_StaticData->Shape, _StaticData->Name);

			_MPLastTime = nCurrentTime;
		}
		else
		{
			if ((_StaticData->CostTime > 0) && (_BotObject != NULL))
			{
				uint cur = (uint)(127.0f * (nCurrentTime - _BuiltDate) / ((float)_StaticData->CostTime));
				uint max = 127;

				if (cur == 0) cur = 1;

				_BotObject->getPhysScores()._PhysicalScores[SCORES::hit_points].Current.directAccessForStructMembers() = cur;
				_BotObject->getPhysScores()._PhysicalScores[SCORES::hit_points].Max.directAccessForStructMembers() = max;
			}
		}
		return;
	}

	// Do the specific management for each type of building
	switch (_StaticData->Type)
	{
		case CStaticOutpostBuilding::TypeDriller:
		{
			if (!EnableOutpostDrillerMPGeneration.get())
				break;
			if (!_Parent->isBelongingToAGuild()) 
				break;
			if (nCurrentTime <= _MPLastTime)
				break;

			uint32 elapsedTime = nCurrentTime - _MPLastTime;
			_MPLastTime = nCurrentTime;

			// check that the elapsed time does not exceed 1 week to limit abnormal values
			static const uint32 maxTime = 60*60*24*7; // 1 week in seconds
			if (elapsedTime > maxTime)
				elapsedTime = maxTime;

			// speed of production in mp / second
			float fSpeed = _StaticData->Driller.TotalMP / OutpostDrillerTimeUnit.get();
			_MPLeft += elapsedTime * fSpeed;
			while (_MPLeft > 1.0f)
			{
				// Produce 1 MP
				uint i, j;
				float pos = RandomGenerator.frand(_StaticData->Driller.TotalMP);
				float curpos = 0.0f;

				for (i = 0; i < _StaticData->Driller.MPQuantities.size(); ++i)
				{
					for (j = 0; j < DRILLER_NB_LEVEL; ++j)
					{
						curpos += _StaticData->Driller.QualityFactor[j] * _StaticData->Driller.MPQuantities[i];
						if (pos <= curpos)
							break;
					}
					if (pos <= curpos)
						break;
				}

				nlassert(_StaticData->Driller.MPs.size() == _StaticData->Driller.MPQuantities.size());

				if ((i < _StaticData->Driller.MPQuantities.size()) && (j < DRILLER_NB_LEVEL))
				{
					TLogContext_Item_OutpostDriller outpostContext(CEntityId::Unknown);
					// 1 mp of type i and quality j is generated
					EGSPD::TGuildId gid = _Parent->getOwnerGuild();
					CGuild *pGuild = CGuildManager::getInstance()->getGuildFromId(gid);
					if (pGuild != NULL)
					{
						CGameItemPtr item;
						item = GameItemManager.createItem(	_StaticData->Driller.MPs[i], 
															(j+1)*(250/DRILLER_NB_LEVEL), 
															true, false);
						if (item != NULL)
							pGuild->putItem(item);
					}
				}

				_MPLeft -= 1.0f;
			}
		}
		break;
		default:
		break;
	}
}

//----------------------------------------------------------------------------
void COutpostBuilding::onAISUp()
{
	// We have to respawn the buildings
	_Constructing = true;
}

//----------------------------------------------------------------------------
void COutpostBuilding::onSpawned(CCreature *pBuilding)
{
	nlassert(pBuilding != NULL);

	pBuilding->setGodMode(true);
	_BotObject = pBuilding;

	if (_StaticData == NULL) return;

	if (_Constructing)
	{
		// The object spawned is the object used during construction
		_BotObject->setOutpostBuilding(NULL);
	}
	else
	{
		// assign buildings that can be built on top of this building
		// and assign the bot chat program (build, upgrade, destroy)
		if (_StaticData->Upgrade.size() > 0)
			_BotObject->setOutpostBuilding(this);
		else
			_BotObject->setOutpostBuilding(NULL);
	}

	// update the client database
	COutpostManager::getInstance().askOutpostGuildDBUpdate(_Parent->getAlias(), COutpostGuildDBUpdater::BUILDINGS);
}

//----------------------------------------------------------------------------
std::string COutpostBuilding::toString() const
{
	string desc = "no static data";
	if (_StaticData == NULL)
		return desc;

	desc = CStaticOutpostBuilding::toString(_StaticData->Type) + " (";
	desc += _CurrentSheet.toString() + ") ";
	if (_Constructing)
		desc += "constructing ";

	return desc;
}

//----------------------------------------------------------------------------
void COutpostBuilding::setConstructionTime(uint32 nNbSeconds, uint32 nCurrentTime)
{
	if (!_Constructing)	return;

	if (_StaticData == NULL) return;

	_BuiltDate = nCurrentTime - _StaticData->CostTime + nNbSeconds;
}

//----------------------------------------------------------------------------
void COutpostBuilding::changeShape(const CSheetId &botId,const string& name)
{
	CMessage msgout("OUTPOST_SET_BUILDING_BOT_SHEET");
	uint32 messageVersion = 1;
	bool bAutoSpawnDespawn = true;
	TAIAlias outpostAlias = _Parent->getAlias();
	TAIAlias botAlias = _Alias;
	string sLocalCustomName = name;

	msgout.serial(messageVersion);
	msgout.serial(outpostAlias);
	msgout.serial(botAlias);
	CSheetId localBotId = botId;
	msgout.serial(localBotId); // Send .creature sheet (this give the shape of the building)
	msgout.serial(bAutoSpawnDespawn);
	msgout.serial(sLocalCustomName);
	if (_BotObject == NULL)
	{
		CWorldInstances::instance().msgToAIInstance2( _Parent->getAIInstanceNumber(), msgout);
		OUTPOST_INF("OUTPOST_SET_BUILDING_BOT_SHEET msg sent : ai instance %d, alias %s", _Parent->getAIInstanceNumber(), CPrimitivesParser::aliasToString( botAlias ).c_str());
	}
	else
	{
		CWorldInstances::instance().msgToAIInstance2( _BotObject->getInstanceNumber(), msgout);
		OUTPOST_INF("OUTPOST_SET_BUILDING_BOT_SHEET msg sent : ai instance %d, alias %s", _BotObject->getInstanceNumber(), CPrimitivesParser::aliasToString( botAlias ).c_str());
	}
}

//----------------------------------------------------------------------------
void COutpostBuilding::postLoad()
{
	_StaticData = CSheets::getOutpostBuildingForm(_CurrentSheet);
}


#define PERSISTENT_MACROS_AUTO_UNDEF

//----------------------------------------------------------------------------
// Persistent data for COutpost
//----------------------------------------------------------------------------

#define PERSISTENT_CLASS COutpostBuilding

#define PERSISTENT_PRE_APPLY\
	H_AUTO(COutpostBuildingApply);\
	/*preApply();*/\
	
#define PERSISTENT_POST_APPLY\
	postLoad();\
	
//#define PERSISTENT_POST_STORE\
//	postStore();

#define PERSISTENT_DATA\
	/*PROP2(VERSION, uint32, COutpostBuildingVersionAdapter::getInstance()->currentVersionNumber(), version = val)*/\
	\
	PROP(uint32, _BuiltDate)\
	PROP(bool, _Constructing)\
	PROP(CSheetId, _CurrentSheet)\
	PROP(float, _MPLeft)\
	PROP(uint32, _MPLastTime)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

