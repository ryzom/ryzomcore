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

#include "r2_modules_itf.h"

namespace R2
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CServerEditionItfSkel::TMessageHandlerMap &CServerEditionItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			init = true;
		}

		return handlers;
	}
	bool CServerEditionItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CServerAnimationItfSkel::TMessageHandlerMap &CServerAnimationItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("GSP"), &CServerAnimationItfSkel::getStartParams_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("ASUCAP"), &CServerAnimationItfSkel::askSetUserCharActPosition_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SAEE"), &CServerAnimationItfSkel::activateEasterEgg_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SDSSM"), &CServerAnimationItfSkel::dssMessage_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSSP"), &CServerAnimationItfSkel::setScenarioPoints_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SST"), &CServerAnimationItfSkel::startScenarioTiming_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("EST"), &CServerAnimationItfSkel::endScenarioTiming_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SDEE"), &CServerAnimationItfSkel::deactivateEasterEgg_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SOEEL"), &CServerAnimationItfSkel::onEasterEggLooted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SCTR"), &CServerAnimationItfSkel::onCharTargetReceived_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STOCTO"), &CServerAnimationItfSkel::teleportCharacter_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CRDY"), &CServerAnimationItfSkel::characterReady_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CServerAnimationItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	void CServerAnimationItfSkel::getStartParams_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_getStartParams_GSP);
		uint32	charId;
			nlRead(__message, serial, charId);
		TSessionId	lastStoredSessionId;
			nlRead(__message, serial, lastStoredSessionId);
		getStartParams(sender, charId, lastStoredSessionId);
	}

	void CServerAnimationItfSkel::askSetUserCharActPosition_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_askSetUserCharActPosition_ASUCAP);
		uint32	charId;
			nlRead(__message, serial, charId);
		askSetUserCharActPosition(sender, charId);
	}

	void CServerAnimationItfSkel::activateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_activateEasterEgg_SAEE);
		uint32	easterEggId;
			nlRead(__message, serial, easterEggId);
		TSessionId	scenarioId;
			nlRead(__message, serial, scenarioId);
		uint32	actId;
			nlRead(__message, serial, actId);
		std::string	items;
			nlRead(__message, serial, items);
		float	x;
			nlRead(__message, serial, x);
		float	y;
			nlRead(__message, serial, y);
		float	z;
			nlRead(__message, serial, z);
		float	heading;
			nlRead(__message, serial, heading);
		std::string	grpCtrl;
			nlRead(__message, serial, grpCtrl);
		std::string	name;
			nlRead(__message, serial, name);
		std::string	look;
			nlRead(__message, serial, look);
		activateEasterEgg(sender, easterEggId, scenarioId, actId, items, x, y, z, heading, grpCtrl, name, look);
	}

	void CServerAnimationItfSkel::dssMessage_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_dssMessage_SDSSM);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		std::string	mode;
			nlRead(__message, serial, mode);
		std::string	who;
			nlRead(__message, serial, who);
		std::string	msg;
			nlRead(__message, serial, msg);
		dssMessage(sender, sessionId, mode, who, msg);
	}

	void CServerAnimationItfSkel::setScenarioPoints_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_setScenarioPoints_SSSP);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		float	scenarioPoints;
			nlRead(__message, serial, scenarioPoints);
		setScenarioPoints(sender, sessionId, scenarioPoints);
	}

	void CServerAnimationItfSkel::startScenarioTiming_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_startScenarioTiming_SST);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		startScenarioTiming(sender, sessionId);
	}

	void CServerAnimationItfSkel::endScenarioTiming_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_endScenarioTiming_EST);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		endScenarioTiming(sender, sessionId);
	}

	void CServerAnimationItfSkel::deactivateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_deactivateEasterEgg_SDEE);
		uint32	easterEggId;
			nlRead(__message, serial, easterEggId);
		TSessionId	scenarioId;
			nlRead(__message, serial, scenarioId);
		uint32	actId;
			nlRead(__message, serial, actId);
		deactivateEasterEgg(sender, easterEggId, scenarioId, actId);
	}

	void CServerAnimationItfSkel::onEasterEggLooted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_onEasterEggLooted_SOEEL);
		uint32	eggId;
			nlRead(__message, serial, eggId);
		TSessionId	scenarioId;
			nlRead(__message, serial, scenarioId);
		onEasterEggLooted(sender, eggId, scenarioId);
	}

	void CServerAnimationItfSkel::onCharTargetReceived_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_onCharTargetReceived_SCTR);
		NLMISC::CEntityId	eid;
			nlRead(__message, serial, eid);
		NLMISC::CEntityId	creatureId;
			nlRead(__message, serial, creatureId);
		uint32	creatureAlias;
			nlRead(__message, serial, creatureAlias);
		TDataSetRow	creatureRowId;
			nlRead(__message, serial, creatureRowId);
		ucstring	name;
			nlRead(__message, serial, name);
		uint32	nameId;
			nlRead(__message, serial, nameId);
		std::vector<std::string>	params;
			nlRead(__message, serialCont, params);
		bool	alived;
			nlRead(__message, serial, alived);
		onCharTargetReceived(sender, eid, creatureId, creatureAlias, creatureRowId, name, nameId, params, alived);
	}

	void CServerAnimationItfSkel::teleportCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_teleportCharacter_STOCTO);
		NLMISC::CEntityId	player;
			nlRead(__message, serial, player);
		float	x;
			nlRead(__message, serial, x);
		float	y;
			nlRead(__message, serial, y);
		float	z;
			nlRead(__message, serial, z);
		teleportCharacter(sender, player, x, y, z);
	}

	void CServerAnimationItfSkel::characterReady_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerAnimationItfSkel_characterReady_CRDY);
		NLMISC::CEntityId	charEid;
			nlRead(__message, serial, charEid);
		characterReady(sender, charEid);
	}
		// Ask for the position, season and adventure mode of a connecting character
		// The reply will either give a new position or tell to load the last stored one
		// Used by the EGS of a Ring shard to send the start position to a connecting client
	void CServerAnimationItfProxy::getStartParams(NLNET::IModule *sender, uint32 charId, TSessionId lastStoredSessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->getStartParams(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, lastStoredSessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_getStartParams(__message, charId, lastStoredSessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask to server animation module to re-send usre char entry point
		// The reply call CCharacterControlItf::setUserCharActPosition
	void CServerAnimationItfProxy::askSetUserCharActPosition(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->askSetUserCharActPosition(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_askSetUserCharActPosition(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to activate a scenario generated easter egg
	void CServerAnimationItfProxy::activateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId, uint32 actId, const std::string &items, float x, float y, float z, float heading, const std::string &grpCtrl, const std::string &name, const std::string &look)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->activateEasterEgg(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), easterEggId, scenarioId, actId, items, x, y, z, heading, grpCtrl, name, look);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_activateEasterEgg(__message, easterEggId, scenarioId, actId, items, x, y, z, heading, grpCtrl, name, look);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to make the dss send a message
	void CServerAnimationItfProxy::dssMessage(NLNET::IModule *sender, TSessionId sessionId, const std::string &mode, const std::string &who, const std::string &msg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->dssMessage(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, mode, who, msg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_dssMessage(__message, sessionId, mode, who, msg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to make the dss set the scenario points
	void CServerAnimationItfProxy::setScenarioPoints(NLNET::IModule *sender, TSessionId sessionId, float scenarioPoints)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setScenarioPoints(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, scenarioPoints);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setScenarioPoints(__message, sessionId, scenarioPoints);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to make the dss start scenario timing
	void CServerAnimationItfProxy::startScenarioTiming(NLNET::IModule *sender, TSessionId sessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->startScenarioTiming(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_startScenarioTiming(__message, sessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to make the dss end scenario timing
	void CServerAnimationItfProxy::endScenarioTiming(NLNET::IModule *sender, TSessionId sessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->endScenarioTiming(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_endScenarioTiming(__message, sessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to activate a scenario generated easter egg
	void CServerAnimationItfProxy::deactivateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId, uint32 actId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->deactivateEasterEgg(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), easterEggId, scenarioId, actId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_deactivateEasterEgg(__message, easterEggId, scenarioId, actId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS Message to indicates that an easter egg is looted
	void CServerAnimationItfProxy::onEasterEggLooted(NLNET::IModule *sender, uint32 eggId, TSessionId scenarioId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onEasterEggLooted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), eggId, scenarioId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onEasterEggLooted(__message, eggId, scenarioId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS message to indicates info of the target of a player
	void CServerAnimationItfProxy::onCharTargetReceived(NLNET::IModule *sender, const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId, uint32 creatureAlias, TDataSetRow creatureRowId, const ucstring &name, uint32 nameId, const std::vector<std::string> &params, bool alived)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onCharTargetReceived(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), eid, creatureId, creatureAlias, creatureRowId, name, nameId, params, alived);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onCharTargetReceived(__message, eid, creatureId, creatureAlias, creatureRowId, name, nameId, params, alived);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS message to ask the dss to teleport a character to a position
	void CServerAnimationItfProxy::teleportCharacter(NLNET::IModule *sender, const NLMISC::CEntityId &player, float x, float y, float z)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->teleportCharacter(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), player, x, y, z);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_teleportCharacter(__message, player, x, y, z);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS message to indicates that a character is ready in mirror
	void CServerAnimationItfProxy::characterReady(NLNET::IModule *sender, const NLMISC::CEntityId &charEid)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->characterReady(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEid);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_characterReady(__message, charEid);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_getStartParams(NLNET::CMessage &__message, uint32 charId, TSessionId lastStoredSessionId)
	{
		__message.setType("GSP");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, lastStoredSessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_askSetUserCharActPosition(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("ASUCAP");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_activateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId, uint32 actId, const std::string &items, float x, float y, float z, float heading, const std::string &grpCtrl, const std::string &name, const std::string &look)
	{
		__message.setType("SAEE");
			nlWrite(__message, serial, easterEggId);
			nlWrite(__message, serial, scenarioId);
			nlWrite(__message, serial, actId);
			nlWrite(__message, serial, const_cast < std::string& > (items));
			nlWrite(__message, serial, x);
			nlWrite(__message, serial, y);
			nlWrite(__message, serial, z);
			nlWrite(__message, serial, heading);
			nlWrite(__message, serial, const_cast < std::string& > (grpCtrl));
			nlWrite(__message, serial, const_cast < std::string& > (name));
			nlWrite(__message, serial, const_cast < std::string& > (look));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_dssMessage(NLNET::CMessage &__message, TSessionId sessionId, const std::string &mode, const std::string &who, const std::string &msg)
	{
		__message.setType("SDSSM");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, const_cast < std::string& > (mode));
			nlWrite(__message, serial, const_cast < std::string& > (who));
			nlWrite(__message, serial, const_cast < std::string& > (msg));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_setScenarioPoints(NLNET::CMessage &__message, TSessionId sessionId, float scenarioPoints)
	{
		__message.setType("SSSP");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, scenarioPoints);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_startScenarioTiming(NLNET::CMessage &__message, TSessionId sessionId)
	{
		__message.setType("SST");
			nlWrite(__message, serial, sessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_endScenarioTiming(NLNET::CMessage &__message, TSessionId sessionId)
	{
		__message.setType("EST");
			nlWrite(__message, serial, sessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_deactivateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId, uint32 actId)
	{
		__message.setType("SDEE");
			nlWrite(__message, serial, easterEggId);
			nlWrite(__message, serial, scenarioId);
			nlWrite(__message, serial, actId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_onEasterEggLooted(NLNET::CMessage &__message, uint32 eggId, TSessionId scenarioId)
	{
		__message.setType("SOEEL");
			nlWrite(__message, serial, eggId);
			nlWrite(__message, serial, scenarioId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_onCharTargetReceived(NLNET::CMessage &__message, const NLMISC::CEntityId &eid, const NLMISC::CEntityId &creatureId, uint32 creatureAlias, TDataSetRow creatureRowId, const ucstring &name, uint32 nameId, const std::vector<std::string> &params, bool alived)
	{
		__message.setType("SCTR");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (eid));
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (creatureId));
			nlWrite(__message, serial, creatureAlias);
			nlWrite(__message, serial, creatureRowId);
			nlWrite(__message, serial, const_cast < ucstring& > (name));
			nlWrite(__message, serial, nameId);
			nlWrite(__message, serialCont, const_cast < std::vector<std::string>& > (params));
			nlWrite(__message, serial, alived);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_teleportCharacter(NLNET::CMessage &__message, const NLMISC::CEntityId &player, float x, float y, float z)
	{
		__message.setType("STOCTO");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (player));
			nlWrite(__message, serial, x);
			nlWrite(__message, serial, y);
			nlWrite(__message, serial, z);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerAnimationItfProxy::buildMessageFor_characterReady(NLNET::CMessage &__message, const NLMISC::CEntityId &charEid)
	{
		__message.setType("CRDY");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEid));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CCharacterControlItfSkel::TMessageHandlerMap &CCharacterControlItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("RSP"), &CCharacterControlItfSkel::setUserCharStartParams_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CJAS"), &CCharacterControlItfSkel::charJoinAnimSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CLAS"), &CCharacterControlItfSkel::charLeaveAnimSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("RSPP"), &CCharacterControlItfSkel::setUserCharActPosition_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("AST"), &CCharacterControlItfSkel::animSessionStarted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("ASE"), &CCharacterControlItfSkel::animSessionEnded_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSE"), &CCharacterControlItfSkel::scenarioEnded_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSIT"), &CCharacterControlItfSkel::sendItemDescription_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("AEE"), &CCharacterControlItfSkel::activateEasterEgg_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DEE1"), &CCharacterControlItfSkel::deactivateEasterEgg_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("DEE2"), &CCharacterControlItfSkel::deactivateEasterEggs_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SCT"), &CCharacterControlItfSkel::sendCharTargetToDss_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STPA"), &CCharacterControlItfSkel::onTpPositionAsked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SDC"), &CCharacterControlItfSkel::disconnectChar_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRPS"), &CCharacterControlItfSkel::returnToPreviousSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSPR"), &CCharacterControlItfSkel::setPioneerRight_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STOCTA"), &CCharacterControlItfSkel::teleportOneCharacterToAnother_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STCTN"), &CCharacterControlItfSkel::teleportCharacterToNpc_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SUCCS"), &CCharacterControlItfSkel::setUserCharCurrentSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRLS"), &CCharacterControlItfSkel::reportLinkedSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRUS"), &CCharacterControlItfSkel::reportUnlinkedSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SGRM"), &CCharacterControlItfSkel::giveRewardMessage_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRNC"), &CCharacterControlItfSkel::reportNpcControl_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRSNC"), &CCharacterControlItfSkel::reportStopNpcControl_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SCIRU"), &CCharacterControlItfSkel::subscribeCharacterInRingUniverse_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("UCIRU"), &CCharacterControlItfSkel::unsubscribeCharacterInRingUniverse_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CCharacterControlItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	void CCharacterControlItfSkel::setUserCharStartParams_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_setUserCharStartParams_RSP);
		uint32	charId;
			nlRead(__message, serial, charId);
		CFarPosition	farPos;
			nlRead(__message, serial, farPos);
		bool	reloadPos;
			nlRead(__message, serial, reloadPos);
		uint8	scenarioSeason;
			nlRead(__message, serial, scenarioSeason);
		R2::TUserRole	role;
			nlRead(__message, serial, role);
		setUserCharStartParams(sender, charId, farPos, reloadPos, scenarioSeason, role);
	}

	void CCharacterControlItfSkel::charJoinAnimSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_charJoinAnimSession_CJAS);
		uint32	charId;
			nlRead(__message, serial, charId);
		uint32	sessionId;
			nlRead(__message, serial, sessionId);
		charJoinAnimSession(sender, charId, sessionId);
	}

	void CCharacterControlItfSkel::charLeaveAnimSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_charLeaveAnimSession_CLAS);
		uint32	charId;
			nlRead(__message, serial, charId);
		uint32	sessionId;
			nlRead(__message, serial, sessionId);
		charLeaveAnimSession(sender, charId, sessionId);
	}

	void CCharacterControlItfSkel::setUserCharActPosition_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_setUserCharActPosition_RSPP);
		uint32	charId;
			nlRead(__message, serial, charId);
		CFarPosition	farPos;
			nlRead(__message, serial, farPos);
		uint8	season;
			nlRead(__message, serial, season);
		setUserCharActPosition(sender, charId, farPos, season);
	}

	void CCharacterControlItfSkel::animSessionStarted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_animSessionStarted_AST);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		TRunningScenarioInfo	scenarioInfo;
			nlRead(__message, serial, scenarioInfo);
		animSessionStarted(sender, sessionId, scenarioInfo);
	}

	void CCharacterControlItfSkel::animSessionEnded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_animSessionEnded_ASE);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	scenarioScore;
			nlRead(__message, serial, scenarioScore);
		NLMISC::TTime	timeTaken;
			nlRead(__message, serial, timeTaken);
		animSessionEnded(sender, sessionId, scenarioScore, timeTaken);
	}

	void CCharacterControlItfSkel::scenarioEnded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_scenarioEnded_SSE);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		scenarioEnded(sender, sessionId);
	}

	void CCharacterControlItfSkel::sendItemDescription_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_sendItemDescription_SSIT);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		std::vector<R2::TMissionItem>	missionItem;
			nlRead(__message, serialCont, missionItem);
		sendItemDescription(sender, sessionId, missionItem);
	}

	void CCharacterControlItfSkel::activateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_activateEasterEgg_AEE);
		uint32	easterEggId;
			nlRead(__message, serial, easterEggId);
		TSessionId	scenarioId;
			nlRead(__message, serial, scenarioId);
		uint32	aiInstanceId;
			nlRead(__message, serial, aiInstanceId);
		std::vector<R2::TItemAndQuantity>	items;
			nlRead(__message, serialCont, items);
		CFarPosition	pos;
			nlRead(__message, serial, pos);
		std::string	name;
			nlRead(__message, serial, name);
		std::string	look;
			nlRead(__message, serial, look);
		activateEasterEgg(sender, easterEggId, scenarioId, aiInstanceId, items, pos, name, look);
	}

	void CCharacterControlItfSkel::deactivateEasterEgg_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_deactivateEasterEgg_DEE1);
		uint32	easterEggId;
			nlRead(__message, serial, easterEggId);
		TSessionId	scenarioId;
			nlRead(__message, serial, scenarioId);
		deactivateEasterEgg(sender, easterEggId, scenarioId);
	}

	void CCharacterControlItfSkel::deactivateEasterEggs_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_deactivateEasterEggs_DEE2);
		std::set<uint32>	items;
			nlRead(__message, serialCont, items);
		TSessionId	scenarioId;
			nlRead(__message, serial, scenarioId);
		deactivateEasterEggs(sender, items, scenarioId);
	}

	void CCharacterControlItfSkel::sendCharTargetToDss_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_sendCharTargetToDss_SCT);
		NLMISC::CEntityId	eid;
			nlRead(__message, serial, eid);
		std::vector<std::string>	params;
			nlRead(__message, serialCont, params);
		sendCharTargetToDss(sender, eid, params);
	}

	void CCharacterControlItfSkel::onTpPositionAsked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_onTpPositionAsked_STPA);
		NLMISC::CEntityId	eid;
			nlRead(__message, serial, eid);
		float	x;
			nlRead(__message, serial, x);
		float	y;
			nlRead(__message, serial, y);
		float	z;
			nlRead(__message, serial, z);
		uint8	season;
			nlRead(__message, serial, season);
		R2::TR2TpInfos	teleportInfos;
			nlRead(__message, serial, teleportInfos);
		onTpPositionAsked(sender, eid, x, y, z, season, teleportInfos);
	}

	void CCharacterControlItfSkel::disconnectChar_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_disconnectChar_SDC);
		uint32	charId;
			nlRead(__message, serial, charId);
		disconnectChar(sender, charId);
	}

	void CCharacterControlItfSkel::returnToPreviousSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_returnToPreviousSession_SRPS);
		uint32	charId;
			nlRead(__message, serial, charId);
		returnToPreviousSession(sender, charId);
	}

	void CCharacterControlItfSkel::setPioneerRight_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_setPioneerRight_SSPR);
		uint32	charId;
			nlRead(__message, serial, charId);
		bool	isDM;
			nlRead(__message, serial, isDM);
		setPioneerRight(sender, charId, isDM);
	}

	void CCharacterControlItfSkel::teleportOneCharacterToAnother_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_teleportOneCharacterToAnother_STOCTA);
		uint32	sourceId;
			nlRead(__message, serial, sourceId);
		uint32	destId;
			nlRead(__message, serial, destId);
		uint8	season;
			nlRead(__message, serial, season);
		teleportOneCharacterToAnother(sender, sourceId, destId, season);
	}

	void CCharacterControlItfSkel::teleportCharacterToNpc_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_teleportCharacterToNpc_STCTN);
		uint32	sourceId;
			nlRead(__message, serial, sourceId);
		NLMISC::CEntityId	destEid;
			nlRead(__message, serial, destEid);
		uint8	season;
			nlRead(__message, serial, season);
		teleportCharacterToNpc(sender, sourceId, destEid, season);
	}

	void CCharacterControlItfSkel::setUserCharCurrentSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_setUserCharCurrentSession_SUCCS);
		uint32	charId;
			nlRead(__message, serial, charId);
		TSessionId	oldSessionId;
			nlRead(__message, serial, oldSessionId);
		CFarPosition	respawnPoint;
			nlRead(__message, serial, respawnPoint);
		R2::TUserRole	role;
			nlRead(__message, serial, role);
		setUserCharCurrentSession(sender, charId, oldSessionId, respawnPoint, role);
	}

	void CCharacterControlItfSkel::reportLinkedSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_reportLinkedSession_SRLS);
		TSessionId	editionSession;
			nlRead(__message, serial, editionSession);
		TSessionId	animationSession;
			nlRead(__message, serial, animationSession);
		reportLinkedSession(sender, editionSession, animationSession);
	}

	void CCharacterControlItfSkel::reportUnlinkedSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_reportUnlinkedSession_SRUS);
		TSessionId	editionSession;
			nlRead(__message, serial, editionSession);
		TSessionId	animationSession;
			nlRead(__message, serial, animationSession);
		reportUnlinkedSession(sender, editionSession, animationSession);
	}

	void CCharacterControlItfSkel::giveRewardMessage_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_giveRewardMessage_SGRM);
		TDataSetRow	characterRowId;
			nlRead(__message, serial, characterRowId);
		TDataSetRow	creatureRowId;
			nlRead(__message, serial, creatureRowId);
		std::string	rewardText;
			nlRead(__message, serial, rewardText);
		std::string	rareRewardText;
			nlRead(__message, serial, rareRewardText);
		std::string	inventoryFullText;
			nlRead(__message, serial, inventoryFullText);
		std::string	notEnoughPointsText;
			nlRead(__message, serial, notEnoughPointsText);
		giveRewardMessage(sender, characterRowId, creatureRowId, rewardText, rareRewardText, inventoryFullText, notEnoughPointsText);
	}

	void CCharacterControlItfSkel::reportNpcControl_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_reportNpcControl_SRNC);
		NLMISC::CEntityId	playerEid;
			nlRead(__message, serial, playerEid);
		NLMISC::CEntityId	botEid;
			nlRead(__message, serial, botEid);
		reportNpcControl(sender, playerEid, botEid);
	}

	void CCharacterControlItfSkel::reportStopNpcControl_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_reportStopNpcControl_SRSNC);
		NLMISC::CEntityId	playerEid;
			nlRead(__message, serial, playerEid);
		NLMISC::CEntityId	botEid;
			nlRead(__message, serial, botEid);
		reportStopNpcControl(sender, playerEid, botEid);
	}

	void CCharacterControlItfSkel::subscribeCharacterInRingUniverse_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_subscribeCharacterInRingUniverse_SCIRU);
		uint32	charId;
			nlRead(__message, serial, charId);
		subscribeCharacterInRingUniverse(sender, charId);
	}

	void CCharacterControlItfSkel::unsubscribeCharacterInRingUniverse_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterControlItfSkel_unsubscribeCharacterInRingUniverse_UCIRU);
		uint32	charId;
			nlRead(__message, serial, charId);
		unsubscribeCharacterInRingUniverse(sender, charId);
	}
		// The reply of CServerAnimationItf::getStartParams. If reloadPos is true,
		// the character will start from his current saved pos, otherwise the character
		// will start at farPos. In all cases farPos is the respawn point to set.
	void CCharacterControlItfProxy::setUserCharStartParams(NLNET::IModule *sender, uint32 charId, const CFarPosition &farPos, bool reloadPos, uint8 scenarioSeason, R2::TUserRole role)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setUserCharStartParams(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, farPos, reloadPos, scenarioSeason, role);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setUserCharStartParams(__message, charId, farPos, reloadPos, scenarioSeason, role);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A character enter an anim session as player
	void CCharacterControlItfProxy::charJoinAnimSession(NLNET::IModule *sender, uint32 charId, uint32 sessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->charJoinAnimSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, sessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_charJoinAnimSession(__message, charId, sessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A character leave an anim session as player
	void CCharacterControlItfProxy::charLeaveAnimSession(NLNET::IModule *sender, uint32 charId, uint32 sessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->charLeaveAnimSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, sessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_charLeaveAnimSession(__message, charId, sessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The reply of CServerAnimationItf::startAct telling to teleport user
	void CCharacterControlItfProxy::setUserCharActPosition(NLNET::IModule *sender, uint32 charId, const CFarPosition &farPos, uint8 season)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setUserCharActPosition(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, farPos, season);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setUserCharActPosition(__message, charId, farPos, season);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS to EGS signal that an anim session is started
	void CCharacterControlItfProxy::animSessionStarted(NLNET::IModule *sender, TSessionId sessionId, const TRunningScenarioInfo &scenarioInfo)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->animSessionStarted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, scenarioInfo);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_animSessionStarted(__message, sessionId, scenarioInfo);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS to EGS signal that an anim session is ended
	void CCharacterControlItfProxy::animSessionEnded(NLNET::IModule *sender, TSessionId sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->animSessionEnded(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, scenarioScore, timeTaken);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_animSessionEnded(__message, sessionId, scenarioScore, timeTaken);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to signal that a session is ended
	void CCharacterControlItfProxy::scenarioEnded(NLNET::IModule *sender, TSessionId sessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->scenarioEnded(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_scenarioEnded(__message, sessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A DSS Message to register mission item of a scenario
	void CCharacterControlItfProxy::sendItemDescription(NLNET::IModule *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->sendItemDescription(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, missionItem);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_sendItemDescription(__message, sessionId, missionItem);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to activate a scenario generated easter egg
	void CCharacterControlItfProxy::activateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector<R2::TItemAndQuantity> &items, const CFarPosition &pos, const std::string &name, const std::string &look)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->activateEasterEgg(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), easterEggId, scenarioId, aiInstanceId, items, pos, name, look);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_activateEasterEgg(__message, easterEggId, scenarioId, aiInstanceId, items, pos, name, look);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to deactivate a scenario generated easter egg
	void CCharacterControlItfProxy::deactivateEasterEgg(NLNET::IModule *sender, uint32 easterEggId, TSessionId scenarioId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->deactivateEasterEgg(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), easterEggId, scenarioId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_deactivateEasterEgg(__message, easterEggId, scenarioId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to deactivate a multiple easterEgg scenario generated easter egg
	void CCharacterControlItfProxy::deactivateEasterEggs(NLNET::IModule *sender, const std::set<uint32> &items, TSessionId scenarioId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->deactivateEasterEggs(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), items, scenarioId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_deactivateEasterEggs(__message, items, scenarioId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to ask info of the target of a player
	void CCharacterControlItfProxy::sendCharTargetToDss(NLNET::IModule *sender, const NLMISC::CEntityId &eid, const std::vector<std::string> &params)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->sendCharTargetToDss(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), eid, params);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_sendCharTargetToDss(__message, eid, params);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to ask the tp of a pioneer
	void CCharacterControlItfProxy::onTpPositionAsked(NLNET::IModule *sender, const NLMISC::CEntityId &eid, float x, float y, float z, uint8 season, const R2::TR2TpInfos &teleportInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onTpPositionAsked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), eid, x, y, z, season, teleportInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_onTpPositionAsked(__message, eid, x, y, z, season, teleportInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to ask to disconnect a char
	void CCharacterControlItfProxy::disconnectChar(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->disconnectChar(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_disconnectChar(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to ask the egs to return a player to mainland
	void CCharacterControlItfProxy::returnToPreviousSession(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->returnToPreviousSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_returnToPreviousSession(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to ask the egs to set DM righ (aggro, visible, god)
	void CCharacterControlItfProxy::setPioneerRight(NLNET::IModule *sender, uint32 charId, bool isDM)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setPioneerRight(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, isDM);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setPioneerRight(__message, charId, isDM);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to ask the egs to teleport a character to another character
	void CCharacterControlItfProxy::teleportOneCharacterToAnother(NLNET::IModule *sender, uint32 sourceId, uint32 destId, uint8 season)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->teleportOneCharacterToAnother(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sourceId, destId, season);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_teleportOneCharacterToAnother(__message, sourceId, destId, season);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to ask the egs to teleport a character to a npc
	void CCharacterControlItfProxy::teleportCharacterToNpc(NLNET::IModule *sender, uint32 sourceId, const NLMISC::CEntityId &destEid, uint8 season)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->teleportCharacterToNpc(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sourceId, destEid, season);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_teleportCharacterToNpc(__message, sourceId, destEid, season);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to update the respawn point
	void CCharacterControlItfProxy::setUserCharCurrentSession(NLNET::IModule *sender, uint32 charId, TSessionId oldSessionId, const CFarPosition &respawnPoint, R2::TUserRole role)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setUserCharCurrentSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, oldSessionId, respawnPoint, role);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setUserCharCurrentSession(__message, charId, oldSessionId, respawnPoint, role);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to indicates to the egs that session may be linked
	void CCharacterControlItfProxy::reportLinkedSession(NLNET::IModule *sender, TSessionId editionSession, TSessionId animationSession)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportLinkedSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), editionSession, animationSession);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportLinkedSession(__message, editionSession, animationSession);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to indicates to the egs that linked session are no more linked
	void CCharacterControlItfProxy::reportUnlinkedSession(NLNET::IModule *sender, TSessionId editionSession, TSessionId animationSession)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportUnlinkedSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), editionSession, animationSession);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportUnlinkedSession(__message, editionSession, animationSession);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to give some reward to the player
	void CCharacterControlItfProxy::giveRewardMessage(NLNET::IModule *sender, TDataSetRow characterRowId, TDataSetRow creatureRowId, const std::string &rewardText, const std::string &rareRewardText, const std::string &inventoryFullText, const std::string &notEnoughPointsText)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->giveRewardMessage(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), characterRowId, creatureRowId, rewardText, rareRewardText, inventoryFullText, notEnoughPointsText);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_giveRewardMessage(__message, characterRowId, creatureRowId, rewardText, rareRewardText, inventoryFullText, notEnoughPointsText);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to indicates that a bot is being controled
	void CCharacterControlItfProxy::reportNpcControl(NLNET::IModule *sender, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportNpcControl(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), playerEid, botEid);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportNpcControl(__message, playerEid, botEid);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AIS Message to indicates that a bot is stop being controled
	void CCharacterControlItfProxy::reportStopNpcControl(NLNET::IModule *sender, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportStopNpcControl(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), playerEid, botEid);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportStopNpcControl(__message, playerEid, botEid);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS ask to put a character in the ring universe channel
		// This is for editors and animator characters only
	void CCharacterControlItfProxy::subscribeCharacterInRingUniverse(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->subscribeCharacterInRingUniverse(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_subscribeCharacterInRingUniverse(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS ask to remove a character from the ring universe channel
		// This is for editors and animator characters only
	void CCharacterControlItfProxy::unsubscribeCharacterInRingUniverse(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->unsubscribeCharacterInRingUniverse(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_unsubscribeCharacterInRingUniverse(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_setUserCharStartParams(NLNET::CMessage &__message, uint32 charId, const CFarPosition &farPos, bool reloadPos, uint8 scenarioSeason, R2::TUserRole role)
	{
		__message.setType("RSP");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, const_cast < CFarPosition& > (farPos));
			nlWrite(__message, serial, reloadPos);
			nlWrite(__message, serial, scenarioSeason);
			nlWrite(__message, serial, role);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_charJoinAnimSession(NLNET::CMessage &__message, uint32 charId, uint32 sessionId)
	{
		__message.setType("CJAS");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, sessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_charLeaveAnimSession(NLNET::CMessage &__message, uint32 charId, uint32 sessionId)
	{
		__message.setType("CLAS");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, sessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_setUserCharActPosition(NLNET::CMessage &__message, uint32 charId, const CFarPosition &farPos, uint8 season)
	{
		__message.setType("RSPP");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, const_cast < CFarPosition& > (farPos));
			nlWrite(__message, serial, season);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_animSessionStarted(NLNET::CMessage &__message, TSessionId sessionId, const TRunningScenarioInfo &scenarioInfo)
	{
		__message.setType("AST");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, const_cast < TRunningScenarioInfo& > (scenarioInfo));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_animSessionEnded(NLNET::CMessage &__message, TSessionId sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken)
	{
		__message.setType("ASE");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, scenarioScore);
			nlWrite(__message, serial, timeTaken);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_scenarioEnded(NLNET::CMessage &__message, TSessionId sessionId)
	{
		__message.setType("SSE");
			nlWrite(__message, serial, sessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_sendItemDescription(NLNET::CMessage &__message, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem)
	{
		__message.setType("SSIT");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serialCont, const_cast < std::vector<R2::TMissionItem>& > (missionItem));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_activateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector<R2::TItemAndQuantity> &items, const CFarPosition &pos, const std::string &name, const std::string &look)
	{
		__message.setType("AEE");
			nlWrite(__message, serial, easterEggId);
			nlWrite(__message, serial, scenarioId);
			nlWrite(__message, serial, aiInstanceId);
			nlWrite(__message, serialCont, const_cast < std::vector<R2::TItemAndQuantity>& > (items));
			nlWrite(__message, serial, const_cast < CFarPosition& > (pos));
			nlWrite(__message, serial, const_cast < std::string& > (name));
			nlWrite(__message, serial, const_cast < std::string& > (look));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_deactivateEasterEgg(NLNET::CMessage &__message, uint32 easterEggId, TSessionId scenarioId)
	{
		__message.setType("DEE1");
			nlWrite(__message, serial, easterEggId);
			nlWrite(__message, serial, scenarioId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_deactivateEasterEggs(NLNET::CMessage &__message, const std::set<uint32> &items, TSessionId scenarioId)
	{
		__message.setType("DEE2");
			nlWrite(__message, serialCont, const_cast < std::set<uint32>& > (items));
			nlWrite(__message, serial, scenarioId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_sendCharTargetToDss(NLNET::CMessage &__message, const NLMISC::CEntityId &eid, const std::vector<std::string> &params)
	{
		__message.setType("SCT");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (eid));
			nlWrite(__message, serialCont, const_cast < std::vector<std::string>& > (params));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_onTpPositionAsked(NLNET::CMessage &__message, const NLMISC::CEntityId &eid, float x, float y, float z, uint8 season, const R2::TR2TpInfos &teleportInfos)
	{
		__message.setType("STPA");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (eid));
			nlWrite(__message, serial, x);
			nlWrite(__message, serial, y);
			nlWrite(__message, serial, z);
			nlWrite(__message, serial, season);
			nlWrite(__message, serial, const_cast < R2::TR2TpInfos& > (teleportInfos));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_disconnectChar(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("SDC");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_returnToPreviousSession(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("SRPS");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_setPioneerRight(NLNET::CMessage &__message, uint32 charId, bool isDM)
	{
		__message.setType("SSPR");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, isDM);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_teleportOneCharacterToAnother(NLNET::CMessage &__message, uint32 sourceId, uint32 destId, uint8 season)
	{
		__message.setType("STOCTA");
			nlWrite(__message, serial, sourceId);
			nlWrite(__message, serial, destId);
			nlWrite(__message, serial, season);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_teleportCharacterToNpc(NLNET::CMessage &__message, uint32 sourceId, const NLMISC::CEntityId &destEid, uint8 season)
	{
		__message.setType("STCTN");
			nlWrite(__message, serial, sourceId);
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (destEid));
			nlWrite(__message, serial, season);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_setUserCharCurrentSession(NLNET::CMessage &__message, uint32 charId, TSessionId oldSessionId, const CFarPosition &respawnPoint, R2::TUserRole role)
	{
		__message.setType("SUCCS");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, oldSessionId);
			nlWrite(__message, serial, const_cast < CFarPosition& > (respawnPoint));
			nlWrite(__message, serial, role);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_reportLinkedSession(NLNET::CMessage &__message, TSessionId editionSession, TSessionId animationSession)
	{
		__message.setType("SRLS");
			nlWrite(__message, serial, editionSession);
			nlWrite(__message, serial, animationSession);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_reportUnlinkedSession(NLNET::CMessage &__message, TSessionId editionSession, TSessionId animationSession)
	{
		__message.setType("SRUS");
			nlWrite(__message, serial, editionSession);
			nlWrite(__message, serial, animationSession);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_giveRewardMessage(NLNET::CMessage &__message, TDataSetRow characterRowId, TDataSetRow creatureRowId, const std::string &rewardText, const std::string &rareRewardText, const std::string &inventoryFullText, const std::string &notEnoughPointsText)
	{
		__message.setType("SGRM");
			nlWrite(__message, serial, characterRowId);
			nlWrite(__message, serial, creatureRowId);
			nlWrite(__message, serial, const_cast < std::string& > (rewardText));
			nlWrite(__message, serial, const_cast < std::string& > (rareRewardText));
			nlWrite(__message, serial, const_cast < std::string& > (inventoryFullText));
			nlWrite(__message, serial, const_cast < std::string& > (notEnoughPointsText));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_reportNpcControl(NLNET::CMessage &__message, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid)
	{
		__message.setType("SRNC");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (playerEid));
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (botEid));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_reportStopNpcControl(NLNET::CMessage &__message, const NLMISC::CEntityId &playerEid, const NLMISC::CEntityId &botEid)
	{
		__message.setType("SRSNC");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (playerEid));
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (botEid));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_subscribeCharacterInRingUniverse(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("SCIRU");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterControlItfProxy::buildMessageFor_unsubscribeCharacterInRingUniverse(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("UCIRU");
			nlWrite(__message, serial, charId);


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CAisControlItfSkel::TMessageHandlerMap &CAisControlItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			init = true;
		}

		return handlers;
	}
	bool CAisControlItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CR2SessionBackupModuleItfSkel::TMessageHandlerMap &CR2SessionBackupModuleItfSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("SRDS"), &CR2SessionBackupModuleItfSkel::reportDeletedSessions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRHS"), &CR2SessionBackupModuleItfSkel::reportHibernatedSessions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRSS"), &CR2SessionBackupModuleItfSkel::reportSavedSessions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SRDSS"), &CR2SessionBackupModuleItfSkel::registerDss_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CR2SessionBackupModuleItfSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	void CR2SessionBackupModuleItfSkel::reportDeletedSessions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CR2SessionBackupModuleItfSkel_reportDeletedSessions_SRDS);
		std::vector<TSessionId>	sessionIds;
			nlRead(__message, serialCont, sessionIds);
		reportDeletedSessions(sender, sessionIds);
	}

	void CR2SessionBackupModuleItfSkel::reportHibernatedSessions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CR2SessionBackupModuleItfSkel_reportHibernatedSessions_SRHS);
		std::vector<TSessionId>	sessionIds;
			nlRead(__message, serialCont, sessionIds);
		reportHibernatedSessions(sender, sessionIds);
	}

	void CR2SessionBackupModuleItfSkel::reportSavedSessions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CR2SessionBackupModuleItfSkel_reportSavedSessions_SRSS);
		std::vector< TR2SbmSessionInfo >	sessionInfos;
			nlRead(__message, serialCont, sessionInfos);
		reportSavedSessions(sender, sessionInfos);
	}

	void CR2SessionBackupModuleItfSkel::registerDss_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CR2SessionBackupModuleItfSkel_registerDss_SRDSS);
		TShardId	shardId;
			nlRead(__message, serial, shardId);
		registerDss(sender, shardId);
	}
		// DSS message to report session backup that have been deleteed
	void CR2SessionBackupModuleItfProxy::reportDeletedSessions(NLNET::IModule *sender, const std::vector<TSessionId> &sessionIds)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportDeletedSessions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionIds);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportDeletedSessions(__message, sessionIds);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to report session backup that have been hibernated
	void CR2SessionBackupModuleItfProxy::reportHibernatedSessions(NLNET::IModule *sender, const std::vector<TSessionId> &sessionIds)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportHibernatedSessions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionIds);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportHibernatedSessions(__message, sessionIds);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to report session backup that have been deleteed
	void CR2SessionBackupModuleItfProxy::reportSavedSessions(NLNET::IModule *sender, const std::vector< TR2SbmSessionInfo > &sessionInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportSavedSessions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportSavedSessions(__message, sessionInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// DSS message to register itself to the R2SBM
	void CR2SessionBackupModuleItfProxy::registerDss(NLNET::IModule *sender, TShardId shardId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerDss(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_registerDss(__message, shardId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CR2SessionBackupModuleItfProxy::buildMessageFor_reportDeletedSessions(NLNET::CMessage &__message, const std::vector<TSessionId> &sessionIds)
	{
		__message.setType("SRDS");
			nlWrite(__message, serialCont, const_cast < std::vector<TSessionId>& > (sessionIds));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CR2SessionBackupModuleItfProxy::buildMessageFor_reportHibernatedSessions(NLNET::CMessage &__message, const std::vector<TSessionId> &sessionIds)
	{
		__message.setType("SRHS");
			nlWrite(__message, serialCont, const_cast < std::vector<TSessionId>& > (sessionIds));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CR2SessionBackupModuleItfProxy::buildMessageFor_reportSavedSessions(NLNET::CMessage &__message, const std::vector< TR2SbmSessionInfo > &sessionInfos)
	{
		__message.setType("SRSS");
			nlWrite(__message, serialCont, const_cast < std::vector< TR2SbmSessionInfo >& > (sessionInfos));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CR2SessionBackupModuleItfProxy::buildMessageFor_registerDss(NLNET::CMessage &__message, TShardId shardId)
	{
		__message.setType("SRDSS");
			nlWrite(__message, serial, shardId);


		return __message;
	}

}
