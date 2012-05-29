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

#ifndef GUS_MODULE_MANAGER_H
#define GUS_MODULE_MANAGER_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/smart_ptr.h"

#include "gus_module.h"
#include "gus_net_connection.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// class CModuleManager
	//-----------------------------------------------------------------------------

	class CModuleManager
	{
	public:
		// get the singleton instance
		static CModuleManager* getInstance();

	public:
		// add a new module - returns id on success or ~0u on failure
		virtual uint32 addModule(const NLMISC::CSString& name,const NLMISC::CSString& rawArgs)=0;

		// remove a module
		virtual void removeModule(uint32 moduleId)=0;

		// display the set of active modules
		virtual void displayModules() const=0;

		// retrieve a module's unique id
		virtual uint32 getModuleId(const IModule* module) const=0;

		// lookup the module belonging to a given unique id
		virtual TModulePtr lookupModuleById(uint32 id) const=0;

		// get a vector of pointers to the active modules
		typedef std::vector<TModulePtr> TModuleVector;
		virtual void getModules(TModuleVector& result) const=0;
	};
}


//-----------------------------------------------------------------------------
#endif
