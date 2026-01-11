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

#ifndef GUS_MODULE_H
#define GUS_MODULE_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/smart_ptr.h"
#include "nel/net/unified_network.h"

#include "gus_net_types.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// class IModule
	//-----------------------------------------------------------------------------

	class IModule: public NLMISC::CRefCount
	{
	public:
		// Derived classes should only be instanciated by the registerModule() routine
		// the IModule ctor asserts at run time if called outside the context of the
		// registerModule() routine
		IModule();

		// virtual dtor
		virtual ~IModule();

		// init and release methods
		virtual bool initialiseModule(const NLMISC::CSString& rawArgs)=0;
		virtual void release() {}

		// methods called at the service update and tick update
		virtual void tickUpdate(NLMISC::TGameCycle tickNumber) {}
		virtual void serviceUpdate(NLMISC::TTime localTime) {}

		// methods called on reception of serviceUp/ serviceDown messages from the naming service
		virtual void serviceUp(NLNET::TServiceId serviceId,const std::string& serviceName) {}
		virtual void serviceDown(NLNET::TServiceId serviceId,const std::string& serviceName) {}

		// methods called as new modules are registered accross the GUSNET network
		virtual void moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule) {}
		virtual void moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule) {}

		// method called to service incoming messages
		virtual void receiveModuleMessage(GUSNET::CModuleMessage& msg) {}

		// method to retrieve & display the name, description and state of the module
		virtual NLMISC::CSString getState() const=0;
		virtual NLMISC::CSString getName() const=0;
		virtual NLMISC::CSString getParameters() const=0;
		virtual void displayModule() const=0;
	};
	typedef NLMISC::CSmartPtr<IModule> TModulePtr;


	//-----------------------------------------------------------------------------
	// utility Routines
	//-----------------------------------------------------------------------------

	// extract a named parameter from a string
	// input string is formated as: abc(def) ghi(123) klm("this is it:(") nop(a(),b())
	// - extractNamedParameter("abc",s) returns: def
	// - extractNamedParameter("ghi",s) returns: 123
	// - extractNamedParameter("klm",s) returns: "this is it:("
	// - extractNamedParameter("nop",s) returns: a(),b()
	NLMISC::CSString extractNamedParameter(const NLMISC::CSString& argName,NLMISC::CSString rawArgs);

	// extract a named path parameter from a string
	// this is a wrapper round extractNamedParameter which looks after stripping quotes and normalising the
	// path as required - the result is a linux-style path
	// - extractNamedPathParameter("path","name(toto) path("c:\\abcd\\def.ghi") returns: c:/abcd/def.ghi
	NLMISC::CSString extractNamedPathParameter(const NLMISC::CSString& argName,NLMISC::CSString rawArgs);
}

//-----------------------------------------------------------------------------
#endif
