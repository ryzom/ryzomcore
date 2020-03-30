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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "stdpch.h"

#include "r2_share_itf.h"

namespace R2
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CShareServerAnimationItfSkel::TMessageHandlerMap &CShareServerAnimationItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("RCAMP"), &CShareServerAnimationItfSkel::connectAnimationModePlay_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SAID"), &CShareServerAnimationItfSkel::askMissionItemsDescription_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SAAPD"), &CShareServerAnimationItfSkel::askActPositionDescriptions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SAUTD"), &CShareServerAnimationItfSkel::askUserTriggerDescriptions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SOUTT"), &CShareServerAnimationItfSkel::onUserTriggerTriggered_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SODT"), &CShareServerAnimationItfSkel::onDssTarget_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CShareServerAnimationItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}


	void CShareServerAnimationItfSkel::connectAnimationModePlay_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareServerAnimationItfSkel_connectAnimationModePlay_RCAMP);
		connectAnimationModePlay(sender);
	}

	void CShareServerAnimationItfSkel::askMissionItemsDescription_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareServerAnimationItfSkel_askMissionItemsDescription_SAID);
		askMissionItemsDescription(sender);
	}

	void CShareServerAnimationItfSkel::askActPositionDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareServerAnimationItfSkel_askActPositionDescriptions_SAAPD);
		askActPositionDescriptions(sender);
	}

	void CShareServerAnimationItfSkel::askUserTriggerDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareServerAnimationItfSkel_askUserTriggerDescriptions_SAUTD);
		askUserTriggerDescriptions(sender);
	}

	void CShareServerAnimationItfSkel::onUserTriggerTriggered_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerAnimationItfSkel_onUserTriggerTriggered_SOUTT);
		uint32	actId;
			nlRead(__message, serial, actId);
		uint32	triggerId;
			nlRead(__message, serial, triggerId);
		onUserTriggerTriggered(sender, actId, triggerId);
	}

	void CShareServerAnimationItfSkel::onDssTarget_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerAnimationItfSkel_onDssTarget_SODT);
		std::vector<std::string>	args;
			nlRead(__message, serialCont, args);
		onDssTarget(sender, args);
	}
		// request the connection to play mode in an animation session
	void CShareServerAnimationItfProxy::connectAnimationModePlay(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->connectAnimationModePlay(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_connectAnimationModePlay(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A client Message to register mission item of a scenario
	void CShareServerAnimationItfProxy::askMissionItemsDescription(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->askMissionItemsDescription(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_askMissionItemsDescription(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A client Message to update client Act Position Description
	void CShareServerAnimationItfProxy::askActPositionDescriptions(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->askActPositionDescriptions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_askActPositionDescriptions(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A client Message to update client User Trigger Description
	void CShareServerAnimationItfProxy::askUserTriggerDescriptions(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->askUserTriggerDescriptions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_askUserTriggerDescriptions(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// client wants to trigger an user trigger
	void CShareServerAnimationItfProxy::onUserTriggerTriggered(NLNET::IModule *sender, uint32 actId, uint32 triggerId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onUserTriggerTriggered(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), actId, triggerId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onUserTriggerTriggered(__message, actId, triggerId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// client wants to execute a dm action on its target
	void CShareServerAnimationItfProxy::onDssTarget(NLNET::IModule *sender, const std::vector<std::string> &args)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onDssTarget(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), args);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onDssTarget(__message, args);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerAnimationItfProxy::buildMessageFor_connectAnimationModePlay(NLNET::CMessage &__message)
	{
		__message.setType("RCAMP");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerAnimationItfProxy::buildMessageFor_askMissionItemsDescription(NLNET::CMessage &__message)
	{
		__message.setType("SAID");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerAnimationItfProxy::buildMessageFor_askActPositionDescriptions(NLNET::CMessage &__message)
	{
		__message.setType("SAAPD");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerAnimationItfProxy::buildMessageFor_askUserTriggerDescriptions(NLNET::CMessage &__message)
	{
		__message.setType("SAUTD");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerAnimationItfProxy::buildMessageFor_onUserTriggerTriggered(NLNET::CMessage &__message, uint32 actId, uint32 triggerId)
	{
		__message.setType("SOUTT");
			nlWrite(__message, serial, actId);
			nlWrite(__message, serial, triggerId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerAnimationItfProxy::buildMessageFor_onDssTarget(NLNET::CMessage &__message, const std::vector<std::string> &args)
	{
		__message.setType("SODT");
			nlWrite(__message, serialCont, const_cast < std::vector<std::string>& > (args));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CShareServerEditionItfSkel::TMessageHandlerMap &CShareServerEditionItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("RSS1"), &CShareServerEditionItfSkel::startingScenario_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("RSS2"), &CShareServerEditionItfSkel::startScenario_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("ADC_ACK"), &CShareServerEditionItfSkel::advConnACK_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SUCR"), &CShareServerEditionItfSkel::onUserComponentRegistered_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SUCD"), &CShareServerEditionItfSkel::onUserComponentDownloading_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSUA"), &CShareServerEditionItfSkel::onScenarioUploadAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SNSA"), &CShareServerEditionItfSkel::onNodeSetAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SNIA"), &CShareServerEditionItfSkel::onNodeInsertAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SNEA"), &CShareServerEditionItfSkel::onNodeEraseAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SNMA"), &CShareServerEditionItfSkel::onNodeMoveAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SMCA"), &CShareServerEditionItfSkel::onMapConnectionAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SCMUA"), &CShareServerEditionItfSkel::onCharModeUpdateAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STPA"), &CShareServerEditionItfSkel::onTpPositionAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("TPEP"), &CShareServerEditionItfSkel::tpToEntryPoint_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STA"), &CShareServerEditionItfSkel::setStartingAct_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRAU"), &CShareServerEditionItfSkel::onScenarioRingAccessUpdated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSSFA"), &CShareServerEditionItfSkel::saveScenarioFile_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SLSFA"), &CShareServerEditionItfSkel::loadScenarioFile_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SUCF"), &CShareServerEditionItfSkel::saveUserComponentFile_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("LUCF"), &CShareServerEditionItfSkel::loadUserComponentFile_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STOCTA"), &CShareServerEditionItfSkel::teleportOneCharacterToAnother_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STWUS"), &CShareServerEditionItfSkel::teleportWhileUploadingScenario_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DSS_HEAD"), &CShareServerEditionItfSkel::multiPartMsgHead_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DSS_MSG"), &CShareServerEditionItfSkel::multiPartMsgBody_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DSS_FOOT"), &CShareServerEditionItfSkel::multiPartMsgFoot_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DSS_FW"), &CShareServerEditionItfSkel::forwardToDss_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CShareServerEditionItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}


	void CShareServerEditionItfSkel::startingScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareServerEditionItfSkel_startingScenario_RSS1);
		startingScenario(sender);
	}

	void CShareServerEditionItfSkel::startScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_startScenario_RSS2);
		bool	ok;
			nlRead(__message, serial, ok);
		TScenarioHeaderSerializer	header;
			nlRead(__message, serial, header);
		CObjectSerializerServer	data;
			nlRead(__message, serial, data);
		uint32	startingAct;
			nlRead(__message, serial, startingAct);
		startScenario(sender, ok, header, data, startingAct);
	}

	void CShareServerEditionItfSkel::advConnACK_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareServerEditionItfSkel_advConnACK_ADC_ACK);
		advConnACK(sender);
	}

	void CShareServerEditionItfSkel::onUserComponentRegistered_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onUserComponentRegistered_SUCR);
		NLMISC::CHashKeyMD5	md5;
			nlRead(__message, serial, md5);
		onUserComponentRegistered(sender, md5);
	}

	void CShareServerEditionItfSkel::onUserComponentDownloading_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onUserComponentDownloading_SUCD);
		NLMISC::CHashKeyMD5	md5;
			nlRead(__message, serial, md5);
		onUserComponentDownloading(sender, md5);
	}

	void CShareServerEditionItfSkel::onScenarioUploadAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onScenarioUploadAsked_SSUA);
		uint32	msgId;
			nlRead(__message, serial, msgId);
		CObjectSerializerServer	hlScenario;
			nlRead(__message, serial, hlScenario);
		bool	mustBrodcast;
			nlRead(__message, serial, mustBrodcast);
		onScenarioUploadAsked(sender, msgId, hlScenario, mustBrodcast);
	}

	void CShareServerEditionItfSkel::onNodeSetAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onNodeSetAsked_SNSA);
		uint32	msgId;
			nlRead(__message, serial, msgId);
		std::string	instanceId;
			nlRead(__message, serial, instanceId);
		std::string	attrName;
			nlRead(__message, serial, attrName);
		R2::CObjectSerializerServer	value;
			nlRead(__message, serial, value);
		onNodeSetAsked(sender, msgId, instanceId, attrName, value);
	}

	void CShareServerEditionItfSkel::onNodeInsertAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onNodeInsertAsked_SNIA);
		uint32	msgId;
			nlRead(__message, serial, msgId);
		std::string	instanceId;
			nlRead(__message, serial, instanceId);
		std::string	attrName;
			nlRead(__message, serial, attrName);
		sint32	position;
			nlRead(__message, serial, position);
		std::string	key;
			nlRead(__message, serial, key);
		R2::CObjectSerializerServer	value;
			nlRead(__message, serial, value);
		onNodeInsertAsked(sender, msgId, instanceId, attrName, position, key, value);
	}

	void CShareServerEditionItfSkel::onNodeEraseAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onNodeEraseAsked_SNEA);
		uint32	msgId;
			nlRead(__message, serial, msgId);
		std::string	instanceId;
			nlRead(__message, serial, instanceId);
		std::string	attrName;
			nlRead(__message, serial, attrName);
		sint32	position;
			nlRead(__message, serial, position);
		onNodeEraseAsked(sender, msgId, instanceId, attrName, position);
	}

	void CShareServerEditionItfSkel::onNodeMoveAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onNodeMoveAsked_SNMA);
		uint32	msgId;
			nlRead(__message, serial, msgId);
		std::string	instanceId1;
			nlRead(__message, serial, instanceId1);
		std::string	attrName1;
			nlRead(__message, serial, attrName1);
		sint32	position1;
			nlRead(__message, serial, position1);
		std::string	instanceId2;
			nlRead(__message, serial, instanceId2);
		std::string	attrName2;
			nlRead(__message, serial, attrName2);
		sint32	position2;
			nlRead(__message, serial, position2);
		onNodeMoveAsked(sender, msgId, instanceId1, attrName1, position1, instanceId2, attrName2, position2);
	}

	void CShareServerEditionItfSkel::onMapConnectionAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onMapConnectionAsked_SMCA);
		TSessionId	scenarioId;
			nlRead(__message, serial, scenarioId);
		bool	updateHighLevel;
			nlRead(__message, serial, updateHighLevel);
		bool	mustTp;
			nlRead(__message, serial, mustTp);
		R2::TUserRole	role;
			nlRead(__message, serial, role);
		onMapConnectionAsked(sender, scenarioId, updateHighLevel, mustTp, role);
	}

	void CShareServerEditionItfSkel::onCharModeUpdateAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onCharModeUpdateAsked_SCMUA);
		R2::TCharMode	mode;
			nlRead(__message, serial, mode);
		onCharModeUpdateAsked(sender, mode);
	}

	void CShareServerEditionItfSkel::onTpPositionAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onTpPositionAsked_STPA);
		float	x;
			nlRead(__message, serial, x);
		float	y;
			nlRead(__message, serial, y);
		float	z;
			nlRead(__message, serial, z);
		onTpPositionAsked(sender, x, y, z);
	}

	void CShareServerEditionItfSkel::tpToEntryPoint_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_tpToEntryPoint_TPEP);
		uint32	actIndex;
			nlRead(__message, serial, actIndex);
		tpToEntryPoint(sender, actIndex);
	}

	void CShareServerEditionItfSkel::setStartingAct_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_setStartingAct_STA);
		uint32	actIndex;
			nlRead(__message, serial, actIndex);
		setStartingAct(sender, actIndex);
	}

	void CShareServerEditionItfSkel::onScenarioRingAccessUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_onScenarioRingAccessUpdated_SRAU);
		bool	ok;
			nlRead(__message, serial, ok);
		std::string	ringAccess;
			nlRead(__message, serial, ringAccess);
		std::string	errMsg;
			nlRead(__message, serial, errMsg);
		onScenarioRingAccessUpdated(sender, ok, ringAccess, errMsg);
	}

	void CShareServerEditionItfSkel::saveScenarioFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_saveScenarioFile_SSSFA);
		std::string	md5;
			nlRead(__message, serial, md5);
		R2::TScenarioHeaderSerializer	header;
			nlRead(__message, serial, header);
		saveScenarioFile(sender, md5, header);
	}

	void CShareServerEditionItfSkel::loadScenarioFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_loadScenarioFile_SLSFA);
		std::string	md5;
			nlRead(__message, serial, md5);
		std::string	signature;
			nlRead(__message, serial, signature);
		loadScenarioFile(sender, md5, signature);
	}

	void CShareServerEditionItfSkel::saveUserComponentFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_saveUserComponentFile_SUCF);
		std::string	md5;
			nlRead(__message, serial, md5);
		R2::TScenarioHeaderSerializer	header;
			nlRead(__message, serial, header);
		saveUserComponentFile(sender, md5, header);
	}

	void CShareServerEditionItfSkel::loadUserComponentFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_loadUserComponentFile_LUCF);
		std::string	md5;
			nlRead(__message, serial, md5);
		std::string	signature;
			nlRead(__message, serial, signature);
		loadUserComponentFile(sender, md5, signature);
	}

	void CShareServerEditionItfSkel::teleportOneCharacterToAnother_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_teleportOneCharacterToAnother_STOCTA);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	sourceId;
			nlRead(__message, serial, sourceId);
		uint32	destId;
			nlRead(__message, serial, destId);
		teleportOneCharacterToAnother(sender, sessionId, sourceId, destId);
	}

	void CShareServerEditionItfSkel::teleportWhileUploadingScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_teleportWhileUploadingScenario_STWUS);
		std::string	island;
			nlRead(__message, serial, island);
		std::string	entryPoint;
			nlRead(__message, serial, entryPoint);
		std::string	season;
			nlRead(__message, serial, season);
		teleportWhileUploadingScenario(sender, island, entryPoint, season);
	}

	void CShareServerEditionItfSkel::multiPartMsgHead_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_multiPartMsgHead_DSS_HEAD);
		uint32	charId;
			nlRead(__message, serial, charId);
		std::string	msgName;
			nlRead(__message, serial, msgName);
		uint32	nbPacket;
			nlRead(__message, serial, nbPacket);
		uint32	size;
			nlRead(__message, serial, size);
		multiPartMsgHead(sender, charId, msgName, nbPacket, size);
	}

	void CShareServerEditionItfSkel::multiPartMsgBody_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_multiPartMsgBody_DSS_MSG);
		uint32	charId;
			nlRead(__message, serial, charId);
		uint32	partId;
			nlRead(__message, serial, partId);
		std::vector<uint8>	data;
			nlRead(__message, serialCont, data);
		multiPartMsgBody(sender, charId, partId, data);
	}

	void CShareServerEditionItfSkel::multiPartMsgFoot_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_multiPartMsgFoot_DSS_FOOT);
		uint32	charId;
			nlRead(__message, serial, charId);
		multiPartMsgFoot(sender, charId);
	}

	void CShareServerEditionItfSkel::forwardToDss_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareServerEditionItfSkel_forwardToDss_DSS_FW);
		uint32	charId;
			nlRead(__message, serial, charId);
		NLNET::CMessage	msg;
			nlRead(__message, serialMessage, msg);
		forwardToDss(sender, charId, msg);
	}
	void CShareServerEditionItfProxy::startingScenario(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->startingScenario(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_startingScenario(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Request the start of a test scenario
	void CShareServerEditionItfProxy::startScenario(NLNET::IModule *sender, bool ok, const TScenarioHeaderSerializer &header, const CObjectSerializerServer &data, uint32 startingAct)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->startScenario(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), ok, header, data, startingAct);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_startScenario(__message, ok, header, data, startingAct);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Client has received the ADV_CONN message
	void CShareServerEditionItfProxy::advConnACK(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->advConnACK(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_advConnACK(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client announce to the server that he has registered a component.
	void CShareServerEditionItfProxy::onUserComponentRegistered(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onUserComponentRegistered(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onUserComponentRegistered(__message, md5);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client announce to the server that he need a componennt so the server must uploading it.
	void CShareServerEditionItfProxy::onUserComponentDownloading(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onUserComponentDownloading(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onUserComponentDownloading(__message, md5);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Upload the high level scenario.
	void CShareServerEditionItfProxy::onScenarioUploadAsked(NLNET::IModule *sender, uint32 msgId, const CObjectSerializerServer &hlScenario, bool mustBrodcast)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onScenarioUploadAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgId, hlScenario, mustBrodcast);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onScenarioUploadAsked(__message, msgId, hlScenario, mustBrodcast);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client request to set a node on a hl scenario.
	void CShareServerEditionItfProxy::onNodeSetAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerServer &value)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeSetAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgId, instanceId, attrName, value);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeSetAsked(__message, msgId, instanceId, attrName, value);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client request to insert a node on a hl scenario.
	void CShareServerEditionItfProxy::onNodeInsertAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerServer &value)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeInsertAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgId, instanceId, attrName, position, key, value);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeInsertAsked(__message, msgId, instanceId, attrName, position, key, value);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client request to erase a node on a hl scenario.
	void CShareServerEditionItfProxy::onNodeEraseAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeEraseAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgId, instanceId, attrName, position);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeEraseAsked(__message, msgId, instanceId, attrName, position);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client request to move a node on a hl scenario.
	void CShareServerEditionItfProxy::onNodeMoveAsked(NLNET::IModule *sender, uint32 msgId, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeMoveAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgId, instanceId1, attrName1, position1, instanceId2, attrName2, position2);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeMoveAsked(__message, msgId, instanceId1, attrName1, position1, instanceId2, attrName2, position2);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Call by the client in order to download its current scenario (and tp).
	void CShareServerEditionItfProxy::onMapConnectionAsked(NLNET::IModule *sender, TSessionId scenarioId, bool updateHighLevel, bool mustTp, R2::TUserRole role)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onMapConnectionAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), scenarioId, updateHighLevel, mustTp, role);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onMapConnectionAsked(__message, scenarioId, updateHighLevel, mustTp, role);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Call by the client when he change its mode (Dm, Tester, Player)
	void CShareServerEditionItfProxy::onCharModeUpdateAsked(NLNET::IModule *sender, R2::TCharMode mode)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onCharModeUpdateAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), mode);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onCharModeUpdateAsked(__message, mode);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// client wants to tp at a specific position (clicking in map)
	void CShareServerEditionItfProxy::onTpPositionAsked(NLNET::IModule *sender, float x, float y, float z)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onTpPositionAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), x, y, z);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onTpPositionAsked(__message, x, y, z);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the mode of the pioneer (DM/TEST).
	void CShareServerEditionItfProxy::tpToEntryPoint(NLNET::IModule *sender, uint32 actIndex)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->tpToEntryPoint(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), actIndex);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_tpToEntryPoint(__message, actIndex);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Set the starting act of the scenario
	void CShareServerEditionItfProxy::setStartingAct(NLNET::IModule *sender, uint32 actIndex)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setStartingAct(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), actIndex);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setStartingAct(__message, actIndex);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the ring access of a scenario.
	void CShareServerEditionItfProxy::onScenarioRingAccessUpdated(NLNET::IModule *sender, bool ok, const std::string &ringAccess, const std::string &errMsg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onScenarioRingAccessUpdated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), ok, ringAccess, errMsg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onScenarioRingAccessUpdated(__message, ok, ringAccess, errMsg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a file waiting to be saved
	void CShareServerEditionItfProxy::saveScenarioFile(NLNET::IModule *sender, const std::string &md5, const R2::TScenarioHeaderSerializer &header)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->saveScenarioFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, header);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_saveScenarioFile(__message, md5, header);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a file waiting to be loaded
	void CShareServerEditionItfProxy::loadScenarioFile(NLNET::IModule *sender, const std::string &md5, const std::string &signature)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->loadScenarioFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, signature);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_loadScenarioFile(__message, md5, signature);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a user component file waiting to be saved
	void CShareServerEditionItfProxy::saveUserComponentFile(NLNET::IModule *sender, const std::string &md5, const R2::TScenarioHeaderSerializer &header)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->saveUserComponentFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, header);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_saveUserComponentFile(__message, md5, header);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a user component file waiting to be loaded
	void CShareServerEditionItfProxy::loadUserComponentFile(NLNET::IModule *sender, const std::string &md5, const std::string &signature)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->loadUserComponentFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, signature);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_loadUserComponentFile(__message, md5, signature);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to ask the dss to teleport a character to another character
	void CShareServerEditionItfProxy::teleportOneCharacterToAnother(NLNET::IModule *sender, TSessionId sessionId, uint32 sourceId, uint32 destId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->teleportOneCharacterToAnother(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, sourceId, destId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_teleportOneCharacterToAnother(__message, sessionId, sourceId, destId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// teleport the player while uploading the scenario
	void CShareServerEditionItfProxy::teleportWhileUploadingScenario(NLNET::IModule *sender, const std::string &island, const std::string &entryPoint, const std::string &season)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->teleportWhileUploadingScenario(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), island, entryPoint, season);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_teleportWhileUploadingScenario(__message, island, entryPoint, season);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// send the header of a multi-part message
	void CShareServerEditionItfProxy::multiPartMsgHead(NLNET::IModule *sender, uint32 charId, const std::string &msgName, uint32 nbPacket, uint32 size)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->multiPartMsgHead(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, msgName, nbPacket, size);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_multiPartMsgHead(__message, charId, msgName, nbPacket, size);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// send a part of a multi-part message
	void CShareServerEditionItfProxy::multiPartMsgBody(NLNET::IModule *sender, uint32 charId, uint32 partId, const std::vector<uint8> &data)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->multiPartMsgBody(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, partId, data);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_multiPartMsgBody(__message, charId, partId, data);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// send the footer of a multi-part message
	void CShareServerEditionItfProxy::multiPartMsgFoot(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->multiPartMsgFoot(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_multiPartMsgFoot(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// simulate the forward of a message (to dss)
	void CShareServerEditionItfProxy::forwardToDss(NLNET::IModule *sender, uint32 charId, const NLNET::CMessage &msg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->forwardToDss(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, msg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_forwardToDss(__message, charId, msg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_startingScenario(NLNET::CMessage &__message)
	{
		__message.setType("RSS1");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_startScenario(NLNET::CMessage &__message, bool ok, const TScenarioHeaderSerializer &header, const CObjectSerializerServer &data, uint32 startingAct)
	{
		__message.setType("RSS2");
			nlWrite(__message, serial, ok);
			nlWrite(__message, serial, const_cast < TScenarioHeaderSerializer& > (header));
			nlWrite(__message, serial, const_cast < CObjectSerializerServer& > (data));
			nlWrite(__message, serial, startingAct);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_advConnACK(NLNET::CMessage &__message)
	{
		__message.setType("ADC_ACK");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onUserComponentRegistered(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5)
	{
		__message.setType("SUCR");
			nlWrite(__message, serial, const_cast < NLMISC::CHashKeyMD5& > (md5));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onUserComponentDownloading(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5)
	{
		__message.setType("SUCD");
			nlWrite(__message, serial, const_cast < NLMISC::CHashKeyMD5& > (md5));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onScenarioUploadAsked(NLNET::CMessage &__message, uint32 msgId, const CObjectSerializerServer &hlScenario, bool mustBrodcast)
	{
		__message.setType("SSUA");
			nlWrite(__message, serial, msgId);
			nlWrite(__message, serial, const_cast < CObjectSerializerServer& > (hlScenario));
			nlWrite(__message, serial, mustBrodcast);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onNodeSetAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerServer &value)
	{
		__message.setType("SNSA");
			nlWrite(__message, serial, msgId);
			nlWrite(__message, serial, const_cast < std::string& > (instanceId));
			nlWrite(__message, serial, const_cast < std::string& > (attrName));
			nlWrite(__message, serial, const_cast < R2::CObjectSerializerServer& > (value));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onNodeInsertAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerServer &value)
	{
		__message.setType("SNIA");
			nlWrite(__message, serial, msgId);
			nlWrite(__message, serial, const_cast < std::string& > (instanceId));
			nlWrite(__message, serial, const_cast < std::string& > (attrName));
			nlWrite(__message, serial, position);
			nlWrite(__message, serial, const_cast < std::string& > (key));
			nlWrite(__message, serial, const_cast < R2::CObjectSerializerServer& > (value));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onNodeEraseAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId, const std::string &attrName, sint32 position)
	{
		__message.setType("SNEA");
			nlWrite(__message, serial, msgId);
			nlWrite(__message, serial, const_cast < std::string& > (instanceId));
			nlWrite(__message, serial, const_cast < std::string& > (attrName));
			nlWrite(__message, serial, position);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onNodeMoveAsked(NLNET::CMessage &__message, uint32 msgId, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2)
	{
		__message.setType("SNMA");
			nlWrite(__message, serial, msgId);
			nlWrite(__message, serial, const_cast < std::string& > (instanceId1));
			nlWrite(__message, serial, const_cast < std::string& > (attrName1));
			nlWrite(__message, serial, position1);
			nlWrite(__message, serial, const_cast < std::string& > (instanceId2));
			nlWrite(__message, serial, const_cast < std::string& > (attrName2));
			nlWrite(__message, serial, position2);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onMapConnectionAsked(NLNET::CMessage &__message, TSessionId scenarioId, bool updateHighLevel, bool mustTp, R2::TUserRole role)
	{
		__message.setType("SMCA");
			nlWrite(__message, serial, scenarioId);
			nlWrite(__message, serial, updateHighLevel);
			nlWrite(__message, serial, mustTp);
			nlWrite(__message, serial, role);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onCharModeUpdateAsked(NLNET::CMessage &__message, R2::TCharMode mode)
	{
		__message.setType("SCMUA");
			nlWrite(__message, serial, mode);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onTpPositionAsked(NLNET::CMessage &__message, float x, float y, float z)
	{
		__message.setType("STPA");
			nlWrite(__message, serial, x);
			nlWrite(__message, serial, y);
			nlWrite(__message, serial, z);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_tpToEntryPoint(NLNET::CMessage &__message, uint32 actIndex)
	{
		__message.setType("TPEP");
			nlWrite(__message, serial, actIndex);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_setStartingAct(NLNET::CMessage &__message, uint32 actIndex)
	{
		__message.setType("STA");
			nlWrite(__message, serial, actIndex);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_onScenarioRingAccessUpdated(NLNET::CMessage &__message, bool ok, const std::string &ringAccess, const std::string &errMsg)
	{
		__message.setType("SRAU");
			nlWrite(__message, serial, ok);
			nlWrite(__message, serial, const_cast < std::string& > (ringAccess));
			nlWrite(__message, serial, const_cast < std::string& > (errMsg));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_saveScenarioFile(NLNET::CMessage &__message, const std::string &md5, const R2::TScenarioHeaderSerializer &header)
	{
		__message.setType("SSSFA");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, const_cast < R2::TScenarioHeaderSerializer& > (header));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_loadScenarioFile(NLNET::CMessage &__message, const std::string &md5, const std::string &signature)
	{
		__message.setType("SLSFA");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, const_cast < std::string& > (signature));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_saveUserComponentFile(NLNET::CMessage &__message, const std::string &md5, const R2::TScenarioHeaderSerializer &header)
	{
		__message.setType("SUCF");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, const_cast < R2::TScenarioHeaderSerializer& > (header));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_loadUserComponentFile(NLNET::CMessage &__message, const std::string &md5, const std::string &signature)
	{
		__message.setType("LUCF");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, const_cast < std::string& > (signature));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_teleportOneCharacterToAnother(NLNET::CMessage &__message, TSessionId sessionId, uint32 sourceId, uint32 destId)
	{
		__message.setType("STOCTA");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, sourceId);
			nlWrite(__message, serial, destId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_teleportWhileUploadingScenario(NLNET::CMessage &__message, const std::string &island, const std::string &entryPoint, const std::string &season)
	{
		__message.setType("STWUS");
			nlWrite(__message, serial, const_cast < std::string& > (island));
			nlWrite(__message, serial, const_cast < std::string& > (entryPoint));
			nlWrite(__message, serial, const_cast < std::string& > (season));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_multiPartMsgHead(NLNET::CMessage &__message, uint32 charId, const std::string &msgName, uint32 nbPacket, uint32 size)
	{
		__message.setType("DSS_HEAD");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, const_cast < std::string& > (msgName));
			nlWrite(__message, serial, nbPacket);
			nlWrite(__message, serial, size);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_multiPartMsgBody(NLNET::CMessage &__message, uint32 charId, uint32 partId, const std::vector<uint8> &data)
	{
		__message.setType("DSS_MSG");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, partId);
			nlWrite(__message, serialCont, const_cast < std::vector<uint8>& > (data));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_multiPartMsgFoot(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("DSS_FOOT");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareServerEditionItfProxy::buildMessageFor_forwardToDss(NLNET::CMessage &__message, uint32 charId, const NLNET::CMessage &msg)
	{
		__message.setType("DSS_FW");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serialMessage, const_cast < NLNET::CMessage& > (msg));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CShareClientEditionItfSkel::TMessageHandlerMap &CShareClientEditionItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("RSS1"), &CShareClientEditionItfSkel::startingScenario_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("RSS2"), &CShareClientEditionItfSkel::startScenario_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUCR"), &CShareClientEditionItfSkel::onUserComponentRegistered_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUCU"), &CShareClientEditionItfSkel::onUserComponentUploading_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CSUA"), &CShareClientEditionItfSkel::onScenarioUploaded_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CNSA"), &CShareClientEditionItfSkel::onNodeSet_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CNIA"), &CShareClientEditionItfSkel::onNodeInserted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CNEA"), &CShareClientEditionItfSkel::onNodeErased_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CNMA"), &CShareClientEditionItfSkel::onNodeMoved_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CQU"), &CShareClientEditionItfSkel::onQuotaUpdated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CCMU"), &CShareClientEditionItfSkel::onCharModeUpdated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CTMD"), &CShareClientEditionItfSkel::onTestModeDisconnected_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CTPPS"), &CShareClientEditionItfSkel::onTpPositionSimulated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("COK"), &CShareClientEditionItfSkel::onKicked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("COD"), &CShareClientEditionItfSkel::onDisconnected_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSA1"), &CShareClientEditionItfSkel::scheduleStartAct_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("OAMC"), &CShareClientEditionItfSkel::onAnimationModePlayConnected_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUSH"), &CShareClientEditionItfSkel::updateScenarioHeader_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUIT"), &CShareClientEditionItfSkel::updateMissionItemsDescription_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUAPD"), &CShareClientEditionItfSkel::updateActPositionDescriptions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUUTD"), &CShareClientEditionItfSkel::updateUserTriggerDescriptions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUAIU"), &CShareClientEditionItfSkel::onCurrentActIndexUpdated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUTAL"), &CShareClientEditionItfSkel::updateTalkingAsList_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUIL"), &CShareClientEditionItfSkel::updateIncarningList_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CSM"), &CShareClientEditionItfSkel::systemMsg_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CRAU"), &CShareClientEditionItfSkel::onRingAccessUpdated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CSSFA"), &CShareClientEditionItfSkel::saveScenarioFileAccepted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CLSFA"), &CShareClientEditionItfSkel::loadScenarioFileAccepted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SUCFA"), &CShareClientEditionItfSkel::saveUserComponentFileAccepted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("LUCFA"), &CShareClientEditionItfSkel::loadUserComponentFileAccepted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DSS_HEAD"), &CShareClientEditionItfSkel::multiPartMsgHead_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DSS_MSG"), &CShareClientEditionItfSkel::multiPartMsgBody_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DSS_FOOT"), &CShareClientEditionItfSkel::multiPartMsgFoot_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CACKMSG"), &CShareClientEditionItfSkel::ackMsg_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CShareClientEditionItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}


	void CShareClientEditionItfSkel::startingScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_startingScenario_RSS1);
		uint32	charId;
			nlRead(__message, serial, charId);
		startingScenario(sender, charId);
	}

	void CShareClientEditionItfSkel::startScenario_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_startScenario_RSS2);
		bool	ok;
			nlRead(__message, serial, ok);
		uint32	startingAct;
			nlRead(__message, serial, startingAct);
		std::string	errorMsg;
			nlRead(__message, serial, errorMsg);
		startScenario(sender, ok, startingAct, errorMsg);
	}

	void CShareClientEditionItfSkel::onUserComponentRegistered_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onUserComponentRegistered_CUCR);
		NLMISC::CHashKeyMD5	md5;
			nlRead(__message, serial, md5);
		onUserComponentRegistered(sender, md5);
	}

	void CShareClientEditionItfSkel::onUserComponentUploading_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onUserComponentUploading_CUCU);
		NLMISC::CHashKeyMD5	md5;
			nlRead(__message, serial, md5);
		onUserComponentUploading(sender, md5);
	}

	void CShareClientEditionItfSkel::onScenarioUploaded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onScenarioUploaded_CSUA);
		R2::CObjectSerializerClient	hlScenario;
			nlRead(__message, serial, hlScenario);
		onScenarioUploaded(sender, hlScenario);
	}

	void CShareClientEditionItfSkel::onNodeSet_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onNodeSet_CNSA);
		std::string	instanceId;
			nlRead(__message, serial, instanceId);
		std::string	attrName;
			nlRead(__message, serial, attrName);
		R2::CObjectSerializerClient	value;
			nlRead(__message, serial, value);
		onNodeSet(sender, instanceId, attrName, value);
	}

	void CShareClientEditionItfSkel::onNodeInserted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onNodeInserted_CNIA);
		std::string	instanceId;
			nlRead(__message, serial, instanceId);
		std::string	attrName;
			nlRead(__message, serial, attrName);
		sint32	position;
			nlRead(__message, serial, position);
		std::string	key;
			nlRead(__message, serial, key);
		R2::CObjectSerializerClient	value;
			nlRead(__message, serial, value);
		onNodeInserted(sender, instanceId, attrName, position, key, value);
	}

	void CShareClientEditionItfSkel::onNodeErased_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onNodeErased_CNEA);
		std::string	instanceId;
			nlRead(__message, serial, instanceId);
		std::string	attrName;
			nlRead(__message, serial, attrName);
		sint32	position;
			nlRead(__message, serial, position);
		onNodeErased(sender, instanceId, attrName, position);
	}

	void CShareClientEditionItfSkel::onNodeMoved_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onNodeMoved_CNMA);
		std::string	instanceId1;
			nlRead(__message, serial, instanceId1);
		std::string	attrName1;
			nlRead(__message, serial, attrName1);
		sint32	position1;
			nlRead(__message, serial, position1);
		std::string	instanceId2;
			nlRead(__message, serial, instanceId2);
		std::string	attrName2;
			nlRead(__message, serial, attrName2);
		sint32	position2;
			nlRead(__message, serial, position2);
		onNodeMoved(sender, instanceId1, attrName1, position1, instanceId2, attrName2, position2);
	}

	void CShareClientEditionItfSkel::onQuotaUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onQuotaUpdated_CQU);
		uint32	maxNpcs;
			nlRead(__message, serial, maxNpcs);
		uint32	maxStaticObjects;
			nlRead(__message, serial, maxStaticObjects);
		onQuotaUpdated(sender, maxNpcs, maxStaticObjects);
	}

	void CShareClientEditionItfSkel::onCharModeUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onCharModeUpdated_CCMU);
		R2::TCharMode	mode;
			nlRead(__message, serial, mode);
		onCharModeUpdated(sender, mode);
	}

	void CShareClientEditionItfSkel::onTestModeDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onTestModeDisconnected_CTMD);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	lastActIndex;
			nlRead(__message, serial, lastActIndex);
		R2::TScenarioSessionType	animationType;
			nlRead(__message, serialEnum, animationType);
		onTestModeDisconnected(sender, sessionId, lastActIndex, animationType);
	}

	void CShareClientEditionItfSkel::onTpPositionSimulated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onTpPositionSimulated_CTPPS);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint64	characterId64;
			nlRead(__message, serial, characterId64);
		sint32	x;
			nlRead(__message, serial, x);
		sint32	y;
			nlRead(__message, serial, y);
		sint32	z;
			nlRead(__message, serial, z);
		uint8	scenarioSeason;
			nlRead(__message, serial, scenarioSeason);
		onTpPositionSimulated(sender, sessionId, characterId64, x, y, z, scenarioSeason);
	}

	void CShareClientEditionItfSkel::onKicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onKicked_COK);
		uint32	timeBeforeDisconnection;
			nlRead(__message, serial, timeBeforeDisconnection);
		bool	mustKick;
			nlRead(__message, serial, mustKick);
		onKicked(sender, timeBeforeDisconnection, mustKick);
	}

	void CShareClientEditionItfSkel::onDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareClientEditionItfSkel_onDisconnected_COD);
		onDisconnected(sender);
	}

	void CShareClientEditionItfSkel::scheduleStartAct_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_scheduleStartAct_SSA1);
		uint32	errorId;
			nlRead(__message, serial, errorId);
		uint32	actId;
			nlRead(__message, serial, actId);
		uint32	nbSeconds;
			nlRead(__message, serial, nbSeconds);
		scheduleStartAct(sender, errorId, actId, nbSeconds);
	}

	void CShareClientEditionItfSkel::onAnimationModePlayConnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareClientEditionItfSkel_onAnimationModePlayConnected_OAMC);
		onAnimationModePlayConnected(sender);
	}

	void CShareClientEditionItfSkel::updateScenarioHeader_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_updateScenarioHeader_CUSH);
		R2::TScenarioHeaderSerializer	scenarioHeader;
			nlRead(__message, serial, scenarioHeader);
		updateScenarioHeader(sender, scenarioHeader);
	}

	void CShareClientEditionItfSkel::updateMissionItemsDescription_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_updateMissionItemsDescription_CUIT);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		std::vector<R2::TMissionItem>	missionItem;
			nlRead(__message, serialCont, missionItem);
		updateMissionItemsDescription(sender, sessionId, missionItem);
	}

	void CShareClientEditionItfSkel::updateActPositionDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_updateActPositionDescriptions_CUAPD);
		R2::TActPositionDescriptions	actPositionDescriptions;
			nlRead(__message, serialCont, actPositionDescriptions);
		updateActPositionDescriptions(sender, actPositionDescriptions);
	}

	void CShareClientEditionItfSkel::updateUserTriggerDescriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_updateUserTriggerDescriptions_CUUTD);
		R2::TUserTriggerDescriptions	userTriggerDescriptions;
			nlRead(__message, serialCont, userTriggerDescriptions);
		updateUserTriggerDescriptions(sender, userTriggerDescriptions);
	}

	void CShareClientEditionItfSkel::onCurrentActIndexUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onCurrentActIndexUpdated_CUAIU);
		uint32	actIndex;
			nlRead(__message, serial, actIndex);
		onCurrentActIndexUpdated(sender, actIndex);
	}

	void CShareClientEditionItfSkel::updateTalkingAsList_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_updateTalkingAsList_CUTAL);
		std::vector<uint32>	botsId;
			nlRead(__message, serialCont, botsId);
		updateTalkingAsList(sender, botsId);
	}

	void CShareClientEditionItfSkel::updateIncarningList_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_updateIncarningList_CUIL);
		std::vector<uint32>	botsId;
			nlRead(__message, serialCont, botsId);
		updateIncarningList(sender, botsId);
	}

	void CShareClientEditionItfSkel::systemMsg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_systemMsg_CSM);
		std::string	msgType;
			nlRead(__message, serial, msgType);
		std::string	who;
			nlRead(__message, serial, who);
		std::string	msg;
			nlRead(__message, serial, msg);
		systemMsg(sender, msgType, who, msg);
	}

	void CShareClientEditionItfSkel::onRingAccessUpdated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_onRingAccessUpdated_CRAU);
		std::string	ringAccess;
			nlRead(__message, serial, ringAccess);
		onRingAccessUpdated(sender, ringAccess);
	}

	void CShareClientEditionItfSkel::saveScenarioFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_saveScenarioFileAccepted_CSSFA);
		std::string	md5;
			nlRead(__message, serial, md5);
		std::string	signature;
			nlRead(__message, serial, signature);
		bool	isAccepted;
			nlRead(__message, serial, isAccepted);
		saveScenarioFileAccepted(sender, md5, signature, isAccepted);
	}

	void CShareClientEditionItfSkel::loadScenarioFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_loadScenarioFileAccepted_CLSFA);
		std::string	md5;
			nlRead(__message, serial, md5);
		bool	ok;
			nlRead(__message, serial, ok);
		loadScenarioFileAccepted(sender, md5, ok);
	}

	void CShareClientEditionItfSkel::saveUserComponentFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_saveUserComponentFileAccepted_SUCFA);
		std::string	md5;
			nlRead(__message, serial, md5);
		std::string	signature;
			nlRead(__message, serial, signature);
		bool	isAccepted;
			nlRead(__message, serial, isAccepted);
		saveUserComponentFileAccepted(sender, md5, signature, isAccepted);
	}

	void CShareClientEditionItfSkel::loadUserComponentFileAccepted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_loadUserComponentFileAccepted_LUCFA);
		std::string	md5;
			nlRead(__message, serial, md5);
		bool	ok;
			nlRead(__message, serial, ok);
		loadUserComponentFileAccepted(sender, md5, ok);
	}

	void CShareClientEditionItfSkel::multiPartMsgHead_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_multiPartMsgHead_DSS_HEAD);
		std::string	msgName;
			nlRead(__message, serial, msgName);
		uint32	nbPacket;
			nlRead(__message, serial, nbPacket);
		uint32	size;
			nlRead(__message, serial, size);
		multiPartMsgHead(sender, msgName, nbPacket, size);
	}

	void CShareClientEditionItfSkel::multiPartMsgBody_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_multiPartMsgBody_DSS_MSG);
		uint32	partId;
			nlRead(__message, serial, partId);
		uint32	packetSize;
			nlRead(__message, serial, packetSize);
		multiPartMsgBody(sender, partId, packetSize);
	}

	void CShareClientEditionItfSkel::multiPartMsgFoot_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CShareClientEditionItfSkel_multiPartMsgFoot_DSS_FOOT);
		multiPartMsgFoot(sender);
	}

	void CShareClientEditionItfSkel::ackMsg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CShareClientEditionItfSkel_ackMsg_CACKMSG);
		uint32	msgId;
			nlRead(__message, serial, msgId);
		bool	ok;
			nlRead(__message, serial, ok);
		ackMsg(sender, msgId, ok);
	}
		// The start of a test has been requested
	void CShareClientEditionItfProxy::startingScenario(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->startingScenario(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_startingScenario(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A Scenario has started
	void CShareClientEditionItfProxy::startScenario(NLNET::IModule *sender, bool ok, uint32 startingAct, const std::string &errorMsg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->startScenario(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), ok, startingAct, errorMsg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_startScenario(__message, ok, startingAct, errorMsg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A User component has been registered
	void CShareClientEditionItfProxy::onUserComponentRegistered(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onUserComponentRegistered(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onUserComponentRegistered(__message, md5);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Request the upload of a component
	void CShareClientEditionItfProxy::onUserComponentUploading(NLNET::IModule *sender, const NLMISC::CHashKeyMD5 &md5)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onUserComponentUploading(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onUserComponentUploading(__message, md5);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client request to upload an hl ata.
	void CShareClientEditionItfProxy::onScenarioUploaded(NLNET::IModule *sender, const R2::CObjectSerializerClient &hlScenario)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onScenarioUploaded(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), hlScenario);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onScenarioUploaded(__message, hlScenario);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The client request to set a node on a hl scenario.
	void CShareClientEditionItfProxy::onNodeSet(NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeSet(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), instanceId, attrName, value);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeSet(__message, instanceId, attrName, value);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The ServerEditionMode inserts a node on a hl scenario.
	void CShareClientEditionItfProxy::onNodeInserted(NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeInserted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), instanceId, attrName, position, key, value);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeInserted(__message, instanceId, attrName, position, key, value);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The ServerEditionMode erases a node on a hl scenario.
	void CShareClientEditionItfProxy::onNodeErased(NLNET::IModule *sender, const std::string &instanceId, const std::string &attrName, sint32 position)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeErased(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), instanceId, attrName, position);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeErased(__message, instanceId, attrName, position);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The ServerEditionMode a move node on a hl scenario.
	void CShareClientEditionItfProxy::onNodeMoved(NLNET::IModule *sender, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onNodeMoved(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), instanceId1, attrName1, position1, instanceId2, attrName2, position2);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onNodeMoved(__message, instanceId1, attrName1, position1, instanceId2, attrName2, position2);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Updates the client quota
	void CShareClientEditionItfProxy::onQuotaUpdated(NLNET::IModule *sender, uint32 maxNpcs, uint32 maxStaticObjects)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onQuotaUpdated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), maxNpcs, maxStaticObjects);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onQuotaUpdated(__message, maxNpcs, maxStaticObjects);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Updates the client Mode (tester, dm, editor, player) be the speed
	void CShareClientEditionItfProxy::onCharModeUpdated(NLNET::IModule *sender, R2::TCharMode mode)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onCharModeUpdated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), mode);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onCharModeUpdated(__message, mode);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Indicates to the client that an animation has stop (animation, play, test)
	void CShareClientEditionItfProxy::onTestModeDisconnected(NLNET::IModule *sender, TSessionId sessionId, uint32 lastActIndex, R2::TScenarioSessionType animationType)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onTestModeDisconnected(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, lastActIndex, animationType);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onTestModeDisconnected(__message, sessionId, lastActIndex, animationType);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to make a local client tp (because egs can not do it)/
	void CShareClientEditionItfProxy::onTpPositionSimulated(NLNET::IModule *sender, TSessionId sessionId, uint64 characterId64, sint32 x, sint32 y, sint32 z, uint8 scenarioSeason)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onTpPositionSimulated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, characterId64, x, y, z, scenarioSeason);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onTpPositionSimulated(__message, sessionId, characterId64, x, y, z, scenarioSeason);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to indicates that the client will be disconnect in secondes./
	void CShareClientEditionItfProxy::onKicked(NLNET::IModule *sender, uint32 timeBeforeDisconnection, bool mustKick)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onKicked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), timeBeforeDisconnection, mustKick);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onKicked(__message, timeBeforeDisconnection, mustKick);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to make to disconnect the client./
	void CShareClientEditionItfProxy::onDisconnected(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onDisconnected(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onDisconnected(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Tell to the client that an act begin in nbSeconds
	void CShareClientEditionItfProxy::scheduleStartAct(NLNET::IModule *sender, uint32 errorId, uint32 actId, uint32 nbSeconds)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->scheduleStartAct(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), errorId, actId, nbSeconds);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_scheduleStartAct(__message, errorId, actId, nbSeconds);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Tell to the client that he is connected in play mode in an animation session
	void CShareClientEditionItfProxy::onAnimationModePlayConnected(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onAnimationModePlayConnected(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onAnimationModePlayConnected(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to update the scenario Header
	void CShareClientEditionItfProxy::updateScenarioHeader(NLNET::IModule *sender, const R2::TScenarioHeaderSerializer &scenarioHeader)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateScenarioHeader(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), scenarioHeader);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateScenarioHeader(__message, scenarioHeader);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to update discription mission item of a scenario
	void CShareClientEditionItfProxy::updateMissionItemsDescription(NLNET::IModule *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateMissionItemsDescription(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, missionItem);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateMissionItemsDescription(__message, sessionId, missionItem);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to update the discription of acts (name and positions)
	void CShareClientEditionItfProxy::updateActPositionDescriptions(NLNET::IModule *sender, const R2::TActPositionDescriptions &actPositionDescriptions)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateActPositionDescriptions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), actPositionDescriptions);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateActPositionDescriptions(__message, actPositionDescriptions);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to update the discription of acts (name and positions)
	void CShareClientEditionItfProxy::updateUserTriggerDescriptions(NLNET::IModule *sender, const R2::TUserTriggerDescriptions &userTriggerDescriptions)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateUserTriggerDescriptions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userTriggerDescriptions);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateUserTriggerDescriptions(__message, userTriggerDescriptions);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to update the discription of acts (name and positions)
	void CShareClientEditionItfProxy::onCurrentActIndexUpdated(NLNET::IModule *sender, uint32 actIndex)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onCurrentActIndexUpdated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), actIndex);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onCurrentActIndexUpdated(__message, actIndex);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the Talking as list.
	void CShareClientEditionItfProxy::updateTalkingAsList(NLNET::IModule *sender, const std::vector<uint32> &botsId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateTalkingAsList(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), botsId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateTalkingAsList(__message, botsId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the Incarning list.
	void CShareClientEditionItfProxy::updateIncarningList(NLNET::IModule *sender, const std::vector<uint32> &botsId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateIncarningList(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), botsId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateIncarningList(__message, botsId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A message that will be printed an client
	void CShareClientEditionItfProxy::systemMsg(NLNET::IModule *sender, const std::string &msgType, const std::string &who, const std::string &msg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->systemMsg(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgType, who, msg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_systemMsg(__message, msgType, who, msg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the ring access of the client
	void CShareClientEditionItfProxy::onRingAccessUpdated(NLNET::IModule *sender, const std::string &ringAccess)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onRingAccessUpdated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), ringAccess);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onRingAccessUpdated(__message, ringAccess);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a file waiting to be saved
	void CShareClientEditionItfProxy::saveScenarioFileAccepted(NLNET::IModule *sender, const std::string &md5, const std::string &signature, bool isAccepted)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->saveScenarioFileAccepted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, signature, isAccepted);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_saveScenarioFileAccepted(__message, md5, signature, isAccepted);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a file waiting to be loaded
	void CShareClientEditionItfProxy::loadScenarioFileAccepted(NLNET::IModule *sender, const std::string &md5, bool ok)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->loadScenarioFileAccepted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, ok);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_loadScenarioFileAccepted(__message, md5, ok);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a user component file waiting to be saved
	void CShareClientEditionItfProxy::saveUserComponentFileAccepted(NLNET::IModule *sender, const std::string &md5, const std::string &signature, bool isAccepted)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->saveUserComponentFileAccepted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, signature, isAccepted);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_saveUserComponentFileAccepted(__message, md5, signature, isAccepted);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// a message to validate a user component file waiting to be loaded
	void CShareClientEditionItfProxy::loadUserComponentFileAccepted(NLNET::IModule *sender, const std::string &md5, bool ok)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->loadUserComponentFileAccepted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), md5, ok);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_loadUserComponentFileAccepted(__message, md5, ok);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// send the header of a multi-part message
	void CShareClientEditionItfProxy::multiPartMsgHead(NLNET::IModule *sender, const std::string &msgName, uint32 nbPacket, uint32 size)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->multiPartMsgHead(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgName, nbPacket, size);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_multiPartMsgHead(__message, msgName, nbPacket, size);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// send a part of a multi-part message
	void CShareClientEditionItfProxy::multiPartMsgBody(NLNET::IModule *sender, uint32 partId, uint32 packetSize)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->multiPartMsgBody(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), partId, packetSize);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_multiPartMsgBody(__message, partId, packetSize);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// send the footer of a multi-part message
	void CShareClientEditionItfProxy::multiPartMsgFoot(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->multiPartMsgFoot(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_multiPartMsgFoot(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// send an ack messag to the client
	void CShareClientEditionItfProxy::ackMsg(NLNET::IModule *sender, uint32 msgId, bool ok)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->ackMsg(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), msgId, ok);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_ackMsg(__message, msgId, ok);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_startingScenario(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("RSS1");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_startScenario(NLNET::CMessage &__message, bool ok, uint32 startingAct, const std::string &errorMsg)
	{
		__message.setType("RSS2");
			nlWrite(__message, serial, ok);
			nlWrite(__message, serial, startingAct);
			nlWrite(__message, serial, const_cast < std::string& > (errorMsg));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onUserComponentRegistered(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5)
	{
		__message.setType("CUCR");
			nlWrite(__message, serial, const_cast < NLMISC::CHashKeyMD5& > (md5));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onUserComponentUploading(NLNET::CMessage &__message, const NLMISC::CHashKeyMD5 &md5)
	{
		__message.setType("CUCU");
			nlWrite(__message, serial, const_cast < NLMISC::CHashKeyMD5& > (md5));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onScenarioUploaded(NLNET::CMessage &__message, const R2::CObjectSerializerClient &hlScenario)
	{
		__message.setType("CSUA");
			nlWrite(__message, serial, const_cast < R2::CObjectSerializerClient& > (hlScenario));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onNodeSet(NLNET::CMessage &__message, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value)
	{
		__message.setType("CNSA");
			nlWrite(__message, serial, const_cast < std::string& > (instanceId));
			nlWrite(__message, serial, const_cast < std::string& > (attrName));
			nlWrite(__message, serial, const_cast < R2::CObjectSerializerClient& > (value));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onNodeInserted(NLNET::CMessage &__message, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value)
	{
		__message.setType("CNIA");
			nlWrite(__message, serial, const_cast < std::string& > (instanceId));
			nlWrite(__message, serial, const_cast < std::string& > (attrName));
			nlWrite(__message, serial, position);
			nlWrite(__message, serial, const_cast < std::string& > (key));
			nlWrite(__message, serial, const_cast < R2::CObjectSerializerClient& > (value));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onNodeErased(NLNET::CMessage &__message, const std::string &instanceId, const std::string &attrName, sint32 position)
	{
		__message.setType("CNEA");
			nlWrite(__message, serial, const_cast < std::string& > (instanceId));
			nlWrite(__message, serial, const_cast < std::string& > (attrName));
			nlWrite(__message, serial, position);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onNodeMoved(NLNET::CMessage &__message, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2)
	{
		__message.setType("CNMA");
			nlWrite(__message, serial, const_cast < std::string& > (instanceId1));
			nlWrite(__message, serial, const_cast < std::string& > (attrName1));
			nlWrite(__message, serial, position1);
			nlWrite(__message, serial, const_cast < std::string& > (instanceId2));
			nlWrite(__message, serial, const_cast < std::string& > (attrName2));
			nlWrite(__message, serial, position2);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onQuotaUpdated(NLNET::CMessage &__message, uint32 maxNpcs, uint32 maxStaticObjects)
	{
		__message.setType("CQU");
			nlWrite(__message, serial, maxNpcs);
			nlWrite(__message, serial, maxStaticObjects);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onCharModeUpdated(NLNET::CMessage &__message, R2::TCharMode mode)
	{
		__message.setType("CCMU");
			nlWrite(__message, serial, mode);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onTestModeDisconnected(NLNET::CMessage &__message, TSessionId sessionId, uint32 lastActIndex, R2::TScenarioSessionType animationType)
	{
		__message.setType("CTMD");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, lastActIndex);
			nlWrite(__message, serialEnum, animationType);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onTpPositionSimulated(NLNET::CMessage &__message, TSessionId sessionId, uint64 characterId64, sint32 x, sint32 y, sint32 z, uint8 scenarioSeason)
	{
		__message.setType("CTPPS");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, characterId64);
			nlWrite(__message, serial, x);
			nlWrite(__message, serial, y);
			nlWrite(__message, serial, z);
			nlWrite(__message, serial, scenarioSeason);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onKicked(NLNET::CMessage &__message, uint32 timeBeforeDisconnection, bool mustKick)
	{
		__message.setType("COK");
			nlWrite(__message, serial, timeBeforeDisconnection);
			nlWrite(__message, serial, mustKick);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onDisconnected(NLNET::CMessage &__message)
	{
		__message.setType("COD");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_scheduleStartAct(NLNET::CMessage &__message, uint32 errorId, uint32 actId, uint32 nbSeconds)
	{
		__message.setType("SSA1");
			nlWrite(__message, serial, errorId);
			nlWrite(__message, serial, actId);
			nlWrite(__message, serial, nbSeconds);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onAnimationModePlayConnected(NLNET::CMessage &__message)
	{
		__message.setType("OAMC");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_updateScenarioHeader(NLNET::CMessage &__message, const R2::TScenarioHeaderSerializer &scenarioHeader)
	{
		__message.setType("CUSH");
			nlWrite(__message, serial, const_cast < R2::TScenarioHeaderSerializer& > (scenarioHeader));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_updateMissionItemsDescription(NLNET::CMessage &__message, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem)
	{
		__message.setType("CUIT");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serialCont, const_cast < std::vector<R2::TMissionItem>& > (missionItem));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_updateActPositionDescriptions(NLNET::CMessage &__message, const R2::TActPositionDescriptions &actPositionDescriptions)
	{
		__message.setType("CUAPD");
			nlWrite(__message, serialCont, const_cast < R2::TActPositionDescriptions& > (actPositionDescriptions));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_updateUserTriggerDescriptions(NLNET::CMessage &__message, const R2::TUserTriggerDescriptions &userTriggerDescriptions)
	{
		__message.setType("CUUTD");
			nlWrite(__message, serialCont, const_cast < R2::TUserTriggerDescriptions& > (userTriggerDescriptions));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onCurrentActIndexUpdated(NLNET::CMessage &__message, uint32 actIndex)
	{
		__message.setType("CUAIU");
			nlWrite(__message, serial, actIndex);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_updateTalkingAsList(NLNET::CMessage &__message, const std::vector<uint32> &botsId)
	{
		__message.setType("CUTAL");
			nlWrite(__message, serialCont, const_cast < std::vector<uint32>& > (botsId));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_updateIncarningList(NLNET::CMessage &__message, const std::vector<uint32> &botsId)
	{
		__message.setType("CUIL");
			nlWrite(__message, serialCont, const_cast < std::vector<uint32>& > (botsId));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_systemMsg(NLNET::CMessage &__message, const std::string &msgType, const std::string &who, const std::string &msg)
	{
		__message.setType("CSM");
			nlWrite(__message, serial, const_cast < std::string& > (msgType));
			nlWrite(__message, serial, const_cast < std::string& > (who));
			nlWrite(__message, serial, const_cast < std::string& > (msg));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_onRingAccessUpdated(NLNET::CMessage &__message, const std::string &ringAccess)
	{
		__message.setType("CRAU");
			nlWrite(__message, serial, const_cast < std::string& > (ringAccess));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_saveScenarioFileAccepted(NLNET::CMessage &__message, const std::string &md5, const std::string &signature, bool isAccepted)
	{
		__message.setType("CSSFA");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, const_cast < std::string& > (signature));
			nlWrite(__message, serial, isAccepted);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_loadScenarioFileAccepted(NLNET::CMessage &__message, const std::string &md5, bool ok)
	{
		__message.setType("CLSFA");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, ok);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_saveUserComponentFileAccepted(NLNET::CMessage &__message, const std::string &md5, const std::string &signature, bool isAccepted)
	{
		__message.setType("SUCFA");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, const_cast < std::string& > (signature));
			nlWrite(__message, serial, isAccepted);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_loadUserComponentFileAccepted(NLNET::CMessage &__message, const std::string &md5, bool ok)
	{
		__message.setType("LUCFA");
			nlWrite(__message, serial, const_cast < std::string& > (md5));
			nlWrite(__message, serial, ok);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_multiPartMsgHead(NLNET::CMessage &__message, const std::string &msgName, uint32 nbPacket, uint32 size)
	{
		__message.setType("DSS_HEAD");
			nlWrite(__message, serial, const_cast < std::string& > (msgName));
			nlWrite(__message, serial, nbPacket);
			nlWrite(__message, serial, size);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_multiPartMsgBody(NLNET::CMessage &__message, uint32 partId, uint32 packetSize)
	{
		__message.setType("DSS_MSG");
			nlWrite(__message, serial, partId);
			nlWrite(__message, serial, packetSize);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_multiPartMsgFoot(NLNET::CMessage &__message)
	{
		__message.setType("DSS_FOOT");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CShareClientEditionItfProxy::buildMessageFor_ackMsg(NLNET::CMessage &__message, uint32 msgId, bool ok)
	{
		__message.setType("CACKMSG");
			nlWrite(__message, serial, msgId);
			nlWrite(__message, serial, ok);


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CIOSRingItfSkel::TMessageHandlerMap &CIOSRingItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("SINFAI"), &CIOSRingItfSkel::storeItemNamesForAIInstance_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CIOSRingItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}


	void CIOSRingItfSkel::storeItemNamesForAIInstance_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CIOSRingItfSkel_storeItemNamesForAIInstance_SINFAI);
		uint32	aiInstance;
			nlRead(__message, serial, aiInstance);
		std::vector < TCharMappedInfo >	itemInfos;
			nlRead(__message, serialCont, itemInfos);
		storeItemNamesForAIInstance(sender, aiInstance, itemInfos);
	}
		// DSS send a list of ring names user item with a AI instance
	void CIOSRingItfProxy::storeItemNamesForAIInstance(NLNET::IModule *sender, uint32 aiInstance, const std::vector < TCharMappedInfo > &itemInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->storeItemNamesForAIInstance(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), aiInstance, itemInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_storeItemNamesForAIInstance(__message, aiInstance, itemInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CIOSRingItfProxy::buildMessageFor_storeItemNamesForAIInstance(NLNET::CMessage &__message, uint32 aiInstance, const std::vector < TCharMappedInfo > &itemInfos)
	{
		__message.setType("SINFAI");
			nlWrite(__message, serial, aiInstance);
			nlWrite(__message, serialCont, const_cast < std::vector < TCharMappedInfo >& > (itemInfos));


		return __message;
	}

}
