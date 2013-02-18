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

#include "bar_manager.h"
#include "interface_manager.h"
#include "nel/gui/interface_expr.h"
#include "../time_client.h"


using namespace std;
using namespace NLMISC;


// ***************************************************************************
// LOG, for debug
#define ALLOW_BAR_LOG 0

// don't change this (ensure no bug in final version)
#if FINAL_VERSION
#undef ALLOW_BAR_LOG
#define ALLOW_BAR_LOG 0
#endif
#if ALLOW_BAR_LOG
#	define barInfoLog nlinfo
#else // NL_RELEASE
#	ifdef NL_COMP_VC71
#		define barInfoLog __noop
#	else
#		define barInfoLog 0&&
#	endif
#endif // NL_RELEASE


static const char *entryTypeToStr(CBarManager::TEntryType et)
{
	switch(et)
	{
	case CBarManager::EntityType: return "Entity"; break;
	case CBarManager::TeamMemberType: return "TeamMember"; break;
	case CBarManager::AnimalType: return "Animal"; break;
	case CBarManager::TargetType: return "Target"; break;
	default: return "Unknown (Error!!)"; break;
	}
}


// ***************************************************************************
CBarManager		*CBarManager::_Instance= NULL;


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CBarManager::CBarDataEntry::CBarDataEntry()
{
	clear();
	resetDB();
}

// ***************************************************************************
void	CBarManager::CBarDataEntry::clear()
{
	DataSetId= CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
	BarInfo.reset();
}

// ***************************************************************************
void	CBarManager::CBarDataEntry::resetDB()
{
	UIDIn= NULL;
	PresentIn= NULL;
	for(uint sc=0;sc<SCORES::NUM_SCORES;sc++)
	{
		ScoreIn[sc]= NULL;
		ScoreOut[sc]= NULL;
	}
}

// ***************************************************************************
void	CBarManager::CBarDataEntry::connectDB(const std::string &baseDBin, const std::string &baseDBout, const std::string &presentDB,
			const std::string &hpDB, const std::string &sapDB, const std::string &staDB, const std::string &focusDB)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// can only manage 4 scores here
	nlctassert(SCORES::NUM_SCORES==4);

	// try to connect each input entry (don't create)
	if(!baseDBin.empty())
	{
		UIDIn= NLGUI::CDBManager::getInstance()->getDbProp(baseDBin+"UID", false);
		if(!presentDB.empty())
			PresentIn= NLGUI::CDBManager::getInstance()->getDbProp(baseDBin+presentDB, false);
		if(!hpDB.empty())
			ScoreIn[SCORES::hit_points]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBin+hpDB, false);
		if(!sapDB.empty())
			ScoreIn[SCORES::sap]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBin+sapDB, false);
		if(!staDB.empty())
			ScoreIn[SCORES::stamina]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBin+staDB, false);
		if(!focusDB.empty())
			ScoreIn[SCORES::focus]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBin+focusDB, false);
	}

	// try to connect each output entry (don't create)
	if(!baseDBout.empty())
	{
		if(!hpDB.empty())
			ScoreOut[SCORES::hit_points]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBout+hpDB, false);
		if(!sapDB.empty())
			ScoreOut[SCORES::sap]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBout+sapDB, false);
		if(!staDB.empty())
			ScoreOut[SCORES::stamina]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBout+staDB, false);
		if(!focusDB.empty())
			ScoreOut[SCORES::focus]= NLGUI::CDBManager::getInstance()->getDbProp(baseDBout+focusDB, false);
	}
}

// ***************************************************************************
void	CBarManager::CBarDataEntry::flushDBOut()
{
	for(uint sc=0;sc<SCORES::NUM_SCORES;sc++)
	{
		if(ScoreOut[sc])
			ScoreOut[sc]->setValue8(BarInfo.Score[sc]);
	}
}

// ***************************************************************************
void	CBarManager::CBarDataEntry::modifyFromDBIn(CBarInfo &barInfo) const
{
	for(uint sc=0;sc<SCORES::NUM_SCORES;sc++)
	{
		if(ScoreIn[sc])
			barInfo.Score[sc]= ScoreIn[sc]->getValue8();
	}
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CBarManager::CBarManager()
{
	// if more than 256 entities, must change MaxEntity
	nlctassert(sizeof(CLFECOMMON::TCLEntityId)==1);

	// Allocate the array now
	nlctassert(MaxEntryType==4);
	_EntryBars[EntityType].resize(MaxEntity);
	_EntryBars[TeamMemberType].resize(MaxTeamMember);
	_EntryBars[AnimalType].resize(MaxAnimal);
	_EntryBars[TargetType].resize(MaxTarget);

	// no msg still sent
	_LastUserBarMsgNumber= 0;
}

// ***************************************************************************
void CBarManager::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ***************************************************************************
void		CBarManager::initInGame()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	//resetShardSpecificData(); // directly called by CFarTP::disconnectFromPreviousShard()

	/* *** Verify that MaxTeamMember is correct according to database
		change MaxTeamMember if no more the case
	*/
	uint	i=0;
	while( NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:GROUP:%d:NAME", i), false) )
		i++;
	nlassert(i==MaxTeamMember);


	// *** create connexion to the Local Output database
	for(i=0;i<_EntryBars[TeamMemberType].size();i++)
	{
		// don't connect FOCUS, since not setuped by SERVER
		_EntryBars[TeamMemberType][i].connectDB(
			toString("SERVER:GROUP:%d:",i),
			toString("UI:VARIABLES:BARS:TEAM:%d:",i),
			"PRESENT",
			"HP", "SAP", "STA", "");
	}
	for(i=0;i<_EntryBars[AnimalType].size();i++)
	{
		// don't connect STA, SAP and FOCUS for animal, since they don't have
		_EntryBars[AnimalType][i].connectDB(
			toString("SERVER:PACK_ANIMAL:BEAST%d:",i),
			toString("UI:VARIABLES:BARS:ANIMAL:%d:",i),
			"STATUS",
			"HP", "", "", "");
	}
	nlassert(_EntryBars[TargetType].size()==1);
	_EntryBars[TargetType][0].connectDB(
		"SERVER:TARGET:BARS:",
		"UI:VARIABLES:BARS:TARGET:",
		"",	// no present flag for target (not so important)
		"HP", "SAP", "STA", "FOCUS");

	// NB: don't connect the DB for entities, since CEntityCL read it directly from getBarsByEntityId() (simpler and faster)


	// *** init _EntryScoreFlags
	nlctassert(MaxEntryType==4);
	nlctassert(SCORES::NUM_SCORES==4);
	// For each entry type, tells what score they can affect (see DB connection above)
	_EntryScoreFlags[EntityType]= HpFlag | SapFlag | StaFlag | FocusFlag;	// all
	_EntryScoreFlags[TeamMemberType]= HpFlag | SapFlag | StaFlag;			// anything but focus
	_EntryScoreFlags[AnimalType]= HpFlag;									// Hp only
	_EntryScoreFlags[TargetType]= HpFlag | SapFlag | StaFlag | FocusFlag;	// all


	// *** create connexion for User Bar mgt
	// user now can only manage 4 scores
	nlctassert(SCORES::NUM_SCORES==4);
	// Input max values
	_UserScores[SCORES::hit_points].DBInMax= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SCORES0:Max", false);
	_UserScores[SCORES::sap].DBInMax= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SCORES2:Max", false);
	_UserScores[SCORES::stamina].DBInMax= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SCORES1:Max", false);
	_UserScores[SCORES::focus].DBInMax= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:SCORES3:Max", false);
	// Output real values
	_UserScores[SCORES::hit_points].DBOutVal= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:HP", false);
	_UserScores[SCORES::sap].DBOutVal= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:SAP", false);
	_UserScores[SCORES::stamina].DBOutVal= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:STA", false);
	_UserScores[SCORES::focus].DBOutVal= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:FOCUS", false);
	// Output ratio values
	_UserScores[SCORES::hit_points].DBOutRatio= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:HP_RATIO", false);
	_UserScores[SCORES::sap].DBOutRatio= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:SAP_RATIO", false);
	_UserScores[SCORES::stamina].DBOutRatio= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:STA_RATIO", false);
	_UserScores[SCORES::focus].DBOutRatio= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:FOCUS_RATIO", false);
}

// ***************************************************************************
void		CBarManager::releaseInGame()
{
	// Reset all, and also the DB out
	for(uint type=0;type<MaxEntryType;type++)
	{
		for(uint i=0;i<_EntryBars[type].size();i++)
		{
			_EntryBars[type][i].clear();
			_EntryBars[type][i].resetDB();
		}
	}

	// Reset the map
	_UIDBars.clear();
}

// ***************************************************************************
void		CBarManager::resetShardSpecificData()
{
	// Reset update counter to update properly when reselecting character
	_LastUserBarMsgNumber = 0;
}

// ***************************************************************************
void		CBarManager::addEntry(TEntryType type, uint entryId, uint dataSetId)
{
	std::vector<CBarDataEntry> &entryArray= _EntryBars[type];
	nlassert(entryId<entryArray.size());

	barInfoLog("BARS: addEntry(%s, eid=%d, dsid=%x)", entryTypeToStr(type), entryId, dataSetId);

	// if bad id, quit
	if(dataSetId==CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
		return;
	// if already registered, quit
	if(entryArray[entryId].DataSetId==dataSetId)
		return;

	// remove the preceding entry, reseting values if necessary
	delEntry(type, entryId);

	// Add me to list of entries
	CBarDataEntry	&bde= entryArray[entryId];
	bde.DataSetId= dataSetId;
	// Add the entry connexion to map by DataSetId
	CBarDataUID		&barUID= _UIDBars[dataSetId];
	barUID.EntryId[type].insert(entryId);

	// Special case: if the entry added is the user (slot 0), then force the correct bars info
	// This is important because the UserEntity can be created AFTER the receive of USER:BARS message.
	if(dataSetId==_EntryBars[EntityType][0].DataSetId)
	{
		barUID.BarInfo= _UserBarInfo;
	}

	// Copy the current Bar Info from the map by dataSetId in case it exists before...
	// Eg: an entity comes in vision (EntityType entry) while precedingly available in TeamMemberType Entry
	bde.BarInfo= barUID.BarInfo;
	// then report to DB (if any)
	bde.flushDBOut();
}

// ***************************************************************************
void		CBarManager::delEntry(TEntryType type, uint entryId)
{
	std::vector<CBarDataEntry> &entryArray= _EntryBars[type];
	nlassert(entryId<entryArray.size());

	barInfoLog("BARS: delEntry(%s, eid=%d)", entryTypeToStr(type), entryId);

	// if a DataSetId was registered before
	uint	dataSetId= entryArray[entryId].DataSetId;
	if(dataSetId!=CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
	{
		// then unconnect this from the map by UID
		_UIDBars[dataSetId].EntryId[type].erase(entryId);
		// if no more connexion are made to this UID
		if(_UIDBars[dataSetId].noMoreEntry())
		{
			// erase it
			_UIDBars.erase(dataSetId);
		}
	}

	// clear entry
	entryArray[entryId].clear();
	// flush the clear to the DB if any
	entryArray[entryId].flushDBOut();
}

// ***************************************************************************
void		CBarManager::addEntity(CLFECOMMON::TCLEntityId entityId, uint dataSetId)
{
	addEntry(EntityType, entityId, dataSetId);
}

// ***************************************************************************
void		CBarManager::delEntity(CLFECOMMON::TCLEntityId entityId)
{
	delEntry(EntityType, entityId);
}

// ***************************************************************************
void		CBarManager::addTeamMember(uint teamMemberId, uint dataSetId)
{
	addEntry(TeamMemberType, teamMemberId, dataSetId);
}

// ***************************************************************************
void		CBarManager::delTeamMember(uint teamMemberId)
{
	delEntry(TeamMemberType, teamMemberId);
}

// ***************************************************************************
void		CBarManager::addAnimal(uint paId, uint dataSetId)
{
	addEntry(AnimalType, paId, dataSetId);
}

// ***************************************************************************
void		CBarManager::delAnimal(uint paId)
{
	delEntry(AnimalType, paId);
}

// ***************************************************************************
void		CBarManager::addTarget(uint dataSetId)
{
	addEntry(TargetType, 0, dataSetId);
}

// ***************************************************************************
void		CBarManager::delTarget()
{
	delEntry(TargetType, 0);
}

// ***************************************************************************
void		CBarManager::updateBars(uint dataSetId, CBarInfo barInfo, TGameCycle serverTick, uint scoreFlags)
{
	// if dataSetId not registered, quit
	TUIDToDatas::iterator	it= _UIDBars.find(dataSetId);
	if(it==_UIDBars.end())
		return;

	barInfoLog("BARS: updateBars(dsid=%x, biHP=%d, t=%d, sf=%x", dataSetId, barInfo.Score[SCORES::hit_points], serverTick, scoreFlags);

	// special Case: if the info is for the User (slot 0)
	if(dataSetId==_EntryBars[EntityType][0].DataSetId)
	{
		/* Then override the bar info with the one received by USER:BARS message (always take this one)
			For instance user bars can be received from VP or from TARGET database (if he targets himself)
			But always consider the message USER:BARS as the most accurate one
		*/
		barInfo= _UserBarInfo;
	}

	// fill bar info, with relevant values only
	CBarDataUID		&barUid= it->second;
	for(uint sc=0;sc<SCORES::NUM_SCORES;sc++)
	{
		// if the update affect this score, and if the modification date is more recent (or at least the same)
		if( (scoreFlags&(1<<sc)) && serverTick>=barUid.ScoreDate[sc] )
		{
			// then change this score with the more recent one!
			barUid.ScoreDate[sc]= serverTick;
			barUid.BarInfo.Score[sc]= barInfo.Score[sc];
		}
	}

	// and report result to all connected entries
	for(uint type=0;type<MaxEntryType;type++)
	{
		// For any connected entries of this type (see EntryId definitio why a set is necessary)
		std::set<uint>::iterator	itEid= barUid.EntryId[type].begin();
		std::set<uint>::iterator	itEidEnd= barUid.EntryId[type].end();
		for(;itEid!=itEidEnd;itEid++)
		{
			uint	entryId= *itEid;
			nlassert(entryId<_EntryBars[type].size());
			CBarDataEntry	&bde= _EntryBars[type][entryId];
			// copy the bar info (with relevant values)
			bde.BarInfo= barUid.BarInfo;
			// and flush DB (if any)
			bde.flushDBOut();
		}
	}
}

// ***************************************************************************
void		CBarManager::updateEntryFromDBNoAddDel(TEntryType type, CBarDataEntry &bde)
{
	// get the Bar Info, from the input DB of this entry (only values linked)
	CBarInfo	barInfo;
	bde.modifyFromDBIn(barInfo);

	// Get the last DB modification server tick
	TGameCycle	serverTick= 0;
	/* To do this, I test all DB input, and choose the highest changeDate
		This works because the branch is atomic (all DB are relevant to the others, and
		therefore relevant agst the most recent one)
	*/
	if(bde.UIDIn && bde.UIDIn->getLastChangeGC() > serverTick )
		serverTick= bde.UIDIn->getLastChangeGC();
	if(bde.PresentIn && bde.PresentIn->getLastChangeGC() > serverTick )
		serverTick= bde.PresentIn->getLastChangeGC();
	for(uint sc=0;sc<SCORES::NUM_SCORES;sc++)
	{
		if( bde.ScoreIn[sc] && bde.ScoreIn[sc]->getLastChangeGC() > serverTick )
			serverTick= bde.ScoreIn[sc]->getLastChangeGC();
	}

	// then update the bars
	updateBars(bde.DataSetId, barInfo, serverTick, _EntryScoreFlags[type] );
}

// ***************************************************************************
void		CBarManager::updateEntryFromDB(TEntryType type, uint entryId)
{
	std::vector<CBarDataEntry> &entryArray= _EntryBars[type];
	nlassert(entryId<entryArray.size());
	CBarDataEntry	&bde= entryArray[entryId];

	// if the UID db was not found, can't do nothing... => abort
	if(bde.UIDIn==NULL)
		return;

	// get the new UID from the SERVER DB
	uint	newDataSetId= bde.UIDIn->getValue32();
	// if present flag is linked, and if 0, then force invalid data set id
	if(bde.PresentIn && bde.PresentIn->getValue32()==0)
		newDataSetId= CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;

	// *** if the data set id does not correspond as the cached one
	if(newDataSetId!=bde.DataSetId)
	{
		// if deleted, delete this entry
		if(newDataSetId==CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
			delEntry(type, entryId);
		// else add
		else
			addEntry(type, entryId, newDataSetId);
		// should be changed
		nlassert(bde.DataSetId==newDataSetId);
	}

	// *** update from DB
	if(newDataSetId!=CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
	{
		updateEntryFromDBNoAddDel(type, bde);
	}
}

// ***************************************************************************
void		CBarManager::updateTeamMemberFromDB(uint teamMemberId)
{
	updateEntryFromDB(TeamMemberType, teamMemberId);
}

// ***************************************************************************
void		CBarManager::updateAnimalFromDB(uint paId)
{
	updateEntryFromDB(AnimalType, paId);
}

// ***************************************************************************
void		CBarManager::updateTargetFromDB()
{
	/* Special case for Target. The DataSetId is not replaced with those in DB (setuped with setLocalTarget())
		Instead, we ensure that the 2 match, else ignore DB change
	*/
	CBarDataEntry	&bde= _EntryBars[TargetType][0];

	// if the UID db was not found, can't do nothing... => abort
	if(bde.UIDIn==NULL)
		return;

	// get the new UID from the SERVER DB
	uint	serverDataSetId= bde.UIDIn->getValue32();

	barInfoLog("BARS: updateTargetFromDB(dsid=%x)", serverDataSetId);

	// *** if the server data set id correspond to the local one (and not invalid), update from DB
	if(serverDataSetId==bde.DataSetId && serverDataSetId!=CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
	{
		updateEntryFromDBNoAddDel(TargetType, bde);
	}
}

// ***************************************************************************
void		CBarManager::setLocalTarget(uint dataSetId)
{
	CBarDataEntry	&bde= _EntryBars[TargetType][0];

	barInfoLog("BARS: setLocalTarget(dsid=%x)", dataSetId);

	// *** if the data set id does not correspond as the cached one
	if(dataSetId!=bde.DataSetId)
	{
		// if deleted, delete this entry
		if(dataSetId==CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
			delEntry(TargetType, 0);
		// else add
		else
			addEntry(TargetType, 0, dataSetId);
		// should be changed
		nlassert(bde.DataSetId==dataSetId);
	}
}

// ***************************************************************************
CBarManager::CBarInfo	CBarManager::getBarsByEntityId(CLFECOMMON::TCLEntityId entityId) const
{
	const std::vector<CBarDataEntry>		&entityBars= _EntryBars[EntityType];
	nlassert(entityId<entityBars.size());

	return entityBars[entityId].BarInfo;
}

// ***************************************************************************
void	CBarManager::setupUserBarInfo(uint8 msgNumber, sint32 hp, sint32 sap, sint32 sta, sint32 focus)
{
	/*
		Since we are not sure of the message order, use a little counter to discard old messages
		suppose that cannot have more than a jump of 127 messages.
		eg:
			If i Receive:			0..1..4..3..2..5
			This will be applied:	0..1..4........5
	*/

	// compute the difference beetween the msgNumber and the last received
	sint32	diff;
	if(msgNumber>=_LastUserBarMsgNumber)
		diff= (sint32)msgNumber - (sint32)_LastUserBarMsgNumber;
	else
		diff= (sint32)msgNumber + 256 - (sint32)_LastUserBarMsgNumber;
	// if too big difference, suppose a roll (eg last==255, cur=0, real diff==1)
	if(diff>127)
		diff-=256;

	// if diff<0, means that the cur message is actually too old => discard
	if(diff<0)
		return;
	else
	{
		// bkup last
		_LastUserBarMsgNumber= msgNumber;
		// user now can only manage 4 scores
		nlctassert(SCORES::NUM_SCORES==4);
		_UserScores[SCORES::hit_points].Score= hp;
		_UserScores[SCORES::sap].Score= sap;
		_UserScores[SCORES::stamina].Score= sta;
		_UserScores[SCORES::focus].Score= focus;

		// update actual database now.
		for(uint i=0;i<SCORES::NUM_SCORES;i++)
		{
			// Clamp To 0, since used only by entries that don't need negative values (for comma mode)
			if(_UserScores[i].DBOutVal)	_UserScores[i].DBOutVal->setValue32(max((sint32)0,_UserScores[i].Score));
		}

		// Update the Player Entry
		updateUserBars();
	}
}

// ***************************************************************************
void	CBarManager::updateUserBars()
{
	// for all scores
	for(uint i=0;i<SCORES::NUM_SCORES;i++)
	{
		CUserScore	&us= _UserScores[i];

		// get max value
		sint32	maxVal= 1;
		if(us.DBInMax)
		{
			maxVal= us.DBInMax->getValue32();
			maxVal= max((sint32)1, maxVal);
		}

		// setup signed ratio. It is used for the view of Player interface
		if(us.DBOutRatio)
		{
			sint32	v= UserBarMaxRatio*us.Score / maxVal;
			clamp(v, (sint32)-UserBarMaxRatio, (sint32)UserBarMaxRatio);
			us.DBOutRatio->setValue32(v);
		}

		// compute signed ratio -127 / 127, for CBarInfo replace
		{
			sint32	v= 127*us.Score / maxVal;
			clamp(v, -127, 127);
			_UserBarInfo.Score[i]= (sint8)v;
		}
	}

	// replace the user Entry with the bar info.
	// This is used only for the view of bars InScene, and in case the user target himself
	uint	userDataSetId= _EntryBars[EntityType][0].DataSetId;
	if(userDataSetId!=CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
	{
		// Do a little cheat: force the update by retrieving the last update time in the BarData
		TUIDToDatas::iterator	it= _UIDBars.find(userDataSetId);
		if(it!=_UIDBars.end())
		{
			TGameCycle	serverTick= 0;
			for(uint sc=0;sc<SCORES::NUM_SCORES;sc++)
			{
				if(it->second.ScoreDate[sc] > serverTick)
					serverTick= it->second.ScoreDate[sc];
			}

			// update (user can only manage 4 scores for now)
			nlctassert(SCORES::NUM_SCORES==4);
			updateBars(userDataSetId, _UserBarInfo, serverTick, HpFlag | SapFlag | StaFlag | FocusFlag);
		}
	}
}

// ***************************************************************************
sint32 CBarManager::getUserScore(SCORES::TScores score)
{
	nlassert((uint) score < SCORES::NUM_SCORES);
	nlassert(_UserScores[score].DBOutVal); // initInGame() not called ?
	return _UserScores[score].DBOutVal->getValue32();
}


// ***************************************************************************
// ***************************************************************************
// ACTION HANDLERS
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
DECLARE_INTERFACE_CONSTANT(getMaxAnimal, CBarManager::MaxAnimal);
DECLARE_INTERFACE_CONSTANT(getMaxTeamMember, CBarManager::MaxTeamMember);


// ***************************************************************************
class CAHBarManagerOnTarget : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CBarManager::getInstance()->updateTargetFromDB();
	}
};
REGISTER_ACTION_HANDLER(CAHBarManagerOnTarget, "bar_manager_on_target");

// ***************************************************************************
class CAHBarManagerOnTeam : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		uint	index;
		fromString(Params, index);
		if(index<CBarManager::MaxTeamMember)
			CBarManager::getInstance()->updateTeamMemberFromDB(index);
	}
};
REGISTER_ACTION_HANDLER(CAHBarManagerOnTeam, "bar_manager_on_team");

// ***************************************************************************
class CAHBarManagerOnAnimal : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		uint	index;
		fromString(Params, index);
		if(index<CBarManager::MaxAnimal)
			CBarManager::getInstance()->updateAnimalFromDB(index);
	}
};
REGISTER_ACTION_HANDLER(CAHBarManagerOnAnimal, "bar_manager_on_animal");

// ***************************************************************************
class CAHBarManagerOnUserScores : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CBarManager::getInstance()->updateUserBars();
	}
};
REGISTER_ACTION_HANDLER(CAHBarManagerOnUserScores, "bar_manager_on_user_scores");



