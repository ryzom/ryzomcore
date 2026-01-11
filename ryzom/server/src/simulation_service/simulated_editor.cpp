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

/////////////
// INCLUDE //
/////////////
#include "simulated_editor.h"
#include "client_cfg.h"
//#include "game_share/generic_xml_msg_mngr.h"
//#include "game_share/msg_client_server.h"
#include "nel/misc/common.h"
#include "game_share/utils.h" // for BOMB_IF

#include "game_share/generic_xml_msg_mngr.h"

#include "nel/net/module_manager.h"
#include "r2_share/object.h"
#include "r2_share/scenario.h"

// test file write as text
#include <iostream>
#include <sstream>

extern CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace R2;

// impulse callbacks
void impulseShardId(NLMISC::CBitMemStream &impulse);
void impulseUserBars(NLMISC::CBitMemStream &impulse);
void impulseNop(NLMISC::CBitMemStream &impulse);

/* 
 * CSimulatedEditor
 */
CSimulatedEditor::CSimulatedEditor( uint id ) :
	CSimulatedClient( id ),
	_DMC( NULL ),
	_ses( sesUninitialized ),
	_bLoggedIn( false )
{
}

CSimulatedEditor::~CSimulatedEditor()
{
	delete _DMC;
	_DMC = NULL;
	_ses = sesUninitialized;
}

/*static*/ void CSimulatedEditor::initImpulseCallbacks()
{
	// initialize impulse message handlers
	GenericMsgHeaderMngr.setCallback( "CONNECTION:SHARD_ID", impulseShardId );

	GenericMsgHeaderMngr.setCallback( "USER:BARS", impulseNop );

	// AJM: NOP these for now, 
	// AJM TODO: double-check that we really want to do this for each of these
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:PHRASE_SEND", impulseNop);
	GenericMsgHeaderMngr.setCallback("STRING:DYN_STRING", impulseNop);
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:RELOAD_CACHE", impulseNop);
	GenericMsgHeaderMngr.setCallback("PHRASE:DOWNLOAD", impulseNop);
	GenericMsgHeaderMngr.setCallback("GUILD:UPDATE_PLAYER_TITLE", impulseNop);
	GenericMsgHeaderMngr.setCallback("HARVEST:CLOSE_TEMP_INVENTORY", impulseNop);
	GenericMsgHeaderMngr.setCallback("DB_GROUP:INIT_BANK", impulseNop);
	GenericMsgHeaderMngr.setCallback("DEATH:RESPAWN_POINT", impulseNop);
	GenericMsgHeaderMngr.setCallback("TEAM:CONTACT_INIT", impulseNop);
	GenericMsgHeaderMngr.setCallback("PVP_FACTION:FACTION_WARS", impulseNop);
	GenericMsgHeaderMngr.setCallback("DB_INIT:INV",	impulseNop);
	GenericMsgHeaderMngr.setCallback("ENCYCLOPEDIA:INIT", impulseNop);
	GenericMsgHeaderMngr.setCallback("DB_UPD_PLR", impulseNop);

	// on receipt of this message, assume login is completed and it is safe to connect to DSS
	GenericMsgHeaderMngr.setCallback("DB_INIT:PLR", impulseDatabaseInitPlayer);
}

void CSimulatedEditor::init()
{
	BOMB_IF( ClientCfg.Local, "Config file set to local mode!", return )

	_ses = sesInitialized;
}

// login to frontend server
void CSimulatedEditor::login()
{
	_CurrentContext = this;

	IModuleManager &mm = IModuleManager::getInstance();

	// login
	bool res = autoLogin( "", "", true );
	_Direction = (TDirection)(uint)frand( DMaxDir+1 );
	if ( _Direction > DMaxDir )
		_Direction = DMaxDir;

	// set the unique name root
	IModuleManager::getInstance().setUniqueNameRoot( toString("U%u", _Id ) );
	string gwName = toString( "simEditorGw%u", _Id );

	// create a local gateway module
	_ModuleGateway = mm.createModule( "StandardGateway", gwName, "" );
	nlassert( _ModuleGateway != NULL );

	// connect the client gateway to the FES
	CCommandRegistry::getInstance().execute( gwName + ".transportAdd FEClient fec", *InfoLog );
	CCommandRegistry::getInstance().execute( gwName + ".transportCmd fec(open)", *InfoLog );

	_CurrentContext = NULL;
}

// connect client editor to dynamic scenario service
void CSimulatedEditor::connectToDSS()
{
	_CurrentContext = this;

	IModuleManager &mm = IModuleManager::getInstance();

	// get the gateway socket
	string gwName = toString( "simEditorGw%u", _Id );
	NLNET::IModuleSocket *socketGw = mm.getModuleSocket( gwName );
	nlassert( socketGw != NULL );

	// create a simulated dynamic map client
	string simEditorName = toString( "simEditor%u", _Id );
	_DMC = new CDynamicMapClient( simEditorName, socketGw, NULL );	// getLua().getStatePointer() );

	// create the simulated client edition and animation modules
	_DMC->init( _Id, NULL ); 

	_CurrentContext = NULL;
}

// test
void CSimulatedEditor::test()
{
	if( false )
	{
		// load and run a lua test script
		nlassert( _DMC );
		_DMC->doFile( "testing123.lua" );
	}
	nlinfo( "SimEditor %d  started", _Id );
}

bool CSimulatedEditor::isLoggedIn()
{
	return getNetworkConnection().isConnected();
}

bool CSimulatedEditor::isConnected() const
{
	if( !_DMC )
		return false;
	return _DMC->isSEMConnected();
}

// test creating and uploading a scenario to the ServerEditionModule
void CSimulatedEditor::testCreateScenario( const std::string &fileName )
{
	if( !isConnected() )
	{
		nlwarning( "Server Edition Module is not connected!" );
		return;
	}
	
	// load a scenario
	CObject *pData = loadScenarioData( fileName );
	if( !pData )
	{
		nlwarning( "testCreateScenario: could not load scenario." );
		return;
	}

	// load scenario rtdata
	nlinfo("reading rtdata from %s.rt.bin", fileName.c_str());
	CObject *pRtData = loadScenarioRtData( fileName );
	if( !pRtData )
	{
		nlwarning( "testCreateScenario: could not load scenario rtdata." );
		return;
	}

	if( !checkScenarioRtData( pRtData ) )
	{
		nlwarning( "testCreateScenario: scenario rtdata failed validity check." );
		delete pData;
		delete pRtData;
		return;
	}

	// create scenario stub (high-level)
	CObject *pScenarioStub = createScenarioStub( pData );
	if( !pScenarioStub )
	{
		nlwarning( "testCreateScenario: scenario description failed validity check." );
		delete pData;
		delete pRtData;
		delete pScenarioStub;
		return;
	}

	// send a CreateScenario message to the SEM to get a slot
	_DMC->requestCreateScenario( pScenarioStub );

	// send an UploadRtData message to the SEM to upload the scenario
	_DMC->requestUpdateRtScenario( pRtData );

	delete pData;
	delete pScenarioStub;
	delete pRtData;
}

// load scenario data file, and create and return scenario stub (description only)
CObject *CSimulatedEditor::loadScenarioStub( const std::string &filename )
{
	// load scenario (high-level)
	nlinfo("reading data from %s", filename.c_str());
	CObject *pData = loadScenarioData( filename );
	if( !pData )
	{
		nlwarning( "could not load scenario %s", filename.c_str() );
		return NULL;
	}

	// create scenario stub (high-level) with just the description
	CObject *pScenarioStub = createScenarioStub( pData );
	if( !pScenarioStub )
	{
		nlwarning( "failed to create stub for scenario %s", filename.c_str() );
		delete pData;
		delete pScenarioStub;
		return NULL;
	}

	delete pData;
	return pScenarioStub;
}

// load scenario rtdata file, validate, and return rtdata
CObject *CSimulatedEditor::loadRtData( const std::string &filename )
{
	// load scenario rtdata
	nlinfo("reading rtdata for scenario %s", filename.c_str());
	CObject *pRtData = loadScenarioRtData( filename );
	if( !pRtData )
	{
		nlwarning( "could not load rtdata for scenario %s", filename.c_str() );
		return NULL;
	}

	if( !checkScenarioRtData( pRtData ) )
	{
		nlwarning( "rtdata failed validity check for scenario %s", filename.c_str() );
		delete pRtData;
		return NULL;
	}
	return pRtData;
}

// upload scenario stub and rtdata to the ServerEditionModule
void CSimulatedEditor::uploadScenario( CObject *pStub, CObject *pRtData )
{
	// send a CreateScenario message to the SEM to get a slot
	_DMC->requestCreateScenario( pStub );

	// send an UploadRtData message to the SEM to upload the scenario
	_DMC->requestUpdateRtScenario( pRtData );
}

// run scenario (aka "start test" or "go live") and enter "animation mode"
void CSimulatedEditor::runScenario()
{
	_DMC->requestGoTest();
}

// start act
void CSimulatedEditor::startAct( uint actId )
{
	_DMC->requestStartAct( actId );
}

// end scenario and return to "edition mode"
void CSimulatedEditor::endScenario()
{
	_DMC->requestStopTest();
}

// create a scenario stub, with just enough data to pass validation checks on the SEM
CObject *CSimulatedEditor::createScenarioStub( const R2::CObject *pData ) const
{
	CObject *pStub = new CObjectTable();

	if( pData && pData->findAttr("Description") )
	{
		CObject *description = pData->findAttr("Description");
		pStub->add( "Description", description->clone() );
	}
	else
	{
		CObject *pDesc = new CObjectTable();
		pStub->add( "Description", pDesc );
		pDesc->add( "LevelId", (uint)0 );
		pDesc->add( "LocationId", 1 );
		pDesc->add( "EntryPointId", (uint)0 );
		pDesc->add( "RuleId", (uint)0 );
		pDesc->add( "MaxEntities", 50 );
		pDesc->add( "MaxPlayers", 5 );
		pDesc->add( "Title", "my Title");
		pDesc->add( "ShortDescription", "my ShortDescription");
	}
	
	// see if this will pass SEM validation
	if( true )
	{
		CObject* description = pStub->findAttr("Description");
	
		if (!description)
		{
			nlwarning("Invalid scenario");
		}
		CObject* levelId = description->findAttr("LevelId");
		CObject* locationId = description->findAttr("LocationId");
		CObject* entryPointId = description->findAttr("EntryPointId");
		CObject* ruleId = description->findAttr("RuleId");
		CObject* maxEntities = description->findAttr("MaxEntities");
		CObject* maxPlayers = description->findAttr("MaxPlayers");
		CObject* title = description->findAttr("Title");
		CObject* shortDescription = description->findAttr("ShortDescription");


		if (!locationId || !levelId || !entryPointId || !ruleId || !maxEntities || !maxPlayers
			|| !title || !shortDescription

			|| !levelId->isNumber() || !ruleId->isNumber()
			|| !locationId->isNumber() || !entryPointId->isNumber()			
			|| !maxEntities->isNumber()  || !maxPlayers->isNumber()
			|| !title->isString() || !shortDescription->isString()
			)
		{
			nlwarning("Invalid scenario");
		}

	}
	return pStub;	
}

// load scenario data from a lua file
CObject *CSimulatedEditor::loadScenarioData( const std::string &fileName ) const
{
	string fileplusr2(fileName);
	uint len = fileplusr2.size();
	const char *szfn = fileplusr2.c_str();
	if( (len < 4) || (0 != stricmp(&szfn[len-3], ".r2")) )
		fileplusr2.append(".r2");

	// try to read a scenario from a lua file
	nlinfo( "loading scenario file %s...", fileplusr2.c_str() );
	CObject *pData = _DMC->loadScenario( fileplusr2 );
	if( !pData )
	{
		nlwarning("Error while loading scenario %s", fileplusr2.c_str());
		return NULL;
	}

	nlinfo( "...scenario loaded.", fileplusr2.c_str() );
	return pData;
}

// load rtdata for a scenario from an xxx,rt.bin file
CObject *CSimulatedEditor::loadScenarioRtData( const std::string &fileName ) const
{
	CObject *pRtData = NULL;

	string fileplusr2(fileName);
	uint len = fileplusr2.size();
	const char *szfn = fileplusr2.c_str();
	if( (len>3) && !stricmp(&szfn[len-3], ".r2") )
	{
		fileplusr2.erase(len-3);
		fileplusr2.append(".bin");
	}
	else if( (len<8) || (0 != stricmp(&szfn[len-7], ".rt.bin")) )
		fileplusr2.append(".rt.bin");

	// try to open the file for read
	CIFile input;
	bool fileIsOpen = false;

	try
	{
		if( !input.open( fileplusr2 ) )
		{
			nlwarning( "Scenario rtdata file %s not found.", fileplusr2.c_str() );
			return NULL;
		}
		fileIsOpen=true;

		CObjectSerializer serializer;
		input.serial( serializer );
		pRtData = serializer.takeData();

		// try writing the data back out, for comparison with the original
		if( false )
		{
			// as binary
			COFile output;
			output.open("outpout.rt.check.bin");
			CObjectSerializer serializer( pRtData );
			serializer.serial(output);
			output.flush();
			output.close();
		}

		if( false )
		{
			// as text
			COFile output;
			std::stringstream ss;
			output.open("outpout.rt.check.txt");
			pRtData->serialize(ss);
			std::string str = ss.str();
			output.serial(str);
			output.flush();
			output.close();
		}
	}
	catch(NLMISC::Exception &)
	{
		nlwarning( "Problem reading rtdata file %s.", fileplusr2.c_str() );
	}
	// close file outside exception handling in case exception thrown somewhere strange
	if( fileIsOpen )
		input.close();

	return pRtData;
}

// check rtdata for validity
// AJM: this is basically duplicated from Server edition module
bool CSimulatedEditor::checkScenarioRtData( CObject *rtData ) const
{
#define MAX_NPCS 50		// hardcoded in Server edition module
	
	uint32 nbActs = 0;
	uint32 nbStates = 0;
	uint32 maxStates = 0;
	uint32 maxNpcs = 0;
	uint32 baseActCost = 0;
	uint32 baseActStates = 0;

	CObject* acts = rtData->findAttr("Acts");
	if( acts && acts->isTable() )
	{
		uint32 nbActs = acts->getSize();
		for(uint32 i = 0;i<nbActs;i++)
		{
			CObject * act = acts->getValue(i);
			CObject * npcs = act->findAttr("Npcs");
			CObject * states = act->findAttr("AiStates");
			if (states && states->isTable())
			{
				uint32 max2 =  states->getSize();
				if (i==0)
				{
					baseActStates = max2;
				}
				else
				{
					if(baseActStates + max2 > maxStates)
					{
						maxStates = baseActStates + max2;
					}
				}
				nlinfo("states count in act %d : %d",i,max2);
			}
			if(npcs && npcs->isTable())
			{
				uint32 max2 = npcs->getSize();
				if(i==0)
				{
					baseActCost = max2;
				}
				else
				{
					if (baseActCost+max2 > MAX_NPCS)
					{
						nlwarning("too many npcs: %d",(baseActCost+max2));
						return false;
					}
					
					if (maxNpcs < baseActCost+max2)
					{
						maxNpcs = baseActCost+max2;
					}

				}
			}
			else
			{
				nlwarning("checkScenario: no npcs in act %d!", i);
				return false;
			}
		}
	}
	else
	{
		nlwarning("checkScenarioRtData: no acts!");
		return false;
	}
	nlinfo("Number of Acts : %d",maxNpcs);
	nlinfo("Max number of Npcs : %d",maxNpcs);
	nlinfo("Max number of States : %d",maxStates);
	return true;
}

uint CSimulatedEditor::getNumActs( R2::CObject *rtData )
{
	uint numActs = 0;
	CObject* acts = rtData->findAttr("Acts");
	if( acts && acts->isTable() )
	{
		numActs = acts->getSize();
	}
	return numActs;
}

// impulse message handlers
void impulseNop(NLMISC::CBitMemStream &impulse)
{
	// just NOP, we don't care about these messages
}

void impulseShardId(NLMISC::CBitMemStream &impulse)
{
	// received SHARD_ID
	uint32	shardId;
	impulse.serial(shardId);

	std::string	webHost;
	impulse.serial(webHost);

	nlinfo("WEB: Received SHARD_ID %d, web hosted at '%s'", shardId, webHost.c_str());
}

// on receipt of this message, assume login is completed and it is safe to connect to DSS
void impulseDatabaseInitPlayer(NLMISC::CBitMemStream &impulse)
{
	uint32 userId = CSimulatedClient::currentContext()->getNetworkConnection().getUserId();
	nlinfo( "DB_INIT:PLR received for user %d", userId );
}