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

#include <functional>

#include "child_container.h"
#include "ai_mgr.h"
#include "ai_entity_matrix.h"
#include "ai_player.h"
#include "ai_mgr_pet.h"
#include "ai_grp.h"
#include "ai_mgr_fauna.h"
#include "ai_mgr_npc.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"
#include "ai_grp_pet.h"

#include "ai_script_data_manager.h"

#include "ais_user_models.h"
#include "continent.h"
#include "client_message.h"
#include "ai_outpost.h"
// Georges
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_dfn.h"
// Game share
#include "game_share/emote_list_parser.h"
#include "game_share/backup_service_interface.h"

#include "ai_variables.h"
#include "server_share/r2_variables.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace NLGEORGES;

//--------------------------------------------------------------------------
// SINGLETON DATA
//--------------------------------------------------------------------------


CAIS		*CAIS::_Instance = NULL;

CRandom										CAIS::_random;

const	std::string	disengageString("DISENGAGE");
const	std::string	egsString("EGS");


const	uint32 Default_MaxPlayers=5000;
const	uint32 Default_MaxBotsPet=Default_MaxPlayers*4;
const	uint32 Default_MaxBotsFauna=40000;
const	uint32 Default_MaxBotsNpc=20000;
const	uint32 Default_MaxBotsFx=200;

CAIS &CAIS::instance()
{
	if (_Instance == NULL)
	{
		_Instance = new CAIS();
		// init the AI engine
		_Instance->initAI();
	}

	return *_Instance;
}


CAIS::CAIS()
:	_PetBotCounter(TotalMaxPet),
	_FaunaBotCounter(TotalMaxFauna),
	_NpcBotCounter(TotalMaxNpc)
{
//	_initialised=false;
	_TotalBotsSpawned = 0;
	_ClientCreatureDebug=false;
}

void setMaxPetCallBack(IVariable &var)
{
	uint32	newMax=NLMISC::safe_cast<CVariable<uint32>*>(&var)->get();
	if (CAIS::instanceCreated())
		CAIS::instance()._PetBotCounter.setMax(newMax);
}
void setMaxFaunaCallBack(IVariable &var)
{
	uint32	newMax=NLMISC::safe_cast<CVariable<uint32>*>(&var)->get();
	if (CAIS::instanceCreated())
		CAIS::instance()._FaunaBotCounter.setMax(newMax);
}
void setMaxNpcCallBack(IVariable &var)
{
	uint32	newMax=NLMISC::safe_cast<CVariable<uint32>*>(&var)->get();
	if (CAIS::instanceCreated())
		CAIS::instance()._NpcBotCounter.setMax(newMax);
}

CVariable<uint32>	TotalMaxPlayer("ai", "NbPlayersLimit", "Security absolute limit to the number of Player",	Default_MaxPlayers,		0, true	);
CVariable<uint32>	TotalMaxPet("ai", "NbPetLimit", "Security absolute limit to the number of Pets",			Default_MaxBotsPet,		0, true, setMaxPetCallBack		);
CVariable<uint32>	TotalMaxFauna("ai", "NbFaunaLimit", "Security absolute limit to the number of Faunas",		Default_MaxBotsFauna,	0, true, setMaxFaunaCallBack	);
CVariable<uint32>	TotalMaxNpc("ai", "NbNpcLimit", "Security absolute limit to the number of Npcs",			Default_MaxBotsNpc,		0, true, setMaxNpcCallBack		);
CVariable<uint32>	TotalMaxFx("ai", "NbFxLimit", "Security absolute limit to the number of Fx",				Default_MaxBotsFx,		0, true );

CVariable<string>	BotRepopFx("ai", "BotRepopFx", "Fx sheet to use when changing the sheet of a bot",			string(),				0, true );

//--------------------------------------------------------------------------
// DATA TABLES FOR ENTITY MATRIX
//--------------------------------------------------------------------------

// a series of tables giving the minimum iterator table forms for entity matrix iterators for all sizes up to 127m
static uint32 EntityMatrixTbl0[]   = { 3,  3,  3};
static uint32 EntityMatrixTbl16[]  = { 3,  5,  5,  5,  3};
static uint32 EntityMatrixTbl23[]  = { 5,  5,  5,  5,  5};
static uint32 EntityMatrixTbl32[]  = { 3,  5,  7,  7,  7,  5,  3};
static uint32 EntityMatrixTbl36[]  = { 5,  7,  7,  7,  7,  7,  5};
static uint32 EntityMatrixTbl46[]  = { 7,  7,  7,  7,  7,  7,  7};
static uint32 EntityMatrixTbl48[]  = { 3,  7,  7,  9,  9,  9,  7,  7,  3};
static uint32 EntityMatrixTbl51[]  = { 5,  7,  9,  9,  9,  9,  9,  7,  5};
static uint32 EntityMatrixTbl58[]  = { 7,  9,  9,  9,  9,  9,  9,  9,  7};
static uint32 EntityMatrixTbl64[]  = { 3,  7,  9,  9, 11, 11, 11,  9,  9,  7,  3};
static uint32 EntityMatrixTbl66[]  = { 5,  7,  9, 11, 11, 11, 11, 11,  9,  7,  5};
static uint32 EntityMatrixTbl68[]  = { 5,  9,  9, 11, 11, 11, 11, 11,  9,  9,  5};
static uint32 EntityMatrixTbl72[]  = { 7,  9, 11, 11, 11, 11, 11, 11, 11,  9,  7};
static uint32 EntityMatrixTbl80[]  = { 3,  9, 11, 11, 11, 13, 13, 13, 11, 11, 11,  9,  3};
static uint32 EntityMatrixTbl82[]  = { 5,  9, 11, 11, 13, 13, 13, 13, 13, 11, 11,  9,  5};
static uint32 EntityMatrixTbl87[]  = { 7,  9, 11, 13, 13, 13, 13, 13, 13, 13, 11,  9,  7};
static uint32 EntityMatrixTbl91[]  = { 7, 11, 11, 13, 13, 13, 13, 13, 13, 13, 11, 11,  7};
static uint32 EntityMatrixTbl94[]  = { 9, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 11,  9};
static uint32 EntityMatrixTbl96[]  = { 3,  9, 11, 13, 13, 13, 15, 15, 15, 13, 13, 13, 11,  9,  3};
static uint32 EntityMatrixTbl98[]  = { 5,  9, 11, 13, 13, 15, 15, 15, 15, 15, 13, 13, 11,  9,  5};
static uint32 EntityMatrixTbl102[] = { 7,  9, 11, 13, 15, 15, 15, 15, 15, 15, 15, 13, 11,  9,  7};
static uint32 EntityMatrixTbl103[] = { 7, 11, 13, 13, 15, 15, 15, 15, 15, 15, 15, 13, 13, 11,  7};
static uint32 EntityMatrixTbl108[] = { 9, 11, 13, 15, 15, 15, 15, 15, 15, 15, 15, 15, 13, 11,  9};
static uint32 EntityMatrixTbl112[] = { 3,  9, 11, 13, 15, 15, 15, 17, 17, 17, 15, 15, 15, 13, 11,  9,  3};
static uint32 EntityMatrixTbl114[] = { 5,  9, 13, 13, 15, 15, 17, 17, 17, 17, 17, 15, 15, 13, 13,  9,  5};
static uint32 EntityMatrixTbl116[] = { 5, 11, 13, 15, 15, 15, 17, 17, 17, 17, 17, 15, 15, 15, 13, 11,  5};
static uint32 EntityMatrixTbl117[] = { 7, 11, 13, 15, 15, 17, 17, 17, 17, 17, 17, 17, 15, 15, 13, 11,  7};
static uint32 EntityMatrixTbl122[] = { 9, 11, 13, 15, 17, 17, 17, 17, 17, 17, 17, 17, 17, 15, 13, 11,  9};
static uint32 EntityMatrixTbl125[] = { 9, 13, 15, 15, 17, 17, 17, 17, 17, 17, 17, 17, 17, 15, 15, 13,  9};

// a few larger special case matrices
static uint32 EntityMatrixTblUpTo150[] = { 7, 11, 15, 17, 17, 19, 19, 21, 21, 21, 21, 21, 21, 21, 19, 19, 17, 17, 15, 11, 7};
static uint32 EntityMatrixTblUpTo200[] = { 9, 13, 17, 19, 21, 23, 23, 25, 25, 27, 27, 27, 27, 27, 27, 27, 27, 27, 25, 25, 23, 23, 21, 19, 17, 13, 9};
static uint32 EntityMatrixTblUpTo250[] = {11, 15, 19, 23, 25, 27, 27, 29, 29, 31, 31, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 31, 31, 29, 29, 27, 27, 25, 23, 19, 15, 11};

static void initLinearMatrixIteratorTables(std::vector<CAIEntityMatrixIteratorTblLinear *> &vect)
{
	// initialise the vector with the first table
	vect.push_back(new CAIEntityMatrixIteratorTblLinear(&(EntityMatrixTbl0[0]),3));

	// local macro undefined at end of function
	#define ADD_TBL(d) \
	{				   \
		while (vect.size()<d) \
			vect.push_back(vect[vect.size()-1]); \
		vect.push_back(new CAIEntityMatrixIteratorTblLinear(EntityMatrixTbl##d,sizeof(EntityMatrixTbl##d)/sizeof(EntityMatrixTbl##d[0]))); \
	}

	// setup the tables ...
	ADD_TBL(16)	ADD_TBL(23)
	ADD_TBL(32)	ADD_TBL(36)	ADD_TBL(46)
	ADD_TBL(48)	ADD_TBL(51)	ADD_TBL(58)
	ADD_TBL(64)	ADD_TBL(66)	ADD_TBL(68)	ADD_TBL(72)
	ADD_TBL(80)	ADD_TBL(82)	ADD_TBL(87)	ADD_TBL(91)	ADD_TBL(94)
	ADD_TBL(96)	ADD_TBL(98)	ADD_TBL(102) ADD_TBL(103) ADD_TBL(108)
	ADD_TBL(112) ADD_TBL(114) ADD_TBL(116) ADD_TBL(117) ADD_TBL(122) ADD_TBL(125)

	#undef ADD_TBL
}

bool	CAIS::markTagForDelete(const std::string &filename)
{
	const	TStringId fileId = CStringMapper::map(filename);
	for (CCont<CAIInstance>::iterator	it=_AIInstances.begin(), itEnd=_AIInstances.end(); it!=itEnd;++it)
	{
		// first: tag the dynamic regions in the continents
		for_each(it->continents().begin(), it->continents().end(),
			bind2nd(mem_fun(&CContinent::markTagForDelete), fileId));

		for_each(it->managers().begin(),it->managers().end(),
			CAliasTreeRoot::CMarkTagForDelete(fileId));
	}
	return	true;
}

void CAIS::deleteTaggedAlias(const std::string &filename)
{
	const	TStringId fileId = CStringMapper::map(filename);
	FOREACH(it, CCont<CAIInstance>, _AIInstances)
	{
		// first: tag the dynamic regions in the continents
		for_each(it->continents().begin(), it->continents().end(),
			bind2nd(mem_fun(&CContinent::deleteTaggedAlias),fileId));

		for_each(it->managers().begin(),it->managers().end(),
			CAliasTreeRoot::CDeleteTagged<CManager>(it->managers()));
	}

}


uint32	CAIS::getEmotNumber(const std::string &name)
{
	std::map<std::string, uint32>::iterator it(_EmotNames.find(name));
	if	(it==_EmotNames.end())
		return	std::numeric_limits<uint32>::max();
	return it->second;
}



bool CAIS::advanceUserTimer	(uint32 nbTicks)
{
	// for each manager, look for a timer event
	for_each(AIList().begin(), AIList().end(), bind2nd(mem_fun(&CAIInstance::advanceUserTimer),nbTicks) );
	return	true;
}



// initialise the singleton
void	CAIS::initAI()
{
//	if (_initialised)
//		return;
//	_initialised=true;
	nlinfo("---------- Initialising AI Singleton ----------");

	// setup the random number generator
	_random.srand( (sint32)NLMISC::CTime::getLocalTime() );

	// allocate RAM for the players

	// setup the standard iterator tables for scanning the entity matrices
	_matrixIterator2x2.push_back(-1,-1); _matrixIterator2x2.push_back(1,0);
	_matrixIterator2x2.push_back(-1, 1); _matrixIterator2x2.push_back(1,0);

	_matrixIterator3x3.push_back(-1,-1); _matrixIterator3x3.push_back(1,0); _matrixIterator3x3.push_back(1,0);
	_matrixIterator3x3.push_back(-2, 1); _matrixIterator3x3.push_back(1,0); _matrixIterator3x3.push_back(1,0);
	_matrixIterator3x3.push_back(-2, 1); _matrixIterator3x3.push_back(1,0); _matrixIterator3x3.push_back(1,0);

	// setup the set of linear iterator tables for generating visions of given distances
	initLinearMatrixIteratorTables(_matrixIteratorsByDistance);

	EMOTE_LIST_PARSER::initEmoteList(_EmotNames);

	// init the client message callbacks
	CAIClientMessages::init();
}


uint32	CAIS::createAIInstance(const std::string &continentName, uint32 instanceNumber)
{
	// first, check that an instance with this number is not already running

	for (CCont<CAIInstance>::iterator	it=_AIInstances.begin(), itEnd=_AIInstances.end();it!=itEnd;++it)
	{
		if	(it->getInstanceNumber()!=instanceNumber)
			continue;

		nlwarning("CAIS::createAIInstance: instance number %u is already in use, can't create new instance.", instanceNumber);
		return	std::numeric_limits<uint32>::max();
	}

	CAIInstance	*aii = _AIInstances.addChild(new CAIInstance(this));

	// ok, set the continent name and instance number
	aii->initInstance(continentName, instanceNumber);

	return aii->getChildIndex();
}

void	CAIS::destroyAIInstance(uint32 instanceNumber, bool displayWarningIfInstanceNotExist)
{
	// this method is not fully tested for a Ryzom shard
	// but it should work as expected for a Ring shard
	nlassert(IsRingShard.get());

	CRefPtr<CAIInstance> aii = getAIInstance(instanceNumber);
	if (aii == NULL)
	{
		if (displayWarningIfInstanceNotExist)
		{
			nlwarning("AI instance %u does not exist but it was asked to delete here", instanceNumber);
		}
		return;
	}
	aii->despawn();
	_AIInstances.removeChildByIndex(aii->getChildIndex());
	nlassert(aii == NULL);

	// notify the EGS
	if (EGSHasMirrorReady)
	{
		CReportAIInstanceDespawnMsg msg;
		msg.InstanceNumbers.push_back(instanceNumber);
		msg.send("EGS");
	}
}


// release the singleton before program exit
void CAIS::release ()
{
	// force an update to save the persistent var if needed
	updatePersistentVariables();

	//	erase all ai instance.
	AIList().clear();

	CAIUserModelManager::getInstance()->destroyInstance();
	// release the client message callbacks
	CAIClientMessages::release();


	// free up the vision matrix iterator tables
	if	(!_matrixIteratorsByDistance.empty())
	{
		for	(uint i=0;i<_matrixIteratorsByDistance.size();)
		{
			// erase the iterator table
			delete _matrixIteratorsByDistance[i];
 			// run i forwards past repeated refs to the iterator tbl that we just deleted
			for (++i;i<_matrixIteratorsByDistance.size() && _matrixIteratorsByDistance[i]==_matrixIteratorsByDistance[i-1];++i) {}
		}
		_matrixIteratorsByDistance.clear();
	}
	_Instance = NULL;
	delete this;
}

void	CAIS::serviceEvent	(const	CServiceEvent	&info)
{
	if (info.getEventType() == CServiceEvent::SERVICE_UP && info.getServiceName() == "EGS")
	{
		// send the list of available collision data
		CReportAICollisionAvailableMsg msg;
		msg.ContinentsCollision = CWorldContainer::getContinentList();
		msg.send(info.getServiceId());
	}

	// event on all ai instance

//	for_each(_AIInstances.begin(), _AIInstances.end(), bind2nd(mem_fun1(&CAIInstance::serviceEvent), info));
//	don't compile coz we need to pass an object and not a reference (info). have to build an object that represents the reference.

	FOREACH(it, CCont<CAIInstance>, _AIInstances)
		it->serviceEvent	(info);
}


//--------------------------------------------------------------------------
// update() & save()
//--------------------------------------------------------------------------

// the update routine called once per tick
// this is the routine that calls the managers' updates
extern	void	execBufferedCommands();
extern	void	execNamedEntityChanges();

void	CAIS::update()
{
	if (!EGSHasMirrorReady)
		return;

	H_AUTO(AIUpdate);


	// Init stat counters
	AISStat::countersBegin();

		// Execute buffered Task
	uint32 tick = CTimeInterface::gameCycle();
	_TickedTaskList.execute(tick);

	// Execute buffered functions that need to be executed in the correct context
	execBufferedCommands();


	execNamedEntityChanges();

	// Update AI instances
	FOREACH(it, CCont<CAIInstance>, CAIS::instance().AIList())
		(*it)->CAIInstance::update();

	// Send systematic messages to EGS
	if (EGSHasMirrorReady)
	{
		// send the fauna description message to EGS then clear the content.
		if (!_FaunaDescriptionList.Bots.empty())
		{
			nlassert(_FaunaDescriptionList.Bots.size() == _FaunaDescriptionList.GrpAlias.size());
			_FaunaDescriptionList.send("EGS");
			_FaunaDescriptionList.Bots.clear();
			_FaunaDescriptionList.GrpAlias.clear();
		}
		// send agglomerated hp changes
		if (!_CreatureChangeHPList.Entities.empty())
		{
			nlassert(_CreatureChangeHPList.Entities.size()==_CreatureChangeHPList.DeltaHp.size());
			_CreatureChangeHPList.send("EGS");
			_CreatureChangeHPList.Entities.clear();
			_CreatureChangeHPList.DeltaHp.clear();
		}
		if (!_CreatureChangeMaxHPList.Entities.empty())
		{
			nlassert(_CreatureChangeMaxHPList.Entities.size()==_CreatureChangeMaxHPList.MaxHp.size());
			nlassert(_CreatureChangeMaxHPList.Entities.size()==_CreatureChangeMaxHPList.SetFull.size());
			_CreatureChangeMaxHPList.send("EGS");
			_CreatureChangeMaxHPList.Entities.clear();
			_CreatureChangeMaxHPList.MaxHp.clear();
			_CreatureChangeMaxHPList.SetFull.clear();
		}
	}

	//
	// update persistent variables every 1024 tick if AI script data manager is flagged
	if ((tick & 0x3FF) == 0)
	{
		updatePersistentVariables();
	}

	//TODO: UserModelManager must send UserModels to EGS if not done yet
	//CAIUserModelManager::getInstance()->sendUserModels();
	// Terminate counters and store stats in an accessible place
	AISStat::countersEnd();
}

// provoke a general 'save to backup' across the whole service
void CAIS::save()
{
	nlinfo("*** save() NOT IMPLEMENTED YET ***");
}


//--------------------------------------------------------------------------
// management of iterator tables for vision matrices
//--------------------------------------------------------------------------

const CAIEntityMatrixIteratorTblLinear* CAIS::bestLinearMatrixIteratorTbl(uint32 distInMeters)
{
#if !FINAL_VERSION
	nlassert(!_matrixIteratorsByDistance.empty());
#endif

	if (distInMeters >= _matrixIteratorsByDistance.size())
	{
		//nlwarning("Try to access to a Vision Matrix to far %u the farest is only %u", distInMeters, _matrixIteratorsByDistance.size());
		return _matrixIteratorsByDistance.back();
	}

	return _matrixIteratorsByDistance[distInMeters];
}

int	getInt64FromStr	(const	char*	str)
{
	if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		return	(int)	atoiInt64(str+2,16);
	return	(int)	atoiInt64(str,10);
}

//	all these dynamics casts can be throwned away ..
CAIInstance*	CAIS::tryToGetAIInstance(const	char*	str)
{
	return	dynamic_cast<CAIInstance*>	(tryToGetEntity(str,CAIS::AI_INSTANCE));
}

CContinent			*CAIS::tryToGetContinent		(const	char	*str)
{
	return	dynamic_cast<CContinent*>	(tryToGetEntity(str,CAIS::AI_CONTINENT));
}

CRegion				*CAIS::tryToGetRegion			(const	char	*str)
{
	return	dynamic_cast<CRegion*>	(tryToGetEntity(str,CAIS::AI_REGION));
}

CCellZone			*CAIS::tryToGetCellZone		(const	char	*str)
{
	return	dynamic_cast<CCellZone*>	(tryToGetEntity(str,CAIS::AI_CELL_ZONE));
}

CFamilyBehavior		*CAIS::tryToGetFamilyBehavior	(const	char	*str)
{
	return	dynamic_cast<CFamilyBehavior*>	(tryToGetEntity(str,CAIS::AI_FAMILY_BEHAVIOR));
}


CManager*	CAIS::tryToGetManager(const	char*	str)
{
	return	dynamic_cast<CManager*>	(tryToGetEntity(str,CAIS::AI_MANAGER));
}

CGroup*		CAIS::tryToGetGroup(const	char*	str)
{
	return	dynamic_cast<CGroup*>	(tryToGetEntity(str,CAIS::AI_GROUP));
}

CBot*		CAIS::tryToGetBot(const	char*	str)
{
	return	dynamic_cast<CBot*>	(tryToGetEntity(str,CAIS::AI_BOT));
}

CAIEntity*	CAIS::tryToGetAIEntity(const	char*	str)
{
	return	dynamic_cast<CAIEntity*>	(tryToGetEntity(str));
}

CAIEntityPhysical*	CAIS::tryToGetEntityPhysical(const	char*	str)
{
	CEntityId	entityId;
	entityId.fromString(str);
	return	CAIS::getEntityPhysical(CMirrors::DataSet->getDataSetRow(entityId));
}

CAIEntity*	CAIS::tryToGetEntity(const	char*	str, TSearchType searchType)
{
	CAIInstance		*aii = NULL;
	CManager		*mgr = NULL;
	CBot			*bot = NULL;
	CGroup			*grp = NULL;


	vector<string>	parts;
	explode(string(str), string(":"), parts, false);

	if (parts.empty() || parts[0].empty())
		return NULL;

	// skip AIS number if any
	if (parts[0].substr(0, 4) == "AIS_")
		parts.erase(parts.begin());

	// check
	if (parts.empty() || parts[0].empty())	return NULL;

	// instance index
	uint32	index = atoui(parts[0].c_str());
	if (index >= CAIS::instance().AIList().size())
		goto tryWithEntityId;
	aii = CAIS::AIList()[index];
	if (!aii)
		goto tryWithEntityId;
	if (searchType==CAIS::AI_INSTANCE
		|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1))
		return aii;

	parts.erase(parts.begin());

	// check
	if (parts.empty() || parts[0].empty())	return NULL;

	// branch on static or dynamic system
	// manager index
	if (parts[0].find("dyn_") == 0)
	{
		// parse dynamic id

		// continent index
		index = atoui(parts[0].substr(4).c_str());
		if (index >= aii->continents().size())
			return NULL;
		CContinent *continent = aii->continents()[index];
		if (searchType==CAIS::AI_CONTINENT
			|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1))
			return continent;

		parts.erase(parts.begin());

		// check
		if (parts.empty() || parts[0].empty())	return NULL;

		// region index
		index = atoui(parts[0].c_str());
		if (index >= continent->regions().size())
			return NULL;
		CRegion *region = continent->regions()[index];
		if (searchType==CAIS::AI_REGION
			|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1))
			return region;

		parts.erase(parts.begin());

		// check
		if (parts.empty() || parts[0].empty())	return NULL;

		// cellzone index
		index = atoui(parts[0].c_str());
		if (index >= region->cellZones().size())
			return NULL;
		CCellZone *cz = region->cellZones()[index];
		if (searchType==CAIS::AI_CELL_ZONE
			|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1))
			return cz;

		parts.erase(parts.begin());

		// check
		if (parts.empty() || parts[0].empty())	return NULL;

		// family behavior index
		index = atoui(parts[0].c_str());
		if (index >= cz->familyBehaviors().size())
			return NULL;
		CFamilyBehavior *fb = cz->familyBehaviors()[index];
		if (searchType==CAIS::AI_FAMILY_BEHAVIOR
			|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1))
			return fb;

		parts.erase(parts.begin());

		// check
		if (parts.empty() || parts[0].empty())
			return NULL;

		// manager index
		if (parts[0] == "npc")
			mgr = fb->mgrNpc();
		else if (parts[0] == "fauna")
			mgr = fb->mgrFauna();

		if (!mgr)
			return NULL;

		if (searchType==CAIS::AI_MANAGER
			|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1)
			|| mgr == NULL)
			return mgr;

		parts.erase(parts.begin());

		// check
		if (parts.empty() || parts[0].empty())	return NULL;
	}
	else
	{
		// parse static id

		// Manager index
		index = atoui(parts[0].c_str());
		if (index >= aii->managers().size())
			return NULL;
		mgr = aii->managers()[index];
		if (searchType==CAIS::AI_MANAGER
			|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1)
			|| mgr == NULL)
			return mgr;

		parts.erase(parts.begin());


		// check
		if (parts.empty() || parts[0].empty())	return NULL;
	}

	// group index
	index = atoui(parts[0].c_str());
	if (index >= mgr->groups().size())
		return NULL;
	grp = mgr->groups()[index];
	if (searchType==CAIS::AI_GROUP
		|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1))
		return grp;

	parts.erase(parts.begin());

	// check
	if (parts.empty() || parts[0].empty())	return NULL;

	// bot index
	index = atoui(parts[0].c_str());
	if (index >= grp->bots().size())
		return NULL;
	bot = grp->bots()[index];
	if (searchType==CAIS::AI_BOT
		|| (searchType == CAIS::AI_UNDEFINED && parts.size() == 1))
		return bot;

	// what ?
	return NULL;

tryWithEntityId:

	CEntityId	entityId;
	entityId.fromString(str);

	if (entityId.isUnknownId())
		return	NULL;

	CCont<CAIInstance>::iterator instanceIt=CAIS::instance().AIList().begin(), instanceItEnd=CAIS::instance().AIList().end();

	while (instanceIt!=instanceItEnd)
	{
		CAIInstance	*instancePtr=*instanceIt;

		CCont<CManager>::iterator it=instancePtr->managers().begin(), itEnd=instancePtr->managers().end();

		while (it!=itEnd)
		{
			CManager *mgrPtr	=	*it;

			CGroup *grpPtr	=	mgrPtr->getNextValidGroupChild	();
			while (grpPtr)
			{
				CBot *botPtr	=	grpPtr->getNextValidBotChild();
				while (botPtr)
				{
					if	(	botPtr->isSpawned()
						&&	botPtr->getSpawnObj()->getEntityId() == entityId)
						return	dynamic_cast<CAIEntity*>	(botPtr);
					botPtr	=	grpPtr->getNextValidBotChild	(botPtr);
				}
				grpPtr=mgrPtr->getNextValidGroupChild	(grpPtr);
			}
			++it;
		}
		++instanceIt;
	}
	return	NULL;
}






//--------------------------------------------------------------------------
// manageing the set of maps
//--------------------------------------------------------------------------

CAIEntityPhysical	*CAIS::getEntityPhysical(const TDataSetRow&	row)
{
	CHashMap<int,NLMISC::CDbgPtr<CAIEntityPhysical> >::iterator	it(_CAIEntityByDataSetRow.find(row.getIndex()));

	if	(it!=_CAIEntityByDataSetRow.end())
		return	(*it).second;
	else
		return	NULL;
//	the code below generates an error .. :( hu !
}


//-------------------------------------------------------------------
// Interface to bot chat - callbacks called when bots start or
// stop chatting with player(s)
//-------------------------------------------------------------------

void	CAIS::beginBotChat(const TDataSetRow &bot,const TDataSetRow &player)
{
#ifdef NL_DEBUG
	/// Is this still true?
	nlwarning("Chat can't work now as bot are now splitted in persistent and spawnable part. Have to rework on this part.");
#endif

	// get a pointer to the bot
	CSpawnBotNpc*	botNpc=dynamic_cast<CSpawnBotNpc*>(CAIS::getEntityPhysical(bot));
	if (!botNpc)
	{
//		nlwarning("CAIS::beginBotChat(): Bot chat message identifies an entity that isn't an NPC!!!");
		return;
	}

	// get a pointer to the player
	CBotPlayer*	plrPtr	=	dynamic_cast<CBotPlayer*>(CAIS::getEntityPhysical(player));
	if (plrPtr==NULL)
	{
//		nlwarning("CAIS::beginBotChat(): Bot chat message identifies an unknown player!!!");
		return;
	}

	// have the bot register the chat
	botNpc->beginBotChat(plrPtr);
}

void	CAIS::endBotChat(const TDataSetRow &bot, const TDataSetRow &player)
{
	// get a pointer to the bot
	CSpawnBotNpc*	botNpc=dynamic_cast<CSpawnBotNpc*>(CAIS::getEntityPhysical(bot));
	if (!botNpc)
	{
//		nlwarning("CAIS::endBotChat(): Bot chat message identifies an entity that isn't an NPC!!!");
		return;
	}

	// get a pointer to the player
	CBotPlayer*	plrPtr	=	dynamic_cast<CBotPlayer*>(CAIS::getEntityPhysical(player));
	if (!plrPtr)
	{
//		nlwarning("CAIS::endBotChat(): Bot chat message identifies an unknown player!!!");
		return;
	}

	// have the bot register the chat end
	botNpc->endBotChat(plrPtr);
}

void	CAIS::beginDynChat(const TDataSetRow &bot)
{
	// get a pointer to the bot
	CSpawnBotNpc*	botNpc=dynamic_cast<CSpawnBotNpc*>(CAIS::getEntityPhysical(bot));
	if (!botNpc)
	{
//		nlwarning("CAIS::beginBotChat(): Bot chat message identifies an entity that isn't an NPC!!!");
		return;
	}

	// have the bot register the chat
	botNpc->beginDynChat();
	nldebug( "DYNCHT: E%u: %u dyn chats", bot.getIndex(), botNpc->getNbActiveDynChats() );
}

void	CAIS::endDynChat(const TDataSetRow &bot)
{
	// get a pointer to the bot
	CSpawnBotNpc*	botNpc=dynamic_cast<CSpawnBotNpc*>(CAIS::getEntityPhysical(bot));
	if (!botNpc)
	{
//		nlwarning("CAIS::endBotChat(): Bot chat message identifies an entity that isn't an NPC!!!");
		return;
	}

	// have the bot register the chat end
	botNpc->endDynChat();
	nldebug( "DYNCHT: E%u: %u dyn chats", bot.getIndex(), botNpc->getNbActiveDynChats() );
}


void CAIPlaceXYR::display(CStringWriter	&stringWriter) const
{
	stringWriter.append("XYR: ("+_pos.x().toString()
		+" "
		+_pos.y().toString()
		+" "+toString(_pos.h())
		+") Radius "
		+toString(_radius)
		+" "
		+getName());

//		nlinfo("XYR: (%s,%s,%d) x %f :%s",_pos.x().toString().c_str(),_pos.y().toString().c_str(),_pos.h(),_radius,getName().c_str());
}


void CAIS::warnBadInstanceMsgImp(const std::string &serviceName, TServiceId serviceId, CWarnBadInstanceMsgImp &msg)
{
	// EGS says that an instance is spoofing an instance number or using a bad static instance number/continent name association.
	// we must despawn/delete the aiinstance

	FOREACH(it, CCont<CAIInstance>, _AIInstances)
	{
		if	((*it)->getInstanceNumber()!=msg.InstanceNumber)
			continue;

		// ok, we found the bad guy !
		nlwarning("CAIS::warnBadInstanceMsgImp: despawning AIInstance %u, instance number %u, continent '%s'",
			(*it)->getChildIndex(), msg.InstanceNumber, (*it)->getContinentName().c_str());
		_AIInstances.removeChildByIndex((*it)->getChildIndex());
		return;
	}

	// not found
	nlwarning("CAIS::warnBadInstanceMsgImp: can't find AIInstance with instance number %u ! Can't despawn it",	msg.InstanceNumber);
}

void CAIS::updatePersistentVariables()
{
	if (CAIScriptDataManager::getInstance()->needsPersistentVarUpdate() == true)
	{
		// sending data to bs
		CPersistentDataRecord pdr("AiTokenFamily");

		CAIScriptDataManager::getInstance()->getPersistentVariables().store(pdr);

		uint32 bufSize= pdr.totalDataSize();
		vector<char> buffer;
		buffer.resize(bufSize);
		pdr.toBuffer(&buffer[0],bufSize);

		CBackupMsgSaveFile msg( CAIScriptDataManager::getInstance()->makePdrFileName(), CBackupMsgSaveFile::SaveFile, Bsi );
		msg.DataMsg.serialBuffer((uint8*)&buffer[0], bufSize);

		Bsi.sendFile( msg );
		CAIScriptDataManager::getInstance()->clearDirtyFlag();
	}
}

CAIInstance	*CAIS::getAIInstance(uint32 instanceNumber)
{
	FOREACH(it, CCont<CAIInstance>, _AIInstances)
	{
		if ((*it)->getInstanceNumber()==instanceNumber)
			return	*it;
	}
	return NULL;
}
