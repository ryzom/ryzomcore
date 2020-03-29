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
// include
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/service.h"

// game_share
#include "game_share/utils.h"
#include "game_share/deployment_configuration.h"

// patchman
#include "administered_module.h"
#include "patchman_constants.h"
#include "deployment_configuration_synchroniser.h"

// nelns
#include "../../nelns/admin_executor_service/log_report.cpp" // including cpp because the "DSP to Makefile" system does not suppport external dependencies

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace PATCHMAN;


//-----------------------------------------------------------------------------
// config variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// module LogReport
//-----------------------------------------------------------------------------
class CLogReportModule :
	public CAdministeredModuleBase
{
public:
	
	CLogReportModule() {}

	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CLogReportModule, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CLogReportModule, displayLogDirs,
			"Display the log directories available in the specified domain directory (ex: will return 'egs_mainland01', 'fs_ring01', etc., not 'live' or 'next'). If domain dir not specified, will use the available domains", "[<domainDirectory>]");
		NLMISC_COMMAND_HANDLER_ADD(CLogReportModule, makeLogReport,
			"Start a background task that will build a report of logs produced on the machine (ex: \"makeLogPath start v\" -> starts log report for logs of current version)", "[start=<available domain, list will be displayed if multiple found>|<domainName>|<logpath>|stop [v|v*|v<n>|v<n>+|v<n>-|* [<recurseSubDirs>=1]]]");
		NLMISC_COMMAND_HANDLER_ADD(CLogReportModule, displayLogReport,
			"Display summary, all or a part of the log report built by makeLogReport (default: summary)", "[all | <service id> | <-p page number>]");
	NLMISC_COMMAND_HANDLER_TABLE_END

	bool initModule(const TParsedCommandLine &initInfo)
	{
		// setup a variable to build the init message in...
		NLMISC::CSString logMsg;

		// initialise the module base classes...
		logMsg+= CAdministeredModuleBase::init(initInfo);

		// deal with the obligatory 'path' argument
		const TParsedCommandLine *rootArg = initInfo.getParam("path");
		DROP_IF(rootArg==NULL,"path() parameter not found in command line",return false);
		_RootPath = NLMISC::CPath::standardizePath(rootArg->ParamValue);

		// we're all done so let the world know
		registerProgress(string("LogReportModule Initialised: ")+logMsg);
		setStateVariable("State","Initialised");
		broadcastStateInfo();

		return true;
	}

	void onModuleUp(IModuleProxy *module)
	{
		// allow the base classes a chance to do their stuff
		CAdministeredModuleBase::onModuleUp(module);

		// treat patch manager update specially to get the latest depcfg update
		if (CSString(module->getModuleManifest()).contains(ManifestEntryIsAdministrator))
		{
			// register with the administrator
			CServerPatchManagerProxy manager(module);
			manager.registerAdministeredModule(this,false,false,false,true);
			registerProgress("registering with manager: "+module->getModuleName());
		}
	}

	void onModuleDown(IModuleProxy *module)
	{
		// allow the base classes a chance to do their stuff
		CAdministeredModuleBase::onModuleDown(module);
	}

	void onModuleUpdate()
	{
		H_AUTO(CServerPatchApplier_onModuleUpdate);

		// allow the base class a chance to do it's stuff
		CAdministeredModuleBase::onModuleUpdate();
	}

	NLMISC_CLASS_COMMAND_DECL(displayLogDirs)
	{
		vector<string> domainDirs;
		if ( args.size() > 0 )
			domainDirs.push_back( args[0] );
		else
		{
			DEPCFG::TDomainNames domainNames = getDomainNames();
			if ( domainNames.empty() )
			{
				log.displayNL( "No domain installed on this machine" );
				return true;
			}
			for ( DEPCFG::TDomainNames::const_iterator it=domainNames.begin(); it!=domainNames.end(); ++it )
				domainDirs.push_back( _RootPath + (*it) );
		}
		for ( vector<string>::const_iterator it=domainDirs.begin(); it!=domainDirs.end(); ++it )
		{
			const string& domainDir = (*it);
			log.displayNL( "Browsing subdirs of %s...", domainDir.c_str() );
			vector<string> dirs;
			if ( getLogSubDirs( domainDir, dirs ) )
			{
				for ( vector<string>::const_iterator it=dirs.begin(); it!=dirs.end(); ++it )
					log.displayNL( (*it).c_str() );
			}
			else
				log.displayNL( "No subdirs found in %s or directory not found", domainDir.c_str() );
		}
		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(makeLogReport)
	{
		bool start = args.empty() || (!args.empty() && args[0] != "stop");
		string logTarget = "v";
		bool recurseSubDirs = true;
		if ( args.size() > 1 )
		{
			logTarget = args[1];
			if ( args.size() > 2 )
				recurseSubDirs = (atoi(args[2].c_str()) == 1);
		}

		if (start)
		{
			// Attempt to start the background thread
			if ( ! _MakingLogTask.isRunning() )
			{
				// Set log path(s)
				vector<string> logPaths(1);
				bool useAutoDomain = args.empty() || (args[0] == "start");
				if ( useAutoDomain )
				{
					DEPCFG::TDomainNames domainNames = getDomainNames();
					switch ( domainNames.size() )
					{
					case 0:
						log.displayNL( "No domain installed on this machine" );
						return true; //break;
					case 1:
						logPaths[0] = _RootPath + domainNames[0];
						break;
					default:
						log.displayNL( "Multiple domains are installed on this machine. Please choose between these:" );
						for ( DEPCFG::TDomainNames::const_iterator it=domainNames.begin(); it!=domainNames.end(); ++it )
							log.displayRawNL( "%s", (*it).c_str() );
						return true;
					}
				}
				else
				{
					// Try interpreting the argument as a path, or a a domain if the path is not valid
					logPaths[0] = args[0];
					if ( (! CFile::isExists( logPaths[0] )) || (! CFile::isDirectory( logPaths[0] )) )
					{
						logPaths[0] = _RootPath + args[0];
						if ( (! CFile::isExists( logPaths[0] )) || (! CFile::isDirectory( logPaths[0] )) )
						{
							log.displayNL( "No such path %s or %s", args[0].c_str(), logPaths[0].c_str() );
							return true;
						}
					}
				}
				if ( recurseSubDirs )
					getLogSubDirs( string(logPaths[0]), logPaths ); // avoid passing fisrt arg by ref because it is in the output vector
				_MakingLogTask.setLogPaths( logPaths );

				// Set log target and start the background thread
				_MakingLogTask.setLogTarget( logTarget );
				_MakingLogTask.start();
				log.displayNL( "Task started in %s%s", logPaths[0].c_str(), recurseSubDirs ? NLMISC::toString(" (and %u subdirectories)", logPaths.size()-1).c_str() : "" );
				setStateVariable("State","Running");
			}
			else
			{
				log.displayNL( "The makeLogReport task is already in progress, please wait until the end, or call 'makeLogReport stop' to terminate it");
			}
		}
		else if ( args[0] == "stop" )
		{
			// Attempt to stop the background thread before its ending
			if ( _MakingLogTask.isRunning() )
			{
				log.displayNL( "Stopping makeLogReport task..." );
				_MakingLogTask.terminateTask();
				log.displayNL( "Task stopped" );
				setStateVariable("State","Stopped");
			}
			else
				log.displayNL( "Task is not running" );
		} 

		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(displayLogReport)
	{
		if ( !_MakingLogTask.getLogReport() )
		{
			log.displayNL( "Please call makeLogReport first");
			return true;
		}
		if ( _MakingLogTask.isRunning() )
		{
			uint currentFile, totalFiles;
			_MakingLogTask.getLogReport()->getProgress( currentFile, totalFiles );
			log.displayNL( "Please wait until the completion of the makeLogReport task, or stop it" );
			log.displayNL( "Currently processing file %u of %u", currentFile, totalFiles );
			return true;
		}
		log.displayNL( _MakingLogTask.isComplete() ? "Full stats:" : "Temporary stats" );
		if ( args.empty() )
		{
			// Summary
			_MakingLogTask.getLogReport()->report( &log, false );
		}
		else if ( args[0] == "all" )
		{
			// All services
			_MakingLogTask.getLogReport()->report( &log, true );
		}
		else if ( args[0] == "-p" )
		{
			// By page
			if ( args.size() < 2 )
			{
				log.displayNL( "Page number missing" );
				return false;
			}
			uint pageNum = atoi( args[1].c_str() );
			_MakingLogTask.getLogReport()->reportPage( pageNum, &log );
		}
		else
		{
			// By service
			_MakingLogTask.getLogReport()->reportByService( args[0], &log );
		}

		return true;
	}

protected:

	/// Append the subdirectories into logDirs, except non-log dirs ('live', 'next', etc.). Return false if dir not found or not having subdirs.
	bool getLogSubDirs( const std::string& domainDir, std::vector<std::string>& logDirs ) const
	{
		vector<string> dirs;
		CPath::getPathContent( domainDir, false, true, false, dirs );
		if (dirs.empty())
			return false;

		for ( vector<string>::const_iterator it=dirs.begin(); it!=dirs.end(); ++it )
		{
			const string& dir = (*it);
			if ( (dir.find("/live/") != string::npos) || (dir.find("/next/") != string::npos) || (dir.find("/data_shard_local/") != string::npos) || (dir.find("/save_shard/") != string::npos) || (dir.find("/www/") != string::npos) )
				continue;
			logDirs.push_back( dir );
		}
		return true;
	}

	/**
	 * Get the domain names for the current machine. Must not be called too early
	 * because the deployment configuration is synchronized by another module during
     * execution, not at initialization time.
	 */
	DEPCFG::TDomainNames getDomainNames() const
	{
		DEPCFG::SHostDescription hostDescription;
		DEPCFG::CDeploymentConfiguration::getInstance().getHost( IService::getInstance()->getHostName(), hostDescription );
		return hostDescription.Domains;
	}

private:

	CMakeLogTask			_MakingLogTask;
	std::string				_RootPath;
};


NLNET_REGISTER_MODULE_FACTORY(CLogReportModule, "LogReportModule");
