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

#ifndef NL_BAR_MANAGER_H
#define NL_BAR_MANAGER_H


#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "game_share/inventories.h"
#include "game_share/scores.h"

namespace NLMISC{
	class CCDBNodeLeaf;
}


// ***************************************************************************
/**
 * Class that Manage display of Bars (HP, Sta, Sap, Focus)
 *	Such a manager is necessary because the property are sent in 2 ways:
 *		- From Visual Property, often more frequently updated, but only if Entity is in Vision AND within
 *			a certain distance (aka VP threshold which is for instance 30m for Bars)
 *		- From DB (Target, Team, Animal), which are always sent, but less frequently updated
 *	The purpose of this manager is to take either the Visual Property or the Database Value, deciding which is the most
 *	accurate one.
 *	Then the same values are used for the Target, Team and Animal interface, as the 3D InScene interface
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CBarManager
{
public:
	// singleton
	static CBarManager	*getInstance()
	{
		if(!_Instance)
		{
			_Instance= new CBarManager;
		}
		return _Instance;
	}

	// release singleton
	static void releaseInstance();

	// types of entries
	enum	TEntryType
	{
		EntityType= 0,
		TeamMemberType,
		AnimalType,
		TargetType,

		MaxEntryType
	};

	// Max Array size for each entry
	enum
	{
		MaxEntity= 256,
		MaxTeamMember= 8,
		MaxAnimal= MAX_INVENTORY_ANIMAL,
		MaxTarget= 1
	};

	// Flags
	enum	TScoreFlag
	{
		HpFlag=		(1<<SCORES::hit_points),
		SapFlag=	(1<<SCORES::sap),
		StaFlag=	(1<<SCORES::stamina),
		FocusFlag=	(1<<SCORES::focus)
	};

	// Bar Info
	struct CBarInfo
	{
		sint8	Score[SCORES::NUM_SCORES];
		CBarInfo()
		{
			reset();
		}
		void	reset()
		{
			nlctassert(SCORES::NUM_SCORES==4);
			Score[0]= Score[1]= Score[2]= Score[3]= 0;
		}
	};


public:

	// init. Call after Server/Client Database initialisation (CInterfaceManager)
	void		initInGame();
	void		releaseInGame();
	void		resetShardSpecificData();

	/** \name Bar registration. Each Entity is uniquely identified by its dataSetId.
	 *	The entity can be "connected" through diffent ways (entity in vision, entry in team list etc...).
	 *	If it was not connected before, its Bars value are reseted
	 *	WARNING: each of this method assert that the entryId (entityId etc...) is less than the corresponding Max Size (MaxEntity etc...)
	 */
	// @{
	// register an entity in Vision. reset bar values to 0, and assign dataSetId to entity
	void		addEntity(CLFECOMMON::TCLEntityId entityId, uint dataSetId);
	void		delEntity(CLFECOMMON::TCLEntityId entityId);
	// register a TeamMember. NB: no-op if already registered. YOU SHOULD USE updateTeamMemberFromDB()
	void		addTeamMember(uint teamMemberId, uint dataSetId);
	void		delTeamMember(uint teamMemberId);
	// register a Pet Animal. NB: no-op if already registered. YOU SHOULD USE updateAnimalFromDB()
	void		addAnimal(uint paId, uint dataSetId);
	void		delAnimal(uint paId);
	// register the target. NB: no-op if already registered. YOU SHOULD USE updateTargetFromDB()
	void		addTarget(uint dataSetId);
	void		delTarget();
	// @}

	/** called either by VP or DB receive. NB: no-op if the dataSetId was not added through any of preceding fct
	 *	\param serverTick the server date validity of this info (if too old, skiped)
	 *	\param scoreFlags an ORed of TScoreFlag. if not set, the value is not relevant (eg: Team DB don't precise Focus)
	 */
	void		updateBars(uint dataSetId, CBarInfo barInfo, NLMISC::TGameCycle serverTick, uint scoreFlags);

	// do the appropriate addEntry(), delEntry() or/and updateBars(), according to SERVER db change
	void		updateTeamMemberFromDB(uint teamMemberId);
	void		updateAnimalFromDB(uint paId);
	void		updateTargetFromDB();

	// special for the Target. Called when the client select an entity => Out Target Database is flushed with current data
	void		setLocalTarget(uint dataSetId);

	// get the bar values. WARNING assert that entityId < MaxEntity
	CBarInfo	getBarsByEntityId(CLFECOMMON::TCLEntityId entityId) const;

	/*
	 * For Interface Team, Target, and Animal, values are updated in the database:
	 *		UI:VARIABLES:BARS:TEAM:i:HP
	 *		UI:VARIABLES:BARS:ANIMAL:i:HP
	 *		UI:VARIABLES:BARS:TARGET:HP
	 * They are updated on a updateBars(), addxxx() or delxxxx()
	 */

	/// Special Message to set the current HP/SAP/STA/FOCUS for the user.
	void	setupUserBarInfo(uint8 msgNumber, sint32 hp, sint32 sap, sint32 sta, sint32 focus);
	/// From last setuped user HP/SAP/STA/FOCUS, and current database MAX, setup the Bars for the user (slot 0) entry
	void	updateUserBars();

	sint32 getUserScore(SCORES::TScores score);

// ************
private:
	CBarManager();
	~CBarManager() {}
	static	CBarManager		*_Instance;


	// *** Tell us the Bar values, and to which input/output they are connected
	class CBarDataUID
	{
	public:
		/* What connexion are valid. empty() in each case if not, else the index in the array of entries
		 *	NB: this is a set because some time, a bar data may be connected to multiple entries of same type
		 *	This typically happens when you dismiss the team member 0 'Paul' while you have a team member 1 'Pierre':
		 *	because of the server array shift,	there will be a short time where TeamMember0= TeamMember1= Pierre
		 *	Hence the set: Pierre is bound to 2 teammember entries: 0 and 1.
		 */
		std::set<uint>	EntryId[MaxEntryType];

		// The current values of the bars
		CBarInfo	BarInfo;

		// For each score, server tick of last setup
		NLMISC::TGameCycle	ScoreDate[SCORES::NUM_SCORES];

		CBarDataUID()
		{
			for(uint i=0;i<SCORES::NUM_SCORES;i++)
				ScoreDate[i]= 0;
		}
		bool	noMoreEntry() const
		{
			for(uint i=0;i<MaxEntryType;i++)
			{
				if(!EntryId[i].empty())
					return false;
			}
			return true;
		}
	};
	// Data sorted by dataSetId
	typedef	std::map<uint, CBarDataUID>	TUIDToDatas;
	TUIDToDatas							_UIDBars;


	// *** Data sorted by connexion Id (duplication for faster access...)
	class CBarDataEntry
	{
	public:
		// To which DataSetId this apply (INVALID_CLIENT_DATASET_INDEX if not valid)
		uint		DataSetId;

		// The current values of the bars
		CBarInfo	BarInfo;

		// Connection input (used only for TargetType, TeamMemberType and AnimalType)
		NLMISC::CCDBNodeLeaf	*UIDIn;
		NLMISC::CCDBNodeLeaf	*PresentIn;		// if not NULL, this is an additional test: if(PresentIn->getValue()==0) => not present
		NLMISC::CCDBNodeLeaf	*ScoreIn[SCORES::NUM_SCORES];

		// Connection output
		NLMISC::CCDBNodeLeaf	*ScoreOut[SCORES::NUM_SCORES];

	public:
		CBarDataEntry();
		// reset the DataSetId, and BarInfo (not DB)
		void	clear();
		// connect
		void	connectDB(const std::string &baseDBin, const std::string &baseDBout, const std::string &presentDB,
			const std::string &hpDB, const std::string &sapDB, const std::string &staDB, const std::string &focusDB);
		void	resetDB();
		// flush the value to the DB (only values linked)
		void	flushDBOut();
		// modify from the DB in (only values linked)
		void	modifyFromDBIn(CBarInfo &barInfo) const;
	};

	// "template" method to add or remove an entry
	void		addEntry(TEntryType type, uint entryId, uint dataSetId);
	void		delEntry(TEntryType type, uint entryId);

	// "template" method to update an entry from DB
	void		updateEntryFromDB(TEntryType type, uint entryId);
	void		updateEntryFromDBNoAddDel(TEntryType type, CBarDataEntry &bde);

	// One array for each type
	std::vector<CBarDataEntry>		_EntryBars[MaxEntryType];

	// For each type, tells what entry are connected
	uint							_EntryScoreFlags[MaxEntryType];


	/// Special For UserBars (transferred from impulse)
	// @{
	uint8		_LastUserBarMsgNumber;
	struct CUserScore
	{
		// last score get from impulse USER:BARS
		sint32	Score;
		// input DB value, to get the current MAX
		NLMISC::CCDBNodeLeaf	*DBInMax;
		// output DB to store the real value, but clamped to 0
		NLMISC::CCDBNodeLeaf	*DBOutVal;
		// output DB to store the ratio -1024,1024 value
		NLMISC::CCDBNodeLeaf	*DBOutRatio;
		CUserScore()
		{
			Score= 0;
			DBInMax= DBOutVal= DBOutRatio= NULL;
		}
	};
	CUserScore		_UserScores[SCORES::NUM_SCORES];
	CBarInfo		_UserBarInfo;
	enum	{UserBarMaxRatio= 1024};
	// @}

};


#endif // NL_BAR_MANAGER_H

/* End of bar_manager.h */
