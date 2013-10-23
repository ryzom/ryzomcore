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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.

// Misc
#include "nel/misc/file.h"
#include "nel/misc/async_file_manager.h"
// 3d
#include "nel/3d/u_text_context.h"

// Client
#include "debug_client.h"
#include "graph.h"
#include "client_cfg.h"
#include "net_manager.h"
#include "user_entity.h"
#include "view.h"
#include "login.h"
#include "game_share/ryzom_version.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/sphrase_manager.h"
#include "entities.h"
#include "nel/gui/lua_helper.h"
using namespace NLGUI;
#include "character_cl.h"
#include "r2/editor.h"
#include "r2/dmc/client_edition_module.h"
#include "nel/gui/lua_manager.h"

///////////
// USING //
///////////
using namespace NL3D;
using namespace NLMISC;
using namespace std;
using namespace R2;


////////////
// GLOBAL //
////////////
#if defined(NL_OS_WINDOWS)
MEMORYSTATUS MemoryStatus;
#endif
bool	FreezeGraph = false;

//the NEL 3d textcontext
extern NL3D::UTextContext *TextContext;
extern uint8 ShowInfos;		// 0=no info 1=text info 2=graph info 3=streaming info 4=fps only
extern UDriver			*Driver;

/// domain server version for patch
extern string	R2ServerVersion;

#define DIV 1024
#define WIDTH 7
static const char *divisor = "K";

// Check if the stack is empty (of debug str not infos).
uint						DebugStackEmpty = true;
// vector that contains debug strings.
std::vector<std::string>	DebugStack;

COFile DebugFile;
bool IsDebugFile = false;
// Verbose mode about the animation of the selection.
bool VerboseAnimSelection	= false;
// Verbose mode about the animation of the user.
bool VerboseAnimUser		= false;
// Verbose Mode about visual properties.
bool VerboseVP				= false;
// Slot of the entity to display with the debug page.
CLFECOMMON::TCLEntityId WatchedEntitySlot = CLFECOMMON::INVALID_SLOT;


// To Display some Debug information
uint32 Verbose = 0;

// Time in main loop
uint64 IngameEnterTime = 0;

///////////////
// FUNCTIONS //
///////////////
//-----------------------------------------------
// pushInfoStr :
// Push a string in a debug stack but will display only if there is a debug string in the stack when flush.
//-----------------------------------------------
void pushInfoStr(const std::string &str)
{
	// Add the string to the debug stack.
	if (!ClientCfg.Light)
		DebugStack.push_back(str);
}// pushInfoStr //

//-----------------------------------------------
// pushDebugStr :
// Push a string in a debug stack.
//-----------------------------------------------
void pushDebugStr(const std::string &str)
{
	if (!ClientCfg.Light)
	{
		// Add the string to the debug stack.
		DebugStack.push_back("=> " + str);

		// Stack is no more empty.
		DebugStackEmpty = false;
	}
}// pushDebugStr //

//-----------------------------------------------
// flushDebugStack :
// Display 'title' and a warning for each element in the Debug Stack.
//-----------------------------------------------
void flushDebugStack(const std::string &title)
{
	// If debug stack is not empty
	if(!DebugStackEmpty)
	{
		if(IsDebugFile)
		{
			// Log Title.
			string strTmp = toString("  %s\n", title.c_str());
			DebugFile.serialBuffer((uint8*)strTmp.c_str(), (uint)strTmp.size());

			for(uint i=0; i<DebugStack.size(); ++i)
			{
				strTmp = toString("  %s\n", DebugStack[i].c_str());
				DebugFile.serialBuffer((uint8*)strTmp.c_str(), (uint)strTmp.size());
			}

			// Empty line separator
			strTmp = toString("\n");
			DebugFile.serialBuffer((uint8*)strTmp.c_str(), (uint)strTmp.size());
		}
		// No Output File -> nlwarning
		else
		{
			nlwarning("%s", title.c_str());
			for(uint i=0; i<DebugStack.size(); ++i)
				nlwarning("  %s", DebugStack[i].c_str());

			// Empty line separator
			nlwarning("");
		}
	}

	// Clean the stack (infos could remain in the stack so clean here).
	DebugStack.clear();
	// Stack is empty now.
	DebugStackEmpty = true;
}// flushDebugStack //

//-----------------------------------------------
// setDebugOutput :
// Set an output file to log debugs.
//-----------------------------------------------
void setDebugOutput(const std::string &filename)
{
	// Remove output
	if(filename.empty())
	{
		DebugFile.close();
		IsDebugFile = false;
		return;
	}

	// Open The Item Association File
	if(!DebugFile.open(filename, false, true))
	{
		nlwarning("setDebugOutput: Cannot Open the '%s'.", filename.c_str());
		IsDebugFile = false;
	}
	else
		IsDebugFile = true;
}// setDebugOutput //



//-----------------------------------------------
// initDebugMemory :
// ...
//-----------------------------------------------
void initDebugMemory()
{
#if defined(NL_OS_WINDOWS)
	GlobalMemoryStatus(&MemoryStatus);

/*
	nlwarning("The MemoryStatus structure is %ld bytes long.", stat.dwLength);
	nlwarning("It should be %d.", sizeof (stat));
	nlwarning("%ld percent of memory is in use.", stat.dwMemoryLoad);
	nlwarning("There are %*ld total %sbytes of physical memory.", WIDTH, stat.dwTotalPhys/DIV, divisor);
	nlwarning("There are %*ld free %sbytes of physical memory.", WIDTH, stat.dwAvailPhys/DIV, divisor);
	nlwarning("There are %*ld total %sbytes of paging file.", WIDTH, stat.dwTotalPageFile/DIV, divisor);
	nlwarning("There are %*ld free %sbytes of paging file.", WIDTH, stat.dwAvailPageFile/DIV, divisor);
	nlwarning("There are %*lx total %sbytes of virtual memory.", WIDTH, stat.dwTotalVirtual/DIV, divisor);
	nlwarning("There are %*lx free %sbytes of virtual memory.\n", WIDTH, stat.dwAvailVirtual/DIV, divisor);
*/
#endif
}

//-----------------------------------------------
// memoryUsedSinceLastCall :
// ...
//-----------------------------------------------
double memoryUsedSinceLastCall()
{
#if defined(NL_OS_WINDOWS)
	MEMORYSTATUS stat;
	GlobalMemoryStatus(&stat);

	double mem = (double)MemoryStatus.dwAvailPhys-(double)stat.dwAvailPhys;

	MemoryStatus = stat;

	return mem;

/*
	nlwarning("%ld percent of memory is in use.", stat.dwMemoryLoad);
	nlwarning("There are %*ld free %sbytes of physical memory.", WIDTH, stat.dwAvailPhys/DIV, divisor);
	nlwarning("There are %*ld free %sbytes of paging file.", WIDTH, stat.dwAvailPageFile/DIV, divisor);
	nlwarning("There are %*lx free %sbytes of virtual memory.\n", WIDTH, stat.dwAvailVirtual/DIV, divisor);
*/
#endif
	return std::numeric_limits<double>::quiet_NaN();
}


/*
 * Constructor
 */
CDebugClient::CDebugClient()
{
}

/*
#define INFO if (info_is_active(ANIM_INFO)) nlinfo
#define INFO (info_is_active(ANIM_INFO)?nlinfo:)


INFO("fjkljf %gf %fhgf", dg, dsf);
*/

// ***************************************************************************

CGraph SpfGraph ("time per frame (100 ms)", 10.0f, 10.0f, 250.0f, 100.0f, CRGBA(0,128,0,255), 0, 100.0f, 3);
CGraph CurrentTaskGraph ("current task", 10.0f, 120.0f, 250.0f, 50.0f, CRGBA(0,0,128,255), 0, 1.0f, 3);
CGraph VRAMDownGraph ("vram download (1Mo)", 10.0f, 180.0f, 250.0f, 50.0f, CRGBA(128,0,0,255), 0, 1.f, 3);
CGraph VRAMUpGraph ("vram upload (1Mo)", 10.0f, 240.0f, 250.0f, 50.0f, CRGBA(128,0,0,255), 0, 1.f, 3);
CGraph OpenedFileGraph ("fopen (10)", 10.0f, 300.0f, 250.0f, 50.0f, CRGBA(128,128,0,255), 0, 10.f, 3);
CGraph ByteReadGraph ("byte read (500 ko)", 10.0f, 360.0f, 250.0f, 100.0f, CRGBA(0,128,128,255), 0, 500, 3);
CGraph ByteReadGraphInstant ("instant byte read (500 ko)", 10.0f, 470.0f, 250.0f, 100.0f, CRGBA(64,0,128,255), 0, 500, 3);
CGraph FileReadGraph ("fread (10)", 10.0f, 580.0f, 250.0f, 50.0f, CRGBA(128,0,128,255), 0, 10, 3);
CGraph LuaMemGraph ("Lua memory (mb)", 290.0f, 10.0f, 250.0f, 100.0f, CRGBA(0,128,64,255), 0, 64.0f, 3);

void displayStreamingDebug ()
{
	// Yoyo: display with getPerformanceTime() for better precision.
	static TTicks oldTick = CTime::getPerformanceTime();
	TTicks newTick = CTime::getPerformanceTime();
	double deltaTime = CTime::ticksToSecond (newTick-oldTick);
	oldTick = newTick;
	static NLMISC::CValueSmoother smooth;
	smooth.addValue((float)deltaTime);
	deltaTime = deltaTime * 5.0f / 6.0f;
	float deltaTimeSmooth = smooth.getSmoothValue () * 5.0f / 6.0f;

	if (!FreezeGraph)
	{
		// Vram delta
		static uint32 lastVRAMUsed = Driver->profileAllocatedTextureMemory();
		uint32 VRAMUsed = Driver->profileAllocatedTextureMemory();
		sint value = (sint)VRAMUsed-(sint)lastVRAMUsed;
		if (value>=0)
		{
			VRAMUpGraph.addOneValue ((float)value/(1024.f*1024.f));
			VRAMDownGraph.addOneValue (0);
		}
		else
		{
			VRAMDownGraph.addOneValue ((float)(-value)/(1024.f*1024.f));
			VRAMUpGraph.addOneValue (0);
		}
		lastVRAMUsed = VRAMUsed;

		// File opened delta
		static uint32 lastFileOpened = CIFile::getNumFileOpen();
		uint32 fileOpened = CIFile::getNumFileOpen();
		OpenedFileGraph.addOneValue((float)(fileOpened-lastFileOpened));
		lastFileOpened=fileOpened;

		// Byte read delta
		static uint32 lastByteRead = CIFile::getReadFromFile();
		uint32 byteRead = CIFile::getReadFromFile();
		ByteReadGraph.addOneValue((float)CIFile::getReadingFromFile()/1024.f);
		ByteReadGraphInstant.addOneValue((float)(byteRead-lastByteRead)/1024.f);
		lastByteRead=byteRead;

		// Byte read delta
		static uint32 lastFileRead = CIFile::getNumFileRead();
		uint32 fileRead = CIFile::getNumFileRead();
		FileReadGraph.addOneValue((float)(fileRead-lastFileRead));
		lastFileRead=fileRead;

		// MS per frame
		SpfGraph.addOneValue (1000.f*(float)deltaTime);

		// lua memory
		LuaMemGraph.addOneValue(CLuaManager::getInstance().getLuaState()->getGCCount() / 1024.f);

		// Count of waitinf instance
		CurrentTaskGraph.addOneValue (CAsyncFileManager::getInstance().isTaskRunning()?1.f:0.f);
	}

	// Add graph value

	if(ShowInfos == 3)
	{
		float lineStep = ClientCfg.DebugLineStep;
		float line;

		// Initialize Pen //
		//----------------//
		// Create a shadow when displaying a text.
		TextContext->setShaded(true);
		// Set the font size.
		TextContext->setFontSize(ClientCfg.DebugFontSize);
		// Set the text color
		TextContext->setColor(ClientCfg.DebugFontColor);

		// TOP LEFT //
		//----------//
		TextContext->setHotSpot(UTextContext::TopLeft);

		// FPS and Ms per frame
		line = 0.99f;
		TextContext->printfAt(0.01f, line, "STREAMING INFORMATION");
		if(deltaTimeSmooth != 0.f)
			TextContext->printfAt(0.8f, line,"%.1f fps", 1.f/deltaTimeSmooth);
		else
			TextContext->printfAt(0.8f, line,"%.1f fps", 0.f);
		TextContext->printfAt(0.9f, line, "%d ms", (uint)(deltaTimeSmooth*1000));


		// Dump the task array
		line = 0.90f;
		TextContext->printfAt(0.3f, line,"Task manager:");
		line -= lineStep;
		static vector<string> names;
		CAsyncFileManager::getInstance().dump(names);
		uint i;
		for (i=0; i<names.size (); i++)
		{
			TextContext->printfAt(0.3f, line, "  %s", names[i].c_str());
			line -= lineStep;
		}

		// Dump the opened file array
		line = 0.90f;
		TextContext->printfAt(0.65f, line,"Files opened:");
		line -= lineStep;
		CIFile::dump(names);
		for (i=0; i<names.size (); i++)
		{
			TextContext->printfAt(0.65f, line, "  %s", names[i].c_str());
			line -= lineStep;
		}

		// No more shadow when displaying a text.
		TextContext->setShaded(false);
	}
}

// ***************************************************************************
/*
	Display short debug information of FartTP / reselectperso events
*/
class CDebugConnectionHistory
{
public:
	enum	TEvent
	{
		ServerHopEvent= 0,
		FarTPEvent,
		ReselectPersoEvent,
		NumDebugConnectionEvent
	};

public:
	CDebugConnectionHistory()
	{
		MaxQueueSize= 8;
		for(uint i=0;i<NumDebugConnectionEvent;i++)
			EventCounters[i]= 0;
	}

	// Add a connection event
	void	debugAddConnectionEvent(TEvent ev)
	{
		nlassert(ev<NumDebugConnectionEvent);
		EventCounters[ev]++;
		EventQueue.push_back(ev);
		if(EventQueue.size()>MaxQueueSize)
			EventQueue.pop_front();
	}

	// display debug information in a string
	void	debugDisplayConnectionEvent(string &str)
	{
		// display counters
		for(uint i=0;i<NumDebugConnectionEvent;i++)
		{
			str+= toString("%s: %d\n", CounterNames[i], EventCounters[i]);
		}
		// display queue of last events:
		str+= "Connection Events: ";
		for(uint i=0;i<EventQueue.size();i++)
		{
			if(i>0)
				str+= ", ";
			// Avoid crash in the crash log: in the improbable case where EventQueue has been crashed, at least don't crash here
			uint	index= EventQueue[i];
			clamp(index,0U,uint(NumDebugConnectionEvent-1));
			str+= EventNames[index];
		}
		str+= "\n";
	}

private:
	static const char		*EventNames[NumDebugConnectionEvent];
	static const char		*CounterNames[NumDebugConnectionEvent];

	uint					EventCounters[NumDebugConnectionEvent];
	deque<TEvent>			EventQueue;
	uint					MaxQueueSize;

};
static CDebugConnectionHistory		DebugConnectionHistory;
const char		*CDebugConnectionHistory::EventNames[CDebugConnectionHistory::NumDebugConnectionEvent]=
	{"HOP", "FTP", "RSL"};
const char		*CDebugConnectionHistory::CounterNames[CDebugConnectionHistory::NumDebugConnectionEvent]=
	{"NumServerHOP", "NumFarTP", "NumReselectPerso"};


// extern methods
void	crashLogAddServerHopEvent()
{
	DebugConnectionHistory.debugAddConnectionEvent(CDebugConnectionHistory::ServerHopEvent);
}
void	crashLogAddFarTpEvent()
{
	DebugConnectionHistory.debugAddConnectionEvent(CDebugConnectionHistory::FarTPEvent);
}
void	crashLogAddReselectPersoEvent()
{
	DebugConnectionHistory.debugAddConnectionEvent(CDebugConnectionHistory::ReselectPersoEvent);
}


// ***************************************************************************
string getDebugInformation()
{
	string str;

	str += toString("UserId: %u\n", NetMngr.getUserId());
	str += toString("HomeId: %u\n", CharacterHomeSessionId.asInt());
	extern TSessionId HighestMainlandSessionId;
	str += toString("ShardId: %u\n", HighestMainlandSessionId.asInt());
	extern bool IsInRingSession;
	if (IsInRingSession)
	{
		if (getEditor().isInitialized())
			str += toString("SessionId: %u\n", getEditor().getDMC().getEditionModule().getCurrentAdventureId().asInt());
		extern R2::TUserRole UserRoleInSession;
		str += toString("Role: %s\n", UserRoleInSession.toString().c_str());
	}
	else
	{
		str += toString("On a Mainland Shard\n");
	}
	CConfigFile::CVar	*varPtr= ClientCfg.ConfigFile.getVarPtr("Application");
	if(varPtr)
		str += toString("Application: %s\n", varPtr->asString(0).c_str());
	else
		str += toString("Application: NotFound\n");

	if(UserEntity)
	{
		str += toString("Player Name: '%s'\n", UserEntity->getEntityName().toString().c_str());
		str += toString("UserPosition: %.2f %.2f %.2f\n", UserEntity->pos().x, UserEntity->pos().y, UserEntity->pos().z);
	}
	else
	{
		str += "No user entity information\n";
	}

	str += toString("ViewPosition: %.2f %.2f %.2f\n", View.viewPos().x, View.viewPos().y, View.viewPos().z);
	uint64 timeInGame = ingameTime1 ();
	str += toString("Time in game: %dh %dmin %dsec\n", (uint)(timeInGame/(60*60*1000)), (uint)(timeInGame/(60*1000))%60, (uint)(timeInGame/1000)%60);
	str += toString("LocalTime: %s\n", NLMISC::IDisplayer::dateToHumanString(time(NULL)));
	str += toString("ServerTick: %u\n", NetMngr.getCurrentServerTick());
	str += toString("ConnectState: %s\n", NetMngr.getConnectionStateCStr());
	str += toString("LocalAddress: %s\n", NetMngr.getAddress().asString().c_str());
	str += toString("Language: %s\n", CI18N::getCurrentLanguageName().toString().c_str());
	str += toString("ClientVersion: "RYZOM_VERSION"\n");
	if (ClientCfg.R2Mode)
	{
		str += toString("PatchVersion: %s\n", R2ServerVersion.c_str());
	}
	else if ((ShardSelected >= 0) && (ShardSelected < (sint32)Shards.size()))
	{
		str += toString("PatchVersion: %s\n", Shards[ShardSelected].Version.c_str());
	}
	str += string("Client is ") + string((ClientCfg.Local?"off":"on")) + string("line\n");
	// FarTP/ReselectPerso
	DebugConnectionHistory.debugDisplayConnectionEvent(str);

	return str;
}

void resetIngameTime ()
{
	IngameEnterTime = T1;
}

uint64 ingameTime0 ()
{
	return T0 - IngameEnterTime;
}

uint64 ingameTime1 ()
{
	return T1 - IngameEnterTime;
}

// ***************************************************************************

void displayNetDebug ()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	float lineStep = ClientCfg.DebugLineStep;
	float line;

	// Initialize Pen //
	//----------------//
	// Create a shadow when displaying a text.
	TextContext->setShaded(true);
	// Set the font size.
	TextContext->setFontSize(ClientCfg.DebugFontSize);
	// Set the text color
	TextContext->setColor(ClientCfg.DebugFontColor);


	// BOTTOM RIGHT //
	//--------------//
	TextContext->setHotSpot(UTextContext::BottomRight);
	line = 0.f;
	// Database Synchronisation counter
	// Local Counter
	uint	val= pIM->getLocalSyncActionCounter() ;
	val&= pIM->getLocalSyncActionCounterMask();
	TextContext->printfAt(1.f, line, "Local Counter: %d", val);
	line += lineStep;
	// Inventory Counter
	val= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:COUNTER")->getValue32();
	val&= pIM->getLocalSyncActionCounterMask();
	TextContext->printfAt(1.f, line, "INVENTORY:COUNTER: %d", val);
	line += lineStep;
	// Exchange Counter
	val= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:EXCHANGE:COUNTER")->getValue32();
	val&= pIM->getLocalSyncActionCounterMask();
	TextContext->printfAt(1.f, line, "EXCHANGE:COUNTER: %d", val);
	line += lineStep;
	// Programme Counter
	val= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TARGET:CONTEXT_MENU:COUNTER")->getValue32();
	val&= pIM->getLocalSyncActionCounterMask();
	TextContext->printfAt(1.f, line, "TARGET:CONTEXT_MENU:COUNTER: %d", val);
	line += lineStep;
	// User Counter
	val= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:COUNTER")->getValue32();
	val&= pIM->getLocalSyncActionCounterMask();
	TextContext->printfAt(1.f, line, "USER:COUNTER: %d", val);
	line += lineStep;
	line += lineStep;

	// SPhrase Execution Synchronisation Counter
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	// Next action
	uint	srvVal= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_NEXT)->getValue32();
	uint	locVal= pPM->getPhraseNextExecuteCounter() ;
	srvVal&= PHRASE_EXECUTE_COUNTER_MASK;
	locVal&= PHRASE_EXECUTE_COUNTER_MASK;
	TextContext->printfAt(1.f, line, "NextAction  (loc/srv): %d/%d", locVal, srvVal);
	line += lineStep;
	// CycleAction
	srvVal= NLGUI::CDBManager::getInstance()->getDbProp(PHRASE_DB_COUNTER_CYCLE)->getValue32();
	locVal= pPM->getPhraseCycleExecuteCounter();
	srvVal&= PHRASE_EXECUTE_COUNTER_MASK;
	locVal&= PHRASE_EXECUTE_COUNTER_MASK;
	TextContext->printfAt(1.f, line, "CycleAction (loc/srv): %d/%d", locVal, srvVal);
	line += lineStep;


	// BOTTOM MIDDLE //
	//--------------//
	TextContext->setHotSpot(UTextContext::BottomLeft);
	float	xUser= 0.3f;
	float	xWatched= 0.5f;
	// Display information about the debug entity slot.
	if(WatchedEntitySlot != CLFECOMMON::INVALID_SLOT)
	{
		line = 0.f;
		TextContext->printfAt(xWatched, line, "Watched");
		line += lineStep;
		// Get a pointer on the target.
		CEntityCL *watchedEntity = EntitiesMngr.entity(WatchedEntitySlot);
		if(watchedEntity)
			watchedEntity->displayDebugPropertyStages(xWatched, line, lineStep);
	}
	// Display information about the user
	if(UserEntity)
	{
		line = 0.f;
		TextContext->printfAt(xUser, line, "User");
		line += lineStep;
		UserEntity->displayDebugPropertyStages(xUser, line, lineStep);
	}

}

// ***************************************************************************
bool verboseVPAdvanceTest(CEntityCL *en, uint32 form)
{
	// TestYoyo: Use this method to test only a part of the entities
	// was used initialy to debug mektoub mounts bugs.

	if(!VerboseVP)
		return false;

	if( NetMngr.getUserId()!=1507 )
		return false;

	// creation test (by form to create)
	if(form!=0)
	{
		CSheetId	sheetId(form);
		CSheetId	playerSheetId("matis.race_stats");
		CSheetId	mektoubSheetId("chilb2.creature");
		return (sheetId==playerSheetId || sheetId==mektoubSheetId);
	}
	// update vp or remove test
	else
	{
		CCharacterCL	*e= dynamic_cast<CCharacterCL*>(en);
		if(!e)
			return false;
		if(	e->isPlayer() || e->getSheet()->Id== CSheetId("chilb2.creature") )
		{
			return true;
		}
		return false;
	}
}
