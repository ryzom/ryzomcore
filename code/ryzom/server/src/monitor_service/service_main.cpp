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

#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_version.h"
#include "game_share/crypt.h"
#include "nel/misc/time_nl.h"

#include "client.h"
#include "mirrors.h"
#include "messages.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <WinSock2.h>
#	include <Windows.h>
typedef unsigned long ulong;
#endif // NL_OS_WINDOWS

#include <mysql.h>

using namespace NLMISC;
using namespace NLNET;
using namespace std;

bool EGSHasMirrorReady = false;
bool IOSHasMirrorReady = false;
bool EmulateShard = false;

std::map<TYPE_NAME_STRING_ID, std::string>	StringMap;
std::set<TYPE_NAME_STRING_ID>				StringAsked;

// Must be a pointer to control when to start listening socket and when stop it
CCallbackServer *Server;

// Dates at which bad login msg should be sent
std::multimap<NLMISC::TTime, CRefPtr<CMonitorClient> > BadLoginClients;


const NLMISC::TTime LOGIN_RETRY_DELAY_IN_MILLISECONDS = 4000;

// Code taken from login service
/**
 * Encapsulation of MySQL result
 */
class CMySQLResult
{
public:

	/// Constructor
	CMySQLResult(MYSQL_RES* res = NULL);
	/// Constructor
	CMySQLResult(MYSQL* database);
	/// Destructor
	~CMySQLResult();

	/// Cast operator
	operator MYSQL_RES*();

	/// Affectation
	CMySQLResult&	operator = (MYSQL_RES* res);


	/// Test success
	bool			success() const;
	/// Test failure
	bool			failed() const;



	/// Number of rows of result
	uint			numRows();
	/// Number of fields  of result
	uint			numFields();
	/// Fetch row
	MYSQL_ROW		fetchRow();


private:

	MYSQL_RES*	_Result;

};
//
// Constructor
CMySQLResult::CMySQLResult(MYSQL_RES* res)
{
	_Result = res;
}

/// Constructor
CMySQLResult::CMySQLResult(MYSQL* database)
{ 
	_Result = mysql_store_result(database);
}
/// Destructor
CMySQLResult::~CMySQLResult()
{
	if (_Result != NULL)
		mysql_free_result(_Result);
}

/// Cast operator
CMySQLResult::operator MYSQL_RES*()
{
	return _Result;
}

/// Affectation
CMySQLResult&	CMySQLResult::operator = (MYSQL_RES* res)
{
	if (res == _Result)
		return *this;
	if (_Result != NULL)
		mysql_free_result(_Result);
	_Result = res;
	return *this;
}


/// Test success
bool			CMySQLResult::success() const
{
	return _Result != NULL;
}
/// Test failure
bool			CMySQLResult::failed() const
{
	return !success();
}



/// Number of rows of result
uint			CMySQLResult::numRows()
{
	return (uint)mysql_num_rows(_Result);
}

uint			CMySQLResult::numFields()
{
	return (uint)mysql_num_fields(_Result);
}


/// Fetch row
MYSQL_ROW		CMySQLResult::fetchRow()
{
	return mysql_fetch_row(_Result);
}




/* ***************************************************************************
	Doc :

	When an entity is added in the service mirror, the service checks if its name is in the name cache. If 
	the string ID is not known, the server ask the IOS for the string.

	When the IOS sends back a string, the server broadcasts this string to the connected clients.

	When a client connects the server, the server send it all the strings in the cache. The client sends a WINDOW message
	to set the world window that is visible on the client.

	A each tick, the server update some entities (UpdatePerTick) to the connected clients.
	If the entity doesn't exist on the client, it send a ADD message with its entity ID and the string ID.
	The server always sends a POS message for the entites it updates.

// ***************************************************************************/


// ***************************************************************************
//	PASSWORD DB
// ***************************************************************************


MYSQL *DatabaseConnection = NULL;

void disconnectFromDatabase();

void connectToDatabase()
{
	disconnectFromDatabase();
	std::string DatabaseName;
	std::string DatabaseHost;
	std::string DatabaseLogin;
	std::string DatabasePassword;
	try
	{
		DatabaseName = IService::getInstance ()->ConfigFile.getVar("DatabaseName").asString ();
		DatabaseHost = IService::getInstance ()->ConfigFile.getVar("DatabaseHost").asString ();
		DatabaseLogin = IService::getInstance ()->ConfigFile.getVar("DatabaseLogin").asString ();
		DatabasePassword = IService::getInstance ()->ConfigFile.getVar("DatabasePassword").asString ();
	}
	catch(const EConfigFile &e)
	{
		nlwarning(e.what());
		return;
	}

	MYSQL *db = mysql_init(NULL);
	if(db == NULL)
	{
		nlwarning ("mysql_init() failed");
		return;
	}

	DatabaseConnection = mysql_real_connect(db, DatabaseHost.c_str(), DatabaseLogin.c_str(), DatabasePassword.c_str(), DatabaseName.c_str(),0,NULL,0);
	if (DatabaseConnection == NULL || DatabaseConnection != db)
	{
		mysql_close(db);
		nlerror ("mysql_real_connect() failed to '%s' with login '%s' and database name '%s'", DatabaseHost.c_str(), DatabaseLogin.c_str(), DatabaseName.c_str());
		return;
	}
}

void disconnectFromDatabase()
{
	if (DatabaseConnection)
	{
		mysql_close(DatabaseConnection);
		DatabaseConnection = NULL;
	}
}

// ***************************************************************************
//						SERVICE CLASS
// ***************************************************************************

class CMonitorService : public NLNET::IService
{
public:
	bool LoginRequired;
public:
	CMonitorService() : LoginRequired(false) {}
	void init();
	bool update();
	void release();
};

// get instanceof the service
static CMonitorService &getMonitorService()
{
	return *NLMISC::safe_cast<CMonitorService *>(IService::getInstance());
}

// callback for the tick service 'tick' message
void cbTick();

// ***************************************************************************

void clientWantsToConnect ( TSockId from, void *arg )
{
	// Called when a client wants to connect
	nlinfo ("Add client %d", Clients.size());
	Clients.push_back (new CMonitorClient(from));

	
	CMonitorService  &ms = getMonitorService();

	// send params about this sever the client
	CMessage msgout;
	msgout.setType("SERVER_PARAMS");
	uint32 version = 0;
	msgout.serial(version);
	msgout.serial(ms.LoginRequired);
	Server->send(msgout, from);	
	Clients.back()->Authentificated = !ms.LoginRequired;


	// Send all the string in cache to the client
	uint i;
	for (i=0; i<Clients.size(); i++)
	{
		std::map<TYPE_NAME_STRING_ID, std::string>::iterator ite = StringMap.begin ();
		while (ite != StringMap.end ())
		{
			Clients[i]->Str.push_back (ite->first);
			ite++;
		}
	}

}

// ***************************************************************************

void clientWantsToDisconnect ( TSockId from, void *arg )
{
	// Called when a client wants to disconnect
	for (uint i = 0; i < Clients.size(); ++i)
	{
		if (Clients[i]->getSock() == from)
		{
			nlinfo ("Remove client %d", i);
			Clients.erase(Clients.begin()+i);
			return;
		}
	}
	nlwarning ("Client not found for remove");
}

// ***************************************************************************

void clientSetWindow (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Called when a client sent a WINDOW message
	float xmin, ymin, xmax, ymax;
	msgin.serial(xmin);
	msgin.serial(ymin);
	msgin.serial(xmax);
	msgin.serial(ymax);

	for (uint i = 0; i < Clients.size(); ++i)
	{
		if (Clients[i]->getSock() == from && Clients[i]->Authentificated)
		{			
			nlinfo ("Client %d sets window (%.0f,%.0f) (%.0f,%.0f)", i, xmin, ymin, xmax, ymax);
			Clients[i]->setWindow(xmin,ymin,xmax,ymax);
			Clients[i]->resetVision();
			return;
		}
	}
}

// ***************************************************************************

void clientSetBandwidth (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Called when a client sent a WINDOW message
	uint32	bandw;
	msgin.serial(bandw);

	for (uint i = 0; i < Clients.size(); ++i)
	{
		if (Clients[i]->getSock() == from && Clients[i]->Authentificated)
		{
			nlinfo ("Client %d sets bandwidth to %.1f kB/s", i, bandw/1024.0);
			Clients[i]->AllowedUploadBandwidth = bandw;
			return;
		}
	}
}




// ***************************************************************************
void clientAuthentication(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	std::string login;
	std::string password;
	sint version = msgin.serialVersion(0);
	msgin.serial(login);
	msgin.serial(password);

	for (uint i = 0; i < Clients.size(); ++i)
	{
		if (!Clients[i]->Authentificated && Clients[i]->getSock() == from)
		{
			if (!Clients[i]->BadLogin) // don't allow new login attempt while thisflag is set
			{
				// make a db request to to db to see if password is valid
				std::string queryStr = toString("SELECT Password FROM user where Login='%s'", login.c_str());
				int result = mysql_query(DatabaseConnection, queryStr.c_str());
				if (result == 0)
				{
					CMySQLResult sqlResult(DatabaseConnection);
					if (sqlResult.success() && sqlResult.numRows() == 1)
					{
						MYSQL_ROW row =  sqlResult.fetchRow();	
						if (sqlResult.numFields() == 1)
						{
							if (strlen(row[0]) > 2)
							{
								std::string salt = std::string(row[0], row[0] + 2);								
								std::string cryptedVersion = CCrypt::crypt(password, salt);
								if (cryptedVersion == row[0])
								{
									Clients[i]->Authentificated = true;
									// password is  good
									CMessage msgout;
									msgout.setType("AUTHENT_VALID");							
									Server->send(msgout, from);
									return;
								}
							}
						}				
					}
				}
				// fail the authentication
				// Do not send result immediatly to avoid a potential hacker
				// to try a dictionnary or that dort of things
				BadLoginClients.insert(std::make_pair(NLMISC::CTime::getLocalTime() + LOGIN_RETRY_DELAY_IN_MILLISECONDS, Clients[i]));
				Clients[i]->BadLogin =true;
				return;
			}
		}
	}
}

// ***************************************************************************

void cbReceiveString( CMessage& msgin, const string &serviceName, TServiceId  serviceId )
{
	uint32 nameIndex;
	ucstring ucs;
	msgin.serial( nameIndex );
	msgin.serial( ucs );

	// Add the string to the map
	StringMap.insert (std::map<TYPE_NAME_STRING_ID, std::string>::value_type (nameIndex, ucs.toString()));
	StringAsked.erase (nameIndex);

	// Add string a id to send to the clients
	uint i;
	for (i=0; i<Clients.size(); i++)
	{
		Clients[i]->Str.push_back (nameIndex);
	}
}

// ***************************************************************************

TCallbackItem CallbackArray[] =
{
	{ "WINDOW", clientSetWindow },
	{ "BANDW", clientSetBandwidth },
	{ "AUTHENT", clientAuthentication }
};

TUnifiedCallbackItem CallbackArray5[] =
{
	{ "RECV_STRING", cbReceiveString },
};

// ***************************************************************************
//						SERVICE INIT & RELEASE
// ***************************************************************************

static void cbServiceMirrorUp( const std::string& serviceName, TServiceId  serviceId, void * )
{
	if ( serviceName == "IOS" )
		IOSHasMirrorReady = true;

	if ( serviceName == "EGS" )
		EGSHasMirrorReady = true;
}

// ***************************************************************************

static void cbServiceDown( const std::string& serviceName, TServiceId  serviceId, void * )
{
	if ( serviceName == "IOS" )
		IOSHasMirrorReady = false;

	if ( serviceName == "EGS" )
		EGSHasMirrorReady = false;
}

// ***************************************************************************

void CMonitorService::init ()
{
	setVersion (RYZOM_VERSION);

	// Init the server on port
	Server = new CCallbackServer();
	Server->init (48888);
	Server->setConnectionCallback (clientWantsToConnect, NULL);
	Server->setDisconnectionCallback (clientWantsToDisconnect, NULL);
	Server->addCallbackArray (CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));

	// setup the update systems
	setUpdateTimeout(100);

	// read sheet_id.bin and don't prune out unknown files
	CSheetId::init(false);

	// init sub systems
	CMirrors::init(cbTick);
	// CMessages::init();

	// register the service up and service down callbacks
	CMirrors::Mirror.setServiceMirrorUpCallback("*", cbServiceMirrorUp, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbServiceDown, 0);

	LoginRequired = false;

	// if password are required for that service, then connect to db
	CConfigFile::CVar *loginRequiredVar = getMonitorService().ConfigFile.getVarPtr("LoginRequired");
	LoginRequired = loginRequiredVar ? loginRequiredVar->asInt() != 0 : false;
	if (LoginRequired)
	{
		connectToDatabase();
	}
}

// ***************************************************************************

void CMonitorService::release ()	
{
	disconnectFromDatabase();	
	// release sub systems
	// CMessages::release();
	CMirrors::release();

	if (Server)
	{
		delete Server;
		Server = NULL;
	}
}

// ***************************************************************************
//						SERVICE UPDATES
// ***************************************************************************

///update called on each 'tick' message from tick service

void cbTick()
{
}

uint ForceTicks=0;

///update called every complete cycle of service loop
bool CMonitorService::update ()
{
	Server->update();

	uint	iclient;

	for (iclient=0; iclient<Clients.size(); ++iclient)
	{
		CMonitorClient	&client = *Clients[iclient];
		if (client.Authentificated)
		{
			client.update();
		}
	}
	
	// Sent bad login msg to clients at the right time
	NLMISC::TTime currentTime = NLMISC::CTime::getLocalTime();
	while (!BadLoginClients.empty() && BadLoginClients.begin()->first <= currentTime)
	{
		CMonitorClient *client = BadLoginClients.begin()->second;
		if (client != NULL)
		{
			CMessage msgout;
			msgout.setType("AUTHENT_INVALID");		
			Server->send(msgout, client->getSock());
			client->BadLogin = false; // allow to accept login again for that client			
		}
		BadLoginClients.erase(BadLoginClients.begin());
	}

/*
	if (!Clients.empty() && !Entites.empty())
	{
		// Update some primitive
		static uint primitiveToUpdate = 0;
		CConfigFile::CVar *var = ConfigFile.getVarPtr ("UpdatePerTick");
		uint count = 10;
		if (var && (var->Type == CConfigFile::CVar::T_INT))
			count = var->asInt();

		// Loop to the beginning
		if (primitiveToUpdate >= Entites.size())
			primitiveToUpdate = 0;

		// Resize the client array
		// For each client
		uint i;
		for (i=0; i<Clients.size(); i++)
		{
			nlassert (Clients[i]->Entites.size() <= Entites.size());
			Clients[i]->Entites.resize (Entites.size());
		}

		// For each primitive
		uint firstPrimitiveToUpdate = primitiveToUpdate;
		while (count)
		{
			// Present ?
			if (Entites[primitiveToUpdate].Flags & CEntityEntry::Present)
			{
				// One more
				count--;

				// Get the primitive position
				TDataSetRow	entityIndex = TDataSetRow::createFromRawIndex (primitiveToUpdate);
				CMirrorPropValueRO<TYPE_POSX> valueX( TheDataset, entityIndex, DSPropertyPOSX );
				CMirrorPropValueRO<TYPE_POSY> valueY( TheDataset, entityIndex, DSPropertyPOSY );
				CMonitorClient::CPosData posData;
				posData.X = (float)valueX / 1000.f;
				posData.Y = (float)valueY / 1000.f;

				// For each client
				for (i=0; i<Clients.size(); i++)
				{
					// The client
					CMonitorClient &client = *Clients[i];

					// Send position ?
					bool sendPos = false;

					// Clipped ?
					const NLMISC::CVector &topLeft = client.getTopLeft();
					const NLMISC::CVector &bottomRight = client.getBottomRight();
					if ((posData.X>=topLeft.x) && (posData.Y>=topLeft.y) && (posData.X<=bottomRight.x) && (posData.Y<=bottomRight.y))
					{
						// Inside
						if (client.Entites[primitiveToUpdate].Flags & CMonitorClient::CEntityEntry::Present)
						{
							// Send a POS message
							sendPos = true;
						}
						else
						{
							// Send a ADD and a POS message
							CMirrorPropValueRO<TYPE_NAME_STRING_ID> stringId( TheDataset, entityIndex, DSPropertyNAME_STRING_ID);
							CMonitorClient::CAddData addData;
							addData.Id = primitiveToUpdate;
							addData.StringId = stringId;
							addData.EntityId = TheDataset.getEntityId (entityIndex);
							client.Add.push_back (addData);

							sendPos = true;
						}
					}
					else
					{
						// Outside
						// Inside
						if (client.Entites[primitiveToUpdate].Flags & CMonitorClient::CEntityEntry::Present)
						{
							// Send a RMV message
							client.Rmv.push_back (primitiveToUpdate);
						}
					}

					// Send position ?
					if (sendPos)
					{
						CMirrorPropValueRO<TYPE_ORIENTATION> valueT( TheDataset, entityIndex, DSPropertyORIENTATION );
						posData.Id = primitiveToUpdate;
						posData.Tetha = valueT;
						client.Pos.push_back (posData);
					}
				}
			}
			else
			{
				// Not present
				for (i=0; i<Clients.size(); i++)
				{
					// The client
					if (Clients[i]->Entites[primitiveToUpdate].Flags & CMonitorClient::CEntityEntry::Present)
					{
						// One more
						count--;

						// Send a RMV message
						Clients[i]->Rmv.push_back (primitiveToUpdate);
					}
				}
			}

			// Next primitive
			primitiveToUpdate++;

			// Loop to the beginning
			if (primitiveToUpdate >= Entites.size())
				primitiveToUpdate = 0;

			// Return to the first ?
			if (firstPrimitiveToUpdate == primitiveToUpdate)
				break;
		}

		// For each client
		for (i=0; i<Clients.size(); i++)
			Clients[i].update ();
	}
*/
	return true;
}

// ***************************************************************************
//						NLNET_SERVICE_MAIN
// ***************************************************************************
NLNET_SERVICE_MAIN (CMonitorService, "MOS", "monitor_service", 0, CallbackArray5, "", "")

