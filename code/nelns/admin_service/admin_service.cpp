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
#include "config.h"
#endif // HAVE_CONFIG_H

#ifndef NELNS_CONFIG
#define NELNS_CONFIG ""
#endif // NELNS_CONFIG

#ifndef NELNS_LOGS
#define NELNS_LOGS ""
#endif // NELNS_LOGS

#include "nel/misc/types_nl.h"

#include <string>
#include <list>

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
#include <windows.h>
typedef unsigned long ulong;
#endif

#include <mysql.h>
#include <mysql_version.h>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"

#include "nel/net/service.h"
#include "nel/net/varpath.h"
#include "nel/net/email.h"

#include "connection_web.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// NeL Variables (for config file, etc)
//

// this variable should be used in conjunction with UseExplicitAESRegistration.
// the AS / AES registration process works as follows:
// - aes creates a layer 5 connection to as
// - as gets a serviceUp callback and looks in the database to try to find a match for the AES
// - if the match fails then AS sends a reject message to the AES
// - when the AES receives the reject message they check their UseExplicitAESRegistration flag - if it's set they
//   attempt an explicit connection, sending the info required by the AS that would normally come from the database
// - when the AS receives an explicit registration, it verifies the state of the AllowExplicitAESRegistration flag
//   and completes the registration work that failed earlier due to the database access failure
CVariable<bool> AllowExplicitAESRegistration("as","AllowExplicitAESRegistration","flag to allow AES services to register explicitly",false,0,true);

// this variable allows one to launch an AS on a machine that doesn't have a database setup
// the functionality of the AS is reduced (particularly in respect to alarms and graphs which are configured via the database)
CVariable<bool> DontUseDataBase("as","DontUseDataBase","if this flag is set calls to the database will be ignored",false,0,true);


//
// Structures
//

struct CRequest
{
	CRequest (uint32 id, TSockId from) : Id(id), NbWaiting(0), NbReceived(0), From(from), NbRow(0), NbLines(1)
	{
		Time = CTime::getSecondsSince1970 ();
	}

	uint32			Id;
	uint			NbWaiting;
	uint32			NbReceived;
	TSockId			From;
	uint32			Time;	// when the request was ask
	
	uint32			NbRow;
	uint32			NbLines;

	vector<vector<string> > Array;	// it's the 2 dimensional array that will be send to the php for variables
	vector<string> Log;				// this log contains the answer if a command was asked, othewise, Array contains the results

	uint32 getVariable(const string &variable)
	{
		for (uint32 i = 0; i < NbRow; i++)
			if (Array[i][0] == variable)
				return i;

		// need to add the variable
		vector<string> NewRow;
		NewRow.resize (NbLines);
		NewRow[0] = variable;
		Array.push_back (NewRow);
		return NbRow++;
	}

	void addLine ()
	{
		for (uint32 i = 0; i < NbRow; i++)
			Array[i].push_back("");

		NbLines++;
	}

	void display ()
	{
		if (Log.empty())
		{
			nlinfo ("Display answer array for request %d: %d row %d lines", Id, NbRow, NbLines);
			for (uint i = 0; i < NbLines; i++)
			{
				for (uint j = 0; j < NbRow; j++)
				{
					nlassert (Array.size () == NbRow);
					InfoLog->displayRaw ("%-20s", Array[j][i].c_str());
				}
				InfoLog->displayRawNL ("");
			}
			InfoLog->displayRawNL ("End of the array");
		}
		else
		{
			nlinfo ("Display the log for request %d: %d lines", Id, Log.size());
			for (uint i = 0; i < Log.size(); i++)
			{
				InfoLog->displayRaw ("%s", Log[i].c_str());
			}
			InfoLog->displayRawNL ("End of the log");
		}
	}
};

struct CAdminExecutorService
{
	CAdminExecutorService (const string &shard, const string &name, TServiceId sid) : Shard(shard), SId(sid), Name(name) { }

	string  Shard;			/// Name of the shard
	TServiceId	SId;			/// uniq number to identify the AES
	string	Name;			/// name of the admin executor service

	vector<uint32>	WaitingRequestId;		/// contains all request that the server hasn't reply yet

};

typedef list<CAdminExecutorService> TAdminExecutorServices;
typedef list<CAdminExecutorService>::iterator AESIT;


//
// Variables
//

TAdminExecutorServices AdminExecutorServices;

MYSQL *DatabaseConnection = NULL;

vector<CRequest> Requests;

uint32 RequestTimeout = 5;	// in second

// cumulate 5 seconds of alert
sint32 AdminAlertAccumlationTime = 5;


//
// Functions
//

AESIT findAES (TServiceId sid, bool asrt = true)
{
	AESIT aesit;
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		if ((*aesit).SId == sid)
			break;
		
		if (asrt)
			nlassert (aesit != AdminExecutorServices.end());
		return aesit;
}

AESIT findAES (const string &name, bool asrt = true)
{
	AESIT aesit;
	for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		if ((*aesit).Name == name)
			break;
		
		if (asrt)
			nlassert (aesit != AdminExecutorServices.end());
		
		return aesit;
}


//
// SQL helpers
//

MYSQL_RES *sqlCurrentQueryResult = NULL;

MYSQL_ROW sqlQuery (const char *format, ...)
{
	if (DontUseDataBase)
		return 0;

	char *query;
	NLMISC_CONVERT_VARGS (query, format, 1024);
	
	if (DatabaseConnection == 0)
	{
		nlwarning ("MYSQL: mysql_query (%s) failed: DatabaseConnection is 0", query);
		return NULL;
	}
	
	int ret = mysql_query (DatabaseConnection, query);
	if (ret != 0)
	{
		nlwarning ("MYSQL: mysql_query () failed for query '%s': %s", query,  mysql_error(DatabaseConnection));
		return 0;
	}
	
	sqlCurrentQueryResult = mysql_store_result(DatabaseConnection);
	if (sqlCurrentQueryResult == 0)
	{
		nlwarning ("MYSQL: mysql_store_result () failed for query '%s': %s", query,  mysql_error(DatabaseConnection));
		return 0;
	}
	
	MYSQL_ROW row = mysql_fetch_row(sqlCurrentQueryResult);
	if (row == 0)
	{
		nlwarning ("MYSQL: mysql_fetch_row () failed for query '%s': %s", query,  mysql_error(DatabaseConnection));
	}
	
	nldebug ("MYSQL: sqlQuery(%s) returns %d rows", query, mysql_num_rows(sqlCurrentQueryResult));
	
	return row;	
}

MYSQL_ROW sqlNextRow ()
{
	if (DontUseDataBase)
		return 0;

	if (sqlCurrentQueryResult == 0)
		return 0;
	
	return mysql_fetch_row(sqlCurrentQueryResult);
}

void	sqlFlushResult()
{
	if (DontUseDataBase)
		return;

	if (sqlCurrentQueryResult == NULL)
		return;

	mysql_free_result(sqlCurrentQueryResult);
	sqlCurrentQueryResult = NULL;
}


//
// Admin functions
//

string Email;
uint32 FirstEmailTime = 0;

void sendAdminAlert (const char *format, ...)
{
	char *text;
	NLMISC_CONVERT_VARGS (text, format, 4096);

	if (AdminAlertAccumlationTime == -1)
	{
		// we don't send email so just display a warning
		nlwarning ("ALERT: %s", text);
	}
	else
	{
		if(Email.empty() && FirstEmailTime == 0)
		{
			Email += text;
			FirstEmailTime = CTime::getSecondsSince1970();
		}
		else
		{
			Email += "\n";
			Email += text;
		}
		nldebug ("ALERT: pushing email into queue: %s", text);
	}
}

void updateSendAdminAlert ()
{
	if(!Email.empty() && FirstEmailTime != 0 && AdminAlertAccumlationTime >=0 && CTime::getSecondsSince1970() > FirstEmailTime + AdminAlertAccumlationTime)
	{
		vector<string> lines;
		explode (Email, string("\n"), lines, true);

		if (!lines.empty())
		{

			if (IService::getInstance()->ConfigFile.exists("SysLogPath") && IService::getInstance()->ConfigFile.exists("SysLogParams"))
			{
				// syslog
				string param;
				if (lines.size() > 1)
				{
					param = "Multiple problems, first is: ";
				}
				param += lines[0];
				string res = toString(IService::getInstance()->ConfigFile.getVar("SysLogParams").asString().c_str(), param.c_str());
				launchProgram(IService::getInstance()->ConfigFile.getVar("SysLogPath").asString(), res);
			}

			if (IService::getInstance()->ConfigFile.exists("AdminEmail"))
			{
				// email
				string subject;
				if (lines.size() == 1)
				{
					subject = lines[0];
				}
				else
				{
					subject = "Multiple problems";
				}
				
				std::string from;
				if(IService::getInstance()->ConfigFile.exists("AdminEmailFrom"))
					from = IService::getInstance()->ConfigFile.getVar("AdminEmailFrom").asString();
				CConfigFile::CVar &var = IService::getInstance()->ConfigFile.getVar("AdminEmail");
				for (uint i = 0; i < var.size(); i++)
				{
					if (!sendEmail ("", from, var.asString(i), subject, Email))
					{
						nlwarning ("Can't send email to '%s'", var.asString(i).c_str());
					}
					else
					{
						nlinfo ("ALERT: Sent email to admin %s the subject: %s", var.asString(i).c_str(), subject.c_str());
					}
				}
			}
		}

		Email = "";
		FirstEmailTime = 0;
	}
}


static void cbAdminEmail (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	string str;
	msgin.serial(str);
	sendAdminAlert (str.c_str());
}

static void cbGraphUpdate (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32 CurrentTime;
	msgin.serial (CurrentTime);

	while (msgin.getPos() < (sint32)msgin.length())
	{
		string var, service;
		sint32 val;
		msgin.serial (service, var, val);

		AESIT aesit = findAES (sid);
		
		string shard, server;
		shard = (*aesit).Shard;
		server = (*aesit).Name;
		
		if (!shard.empty() && !server.empty() && !service.empty() && !var.empty())
		{
			string path = CPath::standardizePath (IService::getInstance()->ConfigFile.getVar("RRDVarPath").asString());
			string rrdfilename = path + shard+"."+server+"."+service+"."+var+".rrd";

			string arg;
			
			if (!NLMISC::CFile::fileExists(rrdfilename))
			{
				MYSQL_ROW row = sqlQuery ("select graph_update from variable where path like '%%%s' and graph_update!=0", var.c_str());
				if (row != NULL)
				{
					uint32 freq = atoi(row[0]);
					arg = "create "+rrdfilename+" --step "+toString(freq)+" DS:var:GAUGE:"+toString(freq*2)+":U:U RRA:AVERAGE:0.5:1:1000 RRA:AVERAGE:0.5:10:1000 RRA:AVERAGE:0.5:100:1000";
					launchProgram(IService::getInstance()->ConfigFile.getVar("RRDToolPath").asString(), arg);
				}
				else
				{
					nlwarning ("Can't create the rrd because no graph_update in database");
				}
				sqlFlushResult();
			}

			arg = "update " + rrdfilename + " " + toString (CurrentTime) + ":" + toString(val);
			launchProgram(IService::getInstance()->ConfigFile.getVar("RRDToolPath").asString(), arg);
		}
		else
		{
			nlwarning ("Shard server service var val is empty");
		}
	}
}


//
// Request functions
//

uint32 newRequest (TSockId from)
{
	static uint32 NextId = 5461231;

	Requests.push_back (CRequest(NextId, from));

	return NextId++;
}

void addRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting++;
			Requests[i].Time = CTime::getSecondsSince1970 ();
			return;
		}
	}
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
}

void subRequestWaitingNb (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbWaiting--;
			return;
		}
	}
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
}

void addRequestReceived (uint32 rid)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].Id == rid)
		{
			Requests[i].NbReceived++;
			return;
		}
	}
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
}

void addRequestAnswer (uint32 rid, const vector<string> &variables, const vector<string> &values)
{
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		Requests[i].addLine ();
		if (Requests[i].Id == rid)
		{
			if (!variables.empty() && variables[0]=="__log")
			{
				nlassert (variables.size() == 1);

				for (uint j = 0; j < values.size(); j++)
				{
					Requests[i].Log.push_back (values[j]);
				}
			}
			else
			{
				nlassert (variables.size() == values.size ());
				for (uint j = 0; j < variables.size(); j++)
				{
					uint32 pos = Requests[i].getVariable (variables[j]);
					Requests[i].Array[pos][Requests[i].NbLines-1] = values[j];
				}
			}
			return;
		}
	}
	nlwarning ("REQUEST: Received an answer from an unknown resquest %d (perhaps due to a AS timeout)", rid);
}

bool emptyRequest (uint32 rid)
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

void cleanRequest ()
{
	uint32 currentTime = CTime::getSecondsSince1970 ();

	bool timeout;

	for (uint i = 0 ; i < Requests.size ();)
	{
		// the AES doesn't answer quickly
		timeout = (currentTime >= Requests[i].Time+RequestTimeout);

		if (Requests[i].NbWaiting <= Requests[i].NbReceived || timeout)
		{
			// the request is over, send to the php

			string str;

			if (timeout)
			{
				nlwarning ("REQUEST: Request %d timeouted, only %d on %d services have replied", Requests[i].Id, Requests[i].NbReceived, Requests[i].NbWaiting);
			}
			
			if (Requests[i].Log.empty())
			{
				if (Requests[i].NbRow == 0 && timeout)
				{
					str = "1 ((TIMEOUT))";
				}
				else
				{
					str = toString(Requests[i].NbRow) + " ";
					for (uint k = 0; k < Requests[i].NbLines; k++)
					{
						for (uint j = 0; j < Requests[i].NbRow; j++)
						{
							nlassert (Requests[i].Array.size () == Requests[i].NbRow);
							if (Requests[i].Array[j][k].empty ())
								str += "??? ";
							else
							{
								str += Requests[i].Array[j][k];
								if (timeout)
									str += "((TIMEOUT))";
								str += " ";
							}
						}
					}
				}
			}
			else
			{
				for (uint k = 0; k < Requests[i].Log.size(); k++)
				{
					str += Requests[i].Log[k];
					if (timeout)
						str += "((TIMEOUT))";
				}
			}

			sendString (Requests[i].From, str);

			// set to 0 to erase it
			Requests[i].NbWaiting = 0;
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


//
// SQL functions
//

void sqlInit ()
{
	if (DontUseDataBase)
		return;

	MYSQL *db = mysql_init(NULL);
	if(db == NULL)
	{
		nlerror ("mysql_init() failed");
	}

	my_bool opt = true;
	if (mysql_options (db, MYSQL_OPT_RECONNECT, &opt))
	{
		mysql_close(db);
		DatabaseConnection = 0;
		nlerror("mysql_options() failed for database connection to '%s'", IService::getInstance()->ConfigFile.getVar("DatabaseHost").asString().c_str());
 		return;
	}

	DatabaseConnection = mysql_real_connect(db,
		IService::getInstance()->ConfigFile.getVar("DatabaseHost").asString().c_str(),
		IService::getInstance()->ConfigFile.getVar("DatabaseLogin").asString().c_str(),
		IService::getInstance()->ConfigFile.getVar("DatabasePassword").asString().c_str(),
		IService::getInstance()->ConfigFile.getVar("DatabaseName").asString().c_str(),
		0,NULL,0);
	if (DatabaseConnection == NULL || DatabaseConnection != db)
	{
		nlerror ("mysql_real_connect() failed to '%s' with login '%s' and database name '%s' with %s",
			IService::getInstance()->ConfigFile.getVar("DatabaseHost").asString().c_str(),
			IService::getInstance()->ConfigFile.getVar("DatabaseLogin").asString().c_str(),
			IService::getInstance()->ConfigFile.getVar("DatabaseName").asString().c_str(),
			(IService::getInstance()->ConfigFile.getVar("DatabasePassword").asString().empty()?"empty password":"password")
			);
	}

#if MYSQL_VERSION_ID < 50019
	opt = true;
	if (mysql_options (DatabaseConnection, MYSQL_OPT_RECONNECT, &opt))
	{
		mysql_close(db);
		DatabaseConnection = 0;
		nlerror("mysql_options() failed for database connection to '%s'", IService::getInstance()->ConfigFile.getVar("DatabaseHost").asString().c_str());
 		return;
	}
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE AES ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void sendAESInformation (TServiceId sid)
{
	AESIT aesit = findAES (sid);

	vector<string> information;

	CMessage msgout("AES_INFO");
	
	//
	// send services that should be running on this AES
	//
	information.clear ();
	MYSQL_ROW row = sqlQuery ("select name from service where server='%s'", (*aesit).Name.c_str());
	while (row != NULL)
	{
		string service = row[0];
		nlinfo ("Adding '%s' in registered services to AES-%hu", row[0], sid.get());
		information.push_back (service);
		row = sqlNextRow ();
	}
	sqlFlushResult();
	msgout.serialCont (information);

	//
	// send variable alarms for services that should running on this AES
	//
	information.clear ();
	row = sqlQuery ("select path, error_bound, alarm_order from variable where error_bound!=-1");
	while (row != NULL)
	{
		nlinfo ("Adding '%s' '%s' '%s' in alarm to AES-%hu", row[0], row[1], row[2], sid.get());
		information.push_back (row[0]);
		information.push_back (row[1]);
		information.push_back (row[2]);
		row = sqlNextRow ();
	}
	sqlFlushResult();
	msgout.serialCont (information);
	
	//
	// send graph update for services that should running on this AES
	//
	information.clear ();
	row = sqlQuery ("select path, graph_update from variable where graph_update!=0");
	while (row != NULL)
	{
		CVarPath varpath (row[0]);

		for(uint i = 0; i < varpath.Destination.size(); i++)
		{
			string a  = varpath.Destination[i].first, b = (*aesit).Shard;
			if(varpath.Destination[i].first == "*" || varpath.Destination[i].first == (*aesit).Shard)
			{
				CVarPath varpath2 (varpath.Destination[i].second);

				for(uint j = 0; j < varpath2.Destination.size(); j++)
				{
					string c  = varpath2.Destination[j].first, d = (*aesit).Name;
					if(varpath2.Destination[j].first == "*" || varpath2.Destination[j].first == (*aesit).Name)
					{
						nlinfo ("Adding '%s' '%s' in graph to AES-%hu", row[0], row[1], sid.get());
						information.push_back (row[0]);
						information.push_back (row[1]);
					}
				}
			}
		}
		row = sqlNextRow ();
	}
	sqlFlushResult();
	msgout.serialCont (information);
	
	nlinfo ("Sending all information about %s AES-%hu (hostedservices, alarms,grapupdate)", (*aesit).Name.c_str(), (*aesit).SId.get());
	CUnifiedNetwork::getInstance ()->send (sid, msgout);
}

void rejectAES(TServiceId sid, const string &res)
{
	CMessage msgout("REJECTED");
	msgout.serial ((string &)res);
	CUnifiedNetwork::getInstance ()->send (sid, msgout);
}

// i'm connected to a new admin executor service
static void cbNewAESConnection (const std::string &serviceName, TServiceId sid, void *arg)
{
	TSockId from;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (sid, from);
	const CInetAddress &ia = cnb->hostAddress (from);

	AESIT aesit = findAES (sid, false);

	if (aesit != AdminExecutorServices.end ())
	{
		nlwarning ("Connection of an AES that is already in the list (%s)", ia.asString ().c_str ());
		rejectAES (sid, "This AES is already in the AS list");
		return;
	}

	MYSQL_ROW row = sqlQuery ("select name from server where address='%s'", ia.ipAddress().c_str());
	if (row == NULL)
	{
		if (!AllowExplicitAESRegistration)
		{
			nlwarning ("Connection of an AES that is not in database server list (%s)", ia.asString ().c_str ());
		}
		else
		{
			nlinfo ("Rejecting auto-connection of an AES (%s) - this should provke explicitly reconnect", ia.asString ().c_str ());
		}
		rejectAES (sid, "This AES is not registered in the database");
		sqlFlushResult();
		return;
	}
	string server = row[0];
	sqlFlushResult();

	row = sqlQuery ("select shard from service where server='%s'", server.c_str());
	if (row == NULL)
	{
		nlwarning ("Connection of an AES that is not in database server list (%s)", ia.asString ().c_str ());
		rejectAES (sid, "This AES is not registered in the database");
		sqlFlushResult();
		return;
	}
	string shard = row[0];
	sqlFlushResult();
	
	AdminExecutorServices.push_back (CAdminExecutorService(shard, server, sid));

	nlinfo ("%s-%hu, server name %s, for shard %s connected and added in the list", serviceName.c_str(), sid.get(), server.c_str(), shard.c_str());
	
	// send him services that should run on this server
	sendAESInformation (sid);
}

// i'm disconnected from an admin executor service
static void cbNewAESDisconnection (const std::string &serviceName, TServiceId sid, void *arg)
{
	TSockId from;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance ()->getNetBase (sid, from);
	const CInetAddress &ia = cnb->hostAddress (from);

	AESIT aesit = findAES (sid, false);

	if (aesit == AdminExecutorServices.end ())
	{
		nlwarning ("Disconnection of %s-%hu that is not in my list (%s)", serviceName.c_str (), sid.get(), ia.asString ().c_str ());
		return;
	}

	nlinfo ("%s-%hu, shard name %s, disconnected and removed from the list", serviceName.c_str(), sid.get(), (*aesit).Name.c_str ());

	// we need to remove pending request

	for(uint i = 0; i < (*aesit).WaitingRequestId.size (); i++)
	{
		subRequestWaitingNb ((*aesit).WaitingRequestId[i]);
	}

	AdminExecutorServices.erase (aesit);
}

// we receive an explicit registration message from an AES
void cbRegisterAES(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	if (!AllowExplicitAESRegistration)
	{
		nlwarning("Ignoring attempted AES registration because AllowExplicitAESRegistration==false");
		return;
	}

	string server;
	string shard;
	try
	{
		msgin.serial(server);
		msgin.serial(shard);
	}
	catch(...)
	{
		nlwarning("Ignoring attempted AES registration due to execption during message decoding");
		return;
	}

	AdminExecutorServices.push_back (CAdminExecutorService(shard, server, sid));

	nlinfo ("%s-%hu, server name %s, for shard %s connected and added in the list", serviceName.c_str(), sid.get(), server.c_str(), shard.c_str());
	
	// send him services that should run on this server
	sendAESInformation (sid);
}

static void cbView (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32 rid;
	msgin.serial (rid);

	AESIT aesit = findAES (sid);

	for (uint i = 0; i < (*aesit).WaitingRequestId.size();)
	{
		if ((*aesit).WaitingRequestId[i] == rid)
		{
			(*aesit).WaitingRequestId.erase ((*aesit).WaitingRequestId.begin ()+i);
		}
		else
		{
			i++;
		}
	}

	MYSQL_ROW row = sqlQuery ("select distinct shard from service where server='%s'", (*aesit).Name.c_str ());

	// shard name is find using the "service" table, so, if there s no shard name in it, it returns ???
	string shardName;
	if (row != NULL) shardName = row[0];
	else shardName = DontUseDataBase? aesit->Shard: "???";

	vector<string> vara, vala;

	while ((uint32)msgin.getPos() < msgin.length())
	{
		vara.clear ();
		vala.clear ();

		// adding default row
		vara.push_back ("shard");
		vara.push_back ("server");

		vala.push_back (shardName);
		vala.push_back ((*aesit).Name);

		uint32 i, nb;
		string var, val;

		msgin.serial (nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial (var);
			if (var == "__log")
			{
				vara.clear ();
				vala.clear ();
			}
			vara.push_back (var);
		}

		if (vara.size() > 0 && vara[0] == "__log")
			vala.push_back ("----- Result from Shard "+shardName+" Server "+(*aesit).Name+"\n");

		msgin.serial (nb);
		for (i = 0; i < nb; i++)
		{
			msgin.serial (val);
			vala.push_back (val);
		}
		addRequestAnswer (rid, vara, vala);
	}
	sqlFlushResult();

	// inc the NbReceived counter
	addRequestReceived (rid);
}

TUnifiedCallbackItem CallbackArray[] =
{
	{ "REGISTER_AES", cbRegisterAES },
	{ "VIEW", cbView },
	{ "ADMIN_EMAIL", cbAdminEmail },
	{ "GRAPH_UPDATE", cbGraphUpdate },
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// CONNECTION TO THE CLIENT ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void addRequest (const string &rawvarpath, TSockId from)
{
	nlinfo ("addRequest from %s: '%s'", from->asString ().c_str (), rawvarpath.c_str ());

	if(rawvarpath.empty ())
	{
		// send an empty string to say to php that there's nothing
		string str;
		sendString (from, str);
	}

	//
	// special cases
	//

	if(rawvarpath == "reload")
	{
		// it means the we have to resend the list of services managed by AES from the mysql tables
		for (AESIT aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
		{
			sendAESInformation ((*aesit).SId);
		}

		// send an empty string to say to php that there's nothing
		string str;
		sendString (from, str);
		return;
	}

	//
	// normal cases
	//

	CVarPath varpath (rawvarpath);

	uint32 rid = newRequest (from);

	for (uint i = 0; i < varpath.Destination.size (); i++)
	{
		string shard = varpath.Destination[i].first;

		CVarPath subvarpath (varpath.Destination[i].second);

		for (uint j = 0; j < subvarpath.Destination.size (); j++)
		{
			string server = subvarpath.Destination[j].first;

			if (shard == "*" && server == "*")
			{
				// Send the request to all online servers of all online shards

				AESIT aesit;
				for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
				{
					addRequestWaitingNb (rid);
					(*aesit).WaitingRequestId.push_back (rid);

					CMessage msgout("AES_GET_VIEW");
					msgout.serial (rid);
					msgout.serial (subvarpath.Destination[j].second);
					CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
					nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId.get());
				}
			}
			else if (shard == "*" && server == "#")
			{
				// Select all shard all server including offline one
				
				MYSQL_ROW row = sqlQuery ("select distinct server, shard from service");
				
				while (row != NULL)
				{
					AESIT aesit = findAES (row[0], false);
					
					if (aesit != AdminExecutorServices.end())
					{
						addRequestWaitingNb (rid);
						(*aesit).WaitingRequestId.push_back (rid);
						
						CMessage msgout("AES_GET_VIEW");
						msgout.serial (rid);
						msgout.serial (subvarpath.Destination[j].second);
						CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
						nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId.get());
						
					}
					else if (server == "#")
					{
						vector<string> vara, vala;
						
						// adding default row
						vara.push_back ("shard");
						vala.push_back (row[1]);

						vara.push_back ("server");
						vala.push_back (row[0]);

						vara.push_back ("service");
						vala.push_back ("AES");
						
						vara.push_back ("State");
						vala.push_back ("Offline");
						
						addRequestAnswer (rid, vara, vala);
					}
					row = sqlNextRow ();
				}
				sqlFlushResult();
			}
			else if (server == "*" || server == "#")
			{
				// Send the request to all online server of a specific shard

				MYSQL_ROW row = sqlQuery ("select distinct server from service where shard='%s'", shard.c_str ());

				while (row != NULL)
				{
					AESIT aesit = findAES (row[0], false);

					if (aesit != AdminExecutorServices.end())
					{
						addRequestWaitingNb (rid);
						(*aesit).WaitingRequestId.push_back (rid);

						CMessage msgout("AES_GET_VIEW");
						msgout.serial (rid);
						msgout.serial (subvarpath.Destination[j].second);
						CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
						nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId.get());

					}
					else if (server == "#")
					{
						vector<string> vara, vala;
						
						// adding default row
						vara.push_back ("shard");
						vala.push_back (shard);

						vara.push_back ("server");
						vala.push_back (row[0]);
						
						vara.push_back ("service");
						vala.push_back ("AES");
						
						vara.push_back ("State");
						vala.push_back ("Offline");

						addRequestAnswer (rid, vara, vala);
					}
					row = sqlNextRow ();
				}

				sqlFlushResult();
			}
			else
			{
				AESIT aesit = findAES (server, false);

				if (aesit != AdminExecutorServices.end())
				{
					addRequestWaitingNb (rid);
					(*aesit).WaitingRequestId.push_back (rid);

					CMessage msgout("AES_GET_VIEW");
					msgout.serial (rid);
					msgout.serial (subvarpath.Destination[j].second);
					CUnifiedNetwork::getInstance ()->send ((*aesit).SId, msgout);
					nlinfo ("REQUEST: Sent view '%s' to shard name %s 'AES-%hu'", subvarpath.Destination[j].second.c_str(), (*aesit).Name.c_str(), (*aesit).SId.get());
				}
				else
				{
					nlwarning ("Server %s is not found in the list", server.c_str ());
				}
			}
		}
	}
}

static void varRequestTimeout(CConfigFile::CVar &var)
{
	RequestTimeout = var.asInt();
	nlinfo ("Request timeout is now after %d seconds", RequestTimeout);
}

static void varAdminAlertAccumlationTime (CConfigFile::CVar &var)
{
	AdminAlertAccumlationTime = var.asInt();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// SERVICE IMPLEMENTATION //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAdminService : public IService
{
public:

	/// Init the service, load the universal time.
	void init ()
	{
		setDefaultEmailParams (ConfigFile.getVar ("SMTPServer").asString (), ConfigFile.getVar("DefaultEmailFrom").asString(), "");

		sqlInit ();

		connectionWebInit ();

		//CVarPath toto ("[toto");

		//CVarPath toto ("*.*.*.*");
		//CVarPath toto ("[srv1,srv2].*.*.*");
		//CVarPath toto ("[svr1.svc1,srv2.svc2].*.*");
		//CVarPath toto ("[svr1.[svc1,svc2].*.var1,srv2.svc2.fe*.var2].toto");
		//CVarPath toto ("[svr1.svc1.*.toto,srv2.svc2.*.tata]");

		CUnifiedNetwork::getInstance ()->setServiceUpCallback ("AES", cbNewAESConnection);
		CUnifiedNetwork::getInstance ()->setServiceDownCallback ("AES", cbNewAESDisconnection);

		varRequestTimeout (ConfigFile.getVar ("RequestTimeout"));
		ConfigFile.setCallback("RequestTimeout", &varRequestTimeout);
		
		varAdminAlertAccumlationTime (ConfigFile.getVar ("AdminAlertAccumlationTime"));
		ConfigFile.setCallback("AdmimAlertAccumlationTime", &varAdminAlertAccumlationTime);
		
	}

	bool update ()
	{
		cleanRequest ();
		connectionWebUpdate ();
		
		updateSendAdminAlert ();
		return true;
	}

	void release ()
	{
		connectionWebRelease ();
	}
};


/// Admin Service
NLNET_SERVICE_MAIN (CAdminService, "AS", "admin_service", 49996, CallbackArray, NELNS_CONFIG, NELNS_LOGS);


NLMISC_COMMAND (getViewAS, "send a view and receive an array as result", "<varpath>")
{
	string cmd;
	for (uint i = 0; i < args.size(); i++)
	{
		if (i != 0) cmd += " ";
		cmd += args[i];
	}

	addRequest (cmd, NULL);

	return true;
}

NLMISC_COMMAND (clearRequests, "clear all pending requests", "")
{
	if(args.size() != 0) return false;

	// for all request, set the NbWaiting to NbReceived, next cleanRequest() will send answer and clear all request
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		if (Requests[i].NbWaiting <= Requests[i].NbReceived)
		{
			Requests[i].NbWaiting = Requests[i].NbReceived;
		}
	}

	return true;
}

NLMISC_COMMAND (displayRequests, "display all pending requests", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display %d pending requests", Requests.size ());
	for (uint i = 0 ; i < Requests.size (); i++)
	{
		log.displayNL ("id: %d wait: %d recv: %d from: %s nbrow: %d", Requests[i].Id, Requests[i].NbWaiting, Requests[i].NbReceived, Requests[i].From->asString ().c_str (), Requests[i].NbRow);
	}
	log.displayNL ("End of display pending requests");

	return true;
}

NLMISC_COMMAND (generateAlert, "generate an alert", "<text>")
{
	if(args.size() != 1) return false;
	
	sendAdminAlert (args[0].c_str());

	return true;
}
