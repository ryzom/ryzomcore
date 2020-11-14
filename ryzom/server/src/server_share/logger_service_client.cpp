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

#include "stdpch.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "game_share/utils.h"
#include "logger_service_client.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CVariable<bool>	VerboseLogger("LGS", "VerboseLogger", "Activate verbose logging in serive output", false, 0, true);

extern void forceLinkOfAllLogs();

namespace LGS
{

	class CLoggerServiceClient 
		:	public ILoggerServiceClient,
			public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
			IModuleTrackerCb
	{
		friend class ILoggerServiceClient;
		/// A flag stating if the logger comm is started
		NL_MISC_SAFE_CLASS_GLOBAL(bool, CommStarted, false);
//		static bool				_CommStarted;

		/// The module proxies of the logger service
//		set<TModuleProxyPtr>	_LoggerServices;
		typedef CModuleTracker<TModuleClassPred>	TLoggerServices;
		TLoggerServices	_LoggerServices;

		typedef map<std::string, TLogDefinition>	TLogDefinitions;
		/// The log definitions, stored by log name
		static CLoggerServiceClient::TLogDefinitions &getLogDefinitions()
		{
			static TLogDefinitions	logDefinitions;

			return logDefinitions;
		}

		typedef vector<TLogInfo>	TLogInfos;
		/// the list of log stored during a service loop
		TLogInfos		_LogInfos;

		/// keep track of the number of open context, must be 0 at end of service loop
		uint32			_NbOpenContext;


	public:
		CLoggerServiceClient()
			:	_LoggerServices(TModuleClassPred("LoggerService")),
				_NbOpenContext(0)
		{
			_LoggerServices.init(this, this);

			if (getGlobal_CommStarted())
				startLoggerComm();

			// this call is jsut to make sure ALL log definition are included by
			// the linker
			forceLinkOfAllLogs();
		}

		void onModuleUpdate()
		{
			if (getGlobal_CommStarted())
			{
				if (_NbOpenContext != 0)
				{
					WARN("LoggerClient : there are "<<_NbOpenContext<<" open context not closed !. Logs for this loop will be lost");
					// TODO : have a better handling of unpaired log context to not loast all logs
					_LogInfos.clear();
				}
				else
				{
					// send the logs to all known logger services
					const TLoggerServices::TTrackedModules &loggers = _LoggerServices.getTrackedModules();
					
					TLoggerServices::TTrackedModules::iterator first(loggers.begin()), last(loggers.end());
					for (; first != last; ++first)
					{
						CLoggerServiceProxy logger(*first);
						
						logger.reportLog(this, _LogInfos);
					}
					
					// cleanup accumulated logs
					_LogInfos.clear();
				}
			}
		}

		void registerWithLogger(IModuleProxy *loggerService)
		{
			// build a simple vector of log definitions
			vector<TLogDefinition> logDefs;
			TLogDefinitions::iterator first(getLogDefinitions().begin()), last(getLogDefinitions().end());
			for (; first != last; ++first)
			{
				logDefs.push_back(first->second);
			}

			// register the client
			CLoggerServiceProxy logger(loggerService);
			logger.registerClient(this, IService::getInstance()->getShardId(), logDefs);
		}

		/// Add a set of log description 
		static void addLogDefinitions(const std::vector<TLogDefinition> &logDefs)
		{
			BOMB_IF(getGlobal_CommStarted(), "Registering of log definition done AFTER comm started !", return);

			for (uint i=0; i<logDefs.size(); ++i)
			{
				nlassert(getLogDefinitions().find(logDefs[i].getLogName()) == getLogDefinitions().end());
				getLogDefinitions().insert(make_pair(logDefs[i].getLogName(), logDefs[i]));
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Implementation of IModuleTrackerCB
		///////////////////////////////////////////////////////////////////////
		void onTrackedModuleUp(IModuleProxy *moduleProxy)
		{
			// if comm started, send our log definition
			if (getGlobal_CommStarted())
			{
				registerWithLogger(moduleProxy);
			}
		}
		void onTrackedModuleDown(IModuleProxy *moduleProxy)
		{
			// nothing
		}
		///////////////////////////////////////////////////////////////////////
		// Implementation of ILoggerServiceClient
		///////////////////////////////////////////////////////////////////////

		/// Activate comm with any known logger service
		void startLoggerComm()
		{
			getGlobal_CommStarted() = true;

			// send the log definitions to all known loggers
			const TLoggerServices::TTrackedModules &loggers = _LoggerServices.getTrackedModules();

			TLoggerServices::TTrackedModules::iterator first(loggers.begin()), last(loggers.end());
			for (; first != last; ++first)
			{
				registerWithLogger(*first);
			}

			// the next module update will send the accumulated logs
		}

		/// Send a log to the logger service (actual send granularity is handled by CLoggerServiceClient)
		void sendLog(const TLogInfo &logInfo)
		{
			TLogDefinitions::iterator it(getLogDefinitions().find(logInfo.getLogName()));
			BOMB_IF(it == getLogDefinitions().end(), "sendLog : Unknow log or log context named '"<<logInfo.getLogName()<<"'", return);
			BOMB_IF(it->second.getParams().size() != logInfo.getParams().size(), "sendLog : on log "<<logInfo.getLogName()<<", invalid number of log parameter. Need "<<it->second.getParams().size()<<", received "<<logInfo.getParams().size(), return);
			_LogInfos.push_back(logInfo);
			// log are effectively sent at next module update

			if (VerboseLogger)
			{
				const TLogDefinition &ld = it->second;
				char buffer[1024];
				int pos = sprintf(buffer, "LGS : log : %s : %s", 
					ld.getLogName().c_str(), 
					ld.getLogText().c_str());
				for (uint i=0; i<ld.getParams().size(); ++i)
				{
					pos += sprintf(buffer+pos, ", %s = %s", ld.getParams()[i].getName().c_str(), logInfo.getParams()[i].toString().c_str());
				}
				nlinfo("%s", buffer);
			}
		
		}

		/// Push a new log context (any following logs will be stored inside this context)
		void pushLogContext(const std::string &contextName)
		{
			// check the context name
			TLogDefinitions::iterator it(getLogDefinitions().find(contextName));
			BOMB_IF(it == getLogDefinitions().end(), "pushLogContext : Unknow log or log context named '"<<contextName<<"'", return);
			const TLogDefinition &ld = it->second;
			BOMB_IF(!ld.getContext(), "Push log context with name '"<<contextName<<"' is not a context name", return);
			// store the log info for the context
			_LogInfos.push_back(TLogInfo());
			_LogInfos.back().setLogName(contextName);
			_LogInfos.back().setTimeStamp(0);
			++_NbOpenContext;

			if (VerboseLogger)
			{
				nlinfo("LGS : Open log context '%s'", contextName.c_str());
			}
		}

		/// Pop a log context
		void popLogContext(const std::string &contextName)
		{
			// advance to the previous opening log context
			TLogInfos::reverse_iterator it=_LogInfos.rbegin();

			while (it != _LogInfos.rend() && it->getTimeStamp() != 0 && it->getLogName() != contextName)
				++it;

			BOMB_IF(it == _LogInfos.rend(), "popLogContext : Can't find opening context", return);

			// ok, we have found the opening tag
			if (it == _LogInfos.rbegin())
			{
				// the log context is empty, remove it
				_LogInfos.pop_back();
			}
			else
			{
				// create the log context closing
				_LogInfos.push_back(TLogInfo());
				_LogInfos.back().setLogName(contextName);
				// tag as 'closing' with std::numeric_limits<uint32>::max()
				_LogInfos.back().setTimeStamp(std::numeric_limits<uint32>::max());
			}
			--_NbOpenContext;
			if (VerboseLogger)
			{
				nlinfo("LGS : Close log context '%s'", contextName.c_str());
			}
		}

	};

	NLNET_REGISTER_MODULE_FACTORY(CLoggerServiceClient, "LoggerServiceClient");


	///////////////////////////////////////////////////////////////////////////
	// Logger service client static member instance
	///////////////////////////////////////////////////////////////////////////
//	bool									CLoggerServiceClient::_CommStarted = false;

	///////////////////////////////////////////////////////////////////////////
	// ILogger service client static functions
	///////////////////////////////////////////////////////////////////////////

	/// Add a set of log description 
	void ILoggerServiceClient::addLogDefinitions(const std::vector<TLogDefinition> &logDefs)
	{
		CLoggerServiceClient::addLogDefinitions(logDefs);
	}

	void ILoggerServiceClient::startLoggerComm()
	{
		CLoggerServiceClient::getGlobal_CommStarted() = true;
		if (CLoggerServiceClient::isInitialized())
		{
			// call the start logger method in the concrete class
			static_cast<CLoggerServiceClient*>(ILoggerServiceClient::getInstance())->startLoggerComm();
		}
	}


} // namespace LGS

