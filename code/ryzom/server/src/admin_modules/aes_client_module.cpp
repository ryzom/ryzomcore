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
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"

#include "admin_modules_itf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

void aesclient_forceLink() {}

namespace ADMIN
{
	class CAdminExecutorServiceClient 
		:	/*public CManualSingleton<CAdminExecutorService>,*/
			public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
			public CAdminExecutorServiceClientSkel
	{

		enum
		{
			// Maximum time without sending report string (a kind of 'keep alive')
			MAX_DELAY_BETWEEN_REPORT = 30,	// 30 seconds
		};

		/// Flag to inform AES that we don't want to be affected by shard orders
		bool				_DontUseShardOrders;

		/// Admin executor service module
		TModuleProxyPtr		_AdminExecutorService;

		/// Date of last state reporting to AES
		uint32				_LastStateReport;

		/// Last date of status string update
		uint32				_LastStatusStringReport;
		/// Last status string sent (to avoid double send)
		string				_LastSentStatus;

		/// The service alias (must be an unique name)
		string				_ServiceAlias;

		/// A cache of the value because reading it is very slow
		uint32				_ProcessUsedMemory;

		struct TGraphSample
		{
			// The date of the sample (in second)
			uint32	TimeStamp;
			// The date of the sample (in ms)
			TTime	HighRezTimeStamp;
			// sample value
			double	SampleValue;
		};
		struct TGraphVarInfo
		{
			/// Name of the graphed var
			string	VarName;
			/** Mean time between two sample in ms
			 *	(in fact, if will be the min period)
			 *	Set it to 1 to have a sample at each tick
			 *	If the period is set less than 1000 ms,
			 *	then the var is considered 'high rez'.
			 *	Otherwise, the period is rounded at the
			 *	nearest integer second.
			 *	For 'high rez' var, the service buffer
			 *	the relative timestamp in ms at each
			 *	tick loop and send update every seconds 
			 *	to the AES service.
			 *	In addition, HighRez var are also sent 
			 *	every second as normal sample.
			 */
			uint32	MeanSamplePeriod;

			/// Date of last sample (low rez)
			uint32	LastSampleTimeStamp;
			/// Date of last sample (high rez)
			TTime	LastHighRezTimeStamp;

			/// The vector of buffered samples
			vector<TGraphSample>	Samples;

			TGraphVarInfo()
				:	MeanSamplePeriod(1000),
					LastSampleTimeStamp(0),
					LastHighRezTimeStamp(0)
			{
			}
		};

		/// The list of variable to graph (build from service config file var 'GraphVars')
		vector<TGraphVarInfo>	_GraphVars;

		/// Date of last graph

	public:

		CAdminExecutorServiceClient()
			:	_DontUseShardOrders(false),
				_LastStateReport(0),
				_LastStatusStringReport(0),
				_ProcessUsedMemory(0)
		{
			CAdminExecutorServiceClientSkel::init(this);

		}

		std::string makeServiceAlias()
		{
			string serviceAlias = IService::getInstance()->getServiceAliasName();
			if (serviceAlias.empty())
			{
				serviceAlias = IService::getInstance()->getHostName()+"."+IService::getInstance()->getServiceUnifiedName();
			}
			return serviceAlias;
		}

		string getModuleManifest() const
		{
			uint32 pid = getpid ();

			string serviceAlias = _ServiceAlias;

			CSString manifest;

			manifest << "LongName=" << IService::getInstance()->getServiceLongName()
				<< " ShortName=" << IService::getInstance()->getServiceShortName()
				<< " AliasName=" << serviceAlias
				<< " PID=" << pid
				<< " DontUseShardOrders=" << _DontUseShardOrders;

			return manifest;
		}

		bool initModule(const TParsedCommandLine &pcl)
		{
			if (!CModuleBase::initModule(pcl))
				return false;

			// try to read the config file
			IService *service = IService::getInstance();
			if (service == NULL)
			{
				nlwarning("Failed to get the IService singleton instance");
				return false;
			}

			CConfigFile::CVar *gv = service->ConfigFile.getVarPtr("GraphVars");
			if (gv)
			{
				_GraphVars.clear();
				for (uint i =0; i<gv->size()/2; ++i)
				{
					TGraphVarInfo gvi;

					gvi.VarName = gv->asString(i*2);
					gvi.MeanSamplePeriod = max(1, gv->asInt((i*2)+1));

					_GraphVars.push_back(gvi);
				}
			}

			// precompute the service name
			_ServiceAlias = makeServiceAlias();

			// loop for an optional 'dontUseShardOrders' flag in init params
			const TParsedCommandLine *duso = pcl.getParam("dontUseShardOrders");
			if (duso != NULL)
				_DontUseShardOrders = (duso->ParamValue == "true" || duso->ParamName == "1");

			return true;
		}


		void onModuleUp(IModuleProxy *proxy)
		{
			if (proxy->getModuleClassName() == "AdminExecutorService")
			{
				nldebug("CAdminExecutorServiceClient : admin executor service up as '%s'", proxy->getModuleName().c_str());
				// we found the manager of AES
				if (_AdminExecutorService != NULL)
				{
					nlwarning("CAdminExecutorServiceClient : admin executor service already known as '%s', replacing with new one", _AdminExecutorService->getModuleName().c_str());
				}
				_AdminExecutorService = proxy;

//				// send basic service info to AES
//				CAdminExecutorServiceProxy aes(proxy);
//
//				uint32 pid = getpid ();
//
//				string serviceAlias = IService::getInstance()->getServiceAliasName();
//				if (serviceAlias.empty())
//					serviceAlias = getModuleFullyQualifiedName();
//
//				aes.serviceConnected(this, 
//					IService::getInstance()->getServiceLongName(), 
//					IService::getInstance()->getServiceShortName(),
//					serviceAlias,
//					pid);
				// for resend of the current status to the new AES
				_LastSentStatus = "";
				sendServiceStatus();
			}
		}

		void onModuleDown(IModuleProxy *proxy)
		{
			if (proxy == _AdminExecutorService)
			{
				nldebug("CAdminExecutorServiceClient : admin executor service '%s' is down", proxy->getModuleName().c_str());

				_AdminExecutorService = NULL;
			}
		}

		void onModuleUpdate()
		{
			H_AUTO(CAdminExecutorServiceClient_onModuleUpdate);

			uint32 now = CTime::getSecondsSince1970();
			TTime timer = CTime::getLocalTime();

			// update every HR variables
			for (uint i=0; i<_GraphVars.size(); ++i)
			{
				if (_GraphVars[i].MeanSamplePeriod < 1000)
				{
					// this is a HR var
					TGraphVarInfo &gvi = _GraphVars[i];
					if (gvi.LastHighRezTimeStamp + gvi.MeanSamplePeriod < timer)
					{
						// it's time to get a sample
						// create a sample
						gvi.Samples.push_back(TGraphSample());
						TGraphSample &gs = gvi.Samples.back();
						// inialise it
						gs.TimeStamp = now;
						gs.HighRezTimeStamp = timer;
						IVariable *var = dynamic_cast<IVariable*>(ICommand::getCommand(gvi.VarName));
						if (var != NULL)
							gs.SampleValue = atof(var->toString().c_str());
					}
				}
			}

			if (_LastStateReport != now)
			{

				if ((now & 0xf) == 0)
				{
					// every 16 seconds because very slow
					IVariable *var = dynamic_cast<IVariable*>(ICommand::getCommand("ProcessUsedMemory"));
					if (var != NULL)
						NLMISC::fromString(var->toString(), _ProcessUsedMemory);
				}

				// at least one second as passed, check for updates to send to 
				// AES

				TGraphDatas	graphDatas;
				graphDatas.setCurrentTime(now);

				THighRezDatas highRezDatas;
				highRezDatas.setServiceAlias(_ServiceAlias);
				highRezDatas.setCurrentTime(now);

				vector<TGraphData>	&datas = graphDatas.getDatas();

				for (uint i=0; i<_GraphVars.size(); ++i)
				{
					if (_GraphVars[i].LastSampleTimeStamp+(_GraphVars[i].MeanSamplePeriod/1000) < now)
					{
						TGraphVarInfo &gvi = _GraphVars[i];
						// it's time to send update for this var
						// create a new sample entry
						datas.push_back(TGraphData());
						// and fill it
						TGraphData &gd = datas.back();
						gd.setServiceAlias(_ServiceAlias);
						gd.setVarName(gvi.VarName);
						gd.setSamplePeriod(max(uint32(1), uint32(gvi.MeanSamplePeriod/1000)));
						if (gvi.Samples.empty())
						{
							// no sample collected yet, just ask a new one
							IVariable *var = dynamic_cast<IVariable*>(ICommand::getCommand(gvi.VarName));
							if (var != NULL)
								gd.setValue(atof(var->toString().c_str()));
						}
						else
						{
							// we have some sample collected, just use the last one
							gd.setValue(gvi.Samples.back().SampleValue);
						}
						
						// if it's a high rez sampler, send the complete buffer
						if (gvi.MeanSamplePeriod < 1000 && _AdminExecutorService != NULL)
						{
							// build the message
							highRezDatas.setVarName(gvi.VarName);
							highRezDatas.getDatas().clear();

							for (uint j=0; j<gvi.Samples.size(); ++j)
							{
								highRezDatas.getDatas().push_back(THighRezData());
								THighRezData &hrd = highRezDatas.getDatas().back();
								hrd.setSampleTick(gvi.Samples[j].HighRezTimeStamp);
								hrd.setValue(gvi.Samples[j].SampleValue);
							}

							if (!highRezDatas.getDatas().empty() && _AdminExecutorService != NULL)
							{
								// send the high rez data
								CAdminExecutorServiceProxy aes(_AdminExecutorService);
								aes.highRezGraphUpdate(this, highRezDatas);
							}

							// we don't send normal update for high rez sampler
							datas.pop_back();
						}

						// update the time stamp
						gvi.LastSampleTimeStamp = now;
						// clear the buffer
						gvi.Samples.clear();
					}
				}

				// if we have some data to send, send them
				if (!datas.empty() && _AdminExecutorService != NULL)
				{
					CAdminExecutorServiceProxy aes(_AdminExecutorService);
					aes.graphUpdate(this, graphDatas);
				}
						
				// update the last report date
				_LastStateReport = now;
			}

			// send an update of the status (if needed)
			sendServiceStatus();
		}

		void sendServiceStatus()
		{
			CSString status;
			uint32 now = NLMISC::CTime::getSecondsSince1970();

			status << "\tServiceAlias=" << _ServiceAlias;

			// build the status string
			IVariable *var = dynamic_cast<IVariable*>(ICommand::getCommand("State"));
			if (var != NULL)
				status << "\tState=" <<var->toString();

			var = dynamic_cast<IVariable*>(ICommand::getCommand("UserSpeedLoop"));
			if (var != NULL)
				status << "\tUserSpeedLoop=" <<var->toString();

			var = dynamic_cast<IVariable*>(ICommand::getCommand("TickSpeedLoop"));
			if (var != NULL)
				status << "\tTickSpeedLoop=" <<var->toString();

			if (_ProcessUsedMemory != 0)
				status << "\tProcessUsedMemory=" <<_ProcessUsedMemory;

			var = dynamic_cast<IVariable*>(ICommand::getCommand("Uptime"));
			if (var != NULL)
				status << "\tUpTime=" <<var->toString();

			var = dynamic_cast<IVariable*>(ICommand::getCommand("NbPlayers"));
			if (var != NULL)
				status << "\tNbPlayers=" <<var->toString();

			var = dynamic_cast<IVariable*>(ICommand::getCommand("NbEntities"));
			if (var != NULL)
				status << "\tNbEntities=" <<var->toString();

			var = dynamic_cast<IVariable*>(ICommand::getCommand("LocalEntities"));
			if (var != NULL)
				status << "\tLocalEntities=" <<var->toString();

			uint32 shardId = IService::getInstance()->getShardId();
			status << "\tShardId=" <<toString(shardId);

			// add any service specific info
			if (IService::isServiceInitialized())
			{
				status << "\t" << IService::getInstance()->getServiceStatusString();
			}

			if ((status != _LastSentStatus || (now - _LastStatusStringReport) > MAX_DELAY_BETWEEN_REPORT) 
				&& _AdminExecutorService != NULL)
			{
				CAdminExecutorServiceProxy aes(_AdminExecutorService);
				aes.serviceStatusUpdate(this, status);

				_LastSentStatus = status;
				_LastStatusStringReport = now;
			}
		}

		///////////////////////////////////////////////////////////////
		// implementation from Admin executor service client
		///////////////////////////////////////////////////////////////

		// execute a command and return the result.
		virtual void serviceCmd(NLNET::IModuleProxy *sender, uint32 commandId, const std::string &command)
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
			CStringDisplayer stringDisplayer;
			IService::getInstance()->CommandLog.addDisplayer(&stringDisplayer);

			// retrieve the command from the input message and execute it
			nlinfo ("ADMIN: Executing command from network : '%s'", command.c_str());
			ICommand::execute (command, IService::getInstance()->CommandLog);

			// unhook our displayer as it's work is now done
			IService::getInstance()->CommandLog.removeDisplayer(&stringDisplayer);

			string serviceAlias = IService::getInstance()->getServiceAliasName();
			if (serviceAlias.empty())
				serviceAlias = getModuleFullyQualifiedName();

			// return the result to AES
			CAdminExecutorServiceProxy aes(sender);
			aes.commandResult(this, commandId, serviceAlias, stringDisplayer._Data);
		}

		// execute a command without result
		virtual void serviceCmdNoReturn(NLNET::IModuleProxy *sender, const std::string &command)
		{
			// retrieve the command from the input message and execute it
			nlinfo ("ADMIN: Executing command from network : '%s'", command.c_str());
			ICommand::execute (command, IService::getInstance()->CommandLog);
		}
	};

	NLNET_REGISTER_MODULE_FACTORY(CAdminExecutorServiceClient, "AdminExecutorServiceClient");


} // namespace ADMIN

