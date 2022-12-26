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

#include "nel/net/unified_network.h"

#include "game_share/singleton_registry.h"
#include "game_share/tick_event_handler.h"
#include "game_share/utils.h"

#include "gus_module_manager.h"
#include "gus_module_factory.h"
#include "gus_net.h"
#include "gus_net_remote_module.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUSNET;


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// class CModuleManagerImplementation
	//-----------------------------------------------------------------------------

	class CModuleManagerImplementation: public IServiceSingleton, public CModuleManager
	{
	public:
		// get hold of the singleton instance
		static CModuleManagerImplementation* getInstance();
//		NLMISC_SAFE_SINGLETON_DECL_PTR(CModuleManagerImplementation);

	private:
		// this is a singleton so prohibit instantiation
		CModuleManagerImplementation();

	public:
		// CModuleManager interface
		uint32 addModule(const NLMISC::CSString& name,const NLMISC::CSString& rawArgs);
		void removeModule(uint32 moduleId);
		void displayModules() const;
		uint32 getModuleId(const IModule* module) const;
		TModulePtr lookupModuleById(uint32 id) const;
		void getModules(TModuleVector& result) const;
		
		// service up and servise down callback managers
		void serviceUp(NLNET::TServiceId serviceId,const std::string& serviceName);
		void serviceDown(NLNET::TServiceId serviceId,const std::string& serviceName);

		// IServiceSingleton interface
		void init();
		void serviceUpdate();
		void tickUpdate();
		void release();

	private:
		// private data
		typedef std::map<uint32,TModulePtr> TModules;
		TModules _Modules;
		typedef std::map<NLNET::TServiceId, CSString> TActiveServices;
		TActiveServices _ActiveServices;
	};


	//-----------------------------------------------------------------------------
	// utilities for CModuleManagerImplementation
	//-----------------------------------------------------------------------------

	static void cbServiceUp(const std::string& serviceName,NLNET::TServiceId serviceId,void*)
	{
		CModuleManagerImplementation::getInstance()->serviceUp(serviceId,serviceName);
	}

	static void cbServiceDown(const std::string& serviceName,NLNET::TServiceId serviceId,void*)
	{
		CModuleManagerImplementation::getInstance()->serviceDown(serviceId,serviceName);
	}


	//-----------------------------------------------------------------------------
	// methods CModuleManagerImplementation
	//-----------------------------------------------------------------------------

	CModuleManagerImplementation* CModuleManagerImplementation::getInstance()
	{
		static CModuleManagerImplementation* ptr=NULL;
		if (ptr==NULL)
			ptr= new CModuleManagerImplementation;
		return ptr;
	}
//	NLMISC_SAFE_SINGLETON_IMPL(CModuleManagerImplementation)

	CModuleManagerImplementation::CModuleManagerImplementation()
	{
	}

	uint32 CModuleManagerImplementation::addModule(const CSString& name,const CSString& rawArgs)
	{
		// build the new module
		TModulePtr theModule= CModuleFactory::getInstance()->buildNewModule(name,rawArgs);

		// if build returned NULL then give up
		if (theModule==NULL)
		{
			nlwarning("Failed to add module: %s (%s)",name.c_str(),rawArgs.c_str());
			return ~0u;
		}

		// generate a unique id
		GUSNET::CRemoteModuleViaConnection dummy;
		uint32 uniqueId= dummy.getUniqueId();

		// append the module to the modules vector
		nlinfo("Adding module %3u: %s (%s)",_Modules.size(),name.c_str(),rawArgs.c_str());
		_Modules[uniqueId]=theModule;

		// inform the net system of the arrival of new module
		CGusNet::getInstance()->registerModule(theModule);

		// call the module's 'serviceUp' callback for all registered services
		for (TActiveServices::iterator it= _ActiveServices.begin();it!=_ActiveServices.end();++it)
		{
			theModule->serviceUp(it->first, it->second);
		}

		return uniqueId;
	}

	void CModuleManagerImplementation::removeModule(uint32 moduleId)
	{
		// get a pointer to the module from its ID
		TModulePtr ptr= lookupModuleById(moduleId);
		if (ptr==NULL)
		{
			nlwarning("Failed to identify the module to remove: %u",moduleId);
			return;
		}

		// have the net singleton spread the word that this module is dead
		CGusNet::getInstance()->unregisterModule(ptr);

		// have the module perform any release operations necessary before removing it
		ptr->release();

		// remove the module from the _Modules container
		_Modules.erase(moduleId);
	}

	void CModuleManagerImplementation::displayModules() const
	{
		if (_Modules.empty())
		{
			nlinfo("No active modules");
			return;
		}

		InfoLog->displayNL("Active modules:");
		for (TModules::const_iterator it=_Modules.begin();it!=_Modules.end();++it)
		{
			InfoLog->display("- %4u:",(*it).first);
			InfoLog->display(" %s",(*it).second->getState().c_str());
			InfoLog->displayNL("");
		}
	}

	uint32 CModuleManagerImplementation::getModuleId(const IModule* module) const
	{
		for (TModules::const_iterator it=_Modules.begin();it!=_Modules.end();++it)
		{
			if ((*it).second==module)
				return (*it).first;
		}
		BOMB("Failed to find requested module",return ~0u);
	}

	TModulePtr CModuleManagerImplementation::lookupModuleById(uint32 id) const
	{
		TModules::const_iterator it= _Modules.find(id);
		if(it==_Modules.end()) return NULL;

		return (*it).second;
	}

	void CModuleManagerImplementation::getModules(TModuleVector& result) const
	{
		result.clear();
		for (TModules::const_iterator it=_Modules.begin();it!=_Modules.end();++it)
		{
			result.push_back((*it).second);
		}
	}

	void CModuleManagerImplementation::init()
	{
		CUnifiedNetwork::getInstance()->setServiceUpCallback("*",cbServiceUp);
		CUnifiedNetwork::getInstance()->setServiceDownCallback("*",cbServiceDown);
	}

	void CModuleManagerImplementation::serviceUpdate()
	{
		TModules tmpModuleContainer= _Modules;
		for (TModules::iterator it=tmpModuleContainer.begin();it!=tmpModuleContainer.end();++it)
		{
			(*it).second->serviceUpdate(CTime::getLocalTime());
		}
	}

	void CModuleManagerImplementation::tickUpdate()
	{
		TModules tmpModuleContainer= _Modules;
		for (TModules::iterator it=tmpModuleContainer.begin();it!=tmpModuleContainer.end();++it)
		{
			(*it).second->tickUpdate(CTickEventHandler::getGameCycle());
		}
	}

	void CModuleManagerImplementation::release()
	{
		TModules tmpModuleContainer= _Modules;
		for (TModules::iterator it=tmpModuleContainer.begin();it!=tmpModuleContainer.end();++it)
		{
			(*it).second->release();
		}
	}

	void CModuleManagerImplementation::serviceUp(NLNET::TServiceId serviceId,const std::string& serviceName)
	{
		_ActiveServices[serviceId]= serviceName;
		TModules tmpModuleContainer= _Modules;
		for (TModules::iterator it=tmpModuleContainer.begin();it!=tmpModuleContainer.end();++it)
		{
			(*it).second->serviceUp(serviceId,serviceName);
		}
	}

	void CModuleManagerImplementation::serviceDown(NLNET::TServiceId serviceId,const std::string& serviceName)
	{
		_ActiveServices.erase(serviceId);
		TModules tmpModuleContainer= _Modules;
		for (TModules::iterator it=tmpModuleContainer.begin();it!=tmpModuleContainer.end();++it)
		{
			(*it).second->serviceDown(serviceId,serviceName);
		}
	}


	//-----------------------------------------------------------------------------
	// CModuleManagerImplementation instantiator
	//-----------------------------------------------------------------------------

	class CModuleManagerImplementationInstantiator
	{
	public:
		CModuleManagerImplementationInstantiator()
		{
			CModuleManagerImplementation::getInstance();
		}
	};
	static CModuleManagerImplementationInstantiator ModuleManagerInstantiator;

	
	//-----------------------------------------------------------------------------
	// methods CModuleManager
	//-----------------------------------------------------------------------------

	CModuleManager* CModuleManager::getInstance()
	{
		return CModuleManagerImplementation::getInstance();
	}
}
