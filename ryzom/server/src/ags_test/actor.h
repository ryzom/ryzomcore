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


#ifndef GD_ACTOR_H
#define GD_ACTOR_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/vectord.h"

// Nel Pacs
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_move_container.h"
// Nel Ligo
#include "nel/ligo/primitive.h"


// Game share
#include "game_share/mode_and_behaviour.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/player_visual_properties.h"

// AgS test
#include "move_manager.h"
#include "ags_timer.h"
#include "mirrors.h"

namespace AGS_TEST
{
// external refs
class CActorGroup;

// the class
class CActor
{
public:
	enum EActivity { 
		UNKNOWN=-1, 
		NOTHING=0, 
		SQUARE, 
		FIGHT, 
		WANDER,
		FAUNA_WANDER,
		FAUNA_WAIT, 
		FAUNA_EAT,
		FAUNA_SLEEP,
		ATTACK,
		FLEE
	};

	/*
	
	enum ECycleState
	{
		STATE_WANDER,
		STATE_HUNGRY,
		STATE_VERY_HUNGRY,
		STATE_STARVING,
		STATE_EATING,
		STATE_DIGESTING,
		STATE_TIRED,
		STATE_VERY_TIRED,
		STATE_EXHAUSTED,
		STATE_SLEEPING,
		STATE_SHAKING
	};
*/

	enum ECycleState
	{
		STATE_WANDER,
		STATE_HUNGRY,
		STATE_VERY_HUNGRY,
		STATE_STARVING,
		STATE_DIGESTING,
		STATE_TIRED,
		STATE_VERY_TIRED,
		STATE_EXHAUSTED,
		STATE_SHAKING,
		STATE_LAST
	};

	// The only legal constructor
	CActor(const std::string &type, const std::string &name, const NLMISC::CEntityId &id, bool &success);

	//
	~CActor();

	// Removing x-refs to other actors if they are deleted
	void			removeRefs(CActor *actor);

	// some basic read accessors
	std::string		getName()									{ return _name; }
	std::string		getType()									{ return _sheetId.toString(); }
	sint32			getSheetID()								{ return _sheetId.asInt(); }
	const NLMISC::CEntityId &getSid()							{ return _id; }

	sint32			getX()										{ return _x(); }
	sint32			getY()										{ return _y(); }
	sint32			getZ()										{ return _z(); }
	float			getAngle()									{ return _angle(); }
//	uint8			getMode()									{ return (uint8)(_mode().Mode); }
//	void			initMode()									{ _mode.init( TheDataset, _id, "Mode" ); }

	//bool positionChanged()				{ return _positionChanged; }
	
	// some basic write accessors
	void			setPos(sint32 x, sint32 y, sint32 z);		// { _x=x; _y=y; _z=z; _positionChanged=true; }	// commented by ben, uses pacs now
	void			setAngle(float angle)						{ _angle=angle; /*_positionChanged=true;*/ }
	void			setMagnetPos(const NLMISC::CVector &pos)	{ _Magnet = pos; }
	void			setMagnetRange(float range,float decay)		{ _MagnetDistance=range; _MagnetDecayDistance=decay; }
	void			setSpawnTime(float min,float max)			{ _MinSpawnDelay=min; _MaxSpawnDelay=max; }
	void			setAttackDistance(float distance)			
	{
		_AttackDistance = distance;
		nlinfo("Set attack distance for <%s> to %f meters",_name.c_str(),_AttackDistance);
	}
	void			setPrimZone(NLLIGO::CPrimZone *primZone)	{ _PrimZone = primZone; }
	void			setBehaviour(std::string behaviourName);
/*	void			setMode(std::string behaviourName);
	void			setMode(MBEHAV::EMode mode);*/
	void			setLeftHand(NLMISC::CSheetId sheetId);
	void			setRightHand(NLMISC::CSheetId sheetId);
	void			setGroup(CActorGroup *group);
	void			setVPA(const SAltLookProp &vpa);
	void			setActivity(EActivity activity);
	void			setDefaultActivity(EActivity activity)		{ _DefaultActivity = activity; }
	void			setChatSet(sint32 set)						{ _ChatSet = set; }
	sint32			getChatSet() const							{ return _ChatSet; }

	// connections to the other game services
	void			addToOtherServices();
	void			removeFromOtherServices();

	// the following used to connect to a specific service identified by service ID
	//void addToGPMS(uint8 serviceID);
	//void addToIOS(uint8 serviceID);

	// some complex movement - involving GPMS update
	void			initMovePrimitive(sint32 x, sint32 y, sint32 z);
	void			moveBy(sint32 x, sint32 y, sint32 z);
	//void addPositionChangesToMessage(NLNET::CMessage &msg);
	void			teleportTo(sint32 x, sint32 y, sint32 z);

	// displaying the actor's current state
	void			display(NLMISC::CLog *log = NLMISC::InfoLog);
	const char		*getActivityName()							{ switch(_activity) { case FIGHT: return "FIGHT"; case NOTHING: return "IDLE"; case SQUARE: return "SQUARE"; case WANDER: return "WANDER"; } return ""; }

	// changeing the actor's activity
	void			resetActivity()								{ setActivity(_DefaultActivity); }
	void			doNothing();
	void			doSquare();
	void			doWander();
	void			doEat();
	void			doSleep();
	void			doFight(CActor *target);
	void			doFight(const NLMISC::CEntityId &id);
	void			doFight(CActorGroup *target);
	void			doAttacked(CActor *target);
	void			doAttack();
	void			doFlee();
	bool			fighting()									{ return _activity==FIGHT; }

	void			stopFight();

	void			checkRespawn();
	bool			checkAttack( NLMISC::CVectorD & );

	
	// Functions called at each update depending on the current _activity
	bool			processSquare();	
	bool			processFight();
	bool			processEat();
	bool			processSleep();

	bool			computeMove( NLMISC::CVectorD &pos);
	bool			computeFlee( NLMISC::CVectorD &pos);

	// the update routine that moves the actor and does other stuff
	// returns false if the actor is dead, remote controled, etc
	bool			update();

	// script management
/*
	static void		addScript(std::string name, std::vector<std::string> args, sint32 firstArg);
	void			assignScript(std::string name);
*/

	// utility functions
	double			calcDist(CActor &target)					{ return (_x-target.getX()) * (_x-target.getX()) + (_y-target.getY()) * (_y-target.getY()); }
	bool			testPositionInPatat(const NLMISC::CVector &pos) { return (_PrimZone) ? _PrimZone->contains(pos) : true; }	// if no prim zone, always true

//private:
	CActor();

	// basic identity
	NLMISC::CEntityId _id;
	NLMISC::CSheetId _sheetId;
	std::string _name;
	std::string _visualName;


	// Eat and rest cycles 
	CAGSTimer _Timer;				// Timer for eat and sleep cycles ( synchronized with TICK service)
	ECycleState _CyclesState;		// Current eat and sleep cycles
	void updateCycles();			// State machine computing transitions between timer states (eat and sleep)
	void checkTimers();				// called at each update to see if the last timer delay has expired (if true calls updateCycles)



	// position
	CMirrorPropValue1DS<TYPE_POSX>			_x;
	CMirrorPropValue1DS<TYPE_POSY>			_y;
	CMirrorPropValue1DS<TYPE_POSZ>			_z;
	CMirrorPropValue1DS<TYPE_ORIENTATION>	_angle;
	CMirrorPropValue1DS<TYPE_VPA>			_vpa;
	CMirrorPropValue1DS<TYPE_WHO_SEES_ME>	_whoseesme;
	CMirrorPropValue1DS<TYPE_BEHAVIOUR>		_behaviour;
	CMirrorPropValue1DS<TYPE_MODE>			_mode;
	//bool							_positionChanged;	// to know whether need to send position to GPS

	// properties used to govern the update() code
	EActivity _activity;				// behavioural activity
	EActivity _DefaultActivity;			// the activity we were firt attributed and return to when nothing better to do
	CActor *_target;					// target for combat

	// continent the entity spawned in
	uint8					_Continent;

	// pacs move container for the actor
	NLPACS::UMoveContainer	*_MoveContainer;

	// pacs primitive for the actor
	NLPACS::UMovePrimitive	*_MovePrimitive;

	// iterator for the move grid
	CMoveManager::TObstacleGrid::CIterator	_GridIterator;

	// magnet
	NLMISC::CVector			_Magnet;
	float					_MagnetDistance;
	float					_MagnetDecayDistance;

	// attack distance
	float					_AttackDistance;

	// prim zone
	NLLIGO::CPrimZone*		_PrimZone;

	// actor group
	CActorGroup*			_Group;

	// bot page set
	sint32					_ChatSet;

	// respawn counter
	sint32					_RespawnCounter;
	float					_MinSpawnDelay;
	float					_MaxSpawnDelay;

	// added on services
	bool					_AlreadyAdded;


};

} // end of namespace AGS_TEST

#endif // GD_ACTOR_H
