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

#include "nel/misc/types_nl.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "../server_share/backup_service_itf.h"

#include "backup_service.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;


namespace BS
{


	class CBackupServiceMod: 
		public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public CBackupServiceSkel
	{
	public:

		CBackupServiceMod()
		{
			CBackupServiceSkel::init(this);
		}

//		static const std::string &getInitStringHelp()
//		{
//			static string help("ring_db(host=<mysql_hostname> user=<user> password=<password> base=<database_name>)");
//			return help;
//		}
//
//		bool initModule(const TParsedCommandLine &initInfo)
//		{
//			return CModuleBase::initModule(initInfo);
//
//		}

		void onModuleDown(IModuleProxy *proxy)
		{
			// we must check that this module is not in the listener list
			CBackupService::getInstance()->onModuleDown(proxy);
		}

//		bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//		{
//			if (CBackupServiceSkel::onDispatchMessage(sender, message))
//				return true;
//
//			nlwarning("CBackupServiceMod: Unknown message '%s' received", message.getName().c_str());
//
//			return false;
//		}


		/////////////////////////////////////////////////////////////
		//// CBackupServiceSkel interface impl
		/////////////////////////////////////////////////////////////

		// A module ask to save a file in the backup repository
		void saveFile(NLNET::IModuleProxy *sender, const std::string &fileName, const NLNET::TBinBuffer &data)
		{
			CWriteFile*	access = new CWriteFile(fileName, TRequester(sender), 0, data.getBuffer(), data.getBufferSize());

			access->FailureMode = CWriteFile::MajorFailureIfFileUnwritable | CWriteFile::MajorFailureIfFileUnbackupable;
			access->BackupFile = true;
			access->Append = false;

			CBackupService::getInstance()->FileManager.stackFileAccess(access);

		}

		// A module ask to load a file
		void loadFile(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 requestId)
		{
			CLoadFile* access = new CLoadFile(fileName, TRequester(sender), requestId);

			CBackupService::getInstance()->FileManager.stackFileAccess(access);
		}


		/////////////////////////////////////////////////////////////
		//// commands
		/////////////////////////////////////////////////////////////
	
		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CBackupServiceMod, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CBackupServiceMod, dump, "dump the module internal state", "no param");
//			NLMISC_COMMAND_HANDLER_ADD(CMailForumNotifierFwd, simMailNotify, "Simulate a mail notification", "<charId>");
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			// call the base class dump
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			return true;
		}
	}; 
	
	NLNET_REGISTER_MODULE_FACTORY(CBackupServiceMod, "BackupServiceMod");



} // namespace RSMGR

// force module linking
void forceBackupServiceModLink()
{
}
