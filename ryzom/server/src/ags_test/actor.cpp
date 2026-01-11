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


// Nel Misc
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"
#include "nel/misc/aabbox.h"

//#include "combat_interface.h"

// Game share
#include "game_share/tick_event_handler.h"
#include "game_share/synchronised_message.h"
//#include "game_share/msg_brick_service.h"

// Local includes
#include "actor.h"
#include "actor_manager.h"
#include "move_manager.h"
#include "sheets.h"
#include "mirrors.h"
#include "actor_group.h"

using namespace NLMISC;
using namespace NLNET;
using namespace NLPACS;
using namespace std;

namespace AGS_TEST
{

#define DT_TEMP	200

//----------------------------------------------------------------------------------
CActor::CActor(const std::string &type, const std::string &name, const CEntityId &id, bool &success)
{
	_sheetId=type;
// 	if (CSheets::lookup(_sheetId)->Name.empty())
 		_visualName=name;
 //	else
 //		_visualName=CSheets::lookup(_sheetId)->Name;
	_name=name;
	_id=id;
	_activity=SQUARE; //NOTHING;
	_DefaultActivity=UNKNOWN;
	_target=0;
	//_positionChanged=false;

	if ( ! CMirrors::Mirror.createAndDeclareEntity(_id) )
	{
		success = false;
		return;
	}

	_MoveContainer = NULL;
	_MovePrimitive = NULL;
	_Continent = 0xFF;

	// _mode is setup outside the constructor :o(
	_x.init( TheDataset, id, "X" );
	_y.init( TheDataset, id, "Y" );
	_z.init( TheDataset, id, "Z" );
	_angle.init( TheDataset, id, "Theta" );
	_vpa.init( TheDataset, id, "VisualPropertyA" );
	_whoseesme.init( TheDataset, id, "WhoSeesMe" );
	_behaviour.init( TheDataset, id, "Behaviour" );
	_mode.init( TheDataset, id, "Mode" );

	_AttackDistance = 5.0f;
	_PrimZone = NULL;

	SAltLookProp	vpa;
	vpa.Element.Seed = 1;

	_vpa = vpa.Summary;
	_whoseesme = (sint64)(-1);

	_Group = NULL;
	_ChatSet = -1;

	_RespawnCounter = -1;
	_AlreadyAdded = false;
	_CyclesState = STATE_WANDER;


	_Timer.set( DT_TEMP );
	success = true;
}

//
CActor::~CActor()
{
	CMirrors::Mirror.removeEntity(_id);
}

void CActor::removeRefs(CActor *actor)
{
	if (this==0) return;
	if (_target==actor)
		_target=0;
}


//----------------------------------------------------------------------------------
// set the actor's current group
void CActor::setGroup(CActorGroup *group)
{
	if (this==0) return;

	if (_Group != NULL)
	{
		_Group->removeActor(this);
	}

	if (group != NULL)
	{
		group->addActor(this);
	}

	_Group = group;
}

//----------------------------------------------------------------------------------
// displaying the actor's current state
void CActor::display(NLMISC::CLog *log)
{
	if (this==0) return;

	const char *angleNames[]= {"E", "ENE", "NE", "NNE", "N","NNW","NW","WNW","W","WSW","SW","SSW","S","SSE","SE","ESE"};

	log->displayNL("Actor id=%s sheetId='%s'(%08x) name='%s'",
		_id.toString().c_str(),
		_sheetId.toString().c_str(),
		_sheetId.asInt(),
		_name.c_str() );
	log->displayNL("\tpos=(%d,%d,%d) Orientation=%s(%d)",
		_x(),_y(),_z(),
		angleNames[unsigned((_angle()<0.0?-0.5:0.5)+_angle()*8.0/3.14159265359)&15],
		int(_angle()*180.0/3.14159265359));
	log->displayNL("\t attackDist=%f dialogue=%d activity=%s Magnet(%.1f,%.1f,%.1f) MagnetRange=%.1f MagnetDecay=%.1f",
		_AttackDistance,
		_ChatSet,
		getActivityName(),
		_Magnet.x,_Magnet.y,_Magnet.z,
		_MagnetDistance,
		_MagnetDecayDistance);
}

//----------------------------------------------------------------------------------
// script management

void CActor::setActivity(EActivity activity)
{
	if ( this == 0 ) 
		return;

	if ( _DefaultActivity == CActor::UNKNOWN )
		setDefaultActivity( activity );

	switch( activity )
	{
		case NOTHING:	
			doNothing(); 
			break;

		case WANDER:	
			doWander(); 
			break;

		case SQUARE:	
			doSquare(); 
			break;

		case FAUNA_EAT:	
			doEat(); 
			break;

		case FAUNA_SLEEP:	
			doSleep(); 
			break;

		case FIGHT:		
			nlwarning("CActor::setActivity(): can't set activty to FIGHT"); 
			break;

		case ATTACK:
			doAttack();
			break;

		case FLEE:
			doFlee();
			break;


		default:		
			nlwarning("CActor::setActivity(): unknown activity");
	}
}

void CActor::doNothing()
{
	if ( this == 0 )  
		return;

	if ( _activity == FIGHT )	
		stopFight();

	_activity = NOTHING;
}

void CActor::doSquare()
{
	if ( this == 0 )  
		return;

	if ( _activity == FIGHT )	
		stopFight();

	_activity = SQUARE;
}

void CActor::doWander()
{
	if ( this == 0 )  
		return;

	if ( _activity == FIGHT )	
		stopFight();

	_activity = WANDER;
}

void CActor::doEat()
{
	if ( this == 0 ) 
		return;

	if ( _activity == FIGHT )	
		stopFight();

	_activity = FAUNA_EAT;
}

void CActor::doSleep()
{
	if ( this == 0 ) 
		return;

	if ( _activity == FIGHT )	
		stopFight();

	_activity = FAUNA_SLEEP;
}

void CActor::doFight(CActorGroup *target)
{
	if ( this == 0 ) 
		return;

	if ( target->actorCount() == 0 )
		return;

	unsigned i,j=0;
	double mindist= calcDist(*(*target)[0]);
	for (i=1;i<target->actorCount();i++)
		if( calcDist(*(*target)[i]) < mindist)
		{
			mindist=	calcDist(*(*target)[i]);
			j=i;
		}
	doFight((*target)[j]);
}

void CActor::doFight(CActor *target)
{
	if ( this == 0 || target == 0 ) 
		return;

	doFight( target->_id );
}

void CActor::doFight(const NLMISC::CEntityId &id)
{
	if ( this == 0 ) 
		return;
//	if (_activity==FIGHT)	stopFight();

	// turn to face the target

	_activity=FIGHT;
	_target=CActorManager::getActor(id);	// returns 0 if the target not managed here


	nlassert(false);
	// send the message to the rules service
//	CCombatInterface::attack(_id,id,0,true);

	// make sure the target stands still
	if (_target!=0 && !_target->fighting())
		_target->doAttacked(this);
}

void CActor::doAttack()
{
	if ( this == 0 ) 
		return;
/*
	if ( _activity == FIGHT )	
		stopFight();
*/
	_activity = ATTACK;
}

void CActor::doFlee()
{
	if ( this == 0 ) 
		return;

	if ( _activity == FIGHT )	
		stopFight();

	_activity = FLEE;
}



void CActor::doAttacked(CActor *target)
{
	if ( this == 0 ) 
		return;

	if (_activity!=FIGHT)
		doNothing();
}

void CActor::stopFight()
{
	if ( this == 0 ) 
		return;

	if (_activity!=FIGHT)
		return;

	_activity=NOTHING;

	nlassert(false);

	// send a 'engage in combat' message to the brick service
//	CCombatInterface::disengage(_id);
}

bool CActor::update()
{
	if (this==0) 
		return false;

	// generate a position in meters (floating point)
	CVectorD	pos(_x*0.001, _y*0.001, 0.0);
	
	checkRespawn();
	checkTimers();

	switch (_activity)
	{
		case NOTHING:
			break;

		case FAUNA_WAIT:
			if ( checkAttack( pos ) )
				break;

			if ( _CyclesState == STATE_WANDER )
			{
				setActivity( FAUNA_WANDER );
				break;
			}
			else if ( _CyclesState == STATE_VERY_TIRED || _CyclesState == STATE_EXHAUSTED )
			{
				setActivity( FAUNA_SLEEP );
				break;
			}
			else if ( _CyclesState == STATE_VERY_HUNGRY || _CyclesState == STATE_STARVING )
			{
				setActivity( FAUNA_EAT );
				break;
			}
			break;


		case SQUARE:
			if ( checkAttack( pos ) )
				break;

			processSquare();
			break;

		case FIGHT:
			processFight();
			break;

		case WANDER:
			if ( ! computeMove( pos ) )
				return false;
			break;

		case FAUNA_WANDER:
			if ( checkAttack( pos ) )
				break;

			if ( ! computeMove( pos ) )
				return false;

			if ( _CyclesState == STATE_VERY_TIRED || _CyclesState == STATE_EXHAUSTED )
			{
				setActivity( FAUNA_SLEEP );
				break;
			} 
			else if ( _CyclesState == STATE_VERY_HUNGRY || _CyclesState == STATE_STARVING )
			{
				setActivity( FAUNA_EAT );
				break;
			}

			break;

		case FAUNA_SLEEP:
			if ( _CyclesState == STATE_SHAKING )
			{
				setActivity( FAUNA_WAIT );
				break;
			}

			processSleep();
			break;

		case FAUNA_EAT:
			if ( checkAttack( pos ) )
				break;

			if ( _CyclesState == STATE_DIGESTING )
			{
				setActivity( FAUNA_WAIT );
				break;
			}
					
			processEat();
			break;

	}
	return true;
}

//----------------------------------------------------------------------------------
// code requiring network comms
	
// connections to the GPMS
void CActor::addToOtherServices()
{
	if (this==0) return;
/*
	if (!CMirrors::exists(_id))
		CMirrors::Mirror.createAndDeclareEntity( _id );
*/
	// add the actor name to the IOS
	{
		CMessage msgout("CHARACTER_NAME");
		TDataSetRow raw = TheDataset.getDataSetRow( _id );
		msgout.serial( raw );
		ucstring uname( _visualName );
		msgout.serial( uname );
		sendMessageViaMirror( "IOS", msgout );
	}

	_AlreadyAdded = true;
}

void CActor::removeFromOtherServices()
{
	if (this==0) return;

	CMirrors::Mirror.removeEntity( _id );
	_AlreadyAdded = false;
}


// pos management
void CActor::initMovePrimitive(sint32 x, sint32 y, sint32 z)
{
	if (this==0) return;

	// added by ben
	sint	continent = CMoveManager::Continents.findContinent(CVectorD(x*0.001, y*0.001, z*0.001));
	if (continent == -1)
	{
		nlwarning("Unable to spawn %s in any continent! Position (%.1f,%.1f) not valid!", _id.toString().c_str(), x*0.001, y*0.001);
		continent = 0;
	}

	// if a primitive already exists
	if (_MovePrimitive != NULL)
	{
		// previous move container must not be null
		nlassert(_Continent != 0xFF);

		// check the previous move container is different (otherwise does not create a primitive)
		if (continent != _Continent)
		{
			// depending on the type of the previous primitive, creates a copy of it
			UMovePrimitive		*primitive = CMoveManager::Continents.getMoveContainer(continent)->addNonCollisionablePrimitive(_MovePrimitive);
			nlassert(primitive != NULL);

			// removes previous primitive
			CMoveManager::Continents.getMoveContainer(_Continent)->removePrimitive(_MovePrimitive);

			// and set new one as entity primitive
			_MovePrimitive = primitive;
			_Continent = continent;
			_MoveContainer = CMoveManager::Continents.getMoveContainer(_Continent);
		}
	}
	else
	{
		const CSheets::CSheet	*sheet = CSheets::lookup(_sheetId);

		float	primRadius = 0.5f;
		float	primHeight = 2.0f;
		
		if (sheet != NULL)
		{
			primRadius = sheet->Radius;
			primHeight = sheet->Height;
		}

		_Continent = continent;
		_MoveContainer = CMoveManager::Continents.getMoveContainer(_Continent);

		// init pacs primitive
		_MovePrimitive = _MoveContainer->addNonCollisionablePrimitive();
		nlassert( _MovePrimitive != 0 );
		_MovePrimitive->UserData = (uint64) this;
		_MovePrimitive->setPrimitiveType( UMovePrimitive::_2DOrientedCylinder );
		_MovePrimitive->setReactionType( UMovePrimitive::Slide );
		_MovePrimitive->setTriggerType( UMovePrimitive::NotATrigger );
		_MovePrimitive->setCollisionMask( 0x00000000 );
		_MovePrimitive->setOcclusionMask( 0xffffffff );
		_MovePrimitive->setObstacle( false );
		_MovePrimitive->setAbsorbtion( 0 );
		_MovePrimitive->setHeight( primHeight );
		_MovePrimitive->setRadius( primRadius );

		// insert actor in grid as an obstacle
		CObstacle	obstacle;
		obstacle.Id = _id;
		obstacle.Position = CVector(x*0.001f, y*0.001f, 0.0f);
		obstacle.Radius = primRadius;

		_GridIterator = CMoveManager::Grid.insert(obstacle, obstacle.Position);
	}

	setPos(x, y, z);

	_Magnet = CVector(x*0.001f, y*0.001f, 0.0f);
	_MagnetDistance = 50.0f;
	_MagnetDecayDistance = 10.0f;
}

// simple pos setup, pacs jump to position
void CActor::setPos(sint32 x, sint32 y, sint32 z)
{
	if (this==0) return;

	if (_MovePrimitive == NULL || _MoveContainer == NULL)
		initMovePrimitive(x, y, z);

	CVectorD	pos(x*0.001, y*0.001, z*0.001);
	_MovePrimitive->setGlobalPosition( pos, 0 );
	_MovePrimitive->setOrientation( _angle , 0 );
	_MoveContainer->evalCollision(1, 0);				// world image 0 for all
	pos = _MovePrimitive->getFinalPosition( 0 );

	(*_GridIterator).Position = CVector(pos);
	CMoveManager::Grid.move(_GridIterator, (*_GridIterator).Position);

	_x = (sint32)(1000*pos.x);
	_y = (sint32)(1000*pos.y);
	_z = (sint32)(1000*pos.z);
	//_positionChanged=true;
}

// some complex movement - involving GPMS update
void CActor::moveBy(sint32 x, sint32 y, sint32 z)
{
	if (this==0) return;
	if ((x|y|z)==0) return;

	// modif by ben
	//setPos(_x+x, _y+y, _z+z);
	//_positionChanged=true;

	if (_MovePrimitive == NULL || _MoveContainer == NULL)
	{
		nlwarning("Can't moveBy(%d,%d,%d) actor %s, _MovePrimitive or _MoveContainer not set", x, y, z, _id.toString().c_str());
		return;
	}

	_MovePrimitive->move(CVectorD(x*0.001, y*0.001, 0.0), 0);
	_MoveContainer->evalNCPrimitiveCollision(1.0, _MovePrimitive, 0);
	CVectorD	pos = _MovePrimitive->getFinalPosition( 0 );

	(*_GridIterator).Position = CVector(pos);
	CMoveManager::Grid.move(_GridIterator, (*_GridIterator).Position);

	_x = (sint32)(1000*pos.x);
	_y = (sint32)(1000*pos.y);
	_z = (sint32)(1000*pos.z);
	//_positionChanged=true;
}

/*
void CActor::addPositionChangesToMessage(CMessage &msg)
{
	if (this==0) return;
	if (!_positionChanged) return;

	msg.serial( _id );
	msg.serial( _x );
	msg.serial( _y );
	msg.serial( _z );
	msg.serial( _angle );
	NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle()-1;
	msg.serial( tick );

	_positionChanged=false;
}
*/

void CActor::teleportTo(sint32 x, sint32 y, sint32 z)
{
	if (this==0) return;

	setPos(x, y, z);
/*
	// added/modified by ben
	initMovePrimitive(x, y, z);

	// displaced by ben
	CMessage msgout("ENTITY_TELEPORTATION");
	msgout.serial( _id );
	msgout.serial( _x );
	msgout.serial( _y );
	msgout.serial( _z );
	msgout.serial( _angle );
	NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle()-1;
	msgout.serial( tick );
	sendMessageViaMirror( "GPMS", msgout );

	//_positionChanged=false;
*/
}


//----------------------------------------------------------------------------------
// setting the actor behaviour

void CActor::setBehaviour(std::string behaviourName)
{
	if (this==0) return;
	if (behaviourName.empty()) return;
/*
	MBEHAV::EBehaviour behaviour=MBEHAV::stringToBehaviour(behaviourName);
	CMessage msgout("SET_BEHAVIOUR");
	msgout.serial( _id );
	MBEHAV::CBehaviour b = behaviour;
	msgout.serial( b );
	sendMessageViaMirror( "EGS", msgout );
*/
	MBEHAV::EBehaviour behaviour=MBEHAV::stringToBehaviour(behaviourName);
	MBEHAV::CBehaviour b = behaviour;

	_behaviour = (uint64)b;
}
/*
// setting the actor mode
void CActor::setMode(std::string modeName)
{
	if ( this == 0 ) 
		return;
	if ( modeName.empty() ) 
		return;

	MBEHAV::EMode mode = MBEHAV::stringToMode( modeName );
	CMessage msgout("SET_MODE");
	msgout.serial( _id );
	MBEHAV::EMode b = mode;
	msgout.serialEnum( b );
	sendMessageViaMirror( "EGS", msgout );
}

void CActor::setMode(MBEHAV::EMode mode)
{
	if ( this == 0 ) 
		return;

	CMessage msgout("SET_MODE");
	msgout.serial( _id );
	msgout.serialEnum( mode );
	sendMessageViaMirror( "EGS", msgout );
}
*/



//----------------------------------------------------------------------------------
// setting the objects carried in the actor's hands

void CActor::setVPA(const SAltLookProp &vpa)
{
	_vpa = vpa.Summary;
}

void CActor::setLeftHand(NLMISC::CSheetId sheetId)
{
	if (this==0) return;
}

void CActor::setRightHand(NLMISC::CSheetId sheetId)
{
	if (this==0) return;
}


void CActor::checkRespawn()
{

	// if actor doesn't exist in mirror and has not yet been added to gpms, spawn in and set as wandering
	if (!CMirrors::exists(_id) && !_AlreadyAdded)
	{
		setPos((sint32)(_Magnet.x*1000), (sint32)(_Magnet.y*1000), (sint32)(_Magnet.z*1000));
		addToOtherServices();
		//setMode( MBEHAV::NORMAL );

		setActivity(_DefaultActivity);	// doWander();
	}

	// if actor is dead and was on gpms previously then:
	// 1. initialise counter so that actor will respawn when counter reaches 0
	// 2. remove actor from gpms, and lock respawn counter to 0
	if (CMirrors::mode(_id) == MBEHAV::DEATH && _AlreadyAdded)
	{
		if (_RespawnCounter<0)
		{
			_RespawnCounter = (int)((_MinSpawnDelay + (_MaxSpawnDelay-_MinSpawnDelay) * (float)(((double)rand()-RAND_MAX/2)/(float)(RAND_MAX/2)))*10.0f);
			nlinfo("Actor dies %s, will be respawned soon...", _id.toString().c_str());
			doNothing();
		}
		else if (_RespawnCounter == 1)
		{
			nlinfo("Dead Actor %s removed, will be respawned soon...", _id.toString().c_str());
			// remove agent from gpms
			removeFromOtherServices();
			--_RespawnCounter;
		}
		else if (_RespawnCounter > 0)
		{
			--_RespawnCounter;
		}
	}
	else
	{
		_RespawnCounter = -1;
	}
}

bool CActor::processSquare()
{
/*		if (_mode().Mode!=MBEHAV::NORMAL)
			return false;
*/
		sint32 speed=sint32(100.0*CSheets::lookup(_sheetId)->WalkSpeed);
		sint32 key= (sint32)CTickEventHandler::getGameCycle()-1;
		key>>=5;
		// key+=(sint32)_sid.Id;	// this line makes the actors move out of sync with eachother
		moveBy((key&1)*((key&2)-1)*speed,(1-(key&1))*((key&2)-1)*speed,0); // this is a grid
		//sint32 keyx=key>>4;
		//sint32 keyy=keyx+2;
		//moveBy( (keyx&4?speed:-speed)*(keyx&3==3?0:1), (keyy&4?speed:-speed)*(keyy&3==3?0:1), 0 ); // this is an octagon
		_angle = (float)::atan2((1-(key&1))*((key&2)-1)*speed, (key&1)*((key&2)-1)*speed);
		return true;
}


bool CActor::processFight()
{
/*		if (_mode().Mode!=MBEHAV::COMBAT)
			return false;
		{
			sint32 key= (sint32)CTickEventHandler::getGameCycle()-1;
			key%=200; 
			if (!key && _target)
			{
				nlassert(false);
//				CCombatInterface::attack(_id,_target->_id,1,false);
			}

			// if we have position info for the target then setup orientation info
			if (_target)
			{
				CMoveManager::TObstacleMap::iterator	it;
				it = CMoveManager::ObstacleMap.find(_target->_id);
				if (it != CMoveManager::ObstacleMap.end())
				{
					NLMISC::CVectorD pos(float(_x)/1000.0f,float(_y)/1000.0f,float(_z)/1000.0f);
					pos -= (*((*it).second)).Position;
					setAngle((float)atan2(pos.y,pos.x));
				}
			}
		}
*/
		//todo:
			// check hitpoints in georges
			// check regen rate in george (should be: rate=1000 value=0)
			// subscribe to hitpoint mirror (sint16)
			// add group management to command to engage in combat

		// if target dead go to idle

		// from time to time insert a 'coup en force' attack
			// CBSExecuteMsg msg;
			// msg.PlayerId=_sid;
			// msg.TargetId=_target;
			// msg.Bricks.push_back(CSheetId("coup_en_force.brick_base"));
			// msg.send("BS");
		return true;
}

bool CActor::processEat()
{
/*
	if ( getMode() != MBEHAV::EAT )
		setMode( MBEHAV::EAT );
*/	return true;
}

bool CActor::processSleep()
{
/*	if ( getMode() != MBEHAV::REST )
		setMode( MBEHAV::REST );
*/	return true;
}


bool CActor::checkAttack( CVectorD &pos )
{
	/*
		if we're close to an actor then attack them
	*/
	CAABBox	box;
	box.setCenter(CVector(_x*0.001f, _y*0.001f, 0.0f));
	box.setHalfSize(CVector(5.0f, 5.0f, 1.0f));

	CMoveManager::Grid.clearSelection();
	CMoveManager::Grid.select(box);

	CMoveManager::TObstacleGrid::CIterator	it;
	for (it=CMoveManager::Grid.begin(); it!=CMoveManager::Grid.end(); ++it)
	{
		CObstacle		&other = (*it);

		// attacks player if close to me
		CVector	d = pos-other.Position;
		d.z = 0.0f;
		if (other.Id.getType() == RYZOMID::player && d.norm() <= _AttackDistance)
		{
			nlinfo("Actor <%s> attacks player because _AttackDistance = %f and separation = %f", _name.c_str(), _AttackDistance, d.norm());
			doFight(other.Id);
			return true;
		}
	}
	return false;
}


// State machine defining transitions between cycle states
void CActor::updateCycles()
{
	_CyclesState = (ECycleState) ( ( (int)_CyclesState ) + 1 );
	if ( _CyclesState == STATE_LAST )
		_CyclesState = STATE_WANDER;
}

/*
void CActor::updateCycles()
{
	switch( _CyclesState )
	{
		case STATE_WANDER:
			_CyclesState = STATE_HUNGRY;
			setActivity( WANDER );
			break;

		case STATE_HUNGRY:
			_CyclesState = STATE_VERY_HUNGRY;
			break;

		case STATE_VERY_HUNGRY:
			_CyclesState = STATE_STARVING;
			break;

		case STATE_STARVING:
			_CyclesState = STATE_EATING;
			break;

		case STATE_EATING:
			{
				_CyclesState = STATE_HUNGRY;
				setActivity( EAT );
			}
			break;

		case STATE_DIGESTING:
			_CyclesState = STATE_TIRED;
			setActivity( WANDER );
			break;

		case STATE_TIRED:
			_CyclesState = STATE_VERY_TIRED;
			break;

		case STATE_VERY_TIRED:
			_CyclesState = STATE_EXHAUSTED;
			break;

		case STATE_EXHAUSTED:
			_CyclesState = STATE_SLEEPING;
			break;

		case STATE_SLEEPING:
			{
				_CyclesState = STATE_HUNGRY;
				setActivity( SLEEP );
			}
			break;

		case STATE_SHAKING:
			{
				_CyclesState = STATE_SHAKING;
				setActivity( WANDER );
			}
			break;
	}
}
*/
void CActor::checkTimers()
{
	if ( _Timer.test() )
	{
		updateCycles();
		_Timer.add( DT_TEMP );
	}
}




bool CActor::computeMove( NLMISC::CVectorD &pos)
{
	// use the current tick and the entity's id to decide when to move and when to stand still
	// the objective is to move in short bursts, standing still between times
	sint32 key= (sint32)CTickEventHandler::getGameCycle()-1;
	key>>=2;
	key>>=3;
	key+=(sint32)_id.getShortId();	// this line makes the actors move out of sync with eachother
	if (!(key & 3))
	{
		// we're not moving but we can turn round
		sint32 lastkey= (sint32)CTickEventHandler::getGameCycle()-2;
		lastkey>>=2;
		lastkey+=(sint32)_id.getShortId();	// this line makes the actors move out of sync with eachother
		lastkey>>=3;
		if (!(lastkey & 1))
			_angle = _angle() + (float)(((double)rand()-RAND_MAX/2)/(float)(RAND_MAX/2)*3.14159265359f/2.0f);

		return true;
	}

	// lookup my walk and run speeds and so on from my sheetid
	const CSheets::CSheet	*sheet = CSheets::lookup(_sheetId);
	if (!sheet)
	{
//		nlwarning("Unknown sheet id for entity %s",_Id.toString().c_str());
		return false;
	}
	float	walkSpeed = sheet->WalkSpeed;
	float	runSpeed = sheet->RunSpeed;
	float	radius = sheet->Radius;

	double	dt = (double)(CTickEventHandler::getGameTimeStep());
	CVectorD	move = CVectorD(0.0, 0.0, 0.0);

	// randomly change direction
	key= (sint32)CTickEventHandler::getGameCycle()-1;
	key+=(sint32)_id.getShortId();
	if ((key & 3) == 3)
	{
//		_angle += (float)((3.14159265359f/8.0f)*sin((2.0f*3.14159265359f/360.0f)*(float)((CTickEventHandler::getGameCycle()+7*_id.getShortId())%360)));
		_angle = _angle() + (float)(((double)rand()-RAND_MAX/2)/(RAND_MAX/2)*5.7*dt);
		while (_angle() > +3.14159265359f) _angle = _angle() - 2.0f*3.14159265359f;
		while (_angle() < -3.14159265359f) _angle = _angle() + 2.0f*3.14159265359f;
	}
	move.x = walkSpeed*cos(_angle)*dt;
	move.y = walkSpeed*sin(_angle)*dt;

	// some attraction by the magnet
	CVectorD	dmagnet = _Magnet-pos;
	dmagnet.z = 0.0;
	double		nmagnet = dmagnet.norm();

	if (nmagnet > _MagnetDistance)
	{
		if (_MagnetDecayDistance<1) _MagnetDecayDistance=1; // big bad anti-bug
		move += dmagnet.normed()*walkSpeed*dt*(nmagnet-_MagnetDistance)/_MagnetDecayDistance;
	}

	// avoid obstacles
	CVectorD	avoid = CVectorD::Null;

	CMoveManager::TObstacleGrid::CIterator	it;
	for (it=CMoveManager::Grid.begin(); it!=CMoveManager::Grid.end(); ++it)
	{
		CObstacle		&other = (*it);

		// don't avoid self
		if (other.Id == _id)
			continue;

		CVectorD		d = other.Position-pos;
		d.z = 0.0;
		double			dist = d.norm()+0.001;

		const float	EntitiesRepulsSpacing = 1.0f;
		float		EntitiesRepulsDistance = EntitiesRepulsSpacing+radius+other.Radius;
		const float	EntitiesRepulsStrength = 0.5f;

		if (dist < EntitiesRepulsDistance)
		{
			double		coef = 1.0f - (float)(pow(EntitiesRepulsDistance, EntitiesRepulsStrength)/pow(dist, EntitiesRepulsStrength));
			avoid += d*(walkSpeed*dt*coef/dist);
		}
	}

	move += avoid;

	CVectorD	front = move.normed();
	CVectorD	lateral(front.y, -front.x, 0.0);
	CVectorD	normal;

	// avoid walls
	if (!_MoveContainer->testMove(_MovePrimitive, front, 1, 0, &normal))
		move += normal*walkSpeed*0.3*dt;
	if (!_MoveContainer->testMove(_MovePrimitive, (front+lateral)*0.5, 1, 0, &normal))
		move += normal*walkSpeed*0.3*dt;
	if (!_MoveContainer->testMove(_MovePrimitive, (front+lateral)*0.5, 1, 0, &normal))
		move += normal*walkSpeed*0.3*dt;

	// avoid patat borders
	if (!testPositionInPatat(pos+front))
	{
		uint	j, n=4;
		CVector	newPos;
		for (j=1; j<n; ++j)
		{
			newPos = pos+front*cos(j*3.1415926535/n)+lateral*sin(j*3.1415926535/n);
			if (testPositionInPatat(newPos))
				return true;
			newPos = pos+front*cos(j*3.1415926535/n)-lateral*sin(j*3.1415926535/n);
			if (testPositionInPatat(newPos))
				return true;
		}

		move = newPos-pos;
		front = move.normed();
	}

	// compute real move, and use this position as the next way point
	_angle = (float)::atan2(front.y, front.x);
	double	movenorm = move.norm();
	if (movenorm > dt*runSpeed)
		move *= dt*runSpeed/movenorm;

	//setup the angle field
	_angle = (float)::atan2(move.y, move.x);

	moveBy((sint32)(move.x*1000.0), (sint32)(move.y*1000.0), 0);
	return true;
}

} // end of namespace AGS_TEST

/*
		NLMISC::CVectorD posE = e->getExtrapoledPosition();
		agressivite = 0.0;
		instinct = 0.0;
		TCarrieerCombat carriere = getProfile(c);
		
		r = getEnnemiFar(posE, c->getCoordinate());		

		double fameCoputed = (double)f;


		if(fameCoputed >= 0)
							return NOCombat;
		agressivite = -fameCoputed;
		double l = getLevelPoint(c) - getLevelPoint(e->getClass());
		
		agressivite += (l>0.0) ? l*20 : 0.0;

		agressivite += getProfileAvantage(carriere, e);

		if ( r < 3) agressivite += 20;

		instinct += (l<0.0) ? -l*20 : 0.0;					
		double l100 = getNormedLifePoint(c);
		if( l100 < 0.5 )
		{
			if( l100 < 0.25 )
				instinct += 60;
			else
				instinct += 40;
		}

		double seuil = agressivite - instinct;
		if(seuil >= getSeuilBelliqueux(c))
		{
			if(r < _REngage) return Attack;
			else
			{
				  return Poursuivre;
			}
		}
		else
		{
			double fuite = getSeuilFuite(c);
			if(instinct >= fuite)
				return Fuite;
			else
				return NOCombat;

		}		
		return NOCombat;
*/
