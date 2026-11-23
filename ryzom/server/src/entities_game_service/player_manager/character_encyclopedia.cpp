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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_encyclo.h"
#include "game_share/msg_encyclopedia.h"
#include "game_share/string_manager_sender.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "mission_manager/mission_manager.h"
#include "player_manager/character_encyclopedia.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CCharacterEncyclopedia);

//-----------------------------------------------------------------------------
// methods CCharacterEncyclopedia
//-----------------------------------------------------------------------------

CCharacterEncyclopedia::CCharacterEncyclopedia(CCharacter &c) : _Char(c), _SentToClient(false)
{
	init();
}

//-----------------------------------------------------------------------------

/// The mission of the rite (0) or of a task (1 and up) of a thema, NULL if there is none
static CMissionTemplate *getRiteTaskTemplate(const CStaticEncycloThema &thema, uint32 riteTask)
{
	if (riteTask > thema.Tasks.size())
		return NULL;
	const string &name = riteTask == 0 ? thema.Rite : thema.Tasks[riteTask - 1];
	return CMissionManager::getInstance()->getTemplate(CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(name));
}

/// The bot that the encyclopedia names for a mission, and the id of its name. false while they cannot be given: the
/// bot is not there (the AI service is down or starts, the missions were just built again), or it has no name yet.
static bool getEncyclopediaNpc(const CMissionTemplate &templ, TDataSetRow &giver, uint32 &npcName)
{
	npcName = 0;
	giver = TheDataset.getDataSetRow(CAIAliasTranslator::getInstance()->findEntityId(templ.EncycloNPC));
	if (giver.isNull())
		return false;
	CMirrorPropValueRO<TYPE_NAME_STRING_ID> nameId(TheDataset, giver, DSPropertyNAME_STRING_ID);
	npcName = nameId;
	return npcName != 0;
}

//-----------------------------------------------------------------------------

void CCharacterEncyclopedia::init()
{
	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	_EncyCharAlbums.resize( rSE.getNbAlbum() );
	for (uint32 i = 0; i < rSE.getNbAlbum(); ++i)
		_EncyCharAlbums[i].Themas.resize( rSE.getNbThema(i) );
}

//-----------------------------------------------------------------------------

void CCharacterEncyclopedia::clear()
{
	_EncyCharAlbums.clear();
}

//-----------------------------------------------------------------------------

void CCharacterEncyclopedia::updateTask(uint32 nAlbum, uint32 nThema, uint32 nTask, uint8 nNewState, bool bSendMsg)
{
	nlassert(nAlbum < _EncyCharAlbums.size());
	nlassert(nThema > 0); // Thema must start at 1
	nThema = nThema - 1;
	nlassert(nThema < _EncyCharAlbums[nAlbum].Themas.size());
	nlassert(nTask < 8);

	// If album or thema already completed there is some kind of problem mission should not be accessible anymore
	if (_EncyCharAlbums[nAlbum].AlbumState == 2)
	{
		nlwarning("album already completed album:%d thema:%d task:%d", nAlbum, nThema+1, nTask);
		return;
	}
	else if (_EncyCharAlbums[nAlbum].Themas[nThema].ThemaState == 2)
	{
		nlwarning("thema already completed album:%d thema:%d task:%d", nAlbum, nThema+1, nTask);
		return;
	}
	else if (_EncyCharAlbums[nAlbum].Themas[nThema].getTaskState((uint8)nTask) == nNewState)
	{
		nlwarning("task already completed album:%d thema:%d task:%d", nAlbum, nThema+1, nTask);
		return;
	}
	_EncyCharAlbums[nAlbum].Themas[nThema].setTaskState((uint8)nTask, nNewState);

	// If the album was not shown previously show the album and thema corresponding
	_EncyCharAlbums[nAlbum].AlbumState = 1;
	_EncyCharAlbums[nAlbum].Themas[nThema].ThemaState = 1;

	// Allow the user to see the Rite
	if (_EncyCharAlbums[nAlbum].Themas[nThema].getTaskState(0) == 0)
		_EncyCharAlbums[nAlbum].Themas[nThema].setTaskState(0, 1);

	checkIfThemaCompleted(nAlbum, nThema, bSendMsg);
	checkIfAlbumCompleted(nAlbum);

	if (bSendMsg)
	{
		// Create update album message
		sendAlbumMessage(nAlbum, nThema);

		// and send a message to the client
		if (nNewState == 2)
			CCharacter::sendDynamicSystemMessage(_Char.getEntityRowId(), "EGS_TASK_ENCYCLO_SUCCESS");
	}
}

//-----------------------------------------------------------------------------

void CCharacterEncyclopedia::unlockThema(uint32 nAlbum, uint32 nThema, bool bSendMsg)
{
	nlassert(nAlbum < _EncyCharAlbums.size());
	nlassert(nThema > 0); // Thema must start at 1
	nThema = nThema - 1;
	nlassert(nThema < _EncyCharAlbums[nAlbum].Themas.size());

	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	const CStaticEncycloThema *pSThm = rSE.getThema(nAlbum, nThema+1);

	if (pSThm == NULL)
	{
		nlwarning("thema unknown album:%d thema:%d",nAlbum, nThema+1);
		return;
	}

	if (_EncyCharAlbums[nAlbum].AlbumState == 0)
		_EncyCharAlbums[nAlbum].AlbumState = 1;
	
	if (_EncyCharAlbums[nAlbum].Themas[nThema].ThemaState == 0)
		_EncyCharAlbums[nAlbum].Themas[nThema].ThemaState = 1;
	
	uint8 nNbTask = (uint8)pSThm->Tasks.size()+1;
	for (uint8 i = 0; i < nNbTask; ++i)
	{
		if (_EncyCharAlbums[nAlbum].Themas[nThema].getTaskState(i) == 0)
			_EncyCharAlbums[nAlbum].Themas[nThema].setTaskState(i, 1);
	}

	if (bSendMsg)
	{
		// Create update album message
		sendAlbumMessage(nAlbum, nThema);
		
		// and send a message to the client
		CCharacter::sendDynamicSystemMessage(_Char.getEntityRowId(), "EGS_UNLOCK_ENCYCLO");
	}
}

//-----------------------------------------------------------------------------

bool CCharacterEncyclopedia::isAllTaskDone(uint32 nAlbum, uint32 nThema)
{
	nlassert(nAlbum < _EncyCharAlbums.size());
	nlassert(nThema > 0); // Thema must start at 1
	nThema = nThema - 1;
	nlassert(nThema < _EncyCharAlbums[nAlbum].Themas.size());

	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	const CStaticEncycloThema *pSThm = rSE.getThema(nAlbum, nThema+1);
	
	if (pSThm == NULL)
	{
		nlwarning("thema unknown album:%d thema:%d",nAlbum, nThema+1);
		return false;
	}
	
	uint8 nNbTask = (uint8)pSThm->Tasks.size()+1;
	for (uint8 i = 1; i < nNbTask; ++i)
	{
		if (_EncyCharAlbums[nAlbum].Themas[nThema].getTaskState(i) != 2)
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------

bool CCharacterEncyclopedia::isAtLeastOneTaskNotDone(uint32 nAlbum, uint32 nThema)
{
	nlassert(nAlbum < _EncyCharAlbums.size());
	nlassert(nThema > 0); // Thema must start at 1
	nThema = nThema - 1;
	nlassert(nThema < _EncyCharAlbums[nAlbum].Themas.size());
	
	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	const CStaticEncycloThema *pSThm = rSE.getThema(nAlbum, nThema+1);
	
	if (pSThm == NULL)
	{
		nlwarning("thema unknown album:%d thema:%d",nAlbum, nThema+1);
		return false;
	}
	
	uint8 nNbTask = (uint8)pSThm->Tasks.size()+1;
	for (uint8 i = 1; i < nNbTask; ++i)
	{
		if (_EncyCharAlbums[nAlbum].Themas[nThema].getTaskState(i) != 2)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

void CCharacterEncyclopedia::sendEncycloToClient()
{
	CEncyclopediaUpdateMsg msgToClient;
	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	msgToClient.Type = CEncyclopediaUpdateMsg::UpdateInit;
	
	for (uint32 nAlbum = 0; nAlbum < rSE.getNbAlbum(); ++nAlbum)
	{
		CEncyMsgAlbum albumOut;
		const CStaticEncycloAlbum *pSAlb = rSE.getAlbum(nAlbum);
		if (pSAlb == NULL) continue;

		CEncyCharAlbum &rECAlb = _EncyCharAlbums[nAlbum];
		if (rECAlb.AlbumState == 0) continue;
		TVectorParamCheck vParams;

		if (rECAlb.AlbumNameCache == 0)
			rECAlb.AlbumNameCache = STRING_MANAGER::sendStringToClient(	_Char.getEntityRowId(), 
																		pSAlb->Title, vParams	);
		albumOut.Name = rECAlb.AlbumNameCache;
		albumOut.State = rECAlb.AlbumState;
		// If the album is finished display the associated brick
		if (albumOut.State == 2)
			albumOut.RewardBrick = pSAlb->RewardBrick.asInt();

		for (uint32 nThema = 0; nThema < rSE.getNbThema(nAlbum); ++nThema)
		{
			CEncyMsgThema t;
			makeThemaMessage(t, nAlbum, nThema);
			if (t.State != 0)
				albumOut.Themas.push_back(t);
		}

		msgToClient.AllAlbums.push_back(albumOut);
	}

	PlayerManager.sendImpulseToClient(_Char.getId(), "ENCYCLOPEDIA:INIT", msgToClient);
	_SentToClient = true;
}

//-----------------------------------------------------------------------------

bool CCharacterEncyclopedia::checkIfThemaCompleted(uint32 nAlbum, uint32 nThemaInt, bool bSendMsg)
{
	// nThemaInt begins at 0
	nlassert(nAlbum < _EncyCharAlbums.size());
	nlassert(nThemaInt < _EncyCharAlbums[nAlbum].Themas.size());
	// Get the number of tasks of this thema
	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	const CStaticEncycloThema *pThm = rSE.getThema(nAlbum, nThemaInt+1);
	if (pThm == NULL) return false;
	uint32 nNbTask = (uint32)pThm->Tasks.size();

	bool bRiteFinished = (_EncyCharAlbums[nAlbum].Themas[nThemaInt].getTaskState(0) == 2);
	bool bTasksFinished[7];
	for (uint8 nTask = 1; nTask < 8; ++nTask)
	{
		bTasksFinished[nTask-1] = (_EncyCharAlbums[nAlbum].Themas[nThemaInt].getTaskState(nTask) == 2);
	}

	bool bThemaFinished = bRiteFinished;
	for (uint32 i = 0; i < nNbTask; ++i)
	{
		bThemaFinished = bThemaFinished && bTasksFinished[i];
	}

	if (bThemaFinished)
	{
		_EncyCharAlbums[nAlbum].Themas[nThemaInt].ThemaState = 2;
		if (bSendMsg)
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage(_Char.getEntityRowId(), "ENCY_THEMA_FINISHED");
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

bool CCharacterEncyclopedia::checkIfAlbumCompleted(uint32 nAlbum)
{
	nlassert(nAlbum < _EncyCharAlbums.size());

	bool bAlbumFinished = true;

	for (uint32 i = 0; i < _EncyCharAlbums[nAlbum].Themas.size(); ++i)
	{
		bAlbumFinished = bAlbumFinished && (_EncyCharAlbums[nAlbum].Themas[i].ThemaState == 2);
	}

	if (bAlbumFinished)
	{
		_EncyCharAlbums[nAlbum].AlbumState = 2;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

void CCharacterEncyclopedia::sendAlbumMessage(uint32 nAlbum, uint32 nThema, bool warnBotAway)
{
	CEncyclopediaUpdateMsg msgToClient;
	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	msgToClient.Type = CEncyclopediaUpdateMsg::UpdateAlbum;

	const CStaticEncycloAlbum *pSAlb = rSE.getAlbum(nAlbum);
	const CStaticEncycloThema *pSThm = rSE.getThema(nAlbum, nThema+1);
	if ((pSAlb == NULL) || (pSThm == NULL)) return;
	TVectorParamCheck vParams;
	if (_EncyCharAlbums[nAlbum].AlbumNameCache == 0)
		_EncyCharAlbums[nAlbum].AlbumNameCache = STRING_MANAGER::sendStringToClient(_Char.getEntityRowId(), pSAlb->Title, vParams);
	msgToClient.Album.Name = _EncyCharAlbums[nAlbum].AlbumNameCache;
	msgToClient.Album.State = _EncyCharAlbums[nAlbum].AlbumState;
	// If the album is finished display the associated brick
	if (msgToClient.Album.State == 2)
		msgToClient.Album.RewardBrick = pSAlb->RewardBrick.asInt();

	CEncyMsgThema t;
	makeThemaMessage(t, nAlbum, nThema, warnBotAway);

	msgToClient.Album.Themas.push_back(t);

	PlayerManager.sendImpulseToClient(_Char.getId(), "ENCYCLOPEDIA:UPDATE", msgToClient);
}

//-----------------------------------------------------------------------------

void CCharacterEncyclopedia::makeThemaMessage(CEncyMsgThema &t, uint32 nAlbum, uint32 nThema, bool warnBotAway)
{
	const CStaticEncyclo &rSE = CSheets::getEncyclopedia();
	// nThema is from 0 because this method is internally used
	const CStaticEncycloThema *pSThm = rSE.getThema(nAlbum, nThema+1);
	if (pSThm == NULL) return;
	
	CEncyCharThema &rECThm = _EncyCharAlbums[nAlbum].Themas[nThema];
	if (rECThm.ThemaState == 0) return;
	
	TVectorParamCheck vParams;

	if (rECThm.ThemaNameCache == 0)
		rECThm.ThemaNameCache = STRING_MANAGER::sendStringToClient(	_Char.getEntityRowId(), 
																	pSThm->Title, vParams);
	t.Name = rECThm.ThemaNameCache;
	t.State = rECThm.ThemaState;
	// If the thema is finished display associated stuff
	if (t.State == 2)
	{
		if (rECThm.RewardTextCache == 0)
			rECThm.RewardTextCache = STRING_MANAGER::sendStringToClient( _Char.getEntityRowId(), 
																		pSThm->RewardText, vParams);
		t.RewardText = rECThm.RewardTextCache;
		t.RewardSheet = pSThm->RewardSheet.asInt();
	}
	t.NbTask = pSThm->Tasks.size()+1;
	t.RiteTaskStatePacked = rECThm.RiteTaskStatePacked;
	
	// For the moment update all tasks and rite
	for (uint32 i = 0; i < t.NbTask; ++i)
	{
		if (rECThm.getTaskState((uint8)i) != 0)
		{
			CMissionTemplate *pMT = getRiteTaskTemplate(*pSThm, i);
			if (pMT == NULL) continue;

			// Without the bot (the AI service is down, or starts), or before the bot has its name, the character has no
			// name of the bot: the thema is sent again when the bot has it (botNameChanged)
			TDataSetRow rGiver;
			uint32 nNPCname;
			const bool npcThere = getEncyclopediaNpc(*pMT, rGiver, nNPCname);
			if (rGiver.isNull() && warnBotAway)
			{
				const string &missionName = i == 0 ? pSThm->Rite : pSThm->Tasks[i - 1];
				nlwarning("ENCYCLOPEDIA: the bot %u of the mission '%s' is not there, character %s has no name of it until it is", pMT->EncycloNPC, missionName.c_str(), _Char.getId().toString().c_str());
			}

			// The title is made once, when the bot has its name, which the title can use. A title that the character has
			// is kept while its bot is away.
			if (rECThm.RiteTaskNameCache[i] == 0 && npcThere)
				rECThm.RiteTaskNameCache[i] = pMT->sendTitleText(_Char.getEntityRowId(), rGiver);
			t.TaskName[i] = rECThm.RiteTaskNameCache[i];
			t.TaskNPCName[i] = nNPCname;
		}
	}
}

//-----------------------------------------------------------------------------

bool CCharacterEncyclopedia::botNameChanged(uint32 nAlbum, uint32 nThema, uint8 riteTasks)
{
	// The whole encyclopedia, when it is sent to the client that comes into the game, has the name
	if (!_SentToClient)
		return false;

	// nThema is from 1, as in the mission templates
	if (nAlbum >= _EncyCharAlbums.size() || nThema == 0 || nThema > _EncyCharAlbums[nAlbum].Themas.size())
		return false;
	nThema = nThema - 1;
	CEncyCharThema &rECThm = _EncyCharAlbums[nAlbum].Themas[nThema];
	if (rECThm.ThemaState == 0)
		return false;

	// only for the rite and the tasks that the character has; their titles can have the name of the bot in them
	bool send = false;
	for (uint8 i = 0; i < 8; ++i)
	{
		if ((riteTasks & (1 << i)) != 0 && rECThm.getTaskState(i) != 0)
		{
			rECThm.RiteTaskNameCache[i] = 0;
			send = true;
		}
	}
	if (send)
		sendAlbumMessage(nAlbum, nThema, false);
	return send;
}
