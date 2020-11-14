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

//nel
#include "nel/misc/command.h"
#include "nel/misc/variable.h"

// game share
#include "game_share/utils.h"
#include "game_share/file_description_container.h"
#include "game_share/singleton_registry.h"

// local
#include "em_event_manager.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

NLMISC::CVariable<string>	ToolsDirectory("EventManager", "EventToolDirectory", "Directory containing event tools", string("./events/tools/"), 0, true);
NLMISC::CVariable<string>	EventDirectory("EventManager", "EventUserDirectory", "Directory containing event files", string("./events/user/"), 0, true);
NLMISC::CVariable<string>	LandsDirectory("EventManager", "EventLandDirectory", "Directory containing event files", string("./events/land/"), 0, true);
NLMISC::CVariable<string>	Shard("EventManager", "ActiveShard", "Shard we are logged into", string(""), 0, true);
NLMISC::CVariable<string>	Login("EventManager", "ActiveShardLogin", "User name we're logged in under", string(""), 0, true);
NLMISC::CVariable<string>	EventName("EventManager", "EventName", "Directory containing event files", string(""), 0, true);
NLMISC::CVariable<string>	ToolsShardName("EventExecutor", "ToolsShardName", "Shard from which tools are to be downloaded", string("ravna"), 0, true);


//-----------------------------------------------------------------------------
// Static routines for storing variables to / reading from a file
//-----------------------------------------------------------------------------

const char* variablesFilename= "em_active_event.txt";

static void readVariablesFromFile()
{
	NLMISC::CSString s;
	if (NLMISC::CFile::fileExists(variablesFilename))
		s.readFromFile(variablesFilename);
	EventName= s.line(0);
	Shard= s.line(1);
	Login= s.line(2);
}

static void writeVariablesToFile()
{
	NLMISC::CSString s;
	s+= EventName.get()+"\n";
	s+= Shard.get()+"\n";
	s+= Login.get()+"\n";
	s.writeToFile(variablesFilename);
}


//-----------------------------------------------------------------------------
// CContestCtrlScript Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(EventManager,emPeek,"display the currently loaded event for a given shard","[<shard>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>0)
		Shard= args[0];

	CEventManager::getInstance()->peekInstalledEvent(Shard.get());

	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emLogin,"login to an event shard","<shard> <user> <pass phrase>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()<3)
		return false;

	Shard=args[0];
	Login=args[1];
	writeVariablesToFile();
	NLMISC::CSString password;
	for (uint32 i=2;i<args.size();++i)
	{
		password+= args[i];
		password+= " ";
	}
	CEventManager::getInstance()->login(args[0],args[1],password.strip());

	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emNewEvent,"create a new event","<continent> <event name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	DROP_IF(args[0]!="fyros" && args[0]!="matis" && args[0]!="tryker" && args[0]!="zorai",
		"Bad continent name (should be fyros / matis / tryker / zorai): "+args[0],	return true);
	NLMISC::CSString continent= args[0];

	EventName= args[1];
	writeVariablesToFile();

	// make sure an event for this name doesn't already exist
	DROP_IF(NLMISC::CFile::fileExists(EventDirectory.get()+EventName.get()+".worldedit"),
		"An event already exists with this name: "+EventName.get(),return true);

	// create the directory tree as required
	NLMISC::CFile::createDirectoryTree(EventDirectory.get()+EventName.get());

	// build a file description record containing all of the event files in a given directory
	CFileDescriptionContainer fdc;
	NLMISC::CVectorSString wildcards;
	wildcards.push_back("*.ref");
	fdc.addFiles(ToolsDirectory.get()+"common"+"/", reinterpret_cast<vector<string> &>(wildcards));
	fdc.addFiles(ToolsDirectory.get()+continent+"/", reinterpret_cast<vector<string> &>(wildcards));

	// run through the files copying them to the new directory
	NLMISC::CSString worldEditFile;
	for (uint32 i=0;i<fdc.size();++i)
	{
		// extract the file name
		NLMISC::CSString filename= NLMISC::CFile::getFilename(fdc[i].FileName.rightCrop(4));
		NLMISC::InfoLog->displayNL("- Creating file: %s",filename.c_str());

		// copy the file
		NLMISC::CSString fileContents;
		fileContents.readFromFile(fdc[i].FileName.c_str());
		fileContents.writeToFile(EventDirectory.get()+EventName.get()+"/"+filename);
	}

	// create the world editor file
	NLMISC::InfoLog->displayNL("- Creating world editor script: %s",(EventName.get()+".worldedit").c_str());
	NLMISC::CSString weFile;
	weFile.readFromFile(ToolsDirectory.get()+continent+"/"+continent+".we_ref");
	weFile=weFile.replace("$event$",(EventName.get()).c_str());
	weFile=weFile.replace("$tools$",(ToolsDirectory.get()).c_str());
	weFile=weFile.replace("$lands$",(LandsDirectory.get()).c_str());
	weFile.writeToFile(EventDirectory.get()+EventName.get()+".worldedit");

	NLMISC::InfoLog->displayNL("Done.");
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emSetEvent,"activate an existing event","<event name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	DROP_IF(!NLMISC::CFile::fileExists(EventDirectory.get()+args[0]+".worldedit"), "Event not found: "+args[0],return true);

	EventName= args[0];
	writeVariablesToFile();

	return true;
}

//NLMISC_CATEGORISED_COMMAND(EventManager,emRestartShard,"restart the given event shard","[<shard>]")
//{
//	CNLSmartLogOverride logOverride(&log);
//
//	if (args.size()>0)
//		Shard= args[0];
//
//	CEventManager::getInstance()->restartShard(Shard.get());
//
//	return true;
//}

NLMISC_CATEGORISED_COMMAND(EventManager,emUpload,"upload an event to a given event shard","[<event name>] [<shard>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>0)
		EventName= args[0];

	if (args.size()>1)
		Shard= args[1];

	// build a file description record containing all of the event files in a given directory
	CFileDescriptionContainer fdc;
	NLMISC::CVectorSString wildcards;
	wildcards.push_back("*.primitive");
	wildcards.push_back("*.txt");
	wildcards.push_back("*.uxt");
	wildcards.push_back("*.gss");
	fdc.addFiles(EventDirectory.get()+"/"+EventName.get()+"/", reinterpret_cast<vector<string> &>(wildcards));

	// prepare to read the files from disk
	NLMISC::InfoLog->displayNL("Scanning disk for file list...");
	NLMISC::CVectorSString fileBodies;

	// load up the files and add them to the data block one by one
	uint32 totalsize=0;
	for (uint32 i=0;i<fdc.size();++i)
	{
		NLMISC::InfoLog->displayNL("- Adding file: %s",fdc[i].FileName.c_str());
		vectAppend(fileBodies).readFromFile(fdc[i].FileName);
		DROP_IF(fileBodies.back().size()!=fdc[i].FileSize,"ERROR: Failed to read file: "+fdc[i].FileName,return true);
		totalsize+= fdc[i].FileSize;
	}

	// have the event manager do the uploading...
	nlinfo("Sending data: %d bytes in %d files",totalsize,fdc.size());
	CEventManager::getInstance()->upload(Shard.get(),EventName.get(),fdc,fileBodies);

	return true;
}


NLMISC_CATEGORISED_COMMAND(EventManager,emUnload,"upload an event to a given event shard","[<shard>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>1)
		return false;

	if (args.size()>0)
		Shard= args[0];

	// build a file description record containing all of the event files in a given directory
	CFileDescriptionContainer fdc;
	NLMISC::CVectorSString fileBodies;

	// have the event manager do the uploading...
	nlinfo("Sending empty event data: 0 bytes in 0 files");
	CEventManager::getInstance()->upload(Shard.get(),"No Event",fdc,fileBodies);

	return true;
}


NLMISC_CATEGORISED_COMMAND(EventManager,emEventStart,"start the event running on a given event shard","[<shard>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>1)
		return false;

	if (args.size()>0)
		Shard= args[0];

	// have the event manager do the work...
	CEventManager::getInstance()->startEvent(Shard.get());

	return true;
}


NLMISC_CATEGORISED_COMMAND(EventManager,emEventStop,"stop the event that is currently running on a given event shard","[<shard>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>1)
		return false;

	if (args.size()>0)
		Shard= args[0];

	// have the event manager do the work...
	CEventManager::getInstance()->stopEvent(Shard.get());

	return true;
}


NLMISC_CATEGORISED_COMMAND(EventManager,emUpdateTools,"update tools installed on local PC","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	// have the event manager do the work...
	CEventManager::getInstance()->updateTools();

	return true;
}


//-----------------------------------------------------------------------------
// Extra commands that open MFC windows
//-----------------------------------------------------------------------------

//#ifdef _WINDOWS
#if 0

#include "mfc/stdafx.h"
#include "nel/misc/win_displayer.h"
#include "nel/net/service.h"
#include "mfc/enter_name.h"
#include "mfc/login_window.h"
#include "mfc/select_event_window.h"
#include "mfc/upload_window.h"
#include "mfc/shard_select_window.h"

class CMFCDummy: public IServiceSingleton
{
public:
	void init()				
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
		static CWinApp App;
		AfxWinInit(GetModuleHandle(NULL), NULL, GetCommandLine(),0);
	}
};
static CMFCDummy MFCDummy;

NLMISC_CATEGORISED_COMMAND(EventManager,emWinLogin,"login to an event shard","")
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CLoginWindow dlg(&parent);
	dlg.Shard	= Shard;
	dlg.Login	= Login;
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emLogin "+dlg.Shard+" "+dlg.Login+" "+dlg.Password).c_str(),log,quiet,human);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emWinNewEvent,"create a new event","")
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CEnterName dlg(&parent);
	dlg.ComboValue= "fyros";
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emNewEvent "+dlg.ComboValue+" "+dlg.Text).c_str(),log,quiet,human);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emWinSetEvent,"activate an existing event","")
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CSelectEventWindow dlg(&parent);
	dlg.Name	= "lastName";
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emSetEvent "+dlg.Name).c_str(),log,quiet,human);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emWinUpload,"upload an event to a given event shard","")
{
	NLMISC::CVectorSString shardNames;
	CEventManager::getInstance()->getShards(shardNames);
	DROP_IF(shardNames.empty(),"You must login before you can upload events",return true);

	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CUploadWindow dlg(&parent);
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emUpload "+dlg.Event+" "+dlg.Shard).c_str(),log,quiet,human);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emWinUnload,"unload the event on a given event shard","")
{
	NLMISC::CVectorSString shardNames;
	CEventManager::getInstance()->getShards(shardNames);
	DROP_IF(shardNames.empty(),"You must login before you can unload events",return true);

	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CShardSelectWindow dlg(&parent);
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emUnload "+dlg.ShardName).c_str(),log,quiet,human);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emWinPeek,"display the currently loaded event for a given shard","")
{
	NLMISC::CVectorSString shardNames;
	CEventManager::getInstance()->getShards(shardNames);
	DROP_IF(shardNames.empty(),"You must login before you can peek at installed events",return true);

	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CShardSelectWindow dlg(&parent);
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emPeek "+dlg.ShardName).c_str(),log,quiet,human);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emWinEventStart,"start the event running on a given event shard","")
{
	NLMISC::CVectorSString shardNames;
	CEventManager::getInstance()->getShards(shardNames);
	DROP_IF(shardNames.empty(),"You must login before you can start or stop events",return true);

	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CShardSelectWindow dlg(&parent);
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emEventStart "+dlg.ShardName).c_str(),log,quiet,human);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(EventManager,emWinEventStop,"stop the event that is currently running on a given event shard","")
{
	NLMISC::CVectorSString shardNames;
	CEventManager::getInstance()->getShards(shardNames);
	DROP_IF(shardNames.empty(),"You must login before you can start or stop events",return true);

	AFX_MANAGE_STATE(AfxGetStaticModuleState ( ));
	NLMISC::CWinDisplayer *windowDisplayer = (NLMISC::CWinDisplayer*)NLNET::IService::getInstance()->WindowDisplayer;
	CWnd parent;
	parent.Attach (windowDisplayer->getHWnd());
	CShardSelectWindow dlg(&parent);
	if (dlg.DoModal ()== IDOK)
	{
		NLMISC::ICommand::execute(NLMISC::CSString("emEventStop "+dlg.ShardName).c_str(),log,quiet,human);
	}
	return true;
}

#endif


//-----------------------------------------------------------------------------
// Dummy class that servs to force reading of last active event name at init time
//-----------------------------------------------------------------------------

class CEMCommandDummy: public IServiceSingleton
{
	void init()				
	{
		readVariablesFromFile();
	}
};
static CEMCommandDummy EMCommandDummy;


//-----------------------------------------------------------------------------
