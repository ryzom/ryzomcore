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

#include "db_manager_messages.h"
#include "db_manager.h"

#include <nel/misc/hierarchical_timer.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;

TUnifiedCallbackItem	DbManagerCbArray[] =
{
	{ "PD_INIT",			&cbInitDatabase },
	{ "PD_UPDATE",			&cbUpdateDb },
	{ "PD_SHEETID_MAPPING",	&cbSheetIdMapping },
	//{ "PD_SM_STRING",		&cbAddString },

	{ "RB_TASK_SUCCESS",	&cbRBSTaskSuccess },
	{ "RB_TASK_FAILED",		&cbRBSTaskFailure },
};


void	sendInitFailed(uint32 databaseId, const std::string& message, NLNET::TServiceId sid)
{
	CMessage	msgout("PD_INIT_FAILED");
	msgout.serial(databaseId);
	string		hrmsg = message;
	msgout.serial(hrmsg);
	CUnifiedNetwork::getInstance()->send(sid, msgout);
}


void	cbInitDatabase(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	uint32	id;
	string	description;

	nlinfo("Service '%s' '%d' connected", serviceName.c_str(), serviceId.get());

	msgin.serial(id, description);

	CDatabase*	db = CDbManager::loadDatabase(id, description);
	if (db == NULL)
	{
		CDbManager::unmapService(serviceId);
		sendInitFailed(id, "Unable to loadDatabase() '"+toString(id)+"'", serviceId);
		return;
	}

	// check database mapping success
	if (!CDbManager::mapService(serviceId, id))
	{
		sendInitFailed(id, "Unable to map service id", serviceId);
		return;
	}

	// build & send allocators to requesting service
	vector<RY_PDS::CIndexAllocator> allocators;
	db->buildIndexAllocators(allocators);

	// send allocators state
	CMessage	msgalloc("PD_ALLOCS");
	msgalloc.serial(id);
	msgalloc.serialCont(allocators);
	CUnifiedNetwork::getInstance()->send(serviceId, msgalloc);

	// send string manager
//	CMessage	msgsm("PD_SM_INIT");
//	RY_PDS::CPDStringManager	&sm = db->getStringManager();
//	msgsm.serial(id);
//	msgsm.serial(sm);
//	CUnifiedNetwork::getInstance()->send(serviceId, msgsm);

	// everything is ok
	CMessage	msgready("PD_READY");
	msgready.serial(id);
	uint32		updateId = db->getLastUpdateId();
	msgready.serial(updateId);
	CUnifiedNetwork::getInstance()->send(serviceId, msgready);
}





CMessageStat			stats;

void	cbUpdateDb(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(PDS_cbUpdateDb);

	stats.clear();

	TDatabaseId	databaseId;
	msgin.serial(databaseId);

	uint32		updateId;
	msgin.serial(updateId);

	CDatabase*	database = CDbManager::getDatabase(databaseId);
	if (database == NULL)
	{
		nlwarning("cbUpdateDb(): failed to retrieve database '%d'", databaseId);
		return;
	}

	RY_PDS::CDbMessageQueue*	updateQueue = database->getUpdateMessageQueue(updateId);

	if (updateQueue == NULL)
	{
		nlwarning("cbUpdateDb(): failed to create updateQueue for update %d of database %d", updateId, databaseId);
		return;
	}


	updateQueue->clear();
	msgin.serial(*updateQueue);

	// compute stat
	stats.addMessage(msgin.length());
	stats.TotalHeaderSize += msgin.getPos();

	uint	i;
	for (i=0; i<updateQueue->getNumMessages(); ++i)
	{
		RY_PDS::CDbMessage&	updMsg = updateQueue->getMessage(i);

		// compute stat
		stats.addSubMessage(updMsg.getMessageHeaderSize(), updMsg.getType());

		switch (updMsg.getType())
		{
		case RY_PDS::CDbMessage::UpdateValue:
			{
				database->set(updMsg.getTable(), updMsg.getRow(), updMsg.getColumn(), updMsg.getDatasize(), updMsg.getData());

				// compute stat
				stats.addUpdate((uint16)updMsg.getTable(), (uint32)updMsg.getRow(), (uint16)updMsg.getColumn());
			}
			break;

		case RY_PDS::CDbMessage::SetParent:
			{
				database->setParent(updMsg.getTable(), updMsg.getRow(), updMsg.getColumn(), updMsg.getObjectIndex());

				// compute stat
				stats.addUpdate((uint16)updMsg.getTable(), (uint32)updMsg.getRow(), (uint16)updMsg.getColumn());
			}
			break;

		case RY_PDS::CDbMessage::AllocRow:
			if (!database->allocate(RY_PDS::CObjectIndex(updMsg.getTable(), updMsg.getRow())))
				break;
			if (updMsg.getValue64bits() != 0)
				database->mapRow(RY_PDS::CObjectIndex(updMsg.getTable(), updMsg.getRow()), updMsg.getValue64bits());
			break;

		case RY_PDS::CDbMessage::DeallocRow:
			database->deallocate(RY_PDS::CObjectIndex(updMsg.getTable(), updMsg.getRow()));
			break;

		case RY_PDS::CDbMessage::LoadRow:
			{
				// get object index associated to the given key
				CMessage				msgout("PD_FETCH");
				msgout.serial(databaseId);
				RY_PDS::TTableIndex		table = updMsg.getTable();
				uint64					key = updMsg.getValue64bits();
				RY_PDS::CObjectIndex	index = database->getMappedRow(table, key);
				if (index.isValid() && database->fetch(index, msgout))
				{
					CUnifiedNetwork::getInstance()->send(serviceId, msgout);
				}
				else
				{
					CMessage			msgfail("PD_FETCH_FAILURE");
					msgfail.serial(databaseId);
					msgfail.serial(table, key);
					CUnifiedNetwork::getInstance()->send(serviceId, msgfail);
				}
			}
			break;

		case RY_PDS::CDbMessage::ReleaseRow:
			database->release(RY_PDS::CObjectIndex(updMsg.getTable(), updMsg.getRow()));
			break;

//		case RY_PDS::CDbMessage::AddString:
//			{
//				NLMISC::CEntityId			eId = NLMISC::CEntityId(updMsg.getValue64bits());
//				const ucstring&				str = updMsg.getString();
//				RY_PDS::CPDStringManager&	sm = database->getStringManager();
//				sm.addString(eId, str);
//			}
//			break;
//
//		case RY_PDS::CDbMessage::UnmapString:
//			{
//				NLMISC::CEntityId			eId = NLMISC::CEntityId(updMsg.getValue64bits());
//				RY_PDS::CPDStringManager&	sm = database->getStringManager();
//				sm.unmap(eId);
//			}
//			break;

		case RY_PDS::CDbMessage::Log:
			break;

		case RY_PDS::CDbMessage::PushContext:
			break;

		case RY_PDS::CDbMessage::PopContext:
			break;

		case RY_PDS::CDbMessage::LogChat:
			break;

		default:
			nlwarning("CDbManager::cbUpdateDb(): unhandled message type '%d' from database %d (service %s-%d)", updMsg.getType(), databaseId, serviceName.c_str(), serviceId.get());
			break;
		}
	}

	// mark update
	database->receiveUpdate(updateId);
}





/* *** DEPRECATED *** */
/*
void	cbAddString(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TDatabaseId	id = CDbManager::getDatabaseId(serviceId);
	if (id == INVALID_DATABASE_ID)
		return;

	CEntityId							eId;
	RY_PDS::CPDStringManager::TEntryId	id;
	ucstring							str;

	msgin.serial(eId);
	msgin.serial(id);
	msgin.serial(str);

	CDbManager::addString(id, eId, pdId, str);
}
*/


void	cbSheetIdMapping(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TDatabaseId	id;
	msgin.serial(id);

	CDatabase*	database = CDbManager::getDatabase(id);

	if (database == NULL)
	{
		nlwarning("cbSheetIdMapping(): failed to retrieve database '%d'", id);
		return;
	}

	database->serialSheetIdStringMapper(msgin);
}




void	cbRBSTaskSuccess(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	uint32	taskId;
	msgin.serial(taskId);

	CDbManager::notifyRBSSuccess(taskId);
}

void	cbRBSTaskFailure(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	uint32	taskId;
	msgin.serial(taskId);

	nlwarning("cbRBSTaskFailure(): task %d failure, please call maintenance. Data failure may appear", taskId);

	CDbManager::notifyRBSFailure(taskId);
}





void	onServiceDown(const std::string &serviceName, NLNET::TServiceId sid, void *arg)
{
	CDbManager::unmapService(sid);
}


void	onRBSUp(const std::string &serviceName, NLNET::TServiceId sid, void *arg)
{
	CDbManager::RBSUp();
}

void	onRBSDown(const std::string &serviceName, NLNET::TServiceId sid, void *arg)
{
	CDbManager::RBSDown();
}




void	initDbManagerMessages()
{
	CUnifiedNetwork::getInstance()->addCallbackArray(DbManagerCbArray, sizeof(DbManagerCbArray)/sizeof(TUnifiedCallbackItem));
	CUnifiedNetwork::getInstance()->setServiceDownCallback("*", onServiceDown);
	CUnifiedNetwork::getInstance()->setServiceUpCallback("RBS", onRBSUp);
	CUnifiedNetwork::getInstance()->setServiceDownCallback("RBS", onRBSDown);
}



NLMISC_COMMAND(PDDisplayMsgStats, "", "")
{
	stats.display(&log);
	return true;
}

