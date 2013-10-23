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



#ifndef RYAI_AI_H
#define RYAI_AI_H


#include "child_container.h"
#include "server_share/msg_ai_service.h"
#include "nel/misc/random.h"
#include "nel/misc/variable.h"
#include "ai_entity_matrix.h"
#include "service_dependencies.h"
#include "game_share/task_list.h"


class	CBot;
class	CGroup;
class	CManager;
class	CContinent;
class	CRegion;
class	CCellZone;
class	CFamilyBehavior;

extern	bool	IOSHasMirrorReady;
extern	bool	EGSHasMirrorReady;

extern	const	std::string	disengageString;
extern	const	std::string	egsString;

extern	NLMISC::CVariable<uint32>	TotalMaxPlayer;
extern	NLMISC::CVariable<uint32>	TotalMaxPet;
extern	NLMISC::CVariable<uint32>	TotalMaxFauna;
extern	NLMISC::CVariable<uint32>	TotalMaxNpc;
extern	NLMISC::CVariable<uint32>	TotalMaxFx;

extern	NLMISC::CVariable<std::string>	BotRepopFx;


template	<class	T>	class CAIEntityMatrix;
class CAIEntityMatrixIteratorTblRandom;
class CAIEntityMatrixIteratorTblLinear;


//class CManager;
class CAIEntityPhysical;
class CAIEntity;
class CMgrPet;
class CManagerPlayer;
class CGroupNpc;
class CPersistentOfPhysical;


int	getInt64FromStr	(const	char*	str);

/*
  -----------------------------------------------------------------------------

  AI is the singleton manager class for the high speed ai system

  -----------------------------------------------------------------------------
*/

class	CManagerPlayer;
class	CAIInstance;

class CWarnBadInstanceMsgImp : public CWarnBadInstanceMsg
{
public:
	void callback(const std::string &name, NLNET::TServiceId id);
};


class CAIS : public CServiceEvent::CHandler
{
public:

	// singleton access
	static CAIS &instance();

	static bool instanceCreated()	{ return _Instance != NULL; }
	
	static	std::string	getIndexString()	{	return	NLMISC::toString("AIS_%u", NLNET::IService::getInstance()->getServiceId().get());	}
	
	//-------------------------------------------------------------------
	// Constants

	//-------------------------------------------------------------------
	// classic init(), update() and release()

	/** create an AI instance, return the instance index in the AIList
	 *	Return ~0 if the instance number is already in use.
	 */
	uint32	createAIInstance(const std::string &continentName, uint32 instanceNumber);
	/** destroy an AI Instance (useful for ring creation / destruction of session)
	@param instanceNumber is the AiInstance Id
	@param displayWarningIfInstanceNotExist If false nothing happends when the specified instance do not exist
	*/
	void	destroyAIInstance(uint32 instanceNumber, bool displayWarningIfInstanceNotExist);

	// the update routine called once per tick
	// this is the routine that calls the managers' updates
	void update();

	// This routine is called once every 1000 ticks
	// updates the persistent ai script variables values on the Backup Service
	void updatePersistentVariables();

	// release the singleton before program exit
	void release();

	void	serviceEvent	(const	CServiceEvent	&info);

	// Management of deleted root alias
	bool markTagForDelete(const std::string &filename);
	void deleteTaggedAlias(const std::string &filename);

	//-------------------------------------------------------------------
	// dealing with backups (save and restore)

	// provoke a general 'save to backup' across the whole service
	void save();

	inline	void	setClientCreatureDebug	(bool clientCreatureDebug)
	{
		_ClientCreatureDebug=clientCreatureDebug;
	}

	inline	bool	clientCreatureDebug	()
	{
		return	_ClientCreatureDebug;
	}
		


	//-------------------------------------------------------------------
	// Interface to bot chat - callbacks called when bots start or 
	// stop chatting with player(s)
	void beginBotChat	(const TDataSetRow &bot, const TDataSetRow &player);
	void endBotChat		(const TDataSetRow &bot, const TDataSetRow &player);
	void beginDynChat	(const TDataSetRow &bot);
	void endDynChat		(const TDataSetRow &bot);
	
	//-------------------------------------------------------------------
	// manageing the set of managers

	//	a method that parse a supposed know type of manager:group:bot hierarchy and return the element as CAIEntity.
	CAIEntityPhysical	*tryToGetEntityPhysical	(const	char	*str);
	CAIInstance			*tryToGetAIInstance		(const	char	*str);

	// dynamic system branch
	CContinent			*tryToGetContinent		(const	char	*str);
	CRegion				*tryToGetRegion			(const	char	*str);
	CCellZone			*tryToGetCellZone		(const	char	*str);
	CFamilyBehavior		*tryToGetFamilyBehavior	(const	char	*str);
	
	// generic branch
	CManager			*tryToGetManager		(const	char	*str);
	CGroup				*tryToGetGroup			(const	char	*str);
	CBot				*tryToGetBot			(const	char	*str);
	CAIEntity			*tryToGetAIEntity		(const	char	*str);
	
	//-------------------------------------------------------------------
	//	the previous interfaces for searching the data structures for named objects are transfered in CAIEntityId 
	//	as its one of their object behavior. a solution to build id directly was added.
		
	//	to update the map.
	friend	class	CAIEntityPhysical;
	CAIEntityPhysical		*getEntityPhysical(const TDataSetRow &);
	
	//-------------------------------------------------------------------
	// Interface to the vision management matrices

	// read accessors for getting hold of the vision matrices and their associated iterator tables
	inline const CAIEntityMatrixIteratorTblRandom	*matrixIterator2x2();
	inline const CAIEntityMatrixIteratorTblRandom	*matrixIterator3x3();
	const CAIEntityMatrixIteratorTblLinear			*bestLinearMatrixIteratorTbl(uint32 distInMeters);

	//-------------------------------------------------------------------
	// Interface to the random number generator
	static inline sint32 randPlusMinus(uint16 mod);
	static inline float frand(double mod=1.0);
	static inline float frandPlusMinus(double mod);
	static inline uint32 rand32();
	// WARNING : this rand has a 'coherent' behavior : ie it return value between 0 and mod-1
	static inline uint32 rand32(uint32 mod);
	// WARNING : this rand has a 'coherent' behavior : ie it return value between 0 and mod-1
	static inline uint32 rand16(uint32 mod);

	/// Time warp managment. This method is called when time as warped more than 600ms
	bool	advanceUserTimer(uint32 nbTicks);

	/// Retreive emot number given it's name, return ~0 if not found
	uint32	getEmotNumber(const std::string &name);

	CCont<CAIInstance>		&AIList	()		{	return	_AIInstances;	}
	CCont<CAIInstance>		&aiinstances()	{	return	_AIInstances;	} ///< This is a synonym for AIList, but should replace it as it's more coherent with general 'nomenclature'
	
	CAIInstance		*getAIInstance(uint32 instanceNumber);
	
	CFaunaBotDescription	&getFaunaDescription()
	{
		return _FaunaDescriptionList;
	}
	CChangeCreatureHPMsg	&getCreatureChangeHP()
	{
		return _CreatureChangeHPList;
	}

	CChangeCreatureMaxHPMsg &getCreatureChangeMaxHP()
	{
		return _CreatureChangeMaxHPList;
	}
	
	enum	TSearchType
	{
		AI_INSTANCE = 0,
			AI_CONTINENT,
			AI_REGION,
			AI_CELL_ZONE,
			AI_FAMILY_BEHAVIOR,
			AI_MANAGER,
			AI_GROUP,
			AI_BOT,
			AI_UNDEFINED
	};

	class CCounter
	{
	public:
		CCounter(const uint32 max=~0):_Total(0),_Max(max)
		{}
		virtual ~CCounter()
		{}
		void	setMax(const uint32 max)	{	_Max=max;		}
		void	inc()		{	_Total++;	}
		void	dec()		{	_Total--;	}
		uint32	getTotal()	const	{	return _Total;	}
		bool	remainToMax	(uint32 nbMore=1)	const	{ return (_Total+nbMore)<_Max; }
	protected:
	private:
		uint32	_Total;
		uint32	_Max;
	};
	
	CCounter	_PetBotCounter;
	CCounter	_NpcBotCounter;
	CCounter	_FaunaBotCounter;

	// message from EGS about bad aiinstance 
	void warnBadInstanceMsgImp(const std::string &serviceName, NLNET::TServiceId serviceId, CWarnBadInstanceMsgImp &msg);

	void addTickedTask(uint32 tick, CTask<uint32>* task) { _TickedTaskList.addTaskAt(tick, task); }
	

private:

	// private constructor
	CAIS();

	// initialise the singleton
	void initAI();

	/// Singleton instance
	static CAIS		*_Instance;

	CAIEntity*	tryToGetEntity	(const	char*	str, TSearchType searchType=AI_UNDEFINED);
	// Global map of entity in this shard
	CHashMap<int,NLMISC::CDbgPtr<CAIEntityPhysical> >	_CAIEntityByDataSetRow;
	
	// the random number generator
	static	NLMISC::CRandom	_random;

	CAIEntityMatrixIteratorTblRandom			_matrixIterator2x2;
	CAIEntityMatrixIteratorTblRandom			_matrixIterator3x3;
	std::vector<CAIEntityMatrixIteratorTblLinear*> _matrixIteratorsByDistance;

	// Faunas descriptions to be sent each frame 
	CFaunaBotDescription	_FaunaDescriptionList;
	CChangeCreatureHPMsg	_CreatureChangeHPList;
	CChangeCreatureMaxHPMsg	_CreatureChangeMaxHPList;

	/// The emot identifiers
	std::map<std::string, uint32>	_EmotNames;
	CCont<CAIInstance>				_AIInstances;

	uint32	_TotalBotsSpawned;
	bool	_ClientCreatureDebug;
	CTaskList<uint32> _TickedTaskList;
};



#include "ai_inline.h"

#endif
