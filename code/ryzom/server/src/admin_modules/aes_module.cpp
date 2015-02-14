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
#include "nel/misc/singleton.h"
#include <time.h>
#include "nel/misc/path.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"

#include "game_share/utils.h"

#include "admin_modules_itf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

void aes_forceLink() {}

namespace ADMIN
{
	const char* LAUNCH_CTRL_START = "LAUNCH";
	const char* LAUNCH_CTRL_STOP = "STOP";

	const char *AESPersistentStateFilename = "aes_state.txt";

	/// We want 10 slot (you can change this, but need at least 3 slots)
	const uint32 CRASH_COUNTER_SLOT = 10;
	/// The delay (in second) between slots roll. This value * CRASH_COUNTER_SLOT give the total sampling period
	const uint32 CRASH_COUNTER_ROLL_DELAY = 10*60;	// 10 mn
	/// If we have more than 5 start of a service in the sampling period, we tag the service as 'chain crashing'
	const uint32 CRASH_COUNTER_CHAIN_THRESHOLD = 5;

	/** the name of the file written by the patch man to request a global shutdown
	 *	of all registered the services before switching to a new version.
	 */
	CVariable<string> ShutdownRequestFileName("aes","ShutdownRequestFileName", "name of the file to use for shutdown requests", "./global.launch_ctrl", 0, true);

	/** A kind rolling buffer used to count services start from the runner
	 *	script.
	 */
	class CRunnerLoopCounter
	{
		/// The slot table. Each slot accumulate the service start for a time frame
		uint32	_Slots[CRASH_COUNTER_SLOT];
		/** The last value read from the runner script. This is used to compute 
		 *	the delta value to add to the first slot
		 */
		uint32	_LastValueRead;
		/// The total sum of all slot (could be recomputed on demand, but a little more efficient)
		uint32	_CounterSum;
	public:

		CRunnerLoopCounter()
		{
			// we need at least 3 slots
			nlctassert(CRASH_COUNTER_SLOT >= 3);

			// init all slots with 0
			for (uint i=0; i<CRASH_COUNTER_SLOT; ++i)
			{
				_Slots[i] = 0;
			}

			// init the last value with a magic value so that the first
			// update will not compute a delta but only take
			// the first value as initial reference
			_LastValueRead = 0xffffffff;
			_CounterSum = 0;
		}

		/** Updat the counter by submitting the current start counter
		 *	written by the runner script.
		 *	Note that the runner script only increment the counter
		 *	so we need to compute the delta from _LastValueRead
		 *	before accumulating in the first slot.
		 */
		void updateCounter(uint32 lastValue)
		{
			if (_LastValueRead == 0xffffffff || lastValue < _LastValueRead)
			{
				// this is the first sample, just init the last value read
				// or the counter have been reset to a smaller value
				_LastValueRead = lastValue;
			}
			else
			{
				// not the first sample, compute the delta and accumulate
				uint32 delta = lastValue - _LastValueRead;
				_Slots[0] += delta;
				_LastValueRead = lastValue;
				// update summ
				_CounterSum += delta;
			}
		}

		/// Roll the slots. The last slot is ejected and
		/// each slot are copied in the next one (in
		/// inverse order obviously)
		/// The first slot in then set to 0
		void rollCounter()
		{

			_CounterSum -= _Slots[CRASH_COUNTER_SLOT-1];

			for (uint i=CRASH_COUNTER_SLOT-1; i>0; --i)
			{
				_Slots[i] = _Slots[i-1];
			}
			_Slots[0] = 0;
		}

		/// Return the sum of all the slots
		uint32 getSum()
		{
			return _CounterSum;
		}

		/// Return the sum of the first slot, the tree first slot and
		/// the total of all slots.
		/// This is useful to understand the behavoir of a crashing
		/// service over the sampling period.
		void getCounters(uint32 &oneSlot, uint32 &treeSlots, uint32 &allSlots)
		{
			oneSlot = _Slots[0];
			treeSlots = _Slots[0]+_Slots[1]+_Slots[2];
			allSlots = _CounterSum;
		}


		/// Reset all counter to zero
		void resetCounter()
		{
			for (uint i=0; i<CRASH_COUNTER_SLOT; ++i)
			{
				_Slots[i] = 0;
			}
			_CounterSum = 0;
		}

	};


	class CAdminExecutorService 
		:	/*public CManualSingleton<CAdminExecutorService>,*/
			public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
			public CAdminExecutorServiceSkel,
			public IModuleTrackerCb
	{
	public:
		enum 
		{
			SLOW_TO_START_THRESHOLD = 60,		// 1 mn
			SLOW_TO_STOP_THRESHOLD = 60,		// 1 mn
			_NagiosReportDelay = 60,			// 1 mn
		};

	private:
	
		typedef CModuleTracker<TModuleClassPred>	TServiceTracker;
		// tracker for admin executor client modules
		TServiceTracker		_ServiceTracker;

		/// Admin service module
		TModuleProxyPtr		_AdminService;

		/// Date of last state reporting to AS
		uint32				_LastStateReport;

		/// Date of last nagios report output
		uint32				_LastNagiosReport;

		typedef string	TAliasName;
		typedef string	TShardName;
		typedef set<TAliasName>	TRegisteredServices;
		/// List of 'registered service', ie. those that are configured in aes cfg.
		TRegisteredServices		_RegisteredServices;

		/// A set of data for each registered or connected service
		struct TServiceState
		{
			string			ShardName;	
			bool			DontUseShardOrders;
			TRunningState	RunningState;
			set<TRunningTag> RunningTags;	
			string			LongName;
			string			ShortName;
			uint32			PID;
			string			State;
			uint32			LastStateDate;
			uint32			StopRequestDate;	
			uint32			StartRequestDate;
			TModuleProxyPtr	ServiceModule;
			CRunnerLoopCounter	RunnerLoopCounter;

			TServiceState()
				:	DontUseShardOrders(false),
					RunningState(TRunningState::rs_stopped),
					PID(0),
					LastStateDate(0),
					StopRequestDate(0),
					StartRequestDate(0)
			{}
		};

		typedef map<TAliasName, TServiceState>	TServiceStates;
		/// States for each connected or registered service
		TServiceStates				_ServiceStates;

		typedef map<TModuleProxyPtr, TAliasName>	TConnectedServiceIndex;
		/// Index of connected service proxy to alias name
		TConnectedServiceIndex		_ConnectedServiceIndex;

		typedef map<TAliasName, TRunningOrders>	TPersistentServiceOrders;
		/// Persistent service state, i.e state that are restored after a stop/start of the aes
		TPersistentServiceOrders	_PersistentServiceOrders;
		
		typedef map<TShardName, TShardOrders>	TShardsOrders;
		/// Shard orders (set by AS)
		TShardsOrders				_ShardOrders;

		/// flag for shutdown request form patch manager.
		bool						_ShutdownForPatch;

		/// A flag that mean we need to save the persistent state file
		bool						_NeedToWriteStateFile;

		/// Date of last roll of the runner loop counters
		uint32						_LastRunnerLoopCounterRoll;

		/// Data for each command pending result from a service
		struct TPendingWebCommand
		{
			/// Date of reception of the command for timeout
			uint32	ReceptionDate;
			/// Name of the target service
			string	ServiceAlias;
			/// Command
			string	Command;
		};
		typedef uint32	TCommandId;
		typedef map<TCommandId, TPendingWebCommand>	TPendingWebCommands;
		/// A list of pending command sent to service and waiting result
		TPendingWebCommands	_PendingWebCommands;

		/// information about shard being stopped
		struct TStopingShardInfo
		{
			/// Name of the shard to stop
			string		ShardName;
			/// Delay before stop
			uint32		Delay;
			/// Begin date of countdown
			uint32		BeginDate;
		};

		typedef vector<TStopingShardInfo>	TStopingShardInfos;

		/// The vector of shard to stop.
		TStopingShardInfos	_StopingShards;


	public:
		CAdminExecutorService()
			:	_ServiceTracker(TModuleClassPred("AdminExecutorServiceClient")),
				_LastStateReport(0),
				_LastNagiosReport(0),
				_ShutdownForPatch(false),
				_NeedToWriteStateFile(false),
				_LastRunnerLoopCounterRoll(0)
		{
			CAdminExecutorServiceSkel::init(this);
			_ServiceTracker.init(this, this);

		}

		bool initModule(const TParsedCommandLine &pcl)
		{
			CModuleBase::initModule(pcl);

			// read the persistent state file if any
			string filename = CPath::standardizePath(IService::getInstance()->SaveFilesDirectory.toString(), true)+AESPersistentStateFilename;
			FILE *fp = fopen(filename.c_str(), "rt");
			if (fp != NULL)
			{
				char buffer[1024];
				char *ret;
				while ((ret=fgets(buffer, 1024, fp)) != NULL)
				{
					CSString line(buffer);
					CSString cmd(line.firstWord(true));

					if (cmd == "ServiceState")
					{
						CSString serviceAlias = line.firstWord(true);
						CSString serviceOrders = line.firstWord(true);

						TRunningOrders runningOrders(serviceOrders);
						if (!serviceAlias.empty() && runningOrders != TRunningOrders::invalid_val)
						{
							// add this one in the list of persistent state
							_PersistentServiceOrders[serviceAlias] = runningOrders;
						}
					}
					else if (cmd == "ShardOrders")
					{
						string shardName(line.firstWord(true));
						TShardOrders shardOrders(line.firstWord(true));
						if (shardOrders != TShardOrders::invalid_val)
							_ShardOrders[shardName] = shardOrders;
					}
				}
				// clear the flag because 'setGlobalState' has set it
				_NeedToWriteStateFile = false;

				fclose(fp);
			}

			return true;
		}

		void onModuleUp(IModuleProxy *proxy)
		{
			if (proxy->getModuleClassName() == "AdminService")
			{
				nldebug("CAdminExecutorService : admin service up as '%s'", proxy->getModuleName().c_str());
				// we found the manager of AES
				if (_AdminService != NULL)
				{
					nlwarning("CAdminExecutorService : admin service already known as '%s', replacing with new one", _AdminService->getModuleName().c_str());
				}
				_AdminService = proxy;

				// cleanup the persistent service state by removing any state not in registered or connected service
				{
					set<string>	removeList;

					// first, fill the list with all the persistent state service name
					{
						TPersistentServiceOrders::iterator first(_PersistentServiceOrders.begin()), last(_PersistentServiceOrders.end());
						for (; first != last; ++first)
						{
							removeList.insert(first->first);
						}
					}

					// remove the registered service from the removelist
					{
						TRegisteredServices::iterator first(_RegisteredServices.begin()), last(_RegisteredServices.end());
						for (; first != last; ++first)
						{
							removeList.erase(*first);
						}
					}
					// remove any connected service (even unregistered one)
					{
						TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
						for (; first != last; ++first)
						{
							removeList.erase(first->first);
						}
					}

					// no remove persistent state that left in the remove list
					while (!removeList.empty())
					{
						_PersistentServiceOrders.erase(*(removeList.begin()));

						_NeedToWriteStateFile = true;

						removeList.erase(removeList.begin());
					}
					
				}
				// send the current status
				sendUpServiceUpdate();
			}

			uint32 now = NLMISC::CTime::getSecondsSince1970();
			// check pending command timeout
			TPendingWebCommands::iterator first(_PendingWebCommands.begin()), last(_PendingWebCommands.end());
			for (; first != last; ++first)
			{
				TPendingWebCommand &pwc = first->second;

				if (now - pwc.ReceptionDate > 10)
				{
					// timeout
					if (_AdminService != NULL)
					{
						CAdminServiceProxy as(_AdminService);
						as.commandResult(this, first->first, pwc.ServiceAlias, "ERROR : AES : no reponse from service");
					}

					_PendingWebCommands.erase(first);

					// check other pending commands at next update
					break;
				}
			}
		}

		void onModuleDown(IModuleProxy *proxy)
		{
			if (proxy == _AdminService)
			{
				nldebug("CAdminExecutorService : admin service '%s' is down", proxy->getModuleName().c_str());

				_AdminService = NULL;
			}
		}

		void onModuleUpdate()
		{
			H_AUTO(CAdminExecutorService_onModuleUpdate);

			uint32 now = CTime::getSecondsSince1970();

			if (_LastStateReport < now)
			{
				// every second

				// check services every second
				TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
				for (; first != last; ++first)
				{
					string aliasName = first->first;
					TServiceState &ss = first->second;
					if (_RegisteredServices.find(aliasName) != _RegisteredServices.end())
					{
						// this is a registered service, we need to control is running state

						// read the actual running state from the runner script written file
						if (getOfflineServiceState(aliasName) == "RUNNING")
						{
							// the service is running
							ss.RunningTags.erase(TRunningTag::rt_locally_stopped);
							ss.RunningTags.erase(TRunningTag::rt_globally_stopped);
							ss.RunningTags.erase(TRunningTag::rt_stopped_for_patch);

							if (ss.StopRequestDate != 0)
							{
								// still not stopped, check for slow to stop service
								if (now - ss.StopRequestDate > SLOW_TO_STOP_THRESHOLD)
								{
									// add a running tag
									ss.RunningTags.insert(TRunningTag::rt_slow_to_stop);
								}
							}

							if (ss.RunningState != TRunningState::rs_online)
							{
								// tag slow to start service
								if (now - ss.StartRequestDate > SLOW_TO_START_THRESHOLD)
								{
									// add a running tag
									ss.RunningTags.insert(TRunningTag::rt_slow_to_start);
								}
								else
								{
									ss.RunningState = TRunningState::rs_running;
								}
							}
						}
						else
						{
							// the service is stopped
							ss.RunningState = TRunningState::rs_stopped;
							ss.RunningTags.erase(TRunningTag::rt_locally_started);
							ss.RunningTags.erase(TRunningTag::rt_externaly_started);
							ss.RunningTags.erase(TRunningTag::rt_slow_to_stop);

							// clean the stop request date
							ss.StopRequestDate = 0;
						}

						// try to obtains service orders from its shard
						TShardOrders shardOrders(TShardOrders::so_autostart_on);
						if (_ShardOrders.find(ss.ShardName) != _ShardOrders.end())
						{
							shardOrders = _ShardOrders[ss.ShardName];
						}
						// little check, the service must have a entry in the service orders container.
						nlassert(_PersistentServiceOrders.find(aliasName) != _PersistentServiceOrders.end());

						TRunningOrders serviceOrders = _PersistentServiceOrders[aliasName];

						// look if service need to be started
						if (ss.RunningState == TRunningState::rs_stopped		// its stopped
							&& serviceOrders == TRunningOrders::ro_activated	// and activated
							&& shardOrders == TShardOrders::so_autostart_on		// and shard is autostart on
							&& !ss.DontUseShardOrders							// and this service follow its shard orders
							&& !_ShutdownForPatch								// and no patch pending
							)
						{
							// we must start this service !
							startService(aliasName);
						}

						// look for service that need to be stopped
						if (ss.RunningState != TRunningState::rs_stopped		// its not stopped
							&& (serviceOrders == TRunningOrders::ro_deactivated	// and deactivated
								|| _ShutdownForPatch)							// or patch pending
							&& ss.StopRequestDate == 0								// no stop request send
							)
						{
							// we must stop this service
							stopService(aliasName);

							// store the sop
							ss.StopRequestDate = now;
						}
						// shuted down for patch ?
						if (_ShutdownForPatch)
							ss.RunningTags.insert(TRunningTag::rt_stopped_for_patch);
						else
							ss.RunningTags.erase(TRunningTag::rt_stopped_for_patch);

						// chain crashing ?
						if (ss.RunnerLoopCounter.getSum() > CRASH_COUNTER_CHAIN_THRESHOLD)
							ss.RunningTags.insert(TRunningTag::rt_chain_crashing);
						else
							ss.RunningTags.erase(TRunningTag::rt_chain_crashing);

						// update the crash counter
						ss.RunnerLoopCounter.updateCounter(getServiceStartLoopCounter(aliasName));
					}
				


				}

				// if we have an admin service connected, send it an update of service state
				if (_AdminService != NULL)
					sendUpServiceUpdate();


				if ((now & 0xf) == 0)
				{
					// every 8 seconds for low frequency work

					// check for shutdown request from patchman
					checkShutdownRequest();
				}

				// check for shard to stop (and warning to send)
				checkServiceToStop();

				// time to output the nagios report ?
				if (now > _LastNagiosReport+_NagiosReportDelay)
				{
					// write the nagios report
					FILE *fp = fopen("aes_nagios_report.txt", "wt");
					if (fp != NULL)
					{
						// output the current date
						time_t t = now;
						fprintf(fp, "AESReportDate=%s", ::ctime(&t));

						fprintf(fp, "NBService=%u\n", (uint)_ServiceStates.size());
						// output state of each service
						TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
						for (; first != last; ++first)
						{
							CSString serviceLine;
							TServiceState &ss = first->second;
							const string &aliasName = first->first;

							CSString runningTags;
							set<TRunningTag>::iterator rtf(ss.RunningTags.begin()), rte(ss.RunningTags.end());
							for (; rtf != rte; ++rtf)
							{
								runningTags<<"<"<<rtf->toString()<<">";
							}

							bool registered = _RegisteredServices.find(aliasName) != _RegisteredServices.end();

							serviceLine << "ServiceAlias='"<<aliasName<<"' RunningState='"<<ss.RunningState.toString()<<"' RunningTag='"<<runningTags<<"'";
							serviceLine << " NoReportSince="<<now-ss.LastStateDate;
							serviceLine << " State='"<<ss.State<<"'";

							fprintf(fp, "%s\n", serviceLine.c_str());
						}


						fclose(fp);
					}
					else
					{
						nlwarning("Can't open the nagios report file !");
					}

					_LastNagiosReport = now;
				}

					
				// update the last report date
				_LastStateReport = now;
			}

			// check runner loop counter roll timer
			if (_LastRunnerLoopCounterRoll+CRASH_COUNTER_ROLL_DELAY < now)
			{
				// it's time to roll the crash counter
				TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
				for (; first != last; ++first)
				{
					first->second.RunnerLoopCounter.rollCounter();
				}

				_LastRunnerLoopCounterRoll = now;
			}

			if (_NeedToWriteStateFile)
			{
				/// The persistent service orders need to be saved
				string filename = CPath::standardizePath(IService::getInstance()->SaveFilesDirectory.toString(), true)+AESPersistentStateFilename;
				FILE *fp = fopen(filename.c_str(), "wt");
				if (fp != NULL)
				{
					{
						CSString line;
						TShardsOrders::iterator first(_ShardOrders.begin()),  last(_ShardOrders.end());
						for (; first != last; ++first)
						{
							line << "ShardOrders "<<first->first<<" "<<first->second.toString()<<"\n";
						}
						
						fputs(line.c_str(), fp);
					}

					{
						TPersistentServiceOrders::iterator first(_PersistentServiceOrders.begin()), last(_PersistentServiceOrders.end());
						for (; first != last; ++first)
						{
							CSString line;
							line << "ServiceState "<<first->first<<" "<<first->second.toString()<<"\n";
							fputs(line.c_str(), fp);
						}
					}
					// clear the flag because 'setGlobalState' has set it
					_NeedToWriteStateFile = false;

					fclose(fp);
				}
			}
		}

		void sendUpServiceUpdate()
		{
			if (_AdminService != NULL)
			{
				vector<TServiceStatus>	serviceStatus;
				set<TAliasName>	missingServices = _RegisteredServices;
				// send an updated list to AES
				TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
				for (; first != last; ++first)
				{
					const string &aliasName = first->first;
					bool registered = _RegisteredServices.find(aliasName) != _RegisteredServices.end();
					TServiceState &ss = first->second;
					serviceStatus.push_back(TServiceStatus());
					TServiceStatus &ts = serviceStatus.back();
					ts.setShardName(ss.ShardName);
					ts.setServiceLongName(ss.LongName);
					ts.setServiceShortName(ss.ShortName);
					ts.setServiceAliasName(aliasName);
					ts.setRunningState(ss.RunningState);
					if (registered)
						ts.setRunningOrders(_PersistentServiceOrders[aliasName]);
					else
						ts.setRunningOrders(TRunningOrders::invalid_val);
					ts.setRunningTags(ss.RunningTags);
					CSString state;
					state << ss.State << "\tNoReportSince=" << (NLMISC::CTime::getSecondsSince1970()-ss.LastStateDate);

					// add the host name
					state << "\tHostname=" << IService::getInstance()->getHostName();

					if (registered)
					{
						// this is a registered service, send the start counter
						uint32 oneSlot, treeSlots, allSlots;
						ss.RunnerLoopCounter.getCounters(oneSlot, treeSlots, allSlots);
						state << "\tStartCounter="<<oneSlot<<" "<<treeSlots<<" "<<allSlots;
					}
					ts.setStatus(state);

					missingServices.erase(aliasName);
				}

				CAdminServiceProxy as(_AdminService);
				as.upServiceUpdate(this, serviceStatus);
			}
		}

		IModuleProxy *findOnlineService(const std::string &serviceAlias)
		{
			TConnectedServiceIndex::iterator first(_ConnectedServiceIndex.begin()), last(_ConnectedServiceIndex.end());
			for (; first != last; ++first)
			{
				if (first->second == serviceAlias)
				{
					// ok, we found it
					return first->first;
				}
			}

			// not found
			return NULL;
		}

		void checkShutdownRequest()
		{
			// if there's no ctrl file to be found then giveup
			if (!NLMISC::CFile::fileExists(ShutdownRequestFileName)) return;

			// if a shutdown ctrl file exists then read it's contents (if the file doesn't exist this returns an empty string)
			CSString fileContents;
			fileContents.readFromFile(ShutdownRequestFileName.c_str());

			// see if the file exists
			if (!fileContents.empty())
			{
				NLMISC::CFile::deleteFile(ShutdownRequestFileName);
				fileContents= fileContents.strip().splitToOneOfSeparators(" \t\n\r\x1a");

				NLMISC::fromString(fileContents, _ShutdownForPatch);
				_ShutdownForPatch = !_ShutdownForPatch;
			}
		}

		void checkServiceToStop()
		{
			uint32 now = CTime::getSecondsSince1970();
			// for each shard to stop
			for (uint i=0; i<_StopingShards.size(); ++i)
			{
				const TStopingShardInfo &stopShardInfo = _StopingShards[i];
				
				bool timeToStop = stopShardInfo.BeginDate + stopShardInfo.Delay <= now;
				uint32 timeLeft = (stopShardInfo.BeginDate + stopShardInfo.Delay) - now;
				// check every service
				TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
				for (; first != last; ++first)
				{
					TServiceState &serviceState = first->second;

					if (serviceState.ServiceModule != NULL &&  serviceState.ShardName == stopShardInfo.ShardName)
					{
						// this one belong to the shard to stop
						if (!timeToStop)
						{
							// send a warning every 30 s
							if (((now - stopShardInfo.BeginDate) % 30) == 0)
							{
								CAdminExecutorServiceClientProxy aec(serviceState.ServiceModule);
								nlinfo("Sending command 'quitDelay' to service '%s'", first->first.c_str());
								aec.serviceCmdNoReturn(this, toString("quitDelay %u", timeLeft));
							}
						}
						else
						{
							// stop the service
							stopService(first->first);
						}
					}
				}

				if (timeToStop)
				{
					nlinfo("All local service for shard %s are stopped", stopShardInfo.ShardName.c_str());
					// shard stopped, erase this entry 
					_StopingShards.erase(_StopingShards.begin()+i);
					--i;
				}
			}
		}


		// the following routine reads the text string contained in the ".state" file for this service
		// it's used to provide a 'state' value for services that are registered but are not connected
		// to give info on whether they've been launched, whether their launcher is online, etc
		std::string getOfflineServiceState(const std::string& serviceAlias)
		{
			// open the file for reading
			FILE* f= fopen(getServiceStateFileName(serviceAlias).c_str(),"rt");
			if (f==NULL) return "STOPPED";

			// setup a buffer to hold the text read from the file
			uint32 fileSize= NLMISC::CFile::getFileSize(f);
			std::string txt;
			txt.resize(fileSize);

			// read the text from the file - note that the number of bytes read may be less than the
			// number of bytes requested because we've opened the file in text mode and not binary mode
			uint32 bytesRead= (uint32)fread(&txt[0],1,fileSize,f);
			txt.resize(bytesRead);
			fclose(f);

			// return the text read from the file
			return txt;
		}

		
		// the following routine reads the text string contained in the "pid.state" file for this service
		// it's used to provide a early pid information to the AES (before the service is connected)
		uint32 getOfflineServicePID(const std::string& serviceAlias)
		{
			// open the file for reading
			FILE* f= fopen(getServicePIDFileName(serviceAlias).c_str(),"rt");
			if (f==NULL) return 0;

			// setup a buffer to hold the text read from the file
			uint32 fileSize= NLMISC::CFile::getFileSize(f);
			std::string txt;
			txt.resize(fileSize);

			// read the text from the file - note that the number of bytes read may be less than the
			// number of bytes requested because we've opened the file in text mode and not binary mode
			uint32 bytesRead= (uint32)fread(&txt[0],1,fileSize,f);
			txt.resize(bytesRead);
			fclose(f);

			// return the pid read from the file
			uint32 pid;
			NLMISC::fromString(txt, pid);

			return pid;
		}


		// the following routine reads the text string contained in the ".start_counter" file for this service
		// it's used to provide the number of start done by the runner loop on the service
		// This is used for the chain crash detector system.
		uint32 getServiceStartLoopCounter(const std::string& serviceAlias)
		{
			// open the file for reading
			FILE* f= fopen(getServiceLoopCounterFileName(serviceAlias).c_str(),"rt");
			if (f==NULL) 
				return 0;

			// setup a buffer to hold the text read from the file
			uint32 fileSize= NLMISC::CFile::getFileSize(f);
			std::string txt;
			txt.resize(fileSize);

			// read the text from the file - note that the number of bytes read may be less than the
			// number of bytes requested because we've opened the file in text mode and not binary mode
			uint32 bytesRead= (uint32)fread(&txt[0],1,fileSize,f);
			txt.resize(bytesRead);
			fclose(f);

			// parse the text in the buffer
			uint32 counter;
			NLMISC::fromString(txt, counter);

			return counter;
		}

		// retrieve service launch info in the config file
		bool getServiceLaunchInfo(const string& serviceAlias, string& path)
		{
			string basePath;
			CConfigFile::CVar *launchDir = IService::getInstance()->ConfigFile.getVarPtr("AESLauncherDir");
			if (launchDir != NULL)
			{
				basePath = launchDir->asString()+"/";
			}

			if (_RegisteredServices.find(serviceAlias) == _RegisteredServices.end())
				return false;
			path = basePath + serviceAlias+"/";

			return true;
		}

		
		std::string getServiceStateFileName(const std::string& serviceAlias)
		{
			string servicePath;
			if (getServiceLaunchInfo(serviceAlias, servicePath))
				return NLMISC::CPath::standardizePath(servicePath)+serviceAlias+".state";
			else
				return string();
		}

		std::string getServicePIDFileName(const std::string& serviceAlias)
		{
			string servicePath;
			if (getServiceLaunchInfo(serviceAlias, servicePath))
				return NLMISC::CPath::standardizePath(servicePath)+"pid.state";
			else
				return string();
		}

		std::string getServiceLoopCounterFileName(const std::string& serviceAlias)
		{
			string servicePath;
			if (getServiceLaunchInfo(serviceAlias, servicePath))
				return NLMISC::CPath::standardizePath(servicePath)+serviceAlias+".start_count";
			else
				return string();
		}

		std::string getServiceLaunchCtrlFileName(const std::string& serviceAlias,const std::string& serviceExecutionPath, bool deferred)
		{
			return NLMISC::CPath::standardizePath(serviceExecutionPath)+serviceAlias+(deferred?".deferred_":".")+"launch_ctrl";
		}


		bool writeServiceLaunchCtrl(const std::string& serviceAlias, bool deferred, const std::string& txt)
		{
			string path;
			if (!getServiceLaunchInfo(serviceAlias, path))
				return false;

			// make sure the path exists
			NLMISC::CFile::createDirectoryTree(path);

			// open the file for writing
			FILE* f= fopen(getServiceLaunchCtrlFileName(serviceAlias, path, deferred).c_str(),"wt");
			if (f==NULL) return false;

			// write the text to the file
			fprintf(f,"%s",txt.c_str());
			fclose(f);

			return true;
		}

		bool startService(const std::string &serviceAlias)
		{
			if (_ServiceStates.find(serviceAlias) != _ServiceStates.end())
			{
				TServiceState &ss = _ServiceStates[serviceAlias];
				if (ss.RunningState != TRunningState::rs_stopped)
				{
					nlwarning("startService '%s' : the service is already running", serviceAlias.c_str());
					return false;
				}

				// store the start date
				ss.StartRequestDate = CTime::getSecondsSince1970();

			}
			if (_RegisteredServices.find(serviceAlias) == _RegisteredServices.end())
			{
				nlwarning("startService '%s' : the service in not registered, can't start it", serviceAlias.c_str());
				return false;
			}

			// write the start command
			bool ret = writeServiceLaunchCtrl(serviceAlias, false, LAUNCH_CTRL_START);

			return ret;
		}

		bool stopService(const std::string &serviceAlias)
		{
			// check that the service is running
			TServiceStates::iterator it(_ServiceStates.find(serviceAlias));
			if (it == _ServiceStates.end())
			{
				nlwarning("stopService : Failed to found service '%s' in the list of services", serviceAlias.c_str());
				return false;
			}

			TServiceState &ss = it->second;
			// write the launch control to stop
			if (_RegisteredServices.find(serviceAlias) != _RegisteredServices.end())
			{
				if (!writeServiceLaunchCtrl(serviceAlias, false, LAUNCH_CTRL_STOP))
				{
					nlwarning("Failed to write launch control file for service '%s'", serviceAlias.c_str());
					return false;
				}
				else
					nlinfo("Service '%s' launch control file updated", serviceAlias.c_str());
			}

			// set the stopre request date if needed
			if (ss.StopRequestDate != 0)
			{
				ss.StopRequestDate = CTime::getSecondsSince1970();
			}

			if (ss.ServiceModule == NULL)
			{
				nlwarning("stopService : The service '%s' is not connected, can't ask him to stop", serviceAlias.c_str());
				return false;
			}

			// send the "quit" command to the service
			CAdminExecutorServiceClientProxy aec(ss.ServiceModule);
			nlinfo("Sending command 'quit' to service '%s'", serviceAlias.c_str());
			aec.serviceCmdNoReturn(this, "quit");

			return true;
		}


		///////////////////////////////////////////////////////////////////////
		//// Virtuals from IModuleTrackerCb
		///////////////////////////////////////////////////////////////////////

		virtual void onTrackedModuleUp(IModuleProxy *moduleProxy)
		{
			nldebug("Service module '%s' UP", moduleProxy->getModuleName().c_str());


			TParsedCommandLine pcl;
			if (!pcl.parseParamList(moduleProxy->getModuleManifest()))
			{
				nlwarning("CAdminExecutorService:onTrackedModuleUp : failed to parse manifest");
			}

			const TParsedCommandLine *pclLongName = pcl.getParam("LongName");
			const TParsedCommandLine *pclShortName = pcl.getParam("ShortName");
			const TParsedCommandLine *pclAliasName = pcl.getParam("AliasName");
			const TParsedCommandLine *pclPID = pcl.getParam("PID");
			const TParsedCommandLine *pclDontUseShardOrders = pcl.getParam("DontUseShardOrders");

			string aliasName = pclAliasName != NULL ? pclAliasName->ParamValue : moduleProxy->getModuleName();

			// remove the temporary state and update connected service index
			_ServiceStates.erase(moduleProxy->getModuleName());
			_ConnectedServiceIndex[moduleProxy] = aliasName;

			nlinfo("AES client module %s for service %s is up",
				moduleProxy->getModuleName().c_str(),
				aliasName.c_str());

			// create a new entry or get an existing one
			TServiceState &ss = _ServiceStates[aliasName];
			// update the service state
			ss.RunningState = TRunningState::rs_online;
			if (pclDontUseShardOrders)
				NLMISC::fromString(pclDontUseShardOrders->ParamValue, ss.DontUseShardOrders);
			else
				ss.DontUseShardOrders = false;
			ss.LongName = pclLongName != NULL ? pclLongName->ParamValue : "unknown";
			ss.ShortName = pclShortName != NULL ? pclShortName->ParamValue : "unknown";

			if (pclPID!= NULL)
			{
				NLMISC::fromString(pclPID->ParamValue, ss.PID);
			}
			else
			{
				ss.PID = 0;
			}

			ss.State = "";
			ss.LastStateDate = NLMISC::CTime::getSecondsSince1970();
			ss.ServiceModule = moduleProxy;
			ss.StartRequestDate = 0;
			ss.RunningTags.erase(TRunningTag::rt_slow_to_start);
			if (_RegisteredServices.find(aliasName) == _RegisteredServices.end())
			{
				ss.RunningTags.insert(TRunningTag::rt_externaly_started);
			}
//			else
//			{
//				// if this service is locally stopped or if the shard it belong to is stopped,
//				// then flag it as 'localy started'
//				if (_PersistentServiceOrders.find(aliasName) != _PersistentServiceOrders.end()
//					&& _PersistentServiceOrders[aliasName] == TRunningOrders::ro_stopped)
//				{
//					// flag it as started
//					_PersistentServiceOrders[aliasName] = TRunningOrders::ro_running;
//					ss.RunningTags.insert(TRunningTag::rt_locally_started);
//					_NeedToWriteStateFile = true;
//				}
//				else if (_ShardOrders.find(ss.ShardName) != _ShardOrders.end() 
//					&& _ShardOrders[ss.ShardName] == TRunningOrders::ro_stopped)
//				{
//					// the shard is stopped, flag the service as started
//					_PersistentServiceOrders[aliasName] = TRunningOrders::ro_running;
//					ss.RunningTags.insert(TRunningTag::rt_locally_started);
//					_NeedToWriteStateFile = true;
//				}
//			}

			sendUpServiceUpdate();			
		}
		virtual void onTrackedModuleDown(IModuleProxy *moduleProxy)
		{
			nldebug("Service module '%s' DOWN", moduleProxy->getModuleName().c_str());

			TConnectedServiceIndex::iterator it(_ConnectedServiceIndex.find(moduleProxy));
			if (it != _ConnectedServiceIndex.end())
			{
				string &aliasName = it->second;
				nlinfo("AES client module %s of service %s is down",
					moduleProxy->getModuleName().c_str(),
					aliasName.c_str());
				BOMB_IF(_ServiceStates.find(aliasName) == _ServiceStates.end(), "Service down from "<<moduleProxy->getModuleName()<<" with alias "<<aliasName<<" not found in _ServiceStates table", _ConnectedServiceIndex.erase(it); return);
				if (_RegisteredServices.find(aliasName) == _RegisteredServices.end())
				{
					// this is not a registered service, remove the status record
					_ServiceStates.erase(aliasName);
				}
				else
				{
					TServiceState &ss = _ServiceStates[aliasName];
					// update the running state
					ss.RunningState = TRunningState::rs_running;

					ss.ServiceModule = NULL;

					// kill the service to be sure that it is really dead !
					if (ss.PID > 1)
					{
						nlinfo("Killing process %u (%s) because aes client module '%s' is down",
							ss.PID,
							aliasName.c_str(),
							moduleProxy->getModuleName().c_str());
						killProgram(ss.PID);
					}
				}

retry_pending_command_loop:
				// check for pending command
				TPendingWebCommands::iterator first(_PendingWebCommands.begin()), last(_PendingWebCommands.end());
				for (; first != last; ++first)
				{
					TPendingWebCommand &pwc = first->second;
					if (pwc.ServiceAlias == aliasName)
					{
						if (_AdminService != NULL)
						{
							CAdminServiceProxy as(_AdminService);
							as.commandResult(this, first->first, pwc.ServiceAlias, "ERROR : AES : service connection lost during command");
						}

						_PendingWebCommands.erase(first);
						// goto to avoid iterator dodging
						goto retry_pending_command_loop;

					}
				}

				// remove the index record
				_ConnectedServiceIndex.erase(it);
			}
			else
			{
				nlinfo("AES client module %s is not associated with a service",
					moduleProxy->getModuleName().c_str());
			}


			sendUpServiceUpdate();			
		}

		///////////////////////////////////////////////////////////////////////
		//// Virtuals from CAdminExecutorServiceSkel
		///////////////////////////////////////////////////////////////////////

		// AS send orders for a shard
		virtual void setShardOrders(NLNET::IModuleProxy *sender, const std::string &shardName, const TShardOrders &shardOrders)
		{
			nlinfo("AS setShardOrders for shard '%s' to '%s'", shardName.c_str(), shardOrders.toString().c_str());

			if (_ShardOrders[shardName] == shardOrders)
			{
				// nothing to do
				return;
			}
			_ShardOrders[shardName] = shardOrders;
			_NeedToWriteStateFile = true;

			// nothing more to do, if service need to be started, they are started
			// by the module update function

		}

		// AS send a command to shutdown a shard with a delay
		virtual void shutdownShard(NLNET::IModuleProxy *sender, const std::string &shardName, uint32 delay)
		{
			TStopingShardInfo ssi;
			ssi.ShardName = shardName;
			ssi.Delay = delay;
			ssi.BeginDate = CTime::getSecondsSince1970();

			_StopingShards.push_back(ssi);

			nlinfo("Received command to stop all service of shard %s in %us", ssi.ShardName.c_str(), ssi.Delay);

			// force a first update (to send the first warning message or stop immediately)
			checkServiceToStop();

		}

		// AS send a control command to this AES
		virtual void controlCmd(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command)
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

			nldebug("Control command from '%s' : '%s' '%s'", 
				sender->getModuleName().c_str(),
				serviceAlias.c_str(),
				command.c_str());

			// look in the list of service for a matching one
			IModuleProxy *service = findOnlineService(serviceAlias);
			if (service == NULL && _RegisteredServices.find(serviceAlias) == _RegisteredServices.end())
			{
				CAdminServiceProxy as(sender);
				as.commandResult(this, commandId, serviceAlias, "ERROR : AES : service not found will dispatching the control command");
				return;
			}
			
			// ok, we can execute the command concerning the service.
			CStringDisplayer stringDisplayer;
			IService::getInstance()->CommandLog.addDisplayer(&stringDisplayer);

			// build the command line
			CSString args(command);
			CSString cmdName = args.firstWord(true);
			CSString cmdLine;
			cmdLine << getCommandHandlerName() << "." << cmdName  << " " << serviceAlias << " " << args;
			// retrieve the command from the input message and execute it
			nlinfo ("ADMIN: Executing control command : '%s' for service '%s'", cmdLine.c_str(), serviceAlias.c_str());
			ICommand::execute (cmdLine, IService::getInstance()->CommandLog);

			// unhook our displayer as it's work is now done
			IService::getInstance()->CommandLog.removeDisplayer(&stringDisplayer);

			// send the result back to AS
			CAdminServiceProxy as(sender);
			as.commandResult(this, commandId, serviceAlias, stringDisplayer._Data);
		}

		//The return is sent back by another message
		virtual void serviceCmd(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command)
		{
			// look in the list of service for a matching one
			IModuleProxy *proxy = findOnlineService(serviceAlias);
			if (proxy == NULL)
			{
				CAdminServiceProxy as(sender);
				as.commandResult(this, commandId, serviceAlias, "ERROR AES : unknown service");
				return;
			}

			// ok, we found it !
			TPendingWebCommand pwc;
			pwc.Command = command;
			pwc.ReceptionDate = NLMISC::CTime::getSecondsSince1970();
			pwc.ServiceAlias = serviceAlias;

			_PendingWebCommands.insert(make_pair(commandId, pwc));

			CAdminExecutorServiceClientProxy service(proxy);
			service.serviceCmd(this, commandId, command);
		}

		// AES client send back the result of execution of a command
		virtual void commandResult(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &serviceAlias, const std::string &result)
		{
			// check for waiting commands
			TPendingWebCommands::iterator it(_PendingWebCommands.find(commandId));

			if (it == _PendingWebCommands.end())
			{
				if (commandId != 0)
					nlwarning("CAdminExecutor::commandResult : service '%s' sent result for command ID %u but not in pending command table",
					sender->getModuleName().c_str(),
					commandId);
				return;
			}

			// send the result back to AS
			if (_AdminService != NULL)
			{
				CAdminServiceProxy as(_AdminService);

				as.commandResult(this, commandId, serviceAlias, result);
			}

			_PendingWebCommands.erase(commandId);
		}


		// An AES send graph data update
		virtual void graphUpdate(NLNET::IModuleProxy *sender, const TGraphDatas &graphDatas)
		{
			if (_AdminService != NULL)
			{ 
				CAdminServiceProxy as(_AdminService);
				as.graphUpdate(this, graphDatas);
			}
		}

		// A service high rez graph data update
		virtual void highRezGraphUpdate(NLNET::IModuleProxy *sender, const THighRezDatas &graphDatas)
		{
			if (_AdminService != NULL)
			{
				CAdminServiceProxy as(_AdminService);
				as.highRezGraphUpdate(this, graphDatas);
			}
		}

		// A service send an update of of it's status string
		virtual void serviceStatusUpdate(NLNET::IModuleProxy *sender, const std::string &status)
		{
			TConnectedServiceIndex::iterator it(_ConnectedServiceIndex.find(sender));
			if (it == _ConnectedServiceIndex.end())
			{
				nlwarning("serviceStatusUpdate : service '%s' send status but is unknown !", sender->getModuleName().c_str());
				return;
			}

			string &aliasName = it->second;
			TServiceStates::iterator it2(_ServiceStates.find(aliasName));
			BOMB_IF(it2 == _ServiceStates.end(), "serviceStateUpdate : service '"
				<<sender->getModuleName()
				<<"' send an update, but alias '"<<aliasName<<"' is not found in service status", return);

			TServiceState &ss = it2->second;
			ss.State = status;
			ss.LastStateDate = NLMISC::CTime::getSecondsSince1970();
		}


		///////////////////////////////////////////////////////////////////////
		//// commands handlers
		///////////////////////////////////////////////////////////////////////
		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CAdminExecutorService, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, dump, "Dump a status report to appropriate output logger", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, addRegisteredService, "add a registered service", "<serviceAlias> <shardName>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, removeRegisteredService, "remove a registered service", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, startService, "start a registered service", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, restartService, "stop then start a registered service", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, stopService, "stop a service (registered or not)", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, killService, "kill a (possibly not responding) service (registered or not)", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, abortService, "abort a (possibly not responding) service with SIGABORT (registered or not)", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, activateService, "activate a service, i.e make it startable either manually or from a shard orders", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, deactivateService, "deactivate a service, i.e make it unstartable (either manually or from a shard orders) and stop it if needed", "<serviceAlias>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, execScript, "execute the predefined bash script '/home/nevrax/patchman/aes_runnable_script.sh' and give it the passed parameters", "<any parameter>")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, resetStartCounter, "reset the start counter to 0", "no params")
			NLMISC_COMMAND_HANDLER_ADD(CAdminExecutorService, stopShard, "Stop all service of a given shard aftert the provided delay", "<shardName> <delay (in s)>")
		NLMISC_COMMAND_HANDLER_TABLE_END


		NLMISC_CLASS_COMMAND_DECL(stopShard)
		{
			if (args.size() != 2)
				return false;

			string shardName = args[0];
			uint32 delay;
			NLMISC::fromString(args[1], delay);

			log.displayNL("Received command to stop all service of shard %s in %us", shardName.c_str(), delay);

			shutdownShard(NULL, shardName, delay);

			return true;
		}


		NLMISC_CLASS_COMMAND_DECL(resetStartCounter)
		{
			if (args.size() != 0)
				return false;


			TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
			for (; first != last; ++first)
			{
				TServiceState &ss = first->second;
				
				ss.RunnerLoopCounter.resetCounter();
			}

			return true;
		}

		
		NLMISC_CLASS_COMMAND_DECL(execScript)
		{
			string cmdLine("/home/nevrax/patchman/aes_runnable_script.sh");

			// add parameters
			for (uint i=0; i<args.size(); ++i)
			{
				cmdLine += " "+args[i];
			}

			// add redirection
			string logFile = CPath::getTemporaryDirectory() + "aes_command_output.log";

			cmdLine += " > "+logFile;

			log.displayNL("Executing '%s'", cmdLine.c_str());
			// execute the command
			int ret = system(cmdLine.c_str());

			// echo the output to the requester
			CSString output;
			output.readFromFile(logFile);

			vector<CSString> lines;
			output.splitLines(lines);

			log.displayNL("Command returned value %d", ret);
			log.displayNL("-------------------- Command output begin -----------------------");
			for (uint i=0; i<lines.size(); ++i)
			{
				log.displayNL("%s", lines[i].c_str());
			}
			log.displayNL("-------------------- Command output end -----------------------");
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(deactivateService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			if (_PersistentServiceOrders.find(serviceAlias) == _PersistentServiceOrders.end())
			{
				log.displayNL("Unregistered service '%s', could not deactivate it", serviceAlias.c_str());
				return true;
			}

			_PersistentServiceOrders[serviceAlias] = TRunningOrders::ro_deactivated;

			_NeedToWriteStateFile = true;

			log.displayNL("Service '%s' deactivated", serviceAlias.c_str());

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(activateService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			if (_PersistentServiceOrders.find(serviceAlias) == _PersistentServiceOrders.end())
			{
				log.displayNL("Unregistered service '%s', could not activate it", serviceAlias.c_str());
				return true;
			}

			_PersistentServiceOrders[serviceAlias] = TRunningOrders::ro_activated;

			_NeedToWriteStateFile = true;

			log.displayNL("Service '%s' activated", serviceAlias.c_str());

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(abortService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			// check that the service is running
			TServiceStates::iterator it(_ServiceStates.find(serviceAlias));
			if (it == _ServiceStates.end())
			{
				log.displayNL("Failed to found service '%s' in the list of running services", serviceAlias.c_str());
				return true;
			}

			TServiceState &ss = it->second;
			if (ss.RunningState == TRunningState::rs_stopped)
			{
				log.displayNL("The service to abort '%s' is currently stopped", serviceAlias.c_str());
				return true;
			}
			if (ss.PID < 2)
			{
				log.displayNL("AES have no valid PID to abort the service '%s'", serviceAlias.c_str());
				return true;
			}

			// abort it
			log.displayNL("Aborting service '%s' with pid %u", serviceAlias.c_str(), ss.PID);
			abortProgram(ss.PID);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(killService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			// check that the service is running
			TServiceStates::iterator it(_ServiceStates.find(serviceAlias));
			if (it == _ServiceStates.end())
			{
				log.displayNL("Failed to found service '%s' in the list of running services", serviceAlias.c_str());
				return true;
			}

			TServiceState &ss = it->second;
			if (ss.RunningState == TRunningState::rs_stopped)
			{
				log.displayNL("The service to kill '%s' is currently stopped", serviceAlias.c_str());
				return true;
			}
			if (ss.PID < 2)
			{
				log.displayNL("AES have no valid PID to kill the service '%s'", serviceAlias.c_str());
				return true;
			}
			// kill it
			log.displayNL("Killing service '%s' with pid %u", serviceAlias.c_str(), ss.PID);
			killProgram(ss.PID);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(stopService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			if (_ServiceStates.find(serviceAlias) == _ServiceStates.end())
			{
				log.displayNL("Unknown service '%s', could not stop it", serviceAlias.c_str());
				return true;
			}

			TServiceState &ss = _ServiceStates[serviceAlias];
			// look for a shard orders for this service
			TShardsOrders::iterator it(_ShardOrders.find(ss.ShardName));
			if (it != _ShardOrders.end())
			{
				TShardOrders &so = it->second;
				if (so == TShardOrders::so_autostart_on)
				{
					log.displayNL("Can't stop service '%s' because shard '%s' is autostarting, considers either to deactivate the service or just restart it",
						serviceAlias.c_str(),
						ss.ShardName.c_str());
					return true;
				}
			}

			if (stopService(serviceAlias))
				log.displayNL("Failed to stop the service '%s'", serviceAlias.c_str());
			else
				log.displayNL("Service '%s' stop request done", serviceAlias.c_str());

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(restartService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			if (_RegisteredServices.find(serviceAlias) == _RegisteredServices.end())
			{
				log.displayNL("startService %s : the service in not registered, can't restart it", serviceAlias.c_str());
				return true;
			}

			// look for service orders for this service
			if (_PersistentServiceOrders.find(serviceAlias) != _PersistentServiceOrders.end())
			{
				if (_PersistentServiceOrders[serviceAlias] == TRunningOrders::ro_deactivated)
				{
					log.displayNL("Can't restart service '%s' because it is currently deactivated", serviceAlias.c_str());
					return true;
				}
			}



			// check that the service is running
			TServiceStates::iterator it(_ServiceStates.find(serviceAlias));
			if (it == _ServiceStates.end())
			{
				log.displayNL("Failed to found service '%s' in the list of running services", serviceAlias.c_str());
				return true;
			}

			// write the deferred start command
			if (!writeServiceLaunchCtrl(serviceAlias, true, LAUNCH_CTRL_START))
			{
				log.displayNL("Failed to write deferred start control file to restart service '%s'", serviceAlias.c_str());
				return true;
			}
			else
				log.displayNL("Service '%s' start command written", serviceAlias.c_str());

			if (it->second.ServiceModule == NULL)
			{
				log.displayNL("The AES client module proxy is null ! can't send 'quit' command");
			}

			// send the "quit" command to the service
			CAdminExecutorServiceClientProxy aec(it->second.ServiceModule);
			aec.serviceCmd(this, 0, "quit");
			log.displayNL("Service '%s' command 'quit' sent", serviceAlias.c_str());

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(startService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			if (_ServiceStates.find(serviceAlias) == _ServiceStates.end())
			{
				log.displayNL("Unknown service '%s', could not start it", serviceAlias.c_str());
				return true;
			}

			TServiceState &ss = _ServiceStates[serviceAlias];

			// look for service orders for this service
			if (_PersistentServiceOrders.find(serviceAlias) != _PersistentServiceOrders.end())
			{
				if (_PersistentServiceOrders[serviceAlias] == TRunningOrders::ro_deactivated)
				{
					log.displayNL("Can't start service '%s' because it is curently deactivated", serviceAlias.c_str());
					return true;
				}
			}

			// look for a shard orders for this service
			TShardsOrders::iterator it(_ShardOrders.find(ss.ShardName));
			if (it != _ShardOrders.end())
			{
				TShardOrders &so = it->second;
				if (so == TShardOrders::so_autostart_on)
				{
					log.displayNL("Can't start service '%s' because shard '%s' is autostarting, consider to restart it",
						serviceAlias.c_str(),
						ss.ShardName.c_str());
					return true;
				}
			}

			if (!startService(serviceAlias))
				log.displayNL("Failed to start service '%s'", serviceAlias.c_str());
			else
				log.displayNL("Service '%s' start command written", serviceAlias.c_str());

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(removeRegisteredService)
		{
			if (args.size() != 1)
				return false;

			string serviceAlias = args[0];

			if (_ServiceStates.find(serviceAlias) == _ServiceStates.end())
			{
				log.displayNL("Unknown service '%s', could not start it", serviceAlias.c_str());
				return true;
			}

			TServiceState &ss = _ServiceStates[serviceAlias];

			_RegisteredServices.erase(serviceAlias);

			if (ss.RunningState == TRunningState::rs_stopped)
			{
				// remove the record
				_ServiceStates.erase(serviceAlias);
			}
			else
			{
				// just update some data related the registered service				
				ss.ShardName = "";
				ss.RunningTags.erase(TRunningTag::rt_locally_started);
				ss.RunningTags.erase(TRunningTag::rt_chain_crashing);
				ss.RunningTags.insert(TRunningTag::rt_externaly_started);
			}


			_PersistentServiceOrders.erase(serviceAlias);
			_NeedToWriteStateFile = true;

			// update the state of services to the AS
			sendUpServiceUpdate();

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(addRegisteredService)
		{
			if (args.size() != 2)
				return false;

			string serviceAlias = args[0];
			string shardName = args[1];

			_RegisteredServices.insert(serviceAlias);
			_ServiceStates.insert(make_pair(serviceAlias, TServiceState()));
			_ServiceStates[serviceAlias].ShardName = shardName;
//			_ServiceRunnerLoopCounters.insert(make_pair(serviceAlias, TRunnerLoopCounter()));

			if (_PersistentServiceOrders.find(serviceAlias) == _PersistentServiceOrders.end())
			{
				_PersistentServiceOrders[serviceAlias] = TRunningOrders::ro_activated;
				_NeedToWriteStateFile = true;
			}

			// update the state of services to the AS
			sendUpServiceUpdate();

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			log.displayNL("===============================");
			log.displayNL(" Dumping Admin executor states");
			log.displayNL("===============================");

			{
				log.displayNL("  There are %u known shard :", _ShardOrders.size());
				{
					TShardsOrders::iterator first(_ShardOrders.begin()), last(_ShardOrders.end());
					for (; first != last; ++first)
					{
						log.displayNL("  + Shard '%s' orders is '%s'", first->first.c_str(), first->second.toString().c_str());
					}
				}
				if (_ShutdownForPatch)
					log.displayNL("  All service are shuting down for patch !");
				log.displayNL("  There are %u known services :", _ServiceStates.size());
				TServiceStates::iterator first(_ServiceStates.begin()), last(_ServiceStates.end());
				for (; first != last; ++first)
				{
					TServiceState &ss = first->second;
					const string &aliasName = first->first;

					CSString runningTags;
					set<TRunningTag>::iterator rtf(ss.RunningTags.begin()), rte(ss.RunningTags.end());
					for (; rtf != rte; ++rtf)
					{
						runningTags<<"<"<<rtf->toString()<<">";
					}

					bool registered = _RegisteredServices.find(aliasName) != _RegisteredServices.end();

					log.displayNL("    + Service alias='%s' (%s) ShardName = '%s' RunningState='%s' RunningTag='%s'", 
						aliasName.c_str(),
						registered ? "REGISTERED" : "NOT REGISTERED",
						ss.ShardName.c_str(),
						ss.RunningState.toString().c_str(),
						runningTags.c_str());

					log.display("    |   %s", ss.DontUseShardOrders ? "DontUseShardOders" : "UseShardOrders");

					if (ss.RunningState != TRunningState::rs_stopped)
					{
						// the pid should be valid
						log.display(" PID=%u",	ss.PID);
					}
					if (registered)
					{
						log.display(" ServiceOrders=%s", _PersistentServiceOrders[aliasName].toString().c_str());
					}
					log.displayNL("");


					if (ss.ServiceModule != NULL)
					{
						// dump a connected service
						log.displayNL("    |   longName='%s' shortName='%s' moduleName='%s'",
							ss.LongName.c_str(),
							ss.ShortName.c_str(),
							ss.ServiceModule->getModuleName().c_str());
						log.displayNL("    |   State '%s' (last received %sago)", ss.State.c_str(), NLMISC::CTime::getHumanRelativeTime(NLMISC::CTime::getSecondsSince1970() - ss.LastStateDate).c_str());
					}
					else
					{
						// dump a offline registered service
						// dump a connected service
						log.displayNL("    |   longName='%s' shortName='%s' ",
							ss.LongName.c_str(),
							ss.ShortName.c_str());
						log.displayNL("    |   State '%s' (last received %sago)", ss.State.c_str(), NLMISC::CTime::getHumanRelativeTime(NLMISC::CTime::getSecondsSince1970() - ss.LastStateDate).c_str());
					}
					if (registered)
					{
						uint32 c1, c2, c3;
						ss.RunnerLoopCounter.getCounters(c1, c2, c3);
						log.displayNL("    |   Service Runner Start counter (%u mn:%u run, %u mn:%u run, %u mn:%u run)",
							CRASH_COUNTER_ROLL_DELAY/60, c1,
							(CRASH_COUNTER_ROLL_DELAY*3)/60, c2,
							(CRASH_COUNTER_ROLL_DELAY*CRASH_COUNTER_SLOT)/60, c3);
					}
				}
			}

			return true;
		}


	};

	NLNET_REGISTER_MODULE_FACTORY(CAdminExecutorService, "AdminExecutorService");

} // namespace ADMIN

