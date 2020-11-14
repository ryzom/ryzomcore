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

#include "game_share/utils.h"
#include "gus_net_messages.h"


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// methods CMsgRegisterModule
	//-----------------------------------------------------------------------------

	CMsgRegisterModule::CMsgRegisterModule()
	{
		_ModuleId= InvalidRemoteModuleId;
	}

	void CMsgRegisterModule::serial(NLMISC::IStream& stream)
	{
		stream.serial(_ModuleId);
		stream.serial(_ModuleName);
		stream.serial(_Parameters);
	}

	void CMsgRegisterModule::setup(uint32 moduleId,const NLMISC::CSString& moduleName,const NLMISC::CSString& parameters)
	{
		_ModuleId= moduleId;
		_ModuleName= moduleName;
		_Parameters= parameters;
	}

	uint32 CMsgRegisterModule::getModuleId() const
	{
		return _ModuleId;
	}

	const NLMISC::CSString& CMsgRegisterModule::getModuleName() const
	{
		return _ModuleName;
	}

	const NLMISC::CSString& CMsgRegisterModule::getParameters() const
	{
		return _Parameters;
	}


	//-----------------------------------------------------------------------------
	// methods CMsgUnregisterModule
	//-----------------------------------------------------------------------------

	CMsgUnregisterModule::CMsgUnregisterModule()
	{
		_ModuleId= InvalidRemoteModuleId;
	}

	void CMsgUnregisterModule::serial(NLMISC::IStream& stream)
	{
		stream.serial(_ModuleId);
	}

	void CMsgUnregisterModule::setup(uint32 moduleId)
	{
		_ModuleId= moduleId;
	}

	uint32 CMsgUnregisterModule::getModuleId() const
	{
		return _ModuleId;
	}
}

//-----------------------------------------------------------------------------
