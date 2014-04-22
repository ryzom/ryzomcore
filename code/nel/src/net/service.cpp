// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdnet.h"

//
// Includes
//

#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#	define _WIN32_WINDOWS	0x0410
#	define WINVER			0x0400
#	define NOMINMAX
#	include <windows.h>
#	include <direct.h>
#elif defined NL_OS_UNIX
#	include <unistd.h>
#endif

#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mutex.h"
#include "nel/misc/window_displayer.h"
#include "nel/misc/gtk_displayer.h"
#include "nel/misc/win_displayer.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/report.h"
#include "nel/misc/system_info.h"
#include "nel/misc/timeout_assertion_thread.h"

#include "nel/net/naming_client.h"
#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/net_displayer.h"
#include "nel/net/email.h"
#include "nel/net/varpath.h"
#include "nel/net/admin.h"
#include "nel/net/module_manager.h"
#include "nel/net/transport_class.h"

#include "stdin_monitor_thread.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;

namespace NLNET
{


//
// Constants
//

static const sint Signal[] = {
  SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

static const char *SignalName[]=
{
  "SIGABRT", "SIGFPE", "SIGILL", "SIGINT", "SIGSEGV", "SIGTERM"
};

static const char* NegFiltersNames[] =
{
   "NegFiltersDebug",
   "NegFiltersInfo",
   "NegFiltersWarning",
   "NegFiltersAssert",
   "NegFiltersError",
   0
};


//
// Variables
//

TUnifiedCallbackItem EmptyCallbackArray[1] = { { "", NULL } };

// class static member
IService	*IService::_Instance = NULL;

static sint ExitSignalAsked = 0;

// services stat
CVariable<sint32> UserSpeedLoop ("nel", "UserSpeedLoop", "duration of the last user loop (in ms)", 10, false);
CVariable<sint32> NetSpeedLoop ("nel", "NetSpeedLoop", "duration of the last network loop (in ms)", 10, false);
/// The time passed in callback during the loop
CVariable<uint32> L5CallbackTime("nel", "L5CallbackTime", "Time passed in the L5 callback function in the last loop (in ms)", 0, 100 );
/// The number of L5 callback treated
CVariable<uint32> L5CallbackCount("nel", "L5CallbackCount", "The number of layer 5 callback received in the last loop", 0, 100 );

extern uint32 TotalCallbackCalled;
extern uint32 TimeInCallback;
uint32 LastTotalCallbackCalled = 0;
uint32 LastTimeInCallback = 0;



// this is the thread that initialized the signal redirection
// we'll ignore other thread signals
static size_t SignalisedThread;

static CFileDisplayer fd;
static CNetDisplayer commandDisplayer(false);
//static CLog commandLog;

static string CompilationDate;
static uint32 LaunchingDate;

static uint32 NbUserUpdate = 0;

string CompilationMode = nlMode;

//static bool Bench = false;

CVariable<bool> Bench ("nel", "Bench", "1 if benching 0 if not", 0, true);

// This produce an assertion in the thread if the update loop is too slow
static CTimeoutAssertionThread	MyTAT;
static void						UpdateAssertionThreadTimeoutCB(IVariable &var) { uint32 timeOut; fromString(var.toString(), timeOut); MyTAT.timeout(timeOut); }
static CVariable<uint32>		UpdateAssertionThreadTimeout("nel", "UpdateAssertionThreadTimeout", "in millisecond, timeout before thread assertion", 0, 0, true, UpdateAssertionThreadTimeoutCB);

// Flag to enable/disable the flushing of the sending queues when the service is shut down
// Default: false (matches the former behavior)
// Set it to true in services that need to send data on exit (for instance in their release() method)
CVariable<bool>					FlushSendingQueuesOnExit("nel", "FlushSendingQueuesOnExit",
	"Flag to enable/disable the flushing of the sending queues when the service is shut down", false, 0, true );

// If FlushSendingQueuesOnExit is on, only the sending queues to these specified services will be flushed
// Format: service short names separated by ':'
// Default: "" (all will be flushed if FlushSendingQueuesOnExit is on, none if it is off)
CVariable<string>				NamesOfOnlyServiceToFlushSending("nel", "NamesOfOnlyServiceToFlushSending",
	"If FlushSendingQueuesOnExit is on, only the sending queues to these specified services will be flushed (ex: \"WS:LS\"; all will be flushed if empty string)", "", 0, true );


//
// Signals managing
//

// This function is called when a signal comes
static void sigHandler(int Sig)
{
	// redirect the signal for the next time
	signal(Sig, sigHandler);

	// find the signal
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		if (Sig == Signal[i])
		{
			if (getThreadId () != SignalisedThread)
			{
				nldebug ("SERVICE: Not the main thread (%u, %u) received the signal (%s, %d), ignore it", getThreadId (), SignalisedThread, SignalName[i],Sig);
				return;
			}
			else
			{
				nlinfo ("SERVICE: Signal %s (%d) received", SignalName[i], Sig);
				switch (Sig)
				{
				// Note: SIGKILL (9) and SIGSTOP (19) can't be trapped
				case SIGINT  :
				  if (IService::getInstance()->haveLongArg("nobreak"))
				    {
				      // ignore ctrl-c
				      nlinfo("SERVICE: Ignoring ctrl-c");
				      return;
				    }
				case SIGABRT :
				case SIGILL  :
				case SIGSEGV :
				case SIGTERM :
				// you should not call a function and system function like printf in a SigHandle because
				// signal-handler routines are usually called asynchronously when an interrupt occurs.
				if (ExitSignalAsked == 0)
				{
					nlinfo ("SERVICE: Receive a signal that said that i must exit");
					ExitSignalAsked = Sig;
					return;
				}
				else
				{
					nlinfo ("SERVICE: Signal already received, launch the brutal exit");
					exit (EXIT_FAILURE);
				}
				break;
				}
			}
		}
	}
	nlwarning ("SERVICE: Unknown signal received (%d)", Sig);
}

// Initialise the signal redirection
static void initSignal()
{
	SignalisedThread = getThreadId ();
#ifdef NL_DEBUG
	// in debug mode, we only trap the SIGINT signal (for ctrl-c handling)
	//signal(Signal[3], sigHandler);
	//nldebug("Signal : %s (%d) trapped", SignalName[3], Signal[3]);
#else
	// in release, redirect all signals
// don't redirect now because too hard to debug...
//	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
//	{
//		signal(Signal[i], sigHandler);
//		nldebug("Signal %s (%d) trapped", SignalName[i], Signal[i]);
//	}
//
	if (IService::getInstance()->haveLongArg("nobreak"))
	{
		signal(Signal[3], sigHandler);
	}
#endif
}

void cbDirectoryChanged (IVariable &var)
{
	IService *instance = IService::getInstance();

	// Convert to full path if required
	// (warning: ConvertSavesFilesDirectoryToFullPath, read from the config file, won't be ready for the initial variable assigments done before the config file has been loaded)
	string vp = var.toString();
	if ((var.getName() != "SaveFilesDirectory") || instance->ConvertSavesFilesDirectoryToFullPath.get())
	{
		vp = CPath::getFullPath(vp);
		var.fromString(vp);
	}
	nlinfo ("SERVICE: '%s' changed to '%s'", var.getName().c_str(), vp.c_str());

	// Update the running directory if needed
	if (var.getName() == "RunningDirectory")
	{
		CPath::setCurrentPath(vp);
	}

	// Call the callback if provided
	if (instance->_DirectoryChangedCBI)
		instance->_DirectoryChangedCBI->onVariableChanged(var);
}


//
// Service built-in callbacks
//

void cbReceiveShardId (CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	uint32 shardId;
	msgin.serial(shardId);

	if (IService::getInstance()->getDontUseNS())
	{
		// we don't use NS, so shard ID message don't concern us
		return;
	}

	if (serviceName != "WS")
	{
		nlwarning("SERVICE: received unauthorized R_SH_ID callback from service %s-%uh asking to set ShardId to %d", serviceName.c_str(), serviceId.get(), shardId);
		return;
	}

	nlinfo("SERVICE: ShardId is %u", shardId);
	IService::getInstance()->setShardId( shardId );
}

std::string IService::getServiceStatusString() const
{
	static string emptyString;

	return emptyString;
}


//
void IService::anticipateShardId( uint32 shardId )
{
	if ( ! ((_ShardId == DEFAULT_SHARD_ID) || (shardId == _ShardId)) )
		nlerror( "IService::anticipateShardId() overwrites %u with %u", _ShardId, shardId );
	_ShardId = shardId;
}

//
void IService::setShardId( uint32 shardId )
{
	if ( ! ((_ShardId == DEFAULT_SHARD_ID) || (shardId == _ShardId)) )
		nlwarning( "SERVICE: The shardId from the WS (%u) is different from the anticipated shardId (%u)", shardId, _ShardId );
	_ShardId = shardId;
}

TUnifiedCallbackItem builtinServiceCallbacks [] =
{
	{ "R_SH_ID", cbReceiveShardId },
};



//
// Class implementation
//

// Ctor
IService::IService() :
	WindowDisplayer(0),
	WriteFilesDirectory("nel", "WriteFilesDirectory", "directory where to save generic shard information (packed_sheets for example)", ".", 0, true, cbDirectoryChanged),
	SaveFilesDirectory("nel", "SaveFilesDirectory", "directory where to save specific shard information (shard time for example)", ".", 0, true, cbDirectoryChanged),
	ConvertSavesFilesDirectoryToFullPath("nel", "ConvertSaveFilesDirectoryToFullPath", "If true (default), the provided SaveFilesDirectory will be converted to a full path (ex: saves -> /home/dir/saves)", true, 0, true ),
	ListeningPort("nel", "ListeningPort", "listening port for this service", 0, 0, true),
	_RecordingState(CCallbackNetBase::Off),
	_UpdateTimeout(100),
	_SId(0),
	_ExitStatus(0),
	_Initialized(false),
	ConfigDirectory("nel", "ConfigDirectory", "directory where config files are", ".", 0, true, cbDirectoryChanged),
	LogDirectory("nel", "LogDirectory", "directory where the service is logging", ".", 0, true, cbDirectoryChanged),
	RunningDirectory("nel", "RunningDirectory", "directory where the service is running on", ".", 0, true, cbDirectoryChanged),
	Version("nel", "Version", "Version of the shard", ""),
	_CallbackArray (0),
	_CallbackArraySize (0),
	_DontUseNS(false),
	_DontUseAES(false),
	_ResetMeasures(false),
	_ShardId(0),
	_ClosureClearanceStatus(CCMustRequestClearance),
	_RequestClosureClearanceCallback(NULL),
	_DirectoryChangedCBI(NULL)
{
	// Singleton
	_Instance = this;

	// register in the safe singleton registry
	INelContext::getInstance().setSingletonPointer("IService", this);
}

IService::~IService()
{
	// unregister the singleton
	INelContext::getInstance().releaseSingletonPointer("IService", this);
}


bool IService::haveArg (char argName) const
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if (_Args[i][1] == argName)
			{
				return true;
			}
		}
	}
	return false;
}


string IService::getArg (char argName) const
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if (_Args[i][1] == argName)
			{
				/* Remove the first and last '"' :
				-c"C:\Documents and Settings\toto.tmp"
				will return :
				C:\Documents and Settings\toto.tmp
				*/
				uint begin = 2;
				if (_Args[i].size() < 3)
					return "";
					//throw Exception ("Parameter '-%c' is malformed, missing content", argName);

				if (_Args[i][begin] == '"')
					begin++;

				// End
				uint size = (uint)_Args[i].size();
				if (size && _Args[i][size-1] == '"')
					size--;
				size = (uint)(std::max((int)0, (int)size-(int)begin));
				return _Args[i].substr(begin, size);
			}
		}
	}
	throw Exception ("Parameter '-%c' is not found in command line", argName);
}


bool IService::haveLongArg (const char* argName) const
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].left(2)=="--" && _Args[i].leftCrop(2).splitTo('=')==argName)
		{
			return true;
		}
	}
	return false;
}


string IService::getLongArg (const char* argName) const
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].left(2)=="--" && _Args[i].leftCrop(2).splitTo('=')==argName)
		{
			NLMISC::CSString val= _Args[i].splitFrom('=');
			if (!val.empty())
			{
				return val.unquoteIfQuoted();
			}
			if (i+1<_Args.size() && _Args[i+1].c_str()[0]!='-')
			{
				return _Args[i+1].unquoteIfQuoted();
			}

			return std::string();
		}
	}
	return std::string();
}


void IService::setArgs (const char *args)
{
	_Args.push_back ("<ProgramName>");

	string sargs (args);
	string::size_type pos1 = 0, pos2 = 0;

	do
	{
		// Look for the first non space character
		pos1 = sargs.find_first_not_of (" ", pos2);
		if (pos1 == string::npos) break;

		// Look for the first space or "
		pos2 = sargs.find_first_of (" \"", pos1);
		if (pos2 != string::npos)
		{
			// " ?
			if (sargs[pos2] == '"')
			{
				// Look for the final \"
				pos2 = sargs.find_first_of ("\"", pos2+1);
				if (pos2 != string::npos)
				{
					// Look for the first space
					pos2 = sargs.find_first_of (" ", pos2+1);
				}
			}
		}

		// Compute the size of the string to extract
		string::difference_type length = (pos2 != string::npos) ? pos2-pos1 : string::npos;

		string tmp = sargs.substr (pos1, length);
		_Args.push_back (tmp);
	}
	while (pos2 != string::npos);
}

void IService::setArgs (int argc, const char **argv)
{
	for (sint i = 0; i < argc; i++)
	{
		_Args.push_back (argv[i]);
	}
}


void cbLogFilter (CConfigFile::CVar &var)
{
	CLog *log = NULL;
	if (var.Name == "NegFiltersDebug")
	{
		log = DebugLog;
	}
	else if (var.Name == "NegFiltersInfo")
	{
		log = InfoLog;
	}
	else if (var.Name == "NegFiltersWarning")
	{
		log = WarningLog;
	}
	else if (var.Name == "NegFiltersAssert")
	{
		log = AssertLog;
	}
	else if (var.Name == "NegFiltersError")
	{
		log = ErrorLog;
	}
	else
	{
		nlstop;
	}

	nlinfo ("SERVICE: Updating %s from config file", var.Name.c_str());

	// remove all old filters from config file
	CConfigFile::CVar &oldvar = IService::getInstance()->ConfigFile.getVar (var.Name);
	for (uint j = 0; j < oldvar.size(); j++)
	{
		log->removeFilter (oldvar.asString(j).c_str());
	}

	// add all new filters from config file
	for (uint i = 0; i < var.size(); i++)
	{
		log->addNegativeFilter (var.asString(i).c_str());
	}
}

void cbExecuteCommands (CConfigFile::CVar &var)
{
	for (uint i = 0; i < var.size(); i++)
	{
		ICommand::execute (var.asString(i), IService::getInstance()->CommandLog);
	}
}



//
// The main function of the service
//

sint IService::main (const char *serviceShortName, const char *serviceLongName, uint16 servicePort, const char *configDir, const char *logDir, const char *compilationDate)
{
	bool userInitCalled = false;
	CConfigFile::CVar *var = NULL;

	IThread *timeoutThread = NULL;

	// a short name service can't be a number
	uint tmp;
	nlassert (!fromString(serviceShortName, tmp));

	try
	{
		createDebug();
		// init the module manager
		IModuleManager::getInstance();
		//
		// Init parameters
		//

		// at the very beginning, eventually wrote a file with the pid
		if (haveLongArg("writepid"))
		{
			// use legacy C primitives
			FILE *fp = fopen("pid.state", "wt");
			if (fp)
			{
				fprintf(fp, "%u", getpid());
				fclose(fp);
			}
		}

		_ShortName = serviceShortName;
		CLog::setProcessName (_ShortName);

		// get the path where to run the service if any in the command line
		if (haveArg('A'))
			RunningDirectory = CPath::standardizePath(getArg('A'));

		ConfigDirectory = CPath::standardizePath(configDir);
		LogDirectory = CPath::standardizePath(logDir);
		_LongName = serviceLongName;

		CompilationDate = compilationDate;

		LaunchingDate = CTime::getSecondsSince1970();

		ListeningPort = servicePort;

		setReportEmailFunction ((void*)sendEmail);
		// setDefaultEmailParams ("gw.nevrax.com", "", "cado@nevrax.com");


		//
		// Load the config file
		//

		// get the config file dir if any in the command line
		if (haveArg('C'))
			ConfigDirectory = CPath::standardizePath(getArg('C'));

		string cfn = ConfigDirectory.c_str() + _LongName + ".cfg";
		if (!CFile::fileExists(ConfigDirectory.c_str() + _LongName + ".cfg"))
		{
			// check if the default exists
			if (!CFile::fileExists(ConfigDirectory.c_str() + _LongName + "_default.cfg"))
			{
				nlerror ("SERVICE: Neither the config file '%s' nor the default one can be found, can't launch the service", cfn.c_str());
			}
			else
			{
				// create the basic .cfg that link the default one
				FILE *fp = fopen (cfn.c_str(), "w");
				if (fp == NULL)
				{
					nlerror ("SERVICE: Can't create config file '%s'", cfn.c_str());
				}
				fprintf(fp, "// link the default config file for %s\n", _LongName.c_str());
				fprintf(fp, "RootConfigFilename = \"%s_default.cfg\";\n", _LongName.c_str());
				fclose (fp);
			}
		}

		ConfigFile.load (cfn);

		// setup variable with config file variable
		IVariable::init (ConfigFile);

		if (ConfigFile.exists("DefaultEmailSMTP") && ConfigFile.exists("DefaultEmailTo"))
			NLNET::setDefaultEmailParams(
				ConfigFile.getVar("DefaultEmailSMTP").asString(),
				ConfigFile.exists("DefaultEmailFrom")
				? ConfigFile.getVar("DefaultEmailFrom").asString()
				: "service@opennel.org",
				ConfigFile.getVar("DefaultEmailTo").asString());

		//
		// Set the shard Id
		//

		if ((var = ConfigFile.getVarPtr("NoWSShardId")) != NULL)
		{
			_ShardId = var->asInt();
		}
		else
		{
			// something high enough as default
			_ShardId = DEFAULT_SHARD_ID;
		}

		if (haveArg('Z'))
		{
			string s = IService::getInstance()->getArg('Z');
			if (s == "u")
			{
				// do not release the module manager
			}
			else
			{
				// release the module manager
				IModuleManager::getInstance().releaseInstance();
			}

			return 0;
		}

		// we have to call this again because the config file can changed this variable but the cmd line is more prioritary
		if (haveArg('A'))
			RunningDirectory = CPath::standardizePath(getArg('A'));


		//
		// Init debug/log stuffs (must be first things otherwise we can't log if errors)
		//

		// get the log dir if any in the command line
		if (haveArg('L'))
			LogDirectory = CPath::standardizePath(getArg('L'));

		changeLogDirectory (LogDirectory);

		bool noLog= (ConfigFile.exists ("DontLog")) && (ConfigFile.getVar("DontLog").asInt() == 1);
		noLog|=haveLongArg("nolog");
		if (!noLog)
		{
			// we create the log with service name filename ("test_service_ALIAS.log" for example)
			string logname = LogDirectory.toString() + _LongName;
			if (haveArg('N'))
				logname += "_" + toLower(getArg('N'));
			logname += ".log";
			fd.setParam (logname, false);

			DebugLog->addDisplayer (&fd);
			InfoLog->addDisplayer (&fd);
			WarningLog->addDisplayer (&fd);
			AssertLog->addDisplayer (&fd);
			ErrorLog->addDisplayer (&fd);
			CommandLog.addDisplayer (&fd, true);
		}

		bool dontUseStdIn= (ConfigFile.exists ("DontUseStdIn")) && (ConfigFile.getVar("DontUseStdIn").asInt() == 1);
		if (!dontUseStdIn)
		{
			IStdinMonitorSingleton::getInstance()->init();
		}

		//
		// Init the hierarchical timer
		//

		CHTimer::startBench(false, true);
		CHTimer::endBench();

		//
		// Set the assert mode
		//

		if (ConfigFile.exists ("Assert"))
			setAssert (ConfigFile.getVar("Assert").asInt() == 1);

		//
		// Set the update timeout if found in the cfg
		//
		if ((var = ConfigFile.getVarPtr("UpdateTimeout")) != NULL)
		{
			_UpdateTimeout = var->asInt();
		}

		//
		// Set the negative filter from the config file
		//

		for(const char **name = NegFiltersNames; *name; name++)
		{
			if ((var = ConfigFile.getVarPtr (*name)) != NULL)
			{
				ConfigFile.setCallback (*name, cbLogFilter);
				cbLogFilter(*var);
			}
		}

		ConfigFile.setCallback ("Commands", cbExecuteCommands);
		if ((var = ConfigFile.getVarPtr ("Commands")) != NULL)
		{
			cbExecuteCommands(*var);
		}

		//
		// Command line start
		//
		commandStart ();

		//
		// Create the window if needed
		//

		if ((var = ConfigFile.getVarPtr ("WindowStyle")) != NULL)
		{
			string disp = var->asString ();
#ifdef NL_USE_GTK
			if (disp == "GTK")
			{
				WindowDisplayer = new CGtkDisplayer ("DEFAULT_WD");
			}
#endif // NL_USE_GTK

#ifdef NL_OS_WINDOWS
			if (disp == "WIN")
			{
				WindowDisplayer = new CWinDisplayer ("DEFAULT_WD");
			}
#endif // NL_OS_WINDOWS

			if (WindowDisplayer == NULL && disp != "NONE")
			{
				nlinfo ("SERVICE: Unknown value for the WindowStyle (should be GTK, WIN or NONE), use no window displayer");
			}
		}

		vector <pair<string,uint> > displayedVariables;
		//uint speedNetLabel, speedUsrLabel, rcvLabel, sndLabel, rcvQLabel, sndQLabel, scrollLabel;
		if (WindowDisplayer != NULL)
		{
			//
			// Init window param if necessary
			//

			sint x=-1, y=-1, w=-1, h=-1, fs=10, history=-1;
			bool iconified = false, ww = false;
			string fn;

			if ((var = ConfigFile.getVarPtr("XWinParam")) != NULL) x = var->asInt();
			if ((var = ConfigFile.getVarPtr("YWinParam")) != NULL) y = var->asInt();
			if ((var = ConfigFile.getVarPtr("WWinParam")) != NULL) w = var->asInt();
			if ((var = ConfigFile.getVarPtr("HWinParam")) != NULL) h = var->asInt();
			if ((var = ConfigFile.getVarPtr("HistoryWinParam")) != NULL) history = var->asInt();
			if ((var = ConfigFile.getVarPtr("IWinParam")) != NULL) iconified = var->asInt() == 1;
			if ((var = ConfigFile.getVarPtr("FontSize")) != NULL) fs = var->asInt();
			if ((var = ConfigFile.getVarPtr("FontName")) != NULL) fn = var->asString();
			if ((var = ConfigFile.getVarPtr("WordWrap")) != NULL) ww = var->asInt() == 1;

			if (haveArg('I')) iconified = true;

			WindowDisplayer->create (string("*INIT* ") + _ShortName + " " + _LongName, iconified, x, y, w, h, history, fs, fn, ww, &CommandLog);

			DebugLog->addDisplayer (WindowDisplayer);
			InfoLog->addDisplayer (WindowDisplayer);
			WarningLog->addDisplayer (WindowDisplayer);
			ErrorLog->addDisplayer (WindowDisplayer);
			AssertLog->addDisplayer (WindowDisplayer);
			CommandLog.addDisplayer(WindowDisplayer, true);

			// adding default displayed variables
			displayedVariables.push_back(make_pair(string("NetLop|NetSpeedLoop"), WindowDisplayer->createLabel ("NetLop")));
			displayedVariables.push_back(make_pair(string("UsrLop|UserSpeedLoop"), WindowDisplayer->createLabel ("UsrLop")));
			displayedVariables.push_back(make_pair(string("|Scroller"), WindowDisplayer->createLabel ("NeL Rulez")));

			CConfigFile::CVar *v = ConfigFile.getVarPtr("DisplayedVariables");
			if (v != NULL)
			{
				for (uint i = 0; i < v->size(); i++)
				{
					displayedVariables.push_back(make_pair(v->asString(i), WindowDisplayer->createLabel (v->asString(i).c_str())));
				}
			}
		}

		nlinfo ("SERVICE: Starting Service '%s' using NeL ("__DATE__" "__TIME__") compiled %s", _ShortName.c_str(), CompilationDate.c_str());
		nlinfo ("SERVICE: On OS: %s", CSystemInfo::getOS().c_str());

		setExitStatus (EXIT_SUCCESS);

		//
		// Redirect signal if needed (in release mode only)
		//

#ifdef NL_OS_WINDOWS
#	ifdef NL_NO_DEBUG
		initSignal();
#	else
		// don't install signal is the application is started in debug mode
		if (IsDebuggerPresent ())
		{
			//nlinfo("Running with the debugger, don't redirect signals");
			initSignal();
		}
		else
		{
			//nlinfo("Running without the debugger, redirect SIGINT signal");
			initSignal();
		}
#	endif
#else // NL_OS_UNIX
		initSignal();
#endif


		//
		// Ignore SIGPIPE (broken pipe) on unix system
		//

#ifdef NL_OS_UNIX
		// Ignore the SIGPIPE signal
		sigset_t SigList;
		bool IgnoredPipe = true;
		if (sigemptyset (&SigList) == -1)
		{
			perror("sigemptyset()");
			IgnoredPipe = false;
		}

		if (sigaddset (&SigList, SIGPIPE) == -1)
		{
			perror("sigaddset()");
			IgnoredPipe = false;
		}

		if (sigprocmask (SIG_BLOCK, &SigList, NULL) == -1)
		{
			perror("sigprocmask()");
			IgnoredPipe = false;
		}
		nldebug ("SERVICE: SIGPIPE %s", IgnoredPipe?"Ignored":"Not Ignored");
#endif // NL_OS_UNIX


		//
		// Initialize the network system
		//

		string localhost;
		try
		{
			// Initialize WSAStartup and network stuffs
			CSock::initNetwork();

			// Get the localhost name
			localhost = CInetAddress::localHost().hostName();
		}
		catch (const NLNET::ESocket &)
		{
			localhost = "<UnknownHost>";
		}

		// Set the localhost name and service name to the logger
		CLog::setProcessName (localhost+"/"+_ShortName);
		nlinfo ("SERVICE: Host: %s", localhost.c_str());

		//
		// Initialize server parameters
		//

		// set the listen port if there are a port arg in the command line
		if (haveArg('P'))
		{
			NLMISC::fromString(getArg('P'), ListeningPort);
		}

		// set the aes aliasname if present in cfg file
		CConfigFile::CVar *varAliasName= ConfigFile.getVarPtr("AESAliasName");
		if (varAliasName != NULL)
		{
			_AliasName = varAliasName->asString();
			nlinfo("SERVICE: Setting alias name to: '%s'",_AliasName.c_str());
		}

		// set the aes aliasname if is present in the command line
		if (haveArg('N'))
		{
			_AliasName = getArg('N');
			nlinfo("SERVICE: Setting alias name to: '%s'",_AliasName.c_str());
		}

		// Load the recording state from the config file
		if ((var = ConfigFile.getVarPtr ("Rec")) != NULL)
		{
			string srecstate = toUpper(var->asString());
			if ( srecstate == "RECORD" )
			{
				_RecordingState = CCallbackNetBase::Record;
				nlinfo( "SERVICE: Service recording messages" );
			}
			else if ( srecstate == "REPLAY" )
			{
				_RecordingState = CCallbackNetBase::Replay;
				nlinfo( "SERVICE: Service replaying messages" );
			}
			else
			{
				_RecordingState = CCallbackNetBase::Off;
			}
		}
		else
		{
			// Not found
			_RecordingState = CCallbackNetBase::Off;
		}

		// Load the default stream format
		if ((var = ConfigFile.getVarPtr ("StringMsgFormat")) != NULL)
		{
			CMessage::setDefaultStringMode( var->asInt() == 1 );
		}
		else
		{
			// Not found => binary
			CMessage::setDefaultStringMode( false );
		}


		///
		/// Layer5 Startup
		///

		// get the sid
		if ((var = ConfigFile.getVarPtr ("SId")) != NULL)
		{
			sint32 sid = var->asInt();
			if (sid<=0 || sid>255)
			{
				nlwarning("SERVICE: Bad SId value in the config file, %d is not in [0;255] range", sid);
				_SId.set(0);
			}
			else
			{
				_SId.set(static_cast<uint16>(sid));
			}
		}
		else
		{
			// ok, SId not found, use dynamic sid
			_SId.set(0);
		}


		// look if we don't want to use NS
		if ((var = ConfigFile.getVarPtr ("DontUseNS")) != NULL)
		{
			// if we set the value in the config file, get it
			_DontUseNS = (var->asInt() == 1);
		}
		else
		{
			// if not, we use ns only if service is not ns, ls, aes, as
			_DontUseNS = false;
		}
		if (haveLongArg("nons"))
		{
			// command line override
			_DontUseNS = true;
		}

		//
		// Register all network associations (must be before the CUnifiedNetwork::getInstance()->init)
		//

		if ((var = ConfigFile.getVarPtr ("Networks")) != NULL)
		{
			for (uint8 i = 0; i < var->size (); i++)
				CUnifiedNetwork::getInstance()->addNetworkAssociation (var->asString(i), i);
		}

		if ((var = ConfigFile.getVarPtr ("DefaultNetworks")) != NULL)
		{
			for (uint8 i = 0; i < var->size (); i++)
				CUnifiedNetwork::getInstance()->addDefaultNetwork(var->asString(i));
		}

		// normal setup for the common services
		if (!_DontUseNS)
		{
			bool ok = false;
			while (!ok)
			{
				string LSAddr;

				if (haveArg('B'))
				{
					// if the naming service address is set on the command line, get it (overwrite the cfg)
					LSAddr = getArg('B');
				}
				else
				{
					// else read the naming service address from the config file
					LSAddr = ConfigFile.getVar ("NSHost").asString();
				}

				// if there's no port to the NS, use the default one 50000
				if (LSAddr.find(":") == string::npos)
					LSAddr += ":50000";

				CInetAddress loc(LSAddr);
				try
				{
					// todo: check if app not closed by user, or you get stuck here

					if ( CUnifiedNetwork::getInstance()->init (&loc, _RecordingState, _ShortName, ListeningPort, _SId) )
					{
						ok = true;
					}
					else
					{
						nlinfo( "SERVICE: Exiting..." );
						beep( 880, 400 );
						beep( 440, 400 );
						beep( 220, 400 );

						// remove the stdin monitor thread
						IStdinMonitorSingleton::getInstance()->release(); // does nothing if not initialized

						// release the module manager
						IModuleManager::getInstance().releaseInstance();

						return 10;
					}
				}
				catch (const ESocketConnectionFailed &)
				{
					nlinfo ("SERVICE: Could not connect to the Naming Service (%s). Retrying in a few seconds...", loc.asString().c_str());
					nlSleep (5000);
				}
			}
		}
		else
		{
			CUnifiedNetwork::getInstance()->init(NULL, _RecordingState, _ShortName, ListeningPort, _SId);
		}

		// get the hostname for later use
		_HostName = CInetAddress::localHost().hostName();

		// At this point, the _SId must be ok if we use the naming service.
		// If it's 0, it means that we don't use NS and we left the other side server to find a sid for your connection

		if(!_DontUseNS)
		{
			nlassert (_SId.get() != 0);
		}

		//
		// Connect to the local AES and send identification
		//

		// look if we don't want to use NS
		if ((var = ConfigFile.getVarPtr ("DontUseAES")) != NULL)
		{
			// if we set the value in the config file, get it
			_DontUseAES = var->asInt() == 1;
		}
		else
		{
			// if not, we use aes only if service is not aes or as
			_DontUseAES = false;
		}

		initAdmin (_DontUseAES);

		while (NLNET::CUnifiedNetwork::getInstance()->tryFlushAllQueues() != 0)
		{
			nlSleep(10);
		}

		//
		// Add callback array
		//

		// add inner service callback array
		NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(builtinServiceCallbacks, sizeof(builtinServiceCallbacks)/sizeof(builtinServiceCallbacks[0]));

		// add callback set in the NLNET_SERVICE_MAIN macro
		NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(_CallbackArray, _CallbackArraySize);

		//
		// Now we have the service id, we can set the entites id generator
		//

		NLMISC::CEntityId::setServiceId(TServiceId8(_SId).get());

		// Set the localhost name and service name and the sid
		CLog::setProcessName (localhost+"/"+_ShortName+"-"+toString(_SId.get()));


		//
		// Add default pathes
		//

		if ((var = ConfigFile.getVarPtr ("IgnoredFiles")) != NULL)
		{
			for (uint i = 0; i < var->size(); i++)
			{
				CPath::addIgnoredDoubleFile (var->asString(i));
			}
		}

		if ((var = ConfigFile.getVarPtr ("Paths")) != NULL)
		{
			for (uint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), true, false);
			}
		}

		if ((var = ConfigFile.getVarPtr ("PathsNoRecurse")) != NULL)
		{
			for (uint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), false, false);
			}
		}

		// if we can, try to setup where to save files
		if (IService::getInstance()->haveArg('W'))
		{
			// use the command line param if set (must be done after the config file has been loaded)
			SaveFilesDirectory = IService::getInstance()->getArg('W');
		}

		CTransportClass::init();

		//
		// Call the user service init
		//

		userInitCalled = true; // the bool must be put *before* the call to init()

		setCurrentStatus("Initializing");
		init ();
		clearCurrentStatus("Initializing");


		//
		// Connects to the present services
		// WARNING: only after the user init() was called because the
		// addService may call up service callbacks.
		//

		CUnifiedNetwork::getInstance()->connect();

		//
		// Say to the AES that the service is ready
		//

		if (!_DontUseAES)
		{
			// send the ready message (service init finished)
			CMessage msgout ("SR");
			CUnifiedNetwork::getInstance()->send("AES", msgout);
		}


		_Initialized = true;

		nlinfo ("SERVICE: Service initialized, executing StartCommands");

		//
		// Call the user command from the config file if any
		//
		string cmdRoot("StartCommands");
		vector<string>	posts;

		// add an empty string (for the common part of start commands)
		posts.push_back(string());

		if (IService::getInstance()->haveArg('S'))
		{
			string s = IService::getInstance()->getArg('S');
			posts.push_back(s);
		}

		CConfigFile::CVar *var;
		for (uint i=0; i<posts.size(); ++i)
		{
			string varName = cmdRoot + posts[i];
			if ((var = IService::getInstance()->ConfigFile.getVarPtr (varName)) != NULL)
			{
				for (uint i = 0; i < var->size(); i++)
				{
					ICommand::execute (var->asString(i), CommandLog);
				}
			}
		}

		string str;
		CLog logDisplayVars;
		CLightMemDisplayer mdDisplayVars;
		logDisplayVars.addDisplayer (&mdDisplayVars);

		//
		// Activate the timeout assertion thread
		//

		timeoutThread = IThread::create(&MyTAT, 1024*4);
		timeoutThread->start();

		//
		// Set service ready
		//

		nlinfo ("SERVICE: Service ready");

		if (WindowDisplayer != NULL)
			WindowDisplayer->setTitleBar (_ShortName + " " + _LongName + " " + Version.c_str());

		//
		// Call the user service update each loop and check files and network activity
		//

		TTime	checkCpuProcTime = 0;

		for(;;)
		{
			MyTAT.activate();

			if(Bench) CHTimer::startBench(false, true, false);

			// count the amount of time to manage internal system
			TTime bbefore = CTime::getLocalTime ();

			// every second, check for CPU usage
			if (bbefore - checkCpuProcTime > 1000)
			{
				checkCpuProcTime = bbefore;
				_CPUUsageStats.peekMeasures();
			}

			// call the user update and exit if the user update asks it
			{
				H_AUTO(NLNETServiceUpdate);
				if (!update ())
				{
					CHTimer::endBench();
					break;
				}
			}

			// deal with any input waiting from stdin
			{
				H_AUTO(NLNETStdinMonitorUpdate);
				IStdinMonitorSingleton::getInstance()->update();
			}

			// if the launching mode is 'quit after the first update' we set the exit signal
			if (haveArg('Q'))
			{
				// we use -100 so that the final return code ends as 0 (100+ExitSignalAsked) because
				// otherwise we can't launch services with -Q in a makefile
				ExitSignalAsked = -100;
			}
			NbUserUpdate++;

			// count the amount of time to manage internal system
			TTime before = CTime::getLocalTime ();

			if (WindowDisplayer != NULL)
			{
				// update the window displayer and quit if asked
				if (!WindowDisplayer->update ())
				{
					nlinfo ("SERVICE: The window displayer was closed by user, need to quit");
					ExitSignalAsked = 1;
				}
			}

			// stop the loop if the exit signal asked
			if (ExitSignalAsked != 0)
			{
				if ( _RequestClosureClearanceCallback )
				{
					if ( _ClosureClearanceStatus == CCClearedForClosure )
					{
						// Clearance has been granted
						CHTimer::endBench();
						break;
					}
					else if ( _ClosureClearanceStatus == CCMustRequestClearance )
					{
						if ( _RequestClosureClearanceCallback() )
						{
							// Direct clearance
							_ClosureClearanceStatus = CCClearedForClosure;
							CHTimer::endBench();
							break;
						}
						else
						{
							// Delayed clearance
							_ClosureClearanceStatus = CCWaitingForClearance;
						}
					}
					else if ( _ClosureClearanceStatus >= CCCallbackThenClose )
					{
						// Always direct closure, because we don't have a connection to the naming service anymore
						// But still call the callback
						_RequestClosureClearanceCallback();
						CHTimer::endBench();
						break;
					}
				}
				else
				{
					// Immediate closure, no clearance needed
					CHTimer::endBench();
					break;
				}
			}

			CConfigFile::checkConfigFiles ();

			updateAdmin ();

			CFile::checkFileChange();

			// update updatable interface
			set<IServiceUpdatable*>::iterator first(_Updatables.begin()), last(_Updatables.end());
			for (; first != last; ++first)
			{
				IServiceUpdatable *updatable = *first;
				updatable->serviceLoopUpdate();
			}

			// get and manage layer 5 messages
			CUnifiedNetwork::getInstance()->update (_UpdateTimeout);


			// update modules
			IModuleManager::getInstance().updateModules();


			// Allow direct closure if the naming service was lost
			if ( _RequestClosureClearanceCallback )
			{
				if ( ! CNamingClient::connected() )
				{
					if ( _ClosureClearanceStatus < CCCallbackThenClose )
						_ClosureClearanceStatus += CCCallbackThenClose; // change status but backup old value
				}
				else
				{
					if ( _ClosureClearanceStatus >= CCCallbackThenClose )
						_ClosureClearanceStatus -= CCCallbackThenClose; // set the closure state back if the NS comes back
				}
			}

			NetSpeedLoop = (sint32) (CTime::getLocalTime () - before);
			UserSpeedLoop = (sint32) (before - bbefore);
			L5CallbackTime = TimeInCallback - LastTimeInCallback;
			LastTimeInCallback = TimeInCallback;
			L5CallbackCount = TotalCallbackCalled - LastTotalCallbackCalled;
			LastTotalCallbackCalled = TotalCallbackCalled;

			if (WindowDisplayer != NULL)
			{
				static TTime lt = 0;
				TTime ct = CTime::getLocalTime();
				if(ct > lt+100)
				{
					lt = ct;
					uint64 rcv, snd, rcvq, sndq;
					rcv = CUnifiedNetwork::getInstance()->getBytesReceived ();
					snd = CUnifiedNetwork::getInstance()->getBytesSent ();
					rcvq = CUnifiedNetwork::getInstance()->getReceiveQueueSize ();
					sndq = CUnifiedNetwork::getInstance()->getSendQueueSize ();

					for (uint i = 0; i < displayedVariables.size(); i++)
					{
						// it s a separator, do nothing
						if (displayedVariables[i].first.empty())
							continue;

						// it s a command, do nothing
						if (displayedVariables[i].first[0] == '@')
							continue;

						string dispName = displayedVariables[i].first;
						string varName = dispName;
						string::size_type pos = dispName.find("|");
						if (pos != string::npos)
						{
							varName = displayedVariables[i].first.substr(pos+1);
							dispName = displayedVariables[i].first.substr(0, pos);
						}

						if (dispName.empty())
							str = "";
						else
							str = dispName + ": ";

						mdDisplayVars.clear ();
						ICommand::execute(varName, logDisplayVars, true);
						const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
						if (strs.size()>0)
						{
							str += strs[0].substr(0,strs[0].size()-1);
						}
						else
						{
							str += "???";
						}
						mdDisplayVars.unlockStrings();
						WindowDisplayer->setLabel (displayedVariables[i].second, str);
					}

				}

			}

			// nldebug ("SYNC: updatetimeout must be %d and is %d, sleep the rest of the time", _UpdateTimeout, delta);

			CHTimer::endBench();

			// Resetting the hierarchical timer must be done outside the top-level timer
			if ( _ResetMeasures )
			{
				CHTimer::clear();
				_ResetMeasures = false;
			}

			MyTAT.deactivate();
		}
	}
	catch (const EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setExitStatus (EXIT_FAILURE);
	}
	catch (const ESocket &e)
	{
		// Catch NeL network exception to release the system cleanly		setExitStatus (EXIT_FAILURE);
		ErrorLog->displayNL( "NeL Exception in \"%s\": %s", _ShortName.c_str(), e.what() );
	}
	catch ( uint ) // SEH exceptions
	{
		ErrorLog->displayNL( "SERVICE: System exception" );
	}

	try
	{
		nlinfo ("SERVICE: Service starts releasing");

		//
		// Call the user service release() if the init() was called
		//

		if (userInitCalled)
		{
			setCurrentStatus("Releasing");
			release ();
		}

		CTransportClass::release();

		//
		// Delete all network connection (naming client also)
		//

		std::vector<std::string> namesOfOnlyServiceToFlushSendingV;
		explode( NamesOfOnlyServiceToFlushSending.get(), string(":"), namesOfOnlyServiceToFlushSendingV, true );
		CUnifiedNetwork::getInstance()->release (FlushSendingQueuesOnExit.get(), namesOfOnlyServiceToFlushSendingV);

		// warn the module layer that the application is about to close
		IModuleManager::getInstance().applicationExit();

//		// release the network
//		CSock::releaseNetwork ();
//
		//
		// Remove the window displayer
		//

		if (WindowDisplayer != NULL)
		{
			DebugLog->removeDisplayer (WindowDisplayer);
			InfoLog->removeDisplayer (WindowDisplayer);
			WarningLog->removeDisplayer (WindowDisplayer);
			ErrorLog->removeDisplayer (WindowDisplayer);
			AssertLog->removeDisplayer (WindowDisplayer);
			CommandLog.removeDisplayer (WindowDisplayer);

			delete WindowDisplayer;
			WindowDisplayer = NULL;
		}

		nlinfo ("SERVICE: Service released successfully");
	}
	catch (const EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setExitStatus (EXIT_FAILURE);
	}

	// remove the stdin monitor thread
	IStdinMonitorSingleton::getInstance()->release(); // does nothing if not initialized

	// release the module manager
	IModuleManager::getInstance().releaseInstance();

	// release the network
	CSock::releaseNetwork ();

	// stop the timeout thread
	MyTAT.quit();
	if (timeoutThread != NULL)
	{
		timeoutThread->wait();
		delete timeoutThread;
	}

	CHTimer::display();
	CHTimer::displayByExecutionPath ();
	CHTimer::displayHierarchical(&CommandLog, true, 64);
	CHTimer::displayHierarchicalByExecutionPathSorted (&CommandLog, CHTimer::TotalTime, true, 64);

	nlinfo ("SERVICE: Service ends");

	return ExitSignalAsked?100+ExitSignalAsked:getExitStatus ();
}

void IService::exit (sint code)
{
	nlinfo ("SERVICE: Somebody called IService::exit(), I have to quit");
	ExitSignalAsked = code;
}

/// Push a new status on the status stack.
void IService::setCurrentStatus(const std::string &status)
{
	// remove the status if it is already in the stack
	_ServiceStatusStack.erase(std::remove(_ServiceStatusStack.begin(), _ServiceStatusStack.end(), status), _ServiceStatusStack.end());

	// insert the status on top of the stack
	_ServiceStatusStack.push_back(status);

}

/// Remove a status from the status stack. If this status is at top of stack, the next status become the current status
void IService::clearCurrentStatus(const std::string &status)
{
	// remove the status of the stack
	_ServiceStatusStack.erase(std::remove(_ServiceStatusStack.begin(), _ServiceStatusStack.end(), status), _ServiceStatusStack.end());
}

/// Add a tag in the status string
void IService::addStatusTag(const std::string &statusTag)
{
	_ServiveStatusTags.insert(statusTag);
}

/// Remove a tag from the status string
void IService::removeStatusTag(const std::string &statusTag)
{
	_ServiveStatusTags.erase(statusTag);
}

/// Get the current status with attached tags
std::string IService::getFullStatus() const
{
	string result;

	// get hold of the status at the top of the status stack
	if (!_ServiceStatusStack.empty())
	{
		result = _ServiceStatusStack.back();
	}

	// add status tags to the result so far
	set<string>::const_iterator first(_ServiveStatusTags.begin()), last(_ServiveStatusTags.end());
	for (; first != last; ++first)
	{
		if (first != _ServiveStatusTags.begin() || !result.empty())
			result += " ";
		result += *first;
	}

	// return the result
	return result.empty()? "Online": result;
}


/*
 * Require to reset the hierarchical timer
 */
void IService::requireResetMeasures()
{
	_ResetMeasures = true;
}


/*
 *
 */
std::string IService::getServiceUnifiedName () const
{
	nlassert (!_ShortName.empty());
	string res;
	if (!_AliasName.empty())
	{
		res = _AliasName+"/";
	}
	res += _ShortName;
	if (_SId.get() != 0)
	{
		res += "-";
		res += toString (_SId.get());
	}
	return res;
}


/*
 * Returns the date of launch of the service. Unit: see CTime::getSecondsSince1970()
 */
uint32		IService::getLaunchingDate () const
{
	return LaunchingDate;
}


void IService::registerUpdatable(IServiceUpdatable *updatable)
{
	_Updatables.insert(updatable);
}

void IService::unregisterUpdatable(IServiceUpdatable *updatable)
{
	_Updatables.erase(updatable);
}

//
// Commands and Variables for controling all services
//

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, LaunchingDate, "date of the launching of the program")
{
	nlunreferenced(human);
	if (get) *pointer = asctime (localtime ((time_t*)&LaunchingDate));
}

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, Uptime, "time from the launching of the program")
{
	if (get)
	{
		if (human)
			*pointer = secondsToHumanReadable (CTime::getSecondsSince1970() - LaunchingDate);
		else
			*pointer = NLMISC::toString(CTime::getSecondsSince1970() - LaunchingDate);
	}
	else
	{
		NLMISC::fromString(*pointer, LaunchingDate);
		LaunchingDate = CTime::getSecondsSince1970() - LaunchingDate;
	}
}

NLMISC_CATEGORISED_VARIABLE(nel, string, CompilationDate, "date of the compilation");
NLMISC_CATEGORISED_VARIABLE(nel, string, CompilationMode, "mode of the compilation");

NLMISC_CATEGORISED_VARIABLE(nel, uint32, NbUserUpdate, "number of time the user IService::update() called");

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, Scroller, "current size in bytes of the sent queue size")
{
	nlunreferenced(human);

	if (get)
	{
		// display the scroll text
		static string foo =	"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! "
							"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! ";
		static int pos = 0;
		*pointer = foo.substr ((pos++)%(foo.size()/2), 10);
	}
}

NLMISC_CATEGORISED_COMMAND(nel, quit, "exit the service", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 0) return false;

	log.displayNL("User ask me with a command to quit");
	ExitSignalAsked = 0xFFFF;

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, brutalQuit, "exit the service brutally", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(log);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 0) return false;

	::exit (0xFFFFFFFF);

	return true;
}


#ifdef MUTEX_DEBUG
NLMISC_CATEGORISED_COMMAND(nel, mutex, "display mutex values", "")
{
	if(args.size() != 0) return false;

	map<CFairMutex*,TMutexLocks>	acquiretimes = getNewAcquireTimes();

	map<CFairMutex*,TMutexLocks>::iterator im;
	for ( im=acquiretimes.begin(); im!=acquiretimes.end(); ++im )
	{
		log.displayNL( "%d %p %s: %.0f %.0f, called %u times th(%d, %d wait)%s", (*im).second.MutexNum, (*im).first, (*im).second.MutexName.c_str(),
			CTime::cpuCycleToSecond((*im).second.TimeToEnter)*1000.0, CTime::cpuCycleToSecond((*im).second.TimeInMutex)*1000.0,
			(*im).second.Nb, (*im).second.ThreadHavingTheMutex, (*im).second.WaitingMutex,
			(*im).second.Dead?" DEAD":"");
	}

	return true;
}
#endif // MUTEX_DEBUG

NLMISC_CATEGORISED_COMMAND(nel, serviceInfo, "display information about this service", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 0) return false;

	log.displayNL ("Service %s '%s' using NeL ("__DATE__" "__TIME__")", IService::getInstance()->getServiceLongName().c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
	log.displayNL ("Service listening port: %d", IService::getInstance()->ListeningPort.get());
	log.displayNL ("Service running directory: '%s'", IService::getInstance()->RunningDirectory.c_str());
	log.displayNL ("Service log directory: '%s'", IService::getInstance()->LogDirectory.c_str());
	log.displayNL ("Service save files directory: '%s'", IService::getInstance()->SaveFilesDirectory.c_str());
	log.displayNL ("Service write files directory: '%s'", IService::getInstance()->WriteFilesDirectory.c_str());
	log.displayNL ("Service config directory: '%s' config filename: '%s.cfg'", IService::getInstance()->ConfigDirectory.c_str(), IService::getInstance()->_LongName.c_str());
	log.displayNL ("Service id: %hu", IService::getInstance()->_SId.get());
	log.displayNL ("Service update timeout: %dms", IService::getInstance()->_UpdateTimeout);
	log.displayNL ("Service %suse naming service", IService::getInstance()->_DontUseNS?"don't ":"");
	log.displayNL ("Service %suse admin executor service", IService::getInstance()->_DontUseAES?"don't ":"");
	log.displayNL ("NeL is compiled in %s mode", CompilationMode.c_str());

	log.displayNL ("Services arguments: %d args", IService::getInstance()->_Args.size ());
	for (uint i = 0; i < IService::getInstance()->_Args.size (); i++)
	{
		log.displayNL ("  argv[%d] = '%s'", i, IService::getInstance()->_Args[i].c_str ());
	}

	log.displayNL ("Naming service info: %s", CNamingClient::info().c_str());

	ICommand::execute ("services", log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, resetMeasures, "reset hierarchical timer", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(args);
	nlunreferenced(log);
	nlunreferenced(quiet);
	nlunreferenced(human);

	IService::getInstance()->requireResetMeasures();
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, getWinDisplayerInfo, "display the info about the pos and size of the window displayer", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(args);
	nlunreferenced(quiet);
	nlunreferenced(human);

	uint32 x,y,w,h;
	IService::getInstance()->WindowDisplayer->getWindowPos (x,y,w,h);
	log.displayNL ("Window Displayer : XWinParam = %d; YWinParam = %d; WWinParam = %d; HWinParam = %d;", x, y, w, h);
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, displayConfigFile, "display the variables of the default configfile", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(args);
	nlunreferenced(quiet);
	nlunreferenced(human);

	IService::getInstance()->ConfigFile.display (&log);
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, getUnknownConfigFileVariables, "display the variables from config file that are called but not present", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(args);
	nlunreferenced(quiet);
	nlunreferenced(human);

	log.displayNL ("%d Variables not found in the configfile '%s'", IService::getInstance()->ConfigFile.UnknownVariables.size(), IService::getInstance()->ConfigFile.getFilename().c_str() );
	for (uint i = 0; i < IService::getInstance()->ConfigFile.UnknownVariables.size(); i++)
	{
		log.displayNL ("  %s", IService::getInstance()->ConfigFile.UnknownVariables[i].c_str());
	}
	return true;
}

// -1 = service is quitting
// 0 = service is not connected
// 1 = service is running
// 2 = service is launching
// 3 = service failed launching

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, State, "Set this value to 0 to shutdown the service and 1 to start the service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)
	{
	}
	else
	{
		if (IService::getInstance()->getServiceShortName() == "AES" || IService::getInstance()->getServiceShortName() == "AS")
		{
			nlinfo ("SERVICE: I can't set State=0 because I'm the admin and I should never quit");
		}
		else if (*pointer == "0" || *pointer == "2")
		{
			// ok, we want to set the value to false, just quit
			nlinfo ("SERVICE: User ask me with a command to quit using the State variable");
			ExitSignalAsked = 0xFFFE;
			IService *srv = IService::getInstance();
			if( srv == NULL )
			{
				return;
			}
			srv->setCurrentStatus("Quitting");
		}
		else
		{
			nlwarning ("SERVICE: Unknown value for State '%s'", (*pointer).c_str());
		}
	}

	// whether reading or writing, the internal value of the state variable should end up as the result of getFullStatus()
	IService *srv = IService::getInstance();
	if( srv == NULL )
	{
		return;
	}
	*pointer= srv->getFullStatus();
}


NLMISC_CATEGORISED_DYNVARIABLE(nel, uint32, ShardId, "Get value of shardId set for this particular service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)
	{
		*pointer = IService::getInstance()->getShardId();
	}
}


NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, CPULoad, "Get instant CPU load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPULoad(); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, ProcessLoad, "Get instant CPU load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessLoad(); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, CPUUserLoad, "Get instant CPU user load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUUserLoad(); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, CPUSytemLoad, "Get instant CPU system load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUSystemLoad(); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, CPUNiceLoad, "Get instant CPU nice processes load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUNiceLoad(); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, CPUIOWaitLoad, "Get instant CPU IO wait load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUIOWaitLoad(); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, ProcessUserLoad, "Get instant CPU user load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessUserLoad(); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, ProcessSystemLoad, "Get instant CPU system load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessSystemLoad(); }
}


NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanCPULoad, "Get instant CPU load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPULoad(CCPUTimeStat::Mean); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanProcessLoad, "Get instant CPU load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessLoad(CCPUTimeStat::Mean); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanCPUUserLoad, "Get instant CPU user load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUUserLoad(CCPUTimeStat::Mean); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanCPUSytemLoad, "Get instant CPU system load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUSystemLoad(CCPUTimeStat::Mean); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanCPUNiceLoad, "Get instant CPU nice processes load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUNiceLoad(CCPUTimeStat::Mean); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanCPUIOWaitLoad, "Get instant CPU IO wait load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUIOWaitLoad(CCPUTimeStat::Mean); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanProcessUserLoad, "Get instant CPU user load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessUserLoad(CCPUTimeStat::Mean); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, MeanProcessSystemLoad, "Get instant CPU system load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessSystemLoad(CCPUTimeStat::Mean); }
}



NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakCPULoad, "Get instant CPU load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPULoad(CCPUTimeStat::Peak); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakProcessLoad, "Get instant CPU load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessLoad(CCPUTimeStat::Peak); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakCPUUserLoad, "Get instant CPU user load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUUserLoad(CCPUTimeStat::Peak); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakCPUSytemLoad, "Get instant CPU system load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUSystemLoad(CCPUTimeStat::Peak); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakCPUNiceLoad, "Get instant CPU nice processes load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUNiceLoad(CCPUTimeStat::Peak); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakCPUIOWaitLoad, "Get instant CPU IO wait load of the server")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getCPUIOWaitLoad(CCPUTimeStat::Peak); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakProcessUserLoad, "Get instant CPU user load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessUserLoad(CCPUTimeStat::Peak); }
}

NLMISC_CATEGORISED_DYNVARIABLE(cpu, float, PeakProcessSystemLoad, "Get instant CPU system load of the process/service")
{
	nlunreferenced(human);
	// read or write the variable
	if (get)	{ *pointer = IService::getInstance()->getCPUUsageStats().getProcessSystemLoad(CCPUTimeStat::Peak); }
}


} //NLNET
