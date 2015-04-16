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
#include "nel/misc/types_nl.h"
#include <time.h>
#include "nel/misc/file.h"
#include "nel/misc/sstring.h"
#include "nel/misc/mutable_container.h"
#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "admin_modules_itf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


void as_forceLink() {}

namespace ADMIN
{

	/// name of the persistent state file
	const char *ASPersistentStateFilename = "as_state.txt";

	class CAdminService 
		:	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
			CAdminServiceSkel,
			CAdminServiceWebItf,
			IModuleTrackerCb
	{

		enum 
		{
			/// The maximum time without report from an AES before flagging it as 'not responding'
			AES_REPORT_WARNING_DELAY = 5,
		};

		typedef CModuleTracker<TModuleClassPred>	TAESTracker;
		/// Tracker for EAS modules
		TAESTracker		_AESTracker;

		struct TAESServices
		{
			/// The date of last report send by this AES. Used to display 'not responding' AES
			uint32					LastReportDate;
			/// The list of service known by this AES
			vector<TServiceStatus>	ServiceStatus;

			TAESServices()
				:	LastReportDate(0)
			{}
		};

		/// all known service status by known AES
		typedef map<TModuleProxyPtr,  TAESServices>	TKnownServices;
		TKnownServices	_KnownServices;

		/// data for a command comming form web and waiting execution by the module task
		struct TPendingWebCommand
		{
			/// Received command date
			uint32		ReceptionDate;	
			/// Is this a control command (otherwise it's a service command)
			bool		ControlCommand;
			/// Sock id of the web connection that wait the command result
			TSockId		Requester;
			/// Alias of the command target service
			string		ServiceAlias;
			/// the command and it's parameters
			string		Command;
		};
		typedef uint32 TCommandId;
		TCommandId			_NextCommandId;
		typedef map<TCommandId, TPendingWebCommand>	TPendingWebCommands;
		/// A stack of web command request to be treated by module task
		TPendingWebCommands	_PendingWebCommands;

		/// The global running state of the domain
//		TRunningOrders	_GlobalOrders;

		typedef string	TShardName;

		typedef map<TShardName, TShardOrders> TShardsOrders;
		/// The running state of each shard
		TShardsOrders	_ShardOrders;

		/// a flag to write the state file at next module update
		bool			_NeedToWriteStateFile;


	public:
		CAdminService()
			:	_AESTracker(TModuleClassPred("AdminExecutorService")),
				_NextCommandId(0),
//				_GlobalOrders(TRunningOrders::ro_running),
				_NeedToWriteStateFile(false)

		{
			CAdminServiceSkel::init(this);
			_AESTracker.init(this, this);
		}

		~CAdminService()
		{}

		void setShardOrders(const std::string &shardName, TShardOrders newOrders)
		{
			_ShardOrders[shardName] = newOrders;
//			switch(_GlobalOrders.getValue())
//			{
//			case TRunningOrders::ro_stopped:
//				IService::getInstance()->addStatusTag("GLOBAL_STOPPED");
//				break;
//			case TRunningOrders::ro_running:
//				IService::getInstance()->removeStatusTag("GLOBAL_STOPPED");
//				break;
//			}

			_NeedToWriteStateFile = true;

			// update all AES with the new state
			CAdminExecutorServiceProxy::broadcast_setShardOrders(_AESTracker.getTrackedModules().begin(), _AESTracker.getTrackedModules().end(), 
				this, shardName, newOrders);
		}

		/// Methods called by a module task to handle web command request
		void sendCommandToAES(TCommandId commandId, TPendingWebCommand &pwc)
		{
			// look in the list of known state to retrieve the target of the command
			TKnownServices::iterator first(_KnownServices.begin()), last(_KnownServices.end());
			for (; first != last; ++first)
			{
				const vector<TServiceStatus> &status = first->second.ServiceStatus;
				for (uint i=0; i<status.size(); ++i)
				{
					const TServiceStatus &ss = status[i];
					if (ss.getServiceAliasName() == pwc.ServiceAlias)
					{
						// ok, we found it !
						CAdminExecutorServiceProxy aes(first->first);

						if (pwc.ControlCommand)
						{
							// this is a control command
							aes.controlCmd(this, commandId, pwc.ServiceAlias, pwc.Command);
						}
						else
						{
							// this is a service command
							aes.serviceCmd(this, commandId, pwc.ServiceAlias, pwc.Command);
						}

						// terminate here !
						return;
	//-----------------------------------------------------
					}
				}
			}

			// not found !
			CAdminServiceWebItf::commandResult(pwc.Requester, pwc.ServiceAlias, "ERROR : AS : unknown service alias");
			// remove the pending command
			_PendingWebCommands.erase(commandId);
		}


		bool initModule(const TParsedCommandLine &pcl)
		{
			CModuleBase::initModule(pcl);

			// read the command line
			const TParsedCommandLine *webPort = pcl.getParam("webPort");
			nlassert(webPort != NULL);
			uint16 port;
			NLMISC::fromString(webPort->ParamValue, port);
			// open the web interface
			CAdminServiceWebItf::openItf(port);

			// read the persistent state file if any
			string filename = CPath::standardizePath(IService::getInstance()->SaveFilesDirectory.toString(), true)+ASPersistentStateFilename;
			FILE *fp = fopen(filename.c_str(), "rt");
			if (fp != NULL)
			{
				char buffer[1024];
				char *ret;
				while ((ret=fgets(buffer, 1024, fp)) != NULL)
				{
					CSString line(buffer);
					string cmd = line.firstWord(true);

					if (cmd == "ShardOrders")
					{
						string shardName = line.firstWord(true);
						string orders = line.firstWord(true);
						TShardOrders shardOrders(orders);
						if (shardOrders != TShardOrders::invalid_val)
							setShardOrders(shardName, shardOrders);
					}
				}
				// clear the flag because 'setGlobalState' has set it
				_NeedToWriteStateFile = false;

				fclose(fp);
			}

			return true;
		}

		void onModuleUpdate()
		{
			H_AUTO(CAdminService_onModuleUpdate);

			CAdminServiceWebItf::update();

			if (_NeedToWriteStateFile)
			{
				string filename = CPath::standardizePath(IService::getInstance()->SaveFilesDirectory.toString(), true)+ASPersistentStateFilename;
				FILE *fp = fopen(filename.c_str(), "wt");
				if (fp != NULL)
				{
					CSString line;
					TShardsOrders::iterator first(_ShardOrders.begin()), last(_ShardOrders.end());
					for (; first != last; ++first)
					{
						line << "ShardOrders "<<first->first<<" "<<first->second.toString()<<"\n";
					}
					fputs(line.c_str(), fp);
					fclose(fp);

					_NeedToWriteStateFile = false;
				}
			}

			uint32 now = NLMISC::CTime::getSecondsSince1970();

			// check for timeout commands
			TPendingWebCommands::iterator first(_PendingWebCommands.begin()), last(_PendingWebCommands.end());
			for (; first != last; ++first)
			{
				TPendingWebCommand &pwc = first->second;

				if (now - pwc.ReceptionDate > 10)
				{
					CAdminServiceWebItf::commandResult(pwc.Requester, pwc.ServiceAlias, "ERROR : no response from service or AES");
					_PendingWebCommands.erase(first);
					// check at next update for the rest
					break;
				}
			}

			{
				// save one high rez graph at a time
				static string lastCheckedBuffer;

				THighRezBuffers::iterator it(_HighRezBuffers.upper_bound(lastCheckedBuffer));
				if (it == _HighRezBuffers.end())
					lastCheckedBuffer = "";
				else
				{
					lastCheckedBuffer = it->first;
					THighRezBuffer &hrBuffer = it->second;
					if (hrBuffer.Dirty)
					{
						// save this buffer
						CMemStream sbuff;
						// write the updated buffer
						sbuff.serial(hrBuffer);

						string filename = getHighRezBufferFilename(it->first);
						NLMISC::COFile of(filename);

						if (of.isOpen())	// test added, because sometime on windows, the file fail to open !
						{
							of.serialBuffer((uint8*)sbuff.buffer(), sbuff.size());
							
							hrBuffer.Dirty = false;
						}
						else
						{
							nlwarning("CAdminService::onUpdateModule : failed to open file %s for writing", filename.c_str());
						}
					}
				}
			}
		}

		///////////////////////////////////////////////////////////////////////
		//// Virtuals from IModuleTrackerCb
		///////////////////////////////////////////////////////////////////////

		virtual void onTrackedModuleUp(IModuleProxy *moduleProxy)
		{
			nldebug("AES module '%s' UP", moduleProxy->getModuleName().c_str());
			
			// send it the current global state
			CAdminExecutorServiceProxy aes(moduleProxy);

			TShardsOrders::iterator first(_ShardOrders.begin()), last(_ShardOrders.end());
			for (; first != last; ++first)
			{
				aes.setShardOrders(this, first->first, first->second);
			}
		}
		virtual void onTrackedModuleDown(IModuleProxy *moduleProxy)
		{
			nldebug("AES module '%s' DOWN", moduleProxy->getModuleName().c_str());

			
			// check for any pending commands with this AES
			TAESServices &as = _KnownServices[moduleProxy];
			for (uint i=0; i<as.ServiceStatus.size(); ++i)
			{
				TServiceStatus &ss = as.ServiceStatus[i];
				const string &aliasName = ss.getServiceAliasName();

retry_pending_command:
				TPendingWebCommands::iterator first(_PendingWebCommands.begin()), last(_PendingWebCommands.end());
				for (; first != last; ++first)
				{
					TPendingWebCommand &pwc = first->second;
					if (pwc.ServiceAlias == aliasName)
					{
						// remove this command
						CAdminServiceWebItf::commandResult(pwc.Requester, pwc.ServiceAlias, "ERROR : connection lost with AES during command");
						TCommandId commandId = first->first;
						_PendingWebCommands.erase(first);

						// restart the loop to avoid iterator dodging
						goto retry_pending_command;
					}
				}
			}

			// remove any service status
			_KnownServices.erase(moduleProxy);

		}

		///////////////////////////////////////////////////////////////////////
		//// Virtuals from CAdminServiceSkel
		///////////////////////////////////////////////////////////////////////

		// An AES send an update of the list of service up
		virtual void upServiceUpdate(NLNET::IModuleProxy *sender, const std::vector < TServiceStatus > &serviceStatus)
		{
			if (_AESTracker.getTrackedModules().find(sender) == _AESTracker.getTrackedModules().end())
			{
				nlwarning("'%s' send upServiceUpdate but is not an valid AES", sender->getModuleName().c_str());
				return;
			}

			_KnownServices[sender].LastReportDate = NLMISC::CTime::getSecondsSince1970();
			_KnownServices[sender].ServiceStatus = serviceStatus;

			// check that we have this shards in the shard orders table
			for (uint i=0; i<serviceStatus.size(); ++i)
			{
				const TServiceStatus &ss = serviceStatus[i];

				if (_ShardOrders.find(ss.getShardName()) == _ShardOrders.end())
				{
					// the shard is not in the table, add it
					_ShardOrders[ss.getShardName()] = TShardOrders::so_autostart_on;
				}
			}
		}

		// An AES send graph data update
		virtual void graphUpdate(NLNET::IModuleProxy *sender, const TGraphDatas &graphDatas)
		{
			// dump the datas
//			nldebug("Received graph data for time %u", 
//				graphDatas.getCurrentTime());
//
//			for (uint i=0; i<graphDatas.getDatas().size(); ++i)
//			{
//				nldebug("  %s.%s = %f", 
//					graphDatas.getDatas()[i].getServiceName().c_str(), 
//					graphDatas.getDatas()[i].getVarName().c_str(), 
//					graphDatas.getDatas()[i].getValue());
//			}

			for (uint i=0; i<graphDatas.getDatas().size(); ++i)
			{
				const TGraphData &gd = graphDatas.getDatas()[i];

				// compute var filename
				CSString rrdfilename = CPath::standardizePath (IService::getInstance()->ConfigFile.getVar("RRDVarPath").asString());
				rrdfilename << gd.getServiceAlias() <<"." <<gd.getVarName()<<".rrd";

				CSString arg;
				
				if (!NLMISC::CFile::fileExists(rrdfilename))
				{
					arg <<"create "<<rrdfilename
						<<" --step "<<toString(gd.getSamplePeriod())
						<<" DS:var:GAUGE:"<<toString(gd.getSamplePeriod()*2)
						<<":U:U RRA:AVERAGE:0.5:1:1000 RRA:AVERAGE:0.5:10:1000 RRA:AVERAGE:0.5:100:1000";
					launchProgram(IService::getInstance()->ConfigFile.getVar("RRDToolPath").asString(), arg);
					arg = "";
				}

				arg<<"update "<<rrdfilename<<" "<<toString (graphDatas.getCurrentTime())<<":"<<toString(gd.getValue());
				launchProgram(IService::getInstance()->ConfigFile.getVar("RRDToolPath").asString(), arg);
			}
		}

		enum
		{
			HR_BUFFER_SIZE = 5000,
		};

		/// Circular buffer to store high resolution samples
		struct THighRezBuffer
		{
			bool		Dirty;
			uint32		NBSample;
			uint32		FrameStart;
			uint32		FrameEnd;		// == FrameStart if empty

			struct THighRezItem
			{
				uint32		Date;
				TTime		SampleTick;
				double		Value;

				void serial(NLMISC::IStream &s)
				{
					s.serial(Date);
					s.serial(SampleTick);
					s.serial(Value);
				}
			};

			vector<THighRezItem>	Datas;


			THighRezBuffer()
				:	Dirty(false),
					NBSample(HR_BUFFER_SIZE),
					FrameStart(0),
					FrameEnd(0)
			{
				Datas.resize(NBSample);
			}

			void serial(NLMISC::IStream &s)
			{
				s.serial(NBSample);
				s.serial(FrameStart);
				s.serial(FrameEnd);
				s.serialCont(Datas);

				if (s.isReading())
				{
					// make some adjustment in case of HR_BUFFER_SIZE change
					Datas.resize(HR_BUFFER_SIZE);
					FrameEnd %= HR_BUFFER_SIZE;
					FrameStart %= HR_BUFFER_SIZE;
					NBSample = HR_BUFFER_SIZE;
				}
			}
		};

		typedef map<std::string , THighRezBuffer>	THighRezBuffers;
		THighRezBuffers	_HighRezBuffers;

		string getHighRezBufferFilename(const std::string &varAddr)
		{
			CSString filename = CPath::standardizePath (IService::getInstance()->ConfigFile.getVar("RRDVarPath").asString());
			filename << varAddr<<".hrd";
	
			return filename;
		}

		// An AES send high rez graph data update
		virtual void highRezGraphUpdate(NLNET::IModuleProxy *sender, const THighRezDatas &graphDatas)
		{
			// dump the datas
//			nldebug("Received high rez graph info for var %s from service %s", 
//				graphDatas.getServiceName().c_str(),
//				graphDatas.getVarName().c_str());
//
//			for (uint i=0; i<graphDatas.getDatas().size(); ++i)
//			{
//				nldebug("  At %10"NL_I64"u	%f", 
//					graphDatas.getDatas()[i].getSampleTick(), 
//					graphDatas.getDatas()[i].getValue());
//			}

			// compute the var address
			CSString varAddr;
			varAddr << graphDatas.getServiceAlias()<<"."<<graphDatas.getVarName();


			// check if the var is already known
			bool notExist = (_HighRezBuffers.find(varAddr) == _HighRezBuffers.end());
			// create it anyway
			THighRezBuffer &hrBuffer = _HighRezBuffers[varAddr];

			if (notExist)
			{
				// compute var filename
				CSString filename = getHighRezBufferFilename(varAddr);

				// we don't know this var, try to load it from file
				if (CFile::isExists(filename))
				{
					// the file exist, load it
					CIFile ifile(filename);
					hrBuffer.serial(ifile);
				}
			}

			// store the new data in the buffer
			for (uint i=0; i<graphDatas.getDatas().size(); ++i)
			{
				const THighRezData &data = graphDatas.getDatas()[i];

				// mark this buffer as dirty
				hrBuffer.Dirty = true;
				
				nlassert(hrBuffer.FrameEnd < hrBuffer.Datas.size());
				hrBuffer.Datas[hrBuffer.FrameEnd].Date = graphDatas.getCurrentTime();		
				hrBuffer.Datas[hrBuffer.FrameEnd].SampleTick = data.getSampleTick();
				hrBuffer.Datas[hrBuffer.FrameEnd].Value = data.getValue();
				
				// advance write pointer
				++hrBuffer.FrameEnd;
				if (hrBuffer.FrameEnd == HR_BUFFER_SIZE)
					hrBuffer.FrameEnd = 0;

				// advance read pointer if needed
				if (hrBuffer.FrameEnd == hrBuffer.FrameStart)
				{
					++hrBuffer.FrameStart;
					if (hrBuffer.FrameStart == HR_BUFFER_SIZE)
						hrBuffer.FrameStart = 0;
				}
			}

//			// write the updated buffer
//			NLMISC::COFile of(filename);
//			of.serial(hrBuffer);
		}

		// AES send back the result of execution of a command
		virtual void commandResult(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceName, const std::string &result)
		{
			TPendingWebCommands::iterator it(_PendingWebCommands.find(commandId));

			if (it == _PendingWebCommands.end())
			{
				nlwarning("CAdminService::commandResult : AES '%s' returned a command result from service '%s' with command ID %u that is not in pending table",
					sender->getModuleClassName().c_str(),
					serviceName.c_str(),
					commandId);
				return;
			}

			TPendingWebCommand &pwc = it->second;

			CAdminServiceWebItf::commandResult(pwc.Requester, pwc.ServiceAlias, result);

			// erase this command
			_PendingWebCommands.erase(it);
		}

		
		// An AES send it's updated state strings
//		virtual void updateAESStates(NLNET::IModuleProxy *sender, const std::vector < std::string > &states)
//		{
//			nlstop;
//		}

		// AES send back the result of execution of a control command
		virtual void controlCmdResult(NLNET::IModuleProxy *sender, const std::string &serviceName, const std::vector < std::string > &result)
		{
			nlstop;
		}


		///////////////////////////////////////////////////////////////////////
		//// Virtuals from CAdminServiceWebItf
		///////////////////////////////////////////////////////////////////////

		/// Connection callback : a new interface client connect
		virtual void on_CAdminServiceWeb_Connection(NLNET::TSockId from)
		{
		}
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CAdminServiceWeb_Disconnection(NLNET::TSockId from)
		{
		}


		// This is used to issue global commands like 'as.allStart' or 'as.allStop'.
		// The result is returned by the return message
		// serviceCmdResult.
		virtual void on_globalCmd(NLNET::TSockId from, const std::string &command)
		{
			// create a displayer to gather the output of the command
			class CStringDisplayer: public IDisplayer
			{
			public:
				virtual void doDisplay( const CLog::TDisplayInfo& args, const char *message)
				{
					_Data += message;
				}

				std::string _Data;
			};

			nldebug("Global command from web : '%s'", 
				command.c_str());

			// ok, we can execute the command concerning the service.
			CStringDisplayer stringDisplayer;
			IService::getInstance()->CommandLog.addDisplayer(&stringDisplayer);

			// build the command line
			CSString cmdLine;
			cmdLine << getCommandHandlerName() << "." << command;
			// retrieve the command from the input message and execute it
			nlinfo ("ADMIN: Executing global command : '%s'", cmdLine.c_str());
			ICommand::execute (cmdLine, IService::getInstance()->CommandLog);

			// unhook our displayer as it's work is now done
			IService::getInstance()->CommandLog.removeDisplayer(&stringDisplayer);

			// send the result back to the web
			CAdminServiceWebItf::commandResult(from, "", stringDisplayer._Data);
		}

		// Send a service related command to the executor 
		// (not to the controled service)
		// The return value is a string containing the content
		// returned by the command.
		virtual void on_controlCmd(NLNET::TSockId from, const std::string &serviceAlias, const std::string &command)
		{
			// push the request info
			TPendingWebCommand	pwc;
			pwc.ReceptionDate = NLMISC::CTime::getSecondsSince1970();
			pwc.Command = command;
			pwc.ControlCommand = true;
			pwc.Requester = from;
			pwc.ServiceAlias = serviceAlias;

			_PendingWebCommands.insert(make_pair(_NextCommandId,  pwc));

			// send the request to the AES
			sendCommandToAES(_NextCommandId++, pwc);
		}

		// Send a command to the AS.
		// Send a command to a service.
		// The return value is a string containing the content returned by the
		virtual void on_serviceCmd(NLNET::TSockId from, const std::string &serviceAlias, const std::string &command)
		{
			// push the request info
			TPendingWebCommand	pwc;
			pwc.ReceptionDate = NLMISC::CTime::getSecondsSince1970();
			pwc.Command = command;
			pwc.ControlCommand = false;
			pwc.Requester = from;
			pwc.ServiceAlias = serviceAlias;

			_PendingWebCommands.insert(make_pair(_NextCommandId,  pwc));

			// send the request to the AES
			sendCommandToAES(_NextCommandId++, pwc);
		}


		// Get the orders of each known shard.
		// The return value is a vector of string, one entry by shard
		virtual std::vector<std::string> on_getShardOrders(NLNET::TSockId from)
		{
			vector<string> ret;

			TShardsOrders::iterator first(_ShardOrders.begin()), last(_ShardOrders.end());
			for (; first != last; ++first)
			{
				CSString orders;

				orders << "ShardName=" << first->first;
				orders << "\tOrders=" << first->second.toString();

				ret.push_back(orders);
			}
			

			return ret;
		}

		// Get the last known state of all services.
		// The return value is a vector of string, one entry by service
		virtual std::vector<std::string> on_getStates(NLNET::TSockId from)
		{
			uint32 now = NLMISC::CTime::getSecondsSince1970();
			vector<string> ret;
			TAESTracker::TTrackedModules::iterator first(_AESTracker.getTrackedModules().begin()), last(_AESTracker.getTrackedModules().end());
			for (; first != last; ++first)
			{
				IModuleProxy *aes = *first;
				const vector<TServiceStatus>	&status = _KnownServices[*first].ServiceStatus;

				uint32 aesStallDelay = now - _KnownServices[*first].LastReportDate;
				bool aesStall = aesStallDelay > AES_REPORT_WARNING_DELAY;

				for (uint i=0; i<status.size(); ++i)
				{
					CSString state;
					const TServiceStatus &ss = status[i];
					state << "ShardName=" << ss.getShardName();
					state << "\tLongName=" << ss.getServiceLongName();
					state << "\tShortName=" << ss.getServiceShortName();
					state << "\tAliasName=" << ss.getServiceAliasName();
					if (ss.getRunningOrders() != TRunningOrders::invalid_val)
						state << "\tRunningOrders=" << ss.getRunningOrders().toString();
					state << "\tRunningState=" << ss.getRunningState().toString();
					state << "\tRunningTags=";
					set < TRunningTag >::const_iterator frt(ss.getRunningTags().begin()), lrt(ss.getRunningTags().end());
					for (; frt != lrt; ++frt)
					{
						state << frt->toString() << " ";
					}
					state << "\t" << status[i].getStatus();

					if (aesStall)
					{
						state << "\tAESStall=" << toString(aesStallDelay);
					}
					ret.push_back(state);
				}
			}

			return ret;
		}


		// Get information about a high rez graph.
		// The return is a string array containing
		// the name of the var, the available sample
		// period as two unix date (start dans end)
		// and the number of samples available
		// If the var is not found, an empty array is returned
		virtual std::vector<std::string> on_getHighRezGraphInfo(NLNET::TSockId from, const std::string &varAddr)
		{
			vector<string> ret;
			THighRezBuffers::iterator it(_HighRezBuffers.find(varAddr));
			if (it == _HighRezBuffers.end())
			{
				return ret;
			}

			THighRezBuffer &buffer = it->second;

			ret.push_back(varAddr);
			if (buffer.FrameStart == buffer.FrameEnd)
			{
				// the buffer is empty
				ret.push_back(0);
				ret.push_back(0);
				ret.push_back(0);
			}
			else
			{
				ret.push_back(toString(buffer.Datas[buffer.FrameStart].Date));
				ret.push_back(toString(buffer.Datas[(buffer.FrameEnd-1)%HR_BUFFER_SIZE].Date));
				ret.push_back(toString((buffer.FrameEnd - buffer.FrameStart) % HR_BUFFER_SIZE));
			}

			return ret;
		}

		// Get the data for a high resolution graph.
		// The return is a string array, each
		// string containing 'time:milliOffset:value
		virtual std::vector<std::string> on_getHighRezGraph(NLNET::TSockId from, const std::string &varAddr, uint32 startDate, uint32 endDate, uint32 milliStep)
		{
			vector<string> ret;
			THighRezBuffers::iterator it(_HighRezBuffers.find(varAddr));
			if (it == _HighRezBuffers.end())
			{
				return ret;
			}

			THighRezBuffer &buffer = it->second;


			if (buffer.FrameStart == buffer.FrameEnd)
			{
				// the buffer is empty
				return ret;
			}

			if (endDate == 0)
			{
				// the end date is zero, this mean that start date is relative
				// to the last sample date.
				endDate = buffer.Datas[(buffer.FrameEnd-1)%HR_BUFFER_SIZE].Date;
				if (endDate > startDate)
					startDate = endDate - startDate;
				else
					startDate = 0;
			}

			// advance in the buffer until we found a time greater than start date
			uint32 startPointer = buffer.FrameStart;
			while (startPointer != buffer.FrameEnd && buffer.Datas[startPointer].Date < startDate)
			{
				++startPointer;
				if (startPointer == HR_BUFFER_SIZE)
					startPointer = 0;
			}

			if (startPointer == buffer.FrameEnd)
				return ret;

			// retrieve starting times
			uint32 startTime = buffer.Datas[buffer.FrameStart].Date;
			TTime startMilli = buffer.Datas[buffer.FrameStart].SampleTick;
			TTime lastSampleTick = startMilli;
			double minSample = DBL_MAX;
			double maxSample = DBL_MIN;
			double meanSample = 0;
			uint32 nbMergedSample = 0;

			// now collect sample in the result until end of buffer or end date
			for (uint32 i=startPointer; i != buffer.FrameEnd; ++i%=HR_BUFFER_SIZE)
			{

				// check for end of time slice
				if (buffer.Datas[i].Date >= endDate)
				{
					// stop the loop
					break;
				}

				double value = buffer.Datas[i].Value;

				if (value < minSample)
					minSample = value;
				if (value > maxSample)
					maxSample = value;
				meanSample += value;
				++nbMergedSample;

				if (buffer.Datas[i].SampleTick - lastSampleTick > milliStep)
				{
					// output this sample
					uint32 date = startTime + uint32((buffer.Datas[i].SampleTick - startMilli)/1000);
					ret.push_back(toString("%u : %"NL_I64"u : %f %f %f", date, buffer.Datas[i].SampleTick, minSample, meanSample/nbMergedSample, maxSample));
					lastSampleTick = buffer.Datas[i].SampleTick;

					minSample = DBL_MAX;
					maxSample = DBL_MIN;
					meanSample = 0;
					nbMergedSample = 0;
				}
			}

			return ret;
		}


		///////////////////////////////////////////////////////////////////////
		//// commands handlers
		///////////////////////////////////////////////////////////////////////
		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CAdminService, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CAdminService, dump, "Dump a status report to appropriate output logger", "no args")
//			NLMISC_COMMAND_HANDLER_ADD(CAdminService, allStart, "set the state of the controled domain to started", "no args")
//			NLMISC_COMMAND_HANDLER_ADD(CAdminService, allStop, "set the state of the controled domain to stopped", "no args")
//			NLMISC_COMMAND_HANDLER_ADD(CAdminService, startShard, "start a shard in the controled domain", "<shardName>")
//			NLMISC_COMMAND_HANDLER_ADD(CAdminService, stopShard, "stop a shard in the controled domain", "<shardName>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminService, setShardStartMode, "set the autostart mode of a shard", "<shardName> on|off")
			NLMISC_COMMAND_HANDLER_ADD(CAdminService, stopShard, "stop all service of a shard with a programmable timer (can be 0 for immediate shutdown)", "<shardName> <delay (s)>")
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(setShardStartMode)
		{
			if (args.size() != 2)
				return false;

			string shardName = args[0];

			if (_ShardOrders.find(shardName) == _ShardOrders.end())
			{
				log.displayNL("Unknown shard '%s'", shardName.c_str());
				return true;
			}

			TShardOrders shardOrders;
			if (args[1] == "on")
				shardOrders = TShardOrders::so_autostart_on;
			else if (args[1] == "off")
				shardOrders = TShardOrders::so_autostart_off;
			else
			{
				log.displayNL("Invalid option '%s', must be 'on' or 'off'", args[1].c_str());
				return true;
			}

			setShardOrders(shardName, shardOrders);

			return true;
		}



//		NLMISC_CLASS_COMMAND_DECL(startShard)
//		{
//			if (args.size() != 1)
//				return false;
//
//			string shardName = args[0];
//
//			if (_ShardOrders.find(shardName) == _ShardOrders.end())
//			{
//				log.displayNL("Unknown shard '%s'", shardName.c_str());
//				return true;
//			}
//
//			setShardOrders(shardName, TRunningOrders::ro_running);
//
//			return true;
//		}

//		NLMISC_CLASS_COMMAND_DECL(stopShard)
//		{
//			if (args.size() != 1)
//				return false;
//
//			string shardName = args[0];
//
//			if (_ShardOrders.find(shardName) == _ShardOrders.end())
//			{
//				log.displayNL("Unknown shard '%s'", shardName.c_str());
//				return true;
//			}
//
//			setShardOrders(shardName, TRunningOrders::ro_stopped);
//
//			return true;
//		}
//

//		NLMISC_CLASS_COMMAND_DECL(allStart)
//		{
//			if (args.size() != 0)
//				return false;
//
//			setGlobalOrders(TRunningOrders::ro_running);
//
//			return true;
//		}
//
//		NLMISC_CLASS_COMMAND_DECL(allStop)
//		{
//			if (args.size() != 0)
//				return false;
//
//			setGlobalOrders(TRunningOrders::ro_stopped);
//
//			return true;
//		}

		NLMISC_CLASS_COMMAND_DECL(stopShard)
		{
			if (args.size() != 2)
				return false;

			string shardName = args[0];
			uint32 delay;
			NLMISC::fromString(args[1], delay);

			if (_ShardOrders.find(shardName) == _ShardOrders.end())
			{
				log.displayNL("Unknown shard '%s'", shardName.c_str());
				return true;
			}

			// dispatch the request to all AES (they will apply to the pertinent service)
			CAdminExecutorServiceProxy::broadcast_shutdownShard(_AESTracker.getTrackedModules().begin(), _AESTracker.getTrackedModules().end(), 
				this, shardName, delay);

			return true;
		}

		
		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			log.displayNL("===============================");
			log.displayNL(" Dumping Admin states");
			log.displayNL("===============================");

//			log.displayNL("  Global orders is '%s'", _GlobalOrders.toString().c_str());

			log.displayNL("  There are %u known shards :", _ShardOrders.size());
			{
				TShardsOrders::iterator first(_ShardOrders.begin()), last(_ShardOrders.end());
				for (; first != last; ++first)
				{
					log.displayNL("  + Shard '%s' is '%s'", first->first.c_str(), first->second.toString().c_str());
				}
			}
			log.displayNL("  There are %u AES services :", _AESTracker.getTrackedModules().size());
			TAESTracker::TTrackedModules::iterator first(_AESTracker.getTrackedModules().begin()), last(_AESTracker.getTrackedModules().end());
			for (; first != last; ++first)			{
				IModuleProxy *aes = *first;
				const vector<TServiceStatus>	&status = _KnownServices[*first].ServiceStatus;
				log.displayNL("  + AES '%s', with %u connected services", 
					aes->getModuleName().c_str(), 
					status.size());

				for (uint i=0; i<status.size(); ++i)
				{
					const TServiceStatus &ss = status[i];
					log.displayNL("  |  + Service '%s' (%s): state '%s'", 
						ss.getServiceAliasName().c_str(), 
						ss.getServiceShortName().c_str(),
						ss.getStatus().c_str());
					log.display("  |  | Shard '%s' RunningState '%s'", 
						ss.getShardName().c_str(),
						ss.getRunningState().toString().c_str());

					if (ss.getRunningOrders() != TRunningOrders::invalid_val)
						log.display("  RunningOrders '%s'", 
							ss.getRunningOrders().toString().c_str());

					log.displayNL("");
				}
			}

			return true;
		}
	};

	NLNET_REGISTER_MODULE_FACTORY(CAdminService, "AdminService");

} //namespace ADMIN

