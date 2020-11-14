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


//
// Includes
//

#include "stdnet.h"

#include "nel/net/service.h"
#include "nel/net/admin.h"
#include "nel/net/varpath.h"


//
// Namspaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


namespace NLNET {


//
// Structures
//

struct CRequest
{
	CRequest (uint32 id, TServiceId sid) : Id(id), NbWaiting(0), NbReceived(0), SId(sid)
	{
		nldebug ("ADMIN: ++ NbWaiting %d NbReceived %d", NbWaiting, NbReceived);
		Time = CTime::getSecondsSince1970 ();
	}

	uint32			Id;
	uint			NbWaiting;
	uint32			NbReceived;
	TServiceId			SId;
	uint32			Time;	// when the request was ask

	TAdminViewResult Answers;
};


//
// Variables
//

TRemoteClientCallback RemoteClientCallback = 0;

vector<CAlarm> Alarms;

vector<CGraphUpdate> GraphUpdates;

// check alarms every 5 seconds
const uint32 AlarmCheckDelay = 5;

vector<CRequest> Requests;

uint32 RequestTimeout = 4;	// in second


//
// Callbacks
//

static void cbInfo (CMessage &msgin, const std::string &/* serviceName */, TServiceId /* sid */)
{
	nlinfo ("ADMIN: Updating admin information");

	vector<string> alarms;
	msgin.serialCont (alarms);
	vector<string> graphupdate;
	msgin.serialCont (graphupdate);

	setInformation (alarms, graphupdate);
}

static void cbServGetView (CMessage &msgin, const std::string &/* serviceName */, TServiceId sid)
{
	uint32 rid;
	string rawvarpath;

	msgin.serial (rid);
	msgin.serial (rawvarpath);

	Requests.push_back (CRequest(rid, sid));

	TAdminViewResult answer;
	// just send the view in async mode, don't retrieve the answer
	serviceGetView (rid, rawvarpath, answer, true);
	nlassert (answer.empty());
}

static void cbExecCommand (CMessage &msgin, const std::string &/* serviceName */, TServiceId sid)
{
	// create a displayer to gather the output of the command
	class CStringDisplayer: public IDisplayer
	{
	public:
		void serial(NLMISC::IStream &stream)
		{
			stream.serial(_Data);
		}

	protected:
		virtual void doDisplay( const CLog::TDisplayInfo& /* args */, const char *message)
		{
			_Data += message;
		}

		std::string _Data;
	};
	CStringDisplayer stringDisplayer;
	IService::getInstance()->CommandLog.addDisplayer(&stringDisplayer);

	// retreive the command from the input message and execute it
	string command;
	msgin.serial (command);
	nlinfo ("ADMIN: Executing command from network : '%s'", command.c_str());
	ICommand::execute (command, IService::getInstance()->CommandLog);

	// unhook our displayer as it's work is now done
	IService::getInstance()->CommandLog.removeDisplayer(&stringDisplayer);

	// send a reply message to the originating service
	CMessage msgout("EXEC_COMMAND_RESULT");
	stringDisplayer.serial(msgout);
	CUnifiedNetwork::getInstance()->send(sid, msgout);
}


// AES wants to know if i'm not dead, I have to answer faster as possible or i'll be killed
static void cbAdminPing (CMessage &/* msgin */, const std::string &/* serviceName */, TServiceId sid)
{
	// Send back a pong to say to the AES that I'm alive
	CMessage msgout("ADMIN_PONG");
	CUnifiedNetwork::getInstance()->send(sid, msgout);
}

static void cbStopService (CMessage &/* msgin */, const std::string &serviceName, TServiceId sid)
{
	nlinfo ("ADMIN: Receive a stop from service %s-%hu, need to quit", serviceName.c_str(), sid.get());
	IService::getInstance()->exit (0xFFFF);
}


void cbAESConnection (const string &/* serviceName */, TServiceId /* sid */, void * /* arg */)
{
	// established a connection to the AES, identify myself

	//
	// Sends the identification message with the name of the service and all commands available on this service
	//

	nlinfo("cbAESConnection: Identifying self as: AliasName='%s' LongName='%s' PId=%u",
		IService::getInstance()->_AliasName.c_str(),
		IService::getInstance()->_LongName.c_str(),
		getpid ());
	CMessage msgout ("SID");
	uint32 pid = getpid ();
	msgout.serial (IService::getInstance()->_AliasName, IService::getInstance()->_LongName, pid);
	ICommand::serialCommands (msgout);
	CUnifiedNetwork::getInstance()->send("AES", msgout);

	if (IService::getInstance()->_Initialized)
	{
		CMessage msgout2 ("SR");
		CUnifiedNetwork::getInstance()->send("AES", msgout2);
	}
}


static void cbAESDisconnection (const std::string &serviceName, TServiceId sid, void * /* arg */)
{
	nlinfo("Lost connection to the %s-%hu", serviceName.c_str(), sid.get());
}


static TUnifiedCallbackItem CallbackArray[] =
{
	{ "INFO",			cbInfo },
	{ "GET_VIEW",		cbServGetView },
	{ "STOPS",			cbStopService },
	{ "EXEC_COMMAND",	cbExecCommand },
	{ "ADMIN_PING",		cbAdminPing },
};


//
// Functions
//

void setRemoteClientCallback (TRemoteClientCallback cb)
{
	RemoteClientCallback = cb;
}


//
// Request functions
//

static void addRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting++;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting+ %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			// if we add a waiting, reset the timer
			Requests[i].Time = CTime::getSecondsSince1970 ();
			return;
		}
	}
	nlwarning ("ADMIN: addRequestWaitingNb: can't find the rid %d", rid);
}

/*
static void subRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting--;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting- %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			return;
		}
	}
	nlwarning ("ADMIN: subRequestWaitingNb: can't find the rid %d", rid);
}
*/

void addRequestAnswer (uint32 rid, const TAdminViewVarNames& varNames, const TAdminViewValues& values)
{
	if (!varNames.empty() && varNames[0] == "__log")
	{	nlassert (varNames.size() == 1); }
	else
	{	nlassert (varNames.size() == values.size()); }

	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].Answers.push_back (SAdminViewRow(varNames, values));

			Requests[i].NbReceived++;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting %d NbReceived+ %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);

			return;
		}
	}
	// we received an unknown request, forget it
	nlwarning ("ADMIN: Receive an answer for unknown request %d", rid);
}

/*
static bool emptyRequest (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid && Requests[i].NbWaiting != 0)
		{
			return false;
		}
	}
	return true;
}
*/

static void cleanRequest ()
{
	uint32 currentTime = CTime::getSecondsSince1970 ();

	for (uint i = 0 ; i < Requests.size ();)
	{
		// timeout
		if (currentTime >= Requests[i].Time+RequestTimeout)
		{
			nlwarning ("ADMIN: **** i %d rid %d -> Requests[i].NbWaiting (%d) != Requests[i].NbReceived (%d)", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
			Requests[i].NbWaiting = Requests[i].NbReceived;
		}

		if (Requests[i].NbWaiting <= Requests[i].NbReceived)
		{
			// the request is over, send to the php

			CMessage msgout("VIEW");
			msgout.serial (Requests[i].Id);

			for (uint j = 0; j < Requests[i].Answers.size (); j++)
			{
				msgout.serialCont (Requests[i].Answers[j].VarNames);
				msgout.serialCont (Requests[i].Answers[j].Values);
			}

			if (Requests[i].SId.get() == 0)
			{
				nlinfo ("ADMIN: Receive an answer for the fake request %d with %d answers", Requests[i].Id, Requests[i].Answers.size ());
				for (uint j = 0; j < Requests[i].Answers.size (); j++)
				{
					uint k;
					for (k = 0; k < Requests[i].Answers[j].VarNames.size(); k++)
					{
						InfoLog->displayRaw ("%-10s", Requests[i].Answers[j].VarNames[k].c_str());
					}
					InfoLog->displayRawNL("");
					for (k = 0; k < Requests[i].Answers[j].Values.size(); k++)
					{
						InfoLog->displayRaw ("%-10s", Requests[i].Answers[j].Values[k].c_str());
					}
					InfoLog->displayRawNL("");
					InfoLog->displayRawNL("-------------------------");
				}
			}
			else
			{
				nlinfo ("ADMIN: The request is over, send the result to AES");
				CUnifiedNetwork::getInstance ()->send (Requests[i].SId, msgout);
			}

			// set to 0 to erase it
			Requests[i].NbWaiting = 0;
			nldebug ("ADMIN: ++ i %d rid %d NbWaiting0 %d NbReceived %d", i, Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived);
		}

		if (Requests[i].NbWaiting == 0)
		{
			Requests.erase (Requests.begin ()+i);
		}
		else
		{
			i++;
		}
	}
}

// all remote command start with rc or RC
bool isRemoteCommand(string &str)
{
	if (str.size()<2) return false;
	return tolower(str[0]) == 'r' && tolower(str[1]) == 'c';
}


// this callback is used to create a view for the admin system
void serviceGetView (uint32 rid, const string &rawvarpath, TAdminViewResult &answer, bool async)
{
	string str;
	CLog logDisplayVars;
	CLightMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer (&mdDisplayVars);
	mdDisplayVars.setParam (4096);

	CVarPath varpath(rawvarpath);

	if (varpath.empty())
		return;

	// special case for named command handler
	if (CCommandRegistry::getInstance().isNamedCommandHandler(varpath.Destination[0].first))
	{
		varpath.Destination[0].first += "."+varpath.Destination[0].second;
		varpath.Destination[0].second.clear();
	}

	if (varpath.isFinal())
	{
		TAdminViewVarNames varNames;
		TAdminViewValues values;

		// add default row
		varNames.push_back ("service");
		values.push_back (IService::getInstance ()->getServiceUnifiedName());

		for (uint j = 0; j < varpath.Destination.size (); j++)
		{
			string cmd = varpath.Destination[j].first;

			// replace = with space to execute the command
			string::size_type eqpos = cmd.find("=");
			if (eqpos != string::npos)
			{
				cmd[eqpos] = ' ';
				varNames.push_back(cmd.substr(0, eqpos));
			}
			else
				varNames.push_back(cmd);

			mdDisplayVars.clear ();
			ICommand::execute(cmd, logDisplayVars, !ICommand::isCommand(cmd));
			const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();

			if (ICommand::isCommand(cmd))
			{
				// we want the log of the command
				if (j == 0)
				{
					varNames.clear ();
					varNames.push_back ("__log");
					values.clear ();
				}

				values.push_back ("----- Result from "+IService::getInstance()->getServiceUnifiedName()+" of command '"+cmd+"'\n");
				for (uint k = 0; k < strs.size(); k++)
				{
					values.push_back (strs[k]);
				}
			}
			else
			{

				if (!strs.empty())
				{
					str = strs[0].substr(0,strs[0].size()-1);
					// replace all spaces into udnerscore because space is a reserved char
					for (uint i = 0; i < str.size(); i++) if (str[i] == ' ') str[i] = '_';
				}
				else
				{
					str = "???";
				}
				values.push_back (str);
				nlinfo ("ADMIN: Add to result view '%s' = '%s'", varpath.Destination[j].first.c_str(), str.c_str());
			}
			mdDisplayVars.unlockStrings();
		}

		if (!async)
			answer.push_back (SAdminViewRow(varNames, values));
		else
		{
			addRequestWaitingNb (rid);
			addRequestAnswer (rid, varNames, values);
		}
	}
	else
	{
		// there s an entity in the varpath, manage this case

		TAdminViewVarNames *varNames=0;
		TAdminViewValues *values=0;

		// varpath.Destination		contains the entity number
		// subvarpath.Destination	contains the command name

		for (uint i = 0; i < varpath.Destination.size (); i++)
		{
			CVarPath subvarpath(varpath.Destination[i].second);

			for (uint j = 0; j < subvarpath.Destination.size (); j++)
			{
				// set the variable name
				string cmd = subvarpath.Destination[j].first;

				if (isRemoteCommand(cmd))
				{
					if (async && RemoteClientCallback != 0)
					{
						// ok we have to send the request to another side, just send and wait
						addRequestWaitingNb (rid);
						RemoteClientCallback (rid, cmd, varpath.Destination[i].first);
					}
				}
				else
				{
					// replace = with space to execute the command
					string::size_type eqpos = cmd.find("=");
					if (eqpos != string::npos)
					{
						cmd[eqpos] = ' ';
						// add the entity
						cmd.insert(eqpos, " "+varpath.Destination[i].first);
					}
					else
					{
						// add the entity
						cmd += " "+varpath.Destination[i].first;
					}

					mdDisplayVars.clear ();
					ICommand::execute(cmd, logDisplayVars, true);
					const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
					for (uint k = 0; k < strs.size(); k++)
					{
						const string &str = strs[k];

						string::size_type pos = str.find(" ");
						if(pos == string::npos)
							continue;

						string entity = str.substr(0, pos);
						string value = str.substr(pos+1, str.size()-pos-2);
						for (uint u = 0; u < value.size(); u++) if (value[u] == ' ') value[u] = '_';

						// look in the array if we already have something about this entity

						if (!async)
						{
							uint y;
							for (y = 0; y < answer.size(); y++)
							{
								if (answer[y].Values[1] == entity)
								{
									// ok we found it, just push_back new stuff
									varNames = &(answer[y].VarNames);
									values = &(answer[y].Values);
									break;
								}
							}
							if (y == answer.size ())
							{
								answer.push_back (SAdminViewRow());

								varNames = &(answer[answer.size()-1].VarNames);
								values = &(answer[answer.size()-1].Values);

								// don't add service if we want an entity
		// todo when we work on entity, we don't need service name and server so we should remove them and collapse all var for the same entity
								varNames->push_back ("service");
								string name = IService::getInstance ()->getServiceUnifiedName();
								values->push_back (name);

								// add default row
								varNames->push_back ("entity");
								values->push_back (entity);
							}

							varNames->push_back (cmd.substr(0, cmd.find(" ")));
							values->push_back (value);
						}
						else
						{
							addRequestWaitingNb (rid);

							TAdminViewVarNames varNames;
							TAdminViewValues values;
							varNames.push_back ("service");
							string name = IService::getInstance ()->getServiceUnifiedName();
							values.push_back (name);

							// add default row
							varNames.push_back ("entity");
							values.push_back (entity);

							varNames.push_back (cmd.substr(0, cmd.find(" ")));
							values.push_back (value);

							addRequestAnswer (rid, varNames, values);
						}
						nlinfo ("ADMIN: Add to result view for entity '%s', '%s' = '%s'", varpath.Destination[i].first.c_str(), subvarpath.Destination[j].first.c_str(), str.c_str());
					}
					mdDisplayVars.unlockStrings();
				}
			}
		}
	}
}


//
// Alarms functions
//

void sendAdminEmail (const char *format, ...)
{
	char *text;
	NLMISC_CONVERT_VARGS (text, format, 4096);

	time_t t = time (&t);

	string str;
	str  = asctime (localtime (&t));
	str += " Server " + IService::getInstance()->getHostName();
	str += " service " + IService::getInstance()->getServiceUnifiedName();
	str += " : ";
	str += text;

	CMessage msgout("ADMIN_EMAIL");
	msgout.serial (str);
	if(IService::getInstance ()->getServiceShortName()=="AES")
		CUnifiedNetwork::getInstance ()->send ("AS", msgout);
	else
		CUnifiedNetwork::getInstance ()->send ("AES", msgout);

	nlinfo ("ADMIN: Forwarded email to AS with '%s'", str.c_str());
}

void initAdmin (bool dontUseAES)
{
	if (!dontUseAES)
	{
		CUnifiedNetwork::getInstance()->setServiceUpCallback ("AES", cbAESConnection, NULL);
		CUnifiedNetwork::getInstance()->setServiceDownCallback ("AES", cbAESDisconnection, NULL);
		CUnifiedNetwork::getInstance()->addService ("AES", CInetAddress("localhost:49997"));
	}
	CUnifiedNetwork::getInstance()->addCallbackArray (CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));
}


void updateAdmin()
{
	uint32 CurrentTime = CTime::getSecondsSince1970();


	//
	// check admin requests
	//

	cleanRequest ();


	//
	// Check graph updates
	//

	static uint32 lastGraphUpdateCheck = 0;

	if (CurrentTime >= lastGraphUpdateCheck+1)
	{
		string str;
		CLog logDisplayVars;
		CLightMemDisplayer mdDisplayVars;
		logDisplayVars.addDisplayer (&mdDisplayVars);

		lastGraphUpdateCheck = CurrentTime;

		CMessage msgout ("GRAPH_UPDATE");
		bool empty = true;
		for (uint j = 0; j < GraphUpdates.size(); j++)
		{
			if (CurrentTime >= GraphUpdates[j].LastUpdate + GraphUpdates[j].Update)
			{
				// have to send a new update for this var
				ICommand::execute(GraphUpdates[j].Name, logDisplayVars, true, false);
				const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
				sint32 val;
				if (strs.size() != 1)
				{
					nlwarning ("ADMIN: The graph update command execution not return exactly 1 line but %d", strs.size());
					for (uint i = 0; i < strs.size(); i++)
					  nlwarning ("ADMIN: line %d: '%s'", i, strs[i].c_str());
					val = 0;
				}
				else
				{
					fromString(strs[0], val);
				}
				mdDisplayVars.unlockStrings ();
				mdDisplayVars.clear ();

				string name = IService::getInstance()->getServiceAliasName();
				if (name.empty())
					name = IService::getInstance()->getServiceShortName();

				if(empty)
					msgout.serial (CurrentTime);

				msgout.serial (name);
				msgout.serial (GraphUpdates[j].Name);
				msgout.serial (val);

				empty = false;

				GraphUpdates[j].LastUpdate = CurrentTime;
			}
		}

		if(!empty)
		{
			if(IService::getInstance ()->getServiceShortName()=="AES")
				CUnifiedNetwork::getInstance ()->send ("AS", msgout);
			else
				CUnifiedNetwork::getInstance ()->send ("AES", msgout);
		}
	}


	//
	// Check alarms
	//

	static uint32 lastAlarmsCheck = 0;

	if (CurrentTime >= lastAlarmsCheck+AlarmCheckDelay)
	{
		string str;
		CLog logDisplayVars;
		CLightMemDisplayer mdDisplayVars;
		logDisplayVars.addDisplayer (&mdDisplayVars);

		lastAlarmsCheck = CTime::getSecondsSince1970();

		for (uint i = 0; i < Alarms.size(); )
		{
			mdDisplayVars.clear ();
			ICommand::execute(Alarms[i].Name, logDisplayVars, true, false);
			const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();

			if (!strs.empty())
			{
				str = strs[0].substr(0,strs[0].size()-1);
			}
			else
			{
				str = "???";
			}

			mdDisplayVars.unlockStrings();

			if (str == "???")
			{
				// variable doesn't exist, remove it from alarms
				nlwarning ("ADMIN: Alarm problem: variable '%s' returns ??? instead of a good value", Alarms[i].Name.c_str());
				Alarms.erase (Alarms.begin()+i);
			}
			else
			{
				// compare the value
				uint32 err = Alarms[i].Limit;
				uint32 val = humanReadableToBytes(str);
				if (Alarms[i].GT && val >= err)
				{
					if (!Alarms[i].Activated)
					{
						nlinfo ("ADMIN: VARIABLE TOO BIG '%s' %u >= %u", Alarms[i].Name.c_str(), val, err);
						Alarms[i].Activated = true;
						sendAdminEmail ("Alarm: Variable %s is %u that is greater or equal than the limit %u", Alarms[i].Name.c_str(), val, err);
					}
				}
				else if (!Alarms[i].GT && val <= err)
				{
					if (!Alarms[i].Activated)
					{
						nlinfo ("ADMIN: VARIABLE TOO LOW '%s' %u <= %u", Alarms[i].Name.c_str(), val, err);
						Alarms[i].Activated = true;
						sendAdminEmail ("Alarm: Variable %s is %u that is lower or equal than the limit %u", Alarms[i].Name.c_str(), val, err);
					}
				}
				else
				{
					if (Alarms[i].Activated)
					{
						nlinfo ("ADMIN: variable is ok '%s' %u %s %u", Alarms[i].Name.c_str(), val, (Alarms[i].GT?"<":">"), err);
						Alarms[i].Activated = false;
					}
				}

				i++;
			}
		}
	}
}

void setInformation (const vector<string> &alarms, const vector<string> &graphupdate)
{
	uint i;
	sint tmp;

	// add only commands that I understand
	Alarms.clear ();
	for (i = 0; i < alarms.size(); i+=3)
	{
		CVarPath shardvarpath (alarms[i]);
		if(shardvarpath.Destination.empty() || shardvarpath.Destination[0].second.empty())
			continue;
		CVarPath servervarpath (shardvarpath.Destination[0].second);
		if(servervarpath.Destination.empty() || servervarpath.Destination[0].second.empty())
			continue;
		CVarPath servicevarpath (servervarpath.Destination[0].second);
		if(servicevarpath.Destination.empty() || servicevarpath.Destination[0].second.empty())
			continue;

		string name = servicevarpath.Destination[0].second;

		if (IService::getInstance()->getServiceUnifiedName().find(servicevarpath.Destination[0].first) != string::npos && ICommand::exists(name))
		{
			fromString(alarms[i+1], tmp);
			nlinfo ("ADMIN: Adding alarm '%s' limit %d order %s (varpath '%s')", name.c_str(), tmp, alarms[i+2].c_str(), alarms[i].c_str());
			Alarms.push_back(CAlarm(name, tmp, alarms[i+2]=="gt"));
		}
		else
		{
			if (IService::getInstance()->getServiceUnifiedName().find(servicevarpath.Destination[0].first) == string::npos)
			{
				nlinfo ("ADMIN: Skipping alarm '%s' limit %d order %s (varpath '%s') (not for my service, i'm '%s')", name.c_str(), fromString(alarms[i+1], tmp) ? tmp:tmp, alarms[i+2].c_str(), alarms[i].c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
			}
			else
			{
				nlinfo ("ADMIN: Skipping alarm '%s' limit %d order %s (varpath '%s') (var not exist)", name.c_str(), fromString(alarms[i+1], tmp) ? tmp:tmp, alarms[i+2].c_str(), alarms[i].c_str());
			}
		}
	}

	// do the same with graph update
	GraphUpdates.clear ();
	for (i = 0; i < graphupdate.size(); i+=2)
	{
		CVarPath shardvarpath (graphupdate[i]);
		if(shardvarpath.Destination.empty() || shardvarpath.Destination[0].second.empty())
			continue;
		CVarPath servervarpath (shardvarpath.Destination[0].second);
		if(servervarpath.Destination.empty() || servervarpath.Destination[0].second.empty())
			continue;
		CVarPath servicevarpath (servervarpath.Destination[0].second);
		if(servicevarpath.Destination.empty() || servicevarpath.Destination[0].second.empty())
			continue;

		string VarName = servicevarpath.Destination[0].second;
		string ServiceName = servicevarpath.Destination[0].first;

		if (ICommand::exists(VarName) && (ServiceName == "*" || IService::getInstance()->getServiceShortName() == ServiceName))
		{
			fromString(graphupdate[i+1], tmp);
			nlinfo ("ADMIN: Adding graphupdate '%s' update %d (varpath '%s')", VarName.c_str(), tmp, graphupdate[i].c_str());
			GraphUpdates.push_back(CGraphUpdate(VarName, tmp));
		}
		else
		{
			if (IService::getInstance()->getServiceShortName() != ServiceName)
			{
				nlinfo ("ADMIN: Skipping graphupdate '%s' limit %d (varpath '%s') (not for my service, i'm '%s')", VarName.c_str(), fromString(graphupdate[i+1], tmp) ? tmp:tmp, graphupdate[i].c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
			}
			else
			{
				nlinfo ("ADMIN: Skipping graphupdate '%s' limit %d (varpath '%s') (var not exist)", VarName.c_str(), fromString(graphupdate[i+1], tmp) ? tmp:tmp, graphupdate[i].c_str());
			}
		}
	}
}

//
// Commands
//

NLMISC_CATEGORISED_COMMAND(nel, displayInformation, "displays all admin information", "")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(args);
	nlunreferenced(quiet);
	nlunreferenced(human);

	uint i;

	log.displayNL("There're %d alarms:", Alarms.size());
	for (i = 0; i < Alarms.size(); i++)
	{
		log.displayNL(" %d %s %d %s %s", i, Alarms[i].Name.c_str(), Alarms[i].Limit, (Alarms[i].GT?"gt":"lt"), (Alarms[i].Activated?"on":"off"));
	}
	log.displayNL("There're %d graphupdate:", GraphUpdates.size());
	for (i = 0; i < GraphUpdates.size(); i++)
	{
		log.displayNL(" %d %s %d %d", i, GraphUpdates[i].Name.c_str(), GraphUpdates[i].Update, GraphUpdates[i].LastUpdate);
	}
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, getView, "send a view and receive an array as result", "<varpath>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 1) return false;

	TAdminViewResult answer;
	serviceGetView (0, args[0], answer);

	log.displayNL("have %d answer", answer.size());
	for (uint i = 0; i < answer.size(); i++)
	{
		log.displayNL("  have %d value", answer[i].VarNames.size());

		nlassert (answer[i].VarNames.size() == answer[i].Values.size());

		for (uint j = 0; j < answer[i].VarNames.size(); j++)
		{
			log.displayNL("    %s -> %s", answer[i].VarNames[j].c_str(), answer[i].Values[j].c_str());
		}
	}

	return true;
}

} // NLNET
