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

#ifndef SPD_SERVER_PATCH_DOWNLOADER_H
#define SPD_SERVER_PATCH_DOWNLOADER_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/net/module.h"

// local
#include "file_receiver.h"


//-----------------------------------------------------------------------------
// namespace PATCHMAN
//-----------------------------------------------------------------------------

namespace PATCHMAN
{

	//-----------------------------------------------------------------------------
	// class CServerPatchDownloader
	//-----------------------------------------------------------------------------

	class CServerPatchDownloader: 
		public NLNET::CEmptyModuleServiceBehav<NLNET::CEmptyModuleCommBehav<NLNET::CEmptySocketBehav<NLNET::CModuleBase> > >,
		public CFileReceiver
	{
	public:
		// CModuleBase specialisation implementation
		bool initModule(const NLNET::TParsedCommandLine &initInfo);
		void onModuleUp(NLNET::IModuleProxy *module);
		void onModuleDown(NLNET::IModuleProxy *module);
//		void onProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &msg);
		void onModuleUpdate();
		std::string buildModuleManifest() const;

		// prevent modules from misbehaving when they run on the same service together
		bool isImmediateDispatchingSupported() const { return false; }

		// specialisations of overloadable callback methods
		void cbValidateRequestMatches(TFileRequestMatches& requestMatches);
		void cbFileDownloadSuccess(const NLMISC::CSString& fileName,const NLMISC::CMemStream& data);
		void cbRetryAfterFileDownloadFailure(const NLMISC::CSString& fileName);

	public:
		// remaining public interface
		CServerPatchDownloader();

	private:
		// private data
		NLMISC::CSString _RootDirectory;
		// setup a dummy CAdministeredModuleWrapper object to give us a standardised logging interface
		CAdministeredModuleWrapper _AdministeredModuleStub;

	protected:
		// NLMISC_COMMANDs for this module
		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CServerPatchDownloader, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CServerPatchDownloader, dump, "Dump the current status", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CServerPatchDownloader, getFile, "get a file", "<file name>")
			NLMISC_COMMAND_HANDLER_ADD(CServerPatchDownloader, getFileInfo, "get info on a file", "<file name>")
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(dump);
		NLMISC_CLASS_COMMAND_DECL(getFile);
		NLMISC_CLASS_COMMAND_DECL(getFileInfo);
	};

} // end of namespace

#endif
