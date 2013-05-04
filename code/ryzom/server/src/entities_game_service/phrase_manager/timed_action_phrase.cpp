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
// game share
#include "game_share/brick_families.h"
#include "game_share/string_manager_sender.h"
//
#include "timed_action_phrase.h"
#include "s_phrase_factory.h"
#include "phrase_manager/s_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "egs_globals.h"
#include "egs_sheets/egs_sheets.h"
//


DEFAULT_SPHRASE_FACTORY( CTimedActionPhrase, BRICK_TYPE::TIMED_ACTION );

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;

//-----------------------------------------------
// ctor
//-----------------------------------------------
CTimedActionPhrase::CTimedActionPhrase()
{
	_IsStatic = true;
	_TimedAction = NULL;
	_ExecutionDuration = 0;
	_PhraseType = BRICK_TYPE::TIMED_ACTION;
	_ActionType = CLIENT_ACTION_TYPE::None;
}

//-----------------------------------------------
// dtor
//-----------------------------------------------
CTimedActionPhrase::~CTimedActionPhrase()
{
	if (_TimedAction != NULL)
	{
		delete _TimedAction;
		_TimedAction = NULL;
	}
}

//-----------------------------------------------
// CTimedActionPhrase build
//-----------------------------------------------
bool CTimedActionPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute )
{
	_ActorRowId = actorRowId;

	for (uint i = 0 ; i < bricks.size() ; ++i)
	{
		const CStaticBrick *brick = bricks[i];
		nlassert(brick != NULL);

		if (i==0)
			_RootSheetId = brick->SheetId;

		// process params
		for ( uint j = 0 ; j < brick->Params.size() ; ++j)
		{
			TBrickParam::IId	*param = brick->Params[j];
			if (!param) continue;

			switch(param->id())
			{
			case TBrickParam::TA_TELEPORT:
				{
					_TimedAction = new CTPTimedAction();
					if (_TimedAction == NULL)
					{
						nlwarning("Error allocating new CTPTimedAction object");
						return false;
					}
					_ExecutionDuration = DelayBeforeItemTP;
					_ActionType = CLIENT_ACTION_TYPE::Teleport;
				}
				break;

			case TBrickParam::TA_DISCONNECT:
				{
					_TimedAction = new CDisconnectTimedAction();
					if (_TimedAction == NULL)
					{
						nlwarning("Error allocating new CTPTimedAction object");
						return false;
					}

					_ExecutionDuration = TimeBeforeDisconnection;
					if (IsRingShard)
					{
						// Find out how much time to wait depending on the role of the character
						CCharacter* player = PlayerManager.getChar(_ActorRowId);
						if (player)
						{
							// In Ring edition and animation mode, take a short cut when Far Teleporting
							R2::TUserRole role = player->sessionUserRole();
							if ((role == R2::TUserRole::ur_editor) || (role == R2::TUserRole::ur_animator))
								_ExecutionDuration = 1;
						}
					}
					_ActionType = CLIENT_ACTION_TYPE::Disconnect;
				}
				break;

			case TBrickParam::TA_MOUNT:
				{
					_TimedAction = new CMountTimedAction();
					if (_TimedAction == NULL)
					{
						nlwarning("Error allocating new CTPTimedAction object");
						return false;
					}
					_ExecutionDuration = MountDuration;
					_ActionType = CLIENT_ACTION_TYPE::Mount;
				}
				break;

			case TBrickParam::TA_UNMOUNT:
				{
					_TimedAction = new CUnmountTimedAction();
					if (_TimedAction == NULL)
					{
						nlwarning("Error allocating new CTPTimedAction object");
						return false;
					}
					_ExecutionDuration = UnmountDuration;
					_ActionType = CLIENT_ACTION_TYPE::Unmount;
				}
				break;

			case TBrickParam::TA_CONSUME:
				{
					_TimedAction = new CConsumeItemTimedAction();
					if (_TimedAction == NULL)
					{
						nlwarning("Error allocating new CConsumeItemTimedAction object");
						return false;
					}
					// get item to consume to init consumption time
					CCharacter *player = PlayerManager.getChar(actorRowId);
					if (player)
					{
						CGameItemPtr item =player->getConsumedItem();
						if (item != NULL)
						{
							const CStaticItem *form = item->getStaticForm();
							if (form && form->ConsumableItem)
							{
								_ExecutionDuration = TGameCycle(form->ConsumableItem->ConsumptionTime / CTickEventHandler::getGameTimeStep());
							}
						}
					}
					_ActionType = CLIENT_ACTION_TYPE::ConsumeItem;
				}
				break;

			default:;
			};
		}
	}

	return true;
}// CTimedActionPhrase build


//-----------------------------------------------
// CTimedActionPhrase evaluate
//-----------------------------------------------
bool CTimedActionPhrase::evaluate()
{
	return true;
}// CTimedActionPhrase evaluate


//-----------------------------------------------
// CTimedActionPhrase validate
//-----------------------------------------------
bool CTimedActionPhrase::validate()
{
	string errorCode;
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_ActorRowId);
	if (!entity)
		return false;

	if (_TimedAction != NULL)
		return _TimedAction->validate(this, entity);

	return true;
}// CTimedActionPhrase validate


//-----------------------------------------------
// CTimedActionPhrase update
//-----------------------------------------------
bool  CTimedActionPhrase::update()
{
	// nothing to do
	return true;
}// CTimedActionPhrase update


//-----------------------------------------------
// CTimedActionPhrase execute
//-----------------------------------------------
void  CTimedActionPhrase::execute()
{
	_ExecutionEndDate = CTickEventHandler::getGameCycle() + _ExecutionDuration;

	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		if (_IsStatic)
		{
			switch( _ActionType )
			{
				case CLIENT_ACTION_TYPE::Mount :
					player->staticActionInProgress(true, STATIC_ACT_TYPES::Mount);
					break;
				default:
					player->staticActionInProgress(true, STATIC_ACT_TYPES::Teleport);
			}
		}
		else
			player->cancelStaticActionInProgress();
		
		player->setCurrentAction(_ActionType, _ExecutionEndDate);
		if (_RootSheetId != CSheetId::Unknown)
		{
//			player->_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", _RootSheetId.asInt() );
			CBankAccessor_PLR::getEXECUTE_PHRASE().setSHEET(player->_PropertyDatabase, _RootSheetId);
//			player->_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", 0 );
			CBankAccessor_PLR::getEXECUTE_PHRASE().setPHRASE(player->_PropertyDatabase, 0);
		}
	}
}// CTimedActionPhrase execute


//-----------------------------------------------
// CTimedActionPhrase launch
//-----------------------------------------------
bool CTimedActionPhrase::launch()
{
	// apply immediatly
	_ApplyDate = 0;
	return true;
}//CTimedActionPhrase launch


//-----------------------------------------------
// CTimedActionPhrase apply
//-----------------------------------------------
void CTimedActionPhrase::apply()
{
	// keep a ptr on this to prevent it to be deleted when timed action is deconnection
	CSPhrasePtr selfPtr = this;

	_LatencyEndDate = 0;

	CEntityBase *actor = CEntityBaseManager::getEntityBasePtr(_ActorRowId);
	if (!actor)
		return;

	if (_TimedAction != NULL)
		_TimedAction->applyAction(this, actor);

}//CTimedActionPhrase apply


//-----------------------------------------------
// CTimedActionPhrase end
//-----------------------------------------------
void CTimedActionPhrase::end()
{
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->clearCurrentAction();
	}
} // end //

//-----------------------------------------------
// CTimedActionPhrase stop
//-----------------------------------------------
void CTimedActionPhrase::stop()
{
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (player)
	{
		player->clearCurrentAction();
		if (_TimedAction != NULL)
			_TimedAction->stopAction(this, player);
	}
} // stop //

//-----------------------------------------------
// CTimedActionPhrase stopBeforeExecution
//-----------------------------------------------
void CTimedActionPhrase::stopBeforeExecution()
{
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if ( player != NULL && _TimedAction != NULL)
		_TimedAction->stopBeforeExecution(this, player);
} // stopBeforeExecution //

//-----------------------------------------------
bool CTimedActionPhrase::testCancelOnHit( sint32 attackSkillValue, CEntityBase * entity, CEntityBase * defender)
{
	if (_TimedAction != NULL)
		return _TimedAction->testCancelOnHit(attackSkillValue, entity, defender);

	return false;	
}
