// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif // HAVE_CONFIG_H

#ifndef NELNS_CONFIG
#	define NELNS_CONFIG ""
#endif // NELNS_CONFIG

#ifndef NELNS_LOGS
#	define NELNS_LOGS ""
#endif // NELNS_LOGS

#include "nel/misc/types_nl.h"

#include <fcntl.h>
#include <sys/stat.h>

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	include <direct.h>
#	undef max
#	undef min
#else
#	include <unistd.h>
#	include <errno.h>
#endif

#include <string>
#include <list>

#include "nel/misc/debug.h"
#include "nel/misc/system_info.h"
#include "nel/misc/config_file.h"
#include "nel/misc/thread.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"
#include "nel/misc/value_smoother.h"
#include "nel/misc/singleton.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"

#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/varpath.h"
#include "nel/net/email.h"
#include "nel/net/admin.h"

#include "log_report.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Structures
//

struct CRequest
{
	CRequest(uint32 id, TServiceId sid) : Id(id), NbWaiting(0), NbReceived(0), SId(sid)
	{
		nldebug ("REQUEST: ++ NbWaiting %d NbReceived %d", NbWaiting, NbReceived);
		Time = CTime::getSecondsSince1970 ();
	}

	uint32			Id;
	uint			NbWaiting;
	uint32			NbReceived;
	TServiceId		SId;
	uint32			Time;	// when the request was ask

	TAdminViewResult Answers;
};

struct CService
{
	CService() { reset(); }

	string			AliasName;		/// alias of the service used in the AES and AS to find him (unique per AES)
									/// If alias name is not empty, it means that the service was registered

	string			ShortName;		/// name of the service in short format ("NS" for example)
	string			LongName;		/// name of the service in long format ("naming_service")
	TServiceId		ServiceId;		/// service id of the service.
	bool			Ready;			/// true if the service is ready
	bool			Connected;		/// true if the service is connected to the AES
	vector<CSerialCommand>	Commands;
	bool			AutoReconnect;	/// true means that AES must relaunch the service if lost
	uint32			PId;			/// process Id used to kill the application
	bool			Relaunch;		/// if true, it means that the admin want to close and relaunch the service

	uint32			LastPing;		/// time in seconds of the last ping sent. If 0, means that the service already pong

	vector<uint32>	WaitingRequestId;		/// contains all request that the server hasn't reply yet

	string toString() const
	{
		return getServiceUnifiedName();
	}

	void init(const string &shortName, TServiceId serviceId)
	{
		reset();

		ShortName = shortName;
		ServiceId = serviceId;
		Connected = true;
	}

	void reset()
	{
		AliasName.clear();
		ShortName.clear();
		LongName.clear();
		ServiceId.set(0);
		Ready = false;
		Connected = false;
		Commands.clear();
		AutoReconnect = false;
		PId = 0;
		Relaunch = false;
		LastPing = 0;
		WaitingRequestId.clear();
	}

	std::string getServiceUnifiedName() const
	{
		nlassert(!ShortName.empty());
		string res;
		if(!AliasName.empty())
		{
			res = AliasName+"/";
		}
		res += ShortName;
		if(ServiceId.get() != 0)
		{
			res += "-";
			res += NLMISC::toString(ServiceId);
		}
		if(res.empty())
			res = "???";
		return res;
	}
};


//
// CVariables (variables that can be set in cfg files, etc)
//

CVariable<uint32> RequestTimeout("aes","RequestTimeout", "in seconds, time before a request is timeout", 5, 0, true);		// in seconds, timeout before canceling the request
CVariable<uint32> PingTimeout("aes","PingTimeout", "in seconds, time before services have to answer the ping message or will be killed", 900, 0, true);		// in seconds, timeout before killing the service
CVariable<uint32> PingFrequency("aes","PingFrequency", "in seconds, time between each ping message", 60, 0, true);		// in seconds, frequency of the send ping to services
CVariable<bool>   KillServicesOnDisconnect("aes","KillServicesOnDisconnect", "if set, call killProgram on services as they disconnect", false, 0, true);

//
// Global Variables (containers)
//

typedef vector<CService> TServices;
TServices Services;

vector<CRequest> Requests;

vector<string> RegisteredServices;

vector<pair<uint32, string> > WaitingToLaunchServices;	// date and alias name

vector<string> AllAdminAlarms;	// contains *all* alarms

vector<string> AllGraphUpdates;	// contains *all* the graph updates


//
// Global Variables (scalars)
//

uint32 LastPing = 0;		// contains the date of the last ping sent to the services


//
// Alarms
//

void sendInformation(TServiceId sid)
{
	CMessage msgout("INFO");
	msgout.serialCont(AllAdminAlarms);
	msgout.serialCont(AllGraphUpdates);
	for (uint j = 0; j < Services.size(); j++)
	{
		if (Services[j].ServiceId == sid && Services[j].Connected)
		{
			CUnifiedNetwork::getInstance()->send(Services[j].ServiceId, msgout);
		}
	}
}


//
// Launch services functions
//

void sendAdminEmail(const char *format, ...)
{
	char *text;
	NLMISC_CONVERT_VARGS(text, format, 4096);

	CMessage msgout("ADMIN_EMAIL");
	string str = text;
	msgout.serial(str);
	CUnifiedNetwork::getInstance()->send("AS", msgout);

	nlinfo("Forwarded email to AS with '%s'", text);
}

static void cbAdminEmail(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	string str;
	msgin.serial(str);
	sendAdminEmail(str.c_str());
}

static void cbGraphUpdate(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	CMessage msgout("GRAPH_UPDATE");
	uint32 CurrentTime;
	msgin.serial(CurrentTime);
	msgout.serial(CurrentTime);
	while (msgin.getPos() < (sint32)msgin.length())
	{
		string service, var;
		sint32 val;
		msgin.serial(service, var, val);
		msgout.serial(service, var, val);
	}
	CUnifiedNetwork::getInstance()->send("AS", msgout);

	nlinfo("GRAPH: Forwarded graph update to AS");
}

// decode a service in a form 'alias/shortname-sid'
void decodeUnifiedName(const string &unifiedName, string &alias, string &shortName, TServiceId &sid)
{
	size_t pos1 = 0, pos2 = 0;
	pos1 = unifiedName.find("/");
	if (pos1 != string::npos)
	{
		alias = unifiedName.substr(0, pos1);
		pos1++;
	}
	else
	{
		alias = "";
		pos1 = 0;
	}
	pos2 = unifiedName.find("-");
	if (pos2 != string::npos)
	{
		shortName = unifiedName.substr(pos1,pos2-pos1);
		sid.set(atoi(unifiedName.substr(pos2+1).c_str()));
	}
	else
	{
		shortName = unifiedName.substr(pos1);
		sid.set(0);
	}

	if (alias.empty())
	{
		alias = shortName;
	}
}


bool getServiceLaunchInfo(const string& unifiedName,string& alias,string& command,string& path,string& arg)
{
	string shortName;
	TServiceId sid;
	decodeUnifiedName(unifiedName, alias, shortName, sid);
	try
	{
		path = IService::getInstance()->ConfigFile.getVar(alias).asString(0);
		command = IService::getInstance()->ConfigFile.getVar(alias).asString(1);
	}
	catch(EConfigFile &e)
	{
		nlwarning("Error in service '%s' in config file (%s)", unifiedName.c_str(), e.what());
		return false;
	}

	if (IService::getInstance()->ConfigFile.getVar(alias).size() >= 3)
	{
		arg = IService::getInstance()->ConfigFile.getVar(alias).asString(2);
	}

	path = NLMISC::CPath::standardizePath(path);
	NLMISC::CFile::createDirectoryTree(path);
	return true;
}

std::string getServiceStateFileName(const std::string& serviceAlias,const std::string& serviceExecutionPath)
{
	return NLMISC::CPath::standardizePath(serviceExecutionPath)+serviceAlias+".state";
}

// the following routine reads the text string contained in the ".state" file for this service
// it's used to provide a 'state' value for services that are registered but are not connected
// to give info on whether they've been launched, whether their launcher is online, etc
std::string getOfflineServiceState(const std::string& serviceAlias,const std::string& serviceExecutionPath)
{
	// open the file for reading
	FILE* f= fopen(getServiceStateFileName(serviceAlias,serviceExecutionPath).c_str(),"rt");
	if (f==NULL) return "Offline";

	// setup a buffer to hold the text read from the file
	uint32 fileSize= NLMISC::CFile::getFileSize(f);
	std::string txt;
	txt.resize(fileSize);

	// read the text from the file - note that the number of bytes read may be less than the
	// number of bytes requested because we've opened the file in text mode and not binary mode
	uint32 bytesRead= fread(&txt[0],1,fileSize,f);
	txt.resize(bytesRead);
	fclose(f);

	// return the text read from the file
	return txt;
}


const char* LaunchCtrlStart= "LAUNCH";
const char* LaunchCtrlStop= "STOP";

std::string getServiceLaunchCtrlFileName(const std::string& serviceAlias,const std::string& serviceExecutionPath,bool delay)
{
	return NLMISC::CPath::standardizePath(serviceExecutionPath)+serviceAlias+(delay?"_waiting":"_immediate")+".launch_ctrl";
}

bool writeServiceLaunchCtrl(const std::string& serviceAlias,const std::string& serviceExecutionPath,bool delay,const std::string& txt)
{
	// make sure the path exists
	NLMISC::CFile::createDirectoryTree(serviceExecutionPath);

	// open the file for writing
	FILE* f= fopen(getServiceLaunchCtrlFileName(serviceAlias,serviceExecutionPath,delay).c_str(),"wt");
	if (f==NULL) return false;

	// write the text to the file
	fprintf(f,"%s",txt.c_str());
	fclose(f);

	return true;
}

bool writeServiceLaunchCtrl(TServiceId serviceId,bool delay,const std::string& txt)
{
	// run trough the services container looking for a match for the service id that we've been given
	for (TServices::iterator it= Services.begin(); it!=Services.end(); ++it)
	{
		if (it->ServiceId==serviceId)
		{
			// we found a match for the service id so try to do something sensible with it...

			// get hold of the different components of the command description...
			string alias, command, path, arg;
			bool ok= getServiceLaunchInfo(it->AliasName,alias,command,path,arg);
			if (!ok) return false;

			// go ahead and write the launch ctrl file...
			return writeServiceLaunchCtrl(alias,path,delay,txt);
		}
	}

	// we failed to find a match for the serviceId that we've been given so complain and return false
	nlwarning("Failed to write launch_ctrl file for unknown service: %u",serviceId.get());
	return false;
}

// start a service imediatly
bool startService(const string &unifiedName)
{
	// lookup the alias, command to execute, execution path and defined command args for the given service
	string alias, command, path, arg;
	bool ok= getServiceLaunchInfo(unifiedName,alias,command,path,arg);

	// make sure the alias, command, etc were setup ok
	if (!ok) return false;
	nlinfo("Starting the service alias '%s'", alias.c_str());

	bool dontLaunchServicesDirectly= IService::getInstance()->ConfigFile.exists("DontLaunchServicesDirectly")? IService::getInstance()->ConfigFile.getVar("DontLaunchServicesDirectly").asBool(): false;
	if (!dontLaunchServicesDirectly)
	{
		// give the service alias to the service to forward it back when it will connected to the aes.
		arg += " -N";
		arg += alias;

		// set the path for running
		arg += " -A";
		arg += path;

		// suppress output to stdout
		#ifdef NL_OS_WINDOWS
			arg += " >NUL:";
		#else
			arg += " >/dev/null";
		#endif

		// launch the service
		bool res = launchProgram(command, arg);

		// if launching ok, leave 1 second to the new launching service before lauching next one
		if (res)
			nlSleep(1000);

		return res;
	}
	else
	{
		// there's some other system responsible for launching apps so just set a flag
		// for the system in question to use
		return writeServiceLaunchCtrl(alias,path,false,LaunchCtrlStart);
	}
}


// start service in future
void startService(uint32 delay, const string &unifiedName)
{
	// make sure there really is a delay specified - otherwise just launch the servicde directly
	if (delay == 0)
	{
		startService(unifiedName);
		return;
	}

	// lookup the alias, command to execute, execution path and defined command args for the given service
	string alias, command, path, arg;
	bool ok= getServiceLaunchInfo(unifiedName,alias,command,path,arg);

	// make sure the alias, command, etc were setup ok
	if (!ok) return;
	nlinfo("Setting up restart for service '%s'", alias.c_str());

	// check whether a config file variable has been set to signal that some other process is responsible for launching services
	bool dontLaunchServicesDirectly= IService::getInstance()->ConfigFile.exists("DontLaunchServicesDirectly")? IService::getInstance()->ConfigFile.getVar("DontLaunchServicesDirectly").asBool(): false;
	if (dontLaunchServicesDirectly)
	{
		// there's some other system responsible for launching apps so just set a flag
		// for the system in question to use
		writeServiceLaunchCtrl(alias,path,true,LaunchCtrlStart);
		return;
	}

	// check that the service isn't already in the queue of waiting services
	for(uint i = 0; i < WaitingToLaunchServices.size(); i++)
	{
		if (WaitingToLaunchServices[i].second == alias)
		{
			nlwarning("Service %s already in waiting queue to launch", unifiedName.c_str());
			return;
		}
	}

	// queue up this service for launching
	nlinfo("Relaunching the service %s in %d seconds", unifiedName.c_str(), delay);
	WaitingToLaunchServices.push_back(make_pair(CTime::getSecondsSince1970() + delay, unifiedName));
}

void checkWaitingServices()
{
	uint32 d = CTime::getSecondsSince1970();

	for(uint i = 0; i < WaitingToLaunchServices.size(); )
	{
		if (WaitingToLaunchServices[i].first <= d)
		{
			startService(WaitingToLaunchServices[i].second);
			WaitingToLaunchServices.erase(WaitingToLaunchServices.begin()+i);
		}
		else
		{
			i++;
		}
	}
}

static void checkPingPong()
{
	uint32 d = CTime::getSecondsSince1970();

	bool allPonged = true;
	bool haveService = false;

	for(uint i = 0; i < Services.size(); i++)
	{
		if(Services[i].Ready)
		{
			haveService = true;
			if(Services[i].LastPing != 0)
			{
				allPonged = false;
				if(d > Services[i].LastPing + PingTimeout)
				{
					nlwarning("Service %s-%hu seems dead, no answer for the last %d second, I kill it right now and relaunch it in few time", Services[i].LongName.c_str(), Services[i].ServiceId.get(),(uint32)PingTimeout);
					Services[i].AutoReconnect = false;
					Services[i].Relaunch = true;
					abortProgram(Services[i].PId);
					Services[i].LastPing = 0;
				}
			}
		}
	}

	if(haveService && allPonged && d > LastPing + PingFrequency)
	{
		LastPing = d;
		nlinfo("send ping");
		for(uint i = 0; i < Services.size(); i++)
		{
			if(Services[i].Ready)
			{
				Services[i].LastPing = d;
				nlinfo("send ping to one service");
				CMessage msgout("ADMIN_PING");
				CUnifiedNetwork::getInstance()->send(Services[i].ServiceId, msgout);
			}
		}
	}
}

// check to see if a file exists that should provoke a shutdown of running services
// this is typically used in the deployment process to ensure that all services are
// given a chance to shut down cleanly before servre patches are applied
CVariable<string> ShutdownRequestFileName("aes","ShutdownRequestFileName", "name of the file to use for shutdown requests", "./global.launch_ctrl", 0, true);
static void checkShutdownRequest()
{
	// a little system to prevent us from eating too much CPU on systems that have a cstly 'fileExists()'
	static uint32 count=0;
	if ((++count)<10)	return;
	count=0;

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
		// get rid of any unwanted junk surrounding the file contents
		nlinfo("Treating shutdown request from ctrl file %s: %s",ShutdownRequestFileName.c_str(),("#.State="+fileContents).c_str());
		NLMISC::ICommand::execute("getViewAES #.State="+fileContents, *NLMISC::InfoLog);
	}
}

static void cbAdminPong(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	for(uint i = 0; i < Services.size(); i++)
	{
		if(Services[i].ServiceId == sid)
		{
			nlinfo("receive pong");
			if(Services[i].LastPing == 0)
				nlwarning("Received a pong from service %s-%hu but we didn't expect a pong from it", serviceName.c_str(), sid.get());
			else
				Services[i].LastPing = 0;
			return;
		}
	}
	nlwarning("Received a pong from service %s-%hu that is not in my service list", serviceName.c_str(), sid.get());
}


//
// Request functions
//

void addRequestWaitingNb(uint32 rid)
{
	for (uint i = 0 ; i < Requests.size(); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting++;
			nldebug("REQUEST: ++ i %d rid %d NbWaiting+ %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			// if we add a waiting, reset the timer
			Requests[i].Time = CTime::getSecondsSince1970();
			return;
		}
	}
	nlwarning("addRequestWaitingNb: can't find the rid %d", rid);
}

void subRequestWaitingNb(uint32 rid)
{
	for (uint i = 0 ; i < Requests.size(); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting--;
			nldebug("REQUEST: ++ i %d rid %d NbWaiting- %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			return;
		}
	}
	nlwarning("subRequestWaitingNb: can't find the rid %d", rid);
}

void aesAddRequestAnswer(uint32 rid, const TAdminViewResult& answer)
{
	for (uint i = 0 ; i < Requests.size(); i++)
	{
		if (Requests[i].Id == rid)
		{
			for (uint t = 0; t < answer.size(); t++)
			{
				if (!answer[t].VarNames.empty() && answer[t].VarNames[0] == "__log")
				{	nlassert(answer[t].VarNames.size() == 1); }
				else
				{	nlassert(answer[t].VarNames.size() == answer[t].Values.size()); }
				Requests[i].Answers.push_back(SAdminViewRow(answer[t].VarNames, answer[t].Values));
			}
			Requests[i].NbReceived++;
			nldebug("REQUEST: ++ i %d rid %d NbWaiting %d NbReceived+ %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			return;
		}
	}
	// we received an unknown request, forget it
	nlwarning("Receive an answer for unknown request %d", rid);
}


void aesAddRequestAnswer(uint32 rid, TAdminViewVarNames& varNames, const TAdminViewValues& values)
{
	if (!varNames.empty() && varNames[0] == "__log")
	{	nlassert(varNames.size() == 1); }
	else
	{	nlassert(varNames.size() == values.size()); }

	for (uint i = 0 ; i < Requests.size(); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].Answers.push_back(SAdminViewRow(varNames, values));

			Requests[i].NbReceived++;
			nldebug("REQUEST: ++ i %d rid %d NbWaiting %d NbReceived+ %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);

			return;
		}
	}
	// we received an unknown request, forget it
	nlwarning("Receive an answer for unknown request %d", rid);
}

void cleanRequests()
{
	uint32 currentTime = CTime::getSecondsSince1970();

	// just a debug check
	for (uint t = 0 ; t < Requests.size(); t++)
	{
		uint32 NbWaiting = Requests[t].NbWaiting;
		uint32 NbReceived = Requests[t].NbReceived;

		uint32 NbRef = 0;
		for (uint j = 0; j < Services.size(); j++)
		{
			if (Services[j].Connected)
			{
				for (uint k = 0; k < Services[j].WaitingRequestId.size(); k++)
				{
					if(Services[j].WaitingRequestId[k] == Requests[t].Id)
					{
						NbRef++;
					}
				}
			}
		}
		nlinfo("REQUEST: Waiting request %d: NbRef %d NbWaiting %d NbReceived %d", Requests[t].Id, NbRef, NbWaiting, NbReceived);

		if (NbRef != NbWaiting - NbReceived)
		{
			nlwarning("REQUEST: **** i %d rid %d -> NbRef(%d) != NbWaiting(%d) - NbReceived(%d) ", t, Requests[t].Id, NbRef, NbWaiting, NbReceived);
		}
	}

	for (uint i = 0 ; i < Requests.size();)
	{
		// timeout
		if (currentTime >= Requests[i].Time+RequestTimeout)
		{
			nlwarning("REQUEST: Request %d timeouted, only %d on %d services have replied", Requests[i].Id, Requests[i].NbReceived, Requests[i].NbWaiting);

			TAdminViewVarNames varNames;
			TAdminViewValues values;

			varNames.push_back("service");
			for (uint j = 0; j < Services.size(); j++)
			{
				if (Services[j].Connected)
				{
					for (uint k = 0; k < Services[j].WaitingRequestId.size(); k++)
					{
						if(Services[j].WaitingRequestId[k] == Requests[i].Id)
						{
							// this services didn't answer
							string s;
							if(Services[j].AliasName.empty())
								s = Services[j].ShortName;
							else
								s = Services[j].AliasName;
							s += "-"+toString(Services[j].ServiceId);
							s += "((TIMEOUT))";
							values.clear();
							values.push_back(s);
							aesAddRequestAnswer(Requests[i].Id, varNames, values);
							break;
						}
					}
				}
			}
			if (Requests[i].NbWaiting != Requests[i].NbReceived)
			{
				nlwarning("REQUEST: **** i %d rid %d -> Requests[i].NbWaiting(%d) != Requests[i].NbReceived(%d)", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
				nlwarning("REQUEST: Need to add dummy answer");
				values.clear();
				values.push_back("UnknownService");
				while (Requests[i].NbWaiting != Requests[i].NbReceived)
					aesAddRequestAnswer(Requests[i].Id, varNames, values);
			}
		}

		if (Requests[i].NbWaiting <= Requests[i].NbReceived)
		{
			// the request is over, send to the php

			CMessage msgout("VIEW");
			msgout.serial(Requests[i].Id);

			for (uint j = 0; j < Requests[i].Answers.size(); j++)
			{
				msgout.serialCont(Requests[i].Answers[j].VarNames);
				msgout.serialCont(Requests[i].Answers[j].Values);
			}

			if (Requests[i].SId == TServiceId(0))
			{
				nlinfo("REQUEST: Receive an answer for the fake request %d with %d answers", Requests[i].Id, Requests[i].Answers.size());
				for (uint j = 0; j < Requests[i].Answers.size(); j++)
				{
					uint k;
					for (k = 0; k < Requests[i].Answers[j].VarNames.size(); k++)
					{
						InfoLog->displayRaw("%-20s ", Requests[i].Answers[j].VarNames[k].c_str());
					}
					InfoLog->displayRawNL("");
					for (k = 0; k < Requests[i].Answers[j].Values.size(); k++)
					{
						InfoLog->displayRaw("%-20s", Requests[i].Answers[j].Values[k].c_str());
					}
					InfoLog->displayRawNL("");
					InfoLog->displayRawNL("----------------------------------------------");
				}
			}
			else
				CUnifiedNetwork::getInstance()->send(Requests[i].SId, msgout);

			// set to 0 to erase it
			Requests[i].NbWaiting = 0;
			nldebug("REQUEST: ++ i %d rid %d NbWaiting0 %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
		}

		if (Requests[i].NbWaiting == 0)
		{
			Requests.erase(Requests.begin()+i);
		}
		else
		{
			i++;
		}
	}
}

//
// Functions
//

void findServices(const string &name, vector<TServices::iterator> &services)
{
	string alias, shortName;
	TServiceId sid;
	decodeUnifiedName(name, alias, shortName, sid);

	services.clear();

	// try to find if the name match an alias name
	TServices::iterator sit;
	for (sit = Services.begin(); sit != Services.end(); sit++)
	{
		if ((*sit).AliasName == alias)
		{
			services.push_back(sit);
		}
	}
	// if we found some alias, it's enough, return
	if(!services.empty())
		return;

	// not found in alias, try with short name
	for (sit = Services.begin(); sit != Services.end(); sit++)
	{
		if ((*sit).ShortName == shortName)
		{
			services.push_back(sit);
		}
	}
}

bool isRegisteredService(const string &name)
{
	string alias, shortName;
	TServiceId sid;
	decodeUnifiedName(name, alias, shortName, sid);

	for (uint i = 0; i != RegisteredServices.size(); i++)
		if (RegisteredServices[i] == alias)
			return true;
	return false;
}

TServices::iterator findService(TServiceId sid, bool asrt = true)
{
	TServices::iterator sit;
	for (sit = Services.begin(); sit != Services.end(); sit++)
	{
		if ((*sit).ServiceId == sid) break;
	}
	if (asrt)
		nlassert(sit != Services.end());
	return sit;
}

void treatRequestForRunnignService(uint32 rid, TServices::iterator sit, const string& viewStr)
{
	CVarPath subvarpath(viewStr);

	bool send = true;

	// check if the command is not to stop the service
	for (uint k = 0; k < subvarpath.Destination.size(); k++)
	{
		if (subvarpath.Destination[k].first == "State=0")
		{
			// If we stop the service, we don't have to reconnect the service
			sit->AutoReconnect = false;
			writeServiceLaunchCtrl(sit->ServiceId,true,LaunchCtrlStop);
		}
		else if (subvarpath.Destination[k].first == "State=-1")
		{
			sit->AutoReconnect = false;
			killProgram(sit->PId);
			send = false;
			writeServiceLaunchCtrl(sit->ServiceId,true,LaunchCtrlStop);
		}
		else if (subvarpath.Destination[k].first == "State=-2")
		{
			sit->AutoReconnect = false;
			abortProgram(sit->PId);
			send = false;
			writeServiceLaunchCtrl(sit->ServiceId,true,LaunchCtrlStop);
		}
		else if (subvarpath.Destination[k].first == "State=2")
		{
			sit->AutoReconnect = false;
			sit->Relaunch = true;
			writeServiceLaunchCtrl(sit->ServiceId,true,LaunchCtrlStart);
		}
		else if (subvarpath.Destination[k].first == "State=3")
		{
			sit->AutoReconnect = false;
			sit->Relaunch = true;
			killProgram(sit->PId);
			send = false;
			writeServiceLaunchCtrl(sit->ServiceId,true,LaunchCtrlStart);
		}
	}

	if (send)
	{
		// now send the request to the service
		addRequestWaitingNb(rid);
		sit->WaitingRequestId.push_back(rid);
		CMessage msgout("GET_VIEW");
		msgout.serial(rid);
		msgout.serial(const_cast<string&>(viewStr));
		nlassert(sit->ServiceId.get());
		CUnifiedNetwork::getInstance()->send(sit->ServiceId, msgout);
		nlinfo("REQUEST: Sent view '%s' to service '%s'", viewStr.c_str(), sit->toString().c_str());
	}
}

void treatRequestOneself(uint32 rid, const string& viewStr)
{
	addRequestWaitingNb(rid);
	TAdminViewResult answer;
	serviceGetView(rid, viewStr, answer);
	aesAddRequestAnswer(rid, answer);
	nlinfo("REQUEST: Treated view myself directly: '%s'", viewStr.c_str());
}

void treatRequestForOfflineService(uint32 rid, const string& serviceName, const string& viewStr)
{
	CVarPath subvarpath(viewStr);

	addRequestWaitingNb(rid);

	TAdminViewVarNames varNames;
	TAdminViewValues values;

	// add default row
	varNames.push_back("service");
	values.push_back(serviceName);

	for (uint k = 0; k < subvarpath.Destination.size(); k++)
	{
		size_t pos = subvarpath.Destination[k].first.find("=");
		if (pos != string::npos)
			varNames.push_back(subvarpath.Destination[k].first.substr(0, pos));
		else
			varNames.push_back(subvarpath.Destination[k].first);

		string val = "???";
		// handle special case of non running service
		if (subvarpath.Destination[k].first == "State")
		{
			// lookup the alias, command to execute, execution path and defined command args for the given service
			string alias, command, path, arg;
			getServiceLaunchInfo(serviceName,alias,command,path,arg);
			val = getOfflineServiceState(alias,path);
		}
		else if (subvarpath.Destination[k].first == "State=1" ||
				 subvarpath.Destination[k].first == "State=2" ||
				 subvarpath.Destination[k].first == "State=3")
		{
			// we want to start the service
			if (startService(serviceName))
				val = "Launching";
			else
				val = "Failed";
		}
		else if (subvarpath.Destination[k].first == "State=0" ||
				 subvarpath.Destination[k].first == "State=-1" ||
				 subvarpath.Destination[k].first == "State=-2")
		{
			// lookup the alias, command to execute, execution path and defined command args for the given service
			string alias, command, path, arg;
			bool ok= getServiceLaunchInfo(serviceName,alias,command,path,arg);
			if (ok) writeServiceLaunchCtrl(alias,path,true,LaunchCtrlStop);
			if (ok) writeServiceLaunchCtrl(alias,path,false,LaunchCtrlStop);
			val= "Stopping";
		}

		values.push_back(val);
	}

	aesAddRequestAnswer(rid, varNames, values);
	nlinfo("REQUEST: Sent and received view '%s' to offline service '%s'", viewStr.c_str(), serviceName.c_str());
}

void addRequestForOnlineServices(uint32 rid, const string& viewStr)
{
	// add services that I manage
	for (TServices::iterator sit= Services.begin(); sit!=Services.end(); ++sit)
	{
		if (sit->Connected)
		{
			treatRequestForRunnignService(rid,sit,viewStr);
		}
	}

	// add myself
	treatRequestOneself(rid,viewStr);
}

void addRequestForAllServices(uint32 rid, const string& viewStr)
{
	uint j;

	// add registered services that are not online
	for (j = 0; j < RegisteredServices.size(); j++)
	{
		vector<TServices::iterator> sits;
		findServices(RegisteredServices[j], sits);

		// check for service that is registered but not online
		if (sits.empty())
		{
			treatRequestForOfflineService(rid,RegisteredServices[j],viewStr);
		}
	}

	// add all running services (and for oneself)
	addRequestForOnlineServices(rid,viewStr);
}

void addRequestForNamedService(uint32 rid, const string& service, const string& viewStr)
{
	if (service.find("AES") != string::npos)
	{
		// it's for me, I don't send message to myself so i manage it right now
		treatRequestOneself(rid,viewStr);
	}
	else
	{
		// see if the service is running
		vector<TServices::iterator> sits;
		findServices(service, sits);
		if (sits.empty())
		{
			// the service is not running - so make sure it's registered
			if (!isRegisteredService(service))
			{
				nlwarning("Service %s is not online and not found in registered service list", service.c_str());
			}
			else
			{
				treatRequestForOfflineService(rid,service,viewStr);
			}
		}
		else
		{
			// there is at least one match from a running service for the given service name so treat the matches
			for(uint p = 0; p < sits.size(); p++)
			{
				treatRequestForRunnignService(rid, sits[p], viewStr);
			}
		}
	}
}

void addRequest(uint32 rid, const string &rawvarpath, TServiceId sid)
{
	nlinfo("REQUEST: addRequest from %hu: '%s'", sid.get(), rawvarpath.c_str());

	string str;
	CLog logDisplayVars;
	CMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer(&mdDisplayVars);

	CVarPath varpath(rawvarpath);

	// add the request
	Requests.push_back(CRequest(rid, sid));

	// for each destination in the rawvarpath...
	for (uint i = 0; i < varpath.Destination.size(); i++)
	{
		CVarPath vp(varpath.Destination[i].first);
		for (uint t = 0; t < vp.Destination.size(); t++)
		{
			string service = vp.Destination[t].first;

			if (service == "*")
			{
				addRequestForOnlineServices(rid,varpath.Destination[i].second);
			}
			else if (service == "#")
			{
				addRequestForAllServices(rid,varpath.Destination[i].second);
			}
			else
			{
				addRequestForNamedService(rid,service,varpath.Destination[i].second);
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE SERVICES //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cbServiceIdentification(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	if (sid.get() >= Services.size())
	{
		nlwarning("Identification of an unknown service %s-%hu", serviceName.c_str(), sid.get());
		return;
	}

	if (!Services[sid.get()].Connected)
	{
		nlwarning("Identification of an unknown service %s-%hu", serviceName.c_str(), sid.get());
		return;
	}

	msgin.serial(Services[sid.get()].AliasName, Services[sid.get()].LongName, Services[sid.get()].PId);
	msgin.serialCont(Services[sid.get()].Commands);
	nlinfo("Received service identification: Sid=%-3i Alias='%s' LongName='%s' ShortName='%s' PId=%u",sid.get(),Services[sid.get()].AliasName.c_str(),Services[sid.get()].LongName.c_str(),serviceName.c_str(),Services[sid.get()].PId);

	// if there's an alias, it means that it s me that launch the services, autoreconnect it
	if (!Services[sid.get()].AliasName.empty())
		Services[sid.get()].AutoReconnect = true;

	nlinfo("*:*:%d is identified to be '%s' '%s' pid:%d", Services[sid.get()].ServiceId.get(), Services[sid.get()].getServiceUnifiedName().c_str(), Services[sid.get()].LongName.c_str(), Services[sid.get()].PId);
}

static void cbServiceReady(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	if (sid.get() >= Services.size())
	{
		nlwarning("Ready of an unknown service %s-%hu", serviceName.c_str(), sid.get());
		return;
	}

	if (!Services[sid.get()].Connected)
	{
		nlwarning("Ready of an unknown service %s-%hu", serviceName.c_str(), sid.get());
		return;
	}

	nlinfo("*:*:%d is ready '%s'", Services[sid.get()].ServiceId.get(), Services[sid.get()].getServiceUnifiedName().c_str());
	Services[sid.get()].Ready = true;
}

static void getRegisteredServicesFromCfgFile()
{
	// look and see if we have a registered services entry in out cfg file
	CConfigFile::CVar* ptr= IService::getInstance()->ConfigFile.getVarPtr("RegisteredServices");
	if (ptr!=NULL)
	{
		// the variable exists so extract the service list...
		for (uint32 i=0;i<ptr->size();++i)
		{
			RegisteredServices.push_back(ptr->asString(i));
		}
	}
}

static void cbAESInfo(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	nlinfo("Updating all information for AES and hosted service");

	//
	// setup the list of all registered services
	//
	RegisteredServices.clear();
	// start with the service list that we've been sent
	msgin.serialCont(RegisteredServices);
	// append the registered services listed in our cfg file
	getRegisteredServicesFromCfgFile();

	//
	// receive the list of all alarms and graph update
	//
	msgin.serialCont(AllAdminAlarms);
	msgin.serialCont(AllGraphUpdates);

	// set our own alarms for this service
	setInformation(AllAdminAlarms, AllGraphUpdates);

	// now send alarms to all services
	for (uint j = 0; j < Services.size(); j++)
	{
		if (Services[j].Connected)
		{
			sendInformation(Services[j].ServiceId);
		}
	}
}

CVariable<string> ShardName("aes","ShardName","the shard name to send to the AS in explicit registration","local",0,true);
CVariable<bool> UseExplicitAESRegistration("aes","UseExplicitAESRegistration","flag to allow AES services to register explicitly if automatic registration fails",false,0,true);
static void cbRejected(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	if (!UseExplicitAESRegistration)
	{
		// receive a message that means that the AS doesn't want me
		string res;
		msgin.serial(res);
		nlwarning("AS rejected our connection: %s", res.c_str());
		IService::getInstance()->exit();
	}
	else
	{
		// we assume that the rejection message from the AS is not important... we'll try an explicit registration

		// lookup the machine's host name and prune off the domain name
		string server= CSString(::NLNET::CInetAddress::localHost().hostName()).splitTo('.');

		// setup the shard name from the config file vaiable
		string shard= ShardName;

		// setup the registration message and dispatch it to the AS
		nlinfo("Sending manual registration: server='%s' shard='%s' (auto-registration was rejected by AS)",server.c_str(),shard.c_str());
		CMessage msgout("REGISTER_AES");
		msgout.serial(server);
		msgout.serial(shard);
		CUnifiedNetwork::getInstance()->send(sid, msgout);
	}
}

static void cbView(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// receive an view answer from the service
	TServices::iterator sit = findService(sid);

	uint32 rid;
	msgin.serial(rid);

	TAdminViewResult answer;
	TAdminViewVarNames varNames;
	TAdminViewValues values;

	while ((uint32)msgin.getPos() < msgin.length())
	{
		varNames.clear();
		values.clear();

		// adding default row

		uint32 i, nb;
		string var, val;

		msgin.serial(nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial(var);
			varNames.push_back(var);
		}

		msgin.serial(nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial(val);
			values.push_back(val);
		}
		answer.push_back(SAdminViewRow(varNames,values));
	}
	aesAddRequestAnswer(rid, answer);

	// remove the waiting request
	for (uint i = 0; i < (*sit).WaitingRequestId.size();)
	{
		if ((*sit).WaitingRequestId[i] == rid)
		{
			(*sit).WaitingRequestId.erase((*sit).WaitingRequestId.begin()+i);
		}
		else
		{
			i++;
		}
	}
}


static void cbGetView(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32 rid;
	string rawvarpath;

	msgin.serial(rid);
	msgin.serial(rawvarpath);

	addRequest(rid, rawvarpath, sid);
}

void serviceConnection(const std::string &serviceName, TServiceId sid, void *arg)
{
	// don't add AS
	if (serviceName == "AS")
		return;

	if (sid.get() >= Services.size())
	{
		Services.resize(sid.get()+1);
	}

	Services[sid.get()].init(serviceName, sid);

	sendInformation(sid);

	nlinfo("%s-%hu connected", Services[sid.get()].ShortName.c_str(), Services[sid.get()].ServiceId.get());

}

void serviceDisconnection(const std::string &serviceName, TServiceId sid, void *arg)
{
	// don't remove AS
	if (serviceName == "AS")
		return;

	if (sid.get() >= Services.size())
	{
		nlwarning("Disconnection of an unknown service %s-%hu", serviceName.c_str(), sid.get());
		return;
	}

	if (!Services[sid.get()].Connected)
	{
		nlwarning("Disconnection of an unknown service %s-%hu", serviceName.c_str(), sid.get());
		return;
	}

	// we need to remove pending request

	for(uint i = 0; i < Services[sid.get()].WaitingRequestId.size(); i++)
	{
		subRequestWaitingNb(Services[sid.get()].WaitingRequestId[i]);
	}

	nlinfo("%s-%hu disconnected", Services[sid.get()].ShortName.c_str(), Services[sid.get()].ServiceId.get());

	if (Services[sid.get()].Relaunch)
	{
		// we have to relaunch it in time because ADMIN asked it
		sint32 delay = IService::getInstance()->ConfigFile.exists("RestartDelay")? IService::getInstance()->ConfigFile.getVar("RestartDelay").asInt(): 5;

		// must restart it so if delay is -1, set it by default to 5 seconds
		if (delay == -1) delay = 5;

		startService(delay, Services[sid.get()].getServiceUnifiedName());
	}
	else if (Services[sid.get()].AutoReconnect)
	{
		// we have to relaunch it
		sint32 delay = IService::getInstance()->ConfigFile.exists("RestartDelay")? IService::getInstance()->ConfigFile.getVar("RestartDelay").asInt(): 5;
		if (delay >= 0)
		{
			startService(delay, Services[sid.get()].getServiceUnifiedName());
		}
		else
			nlinfo("Don't restart the service because RestartDelay is %d", delay);

		sendAdminEmail("Server %s service '%s' : Stopped, auto reconnect in %d seconds", CInetAddress::localHost().hostName().c_str(), Services[sid.get()].getServiceUnifiedName().c_str(), delay);
	}

	// if the appropriate cfg file variable is set then we kill the service brutally on disconnection - this
	// allows one to fix a certain number of problem cases in program shut-down that would otherwise require
	// manual intervention on the machine
	if (KillServicesOnDisconnect)
	{
		killProgram(Services[sid.get()].PId);
	}

	Services[sid.get()].reset();
}


/** Callback Array
 */
static TUnifiedCallbackItem CallbackArray[] =
{
	// messages sent by services (see nel/src/net/admin.cpp)
	{ "SID", cbServiceIdentification },
	{ "ADMIN_PONG", cbAdminPong },

	// messages sent by services (see nel/src/net/admin.cpp) - for forwarding to admin service
	{ "VIEW", cbView },
	{ "ADMIN_EMAIL", cbAdminEmail },
	{ "GRAPH_UPDATE", cbGraphUpdate },

	// messages sent by services (see nel/src/net/admin.cpp AND nel/src/net/service.cpp)
	{ "SR", cbServiceReady },

	// messages sent by admin service (see nelns/admin_service/admin_service.cpp)
	{ "AES_INFO", cbAESInfo },
	{ "REJECTED", cbRejected },
	{ "AES_GET_VIEW", cbGetView },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AS ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ASConnection(const string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Connected to %s-%hu", serviceName.c_str(), sid.get());
}

static void ASDisconnection(const string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Disconnected to %s-%hu", serviceName.c_str(), sid.get());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// SERVICE IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAdminExecutorService : public IService
{
public:

	/// Init the service
	void init()
	{
		// be warn when a new service comes
		CUnifiedNetwork::getInstance()->setServiceUpCallback("*", serviceConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback("*", serviceDisconnection, NULL);

		// add connection to the admin service
		CUnifiedNetwork::getInstance()->setServiceUpCallback("AS", ASConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback("AS", ASDisconnection, NULL);

		// setup the connection address for the admin service
		string ASHost = ConfigFile.exists("ASHost")? ConfigFile.getVar("ASHost").asString(): "localhost";
		if (ASHost.find(":") == string::npos)
			ASHost += ":49996";

		// try to connect to the admin service (if we fail then NeL'll try again every few seconds until it succeeds...)
		try
		{
		  CUnifiedNetwork::getInstance()->addService("AS", CInetAddress(ASHost));
		}
		catch ( ESocket& e )
		{
		  nlerror( "Can't connect to AS: %s", e.what() );
		}

		// setup the default registered service list (from our cfg file)
		getRegisteredServicesFromCfgFile();

		// get rid of the ".state" file (if we have one) to make sure that external processes that wait for
		// shards to stop don't wait for us
		std::string stateFileName= getServiceStateFileName(IService::getServiceAliasName(),"./");
		if (NLMISC::CFile::fileExists(stateFileName))
		{
			NLMISC::CFile::deleteFile(stateFileName);
		}
	}

	bool update()
	{
		cleanRequests();
		checkWaitingServices();
		checkPingPong();
		checkShutdownRequest();
		return true;
	}
};

/// Admin executor Service
NLNET_SERVICE_MAIN(CAdminExecutorService, "AES", "admin_executor_service", 49997, CallbackArray, NELNS_CONFIG, NELNS_LOGS);

NLMISC_COMMAND(getViewAES, "send a view and receive an array as result", "<varpath>")
{
	string cmd;
	for (uint i = 0; i < args.size(); i++)
	{
		if (i != 0) cmd += " ";
		cmd += args[i];
	}

	static uint32 requestId=0;
	addRequest(requestId++, cmd, TServiceId(0));

	return true;
}

NLMISC_COMMAND(clearRequests, "clear all pending requests", "")
{
	if(args.size() != 0) return false;

	// for all request, set the NbWaiting to NbReceived, next cleanRequests() will send answer and clear all request
	for (uint i = 0 ; i < Requests.size(); i++)
	{
		if (Requests[i].NbWaiting <= Requests[i].NbReceived)
		{
			Requests[i].NbWaiting = Requests[i].NbReceived;
			nldebug("REQUEST: ++ i %d rid %d NbWaiting= %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
		}
	}

	return true;
}

NLMISC_COMMAND(displayRequests, "display all pending requests", "")
{
	if(args.size() != 0) return false;

	log.displayNL("Display %d pending requests", Requests.size());
	for (uint i = 0 ; i < Requests.size(); i++)
	{
		log.displayNL("id: %d wait: %d recv: %d sid: %hu", Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived, Requests[i].SId.get());
	}
	log.displayNL("End of display pending requests");

	return true;
}

NLMISC_COMMAND(sendAdminEmail, "Send an email to admin", "<text>")
{
	if(args.size() <= 0)
		return false;

	string text;
	for (uint i =0; i < args.size(); i++)
	{
		text += args[i]+" ";
	}
	sendAdminEmail(text.c_str());

	return true;
}



#ifdef NL_OS_UNIX

static inline char *skipToken(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}

// col: 0        1       2    3    4    5     6          7         8        9       10   11   12   13    14      15
//      receive                                                    sent
//      bytes    packets errs drop fifo frame compressed multicast bytes    packets errs drop fifo colls carrier compressed
uint64 getSystemNetwork(uint col)
{
	if (col > 15)
		return 0;

	int fd = open("/proc/net/dev", O_RDONLY);
	if (fd == -1)
	{
		nlwarning("Can't get OS from /proc/net/dev: %s", strerror(errno));
		return 0;
	}
	else
	{
		char buffer[4096+1];
		int len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		buffer[len] = '\0';

		char *p = strchr(buffer, '\n')+1;
		p = strchr(p, '\n')+1;

		uint64 val = 0;
		while (true)
		{
			p = strchr(p, ':');
			if (p == NULL)
				break;
			p++;
			for (uint i = 0; i < col; i++)
			{
				p = skipToken(p);
			}
			val += strtoul(p, &p, 10);
		}
		return val;
	}
}

NLMISC_DYNVARIABLE(string, NetBytesSent, "Amount of bytes sent to all networks cards in bytes")
{
	if (get) *pointer = bytesToHumanReadable(getSystemNetwork(8));
}

NLMISC_DYNVARIABLE(string, NetBytesReceived, "Amount of bytes received to all networks cards in bytes")
{
	if (get) *pointer = bytesToHumanReadable(getSystemNetwork(0));
}

NLMISC_DYNVARIABLE(uint32, NetError, "Number of error on all networks cards")
{
	if (get) *pointer = (uint32) (getSystemNetwork(2) + getSystemNetwork(10));
}

#endif // NL_OS_UNIX

NLMISC_COMMAND(aesSystem, "Execute a system() call", "<command>")
{
	if(args.size() <= 0)
		return false;

	string cmd;
	for (uint i =0; i < args.size(); i++)
	{
		cmd += args[i]+" ";
	}

	string path;
#ifdef NL_OS_UNIX
	path = "/tmp/";
#endif

	string fn = path+CFile::findNewFile("aessys.tmp");
	string fne = path+CFile::findNewFile("aessyse.tmp");

	cmd += " >" + fn + " 2>" + fne;

	log.displayNL("Executing: '%s' in directory '%s'", cmd.c_str(), CPath::getCurrentPath().c_str());

	system(cmd.c_str());

	char str[1024];

	FILE *fp = fopen(fn.c_str(), "rt");
	if (fp != NULL)
	{
		while (true)
		{
			char *res = fgets(str, 1023, fp);
			if (res == NULL)
				break;
			log.displayRaw(res);
		}

		fclose(fp);
	}
	else
	{
		log.displayNL("No stdout");
	}

	fp = fopen(fne.c_str(), "rt");
	if (fp != NULL)
	{
		while (true)
		{
			char *res = fgets(str, 1023, fp);
			if (res == NULL)
				break;
			log.displayRaw(res);
		}

		fclose(fp);
	}
	else
	{
		log.displayNL("No stderr");
	}

	CFile::deleteFile(fn);
	CFile::deleteFile(fne);

	return true;
}



CMakeLogTask MakingLogTask;


NLMISC_COMMAND( makeLogReport, "Build a report of logs produced on the machine", "[stop | <logpath>]" )
{


	bool start = args.empty() || (!args.empty() && args[0] != "stop");

	if (start)
	{

		if (!args.empty())
		{
			MakingLogTask.setLogPath(args[0]);
		}
		else
		{
			MakingLogTask.setLogPathToDefault();
		}

		if ( ! MakingLogTask.isRunning() )
		{
			MakingLogTask.start();
			log.displayNL( "Task started" );
		}
		else
		{
			log.displayNL( "The makeLogReport task is already in progress, wait until the end of call 'makeLogReport stop' to terminate it");
		}
	}
	else if ( args[0] == "stop" )
	{
		if ( MakingLogTask.isRunning() )
		{
			nlinfo( "Stopping makeLogReport task..." );
			MakingLogTask.terminateTask();
			log.displayNL( "Task stopped" );
		}
		else
			log.displayNL( "Task is not running" );
	}

	return true;
}

NLMISC_COMMAND( displayLogReport, "Display summary of a part of the log report built by makeLogReport",
			    "[<service id> | <-p page number>]" )
{
	if ( MakingLogTask.isRunning() )
	{
		//uint currentFile, totalFiles;
		//MainLogReport.getProgress( currentFile, totalFiles );
		log.displayNL( "Please wait until the completion of the makeLogReport task, or stop it" );
		//log.displayNL( "Currently processing file %u of %u", currentFile, totalFiles );
		return true;
	}
	log.displayNL( MakingLogTask.isComplete() ? "Full stats:" : "Temporary stats" );
	if ( args.empty() )
	{
		// Summary
		//MainLogReport.report( &log );
	}
	else if ( (! args[0].empty()) && (args[0] == "-p") )
	{
		// By page
		if ( args.size() < 2 )
		{
			log.displayNL( "Page number missing" );
			return false;
		}
		//uint pageNum = atoi( args[1].substr( 1 ).c_str() );
		//MainLogReport.reportPage( pageNum, &log );
	}
	else
	{
		// By service
		//MainLogReport.reportByService( args[0], &log );
	}

	return true;
}

/*
 * Command to allow AES to create a file with content
 */
NLMISC_COMMAND( createFile, "Create a file and fill it with given content", "<filename> [<content>]" )
{
	// check args
	if (args.size() < 1 || args.size() > 2)
		return false;

	COFile	f;
	if (!f.open(args[0]))
	{
		log.displayNL("Failed to open file '%s' in write mode", args[0].c_str());
		return false;
	}

	// check something to write, in case serialBuffer() doesn't accept empty buffer
	if (args.size() == 2)
	{
		// dirty const cast, but COFile won't erase buffer content
		char*	buffer = const_cast<char*>(args[1].c_str());

		f.serialBuffer((uint8*)buffer, args[1].size());
	}

	f.close();

	return true;
}

/*
 * Command for testing purposes - allows us to add an entry to the 'registered services' container
 */
NLMISC_COMMAND( addRegisteredService, "Add an entry to the registered services container", "<service alias name>" )
{
	// check args
	if (args.size() != 1 )
		return false;

	RegisteredServices.push_back(args[0]);

	return true;
}
