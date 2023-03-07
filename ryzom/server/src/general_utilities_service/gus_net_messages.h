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

#ifndef GUS_NET_MESSAGES_H
#define GUS_NET_MESSAGES_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"

#include "gus_net.h"


//-----------------------------------------------------------------------------
// MACROS for message names
//-----------------------------------------------------------------------------

// messages from connections to hub
#define REGISTER_MODULE_TO_HUB		"REGISTER_MODULE_TO_HUB"
#define UNREGISTER_MODULE_TO_HUB	"UNREGISTER_MODULE_TO_HUB"

// messages from hubs to connections
#define REGISTER_MODULE_FROM_HUB	"REGISTER_MODULE_FROM_HUB"
#define UNREGISTER_MODULE_FROM_HUB	"UNREGISTER_MODULE_FROM_HUB"

// messages from connections to connectinos via hubs
#define BROADCAST_MSG				"BROADCAST_MSG"
#define MODULE_MSG					"MODULE_MSG"


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// class CMsgRegisterModule
	//-----------------------------------------------------------------------------

	class CMsgRegisterModule
	{
	public:
		// ctor
		CMsgRegisterModule();

		// serial
		void serial(NLMISC::IStream& stream);

		// setting up properties
		void setup(uint32 moduleId,const NLMISC::CSString& name,const NLMISC::CSString& parameters);

		// read accessors
		uint32 getModuleId() const;
		const NLMISC::CSString& getModuleName() const;
		const NLMISC::CSString& getParameters() const;

	private:
		uint32 _ModuleId;
		NLMISC::CSString _ModuleName;
		NLMISC::CSString _Parameters;
	};


	//-----------------------------------------------------------------------------
	// class CMsgUnregisterModule
	//-----------------------------------------------------------------------------

	class CMsgUnregisterModule
	{
	public:
		// ctor
		CMsgUnregisterModule();

		// serial
		void serial(NLMISC::IStream& stream);

		// setting up properties
		void setup(uint32 moduleId);

		// read accessors
		uint32 getModuleId() const;

	private:
		uint32 _ModuleId;
	};
}

//-----------------------------------------------------------------------------
#endif
