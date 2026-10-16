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

#ifndef SPR_SERVER_PATCH_REPOSITORY_H
#define SPR_SERVER_PATCH_REPOSITORY_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// local
#include "administered_module.h"
#include "file_repository.h"


//-----------------------------------------------------------------------------
// namespace PATCHMAN
//-----------------------------------------------------------------------------

namespace PATCHMAN
{

	//-----------------------------------------------------------------------------
	// class CServerPatchRepository
	//-----------------------------------------------------------------------------

	class CServerPatchRepository: 
		public CAdministeredModuleBase,
		public CFileRepository
	{
	public:
		// ctor
		CServerPatchRepository();

		// CModuleBase specialisation implementation
		bool initModule(const NLNET::TParsedCommandLine &initInfo) NL_OVERRIDE;
		void onModuleUp(NLNET::IModuleProxy *module) NL_OVERRIDE;
		void onModuleDown(NLNET::IModuleProxy *module) NL_OVERRIDE;
//		void onProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &msg);
		void onModuleUpdate() NL_OVERRIDE;
		std::string buildModuleManifest() const NL_OVERRIDE;
		static const std::string &getHelperString();

		// make sure module system doesn't misbehave when modules are on the same service
		bool isImmediateDispatchingSupported() const NL_OVERRIDE { return false; }

	protected:
		// IFileRequestValidator specialisation implementation
		bool cbValidateDownloadRequest(const NLNET::IModuleProxy *sender,const std::string &fileName) NL_OVERRIDE;
		bool cbValidateFileInfoRequest(const NLNET::IModuleProxy *sender,const std::string &fileName) NL_OVERRIDE;

		// IFileInfoUpdateListener specialisation implementation
		void cbFileInfoRescanning(const NLMISC::CSString& fileName,uint32 fileSize) NL_OVERRIDE;

	private:
		// private data
		bool _NoUpdate;
		bool _AcceptFileRequests;
		bool _AcceptFileInfoRequests;

		uint32 _ServiceUpdatesPerUpdate;
		uint32 _ServiceUpdatesSinceLastUpdate;
		uint32 _UpdateCount;

	protected:
		// NLMISC_COMMANDs for this module
		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerPatchRepository, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CServerPatchRepository, dump, "Dump the current status", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerPatchRepository, AcceptGetFileRequests, "set false to prohibit response to file requests", "[true|false|1|0]")
			NLMISC_COMMAND_HANDLER_ADD(CServerPatchRepository, AcceptGetInfoRequests, "set false to prohibit response to file info requests", "[true|false|1|0]")
			NLMISC_COMMAND_HANDLER_ADD(CServerPatchRepository, ServiceUpdatesPerUpdate, "number of service loops per update", "[<num updates (default=10)>]")

			// add commands defined in base classes
			NLMISC_COMMAND_HANDLER_ADDS_FOR_FILE_REPOSITORY(CServerPatchRepository)
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(dump);
		NLMISC_CLASS_COMMAND_DECL(AcceptGetFileRequests);
		NLMISC_CLASS_COMMAND_DECL(AcceptGetInfoRequests);
		NLMISC_CLASS_COMMAND_DECL(ServiceUpdatesPerUpdate);
	};

} // end of namespace

#endif
